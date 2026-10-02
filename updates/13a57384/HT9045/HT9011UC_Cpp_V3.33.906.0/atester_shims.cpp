// =============================================================================
//  atester_shims.cpp  --  offline bodies for the W6.4 TESTER/INDEX cross-module shims
//
//  Translation wave: W6.4 (TESTER/INDEX ENGINE -- atester.cpp)
//  Translator: AI(W6.4-TESTER) 20260626
//
//  All bodies are CONSERVATIVE offline defaults over the Sim HAL.  See
//  atester_shims.h for per-symbol rationale + golden provenance.
//    * 32Site / Front / Rear "Do*" report COMPLETE (true) so the gated tester SM
//      skeletons advance; "Init*" no-op; cursor ints home to the golden value =1.
//    * ProcessCount void bodies no-op; DoLowYieldAlarm returns 0 (no alarm).
//    * fContact->fShow=false (so ProcessTestResult takes "can set bin");
//      Do_ROILearning()=true (learning "done").
//    * ADAM_* EP DAQ: writes no-op, Alarm()=false (no EP leak) so EP-check advances.
//
//  Big5: Chinese comments preserved as UTF-8 (cp950).  ZERO U+FFFD.
// =============================================================================
#include "atester_shims.h"
#include "csystem.h"                 // IsInArmCleanOutFinish / IsInArmOneCycleFinish /
                                     // CanYieldAlarmRemainInSHT / CheckIndexConnect declarations
// AI(W64b-Integrate) 20260706: TestIF_File / site-mode enums for the GetSiteCount
// shim below (golden home cprod.cpp, whole function-body region gated -- see there).
#include "cprod.h"
// AI(W5-Automation-Integrate) 20260710: TMyKitSuck (CheckTestSuckICOn stand-in below).
#include "aHotPlateSubstrate.h"
// AI(W906-cContactLeaf) 20260721: ComputeIsRun2DCheck (real TfContact::IsRun2DCheck body,
// translated as a free function) for the TfContactShim::IsRun2DCheck() wire-up below.
#include "cContact.h"
// AI(W906-W7-L2) 20260803: substrate for the FAITHFUL TCOM2Shim::ATCAlarmSenCheck
// translation below -- cmydef.h gives bUT150Install[] (:3305) + SnATCAlarm1 (:944),
// mysensor.h gives Sen[]/TMySensor::IsOff (:48/:43).  tcAa1/tcAb1/tcAa2/tcAb2 come
// from MachineType.h:639-640, already pulled in by atester_shims.h.
#include "cmydef.h"
#include "mysensor.h"

// ===========================================================================
//  csystem.h cross-module predicates referenced ONLY by the tester/index engine
//  (their real homes are ainarm / index-connect HW, NOT translated this wave).
//  Offline-safe defaults (the W6 contract):
//    * IsInArmCleanOutFinish / IsInArmOneCycleFinish -> false (no clean-out/one-cycle
//      pending) so DoTestY case 1 takes the normal dispatch (-> Task=50, golden :4890).
//    * CanYieldAlarmRemainInSHT -> false (no yield-alarm half-cycle pending).
//    * CheckIndexConnect -> 0 (all index heaters "connected", no WAR0360).
//  These are the csystem.h-declared bodies; no other translated TU defines them.
// ===========================================================================
#if 0   // PT-W5c RETIRED (IsInArmCleanOutFinish)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool IsInArmCleanOutFinish(int /*iIsOneCycle*/) { return false; }  // golden csystem.h:89
#endif
#if 0   // PT-W5c RETIRED (IsInArmOneCycleFinish)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool IsInArmOneCycleFinish()                    { return false; }  // golden csystem.h:91
#endif
#if 0   // PT-W5f RETIRED (CanYieldAlarmRemainInSHT)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
bool CanYieldAlarmRemainInSHT()                 { return false; }  // golden csystem.h:288
#endif
#if 0   // PT-W5f RETIRED (CheckIndexConnect)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
int  CheckIndexConnect()                        { return 0; }      // golden csystem.h:212 (Dell 20111130)
#endif
// TODO(W6.4b): real body golden csystem.cpp:23513, remove this stub when translated.
//   DoTestHeadMotor (atester.cpp:1732) fires the TTL RS232 SOT signal to the dual
//   TTL board.  Offline: no RS232/TTL board wired -> no-op (the SOT pulse is a
//   hardware side-effect; the index SM advances regardless of its return).
#if 0   // PT-W5f RETIRED (SendTTLRS232CSOTsignal)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void SendTTLRS232CSOTsignal()                   {}                 // golden csystem.h:280 (Isaac 20210309)
#endif

