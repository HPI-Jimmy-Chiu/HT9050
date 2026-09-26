// =============================================================================
//  atester_shims.h  --  W6.4 offline cross-module shims for the TESTER/INDEX engine
//
//  Translation wave: W6.4 (TESTER/INDEX ENGINE -- atester.cpp)
//  Translator: AI(W6.4-TESTER) 20260626
//
//  WHY THIS FILE EXISTS
//  --------------------
//  The faithfully-transcribed tester/index engine (atester.cpp) reaches a set of
//  cross-module subsystems that have NO translated home this wave:
//    * atester_32Site.cpp   (4749 lines -- W6.4b sibling)
//    * atester_ProcessCount.cpp (2161 lines -- W6.4b sibling)
//    * aTester_Front.cpp / aTester_Rear.cpp (front/rear test-head SMs -- W6.4b)
//    * fContact (cContact.h, PACKAGE) -- contact-mode index-check form
//    * the ADAM_* EP-pressure DAQ API (adam6024.h)
//
//  Mirroring acatchtray_shims, this header exposes ONLY the surface the engine
//  dereferences, on transparently-named non-VCL stand-ins, with offline-safe
//  bodies (true / false / 0 / no-op) in atester_shims.cpp.  Per the W6 pattern,
//  the 32Site/Front/Rear "Do*" report COMPLETE (true) so the gated SM advances,
//  "Init*" are no-op, and their cursor ints live here (init per golden =1).
//
//  Each symbol cites its golden home.  When the real subsystem lands (W6.4b/W7),
//  these shims are replaced and the call sites bind to the real bodies unchanged.
//
//  Big5: Chinese comments decoded via cp950, preserved UTF-8.  ZERO U+FFFD.
// =============================================================================
#ifndef atester_shimsH
#define atester_shimsH

#include "vclcompat/vcl_compat.h"   // AnsiString
#include "vclcompat/Controls.h"     // AI(W906-W7-F2) 20260729: TPanel -- the unified stock-widget stand-in that replaced this file's own TfObserverLabel
#include "MachineType.h"            // MAX_SOCKET_ROW / MAX_SOCKET_COL
#include <vector>                   // AI(W906-SaveTestSummarySECS) 20260721: TfObserverMemoLotSummaryLines::Strings

// ===========================================================================
//  atester_32Site.cpp -- AI(W5-Automation-Integrate) 20260710: ALL 10 symbols
//  formerly shimmed here are now REAL (atester_32Site.h/.cpp, added to
//  ht9045_sm -- see CMakeLists.txt).  atester.cpp now
//  `#include "atester_32Site.h"` directly for these declarations; the offline
//  stand-ins that used to live here have been REMOVED (would otherwise
//  duplicate-define InitTestSuckTestIC_TwoArm32Site_Task/InitTestYTwoArm32SiteTask/
//  DoInterFaceErrorStep_TwoArm32Site/DoTestSuckTestIC_TwoArm32Site/
//  DoTestY_TwoArm32Site/Do32SiteTestDestroyIC/Do32SiteTestSuckIC/
//  i32RTCAutoModelVerifyTask/iTestSuckTestIC_TwoArm32Site_Task/
//  iTestTwoArm32SiteTask).
// ===========================================================================

// ===========================================================================
//  atester_ProcessCount.cpp -- AI(W5-Automation-Integrate) 20260710: 11 of the
//  symbols formerly shimmed here are now REAL (atester_ProcessCount.h/.cpp,
//  added to ht9045_sm) -- InitialPiggyBackFunction/ProcessPiggyBackFunction/
//  RecordPiggyBackStartEnd/ProcessSocketPurgeCount/CheckContinuoussFail/
//  ProcessContinuoussFailForATC/CheckContinuoussPassBySocket/
//  CheckContinuoussPass/DoLowYieldAlarm/DoLowYieldAlarmFirst/
//  ProcessAutoloadcellMeasureCount -- REMOVED from here (would otherwise
//  duplicate-define).  atester.cpp now `#include "atester_ProcessCount.h"`
//  directly for those.  The remaining 5 golden atester_ProcessCount.cpp
//  exports are STILL out of scope this wave and STAY here exactly as before
//  (do NOT re-declare them in atester_ProcessCount.h -- see that header's own
//  scope note): ProcessShowTestStatus / ProcessCount / ProcessStartTestData /
//  RecordHistroy / ProcessQASampling.
// ===========================================================================
void ProcessShowTestStatus(int Index);         // (also declared in atester.h)
void ProcessCount(int Index, bool bHasIC=true);// (also declared in atester.h)  //Eastsun 20260515 F022: D7
void ProcessStartTestData(int Direct);
void RecordHistroy(int attr);                  // golden -- (__fastcall removed)
void ProcessQASampling(int Index);

