// =============================================================================
//  test_rs232_torque.cpp  --  AI(W906-R28TORQ) 20260925
//
//  鎖住 rs232.cpp（golden TCOM2 的 Index Z 扭力那一半）照 golden 走的 RS232 交握：
//    [A] 兩個組態都跑：WriteIndexTorqueSetting_Pana／ReadIndexTorqueSetting_Pana ＋ ReadWriterParameter_Panasonic
//        （golden rs232.cpp:1368／:1414／:1523，都不看 SOFT_SIMULTE）—— 送出的位元組逐一比對手算的 golden 封包，
//        讀回值寫進 fMain->edtReadZ1（golden rs232.cpp:1818-1821）。
//    [F] SPComm「100 ms 安靜才交付」的語意（rs232.cpp 檔頭 (1)）：ACK 與 ENQ 分兩次、間隔 20 ms 到達，
//        仍要當成**一包**交給 Comm1ReceiveData（golden 用 data[0]/data[1] 判 ACK+ENQ，rs232.cpp:1596）。
//    [B] 只在出貨組態：iWriteAndCheckMotorTorque(0,300)（golden rs232.cpp:1847-2009）完整跑完 → 回 1；
//        伺服器讀回錯的值 → 重試 5 次後回 2（golden :1952-1961）。模擬組態照 golden 的 SOFT_SIMULTE 臂立刻回 1。
//    [C] 只在出貨組態：ReadTorque_Panasonic（golden :820-1000）讀扭力回饋 → edTorue0 == "15.00"、autoTask==999、
//        chkReadTorque1 被清掉（golden :992-994）。
//
//  假伺服器照 Panasonic A5 的協定回話（封包值取自 golden 本身的組字，見各 CHECK 的註解）。
//  Comm1 強制 SIM 模式（vclcompat/Comm.h SetSimMode），不開任何 COM 埠；不讀寫任何機台檔
//  （不呼叫 W906_CreateFormBoot／RS232Init —— 前者會讀寫 Gerneral.ini，後者會探測真的 COM 埠）。
// =============================================================================
#include "atester_shims.h"
#include "cmydef.h"
#include "FormsFacade.h"
#include "MachineType.h"
#include "Motor/mymotor.h"                // MOT[]（AI(W906-TORQUE-1203) 20260925：HT9050 段看 MTestZ2 有沒有啟用）
#include "database.h"                     // HSys.sTorqueComPort（AI(W906-TORQUE-1203) 20260926：[H4] RS232Init）
#include <windows.h>
#include <cstdio>
#include <vector>

static int g_pass=0, g_fail=0;
#define CHECK(c, msg) do { if(c){ g_pass++; std::printf("  PASS %s\n", msg); } \
                           else  { g_fail++; std::printf("  FAIL %s\n", msg); } } while(0)

static std::vector<unsigned char> TakeTx()
{
    const std::vector<char>& v = COM2->Comm1->SimTxBuffer();
    std::vector<unsigned char> out(v.begin(), v.end());
    COM2->Comm1->SimClearTx();
    return out;
}

static void Reply(const std::vector<unsigned char>& r)
{
    COM2->Comm1->SimInjectReceive(&r[0], (Spcomm::Word)r.size());   // -> OnReceiveData -> W906_Comm1QueueRx
    ::Sleep(130);                                                   // > rs232.dfm Comm1 ReadIntervalTimeout 100 ms
}

static std::vector<unsigned char> V(std::initializer_list<unsigned char> l) { return std::vector<unsigned char>(l); }

static unsigned char Sum2(const std::vector<unsigned char>& f, size_t n)   // golden: sum=~sum+1 over the first n bytes
{
    unsigned char s=0;
    for(size_t i=0; i<n; i++) s+=f[i];
    s=~s+1;
    return s;
}