// AI(W5-Automation-Integrate) 20260710: 3 more csystem.h-declared bodies atester_32Site.cpp
// needs (golden home csystem.cpp, whole regions not translated this wave -- HeadContactCount/
// temperature-log/index-time-log subsystems).  Same "these are the csystem.h-declared bodies;
// no other translated TU defines them" contract as the 4 above.
//   CheckContactOver -> false: golden guards its whole body on CosFunction.bUseHeadContactCount
//     (default false offline), so false matches the feature-disabled golden outcome exactly.
//   ShowIndexTime -> no-op: pure debug index-timing log, no control-flow effect on callers.
//   TemperatureStorageLog -> false: golden's own first statement is `if(InitialOK==false) return
//     false;` (offline InitialOK is false), matching the not-yet-initialized golden outcome.
#if 0   // PT-W5c RETIRED (CheckContactOver)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckContactOver()                         { return false; }  // golden csystem.h:156
#endif
#if 0   // PT-W5f RETIRED (ShowIndexTime)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error, measured in build_0810_w5f.
void ShowIndexTime(int /*Item*/)                {}                 // golden csystem.h:200
#endif
#if 0   // PT-W5f RETIRED (TemperatureStorageLog)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
bool TemperatureStorageLog(int /*iRecord*/)     { return false; }  // golden csystem.h:275
#endif

// AI(W5-Automation-Integrate) 20260710: atester_32Site.cpp's own local `extern` globals/
// predicate (golden main.cpp-owned bEcho/bExist/bUnderTest/bEchoStop -- same untranslated-main.cpp
// gap already documented in Interface/InterfaceSYS.cpp's file header; CheckTestSuckICOn is a
// golden atester.cpp-family leaf with no translated home yet).  Offline-safe defaults: the bools
// start false (no echo/exist/under-test/stop-request pending, matching a freshly-started handler);
// CheckTestSuckICOn->false (no per-site suck-IC-on-during-drop alarm) -- and its only call site
// (atester_32Site.cpp :1063/:1069) is itself guarded by `if(LastSet.iRealDummy==REALLY)` (false
// in the Sim canary), so this is inert offline either way.
bool bEcho=false, bExist=false, bUnderTest=false;                  // golden main.cpp (untranslated)
bool bEchoStop=false;                                              // golden main.cpp (untranslated) //ChungHung 20130326 add
#if 0   // PT-W5c RETIRED (CheckTestSuckICOn)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckTestSuckICOn(TMyKitSuck &/*Ptr*/, int /*iR*/, int /*iC*/) { return false; }
#endif

// AI(W906-TesterTCPTimer) 20260720: iBin[4][8] production definition -- golden
// main.cpp:15156 (untranslated, file-scope, zero-initialized). Same
// untranslated-main.cpp-owned-global family as bEcho/bExist/bUnderTest just
// above (atester.cpp:652 and Interface/InterfaceSYS.cpp:77 both already
// `extern unsigned int iBin[4][8];` this symbol). Until now the ONLY definition
// lived test-locally in tests/test_interfacesys.cpp -- moved here (the real
// ht9045_sm production home) because Interface/TesterTCP_Socket.cpp's new
// TimerProcessTCPDataTimer/SimulateBin (this wave) write iBin directly, so
// test_testertcp_socket now also needs a link-time definition, not just
// test_interfacesys. tests/test_interfacesys.cpp's own local definition was
// changed to `extern` in the same step to avoid a duplicate-symbol link error
// (see that file's own comment).
unsigned int iBin[4][8];

