// =============================================================================
//  test_dtm_channel_map.cpp  --  AI(W906-I03b) 20261005 (Ifor01)
//
//  I-03b 第二步（FROM_IFOR §1 1005 13:4x；方案 docs/I03B_HT9050_DTM_CHANNEL_MAP.md＝MR !201，Ifor 1005 定 D1～D4 照建議）：
//  HT9050 的溫區全部在台達 DTM 上（3 站 24 通道），用新開關 `[TempCtrl] DTM_CHANNEL_MAP=HT9050` 打開 EJ1N/DtmChannelMap 的表。
//
//    [1] 表：預設不啟用（每個代號都查不到通道、通道數 0）；"" 與不認得的名字＝不啟用；" ht9050 "（大小寫、空白不管）＝啟用：
//        24 通道、19 列、站 2 CH4-8 空、熱風槍 K-type、SLK-1～8 不武裝、反查一致
//    [2] 表跟硬體工作簿的 24 列逐列相同：代號、感測器、confirmed＝武裝／derived＝不武裝／spare＝空。24 列由本測試自己帶
//        （kWorkbook，W-81 1006）；`.claude/skills/ht9050-hw/data/HT9050-TempMap.json`（Steven 0924）有的話再交叉比對，沒有就略過
//    [3] 開機建表單：表沒開＋USE_16_HEATER=0 ⇒ 不建；表開＋USE_16_HEATER=0（HT9050 現值）⇒ 建 uDTME08Control、通道數 24、
//        每個面板設定 SV＝0（golden 是 edSV "30"）
//    [4] 接假 DTM：開機寫入只到 3 站（站 4 一次都沒問）；感測器型別逐通道（站 2＝PT100、K、K、PT100…）；開機 SV 全 0；之後讀 3 站 PV
//    [5] 溫控狀態機（DoThermo，DTK4848 序列埠是模擬的，假溫控器只回寫入）跑恆溫：USE_16_HEATER=5（以後 HT9050 可能的值）：
//        DUT 1 的 SV＝GetFactSetTemp(tcDUT1, 固定溫 60)（golden 序列路徑的算法，不是 DTM Index 路徑的工作溫度 80）、Chamber＝工作溫度 80；
//        SV 真的寫進假 DTM；SLK 不武裝 ⇒ 站 3 的 SV 一直是 0，但 PV 照讀；Chamber PV 不換算、熱風槍 PV 進 UN150ReadReal；
//        表上的通道一次都沒有走序列埠；DoSetSVOfDTME08（golden Index-only）不動面板
//    [6] 假 DTM 斷線 ⇒ 連 5 次以上 ⇒ UN150Read=999＋UN150CommError（跟 golden case 2500 一樣）
//    [7] 讀原始碼：database.cpp 只讀（ReadString，不用會種鍵的 CheckAndReadIniDataGeneral）；wb_serve 還沒有呼叫武裝
//    [8] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同
//  反向驗證（第 15 條）見 MR 說明：表對調兩列、SLK 改武裝、拿掉 case 100 的分支、拿掉 DoSetSVOfDTME08 的提早返回、通道數改 32 ⇒ 各自變紅。
//  不開 COM 埠（Comm2 強制 SIM）、只連 127.0.0.1（本測試自己開的假 DTM）。約 20～40 秒。
// =============================================================================
#include <winsock2.h>                     // before anything that pulls <windows.h>
#include "atester_shims.h"                // COM2
#include "MachineType.h"
#include "cmydef.h"                       // USE_16_HEATER, InitialOK, TC401HeaterControl, ATC_SYSTEM, UN150Read, bUT150Install, SHUTTLE_COOLING
#include "cpublic.h"                      // g_pDTKComm
#include "cprod.h"                        // Temperature, TestIF_File
#include "Config.h"                       // IniConfig
#include "CosFunction.h"                  // CosFunction
#include "LastSet.h"                      // LastSet.iTemperature
#include "bthermo.h"                      // DoThermo, DoSetSVOfDTME08, iThermoTask
#include "acarry_shims.h"                 // ATC_InterfaceForm
#include "mysensor.h"                      // Sen[SnHeaterDoor] / Sen[SnHeaterDoor2]
#include "myswitch.h"                      // SW[SwHeaterRelay]
#include "forms/fDTME08.h"
#include "EJ1N/uDTME08Control.h"
#include "EJ1N/DtmChannelMap.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <set>
#include <string>
#include <vector>

