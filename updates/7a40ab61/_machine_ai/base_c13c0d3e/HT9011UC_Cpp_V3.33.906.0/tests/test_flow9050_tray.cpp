// =============================================================================
//  test_flow9050_tray.cpp  --  F9050-BD VERIFY: Frank's 910 HT9050 tray-arm /
//  loader layered flows, their Type_HT9050 dispatch arms, the B-batch
//  definitions, and the Type_HT9046_LS answers (what 9050GPIB decodes to
//  today, database.cpp:517) unchanged.
//
//  AI(W906-F9050-BD) 20261004: new file (ctest Flow9050_Tray).
//
//  WHAT IS UNDER TEST (translated line by line from Frank's 910 tree,
//  ref/frank-910-9050 2275f78c, HT9011UC_Code_V3.33.910.0_20260820_beforeFinePitch;
//  ledger ids from docs/FLOW9050_PORT_LEDGER.md):
//    H005/H010/H011  MaxCylinderItem 321, MAX_SENSOR_ITEM 900, MAX_LOADER_LAYER_9050 20
//    H083/H084/H085  the 26 C_* (295-320) / 75 Sn* (802-876) constants, the layer counters (-1)
//    H013/H014       their names, registered ONLY for Type_HT9050 (cinitial.cpp InitialSensorName / InitialCylinderName)
//    H064            acatchtray.cpp DoCatchFromLoader_9050 / DoCatchFromEmpty_9050 (910 :2434-2603) + TrayXMoveCheckEnc (:70-87)
//    H065            acatchtray.cpp DoPlaceTrayToEmpty_9050 / DoPlaceTrayToAuto_9050 / DoPlaceToBuffer_9050 (910 :3961-4302)
//    H076            asendic_Loader.cpp DoLoadNewICTray_9050 / DoLoad_9050 (910 :2459-2739)
//    H091            csystem.cpp DoReceiveAllToBottom_9050 (910 :7611-7714)
//    H068-H072 / H075 / H077 / H090  the Type_HT9050 arms of DoCatchTray (cases 200 / 250 / 2120 / 6100),
//                    DoInspectTrayColorOnLoader (case 100), DoLoad (top) and InitAllProcessTask, run for both types.
//
//  ORACLES are hand-derived from the 910 text, NOT from the translation:
//    [S]  static_asserts: the limits, Type_HT9050 == 800, sizeof(sCylinderData) == 13 x MaxCylinderItem AnsiStrings
//         (forms/fStartCondition.h:339 is a literal), Prod.TrayZ_Down[MAX_TRACK], Prod.EmptyZ_Up[MAX_LOADER_LAYER_9050], and
//         910's PROD_INFO_ST field order iTemperature .. iBS1Right_Half (offsetof), on which 910 defect (a)'s reads depend.
//    [K]  constant values (910 cmydef.cpp:702-727 / :1770-1844) and the 910 start values of every 9050 counter /
//         cursor (910 cmydef.cpp:6108-6111, acatchtray.cpp:2434 / :2529-2531 / :3964 / :4065 / :4236,
//         asendic_Loader.cpp:2459 / :2555) -- checked before anything runs (W-01 Q2 pins the -1).
//    [N]  names: under Type_HT9046_LS slots 295-320 / 802-899 stay unnamed and the named counts are those of today;
//         under Type_HT9050 the 26 + 75 slots get names and slots 0-294 / 0-801 keep exactly the same names.
//    [X]  TrayXMoveCheckEnc: 0 while TrayArmMotorMove has not arrived, 1 at the target (910 :74-86).
//    [CL] [CE] [PE] [PA] [PB] [LN] [LD] [RB]: the case sequence of each state machine, its result and the
//         axis / tray / counter it leaves; each 9050 case moves at most one step per call (no fall-through), so
//         the sequence of distinct cursor values is exact.  [PA] also pins the case-700 deviation (MOutArmX AND
//         MOutArmY unlocked, Frank 1004 17:45; 910 :4222-4223 writes MOutArmY twice) and [PB] the Frank 1002
//         `static int pos` fix (Tray X is never sent to 0 on the way to Empty).
//    [H2120]  910 defect (b), kept on purpose (decision D3, NIGHT_REPORT s0 #105): with Tray.Data ToBuffer=2
//         DoCatchTray stays in case 2120 for ever, raises no alarm, and InitAllProcessTask does not reset the
//         9050 cursor (InitialPlaceToBuffer_9050 has no caller in 910).
//    [D]  dispatch, both types: sentinels in the golden cursor and in the 9050 cursor show WHICH function ran.
//  NOT unit-tested here (review only): DoCatchTray case 1100 (its `static int Target` is local to DoCatchTray),
//  DoTrayFeed's flag3 arm (case 200 also drives the Galil Z / Y), the HOME flag19 edits (ProcessMotorHome step 1310
//  needs the whole HOME fixture), DoLoad case 900 (dead for 9050: DoLoad hands a 9050 to DoLoad_9050 at its top), and
//  TrayXMoveCheckEnc's encoder-error 2 (it writes a Process log line).
//
//  FIXTURE NOTES (known traps, measured):
//    * HTMotor::CheckIsSafeDoorOpen() is TRUE (door open) with no callback and Enable==true: MotorMove then returns -1,
//      which `==1` waits on for ever and TrayArmMotorMove's bool turns into "arrived".  Every axis the flows move gets a
//      door-closed callback (same fixture as tests/test_flow9050_shuttle.cpp).
//    * TMySimMotor: MoveToPos jumps, MotionDone is always true, the encoder equals the command -> MotorMove returns 0 on
//      the call that issues a new target and 1 on the next one (1 at once when already there).
//    * Every Cylinder[] stays Enable=false: Push()/Pop() return true on the first call, On()/Off() only set flags.
//      Every Sen[] stays Enable=false, so IsOn() and IsOff() BOTH return false -- except where a case sets
//      SnLoaderTrayHasTray Enable=1 / Type=1 at the all-zero address: MyLaneIO.IOInputBit returns false for address
//      0/0/0/0 before touching any card, so IsOff() is true in the SIM and the SHIP build alike.
//    * Layer indices: never let a counter reach -1 or >= 9 for the Loader (Prod.TrayZ_Up/Down are [MAX_TRACK]) or >= 20
//      for the others -- that is 910 defect (a), undefined behaviour, not something to pin.
//    * DoLoad_9050's layer probe is a function-local static starting at 0: the direct-entry checks of [LD] run first.
//  Memory only: no file is opened by this test (the Gerneral.ini sandbox seed is the versioned
//  machines/HT9050/sim_9378/Gerneral.ini, tests/CMakeLists.txt); the ShowErrorMessage seam answers the alarms.  It still
//  refuses to run outside ctest's redirect roots (w906_ctest_guard.h), because the golden paths it drives can log.
// =============================================================================
#include "vclcompat/vcl_compat.h"   // AnsiString
#include "aHotPlateSubstrate.h"     // CatchTraySuck / InArmSuck
#include "cprod.h"                  // Prod / IniConfig / TestIF_File / TrayForm
#include "cpublic.h"                // CosFunction
#include "cmydef.h"                 // MachineTypeChoice / M* / C_* / Sn* / the 9050 externs at the end of the file
#include "MachineType.h"            // Type_HT9050 / Type_HT9046_LS / eAboveCoveyor / eUnderCoveyor / MAX_LOADER_LAYER_9050
#include "Motor/mymotor.h"          // MOT[] / TrayArmMotorMove
#include "Motor/mySimMotor.h"       // TMySimMotor
#include "mysensor.h"               // Sen[]
#include "mycylin.h"                // Cylinder[] / InitialCylinderName / W906_CylinderRowsShown
#include "acatchtray.h"             // DoCatchTray / CatchTrayTask / iCatchFromLoaderTask / iPlaceToBufferTask / MTrayXCanSafeMove
#include "asendic_Loader.h"         // DoLoad / LoadTask / iLoadNewICTrayTask / DoInspectTrayColorOnLoader / iInspectTrayColorOnLoaderTask
#include "csystem.h"                // InitAllProcessTask
#include "cinitial.h"               // InitialSensorName
#include "forms/fTrayForm.h"        // TfTrayForm / fTrayForm (IsEnableColorSensor is virtual)
#include "forms/fStartCondition.h"  // TfStartCondition::sCylinderData (size pin)
#include "canary_support.h"         // ShowErrorMessage seam (W906_ShowErrorMessage_*)
#include <cstddef>                  // offsetof
#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>
#include <initializer_list>
#include "w906_test_motors.h"       // W906_TestEnsureSimMotors (MOT[].Motor boot invariant)
#include "w906_ctest_guard.h"       // W906TestRequireCtestRedirects
#include "LastSet.h"                // Tech (AI(W906-MACH0210) 20261005: TrayArmMotorMove's station guard reads Tech.iTrayXEmpty / iTrayXColor)

// No header home (910 declares none of these either).
int  TrayXMoveCheckEnc(int iTarget);                 // acatchtray.cpp   (910 acatchtray.cpp:72)
void InitialCatchFromLoader_9050();                  // acatchtray.cpp   (910 :2435)
int  DoCatchFromLoader_9050();                       // acatchtray.cpp   (910 :2439)
void InitialCatchFromEmpty_9050();                   // acatchtray.cpp   (910 :2532)
int  DoCatchFromEmpty_9050();                        // acatchtray.cpp   (910 :2536)
void InitialPlaceToEmpty_9050();                     // acatchtray.cpp   (910 :3965)
int  DoPlaceTrayToEmpty_9050();                      // acatchtray.cpp   (910 :3969)
void InitialPlaceToAuto_9050();                      // acatchtray.cpp   (910 :4066)
int  DoPlaceTrayToAuto_9050(int iAutoTarget);        // acatchtray.cpp   (910 :4070)
void InitialPlaceToBuffer_9050();                    // acatchtray.cpp   (910 :4238)
bool DoPlaceToBuffer_9050();                         // acatchtray.cpp   (910 :4245)
void InitLoadNewICTrayTask_9050();                   // asendic_Loader.cpp (910 :2460)
void DoLoad_9050();                                  // asendic_Loader.cpp (910 :2561)
extern int iCatchFromEmpty9050Task;                  // acatchtray.cpp   (910 :2531)
extern int iEmptyLayerCountDetect_9050;              // acatchtray.cpp   (910 :2530)
extern int iPlaceToEmpty9050Task;                    // acatchtray.cpp   (910 :3964)
extern int iPlaceToAuto9050Task;                     // acatchtray.cpp   (910 :4065)
extern int iPlaceToBuffer9050Task;                   // acatchtray.cpp   (910 :4236)
extern int LoadTask_9050;                            // asendic_Loader.cpp (910 :2555)