// ===========================================================================
//  aTester_Front.cpp / aTester_Rear.cpp  -- golden aTester_Front.h / aTester_Rear.h.
//  Sibling test-head SMs (8+ call sites in DoTestY / DoTestHeadMotor).  Offline:
//  Do* = "complete" (true) so the dispatcher advances; Init* = no-op; cursor ints
//  owned here.  Front/Rear share the same offline contract as the 3 arm engines.
// ===========================================================================
//  -- Front cursors (golden aTester_Front.h) --
extern int iFTestSuckTestICTask;
// AI(W64b-Integrate) 20260706: iFrontTestDestroyICTask now defined for real in
// aTester_Front.cpp/.h (golden aTester_Front.h:300) -- extern moved there.
// AI(W5-Automation-Integrate) 20260710: iFrontTestSuckICTask now defined for
// real in aTester_Front.cpp/.h too -- extern moved there (REMOVED here).
extern int iTestYFrontTask;
extern int iFRTCUseSocketFloatTask;
extern int iFRTCAutoModelVerifyTask;
extern int iFrontTestPurgBeforePickShuttle;
extern int iIndexArm2PickUpErrNeedPiggybackTask;
extern int iFTestSocketClampCloseTask;
extern int iFTestSocketClampOpenTask;
extern bool bArm2NeedCombine;                  //JerryYang 20180520 (wei)
extern bool bArm1SuckFinish[4][8];
int  CheckAnyCaseNeedToDoArm1();               // golden -- offline 0
void InitTestYFrontTask();
void InitFTestSuckTestICTask();                // (also declared in atester.h)
// AI(W64b-Integrate) 20260706: InitFrontTestDestroyICTask/TestZ1SetPos/
// DoFrontTestDestroyIC/CheckZ1IsDown/TestZ1OutRandge now defined for real in
// aTester_Front.cpp; declarations moved to aTester_Front.h (golden aTester_Front.h).
// AI(W5-Automation-Integrate) 20260710: InitFrontTestSuckICTask/DoFrontTestSuckIC
// now defined for real in aTester_Front.cpp too -- declarations moved there
// (REMOVED here).
void InitFrontTestPurgBeforePickShuttle();
void DoArm1Suck();
void DoArm1D44VacCheck();
bool FTestNeedDestroy();
bool DoFTestSuckTestIC();                       // golden -- offline complete (true)
bool DoFrontTestPurgBeforePickShuttle(int isp);
bool DoTestYFront();                            // golden -- offline complete (true)
bool TestZ1OutRandge2();
bool FTestSeparateSLK(bool bReset=false);
bool FTestCombineSLK(bool bReset=false);
bool DoFRTCAutoModelVerify(bool bInitial);
//  -- Rear cursors (golden aTester_Rear.h) --
extern int iBTestSuckTestICTask;
// AI(W64b-Integrate) 20260706: iRearTestDestroyICTask now defined for real in
// aTester_Rear.cpp/.h (golden aTester_Rear.h:300) -- extern moved there.
// AI(W5-Automation-Integrate) 20260710: iRearTestSuckICTask now defined for
// real in aTester_Rear.cpp/.h too -- extern moved there (REMOVED here).
extern int iTestYRearTask;
extern int iBRTCUseSocketFloatTask;
extern int iBRTCGiveWayCheckTask;
extern int iBRTCAutoModelVerifyTask;
extern int iBTestSocketClampCloseTask;
extern int iBTestSocketClampOpenTask;
extern int iRearTestPurgBeforePickShuttle;
extern int iIndexArm1PickUpErrNeedPiggybackTask;
extern bool bArm2SuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL];
int  CheckAnyCaseNeedToDoArm2();               // golden -- offline 0
void InitTestYRearTask();
void InitBTestSuckTestICTask();
// AI(W64b-Integrate) 20260706: InitRearTestDestroyICTask/TestZ2SetPos/
// DoRearTestDestroyIC/CheckZ2IsDown/TestZ2OutRandge now defined for real in
// aTester_Rear.cpp; declarations moved to aTester_Rear.h (golden aTester_Rear.h).
// AI(W5-Automation-Integrate) 20260710: InitRearTestSuckICTask/DoRearTestSuckIC
// now defined for real in aTester_Rear.cpp too -- declarations moved there
// (REMOVED here).
void InitRearTestPurgBeforePickShuttle();
void DoArm2Suck();
void DoArm2D44VacCheck();
bool BTestNeedDestroy();
bool DoBTestSuckTestIC();                        // golden -- offline complete (true)
bool DoRearTestPurgBeforePickShuttle(int isp);
bool DoTestYRear();                             // golden -- offline complete (true)
bool TestZ2OutRandge2();
bool BTestCombineSLK(bool bReset=false);
bool BTestSeparateSLK(bool bReset=false);
bool DoBRTCAutoModelVerify(bool bInitial);

