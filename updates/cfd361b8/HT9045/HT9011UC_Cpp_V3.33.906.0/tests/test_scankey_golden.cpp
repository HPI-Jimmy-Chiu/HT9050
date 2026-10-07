// =============================================================================
//  tests/test_scankey_golden.cpp -- AI(W906-ST02-SKG) 20261003 (St02-E helper).  Suite: ScanKeyGolden (both configs).
//
//  ScanKey combined MR step 5: St02's S-20 panel-key test (tests/test_scankey.cpp at 74776f87, sections [1]-[17], never
//  merged) ported onto MC01's panel-key implementation WebMainScanKey.cpp (901fb732 + St02's R188 M1 / M2 / first-scan-guard
//  commits).  golden = 906 0618 (D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618, cp950); ":N" below = golden 0618 main.cpp:N.
//  WebMainScanKey.cpp is a wb_serve-only TU, compiled in here as wb_serve compiles it.  MC01's own ctest MainScanKey
//  (tests/test_main_scankey.cpp) feeds keys through the *With(scan) entries into a FakeMain; THIS test drives the production
//  entry W906_MainScanKeyTick(noticeUp) = the REAL ScanPannelKey (ckernel.cpp) through the sim sensor seam of
//  tests/test_sysinit_boot.cpp (TYPE_B reads ON offline) and the REAL fMain (forms/fMain.cpp): hold a key, one beat (Beat()),
//  release, clear the latch.  W906_RemoteRun (forms/fMain.h) gets fakes -- Start counts and keeps the Func; Pause counts.
//
//  S-20 -> here (the entry and the seams):
//    W906_TimerScanKeyTimer()               -> Beat() = W906_MainScanKeyTick(g_noticeUp) (WebMainScanKey.cpp:455, real ScanPannelKey)
//    W906_ScanKeyNoticeUp() stub            -> the noticeUp argument, g_noticeUp (wb_serve passes "a notice in the alarm slot")
//    FileRW_ContactOneCycleFinish() stub    -> MC01's file-static s_bOneCycleFinish, set on the Contact page's closed->open edge
//                                              (MskContactFormShowEdge, WebMainScanKey.cpp:63-75 = golden TfContact::FormShow
//                                              cContact.cpp:1192-1205): ContactPageOpens(icOnBoard) below
//    FileRW_ContactOneCycleProcess() stub   -> W906_Contact_OneCycleProcess, a counting stub (wb_serve's is FileRW/DeviceForm_File.cpp
//                                              EOF; MC01's tests/test_main_scankey_stubs.cpp copy belongs to MC01's exe)
//    FormTryLock / FormUnlock stubs         -> ht9045::formjson::FormLock / FormUnlock, depth-counted (blocking in wb_serve,
//                                              JsonBridge/FormJson.cpp): taken and released once by CLEAN OUT / ONE CYCLE / TRAY
//                                              FEED, free (depth 0) at START / PAUSE / HOME / RESET (WebMainScanKey.cpp:19-20)
//    W906_MsgBoxModelessPanelKeyTick        -> a counting stub, as S-20 ([16])
//  ROWS: Row(code, S, Debug) = NewRecordProcess's ExString AND the row it appended to the HANDLER LOG since the press
//    (cMyDB.cpp MyDBIProcessNew -> SaveEventLogInfo(Code, S, 22, " ", Debug), under ctest's asSaveEventLogPath); LogRow() = the
//    HANDLER LOG half only; NoRow() = no " pressed" row at all.
//
//  SECTIONS (as S-20 unless marked; CHECK ids = REVERSE_SKG.md):
//    [0] fixture: the form globals exist
//    [1] guards: InitialOK / employee-ID / S10F3 (+ the note lifting it) / fNote / a notice (noticeUp) / MyMessageBox / the IO
//        page -> nothing runs and the key stays for the box's reader
//    [2] START -> W906_RemoteRunStart("ScanKey_2") once, outside the form lock; refusals: the Home dialog (:2514), the Contact
//        page opened with an IC on board (MES1645), the AutoClean arm checks, SPIL offline, the two bin checks, [A56] auto
//        alignment; the AAL flags; the seat not installed
//    [3] re-entry: a box inside START serves the clock again -> the nested beat returns at bRunTimer1
//    [4] PAUSE -> MES2111 "ScanKey_1", BtnPauseClick -> the Pause seat, outside the form lock; the Home dialog up with its
//        Timer1 on: TfHome::ScanKey reads the pad FIRST (abort + close) and the main ScanKey gets nothing
//    [5] PAUSE (2) [bFinishSuckAfterPause]
//    [6] HOME -> the real TfMain::Home("ScanKey") (MES2112, SoftStart, iHome); BtnHome disabled; [G06]; the two bin checks
//    [7] RESET -> MES2113, the kept BtnResetClick no-op, golden's tail (:2579-2586), outside the form lock; its refusals
//    [8] CLEAN OUT -> BtnCleanOutClick under the form lock (taken once, released); refusals take no lock
//    [9] ONE CYCLE -> BtnOneCycleClick under the form lock; the Contact page -> MES2115 "Contact Test" + OneCycleProcess,
//        no lock in ScanKey (the callee takes it, pinned in [14])
//   [10] TRAY FEED -> InitialTrayFeedTask("ScanKey") under the form lock; refusals take no lock
//   [11] ALARM RESET   [12] POWER OFF / ON (front and rear pad)   [13] TRAY END [AutoSKIP + bASkStart]
//   [14] source pins, REWRITTEN for WebMainScanKey.cpp and the wb_serve hooks (argv[1] = tree root, read only)
//   [15] [W906] first-scan guard (W906_ScanKeyFirstScanRearm_St02, WebMainScanKey.cpp:145)
//   [16] a MyMessageBox up: the non-modal box's panel-key reader once per beat, the key stays; wb_serve's reader pinned
//   [17] panel HOME while START is blocked: TfMain::Home's W906_HomeBlockedHook (forms/fMain.cpp, !146) refuses first;  [18] [15] with the RS-232 pad (ST02-P1);  [19] screen panel keys via golden's bAse* (ST02-P2)
//
//  ADAPTED (MC01 differs on purpose -- the S-20 expectation changed, why):
//    [2] Home dialog: MC01 runs TfHome::ScanKey first in the tick (WebMainScanKey.cpp:439) while TfHome's Timer1 is on (boot
//        value, uhome.cpp:649), so it -- not :2514 -- would take the START; the :2514 check runs with Timer1 off (golden after
//        the first Home FormClose, uhome.cpp:661), and a second CHECK pins that TfHome::ScanKey ignores START (golden uhome.cpp:4878)
//    [2] Contact page: S-20's g_contactFinish false / true -> ContactPageOpens(true) / (false) (the page's open edge decides)
//    [2] SPIL offline supervisor: S-20 = START goes on (re-login GATEd); MC01 fails closed (WebMainScanKey.cpp:254-269, the
//        login box cannot be opened from a panel key) -> refused, no dialog, its RecordProcess line; + control: the
//        setup-teach flow (bNeedSetupTeach, :2475) skips the re-login in golden -> START goes on
//    [2] [A56] AutoTeach: S-20 = GATE (golden's answer always true); MC01 fails closed while the manual-step auto teach is on
//        (WebMainScanKey.cpp:105-114) -> refused; + control: [A56] alone -> START goes on
//    [2] [4] [7] [17] + the form lock is free when START / PAUSE / RESET's tail / HOME run (S-20 had no lock there)
//    [4] + TfHome::ScanKey first: the Home dialog up, Timer1 on, PAUSE -> abort + Close, no main-screen pause
//    [8] [10] + FormLock taken once and released around BtnCleanOutClick / InitialTrayFeedTask (MskFormLock); none on a refusal
//    [9] S-20 "FormTryLock taken and released around OneCycleProcess" -> MC01 takes no lock in ScanKey for the Contact branch
//        (W906_Contact_OneCycleProcess takes it itself); + FormLock taken once and released around BtnOneCycleClick
//   [14] every pin rewritten (list in the section)
//  DROPPED (S-20 CHECK -> why):
//    [9]  "the page holds the form lock: skip this beat, no toggle" -- MC01's FormLock blocks (no FormTryLock, no busy path)
//    [14] fast clock registers W906_TimerScanKeyTimer at 30 ms -- MC01 ticks from the main loop + the drag keepalive; replaced by
//         the two wb_serve tick pins and "no ScanKey job on the fast clock"
//    [14] wb_serve.cpp defines W906_ScanKeyNoticeUp on the mailbox slot -- no such function; replaced by the noticeUp expression pin
//    [14] DeviceForm_File.cpp defines FileRW_ContactOneCycleFinish / FileRW_ContactOneCycleProcess -- no such accessors; replaced
//         by the W906_Contact_OneCycleProcess pin (it takes the form lock itself)
//    [14] MainScanKey.cpp "if(ht9045::formjson::FormTryLock())" + one FormUnlock -- replaced by the MskFormLock pin (3 uses)
//    [14] CMakeLists.txt "FastClockWbServe.cpp  MainScanKey.cpp" -- replaced by "WebMainCtlButtons.cpp  WebMainScanKey.cpp"
//  NOT COVERED (no seam): `Key==SnRKTrayEnd` (:2646) -- ScanPannelKey never answers SnRKTrayEnd; the OEE ClickPause (not called,
//    V912's fix), the [I52] AQL GATE (a console line only), labSecsGemLock (no widget) -- as S-20.
//  CONTROLS (expected, NOT run -- compiled only on STEVEN-NB3; for St01's proxy run): one per CHECK in REVERSE_SKG.md (St02
//    scratchpad skt/).
//  Writes: the bodies' NewRecordProcess / RecordProcess rows (cMyDB log roots) -> ctest's machine_log_scratch; the guard refuses
//    elsewhere.  The HANDLER LOG is only read back.  The source files of [14] / [16] are only read.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "ckernel.h"                    // ScanPannelKey
#include "csystem.h"                    // W906_Stop1203AllHook (TfHome::GaliMotorServoOff's 1203 hook), IndexHasIC / ShuttleHasIC
#include "mysensor.h"                   // Sen[]
#include "myswitch.h"                   // SW[] (the two alarm-reset lamps)
#include "mycylin.h"                    // Cylinder[] (the RESET tail)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h virtual defaults (not this file's code; test_b8_ct3a_contactflags.cpp)
#include "Motor/mymotor.h"              // MOT[MMTrayY].Tray (TRAY END)
#include "w906_test_motors.h"           // W906_TestEnsureSimMotors: golden's boot invariant MOT[].Motor != NULL (GaliMotorServoOff)
#pragma GCC diagnostic pop
#include "canary_support.h"             // W906_ShowErrorMessage_* / W906_ShowMyMessage_* seams
#include "common.h"                     // as9045LogPath / asSaveEventLogPath
#include "SECSGEM/SecsEventReport.h"    // g_SimLastEventReportCeid / g_SimEventReportCount / ResetSimEventReport
#include "SECSGEM/SecsEventType.h"      // SECS_EVENT
#include "aHotPlateSubstrate.h"         // InArmSuck (PAUSE (2), AAL, the Contact page's IC test)
#include "forms/fMain.h"
#include "forms/fNote.h"
#include "forms/fHome.h"
#include "forms/fSortCT.h"              // fSortCT->pnlLoad (TRAY END)
#include "atester_shims.h"              // fContact, fiosetview
#include "mymessbox_shim.h"             // MyMessageBox
#include "JsonBridge/FormJson.h"        // ht9045::formjson::FormLock / FormUnlock (stubbed below)
#include "w906_ctest_guard.h"
#include "PadInterface_St02.h"          // AI(W906-ST02-P1) 20261005 (St02-E): fPadInterface -- [18] the RS-232 pad (ControlPanelMode=1)
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

