//---------------------------------------------------------------------------
//  atester_32Site.cpp  --  32-Site dual-arm test-head SM (W5 translation)
//
//  Translation wave: W5-atester32site (biggest single unit in this batch)
//  Translator: AI(W5-atester32site-Translate) 20260710
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/atester_32Site.cpp
//                 (4749 lines, cp950) + atester_32Site.h (17 lines)
//
//  WHAT THIS FILE IS
//  -----------------
//  The 32-Site (4x8, dual test-head arm) variant of the tester/index engine:
//  when a machine is configured for the 32-site kit, IniConfig.bD59_32SitePnpTogether
//  routes DoTestY_TwoArm32Site's case 210/221 through THIS file's
//  Do32SiteTestDestroyIC()/Do32SiteTestSuckIC() (both arms suck/destroy together)
//  instead of the single-arm aTester_Front.cpp/aTester_Rear.cpp pair.
//
//  W5 SCOPE
//  --------
//  ACTIVE, faithfully transcribed (cursor values + formulas + fall-throughs
//  VERBATIM, structurally identical to the already-proven atester.cpp /
//  aTester_Front.cpp / aTester_Rear.cpp translations of the same idiom family):
//    * DoInterFaceErrorStep_TwoArm32Site  (golden :70-179)
//    * Do32RTCAutoModelVerify             (golden :184-681) -- EXCEPT the vision
//      protocol cases 12500-14000, see GATED below.
//    * InitTestYTwoArm32SiteTask          (golden :685-688)
//    * DoTestY_TwoArm32Site                (golden :693-2291) -- EXCEPT cases
//      55-68, see GATED below.
//    * InitTestSuckTestIC_TwoArm32Site_Task (golden :2293-2296)
//    * DoTestSuckTestIC_TwoArm32Site        (golden :2300-3138)
//    * Do32SiteTestDestroyIC                (golden :3141-3684)
//    * Do32SiteTestSuckIC                   (golden :3689-4749)
//
//  GATED (#if 0 // TODO(W5-atester32site-vision)) with an ACTIVE default-safe
//  stub, per the two explicitly-named islands:
//    1. DoTestY_TwoArm32Site cases 55,60,61,62,64,65,66,67,68 (golden :801-1011,
//       ~211 lines) -- the RTC-full-view-verify + fContact ROI-learning hand-off
//       cluster (the SAME COM2/fContact surface as atester.cpp's own gated
//       DoAllPassVerifyRTC/DoHalfView*VerifyRTC leaves).  Gated as ONE block;
//       replaced with a conservative default that neither fakes a vision-pass
//       nor a vision-fail -- it simply cannot run this check offline, so it
//       parks the cursor at the block entry (case 55) rather than fabricate an
//       outcome.  NOTE: this differs from the surrounding cases' seam-based
//       translation (see "DISCREPANCY" note below the includes) precisely
//       because THIS cluster's whole purpose is the vision decision itself.
//    2. Do32RTCAutoModelVerify cases 12500-14000 (golden :364-679, ~316 lines)
//       -- the RTC real-time-vision handshake (COM2->SendCommToVision /
//       bRealTimeCom_ReceiveOK / OpenRTCComPortAgain / InitRealTimeCCDPara).
//       Gated as ONE block; parks at the block entry (case 12500) rather than
//       fabricate a verify-pass/fail.  Cases 1/100/200/12010-12400 (motor
//       moves + the DoAllPassVerifyRTC call, already itself gated in
//       atester.cpp) stay ACTIVE.
//
//  DISCREPANCY vs the unit-spec's initial characterization (golden wins; see
//  CLAUDE.md "report discrepancy" convention) -- READ BEFORE JUDGING THE TWO
//  GATES ABOVE AS "TOO NARROW":
//    The unit spec described "DoTestY_TwoArm32Site cases ~55-68" as one of two
//    blocked islands.  Recon (and this translator's own re-measurement) is
//    correct that this specific span is where the RTC-vision DECISION is made
//    (fContact->InitROILearningTask/Do_ROILearning hand-off).  But the SAME
//    COM2 members (bRealTimeCom_ReceiveOK[]/rtFullTOK/rtFullTNG/RTC_ResetAlarm/
//    SendCommToVision/OpenRTCComPortAgain) are ALSO referenced OUTSIDE that
//    span -- at cases 120, 225, 226, 2000, all still inside DoTestY_TwoArm32Site
//    -- purely as `if(REAL_TIME_CCD==true && COM2->...)` SHORT-CIRCUIT guards
//    with a real, already-present, non-vision "else" path (the every-day
//    behaviour for a machine with no RTC camera wired -- see KNOWLEDGE.md's own
//    "本機關閉(出範圍): ... AOI-CCD ..." precedent).  atester.cpp's OWN T1 wave
//    solved this EXACT surface gap (identical member names) via a TU-local
//    "W7T1_SEAM" COM2 extension (all no-op/false defaults) rather than gating
//    those call sites -- see atester.cpp:1504-1642.  This file mirrors that
//    established, already-reviewed precedent (below, "W5_32S_SEAM") for cases
//    120/225/226/2000/3072 so they compile + run FAITHFULLY (the RTC branch is
//    provably unreachable given the seam's fixed false/true defaults, so
//    nothing is fabricated -- it is the SAME "no RTC hardware" default state
//    every other translated engine assumes).  The two vision-DECISION spans
//    named above (where the code's only job is the vision verify/fail
//    itself, with no non-vision alternative to fall back on) are gated exactly
//    as the unit spec instructed.  Net effect: LOWER risk than a blanket gate
//    of all COM2 touches, HIGHER fidelity than gating everything COM2-shaped,
//    and zero pass-fabrication anywhere.  Flagged prominently per instructions
//    for a human/integrate-agent to override if this judgement call is wrong.
//
//  Beyond the two vision islands, several smaller substrate gaps were
//  discovered while translating (same "#if 0 // TODO(W7)" convention already
//  used pervasively in atester.cpp / aTester_Front.cpp / aTester_Rear.cpp /
//  csystem.cpp for a foreign shared class that is missing ONE method/member and
//  cannot be extended by this unit -- see the "Hard boundary" rule). Each is
//  cited at its call site AND summarised in the W5_32S_SEAM block below:
//    * FTestSuck/BTestSuck.bNeedCheck[][]        -- TMyKitSuck, no such member
//      (SAME gap as aTester_Front.cpp's W64B_NEEDCHECK_GET/SET; mirrored here).
//    * FTestSuck/BTestSuck/FRCarryKit/BRCarryKit.MoveSuckData(...)  -- TMyKitSuck,
//      no such method (SAME gap as aTester_Front.cpp gate #3).
//    * FRCarryKit/BRCarryKit.SetHasNullIcToNullIc()  -- TMyKitSuck, no such
//      method (SAME gap as aTester_Front.cpp gate #9 / csystem.cpp's
//      W7C2_BLCARRYKIT_SETHASNULLIC).
//    * TMyProductionRecord::AddIndexPlaceShuttleRecord / AddIndexPickVacuum --
//      not declared anywhere (SAME gap as aTester_Front.cpp gate #4).
//    * TMyProductionRecord::AddTestResultRecord / AddIndexCycleTimeRecord --
//      DECLARED (Public/MyProductionRecord.h) but have NO compiled body
//      anywhere in the tree (that header is a declaration-only forward-stub);
//      calling them would be an undefined-reference LINK error, so every call
//      site is gated (not just a "no such declaration" case).
//    * LastSet.iIndexInputOutPut[]                -- not in the LAST_GENERAL_SET
//      minimal shim (canary_support.h) yet (SAME gap as aTester_Front.cpp
//      gate #6).
//    * fiosetview->ProcessIndexSuckDestroy1/2(int) / ->ResetIndexSuck()  --
//      TfiosetviewShim exposes bIndexSuck[][][] but not these pump methods
//      (SAME documented gap atester.cpp/aTester_Front.cpp already gate via
//      W7T1_FIOSET_PISD1/2 / W64B_FIOSET_PISD1; mirrored here for both arms).
//    * fContact->InitROILearningTask() / ->Do_ROILearning(bool) overload --
//      TfContactShim (atester_shims.h) exposes only Do_ROILearning() [no-arg]
//      (SAME gap atester.cpp gates via W7T1_FCONTACT_DOROI/INITROI).
//    * TestSocket.MergeSuck/MergeSuck1Row(BTestSuck,FTestSuck,bool) and
//      .SplitSuck/SplitSuck1Row(BTestSuck,FTestSuck,bool)  -- TMyKitSuck, no
//      such methods anywhere in the tree yet.  NEW gap (not previously
//      catalogued by a sibling wave) -- see W5_32S_SEAM.  Real behavioural
//      consequence: TestSocket's per-site test-result grid is not folded back
//      out to FTestSuck/BTestSuck (Split) nor refreshed in from them (Merge)
//      offline.  Flagged prominently in the final report; not a cosmetic gap.
//    * ATC_InterfaceForm->SendHandler2DID(int,bool)  -- TATC_InterfaceFormShim
//      (acarry_shims.h) exposes only iATC_MODE_TYPE.  NEW gap; call-site macro.
//    * MyMessageBox->lblChineseMsg->Font->Size        -- TMyMessageBoxShim
//      (acatchtray_shims.h) exposes Visible/fShow/Close() only.  Pure font-size
//      cosmetic toggle around an already-working ShowMyMessage() call; the two
//      font-size lines are gated, ShowMyMessage() itself stays ACTIVE.
//    * fMain->chkReadTorque1/2 / edTorue0 / lbArm0Torque / tInitSoakTimer /
//      SendMSG_CMD / CloseGpibProgram  -- TfMain (FormsFacade.h) lacks these
//      (SAME gap atester.cpp gates via its own W7T1_TfMainTorqueSeam for the
//      Checked/Text/Caption widgets; SendMSG_CMD/CloseGpibProgram/tInitSoakTimer
//      are NEW call-site macros, same convention).
//    * fAutoTeach->SetIndexBinError(int) / fNote->fShow  -- no translated home
//      for either form (golden AutoTeach.h / note.h).  Call-site macros
//      (no-op / false), matching csystem.cpp's own W7C2_FAUTOTEACH_ISRUN()
//      precedent for the identical fAutoTeach gap.
//    * SetHangupMaxTime() / RecordEndTestTime(int) / SendTestResultToHttp()  --
//      free functions with NO declaration anywhere in the translated tree
//      (golden itself forward-declares SendTestResultToHttp LOCALLY in this
//      same .cpp, :2299).  Defined here as TU-local `static` stand-ins (offline
//      no-op / "OK") to avoid any cross-unit ODR risk -- flagged in the report.
//
//  TRANSLATION RULES (mirrors atester.cpp/aTester_Front.cpp/aTester_Rear.cpp)
//  --------------------------------------------------------------------------
//    * Off Borland: __fastcall removed (none in this file); AnsiString/
//      TQPF_Timer via vclcompat; numeric semantics EXACT (no int/float rewrites).
//    * ASE_KaohSiung / INDEX_PROTECT_TMOVE / DEBUG_HANGUP_NO_HOME /
//      TEST_BIN_MISS_SIMULATE / SOFT_SIMULTE are NOT defined in this target
//      build (MachineType.h keeps every golden toggle commented out the same
//      way) -- every `#ifdef`/`#else`/`#endif` block using these is reproduced
//      VERBATIM (both branches, letting the SAME MachineType.h decide, exactly
//      as golden does); the preprocessor selects the `#else`/undefined branch
//      today, same as every other translated engine file.
//    * Big5 Chinese comments decoded via cp950, preserved as UTF-8. ZERO U+FFFD.
//---------------------------------------------------------------------------
#include "MachineDefine.h"

#include "atester.h"
#include "atester_32Site.h"
#include "atester_shims.h"          // 32Site/ProcessCount/fContact/ADAM_* offline shims + this file's OWN cursor ints (i32RTCAutoModelVerifyTask/iTestSuckTestIC_TwoArm32Site_Task/iTestTwoArm32SiteTask -- see integrate note below)
#include "aTester_Front.h"          // iFrontTestDestroyICTask / iFrontTestSuckICTask (shared cursors -- golden quirk, see below) / DoFrontTestSuckIC (still an atester_shims stub) / TestZ1SetPos
#include "aTester_Rear.h"           // iRearTestDestroyICTask / DoRearTestSuckIC (atester_shims stub) / TestZ2SetPos

#include "aArmHeader.h"             // __FUNC__ shim
#include "MachineType.h"            // MAX_SOCKET_ROW/COL, NN_2Row/NN_1Row, eSpeedPart::IndexArm
#include "Motor/mymotor.h"          // MOT[], TTrayMotor, RecordIndexPositionError, ShowIndexMotorError
#include "mysensor.h"                // Sen[]
#include "myswitch.h"                // SW[]
#include "csystem.h"                // state predicates (InShtInLF/InSHT1InRT/OutSHT1InLF/CheckIndexAllSuckICFallDown/IsInArmOneCycleFinish/IsInArmCleanOutFinish/CheckContactOver/ShowIndexTime/TemperatureStorageLog/CheckIndexSuckICFallDownSetToHasNullIC)
#include "cprod.h"                  // Prod / TestIF / TestIF_File / CosFunction / DeviceForm / DeviceForm_File / ArmSpeed / ArmSpeed_File / Temperature
#include "cmydef.h"                 // global scalar universe + IC consts + enums + IndexSuckName / bTestSiteUse / bTestSiteNeedSetErrBin
#include "cpublic.h"                // COM2 (base minimal shim; extended below via seam)
#include "common.h"
#include "aHotPlateSubstrate.h"     // FTestSuck/BTestSuck/FLCarryKit/BLCarryKit/FRCarryKit/BRCarryKit/TestSocket/InArmSuck
#include "FormsFacade.h"            // fMain / fSCKART offline stand-ins
#include "acarry_shims.h"           // ATC_InterfaceForm (golden ATC_Handler_Side.h, offline shim)
#include "acatchtray_shims.h"       // MyMessageBox (golden mymessbox.h, offline shim)
#include "canary_support.h"         // LastSet / ShowMyMessage / ShowErrorMessage / RecordProcess
//---------------------------------------------------------------------------

// ===========================================================================
//  W5_32S_SEAM -- TU-local stand-ins for substrate gaps discovered while
//  translating (see file-header banner for the full rationale of each).  Every
//  stand-in mirrors an ALREADY-ESTABLISHED precedent elsewhere in this tree
//  (atester.cpp's W7T1_SEAM / aTester_Front.cpp's W64B_* / csystem.cpp's
//  W7C1_/W7C2_* seams) for the IDENTICAL golden member surface -- none of this
//  is a new design, it is the same "TU-local offline-safe stand-in, #define
//  redirect at the call site" convention applied to the same gaps a sibling
//  wave already hit.  TODO(W7 / integrate): once the real substrate gains
//  these members, delete this block and the call sites bind unchanged.
// ===========================================================================
#ifndef W5_32S_SEAM
#define W5_32S_SEAM

// ---- FTestSuck/BTestSuck.bNeedCheck[][] (golden MyKitSuck.h) --------------
//  SAME gap as aTester_Front.cpp's W64B_NEEDCHECK_GET/SET.  Reads default
//  false / writes no-op (mirrors IniConfig.bD44CheckIndexICDestroy's own
//  zero-initialized default -- the D44 recheck sub-branches are inert offline).
#define W5_32S_NEEDCHECK_GET(kit,i,j)      ((kit).bNeedCheck[i][j])   //AI(W906-IDXSUCK) 20260927: 以前恆 false；A4-6 之後 TMyKitSuck 是 golden 佈局（mykitsuck.h:303 bNeedCheck），同 aTester_Front／Rear 的 W2-A46（0926）
#define W5_32S_NEEDCHECK_SET(kit,i,j,v)    ((kit).bNeedCheck[i][j]=(v))   //AI(W906-IDXSUCK) 20260927: 以前是空的 do{}while(0)