// AI(W64b-Integrate) 20260706: GetSiteCount (golden cprod.cpp:305, declared
// cprod.h:3299) -- discovered as an undefined-reference link error while
// integrating aTester_Front.cpp/aTester_Rear.cpp (case 650/750:
// `CosFunction.bD44Use4ea && GetSiteCount()>4`).  The real golden body is a
// pure TestIF_File.iTestMode/iSiteMap[][] computation with NO untranslated
// substrate dependency, BUT it textually lives inside cprod.cpp's whole-file
// `#if 0 // TODO(W6): function bodies depend on untranslated state machines +
// globals` region (cprod.cpp:184-4036), so it never actually compiles into
// libht9045_globals.a today.  Faithful duplicate here (same pattern already
// used a few lines up for IsInArmCleanOutFinish/IsInArmOneCycleFinish, which
// are ALSO real csystem.cpp bodies shimmed here for the identical reason).
// TODO(W6): delete this duplicate once cprod.cpp's #if 0 gate lifts (would
// otherwise ODR-conflict with the real definition).
// AI(W906-GA1-B2-integrate) 20260804: GetSiteCount duplicate RETIRED exactly as
// this banner's own TODO(W6) anticipated -- cprod.cpp's gate lifted (GA-1-B2), the
// real golden body now compiles into libht9045_globals.a.

// ---- atester_32Site: AI(W5-Automation-Integrate) 20260710: ALL bodies that
// used to live here are now REAL (atester_32Site.cpp) -- REMOVED (would
// otherwise duplicate-define).

// ---- atester_ProcessCount: AI(W5-Automation-Integrate) 20260710: 11 bodies
// that used to live here are now REAL (atester_ProcessCount.cpp) -- REMOVED
// (would otherwise duplicate-define).  The 5 still-out-of-scope exports keep
// their offline no-op bodies here exactly as before.
#if 0   // PT-W8 RETIRED (ProcessShowTestStatus): real translated body now live
void ProcessShowTestStatus(int /*Index*/)   {}
#endif
#if 0   // PT-W8 RETIRED (ProcessCount): real translated body now live
void ProcessCount(int /*Index*/, bool /*bHasIC*/) {}    //Eastsun 20260515 F022: D7
#endif
#if 0   // PT-W8 RETIRED (ProcessStartTestData): real translated body now live
void ProcessStartTestData(int /*Direct*/)   {}
#endif
#if 0   // PT-W8 RETIRED (RecordHistroy): real translated body now live
void RecordHistroy(int /*attr*/)            {}
#endif
#if 0   // PT-W8 RETIRED (ProcessQASampling): real translated body now live
void ProcessQASampling(int /*Index*/)       {}
#endif

