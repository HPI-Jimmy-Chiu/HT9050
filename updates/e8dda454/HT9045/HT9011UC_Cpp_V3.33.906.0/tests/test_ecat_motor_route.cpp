// ===========================================================================
//  tests/test_ecat_motor_route.cpp   (ctest: EcatMotorRoute)
//
//  AI(W906-ECAT-ROUTE) 20260929: TMyEtherCatMotor (Motor/myEthercatmotor.cpp,
//  ht9045_motor, built WITHOUT HAVE_PCI1203) against a FAKE engine motor route
//  (Motor/EcatMotorRoute.h) that records every (board, port, op, which, value)
//  and serves scripted samples. Design docs/ENGINE_1203_MOTOR_ROUTE_DESIGN.md 7.4:
//    A  no route installed = every method's return and side effect as before (7.5)
//    B  identity: the route sees golden's (BoardID, Port) and MotorID
//    C  each method's command sequence = the HAVE_PCI1203 arm's (order, values)
//    D  read decoding (ScanMotorStatus's 8 bits + ERROR_STOP, GetAlarm 0x3004e, positions)
//    E  pending: MotionDone / Busy / MoveToPos never act on a pending sample
//    F  InitMotor: EMG off = nothing; initCfg gets golden's parameters; the tail
//    G  homing through TMyMotor::MotorHome (golden, unchanged): DS402 and card-side
//    H  Direction=1 is never claimed
//    I  StopAllMotor(true): golden's ServoAlarmOn x SystemStart rule; the Index-name hole (Q4)
//    R  reference: the engine path and Motor Test's pure functions (WebMotorAccess.cpp
//       MotorUserToCard / MotorSpeedFromPct / MotorRateFromGolden) give the same card numbers
//  The real route body (pending ledger, Q1, Q7, Q11, homing states) is tested separately
//  in tests/test_pci1203_motor_route.cpp. Nothing here opens a card or writes a file.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "mysensor.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"
#include "Motor/myGALILmotor.h"
#include "Motor/EcatMotorRoute.h"
#include "WebMotorAccess.h"

#include <cstdio>
#include <map>
#include <vector>
#include <windows.h>

extern bool bAlarm;                 // Motor/myEthercatmotor.cpp file-global (golden :44)

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_ecat_motor_route.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
//  The fake route
// ---------------------------------------------------------------------------
struct Rec { int b, p, op, which; double v; };
static std::vector<Rec> g_calls;
static int  g_binds = 0, g_bindB = -1, g_bindP = -1, g_bindId = -1;
static bool g_bindDir = false, g_bindOk = true;
static TEcatAxisRead g_s;
static bool g_readOk = true;
static int  g_sAddr = -1;       //AI(W906-ENG1203) 20260929: review E4/E5 -- the ONE (BoardID*100+Port) the sample belongs to; any other read fails
static std::map<int, unsigned long> g_rc;   // per op
static int  g_inits = 0, g_initCls = -1;
static bool g_initSt = false, g_initIn1 = false, g_initOk = true;
static double g_initMv = 0, g_initMa = 0, g_initMd = 0;
static int  g_homeStarts = 0, g_homeDoneRet = 0;
static bool g_homeDir = false, g_cardSide = false;
static double g_homeHi = 0, g_homeLo = 0, g_homeAcc = 0, g_homeDec = 0;

static bool FBind(int b, int p, int id, bool dir) { ++g_binds; g_bindB = b; g_bindP = p; g_bindId = id; g_bindDir = dir; return g_bindOk; }
static bool FRead(int b, int p, TEcatAxisRead* out) { if (!g_readOk || b * 100 + p != g_sAddr) return false; *out = g_s; return true; }   //AI(W906-ENG1203) 20260929: honours board / port (review E5)
static unsigned long FCall(int b, int p, int op, int which, double v)
{
    Rec r = { b, p, op, which, v };
    g_calls.push_back(r);
    std::map<int, unsigned long>::const_iterator it = g_rc.find(op);
    return it == g_rc.end() ? 0ul : it->second;
}
static bool FInit(int, int, int cls, bool st, bool in1, double mv, double ma, double md)
{
    ++g_inits; g_initCls = cls; g_initSt = st; g_initIn1 = in1; g_initMv = mv; g_initMa = ma; g_initMd = md;
    return g_initOk;
}
static int FHomeStart(int, int, bool dir, double hi, double lo, double acc, double dec)
{
    ++g_homeStarts; g_homeDir = dir; g_homeHi = hi; g_homeLo = lo; g_homeAcc = acc; g_homeDec = dec;
    return 1;
}
static int  FHomeDone(int, int) { return g_homeDoneRet; }
static bool FCardSide(int, int) { return g_cardSide; }
static const TEcatMotorRoute kFake = { FBind, FRead, FCall, FInit, FHomeStart, FHomeDone, FCardSide };

