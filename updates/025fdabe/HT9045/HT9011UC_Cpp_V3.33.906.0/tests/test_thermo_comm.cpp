// =============================================================================
//  test_thermo_comm.cpp  --  AI(W906-I03) 20261002 (Ifor01)
//
//  I-03 第一階段（docs/handoff/TO_IFOR.md I-03）：golden TCOM2 的溫控器序列埠 Comm2（rs232.cpp 檔尾）＋cpublic.cpp 的
//  KT4H／TMC401／E5DC 封包＋bthermo.cpp DoThermoReal 走這條線。Comm2 強制 SIM 模式（vclcompat/Comm.h SetSimMode），
//  不開任何 COM 埠；不呼叫 RS232Init／W906_CreateFormBoot（會探測真的 COM 埠、讀寫 Gerneral.ini）。
//
//    [1] 封包逐位元組比對（另寫一份編碼器，不呼叫被測的 helper）：KT4H 讀／寫（Modbus ASCII＋LRC，golden cpublic.cpp:177-200）、
//        TMC401 讀／寫（Modbus RTU＋CRC16，:423-460）、E5DC 讀／寫（CompoWay/F：STX＋命令＋ETX＋BCC＋CRLF，:462-487）、
//        DTK4848 讀（g_pDTKComm 從建構子起就是 COM2->Comm2，:217-228）
//    [2] 收件：位元組分兩次、間隔 20 ms 到達 ⇒ 安靜不滿 100 ms（rs232.dfm:30）不交付；滿了才整包交給 Comm2ReceiveData
//        （golden rs232.cpp:721-757）：KT4H ':' 開頭 → Com2Buffer＋Com2ReceiveOK；少於 7 個位元組丟掉；不是 ':' 開頭丟掉；
//        InitialOK=false 丟掉；TC401 → READBUFF[0..7]
//    [3] DoThermo 全程（KT4H、只裝 tcHotPlate1）：假溫控器回話（寫入 06 原樣回、讀取 03 回 100.0 度）⇒ UN150ReadReal==100.0、
//        UN150CommError==false
//    [4] 同一個通道、溫控器不回：每一次 DoThermo 的最長耗時（I-03 卡要的「一拍最多卡多久」）、多久變 999／UN150CommError
//    [5] 讀原始碼：DoThermo 在 DoThermoReal 之前交付 Comm2；RS232Init 呼叫兩支溫控 helper；三個檔的閘都拿掉了（argv[1]＝原始碼根目錄）
//    [6] 只動記憶體：D:\HT9045\system\Gerneral.ini 與 D:\HT9045\config\config.ini 前後逐位元組相同
// =============================================================================
#include "atester_shims.h"
#include "cmydef.h"
#include "cpublic.h"
#include "bthermo.h"
#include "MachineType.h"
#include "LastSet.h"
#include "Config.h"
#include "cprod.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

extern Spcomm::TComm* g_pCOM2Comm2;                 // cpublic.cpp（golden COM2->Comm2 的縫）
void W906_PumpTempComm2();                          // rs232.cpp 檔尾

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; } else { g_fail++; std::printf("  FAIL [test_thermo_comm.cpp:%d] %s\n", __LINE__, msg); } } while(0)