// ---------------------------------------------------------------------------
//  Fake Panasonic A5 servo.  Reacts to what the handler just wrote.
// ---------------------------------------------------------------------------
struct FakeServo {
    enum Req { NONE, WRITE, READ_PARAM, READ_TORQUE } pending;
    int  stored;          // value the drive holds after a write
    int  readBackBias;    // added to the read-back (to force a mismatch)
    int  torqueRaw;       // raw feedback torque (k); handler shows k/20
    int  writes, paramReads, torqueReads;
    std::vector<std::vector<unsigned char> > log;
    FakeServo(): pending(NONE), stored(0), readBackBias(0), torqueRaw(300), writes(0), paramReads(0), torqueReads(0) {}

    // returns the reply to inject (empty = none)
    std::vector<unsigned char> OnTx(const std::vector<unsigned char>& tx)
    {
        log.push_back(tx);
        if(tx.size()==1 && tx[0]==0x05) return V({0x04});                     // ENQ -> EOT
        if(tx.size()==10 && tx[0]==0x06 && tx[2]==0x17)                        // A5 write frame (golden :1426-1441)
        { pending=WRITE; stored=tx[5]+tx[6]*256; writes++; return V({0x06,0x05}); }
        if(tx.size()==6 && tx[0]==0x02 && tx[2]==0x07)                         // A5 read-setting frame (golden :1377-1389)
        { pending=READ_PARAM; paramReads++; return V({0x06,0x05}); }
        if(tx.size()==4 && tx[0]==0x00 && tx[2]==0x52)                         // torque-feedback request (golden :833, :909-911)
        { pending=READ_TORQUE; torqueReads++; return V({0x06,0x05}); }
        if(tx.size()==1 && tx[0]==0x04)                                        // EOT: "send me your data"
        {
            Req r=pending; pending=NONE;
            if(r==WRITE)      return V({0x01,0x00,0x97,0x00});                 // golden :1630 A5 write ack: iPanasonicNum==10 && ptreot==0x01
            if(r==READ_PARAM) { int v=stored+readBackBias;                      // golden :1624 iPanasonicNum==6 && ptreot==0x05; :1801-1806 A5 9 bytes; k=str[4]<<8|str[3]
                                return V({0x05,0x00,0x87,(unsigned char)(v%256),(unsigned char)(v/256),0x00,0x00,0x00,0x00}); }
            if(r==READ_TORQUE) return V({0x03,0x00,0xD2,(unsigned char)(torqueRaw%256),(unsigned char)(torqueRaw/256),0x00,0x00}); // golden :967 ptreot==0x03; 7 bytes
        }
        return std::vector<unsigned char>();                                   // ACK etc.
    }
};

// One MainProc-shaped beat: golden csystem.cpp:16848/:16860 call these two every tick.
static void Beat()
{
    COM2->ReadTorque();
    COM2->ReadWriterParameter();
}

// Run beats, answering as the fake servo, until pred() or timeout.
template<class P>
static bool RunUntil(FakeServo& s, P pred, int timeoutMs)
{
    DWORD t0=::GetTickCount();
    while((int)(::GetTickCount()-t0) < timeoutMs)
    {
        if(pred()) return true;
        Beat();
        std::vector<unsigned char> tx=TakeTx();
        if(!tx.empty())
        {
            std::vector<unsigned char> r=s.OnTx(tx);
            if(!r.empty()) Reply(r);
        }
        else
            ::Sleep(5);
    }
    return pred();
}

// AI(W906-TORQUE-1203) 20260925: HT9050 1203 扭力掛鉤的假後端
static int g_hCalls = 0, g_hOkAt = 0, g_hMi = -1, g_hVal = -1;
static int FakeTorqueHook(int mi, int v, AnsiString* why)
{
    ++g_hCalls; g_hMi = mi; g_hVal = v;
    if (g_hCalls >= g_hOkAt) return 1;
    if (why) *why = "fake: read-back mismatch";
    return 0;
}