// ===========================================================================
//  fContact (cContact.h, PACKAGE TfContact* fContact) -- contact-mode index-check
//  form.  atester.cpp derefs it ~57x; the only value read on an ACTIVE skeleton
//  path is fContact->fShow (ProcessTestResult :2693).  Minimal offline mirror:
//  fShow=false so the bin-set guard takes its "can set bin" branch.  The contact
//  SM DoIndecxCHECkFunction itself is NOT called from atester.cpp (see roadmap).
// ===========================================================================
class TfContactShim
{
public:
    bool fShow;                                 // golden cContact.h -- form visible? (offline false)
    // -----------------------------------------------------------------------
    //  AI(W906-W7-L2) 20260803: FIVE members added for golden ckernel.cpp's
    //  manual-key trio -- WaitManualStepKey (golden ckernel.cpp:54-94),
    //  WaitManualStartKey (:96-131) and WaitManualRetryKey (:133-149; that third
    //  one derefs fShuttleMove, NOT fContact, so it needs nothing from here).
    //  All five golden declarations were read directly from golden cContact.h
    //  this wave (cp950); the two widgets reuse the unified stand-ins from
    //  vclcompat/Controls.h (already included above and already hoisted into the
    //  global namespace by Controls.h:481/:484), per forms/FormWidgets.h's own
    //  banner rule "if a wave thinks it needs a new widget type here, the answer
    //  is almost always an existing vclcompat/Controls.h type".  forms/
    //  FormWidgets.h itself needs NO change for this wave.
    //
    //  GOLDEN ODDITY TO PRESERVE, NOT TO "FIX" (verified by reading both bodies):
    //  the Step and Start early-outs test a STRUCTURALLY IDENTICAL condition --
    //      :56-57  if(fContact->fShow && fContact->cbOneTouchAutoContactHight->Checked==true) return TRUE;
    //      :98-99  if(fContact->fShow && fContact->cbOneTouchAutoContactHight->Checked==true) return FALSE;
    //  -- and return OPPOSITE signs.  THE ASYMMETRY IS A VERIFIED FACT (both
    //  bodies read from golden this wave); the WHY below is my INFERENCE and is
    //  labelled as such -- golden's own comment on both lines says only
    //  "//Ifor 20220803 add 一鍵完成Auto Contact Hight" (one-key-completes Auto
    //  Contact Height) and gives no rationale.  Plausible reading: under one-key
    //  mode the operator presses nothing, so STEP is auto-granted (true) while
    //  START is auto-denied (false), leaving the contact-height SM single-stepping
    //  under its own control instead of free-running.  Whatever the reason, the
    //  ckernel translation MUST reproduce both signs verbatim -- do not "align"
    //  them.
    // -----------------------------------------------------------------------
    // Offline default Checked=false (vclcompat::TCheckBox ctor, Controls.h:358).
    // That default CLOSES the one-touch early-out at BOTH golden ckernel.cpp:56-57
    // and :98-99, so WaitManualStepKey falls through to its real manual-step
    // polling body (golden :59-93) and WaitManualStartKey to its own (:101-130).
    // Today the `fContact->fShow &&` left conjunct (offline false) already
    // short-circuits both tests, so this default is currently masked -- it becomes
    // load-bearing the moment the contact form is ever shown, and false is the
    // value that keeps the manual-key path alive rather than auto-answering it.
    TCheckBox *cbOneTouchAutoContactHight;      // golden cContact.h:305 (TCheckBox* cbOneTouchAutoContactHight)
    // Offline default Caption="" (AnsiString).  Honest statement: this default
    // selects NO branch -- ->Caption is WRITE-ONLY across the whole golden tree.
    // Exhaustive grep of golden *.cpp/*.h for `btnTStep->`/`btnTStart->` this wave
    // returns 11 hits and NOT ONE is a read: the four Caption ASSIGNMENTS in
    // ckernel.cpp (:62/:64 "T.Step"/"", :104/:106 "T.Start"/"", which merely blink
    // the on-screen key label in step with bLampManualSetp/bLampManualStart), four
    // ->Visible writes on THIS form (cContact.cpp:1603/1604, :1637/1638), and three
    // ->Visible writes on a DIFFERENT form's same-named member (ShuttleMove.cpp
    // :2075/:2077/:2090 -- TfShuttleMove has its own btnTStep; do not conflate).
    // So the ckernel translation writes into a display sink, and "" vs anything
    // else cannot change control flow.
    TButton   *btnTStep;                        // golden cContact.h:79 (TButton* btnTStep)
    TButton   *btnTStart;                       // golden cContact.h:78 (TButton* btnTStart)
    // Offline default false -- and this one is NOT an offline invention: golden's
    // own TfContact constructor assigns exactly `bSetupStart=false;` (golden
    // cContact.cpp:238) and `bSetupStep=false;` (:239), right beside the
    // `fShow=false;` this shim already mirrors (:237).  They are the click-latches
    // for the two on-screen keys (golden cContact.cpp:2251-2254 btnTStartClick sets
    // bSetupStart=true; :2256-2259 btnTStepClick sets bSetupStep=true).
    // Branch selected: false makes the FIRST disjunct of golden's 3-way OR false at
    // ckernel.cpp:74 (bSetupStep || SnRKManualStep || bButtonManualStep) and :116
    // (bSetupStart || SnRKManualTStart || bButtonManualTStart), so with no real
    // key/sensor event both functions reach `return false` (:93 / :130) -- i.e. the
    // SM KEEPS WAITING for a genuine manual key.  Deliberately NOT this file's
    // "report OK so the gated SM advances" idiom: auto-firing a manual STEP/START
    // would make the handler self-advance through an operator-gated contact
    // sequence, which is the opposite of golden.
    bool bSetupStart;                           // golden cContact.h:546
    bool bSetupStep;                            // golden cContact.h:553
    bool Do_ROILearning();                      // golden -- RTC ROI learning (offline: done=true)
    // -- W6.2b1x1 ADD: in-arm 1x1_1 place SM (DoInArmPlaceToShuttle_9045_1x1_1)
    //    reads fContact->IsRun2DCheck() (golden cContact.h:627).  Offline: not
    //    running a 2DID re-check -> false.
    bool IsRun2DCheck();                        // golden cContact.h:627 -- offline false
    // -- W906-AutoCleanCluster ADD: DoAutoCleanKit's optional pre-clean "Full
    //    View Check" RTC hand-shake (golden cContact.h:577/578, body cContact.cpp
    //    :15064-15162 [not just :15064-15130 -- cases 400/500/600 run past that,
    //    see below] -- a multi-step COM2/vision-comm state machine). TWO call
    //    sites in golden, not one: (1) AutoClean.cpp:4514-4521/4537, gated behind
    //    `CosFunction.bFullTestBeforeAutoClean && REAL_TIME_CCD==true &&
    //    !COM2->bCCDDummyRum`; (2) cContact.cpp:11906-11963 (TfContact::
    //    DoTestContactFunction, case 70/75), gated behind the structurally
    //    identical `REAL_TIME_CCD==true && !COM2->bCCDDummyRum &&
    //    CosFunction.bFullTestBeforeContactHeight`. COM2->bCCDDummyRum is
    //    offline `true` (TCOM2Shim ctor above), so `!COM2->bCCDDummyRum` is
    //    unconditionally false offline at BOTH sites: dead-by-construction
    //    under every reachable state today (site 2's enclosing
    //    DoTestContactFunction is also itself still untranslated, cContact.h,
    //    ~11347 lines -- a second, independent reason it can't fire yet).
    //    Matches this file's established "Do* reports complete (true) so the
    //    gated SM advances, Init* is a no-op" idiom (banner comment above)
    //    rather than translating the real vision-comm SM.
    void InitDoFullViewCheck();                 // golden cContact.h:577 -- offline no-op
    bool DoFullViewCheck();                     // golden cContact.h:578 -- offline true (complete)
    TfContactShim();
};
extern TfContactShim *fContact;                 // golden cContact.h:667 (PACKAGE TfContact* fContact)

