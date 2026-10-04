// ===========================================================================
//  tests/test_s26_r3_inpos.cpp   (ctest: S26_R3_InPos)
//
//  AI(W906-S26-R3) 20261004: S-26 Appendix A R3 (St01, docs/handoff/S26_FINDINGS_20261004.md) -- the
//  in-position lamp Led[iInposLed] of a 1203 engine axis. Golden never writes it on the 1203 (its own INP
//  decode is commented out "not work", Motor/myEthercatmotor.cpp:1180 = golden :874), so "the tray arm is
//  moving" was always false and IsTrayArmMoveAvoidOutArmCrash (acatchtray.cpp, golden :7979) never held OutArm
//  X/Y while MTrayX crossed Empty..Color. TMyEtherCatMotor::ScanMotorStatus's route arm now writes golden's
//  meaning ("NOT in position", SMC / MN200 card classes) from the monitor sample: true unless the sample is
//  READY and not pending.
//    A  decode: READY + not pending -> false; every other STA_AX_* state -> true; pending READY -> true;
//       the INP bit (motionIO bit 13) is NOT what decides; failed read / no route / Enable=0 as before
//    B  the consumer: IsTrayArmMoveAvoidOutArmCrash -- MTrayX at/after Empty and moving (or a command not
//       yet seen by a poll, or stopped in a fault) -> true; standing, before Empty, under-conveyor -> false
//    C  the caller that holds OutArm: SetOutArm_9045 (aoutarm9045.cpp, golden :2497) returns false and stops
//       OutArm X and Y (StopDec through the route, golden PCIL132_StopMotor's fCMD=false) while it holds
//    D  MotorMovePosition with ServoAlarmOn=1 (golden mymotor.cpp :723-724 re-check after MotionDone) still
//       arrives on the first READY, non-pending sample: the lamp adds no new wait
//  Memory only: a FAKE engine motor route (Motor/EcatMotorRoute.h) serving one sample per (board, port); MOT[]
//  rows point at stack TMyEtherCatMotor objects and are reset before return. No card, no file, no thread.
//  Red on the tree before the fix: A's moving states, B's "moving -> true" and all of C fail.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "cprod.h"
#include "MachineType.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"
#include "Motor/EcatMotorRoute.h"
#include "acatchtray.h"
#include "aoutarm9045.h"

#include <cstdio>
#include <map>
#include <vector>

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_s26_r3_inpos.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
//  The fake route: one sample per (board * 100 + port); every call recorded
// ---------------------------------------------------------------------------
struct Rec { int b, p, op; };
static std::vector<Rec> g_calls;
static std::map<int, TEcatAxisRead> g_smp;
static bool FBind(int, int, int, bool dir) { return !dir; }
static bool FRead(int b, int p, TEcatAxisRead* out)
{
    std::map<int, TEcatAxisRead>::const_iterator it = g_smp.find(b * 100 + p);
    if (it == g_smp.end()) return false;
    *out = it->second;
    return true;
}
static unsigned long FCall(int b, int p, int op, int, double) { Rec r = { b, p, op }; g_calls.push_back(r); return 0ul; }
static const TEcatMotorRoute kFake = { FBind, FRead, FCall, 0, 0, 0, 0, 0 };

static TEcatAxisRead Smp(unsigned short state, bool pending, double pos, unsigned long io)
{
    TEcatAxisRead s;
    s.valid = true; s.state = state; s.pending = pending; s.cmdPos = pos; s.actPos = pos; s.motionIO = io;
    return s;
}
static int StopsAt(int addr)
{
    int n = 0;
    for (std::size_t k = 0; k < g_calls.size(); ++k)
        if (g_calls[k].op == kEcStopDec && g_calls[k].b * 100 + g_calls[k].p == addr) ++n;
    return n;
}

struct EcatPeek : TMyEtherCatMotor {
    explicit EcatPeek(int a) : TMyEtherCatMotor(a) {}
    void SetISpeed(unsigned v) { iSpeed = v; }   // HTMotor's ctor leaves iSpeed unset
};
static bool DoorClosed() { return false; }
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

