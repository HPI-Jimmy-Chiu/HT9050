// ===========================================================================
//  tests/test_fullhome_zsafe.cpp   (ctest: FullHomeZSafe_On, FullHomeZSafe_Default)
//
//  AI(W906-FULLHOME-ZSAFE) 20261005: NB2-1 R231 (laptop W-63) -- MachineType.h W906_HT9050_FULLHOME_ZSAFE, DEFAULT OFF.
//  TMyMotor::MotorHome (Motor/mymotor.cpp) on an HT9050 arm Z the drive homed (DS402, bW906HomeTrusted): with the switch ON it
//  goes on to ZSafePos before answering 1 (golden ProcessSingleMotorHome case 500's move, now for every engine home of such a
//  Z, the full HOME included); OFF = it answers 1 at case 20 as before. One source, two executables:
//    test_fullhome_zsafe_on       compiles Motor/mymotor.cpp ITSELF with "W906_HT9050_FULLHOME_ZSAFE=" (that target only)
//    test_fullhome_zsafe_default  links ht9045_motor's mymotor.cpp as every build does
//    1  MInArmZA, DS402 home, HT9050 rule installed: ON -> HomeFlag 0, MoveAbs(ZSafePos) through the route, 1 only after the
//       axis reads there; default -> 1 at case 20, no move
//    2  not an arm Z (MInArmX) -> 1 at case 20 in both builds
//    3  no HT9050 rule installed (the hook absent = -2: not wb_serve on a 9050GPIB machine) -> 1 at case 20 in both builds
//    4  a card-side home (not trusted; the lamp decides) -> 1 at case 20 in both builds, no move
//    5  ON: a door opened during the move -> MotorHome waits (golden's door check at its top), then finishes
//  FAKE engine route (Motor/EcatMotorRoute.h, test_ecat_motor_route's shape). Memory only: no card, no file, no thread;
//  ZSafePos, the hook and the route slot restored before return.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "mysensor.h"
#include "MachineType.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"
#include "Motor/EcatMotorRoute.h"

#include <cstdio>
#include <map>
#include <vector>
#include <windows.h>

#if defined(W906_HT9050_FULLHOME_ZSAFE)
static const bool kOn = true;
#else
static const bool kOn = false;
#endif

extern bool bAlarm;            // Motor/myEthercatmotor.cpp file-global (golden :44)
extern int  ZSafePos;          // Motor/mymotor.cpp:81

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_fullhome_zsafe.cpp:%d] (%s)  %s\n", line, kOn ? "switch ON" : "default", e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
//  The fake route (test_ecat_motor_route's shape): one sample, every call recorded
// ---------------------------------------------------------------------------
struct Rec { int op; double v; };
static std::vector<Rec> g_calls;
static TEcatAxisRead g_s;
static int  g_sAddr = -1;
static int  g_homeDoneRet = 0;
static bool g_cardSide = false;
static bool FBind(int, int, int, bool dir) { return !dir; }
static bool FRead(int b, int p, TEcatAxisRead* out) { if (b * 100 + p != g_sAddr) return false; *out = g_s; return true; }
static unsigned long FCall(int, int, int op, int, double v) { Rec r = { op, v }; g_calls.push_back(r); return 0ul; }
static bool FInit(int, int, int, bool, bool, double, double, double) { return true; }
static int  FHomeStart(int, int, bool, double, double, double, double) { return 1; }
static int  FHomeDone(int, int) { return g_homeDoneRet; }
static bool FCardSide(int, int) { return g_cardSide; }
static const TEcatMotorRoute kFake = { FBind, FRead, FCall, FInit, FHomeStart, FHomeDone, FCardSide, 0 };

enum { kStaPtpMot = 5 };                                // STA_AX_PTP_MOT (test_s26_r1_zsafe.cpp uses the same value)
static int Count(int op) { int n = 0; for (std::size_t i = 0; i < g_calls.size(); ++i) if (g_calls[i].op == op) ++n; return n; }
static double Last(int op) { double v = -1.0e300; for (std::size_t i = 0; i < g_calls.size(); ++i) if (g_calls[i].op == op) v = g_calls[i].v; return v; }

struct EcatPeek : TMyEtherCatMotor {
    explicit EcatPeek(int a) : TMyEtherCatMotor(a) {}
    void SetISpeed(unsigned v) { iSpeed = v; }   // HTMotor's ctor leaves iSpeed unset (golden: OldSpeed=iSpeed reads it)
};
static bool g_door = false;
static bool Door() { return g_door; }
static const int kEmg[4] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG, SnRearRightEMG };
static void EmgReleased() { for (int i = 0; i < 4; ++i) Sen[kEmg[i]].Enable = false; }

static void Setup(EcatPeek& m)
{
    m.SetISpeed(0);
    m.Enable = true; m.Direction = false; m.HomeDirection = true; m.GearRatio = 1.0;
    m.PJogHighSpeed = 100000; m.PJogLowSpeed = 100; m.InitSpeed = 1000;
    m.PHomeHighSpeed = 20000; m.PHomeLowSpeed = 2000;
    m.MotorType = Servo_Motor; m.bSensorType = false; m.bIn1Logic = true; m.PServoAlarmOn = true;
    m.SetAccDataBase(500000.0); m.SetDecDataBase(400000.0); m.SetAcc(500000.0); m.SetDec(400000.0);
    m.PSoftLimitP = 999999; m.PSoftLimitN = -999999; m.SetRange(100);
    m.MotorIdleSafeDoorCheck = Door;
}

