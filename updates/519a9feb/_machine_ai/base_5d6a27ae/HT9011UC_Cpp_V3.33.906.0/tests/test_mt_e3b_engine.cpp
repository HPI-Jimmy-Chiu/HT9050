// =============================================================================
//  tests/test_mt_e3b_engine.cpp -- AI(W906-MT-E3b) 20260925
//
//  The engine half of the Motor Test wave (EastSun rulings 20260925).  Every
//  part is built so that it CAN fail -- each one is measured in both directions
//  where a direction exists:
//
//    A  R10  machinerecord.dat containment -- W906_MACHINERECORD_DIR must be set
//            by ctest and point at an existing directory, the seam must return the
//            golden literal byte for byte when it is unset, SaveMachineRecord must
//            land in the scratch directory, and the machine's own files under
//            d:\HT9045\system must keep their size and stamp.  ⚠ A failed redirect
//            would otherwise look exactly like a pass -- so this part REFUSES to
//            call SaveMachineRecord at all when the variable is missing.
//            Also pins sizeof(struct MachineRecord) = 876,908 (binary file layout).
//    B       TfHome::GaliMotorServoOff (golden uhome.cpp:4988-5016) + the 1203 hook.
//    C       TfHome::sbAbortHomeClick (golden uhome.cpp:4980-4986).
//    D  R8   MainProc: the stop arm aborts a shown home (SAFETY-GATE T6-ABORTHOME
//            lifted) -- and while the Motor Test web window is "open" MainProc
//            returns BEFORE that arm (golden MainProc pause).
//    E  R8   VerifyMotorAction (golden uteach.cpp:5346-5426): lock on motion, the
//            2 s idle StopAllMotor + AllBtnUp, "*Lock by Homeing", Enable guard.
//    F       W906_DoMotorPowerOnBegin/Step (cross-tick DoMotorPowerOn, 1 s).
//    G  R2   database.cpp: with INDEX_MOTION_CARD==0 the PCI1203 Index row (M14
//            MTestZ1) keeps its own CardModel/BoardID/Port; the SMC Index rows keep
//            golden's forced SMC override.  argv[1] = machines/HT9050/Mot_Table.csv.
//
//  NOT covered here (said so, not implied): the door-OPEN arm of
//  VerifyMotorAction (needs a live safe-door sensor), and anything on the 1203
//  card -- this test registers FAKE hooks; the real ones live in wb_serve.
//  It reads only argv[1]; it writes only the ctest scratch directories.
// =============================================================================
#include "vclcompat/vcl_compat.h"   // first: brings <windows.h> (Sleep, GetFileAttributesExA)
#include "database.h"               // HSys / TMOTDATA (part G)
#include "cmydef.h"                 // SystemStart / fAllMotorHome / bMotorPowerState / MotorPowerOnDelay / InitialOK
#include "cinitial.h"               // SaveMachineRecord
#include "csystem.h"                // MainProc / W906_* hooks / W906_DoMotorPowerOn*
#include "atester_shims.h"          // fContact stand-in (as test_mainproc_guard)
#include "Motor/mymotor.h"          // MOT[] / TOTAL_MOTOR
#include "myswitch.h"               // SW[]
#include "mysensor.h"               // Sen[]
#include "forms/fHome.h"            // fHome / TfHome
#include "w906_test_motors.h"       // MOT[].Motor boot invariant

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

// --- names this test needs that no header it can include declares -----------
extern AnsiString W906_MachineRecordRedirect(const AnsiString& goldenPath);   // cinitial.cpp AI(W906-MT-E3b)
extern unsigned   W906_MachineRecordSize();                                   // cinitial.cpp AI(W906-MT-E3b)
class uPlateInfo;
extern uPlateInfo *PickFromHPList;                                            // Public/HTEditList.cpp:240
uPlateInfo *W906_NewPlateInfo();                                              // Public/HTEditList.cpp:263
extern AnsiString sHPPickRec;                                                 // Public/HTEditList.cpp:268
extern bool IndexZCanMove[2];                                                 // golden ainarm2.h:48
extern bool bDestoryOnSht;
bool VerifyMotorAction();                                                     // forms/fTeach.h:830 (golden uteach.h:2398)