void W906_MainScanKeyTick(bool noticeUp);                                        // WebMainScanKey.cpp:455 (the production entry)
void W906_ScanKeyFirstScanRearm_St02();                                          // WebMainScanKey.cpp:145: [15] the first-scan guard's boot state
extern bool W906_HomeTimer1Enabled;                                              // uhome.cpp:649 (golden TfHome::Timer1->Enabled)
void GetTimeInfo();                                                              // cpublic.h:23 (the HANDLER LOG's date globals)
extern AnsiString ExString;                                                      // cMyDB.cpp (NewRecordProcess's / RecordProcess's last row text)
extern bool SECS_GEM_PPMUSIC_CONTROL_flag;                                       // ckernel.cpp
extern bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag;                                 // ckernel.cpp

// ---- the wb_serve-only names WebMainScanKey.cpp calls: stubs (this exe only) ---------------------------------------------
static int g_lockDepth = 0;                                                      // the form lock's depth now (0 = free)
static int g_lockCalls = 0;                                                      // FormLock calls so far
namespace ht9045 { namespace formjson {
void FormLock()   { ++g_lockDepth; ++g_lockCalls; }                              // JsonBridge/FormJson.cpp (wb_serve): blocking
void FormUnlock() { --g_lockDepth; }
} }
static int g_contactOneCycle = 0;
void W906_Contact_OneCycleProcess() { ++g_contactOneCycle; }                     // FileRW/DeviceForm_File.cpp EOF (wb_serve)
static int g_modelessTicks = 0;
void W906_MsgBoxModelessPanelKeyTick() { ++g_modelessTicks; }                    // tools/wb_serve.cpp EOF (NB2 R188 M2): [16]

static bool g_noticeUp = false;                                                  // wb_serve: g_alarmSlot.kind == kNotice ([14])
static bool g_interlockBlocks = false;   // [17]: the fake W906_HomeBlockedHook's answer and how often it was asked -- the HOME
static int  g_interlockAsked = 0;        //   interlock is TfMain::Home's (NB2-1 !146); IdleHomed resets both
static int  g_homeLockDepth = -1;
static std::string g_homeAskedFunc;
extern bool (*W906_HomeBlockedHook)(const char* func, std::string& why);    // forms/fMain.cpp (!146 HOMEBLOCK)
extern int W906_HomeRefusedCount;
static bool FakeHomeBlocked(const char* func, std::string& why)
{
    ++g_interlockAsked;
    g_homeAskedFunc = func ? func : "";
    g_homeLockDepth = g_lockDepth;
    if (g_interlockBlocks) why = "[17] fake: the Teach Arm Cell job runs";
    return g_interlockBlocks;
}

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- one beat of the production tick (wb_serve: every main-loop pass + the drag keepalive) -------------------------------
static void Beat() { W906_MainScanKeyTick(g_noticeUp); }

// ---- W906_RemoteRun fakes (the seat wb_serve fills with TfMainWeb::StartFromWeb / PauseFromWeb) --------------------------
static int         g_starts = 0, g_pauses = 0;
static int         g_startLockDepth = -1, g_pauseLockDepth = -1;
static std::string g_startFunc, g_pauseFunc;
static bool        g_boxInStart = false;                                         // [3]: a box inside Start serves the clock
static int         g_nestedKey = -1;
static void SimOn(int idx)      { Sen[idx].Enable = true;  Sen[idx].Type = TYPE_B; Sen[idx].ISABase = eISABase; }   // test_sysinit_boot.cpp seam
static void SimOff(int idx)     { Sen[idx].Enable = true;  Sen[idx].Type = TYPE_A; Sen[idx].ISABase = eISABase; }
static void SimUnknown(int idx) { Sen[idx].Enable = false; }
static bool FakeStart(AnsiString f)
{
    ++g_starts;
    g_startFunc = f.c_str();
    g_startLockDepth = g_lockDepth;
    if (g_boxInStart) {                                                          // golden: Start opens a box, ShowModal's loop fires
        g_boxInStart = false;                                                    // TimerScanKey again while ScanKey is on the stack
        SimOff(SnFKStart);                                                       // let go of START first: ScanPannelKey answers the LAST
        SimOn(SnFKPause);                                                        // key it tests (START after PAUSE, ckernel.cpp), latched
        Beat();
        g_nestedKey = ScanPannelKey();                                           // still there: the nested beat did not read it
        SimOff(SnFKPause);
        ScanPannelKey();                                                         // release sweep
    }
    return true;
}
static bool FakePause(AnsiString f) { ++g_pauses; g_pauseFunc = f.c_str(); g_pauseLockDepth = g_lockDepth; return true; }
static int  g_servoOffs = 0, g_stopLockDepth = -1;
static void FakeStop1203(const char*) { ++g_servoOffs; g_stopLockDepth = g_lockDepth; }   // TfHome::GaliMotorServoOff calls it (uhome.cpp:4970)

// ---- rows: NewRecordProcess's ExString + the row it appends to the HANDLER LOG -------------------------------------------
static std::string g_logPath;
static size_t      g_logMark = 0;
static std::string ReadFileAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static std::string HandlerLogFile()          // cMyDB.cpp SaveEventLogInfo: asSaveEventLogPath\HANDLER LOG_<SocketHandlerID>_yyyy_mm_dd.csv
{
    char n[300];
    std::snprintf(n, sizeof(n), "\\HANDLER LOG_%s_%04d_%02d_%02d.csv", IniConfig.SocketHandlerID.c_str(),
                  (int)SystemYear, (int)SystemMonth, (int)SystemDate);
    return std::string(asSaveEventLogPath.c_str()) + n;
}
static void MarkLog() { g_logPath = HandlerLogFile(); g_logMark = ReadFileAll(g_logPath).size(); }
static std::string NewLogText()             // what the HANDLER LOG got since MarkLog, '\r' dropped (text-mode fputs)
{
    const std::string p = HandlerLogFile();
    const std::string all = ReadFileAll(p);
    const std::string s = (p == g_logPath && all.size() >= g_logMark) ? all.substr(g_logMark) : all;
    std::string o;
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] != '\r') o += s[i];
    return o;
}
static std::string EventCode(const char* mes)   // cMyDB.cpp MyDBIProcessNew: Code.sprintf("2%02d%06d", SubString(4, 2), SubString(4, ...))
{
    char b[40];
    std::snprintf(b, sizeof(b), "2%02d%06d", std::atoi(std::string(mes + 3, 2).c_str()), std::atoi(mes + 3));
    return b;
}
static bool LogRow(const char* code, const char* s, const char* debug)
{
    const std::string want = "," + EventCode(code) + ",PROCESS," + s + "," + debug + "\n";
    const std::string got = NewLogText();
    const bool inLog = got.find(want) != std::string::npos;
    if (!inLog)
        std::printf("    (HANDLER LOG %s: no \",%s,PROCESS,%s,%s\" since the press; it got: %s)\n",
                    HandlerLogFile().c_str(), EventCode(code).c_str(), s, debug, got.c_str());
    return inLog;
}
static bool Row(const char* code, const char* s, const char* debug) { return LogRow(code, s, debug) && ExString == AnsiString(s); }
static bool NoRow() { return ExString == AnsiString("") && NewLogText().find(" pressed,") == std::string::npos; }
static bool Has(const AnsiString& a, const char* s) { return std::string(a.c_str()).find(s) != std::string::npos; }