// ---- aTester_Front cursors + bodies ----------------------------------------
int iFTestSuckTestICTask                  = 1;
// AI(W64b-Integrate) 20260706: iFrontTestDestroyICTask now defined for real in
// aTester_Front.cpp (golden aTester_Front.cpp:300) -- stub definition removed.
// AI(W5-Automation-Integrate) 20260710: iFrontTestSuckICTask now defined for
// real in aTester_Front.cpp too -- stub definition removed.
int iTestYFrontTask                       = 1;
int iFRTCUseSocketFloatTask               = 1;
int iFRTCAutoModelVerifyTask              = 1;
int iFrontTestPurgBeforePickShuttle       = 1;
int iIndexArm2PickUpErrNeedPiggybackTask  = 1;
int iFTestSocketClampCloseTask            = 0;   //AI(W906-FLOW-1) 20260927: golden aTester_Front.cpp:73 has no initialiser (=0); FTestSeparateSLK sets 1 on reset / no-SLK (golden :8117-8123), called only in the test flow -- was 1
int iFTestSocketClampOpenTask             = 0;   //AI(W906-FLOW-1) 20260927: golden aTester_Front.cpp:74 has no initialiser (=0); FTestCombineSLK sets 1 on reset / no-SLK (golden :8219-8223) -- was 1
bool bArm2NeedCombine                     = false;
bool bArm1SuckFinish[4][8]                = {{false}};
#if 0   // PT-W7b RETIRED (CheckAnyCaseNeedToDoArm1): real body now lives in aTester_Rear.cpp
int  CheckAnyCaseNeedToDoArm1()             { return 0; }
#endif
#if 0   // PT-W7b RETIRED (InitTestYFrontTask): real body now lives in aTester_Rear.cpp
void InitTestYFrontTask()                   {}
#endif
#if 0   // PT-W7b RETIRED (InitFTestSuckTestICTask): real body now lives in aTester_Rear.cpp
void InitFTestSuckTestICTask()              {}
#endif
// AI(W64b-Integrate) 20260706: InitFrontTestDestroyICTask/TestZ1SetPos/
// DoFrontTestDestroyIC/CheckZ1IsDown/TestZ1OutRandge now defined for real in
// aTester_Front.cpp (golden aTester_Front.cpp:170-298,309-865) -- stub bodies removed.
// AI(W5-Automation-Integrate) 20260710: InitFrontTestSuckICTask/DoFrontTestSuckIC
// now defined for real in aTester_Front.cpp too -- stub bodies removed.
#if 0   // PT-W7b RETIRED (InitFrontTestPurgBeforePickShuttle): real body now lives in aTester_Rear.cpp
void InitFrontTestPurgBeforePickShuttle()   {}
#endif
#if 0   // PT-W7b RETIRED (DoArm1Suck): real body now lives in aTester_Rear.cpp
void DoArm1Suck()                           {}
#endif
#if 0   // PT-W7b RETIRED (DoArm1D44VacCheck): real body now lives in aTester_Rear.cpp
void DoArm1D44VacCheck()                    {}
#endif
#if 0   // PT-W7b RETIRED (FTestNeedDestroy): real body now lives in aTester_Rear.cpp
bool FTestNeedDestroy()                     { return false; }
#endif
#if 0   // PT-W7b RETIRED (DoFTestSuckTestIC): real body now lives in aTester_Rear.cpp
bool DoFTestSuckTestIC()                    { return true;  }
#endif
#if 0   // PT-W7b RETIRED (DoFrontTestPurgBeforePickShuttle): real body now lives in aTester_Rear.cpp
bool DoFrontTestPurgBeforePickShuttle(int /*isp*/) { return true; }
#endif
#if 0   // PT-W7b RETIRED (DoTestYFront): real body now lives in aTester_Rear.cpp
bool DoTestYFront()                         { return true;  }
#endif
#if 0   // PT-W7b RETIRED (TestZ1OutRandge2): real body now lives in aTester_Rear.cpp
bool TestZ1OutRandge2()                     { return false; }
#endif
#if 0   // PT-W7b RETIRED (FTestSeparateSLK): real body now lives in aTester_Rear.cpp
bool FTestSeparateSLK(bool /*bReset*/)      { return true;  }
#endif
#if 0   // PT-W7b RETIRED (FTestCombineSLK): real body now lives in aTester_Rear.cpp
bool FTestCombineSLK(bool /*bReset*/)       { return true;  }
#endif
#if 0   // PT-W7b RETIRED (DoFRTCAutoModelVerify): real body now lives in aTester_Rear.cpp
bool DoFRTCAutoModelVerify(bool /*bInitial*/){ return true; }
#endif

