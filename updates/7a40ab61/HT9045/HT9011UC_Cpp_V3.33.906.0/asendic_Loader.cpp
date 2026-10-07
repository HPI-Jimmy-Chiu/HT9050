// =============================================================================
//  asendic_Loader.cpp  --  Loader (incoming IC tray supply) stack-feed SM
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/asendic_Loader.cpp
//                   (3312 lines of code + 1 trailing separator = 3313 decoded
//                    lines; count re-derived from the cp950 decode in this pass)
//  Translation wave: W906-W7-L1 Wave 1 (Loader + Loader_RT bundle)
//  Translator: AI(W906-W7-L1-Loader) 20260802
//
//  FAITHFUL TRANSLATION
//  --------------------
//  Names, 1-based numeric Task states, switch(Task) semantics, the case
//  fall-throughs and every Sen[]/Cylinder[]/SW[]/MOT[] spelling are preserved
//  VERBATIM, same idiom as asendic_Empty.cpp (W6.1 canary) and
//  asendic_Auto2.cpp.  Each Do* owns its own extern cursor int (W6 strategy).
//  ALL 16 golden top-level functions are translated -- golden asendic_Loader.cpp
//  :62, :70, :150, :155, :1469, :1474, :1876, :2014, :2022, :2443, :2448, :3055,
//  :3060, :3167, :3233, :3256.  Nothing in this file is #if 0.
//
//  THIS FILE IS NOT A CLONE OF asendic_Empty.cpp.  A W7-L1 planner measured the
//  pairing: Loader vs Empty pair ZERO functions by name and the best-matching
//  function pair is only 12.1% identical.  It was translated from its own golden,
//  function by function.
//
//  GOLDEN FALL-THROUGHS PRESERVED (every `case N:` whose preceding statement is
//  not break/return, re-derived by scanning the decoded golden in this pass):
//    (1) DoSupplyNewICTray case 2250 -> case 2300 (golden :2321-2325) -- LIVE:
//        the `if(...Pop()) Task=2300; else break;` shape deliberately runs case
//        2300's body in the SAME tick when the cylinder reports done.
//    (2) DoLoadNewICTray  case 2250 -> case 2300 (golden :2398-2403) -- the same
//        shape, in the second copy of the OCR sub-flow.
//  There are NO other fall-throughs in this file.
//
//  INCLUDES (BCB6 original vs this tree)
//  -------------------------------------
//  BCB6 pulled: MachineDefine.h, asendic_Loader.h, csystem.h, asendic.h,
//  MyMotor.h, mycylin.h, main.h, mymessbox.h, note.h, mysensor.h, myswitch.h,
//  acatchtray.h, cprod.h, OCR.h, AGV.h, OCRInsp.h, cMyDB.h, cmydef.h,
//  uHGemHT9045.h, uLotInfo.h, adam6024.h, uTrayEditForm.h, aArmHeader.h,
//  SCK_ART.h, cTrayMapping.h, ainarm2.h, cContact.h, LoadCCDMap.h, cSortCT.h,
//  AMR.h.  Replaced by:
//    Motor/mymotor.h      (MOT[])                         -- W4 HAL
//    mycylin.h            (Cylinder[])                    -- W6.0 HAL gap
//    mysensor.h / myswitch.h (Sen[] / SW[])               -- W4-IO HAL
//    csystem.h            (ReadWriteTrayID + predicates)  -- frozen interface
//    asendic.h            (CylinderUp/Middle/Lower + TrayMove* + TrayMoveStatus)
//    acatchtray.h         (MTrayXCanSafeMove, InitialCoverTrayIDTask,
//                          WhichAutoNeedTray)
//    acatchtray_shims.h   (fTrayMapping, LoadCCD, MoveInArm2XYToWait,
//                          IsMoveInArm2XYToWait, CheckInArmFinishAllPickerAction,
//                          InitProcessSingleMotorTask)   -- offline stand-ins
//    atester_shims.h      (fContact, ADAM_DirectWriteData)
//    FormsFacade.h        (fMain / fSortCT / fSCKART / fAGV / fTrayForm / fOCR /
//                          fLotInfo / fProductionInfo)    -- non-VCL satellites
//    canary_support.h     (LastSet, __FUNC__, ShowErrorMessage, ShowMyMessage,
//                          RecordProcess)
//    SECSGEM/SecsEventType.h + SecsEventReport.h  (SECS_EVENT / EventReport)
//    Automation/AMR.h     (the real TTeraPowerAMR `AMR` global)
//    common.h             (MyTickCount / asDupBundleID)
//    cprod.h/cpublic.h/cmydef.h  (Prod/Tech/IniConfig/CosFunction/TrayForm/
//                          TestIF/TestIF_File/Ld_UldDelayTime/TrayID/enums)
//    asendic_Loader_RT.h  (InitUnLoadNewLoaderTrayTask / DoUnLoadNewLoaderToStack
//                          -- the RT half of this bundle; golden reaches them via
//                          asendic.h's god-include, which this tree's asendic.h
//                          subset does not reproduce)
//
//  SOFT_SIMULTE is NOT defined: the #ifndef SOFT_SIMULTE (real-machine) branches   ⚠ AI(W906-SENSORSCAN) 20260924: 本段（:67-69）已過期 —— MachineType.h:63-64 現在定義 SOFT_SIMULTE（未設 W906_NO_SOFT_SIMULTE 時），預設建置編的是 #ifdef SOFT_SIMULTE 臂（例：:3212-3218 讀 fMain->CheckBox1）；真機組態才編 #ifndef 臂
//  are compiled, the #ifdef SOFT_SIMULTE branches are not -- i.e. the REAL logic
//  is translated, driven via the Sim HAL.  Both arms are kept verbatim.
//
//  TU-LOCAL STAND-INS + `#define` REDIRECTS -- READ THIS BEFORE EDITING
//  --------------------------------------------------------------------
//  Eleven free functions this file calls have NO linkable home in the ported tree
//  (plus one data field, #12, and one global DEFINITION this file has to own --
//  both documented in the same block).
//  Rather than gate ten call sites (which would destroy the verbatim switch(Task)
//  bodies this translation exists to preserve), each is redirected to a
//  transparently-named TU-local stand-in with a `#define`, exactly the idiom
//  csystem.cpp already uses for RecordAutoCleanOutStartEnd
//  (`static void W7C1_RecordAutoCleanOutStartEnd(bool){}` + `#define`).  The
//  redirects live BELOW all #includes, so no header declaration is perturbed;
//  they are TU-local, so no duplicate symbol can ever be created; and each one
//  names the wave that retires it.  The full list, why each has no home, and
//  what its offline value COSTS in behaviour, is in the block right above the
//  `#define`s.  Do NOT convert any of them into a shim in a shared header:
//  acatchtray_shims.h's Wave-0 banner explicitly forbids that for the asendic.cpp
//  family, and the other files' owners are landing in parallel.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  ZERO U+FFFD.
// =============================================================================
#include "asendic_Loader.h"
#include "csystem.h"
//---------------------------------------------------------------------------
#include "asendic.h"
#include "asendic_Loader_RT.h"  // InitUnLoadNewLoaderTrayTask / DoUnLoadNewLoaderToStack
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "acatchtray.h"         // MTrayXCanSafeMove / InitialCoverTrayIDTask
#include "acatchtray_shims.h"   // fTrayMapping / LoadCCD / in-arm move helpers
#include "aHotPlateSubstrate.h" // bPlaceToHotplate / bPlaceShuttle (golden ainarm2.h:51/:53)
#include "atester_shims.h"      // fContact / ADAM_DirectWriteData (was adam6024.h)
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"
#include "common.h"             // MyTickCount / asDupBundleID
#include "FormsFacade.h"        // fMain / fSortCT / fSCKART / fAGV / fTrayForm /
                                // fOCR / fLotInfo / fProductionInfo
#include "canary_support.h"     // LastSet, __FUNC__, ShowErrorMessage, etc.
#include "SECSGEM/SecsEventType.h"      // SECS_EVENT      (was uHGemHT9045.h)
#include "SECSGEM/SecsEventReport.h"    // EventReport()   (was uHGemHT9045.h)
#include "Automation/AMR.h"             // AMR             (was AMR.h)
#include <cstdlib>                      // atoi (golden calls it at :2069-:2070)

// =============================================================================
//  TU-LOCAL STAND-INS FOR SYMBOLS WITH NO PORTED HOME  (see banner)
//  Each entry: golden home -> why there is nothing to link -> offline value ->
//  the wave that retires the stand-in + its `#define`.
//
//   1. GetColorSensorIsMapping(AnsiString&)   golden cprod.cpp:2972
//      DECLARED in the ported cprod.h:3301, but its BODY sits inside cprod.cpp's
//      `#if 0 // TODO(W6)` block (cprod.cpp:184-4036), so the symbol does not
//      exist at link time.  This is a LINK error, not a compile error -- the two
//      call sites (golden :1971, :2116) compile fine against the header.
//      cprod.cpp is not this agent's file and must not be un-gated.
//      OFFLINE VALUE: true.  That is golden's own simulation answer -- golden's
//      body ends with `#ifdef SOFT_SIMULTE bReturn=true; #endif` -- and it is
//      also the only defensible offline reading (no MU-N colour sensor exists,
//      so nothing can fail the colour map).  The out-param is left untouched.
//      COST: none observable, because both call sites are already dead offline
//      (fTrayForm->IsEnableColorSensor() is offline false and gates both).
//      RETIRED BY: the wave that un-gates cprod.cpp.  [AI(W906-FLOW-2) 20260928: RETIRED -- the real body cprod.cpp:3106 is live; its FT/RT lookup is still gated (cprod.cpp:3110 GA1-B2, uColorSensorInfo absent), so it still returns true]
//   2. PushLoaderTrayInAverageTime(DWORD)     golden asendic.h:22 / asendic.cpp:88
//   3. RecordAutoCleanOutStartEnd(bool)       golden asendic.h:23 / asendic.cpp:61
//      Both are golden asendic.cpp free functions.  acatchtray_shims.h's Wave-0
//      SHIM RETIREMENT DEBT REGISTER states explicitly that these land as REAL
//      translated bodies in Wave 3 and that "no shim for them exists here to
//      retire, and none should be added" -- so they are stood in TU-locally here
//      instead of being added to a shared header.
//      OFFLINE VALUE: no-op both.
//      COST, stated because it is a real behaviour delta and not nothing:
//        * (2) golden's body maintains a 5-deep ring buffer and writes the mean
//          into LastSet.iLoaderTraySimulateTime (golden asendic.cpp:107).  With
//          the no-op that field is never updated, so the DUMMY-run tray-feed
//          simulation time stays at whatever LastSet was loaded with.  The only
//          consumer in THIS file is golden :392
//          (htLoaderTimeDelay.SetMSAndOn(LastSet.iLoaderTraySimulateTime+1000)).
//        * (3) golden's body latches an auto-clean-out cycle timer and writes one
//          RecordProcess log line; it is gated on
//          IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain and has no SM-visible
//          side effect.  csystem.cpp already ships the identical no-op stand-in.
//      RETIRED BY: Wave 3 (asendic.cpp L1a).
//   4. CheckInArmAutoAlignmentTrayModeBeUse(int,bool)  golden AutoAlignment.h:265
//   5. InitDoInArmTeachAlignmentProcessTask()          golden AutoAlignment.h:224
//      The AutoAlignment subsystem is not translated.  acatchtray_shims.h already
//      carries the OUT-arm twins of exactly these two
//      (CheckOutArmAutoAlignmentTrayModeBeUse / InitDoOutArmTeachAlignmentProcess-
//      Task); the IN-arm pair was simply never needed before, so Wave 0 did not
//      land them.  Reported as a surface gap.
//      OFFLINE VALUE: false / no-op -- identical to the out-arm twins.
//      COST: golden :1158-1162's AOA-every-tray hook never arms
//      bLoaderNeedTrayMustFinish.  Already unreachable anyway: the enclosing
//      guard needs MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment.
//      RETIRED BY: the wave that lands AutoAlignment.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, neither name is defined anywhere in the tree]
//   6. InitOCRFlow(bool)   golden OCRInsp.h:4  / OCRInsp.cpp:369
//   7. CleanOCRData()      golden OCRInsp.h:11 / OCRInsp.cpp:1496
//      The OCR inspection subsystem is not translated (forms/fOCR.h is only the
//      ring-light facade, not OCRInsp).  OFFLINE VALUE: no-op both.
//      COST: the two OCR sub-flows (DoSupplyNewICTray case 2000-2300 and
//      DoLoadNewICTray case 1900-2300) still walk their cursors but never start
//      or clear a real OCR job.  Both are gated behind INSTALL_OCR!=eocrUninstal.
//      RETIRED BY: the wave that lands OCRInsp.  [AI(W906-FLOW-2) 20260928: RETIRED -- OCRInsp.cpp:777/:1929 landed (OCRInsp.h included at :277)]
//   8. MyDeCodeASCII(int)  golden EJ1N/TextProcess.h:9 / TextProcess.cpp:23
//      The whole EJ1N directory is absent from the ported tree.
//      OFFLINE VALUE: BEHAVIOURALLY IDENTICAL to golden, not an approximation.
//      Golden is a 129-case switch: 0..31 -> the standard control-code mnemonics,
//      32..126 -> the literal character, 127 -> "DEL", default -> "Err".  Cases
//      32..126 in golden are exactly the ASCII characters (verified case by case
//      against the decoded golden in this pass), so the table+cast form below
//      returns the same AnsiString for every int.  Kept short deliberately: the
//      body belongs to EJ1N, not to this file.
//      RETIRED BY: the wave that lands EJ1N/TextProcess.  [AI(W906-FLOW-2) 20260928: RETIRED -- EJ1N/TextProcess.cpp:155 landed (included at :281)]
//   9. EditTray(int,int)   golden uTrayEditForm.h:60 / uTrayEditForm.cpp:33
//      A VCL modal tray editor form; no ported home.  OFFLINE VALUE: no-op.
//      COST: golden :1243's TSMC manual-sort hand-editing step does nothing.
//      Gated behind IniConfig.bI27_ManualSortMode && bRunManualSortMode.
//      RETIRED BY: the wave that lands uTrayEditForm.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, no EditTray definition anywhere in the tree]
//  10. ShowMessage(AnsiString)  golden VCL Dialogs.hpp
//      The real VCL modal dialog.  Same situation, and the same answer, as
//      SECSGEM/SecsSvEcRegistration.cpp's `Gated_ShowMessage` precedent.
//      OFFLINE VALUE: no-op.  Only call site is the `default:` arm of
//      DoKnockBeforeRemainICCheck (golden :3307), i.e. the corrupt-cursor path.
//      RETIRED BY: W7-UI.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, no ShowMessage definition anywhere in the tree]
//  11. LogIndexMaxMinPos(AnsiString)  golden cpublic.cpp:1778
//      DECLARED in the ported cpublic.h:327, but its BODY sits inside
//      cpublic.cpp's `#if 0 // TODO(W6: ... needs fMain->slIndexYMaxMinShift +
//      iMax/MinCommandY* globals + InitialMaxMinValue)` block starting
//      cpublic.cpp:1776 -- the SAME shape as GetColorSensorIsMapping (#1): a LINK
//      error, not a compile error.  OFFLINE VALUE: no-op.
//      COST: golden's one call (golden :193, "Load a new tray") writes an
//      encoder-vs-command-position delta log line per tray.  Pure diagnostics; no
//      SM-visible effect.
//      RETIRED BY: the wave that un-gates cpublic.cpp's W6 block.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, cpublic.cpp:1816 #if 0 (GA1-B3) still closed: fMain->slIndexYMaxMinShift now exists (forms/fMain.h:1284) but fMain->AddIndexPosLog is defined nowhere]
//
//  ONE MORE, AND IT IS A DATA STAND-IN RATHER THAN A FUNCTION
//  ----------------------------------------------------------
//  12. Tech.iMLoaderYSurePos    golden LastSet.h:1027 (TECH), read at golden
//      asendic_Loader.cpp:548 and :606.
//      The ported tree DOES have a `Tech` object -- acarry_shims.h's
//      `TECH_AcarryShim Tech` -- but it is an explicitly MINIMAL mirror carrying
//      only the SIX shuttle detect-position fields the W6.5 carry engine needed;
//      iMLoaderYSurePos is not one of them.  acarry_shims.h is not this agent's
//      file, so the field is NOT added there.  A TU-local `W7L1L_Tech` object
//      supplies it instead, and this file deliberately does NOT include
//      acarry_shims.h, so the two names never meet in one translation unit.
//      OFFLINE VALUE: 0 (the same default TECH_AcarryShim gives its own fields).
//      COST: golden's two `TrayMoveIn(true, 0, Tech.iMLoaderYSurePos)` calls pass
//      0 instead of the teach position.  TrayMoveIn's body in this tree is an
//      empty Sim no-op (asendic.cpp), so today that costs nothing observable --
//      but it WILL matter the moment Wave 3 lands the real belt bodies, which is
//      why it is reported as a surface gap rather than left silent.
//      RETIRED BY: the wave that lands the real translated TECH (LastSet.h).  [AI(W906-FLOW-2) 20260928: RETIRED -- real TECH Tech LastSet.cpp:40 / LastSet.h:1158; TECH_AcarryShim was itself retired 20260804 (acarry_shims.h:61)]
// =============================================================================
// AI(W906-W7-L1-Loader) 20260802: golden ainarm2.h:219 `extern int
// iInArmWaitPosition;`.  ainarm2.h is not translated, and the ported tree
// declares this symbol in NO header (acatchtray_shims.h:415 records only that
// acatchtray.cpp OWNS the definition, acatchtray.cpp:135).  This is a
// DECLARATION of that existing external symbol -- not a new definition, not a
// stand-in -- so the two golden write sites (:1906, :2535, :2595) bind to the
// one real object.  Reported as a surface gap: it belongs in a shared header.
extern int iInArmWaitPosition;                                                  // golden ainarm2.h:219

// AI(W906-W7-L1-Loader) 20260802: `bool bWaitingAMR` -- golden cmydef.cpp
// (RogerYang 20250617).  The ported cmydef.h:5848 DECLARES it but the ported
// cmydef.cpp's definition (:5953) sits inside that file's `#if 0` block
// :5816-6025, so the symbol has NO definition anywhere in the tree.  This file
// is its ONLY consumer -- grepped the whole ported tree, and separately the whole
// cp950-decoded golden tree, where the only other users are aTester_Front.cpp and
// aTester_Rear.cpp (1 reference each, neither reachable in this build).  So the
// DEFINITION is owned here, following the exact precedent acatchtray.cpp:135 set
// for iInArmWaitPosition (a golden-cmydef global whose ported definition is
// gated, owned instead by the translated engine that uses it).  Initial value
// FALSE, verbatim from golden cmydef.cpp:5953.
// RETIREMENT DEBT: when cmydef.cpp is un-gated this becomes a duplicate symbol --
// delete THIS definition then, not the cmydef.cpp one.
bool bWaitingAMR=false;                                                         // golden cmydef.cpp:5953 //RogerYang 20250617