// ---- fiosetview->ProcessIndexSuckDestroy1/2([iType]) / ->ResetIndexSuck() --
//  SAME documented gap as atester.cpp's W7T1_FIOSET_PISD1/2 (no-arg) --
//  extended here with the int-arg overload this file ALSO calls (golden
//  ProcessIndexSuckDestroy1(1) etc.).  Offline: suck self-check "done" (true)
//  so the D44/negative-pressure recheck SM advances instead of looping
//  forever on a DAQ that does not exist offline.
static bool W5_32S_ProcessIndexSuckDestroy1(int iType=0){ return fiosetview->ProcessIndexSuckDestroy1(iType); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）   // golden iosetview.h:3018
static bool W5_32S_ProcessIndexSuckDestroy2(int iType=0){ return fiosetview->ProcessIndexSuckDestroy2(iType); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）   // golden iosetview.h
#define W5_32S_FIOSET_RESET()          fiosetview->ResetIndexSuck()            // golden fiosetview->ResetIndexSuck()   //AI(W906-IDXSUCK) 20260927: 以前是空的 do{}while(0)，TfiosetviewShim 現在有 golden 照翻的本體（atester_shims.cpp 檔尾）
#define W5_32S_FIOSET_PISD1(iType)     W5_32S_ProcessIndexSuckDestroy1(iType)
#define W5_32S_FIOSET_PISD2(iType)     W5_32S_ProcessIndexSuckDestroy2(iType)

// ---- fContact ROI-learning hand-off (golden cContact.h) -------------------
//  SAME gap as atester.cpp's W7T1_FCONTACT_DOROI/INITROI.  Only reachable
//  inside the gated vision-decision islands (see file banner); kept here so
//  the SAME macro name/shape is available if a future wave needs it outside
//  those islands too.
#define W5_32S_FCONTACT_INITROI()      do{}while(0)                            // golden fContact->InitROILearningTask()

// ---- FRCarryKit/BRCarryKit.SetHasNullIcToNullIc() (golden MyKitSuck.h) ----
//  SAME gap as aTester_Front.cpp gate #9 / csystem.cpp's
//  W7C2_BLCARRYKIT_SETHASNULLIC.  Offline no-op.
#define W5_32S_SETHASNULLIC(kit)       do{}while(0)

// ---- *.MoveSuckData(TMyKitSuck&,int,int) (golden MyKitSuck.h) -------------
//  SAME gap as aTester_Front.cpp gate #3.  Offline no-op -- KNOWN real
//  behavioural gap (Item/iBinData/etc. do not carry over between the test
//  grid and the carry-kit grid offline); see file-header banner.
#define W5_32S_MOVESUCKDATA(dst,src,i,j)   do{}while(0)

// ---- TMyProductionRecord telemetry methods either undeclared or declared- --
//  only-no-body (see file banner).  Offline no-op (pure logging, no state read
//  back by any SM).
#define W5_32S_ADD_INDEX_PLACE_SHUTTLE_RECORD(kit,i,j,a1,a2,a3)  do{}while(0)  // golden .AddIndexPlaceShuttleRecord(site,row,col)
#define W5_32S_ADD_INDEX_PICK_VACUUM(kit,i,j,val)                ((void)(val), (kit).PordRec[i][j].AddIndexPickVacuum((kit).Suck[i][j].GetIOValue()))  // golden .AddIndexPickVacuum(iValue) -- AI(W906-W2-A46) 20260926: 不再是空巨集 —— TMySucker::GetIOValue（mykitsuck.h:237）與 AddIndexPickVacuum（MyProductionRecord.h:188）都在，照 golden 用 GetIOValue()；第 4 個參數保留不用
#define W5_32S_ADD_TEST_RESULT_RECORD(kit,i,j,bin,sbin,tag)      do{}while(0)  // golden .AddTestResultRecord(bin,sbin,tag) -- DECLARED, no body anywhere -> would be undefined-reference

// ---- LastSet.iIndexInputOutPut[] (golden LastSet.h) -----------------------
//  Not in the LAST_GENERAL_SET minimal shim (canary_support.h) yet.  SAME gap
//  as aTester_Front.cpp gate #6.  Offline no-op (pure counter, not read back).
#define W5_32S_LASTSET_INDEXINOUT_INC(idx)   do{}while(0)

// ---- TestSocket.MergeSuck/MergeSuck1Row / .SplitSuck/SplitSuck1Row --------
//  (golden MyKitSuck.h)  NEW gap (no TMyKitSuck method anywhere in the tree).
//  Offline no-op.  KNOWN real behavioural gap: the test-result grid is not
//  folded out to / refreshed in from FTestSuck/BTestSuck offline -- see the
//  file-header "DISCREPANCY"-adjacent banner paragraph on this exact gate.
#define W5_32S_MERGESUCK(dst,a,b,flag)     do{}while(0)   // golden .MergeSuck(BTestSuck,FTestSuck,bool) / .MergeSuck1Row(BTestSuck,FTestSuck)
#define W5_32S_SPLITSUCK(dst,a,b,flag)     do{}while(0)   // golden .SplitSuck(BTestSuck,FTestSuck,bool) / .SplitSuck1Row(BTestSuck,FTestSuck)

// ---- ATC_InterfaceForm->SendHandler2DID(int,bool) (golden ATC_Handler_Side.h)
//  TATC_InterfaceFormShim (acarry_shims.h) exposes only iATC_MODE_TYPE.  NEW
//  gap; offline no-op (no ATC link offline; matches the shim's own
//  iATC_MODE_TYPE=0 "no ATC" posture).
#define W5_32S_ATC_SENDHANDLER2DID(a,b)    do{}while(0)

// ---- fMain torque-read UI widgets (golden main.h) -------------------------
//  SAME gap as atester.cpp's W7T1_TfMainTorqueSeam; extended with the two
//  method calls (SendMSG_CMD/CloseGpibProgram) + the soak-timer object
//  (tInitSoakTimer) this file ALSO derefs.  Reads/writes are widget-shaped
//  (Checked/Text/Caption) but never consumed by any ACTIVE decision here (the
//  torque-read cluster is reached only when IniConfig.bD01EnableReadTorque is
//  on AND, for the ASE_KaohSiung branch, that customer macro is compiled out).
//  AI(W906-W7-F2) 20260729: W5_32S_TCheckSeam / W5_32S_TEditSeam / W5_32S_TLabelSeam
//  RETIRED -- vclcompat/Controls.h now owns the unified stock-widget stand-ins
//  (plan D4), so the three file-scope objects name those types directly.  Golden
//  classes re-read from golden main.h for this change: chkReadTorque1/2 :464-465
//  TCheckBox, edTorue0 :466 TEdit, lbArm0Torque :798 **TPanel** (the retired
//  W5_32S_TLabelSeam name was misleading -- golden's lbArm0Torque is a TPanel, not a
//  TLabel).  Zero behaviour change: same single member, same default ("" / false),
//  and these four `static` objects are the only instances (no by-value copy, no
//  aggregate initialisation), so the unified types' vtable disturbs nothing.
//  W5_32S_TTimerSeam is NOT retired: it stands in for a TQPF_Timer, not a widget, and
//  has no unified equivalent in Controls.h.
struct W5_32S_TTimerSeam { void LatchCycleTimeSec(bool){} };                    // golden main.h TfMain->tInitSoakTimer (TQPF_Timer-shaped; only this method touched)
static TCheckBox          W5_32S_chkReadTorque1, W5_32S_chkReadTorque2;         // golden main.h:464-465 (TCheckBox*)
static TEdit              W5_32S_edTorue0;                                      // golden main.h:466 (TEdit*)
static TPanel             W5_32S_lbArm0Torque;                                  // golden main.h:798 (TPanel*)
static W5_32S_TTimerSeam  W5_32S_tInitSoakTimer;
#define W5_32S_FMAIN_CHKREADTORQUE1   (&W5_32S_chkReadTorque1)
#define W5_32S_FMAIN_CHKREADTORQUE2   (&W5_32S_chkReadTorque2)
#define W5_32S_FMAIN_EDTORUE0         (&W5_32S_edTorue0)
#define W5_32S_FMAIN_LBARM0TORQUE     (&W5_32S_lbArm0Torque)
#define W5_32S_FMAIN_TINITSOAKTIMER   (W5_32S_tInitSoakTimer)
#define W5_32S_FMAIN_SENDMSG_CMD(cmd)     do{}while(0)   // golden fMain->SendMSG_CMD(int) -- offline no-op (no real GPIB LOG side channel)
#define W5_32S_FMAIN_CLOSEGPIBPROGRAM(f)  do{}while(0)   // golden fMain->CloseGpibProgram(AnsiString) -- offline no-op

// ---- fAutoTeach->SetIndexBinError(int) (golden AutoTeach.h) ---------------
//  No translated home for fAutoTeach anywhere (SAME gap as csystem.cpp's own
//  W7C2_FAUTOTEACH_ISRUN() for the identical form).  Offline no-op.
#define W5_32S_FAUTOTEACH_SETINDEXBINERROR(v)  do{}while(0)

// ---- fAutomation->TestMode (golden automation.h) --------------------------
//  TfAutomationShim (atester_shims.h) exposes DoCommandBuffer/GetEventNum/
//  GetEventResult/GetEventStrResult/ClearEvent but no TestMode data member.
//  NEW gap.  Only read on the CUSTOMER_CODE==CC_MTI/CC_PTI branch (DoStartMode/
//  DoEndMode's own mode==0 case returns true trivially) -- offline stand-in 0.
#define W5_32S_FAUTOMATION_TESTMODE   (0)

// ---- fNote->fShow (golden note.h, alarm form visibility) ------------------
//  No translated home for fNote.  Offline: no alarm dialog is ever "showing"
//  -> false (so the guarded `if(fNote->fShow) break;` never short-circuits,
//  matching the default "no alarm currently displayed" state).
#define W5_32S_FNOTE_FSHOW   (false)

// ---- TestSocket.iBinDataBackUp[][] (golden MyKitSuck.h) -------------------
//  TMyKitSuck (aHotPlateSubstrate.h) has iBinData[][] but no backup-copy
//  member.  NEW gap; only read/written on the CC_TERAPOWER customer's D22
//  multi-double-contact branch (niche).  Offline no-op (the backup copy is
//  never read back within THIS file; ProcessCount's own consumption of it, if
//  any, is out of scope here).
#define W5_32S_BINDATA_BACKUP(i,j)   do{}while(0)   // golden TestSocket.iBinDataBackUp[i][j]=TestSocket.iBinData[i][j]

// ---- AMR.NeedAMRTransport() (golden Automation/AMR.h) ---------------------
//  DISCOVERED SYMBOL COLLISION (flagged for integrate/cross-unit attention):
//  the tree has TWO different `extern ... AMR;` globals with the SAME name
//  but DIFFERENT types -- acatchtray_shims.h:171 `extern TAMRShim AMR;`
//  (SupplyCover only) vs Automation/AMR.h:65 `extern class TTeraPowerAMR AMR;`
//  (the real NeedAMRTransport()/CheckLoaderCount()/etc., proven by
//  tests/test_amr.cpp).  This file needs MyMessageBox (acatchtray_shims.h) AND
//  AMR.NeedAMRTransport() (Automation/AMR.h) -- but the two headers cannot
//  both be included (conflicting `AMR` redeclaration with a different type).
//  Resolved here by NOT including Automation/AMR.h and gating the ONE call
//  site instead; offline default: false (no AMR transport pending -> matches
//  the "TeraPower AMR" niche-customer-feature-off default already assumed
//  elsewhere in this tree for uninstalled optional hardware).
#define W5_32S_AMR_NEEDTRANSPORT()   (false)

// ---- hFTestTimeOutDelay (golden aTester_Front.cpp:99 / aTester_Front.h:19) -
//  Real golden home is aTester_Front.cpp (TQPF_Timer, plain data, no
//  behavioural gap) -- but that file's CURRENT translated scope (W64b Wave 1,
//  DoFrontTestDestroyIC only) does not declare/define it yet (golden line 99
//  is outside golden :309-865).  TU-local `static` TQPF_Timer + `#define`
//  redirect here avoids any risk of a duplicate-definition landmine when
//  aTester_Front.cpp's scope widens and adds the real global -- flagged in
//  the final report for the integrate step to reconcile (move the real
//  definition to aTester_Front.cpp, delete this stand-in, both TUs then share
//  the one real timer object exactly as golden intends).
extern TQPF_Timer hFTestTimeOutDelay;   //AI(W906-GB-P2b) 20260926: the real golden global (aTester_Front.cpp:3085), armed by SetTestTimeOutTimer
// (was a #define redirect to a TU-local stand-in that nothing armed: On-Line immediate test time-out on 32-site)

// ---- COM2 RTC real-time-vision + torque members (golden rs232.h) ---------
//  SAME documented gap + SAME resolution as atester.cpp's own W7T1_TCOM2Ext
//  (atester.cpp:1604-1640) -- this file needs a slightly larger member set
//  (rtRelease/rtInspEnd/rtOPENVERIFYOK/rtOPENVERIFYNG/rtCLOSEVERIFY/
//  rtALLFAILOK/rtALLFAILNG/rtAlarHasIC/rtArmFinish on top of rtFullTOK/
//  rtFullTNG) because the OUTER (non-gated) cases of DoTestY_TwoArm32Site
//  reference more of the RT-channel enum than atester.cpp's own down-press
//  tree does.  All channels default "never received" (false); bCCDDummyRum
//  stays true (== the base TCOM2Shim); RTC_ResetAlarm/SendCommToVision/
//  InitRealTimeCCDPara/InitReadTorueTask no-op; OpenRTCComPortAgain false.
//  Net effect: EVERY `if(REAL_TIME_CCD==true && COM2->...)` guard in this file
//  takes its already-present, real "no RTC hardware" else-branch -- nothing is
//  fabricated, this is the same default posture atester.cpp's own seam
//  establishes for the identical member family.
//  #define COM2 redirects EVERY COM2 use in THIS TU only (does not touch
//  atester.cpp's own TU-local redirect, nor the base TCOM2Shim other TUs see).
// ===========================================================================
enum { W5_32S_rtRelease=0, W5_32S_rtInspEnd=1, W5_32S_rtOPENVERIFYOK=2, W5_32S_rtOPENVERIFYNG=3,
       W5_32S_rtALLFAILOK=4, W5_32S_rtALLFAILNG=5, W5_32S_rtCLOSEVERIFY=6, W5_32S_rtFullTOK=7,
       W5_32S_rtFullTNG=8, W5_32S_rtAlarHasIC=9, W5_32S_rtArmFinish=10, W5_32S_RT_N=16 };
struct W5_32S_TCOM2Ext
{
    bool bCCDDummyRum;                                          // golden rs232.h:157 -- offline true (== base TCOM2Shim)
    bool bGetValue, bGetValue1;                                  // golden rs232.h -- offline false (torque-read cluster; #ifdef ASE_KaohSiung anyway)
    bool bRealTimeCom_ReceiveOK[W5_32S_RT_N];                    // golden rs232.h -- offline all false ("vision never responds")
    int  rtRelease, rtInspEnd, rtOPENVERIFYOK, rtOPENVERIFYNG, rtALLFAILOK, rtALLFAILNG,
         rtCLOSEVERIFY, rtFullTOK, rtFullTNG, rtAlarHasIC, rtArmFinish;
    void DoReleaseAndInspEnd(){}                                 // golden rs232.h:160 -- offline no-op (== base TCOM2Shim)
    void SendCommToVision(int /*ch*/, bool /*b*/){}              // golden rs232.h -- offline no-op
    bool OpenRTCComPortAgain(){ return false; }                  // golden rs232.h -- offline: no re-open
    void RTC_ResetAlarm(){}                                      // golden rs232.h -- offline no-op
    void InitRealTimeCCDPara(){}                                 // golden rs232.h -- offline no-op
    void InitReadTorueTask(){}                                   // golden rs232.h -- offline no-op
    void ReadIndexTorqueSetting(int /*iArm*/){}                  // golden rs232.h -- offline no-op (#ifdef ASE_KaohSiung anyway)
    W5_32S_TCOM2Ext():bCCDDummyRum(true), bGetValue(false), bGetValue1(false),
        rtRelease(W5_32S_rtRelease), rtInspEnd(W5_32S_rtInspEnd),
        rtOPENVERIFYOK(W5_32S_rtOPENVERIFYOK), rtOPENVERIFYNG(W5_32S_rtOPENVERIFYNG),
        rtALLFAILOK(W5_32S_rtALLFAILOK), rtALLFAILNG(W5_32S_rtALLFAILNG),
        rtCLOSEVERIFY(W5_32S_rtCLOSEVERIFY), rtFullTOK(W5_32S_rtFullTOK), rtFullTNG(W5_32S_rtFullTNG),
        rtAlarHasIC(W5_32S_rtAlarHasIC), rtArmFinish(W5_32S_rtArmFinish)
    { for(int i=0;i<W5_32S_RT_N;i++) bRealTimeCom_ReceiveOK[i]=false; }
};
static W5_32S_TCOM2Ext W5_32S_com2_ext;
#ifdef COM2
#undef COM2
#endif
#define COM2 (&W5_32S_com2_ext)

// ---- Free functions with NO declaration anywhere in the translated tree ---
//  (golden itself forward-declares SendTestResultToHttp LOCALLY in this same
//  .cpp, golden :2299 -- mirrored here).  TU-local `static` to avoid any
//  cross-unit ODR risk (flagged in the final report).
static void W5_32S_SetHangupMaxTime(){}                          // golden (Wei 20230224) -- offline no-op (hang-time reset helper)
static int  W5_32S_RecordEndTestTime(int /*iWhich*/){ return 1; } // golden (Sam 20201231) -- offline: OK(1), matches golden's own DummyMode==false "no XML retry needed" default
static int  W5_32S_SendTestResultToHttp(){ return 1; }            // golden atester_ProcessCount.cpp -- offline: OK(1) (no HTTP endpoint offline)
#define SetHangupMaxTime            W5_32S_SetHangupMaxTime
#define RecordEndTestTime           W5_32S_RecordEndTestTime
#define SendTestResultToHttp        W5_32S_SendTestResultToHttp

// ---- main-screen UI / encoder-teach telemetry (golden main.h / atester.cpp) -
//  SAME gap + SAME resolution as atester.cpp's own W7T1_SEAM
//  (ShowMainScreenPresure/RecordIndexPosition/EncoderTeachingMaxMinCount,
//  atester.cpp:1532-1539) -- TU-local `static` + `#define` redirect, offline
//  no-op (pure UI/telemetry, never read back by any SM decision).
//  MyDBIProcessNew (golden cMyDB.h:21) has NO compiled body anywhere in the
//  tree yet (cMyDB.cpp is W3/vendor-sqlite deferred) -- TU-local no-op stand-in
//  (SAME pattern as MyDBIProcess, which already has a real stub elsewhere).
static void W5_32S_ShowMainScreenPresure(int /*iArm*/){}
static void W5_32S_RecordIndexPosition(int /*a*/, int /*b*/){}
static void W5_32S_EncoderTeachingMaxMinCount(int /*a*/){}
static void W5_32S_MyDBIProcessNew(AnsiString /*asTable*/, AnsiString /*AlarmCode*/, AnsiString /*S1*/, AnsiString /*S2*/=" "){}
#define ShowMainScreenPresure        W5_32S_ShowMainScreenPresure
#define RecordIndexPosition          W5_32S_RecordIndexPosition
#define EncoderTeachingMaxMinCount   W5_32S_EncoderTeachingMaxMinCount
#define MyDBIProcessNew              W5_32S_MyDBIProcessNew

#endif // W5_32S_SEAM
//---------------------------------------------------------------------------

extern bool bEcho, bExist, bUnderTest;
extern bool bEchoStop;                                                          //ChungHung 20130326 add
extern bool CheckTestSuckICOn(class TMyKitSuck &Ptr, int iR, int iC);
extern void InitInArmTask();
extern bool MoveInArm2XYToWait();
extern bool bHangTimePause;                                                     //Steven 20090827 : Hang Up dectector
extern bool bDoubleContact;
extern int  iTesterBIN[MAX_SOCKET_ROW][MAX_SOCKET_COL];

int iCCDTimeOutCount_32=0;
int iHangupCTArm1_32=0;
//==============================================================================
//  DoInterFaceErrorStep_TwoArm32Site (golden :70-179) -- 32-site twin of
//  atester.cpp's (gated) DoInterFaceErrorStep: on a contact-force-over alarm,
//  bring both index arms up to the test position and hold/release the
//  interface-error state.  ACTIVE, faithful; two small cosmetic/substrate
//  gates (MyMessageBox font-size toggle; ATC_InterfaceForm->SendHandler2DID;
//  TestSocket.SplitSuck/SplitSuck1Row -- see W5_32S_SEAM).
//==============================================================================
bool DoInterFaceErrorStep_TwoArm32Site()                                        //Steven 20140815 : Add For 32 Site
{
    static bool bArm1OK=false, bArm2OK=false;

    int &Task=iDoInterFaceErrorStepTask;
    AnsiString asChinese=AnsiString("下壓次數已經超過設定值，請打開Chamber側門並清潔Socket");       //Steven 20230104 : 直接寫死
    AnsiString asEnglish=AnsiString("Contact over setting # Please Open Chamber Side Door and clean socket");           //ChungHung 20121029 alter 客戶會誤解。

    switch(Task)
    {
        case 1:
            bArm1OK=false;
            bArm2OK=false;
            if(bContactCTOverCHK)
            {
                bContactTimeOverStep=true;
            }
            else
            {
                bNowDoInterFaceErrorStep=true;
            }
            Task=2;
        case 2:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, "DoInterFaceErrorStep_TwoArm32Site 2"))
                Task=100;
            break;
        case 100:
            if(bContactCTOverCHK)
            {
                // golden :99-101 -- MyMessageBox->lblChineseMsg->Font->Size 10/12 cosmetic
                // toggle around ShowMyMessage (TMyMessageBoxShim has no lblChineseMsg
                // label yet, acatchtray_shims.h).  ShowMyMessage itself stays ACTIVE.
#if 0 // TODO(W7-UI) -- golden :99,101 MyMessageBox->lblChineseMsg->Font->Size (cosmetic only)
                MyMessageBox->lblChineseMsg->Font->Size=10;
#endif
                ShowMyMessage(asEnglish, asChinese);                            //jou 2012-06-05
#if 0 // TODO(W7-UI) -- golden :101 MyMessageBox->lblChineseMsg->Font->Size (cosmetic only)
                MyMessageBox->lblChineseMsg->Font->Size=12;
#endif
                Task=150;
            }
            else
            {
                if(bEchoStop==true)                                             //ChungHung 20130326 add
                {
                    bEchoStop=false;
                    ShowMyMessage("Receive ECHOSTOP Form TESTER");
                    Task=200;
                }
                else
                {
                    ProcessTesterTimeOut(0);
                    SetNoiseDelay=false;
                    TestISTimeOut=false;
                    ProcessCount(0);
                    if(TestSocket.UseSiteHasIC())                               //jou 20230828 : 修正index all drop error
                    {
                        if(IsNNMode()==NN_1Row)
                            W5_32S_SPLITSUCK(TestSocket, BTestSuck, FTestSuck, false); // golden TestSocket.SplitSuck1Row(BTestSuck,FTestSuck) -- TMyKitSuck has no such method (see W5_32S_SEAM)
                        else
                            W5_32S_SPLITSUCK(TestSocket, BTestSuck, FTestSuck, false); // golden TestSocket.SplitSuck(BTestSuck,FTestSuck,false) -- Steven 20140815 : Add For 32 Site
                    }
                    RecordHistroy(0);
                    bFinshTest=true;
                    bInitStartDelayNotFinish=true;                              //Ifor 20181220 : add Init Start Delay Time Not Finish
                    bTJControlMode=false;                                       //Ifor 20190328 : add TJ Temp Over Range
                    W5_32S_ATC_SENDHANDLER2DID(0, false);                       // golden ATC_InterfaceForm->SendHandler2DID(0,false) -- no such method yet (see W5_32S_SEAM)
                    SW[SwTesterPower].Off();
                    Task=200;
                }
                break;
            }
            break;
        case 150:
            if(bContactCTOverCHK==false)                                        //已經開門了
            {
                Task=170;
            }
            else
            {
#if 0 // TODO(W7-UI) -- golden :143,145 MyMessageBox->lblChineseMsg->Font->Size (cosmetic only)
                MyMessageBox->lblChineseMsg->Font->Size=10;
#endif
                ShowMyMessage(asEnglish, asChinese);                            //jou 2012-06-05
#if 0 // TODO(W7-UI) -- golden :145 MyMessageBox->lblChineseMsg->Font->Size (cosmetic only)
                MyMessageBox->lblChineseMsg->Font->Size=12;
#endif
            }

            bArm1OK=false;
            bArm2OK=false;
            break;
        case 170:
            if(bArm1OK==false)
                bArm1OK=MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, MOT[MTestZ1].GailSpeed);

            if(bArm2OK==false)
                bArm2OK=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, MOT[MTestZ2].GailSpeed);

            if(bArm1OK && bArm2OK)
            {
                bContactTimeOverStep=false;
                return true;
            }
            break;
        case 200:
            if(bArm1OK==false)
                bArm1OK=MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test, MOT[MTestZ1].GailSpeed);

            if(bArm2OK==false)
                bArm2OK=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test, MOT[MTestZ2].GailSpeed);

            if(bArm1OK && bArm2OK)
            {
                bNowDoInterFaceErrorStep=false;
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer DoTestYFrontDelay_32, DoTestYFrontDelay2_32;

//==============================================================================
//  Do32RTCAutoModelVerify (golden :184-681) -- RTC auto model verify, dual-arm
//  32-site variant.  ACTIVE: cases 1/100/200 (motor safe + Y-split + the call
//  into the already-gated atester.cpp DoAllPassVerifyRTC stub, which always
//  returns false offline -- so this function never actually advances past
//  case 200 in the default/offline build, exactly mirroring the "no RTC
//  hardware" posture atester.cpp's own DoAllPassVerifyRTC gate establishes)
//  and cases 12010-12400 (motor moves + front/rear suck-drop bookkeeping, no
//  vision-comm touch).  GATED per unit spec: cases 12500-14000 (the RTC
//  real-time-vision handshake) -- see file-header banner island #2.
//==============================================================================
int i32RTCAutoModelVerifyTask=1;
// ===========================================================================
//  GOLDEN VERBATIM PAIR -- Do32RTCAutoModelVerify
//  golden atester_32Site.cpp:184-681 (498 golden lines), transcribed CHARACTER-FOR-CHARACTER
//  (Big5/cp950 -> UTF-8 only) and GATED OFF.  The block is INERT: the ACTIVE
//  Do32RTCAutoModelVerify is the 204-line body immediately after the #endif, and that body is
//  UNCHANGED by this pair being here.  Net behaviour change: ZERO.
//
//  WHY: the census scored this function "translated" because a same-named
//  LIVE body exists -- but it never compares SIZE.  That body is 204 lines
//  standing in for 498 golden lines whose text was NOWHERE in this tree.
//  That is LOST TEXT, not deferred behaviour.  Now the text EXISTS and is
//  auditable line-by-line, and a later un-gate is mechanical rather than a
//  re-translation.
//
//  NOTE ON WHAT WAS ACTUALLY MISSING: the live body faithfully carries golden
//  cases 1/100/200/12010-12400 (golden :218-363).  The lost text is golden
//  :364-679 -- cases 12500..14000, the RTC real-time-vision handshake -- for
//  which the live body holds only a one-line `case 12500:` placeholder inside
//  its own pre-existing `#if 0 // TODO(W5-atester32site-vision)` gate.  This
//  pair restores the WHOLE function (:184-681) rather than just that island so
//  a diff of the gate against golden's range is exactly empty.
//
//  NOTHING inside the gate is fixed, renamed, reflowed or reindented --
//  golden's own defects are preserved ON PURPOSE so a diff against golden
//  stays EMPTY (this wave's report lists them with their golden line
//  numbers).  Same shape as this tree's existing pairs: atester.cpp
//  GetTesterResult (golden 1,705 gated / 6-line live) and csystem.cpp
//  MainProc (2,390 / 26), DoTrayFeedProcess (1,235 / 6),
//  CheckContinusStartIsReady (700 / 9), DoAllProcess.  Being gated it needs
//  NO callee to exist -- only lexical validity; no stub, declaration or
//  header edit was added anywhere for its symbols.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden atester_32Site.cpp:184-681.  INERT reference text; the live Do32RTCAutoModelVerify follows the #endif.
bool Do32RTCAutoModelVerify(bool bInitial)
{
    if(bInitial==true)
    {
        i32RTCAutoModelVerifyTask=1;
        return false;
    }

    static bool bVerifyNG=false;
    static bool bFTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bBTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    int &Task=i32RTCAutoModelVerifyTask;
    bool bCheckAllSuck=false, flag=false;

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "Do32RTCAutoModelVerify 1"))
            {
                bVerifyNG=false;
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "Do32RTCAutoModelVerify 100"))
            {
                DoAllPassVerifyRTC(true);
                Task=200;
            }
            break;
        case 200:
            if(DoAllPassVerifyRTC())
            {
                Task=12010;
            }
            break;
        case 12010:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Middle, iSpeedY, "Do32RTCAutoModelVerify 12010"))
            {
                Task=12020;
            }
            break;
        case 12020:
            if(Prod.TestZ1_Drop_Offset!=0)
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+200, iSpeedSlow))
                {
                    Task=12100;
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+200, iSpeedSlow))
                {
                    Task=12100;                                                 //Steven 20110511
                }
            }
            break;
        case 12100:
            bFTestSuckDrop=true;

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                       FTestSuck.Item[i][j]==HAS_IC)
                    {
                        FTestSuck.Suck[i][j].Off();
                    }

                    DoTestYFrontDelay_32.SetSecAndOn(3);
                }
            }
            Task=12200;
            break;
        case 12200:
            if(DoTestYFrontDelay_32.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                           FTestSuck.Item[i][j]==HAS_IC)
                            FTestSuck.Suck[i][j].Normal();
                    }
                }

                bIndexCheckNoStopVaccum=false;
                Task=12300;
            }
            break;
        case 12300:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed))
            {
                Task=12310;
            }
            break;
        case 12310:
            if(Prod.TestZ2_Drop_Offset!=0)
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+200, iSpeedSlow))
                {
                    Task=12320;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+200, iSpeedSlow))
                {
                    Task=12320;                                                 //Steven 20110511
                }
            }
            break;
        case 12320:
            bBTestSuckDrop=true;

            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                       BTestSuck.Item[i][j]==HAS_IC)
                    {
                        BTestSuck.Suck[i][j].Off();
                    }

                    DoTestYFrontDelay_32.SetSecAndOn(3);
                }
            }
            Task=12330;
            break;
        case 12330:
            if(DoTestYFrontDelay_32.Off())
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                           BTestSuck.Item[i][j]==HAS_IC)
                            BTestSuck.Suck[i][j].Normal();
                    }
                }

                bIndexCheckNoStopVaccum=false;
                Task=12340;
            }
            break;
        case 12340:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed, "Do32RTCAutoModelVerify 12340"))
            {
                Task=12400;
            }
            break;
        case 12400:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "Do32RTCAutoModelVerify 12400"))
            {
                Task=12500;
            }
            break;
        case 12500:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease]=false;
            COM2->SendCommToVision(COM2->rtRelease, true);
            DoTestYFrontDelay_32.SetSecAndOn(10);
            Task=12600;
            break;
        case 12600:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtRelease])
            {
                Task=12700;
            }
            else if(DoTestYFrontDelay_32.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Release Time out of Auto Model Verify");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 12700:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd]=false;
            COM2->SendCommToVision(COM2->rtInspEnd, true);
            DoTestYFrontDelay_32.SetSecAndOn(10);
            Task=12800;
            break;
        case 12800:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtInspEnd])
            {
                Task=12900;
            }
            else if(DoTestYFrontDelay_32.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC End Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 12900:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYOK]=false;
            COM2->SendCommToVision(COM2->rtOPENVERIFYNG, true);
            DoTestYFrontDelay_32.SetSecAndOn(10);
            Task=13000;
            break;
        case 13000:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYOK])
            {
                Task=13100;
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtOPENVERIFYNG])
            {
                ShowMyMessage("RTC Open Verify NG");
                Task=12500;
            }
            else if(DoTestYFrontDelay_32.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Open Verify Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 13100:
            COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG]=false;
            COM2->SendCommToVision(COM2->rtALLFAILOK, true);
            DoTestYFrontDelay_32.SetSecAndOn(10);
            Task=13200;
            break;
        case 13200:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILOK])
            {
                bVerifyNG=false;
                Task=13400;
            }
            else if(COM2->bRealTimeCom_ReceiveOK[COM2->rtALLFAILNG])
            {
                bVerifyNG=true;
                Task=13300;
            }
            else if(DoTestYFrontDelay_32.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Verify All Fail Time out");
                COM2->DoReleaseAndInspEnd();
                Task=12500;
            }
            break;
        case 13300:
            if(IndexAlarmInArmAway()==true)
            {
                ShowMyMessage("RTC Verify All Fail NG");
                Task=13400;
            }
            break;
        case 13400:
            COM2->SendCommToVision(COM2->rtCLOSEVERIFY, true);
            DoTestYFrontDelay_32.SetSecAndOn(10);
            Task=13500;
            break;
        case 13500:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtCLOSEVERIFY])
            {
                Task=13600;
            }
            else if(DoTestYFrontDelay_32.Off())
            {
                if(COM2->OpenRTCComPortAgain())
                    ShowMyMessage("RTC Close Verify Time out");
                COM2->DoReleaseAndInspEnd();
                Task=13400;
            }
            break;
        case 13600:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Middle, iSpeedY, "Do32RTCAutoModelVerify 13600"))
            {
                Task=13700;
            }
            break;
        case 13700:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, iSpeedSlow))
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            FTestSuck.Suck[i][j].Error=false;
                            bFTestSuckUse[i][j]=true;
                        }
                        bFTSuckFinish[i][j]=false;
                    }
                }

                Task=13800;
            }
            break;
        case 13800:
            bCheckAllSuck=true;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)
                    {
                        if(bFTestSuckUse[i][j])
                        {
                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                bFTSuckFinish[i][j]=true;
                                bFTestSuckUse[i][j]=false;
                            }
                            else if(FTestSuck.Suck[i][j].Error)
                            {
                                bFTSuckFinish[i][j]=true;
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bFTSuckFinish[i][j]=true;
                        }
                    }
                }
            }

            flag=true;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)
                        flag=false;
                }
            }

            if(flag)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)                          //若有吸取錯誤
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            if(IniConfig.bD62PickUpErrorNeedPurge)              //Steven 20161024 : 吸取異常需要吹氣一次
                            {
                                FTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(bCheckAllSuck)
                {
                    Task=13810;
                }
            }
            break;
        case 13810:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeedSlow))
            {
                Task=13820;
            }
            break;
        case 13820:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset, iSpeedSlow))
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]!=NULL_IC &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            BTestSuck.Suck[i][j].Error=false;
                            bBTestSuckUse[i][j]=true;
                        }
                        bBTSuckFinish[i][j]=false;
                    }
                }

                Task=13830;
            }
            break;
        case 13830:
            bCheckAllSuck=true;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTSuckFinish[i][j]==false)
                    {
                        if(bBTestSuckUse[i][j])
                        {
                            if(BTestSuck.Suck[i][j].Suck())
                            {
                                bBTSuckFinish[i][j]=true;
                                bBTestSuckUse[i][j]=false;
                            }
                            else if(BTestSuck.Suck[i][j].Error)
                            {
                                bBTSuckFinish[i][j]=true;
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bBTSuckFinish[i][j]=true;
                        }
                    }
                }
            }

            flag=true;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTSuckFinish[i][j]==false)
                        flag=false;
                }
            }

            if(flag)
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)                          //若有吸取錯誤
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            if(IniConfig.bD62PickUpErrorNeedPurge)              //Steveb 20161024 : 吸取異常需要吹氣一次
                            {
                                BTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(bCheckAllSuck)
                {
                    Task=13900;
                }
            }
            break;
        case 13900:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "Do32RTCAutoModelVerify 13900"))
            {
                Task=14000;
            }
            break;
        case 14000:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear, iSpeedY, "Do32RTCAutoModelVerify 14000"))
            {
                if(bVerifyNG==true)
                {
                    ShowMyMessage("RTC Auto Verify fail!!");
                    Task=1;
                }
                else
                {
                    COM2->InitRealTimeCCDPara();
                    bFTestSuckDrop=false;
                    IndexStatus=Z1Down_Z2Up;
                    return true;
                }
            }
            break;
    }
    return false;
}
#endif // GOLDEN VERBATIM -- golden atester_32Site.cpp:184-681 (end)
bool Do32RTCAutoModelVerify(bool bInitial)
{
    if(bInitial==true)
    {
        i32RTCAutoModelVerifyTask=1;
        return false;
    }

    static bool bVerifyNG=false;
    static bool bFTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bBTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false} };

    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    int &Task=i32RTCAutoModelVerifyTask;
    bool bCheckAllSuck=false, flag=false;

    switch(Task)
    {
        case 1:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "Do32RTCAutoModelVerify 1"))
            {
                bVerifyNG=false;
                Task=100;
            }
            break;
        case 100:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "Do32RTCAutoModelVerify 100"))
            {
                DoAllPassVerifyRTC(true);
                Task=200;
            }
            break;
        case 200:
            if(DoAllPassVerifyRTC())
            {
                Task=12010;
            }
            break;
        case 12010:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Middle, iSpeedY, "Do32RTCAutoModelVerify 12010"))
            {
                Task=12020;
            }
            break;
        case 12020:
            if(Prod.TestZ1_Drop_Offset!=0)
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+200, iSpeedSlow))
                {
                    Task=12100;
                }
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+200, iSpeedSlow))
                {
                    Task=12100;                                                 //Steven 20110511
                }
            }
            break;
        case 12100:
            bFTestSuckDrop=true;

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                       FTestSuck.Item[i][j]==HAS_IC)
                    {
                        FTestSuck.Suck[i][j].Off();
                    }

                    DoTestYFrontDelay_32.SetSecAndOn(3);
                }
            }
            Task=12200;
            break;
        case 12200:
            if(DoTestYFrontDelay_32.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC ||
                           FTestSuck.Item[i][j]==HAS_IC)
                            FTestSuck.Suck[i][j].Normal();
                    }
                }

                bIndexCheckNoStopVaccum=false;
                Task=12300;
            }
            break;
        case 12300:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed))
            {
                Task=12310;
            }
            break;
        case 12310:
            if(Prod.TestZ2_Drop_Offset!=0)
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+200, iSpeedSlow))
                {
                    Task=12320;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test+200, iSpeedSlow))
                {
                    Task=12320;                                                 //Steven 20110511
                }
            }
            break;
        case 12320:
            bBTestSuckDrop=true;

            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                       BTestSuck.Item[i][j]==HAS_IC)
                    {
                        BTestSuck.Suck[i][j].Off();
                    }

                    DoTestYFrontDelay_32.SetSecAndOn(3);
                }
            }
            Task=12330;
            break;
        case 12330:
            if(DoTestYFrontDelay_32.Off())
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_HOT_IC ||
                           BTestSuck.Item[i][j]==HAS_IC)
                            BTestSuck.Suck[i][j].Normal();
                    }
                }

                bIndexCheckNoStopVaccum=false;
                Task=12340;
            }
            break;
        case 12340:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, MOT[MTestZ2].GailSpeed, "Do32RTCAutoModelVerify 12340"))
            {
                Task=12400;
            }
            break;
        case 12400:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedY, "Do32RTCAutoModelVerify 12400"))
            {
                Task=12500;
            }
            break;

        // =====================================================================
        // GATED per unit spec (island #2, ~316 golden lines, golden :364-679):
        // cases 12500..14000 -- the RTC real-time-vision protocol handshake
        // (COM2->SendCommToVision/bRealTimeCom_ReceiveOK/OpenRTCComPortAgain/
        // InitRealTimeCCDPara) that decides bVerifyNG (vision pass/fail).  No
        // non-vision fallback exists for this range (unlike the surrounding
        // motor-move cases) -- this IS the vision check.  Faking either
        // outcome would fabricate an unverified pass/fail signal, so instead
        // this parks at the block entry: no motor motion, no vision call, no
        // verdict.  golden default on total failure is `Task=1` (retry from
        // the top, golden :668); we do NOT even take that path automatically
        // since it would silently loop forever issuing ShowMyMessage() popups
        // with no operator visibility into WHY.  Conservative-safe: hold here.
        // =====================================================================