// ---- fixture ----------------------------------------------------------------------------------------------------------
static void AllKeysUnknown()
{
    for (int i = 0; i < 32; ++i) SimUnknown(i);                                  // every front / rear key id + SnRKCoverOpen (27)
    SimUnknown(SnRearPadActive);
}
static void IdleHomed()                                                          // what the guards look at (test_main_ctlbuttons.cpp)
{
    InitialOK = true;  SystemInitialOK = true;
    SystemStart = false;  SoftStart = false;  fAllMotorHome = true;
    iOneCycle = 0;  iCleanOut = 0;  iTrayFeed = 0;  iTrayFeedTask = 0;  iHome = 0;  iReset = 0;
    bSECSGEMAlarm = false;  bSECSGEM_NoteAlarm = false;  bEnableEmployeeIDCheck = false;
    bManualOneCycle = false;  bDoEmptySocketOneCycle = false;  bIsAutoOneCycle = false;  bOneCycle_BackUp = false;
    bRunAutoClean = false;  bUnloading = false;  bAlarmBuzzer = false;  iPauseBackUp = -1;
    bHomeinitialCheckPushZ1 = false;  bHomeUnlock = false;
    fMain->BtnOneCycle->Down = false;
    fNote->fShow = false;  MyMessageBox->fShow = false;  fiosetview->fShow = false;  fContact->fShow = false;  fHome->fShow = false;
    g_noticeUp = false;  g_contactOneCycle = 0;  g_interlockBlocks = false;  g_interlockAsked = 0;
    IniConfig.bEnable_SECS_GEM = true;                                           // the golden EventReport lines become observable
    IniConfig.bFinishSuckAfterPause = false;  IniConfig.bG06HomeinitialCheckZ1 = false;  IniConfig.bA17RESETButtonDisable = false;
    IniConfig.bSPILFunction = false;  IniConfig.bEnableAutoCleanFunction = false;  IniConfig.bP28Auto1OnlyBin1 = false;
    IniConfig.bI52_bAQLSortMode = false;  IniConfig.bO01_ResetNeedClearAndCheckHP = false;
    IniConfig.bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed = false;  IniConfig.bG22NoticeTakeoutTray = false;
    IniConfig.bI01TesterFinishThenHome = false;
    IniConfig.bA56EnableAutoTeachFunciton = false;  CosFunction.bManualSteplAutoTeach = false;   // MskAutoTeachKeyStartEnable -> true (MC01's Idle)
    IniConfig.bI40_bStartProductOnLine = false;  IniConfig.bE53LowYieldAutoClean = false;          // CleanOut's optional arms (cCleanOut.cpp:78 / :84)
    CosFunction.bOEEFunction = false;  CosFunction.bUsePassBinOnlyCanSetOneBin = false;  CosFunction.bOLPFunction = false;
    CosFunction.bUseBarcodeAutoAdjustLight = false;
    OFFLINE_ALARM = false;  for (int t = 0; t < 4; ++t) UserDefForm[t].YPitch = 1600;   // AI(W906-ST02-SKC) 20261004 (St02-E, NB2-1 R209): YPitch seeded at the real source of TfMain::Home's AutoCalculateInArmYClosePitch (ainarm9045.cpp:7753-7754, UserDefForm[TrayForm.Loader.iTrayType]) -- a ctest reads no Tray.Data, and YPitch 0 makes W906-PITCH0 (:7764-7772) overwrite ExString right after MES2112; same line, the reverse list's line numbers stay
    ExString = "";
    ResetSimEventReport();
    W906_ShowErrorMessage_Reset();
    W906_ShowMyMessage_Reset();
    g_starts = 0;  g_pauses = 0;  g_startFunc.clear();  g_pauseFunc.clear();  g_startLockDepth = -1;  g_pauseLockDepth = -1;
    MarkLog();
}
// one press: hold the key, one beat, release, clear ScanPannelKey's latch (its release sweep)
static void Press(int key)
{
    MarkLog();
    SimOn(key);
    Beat();
    SimOff(key);
    ScanPannelKey();
}
// a press the tick must NOT read: afterwards ScanPannelKey itself still answers the key (the box's reader would get it)
static bool LeftForTheBox(int key)
{
    SimOn(key);
    Beat();
    const int r = ScanPannelKey();
    SimOff(key);
    ScanPannelKey();
    return r == key;
}
// the Contact page opens: MskContactFormShowEdge (WebMainScanKey.cpp:63-75) sets s_bOneCycleFinish on the page's closed->open
//   edge from golden FormShow's test (cContact.cpp:1192-1205: an IC in Index / InArm / OutArm / Shuttle -> false, else true).
//   One beat closed (the edge's memory), one beat open with an IC on the in-arm or not, then the IC goes away: the flag stays
//   (golden writes it only in FormShow, cContact.cpp:1198 / :1204).  No key is held in either beat.
static void ContactPageOpens(bool icOnBoard)
{
    fContact->fShow = false;
    Beat();
    const int s = InArmSuck.Item[0][0];
    if (icOnBoard) InArmSuck.Item[0][0] = HAS_IC;                                // iMaxRow / iMaxCol = 1 (ctor, golden MyKitSuck.cpp:78-79)
    fContact->fShow = true;
    Beat();
    InArmSuck.Item[0][0] = s;
}
// the two bin checks golden runs before START (:2494 / :2499) and HOME (:2552 / :2557) -- MainCalcCore.cpp's tests
static int g_sBinCount = 0, g_sIsPass0 = 0, g_sCat0 = 0, g_sCat1 = 0, g_sPos0 = 0;
static void SaveBins()    { g_sBinCount = iTestBinCount;  g_sIsPass0 = Prod.iIsPassT6[0];  g_sCat0 = Prod.iT6CatData[0];
                            g_sCat1 = Prod.iT6CatData[1];  g_sPos0 = Prod.iT6PosCate[0]; }
static void RestoreBins() { iTestBinCount = g_sBinCount;  Prod.iIsPassT6[0] = g_sIsPass0;  Prod.iT6CatData[0] = g_sCat0;
                            Prod.iT6CatData[1] = g_sCat1;  Prod.iT6PosCate[0] = g_sPos0; }
static void TwoBinsOnPassTray0()     // CheckAutoOnlySetOneBin (golden :32523-32553): a Pass tray that holds two bins
{
    CosFunction.bUsePassBinOnlyCanSetOneBin = true;
    iTestBinCount = 2;  Prod.iIsPassT6[0] = 1;  Prod.iT6CatData[0] = 0;  Prod.iT6CatData[1] = 0;
}
static void Bin0OnAuto1()            // CheckAuto1OnlyBin1 (golden :32502-32521): bin 0 set to Auto1 (only bin 1 may be)
{
    iTestBinCount = 2;  Prod.iT6PosCate[0] = ePosAuto1;
}