// AI(W906-W7-L1-Wave3) 20260802: the W7L1L_PushLoaderTrayInAverageTime and
// W7L1L_RecordAutoCleanOutStartEnd stand-ins (both empty no-ops) and their two
// `#define` redirects are DELETED -- Wave 3 landed golden asendic.cpp in full, so
// this file's golden call sites now reach the REAL bodies (golden
// asendic.cpp:88-109 and :61-84) through the `#include "asendic.h"` above.
// UNLIKE the sibling retirements in asendic_Auto*.cpp, this one IS a behaviour
// change and is meant to be: the stand-ins did nothing, whereas
// PushLoaderTrayInAverageTime now writes LastSet.iLoaderTraySimulateTime (only
// for 500 <= st <= 8000) and RecordAutoCleanOutStartEnd now emits a
// RecordProcess() line -- but only when IniConfig
// .bA08LastLoaderAutoCleanOutAndCheckAgain is set, which it is not by default,
// so the second one still early-returns in the default fixture.
//AI(W906-W7-L1-W3fixA) 20260802: CORRECTION to the paragraph above, which read as
// though retiring the stand-in HERE completed the wiring for (3).  It did not.
// RecordAutoCleanOutStartEnd is a MATCHED PAIR and those are its only two call
// sites in the whole golden tree: this file's :1763 passes true (START -- latches
// tAutoCleanTimer) and csystem.cpp:15653 passes false (END -- reads that timer and
// emits the elapsed-time line).  Wave 3 retired only the START stand-in; csystem.cpp
// kept `#define RecordAutoCleanOutStartEnd W7C1_RecordAutoCleanOutStartEnd`, so for
// one round the timer latched here and NOTHING ever read it.  The END stand-in is
// now retired too (csystem.cpp, same pass), so the pair is symmetric and the wiring
// really is complete.  Do not re-stub either half alone.
//AI(W906-FLOW-2) 20260928: RETIRED stand-in #1 -- the real GetColorSensorIsMapping is compiled (cprod.cpp:3106, decl cprod.h:3301; golden cprod.cpp:2938). Was: static bool W7L1L_GetColorSensorIsMapping(AnsiString & /*sErrorMsg*/)  { return true; }
static void W7L1L_LogIndexMaxMinPos(AnsiString /*str*/)                {}
static bool W7L1L_CheckInArmAutoAlignmentTrayModeBeUse(unsigned long /*iMode*/,
                                                       bool /*bSet*/ = false) { return false; }
static void W7L1L_InitDoInArmTeachAlignmentProcessTask()               {}
#include "OCRInsp.h"  //AI(W906-FLOW-2) 20260928: RETIRED stand-in #6 -- golden asendic_Loader.cpp:21 includes OCRInsp.h; real InitOCRFlow OCRInsp.cpp:777 (golden OCRInsp.cpp:369). Was: static void W7L1L_InitOCRFlow(bool /*bTrain*/)                         {}
//AI(W906-FLOW-2) 20260928: RETIRED stand-in #7 -- real CleanOCRData OCRInsp.cpp:1929 (golden OCRInsp.cpp:1496, decl OCRInsp.h:43). Was: static void W7L1L_CleanOCRData()                                       {}
static void W7L1L_EditTray(int /*MotorIndexIndex*/, int /*iHasMap*/ = 0) {}
static void W7L1L_ShowMessage(const AnsiString & /*S*/)                {}
#include "EJ1N/TextProcess.h"  //AI(W906-FLOW-2) 20260928: RETIRED stand-in #8 (:281-:294) -- real MyDeCodeASCII EJ1N/TextProcess.cpp:155 (decl TextProcess.h:106; golden EJ1N/TextProcess.cpp:23), same AnsiString for every int (checked case by case). Was: static AnsiString W7L1L_MyDeCodeASCII(int iInPut)
//{
//    static const char *kCtrl[32] =
//    {
//        "NUL","SOH","STX","ETX","EOT","ENQ","ACK","BEL",
//        "BS" ,"HT" ,"LF" ,"VT" ,"FF" ,"CR" ,"SO" ,"SI" ,
//        "DLE","DC1","DC2","DC3","DC4","NAK","SYN","ETB",
//        "CAN","EM" ,"SUB","ESC","FS" ,"GS" ,"RS" ,"US"
//    };
//    if(iInPut>=0   && iInPut<=31)  return AnsiString(kCtrl[iInPut]);             // golden :26-57
//    if(iInPut>=32  && iInPut<=126) return AnsiString((char)iInPut);              // golden :60-154
//    if(iInPut==127)                return AnsiString("DEL");                     // golden :155
//    return AnsiString("Err");                                                    // golden :156
//}
//AI(W906-FLOW-2) 20260928: RETIRED data stand-in #12 (:295-:300) -- `Tech` is the real TECH Tech again (LastSet.cpp:40, extern LastSet.h:1158 via canary_support.h:59), loaded from teach.ini by the TechPara row forms/fTeachRegistry.cpp:701. Was: struct W7L1L_TECH_LoaderShim
//{
//    int iMLoaderYSurePos;                       // golden LastSet.h:1027 (TECH)
//    W7L1L_TECH_LoaderShim() : iMLoaderYSurePos(0) {}
//};
//static W7L1L_TECH_LoaderShim W7L1L_Tech;
//AI(W906-FLOW-2) 20260928: RETIRED -- golden :548/:606 read the taught Tech.iMLoaderYSurePos again (same value SetTechDataToProd_AOI copies into Prod.iMLoaderYSurePos, cinitial.cpp:7062). Was: #define Tech                                W7L1L_Tech
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define GetColorSensorIsMapping             W7L1L_GetColorSensorIsMapping
#define LogIndexMaxMinPos                   W7L1L_LogIndexMaxMinPos
#define CheckInArmAutoAlignmentTrayModeBeUse W7L1L_CheckInArmAutoAlignmentTrayModeBeUse
#define InitDoInArmTeachAlignmentProcessTask W7L1L_InitDoInArmTeachAlignmentProcessTask
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define InitOCRFlow                         W7L1L_InitOCRFlow
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define CleanOCRData                        W7L1L_CleanOCRData
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define MyDeCodeASCII                       W7L1L_MyDeCodeASCII
#define EditTray                            W7L1L_EditTray
#define ShowMessage                         W7L1L_ShowMessage

TQPF_Timer DetectLoaderTime;
TQPF_Timer StackDelay;
TQPF_Timer htLoaderTimeDelay;
TQPF_Timer DoSupplyNewICTrayDelay;
TQPF_Timer htCylinLowerDelay;                                                   //JerryYang 20190811 Loader下降到位後增加逾時判斷
TQPF_Timer htDetectICFloatingDelay;                                             //Sam 20200316 : Loader Detect Tray
TQPF_Timer htDetectTrayRetryDelay;                                              //Sam 20200803 : Alarm retry timeout
TQPF_Timer LoadReadTrayID;                                                      //kevin 20210623 read tray id time out
TQPF_Timer CCDLoadTrayMap;                                                      //kevin 20220401 CCD 判斷連線超時
TQPF_Timer htLoadHasTrayDelay;                                                  //JerryYang 20210507 : sensor必須on 3秒以上才當作有tray
int iTrayZLoadTrayToWaitTask=1;
int iLoadNewICTrayTask=1;
int iCassetteLoadNewICTrayTask=1;                                               //Frank 20251217 add
int iInspectTrayColorOnLoaderTask=1;
bool bSupplyNewICTrayPause=false;
bool bLoadTrayCCDMapOK=false;                                                   //kevin 20220401 Get CCD Data
int iKnockTrayTask=1;
//==============================================================================
// HT9046CR TrayZ 去供料機構搬移一個新TRAY到軌道下方
//==============================================================================
//------------------------------------------------------------------------------
void InitCassetteLoadNewICTrayTask()                                            //Frank 20251217 add
{
    iCassetteLoadNewICTrayTask=1;
}
//==============================================================================
// HT9046CR TrayZ 去供料機構搬移一個新TRAY到軌道下方
//==============================================================================
//------------------------------------------------------------------------------
bool DoCassetteLoadNewICTray()                                                  //Frank 20251217 add
{
//    int ret=0;
    int &Task=iCassetteLoadNewICTrayTask;
    static bool bLoadHasDuplicateError=false;
    static bool bYCCWMove=false;
    static bool bYCWMove=false;
//    static bool bZPushMove=false;
    static int iPos=0;

    if(Sen[SnLoaderSureTray].IsOn() && Sen[SnLoaderPreDete].IsOn())             //Loader 完成進料
        return true;

    if(bBoatChangeCasset[0]==true)
        return false;

    switch(Task)
    {
        case 1:                                                                 //new start
            if(Sen[SnLoaderBoatActDetect].IsOn())
            {
                ShowErrorMessage("MES0921", 0, MMTrayZ);
                //不能移動
                return false;
            }
            bYCCWMove=false;
            bYCWMove=false;
            Task=10;
            break;
        case 10:                                                                //do CW/CCW back
            bYCWMove=MOT[MLoaderY].MotorMove(Prod.iLDCassetteFrontBack);
            bYCCWMove=MOT[MLoaderY_CCW].MotorMove(Prod.iLDCassetteRearBack);
            if(bYCWMove==true && bYCCWMove==true)
            {
                bYCWMove=false;
                bYCCWMove=false;
                Task=20;
            }
            break;
        case 20:                                                                //do cyn down
            if(Cylinder[C_LoaderCarrier].Pop())
            {
                TestIF_File.iCassetteLDNowLayer++;
                iPos=Prod.iLDCassetteZStart+((TestIF_File.iCassetteLDNowLayer-1)*UserDefForm[0].dCassetteZPitch);
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTrayZ].MotorMove(iPos))
            {
                TrayMoveIn(true, 0, 99999);
                Task=110;
            }
            break;
        case 110:
            bYCWMove=MOT[MLoaderY].MotorMove(Prod.iLDCassetteFront);
            if(bYCWMove==true)
            {
                bYCWMove=false;
                Task=120;
            }
            break;
        case 120:
            bYCWMove=MOT[MLoaderY].MotorMove(Prod.iLDCassetteFrontBack);
            if(bYCWMove==true)
            {
                Task=200;
            }
            break;
        case 200:
            MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
            bLoadHasDuplicateError=false;
            Task=1;
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
int  iSupplyNewIC_From_LoaderCar=1;
//------------------------------------------------------------------------------
void InitSupplyNewIC_From_LoaderCarTask()
{
    iSupplyNewIC_From_LoaderCar=1;
}
//---------------------------------------------------------------------------
bool DoSupplyNewICTray()                                                        //Supply IC from Loader car
{
    int &Task=iSupplyNewIC_From_LoaderCar;
    int ret, iSafePos=0;
#ifdef SOFT_SIMULTE
    int ct, x, y, i;
#endif
    bool flag;
    static bool bTrayDuplicateErr=false;                                        //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題
    static bool bYFix=false, bXFix=false;
    static int dwStartLoaderTray, dwEndLoaderTray, iTemp;                       //20111128  Dell
    static int iOCRTrayCount=0;
    static int iVibrateCount=0;                                                 //JerryYang 20191001 loader震動馬達
    static int iLoaderUpPressCount=0;
    AnsiString str="";
    bool bSOFT_SIMULTE=false;                                                   //Ztex 2023.07.28 Add bSOFT_SIMULTE
    #ifdef SOFT_SIMULTE
        bSOFT_SIMULTE=true;
    #endif
    MOT[MLoaderY].SetSpeed(100);

    if(Sen[SnLoaderTrackDetect].Enable)
    {
        if(CheckLoaderICFloating(0)==false)                                     //Sam 20240108 : 整合 Loader 跳料檢查。
            return false;
    }

    switch(Task)
    {
        case 1:
            if(fTrayMapping->IsRunTrayMapLaser())                               //JimmyChiu 20220219 : 殘料檢查 From Loader
            {
                fTrayMapping->DoSettingBeforeMotorMove(fTrayMapping->eAOIType_TrayMapLaser,true,fTrayMapping->asErrorLaserScan);
            }

            if(MOT[MMTrayY_Car].fHasTray==true &&                               //jou 2013-07-15 入Tray必須要先將汽缸放開
               MOT[MMTrayY].fHasTray==false)
            {
                LogIndexMaxMinPos("Load a new tray");                           //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值，記錄並存檔，一盤tray記錄一次
                if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    Cylinder[C_LoaderPushBack_Back].Off();
                    Cylinder[C_LoaderPushBack_Push].On();
                }
                else
                {
                    Cylinder[C_TrayY_Fixer].Off();                              //Open Fix Supply Try Fix Cylinder
                }
                Cylinder[C_LoaderEdgePush].Off();
                Cylinder[C_LoaderUpPress].Off();                                //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
                iVibrateCount=0;                                                //JerryYang 20191001 loader震動馬達

                if(USE_CKD_FCM_CleanAir)                                        //Ifor 20150710 :開啟 CKD 吹氣控制  //Ifor 20160308 換位置
                {
                    if(iEPControlValue==100)                                    //百分比
                    {
                        ADAM_DirectWriteData(4095, 1);
                    }
                    else
                    {
                        ADAM_DirectWriteData((iEPControlValue*41), 1);
                    }
                }

                if(SW[SwLoaderAirClean].Enable)
                {
                    SW[SwLoaderAirClean].On();
                }

                if(TRAY_ARM_MODE==eUnderCoveyor)
                    TrayCylinMoveOut(0);                                        //需要往front 到等待點

                if(Sen[SnDoubleLoadDetection].Enable==true &&                   //Steven 20240426 : 偵測loader疊盤
                   Sen[SnDoubleLoadDetection].IsOff()==false)
                {
                    Task=5;
                }
                else
                {
                    Task=50;
                }
            }
            else if(MOT[MMTrayY_Car].fHasTray==false &&
                    MOT[MMTrayY].fHasTray==false)                               //jou 2011-03-24 start : Auto Site Mapping
            {
                Task=1;
                return true;
            }
            break;
        case 5:                                                                 //Steven 20240426 : 偵測loader疊盤
            if(Sen[SnDoubleLoadDetection].IsOff()==false)
            {
                ShowMyMessage("Loader Car Double Tray(Detect Sensor On)!!! # Please Remove The Loader Car Top Tray!!");
                Task=5;
            }
            else
            {
                Task=50;
            }
            break;
        case 50:
            bPortIsBusy[ePortLoader]=true;                                      //RogerYang 20250611 避免收到STOP_LOAD_PORT時狀態被切成eLoadReadyToLoad氣缸還在動
            if(IniConfig.bA68_AutoLoadUnload==true)                             //RogerYang 20250617 加入Log，客戶表示需要知道萬一AMR上料時翻盤的原因
                RecordProcess("DoSupplyNewICTray(Start)");

            bYFix=false;
            bXFix=false;
            if(Sen[SnLoaderCarHasTray].IsOff()==false)
            {
                if(Cylinder[C_LoaderUpPress].Enable    ||
                   (IniConfig.bP05_LoaderCylinderPreOn &&                       //Steven 20220127 : 整合P05, 避免影響UPH
                   (Cylinder[C_TrayY_Fixer].Enable ||
                    Cylinder[C_LoaderEdgePush].Enable)))                        //KaiChen 20190312 :伸縮一次避免爆衝 //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                {
                    if(MTrayXCanSafeMove()==true ||                             //JerryYang 20200909 : tray arm上升才能壓tray
                       TRAY_ARM_MODE==eUnderCoveyor)
                    {
                        if(Cylinder[C_LoaderUpPress].Enable)
                            Cylinder[C_LoaderUpPress].On();
                        if(Cylinder[C_TrayY_Fixer].Enable)                      //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                            Cylinder[C_TrayY_Fixer].On();
                        if(Cylinder[C_LoaderEdgePush].Enable)                   //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                            Cylinder[C_LoaderEdgePush].On();
                        DoSupplyNewICTrayDelay.SetSecAndOn(2);                  //Kenhsieh 20210917 ： 延長壓Tray時間(0.5->2)，防止未壓導致暴衝
                        Task=70;
                    }
                }
                else if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    if(Cylinder[C_LoaderEdgePush].Enable)
                    {
                        Cylinder[C_LoaderEdgePush].On();
                        DoSupplyNewICTrayDelay.SetSecAndOn(0.5);
                        Task=70;
                    }
                    else
                    {
                        Task=100;
                    }
                }
                else
                {
                    Task=100;
                }
            }
            else
            {
                if(LastSet.iRealDummy==DUMMY)
                    Task=110;
                else
                    Task=100;
            }
            break;
        case 70:                                                                //KaiChen 20190312 :伸縮一次避免爆衝
            if(DoSupplyNewICTrayDelay.Off())
            {
                Cylinder[C_LoaderUpPress].Off();
                Cylinder[C_TrayY_Fixer].Off();                                  //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                Cylinder[C_LoaderEdgePush].Off();                               //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                DoSupplyNewICTrayDelay.SetSecAndOn(1);
                iLoaderUpPressCount++;
                Task=75;
            }
            break;
        case 75:                                                                //KaiChen 20190312 :伸縮一次避免爆衝
            if(DoSupplyNewICTrayDelay.Off())
            {
                if(iLoaderUpPressCount<2)
                {
                    if(Cylinder[C_LoaderUpPress].Enable)
                        Cylinder[C_LoaderUpPress].On();
                    if(Cylinder[C_TrayY_Fixer].Enable)                          //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                        Cylinder[C_TrayY_Fixer].On();
                    if(Cylinder[C_LoaderEdgePush].Enable)                       //Kenhsieh 20211007 ： 汽缸預動作防止暴衝
                        Cylinder[C_LoaderEdgePush].On();
                    DoSupplyNewICTrayDelay.SetSecAndOn(2);                      //Kenhsieh 20210917 ： 延長壓Tray時間(0.5->2)，防止未壓導致暴衝
                    Task=70;
                }
                else
                {
                    iLoaderUpPressCount=0;
                    Task=100;
                }
            }
            break;
        case 100:                                                               //Wenqi 20240516 Modify Loader Process ==>
            if(INSTALL_OCR_YMot==eocrYMotInstal)
            {
                if(Cylinder[C_TrayY_Fixer].Enable)
                    Cylinder[C_TrayY_Fixer].On();
                if(Cylinder[C_LoaderEdgePush].Enable)
                    Cylinder[C_LoaderEdgePush].On();
            }

            if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                if(Cylinder[C_LoaderSeparate].Pop())
                {
                    Task=105;
                }
            }
            else
            {
                Task=105;
            }
            break;
        case 105:
            if(fTrayMapping->IsRunTrayMapLaser())
            {
                if(fTrayMapping->DoSettingBeforeMotorMove(fTrayMapping->eAOIType_TrayMapLaser,false,fTrayMapping->asErrorLaserScan))
                {
                    if(fTrayMapping->asErrorLaserScan!="")
                    {
                        ShowMyMessage("[DoLoad]"+fTrayMapping->asErrorLaserScan,"");
                        fTrayMapping->DoSettingBeforeMotorMove(fTrayMapping->eAOIType_TrayMapLaser,true,fTrayMapping->asErrorLaserScan);
                        Task=100;
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            bSupplyNewICTrayPause=false;

            if(TRAY_ARM_MODE==eAboveCoveyor)
            {
                if(MOT[MMTrayY_Car].fCanMove==false ||
                   MOT[MTrayX].ReadPos()<Prod.iXTrayLoad+1000)
                    break;
            }

            dwStartLoaderTray=MyTickCount();
            if(LastSet.iRealDummy==DUMMY)
            {
                htLoaderTimeDelay.SetMSAndOn(LastSet.iLoaderTraySimulateTime+1000);
            }

            if(INSTALL_OCR!=eocrUninstal &&
               CosFunction.bTrayOCR &&                                          //wei 20150925 待機位置改道 Color
               INSTALL_OCR_YMot!=eocrYMotInstal)
            {
                TrayMoveIn(true, 0, 99999);
            }
            else if(USE_LdUldCassetteMode==1)                                   //Frank 20251217 add
            {
            }
            else
            {
                TrayMoveIn(true, 0, Prod.iMLoaderYSurePos);
            }

            DetectLoaderTime.SetSecAndOn(20);

            if(USE_LdUldCassetteMode==1)                                        //Frank 20251217 add
            {
                InitCassetteLoadNewICTrayTask();
                Task=130;
            }

            if(INSTALL_OCR_YMot==eocrYMotInstal)                                //Frank 20250214 add
            {
                Task=140;
            }
            else
            {
                Task=150;
            }
            break;
        case 110:
            if(MOT[MMTrayY_Car].fCanMove==false)
                break;

            if(USE_LdUldCassetteMode==1)                                        //Frank 20251217 add
            {
                Task=130;
                break;
            }
            else
            {
                TrayMoveIn(true, 0, Prod.iMLoaderYSurePos);
            }
            DetectLoaderTime.SetSecAndOn(2);
            Task=160;
            break;
        case 130:
            if(DoCassetteLoadNewICTray())                                       //RogerYang 20260205 : Add for 9046CR
            {
                if(LastSet.iRealDummy==DUMMY)
                    Task=160;
                else
                    Task=150;
            }
            break;
        case 140:                                                               //Frank 20250214 add
            if(bSupplyNewICTrayPause)
            {
                bSupplyNewICTrayPause=false;
                DetectLoaderTime.SetSecAndOn(20);
            }

            if(MOT[MLoaderY].MotorMove(Prod.iMLoaderYSurePos))
            {
                Task=150;
            }
            break;
        case 150:
            if(bSupplyNewICTrayPause)
            {
                bSupplyNewICTrayPause=false;
                DetectLoaderTime.SetSecAndOn(20);
            }

            if(INSTALL_OCR_YMot==eocrYMotInstal)
            {
                if(Cylinder[C_TrayY_Fixer].Enable)
                    Cylinder[C_TrayY_Fixer].On();
                if(Cylinder[C_LoaderEdgePush].Enable)
                    Cylinder[C_LoaderEdgePush].On();
            }

            if(TrayMoveStatus(0,__FUNC__)==0)                                                                           //Sam 20240108 : 整合 Loader 跳料檢查。
                DetectLoaderTime.SetSecAndOn(20);

            if(INSTALL_OCR!=eocrUninstal &&
               CosFunction.bTrayOCR      &&
               INSTALL_OCR_YMot==eocrYMotUninstal)                              //KenHsieh 20260513 : 區分皮帶及台車
                TrayMoveIn(true, 0, 99999);
            else
                TrayMoveIn(true, 0, Prod.iMLoaderYSurePos);

            if(DOUBLE_BELT_MODE==1)                                                                                     //Ztex 2023.12.13 Add Double Belt Mode
            {
                if(LastSet.iRealDummy==DUMMY ||
                   bSOFT_SIMULTE==true ||
                   Sen[SnLoaderSureTray].IsOff()==false)
                {
                    DetectLoaderTime.SetSecAndOn(1);
                    Task=250;
                    break;
                }
                else
                {
                    TrayMoveIn(true, 0, 99999);
                }
            }
            else if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                if(Sen[SnLoaderSeparateHasTray].IsOff()==false ||
                   LastSet.iRealDummy==DUMMY)
                {
                    Cylinder[C_TrayY_Fixer].On();                                                                       //後推上升
                    DetectLoaderTime.SetSecAndOn(1);
                    Task=250;
                    break;
                }
                else
                {
                    TrayMoveIn(true, 0, 99999);
                }
            }
            else
            {
                if(Sen[SnLoaderSureTray].IsOff()==false ||                                                              //滑到定位
                   LastSet.iRealDummy==DUMMY)                                                                           //kevin 20210803 add dummy
                {
                    if(dwStartLoaderTray!=-1)
                    {
                        if(LastSet.iRealDummy!=DUMMY)
                        {
                            dwEndLoaderTray=MyTickCount();
                            PushLoaderTrayInAverageTime(dwEndLoaderTray-dwStartLoaderTray);
                        }
                        else
                        {
                            if(htLoaderTimeDelay.Off()==false)
                                break;
                        }
                    }

                    bTrayDuplicateErr=false;                                                                            //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題
                    Task=300;
                    break;
                }
                else
                {
                    if(INSTALL_OCR!=eocrUninstal &&
                       CosFunction.bTrayOCR      &&
                       INSTALL_OCR_YMot==eocrYMotUninstal)                      //KenHsieh 20260513 : 區分皮帶及台車
                        TrayMoveIn(true, 0, 99999);
                    else
                        TrayMoveIn(true, 0, Tech.iMLoaderYSurePos);
                }
            }

            if(DetectLoaderTime.Off())
            {
                dwStartLoaderTray=-1;
                if(LastSet.iRealDummy==REALLY &&
                   ((bForKyecBu3RunART==false && Sen[SnLoaderTrayHasTray].IsOff()==false) ||                            //wei 20161118 bRunART-->bForKyecBu3RunART
                    (bForKyecBu3RunART==true  && Sen[SnLoaderTrayHasTray_ART].IsOff()==false)))                         //wei 20160113 ART增加一顆sensor
                    ret=ShowErrorMessage("JAM0901", K_RETRY, MMTrayY, bTrayDuplicateErr, __FUNC__);                     //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題
                else
                    ret=ShowErrorMessage("JAM0901", K_RETRY|K_SKIP, MMTrayY, bTrayDuplicateErr, __FUNC__);              //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題

                if(ret==K_SKIP)
                {
                    MOT[MMTrayY_Car].ClearTray(__FUNC__);
                    TrayMoveIn(false, 0);
                    if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)                                               //wei 20150925 待機位置改道 Color
                    {
                        if(INSTALL_OCR_YMot==eocrYMotUninstal)                                                          //Frank 20250214 add
                            MOT[MLoaderY].PCIL132_SetPos(0);
                        else
                            InitProcessSingleMotorTask(MLoaderY);
                    }

                    if(USE_CKD_FCM_CleanAir)                                                                            //jou 20170209 (Steven) : 修正Load new tray 延遲問題
                    {
                        ADAM_DirectWriteData(0, 1);                                                                     //Ifor 20150710 :關閉CKD 吹氣流量
                    }

                    if(SW[SwLoaderAirClean].Enable)
                    {
                        SW[SwLoaderAirClean].Off();
                    }

                    Task=1;
                    bTrayDuplicateErr=false;                                                                            //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題
                    bLoaderHasSkip=false;                                                                               //Steven 20170313 (wei) : fixed for initial parameter
                    bNeedManualRemoveTray=false;                                                                        //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                    return true;
                }
                else if(ret==K_RETRY)
                {
                    bTrayDuplicateErr=true;                                                                             //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題
                    Task=100;
                }
            }
            break;
        case 160:
            if(INSTALL_OCR_YMot==eocrYMotInstal)
            {
                if(Cylinder[C_TrayY_Fixer].Enable)
                    Cylinder[C_TrayY_Fixer].On();
                if(Cylinder[C_LoaderEdgePush].Enable)
                    Cylinder[C_LoaderEdgePush].On();
            }

            TrayMoveIn(true, 0, Tech.iMLoaderYSurePos);

            if(DetectLoaderTime.Off())
            {
                TrayMoveIn(false, 0);
                if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)           //wei 20150925 待機位置改道 Color
                {
                    if(INSTALL_OCR_YMot==eocrYMotUninstal)                      //Frank 20250214 add
                        MOT[MLoaderY].PCIL132_SetPos(0);
                }

                if(USE_CKD_FCM_CleanAir)                                        //jou 20170209 (Steven) : 修正Load new tray 延遲問題
                {
                    ADAM_DirectWriteData(0, 1);                                 //Ifor 20150710 :關閉CKD 吹氣流量
                }

                if(SW[SwLoaderAirClean].Enable)
                {
                    SW[SwLoaderAirClean].Off();
                }
                Task=300;
            }
            break;
        case 250:
            if(DOUBLE_BELT_MODE==1)                                             //Ztex 2023.12.13 Add Double Belt Mode
            {
                Cylinder[C_TrayY_Fixer].On();
                if(DetectLoaderTime.Off()==false)
                    break;
            }
            else if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                TrayCylinMoveIn(0);
                if(DetectLoaderTime.Off()==false)
                    break;
            }

            if(Sen[SnLoaderSureTray].IsOff()==false ||
               (Sen[SnLoaderSeparateHasTray].IsOff()==false && TRAY_ARM_MODE==eUnderCoveyor) ||
               LastSet.iRealDummy==DUMMY)
            {
                if(dwStartLoaderTray!=-1)
                {
                    if(LastSet.iRealDummy!=DUMMY)
                    {
                        dwEndLoaderTray=MyTickCount();
                        PushLoaderTrayInAverageTime(dwEndLoaderTray-dwStartLoaderTray);
                    }
                    else
                    {
                        if(htLoaderTimeDelay.Off()==false)
                            break;
                    }
                }

                bTrayDuplicateErr=false;                                        //jou 2014-04-21 修正Loader tray alarm重複記數jam rate的問題
                Task=300;
                break;
            }
            break;
        case 300:
            if(USE_LdUldCassetteMode==1)                                        //Frank 20251217 add
            {
                Task=310;
            }
            else
            {
                StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);
                Task=350;
            }
            break;
        case 310:                                                               //Frank 20251217 add
            if(MOT[MLoaderY].MotorMove(Prod.iLDCassetteFrontBack))
            {
                StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);
                Task=350;
            }
            break;
        case 350:
            if(StackDelay.Off())
            {
                if(Cylinder[C_LoaderEdgePush].Enable)
                {
                    Cylinder[C_LoaderEdgePush].On();                            //jou 2011-02-23 先打出去0.15sec
                    DoSupplyNewICTrayDelay.SetSecAndOn(0.15);
                    Task=360;
                }
                else
                {
                    Task=1000;
                }
            }
            break;
        case 360:
            if(DoSupplyNewICTrayDelay.Off())
            {
                Cylinder[C_LoaderEdgePush].Off();                               //jou 2011-02-23 縮回來0.15sec
                DoSupplyNewICTrayDelay.SetSecAndOn(0.15);
                Task=370;
            }
            break;
        case 370:
            if(DoSupplyNewICTrayDelay.Off())
            {
                Cylinder[C_LoaderEdgePush].On();                                //jou 2011-02-23 正式打出去
                DoSupplyNewICTrayDelay.SetSecAndOn(0.2);                        //Ztex 2024.01.26 Add Change Loader Feed Tray Action
                Task=400;
            }
            break;
        case 400:
            if(DoSupplyNewICTrayDelay.Off())
                Task=1000;
            break;
        case 1000:
            if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                if(Cylinder[C_TrayY_Fixer].Push())                              //Ztex 2024.01.26 Add Change Loader Feed Tray Action
                {
                    Task=1100;
                }
            }
            else
            {
                if(Cylinder[C_TrayY_Fixer].Push())
                {
                    StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_FixTrayDely);
                    if(Cylinder[C_LoaderUpPress].Enable==true)                  //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
                    {
                        Task=1050;
                    }
                    else
                    {
                        Task=1100;
                    }
                }
            }
            break;
        case 1050:
            if(StackDelay.Off())
            {
                if(Cylinder[C_LoaderUpPress].Push())
                {
                    StackDelay.SetSecAndOn(1);
                    if(USE_LOADER_VIBRATION==true &&
                       Ld_UldDelayTime.LD_EnableVibrate)                        //JerryYang 20191001 loader震動馬達
                    {
                        Task=1060;
                    }
                    else
                    {
                        Task=1100;
                    }
                }
            }
            break;