static void ResetFake(int addr)   //AI(W906-ENG1203) 20260929: + addr = the motor whose sample the fake serves (-1 = none)
{
    g_calls.clear(); g_binds = 0; g_bindOk = true; g_s = TEcatAxisRead(); g_s.valid = true; g_s.state = kEcStaReady;
    g_readOk = true; g_rc.clear(); g_inits = 0; g_initOk = true; g_homeStarts = 0; g_homeDoneRet = 0; g_cardSide = false;
    g_sAddr = addr;
}
static int Count(int op) { int n = 0; for (std::size_t i = 0; i < g_calls.size(); ++i) if (g_calls[i].op == op) ++n; return n; }

// protected members, for the checks only
struct EcatPeek : TMyEtherCatMotor {
    explicit EcatPeek(int a) : TMyEtherCatMotor(a) {}
    using TMyEtherCatMotor::Busy;
    using TMyEtherCatMotor::Error;
    using TMyEtherCatMotor::GetHomeIO;
    int      Task() const { return iHomeObjectTask; }
    unsigned Spd()  const { return iSpeed; }
    void     SetISpeed(unsigned v) { iSpeed = v; }   // HTMotor's ctor leaves iSpeed unset (golden: OldSpeed=iSpeed reads it)
};

static bool DoorClosed() { return false; }
static const int kEmg[4] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG, SnRearRightEMG };
static void EmgReleased() { for (int i = 0; i < 4; ++i) Sen[kEmg[i]].Enable = false; }     // IsOff() == false
static void EmgPressed()  { for (int i = 0; i < 4; ++i) { Sen[kEmg[i]].Enable = true; Sen[kEmg[i]].ISABase = 99; Sen[kEmg[i]].Type = 1; } }   // IsOff() == true, no IO read

static void Setup(EcatPeek& m)
{
    m.SetISpeed(0);
    m.Enable = true; m.Direction = false; m.HomeDirection = true; m.GearRatio = 1.0;
    m.PJogHighSpeed = 100000; m.PJogLowSpeed = 100; m.InitSpeed = 1000;
    m.PHomeHighSpeed = 20000; m.PHomeLowSpeed = 2000;
    m.MotorType = Servo_Motor; m.bSensorType = true; m.bIn1Logic = true; m.PServoAlarmOn = true;
    m.SetAccDataBase(500000.0); m.SetDecDataBase(400000.0); m.SetAcc(500000.0); m.SetDec(400000.0);
    m.PSoftLimitP = 999999; m.PSoftLimitN = -999999; m.SetRange(100);
    m.MotorIdleSafeDoorCheck = DoorClosed;
}