void W906_DTME08_CreateFormBoot();        // forms/fDTME08.cpp EOF
double GetConvertTemp(int Addr, double T);   // bthermo.cpp
double GetFactSetTemp(int Addr, double T);   // bthermo.cpp
void W906_PumpTempComm2();                   // rs232.cpp EOF

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_dtm_channel_map.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static bool Has(const std::string& s, const char* needle) { return s.find(needle)!=std::string::npos; }

// ---------------------------------------------------------------------------
//  The fake DTM (same as tests/test_dtme08_cycle.cpp): Modbus/TCP on 127.0.0.1:<ephemeral>, one client, pumped from the
//  test loop.  Per station 0x300 words: +0x000 SV, +0x028 sensor type, +0x0D0 Out2, +0x0F8 cycle time, +0x268 PV, +0x288 status.
// ---------------------------------------------------------------------------
struct Req { int fc; int station; int off; int qty; std::vector<int> vals; };
struct FakeDtm
{
    SOCKET l=INVALID_SOCKET, c=INVALID_SOCKET;
    int port=0;
    std::string in;
    int reg[4][0x300];
    std::vector<Req> reqs;
    FakeDtm() { std::memset(reg, 0, sizeof(reg)); }
    bool Start()
    {
        l=::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in a; std::memset(&a, 0, sizeof(a));
        a.sin_family=AF_INET; a.sin_addr.s_addr=::inet_addr("127.0.0.1"); a.sin_port=0;
        if(l==INVALID_SOCKET || ::bind(l, (sockaddr*)&a, sizeof(a))!=0 || ::listen(l, 2)!=0) return false;
        int len=sizeof(a); ::getsockname(l, (sockaddr*)&a, &len); port=::ntohs(a.sin_port);
        return true;
    }
    void Stop()
    {
        if(c!=INVALID_SOCKET) { ::closesocket(c); c=INVALID_SOCKET; }
        if(l!=INVALID_SOCKET) { ::closesocket(l); l=INVALID_SOCKET; }
    }
    static bool Readable(SOCKET s) { fd_set r; FD_ZERO(&r); FD_SET(s, &r); timeval tv; tv.tv_sec=0; tv.tv_usec=0; return ::select(0, &r, 0, 0, &tv)>0; }
    void Pump()
    {
        if(c==INVALID_SOCKET && l!=INVALID_SOCKET && Readable(l)) c=::accept(l, 0, 0);
        if(c==INVALID_SOCKET) return;
        while(Readable(c))
        {
            char b[512]; const int n=::recv(c, b, sizeof(b), 0);
            if(n<=0) { ::closesocket(c); c=INVALID_SOCKET; return; }
            in.append(b, (size_t)n);
        }
        while(in.size()>=7)
        {
            const unsigned char* u=(const unsigned char*)in.data();
            const size_t total=6+((u[4]<<8)|u[5]);
            if(in.size()<total) break;
            Answer(in.substr(0, total));
            in.erase(0, total);
        }
    }
    void Answer(const std::string& f)
    {
        const unsigned char* u=(const unsigned char*)f.data();
        Req q; q.fc=u[7];
        const int addr=(u[8]<<8)|u[9];
        q.station=addr>>12; q.off=addr&0xFFF; q.qty=(u[10]<<8)|u[11];
        std::string out(f.substr(0, 7));
        if(q.fc==3 && q.station<4 && q.off+q.qty<=0x300)
        {
            out+=(char)3; out+=(char)(q.qty*2);
            for(int i=0; i<q.qty; i++) { const int v=reg[q.station][q.off+i]&0xFFFF; out+=(char)(v>>8); out+=(char)(v&0xFF); }
        }
        else if(q.fc==16 && q.station<4 && q.off+q.qty<=0x300)
        {
            for(int i=0; i<q.qty; i++) { const int v=(u[13+i*2]<<8)|u[14+i*2]; q.vals.push_back(v); reg[q.station][q.off+i]=v; }
            out+=f.substr(7, 5);
        }
        else
            return;
        out[4]=(char)(((out.size()-6)>>8)&0xFF); out[5]=(char)((out.size()-6)&0xFF);
        reqs.push_back(q);
        ::send(c, out.data(), (int)out.size(), 0);
    }
};