#if 0 // TODO(W5-atester32site-vision) -- golden :364-679 (cases 12500-14000, RTC vision handshake: COM2->SendCommToVision/bRealTimeCom_ReceiveOK/OpenRTCComPortAgain/InitRealTimeCCDPara/rtRelease/rtInspEnd/rtOPENVERIFYOK/rtOPENVERIFYNG/rtALLFAILOK/rtALLFAILNG/rtCLOSEVERIFY)
        case 12500: /* golden entry to the vision handshake -- see gate above */
#endif
            (void)bCheckAllSuck; (void)flag; (void)bFTestSuckUse; (void)bFTSuckFinish;
            (void)bBTestSuckUse; (void)bBTSuckFinish;
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
int iTestTwoArm32SiteTask=1;

void InitTestYTwoArm32SiteTask()                                                //2013-01-15    Dell    Add nn Mode
{
    iTestTwoArm32SiteTask=1;
}
//---------------------------------------------------------------------------
TQPF_Timer hTestZ2Delay_32;
TQPF_Timer iWaitIndexArm1_32;
TQPF_Timer TorqueTimeOutDelay;                                                  //KenHsieh 20220409 : nn Mode扭力紀錄

//==============================================================================
//  DoTestY_TwoArm32Site (golden :693-2291) -- the 32-site dual-arm test-cycle
//  dispatcher: index-down -> negative-pressure/full-view check -> contact ->
//  suck/destroy hand-off -> index-up.  ACTIVE, faithful, EXCEPT cases
//  55,60,61,62,64,65,66,67,68 (golden :801-1011) -- see file-header banner
//  island #1 -- which are GATED as one block (RTC full-view-verify + fContact
//  ROI-learning decision).  The OUTER RTC-guarded touches at cases 120/225/226/
//  2000 use the W5_32S_SEAM COM2 extension (ACTIVE; provably takes the
//  non-vision else-branch given the seam's fixed defaults -- see banner).
//==============================================================================
bool DoTestY_TwoArm32Site()                                                     //2013-01-15    Dell    Add nn Mode
{
    static bool bSucketHasICError=false;
    static bool bOneTimeFlag=true, bSocketCheckSkip=false, flag1=false;
    static bool bCheckShuttle=false;
    static bool bFTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    static bool bBTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL];
    static bool bMovOK_1=false, bMovOK_2=false, bIndexSuckCheck1=false, bIndexSuckCheck2=false;
    static bool bintered1=true;                                                 //Isaac 20200922 : record indexArmY encoder value vs command value
    static AnsiString ErrPart="";                                               //Steven 20101122
    static AnsiString str="";                                                   //kevin 20180508 socket sensor for 32 site

    DoTestYTwoArm_ReStart:
    QueueTaskList[51].CheckTaskChange();                                        //Steven 20200821 : Goto also needs to record Task change

    int &Task=iTestTwoArm32SiteTask;
    int iIndexUpPos=0;
    int iAZ1=0, iAZ2=0;
    int iNN=IsNNMode();
    int iFinishTestUpWaitHeight_Z1=0;                                           //jou 2011-10-31 start : test-drop mode + different speed modes also need two-stage rise support
    int iFinishTestUpWaitHeight_Z2=0;
    int ret;
    bool bCheckAllSuck=false, flagSuckErr=false;
    bool flag=false;
    bool TMode=false;
    bool PickShuttleIC_Together=IniConfig.bD59_32SitePnpTogether;               //JerryYang 20220830 : unify via D59
    AnsiString sBufferT="", str1;

    switch(Task)
    {
        case 1:
            fRearNeedSuck=false;
            fRearNeedDestroy=false;
            InitRearTestSuckICTask();
            InitRearTestDestroyICTask();
            fFrontNeedSuckIC=false;

            fFrontNeedSuck=false;
            fFrontNeedDestroy=false;
            InitFrontTestSuckICTask();
            InitFrontTestDestroyICTask();
            InitBTestSuckTestICTask();
            fRearNeedSuckIC=false;

            if(FTestSuck.UseSiteNoIC())
            {
                InitFrontTestSuckICTask();
                fFrontNeedSuck=true;
            }
            else
            {
                if((FTestSuck.UseSiteHasIC() && FTestSuck.AlreadyTest()) || FTestNeedDestroy())
                {
                    fFrontNeedDestroy=true;                                                                             // release already-tested IC first, then pick up new IC
                    fFrontNeedSuck=true;
                    InitFrontTestSuckICTask();
                    InitFrontTestDestroyICTask();
                }
            }

            if(BTestSuck.UseSiteNoIC())
            {
                InitRearTestSuckICTask();
                fRearNeedSuck=true;
            }
            else
            {
                if((BTestSuck.UseSiteHasIC() && BTestSuck.AlreadyTest()) || BTestNeedDestroy())
                {
                    fRearNeedDestroy=true;                                                                              // release already-tested IC first, then pick up new IC
                    fRearNeedSuck=true;
                    InitRearTestSuckICTask();
                    InitRearTestDestroyICTask();
                }
            }

            fFrontNeedTest=false;
            fRearNeedTest =false;

            if(iOneCycle &&
               IsInArmOneCycleFinish()==true &&
               bCanNotDisableOneCycle==false &&
               FLCarryKit.UseSiteNoIC() &&
               InArmSuck.HasRealIC()==false &&
               ((LastSet.iTemperature==Tempture_Hot && bPickFromHotplate==false) ||                                     //Steven 20220121 : avoid half-suck race on in-arm
                (LastSet.iTemperature!=Tempture_Hot && bPickFromLoader==false)))
            {
                fFrontNeedSuck=false;
            }

            Task=100;
            goto DoTestYTwoArm_ReStart;                                                                                 //2008/10/20 lee

    //-------------------------
    //drop-mode handling
    //-------------------------
        case 50:
            MOT[MTestZ2].MovFlag=false;
            MOT[MTestZ2].bScanFlag=false;
            DoTestYFrontDelay_32.SetMSAndOn(300);
            Task=52;
            break;
        case 52:
            if(DoTestYFrontDelay_32.Off())
                Task=55;
            break;

        // =====================================================================
        // GATED per unit spec (island #1, ~211 golden lines, golden :801-1011):
        // cases 55,60,61,62,64,65,66,67,68 -- RTC full-view-verify + fContact
        // ROI-learning hand-off cluster.  See file-header banner for the full
        // rationale (this is the vision DECISION itself, unlike the outer
        // COM2-guarded short-circuits at cases 120/225/226/2000 which have a
        // real non-vision else-branch and are translated via the W5_32S_SEAM
        // COM2 extension instead).  Parks at the block entry (no motor motion,
        // no vision call, no verdict) rather than fabricate a pass/fail.
        // =====================================================================
#if 0 // TODO(W5-atester32site-vision) -- golden :801-1011 (cases 55,60,61,62,64,65,66,67,68: RTC full-view verify [MOT Z1/Z2 encoder-range check + COM2->bRealTimeCom_ReceiveOK[rtFullTNG/rtFullTOK]/RTC_ResetAlarm] + fContact->InitROILearningTask()/Do_ROILearning(true) hand-off)
        case 55: /* gate entry -- see banner + gate comment above */
#endif
            (void)bSucketHasICError; (void)ErrPart;
            break;

        case 81:
            if(DeviceForm.ContactMode==TMove ||
               DeviceForm.ContactMode==TMoveDrop ||                             //jou 2012-02-03 add T Move Drop
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                TMode=true;
            }
            else
            {
                if(IniConfig.bIndexPickupErrStop==true &&                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                   bIndexArm2PickupErrStop==true)
                {
                    TMode=true;
                }
                else
                {
                    TMode=false;
                }
            }

            if(bintered1==true)                                                 //Isaac 20200922 : record indexArmY encoder value vs command value
            {
                bintered1=false;
                RecordIndexPosition(1, 1);                                      //Isaac 20200922 : record indexArmY encoder value vs command value Arm1/Socket
                EncoderTeachingMaxMinCount(1);                                  //Isaac 20201012 : compare Encoder vs Teaching delta after every action
            }

            #ifdef INDEX_PROTECT_TMOVE
            if(bOverRangeDoTMode==true && bTriger4Indexhome==false)             //Isaac 20201012 : index Y over range, do one Tmode
            {
                bTriger4Indexhome=true;
                bOverRange4Indexhome=true;
                TrigerIndexAxisHome();                                          //Isaac 20201012 : index Y over range, do one Tmode, initialize, start auto-cal
                return false;
            }
            #endif

            if(MOT[MTestY1].Gali_nnMode_Z1Z2_Down(MOT[MTestZ1].GailSpeed, TMode))
            {
                IndexStatus=Z1_Z2_Down;
                if(LastSet.bD41TestSocketICCheckSkip)                           //checkbox ticked -> skip check
                {
                    Task=100;
                    break;
                }

                W5_32S_FIOSET_RESET();                                         // golden fiosetview->ResetIndexSuck() -- no such method yet (see W5_32S_SEAM)
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(INDEX_SUCKER_TYPE==0)
                        {
                            FTestSuck.Suck[i][j].On();
                            BTestSuck.Suck[i][j].On();
                        }
                        else                                                    //negative-pressure check
                        {                                                       //closed sites skip vacuum detect,
                            if((FTestSuck.Item[i][j]==NULL_IC ||                //blind spot: dropped IC landing on a closed site is undetectable.
                                FTestSuck.Item[i][j]==HAS_NULL_IC) &&           //Steven 20210309 : do not check sites that have IC
                                bTestSiteUse[0][i+iNN][j]==true)                //jou 20110504
                            {
                                flag1=false;                                    //jou 20110503 start
                                fiosetview->bIndexSuck[0][i][j]=true;
                                do
                                {
                                    if(flag1==false)
                                        flag1=W5_32S_FIOSET_PISD1(0);          // golden fiosetview->ProcessIndexSuckDestroy1() -- no such method yet (see W5_32S_SEAM)

                                    MySleepEx(1, true);
                                }
                                while(flag1==false);                            //jou 20110503 end
                            }

                            //closed sites skip vacuum detect,
                            //blind spot: dropped IC landing on a closed site is undetectable.
                            if((BTestSuck.Item[i][j]==NULL_IC ||
                                BTestSuck.Item[i][j]==HAS_NULL_IC) &&           //Steven 20210309 : do not check sites that have IC
                               bTestSiteUse[1][i][j]==true)                     //jou 20110504
                            {
                                flag1=false;                                    //jou 20110503 start
                                fiosetview->bIndexSuck[1][i][j]=true;
                                do
                                {
                                    if(flag1==false)
                                       flag1=W5_32S_FIOSET_PISD2(0);           // golden fiosetview->ProcessIndexSuckDestroy2() -- no such method yet (see W5_32S_SEAM)

                                    MySleepEx(1, true);
                                }
                                while(flag1==false);                            //jou 20110503 end
                            }
                        }
                    }
                }

                DoTestYFrontDelay_32.SetSecAndOn(0.5);
                Task=82;
                flag1=false;
            }
            break;
        case 82:
            if(DoTestYFrontDelay_32.Off() || INDEX_SUCKER_TYPE==1)              //jou 20110503
            {
                bMovOK_1=false;
                Task=84;
            }
            break;
        case 84:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ1_Test);                    //Steven 20140620 : integrated into a function
            bMovOK_1=MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+iIndexUpPos, iSpeedFast);

            if(bMovOK_1)
            {
                if(CUSTOMER_CODE==CC_Greatek)                                   //Wei 20160413
                    DoTestYFrontDelay_32.SetSecAndOn(5);                        //Steven 20110908 : delay a bit after coming up too
                else
                    DoTestYFrontDelay_32.SetSecAndOn(0.5);                      //Steven 20110908 : delay a bit after coming up too
                bMovOK_2=false;
                Task=85;
            }
            break;
        case 85:
            iIndexUpPos=GetSocketCheckPos(Prod.TestZ2_Test);                    //Steven 20140620 : integrated into a function
            if(bMovOK_2==false)
                bMovOK_2=MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+iIndexUpPos, iSpeedFast);
            if(bMovOK_2)
            {
                if(CUSTOMER_CODE==CC_Greatek)                                   //Wei 20160413
                    DoTestYFrontDelay_32.SetSecAndOn(5);                        //Steven 20110908 : delay a bit after coming up too
                else
                    DoTestYFrontDelay_32.SetSecAndOn(0.5);                      //Steven 20110908 : delay a bit after coming up too
                Task=90;
            }
            break;
        case 90:
            if(DoTestYFrontDelay_32.Off())
            {
                bSucketHasICError=false;                                        //Steven 20101214 : change position
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(CheckTestSuckICOn(FTestSuck, i, j))
                            {
                                bSucketHasICError=true;
                                ErrPart+=IndexSuckName[i+iNN][j];               //kevin 20180514 arm 1 +2 needs to map into the 32-site alarm position correctly
                            }

                            if(CheckTestSuckICOn(BTestSuck, i, j))
                            {
                                bSucketHasICError=true;
                                ErrPart+=IndexSuckName[i][j];
                            }
                        }
                    }
                }
                Task=95;
            }
            break;
        case 95:
            for(int i=0; i<FTestSuck.iShtRow; i++)                              //Steven 20110719 : no need to blow all, for 9046 1x4 bug
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==NULL_IC || FTestSuck.Item[i][j]==HAS_NULL_IC)
                        FTestSuck.Suck[i][j].Off();
                    if(BTestSuck.Item[i][j]==NULL_IC || BTestSuck.Item[i][j]==HAS_NULL_IC)
                        BTestSuck.Suck[i][j].Off();
                }
            }
            DoTestYFrontDelay_32.SetSecAndOn(5);
            Task=97;
            break;
        case 97:
            if(DoTestYFrontDelay_32.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)                          //Steven 20110719 no need to blow all, for 9046 1x4 bug
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==NULL_IC || FTestSuck.Item[i][j]==HAS_NULL_IC)
                            FTestSuck.Suck[i][j].Normal();                      //jou 2011-11-01 destroy and vacuum can't both be on -> OffDestroy -> Normal
                        if(BTestSuck.Item[i][j]==NULL_IC || BTestSuck.Item[i][j]==HAS_NULL_IC)
                            BTestSuck.Suck[i][j].Normal();                      //jou 2011-11-01 destroy and vacuum can't both be on -> OffDestroy -> Normal
                    }
                }

                if(bSucketHasICError)                                           // sucket has ic error  //Steven 20101214 : moved to Delay.Off()
                {
                    Task=50;
                    if(bSocketCheckSkip)
                    {
                        bSocketCheckSkip=false;
                        Task=100;
                    }
                }
                else
                {
                    Task=100;
                }
            }
            break;
        case 100:
            if(CheckIndexStatus("DoTestY_TwoArm32Site"))
            {
                if(IniConfig.bD21EnableFinishTestUpWait &&                      //jou 2011-10-31 start : test-drop mode + different speed modes also need two-stage rise support
                   IndexStatus==Z1_Z2_Down)                                     //Steven 20180613 (Jou) : add protection to avoid crash
                    Task=105;
                else
                    Task=108;

                bMovOK_1=false;
                goto DoTestYTwoArm_ReStart;                                     //2008/10/20 lee
            }
            break;
        case 105:                                                                                                                                               //jou 2011-10-31 start : test-drop mode + different speed modes also need two-stage rise support
            if(DeviceForm.ContactMode==DropContact ||
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveSlowContact)                                                                                                        //Steven 20160130 : TMove Soft contact
            {
                iFinishTestUpWaitHeight_Z1=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+IniConfig.iD21FinishTestUpWaitHeight;
                iFinishTestUpWaitHeight_Z2=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+IniConfig.iD21FinishTestUpWaitHeight;
            }
            else
            {
                iFinishTestUpWaitHeight_Z1=Prod.TestZ1_Test+IniConfig.iD21FinishTestUpWaitHeight;
                iFinishTestUpWaitHeight_Z2=Prod.TestZ2_Test+IniConfig.iD21FinishTestUpWaitHeight;
            }

            bMovOK_1=MOT[MTestZ1].Gali_Two_ZAxis_Move(iFinishTestUpWaitHeight_Z1, 10000, "DoTestY_TwoArm32Site 105", true, iFinishTestUpWaitHeight_Z2);         //Steven 20150930 : Fixed for 32Site

            if(bMovOK_1)
            {
                Task=106;
                DoTestYFrontDelay_32.SetMSAndOn(IniConfig.iD21FinishTestUpWaitTime);
            }
            break;
        case 106:
            if(DoTestYFrontDelay_32.Off())
                Task=108;
            break;
        case 108:
            bCheckShuttle=false;
            Task=110;

            goto DoTestYTwoArm_ReStart;                                         //2008/10/20 lee
        case 110:
            if(USE_IO_CHANGE_TOQUE==true)  //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(bCheckShuttle==false)
            {
                if(CheckShuttlePos())
                {
                    MOT[MInShuttle1].fCanMoveM=false;
                    MOT[MInShuttle2].fCanMoveM=false;
                }
                else
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    return false;
                }

                bCheckShuttle=true;
            }

            if(IndexStatus==Z1Up_Z2Down || IndexStatus==Z1Down_Z2Up)
            {
                ShowIndexMotorError(AnsiString("DoTestY_TwoArm32Site110"));
                return false;
            }
            else
            {
                    //jou 2011-08-16 +100 -> +500 drop mode too easily mis-detects
                    //jou 2012-01-17 +500 -> +750 seen mis-detect cases, oscillating back and forth Hang up (note: 750 units = 7.5mm)
                    if((MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+750)) ||  //2008/10/20 lee
                       (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)                            )  //Ifor 20210716 add: avoid missing drop-detect when arm is closed
                    {
                        flagSuckErr=false;
                        if(LastSet.iRealDummy==REALLY)
                        {
                            for(int i=0; i<FTestSuck.iShtRow; i++)
                            {
                                for(int j=0; j<FTestSuck.iShtCol; j++)
                                {
                                    if(FTestSuck.Suck[i][j].Enable       &&
                                       FTestSuck.Suck[i][j].SenUsing!="" &&
                                       FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                       FTestSuck.Item[i][j]!=NULL_IC)
                                    {
                                        if(FTestSuck.Suck[i][j].GetStatus()==false)
                                        {
                                            FTestSuck.Suck[i][j].Normal();  //jou 2012-01-17 close directly, avoid dropping onto shuttle, also avoid a stuck drop causing Hang up
                                            flagSuckErr=true;
                                            bRecIndexDropAlarm1=true;  //jou 2012-01-17 record index Drop alarm
                                            ErrPart+=IndexSuckName[i+iNN][j];  //kevin 20220924  add  vacuum-no-signal log
                                        }
                                    }

                                    if(BTestSuck.Suck[i][j].Enable       &&
                                       BTestSuck.Suck[i][j].SenUsing!="" &&
                                       BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                       BTestSuck.Item[i][j]!=NULL_IC)
                                    {
                                        if(BTestSuck.Suck[i][j].GetStatus()==false)
                                        {
                                            BTestSuck.Suck[i][j].Normal();  //jou 2012-01-17 close directly, avoid dropping onto shuttle, also avoid a stuck drop causing Hang up
                                            flagSuckErr=true;
                                            bRecIndexDropAlarm2=true;  //jou 2012-01-17 record index Drop alarm
                                            ErrPart+=IndexSuckName[i][j];  //kevin 20220924  add  vacuum-no-signal log
                                        }
                                    }
                                }
                            }
                        }

                        if(flagSuckErr)
                        {
                            ShowIndexTime(-2);
                            MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:110"));
                            Task=50;
                            str1.sprintf("DoTestY_TwoArm32Site 110 to 50: bRecIndexDropAlarm1=%d, bRecIndexDropAlarm2=%d :%s", bRecIndexDropAlarm1?1:0, bRecIndexDropAlarm2?1:0 ,ErrPart);  //kevin 20220924  add vacuum-no-signal log   //Steven 20201201 : add event log for debug
                            RecordProcess(str1);
                            return false;
                        }
                        else
                        {
                            bRecIndexDropAlarm1=false;  //jou 2012-01-17 record index Drop alarm
                            bRecIndexDropAlarm2=false;  //jou 2012-01-17 record index Drop alarm
                        }
                    }

                if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor)  //kevin 20130504 socket sensor detect error
                {
                    iAZ1=MOT[MTestZ1].Gali_ReadEncoderPos();  //JerryYang 20190327 fix socket sensor misjudge, must read Encoder position
                    iAZ2=MOT[MTestZ2].Gali_ReadEncoderPos();
                    if(Prod.TestZ1_Test==0)  //jimmychiu 20230830 : add
                        iSocketSenSosPos1=-1000;
                    else
                        iSocketSenSosPos1=Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000;
                    if(Prod.TestZ2_Test==0)
                        iSocketSenSosPos2=-1000;
                    else
                        iSocketSenSosPos2=Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000;
                    if((iAZ1>iSocketSenSosPos1) && (iAZ2>iSocketSenSosPos2))  //kevin 20140508 socket sensor
                    {
                        for(int i=0; i<TestIF_File.iSocketCount; i++)
                        {
                            if(Sen[SThreadPara.iSocketSensor[i]].Enable && Sen[SThreadPara.iSocketSensor[i]].IsOn())  //On means residual
                            {
                                MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:110"));
                                flag=true;
                                iShowSocketSensor=1;
                                str+=IntToStr(i+1)+",";
                            }
                        }

                        if(flag)
                        {
                            ShowIndexTime(-2);
                            sBufferT="Z1Z2_Up: Z1Pos "+IntToStr(iAZ1)+">"+IntToStr(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+2000)+"Z2Pos "+IntToStr(iAZ2)+">"+IntToStr(Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+2000);
                            RecordProcess(sBufferT);  //kevin 20150506
                            Task=50;
                            return false;
                        }
                    }
                }

                if(bOneTimeFlag)
                {
                    bOneTimeFlag=false;
                    iBackUpZ2DownPosition=Prod.TestZ2_Test;
                }

                ShowIndexTime(-1);

                if(IniConfig.bIndexPickupErrStop==false)
                {
                    bIndexArm1PickupErrStop=false;
                    bIndexArm2PickupErrStop=false;
                }

                if(DeviceForm.ContactMode==TMove ||
                   DeviceForm.ContactMode==TMoveDrop ||  //jou 2012-02-03 add T Move Drop
                   DeviceForm.ContactMode==TMoveDropSlowContact ||
                   DeviceForm.ContactMode==TMoveSlowContact)  //Steven 20160130 : TMove Soft contact
                {
                    TMode=true;
                }
                else
                {
                    if(IniConfig.bIndexPickupErrStop==true &&  //jou 2012-02-29 index pick up error,index arm move to center & alarm
                       bIndexArm2PickupErrStop==true)
                    {
                        TMode=true;
                    }
                    else
                    {
                        TMode=false;
                    }
                }

                if(MOT[MTestY1].Gali_nnMode_Z1Z2_Up(MOT[MTestZ1].GailSpeed, TMode))
                {
                    Task=120;
                }
            }
            break;
        case 120:                                                               //Steven 20140725 : avoid index Y-axis position drift
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, MOT[MTestY1].GailSpeed, "DoTestY_TwoArm32Site 120"))
            {
                bIndexArm2PickupErrStop=false;
                MOT[MInShuttle1].fCanMoveM=true;
                MOT[MInShuttle2].fCanMoveM=true;
                bCheckShuttle=false;
                bOneTimeFlag=true;
                ShowIndexTime();

                ShowMainScreenPresure(1);                                       //jou 2010-06-23 screen Z1,Z2 encoder display

                Task=200;
                if(REAL_TIME_CCD && COM2->bCCDDummyRum==false)
                {
                    bRTCArm1HalfViewError=false;                                //Steven 20120206 : RTC repeat-error dedupe
                    COM2->SendCommToVision(COM2->rtArmFinish, false);

                    COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;        //20220815 wei moved up here; can do RTC as soon as both arms are up, saves index cycle time //Steven 20240919 : unmark for RTC
                    COM2->SendCommToVision(COM2->rtFullTOK, true);
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    if(TMode==true)                                             //20110923 Tmode brake
                    {
                        hTestZ2Delay_32.SetSecAndOn(0.1);
                        Task=130;
                    }
                }
                goto DoTestYTwoArm_ReStart;                                     //2008/10/20 lee
            }
            break;
        case 130:
            if(hTestZ2Delay_32.Off())
            {
                SW[SwBMotorBreaker].Off();
                Task=200;
            }
            break;
        case 200:
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].On();
                SW[SwIndexChangeToque2].Off();
            }

            Task=209;
        case 209:
            HangTime.SetSecAndOn(Prod.iHangupMaxTime);                          //Steven 20090827 Start: Hang Up dectector
            bHangTimePause=false;
            f32SiteNeedDestroy=false;
            if(FRCarryKit.UseSiteNoIC() &&
               OutSHT1InLF() &&
               BRCarryKit.UseSiteNoIC() &&
               OutSHT2InLF())
            {
                if(iOneCycle==0 ||                                              //Steven 20151006 : For 32Site One Cycle issue
                   IsInArmOneCycleFinish()==false)
                    f32SiteNeedDestroy=true;
            }
            Task=210;
        case 210:                                                               // number must not change *******************
            if(IniConfig.bD59_32SitePnpTogether && f32SiteNeedDestroy)          //Steven 20150910 : 32Site dual-arm suck/destroy together
            {
                fRearNeedDestroy=!Do32SiteTestDestroyIC();
                fFrontNeedDestroy=fRearNeedDestroy;
            }
            else
            {
                if(fRearNeedDestroy)
                {
                    fRearNeedDestroy=!DoRearTestDestroyIC(true);                //ChungHung 20111115
                }

                if(fFrontNeedDestroy)                                           //Index1 has IC, whether to drop it (Index1 already on Shuttle1)
                {
                    fFrontNeedDestroy=!DoFrontTestDestroyIC(true);
                }
            }

            if(fFrontNeedDestroy==false && fRearNeedDestroy==false)
            {
                Task=220;
            }
            else
            {
                break;
            }
        case 220:
            f32SiteNeedSuck=false;
            if(InSHT1InRT() && InSHT2InRT())
            {
                if(iCleanOut &&
                   FLCarryKit.UseSiteHasIC() &&                                 //SHUTTLE 1 HAS IC
                   BLCarryKit.UseSiteNoIC() &&                                  //SHUTTLE 2 NO IC
                   InArmSuck.NoIC() &&
                   MOT[MMPlate1].HasIC()==false &&
                   MOT[MMPlate2].HasIC()==false)                                //JerryYang 20230717 : fix 32 site clean out hang up
                {
                    f32SiteNeedSuck=false;
                }
                else
                {
                    if(iOneCycle==0 ||                                          //Steven 20151006 : For 32Site One Cycle issue
                       IsInArmOneCycleFinish()==false)
                        f32SiteNeedSuck=true;
                }
                Task=221;
            }
            else if(PickShuttleIC_Together)
            {
                if(InSHT1InRT() && InSHT2InRT())
                {
                    if(iCleanOut &&
                       FLCarryKit.UseSiteHasIC() &&                             //SHUTTLE 1 HAS IC
                       BLCarryKit.UseSiteNoIC() &&                              //SHUTTLE 2 NO IC
                       InArmSuck.NoIC() &&
                       MOT[MMPlate1].HasIC()==false &&
                       MOT[MMPlate2].HasIC()==false)                            //JerryYang 20230717 : fix 32 site clean out hang up
                    {
                        f32SiteNeedSuck=false;
                    }
                    else
                    {
                        if(iOneCycle==0 ||                                      //Steven 20151006 : For 32Site One Cycle issue
                           IsInArmOneCycleFinish()==false)
                            f32SiteNeedSuck=true;
                    }

                    Task=221;
                }
                else if((fFrontNeedTest || fRearNeedTest) &&                    //Richard 20230417 : fix RTC hang up error.
                        (FTestSuck.HasRealIC() || BTestSuck.HasRealIC()))
                {
                    Task=221;
                }
                else
                {
                    break;
                }
            }
            else
            {
                Task=221;
            }
        case 221:
            if(IniConfig.bD59_32SitePnpTogether &&                                                                      //Steven 20150910 : 32Site dual-arm suck/destroy together
               f32SiteNeedSuck)
            {
                fRearNeedSuck=!Do32SiteTestSuckIC();
                fFrontNeedSuck=fRearNeedSuck;
            }
            else
            {
                if(fRearNeedSuck)
                {
                    if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                      (LastSet.iTemperature==Tempture_Hot ||
                       LastSet.iTemperature==Tempture_AmbientHot) &&
                       DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                                          //JerryYang 20181214 : non-hotplate mode also needs temperature-priority support  //Ifor 20180606 (wei) : add Index wait on Shuttle for test result
                    {
                        if(fFrontNeedTest==false)
                            fRearNeedSuck=!DoRearTestSuckIC();
                    }
                    else
                    {
                        fRearNeedSuck=!DoRearTestSuckIC();
                    }
                }

                if(fFrontNeedSuck)                                                                                      //Index1 needs to pick up IC?
                {
                    if(DeviceForm.bSuckShuttleDeviceAfterTested &&
                       (LastSet.iTemperature==Tempture_Hot ||
                        LastSet.iTemperature==Tempture_AmbientHot) &&
                        DeviceForm_File.bSuckShuttleDeviceWaitOnShuttle==false)                                         //JerryYang 20181214 : non-hotplate mode also needs temperature-priority support  //Ifor 20180606 (wei) : add Index wait on Shuttle for test result
                    {
                        if(fRearNeedTest==false)
                            fFrontNeedSuck=!DoFrontTestSuckIC();
                    }
                    else
                    {
                        fFrontNeedSuck=!DoFrontTestSuckIC();
                    }
                }
            }

            if(fFrontNeedSuck==false &&
               fRearNeedSuck==false)
            {
                bZ1PickShuttle=false;                                                                                   //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                bZ2PickShuttle=false;                                                                                   //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing

                if(FTestSuck.UseSiteHasIC() && FTestSuck.AlreadyTest()==false)
                {
                    fFrontNeedTest=true;
                    InitFTestSuckTestICTask();
                }

                if(BTestSuck.UseSiteHasIC() && BTestSuck.AlreadyTest()==false)
                {
                    fRearNeedTest=true;
                    InitBTestSuckTestICTask();
                }

                Task=225;
            }
            else
            {
                break;
            }
        case 225:                                                               //----- by dell ccd realtime-------------
            if(IndexStatus==Z1_Z2_Normal && REAL_TIME_CCD==true && !COM2->bCCDDummyRum)
            {
                //----- by dell ccd realtime-------------
//                COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG]=false;          //20220815 wei moved up here; can do RTC as soon as both arms are up, saves index cycle time //Steven 20240919 : mark for RTC
//                COM2->SendCommToVision(COM2->rtFullTOK, true);
                iWaitIndexArm1_32.SetSecAndOn(10);                              //Steven 20250626 32 sites RTC time out alarm 4 -> 10
                //---------------------------------------
                Task=226;
            }
            else
            {
                Task=230;
            }
            //---------------------------------------

            goto DoTestYTwoArm_ReStart;                                         //2008/10/20 lee
        case 226:
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTOK])
            {
                COM2->bRealTimeCom_ReceiveOK[COM2->rtAlarHasIC]=false;
                COM2->RTC_ResetAlarm();     //wei 20221222 RTC ARM Error
                COM2->DoReleaseAndInspEnd();
                bRTCFullViewError=false;                                        //Steven 20120206 : RTC repeat-error dedupe
                Task=230;
                goto DoTestYTwoArm_ReStart;                                     //2008/10/20 lee
            }

            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtFullTNG])
            {
                Task=50;
            }

            if(iWaitIndexArm1_32.Off())
            {                                                                   //Steven 20110824 : Real time CCD - cannot close CCD
                ShowErrorMessage("WAR0337", 0, MMIndex, 0, __FUNC__);           //RTC FullT Time Out Error.!
                COM2->DoReleaseAndInspEnd();
                bSendRealCCDSendStart=true;
                Task=225;
            }
            break;
        case 230:                                                                                                       //Steven 20190903 : add drop-detect while index is pressing down
                if((MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+750)) ||                      //2008/10/20 lee
                   (TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1))                                                  //Ifor 20210716 add: avoid missing drop-detect when arm is closed
                {
                    //Steven 20101214 Start
                    flagSuckErr=false;
                    if(LastSet.iRealDummy==REALLY)
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(FTestSuck.Suck[i][j].Enable       &&
                                   FTestSuck.Suck[i][j].SenUsing!="" &&
                                   FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   FTestSuck.Item[i][j]!=NULL_IC)
                                {
                                    if(FTestSuck.Suck[i][j].GetStatus()==false)
                                    {
                                        FTestSuck.Suck[i][j].Normal();                                                  //jou 2012-01-17 close directly, avoid dropping onto shuttle, also avoid a stuck drop causing Hang up
                                        flagSuckErr=true;
                                        bRecIndexDropAlarm1=true;                                                       //jou 2012-01-17 record index Drop alarm
                                    }
                                }

                                if(BTestSuck.Suck[i][j].Enable       &&
                                   BTestSuck.Suck[i][j].SenUsing!="" &&
                                   BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   BTestSuck.Item[i][j]!=NULL_IC)
                                {
                                    if(BTestSuck.Suck[i][j].GetStatus()==false)
                                    {
                                        BTestSuck.Suck[i][j].Normal();                                                  //jou 2012-01-17 close directly, avoid dropping onto shuttle, also avoid a stuck drop causing Hang up
                                        flagSuckErr=true;
                                        bRecIndexDropAlarm2=true;                                                       //jou 2012-01-17 record index Drop alarm
                                    }
                                }
                            }
                        }
                    }

                    if(flagSuckErr)
                    {
                        MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:230"));
                        Task=50;

                        return false;
                    }
                    else
                    {
                        bRecIndexDropAlarm1=false;                                                                      //jou 2012-01-17 record index Drop alarm
                        bRecIndexDropAlarm2=false;                                                                      //jou 2012-01-17 record index Drop alarm
                    }
                    //----- by dell ccd realtime-------------
                    //Steven 20101214 End
                }

            if(DeviceForm.ContactMode==TMove ||
               DeviceForm.ContactMode==TMoveDrop ||                                                                     //jou 2012-02-03 add T Move Drop
               DeviceForm.ContactMode==TMoveDropSlowContact ||
               DeviceForm.ContactMode==TMoveSlowContact)                                                                //Steven 20160130 : TMove Soft contact
            {
                TMode=true;
            }
            else
            {
                if(IniConfig.bIndexPickupErrStop==true &&                                                               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                   bIndexArm2PickupErrStop==true)
                {
                    TMode=true;
                }
                else
                {
                    TMode=false;
                }
            }

            if(bintered1==true)                                                                                         //Isaac 20200922 : record indexArmY encoder value vs command value
            {
                bintered1=false;
                RecordIndexPosition(1, 1);                                                                              //Isaac 20200922 : record indexArmY encoder value vs command value Arm1/Socket
                EncoderTeachingMaxMinCount(1);                                                                          //Isaac 20201012 : compare Encoder vs Teaching delta after every action
            }

            #ifdef INDEX_PROTECT_TMOVE
            if(bOverRangeDoTMode==true && bTriger4Indexhome==false)                                                     //Isaac 20201012 : index Y over range, do one Tmode
            {
                bTriger4Indexhome=true;
                bOverRange4Indexhome=true;
                TrigerIndexAxisHome();                                                                                  //Isaac 20201012 : index Y over range, do one Tmode, initialize, start auto-cal
                return false;
            }
            #endif

            if(MOT[MTestY1].Gali_nnMode_Z1Z2_Down(MOT[MTestZ1].GailSpeed, TMode))
            {
                if(fFrontNeedTest || fRearNeedTest)
                {
                    fTwoArmNeedTest=true;
                    InitTestSuckTestIC_TwoArm32Site_Task();
                }
                Task=240;

                if(Prod.TestZ2_Drop_Offset!=0)                                                                          //DropContact or DirectContactModeDiffentSpeed  //jou 2015-04-29 fix 32 sites drop mode index crash
                {
                    Task=300;
                    break;
                }
            }
            break;
        case 240:                                                               //start testing
