// =============================================================================
//  AutoRetest.cpp  --  Auto-Retest (ART) top-level flow: the driver that recycles
//                      finished trays Auto1..Auto6 -> TrayArm -> Loader and then
//                      re-arms the machine for the retest lot
//
//  Faithful translation of golden AutoRetest.cpp (1697 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-ART) 20260807
//  Translation wave: W906-ART (pure-translation completion campaign).  Sibling of
//  the already-landed asendic_Auto_RT.cpp / asendic_Loader_RT.cpp tray SMs: this
//  file is golden's ONLY caller of InitAutoTrayToRearTask / DoAutoTrayToRear /
//  InitLoadNewAutoTrayToCarTask / DoLoadNewEmptyTrayToCar_RT (golden :1108/:1129/
//  :1136/:1151/:1208) and of InitUnLoadNewLoaderTrayTask / DoUnLoadNewLoaderToStack
//  / InitLoaderTrayToFrontTask / DoLoaderTrayToFront, so landing it makes those
//  four already-translated bodies reachable for the first time.
//
//  ROLE: owns 9 golden functions and the whole ART file-scope cursor/timer block.
//    DoAutoRetest            master ART sequencer.  switch(Task) 1 / 2 / 100 /
//                            110 / 130 / 135 / 140 / 150 / 160 / 170 / 200 / 205 /
//                            210 / 215 / 220 / 221 / 225 / 250 / 300
//    DoTrayArm_AutoRT        1 / 100 / 200 / 300 / 400
//    DoPickFromAuto_AutoRT   1 / 100 / 200 / 300 / 350 / 400 / 150 / 600 / 650 /
//                            500 / 900 / 1000 + exception arms 700 / 750 / 800
//                            (source order is NOT numeric order -- preserved)
//    DoPlaceToLoader_AutoRT  1 / 100 / 200 / 250 / 300 / 400 / 500 / 600 / 700 /
//                            900 + exception arm 800
//    DoAuto_AutoRT           1 / 100 / 200 / 210 / 211 / 212 / 213 / 300 / 400
//    DoLoader_AutoRT         1 / 100 / 200 / 300 / 400 / 500
//    GPIB_RemoteCommand / GPIB_QueryData / GPIB_SetData
//                            the RCMD: / SVID: / ECID: text protocol golden
//                            main.cpp:15657/:15674/:15689 feeds from the GPIB
//                            bridge (jou 2015-09-21)
//
//  WAVE SCOPE -- ACTIVE vs satisfied-by-shim:
//   ACTIVE (faithful, verbatim -- every golden line from :36 to :1697):
//     all 9 functions, all 6 task cursors, iARTCatchAutoTrayFinish,
//     bTrayArmPlaceToLoadFinish and all 5 TQPF_Timer objects.
//   SATISFIED-BY-SHIM (so it LINKS -- all SEVEN are TU-LOCAL to this file; this
//     unit adds NOTHING to any shared header):
//       S1 fMain->SetLotState(int)                 -> W906ART_FMAIN_SETLOTSTATE
//       S2 fMain->tGPIBMsg (TStringList*)          -> W906ART_FMAIN_TGPIBMSG
//       S3 fMain->CheckBox1 (TCheckBox*)           -> W906ART_FMAIN_CHECKBOX1
//       S4 fMain->ChangeTempMode(int,bool,bool)    -> W906ART_FMAIN_CHANGETEMPMODE
//       S5 fMain->RENESAS_Server->iFTRTCntTmpFor70 -> W906ART_RENESAS_IFTRTCNTTMPFOR70
//       S6 fMain->hanaART->IsContactAvailable()    -> W906ART_HANAART (REAL uHANA_ART)
//       S7 fFTPClient (TfFTPClient*)               -> W906ART_FTPCLIENT
//       plus the two MSG_CMD_*BarCode constants (golden MessageDef.h, untranslated),
//       mirrored TU-locally at their golden values.
//     Each is re-cited at its own call site below.  Full rationale + the exact
//     retirement action for each is in the SEAM REGISTER further down.
//   GATE (#if 0): NONE.  Nothing in this file is gated.  Everything golden does,
//     this file does.
//
//  WHY THERE IS NO GATE HERE.  Every cross-module surface golden reaches was
//  grepped against the ported tree before writing a line, and the tree turned out
//  to already own nearly all of it: MoveInArm2XYToWait / IsMoveInArm2XYToWait /
//  NewRecordProcess / SetMotorScaleSpeed / fTrayMapping->DoTrayMapCCD
//  (acatchtray_shims.h), MTrayXCanSafeMove / C_CatchTray_Fix_Puch / C_CatchTray_Fix_Pop
//  / InitialTrayMapTask (acatchtray.h), TrayArmMotorMove (Motor/mymotor.h:466),
//  the four Loader_RT SMs (asendic_Loader_RT.h), the four Auto_RT SMs
//  (asendic_Auto_RT.h), the loader-track float probe (asendic_Loader.h),
//  AutoCylinderUp/Middle/Lower (asendic.h), HasICUnderMachine (csystem.h:105),
//  SetRunStartMode (aHotPlateSubstrate.h:897), GetLastOpenFN / WriteIniData /
//  WriteDataToFile / DataPath / asBarCodeDownLot (common.h), and the whole
//  LastSet / Prod / TestIF / TestIF_File / IniConfig / CosFunction field set.
//  ALSO NOT A GATE: golden's `HGemPtr->GetECDataValue(S3)` -- that function IS
//  translated (golden uHGemEquipment.cpp:3650-3925), it simply now lives on the
//  extracted engine THGem embeds as `SvEcReg`, so the call is spelled
//  `HGemPtr->SvEcReg.GetECDataValue(S3)`.  Same body, same return.
//
//  ONE HEADER CONFLICT HAD TO BE WORKED AROUND -- READ BEFORE EDITING INCLUDES.
//  SECSGEM/uHGemEquipment.h:393-399 defines `typedef int TColor;` + clRed/clLime/
//  clYellow/clBlack at namespace scope with NO include guard around them, while
//  acatchtray_shims.h:124-129 defines TColor/clWhite/clYellow inside
//  `#ifndef HT9045_TCOLOR_SHIM`.  Including both -- which this unit must, one for
//  THGem and one for fTrayMapping/MoveInArm2XYToWait/NewRecordProcess -- is a hard
//  "redefinition of 'const TColor clYellow'" error in EITHER order.  Resolved here
//  the only way a file that owns neither header can: uHGemEquipment.h goes first
//  and `HT9045_TCOLOR_SHIM` is defined before acatchtray_shims.h so its (identical)
//  block is skipped.  Verified with `g++ -std=c++17 -fsyntax-only` on THIS file (exit
//  0, one warning, which is golden defect G below).  clWhite is the one symbol lost
//  that way; this unit does not use it.
//  ORDERING IS LOAD-BEARING, so do not reshuffle the include block: the `#define`
//  only works while NO include ABOVE it transitively pulls acatchtray_shims.h (if one
//  did, the shim block would run first, set the guard itself, and uHGemEquipment.h
//  would then be the redefinition).  Checked in this pass -- none of acatchtray.h /
//  asendic*.h / aHotPlateSubstrate.h / csystem.h includes it.
//  REPORTED (cannot fix from here -- both headers are outside this unit's write
//  scope): uHGemEquipment.h's four clXxx constants should get the same
//  HT9045_TCOLOR_SHIM guard, after which this `#define` line can be deleted.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init) dropped.
//  AnsiString (1-based) + AnsiString::sprintf via vclcompat (verbatim -- golden's
//  `Str.sprintf("...%s", sTrayPosName[...])` passing a raw AnsiString for %s is
//  kept as-is).  NO __fastcall / __property / __published in this .cpp.
//  SOFT_SIMULTE is NOT defined: #ifndef paths run, #ifdef paths are reproduced
//  verbatim but inert.  There are FOUR `#ifdef SOFT_SIMULTE` directives, spread
//  over THREE functions (counted by directive, not by "region", because the two
//  in DoAuto_AutoRT case 100 are separate blocks): golden :665 (with #else :668,
//  #endif :689 -- DoPickFromAuto_AutoRT case 500), golden :957 (#else :974,
//  #endif :1054 -- DoAuto_AutoRT case 1), golden :1058 (#endif :1102, no #else --
//  DoAuto_AutoRT case 100 head) and golden :1141 (#else :1144, #endif :1146 --
//  DoAuto_AutoRT case 100 tail).  4 #ifdef + 3 #else + 4 #endif = the 11
//  preprocessor directives this file's body carries, which matches golden exactly.
//  Numeric semantics kept EXACT (no int/float rewrites).
//
//  GOLDEN FALL-THROUGHS PRESERVED (every `case N:` whose predecessor does not
//  break/return; re-derived by scanning the decoded golden in this pass -- there
//  are FIVE, and none is "fixed" with a break):
//    (1) DoAutoRetest          case 2   -> case 100  (golden :113-114)
//    (2) DoAutoRetest          case 221 -> case 225  (golden :311-316; the `else`
//                                         arm breaks, the success arm falls through)
//    (3) DoPickFromAuto_AutoRT case 1   -> case 100  (golden :533-536)
//    (4) DoPickFromAuto_AutoRT case 700 -> case 750  (golden :707-711)
//    (5) DoPlaceToLoader_AutoRT case 1  -> case 100  (golden :768-770)
//
//  GOLDEN DEFECTS FOUND AND TRANSLATED FAITHFULLY ANYWAY.  Each was re-derived
//  from the decoded golden in this pass.  None is fixed here; all are reported.
//   A. DoPickFromAuto_AutoRT case 500, the `LastSet.iRealDummy==DUMMY` arm
//      (golden :672-675) sets `ret=true` and then falls to the shared `break;`
//      WITHOUT ever advancing Task.  Task stays 500 forever, so in DUMMY mode the
//      ART pick sub-SM HANGS at "cover the tray".  (The sibling `else` arm sets
//      Task=900.)  `ret` is also written and never read on that arm.
//   B. `iRetestNoIC` is dimensioned [3] (cmydef.h:4018 / cmydef.cpp:4191, matching
//      golden) but is indexed with `Pos`, and `Pos` comes from DoTrayArm_AutoRT's
//      `for(int i=eAuto1; i<=iAutoRight; i++)` loops (golden :415 / :451), so its
//      range is 0..iAutoRight: golden :526 `iRetestNoIC[Pos]=0;` and :573
//      `iRetestNoIC[Pos]++;`.  CONFIG-DEPENDENT, stated precisely rather than as
//      "always broken": with the shipped default iAutoRight==2 (cmydef.cpp:3031)
//      it stays in bounds, but on any 4/5/6-Auto machine Auto4..Auto6 write PAST
//      THE END of the array.  Same class of defect asendic_Auto_RT.cpp's banner
//      records for iReceiveAutoTray[3].
//   C. `int iAuto_AutoRT_Task[MAX_AUTO_TRAY] ={1};` (golden :38) initialises only
//      element [0]; [1]..[5] get 0, and DoAuto_AutoRT's switch has no `case 0:`,
//      so Auto2..Auto6 fall straight through to `return false` until their
//      DoAuto_AutoRT(Pos,true) reset runs.  bAutoOK_AutoRT (golden :44) has the
//      same one-element-initialiser shape.  Kept verbatim.
//   D. DoTrayArm_AutoRT case 200's production-log call (golden :474-485) has only
//      THREE arms -- Pos==0 -> MES0653 "from Auto 1", Pos==1 -> MES0654
//      "from Auto 2", else -> MES0655 "from Auto 3".  Auto4/5/6 are all logged as
//      "Auto 3".  The surrounding loop is 6-wide (golden :415/:451), so this is a
//      stale 3-Auto assumption, not a deliberate collapse.
//   E. GPIB_SetData's `do { ... } while(S2.Length()>0);` (golden :1558-1693) can
//      SPIN FOREVER on malformed input: the loop's only advance is
//      `S2=S4.SubString(S4.Pos(";")+1, S4.Length());` (golden :1692), and when S4
//      contains no ';' that is `SubString(1, len)`, i.e. S2 is reassigned its own
//      value and nothing shrinks.  GPIB_QueryData's twin loop was hardened against
//      exactly this in 2024 (`Jimmychiu 20241219`, golden :1492 and :1530) -- the
//      ECID side never got the same fix.
//   F. DoPlaceToLoader_AutoRT case 1 (golden :768-769) assigns `Task=1;` -- its own
//      label -- and then falls through into case 100.  Re-entry therefore always
//      restarts at case 1; harmless because case 100 immediately overwrites Task
//      on success, but it is why the SM has no way to resume mid-sequence.
//   G. DoAuto_AutoRT recomputes `iAutoTray[]` from five UI edits (golden :932-936,
//      five atoi() calls) on EVERY call, yet the array is READ only inside
//      `#ifdef SOFT_SIMULTE` (golden :958/:969/:1065/:1073/:1081/:1089/:1097).
//      In this build the whole block is dead work.  Also note index [eAuto1] is
//      never assigned -- only edtAuto2..edtAuto6 are read.
//   H. DoPickFromAuto_AutoRT's cursor `iTrayArm_PickFromAuto_AutoRT_Task` is a
//      SINGLE shared int while its timers/flags (DoPickFromAuto_AutoRTDelay[],
//      bDuplicateErr[], bC_Auto_Fixer_Off[]) are per-Auto arrays.  Two Autos
//      cannot be picked concurrently; DoTrayArm_AutoRT case 100 serialises them,
//      so this is consistent -- recorded because the mixed shape looks like a bug.
//   I. DoPickFromAuto_AutoRT case 350's time-out arm (golden :582-588) re-arms its
//      own 3 s timer and `break`s WITHOUT advancing Task, so "Unloader up press pop
//      error" re-raises every 3 s until the sensor clears.  Intentional-looking,
//      but it is an unbounded operator-dialog loop.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "AutoRetest.h"
//   #include "acatchtray.h" / "MachineType.h" / "note.h" / "mymessbox.h" /
//   "asendic.h" / "mymotor.h" / "aArmHeader.h" / "mycylin.h" / "mysensor.h" /
//   "MyKitSuck.h" / "main.h" / "cmydef.h" / "csystem.h" / "FTPClient.h" /
//   "database.h" / "cCounterClear.h" / "uLotInfo.h" / "common.h" / "cinitial.h" /
//   "SCK_ART.h" / "cTrayMapping.h"
//   //==> Eastsun 20260526 #026-4.PinN.P-N9inc Pin1 MSG_CMD :KYEC
//   #include "MessageDef.h"
//   //<== Eastsun 20260526 #026-4.PinN.P-N9inc
//   ; #pragma package(smart_init)
//
// SUBSTITUTIONS APPLIED (golden -> this tree):
//   <vcl.h>/MachineDefine.h -> "MachineDefine.h"        (de-VCL'd include hub)
//   note.h                  -> "canary_support.h"       (ShowErrorMessage)
//   mymessbox.h             -> "canary_support.h"       (ShowMyMessage)
//   cMyDB.h / main.h sinks  -> "acatchtray_shims.h" (NewRecordProcess) +
//                              "canary_support.h" (RecordProcess)
//   main.h  (TfMain)        -> "FormsFacade.h"          (fMain)
//   uLotInfo.h              -> "FormsFacade.h"          (fLotInfo->cbRunMode)
//   SCK_ART.h               -> "FormsFacade.h"          (fSCKART->iCurrent93KARTStep)
//   mymotor.h               -> "Motor/mymotor.h"        (MOT[] / TrayArmMotorMove)
//   MyKitSuck.h             -> "aHotPlateSubstrate.h"   (CatchTraySuck / SetRunStartMode)
//   cTrayMapping.h          -> "acatchtray_shims.h"     (fTrayMapping)
//   cinitial.h              -> "acatchtray_shims.h"     (SetMotorScaleSpeed)
//   aArmHeader.h            -> "acatchtray_shims.h"     (MoveInArm2XYToWait /
//                              IsMoveInArm2XYToWait) + a file-local extern for
//                              MoveOutArmXY_ToFix_Tray_Full, exactly as
//                              acatchtray.cpp:193 declares it (that function has a
//                              real body at acatchtray_shims.cpp:214 but NO header
//                              declaration anywhere in the tree)
//   asendic.h               -> "asendic.h" + "asendic_Auto_RT.h" +
//                              "asendic_Loader.h" + "asendic_Loader_RT.h" (the tray
//                              SMs golden reached through the asendic god-header)
//   FTPClient.h             -> TU-local seam S7 (the KYECFTP wave demoted fFTPClient)
//   MessageDef.h            -> TU-local mirror of the two MSG_CMD_*BarCode values
//   cCounterClear.h         -> not needed (no counter-clear symbol in this body)
// =============================================================================
#include "MachineDefine.h"