// ---------------------------------------------------------------------------
//  [S] compile-time pins
// ---------------------------------------------------------------------------
static_assert(MaxCylinderItem == 321, "mycylin.h MaxCylinderItem must be 910's 321 (910 mycylin.h:8)");
static_assert(MAX_SENSOR_ITEM == 900, "MachineType.h MAX_SENSOR_ITEM must be 910's 900 (910 MachineType.h:389)");
static_assert(MAX_LOADER_LAYER_9050 == 20, "MAX_LOADER_LAYER_9050 must be 910's 20 (910 MachineType.h:398)");
static_assert(Type_HT9050 == 800, "Type_HT9050 is 800 (MachineType.h)");
static_assert(sizeof(TfStartCondition::sCylinderData) == sizeof(AnsiString) * 13 * MaxCylinderItem,
              "forms/fStartCondition.h:339 sCylinderData must be [13][MaxCylinderItem] (golden bound)");
static_assert(sizeof(Prod.TrayZ_Down) == sizeof(int) * MAX_TRACK, "910 cprod.h:488 TrayZ_Down[MAX_TRACK]");
static_assert(sizeof(Prod.EmptyZ_Up) == sizeof(int) * MAX_LOADER_LAYER_9050, "910 cprod.h:494 EmptyZ_Up[MAX_LOADER_LAYER_9050]");
static_assert(sizeof(Prod.Auto3Z_Down) == sizeof(int) * MAX_LOADER_LAYER_9050, "910 cprod.h:513 Auto3Z_Down[MAX_LOADER_LAYER_9050]");
// 910's field order around them (910 cprod.h:485-519; F9050-BD review): the out-of-bounds layer reads of 910 defect (a)
// (Loader layer >= 9 or -1, Empty / Auto layer -1 or >= 20) hit the same neighbours as in 910 only with this exact order.
#define W906_OFS(f) offsetof(PROD_INFO_ST, f)
static_assert(W906_OFS(TrayZ_Up)   == W906_OFS(iTemperature) + sizeof(int),          "910 cprod.h:485/487: TrayZ_Up follows iTemperature");
static_assert(W906_OFS(TrayZ_Down) == W906_OFS(TrayZ_Up)   + sizeof(int) * MAX_TRACK, "910 cprod.h:488: TrayZ_Down sits right after TrayZ_Up");
static_assert(W906_OFS(TrayZ_Mid)  == W906_OFS(TrayZ_Down) + sizeof(int) * MAX_TRACK, "910 cprod.h:489: TrayZ_Mid right after TrayZ_Down");
static_assert(W906_OFS(TrayZ_Home) == W906_OFS(TrayZ_Mid)  + sizeof(int) * MAX_TRACK, "910 cprod.h:490: TrayZ_Home right after TrayZ_Mid");
static_assert(W906_OFS(EmptyZ_Up)  == W906_OFS(iCatchExtraLift_9050) + sizeof(int),  "910 cprod.h:492-494: EmptyZ_Up right after iCatchExtraLift_9050");
static_assert(W906_OFS(Auto1Z_Up)  == W906_OFS(iEmptyCatchExtraLift_9050) + sizeof(int), "910 cprod.h:498-500");
static_assert(W906_OFS(Auto2Z_Up)  == W906_OFS(iAuto1CatchExtraLift_9050) + sizeof(int), "910 cprod.h:504-506");
static_assert(W906_OFS(Auto3Z_Up)  == W906_OFS(iAuto2CatchExtraLift_9050) + sizeof(int), "910 cprod.h:510-512");
static_assert(W906_OFS(iBS1Right_Half) == W906_OFS(iAuto3CatchExtraLift_9050) + sizeof(int),
              "910 cprod.h:516-518: iBS1Right_Half right after iAuto3CatchExtraLift_9050");
static_assert(W906_OFS(iBS1Right_Half) - W906_OFS(TrayZ_Home) == sizeof(int) * (3 + 4 * (2 * MAX_LOADER_LAYER_9050 + 3)),
              "910 cprod.h:490-516 = 3 Loader ints + 4 blocks of Up[20] / Down[20] / Home / InitDetectLayer / CatchExtraLift");
#undef W906_OFS

// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool DoorClosed() { return false; }   // PF_CHECK (Motor/HTMotor.h); same fixture as tests/test_flow9050_shuttle.cpp

// AI(W906-POOL5-4) 20261006 (Ifor01): POOL-5 #6 (Frank FR-PR1 2C; TO_IFOR 18:4x = A) -- the cylinder name counts 260 / 285+25 are
//   replaced by tests/oracle/cylinder_names_906.txt (same oracle as test_machine_cylinders.cpp): G = golden 906, A = every type,
//   H = Type_HT9050 only.  `kinds` selects which lines to read.
static std::set<std::string> NameOracle(const std::string& path, const char* kinds)
{
    std::set<std::string> out;
    FILE* in = std::fopen(path.c_str(), "rb");
    if (!in) return out;
    char line[512];
    while (std::fgets(line, sizeof line, in)) {
        if (line[0] == 0 || line[1] != ' ' || !std::strchr(kinds, line[0])) continue;
        std::string n(line + 2);
        while (!n.empty() && (n.back() == '\r' || n.back() == '\n' || n.back() == ' ')) n.pop_back();
        if (!n.empty()) out.insert(n);
    }
    std::fclose(in);
    return out;
}
static std::string g_namesPath;   // --names=<tests/oracle/cylinder_names_906.txt>
static int SetDiff(const std::set<std::string>& want, const std::set<std::string>& got, const char* tag)
{
    int bad = 0;
    for (const std::string& n : want) if (!got.count(n)) { if (bad++ < 6) printf("    %s: in the oracle but not assigned: %s\n", tag, n.c_str()); }
    for (const std::string& n : got) if (!want.count(n)) { if (bad++ < 6) printf("    %s: assigned but not in the oracle: %s\n", tag, n.c_str()); }
    return bad;
}

static void SetMotPos(int m, int p)
{
    MOT[m].Position = p;
    if (MOT[m].Motor != NULL)
        MOT[m].Motor->SetPosition(p);
    MOT[m].fCMD = false;
}
static void FreeAxis(int m)
{
    MOT[m].fCanMove = true; MOT[m].fCanMoveR = true; MOT[m].fCanMoveM = true; MOT[m].fCanMoveL = true;
    MOT[m].Motor->Enable = true;
    MOT[m].Motor->MotorIdleSafeDoorCheck = &DoorClosed;
}
static int Pos(int m) { return MOT[m].ReadPos(); }

// Calls fn() up to maxCalls times; seq = the cursor before the first call, then every new value it takes (a repeat of
// the previous value is not recorded); stops after the call whose result == stopOn.  Returns the number of calls made
// (maxCalls+1 = stopOn never came).
template <class F> static int Pump(int &cursor, F fn, int maxCalls, int stopOn, std::vector<int> &seq)
{
    seq.clear();
    seq.push_back(cursor);
    for (int i = 1; i <= maxCalls; ++i)
    {
        const int r = (int)fn();
        if (cursor != seq.back()) seq.push_back(cursor);
        if (r == stopOn) return i;
    }
    return maxCalls + 1;
}
static std::string SeqStr(const std::vector<int> &s)
{
    std::string r;
    char b[24];
    for (size_t i = 0; i < s.size(); ++i) { sprintf(b, i ? ">%d" : "%d", s[i]); r += b; }
    return r;
}
static bool SeqIs(const std::vector<int> &s, std::initializer_list<int> want)
{
    const bool ok = (s == std::vector<int>(want));
    if (!ok) printf("    got %s\n", SeqStr(s).c_str());
    return ok;
}

