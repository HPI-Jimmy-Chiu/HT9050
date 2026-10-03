// =============================================================================
//  test_dtme08_cycle.cpp  --  AI(W906-I03) 20261002 (Ifor01)
//
//  I-03 第二階段（docs/handoff/TO_IFOR.md I-03）：golden EJ1N/fDTME08.cpp 的 TfrmDTME08（台達 DTM／DTME08，Modbus/TCP）
//  照 golden 開閘後，對一台假的 DTM（本測試自己開的 127.0.0.1 暫用埠，單執行緒、每圈抽一次）跑真的 socket：
//
//    [1] 開機建表單（golden 建構子 :19-27）：USE_16_HEATER 不是 DTME08 ⇒ 什麼都不建；是 eht32HeaterDTME08 ⇒ uDTME08Control、
//        32 個通道面板（CH1..CH32、edSV="30"）、cbCh 32 項；W906_DTME08_CreateFormBoot 第二次呼叫不再多建面板
//    [2] 連不到（192.0.2.1，TEST-NET-1）：每一拍都立刻回來（非阻塞 connect），Module 燈滅，之後 10 s 內不再試（golden :489-507）
//    [3] 接上假 DTM：開機第一輪照 golden :511-544 依序寫 感溫線種類（PT100=12）→ 週期（2 s→20）→ Out2（停用=2）→ SV（edSV 30→300），
//        每種 4 站、每站 8 通道，位址 = 站*0x1000 + 功能碼位址（uDTME08Control.cpp GetFunctionCode）；之後每輪讀 PV／狀態／SV
//        ⇒ 面板 GetPV／GetSV＝假 DTM 的值；PV 0x80xx ⇒ 該通道 Event＝錯誤說明、PV=0；封包逐位元組比對（另寫的編碼）
//    [4] bthermo DoSetSVOfDTME08（G29）：常溫 ⇒ 每個通道 SetSettingSV(i, 0.0)、UN150Read[iTempCode[i]]＝GetConvertTemp(PV)；
//        之後下一輪寫 SV＝0 到 4 站（IsNeedSetSV，golden :580-591）
//    [5] W906_DTME08TimerTick：TimerUpdate 沒開不做事；開了照 VCL 第一次等一個 Interval；InitialOK=false／ATC 6.0 不跑（golden :160-166）；
//        W906_DTME08_FormShowArm：模擬組態不武裝；出貨組態照 golden :10523-10525 的 `A && B || C`（NoHeater＋32CH 也會武裝）
//    [6] 假 DTM 斷線 ⇒ 每拍照樣立刻回來
//    [7] 讀原始碼：bthermo G29 開了；WebBridgeTags.cpp 每拍呼叫 W906_DTME08TimerTick；wb_serve.cpp 在 FileRW_Winway_Boot 之後呼叫
//        W906_DTME08_CreateFormBoot，**而且還沒有呼叫 W906_DTME08_FormShowArm**（FROM_IFOR §1 1002 14:0x：武裝等 Jimmy／I-03b；
//        接上的那一顆 commit 要同時改這一項）（argv[1]＝移植樹根目錄，唯讀）
//    [8] 只動記憶體：D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 前後逐位元組相同；D:\HT9045\EXE\DTME08_Control.ini 只讀（D-035：golden 讀 exe 資料夾，Ifor 1002 照建議）
//        （有沒有這個檔前後一樣）
//  不寫任何檔、不開 COM 埠、不連 127.0.0.1 以外的位址（192.0.2.1 是文件保留位址，永遠連不到）。約 25～30 秒：golden 連線失敗後
//  等 10 秒才重試（GetReconnectTime），照翻，所以 [2]→[3] 要等兩次。
// =============================================================================
#include <winsock2.h>                     // before anything that pulls <windows.h>
#include "MachineType.h"                  // SOFT_SIMULTE, eht*, eATC60, eNewATCSystem
#include "cmydef.h"                       // USE_16_HEATER, InitialOK, TC401HeaterControl, ATC_SYSTEM, iTempCode, UN150Read, bSystemClose
#include "cprod.h"                        // Temperature
#include "LastSet.h"                      // LastSet.iTemperature
#include "bthermo.h"                      // DoSetSVOfDTME08
#include "acarry_shims.h"                 // ATC_InterfaceForm
#include "forms/fDTME08.h"
#include "EJ1N/uDTME08Control.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <functional>

