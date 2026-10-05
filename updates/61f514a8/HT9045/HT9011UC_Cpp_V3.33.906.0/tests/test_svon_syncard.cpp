// ===========================================================================
//  tests/test_svon_syncard.cpp   (ctest: SvonSyncCard)
//
//  AI(W906-SVON-SYNCARD) 20261005: NB2-1 R230 (laptop W-60; St01 W-47, docs/handoff/ST01_W_ANSWERS_20261004.md) --
//  TMyMotor::PCIL132_ResetPos (Motor/mymotor.cpp, golden mymotor.cpp:1838-1856) on a PCIE-1203 row. Golden picks its arm
//  by the GLOBAL MOTION_CARD_TYPE: SYN -> Motor->ResetPos() (TMySYNTEKMotor ignores the argument and copies the encoder
//  into the command, golden mySYNTEKmotor.cpp:673-681), otherwise ResetPos(encoder). Both arms mean "command := encoder".
//  A 1203 row is a TMyEtherCatMotor whose ResetPos(p) WRITES p, so on HT9050 (Gerneral.ini MOTION_CARD_TYPE=0) the
//  servo-ON sync wrote 0 and the route refused it (Q1). Now a 1203 row takes the encoder arm under any card type; every
//  other class keeps golden's dispatch.
//    1  HT9050 shape (SYN, 1203, PServoAlarmOn): ServoOnOff(true) = SvOn, then SetCmdPos / SetActPos with the encoder; the
//       fake route answers coordinate writes like the real one on a DS402 drive (Q1: accepted only within 1 pulse of the
//       encoder) and refuses none; negative / fractional / zero encoders; PCIL132_ResetPos alone; the old arm's
//       ResetPos() (0) is what the route refused
//    2  Contec: the same (golden's own encoder arm, unchanged)
//    3  golden's gates kept: PServoAlarmOn=0 -> SvOn only; ServoOnOff(false) -> SvOn 0 only; Enable=false -> nothing
//    4  another class keeps golden: SYN -> ResetPos() (argument 0); Contec -> ResetPos(encoder), negated with Direction
//    5  W906_Is1203Motor: NULL / TMySYNTEKMotor / another HTMotor -> false; TMyEtherCatMotor -> true
//    6  no route installed (every SIM build): nothing sent, nothing breaks
//  FAKE engine route (Motor/EcatMotorRoute.h, test_ecat_motor_route's shape). Memory only: no card, no file, no thread;
//  MOTION_CARD_TYPE, bAlarm and the route slot restored before return.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "cmydef.h"
#include "mysensor.h"
#include "Motor/mymotor.h"
#include "Motor/myEthercatmotor.h"
#include "Motor/mySYNTEKmotor.h"
#include "Motor/EcatMotorRoute.h"

#include <cmath>
#include <cstdio>
#include <vector>

extern bool bAlarm;                                     // Motor/myEthercatmotor.cpp file-global (golden :44)
bool W906_Is1203Motor(const HTMotor* m);                // Motor/mymotor.cpp, end of file

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_svon_syncard.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// ---------------------------------------------------------------------------
//  The fake route: one axis (board 22, port 1), one sample. A coordinate write is answered like
//  EtherCAT/Pci1203MotorRoute.cpp Q1 on a DS402 drive: accepted only within 1 pulse of the drive's encoder.
// ---------------------------------------------------------------------------
struct Rec { int op; double v; unsigned long rc; };
static std::vector<Rec> g_calls;
static TEcatAxisRead g_s;
static const int kAddr = 2201;
static bool FBind(int, int, int, bool dir) { return !dir; }
static bool FRead(int b, int p, TEcatAxisRead* out) { if (b * 100 + p != kAddr) return false; *out = g_s; return true; }
static unsigned long FCall(int, int, int op, int, double v)
{
    unsigned long rc = 0ul;
    if ((op == kEcSetCmdPos || op == kEcSetActPos) && std::fabs(v - g_s.actPos) > 1.0) rc = (unsigned long)kEcRcDs402Coord;
    Rec r = { op, v, rc };
    g_calls.push_back(r);
    return rc;
}
static const TEcatMotorRoute kFake = { FBind, FRead, FCall, 0, 0, 0, 0, 0 };