// Teach values (Prod is zero at start and 910 assigns none of the 9050 fields): distinct, positive, never 0, so that a
// motor that is sent to an unset (0) value would be seen.
static void TeachValues()
{
    //AI(W906-PKG146) 20261005 (machine): station order as on the machine -- Load < Empty < safe line < Color < Auto. The machine's 10-04
    //  HT9050-TYPE puts Type_HT9050 into TrayArmMotorMove's LS station guard (Motor/mymotor.cpp, safe line = (Tech.iTrayXEmpty +
    //  Tech.iTrayXColor) / 2 + 6500), so Tech is set too: (30000 + 45000) / 2 + 6500 = 44000. Was Load 60000 / Empty 40000 with Tech 0
    //  (safe line 6500) -- every leftward move was refused and the 9050 flows never left their move cases.
    Prod.iXTrayLoad  = 120000;  Prod.iXTrayEmpty = 30000;  Prod.iXTrayColor = 45000;   // AI(W906-TRAYSAFE9050) 20261007: the HT9050 order (EastSun 1007: Loader at one end, Empty at the other, the Autos between; machine teach Loader 191213 / Auto1 140718 / Auto2 94735 / Auto3 46762 / Empty 732) -- was Load 20000 < Empty 30000 (the 9046 layout), which the HT9050 guard now refuses
    Tech.iTrayXEmpty = 30000;  Tech.iTrayXColor = 45000;
    Prod.iXTrayAuto[0] = 90000; Prod.iXTrayAuto[1] = 80000; Prod.iXTrayAuto[2] = 70000;
    for (int k = 0; k < MAX_TRACK; ++k) { Prod.TrayZ_Up[k] = 1000 * (k + 1); Prod.TrayZ_Down[k] = 1000 * (k + 1) - 600; }
    Prod.TrayZ_Home = 500;  Prod.iCatchExtraLift_9050 = 300;  Prod.iLoaderInitDetectLayer = 3;
    for (int k = 0; k < MAX_LOADER_LAYER_9050; ++k)
    {
        Prod.EmptyZ_Up[k] = 2000 + 100 * k;   Prod.EmptyZ_Down[k] = 1500 + 100 * k;
        Prod.Auto1Z_Up[k] = 3000 + 100 * k;   Prod.Auto1Z_Down[k] = 2500 + 100 * k;
        Prod.Auto2Z_Up[k] = 6000 + 100 * k;   Prod.Auto2Z_Down[k] = 5500 + 100 * k;
        Prod.Auto3Z_Up[k] = 9000 + 100 * k;   Prod.Auto3Z_Down[k] = 8500 + 100 * k;
    }
    Prod.EmptyZ_Home = 510;  Prod.iEmptyCatchExtraLift_9050 = 310;
    Prod.Auto1Z_Home = 521;  Prod.iAuto1CatchExtraLift_9050 = 321;
    Prod.Auto2Z_Home = 522;  Prod.iAuto2CatchExtraLift_9050 = 322;
    Prod.Auto3Z_Home = 523;  Prod.iAuto3CatchExtraLift_9050 = 323;  { static const int up0[5] = { 1000, 2000, 3000, 6000, 9000 }, dn0[5] = { 400, 1500, 2500, 5500, 8500 }, pit[5] = { -1000, -100, -100, -100, -100 }, hm[5] = { 500, 510, 521, 522, 523 }, lf[5] = { 300, 310, 321, 322, 323 }; for (int z = 0; z < 5; ++z) { Tech.iTrayZ9050UpBase[z] = up0[z]; Tech.iTrayZ9050DownBase[z] = dn0[z]; Tech.iTrayZ9050Pitch[z] = pit[z]; Tech.iTrayZ9050Home[z] = hm[z]; Tech.iTrayZ9050Lift[z] = lf[z]; } Tech.iTrayZ9050ProbeStart = 3; Tech.iTrayZ9050ProbeLimit = 0; }   //AI(W906-TRAYZ-PITCH) 20261006: the flow now reads Tech (base - layer*pitch, cinitial.cpp EOF); these bases / pitches give exactly the Prod values above for every layer (e.g. Loader Up 1000 - k*(-1000) = 1000*(k+1)), so the step checks below are unchanged in meaning. Same line
}
int W906_TrayZ9050(int zone, int up, int layer); int W906_TrayZ9050Home(int zone);   // The globals the 9050 functions and the dispatchers read, in their "plain HT9050, no options" state.   //AI(W906-TRAYZ-PITCH) 20261006: + the two declarations, same line
static void Globals()
{
    TRAY_ARM_MODE = eAboveCoveyor;            // disabled cylinders make MTrayXCanSafeMove() true (OffSensor/OnSensor true, GetOutBit false)
    INSTALL_OCR = eocrUninstal;
    CosFunction.bTrayOCR = false;
    IniConfig.bP56TrayArmWaitAtColorTrack = false;
    IniConfig.bEnable_SECS_GEM = false;
    CUSTOMER_CODE = 0;
    USE_BU5_Function = 0;
    USE_LdUldCassetteMode = 0;
    USE_TRAY_MAPPING = etmUninstall;
    iCleanOut = 0; iOneCycle = 0;
    iPauseBackUp = -1;                        // DoCatchTray's first early return off
    bEject = false;                           // DoCatchTray's second early return off
    bOneTimeHotPlateCheckAll = false;
    TestIF_File.bPreventDropfunction = false;
    TrayForm.bEnableAMR = false;
    iRunStartMode = 0;
    W906_ShowErrorMessage_Reset();
}
// A tray form whose MU-N colour sensor is "installed" -- the only way into DoInspectTrayColorOnLoader's switch.
class TColourOn : public TfTrayForm
{
public:
    virtual bool IsEnableColorSensor() { return true; }
};