#include "AutoRetest.h"

//---------------------------------------------------------------------------

#include "acatchtray.h"             // MTrayXCanSafeMove / C_CatchTray_Fix_Puch / C_CatchTray_Fix_Pop / InitialTrayMapTask
#include "MachineType.h"            // MAX_AUTO_TRAY / eAuto1..eAuto6 / e3Auto1.. / tTrayAuto / tNotUse / eartInstall / etmInstall / eocrUninstal / eocrYMotInstal / iTray_Map / TrayArm / MAX_SOCKET_ROW/COL / CC_*
#include "asendic.h"                // AutoCylinderUp / AutoCylinderMiddle / AutoCylinderLower
#include "asendic_Auto_RT.h"        // InitAutoTrayToRearTask / DoAutoTrayToRear / InitLoadNewAutoTrayToCarTask / DoLoadNewEmptyTrayToCar_RT
#include "asendic_Loader.h"         // InitLoaderTrackDetectICFloatingTask / DoLoaderTrackDetectICFloating
#include "asendic_Loader_RT.h"      // InitUnLoadNewLoaderTrayTask / DoUnLoadNewLoaderToStack / InitLoaderTrayToFrontTask / DoLoaderTrayToFront
#include "Motor/mymotor.h"          // MOT[] (TTrayMotor: fHasTray/fCanMove/Tray/ClearTray/SetTray/MotorMove/CompareCommandPos) + TrayArmMotorMove
#include "mycylin.h"                // Cylinder[]  (also pulls myTimer.h -> TQPF_Timer)
#include "mysensor.h"               // Sen[]
#include "myTimer.h"                // TQPF_Timer (explicit -- this file owns 5 of them)
#include "aHotPlateSubstrate.h"     // CatchTraySuck (TMyKitSuck: iWhichTray / SetItemData) + SetRunStartMode / rsmInitial_ART
#include "FormsFacade.h"            // fMain / fLotInfo->cbRunMode / fSCKART->iCurrent93KARTStep
#include "cmydef.h"                 // global scalar universe: MTrayX.. / C_*.. / Sn*.. / iMMAuto[] / sJAM1110[] / K_RETRY / K_SKIP / DUMMY / NULL_IC / HAS_IC / CUSTOMER_CODE / USE_* / Tempture_* / iArt_AutoTotal / iRetestNoIC / bARTTrayArmOK / ...
#include "csystem.h"                // HasICUnderMachine()
#include "database.h"               // HSys (SYSTEM_MODULAR) -> MyGem
#include "common.h"                 // DataPath / GetLastOpenFN / WriteIniData / WriteDataToFile / asBarCodeDownLot
#include "cprod.h"                  // Prod / TestIF / TestIF_File / ArmSpeed[] / UserDefForm[] / Ld_UldDelayTime / Temperature_NET
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "canary_support.h"         // ShowErrorMessage / ShowMyMessage / RecordProcess / LastSet / __FUNC__
// SECSGEM: THGem (for HGemPtr->SvEcReg.GetECDataValue) MUST be included BEFORE
// acatchtray_shims.h -- see the header-conflict note in the banner above.
#include "SECSGEM/uHGemClass.h"     // HTGem (the type of HSys.MyGem) -> HGemPtr
#include "SECSGEM/uHGemEquipment.h" // THGem (complete type) -> SvEcReg; also defines TColor/clRed/clLime/clYellow/clBlack UNGUARDED
#define HT9045_TCOLOR_SHIM          // AI(W906-ART) 20260807: suppress acatchtray_shims.h:124-129's IDENTICAL TColor/clYellow block (banner "ONE HEADER CONFLICT"); clWhite is unused here
#include "acatchtray_shims.h"       // MoveInArm2XYToWait / IsMoveInArm2XYToWait / NewRecordProcess / SetMotorScaleSpeed / fTrayMapping->DoTrayMapCCD / SECS_EVENT + EventReport
#include "Automation/HANA_ART.h"    // uHANA_ART -- the REAL body behind seam S6 (same wiring csystem.cpp:143 uses)

// =============================================================================
//  SEAM REGISTER -- SEVEN TU-LOCAL STAND-INS + TWO MIRRORED CONSTANTS
//
//  All of them are file-static, so no duplicate symbol can ever be created and no
//  shared header is perturbed.  Every one is re-cited at its own call site below.
//  This is the same idiom csystem.cpp:2754-2884 and Automation/SCK_ART_Remainder.cpp
//  :309-316 already use.  Placed BELOW all #includes on purpose.
//
//  RETIREMENT ACTIONS (each is a one-liner in a file this unit does not own):
//   S1 add `virtual void SetLotState(int iState);` to forms/fMain.h and retire
//      THIS macro together with csystem.cpp:2874's and SCK_ART_Remainder.cpp:316's
//      -- three copies of the same stand-in exist today.
//   S2 add `TStringList *tGPIBMsg;` to forms/fMain.h (golden main.h; same shape as
//      the already-real tTestResult / tBarCodeList / tSiteOnOff trio).
//   S3 add `TfMainCheckBox *CheckBox1;` to forms/fMain.h and DELETE the
//      "DELIBERATELY NOT ADDED" note at forms/fMain.h:600-607 -- its premise
//      ("its ONLY reference ... sits inside `#ifdef SOFT_SIMULTE`") is falsified
//      by golden AutoRetest.cpp:977 and :1111, both live real-machine code.
//   S4 add `virtual int ChangeTempMode(int iMode, bool bManual, bool bRefresh);`
//      to forms/fMain.h and collapse Automation/auto9045.cpp:218's TU-local copy
//      onto it.
//   S5 add `int iFTRTCntTmpFor70;` to TfMainRENESASServer (forms/fMain.h:126-133).
//   S6 retire when TfMainHanaART is retyped onto the real uHANA_ART -- the exact
//      change csystem.cpp:2683-2693 already flags for its own identical seam.
//   S7 retire when a home for golden's TfFTPClient recipe-download surface exists.
//      NOTE this is NOT the KYECFTP transfer layer that already landed:
//      bControlBySECSGEM / aSetUpNameBySECSGEM / ShowFTPModal(int) are the
//      SECS-driven modal recipe download, and no translated code carries them.
//   S8/S9 (the two constants) retire when golden MessageDef.h/.cpp is translated.
// =============================================================================