// ===========================================================================
//  ADAM_* EP-pressure DAQ API (golden adam6024.h).  IndexEveryTimeCheckEP /
//  CheckAndRecodrEP drive the EP balloon pressure.  Offline: no DAQ card -> write
//  ops no-op, Alarm() reports "no alarm" (false) so the EP-check SM advances.
// ===========================================================================
//AI(W906-P2b-CF) 20260919: 第三個參數補回來。
// golden adam6024.h:23 是 `void ADAM_DirectWriteData(WORD data, int Num, int iAdd=1);`
// —— **三個參數**，第三個有預設值 1。移植樹的樁只接了兩個，
// 而既有的四個呼叫點（asendic_Loader.cpp:482/486/847/890）剛好都只傳
// 兩個，所以一直沒被發現。W6-G 解閘後的 `ADAM_DirectWriteData(iInputValue, 0, 0)`
// （golden main.cpp:6252）傳三個，編譯器才把它抓出來。
// 補回預設值不會動到那四個呼叫點（省略就是 1，與 golden 相同）。
void ADAM_DirectWriteData(int iData, int iCh, int iAdd = 1);  // golden adam6024.h:23 -- no-op
void ADAM_WriteVoltage(double dVolt);            // golden adam6024.h -- no-op
bool ADAM_Alarm();                               // golden adam6024.h -- offline false
bool ADAM_Alarm(int iCh);                        // golden adam6024.h -- offline false
void ADAM_Rang(int iRange);                      // golden adam6024.h -- no-op