//JerryYang 20191001 loader震動馬達
//==>
        case 1060:                                                              //Loader震動馬達On
            if(StackDelay.Off())
            {
                SW[SwLoaderVibration].On();
                StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_VibrateOnDelay*0.1);
                Task=1065;
            }
            break;
        case 1065:                                                              //Loader震動馬達Off
            if(StackDelay.Off())
            {
                iVibrateCount++;
                SW[SwLoaderVibration].Off();
                StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_VibrateOffDelay*0.1);
                Task=1070;
            }
            break;
        case 1070:
            if(StackDelay.Off())
            {
                if(iVibrateCount>=Ld_UldDelayTime.LD_VibrateLoopCnt)
                {
                    Task=1100;
                    iVibrateCount=0;
                }
                else
                {
                    Task=1060;
                }
            }
            break;
//<==
//JerryYang 20191001 loader震動馬達
        case 1100:
            if(StackDelay.Off())
            {
                flag=CheckInArmFinishAllPickerAction();                         //Steven 20171226 (Wei) : 修改in arm讓開的flag

                Cylinder[C_LoaderEdgePush].On();
                TrayMoveIn(false, 0);
                if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)           //wei 20150925 待機位置改道 Color
                {
                    if(INSTALL_OCR_YMot==eocrYMotUninstal)                      //Frank 20250214 add
                        MOT[MLoaderY].PCIL132_SetPos(0);
                }

                if(USE_CKD_FCM_CleanAir)                                        //jou 20170209  (Steven) : 修正Load new tray 延遲問題
                {
                    ADAM_DirectWriteData(0, 1);                                 //Ifor 20150710 :關閉CKD 吹氣流量
                }

                if(SW[SwLoaderAirClean].Enable)
                {
                    SW[SwLoaderAirClean].Off();
                }

                if(INSTALL_OCR!=eocrUninstal &&
                   TestIF.bOcrFunction==true &&
                   IniConfig.iOCRConditions==0 &&
                   CosFunction.bTrayOCR==false)                                 //wei 20161121 卡掉之前的OCR系統
                {
                    if(bDoOCRFunction==false)
                        iOCRTrayCount++;

                    if(iOCRTrayCount>=IniConfig.iOCRByNewTrayIntrvalTray)
                        bDoOCRFunction=true;
                }

                if(fAGV->IsSPIL_AMR())                                          //JerryYang 20250521 : For AMR
                {
                    InitialCoverTrayIDTask(iKeyenceCoverTrayID_Loader);

                    if(iLoadTrayCount>fSCKART->iBundleInCnt)                    //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        iLoadTrayCount=0;
                        if(IniConfig.bEnable_SECS_GEM==true)                    //JerryYang 20240318 : add
                            EventReport(SECS_EVENT.BundleCompleteProcessed);
                    }

                    if(fSortCT->pnlLoadCID->Caption=="")
                        Task=6000;
                }
                else if(INSTALL_OCR!=eocrUninstal &&
                        TestIF.bOcrFunction==true &&
                        bDoOCRFunction &&
                        CosFunction.bTrayOCR==false)                            //Steven 20120716 : OCR
                {
                    if(flag==true)
                    {
                        iOCRTrayCount=0;
                        bDoOCRFunction=false;
                        MOT[MMOCR].SetTray(HAS_IC, __FUNC__);
                        bRunOcrInsp=true;
                        Task=2000;
                    }
                    else
                    {
                        return false;
                    }
                }
                else if(USE_TRAY_MAPPING==etmInstall &&                         //wei 20161219 Tray Mapping
                        (TestIF_File.bEnableTrayMap==true ||
                         TestIF_File.bEnableTrayID==true) &&
                         W906_FormShowing("fContact", fContact->fShow)==false)  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
                {
                    Task=3000;
                }
                else
                {
                    Task=1200;
                }
            }
            break;
        case 1200:
            if(TrayForm.bChkLoadDirection &&                                    //Steven 20190815 : JCET不重測Tray偵測
               LastSet.iRealDummy!=DUMMY &&
               Sen[SnCheckLoadDirection].Enable &&                              //JerryYang 20190902 改用SnCheckLoadDirection
               Sen[SnCheckLoadDirection].IsOn())
            {
                Task=1400;
                break;
            }
            else
            {
                if(IniConfig.b17bUseLoadCCDTrayMap)                             //kevin 20220823 change by config add 呼叫eKeep 拍照
                {
                    Task=1290;
                }
                else if(fTrayMapping->IsRunTrayMapLaser())                      //JimmyChiu 20220219 : 殘料檢查 From Loader
                {
                    fTrayMapping->DoGetValueAfterMotorArrival(fTrayMapping->eAOIType_TrayMapLaser,true,fTrayMapping->asErrorLaserScan);
                    Task=1240;
                }
                else
                {
                    Task=1300;
                }
            }
            break;
        case 1240:
            if(fTrayMapping->IsRunTrayMapLaser())                               //JimmyChiu 20220219 : 殘料檢查 From Loader
            {
                if(fTrayMapping->DoGetValueAfterMotorArrival(fTrayMapping->eAOIType_TrayMapLaser,false,fTrayMapping->asErrorLaserScan))
                {
                    Task=1245;
                }
            }
                else
            {
                Task=1300;
                break;
            }
            break;
        case 1245:                                                              //JimmyChiu 20220219 : 殘料檢查 From Loader
            if(fTrayMapping->IsRunTrayMapLaser()==false)
            {
                    Task=1300;
                break;
            }
            else if(fTrayMapping->GetTrayMapHasNullIC())
            {
                RecordProcess("WAR0890 Tray has null ic.");
//                ret=ShowErrorMessage("WAR0890", K_SKIP, MInArmX, 0, __FUNC__);  //RogerYang 20251030 : 需要驗證 暫時沒開放alarm
//                if(ret==K_SKIP)
                {
                    MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);                     //Steven 20130909
//                    for(int iRow=0; iRow<=MOT[MMTrayY].Tray.YItem-1; iRow++)
//                    {
//                        for(int iCol=0; iCol<=MOT[MMTrayY].Tray.XItem-1; iCol++)
//                        {
//                            if(fTrayMapping->mtTrayMap->GetCellData(iRow,iCol)==0)
//                                MOT[MMTrayY].SetTraySingleData(iCol, iRow, NULL_IC);
//                        }
//                    }
                }