void W906_DTME08_CreateFormBoot();        // forms/fDTME08.cpp EOF
void W906_DTME08_FormShowArm();
namespace ht9045 { void W906_DTME08TimerTick(); }   // forms/fDTME08.cpp EOF (namespace ht9045, as its PumpTick caller)
double GetConvertTemp(int Addr, double T);   // bthermo.cpp:836

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_dtme08_cycle.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p, bool* exists=0)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(exists) *exists=(f!=0);
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static bool Has(const std::string& s, const char* needle) { return s.find(needle)!=std::string::npos; }

// ---------------------------------------------------------------------------
//  The fake DTM: Modbus/TCP server on 127.0.0.1:<ephemeral>, one client, pumped from the test loop (no thread).
//  Registers per station (4 x 0x300 words): +0x000 SV, +0x028 sensor type, +0x0D0 Out2 action, +0x0F8 cycle time,
//  +0x250 AT, +0x268 PV, +0x288 status (the addresses the code under test uses, uDTME08Control.cpp GetFunctionCode).
// ---------------------------------------------------------------------------
struct Req { int fc; int station; int off; int qty; std::vector<int> vals; std::string raw; };
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
        Req q; q.raw=f; q.fc=u[7];
        const int addr=(u[8]<<8)|u[9];
        q.station=addr>>12; q.off=addr&0xFFF; q.qty=(u[10]<<8)|u[11];
        std::string out(f.substr(0, 7));                        // transaction / protocol / length (patched) / unit
        if(q.fc==3 && q.station<4 && q.off+q.qty<=0x300)
        {
            out+=(char)3; out+=(char)(q.qty*2);
            for(int i=0; i<q.qty; i++) { const int v=reg[q.station][q.off+i]&0xFFFF; out+=(char)(v>>8); out+=(char)(v&0xFF); }
        }
        else if(q.fc==16 && q.station<4 && q.off+q.qty<=0x300)
        {
            for(int i=0; i<q.qty; i++) { const int v=(u[13+i*2]<<8)|u[14+i*2]; q.vals.push_back(v); reg[q.station][q.off+i]=v; }
            out+=f.substr(7, 5);                                // fc, address, quantity
        }
        else
            return;                                             // anything else: no answer (the code under test times out)
        out[4]=(char)(((out.size()-6)>>8)&0xFF); out[5]=(char)((out.size()-6)&0xFF);
        reqs.push_back(q);
        ::send(c, out.data(), (int)out.size(), 0);
    }
};

// independent request encoders (Modbus/TCP, not the code under test)
static std::string ReadFrame(int addr, int qty)
{
    const unsigned char b[12]={0,0, 0,0, 0,6, 1, 3, (unsigned char)(addr>>8), (unsigned char)(addr&0xFF), (unsigned char)(qty>>8), (unsigned char)(qty&0xFF)};
    return std::string((const char*)b, 12);
}
static std::string WriteFrame(int addr, const std::vector<int>& v)
{
    std::string s;
    const int len=7+(int)v.size()*2;
    const unsigned char h[13]={0,0, 0,0, (unsigned char)(len>>8), (unsigned char)(len&0xFF), 1, 16,
                               (unsigned char)(addr>>8), (unsigned char)(addr&0xFF), 0, (unsigned char)v.size(), (unsigned char)(v.size()*2)};
    s.assign((const char*)h, 13);
    for(int x : v) { s+=(char)((x>>8)&0xFF); s+=(char)(x&0xFF); }
    return s;
}