//  S1 -- golden fMain->SetLotState(int).  Pure no-op, exactly like its two
//  existing twins.  Golden's body is a lot-state UI/telemetry update in the
//  untranslated main.cpp.
#define W906ART_FMAIN_SETLOTSTATE(n)   do { (void)(n); } while(0)   // golden fMain->SetLotState(int)

//  S2 -- golden fMain->tGPIBMsg (TStringList*, the operator-visible GPIB message
//  strip).  REAL storage rather than a sink: the two golden Add() calls are the
//  only way a tester-side RCMD:S10F3 / RCMD:TESTER_ERROR is ever surfaced, so a
//  test must be able to read the text back.
static TStringList          W906ART_fMain_tGPIBMsg;
#define W906ART_FMAIN_TGPIBMSG        (&W906ART_fMain_tGPIBMsg)

//  S3 -- golden fMain->CheckBox1 (TCheckBox*, golden main.h:361).  Offline
//  Checked==false, which is the faithful posture: golden's DUMMY arms read it to
//  decide whether the operator has asked for a simulated tray, and there is no
//  operator here.  A settable object (not a constant) so a fixture can drive the
//  DUMMY branch at golden :977 / :1111.
static TfMainCheckBox       W906ART_fMain_CheckBox1;
#define W906ART_FMAIN_CHECKBOX1       (&W906ART_fMain_CheckBox1)

//  S4 -- golden fMain->ChangeTempMode(int iMode, bool bManual, bool bRefresh).
//  Returns 0 == golden's success code, so the golden `if(ret==0)` success cascade
//  at :1614-1622 stays reachable.  Byte-identical to the stand-in
//  Automation/auto9045.cpp:218 already carries for the same golden member.
static int  W906ART_ChangeTempMode(int /*iMode*/, bool /*bManual*/, bool /*bRefresh*/) { return 0; }
#define W906ART_FMAIN_CHANGETEMPMODE  W906ART_ChangeTempMode

//  S5 -- golden fMain->RENESAS_Server->iFTRTCntTmpFor70 (RogerYang 20251019,
//  RENESAS FT-CT retry budget).  Write-only from this file and read by nothing in
//  the ported tree, so a plain int is behaviour-complete here.
static int                  W906ART_RENESAS_IFTRTCNTTMPFOR70 = 0;

//  S6 -- golden fMain->hanaART->IsContactAvailable().  NOT a stub: wired to a REAL
//  embedded uHANA_ART (Automation/HANA_ART.cpp:905), 1:1 signature match, exactly
//  as csystem.cpp:2694-2701's W7C2_TfHanaSeam does.  The sibling call on the same
//  golden line, fMain->hanaART->IsHanaArtAvailable(), is left on the REAL facade
//  member (forms/fMain.h:83) and short-circuits this one offline.
struct W906ART_TfHanaSeam {
    uHANA_ART real;
    bool IsContactAvailable(){ return real.IsContactAvailable(); }
};
static W906ART_TfHanaSeam   W906ART_fHana_ext;
#define W906ART_HANAART               (&W906ART_fHana_ext)

//  S7 -- golden fFTPClient (TfFTPClient*, golden FTPClient.h:207): the SECS-driven
//  modal recipe download behind `RCMD:DOWNLOAD_RECIPE_BY_FTP`.  Member names and
//  types are golden's, verbatim.  ShowFTPModal(int) is a no-op offline (there is no
//  FTP server and no modal form), which leaves golden's own verdict line --
//  `if(fMain->cbSetupFileName->Text==S2)` -- to decide HCACK exactly as golden
//  intends: the recipe name did not change, so HCACK=1 (failed), not 0.
struct W906ART_TfFTPClientSeam {
    bool       bControlBySECSGEM;      // golden FTPClient.h -- a SECS download is in flight
    AnsiString aSetUpNameBySECSGEM;    // golden FTPClient.h -- requested recipe name
    void ShowFTPModal(int /*iMode*/){} // golden FTPClient.h -- offline: no modal, no transfer
    W906ART_TfFTPClientSeam():bControlBySECSGEM(false){}
};
static W906ART_TfFTPClientSeam W906ART_fFTPClient_ext;
#define W906ART_FTPCLIENT             (&W906ART_fFTPClient_ext)

//  S8/S9 -- golden MessageDef.h (Eastsun 20260526 #026-4.PinN.P-N9inc Pin1
//  MSG_CMD :KYEC).  MessageDef.cpp is not translated; the two values used here are
//  mirrored VERBATIM from golden MessageDef.cpp:47-48 (both `const unsigned int`,
//  31 and 32).  fMain->SendMSG_CMD(int) that consumes them IS real.
static const unsigned int W906ART_MSG_CMD_EnableBarCode  = 31;   // golden MessageDef.cpp:47
static const unsigned int W906ART_MSG_CMD_DisableBarCode = 32;   // golden MessageDef.cpp:48

//  Out-arm safe-position helper.  golden aoutarm.h:41 declares it; this tree has a
//  REAL body (acatchtray_shims.cpp:214) but NO header declaration anywhere, so the
//  extern is written at file scope exactly the way acatchtray.cpp:193 writes it.
extern bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false);                    //Eastsun 20260515 F011 輸出:KYEC AMR cover-tray extern decls