// ---- aTester_Rear cursors + bodies -----------------------------------------
int iBTestSuckTestICTask                  = 1;
// AI(W64b-Integrate) 20260706: iRearTestDestroyICTask now defined for real in
// aTester_Rear.cpp (golden aTester_Rear.cpp:300) -- stub definition removed.
// AI(W5-Automation-Integrate) 20260710: iRearTestSuckICTask now defined for
// real in aTester_Rear.cpp too -- stub definition removed.
int iTestYRearTask                        = 1;
int iBRTCUseSocketFloatTask               = 1;
int iBRTCGiveWayCheckTask                 = 1;
int iBRTCAutoModelVerifyTask              = 1;
int iBTestSocketClampCloseTask            = 0;   //AI(W906-FLOW-1) 20260927: golden aTester_Rear.cpp:74 has no initialiser (=0); BTestSeparateSLK (:9335) resets it in the test flow -- was 1
int iBTestSocketClampOpenTask             = 0;   //AI(W906-FLOW-1) 20260927: golden aTester_Rear.cpp:75 has no initialiser (=0); BTestCombineSLK (:9442) resets it in the test flow -- was 1
int iRearTestPurgBeforePickShuttle        = 1;
int iIndexArm1PickUpErrNeedPiggybackTask  = 1;
bool bArm2SuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL] = {{false}};
#if 0   // PT-W7b RETIRED (CheckAnyCaseNeedToDoArm2): real body now lives in aTester_Rear.cpp
int  CheckAnyCaseNeedToDoArm2()             { return 0; }
#endif
#if 0   // PT-W7b RETIRED (InitTestYRearTask): real body now lives in aTester_Rear.cpp
void InitTestYRearTask()                    {}
#endif
#if 0   // PT-W7b RETIRED (InitBTestSuckTestICTask): real body now lives in aTester_Rear.cpp
void InitBTestSuckTestICTask()              {}
#endif
// AI(W64b-Integrate) 20260706: InitRearTestDestroyICTask/TestZ2SetPos/
// DoRearTestDestroyIC/CheckZ2IsDown/TestZ2OutRandge now defined for real in
// aTester_Rear.cpp (golden aTester_Rear.cpp:170-298,309-886) -- stub bodies removed.
// AI(W5-Automation-Integrate) 20260710: InitRearTestSuckICTask/DoRearTestSuckIC
// now defined for real in aTester_Rear.cpp too -- stub bodies removed.
#if 0   // PT-W7b RETIRED (InitRearTestPurgBeforePickShuttle): real body now lives in aTester_Rear.cpp
void InitRearTestPurgBeforePickShuttle()    {}
#endif
#if 0   // PT-W7b RETIRED (DoArm2Suck): real body now lives in aTester_Rear.cpp
void DoArm2Suck()                           {}
#endif
#if 0   // PT-W7b RETIRED (DoArm2D44VacCheck): real body now lives in aTester_Rear.cpp
void DoArm2D44VacCheck()                    {}
#endif
#if 0   // PT-W7b RETIRED (BTestNeedDestroy): real body now lives in aTester_Rear.cpp
bool BTestNeedDestroy()                     { return false; }
#endif
#if 0   // PT-W7b RETIRED (DoBTestSuckTestIC): real body now lives in aTester_Rear.cpp
bool DoBTestSuckTestIC()                    { return true;  }
#endif
#if 0   // PT-W7b RETIRED (DoRearTestPurgBeforePickShuttle): real body now lives in aTester_Rear.cpp
bool DoRearTestPurgBeforePickShuttle(int /*isp*/) { return true; }
#endif
#if 0   // PT-W7b RETIRED (DoTestYRear): real body now lives in aTester_Rear.cpp
bool DoTestYRear()                          { return true;  }
#endif
#if 0   // PT-W7b RETIRED (TestZ2OutRandge2): real body now lives in aTester_Rear.cpp
bool TestZ2OutRandge2()                     { return false; }
#endif
#if 0   // PT-W7b RETIRED (BTestCombineSLK): real body now lives in aTester_Rear.cpp
bool BTestCombineSLK(bool /*bReset*/)       { return true;  }
#endif
#if 0   // PT-W7b RETIRED (BTestSeparateSLK): real body now lives in aTester_Rear.cpp
bool BTestSeparateSLK(bool /*bReset*/)      { return true;  }
#endif
#if 0   // PT-W7b RETIRED (DoBRTCAutoModelVerify): real body now lives in aTester_Rear.cpp
bool DoBRTCAutoModelVerify(bool /*bInitial*/){ return true; }
#endif

// ---- fContact (offline contact-mode form) ----------------------------------
// AI(W906-W7-L2) 20260803: bSetupStart/bSetupStep added to the init list and the two
// key widgets allocated -- these mirror golden TfContact's OWN constructor, which sets
// `fShow=false; bSetupStart=false; bSetupStep=false;` on three consecutive lines
// (golden cContact.cpp:237-239), so the shim's ctor is now a faithful echo of golden's
// rather than an offline guess.  Widget defaults come straight from the unified
// stand-ins (TCheckBox::Checked=false Controls.h:358; TButton::Caption="" via
// AnsiString) -- see atester_shims.h for the per-member branch justification.
TfContactShim::TfContactShim() : fShow(false), bSetupStart(false), bSetupStep(false)
{
    cbOneTouchAutoContactHight = new TCheckBox();   // golden cContact.h:305
    btnTStep                   = new TButton();     // golden cContact.h:79
    btnTStart                  = new TButton();     // golden cContact.h:78
}
bool TfContactShim::Do_ROILearning() { return true; }   // offline: ROI learning "done"
// AI(W906-cContactLeaf) 20260721: was a hardcoded `return false;` stub (W6.2b1x1 note, now
// superseded).  Swapped in the real golden predicate (TfContact::IsRun2DCheck, cContact.cpp:
// 21203-21219, translated as ComputeIsRun2DCheck in cContact.h/.cpp) over the real globals it
// reads.  Checked this wave: bRun2DCheck (cmydef.h:5830) has no ASSIGNMENT anywhere in the   //AI(W906-AMB-L2) 20260929: stale -- bRun2DCheck is now assigned at csystem.cpp:8162 (DoTrayFeedProcess, live since AMB-L2) and :9591, so IsRun2DCheck() can return true; its consumer ainarm9045.cpp:1152 keeps GetInArm2DIDMapping() held (GATE h4-G3, INBOX 111b item 4)
// translated tree yet -- only its own zero-init definition (cmydef.cpp:5811, un-gated this same
// wave; see the AI(W906-cContactLeaf) comment there) -- so this still evaluates to `false` under
// every reachable state today, identical to the old stub; see cContact.h's ComputeIsRun2DCheck
// banner for the full call-site risk check.
bool TfContactShim::IsRun2DCheck()
{
    return ComputeIsRun2DCheck(IniConfig.bF33_Check2DHardware,
                                BAR_CODE_INSTALL,
                                TestIF_File.bEnableBarCode,
                                InArmSuck.iShtRow,
                                iContactMode,
                                bRun2DCheck);
}
void TfContactShim::InitDoFullViewCheck() {}                   // offline no-op
bool TfContactShim::DoFullViewCheck() { return true; }         // offline: reached (true) -- see .h banner
TfContactShim *fContact = new TfContactShim();