// The serial side: Comm2 in SIM.  A fake DTK4848 answers SV writes (function 06, echoed: golden case 2500 takes "6" at
// position 5 as the write acknowledgement) so the uninstalled channels' first-round SV 0 does not wait 0.5 s each, and
// records the address of every frame (":%02X" = Addr+1, cpublic.cpp DTK4848WordWriteNoSucm / ReadNoSucm).
struct FakeDtk
{
    std::string acc;
    std::set<int> addrs;
    int frames=0;
    void Serve()
    {
        const std::vector<char>& v=COM2->Comm2->SimTxBuffer();
        acc.append(v.begin(), v.end());
        COM2->Comm2->SimClearTx();
        for(;;)
        {
            const size_t a=acc.find(':');
            if(a==std::string::npos) { acc.clear(); return; }
            const size_t e=acc.find("\r\n", a);
            if(e==std::string::npos) { acc.erase(0, a); return; }
            const std::string f=acc.substr(a, e+2-a);
            acc.erase(0, e+2);
            if(f.size()<7) continue;
            frames++;
            addrs.insert((int)std::strtol(f.substr(1, 2).c_str(), 0, 16)-1);
            if(f.substr(3, 2)=="06") COM2->Comm2->SimInjectReceive(f.data(), (Spcomm::Word)f.size());
        }
    }
};

static FakeDtm g_srv;
static FakeDtk g_dtk;
static void DtmTick()
{
    g_srv.Pump();
    frmDTME08->W906_PollSocket();
    frmDTME08->DoDTME08Cycle();
    g_srv.Pump();
}
static void ThermoTick()
{
    DoThermo();
    g_dtk.Serve();
}
static bool SpinUntil(DWORD ms, bool bThermo, const std::function<bool()>& done)
{
    const DWORD t0=::GetTickCount();
    while(::GetTickCount()-t0<ms)
    {
        DtmTick();
        if(bThermo) for(int k=0; k<8; k++) ThermoTick();
        if(done()) return true;
        ::Sleep(1);
    }
    return done();
}
static const Req* FindReq(int fc, int station, int off)
{
    for(const Req& q : g_srv.reqs) if(q.fc==fc && q.station==station && q.off==off) return &q;
    return 0;
}
static int CountStation(int station)
{
    int n=0;
    for(const Req& q : g_srv.reqs) if(q.station==station) n++;
    return n;
}
static bool Near(double a, double b, double tol) { return a>=b-tol && a<=b+tol; }