// =============================================================================
//  Everything below this line is golden AutoRetest.cpp:36-1697, VERBATIM apart
//  from the seam substitutions listed in the SEAM REGISTER above (each one is
//  re-cited, with its golden spelling, at its own call site).
// =============================================================================
int iAutoRetestTask                     =1;
int iTrayArm_AutoRT_Task                =1;
int iAuto_AutoRT_Task[MAX_AUTO_TRAY]    ={1};
int iLoader_AutoRT_Task                 =1;
int iTrayArm_PickFromAuto_AutoRT_Task   =1;
int iTrayArm_PlaceToLoad_AutoRT_Task    =1;
int iARTCatchAutoTrayFinish             =0;                                     //ChungHung 20141208 add fix ART catch Tray 與 Auto Tray Send to Rear 互搶
bool bTrayArmOK_AutoRT                  =false;
bool bAutoOK_AutoRT[MAX_AUTO_TRAY]      ={false};
bool bLoadOK_AutoRT                     =false;
bool bTrayArmPlaceToLoadFinish          =false;
TQPF_Timer DoARTFixTrayDelay;                                                   //Sam 20200203 : 搬完 Tray 後要夾
TQPF_Timer tCommandDelay;
//---------------------------------------------------------------------------
bool DoAutoRetest(bool bReset)
{
    bool bResult=false;
    int &Task=iAutoRetestTask;

    if(bReset==true)
    {
        Task=1;
        bTrayArmOK_AutoRT=false;
        bAutoOK_AutoRT[eAuto1]=(Prod.iTrayType[eAuto1]==tTrayAuto)?false:true;
        bAutoOK_AutoRT[eAuto2]=(Prod.iTrayType[eAuto2]==tTrayAuto)?false:true;
        bAutoOK_AutoRT[eAuto3]=(Prod.iTrayType[eAuto3]==tTrayAuto)?false:true;
        bAutoOK_AutoRT[eAuto4]=(Prod.iTrayType[eAuto4]==tTrayAuto)?false:true;
        bAutoOK_AutoRT[eAuto5]=(Prod.iTrayType[eAuto5]==tTrayAuto)?false:true;
        bAutoOK_AutoRT[eAuto6]=(Prod.iTrayType[eAuto6]==tTrayAuto)?false:true;
        bLoadOK_AutoRT   =false;
        bARTTrayArmOK    =false;
        SetMotorScaleSpeed(MTrayX, IniConfig.iARTTrayArmSpeed);                 //Steven 20170118 : For ART Tray Arm Speed
        //AI(W906-ART) 20260807: golden fMain->RENESAS_Server->iFTRTCntTmpFor70 on the next
        //  if's body line.  The RENESAS_Server pointer IS real (forms/fMain.h:599) but
        //  TfMainRENESASServer carries only bLoadingCountFullFlag / DoNeedSupplyOrNot -- this
        //  counter has no home, so it is a TU-local int (write-only here; nothing in this
        //  tree reads it back).  Comment kept ABOVE the `if` so the unbraced body stays visibly
        //  attached to it.
        if(TestIF_File.bRENESAS_EnableFTCT==true)                               //RogerYang 20251019 : 瑞薩FT-CT
            W906ART_RENESAS_IFTRTCNTTMPFOR70=TestIF_File.iSCKART_TryCnt;

        return bResult;
    }

    if(MOT[MInArmX].CompareCommandPos(Prod.iInArmSafeX, 2)!=1 ||                //Steven 20240110 : fixed for ART hang up
       MOT[MInArmY].CompareCommandPos(Prod.iInArmSafeY, 2)!=1)
    {
        if(MoveInArm2XYToWait()==false)
            return false;
    }

    if(MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)!=1)                //Steven 20240110 : fixed for ART hang up  //Steven 20211202 : 修正因為更換X安全位置導致ART hang up
    {
        if(MoveOutArmXY_ToFix_Tray_Full()==false)
            return false;
    }

    switch(Task)
    {
        case 1:
            Cylinder[C_HotplateVibration].Off();                                //JerryYang 20200311 修正ART搬tray時振動馬達一直震
            Cylinder[C_TrayVibration].Off();
            Task=2;
            break;
        case 2:
            fSCKART->iCurrent93KARTStep=6;
            if(IniConfig.bEnable_SECS_GEM==true)                                //ChungHung 20150511 modify
                EventReport(SECS_EVENT.ArtReceiveTraySTART);                    //60    開關Site

            if(CUSTOMER_CODE==CC_KYEC_XILINX)                                   //Frank 20160728 add
            {
                bAutoRetestMusic=true;
            }

            DoTrayArm_AutoRT(true);                                             //初始化 TrayArm 動作
            DoAuto_AutoRT(eAuto1, true);                                        //初始化 Auto1 入Tray 動作
            DoAuto_AutoRT(eAuto2, true);                                        //初始化 Auto2 入Tray 動作
            DoAuto_AutoRT(eAuto3, true);                                        //初始化 Auto3 入Tray 動作
            DoAuto_AutoRT(eAuto4, true);                                        //初始化 Auto4 入Tray 動作
            DoAuto_AutoRT(eAuto5, true);                                        //初始化 Auto5 入Tray 動作
            DoAuto_AutoRT(eAuto6, true);                                        //初始化 Auto6 入Tray 動作
            DoLoader_AutoRT(true);                                              //初始化 Loader 出Tray 動作
            iARTCatchAutoTrayFinish=0;                                          //ChungHung 20141208 add fix ART catch Tray 與 Auto Tray Send to Rear 互搶
            Task=100;
        case 100:
            fSCKART->iCurrent93KARTStep=6;
            if(bAutoOK_AutoRT[eAuto1]==false)
            {
                if(Prod.iTrayType[eAuto1]==tTrayAuto)
                    bAutoOK_AutoRT[eAuto1]=DoAuto_AutoRT(eAuto1);
                else
                    bAutoOK_AutoRT[eAuto1]=true;
            }

            if(bAutoOK_AutoRT[eAuto2]==false)
            {
                if(Prod.iTrayType[eAuto2]==tTrayAuto)
                    bAutoOK_AutoRT[eAuto2]=DoAuto_AutoRT(eAuto2);
                else
                    bAutoOK_AutoRT[eAuto2]=true;
            }

            if(bAutoOK_AutoRT[eAuto3]==false)
            {
                if(Prod.iTrayType[eAuto3]==tTrayAuto)
                    bAutoOK_AutoRT[eAuto3]=DoAuto_AutoRT(eAuto3);
                else
                    bAutoOK_AutoRT[eAuto3]=true;
            }

            if(bAutoOK_AutoRT[eAuto4]==false)
            {
                if(Prod.iTrayType[eAuto4]==tTrayAuto)
                    bAutoOK_AutoRT[eAuto4]=DoAuto_AutoRT(eAuto4);
                else
                    bAutoOK_AutoRT[eAuto4]=true;
            }

            if(bAutoOK_AutoRT[eAuto5]==false)
            {
                if(Prod.iTrayType[eAuto5]==tTrayAuto)
                    bAutoOK_AutoRT[eAuto5]=DoAuto_AutoRT(eAuto5);
                else
                    bAutoOK_AutoRT[eAuto5]=true;
            }

            if(bAutoOK_AutoRT[eAuto6]==false)
            {
                if(Prod.iTrayType[eAuto6]==tTrayAuto)
                    bAutoOK_AutoRT[eAuto6]=DoAuto_AutoRT(eAuto6);
                else
                    bAutoOK_AutoRT[eAuto6]=true;
            }

            if(bTrayArmOK_AutoRT==false)
            {
                bTrayArmOK_AutoRT=DoTrayArm_AutoRT();
            }

            if(bLoadOK_AutoRT==false)
            {
                bLoadOK_AutoRT=DoLoader_AutoRT();
            }

            if(bTrayArmOK_AutoRT==true)
            {
                iArt_AutoTotal[eAuto1][1]=LastSet.BinCT_ART[0][e3Auto1];        //kevin 20170830 (Steven) 記錄ART Auto Tray IC數量
                iArt_AutoTotal[eAuto2][1]=LastSet.BinCT_ART[0][e3Auto2];        //kevin 20170830 (Steven) 記錄ART Auto Tray IC數量
                iArt_AutoTotal[eAuto3][1]=LastSet.BinCT_ART[0][e3Auto3];        //kevin 20170830 (Steven) 記錄ART Auto Tray IC數量
                iArt_AutoTotal[eAuto4][1]=LastSet.BinCT_ART[0][e3Auto4];
                iArt_AutoTotal[eAuto5][1]=LastSet.BinCT_ART[0][e3Auto5];
                iArt_AutoTotal[eAuto6][1]=LastSet.BinCT_ART[0][e3Auto6];
            }

            if(bTrayArmOK_AutoRT &&
               bAutoOK_AutoRT[eAuto1] &&
               bAutoOK_AutoRT[eAuto2] &&
               bAutoOK_AutoRT[eAuto3] &&
               bAutoOK_AutoRT[eAuto4] &&
               bAutoOK_AutoRT[eAuto5] &&
               bAutoOK_AutoRT[eAuto6])
            {
                bARTTrayArmOK=true;                                             //wei 20151217 ART 回盤OK
            }

            if(bTrayArmOK_AutoRT &&
               bAutoOK_AutoRT[eAuto1] &&
               bAutoOK_AutoRT[eAuto2] &&
               bAutoOK_AutoRT[eAuto3] &&
               bAutoOK_AutoRT[eAuto4] &&
               bAutoOK_AutoRT[eAuto5] &&
               bAutoOK_AutoRT[eAuto6] &&
               bLoadOK_AutoRT)
            {
                if(CUSTOMER_CODE==CC_KYEC_XILINX)                               //Frank 20160728 add
                {
                    bAutoRetestMusic=false;
                }

                if(CosFunction.bAutoRetestGPIBmode==true)                       //jou 2015-10-02 Auto Retest GPIB mode
                {
                    InitLoaderTrackDetectICFloatingTask();                      //Sam 20200316 : Loader Detect Tray
                    Task=110;                                                   //Sam 20200316 : Loader Detect Tray
                }
                else
                {
                    bResult=true;
                }

                if(CUSTOMER_CODE==CC_TSMC_TAINAN)                               //wei 20170119 (jou) ATR FT/RT count
                {
                    iATRFtRtMode=1;
                }
            }
            break;
        case 110:                                                               //Sam 20200316 : Loader Detect Tray
            if(Sen[SnLoaderTrackDetect].Enable==true)
            {
                if(DoLoaderTrackDetectICFloating())
                {
                     Task=130;                                                  //Sam 20200320 : TrayY_Fixer 先勾推一下
                }
            }
            else
            {
                Task=130;                                                       //Sam 20200320 : TrayY_Fixer 先勾推一下
            }
            break;
       case 130:                                                                //Sam 20200320 : TrayY_Fixer 先勾推一下
            if(Cylinder[C_TrayY_Fixer].Push())
            {
                DoARTFixTrayDelay.SetSecAndOn(Ld_UldDelayTime.LD_FixTrayDely+0.5);
                Task=135;
            }
            break;
        case 135:
            if(DoARTFixTrayDelay.Off())
                Task=140;
            break;
        case 140:
            if(Cylinder[C_TrayY_Fixer].Pop())
                Task=150;
            break;
        case 150:
            Cylinder[C_LoaderEdgePush].On();
            DoARTFixTrayDelay.SetSecAndOn(0.2);
            Task=160;
            break;
        case 160:
            if(DoARTFixTrayDelay.Off())
                Task=170;
            break;
        case 170:
            if(Cylinder[C_TrayY_Fixer].Push())
            {
                Task=200;
            }
            break;
        case 200:                                                               //Sam 20201113 : ART 完要跑 Tray Map
            if(USE_TRAY_MAPPING==etmInstall &&
               TestIF_File.bEnableTrayMap)
            {
                Task=205;
            }
            else
            {
                Task=250;
            }
            break;
        case 205:
            if(IsMoveInArm2XYToWait())
            {
                InitialTrayMapTask(iTray_Map);
                bLoaderTrayAction=true;
                Task=215;
            }
            else
            {
                Task=210;
            }
            break;
        case 210:
            if(MoveInArm2XYToWait())
            {
                Task=205;
            }
            break;
        case 215:
            InitialTrayMapTask(iTray_Map);                                      //Sam 20200507 : 整合 Tray Map Function
            Task=220;
            break;
        case 220:
            if(fTrayMapping->DoTrayMapCCD(iTray_Map)==true)                     //Sam 20200507 : 整合 Tray Map Function
            {
                Task=225;
            }
            else if(TestIF_File.bEnableTrayMap==false)
            {
                Task=225;
            }
            break;
        case 221:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=225;
            else
                break;
        case 225:
            if(MTrayXCanSafeMove()==false)
            {
                Task=221;
                break;
            }

            if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bLoaderTrayAction=false;
                    Task=250;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bLoaderTrayAction=false;
                    Task=250;
                }
            }
            break;
        case 250:
            if(fMain->hanaART->IsHanaArtAvailable()==true)                      //JimmyChiu 20241023 HANA ART Function
            {
            }
            else if(CUSTOMER_CODE==CC_PTI &&
                    IniConfig.bB03_TesterReport &&                              //Sam 20240809 : PTI ART 模式
                    LastSet.bWaitEndLotAutoRetestGPIB==false)
            {
                break;
            }

            LastSet.bWaitStartLotAutoRetestGPIB=false;
            LastSet.bEndLotAutoRetestGPIB=false;
            //AI(W906-ART) 20260807: golden fMain->SetLotState(int) -- absent from TfMain.  TU-local
            //  no-op macro, identical to csystem.cpp:2874 W7C2_FMAIN_SETLOTSTATE and
            //  Automation/SCK_ART_Remainder.cpp:316 W5SCKARTREM_FMAIN_SETLOTSTATE.
            W906ART_FMAIN_SETLOTSTATE(4);                                              //ART RT Start
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20220923 : add SPIL
            {
                ShowMyMessage("(1)Test summary lot end finished, please key in EAP GUI to do next process.\r\n(2)After put the IC on the handler loader and then press handler Start button to start test",
                            "(1)測試報表已結檔, 請操作EAP GUI 繼續下一步流程.\r\n(2)handler完成上料後按Start鍵 ,開始測試.");
            }
            Task=300;
            break;
        case 300:                                                               //jou 2015-10-02 Auto Retest GPIB mode
            if(LastSet.bWaitStartLotAutoRetestGPIB==true ||
               (fMain->hanaART->IsHanaArtAvailable()==true &&
                W906ART_HANAART->IsContactAvailable()==true) ||
               (CosFunction.iAutoRetestTCPmode==2 &&                            //Rogeryang 20250918 : 瑞薩FT-CT
                TestIF_File.bRENESAS_EnableFTCT==true))
            {
                if(CUSTOMER_CODE==CC_PTI && IniConfig.bB03_TesterReport)        //Sam 20240809 : PTI ART 模式
                {
                    //不要變更
                }
                else
                {
                    fLotInfo->cbRunMode->Text="RT";
                }

                if(CUSTOMER_CODE==CC_AMKOR_Korea)                               //Steven 20231214 : #R231026-ATK-H9-02 , V3.21.808.7 , Request Count Clear when ART from FT to RT.
                {
                    fMain->Clarn_Data(0, "Auto Retest");
                }

                fMain->Clarn_Data(7, "WaitStartLotAutoRetestGPIB Sorting Count");
                bResult=true;
                Task=400;
            }
            break;
    }

    if(bResult==true)                                                           //Steven 20170118 : For ART Tray Arm Speed
    {
        SetMotorScaleSpeed(MTrayX, ArmSpeed[TrayArm].iBodySP);
    }

    return bResult;
}
//---------------------------------------------------------------------------
bool DoTrayArm_AutoRT(bool bReset)
{
    bool bResult=false;
    bool ret;
    int &Task=iTrayArm_AutoRT_Task;
    static int Pos;

    if(bReset==true)
    {
        Task=1;
        bTrayArmOK_AutoRT=false;
        bTrayArmPlaceToLoadFinish=false;
        DoPlaceToLoader_AutoRT(true);                                           //ChungHung 20140815 add AutoRetest

        for(int i=eAuto1; i<=iAutoRight; i++)
        {
            DoPickFromAuto_AutoRT(i, true);
            for(int j=0; j<2; j++)
                iArt_AutoTotal[i][j]=0;                                         //kevin 20170830 (Steven) 記錄ART Auto Tray 數量
        }
        return bResult;
    }

    switch(Task)
    {
        case 1:
            if(MOT[MTrayX].fHasTray)
            {
                bTrayArmPlaceToLoadFinish=false;
                Task=300;
            }
            else if((bAutoOK_AutoRT[eAuto1]==false ||
                     bAutoOK_AutoRT[eAuto2]==false ||
                     bAutoOK_AutoRT[eAuto3]==false ||
                     bAutoOK_AutoRT[eAuto4]==false ||
                     bAutoOK_AutoRT[eAuto5]==false ||
                     bAutoOK_AutoRT[eAuto6]==false) &&
                     MOT[MTrayX].fHasTray==false)
            {
                bTrayArmPlaceToLoadFinish=false;
                Task=100;
            }
            else                                                                //做完了
            {
                bTrayArmPlaceToLoadFinish=true;
                Task=400;
            }
            break;
        case 100:
            Pos=-1;
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(bAutoOK_AutoRT[i]==false && MOT[iMMAuto[i]].fHasTray)
                {
                    Pos=i;
                    break;                                                      //跳出迴圈
                }
            }

            if(Pos!=-1)
            {
                DoPickFromAuto_AutoRT(Pos, true);
                Task=200;
            }
            else
            {
                Task=1;
            }
            break;
        case 200:
            ret=DoPickFromAuto_AutoRT(Pos);
            if(ret)
            {
                if(Pos==0)                                                      //Steven 20220331 : Tray Arm動作加上紀錄
                {
                    NewRecordProcess("MES0653", "Tray Arm - Pick tray from Auto 1", "DoTrayArm_AutoRT");
                }
                else if(Pos==1)
                {
                    NewRecordProcess("MES0654", "Tray Arm - Pick tray from Auto 2", "DoTrayArm_AutoRT");
                }
                else
                {
                    NewRecordProcess("MES0655", "Tray Arm - Pick tray from Auto 3", "DoTrayArm_AutoRT");
                }
                iArt_AutoTotal[Pos][0]++;                                       //kevin 20170830 (Steven) 記錄ART Auto Tray 數量
                DoPlaceToLoader_AutoRT(true);
                Task=1;
            }
            break;
        case 300:
            ret=DoPlaceToLoader_AutoRT();
            if(ret)
            {
                bTrayArmPlaceToLoadFinish=true;
                Task=1;
            }
            break;
        case 400:
            ret=TrayArmMotorMove(Prod.iXTrayEmpty);
            if(ret)
            {
                bTrayArmPlaceToLoadFinish=true;
                bResult=true;
                Task=1;
            }
            break;
    }
    return bResult;
}
//---------------------------------------------------------------------------
TQPF_Timer DoPickFromAuto_AutoRTDelay[MAX_AUTO_TRAY];
//---------------------------------------------------------------------------
bool DoPickFromAuto_AutoRT(int Pos, bool bReset)
{
    bool bResult=false;
    static bool bDuplicateErr[MAX_AUTO_TRAY]={false};
    static bool bC_Auto_Fixer_Off[MAX_AUTO_TRAY]={false};
    bool ret;
    int AlarmRet;
    int &Task=iTrayArm_PickFromAuto_AutoRT_Task;

    if(bReset==true)
    {
        bDuplicateErr[Pos]=false;
        iRetestNoIC[Pos]=0;                                                     //wei 20160203 回盤為空盤不計數
        Task=1;
        return bResult;
    }

    switch(Task)
    {
        case 1:
            iARTCatchAutoTrayFinish=1;                                          //ChungHung 20141208 add fix ART catch Tray 與 Auto Tray Send to Rear 互搶
            Task=100;
        case 100:                                                               //先確認Out Arm在安全點
            ret=MoveOutArmXY_ToFix_Tray_Full();
            if(ret)
            {
                DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);
                Cylinder[C_TrayCover].Off();                                    //ChungHung 20140625 先讓蓋子在上
                Task=200;
            }
            break;
        case 200:                                                               //再確認汽缸在上
            ret=Cylinder[C_TrayX_UpDown].Pop();
            if(ret)
            {
                bC_Auto_Fixer_Off[Pos]=false;
                Task=300;
            }
            break;
        case 300:                                                               //移動到Auto 上面
            ret=TrayArmMotorMove(Prod.iXTrayAuto_ART[Pos]);                     //kevin 20170831 (Steven) add ART-offset
            Cylinder[C_AutoEdgePush[Pos]].Off();
            Cylinder[C_AutoUpPress[Pos]].Off();                                 //JerryYang 20190423 新增unloader壓tray
            bC_Auto_Fixer_Off[Pos]=Cylinder[C_AutoSide_Fixer[Pos]].Pop();

            if(ret && bC_Auto_Fixer_Off[Pos])
            {
                if(Cylinder[C_AutoUpPress[Pos]].Enable==true)                   //JerryYang 20190423 新增unloader壓tray
                {
                    DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(3);
                    Task=350;
                }
                else
                {
                    Task=400;
                }

                if(CUSTOMER_CODE==CC_KYEC_LEE &&
                   MOT[iMMAuto[Pos]].Tray.HasRealIC()==false)                   //wei 20160203 回盤為空盤不計數
                    iRetestNoIC[Pos]++;
            }
            break;
        case 350:
            if(Sen[SnAutoUpPress[Pos]].IsOff())                                 //JerryYang 20190423 新增unloader壓tray
            {
                Task=400;
            }

            if(DoPickFromAuto_AutoRTDelay[Pos].Off())
            {
                ShowMyMessage("Unloader up press pop error", "Unloader 上壓汽缸放開異常");
                Cylinder[C_AutoUpPress[Pos]].Off();                             //JerryYang 20190423 新增unloader壓tray
                DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(3);
                break;
            }
            break;
        case 400:                                                               //汽缸向下
            ret=Cylinder[C_TrayX_UpDown].Push();
            if(ret)
            {
                Task=150;                                                       //kevin 20150812
                if(USE_AUTO_RETEST==eartInstall)                                //ChungHung 20140625 add AutoRetest catch Tray use 2 Output
                    C_CatchTray_Fix_Puch(true);
                if(IniConfig.bSPILFunction &&
                   IniConfig.bC03UseCatchTray &&
                   IniConfig.bA10_AutoReTest)                                   //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
                {
                    DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT+1.5);
                }
                else
                {
                    DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);
                }
            }
            break;
        case 150:                                                               //kevin 20150812 上蓋下來
            if(DoPickFromAuto_AutoRTDelay[Pos].Off()==false)
                break;

            Task=600;                                                           //ChungHung 20140626 先夾在蓋蓋子
            if(USE_AUTO_RETEST==eartInstall)                                    //ChungHung 20140625 add AutoRetest catch Tray use 2 Output
                C_CatchTray_Fix_Puch(true);
            DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT*2);
            break;
        case 600:                                                               //夾起來
            if(DoPickFromAuto_AutoRTDelay[Pos].Off()==false)
                break;

            if(LastSet.iRealDummy==DUMMY)                                       //Steven 20100618
            {
                ret=true;
            }
            else //if(IniConfig.bC03UseCatchTray)                               //use catch Tray Fix
            {
                if(USE_AUTO_RETEST==eartInstall)                                //ChungHung 20140625 add AutoRetest catch Tray use 2 Output
                    ret=C_CatchTray_Fix_Puch();
                else
                    ret=Cylinder[C_CatchTray_Fix].Push();

                if(ret)
                {
                    if(LastSet.iRealDummy!=DUMMY && Sen[SnCatchTrayFix1On].IsOn()==true && Sen[SnCatchTrayFix2On].IsOn()==true)
                    {
                        Task=700;
                        break;
                    }
                }
            }

            if(ret)
            {
                Task=650;
            }
            break;
        case 650:                                                               //Steven 20190314 : Fix ART catch auto tray hang up
            MOT[MTrayX].fHasTray=true;
            MOT[iMMAuto[Pos]].ClearTray(__FUNC__);
            CatchTraySuck.iWhichTray=3+Pos;                                     //Steven 20220331 : 紀錄Tray從哪來的
            CatchTraySuck.SetItemData(0, 0, 2);
            DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(ArmSpeed[TrayArm].dVacuumTI);
            if(CosFunction.bThickTrayNoNeedCover &&
               UserDefForm[0].ZDepth>1500)                                      //Steven 20200723 : 太厚的Tray不能蓋蓋子
            {
            }
            else
            {
                Cylinder[C_TrayCover].On();                                     //kevin 20150812
            }
            Task=500;                                                           //ChungHung 20140626 先夾在蓋蓋子
            break;
        case 500:                                                               //蓋蓋子
            #ifdef SOFT_SIMULTE
            Task=1000;
            break;
            #else
            if(DoPickFromAuto_AutoRTDelay[Pos].Off()==false)
                break;

            if(LastSet.iRealDummy==DUMMY)
            {
                ret=true;
            }
            else
            {
                if(CosFunction.bThickTrayNoNeedCover &&
                   UserDefForm[0].ZDepth>1500)                                  //Steven 20200723 : 太厚的Tray不能蓋蓋子
                {
                }
                else
                {
                    Cylinder[C_TrayCover].On();                                 //kevin 20150812
                }
                Task=900;                                                       //kevin 20150812 //ChungHung 20140626 先夾在蓋蓋子
            }
            break;
            #endif
        case 900:                                                               //上升
            if(DoPickFromAuto_AutoRTDelay[Pos].Off())
            {
                ret=Cylinder[C_TrayX_UpDown].Pop();
                if(ret)
                {
                    Task=1000;
                }
            }
            break;
        case 1000:                                                              //結束
            iARTCatchAutoTrayFinish=2;                                          //ChungHung 20141208 add fix ART catch Tray 與 Auto Tray Send to Rear 互搶
            bDuplicateErr[Pos]=false;
            bResult=true;
            break;

