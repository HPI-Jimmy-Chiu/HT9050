// ===========================================================================
//  tests/test_s26_r1_zsafe.cpp   (ctest: S26_R1_ZSafe_On, S26_R1_ZSafe_Default)
//
//  AI(W906-S26-R1) 20261004: S-26 Appendix A R1 (St01 K4, docs/handoff/S26_FINDINGS_20261004.md) -- "Z at its
//  origin" in InArmZSafe / OutArmZSafe (iFlag & 1) and CheckInArmZNeedHome / CheckOutArmZNeedHome (Motor/mymotor.cpp,
//  golden mymotor.cpp:2127-2323) behind MachineType.h W906_DS402_ZSAFE_HOMEFLAG, DEFAULT OFF. One source, two
//  executables:
//    test_s26_r1_zsafe_on       compiles Motor/mymotor.cpp ITSELF with the switch ON (that target's compile definition
//                               "W906_DS402_ZSAFE_HOMEFLAG="); the libraries stay as built
//    test_s26_r1_zsafe_default  links ht9045_motor's mymotor.cpp as every build does: the lamp only, as golden
//  Every case states both answers. They differ only where the rule applies: the lamp is off and the Z is a 1203 row,
//  fAllMotorHome, HomeFlag 1, its sample READY and not pending with SVON on and no ALM / EMG, its last route home not
//  card-side, |command| <= 1 pulse and |encoder| <= 1 pulse.
//    1  lamp on                         -> at origin in both builds (golden)
//    2  the rule                        -> ON: at origin; default: the Z motor (lamp only)
//    3  |command| <= 1 boundaries; Z down
//    4  DETECT_ALL_FLAG: golden's position half (ReadPos() < 0) still answers
//    5  each condition of the rule alone breaks it (HomeFlag 0 / 2, pending, moving, ERROR_STOP, DISABLE,
//       card-side home, not PCI1203, no sample, no route)
//    6  HT9050's lamp (W906_Ht9050OrgHome hook) still feeds the lamp half
//    7  CheckIn/OutArmZNeedHome only look while the command bookkeeping is at the safe height (golden)
//    8  a disabled Z is skipped (golden)
//    9  AI(W906-S26-R1-REVIEW) 20261005, laptop review R1-1: fAllMotorHome false (a power drop / EMG / alarm since the
//       full HOME -- HomeFlag survives those), servo off, ALM with READY, EMG, command 0 with the encoder at 5000 / 1.5,
//       each alone -> the lamp only; the encoder's own +-1 boundaries; the lamp still wins with fAllMotorHome false
//  FAKE engine route (Motor/EcatMotorRoute.h): one sample and one "last home card-side" per (board, port). Memory only:
//  no card, no file, no thread; MOT[] / InArmSuck / OutArmSuck / Prod / the picker globals / fAllMotorHome restored before return.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "cprod.h"
#include "MachineType.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"
#include "Motor/EcatMotorRoute.h"
#include "aHotPlateSubstrate.h"

#include <cstdio>
#include <map>

#if defined(W906_DS402_ZSAFE_HOMEFLAG)
static const bool kOn = true;
#else
static const bool kOn = false;
#endif

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_s26_r1_zsafe.cpp:%d] (%s)  %s\n", line, kOn ? "switch ON" : "default", e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
//  The fake route
// ---------------------------------------------------------------------------
static std::map<int, TEcatAxisRead> g_smp;
static std::map<int, bool> g_cardSide;
static bool FBind(int, int, int, bool dir) { return !dir; }
static bool FRead(int b, int p, TEcatAxisRead* out)
{
    std::map<int, TEcatAxisRead>::const_iterator it = g_smp.find(b * 100 + p);
    if (it == g_smp.end()) return false;
    *out = it->second;
    return true;
}
static unsigned long FCall(int, int, int, int, double) { return 0ul; }
static bool FCardSide(int b, int p)
{
    std::map<int, bool>::const_iterator it = g_cardSide.find(b * 100 + p);
    return it != g_cardSide.end() && it->second;
}
static const TEcatMotorRoute kFake = { FBind, FRead, FCall, 0, 0, 0, FCardSide, 0 };

static const unsigned long kSvon = 0x00004000ul;   // AX_MOTION_IO_SVON
static const unsigned long kOrg  = 0x00000010ul;   // AX_MOTION_IO_ORG: the lamp (no HT9050 hook installed)
static const unsigned long kAlm  = 0x00000002ul;   // AX_MOTION_IO_ALM  (case 9)
static const unsigned long kEmg  = 0x00000040ul;   // AX_MOTION_IO_EMG  (case 9)
enum { kStaDisable = 0, kStaPtpMot = 5 };

static TEcatAxisRead Smp(unsigned short state, bool pending, double cmd, unsigned long io)
{
    TEcatAxisRead s;
    s.valid = true; s.state = state; s.pending = pending; s.cmdPos = cmd; s.actPos = cmd; s.motionIO = io;
    return s;
}