// one channel line of HT9050-TempMap.json: { "idx": 9, ..., "tcValue": 27, ..., "sensor": "K-Type", ..., "confidence": "confirmed" }
struct JRow { int idx=-1; int tc=-1; std::string sensor, conf; };
static std::string JStr(const std::string& line, const char* key)
{
    const std::string k=std::string("\"")+key+"\": \"";
    const size_t a=line.find(k);
    if(a==std::string::npos) return "";
    const size_t b=line.find('"', a+k.size());
    return line.substr(a+k.size(), b-(a+k.size()));
}
static int JInt(const std::string& line, const char* key)
{
    const std::string k=std::string("\"")+key+"\": ";
    const size_t a=line.find(k);
    if(a==std::string::npos || line.compare(a+k.size(), 4, "null")==0) return -1;
    return std::atoi(line.c_str()+a+k.size());
}

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("DtmChannelMap"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_dtm_channel_map (I-03b): the HT9050 DTM channel map, the DTM form and DoThermo against a fake DTM\n");
    static const char* const kReal[2]={ "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for(int i=0; i<2; i++) before[i]=Slurp(kReal[i]);

    // ---- [1] the table ------------------------------------------------------------------------------
    std::printf("[1] the table\n");
    CHECK(!DtmMap_Active() && DtmMap_ChannelCount()==0 && DtmMap_ChannelOf(tcHotPlate1)==-1 && DtmMap_AddrOf(0)==-1, "[1] inactive by default: no channel, count 0");
    CHECK(DtmMap_Select("")==false && !DtmMap_Active(), "[1] \"\" -> inactive");
    CHECK(DtmMap_Select("HT9051")==false && !DtmMap_Active() && DtmMap_ChannelOf(tcChamber)==-1, "[1] unknown name -> inactive (golden)");
    CHECK(DtmMap_Select(" ht9050 ")==true && DtmMap_Active() && std::string(DtmMap_Name())=="HT9050", "[1] \" ht9050 \" -> HT9050 (case / blanks ignored)");
    CHECK(DtmMap_ChannelCount()==24 && DtmMap_RowCount()==19, "[1] 3 stations x 8 = 24 channels, 19 used");
    CHECK(DtmMap_ChannelOf(tcHotPlate1)==0 && DtmMap_ChannelOf(tcDUT4)==7 && DtmMap_ChannelOf(tcChamber)==8 &&
          DtmMap_ChannelOf(tcHeatGun2)==10 && DtmMap_ChannelOf(tcAa1)==16 && DtmMap_ChannelOf(tcBd1)==23, "[1] channel = (station-1)*8 + (ch-1)");
    CHECK(DtmMap_ChannelOf(tcAa2)==-1 && DtmMap_ChannelOf(tcHead1)==-1 && DtmMap_ChannelOf(tcSocket)==-1, "[1] zones not on the map -> -1 (golden path)");
    {
        bool bEmpty=true;
        for(int ch=11; ch<=15; ch++) bEmpty=bEmpty && DtmMap_AddrOf(ch)==-1 && !DtmMap_ArmedCh(ch) && DtmMap_SensorOf(ch)==edmsPT100;
        CHECK(bEmpty, "[1] station 2 CH4-8 empty: no zone, not armed, PT100");
        bool bBack=true, bUnique=true;
        std::set<int> seen;
        for(int i=0; i<DtmMap_RowCount(); i++)
        {
            const TDtmMapRow* w=DtmMap_Row(i);
            const int ch=(w->iStation-1)*8+(w->iCh-1);
            bBack=bBack && DtmMap_AddrOf(ch)==w->iAddr && DtmMap_ChannelOf(w->iAddr)==ch;
            bUnique=bUnique && seen.insert(w->iAddr).second;
        }
        CHECK(bBack && bUnique, "[1] every row: AddrOf(ChannelOf) round trip, no eTempControll twice");
        bool bSlk=true;
        for(int ch=16; ch<24; ch++) bSlk=bSlk && !DtmMap_ArmedCh(ch);
        CHECK(bSlk && DtmMap_ArmedCh(0) && DtmMap_ArmedCh(10), "[1] SLK-1..8 (station 3) NOT armed; Hotplate 1 / Hot Air 2 armed");
        CHECK(DtmMap_SensorOf(8)==edmsPT100 && DtmMap_SensorOf(9)==edmsKType && DtmMap_SensorOf(10)==edmsKType && DtmMap_SensorOf(4)==edmsPT100,
              "[1] Chamber PT100, Hot Air 1 / 2 K-type, DUT 1 PT100");
    }

    // ---- [2] the table against the hardware workbook ----------------------------------------------------
    //  AI(W906-W81) 20261006: NB2-1 (laptop card W-81, CHAT_JIMMY 1006 06:2x; the card was Ifor01's): the workbook's 24 rows now travel
    //  WITH the test (kWorkbook below = .claude/skills/ht9050-hw/data/HT9050-TempMap.json, generated 2026-09-24 from
    //  HP-9050開發機資料-20260717.xlsx / 04_溫控器站號). That JSON lives only in a full checkout -- the machine's build tree and NB2's
    //  sparse worktrees have no .claude/skills, so [2] was red there for a missing file, not for a wrong table. The table is always checked
    //  against the fixture; the JSON, when present, is cross-checked against the fixture (change one, change the other); when absent, one
    //  line says the cross-check was skipped (not a failure).
    static const JRow kWorkbook[24]={
        { 0,  0, "PT100",  "confirmed" }, { 1,  1, "PT100",  "confirmed" }, { 2,  2, "PT100",  "confirmed" }, { 3,  3, "PT100",  "confirmed" },
        { 4, 29, "PT100",  "confirmed" }, { 5, 30, "PT100",  "confirmed" }, { 6, 31, "PT100",  "confirmed" }, { 7, 32, "PT100",  "confirmed" },
        { 8,  9, "PT100",  "confirmed" }, { 9, 27, "K-Type", "confirmed" }, {10, 28, "K-Type", "confirmed" },
        {11, -1, "",       "spare"     }, {12, -1, "",       "spare"     }, {13, -1, "",       "spare"     }, {14, -1, "",       "spare"     },
        {15, -1, "",       "spare"     },
        {16, 11, "PT100",  "derived"   }, {17, 12, "PT100",  "derived"   }, {18, 13, "PT100",  "derived"   }, {19, 14, "PT100",  "derived"   },
        {20, 15, "PT100",  "derived"   }, {21, 16, "PT100",  "derived"   }, {22, 17, "PT100",  "derived"   }, {23, 18, "PT100",  "derived"   } };
    std::printf("[2] against the hardware workbook (24 rows carried by this test; the JSON cross-checked when present)\n");
    {
        int bad=0;
        for(const JRow& r : kWorkbook)
        {
            const bool bSpare=(r.conf=="spare");
            const int wantSensor=(r.sensor=="K-Type") ? (int)edmsKType : (int)edmsPT100;
            const bool ok=DtmMap_AddrOf(r.idx)==(bSpare ? -1 : r.tc) &&
                          DtmMap_SensorOf(r.idx)==wantSensor &&
                          DtmMap_ArmedCh(r.idx)==(r.conf=="confirmed");
            if(!ok) { bad++; std::printf("    idx %d: tc %d/%d sensor %s armed %d conf %s\n", r.idx, DtmMap_AddrOf(r.idx), r.tc, r.sensor.c_str(), (int)DtmMap_ArmedCh(r.idx), r.conf.c_str()); }
        }
        CHECK(bad==0, "[2] every row: eTempControll, sensor, confirmed = armed / derived = not armed / spare = empty");

        const std::string js=Slurp(root+"/../.claude/skills/ht9050-hw/data/HT9050-TempMap.json");
        if(js=="<missing>")
            std::printf("  (.claude/skills/ht9050-hw/data/HT9050-TempMap.json is not in this tree -- JSON cross-check skipped; the 24 rows above were checked)\n");
        else
        {
            std::vector<JRow> rows;
            size_t p=0;
            while((p=js.find("\"idx\":", p))!=std::string::npos)
            {
                const size_t e=js.find('}', p);
                const std::string line=js.substr(p-2, e-p+2);
                JRow r; r.idx=JInt(line, "idx"); r.tc=JInt(line, "tcValue"); r.sensor=JStr(line, "sensor"); r.conf=JStr(line, "confidence");
                rows.push_back(r);
                p=e;
            }
            CHECK(rows.size()==24, "[2] 24 channel rows in the JSON");
            int diff=0;
            for(const JRow& r : rows)
            {
                const bool known=r.idx>=0 && r.idx<24;
                const JRow& w=kWorkbook[known ? r.idx : 0];
                if(!known || w.idx!=r.idx || w.tc!=r.tc || w.sensor!=r.sensor || w.conf!=r.conf)
                {
                    diff++;
                    std::printf("    JSON idx %d: tc %d sensor %s conf %s -- fixture says tc %d sensor %s conf %s\n", r.idx, r.tc, r.sensor.c_str(), r.conf.c_str(),
                                w.tc, w.sensor.c_str(), w.conf.c_str());
                }
            }
            CHECK(diff==0, "[2] the JSON still says what kWorkbook says (the workbook changed? update both)");
        }
    }

    // ---- [3] boot ----------------------------------------------------------------------------------
    std::printf("[3] boot\n");
    WSADATA wsa;
    CHECK(::WSAStartup(MAKEWORD(2, 2), &wsa)==0, "WSAStartup");
    COM2->Comm2->SetSimMode(true);
    COM2->Comm2->StartComm();
    CHECK(g_pDTKComm==COM2->Comm2, "[3] DTK4848 writes go to the SIM Comm2");
    InitialOK=true;
    bSystemClose=false;
    ATC_SYSTEM=0;
    ATC_InterfaceForm->iATC_MODE_TYPE=0;
    Temperature.bATCActiveCooling=false;
    TC401HeaterControl=DTK4848;                             // HT9050 Gerneral.ini HEATER_CTRL_TYPE=4 (TO_IFOR 1003 11:3x)
    USE_16_HEATER=eht4Heater;                               // HT9050 today (Ifor 1005: keep 0)
    DtmMap_Select("");
    frmDTME08->W906_CreateFormBody();
    CHECK(frmDTME08->W906_Control()==NULL, "[3] map off + USE_16_HEATER 0 -> no DTM form (golden)");
    DtmMap_Select("HT9050");
    W906_DTME08_CreateFormBoot();
    uDTME08Control* ctl=frmDTME08->W906_Control();
    CHECK(ctl!=NULL, "[3] map on + USE_16_HEATER 0 -> uDTME08Control built");
    CHECK(ctl!=NULL && frmDTME08->GetChannelNumber()==24, "[3] 24 channels (3 stations)");
    if(ctl==NULL) { std::printf("test_dtm_channel_map: %d passed, %d failed\n", g_pass, g_fail); return 1; }
    {
        bool b0=true;
        for(int i=0; i<24; i++) b0=b0 && frmDTME08->GetPalGroup(i)->GetSettingSV()==0.0 && frmDTME08->GetPalGroup(i)->edSV->Text=="0";
        CHECK(b0, "[3] every panel boots with setting SV 0 / edSV \"0\" (golden \"30\")");
    }

    // ---- [4] the fake DTM --------------------------------------------------------------------------
    std::printf("[4] fake DTM on 127.0.0.1\n");
    CHECK(g_srv.Start(), "[4] fake DTM listening");
    for(int s=0; s<3; s++)
        for(int ch=0; ch<8; ch++) { g_srv.reg[s][0x268+ch]=250+s*10+ch; g_srv.reg[s][0x288+ch]=0x04; g_srv.reg[s][0x000+ch]=300; }
    g_srv.reg[1][0x268+0]=321;                              // Chamber PV 32.1
    g_srv.reg[1][0x268+1]=456;                              // Hot Air 1 PV 45.6
    g_srv.reg[2][0x268+0]=278;                              // SLK-1 PV 27.8
    frmDTME08->W906_UseRealSocket();
    ctl->SetSocketAddress("127.0.0.1");
    ctl->SetSocketPort(AnsiString(g_srv.port));
    const bool bBoot=SpinUntil(30000, false, []{ return FindReq(16, 2, 0x000)!=0 && FindReq(3, 2, 0x268)!=0 && FindReq(3, 2, 0x000)!=0; });
    CHECK(bBoot, "[4] boot writes then PV / SV reads reached station 3");
    const Req* rs1=FindReq(16, 1, 0x028);
    CHECK(rs1 && rs1->vals==std::vector<int>({12, 0, 0, 12, 12, 12, 12, 12}), "[4] station 2 sensor type per channel: PT100(12), K(0), K(0), PT100 x 5");
    const Req* rs0=FindReq(16, 0, 0x028);
    const Req* rs2=FindReq(16, 2, 0x028);
    CHECK(rs0 && rs0->vals==std::vector<int>(8, 12) && rs2 && rs2->vals==std::vector<int>(8, 12), "[4] stations 1 and 3 all PT100");
    bool bSv0=true;
    for(int s=0; s<3; s++) { const Req* r=FindReq(16, s, 0x000); bSv0=bSv0 && r && r->vals==std::vector<int>(8, 0); }
    CHECK(bSv0, "[4] boot SV write = 0 on all 3 stations (golden would write edSV 30 -> 300)");
    CHECK(CountStation(3)==0, "[4] station 4 is never addressed (golden 32 channels would poll it)");
    SpinUntil(3000, false, []{ return frmDTME08->GetPalGroup(9)->GetPV()==45.6; });
    CHECK(Near(frmDTME08->GetPalGroup(8)->GetPV(), 32.1, 0.051) && Near(frmDTME08->GetPalGroup(9)->GetPV(), 45.6, 0.051), "[4] panels read the PV (Chamber 32.1, Hot Air 1 45.6)");

    // ---- [5] DoThermo ------------------------------------------------------------------------------
    std::printf("[5] DoThermo (hot, USE_16_HEATER 5) through the map\n");
    USE_16_HEATER=eht16HeaterDTME08;                        // the value HT9050 will likely need later (!201 s4); the map still drives every DTM channel
    LastSet.iTemperature=Tempture_Hot;
    Temperature.fWorkTemperBase=80.0;
    Temperature.bUseFixTemp=true;
    Temperature.dFixedTemp=60.0;
    Temperature.bUseIndividualTemp=false;
    CosFunction.bUseIndividulTempSet=false;
    Temperature.iIndexHeatMode=HeadChamberSocket;
    IniConfig.bTemp25degControl=false;
    IniConfig.bEnableKT4HAlarm1=false;
    Tri_Temp_Machine=0;
    SHUTTLE_COOLING=0.0;
    PauseUT150Polling=false;
    Sen[SnHeaterDoor].Enable=false;                          // no I/O in the test: a disabled sensor is never "off" (mysensor.cpp IsOff),
    Sen[SnHeaterDoor2].Enable=false;                         //   so golden case 100 does not take the heater door as open (which zeroes Temp)
    SW[SwHeaterRelay].Enable=true;                           // the heater relay ON: golden case 100 zeroes every SV while it is off
    SW[SwHeaterRelay].ISABase=-1;                            //   (no I/O card: a base none of myswitch.cpp Status()'s routes takes ->
    SW[SwHeaterRelay].Type=1;                                //   Status() returns OutValue)
    SW[SwHeaterRelay].OutValue=true;
    for(int i=0; i<tcTotalCount; i++) { bUT150Install[i]=false; UN150CommError[i]=false; UN150Read[i]=0; UN150ReadReal[i]=0; }
    for(int i=0; i<DtmMap_RowCount(); i++) bUT150Install[DtmMap_Row(i)->iAddr]=true;
    iThermoTask=1;
    Com2ReceiveOK=false;
    COM2->Comm2->SimClearTx();
    const double wantDut=GetFactSetTemp(tcDUT1, 60.0), wantCh=GetFactSetTemp(tcChamber, 80.0);
    std::printf("    want DUT 1 SV %.2f (fixed 60), Chamber SV %.2f (work 80)\n", wantDut, wantCh);
    const bool bSv=SpinUntil(60000, true, [&]{
        return frmDTME08->GetPalGroup(4)->GetSettingSV()==wantDut && frmDTME08->GetPalGroup(8)->GetSettingSV()==wantCh &&
               UN150ReadReal[tcAa1]!=0.0 && UN150ReadReal[tcHeatGun1]!=0.0; });
    CHECK(bSv, "[5] DoThermo reached DUT 1, Chamber, Hot Air 1 and SLK-1");
    if(!bSv) for(int i=0; i<24; i++) std::printf("    panel %d addr %d setting %.2f PV %.2f real %.2f\n", i, DtmMap_AddrOf(i), frmDTME08->GetPalGroup(i)->GetSettingSV(), frmDTME08->GetPalGroup(i)->GetPV(), DtmMap_AddrOf(i)>=0 ? UN150ReadReal[DtmMap_AddrOf(i)] : -1.0);   // on failure: every panel
    CHECK(frmDTME08->GetPalGroup(4)->GetSettingSV()==wantDut && wantDut!=GetFactSetTemp(tcDUT1, 80.0),
          "[5] DUT 1 SV = GetFactSetTemp(tcDUT1, fixed 60): golden's SERIAL-path computation, not the work temperature");
    CHECK(frmDTME08->GetPalGroup(8)->GetSettingSV()==wantCh, "[5] Chamber SV = GetFactSetTemp(tcChamber, 80)");
    const bool bReg=SpinUntil(30000, true, [&]{ return g_srv.reg[0][4]!=0 && g_srv.reg[1][0]!=0; });
    CHECK(bReg && Near(g_srv.reg[0][4]/10.0, wantDut, 0.11) && Near(g_srv.reg[1][0]/10.0, wantCh, 0.11), "[5] the DTM cycle wrote those SVs to the fake DTM (x10)");
    bool bSlk0=true;
    for(int ch=0; ch<8; ch++) bSlk0=bSlk0 && g_srv.reg[2][ch]==0 && frmDTME08->GetPalGroup(16+ch)->GetSettingSV()==0.0;
    CHECK(bSlk0, "[5] SLK-1..8 not armed: station 3 SV stays 0 in hot mode");
    CHECK(Near(UN150ReadReal[tcAa1], 27.8, 0.051), "[5] ... but its PV is read (UN150ReadReal[tcAa1] = 27.8)");
    CHECK(Near(UN150Read[tcChamber], 32.1, 0.051) && Near(UN150ReadReal[tcChamber], 32.1, 0.051), "[5] Chamber PV stored unconverted (golden case 2500)");
    CHECK(Near(UN150ReadReal[tcHeatGun1], 45.6, 0.051) && UN150CommError[tcHeatGun1]==false, "[5] Hot Air 1 PV in UN150ReadReal, no comm error");
    {
        bool bNoSerial=true;
        for(int a : g_dtk.addrs) if(DtmMap_ChannelOf(a)>=0) { bNoSerial=false; std::printf("    serial frame for mapped zone %d\n", a); }
        std::printf("    serial frames %d (uninstalled zones' first-round SV 0), %d zones\n", g_dtk.frames, (int)g_dtk.addrs.size());
        CHECK(bNoSerial, "[5] no zone on the map ever went to the serial port");
        CHECK(g_dtk.frames>0, "[5] the serial path still runs for the zones off the map (golden)");
    }
    {
        const double sv0=frmDTME08->GetPalGroup(0)->GetSettingSV(), sv4=frmDTME08->GetPalGroup(4)->GetSettingSV();
        DoSetSVOfDTME08();
        CHECK(frmDTME08->GetPalGroup(0)->GetSettingSV()==sv0 && frmDTME08->GetPalGroup(4)->GetSettingSV()==sv4,
              "[5] DoSetSVOfDTME08 (golden Index-only, iTempCode) leaves the panels alone while the map is active");
    }

    // ---- [6] the DTM goes away ---------------------------------------------------------------------
    std::printf("[6] fake DTM gone\n");
    g_srv.Stop();
    const bool b999=SpinUntil(20000, true, []{ return UN150Read[tcDUT1]==999 && UN150CommError[tcDUT1]; });
    CHECK(b999, "[6] not connected for more than 5 visits -> UN150Read 999 + UN150CommError (golden case 2500)");

    // ---- [7] source pins ---------------------------------------------------------------------------
    std::printf("[7] source\n");
    {
        const std::string db=Slurp(root+"/database.cpp");
        CHECK(Has(db, "DtmMap_Select(INIFileGeneral->ReadString(\"TempCtrl\", \"DTM_CHANNEL_MAP\", \"\").c_str())"), "[7] database.cpp reads [TempCtrl] DTM_CHANNEL_MAP with ReadString");
        CHECK(!Has(db, "CheckAndReadIniDataGeneral(\"TempCtrl\", \"DTM_CHANNEL_MAP\""), "[7] ... never with CheckAndReadIniDataGeneral (it would seed the key into every machine's Gerneral.ini)");
        const std::string ws=Slurp(root+"/tools/wb_serve.cpp");
        CHECK(Has(ws, "W906_DTME08_CreateFormBoot") && !Has(ws, "W906_DTME08_FormShowArm();"), "[7] wb_serve builds the DTM form but does not arm it yet (!201 s5: arming after the on-site checks)");
    }

    // ---- [8] real files ----------------------------------------------------------------------------
    for(int i=0; i<2; i++) CHECK(Slurp(kReal[i])==before[i], "[8] real machine file unchanged");

    ::WSACleanup();
    std::printf("test_dtm_channel_map: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