int main()
{
    std::printf("[rs232_torque] %s configuration\n",
#ifdef SOFT_SIMULTE
        "SIM (SOFT_SIMULTE)"
#else
        "SHIP (no SOFT_SIMULTE)"
#endif
    );

    InitialOK=true;                                   // golden Comm1ReceiveData :1753 guard
    INDEX_DRIVER_TYPE=Panasonic_DRIVER;               // what W906_CreateFormBoot leaves for an A5 machine (golden :103-111)
    iPanasonicDriverType=Panasonic_DRIVER_A5;
    TorqueUseHPComCard=false;
    COM2->rwCommandDelay=5;                           // golden :119
    {   // AI(W906-TORQUE-1203) 20260926: [H4] NB2 R46／R53 —— HT9050 的 RS232Init 不探測、不開扭力埠（它的扭力走 1203）  AI(W906-HT9050-AS-LS) 20260926：判準改成 IO_CARD_TYPE==PCI1203_IO
        const int oldIo = IO_CARD_TYPE;
        IO_CARD_TYPE = PCI1203_IO;           // AI(W906-HT9050-AS-LS) 20260926：HT9050＝1203 IO 卡（原本設 MachineTypeChoice=Type_HT9050）
        HSys.sTorqueComPort = "COM97";                  // 不存在的埠：若還會探測／開啟，就會跳 port error 或開成功
        COM2->RS232Init();
        CHECK(!COM2->Comm1->IsOpen(), "[H4] 1203 IO card (HT9050): RS232Init leaves the torque COM port closed (no probe, no StartComm)");
        IO_CARD_TYPE = oldIo;
    }
    COM2->Comm1->SetSimMode(true);
    COM2->Comm1->StartComm();
    CHECK(COM2->Comm1->IsOpen() && COM2->Comm1->IsSimMode(), "Comm1 opened in SIM mode (no COM port touched)");

    // ---------------- [A] write setting 300 to Z1 (golden WriteIndexTorqueSetting_Pana A5) ----------------
    {
        FakeServo s;
        COM2->WriteIndexTorqueSetting(0, "300");
        CHECK(COM2->fPanasonicParameterRW==true, "[A] WriteIndexTorqueSetting sets fPanasonicParameterRW (golden :1459)");
        bool done=RunUntil(s, []{ return COM2->fPanasonicParameterRW==false; }, 8000);
        CHECK(done, "[A] write handshake completes (ENQ/EOT/data/ACK+ENQ/EOT/reply/ACK, golden :1523-1681)");
        CHECK(s.writes==1 && s.stored==300, "[A] drive received torque 300");
        // golden frame: {06 00 17 00 0D lo hi 00 00 sum}, lo/hi = 300%256 / 300/256 = 0x2C / 0x01
        bool frameOk=false;
        for(size_t i=0;i<s.log.size();i++)
        {
            const std::vector<unsigned char>& f=s.log[i];
            if(f.size()==10)
                frameOk = f[0]==0x06 && f[1]==0x00 && f[2]==0x17 && f[3]==0x00 && f[4]==0x0D &&
                          f[5]==0x2C && f[6]==0x01 && f[7]==0x00 && f[8]==0x00 && f[9]==Sum2(f,9) && f[9]==0xA9;
        }
        CHECK(frameOk, "[A] A5 write frame bytes == golden (06 00 17 00 0D 2C 01 00 00 A9)");
        CHECK(!s.log.empty() && s.log.front().size()==1 && s.log.front()[0]==0x05, "[A] first byte on the wire is ENQ (golden :1552)");
        CHECK(!s.log.empty() && s.log.back().size()==1 && s.log.back()[0]==0x06, "[A] last byte on the wire is ACK (golden :1632)");
    }
    // ---------------- [A] read setting back (golden ReadIndexTorqueSetting_Pana A5) ----------------
    {
        FakeServo s; s.stored=300;
        fMain->edtReadZ1->Text=0;
        COM2->ReadIndexTorqueSetting(0);
        bool done=RunUntil(s, []{ return COM2->fPanasonicParameterRW==false; }, 8000);
        CHECK(done, "[A] read-setting handshake completes");
        bool frameOk=false;
        for(size_t i=0;i<s.log.size();i++)
            if(s.log[i].size()==6)
                frameOk = s.log[i][0]==0x02 && s.log[i][1]==0x00 && s.log[i][2]==0x07 && s.log[i][3]==0x00 &&
                          s.log[i][4]==0x0D && s.log[i][5]==0xEA;   // golden :1377-1385: ~(02+07+0D)+1 = 0xEA
        CHECK(frameOk, "[A] A5 read frame bytes == golden (02 00 07 00 0D EA)");
        CHECK(fMain->edtReadZ1->Text=="300", "[A] read-back lands in fMain->edtReadZ1 (golden :1818-1821)");
    }
    // ---------------- [F] ACK and ENQ arriving 20 ms apart are ONE packet ----------------
    {
        COM2->WriteIndexTorqueSetting(1, "123");
        std::vector<unsigned char> tx;
        DWORD t0=::GetTickCount();
        while(tx.empty() && ::GetTickCount()-t0<3000) { Beat(); tx=TakeTx(); if(tx.empty()) ::Sleep(5); }
        CHECK(tx.size()==1 && tx[0]==0x05, "[F] ENQ sent");
        Reply(V({0x04}));                                         // EOT
        Beat(); tx=TakeTx();
        CHECK(tx.size()==10, "[F] data frame sent after EOT");
        COM2->Comm1->SimInjectReceive("\x06", 1);                 // ACK ...
        ::Sleep(20);
        Beat();                                                   // 20 ms of silence: must NOT deliver yet
        CHECK(TakeTx().empty(), "[F] nothing delivered before 100 ms of silence");
        COM2->Comm1->SimInjectReceive("\x05", 1);                 // ... then ENQ 20 ms later
        ::Sleep(130);
        Beat(); tx=TakeTx();
        CHECK(tx.size()==1 && tx[0]==0x04, "[F] ACK+ENQ split 20 ms apart handled as one packet -> EOT (golden :1596)");
        Reply(V({0x01,0x00,0x97,0x00}));
        Beat(); tx=TakeTx();
        CHECK(tx.size()==1 && tx[0]==0x06, "[F] ACK after the drive's reply");
        FakeServo s;
        RunUntil(s, []{ return COM2->fPanasonicParameterRW==false; }, 3000);
    }

#ifdef SOFT_SIMULTE
    {
        int ret=COM2->iWriteAndCheckMotorTorque(0, 300);
        CHECK(ret==1, "[B] SIM: iWriteAndCheckMotorTorque returns 1 at once (golden :1851-1857 SOFT_SIMULTE arm)");
    }
#else
    // ---------------- [B] full iWriteAndCheckMotorTorque(0,300) ----------------
    {
        FakeServo s;
        InitWriteAndCheckMotorTorqueTask();
        int ret=0;
        DWORD t0=::GetTickCount();
        while(ret==0 && ::GetTickCount()-t0<30000)
        {
            ret=COM2->iWriteAndCheckMotorTorque(0, 300);
            Beat();
            std::vector<unsigned char> tx=TakeTx();
            if(!tx.empty()) { std::vector<unsigned char> r=s.OnTx(tx); if(!r.empty()) Reply(r); }
            else ::Sleep(5);
        }
        CHECK(ret==1, "[B] SHIP: iWriteAndCheckMotorTorque(0,300) -> 1 after a real write + read-back");
        CHECK(s.writes>=1 && s.stored==300 && s.paramReads>=1, "[B] the drive really got 300 and was read back");
        CHECK(fMain->edtReadZ1->Text=="300", "[B] edtReadZ1 == \"300\" (golden :1952 compare)");
        CHECK(fMain->chkReadTorque1->Checked==true, "[B] chkReadTorque1 left set for the feedback read (golden :1892)");
    }
    // ---------------- [C] torque feedback read (ReadTorque_Panasonic) ----------------
    {
        FakeServo s; s.torqueRaw=300;                               // -> 300/20 = 15.00
        fMain->edTorue0->Text="";
        COM2->InitReadTorueTask();
        bool done=RunUntil(s, []{ return COM2->GetReadTorueTask()==999; }, 8000);
        CHECK(done, "[C] ReadTorque_Panasonic reaches Task 999 (golden :992)");
        CHECK(s.torqueReads>=1, "[C] 4-byte torque request went out (golden :909-911)");
        CHECK(fMain->edTorue0->Text=="15.00", "[C] edTorue0 == \"15.00\" (k/20, golden :1811-1813 + :975)");
        CHECK(fMain->chkReadTorque1->Checked==false && fMain->chkReadTorque2->Checked==false, "[C] chkReadTorque1/2 cleared (golden :993-994)");
    }
    // ---------------- [B-] wrong read-back -> 5 retries -> 2 ----------------
    {
        FakeServo s; s.readBackBias=-1;                             // drive reports 299
        InitWriteAndCheckMotorTorqueTask();
        fMain->chkReadTorque1->Checked=false;
        int ret=0;
        DWORD t0=::GetTickCount();
        while(ret==0 && ::GetTickCount()-t0<60000)
        {
            ret=COM2->iWriteAndCheckMotorTorque(0, 300);
            fMain->chkReadTorque1->Checked=false;                    // keep the feedback reader out of the way (atester 12200 does the same)
            fMain->chkReadTorque2->Checked=false;
            Beat();
            std::vector<unsigned char> tx=TakeTx();
            if(!tx.empty()) { std::vector<unsigned char> r=s.OnTx(tx); if(!r.empty()) Reply(r); }
            else ::Sleep(5);
        }
        CHECK(ret==2, "[B-] SHIP: read-back 299 != 300 -> returns 2 (golden :1952-1961)");
        CHECK(s.writes==5, "[B-] exactly 5 write attempts before giving up (golden iRetryCount<5, :1955)");
    }
    {   // AI(W906-TORQUE-1203) 20260925: HT9050 走 1203 掛鉤（rs232.cpp 檔尾 W906_Ht9050TorqueLimit）
        extern int (*W906_Pci1203TorqueLimitHook)(int, int, AnsiString*);
        const AnsiString oldCard = MOT[MTestZ1].CardType;
        MOT[MTestZ1].CardType = "PCI1203";   // AI(W906-HT9050-AS-LS) 20260926：判準改成 Index Z 是 1203 卡（原本設 MachineTypeChoice=Type_HT9050）
        W906_Pci1203TorqueLimitHook = 0;
        TakeTx();
        CHECK(COM2->iWriteAndCheckMotorTorque(0, 30) == 2 && TakeTx().empty(), "[H0] HT9050 without the 1203 hook -> 2 at once, nothing sent on RS232");
        g_hCalls = 0; g_hOkAt = 3;
        W906_Pci1203TorqueLimitHook = FakeTorqueHook;
        int r1 = COM2->iWriteAndCheckMotorTorque(0, 30), r2 = COM2->iWriteAndCheckMotorTorque(0, 30), r3 = COM2->iWriteAndCheckMotorTorque(0, 30);
        CHECK(r1 == 0 && r2 == 0 && r3 == 1 && g_hMi == MTestZ1 && g_hVal == 300, "[H1] 30% -> hook(MTestZ1, 300 = 0.1% units); matches on the 3rd call -> 0,0,1");
        g_hCalls = 0; g_hOkAt = 99;
        int last = -1, n = 0;
        while (n < 10 && (last = COM2->iWriteAndCheckMotorTorque(0, 300)) == 0) n++;
        CHECK(last == 2 && g_hCalls == 5 && g_hVal == 3000, "[H2] never matches -> 2 after exactly 5 tries (golden iRetryCount<5); home 300% -> 3000");
        g_hCalls = 0;
        CHECK(COM2->iWriteAndCheckMotorTorque(1, 30) == (MOT[MTestZ2].Motor == 0 || !MOT[MTestZ2].Motor->Enable ? 1 : COM2->iWriteAndCheckMotorTorque(1, 30)) && g_hCalls == 0,
              "[H3] MotorIndex 1 (MTestZ2 not enabled on HT9050) -> 1, hook not called");
        W906_Pci1203TorqueLimitHook = 0;
        MOT[MTestZ1].CardType = oldCard;
    }
#endif

    COM2->Comm1->StopComm();
    std::printf("[rs232_torque] %d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