int main(int argc, char** argv)
{   extern AnsiString DataPath; const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    for (int k = 1; k < argc; ++k)
        if (std::strncmp(argv[k], "--names=", 8) == 0) g_namesPath = argv[k] + 8;   // AI(W906-POOL5-4)
    if (!W906TestRequireCtestRedirects("Flow9050_Tray", rt)) return 2;   // drives golden DoCatchTray / DoLoad / InitAllProcessTask

    setvbuf(stdout, NULL, _IONBF, 0);   // unbuffered (as tests/test_flow9050_shuttle.cpp): a crash shows the last PASS line
    printf("==== F9050-BD: HT9050 tray arm / loader layered flows (910 2275f78c) + Type_HT9050 dispatch ====\n");

    // =======================================================================
    //  [K] constants and the 910 start values -- before anything runs
    // =======================================================================
    printf("[K] constants (910 cmydef.cpp:702-727 / :1770-1844) and start values\n");
    CHECK(C_MobileTrayTableSelect == 295 && C_LoaderEdgeClip == 315 && C_Auto1EdgeClip == 316 && C_Auto2EdgeClip == 317 &&
          C_Auto3EdgeClip == 318 && C_EmptyEdgeClip == 319 && C_CasArm_Clip == 320 && C_CasArm_Clip < MaxCylinderItem,
          "C_MobileTrayTableSelect 295, C_LoaderEdgeClip 315, C_Auto1~3EdgeClip 316-318, C_EmptyEdgeClip 319, C_CasArm_Clip 320 < 321");
    CHECK(SnMobileTrayHasTray == 802 && SnLoaderDrawerHasTray == 815 && SnCasArmHasTray == 876 && SnCasArmHasTray < MAX_SENSOR_ITEM,
          "SnMobileTrayHasTray 802, SnLoaderDrawerHasTray 815, SnCasArmHasTray 876 < 900");
    CHECK(iLoaderLayerCount_9050 == -1 && iEmptyLayerCount_9050 == -1 && iAuto1LayerCount_9050 == 0 &&
          iAuto2LayerCount_9050 == 0 && iAuto3LayerCount_9050 == 0 && iEmptyLayerCountDetect_9050 == 0,
          "Loader / Empty start at -1 (= probe), Auto1~3 at 0 (= empty; AI(W906-TRAYFLOW9050) 20261007, was -1 in 910); iEmptyLayerCountDetect_9050 0 (never used)");
    CHECK(iCatchFromLoader9050Task == 1 && iCatchFromEmpty9050Task == 1 && iPlaceToEmpty9050Task == 1 && iPlaceToAuto9050Task == 1 &&
          iPlaceToBuffer9050Task == 1 && iLoadNewICTrayTask_9050 == 1 && LoadTask_9050 == 1,
          "every 9050 cursor starts at 1");

    // Fixture: every axis a TMySimMotor; the axes the flows move get a closed door and free CanMove flags.
    W906_TestEnsureSimMotors();
    const int axes[] = { MTrayX, MLoaderZ, MEmptyZ, MAuto1Z, MAuto2Z, MAuto3Z };
    for (size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i) FreeAxis(axes[i]);
    Globals();
    TeachValues();

    // =======================================================================
    //  [N] names (cinitial.cpp): registered only for Type_HT9050
    // =======================================================================
    printf("[N] InitialCylinderName / InitialSensorName: 26 + 75 names only for Type_HT9050\n");
    {
        std::vector<std::string> cylLS(MaxCylinderItem), senLS(MAX_SENSOR_ITEM);
        MachineTypeChoice = Type_HT9046_LS;
        InitialCylinderName();
        InitialSensorName();
        int nCylLS = 0, nSenLS = 0, newNamedLS = 0;
        for (int i = 0; i < MaxCylinderItem; ++i)
        {
            cylLS[i] = Cylinder[i].CylinderName.c_str();
            if (!cylLS[i].empty()) { ++nCylLS; if (i >= 295) ++newNamedLS; }
        }
        for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
        {
            senLS[i] = Sen[i].Name.c_str();
            if (!senLS[i].empty()) { ++nSenLS; if (i >= 802) ++newNamedLS; }
        }
        {   // AI(W906-POOL5-4) 20261006 (Ifor01): was `nCylLS == 260` -- now the name set against the oracle (G + A)
            const std::set<std::string> want = NameOracle(g_namesPath, "GA");
            std::set<std::string> got;
            for (const std::string& n : cylLS) if (!n.empty()) got.insert(n);
            printf("  Type_HT9046_LS: %d named slots, %d names, oracle G+A %d\n", nCylLS, (int)got.size(), (int)want.size());
            CHECK(!want.empty() && SetDiff(want, got, "Type_HT9046_LS") == 0 && nCylLS == (int)got.size(),
                  "Type_HT9046_LS: the cylinder names = golden 906 + C_OutArmSmallY (tests/oracle/cylinder_names_906.txt G+A), one slot each");
        }   //AI(W906-PKG146) 20261005 (machine)
        CHECK(newNamedLS == 0, "Type_HT9046_LS: Cylinder[295..320] and Sen[802..899] stay unnamed (nothing can bind an IO_Table row)");
        CHECK(W906_CylinderRowsShown() == 295, "Type_HT9046_LS: W906_CylinderRowsShown() == 295 (SmartDiagnostic grid stays 296 rows)");

        MachineTypeChoice = Type_HT9050;
        InitialCylinderName();
        InitialSensorName();
        int nCyl = 0, nSen = 0, sameOld = 0, newCylNamed = 0, newSenNamed = 0, tailNamed = 0;
        for (int i = 0; i < MaxCylinderItem; ++i)
        {
            const std::string n = Cylinder[i].CylinderName.c_str();
            if (!n.empty()) ++nCyl;
            if (i < 295) { if (n == cylLS[i]) ++sameOld; }
            else if (!n.empty()) ++newCylNamed;
        }
        for (int i = 0; i < MAX_SENSOR_ITEM; ++i)
        {
            const std::string n = Sen[i].Name.c_str();
            if (!n.empty()) ++nSen;
            if (i < 802) { if (n == senLS[i]) ++sameOld; }
            else if (i <= 876) { if (!n.empty()) ++newSenNamed; }
            else if (!n.empty()) ++tailNamed;
        }
        {   // AI(W906-POOL5-4) 20261006 (Ifor01): was `nCyl == 285 && newCylNamed == 25` -- now the oracle (G + A + H), and the H names
            //   (F9050-BD, Type_HT9050 only) are exactly what slots 295-320 carry (machine: C_OutArmSmallY stays at RULE10's slot 152)
            const std::set<std::string> want = NameOracle(g_namesPath, "GAH"), h = NameOracle(g_namesPath, "H");
            std::set<std::string> got, tail;
            for (int i = 0; i < MaxCylinderItem; ++i) {
                const std::string n = Cylinder[i].CylinderName.c_str();
                if (n.empty()) continue;
                got.insert(n);
                if (i >= 295) tail.insert(n);
            }
            printf("  Type_HT9050: %d named slots, %d names, oracle G+A+H %d; slots 295-320 named %d, oracle H %d\n",
                   nCyl, (int)got.size(), (int)want.size(), newCylNamed, (int)h.size());
            CHECK(!h.empty() && SetDiff(want, got, "Type_HT9050") == 0 && nCyl == (int)got.size(),
                  "Type_HT9050: the cylinder names = golden 906 + C_OutArmSmallY + F9050-BD (oracle G+A+H), one slot each");
            CHECK(SetDiff(h, tail, "slots 295-320") == 0 && newCylNamed == (int)h.size(),
                  "Type_HT9050: slots 295-320 carry exactly the F9050-BD names (oracle H) -- every one of them, nothing else");
        }   //AI(W906-PKG146) 20261005 (machine)
        CHECK(nSen == nSenLS + 75 && newSenNamed == 75 && tailNamed == 0, "Type_HT9050: +75 sensor names in slots 802-876, 877-899 unnamed");
        CHECK(sameOld == 295 + 802, "Type_HT9050: slots 0-294 / 0-801 carry exactly the Type_HT9046_LS names (no collision, nothing moved)");
        CHECK(Cylinder[C_MobileTrayTableSelect].CylinderName == AnsiString("C_MobileTrayTableSelect") &&
              Cylinder[C_LoaderEdgeClip].CylinderName == AnsiString("C_LoaderEdgeClip") &&
              Cylinder[C_CasArm_Clip].CylinderName == AnsiString("C_CasArm_Clip"),
              "Type_HT9050: Cylinder[295] / [315] / [320] named as 910 cinitial.cpp:4465 / :4485 / :4490");
        CHECK(Sen[SnMobileTrayHasTray].Name == AnsiString("SnMobileTrayHasTray") &&
              Sen[SnLoaderDrawerHasTray].Name == AnsiString("SnLoaderDrawerHasTray") &&
              Sen[SnCasArmHasTray].Name == AnsiString("SnCasArmHasTray"),
              "Type_HT9050: Sen[802] / [815] / [876] named as 910 cinitial.cpp:2471 / :2484 / :2545");
        CHECK(W906_CylinderRowsShown() == 321, "Type_HT9050: W906_CylinderRowsShown() == MaxCylinderItem (321)");

        // Back to the boot state of a 9050GPIB machine (InitialSensorName never clears names; InitialCylinderName does).
        for (int i = 802; i < MAX_SENSOR_ITEM; ++i) Sen[i].Name = "";
        MachineTypeChoice = Type_HT9046_LS;
        InitialCylinderName();
        int back = 0;
        for (int i = 295; i < MaxCylinderItem; ++i) if (Cylinder[i].CylinderName != AnsiString("")) ++back;
        CHECK(back == 0, "restored: Cylinder[295..320] unnamed again under Type_HT9046_LS");
    }

    MachineTypeChoice = Type_HT9050;          // the unit parts below call the 9050 functions directly

    // =======================================================================
    //  [X] TrayXMoveCheckEnc (910 acatchtray.cpp:72-87)
    // =======================================================================
    printf("[X] TrayXMoveCheckEnc: 0 until TrayArmMotorMove arrives, then 1 (encoder == target)\n");
    {
        SetMotPos(MTrayX, 0);
        const int r1 = TrayXMoveCheckEnc(Prod.iXTrayLoad);
        int calls = 1, r = r1;
        while (r != 1 && calls < 5) { r = TrayXMoveCheckEnc(Prod.iXTrayLoad); ++calls; }
        CHECK(r1 == 0 && r == 1 && calls == 2 && Pos(MTrayX) == Prod.iXTrayLoad,
              "first call 0 (command issued), second call 1, MTrayX at iXTrayLoad");
    }

    // =======================================================================
    //  [CL] DoCatchFromLoader_9050 (910 :2439-2524)
    // =======================================================================
    //AI(W906-F9050-FIX1) 20261005: Frank 1005 -- the tray is handed over from MMTrayY (where DoLoadNewICTray_9050 case 300 put it), not MMTrayZ
    //  (910 :2492-2494 as written; Frank「目前的應該是MMTrayY才對，MMTrayZ是馬達動作，在程式裡面資訊流判斷都是以Y為主」). MMTrayZ is left alone.
    printf("[CL] DoCatchFromLoader_9050: 1>10>20>30>40>50>60>1, layer +1, tray MMTrayY -> MTrayX\n");
    {
        std::vector<int> seq;
        iLoaderLayerCount_9050 = 2;
        SetMotPos(MLoaderZ, 0);
        MOT[MTrayX].ClearTray("Flow9050_Tray");
        MOT[MMTrayZ].ClearTray("Flow9050_Tray");
        MOT[MMTrayY].fHasTray = true; MOT[MMTrayY].iIsCoverTray = 7; MOT[MMTrayY].sTrayID = "T9050";
        MOT[MMTrayZ].fHasTray = true; MOT[MMTrayZ].sTrayID = "ZSTACK";   // must NOT be what is handed over or cleared
        MOT[MInArmX].fCanMove = false; MOT[MInArmY].fCanMove = false;
        bCatchTrayFinishAction = true;
        InitialCatchFromLoader_9050();
        const int n = Pump(iCatchFromLoader9050Task, [] { return DoCatchFromLoader_9050(); }, 40, 1, seq);
        CHECK(n <= 40 && SeqIs(seq, { 1, 10, 20, 30, 40, 50, 60, 1 }), "case sequence 1>10>20>30>40>50>60>1, returns 1");
        CHECK(iLoaderLayerCount_9050 == 3, "iLoaderLayerCount_9050 2 -> 3 (case 20)");
        CHECK(Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 3), "MLoaderZ ends at TrayZ_Down[3] (case 50)");
        CHECK(Pos(MTrayX) == Prod.iXTrayEmpty, "MTrayX ends at iXTrayEmpty (case 60, no P56 / OCR)");
        CHECK(MOT[MTrayX].fHasTray && MOT[MTrayX].iIsCoverTray == 7 && MOT[MTrayX].sTrayID == AnsiString("T9050") &&
              MOT[MMTrayY].fHasTray == false && MOT[MMTrayY].sTrayID == AnsiString(""),
              "tray ownership MMTrayY -> MTrayX with cover flag and tray ID (case 40, Frank 1005)");
        CHECK(MOT[MMTrayZ].fHasTray && MOT[MMTrayZ].sTrayID == AnsiString("ZSTACK"),
              "MMTrayZ untouched by the hand-over (it is the Z motor's, Frank 1005)");
        MOT[MMTrayZ].ClearTray("Flow9050_Tray");
        CHECK(MOT[MInArmX].fCanMove && MOT[MInArmY].fCanMove && bCatchTrayFinishAction == false,
              "InArm X / Y unlocked and bCatchTrayFinishAction cleared (case 60)");

        // Return 3: MTrayXCanSafeMove()==false (Under-conveyor arm with MTrayZ off its home sensor).
        TRAY_ARM_MODE = eUnderCoveyor;
        MOT[MTrayZ].Led[iHomeLed] = false;
        iCatchFromLoader9050Task = 1;
        const int r3 = DoCatchFromLoader_9050();
        CHECK(MTrayXCanSafeMove() == false && r3 == 3 && iCatchFromLoader9050Task == 1,
              "Tray X not safe to move: returns 3, cursor stays 1 (910 :2445-2450); no return 2 exists");
        TRAY_ARM_MODE = eAboveCoveyor;
    }

    // =======================================================================
    //  [CE] DoCatchFromEmpty_9050 (910 :2536-2603) -- no caller in 910 (defect (c)); pinned anyway
    // =======================================================================
    printf("[CE] DoCatchFromEmpty_9050 (unwired): 1>10>20>30>40>70>1, layer unchanged\n");
    {
        std::vector<int> seq;
        iEmptyLayerCount_9050 = 1;
        SetMotPos(MEmptyZ, 0);
        SetMotPos(MTrayX, Prod.iXTrayLoad);
        MOT[MTrayX].ClearTray("Flow9050_Tray");
        MOT[MMEmpty].fHasTray = true; MOT[MMEmpty].iIsCoverTray = 5; MOT[MMEmpty].sTrayID = "E1";
        InitialCatchFromEmpty_9050();
        const int n = Pump(iCatchFromEmpty9050Task, [] { return DoCatchFromEmpty_9050(); }, 40, 1, seq);
        CHECK(n <= 40 && SeqIs(seq, { 1, 10, 20, 30, 40, 70, 1 }), "case sequence 1>10>20>30>40>70>1, returns 1");
        CHECK(iEmptyLayerCount_9050 == 0, "AI(W906-TRAYFLOW9050) 20261007: a pick takes the top tray, 1 -> 0 (910 left the count unchanged)");
        CHECK(Pos(MEmptyZ) == W906_TrayZ9050(1, 0, 0) && Pos(MTrayX) == Prod.iXTrayEmpty, "MEmptyZ at EmptyZ_Down[1], MTrayX at iXTrayEmpty");
        CHECK(MOT[MTrayX].fHasTray && MOT[MTrayX].iIsCoverTray == 5 && MOT[MTrayX].sTrayID == AnsiString("E1") && MOT[MMEmpty].fHasTray == false,
              "tray ownership MMEmpty -> MTrayX (case 30)");
    }

    // =======================================================================
    //  [PE] DoPlaceTrayToEmpty_9050 (910 :3969-4063)
    // =======================================================================
    printf("[PE] DoPlaceTrayToEmpty_9050: 1>200>300>400>500>1 (+1 layer); MEmptyZ.fHasTray forced -> 1>100>110>200>...>1 (+2)\n");
    {
        std::vector<int> seq;
        iEmptyLayerCount_9050 = 1;
        MOT[MEmptyZ].fHasTray = false;
        SetMotPos(MEmptyZ, 0);
        SetMotPos(MTrayX, Prod.iXTrayLoad);
        MOT[MTrayX].fHasTray = true;
        MOT[MInArmX].fCanMove = false; MOT[MInArmY].fCanMove = false;
        InitialPlaceToEmpty_9050();
        int n = Pump(iPlaceToEmpty9050Task, [] { return DoPlaceTrayToEmpty_9050(); }, 40, 1, seq);
        CHECK(n <= 40 && SeqIs(seq, { 1, 200, 300, 400, 500, 1 }), "MEmptyZ.fHasTray false (never written in 910): 1>200>300>400>500>1");
        CHECK(iEmptyLayerCount_9050 == 2 && Pos(MEmptyZ) == W906_TrayZ9050(1, 0, 2), "layer 1 -> 2, MEmptyZ at EmptyZ_Down[2]");
        CHECK(MOT[MTrayX].fHasTray == false && Pos(MTrayX) == Prod.iXTrayEmpty && MOT[MInArmX].fCanMove && MOT[MInArmY].fCanMove,
              "tray left MTrayX (case 300); Tray X waits at iXTrayEmpty, InArm X / Y unlocked (case 500)");

        // (b) 910 never writes MOT[MEmptyZ].fHasTray (defect (f)) -- forced here to pin what the dead branch would do.
        iEmptyLayerCount_9050 = 1;
        MOT[MEmptyZ].fHasTray = true;
        MOT[MMEmpty].fHasTray = true;
        MOT[MTrayX].fHasTray = true;
        InitialPlaceToEmpty_9050();
        n = Pump(iPlaceToEmpty9050Task, [] { return DoPlaceTrayToEmpty_9050(); }, 40, 1, seq);
        CHECK(n <= 40 && SeqIs(seq, { 1, 100, 110, 200, 300, 400, 500, 1 }), "forced MEmptyZ.fHasTray: 1>100>110>200>300>400>500>1");
        CHECK(iEmptyLayerCount_9050 == 3 && MOT[MMEmpty].fHasTray == false, "layer +2 and MMEmpty cleared at case 110");
        MOT[MEmptyZ].fHasTray = false;
    }

    // =======================================================================
    //  [PA] DoPlaceTrayToAuto_9050 (910 :4070-4234), incl. the decided case-700 deviation
    // =======================================================================
    printf("[PA] DoPlaceTrayToAuto_9050(1..3): Out Arm X / Y locked at case 1, BOTH unlocked at case 700\n");
    {
        CHECK(DoPlaceTrayToAuto_9050(0) == 0 && DoPlaceTrayToAuto_9050(4) == 0 && iPlaceToAuto9050Task == 1,
              "(0) and (4): return 0 before the switch, cursor untouched (910 :4072-4073; (0) = defect (b))");
        const int motZ[3]   = { MAuto1Z, MAuto2Z, MAuto3Z };
        const int mmMot[3]  = { MMAuto1, MMAuto2, MMAuto3 };
        int *const layer[3] = { &iAuto1LayerCount_9050, &iAuto2LayerCount_9050, &iAuto3LayerCount_9050 };
        const int down2[3] = { W906_TrayZ9050(2, 0, 2), W906_TrayZ9050(3, 0, 2), W906_TrayZ9050(4, 0, 2) };   //AI(W906-TRAYZ-PITCH) 20261006: was { Prod.Auto1Z_Down, .. } read at [i][2]
        for (int k = 1; k <= 3; ++k)
        {
            const int i = k - 1;
            char msg[200];
            std::vector<int> seq;
            *layer[i] = 1;
            MOT[motZ[i]].fHasTray = false;
            MOT[mmMot[i]].fHasTray = false;
            SetMotPos(motZ[i], 0);
            SetMotPos(MTrayX, Prod.iXTrayLoad);
            SetMotPos(MOutArmY, 0);                          // IsOutArmSafe(): Y >= YOutArm_Shuttle1_Pick[0][2]-3500 (0)
            MOT[MTrayX].fHasTray = true;
            MOT[MOutArmX].fCanMove = true; MOT[MOutArmY].fCanMove = true;
            InitialPlaceToAuto_9050();
            seq.clear(); seq.push_back(iPlaceToAuto9050Task);
            int r = DoPlaceTrayToAuto_9050(k);
            if (iPlaceToAuto9050Task != seq.back()) seq.push_back(iPlaceToAuto9050Task);
            sprintf(msg, "Auto%d case 1: MOutArmX and MOutArmY locked (eAboveCoveyor, 910 :4114-4119)", k);
            CHECK(r == 0 && MOT[MOutArmX].fCanMove == false && MOT[MOutArmY].fCanMove == false, msg);
            int n = 1;
            while (r != 1 && n < 60)
            {
                r = DoPlaceTrayToAuto_9050(k);
                ++n;
                if (iPlaceToAuto9050Task != seq.back()) seq.push_back(iPlaceToAuto9050Task);
            }
            sprintf(msg, "Auto%d: 1>200>300>400>500>600>700>1, returns 1", k);
            CHECK(r == 1 && SeqIs(seq, { 1, 200, 300, 400, 500, 600, 700, 1 }), msg);
            sprintf(msg, "Auto%d: layer 1 -> 2, MAuto%dZ at Auto%dZ_Down[2], MMAuto%d has the tray, Tray X back at iXTrayEmpty", k, k, k, k);
            CHECK(*layer[i] == 2 && Pos(motZ[i]) == down2[i] && MOT[mmMot[i]].fHasTray && MOT[MTrayX].fHasTray == false &&
                  Pos(MTrayX) == Prod.iXTrayEmpty, msg);
            sprintf(msg, "Auto%d case 700: MOutArmX AND MOutArmY unlocked (DEVIATION from 910 :4222, Frank 1004 17:45)", k);
            CHECK(MOT[MOutArmX].fCanMove && MOT[MOutArmY].fCanMove, msg);
        }
        // MAuto1Z.fHasTray forced (never written in 910, defect (f)): the 100 / 110 branch.
        std::vector<int> seq;
        iAuto1LayerCount_9050 = 1;
        MOT[MAuto1Z].fHasTray = true;
        MOT[MMAuto1].fHasTray = true;
        SetMotPos(MTrayX, Prod.iXTrayLoad);
        MOT[MTrayX].fHasTray = true;
        InitialPlaceToAuto_9050();
        const int n = Pump(iPlaceToAuto9050Task, [] { return DoPlaceTrayToAuto_9050(1); }, 60, 1, seq);
        CHECK(n <= 60 && SeqIs(seq, { 1, 100, 110, 200, 300, 400, 500, 600, 700, 1 }) && iAuto1LayerCount_9050 == 3,
              "forced MAuto1Z.fHasTray: 1>100>110>200>...>700>1, layer +2");
        MOT[MAuto1Z].fHasTray = false;
    }

    // =======================================================================
    //  [PB] DoPlaceToBuffer_9050 (910 :4245-4302)
    // =======================================================================
    //AI(W906-F9050-FIX2) 20261006: Frank 1006「先改成Loader空盤只能到Empty Tray」-- ToBuffer=2 (Item 3, [3]AUTO1) goes to Empty too; it used to stay
    //  in case 200 for ever (DoPlaceTrayToAuto_9050(0), defect (b), 910 as written).
    printf("[PB] DoPlaceToBuffer_9050: ToBuffer 0 / 1 / 2 -> Empty (1>10>100>1), case 200 never reached (Frank 1006)\n");
    {
        for (int to = 0; to <= 2; ++to)
        {
            char msg[200];
            std::vector<int> seq;
            TrayForm.LoaderToEmptyColor[0] = to; TrayForm.LoaderToEmptyColor[1] = to;
            iEmptyLayerCount_9050 = 1;
            MOT[MEmptyZ].fHasTray = false;
            SetMotPos(MTrayX, Prod.iXTrayLoad);
            MOT[MTrayX].fHasTray = true;
            InitialPlaceToBuffer_9050();
            int minX = Pos(MTrayX), firstNew = -1;
            seq.clear(); seq.push_back(iPlaceToBuffer9050Task);
            bool done = false;
            for (int c = 0; c < 60 && !done; ++c)
            {
                done = DoPlaceToBuffer_9050();
                if (iPlaceToBuffer9050Task != seq.back()) seq.push_back(iPlaceToBuffer9050Task);
                const int x = Pos(MTrayX);
                if (x < minX) minX = x;
                if (firstNew < 0 && x != Prod.iXTrayLoad) firstNew = x;
            }
            sprintf(msg, "ToBuffer=%d: Item %d -> 1>10>100>1, returns true", to, to + 1);
            CHECK(done && CatchTraySuck.Item[0][0] == to + 1 && SeqIs(seq, { 1, 10, 100, 1 }), msg);
            sprintf(msg, "ToBuffer=%d: Tray X goes straight to iXTrayEmpty and never to 0 (Frank 1002 `static int pos`)", to);
            CHECK(firstNew == Prod.iXTrayEmpty && minX > 0, msg);
            CHECK(iPlaceToEmpty9050Task == 1 && MOT[MTrayX].fHasTray == false, "inner DoPlaceTrayToEmpty_9050 finished (cursor 1, tray placed)");
        }
        CHECK(iPlaceToAuto9050Task == 1, "ToBuffer 0..2: the Auto placer (DoPlaceTrayToAuto_9050) is never entered");
        InitialPlaceToBuffer_9050();
    }

    // =======================================================================
    //  [TZ] AI(W906-F9050-FIX2) 20261006: Frank 1006「Tray Arm在移動過程中 In/Out Arm ZA要在上位點」-- Motor/mymotor.cpp TrayArmMotorMove,
    //       Type_HT9050 only; "up" = golden InArmZSafe / OutArmZSafe(DETECT_POS_FLAG) (ReadPos() >= 0, not the HOME lamp)
    // =======================================================================
    printf("[CW] AI(W906-TRAYFLOW9050) 20261007: W906_CatchFromEmpty9050 (DoCatchTray 500/600 on HT9050) and the HOME / START reset\n");
    {
        extern int W906_CatchFromEmpty9050(int iWhichAuto); extern void W906_InitTrayTasks9050();
        MachineTypeChoice = Type_HT9050;
        SetMotPos(MInArmZA, 0); SetMotPos(MOutArmZA, 0);
        W906_InitTrayTasks9050();
        CHECK(iEmptyLayerCount_9050 == -1 && iCatchFromEmpty9050Task == 1, "HOME / START reset: Empty count -1 (re-probe), Empty pick cursor 1");
        iEmptyLayerCount_9050 = 0;  SetMotPos(MEmptyZ, 4321);
        int r = 0; for (int i = 0; i < 50; ++i) r = W906_CatchFromEmpty9050(1);
        CHECK(r == 2 && Pos(MEmptyZ) == 4321 && iCatchFromEmpty9050Task == 1, "Empty count 0: answers 2 (no tray -> DoCatchTray case 1), nothing moves");
        iEmptyLayerCount_9050 = 2;  SetMotPos(MEmptyZ, 0);  SetMotPos(MTrayX, Prod.iXTrayLoad);
        MOT[MTrayX].ClearTray("Flow9050_Tray");
        MOT[MMEmpty].fHasTray = true; MOT[MMEmpty].iIsCoverTray = 0; MOT[MMEmpty].sTrayID = "E2";
        r = 0; for (int i = 0; i < 200 && r != 1; ++i) r = W906_CatchFromEmpty9050(1);
        CHECK(r == 1 && iEmptyLayerCount_9050 == 1 && MOT[MTrayX].fHasTray && Pos(MEmptyZ) == W906_TrayZ9050(1, 0, 1),
              "Empty count 2: one pick of the top tray -> answers 1, tray on Tray X, count 1, MEmptyZ at Down[1]");
        MachineTypeChoice = Type_HT9046_LS;  iCatchFromEmpty9050Task = 77;
        extern void InitAllProcessTask();  InitAllProcessTask();
        CHECK(iCatchFromEmpty9050Task == 77, "Type_HT9046_LS: InitAllProcessTask leaves the 9050 cursors alone");
        MachineTypeChoice = Type_HT9050;  InitAllProcessTask();
        CHECK(iCatchFromEmpty9050Task == 1 && iEmptyLayerCount_9050 == -1, "Type_HT9050: InitAllProcessTask (HOME / START) resets them");
        iEmptyLayerCount_9050 = -1;
    }
    printf("[TS] AI(W906-TRAYSAFE9050) 20261007: TrayArmMotorMove's HT9050 station order (Loader | Autos | Empty, sides from the teach values)\n");
    {
        MachineTypeChoice = Type_HT9050;
        SetMotPos(MInArmZA, 0); SetMotPos(MOutArmZA, 0);
        auto tryOk = [](int target) { bool ok = false; for (int i = 0; i < 200 && !ok; ++i) ok = TrayArmMotorMove(target, true); return ok; };
        SetMotPos(MTrayX, Prod.iXTrayAuto[1]);
        CHECK(tryOk(Prod.iXTrayEmpty) && Pos(MTrayX) == Prod.iXTrayEmpty, "Empty (30000, beyond every Auto on its side): moves -- golden's 9046 safe line refused it");
        CHECK(tryOk(Prod.iXTrayLoad) && Pos(MTrayX) == Prod.iXTrayLoad, "Loader (120000, beyond every Auto on the other side): moves");
        CHECK(tryOk(Prod.iXTrayAuto[0]) && Pos(MTrayX) == Prod.iXTrayAuto[0], "Auto1 (between Empty and Loader): moves");
        { const int col = Prod.iXTrayColor;  Prod.iXTrayColor = Prod.iXTrayEmpty - 732;   // the machine 1008: Color = teach 0 + 6800 = 6800, just beyond Empty 7532
          CHECK(tryOk(Prod.iXTrayColor) && Pos(MTrayX) == Prod.iXTrayColor, "Color beyond Empty (HT9050 has no Color track; golden waits there): moves -- 1008 00:04 box");
          Prod.iXTrayColor = col; }
        const int ld = Prod.iXTrayLoad;  Prod.iXTrayLoad = 85000;                 // a Loader taught between Auto1 (90000) and Auto2 (80000)
        SetMotPos(MTrayX, Prod.iXTrayAuto[1]);
        CHECK(!TrayArmMotorMove(Prod.iXTrayLoad, true) && Pos(MTrayX) == Prod.iXTrayAuto[1], "Loader taught among the Autos: refused, the Tray Arm stays");
        Prod.iXTrayLoad = ld;
        CHECK(!TrayArmMotorMove(Prod.iXTrayLoad + 50000, true), "an unknown target outside [Empty, Loader]: refused");
        SetMotPos(MTrayX, Prod.iXTrayLoad);
    }
    printf("[TZ] TrayArmMotorMove, Type_HT9050: the Tray Arm moves only with the In / Out Arm ZA at or above their origin\n");
    {
        const int inR = InArmSuck.iMotRow, inC = InArmSuck.iMotCol, inM = InArmSuck.Suck[0][0].iMotNo;
        const int outR = OutArmSuck.iMotRow, outC = OutArmSuck.iMotCol, outM = OutArmSuck.Suck[0][0].iMotNo;
        InArmSuck.iMotRow = 1;  InArmSuck.iMotCol = 1;  InArmSuck.Suck[0][0].iMotNo = MInArmZA;     // the single picker (USE_PICKER_COUNT=4)
        OutArmSuck.iMotRow = 1; OutArmSuck.iMotCol = 1; OutArmSuck.Suck[0][0].iMotNo = MOutArmZA;
        auto tryMove = [](int target, int calls) { bool ok = false; for (int i = 0; i < calls && !ok; ++i) ok = TrayArmMotorMove(target); return ok; };
        MachineTypeChoice = Type_HT9050;
        SetMotPos(MOutArmZA, 0); SetMotPos(MInArmZA, -100); SetMotPos(MTrayX, Prod.iXTrayLoad);
        bool ok = tryMove(Prod.iXTrayEmpty, 20);
        CHECK(!ok && Pos(MTrayX) == Prod.iXTrayLoad, "In Arm ZA at -100 (down): TrayArmMotorMove false 20 times, the Tray Arm stays at the Loader");
        SetMotPos(MInArmZA, 0); SetMotPos(MOutArmZA, -1);
        ok = tryMove(Prod.iXTrayEmpty, 20);
        CHECK(!ok && Pos(MTrayX) == Prod.iXTrayLoad, "Out Arm ZA at -1 (one count below its origin): the Tray Arm stays");
        SetMotPos(MOutArmZA, 0);
        ok = tryMove(Prod.iXTrayEmpty, 200);
        CHECK(ok && Pos(MTrayX) == Prod.iXTrayEmpty, "both ZA at 0 (where the full HOME leaves them, lamp or not): the Tray Arm moves and arrives");
        SetMotPos(MInArmZA, 50); SetMotPos(MOutArmZA, 50); SetMotPos(MTrayX, Prod.iXTrayLoad);
        ok = tryMove(Prod.iXTrayEmpty, 200);
        CHECK(ok && Pos(MTrayX) == Prod.iXTrayEmpty, "both ZA up at their safe height (+50): moves");
        MachineTypeChoice = Type_HT9046_LS;
        SetMotPos(MInArmZA, -100); SetMotPos(MTrayX, Prod.iXTrayLoad);
        ok = tryMove(Prod.iXTrayEmpty, 200);
        CHECK(ok && Pos(MTrayX) == Prod.iXTrayEmpty, "Type_HT9046_LS: unchanged (no new interlock) -- moves with the In Arm ZA down");
        MachineTypeChoice = Type_HT9050;
        SetMotPos(MInArmZA, 0); SetMotPos(MOutArmZA, 0);
        InArmSuck.iMotRow = inR;  InArmSuck.iMotCol = inC;  InArmSuck.Suck[0][0].iMotNo = inM;
        OutArmSuck.iMotRow = outR; OutArmSuck.iMotCol = outC; OutArmSuck.Suck[0][0].iMotNo = outM;
    }

    // =======================================================================
    //  [LN] DoLoadNewICTray_9050 (910 :2466-2553)
    // =======================================================================
    printf("[LN] DoLoadNewICTray_9050: layer 0 -> true at once; 1>100>200>300>400>1; MES0920 answers\n");
    {
        std::vector<int> seq;
        iLoaderLayerCount_9050 = 0;
        InitLoadNewICTrayTask_9050();
        CHECK(DoLoadNewICTray_9050() == true && iLoadNewICTrayTask_9050 == 1, "layer 0: returns true at once, cursor stays 1 (910 :2473-2477)");

        iLoaderLayerCount_9050 = 3;
        MOT[MMTrayY].fHasTray = false;
        SetMotPos(MLoaderZ, 0);
        const int n = Pump(iLoadNewICTrayTask_9050, [] { return DoLoadNewICTray_9050(); }, 40, 1, seq);
        CHECK(n <= 40 && SeqIs(seq, { 1, 100, 200, 300, 400, 1 }), "layer 3: 1>100>200>300>400>1, returns true");
        CHECK(iLoaderLayerCount_9050 == 2 && MOT[MMTrayY].fHasTray && Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 2),
              "layer 3 -> 2 (case 100), MMTrayY gets the tray (case 300), MLoaderZ at TrayZ_Down[2]");

        // MES0920 (case 200): SnLoaderTrayHasTray reads OFF (Enable 1, Type 1, address 0/0/0/0 -> IsOff() true in both builds).
        Sen[SnLoaderTrayHasTray].Enable = true;
        Sen[SnLoaderTrayHasTray].Type = 1;
        CHECK(Sen[SnLoaderTrayHasTray].IsOff() == true, "fixture: SnLoaderTrayHasTray IsOff() true");
        W906_ShowErrorMessage_Reset();
        iLoaderLayerCount_9050 = 4;
        SetMotPos(MLoaderZ, 0);
        InitLoadNewICTrayTask_9050();
        int c = 0;
        while (W906_ShowErrorMessage_Count == 0 && c < 10) { DoLoadNewICTray_9050(); ++c; }
        CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == AnsiString("MES0920") &&
              W906_ShowErrorMessage_LastKCode == (K_RETRY | K_CLEAN_OUT | K_SKIP) && iLoadNewICTrayTask_9050 == 200 &&
              iLoaderLayerCount_9050 == 3,
              "no tray at TrayZ_Up[3]: MES0920 (RETRY / CLEAN OUT / SKIP); RETRY keeps case 200 and the layer");
        W906_ShowErrorMessage_SimReturn = K_SKIP;
        DoLoadNewICTray_9050();
        W906_ShowErrorMessage_SimReturn = K_RETRY;
        CHECK(W906_ShowErrorMessage_Count == 2 && iLoaderLayerCount_9050 == 2 && iLoadNewICTrayTask_9050 == 200,
              "SKIP: layer 3 -> 2, still case 200 (910 :2502-2505)");
        W906_ShowErrorMessage_SimReturn = K_CLEAN_OUT;
        c = 0;
        while (W906_ShowErrorMessage_Count == 2 && c < 10) { DoLoadNewICTray_9050(); ++c; }
        W906_ShowErrorMessage_SimReturn = K_RETRY;
        CHECK(W906_ShowErrorMessage_Count == 3 && iLoadNewICTrayTask_9050 == 250 && iLoaderLayerCount_9050 == 2,
              "CLEAN OUT: case 250 (910 :2506-2509)");
        bool done = false;
        c = 0;
        while (!done && c < 10) { done = DoLoadNewICTray_9050(); ++c; }
        CHECK(done && iLoadNewICTrayTask_9050 == 1 && Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 2), "case 250: MLoaderZ to TrayZ_Down[2], returns true");
        Sen[SnLoaderTrayHasTray].Enable = false;
        Sen[SnLoaderTrayHasTray].Type = 0;
        W906_ShowErrorMessage_Reset();
    }

    // =======================================================================
    //  [LD] DoLoad_9050 (910 :2561-2739) -- the probe static starts at 0: direct-entry checks first
    // =======================================================================
    printf("[LD] DoLoad_9050: guard, probe exits (MES0924 / boundary), 600, 800/900, boot path\n");
    {
        std::vector<int> seq;
        // (1) guard (910 :2569-2570): DoCatchFromLoader_9050 busy -> nothing
        iCatchFromLoader9050Task = 10;
        LoadTask_9050 = 600;
        iLoaderLayerCount_9050 = 2;
        MOT[MMTrayY].fHasTray = true;
        SetMotPos(MLoaderZ, 1234);
        DoLoad_9050();
        CHECK(LoadTask_9050 == 600 && Pos(MLoaderZ) == 1234, "iCatchFromLoader9050Task != 1: DoLoad_9050 returns at once");
        iCatchFromLoader9050Task = 1;

        // (2) case 1120 with the probe at its start value 0: 0 -> -1 -> MES0924, RETRY re-seeds the probe, Task 1 (910 :2704-2718)
        W906_ShowErrorMessage_Reset();
        LoadTask_9050 = 1120;
        DoLoad_9050();
        CHECK(W906_ShowErrorMessage_Count == 1 && W906_ShowErrorMessage_LastCode == AnsiString("MES0924") &&
              W906_ShowErrorMessage_LastKCode == K_RETRY && LoadTask_9050 == 1,
              "case 1120, probe 0 -> -1: MES0924 (RETRY only); RETRY -> probe = iLoaderInitDetectLayer, Task 1");

        // (3) case 1110 with the probe re-seeded to 3: 4, MLoaderZ to TrayZ_Up[4], sensor off -> layer 3 (910 :2670-2701)
        LoadTask_9050 = 1110;
        iLoaderLayerCount_9050 = 7;
        int n = 0;
        seq.clear(); seq.push_back(LoadTask_9050);
        while (LoadTask_9050 != 1 && n < 6) { DoLoad_9050(); ++n; if (LoadTask_9050 != seq.back()) seq.push_back(LoadTask_9050); }
        CHECK(SeqIs(seq, { 1110, 1115, 1 }) && iLoaderLayerCount_9050 == 3 && Pos(MLoaderZ) == W906_TrayZ9050(0, 1, 4),
              "case 1110: probe 3 -> 4, TrayZ_Up[4] has no tray -> iLoaderLayerCount_9050 = 4-1 = 3");

        // (4) case 600: a tray on MMTrayY, MLoaderZ off TrayZ_Down[layer]
        LoadTask_9050 = 1;
        iLoaderLayerCount_9050 = 2;
        MOT[MMTrayY].fHasTray = true;
        SetMotPos(MLoaderZ, 9999);
        n = Pump(LoadTask_9050, [] { DoLoad_9050(); return 0; }, 6, -77, seq);
        CHECK(SeqIs(seq, { 1, 600, 1 }) && Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 2), "case 1 -> 600 -> 1: MLoaderZ to TrayZ_Down[2], then stays 1");

        // (5) case 800: clean-out holds it; then 900 loads (layer 0 -> DoLoadNewICTray_9050 true at once)
        LoadTask_9050 = 1;
        iLoaderLayerCount_9050 = 0;
        MOT[MMTrayY].fHasTray = false;
        iCleanOut = 1;
        DoLoad_9050(); DoLoad_9050(); DoLoad_9050();
        CHECK(LoadTask_9050 == 800, "no tray on MMTrayY: case 800, held while iCleanOut==1 (910 :2612-2617)");
        iCleanOut = 0;
        n = Pump(LoadTask_9050, [] { DoLoad_9050(); return 0; }, 2, -77, seq);
        CHECK(SeqIs(seq, { 800, 900, 1 }), "iCleanOut 0: 800 -> 900 -> 1 (layer 0 loads nothing, so a 3rd call goes 1 -> 800 again, 910 as written)");

        // (6) boot path: layer -1 and no tray -> case 1000
        LoadTask_9050 = 1;
        iLoaderLayerCount_9050 = -1;
        MOT[MMTrayY].fHasTray = false;
        W906_ShowErrorMessage_Reset();