static FakeDtm g_srv;
static DWORD g_maxCallMs=0;
static void OneTick()
{
    g_srv.Pump();
    frmDTME08->W906_PollSocket();
    const DWORD t0=::GetTickCount();
    frmDTME08->DoDTME08Cycle();
    const DWORD dt=::GetTickCount()-t0;
    if(dt>g_maxCallMs) g_maxCallMs=dt;
    g_srv.Pump();
}
static bool SpinUntil(DWORD ms, const std::function<bool()>& done)
{
    const DWORD t0=::GetTickCount();
    while(::GetTickCount()-t0<ms)
    {
        OneTick();
        if(done()) return true;
        ::Sleep(2);
    }
    return done();
}
static int CountWrites(int off, int station=-1)
{
    int n=0;
    for(const Req& q : g_srv.reqs) if(q.fc==16 && q.off==off && (station<0 || q.station==station)) n++;
    return n;
}
static const Req* FindReq(int fc, int station, int off)
{
    for(const Req& q : g_srv.reqs) if(q.fc==fc && q.station==station && q.off==off) return &q;
    return 0;
}

int main(int argc, char** argv)
{
    if(!W906TestRequireCtestRedirects("DTME08Cycle"))
        return 2;
    const std::string root=(argc>1)?argv[1]:".";
    std::printf("test_dtme08_cycle (I-03 phase 2): golden TfrmDTME08 against a fake Modbus/TCP DTM\n");

    bool bIniBefore=false;
    const std::string sGen0=Slurp("D:\\HT9045\\system\\Gerneral.ini");
    const std::string sCfg0=Slurp("D:\\HT9045\\config\\config.ini");
    const std::string sDtm0=Slurp("D:\\HT9045\\EXE\\DTME08_Control.ini", &bIniBefore);

    WSADATA wsa;
    CHECK(::WSAStartup(MAKEWORD(2, 2), &wsa)==0, "WSAStartup");
    InitialOK=true;
    bSystemClose=false;
    ATC_SYSTEM=0;
    ATC_InterfaceForm->iATC_MODE_TYPE=0;
    Temperature.bATCActiveCooling=false;

    // ---- [1] boot ------------------------------------------------------------------------------------
    std::printf("[1] boot (golden ctor :19-27 at CreateForm HT9045.cpp:272)\n");
    USE_16_HEATER=eht16HeaterEJ1N;
    frmDTME08->W906_CreateFormBody();
    CHECK(frmDTME08->W906_Control()==NULL, "[1] not a DTME08 machine -> no uDTME08Control");
    USE_16_HEATER=eht32HeaterDTME08;
    W906_DTME08_CreateFormBoot();
    uDTME08Control* ctl=frmDTME08->W906_Control();
    CHECK(ctl!=NULL, "[1] eht32HeaterDTME08 -> uDTME08Control created");
    if(ctl==NULL) { std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail); return 1; }
    CHECK(frmDTME08->GetChannelNumber()==32, "[1] 32 channels (InitialData golden :61-64)");
    CHECK(frmDTME08->cbCh->Items->Count==32 && frmDTME08->cbCh->ItemIndex==0, "[1] cbCh CH1..CH32 (InitialComboList)");
    CHECK(frmDTME08->GetPalGroup(31)!=NULL && frmDTME08->GetPalGroup(31)->iMyTag==31, "[1] panel 32 exists (AddTempGUI 4 stations x 8)");
    CHECK(frmDTME08->GetPalGroup(31)->GroupBox->Caption=="CH32", "[1] panel caption CH32");
    CHECK(frmDTME08->GetPalGroup(5)->edSV->Text=="30", "[1] edSV = 30 (AddSingleView golden :120)");
    CHECK(frmDTME08->GetPalGroup(5)->GetSV()==0.0 && frmDTME08->GetPalGroup(5)->GetSettingSV()==0.0, "[1] dSV / dSettingSV start at 0 (VCL zero-fill)");
    CHECK(frmDTME08->GetPalGroup(32)->iMyTag==0, "[1] index 32 clamps to panel 0 (GetPalGroup golden :138)");
    W906_DTME08_CreateFormBoot();
    CHECK(frmDTME08->GetPalGroup(32)->iMyTag==0, "[1] a second boot call adds no panels");
    CHECK(frmDTME08->TimerUpdate->Enabled==false, "[1] TimerUpdate off after boot (dfm Enabled = False, InitialData :59)");
    {
        // D-035 (Ifor 1002): Reload reads golden's exe folder, D:\HT9045\EXE\DTME08_Control.ini [SocketSetting] -- read here
        // independently with the Win32 profile API (missing file / key -> uSocketBase::InitialData's 127.0.0.1 / 59999)
        char a[256]={0}, p[64]={0};
        ::GetPrivateProfileStringA("SocketSetting", "asAddress", "127.0.0.1", a, sizeof(a), "D:\\HT9045\\EXE\\DTME08_Control.ini");
        ::GetPrivateProfileStringA("SocketSetting", "asPort", "59999", p, sizeof(p), "D:\\HT9045\\EXE\\DTME08_Control.ini");
        std::printf("    D:\\HT9045\\EXE\\DTME08_Control.ini -> %s:%s\n", a, p);
        CHECK(ctl->GetSocketAddress()==AnsiString(a) && ctl->GetSocketPort()==AnsiString(p), "[1] address / port from D:\\HT9045\\EXE\\DTME08_Control.ini (golden exe folder, D-035)");
        CHECK(frmDTME08->edAddress->Text==AnsiString(a) && frmDTME08->edPort->Text==AnsiString(p), "[1] Reload copies them to edAddress / edPort (golden :84-85)");
    }

    // ---- [2] unreachable controller ------------------------------------------------------------------
    std::printf("[2] unreachable address (192.0.2.1): non-blocking\n");
    frmDTME08->W906_UseRealSocket();
    ctl->SetSocketAddress("192.0.2.1");
    ctl->SetSocketPort("502");
    g_maxCallMs=0;
    frmDTME08->ledEJ1N1->Value=true;
    SpinUntil(400, []{ return false; });
    CHECK(g_maxCallMs<1000, "[2] no DoDTME08Cycle call waits for the connect (all < 1 s; a blocking connect waits the OS timeout, ~21 s measured)");
    std::printf("    longest call %lu ms\n", (unsigned long)g_maxCallMs);
    CHECK(frmDTME08->ledEJ1N1->Value==false, "[2] Module lamp off after the failed attempt (golden :498-499)");
    CHECK(g_srv.reqs.empty(), "[2] nothing sent");

    // ---- [3] the fake DTM ----------------------------------------------------------------------------
    std::printf("[3] fake DTM on 127.0.0.1 -- golden waits GetReconnectTime() = 10 s between attempts\n");
    CHECK(g_srv.Start(), "[3] fake DTM listening");
    for(int s=0; s<4; s++)
        for(int ch=0; ch<8; ch++)
        {
            g_srv.reg[s][0x268+ch]=200+s*10+ch;            // PV 20.0 + s + 0.1*ch
            g_srv.reg[s][0x288+ch]=0x04;                   // status: IsCelsius (bit 2)
        }
    g_srv.reg[2][0x268+3]=0x8002;                          // station 3 CH4: sensor broken
    ctl->SetSocketAddress("127.0.0.1");
    ctl->SetSocketPort(AnsiString(g_srv.port));
    g_maxCallMs=0;
    const bool bInit=SpinUntil(30000, []{ return CountWrites(0x000)>=4; });
    CHECK(bInit, "[3] the boot sequence reached the SV writes of all 4 stations");
    CHECK(frmDTME08->ledEJ1N1->Value==true, "[3] Module lamp on once connected");
    CHECK(frmDTME08->btnCtrl->Caption=="Running", "[3] btnCtrl caption Running (golden :481)");
    // order: 4 x sensor, 4 x cycle, 4 x Out2, 4 x SV, stations 0..3 each
    {
        std::vector<int> seq;
        for(const Req& q : g_srv.reqs) if(q.fc==16) seq.push_back(q.off*10+q.station);
        const int want[16]={0x028*10+0,0x028*10+1,0x028*10+2,0x028*10+3, 0x0F8*10+0,0x0F8*10+1,0x0F8*10+2,0x0F8*10+3,
                            0x0D0*10+0,0x0D0*10+1,0x0D0*10+2,0x0D0*10+3, 0x000*10+0,0x000*10+1,0x000*10+2,0x000*10+3};
        bool ok=seq.size()>=16;
        for(int i=0; ok && i<16; i++) ok=(seq[i]==want[i]);
        CHECK(ok, "[3] boot writes in golden order: sensor type, cycle time, Out2, SV -- stations 1..4 each (golden :511-544, :656-668)");
        CHECK(!g_srv.reqs.empty() && g_srv.reqs[0].fc==16, "[3] nothing is read before the boot writes");
    }
    const Req* rs=FindReq(16, 1, 0x028);
    CHECK(rs && rs->raw==WriteFrame(0x1028, std::vector<int>(8, 12)), "[3] frame: station 2 sensor type = PT100 (12) x 8, byte for byte");
    const Req* rc=FindReq(16, 0, 0x0F8);
    CHECK(rc && rc->vals==std::vector<int>(8, 20), "[3] cycle time 2 s -> 20 x 8 (SetCycleTime x10)");
    const Req* ro=FindReq(16, 3, 0x0D0);
    CHECK(ro && ro->vals==std::vector<int>(8, 2), "[3] Out2 control action = ecaDisable (2) x 8");
    const Req* rv=FindReq(16, 2, 0x000);
    CHECK(rv && rv->raw==WriteFrame(0x2000, std::vector<int>(8, 300)), "[3] frame: station 3 SV = edSV 30 -> 300 x 8, byte for byte");
    // the cycle afterwards: PV, status, SV reads
    const bool bRead=SpinUntil(15000, []{
        return FindReq(3, 3, 0x268)!=0 && FindReq(3, 3, 0x288)!=0 && FindReq(3, 3, 0x000)!=0 &&
               frmDTME08->GetPalGroup(31)->GetSV()==30.0; });
    CHECK(bRead, "[3] the cycle reads PV, status and SV of all 4 stations");
    const Req* rp=FindReq(3, 1, 0x268);
    CHECK(rp && rp->raw==ReadFrame(0x1268, 8), "[3] frame: read station 2 PV (fc 3, 0x1268, 8 words), byte for byte");
    bool bPV=true;
    for(int s=0; s<4; s++)
        for(int ch=0; ch<8; ch++)
        {
            if(s==2 && ch==3) continue;
            const double want=(200+s*10+ch)*0.1;
            const double got=frmDTME08->GetPalGroup(s*8+ch)->GetPV();
            if(got<want-0.051 || got>want+0.051) { bPV=false; std::printf("    PV ch%d got %.2f want %.2f\n", s*8+ch, got, want); }
        }
    CHECK(bPV, "[3] every panel's GetPV = the DTM's PV (x0.1, %5.1f)");
    CHECK(frmDTME08->GetPalGroup(19)->GetPV()==0.0, "[3] station 3 CH4 0x8002 -> PV 0 (golden DoGetPV error branch; its Event text goes to the panel's gated paint only)");
    CHECK(frmDTME08->GetPalGroup(0)->GetSV()==30.0 && frmDTME08->GetPalGroup(31)->GetSV()==30.0, "[3] GetSV = 30 read back after the boot write");
    CHECK(frmDTME08->IsNeedSetSV()==true, "[3] IsNeedSetSV true: nobody called SetSettingSV yet (0) while SV reads 30 -- golden then rewrites the edSV values every 10th cycle (:580-585)");
    std::printf("    longest connected call %lu ms (golden SendCommand Sleep(50) + Sleep(5))\n", (unsigned long)g_maxCallMs);
    CHECK(g_maxCallMs<1000, "[3] no connected call blocks (< 1 s; golden sleeps 55 ms per send)");

    // ---- [4] bthermo DoSetSVOfDTME08 (G29) -----------------------------------------------------------
    std::printf("[4] bthermo DoSetSVOfDTME08 (G29): SetSettingSV + read PV\n");
    LastSet.iTemperature=Tempture_Ambient;                 // not hot -> every Temp[Addr]=0.0 (bthermo DoSetSVOfDTME08 else arm)
    for(int i=0; i<INDEX_HEAT_COUNT; i++) UN150Read[iTempCode[i]]=-1.0;
    const size_t nBefore=g_srv.reqs.size();
    DoSetSVOfDTME08();
    CHECK(frmDTME08->GetPalGroup(0)->GetSettingSV()==0.0 && frmDTME08->GetPalGroup(0)->edSV->Text=="0", "[4] SetSettingSV(0, 0.0): SV 30 != 0 -> panel setting 0, edSV 0 (golden :452-460)");
    CHECK(UN150Read[iTempCode[1]]==GetConvertTemp(iTempCode[1], frmDTME08->GetPalGroup(1)->GetPV()), "[4] UN150Read[iTempCode[1]] = GetConvertTemp(panel 2 PV) (golden bthermo :4776)");
    CHECK(UN150Read[iTempCode[1]]!=-1.0, "[4] UN150Read written");
    CHECK(frmDTME08->IsNeedSetSV()==true, "[4] IsNeedSetSV: setting 0 != SV 30");
    CHECK(g_srv.reqs.size()==nBefore, "[4] DoSetSVOfDTME08 itself sends nothing (the cycle does)");
    const bool bSV0=SpinUntil(15000, []{ return g_srv.reg[0][0]==0 && g_srv.reg[3][7]==0; });
    CHECK(bSV0, "[4] the cycle wrote SV 0 to all 4 stations (IsNeedSetSV -> DoSetSV, golden :580-585)");
    const bool bBack=SpinUntil(15000, []{ return frmDTME08->GetPalGroup(31)->GetSV()==0.0; });
    CHECK(bBack && frmDTME08->IsNeedSetSV()==false, "[4] SV read back 0 -> IsNeedSetSV false");

    // ---- [5] the timer and the arm -------------------------------------------------------------------
    std::printf("[5] W906_DTME08TimerTick / W906_DTME08_FormShowArm\n");
    frmDTME08->TimerUpdate->Enabled=false;
    frmDTME08->btnCtrl->Caption="X";
    ht9045::W906_DTME08TimerTick(); ::Sleep(150); ht9045::W906_DTME08TimerTick();
    CHECK(frmDTME08->btnCtrl->Caption=="X", "[5] TimerUpdate off -> the tick does nothing");
    frmDTME08->TimerUpdate->Enabled=true;
    ht9045::W906_DTME08TimerTick();
    CHECK(frmDTME08->btnCtrl->Caption=="X", "[5] first tick after enable waits one Interval (VCL)");
    InitialOK=false;
    ::Sleep(150); ht9045::W906_DTME08TimerTick();
    CHECK(frmDTME08->btnCtrl->Caption=="X", "[5] InitialOK=false -> TimerUpdateTimer returns (golden :160)");
    InitialOK=true; ATC_SYSTEM=eATC60;
    ::Sleep(150); ht9045::W906_DTME08TimerTick();
    CHECK(frmDTME08->btnCtrl->Caption=="X", "[5] ATC 6.0 -> TimerUpdateTimer returns (golden :163-166)");
    ATC_SYSTEM=0;
    ::Sleep(150); ht9045::W906_DTME08TimerTick();
    CHECK(frmDTME08->btnCtrl->Caption=="Running", "[5] due tick runs DoDTME08Cycle");
    frmDTME08->TimerUpdate->Enabled=false;
    TC401HeaterControl=NoHeater; USE_16_HEATER=eht32HeaterDTME08;
    W906_DTME08_FormShowArm();