#if 0 // TODO(W7-UI) -- golden :1806 fMain->SendMSG_CMD(MSG_CMD_Arm1Down) (TfMain has no SendMSG_CMD method yet, golden main.h; MSG_CMD_Arm1Down constant not translated either)
            fMain->SendMSG_CMD(MSG_CMD_Arm1Down);                               //Steven 20210225 : Add GPIB LOG
#else
            W5_32S_FMAIN_SENDMSG_CMD(0);
#endif
            Task=241;
        case 241:
            if(fTwoArmNeedTest)
            {
                fTwoArmNeedTest=!DoTestSuckTestIC_TwoArm32Site();
                CheckIndexAllSuckICFallDown(false, true);                                                               //Steven 20110725 : revise negative-pressure check
            }

            if(fTwoArmNeedTest==false)
            {
                #ifdef ASE_KaohSiung
                if(TestIF_File.bEnableReadAndCheckTorque &&                                                             //KenHsieh 20220409 : nn Mode torque logging
                   TorqueUseHPComCard &&
                   COM2->bGetValue &&
                   COM2->bGetValue1)                                                                                    //kevin 20210804 change
                {
                    if(NewCheckAndRecodeTorque(0)==false)
                    {
                        if(bRetryReadToqu==false)                                                                       //kevin 20210419 re-read torque
                        {
                            bRetryReadToqu=true;
                            bNeedCheckIndexToque=false;                                                                 //KEVIN 20210419 ADD TORQUE
                            bNeedCheckIndexToque1=false;
                            fMain->chkReadTorque1->Checked=true;
                            COM2->ReadIndexTorqueSetting(0);
                        }

                        if(TorqueTimeOutDelay.Off()==false)                                                             //kevin 2021018 torque
                           return false;
                    }

                    if(NewCheckAndRecodeTorque(1)==false)
                    {
                        if(bRetryReadToqu==false)                                                                       //kevin 20210419 re-read torque
                        {
                            bRetryReadToqu=true;
                            bNeedCheckIndexToque=false;                                                                 //KEVIN 20210419 ADD TORQUE
                            bNeedCheckIndexToque1=false;
                            fMain->chkReadTorque2->Checked=true;
                            COM2->ReadIndexTorqueSetting(1);
                        }

                        if(TorqueTimeOutDelay.Off()==false)                                                             //kevin 2021018 torque
                           return false;
                    }
                }
                #endif

                if(USE_IO_CHANGE_TOQUE==true)                                                                           //jou 2012-06-21 Enable index I/O Change Toque
                {
                    SW[SwIndexChangeToque1].Off();
                    SW[SwIndexChangeToque2].Off();
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    SW[SwBMotorBreaker].On();                                                                           //20110923 Tmode brake
                    MySleep(200);
                }

                #ifdef ASE_KaohSiung
                if(TestIF_File.bEnableReadAndCheckTorque &&
                   TorqueUseHPComCard)                                                                                  //KenHsieh 20220409 : nn Mode torque logging
                {
                    if(COM2->bGetValue==false || COM2->bGetValue1==false)
                        return false;
                }
                #endif
                if(DeviceForm.VacuumMode==VacuumONMode)
                {
                    iHangupCTArm1_32=0;
                    return true;
                }
                else
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)                                                              //ARM->Index
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC &&
                               FTestSuck.Item[i][j]!=HAS_NULL_IC)                                                       //Steven 20180402 (Jou) : added
                            {
                                FTestSuck.Suck[i][j].Error=false;
                                bFTestSuckUse[i][j]=true;
                            }
                            bFTSuckFinish[i][j]=false;

                            if(BTestSuck.Item[i][j]!=NULL_IC &&
                               BTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Error=false;
                                bBTestSuckUse[i][j]=true;
                            }
                            bBTSuckFinish[i][j]=false;                                                                  //Steven 20110301
                        }
                    }
                    Task=250;
                    iHangupCTArm1_32=0;
                }
            }

            if(HangTime.Off() && TestISTimeOut==false && bHangTimePause==false)
            {
                if(W5_32S_AMR_NEEDTRANSPORT())                                                                          // golden AMR.NeedAMRTransport() -- AMR symbol type collision, see W5_32S_SEAM  //Sam 20250423 : AMR transport can take long enough to alarm; needs a bypass
                {
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    break;
                }

                if(IniConfig.bTModeMotorFree==true)
                {
                    SW[SwBMotorBreaker].On();                                                                           //20110923 Tmode brake
                    MySleep(200);
                }

                if(CheckHeaterOK()==false)                                                                              //Steven 20250116 : confirm HeaterOK
                {
                    bHangTimePause=true;
                    break;                                                                                              //kevin 20161102 break;
                }

                bNoUseAutoRecord=true;                                                                                  //wei 20160311

                RecordProcess("Auto State Record by DoTestY_TwoArm32Site");
                fMain->DoStateRecord(0, false);                                                                         //Steven 20120705 : Hang Up auto-saves the screen     //KenHsieh 20230116 : distinguish manual vs auto(sbclick -> Function)

                #ifdef DEBUG_HANGUP_NO_HOME
                    ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
                    Task=209;
                #else
                    iHangupCTArm1_32++;
                    if(iHangupCTArm1_32>1)
                    {
                        iHangupCTArm1_32=0;
                        ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
                        Task=209;
                    }
                    else
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                             //kevin 20160722 ASE KaohSiung: cancelled
                        {
                            ShowErrorMessage("JAM0316", K_SKIP, MTestZ1);
                            Task=209;
                        }
                        else
                        {
                            MyDBIProcessNew("Motion", "WAR2206", "Auto homing", "0316");                                //Steven 20120705 : Hang Up auto-saves the screen
                            bHomeByStart=true;
                            fAllMotorHome=false;
                        }
                    }
                #endif
                break;
            }
            break;
        case 250:                                                               //drop-test with vacuum off mode: need to suck IC back first
            bCheckAllSuck=true;

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)                              //Steven 20110301
                    {
                        if(bFTestSuckUse[i][j])
                        {
                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                bFTSuckFinish[i][j]=true;                       //Steven 20110301
                                bFTestSuckUse[i][j]=false;
                            }
                            else if(FTestSuck.Suck[i][j].Error)                 //jou 2011-08-16
                            {
                                bFTSuckFinish[i][j]=true;                       //Steven 20110301
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bFTSuckFinish[i][j]=true;                           //Steven 20110301
                        }
                    }

                    if(bBTSuckFinish[i][j]==false)                              //Steven 20110301
                    {
                        if(bBTestSuckUse[i][j])
                        {
                            if(BTestSuck.Suck[i][j].Suck())
                            {
                                bBTSuckFinish[i][j]=true;                       //Steven 20110301
                                bBTestSuckUse[i][j]=false;
                            }
                            else if(BTestSuck.Suck[i][j].Error)                 //jou 2011-08-16
                            {
                                bBTSuckFinish[i][j]=true;                       //Steven 20110301
                            }
                            else
                            {
                                bCheckAllSuck=false;
                            }
                        }
                        else
                        {
                            bBTSuckFinish[i][j]=true;                           //Steven 20110301
                        }
                    }
                }
            }

            flagSuckErr=true;                                                   //Steven 20110301 : Start
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if((bBTSuckFinish[i][j]==false) ||
                       (bFTSuckFinish[i][j]==false))
                        flagSuckErr=false;
                }
            }

            if(flagSuckErr)                                                     //Steven 20110301
            {
                if(bCheckAllSuck)
                {
                    return true;
                }
            }
            break;
        case 300:                                                               //drop-test mode: about to move further down by offset -- here close vacuum first, then drop IC
            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||         //Eliot 2011_0318 Start
               DeviceForm.ContactMode==TMoveSlowContact)                        //Steven 20160130 : TMove Soft contact
            {
                Task=320;
                break;
            }                                                                   //Eliot 2011_0318 end

            bFTestSuckDrop=true;                                                //jou 2011-12-13 only drop-test needs this set true
            fFrontNeedSuckIC=false;
            bBTestSuckDrop=true;
            fRearNeedSuckIC=false;

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                        FTestSuck.Suck[i][j].Off();
                    if(BTestSuck.Item[i][j]==HAS_HOT_IC || BTestSuck.Item[i][j]==HAS_IC)
                        BTestSuck.Suck[i][j].Off();
                }
            }
            DoTestYFrontDelay_32.SetSecAndOn(Prod.TestZ_Drop_Wait);             // delay 0.3 sec for ic down        //Steven 20140909 : moved outside the loop
            Task=310;
            break;
        case 310:                                                               //close blow-off
            if(DoTestYFrontDelay_32.Off())
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                            FTestSuck.Suck[i][j].Normal();
                        if(BTestSuck.Item[i][j]==HAS_HOT_IC || BTestSuck.Item[i][j]==HAS_IC)
                            BTestSuck.Suck[i][j].Normal();
                    }
                }

                bMovOK_1=false;
                if(IniConfig.bC08_SocketSensor &&                               //Steven 20180313 (Jou) : use Socket Sensor to verify offset
                   TestIF_File.bEnSocketSensor &&
                   TestIF_File.bCheckSocketFloating)
                {
                    Task=311;
                    break;
                }
                Task=320;
            }
            break;
        case 311:                                                               //Steven 20180313 (Jou) : use Socket Sensor to verify offset
            flag=false;
            for(int i=0; i<TestIF_File.iSocketCount; i++)
            {
                if(TestIF_File.iSensorCheckType[i]==2 &&                        //Steven 20200420 : Socket Sensor function selectable
                   Sen[SThreadPara.iSocketSensor[i]].IsOn())                    //On means offset detected
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case:311"));
                    flag=true;
                    iShowSocketSensor=2;
                    str+=IntToStr(i+1)+",";
                }
            }

            if(flag)
            {
                Task=312;
                return false;
            }
            else
            {
                Task=320;
            }
            break;
        case 312:                                                               //after drop-test IC dropped, offset detected, Index goes up
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeedSlow, "DoTestY_TwoArm32Site 312"))
            {
                Task=313;
            }
            break;
        case 313:                                                               //Y axis separates
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 50000, "DoTestY_TwoArm32Site 313"))
            {
                Task=314;
            }
            break;
        case 314:                                                               //after drop-test IC dropped, offset detected, Alarm
            if(IndexAlarmInArmAway()==true)                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
            {
                if(IniConfig.bD40IndexICFallDownMustPressFMotorDown)            //Steven 20151022 : add for MAXIM
                    bIsTestSitICFallDown=true;
                bIsSocketSensor=true;

                ShowErrorMessage("WAR0323", K_RETRY, MTestZ2, false, str);      //Socket detect device floting error
                Task=315;
            }
            break;
        case 315:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Middle, 50000, "DoTestY_TwoArm32Site 315"))
            {
                Task=316;
            }
            break;
        case 316:                                                               //after drop-test IC dropped, offset cleared, Index moves back down
            iFinishTestUpWaitHeight_Z1=Prod.TestZ1_Test;
            iFinishTestUpWaitHeight_Z2=Prod.TestZ2_Test;

            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(iFinishTestUpWaitHeight_Z1, 10000, "DoTestY_TwoArm32Site 316", true, iFinishTestUpWaitHeight_Z2))
            {
                Task=311;
            }
            break;
        case 320:  //move down to contact
            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||  //Eliot 2011_0318 Start
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveSlowContact ||  //Steven 20160130 : TMove Soft contact
               DeviceForm.ContactMode==TMoveDropSlowContact)
            {
                bMovOK_1=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, ((MOT[MTestZ1].GailSpeed*Prod.TestZ_Drop_Speed)/100), "DoTestY_TwoArm32Site 320", true, Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset);  //Steven 20150930 : Fixed for 32Site
                if(bMovOK_1)
                {
                    if(DeviceForm.ContactMode==DropContactModeDiffentSpeed ||  //JerryYang 20201022 fix 32 site drop-test mode not sucking vacuum
                       DeviceForm.ContactMode==TMoveDropSlowContact)
                    {
                        Task=330;  //kevin 20191210
                    }
                    else
                    {
                        //Task=240;                                             //Steven 20110511 //jou 2015-04-29 fix 32 sites drop mode index crash
                        Task=325;  //kevin 20191210
                    }
                }
            }
            else
            {
                bMovOK_1=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset, MOT[MTestZ1].GailSpeed, "DoTestY_TwoArm32Site 320", true, Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset);  //Steven 20150930 : Fixed for 32Site
                if(bMovOK_1)
                    Task=325;
            }
            break;
        case 325:                                                               //kevin 20191210 add SOFT EP  charge
            if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
               DeviceForm_File.ContactMode==DropContactSoftEP)
            {                                                                   //kevin 20191210 Soft Contact mode
//                bContSoftEpSwitch(0, true);                                     //ARM1 in test zone, floating-head charge
//                bContSoftEpSwitch(1, true);                                     //ARM2 in test zone, floating-head charge
                EPSwitchOnOff(eEPSwBoth);                                       //Steven 20250417 : fixed for [D58]

                if(DeviceForm.fAireForce==0)
                    DeviceForm.fAireForce=DeviceForm.dPress;                    //kevin 20220505 add 32 site soft EP Pick up error

                ADAM_WriteVoltage(DeviceForm.fAireForce);                       //kevin 20220505 add  EP charge
            }

            if(DeviceForm.ContactMode==DirectContactModeDiffentSpeed ||         //Eliot 2011_0318 Start
               DeviceForm.ContactMode==DropContactModeDiffentSpeed ||
               DeviceForm.ContactMode==TMoveSlowContact ||                      //Steven 20160130 : TMove Soft contact
               DeviceForm.ContactMode==TMoveDropSlowContact)
            {
                Task=240;                                                       //Steven 20110511 //jou 2015-04-29 fix 32 sites drop mode index crash
                break;
            }
            Task=330;
            break;
        case 330:                                                               //vacuum-on mode: must suck IC first
            if(DeviceForm.VacuumMode==VacuumONMode)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_HOT_IC || FTestSuck.Item[i][j]==HAS_IC)
                        {
                            if(INDEX_SUCKER_TYPE==1)                            //Steven 20111202
                            {
                                fiosetview->bIndexSuck[0][i][j]=true;
                                bIndexCheckNoStopVaccum=true;                   //Sam 20220816 : protect against all-site drop on pause
                            }
                            else
                            {
                                FTestSuck.Suck[i][j].On();
                            }
                        }

                        if(BTestSuck.Item[i][j]==HAS_HOT_IC || BTestSuck.Item[i][j]==HAS_IC)
                        {
                            if(INDEX_SUCKER_TYPE==1)                            //Steven 20111202
                            {
                                fiosetview->bIndexSuck[1][i][j]=true;
                                bIndexCheckNoStopVaccum=true;                   //Sam 20220816 : protect against all-site drop on pause
                            }
                            else
                            {
                                BTestSuck.Suck[i][j].On();
                            }
                        }
                    }
                }
            }

            bIndexSuckCheck1=false;
            bIndexSuckCheck2=false;
            if(Prod.TestZ_Drop_Wait<0.1)                                        //Steven 20220622 : made settable
                DoTestYFrontDelay_32.SetSecAndOn(0.1);                          // delay 0.1 sec for ic down
            else
                DoTestYFrontDelay_32.SetSecAndOn(Prod.TestZ_Drop_Wait);
            Task=340;