struct EcatPeek : TMyEtherCatMotor {
    explicit EcatPeek(int a) : TMyEtherCatMotor(a) {}
    void SetISpeed(unsigned v) { iSpeed = v; }
};
static bool DoorClosed() { return false; }
static void Setup(EcatPeek& m)
{
    m.SetISpeed(0);
    m.Enable = true; m.Direction = false; m.HomeDirection = true; m.GearRatio = 1.0;
    m.PJogHighSpeed = 100000; m.PJogLowSpeed = 100; m.InitSpeed = 1000;
    m.MotorType = Servo_Motor; m.bSensorType = true; m.bIn1Logic = true; m.PServoAlarmOn = false;   // M03 / M22: ServoAlarmOn 0
    m.PSoftLimitP = 999999; m.PSoftLimitN = -999999;
    m.MotorIdleSafeDoorCheck = DoorClosed;
}
static int HookNotHome(int) { return 0; }   // HT9050's ORG rule says "not at home"
static int HookHome(int)    { return 1; }   // ... "at home"

int main()
{
    std::printf("S-26 R1: W906_DS402_ZSAFE_HOMEFLAG %s\n", kOn ? "ON (this target only)" : "OFF (the default build)");
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;

    const int keepPicker = USE_PICKER_COUNT, keepPum = InOutArmPickerUseMotor;
    const int keepInRow = InArmSuck.iMotRow, keepInCol = InArmSuck.iMotCol, keepInMot = InArmSuck.Suck[0][0].iMotNo;
    const int keepOutRow = OutArmSuck.iMotRow, keepOutCol = OutArmSuck.iMotCol, keepOutMot = OutArmSuck.Suck[0][0].iMotNo;
    const int keepZin = Prod.ZInArmSafe[0][0], keepZout = Prod.ZOutArmSafe[0][0];
    int (*const keepHook)(int) = W906_Ht9050OrgHomeHook;
    const bool keepAllHome = fAllMotorHome;

    USE_PICKER_COUNT = ep1Picker; InOutArmPickerUseMotor = 0;                    // HT9050: one Z per arm (K1)
    InArmSuck.iMotRow = 1;  InArmSuck.iMotCol = 1;  InArmSuck.Suck[0][0].iMotNo = MInArmZA;
    OutArmSuck.iMotRow = 1; OutArmSuck.iMotCol = 1; OutArmSuck.Suck[0][0].iMotNo = MOutArmZA;
    Prod.ZInArmSafe[0][0] = 0; Prod.ZOutArmSafe[0][0] = 0;                       // teach.ini zeros (machine WORKLOG s1)
    W906_Ht9050OrgHomeHook = 0;

    EcatPeek zi(301), zo(2201);                                                  // M03 MInArmZA at (3, 1); M22 MOutArmZA at (22, 1)
    Setup(zi); Setup(zo);
    struct Row { int mot; int addr; EcatPeek* m; int (*zsafe)(int); int (*needHome)(); const char* name; };
    const Row rows[2] = { { MInArmZA, 301, &zi, InArmZSafe, CheckInArmZNeedHome, "InArm" },
                          { MOutArmZA, 2201, &zo, OutArmZSafe, CheckOutArmZNeedHome, "OutArm" } };
    for (int r = 0; r < 2; ++r) { MOT[rows[r].mot].Motor = rows[r].m; MOT[rows[r].mot].Mot_Name = rows[r].mot; }
    SetEcatMotorRoute(&kFake);

    for (int r = 0; r < 2; ++r) {
        const int mot = rows[r].mot, addr = rows[r].addr;
        int (*zsafe)(int) = rows[r].zsafe;
        int (*needHome)() = rows[r].needHome;
        TMyMotor& M = MOT[mot];
        const int rule = kOn ? -1 : mot;                                         // the answer where the two builds differ
        auto reset = [&]() {
            M.HomeFlag = 1; M.CardType = "PCI1203"; M.Position = 0;
            fAllMotorHome = true;                                                // a full HOME ended, nothing voided it (case 9)
            g_cardSide[addr] = false;
            g_smp[addr] = Smp(kEcStaReady, false, 0.0, kSvon);                   // lamp OFF, READY, not pending, command 0
            W906_Ht9050OrgHomeHook = 0;
            SetEcatMotorRoute(&kFake);
        };
        std::printf("-- %s Z (M%02d)\n", rows[r].name, mot);

        // 1 lamp on: golden, both builds
        reset(); g_smp[addr].motionIO = kSvon | kOrg;
        CHECK(zsafe(DETECT_SENSOR_FLAG) == -1);
        CHECK(zsafe(DETECT_ALL_FLAG) == -1);
        CHECK(needHome() == -1);

        // 2 the rule
        reset(); CHECK(zsafe(DETECT_SENSOR_FLAG) == rule);
        reset(); CHECK(zsafe(DETECT_ALL_FLAG) == rule);
        reset(); CHECK(needHome() == rule);

        // 3 |command| <= 1 pulse
        reset(); g_smp[addr].cmdPos = 1.0;    CHECK(zsafe(DETECT_SENSOR_FLAG) == rule);
        reset(); g_smp[addr].cmdPos = -1.0;   CHECK(zsafe(DETECT_SENSOR_FLAG) == rule);
        reset(); g_smp[addr].cmdPos = 1.5;    CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].cmdPos = -2.0;   CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].cmdPos = 5000.0; CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // Z down

        // 4 golden's position half: command -1 passes the lamp half under the rule, but ReadPos() < 0
        reset(); g_smp[addr].cmdPos = -1.0;   CHECK(zsafe(DETECT_ALL_FLAG) == mot);

        // 5 every condition alone
        reset(); M.HomeFlag = 0;                      CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); M.HomeFlag = 2;                      CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].pending = true;          CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].state = kStaPtpMot;      CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].state = kEcStaErrorStop; CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].state = kStaDisable;     CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_cardSide[addr] = true;             CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); M.CardType = "SMC";                  CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp.erase(addr);                   CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); SetEcatMotorRoute(0);                CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // every SIM build
        reset(); M.HomeFlag = 0;                      CHECK(needHome() == mot);
        reset(); g_cardSide[addr] = true;             CHECK(needHome() == mot);

        // 6 HT9050's ORG rule is the lamp (TMyMotor::ScanMotorStatus applies the hook after the card decode)
        reset(); g_smp[addr].motionIO = kSvon | kOrg; W906_Ht9050OrgHomeHook = HookNotHome;
        CHECK(zsafe(DETECT_SENSOR_FLAG) == rule);                                // the hook says off, the rule decides
        reset(); g_smp[addr].cmdPos = 5000.0; W906_Ht9050OrgHomeHook = HookHome;
        CHECK(zsafe(DETECT_SENSOR_FLAG) == -1);                                  // the hook says home: golden, whatever the command

        // 7 the need-home check only looks while the command bookkeeping is at the safe height
        reset(); M.Position = 300; CHECK(needHome() == -1);

        // 8 a disabled Z is skipped
        reset(); rows[r].m->Enable = false;
        CHECK(zsafe(DETECT_SENSOR_FLAG) == -1 && needHome() == -1);
        rows[r].m->Enable = true;

        // 9 laptop review R1-1: HomeFlag + command 0 is not enough -- each new condition alone keeps golden's lamp
        reset(); fAllMotorHome = false;                       CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // power drop / EMG / alarm since the HOME
        reset(); fAllMotorHome = false;                       CHECK(zsafe(DETECT_ALL_FLAG) == mot);
        reset(); fAllMotorHome = false;                       CHECK(needHome() == mot);
        reset(); fAllMotorHome = false; g_smp[addr].motionIO = kSvon | kOrg;
        CHECK(zsafe(DETECT_SENSOR_FLAG) == -1 && needHome() == -1);                                     // the lamp itself: golden, untouched
        reset(); g_smp[addr].motionIO = 0;                    CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // servo off
        reset(); g_smp[addr].motionIO = kSvon | kAlm;         CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // drive alarm while the card still reads READY
        reset(); g_smp[addr].motionIO = kSvon | kEmg;         CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // EMG
        reset(); g_smp[addr].motionIO = 0;                    CHECK(needHome() == mot);
        reset(); g_smp[addr].actPos = 5000.0;                 CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);   // command 0, encoder 5000: moved while servo-off / slipped
        reset(); g_smp[addr].actPos = 5000.0;                 CHECK(needHome() == mot);
        reset(); g_smp[addr].actPos = 1.5;                    CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].actPos = -2.0;                   CHECK(zsafe(DETECT_SENSOR_FLAG) == mot);
        reset(); g_smp[addr].actPos = 1.0;                    CHECK(zsafe(DETECT_SENSOR_FLAG) == rule);  // the encoder's boundaries: within 1 pulse
        reset(); g_smp[addr].actPos = -1.0;                   CHECK(zsafe(DETECT_SENSOR_FLAG) == rule);
    }

    SetEcatMotorRoute(0);
    for (int r = 0; r < 2; ++r) MOT[rows[r].mot].Motor = NULL;
    W906_Ht9050OrgHomeHook = keepHook;
    fAllMotorHome = keepAllHome;
    Prod.ZInArmSafe[0][0] = keepZin; Prod.ZOutArmSafe[0][0] = keepZout;
    InArmSuck.iMotRow = keepInRow;   InArmSuck.iMotCol = keepInCol;   InArmSuck.Suck[0][0].iMotNo = keepInMot;
    OutArmSuck.iMotRow = keepOutRow; OutArmSuck.iMotCol = keepOutCol; OutArmSuck.Suck[0][0].iMotNo = keepOutMot;
    USE_PICKER_COUNT = keepPicker; InOutArmPickerUseMotor = keepPum;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