// ---- ADAM_* EP DAQ (offline: no DAQ card) ----------------------------------  //AI(W906-FLOW-5) 20260929: real blocker is the missing ADAMTCP transport binding, not a missing card (see the atester_shims.h ADAM_WriteVoltage note)  //AI(W906-ST02-ADAM) 20261002 (St02-E helper H4): the five stand-ins below are RETIRED -- golden bodies Adam6024Comm_St02.cpp (ADAM_DirectWriteData / ADAM_WriteVoltage, golden 912 adam6024.cpp:1952-2037 / :1814-1931) and Adam6024Pressure_St02.cpp (ADAM_Alarm :549-605 / ADAM_Rang :543-546); with the EP live switch OFF they answer exactly like these did (no-op / false)
//AI(W906-ST02-ADAM) 20261002: RETIRED (see :325) -- was: void ADAM_DirectWriteData(int /*iData*/, int /*iCh*/, int /*iAdd*/) {}
//AI(W906-ST02-ADAM) 20261002: RETIRED (see :325) -- was: void ADAM_WriteVoltage(double /*dVolt*/)              {}
//AI(W906-ST02-ADAM) 20261002: RETIRED (see :325) -- was: bool ADAM_Alarm()                                     { return false; }
//AI(W906-ST02-ADAM) 20261002: RETIRED (see :325) -- was: bool ADAM_Alarm(int /*iCh*/)                          { return false; }
//AI(W906-ST02-ADAM) 20261002: RETIRED (see :325) -- was: void ADAM_Rang(int /*iRange*/)                        {}

// ---- EP switch / heater-OK / NN-mode / in-arm-giveway helpers --------------
void EPSwitchOnOff(int iArm);                                              //AI(W906-FLOW-2) 20260928: stub RETIRED -- golden body (adam6024.cpp:3016-3049, with EpSwitch :3051-3101) now lives at the end of adam6024.cpp (its golden home). The old reason "offline: no EP DAQ" was wrong: golden switches the SW[SwEpArm1/2] / SW[SwIndEpArm1/2] digital outputs, all live; keeping an empty body here would be a multiple-definition error
#if 0   // PT-W5c RETIRED (CheckHeaterOK)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
bool CheckHeaterOK()                                  { return true; }  // offline: heater ready
#endif
#if 0   // PT-W7a RETIRED (IsNNMode)
//AI(ht9045-v906) 20260810: PT-W7a -- RETIRED. cinitial.cpp now holds golden's real body (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0810_w7a.
int  IsNNMode()                                       { return 0; }     // offline: not NN mode
#endif
// CheckInArmFinishAllPickerAction() / MoveInArm2XYToWait() are defined ONCE in
// acatchtray_shims.cpp (golden ainarm2.h:95 / :146).  Declared in atester_shims.h
// so atester.cpp sees them, but NOT re-defined here (would be a multiple-definition
// ODR clash at link).  TODO(W6.4b): real bodies golden ainarm2.cpp; both shims drop
// together when the in-arm give-way SM is translated.
// AI(W7T1-Integrate) 20260701: iCASE_REAL_CCD2 definition REMOVED -- moved to its golden
// file-scope home in atester.cpp (const int iCASE_REAL_CCD2..6 = 40200..40510, golden
// atester.cpp:5551-5555).  The W6.4 =9 remap was valid only while the RTC case tree was
// gated; the tree is now ACTIVE so =9 would collide with the active `case 9:`.