//            break;
        case 340:
            if(INDEX_SUCKER_TYPE==1)
            {
                if(bIndexSuckCheck1==false)
                    bIndexSuckCheck1=W5_32S_FIOSET_PISD1(1);                   // golden fiosetview->ProcessIndexSuckDestroy1(1) -- no such method yet (see W5_32S_SEAM)  //JerryYang 20250120 : modify
                if(bIndexSuckCheck2==false)
                    bIndexSuckCheck2=W5_32S_FIOSET_PISD2(1);                   // golden fiosetview->ProcessIndexSuckDestroy2(1) -- no such method yet (see W5_32S_SEAM)  //JerryYang 20250120 : modify
            }

            if(DoTestYFrontDelay_32.Off() &&
               (bIndexSuckCheck1 &&
                bIndexSuckCheck2))                                              //Steven 20220622 : || --> && fixes drop-on-pause during drop-test
            {
                bIndexCheckNoStopVaccum=false;                                  //Sam 20220902 : fix All Site drop
                bBTestSuckDrop=false;
                Task=240;                                                       //jou 2015-04-29 fix 32 sites drop mode index crash
            }
            break;
        case 2000:
            if(REAL_TIME_CCD==true && COM2->bCCDDummyRum==false)
            {
                Task=65;
            }
            else
            {
                Task=81;
            }
            break;
        case 12000:                                                             //jou 2014-06-24 RTC auto model verify start
            Do32RTCAutoModelVerify(true);
            Task=12010;
            break;
        case 12010:
            if(Do32RTCAutoModelVerify(false))
            {
                Task=225;
            }
            break;
        default:
            break;
    }

    return false;
}
//------------------------------------------------------------------------------
int iTestSuckTestIC_TwoArm32Site_Task=1;
void InitTestSuckTestIC_TwoArm32Site_Task()                                     //2013-01-15    Dell    Add nn Mode
{
    iTestSuckTestIC_TwoArm32Site_Task =1;
}
//==============================================================================
//  DoTestSuckTestIC_TwoArm32Site (golden :2300-3138) -- drives the actual
//  test (TestStart/TestEnd handshake), reads back the tester result, folds it
//  into the test-socket grid, and handles interface-error / time-out / torque-
//  read side branches.  ACTIVE, faithful.  Substrate gates (all via
//  W5_32S_SEAM, see file banner): TestSocket.SplitSuck/SplitSuck1Row/MergeSuck/
//  MergeSuck1Row (NEW gap); fAutomation->TestMode (NEW gap);
//  ATC_InterfaceForm->SendHandler2DID; fNote->fShow; fMain->chkReadTorque1/2/
//  edTorue0/lbArm0Torque/CloseGpibProgram; TestSocket.AddTestResultRecord
//  (declared, no body anywhere -> gated).
//==============================================================================
TQPF_Timer DoFTestSuckTestICDelay_32;
bool DoTestSuckTestIC_TwoArm32Site()                                            //2013-01-15    Dell    Add nn Mode
{
    static int iTestCount=0, iReadTIntervel=0, iReadCount=0, iXMLRetryCnt=0;
    static bool bFirstIn=true;                                                  //JerryYang 20170610 (wei) only need to enter once, before testing the out-shuttle-lose-IC sites must be set to error bin
    static DWORD dwNowTickCount, dwOldTickCount=0;

    int &Task=iTestSuckTestIC_TwoArm32Site_Task;
    int iMaxDoubleContact=0, ret=0, ret2=0, iTestCh=0;
    int iErrCnt=0;
    int iNN=IsNNMode();
    bool bHasFailIC=false;
    bool bAlreadyTested;
    AnsiString ErrPart1, ErrPart2, ErrCnt;

    switch(Task)
    {
        case 1:
            iXMLRetryCnt=0;                                                     //Steven 20201102 : For Murata upload data
            bAlreadyTested=false;                                               //Steven 20170517 (jou) : add protection -- if IC already tested, leave
            ErrPart1="";
            ErrPart2="";
            iTestCh=0;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j]!=NULL_IC     &&
                       FTestSuck.Item[i][j]!=HAS_NULL_IC)
                    {
                        if(FTestSuck.Item[i][j]>=TEST_PASS)
                            bAlreadyTested=true;

                        if((CosFunction.bBarcodeErrNoTestAndShowH==true ||      //jou 20191007 : Barcode Error No Test & Show "H"
                            TestIF_File.iNoCodeDeviceToErr==2) &&               //Steven 20200909 : make 2DID all-site-fail an option
                           TestIF_File.bEnableBarCode==true &&
                           (FTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                            FTestSuck.cDeviceInf[i][j]==""))
                        {
                            ;
                        }
                        else
                        {
                            iTestCh++;                                          //Steven 20250318 : count how many sites need testing first
                        }
                    }

                    if(BTestSuck.Item[i][j]!=NULL_IC     &&
                       BTestSuck.Item[i][j]!=HAS_NULL_IC)
                    {
                        if(BTestSuck.Item[i][j]>=TEST_PASS)
                            bAlreadyTested=true;

                        if((CosFunction.bBarcodeErrNoTestAndShowH==true ||      //jou 20191007 : Barcode Error No Test & Show "H"
                            TestIF_File.iNoCodeDeviceToErr==2) &&               //Steven 20200909 : make 2DID all-site-fail an option
                           TestIF_File.bEnableBarCode==true &&
                           (BTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                            BTestSuck.cDeviceInf[i][j]==""))
                        {
                            ;
                        }
                        else
                        {
                            iTestCh++;                                          //Steven 20250318 : count how many sites need testing first
                        }
                    }
                }
            }

            if(bAlreadyTested==true)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        ErrPart1+=IndexSuckName[i+iNN][j]+", ";
                        ErrPart2+=IndexSuckName[i][j]+", ";
                        if(FTestSuck.Item[i][j]!=NULL_IC     &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                           FTestSuck.Item[i][j]<TEST_PASS)
                        {
                            FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                        }

                        if(BTestSuck.Item[i][j]!=NULL_IC     &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                           BTestSuck.Item[i][j]<TEST_PASS)
                        {
                            BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                        }
                    }
                }
                ShowMyMessage("IC in index already tested, abort process.", ErrPart1+"; "+ErrPart2);
                return true;
            }

            if(iTestCh==0)                                                      //Steven 20250318 : if nothing needs testing, return true
            {
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC &&
                           FTestSuck.Item[i][j]<TEST_PASS &&
                           FTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            if((CosFunction.bBarcodeErrNoTestAndShowH==true ||  //jou 20191007 : Barcode Error No Test & Show "H"
                                TestIF_File.iNoCodeDeviceToErr==2) &&           //Steven 20200909 : make 2DID all-site-fail an option
                               TestIF_File.bEnableBarCode==true &&
                               (FTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                                FTestSuck.cDeviceInf[i][j]==""))
                            {
                                FTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                FTestSuck.iBinData[i][j]=iTestBinCount;
                                W5_32S_ADD_TEST_RESULT_RECORD(FTestSuck, i, j, iTestBinCount, FTestSuck.cSBin[i][j], "NonTestToRBin"); // golden FTestSuck.PordRec[i][j].AddTestResultRecord(...) -- declared, no body anywhere (see W5_32S_SEAM)
                            }
                        }

                        if(BTestSuck.Item[i][j]!=NULL_IC &&
                           BTestSuck.Item[i][j]<TEST_PASS &&
                           BTestSuck.Item[i][j]!=HAS_NULL_IC)
                        {
                            if((CosFunction.bBarcodeErrNoTestAndShowH==true ||  //jou 20191007 : Barcode Error No Test & Show "H"
                                TestIF_File.iNoCodeDeviceToErr==2) &&           //Steven 20200909 : make 2DID all-site-fail an option
                               TestIF_File.bEnableBarCode==true &&
                               (BTestSuck.cDeviceInf[i][j]==asBarCodeErrorSend ||
                                BTestSuck.cDeviceInf[i][j]==""))
                            {
                                BTestSuck.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                BTestSuck.iBinData[i][j]=iTestBinCount;
                                W5_32S_ADD_TEST_RESULT_RECORD(BTestSuck, i, j, iTestBinCount, BTestSuck.cSBin[i][j], "NonTestToRBin"); // golden BTestSuck.PordRec[i][j].AddTestResultRecord(...) -- see W5_32S_SEAM
                            }
                        }
                    }
                }
                return true;
            }

            if(IniConfig.bD01EnableReadTorque &&                                //Steven 20100617 Start: Add form 9080A for live torque update
               IniConfig.iD01ReadTorqueTimeCount>0)
            {
                iReadTIntervel++;
                if(iReadTIntervel>IniConfig.iD01ReadTorqueTimeCount)
                {
                    iReadTIntervel=0;
                    bReadRearTestArmTorque=true;
                }
            }

            if(CosFunction.bSocketSensorCheckICAtArmDown==true)
            {
                CheckICExistInSocket(__FUNC__);
            }
            #ifdef SOFT_SIMULTE
                bReadRearTestArmTorque=false;
            #endif
            if(IniConfig.bD02OffReadTorqueDuringTest==true)
                bReadRearTestArmTorque=false;

            if(bReadRearTestArmTorque)
            {
                bReadRearTestArmTorque=false;
                W5_32S_FMAIN_CHKREADTORQUE1->Checked=true;                     // golden fMain->chkReadTorque1->Checked=true -- no such widget yet (see W5_32S_SEAM)
                W5_32S_FMAIN_CHKREADTORQUE2->Checked=true;                     // golden fMain->chkReadTorque2->Checked=true
                Task=5000;
                break;
            }
            else
            {
                if(IsNNMode()==NN_1Row)
                    W5_32S_MERGESUCK(TestSocket, BTestSuck, FTestSuck, false);  // golden TestSocket.MergeSuck1Row(BTestSuck,FTestSuck) -- no such method yet (see W5_32S_SEAM)
                else
                    W5_32S_MERGESUCK(TestSocket, BTestSuck, FTestSuck, false);  // golden TestSocket.MergeSuck(BTestSuck,FTestSuck,false)
                bDoubleContact=false;
                InitTestTask();
                iTestCount=0;
                ZeroMemory(iReContactCnt, sizeof(iReContactCnt));               //Steven 20231205 : per-site contact-count tally

                if(CUSTOMER_CODE==CC_MTI ||
                   CUSTOMER_CODE==CC_PTI)                                       //Sam 20190429 : Add CC_PTI_NEWWORK
                {
                    InitDoStartMode();                                          //ChungHung 聚成 Init Test Start
                }

                Task=2200;
            }
        case 2200:
            if(CUSTOMER_CODE==CC_MTI ||                                         //ChungHung 聚成 Test Start
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                if(DoStartMode(W5_32S_FAUTOMATION_TESTMODE)==false)             // golden DoStartMode(fAutomation->TestMode) -- TfAutomationShim has no TestMode member yet (see W5_32S_SEAM)
                    return false;
            }

            ProcessStartTestData(0);
            SetTestTimeOutTimer(0);                                             //Steven 20200407 : integrated Time Out setting

            HangTime.SetSecAndOn(Prod.iHangupMaxTime);

            if(CUSTOMER_CODE==CC_MTI ||
               CUSTOMER_CODE==CC_PTI)                                           //Sam 20190429 : Add CC_PTI_NEWWORK
            {
                InitDoEndMode();                                                //ChungHung 聚成 only
            }

            if(IniConfig.bL10IndexTestlogTemp)                                  //kevin 20190323 : record temperature only during index test
                TemperatureStorageLog(1);                                       //kevin 20190323 add Steven 20140617 : for HiSilicon
            Task=2400;
        case 2400:                                                                                                                                              // number must not change *******************
            if(dwOldTickCount==0)
            {
                dwOldTickCount=MyTickCount();
                dwNowTickCount=dwOldTickCount;
                SetTestTimeOutTimer(0);                                                                                                                         //Steven 20200407 : integrated Time Out setting
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);
            }
            else
            {
                dwNowTickCount=MyTickCount();
                if(dwNowTickCount<dwOldTickCount)
                {
                    dwOldTickCount=dwNowTickCount;                                                                                                              //Jou 20101018 Start : re-setting Time Out time must also reset Hang Up time
                    SetTestTimeOutTimer(0);                                                                                                                     //Steven 20200407 : integrated Time Out setting
                    HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                }
                else
                {
                    if(bHandlerPause || bTesterSendPause)
                    {
                        if(CosFunction.bStopMustTestTimeOut==false ||
                           bEnterTestIF==true ||
                           bTesterSendPause)
                        {
                            bEnterTestIF=false;
                            SetTestTimeOutTimer(0);                                                                                                             //Steven 20200407 : integrated Time Out setting
                        }

                        HangTime.SetSecAndOn(Prod.iHangupMaxTime);
                    }
                    dwOldTickCount=dwNowTickCount;
                }
            }

            if(CosFunction.bOutShtLoseICSetErrUntilOneCycle==true &&                                                                                            //Steven 20210904 : cover 32site too
               bFirstIn==true)                                                                                                                                  //JerryYang 20170610 (wei) only need to enter once, before testing the out-shuttle-lose-IC sites must be set to error bin
            {
                bFirstIn=false;
                if(bOutShtLoseICNeedSetErrBin==true)
                {
                    if(IsNNMode()==NN_1Row)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(bTestSiteNeedSetErrBin[2][j]==true &&                                                                                            //Shuttle 1
                               TestSocket.Item[1][j]!=NULL_IC     &&
                               TestSocket.Item[1][j]!=HAS_NULL_IC &&
                               TestSocket.Item[1][j]<TEST_PASS    )
                            {
                                TestSocket.SetItemData(1, j, TEST_PASS+iTestBinCount);
                                iErrCnt++;
                            }

                            if(bTestSiteNeedSetErrBin[0][j]==true &&                                                                                            //Shuttle 2
                               TestSocket.Item[0][j]!=NULL_IC     &&
                               TestSocket.Item[0][j]!=HAS_NULL_IC &&
                               TestSocket.Item[0][j]<TEST_PASS    )
                            {
                                TestSocket.SetItemData(0, j, TEST_PASS+iTestBinCount);
                                iErrCnt++;
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(bTestSiteNeedSetErrBin[i+2][j]==true &&
                                   TestSocket.Item[i+2][j]!=NULL_IC     &&
                                   TestSocket.Item[i+2][j]!=HAS_NULL_IC &&
                                   TestSocket.Item[i+2][j]<TEST_PASS)
                                {
                                    TestSocket.SetItemData(i+2, j, TEST_PASS+iTestBinCount);
                                    iErrCnt++;
                                }

                                if(bTestSiteNeedSetErrBin[i][j]==true &&
                                   TestSocket.Item[i][j]!=NULL_IC     &&
                                   TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                   TestSocket.Item[i][j]<TEST_PASS)
                                {
                                    TestSocket.SetItemData(i, j, TEST_PASS+iTestBinCount);
                                    iErrCnt++;
                                }
                            }
                        }
                    }

                    if(iErrCnt>0)
                    {
                        ErrCnt.sprintf("Shuttle lose IC, Arm 1/2 set to error Bin (before test) : %d pcs", iErrCnt);
                        MyDBIProcess("Message", ErrCnt);
                    }
                }
            }

            bFinshTest=false;

            if(bResetMode ||                                                                                                                                    //jou 981219 start : reset mode
               bResetModeAndCleanOut ||                                                                                                                         //JerryYang 20151007 : ResetMode can choose OneCycle or CleanOut
               (bOneTimeHotPlateCheckAll &&                                                                                                                     //Steven 20120315 : HotPlate Check after CleanOut, must go to Error Bin
                IniConfig.bE39_1PutTheDevicesToErrorBin &&
                bHotPlateCheckNeedTrayFeed))
            {
                ret=1;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(TestSocket.Item[i][j]!=NULL_IC &&
                           TestSocket.Item[i][j]!=HAS_NULL_IC &&
                           TestSocket.Item[i][j]<TEST_PASS)
                        {
                            if(bDoEmptySocketOneCycle)                                                                                                          //Steven 20220817 : Bin of ESC function iTestBinCount --> IniConfig.iI41_BinOfESC
                            {
                                iTesterBIN[i][j]=IniConfig.iI41_BinOfESC;
                            }
                            else
                            {
                                iTesterBIN[i][j]=iTestBinCount;
                            }
                            TestSocket.SetItemData(i, j, TEST_PASS+iTesterBIN[i][j]);
                            W5_32S_ADD_TEST_RESULT_RECORD(TestSocket, i, j, iTesterBIN[i][j], TestSocket.cSBin[i][j], "RESET_Mode"); // golden TestSocket.PordRec[i][j].AddTestResultRecord(...) -- see W5_32S_SEAM //Frank 20160505 add
                        }
                    }
                }
            }
            else
            {
                if(DeviceForm.DummyMode)                                                                                                                        //contect form
                {
                    if(iTestCount<1)
                    {
                        ret=1;
                    }
                    else
                    {
                        ret=ProcessTestResult(0);
                    }
                }
                else
                {
                    ret=ProcessTestResult(0);
                }
            }

            if(ret==1)
            {
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) only need to enter once, before testing the out-shuttle-lose-IC sites must be set to error bin
                if(bATCHasAlarmBinNeedToError)                                                                                                                  //Ifor 20160726 add ATC-abnormal IC must go to Error Bin
                {
                    iDoInterFaceErrorStepTask=1;
                    bContactCTOverCHK=false;                                                                                                                    //Ifor 20160726 avoid ATC-abnormal-IC-to-error-bin flow being disturbed
                    Task=2500;                                                                                                                                  //skip the flow below
                    break;
                }

                bInitialMaxTime=false;                                                                                                                          //jou 2011-11-09 add initial max time set
                SetHangupMaxTime();                                                                                                                             //Wei 20230224 : reset hang-time
                if(CUSTOMER_CODE==CC_MTI || CUSTOMER_CODE==CC_PTI)                                                                                              //Sam 20190429 : Add CC_PTI_NEWWORK
                {
                    if(DoEndMode(W5_32S_FAUTOMATION_TESTMODE)==false)                                                                                           // golden DoEndMode(fAutomation->TestMode) -- see W5_32S_SEAM  //ChungHung 聚成 Test End
                        return false;
                }

                SW[SwTesterPower].Off();
                bHasFailIC=false;
                for(int i=0; i<TestSocket.iShtRow; i++)
                {
                    for(int j=0; j<TestSocket.iShtCol; j++)
                    {
                        if(IniConfig.bD22_4_PassBinCanDoubleContact)                                                                                            //JerryYang 20230909 : pass bin can also be set to Double contact
                        {
                            if(TestSocket.Item[i][j]!=NULL_IC  &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC &&
                               TestSocket.bNeedReTest[i][j])
                            {
                                bHasFailIC=true;
                            }
                        }
                        else
                        {
                            if(TestSocket.Item[i][j]!=NULL_IC  &&
                               TestSocket.Item[i][j]!=HAS_NULL_IC &&
                               TestSocket.bPass[i][j]==false   &&
                               TestSocket.bNeedReTest[i][j])
                            {
                                bHasFailIC=true;
                            }
                        }
                    }
                }

                if(bHasFailIC || DeviceForm.DummyMode ||
                   bNeedReplunge_RFMD)                                                                                                                          //Steven 20201022 : For RFMD
                {
                    iTestCount++;
                    iMaxDoubleContact=2;
                    if(Prod.bD22SupportMultiDoubleContact)
                    {
                        if(CUSTOMER_CODE==CC_TERAPOWER && iTestCount==1)                                                                                        //Sam 20180612 : when D22 Double Contact is on, use the first test result for ProcessCount
                        {
                            for(int i=0; i<TestSocket.iShtRow; i++)
                            {
                                for(int j=0; j<TestSocket.iShtCol; j++)
                                {
                                    W5_32S_BINDATA_BACKUP(i,j); // golden TestSocket.iBinDataBackUp[i][j]=TestSocket.iBinData[i][j] -- see W5_32S_SEAM
                                }
                            }
                            ProcessCount(0);
                        }

                        iMaxDoubleContact=Prod.iD22DoubleContactCount+2;                                                                                        //Sam 20231117 : integrated into QA mode
                        if(iMaxDoubleContact<2)
                            iMaxDoubleContact=2;
                        if(iMaxDoubleContact>10)
                            iMaxDoubleContact=10;
                    }

                    if(bNeedReplunge_RFMD)                                                                                                                      //Steven 20201022 : For RFMD
                    {
                        bNeedReplunge_RFMD=false;
                        ProcessShowTestStatus(0);
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(TestSocket.Item[i][j]!=NULL_IC     &&
                                   TestSocket.Item[i][j]!=HAS_NULL_IC)
                                {
                                    TestSocket.SetItemData(i, j, HAS_IC);
                                }
                            }
                        }
                        bDoubleContact=true;
                        Task=3000;
                        break;
                    }
                    else if(iTestCount<iMaxDoubleContact)
                    {
                        ProcessShowTestStatus(0);
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                if(IniConfig.bD22_4_PassBinCanDoubleContact)                                                                                    //JerryYang 20230909 : pass bin can also be set to Double contact
                                {
                                    if(TestSocket.Item[i][j]!=NULL_IC     &&
                                       TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                       TestSocket.bNeedReTest[i][j])
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);
                                    }
                                }
                                else
                                {
                                    if(TestSocket.Item[i][j]!=NULL_IC     &&
                                       TestSocket.Item[i][j]!=HAS_NULL_IC &&
                                       TestSocket.bPass[i][j]==false      &&
                                       TestSocket.bNeedReTest[i][j])
                                    {
                                        TestSocket.SetItemData(i, j, HAS_IC);
                                    }
                                }
                            }
                        }
                        bDoubleContact=true;
                        Task=3000;
                        break;
                    }
                }

                if(TestSocket.HasRealIC())                                                                                                                      //Steven 20210218 : fix test-time recording
                    ret2=RecordEndTestTime(2);                                                                                                                  //Sam 20201231 : fix Index Cycle time anomaly after closing an Arm.  0:arm1 1:arm2 2:both-arm
                else
                    ret2=1;

                SetNoiseDelay=false;
                TestISTimeOut=false;

                if(CUSTOMER_CODE==CC_TERAPOWER &&                                                                                                               //Sam 20180612 : when D22 Double Contact is on, use the first test result for ProcessCount
                   Prod.bD22SupportMultiDoubleContact)
                {
                    if(iTestCount==0)
                    {
                        for(int i=0; i<TestSocket.iShtRow; i++)
                        {
                            for(int j=0; j<TestSocket.iShtCol; j++)
                            {
                                W5_32S_BINDATA_BACKUP(i,j); // golden TestSocket.iBinDataBackUp[i][j]=TestSocket.iBinData[i][j] -- see W5_32S_SEAM
                            }
                        }
                        ProcessCount(0);
                    }
                }
                else
                {
                    ProcessCount(0);
                }

                RecordPiggyBackStartEnd(true);                                                                                                                  //jou 2011-12-26 this function must run after ProcessCount

                #ifdef TEST_BIN_MISS_SIMULATE
                if(MakeNoise==true)
                {
                    if(TestSocket.Item[0][0]==(TEST_PASS+1))
                        TestSocket.Item[0][0]=(TEST_PASS+2);
                    else
                        TestSocket.Item[0][0]=(TEST_PASS+1);
                    MakeNoise=false;
                }
                #endif

                if(TestSocket.UseSiteHasIC())                                                                                                                   //KenHsieh 20231208 : only push when there is data, avoid overwriting with NULL by moving repeatedly
                {
                    if(IsNNMode()==NN_1Row)
                        W5_32S_SPLITSUCK(TestSocket, BTestSuck, FTestSuck, false); // golden TestSocket.SplitSuck1Row(BTestSuck,FTestSuck) -- see W5_32S_SEAM
                    else
                        W5_32S_SPLITSUCK(TestSocket, BTestSuck, FTestSuck, false); // golden TestSocket.SplitSuck(BTestSuck,FTestSuck,false)  //2013-01-15    Dell    Add nn Mode
                }
                RecordHistroy(0);
                bFinshTest=true;
                bInitStartDelayNotFinish=true;                                                                                                                  //Ifor 20181220 : add Init Start Delay Time Not Finish
                bTJControlMode=false;                                                                                                                           //Ifor 20190328 : add TJ Temp Over Range
                W5_32S_ATC_SENDHANDLER2DID(0, false);                                                                                                           // golden ATC_InterfaceForm->SendHandler2DID(0,false) -- see W5_32S_SEAM
                if(IniConfig.bTesterTimeUpErrorNeedPassword==true &&
                   CUSTOMER_CODE==CC_LINGSEN &&                                                                                                                 //jou 2012-08-28 LingSen requires the two tests after a Test-Time-Up Error to route to R regardless of result
                   iTestTimeUpErrContinueR>0)
                {
                    iTestTimeUpErrContinueR--;
                    for(int i=0; i<TestSocket.iShtRow; i++)
                    {
                        for(int j=0; j<TestSocket.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]!=NULL_IC && FTestSuck.Item[i][j]!=HAS_NULL_IC)
                                FTestSuck.SetItemData(i, j, TEST_PASS+15);
                            if(BTestSuck.Item[i][j]!=NULL_IC && BTestSuck.Item[i][j]!=HAS_NULL_IC)
                                BTestSuck.SetItemData(i, j, TEST_PASS+15);
                        }
                    }
                    MyDBIProcess("Message", "Arm 1/Arm 2 Tester Time Up error place to R Bin : "+AnsiString(FTestSuck.CountRealIC())+" pcs" );
                }

                if(ret2!=1)                                                                                                                                     //Steven 20201102 : For Murata upload data
                {
                    iXMLRetryCnt++;
                    DoFTestSuckTestICDelay_32.SetSecAndOn(60);
                    Task=6000;
                }
                else
                {
                    iXMLRetryCnt=0;                                                                                                                             //Steven 20201102 : For Murata upload data
                    if(CheckContactOver())
                    {
                        iDoInterFaceErrorStepTask=1;
                        bContactCTOverCHK=true;
                        Task=2500;
                    }
                    else
                    {
                        return true;
                    }
                }
            }
            else if(ret==2)                                                                                                                                     //interface error
            {
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) only need to enter once, before testing the out-shuttle-lose-IC sites must be set to error bin
                if(bATCHasAlarmBinNeedToError)                                                                                                                  //Ifor 20160726 add ATC-abnormal IC must go to Error Bin
                {
                    iDoInterFaceErrorStepTask=1;
                    bContactCTOverCHK=false;                                                                                                                    //Ifor 20160726 avoid ATC-abnormal-IC-to-error-bin flow being disturbed
                    Task=2500;                                                                                                                                  //skip the flow below
                    break;
                }
                RecordPiggyBackStartEnd(true);
                bInitialMaxTime=false;                                                                                                                          //jou 2011-11-09 add initial max time set
                SetHangupMaxTime();                                                                                                                             //Wei 20230224 : reset hang-time
                SetNoiseDelay=false;
                Task=2200;
            }
            else if((LastSet.iTester==ON_LINE &&                                                                                                                //Steven 20150713 : cleaned up LastSet.iTester
                     hFTestTimeOutDelay.Off()) ||                                                                                                               //Jou 20101018  //Time Out
                    (bEcho && bTimeOutForNoFullSite==true))                                                                                                     //Steven 20141016 : FullSite's own Test Time Out
            {
                bFirstIn=true;                                                                                                                                  //JerryYang 20170610 (wei) only need to enter once, before testing the out-shuttle-lose-IC sites must be set to error bin
                if(bATCHasAlarmBinNeedToError)                                                                                                                  //Ifor 20160726 add ATC-abnormal IC must go to Error Bin
                {
                    iDoInterFaceErrorStepTask=1;
                    bContactCTOverCHK=false;                                                                                                                    //Ifor 20160726 avoid ATC-abnormal-IC-to-error-bin flow being disturbed
                    Task=2500;                                                                                                                                  //skip the flow below
                    break;
                }

                RecordPiggyBackStartEnd(true);
                if(IniConfig.bD52InterFaceErrHeadNeedUp)
                {
                    if(CosFunction.bStopMustTestTimeOut &&                                                                                                      //Steven 20200330 : Pause also needs Time out, conflicts with [D52]
                       (SoftStop || SystemStart==false))
                    {
                    }
                    else
                    {
                        iDoInterFaceErrorStepTask=1;
                        Task=2500;                                                                                                                              //skip the flow below
                        bD52IndexArmUp=true;                                                                                                                    //JerryYang 20200804 : fix D52 & Index-arm-preheat-on-shuttle-height feature both enabled falsely raising a handler hang up on tester timeout
                        break;
                    }
                }

                if((LastSet.iTester==ON_LINE && hFTestTimeOutDelay.Off()) ||                                                                                    //Jou 20101018   //Steven 20150713 : cleaned up LastSet.iTester
                   (bEcho && bTimeOutForNoFullSite==true))                                                                                                      //Steven 20141016 : FullSite's own Test Time Out
                {
                    if(W5_32S_FNOTE_FSHOW)                                                                                                                      // golden fNote->fShow -- no translated home yet (see W5_32S_SEAM)  //JerryYang 20200408 : fix so Test time out still fires while an Alarm screen is up
                        break;
                    bEcho=false;                                                                                                                                //Steven 20150306 : Fixed for FullSite's own Test Time Out
                    bTimeOutForNoFullSite=false;
                    ret=ProcessTesterTimeOut(0);

                    if(ret==2)
                    {
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
                           CUSTOMER_CODE==CC_ASE_KaohSiung_K12)                                                                                                 //Steven 20131101 : Add ASE-K12
                            InitTestTask();
                        SetNoiseDelay=false;
                        TestISTimeOut=true;

                        if(TestIF.iTestType==GPIB_MODE ||
                           TestIF.iTestType==TCP_IP_MODE)                                                                                                       //wei 20211027 open short TCP/IP
                        {
                            if(IniConfig.bRetryNoNeedRestartGpib)                                                                                               //Steven 20111220 : test time-out retry does not need to reopen GPIB
                            {                                                                                                                                   //connecting to an OT BOX does not need to reopen GPIB
                                if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)                                                                            //Steven 20181121 : fixed a re-test bug after time out
                                    InitTestTask();
                                Task=2200;
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                        else
                        {
                            Task=2200;
                        }

                        bTimeOutForNoFullSite=false;                                                                                                            //ChungHung 20141017 fix Full Site Test Time Out problem
                    }
                    else
                    {
                        if(ret==1)
                            ProcessTestResult(0);
                        SetNoiseDelay=false;
                        TestISTimeOut=false;
                        ProcessCount(0);
                        if(TestSocket.UseSiteHasIC())                                                                                                           //KenHsieh 20231208 : only push when there is data, avoid overwriting with NULL by moving repeatedly
                        {
                            if(IsNNMode()==NN_1Row)
                                W5_32S_SPLITSUCK(TestSocket, BTestSuck, FTestSuck, false); // golden TestSocket.SplitSuck1Row(BTestSuck,FTestSuck) -- see W5_32S_SEAM
                            else
                                W5_32S_SPLITSUCK(TestSocket, BTestSuck, FTestSuck, false); // golden TestSocket.SplitSuck(BTestSuck,FTestSuck,false)  //2013-01-15    Dell    Add nn Mode  //Steven 20140815 : Add For 32 Site
                        }
                        RecordHistroy(0);
                        bFinshTest=true;
                        bInitStartDelayNotFinish=true;                                                                                                          //Ifor 20181220 : add Init Start Delay Time Not Finish
                        bTJControlMode=false;                                                                                                                   //Ifor 20190328 : add TJ Temp Over Range
                        bTimeOutForNoFullSite=false;                                                                                                            //ChungHung 20141017 fix Full Site Test Time Out problem
                        W5_32S_ATC_SENDHANDLER2DID(0, false);                                                                                                   // golden ATC_InterfaceForm->SendHandler2DID(0,false) -- see W5_32S_SEAM
                        SW[SwTesterPower].Off();
                        if(CheckContactOver())
                        {
                            iDoInterFaceErrorStepTask=1;
                            bContactCTOverCHK=true;
                            Task=2500;
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
            }
            break;
        case 2500:
            if(SoftStop || SystemStart==false)                                  //Steven 20111004
                break;

            if(DoInterFaceErrorStep_TwoArm32Site())                             //ChungHung 20140516 32Site both arms must come up together  //Steven 20140815 : Add For 32 Site
            {
                HangTime.SetSecAndOn(Prod.iHangupMaxTime);                      //Steven 20090827 Start: Hang Up dectector
                bHangTimePause=true;
                return true;
            }
            break;
        case 2600:
            if(SoftStop || SystemStart==false)
                break;

            if(IniConfig.bI12TesterTimerOutNotNeedReTest==false)                //Steven 20111214 : if not re-testing, cannot close GPIB  //steven 2011-12-16 true -> false bTesterTimerOutNotNeedReTest logic reversed
            {
#if 0 // TODO(W7-UI) -- golden :3009 fMain->CloseGpibProgram(AnsiString) (TfMain has no such method yet, golden main.h)
                fMain->CloseGpibProgram(__FUNC__);
#else
                W5_32S_FMAIN_CLOSEGPIBPROGRAM(AnsiString(__FUNC__));
#endif
            }

            DoFTestSuckTestICDelay_32.SetSecAndOn(0.3);
            Task=2700;
            break;
        case 2700:
            if(DoFTestSuckTestICDelay_32.Off())
                Task=2200;
            break;
        case 3000:
            #ifdef SOFT_SIMULTE
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+20000, 30*300))
                    Task=3100;
                break;
            #else
                if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test+5000, 30*3000))
                    Task=3100;
                break;
            #endif
        case 3100:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Test,30*3000))
            {
                InitTestTask();
                Task=2200;
            }
            break;
        case 5000:                                                              //Steven 20100617 Start: Add form 9080A for live torque update
            DoFTestSuckTestICDelay_32.SetSecAndOn(0.01);                        // for switch read torque relay
            Task=5100;
            break;
        case 5100:
            if(DoFTestSuckTestICDelay_32.Off())
                Task=5200;
            break;
        case 5200:
            W5_32S_FMAIN_CHKREADTORQUE1->Checked=true;                         //2008/06/24 lee -- golden fMain->chkReadTorque1->Checked (see W5_32S_SEAM)
            W5_32S_FMAIN_CHKREADTORQUE2->Checked=false;                        //2008/06/24 lee

            W5_32S_FMAIN_EDTORUE0->Text="";                                     // golden fMain->edTorue0->Text (see W5_32S_SEAM)
            W5_32S_FMAIN_LBARM0TORQUE->Caption="1:Reading";                     // golden fMain->lbArm0Torque->Caption (see W5_32S_SEAM)
            iReadCount=0;
            DoFTestSuckTestICDelay_32.SetSecAndOn(0.2);                         //2008/07/15 lee
            Task=5300;
            break;
        case 5300:
            W5_32S_FMAIN_CHKREADTORQUE1->Checked=true;                         //2008/07/15 lee
            W5_32S_FMAIN_CHKREADTORQUE2->Checked=false;                        //2008/07/15 lee

            if(W5_32S_FMAIN_EDTORUE0->Text!="")
            {
                iReadCount++;
                if(iReadCount>1)                                                //2008/07/15 lee
                {
                    ShowMainScreenPresure(0);
                    if(atoi(W5_32S_FMAIN_EDTORUE0->Text.c_str())>=Prod.iMaxPreasure)
                        Task=5400;
                    else
                        Task=1;
                }
                else
                {
                    W5_32S_FMAIN_EDTORUE0->Text="";
                    COM2->InitReadTorueTask();
                }
            }
            else if(DoFTestSuckTestICDelay_32.Off())
            {
                ShowMainScreenPresure(0);
                Task=1;
            }
            break;
        case 5400:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "DoTestSuckTestIC_TwoArm32Site 5400"))
                Task=5500;
            break;
        case 5500:
            ShowMyMessage("Index 1 contact force over error", "Index 1 扭力過大錯誤!!", "DoFTestSuckTestIC");
            fAllMotorHome=false;
            iHome=1;
            Task=1;
            break;
        case 6000:                                                              //Steven 20201102 : For Murata upload data
            if(iXMLRetryCnt>=3)                                                 //Steven 20201113 : 3 upload failures need an Alarm
            {
                ret=ShowErrorMessage("WAR16321", K_RETRY|K_RESET, MTestZ1, false);
                if(ret==K_RESET)
                {
                    fMain->Reset("DoTestSuckTestIC_TwoArm32Site");
                    Task=6100;
                }
                else
                {
                    iXMLRetryCnt=0;
                    DoFTestSuckTestICDelay_32.SetSecAndOn(30);
                    Task=6000;
                }
            }
            else if(DoFTestSuckTestICDelay_32.Off())
            {
                ret2=SendTestResultToHttp();

                if(ret2!=1)
                {
                    iXMLRetryCnt++;
                    DoFTestSuckTestICDelay_32.SetSecAndOn(30);
                    Task=6000;
                }
                else
                {
                    Task=6100;
                }
            }
            break;
        case 6100:
            iXMLRetryCnt=0;
            if(CheckContactOver())
            {
                iDoInterFaceErrorStepTask=1;
                bContactCTOverCHK=true;
                Task=2500;
            }
            else
            {
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
//  Do32SiteTestDestroyIC (golden :3141-3684) -- 32-site dual-arm "destroy"
//  (drop tested IC to the out-shuttle) hand-off.  Shares its Task cursor with
//  aTester_Front.cpp's DoFrontTestDestroyIC (iFrontTestDestroyICTask) -- a
//  genuine golden quirk (both functions are mutually exclusive at runtime,
//  gated by IniConfig.bD59_32SitePnpTogether in DoTestY_TwoArm32Site case 210)
//  preserved faithfully, NOT "fixed."  ACTIVE, faithful.  Substrate gates all
//  mirror aTester_Front.cpp's OWN already-reviewed gate list for the identical
//  single-arm idiom (gates #1/#3/#4/#6/#8/#9 in that file's header banner):
//  bNeedCheck[][], MoveSuckData, AddIndexPlaceShuttleRecord,
//  LastSet.iIndexInputOutPut[], ProcessIndexSuckDestroy1/2(int),
//  SetHasNullIcToNullIc().
//------------------------------------------------------------------------------
TQPF_Timer Do32TestDestroyICDelay;
bool Do32SiteTestDestroyIC()                                                    //Steven 20150910 : 32Site dual-arm suck/destroy together
{
    static int iRetry=0;
    static bool bFTDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false}};

    static bool bBTDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false}};

    int &Task=iFrontTestDestroyICTask;                                          // golden shared cursor with DoFrontTestDestroyIC -- see banner
    int iPos=0;
    int sp;
    int iNN=IsNNMode();

    bool flag, flag1, flag2, bIndexSuckCheck1, bIndexSuckCheck2;
    bool bHasDuplicateErr=false;
    bool bHasErr=false;

    AnsiString sTempMessge;
    AnsiString sBuffer="";
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:
            MOT[MInShuttle1].fCanMoveM=false;
            MOT[MInShuttle2].fCanMoveM=false;
            if(InShtInLF(0)==false)
            {
                ShowMyMessage("Shuttle 1 position error ", "Shuttle 1 位置錯誤");
                MOT[MInShuttle1].fCanMoveM=true;
                Task=1;
            }
            else if(InShtInLF(1)==false)
            {
                ShowMyMessage("Shuttle 2 position error ", "Shuttle 2 位置錯誤");
                MOT[MInShuttle2].fCanMoveM=true;
                Task=1;
            }
            else
            {
                flag2=false;
                flag1=false;
                if(FTestSuck.HasRealIC()==false)
                    flag1=true;
                if(BTestSuck.HasRealIC()==false)
                    flag2=true;

                if(IniConfig.bD54SlowDown &&
                  (flag1==false || flag2==false))                               //Sam 20240904 : still finish the move even with no IC, avoid a downstream IndexArmPosition error
                {
                    sp=GetIndexZSpeed(0);                                       //Steven 20160524 : Index Z軸速度整合為Function
                    if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Place, sp, "Do32SiteTestDestroyIC 1", true, Prod.TestZ2_Place))
                    {
                        FTestSuck.ResetAll();
                        BTestSuck.ResetAll();
                        Task=310;
                    }
                }
                else
                {
                    if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Place, MOT[MTestZ1].GailSpeed, "Do32SiteTestDestroyIC 1", true, Prod.TestZ2_Place))
                    {
                        FTestSuck.ResetAll();
                        BTestSuck.ResetAll();
                        Task=310;
                    }
                }
            }
            break;
        case 310:
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j])
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                           FTestSuck.Suck[i][j].Destroy())                                                              //drop the IC
                        {
                            if(IniConfig.bD44CheckIndexICDestroy)
                            {
                                if(FTestSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    FTestSuck.Suck[i][j].Off();                                                         //Suck Off and Destroy On
                                    W5_32S_NEEDCHECK_SET(FTestSuck, i, j, true);                                        // golden FTestSuck.bNeedCheck[i][j]=true -- see W5_32S_SEAM   //Destroy finish
                                    iIndexTakeDeviceChk1=2;                                                             //kevin 20190103 re-suck check state
                                }
                            }

                            if(FTestSuck.Item[i][j]!=HAS_NULL_IC)                                                       //kevin 20130125
                            {
                                W5_32S_LASTSET_INDEXINOUT_INC(1);                                                       // golden LastSet.iIndexInputOutPut[1]++ -- see W5_32S_SEAM  //kevin 20130125
                            }
                            W5_32S_MOVESUCKDATA(FRCarryKit, FTestSuck, i, j);                                           // golden FRCarryKit.MoveSuckData(FTestSuck,i,j) -- see W5_32S_SEAM
                            W5_32S_ADD_INDEX_PLACE_SHUTTLE_RECORD(FRCarryKit, i, j, TestIF.iSiteMap[i+iNN][j], i+iNN, j); // golden FRCarryKit.PordRec[i][j].AddIndexPlaceShuttleRecord(...) -- see W5_32S_SEAM  //Sam 20201216 : Add record  //Sam 20200716 : Add record
                            bFTDuplicateErr[i][j]=false;
                        }
                    }

                    if(BTestSuck.Item[i][j])
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                           BTestSuck.Suck[i][j].Destroy())
                        {
                            if(IniConfig.bD44CheckIndexICDestroy)
                            {
                                if(BTestSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    BTestSuck.Suck[i][j].Off();
                                    W5_32S_NEEDCHECK_SET(BTestSuck, i, j, true);                                        // golden BTestSuck.bNeedCheck[i][j]=true -- see W5_32S_SEAM
                                    iIndexTakeDeviceChk2=2;                                                             //kevin 20190103 re-suck check state
                                }
                            }

                            if(BTestSuck.Item[i][j]!=HAS_NULL_IC)
                            {
                                W5_32S_LASTSET_INDEXINOUT_INC(1);                                                       // golden LastSet.iIndexInputOutPut[1]++ -- see W5_32S_SEAM  //kevin 20130125
                            }
                            W5_32S_MOVESUCKDATA(BRCarryKit, BTestSuck, i, j);                                           // golden BRCarryKit.MoveSuckData(BTestSuck,i,j) -- see W5_32S_SEAM
                            W5_32S_ADD_INDEX_PLACE_SHUTTLE_RECORD(BRCarryKit, i, j, TestIF.iSiteMap[i][j], i, j);       // golden BRCarryKit.PordRec[i][j].AddIndexPlaceShuttleRecord(...) -- see W5_32S_SEAM  //Sam 20201216 : Add record //Sam 20200716 : Add record
                            bBTDuplicateErr[i][j]=false;
                        }
                    }
                }
            }

            flag=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FTestSuck.Item[i][j] && FTestSuck.Suck[i][j].Error==false)
                    {
                        flag=true;
                        break;
                    }

                    if(BTestSuck.Item[i][j] && BTestSuck.Suck[i][j].Error==false)
                    {
                        flag=true;
                        break;
                    }
                }
            }

            if(flag==false)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            Task=320;
                            return false;
                        }

                        if(BTestSuck.Suck[i][j].Error)
                        {
                            Task=320;
                            return false;
                        }
                    }
                }
            }

            if(FTestSuck.UseSiteHasIC() ||
               BTestSuck.UseSiteHasIC())
            {
                iRetry=0;
                break;
            }

            iRetry=0;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    bFTDuplicateErr[i][j]=false;
                }
            }
            Task=500;
            break;
        case 320:                                                                                                       //destroy error handling
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iNN][j];                                                               //kevin 20180514 arm 1 +2 needs to map into the 32-site alarm position correctly
                        FTestSuck.Suck[i][j].Error=false;
                        bFTDuplicateErr[i][j]=true;
                    }
                    else
                    {
                        bFTDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                               //Vacuum sensor OFF error

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];
                        BTestSuck.Suck[i][j].Error=false;                                                               //Steven 20101229 : moved position
                        bBTDuplicateErr[i][j]=true;                                                                     //Steven 20101229 : moved position
                    }
                    else
                    {
                        bBTDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);                               //Vacuum sensor OFF error

            bHasErr=false;
            Task=310;
            break;
        case 500:
            if(IniConfig.bD44CheckIndexICDestroy)
            {
                if(MOT[MTestZ1].Gali_ReadPos()<(Prod.TestZ1_Place+10))          //kevin 20130719  blowing too high would blow the IC away
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(W5_32S_NEEDCHECK_GET(FTestSuck, i, j))            // golden FTestSuck.bNeedCheck[i][j] -- see W5_32S_SEAM   //Suck Off Destroy On
                            {
                                FTestSuck.Suck[i][j].Off();                     //kevin 20130615 close vacuum, open destroy after placing on shuttle
                            }
                        }
                    }
                }
                else                                                            //kevin 20130615 rising after placing on shuttle -> close destroy
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(W5_32S_NEEDCHECK_GET(FTestSuck, i, j))            // Destroy Off
                            {
                                FTestSuck.Suck[i][j].OffDestroy();              //kevin 20130719 close blow-off
                            }
                        }
                    }
                }

                if(MOT[MTestZ2].Gali_ReadPos()<(Prod.TestZ2_Place+10))          //kevin 20130719 close blow-off
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(W5_32S_NEEDCHECK_GET(BTestSuck, i, j))            //  kevin 20130615 close vacuum, open destroy after placing on shuttle
                            {
                                BTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }
                else                                                            //  kevin 20130615 rising after placing on shuttle -> close destroy
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(W5_32S_NEEDCHECK_GET(BTestSuck, i, j))
                            {
                                BTestSuck.Suck[i][j].OffDestroy();              //kevin 20130719 close blow-off
                            }
                        }
                    }
                }
            }

            iPos=Prod.TestZ1_Safe;
            sp=GetIndexZSpeed(0);                                               //Steven 20160524 : Index Z軸速度整合為Function
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(iPos, sp, "Do32SiteTestDestroyIC 500"))
            {
                flag=false;
                if(IniConfig.bD44CheckIndexICDestroy)
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(W5_32S_NEEDCHECK_GET(FTestSuck, i, j))            //Suck On Destry Off
                            {
                                bIndexPlaceIcCheck=true;                        //kevin 20110505
                                flag=true;

                                if(INDEX_SUCKER_TYPE==1)                        //Steven 20111202
                                {
                                    fiosetview->bIndexSuck[0][i][j]=true;
                                }
                                else
                                {
                                    FTestSuck.Suck[i][j].On();                  //kevin 20110504 check drop
                                }
                            }

                            if(W5_32S_NEEDCHECK_GET(BTestSuck, i, j))
                            {
                                bIndexPlaceIcCheck=true;                        //kevin 20110505
                                flag=true;
                                if(INDEX_SUCKER_TYPE==1)                        //Steven 20111202
                                {
                                    fiosetview->bIndexSuck[1][i][j]=true;
                                }
                                else
                                {
                                    BTestSuck.Suck[i][j].On();
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    Do32TestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                    Task=600;
                    break;
                }

                W5_32S_SETHASNULLIC(FRCarryKit);                                // golden FRCarryKit.SetHasNullIcToNullIc() -- see W5_32S_SEAM   //jou 2011-06-29 out shuttle has_null_ic hang up fix    //Steven 20160524 : ADD SetHasNullIcToNullIc
                W5_32S_SETHASNULLIC(BRCarryKit);                                // golden BRCarryKit.SetHasNullIcToNullIc() -- see W5_32S_SEAM
                MOT[MInShuttle1].fCanMoveM=true;
                MOT[MInShuttle2].fCanMoveM=true;
                iIndexTakeDeviceChk1=0;                                         //kevin 20190103 re-suck check state
                iIndexTakeDeviceChk2=0;                                         //kevin 20190103 re-suck check state
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        bFTDuplicateErr[i][j]=false;
                        bBTDuplicateErr[i][j]=false;
                    }
                }
                return true;
            }
            break;
        case 600:
            if(INDEX_SUCKER_TYPE==1)                                                                                    //jou 2012-01-04 need to confirm the Index suck fully finished
            {
                bIndexSuckCheck1=W5_32S_FIOSET_PISD1(1);                                                                // golden fiosetview->ProcessIndexSuckDestroy1(1) -- see W5_32S_SEAM  //Sam 20220920 : D44 Index re-stick check picks up at most 4 per pass
                bIndexSuckCheck2=W5_32S_FIOSET_PISD2(1);                                                                // golden fiosetview->ProcessIndexSuckDestroy2(1) -- see W5_32S_SEAM
            }
            else
            {
                bIndexSuckCheck1=true;
                bIndexSuckCheck2=true;
            }

            if(Do32TestDestroyICDelay.Off() &&
               bIndexSuckCheck1==true &&
               bIndexSuckCheck2==true)
            {
                flag=false;
                ErrPart=" ";
                bHasErr=false;
                bHasDuplicateErr=false;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(bFTDuplicateErr[i][j])
                            bHasDuplicateErr=true;
                    }
                }

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(W5_32S_NEEDCHECK_GET(FTestSuck, i, j))                                                       //confirm vacuum state
                        {
                            if(FTestSuck.Suck[i][j].GetStatus())
                            {
                                bHasErr=true;
                                ErrPart+=IndexSuckName[i+iNN][j];                                                       //kevin 20180514 arm 1 +2 needs to map into the 32-site alarm position correctly
                                flag=true;
                                bFTDuplicateErr[i][j]=true;                                                             //Steven 20110127
                                iIndexTakeDeviceChk1=3;                                                                 //kevin 20190103 re-suck check state
                            }
                            else
                            {
                                W5_32S_NEEDCHECK_SET(FTestSuck, i, j, false);                                           //confirmed
                                FTestSuck.Suck[i][j].Normal();
                                bFTDuplicateErr[i][j]=false;                                                            //Steven 20110127
                            }
                        }

                        if(INDEX_SUCKER_TYPE==1)                                                                        //jou 2011-11-01 negative pressure cannot be left on, must close it
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                bIndexPlaceIcCheck=false;                                                                               //kevin 20110505
                if(bHasErr)
                {
                    if(IndexAlarmInArmAway()==true)                                                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
                    {
                        ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);                       //Vacuum Sensor Off Error jou 2010-11-23
                    }
                    else
                    {
                        return false;
                    }
                }
                bHasErr=false;

                flag=false;
                ErrPart=" ";
                bHasErr=false;

                bHasDuplicateErr=false;
                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(bBTDuplicateErr[i][j])
                            bHasDuplicateErr=true;
                    }
                }

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(W5_32S_NEEDCHECK_GET(BTestSuck, i, j))
                        {
                            if(BTestSuck.Suck[i][j].GetStatus())
                            {
                                bHasErr=true;
                                ErrPart+=IndexSuckName[i][j];
                                flag=true;
                                bBTDuplicateErr[i][j]=true;                                                             //Steven 20110127
                                iIndexTakeDeviceChk2=3;                                                                 //kevin 20190103 re-suck check state
                            }
                            else
                            {
                                W5_32S_NEEDCHECK_SET(BTestSuck, i, j, false);
                                BTestSuck.Suck[i][j].Normal();
                                bBTDuplicateErr[i][j]=false;                                                            //Steven 20110127
                            }
                        }

                        if(INDEX_SUCKER_TYPE==1)                                                                        //jou 2011-11-01 negative pressure cannot be left on, must close it
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                }
                bIndexPlaceIcCheck=false;                                                                               //kevin 20110505
                if(bHasErr)
                {
                    if(IndexAlarmInArmAway()==true)                                                                     //Steven 20130613 : Index異常時, In Arm要先讓位功能
                    {
                        ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);                       //Vacuum Sensor Off Error jou 2010-11-23
                    }
                    else
                    {
                        return false;
                    }
                }

                if(flag==false)
                {
                    Task=500;
                }
                else
                {
                    if(IniConfig.bD44CheckIndexICDestroy)                                                               //jou 2011-11-01 negative pressure cannot be left on, must close it
                    {
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(W5_32S_NEEDCHECK_GET(FTestSuck, i, j))
                                {
                                    bIndexPlaceIcCheck=true;                                                            //kevin 20110505
                                    if(INDEX_SUCKER_TYPE==1)                                                            //Steven 20111202
                                    {
                                        fiosetview->bIndexSuck[0][i][j]=true;
                                    }
                                    else
                                    {
                                        FTestSuck.Suck[i][j].On();                                                      //kevin 20110504 check drop
                                    }
                                }

                                if(W5_32S_NEEDCHECK_GET(BTestSuck, i, j))
                                {
                                    bIndexPlaceIcCheck=true;                                                            //kevin 20110505
                                    if(INDEX_SUCKER_TYPE==1)                                                            //Steven 20111202
                                    {
                                        fiosetview->bIndexSuck[1][i][j]=true;
                                    }
                                    else
                                    {
                                        BTestSuck.Suck[i][j].On();
                                    }
                                }
                            }
                        }
                    }
                    Do32TestDestroyICDelay.SetMSAndOn(IniConfig.iD44TestHeadCheckVacuumTime);
                }
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
extern int CheckOneCycleAction(int iTask);                                      //Steven 20240326 : decide whether one-cycle should keep placing IC (golden ainarm9045.h:73, real body ainarm9045.cpp:2316)
TQPF_Timer DoTestHeadMotorDelay2_32site;
TQPF_Timer hDo32TestSuckIC;