//                else
//                {
//                    Task=1248;
//                    break;
//                }
            }
            else
            {
                MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
            }
            Task=1300;
            break;
        case 1248:                                                              //JimmyChiu 20220219 : 殘料檢查 From Loader    remove tray
            MOT[MMTrayY].ClearTray(__FUNC__);
            MOT[MMTrayY_Car].ClearTray(__FUNC__);
            Task=1400;
            break;
        case 1250:                                                              //kevin 20220401 loadTRAY MAP
            ret=LoadCCD->LoadCCDData();
            if(ret!=999)
            {
                bLoadTrayCCDMapOK=true;                                         //kevin 20220401 Get CCD Data
                Task=1300;
            }
            else if(CCDLoadTrayMap.Off())                                       //LOAD  TRAY 沒有IC
            {
                ShowErrorMessage("WAR09103", K_RETRY, MMTrayY);
                Task=1300;
            }
            break;
        case 1290:                                                                                                      //kevin 20220823 check Load arrival
            flag=false;
            if(Sen[SnLoaderCarHasTray].Enable)
            {
                flag=Sen[SnLoaderSureTray].IsOn();
                if(flag ||
                    LastSet.iRealDummy==DUMMY)                                                                          //kevin 20221026 add DUMMY  run
                {
                    Task=1291;
                }
                else
                {
                    if(IniConfig.bA04LoaderTraySplitFailCanSkip==true)
                        ret=ShowErrorMessage("JAM0929", K_RETRY|K_SKIP, MMTrayY, false, "DoSupplyNewICTray_1290");      //Steven 20120925 : JAM0909 -> JAM0929
                    else
                        ret=ShowErrorMessage("JAM0929", K_RETRY, MMTrayY, false, "DoSupplyNewICTray_1290");             //Steven 20120925 : JAM0909 -> JAM0929
                }
            }
            else
            {
                Task=1300;
            }
            break;
        case 1291:
            if(IniConfig.b17bUseLoadCCDTrayMap)                                 //kevin 20220823 change by config add 呼叫eKeep 拍照
                ShowErrorMessage("WAR09106", K_RETRY, MMTrayY);
            Task=1300;
            break;
        case 1300:
            if(INSTALL_OCR!=eocrUninstal && TestIF.bOcrFunction==true)
            {
                MOT[MMTrayY].fHasTray=MOT[MMOCR].fHasTray;
                if(MOT[MMTrayY].fHasTray)
                {
                    if(CosFunction.bTrayOCR)                                    //wei 20150924
                    {
                        //wei 20161118 強制使用Loader Tray資料
                        //==>
                        for(int iOCRX=0; iOCRX<=MOT[MMTrayY].Tray.XItem-1; iOCRX++)
                        {
                            for(int iOCRY=0; iOCRY<=MOT[MMTrayY].Tray.YItem-1; iOCRY++)
                            {
                                if(IniConfig.bEnabledOCRCheckIC)                //wei 20161228 確認Tray是否有IC
                                {
                                    if(MOT[MMOCR].Tray.Data[iOCRX][iOCRY]==HAS_OCR_OK)
                                        MOT[MMTrayY].SetTraySingleData(iOCRX, iOCRY, HAS_IC);
                                    else
                                        MOT[MMTrayY].SetTraySingleData(iOCRX, iOCRY, NULL_IC);
                                }
                                else
                                {
                                    MOT[MMTrayY].SetTraySingleData(iOCRX, iOCRY, HAS_IC);
                                }
                            }
                        }
                        //<==
                        //wei 20161118 強制使用Loader Tray資料
                        MOT[MMTrayY].Refresh();
                    }
                    else
                    {
                        if(IniConfig.bI37_EnableFIFOMode &&
                           IniConfig.bI37_EnableFIFOSiteOrder &&
                           LastSet.iRunStartMode==rsmFIFOMode)                  //Steven 20170302 (wei) : FIFO MODE
                        {
                            MOT[MMTrayY].InitNewTray(HAS_IC, true, __FUNC__);
                        }
                        else
                        {
                            MOT[MMTrayY].InitNewTray(HAS_IC, false, __FUNC__);
                        }
                    }
                }

                MOT[MMOCR].ClearTray(__FUNC__);
                MOT[MMTrayY].fHasTray=true;
                if(IniConfig.bEnable_SECS_GEM==true)                            //wei 20150525 : Secs Gem  add Load Tray Finish
                    EventReport(SECS_EVENT.LoadTrayFinish);
            }
            else if(USE_TRAY_MAPPING==etmInstall &&
                    TestIF_File.bEnableTrayMap==true &&
                    W906_FormShowing("fContact", fContact->fShow)==false)                                     //wei 20161219 Tray Mapping  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
            {
                if(TestIF_File.bDisableMapSuck==false)
                {
                    for(int j=0; j<=MOT[MMTrayY].Tray.YItem-1; j++)
                    {
                        for(int i=0; i<=MOT[MMTrayY].Tray.XItem-1; i++)
                        {
                            if(MyDeCodeASCII(fTrayMapping->iTrayMappingDate[j][i])==1)
                                MOT[MMTrayY].SetTraySingleData(i, j, HAS_IC);
                            else
                                MOT[MMTrayY].SetTraySingleData(i, j, NULL_IC);
                        }
                    }
                }
                else
                {
                    MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
                }
                MOT[MMTrayY].Refresh();
                MOT[MMTrayY].fHasTray=true;
                if(IniConfig.bEnable_SECS_GEM==true)                            //wei 20150525 : Secs Gem  add Load Tray Finish
                    EventReport(SECS_EVENT.LoadTrayFinish);
            }
            else
            {
                if(IniConfig.bI37_EnableFIFOMode &&
                   IniConfig.bI37_EnableFIFOSiteOrder &&
                   LastSet.iRunStartMode==rsmFIFOMode)                          //Steven 20170302 (wei) : FIFO MODE
                {
                    MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
                    MOT[MMTrayY].InitNewTray(HAS_IC, true, __FUNC__);
                }
                else if(fAGV->IsATK_AMR())                                      //Steven 20251210 : ATK AMR
                {
                    if(MOT[MMTrayY_Car].iIsCoverTray==IS_ID_TRAY ||
                       MOT[MMTrayY_Car].iIsCoverTray==NULL_IC)
                    {
                        MOT[MMTrayY].iIsCoverTray=MOT[MMTrayY_Car].iIsCoverTray;
                        MOT[MMTrayY].sTrayID=MOT[MMTrayY_Car].sTrayID;
                        MOT[MMTrayY].SetTray(NULL_IC, __FUNC__);
                    }
                    else
                    {
                        MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
                    }
                }
                else if(fAGV->IsSPIL_AMR())                                     //JerryYang 20250521 : For AMR
                {
                    if((asBundleTrayID[ePortLoader]!="ERROR" && asBundleTrayID[ePortLoader]!="Error") && asBundleTrayID[ePortLoader]!="")
                    {
                        MOT[MMTrayY].iIsCoverTray=IS_ID_TRAY;
                        MOT[MMTrayY].sTrayID=MOT[MMTrayY_Car].sTrayID;
                        MOT[MMTrayY].SetTray(NULL_IC, __FUNC__);
                    }
                    else
                    {
                        MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
                    }
                }
                //==> Eastsun 20260513 F011 整合:KYEC AMR Loader cover check
                else if(TrayForm.bEnableAMR)
                {
                    if(iLoaderTrayCountCal>=(TestIF_File.iAMRLDNowTrayCount-iAMRCoverTray))
                    {
                        MOT[MMTrayY].SetTray(NULL_IC, __FUNC__);
                        str.sprintf("[%d,%d,%d]", iLoaderTrayCountCal, TestIF_File.iAMRLDNowTrayCount, iAMRCoverTray);
                        RecordProcess(str, "AMR LoaderTray NULL_IC");
                    }
                    else
                    {
                        MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
                    }
                }
                //<== Eastsun 20260513 F011
                else if(fTrayMapping->IsRunTrayMapLaser())                      //JimmyChiu 20220219 : 殘料檢查 From Loader
                {
                    //pass
                }
                else
                {
                    MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
                }

                if(IniConfig.bEnable_SECS_GEM==true)                            //wei 20150525 : Secs Gem  add Load Tray Finish
                    EventReport(SECS_EVENT.LoadTrayFinish);
            }

            if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&                               //kevin 20150601 不是高雄TRAY需留一空排空
               CosFunction.bUseSCKART==false   &&                               //Steven 20161201 : For SCK 93K ART
               CUSTOMER_CODE!=CC_KYEC_LEE)                                      //Eastsun 20260512 F009 整合:Ifor 20210416 add: KYEC ART 不面庠留最後一行
            {
                if(USE_AUTO_RETEST==eartInstall &&
                   (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&
                   LastSet.bLoaderTrayCount_ART &&
                   LastSet.iAutoRetestCount_ART>=1 &&
                   MOT[MMTrayY].fHasTray)                                       //ChungHung 20141002 add for KYEC AutoRetest
                {
                    for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                        MOT[MMTrayY].SetTraySingleData(i, MOT[MMTrayY].Tray.YItem-1, NULL_IC);
                }
            }
            bFirstRecordLoaderData=true;                                        //jou 2013-05-31 first record loader data

            if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&                                //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
               TestIF.bEnableAutoAlignment==true &&
               (LastSet.iRealDummy==HAS_TRAY ||
                LastSet.iRealDummy==REALLY))
            {
                if(CheckInArmAutoAlignmentTrayModeBeUse(AutoAlignmentTray_EveryTray))
                {
                    InitDoInArmTeachAlignmentProcessTask();
                    bLoaderNeedTrayMustFinish=true;
                }
            }

            #ifdef SOFT_SIMULTE
            if(iCleanOut)
            {
                ct=random(MOT[MMTrayY].Tray.YItem);
                for(y=ct; y<MOT[MMTrayY].Tray.YItem; y++)
                {
                    for(x=0; x<MOT[MMTrayY].Tray.XItem; x++)
                        MOT[MMTrayY].SetTraySingleData(x, y, NULL_IC);
                }
                y=ct-1;
                if(y>=0)
                {
                    ct=random(MOT[MMTrayY].Tray.XItem);
                    for(i=0; i<ct; i++)
                    {
                        MOT[MMTrayY].SetTraySingleData(random(MOT[MMTrayY].Tray.XItem), y, NULL_IC);
                    }
                }
            }
            #endif

            CheckContinManualRemoveTray();                                      //Sam 20230203 : 修正P24放Tray異常

            if(CosFunction.bUseSCKART)                                          //Steven 20170320 (wei) : every new tray should clear skip count
            {
                bLoaderHasSkip=false;
                bNeedManualRemoveTray=false;                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                iContinueAutoSkipAutoTrayEnd=0;
            }
            else if(USE_AUTO_RETEST==eartInstall &&
                    (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&
                    LastSet.bLoaderTrayCount_ART==true)                         //ChungHung 20141002 add for KYEC AutoRetest
            {
                if(MOT[MMTrayY].fHasTray)
                {
                    //==> Eastsun 20260512 F009 整合: KYEC AMR 模式 Loader Tray 計數包鹹
                    if(TrayForm.bEnableAMR)
                    {
                        if(iLoaderTrayCountCal+iAMRCoverTray<TestIF_File.iAMRLDSECSTrayCount)
                        {
                            if(CUSTOMER_CODE==CC_KYEC_LEE && LastSet.iAutoRetestCount_ART>=1)
                            {
                                bRTLoaderTrayCount=true;                                       //wei 20160112 ART R1不算空盤
                            }
                            else
                            {
                                bRTLoaderTrayCount=false;                                      //wei 20160112 ART R1不算空盤
                                LastSet.iLoaderTrayCount_ART++;
                            }
                        }
                    }
                    //<== Eastsun 20260512 F009 整合
                    else
                    {
                        if(CUSTOMER_CODE==CC_KYEC_LEE &&
                           LastSet.iAutoRetestCount_ART>=1)
                        {
                            bRTLoaderTrayCount=true;                                //wei 20160112 ART R1不算空盤
                        }
                        else
                        {
                            bRTLoaderTrayCount=false;                               //wei 20160112 ART R1不算空盤
                            LastSet.iLoaderTrayCount_ART++;
                        }
                        fMain->lblLoadTrayCnt->Caption=LastSet.iLoaderTrayCount_ART;
                        if(CosFunction.bContinueAutoSkipAutoTrayEnd)                //ChungHung 20141014 add for KYEC AutoRetest
                            iContinueAutoSkipAutoTrayEnd=0;                         //ChungHung 20141002 add for KYEC AutoRetest
                    }
                }
            }

            bLoaderHasSuck=false;
            MOT[MMTrayY_Car].ClearTray(__FUNC__);
            bRecordUPH=true;

            if(IniConfig.bI27_ManualSortMode && bRunManualSortMode==true)       //Steven 20150915 : For TSMC 手動整盤功能
            {
                fMain->Pause("DoSupplyNewICTray");
                EditTray(MMTrayY, 2);
            }
            bPickUpHomeFinish=false;
            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.UPHRecordStart);                         //53 UPH Record Start

            if(CosFunction.bShowHPICCount)                                      //Steven 20221228 : 計算加熱盤IC數量
            {
                fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
            }

            if(CosFunction.bOneByOneWhenPickErrAtLoader &&                      //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
               bNeedOneByOnePickInArm)
            {
                ArmSpeed_File[InArm].bVariModeFIX=IniConfig.bBackUpInArmMode;
                ArmSpeed[InArm].bVariModeFIX=IniConfig.bBackUpInArmMode;
                bNeedOneByOnePickInArm=false;
            }
            Task=1;
            return true;
        case 1400:                                                              //Steven 20190815 : JCET不重測Tray偵測
            if(Sen[SnLoaderSureTray].IsOff()==true ||
               Sen[SnLoaderPreDete].IsOff()==true)
            {
                InitSupplyNewIC_From_LoaderCarTask();
                InitLoadTask();
            }
            else
            {
                Cylinder[C_TrayY_Fixer].Off();
                Cylinder[C_LoaderEdgePush].Off();
                Cylinder[C_LoaderUpPress].Off();
                ShowErrorMessage("WAR0952", K_RETRY, MMTrayY);
            }
            break;
        case 2000:                                                              //Steven 20120716 : OCR
            InitOCRFlow(false);                                                 //ChungHung 20120830 add OCR Function add
            Task=2100;
            break;
        case 2100:
            if(bRunOcrInsp==false)
            {
                if(Cylinder[C_OCRLight_Up].OnSensor()==false)
                {
                    Task=2200;
                }
                else
                {
                    bOCRRunTime=true;
                    Task=2300;
                }
            }
            else if(TestIF.bOcrFunction==false &&
                    bRunOcrInsp==true)                                          //ChungHung 20130118 add OCR - 可以隨時關閉
            {
                Cylinder[C_OCRLight_Up].Off();
                if(IniConfig.OCRLightChange)
                {
                    fOCR->ChangeLightValue(1, 0);
                    fOCR->ChangeLightValue(2, 0);
                }
                MOT[MMOCR].ClearTray(__FUNC__);
                InitOCRFlow(false);
                Task=2200;
            }
            break;
        case 2200:
            if(Cylinder[C_OCRLight_Up].OnSensor())
            {
                bRunOcrInsp=false;
                Task=2100;
            }
            else
            {
                Cylinder[C_OCRLight_Up].Off();
            }
            break;
        case 2250:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=2300;
            else
                break;
        case 2300:
            if(MTrayXCanSafeMove()==false)                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=2250;
                break;
            }

            if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)               //wei 20150925 待機位置改道 Color
            {
                iSafePos=Prod.iXTrayColor;
            }
            else
            {
                iSafePos=Prod.iXTrayEmpty;
            }

            if(MOT[MTrayX].MotorMove(iSafePos))
            {
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                Task=1200;
            }
            break;
        case 3000:                                                              //wei 20161219 Tray Mapping
            if(IsMoveInArm2XYToWait())
            {
                bLoaderTrayAction=true;
                Task=4000;
            }
            else
            {
                Task=3100;
            }
            break;
        case 3100:
            if(bPlaceToHotplate==true || bPlaceShuttle==true)                   //JerryYang 20221127 : Fix inarm流程與tray map同時下移動指令
            {
                break;
            }

            if(MoveInArm2XYToWait())
            {
                Task=3000;
            }
            break;
        case 4000:                                                              //wei 20161219 Tray Mapping
            if(bLoaderTrayAction==false)
            {
                Task=1200;
            }
            break;
        case 6000:                                                              //JerryYang 20240318
            if(fTrayMapping->DoCoverTrayID(iKeyenceCoverTrayID_Loader)==true)
            {
                if(asTrayIDDataLoader2=="ERROR" ||
                   asTrayIDDataLoader2=="Error" ||
                   asTrayIDDataLoader2=="NOREAD")
                {
                    asBundleTrayID[ePortLoader]="ERROR";
                    if(iLoadTrayCount>=fSCKART->iBundleInCnt)                   //第13盤必須是蓋子，需讀得到Cover ID，否則要ALARM  //JerryYang 20250220 : 2DID硬體順序檢查功能
                    {
                        str.sprintf("Loader Tray第%d盤讀不到Cover ID.", fSCKART->iBundleInCnt);
                        ShowMyMessage(str);
                        break;
                    }
                    else if(bMustCoverIDTray==true)                             //上一盤吸嘴檢查是空TRAY，這一盤應該要是COVER TRAY
                    {
                        ShowMyMessage("Tray ID read error, please check this tray must be COVER TRAY");
                        InitialCoverTrayIDTask(iKeyenceCoverTrayID_Loader);     //RETRY + SKIP
                        break;
                    }
                    else if(fMain->ALed1->Value==false &&
                            MOT[MMTrayY_Car].fHasTray==false &&
                            LastSet.iSCKARTInputCT==fSCKART->iInputCount)
                    {
                        ShowMyMessage("Tray ID read error, please check this last tray must be COVER TRAY");
                        break;
                    }
                }
                else
                {
                    if(IniConfig.bA65_BundleIDList)
                    {
                        slBundlID->Clear();
                        slBundlID->CommaText=fSCKART->sBundleList;
                        slBundlID->Sort();

                        if(slBundlID->Find(asTrayIDDataLoader2, iTemp)==true)   //Cover ID存在於List中
                        {
                            iTemp=0;
                            asBundleTrayID[ePortLoader]=asTrayIDDataLoader2;    //這盤是Cover Tray

                            slDupBundlID->Clear();
                            slDupBundlID->LoadFromFile(asDupBundleID);
                            slDupBundlID->Add(asTrayIDDataLoader2);
                            slDupBundlID->SaveToFile(asDupBundleID);

                            if(IniConfig.bEnable_SECS_GEM==true)
                            {
                                EventReport(SECS_EVENT.BundleCompleteIDRead);   //JerryYang 20240418
                            }
                        }
                        else
                        {
                            asBundleTrayID[ePortLoader]=asTrayIDDataLoader2;    //這盤是Cover Tray

                            if(IniConfig.bEnable_SECS_GEM==true)
                            {
                                EventReport(SECS_EVENT.UnexpectedBundleIDRead);
                            }

                            ShowErrorMessage("WAR0953", K_RETRY, MMTrayY);
                            fMain->BtnOneCycleClick(fMain);
                            break;
                        }
                    }
                    else
                    {
                        asBundleTrayID[ePortLoader]=asTrayIDDataLoader2;        //這盤是Cover Tray
                        if(IniConfig.bEnable_SECS_GEM==true)
                        {
                            EventReport(SECS_EVENT.BundleCompleteIDRead);       //PASS
                        }
                    }

                    iLoadTrayCount=0;
                }

                if(iLoadTrayCount>=fSCKART->iBundleInCnt)                       //10+1+1             //JerryYang 20250220 : 2DID硬體順序檢查功能
                {
                    iLoadTrayCount=0;
                }
                else
                {
                    Task=1200;
                }
            }
            break;
    }
    return false;
}
//==============================================================================
// 2000/05/17  TrayZ 去供料機構搬移一個新TRAY到等待點
//==============================================================================
void InitTrayZLoadTrayToWaitTask()
{
    iTrayZLoadTrayToWaitTask=1;
}
//------------------------------------------------------------------------------
bool DoTrayZLoadTrayToWait()
{
    int ret;
    bool bLoadHasTray=false;
    int &Task=iTrayZLoadTrayToWaitTask;
    AnsiString SData="";                                                        //kevin 20210623 read tray id time out
    static int iRead2D=0, iTest;                                                //kevin 20211116 Tray 2D read state

    switch(Task)
    {
        case 1:
            if(IniConfig.bG11ASEReport &&                                       //kevin 20210623 read tray id time out
               LastSet.iRealDummy!=DUMMY)                                       //kevin 20220117 run dummy
            {
                Task=2;
            }
            else if(fAGV->IsATK_AMR())                                          //Steven 20251210 : ATK AMR
            {
                if(fSortCT->pnlLoadCID->Caption=="")
                {
                    InitialCoverTrayIDTask(iKeyenceCoverTrayID_LoaderCar);
                    Task=10;
                }
                else
                {
                    LoadReadTrayID.SetSecAndOn(10);
                    Task=50;
                }
            }
            else if(fAGV->IsSPIL_AMR())                                         //JerryYang 20250521 : For AMR
            {
                InitialCoverTrayIDTask(iKeyenceCoverTrayID_LoaderCar);
                Task=10;
            }
            else
            {
                LoadReadTrayID.SetSecAndOn(10);                                 //kevin 20210730 add load 軌道是否有TRAY
                Task=50;
            }
            break;
        case 2:
            bReadTrayID=false;
            SData="@e02025";
            RespondASECom(SData);                                               //kevin 20150818 20150610 回應 ase Reset finish
            LoadReadTrayID.SetSecAndOn(5);                                      //kevin 20210623 read tray id time out
            RecordProcess("<AutoMove>Loader Query Tray ID");                    //kevin 20210623 add 讀取 tray id
            iRead2D=0;                                                          //kevin 20211116 Tray 2D read state
            Task=3;
            break;
        case 3:
            if(bReadTrayID)                                                                                             //kevin 20210623 TRAY ID Read
            {
                if(TrayID[0][0]=="")
                {
                    iRead2D=0;
                    Task=2;
                    break;
                }
                SData.sprintf("<AutoMove>Loader Get Tray ID,%s", TrayID[0][0]);                                         //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                ReadWriteTrayID(false);                                                                                 //kevin 20220705 record color tray id move color pick
                RecordProcess(SData);                                                                                   //kevin 20210623 add 讀取 tray id
                Task=50;
            }
            else if(LoadReadTrayID.Off())
            {
                if(iRead2D>=1)                                                                                          //kevin 20211116 send 2D 不經EKeep
                {
                    ret=ShowErrorMessage("WAR16336", K_RETRY, MMSystem);                                                //kevin 20220705 only Retry
                    if(ret==K_RETRY)
                    {
                        iRead2D=0;
                        Task=2;
                    }
                    else
                    {
                        TrayID[0][0]="NoTrayID";
                        SData.sprintf("<AutoMove>Loader Get Tray ID,NoTrayID");                                         //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                        RecordProcess(SData);                                                                           //kevin 20210623 add 讀取 tray id
                        Task=50;
                    }
                }
                else
                {
                    iRead2D++;
                    LoadReadTrayID.SetSecAndOn(5);                                                                      //kevin 20210623 read tray id time out
                }
            }
            break;
        case 10:
            if(fTrayMapping->DoCoverTrayID(iKeyenceCoverTrayID_LoaderCar)==true)
            {
                if(asTrayIDDataCorverLoader=="ERROR" ||
                   asTrayIDDataCorverLoader=="Error" ||
                   asTrayIDDataCorverLoader=="NOREAD")
                {
                    ShowMyMessage("Loader Car Cover tray ID error.");
                    InitialCoverTrayIDTask(iKeyenceCoverTrayID_LoaderCar);
                    break;
                }
                else if(CUSTOMER_CODE==CC_AMKOR_Korea)
                {
                    Task=50;
                }
                else
                {
                    if(IniConfig.bA65_BundleIDList)
                    {
                        slBundlID->Clear();
                        slBundlID->CommaText=fSCKART->sBundleList;
                        slBundlID->Sort();                                                                              //JerryYang 20240501 : 要加SORT不然找不到

                        if(slBundlID->Find(asTrayIDDataCorverLoader, iTest)==true)                                      //Cover ID存在於List中
                        {
                        }
                        else
                        {
                            if(IniConfig.bEnable_SECS_GEM==true)
                                EventReport(SECS_EVENT.UnexpectedBundleIDRead);
                            ShowMyMessage("Cover tray ID is not exist in the bundle ID list");
                            break;
                        }
                    }

                    if(asBundleTrayID[ePortEmpty]=="" ||
                       asBundleTrayID[ePortEmpty]!=asTrayIDDataCorverLoader)                                            //New lot  //New bundle
                    {
                        if(FileExists(asDupBundleID))                                                                   //Steven 20160505 : 加上保護, 不然開程式會跳Error
                        {
                            slDupBundlID->Clear();
                            slDupBundlID->LoadFromFile(asDupBundleID);
                            slDupBundlID->Sort();

                            if(slDupBundlID->Find(asTrayIDDataCorverLoader, iTest)==true)                               //找到重複上報的
                            {
                                if(IniConfig.bEnable_SECS_GEM==true)
                                    EventReport(SECS_EVENT.UnexpectedBundleIDRead);
                                ShowMyMessage("Cover tray ID is duplicate!");
                                break;
                            }
                            else
                            {
                                asBundleTrayID[ePortEmpty]=asTrayIDDataCorverLoader;                                    //JerryYang 202040318 : 這邊要發報Event report
                                if(IniConfig.bEnable_SECS_GEM==true)
                                    EventReport(SECS_EVENT.LoadPortBundleRead);
                            }
                        }
                        else
                        {
                            asBundleTrayID[ePortEmpty]=asTrayIDDataCorverLoader;                                        //JerryYang 202040318 : 這邊要發報Event report
                            if(IniConfig.bEnable_SECS_GEM==true)
                                EventReport(SECS_EVENT.LoadPortBundleRead);
                        }
                    }
                }
                Task=50;
            }
            break;
        case 50:
            if((Sen[SnLoaderCarHasTray].Enable &&
                Sen[SnLoaderCarHasTray].IsOff()) ||
               (Sen[SenLoaderCCWDete].Enable   &&
                Sen[SenLoaderCCWDete].IsOff()) ||
               LastSet.iRealDummy==DUMMY)
            {
                if(INSTALL_OCR_YMot==eocrYMotInstal)                            //Frank 20250214 add
                    Task=52;
                else
                    Task=55;                                                    //kevin 20210730 Load check tray
            }
            else if(LoadReadTrayID.Off())
            {
                #ifdef SOFT_SIMULTE
                    Task=55;
                #else
                ret=ShowErrorMessage("WAR16122", K_RETRY|K_SKIP, MMSystem);     //Steven 20211123 : WAR16119 --> WAR16122
                if(ret==K_SKIP)
                {
                    Task=55;
                    LoadReadTrayID.SetSecAndOn(10);                             //kevin 20210730 add load 軌道是否有TRAY
                }
                #endif
            }
            break;
        case 52:                                                                //Frank 20250214 add
            if(MOT[MMTrayY].fHasTray==false &&
                MTrayXCanSafeMove() &&
                MOT[MMTrayY_Car].fCanMove==true)                                //RogerYang 20251027 : Add
            {
                if(MOT[MLoaderY].ReadPos()!=Prod.iMLoaderYCarPos)
                {
                    Task=53;
                }
                else
                {
                    Task=55;
                }
            }
            break;
        case 53:
            if(MOT[MLoaderY].MotorMove(Prod.iMLoaderYCarPos))
            {
                Task=52;
            }
            break;
       case 55:
            if(fAGV->IsATK_AMR())                                               //Steven 20251210 : ATK AMR
            {
                if(CylinderUp(C_Load_Up))
                {
                    MOT[MMTrayZ].fHasTray=true;
                    Task=100;
                }
            }
            else if(Sen[SnLoaderTrayHasTray_ART].Enable)                        //Ifor 20191125 : add ART Loader 雙 Sensor mode
            {
                if(Sen[SnLoaderTrayHasTray_ART].IsOff() &&
                   Sen[SnLoaderTrayHasTray].IsOn())
                {
                    Task=1000;
                }
                else if(CylinderUp(C_Load_Up))
                {
                    MOT[MMTrayZ].fHasTray=true;
                    Task=100;
                }
            }
            else if(CylinderUp(C_Load_Up))
            {
                MOT[MMTrayZ].fHasTray=true;
                Task=100;
            }
            break;
        case 100:
            if(LastSet.iRealDummy==DUMMY)                                       //Steven 20100618 : Dummy Run時，不要放下Tray
                Task=200;
            else if(Cylinder[C_TrayZ_Selector].Push())
                Task=200;
            break;
        case 200:
            if(CylinderMiddle(C_Load_Up))
            {
                iLoadIsCoverTray=HAS_IC;
                if(LastSet.iRealDummy!=DUMMY &&
                   fAGV->IsATK_AMR())                                           //Steven 20251210 : ATK AMR
                {
                    if(Sen[SnLoaderTrayHasTray].IsOff() &&
                       Sen[SnLoaderTrayHasTray_ART].IsOff())
                    {
                        iLoadIsCoverTray=IS_ID_TRAY;
                    }
                    else if(Sen[SnLoaderTrayHasTray].IsOn() &&
                            Sen[SnLoaderTrayHasTray_ART].IsOff())
                    {
                        iLoadIsCoverTray=NULL_IC;
                    }
                    else
                    {
                        iLoadIsCoverTray=HAS_IC;
                    }
                }
                else if(LastSet.iRealDummy!=DUMMY &&
                        IniConfig.bLastLoaderNoInSide)                          //jou 2010-01-24 start : 如果Loader是最後一盤，不再load進去。
                {
                    if(bForKyecBu3RunART ||
                       Sen[SnLoaderTrayHasTray_ART].Enable)                     //wei 20160113 ART增加一顆sensor //wei 20161118 bRunART-->bForKyecBu3RunART
                        bLoadHasTray=Sen[SnLoaderTrayHasTray_ART].IsOff();
                    else
                        bLoadHasTray=Sen[SnLoaderTrayHasTray].IsOff();

                    if(bLoadHasTray==true)
                    {
                        if(IniConfig.bLastLoaderAutoCleanOut==false &&          //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                           IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain==false)
                        {
                            if(TrayForm.bAutoFeed==false)                       //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
                            {
                                bLoaderNoTrayAutoCleanOut=true;
                            }
                            else
                            {
                                bSecondTimeLoaderCheckNoTray=false;             //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                                bLoaderNoTrayAutoCleanOut=false;
                            }
                        }
                        else
                        {
                            if(IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain &&
                               bSecondTimeLoaderCheckNoTray==false)             //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                            {
                                RecordAutoCleanOutStartEnd(true);               //ChungHung 20130910 add for SCK
                            }

                            bLoaderNoTrayAutoCleanOut=true;

                            if(iCatchFromLoaderTask != 1)
                                { break; }               //AI(ht9045-v899) 20260409: interlock guard - wait for TrayArm DoCatchFromLoader to finish before clear-empty sequence
                        }
                        Task=1000;
                        break;
                    }
                }

                Task=300;
                bSecondTimeLoaderCheckNoTray=false;                             //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                bLoaderNoTrayAutoCleanOut=false;                                //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
            }
            break;
        case 300:
            if(Cylinder[C_TrayZ_Selector].Pop())
            {
                Task=400;
            }
            break;
        case 400:
            if(SW[SwACTrayY].Status()==false)
            {
                MOT[MMTrayZ].fHasTray=true;
                if(iLoadIsCoverTray==IS_ID_TRAY)
                {
                    MOT[MMTrayZ].sTrayID     =asTrayIDDataCorverLoader;
                }
                else
                {
                    MOT[MMTrayZ].sTrayID     ="";
                }

                MOT[MMTrayZ].iIsCoverTray=iLoadIsCoverTray;
                Task=1;
                return true;
            }
            break;
        case 1000:                                                              //jou 2010-01-24 start : 如果Loader是最後一盤，不再load進去。
            if(CylinderUp(C_Load_Up))
            {
                Task=1100;
            }
            break;
        case 1100:
            if(Cylinder[C_TrayZ_Selector].Pop())
            {
                Task=1200;
            }
            break;
        case 1200:
            if(CylinderLower(C_Load_Up))                                        //Steven 20240116 : CylinderMiddle --> CylinderLower
            {
                MOT[MMTrayZ].fHasTray=false;

                if(TrayForm.bAutoFeed==true &&                                  //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                   IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain==false)
                {
                    ret=ShowErrorMessage("MES0920", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, false, "DoTrayZLoadTrayToWait_1200-1");
                    if(ret==K_CLEAN_OUT)
                    {
                        fMain->CleanOut("DoTrayZLoadTrayToWait 1");
                        return true;
                    }
                    else
                    {
                        Task=1;
                    }
                }
                else
                {
                    if(TrayForm.bAutoFeed==false &&                             //Auto Tray Feed 沒有開啟
                       bLoaderNoTrayAutoCleanOut==true &&
                       MOT[MMTrayY_Car].fHasTray==false &&
                        MOT[MMTrayY].fHasTray==false ||
                       (bLoaderNoTrayAutoCleanOut==true &&
                       bSecondTimeLoaderCheckNoTray==true))                     //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                    {
                        ret=ShowErrorMessage("MES0920", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, false, "DoTrayZLoadTrayToWait_1200-2");
                        if(ret==K_CLEAN_OUT)
                        {
                            fMain->CleanOut("DoTrayZLoadTrayToWait 2");
                            return true;
                        }
                        else
                        {
                            Task=1;
                        }
                    }
                    else
                    {
                        if(TrayForm.bAutoFeed==true &&                          //Steven 20180821 : Fixed Auto Tray Feed 開啟時末盤會無限循環
                           bLoaderNoTrayAutoCleanOut==true &&
                           MOT[MMTrayY_Car].fHasTray==false &&
                            MOT[MMTrayY].fHasTray==false)
                        {
                            fMain->CleanOut("DoTrayZLoadTrayToWait 3");
                        }
                        Task=1;
                        return true;
                    }
                }
            }
            break;
//jou 2010-01-24 end
    }
    return false;
}
//==============================================================================
bool DoInspectTrayColorOnLoader(bool bIsFirst)
{
    if(fTrayForm->IsEnableColorSensor()==false  ||                              //Jimmychiu 20230630 : add color sensor MU-N in Loader
      (CosFunction.bOEEFunction==true &&
       (fProductionInfo->sLoadMO_TestFlow.SubString(1,2)!="RT")))               //Jimmychiu 20240506 : Run on RT mode
    {
        TrayMoveOut(false,0);
        return true;
    }
    int ret=0;
    if(bIsFirst==true)
    {
        iInspectTrayColorOnLoaderTask=1;
        return true;
    }
    else
    {
        int &Task=iInspectTrayColorOnLoaderTask;
        switch(Task)
        {
            case 1:                                                                                                     //Initial
                if(Sen[SnLoaderCarHasTray].IsOn() || LastSet.iRealDummy==DUMMY)
                {
                    if(MOT[MMTrayY_Car].fHasTray==true)
                    {
                        Task=200;
                    }
                    else                                                                                                //Please remove the loader tray
                    {
                        int ret=ShowErrorMessage("MES0922", K_RETRY|K_SKIP, MMTrayY_Car, false, __FUNC__);              //jou 2011-03-28 訊息秀錯"JAM0908"改"MES0922"
                        iInArmWaitPosition=0;                                                                           //Ifor 20210209 add:旗標未清除導致Tray Arm 不作動
                        if(ret==K_RETRY)
                        {
                            Task=1;
                        }
                        else
                        {
                            MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);                                                 //Steven 20130909
                            Task=200;
                        }
                    }
                }
                else
                {
                    InitLoadNewICTrayTask();
                    Task=100;
                }
                break;
            case 100:
                //==> Eastsun 20260818 : 9050 uses DoLoadNewICTray_9050   //AI(W906-F9050-BD) 20261004: H075, 910 asendic_Loader.cpp:1943-1945 verbatim; unreachable in V906 for every type while fTrayForm->IsEnableColorSensor() is false (forms/fTrayForm.cpp:12: this function returns at its top)
                if((MachineTypeChoice==Type_HT9050 ? DoLoadNewICTray_9050() : DoLoadNewICTray())==true)
                //<== Eastsun 20260818
                {
                    if(Sen[SnLoaderCarHasTray].IsOn() && MOT[MMTrayY_Car].fHasTray==true)
                    {
                        Task=200;
                    }
                    else
                    {
                        Task=9999;
                    }
                }
                break;
            case 200:                                                           //
                if(Sen[SnLoaderFixCyPush].IsOff()==true && MOT[MMTrayY].fHasTray==true)
                {
                }
                else
                {
                    TrayMoveOut(true, 0, Prod.iMLoaderYCarPos);
                    DetectLoaderTime.SetSecAndOn(IniConfig.iC17DelayTimes);
                    Task=201;
                }
                break;
            case 201:                                                           //
                #ifndef SOFT_SIMULTE

                if(Sen[SnLoaderFixCyPush].IsOff()==true && MOT[MMTrayY].fHasTray==true)
                {
                    Task=200;
                    break;
                }
                else if(Sen[SnCheckTrayDirection].IsOn()==false)
                {
                    Task=200;
                    break;
                }

                if(DetectLoaderTime.Off()==false)                               //not yet finish
                {
                    break;
                }

                TrayMoveOut(true, 0, Prod.iMLoaderYCarPos);
                #endif
                if(MOT[MMTrayY_Car].fHasTray==true || LastSet.iRealDummy==DUMMY)
                {
                    if(GetColorSensorIsMapping(fTrayForm->asErrorMsg)==false)   //Jimmychiu 20230630 : add color sensor MU-N in Loader
                    {
                        InitUnLoadNewLoaderTrayTask();
                        Task=300;
                    }
                    else
                    {
                        Task=9999;
                    }
                }
                else
                {
                    Task=9999;
                }
                break;
            case 300:                                                           //Failure
                if(DoUnLoadNewLoaderToStack(false)==true)
                {
                    if(IniConfig.bC17_1_SkipAlarm==true)
                        ret=ShowErrorMessage("JAM09200", K_RETRY|K_SKIP, MInArmX, 0, fTrayForm->asErrorMsg);
                    else
                        ret=ShowErrorMessage("JAM09200", K_RETRY, MInArmX, 0, fTrayForm->asErrorMsg);
                    if(ret==K_RETRY)
                    {
                        Task=1;
                    }
                    else
                    {
                        Task=9999;
                    }
                }
                break;
            case 9999:                                                          //finish
                Task=1;
                TrayMoveOut(false, 0);
                return true;
        }
    }
    return false;
}
//==============================================================================
//從塔Tray載入新Tray
//==============================================================================
void InitLoadNewICTrayTask()
{
    iLoadNewICTrayTask=1;
}
//==============================================================================
// 2000/05/17  TrayZ 去供料機構搬移一個新TRAY到軌道下方
//==============================================================================
//------------------------------------------------------------------------------
bool DoLoadNewICTray()
{
    int ret, iSafePos=0;
    int &Task=iLoadNewICTrayTask;
    static bool bLoadHasDuplicateError=false;                                   //Steven 20120220 : Load Tray重複Alarm Flag
    if(AMR.WaitAMRSupplyTray())                                                 //Sam 20240827 : 新增 AMR 功能
        return false;                                                           //通知 AMR 來補盤補盤，進盤流程要停止。

    switch(Task)
    {
        case 1:                                                                 //new start
            if(Sen[SnLoaderIsFull].IsOn())                                      //jou 2013-07-19 新增Loader Full Tray sesnor
            {
                ShowErrorMessage("MES0921", 0, MMTrayZ);
                return false;
            }

            if(Sen[SnLoaderCarHasTray].IsOn())
            {
                ret=ShowErrorMessage("JAM0909", K_RETRY, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_1");
                bLoadHasDuplicateError=true;
                return false;
            }
            InitTrayZLoadTrayToWaitTask();
            if(MOT[MMTrayY].fHasTray==false)
            {
                Cylinder[C_TrayY_Fixer].Off();                                  //Open Fix Supply Try Fix Cylinder
                Cylinder[C_LoaderEdgePush].Off();
                Cylinder[C_LoaderUpPress].Off();                                //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
            }

            if(USE_AUTO_RETEST==eartInstall &&                                  //ChungHung 20141002 add for KYEC AutoRetest
               IniConfig.bA10_AutoReTest &&
//               LastSet.bLoaderTrayCount_ART &&                                //Steven 20251216 : mark for ART
               (CUSTOMER_CODE!=CC_ASE_KaohSiung))                               //kevin 20150603
            {
                if(CosFunction.bUseSCKART)                                      //Steven 20161201 (wei) : For SCK 93K ART
                {
                }
                else if(LastSet.bCleanOut_ART==true)
                {
                    if(iCleanOut==0)
                        fMain->CleanOut("DoLoadNewICTray 1");
                    return true;
                }
                //==> Eastsun 20260512 F009 KYEC AMR Loader Tray overflow check (T3)
                else if(TrayForm.bEnableAMR &&
                        atoi(fLotInfo->LabDiffTrayCount->Caption.c_str())==0 &&
                        (atoi(fLotInfo->labLoaderTrayCount->Caption.c_str())-atoi(fLotInfo->labNowTrayCount->Caption.c_str()))==0)
                {
                    if(CUSTOMER_CODE==CC_KYEC_LEE)
                        ret=ShowErrorMessage("WAR0961", K_CLEAN_OUT, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_1");
                    else
                        ret=ShowErrorMessage("WAR0961", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_1");
                    if(ret==K_CLEAN_OUT)
                    {
                        if(CosFunction.bAutoRetestGPIBmode==false)
                        {
                            if(CUSTOMER_CODE!=CC_KYEC_LEE)
                                LastSet.iLoaderTrayCount_ART=0;
                            fMain->lblLoadTrayCnt->Caption=LastSet.iLoaderTrayCount_ART;
                            LastSet.bCleanOut_ART=true;
                            fMain->CleanOut("DoLoadNewICTray AMR");
                        }
                        return true;
                    }
                }
                //<== Eastsun 20260512 F009 KYEC AMR Loader Tray overflow check (T3)
                else if(LastSet.bLoaderTrayCount_ART &&                         //Steven 20251216 : for ART
                        LastSet.iLoaderTrayCount_ART>=LastSet.iLoaderTotalTray)
                {
                    //ShowAlarm "Loader Tray input will over Total Tray"
                    if(CUSTOMER_CODE==CC_KYEC_LEE)                              //wei 20150904 多盤只能CLEAN_OUT      //wei 20151207 只能RETRY
                        ret=ShowErrorMessage("WAR0961", K_CLEAN_OUT, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_1");
                    else
                        ret=ShowErrorMessage("WAR0961", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_1");

                    if(ret==K_CLEAN_OUT)                                        //Clea out -> rsmInitial_ART
                    {
                        if(CosFunction.bAutoRetestGPIBmode==false)              //jou 2015-10-02 Auto Retest GPIB mode
                        {
                            if(CUSTOMER_CODE!=CC_KYEC_LEE)                      //wei 20160127 不先清除
                                LastSet.iLoaderTrayCount_ART=0;
                            fMain->lblLoadTrayCnt->Caption=LastSet.iLoaderTrayCount_ART;
                            LastSet.bCleanOut_ART=true;
                            fMain->CleanOut("DoLoadNewICTray 2");
                        }
                        return true;
                    }
                }
            }

            if(fTrayForm->IsEnableColorSensor())                                //Jimmychiu 20230630 : add color sensor MU-N in Loader
            {
                if(GetColorSensorIsMapping(fTrayForm->asErrorMsg)==true)
                {
                    //pass
                }
                else
                {
                    ret=ShowErrorMessage("WAR09200", K_RETRY|K_SKIP, MInArmX, 0, fTrayForm->asErrorMsg);
                    if(ret==K_RETRY)
                    {
                        Task=1;
                        break;
                    }
                    else
                    {
                        //pass
                    }
                }
            }

            if(USE_LdUldCassetteMode==1)                                        //RogerYang 20260203 : Add for 9046CR dont do anything
                Task=500;
            else
                Task=100;
            break;
        case 100:
            if(DoTrayZLoadTrayToWait())
                Task=300;
            break;
        case 300:
            if(CylinderLower(C_Load_Up))
            {
                if(MOT[MMTrayZ].fHasTray==false)
                {
                    bLoadHasDuplicateError=false;                               //Steven 20120220 : Load Tray重複Alarm Flag
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung  &&
                       USE_AUTO_RETEST==eartInstall     &&
                       bAutoReTest_ART                  &&
                       (LastSet.iRunStartMode==rsmInitial_ART       ||
                        LastSet.iRunStartMode==rsmContinuStart_ART  ||
                        LastSet.iRunStartMode==rsmContinuRetest_ART))           //kevin 20150610
                    {
                        LastSet.bCleanOut_ART=true;                             //kevin 20170901 (Steven) 直接 CLARN OUT
                        fMain->CleanOut("DoLoadNewICTray 3");
                        return true;
                    }
                    return true;
                }
                htCylinLowerDelay.SetSecAndOn(2);                               //JerryYang 20190811 Loader下降到位後增加逾時判斷
                Task=400;
            }
            break;
        case 400:
            if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)                                                                                               //wei 20150925 待機位置改道 Color
            {
                #ifndef SOFT_SIMULTE
                if(Sen[SnLoaderSureTray].IsOff()==false)
                {
                    MOT[MMTrayZ].ClearTray(__FUNC__);
                    return false;
                }
                #endif
            }

            if(Sen[SnLoaderCarHasTray].IsOff()==false ||
               Sen[SnCheckTrayDirection].IsOff()==false ||
               Sen[SenLoaderCCWDete].IsOff()==false ||
               LastSet.iRealDummy==DUMMY)
            {
                if(USE_AUTO_RETEST==eartInstall &&                                                                                                              //ChungHung 20141002 add for KYEC AutoRetest
                   (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&
                   LastSet.bCleanOut_ART)
                {
                    if(MOT[MMTrayZ].fHasTray)
                    {
                        if(fAGV->IsATK_AMR())                                                                                                                   //Steven 20251210 : ATK AMR
                        {
                            if(MOT[MMTrayZ].iIsCoverTray==IS_ID_TRAY ||
                               MOT[MMTrayZ].iIsCoverTray==NULL_IC)
                            {
                                MOT[MMTrayY_Car].SetTray(NULL_IC, __FUNC__);
                            }
                            else
                            {
                                MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                            }
                        }
                        else
                        {
                            MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                        }
                        MOT[MMTrayY_Car].iIsCoverTray=MOT[MMTrayZ].iIsCoverTray;                                                                                //Steven 20251210 : ATK AMR
                        MOT[MMTrayY_Car].sTrayID=MOT[MMTrayZ].sTrayID;
                        MOT[MMTrayZ].ClearTray(__FUNC__);
                    }
                    Task=1;
                    return true;
                }
                else
                {
                    if(fAGV->IsATK_AMR() &&                                                                                                                     //Steven 20251210 : ATK AMR
                       MOT[MMTrayZ].iIsCoverTray==IS_ID_TRAY)
                    {
                        MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                        MOT[MMTrayY_Car].iIsCoverTray=MOT[MMTrayZ].iIsCoverTray;
                        MOT[MMTrayY_Car].sTrayID     =MOT[MMTrayZ].sTrayID;
                        MOT[MMTrayZ].ClearTray(__FUNC__);
                        AMR.LoaderInCount();                                                                                                                    //Sam 20240304 : 新增 AMR 功能
                        bLoadHasDuplicateError=false;                                                                                                           //Steven 20120220 : Load Tray重複Alarm Flag
                        Task=1;
                        return true;
                    }
                    else
                    {
                        bOCRStartPosition=false;
                        if(INSTALL_OCR_YMot!=eocrYMotInstal)                                                                                                    //Frank 20250214 add
                            MOT[MLoaderY].PCIL132_SetPos(0);

                        if(INSTALL_OCR!=eocrUninstal &&
                           TestIF.bOcrFunction==true &&
                           CosFunction.bTrayOCR)                                                                                                                //Steven 20120716 : OCR   && bDoOCRFunction
                        {
                            MOT[MMTrayZ].ClearTray(__FUNC__);
                            if(CosFunction.bTrayOCR)                                                                                                            //wei 20150924
                            {
                                //wei 20161118 強制使用Loader Tray資料
                                //==>
                                for(int iOCRX=0 ;iOCRX<=MOT[MMTrayY].Tray.XItem-1; iOCRX++)
                                {
                                    for(int iOCRY=0; iOCRY<=MOT[MMTrayY].Tray.YItem-1; iOCRY++)
                                    {
                                        MOT[MMOCR].SetTraySingleData(iOCRX, iOCRY, HAS_IC);
                                        MOT[MMOCR].fHasTray=true;
                                    }
                                }
                                //<==
                                //wei 20161118 強制使用Loader Tray資料
                                if(IniConfig.bP26_OCRCheckLot)                                                                                                  //wei 20151117 OCR Lot check
                                {
                                    iOCRLotTrayCount++;
                                    bOCRCheck=true;
                                    iOCRCheck=0;
                                }
                                MOT[MMOCR].Refresh();
                            }
                            else
                            {
                                MOT[MMOCR].SetTray(HAS_IC, __FUNC__);
                            }
                            Task=1900;
                        }
                        else
                        {
                            Task=500;
                        }
                    }
                }
            }
            else
            {
                if(htCylinLowerDelay.Off()==true)                                                                                                               //JerryYang 20190811 Loader下降到位後增加逾時判斷
                {
                    if(IniConfig.bA04LoaderTraySplitFailCanSkip==true)
                    {
                        ret=ShowErrorMessage("JAM0913", K_RETRY|K_SKIP, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_400");                            // 讓滾動　Time out 來 alarm //Steven 20120925 : JAM0909 -> JAM0913
                        bLoadHasDuplicateError=true;                                                                                                            //Steven 20120220 : Load Tray重複Alarm Flag
                        if(ret==K_SKIP)
                        {
                            bLoadHasDuplicateError=false;                                                                                                       //Steven 20120220 : Load Tray重複Alarm Flag
                            MOT[MMTrayZ].ClearTray(__FUNC__);
                            MOT[MMTrayY_Car].ClearTray(__FUNC__);
                            Task=1;
                            return true;
                        }
                    }
                    else
                    {
                        ret=ShowErrorMessage("JAM0913", K_RETRY, MMTrayY_Car, bLoadHasDuplicateError, "DoLoadNewICTray_400");                                   // 讓滾動　Time out 來 alarm //Steven 20120925 : JAM0909 -> JAM0901
                        bLoadHasDuplicateError=true;                                                                                                            //Steven 20120220 : Load Tray重複Alarm Flag
                    }
                }
                return false;
            }
            break;
        case 500:
            MOT[MMTrayY_Car].iIsCoverTray=MOT[MMTrayZ].iIsCoverTray;
            MOT[MMTrayY_Car].sTrayID     =MOT[MMTrayZ].sTrayID;

            if(fAGV->IsATK_AMR())                                               //Steven 20251210 : ATK AMR
            {
                if(MOT[MMTrayY_Car].iIsCoverTray==IS_ID_TRAY ||
                   MOT[MMTrayY_Car].iIsCoverTray==NULL_IC)
                {
                    MOT[MMTrayY_Car].SetTray(NULL_IC, __FUNC__);
                }
                else
                {
                    MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                }
            }
            else
            {
                MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
            }

            MOT[MMTrayZ].ClearTray(__FUNC__);
            AMR.LoaderInCount();                                                //Sam 20240304 : 新增 AMR 功能
            bLoadHasDuplicateError=false;                                       //Steven 20120220 : Load Tray重複Alarm Flag
            Task=1;
            return true;
        case 1900:
            if(INSTALL_OCR_YMot!=eocrYMotInstal)                                //Frank 20250214 add
            {
                if(MOT[MMTrayY].fHasTray==false &&
                   bCatchTrayFinishAction==false)                               //Frank 20160711 add
                {
                    Task=1910;                                                  //wei 20161121 確認Tray Arm在上面，才可以去讀取OCR
                }
            }
            else
            {
                Cylinder[C_TrayY_Fixer].On();
                Cylinder[C_LoaderEdgePush].On();
                htLoadHasTrayDelay.SetSecAndOn(1);
                Task=1901;
            }
            break;
        case 1901:
            if(MOT[MMTrayY].fHasTray==false &&
               bCatchTrayFinishAction==false)                                   //Frank 20160711 add
            {
                Task=1910;                                                      //wei 20161121 確認Tray Arm在上面，才可以去讀取OCR
            }
            break;
        case 1910:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //wei 20161121 確認Tray Arm在上面，才可以去讀取OCR
            {
                bRunOcrInsp=true;
                CleanOCRData();
                Task=2000;
            }
            break;
        case 2000:                                                              //Steven 20120716 : OCR
            InitOCRFlow(false);                                                 //ChungHung 20120830 add OCR Function add
            Task=2100;
            break;
        case 2100:
            if(bRunOcrInsp==false)
            {
                if(Cylinder[C_OCRLight_Up].OnSensor()==false)
                {
                    Task=2200;
                }
                else
                {
                    bOCRRunTime=true;
                    Task=2300;
                }
            }
            else if(TestIF.bOcrFunction==false && bRunOcrInsp==true)            //ChungHung 20130118 add OCR - 可以隨時關閉
            {
                Cylinder[C_OCRLight_Up].Off();
                if(IniConfig.OCRLightChange)
                {
                    fOCR->ChangeLightValue(1, 0);
                    fOCR->ChangeLightValue(2, 0);
                }
                MOT[MMOCR].ClearTray(__FUNC__);
                InitOCRFlow(false);
                Task=2200;
            }
            break;
        case 2200:
            if(Cylinder[C_OCRLight_Up].OnSensor())
            {
                bRunOcrInsp=false;
                Task=2100;
            }
            else
            {
                Cylinder[C_OCRLight_Up].Off();
            }
            break;
        case 2250:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=2300;
            else
                break;
        case 2300:
            if(MTrayXCanSafeMove()==false)                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=2250;
                break;
            }

            if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)               //wei 20150925 待機位置改道 Color
            {
                iSafePos=Prod.iXTrayColor;
            }
            else
            {
                iSafePos=Prod.iXTrayEmpty;
            }

            if(MOT[MTrayX].MotorMove(iSafePos))
            {
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                Task=500;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
//AI(W906-F9050-BD) 20261004: F9050-BD batch D (ledger H076) -- 910 asendic_Loader.cpp:2459-2739 (Eastsun 0819; task 400 axis = Frank 0930),
//  line by line: InitLoadNewICTrayTask_9050 / DoLoadNewICTray_9050 (task 1-400) and InitLoadTask_9050 / DoLoad_9050 (task
//  1-1125, incl. the layer probe 1000-1125 and the new alarm MES0924).  Called ONLY from the Type_HT9050 arms of DoLoad (top),
//  DoInspectTrayColorOnLoader (case 100), DoLoad case 900 (dead for 9050) and InitAllProcessTask => never called today.
//  Pending Frank/Jimmy: TrayZ_Up/Down are [MAX_TRACK]=9 but the probe walks 0..19 and iLoaderLayerCount_9050 starts at -1
//  (910 defect (a); fields in 910's order, so TrayZ_Up[9..17] reads TrayZ_Down[0..8] = 0 and TrayZ_Down[-1] reads TrayZ_Up[8], as in 910); TrayZ_Down / TrayZ_Home are never assigned (0), TrayZ_Up[0..8] = golden teach + 200 (cprod_9050.inc head).  Translated as written.
int iLoadNewICTrayTask_9050 = 1;
void InitLoadNewICTrayTask_9050()
{
    iLoadNewICTrayTask_9050 = 1;
    // 不重置層數 (跨 cycle 保留)
}
//------------------------------------------------------------------------------
int W906_TrayZ9050(int zone, int up, int layer); int W906_TrayZ9050Home(int zone); int W906_TrayZ9050ProbeStart(); int W906_TrayZ9050ProbeLimit();  bool DoLoadNewICTray_9050()   //AI(W906-TRAYZ-PITCH) 20261006: Loader Z = zone 0, base - layer*pitch (cinitial.cpp EOF); declarations on the same line
{
    int ret;
    int &Task = iLoadNewICTrayTask_9050;

    switch(Task)
    {
        case 1:
            if(iLoaderLayerCount_9050==0)
            {
                ret = ShowErrorMessage("MES0920", K_RETRY | K_CLEAN_OUT, MMTrayZ, false, "DoLoadNewICTray_9050_1"); if(ret == K_CLEAN_OUT) fMain->CleanOut("DoLoadNewICTray_9050_1"); iLoaderLayerCount_9050 = -1;   //RecordProcess("iLoaderLayerCount_9050==0");   //AI(W906-TRAYFLOW9050) 20261007: count 0 was a silent 1->800->900->1 loop; now MES0920 (no tray) and -1 -> DoLoad_9050 case 1000 re-probes
                return true;
            }
            Task = 100;
            break;
        case 100:
            if(Cylinder[C_LoaderEdgeClip].Pop() &&                              //邊緣定位開
               Cylinder[C_LoaderEdgePush].Pop())
            {
                if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 1, iLoaderLayerCount_9050)) == 1)   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Up[iLoaderLayerCount_9050]
                {
                    Task = 200;
                    iLoaderLayerCount_9050--;
                }
            }
            break;
        case 200:
            if(Cylinder[C_TrayZ_Selector].Push() &&                               //分離開
               Cylinder[C_LoaderEdgeClip].Pop() &&
               Cylinder[C_LoaderEdgePush].Pop())
            {
                if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 1, iLoaderLayerCount_9050)) == 1) //移動到下一層   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Up[..]
                {
                    if(Sen[SnLoaderTrayHasTray].IsOff())                        //要是沒有感測到有盤 需要叫異常
                    {
                        ret = ShowErrorMessage("MES0920", K_RETRY | K_CLEAN_OUT | K_SKIP,MMTrayZ, false, "DoLoadNewICTray_9050_200");
                        if(ret==K_SKIP)                                         //再往上一層
                        {
                            iLoaderLayerCount_9050--;
                        }
                        else if(ret==K_CLEAN_OUT)
                        {
                            Task = 250;
                        }
                    }
                    else
                    {
                        Task = 300;
                    }
                }
            }
            break;
        case 250:
            if(Cylinder[C_TrayZ_Selector].Pop())
            {
                if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 0, iLoaderLayerCount_9050)) == 1) //移動到目前層數待命位置   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Down[..]
                {
                    Task = 1;
                    return true;
                }
            }
            break;
        case 300:
            Cylinder[C_TrayZ_Selector].Off();                                   //分離關 (Off 回傳 void,不用 if)
            //MOT[MMTrayY].InitNewTray(HAS_IC, false, __FUNC__);
            MOT[MMTrayY].fHasTray = true;
            MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
            Task = 400;
            break;

        case 400:
            if(Cylinder[C_TrayZ_Selector].Pop())
            {
                if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 0, iLoaderLayerCount_9050)) == 1) //移動到目前層數待命位置   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Down[..]
                {
                    if(Sen[SnLoaderDrawerHasTray].IsOn())
                    {
                        if(IniConfig.bEnable_SECS_GEM == true)
                            EventReport(SECS_EVENT.LoadTrayFinish);             // 通知 host 上盤完成
                    }
                    Task = 1;
                    return true;
                }
            }
            break;
    }
    return false;
}