static int g_hook = -2;                                 // what the HT9050 origin rule answers (-2 = not installed)
static int Hook(int) { return g_hook; }

// Drive t.MotorHome to the call where TMyMotor's case 20 runs; returns that call's answer.
static int HomeToCase20(TMyMotor& t, int addr, bool cardSide)
{
    g_calls.clear(); g_s = TEcatAxisRead(); g_s.valid = true; g_s.state = kEcStaReady; g_s.motionIO = (1ul << 14);   // SVON, no ORG
    g_sAddr = addr; g_homeDoneRet = 0; g_cardSide = cardSide;
    t.MotorInitial();
    t.MotorHome(false);                                 // case 1 -> HomeReset, Task 10
    t.MotorHome(false);                                 // case 10: Home() -> the route home starts
    g_homeDoneRet = 1;
    t.MotorHome(false);                                 // the route reports done -> HomeDelay 0.3 s
    ::Sleep(350);
    t.MotorHome(false);                                 // HomeObject true -> TMyMotor Task 20
    return t.MotorHome(false);                          // case 20
}

int main()
{
    const int oldZSafe = ZSafePos;
    const bool oldAlarm = bAlarm;
    ZSafePos = 50; bAlarm = false;
    SetEcatMotorRoute(&kFake);
    EmgReleased();
    W906_Ht9050OrgHomeHook = Hook;

    // ==================== 1: MInArmZA, DS402 home, HT9050 rule installed ====================
    {
        EcatPeek m(301); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = MInArmZA; t.CardType = "PCI1203";
        g_hook = 0;                                     // HT9050 rule: not at the origin (the DS402 edge, lamp off)
        const int r = HomeToCase20(t, 301, false);
        if (kOn) {
            CHECK(r == 0 && t.HomeFlag == 0);           // not done yet: goes on to ZSafePos
            g_calls.clear();
            CHECK(t.MotorHome(false) == 0);             // Task 40: MotorMove(ZSafePos) issued
            CHECK(Count(kEcMoveAbs) == 1 && Last(kEcMoveAbs) == 50.0 && t.HomeFlag == 0);
            g_s.state = kStaPtpMot; g_s.cmdPos = 20.0; g_s.actPos = 18.0;   // the card reports the move under way
            CHECK(t.MotorHome(false) == 0 && t.HomeFlag == 0);   // not there yet
            g_s.state = kEcStaReady; g_s.cmdPos = 50.0; g_s.actPos = 50.0; g_hook = 1;   // there; the lamp is lit at 50 on HT9050
            CHECK(t.MotorHome(false) == 1 && t.HomeFlag == 1);
            CHECK(t.MotorHome(false) == 0);             // a new call starts a new home (Task 1), as golden
        } else {
            CHECK(r == 1 && t.HomeFlag == 1);           // as before: done at case 20
            CHECK(Count(kEcMoveAbs) == 0);
        }
    }
    // ==================== 2: not an arm Z ====================
    {
        EcatPeek m(302); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = MInArmX; t.CardType = "PCI1203";
        g_hook = 0;
        CHECK(HomeToCase20(t, 302, false) == 1 && t.HomeFlag == 1 && Count(kEcMoveAbs) == 0);
    }
    // ==================== 3: no HT9050 rule installed ====================
    {
        EcatPeek m(303); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = MOutArmZA; t.CardType = "PCI1203";
        g_hook = -2;
        CHECK(HomeToCase20(t, 303, false) == 1 && t.HomeFlag == 1 && Count(kEcMoveAbs) == 0);
    }
    // ==================== 4: card-side home: not trusted, the lamp decides ====================
    {
        EcatPeek m(304); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = MOutArmZA; t.CardType = "PCI1203";
        g_hook = 1;                                     // lamp lit
        CHECK(HomeToCase20(t, 304, true) == 1 && t.HomeFlag == 1 && Count(kEcMoveAbs) == 0);
    }
    // ==================== 5: ON -- a door opened during the move ====================
    if (kOn) {
        EcatPeek m(305); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = MOutArmZA; t.CardType = "PCI1203";
        g_hook = 0;
        CHECK(HomeToCase20(t, 305, false) == 0);
        g_door = true;
        CHECK(t.MotorHome(false) == 0 && t.HomeFlag == 0); // golden: MotorHome returns 0 while the door is open
        g_door = false;
        CHECK(t.MotorHome(false) == 0 && Last(kEcMoveAbs) == 50.0);
        g_s.state = kStaPtpMot;
        CHECK(t.MotorHome(false) == 0 && t.HomeFlag == 0);
        g_s.state = kEcStaReady; g_s.cmdPos = 50.0; g_s.actPos = 50.0;
        CHECK(t.MotorHome(false) == 1 && t.HomeFlag == 1);
    }

    W906_Ht9050OrgHomeHook = 0;
    SetEcatMotorRoute(0);
    ZSafePos = oldZSafe; bAlarm = oldAlarm;
    std::printf("%s (%s): %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", kOn ? "switch ON" : "default", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