//==============================================================================
//  Do32SiteTestSuckIC (golden :3689-4749) -- 32-site dual-arm "suck" (pick up
//  IC from the in-shuttle carry-kits into the test sockets) hand-off.  Shares
//  its Task cursor with atester_shims.cpp's offline DoFrontTestSuckIC stub
//  (iFrontTestSuckICTask) -- SAME golden-quirk shared-cursor pattern as
//  Do32SiteTestDestroyIC above (mutually exclusive at runtime).  ACTIVE,
//  faithful.  Substrate gates mirror the SAME family already established by
//  aTester_Front.cpp / this file's own Do32SiteTestDestroyIC: MoveSuckData,
//  AddIndexPickVacuum (NEW gap, same shape as AddIndexPlaceShuttleRecord),
//  LastSet.iIndexInputOutPut[], fAutoTeach->SetIndexBinError (NEW gap).
//==============================================================================
bool Do32SiteTestSuckIC()                                                       //Steven 20150910 : 32Site dual-arm suck/destroy together
{
    static bool bFTDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false}};

    static bool bBTDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false}};

    static bool bFTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]  ={{false, false, false, false, false, false, false, false},                                      //Steven 20110301 : confirm suck finished
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false}};

    static bool bBTSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]  ={{false, false, false, false, false, false, false, false},                                      //Steven 20110301 : confirm suck finished
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false},
                                                                 {false, false, false, false, false, false, false, false}};

    int &Task=iFrontTestSuckICTask, ret;                                        // golden shared cursor with (offline-stub) DoFrontTestSuckIC -- see banner
    int sp;
    int iNN=IsNNMode();
    int ZPos1, ZPos2;
    bool flag1=false, flag2=false;
    bool bHasDuplicateErr=false;
    bool bHasErr=false;
    bool bResult;
    AnsiString ErrPart="";

    switch(Task)
    {
        case 1:                                                                 //decide the IC state on Index vs Shuttle
            if(iOneCycle)
            {
                bResult=false;
                ret=CheckOneCycleAction(0);                                     //Steven 20240326 : decide whether one-cycle should keep placing IC
                if(ret==0)
                {
                    if(FTestSuck.UseSiteNoIC()  &&
                       FRCarryKit.UseSiteNoIC() &&
                       FLCarryKit.UseSiteNoIC() &&
                       !InArmSuck.HasRealIC())
                    {
                        MOT[MInShuttle1].fCanMoveM=true;
                        bResult=true;
                    }

                    if(BTestSuck.UseSiteNoIC()  &&
                       BRCarryKit.UseSiteNoIC() &&
                       BLCarryKit.UseSiteNoIC() &&
                       !InArmSuck.HasRealIC())
                    {
                        MOT[MInShuttle2].fCanMoveM=true;
                        bResult=true;
                    }
                }

                if(bResult)
                {
                    return bResult;
                }
            }

            if(FLCarryKit.UseSiteNoIC() ||
               BLCarryKit.UseSiteNoIC())
            {
                if(iCleanOut &&
                   InArmSuck.NoIC() &&
                   MOT[MMPlate1].HasIC()==false &&
                   MOT[MMPlate2].HasIC()==false)
                {
                    MOT[MInShuttle1].fCanMoveM=true;
                    MOT[MInShuttle2].fCanMoveM=true;
                    return true;
                }
            }

            if(iCleanOut &&
               FLCarryKit.UseSiteNoIC() &&
               InArmSuck.NoIC()  &&
               MOT[MMPlate1].HasIC()==false &&
               MOT[MMPlate2].HasIC()==false)
            {
                return true;
            }

            if(iCleanOut &&
               BLCarryKit.UseSiteNoIC() &&
               InArmSuck.NoIC()  &&
               MOT[MMPlate1].HasIC()==false &&
               MOT[MMPlate2].HasIC()==false)
            {
                return true;
            }

            if(FLCarryKit.UseSiteHasIC() &&
               BLCarryKit.UseSiteHasIC())
            {
                if(InSHT1InRT()==false || InSHT2InRT()==false)
                {
                    return false;
                }

                if(bReadAndCheckCPUName==true &&
                   CosFunction.bInShuttleDetectByLatch &&                       //Isaac 20170418 (Steven) detect In-Shuttle IC via Y-Latch
                   Prod.bF18InshuttleDetect &&                                  //jou 20170418 (Steven) : per SiliconWare-Shiming request, moved to Recipe F18
                   MOTION_CARD_TYPE==MotionCard_Contec &&
                   LastSet.iRealDummy==REALLY)
                {
                    if((bInSht1LtcDetectTesterCanMove==false) || (bInSht2LtcDetectTesterCanMove==false))
                    {
                        return false;
                    }
                }

                if(In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                   bInSh1DoLtc==true && bInSh2DoLtc==true)                      //KenHsieh 20250722 : InSht sensor changed to two, distinguish stacked vs flying IC via Latch
                    return false;

                MOT[MInShuttle1].fCanMoveM=false;
                MOT[MInShuttle2].fCanMoveM=false;
                Task=300;
            }
            break;
        case 299:                                                               //Steven 20140827 : first-time IC suck wait time
            if(DoTestHeadMotorDelay2_32site.Off())
            {
                Task=301;
            }
            break;
        case 300:
            MOT[MInShuttle1].fCanMoveM=false;
            MOT[MInShuttle2].fCanMoveM=false;

            if(DeviceForm.bSuckShuttleDeviceAfterTested &&                      //Steven 20140827 : first-time IC suck wait time
               (LastSet.iTemperature==Tempture_Hot ||                          //2013-11-27   Dell    needs to do Index soak time
                LastSet.iTemperature==Tempture_AmbientHot) &&
               bInitialSackTime &&                                              //JerryYang 20181214 : non-hotplate mode also needs temperature-priority support
               Temperature.iInitialStart1Time>0)                                //kevin 20131112 first-time IC suck wait time)
            {
                DoTestHeadMotorDelay2_32site.SetSecAndOn(Temperature.iInitialStart1Time);
                iInitialSoakTimer = Temperature.iInitialStart1Time;
                W5_32S_FMAIN_TINITSOAKTIMER.LatchCycleTimeSec(true);            // golden fMain->tInitSoakTimer.LatchCycleTimeSec(true) -- see W5_32S_SEAM
                bInitialSackTime=false;
                Task=299;
                break;
            }

            flag2=false;
            Task=301;
        case 301:                                                               //confirm Z-axis position: down -> suck IC -> Z-axis in position -> Task=310
            if(IniConfig.bIndexPickupWait==true &&                              //jou 2012-06-29 Index Pick up need wait Soak Time
               (LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot) &&                   //kevin 20180903 (Steven) : add constant-temperature control
               Temperature.iInitialStart2Time!=0)
            {
                if(FLCarryKit.TSoakTime.Off()==false)
                    break;
            }

            if(IniConfig.bA05UseAutoDocking==true &&                            //jou 2014-04-25 Tester Docking Wait delay time
               Temperature.bUseTesterDocking==true &&                           //ChungHung 20141111 modify for SCK request
               LastSet.iTemperature==Tempture_Hot &&
               iTesterDucking>0)
            {
                bHangTimePause=true;                                            //ChungHung 20140714 add fix Tester Ducking function if wait long time will Auto Home then Show Hangup Message
                break;
            }

            if(bNeedTesterDuckingFinishLightYellowAndAlarmOn)                   //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
            {
                bNeedTesterDuckingFinishLightYellowAndAlarmOn=false;
                bTesterDuckingFinishLightYellowAndAlarmOn=true;
            }

            MOT[MInShuttle1].ScanMotorStatus();
            MOT[MInShuttle2].ScanMotorStatus();

            if(BLCarryKit.UseSiteHasIC() &&
               FLCarryKit.UseSiteHasIC())
            {
                flag2=false;
                if(BLCarryKit.HasRealIC()==false &&
                   FLCarryKit.HasRealIC()==false)
                {
                    flag2=true;
                }
                else
                {
                    ZPos1=MOT[MTestZ1].Gali_ReadPos();
                    ZPos2=MOT[MTestZ2].Gali_ReadPos();

                    if(ZPos1<(Prod.TestZ1_Pick+1000))                           //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                    {
                        bZ1PickShuttle=true;
                    }

                    if(ZPos2<(Prod.TestZ2_Pick+1000))                           //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                    {
                        bZ2PickShuttle=true;
                    }

                    if(flag2==false)
                    {
                        sp=GetIndexZSpeed(0);                                   //Steven 20160524 : Index Z軸速度整合為Function
                        flag2=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Pick, sp, "Do32SiteTestDestroyIC 301", true, Prod.TestZ2_Pick);
                    }
                }
            }
            else if((FLCarryKit.HasIC()==false && FTestSuck.HasIC()) && (BLCarryKit.HasRealIC()==true))
            {
                Task=3010;
                break;
            }
            else if((BLCarryKit.HasIC()==false && BTestSuck.HasIC()) && (FLCarryKit.HasRealIC()==true))
            {
                Task=3020;
                break;
            }
            else
            {
                Task=1;
                break;
            }

            if(LastSet.iRealDummy==REALLY &&
               ArmSpeed[IndexArm].bSuckOnDown &&
               INDEX_SUCKER_TYPE==0)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j]!=NULL_IC &&
                           FLCarryKit.Item[i][j]!=HAS_NULL_IC)
                            FTestSuck.Suck[i][j].On();

                        if(BLCarryKit.Item[i][j]!=NULL_IC &&
                           BLCarryKit.Item[i][j]!=HAS_NULL_IC)
                            BTestSuck.Suck[i][j].On();
                    }
                }
            }

            if(flag2)
            {
                bZ1PickShuttle=true;                                            //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        bFTSuckFinish[i][j]=false;                              //Steven 20110301 : reset, assume none done yet
                        bBTSuckFinish[i][j]=false;                              //Steven 20110301 : reset, assume none done yet
                        FTestSuck.Suck[i][j].Reset();                           //Steven 20140213 : Jordan said the Index goes down without sucking -> Alarm directly
                        BTestSuck.Suck[i][j].Reset();                           //Steven 20140213 : Jordan said the Index goes down without sucking -> Alarm directly
                    }
                }
                flag2=false;
                Task=310;
            }
            break;
        case 310:
            flag1=true;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(FLCarryKit.Item[i][j])
                    {
                        if(FLCarryKit.Item[i][j]==HAS_NULL_IC ||
                           bFTSuckFinish[i][j]==true)                                                                   //Steven 20110301
                        {
                            if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                               FTestSuck.Item[i][j]==NULL_IC)                                                           //Steven 20111202 : Retry could drop IC
                                FTestSuck.Suck[i][j].Normal();                                                          //Steven 20111201 : guard against negative-pressure droop
                            bFTSuckFinish[i][j]=true;                                                                   //Steven 20110301
                            bFTDuplicateErr[i][j]=false;                                                                //Steven 20100105
                        }
                        else
                        {
                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                W5_32S_MOVESUCKDATA(FTestSuck, FLCarryKit, i, j);                                       // golden FTestSuck.MoveSuckData(FLCarryKit,i,j) -- see W5_32S_SEAM
                                W5_32S_ADD_INDEX_PICK_VACUUM(FTestSuck, i, j, 0);                                       // golden FTestSuck.PordRec[i][j].AddIndexPickVacuum(FTestSuck.Suck[i][j].GetIOValue()) -- see W5_32S_SEAM  //Sam 20230210 : new VacuumUnit comm module
                                bFTDuplicateErr[i][j]=false;                                                            //Steven 20100105
                                bFTSuckFinish[i][j]=true;                                                               //Steven 20110301 : done ones are skipped
                                if(bRunAutoClean==false)                                                                //kevin 20130125
                                    W5_32S_LASTSET_INDEXINOUT_INC(0);                                                   // golden LastSet.iIndexInputOutPut[0]++ -- see W5_32S_SEAM  //kevin 20130125 Index suck IC counter
                            }
                            else if(FTestSuck.Suck[i][j].Error)                                                         //Steven 20110301 : errored ones are skipped
                            {
                                bFTSuckFinish[i][j]=true;
                            }
                            else
                            {
                                flag1=false;                                                                            //jou 2011-08-16 keep waiting while anything is unfinished
                            }
                        }
                    }
                    else
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_IC ||
                           FTestSuck.Item[i][j]==NULL_IC)                                                               //Steven 20111202 : Retry could drop IC
                            FTestSuck.Suck[i][j].Normal();                                                              //Steven 20111201 : guard against negative-pressure droop
                        bFTSuckFinish[i][j]=true;                                                                       //Steven 20110301 : skip empty positions
                    }

                    if(BLCarryKit.Item[i][j])
                    {
                        #ifdef SOFT_SIMULTE
                        if(fMain->cbIndexDrop->Checked==true)
                        {
                            bBTSuckFinish[0][1]=true;
                            BTestSuck.Suck[0][1].Error=true;
                        }
                        #endif

                        if(BLCarryKit.Item[i][j]==HAS_NULL_IC ||
                           bBTSuckFinish[i][j]==true)                                                                   //Steven 20110301
                        {
                            if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                               BTestSuck.Item[i][j]==NULL_IC)                                                           //Steven 20111202 : Retry could drop IC
                                BTestSuck.Suck[i][j].Normal();                                                          //Steven 20111201 : guard against negative-pressure droop
                            bBTSuckFinish[i][j]=true;                                                                   //Steven 20110301
                        }
                        else
                        {
                            if(BTestSuck.Suck[i][j].Error)                                                              //Steven 20110301 : errored ones are skipped
                            {
                                bBTSuckFinish[i][j]=true;
                            }
                            else if(BTestSuck.Suck[i][j].Suck())
                            {
                                W5_32S_MOVESUCKDATA(BTestSuck, BLCarryKit, i, j);                                       // golden BTestSuck.MoveSuckData(BLCarryKit,i,j) -- see W5_32S_SEAM
                                W5_32S_ADD_INDEX_PICK_VACUUM(BTestSuck, i, j, 0);                                       // golden BTestSuck.PordRec[i][j].AddIndexPickVacuum(BTestSuck.Suck[i][j].GetIOValue()) -- see W5_32S_SEAM  //Sam 20230210 : new VacuumUnit comm module
                                bBTDuplicateErr[i][j]=false;                                                            //Steven 20100105
                                bBTSuckFinish[i][j]=true;                                                               //Steven 20110301 : done ones are skipped
                                if(bRunAutoClean==false)                                                                //kevin 20120531
                                    W5_32S_LASTSET_INDEXINOUT_INC(0);                                                   // golden LastSet.iIndexInputOutPut[0]++ -- see W5_32S_SEAM  //kevin 20120511 Index suck IC counter
                            }
                            else
                            {
                                flag1=false;                                                                            //jou 2011-08-16 keep waiting while anything is unfinished
                            }
                        }
                    }
                    else
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_IC ||
                           BTestSuck.Item[i][j]==NULL_IC)                                                               //Steven 20111202 : Retry could drop IC
                            BTestSuck.Suck[i][j].Normal();                                                              //Steven 20111201 : guard against negative-pressure droop
                        bBTSuckFinish[i][j]=true;                                                                       //Steven 20110301 : skip empty positions
                    }
                }
            }

            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTSuckFinish[i][j]==false)                                                                      //keep waiting while anything is unfinished
                        flag1=false;

                    if(bBTSuckFinish[i][j]==false)                                                                      //keep waiting while anything is unfinished
                        flag1=false;
                }
            }

            if(flag1==true)                                                                                             //Steven 20110301 : every nozzle finished
            {
                bHasErr=false;
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            bShuttle1HasPickErr=true;                                                                   //Steven 20230118 : avoid In Arm sneaking a place, move up
                            if(IniConfig.bD62PickUpErrorNeedPurge)                                                      //Steven 20161024 : pick-error needs one blow-off
                            {
                                FTestSuck.Suck[i][j].Off();
                            }
                        }

                        if(BTestSuck.Suck[i][j].Error)
                        {
                            bHasErr=true;
                            bShuttle2HasPickErr=true;                                                                   //Steven 20230118 : avoid In Arm sneaking a place, move up
                            if(IniConfig.bD62PickUpErrorNeedPurge)                                                      //Steven 20161024 : pick-error needs one blow-off
                            {
                                BTestSuck.Suck[i][j].Off();
                            }
                        }
                    }
                }

                if(bHasErr)
                {
                    Task=305;
                    return false;
                }

                if(FLCarryKit.HasRealIC() || BLCarryKit.HasRealIC())
                    break;

                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j])
                        {
                            if(FLCarryKit.Item[i][j]==HAS_NULL_IC && FTestSuck.Item[i][j]!=NULL_IC)
                                FLCarryKit.SetItemData(i, j, NULL_IC);
                            if(FLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                FTestSuck.Suck[i][j].Normal();
                                W5_32S_MOVESUCKDATA(FTestSuck, FLCarryKit, i, j);                                       // golden FTestSuck.MoveSuckData(FLCarryKit,i,j) -- see W5_32S_SEAM
                            }
                        }
                        bFTDuplicateErr[i][j]=false;                                                                    //Steven 20100105

                        if(BLCarryKit.Item[i][j])
                        {
                            if(BLCarryKit.Item[i][j]==HAS_NULL_IC && BTestSuck.Item[i][j]!=NULL_IC)
                                BLCarryKit.SetItemData(i, j, NULL_IC);
                            if(BLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();
                                W5_32S_MOVESUCKDATA(BTestSuck, BLCarryKit, i, j);                                       // golden BTestSuck.MoveSuckData(BLCarryKit,i,j) -- see W5_32S_SEAM
                            }
                        }
                        bBTDuplicateErr[i][j]=false;                                                                    //Steven 20100105
                    }
                }

                if(FLCarryKit.UseSiteHasIC() ||
                   BLCarryKit.UseSiteHasIC())
                    break;

                bShuttle1HasPickErr=false;                                                                              //Steven 20220712 : avoid In Arm sneaking a place
                bShuttle2HasPickErr=false;                                                                              //Steven 20220712 : avoid In Arm sneaking a place
                bResetIndexArm1Pick=false;
                Task=500;
            }
            break;
        case 305:
            CheckIndexAllSuckICFallDown(true, true);                            //Steven 20110725 : revise negative-pressure check

            if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Pick+1000))             //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
            {
                bZ1PickShuttle=false;
            }

            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Pick+1000))             //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
            {
                bZ2PickShuttle=false;
            }

            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, MOT[MTestZ1].GailSpeed, "Do32SiteTestDestroyIC 305"))
            {
                bZ1PickShuttle=false;                                           //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                bZ2PickShuttle=false;                                           //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                Task=320;
            }
            break;
        case 3010:
            return DoRearTestSuckIC();
        case 3020:
            return DoFrontTestSuckIC();
        case 320:
            bShuttle1MoveToLeft=false;
            bShuttle2MoveToLeft=false;
            MOT[MInShuttle1].fCanMoveM=false;
            MOT[MInShuttle2].fCanMoveM=false;
            Task=321;
        case 321:                                                                                                       //ChungHung 20130924 add
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            flag1=false;
            for(int i=0; i<FTestSuck.iShtRow; i++)
            {
                for(int j=0; j<FTestSuck.iShtCol; j++)
                {
                    if(bFTDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iNN][j];                                                               //kevin 20180514 arm 1 +2 needs to map into the 32-site alarm position correctly
                        if(IniConfig.bD62PickUpErrorNeedPurge)                                                          //Steveb 20161024 : pick-error needs one blow-off
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(bHasErr)
            {
                if(IndexAlarmInArmAway()==false)                                                                        //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }

                bHasErr=false;

                if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                {
                    ret=K_SKIP;
                }
                else
                {
                    if(CosFunction.bJAM0301NeedOpenChamberDoor)                                                         //wei : JAM0301 & JAM0302 need to open the Chamber door for 10 seconds
                    {
                        bIsTestSitICFallDown=true;
                    }

                    if(IniConfig.bIndexPickErrOnlySKIP==true ||                                                         //jou 2012-02-13 index pick-up error only skip
                       IniConfig.bD64IndexPickErrOnlySKIP)                                                              //kevin 20171103 (wei) add retry function
                        ret=ShowErrorMessage("JAM0301", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart);                    //Devicr Pick-Up Error
                    else
                        ret=ShowErrorMessage("JAM0301", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);            //Devicr Pick-Up Error
                }

                if(ret==K_SKIP)
                {
                    if(LastSet.iRunStartMode==rsmAutoSiteMap)
                        iAutoSiteMapCount++;

                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Item[i][j]==NULL_IC)
                            {
                                FLCarryKit.PordRec[i][j].AddErrorRecord("JAM0301");                                     //Steven 20161214 : add error log for an Index-error Skip
                                if(CosFunction.bUseSCKART)                                                              //Steven 20161214 (wei) : For SCK ART
                                {
                                    if(TestIF_File.bRENESAS_EnableFTCT==true)                                           //RogeryYang 20251014 : FTCT add to HdRejectBT
                                        fSCKART->iInputJamCnt++;
                                    else
                                        fSCKART->AddOutputJamCnt(i, j, ret);                                            //RogerYang 20250923 : integrate ART OutArm JamCount
                                }
                                FTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                FTestSuck.Suck[i][j].Normal();
                            }
                            FLCarryKit.SetItemData(i, j, NULL_IC);
                            bFTDuplicateErr[i][j]=false;
                        }
                    }

                    if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                    {
                    }
                    else
                    {
                        if(IniConfig.bD42IndexPickICShuttlePause)
                        {
                            bInArmNeedToSafePos=true;
                            bShuttle1Pause=true;
                            bIndexArm1PickupErrStop=true;                                                               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                            bShowShuttle1Device=true;                                                                   //kevin 20180504 index pick up error
                        }
                    }
                    bIndexPickUpErrorWaitRetry=false;                                                                   //Ifor 20171119 (Steven) : add protection against In Arm sneaking ahead during Index Pick Up Err causing data corruption -> Hangup
                }
                else
                {
                    for(int i=0; i<FTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iShtCol; j++)
                        {
                            if(FTestSuck.Suck[i][j].Error)
                                bFTDuplicateErr[i][j]=true;
                        }
                    }
                    flag1=true;                                                                                         //Steven 20220712 : avoid the flag being cleared when Arm 1 Jams but Arm 2 does not
                    bIndexPickUpErrorWaitRetry=true;                                                                    //Ifor 20171119 (Steven) : add protection against In Arm sneaking ahead during Index Pick Up Err causing data corruption -> Hangup
                }

                FTestSuck.ResetAll();                                                                                   //Steven 20160323 : guard against vacuum not opened

                if(IniConfig.bNewResetFunction==true && bResetIndexArm1Pick==true)
                {
                    bResetIndexArm1Pick=false;
                }
                else
                {
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                     //ChungHung 20120717 add Index Drop Error Can Retry and Start
                    {
                        MOT[MInShuttle1].fCanMoveM=true;                                                                //wait for Shuttle to move right
                        bShuttle1MoveToRight=true;
                        bShuttle1HasPickErr=true;                                                                       //Steven 20220712 : avoid In Arm sneaking a place
                    }
                }
            }

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<BTestSuck.iShtRow; i++)
            {
                for(int j=0; j<BTestSuck.iShtCol; j++)
                {
                    if(bBTDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];

                        if(IniConfig.bD62PickUpErrorNeedPurge)                                                          //Steveb 20161024 : pick-error needs one blow-off
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(bHasErr)
            {
                if(IndexAlarmInArmAway()==false)                                                                        //Steven 20130613 : Index異常時, In Arm要先讓位功能
                {
                    return false;
                }

                bHasErr=false;

                if(IniConfig.bNewResetFunction==true &&
                   bResetIndexArm2Pick==true)
                {
                    ret=K_SKIP;
                }
                else
                {
                    if(CosFunction.bJAM0301NeedOpenChamberDoor)                                                         //wei : JAM0301 & JAM0302 need to open the Chamber door for 10 seconds
                    {
                        bIsTestSitICFallDown=true;
                    }

                    if(IniConfig.bIndexPickErrOnlySKIP==true ||                                                         //jou 2012-02-13 index pick-up error only skip
                       IniConfig.bD64IndexPickErrOnlySKIP)                                                              //kevin 20171103 (wei) add retry function
                        ret=ShowErrorMessage("JAM0302", K_SKIP, MTestZ2, bHasDuplicateErr, ErrPart);                    //Devicr Pick-Up Error
                    else
                        ret=ShowErrorMessage("JAM0302", K_SKIP|K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);            //Devicr Pick-Up Error
                }

                if(ret==K_SKIP)
                {
                    if(LastSet.iRunStartMode==rsmAutoSiteMap)
                        iAutoSiteMapCount++;                                                                            //Steven 20220811 : record which Shuttle this is

                    for(int i=0; i<BTestSuck.iShtRow; i++)
                    {
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                        {
                            if(BTestSuck.Item[i][j]==NULL_IC)
                            {
                                BLCarryKit.PordRec[i][j].AddErrorRecord("JAM0301");                                     //Steven 20161214 : add error log for an Index-error Skip
                                if(CosFunction.bUseSCKART)                                                              //Steven 20161214 (wei) : For SCK ART
                                {
                                    if(TestIF_File.bRENESAS_EnableFTCT==true)                                           //RogeryYang 20251014 : FTCT add to HdRejectBT
                                        fSCKART->iInputJamCnt++;
                                    else
                                        fSCKART->AddOutputJamCnt(i, j, ret);                                            //RogerYang 20250923 : integrate ART OutArm JamCount
                                }
                                BTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                BTestSuck.Suck[i][j].Normal();
                            }
                            BLCarryKit.SetItemData(i, j, NULL_IC);
                            bBTDuplicateErr[i][j]=false;
                        }
                    }

                    if(IniConfig.bNewResetFunction==true && bResetIndexArm2Pick==true)
                    {
                    }
                    else
                    {
                        if(IniConfig.bD42IndexPickICShuttlePause)
                        {
                            bInArmNeedToSafePos=true;
                            bShuttle2Pause=true;
                            bIndexArm2PickupErrStop=true;                                                               //jou 2012-02-29 index pick up error,index arm move to center & alarm
                            bShowShuttle2Device=true;                                                                   //kevin 20180504 index pick up error
                        }
                    }

                    if(flag1==false)                                                                                    //Steven 20220712 : avoid the flag being cleared when Arm 1 Jams but Arm 2 does not
                        bIndexPickUpErrorWaitRetry=false;                                                               //Ifor 20171119 (Steven) : add protection against In Arm sneaking ahead during Index Pick Up Err causing data corruption -> Hangup
                }
                else
                {
                    for(int i=0; i<BTestSuck.iShtRow; i++)
                        for(int j=0; j<BTestSuck.iShtCol; j++)
                            if(BTestSuck.Suck[i][j].Error)
                                bBTDuplicateErr[i][j]=true;
                    bIndexPickUpErrorWaitRetry=true;                                                                    //Ifor 20171119 (Steven) : add protection against In Arm sneaking ahead during Index Pick Up Err causing data corruption -> Hangup
                }

                BTestSuck.ResetAll();                                                                                   //Steven 20160323 : guard against vacuum not opened

                if(IniConfig.bNewResetFunction==true &&
                   bResetIndexArm2Pick==true)
                {
                    bResetIndexArm2Pick=false;
                }
                else
                {
                    if(IniConfig.bD43IndexDropErrorCanRetryandSkip)                                                     //ChungHung 20120717 add Index Drop Error Can Retry and Start
                    {
                        MOT[MInShuttle2].fCanMoveM=true;                                                                //wait for Shuttle to move right
                        bShuttle2MoveToRight=true;
                        bShuttle2HasPickErr=true;                                                                       //Steven 20220712 : avoid In Arm sneaking a place
                    }
                }
            }

            if(FLCarryKit.HasRealIC() || BLCarryKit.HasRealIC())
            {
                Task=300;
            }
            else
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FLCarryKit.Item[i][j])
                        {
                            if(FLCarryKit.Item[i][j]==HAS_NULL_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC)
                                FLCarryKit.SetItemData(i, j, NULL_IC);

                            if(FLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                FTestSuck.Suck[i][j].Normal();
                                W5_32S_MOVESUCKDATA(FTestSuck, FLCarryKit, i, j);                                       // golden FTestSuck.MoveSuckData(FLCarryKit,i,j) -- see W5_32S_SEAM
                                bFTDuplicateErr[i][j]=false;
                            }
                        }

                        if(BLCarryKit.Item[i][j])
                        {
                            if(BLCarryKit.Item[i][j]==HAS_NULL_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC)
                                BLCarryKit.SetItemData(i, j, NULL_IC);

                            if(BLCarryKit.Item[i][j]==HAS_NULL_IC)
                            {
                                BTestSuck.Suck[i][j].Normal();
                                W5_32S_MOVESUCKDATA(BTestSuck, BLCarryKit, i, j);                                       // golden BTestSuck.MoveSuckData(BLCarryKit,i,j) -- see W5_32S_SEAM
                                bBTDuplicateErr[i][j]=false;
                            }
                        }
                    }
                }

                if(bShuttle1Pause)
                    MOT[MInShuttle1].SetSpeed(10);                                                                      //kevin 20180226 (Steven) add pick up error  shuttle down speed
                if(bShuttle2Pause)
                    MOT[MInShuttle2].SetSpeed(10);                                                                      //kevin 20180226 (Steven) add pick up error  shuttle down speed

                if(FLCarryKit.UseSiteHasIC() ||
                   BLCarryKit.UseSiteHasIC())
                {
                    Task=300;
                    break;
                }
                Task=500;
            }
            break;
        case 500:
            MOT[MInShuttle2].fCanMoveM=false;
            MOT[MInShuttle1].fCanMoveM=false;
            bShuttle1MoveToRight=false;
            bShuttle1HasPickErr=false;                                          //Steven 20220712 : avoid In Arm sneaking a place
            bShuttle2MoveToRight=false;
            bShuttle2HasPickErr=false;                                          //Steven 20230314 : avoid In Arm sneaking a place
            bIndexPickUpErrorWaitRetry=false;                                   //Ifor 20171119 (Steven) : add protection against In Arm sneaking ahead during Index Pick Up Err causing data corruption -> Hangup
            Task=501;
        case 501:                                                                                                       //ChungHung 20130924 add
            if(MOT[MTestZ1].Gali_ReadPos()>(Prod.TestZ1_Pick+1000))                                                     //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
            {
                bZ1PickShuttle=false;
            }

            if(MOT[MTestZ2].Gali_ReadPos()>(Prod.TestZ2_Pick+1000))                                                     //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
            {
                bZ2PickShuttle=false;
            }

            sp=GetIndexZSpeed(0);                                                                                       //Steven 20160524 : Index Z軸速度整合為Function
            flag1=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, sp, "Do32SiteTestDestroyIC 501");
            if(flag1)
            {
                bZ1PickShuttle=false;                                                                                   //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing
                bZ2PickShuttle=false;                                                                                   //Steven 20150407 : fix [D45] Out Arm waits on Index Z, avoid Auto Homing

                #ifndef SOFT_SIMULTE
                {
                    if(LastSet.iRealDummy==REALLY)
                    {
                        ErrPart=" ";
                        bHasErr=false;
                        bHasDuplicateErr=false;
                        for(int i=0; i<FTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<FTestSuck.iShtCol; j++)
                            {
                                if(bFTDuplicateErr[i][j])
                                    bHasDuplicateErr=true;

                                if(FTestSuck.Item[i][j] &&
                                   FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   FTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    bHasErr=true;
                                    FTestSuck.Suck[i][j].Normal();                                                      //Steven 20110707 : guard against negative-pressure drop
                                    FTestSuck.Suck[i][j].Error=true;
                                    ErrPart+=IndexSuckName[i+iNN][j];                                                   //kevin 20180514 arm 1 +2 needs to map into the 32-site alarm position correctly
                                }
                                else
                                {
                                    FTestSuck.Suck[i][j].Error=false;
                                }
                            }
                        }

                        if(bHasErr)
                        {
                            if(IndexAlarmInArmAway()==false)                                                            //Steven 20130613 : Index異常時, In Arm要先讓位功能
                            {
                                return false;
                            }

                            if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                 //Steven : JAM0303 & JAM0403 need to open the Chamber door for 10 seconds
                                bIsTestSitICFallDown=true;                                                              //kevin 20130706

                            bHasErr=false;
                            if(IniConfig.bIndexDropOnlySKIP==true ||
                               IniConfig.bKoreaFunction)                                                                //jou 2012-02-13 index drop error only skip
                            {
                                ret=ShowErrorMessage("JAM0303", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart);            //Device Drop Error
                            }
                            else
                            {
                                ret=ShowErrorMessage("JAM0303", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);    //Device Drop Error
                            }

                            if(ret==K_SKIP)
                            {
                                for(int i=0; i<FTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<FTestSuck.iShtCol; j++)
                                    {
                                        if(FTestSuck.Suck[i][j].Error)
                                        {
                                            FTestSuck.PordRec[i][j].AddErrorRecord("JAM0303");                          //Steven 20161214 : add error log for an Index-error Skip
                                            if(CosFunction.bUseSCKART)                                                  //Steven 20161214 (wei) : For SCK ART
                                            {
                                                fSCKART->AddOutputJamCnt(i, j, ret,
                                                    Prod.bIsPassBin[FTestSuck.iBinData[i][j]]);                         //RogerYang 20250923 : integrate ART OutArm JamCount
                                            }
                                            FTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                            FTestSuck.Suck[i][j].Error=false;
                                        }
                                        bFTDuplicateErr[i][j]=false;
                                    }
                                }

                                if(IniConfig.bD42IndexPickICShuttlePause)
                                {
                                    bInArmNeedToSafePos=true;
                                    bShuttle1Pause=true;
                                    bIndexArm1PickupErrStop=true;                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                                    bShowShuttle1Device=true;                                                           //kevin 20180504 index pick up error
                                }
                            }
                            else
                            {
                                for(int i=0; i<FTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<FTestSuck.iShtCol; j++)
                                    {
                                        if(FTestSuck.Suck[i][j].Error)
                                        {
                                            bFTDuplicateErr[i][j]=true;
                                            W5_32S_MOVESUCKDATA(FLCarryKit, FTestSuck, i, j);                          // golden FLCarryKit.MoveSuckData(FTestSuck,i,j) -- see W5_32S_SEAM
                                            FTestSuck.Suck[i][j].Error=false;
                                        }
                                    }
                                }
                                Task=300;
                                return false;
                            }
                        }

                        ErrPart=" ";
                        bHasErr=false;
                        bHasDuplicateErr=false;
                        for(int i=0; i<BTestSuck.iShtRow; i++)
                        {
                            for(int j=0; j<BTestSuck.iShtCol; j++)
                            {
                                if(bBTDuplicateErr[i][j])
                                    bHasDuplicateErr=true;

                                if(BTestSuck.Item[i][j]&&
                                   BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                                   BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    bHasErr=true;
                                    BTestSuck.Suck[i][j].Normal();                                                      //Steven 20110707 : guard against negative-pressure drop
                                    BTestSuck.Suck[i][j].Error=true;
                                    ErrPart+=IndexSuckName[i][j];
                                }
                                else
                                {
                                    BTestSuck.Suck[i][j].Error=false;
                                }
                            }
                        }

                        if(bHasErr)
                        {
                            if(IndexAlarmInArmAway()==false)                                                            //Steven 20130613 : Index異常時, In Arm要先讓位功能
                            {
                                return false;
                            }

                            bHasErr=false;
                            if(CosFunction.bJAM0303NeedOpenChamberDoor)                                                 //Steven : JAM0303 & JAM0403 need to open the Chamber door for 10 seconds
                                bIsTestSitICFallDown=true;                                                              //kevin 20130706
                            if(IniConfig.bIndexDropOnlySKIP==true ||
                               IniConfig.bKoreaFunction)                                                                //jou 2012-02-13 index drop error only skip
                            {
                                ret=ShowErrorMessage("JAM0304", K_SKIP, MTestZ2, bHasDuplicateErr, ErrPart);            //Device Drop Error
                            }
                            else
                            {
                                ret=ShowErrorMessage("JAM0304", K_SKIP|K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);    //Device Drop Error
                            }

                            if(ret==K_SKIP)
                            {
                                for(int i=0; i<BTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<BTestSuck.iShtCol; j++)
                                    {
                                        if(BTestSuck.Suck[i][j].Error)
                                        {
                                            BTestSuck.PordRec[i][j].AddErrorRecord("JAM0304");                          //Steven 20161214 : add error log for an Index-error Skip
                                            if(CosFunction.bUseSCKART)                                                  //Steven 20161214 (wei) : For SCK ART
                                            {
                                                fSCKART->AddOutputJamCnt(i, j, ret,
                                                    Prod.bIsPassBin[BTestSuck.iBinData[i][j]]);                         //RogerYang 20250923 : integrate ART OutArm JamCount
                                            }
                                            BTestSuck.SetItemData(i, j, HAS_NULL_IC);
                                            BTestSuck.Suck[i][j].Error=false;
                                        }
                                        bBTDuplicateErr[i][j]=false;
                                    }
                                }

                                if(IniConfig.bD42IndexPickICShuttlePause)
                                {
                                    bInArmNeedToSafePos=true;
                                    bShuttle2Pause=true;
                                    bIndexArm2PickupErrStop=true;                                                       //jou 2012-02-29 index pick up error,index arm move to center & alarm
                                    bShowShuttle2Device=true;                                                           //kevin 20180504 index pick up error
                                }
                            }
                            else
                            {
                                for(int i=0; i<BTestSuck.iShtRow; i++)
                                {
                                    for(int j=0; j<BTestSuck.iShtCol; j++)
                                    {
                                        if(BTestSuck.Suck[i][j].Error)
                                        {
                                            bBTDuplicateErr[i][j]=true;
                                            W5_32S_MOVESUCKDATA(BLCarryKit, BTestSuck, i, j);                          // golden BLCarryKit.MoveSuckData(BTestSuck,i,j) -- see W5_32S_SEAM
                                            BTestSuck.Suck[i][j].Error=false;
                                        }
                                    }
                                }
                                Task=300;
                                return false;
                            }
                        }
                    }
                }
                #endif

                if(bShuttle1Pause)
                    MOT[MInShuttle1].SetSpeed(10);                                                                      //kevin 20180226 (Steven) add pick up error  shuttle down speed

                if(bShuttle2Pause)
                    MOT[MInShuttle2].SetSpeed(10);                                                                      //kevin 20180226 (Steven) add pick up error  shuttle down speed

                MOT[MInShuttle1].fCanMoveM=true;
                MOT[MInShuttle2].fCanMoveM=true;

                for(int i=0; i<FTestSuck.iMaxRow; i++)
                {
                    for(int j=0; j<FTestSuck.iMaxCol; j++)
                    {
                        if(i>=FTestSuck.iShtRow || j>=FTestSuck.iShtCol)
                        {
                            FTestSuck.SetItemData(i, j, NULL_IC);
                            BTestSuck.SetItemData(i, j, NULL_IC);
                        }
                    }
                }

                W5_32S_FAUTOTEACH_SETINDEXBINERROR(2);                                                                  // golden fAutoTeach->SetIndexBinError(2) -- no translated home yet (see W5_32S_SEAM)  //JimmyChiu 20211020 : Auto alignment mode
                if(DeviceForm.bSuckShuttleDeviceAfterTested==false &&                                                   //Steven 20140827 : first-time IC suck wait time
                   (LastSet.iTemperature==Tempture_Hot ||
                    LastSet.iTemperature==Tempture_AmbientHot) &&                                                       //JerryYang 20181214 : non-hotplate mode also needs temperature-priority support
                   bInitialSackTime &&                                                                                  //2013-11-27   Dell    needs to do Index soak time
                   (Temperature.iInitialStart1Time>0 ||
                    Temperature.iIndexSoakTime>0))                                                                      //kevin 20131112 first-time IC suck wait time)
                {
                    int iSetTmr=Temperature.iInitialStart1Time;
                    if(Temperature.iIndexSoakTime>Temperature.iInitialStart1Time)
                        iSetTmr=Temperature.iIndexSoakTime;

                    DoTestHeadMotorDelay2_32site.SetSecAndOn(iSetTmr);
                    Task=502;
                    bInitialSackTime=false;
                }
                else                                                                                                    //Steven 20140827 : first-time IC suck wait time
                {
                    return true;
                }
            }
            break;
        case 502:                                                               //kevin 20131112  wait for initial soak time
            if(DoTestHeadMotorDelay2_32site.Off())
            {
                return true;
            }
            break;
    }
    return false;
}