// ===========================================================================
//  EP switch + heater-OK + NN-mode + in-arm-giveway cross-module helpers.
//  These are golden free functions/enums the tester engine derefs:
//    * EPSwitchOnOff / enum EPSwOn          -- golden adam6024.h:46/47-50 (EP DAQ; offline no-op)
//    * CheckHeaterOK                        -- golden aArmHeader.h:77 / uHeaterThread.h:23 (offline true)
//    * IsNNMode                             -- golden cinitial.h:60 (offline 0 = not NN mode)
//    * CheckInArmFinishAllPickerAction      -- golden ainarm2.h:95 (offline true = finished)
//    * MoveInArm2XYToWait                   -- golden ainarm2.h:146 (offline true = at wait pos)
//    * iCASE_REAL_CCD2                      -- golden cinitial.h RTC-CCD entry case id
// ===========================================================================
enum EPSwOn{ eEPSwOff=0, eEPSwArm1=1, eEPSwArm2=2, eEPSwBoth=3 };  // golden adam6024.h:47-50
void EPSwitchOnOff(int iArm);                    // golden adam6024.h:46 -- offline no-op
bool CheckHeaterOK();                            // golden aArmHeader.h:77 -- offline true (heater ready)
int  IsNNMode();                                 // golden cinitial.h:60 -- offline 0 (not NN)
// In-arm give-way helpers the tester engine calls (IndexAlarmInArmAway, atester.cpp
// :264/:272).  DECLARED here so atester.cpp resolves them; DEFINED ONCE in
// acatchtray_shims.cpp (do NOT define in atester_shims.cpp -> multiple-definition).
bool CheckInArmFinishAllPickerAction();          // golden ainarm2.h:95  -- offline true (finished)
bool MoveInArm2XYToWait();                       // golden ainarm2.h:146 -- offline true (at wait pos)
// AI(W7T1-Integrate) 20260701: iCASE_REAL_CCD2 moved to its golden file-scope home in
// atester.cpp (const int iCASE_REAL_CCD2..6 = 40200..40510, golden atester.cpp:5551-5555).
// The W6.4 shim value =9 was valid only while the RTC case tree was gated; the tree is now
// ACTIVE so =9 would collide with `case 9:`.  extern removed here; definition removed in .cpp.

// ===========================================================================
//  Untranslated VCL forms the tester engine derefs on its ACTIVE entry preamble
//  / oracle-window cases.  Minimal non-VCL stand-ins; offline-safe members only.
//    * CCDInterfaceForm  -- golden CCDInterface.h (CCD identification form)
//    * fAutomation       -- golden Automation/automation.h:151 (host-command form)
//    * fObserver         -- golden cObserver.h (OEE observer)
//    * fiosetview        -- golden iosetview.h (index-suck IO view; bIndexSuck grid)
// ===========================================================================
class TCCDInterfaceFormShim
{
public:
    bool bAtestScanCCDProgram;                   // golden CCDInterface.h:99
    void CCDTimerOnOff(bool bOn);                // golden CCDInterface.h:95 -- offline no-op
    bool CCDRunExec();                           // golden CCDInterface.h:96 -- offline false
    TCCDInterfaceFormShim();
};
extern TCCDInterfaceFormShim *CCDInterfaceForm;  // golden CCDInterface.h