#ifdef SOFT_SIMULTE
    CHECK(frmDTME08->TimerUpdate->Enabled==false, "[5] SIM build: the arm does nothing (golden #ifndef SOFT_SIMULTE :10528)");
#else
    CHECK(frmDTME08->TimerUpdate->Enabled==true, "[5] ship build: NoHeater + 32CH still arms (golden `A && B || C`, :10523-10525)");
    frmDTME08->TimerUpdate->Enabled=false;
    USE_16_HEATER=eht16HeaterDTME08;
    W906_DTME08_FormShowArm();
    CHECK(frmDTME08->TimerUpdate->Enabled==false, "[5] ship build: NoHeater + 16CH does not arm");
    frmDTME08->TimerUpdate->Enabled=false;
    USE_16_HEATER=eht32HeaterDTME08;
#endif

    // ---- [6] the DTM goes away -----------------------------------------------------------------------
    std::printf("[6] fake DTM closes\n");
    g_srv.Stop();
    g_maxCallMs=0;
    SpinUntil(500, []{ return false; });
    CHECK(g_maxCallMs<1000, "[6] after the DTM closes no call waits (< 1 s)");

    // ---- [7] source pins -----------------------------------------------------------------------------
    std::printf("[7] source pins (%s)\n", root.c_str());
    const std::string sTh=Slurp(root+"/bthermo.cpp");
    CHECK(Has(sTh, "#if 1 // was: #if 0 // TODO(W7-UI G29)"), "[7] bthermo.cpp G29 open");
    const std::string sWb=Slurp(root+"/WebBridgeTags.cpp");
    CHECK(Has(sWb, "W906_DTME08TimerTick(); }"), "[7] WebBridgeTags.cpp PumpTick calls W906_DTME08TimerTick");
    const std::string sWs=Slurp(root+"/tools/wb_serve.cpp");
    const size_t pW=sWs.find("FileRW_Winway_Boot(); }"), pD=sWs.find("W906_DTME08_CreateFormBoot(); }");
    CHECK(pW!=std::string::npos && pD!=std::string::npos && pD>pW, "[7] wb_serve boots the form after FileRW_Winway_Boot (golden CreateForm order HT9045.cpp:271-272)");
    CHECK(!Has(sWs, "W906_DTME08_FormShowArm();"), "[7] NOT ARMED: wb_serve does not call W906_DTME08_FormShowArm yet (the commit that arms it changes this line)");
    const std::string sF=Slurp(root+"/forms/fDTME08.cpp");
    CHECK(!Has(sF, "\n#if 0 // GATE (E-"), "[7] forms/fDTME08.cpp: no gate left closed");
    const std::string sCtlH=Slurp(root+"/EJ1N/uDTME08Control.h");
    CHECK(Has(sCtlH, "return \"D:\\\\HT9045\\\\EXE\\\\\";"), "[7] EJ1N/uDTME08Control.h GetSettingFilePath = golden's exe folder D:\\HT9045\\EXE\\ (D-035)");

    // ---- [8] machine files ---------------------------------------------------------------------------
    bool bIniAfter=false;
    const std::string sDtm1=Slurp("D:\\HT9045\\EXE\\DTME08_Control.ini", &bIniAfter);
    CHECK(Slurp("D:\\HT9045\\system\\Gerneral.ini")==sGen0, "[8] Gerneral.ini unchanged");
    CHECK(Slurp("D:\\HT9045\\config\\config.ini")==sCfg0, "[8] config.ini unchanged");
    CHECK(bIniAfter==bIniBefore && sDtm1==sDtm0, "[8] D:\\HT9045\\EXE\\DTME08_Control.ini only read (existence and bytes unchanged)");

    ::WSACleanup();
    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