#ifdef SOFT_SIMULTE
        n = Pump(LoadTask_9050, [] { DoLoad_9050(); return 0; }, 40, -77, seq);
        CHECK(SeqIs(seq, { 1, 1000, 1, 800, 900, 1 }), "SIM: 1>1000>1>800>900>1 (case 1000 sets layer 5, 910 :2630-2633)");
        CHECK(iLoaderLayerCount_9050 == 4 && MOT[MMTrayY].fHasTray && Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 4) && W906_ShowErrorMessage_Count == 0,
              "SIM: one tray loaded from layer 5 -> layer 4, MMTrayY has it, MLoaderZ at TrayZ_Down[4]");
#else
        seq.clear(); seq.push_back(LoadTask_9050);
        n = 0;
        while (W906_ShowErrorMessage_Count == 0 && n < 60) { DoLoad_9050(); ++n; if (LoadTask_9050 != seq.back()) seq.push_back(LoadTask_9050); }
        CHECK(SeqIs(seq, { 1, 1000, 1100, 1120, 1125, 1120, 1125, 1120, 1125, 1120, 1 }),
              "SHIP: drawer sensor not off -> probe TrayZ_Up[3] (1100), then [2] [1] [0] (1120/1125 x3), -1 -> MES0924 -> 1");
        CHECK(W906_ShowErrorMessage_LastCode == AnsiString("MES0924") && iLoaderLayerCount_9050 == -1 && LoadTask_9050 == 1,
              "SHIP: no tray found at any layer (sensors disabled): MES0924, layer stays -1");