int  LoadTask_9050 = 1;  bool W906_LoaderProbeSeesTray();   //AI(W906-PROBESENSOR9050) 20261008: end of file
void InitLoadTask_9050()
{
    LoadTask_9050 = 1;
}
//------------------------------------------------------------------------------
void DoLoad_9050()
{
    int ret;
    int &Task = LoadTask_9050;
    static int iLoaderLayerCountDetect_9050=0;

    //Eastsun 20260819 : DoCatchFromLoader_9050 動作中 → DoLoad_9050 直接 return
    //  避免 MLoaderZ 同時被兩個 FSM 移動,以及 tray 交接期間分盤流程干擾
    if(iCatchFromLoader9050Task != 1)
        return;

    switch(Task)
    {
        case 1:
            if(iLoaderLayerCount_9050==-1 && MOT[MMTrayY].fHasTray == false)     //要人工補抽屜盤
            {
                Task = 1000;
            }
            else
            {
                if(MOT[MMTrayY].fHasTray == true)
                {
                    if(MOT[MLoaderZ].ReadPos() == W906_TrayZ9050(0, 0, iLoaderLayerCount_9050)) //下方待命位置   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Down[..] (910's exact compare kept)
                    {

                    }
                    else
                    {
                        Task = 600;                                             //移動到下方待命
                    }
                }
                else
                {
                    Task = 800;                                                 //沒盤補盤
                }
            }

            break;

        case 600:
//            if(MOT[MMTrayY].Tray.HasIC() == false)
//            {
//
//
//            }
            if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 0, iLoaderLayerCount_9050)) == 1) //到下方待命   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Down[..]
            {
                Task=1;
            }
            break;

        case 800:
            // 沒盤路徑:判斷是否要觸發載新盤
            if(iCleanOut == 1 || iOneCycle == 1)
            {
                // Clean out / One cycle 進行中 → 停等
                break;
            }
            InitLoadNewICTrayTask_9050();
            Task = 900;
            break;
        case 900:
            if(DoLoadNewICTray_9050() == true)
            {
                Task = 1;
            }
            break;
        case 1000:
            if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050Home(0)) == 1)                   //回到最下方   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Home
            {
                 #ifdef SOFT_SIMULTE
                 iLoaderLayerCount_9050=5;
                 Task = 1;
                 #else
                 if(Sen[SnLoaderDrawerHasTray].IsOff())                         //沒料報警
                 {
                    if(iCleanOut == 1 || iOneCycle == 1)
                    {
                        break;
                    }
                    //AI(W906-F9050-BD) 20261004: 910 as written: the answer is ignored (K_CLEAN_OUT does nothing here); Task=1 either way.
                    ret = ShowErrorMessage("MES0920", K_RETRY | K_CLEAN_OUT,MMTrayZ, false, "DoLoad_9050_1");
                    Task = 1;
                 }
                 else                                                           //有料 偵測盤數
                 {
                     iLoaderLayerCountDetect_9050 = W906_TrayZ9050ProbeStart();// 初始猜測層數 (teach 值,例:10)   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.iLoaderInitDetectLayer (never written = 0); taught now
                     Task = 1100;
                 }
                 #endif
            }
            break;
        case 1100:
            // 探測層數 起始判斷:先到 TrayZ_Up[iLoaderLayerCount_9050] (初始猜測層)
            //   sensor ON  → 起始層或更上還有 tray → 進 case 1110 往上探 OFF 邊界
            //   sensor OFF → 起始層太高沒 tray → 進 case 1120 往下探 ON 那層
            if(Cylinder[C_LoaderEdgeClip].Pop() &&                              //邊緣定位開
               Cylinder[C_LoaderEdgePush].Pop() &&
               Cylinder[C_TrayZ_Selector].Push())
            {
                if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 1, iLoaderLayerCountDetect_9050)) == 1)   //AI(W906-TRAYZ-PITCH) 20261006: probe = Up of the layer (EastSun「探測就是上位 加層數」); was Prod.TrayZ_Up[..]
                {
                    if(W906_LoaderProbeSeesTray())   /* AI(W906-PROBESENSOR9050) 20261008: was Sen[SnLoaderTrayHasTray].IsOn() */
                        Task = 1110;                                                    // 往上探
                    else
                        Task = 1120;                                                    // 往下探
                }
            }
            break;

        case 1110:
            // 往上探:每步 iLoaderLayerCount++,移動到新層,再檢查 sensor
            //   sensor ON  → 這層還有 tray,再 ++ 繼續
            //   sensor OFF → 這層沒 tray,前一層 (iLoaderLayerCount-1) 是最後 ON 那層 = 實際層數
            //   若 iLoaderLayerCount 增到 MAX_LOADER_LAYER_9050 仍 ON → 異常 MES0924
            iLoaderLayerCountDetect_9050++;
            if(iLoaderLayerCountDetect_9050 >= W906_TrayZ9050ProbeLimit())   //AI(W906-TRAYZ-PITCH) 20261006: EastSun「要可以讓我設探測極限」-- was the hard-coded MAX_LOADER_LAYER_9050 (20, still the default when unset)
            {
                ret = ShowErrorMessage("MES0924", K_RETRY, MMTrayZ, false, "DoLoad_9050_1110");
                if(ret == K_RETRY)
                {
                    //AI(W906-F9050-BD) 20261004: 910 as written: this retry (case 1110) re-seeds iLoaderLayerCount_9050 and goes to 1000, while case 1120's retry
                    //  re-seeds iLoaderLayerCountDetect_9050 and goes to 1 (910 :2680-2683 vs :2714-2717).  Reaching it reads TrayZ_Up[9..19].
                    iLoaderLayerCount_9050 = W906_TrayZ9050ProbeStart();   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.iLoaderInitDetectLayer
                    Task = 1000;
                }
                break;
            }
            Task = 1115;
            break;
        case 1115:
            if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 1, iLoaderLayerCountDetect_9050)) == 1)   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Up[..]
            {
                if(W906_LoaderProbeSeesTray())   /* AI(W906-PROBESENSOR9050) 20261008 */
                {
                    // 這層還 ON → 下次 poll 再 ++
                    Task = 1110;
                }
                else
                {
                    // 這層 OFF → 前一層才是答案
                    iLoaderLayerCount_9050=iLoaderLayerCountDetect_9050-1;
                    Task = 1;
                }
            }
            break;
        case 1120:
            // 往下探:每步 iLoaderLayerCount--,移動到新層,再檢查 sensor
            //   sensor OFF → 這層還沒 tray,再 -- 繼續
            //   sensor ON  → 這層有 tray = 實際層數
            //   若 iLoaderLayerCount 減到 < 0 仍 OFF → 異常 MES0924
            iLoaderLayerCountDetect_9050--;
            if(iLoaderLayerCountDetect_9050 < 0)
            {
                ret = ShowErrorMessage("MES0924", K_RETRY, MMTrayZ, false, "DoLoad_9050_1120");
                if(ret == K_RETRY)
                {
                    iLoaderLayerCountDetect_9050 = W906_TrayZ9050ProbeStart();   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.iLoaderInitDetectLayer
                    Task = 1;
                }
                break;
            }
            Task = 1125;
            break;
        case 1125:
            if(MOT[MLoaderZ].MotorMove(W906_TrayZ9050(0, 1, iLoaderLayerCountDetect_9050)) == 1)   //AI(W906-TRAYZ-PITCH) 20261006: was Prod.TrayZ_Up[..]
            {
                if(W906_LoaderProbeSeesTray())   /* AI(W906-PROBESENSOR9050) 20261008 */
                {
                    // 這層 ON → 就是答案
                    iLoaderLayerCount_9050=iLoaderLayerCountDetect_9050;
                    Task = 1;
                }
                else
                {
                    // 還是 OFF → 下次 poll 再 --
                    Task = 1120;
                }
            }
            break;
    }
}