class TfAutomationShim
{
public:
    void DoCommandBuffer(AnsiString Command, AnsiString SubCommand="",
                         AnsiString AlarmMessage="", int ErrType=0, AnsiString AlarmID=""); // golden automation.h:113
    bool GetEventNum(int &num, AnsiString cmdstr, AnsiString numstr);   // golden automation.h:123 -- offline false
    bool GetEventResult(int num);                                       // golden automation.h:125 -- offline false
    int  GetEventStrResult(int num);                                   // golden automation.h:126 -- offline 0
    void ClearEvent(int num);                                          // golden automation.h:128 -- offline no-op
    // -- W5-Final-Auto9045 INTEGRATE ADD: members Automation/auto9045.cpp's
    //    AmkorUDPGetSiteAndTemp/GetSite/GetTemp + GetTesterMode/SetTesterMode
    //    derefs (golden automation.h) -- ADDITIVE ONLY this integrate pass:
    //    auto9045.cpp still uses its own TU-local W5FA_TfAutomationExt
    //    equivalent (not retargeted here, see this integrate's own report for
    //    the risk/scope rationale); these members are available for a future
    //    consolidation wave.
    int  TestMode;                      // golden automation.h -- host-selected tester mode echo
    char sATKSendMessage[256];          // golden automation.h -- ATK UDP site/temp packet buffer
    int  sATKSendMessageCount;          // golden automation.h -- current packet length
    TfAutomationShim();
};
extern TfAutomationShim *fAutomation;            // golden automation.h:151 (PACKAGE TfAutomation* fAutomation)

// =============================================================================
// AI(W906-FW-ObsSwap) 20260818: TfObserverShim RETIRED (user-approved queue).
// The live `fObserver` global is now backed by the REAL TfObserver facade
// (forms/fObserver.h, cObserver Waves 1/2), and its definition homecame to
// cObserver.cpp. What moved where:
//   * TfObserverMemoLines0 / TfObserverMemoLotSummaryLines /
//     TfObserverMemoLotSummary -- moved VERBATIM into forms/fObserver.h
//     (same names, same shapes; consumers compile unchanged).
//   * labModel/labPowerOnTime/.../labFactory, Memo1Lines, memoLotSummary,
//     bTestIndexZ, RecordInArmTime, AddTimeData -- all carried by the real
//     facade (Waves 1/2 + the swap-enablement block).
// Consequence, stated loudly: RecordInArmTime/AddTimeData at ~146 call sites
// switch from the shim's no-ops to the REAL translated OEE bookkeeping.
// =============================================================================
#include "forms/fObserver.h"

// (TfObserverShim class, its memo structs, and the extern all retired here --
//  see the FW-ObsSwap banner above for where each piece lives now.)

class TfiosetviewShim
{
public:
    bool bIndexSuck[2][4][8];                    // golden iosetview.h:3014
    bool fShow;                                  //AI(W906-GB-P2c) 20260926: golden 912 MSG_CMD_RemoteStart/Stop reads fiosetview->fShow (main.cpp:16431-16470); offline false (the IO-set view is a web page)
    TfiosetviewShim();
};
extern TfiosetviewShim *fiosetview;              // golden iosetview.h