static std::string Slurp(const std::string& p)
{
    FILE* f=std::fopen(p.c_str(), "rb");
    if(!f) return std::string("<missing>");
    std::string s; char buf[4096]; size_t n;
    while((n=std::fread(buf, 1, sizeof(buf), f))>0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static std::string TakeTx()
{
    const std::vector<char>& v=COM2->Comm2->SimTxBuffer();
    std::string out(v.begin(), v.end());
    COM2->Comm2->SimClearTx();
    return out;
}

static void Inject(const std::string& s) { COM2->Comm2->SimInjectReceive(s.data(), (Spcomm::Word)s.size()); }

// ---- independent encoders (the protocols, not the code under test) ----
static int HexVal(char c) { return (c>='0' && c<='9') ? c-'0' : (c>='A' && c<='F') ? c-'A'+10 : (c>='a' && c<='f') ? c-'a'+10 : 0; }
static std::string Hex2(unsigned v) { char b[8]; std::snprintf(b, sizeof(b), "%02X", v&0xFF); return b; }
static std::string Hex4(unsigned v) { char b[8]; std::snprintf(b, sizeof(b), "%04X", v&0xFFFF); return b; }
static std::string Lrc(const std::string& hexBody)              // Modbus ASCII: two's complement of the byte sum
{
    unsigned char sum=0;
    for(size_t i=0; i+1<hexBody.size(); i+=2) sum=(unsigned char)(sum+HexVal(hexBody[i])*16+HexVal(hexBody[i+1]));
    return Hex2((unsigned char)(0x100-sum));
}
static std::string AsciiFrame(const std::string& hexBody) { return ":"+hexBody+Lrc(hexBody)+"\r\n"; }
static unsigned Crc16(const std::string& b)                     // Modbus RTU CRC-16 (0xA001, init 0xFFFF)
{
    unsigned crc=0xFFFF;
    for(unsigned char c : b) { crc^=c; for(int k=0; k<8; k++) crc=(crc&1) ? (crc>>1)^0xA001 : crc>>1; }
    return crc;
}
static std::string RtuFrame(std::initializer_list<unsigned char> l)
{
    std::string b(l.begin(), l.end());
    const unsigned c=Crc16(b);
    b.push_back((char)(c&0xFF)); b.push_back((char)(c>>8));
    return b;
}
static std::string CompoWay(const std::string& cmd)             // STX cmd ETX BCC CR LF; BCC = XOR(cmd) ^ ETX
{
    unsigned char bcc=0;
    for(unsigned char c : cmd) bcc^=c;
    bcc^=0x03;
    return std::string(1, '\x02')+cmd+std::string(1, '\x03')+std::string(1, (char)bcc)+"\r\n";
}

// ---- a fake KT4H on the line: write (06) -> echo, read (03) -> one word ----
struct FakeKT4H {
    std::string acc;
    double pv;
    int writes, reads;
    FakeKT4H() : pv(100.0), writes(0), reads(0) {}
    void Serve()
    {
        acc+=TakeTx();
        for(;;)
        {
            const size_t a=acc.find(':');
            if(a==std::string::npos) { acc.clear(); return; }
            const size_t e=acc.find("\r\n", a);
            if(e==std::string::npos) { acc.erase(0, a); return; }
            const std::string f=acc.substr(a, e+2-a);
            acc.erase(0, e+2);
            if(f.size()<17) continue;
            const std::string addr=f.substr(1, 2), func=f.substr(3, 2);
            if(func=="06") { writes++; Inject(f); }
            else if(func=="03") { reads++; Inject(AsciiFrame(addr+"0302"+Hex4((unsigned)(pv*10.0+0.5)))); }
        }
    }
};

static double MaxCallMs(double cur, LARGE_INTEGER t0, LARGE_INTEGER t1, LARGE_INTEGER fq)
{
    const double ms=(double)(t1.QuadPart-t0.QuadPart)*1000.0/(double)fq.QuadPart;
    return ms>cur ? ms : cur;
}

int main(int argc, char** argv)
{
    static const char* const kReal[2]={ "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini" };
    std::string before[2];
    for(int i=0; i<2; i++) before[i]=Slurp(kReal[i]);

    COM2->Comm2->SetSimMode(true);
    COM2->Comm2->StartComm();

    std::printf("[1] frames on Comm2 (independent encoders)\n");
    CHECK(g_pCOM2Comm2==COM2->Comm2, "g_pCOM2Comm2 == COM2->Comm2 from the constructor (golden COM2->Comm2)");
    CHECK(g_pDTKComm==COM2->Comm2, "g_pDTKComm == COM2->Comm2 from the constructor (golden DTK4848 writes COM2->Comm2)");
    TakeTx();
    UT100WordReadNoSucm(0, 0x0080);
    CHECK(TakeTx()==AsciiFrame("010300800001"), "KT4H read: ':' 01 03 0080 0001 LRC CR LF");
    UT100WordWriteNoSucm(4, 0x0001, 1250);
    CHECK(TakeTx()==AsciiFrame("05060001"+Hex4(1250)), "KT4H write: ':' 05 06 0001 04E2 LRC CR LF");
    TMC401ReadTemp(1, 2);
    CHECK(TakeTx()==RtuFrame({0x02, 0x03, 0x00, 0x02, 0x00, 0x01}), "TMC401 read: 02 03 0002 0001 CRC16");
    TMC401WriteTemp(0, 1, 1234);
    CHECK(TakeTx()==RtuFrame({0x01, 0x06, 0x00, 0xC9, 0x04, 0xD2}), "TMC401 write: 01 06 00C9 04D2 CRC16");
    E5DCReadTemp(0);
    CHECK(TakeTx()==CompoWay("010000101C00000000002"), "E5DC read: STX 01 000 0101 C0 0000 00 0002 ETX BCC CR LF");
    E5DCWriteTemp(2, 0x1F4);
    CHECK(TakeTx()==CompoWay("030000102C100030000010000"+Hex4(0x1F4)), "E5DC write: STX 03 000 0102 C1 0003 00 0001 0000 01F4 ETX BCC CR LF");
    {
        DTK4848WordReadNoSucm(0);
        const std::string t=TakeTx();
        CHECK(t.size()>2 && t[0]==':' && t.compare(t.size()-3, 3, std::string("\r\n\0", 3))==0, "DTK4848 read goes out on Comm2 (golden strlen+1: the NUL too)");
    }

    std::printf("[2] receive: 100 ms quiet, then golden Comm2ReceiveData\n");
    const bool savedInit=InitialOK;
    const int savedCtl=TC401HeaterControl;
    InitialOK=true;
    TC401HeaterControl=KT4H;
    {
        const std::string r=AsciiFrame("01030203E8");
        Com2ReceiveOK=false; Com2Buffer="";
        Inject(r.substr(0, 6));
        ::Sleep(20);
        Inject(r.substr(6));
        W906_PumpTempComm2();
        CHECK(Com2ReceiveOK==false, "two pieces 20 ms apart: not delivered before 100 ms of quiet (rs232.dfm:30)");
        ::Sleep(130);
        W906_PumpTempComm2();
        CHECK(Com2ReceiveOK==true, "after 100 ms of quiet: one frame delivered");
        CHECK(std::string(Com2Buffer.c_str())==r, "Com2Buffer == the whole frame (golden sprintf(\"%s\") of the packet)");

        Com2ReceiveOK=false;
        Inject(":0103");  ::Sleep(130);  W906_PumpTempComm2();
        CHECK(Com2ReceiveOK==false, "fewer than 7 bytes: dropped (golden :727-728)");
        Inject("X0103020064AB\r\n");  ::Sleep(130);  W906_PumpTempComm2();
        CHECK(Com2ReceiveOK==false, "KT4H frame not starting with ':': dropped (golden :747-750)");
        InitialOK=false;
        Inject(r);  ::Sleep(130);  W906_PumpTempComm2();
        CHECK(Com2ReceiveOK==false, "InitialOK==false: dropped (golden :724-725)");
        InitialOK=true;

        TC401HeaterControl=TC401;
        const std::string rtu=RtuFrame({0x01, 0x03, 0x02, 0x03, 0xE8});
        Inject(rtu);  ::Sleep(130);  W906_PumpTempComm2();
        CHECK(Com2ReceiveOK==true && READBUFF[1]==0x03 && READBUFF[3]==0x03 && READBUFF[4]==0xE8, "TC401: READBUFF[0..7] = the RTU reply (golden :736-740)");
        TC401HeaterControl=KT4H;
    }

    std::printf("[3] DoThermo end to end: KT4H, tcHotPlate1, a fake controller answering\n");
    const int savedTemp=LastSet.iTemperature;
    const bool savedAl1=IniConfig.bEnableKT4HAlarm1;
    const int savedAtc=ATC_SYSTEM, savedTri=Tri_Temp_Machine;
    LastSet.iTemperature=Tempture_Ambient;
    IniConfig.bEnableKT4HAlarm1=false;
    ATC_SYSTEM=0;
    Tri_Temp_Machine=0;
    for(int i=0; i<tcTotalCount; i++) { bUT150Install[i]=false; UN150CommError[i]=false; UN150Read[i]=0; UN150ReadReal[i]=0; }
    bUT150Install[tcHotPlate1]=true;
    iThermoTask=1;
    Com2ReceiveOK=false;
    TakeTx();
    {
        FakeKT4H dev;
        const DWORD t0=::GetTickCount();
        while(::GetTickCount()-t0<15000 && UN150ReadReal[tcHotPlate1]==0)
        {
            DoThermo();
            dev.Serve();
            ::Sleep(0);                                                 // yield only: golden's timers are wall-clock (Com2Delay 0.5 s), and the state machine visits ~75 channels per round -- a 2 ms sleep is ~14 ms on Windows and stretches a round past 2 s
        }
        std::printf("     writes answered %d, reads answered %d, %lu ms\n", dev.writes, dev.reads, (unsigned long)(::GetTickCount()-t0));
        CHECK(dev.writes>=1, "the SV write (function 06) went out first (OldTemp starts at -1, golden case 1)");
        CHECK(dev.reads>=1, "a PV read (function 03, register 0080) went out");
        CHECK(UN150ReadReal[tcHotPlate1]==100.0, "UN150ReadReal[tcHotPlate1] == 100.0 (golden case 250: A_Get_MEM_Word()/10.0)");
        CHECK(UN150CommError[tcHotPlate1]==false, "UN150CommError[tcHotPlate1] == false");
    }

    std::printf("[4] the same channel, no controller: longest DoThermo call, time to 999\n");
    {
        // continue from [3]: every channel's SV is already written (OldTemp == Temp), so only tcHotPlate1's PV read goes out.
        // Restarting at case 1 would write SV again to every channel (golden case 100 :1489-1491 sends 0 to the uninstalled
        // ones) and each unanswered write waits 0.5 s -- a ~35 s round with no controller on the line, golden's own pace.
        UN150Read[tcHotPlate1]=0;
        LARGE_INTEGER fq, a, b;
        ::QueryPerformanceFrequency(&fq);
        double maxMs=0.0;
        long calls=0;
        const DWORD t0=::GetTickCount();
        while(::GetTickCount()-t0<30000 && !(UN150Read[tcHotPlate1]==999 && UN150CommError[tcHotPlate1]))
        {
            ::QueryPerformanceCounter(&a);
            DoThermo();
            ::QueryPerformanceCounter(&b);
            maxMs=MaxCallMs(maxMs, a, b, fq);
            calls++;
            TakeTx();
            ::Sleep(0);                                                 // yield only: golden's timers are wall-clock (Com2Delay 0.5 s), and the state machine visits ~75 channels per round -- a 2 ms sleep is ~14 ms on Windows and stretches a round past 2 s
        }
        const DWORD took=::GetTickCount()-t0;
        std::printf("     %ld DoThermo calls, longest %.3f ms; UN150Read=999 after %lu ms\n", calls, maxMs, (unsigned long)took);
        CHECK(UN150Read[tcHotPlate1]==999 && UN150CommError[tcHotPlate1], "no reply: UN150Read = 999 and UN150CommError (golden: 2 retries, then CommunCTErr > 5)");
        CHECK(maxMs<50.0, "no DoThermo call blocks (waiting is Com2Delay 0.5 s polled per call, writes go to the writer thread)");
        CHECK(took>=6000 && took<=20000, "time to 999 is the golden timer sum (8 x 0.5 s reads + 6 x 0.5 s case 230 ~ 7 s)");
    }
    LastSet.iTemperature=savedTemp;
    IniConfig.bEnableKT4HAlarm1=savedAl1;
    ATC_SYSTEM=savedAtc;
    Tri_Temp_Machine=savedTri;
    TC401HeaterControl=savedCtl;
    InitialOK=savedInit;

    std::printf("[5] source\n");
    if(argc>1)
    {
        const std::string root(argv[1]);
        const std::string bt=Slurp(root+"/bthermo.cpp");
        const size_t dt=bt.find("\nvoid DoThermo()");
        const size_t pump=bt.find("W906_PumpTempComm2();", dt==std::string::npos ? 0 : dt);
        const size_t real=bt.find("DoThermoReal();", dt==std::string::npos ? 0 : dt);
        CHECK(dt!=std::string::npos && pump!=std::string::npos && real!=std::string::npos && pump<real, "DoThermo pumps Comm2 before DoThermoReal");
        CHECK(bt.find("#if 0 // TODO(W5 G17)")==std::string::npos && bt.find("#if 0 // TODO(W5 G21")==std::string::npos &&
              bt.find("#if 0 // TODO(W5 G22)")==std::string::npos && bt.find("#if 0 // TODO(W5 G23")==std::string::npos &&
              bt.find("#if 0 // TODO(W7 G20)")==std::string::npos && bt.find("#if 0 // TODO(W7-UI G14a)")==std::string::npos,
              "bthermo.cpp: G14a / G17 / G20 / G21-G23 are open");
        const std::string rs=Slurp(root+"/rs232.cpp");
        const size_t ri=rs.find("void TCOM2Shim::RS232Init()");
        const size_t ck=rs.find("W906_RS232InitTempCheck(flag);", ri==std::string::npos ? 0 : ri);
        const size_t op=rs.find("W906_RS232InitTempOpen(flag);", ri==std::string::npos ? 0 : ri);
        CHECK(ri!=std::string::npos && ck!=std::string::npos && op!=std::string::npos && ck<op, "RS232Init calls the temperature check, then the open (golden :163-182, :264-281)");
        const std::string cp=Slurp(root+"/cpublic.cpp");
        CHECK(cp.find("\n    COM2->Comm2->WriteCommData(")==std::string::npos, "cpublic.cpp: no direct COM2->Comm2 write left (all through g_pCOM2Comm2)");
    }
    else
        CHECK(false, "argv[1] = source root (tests/CMakeLists.txt passes it)");

    COM2->Comm2->StopComm();

    std::printf("[6] memory only: real files unchanged\n");
    for(int i=0; i<2; i++)
    {
        const std::string after=Slurp(kReal[i]);
        CHECK(after==before[i], kReal[i]);
    }

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_pass, g_pass+g_fail);
    return g_fail ? 1 : 0;
}