static const unsigned long kSvon = 0x00004000ul;   // AX_MOTION_IO_SVON
static const unsigned long kInp  = 0x00002000ul;   // AX_MOTION_IO_INP (bit 13, the bit golden decoded and disabled)
enum { kStaPtpMot = 5 };                           // STA_AX_PTP_MOT

int main()
{
    for (int i = 0; i < TOTAL_MOTOR; ++i) MOT[i].Motor = NULL;
    const int keepMode = TRAY_ARM_MODE;
    const int keepEmpty = Prod.iXTrayEmpty;

    // ==================== A: decode ====================
    SetEcatMotorRoute(&kFake);
    {
        EcatPeek m(2101); Setup(m);
        bool Led[10];
        g_smp[2101] = Smp(kEcStaReady, false, 0.0, kSvon);
        for (int i = 0; i < 10; ++i) Led[i] = true;
        m.ScanMotorStatus(Led);
        CHECK(Led[iInposLed] == false);                                 // READY, not pending = in position
        CHECK(Led[iServoOn] && !Led[iAlarmLed]);                         // the other lamps decode as before
        // every other STA_AX_* state (AdvMotDrv.h:792-807): DISABLE, STOPPING, ERROR_STOP, HOMING, PTP, CONTI, SYNC, EXT_JOG,
        // EXT_MPG, PAUSE, BUSY, WAIT_DI, WAIT_PTP, WAIT_VEL, EXT_JOG_READY -> NOT in position
        const unsigned short states[15] = { 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        int notIn = 0;
        for (int k = 0; k < 15; ++k) {
            g_smp[2101] = Smp(states[k], false, 0.0, kSvon);
            Led[iInposLed] = false;
            m.ScanMotorStatus(Led);
            if (Led[iInposLed]) ++notIn;
        }
        CHECK(notIn == 15);
        g_smp[2101] = Smp(kEcStaReady, true, 0.0, kSvon);               // a command the monitor has not polled yet (or Q11)
        Led[iInposLed] = false; m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == true);
        // the INP bit does not decide (golden's decode of it was disabled; never measured on this machine)
        g_smp[2101] = Smp(kEcStaReady, false, 0.0, kSvon);              // READY without INP -> in position
        Led[iInposLed] = true; m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == false);
        g_smp[2101] = Smp(kStaPtpMot, false, 0.0, kSvon | kInp);        // moving with INP -> NOT in position
        Led[iInposLed] = false; m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == true);
        // a failed read returns before any lamp (golden's WAR16121 return): untouched either way
        g_smp.erase(2101);
        Led[iInposLed] = true;  m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == true);
        Led[iInposLed] = false; m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == false);
        // Enable=0: golden's disabled branch (Led[iHomeLed]=true, Led[iInposLed]=false)
        g_smp[2101] = Smp(kStaPtpMot, false, 0.0, kSvon);
        m.Enable = false; Led[iInposLed] = true; Led[iHomeLed] = false;
        m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == false && Led[iHomeLed] == true);
        m.Enable = true;
        // no route installed: the 8 lamps the old arm cleared, Led[iInposLed] untouched (as before)
        SetEcatMotorRoute(0);
        Led[iInposLed] = true; m.ScanMotorStatus(Led); CHECK(Led[iInposLed] == true && Led[iServoOn] == false);
        SetEcatMotorRoute(&kFake);
    }

    // ==================== B: IsTrayArmMoveAvoidOutArmCrash ====================
    g_smp.clear(); g_calls.clear();
    EcatPeek tray(3001), ox(1901), oy(2001);
    Setup(tray); Setup(ox); Setup(oy);
    MOT[MTrayX].Motor = &tray;   MOT[MTrayX].Mot_Name = MTrayX;
    MOT[MOutArmX].Motor = &ox;   MOT[MOutArmX].Mot_Name = MOutArmX;
    MOT[MOutArmY].Motor = &oy;   MOT[MOutArmY].Mot_Name = MOutArmY;
    g_smp[1901] = Smp(kEcStaReady, false, 0.0, kSvon);
    g_smp[2001] = Smp(kEcStaReady, false, 0.0, kSvon);
    TRAY_ARM_MODE = eAboveCoveyor;
    Prod.iXTrayEmpty = 5000;
    {
        g_smp[3001] = Smp(kStaPtpMot, false, 6000.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == true);                 // beyond Empty and moving: hold OutArm (red before the fix)
        g_smp[3001] = Smp(kStaPtpMot, false, 5000.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == true);                 // at Empty exactly (golden >=)
        g_smp[3001] = Smp(kEcStaReady, true, 6000.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == true);                 // a move the monitor has not polled yet
        g_smp[3001] = Smp(kEcStaErrorStop, false, 6000.0, kSvon | 0x2ul);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == true);                 // stopped short in a fault inside the zone: still held
        g_smp[3001] = Smp(kEcStaReady, false, 6000.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == false);                // standing beyond Empty: OutArm may move (golden)
        g_smp[3001] = Smp(kStaPtpMot, false, 4999.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == false);                // moving before Empty: not this interlock
        TRAY_ARM_MODE = eUnderCoveyor;
        g_smp[3001] = Smp(kStaPtpMot, false, 6000.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == false);                // under the conveyor: golden returns false first
        TRAY_ARM_MODE = eAboveCoveyor;
    }

    // ==================== C: SetOutArm_9045 holds OutArm X / Y ====================
    {
        g_smp[3001] = Smp(kStaPtpMot, false, 6000.0, kSvon);
        g_calls.clear();
        MOT[MOutArmX].fCMD = true; MOT[MOutArmY].fCMD = true;
        CHECK(SetOutArm_9045() == false);
        CHECK(StopsAt(1901) == 1 && StopsAt(2001) == 1);                 // golden :2499-2500 PCIL132_StopMotor on both
        CHECK(StopsAt(3001) == 0);                                       // the tray arm itself is not stopped
        CHECK(!MOT[MOutArmX].fCMD && !MOT[MOutArmY].fCMD);               // so the next MotorMove re-issues (golden bookkeeping)
        g_smp[3001] = Smp(kEcStaReady, false, 6000.0, kSvon);
        CHECK(IsTrayArmMoveAvoidOutArmCrash() == false);                 // tray arm READY: the hold is released (from the next pass)
    }

    // ==================== D: MotorMovePosition (ServoAlarmOn=1) arrives as before ====================
    {
        EcatPeek a(2201); Setup(a); a.PServoAlarmOn = true;
        const int ia = 2;                                                // a row with no special case in MotorMovePosition
        MOT[ia].Motor = &a; MOT[ia].Mot_Name = ia;
        MOT[ia].fCMD = false; MOT[ia].iOldPos = 0;
        g_smp[2201] = Smp(kEcStaReady, false, 0.0, kSvon);
        g_calls.clear();
        CHECK(MOT[ia].MotorMove(1000) == 0);                             // MoveAbs(1000) sent, fCMD=true
        CHECK(MOT[ia].fCMD);
        g_smp[2201] = Smp(kStaPtpMot, false, 500.0, kSvon);
        CHECK(MOT[ia].MotorMove(1000) == 0);                             // still moving
        g_smp[2201] = Smp(kEcStaReady, false, 1000.0, kSvon);
        CHECK(MOT[ia].MotorMove(1000) == 1);                             // the first READY, non-pending sample = arrival
        CHECK(MOT[ia].Led[iInposLed] == false);                          // the re-check (golden :723-724) saw "in position"
        MOT[ia].Motor = NULL;
    }

    MOT[MTrayX].Motor = NULL; MOT[MOutArmX].Motor = NULL; MOT[MOutArmY].Motor = NULL;
    TRAY_ARM_MODE = keepMode;
    Prod.iXTrayEmpty = keepEmpty;
    SetEcatMotorRoute(0);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
