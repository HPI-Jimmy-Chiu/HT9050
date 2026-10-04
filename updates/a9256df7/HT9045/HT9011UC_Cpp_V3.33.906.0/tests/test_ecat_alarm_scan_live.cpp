// ===========================================================================
//  tests/test_ecat_alarm_scan_live.cpp -- AI(W906-E045) 20261004 [W906] (St01 ST01-E2)
//
//  todo E-045 (S-26 R4 + Steven 1004 17:0x Q98) through the REAL csystem.cpp ScanAllMotorStatus: HT9050 1203 rows are
//  TMyEtherCatMotor objects reading a fake engine route (Motor/EcatMotorRoute.h -- the shape of
//  tests/test_ecat_motor_route.cpp's kFake), every other MOT[] row is a sim motor (tests/w906_test_motors.h). The JAM is
//  counted by golden ShowMotorErrorMessage's own recorder (forms/fNote_ShowError.cpp, InitialOK false = its stop half +
//  the Exception record, no note), the operator message by ShowMyMessage's (canary_support.cpp).
//    L1  M35 MLoaderZ (ServoAlarmOn 0) ERROR_STOP in auto-run  -> ONE JAM (MotorAlarmNo 7), SystemStart false, iHome 1,
//        one ResetError
//    L2  the reset clears it (pending, then READY)              -> no second JAM, no message, "cleared" line
//    L3  idle ERROR_STOP                                        -> HomeFlag 0, fAllMotorHome false, no JAM, one ResetError
//    L4  the reset does NOT clear it                            -> one power-cycle message, START / HOME gate refuses,
//                                                                  no second reset / message; power-out ends the latch
//    L5  not HT9050                                             -> golden: nothing
//    L6  a pending sample                                       -> nothing
//    L7  ServoAlarmOn 1 row outside golden's 5 re-read axes      -> ONE JAM (MotorAlarmNo 8, "Motor Alarm")
//    L8  a golden re-read axis (MInArmX, ServoAlarmOn 1)          -> still ONE JAM
//    L9  motor power off                                        -> no JAM, no reset (golden's EMG path reports)
//    L10 source pins on csystem.cpp (the call before golden's raise `if`, the OR, the MainProc SoftStart gate)
//    L11 ERROR_STOP after power-up, before the first full HOME (fAllMotorHome false) -> golden idle branch only: no reset,
//        no latch, START / HOME allowed; homed again -> a new alarm gets the one reset (ST01-M 1004 22:3x)
// ===========================================================================
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "cmydef.h"                 // SystemStart / iHome / fAllMotorHome / SoftStart / InitialOK / MachineTypeChoice / MLoaderZ / Sn*
#include "common.h"                 // as9045LogPath / asSaveEventLogPath / asProductionLogPath
#include "database.h"               // W906_GpibModel
#include "MachineType.h"            // Type_HT9050
#include "Motor/mymotor.h"          // MOT[]
#include "Motor/myEthercatmotor.h"  // TMyEtherCatMotor
#include "Motor/EcatMotorRoute.h"   // TEcatMotorRoute / SetEcatMotorRoute
#include "mysensor.h"               // Sen[]
#include "canary_support.h"         // MotorIndexToJamCode, the ShowMyMessage / ShowMotorErrorMessage recorders
#include "EcatAlarmScan.h"
#include "w906_test_motors.h"
#include "w906_ctest_guard.h"