// ===========================================================================
//  COM2 (golden rs232.h:203 PACKAGE TCOM2* COM2) -- the DTK RTC/CCD serial-comm
//  module (W5 tester-comm).  The tester engine derefs COM2->bCCDDummyRum (DoTestY
//  case 20) + COM2->DoReleaseAndInspEnd() (DoTestHeadMotor case 600000).  Offline:
//  bCCDDummyRum=true (so the RTC/CCD branches short-circuit), DoReleaseAndInspEnd
//  no-op.  HTimer (golden cpublic.h fwd) is a minimal timer stand-in used only by
//  the gated GetTesterResult body (bTestFailNeedWait).
// ===========================================================================
#include "myTimer.h"                             // AI(W906-R28TORQ) 20260925: TQPF_Timer member below (golden rs232.h:7); <windows.h> it pulls is already in via vclcompat/vcl_compat.h
class TCOM2Shim
{
public:
    bool bCCDDummyRum;                           // golden rs232.h:157 -- offline true (no real RTC dummy run)
    void DoReleaseAndInspEnd();                  // golden rs232.h:160 -- offline no-op
    // -----------------------------------------------------------------------
    //  AI(W906-W7-L2) 20260803: ATCAlarmSenCheck -- needed by golden ckernel.cpp
    //  :445 inside ScanSystemSensor.  NOT a stub: the real golden body (rs232.cpp
    //  :4262-4275) is a pure bUT150Install[] x Sen[].IsOff() scan with NO
    //  untranslated dependency left, so it is TRANSLATED FAITHFULLY in
    //  atester_shims.cpp (every symbol it touches already exists in the ported
    //  tree: bUT150Install cmydef.h:3305/cmydef.cpp:3531, SnATCAlarm1
    //  cmydef.h:944/cmydef.cpp:1068 =214, tcAa1/tcAb1/tcAa2/tcAb2
    //  MachineType.h:639-640, Sen[] mysensor.h:48/mysensor.cpp:34,
    //  TMySensor::IsOff mysensor.cpp:163).
    //
    //  WHICH BRANCH THE RETURN VALUE SELECTS AT THE CALL SITE (read, not assumed):
    //      golden ckernel.cpp:445  if(bATCInitialFinish==false || COM2->ATCAlarmSenCheck()==false)
    //      golden ckernel.cpp:447      ShowMyMessage("ATC Alarm Sensor Off,...");
    //      golden ckernel.cpp:448      return false;      // ScanSystemSensor REJECTS the start
    //  so at this site "OK" means TRUE.  Returning true takes the fall-through arm
    //  (ScanSystemSensor continues past :450); returning false raises the ATC alarm
    //  dialog and aborts.  The faithful body evaluates to TRUE offline, because
    //  bUT150Install[] is all-false (cmydef.cpp:3531 `={false}` and nothing in the
    //  ported tree assigns it yet -- MainCalcCore.h:78/:89 records the golden writer
    //  as still-untranslated), so the loop body never runs.
    //
    //  BUT DO NOT READ THAT AS "the ATC gate passes offline" -- two things above it
    //  dominate, and both were checked this wave:
    //    (1) the OR short-circuits on `bATCInitialFinish==false` FIRST, and that is
    //        false offline (cmydef.cpp:305), so if the block were ever reached the
    //        alarm arm would be taken NO MATTER what this function returns;
    //    (2) the block is unreachable offline anyway -- its enclosing guard
    //        (golden ckernel.cpp:442) requires ATC_SYSTEM==eATCSiliconType, and
    //        ATC_SYSTEM is 0/eATCUninstall (cmydef.cpp:3567) while eATCSiliconType
    //        is 3 (MachineType.h:693).
    //  Same call shape at golden aTester_Front.cpp:6070 / aTester_Rear.cpp:6329.
    // -----------------------------------------------------------------------
    bool ATCAlarmSenCheck();                     // golden rs232.h:171 (body rs232.cpp:4262-4275) -- faithful translation
    TCOM2Shim();