// ---- CCDInterfaceForm (offline CCD identification form) ---------------------
TCCDInterfaceFormShim::TCCDInterfaceFormShim() : bAtestScanCCDProgram(false) {}
void TCCDInterfaceFormShim::CCDTimerOnOff(bool /*bOn*/) {}
bool TCCDInterfaceFormShim::CCDRunExec()               { return false; }
TCCDInterfaceFormShim *CCDInterfaceForm = new TCCDInterfaceFormShim();

// ---- fAutomation (offline host-command form) -------------------------------
// -- W5-Final-Auto9045 INTEGRATE ADD: TestMode/sATKSendMessage/sATKSendMessageCount --
TfAutomationShim::TfAutomationShim() : TestMode(0), sATKSendMessageCount(0)
{
    for(int i=0;i<256;i++) sATKSendMessage[i]=0;
}
void TfAutomationShim::DoCommandBuffer(AnsiString /*Command*/, AnsiString /*SubCommand*/,
                                       AnsiString /*AlarmMessage*/, int /*ErrType*/, AnsiString /*AlarmID*/) {}
bool TfAutomationShim::GetEventNum(int & /*num*/, AnsiString /*cmdstr*/, AnsiString /*numstr*/) { return false; }
bool TfAutomationShim::GetEventResult(int /*num*/)     { return false; }
int  TfAutomationShim::GetEventStrResult(int /*num*/)  { return 0; }
void TfAutomationShim::ClearEvent(int /*num*/)         {}
TfAutomationShim *fAutomation = new TfAutomationShim();

// ---- fObserver (offline OEE observer) --------------------------------------
// AI(W906-FW-ObsSwap) 20260818: TfObserverShim ctor/methods/global RETIRED --
// the live fObserver is now the REAL TfObserver facade, defined at its golden
// home (cObserver.cpp). RecordInArmTime/AddTimeData call sites tree-wide now
// bind to the real translated OEE bookkeeping instead of these no-ops.

// ---- fiosetview (offline index-suck IO view) -------------------------------
TfiosetviewShim::TfiosetviewShim() : fShow(false) { for(int a=0;a<2;a++) for(int b=0;b<4;b++) for(int c=0;c<8;c++) { bIndexSuck[a][b][c]=false; bIndexDestroy[a][b][c]=false; } }   //AI(W906-IDXSUCK) 20260927: +bIndexDestroy（golden TForm 零初始化）   //AI(W906-GB-P2c) 20260926: golden 912 fShow (P2c)
TfiosetviewShim *fiosetview = new TfiosetviewShim();

// ---- COM2 (offline DTK RTC/CCD serial-comm module, W5) ---------------------
// AI(W906-R28TORQ) 20260925: the TCOM2Shim constructor MOVED to rs232.cpp (next to the Index Z torque
// members it now has to initialise -- Comm1 and its golden rs232.dfm design-time values).
// bCCDDummyRum is still set to true there, so the RTC/CCD branches still short-circuit exactly as before.
void TCOM2Shim::DoReleaseAndInspEnd() {}
// AI(W906-W7-L2) 20260803: FAITHFUL translation of golden TCOM2::ATCAlarmSenCheck
// (golden rs232.cpp:4262-4275, declared rs232.h:171), needed by golden ckernel.cpp:445.
// Transcribed statement-for-statement, INCLUDING golden's own misspelled local
// `bRetrun` and its `==true` comparisons -- this is a translation, not a rewrite.
// Semantics: report ATC-alarm-sensors-OK (true) unless some INSTALLED UT150 ATC site
// has its alarm sensor Off.  Note golden indexes bUT150Install[] by iATCSite[i] but
// Sen[] by the plain loop counter (SnATCAlarm1+i) -- that asymmetry is golden's and is
// preserved verbatim.  See atester_shims.h for the full call-site branch analysis
// (short version: at ckernel.cpp:445 "OK" == true, and offline this returns true
// because bUT150Install[] is all-false, but the enclosing ATC_SYSTEM gate at :442 makes
// the whole block unreachable offline regardless).
bool TCOM2Shim::ATCAlarmSenCheck()                                              // golden rs232.cpp:4262
{
    bool bRetrun=true;
    int iATCSite[4]={tcAa1, tcAb1, tcAa2, tcAb2};

    for(int i=0; i<4; i++)
    {
        if(bUT150Install[iATCSite[i]]==true && Sen[SnATCAlarm1+i].IsOff()==true)
        {
            bRetrun=false;
        }
    }
    return bRetrun;
}
TCOM2Shim *COM2 = new TCOM2Shim();