static void Fresh(double act, double cmd)   // the drive's encoder and the card's (stale) command
{
    g_calls.clear();
    g_s = TEcatAxisRead(); g_s.valid = true; g_s.state = kEcStaReady; g_s.actPos = act; g_s.cmdPos = cmd;
}
static int Count(int op) { int n = 0; for (std::size_t i = 0; i < g_calls.size(); ++i) if (g_calls[i].op == op) ++n; return n; }
static int Index(int op) { for (std::size_t i = 0; i < g_calls.size(); ++i) if (g_calls[i].op == op) return (int)i; return -1; }
static double Value(int op) { const int i = Index(op); return i < 0 ? -1.0e300 : g_calls[i].v; }
static int Refused() { int n = 0; for (std::size_t i = 0; i < g_calls.size(); ++i) if (g_calls[i].rc != 0ul) ++n; return n; }

static void Setup(TMyEtherCatMotor& m)
{
    m.Enable = true; m.Direction = false; m.MotorType = Servo_Motor; m.PServoAlarmOn = true; m.GearRatio = 1.0;
}

// another motor class: records ResetPos's argument, serves a fixed encoder
struct RecMotor : HTMotor {
    int resets = 0, lastReset = -999999, enc = 777;
    bool ResetPos(int Pulse = 0) override { ++resets; lastReset = Pulse; return true; }
    int  ReadEnCoderRealPos() override { return enc; }
};