//例外處理----------------------------------
        case 700:                                                               //夾取錯誤先將氣缸升上來
            Cylinder[C_TrayX_UpDown].Off();
            DoPickFromAuto_AutoRTDelay[Pos].SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT*2);
            Task=750;
        case 750:                                                               //Steven 20190314 : Fix ART catch auto tray hang up
            if(DoPickFromAuto_AutoRTDelay[Pos].Off())
            {
                if(LastSet.iRealDummy!=DUMMY &&
                   Sen[SnCatchTrayFix1On].IsOn()==true &&
                   Sen[SnCatchTrayFix2On].IsOn()==true)
                {
                    Task=800;
                }
                else
                {
                    Task=650;
                }
            }
            break;
        case 800:                                                               //夾取錯誤Alarm
            AlarmRet=ShowErrorMessage("JAM0601", K_SKIP|K_RETRY, MTrayX, bDuplicateErr[Pos], __FUNC__);

            if(AlarmRet==K_RETRY)
            {
                bDuplicateErr[Pos]=true;
                Task=400;
            }
            else if(AlarmRet==K_SKIP)
            {
                bDuplicateErr[Pos]=false;
                Task=900;
                MOT[MTrayX].fHasTray=false;
                MOT[iMMAuto[Pos]].ClearTray(__FUNC__);
            }
            break;
    }

    return bResult;
}
//---------------------------------------------------------------------------
TQPF_Timer DoPlaceToLoader_AutoRTDelay;
TQPF_Timer DoPlaceToLoader_TrayCoverTimeOut;                                    //Frank 20161212 (Jou) Cover Cylinder Time Out
//---------------------------------------------------------------------------
bool DoPlaceToLoader_AutoRT(bool bReset)
{
    bool bResult=false;
    bool ret;
    static bool bDuplicateErr=false;
    int AlarmRet;
    int &Task=iTrayArm_PlaceToLoad_AutoRT_Task;
    AnsiString Str;

    if(bReset==true)
    {
        bDuplicateErr=false;
        Task=1;
        return bResult;
    }

    switch(Task)
    {
        case 1:
            Task=1;
        case 100:                                                               //先確認Iut Arm在安全點
            ret=MoveInArm2XYToWait();
            if(ret)
            {
                Task=200;
            }
            break;
        case 200:                                                               //再確認汽缸在上
            ret=Cylinder[C_TrayX_UpDown].Pop();
            if(ret)
            {
                if(INSTALL_OCR_YMot==eocrYMotInstal &&                          //RogerYang 20260403 : 再次確認台車有到接料位置
                    MOT[MMTrayY].fHasTray==false)
                    Task=250;
                else
                    Task=300;
            }
            break;
        case 250:
            {
                ret=MOT[MLoaderY].MotorMove(Prod.iMLoaderYSurePos);
                if(ret)
                Task=300;
            }
            break;
        case 300:                                                               //移動到Loader 上面
            ret=TrayArmMotorMove(Prod.iXTrayLoad_ART);                          //kevin 20170831 (Steven) add ART
            if(ret)
            {
                                                                                //ChungHung 20140625 add
                Cylinder[C_TrayY_Fixer].Off();
                Cylinder[C_LoaderEdgePush].Off();
                Cylinder[C_LoaderUpPress].Off();                                //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
                Task=400;
            }
            break;
        case 400:                                                               //汽缸向下
            if(MOT[MMTrayY].fHasTray==false &&
               MOT[MMTrayY_Car].fHasTray==false)                                //Sam 20210621 : 退出來時還要一直轉，不然會回車導致翻盤
            {
                ret=Cylinder[C_TrayX_UpDown].Push();
                if(ret)
                {
                    if(USE_AUTO_RETEST==eartInstall)                            //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        C_CatchTray_Fix_Pop(true);
                    if(IniConfig.bSPILFunction &&
                       IniConfig.bC03UseCatchTray &&
                       IniConfig.bA10_AutoReTest)                               //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
                    {
                        DoPlaceToLoader_AutoRTDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT+1.5);   //Steven 20110720 : 等待Hand Down Time
                    }
                    else
                    {
                        DoPlaceToLoader_AutoRTDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);       //Steven 20110720 : 等待Hand Down Time
                    }
                    Task=500;
                }
            }
            break;
        case 500:                                                               //放開夾子

            if(DoPlaceToLoader_AutoRTDelay.Off())
            {
                if(LastSet.iRealDummy==DUMMY)                                   //Steven 20120417 : Dummy不要夾起來
                {
                    ret=true;
                }
                else //if(IniConfig.bC03UseCatchTray)
                {
                    if(USE_AUTO_RETEST==eartInstall)                            //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        ret=C_CatchTray_Fix_Pop();
                    else
                        ret=Cylinder[C_CatchTray_Fix].Pop();
                }

                if(ret)
                {
                    Task=600;
                    DoPlaceToLoader_AutoRTDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);  //Steven 20110720 : 等待Counter Air On Time
                    DoPlaceToLoader_TrayCoverTimeOut.SetSecAndOn(100);          //kevin 20170810 (Steven) 5->100                      //Frank 20161212 (Jou) Cover Cylinder Time Out
                }
            }
            break;
        case 600:          //開蓋子
            if(DoPlaceToLoader_AutoRTDelay.Off())
            {
                if(LastSet.iRealDummy==DUMMY)                                   //Steven 20100618
                {
                    ret=true;
                }
                else
                {
                    Cylinder[C_TrayCover].Off();                                //ChungHung 20140625 open Cover use Off
                    ret=true;                                                   //JerryYang 20170907 (Steven) 這裡不需要判斷
                }

                if(ret)
                {
                    Task=700;
                    DoPlaceToLoader_AutoRTDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);  //Steven 20110720 : 等待Counter Air On Time
                }
            }
            break;
        case 700:                                                               //上升
            if(DoPlaceToLoader_AutoRTDelay.Off())
            {
                ret=Cylinder[C_TrayX_UpDown].Pop();
                if(ret)
                {
                    if(LastSet.iRealDummy!=DUMMY &&
                       Sen[SnLoaderSureTray].IsOff())                           //沒有偵測到放tray
                    {
                        Task=800;
                    }
                    else
                    {
                        Task=900;
                    }
                }
            }
            break;
        case 900:                                                               //結束
            MOT[MTrayX].fHasTray=false;                                         //Steven 20170216 (wei) : Tray Arm資料交換改到case 900
            if(CatchTraySuck.iWhichTray!=-1)
                Str.sprintf("DoPlaceToLoader_AutoRT - From %s", sTrayPosName[CatchTraySuck.iWhichTray]);
            else
                Str.sprintf("DoPlaceToLoader_AutoRT - From Unkonwn");
            NewRecordProcess("MES0670", "Tray Arm - place tray to Loader", Str);//Steven 20220331 : Tray Arm動作加上紀錄
            CatchTraySuck.iWhichTray=-1;                                        //Steven 20220331 : 紀錄Tray從哪來的
            CatchTraySuck.SetItemData(0, 0, NULL_IC);
            MOT[MMTrayY].SetTray(HAS_IC, __FUNC__);
            bDuplicateErr=false;
            bResult=true;
            break;