//==============================================================================
//從Load Stage move for InArm1 to pick device,if no device wait for Catch arm
//==============================================================================
//int SupplyTrayTask=1;
//void InitSupplyTrayTask()
//{
//    SupplyTrayTask=1;
//}
//==============================================================================
// Load Srtage 主流程
//==============================================================================
int  LoadTask=1;
//------------------------------------------------------------------------------
void InitLoadTask()
{
    LoadTask=1;
}
//-----------------------------------------------------------------------------
void DoLoad()
{
    if(MachineTypeChoice==Type_HT9050)                                          //Eastsun 20260819   //AI(W906-F9050-BD) 20261004: H077, 910 asendic_Loader.cpp:2761-2765 verbatim; placed before golden's bOneTimeHotPlateCheckAll / bNewResetFunction checks, so DoLoad_9050 ignores both (910 as written)
    {
        DoLoad_9050();
        return;
    }
    int ret;
    bool flag;
    int &Task=LoadTask;
    static bool bLoadHasDuplicateError=false;                                   //Steven 20120220 : Load Tray重複Alarm Flag
    static bool bDetectLoad=false;                                              //JerryYang 20210507 : sensor必須on 3秒以上才當作有tray
    AnsiString sBuffer="";                                                      //kevin 20220607 add buffer

    if(bOneTimeHotPlateCheckAll)                                                //Steven 20120314 : 檢查HotPlate時,就不要再入料了
        return;

    if(IniConfig.bNewResetFunction==true && bResetLoadTray==true)
    {
        if(MOT[MMTrayY].fHasTray==true)
        {
            if(Sen[SnLoaderSureTray].IsOn()==false)
            {
                MOT[MMTrayY].ClearTray(__FUNC__);
                InitLoadTask();
                if(CosFunction.bShowHPICCount)                                  //Steven 20221228 : 計算加熱盤IC數量
                {
                    fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                }
            }
        }
        bResetLoadTray=false;
        return;
    }

    switch(Task)
    {
        case 1:
            #ifdef SOFT_SIMULTE
                Task=300;
                break;
            #else
                if(MOT[MMTrayZ].fHasTray)                                       //Magazine 載台上有Tray
                {
                    MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                    MOT[MMTrayZ].ClearTray(__FUNC__);
                }
                flag=Sen[SnLoaderSureTray].IsOn() ;                             //Loader 有Tray
                if(LastSet.iRealDummy==DUMMY && flag==false)
                {
                    Task=300;
                    break;
                }

                if(USE_LdUldCassetteMode==1)                                    //RogerYang 20260203 : 這裡沒有判斷裡面有沒有tray就直接入料
                {
                    bLoadHasDuplicateError=false;                               //Steven 20120220 : Load Tray重複Alarm Flag
                    Task=300;
                }
                else if(flag!=MOT[MMTrayY].fHasTray)
                {
                    if(MOT[MMTrayY].fHasTray &&
                       MOT[MTrayX].ReadPos()!=Prod.iXTrayLoad)                  // tray miss
                    {
                        Task=200;
                    }
                    else
                    {
                        Task=100;                                               // more tray or sensor error
                        if(TRAY_ARM_MODE==eUnderCoveyor)                        //Steven 20110501 修正alarm時,就放開汽缸
                        {
                            Cylinder[C_LoaderPushBack_Back].Off();
                            Cylinder[C_LoaderPushBack_Push].On();
                        }
                        else
                        {
                            Cylinder[C_TrayY_Fixer].Off();                      //Open Fix Supply Try Fix Cylinder
                        }
                        Cylinder[C_LoaderEdgePush].Off();
                        Cylinder[C_LoaderUpPress].Off();                        //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
                    }
                    break;
                }
                else                                                            //Loader Sensor 和 Loader Sensor 狀態一致
                {
                    bLoadHasDuplicateError=false;                               //Steven 20120220 : Load Tray重複Alarm Flag
                    Task=300;
                }
                break;
            #endif
        case 100:                                                                                                       //Please remove the loader tray
            ret=ShowErrorMessage("MES0922", K_RETRY, MMTrayY, false, "DoLoad100");                                      //jou 2011-03-28 訊息秀錯"JAM0908"改"MES0922"
            iInArmWaitPosition=0;                                                                                       //Ifor 20210209 add:旗標未清除導致Tray Arm 不作動
            if(ret==K_RETRY)
            {
                Task=1;
            }
            else
            {
                MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                Task=1;
            }
            break;
        case 200:
            if(IniConfig.bA04LoaderTraySplitFailCanSkip==true)
                ret=ShowErrorMessage("JAM0929", K_RETRY|K_SKIP, MMTrayY, bLoadHasDuplicateError, "DoLoad_200");         //Steven 20120925 : JAM0909 -> JAM0929
            else
                ret=ShowErrorMessage("JAM0929", K_RETRY, MMTrayY, bLoadHasDuplicateError, "DoLoad_200");                //Steven 20120925 : JAM0909 -> JAM0929

            bLoadHasDuplicateError=true;                                                                                //Steven 20120220 : Load Tray重複Alarm Flag
            if(ret==K_RETRY)
            {
                Task=1;
            }
            else
            {
                MOT[MMTrayY].ClearTray(__FUNC__);
                if(CosFunction.bShowHPICCount)                                                                          //Steven 20221228 : 計算加熱盤IC數量
                {
                    fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                }
                Task=300;
            }
            break;
        case 300:
            flag=false;
            if(Sen[SnLoaderCarHasTray].Enable)
            {
                flag=Sen[SnLoaderCarHasTray].IsOn();
                if(flag==MOT[MMTrayY_Car].fHasTray)
                {
                    Task=600;
                }
                else if(flag)
                {
                    Task=400;                                                   // has tray under load stage
                }
                else
                {
                    if(LastSet.iRealDummy==DUMMY)
                        Task=600;
                    else
                        Task=500;                                               // tray is lost
                }
            }
            else
            {
                Task=600;
            }
            break;
        case 400:                                                                                                       //Please remove the loader tray
            ret=ShowErrorMessage("MES0922", K_RETRY|K_SKIP, MMTrayY_Car, false, "DoLoad400");                           //jou 2011-03-28 訊息秀錯"JAM0908"改"MES0922"
            iInArmWaitPosition=0;                                                                                       //Ifor 20210209 add:旗標未清除導致Tray Arm 不作動
            if(ret==K_RETRY)
            {
                Task=300;
            }
            else
            {
                MOT[MMTrayY_Car].SetTray(HAS_IC, __FUNC__);
                Task=300;
            }
            break;
        case 500:
            if(IniConfig.bA04LoaderTraySplitFailCanSkip==true)
                ret=ShowErrorMessage("JAM0929", K_RETRY|K_SKIP, MMTrayY_Car, bLoadHasDuplicateError, "DoLoad_500");     //Steven 20120925 : JAM0909 -> JAM0929
            else
                ret=ShowErrorMessage("JAM0929", K_RETRY, MMTrayY_Car, bLoadHasDuplicateError, "DoLoad_500");            //Steven 20120925 : JAM0909 -> JAM0929

            bLoadHasDuplicateError=true;                                                                                //Steven 20120220 : Load Tray重複Alarm Flag
            if(ret==K_RETRY)
            {
                Task=300;
            }
            else
            {
                MOT[MMTrayY_Car].ClearTray(__FUNC__);
                Task=600;
            }
            break;
        case 600:                                                                                                       //kevin 20150530
            bLoadHasDuplicateError=false;                                                                               //Steven 20120220 : Load Tray重複Alarm Flag
            if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&                                                                       //kevin 20150601 高雄需留一空TRAY在上
               CosFunction.bUseSCKART==false)                                                                           //ChungHung 20141002 add for KYEC AutoRetest
            {
                if((IniConfig.bA10_AutoReTest || bAutoReTest_ART) && LastSet.bCleanOut_ART==true)
                {
                    if((bForKyecBu3RunART==false && Sen[SnLoaderTrayHasTray].IsOn()==true) ||
                       (bForKyecBu3RunART==true  && Sen[SnLoaderTrayHasTray_ART].IsOn()==true))                         //wei 20160113 ART增加一顆sensor  //wei 20161118 bRunART-->bForKyecBu3RunART
                    {
                        ShowMyMessage("Please remove Loader Magazine Tray!!!","請將入料區Tray盤移走!!!");
                        Task=600;
                        break;
                    }
                }
            }

            if(MOT[MMTrayY_Car].fHasTray)                                                                               // 上一段程式可能會使用　tray 上升 損壞
            {
                if(TestIF_File.bRENESAS_EnableFTCT==true)                                                               //20250926 RogerYang : 瑞薩FT-CT 有供料了，要解掉
                {
                    fMain->RENESAS_Server->bLoadingCountFullFlag=false;
                    fSCKART->bFirstFullSkip=false;                                                                      //RogerYang 20251112 : ART數量到，如果inarm上有IC直接搬走
                }
                InitSupplyNewIC_From_LoaderCarTask();
                Task=1000;
            }
            else
            {
                if(IniConfig.bA68_AutoLoadUnload==true)                                                                 //RogerYang 20250617 dont show message and waiting for the AMR to come until timeout (6 sec)
                    bWaitingAMR=false;
                Task=800;
            }
            break;
        case 800:
            if(IniConfig.bG12ContractModeManualMessage &&                                                                                                       //kevin 20180222 (Steven) contract mode Load 不入tray
               bLoadContractModeTest)
            {
                Task=600;
                break;
            }

            flag=false;

            if((iCleanOut==1 ||                                                                                                                                 //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
                iOneCycle==1 ||                                                                                                                                 //ChungHung 20150515 add for fix full shuttle hangup
                bOneCycle_BackUp==true) ||                                                                                                                      //JerryYang 20161129 iOneCycle_BackUp改成bool  //JerryYang 20240428 Unmark
               (TrayForm.bAutoFeed==false &&
                bLoaderNoTrayAutoCleanOut==true &&
                MOT[MMTrayY_Car].fHasTray==false))                                                                                                              // no load new tray
            {
                if(iOneCycle==1 &&                                                                                                                              //Steven 20241125 : fixed for one cycle hang up
                   bBackupCleanOut==false &&
                   fMain->ALed1->Value==false &&                                                                                                                //JerryYang 20250228 : fix Loader有tray一直跳 no tray
                   MOT[MMTrayY_Car].fHasTray==false &&
                   (MOT[MMTrayY].fHasTray==false ||
                    MOT[MMTrayY].HasIC()==false))
                {
                    flag=true;
                }
                else if(iOneCycle==1 &&                                                                                                                         //JerryYang 20250522 : fix Loader跟MMTrayY_Car都沒tray, Loader上方有tray的時候觸發one cycle會hang up
                        bBackupCleanOut==false &&
                        fMain->ALed1->Value==true &&
                        MOT[MMTrayY_Car].fHasTray==false &&
                        MOT[MMTrayY].fHasTray==false)
                {
                }
                else
                {
                    Task=600;
                    break;
                }
            }

            if(flag ||
               (fMain->ALed1->Value==false &&
                MOT[MMTrayY_Car].fHasTray==false &&
                MOT[MMTrayY].fHasTray==false))                                                                                                                  // no any tray
            {
                if((USE_AUTO_RETEST==eartInstall &&                                                                                                             //ChungHung 20141002 add for KYEC AutoRetest
                   (IniConfig.bA10_AutoReTest || bAutoReTest_ART) /*&&                                                                                          //Steven 20251216 : mark for ART
                   LastSet.bLoaderTrayCount_ART*/) ||
                   IniConfig.bA65_BundleIDList)
                {
                    if(CosFunction.bUseSCKART)                                                                                                                  //Steven 20161201 (wei) : For SCK 93K ART
                    {
                        if((TestIF_File.bSCKART_LotDeviceCheck &&
                            IniConfig.bA65_BundleIDList==false) &&
                           fSCKART->CheckLoadingCount()==0)
                        {
                            if(CosFunction.bEnableHandlerResultServer &&
                               IniConfig.bA60EnableAMR)                                                                                                         //Sam 20250423 : AMR + ART Retest 修改滿 Tray 也要收盤
                            {
                                                                                //AMR 模式 RT IC 不足時不需要報警。
                            }
                            else
                            {
                                if(TestIF_File.bRENESAS_EnableFTCT==true)                                                                                       //RogerYang 20250913 : 瑞薩FT-CT 數量不符要跳視窗，供給/不供給
                                {
                                    if(fMain->RENESAS_Server->DoNeedSupplyOrNot(true)==true)
                                        ret=K_RETRY;
                                    else
                                        ret=K_CLEAN_OUT;
                                }
                                else if(IniConfig.bSPILFunction==true)                                                                                          //JerryYang 20220923 : add SPIL
                                {
                                    ret=ShowErrorMessage("WAR0119", K_RETRY, MInArmX, false, "DoLoad");   //RogerYang 20260519 : fix bug
                                }
                                else
                                {
                                    ret=ShowErrorMessage("WAR0119", K_RETRY|K_CLEAN_OUT|K_SKIP, MInArmX, false, "DoLoad");
                                }

                                if(ret==K_CLEAN_OUT)
                                {
                                    fMain->CleanOut("DoLoad 1");
                                    if(flag)                                                                                                                    //Steven 20241125 : fixed for one cycle hang up
                                        bBackupCleanOut=true;
                                }
                            }
                        }
                        else if((TestIF_File.bSCKART_LotDeviceCheck &&                                                                                          //JerryYang 20250225 : fix
                                 fSCKART->iInputCount!=0 &&
                                 ((IniConfig.bA65_BundleIDList==false &&
                                   (fSCKART->CheckLoadingCount()==1 ||
                                    fSCKART->iNeedRT>0)) ||                                                                                                     //JerryYang 20250213 : add
                                  (IniConfig.bA65_BundleIDList==true  &&
                                   (fSCKART->CheckLoadingCount()==1 ||
                                    fSCKART->CheckLoadingCount()==2) &&
                                   slDupBundlID->Count>=slBundlID->Count))))
                        {
                            if(TestIF_File.bRENESAS_EnableFTCT==true)                                                                                           //RogerYang 20250913 : 瑞薩FT-CT 數量不符要跳視窗，供給/不供給
                            {                                                                                                                                   //若實際數量比設定數量多，做到底全部完畢之後跳出，供給/不供給視窗，顯示數量多xx枚。
                                if(fMain->RENESAS_Server->DoNeedSupplyOrNot(false)==true)
                                {
                                    Task=600;
                                    break;
                                }
                                //else 直接cleanout
                            }
                            fMain->CleanOut("DoLoad 2");
                            if(flag)                                                                                                                            //Steven 20241125 : fixed for one cycle hang up
                                bBackupCleanOut=true;
                            Task=600;
                            break;
                        }
                        else
                        {
                            if(IniConfig.bA65_BundleIDList &&                                                                                                   //JerryYang 20250521 : For AMR
                               slDupBundlID->Count<slBundlID->Count)
                            {
                                if(fSCKART->CheckLoadingCount()==0 &&
                                   IniConfig.bA68_AutoLoadUnload==true)                                                                                         //RogerYang 20250617 dont show message and waiting for the AMR to come until timeout (6 sec)
                                {
                                    if(bWaitingAMR==false)
                                    {
                                        bWaitingAMR=true;
                                        htLoadHasTrayDelay.SetSecAndOn(360);
                                        RecordProcess("Waiting for the AMR to come until timeout");
                                    }

                                    if(iPortStatus[ePortLoader]==eLoadReadyToLoad)
                                    {
                                        bWaitingAMR=false;
                                        RecordProcess("AMR is ready to load");
//                                        InitSupplyNewIC_From_LoaderCarTask();          //JerryYang 20250618 等待真的上料後，這裡的判斷式就不會再進來
//                                        Task=1000;             //bAskStopPort[ePortLoader] already true
                                        break;
                                    }
                                    else if(htLoadHasTrayDelay.Off()==true)
                                    {
                                        bWaitingAMR=false;
                                        ShowMyMessage("(AMR)Please check all cover tray has been put in loader!!", "(AMR)請確認所有COVER TRAY都已投入Loader!!");
                                        bForceSendLoaderIsEmpty=true;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    ShowMyMessage("Please check all cover tray has been put in loader!!", "請確認所有COVER TRAY都已投入Loader!!");
                                }
                            }

                            if(fAGV->IsATK_AMR() &&                                                                                                             //Steven 20251210 : ATK AMR
                               MOT[MTrayX].iIsCoverTray==IS_ID_TRAY)
                            {
                                ret=K_CLEAN_OUT;
                            }
                            else if(TestIF_File.bRENESAS_EnableFTCT==true)
                            {
                                ret=K_CLEAN_OUT;
                            }
                            else if(IniConfig.bNoTrayAutoCleanOut==false)                                                                                       //jou 2013-08-01 Loader No Tray Auto Clean
                            {
                                ret=ShowErrorMessage("MES0920", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, false, "DoLoad_800-1");                                       //Steven 20100202
                            }
                            else
                            {
                                ret=K_CLEAN_OUT;
                            }

                            if(ret==K_CLEAN_OUT)
                            {
                                fMain->CleanOut("DoLoad 3");
                                if(flag)                                                                                                                        //Steven 20241125 : fixed for one cycle hang up
                                    bBackupCleanOut=true;
                            }
                            Task=600;
                            break;
                        }
                    }
                    else if(LastSet.iLoaderTrayCount_ART<LastSet.iLoaderTotalTray)
                    {
                        if(CUSTOMER_CODE==CC_KYEC_LEE)                                                                                                          //wei 20150904 多盤只能CLEAN_OUT      //wei 20151207 只能RETRY
                            ret=ShowErrorMessage("WAR0962", K_RETRY, MMTrayY_Car, bLoadHasDuplicateError, "DoLoad_800-2");
                        else
                            ret=ShowErrorMessage("WAR0962", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, bLoadHasDuplicateError, "DoLoad_800-3");
                        //Clean out or retry
                        //Clea out -> rsmInitial_ART
                        if(ret==K_CLEAN_OUT)
                        {
                            if(CUSTOMER_CODE!=CC_KYEC_LEE)                                                                                                      //wei 20160127 不先清除
                                LastSet.iLoaderTrayCount_ART=0;
                            fMain->lblLoadTrayCnt->Caption=LastSet.iLoaderTrayCount_ART;
                            LastSet.bCleanOut_ART=true;
                            fMain->CleanOut("DoLoad 4");
                            if(flag)                                                                                                                            //Steven 20241125 : fixed for one cycle hang up
                                bBackupCleanOut=true;
                            Task=600;
                        }
                    }
                    else
                    {
                        if(CUSTOMER_CODE!=CC_KYEC_LEE)                                                                                                          //wei 20160127 不先清除
                            LastSet.iLoaderTrayCount_ART=0;
                        fMain->lblLoadTrayCnt->Caption=LastSet.iLoaderTrayCount_ART;
                        LastSet.bCleanOut_ART=true;
                        fMain->CleanOut("DoLoad 5");
                        if(flag)                                                                                                                                //Steven 20241125 : fixed for one cycle hang up
                            bBackupCleanOut=true;
                        Task=600;
                        break;
                    }
                }
                else if(IniConfig.bP57LoaderAutoCleanOutByInputCT &&                                                                                            //Sam 20250605 : Loader Count AutoCleanOut
                        LastSet.iP57_InputCT!=0 &&
                        fSCKART->CheckLoadingCount()==1)
                {
                    fMain->CleanOut("DoLoad 6");
                    if(flag)
                        bBackupCleanOut=true;
                    Task=600;
                    break;
                }
                else
                {
                    if(CosFunction.bEnableHandlerResultServer && IniConfig.bA60EnableAMR && LastSet.bAMRLoaderLast)                                             //Sam 20240304 : 新增 AMR 功能
                    {
                        ret=K_CLEAN_OUT;
                    }

                    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                                                                                           //wei 20161121 TSMC 新增[P07]Loader No Traya Auto TrayFeed
                    {
                        if(IniConfig.bP07NoTrayaAutoTrayFeed)
                            ret=K_CLEAN_OUT;
                        else
                            ret=ShowErrorMessage("MES0920", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, false, "DoLoad_800-4");                                           //Steven 20100202
                    }
                    else if(IniConfig.bNoTrayAutoCleanOut==false &&                                                                                             //Ifor 20180502 : Fix Clean Out 時執行 Auto Clean 會顯示 No Tray 問題 //KYEC 暫時不修改
                            iCleanOut==0 &&
                            bBackupCleanOut==false)                                                                                                             //jou 2013-08-01 Loader No Tray Auto Clean
                    {
                        ret=ShowErrorMessage("MES0920", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, false, "DoLoad_800-5");                                               //Steven 20100202
                    }
                    else
                    {
                        ret=K_CLEAN_OUT;
                    }

                    if(ret==K_CLEAN_OUT)
                    {
                        if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                                                                                             //Sam 20230220 : 移除客戶碼   //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
                        {
                            MOT[MMTrayY].Tray.bMustToEmpty=true;                                                                                                //to empty   //Sam 20230317 : 修正 P24 功能
                        }
                        fMain->CleanOut("DoLoad 7");

                        if(flag)                                                                                                                                //Steven 20241125 : fixed for one cycle hang up
                            bBackupCleanOut=true;
                    }
                    Task=600;
                    break;
                }
            }

            if(fMain->ALed1->Value==false)
            {
                bDetectLoad=false;                                                                                                                              //JerryYang 20210507 : sensor必須on 3秒以上才當作有tray
                Task=600;
                break;
            }
            else
            {
                if(bDetectLoad==false)                                                                                                                          //JerryYang 20210507 : sensor必須on 3秒以上才當作有tray
                {
                    bDetectLoad=true;
                    htLoadHasTrayDelay.SetSecAndOn(3);
                    break;
                }
                else if(htLoadHasTrayDelay.Off()==false)
                {
                    break;
                }
            }

            #ifdef SOFT_SIMULTE
            if(fMain->CheckBox1->Checked==false)
            {
                Task=600;
                break;
            }
            #endif
            bWaitingAMR=false;                                                                                                                                  //JerryYang 20250521 : For AMR
            bLoadContractModeTest=true;                                                                                                                         //kevin 20180222 (Steven) contract mode Load 不入tray
            bDetectLoad=false;                                                                                                                                  //JerryYang 20210507 : sensor必須on 3秒以上才當作有tray
            if(IniConfig.bEnable_SECS_GEM==true)
                EventReport(SECS_EVENT.LoadPortBundleArrived);
            InitLoadNewICTrayTask();
            Task=900;
            bLoadingNewICTray=true;                                                                                                                             //Sam 20240827 : 新增 AMR 功能
            bPortIsBusy[ePortLoader]=true;                                                                                                                      //JerryYang 20250521 : For AMR
            if(IniConfig.bA68_AutoLoadUnload==true)                                                                                                             //RogerYang 20250617 加入Log，客戶表示需要知道萬一AMR上料時翻盤的原因
                RecordProcess("DoLoadNewICTray(Start)");
            break;
        case 900:
            if(bDoIniStartAutoIonFanClean==true)                                //Isaac 20210609 : IO觸發IonFan清針，做完清針Loader才能入料
            {
                break;
            }
            else if(fTrayMapping->bDoTrayDeviceCheckFromLoader)                 //JimmyChiu 20220219 : 殘料檢查 From Loader
            {
                break;
            }

            if(bAskStopPort[ePortLoader]==true)                                 //JerryYang 20250505 : AGV準備上料, 不要動
            {
                Task=900;                                                       //RogerYang 20250611 add 為了可以卡中斷點
                break;
            }

            //==> Eastsun 20260818 : 9050 uses DoLoadNewICTray_9050   //AI(W906-F9050-BD) 20261004: H078, 910 :3342-3344 verbatim; dead for Type_HT9050 (DoLoad hands a 9050 to DoLoad_9050 at its top), kept as 910
            if((MachineTypeChoice==Type_HT9050 ? DoLoadNewICTray_9050() : DoLoadNewICTray()))
            //<== Eastsun 20260818
            {
                bLoadingNewICTray=false;                                        //Sam 20240827 : 新增 AMR 功能
                bPortIsBusy[ePortLoader]=false;
                if(IniConfig.bA68_AutoLoadUnload==true)                         //RogerYang 20250617 加入Log，客戶表示需要知道萬一AMR上料時翻盤的原因
                    RecordProcess("DoLoadNewICTray(Finish)");

                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20220607 add send load count to eKeep call AGV
                {
                    iStackCount[0]++;                                           //kevin 20220527 add Tray Count 0:load 1:empty 2:Color 3:Auto1 4:Auto2 5:Auto3
                    sBuffer.sprintf("Loader Count:%d", iStackCount[0]);
                    RecordProcess(sBuffer);
                }

                if(fTrayForm->IsEnableColorSensor()==true)                      //Jimmychiu 20230630 : add color sensor MU-N in Loader
                {
                    Task=950;
                    DoInspectTrayColorOnLoader(true);
                    break;
                }
                Task=600;
            }
            break;
        case 950:                                                               //Jimmychiu 20230630 : add color sensor MU-N in Loader
            if(fTrayForm->IsEnableColorSensor()==true)
            {
                if(DoInspectTrayColorOnLoader(false)==true)
                {
                    Task=600;
                }
            }
            else
            {
                Task=600;
            }
            break;
        case 1000:
            if(bDoIniStartAutoIonFanClean==true)                                //Isaac 20210609 : IO觸發IonFan清針，做完清針Loader才能入料
            {
                break;
            }

            if(bAskStopPort[ePortLoader]==true)                                 //JerryYang 20250505 : AGV準備上料, 不要動
            {
                Task=1000;                                                      //RogerYang 20250611 add 為了可以卡中斷點
                break;
            }

            if(DoSupplyNewICTray())                                             //Supply IC from Loader car
            {
                if(fTrayMapping->IsRunDeviceRemainLaser())
                    fTrayMapping->bDoTrayDeviceCheckFromLoader=true;
                if(IniConfig.bA68_AutoLoadUnload==true)                         //RogerYang 20250617 加入Log，客戶表示需要知道萬一AMR上料時翻盤的原因
                    RecordProcess("DoSupplyNewICTray(Finish)");
                bPortIsBusy[ePortLoader]=false;                                 //RogerYang 20250611 避免收到STOP_LOAD_PORT時狀態被切成eLoadReadyToLoad氣缸還在動
                if(Cylinder[C_LoadTrackFloodgate].Enable)
                    Cylinder[C_LoadTrackFloodgate].Off();
                TrayID[0][1]=TrayID[0][0];                                      //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
                TrayID[0][0]="";
                iOneTrayPickCount[0]=0;                                         //kevin 20210623  loader 吸取數量
                ASE_InTrayNum++;                                                //input 第幾盤 //Steven 20160531 : modify
                iLoadTrayCount++;
                iAse_LoadSkipTime=0;                                            //kevin 20170320 (Steven) add auto skip total time
                iAutoTrayendhasIC=0;                                            //kevin 20170314 (Steven) 吸到IC數量計數
                iAse_LoadTrayEndTotalTime=0;                                    //kevin 20170320 (Steven) add auto skip total time
                if(IniConfig.bKoreaFunction==false)
                    RecordProcess("Load New Tray Start");                       //kevin 20170614 (wei) log tray
                bAutoTrayEndHasIC=false;                                        //kevin 20170822 auto tray end 有吸到IC 換TRAY 要ALARM
                bAutoSkipStartXYlog=true;                                       //kevin 20170904 記錄SKIP TRAY X Y 位置
                bAutoSkiplog=false;
                ReadWriteTrayID(false);                                         //kevin 20220705 record color tray id move color pick
                bMustCoverIDTray=false;
                Task=1;
            }
            break;
    }
}
//------------------------------------------------------------------------------
int iLoaderTrackDetectICFloatingTask=1;
void InitLoaderTrackDetectICFloatingTask()                                      //Sam 20200316 : Loader Detect Tray
{
    iLoaderTrackDetectICFloatingTask=1;
}
//------------------------------------------------------------------------------
bool DoLoaderTrackDetectICFloating()                                            //Sam 20200316 : Loader Detect Tray
{
    int ret=0;
    static bool bLoaderHasDuplicateError=false;
    int &Task=iLoaderTrackDetectICFloatingTask;
    switch(Task)
    {
        case 1:
            Cylinder[C_LoaderEdgePush].Off();
            htDetectICFloatingDelay.SetSecAndOn(0.2);
            Task=100;
            break;
        case 100:
            if(htDetectICFloatingDelay.Off())
                Task=200;
            break;
        case 200:
            if(Cylinder[C_TrayY_Fixer].Pop())
            {
                htDetectICFloatingDelay.SetSecAndOn(0.2);
                Task=300;
            }
            break;
        case 300:
            if(htDetectICFloatingDelay.Off())
            {
                TrayMoveOut(true, 0, Prod.iMLoaderYCarPos);
                bLoaderTrayICDetectErr=false;
                DetectLoaderTime.SetSecAndOn(20.0);                             //Sam 20240108 : 整合 Loader 跳料檢查。
                Task=400;
            }
            break;
        case 400:
            if(TrayMoveStatus(0,__FUNC__)==0)                                   //Sam 20240108 : 整合 Loader 跳料檢查。
                DetectLoaderTime.SetSecAndOn(20.0);

            if(CheckLoaderICFloating(1)==false)                                 //Sam 20240108 : 整合 Loader 跳料檢查。
                break;

            TrayMoveOut(true, 0, Prod.iMLoaderYCarPos);
            if(Sen[SnLoaderCarHasTray].IsOff()==false)                          //Loader 完成退料
            {
                TrayMoveIn(true, 0, 99999);
                DetectLoaderTime.SetSecAndOn(20.0);
                Task=500;
            }

            if(DetectLoaderTime.Off())
            {
                ret=ShowErrorMessage("JAM1012", K_SKIP|K_RETRY, MMTrayY, bLoaderHasDuplicateError, __FUNC__);
                if(ret==K_RETRY)
                {
                    bLoaderHasDuplicateError=true;
                    break;
                }
                else
                {
                    bLoaderHasDuplicateError=false;
                    MOT[MMTrayY].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }

            break;
        case 500:
            if(TrayMoveStatus(0,__FUNC__)==0)                                                                           //Sam 20240108 : 整合 Loader 跳料檢查。
                DetectLoaderTime.SetSecAndOn(20.0);

            if(CheckLoaderICFloating(0)==false)                                                                         //Sam 20240108 : 整合 Loader 跳料檢查。
                break;

            TrayMoveIn(true, 0, 99999);
            if(Sen[SnLoaderSureTray].IsOn() && Sen[SnLoaderPreDete].IsOn())                                             //Loader 完成進料
            {
                htDetectICFloatingDelay.SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);                                //Sam 20200420 : 到了在滑一下。
                Task=600;
            }
            else if(DetectLoaderTime.Off())
            {
                ret=ShowErrorMessage("JAM0901", K_RETRY|K_SKIP, MMTrayY, bLoaderHasDuplicateError, __FUNC__);
                if(ret==K_RETRY)
                {
                    bLoaderHasDuplicateError=true;
                    break;
                }
                else
                {
                    bLoaderHasDuplicateError=false;
                    MOT[MMTrayY].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }
            break;
        case 600:
            if(htDetectICFloatingDelay.Off())
            {
                TrayMoveIn(false, 0);
                Task=1;
                return true;                                                    //pass
            }
            break;
    }
    return false;                                                               //pass
}
//------------------------------------------------------------------------------
bool CheckLoaderICFloating(int iDir)                                            //Sam 20240108 : 整合 Loader 跳料檢查。
{
    bool bDir=false ,bReCheckFlag=false;
    if(iDir==0)                                                                 //Tray In 檢查
    {
        bDir=(TrayMoveStatus(0,__FUNC__)==1)?true:false;
        bReCheckFlag=(Sen[SnLoaderCarHasTray].IsOn())?true:false;
    }
    else                                                                        //Tray Out 檢查
    {
        bDir=(TrayMoveStatus(0,__FUNC__)==2)?true:false;
        bReCheckFlag=(Sen[SnLoaderPreDete].IsOn() && Sen[SnLoaderSureTray].IsOn())?true:false;
    }

    if(bDir && bLoaderTrayICDetectErr==false)
    {
        if(Sen[SnLoaderTrackDetect].IsOn())
        {
            TrayMoveIn(false, 0);
            TrayMoveOut(false, 0);
            if(USE_AUTO_RETEST==eartInstall)                                    //Loader 有退回的機制需要退回去再重新進料檢查
            {
                htDetectTrayRetryDelay.SetSecAndOn(20);                         //Sam 20200803 : Alarm retry timeout
                bLoaderTrayICDetectErr=true;
            }
            ShowErrorMessage("JAM09102", K_RETRY, MMTrayY);
            return false;
        }
        else
        {
            return true;
        }
    }

    if(bLoaderTrayICDetectErr==true)
    {
        if(TrayMoveStatus(0,__FUNC__)==0)
            htDetectTrayRetryDelay.SetSecAndOn(20);

        if(iDir==0)
            TrayMoveOut(true, 0, Prod.iMLoaderYCarPos);
        else
            TrayMoveIn(true, 0, 99999);

        if(bReCheckFlag)
        {
            if(iDir==0)
                TrayMoveIn(true, 0, 99999);
            else
                TrayMoveOut(true, 0, Prod.iMLoaderYCarPos);

            bLoaderTrayICDetectErr=false;
        }
        else if(htDetectTrayRetryDelay.Off())                                   //Sam 20200803 : Alarm retry timeout
        {
            htDetectTrayRetryDelay.SetSecAndOn(20);
            if(iDir==0)
                ShowErrorMessage("JAM0912", K_RETRY, MMTrayY_Car);
            else
                ShowErrorMessage("JAM0901", K_RETRY, MMTrayY);
        }
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------
void CheckContinManualRemoveTray()                                              //Sam 20230203 : 修正P24放Tray異常
{
    if(IniConfig.bP24TwoTrayMustManuallyRemovedForGenernal ||
       IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT)                        //Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關
    {
        iManualRemoveTrayCnt--;                                                 //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
        if(iManualRemoveTrayCnt>0)
        {
            if(CosFunction.bSpecialP24)                                         //Sam 20230320 : 整合 P24 北興版為獨立功能碼
            {
                MOT[MMTrayY].Tray.iNeedManualRemoved=1;                         //to color alarm
            }
            else
            {
                if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                    MOT[MMTrayY].Tray.iNeedManualRemoved=0;                     //to empty alarm
                else
                    MOT[MMTrayY].Tray.iNeedManualRemoved=1;                     //to color alarm
            }
        }
    }
}
//------------------------------------------------------------------------------
bool DoKnockBeforeRemainICCheck(bool bIsFirst)
{
    int &Task=iKnockTrayTask;
    static int iVibrateCount=0;                                                 //JerryYang 20191001 loader震動馬達
    if(bIsFirst)
    {
        Task=1;
        return true;
    }
    else
    {
        switch(Task)
        {
//JerryYang 20191001 loader震動馬達
//==>
            case 1:                                                             //Loader震動馬達On
                if(StackDelay.Off())
                {
                    SW[SwLoaderVibration].On();
                    StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_VibrateOnDelay*0.1);
                    Task=1065;
                }
                break;
            case 1065:                                                          //Loader震動馬達Off
                if(StackDelay.Off())
                {
                    iVibrateCount++;
                    SW[SwLoaderVibration].Off();
                    StackDelay.SetSecAndOn(Ld_UldDelayTime.LD_VibrateOffDelay*0.1);
                    Task=1070;
                }
                break;
            case 1070:
                if(StackDelay.Off())
                {
                    if(iVibrateCount>=Ld_UldDelayTime.LD_VibrateLoopCnt)
                    {
                        Task=9999;
                        iVibrateCount=0;
                    }
                    else
                    {
                        Task=1;
                    }
                }
                break;
//<==
//JerryYang 20191001 loader震動馬達
            case 9999:                                                          // finish
                return true;
            default:
                ShowMessage(AnsiString(__FUNC__)+" wrong Step="+IntToStr(Task));
                return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
//==============================================================================
//AI(W906-PROBESENSOR9050) 20261008: EastSun 1008 "while probing the layers SnLoaderDrawerHasTray goes Off when a tray is found and On when not" --
//  DoLoad_9050 cases 1100 / 1115 / 1125 ask this instead of Sen[SnLoaderTrayHasTray].IsOn(). HT9050 (9050GPIB or Type_HT9050): the
//  drawer sensor, inverted (Off = a tray at this layer). Other machines: SnLoaderTrayHasTray on, as 910.
bool W906_LoaderProbeSeesTray()
{
    extern AnsiString W906_GpibModel;
    if(W906_GpibModel=="9050GPIB" || MachineTypeChoice==Type_HT9050)
        return Sen[SnLoaderDrawerHasTray].IsOff();
    return Sen[SnLoaderTrayHasTray].IsOn();
}