int main()
{
    const int oldCard = MOTION_CARD_TYPE;
    const bool oldAlarm = bAlarm;
    bAlarm = false;
    SetEcatMotorRoute(&kFake);

    // ==================== 1: HT9050 shape -- SYN card type, a 1203 row ====================
    MOTION_CARD_TYPE = MotionCard_SYN;
    {
        TMyEtherCatMotor m(kAddr); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";

        Fresh(12345.0, 0.0);                                               // moved by hand while servo-off: command still 0
        t.ServoOnOff(true);
        CHECK(Count(kEcSvOn) == 1 && Value(kEcSvOn) == 1.0);
        CHECK(Count(kEcSetCmdPos) == 1 && Value(kEcSetCmdPos) == 12345.0);   // before R230: 0.0, refused by Q1
        CHECK(Count(kEcSetActPos) == 1 && Value(kEcSetActPos) == 12345.0);   // golden ResetPos(p) = SetCommand(p) + SetPosition(p)
        CHECK(Index(kEcSvOn) < Index(kEcSetCmdPos) && Index(kEcSetCmdPos) < Index(kEcSetActPos));   // golden: SetServoOn, then ResetPos
        CHECK(Refused() == 0);

        Fresh(-2500.0, 0.0);                                               // PCIL132_ResetPos alone (uhome / csystem call ServoOnOff)
        t.PCIL132_ResetPos();
        CHECK(Count(kEcSvOn) == 0 && Value(kEcSetCmdPos) == -2500.0 && Value(kEcSetActPos) == -2500.0 && Refused() == 0);

        Fresh(8000.6, 0.0);                                                // golden `int p`: truncated, still within 1 pulse
        t.PCIL132_ResetPos();
        CHECK(Value(kEcSetCmdPos) == 8000.0 && Refused() == 0);
        Fresh(-8000.6, 0.0);
        t.PCIL132_ResetPos();
        CHECK(Value(kEcSetCmdPos) == -8000.0 && Refused() == 0);

        Fresh(0.0, 0.0);                                                   // at 0 the old arm and the new one agree
        t.PCIL132_ResetPos();
        CHECK(Value(kEcSetCmdPos) == 0.0 && Value(kEcSetActPos) == 0.0 && Refused() == 0);

        Fresh(12345.0, 0.0);                                               // why it never synced: the old arm's ResetPos() writes 0
        CHECK(static_cast<HTMotor&>(m).ResetPos() == false && Value(kEcSetCmdPos) == 0.0 && Refused() == 2);   // through HTMotor, as PCIL132_ResetPos calls it
    }

    // ==================== 2: Contec -- golden's own encoder arm, unchanged ====================
    MOTION_CARD_TYPE = MotionCard_Contec;
    {
        TMyEtherCatMotor m(kAddr); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";
        Fresh(4321.0, 99.0);
        t.ServoOnOff(true);
        CHECK(Count(kEcSvOn) == 1 && Value(kEcSetCmdPos) == 4321.0 && Value(kEcSetActPos) == 4321.0 && Refused() == 0);
    }

    // ==================== 3: golden's gates kept ====================
    MOTION_CARD_TYPE = MotionCard_SYN;
    {
        TMyEtherCatMotor m(kAddr); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";
        m.PServoAlarmOn = false;                                           // HT9050 M03 / M22 / M41 / M42 / M108: golden does not sync
        Fresh(12345.0, 0.0);
        t.ServoOnOff(true);
        CHECK(Count(kEcSvOn) == 1 && Count(kEcSetCmdPos) == 0 && Count(kEcSetActPos) == 0);
        m.PServoAlarmOn = true;
        Fresh(12345.0, 0.0);
        t.ServoOnOff(false);                                               // servo OFF: no coordinate write
        CHECK(Count(kEcSvOn) == 1 && Value(kEcSvOn) == 0.0 && Count(kEcSetCmdPos) == 0 && Count(kEcSetActPos) == 0);
        m.Enable = false;
        Fresh(12345.0, 0.0);
        t.ServoOnOff(true);
        t.PCIL132_ResetPos();
        CHECK(g_calls.empty());
    }

    // ==================== 4: another class keeps golden's dispatch ====================
    {
        RecMotor r; r.Enable = true; r.PServoAlarmOn = true; r.Direction = false;
        TMyMotor u; u.Motor = &r; u.Mot_Name = 0;
        g_calls.clear();
        MOTION_CARD_TYPE = MotionCard_SYN;
        u.PCIL132_ResetPos();
        CHECK(r.resets == 1 && r.lastReset == 0);                         // golden SYN arm: ResetPos() (a SYNTEK copies the encoder)
        u.ServoOnOff(true);
        CHECK(r.resets == 2 && r.lastReset == 0);
        MOTION_CARD_TYPE = MotionCard_Contec;
        u.PCIL132_ResetPos();
        CHECK(r.resets == 3 && r.lastReset == 777);                       // golden else arm: the encoder
        r.Direction = true;
        u.PCIL132_ResetPos();
        CHECK(r.resets == 4 && r.lastReset == -777);                      // golden: if(Direction) p=-p
        r.Enable = false;
        u.PCIL132_ResetPos();
        CHECK(r.resets == 4);                                              // golden: Enable==false -> return
        CHECK(g_calls.empty());                                            // nothing of this went to the 1203 route
    }

    // ==================== 5: the class test ====================
    {
        TMyEtherCatMotor e(kAddr);
        TMySYNTEKMotor s(-1);
        RecMotor r;
        CHECK(W906_Is1203Motor(&e));
        CHECK(!W906_Is1203Motor(&s) && !W906_Is1203Motor(&r) && !W906_Is1203Motor(0));
    }

    // ==================== 6: no route installed (every SIM build) ====================
    SetEcatMotorRoute(0);
    MOTION_CARD_TYPE = MotionCard_SYN;
    {
        TMyEtherCatMotor m(kAddr); Setup(m);
        TMyMotor t; t.Motor = &m; t.Mot_Name = 0; t.CardType = "PCI1203";
        Fresh(12345.0, 0.0);
        t.ServoOnOff(true);
        t.PCIL132_ResetPos();
        CHECK(g_calls.empty());
    }

    MOTION_CARD_TYPE = oldCard;
    bAlarm = oldAlarm;
    SetEcatMotorRoute(0);
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