// ---- source pins ------------------------------------------------------------------------------------------------------
static std::string ReadSource(const std::string& root, const char* rel)
{
    std::ifstream f((root + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
static std::string CodeOnly(const std::string& s)     // comments and literals blanked, newlines kept (test_fastclk_jobs.cpp)
{
    std::string o(s);
    enum { CODE, LINE, BLOCK, STR, CHR } st = CODE;
    for (size_t i = 0; i < o.size(); ++i) {
        const char c = s[i];
        const char n = (i + 1 < s.size()) ? s[i + 1] : '\0';
        switch (st) {
        case CODE:
            if (c == '/' && n == '/') { st = LINE; o[i] = ' '; }
            else if (c == '/' && n == '*') { st = BLOCK; o[i] = ' '; o[i + 1] = ' '; ++i; }
            else if (c == '"') { st = STR; }
            else if (c == '\'') { st = CHR; }
            break;
        case LINE:  if (c == '\n') st = CODE; else if (c != '\r') o[i] = ' '; break;
        case BLOCK: if (c == '*' && n == '/') { st = CODE; o[i] = ' '; o[i + 1] = ' '; ++i; } else if (c != '\n' && c != '\r') o[i] = ' '; break;
        case STR: case CHR:
            if (c == '\\' && i + 1 < s.size()) { o[i] = ' '; if (s[i + 1] != '\n') o[i + 1] = ' '; ++i; }
            else if ((st == STR && c == '"') || (st == CHR && c == '\'')) st = CODE;
            else if (c != '\n' && c != '\r') o[i] = ' ';
            break;
        }
    }
    return o;
}
static std::string Uncomment(const std::string& s)    // comments blanked, literals kept
{
    std::string o(s);
    enum { CODE, LINE, BLOCK, STR, CHR } st = CODE;
    for (size_t i = 0; i < o.size(); ++i) {
        const char c = s[i];
        const char n = (i + 1 < s.size()) ? s[i + 1] : '\0';
        switch (st) {
        case CODE:
            if (c == '/' && n == '/') { st = LINE; o[i] = ' '; }
            else if (c == '/' && n == '*') { st = BLOCK; o[i] = ' '; o[i + 1] = ' '; ++i; }
            else if (c == '"') { st = STR; }
            else if (c == '\'') { st = CHR; }
            break;
        case LINE:  if (c == '\n') st = CODE; else if (c != '\r') o[i] = ' '; break;
        case BLOCK: if (c == '*' && n == '/') { st = CODE; o[i] = ' '; o[i + 1] = ' '; ++i; } else if (c != '\n' && c != '\r') o[i] = ' '; break;
        case STR: case CHR:
            if (c == '\\' && i + 1 < s.size()) ++i;
            else if ((st == STR && c == '"') || (st == CHR && c == '\'')) st = CODE;
            break;
        }
    }
    return o;
}
static int CountOf(const std::string& hay, const std::string& needle)
{
    int n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) ++n;
    return n;
}
// the definition that starts with `head` (the next non-blank character is '{') up to its closing "\n}"; "" when absent
static std::string BodyOf(const std::string& hay, const std::string& head)
{
    for (size_t p = hay.find(head); p != std::string::npos; p = hay.find(head, p + head.size())) {
        size_t q = p + head.size();
        while (q < hay.size() && (hay[q] == ' ' || hay[q] == '\t' || hay[q] == '\r' || hay[q] == '\n')) ++q;
        if (q < hay.size() && hay[q] == '{') {
            const size_t e = hay.find("\n}", q);
            return e == std::string::npos ? hay.substr(p) : hay.substr(p, e + 2 - p);
        }
    }
    return "";
}
// every whole line that contains `needle`, one per occurrence
static std::string LinesWith(const std::string& hay, const std::string& needle)
{
    std::string o;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) {
        const size_t b0 = hay.rfind('\n', p);
        const size_t e0 = hay.find('\n', p);
        const size_t b = (b0 == std::string::npos) ? 0 : b0 + 1;
        const size_t e = (e0 == std::string::npos) ? hay.size() : e0;
        o += hay.substr(b, e - b);
        o += '\n';
    }
    return o;
}
static std::string Squeeze(const std::string& s)      // blanks dropped
{
    std::string o;
    for (size_t i = 0; i < s.size(); ++i)
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\r' && s[i] != '\n') o += s[i];
    return o;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
#ifdef SOFT_SIMULTE
    std::printf("ScanKeyGolden (SIM)\n");
#else
    std::printf("ScanKeyGolden (SHIP)\n");
#endif
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("ScanKeyGolden", rt))
        return 2;
    std::printf("[0] fixture\n");
    CHECK(fMain != 0 && fMain->BtnOneCycle != 0 && fMain->cbRunStartMode != 0 && fNote != 0 && fHome != 0 && fContact != 0 &&
          fiosetview != 0 && MyMessageBox != 0 && fSortCT != 0 && fSortCT->pnlLoad != 0);
    if (fMain == 0 || fMain->BtnOneCycle == 0 || fMain->cbRunStartMode == 0 || fNote == 0 || fHome == 0 || fContact == 0 ||
        fiosetview == 0 || MyMessageBox == 0 || fSortCT == 0 || fSortCT->pnlLoad == 0)
        return 1;
    // the sources [14] / [16] pin (argv[1] = tree root, read only)
    std::string ws, sk, df, cm, tcm, fc;
    if (argc > 1) {
        const std::string root = argv[1];
        ws  = Uncomment(ReadSource(root, "tools/wb_serve.cpp"));
        sk  = CodeOnly(ReadSource(root, "WebMainScanKey.cpp"));
        df  = Uncomment(ReadSource(root, "FileRW/DeviceForm_File.cpp"));
        cm  = ReadSource(root, "CMakeLists.txt");
        tcm = ReadSource(root, "tests/CMakeLists.txt");
        fc  = CodeOnly(ReadSource(root, "FastClockWbServe.cpp"));
    }
    const W906_RemoteRunTable savedRun = W906_RemoteRun;
    W906_RemoteRun.Start = &FakeStart;
    W906_RemoteRun.Pause = &FakePause;
    GetTimeInfo();                                                               // today's date globals before the first row: one HANDLER LOG file
    W906_TestEnsureSimMotors();                                                  // [4] / [7]: StopAllMotor / Gali_Command read MOT[].Motor (golden: never NULL after boot)
    SaveBins();
    iControlPanelMode = 0;                                                       // IsSafeLockCheck's IO arm (test_sysinit_boot.cpp)
    IniConfig.bDisibleResetButton = false;
    AllKeysUnknown();                                                            // SnRKCoverOpen unwired -> the safe lock is released
    SimOff(SnRearPadActive);                                                     // front pad
    IdleHomed();
    Beat();   // [W906] S-20 M3: one idle beat with no key so the first-scan guard sees START / HOME OFF (WebMainScanKey.cpp MskFirstScanSample)

    std::printf("[1] guards: nothing runs, the key stays for the box\n");
    {
        InitialOK = false;
        CHECK(LeftForTheBox(SnFKStart) && g_starts == 0);                       // :31997-32001
        IdleHomed();
        IniConfig.bFinishSuckAfterPause = true;  iPauseBackUp = SnFKPause;       // a PAUSE (2) waiting for the sucks (:2542); they are
        bEnableEmployeeIDCheck = true;                                           //   finished now ([5]), so the next beat would pause
        Beat();                                                                  // :32003-32004 (ScanPannelKey alone answers -1 here, :2387
        CHECK(g_pauses == 0 && iPauseBackUp == SnFKPause);                      //   goes on with the pending pause)
        bEnableEmployeeIDCheck = false;
        Beat();
        CHECK(g_pauses == 1 && iPauseBackUp == -1);                              // in-test control: the same beat without the check pauses
        IdleHomed();
        bSECSGEMAlarm = true;                                                    // :32006-32012 S10F3
        CHECK(LeftForTheBox(SnFKStart) && g_starts == 0);
        bSECSGEM_NoteAlarm = true;                                               // ... a note is up after the S10F3: keys scan again
        Press(SnFKStart);
        CHECK(g_starts == 1);
        IdleHomed();
        fNote->fShow = true;                                                     // :2383 a blocking alarm
        CHECK(LeftForTheBox(SnFKPause) && g_pauses == 0);
        IdleHomed();
        g_noticeUp = true;                                                       // :2383 a notice (wb_serve: the alarm slot holds a kNotice)
        CHECK(LeftForTheBox(SnFKPause) && g_pauses == 0);
        IdleHomed();
        MyMessageBox->fShow = true;                                              // :2385
        CHECK(LeftForTheBox(SnFKHome) && SoftStart == false);
        IdleHomed();
        fiosetview->fShow = true;                                                // :2383 the IO page
        CHECK(LeftForTheBox(SnFKStart) && g_starts == 0);
        IdleHomed();
        Beat();                                                                  // no key, no pending pause: nothing
        CHECK(g_starts == 0 && g_pauses == 0 && SoftStart == false);
    }

    std::printf("[2] START -> the start.run seat, once, outside the form lock; its refusals\n");
    {
        IdleHomed();
        Press(SnFKStart);
        CHECK(g_starts == 1 && g_startFunc == "ScanKey_2");                     // :2516 Start("ScanKey_2")
        CHECK(g_startLockDepth == 0 && g_lockDepth == 0);                        // ADAPTED: no FormLock around START (WebMainScanKey.cpp:19)
        Press(SnFKStart);
        CHECK(g_starts == 2);                                                    // one per press, not per beat
        Beat();
        CHECK(g_starts == 2);

        // ADAPTED: the Home dialog.  MC01 runs TfHome::ScanKey first in the tick (WebMainScanKey.cpp:439) while TfHome's Timer1 is
        //   on, so :2514 is reached only with Timer1 off -- golden's state after the first Home FormClose (uhome.cpp:661 = :4871)
        const bool sT1 = W906_HomeTimer1Enabled;
        IdleHomed();
        W906_HomeTimer1Enabled = false;
        fHome->fShow = true;                                                     // :2514
        Press(SnFKStart);
        CHECK(g_starts == 0);
        IdleHomed();
        W906_HomeTimer1Enabled = true;                                           // Timer1 on (boot, uhome.cpp:649): TfHome::ScanKey takes the
        fHome->fShow = true;  fHome->fAbort = false;                             //   START first and acts on PAUSE only (golden uhome.cpp:4878, WebMainScanKey.cpp:428)
        Press(SnFKStart);
        CHECK(g_starts == 0 && fHome->fShow == true && fHome->fAbort == false && W906_HomeTimer1Enabled == true);
        W906_HomeTimer1Enabled = sT1;

        // ADAPTED: the Contact page (:2392-2396) -- s_bOneCycleFinish comes from the page's open edge (ContactPageOpens)
        IdleHomed();
        CHECK(!(IndexHasIC() || InArmSuck.HasIC() || OutArmSuck.HasIC() || ShuttleHasIC()));   // fixture: no IC anywhere in this process
        IdleHomed();
        ContactPageOpens(true);                                                  // opened with an IC on the in-arm (golden :1192-1198)
        Press(SnFKStart);
        CHECK(g_starts == 0 && W906_ShowErrorMessage_LastCode == "MES1645" && W906_ShowErrorMessage_LastKCode == 0);
        IdleHomed();
        ContactPageOpens(false);                                                 // opened with nothing on board (golden :1204)
        Press(SnFKStart);
        CHECK(g_starts == 1);

        // the AutoClean arm checks (:2432-2463)
        const int sFunc = TestIF_File.iAutoClean_Function, sTray = TestIF_File.iAutoClean_Tray, sMode = TestIF_File.iShuttleMode,
                  sSel = TestIF_File.iShuttle_Sel;
        const double sPitch = TestIF_File.dSiteYPitch;
        const bool sD58 = IniConfig.bD58UseArm1PickPlaceArm2Test, sE43 = IniConfig.bE43AutoCleanUseHotplate, s8P = bUse8Picker,
                   sA1P = TestIF_File.bArm1PickPlaceArm2Test;
        IdleHomed();
        IniConfig.bEnableAutoCleanFunction = true;                               // a clean kit, arm 2 only, pitch > 6350 (:2434-2446)
        TestIF_File.iAutoClean_Function = 1;  TestIF_File.iAutoClean_Tray = eCKPos_CleanKit;
        TestIF_File.iShuttleMode = 1;  TestIF_File.iShuttle_Sel = 1;  TestIF_File.dSiteYPitch = 7000;
        IniConfig.bE43AutoCleanUseHotplate = false;  bUse8Picker = false;
        Press(SnFKStart);
        CHECK(g_starts == 0 && W906_ShowErrorMessage_LastCode == "WAR16102" && W906_ShowErrorMessage_LastKCode == K_RETRY);
        IdleHomed();
        IniConfig.bEnableAutoCleanFunction = true;
        bUse8Picker = true;                                                      // :2438 the 9046LS 8-picker: no WAR16102
        Press(SnFKStart);
        CHECK(g_starts == 1 && W906_ShowErrorMessage_Count == 0);
        bUse8Picker = false;
        IdleHomed();
        IniConfig.bEnableAutoCleanFunction = true;                               // not the clean kit, arm 1 only (:2448-2461)
        TestIF_File.iAutoClean_Tray = eCKPos_CleanKit + 1;  TestIF_File.iShuttle_Sel = 0;  IniConfig.bD58UseArm1PickPlaceArm2Test = false;
        Press(SnFKStart);
        CHECK(g_starts == 0 && W906_ShowErrorMessage_LastCode == "WAR16103" && W906_ShowErrorMessage_LastKCode == K_RETRY);
        IdleHomed();
        IniConfig.bEnableAutoCleanFunction = true;
        IniConfig.bD58UseArm1PickPlaceArm2Test = true;  TestIF_File.bArm1PickPlaceArm2Test = true;   // [D58] arm 1 picks, arm 2 tests (:2452)
        Press(SnFKStart);
        CHECK(g_starts == 0 && W906_ShowErrorMessage_LastCode == "WAR16105" && W906_ShowErrorMessage_LastKCode == K_RETRY);
        TestIF_File.iAutoClean_Function = sFunc;  TestIF_File.iAutoClean_Tray = sTray;  TestIF_File.iShuttleMode = sMode;
        TestIF_File.iShuttle_Sel = sSel;  TestIF_File.dSiteYPitch = sPitch;  IniConfig.bD58UseArm1PickPlaceArm2Test = sD58;
        IniConfig.bE43AutoCleanUseHotplate = sE43;  bUse8Picker = s8P;  TestIF_File.bArm1PickPlaceArm2Test = sA1P;

        // the SPIL offline check (:2465-2492)
        const int sTester = LastSet.iTester, sLevel = AccessLevel;
        const bool sTeach = LastSet.bNeedSetupTeach;
        IdleHomed();
        OFFLINE_ALARM = true;  IniConfig.bSPILFunction = true;  LastSet.iTester = OFF_LINE;  LastSet.bNeedSetupTeach = false;
        AccessLevel = iDefSupervisorLevel - 1;                                   // an operator: refused (:2469-2473)
        Press(SnFKStart);
        CHECK(g_starts == 0 && W906_ShowMyMessage_LastS1 == "Off line Call H/W Check!");
        IdleHomed();
        OFFLINE_ALARM = true;  IniConfig.bSPILFunction = true;  LastSet.iTester = OFF_LINE;
        AccessLevel = iDefSupervisorLevel;                                       // ADAPTED: a supervisor -- golden's re-login (:2476-2489) cannot
        Press(SnFKStart);                                                        //   open from a panel key: MC01 fails closed (WebMainScanKey.cpp:254-269)
        CHECK(g_starts == 0 && W906_ShowMyMessage_Count == 0 && Has(ExString, "panel START refused"));
        IdleHomed();
        OFFLINE_ALARM = true;  IniConfig.bSPILFunction = true;  LastSet.iTester = OFF_LINE;
        AccessLevel = iDefSupervisorLevel;  LastSet.bNeedSetupTeach = true;      // control: the setup-teach flow skips the re-login (:2475)
        Press(SnFKStart);
        CHECK(g_starts == 1 && W906_ShowMyMessage_Count == 0);
        LastSet.iTester = sTester;  AccessLevel = sLevel;  LastSet.bNeedSetupTeach = sTeach;

        // the two bin checks (:2494-2502)
        IdleHomed();
        TwoBinsOnPassTray0();
        Press(SnFKStart);
        CHECK(g_starts == 0 && Has(W906_ShowMyMessage_LastS1, "Pass Bin Only Can Set One Bin."));   // :2494-2497 (:32544)
        RestoreBins();
        IdleHomed();
        IniConfig.bP28Auto1OnlyBin1 = true;  Bin0OnAuto1();
        Press(SnFKStart);
        CHECK(g_starts == 0 && Has(W906_ShowMyMessage_LastS1, "Only Bin 1 is allowed"));             // :2499-2502 (:32516)
        IdleHomed();
        Bin0OnAuto1();                                                           // the same bins without [P28]: START goes on
        Press(SnFKStart);
        CHECK(g_starts == 1 && W906_ShowMyMessage_Count == 0);
        RestoreBins();

        // ADAPTED: [A56] auto alignment (:2504 fAutoTeach->IsKeyStartEnable()) -- S-20 kept golden's GATE (always true); MC01 has no
        //   TfAutoTeach and fails closed while the manual-step auto teach is on (WebMainScanKey.cpp:105-114 MskAutoTeachKeyStartEnable)
        IdleHomed();
        IniConfig.bA56EnableAutoTeachFunciton = true;  CosFunction.bManualSteplAutoTeach = true;
        Press(SnFKStart);
        CHECK(g_starts == 0 && W906_ShowMyMessage_Count == 0 && W906_ShowErrorMessage_Count == 0);
        IdleHomed();
        IniConfig.bA56EnableAutoTeachFunciton = true;                            // control: no manual-step teach -> golden IsRun() false
        Press(SnFKStart);
        CHECK(g_starts == 1);

        // the AAL flags (:2399-2429), then START goes on
        const int sShtRow = InArmSuck.iShtRow, sMode2 = TestIF_File.iShuttleMode, sSel2 = TestIF_File.iShuttle_Sel;
        const bool sAalTif = TestIF_File.bUseBarcodeAutoAdjustLight, sBarcode = TestIF_File.bEnableBarCode, sAalOn = bStartAutoAdjustLight;
        const AnsiString sRunMode = fMain->cbRunStartMode->Text;
        bool sNeed[4];
        for (int i = 0; i < 4; ++i) sNeed[i] = bBarcodeNeedAutoAdjust[i];
        auto aal = [&](int shtRow, int sel, const char* runMode) {
            IdleHomed();
            CosFunction.bUseBarcodeAutoAdjustLight = true;  TestIF_File.bUseBarcodeAutoAdjustLight = true;  TestIF_File.bEnableBarCode = true;
            fMain->cbRunStartMode->Text = runMode;
            bStartAutoAdjustLight = false;
            for (int i = 0; i < 4; ++i) bBarcodeNeedAutoAdjust[i] = false;
            InArmSuck.iShtRow = shtRow;  TestIF_File.iShuttleMode = 1;  TestIF_File.iShuttle_Sel = sel;
            Press(SnFKStart);
        };
        aal(1, 0, "Initial Start");                                              // one shuttle row (:2410-2414), front arm only (:2418-2422)
        CHECK(g_starts == 1 && bStartAutoAdjustLight == true && bBarcodeNeedAutoAdjust[0] == true && bBarcodeNeedAutoAdjust[1] == false &&
              bBarcodeNeedAutoAdjust[2] == false && bBarcodeNeedAutoAdjust[3] == false);
        aal(2, 1, "Initial Start");                                              // two rows, rear arm only (:2423-2427)
        CHECK(g_starts == 1 && bStartAutoAdjustLight == true && bBarcodeNeedAutoAdjust[0] == false && bBarcodeNeedAutoAdjust[1] == false &&
              bBarcodeNeedAutoAdjust[2] == true && bBarcodeNeedAutoAdjust[3] == true);
        aal(2, 1, "Continue");                                                   // :2402 not an "Initial" run mode: no AAL
        CHECK(g_starts == 1 && bStartAutoAdjustLight == false && bBarcodeNeedAutoAdjust[2] == false);
        InArmSuck.iShtRow = sShtRow;  TestIF_File.iShuttleMode = sMode2;  TestIF_File.iShuttle_Sel = sSel2;
        TestIF_File.bUseBarcodeAutoAdjustLight = sAalTif;  TestIF_File.bEnableBarCode = sBarcode;  bStartAutoAdjustLight = sAalOn;
        fMain->cbRunStartMode->Text = sRunMode;
        for (int i = 0; i < 4; ++i) bBarcodeNeedAutoAdjust[i] = sNeed[i];

        IdleHomed();
        W906_RemoteRun.Start = 0;                                                // the seat not installed (other binaries)
        Press(SnFKStart);
        CHECK(g_starts == 0);
        W906_RemoteRun.Start = &FakeStart;
    }

    std::printf("[3] re-entry: a box inside START\n");
    {
        IdleHomed();
        g_boxInStart = true;  g_nestedKey = -1;
        Press(SnFKStart);
        CHECK(g_starts == 1 && g_pauses == 0 && g_nestedKey == SnFKPause);      // the nested beat returned at bRunTimer1 (:32018-32021)
        Press(SnFKPause);
        CHECK(g_pauses == 1);                                                    // bRunTimer1 cleared after the outer beat (:32024)
    }

    std::printf("[4] PAUSE -> BtnPauseClick -> the Pause seat; the Home dialog's own reader first\n");
    {
        IdleHomed();
        const int c0 = fMain->W906_BtnPauseClickCallCount;
        Press(SnFKPause);
        CHECK(Row("MES2111", "PAUSE pressed", "ScanKey_1") && fMain->W906_BtnPauseClickCallCount == c0 + 1 && g_pauses == 1 &&
              g_pauseFunc == "BtnPauseClick");                                   // :2519-2522
        CHECK(g_pauseLockDepth == 0 && g_lockDepth == 0);                        // ADAPTED: no FormLock around PAUSE (WebMainScanKey.cpp:19)
        SystemStart = true;                                                      // golden Pause does not look at SystemStart
        Press(SnFKPause);
        CHECK(g_pauses == 2);

        // ADAPTED (MC01 order, WebMainScanKey.cpp:438-439): the Home dialog up with its Timer1 on -> golden TfHome::Timer1Timer ->
        //   TfHome::ScanKey (uhome.cpp:4874-4889) reads the pad before TfMain::ScanKey: PAUSE aborts the home and closes the dialog,
        //   and the main-screen ScanKey of the same beat finds no key (ScanPannelKey answered it once).
        const bool sT1 = W906_HomeTimer1Enabled;
        void (*const sHook)(const char*) = W906_Stop1203AllHook;
        const bool sPower = bMotorPowerState;
        IdleHomed();
        W906_HomeTimer1Enabled = true;  fHome->fShow = true;  fHome->fAbort = false;
        W906_Stop1203AllHook = &FakeStop1203;  g_servoOffs = 0;
        const int c1 = fMain->W906_BtnPauseClickCallCount;
        Press(SnFKPause);
        CHECK(fHome->fAbort == true && fHome->fShow == false && W906_HomeTimer1Enabled == false && g_servoOffs == 1);   // golden uhome.cpp:4880-4881 (+ :4871)
        CHECK(g_pauses == 0 && fMain->W906_BtnPauseClickCallCount == c1 && NewLogText().find("PAUSE pressed") == std::string::npos);
        W906_HomeTimer1Enabled = sT1;  W906_Stop1203AllHook = sHook;  bMotorPowerState = sPower;  fHome->fAbort = false;
        IdleHomed();                                                             // GaliMotorServoOff cleared SystemStart / fAllMotorHome
    }

    std::printf("[5] PAUSE (2): bFinishSuckAfterPause\n");
    {
        IdleHomed();
        IniConfig.bFinishSuckAfterPause = true;
        const bool fin = InArmSuck.IsPickSuckFinish() && InArmSuck.IsPickDestroyFinish() && OutArmSuck.IsPickSuckFinish() &&
                         OutArmSuck.IsPickDestroyFinish() && FTestSuck.IsShtSuckFinish() && FTestSuck.IsShtDestroyFinish() &&
                         BTestSuck.IsShtSuckFinish() && BTestSuck.IsShtDestroyFinish() && CatchTraySuck.IsShtSuckFinish() &&
                         CatchTraySuck.IsShtDestroyFinish();
        CHECK(fin);                                                              // the test process: every grid idle
        Press(SnFKPause);
        CHECK(g_pauses == 1 && iPauseBackUp == -1 && Row("MES2111", "PAUSE pressed", "ScanKey_2"));   // :2530-2539
        const int sRow = InArmSuck.iPickRow, sCol = InArmSuck.iPickCol;
        const bool sOff = InArmSuck.Suck[0][0].OffEnable || InArmSuck.Suck[0][0].OnEnable;
        CHECK(sOff == false);                                                    // no IO wired here: OnDestroy / Normal only flip the flags
        if (sOff == false) {
            InArmSuck.iPickRow = 1;  InArmSuck.iPickCol = 1;
            InArmSuck.Suck[0][0].OnDestroy();                                    // a destroy still running
            Press(SnFKPause);
            CHECK(g_pauses == 1 && iPauseBackUp == SnFKPause);                   // :2542 iPauseBackUp=Key
            Beat();                                                              // next beat, no key, still not finished
            CHECK(g_pauses == 1 && iPauseBackUp == -1);                          // :2542 with Key == -1: dropped (golden as written)
            Press(SnFKPause);
            CHECK(iPauseBackUp == SnFKPause);
            InArmSuck.Suck[0][0].Normal();                                       // the destroy finished within one beat
            ExString = "";  MarkLog();                                           // (NewRecordProcess skips a repeat of the last row)
            Beat();
            CHECK(g_pauses == 2 && iPauseBackUp == -1 && Row("MES2111", "PAUSE pressed", "ScanKey_2"));   // the pending pause completes (:2536-2538)
            InArmSuck.iPickRow = sRow;  InArmSuck.iPickCol = sCol;
        }
    }

    std::printf("[6] HOME -> Home(\"ScanKey\")\n");
    {
        IdleHomed();
        fAllMotorHome = false;                                                   // a machine to home (Home skips CheckOutArmDestory then)
        Press(SnFKHome);
        CHECK(SoftStart == true && iHome == 1 && Row("MES2112", "HOME pressed", "ScanKey"));   // :2563 Home("ScanKey") (forms/fMain.cpp, golden :6975-7101)
        IdleHomed();
        fAllMotorHome = false;
        SystemStart = true;                                                      // BtnHome->Enabled false (ProcessKeyFlush :4111)
        Press(SnFKHome);
        CHECK(SoftStart == false && iHome == 0 && NoRow());
        IdleHomed();
        fAllMotorHome = false;
        IniConfig.bG06HomeinitialCheckZ1 = true;                                 // :2547-2550 then Home's own check (golden Home :7036-7040)
        Press(SnFKHome);
        CHECK(bHomeinitialCheckPushZ1 == true && SoftStart == false && W906_ShowErrorMessage_LastCode == "MES1699");
        IdleHomed();
        fAllMotorHome = false;
        TwoBinsOnPassTray0();
        Press(SnFKHome);
        CHECK(SoftStart == false && NoRow() && Has(W906_ShowMyMessage_LastS1, "Pass Bin Only Can Set One Bin."));   // :2552-2555
        RestoreBins();
        IdleHomed();
        fAllMotorHome = false;
        IniConfig.bP28Auto1OnlyBin1 = true;  Bin0OnAuto1();
        Press(SnFKHome);
        CHECK(SoftStart == false && NoRow() && Has(W906_ShowMyMessage_LastS1, "Only Bin 1 is allowed"));             // :2557-2560
        RestoreBins();
        IdleHomed();
    }

    std::printf("[7] RESET -> MES2113 + the kept BtnResetClick no-op, then golden's tail (:2579-2586)\n");
    {
        void (*const sHook)(const char*) = W906_Stop1203AllHook;
        const bool sPower = bMotorPowerState;
        const int cyl[] = { C_Empty_Fix, C_Color_Fix,                                                     // :2583-2584
                            C_TrayY_Fixer, C_LoaderEdgePush, C_LoaderUpPress,                             // :2586 AutoTrayCylinderFree
                            C_AutoSide_Fixer[eAuto1], C_AutoEdgePush[eAuto1], C_AutoUpPress[eAuto1] };  //   (csystem.cpp:23603-23615)
        const int nCyl = (int)(sizeof(cyl) / sizeof(cyl[0]));
        bool sStatus[sizeof(cyl) / sizeof(cyl[0])];
        for (int i = 0; i < nCyl; ++i) sStatus[i] = Cylinder[cyl[i]].Status;
        bool fixOff = false, freeOff = false;
        // one RESET press with every one of those cylinders ON, the motor power ON and the 1203 hook counting
        auto reset = [&](bool homed, int home) {
            IdleHomed();
            fAllMotorHome = homed;  iHome = home;
            W906_Stop1203AllHook = &FakeStop1203;  g_servoOffs = 0;  g_stopLockDepth = -1;  bMotorPowerState = true;
            for (int i = 0; i < nCyl; ++i) Cylinder[cyl[i]].Status = true;
            Press(SnFKReset);
            fixOff = !Cylinder[C_Empty_Fix].Status && !Cylinder[C_Color_Fix].Status;
            freeOff = true;
            for (int i = 2; i < nCyl; ++i) freeOff = freeOff && !Cylinder[cyl[i]].Status;
        };
        reset(false, 0);                                                         // not homed
        CHECK(LogRow("MES2113", "RESET pressed", "ScanKey"));                    // :2576 (GaliMotorServoOff then writes its own row)
        CHECK(g_servoOffs == 1 && bMotorPowerState == false);                    // :2579-2582 GaliMotorServoOff (uhome.cpp:4967-4997)
        CHECK(fixOff && freeOff);                                                // :2583-2586
        CHECK(g_stopLockDepth == 0 && g_lockDepth == 0);                         // ADAPTED: no FormLock around RESET (WebMainScanKey.cpp:19)
        reset(true, 1);                                                          // homed but iHome==1 (a home running)
        CHECK(g_servoOffs == 1 && bMotorPowerState == false && fixOff && freeOff);
        reset(true, 0);                                                          // homed, no home running: no servo off, the rest runs
        CHECK(Row("MES2113", "RESET pressed", "ScanKey") && g_servoOffs == 0 && bMotorPowerState == true && fixOff && freeOff);
        for (int i = 0; i < nCyl; ++i) Cylinder[cyl[i]].Status = sStatus[i];
        W906_Stop1203AllHook = sHook;  bMotorPowerState = sPower;
        IdleHomed();
        SystemStart = true;
        Press(SnFKReset);
        CHECK(NoRow());                                                          // :2570
        IdleHomed();
        IniConfig.bA17RESETButtonDisable = true;
        Press(SnFKReset);
        CHECK(NoRow());                                                          // :2573 [A17]
        IdleHomed();
        IniConfig.bSPILFunction = true;
        Press(SnFKReset);
        CHECK(NoRow());                                                          // :2573 [SPIL]
        IdleHomed();
        bRunAutoClean = true;
        Press(SnFKReset);
        CHECK(NoRow());                                                          // :2567
    }

    std::printf("[8] CLEAN OUT -> BtnCleanOutClick under the form lock\n");
    {
        IdleHomed();
        const int l0 = g_lockCalls;
        Press(SnFKCleanOut);
        CHECK(iCleanOut == 1 && g_SimEventReportCount == 2 && g_SimLastEventReportCeid == SECS_EVENT.DoCleanOut &&
              Row("MES2114", "CLEAN OUT pressed", "ScanKey"));                   // :2591-2596
        CHECK(g_lockCalls == l0 + 1 && g_lockDepth == 0);                        // ADAPTED: MskFormLock taken once, released (WebMainScanKey.cpp:359)
        const int l1 = g_lockCalls;
        IdleHomed();
        fAllMotorHome = false;                                                   // BtnCleanOut->Enabled false (ProcessKeyFlush :4117-4119)
        Press(SnFKCleanOut);
        CHECK(iCleanOut == 0 && g_SimEventReportCount == 0 && NoRow());
        IdleHomed();
        SystemStart = true;                                                      // ... (:4107-4109)
        Press(SnFKCleanOut);
        CHECK(iCleanOut == 0 && g_SimEventReportCount == 0 && NoRow());
        IdleHomed();
        iOneCycle = 1;                                                           // ... (:4117-4119)
        Press(SnFKCleanOut);
        CHECK(iCleanOut == 0 && g_SimEventReportCount == 0 && NoRow());
        IdleHomed();
        bRunAutoClean = true;                                                    // :2590
        Press(SnFKCleanOut);
        CHECK(iCleanOut == 0 && g_SimEventReportCount == 0 && NoRow());
        CHECK(g_lockCalls == l1 && g_lockDepth == 0);                            // ADAPTED: no refusal took the lock
    }

    std::printf("[9] ONE CYCLE -> BtnOneCycleClick / the Contact page's OneCycleProcess\n");
    {
        IdleHomed();
        bCleanHotplate_ART = 0;
        const int l0 = g_lockCalls;
        Press(SnFKOneCycle);
        CHECK(fMain->BtnOneCycle->Down == true && bManualOneCycle == true && bCleanHotplate_ART == 1);
        CHECK(g_SimEventReportCount == 2 && g_SimLastEventReportCeid == SECS_EVENT.DoOneCycle && g_contactOneCycle == 0 &&
              Row("MES2115", "ONE CYCLE pressed", "ScanKey"));                   // :2604 (BtnOneCycleClick then writes its own)
        CHECK(g_lockCalls == l0 + 1 && g_lockDepth == 0);                        // ADAPTED: MskFormLock taken once, released (WebMainScanKey.cpp:371)
        IdleHomed();
        bCleanHotplate_ART = 0;
        fAllMotorHome = false;                                                   // BtnOneCycleClick refuses at its first test (golden :4332-4380)
        Press(SnFKOneCycle);                                                     //   -> only ScanKey's own lines show
        CHECK(bCleanHotplate_ART == 1 && bManualOneCycle == true && fMain->BtnOneCycle->Down == false && g_SimEventReportCount == 1 &&
              g_SimLastEventReportCeid == SECS_EVENT.DoOneCycle && Row("MES2115", "ONE CYCLE pressed", "ScanKey"));   // :2600 / :2604-2607
        IdleHomed();
        fContact->fShow = true;                                                  // :2610-2614
        const int l2 = g_lockCalls;
        Press(SnFKOneCycle);
        CHECK(g_contactOneCycle == 1 && fMain->BtnOneCycle->Down == false && g_SimEventReportCount == 0 &&
              Row("MES2115", "ONE CYCLE pressed", "Contact Test"));
        CHECK(g_lockCalls == l2 && g_lockDepth == 0);                            // ADAPTED: ScanKey takes no lock here -- the callee does ([14])
        bCleanHotplate_ART = 0;
    }

    std::printf("[10] TRAY FEED -> InitialTrayFeedTask(\"ScanKey\") under the form lock\n");
    {
        IdleHomed();
        const int l0 = g_lockCalls;
        Press(SnFKTrayFeed);
        CHECK(iTrayFeed == 1 && iTrayFeedTask == 1 && Row("MES2118", "TRAY FEED pressed", "ScanKey"));   // :2624 (cCleanOut.cpp writes Func)
        CHECK(g_lockCalls == l0 + 1 && g_lockDepth == 0);                        // ADAPTED: MskFormLock taken once, released (WebMainScanKey.cpp:386)
        const int l1 = g_lockCalls;
        IdleHomed();
        bUnloading = true;                                                       // :2621
        Press(SnFKTrayFeed);
        CHECK(iTrayFeed == 0 && iTrayFeedTask == 0 && NoRow());
        IdleHomed();
        bRunAutoClean = true;                                                    // :2618
        Press(SnFKTrayFeed);
        CHECK(iTrayFeed == 0 && iTrayFeedTask == 0 && NoRow());
        CHECK(g_lockCalls == l1 && g_lockDepth == 0);                            // ADAPTED: no refusal took the lock
        bUnloading = false;  bRunAutoClean = false;
    }

    std::printf("[11] ALARM RESET\n");
    {
        IdleHomed();
        const bool sSwF = SW[SwFKAlarmReset].OutValue, sSwR = SW[SwRKAlarmReset].OutValue;
        bAlarmBuzzer = true;  bLampAlarmReset = true;  SECS_GEM_PPMUSIC_CONTROL_flag = true;  SECS_GEM_PPSIGNALTOWER_CONTROL_flag = true;
        SW[SwFKAlarmReset].OutValue = true;  SW[SwRKAlarmReset].OutValue = true;  // both reset-key lamps lit
        Press(SnFKAlarmReset);
        CHECK(bAlarmBuzzer == false && bLampAlarmReset == false && SECS_GEM_PPMUSIC_CONTROL_flag == false &&
              SECS_GEM_PPSIGNALTOWER_CONTROL_flag == false && Row("MES2116", "ALARM RESET pressed", "ScanKey") &&
              g_SimEventReportCount == 1 && g_SimLastEventReportCeid == SECS_EVENT.DoAlarmReset);   // :2628-2636
        CHECK(SW[SwFKAlarmReset].OutValue == false && SW[SwRKAlarmReset].OutValue == false);         // :2633-2634
        IdleHomed();
        bLampAlarmReset = true;                                                  // no buzzer: the branch is not taken (:2626)
        SW[SwFKAlarmReset].OutValue = true;  SW[SwRKAlarmReset].OutValue = true;
        Press(SnFKAlarmReset);
        CHECK(bLampAlarmReset == true && g_SimEventReportCount == 0 && NoRow() &&
              SW[SwFKAlarmReset].OutValue == true && SW[SwRKAlarmReset].OutValue == true);
        bLampAlarmReset = false;
        SW[SwFKAlarmReset].OutValue = sSwF;  SW[SwRKAlarmReset].OutValue = sSwR;
    }

    std::printf("[12] POWER OFF / ON rows, front and rear pad\n");
    {
        IdleHomed();
        Press(SnFKPowerOff);
        CHECK(Row("MES2117", "POWER OFF pressed", "ScanKey"));                    // :2638-2641
        Press(SnFKPowerOn);
        CHECK(Row("MES2128", "POWER ON pressed", "ScanKey"));                     // :2642-2645
        IdleHomed();
        SimOn(SnRearPadActive);                                                  // the rear pad: its POWER keys answer the REAR ids
        Press(SnRKPowerOff);                                                     //   (ckernel.cpp:3465-3473)
        CHECK(Row("MES2117", "POWER OFF pressed", "ScanKey"));                    // :2638 Key==SnRKPowerOff
        Press(SnRKPowerOn);
        CHECK(Row("MES2128", "POWER ON pressed", "ScanKey"));                     // :2642 Key==SnRKPowerOn
        SimOff(SnRearPadActive);
    }

    std::printf("[13] TRAY END [AutoSKIP + bASkStart] -> InitNewTray + InitTrayEndFunction\n");
    {
        IdleHomed();
        const int sSkip = ArmSpeed_File[InArm].bAutoSKIP;
        const bool sAsk = bASkStart, sHP = CosFunction.bShowHPICCount;
        TMyTray& tray = MOT[MMTrayY].Tray;
        const int sX = tray.XItem, sY = tray.YItem, sD = tray.Data[0][0];
        const AnsiString sCap = fSortCT->pnlLoad->Caption;
        iTrayFeed = 1;
        ArmSpeed_File[InArm].bAutoSKIP = 0;  bASkStart = true;
        Press(SnFKTrayEnd);
        CHECK(iTrayFeed == 1);                                                   // :2647 AutoSKIP off: nothing
        ArmSpeed_File[InArm].bAutoSKIP = 1;  bASkStart = false;
        Press(SnFKTrayEnd);
        CHECK(iTrayFeed == 1);                                                   // :2647 bASkStart false: nothing
        bASkStart = true;
        tray.XItem = 1;  tray.YItem = 1;  tray.Data[0][0] = HAS_IC;              // one IC on the load tray
        CosFunction.bShowHPICCount = true;  fSortCT->pnlLoad->Caption = "7";
        Press(SnFKTrayEnd);
        CHECK(iTrayFeed == 0);                                                   // :2650 InitTrayEndFunction (csystem.cpp:421)
        CHECK(tray.Data[0][0] == NULL_IC && fSortCT->pnlLoad->Caption == "0");  // :2649 InitNewTray(NULL_IC), :2651-2654 HowManyIC()
        iTrayFeed = 1;  tray.Data[0][0] = HAS_IC;
        SimOn(SnRearPadActive);                                                  // the rear TRAY END answers SnFKTrayEnd (ckernel.cpp:3562-3569)
        Press(SnRKTrayEnd);
        CHECK(iTrayFeed == 0 && tray.Data[0][0] == NULL_IC);
        SimOff(SnRearPadActive);
        tray.XItem = sX;  tray.YItem = sY;  tray.Data[0][0] = sD;
        fSortCT->pnlLoad->Caption = sCap;  CosFunction.bShowHPICCount = sHP;
        ArmSpeed_File[InArm].bAutoSKIP = sSkip;  bASkStart = sAsk;
    }

    // [14] REWRITTEN for MC01's layout: WebMainScanKey.cpp is ticked from wb_serve's main loop and the drag keepalive (not a
    //   fast-clock job), the notice is the tick's argument (no W906_ScanKeyNoticeUp), the Contact page's flag lives in
    //   WebMainScanKey.cpp (no FileRW_ContactOneCycle* accessors), the form lock is the blocking RAII MskFormLock.
    std::printf("[14] source pins (WebMainScanKey.cpp, the wb_serve hooks, the census line)\n");
    if (argc > 1) {
        const std::string tick = "W906_MainScanKeyTick(g_alarmSlot.kind == w906dlg::AlarmSlot::kNotice);";
        CHECK(!ws.empty() && !sk.empty() && !df.empty() && !cm.empty() && !tcm.empty() && !fc.empty());
        CHECK(CountOf(ws, tick) == 2);                                           // two ticks; noticeUp = a KeyCode==0 notice in the alarm slot
        CHECK(CountOf(ws, "W906_MainCtlButtonTick(); }  { extern void W906_MainScanKeyTick(bool); " + tick) == 2);   // each right after the screen buttons' tick
        CHECK(CountOf(BodyOf(ws, "void W906_NativeKeepaliveMain()"), tick) == 1 &&                                   // the drag keepalive
              CountOf(LinesWith(ws, tick), "{ extern void W906_FastClockPassBegin(); W906_FastClockPassBegin(); }") == 1);   // + the main-loop pass
        CHECK(CountOf(sk, "W906_RemoteRunStart(") == 1 && CountOf(sk, "->Start(") == 0 && CountOf(sk, "StartFromWeb(") == 0);
        CHECK(CountOf(sk, "iPauseBackUp=Key;") == 1);                                                   // golden :2542 as written
        CHECK(CountOf(sk, "struct MskFormLock { MskFormLock() { ht9045::formjson::FormLock(); } ~MskFormLock() { ht9045::formjson::FormUnlock(); } };") == 1 &&
              CountOf(sk, "MskFormLock lock;") == 3 && CountOf(sk, "FormTryLock") == 0);              // CLEAN OUT / ONE CYCLE / TRAY FEED only, blocking
        CHECK(CountOf(df, "void W906_Contact_OneCycleProcess()") == 1 &&
              CountOf(Squeeze(BodyOf(df, "void W906_Contact_OneCycleProcess()")),
                      "{ht9045::formjson::FormLock();DF_OneCycleProcess();ht9045::formjson::FormUnlock();}") == 1);   // the Contact ONE CYCLE takes the lock itself
        CHECK(CountOf(cm, "WebMainCtlButtons.cpp  WebMainScanKey.cpp") == 1);                           // in wb_serve's sources
        CHECK(CountOf(fc, "ScanKey") == 0);                                                             // no second reader on the fast clock
        CHECK(CountOf(tcm, "/tools/start_sites_census.py\" --check 36 34 2)") == 1);                    // START_SitesCensus unchanged: 36 / 34 / 2
    } else
        std::printf("  (skipped: no tree root given)\n");

    // [15] the [W906] first-scan guard (WebMainScanKey.cpp:115-149 MskFirstScanHeld).  W906_ScanKeyFirstScanRearm_St02() = the boot
    //   state: no START / HOME input seen OFF yet.
    std::printf("[15] [W906] first-scan guard: START / HOME ON since boot are ignored; RESET keeps golden's fire-once\n");
    {
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (a) front START held since boot
        SimOn(SnFKStart);
        Beat();
        CHECK(g_starts == 0);                                                    // ignored (golden 0618 would Start once here)
        Beat();
        CHECK(g_starts == 0);                                                    // still held: golden's bK latch, no second answer
        SimOff(SnFKStart);
        Beat();                                                                  // let go: seen OFF + ScanPannelKey's release sweep
        Press(SnFKStart);
        CHECK(g_starts == 1 && g_startFunc == "ScanKey_2");                     // the next real press is golden's :2516
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (b) front HOME held since boot
        fAllMotorHome = false;
        SimOn(SnFKHome);
        Beat();
        CHECK(SoftStart == false && iHome == 0 && NoRow());                      // Home("ScanKey") not reached, no MES2112
        SimOff(SnFKHome);
        Beat();
        Press(SnFKHome);
        CHECK(SoftStart == true && iHome == 1 && Row("MES2112", "HOME pressed", "ScanKey"));   // = [6]
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (c) RESET held since boot: NOT guarded
        SimOn(SnFKReset);
        Beat();
        CHECK(LogRow("MES2113", "RESET pressed", "ScanKey"));                    // golden fires a held key once (:2576)
        SimOff(SnFKReset);
        ScanPannelKey();
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (d) rear pad: rear START held since boot
        SimOn(SnRearPadActive);
        SimOn(SnRKStart);
        Beat();
        CHECK(g_starts == 0);                                                    // golden's pad choice: Sen[SnRKStart] (ckernel :2381-2383)
        SimOff(SnRKStart);
        Beat();
        SimOn(SnRKStart);  Beat();  SimOff(SnRKStart);  ScanPannelKey();
        CHECK(g_starts == 1);
        SimOff(SnRearPadActive);
        IdleHomed();
        Beat();                                                                  // every input seen OFF again for what follows
    }

    // [16] a NON-modal MyMessageBox (ShowUnloaderTrayMessage) up: ScanKey returns (golden :2385) and hands the beat to the box's own
    //   panel-key reader (NB2 R188 M2; golden mymessbox.cpp Timer1Timer :540-663) -- WebMainScanKey.cpp:161-165.
    std::printf("[16] a box is up: ScanKey hands the beat to the non-modal box's panel-key reader\n");
    {
        IdleHomed();
        MyMessageBox->fShow = true;
        const int t0 = g_modelessTicks;
        CHECK(LeftForTheBox(SnFKPause) && g_pauses == 0 && g_modelessTicks == t0 + 1);   // one beat = one call; the key stays
        IdleHomed();
        const int t1 = g_modelessTicks;
        Beat();
        CHECK(g_modelessTicks == t1);                                            // no box: not called
        if (argc > 1) {                                                          // the reader itself (wb_serve, not linked here)
            CHECK(CountOf(ws, "void W906_MsgBoxModelessPanelKeyTick()") == 1 &&
                  CountOf(ws, "const std::string io = W906MbIoDismiss();") == 2 &&   // MbWait's + the non-modal reader's
                  CountOf(ws, "g_mbModelessSeqSt02 = g_dialogSeq;") == 1);
        }
    }

    // [17] the panel HOME while START is blocked (manual teach / the Teach Arm Cell job): TfMain::Home asks W906_HomeBlockedHook first
    //   and refuses before golden's first statement (NB2-1 !146 HOMEBLOCK, U14 item 3, R188 H1).
    std::printf("[17] panel HOME while START is blocked: TfMain::Home refuses first (NB2-1 !146)\n");
    {
        IdleHomed();
        fAllMotorHome = false;                                                   // a machine to home (= [6])
        W906_HomeBlockedHook = &FakeHomeBlocked;
        g_interlockBlocks = true;  g_homeLockDepth = -1;
        const int r0 = W906_HomeRefusedCount;
        Press(SnFKHome);
        CHECK(g_interlockAsked == 1 && g_homeAskedFunc == "ScanKey" && W906_HomeRefusedCount == r0 + 1 &&
              SoftStart == false && iHome == 0 && NoRow());                     // refused: no MES2112, nothing armed
        CHECK(g_homeLockDepth == 0 && g_lockDepth == 0);                         // ADAPTED: no FormLock around HOME (WebMainScanKey.cpp:19)
        IdleHomed();
        fAllMotorHome = false;
        Press(SnFKHome);                                                         // not blocked: golden goes on (= [6])
        CHECK(g_interlockAsked == 1 && SoftStart == true && iHome == 1 && Row("MES2112", "HOME pressed", "ScanKey"));
        W906_HomeBlockedHook = 0;
        IdleHomed();
    }

    {   // AI(W906-RSTHELD) 20261007: a held (stuck) ALARM RESET no longer eats the other panel keys; a fresh RESET press still wins
        IdleHomed();
        SimOn(SnFKAlarmReset);
        const int r1 = ScanPannelKey();                                          // fresh press: RESET
        const int r2 = ScanPannelKey();                                          // held: latched, nothing
        SimOn(SnFKHome);
        const int r3 = ScanPannelKey();                                          // HOME while RESET is still held (golden: -1)
        SimOff(SnFKHome);  SimOff(SnFKAlarmReset);  ScanPannelKey();  ScanPannelKey();
        CHECK(r1 == SnFKAlarmReset && r2 == -1 && r3 == SnFKHome);
        SimOn(SnFKHome);  SimOn(SnFKAlarmReset);
        const int r4 = ScanPannelKey();                                          // both pressed fresh together: RESET (golden order)
        SimOff(SnFKHome);  SimOff(SnFKAlarmReset);  ScanPannelKey();  ScanPannelKey();
        CHECK(r4 == SnFKAlarmReset);
        IdleHomed();
    }

    // [18] AI(W906-ST02-P1) 20261005 (St02-E): [15] again with the RS-232 operator pad (ControlPanelMode=1, PadInterface_St02.h).
    //   The keys reach Sen[].IsOn() through the mysensor.cpp pad gate (golden mysensor.cpp:81) by their golden names, so the
    //   first-scan guard (WebMainScanKey.cpp MskFirstScanHeld) needs no pad code.  The pad state is set the way
    //   TfPadInterface::DoScanPanelLed leaves it (PadItem[].mlEvent->Value; tests/test_st02_pad_interface.cpp drives the frames).
    std::printf("[18] ControlPanelMode 1 (RS-232 pad): START held since boot ignored, RESET fires once, rear pad START\n");
    {
        const int   padIds[]   = { SnFKStart, SnFKReset, SnRKStart, SnRearPadActive };
        const char* padNames[] = { "SnFKStart", "SnFKReset", "SnRKStart", "SnRearPadActive" };
        AnsiString  savedNames[4];   bool savedEn18[4];  int savedTy18[4], savedIsa18[4];   // AI(W906-W149) 20261007 (St02-E): [18] hands the four sensors' sim state back as it found it
        for (int i = 0; i < 4; ++i) { savedEn18[i] = Sen[padIds[i]].Enable;  savedTy18[i] = Sen[padIds[i]].Type;  savedIsa18[i] = Sen[padIds[i]].ISABase;  savedNames[i] = Sen[padIds[i]].Name; Sen[padIds[i]].Name = padNames[i]; SimOff(padIds[i]); }   // IO says OFF
        PAD_LED_W906* const fkStart = fPadInterface->PadItem[6].mlEvent;          // golden uPadInterface.cpp:217 / :214 / :230 / :241
        PAD_LED_W906* const fkReset = fPadInterface->PadItem[3].mlEvent;
        PAD_LED_W906* const rkStart = fPadInterface->PadItem[19].mlEvent;
        PAD_LED_W906* const rearOn  = fPadInterface->PadItem[30].mlEvent;
        iControlPanelMode = 1;
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (a) front START held since boot (first frame had 0x40)
        fkStart->Value = true;
        Beat();
        CHECK(g_starts == 0);                                                    // ignored, as [15](a)
        fkStart->Value = false;
        Beat();
        MarkLog();  fkStart->Value = true;  Beat();  fkStart->Value = false;  ScanPannelKey();
        CHECK(g_starts == 1 && g_startFunc == "ScanKey_2");                     // the next pad press starts
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (b) RESET held since boot: not guarded
        fkReset->Value = true;
        Beat();
        CHECK(LogRow("MES2113", "RESET pressed", "ScanKey"));
        fkReset->Value = false;
        ScanPannelKey();
        IdleHomed();
        W906_ScanKeyFirstScanRearm_St02();                                       // (c) rear pad active, rear START held since boot
        rearOn->Value = true;  rkStart->Value = true;
        Beat();
        CHECK(g_starts == 0);
        rkStart->Value = false;
        Beat();
        rkStart->Value = true;  Beat();  rkStart->Value = false;  ScanPannelKey();
        CHECK(g_starts == 1);
        rearOn->Value = false;
        iControlPanelMode = 0;                                                   // (d) control: mode 0 reads the IO (OFF), not the pad
        IdleHomed();
        Beat();
        fkStart->Value = true;
        Beat();  Beat();
        CHECK(g_starts == 0);
        fkStart->Value = false;
        for (int i = 0; i < 4; ++i) { Sen[padIds[i]].Name = savedNames[i]; Sen[padIds[i]].Enable = savedEn18[i];  Sen[padIds[i]].Type = savedTy18[i];  Sen[padIds[i]].ISABase = savedIsa18[i]; }   // AI(W906-W149) 20261007 (St02-E): was SimUnknown -- it left SnRearPadActive unreadable, so a later section's front ALARM RESET was ignored (the machine's RSTHELD block failed after [18] / [19] until b0ba0894 moved it before [18])
        IdleHomed();
        Beat();   CHECK(Sen[SnRearPadActive].Enable == savedEn18[3] && Sen[SnRearPadActive].Type == savedTy18[3] && Sen[SnFKStart].Enable == savedEn18[0]);   // AI(W906-W149) 20261007 (St02-E): handed back
    }

    // [19] AI(W906-SOFTKEY) ST02-P2 20261006 (St02-E): screen panel keys (WS panel.key -> W906_SoftPanelKeyPush, WebMainScanKey.cpp EOF) set
    //   golden's own software-key flags bAse* (cmydef.h:3691-3701; golden's ASE remote sets them); ScanPannelKey answers them in its key
    //   blocks, so the per-key side effects run (the machine's cpp 0153 hook returned BEFORE them: a soft ALARM RESET skipped the N07
    //   silence) and golden's gates hold (bDisibleResetButton).  An unread key is dropped after 3 s; one press per key until read; <= 4 unread.
    std::printf("[19] soft panel keys through golden's bAse* flags: START, ALARM RESET side effects, RESET gate, expiry, limits\n");
    {
        extern bool W906_SoftPanelKeyPush(const std::string&, std::string&);
        extern void W906_SoftPanelKeyResetForTestSt02();
        extern void W906_SoftPanelKeyAgeForTestSt02(unsigned long ms);
        std::string why;   bool en19[33];  int ty19[33], isa19[33];  for (int k = 0; k < 33; ++k) { const int id = k < 32 ? k : SnRearPadActive;  en19[k] = Sen[id].Enable;  ty19[k] = Sen[id].Type;  isa19[k] = Sen[id].ISABase; }   // AI(W906-W149) 20261007 (St02-E): [19] hands the 32 key sensors + SnRearPadActive back as it found them (its AllKeysUnknown below leaves them unreadable; InitialOK is IdleHomed()'s, :252)
        IdleHomed();
        SimOff(SnRearPadActive);                                                 // front pad: golden's front ALARM RESET blocks (ckernel.cpp ScanPannelKey)
        Beat();
        CHECK(!W906_SoftPanelKeyPush("NOPE", why) && why.find("NOPE") != std::string::npos);
        CHECK(W906_SoftPanelKeyPush("START", why) && bAseStart == true);         // (a) START = one press
        Beat();
        CHECK(g_starts == 1 && bAseStart == false);                              // ScanPannelKey consumed the flag; the start seat once (as [2])
        ScanPannelKey();                                                         // ScanPannelKey's release sweep (bK latch)
        IdleHomed();                                                             // (b) ALARM RESET: golden's front block side effects run
        bN07AlarmActive = true;  bN07BuzzerSilenced = false;  bNeedMusicAndAlarmOn = true;  bTesterPauseMusic = true;
        CHECK(W906_SoftPanelKeyPush("ALARMRESET", why));
        Beat();
        CHECK(bAseAlarmReset == false && bN07BuzzerSilenced == true && bNeedMusicAndAlarmOn == false && bTesterPauseMusic == false);
        bN07AlarmActive = false;  bN07BuzzerSilenced = false;
        ScanPannelKey();
        IdleHomed();                                                             // (c) RESET under golden's bDisibleResetButton: not read
        IniConfig.bDisibleResetButton = true;
        CHECK(W906_SoftPanelKeyPush("RESET", why));
        Beat();
        CHECK(bAseReset == true && NoRow());                                     // golden's gate held it: no MES2113
        CHECK(!W906_SoftPanelKeyPush("RESET", why) && why.find("RESET") != std::string::npos);   // one press per key until read
        W906_SoftPanelKeyAgeForTestSt02(3100);
        Beat();
        CHECK(bAseReset == false);                                               // dropped by the tick after 3 s (never fires late)
        InitialOK = false;                                                       // (d) the ack names the gates and the previous key's fate
        CHECK(W906_SoftPanelKeyPush("PAUSE", why) && why.find("InitialOK=0") != std::string::npos && why.find("RESET 3") != std::string::npos);
        IniConfig.bDisibleResetButton = false;
        CHECK(W906_SoftPanelKeyPush("ONECYCLE", why) && W906_SoftPanelKeyPush("CLEANOUT", why) && W906_SoftPanelKeyPush("TRAYFEED", why));
        CHECK(!W906_SoftPanelKeyPush("SKIP", why) && why.find("4") != std::string::npos);   // (e) at most 4 unread
        W906_SoftPanelKeyResetForTestSt02();
        CHECK(!bAsePause && !bAseOneCycle && !bAseCleanOut && !bAseTrayFeed && !bAseSKIP);
        IdleHomed();
        AllKeysUnknown();
        Beat();   for (int k = 0; k < 33; ++k) { const int id = k < 32 ? k : SnRearPadActive;  Sen[id].Enable = en19[k];  Sen[id].Type = ty19[k];  Sen[id].ISABase = isa19[k]; }   CHECK(Sen[SnRearPadActive].Enable == en19[32] && Sen[SnRearPadActive].Type == ty19[32] && Sen[SnFKAlarmReset].Enable == en19[SnFKAlarmReset]);   // AI(W906-W149) 20261007 (St02-E): handed back
    }

    IdleHomed();
    InitialOK = false;  SystemInitialOK = false;
    AllKeysUnknown();
    RestoreBins();
    W906_RemoteRun = savedRun;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