int main()
{
    // ==================== A: no route installed ====================
    SetEcatMotorRoute(0);
    EmgReleased();
    {
        EcatPeek m(301);
        Setup(m);
        CHECK(EcatMotorRoute() == 0);
        CHECK(W906_EcCall(3, 1, kEcStopDec, 0, 0.0) == (unsigned long)kEcRcNoRoute);
        TEcatAxisRead s; CHECK(!W906_EcRead(3, 1, s) && !s.valid);
        CHECK(m.InitMotor(0) == false);                                  // bAxisOpen stays false (golden's open-failed shape)
        bFirstClickJog = false; m.DecStop(); CHECK(bFirstClickJog == false);   // golden's ExtDrive-failed branch: flag untouched
        bFirstClickJog = true;
        CHECK(!m.JogP() && !m.JogN() && bFirstClickJog == true);
        m.SetSpeed(50000); CHECK(m.ReadSpeed() == 50000u);
        CHECK(m.SetPosition(5) == 1 && m.SetCommand(5) == 0 && !m.ResetPos(0));
        CHECK(!m.GetAlarm() && !m.HomeFlag());
        bool Led[10]; for (int i = 0; i < 10; ++i) Led[i] = true;
        bAlarm = true;
        m.ScanMotorStatus(Led);
        CHECK(!Led[iCcwLed] && !Led[iCwLed] && !Led[iHomeLed] && !Led[iSoftcwLed] && !Led[iSoftccwLed] &&
              !Led[iAlarmLed] && !Led[iServoOn] && !Led[iEmgLed]);
        CHECK(Led[iInposLed] && Led[iServoalarmLed] && bAlarm == false);  // the other two untouched; bAlarm = Led[iAlarmLed]
        m.Enable = false; for (int i = 0; i < 10; ++i) Led[i] = false;
        m.ScanMotorStatus(Led); CHECK(Led[iHomeLed] && !Led[iInposLed]);  // golden's disabled branch
        m.Enable = true;
        CHECK(m.ReadRealPos() == 0 && m.ReadEnCoderRealPos() == 0 && m.ReadPos() == 0);
        CHECK(!m.MotionDone() && !m.MoveToPos(1000) && !m.MoveTo(1000));
        CHECK(m.Busy() && !m.Error() && !m.GetHomeIO());
        m.SetHomeobjectTask(1);  CHECK(!m.HomeObject() && m.Task() == 1);
        m.SetHomeobjectTask(10); CHECK(!m.HomeObject() && m.Task() == 1);
        m.SetRate(50); m.SetSoftLimit(10, -10); m.MotOutputOn(1); m.MotOutputOff(1); m.Stop(); m.ResetState();
        CHECK(true);                                                     // reached: none of them needs a route
    }

    // ==================== B + C: identity and command sequences ====================
    SetEcatMotorRoute(&kFake);
    ResetFake(1402);
    {
        EcatPeek m(1402);                                                // BoardID 14, Port 2 -> MotorID 142
        Setup(m);
        bFirstClickJog = false;
        m.DecStop();                                                     // lazy claim, then StopDec + ExtDrive(0)
        CHECK(g_binds == 1 && g_bindB == 14 && g_bindP == 2 && g_bindId == 142 && g_bindDir == false);
        CHECK(g_calls.size() == 2 && g_calls[0].op == kEcStopDec && g_calls[1].op == kEcExtDrive && g_calls[1].v == 0.0);
        CHECK(g_calls[0].b == 14 && g_calls[0].p == 2 && bFirstClickJog == true);
        g_calls.clear(); m.DecStop(); CHECK(g_binds == 1 && g_calls.size() == 2);   // claimed once
        g_calls.clear(); bFirstClickJog = false; g_rc[kEcExtDrive] = 0x80000001ul;
        m.DecStop(); CHECK(bFirstClickJog == false);                     // golden: ExtDrive failing returns before the flag
        g_rc.clear();

        g_calls.clear(); bFirstClickJog = true;
        CHECK(m.JogP());
        CHECK(g_calls.size() == 2 && g_calls[0].op == kEcExtDrive && g_calls[0].v == 1.0 && g_calls[1].op == kEcJog && g_calls[1].v == 0.0);
        CHECK(bFirstClickJog == false);
        g_calls.clear(); CHECK(m.JogP()); CHECK(g_calls.size() == 1 && g_calls[0].op == kEcJog);       // second click: no ExtDrive
        g_calls.clear(); CHECK(m.JogN()); CHECK(g_calls.size() == 1 && g_calls[0].op == kEcJog && g_calls[0].v == 1.0);
        {   // golden quirk: bFirstClickJog is FILE-GLOBAL -- another axis's first jog sends no ExtDrive(1)
            EcatPeek o(1500); Setup(o);
            g_calls.clear(); CHECK(o.JogP()); CHECK(g_calls.size() == 1 && g_calls[0].op == kEcJog);
        }
        g_calls.clear(); bFirstClickJog = true; g_rc[kEcExtDrive] = 0x80000001ul;
        CHECK(!m.JogP()); CHECK(g_calls.size() == 1 && bFirstClickJog == true);   // ExtDrive failed: no Jog, flag kept
        g_rc.clear();

        // SetSpeed: persent = x / PJogHighSpeed clamped [0.01, 1]; VelLow = InitSpeed*p, VelHigh = PJogHighSpeed*p
        g_calls.clear(); m.SetSpeed(50000);
        CHECK(g_calls.size() == 4);
        CHECK(g_calls[0].which == kEcSpdInit && g_calls[0].v == 500.0 && g_calls[1].which == kEcSpdRun && g_calls[1].v == 50000.0);
        CHECK(g_calls[2].which == kEcSpdAcc && g_calls[2].v == 500000.0 && g_calls[3].which == kEcSpdDec && g_calls[3].v == 400000.0);
        g_calls.clear(); m.SetSpeed(50000, true);
        CHECK(g_calls.size() == 8 && g_calls[4].which == kEcSpdJogInit && g_calls[5].which == kEcSpdJogRun && g_calls[5].v == 50000.0 &&
              g_calls[6].which == kEcSpdJogAcc && g_calls[7].which == kEcSpdJogDec);   // CFG_AxJogVLTime (I32): gap, not sent
        g_calls.clear(); m.SetSpeed(10);                                  // below 1 % -> clamped to 0.01
        CHECK(g_calls.size() == 4 && g_calls[0].v == 10.0 && g_calls[1].v == 1000.0);
        g_calls.clear(); m.PJogHighSpeed = 0; m.SetSpeed(10); CHECK(g_calls.empty()); m.PJogHighSpeed = 100000;

        // SetRate -> Acc = Dec = Rate (checked against MotorRateFromGolden in part R)
        g_calls.clear(); m.SetSpeed(50000); g_calls.clear(); m.SetRate(50);
        CHECK(g_calls.size() == 2 && g_calls[0].which == kEcSpdAcc && g_calls[1].which == kEcSpdDec && g_calls[0].v == g_calls[1].v);

        // SetSoftLimit: values only; Direction=1 swaps and negates (golden :773-782)
        g_calls.clear(); m.SetSoftLimit(123, -456);
        CHECK(g_calls.size() == 2 && g_calls[0].which == kEcLimSwPel && g_calls[0].v == 123.0 && g_calls[1].which == kEcLimSwMel && g_calls[1].v == -456.0);

        // SetCommand / SetPosition / ResetPos
        g_calls.clear(); g_rc[kEcSetCmdPos] = (unsigned long)kEcRcDs402Coord;
        CHECK(m.SetCommand(7) == (int)kEcRcDs402Coord && g_calls.size() == 1 && g_calls[0].op == kEcSetCmdPos && g_calls[0].v == 7.0);
        g_rc.clear();
        g_calls.clear(); CHECK(m.SetPosition(8) == 1 && g_calls.size() == 1 && g_calls[0].op == kEcSetActPos && g_calls[0].v == 8.0);
        g_calls.clear(); CHECK(!m.ResetPos(0) && g_calls.size() == 2);   // golden: SetPosition returns 1 -> ResetPos false
        m.Enable = false; g_calls.clear(); CHECK(m.SetCommand(9) == 0 && g_calls.empty()); m.Enable = true;

        // MotOutputOn / Off -- the file-global bAlarm (golden quirk)
        g_calls.clear(); bAlarm = false; m.MotOutputOn(1);
        CHECK(g_calls.size() == 1 && g_calls[0].op == kEcSvOn && g_calls[0].v == 1.0);
        g_calls.clear(); bAlarm = true; m.MotOutputOn(1);
        CHECK(g_calls.size() == 2 && g_calls[0].op == kEcResetError && g_calls[1].op == kEcSvOn);
        bAlarm = false;
        g_calls.clear(); m.MotOutputOff(1); CHECK(g_calls.size() == 1 && g_calls[0].op == kEcSvOn && g_calls[0].v == 0.0);
        g_calls.clear(); m.SetServoOn(true); CHECK(g_calls.size() == 1 && g_calls[0].v == 1.0);
        g_calls.clear(); m.Stop(); CHECK(g_calls.size() == 1 && g_calls[0].op == kEcStopEmg);
        g_calls.clear(); m.ResetState(); CHECK(g_calls.size() == 1 && g_calls[0].op == kEcResetError);

        // MoveToPos: only on a non-pending READY sample; Tar as given (pulse)
        g_calls.clear(); g_s.cmdPos = 0.0;
        CHECK(!m.MoveToPos(1000));                                        // sent, not there yet
        CHECK(g_calls.size() == 1 && g_calls[0].op == kEcMoveAbs && g_calls[0].v == 1000.0);
        CHECK(g_calls[0].b == 14 && g_calls[0].p == 2);                   //AI(W906-ENG1203) 20260929: review E4 -- the move goes to THIS axis (stations (14,2) and (2,14) both exist on a ring)
        g_calls.clear(); g_s.cmdPos = 1000.0;
        CHECK(m.MoveToPos(1000));                                         // golden: MotionDone() && Tar==iPos
        g_calls.clear(); g_s.pending = true;
        CHECK(!m.MoveToPos(2000) && g_calls.empty());                     // E: a pending sample is never "done"
        g_s.pending = false;
        // MoveTo: SetSpeed(iSpeed) + MoveRel(the ABSOLUTE target -- golden quirk)
        g_calls.clear(); g_s.cmdPos = 0.0;
        m.MoveTo(300);
        CHECK(Count(kEcSetSpeed) == 4 && Count(kEcMoveRel) == 1 && g_calls.back().v == 300.0);

        // ==================== D: read decoding ====================
        bool Led[10];
        const struct { unsigned long bit; int led; } bits[8] = {
            { 1ul << 2, iCcwLed }, { 1ul << 3, iCwLed }, { 1ul << 4, iHomeLed }, { 1ul << 16, iSoftcwLed },
            { 1ul << 17, iSoftccwLed }, { 1ul << 1, iAlarmLed }, { 1ul << 14, iServoOn }, { 1ul << 6, iEmgLed } };
        int decOk = 0;
        for (int k = 0; k < 8; ++k) {
            g_s.motionIO = bits[k].bit; g_s.state = kEcStaReady;
            for (int i = 0; i < 10; ++i) Led[i] = false;
            m.ScanMotorStatus(Led);
            bool only = Led[bits[k].led];
            for (int j = 0; j < 8; ++j) if (j != k && Led[bits[j].led]) only = false;
            if (only) ++decOk;
        }
        CHECK(decOk == 8);
        g_s.motionIO = 0; g_s.state = kEcStaErrorStop; m.ScanMotorStatus(Led);
        CHECK(Led[iAlarmLed] && bAlarm == true);                          // golden :1185-1190 ERROR_STOP -> alarm lamp
        g_s.state = kEcStaReady; m.ScanMotorStatus(Led); CHECK(!Led[iAlarmLed] && bAlarm == false);
        g_readOk = false; for (int i = 0; i < 10; ++i) Led[i] = true; bAlarm = true;
        m.ScanMotorStatus(Led);
        CHECK(Led[iCcwLed] && Led[iAlarmLed] && bAlarm == true);          // a failed read returns: nothing touched (golden)
        g_readOk = true; bAlarm = false;
        int almOk = 0;
        const unsigned long almBits[6] = { 1ul << 1, 1ul << 2, 1ul << 3, 1ul << 6, 1ul << 16, 1ul << 17 };
        for (int k = 0; k < 6; ++k) { g_s.motionIO = almBits[k]; if (m.GetAlarm()) ++almOk; }
        CHECK(almOk == 6);
        g_s.motionIO = (1ul << 4) | (1ul << 14); CHECK(!m.GetAlarm());   // ORG / SVON are not alarms
        CHECK(m.GetHomeIO() && m.HomeFlag());
        g_s.motionIO = 0; CHECK(!m.GetHomeIO() && !m.HomeFlag());
        g_s.cmdPos = 1234.7;  CHECK(m.ReadRealPos() == 1234 && m.ReadPos() == 1234);
        g_s.cmdPos = -1234.7; CHECK(m.ReadRealPos() == -1234);
        m.GearRatio = 2.5; g_s.cmdPos = 1234.0; CHECK(m.ReadPos() == 3085); m.GearRatio = 1.0;
        g_s.actPos = 555.9; CHECK(m.ReadEnCoderRealPos() == 555);
        m.Enable = false; CHECK(m.ReadRealPos() == 0 && !m.MotionDone()); m.Enable = true;
        g_s.state = kEcStaReady; CHECK(!m.Busy() && m.MotionDone() && !m.Error());
        g_s.state = 5;           CHECK(m.Busy() && !m.MotionDone());
        g_s.state = kEcStaErrorStop; CHECK(m.Error() && m.Busy());
        g_s.state = kEcStaReady; g_s.pending = true; CHECK(m.Busy() && !m.MotionDone());   // E
        g_s.pending = false; g_readOk = false; CHECK(m.Busy() && !m.MotionDone() && !m.Error());
        g_readOk = true;
    }

    // ==================== F: InitMotor ====================
    ResetFake(2001);
    {
        EcatPeek m(2001);
        Setup(m);
        EmgPressed();
        CHECK(m.InitMotor(0) == false);
        CHECK(g_binds == 1 && g_inits == 0 && g_calls.empty());           // golden: Open_Axis first, then the EMG refusal
        EmgReleased();
        bAlarm = false;
        CHECK(m.InitMotor(0) == true);
        CHECK(g_inits == 1 && g_initCls == 0 && g_initSt == true && g_initIn1 == true);
        CHECK(g_initMv == 100000.0 && g_initMa == 500000.0 && g_initMd == 400000.0);   // MaxVel=PJogHighSpeed, MaxAcc=dAcc, MaxDec=dDec
        CHECK(g_calls.size() == 3 && g_calls[0].op == kEcSvOn && g_calls[0].v == 1.0 &&
              g_calls[1].op == kEcSetCmdPos && g_calls[1].v == 0.0 && g_calls[2].op == kEcSetActPos);   // golden tail :645-647
        m.MotorType = Rotate_Motor; g_calls.clear(); m.InitMotor(0); CHECK(g_initCls == 1);
        m.MotorType = Step_Motor;   g_calls.clear(); m.InitMotor(0); CHECK(g_initCls == 2);
        m.MotorType = Servo_Motor;
        g_initOk = false; g_calls.clear();
        CHECK(m.InitMotor(0) == false && g_calls.empty());               // the ladder gave up: no tail
        g_initOk = true;
        m.Enable = false; CHECK(m.InitMotor(0) == true); m.Enable = true;   // golden :408-409
    }

    // ==================== G: homing through TMyMotor::MotorHome ====================
    for (int pass = 0; pass < 2; ++pass) {
        const bool cardSide = (pass == 1);
        ResetFake(2201);
        g_cardSide = cardSide;
        EcatPeek m(2201);
        Setup(m);
        EmgReleased();
        TMyMotor t;
        t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";
        g_s.motionIO = (1ul << 14);                                       // Servo ON lamp, no ORG yet
        g_s.cmdPos = 4321.0;
        t.MotorInitial();
        CHECK(t.MotorHome(false) == 0);                                   // case 1 -> HomeReset, Task 10
        CHECK(t.MotorHome(false) == 0);                                   // case 10: Home() -> EtherCatMotHome case 1
        CHECK(g_binds == 1 && g_inits == 1 && g_homeStarts == 1);
        CHECK(g_homeDir == true && g_homeHi == 20000.0 && g_homeLo == 2000.0 && g_homeAcc == 500000.0 && g_homeDec == 400000.0);
        CHECK(Count(kEcStopDec) == 1);                                    // golden case 1: DecStop() before InitMotor
        CHECK(Count(kEcSetLimit) == 2);                                   // SetSoftLimit(999999,-999999)
        CHECK(m.Task() == 10);
        g_calls.clear();
        CHECK(t.MotorHome(false) == 0 && m.Task() == 10);                 // homeDone 0: stays
        g_homeDoneRet = 1;
        CHECK(t.MotorHome(false) == 0 && m.Task() == 20);                 // HomeDelay 0.3 s
        ::Sleep(350);
        g_calls.clear();
        CHECK(t.MotorHome(false) == 0);                                   // HomeObject true -> MotorHome Task 20 (Servo ON lamp lit)
        CHECK(m.LastHomePos == -4321);                                    // golden: LastHomePos=-ReadPos()
        CHECK(Count(kEcSetCmdPos) == (cardSide ? 1 : 0) && Count(kEcSetActPos) == (cardSide ? 1 : 0));   // DS402: no zeroing
        CHECK(Count(kEcSetSpeed) == 4 && Count(kEcSetLimit) == 2);        // SetSpeed(OldSpeed), SetSoftLimit(P, N)
        CHECK(m.Task() == 1);
        g_s.motionIO = (1ul << 14) | (1ul << 4);                          // ORG lit (Q3: golden's own check)
        CHECK(t.MotorHome(false) == 1 && t.HomeFlag == 1);
    }
    {   // Servo lamp off after the home -> golden MotorHome returns 4
        ResetFake(2301);
        EcatPeek m(2301); Setup(m); EmgReleased();
        TMyMotor t; t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";
        g_s.motionIO = 0; g_homeDoneRet = 1;
        t.MotorInitial();
        t.MotorHome(false); t.MotorHome(false); t.MotorHome(false);       // Task 1 -> 10 -> start -> done -> 20
        ::Sleep(350);
        CHECK(t.MotorHome(false) == 4);
    }

    // ==================== H: Direction=1 never claimed ====================
    ResetFake(2401);
    {
        EcatPeek m(2401); Setup(m); m.Direction = true;
        m.DecStop();
        CHECK(g_binds == 1 && g_bindDir == true && g_calls.empty());     // the fake even ACCEPTED it: `&& !Direction` refuses
        CHECK(m.InitMotor(0) == false && g_inits == 0);
        CHECK(!m.MoveToPos(1000) && g_calls.empty());
    }

    // ==================== I: StopAllMotor(true) ====================
    ResetFake(-1);
    {
        for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;
        EcatPeek a(2501), b(2502), c(2503);
        Setup(a); Setup(b); Setup(c);
        a.PServoAlarmOn = true; b.PServoAlarmOn = false; c.PServoAlarmOn = true;
        const int ia = 0, ib = 1, ic = MTestZ1;                           // ic: an Index NAME on a 1203 axis (HT9050 M14)
        MOT[ia].Motor = &a; MOT[ia].Mot_Name = ia;
        MOT[ib].Motor = &b; MOT[ib].Mot_Name = ib;
        MOT[ic].Motor = &c; MOT[ic].Mot_Name = ic;
        a.DecStop(); b.DecStop(); c.DecStop();                            // claim all three
        std::map<int, int> stops;
        const bool keep = SystemStart;
        SystemStart = false; g_calls.clear();
        StopAllMotor(true);
        for (std::size_t k = 0; k < g_calls.size(); ++k) if (g_calls[k].op == kEcStopDec) ++stops[g_calls[k].b * 100 + g_calls[k].p];
        CHECK(stops[2501] == 1 && stops[2502] == 1 && stops[2503] == 0);  // idle: both rows stop; the Index name never (Q4 hole)
        stops.clear();
        SystemStart = true; g_calls.clear();
        StopAllMotor(true);
        for (std::size_t k = 0; k < g_calls.size(); ++k) if (g_calls[k].op == kEcStopDec) ++stops[g_calls[k].b * 100 + g_calls[k].p];
        CHECK(stops[2501] == 1 && stops[2502] == 0 && stops[2503] == 0);  // running: ServoAlarmOn=0 kept moving (golden, Q8)
        SystemStart = keep;
        MOT[ia].Motor = NULL; MOT[ib].Motor = NULL; MOT[ic].Motor = NULL;
    }

    // ==================== R: same card numbers as Motor Test's pure functions ====================
    ResetFake(2601);
    {
        EcatPeek m(2601); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";
        int posOk = 0, posN = 0;
        const double gears[3] = { 1.0, 2.5, 0.3 };
        const int targets[6] = { 0, 1, 1001, -2500, 123457, -7 };
        for (int gi = 0; gi < 3; ++gi)
            for (int ti = 0; ti < 6; ++ti) {
                m.GearRatio = gears[gi];
                int p = targets[ti];
                t.GetRealPos(&p);
                ++posN; if (p == ht9045::MotorUserToCard(targets[ti], gears[gi])) ++posOk;
                g_s.cmdPos = (double)p;
                if (m.ReadPos() != ht9045::MotorCardToUser((double)p, gears[gi])) --posOk;
            }
        CHECK(posOk == posN);
        m.GearRatio = 1.0;
        ht9045::MotorGolden g;
        g.jogHigh = m.PJogHighSpeed; g.jogLow = m.PJogLowSpeed; g.initSpeed = m.InitSpeed;
        g.acc = 500000.0; g.dec = 400000.0; g.indexMotor = false; g.zStack = false;
        int spOk = 0;
        const int pcts[5] = { 1, 20, 50, 99, 100 };
        for (int k = 0; k < 5; ++k) {
            g.acc = m.W906_RuntimeAcc(); g.dec = m.W906_RuntimeDec();   // golden SetRate rewrites dAcc: the reference takes what SetSpeed will send
            g_calls.clear();
            t.SetSpeed(pcts[k]);
            const ht9045::Motor1203Speed ref = ht9045::MotorSpeedFromPct(pcts[k], g);
            if (g_calls.size() == 4 && g_calls[0].v == ref.velLow && g_calls[1].v == ref.velHigh &&
                g_calls[2].v == ref.acc && g_calls[3].v == ref.dec && m.Spd() == ref.s) ++spOk;
            g_calls.clear();
            const unsigned a = 25u + 10u * (unsigned)k;
            m.SetRate(a);
            const ht9045::MotorGoldenRate gr = ht9045::MotorRateFromGolden(a, m.PJogHighSpeed, m.InitSpeed, m.Spd(), m.ReadRange());
            if (!gr.skip && g_calls.size() == 2 && g_calls[0].v == gr.rate && g_calls[1].v == gr.rate) ++spOk;
        }
        CHECK(spOk == 10);
    }

    //AI(W906-ENG1203) 20260929: ==================== review fixes (INBOX 112) ====================
    // E12: MoveToPos is the FIRST call on a fresh axis -> its lazy claim, then the move (a first MoveToPos never answering is the failure)
    ResetFake(1403);
    {
        EcatPeek n(1403); Setup(n);                                       // BoardID 14, Port 3
        CHECK(!n.MoveToPos(700));
        CHECK(g_binds == 1 && g_bindB == 14 && g_bindP == 3);
        CHECK(g_calls.size() == 1 && g_calls[0].op == kEcMoveAbs && g_calls[0].v == 700.0 && g_calls[0].b == 14 && g_calls[0].p == 3);
    }
    // E13: golden EtherCatMotHome case 1 -- an axis still moving gets DecStop and case 1 WAITS (no InitMotor / home)
    ResetFake(2701);
    {
        EcatPeek h(2701); Setup(h); EmgReleased();
        g_s.state = 5;                                                    // STA_AX_PTP_MOT: still decelerating from the last move
        h.SetHomeobjectTask(1);
        CHECK(!h.HomeObject() && h.Task() == 1);
        CHECK(g_inits == 0 && g_homeStarts == 0 && Count(kEcStopDec) == 1);
        g_s.state = kEcStaReady;
        CHECK(!h.HomeObject() && h.Task() == 10);                         // now: DecStop, InitMotor, the home command
        CHECK(g_inits == 1 && g_homeStarts == 1 && Count(kEcStopDec) == 2);
    }
    // HIGH-1: W906_EcForeignStopResetFcmd = golden PCIL132_StopMotor's fCMD=false for a stop that bypassed the route
    ResetFake(2801);
    {
        for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;
        EcatPeek a(2801), b(2801), z(2801), off(2801), oth(2802);         // a / b: two rows on one axis (golden allows it)
        Setup(a); Setup(b); Setup(z); Setup(off); Setup(oth);
        off.Enable = false;
        const int ia = 2, ib = 3, iz = MTestZ1, io = 4, ifar = 5;         // iz: an Index NAME on that axis (Q4: moves through Galil)
        MOT[ia].Motor = &a;   MOT[ia].Mot_Name = ia;
        MOT[ib].Motor = &b;   MOT[ib].Mot_Name = ib;
        MOT[iz].Motor = &z;   MOT[iz].Mot_Name = iz;
        MOT[io].Motor = &off; MOT[io].Mot_Name = io;
        MOT[ifar].Motor = &oth; MOT[ifar].Mot_Name = ifar;
        const int idx[5] = { ia, ib, iz, io, ifar };
        for (int k = 0; k < 5; ++k) MOT[idx[k]].fCMD = true;
        SetEcatMotorRoute(0);
        CHECK(W906_EcForeignStopResetFcmd(28, 1) == 0);                   // no route installed: nothing touched
        CHECK(MOT[ia].fCMD && MOT[ib].fCMD && MOT[iz].fCMD && MOT[io].fCMD && MOT[ifar].fCMD);
        SetEcatMotorRoute(&kFake);
        CHECK(W906_EcForeignStopResetFcmd(28, 1) == 2);
        CHECK(!MOT[ia].fCMD && !MOT[ib].fCMD);                            // the rows of (28, 1)
        CHECK(MOT[iz].fCMD && MOT[io].fCMD && MOT[ifar].fCMD);            // PCIL132_StopMotor's guard: Index name, Enable=false; another axis
        CHECK(g_calls.empty());                                           // bookkeeping only: nothing sent

        // the scenario review HIGH-1 (a) describes, end to end through golden TMyMotor::MotorMove:
        //   a move is under way (fCMD=true), another unit's alarm stops the axis short through WebMotorAccess's Stop1203
        //   (not through golden), the axis reads READY -- golden would call that ARRIVAL; after the reset it re-issues.
        for (int k = 0; k < 5; ++k) MOT[idx[k]].Motor = NULL;
        MOT[ia].Motor = &a; a.PServoAlarmOn = false;                      // golden's short arrival arm (no InPos / encoder check)
        MOT[ia].fCMD = false; MOT[ia].iOldPos = 0;
        g_calls.clear(); g_s.cmdPos = 0.0; g_s.state = kEcStaReady;
        CHECK(MOT[ia].MotorMove(1000) == 0);                              // MoveAbs(1000) sent, fCMD=true
        CHECK(Count(kEcMoveAbs) == 1 && MOT[ia].fCMD);
        g_s.cmdPos = 400.0;                                               // stopped short at 400 by the foreign stop, READY
        CHECK(W906_EcForeignStopResetFcmd(28, 1) == 1 && !MOT[ia].fCMD);
        CHECK(MOT[ia].MotorMove(1000) == 0);                              // NOT 1: re-issued to 1000 (what golden does after its own stop)
        CHECK(Count(kEcMoveAbs) == 2 && g_calls.back().op == kEcMoveAbs && g_calls.back().v == 1000.0);
        g_s.cmdPos = 1000.0;
        CHECK(MOT[ia].MotorMove(1000) == 1);                              // arrival for real
        MOT[ia].Motor = NULL;
    }

    SetEcatMotorRoute(0);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