    // -----------------------------------------------------------------------
    //  AI(W906-R28TORQ) 20260925: Index Z 扭力子系統（golden TCOM2 的扭力那一半，rs232.h／rs232.cpp）。
    //  使用者 20260925 裁決第 6 條「出貨組態的 Index Z 扭力上限要對齊原 BCB6 版本做法」（docs/RULINGS_20260925.md §6，
    //  NB2 R14 S1／R16）：atester.cpp 的 COM2->iWriteAndCheckMotorTorque 以前走回 1 的替身（W7T1_TCOM2Ext），
    //  真機上 Prod.iMaxPreasure 根本沒寫進驅動器卻回報成功。本體全部在 rs232.cpp（照 golden 翻），這裡只放宣告。
    //  golden 的 COM2 是 TCOM2（TDataModule，HT9045.cpp:174 CreateForm；rs232.cpp:34 `TCOM2 *COM2;`），扭力走它的
    //  Comm1（SPComm TComm，rs232.dfm:39-71；RS232Init rs232.cpp:236-262 以 HSys.sTorqueComPort 開埠）。
    //  移植樹沒有 TCOM2，COM2 一直是這個 TCOM2Shim —— 所以就把 golden 的扭力成員補在這個唯一的 COM2 物件上。
    //  ⚠ 沒有搬的：HP 通訊卡（TorqueUseHPComCard）那一半 —— 見 rs232.cpp 檔頭「HP 通訊卡為什麼閘著」。
    //    RTC 視覺（bRealTimeCom_ReceiveOK／SendCommToVision…）、溫控 Comm2、ATC、PadComm 都不是這條路，沒動。
    // -----------------------------------------------------------------------
    TComm *Comm1;                                                    // golden rs232.h:15 (rs232.dfm:39 object Comm1: TComm)
    void RS232Init();                                                // golden rs232.h:70  (body rs232.cpp:149-688)
    void TorqueSend(unsigned char *str, int len);                    // golden rs232.h:71  (rs232.cpp:765)
    void ReadTorque();                                               // golden rs232.h:74  (rs232.cpp:809)
    void ReadTorque_Panasonic();                                     // golden rs232.h:75  (rs232.cpp:820)
    bool ReadTorque_Mitsubishi();                                    // golden rs232.h:76  (rs232.cpp:2186)
    void ReadWriterParameter();                                      // golden rs232.h:86  (rs232.cpp:1513)
    void ReadWriterParameter_Panasonic();                            // golden rs232.h:87  (rs232.cpp:1523)
    void ReadWriterParameter_Mitsubishi();                           // golden rs232.h:88  (rs232.cpp:2114)
    void WriteIndexTorqueSetting(int Index, AnsiString Torque);      // golden rs232.h:92  (rs232.cpp:789)
    void ReadIndexTorqueSetting(int Index);                          // golden rs232.h:93  (rs232.cpp:779)
    void WriteIndexTorqueSetting_Pana(int Index, unsigned Data);     // golden rs232.h:95  (rs232.cpp:1414)
    void ReadIndexTorqueSetting_Pana(int index);                     // golden rs232.h:96  (rs232.cpp:1368)
    void WriteIndexTorqueSetting_Mitu(int iIndex, double fTorque);   // golden rs232.h:98  (rs232.cpp:2156)
    void ReadIndexTorqueSetting_Mitu(int iIndex);                    // golden rs232.h:99  (rs232.cpp:2144)
    double DoChangeASCII_TO_INT(char *Input);                        // golden rs232.h:104 (rs232.cpp:2174)
    bool DoSet_Torque_Action(int iIndex, double fTorque);            // golden rs232.h:105 (rs232.cpp:2466)
    bool DoRead_Now_Torque_Action(int iIndex);                       // golden rs232.h:106 (rs232.cpp:2920)
    void InitSetTorque_Mitsubishi();                                 // golden rs232.h:107 (rs232.cpp:2460)
    void InitReadTorque_Mitsubishi();                                // golden rs232.h:108 (rs232.cpp:2915)
    int  iWriteAndCheckMotorTorque(int MotorIndex, int Torque);      // golden rs232.h:109 (rs232.cpp:1847-2009)
    void ResetPanasonicTime();                                       // golden rs232.h:110 (rs232.cpp:1504)
    AnsiString asReceiveTorue;                                       // golden rs232.h:121
    void InitReadTorueTask();                                        // golden rs232.h:122 (rs232.cpp:803)
    int  GetReadTorueTask();                                         // golden rs232.h:162 (rs232.cpp:3763)
    int  rwCommandDelay;                                             // golden rs232.h:173
    TQPF_Timer iWriteAndCheckMotorTorqueDelay;                       // golden rs232.h:174（WebStart.cpp 照 golden main.cpp:5528 呼叫 .On()）
    bool fPanasonicParameterRW;                                      // golden rs232.h:179
    void Comm1ReceiveData(void *Sender, void *Buffer, unsigned short BufferLength);   // golden rs232.h:28 (rs232.cpp:1750)
    // 不是 golden 的成員：golden 的 SPComm 在主執行緒、以 ReadIntervalTimeout=100 ms 切好一包才呼叫 Comm1ReceiveData；
    //   vclcompat 的 TComm 在讀取執行緒上逐塊呼叫。這兩支把那個語意補回來 —— 讀取執行緒只排隊，
    //   MainProc 的節拍（ReadTorque／ReadWriterParameter 開頭）才交付。理由與細節見 rs232.cpp 檔頭。
    void W906_Comm1QueueRx(void *Buffer, unsigned short BufferLength);
    void W906_PumpComm1();
    void W906_CreateFormBoot();                                      // golden TCOM2 建構子（rs232.cpp:95-125）裡讀 Gerneral.ini 的那一段；
                                                                     //   在 golden CreateForm(TCOM2) 的位置（HT9045.cpp:174）由 wb_serve 呼叫
private:
    int autoTask;                                                    // golden rs232.h:55
};
extern TCOM2Shim *COM2;                          // golden rs232.h:203 (PACKAGE TCOM2* COM2)
void InitWriteAndCheckMotorTorqueTask();         // golden rs232.h:205 (body rs232.cpp:1838-1845) -- AI(W906-R28TORQ) 20260925: 本體在 rs232.cpp

struct HTimer { bool Off(){ return true; } void SetSecAndOn(double){} };  // golden cpublic.h fwd (W5); gated-use only

#endif // atester_shimsH