static int g_fail = 0, g_checks = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_checks;
    if (!ok) { ++g_fail; std::printf("  FAIL [test_mt_e3b_engine.cpp:%d]  %s\n", line, what); }
    else     {           std::printf("  ok    %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

// --- file stamp (size + last write) --------------------------------------------
struct Stamp { bool exists; unsigned long long size; unsigned long long mtime; };
static Stamp StampOf(const std::string& p)
{
    Stamp s = { false, 0, 0 };
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &d)) {
        s.exists = true;
        s.size  = ((unsigned long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
        s.mtime = ((unsigned long long)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
    }
    return s;
}
static bool SameStamp(const Stamp& a, const Stamp& b)
{
    return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime;
}
static bool IsDir(const char* p)
{
    const DWORD a = GetFileAttributesA(p);
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// --- fake hooks (the real ones are registered by wb_serve) -------------------
static int         g_stopCalls = 0;
static std::string g_stopWhy;
static void FakeStop1203(const char* why) { ++g_stopCalls; g_stopWhy = why ? why : ""; }

//AI(W906-MT-FIX1) 20260926: fake servo-ON answer per brake group (PART H) -- g_brakeHeld's drives are "not servo-ON".
static std::string g_brakeHeld;
static bool FakeBrakeServoOn(const char* group, AnsiString* why)
{
    const bool held = group && g_brakeHeld == group;
    if (held && why) *why = "fake: not servo-ON";
    return !held;
}

static int         g_btnUpCalls = 0;
static std::string g_btnUpWhy;
static void FakeAllBtnUp(const char* why) { ++g_btnUpCalls; g_btnUpWhy = why ? why : ""; }

static bool g_motorTestOpen = false;
static bool FakeFShow(const char* form) { return g_motorTestOpen && std::strcmp(form, "fMotorTest") == 0; }

static bool g_homing = false;
static bool FakeHoming() { return g_homing; }

static int g_movingMotor = -1;      // this motor reports "moving"; every motor is hook-owned
static int FakeMoving(int mi) { return (mi == g_movingMotor) ? 1 : 0; }

static void ClearHooks()
{
    W906_Stop1203AllHook   = 0;
    W906_FormFShowHook     = 0;
    W906_MotorHomingHook   = 0;
    W906_MotorMovingHook   = 0;
    W906_MotorAllBtnUpHook = 0;
}

static TMOTDATA* RowOf(const char* no)
{
    HSys.mapMotTableIter = HSys.mapMotTable.find(AnsiString(no));
    if (HSys.mapMotTableIter == HSys.mapMotTable.end()) return 0;
    const int k = HSys.mapMotTableIter->second.ToIntDef(-1);
    if (k < 0 || k >= (int)HSys.MotTable.size()) return 0;
    return HSys.MotTable[k];
}

int main(int argc, char** argv)
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("MT_E3b_Engine", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    std::printf("=== test_mt_e3b_engine (AI(W906-MT-E3b)) ===\n");
    W906_TestEnsureSimMotors();
    ClearHooks();
    if (PickFromHPList == 0) PickFromHPList = W906_NewPlateInfo();             // golden main.cpp:2149 (SaveMachineRecord's journal)

    // -----------------------------------------------------------------------
    //  PART A -- R10 containment of machinerecord.dat
    // -----------------------------------------------------------------------
    std::printf("\n-- PART A: machinerecord.dat containment (R10) --\n");
    CHECK(W906_MachineRecordSize() == 876908u);                               // binary layout pin; see the MT-E3b report
    const char* dirEnv = std::getenv("W906_MACHINERECORD_DIR");
    const bool contained = (dirEnv != 0 && dirEnv[0] != '\0' && IsDir(dirEnv));
    CHECK(contained);                                                         // ctest must set it (tests/CMakeLists.txt tail block)
    if (!contained) {
        std::printf("  ⛔ W906_MACHINERECORD_DIR is not set to an existing directory -- refusing to call\n"
                    "     SaveMachineRecord (it would write d:\\HT9045\\system\\machinerecord.dat).\n");
    } else {
        const std::string dir(dirEnv);
        const AnsiString goldenRec("d:\\HT9045\\system\\machinerecord.dat");
        const AnsiString goldenCcd("d:\\HT9045\\system\\machinerecordRealCCD.dat");
        CHECK(std::string(W906_MachineRecordRedirect(goldenRec).c_str()) == dir + "\\machinerecord.dat");
        CHECK(std::string(W906_MachineRecordRedirect(goldenCcd).c_str()) == dir + "\\machinerecordRealCCD.dat");
        CHECK(std::string(W906_MachineRecordRedirect(sHPPickRec).c_str()) == dir + "\\PickHPRec.json");

        // unset => the golden literal, byte for byte (production / F5)
        static std::string saved = std::string("W906_MACHINERECORD_DIR=") + dir;
        HT9045_TEST_PUTENV(const_cast<char*>("W906_MACHINERECORD_DIR="));
        const char* after = std::getenv("W906_MACHINERECORD_DIR");
        CHECK(after == 0 || after[0] == '\0');
        CHECK(std::strcmp(W906_MachineRecordRedirect(goldenRec).c_str(), "d:\\HT9045\\system\\machinerecord.dat") == 0);
        CHECK(std::strcmp(W906_MachineRecordRedirect(goldenCcd).c_str(), "d:\\HT9045\\system\\machinerecordRealCCD.dat") == 0);
        CHECK(std::strcmp(W906_MachineRecordRedirect(sHPPickRec).c_str(), sHPPickRec.c_str()) == 0);
        HT9045_TEST_PUTENV(const_cast<char*>(saved.c_str()));
        const char* back = std::getenv("W906_MACHINERECORD_DIR");
        const bool restored = (back != 0 && dir == back);
        CHECK(restored);

        if (restored) {
            const Stamp realRec0 = StampOf("d:\\HT9045\\system\\machinerecord.dat");
            const Stamp realCcd0 = StampOf("d:\\HT9045\\system\\machinerecordRealCCD.dat");
            const Stamp realHp0  = StampOf("d:\\HT9045\\system\\PickHPRec.json");
            std::remove((dir + "\\machinerecord.dat").c_str());
            std::remove((dir + "\\machinerecordRealCCD.dat").c_str());

            SaveMachineRecord(false);
            SaveMachineRecord(true);

            const Stamp s1 = StampOf(dir + "\\machinerecord.dat");
            const Stamp s2 = StampOf(dir + "\\machinerecordRealCCD.dat");
            const Stamp s3 = StampOf(dir + "\\PickHPRec.json");
            CHECK(s1.exists && s1.size == W906_MachineRecordSize());          // the write really happened, in scratch
            CHECK(s2.exists && s2.size == W906_MachineRecordSize());
            CHECK(s3.exists);
            CHECK(SameStamp(StampOf("d:\\HT9045\\system\\machinerecord.dat"), realRec0));        // ★ machine file untouched
            CHECK(SameStamp(StampOf("d:\\HT9045\\system\\machinerecordRealCCD.dat"), realCcd0));
            CHECK(SameStamp(StampOf("d:\\HT9045\\system\\PickHPRec.json"), realHp0));
            std::printf("  real machinerecord.dat: exists=%d size=%llu (unchanged)\n",
                        (int)realRec0.exists, realRec0.size);
        }
    }

    // -----------------------------------------------------------------------
    //  PART B -- TfHome::GaliMotorServoOff (golden uhome.cpp:4988-5016)
    // -----------------------------------------------------------------------
    std::printf("\n-- PART B: GaliMotorServoOff --\n");
    CHECK(fHome != 0);
    SystemStart = true; fAllMotorHome = true; bMotorPowerState = true;
    SW[SwMotorRelay].On(); SW[SwServerON].On();
    SW[SwInArmZBreaker].On(); SW[SwOutArmZBreaker].On(); SW[SwCassetteLDMotBreaker].On();
    IndexZCanMove[0] = false; IndexZCanMove[1] = false;
    MOT[MTestZ1].MovFlag = true; MOT[MTestZ1].bScanFlag = true; MOT[MTestZ1].GaliSofDelayCount = 7;
    MOT[0].HomeFlag = 1;
    W906_Stop1203AllHook = &FakeStop1203; g_stopCalls = 0; g_stopWhy = "";
    fHome->GaliMotorServoOff("ctest");
    CHECK(g_stopCalls == 1);
    CHECK(g_stopWhy == "GaliMotorServoOff - ctest");
    CHECK(SystemStart == false && fAllMotorHome == false && bMotorPowerState == false);
    CHECK(SW[SwMotorRelay].OutValue == false && SW[SwServerON].OutValue == false);
    CHECK(SW[SwInArmZBreaker].OutValue == false && SW[SwOutArmZBreaker].OutValue == false);   // brakes HELD (outputs ON = release)
    CHECK(SW[SwCassetteLDMotBreaker].OutValue == false);
    CHECK(IndexZCanMove[0] == true && IndexZCanMove[1] == true);              // InitGali_HomeTask
    CHECK(MOT[MTestZ1].MovFlag == false && MOT[MTestZ1].bScanFlag == false && MOT[MTestZ1].GaliSofDelayCount == 0);
    CHECK(MOT[0].HomeFlag == 1);                                              // golden does NOT clear HomeFlag (lead default: kept)
    W906_Stop1203AllHook = 0;
    fHome->GaliMotorServoOff("ctest-nohook");                                 // NULL-safe
    CHECK(g_stopCalls == 1);

    // -----------------------------------------------------------------------
    //  PART C -- TfHome::sbAbortHomeClick (golden uhome.cpp:4980-4986)
    // -----------------------------------------------------------------------
    std::printf("\n-- PART C: sbAbortHomeClick --\n");
    W906_Stop1203AllHook = &FakeStop1203; g_stopCalls = 0;
    fHome->Show();
    CHECK(fHome->fShow == true && fHome->fAbort == false);
    SystemStart = true;
    fHome->sbAbortHomeClick(fHome);
    CHECK(fHome->fShow == false && fHome->fAbort == true);
    CHECK(g_stopCalls == 1 && g_stopWhy == "GaliMotorServoOff - sbAbortHomeClick");
    CHECK(SystemStart == false);
    ClearHooks();

    // -----------------------------------------------------------------------
    //  PART D -- MainProc: stop arm aborts a shown home; Motor Test open pauses it
    // -----------------------------------------------------------------------
    std::printf("\n-- PART D: MainProc stop arm / pause --\n");
    InitialOK = true; SoftStop = false; bDestoryOnSht = false; iHome = 0;
    if (fContact) fContact->fShow = false;
    // D1: no page open -> SystemStart==false -> golden :18923 `if(fHome->fShow) fHome->sbAbortHomeClick(fHome);`
    W906_Stop1203AllHook = &FakeStop1203; g_stopCalls = 0;
    fHome->Show(); SystemStart = false;
    MainProc();
    CHECK(fHome->fShow == false && fHome->fAbort == true);                   // ★ SAFETY-GATE(W906-T6-ABORTHOME) is live
    CHECK(g_stopCalls >= 1);
    // D2: Motor Test window "open" -> golden MainProc returns before that arm
    g_motorTestOpen = true; W906_FormFShowHook = &FakeFShow;
    W906_MotorMovingHook = &FakeMoving; g_movingMotor = -1;
    CHECK(W906_FormFShow("fMotorTest", false) == true && W906_FormFShow("fTeach", false) == false);
    fHome->Show(); SystemStart = false;
    MainProc();
    CHECK(fHome->fShow == true && fHome->fAbort == false);                   // ★ paused: the stop arm did not run
    fHome->Close();
    g_motorTestOpen = false;
    ClearHooks();

    // -----------------------------------------------------------------------
    //  PART E -- VerifyMotorAction (golden uteach.cpp:5346-5426)
    // -----------------------------------------------------------------------
    std::printf("\n-- PART E: VerifyMotorAction --\n");
    for (int d = 0; d < MAX_SAFE_DOOR_CNT; ++d)                               // no live door in this process: make every door
        if (iSafeDoor[d] >= SnSafeDoor1 && iSafeDoor[d] < MAX_SENSOR_ITEM)    // sensor "not installed" so the door reads closed
            Sen[iSafeDoor[d]].Enable = false;
    CHECK(CheckSafeDoorIsClosed() == true);                                   // precondition of the 2 s path below

    // E0: page not open -> nothing
    SystemStart = false;
    const unsigned long u0 = W906_GetMotorLockState().unlockCount;
    CHECK(VerifyMotorAction() == false);
    CHECK(W906_GetMotorLockState().locked == false);

    // E1: open + one enabled motor moving -> lock
    const int mi = 3;
    const AnsiString aliasSave = MOT[mi].NumberAlias;
    MOT[mi].NumberAlias = "[M03] ctest";
    MOT[mi].Motor->Enable = true;
    g_motorTestOpen = true; W906_FormFShowHook = &FakeFShow;
    W906_MotorMovingHook = &FakeMoving; W906_MotorHomingHook = &FakeHoming; g_homing = false;
    W906_Stop1203AllHook = &FakeStop1203; W906_MotorAllBtnUpHook = &FakeAllBtnUp;
    g_stopCalls = 0; g_btnUpCalls = 0;
    g_movingMotor = mi;
    CHECK(VerifyMotorAction() == true);                                       // door closed -> golden returns bDoorClosed
    CHECK(W906_GetMotorLockState().locked == true);
    CHECK(W906_GetMotorLockState().labLockVisible == true);
    CHECK(std::string(W906_GetMotorLockState().labLockCaption.c_str()) == "*Lock by [M03] ctest moveing");
    CHECK(g_stopCalls == 0 && g_btnUpCalls == 0);

    // E2: motion ended -> still locked until 2 s without motion
    g_movingMotor = -1;
    CHECK(VerifyMotorAction() == true);
    CHECK(W906_GetMotorLockState().locked == true && W906_GetMotorLockState().labLockVisible == false);
    CHECK(g_stopCalls == 0);

    // E3: 2 s idle -> StopAllMotor + 1203 stop + AllBtnUp, unlock
    ::Sleep(2150);
    CHECK(VerifyMotorAction() == true);
    CHECK(g_stopCalls == 1 && g_stopWhy == "VerifyMotorAction: 2 s without motion");
    CHECK(g_btnUpCalls == 1 && g_btnUpWhy == "VerifyMotorAction: 2 s without motion");
    CHECK(W906_GetMotorLockState().locked == false);
    CHECK(W906_GetMotorLockState().unlockCount == u0 + 1);

    // E4: a web HOME job == golden btnHome->Down -> "*Lock by Homeing"
    g_homing = true;
    CHECK(VerifyMotorAction() == true);
    CHECK(W906_GetMotorLockState().locked == true);
    CHECK(std::string(W906_GetMotorLockState().labLockCaption.c_str()) == "*Lock by Homeing");
    g_homing = false;
    ::Sleep(2150);
    VerifyMotorAction();
    CHECK(W906_GetMotorLockState().locked == false && g_stopCalls == 2);

    // E5: golden's Enable guard -- a disabled motor never locks the page
    MOT[mi].Motor->Enable = false;
    g_movingMotor = mi;
    CHECK(VerifyMotorAction() == false);                                      // no lock -> door not checked -> false
    CHECK(W906_GetMotorLockState().locked == false);
    MOT[mi].Motor->Enable = true;
    g_movingMotor = -1;
    MOT[mi].NumberAlias = aliasSave;
    g_motorTestOpen = false;
    ClearHooks();

    // -----------------------------------------------------------------------
    //  PART F -- cross-tick DoMotorPowerOn (golden csystem.cpp:19102-19126)
    // -----------------------------------------------------------------------
    std::printf("\n-- PART F: W906_DoMotorPowerOnBegin/Step --\n");
    SW[SwMotorRelay].Off();
    SW[SwInArmZBreaker].On();
    const DWORD t0 = ::GetTickCount();
    W906_DoMotorPowerOnBegin();
    CHECK(SW[SwMotorRelay].OutValue == true);
#ifdef SOFT_SIMULTE
    CHECK(W906_DoMotorPowerOnStep() == true);                                 // golden SIM: relay on and return
#else
    CHECK(SW[SwInArmZBreaker].OutValue == false);                             // IndexMotorBreakerOFF (+ G03 tail) held it
    CHECK(W906_DoMotorPowerOnStep() == false);                                // the 1 s is not over
    bool done = false;
    while (!done && ::GetTickCount() - t0 < 3000) { ::Sleep(20); done = W906_DoMotorPowerOnStep(); }
    const DWORD dt = ::GetTickCount() - t0;
    std::printf("  power-on job finished after %lu ms\n", (unsigned long)dt);
    CHECK(done);
    CHECK(dt >= 900 && dt <= 1600);                                           // golden's 1 s busy loop, spread over ticks
#endif
    SW[SwMotorRelay].Off();

    // -----------------------------------------------------------------------
    //  PART G -- R2: the PCI1203 Index row is not forced to SMC
    // -----------------------------------------------------------------------
    std::printf("\n-- PART G: database.cpp Index rows (R2) --\n");
    if (argc < 2) {
        CHECK(!"argv[1] = machines/HT9050/Mot_Table.csv is required");
    } else {
        static std::string env = std::string("W906_MOTTABLE_PATH=") + argv[1];
        HT9045_TEST_PUTENV(const_cast<char*>(env.c_str()));
        const int saveIndex = INDEX_MOTION_CARD;

        INDEX_MOTION_CARD = 0;                                                // this machine's Gerneral.ini value
        HSys.LoadMotData();
        TMOTDATA* m14 = RowOf("M14");
        TMOTDATA* m13 = RowOf("M13");
        CHECK(m14 != 0 && m13 != 0);
        if (m14 && m13) {
            std::printf("  M14 %s %s board=%d port=%d | M13 %s %s board=%d\n",
                        m14->Alias.c_str(), m14->CardModel.c_str(), m14->iBoardID, m14->iPort,
                        m13->Alias.c_str(), m13->CardModel.c_str(), m13->iBoardID);
            CHECK(m14->Alias == AnsiString("MTestZ1"));
            CHECK(m14->CardModel == AnsiString("PCI1203"));                   // ★ R2: kept, not "SMC"
            CHECK(m14->iBoardID == 14 && m14->iPort == 0);
            CHECK(m13->Alias == AnsiString("MTestY1"));
            CHECK(m13->CardModel == AnsiString("SMC"));                       // golden override kept for the SMC rows
            CHECK(m13->iBoardID == -1 && m13->iPort == -1 && m13->dAcc == 1.0);
        }

        INDEX_MOTION_CARD = 1;                                                // the other arm: every row read as written
        HSys.LoadMotData();
        m14 = RowOf("M14");
        CHECK(m14 != 0 && m14->CardModel == AnsiString("PCI1203") && m14->iBoardID == 14);
        INDEX_MOTION_CARD = saveIndex;
    }

    // -----------------------------------------------------------------------
    //  PART H -- AI(W906-MT-FIX1) 20260926: EastSun rulings 20260926
    //    G16/G05/HOME: a brake group is released only while its 1203 axes are servo-ON (W906_BrakeReleaseOK)
    //    G04: on a PCI1203 Index row, "out of power" = SnMotorPower off or EMG (IsIndexMotorOutOfPower)
    // -----------------------------------------------------------------------
    std::printf("\n-- PART H: brake-release servo-ON guard, HT9050 out-of-power predicate --\n");
    W906_BrakeServoOnHook = 0;
    bMotorPowerState = true;
    CHECK(W906_BrakeReleaseOK("InOutArmZ", "test") == true);                   // no hook (ctest): golden
    W906_BrakeServoOnHook = &FakeBrakeServoOn; g_brakeHeld = "InOutArmZ";
    CHECK(W906_BrakeReleaseOK("InOutArmZ", "test") == false);                  // that group's drive not servo-ON: held
    CHECK(W906_BrakeReleaseOK("Cassette", "test") == true);                    // another group whose drives are ON: released
    bMotorPowerState = false;
    CHECK(W906_BrakeReleaseOK("Cassette", "test") == false);                   // motor power just cut: never released
    bMotorPowerState = true;
#ifndef SOFT_SIMULTE
    {
        const AnsiString saveCard = MOT[MTestZ1].CardType;
        const bool saveSenEnable = Sen[SnMotorPower].Enable;
        Sen[SnMotorPower].Enable = false;                                     // a disabled sensor: IsOff() == false (motor power "on")
        MOT[MTestZ1].CardType = "";                                           // not a 1203 row: golden test (Galil servo LEDs, never lit here)
        MOT[MTestZ1].Led[iServoOn] = false;
        CHECK(IsIndexMotorOutOfPower() == true);                              // golden: TRUE FOREVER on this machine (the G04 note)
        MOT[MTestZ1].CardType = "PCI1203";                                    // HT9050 (R2)
        CHECK(IsIndexMotorOutOfPower() == IsEMGPressed());                    // ruling: SnMotorPower off || EMG -- power is "on" here
        // G16: the countdown reaches 0 -> only the allowed groups are released
        W906_BrakeServoOnHook = &FakeBrakeServoOn; g_brakeHeld = "InOutArmZ";
        SW[SwInArmZBreaker].Off(); SW[SwOutArmZBreaker].Off(); SW[SwCassetteLDMotBreaker].Off();
        MotorPowerOnDelay = 1; SystemSec = (SystemSec + 7) % 60;             // a new second -> decrement to 0 -> the release block
        CountMotorPowerDelay();
        CHECK(MotorPowerOnDelay <= 0);
        CHECK(SW[SwInArmZBreaker].OutValue == false && SW[SwOutArmZBreaker].OutValue == false);   // held: its axes not servo-ON
        CHECK(SW[SwCassetteLDMotBreaker].OutValue == true);                                        // released: its axes are ON
        W906_BrakeServoOnHook = 0;
        MOT[MTestZ1].CardType = saveCard;
        Sen[SnMotorPower].Enable = saveSenEnable;
        SW[SwCassetteLDMotBreaker].Off();
    }
#endif
    W906_BrakeServoOnHook = 0;

    SystemStart = false; iHome = 0;
    std::printf("\n=== %d checks, %d FAIL ===\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