void ScanAllMotorStatus(int iProcessCount);                                 // csystem.cpp:15931

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("FAIL [test_ecat_alarm_scan_live.cpp:%d]  %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

// --- the fake engine route: one sample per (board, port); ResetError marks it pending and, if clearable, READY ---
static std::map<int, TEcatAxisRead> g_samples;
static bool g_clearable = true;
static std::vector<int> g_resetAddr;
static bool FBind(int, int, int, bool) { return true; }
static bool FRead(int b, int p, TEcatAxisRead* out)
{
    std::map<int, TEcatAxisRead>::const_iterator it = g_samples.find(b * 100 + p);
    if (it == g_samples.end()) return false;
    *out = it->second;
    return true;
}
static unsigned long FCall(int b, int p, int op, int, double)
{
    if (op == kEcResetError) {
        g_resetAddr.push_back(b * 100 + p);
        TEcatAxisRead& s = g_samples[b * 100 + p];
        if (g_clearable) { s.state = kEcStaReady; s.motionIO &= ~2ul; }
        s.pending = true;                                                    // the route's own rule: state-changing -> pending
    }
    return 0;
}
static const TEcatMotorRoute kFake = { FBind, FRead, FCall, 0, 0, 0, 0, 0 };

static void Sample(int addr, unsigned short state, unsigned long io, bool pending = false)
{
    TEcatAxisRead s; s.valid = true; s.pending = pending; s.state = state; s.motionIO = io;
    g_samples[addr] = s;
}
static void Poll(int addr) { g_samples[addr].pending = false; }

// --- the note sink and the counters ---
static std::vector<std::string> g_notes;
static void Sink(const char* line) { g_notes.push_back(line ? line : ""); }
static int NotesWith(const char* needle) { int n = 0; for (size_t i = 0; i < g_notes.size(); ++i) if (g_notes[i].find(needle) != std::string::npos) ++n; return n; }
static int Resets(int addr) { int n = 0; for (size_t i = 0; i < g_resetAddr.size(); ++i) if (g_resetAddr[i] == addr) ++n; return n; }

// --- power: a sensor with Enable false reads IsOff()==false (no IO read); Enable + ISABase 99 + Type 1 reads off ---
static void PowerIn()
{
    const int ids[] = { SnFrontLeftEMG, SnFrontRightEMG, SnRearLeftEMG, SnRearRightEMG, SnServo, SnAllEMG, SnMotorPower };
    for (size_t k = 0; k < sizeof(ids) / sizeof(ids[0]); ++k) Sen[ids[k]].Enable = false;
}
static void MotorPowerOff() { Sen[SnMotorPower].Enable = true; Sen[SnMotorPower].ISABase = 99; Sen[SnMotorPower].Type = 1; }

struct EcatRow : TMyEtherCatMotor { explicit EcatRow(int addr) : TMyEtherCatMotor(addr) {} };
static EcatRow* MakeRow(int idx, int addr, const char* alias, bool servoAlarmOn)
{
    EcatRow* m = new EcatRow(addr);
    m->Enable = true; m->Direction = false; m->GearRatio = 1.0; m->MotorType = Servo_Motor; m->bSensorType = true;
    m->PServoAlarmOn = servoAlarmOn;
    MOT[idx].Motor = m;
    MOT[idx].SetAlias(idx, alias);
    for (int k = 0; k < 10; ++k) MOT[idx].Led[k] = false;
    return m;
}
static void Run(int idx) { ScanAllMotorStatus(idx % 4); }
static void Auto() { SystemStart = true; iHome = 0; fAllMotorHome = true; SoftStart = false; SoftStop = false; }
static void Idle() { SystemStart = false; iHome = 0; fAllMotorHome = true; SoftStart = false; SoftStop = false; }

static std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static std::string CodeOf(const std::string& line)                          // the line without its // tail and /*...*/ blocks
{
    std::string s = line, out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s.compare(i, 2, "/*") == 0) { size_t e = s.find("*/", i + 2); if (e == std::string::npos) break; i = e + 1; continue; }
        if (s.compare(i, 2, "//") == 0) break;
        out += s[i];
    }
    return out;
}