//例外處理-------------------------------------
        case 800:                                                               //放到Loader上檢查Sensor錯誤
            AlarmRet=ShowErrorMessage("JAM0911", K_RETRY|K_SKIP, MMTrayY, bDuplicateErr, __FUNC__);

            if(AlarmRet==K_RETRY)
            {
                Task=700;
                bDuplicateErr=true;
            }
            else if(AlarmRet==K_SKIP)
            {
                bDuplicateErr=false;
                Task=900;
            }
            break;
    }

    return bResult;
}
//---------------------------------------------------------------------------
bool DoAuto_AutoRT(int Pos, bool bReset)                                        //ChungHung 20141002 add for KYEC AutoRetest
{
    bool ret;
    bool bResult=false;
    int &Task=iAuto_AutoRT_Task[Pos];

    int bRet=0;                                                                 //kevin 20150613
    int iAutoTray[MAX_AUTO_TRAY]={0, 0, 0, 0, 0, 0};
    iAutoTray[eAuto2]=atoi(fMain->edtAuto2->Text.c_str());
    iAutoTray[eAuto3]=atoi(fMain->edtAuto3->Text.c_str());
    iAutoTray[eAuto4]=atoi(fMain->edtAuto4->Text.c_str());
    iAutoTray[eAuto5]=atoi(fMain->edtAuto5->Text.c_str());
    iAutoTray[eAuto6]=atoi(fMain->edtAuto6->Text.c_str());
    AnsiString Str1, Str2;
    static bool bflag[MAX_AUTO_TRAY]={false, false, false, false, false, false};//JerryYang 20220923 : add
    int ibuffer=0;
    bool bFlag=false;

    if(bReset==true)
    {
        Task=1;
        return bResult;
    }

    switch(Task)
    {
        case 1:
            if(Prod.bART6Tray[Pos]==false)                                      //是否有設定為需要Auto Retest
            {
                bResult=true;
            }
            else
            {
                #ifdef SOFT_SIMULTE                                             //ChungHung 20141002 add for KYEC AutoRetest
                if(iAutoTray[Pos]<=0 && CosFunction.bUseSCKART==false)
                {
                    if(MOT[iMMAuto[Pos]].fHasTray==false && MOT[iMMAuto_Car[Pos]].fHasTray==false)
                        bResult=true;
                    else if(MOT[iMMAuto[Pos]].fHasTray==false && MOT[iMMAuto_Car[Pos]].fHasTray==true)
                        Task=100;
                    else
                        bResult=false;
                }
                else
                {
                    if(MOT[iMMAuto[Pos]].fHasTray==true || MOT[iMMAuto_Car[Pos]].fHasTray==true)
                        break;
                    else
                        bResult=true;
                }
                #else
                if(LastSet.iRealDummy==DUMMY)
                {
                    //AI(W906-ART) 20260807: golden fMain->CheckBox1 (TCheckBox*, golden main.h:361).
                    //  forms/fMain.h:600-607 records it as DELIBERATELY NOT ADDED because "its ONLY
                    //  reference in the whole W7-L1 family is asendic_Loader.cpp:2942, which sits
                    //  inside `#ifdef SOFT_SIMULTE`".  THAT IS NO LONGER TRUE: this site and golden
                    //  :1111 are both LIVE (inside the `#else` / real-machine arm).  Stood in
                    //  TU-locally here because forms/fMain.h is outside this unit`s write scope --
                    //  REPORTED so the facade gains the member and this seam is retired.
                    if(W906ART_FMAIN_CHECKBOX1->Checked==false &&
                       Sen[SnAutoTrayHasTray[Pos]].IsOn()==false &&
                       Sen[SnAutoTrayCar[Pos]].IsOn()    ==false &&
                       Sen[SnAutoTrayDetect[Pos]].IsOn() ==false &&
                       MOT[iMMAuto[Pos]].fHasTray        ==false &&
                       MOT[iMMAuto_Car[Pos]].fHasTray    ==false)
                    {
                        bResult=true;
                    }
                    else
                    {
                        Task=100;
                    }
                }
                else
                {
                    if(Sen[SnAutoTrayHasTray[Pos]].IsOn()==false &&
                       Sen[SnAutoTrayCar[Pos]].IsOn()    ==false &&
                       Sen[SnAutoTrayDetect[Pos]].IsOn() ==false &&
                       MOT[iMMAuto[Pos]].fHasTray        ==false &&
                       MOT[iMMAuto_Car[Pos]].fHasTray    ==false &&
                       (iAutoTrayCount[Pos]<=0 ||
                        CosFunction.bUseSCKART==true))                          //JerryYang 20220923 : add
                    {
                        bResult=true;
                    }
                    else
                    {
                        if(Cylinder[C_TrayX_UpDown].OffSensor() &&              //ChungHung 20140625 add
                           MOT[iMMAuto[Pos]].fHasTray==false &&
                           Sen[SnAutoTrayDetect[Pos]].IsOn())                   //Auto紀錄沒有Tray Auto偵測有Tray
                        {
                            ShowErrorMessage(sJAM1110[Pos], K_RETRY, iMMAuto[Pos]);
                            break;
                        }

                        if(CUSTOMER_CODE==CC_KYEC_LEE)
                        {
                            if(Sen[SnAutoTrayHasTray[Pos]].IsOn()==false &&
                               Sen[SnAutoTrayCar[Pos]].IsOn()    ==false &&
                               Sen[SnAutoTrayDetect[Pos]].IsOn() ==false &&
                               MOT[iMMAuto[Pos]].fHasTray        ==false &&
                               MOT[iMMAuto_Car[Pos]].fHasTray    ==false )
                            {
                                bResult=true;
                            }
                            else if(iAutoTrayCount[Pos]>0)
                            {
                                Task=100;
                            }
                            else if(Sen[SnAutoTrayHasTray[Pos]].IsOn())
                            {
                                Str1.sprintf("Auto %d Tray Over Count ,Please remove Auto %d Up Tray",Pos+1,Pos+1);
                                Str2.sprintf("Auto %d Tray超過數量,請將 Auto %d Tray盤移走",Pos+1,Pos+1);
                                ShowMyMessage(Str1, Str2);
                            }
                        }
                        else
                        {
                            if(iAutoTrayCount[Pos]>0 ||
                               CosFunction.bUseSCKART==true)                    //Steven 20170825 (wei) : Fixed for SCK ART hang up when unloader have tray
                            {
                                Task=100;
                            }
                            else if(iAutoTrayCount[Pos]==0)                     //kevin 20170824 (Steven) 有空tray
                            {
                                break;
                            }
                            else if(Sen[SnAutoTrayHasTray[Pos]].IsOn())
                            {
                                Str1.sprintf("Auto %d Tray Over Count ,Please remove Auto %d Up Tray",Pos+1,Pos+1);
                                Str2.sprintf("Auto %d Tray超過數量,請將 Auto %d Tray盤移走",Pos+1,Pos+1);
                                ShowMyMessage(Str1, Str2);
                            }
                        }
                    }
                }
                #endif
            }
            break;
        case 100:                                                               //確認要做哪個動作
            #ifdef SOFT_SIMULTE                                                 //ChungHung 20141002 add for KYEC AutoRetest
            if(CosFunction.bUseSCKART==false)
            {
                if(Pos==1)
                {
                    if(MOT[iMMAuto_Car[Pos]].fHasTray==false)
                    {
                        fMain->edtAuto2->Text=iAutoTray[1]-1;
                        MOT[iMMAuto_Car[Pos]].SetTray(HAS_IC, __FUNC__);
                    }
                }
                else if(Pos==2)
                {
                    if(MOT[iMMAuto_Car[Pos]].fHasTray==false)
                    {
                        fMain->edtAuto3->Text=iAutoTray[2]-1;
                        MOT[iMMAuto_Car[Pos]].SetTray(HAS_IC, __FUNC__);
                    }
                }
                else if(Pos==3)
                {
                    if(MOT[iMMAuto_Car[Pos]].fHasTray==false)
                    {
                        fMain->edtAuto4->Text=iAutoTray[3]-1;
                        MOT[iMMAuto_Car[Pos]].SetTray(HAS_IC, __FUNC__);
                    }
                }
                else if(Pos==4)
                {
                    if(MOT[iMMAuto_Car[Pos]].fHasTray==false)
                    {
                        fMain->edtAuto5->Text=iAutoTray[4]-1;
                        MOT[iMMAuto_Car[Pos]].SetTray(HAS_IC, __FUNC__);
                    }
                }
                else if(Pos==5)
                {
                    if(MOT[iMMAuto_Car[Pos]].fHasTray==false)
                    {
                        fMain->edtAuto6->Text=iAutoTray[5]-1;
                        MOT[iMMAuto_Car[Pos]].SetTray(HAS_IC, __FUNC__);
                    }
                }
            }
            #endif

            if(LastSet.iRealDummy==DUMMY)
            {
                if(MOT[iMMAuto[Pos]].fHasTray==false && MOT[iMMAuto_Car[Pos]].fHasTray==true)
                {
                    InitAutoTrayToRearTask(Pos);
                    Task=200;
                }
                else if(MOT[iMMAuto_Car[Pos]].fHasTray==false && W906ART_FMAIN_CHECKBOX1->Checked)
                {
                    InitLoadNewAutoTrayToCarTask(Pos);
                    Task=300;
                }
                else
                {
                    Task=400;
                }
            }
            else
            {
                if(MOT[iMMAuto[Pos]].fHasTray==false &&
                   Sen[SnAutoTrayDetect[Pos]].IsOn()==false &&
                   iARTCatchAutoTrayFinish!=1 &&                                //ChungHung 20141208 add fix ART catch Tray 與 Auto Tray Send to Rear 互搶
                   (MOT[iMMAuto_Car[Pos]].fHasTray==true ||
                    Sen[SnAutoTrayCar[Pos]].IsOn()))                            //用||是為了亂槍打鳥, 避免錯誤
                {
                    InitAutoTrayToRearTask(Pos);
                    Task=200;
                }
                else if(MOT[iMMAuto_Car[Pos]].fHasTray==false &&
                        Sen[SnAutoTrayCar[Pos]].IsOn()==false &&
                        Sen[SnAutoTrayHasTray[Pos]].IsOn())
                {
                    InitLoadNewAutoTrayToCarTask(Pos);
                    Task=300;
                }
                else
                {
                    #ifdef SOFT_SIMULTE
                        InitAutoTrayToRearTask(Pos);
                        Task=200;
                    #else
                        Task=400;
                    #endif
                }
            }
            break;
        case 200:                                                               //入Tray到機台裡面
            ret=DoAutoTrayToRear(Pos);
            if(ret)
            {
                iAutoTrayCount[Pos]--;
                if(IniConfig.bP37bAutoCylinderUP)                               //JerryYang 20220923 : add for P37
                {
                    Task=210;
                }
                else
                {
                    Task=400;
                }
            }
            break;
        case 210:
            if(IniConfig.bP37bAutoCylinderUP)
            {
                Task=211;
                bflag[Pos]=false;                                               //升到接盤位置
                AutoCylinderLower(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);   //Steven 20140409 : AutoCylinderXX Add Reset
                AutoCylinderMiddle(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);  //升到接盤位置
                AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);      //升到接盤位置
            }
            break;
        case 211:
            if(Sen[SnAutoTrackDetect[Pos]].IsOn() ||
               Sen[SnAutoTrayCar[Pos]].IsOn())                                  //軌道上有tray
            {
                bflag[Pos]=true;                                                //kevin 20180726 Auto 12 3 up
                bFlag=true;
            }

            if(bFlag)
                Task=212;
            else
                Task=213;
            break;
        case 212:
            if(bflag[Pos])                                                      //kevin 20180726 Auto 12 3 up
            {
                ibuffer++;
                if(AutoCylinderMiddle(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))  //升到接盤位置
                {
                    Cylinder[C_AutoZ_Select[Pos]].On();
                    ibuffer=0;
                    bflag[Pos]=false;
                }
            }

            if(ibuffer==0)
                Task=213;
            break;
        case 213:
            if(AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))       //kevin 20190723 change  //升到接盤位置
                Task=400;
            break;
        case 300:                                                               //把Tray分離到下方
            bRet=DoLoadNewEmptyTrayToCar_RT(Pos);                               //kevin 20150613
            if(bRet==1)
            {
                Task=400;
            }
            else if(bRet==2)
            {
                Task=1;
                bResult=true;
            }
            break;
        case 400:                                                               //結束
            Task=1;
            break;
    }

    return bResult;
}
//---------------------------------------------------------------------------
bool DoLoader_AutoRT(bool bReset)
{
    bool ret;
    bool bResult=false;
    int AlarmRet=0;
    int &Task=iLoader_AutoRT_Task;

    if(bReset==true)
    {
        Task=1;
        iCountReceiveTray_ART=0;                                                //kevin 20150613 計算收回tray
        return bResult;
    }

    switch(Task)
    {
        case 1:
            if(bTrayArmOK_AutoRT==true &&
               bAutoOK_AutoRT[eAuto1]==true &&
               bAutoOK_AutoRT[eAuto2]==true &&
               bAutoOK_AutoRT[eAuto3]==true &&
               bAutoOK_AutoRT[eAuto4]==true &&
               bAutoOK_AutoRT[eAuto5]==true &&
               bAutoOK_AutoRT[eAuto6]==true &&
               Sen[SnLoaderCarHasTray].IsOn()==false &&
               Sen[SnLoaderSureTray].IsOn()  ==false &&
               MOT[MMTrayY].fHasTray         ==false &&
               MOT[MMTrayY_Car].fHasTray     ==false)
            {
                bResult=true;
            }
            else
            {
                Task=100;
            }
            break;
        case 100:  //確認要做哪個動作
            if(LastSet.iRealDummy==DUMMY)
            {
                if(MOT[MMTrayY].fHasTray     ==false &&
                   MOT[MMTrayY_Car].fHasTray ==false)
                {
                    Task=500;
                }
                else if(MOT[MMTrayY_Car].fHasTray)
                {
                    InitUnLoadNewLoaderTrayTask();
                    Task=200;
                }
                else if(MOT[MMTrayY].fHasTray)
                {
                    InitLoaderTrayToFrontTask();
                    LastSet.iInputLoaderCount++;                                //ChungHung 20141002 add for KYEC AutoRetest
                    Task=400;
                }
                else
                {
                    Task=500;
                }
            }
            else
            {
                if(Sen[SnLoaderCarHasTray].IsOn()==false &&
                   Sen[SnLoaderSureTray].IsOn()  ==false &&
                   MOT[MMTrayY].fHasTray         ==false &&
                   MOT[MMTrayY_Car].fHasTray     ==false)
                {
                    Task=500;
                }
                else if(MOT[MMTrayY_Car].fHasTray ||
                        Sen[SnLoaderCarHasTray].IsOn())                         //ChungHung 20140815 modify
                {
                    if(MOT[MMTrayY_Car].fHasTray &&
                       Sen[SnLoaderCarHasTray].IsOff()==false)
                    {
                        InitUnLoadNewLoaderTrayTask();
                        Task=200;
                    }
                    else if(MOT[MMTrayY_Car].fHasTray &&
                            Sen[SnLoaderCarHasTray].IsOff())
                    {
                        AlarmRet=ShowErrorMessage("WAR0915", K_RETRY|K_SKIP, MMTrayY_Car, 0, __FUNC__);
                        if(AlarmRet==K_SKIP)
                        {
                            MOT[MMTrayY_Car].fHasTray=false;
                            Task=1;
                        }
                    }
                }
                else if(MOT[MMTrayY].fHasTray || Sen[SnLoaderSureTray].IsOn())  //ChungHung 20140815 modify
                {
                    if(MOT[MMTrayY].fHasTray && Sen[SnLoaderSureTray].IsOff()==false)
                    {
                        InitLoaderTrayToFrontTask();
                        LastSet.iInputLoaderCount++;                            //ChungHung 20141002 add for KYEC AutoRetest

                        if(CosFunction.bUseSCKART &&                            //Steven 20161214 (wei) : For SCK ART
                           bAutoOK_AutoRT[eAuto1]==true  &&
                           bAutoOK_AutoRT[eAuto2]==true  &&
                           bAutoOK_AutoRT[eAuto3]==true  &&
                           bAutoOK_AutoRT[eAuto4]==true  &&
                           bAutoOK_AutoRT[eAuto5]==true  &&
                           bAutoOK_AutoRT[eAuto6]==true  &&
                           bTrayArmPlaceToLoadFinish==true)                     //Steven 20170216 (wei) : 最後一盤不用退
                        {
                            if(CUSTOMER_CODE==CC_KYEC_LEE) LastSet.iLoaderTrayCount_ART++;     //Eastsun 20260515 F022: D3
                            bResult=true;
                            Task=1;
                        }
                        else
                        {
                            Task=400;
                        }
                    }
                    else if(MOT[MMTrayY].fHasTray && Sen[SnLoaderSureTray].IsOff())
                    {
                        AlarmRet=ShowErrorMessage("WAR0914", K_RETRY|K_SKIP, MMTrayY, 0, __FUNC__);
                        if(AlarmRet==K_SKIP)
                        {
                            MOT[MMTrayY].fHasTray=false;
                            Task=1;
                        }
                    }
                }
                else
                {
                    Task=500;
                }
            }
            break;
        case 200:                                                               //把Tray送到上方
            ret=DoUnLoadNewLoaderToStack(false);
            if(ret)
            {
                MOT[MMTrayY_Car].ClearTray(__FUNC__);
                Task=300;
            }
            break;
        case 300:                                                               //確認有沒有滿Tray
            ret=Sen[SnLoaderIsFull].IsOn();
            if(ret)
            {
                ShowErrorMessage("MES0921", 0, MMTrayZ);
            }
            else
            {
                Task=500;
            }
            break;
        case 400:                                                               //退Tray到機台外面
            ret=DoLoaderTrayToFront();
            if(ret)
            {
                Task=500;
            }
            break;
        case 500:                                                               //結束
            Task=1;
            break;
    }

    return bResult;
}
//---------------------------------------------------------------------------
//extern void SetRunStartMode(eRunStartMode Mode=rsmNull, AnsiString ModeText="");//Steven 20120615 : 改成可用序號(Mode)或名稱(ModeText)進行模式改變
//---------------------------------------------------------------------------
int GPIB_RemoteCommand(AnsiString asCommand)                                    //jou 2015-09-21 Auto Retest function
{
    unsigned char HCACK=1;
    AnsiString S=asCommand, S2, S3;
    int iPos1,iPos2;
    RecordProcess(S);

    if(S.Pos("RCMD:PAUSE")!=0)                                                  //Steven 20190116 : Add GPIB下暫停
    {
        fMain->Pause("RCMD:PAUSE");
        HCACK=0;
    }
    else if(S.Pos("RCMD:S10F3")!=0)                                             //Steven 20190116 : Add GPIB下顯示Message
    {
        S3=S.SubString(11, S.Length());
        //AI(W906-ART) 20260807: golden fMain->tGPIBMsg (TStringList*).  Not on the TfMain facade;
        //  TU-local REAL vclcompat::TStringList so the message text is actually retained and a
        //  test can read it back (same "real storage, not a sink" idiom as fMain->tTestResult).
        W906ART_FMAIN_TGPIBMSG->Add(S3);
        HCACK=0;
    }
    else if(S.Pos("RCMD:TESTER_ERROR")!=0)                                      //Frank 20170601 (Steven) add TESTER ERROR COMMAND
    {
        iPos1=S.Pos(":");
        iPos2=S.Pos(";");
        S2=S.SubString(iPos1+1, iPos2-iPos1-1);

        if(S2!="")
        {
            S3.sprintf("TESTER ERROR : %s", S2);
            W906ART_FMAIN_TGPIBMSG->Add(S3);
            HCACK=0;
        }
        else
        {
            HCACK=1;
        }
    }
    //以下的在機台運行中不可以動作!!----------------------------------------
    else if(SystemStart==true)
    {
        HCACK=2;
    }
    else if(S.Pos("RCMD:INITIAL_START_ART;")!=0)
    {
        if(HasICUnderMachine()==false)
        {
            SetRunStartMode(rsmInitial_ART);
            HCACK=0;
        }
        else
        {
            HCACK=2;
        }
    }
    else if(S.Pos("RCMD:DOWNLOAD_RECIPE_BY_FTP")!=0)
    {
        //AI(W906-ART) 20260807: golden fFTPClient (TfFTPClient*, golden FTPClient.h:207).  The
        //  KYECFTP translation wave deliberately DEMOTED that singleton -- no `fFTPClient->`
        //  surface exists anywhere in this tree (see AutoClean/AutoClean.cpp:835-842 for the same
        //  finding) -- so the three members golden touches here are carried on a TU-local seam
        //  declared above.  Control flow is preserved exactly, including the
        //  `fMain->cbSetupFileName->Text==S2` verdict two lines down (REAL facade member).
        if(HasICUnderMachine()==false && W906ART_FTPCLIENT->bControlBySECSGEM==false)
        {
            iPos1=S.Pos(",");
            iPos2=S.Pos(";");
            S2=S.SubString(iPos1+1, iPos2-iPos1-1);

            W906ART_FTPCLIENT->bControlBySECSGEM=true;
            W906ART_FTPCLIENT->aSetUpNameBySECSGEM = S2;
            W906ART_FTPCLIENT->ShowFTPModal(0);

            if(fMain->cbSetupFileName->Text==S2)
                HCACK=0;
            else
                HCACK=1;
        }
        else if(W906ART_FTPCLIENT->bControlBySECSGEM==true)
        {
            HCACK=1;
        }
        else
        {
            HCACK=2;
        }
    }

    return HCACK;
}
//---------------------------------------------------------------------------
AnsiString GPIB_QueryData(AnsiString asCommand)                                 //jou 2015-09-21 Auto Retest function
{
    int i=0, iTotalPassCT=0;
    AnsiString S=asCommand, S2, S3;
    AnsiString asResult="SVID:";
    AnsiString asString;

    if(S.Pos("\r\n")!=0)
    {
        S=S.SubString(1, S.Pos("\r\n")-1);
    }

    if(S.Pos("SVID:")!=0)
    {
        S2=S.SubString(S.Pos(":")+1, S.Length());
        do
        {
            if(S2.Pos(";")==0)                                                  //Jimmychiu 20241219 : fixed for tester command send no ；
            {
                S3=S2;
            }
            else
            {
                S3=S2.SubString(1, S2.Pos(";")-1);
            }

            if(S3=="1160")
            {
                for(i=0; i<eTrayCount; i++)
                {
                    if(Prod.iTrayType[i]==tNotUse)
                        continue;
                    if(Prod.iIsPassT6[i]==1)                                    //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6 //Steven 20240701 : 0 --> 1
                        iTotalPassCT+=LastSet.BinCT[0][iTo3Unload[i]];
                }
                asString=AnsiString(iTotalPassCT);
            }
            else if(S3=="35915")                                                //wei 20160824 Barcode GPIB      //Alick 2017018(jou) Remark for Remote Wakeup Barcode
            {
                if(BAR_CODE_INSTALL==2 || BAR_CODE_INSTALL==3)                  //Alick 20170202(jou) add use CCD
                    asString="1";
                else
                    asString="0";
            }
            else if(S3=="1011")                                                 //Frank 20170505 (Steven) : Add handler status for Xilinx ART
            {
                asString=AnsiString(fMain->palMainStatus->Caption);
            }
            else
            {
                //AI(W906-ART) 20260807: golden `HGemPtr->GetECDataValue(S3)`.  NOT a gate and NOT a
                //  stub: golden THGem::GetECDataValue (golden uHGemEquipment.cpp:3650-3925) IS
                //  translated in this tree -- the W906-SvEcDataItem wave extracted it into the
                //  SecsSvEcRegistration engine that THGem now embeds BY VALUE as `SvEcReg`
                //  (SECSGEM/uHGemEquipment.h:158, SECSGEM/SecsSvEcRegistration.h:213).  So the only
                //  change is the extra `SvEcReg.` hop to the relocated member; same function, same
                //  body, same return.  HAZARD KEPT GOLDEN-FAITHFUL: HSys.MyGem is NULL until
                //  SystemModularInitial() runs (database.h:294-297), and golden does not null-guard
                //  here either, so this line dereferences NULL if the SVID branch is reached before
                //  the GEM stack is wired.  Not "fixed" -- see this file`s banner.
                asString=HSys.MyGem->HGemPtr->SvEcReg.GetECDataValue(S3);
            }

            asResult=asResult+asString+";";
            S2=S2.SubString(S2.Pos(";")+1, S2.Length());
            if(S2.Pos(";")==0)                                                  //Jimmychiu 20241219 : fixed for tester command send no ；
            {
                break;
            }
        }while(S2.Length()>0);
    }
    return asResult;
}
//---------------------------------------------------------------------------
int GPIB_SetData(AnsiString asCommand)                                          //jou 2015-09-21 Auto Retest function
{
    int i=0,j=0;
    int ret, TempMode;
    AnsiString S=asCommand, S2, S3, S4, S5;
    AnsiString asResult="ECID:";
    AnsiString asString;
    char cStr[256];
    bool bFlag;
    unsigned char HCACK=1;

    if(S.Pos("\r\n")!=0)
    {
        S=S.SubString(1,S.Pos("\r\n")-1);
    }

    if(S.Pos("ECID:")!=0)
    {
        S2=S.SubString(S.Pos(":")+1, S.Length());
        do
        {
            S3=S2.SubString(1, S2.Pos(",")-1);
            S4=S2.SubString(S2.Pos(",")+1, S2.Length());
            S5=S4.SubString(1, S4.Pos(";")-1);

            if(S3=="1530")
            {
                strcpy(cStr, S5.c_str());
                for(i=0; i<MAX_SOCKET_ROW; i++)                                 //kevin 20141015 fix
                {
                    for(j=0; j<MAX_SOCKET_COL; j++)
                    {
                        if(TestIF.iSiteMap[i][j]>0)
                        {
                            if(cStr[TestIF.iSiteMap[i][j]-1]=='0')
                                bFlag=false;
                            else
                                bFlag=true;
                        }
                        else
                        {
                            bFlag=false;
                        }

                        if(bFlag==false)
                        {
                            bTestSiteUse[0][i][j]=bFlag;
                            bTestSiteUse[1][i][j]=bFlag;
                            LastSet.bUseTestSocket[0][i][j]=bFlag;
                            LastSet.bUseTestSocket[1][i][j]=bFlag;
                        }
                    }
                }

                fMain->ShowTestHeadComp(false);
                HCACK=0;
            }
            else if(S3=="1515" || S3=="1519")
            {
                asString=S5;
                i=atoi(S5.c_str());

                if(i>=20 && i<=150)
                {
                    if(i>25)
                        TempMode=Tempture_Hot;
                    else
                        TempMode=Tempture_Ambient;

                    fMain->edWorkTemperBase->Text=i;
                    ret=fMain->SetTemp(false, atof(fMain->edWorkTemperBase->Text.c_str()), atof(fMain->edSoakTime->Text.c_str()));

                    if(ret==0)
                    {
                        //AI(W906-ART) 20260807: golden fMain->ChangeTempMode(int,bool,bool).
                        //  Not on the TfMain facade; TU-local stand-in returns 0 (golden success),
                        //  same shape as Automation/auto9045.cpp:218.
                        ret=W906ART_FMAIN_CHANGETEMPMODE(TempMode, false, bRefreshFunction);
                        if(ret==0)
                        {
                            if(i>25)
                                LastSetTemperature_NET=Tempture_Hot;
                            else
                                LastSetTemperature_NET=Tempture_Ambient;
                            Temperature_NET.fWorkTemperBase=i;
                            HCACK=0;
                        }
                    }
                }
            }
            else if(S3=="1516" || S3=="1520")
            {
                asString=S5;
                i=atoi(S5.c_str());
                if(i>=0 && i<=999)
                {
                    fMain->edSoakTime->Text=i;
                    ret=fMain->SetTemp(false, atof(fMain->edWorkTemperBase->Text.c_str()), atof(fMain->edSoakTime->Text.c_str()));     //Steven 20120730
                    if(ret==0)
                    {
                        Temperature_NET.fSoakTime=i;
                        HCACK=0;
                    }
                }
            }
            else if(S3=="35916")                                                //wei 20160824 Barcode GPIB
            {
                i=atoi(S5.c_str());
                if(BAR_CODE_INSTALL==2 || BAR_CODE_INSTALL==3)
                {
                    if(i==0)
                    {
                        TestIF_File.bEnableBarCode=false;
                    }
                    else
                    {
                        TestIF_File.bEnableBarCode=true;
                    }
                    //==> Eastsun 20260526 #026-4.PinN.P-N9 Pin1 barcode MSG_CMD :KYEC
                    //AI(W906-ART) 20260807: the two MSG_CMD_*BarCode constants on the next
                    //  if/else pair come from golden MessageDef.h, which has no translated home;
                    //  both values are mirrored TU-locally above (golden MessageDef.cpp:47-48 =
                    //  31/32).  fMain->SendMSG_CMD(int) itself is REAL (forms/fMain.h:234).
                    //  Comment kept ABOVE the `if` so the unbraced arms stay visibly attached.
                    if((TestIF_File.bEnableBarCode && TestIF_File.b2DUsePinInspection==false))
                        fMain->SendMSG_CMD(W906ART_MSG_CMD_EnableBarCode);
                    else
                        fMain->SendMSG_CMD(W906ART_MSG_CMD_DisableBarCode);
                    //<== Eastsun 20260526 #026-4.PinN.P-N9

                    AnsiString S="";
                    S=GetLastOpenFN();
                    AnsiString szDir="";
                    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
                    WriteIniData(szDir, "Configuration", "Bar Code", TestIF_File.bEnableBarCode);
                    HCACK=0;
                }
            }
            else if(S3=="37228")                                                //wei 20160824 Barcode GPIB
            {
                asString=S5;
                if(CUSTOMER_CODE==CC_KYEC_XILINX)
                {
                    if(asString.Length()==9)                                    //Alick 20170124(jou) add for "37228"小於九碼
                    {
                        DeleteFile(asBarCodeDownLot);
                        WriteDataToFile(asBarCodeDownLot, asString.c_str());
                        HCACK=0;
                    }
                    else
                    {
                        DeleteFile(asBarCodeDownLot);
                    }
                }
                else
                {
                    DeleteFile(asBarCodeDownLot);
                    WriteDataToFile(asBarCodeDownLot, asString.c_str());
                    HCACK=0;
                }
            }
            S2=S4.SubString(S4.Pos(";")+1, S4.Length());
        }while(S2.Length()>0);
    }
    return HCACK;
}
//---------------------------------------------------------------------------