#endif
        W906_ShowErrorMessage_Reset();
    }

    // =======================================================================
    //  [RB] DoReceiveAllToBottom_9050 (910 csystem.cpp:7626-7714)
    // =======================================================================
    printf("[RB] DoReceiveAllToBottom_9050: five areas to their Home; a tray is pressed into the stack first\n");
    {
        const int motZ[5] = { MLoaderZ, MEmptyZ, MAuto1Z, MAuto2Z, MAuto3Z };
        const int mm[5]   = { MMTrayY, MMEmpty, MMAuto1, MMAuto2, MMAuto3 };   //AI(W906-F9050-FIX2) 20261006: Loader area MMTrayY (Frank 1006「改」; 910 MMTrayZ)
        const int home[5] = { W906_TrayZ9050Home(0), W906_TrayZ9050Home(1), W906_TrayZ9050Home(2), W906_TrayZ9050Home(3), W906_TrayZ9050Home(4) };   //AI(W906-TRAYZ-PITCH) 20261006: was the Prod *_Home fields
        for (int i = 0; i < 5; ++i) { MOT[mm[i]].fHasTray = false; SetMotPos(motZ[i], 7000 + 10 * i); }
        int n = 0;
        bool done = false;
        while (!done && n < 20) { done = DoReceiveAllToBottom_9050(); ++n; }
        bool atHome = true;
        for (int i = 0; i < 5; ++i) if (Pos(motZ[i]) != home[i]) atHome = false;
        CHECK(done && n == 3 && atHome, "all MM empty: 1 -> 90 -> 100 in every area, true on the 3rd call, five Z at their Home");

        iLoaderLayerCount_9050 = 2;
        MOT[MMTrayY].fHasTray = true;
        MOT[MMTrayZ].fHasTray = true;                                           // the Z motor's object: must not be what Receive reads or clears
        n = 0; done = false;
        bool sawUp = false, sawDown = false;
        while (!done && n < 20)
        {
            done = DoReceiveAllToBottom_9050(); ++n;
            if (Pos(MLoaderZ) == W906_TrayZ9050(0, 1, 2)) sawUp = true;
            if (Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 3)) sawDown = true;
        }
        CHECK(done && sawUp && sawDown && Pos(MLoaderZ) == W906_TrayZ9050Home(0),
              "tray on MMTrayY: MLoaderZ TrayZ_Up[2] (case 10) -> TrayZ_Down[3] (case 30) -> TrayZ_Home (case 90)");
        CHECK(iLoaderLayerCount_9050 == -1 && MOT[MMTrayY].fHasTray == false,
              "layer 2 -> 3 (case 20) -> -1 (case 30, 910 as written); MMTrayY emptied (case 20, Frank 1006; 910 MMTrayZ)");
        CHECK(MOT[MMTrayZ].fHasTray == true, "MMTrayZ (the Z motor's) untouched by Receive");
        MOT[MMTrayZ].fHasTray = false;
    }

    // =======================================================================
    //  [H2120] defect (b) through the dispatcher: ToBuffer=2 waits in CatchTrayTask 2120 for ever, no alarm
    // =======================================================================
    //AI(W906-F9050-FIX2) 20261006: was "[H2120] ... ToBuffer=2: no progress, no alarm" (defect (b) pinned as 910 wrote it); Frank 1006 routes the
    //  Loader's empty tray to Empty only, so the dispatcher no longer reaches DoPlaceToBuffer_9050 case 200 (NIGHT_REPORT s0 #105, W-85).
    printf("[H2120] DoCatchTray case 2120, Type_HT9050, ToBuffer=2: goes to Empty, never DoPlaceToBuffer_9050 case 200 (Frank 1006)\n");
    {
        MachineTypeChoice = Type_HT9050;
        Globals();
        TrayForm.LoaderToEmptyColor[0] = 2; TrayForm.LoaderToEmptyColor[1] = 2;
        SetMotPos(MTrayX, Prod.iXTrayEmpty);
        InitialPlaceToBuffer_9050();
        CatchTrayTask = 2120;
        bool saw200 = false, saw100 = false;
        for (int i = 0; i < 60; ++i)
        {
            DoCatchTray();
            if (iPlaceToBuffer9050Task == 200) saw200 = true;
            if (iPlaceToBuffer9050Task == 100) saw100 = true;
        }
        CHECK(!saw200 && iPlaceToAuto9050Task == 1, "60 DoCatchTray calls: DoPlaceToBuffer_9050 never in case 200, the Auto placer never entered");
        CHECK(saw100 || CatchTrayTask != 2120, "ToBuffer=2 now goes the Empty way (case 100) instead of waiting in 2120 for ever");
        InitAllProcessTask();
        CHECK(CatchTrayTask == 1, "InitAllProcessTask resets CatchTrayTask");
        InitialPlaceToBuffer_9050();
        TrayForm.LoaderToEmptyColor[0] = 0; TrayForm.LoaderToEmptyColor[1] = 0;
    }

    // =======================================================================
    //  [D] dispatch arms, both types (sentinels: 77 / 31337 = "this cursor was not touched")
    // =======================================================================
    for (int pass = 0; pass < 2; ++pass)
    {
        const bool is9050 = (pass == 1);
        MachineTypeChoice = is9050 ? Type_HT9050 : Type_HT9046_LS;
        Globals();
        printf("[D] dispatch with MachineTypeChoice = %s\n", is9050 ? "Type_HT9050" : "Type_HT9046_LS (what 9050GPIB decodes to today)");

        // D1 InitAllProcessTask (910 csystem.cpp:5836-5839)
        LoadTask = 77; LoadTask_9050 = 77;
        InitAllProcessTask();
        CHECK(is9050 ? (LoadTask_9050 == 1 && LoadTask == 77) : (LoadTask == 1 && LoadTask_9050 == 77),
              is9050 ? "InitAllProcessTask -> InitLoadTask_9050 (golden LoadTask untouched)" : "InitAllProcessTask -> InitLoadTask (LoadTask_9050 untouched)");

        // D2 DoLoad (910 asendic_Loader.cpp:2761-2765): the 9050 arm runs before golden's bOneTimeHotPlateCheckAll return
        bOneTimeHotPlateCheckAll = true;
        iCatchFromLoader9050Task = 1;
        LoadTask_9050 = 600;
        iLoaderLayerCount_9050 = 2;
        MOT[MMTrayY].fHasTray = true;
        SetMotPos(MLoaderZ, 7777);
        const int loadTask0 = LoadTask;
        for (int i = 0; i < 4; ++i) DoLoad();
        CHECK(is9050 ? (LoadTask_9050 == 1 && Pos(MLoaderZ) == W906_TrayZ9050(0, 0, 2))
                     : (LoadTask_9050 == 600 && LoadTask == loadTask0 && Pos(MLoaderZ) == 7777),
              is9050 ? "DoLoad -> DoLoad_9050 regardless of bOneTimeHotPlateCheckAll (910 order): case 600 done"
                     : "DoLoad: golden returns at bOneTimeHotPlateCheckAll, DoLoad_9050 not called");
        bOneTimeHotPlateCheckAll = false;

        // D3 DoInspectTrayColorOnLoader case 100 (910 :1943-1945) -- reachable only with a colour sensor "installed"
        {
            TfTrayForm *const saved = fTrayForm;
            TColourOn colourOn;
            fTrayForm = &colourOn;
            iInspectTrayColorOnLoaderTask = 100;
            iLoadNewICTrayTask = 31337;              // no such case in golden DoLoadNewICTray -> it returns false
            iLoadNewICTrayTask_9050 = 1;
            iLoaderLayerCount_9050 = 0;              // DoLoadNewICTray_9050 returns true at once
            DoInspectTrayColorOnLoader(false);
            fTrayForm = saved;
            CHECK(is9050 ? (iInspectTrayColorOnLoaderTask == 9999 && iLoadNewICTrayTask == 31337)
                         : (iInspectTrayColorOnLoaderTask == 100 && iLoadNewICTrayTask == 31337),
                  is9050 ? "case 100 -> DoLoadNewICTray_9050 (true) -> 9999 (no loader car tray)"
                         : "case 100 -> golden DoLoadNewICTray (false, cursor 31337 untouched) -> stays 100");
        }

        // D4 DoCatchTray case 200 (910 :7263-7268)
        iCatchFromLoaderTask = 77; iCatchFromLoader9050Task = 77;
        CatchTrayTask = 200;
        DoCatchTray();
        CHECK(CatchTrayTask == 250 && (is9050 ? (iCatchFromLoader9050Task == 1 && iCatchFromLoaderTask == 77)
                                              : (iCatchFromLoaderTask == 1 && iCatchFromLoader9050Task == 77)),
              is9050 ? "case 200 -> InitialCatchFromLoader_9050, Task 250" : "case 200 -> InitialCatchFromLoader, Task 250");

        // D5 DoCatchTray case 250 (910 :7311-7316) with Tray X not safe to move
        TRAY_ARM_MODE = eUnderCoveyor;
        MOT[MTrayZ].Led[iHomeLed] = false;
        iCatchFromLoaderTask = is9050 ? 77 : 1;
        iCatchFromLoader9050Task = is9050 ? 1 : 77;
        CatchTrayTask = 250;
        DoCatchTray();
        CHECK(is9050 ? (CatchTrayTask == 5200 && iCatchFromLoader9050Task == 1 && iCatchFromLoaderTask == 77)
                     : (CatchTrayTask == 250 && iCatchFromLoaderTask == 50 && iCatchFromLoader9050Task == 77),
              is9050 ? "case 250 -> DoCatchFromLoader_9050 returns 3 -> Task 5200" : "case 250 -> golden DoCatchFromLoader case 1 -> its 50, Task stays 250");

        // D6 DoCatchTray case 2120 (910 :8087-8091), still not safe to move
        TrayForm.LoaderToEmptyColor[0] = 0; TrayForm.LoaderToEmptyColor[1] = 0;
        iPlaceToBufferTask = is9050 ? 77 : 1;
        iPlaceToBuffer9050Task = is9050 ? 1 : 77;
        CatchTrayTask = 2120;
        DoCatchTray();
        CHECK(CatchTrayTask == 2120 && (is9050 ? (iPlaceToBuffer9050Task == 10 && iPlaceToBufferTask == 77)
                                               : (iPlaceToBufferTask == 2150 && iPlaceToBuffer9050Task == 77)),
              is9050 ? "case 2120 -> DoPlaceToBuffer_9050 (1 -> 10)" : "case 2120 -> golden DoPlaceToBuffer (1 -> 2100 -> 2150)");
        TRAY_ARM_MODE = eAboveCoveyor;

        // D7 DoCatchTray case 6100 (910 :8591-8596): Tray X already at the (0) Empty / Load teach point
        const int savedEmpty = Prod.iXTrayEmpty, savedLoad = Prod.iXTrayLoad;
        Prod.iXTrayEmpty = 0; Prod.iXTrayLoad = 0;
        SetMotPos(MTrayX, 0);
        bTrayHaveDevice = false;
        iCatchFromLoaderTask = 77; iCatchFromLoader9050Task = 77;
        CatchTrayTask = 6100;
        DoCatchTray();
        CHECK(CatchTrayTask == 250 && (is9050 ? (iCatchFromLoader9050Task == 1 && iCatchFromLoaderTask == 77)
                                              : (iCatchFromLoaderTask == 1 && iCatchFromLoader9050Task == 77)),
              is9050 ? "case 6100 -> InitialCatchFromLoader_9050, Task 250" : "case 6100 -> InitialCatchFromLoader, Task 250");
        Prod.iXTrayEmpty = savedEmpty; Prod.iXTrayLoad = savedLoad;
        iCatchFromLoader9050Task = 1; iPlaceToBuffer9050Task = 1; LoadTask_9050 = 1;
    }
    MachineTypeChoice = Type_HT9045;          // the static default (cmydef.cpp)
    { void TrayZPitchFormula(); TrayZPitchFormula(); }   //AI(W906-TRAYZ-PITCH) 20261006: [TZ] the formula itself, end of file; same line

    printf("==== Flow9050_Tray: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

// ===========================================================================
//  [TZ] AI(W906-TRAYZ-PITCH) 20261006: HT9050 tray-stack Z = base layer - layer * pitch (cinitial.cpp EOF), EastSun 1006
//       「每一層一定是有一個基準層 用pitch算上去」「基準層pitch要用減的」「要可以讓我設探測極限」. Pitch positive as EastSun teaches it.
// ===========================================================================
int W906_TrayZ9050Lift(int zone); int W906_TrayZ9050ProbeStart(); int W906_TrayZ9050ProbeLimit();
void TrayZPitchFormula()
{
    printf("[TZ] Tray Z = base - layer*pitch: five zones, layers -1 / 0 / 19, Up / Down separate bases, Home / lift / probe\n");
    for (int z = 0; z < 5; ++z)
    {
        Tech.iTrayZ9050UpBase[z] = 50000 + 1000 * z; Tech.iTrayZ9050DownBase[z] = 40000 + 1000 * z;
        Tech.iTrayZ9050Pitch[z] = 1200 + 10 * z;     Tech.iTrayZ9050Home[z] = 900 + z; Tech.iTrayZ9050Lift[z] = 70 + z;
    }
    bool ok = true;
    for (int z = 0; z < 5; ++z)
        for (int layer = -1; layer <= 19; ++layer)
        {
            const int p = 1200 + 10 * z;
            if (W906_TrayZ9050(z, 1, layer) != 50000 + 1000 * z - layer * p) ok = false;
            if (W906_TrayZ9050(z, 0, layer) != 40000 + 1000 * z - layer * p) ok = false;
        }
    CHECK(ok, "every zone, layers -1..19: Up = UpBase - layer*pitch, Down = DownBase - layer*pitch (no array, no out-of-bounds read)");
    CHECK(W906_TrayZ9050(0, 1, 0) == 50000 && W906_TrayZ9050(0, 1, 1) == 48800 && W906_TrayZ9050(0, 1, 2) == 47600,
          "Loader: layer 0 = base, each layer one pitch LESS (EastSun「基準層pitch要用減的」)");
    CHECK(W906_TrayZ9050Home(0) == 900 && W906_TrayZ9050Home(4) == 904 && W906_TrayZ9050Lift(2) == 72,
          "Home and hand-over lift per zone");
    CHECK(W906_TrayZ9050(5, 1, 0) == 0 && W906_TrayZ9050(-1, 0, 0) == 0 && W906_TrayZ9050Home(7) == 0 && W906_TrayZ9050Lift(-2) == 0,
          "a zone outside 0..4 gives 0, never a neighbour's value");
    Tech.iTrayZ9050ProbeStart = 7; Tech.iTrayZ9050ProbeLimit = 0;
    CHECK(W906_TrayZ9050ProbeStart() == 7 && W906_TrayZ9050ProbeLimit() == MAX_LOADER_LAYER_9050,
          "probe start taught; probe limit unset (0) = the old hard-coded 20");
    Tech.iTrayZ9050ProbeLimit = 12;
    CHECK(W906_TrayZ9050ProbeLimit() == 12, "probe limit taught (EastSun「要可以讓我設探測極限」)");
    Tech.iTrayZ9050ProbeLimit = 0;
}