int main()
{
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asProductionLogPath", asProductionLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("EcatAlarmScanLive", rt))
        return 2;
    std::printf("=== test_ecat_alarm_scan_live (AI(W906-E045), S-26 R4 + Q98) ===\n");

    W906_TestEnsureSimMotors();                                              // golden boot invariant: MOT[].Motor never NULL
    const int kM35 = 3500, kInArm = 101;                                     // (board 35, port 0), (board 1, port 1)
    MakeRow(MLoaderZ, kM35, "MLoaderZ", false);                              // M35: HT9050's Mot_Table ServoAlarmOn 0
    MakeRow(MInArmX, kInArm, "MInArmX", true);
    Sample(kM35, kEcStaReady, 0);
    Sample(kInArm, kEcStaReady, 0);
    SetEcatMotorRoute(&kFake);
    W906_EcAlarmSetNote(&Sink);
    PowerIn();
    InitialOK = false;                                                       // ShowMotorErrorMessage: stop half + record, no note
    const AnsiString savedModel = W906_GpibModel;
    const int savedType = MachineTypeChoice;
    W906_GpibModel = "9050GPIB";
    std::printf("    MLoaderZ=%d (scan %d), MInArmX=%d (scan %d)\n", MLoaderZ, MLoaderZ % 4, MInArmX, MInArmX % 4);

    // ---- L1 ----
    std::printf("[L1] M35 ERROR_STOP in auto-run\n");
    Auto();
    int jam0 = W906_ShowMotorErrorMessage_Count, msg0 = W906_ShowMyMessage_Count;
    Sample(kM35, kEcStaErrorStop, 0);
    Run(MLoaderZ);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1);
    CHECK(W906_ShowMotorErrorMessage_LastCode == MotorIndexToJamCode(MLoaderZ));
    CHECK(W906_ShowMotorErrorMessage_LastMotorAlarmNo == 7);                 // GetErrorIndex 9 -> 6 (golden :4061-4062) + 1
    CHECK(SystemStart == false && iHome == 1 && fAllMotorHome == false);
    CHECK(Resets(kM35) == 1);
    CHECK(NotesWith("M35 MLoaderZ: drive alarm") == 1 && NotesWith("clear-alarm attempt") == 1);

    // ---- L2 ----
    std::printf("[L2] the reset clears it\n");
    Run(MLoaderZ);                                                           // pending: episode open, golden idle branch, no JAM
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1 && Resets(kM35) == 1);
    Poll(kM35);
    Run(MLoaderZ);                                                           // READY: cleared
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1 && Resets(kM35) == 1);
    CHECK(W906_ShowMyMessage_Count == msg0);
    CHECK(NotesWith("alarm cleared") == 1);
    CHECK(MOT[MLoaderZ].Led[iAlarmLed] == false);
    CHECK(W906_EcAlarmLatchedCount() == 0 && W906_EcAlarmStartAllowed("test"));

    // ---- L3 + L4 ----
    std::printf("[L3] idle ERROR_STOP; [L4] the reset does not clear it\n");
    Idle();
    MOT[MLoaderZ].HomeFlag = 1;
    g_clearable = false;
    jam0 = W906_ShowMotorErrorMessage_Count; msg0 = W906_ShowMyMessage_Count;
    g_resetAddr.clear(); g_notes.clear();
    Sample(kM35, kEcStaErrorStop, 0);
    Run(MLoaderZ);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0);
    CHECK(MOT[MLoaderZ].HomeFlag == 0 && fAllMotorHome == false && SystemStart == false);
    CHECK(Resets(kM35) == 1);
    Run(MLoaderZ);                                                           // pending
    CHECK(W906_ShowMyMessage_Count == msg0 && Resets(kM35) == 1);
    Poll(kM35);
    Run(MLoaderZ);                                                           // the first fresh sample after the reset: still ERROR_STOP
    CHECK(W906_ShowMyMessage_Count == msg0 + 1);
    CHECK(std::strstr(W906_ShowMyMessage_LastS1.c_str(), "could not be cleared") != 0);
    CHECK(W906_EcAlarmLatchedCount() == 1);
    for (int k = 0; k < 5; ++k) Run(MLoaderZ);
    CHECK(W906_ShowMyMessage_Count == msg0 + 1 && Resets(kM35) == 1);        // no retry loop, one message
    CHECK(W906_ShowMotorErrorMessage_Count == jam0);
    CHECK(W906_EcAlarmStartAllowed("test") == false);                         // START / HOME refused ...
    CHECK(W906_ShowMyMessage_Count == msg0 + 2 && NotesWith("START/HOME refused") == 1);
    { std::string why; CHECK(!W906_EcAlarmMotionAllowed(&why) && why.find("M35 MLoaderZ") != std::string::npos); }
    MotorPowerOff();
    Run(MLoaderZ);                                                           // ... until a power cycle
    CHECK(W906_EcAlarmLatchedCount() == 0 && NotesWith("episode is reset") == 1);
    PowerIn();
    CHECK(W906_EcAlarmStartAllowed("test"));
    Sample(kM35, kEcStaReady, 0);
    Run(MLoaderZ);
    CHECK(MOT[MLoaderZ].Led[iAlarmLed] == false);
    g_clearable = true;

    // ---- L5 ----
    std::printf("[L5] not HT9050 = golden\n");
    W906_EcAlarmResetAll();
    W906_GpibModel = "";  MachineTypeChoice = 0;
    Auto();
    MOT[MLoaderZ].HomeFlag = 1;
    jam0 = W906_ShowMotorErrorMessage_Count; g_resetAddr.clear();
    Sample(kM35, kEcStaErrorStop, 0x2);
    Run(MLoaderZ);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 && SystemStart == true && MOT[MLoaderZ].HomeFlag == 1 && Resets(kM35) == 0);
    W906_GpibModel = "9050GPIB";  MachineTypeChoice = savedType;

    // ---- L6 ----
    std::printf("[L6] pending sample\n");
    W906_EcAlarmResetAll();
    Auto();
    jam0 = W906_ShowMotorErrorMessage_Count; g_resetAddr.clear();
    Sample(kM35, kEcStaErrorStop, 0, true);
    MOT[MLoaderZ].Led[iAlarmLed] = false;
    Run(MLoaderZ);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 && SystemStart == true && Resets(kM35) == 0);

    // ---- L7 ----
    std::printf("[L7] ServoAlarmOn 1, not one of golden's 5 re-read axes\n");
    W906_EcAlarmResetAll();
    MOT[MLoaderZ].Motor->PServoAlarmOn = true;
    MOT[MLoaderZ].Led[iAlarmLed] = false;                                    // the stale LED golden leaves on this axis
    Auto();
    jam0 = W906_ShowMotorErrorMessage_Count;
    Sample(kM35, kEcStaReady, 0x2);                                          // ALM bit
    Run(MLoaderZ);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1);
    CHECK(W906_ShowMotorErrorMessage_LastMotorAlarmNo == 8);                 // GetErrorIndex 7 = "Motor Alarm"
    MOT[MLoaderZ].Motor->PServoAlarmOn = false;

    // ---- L8 ----
    std::printf("[L8] a golden re-read axis\n");
    W906_EcAlarmResetAll();
    Auto();
    jam0 = W906_ShowMotorErrorMessage_Count;
    Sample(kInArm, kEcStaErrorStop, 0);
    Run(MInArmX);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 + 1);
    CHECK(W906_ShowMotorErrorMessage_LastCode == MotorIndexToJamCode(MInArmX));
    Sample(kInArm, kEcStaReady, 0);
    Run(MInArmX); Run(MInArmX);

    // ---- L9 ----
    std::printf("[L9] motor power off\n");
    W906_EcAlarmResetAll();
    Auto();
    MotorPowerOff();
    jam0 = W906_ShowMotorErrorMessage_Count; g_resetAddr.clear();
    Sample(kM35, kEcStaErrorStop, 0);
    MOT[MLoaderZ].Led[iAlarmLed] = false;
    Run(MLoaderZ);
    CHECK(W906_ShowMotorErrorMessage_Count == jam0 && SystemStart == true && Resets(kM35) == 0);
    PowerIn();

    // ---- L11: boot, before the first full HOME (fAllMotorHome false): golden raise only, HOME never refused ----
    std::printf("[L11] ERROR_STOP after power-up, not homed yet\n");
    W906_EcAlarmResetAll();
    Idle();
    fAllMotorHome = false;                                                   // boot / after a motor power-off (golden)
    MOT[MLoaderZ].HomeFlag = 0;
    g_clearable = false;                                                     // a drive still coming up: a reset would NOT clear it
    jam0 = W906_ShowMotorErrorMessage_Count; msg0 = W906_ShowMyMessage_Count;
    g_resetAddr.clear(); g_notes.clear();
    Sample(kM35, kEcStaErrorStop, 0x2);
    for (int k = 0; k < 4; ++k) { Run(MLoaderZ); Poll(kM35); }
    CHECK(Resets(kM35) == 0);                                                // no clear attempt of ours
    CHECK(W906_EcAlarmLatchedCount() == 0 && W906_EcAlarmStartAllowed("test"));   // HOME / START not refused
    CHECK(W906_ShowMyMessage_Count == msg0 && W906_ShowMotorErrorMessage_Count == jam0);
    CHECK(MOT[MLoaderZ].HomeFlag == 0 && fAllMotorHome == false);           // golden idle branch: the axis needs HOME
    CHECK(NotesWith("not homed since power-up") == 1);
    Sample(kM35, kEcStaReady, 0);                                            // HOME's InitMotor reset it
    Run(MLoaderZ);
    CHECK(NotesWith("alarm cleared") == 1 && MOT[MLoaderZ].Led[iAlarmLed] == false);
    g_clearable = true;
    fAllMotorHome = true;                                                    // homed: a new alarm is armed again
    Sample(kM35, kEcStaErrorStop, 0x2);
    Run(MLoaderZ);
    CHECK(Resets(kM35) == 1);
    Poll(kM35); Run(MLoaderZ);                                               // the reset cleared it: the episode closes
    CHECK(W906_EcAlarmLatchedCount() == 0);

    // ---- L10: source pins ----
    std::printf("[L10] csystem.cpp source pins\n");
    {
        const std::string src = ReadAll(std::string(W906_SRC_ROOT) + "/csystem.cpp");
        std::vector<std::string> L;
        { std::stringstream ss(src); std::string l; while (std::getline(ss, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); L.push_back(l); } }
        size_t fn = 0;
        for (size_t i = 0; i < L.size(); ++i) if (L[i] == "void ScanAllMotorStatus(int iProcessCount)") { fn = i; break; }
        CHECK(fn != 0);
        size_t raise = 0;
        for (size_t i = fn; i < L.size() && i < fn + 200; ++i) if (CodeOf(L[i]) == "            if(MOT[i].Motor->Enable &&") { raise = i; break; }
        CHECK(raise != 0);
        if (raise != 0) {
            CHECK(CodeOf(L[raise - 1]).find("const bool w906EcAlm = W906_EcAlarmScanRow(i);") != std::string::npos);
            CHECK(CodeOf(L[raise + 1]).find("(MOT[i].Motor->PServoAlarmOn==1 || w906EcAlm) &&") != std::string::npos);
        }
        int gate = 0;
        for (size_t i = 0; i < L.size(); ++i) {
            const std::string c = CodeOf(L[i]);
            const size_t a = c.find("!W906_EcAlarmStartAllowed(\"MainProc SoftStart\")) { SoftStart = false;");
            const size_t b = c.find("W906_PageStartAllowedHook(\"MainProc SoftStart\")");
            if (a != std::string::npos && b != std::string::npos && a < b) ++gate;
        }
        CHECK(gate == 1);
    }

    W906_GpibModel = savedModel;
    SetEcatMotorRoute(0);
    W906_EcAlarmSetNote(0);
    std::printf("%d / %d checks passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