// =============================================================================
//  AI(W906-IDXSUCK) 20260927: golden Tfiosetview::ProcessIndexSuckDestroy2／1（iosetview.cpp:1881-1962）逐行照翻 —— Index 吸嘴真空／破真空的「泵」：
//  對 bIndexSuck／bIndexDestroy 標了旗標的吸嘴逐一做 TMySucker 的 Suck()／Destroy()，全部做完（iCt==0）才回 true。
//  放在全域 fiosetview 真正指到的 TfiosetviewShim（golden 只有一個 fiosetview，就是那個表單；移植樹的 Tfiosetview 類別
//  在 forms/fIoSetView.h、沒有全域指標，見那個檔頭）。以前各檔自己有一組一律回 true 的替身（atester.cpp W7T1_、atester_32Site.cpp
//  W5_32S_、aTester_Front.cpp W64B_／W7cK7_、aTester_Rear.cpp W64bT2_／W7bK8_、AutoClean.cpp W906DIAC_），現在都轉呼叫這裡。
//  INDEX_SUCKER_TYPE==0 的機台不會設旗標（golden 只有 type 1 才寫 bIndexSuck／bIndexDestroy）⇒ iCt==0、立刻回 true，跟替身一樣；
//  type 1（動作流程對照用的真機 HT9046_LS 就是）才真的開／破真空（INBOX 第 84 列）。
// =============================================================================
bool TfiosetviewShim::ProcessIndexSuckDestroy2(int iType)                           //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
{
    int iCt=0;

    for(int i=0; i<BTestSuck.iShtRow; i++)
    {
        for(int j=0; j<BTestSuck.iShtCol; j++)
        {
            if(CosFunction.bD44Once4Suck && iType==1 && iCt>=4)                 //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
                continue;
            if(bIndexSuck[1][i][j]==true)
            {
                iCt++;
                if(BTestSuck.Suck[i][j].Suck())
                {
                    bIndexSuck[1][i][j]=false;
                }

                if(BTestSuck.Suck[i][j].Error==true)
                {
                    BTestSuck.Suck[i][j].Error=false;
                    bIndexSuck[1][i][j]=false;                                  //Sam 20221007 : 修正index check error
                }
            }
            else if(bIndexDestroy[1][i][j]==true)                               //jou 2011-11-01 加個else避免真空破壞同時開啟
            {
                iCt++;
                if(BTestSuck.Suck[i][j].Destroy())
                {
                    bIndexDestroy[1][i][j]=false;
                }
            }
        }
    }

    if(iCt==0)
        return true;
    else
        return false;
}
//---------------------------------------------------------------------------
bool TfiosetviewShim::ProcessIndexSuckDestroy1(int iType)                           //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆 //Sam 20220902 : 增加 Retry 機制
{
    int iCt=0;

    for(int i=0; i<FTestSuck.iShtRow; i++)
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            if(CosFunction.bD44Once4Suck && iType==1 && iCt>=4)                 //Sam 20220920 : D44 Index 回黏檢查一次最多吸四顆
                continue;

            if(bIndexSuck[0][i][j]==true)
            {
                iCt++;
                if(FTestSuck.Suck[i][j].Suck())
                {
                    bIndexSuck[0][i][j]=false;
                }

                if(FTestSuck.Suck[i][j].Error==true)
                {
                    FTestSuck.Suck[i][j].Error=false;
                    bIndexSuck[0][i][j]=false;                                  //Sam 20221007 : 修正index check error
                }
            }
            else if(bIndexDestroy[0][i][j]==true)                               //jou 2011-11-01 加個else避免真空破壞同時開啟
            {
                iCt++;
                if(FTestSuck.Suck[i][j].Destroy())
                {
                    bIndexDestroy[0][i][j]=false;
                }
            }
        }
    }

    if(iCt==0)
        return true;
    else
        return false;
}
// golden Tfiosetview::ResetIndexSuck（iosetview.cpp:1861-1869）逐行照翻（AI(W906-IDXSUCK) 20260927）
void TfiosetviewShim::ResetIndexSuck()
{
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
            for(int k=0; k<8; k++)
                bIndexSuck[i][j][k]=false;
    }
}
