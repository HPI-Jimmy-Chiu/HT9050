// =============================================================================
//  AutoClean/AutoClean.cpp  --  W906-AutoCleanFoundation wave
//
//  Faithful translation of a curated subset of golden AutoClean.cpp (9,137
//  lines, BCB6, Big5/cp950) -- see AutoClean.h's banner for the full in-scope
//  function list + line-number map, and this wave's own report for the exact
//  golden line ranges each function below was read from.
//  Translator: AI(W906-AutoCleanFoundation) 20260721
//
//  EXPLICITLY OUT OF SCOPE (next wave): DoAutoCleanKit, DoAutoCleanPickfromCleanKit,
//  DoPlaceToShuttle, DoPickFromShuttle, DoAutoCleanPlaceToCleanKit,
//  DoShuttle1AutoClean(+_Arm1PickArm2Test variant), DoShuttle2AutoClean,
//  DoIndexAutoClean(+_Arm1PickArm2Test variant) -- the 4 named core engines and
//  their variants. Also out of scope: SetAutoCleanICCount, SetAutoCleanTrayPosition,
//  SearchiAutoCleanNum, SetAutoCleanStringGrid's OWN declaration site (still
//  translated below -- see note at its definition), ReadWriteAutoCleanCount,
//  SearchCleanNum, AutoCleanWriteData, ResetAutoClean, EnableAutoclean,
//  GetMotFunc -- none of these are in the task brief's Part A/HAL-helper list,
//  and (verified by grepping every fMain-> touch site against golden line
//  numbers) none of them are called BY an in-scope function either.
//
//  cCleanKitPickPlan.h -- confirmed (grepped, zero .cpp implementation, zero
//  call sites anywhere in golden) dead/unwired scaffolding.  NOT ported.
// =============================================================================
#include "MachineDefine.h"

#include "AutoClean.h"

#include "aArmHeader.h"             // __FUNC__ shim, RecordProcess, ShowErrorMessage, ShowMyMessage, K_RETRY/K_SKIP
#include "Motor/mymotor.h"          // MOT[], TTrayMotor, TMyTray
#include "cprod.h"                  // Prod / TestIF / TestIF_File / InArmOffSet / ArmSpeed / SHSpeed / AOA_AutoClean
#include "cmydef.h"                 // global scalar universe + IC consts
#include "cpublic.h"                // CosFunction
#include "common.h"
// AI(pt-wave) 20260811 PT-W8 integrate: needed because the one-line
// DoStructUnitConvert stand-in retired from this file (was :199) was also the
// DECLARATION for the call at :8322 in this same TU.  Golden gets the same
// declaration transitively (golden AutoClean.cpp calls it at :4504 without
// including cUnitConvert.h); this tree keeps include lists explicit, so it is
// named here.
#include "cUnitConvert.h"          // DoStructUnitConvert (golden cUnitConvert.h:5)
// AI(W906-AutoCleanFoundation) 20260721: ainarm9045.h / ainarm9045_2x6_8.h MUST
// be included BEFORE aHotPlateSubstrate.h in this TU. Both headers carry a
// `#ifndef ainarm9045H` / `#ifndef ainarm9045_2x6_8H` order-dependent guard
// (aHotPlateSubstrate.h:574/595) that only suppresses ITS OWN placeholder
// redeclarations (InArmLeftSideHasIC/NoIC default args; the bare
// `enum{e2x6OneByOne=2}`) when the real header's include-guard macro is
// *already* defined at that point -- i.e. only when the real header was
// included FIRST. Getting this backwards produces a genuine
// "default argument given twice" / "redeclaration of e2x6OneByOne" compile
// error (hit + fixed this wave).
#include "ainarm9045.h"             // bUseAxExPicker / bUseAxxGPicker / GetJStep
#include "ainarm9045_2x6_8.h"       // iCloseSiteStep_2x6 / e2x6* enum
#include "ainarm9045_2x8_8.h"       // iCloseSiteStep_2x8 (e2x8* enum already in MachineType.h -- no ordering hazard)
#include "ainarm9045_All_1Pick.h"   // GetNowInShuttleRowCol_All_1Picker
#include "aHotPlateSubstrate.h"     // InArmSuck/FLCarryKit/BLCarryKit/FTestSuck/BTestSuck, uPlateInfo (PlaceToCleanList), ainarm2 cursors
#include "atester_shims.h"          // fContact (TfContactShim)
// AI(W906-AutoCleanCluster) 20260728: CheckSocketSensor(int,AnsiString,bool,bool)
// (atester.h:135) + bFTestSuckDrop (atester.h:46) -- DoIndexAutoClean /
// DoIndexAutoClean_Arm1PickArm2Test's own socket-sensor checks need both;
// neither was pulled in by this TU's pre-existing include set.
#include "atester.h"                // CheckSocketSensor / bFTestSuckDrop
#include "FormsFacade.h"            // fMain / fCleaning / fNote stand-ins
#include "canary_support.h"         // LastSet / ShowMyMessage / ShowErrorMessage / RecordProcess
// AI(W906-AutoCleanCluster) 20260722: additional cross-module surface the core
// pick/place engines + DoAutoCleanKit orchestrator reach that the foundation
// wave's include list did not need yet. Each is an EXISTING translated header
// (no new shim file created) -- see this wave's report for the per-symbol map.
#include "acatchtray_shims.h"       // NewRecordProcess
#include "acarry.h"                 // IsFLCarrKitAllHasIC/IsBLCarrKitAllHasIC, DoInOutARM_SHT_MoveSafe, b1/2ShuttleMoveToLeft/Right
#include "acarry_shims.h"           // fLtcSensor (TfLtcSensor*)
#include "ainarm9045_w7_shims.h"    // DoShakeShuttle/DoKnockShuttle/DoVibrateShuttle, IndexZCanMove[2], bShuttleKnock
#include "csystem.h"                // HasAnyICInMachine/HasICUnderMachine (read-only use; csystem.cpp itself NOT touched)
#include "csystem_shims.h"          // bShuttleShake
#include "myswitch.h"               // SW[] (TMySwitch) -- SwTesterAirCooling
#include "mysensor.h"                // Sen[] (TMySensor) -- SnRKManualTStart
// AI(W906-AutoCleanCluster) 20260722: aArmHeader.h (already included above) is
// a GUARD-ONLY shim whose entire golden include list sits behind
// `#if 0 // TODO(W6.x/W7)` (verified by reading it) -- it pulls in NOTHING.
// DoAutoCleanKit's Fix3-cylinder case 1/2 branch and its Cylinder[]-off calls
// need these two directly (same explicit-include idiom acarry.cpp already
// uses for the identical symbols).
#include "aoutarm9045.h"             // UseFix3Cylinder / InitialFix3CanFullTask
#include "mycylin.h"                 // Cylinder[] (TMyCylinder) -- C_HotplateVibration/C_TrayVibration
#include "forms/fContactCT.h"        //AI(W906-FLOW-2) 20260928: TfContactCT *fContactCT (golden cContactCT.h:51; the real object is cContactCT.cpp:99) -- DoAutoCleanKit case 2000 ACSmartClearData (golden AutoClean.cpp:5239)
// AI(W906-AutoCleanFoundation) 20260721: golden AutoClean.cpp:44-53 -- forward
// decls this TU needs from the 16 per-site-variant GetShuttleState_* leaves
// (each already linkable in ht9045_sm from its own ainarm9045_<variant>.cpp;
// see acarry_shims.h:201-210 for the identical "plain forward declaration,
// real body binds at link time" idiom used repeatedly elsewhere in this tree
// -- pulling in all 16 per-variant headers here would be unnecessary coupling
// for one call each).  Signatures verified via grep against each variant's
// own header.
int GetShuttleState_1x1_1(int iSht, bool bPick);
int GetShuttleState_1x2_2_14(int iSht, bool bPick);
int GetShuttleState_1x2_2(int iSht, bool bPick);
int GetShuttleState_1x3_2_14(int iSht, bool bPick);
int GetShuttleState_1x3_4(int iSht, bool bPick);
int GetShuttleState_1x4_2_14(int iSht, bool bPick);
int GetShuttleState_1x4_4(int iSht, bool bPick);
int GetShuttleState_2x2_4_14(int iSht, bool bPick);
int GetShuttleState_2x2_4(int iSht, bool bPick);
int GetShuttleState_2x3_6(int iSht, bool bPick);
int GetShuttleState_2x3_6_14(int iSht, bool bPick);
int GetShuttleState_2x4_4(int iSht, bool bPick);
int GetShuttleState_2x4_8(int iSht, bool bPick);
int GetShuttleState_2x5_8(int iSht, bool bPick);
// GetShuttleState_2x6_8 / _2x8_8 come from the ainarm9045_2x6_8.h / _2x8_8.h
// includes above (already declared there).

// AI(W906-AutoCleanCluster) 20260722: same "plain forward declaration, real
// body binds at link time" idiom as the GetShuttleState_* block above --
// pulling in the whole ainarm9045_1x4_2.h for one call (DoAutoCleanPickfromCleanKit's
// e9045_1x4_2_14 branch) would be unnecessary coupling. Signature verified via
// grep against ainarm9045_1x4_2.h.
int GetNeedSuckActive_1x4_2_14(int iSht, bool bPick);
#include "forms/fShowBinSelect.h"   //AI(W906-FLOW-2) 20260928: TfShowBinSelect *fShowBinSelect (forms/fShowBinSelect.h:1133; btnCleanReset :953) -- DoAutoCleanPickfromCleanKit case 10 (golden AutoClean.cpp:2655/:2714)
// AI(W906-AutoCleanFoundation) 20260721: golden cContact.cpp:83
// `const int CONTACT_DEVICE_MAP_CHECK=9;` -- a plain `extern` forward decl
// left this undefined at link time (cContact.h/.cpp are not linked into
// ht9045_sm yet, and are being touched by a concurrent, unrelated wave today
// -- SECSGEM leaf-extraction -- so deliberately not added as a dependency
// here). A file-local constant with golden's exact real value is
// self-contained and avoids any coupling to that file's in-flight edits.
static const int CONTACT_DEVICE_MAP_CHECK = 9;

// AI(W906-AutoCleanFoundation) 20260721: golden ainarm9045.cpp's real
// DoInArm_SuckerMap (rebuilds the in-arm sucker map after a site-mapping
// change) has NO link-visible home anywhere in this tree yet -- the only
// existing reference (csystem.cpp) is its OWN TU-local
// `static void W7C2_DoInArm_SuckerMap(){}` macro'd stub, invisible outside
// that TU. File-local no-op mirrors the same posture (offline: the sucker
// map does not need rebuilding without a live site-mapping change).
#if 0   // PT-W7e-part2 RETIRED (DoInArm_SuckerMap): real translated body now in ainarm2.cpp
static void DoInArm_SuckerMap() {}
#endif

// golden AutoClean.cpp:46-47 -- file-scope globals (declared in AutoClean.h).
int iInXPos;
int iInYPos;
int iAutoCleanPlaceToShuttleTask=1;
// AI(W906-AutoCleanCluster) 20260722: golden AutoClean.cpp:44/48/50 -- the
// remaining file-scope globals now-in-scope functions read/write (declared in
// AutoClean.h; see that header's banner for why SHT_Kit/DoTestYRearDelayAC are
// deliberately NOT ported).
int iAutoCleanPickFromCleanKitStageTask=1;                                      //jou 2012-05-22
bool bFullViewCheckFinish=false;                                                //JerryYang 20160331
bool bInedxCleanFinish[2]={false,false};                                        //kevin 20170520 (wei) Autoclean index送片後

//------------------------------------------------------------------------------
//  golden AutoClean.cpp:51 -- file-scope timer DoInArmPineRelease alone uses.
//------------------------------------------------------------------------------
TQPF_Timer tCleanInArmServoOnDelay;

// AI(W906-AutoCleanCluster) 20260722: golden AutoClean.cpp:4300/4415 -- two more
// file-scope timers the core engines / orchestrator use (DoAutoCleanPlaceToCleanKit's
// eptUseCyn picker-settle delay; DoAutoCleanKit's E5-Wait-SECS handshake delay).
TQPF_Timer DoAutoCleanPlaceToCleanKitDelay;
TQPF_Timer SECSGEM_WaitSECS;                                                    //KevinCheng 20250919 : Wait SECS

// ---------------------------------------------------------------------------
//  Cross-module SYMBOLS that already have a LINK-VISIBLE definition in a
//  sibling translated TU but whose declaring header this file does not
//  include (same "plain forward declaration, real body binds at link time"
//  idiom already established by acarry_shims.h:206-210 for the identical
//  pair). AI(W906-AutoCleanFoundation) 20260721.
// ---------------------------------------------------------------------------
void SetMotorScaleSpeed(int Index, int ScaleSpeed); // golden cinitial.h:50 (body in acatchtray_shims.cpp)
void SetMotorAccelSpeed(int Index, int AccelSpeed); // golden cinitial.h:51 (body in acatchtray_shims.cpp, this wave)

// ---------------------------------------------------------------------------
//  Cross-module SYMBOLS with NO translated home ANYWHERE in this tree yet
//  (verified by grep): golden cinitial.cpp -- an entire not-yet-translated
//  module (reads Mot_Table.csv/IO_Table.csv, sets up every motor's real
//  accel/speed profile) that CleanSetSpeed's bBackup==false ("restore normal
//  speed") branch calls into. Standing up cinitial.cpp is a substantial
//  separate translation front, well beyond this wave's AutoClean-foundation
//  scope; minimal offline no-op stubs here (file-local, `static`) keep
//  CleanSetSpeed linkable without inventing cinitial.cpp's real logic.
//  MNetLog: golden's REAL body is Motor/myMN200motor.cpp (not in this tree);
//  Motor/mymotor.cpp already has its OWN static no-op precedent for the exact
//  same reason (see that file's "TODO(W5): replace with real MNetLog" note) --
//  mirrored here (file-local, so no cross-TU collision risk with whichever
//  wave eventually lands the real one).
// ---------------------------------------------------------------------------
void SetOutArmSpeed(bool bShow);                                //AI(W906-FLOW-2) 20260928: was a TU-local static no-op ('not yet translated'); the real body is cinitial.cpp:10760 (golden cinitial.cpp:5300), same archive ht9045_sm
void SetSortArmSpeed(bool bShow);                               //AI(W906-FLOW-2) 20260928: was a TU-local static no-op ('not yet translated'); the real body is cinitial.cpp:5454 (golden cinitial.cpp:5425), same archive ht9045_sm
extern void UpdateMyKitSuckDelayTimeToProd();                   // golden cinitial.cpp:6497  AI(W906-W3-12c) 20260925: 原本是檔內 static 空殼（「not yet translated」），把 cinitial.cpp:11087 的真本體遮掉 —— :888 的呼叫一直打到空殼（NB2 R17 RW-03）。改宣告真本體
static bool MNetLog(AnsiString /*Message*/) { return false; }   // golden Motor/myMN200motor.cpp -- not yet translated
// AI(W906-AutoCleanCluster) 20260722: golden cUnitConvert.cpp:243
// `void DoStructUnitConvert()` -- DoAutoCleanKit's case 1 calls this
// unconditionally (Steven: "recipe _File -> live struct" sync step covering
// TestIF/DeviceForm/HotPlateForm/ArmOffset/ArmSpeed/UserDefForm). This whole
// function (and everything it orchestrates) is STILL entirely un-translated in
// cUnitConvert.cpp -- verified: cUnitConvert.cpp/.h carry only `TODO(W6)`/
// `TODO(W6+W7)` comment stubs for DoStructUnitConvert and every Do*Convert
// leaf it calls, with NO function body anywhere (a genuine link gap, not
// merely gated behind #if 0). Standing up the real body is a substantial,
// separate translation front spanning cprod.h's InArmOffSet[]/OutArmOffSet[]/
// DeviceForm/HotPlateForm/UserDefForm structs (most unrelated to AutoClean),
// well beyond this cluster's scope. File-local no-op mirrors the exact same
// "cinitial.cpp/myMN200motor.cpp has no home yet" treatment as
// SetOutArmSpeed/SetSortArmSpeed/MNetLog immediately above (and
// DoInArm_SuckerMap below) -- keeps DoAutoCleanKit linkable without inventing
// cUnitConvert.cpp's real logic.
#if 0   // PT-W8 RETIRED (DoStructUnitConvert): real translated body now live
static void DoStructUnitConvert() {}                            // golden cUnitConvert.cpp:243 -- not yet translated
#endif
// AI(W906-AutoCleanCluster) 20260722: golden csystem.cpp:21360 `void
// SocketAirCoolingStart()` -- socket air-cooling fan on/off timer bookkeeping
// (IniConfig.bL03SocketAirCoolingCT gate + SW[SwTesterAirCooling] + a
// SocketAirCoolingTurnOnTimer/dSocketAirCoolingOffTimer pair). Declared in
// csystem.h (already included) but has NO definition ANYWHERE in this tree
// (verified by grep -- not even gated behind #if 0; a genuine link gap, same
// class as DoStructUnitConvert above). DoAutoCleanKit's only call site (case 5)
// is a fire-and-forget void statement -- no in-scope logic branches on it.
// Real translation needs new globals (bSocketAirCoolingTurnOnTimer/
// bSocketAirCoolingTurnOffTimer/its timer/Temperature.dSocketAirCoolingOffTimer)
// that belong to csystem.cpp's own not-yet-translated bulk, not this cluster.
// NOT `static`: csystem.h already declares it with external linkage (a `static`
// redefinition here would conflict with that prior declaration) -- this is the
// SOLE definition anywhere in the linked tree.
#if 0   // PT-W5f RETIRED (SocketAirCoolingStart)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error. Same convention as csystem_shims.cpp:165.
void SocketAirCoolingStart() {}                                  // golden csystem.cpp:21360 -- not yet translated
#endif
// AI(W906-AutoCleanCluster) 20260722: golden `bool bWaitSECS` (KevinCheng
// 20250919, Wait-SECS handshake flag) -- cmydef.h:5936 declares it extern, but
// its cmydef.cpp:6010 definition sits inside a pre-existing `#if 0 // TODO`
// gated block (verified: the enclosing gate opens at cmydef.cpp:5816 and is
// still open at line 6010) -- so, exactly like ainarm2.cpp's bDestoryOnSht
// precedent (see ainarm2.cpp's own banner), this is the ONE active definition.
bool bWaitSECS=false;                                            // golden cmydef.cpp:6010 (gated there; real home once cmydef.cpp ungates)

// AI(W906-AutoCleanFoundation) 20260721: golden AutoAlignment/AutoAlignment.h:230
// `void CheckInArmXYScaleByAutoTeach(int &iXPos, int &iYPos, int iArea);` --
// the whole AutoAlignment/ (AOA) subsystem is untranslated anywhere in this
// tree yet (verified by grep -- zero hits). MoveInArmXYPickCleanKit's only
// call site is double-gated (MACHINE_HAS_AUTO_ALIGNMENT_CCD && bEnableAutoAlignment,
// both false by default/offline), so a faithful-shape no-op (leaves iXPos/iYPos
// untouched, matching "no AOA scale correction available") keeps that branch
// linkable without inventing AOA logic that belongs to its own future wave.
static void CheckInArmXYScaleByAutoTeach(int &/*iXPos*/, int &/*iYPos*/, int /*iArea*/) {}

// AI(W906-AutoCleanCluster) 20260722: golden ainarm2.cpp:1684 (real body:
// AutoAlignment/AutoAlignment.cpp:8791) -- "is CCD Auto-Alignment enabled for
// the Clean-Kit (CK) position family, and (if bSet) latch that mode change".
// The whole AutoAlignment/ (AOA) subsystem is untranslated anywhere in this
// tree yet (verified by grep -- zero hits, same posture as
// CheckInArmXYScaleByAutoTeach above). DoAutoCleanKit's only call site (golden
// :4457) sits behind `MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true`
// (both false by default/offline) -- offline-false keeps that branch linkable
// without inventing AOA logic that belongs to its own future wave.
static bool CheckInArmAutoAlignmentCKModeBeUse(int /*iMode*/, bool /*bSet*/=false) { return false; }

// ---------------------------------------------------------------------------
//  AI(W906-AutoCleanCluster) 20260728: DoIndexAutoClean / DoIndexAutoClean_
//  Arm1PickArm2Test dependency stand-ins (the SECOND, final dependency
//  cluster this file's foundation/cluster waves deferred -- see the removed
//  placeholder banner that used to sit where DoIndexAutoClean's real body now
//  is, further down this file). Same "TU-local static function + #define
//  macro so the golden call-site syntax is preserved verbatim" idiom as
//  atester.cpp's W7T1_* cluster (atester.cpp:1540-1620) -- duplicated here
//  (not shared) because a `static` function in one TU has no linkage into
//  another TU.
// ---------------------------------------------------------------------------
// -- AI(W906-FLOW-2) 20260928: RETIRED stand-in -- the prose below is history: TfHome now exists (forms/fHome.h) and golden uhome.cpp:4891-4906 is translated at the END of this file.  Original: fHome->InitDoTestZHome() (golden uhome.cpp:4891) -- TfHome (uhome.h) is
//    not yet ported anywhere in this tree (verified by grep: the only other
//    "fHome" hit is csystem.cpp's own unrelated W7C2_FHOME_SERVOOFF macro for
//    a DIFFERENT TfHome method, GaliMotorServoOff). Golden's real body resets
//    TestZTask (TfHome's own PRIVATE Home-form state cursor -- invisible
//    outside TfHome, and never read back by DoIndexAutoClean or
//    DoIndexAutoClean_Arm1PickArm2Test) plus MOT[MTestY1/Y2/Z1/Z2].MovFlag/
//    bScanFlag/GaliSofDelayCount. Offline no-op: safe, since nothing in-scope
//    reads those MOT[] fields back either -- kept a pure no-op (not a partial
//    MOT[]-only translation) per this wave's task brief, which explicitly
//    asks for the W7T1_* no-op idiom here.
#include "forms/fHome.h"                                              //AI(W906-FLOW-2) 20260928: was 'static void W906DIAC_InitDoTestZHome() {}' (offline no-op); TfHome::InitDoTestZHome() and TfHome::TestZTask are declared there now (golden uhome.h:68/:74), body at the end of this file
#define W906DIAC_FHOME_INITDOTESTZHOME()   fHome->InitDoTestZHome()     // golden fHome->InitDoTestZHome()  //AI(W906-FLOW-2) 20260928: the real fHome (forms/fHome.cpp:37, static-init new TfHome())

// -- fiosetview->ProcessIndexSuckDestroy1()/2() (golden iosetview.h) -- the
//    TfiosetviewShim (atester_shims.h) exposes bIndexSuck[][][] but not these
//    two pump methods (the SAME gap atester.cpp's W7T1_FIOSET_PISD1/2 already
//    documents for its OWN call sites -- that pair is `static` to atester.cpp
//    and therefore not reusable here). Offline (no DAQ): report the suck
//    self-check "complete" (true) so the index-check SM advances, mirroring
//    W7T1's identical rationale.
static bool W906DIAC_ProcessIndexSuckDestroy1() { return fiosetview->ProcessIndexSuckDestroy1(); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）  // golden iosetview.h -- offline: suck self-check done
static bool W906DIAC_ProcessIndexSuckDestroy2() { return fiosetview->ProcessIndexSuckDestroy2(); }   //AI(W906-IDXSUCK) 20260927: 以前一律回 true，改成轉呼叫 golden 照翻的本體（atester_shims.cpp 檔尾）  // golden iosetview.h -- offline: suck self-check done
#define W906DIAC_FIOSET_PISD1()   W906DIAC_ProcessIndexSuckDestroy1()  // golden fiosetview->ProcessIndexSuckDestroy1()
#define W906DIAC_FIOSET_PISD2()   W906DIAC_ProcessIndexSuckDestroy2()  // golden fiosetview->ProcessIndexSuckDestroy2()

// -- file-scope timers DoIndexAutoClean / DoIndexAutoClean_Arm1PickArm2Test
//    use (golden AutoClean.cpp:51 / :6464). AutoClean.h's own banner already
//    explained why these were deliberately NOT declared earlier: their sole
//    readers/writers were entirely inside this (until-now out-of-scope)
//    cluster.
TQPF_Timer DoIndexAutoCleanDelay;   // golden AutoClean.cpp:6464
TQPF_Timer DoTestYRearDelayAC;      // golden AutoClean.cpp:51

// -- SetTechDataToProd_AutoClean() (golden cinitial.cpp:11269-11315) -- a REAL
//    (not stubbed) translation: DoIndexAutoClean's case 3000 calls it, and
//    (verified by reading the golden body in full) it has ZERO dependencies
//    beyond already-real globals (Prod/TestIF/TestIF_File/IniConfig/Offset --
//    cprod.h/Config.h/cmydef.h, all already included by this TU). cinitial.cpp
//    itself does not exist anywhere in this ported tree yet (verified by
//    grep/find -- zero hits); landing that whole file is well beyond this
//    wave's scope, so this ONE function is ported directly here instead, per
//    this wave's own task brief. Golden's own forward declaration
//    (`extern void SetTechDataToProd_AutoClean();`, AutoClean.cpp:6462) is
//    unnecessary here since the real body below is defined lexically before
//    its only call site further down this file.
void SetTechDataToProd_AutoClean()   // golden cinitial.cpp:11269-11315
{
    int iACParamSel=1;                                                          //Sam 20230620 : 優先 Smart Auto Clean
    if(IniConfig.bEnableAutoCleanFunction && iRunACSmart>0)                     //Sam 20230111 : Smart Auto Clean
    {
        if(TestIF_File.bACSmart &&
           TestIF_File.iACSmart_Count!=0 &&
           iACUseParam==2)
        {
            iACParamSel=2;
        }
    }

    if(iACParamSel==2)                                                          //Sam 20230620 : 優先 Smart Auto Clean
    {
        Prod.iAutoClean_ContactMode =TestIF_File.iACSmart_ContactMode;
        Prod.iAutoCleanDropHigh     =TestIF_File.iACSmart_DropHigh;
        Prod.iAutoClean_ContactTime =TestIF_File.iACSmart_ContactTime;
        Prod.iAutoClean_ContactCount=TestIF_File.iACSmart_ContactCount;
    }
    else
    {
        Prod.iAutoClean_ContactMode =TestIF_File.iAutoClean_ContactMode;
        Prod.iAutoCleanDropHigh     =TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoClean_ContactTime =TestIF_File.iAutoClean_ContactTime;
        Prod.iAutoClean_ContactCount=TestIF_File.iAutoClean_ContactCount;
    }

    if(IniConfig.bSPILFunction==true)                                           //JerryYang 20220330 SPIL客戶需求，把contact offset與auto clean offset分開
    {
        Prod.iAutoCleanZ_Contact[0] =Prod.TestZ1_Test-Offset.iIndexArmContact[0]-Prod.TestZ1_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness;
        Prod.iAutoCleanZ_Drop[0]    =Prod.TestZ1_Test-Offset.iIndexArmContact[0]-Prod.TestZ1_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[0]   =Prod.TestZ1_Test-Offset.iIndexArmContact[0]-Prod.TestZ1_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoClean_ContactShiftHeight;
        Prod.iAutoCleanZ_Contact[1] =Prod.TestZ2_Test-Offset.iIndexArmContact[1]-Prod.TestZ2_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness;
        Prod.iAutoCleanZ_Drop[1]    =Prod.TestZ2_Test-Offset.iIndexArmContact[1]-Prod.TestZ2_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[1]   =Prod.TestZ2_Test-Offset.iIndexArmContact[1]-Prod.TestZ2_Drop_Offset+TestIF_File.iAutoClean_ContactCleanHeight+TestIF_File.iPadThickness+TestIF_File.iAutoClean_ContactShiftHeight;
    }
    else
    {
        Prod.iAutoCleanZ_Contact[0] =Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness;
        Prod.iAutoCleanZ_Drop[0]    =Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+Prod.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[0]   =Prod.TestZ1_Test-Prod.TestZ1_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+TestIF.iAutoClean_ContactShiftHeight;
        Prod.iAutoCleanZ_Contact[1] =Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness;
        Prod.iAutoCleanZ_Drop[1]    =Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+Prod.iAutoCleanDropHigh;
        Prod.iAutoCleanZ_Shift[1]   =Prod.TestZ2_Test-Prod.TestZ2_Drop_Offset+TestIF.iAutoClean_ContactCleanHeight+TestIF.iPadThickness+TestIF.iAutoClean_ContactShiftHeight;
    }
}

//==============================================================================
//  Part A -- pure calc / config
//==============================================================================

//------------------------------------------------------------------------------
int GetAutoCleanPickCount()
{
    int iCount=4;
    if(TestIF.iTestMode==SingleSite ||                                          //Steven 20140614 : for Auto Clean Single Site
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
        iCount=1;
    }
    else if(bUseAxExPicker())
    {
        iCount=2;
    }
    else if(bUseAxxGPicker() ||                                                 //Steven 20240515 : 齊平吸嘴
            iCloseSiteModeFor1x4==e1x4CloseAbAc)                                //Steven 20241111 : for 1x4 close 2 site)
    {
        iCount=2;
    }
    else
    {
        if(bCleanKitPitchLess4000==true)                                        //Steven 20250923 : HT9045CN 治具最小是20x3mm
            iCount=1;
        else
            iCount=4;
    }
    return iCount;
}
//------------------------------------------------------------------------------
int GetAutoCleanPickStep(int iCol)
{
    int iSuckCol=-1;                                                            //Steven 20240918 : fixed for auto clean
    if(TestIF.iTestMode==SingleSite ||                                          //Steven 20140614 : for Auto Clean Single Site
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
        if(iCol==0)
        {
            if(Prod.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck)       //JerryYang 20260414 : fix 1x1 auto clean
                iSuckCol=1;
            else
                iSuckCol=0;
        }
    }
    else if(bUseAxExPicker() ||
            iInArmType==e9045_1x2_4_Hot)                                        //Jimmychiu 20260109 Add 1x2_4 auto clean error
    {
        if(iCol==0)
            iSuckCol=0;
        else if(iCol==1)
            iSuckCol=2;
    }
    else if(bUseAxxGPicker() ||                                                 //Steven 20240515 : 齊平吸嘴
            iCloseSiteModeFor1x4==e1x4CloseAbAc)                                //Steven 20241111 : for 1x4 close 2 site)
    {
        if(iCol==0)
            iSuckCol=0;
        else if(iCol==1)
            iSuckCol=3;
    }
    else
    {
        iSuckCol=iCol;
    }
    return iSuckCol;
}
//------------------------------------------------------------------------------
int CalculateAutoCleanXPitch()
{
    int iAutoCleanXPitch=1;

    if(TestIF.iTestMode==SingleSite ||
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
        iAutoCleanXPitch=1;
    }
    else if(bUseAxExPicker() ||
            i1x2_4UseACEGPicker==1)                                             //Jimmychiu 20260109 Add 1x2_4 auto clean error
    {
        iAutoCleanXPitch=(int)ceil(TestIF_File.iAutoClean_XDivision/2.0);
    }
    else if(bUseAxxGPicker() ||                                                 //Steven 20240515 : 齊平吸嘴
            iCloseSiteModeFor1x4==e1x4CloseAbAc)                                //Steven 20241111 : for 1x4 close 2 site)
    {
        iAutoCleanXPitch=(int)ceil(TestIF_File.iAutoClean_XDivision/2.0);
    }
    else if(TestIF.iTestMode==QualSite1X4 ||
            TestIF.iTestMode==_8Site2X4N)                                       //Wei 20231211 : 2X4NN Mode
    {
        iAutoCleanXPitch=(int)ceil(TestIF_File.iAutoClean_XDivision/4.0);
    }
    else
    {
        iAutoCleanXPitch=(int)ceil(TestIF_File.iAutoClean_XDivision/4.0);
    }
    return iAutoCleanXPitch;
}
//------------------------------------------------------------------------------
int GetXPitchOfCleanKit_HP()
{
    double dPitch=6000;

    bCleanKitPitchOver12000=false;

    if(TestIF.iTestMode==SingleSite ||                                          //wei 20220823 Single Site使用C治具判斷
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
        iAutoCleanUseXPitch=1;
        dPitch=iXpitchMaxX3;
    }
    else if(bUseAxExPicker())
    {
        if(TestIF.iAutoClean_XDivision==2 &&
            TestIF_File.iTestMode==QualSite2X2 &&
            TestIF.dAutoClean_XPitch>=iXpitchMinX2 &&
            TestIF.dAutoClean_XPitch<=iXpitchMaxX2)                                 //KevinCheng 20260421 : 2x2 cleankit only 2 cols
        {
            iAutoCleanUseXPitch=1;
        }
        else
        {
            iAutoCleanUseXPitch=2;
        }

        dPitch=double(TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch)/2.0*3.0;    //治具1顆治具3欄，比對4欄HotPlateForm.XPitch
        if(dPitch>(iXpitchMaxX3+300))                                           //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
        {
            iAutoCleanUseXPitch=1;
            dPitch=double(TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch)/2.0*3.0;
            bCleanKitPitchOver12000=true;
        }
    }
    else if(bUseAxxGPicker() ||                                                 //Steven 20240515 : 齊平吸嘴
            iCloseSiteModeFor1x4==e1x4CloseAbAc)                                //Steven 20241111 : for 1x4 close 2 site)
    {
        iAutoCleanUseXPitch=2;
        dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch;
        if(dPitch>(iXpitchMaxX3+300))                                           //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
        {
            iAutoCleanUseXPitch=1;
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch;
            bCleanKitPitchOver12000=true;
        }
    }
    else if(TestIF.iAutoClean_DeveicePices<=4)                                  //jou 2013-10-02 修正Auto Clean Qual site 1x4 pitch <= 20mm , 會超出安全範圍.
    {
        iAutoCleanUseXPitch=1;                                                  //kevin 20120229
        dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;                  //治具1顆治具2欄，比對2欄HotPlateForm.XPitch
    }
    else
    {
        if(TestIF.iAutoClean_XDivision>=8 &&
           TestIF.dAutoClean_XPitch<=2000)                                      //Steven 20251023 : fixed for 8x4 x-pitch=20mm auto clean
        {
            iAutoCleanUseXPitch=2;
        }
        else if(TestIF.dAutoClean_XPitch<2000)
        {
            iAutoCleanUseXPitch=2;                                              //Steven 20250930 : fixed for small pitch auto clean
        }
        else
        {
            iAutoCleanUseXPitch=1;                                              //kevin 20120229
        }
        dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;                  //治具1顆治具2欄，比對2欄HotPlateForm.XPitch
        if(dPitch>(iXpitchMaxX3+300))                                           //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
        {
            iAutoCleanUseXPitch=1;
            dPitch=iXpitchMaxX3;                                                //JerryYang 20200622 : x-pitch超過12000會撞機
            bCleanKitPitchOver12000=true;
        }
    }
    return int(dPitch);
}
//------------------------------------------------------------------------------
int GetXPitchOfCleanKit_Kit()
{
    double dPitch=6000;
    bCleanKitPitchOver12000=false;
    bCleanKitPitchLess4000=false;                                               //jou 2015-03-24 tray x pitch太小導致無法治具

    if(TestIF.iTestMode==SingleSite ||
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
        iAutoCleanUseXPitch=1;
        dPitch=iXpitchMaxX3;                                                    //Steven 20140614 : for Auto Clean Single Site   //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
    }
    else if(bUseAxExPicker() ||
            i1x2_4UseACEGPicker==1)                                             //Jimmychiu 20260109 Add 1x2_4 auto clean error
    {
        if(TestIF.iAutoClean_XDivision==2)
        {
            iAutoCleanUseXPitch=1;
        }
        else if(TestIF.iAutoClean_XDivision>2 &&                                //Steven 20240902 : 修正1x2 auto clean
                TestIF.iAutoClean_DeveicePices==2)
        {
            iAutoCleanUseXPitch=1;
        }
        else if(TestIF.bAutoClean_UseTray)                                      //Steven 20240902 : 修正1x2 auto clean
        {
            iAutoCleanUseXPitch=CalculateAutoCleanXPitch();
        }
        else
        {
            iAutoCleanUseXPitch=2;                                              //jou 2014-06-13 4 -> 2 修正 Dual site Clean kit 2x8 start:33 pitch:22 超過安全範圍
        }

        dPitch=double(TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch)/2.0*3.0;    //治具1顆治具3欄，比對4欄HotPlateForm.XPitch
        if(dPitch>(iXpitchMaxX3+300))                                           //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
        {
            iAutoCleanUseXPitch=1;
            dPitch=double(TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch)/2.0*3.0;
            bCleanKitPitchOver12000=true;
        }
        else if(dPitch<iXpitchMax)
        {
            if(TestIF.iAutoClean_XDivision_Kit==2)                              //JerryYang 20210224 : 修正dual site clean kit 1x2 x-pitch 25mm點位小數異常
            {
                iAutoCleanUseXPitch=2;
                dPitch=double(TestIF.dAutoClean_XPitch)*3.0;
            }
            else
            {
                iAutoCleanUseXPitch=4;                                          //Alick 20161107 修正使用CLEAN TRAY,TRAY X-PITCH<13.33會造成點位計算異常,iAutoCleanUseXPitch=4,規劃CleanIC數量4&8
                dPitch=double(TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch)/2.0*3.0;
            }
            bCleanKitPitchLess4000=true;
        }
    }
    else if(bUseAxxGPicker() ||                                                 //Steven 20240515 : 齊平吸嘴
            iCloseSiteModeFor1x4==e1x4CloseAbAc)                                //Steven 20241111 : for 1x4 close 2 site
    {
        if(TestIF.iAutoClean_XDivision==2)
        {
            iAutoCleanUseXPitch=1;
        }
        else if(TestIF.iAutoClean_XDivision==8)                                 //Steven 20250318 : fix for 1x2 / 2x2 auto clean with 8 col, x-pitch 25mm
        {
            iAutoCleanUseXPitch=4;
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch;
            if(dPitch>iXpitchMaxX3)                                             //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
            {
                iAutoCleanUseXPitch=2;
            }
        }
        else if(TestIF.iAutoClean_XDivision>2 &&                                //Steven 20240902 : 修正1x2 auto clean
                TestIF.iAutoClean_DeveicePices==2)
        {
            iAutoCleanUseXPitch=1;
        }
        else if(TestIF.bAutoClean_UseTray)
        {
            iAutoCleanUseXPitch=CalculateAutoCleanXPitch();
        }
        else
        {
            iAutoCleanUseXPitch=2;                                              //jou 2014-06-13 4 -> 2 修正 Dual site Clean kit 2x8 start:33 pitch:22 超過安全範圍
        }

        dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch;                    //治具1顆治具4欄，比對2欄HotPlateForm.XPitch  //Steven 20180829 : #P180827-ATK-H9-01 Pickup position was wrong during Auto Clean in 2x2 mode.
        if(dPitch>iXpitchMaxX3)                                                 //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
        {
            iAutoCleanUseXPitch=1;
            dPitch=iXpitchMaxX3;                                                //Steven 20241111 : for 1x4 run 3 col tray auto clean
            bCleanKitPitchOver12000=true;
        }
        else if(dPitch<iXpitchMax)
        {
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch;
            bCleanKitPitchLess4000=true;                                        //jou 2015-03-24 tray x pitch太小導致無法治具
        }
    }
    else
    {
        if(InArmSuck.iPickCol==4 &&
           TestIF.iAutoClean_XDivision==8 &&                                    //JerryYang 20241002 : X 數量8 齊平治具
           TestIF.iAutoClean_DeveicePices%8==0 &&                               //Sam 20250207 : 要齊平治具 Clean Pad 數量也要對
           TestIF.dAutoClean_XPitch*2<iXpitchMin)                               //Steven 20250923 : HT9045CN 治具最小是20x3mm
        {
            iAutoCleanUseXPitch=4;
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;              //治具1顆治具2欄，比對2欄HotPlateForm.XPitch
            bCleanKitPitchLess4000=true;

            if(dPitch>iXpitchMaxX3)
            {
                int dSumXPitch=0.0;
                iAutoCleanUseXPitch=2;
                for(int igap=0; igap<5; igap++)
                {
                    dSumXPitch=TestIF.dAutoClean_XPitch*igap;
                    if(dSumXPitch>(double)iXpitchMin)
                    {
                        iAutoCleanUseXPitch=igap;
                        break;
                    }
                }
                bCleanKitPitchLess4000=true;
                dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;          //JimmyChiu 20240415 : modified for auto clean
            }
        }
        else if(InArmSuck.iPickCol==4 &&
                TestIF.iAutoClean_XDivision==8 &&                               //JerryYang 20241002 : X 數量8 齊平治具
                TestIF.iAutoClean_DeveicePices%8==0 &&                          //Sam 20250207 : 要齊平治具 Clean Pad 數量也要對
                TestIF.dAutoClean_XPitch*2<=iXpitchMax)
        {
            iAutoCleanUseXPitch=2;
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;              //治具1顆治具2欄，比對2欄HotPlateForm.XPitch

            if(dPitch<iXpitchMax)
            {
                dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*2*3;
                bCleanKitPitchLess4000=true;
            }
        }
        else if(TestIF.bAutoClean_UseTray &&
                TestIF.dAutoClean_XPitch>=iXpitchMin)
        {
            iAutoCleanUseXPitch=1;
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;              //治具1顆治具2欄，比對2欄HotPlateForm.XPitch

            if(dPitch>iXpitchMaxX3)                                             //Isaac 20171204 (Steven) : Xpitch40->50mm, 12000->iXpitchMaxX3
            {
                iAutoCleanUseXPitch=1;
                dPitch=iXpitchMaxX3;                                            //治具1顆治具2欄，比對2欄HotPlateForm.XPitch
                bCleanKitPitchOver12000=true;
            }
        }
        else if(TestIF.dAutoClean_XPitch<iXpitchMin)                            //Steven 20241111 : 1333 -> iXpitchMin
        {
            int dSumXPitch=0.0;
            iAutoCleanUseXPitch=2;
            for(int igap=0; igap<5; igap++)
            {
                dSumXPitch=TestIF.dAutoClean_XPitch*igap;
                if(dSumXPitch>(double)iXpitchMin)
                {
                    iAutoCleanUseXPitch=igap;
                    break;
                }
            }
            bCleanKitPitchLess4000=true;
            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;              //JimmyChiu 20240415 : modified for auto clean
        }
        else if(TestIF.dAutoClean_XPitch>iXpitchMax)                            //Steven 20240305 : 修正Auto Clean Pitch判斷
        {
            iAutoCleanUseXPitch=2;
            bCleanKitPitchOver12000=true;
            dPitch=TestIF.dAutoClean_XPitch*2*3;
        }
        else
        {
            if(InArmSuck.iPickCol==4 &&                                         //JerryYang 20241002 : X 數量8 齊平治具
               TestIF.iAutoClean_XDivision==8 &&
               TestIF.iAutoClean_DeveicePices%8==0 &&                           //Sam 20250207 : 要齊平治具 Clean Pad 數量也要對
               TestIF.dAutoClean_XPitch*2*3<iXpitchMaxX3)                       //Sam 20251020 : 齊平治具要在最大 PitcX 內
            {
                iAutoCleanUseXPitch=2;
            }
            else if(InArmSuck.iPickCol==4 &&                                    //JerryYang 20241002 : X 數量8 齊平治具
                    TestIF.iAutoClean_XDivision==12 &&
                    TestIF.iAutoClean_DeveicePices%12==0 &&                     //Sam 20250207 : 要齊平治具 Clean Pad 數量也要對
                    TestIF.dAutoClean_XPitch*2*3<iXpitchMaxX3)                  //Sam 20251020 : 齊平治具要在最大 PitcX 內
            {
                iAutoCleanUseXPitch=2;
            }
            else
            {
                iAutoCleanUseXPitch=1;
            }

            dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*3;              //治具1顆治具2欄，比對2欄HotPlateForm.XPitch

            if(dPitch<iXpitchMax)
            {
                dPitch=TestIF.dAutoClean_XPitch*iAutoCleanUseXPitch*2*3;
                bCleanKitPitchLess4000=true;
            }
        }
    }
    return int(dPitch);
}
//------------------------------------------------------------------------------
int GetXPitchOfCleanKit()
{
    if(IniConfig.bE43AutoCleanUseHotplate ||
       TestIF_File.iTestMode==_6Site2X3)                                        //Steven 20171220 (Wei) : Fixed for 2x3 Auto Clean Hang Up
        return GetXPitchOfCleanKit_HP();
    else
        return GetXPitchOfCleanKit_Kit();
}
//------------------------------------------------------------------------------
int GetYPitchOfCleanKit()
{
    int iYVariable=TestIF.iARM_Y_PITCH;

    if(IniConfig.bE43AutoCleanUseHotplate && USE_IN_Y_IS_AUTO_PITCH==true)            //JerryYang 20251218 : IN/OUT ARM支援不同模式  //Steven 20160629
    {
        if(TestIF_File.dAutoClean_YPitch<IN_OUT_ARM_Y_PITCH_MIN)                //Steven 20180824 : 修正Y-Pitch小於15mm
        {
            iYVariable=TestIF_File.iARM_Y_PITCH;
        }
        else if(TestIF_File.dAutoClean_YPitch>IN_OUT_ARM_Y_PITCH_MAX)           //Steven 20240801 : fixed for auto clean.
        {
            iYVariable=TestIF_File.iARM_Y_PITCH;
        }
        else
        {
            iYVariable=TestIF_File.dAutoClean_YPitch;
        }
    }

    return iYVariable;
}
//------------------------------------------------------------------------------
bool RunAutoCleanByArmPickArm2Test()                                            //Jimmychiu 20230710 : Auto Clean 用 Arm1 下料 arm2 測試
{
    return (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&
            TestIF_File.bArm1PickPlaceArm2Test==true &&
            TestIF_File.bArm1PickPlaceArm2Test_RunAutoClean==true);
}
//------------------------------------------------------------------------------
bool Special_2X6_Tray_XItem7()                                                  //Sam 20250712 : 新增特殊流程 2X6 AutoClean Tray XItem=7
{                                                                               //使用 Auto Clean Tray 模式且Tray的 XItem 為 7，最右邊 8 欄的物理位置沒有 IC，只當作補位用
    if(iInArmType==e9045_2x6_8                  &&
       TestIF_File.bAutoClean_UseTray           &&
       TrayForm.Loader.XDivision==7             &&
       TestIF_File.iAutoClean_XDivision_Tray==8 &&
       TestIF_File.iAutoClean_YDivision_Tray==2 &&
       LastSet.bUseTestSocket[0][0][5]==true    &&
       LastSet.bUseTestSocket[0][1][5]==true)
    {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void GetInarmSuckRow(int iShtRowKit,int &isuckRow,int &ikitStep)
{
    if(iShtRowKit==1 || iShtRowKit==2)                                          //JerryYang 20160411 當X-Pitch > 40mm,要拆成4欄用shuttle
    {
        ikitStep=0;
        isuckRow=iShtRowKit-1;
    }
    else
    {
        ikitStep=4;
        isuckRow=iShtRowKit-3;
    }

    if(isuckRow<0)
        isuckRow=0;
}

//==============================================================================
//  Part A -- HAL-only state-machine helpers
//==============================================================================

//------------------------------------------------------------------------------
void InitialAutoCleanAllTask()                                                  //Sam 20230504 : 整理 InitialAutoCleanTask
{
    InitialAutoCleanTask();
    InitialShuttleAutoCleanTask();
    InitialIndexAutoCleanTask();
    if(iOneCycle==1 && bIsAutoOneCycle==false && bManualOneCycle)               //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 完成的時又觸發 AutoClean 動作，AutoClean 完成才會執行 OneCycle Finish
    {
        bBackupOneCycle_ByAutoClean=true;
        bManualOneCycle=false;
    }
    bIsAutoOneCycle=true;
    fMain->BtnOneCycleClick(fMain);
}
//------------------------------------------------------------------------------
void InitialAutoCleanTask()
{
    iDoAutoCleanTask=1;
    bPlaceToShuttleByAutoClean=false;                                           //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
    bPickFromShuttleByAutoClean=false;                                          //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
    bLockPlaceToShuttleByAutoClean=false;                                       //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
    bLockPickFromShuttleByAutoClean=false;                                      //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion

    if(CUSTOMER_CODE==CC_SPIL_TAICHUNG_LOGIC &&
       IniConfig.bFTPJamCodeUpload &&
       IniConfig.bEnableFTP)                                                    //Steven 20140917 : 台中SPIL要求Auto Clean要傳送Alarm Code
    {
        // AI(W906-AutoCleanFoundation) 20260721: golden body here is
        // `fNote->aJamCodeFilePath=fFTPClient->SaveJamCodeFile(...); fFTPClient->
        // UploadFileToServer2(...);` -- the KYECFTP translation wave deliberately
        // "demoted" fFTPClient away from a singleton (see KYECFTP/FTPClient_Transfer.h
        // -- UploadFileToServer2 is now a free function, no `fFTPClient->` surface
        // exists anymore), and SaveJamCodeFile was never translated at all
        // (verified by grep -- zero hits). Re-introducing the singleton pattern
        // that wave deliberately removed is out of scope here; gated (matches
        // this tree's established fNote/fFTPClient-no-home precedent -- see
        // FormsFacade.h's TfNote banner). CUSTOMER_CODE==CC_SPIL_TAICHUNG_LOGIC
        // is a narrow single-customer condition, default unreachable elsewhere.
        //   fNote->aJamCodeFilePath=fFTPClient->SaveJamCodeFile(IniConfig.SocketHandlerID, Now(), "MES1607", "Autoclean Start");
        //   fFTPClient->UploadFileToServer2(IniConfig.FtpUplaodPath, fNote->aJamCodeFilePath);    //Steven 20140513 : 移動到Form Show之前
    }
}
//------------------------------------------------------------------------------
void InitialShuttleAutoCleanTask()
{
    iDoShuttle1AutoCleanTask=1;
    iDoShuttle2AutoCleanTask=1;
    iDoShuttleAutoCleanTask=1;
}
//------------------------------------------------------------------------------
void InitialIndexAutoCleanTask()
{
    iDoIndexAutoCleanTask=1;
}
//------------------------------------------------------------------------------
void InitPickFromShuttleTask()
{
    iAutoCleanPickFromShuttleTask=1;
}
//------------------------------------------------------------------------------
void InitPlaceToShuttleTask()
{
    iAutoCleanPlaceToShuttleTask=1;
}
//------------------------------------------------------------------------------
void CleanSetSpeed(bool bBackup)
{
    UpdateMyKitSuckDelayTimeToProd();                                           //Steven 20250319 : 治具OnDelayTime轉換獨立function
    if(bBackup==true)                                                           //Store and Set System Original Speed
    {
        ArmSpeed[InArm].iBodySP     =TestIF.iAutoClean_MotorSpeed[0];           //In Arm
        ArmSpeed[InArm].iACDCBodySP =TestIF.iAutoClean_MotorSpeed[0];           //In Arm
        ArmSpeed[InArm].iVariSP     =TestIF.iAutoClean_MotorSpeed[0];           //In arm pitch
        ArmSpeed[InArm].iACDCVariSP =TestIF.iAutoClean_MotorSpeed[0];           //In arm pitch
        SHSpeed.iSH1Sp              =TestIF.iAutoClean_MotorSpeed[1];           //Shuttle 1
        SHSpeed.iSH1ACDCSp          =TestIF.iAutoClean_MotorSpeed[1];           //Shuttle 1
        SHSpeed.iSH2Sp              =TestIF.iAutoClean_MotorSpeed[1];           //Shuttle 2
        SHSpeed.iSH2ACDCSp          =TestIF.iAutoClean_MotorSpeed[1];           //Shuttle 2
        ArmSpeed[InArm].iACDCZSP    =TestIF.iAutoClean_MotorSpeed[3];           //In Arm Z
        ArmSpeed[InArm].iZSP        =TestIF.iAutoClean_MotorSpeed[3];           //In Arm Z
    }
    else
    {
        ArmSpeed[InArm].iBodySP     =ArmSpeed_File[InArm].iBodySP;              //In Arm
        ArmSpeed[InArm].iACDCBodySP =ArmSpeed_File[InArm].iACDCBodySP;          //In Arm
        ArmSpeed[InArm].iVariSP     =ArmSpeed_File[InArm].iVariSP;              //In arm pitch
        ArmSpeed[InArm].iACDCVariSP =ArmSpeed_File[InArm].iACDCVariSP;          //In arm pitch
        SHSpeed.iSH1Sp              =SHSpeed_File.iSH1Sp;                       //Shuttle 1
        SHSpeed.iSH1ACDCSp          =SHSpeed_File.iSH1ACDCSp;                   //Shuttle 1
        SHSpeed.iSH2Sp              =SHSpeed_File.iSH2Sp;                       //Shuttle 2
        SHSpeed.iSH2ACDCSp          =SHSpeed_File.iSH2ACDCSp;                   //Shuttle 2
        ArmSpeed[InArm].iACDCZSP    =ArmSpeed_File[InArm].iACDCZSP;             //In Arm Z
        ArmSpeed[InArm].iZSP        =ArmSpeed_File[InArm].iZSP;                 //In Arm Z
        SetOutArmSpeed(false);                                                  //Steven 20130620 : 重置Out Arm速度
        if(USE_OUT_SORT_ARM!=eartUninstall)                                     //RogerYang 20250515 add for 9046AU
            SetSortArmSpeed(false);
    }

//AI(W906-FLOW-2) 20260928: GATE (W7a-I4) RETIRED (was '#if 0') -- golden AutoClean.cpp:770-815, the motor-speed PUSH tail, is live again.  Its own retirement condition is met: GATE (W5a-G) was OPENED 20260920 (cinitial.cpp:3833), and wb_serve runs InitialHandler -> InitHontechHardware -> InitialMotorParameter (tools/wb_serve.cpp:4066) before the tick loop (:4493), so MOT[].Motor is non-NULL wherever this runs.  The gate note below is history.
//   WHY GATED, and why this is behaviour-neutral: every call below reaches
//   SetMotorAccelSpeed (cinitial.cpp:16518) / SetMotorScaleSpeed (cinitial.cpp:13296),
//   whose golden bodies deref MOT[Index].Motor->Enable UNGUARDED. Golden may do that
//   because golden ALWAYS constructs the motor objects in InitialMotorParameter and then
//   merely sets Enable=false offline (attach-then-disable, never NULL). In this port that
//   construction is still behind GATE (W5a-G), so MOT[].Motor is NULL offline and the
//   deref SEGFAULTs -- measured 20260810: PT-W7a retired the acatchtray_shims.cpp no-op
//   stubs these calls used to land on, and ctest went 128->127 with 27-AutoClean SEGFAULT
//   in BOTH Debug and Release. Until W5a-G lands, these pushes were ALREADY no-ops (the
//   retired stubs did nothing), so gating them reproduces HEAD's behaviour exactly rather
//   than inventing new behaviour.
//   Deliberately NOT done instead: (a) adding `MOT[i].Motor &&` to the two real bodies --
//   rejected because for a MOTION-SPEED setter, silently skipping the push leaves a motor
//   at whatever rate was last programmed, which is a worse failure mode than crashing;
//   (b) a file-local static no-op shadow here -- rejected because that is exactly the
//   macro/static-seam class task #15 exists to remove, and it would hide the call entirely.
//   RETIRES WHEN: task #10 (GATE W5a-G) constructs MOT[].Motor with Enable=false. At that
//   point expired_gate_scan.py will surface this gate on its own.
//   NOTE: `int iMot` (below) is declared and used only inside this block; nothing after
//   :953 reads it, so gating the whole range leaves no dangling reference.
//   AI(pt-wave) 20260810
    SetMotorAccelSpeed(MInArmX      ,ArmSpeed[InArm].iACDCBodySP);
    SetMotorAccelSpeed(MInArmY      ,ArmSpeed[InArm].iACDCBodySP);
    SetMotorScaleSpeed(MInArmX      ,ArmSpeed[InArm].iBodySP);
    SetMotorScaleSpeed(MInArmY      ,ArmSpeed[InArm].iBodySP);

    SetMotorAccelSpeed(MInArmPitch  ,ArmSpeed[InArm].iACDCVariSP);
    SetMotorScaleSpeed(MInArmPitch  ,ArmSpeed[InArm].iVariSP);

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                                  //JerryYang 20251218 : IN/OUT ARM支援不同模式  //Steven 20160630 : 限制Y-Pitch最低速度為80
    {
        if(ArmSpeed[InArm].iACDCVariSP<80)
            SetMotorAccelSpeed(MInArmPitchY, 80);
        else
            SetMotorAccelSpeed(MInArmPitchY, ArmSpeed[InArm].iACDCVariSP);

        if(ArmSpeed[InArm].iVariSP<80)
            SetMotorScaleSpeed(MInArmPitchY, 80);
        else
            SetMotorScaleSpeed(MInArmPitchY, ArmSpeed[InArm].iVariSP);
        SetMotorAccelSpeed(MInArmPitchX2, ArmSpeed[InArm].iACDCVariSP);
        SetMotorScaleSpeed(MInArmPitchX2, ArmSpeed[InArm].iVariSP);
    }

    int iMot=MInArmZA;
    if(InOutArmPickerUseMotor==eptUseMotCyn)
    {
        SetMotorAccelSpeed(MInArmZA, ArmSpeed[InArm].iACDCZSP);
        SetMotorScaleSpeed(MInArmZA, ArmSpeed[InArm].iZSP);
    }
    else
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                iMot=InArmSuck.Suck[i][j].iMotNo;
                SetMotorAccelSpeed(iMot, ArmSpeed[InArm].iACDCZSP);
                SetMotorScaleSpeed(iMot, ArmSpeed[InArm].iZSP);
            }
        }
    }

    SetMotorAccelSpeed(MInShuttle1      ,SHSpeed.iSH1Sp);
    SetMotorScaleSpeed(MInShuttle1      ,SHSpeed.iSH1ACDCSp);
    SetMotorAccelSpeed(MInShuttle2      ,SHSpeed.iSH2Sp);
    SetMotorScaleSpeed(MInShuttle2      ,SHSpeed.iSH2ACDCSp);
//AI(W906-FLOW-2) 20260928: (end of the retired GATE (W7a-I4) range)

    fShowMessage->ShowSpeed(IniConfig.bG05ShowSpeedMessage);
}
//------------------------------------------------------------------------------
void InOutArmSuckActiveSet()
{
    ZeroMemory(bInArmSuckActive, sizeof(bInArmSuckActive));
}
//------------------------------------------------------------------------------
// kevin 20120217 Z軸移到 SHUTTLE 2 位置
//------------------------------------------------------------------------------
bool MoveInArmZToShuttlePlace(eWhichShuttle iSht, int iRowSel)                  //20140923 wei : For Shuttle Auto Clean
{
    int iShuttlePlace[MAX_ARM_Row][MAX_ARM_Col];
    bool flag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
    bool bNeedDown;

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            if(iSht==euShuttle1)
            {
                if(IniConfig.bEnableAutoCleanFunction &&
                   TestIF_File.iAutoClean_Function &&
                   bRunAutoClean &&
                   IniConfig.bE48_ShuttleUse4Offset_Autoclean &&                //20140923 wei : For Shuttle Auto Clean
                   TestIF.bEnableAutoAlignment==false)                          //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    iShuttlePlace[i][j]=Prod.ZInArm_Shuttle1_Place[i][j] -
                                        InArmOffSet[InOfsInSh1]->GetPlace() +
                                        InArmOffSet[InOfsInSh1_AutoClean]->GetPlace()+
                                        InArmOffSet[InOfsInSh1_AutoClean+iRowSel-1]->SingleOffSet->dPlaceOffSet[i][j];
                }
                else if(CosFunction.bAutoCleanOffsetUseSingleSetting &&         //Sam 20220720 : AutoClean Offset 值使用 Clean 設定 Offset 來套用
                        TestIF.bEnableAutoAlignment==false)                     //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    iShuttlePlace[i][j]=Prod.ZInArm_Shuttle1_Place[i][j]+TestIF.iAutoClean_Shuttle1PlaceOffset-InArmOffSet[InOfsInSh1]->GetPlace();
                }
                else
                {
                    iShuttlePlace[i][j]=Prod.ZInArm_Shuttle1_Place[i][j]+TestIF.iAutoClean_Shuttle1PlaceOffset;
                }
            }
            else
            {
                if(IniConfig.bEnableAutoCleanFunction &&
                   TestIF_File.iAutoClean_Function &&
                   bRunAutoClean &&
                   IniConfig.bE48_ShuttleUse4Offset_Autoclean &&                //20140923 wei : For Shuttle Auto Clean
                   TestIF.bEnableAutoAlignment==false)                          //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    iShuttlePlace[i][j]=Prod.ZInArm_Shuttle2_Place[i][j] -
                                        InArmOffSet[InOfsInSh2]->GetPlace() +
                                        InArmOffSet[InOfsInSh2_AutoClean]->GetPlace()+
                                        InArmOffSet[InOfsInSh2_AutoClean+iRowSel-1]->SingleOffSet->dPlaceOffSet[i][j];
                }
                else if(CosFunction.bAutoCleanOffsetUseSingleSetting &&         //Sam 20220720 : AutoClean Offset 值使用 Clean 設定 Offset 來套用
                        TestIF.bEnableAutoAlignment==false)                     //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    if(IniConfig.bE34InOutArmPitchZOffsetSameOne)
                        iShuttlePlace[i][j]=Prod.ZInArm_Shuttle2_Place[i][j]+TestIF.iAutoClean_Shuttle2PlaceOffset-InArmOffSet[InOfsInSh1]->GetPlace();
                    else
                        iShuttlePlace[i][j]=Prod.ZInArm_Shuttle2_Place[i][j]+TestIF.iAutoClean_Shuttle2PlaceOffset-InArmOffSet[InOfsInSh2]->GetPlace();
                }
                else
                {
                    iShuttlePlace[i][j]=Prod.ZInArm_Shuttle2_Place[i][j]+TestIF.iAutoClean_Shuttle2PlaceOffset; //JerryYang 20160301 修正Offset,1-->2
                }
            }
        }
    }

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            bNeedDown=(InArmSuck.Item[i][j]!=NULL_IC &&
                       InArmSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                       InArmSuck.Suck[i][j].GetNeedDestroyStatus());

            flag[i][j]=bNeedDown;
        }
    }

    bool bRet=InArmZMoveDown(flag, iShuttlePlace, false);                       //Steven 20251020 : modify for auto clean
    return bRet;
}
//------------------------------------------------------------------------------
//0 Full
//1 Row1 - Kit1
//2 Row2 - Kit1
//3 Row1 - Kit2
//4 Row2 - Kit2
int GetShuttleState(eWhichShuttle iSht, bool bPick)
{
    if(iSht==euShuttle1)
        ptrInSHT=&FLCarryKit;
    else
        ptrInSHT=&BLCarryKit;

    if(TestIF.iTestMode==SingleSite ||
       iInArmType==e9045_1x4_1_Ac)                                              //Steven 20200720 : 1x4只開site Ac
    {
        return GetShuttleState_1x1_1((int)iSht, bPick);
    }
    else if(iInArmType==e9045_1x2_2_14)                                         //Steven 20221027 : 修正Auto Clean
    {
        return GetShuttleState_1x2_2_14((int)iSht, bPick);
    }
    else if(TestIF.iTestMode==DualSite || TestIF.iTestMode==QualSite2X2N)
    {
        return GetShuttleState_1x2_2((int)iSht, bPick);
    }
    else if(iInArmType==e9045_1x3_2_14)
    {
        return GetShuttleState_1x3_2_14((int)iSht, bPick);
    }
    else if(iInArmType==e9045_1x3_4)
    {
        return GetShuttleState_1x3_4((int)iSht, bPick);
    }
    else if(iInArmType==e9045_1x4_2_14)
    {
        return GetShuttleState_1x4_2_14((int)iSht, bPick);
    }
    else if(TestIF.iTestMode==QualSite1X4 ||
            TestIF.iTestMode==_8Site2X4N)                                       //Wei 20231211 : 2X4NN Mode
    {
        return GetShuttleState_1x4_4((int)iSht, bPick);
    }
    else if(iInArmType==e9045_2x2_4_14)                                         //Steven 20221027 : 修正Auto Clean
    {
        return GetShuttleState_2x2_4_14((int)iSht, bPick);
    }
    else if(TestIF_File.iTestMode==QualSite2X2)
    {
        return GetShuttleState_2x2_4((int)iSht, bPick);
    }
    else if(iInArmType==e9045_2x3_6)
    {
        return GetShuttleState_2x3_6((int)iSht, bPick);
    }
    else if(iInArmType==e9045_2x3_6_14)
    {
        return GetShuttleState_2x3_6_14((int)iSht, bPick);
    }
    else if(iInArmType==e9045_2x4_4_14)
    {
        return GetShuttleState_2x4_4((int)iSht, bPick);
    }
    else if(TestIF_File.iTestMode==_8Site2X4 ||
            TestIF_File.iTestMode==_16Site4X4)
    {
        return GetShuttleState_2x4_8((int)iSht, bPick);
    }
    else if(TestIF_File.iTestMode==_10Site2X5)
    {
        return GetShuttleState_2x5_8((int)iSht, bPick);
    }
    else if(TestIF_File.iTestMode==_12Site2X6)
    {
        return GetShuttleState_2x6_8((int)iSht, bPick);
    }
    else if(TestIF_File.iTestMode==_16Site2X8)
    {
        return GetShuttleState_2x8_8((int)iSht, bPick);
    }
    else if(TestIF_File.iTestMode==_32Site4X8N)
    {
        return GetShuttleState_2x8_8((int)iSht, bPick);
    }
    else
    {
        ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "GetShuttleState");
    }
    return 0;
}
//------------------------------------------------------------------------------
bool bCleanZMotToPlace[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool bCleanZSuckToPlace[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};
bool MoveInOutArmZToKitPickPlace(int Pick, bool bReset, int iSht, int iShuttleRow)  //ChungHung 20150303 add iSht for Hotplate AutoClean
{
    static bool bflag[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};

    bool flag=false;
    int iZPos[MAX_ARM_Row][MAX_ARM_Col], iMot;

    if(bReset)
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                bflag[i][j]=false;
            }
        }
    }

    for(int j=0; j<InArmSuck.iMotCol; j++)
    {
        if(Pick==bAutoPick)
        {
            iZPos[0][j]=Prod.ZInArm_AutoClean_Pick[0][j]+iArmPickTrayPos;
            iZPos[1][j]=Prod.ZInArm_AutoClean_Pick[1][j]+iArmPickTrayPos;
        }
        else
        {
            iZPos[0][j]=Prod.ZInArm_AutoClean_Place[0][j]+iArmPlaceTrayPos;
            iZPos[1][j]=Prod.ZInArm_AutoClean_Place[1][j]+iArmPlaceTrayPos;
        }
    }

    if(bUse8Picker==false && IniConfig.bE43AutoCleanUseHotplate==false)            //Steven 20260504 : bUse8Picker==false only B-row(row1) Z descent
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            if(bInArmSuckActive[0][j])
            {
                // AI(W906-AutoCleanFoundation) 20260721: golden's real
                // MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2="")
                // is 3-arg (cMyDB.h:20); the target's MyDBIProcess is 2-arg
                // (S1,S2) -- every existing call site in this tree already
                // drops golden's 3rd arg the same way (grepped: acarry.cpp
                // etc. all pass exactly 2 args). Folding "Protection" into the
                // message text preserves the information instead of silently
                // dropping it.
                MyDBIProcess("AutoClean",
                    AnsiString().sprintf("MoveInOutArmZToKitPickPlace: bInArmSuckActive[0][%d]=true force false [Protection]", j));
                bInArmSuckActive[0][j]=false;
            }
        }
    }

    if(Pick==bAutoPick)
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                iMot=InArmSuck.Suck[i][j].iMotNo;
                if(bInArmSuckActive[i][j])
                {
                    if(bflag[i][j]==false)
                        bflag[i][j]=MOT[iMot].MotorMove(iZPos[i][j]);
                }
                else
                {
                    bflag[i][j]=true;
                }
            }
        }
    }
    else
    {
        uHPSuckTeam* ppInfo=PlaceToCleanList->ExtractFirstTeam();
        if(PlaceToCleanList->GetHPFirstTeamMotUse(bCleanZMotToPlace)==false)        //RogerYang 20250624 Add HotPlate ErrMessage
        {
            ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamMotUse", 0));
            return false;
        }

        PlaceToCleanList->GetHPFirstTeamMotUse(bCleanZSuckToPlace);                 //JerryYang 202050813 : fix合併錯誤治具下降
        if(ppInfo!=NULL)
        {
            for(int i=0; i<InArmSuck.iMotRow; i++)
            {
                for(int j=0; j<InArmSuck.iMotCol; j++)
                {
                    iMot=InArmSuck.Suck[i][j].iMotNo;
                    if(bCleanZMotToPlace[i][j])
                    {
                        if(bflag[i][j]==false)
                            bflag[i][j]=MOT[iMot].MotorMove(iZPos[i][j]);
                    }
                    else
                    {
                        bflag[i][j]=true;
                    }
                }
            }
        }
        else
        {
            ShowMyMessage("HPSuckTeamList has no data!");
            return false;
        }
    }

    flag=true;
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            if(bflag[i][j]==false)
                flag=false;
        }
    }

    return flag;
}
//------------------------------------------------------------------------------
// AI(W906-AutoCleanFoundation) 20260721: SetAutoCleanStringGrid itself is NOT
// in the task brief's Part A/HAL-helper list, but 2 in-scope functions
// (RestoreCleanKitData / CheckCleaningCount) call it directly -- translated as
// a required dependency, same treatment the brief gives implicit small
// pass-through helpers.  golden AutoClean.cpp:1171.
//------------------------------------------------------------------------------
void SetAutoCleanStringGrid(int X, int Y, AnsiString Str)                       //Steven 20180524 : Fixed for clean count
{
    fMain->AutoCleanStringGrid->Cells[X][Y]=Str;
    if(Y>0 && Y<4)                                                              //Steven 20200428 : Fixed for auto clean
    {
        if(MOT[MMAutoCleanKit].Tray.Data[X][Y-1]!=NULL_IC)                      //Steven 20210127 : 修正Auto clean新檔時原, 會清掉Count
            MOT[MMAutoCleanKit].Tray.iCleanCount[X][Y-1]=atoi(Str.c_str());
        else
            MOT[MMAutoCleanKit].Tray.iCleanCount[X][Y-1]=0;
    }
}
//------------------------------------------------------------------------------
// AI(W906-AutoCleanFoundation) 20260721: ReadWriteAutoCleanCount itself is NOT
// in the task brief's Part A/HAL-helper list, but RestoreCleanKitData (in
// scope, below) calls it as its final statement -- translated as a required
// dependency. golden AutoClean.cpp:1190.
//------------------------------------------------------------------------------
void ReadWriteAutoCleanCount(bool bRead, bool bReset)                           //Steven 20180524 : Fixed for clean count
{
    AnsiString Str, S="", szDir="";
    int iTemp=0;
    if(bRunAutoClean==false ||                                                  //Ifor 20191024 : add Fix Auto Clean 完成後將的Clean Count錯誤清除
       (bRunAutoClean==true && bReset &&                                        //Steven 20211220 : 修正Clean Count手動歸零的問題
        FLCarryKit.UseSiteNoIC() &&
        BLCarryKit.UseSiteNoIC() &&
        FTestSuck.UseSiteNoIC() &&
        BTestSuck.UseSiteNoIC() &&
        InArmSuck.HasIC()==false))
    {
        if(IniConfig.bE43_1_AutoCleanCountSaveFolder)                           //Steven 20250527 : Save auto clean count to folder
        {
            szDir.sprintf("D:\\HT9045\\IniData\\DefineAutoClean\\AutoCleanCount.Data");
        }
        else
        {
            S=GetLastOpenFN();
            szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
        }

        for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
        {
            for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
            {
                if(bRead)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_FINSH_IC ||   //Jimmychiu 20250103 : fixed for auto clean count to 0 issue
                       MOT[MMAutoCleanKit].Tray.Data[X][Y]==CLEAN_FINISH_IC    ||
                       MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_IC       ||   //Isaac 20180417 (jou) : fix auto clean執行一半治具狀態, clean count會歸零
                       MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_NULL_CLEAN_IC  )
                    {
                        Str.sprintf("iAutoCleanPad_CountTime_%d_%d", Y, X);
                        S=ReadIniData(szDir, "Configuration", Str, AnsiString("0"));
                        SetAutoCleanStringGrid(X, Y+1, S);
                    }
                }
                else
                {
                    Str.sprintf("iAutoCleanPad_CountTime_%d_%d", Y, X);
                    if(bReset)                                                  //Steven 20211220 : 修正Clean Count手動歸零的問題
                        SetAutoCleanStringGrid(X, Y+1, AnsiString("0"));

                    S=fMain->AutoCleanStringGrid->Cells[X][Y+1];
                    iTemp=atoi(S.c_str());                                      //Jimmychiu 20250103 : fixed for auto clean count to 0 issue
                    S=IntToStr(iTemp);
                    WriteIniDataNoLog(szDir, "Configuration", Str, S);
                }
            }
        }
        WriteIniDataNoLog(szDir, "Configuration", "iIndexArmAutoCleanCnt", TestIF_File.iIndexArmAutoCleanCnt);  //Sam 20250820 : AutoClean 在 Index Arm 下做過切換就累加一次
    }
}
//------------------------------------------------------------------------------
bool MoveInArmZ_Shuttle_Pick(eWhichShuttle iSht, int iSelRow)
{
    int iShuttlePick[MAX_ARM_Row][MAX_ARM_Col];
    bool flag[MAX_ARM_Row][MAX_ARM_Col]={{true, true, true, true}, {true, true, true, true}};
    bool bNeedDown=false;

    if(iSht==euShuttle1)
        ptrInSHT=&FLCarryKit;
    else
        ptrInSHT=&BLCarryKit;

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            if(iSht==euShuttle1)
            {
                if((CosFunction.bDeviceMapTest &&
                   W906_FormShowing("fContact", fContact->fShow)==true &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
                    iContactMode==CONTACT_DEVICE_MAP_CHECK) ||
                   fContact->IsRun2DCheck()==true)                              //JerryYang 20250220 : 2DID預鎖順序檢查功能                      //Steven 20221114 :Add for device map function
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle1_Place[i][j] -
                                       InArmOffSet[InOfsInSh1]->GetPlace() +
                                       InArmOffSet[InOfsInSh1]->GetPickUp() +
                                       InArmOffSet[InOfsInSh1+iSelRow-1]->SingleOffSet->dPickUpOffSet[i][j];
                }
                else if(IniConfig.bEnableAutoCleanFunction &&
                        TestIF_File.iAutoClean_Function &&
                        bRunAutoClean &&
                        IniConfig.bE48_ShuttleUse4Offset_Autoclean &&           //20140923 wei : For Shuttle Auto Clean
                        TestIF.bEnableAutoAlignment==false)                     //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle1_Place[i][j] -
                                       InArmOffSet[InOfsInSh1]->GetPlace() +
                                       InArmOffSet[InOfsInSh1_AutoClean]->GetPickUp() +
                                       InArmOffSet[InOfsInSh1_AutoClean+iSelRow-1]->SingleOffSet->dPickUpOffSet[i][j];
                }
                else if(CosFunction.bAutoCleanOffsetUseSingleSetting &&         //Sam 20220720 : AutoClean Offset 值使用 Clean 設定 Offset 來套用
                        TestIF.bEnableAutoAlignment==false)                     //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle1_Place[i][j] +
                                       TestIF.iAutoClean_Shuttle1PickOffset -
                                       InArmOffSet[InOfsInSh1]->GetPlace();
                }
                else
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle1_Place[i][j]+TestIF.iAutoClean_Shuttle1PickOffset;
                }

                if(IniConfig.bE33InOutArmZOffsetSameOne)                        //JerryYang 20210811 : 修正使用E33功能後, shuttle的pick up高度被release offset影響的問題
                {
                    iShuttlePick[i][j]=iShuttlePick[i][j] -
                                       InArmOffSet[InOfsLoader]->GetPlace(i, j) +
                                       InArmOffSet[InOfsLoader]->GetPickUp(i, j);
                }
            }
            else
            {
                if(CosFunction.bDeviceMapTest &&
                   W906_FormShowing("fContact", fContact->fShow)==true &&  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
                   iContactMode==CONTACT_DEVICE_MAP_CHECK)                      //Steven 20221114 :Add for device map function
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle2_Place[i][j] -
                                       InArmOffSet[InOfsInSh2]->GetPlace() +
                                       InArmOffSet[InOfsInSh2]->GetPickUp() +
                                       InArmOffSet[InOfsInSh2+iSelRow-1]->SingleOffSet->dPickUpOffSet[i][j];
                }
                else if(IniConfig.bEnableAutoCleanFunction &&
                        TestIF_File.iAutoClean_Function &&
                        bRunAutoClean &&
                        IniConfig.bE48_ShuttleUse4Offset_Autoclean &&           //20140923 wei : For Shuttle Auto Clean
                        TestIF.bEnableAutoAlignment==false)                     //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle2_Place[i][j] -
                                       InArmOffSet[InOfsInSh2]->GetPlace() +
                                       InArmOffSet[InOfsInSh2_AutoClean]->GetPickUp()+
                                       InArmOffSet[InOfsInSh2_AutoClean+iSelRow-1]->SingleOffSet->dPickUpOffSet[i][j];
                }
                else if(CosFunction.bAutoCleanOffsetUseSingleSetting &&         //Sam 20220720 : AutoClean Offset 值使用 Clean 設定 Offset 來套用
                        TestIF.bEnableAutoAlignment==false)                     //KenHsieh 20220923 : add AOA功能開啟不套影
                {
                    if(IniConfig.bE34InOutArmPitchZOffsetSameOne)
                        iShuttlePick[i][j]=Prod.ZInArm_Shuttle2_Place[i][j] +
                                           TestIF.iAutoClean_Shuttle2PickOffset -
                                           InArmOffSet[InOfsInSh1]->GetPlace();
                    else
                        iShuttlePick[i][j]=Prod.ZInArm_Shuttle2_Place[i][j] +
                                           TestIF.iAutoClean_Shuttle2PickOffset -
                                           InArmOffSet[InOfsInSh2]->GetPlace();
                }
                else
                {
                    iShuttlePick[i][j]=Prod.ZInArm_Shuttle2_Place[i][j] +
                                       TestIF.iAutoClean_Shuttle2PickOffset;    //ChungHung 20141117 : 改為iAutoClean_Shuttle2PickOffset
                }

                if(IniConfig.bE33InOutArmZOffsetSameOne)                        //JerryYang 20210811 : 修正使用E33功能後, shuttle的pick up高度被release offset影響的問題
                {
                    iShuttlePick[i][j]=iShuttlePick[i][j] -
                                       InArmOffSet[InOfsLoader]->GetPlace(i, j) +
                                       InArmOffSet[InOfsLoader]->GetPickUp(i, j);
                }
            }
        }
    }

    if(bUse8Picker)                                                             //Steven 20240221 : 齊平8治具auto clean
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                bNeedDown=(InArmSuck.Item[i][j]==NULL_IC &&                     //Steven 20240306 : 改用NeedSuckCleanPad
                           InArmSuck.iNeedSuck[i][j]!=NULL_IC &&
                           InArmSuck.iNeedSuck[i][j]!=HAS_NULL_CLEAN_IC &&
                           InArmSuck.iNeedSuck[i][j]!=HAS_NULL_IC);             //JerryYang 20250220 : 2DID預鎖順序檢查功能

                flag[i][j]=bNeedDown;
            }
        }
    }
    else
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            bNeedDown=(InArmSuck.Item[1][j]==NULL_IC &&                         //Steven 20240306 : 改用NeedSuckCleanPad
                       InArmSuck.iNeedSuck[1][j]!=NULL_IC &&
                       InArmSuck.iNeedSuck[1][j]!=HAS_NULL_CLEAN_IC &&
                       InArmSuck.iNeedSuck[1][j]!=HAS_NULL_IC);                 //JerryYang 20250220 : 2DID預鎖順序檢查功能

            flag[1][j]=bNeedDown;
            flag[0][j]=false;                                                   //RogerYang 20251119 : bUse8Picker==false的情況下，治具都放在下排
        }
    }

    bool bRet=InArmZMoveDown(flag, iShuttlePick, false);                        //Steven 20251020 : modify for auto clean
    return bRet;
}
//------------------------------------------------------------------------------
bool CheckInSuckICFallDown(int KCode)
{
    int ret;
    AnsiString ErrPart="";                                                      //Steven 20110216 : 合併Alarm
    bool bHasErr=false;                                                         //Steven 20110216 : 合併Alarm
    bool bStatus;

    int iSuckRow=0;                                                             //ChungHung 20141121 add for Use HotPlate AutoClean

    if(IniConfig.bE43AutoCleanUseHotplate)
        iSuckRow=0;
    else
        iSuckRow=1;

    if(LastSet.iRealDummy!=REALLY)
        return false;

    for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                               //ChungHung 20141121 add for Use HotPlate AutoClean
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            #ifndef SOFT_SIMULTE
                if((InArmSuck.Item[i][j]==HAS_CLEAN_IC ||
                    InArmSuck.Item[i][j]==CLEAN_FINISH_IC ||
                    InArmSuck.Item[i][j]==HAS_CLEAN_FINSH_IC) &&
                    InArmSuck.Suck[i][j].GetStatus()==false)                    //ChungHung 20141121 add for Use HotPlate AutoClean
                {
                    ErrPart+=InArmSuck.Suck[i][j].sName;
                    bHasErr=true;
                }
            #else
                if(fMain->chkCleanPadPickErr->Checked==true)
                    bHasErr=true;
            #endif
        }
    }

    if(bHasErr==true)
    {
        ret=ShowErrorMessage("JAM0128", KCode, MInArmX, false, ErrPart);        //JerryYang 20160511 JAM0126->JAM0128,將IC改Clean pad掉料的alarm code分開

        if(ret==K_RETRY)
        {
            for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                       //ChungHung 20141121 add for Use HotPlate AutoClean
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    bStatus=InArmSuck.Suck[i][j].GetStatus();
                    if((InArmSuck.Item[i][j]==HAS_CLEAN_IC ||
                        InArmSuck.Item[i][j]==CLEAN_FINISH_IC ||
                        InArmSuck.Item[i][j]==HAS_CLEAN_FINSH_IC) &&
                        bStatus==false)                                         //ChungHung 20141121 add for Use HotPlate AutoClean
                    {
                        InArmSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);
                    }
                }
            }
            return true;
        }
        else
        {
            for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                       //ChungHung 20141121 add for Use HotPlate AutoClean
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    #ifdef SOFT_SIMULTE
                        if(fMain->chkCleanPadPickErr->Checked==true)
                            bStatus=(i==1 && j==0)?false:true;
                    #else
                        bStatus=InArmSuck.Suck[i][j].GetStatus();
                    #endif

                    if((InArmSuck.Item[i][j]==HAS_CLEAN_IC ||
                        InArmSuck.Item[i][j]==CLEAN_FINISH_IC ||
                        InArmSuck.Item[i][j]==HAS_CLEAN_FINSH_IC) &&
                       bStatus==false)                                          //ChungHung 20141121 add for Use HotPlate AutoClean
                    {
                        InArmSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);
                    }
                }
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
bool TrayHasCleanIC()
{
    bool bFlag=false;

    if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                            //Clean Kit
    {
        for(int Y=0; Y<TestIF_File.iAutoClean_YDivision; Y++)
            for(int X=0; X<TestIF_File.iAutoClean_XDivision; X++)
                if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_IC)
                    return true;

        if(bFlag==false)                                                        //Steven 20160218 : 避免Auto Clean 對Count不同時導致Hang Up
        {
            for(int Y=0; Y<TestIF_File.iAutoClean_YDivision; Y++)
                for(int X=0; X<TestIF_File.iAutoClean_XDivision; X++)
                    if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==CLEAN_FINISH_IC)    //Steven 20211201 : 造成2x4異常資料時原, 會回到原本值
                        MOT[MMAutoCleanKit].SetTraySingleData(X, Y, HAS_CLEAN_IC);
        }
    }
    return bFlag;
}
//------------------------------------------------------------------------------
int TrayHasCleanICCount()                                                       //wei 2015090
{
    int iFlag=0;

    if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit ||
       IniConfig.bE43AutoCleanUseHotplate==true)                                //Clean Kit
    {
        for(int Y=0; Y<TestIF.iAutoClean_YDivision; Y++)
        {
            for(int X=0; X<TestIF.iAutoClean_XDivision; X++)
            {
                if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_IC ||
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==CLEAN_FINISH_IC ||
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_NULL_CLEAN_IC)      //Steven 20210312 : 修正對第二次auto clean時,出現Clean Pad數量錯誤問題
                {
                    iFlag++;
                }
            }
        }
    }
    return iFlag;
}
//------------------------------------------------------------------------------
// AI(W906-AutoCleanFoundation) 20260721: golden SearchCleanNum (AutoClean.cpp:1245,   //AI(W906-FLOW-5) 20260929: SUPERSEDED -- the three drops listed below are translated now: golden :1278 on :1693, golden :1282-1301 (Font->Color + WAR16313) in W906_SearchCleanNum_Tail at the end of this file, called on :1695.  Every drop reason is dead: fShowBinSelect is the real global (cShowBinSelect.cpp:160; forms/fShowBinSelect.h is included at :117), and fMain->pnlCleanCount->Font is the facade TFont fMain->pnlCleanCountFont (forms/fMain.h:373, new'd at forms/fMain.cpp:130).  The text below is history.
// NOT in the task brief's Part A list) -- CheckCleaningCount (in-scope, below)
// calls it, but ONLY inside `if(CosFunction.bCleanCountAlarmByMin)` (a narrow,
// default-false, Gigas-specific gate per its own "Jimmychiu 20260212" comment).
// Translated here as a file-local (`static`) helper: the CORE computation (the
// returned iMin, the minimum per-cell clean count across the whole
// MOT[MMAutoCleanKit] grid) is FAITHFUL and is all CheckCleaningCount's return
// value depends on. Golden's UI-mirror tail is DELIBERATELY DROPPED (does not
// affect the return value, and would need brand-new facade infrastructure
// disproportionate for a function outside this wave's own scope):
//   - fShowBinSelect->ed_AutoCleanCount->Text=... : fShowBinSelect has NO
//     link-visible home in this tree (only a TU-local seam inside csystem.cpp,
//     grepped) -- standing up a whole new facade class for one Text= write
//     that nothing else reads is out of scope here.
//   - fMain->pnlCleanCount->Font->Color=clRed/clNavy : this wave's
//     FormsFacade TfMainPanel stand-in has no ->Font sub-object (golden's
//     nested TFont* shape); Caption is kept (already real, Part B), the
//     Font->Color cosmetic paint is dropped.
//   - the WAR16313 AlarmCount-reached alarm trigger: a real side effect, but
//     one this wave's actual required behavior (CheckCleaningCount's bool
//     return) does not depend on; left for whichever wave gives SearchCleanNum
//     its own full translation.
//------------------------------------------------------------------------------
static void W906_SearchCleanNum_Tail(int iMin);   static int SearchCleanNum()   //AI(W906-FLOW-5) 20260929: forward declaration of the golden :1282-1301 tail (body at the end of this file); SearchCleanNum's own line is unchanged
{
    int iMin=9999;                                                              //Steven 20171211 (Wei) : 修正計算方式
    int iNow=0;
    if(TestIF_File.iAutoClean_Function==0)                                      //Ifor 20191024 : add 避免Auto Clean 未開啟時 Clean Count會歸零
    {
        iMin=atoi(fCleaning->edCleaningCount->Text.c_str());
    }
    else
    {
        for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
        {
            for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
            {
                if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_FINSH_IC ||
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==CLEAN_FINISH_IC    ||
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_IC       ||   //Isaac 20180417 (jou) : fix auto clean執行一半治具狀態, clean count會歸零
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_NULL_CLEAN_IC  )
                {
                    iNow=atoi(fMain->AutoCleanStringGrid->Cells[X][Y+1].c_str());
                    if(iNow!=0 && iNow<iMin)
                    {
                        iMin=iNow;
                    }
                }
            }
        }

        if(iMin==9999)                                                          //Steven 20171211 (Wei) : 修正計算方式
            iMin=0;

        AnsiString sAutoCleanCount=IntToStr(iMin);  fShowBinSelect->ed_AutoCleanCount->Text=sAutoCleanCount;   //Jimmychiu 20250103 : fixed for auto clean count to 0 issue  //Steven 20180524 : Fixed for clean count  //AI(W906-FLOW-5) 20260929: golden :1278 restored (fShowBinSelect = cShowBinSelect.cpp:160; the TEdit is read back by TfShowBinSelect::TimerAutoCleanCountTimer, cShowBinSelect.cpp:2064)
        fCleaning->edCleaningCount->Text=sAutoCleanCount;
        fMain->pnlCleanCount->Caption=AnsiString().sprintf("Cleaned Count %d / %d", iMin, TestIF_File.iAutoClean_AlarmCount);   W906_SearchCleanNum_Tail(iMin);   //Steven 20240731 : add for auto clean count  //AI(W906-FLOW-5) 20260929: golden :1282-1301 restored (pnlCleanCount Font->Color clRed/clNavy + the WAR16313 AlarmCount-reached alarm) -- body at the end of this file
    }
    return iMin;
}
//------------------------------------------------------------------------------
bool CheckCleaningCount()
{
    int iXpos=-1, iYpos=0, iCleanCount=0;
    int iXItem;
    bool bFlag=false;
    if(CosFunction.bCleanCountAlarmByMin)                                       //Jimmychiu 20260212 : Gigas Clear alarms based on minimum usage count
    {
        iCleanCount=SearchCleanNum();                                           //JerryYang 20170801 (wei) iCleanCount為0也要回傳true
        if(iCleanCount<TestIF.iAutoClean_AlarmCount)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    iXItem=MOT[MMAutoCleanKit].Tray.XItem;

    for(int i=0; i<TestIF.iAutoClean_DeveicePices; i++)
    {
        if(TestIF.iTestMode==QualSite2X2 &&                                     //jou 20210712 : 修正 QualSite2X2 Tray pitch 小於2666 hang up
           bCleanKitPitchLess4000==true &&
           TestIF.iAutoClean_DeveicePices==4 &&
           TestIF.iAutoClean_XDivision==8)
        {
            iXpos=i*2;
        }
        else
        {
            iXpos++;
        }

        if(iXpos>=iXItem)
        {
            iXpos=0;
            iYpos++;
        }

        if((TestIF_File.iTestMode==QualSite1X4 ||                               //Steven 20220218 : 修正1x4 auto clean for 不同X-Pitch
            TestIF_File.iTestMode==_8Site2X4N) &&
            TestIF.dAutoClean_XPitch<1333 &&                                    //Steven 20240305 : 修正Auto Clean Pitch判斷
           iInArmType!=e9045_1x4_1_Ac)
        {
            if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]!=NULL_IC)            //Steven 20210127 : 修正auto clean count到達不會alarm
            {
                iCleanCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                if(iCleanCount<0)                                               //Steven 20210127 : 修正auto clean count到達不會alarm
                    iCleanCount=0;
                if(iCleanCount<TestIF.iAutoClean_AlarmCount)                    //JerryYang 20170801 (wei) iCleanCount為0也要回傳true
                {
                    bFlag=true;
                    break;
                }
            }
        }
        else if((TestIF.iTestMode!=DualSite     &&
                 TestIF.iTestMode!=QualSite2X2  &&
                 TestIF.iTestMode!=TriSite1X3   &&
                 TestIF.iTestMode!=_6Site2X3N)  &&                              //Steven 20220425 : 2X3NN Mode
                (iAutoCleanUseXPitch==2 || iAutoCleanUseXPitch==4) &&
                TestIF_File.iAutoClean_DeveicePices<=4 &&                       //JerryYang 20200701 修正auto clean次數到達時沒有發出alarm的問題
                TestIF_File.iAutoClean_XDivision>4)                             //Steven 20240305 : 修正Auto Clean Pitch判斷
        {
            if(iXpos+iYpos*MOT[MMAutoCleanKit].Tray.XItem<TestIF.iAutoClean_DeveicePices)
            {
                if(InArmSuck.iModeX==9 &&
                   TestIF_File.iAutoClean_DeveicePices==4 &&
                   iCloseSiteStep_2x8==3)                                       //KevinCheng 20260126 : 兩顆治具狀態下增加特殊的Site判斷
                {
                    iCleanCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                    if(iCleanCount<0)
                        iCleanCount=0;
                    if(iCleanCount<TestIF.iAutoClean_AlarmCount)
                    {
                        bFlag=true;
                        break;
                    }
                }
                else if(MOT[MMAutoCleanKit].Tray.Data[iXpos*2][iYpos]==HAS_CLEAN_IC ||
                        MOT[MMAutoCleanKit].Tray.Data[iXpos*2][iYpos]==CLEAN_FINISH_IC)
                {
                    iCleanCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos*2][iYpos+1].c_str());
                    if(iCleanCount<0)                                           //Steven 20210127 : 修正auto clean count到達不會alarm
                        iCleanCount=0;
                    if(iCleanCount<TestIF.iAutoClean_AlarmCount)                //JerryYang 20170801 (wei) iCleanCount為0也要回傳true
                    {
                        bFlag=true;
                        break;
                    }
                }
            }
        }
        else
        {
            if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]!=NULL_IC)            //Steven 20210127 : 修正auto clean count到達不會alarm
            {
                iCleanCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                if(iCleanCount<0)                                               //Steven 20210127 : 修正auto clean count到達不會alarm
                    iCleanCount=0;
                if(iCleanCount<TestIF.iAutoClean_AlarmCount)                    //JerryYang 20170801 (wei) iCleanCount為0也要回傳true
                {
                    bFlag=true;
                    break;
                }
            }
        }
    }

    return bFlag;
}
//------------------------------------------------------------------------------
bool CheckInArmSuckFromCleanKitICFallDown(bool bRetry)
{
    bool bIsSuckICFallDown[MAX_SOCKET_ROW][MAX_SOCKET_COL]={false}, bHasErr=false;
    AnsiString ErrPart="";
    int ret=0;

    if(LastSet.iRealDummy==DUMMY)
        return false;

    if(LastSet.iRealDummy==REALLY)
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                if(InArmSuck.Suck[i][j].Enable       &&                         //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                   InArmSuck.Suck[i][j].SenUsing!="" &&
                   InArmSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                   InArmSuck.Item[i][j]!=NULL_IC)                               //wei 20160130
                {
                    if(InArmSuck.Suck[i][j].GetStatus()==false)
                    {
                        bIsSuckICFallDown[i][j]=true;
                        ErrPart+=InArmSuck.Suck[i][j].sName;
                        bHasErr=true;
                    }
                    else
                    {
                        bIsSuckICFallDown[i][j]=false;
                    }
                }
                else
                {
                    bIsSuckICFallDown[i][j]=false;
                }
            }
        }
    }

    if(ret)                                                                     //Debug用
    {
        bIsSuckICFallDown[1][0]=true;
        ErrPart+=InArmSuck.Suck[1][0].sName;
        bHasErr=true;
    }

    if((bUse8Picker ||
        IniConfig.bE43AutoCleanUseHotplate) &&
       bRetry)                                                                  //JerryYang 20160503 第二次吸治具,又要重複第一次選retry
    {
        if(bIsSuckICFallDown[0][0] || bIsSuckICFallDown[0][1] || bIsSuckICFallDown[0][2]|| bIsSuckICFallDown[0][3])
        {
            if(InArmSuck.Item[1][0] && InArmSuck.Item[1][1] && InArmSuck.Item[1][2] && InArmSuck.Item[1][3])
            {
                for(int i=0; i<MAX_ARM_Col ; i++)
                    bIsSuckICFallDown[0][i]=false;

                if(bIsSuckICFallDown[1][0]==false && bIsSuckICFallDown[1][1]==false && bIsSuckICFallDown[1][2]==false && bIsSuckICFallDown[1][3]==false)
                {
                    ErrPart="";
                    bHasErr=false;
                }
            }
        }
    }

    if(bHasErr)
    {
        if(bRetry)
            ret=ShowErrorMessage("JAM0128", K_RETRY, MInArmX, false, ErrPart);  //JerryYang 20160511 JAM0126->JAM0128,將IC改Clean pad掉料的alarm code分開
        else
            ret=ShowErrorMessage("JAM0128", K_SKIP, MInArmX, false, ErrPart);   //JerryYang 20160511 JAM0126->JAM0128,將IC改Clean pad掉料的alarm code分開

        if(SoftStop)
            fMain->Pause("CheckInArmSuckFromCleanKitICFallDown");

        if(bRetry)
        {
            if(ret==K_RETRY)
            {
                for(int i=0; i<2; i++)                                          //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bIsSuckICFallDown[i][j])
                        {
                            MOT[MMAutoCleanKit].SetTraySingleData(InArmSuck.iAutoCleanRecX[i][j], InArmSuck.iAutoCleanRecY[i][j], HAS_CLEAN_IC);    //JerryYang 20160210 修正auto clean 使用hot plate hang up問題
                            InArmSuck.SetItemData(i, j, NULL_IC);
                            bInArmSuckActive[i][j]=true;                        //JerryYang 20160404 [i][j]-->[1][j],修正Drop error後retry後沒有補治具問題
                            iAutoCleanPickPlateY=InArmSuck.iAutoCleanRecY[i][j]; //JerryYang 20160428 修正重複後hang up,忘記紀錄回原本的 Kit Y
                        }
                    }
                }
                return true;
            }
        }
        else
        {
            if(ret==K_SKIP)
            {
                for(int i=0; i<2; i++)                                          //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                {
                    for(int j=0; j<MAX_ARM_Col; j++)
                    {
                        if(bIsSuckICFallDown[i][j])
                        {
                            InArmSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);     //wei 20160130
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void DoPlaceToKitSwapData(bool bPick, int iIC_Type, int iSuckRow, int iSuckCol, int iKitRow, int iKitCol)     //Steven 20170109 : 將資料到Kit資料交換改成Function
{
    if(bPick==bAutoPick)
    {
        int iCleanListType=-1;
        if(iIC_Type==HAS_CLEAN_IC)
        {
            iCleanListType=iIC_Type;
        }
        InspectInArmPosition(MMAutoCleanKit, iSuckRow, iSuckCol, iKitRow, iKitCol, true);
        PlaceToCleanList->SetArrPlateXY(iSuckRow, iSuckCol, 0, iKitRow, iKitCol, iCleanListType);      //Jimmychiu 20230417 : Record the position after placing the IC
        MOT[MMAutoCleanKit].SetTraySingleData(iKitCol, iKitRow, iIC_Type);
        AnsiString sTime=AnsiString().sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
        CleanKitRecord[iKitRow][iKitCol]=sTime;                         //Sam 20230619 : 新增 Clean治具時間 Log
        bCleanKitSuckDuplicateErr[iSuckRow][iSuckCol] =false;
        InArmSuck.SetItemData(iSuckRow, iSuckCol, iIC_Type);
        InArmSuck.iAutoCleanRecX[iSuckRow][iSuckCol]  =iKitCol;
        InArmSuck.iAutoCleanRecY[iSuckRow][iSuckCol]  =iKitRow;
        InArmSuck.PordRec[iSuckRow][iSuckCol].AddPickCleanPad(iSuckRow, iSuckCol, iKitCol, iKitRow, iIC_Type); //Sam 20230616 : Add Auto Clean Record
    }
}
//==============================================================================
bool PickFromCleanKit(int iRowKit)
{
    bool flag1=true;
    int iSuckRow=0, iSuckCol=0, iKitRow=0, iKitCol=0;

    AnsiString sTime="";                                                        //Sam 20230619 : 新增 Clean治具時間 Log
    GetTimeInfo();

    if(bUse8Picker)                                                             //Steven 20201014 : 齊平8治具auto clean
    {
        if(iRowKit==1 || iRowKit==3)
            iSuckRow=0;
        else if(iRowKit==2 || iRowKit==4)
            iSuckRow=1;
    }
    else
    {
        iSuckRow=1;
    }

    for(int j=0; j<4; j++)
    {
        iSuckCol=GetAutoCleanPickStep(j);                                       //Steven 20240918 : fixed for auto clean
        if(iSuckCol==-1)
            continue;
        if(USE_PICKER_COUNT==ep1Picker && j>=1)
            continue;

        iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(j-iAutoCleanStart);
        iKitRow=iAutoCleanPickPlateY;

        if(iKitCol<0 || iKitRow<0 ||
           iKitCol>=50 || iKitRow>=50)
        {
            sTime="";
            continue;                                                           //Steven 20240729 : fixed for auto clean pick 2 times from kit
        }

        if(InArmSuck.Suck[iSuckRow][iSuckCol].Error==true)                      //Steven 20220210 : fixed for Auto Clean吸嘴異常
        {
        }
        else if(bInArmSuckActive[iSuckRow][iSuckCol] &&
                InArmSuck.Item[iSuckRow][iSuckCol]==NULL_IC)                    //ChungHung 20131120 AutoClean use Hotplate1
        {
            if(InArmSuck.Suck[iSuckRow][iSuckCol].Suck())                       //ChungHung 20131120 AutoClean use Hotplate1
            {
                if(Special_2X6_Tray_XItem7() && (iRowKit==3 || iRowKit==4))     //Sam 20250712 : 新增特殊流程 2X6 AutoClean Tray XItem=7
                {
                    if(iKitCol>=iAutoCleanUseXPitch)
                    {
                        iKitCol=iKitCol-iAutoCleanUseXPitch;
                    }
                }
                InspectInArmPosition(MMAutoCleanKit, iSuckRow, iSuckCol, iKitRow, iKitCol, true);
                PlaceToCleanList->SetArrPlateXY(iSuckRow, iSuckCol, 0, iKitRow, iKitCol, HAS_CLEAN_IC);      //Jimmychiu 20230417 : Record the position after placing the IC
                bInArmSuckActive[iSuckRow][iSuckCol] =false;
                MOT[MMAutoCleanKit].SetTraySingleData(iKitCol, iKitRow, HAS_NULL_CLEAN_IC);
                sTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                CleanKitRecord[iKitRow][iKitCol]=sTime;                         //Sam 20230619 : 新增 Clean治具時間 Log
                bCleanKitSuckDuplicateErr[iSuckRow][iSuckCol] =false;
                InArmSuck.SetItemData(iSuckRow, iSuckCol, HAS_CLEAN_IC);
                InArmSuck.iAutoCleanRecX[iSuckRow][iSuckCol]  =iKitCol;
                InArmSuck.iAutoCleanRecY[iSuckRow][iSuckCol]  =iKitRow;
                InArmSuck.PordRec[iSuckRow][iSuckCol].AddPickCleanPad(iSuckRow, iSuckCol, iKitRow, iKitCol, HAS_CLEAN_IC);  //Sam 20230616 : Add Auto Clean Record
            }
            else if(InArmSuck.Suck[iSuckRow][iSuckCol].Error==false)
            {
                flag1=false;
            }
        }
        else if(bInArmSuckActive[iSuckRow][iSuckCol]==false &&
                InArmSuck.Item[iSuckRow][iSuckCol]==NULL_IC)
        {
            if(MOT[MMAutoCleanKit].Tray.Data[iKitCol][iKitRow]==HAS_CLEAN_IC &&
               (iKitCol<MOT[MMAutoCleanKit].Tray.XItem))
            {
                if(Special_2X6_Tray_XItem7() && (iRowKit==3 || iRowKit==4))     //Sam 20250712 : 新增特殊流程 2X6 AutoClean Tray XItem=7
                {
                    iKitCol=iKitCol+iAutoCleanUseXPitch*3;
                }
                PlaceToCleanList->SetArrPlateXY(iSuckRow, iSuckCol, 0, iKitRow, iKitCol, -1);      //Jimmychiu 20230417 : Record the position after placing the IC
                MOT[MMAutoCleanKit].SetTraySingleData(iKitCol, iKitRow, HAS_NULL_CLEAN_IC);
                sTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                CleanKitRecord[iKitRow][iKitCol]=sTime;                         //Sam 20230619 : 新增 Clean治具時間 Log
                bCleanKitSuckDuplicateErr[iSuckRow][iSuckCol] =false;
                InArmSuck.SetItemData(iSuckRow, iSuckCol, HAS_NULL_CLEAN_IC);
                InArmSuck.iAutoCleanRecX[iSuckRow][iSuckCol]  =iKitCol;
                InArmSuck.iAutoCleanRecY[iSuckRow][iSuckCol]  =iKitRow;
                InArmSuck.PordRec[iSuckRow][iSuckCol].AddPickCleanPad(iSuckRow, iSuckCol, iKitCol, iKitRow, HAS_NULL_CLEAN_IC); //Sam 20230616 : Add Auto Clean Record
            }
        }
    }
    return flag1;
}
//------------------------------------------------------------------------------
bool PlaceToCleanKit()
{
    bool flag=true;
    int iSuckRow=0, iSuckCol=0, iKitRow=0, iKitCol=0;

    AnsiString sTime="";
    int iP=0;
    int iPlateR[MAX_ARM_Row][MAX_ARM_Col];                                      //Jimmychiu 20230417 : Record the position after placing the IC
    int iPlateC[MAX_ARM_Row][MAX_ARM_Col];
    bool bSuck[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(iPlateR, sizeof(iPlateR));
    ZeroMemory(iPlateC, sizeof(iPlateC));
    ZeroMemory(bSuck, sizeof(bSuck));
    GetTimeInfo();

    if(PlaceToCleanList->GetHPFirstTeam(&iP, iPlateR, iPlateC, bSuck))
    {
        if(PlaceToCleanList->GetHPFirstTeamMotUse(bCleanZMotToPlace)==false)    //RogerYang 20250624 Add HotPlate ErrMessage
        {
            ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamMotUse", 0));
            return false;
        }
        PlaceToCleanList->GetHPFirstTeamSuckUse(bCleanZSuckToPlace);

        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(bCleanZSuckToPlace[i][j]==true)
                {
                    iSuckRow=i;
                    iSuckCol=j;
                    iKitRow =InArmSuck.iAutoCleanRecY[iSuckRow][iSuckCol];
                    iKitCol =InArmSuck.iAutoCleanRecX[iSuckRow][iSuckCol];

                    if(iKitCol<0   || iKitRow<0 ||
                       iKitCol>=50 || iKitRow>=50)
                    {
                        ShowMyMessage("CleanKitRecord error1", "CleanKitRecord 錯誤3");
                        InArmSuck.SetItemData(i, j, NULL_IC);
                    }
                    else if((bCleanZMotToPlace[i][j]==false &&                  //資料要交換, 治具不用下去, 就是HAS_NULL_CLEAN_IC
                             InArmSuck.Item[i][j]!=NULL_IC) ||
                            (bCleanZMotToPlace[i][j]==true &&
                             InArmSuck.Item[i][j]!=NULL_IC &&
                             InArmSuck.Suck[i][j].Destroy()))
                    {
                        InArmSuck.SetItemData(i, j, NULL_IC);
                        InArmSuck.PordRec[i][j].AddPlaceCleanPad(i, j, InArmSuck.iAutoCleanRecY[i][j], InArmSuck.iAutoCleanRecX[i][j]);    //Sam 20230616 : Add Auto Clean Record
                        InArmSuck.PordRec[1][j].SaveRecordCleanPad();
                        MOT[MMAutoCleanKit].SetTraySingleData(InArmSuck.iAutoCleanRecX[i][j], InArmSuck.iAutoCleanRecY[i][j], CLEAN_FINISH_IC);   //JerryYang 20160210 修正auto clean 使用hot plate hang up問題
                        sTime.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
                        CleanKitRecord[InArmSuck.iAutoCleanRecY[i][j]][InArmSuck.iAutoCleanRecX[i][j]]=sTime;   //Sam 20230619 : 新增 Clean治具時間 Log
                        bInArmSuckActive[i][j]=false;
                        bInArmCheckDestroyACT[i][j]=true;                       //Sam 20240124 : AutoClean 時回吹判斷
                    }
                    else
                    {
                        if(InArmSuck.Item[i][j]!=NULL_IC &&
                           InArmSuck.Suck[1][j].Error==false)
                            flag=false;
                    }
                }
            }
        }
    }

    if(flag==false)
        return false;

    for(int j=0; j<InArmSuck.iMaxCol; j++)
    {
        iSuckCol=j;
        if(bInArmSuckActive[iSuckRow][iSuckCol] &&
           InArmSuck.Item[iSuckRow][iSuckCol]!=NULL_IC)
            return false;
    }
    return flag;
}
//==============================================================================
//  搜尋XY到Clean Kit
//==============================================================================
bool MoveInArmXYPickCleanKit(int iPick, int iShuttleRow, eWhichShuttle iSht)    //ChungHung 20140709 add iSht for SCK CloseSiteByArm Autoclean
{
    int iXPos       =0;
    int iYPos       =0;
    int iMovePitchX =GetXPitchOfCleanKit();
    int iMovePitchY =GetYPitchOfCleanKit();
    int iOffsetPos  =InOfsAutoClean;
    int iSuckRow=0, iSuckCol=0;
    int iKitRow=0, iKitCol=0;
    double dMovePitchX=iMovePitchX;

    if(USE_PICKER_COUNT==ep1Picker)
    {
        dMovePitchX=0;
    }
    else if(USE_PICKER_COUNT==ep16Picker)                                            //齊平為第四種治具
    {
        dMovePitchX=double(iMovePitchX)/7.0;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb ||                               //齊平為第二種治具 //Steven for HT7080
            USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                       //Ztex 2024.02.24 Add HT-1132
    {
        dMovePitchX=double(iMovePitchX)/3.0;
    }
    else                                                                        //齊平為第三種治具
    {
        dMovePitchX=double(iMovePitchX)/3.0;
    }

    ZeroMemory(bZFlgToCleanKit, sizeof(bZFlgToCleanKit));
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iZPosToCleanKit[i][j]=ZSafePos;
        }
    }

    {
        if(IniConfig.bE43AutoCleanUseHotplate)
        {
            iOffsetPos=InOfsHP1;
        }
        else
        {
            iOffsetPos=InOfsAutoClean;
        }
    }

    if(iPick==bAutoPick)
    {
        if(iPickerOrder==1 || iPickerOrder==3)
            iSuckRow=1;
        else
            iSuckRow=0;

        iKitRow=iAutoCleanPickPlateY;                                           //Steven 20240701 : 修正Auto Clean Kit Row異常
        iKitCol=iAutoCleanPickPlateX;

        if(Special_2X6_Tray_XItem7() && (iShuttleRow==3 || iShuttleRow==4))     //Sam 20250712 : 新增特殊流程 2X6 AutoClean Tray XItem=7
        {
            iKitCol=iKitCol-iAutoCleanUseXPitch;
        }
        iSuckCol=GetAutoCleanPickStep(iAutoCleanStart);                         //Steven 20240918 : fixed for auto clean
        if(iSuckCol==-1)
            return true;
    }
    else if(iPick==bAutoPlace)
    {
        if(PlaceToCleanList->GetHPFirstTeamMotUse(bCleanZMotToPlace)==false)    //RogerYang 20250624 Add HotPlate ErrMessage
        {
            ShowMyMessage(AnsiString().sprintf("%s %s Task=%d", __FUNC__, "No GetHPFirstTeamMotUse", 0));
            return false;
        }
        PlaceToCleanList->GetHPFirstTeamSuckUse(bCleanZSuckToPlace);

        iSuckCol=-1;
        for(int j=0; j<InArmSuck.iMotCol; j++)                                  //確認第一排最左邊
        {
            if(iSuckCol==-1 &&
               bCleanZSuckToPlace[0][j]==true)
            {
                iSuckRow=0;
                iSuckCol=j;
            }
        }

        if(iSuckCol==-1)                                                        //如果確認第一排最左邊沒有了, 就找第二排
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                if(iSuckCol==-1 &&
                   bCleanZSuckToPlace[1][j]==true)
                {
                    iSuckRow=1;
                    iSuckCol=j;
                }
            }
        }

        if(iSuckCol==-1)
        {
            return true;
        }
        iKitRow=InArmSuck.iAutoCleanRecY[iSuckRow][iSuckCol];                   //Steven 20240701 : 修正Auto Clean Kit Row異常
        iKitCol=InArmSuck.iAutoCleanRecX[iSuckRow][iSuckCol];

        if(Special_2X6_Tray_XItem7())                                           //Sam 20250712 : 新增特殊流程 2X6 AutoClean Tray XItem=7
        {
            if(iKitCol>0)
                iKitCol=iKitCol-iAutoCleanUseXPitch*4;
        }
    }

    int iYVariable  =GetInArmPitchY_9045(iMovePitchY, iOffsetPos);
    for(int i=0; i<X_PITCH_COUNT; i++)
        iInXPToSht[i]=GetInArmPitchX_9045(iMovePitchX, i, iOffsetPos);

    iXPos=Prod.XInArm_AutoClean_Pick[iInArmYBase][iInArmXBase]+
          dMovePitchX*(0+iInArmXBase-iSuckCol)+                                //A排使用, 用哪個治具, 就是使用哪個治具的距離
          TestIF_File.dAutoClean_XPitch*iKitCol+                                //使用的治具移動到要治具的位置
          HotplatlXOffset;                                                      //kevin 20150720 kit or TRAY autoclean X offset

    if(USE_IN_Y_IS_AUTO_PITCH==true)                                            //JerryYang 20251218 : IN/OUT ARM支援不同模式
    {
        if(iSuckRow==0)
            iYPos=Prod.YInArm_AutoClean_Pick[iInArmYBase][iInArmXBase]-iKitRow*TestIF_File.dAutoClean_YPitch-iMovePitchY;  //JerryYang 20240821 : fix auto clean點位錯誤
        else
            iYPos=Prod.YInArm_AutoClean_Pick[iInArmYBase][iInArmXBase]-iKitRow*TestIF_File.dAutoClean_YPitch;              //JerryYang 20240821 : fix auto clean點位錯誤
    }
    else
    {
        if(bUse8Picker==false)                                                  //Steven 20201014 : 齊平8治具auto clean
            iYPos=Prod.YInArm_AutoClean_Pick[iInArmYBase][iInArmXBase]-iKitRow*TestIF_File.dAutoClean_YPitch+iMovePitchY;
        else if(iSuckRow==1)
            iYPos=Prod.YInArm_AutoClean_Pick[iInArmYBase][iInArmXBase]-iKitRow*TestIF_File.dAutoClean_YPitch+iMovePitchY;
        else
            iYPos=Prod.YInArm_AutoClean_Pick[iInArmYBase][iInArmXBase]-iKitRow*TestIF_File.dAutoClean_YPitch;
    }

    iYPos+=HotplatlYOffset;                                                     //kevin 20150720 kit or TRAY autoclean Y offset

    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF_File.bEnableAutoAlignment)      //KenHsieh 20211214 : AOA add AutoClean
        CheckInArmXYScaleByAutoTeach(iXPos, iYPos, AOA_AutoClean);
    iInXPos=iXPos;
    iInYPos=iYPos;

    if(InArmContinuousMove_9045(iXPos, iYPos, iInXPToSht, iYVariable, bZFlgToCleanKit, iZPosToCleanKit, false))
    {
        return true;
    }

    return false;
}
//------------------------------------------------------------------------------
void SetShuttleIcForSpecialMode(eWhichShuttle iSht, int iType)                  //Steven 20221006 : 針對特殊模式可能Clean Pad放置
{
    if(iSht==euShuttle1)
        ptrInSHT=&FLCarryKit;
    else
        ptrInSHT=&BLCarryKit;

    if(iCloseSiteModeFor2x8==e2x8Run2x2_13 ||
       iCloseSiteModeFor2x8==e2x8Run2x2_14)
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(j!=0+iCloseSiteStep_2x8 &&
                   j!=2+iCloseSiteStep_2x8)
                {
                    ptrInSHT->SetItemData(i, j, iType);
                }
            }
        }
    }
    else if(iCloseSiteModeFor2x8==e2x8CloseEven ||
            iCloseSiteModeFor2x8==e2x8CloseEven1By1)
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(j==1 || j==3 || j==5 || j==7)
                {
                    ptrInSHT->SetItemData(i, j, iType);
                }
            }
        }
    }
    else if(iCloseSiteModeFor2x8==e2x8CloseOdd ||
            iCloseSiteModeFor2x8==e2x8CloseOdd1By1)
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(j==0 || j==2 || j==4 || j==6)
                {
                    ptrInSHT->SetItemData(i, j, iType);
                }
            }
        }
    }
    else if(iCloseSiteModeFor2x8==e2x8Run2x4Standard)                           //Steven 20240801 : fixed for auto clean.
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(j<iCloseSiteStep_2x8 ||
                   j>=iCloseSiteStep_2x8+4)
                {
                    ptrInSHT->SetItemData(i, j, iType);
                }
            }
        }
    }
    else if(iCloseSiteModeFor2x6==e2x6Run2x4)                                   //Steven 20240801 : fixed for auto clean.
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(j<iCloseSiteStep_2x6 ||
                   j>=iCloseSiteStep_2x6+4)
                {
                    ptrInSHT->SetItemData(i, j, iType);
                }
            }
        }
    }
    else if(iCloseSiteModeFor2x6==e2x6CloseCenter2x4)                           //Steven 20241025 : fixed for 2x6 close center site auto clean
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(Prod.bInSuckUse[iSht][i][j]==false)
                {
                    int iSiteCol=GetShuttleCol(i, j);
                    ptrInSHT->SetItemData(i, iSiteCol, iType);
                }
            }
        }
    }
    else if(iCloseSiteModeFor2x8>e2x8OneByOne ||
            iCloseSiteModeFor2x6>e2x6OneByOne)
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(Prod.bInSuckUse[iSht][i][j]==false)
                {
                    ptrInSHT->SetItemData(i, j, iType);
                }
            }
        }
    }
    else if(iType==HAS_NULL_CLEAN_IC)
    {
        if(fCleaning->b1x2SiteAbClosePutDummy)                                  //Steven 20180903 : 1x2 close Ab Auto Clean //Steven 20190509 : Fixed
        {
            if(bUse8Picker)
            {
                if(iInArmType==e9045_1x2_2_14)
                {
                    if(InArmSuck.Item[0][3]==NULL_IC)
                        InArmSuck.SetItemData(0, 3, iType);
                }
                else if(InArmSuck.Item[0][2]==NULL_IC)
                {
                    InArmSuck.SetItemData(0, 2, iType);
                }
            }
            else
            {
                if(iInArmType==e9045_1x2_2_14)
                {
                    if(InArmSuck.Item[1][3]==NULL_IC)
                        InArmSuck.SetItemData(1, 3, iType);
                }
                else if(InArmSuck.Item[1][2]==NULL_IC)
                {
                    InArmSuck.SetItemData(1, 2, iType);
                }
            }
        }
        else if(iInArmType==e9045_1x4_1_Ac)
        {
            ptrInSHT->SetItemData(0, 0, iType);
            ptrInSHT->SetItemData(0, 1, iType);
            ptrInSHT->SetItemData(0, 3, iType);
        }
    }
    else
    {
        for(int i=0; i<MAX_Index_Row; i++)
        {
            for(int j=0; j<MAX_Index_Col; j++)
            {
                if(ptrInSHT->Item[i][j]==HAS_NULL_CLEAN_IC &&
                   (ptrInSHT->iAutoCleanRecX[i][j]==-1 ||
                    ptrInSHT->iAutoCleanRecY[i][j]==-1))                        //Steven 20211221 : 清除因異常留Auto Clean紀錄
                {
                    ptrInSHT->SetItemData(i, j, NULL_IC);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
bool RestoreCleanKitData()                                                      //ChungHung 20130628 add 回復已被InArm吸走的CleanKit上的IC計算  請勿刪除
{
    int iXpos, iYpos;
    int iCount=0;

    if(InArmSuck.HasType(HAS_CLEAN_IC) || InArmSuck.HasType(HAS_NULL_CLEAN_IC)) //KevinCheng 20251209 : 增加判斷避免InArm資料只有NULL_CLEAN_IC時不會被清除
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                iXpos=InArmSuck.iAutoCleanRecX[i][j];
                iYpos=InArmSuck.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        iCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                        iCount--;
                        SetAutoCleanStringGrid(iXpos, iYpos+1, AnsiString(iCount));
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, HAS_CLEAN_IC);
                    }
                }
            }
        }
    }

    if(InArmSuck.HasType(CLEAN_FINISH_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<4; j++)
            {
                iXpos=InArmSuck.iAutoCleanRecX[i][j];
                iYpos=InArmSuck.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, CLEAN_FINISH_IC);
                    }
                }
            }
        }
    }

    if(FLCarryKit.HasType(HAS_CLEAN_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=FLCarryKit.iAutoCleanRecX[i][j];
                iYpos=FLCarryKit.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        iCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                        iCount--;
                        SetAutoCleanStringGrid(iXpos, iYpos+1, AnsiString(iCount));
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, HAS_CLEAN_IC);
                    }
                }
            }
        }
    }

    if(FLCarryKit.HasType(CLEAN_FINISH_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=FLCarryKit.iAutoCleanRecX[i][j];
                iYpos=FLCarryKit.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, CLEAN_FINISH_IC);
                    }
                }
            }
        }
    }

    if(BLCarryKit.HasType(HAS_CLEAN_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=BLCarryKit.iAutoCleanRecX[i][j];
                iYpos=BLCarryKit.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        iCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                        iCount--;
                        SetAutoCleanStringGrid(iXpos, iYpos+1, AnsiString(iCount));
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, HAS_CLEAN_IC);
                    }
                }
            }
        }
    }

    if(BLCarryKit.HasType(CLEAN_FINISH_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=BLCarryKit.iAutoCleanRecX[i][j];
                iYpos=BLCarryKit.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, CLEAN_FINISH_IC);
                    }
                }
            }
        }
    }

    if(BTestSuck.HasType(HAS_CLEAN_IC))                                         //JerryYang 20160825 回復被index arm吸走的CleanKit上CleanKit上的IC計算
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=BTestSuck.iAutoCleanRecX[i][j];
                iYpos=BTestSuck.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        iCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                        iCount--;
                        SetAutoCleanStringGrid(iXpos, iYpos+1, AnsiString(iCount));
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, HAS_CLEAN_IC);
                    }
                }
            }
        }
    }

    if(FTestSuck.HasType(HAS_CLEAN_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=FTestSuck.iAutoCleanRecX[i][j];
                iYpos=FTestSuck.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        iCount=atoi(fMain->AutoCleanStringGrid->Cells[iXpos][iYpos+1].c_str());
                        iCount--;
                        SetAutoCleanStringGrid(iXpos, iYpos+1, AnsiString(iCount));
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, HAS_CLEAN_IC);
                    }
                }
            }
        }
    }

    if(BTestSuck.HasType(CLEAN_FINISH_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=BTestSuck.iAutoCleanRecX[i][j];
                iYpos=BTestSuck.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, CLEAN_FINISH_IC);
                    }
                }
            }
        }
    }

    if(FTestSuck.HasType(CLEAN_FINISH_IC))
    {
        for(int i=0; i<2; i++)
        {
            for(int j=0; j<8; j++)
            {
                iXpos=FTestSuck.iAutoCleanRecX[i][j];
                iYpos=FTestSuck.iAutoCleanRecY[i][j];
                if(iXpos>=0 && iYpos>=0)
                {
                    if(MOT[MMAutoCleanKit].Tray.Data[iXpos][iYpos]==HAS_NULL_CLEAN_IC)
                    {
                        MOT[MMAutoCleanKit].SetTraySingleData(iXpos, iYpos, CLEAN_FINISH_IC);
                    }
                }
            }
        }
    }
    ReadWriteAutoCleanCount(false);                                             //Steven 20180524 : Fixed for clean count
    return true;
}
//------------------------------------------------------------------------------
bool SearchCleanKitRowCol(int& iKRow,int& iKCol)
{
    for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
    {
        for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
        {
            if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_IC)
            {
                iKCol=X;
                iKRow=Y;
                return true;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void SearchCleanKitRowCol(eWhichShuttle iSht)
{
    if((TestIF.iShuttleMode==1 &&
        TestIF.iShuttle_Sel==1 &&
        TestIF.iAutoClean_SelectArm==1) ||                                      //kevin 20170126 (Steven) 選ARM 使用ARM 2
       (IniConfig.bA09_ByArmCloseSite==1 &&
        TestIF.iAutoClean_SelectArm==1) ||                                      //Isaac 20170601 (wei) add
       (CosFunction.bAutoCleanAutoSelIndexArm==true &&
        TestIF.bCleanIndexOtherArm==false &&
        TestIF_File.iShuttleMode==1 &&
        TestIF_File.iShuttle_Sel==1))                                           //JerryYang 20171017 (wei) 修正auto clean只開ARM2會發生hang up
    {
        iSht=euShuttle2;
    }

    bool bFlag[8];

    int iKitRow=0, iKitCol=0;
    int iRealRow=0;
    int iSuckCol=0, iShtRow=0, iShtCol=0;

    ZeroMemory(bFlag, sizeof(bFlag));
    DoInArm_SuckerMap();                                                        //jou 2013-11-06 開放site 治具使用異常修正

    iRealRow=0;
    iSuckCol=0;

    GetNowInShuttleRowCol_All_1Picker(iSht, &iShtRow, &iShtCol, bAutoPlace);

    if(Prod.bInSuckUse[iSht][iShtRow][iSuckCol]==true)
    {
        bFlag[iSuckCol]=(Prod.fInArmSuck4x8[iSht][iShtRow][iShtCol] &&          //Steven 20241008 : fixed [iSuckCol+iKitStep]
                         InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC);
    }

    if(InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC)
    {
        iAutoCleanStart=0;
    }

    int iKRow=0,iKCol=0;
    if(SearchCleanKitRowCol(iKRow,iKCol))
    {
        iAutoCleanPickPlateX=iKCol;
        iAutoCleanPickPlateY=iKRow;
        iSuckCol=0;                                                             //Steven 20240918 : fixed for auto clean
        iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(iSuckCol-iAutoCleanStart);
        iKitRow=iAutoCleanPickPlateY;

        if(MOT[MMAutoCleanKit].Tray.Data[iKitCol][iKitRow]==HAS_CLEAN_IC &&
           iKitCol<MOT[MMAutoCleanKit].Tray.XItem)
        {
            bInArmSuckActive[iRealRow][iSuckCol]=bFlag[iSuckCol];
        }
        else
        {
            bInArmSuckActive[iRealRow][iSuckCol]=false;
        }
        return;
    }
}
//------------------------------------------------------------------------------
void SearchCleanKitUpDown(int iRow, eWhichShuttle iSht)
{
    if(USE_PICKER_COUNT==ep1Picker)
    {
        SearchCleanKitRowCol(iSht);
        return;
    }

    if((TestIF.iShuttleMode==1 &&
        TestIF.iShuttle_Sel==1 &&
        TestIF.iAutoClean_SelectArm==1) ||                                      //kevin 20170126 (Steven) 選ARM 使用ARM 2
       (IniConfig.bA09_ByArmCloseSite==1 &&
        TestIF.iAutoClean_SelectArm==1) ||                                      //Isaac 20170601 (wei) add
       (CosFunction.bAutoCleanAutoSelIndexArm==true &&
        TestIF.bCleanIndexOtherArm==false &&
        TestIF_File.iShuttleMode==1 &&
        TestIF_File.iShuttle_Sel==1))                                           //JerryYang 20171017 (wei) 修正auto clean只開ARM2會發生hang up
    {
        iSht=euShuttle2;
    }

    if(iInArmType==e9045_2x6_8 &&
       iCloseSiteModeFor2x6<e2x6OneByOne &&                                     //Steven 20241113 : for 2x6 auto clean
       (TestIF_File.iAutoClean_DeveicePices==12 ||
        TestIF_File.iAutoClean_DeveicePices==24))
    {
        if(iShuttleRowKit==1 ||
           iShuttleRowKit==2)
        {
            if(bUse8Picker)
            {
                InArmSuck.SetItemData(0, 3, HAS_NULL_CLEAN_IC);
                InArmSuck.SetItemData(1, 3, HAS_NULL_CLEAN_IC);
            }
            else
            {
                InArmSuck.SetItemData(1, 3, HAS_NULL_CLEAN_IC);
            }
        }
        else if(iShuttleRowKit==3 ||
                iShuttleRowKit==4)
        {
            if(bUse8Picker)
            {
                InArmSuck.SetItemData(0, 0, HAS_NULL_CLEAN_IC);
                InArmSuck.SetItemData(1, 0, HAS_NULL_CLEAN_IC);
            }
            else
            {
                InArmSuck.SetItemData(1, 0, HAS_NULL_CLEAN_IC);
            }
        }
    }

    bool bFlag[8];
    bool bRowHasIC=false;
    int iKitRow=0, iKitCol=0;
    int iKitStep=0, iRealRow=1;
    int &iSuckRow=iPickerOrder, iSuckCol=0, iSiteRow=0, iSiteCol=0, iShtRow=0, iShtCol=0;

    ZeroMemory(bFlag, sizeof(bFlag));
    DoInArm_SuckerMap();                                                        //jou 2013-11-06 開放site 治具使用異常修正
    GetInarmSuckRow(iRow,iSuckRow,iKitStep);

    if(IniConfig.bE43AutoCleanUseHotplate ||                                    //ChungHung 20131120 AutoClean use Hotplate1
       bUse8Picker)                                                             //Steven 20201124 : fixed for 2x3 auto clean
    {
        iRealRow=iSuckRow;
    }
    else
    {
        iRealRow=1;
    }

    for(int j=0; j<InArmSuck.iMaxCol; j++)
    {
        iSuckCol=GetAutoCleanPickStep(j);                                       //Steven 20240918 : fixed for auto clean
        if(iSuckCol==-1)
        {
            continue;
        }

        iShtRow=iSuckRow;
        iShtCol=iSuckCol+iKitStep;                                              //Steven 20250917 : iSuckCol+iKitStep --> GetShuttleCol
        iSiteCol=GetShuttleCol(iShtRow, iShtCol);                               //Steven 20250917 : Fixed for 12site auto clean
        if(IsNNMode()==NN_2Row)
        {
            iSiteRow=(iSht==0)?2:0;
        }
        else if(IsNNMode()==NN_1Row)
        {
            iSiteRow=(iSht==0)?1:0;
        }
        else
        {
            iSiteRow=0;
        }

        if(i1x2_4UseACEGPicker==1)                                              //Steven 20230530 : 1x2_4齊平Row A
        {
            if(CosFunction.bUseAutoCleanCloseSiteAlsoDo==true)                  //Ifor 20181222 add 新增Auto Clean Close Site 一樣執行
            {
                bFlag[iSuckCol]=(TestIF.iSiteMap[iSiteRow][iSiteCol]>0 &&
                                 InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC);

                if(InArmSuck.Item[iRealRow][iSuckCol]!=NULL_IC)
                    bRowHasIC=true;
            }
            else
            {
                bFlag[iSuckCol]=(Prod.bInSuckUse[iSht][iShtRow][iShtCol] &&     //Steven 20241008 : fixed [iSuckCol+iKitStep]
                                 InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC);

                if(InArmSuck.Item[iRealRow][iSuckCol]!=NULL_IC)
                    bRowHasIC=true;
            }
        }
        else if(Prod.bInSuckUse[iSht][iShtRow][iShtCol]==true)
        {
            {
                if(CosFunction.bUseAutoCleanCloseSiteAlsoDo==true)                  //Ifor 20181222 add 新增Auto Clean Close Site 一樣執行
                {
                    bFlag[iSuckCol]=(TestIF.iSiteMap[iSiteRow][iSiteCol]>0 &&
                                     InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC);

                    if(InArmSuck.Item[iRealRow][iSuckCol]!=NULL_IC)
                        bRowHasIC=true;
                }
                else
                {
                    bFlag[iSuckCol]=(Prod.fInArmSuck4x8[iSht][iShtRow][iShtCol] &&  //Steven 20241008 : fixed [iSuckCol+iKitStep]
                                     InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC);

                    if(InArmSuck.Item[iRealRow][iSuckCol]!=NULL_IC)
                        bRowHasIC=true;
                }
            }
        }
    }

    for(int j=0; j<InArmSuck.iMaxCol; j++)
    {
        iSuckCol=GetAutoCleanPickStep(j);                                       //Steven 20240918 : fixed for auto clean
        if(iSuckCol==-1)
            continue;

        if((iInArmType==e9045_1x3_4 ||                                          //Steven 20241101 : Fixed for 1x3 auto clean
            iInArmType==e9045_1x3_2_14) &&
           TestIF_File.iAutoClean_DeveicePices%3==0)
        {
            if(iKitStep!=0 && iSuckCol==0)
                continue;
        }

        iShtRow =iSuckRow;
        iShtCol =iSuckCol+iKitStep;                                             //Steven 20250917 : iSuckCol+iKitStep --> GetShuttleCol
        iSiteCol=GetShuttleCol(iShtRow, iShtCol);                               //Steven 20250917 : Fixed for 12site auto clean

        if(bRowHasIC==true)                                                     //Steven 20240829 : 判斷是不是第二次治具
        {
            if((bFlag[iSuckCol] ||                                              //Steven 20240729 : fixed for auto clean pick 2 times from kit
                Prod.bInSuckUse[iSht][iShtRow][iShtCol]) &&                     //Steven 20241010 : iSuckCol --> iShtCol
               InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC)
            {
                iAutoCleanStart=j;
                break;
            }
        }
        else
        {
            if(InArmSuck.Item[iRealRow][iSuckCol]==NULL_IC)
            {
                iAutoCleanStart=j;
                break;
            }
        }
    }
    int iKRow=0,iKCol=0;
    if(SearchCleanKitRowCol(iKRow,iKCol))
    {
        iAutoCleanPickPlateX=iKCol;
        iAutoCleanPickPlateY=iKRow;
        for(int i=0; i<InArmSuck.iMaxCol; i++)
        {
            iSuckCol=GetAutoCleanPickStep(i);                                   //Steven 20240918 : fixed for auto clean
            if(iSuckCol==-1)                                                    //JerryYang 20250326 : fixed
            {
                continue;                                                       //Steven 20260504 : fixed for auto clean
            }
            else if(i-iAutoCleanStart<0)
            {
                {
                    bInArmSuckActive[iRealRow][iSuckCol]=false;
                    continue;
                }
            }

            iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(i-iAutoCleanStart);
            iKitRow=iAutoCleanPickPlateY;

            if(MOT[MMAutoCleanKit].Tray.Data[iKitCol][iKitRow]==HAS_CLEAN_IC &&
               iKitCol<MOT[MMAutoCleanKit].Tray.XItem)
            {
                bInArmSuckActive[iRealRow][iSuckCol]=bFlag[iSuckCol];
            }
            else
            {
                bInArmSuckActive[iRealRow][iSuckCol]=false;
            }
        }
        return;
    }
}
//------------------------------------------------------------------------------
bool CleanPad_PlaceToShuttle(int iSht)
{
    int flag=true;
    int iShtRow=0, iShtCol=0;
    int iPickKit32=(iShuttleRowKit==1 || iShuttleRowKit==2)?0:4;

    if(USE_PICKER_COUNT==ep1Picker)
    {
        GetNowInShuttleRowCol_All_1Picker(iSht, &iShtRow, &iShtCol, bAutoPlace);

        if(InArmSuck.Item[0][0]==HAS_NULL_CLEAN_IC ||
           (InArmSuck.Suck[0][0].GetNeedDestroyStatus() &&
            (InArmSuck.Item[0][0] &&
             InArmSuck.Suck[0][0].Destroy())))
        {
            if(iSht==euShuttle1)
                FLCarryKit.MoveSuckDataDiff(InArmSuck, 0, 0, iShtRow, iShtCol);
            else
                BLCarryKit.MoveSuckDataDiff(InArmSuck, 0, 0, iShtRow, iShtCol);
        }
        else if(InArmSuck.Item[0][0] &&
                InArmSuck.Suck[0][0].GetNeedDestroyStatus())
        {
            flag=false;
        }
    }
    else if(bUse8Picker)
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                if(iCloseSiteModeFor2x8==e2x8_STMMode ||                        //KevinCheng 20251031 : STM與TW153模式共用 +iPickKit32 避免+4的座標算錯
                    iCloseSiteModeFor2x8==e2x8_TW153Mode)
                {
                    iShtRow=i;
                    iShtCol=GetShuttleCol(i, j);

                    if(InArmSuck.Suck[i][j].GetNeedDestroyStatus() &&           //KevinCheng 20251031 : 補判斷條件 避免視為HAS_NULL_CLEAN_IC影響GetShuttleState_2x8_8判斷 造成iShuttleRowKit流程錯誤
                        (InArmSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                        (InArmSuck.Item[i][j] &&
                        InArmSuck.Suck[i][j].Destroy())))
                    {
                        if(iShtCol<8)
                        {
                            if(iSht==euShuttle1)
                                FLCarryKit.MoveSuckDataDiff(InArmSuck, i, j, iShtRow, iShtCol);
                            else
                                BLCarryKit.MoveSuckDataDiff(InArmSuck, i, j, iShtRow, iShtCol);
                        }
                    }
                    else if(InArmSuck.Item[i][j] &&
                            InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                    {
                        flag=false;
                    }
                }
                else
                {
                    iShtRow=i;
                    iShtCol=GetShuttleCol(i, j+iPickKit32);

                    if(InArmSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                       (InArmSuck.Suck[i][j].GetNeedDestroyStatus() &&
                        (InArmSuck.Item[i][j] &&
                         InArmSuck.Suck[i][j].Destroy())))
                    {
                        if(iShtCol<8)
                        {
                            if(iSht==euShuttle1)
                                FLCarryKit.MoveSuckDataDiff(InArmSuck, i, j, iShtRow, iShtCol);
                            else
                                BLCarryKit.MoveSuckDataDiff(InArmSuck, i, j, iShtRow, iShtCol);
                        }
                    }
                    else if(InArmSuck.Item[i][j] &&
                            InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                    {
                        flag=false;
                    }
                }
            }
        }
    }
    else
    {
        iShtRow=(iShuttleRowKit==2 || iShuttleRowKit==4)?1:0;
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            iShtCol=GetShuttleCol(iShtRow, j+iPickKit32);

            if(iCloseSiteModeFor2x8==e2x8_STMMode ||                            //KevinCheng 20251031 : STM與TW153模式共用 +iPickKit32 避免+4的座標算錯
                iCloseSiteModeFor2x8==e2x8_TW153Mode)
            {
                if(InArmSuck.Suck[1][j].GetNeedDestroyStatus() &&               //KevinCheng 20251031 : 補判斷條件 避免視為HAS_NULL_CLEAN_IC影響GetShuttleState_2x8_8判斷 造成iShuttleRowKit流程錯誤
                    (InArmSuck.Item[1][j]==HAS_NULL_CLEAN_IC ||
                   (InArmSuck.Item[1][j] &&
                    InArmSuck.Suck[1][j].Destroy())))
                {
                    if(iShtCol<8)
                    {
                        if(iSht==euShuttle1)
                            FLCarryKit.MoveSuckDataDiff(InArmSuck, 1, j, iShtRow, iShtCol);
                        else
                            BLCarryKit.MoveSuckDataDiff(InArmSuck, 1, j, iShtRow, iShtCol);
                    }
                }
                else if(InArmSuck.Item[1][j] &&
                        InArmSuck.Suck[1][j].GetNeedDestroyStatus())
                {
                    flag=false;
                }
            }
            else
            {
                if(InArmSuck.Item[1][j]==HAS_NULL_CLEAN_IC ||
                   (InArmSuck.Item[1][j] &&
                    InArmSuck.Suck[1][j].GetNeedDestroyStatus() &&
                    InArmSuck.Suck[1][j].Destroy()))
                {
                    if(iShtCol<8)
                    {
                        if(iSht==euShuttle1)
                            FLCarryKit.MoveSuckDataDiff(InArmSuck, 1, j, iShtRow, iShtCol);
                        else
                            BLCarryKit.MoveSuckDataDiff(InArmSuck, 1, j, iShtRow, iShtCol);
                    }
                }
                else if(InArmSuck.Item[1][j] &&
                        InArmSuck.Suck[1][j].GetNeedDestroyStatus())
                {
                    flag=false;
                }
            }
        }
    }

    return flag;
}
//------------------------------------------------------------------------------
bool CleanPad_PickFromShuttle(int iSht, int iSelRow)
{
    bool flag=true;
    int iShtRow, iShtCol;
    int iPickKit32=(iSelRow==1 || iSelRow==2)?0:4;

    if(fContact->IsRun2DCheck()==true)                                          //JerryYang 20260408 : fix 2DID Mapping
    {
        iPickKit32=0;
    }

    if(USE_PICKER_COUNT==ep1Picker)
    {
        GetNowInShuttleRowCol_All_1Picker(iSht, &iShtRow, &iShtCol, bAutoPick);
        if(ptrInSHT->Item[iShtRow][iShtCol] &&
           InArmSuck.iNeedSuck[0][0]!=NULL_IC)
        {
            if(ptrInSHT->Item[iShtRow][iShtCol]==HAS_NULL_CLEAN_IC ||
               ptrInSHT->Item[iShtRow][iShtCol]==HAS_NULL_IC ||         //JerryYang 20250220 : 2DID預鎖順序檢查功能
               InArmSuck.Suck[0][0].Suck())
            {
                InArmSuck.MoveSuckDataDiff(*ptrInSHT, iShtRow, iShtCol, 0, 0);
                InArmSuck.PordRec[0][0].AddPickCleanPadFormShuttle(0, 0, iShtRow, iShtCol);  //Sam 20230616 : Add Auto Clean Record
                ptrInSHT->Item[iShtRow][iShtCol]=NULL_IC;
            }
            else if(InArmSuck.Item[0][0]==NULL_IC &&
                    InArmSuck.Suck[0][0].Error==false)
            {
                flag=false;
            }
        }
    }
    else if(bUse8Picker)                                                             //Steven 20201014 : 齊平8治具auto clean
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                iShtRow=i;
                iShtCol=GetShuttleCol(i, j+iPickKit32);

                if(iShtCol<8 &&
                   ptrInSHT->Item[iShtRow][iShtCol] &&
                   InArmSuck.iNeedSuck[i][j]!=NULL_IC)
                {
                    if(ptrInSHT->Item[iShtRow][iShtCol]==HAS_NULL_CLEAN_IC ||
                       ptrInSHT->Item[iShtRow][iShtCol]==HAS_NULL_IC ||         //JerryYang 20250220 : 2DID預鎖順序檢查功能
                       InArmSuck.Suck[i][j].Suck())
                    {
                        InArmSuck.MoveSuckDataDiff(*ptrInSHT, iShtRow, iShtCol, i, j);
                        InArmSuck.PordRec[i][j].AddPickCleanPadFormShuttle(i, j, iShtRow, iShtCol);  //Sam 20230616 : Add Auto Clean Record
                        ptrInSHT->Item[iShtRow][iShtCol]=NULL_IC;
                    }
                    else if(InArmSuck.Item[i][j]==NULL_IC &&
                            InArmSuck.Suck[i][j].Error==false)
                    {
                        flag=false;
                    }
                }
            }
        }
    }
    else
    {
        iShtRow=(iSelRow==2 || iSelRow==4)?1:0;
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iShtCol=GetShuttleCol(iShtRow, j+iPickKit32);

            if(iShtCol<8 &&
               ptrInSHT->Item[iShtRow][iShtCol] &&
               InArmSuck.iNeedSuck[1][j]!=NULL_IC)
            {
                if(ptrInSHT->Item[iShtRow][iShtCol]==HAS_NULL_CLEAN_IC ||
                   InArmSuck.Suck[1][j].Suck())
                {
                    InArmSuck.MoveSuckDataDiff(*ptrInSHT, iShtRow, iShtCol, 1, j);
                    InArmSuck.PordRec[1][j].AddPickCleanPadFormShuttle(1, j, iShtRow, iShtCol);  //Sam 20230616 : Add Auto Clean Record
                }
                else if(InArmSuck.Item[1][j]==NULL_IC &&
                        InArmSuck.Suck[1][j].Error==false)
                {
                    flag=false;
                }
            }
        }
    }

    return flag;
}
//------------------------------------------------------------------------------
int CheckShuttleSensor_Clean(eWhichShuttle iSht, bool alarmflag)
{
    int ret=0;
    switch(TestIF.iTestMode)
    {
        case SingleSite:                                                        //1x1
            ret=CheckShuttleSensor_9045_1x1(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;                                                              //Ifor 20180313 (Steven) : add 避免 1x1 治具會亂跳
        case DualSite:                                                          //1x2
        case QualSite2X2N:                                                      //Steven 20201014 : for 2x2 nn mode auto clean
            ret=CheckShuttleSensor_9045_1x2(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        case QualSite1X4:                                                       //1x4
        case _8Site1X4:                                                         //ChungHung 20150528 add for 增加 _8Site1x4
        case _8Site2X4N:                                                        //Wei 20231211 : 2X4NN Mode
            ret=CheckShuttleSensor_9045_1x4(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        case QualSite2X2:                                                       //2x2
            ret=CheckShuttleSensor_9045_2x2(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        case TriSite1X3:                                                        //Frank 20160323 1x3 AutoClean Add
        case _6Site2X3N:                                                        //Steven 20220425 : 2X3NN Mode
        case _6Site2X3:                                                         //ChungHung 20150119 add for 2x3 mode autoclean
            ret=CheckShuttleSensor_9045_2x3(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        case _16Site4X4:                                                        //Sam 20190226 : 16Site4X4
        case _8Site2X4:                                                         //2x4
            ret=CheckShuttleSensor_9045_2x4(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        case _10Site2X5:                                                        //wei 20190614 10 site
            ret=CheckShuttleSensor_9045_2x5(iSht, alarmflag, true);
            break;
        case _12Site2X6:
            ret=CheckShuttleSensor_9045_2x6(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        case _16Site2X8:                                                        //2x8
        case _32Site4X8N:                                                       //Steven 20140512: For HT-9047
            ret=CheckShuttleSensor_9045_2x8(iSht, alarmflag, true);             //JerryYang 20191113 新增alarm code區分in shuttle floating error/ clean pad floating error
            break;
        default:
            ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "CheckShuttleSensor_Clean");
    }
    return ret;
}
//==============================================================================
bool CheckAutoCleanCloseSite(int iSht)                                          //Steven 20220929 : 判斷開放Site
{
    bool bResult=false;
    if(bUseTwoArm32Site==true)
    {
        bResult=(InArmSideAllClose(iSht));
    }
    return bResult;
}
//------------------------------------------------------------------------------
bool DoInArmPineRelease()
{
    static bool bHaveServoOn=false;
    static bool bXMoveFlag=false;
    static bool bYMoveFlag=false;
    static bool bNeedDelay=false;

    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20110802
    {
        if(fNote->bMyServoOffInArm)
        {
            if(bHaveServoOn==false)
            {
                MOT[MInArmX].ServoOnOff(true);
                MOT[MInArmY].ServoOnOff(true);
                tCleanInArmServoOnDelay.SetMSAndOn(500);
                bHaveServoOn=true;
                bXMoveFlag=false;
                bYMoveFlag=false;
            }

            if(InArmZSafe(DETECT_ALL_FLAG)==-1)                                 //如果Z軸在安全內可以走
            {
                if(tCleanInArmServoOnDelay.Off())
                {
                    if(bXMoveFlag==false)
                    {
                        bXMoveFlag=MOT[MInArmX].MotorMove(fNote->iMyServoOffInArmPosX);
                    }

                    if(bYMoveFlag==false)
                    {
                        bYMoveFlag=MOT[MInArmY].MotorMove(fNote->iMyServoOffInArmPosY);
                    }

                    if(bXMoveFlag==true && bYMoveFlag==true)
                    {
                        tCleanInArmServoOnDelay.SetMSAndOn(500);
                        fNote->bMyServoOffInArm=false;
                        bHaveServoOn=false;
                        bNeedDelay=true;
                        return true;
                    }
                }
            }
            else
            {
                fNote->bMyServoOffInArm=false;
                fAllMotorHome=false;

                if(IniConfig.bSPILFunction==true ||                             //JerryYang 20170328 (Jou) 揭示客戶會用一起SPILFunction
                   CUSTOMER_CODE==CC_SIGURD_PeiXing)                            //JerryYang 20160328 for 瑞士德_北京,觸發回home的地方要加上log
                    ShowMyMessage("In Arm PICK UP ALARM,Need home");            //Ifor 20151208 :客戶新增回饋才顯示 Show Alarm Message
                StopAllMotor();
                bNeedDelay=false;
            }
        }
        else
        {
            bHaveServoOn=false;
            bNeedDelay=false;
            return true;
        }
    }
    else
    {
        return true;
    }

    if(bNeedDelay==true &&                                                      //Steven 20110804 End: Servo Off後要回復的
       tCleanInArmServoOnDelay.Off()==false)                                    //Steven 20110809 : ServoOn後要Delay一下
    {
        bNeedDelay=false;
    }
    return false;
}
//------------------------------------------------------------------------------
bool DoInArmMoveToWaitPosByAutoClean()                                          //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
{
    if(CosFunction.bIndexJamInArmMoveSafePostionByAutoClaen)
    {
        if(bPlaceToShuttleByAutoClean || bPickFromShuttleByAutoClean)
        {
            bLockPlaceToShuttleByAutoClean=false;
            bLockPickFromShuttleByAutoClean=false;
            return false;
        }
        else
        {
            bLockPlaceToShuttleByAutoClean=true;
            bLockPickFromShuttleByAutoClean=true;

            if(bPlaceToCleanKit)                                                //Steven 20171204 (Wei) : 避免正在放料還沒回到原本位置放
                return false;

            if(bPickFromKitByAutoClean==true)                                   //Steven 20210603 : 新增治具Clean Kit的Flag
                return false;

            if(MoveInArm2XYToWait())                                            //如果要回到,而且還沒到達安全位
            {
                bLockPlaceToShuttleByAutoClean=false;
                bLockPickFromShuttleByAutoClean=false;
                return true;                                                    //可以開放In Arm
            }
        }
    }
    else
    {
        bLockPlaceToShuttleByAutoClean=false;
        bLockPickFromShuttleByAutoClean=false;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void RecDebug(TMyKitSuck &tray, int iTask)
{
    for(int i=0; i<MAX_Index_Row; i++)
    {
        for(int j=0; j<NEW_MAX_Index_Col; j++)
        {
            MNetLog(AnsiString().sprintf("Task=%d Row=%d Col=%d Item=%d", iTask, i, j, tray.Item[i][j]));
        }
    }
}
//------------------------------------------------------------------------------
bool DoSocketSensorAlarm(AnsiString sFunc,int Task)
{
    if(DoInArmMoveToWaitPosByAutoClean()==true)                                 //JerryYang 20241118 : fix auto clean hang up   //Steven 20130613 : Index異常後, In Arm要有先讓功能
    {
        RecordProcess(AnsiString().sprintf("%s__%d: iShowSocketSensor=%d",sFunc,Task, iShowSocketSensor));

        if(TestIF_File.bEnSocketSensor && iShowSocketSensor>0)                  //kevin 20130504 socket sensor
        {
            if(iShowSocketSensor==2)                                            //JerryYang 20161024 Socket sensor改成能歸位置零
            {
                ShowErrorMessage("WAR0323", K_RETRY, MTestZ1, false, sSocketSensorErr);  //Socket detect device floting error
            }
            else if(iShowSocketSensor==1)
            {
                ShowErrorMessage("WAR0322", K_RETRY, MTestZ1, false, sSocketSensorErr);
            }
            iShowSocketSensor=0;
        }
    }
    return true;
}

//==============================================================================
//  Part D -- TfCleaning free-function translations (golden AutoClean/uCleaning.cpp)
//==============================================================================

//------------------------------------------------------------------------------
// AI(W906-AutoCleanFoundation) 20260721: golden TfCleaning::SetCleanCellValue
// (uCleaning.cpp:2045) -- NOT in the task brief's named Part D pair, but
// SetDeviceInTray (below) calls it directly, so it is translated as a
// required file-local dependency (same treatment as SetAutoCleanStringGrid
// above). iMode==eAutoCleanUsed routes to the real MOT[MMAutoCleanKit] HAL;
// iMode==eUcleanUsed (0) routes to fMain->tmyAutoClean->SetCellColorIndex, a
// write-only cosmetic grid-paint sink (see FormsFacade.h's TfMainAutoCleanGrid
// banner) -- not exposed in AutoClean.h (golden TfCleaning-private helper).
//------------------------------------------------------------------------------
static void SetCleanCellValue(int iXPos, int iYPos, int iType, int iMode)  //Jimmychiu 20221027 統一Cleanpad配置方式  iMode:0=SetCellColorIndex    1=SetTraySingleData
{
    if(iMode==eAutoCleanUsed)                                                   //1=SetTraySingleData
    {
        MOT[MMAutoCleanKit].SetTraySingleData(iXPos, iYPos, iType);
    }
    else                                                                        //0=SetCellColorIndex
    {
        fMain->tmyAutoClean->SetCellColorIndex(iXPos, iYPos, iType);
    }
}
//------------------------------------------------------------------------------
void SetDeviceInTray(int iXItem, int iYItem, int iDeviceNum, int iMode)  //Jimmychiu 20221027 統一Cleanpad配置方式
{
    int iHadSetDevNum=0;
    for(int iYPos=0; iYPos<iYItem; iYPos++)
    {
        for(int iXPos=0; iXPos<iXItem; iXPos++)
        {
            if(iHadSetDevNum>=iDeviceNum)
            {
                return;
            }

            if(TestIF_File.iTestMode==TriSite1X3 ||                             //Steven 20240112 : fixed for 1x3_14
               TestIF_File.iTestMode==_6Site2X3N)
            {
                if(TestIF_File.iTestMode==TriSite1X3 &&                         //JerryYang 20260408 : fix 1x3 2治具
                   iInArmType==e9045_1x3_2_14 &&
                   TestIF_File.iSiteMap[0][1]<=0)
                {
                }
                else
                {
                    if(iXItem==4 && iXPos==0)
                        continue;
                }
            }

            // AI(W906-AutoCleanFoundation) 20260721: golden's own operator
            // precedence VERBATIM -- parses as (iTestMode==_12Site2X6 &&
            // iDeviceNum==12) || iDeviceNum==24, i.e. iDeviceNum==24 alone
            // (any iTestMode) also qualifies. Compiler flags this shape
            // (-Wparentheses); preserved unchanged per this wave's
            // faithful-translation mandate (don't silently fix golden quirks).
            if(TestIF_File.iTestMode==_12Site2X6 &&
               iDeviceNum==12 || iDeviceNum==24)                                //Steven 20241111 : for 1x4 run 3 col tray auto clean
            {
                if(TestIF_File.iAutoClean_XDivision==4)
                {
                    if(iXPos==0)
                        continue;
                }
                else if(TestIF_File.iAutoClean_XDivision==8)
                {
                    if(iXPos==0 || iXPos==1)
                        continue;
                }
            }

            if(TestIF_File.iTestMode==QualSite1X4 &&
               TestIF_File.iAutoClean_XDivision==3)                             //Steven 20241111 : for 1x4 run 3 col tray auto clean
            {
                if(iXPos==0)
                    continue;
            }

            if(TestIF_File.iTestMode==DualSite ||                               //Steven 20190509 : 修正位置
               TestIF_File.iTestMode==QualSite2X2N)                             //Steven 20201014 : for 2x2 nn mode auto clean
            {
                if(USE_PICKER_COUNT==0)
                {
                    if(iXPos>=2)
                    {
                        SetCleanCellValue((iXPos*2)-3, iYPos, HAS_CLEAN_IC, iMode);
                        iHadSetDevNum++;
                    }
                    else
                    {
                        SetCleanCellValue(iXPos*2, iYPos, HAS_CLEAN_IC, iMode);
                        iHadSetDevNum++;
                    }
                }
                else if(iDeviceNum==2 && iXItem==8)     //JerryYang 20260527 : fix dual site auto clean卡料問題
                {
                    SetCleanCellValue(iXPos*4, iYPos, HAS_CLEAN_IC, iMode);
                    iHadSetDevNum++;
                }
                else
                {
                    SetCleanCellValue(iXPos, iYPos, HAS_CLEAN_IC, iMode);
                    iHadSetDevNum++;
                }
            }
            else
            {
                if(TestIF_File.iTestMode==QualSite2X2 && iAutoCleanUseXPitch==2)
                {
                    SetCleanCellValue(iXPos, iYPos, HAS_CLEAN_IC, iMode);
                    iHadSetDevNum++;
                }
                else if((TestIF_File.iTestMode==_16Site2X8 ||
                         TestIF_File.iTestMode==_32Site4X8N) &&                 //Steven 20250912 : for 16site auto clean with 12x16 Hot plate
                         iXItem==12 && iYItem==4)
                {
                    if(iXPos<8)
                        continue;

                    SetCleanCellValue(iXPos, iYPos, HAS_CLEAN_IC, iMode);
                    iHadSetDevNum++;
                }
                else if(TestIF_File.iAutoClean_XDivision>4 &&
                        ((bCleanKitPitchLess4000 && TestIF_File.iAutoClean_DeveicePices<=4) ||   //Alick 20161107 add 修正CLEAN TRAY XPITCH<13.33 & CleanIC<=4,AUTOCLEAN放料CLEAN IC放置位置錯誤
                         (iAutoCleanUseXPitch==2 && TestIF_File.iAutoClean_DeveicePices<4)))     //Alick 20170322 (wei) add 1*4時，Tray X>4，iAutoCleanUseXPitch=2，修正使用4顆CLEANPAD時取料會變成兩次 //Sam 20180208 "<=4" => "<4"
                {
                    SetCleanCellValue(iXPos*2, iYPos, HAS_CLEAN_IC, iMode);
                    iHadSetDevNum++;
                }
                else
                {
                    SetCleanCellValue(iXPos, iYPos, HAS_CLEAN_IC, iMode);
                    iHadSetDevNum++;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
bool CleanPadCountCanSupport2Arm()                                              //Steven 20221006 : 修正雙arm auto clean只有一半的clean pad
{
    bool bSupport2Arm=true;

    if(TestIF_File.iTestMode==SingleSite ||
       iInArmType==e9045_1x4_1_Ac ||
       fCleaning->b1x2SiteAbClosePutDummy==true)
    {
        if(TestIF_File.iAutoClean_DeveicePices==1)
            bSupport2Arm=false;
    }
    else if(TestIF_File.iTestMode==DualSite     ||
            TestIF_File.iTestMode==DualSite2x1  ||
            TestIF_File.iTestMode==QualSite2X2N)
    {
        if(TestIF_File.iAutoClean_DeveicePices<=2)
            bSupport2Arm=false;
    }
    else if(TestIF_File.iTestMode==TriSite1X3 ||
            TestIF_File.iTestMode==_6Site2X3N)
    {
        if(TestIF_File.iAutoClean_DeveicePices<=3)
            bSupport2Arm=false;
    }
    else if(TestIF_File.iTestMode==QualSite1X4 ||
            TestIF_File.iTestMode==QualSite2X2 ||
            TestIF_File.iTestMode==_8Site2X4N)                                  //Wei 20231211 : 2X4NN Mode
    {
        if(TestIF_File.iAutoClean_DeveicePices<=4)
            bSupport2Arm=false;
    }
    else if(TestIF_File.iTestMode==_6Site2X3 ||
            TestIF_File.iTestMode==_8Site2X4 ||
            TestIF_File.iTestMode==_16Site4X4 ||
            iCloseSiteModeFor2x6>e2x6OneByOne ||
            iCloseSiteModeFor2x8>e2x8OneByOne)
    {
        if(TestIF_File.iAutoClean_DeveicePices<=8)
            bSupport2Arm=false;
    }
    else if(TestIF_File.iTestMode==_12Site2X6)                                  //Steven 20241113 : for 2x6 auto clean
    {
        if(TestIF_File.iAutoClean_DeveicePices<=12)
            bSupport2Arm=false;
    }
    else if(TestIF_File.iTestMode==_10Site2X5  ||
            TestIF_File.iTestMode==_16Site2X8  ||
            TestIF_File.iTestMode==_32Site4X8N)
    {
        if(TestIF_File.iAutoClean_DeveicePices<=16)
            bSupport2Arm=false;
    }
    else
    {
        ShowMyMessage("The mode is not support!!", "Please save state record and provide to HonPrec software engineer", "CleanPadCountCanSupport2Arm");
    }
    return bSupport2Arm;
}

//==============================================================================
//  W906-AutoCleanCluster (20260722) -- Part E: 9 small helpers
//==============================================================================
//------------------------------------------------------------------------------
void CleanOnlyHasNullInShuttle()                                                //jimmychiu 20220624 add
{
    bool bflag=true;
    for(int i=0; i<InArmSuck.iShtRow; i++)
    {
        for(int j=0; j<InArmSuck.iShtCol; j++)
        {
            if(BLCarryKit.Item[i][j]==HAS_NULL_CLEAN_IC ||
               BLCarryKit.Item[i][j]==NULL_IC)
            {
            }
            else
            {
                bflag=false;
                break;
            }
        }
    }

    if(bflag)
    {
        BLCarryKit.SetAllToNullIC();
    }
}
//------------------------------------------------------------------------------
void InitialSet()
{
    CleanSetSpeed(true);                                                        //kevin 20141228 setup Autoclean的speed

    HotplatlXOffset             =TestIF_File.HotplatlXOffset;                   //kevin 20150209 add hotplate Xpos
    HotplatlYOffset             =TestIF_File.HotplatlYOffset;                   //kevin 20150209 add hotplate Ypos
    HotplatlPickOffset          =TestIF_File.HotplatlPickOffset;                //kevin 20150209 add hotplate Pick
    HotplatlPlaceOffset         =TestIF_File.HotplatlPlaceOffset;               //kevin 20150209 add hotplate Place

    iArmPickTrayPos             =TestIF_File.iPadThickness+HotplatlPickOffset;  //kevin 20120623 In out arm 吸TRAY IC Offset
    iArmPlaceTrayPos            =TestIF_File.iPadThickness+HotplatlPlaceOffset; //kevin 20120623 In out arm 放TRAY IC Offset
    HotplatePitchOffset         =TestIF_File.HotplatlPitchOffset;               //kevin 20150209 add hotplate Pitch

    if(bUse_NewAutoCleanForm==1)                                                //kevin 20150720 使用哪一arm clean
    {
       bUseCleanArm=TestIF_File.iAutoClean_SelectArm;
    }

    if(bUseCleanArm==0)                                                         //kevin 20140903 使用ARM1
    {
        iIndexPickShuttlePos    =Prod.TestZ1_Pick+TestIF_File.iPadThickness;    //kevin 20120623 Index 吸取 Shuttle IC Offset
        iIndexPlaceShuttlePos   =Prod.TestZ1_Place+TestIF_File.iPadThickness;   //TestIF_File.iAutoClean_Fix3PickOffset;//kevin 20120623 Index 放 Shuttle IC Offset
        iIndexWorkDownPos       =Prod.TestZ1_Test+TestIF_File.iPadThickness+TestIF_File.iAutoClean_ContactCleanHeight;//kevin 20120623 Index Clean down pos
        iIndexWorkUpPos         =(Prod.TestZ1_Test+TestIF_File.iPadThickness)/2;//kevin 20121114 Bill 平臺最高的最低一半 Index Clean down pos
        if(iIndexWorkUpPos<Prod.TestZ1_Test)
            iIndexWorkUpPos     =Prod.TestZ1_Test+1000;
    }
    else if(bUseCleanArm==1)//kevin 20140903 使用ARM2
    {
        iIndexPickShuttlePos    =Prod.TestZ2_Pick+TestIF_File.iPadThickness;    //kevin 20120623 Index 吸取 Shuttle IC Offset
        iIndexPlaceShuttlePos   =Prod.TestZ2_Place+TestIF_File.iPadThickness;   //TestIF_File.iAutoClean_Fix3PickOffset;//kevin 20120623 Index 放 Shuttle IC Offset
        iIndexWorkDownPos       =Prod.TestZ2_Test+TestIF_File.iPadThickness+TestIF_File.iAutoClean_ContactCleanHeight;//kevin 20120623 Index Clean down pos
        iIndexWorkUpPos         =(Prod.TestZ2_Test+TestIF_File.iPadThickness)/2;//kevin 20121114 Bill 平臺最高的最低一半 Index Clean down pos

        if(iIndexWorkUpPos<Prod.TestZ2_Test)
            iIndexWorkUpPos     =Prod.TestZ2_Test+1000;
    }
}
//------------------------------------------------------------------------------
void EnableAutoclean(bool Manual)
{
    if(Manual)
    {
        if(TestIF.iAutoClean_Function &&bIsAutoOneCycle==false && iOneCycle==0) //kevin 20140217
        {
            InitialAutoCleanAllTask();                                          //Sam 20230504 : 整理 InitialAutoCleanTask
            bIsAutoOneCycleAutoclean=true;
        }
    }
    else
    {
        fMain->AutoCleanContactCountLabel->Caption=iAutoClean_IndexContactCount;
        if(TestIF.iAutoClean_Function && TestIF.iAutoClean_IntervalContact!=0 &&iOneCycle==0 &&//kevin 20140217
           iAutoClean_IndexContactCount>=(TestIF.iAutoClean_IntervalContact))   //kevin 20121022   iCleanOut==0
        {
            AutoCleanWriteData("iAutoClean_IndexTime", iAutoClean_IndexContactCount);
            InitialAutoCleanAllTask();                                          //Sam 20230504 : 整理 InitialAutoCleanTask
            bIsAutoOneCycleAutoclean=true;
        }
    }
}
//------------------------------------------------------------------------------
void __fastcall AutoCleanWriteData(AnsiString Str, int Data)                    //kevin 20120710 記錄資料
{
    AnsiString S="";
    AnsiString szDir="";
    S=GetLastOpenFN();
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
    WriteIniData(szDir, "Configuration", Str, Data);                            //kevin 20120623
}
//------------------------------------------------------------------------------
void SetAutoCleanICCount(bool Work)                                             //ChungHung 20141027 add for SCK want to record AutoClean_pad count
{
    if(InitialOK==false)                                                        //Steven 20200423 : 加上防呆機制
        return;

    int iXItem=0, iYItem=0, iSet=0, iStep=0;
    // AI(W906-AutoCleanCluster) 20260722: golden calls DoTestIFConvert() here   //AI(W906-FLOW-5) 20260929: SUPERSEDED -- the gate below is retired.  Both premises are dead: the memcpy hazard is gone (the port's DoTestIFConvert, cUnitConvert.cpp:370, assigns TestIF = TestIF_File -- GATE PTW8-UC-1), and "NO live caller" is stale (csystem.cpp:11058 and :13770, FileRW/TestIF_File_Cleaning.cpp:388, FileRW/TestIF_File_Cleaning.gen.inc:1314/:1321/:3367).  The text below is history.
    // (Steven 20160630 : for auto clean, TestIF_File --> TestIF) -- see this
    // wave's own report for the full write-up. SHORT VERSION: DoTestIFConvert's
    // real body (cUnitConvert.cpp:27) opens with
    // `memcpy(&TestIF.iTestMode, &TestIF_File.iTestMode, sizeof(TestIF_File));`
    // -- a raw memcpy blitting the ENTIRE SYSTEM_TEST_IF struct, which contains
    // AnsiString members (sTestMode/sDioName). In THIS tree AnsiString is backed
    // by std::string (vclcompat/AnsiString.h) -- a real heap-owning C++ object,
    // not BCB6's ref-counted-pointer AnsiString -- so memcpy-ing over it bypasses
    // every copy ctor/assignment operator and blits raw std::string internals
    // (SSO buffer / heap pointer / size / capacity), leaving BOTH the source and
    // destination objects believing they own the same (or a stale) heap buffer:
    // a genuine double-free/heap-corruption hazard, materially worse than
    // golden's own already-borderline BCB6 memcpy semantics -- not merely a
    // style concern. A hand-written member-wise substitute would have to span
    // the ENTIRE SYSTEM_TEST_IF struct (~30+ fields, most unrelated to
    // AutoClean: RS232/GPIB/barcode/etc.), which is disproportionate scope
    // creep for this cluster and would silently encode an undocumented
    // full-struct-field-list dependency that rots the moment cprod.h's
    // SYSTEM_TEST_IF changes shape. Per this wave's task brief, gated rather
    // than forced (matching this tree's established "don't force an unsafe
    // unlock" precedent). SetAutoCleanICCount itself has NO live caller
    // anywhere in the translated tree yet (its own golden callers --
    // AutoClean/uCleaning.cpp, csystem.cpp, main.cpp -- are all out of scope
    // this wave), so gating this one call site does not regress any
    // currently-reachable behavior.
    void DoTestIFConvert();   //AI(W906-FLOW-5) 20260929: TODO(W906+) gate RETIRED (was '#if 0 // TODO(W906+): DoTestIFConvert -- see banner comment above (memcpy-over-AnsiString hazard)').  Block-scope declaration: cUnitConvert.h declares no DoTestIFConvert.  Body cUnitConvert.cpp:370 = golden cUnitConvert.cpp:27-56, memory only (TestIF = TestIF_File, six x100 fields, Kit/Tray AutoClean geometry); no file I/O
    DoTestIFConvert();                                                          //Steven 20160630 : for auto clean, TestIF_File --> TestIF
//AI(W906-FLOW-5) 20260929: (was '#endif' of the retired DoTestIFConvert gate; golden AutoClean.cpp:589 is the line above)

    if(TestIF_File.iAutoClean_Function==false)                                  //Ifor 20171024 : TestIF.iAutoClean_Function => TestIF_File.iAutoClean_Function
    {
        for(int j=0; j<TestIF.iAutoClean_YDivision; j++)
        {
            for(int i=0; i<TestIF.iAutoClean_XDivision; i++)
            {
                MOT[MMAutoCleanKit].SetTraySingleData(i, j, NULL_IC);
            }
        }
        return;
    }

    if(bRunAutoClean==true && iAutoCleanAlarm!=2)                               //Steven 20180221 : 修正進去Auto Clean前的資料會被清空的問題  //Ifor 20180727 (wei) ：Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
    {
        return;
    }

    SetAutoCleanTrayPosition();                                                 //Steven 20210825 : 重新整理該Function
    if(Work==false)                                                             //平放位置
    {
        fMain->AutoCleanStringGrid->ColCount=TestIF.iAutoClean_XDivision;
        fMain->AutoCleanStringGrid->RowCount=(TestIF.iAutoClean_YDivision+1)*3; //ChungHung 20140317 alter 如果因為1 RowCount 會小於 TestIF_File.iAutoClean_YDivision*3 --->  (TestIF_File.iAutoClean_YDivision+1)*3

        fMain->tmyAutoClean->XItem=TestIF.iAutoClean_XDivision;
        fMain->tmyAutoClean->YItem=TestIF.iAutoClean_YDivision;
        MOT[MMAutoCleanKit].Tray.SetXYItem(TestIF.iAutoClean_XDivision, TestIF.iAutoClean_YDivision);     //Steven 20160614 : 設定Tray XY Item的function加上防呆

        GetXPitchOfCleanKit();

        if(TestIF.iTestMode==QualSite2X2 &&                                     //jou 20210712 : 修正 QualSite2X2 Tray pitch 小於2666 hang up
           bCleanKitPitchLess4000==true &&
           TestIF.iAutoClean_DeveicePices==4 &&
           TestIF.iAutoClean_XDivision==8)
        {
            for(int i=0; i<TestIF.iAutoClean_DeveicePices; i++)
            {
                iSet=i%2+1;
                int X=i*2;
                int Y=ChangeToFloatNonPcnt((double)(i), (double)(TestIF.iAutoClean_XDivision))+TestIF.iAutoClean_YDivision+2;
                SetAutoCleanStringGrid(X, Y, AnsiString(iSet));
            }
        }
        else
        {
            iStep=GetAutoCleanPickCount();                                      //ChungHung 20130711 add //一次使用幾顆吸嘴

            int iNumGroup=iAutoCleanUseXPitch*iStep;
            for(int i=0; i<TestIF.iAutoClean_DeveicePices; i++)
            {
                iSet=(ChangeToFloatNonPcnt((double)((i)), (double)(iNumGroup)))*(iAutoCleanUseXPitch);
                if((i+1)%iAutoCleanUseXPitch==0)
                    iSet=iSet+iAutoCleanUseXPitch;
                else
                    iSet=iSet+(i+1)%iAutoCleanUseXPitch;

                if(iSet>ChangeToFloatNonPcnt((double)(TestIF.iAutoClean_DeveicePices), (double)(iStep)))
                    iSet=ChangeToFloatNonPcnt((double)(TestIF.iAutoClean_DeveicePices), (double)(iStep));

                if(TestIF.iAutoClean_XDivision<=0 ||
                   TestIF.iAutoClean_YDivision<=0)                              //kevin 20140930
                    continue;

                int X=i%TestIF.iAutoClean_XDivision;
                int Y=ChangeToFloatNonPcnt((double)(i), (double)(TestIF.iAutoClean_XDivision))+TestIF.iAutoClean_YDivision+2;
                SetAutoCleanStringGrid(X, Y, AnsiString(iSet));
            }
        }
    }

    iXItem=TestIF.iAutoClean_XDivision;
    iYItem=TestIF.iAutoClean_YDivision;
    MOT[MMAutoCleanKit].Tray.ClearData();                                       //kevin 20190402 add clean autoclean set
    // AI(W906-AutoCleanCluster) 20260722: golden `fCleaning->SetDeviceInTray(...)`
    // -- SetDeviceInTray is a Wave-14 Part D free-function translation (fCleaning
    // demoted away from a VCL form method), same for eAutoCleanUsed below.
    SetDeviceInTray(iXItem, iYItem, TestIF.iAutoClean_DeveicePices, eAutoCleanUsed);  //Jimmychiu 20221027 統一Cleanpad配置方式

    MOT[MMAutoCleanKit].Refresh();                                              //wei 20150422 Refresh
    ReadWriteAutoCleanCount(true);                                              //Jimmychiu 20250103 : fixed for auto clean count to 0 issue
    ReadWriteAutoCleanCount(false);                                             //Steven 20180524 : Fixed for clean count
}
//------------------------------------------------------------------------------
void SetAutoCleanTrayPosition()                                                 //Steven 20210825 : 重新整理該Function
{
    static int iTop=351;                                                        //fMain->tmyAutoClean->Top;
    static int iWidth=89;                                                       //fMain->tmyAutoClean->Width;
    static int iHeight=25;                                                      //fMain->tmyAutoClean->Height;
    if(IniConfig.bE43AutoCleanUseHotplate)                                      //ChungHung 20131120 AutoClean use Hotplate1
    {
        fMain->tmyAutoClean->Top    =fMain->mtPlate2->Top;
        fMain->tmyAutoClean->Width  =fMain->mtPlate2->Width;
        fMain->tmyAutoClean->Height =fMain->mtPlate2->Height;
    }
    else
    {
        fMain->tmyAutoClean->Top    =iTop;                                      //wei 20220728 clean kit確定位置
        fMain->tmyAutoClean->Width  =iWidth;                                    //wei 20220728 clean kit確定位置
        fMain->tmyAutoClean->Height =iHeight;                                   //wei 20220728 clean kit確定位置
    }
}
//------------------------------------------------------------------------------
void SearchiAutoCleanNum()                                                      //Steven 20171212 (Wei) : 確認目前要吸的下一個Pad位置
{
    int iMin=TestIF_File.iAutoClean_AlarmCount+99999;                           //JerryYang 20210222 : 初始值更大一點//KevinYang 20200602 : 修正設定Alarm Count 1000會hang up的問題
    int iNow=0;
    for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
    {
        for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
        {
            if(MOT[MMAutoCleanKit].Tray.Data[X][Y]!=NULL_IC && MOT[MMAutoCleanKit].Tray.Data[X][Y]!=HAS_NULL_CLEAN_IC)
            {
                iNow=atoi(fMain->AutoCleanStringGrid->Cells[X][Y+1].c_str());
                if(iNow<iMin)
                {
                    iMin=iNow;
                    iAutoCleanNum=atoi(fMain->AutoCleanStringGrid->Cells[X][Y+TestIF_File.iAutoClean_YDivision+2].c_str());
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
AnsiString GetMotFunc(AnsiString asFunc,int iTask)
{
    return AnsiString().sprintf("%s %d", asFunc, iTask);
}
//------------------------------------------------------------------------------
void ResetAutoClean()
{
    bool bKitNeedClear=false;

    if(InArmSuck.HasDefineIC(HAS_NULL_CLEAN_IC) ||                              //JerryYang 20160824 回Home後in arm上有HAS_NULL_CLEAN_IC也要清除
       InArmSuck.HasDefineIC(HAS_CLEAN_IC) ||
       InArmSuck.HasDefineIC(CLEAN_FINISH_IC))
    {
        bKitNeedClear=true;
    }

    if(FLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC) ||                             //2014-04-01 Dell   只有在Autoclean 回home 才要清掉clean pad
       FLCarryKit.HasDefineIC(HAS_CLEAN_IC) ||
       FLCarryKit.HasDefineIC(CLEAN_FINISH_IC))
    {
        bKitNeedClear=true;
    }

    if(BLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC) ||                             //2014-04-01 Dell   只有在Autoclean 回home 才要清掉clean pad
       BLCarryKit.HasDefineIC(HAS_CLEAN_IC) ||
       BLCarryKit.HasDefineIC(CLEAN_FINISH_IC))
    {
        bKitNeedClear=true;
    }

    if(FTestSuck.HasDefineIC(HAS_NULL_CLEAN_IC) ||                              //JerryYang 20160825 回Home後Index arm上有Clean pad要清除
       FTestSuck.HasDefineIC(HAS_CLEAN_IC) ||
       FTestSuck.HasDefineIC(CLEAN_FINISH_IC))
    {
        bKitNeedClear=true;
    }

    if(BTestSuck.HasDefineIC(HAS_NULL_CLEAN_IC) ||                              //JerryYang 20160825 回Home後Index arm上有Clean pad要清除
       BTestSuck.HasDefineIC(HAS_CLEAN_IC) ||
       BTestSuck.HasDefineIC(CLEAN_FINISH_IC))
    {
        bKitNeedClear=true;
    }

    if(bKitNeedClear)
    {
        ShowMyMessage("Please remove all the clean pad on Shuttle, In Arm and Index arm!", "And put it back to the Clean Kit");

        RestoreCleanKitData();                                                  //ChungHung 20130628 add 復原已被InArm吸走的CleanKit上的IC計算  請勿刪除
        InArmSuck.ClearAll();
        FLCarryKit.ClearAll();
        BLCarryKit.ClearAll();
        FTestSuck.ClearAll();                                                   //JerryYang 20160825 回Home後Index arm上有Clean pad要清除
        BTestSuck.ClearAll();                                                   //JerryYang 20160825 回Home後Index arm上有Clean pad要清除
        fAllMotorHome=false;                                                    //JerryYang 20160825 修正auto clean中回home沒有真的回home流程的問題
    }

    bRunAutoClean=false;                                                        //Steven 20221219 : 歸零準備關閉Auto Clean動作的位置
}

//==============================================================================
//  W906-AutoCleanCluster (20260722) -- Part F: 4 core pick/place engines
//==============================================================================
//------------------------------------------------------------------------------
int DoAutoCleanPickfromCleanKit(eWhichShuttle iSht, bool Restart)
{
    int iResult=0, iXPos, iYPos;
    int &Task=iAutoCleanPickFromCleanKitStageTask, ret=0, iContectCount;
    bool flag=false;
    static int iRetryCT=0;
    #ifdef DEBUG_AUTO_CLEAN
    static int iOldTask=-1;
    #endif

    static int iSuckRow=0;                                                      //ChungHung 20131120 AutoClean use Hotplate1   //ChungHung 20140317 alter 改為靜態變數

    int  iKitRow, iKitCol;
    bool bIsSuckICFallDown[MAX_SOCKET_ROW][MAX_SOCKET_COL]={false};
    bool bHasDuplicateErr=false;
    AnsiString ErrPart="";
    AnsiString Message;
    int iStatus=0;
    int iCleanPadCount=0;
    int iSuckCol=0;
    bool bPauseWhenPick=false;

    QueueTaskList[40].CheckTaskChange();                                        //Steven 20220218 : Auto clean記錄Task變化

    if(Restart)
    {
        iAutoCleanNum=1;
        Task=1;
        #ifdef DEBUG_AUTO_CLEAN
            iOldTask=1;
            Message.sprintf("DoAutoCleanPickfromCleanKit initial task");
            fMain->AddAutoCleanMessage(Message);
        #endif
        return iResult;
    }

    #ifdef DEBUG_AUTO_CLEAN
    if(iOldTask!=Task)
    {
        Message.sprintf("DoAutoCleanPickfromCleanKit %d, %d, Go to Task, %d", iSht, iOldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        iOldTask=Task;
    }
    #endif

    switch(Task)
    {
        case 1:
            MOT[MInArmX].PCIL132_StopMotor();
            MOT[MInArmY].PCIL132_StopMotor();
            if(MoveInArmZToPlateSafe(Task))                                     //In arm z軸移至安全位置
            {
                InOutArmSuckActiveSet();                                        //重置min arm吸嘴狀態=false
                Task=10;
            }
            break;
        case 10:
            if(TrayHasCleanIC())                                                //是否有CLEAN PAD
            {
                if(CheckCleaningCount()==false)                                 //Steven 20210127 : 修正auto clean count達到時會alarm //Steven 20220114 : 順序提前, 避免shuttle先被塞入HAS_NULL_CLEAN_IC
                {
                    fCleaning->bResetCleanCount=true;
                    // AI(W906-AutoCleanCluster) 20260722: golden
                    // `fShowBinSelect->btnCleanReset->Enabled=true;` -- fShowBinSelect
                    // has NO link-visible shared home in this tree (only TU-local
                    // #define seams inside csystem.cpp/auto9045.cpp, verified by
                    // grep -- same gap this file's own SearchCleanNum banner
                    // already documented for `->ed_AutoCleanCount->Text=`). Same
                    // treatment: this is a pure UI-enable write nothing else in
                    // this cluster reads back; dropped rather than standing up a
                    // brand-new facade class for one write.
                    fShowBinSelect->btnCleanReset->Enabled=true;                   //AI(W906-FLOW-2) 20260928: golden :2655 restored (fShowBinSelect is a real global now; the drop note above is history)
                    Task=30;
                    break;
                }

                if(iInArmType==e9045_1x4_2_14)
                {
                    if(iSht==euShuttle1)
                        ptrInSHT=&FLCarryKit;
                    else
                        ptrInSHT=&BLCarryKit;
                    iShuttleRowKit=GetNeedSuckActive_1x4_2_14((int)iSht, bAutoPlace);
                }
                else if(USE_PICKER_COUNT==ep1Picker)
                {
                    iShuttleRowKit=GetNowSiteKitMode_All_1Pick(iSht, bAutoPlace);
                }
                else
                {
                    iShuttleRowKit=GetShuttleState(iSht, bAutoPlace);
                }
                Task=20;

                if(USE_PICKER_COUNT==ep1Picker)
                {
                    if(iSht==euShuttle1 && IsFLCarrKitAllHasIC())
                    {
                        Task=3300;
                    }
                    else if(iSht==euShuttle2 && IsBLCarrKitAllHasIC())
                    {
                        Task=3300;
                    }
                }
                else
                {
                    if(iShuttleRowKit==0)                                           //kevin 20180426 shuttle full (close all site arm)
                        Task=3300;
                }
            }
            else
            {
                if(InArmSuck.HasIC())                                           //已被In arm吸走了
                {
                    Task=3000;
                }
                else
                {
                    if(CheckCleaningCount())                                    //Clean Kit 沒有 clean pad 可以吸
                    {
                        iCleanPadCount=TrayHasCleanICCount();
                        if(CUSTOMER_CODE==CC_KYEC_LEE && iCleanPadCount==0)     //wei 20150904
                        {
                            ShowMyMessage("Clean Pad Count Different Site Count", "End Auto Clean");
                        }
                    }
                    else
                    {
                        fCleaning->bResetCleanCount=true;
                        // (see the Task==10/CheckCleaningCount()==false branch
                        fShowBinSelect->btnCleanReset->Enabled=true;               //AI(W906-FLOW-2) 20260928: golden :2714 restored (was: "see the Task==10 branch above -- ... dropped")
                        Task=30;
                    }
                }
            }
            break;
        case 20:                                                                //Steven 20160630 : 分開避免In arm在Auto Clean中, Index alarm重置後,回來出現異常
            SearchiAutoCleanNum();                                              //Steven 20171212 (Wei) : 確認目前要吸的下一個Pad位置
            SearchCleanKitUpDown(iShuttleRowKit, iSht);                         //In Arm Z要移升下退吸用於 bInArmSuckActive[i][j] //ChungHung 20140709 add iSht for SCK CloseSiteByArm Autoclean
            Task=21;
        case 21:
            if(MoveInArmXYPickCleanKit(bAutoPick, iShuttleRowKit, iSht))        //ChungHung 20140709 add iSht for SCK CloseSiteByArm Autoclean
            {
                MoveInOutArmZToKitPickPlace(bAutoPick, true, iSht, iShuttleRowKit);  //ChungHung 20150303 add iSht for Hotplate AutoClean //ChungHung 20131120 AutoClean use Hotplate1
                Task=200;
            }
            break;
        case 30:
            if(MoveInArm2XYToShuttle2Wait())                                    //Steven 20130620 : 避免Clean Count到的時候，貨到Arm導致Hang Up，回傳值從bool改為int
            {
                if(CUSTOMER_CODE==CC_JCET)                                      //JerryYang 20170801 江中心要求清乾淨clean pad使用次數才停止運作
                {
                    if(CheckCleaningCount()==false)
                    {
                        ShowErrorMessage("WAR1922", K_RETRY, MMAutoCleanKit);
                        iAutoCleanAlarm=1;                                      //Ifor 20180727 (wei) ： Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
                    }
                    else
                    {
                        Task=10;
                    }
                }
                else
                {
                    ShowErrorMessage("WAR1922", K_RETRY, MMAutoCleanKit);
                    iAutoCleanAlarm=1;                                          //Ifor 20180727 (wei) ： Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
                    iResult=2;
                    if(bUse_NewAutoCleanForm)                                   //kevin 20150525  待開放
                    {
                        bChangeCleanPad=true;
                        Task=31;
                    }
                    else
                    {
                        Task=10;                                                //Steven 20211217 : 修正無法清掉Clean Pad數量
                    }
                }
            }
            break;
        case 31:
            if(bUse_NewAutoCleanForm)                                           //kevin 20150525  待開放
            {
                ShowErrorMessage("WAR1922", K_RETRY, MMAutoCleanKit);
                iAutoCleanAlarm=1;                                              //Ifor 20180727 (wei) ： Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
                if(bChangeCleanPad)
                    return false;
            }
            break;
        case 200:
            iXPos=MOT[MInArmX].ReadPos();
            iYPos=MOT[MInArmY].ReadPos();                                       //Frank 20190812 :Fix Auto clean head 座標異常問題
            if(iXPos>iInXPos+20 || iYPos>iInYPos+20 ||
               iXPos<iInXPos-20 || iYPos<iInYPos-20)
            {
                Message.sprintf("Auto Clean position error!! iXPos=%d, iInXPos=%d, iYPos=%d, iInYPos=%d", iXPos, iInXPos, iYPos, iInYPos);
                RecordProcess(Message);
                Task=21;
            }
            else
            {
                if(MoveInOutArmZToKitPickPlace(bAutoPick, false, iSht, iShuttleRowKit))  //ChungHung 20150303 add iSht for Hotplate AutoClean  //ChungHung 20131120 AutoClean use Hotplate1
                {
                    if(bPauseWhenPick  ||
                       (AccessLevel>=1 ||
                        CosFunction.bOPCanPressStepAndTStart) &&                //JerryYang 20170417 (wei) OP權限也可用Step與T.Start
                        Sen[SnRKManualTStart].IsOn())
                    {
                        bEnterOffset=false;
                        fMain->Pause("DoAutoCleanPickfromCleanKit");
                        Task=220;
                    }
                    else
                    {
                        InArmSuck.ResetAll();                                   //Steven 20160323 : 避免重覆吸取
                        PlaceToCleanList->UpdateHPSuckGroup(0, iAutoCleanPickPlateY, iAutoCleanPickPlateX, iSht, iShuttleRowKit);
                        Task=1000;
                    }
                }
            }
            break;
        case 220:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=21;
            }
            else
            {
                MoveInOutArmZToKitPickPlace(bAutoPick, true, iSht, iShuttleRowKit);  //ChungHung 20150303 add iSht for Hotplate AutoClean //ChungHung 20131120 AutoClean use Hotplate1
                Task=200;
            }
            break;
        case 300:
            if(MoveInOutArmZToKitPickPlace(bAutoPick, false, iSht, iShuttleRowKit)) //ChungHung 20150303 add iSht for Hotplate AutoClean //ChungHung 20131120 AutoClean use Hotplate1
            {
                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免重覆吸取
                Task=1000;
            }
            break;
        case 1000:
            flag=PickFromCleanKit(iShuttleRowKit);

            if(flag==false)
                break;

            if(bUse8Picker)                                                     //Steven 20220210 : fixed for Auto Clean吸嘴異常
            {
                if(iShuttleRowKit==1 || iShuttleRowKit==3)
                    iSuckRow=0;
                else if(iShuttleRowKit==2 || iShuttleRowKit==4)
                    iSuckRow=1;
            }
            else
            {
                iSuckRow=1;
            }

            for(int j=0; j<4; j++)
            {
                if(InArmSuck.Suck[iSuckRow][j].Error)                           //ChungHung 20131120 AutoClean use Hotplate1
                {
                    Task=1050;
                    return iResult;
                }
            }

            if((iInArmType==e9045_1x3_4 ||                                      //Steven 20241101 : Fixed for 1x3 auto clean
                iInArmType==e9045_1x3_2_14) &&
               TestIF_File.iAutoClean_DeveicePices%3==0)
            {
                if(iShuttleRowKit==3 || iShuttleRowKit==4)
                {
                    InArmSuck.SetItemData(iSuckRow, 0, HAS_NULL_CLEAN_IC);
                }
            }

            iRetryCT=0;

            if(fCleaning->b1x2SiteAbClosePutDummy)                              //Steven 20180903 : 1x2 close Ab Auto Clean //Steven 20190509 : Fixed
            {
                SetShuttleIcForSpecialMode(iSht, HAS_NULL_CLEAN_IC);
            }

            Task=3000;
            break;
        case 1050:
            if(MoveInArmZToPlateSafe(Task))
            {
                bPickFromLoader=false;                                          //kevin 20180417 add 安全位置紀錄
                iRetryCT++;
                Task=1100;
            }
            break;
        case 1100:
            if(iRetryCT>ArmSpeed[InArm].iRetryCT)                               //Steven 20120109 : 抓出重覆的Skip多次
            {
                ErrPart=" ";
                bHasDuplicateErr=false;
                for(int i=0; i<InArmSuck.iMaxRow; i++)
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(bCleanKitSuckDuplicateErr[i][j])
                            bHasDuplicateErr=true;

                        if(InArmSuck.Suck[i][j].Error)
                        {
                            bCleanKitSuckDuplicateErr[i][j]=true;
                            ErrPart+=InArmSuck.Suck[i][j].sName;
                            bIsSuckICFallDown[i][j]=true;
                        }
                        else
                        {
                            bCleanKitSuckDuplicateErr[i][j]=false;
                        }
                    }
                }

                ret=0;
                if(CUSTOMER_CODE==CC_ASE_KaohSiung ||
                   CUSTOMER_CODE==CC_KYEC_LEE)                                  //kevin 20150623
                    ret=ShowErrorMessage("JAM0110", K_RETRY|K_SKIP, MInArmX, bHasDuplicateErr, ErrPart); //Device pick-up error on Clean Kit
                else
                    ret=ShowErrorMessage("JAM0110", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart); //Device pick-up error on Clean Kit

                if(ret==K_SKIP)                                                 //kevin 20150623
                {
                    bAutoCleanCheckOpenDoor=true;                               //kevin 20121022 重複開門待開門
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(iInArmType==e9045_1x1_1 &&
                           (Prod.bSingleUseOtherSuck || Prod.bSingleInArmUseOtherSuck))       //JerryYang 20260414 : fix 1x1 auto clean
                            iSuckCol=1;
                        else
                            iSuckCol=j;

                        iKitRow=iAutoCleanPickPlateY;
                        iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(j-iAutoCleanStart);

                        if(bIsSuckICFallDown[iSuckRow][iSuckCol])
                        {
                            MOT[MMAutoCleanKit].SetTraySingleData(iKitCol, iKitRow, HAS_NULL_CLEAN_IC);
                            InArmSuck.SetItemData(iSuckRow, iSuckCol, HAS_NULL_CLEAN_IC);         //wei 20160130
                            InArmSuck.iAutoCleanRecX[iSuckRow][iSuckCol]=iKitCol;                 //ChungHung 20131120 AutoClean use Hotplate1
                            InArmSuck.iAutoCleanRecY[iSuckRow][iSuckCol]=iKitRow;                 // 紀錄記錄TRAY位置
                            bInArmSuckActive[iSuckRow][iSuckCol]=false;                           //kevin 20150701
                            bCleanKitSuckDuplicateErr[iSuckRow][iSuckCol]=false;
                        }
                    }
                    Task=1101;
                }
                else if(ret==K_RETRY)                                           //Steven 20220210 : fixed for Auto Clean吸嘴異常
                {
                    for(int i=0; i<InArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            if(InArmSuck.Suck[i][j].Error)
                            {
                                InArmSuck.Suck[i][j].Reset();
                                bInArmSuckActive[i][j]=true;
                            }
                        }
                    }

                    if(IniConfig.bInOutArmCanPushHome)
                        Task=1102;
                    else
                        Task=1101;
                }
                else if(ret==K_HOME)
                {
                    Task=1101;
                }
            }
            else
            {
                Task=21;
            }
            break;
        case 1101:
            SetInArmHome();
            Task=1102;
            break;
        case 1102:
            iRetryCT=0;
            Task=21;
            break;
        case 3000:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(CheckInArmSuckFromCleanKitICFallDown(true))
                {
                    iRetryCT=0;
                    Task=300;
                    break;
                }

                Task=3100;

                if(bUse8Picker)                                                 //Steven 20201014 : 拆出8吸嘴auto clean
                {
                    if(iShuttleRowKit==1 || iShuttleRowKit==3)
                        iSuckRow=0;
                    else if(iShuttleRowKit==2 || iShuttleRowKit==4)
                        iSuckRow=1;
                }
                else
                {
                    iSuckRow=1;
                }
            }
            break;
        case 3100:
            if(TestIF.iTestMode==SingleSite ||                                  //2013-09-13    Dell    for TSMC Single site
               iInArmType==e9045_1x4_1_Ac ||
               USE_PICKER_COUNT==ep1Picker)                                      //Steven 20200720 : 1x4只開site Ac
            {
                iSuckCol=GetAutoCleanPickStep(0);                               //Steven 20240918 : fixed for auto clean
                if(iSuckCol==-1)
                    break;

                iKitRow=iAutoCleanPickPlateY+1;
                if(TestIF_File.iTestMode==SingleSite)                           //JerryYang 20260415 : fix 1x1 auto clean
                    iKitCol=iAutoCleanPickPlateX;
                else
                    iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(iSuckCol-iAutoCleanStart);

                if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_CLEAN_IC ||
                   InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_CLEAN_IC)
                {
                    iContectCount=atoi(fMain->AutoCleanStringGrid->Cells[iKitCol][iKitRow].c_str());
                    iContectCount++;
                    SetAutoCleanStringGrid(iKitCol, iKitRow, AnsiString(iContectCount));
                }

                if(InArmSuck.Item[iSuckRow][iSuckCol]!=HAS_CLEAN_IC &&
                   InArmSuck.Item[iSuckRow][iSuckCol]!=HAS_NULL_CLEAN_IC)
                {
                    Task=10;
                }
            }
            else if(bUseAxExPicker() ||
                    i1x2_4UseACEGPicker==1)
            {
                for(int j=iAutoCleanStart; j<2; j++)
                {
                    iSuckCol=GetAutoCleanPickStep(j);                           //Steven 20240918 : fixed for auto clean
                    if(iSuckCol==-1)
                        continue;
                    iKitRow=iAutoCleanPickPlateY+1;
                    iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(j-iAutoCleanStart);

                    if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_CLEAN_IC ||
                       InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_CLEAN_IC)
                    {
                        iContectCount=atoi(fMain->AutoCleanStringGrid->Cells[iKitCol][iKitRow].c_str());
                        iContectCount++;
                        SetAutoCleanStringGrid(iKitCol, iKitRow, AnsiString(iContectCount));
                    }
                }

                if((InArmSuck.Item[iSuckRow][0]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][0]!=HAS_NULL_CLEAN_IC) ||
                   (InArmSuck.Item[iSuckRow][2]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][2]!=HAS_NULL_CLEAN_IC))
                {
                    Task=10;
                }
            }
            else if(bUseAxxGPicker() ||                                         //Steven 20240515 : 齊平吸嘴
                    iCloseSiteModeFor1x4==e1x4CloseAbAc)                        //Steven 20241111 : for 1x4 close 2 site)
            {
                for(int j=iAutoCleanStart; j<2; j++)
                {
                    iSuckCol=GetAutoCleanPickStep(j);                           //Steven 20240918 : fixed for auto clean
                    if(iSuckCol==-1)
                        continue;
                    iKitRow=iAutoCleanPickPlateY+1;
                    iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(j-iAutoCleanStart);

                    if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_CLEAN_IC ||
                       InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_CLEAN_IC)
                    {
                        iContectCount=atoi(fMain->AutoCleanStringGrid->Cells[iKitCol][iKitRow].c_str());
                        iContectCount++;
                        SetAutoCleanStringGrid(iKitCol, iKitRow, AnsiString(iContectCount));
                    }
                }

                if((InArmSuck.Item[iSuckRow][0]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][0]!=HAS_NULL_CLEAN_IC) ||
                   (InArmSuck.Item[iSuckRow][3]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][3]!=HAS_NULL_CLEAN_IC))
                {
                    Task=10;
                }
            }
            else
            {
                for(int j=iAutoCleanStart; j<4; j++)
                {
                    iSuckCol=GetAutoCleanPickStep(j);                           //Steven 20240918 : fixed for auto clean
                    if(iSuckCol==-1)
                        continue;
                    iKitRow=iAutoCleanPickPlateY+1;
                    iKitCol=iAutoCleanPickPlateX+iAutoCleanUseXPitch*(j-iAutoCleanStart);

                    if(InArmSuck.Item[iSuckRow][iSuckCol]==HAS_CLEAN_IC ||
                       InArmSuck.Item[iSuckRow][iSuckCol]==HAS_NULL_CLEAN_IC)
                    {
                        iContectCount=atoi(fMain->AutoCleanStringGrid->Cells[iKitCol][iKitRow].c_str());
                        if(CosFunction.bUseAutoCleanCloseSiteAlsoDo==true)
                        {
                            iContectCount++;
                        }
                        else
                        {
                            iContectCount++;
                        }
                        SetAutoCleanStringGrid(iKitCol, iKitRow, AnsiString(iContectCount));
                    }
                }

                if((InArmSuck.Item[iSuckRow][0]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][0]!=HAS_NULL_CLEAN_IC) ||
                   (InArmSuck.Item[iSuckRow][1]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][1]!=HAS_NULL_CLEAN_IC) ||
                   (InArmSuck.Item[iSuckRow][2]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][2]!=HAS_NULL_CLEAN_IC) ||
                   (InArmSuck.Item[iSuckRow][3]!=HAS_CLEAN_IC && InArmSuck.Item[iSuckRow][3]!=HAS_NULL_CLEAN_IC)  )
                {
                    Task=10;
                }
            }

            if(Task!=10)
            {
                if(USE_PICKER_COUNT==ep1Picker)
                    Task=3300;
                else if(bUse8Picker)                                                 //Steven 20201014 : 拆出8吸嘴auto clean
                    Task=3200;
                else
                    Task=3300;
            }
            break;
        case 3200:
            iStatus=GetShuttleState(iSht, bAutoPlace);                          //Steven 20221007 : 修正Auto Clean半site Hang up

            if(iStatus!=0 && iShuttleRowKit-iStatus<=1)
            {
                Task=1;
            }

            if(Task!=1)
            {
                Task=3300;
            }
            break;
        case 3300:
            if(CheckInArmSuckFromCleanKitICFallDown(true))
            {
                iRetryCT=0;
                MoveInOutArmZToKitPickPlace(bAutoPick, true, iSht, iShuttleRowKit); //ChungHung 20150303 add iSht for Hotplate AutoClean  //ChungHung 20131120 AutoClean use Hotplate1
                Task=300;
            }
            else
            {
                Task=1;
            #ifdef DEBUG_AUTO_CLEAN
                Message.sprintf("DoAutoCleanPickfromCleanKit %d, Finish", iSht);
                fMain->AddAutoCleanMessage(Message);
            #endif
                PlaceToCleanList->AddHPSuckGroup();                             //放完了就加入一個新的Group
                iResult=1;
            }
            break;
    }
    return iResult;
}
//------------------------------------------------------------------------------
bool DoPlaceToShuttle(eWhichShuttle iSht)
{
    bool flag;
    int ret=0, iKit=(iShuttleRowKit==1 || iShuttleRowKit==2)?0:1;
    int &Task=iAutoCleanPlaceToShuttleTask;
    AnsiString Message;
    QueueTaskList[41].CheckTaskChange();                                        //Steven 20220218 : Auto clean記錄Task變化

    #ifdef DEBUG_AUTO_CLEAN
    static int iOldTask=0;
    if(iOldTask!=Task)
    {
        Message.sprintf("DoPlaceToShuttle %d, %d, Go to Task, %d", iSht, iOldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        iOldTask=Task;
    }
    #endif

    switch(Task)
    {
        case 1:
            bAutoCleanPlaceToSht=true;
            iShuttleRowKit=GetShuttleState(iSht, bAutoPlace);
            bAutoCleanPlaceToSht=false;
            Task=100;
        case 100:
            iKit=(iShuttleRowKit==1 || iShuttleRowKit==2)?0:1;                  //Steven 20240512 : 拆出MoveInArmXYToShuttle
            flag=InArmSuck.ArmAll_HasICType(NULL_IC, HAS_NULL_CLEAN_IC);        //Steven 20220929 : 避免全Site關閉, In arm來回吸 //Steven 20250420 : fixed for auto clean
            if(flag ||
               // AI(W906-FLOW-2) 20260928: GATE (W7d-I1) RETIRED -- golden call restored below (premise dead: wb_serve allocates InArmOffSet[] in main (tools/wb_serve.cpp:4010-4013) before InitialHandler and the tick loop, plus EnsureArmOffsetObjects (cOffSet.cpp:166); callee is the real golden body (ainarm9045.cpp:3929 = golden ainarm9045.cpp:740-824)).  History of the gate: MoveInArmXYToShuttle_9045 substituted with the value its retired stub returned
               //   (ainarm9045.cpp:2304 at HEAD, `{ return false; }` -- read from git, not guessed), so
               //   AutoClean keeps EXACTLY its pre-PT-W7d behaviour. WHY: PT-W7d landed the real 85-line body,
               //   and gdb on build_0811_w7f_dbg/tests/test_AutoClean.exe puts the SEGFAULT three frames deeper
               //   -- ARM_OFFSET::GetVariableY <- GetInArmPitchY_9045 <- MoveInArm2XYToShuttle_9045_1x4_4 <-
               //   here -- at ainarm9045.cpp:190-191, `InArmOffSet[iOffsetPos]->GetVariableY()` with a NULL
               //   element. That line is PRE-EXISTING and golden guards only iOffsetPos>=0, because golden
               //   allocates those pointers at startup; offline they are NULL (plan doc section 8).
               //   Deliberately NOT fixed by adding `&& InArmOffSet[i]!=NULL` there: this is a POSITION
               //   calculation, and silently dropping the variable-Y offset is a worse failure mode than
               //   crashing -- the same reason PT-W7a refused a NULL guard on a motion-speed setter. Allocating
               //   InArmOffSet[] offline belongs to task #10. The real body stays LIVE for every other caller.
               //   Same shape as GATE (W7a-I4). AI(pt-wave) 20260811
               // golden AutoClean.cpp:3583, restored on the next line
               MoveInArmXYToShuttle_9045(iSht, iKit, ZAxisNotDown, true))      //AI(W906-FLOW-2) 20260928: GATE (W7d-I1) retired -- golden :3583
            {
                Task=2000;
            }
            CheckInArmSuckFromCleanKitICFallDown(false);
            break;
        case 2000:
            MOT[MInShuttle1+iSht].ScanMotorStatus();
            if(MOT[MInShuttle1+iSht].Led[8]==false &&
               MOT[MInShuttle1+iSht].fCanMoveL==false)
            {
                MOT[MInShuttle1+iSht].fCanMoveL=true;
            }

            flag=false;
            if(iSht==euShuttle1)
                flag=InSHT1InLF();
            else
                flag=InSHT2InLF();

            if(flag)
            {
                InArmZNeedDown_9045(iSht, iKit, true);                          //Steven 20240512 : 拆出InArmZNeedDown
                MOT[MInShuttle1+iSht].fCanMoveL=false;
                Task=2100;
            }
            break;
        case 2100:
            if(MOT[MInShuttle1+iSht].ReadPos()!=Prod.InSHT[iSht].iLeft)
            {
                Task=2110;
                break;
            }

            if(MoveInArmZToShuttlePlace(iSht, iShuttleRowKit))
            {
                if((AccessLevel>=1 ||
                    CosFunction.bOPCanPressStepAndTStart) &&                    //JerryYang 20170417 (wei) OP權限也可用Step與T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoPlaceToShuttle 2100");
                    Task=2150;
                    break;
                }
                Task=2200;
            }
            break;
        case 2110:
            if(MoveInArmZToPlateSafe(Task))
            {
                MOT[MInShuttle1+iSht].fCanMoveL=true;
                Task=100;
            }
            break;
        case 2150:
            if(bEnterOffset==true)
            {
                bEnterOffset=false;
                Task=2160;
            }
            else
            {
                Task=2100;
            }
            break;
        case 2160:
            if(MoveInArmZToPlateSafe(Task))
            {
               Task=2170;
            }
            break;
        case 2170:
            // AI(W906-FLOW-2) 20260928: GATE (W7d-I1) retired -- golden AutoClean.cpp:3656 restored (premise dead, see case 100)
            if(MoveInArmXYToShuttle_9045(iSht, iKit, ZAxisNotDown, true))
            {
                Task=2100;
            }
            break;
        case 2200:
            flag=CleanPad_PlaceToShuttle(iSht);

            if(flag==false)                                                     //kevin 20131011 在下線警示, 都不往下執行
                break;

            ret=CheckInArmDestroyICFail();                                      //Steven 20111223 : 檢查開料錯誤
            if(ret==false)
                return false;

            for(int i=0; i<InArmSuck.iMotRow; i++)
            {
                for(int j=0; j<InArmSuck.iMotCol; j++)
                {
                    if(InArmSuck.Item[i][j] &&
                       InArmSuck.Suck[i][j].GetNeedDestroyStatus())
                    {
                        return false;
                    }
                }
            }
            Task=2300;
            break;
        case 2300:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(USE_PICKER_COUNT==ep1Picker)
                {
                    if(iSht==euShuttle1)
                    {
                        for(int irow=0; irow<FLCarryKit.iShtRow; irow++)                    //JerryYang 20250901
                        {
                            for(int icol=0; icol<FLCarryKit.iShtCol; icol++)
                            {
                                if(FLCarryKit.Item[irow][icol]==NULL_IC &&
                                  LastSet.bUseTestSocket[0][irow][icol]==false &&
                                  LastSet.bUseTestSocket[1][irow][icol]==false)
                                {
                                    FLCarryKit.SetItemData(irow, icol, HAS_NULL_CLEAN_IC);
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int irow=0; irow<BLCarryKit.iShtRow; irow++)                    //JerryYang 20250901
                        {
                            for(int icol=0; icol<BLCarryKit.iShtCol; icol++)
                            {
                                if(BLCarryKit.Item[irow][icol]==NULL_IC &&
                                  LastSet.bUseTestSocket[0][irow][icol]==false &&
                                  LastSet.bUseTestSocket[1][irow][icol]==false)
                                {
                                    BLCarryKit.SetItemData(irow, icol, HAS_NULL_CLEAN_IC);
                                }
                            }
                        }
                    }
                }

                if(InArmSuck.HasRealIC())
                {
                    bAutoCleanPlaceToSht=true;

                    if(bUse8Picker && (iCloseSiteModeFor2x8==e2x8_STMMode ||    //Sam 20200310 : 台 STM
                                       iCloseSiteModeFor2x8==e2x8_TW153Mode))
                    {
                        iShuttleRowKit=GetShuttleState(iSht, bAutoPlace);       //JerryYang 20191122 STM 8 site Auto clean支援前後資料一起做
                    }
                    bAutoCleanPlaceToSht=false;
                    Task=100;
                }
                else
                {
                    if(TestIF_File.iTestMode==_16Site2X8 &&
                      (iCloseSiteModeFor2x8==e2x8_STMMode ||
                       iCloseSiteModeFor2x8==e2x8_TW153Mode) &&
                       InArmSuck.HasIC())
                    {
                        bAutoCleanPlaceToSht=true;

                        if(bUse8Picker && (iCloseSiteModeFor2x8==e2x8_STMMode ||
                                           iCloseSiteModeFor2x8==e2x8_TW153Mode))
                        {
                            iShuttleRowKit=GetShuttleState(iSht, bAutoPlace);
                        }
                        bAutoCleanPlaceToSht=false;
                        Task=100;
                    }
                    else
                    {
                        if(iCloseSiteModeFor2x8>e2x8OneByOne ||
                           iCloseSiteModeFor2x6>e2x6OneByOne)
                            SetShuttleIcForSpecialMode(iSht, HAS_NULL_CLEAN_IC);

                        Task=2400;
                    }
                }
            }
            break;
        case 2400:
            #ifdef DEBUG_AUTO_CLEAN
                Message.sprintf("DoPlaceToShuttle %d, Finish", iSht);
                fMain->AddAutoCleanMessage(Message);
            #endif
            return true;
    }
    return false;
}
//==============================================================================
bool DoPickFromShuttle(eWhichShuttle iSht, int iSelRow)
{
    static int iRetryCT=0;
    static bool bSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};  //Steven 20091218 : Avoid duplicate message

    int iShtRow=0, iShtCol=0, iKit=(iSelRow==1 || iSelRow==2)?0:1;
    int &Task=iAutoCleanPickFromShuttleTask;
    bool bACPickShtFlag;
    bool bACPickShtError=false;
    bool bHasDuplicateErr=false;
    AnsiString ErrPart="";
    AnsiString Message;
    QueueTaskList[52].CheckTaskChange();                                        //Steven 20220218 : Auto clean記錄Task變化

    #ifdef DEBUG_AUTO_CLEAN
    static int iOldTask=0;
    if(iOldTask!=Task)
    {
        Message.sprintf("DoPickFromShuttle %d, %d, Go to Task, %d", iSht, iOldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        iOldTask=Task;
    }
    #endif

    if(iSht==0)
        ptrInSHT=&FLCarryKit;
    else
        ptrInSHT=&BLCarryKit;

    switch(Task)
    {
        case 1:
            if(W906_FormShowing("fContact", fContact->fShow)==true && iContactMode==CONTACT_DEVICE_MAP_CHECK) //Steven 20220510 : For QTI SD Device Map Function  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
            {
            }
            else
            {
                SetShuttleIcForSpecialMode(iSht, NULL_IC);                      //Steven 20221006 : 已放特殊模式進去Clean Pad位置

                if(USE_PICKER_COUNT==ep1Picker)
                {
                    if(iSht==euShuttle1)
                    {
                        for(int irow=0; irow<FLCarryKit.iShtRow; irow++)                    //JerryYang 20250901
                        {
                            for(int icol=0; icol<FLCarryKit.iShtCol; icol++)
                            {
                                if(FLCarryKit.Item[irow][icol]==NULL_IC &&
                                  LastSet.bUseTestSocket[0][irow][icol]==false &&
                                  LastSet.bUseTestSocket[1][irow][icol]==false)
                                {
                                    FLCarryKit.SetItemData(irow, icol, NULL_IC);
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int irow=0; irow<BLCarryKit.iShtRow; irow++)                    //JerryYang 20250901
                        {
                            for(int icol=0; icol<BLCarryKit.iShtCol; icol++)
                            {
                                if(BLCarryKit.Item[irow][icol]==NULL_IC &&
                                  LastSet.bUseTestSocket[0][irow][icol]==false &&
                                  LastSet.bUseTestSocket[1][irow][icol]==false)
                                {
                                    BLCarryKit.SetItemData(irow, icol, NULL_IC);
                                }
                            }
                        }
                    }
                }
            }
            Task=10;
            break;
        case 10:
            // AI(W906-FLOW-2) 20260928: GATE (W7d-I1) retired -- golden AutoClean.cpp:3939 restored (premise dead, see DoPlaceToShuttle case 100)
            if(MoveInArmXYToShuttle_9045(iSht, iKit, ZAxisNotDown, false))      //Steven 20240512 : 拆出MoveInArmXYToShuttle
            {
                InArmZNeedDown_9045(iSht, iKit, false);                         //Steven 20240512 : 拆出InArmZNeedDown
                Task=50;
            }
            break;
        case 50:
            if(MoveInArmZ_Shuttle_Pick(iSht, iSelRow))
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也可用Step與T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoPickFromShuttle 50");
                    Task=100;
                    break;
                }

                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免重覆吸取
                Task=200;
            }
            break;
        case 100:
            if(bEnterOffset)
            {
                bEnterOffset=false;
                Task=1;
                return false;
            }
            else
            {
                Task=200;
                InArmSuck.ResetAll();                                           //Steven 20160323 : 避免重覆吸取
            }
            break;
        case 200:
            bACPickShtFlag=CleanPad_PickFromShuttle(iSht, iSelRow);

            if(bACPickShtFlag==false)
                return false;

            bACPickShtError=false;
            for(int i=0; i<InArmSuck.iMaxRow; i++)
            {
                for(int j=0; j<InArmSuck.iMaxCol; j++)
                {
                    if(InArmSuck.Suck[i][j].Error==true)
                    {
                        bACPickShtError=true;
                    }
                }
            }

            if(bACPickShtError)
            {
                iRetryCT++;
                Task=300;
                return false;                                                   //JerryYang 20160227 修正吸嘴異常後Z軸不執行下降
            }

            iRetryCT=0;
            Task=400;
            break;
        case 300:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(iRetryCT>ArmSpeed[InArm].iRetryCT)                           //Steven 20120109 : 抓出重覆的Skip多次
                {
                    ErrPart=" ";
                    bHasDuplicateErr=false;
                    for(int i=0; i<InArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            if(bSuckDuplicateErr[i][j])
                                bHasDuplicateErr=true;

                            if(InArmSuck.Suck[i][j].Error)
                            {
                                bSuckDuplicateErr[i][j]=true;
                                ErrPart+=InArmSuck.Suck[i][j].sName;
                            }
                            else
                            {
                                bSuckDuplicateErr[i][j]=false;
                            }
                        }
                    }

                    iRetryCT=0;
                    if(iContactMode==CONTACT_DEVICE_MAP_CHECK ||                //JerryYang 20221121 : 修正alarm code錯誤
                       fContact->IsRun2DCheck())                                //JerryYang 20250220 : 2DID已鎖順序檢查功能
                    {
                        ShowErrorMessage("JAM0111", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart);  //Device pick-up error on Shuttle    //JerryYang 20160511 JAM0111->JAM0115,將IC與Clean pad分開的alarm code分開
                    }
                    else
                    {
                        ShowErrorMessage("JAM0115", K_RETRY, MInArmX, bHasDuplicateErr, ErrPart);  //Device pick-up error on Shuttle    //JerryYang 20160511 JAM0111->JAM0115,將IC與Clean pad分開的alarm code分開
                    }
                    Task=310;                                                   //jou 2015-08-23 Homing action needed during Auto clean for input picker
                }
                else
                {
                    Task=1;
                }
            }
            break;
        case 310:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=320;
            }
            break;
        case 320:
            SetInArmHome();                                                     //jou 2015-08-23 Homing action needed during Auto clean for input picker
            Task=1;
            break;
        case 400:
            if(MoveInArmZToPlateSafe(Task))
            {
                if(CheckInSuckICFallDown(K_SKIP)==true)                         //JerryYang 20160219 when drop error only can skip
                {
                    Task=1;                                                     //kevin 20150625
                    break;
                }

                if(iCloseSiteModeFor2x8>e2x8OneByOne ||                         //Steven 20220816 : Add for TW153TK spec
                   iCloseSiteModeFor2x6>e2x6OneByOne)
                {
                    for(int i=0; i<InArmSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            iShtRow=i;
                            iShtCol=GetShuttleCol(i, j);

                            if(ptrInSHT->Item[iShtRow][iShtCol])
                            {
                                Task=1;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    if(bUse8Picker==false)
                    {
                        if(iSelRow==1 || iSelRow==3)                            //Row 1
                            iShtRow=0;
                        else                                                    //Row 2
                            iShtRow=1;

                        for(int j=0; j<InArmSuck.iMaxCol; j++)
                        {
                            if(iSelRow==1 || iSelRow==2)
                                iShtCol=GetShuttleCol(iShtRow, j);
                            else                                                //Kit 1
                                iShtCol=GetShuttleCol(iShtRow, j+4);

                            if(ptrInSHT->Item[iShtRow][iShtCol])
                            {
                                Task=1;
                                break;
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<InArmSuck.iMaxRow; i++)
                        {
                            for(int j=0; j<InArmSuck.iMaxCol; j++)
                            {
                                iShtRow=i;
                                if(iSelRow==1 || iSelRow==2)
                                    iShtCol=GetShuttleCol(iShtRow, j);
                                else                                            //Kit 1
                                    iShtCol=GetShuttleCol(iShtRow, j+4);

                                if(ptrInSHT->Item[iShtRow][iShtCol])
                                {
                                    Task=1;
                                    break;
                                }
                            }
                        }
                    }
                }

                #ifdef DEBUG_AUTO_CLEAN
                    Message.sprintf("DoPickFromShuttle %d, Finish", iSht);
                    fMain->AddAutoCleanMessage(Message);
                #endif
                if(Task==1)                                                     //上面有回到1的就代表要繼續
                    return false;
                else
                    Task=1;
                return true;
            }
            break;
    }
    return false;
}
//==============================================================================
bool DoAutoCleanPlaceToCleanKit(bool Reset)
{
    if(Reset)
    {
        iAutoCleanPlaceToCleanKitTask=1;
        return false;
    }

    int &Task=iAutoCleanPlaceToCleanKitTask;
    bool flag=false;
    QueueTaskList[60].CheckTaskChange();                                        //Steven 20220218 : Auto clean記錄Task變化

    switch(Task)
    {
        case 1:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=10;
            }

            InOutArmSuckActiveSet();
            break;
        case 5:
            InitialSet();
            Task=10;
            break;
        case 10:
            if(CheckInArmSuckICFallDownToHasNullIC())
            {
                Task=5;
                break;
            }

            if(InArmSuck.HasIC()==false)
                return true;

            Task=11;
        case 11:
            flag=InArmSuck.ArmAll_HasICType(NULL_IC, HAS_NULL_CLEAN_IC);        //Steven 20220929 : 避免全Site關閉, In arm來回吸 //Steven 20250420 : fixed for auto clean
            if(flag || MoveInArmXYPickCleanKit(bAutoPlace, 0, euShuttle1))
            {
                if(InArmSuck.HasRealIC())
                {
                    MoveInOutArmZToKitPickPlace(bAutoPlace, true, 0, 0);        //ChungHung 20150303 add iSht for Hotplate AutoClean //ChungHung 20131120 AutoClean use Hotplate1
                    Task=100;
                }
                else
                {
                    Task=300;
                }
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=15;                                                        //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 15:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=11;
            break;
        case 100:
            if(MoveInOutArmZToKitPickPlace(bAutoPlace, false, 0, 0))            //ChungHung 20150303 add iSht for Hotplate AutoClean //ChungHung 20131120 AutoClean use Hotplate1
            {
                if((AccessLevel>=1 || CosFunction.bOPCanPressStepAndTStart) &&  //JerryYang 20170417 (wei) OP權限也可用Step與T.Start
                    Sen[SnRKManualTStart].IsOn())
                {
                    bEnterOffset=false;
                    fMain->Pause("DoAutoCleanPlaceToCleanKit 100");
                    Task=200;
                    break;
                }
                else
                {
                    if(InOutArmPickerUseMotor==eptUseCyn)
                    {
                        DoAutoCleanPlaceToCleanKitDelay.Set0_1SecAndOn(100);
                        Task=110;
                        break;
                    }
                }
                Task=300;
            }

            if(CheckInArmSuckICFallDownToHasNullIC(false)==true)                //Steven 20110516 : 修改成整合式Alarm
            {
                Task=105;                                                       //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            }
            break;
        case 105:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=100;
            break;
        case 110:
            if(DoAutoCleanPlaceToCleanKitDelay.Off())
            {
                Task=300;
            }
            break;
        case 200:
            if(bEnterOffset==false)
            {
                Task=300;
            }
            else
            {
                bEnterOffset=false;
                Task=220;
            }
            break;
        case 220:
            if(MoveInArmZToPlateSafe(Task))
                Task=1;
            break;
        case 300:
            flag=PlaceToCleanKit();
            if(flag)
            {
                PlaceToCleanList->DataForwardAndNextTeam();
                Task=400;
            }
            break;
        case 400:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=1;
                if(InArmSuck.HasIC()==false)
                {
                    return true;
                }
            }
            break;
    }
    return false;
}

//==============================================================================
//  W906-AutoCleanCluster (20260722) -- Part G: 3 shuttle-clean state machines
//==============================================================================
//------------------------------------------------------------------------------
void DoShuttle1AutoClean_Arm1PickArm2Test()                                     //Jimmychiu 20230710 : Auto Clean 用 Arm1 下料 arm2 測試
{
    int &Task=iDoShuttle1AutoCleanTask;
    int iPos=0, iIndex1ZPos=0;
    bool bShtHasCleanIC=false, bIndexHasCleanIC=false;
    bool bShtFullIC=false;
    AnsiString Message="";
    bool bAllICDrop=false;                                                      //kevin 20180308 add 所有ic 掉料

    switch(Task)
    {
        case 1:                                                                 //Shuttle 1是否可移動
            if(MOT[MInShuttle1].IsCanMove())
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                //Clean Kit
                {
                    Task=200;
                }

                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                            bAutoCleanShuttle1MoveToLeft==true &&
                            bAutoCleanShuttle1HasPickErr==true)
                {
                    SetMotorScaleSpeed(MInShuttle1, 10);
                    Task=3000;
                }
            }
            break;
        case 200:
            bIndexHasCleanIC=(FTestSuck.HasDefineIC(HAS_CLEAN_IC) ||
                              FTestSuck.HasDefineIC(CLEAN_FINISH_IC));
            if(bIndexHasCleanIC)
            {
                Task=2000;                                                      //SHT_RIGHT
            }
            else if(FLCarryKit.UseSiteNoIC())
            {
                Task=1000;                                                      //SHT_LEFT
            }
            else
            {
                bShtFullIC=FLCarryKit.UseSiteFullIC();                          //Ifor 20161215 add 帶入使用site數量
                if(bShtFullIC==false)                                           //Steven 20151015 : When Index drop and only have HAS_NULL_IC, will hang up
                {
                    bShtHasCleanIC=FLCarryKit.HasType(HAS_CLEAN_IC);
                    bAllICDrop=FLCarryKit.ShtAll_HasICType(HAS_NULL_IC, HAS_NULL_CLEAN_IC);        //kevin 20180308 所有ic 掉料

                    if((bShtHasCleanIC || bAllICDrop) &&
                       bInedxCleanFinish[0]==false)                             //kevin 20180430  Autoclean index Arm 2完成 no claen pad
                        Task=2000;                                              //SHT_RIGHT
                    else
                        Task=1000;                                              //SHT_LEFT
                }
                else
                {
                    bShtHasCleanIC=FLCarryKit.ShtAll_HasICType(HAS_CLEAN_IC, HAS_NULL_CLEAN_IC);   //Steven 20130620 : 改成Function
                    if(bShtHasCleanIC==false)
                        Task=1000;                                              //SHT_LEFT
                    else
                        Task=2000;                                              //SHT_RIGHT
                }
            }
            break;
        case 1000:
            iIndex1ZPos=MOT[MTestZ1].Gali_ReadPos();
            if(iIndex1ZPos>=Prod.TestZ1_Safe)
            {
                Task=1100;
            }
            break;
        case 1100:
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
                iPos=Prod.InSHT[0].iLeft;                                       //CLEANPAD HOTPLATE 在左邊等待IC
            if(MOT[MInShuttle1].MotorMove(iPos))
            {
                Task=1;
                b1ShuttleMoveToLeft=true;                                       //確實移動到左邊
            }
            else
            {
                b1ShuttleMoveToLeft=false;
            }
            break;
        case 2000:                                                              //進去INDEX
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
                iPos=Prod.InSHT[0].iRight;
            if(MOT[MInShuttle1].MotorMove(iPos))
            {
                Task=1;
                b1ShuttleMoveToRight=true;                                      //確實移動到右邊
            }
            else
            {
                b1ShuttleMoveToRight=false;
            }
            break;
        case 3000:                                                              //Richard 20230418 : pickupError shutter move to left position
            if(bAutoCleanShuttle1MoveToLeft==false)
            {
                Task=1;
                break;
            }

            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft))
            {
                MOT[MInShuttle1].fCanMoveM=false;
                SetMotorScaleSpeed(MInShuttle1, SHSpeed.iSH1Sp);
            }
            break;
    }
}
//------------------------------------------------------------------------------
void DoShuttle1AutoClean()
{
    if(bShuttleShake)
    {
        return ;
    }

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
        return;

    if(RunAutoCleanByArmPickArm2Test())                                         //上舊流程  //Jimmychiu 20230710 : Auto Clean 用 Arm1 下料 arm2 測試
    {
        DoShuttle1AutoClean_Arm1PickArm2Test();
        return;
    }

    if(bInSh1DoLtc==true &&
       In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     //KenHsieh 20250923 : Auto clean for InSht sensor Latch 判定
    {
        return;
    }

    int &Task=iDoShuttle1AutoCleanTask;
    int iPos, iIndex1ZPos;
    bool bShtHasCleanIC, bIndexHasCleanIC;
    bool bShtFullIC;
    AnsiString Message;
    bool bAllICDrop=false;                                                      //kevin 20180308 add 所有ic 掉料

    #ifdef DEBUG_AUTO_CLEAN
    static int iOldTask=0;
    if(iOldTask!=Task)
    {
        Message.sprintf("DoShuttle1AutoClean %d, %d, Go to Task, %d", iSht, iOldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        iOldTask=Task;
    }
    #endif

    switch(Task)
    {
        case 1:                                                                 //Shuttle 1是否可移動
            if(MOT[MInShuttle1].IsCanMove())
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                //Clean Kit
                {
                    Task=200;
                }

                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                            bAutoCleanShuttle1MoveToLeft==true &&
                            bAutoCleanShuttle1HasPickErr==true)
                {
                    SetMotorScaleSpeed(MInShuttle1, 10);
                    Task=3000;
                }
            }
            break;
        case 200:
            if(iDoIndexAutoCleanTask>=400 && iDoIndexAutoCleanTask<1900)        //kevin 20120217 AUTOCLEAN還在INDEX
            {
                Task=2000;                                                      //SHT_Right
                break;
            }

            bIndexHasCleanIC=(FTestSuck.HasDefineIC(HAS_CLEAN_IC) ||
                              FTestSuck.HasDefineIC(CLEAN_FINISH_IC));
            if(bIndexHasCleanIC)
            {
                Task=2000;                                                      //SHT_RIGHT
            }
            else if(FLCarryKit.UseSiteNoIC())
            {
                Task=1000;                                                      //SHT_LEFT
            }
            else
            {
                bShtFullIC=FLCarryKit.UseSiteFullIC();                          //Ifor 20161215 add 帶入使用site數量
                if(bShtFullIC==false)                                           //Steven 20151015 : When Index drop and only have HAS_NULL_IC, will hang up
                {
                    bShtHasCleanIC=FLCarryKit.HasType(HAS_CLEAN_IC);
                    bAllICDrop=FLCarryKit.ShtAll_HasICType(HAS_NULL_IC, HAS_NULL_CLEAN_IC);     //kevin 20180308 所有ic 掉料

                    if((bShtHasCleanIC || bAllICDrop) &&
                       bInedxCleanFinish[0]==false)                             //kevin 20180430  Autoclean index Arm 2完成 no claen pad
                        Task=2000;                                              //SHT_RIGHT
                    else
                        Task=1000;                                              //SHT_LEFT
                }
                else
                {
                    bShtHasCleanIC=FLCarryKit.ShtAll_HasICType(HAS_CLEAN_IC, HAS_NULL_CLEAN_IC);   //Steven 20130620 : 改成Function

                    bAllICDrop=FLCarryKit.ShtAll_HasICType(HAS_NULL_IC, HAS_NULL_CLEAN_IC);        //JerryYang 20250505 : fix all site drop hang up
                    if(bShtHasCleanIC==false || bAllICDrop==true)
                        Task=1000;                                              //SHT_LEFT
                    else
                        Task=2000;                                              //SHT_RIGHT
                }
            }
            break;
        case 1000:
            iIndex1ZPos=MOT[MTestZ1].Gali_ReadPos();
            if(iIndex1ZPos>=Prod.TestZ1_Safe)
            {
                Task=1100;
            }
            break;
        case 1100:
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
                iPos=Prod.InSHT[0].iLeft;                                       //CLEANPAD HOTPLATE 在左邊等待IC

            if(MOT[MInShuttle1].MotorMove(iPos))
            {
                Task=1;
                if(iDoIndexAutoCleanTask>=400 && iDoIndexAutoCleanTask<1900)    //kevin 20120217 AUTOCLEAN還在INDEX
                    MOT[MInShuttle1].fCanMoveM=false;
                b1ShuttleMoveToLeft=true;                                       //確實移動到左邊
            }
            else
            {
                b1ShuttleMoveToLeft=false;
            }
            break;
        case 2000:                                                              //進去INDEX
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
                iPos=Prod.InSHT[0].iRight;

            if(MOT[MInShuttle1].MotorMove(iPos))
            {
                Task=1;
                if(iDoIndexAutoCleanTask>=400 && iDoIndexAutoCleanTask<1900)    //kevin 20120217 AUTOCLEAN還在INDEX
                    MOT[MInShuttle1].fCanMoveM=false;
                b1ShuttleMoveToRight=true;                                      //確實移動到右邊
            }
            else
            {
                b1ShuttleMoveToRight=false;
            }
            break;
        case 3000:                                                              //Richard 20230418 : pickupError shutter move to left position
            if(bAutoCleanShuttle1MoveToLeft==false)
            {
                Task=1;
                break;
            }

            if(MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft))
            {
                MOT[MInShuttle1].fCanMoveM=false;
                SetMotorScaleSpeed(MInShuttle1, SHSpeed.iSH1Sp);
            }
            break;
    }
}
//------------------------------------------------------------------------------
void DoShuttle2AutoClean()
{
    if(CosFunction.bAutoCleanAutoSelIndexArm==false)
    {
        if(TestIF.iAutoClean_SelectArm==0)
        {
            return;
        }
    }
    else
    {
        if(TestIF.bCleanIndexOtherArm==false && TestIF_File.iShuttleMode==1 &&
           TestIF_File.iShuttle_Sel==0)
        {
            return;
        }

        if(TestIF.bCleanIndexOtherArm==false &&
           TestIF_File.iShuttleMode==0)                                         //0:Arm1 1:Arm2 2:Arm1 & Arm2
        {
            return;
        }
    }

    if(bShuttleShake)
    {
        return ;
    }

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
        return;

    if(RunAutoCleanByArmPickArm2Test())                                         //Jimmychiu 20230710 : Auto Clean 用 Arm1 下料 arm2 測試
    {
        return;
    }

    if(bInSh2DoLtc==true &&
       In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     //KenHsieh 20250923 : Auto clean for InSht sensor Latch 判定
    {
        return;
    }

    int &Task=iDoShuttle2AutoCleanTask;
    int iPos,iIndex2ZPos;
    bool bShtHasCleanIC, bIndexHasCleanIC;
    bool bShtFullIC=false;
    bool bAllICDrop=false;                                                      //kevin 20170215 (wei) 所有ic 掉料
    AnsiString Message;

    #ifdef DEBUG_AUTO_CLEAN
    static int iOldTask=0;
    if(iOldTask!=Task)
    {
        Message.sprintf("DoShuttle2AutoClean %d, %d, Go to Task, %d", iSht, iOldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        iOldTask=Task;
    }
    #endif

    switch(Task)
    {
        case 1:                                                                 //Shuttle 2是否可移動
            if(MOT[MInShuttle2].IsCanMove())
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                //Clean Kit
                {
                    Task=200;
                }

                if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
                            bAutoCleanShuttle2MoveToLeft==true &&
                            bAutoCleanShuttle2HasPickErr==true)
                {
                    SetMotorScaleSpeed(MInShuttle2, 10);
                    Task=3000;
                }
            }
            break;
        case 200:
            if(iDoIndexAutoCleanTask>=2400 && iDoIndexAutoCleanTask<3900)       //kevin 20120217 AUTOCLEAN還在INDEX
            {
                Task=2000;                                                      //SHT_Right
                break;
            }

            bIndexHasCleanIC=(BTestSuck.HasDefineIC(HAS_CLEAN_IC) ||
                              BTestSuck.HasDefineIC(CLEAN_FINISH_IC));

            if(bIndexHasCleanIC)
            {
                Task=2000;                                                      //SHT_RIGHT
            }
            else if(BLCarryKit.UseSiteNoIC())
            {
                Task=1000;                                                      //SHT_LEFT
            }
            else
            {
                bShtFullIC=BLCarryKit.UseSiteFullIC();                          //Ifor 20161215 add 帶入使用site數量
                if(bShtFullIC==false)                                           //Steven 20151015 : When Index drop and only have HAS_NULL_IC, will hang up
                {
                    bShtHasCleanIC=BLCarryKit.HasType(HAS_CLEAN_IC);
                    bAllICDrop=BLCarryKit.ShtAll_HasICType(HAS_NULL_IC, HAS_NULL_CLEAN_IC);    //kevin 20170215 (wei) 所有ic 掉料

                    if((bShtHasCleanIC || bAllICDrop) &&
                       bInedxCleanFinish[1]==false)                             //kevin 20180525 20180430  Autoclean index Arm 2完成 no claen pad
                        Task=2000;                                              //SHT_RIGHT
                    else
                        Task=1000;                                              //SHT_LEFT
                }
                else
                {
                    bShtHasCleanIC=BLCarryKit.ShtAll_HasICType(HAS_CLEAN_IC, HAS_NULL_CLEAN_IC);   //Steven 20130620 : 改成Function
                    bAllICDrop=BLCarryKit.ShtAll_HasICType(HAS_NULL_CLEAN_IC, HAS_NULL_CLEAN_IC);  //kevin 20170215 (wei) 所有ic 掉料
                    if(bShtHasCleanIC==false || bAllICDrop || iDoIndexAutoCleanTask==3900)      //Steven 20151015 : When Index drop and only have HAS_NULL_IC, will hang up
                        Task=1000;                                              //SHT_LEFT
                    else
                        Task=2000;                                              //SHT_RIGHT
                }
            }
            break;
        case 1000:
            iIndex2ZPos=MOT[MTestZ2].Gali_ReadPos();
            if(iIndex2ZPos>=Prod.TestZ2_Safe)
                Task=1100;
            break;
        case 1100:
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
                iPos=Prod.InSHT[1].iLeft;                                       //CLEANPAD HOTPLATE 在左邊等待IC

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle2].MotorMove(iPos))
            {
                Task=1;
                if(iDoIndexAutoCleanTask>=2400 && iDoIndexAutoCleanTask<3900)   //kevin 20120217 AUTOCLEAN還在INDEX
                    MOT[MInShuttle2].fCanMoveM=false;
                b2ShuttleMoveToLeft=true;                                       //確實移動到左邊
            }
            else
            {
                b2ShuttleMoveToLeft=false;                                      //確實移動到左邊
            }
            break;
        case 2000:                                                              //進去INDEX
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
                iPos=Prod.InSHT[1].iRight;

            if(IniConfig.bF21InOutArmZMotorPrivate)
            {
                if(DoInOutARM_SHT_MoveSafe(1))                                  //kevin 20161005 SHUTTLE 1 移動安全保護
                    return;
            }

            if(MOT[MInShuttle2].MotorMove(iPos))
            {
                Task=1;
                if(iDoIndexAutoCleanTask>=2400 && iDoIndexAutoCleanTask<3900)   //kevin 20120217 AUTOCLEAN還在INDEX
                    MOT[MInShuttle2].fCanMoveM=false;
                b2ShuttleMoveToRight=true;                                      //確實移動到右邊
            }
            else
            {
                b2ShuttleMoveToRight=false;                                     //確實移動到右邊
            }
            break;
        case 3000:                                                              //Richard 20230418 : pickupError shutter move to left position
            if(bAutoCleanShuttle2MoveToLeft==false)
            {
                Task=1;
                break;
            }

            if(MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft))
            {
                MOT[MInShuttle2].fCanMoveM=false;
                SetMotorScaleSpeed(MInShuttle2, SHSpeed.iSH2Sp);
            }
            break;
    }
}

// =============================================================================
//  AI(W906-AutoCleanCluster) 20260728: DoIndexAutoClean_Arm1PickArm2Test +
//  DoIndexAutoClean -- the SECOND (and final) dependency cluster this file's
//  earlier waves deferred (their own banners said as much; see AutoClean.h's
//  header banner history). REAL, faithful translations replacing the
//  TEMPORARY placeholder that used to sit here (an empty `void
//  DoIndexAutoClean(){}` -- see git history for that stub's own banner/
//  rationale, no longer applicable now that both functions are landed for
//  real).
//
//  Golden: AutoClean.cpp:6465-7358 (DoIndexAutoClean_Arm1PickArm2Test) then
//  :7360-9105 (DoIndexAutoClean, which calls the former at golden :7423).
//  Translated in that same order (callee first) below, mirroring golden's own
//  file layout exactly.
//
//  GOLDEN QUIRK preserved: `ErrPart` is passed BY VALUE (no `&`) into
//  DoIndexAutoClean_Arm1PickArm2Test (golden :6470, verified against the
//  exact signature) -- any `ErrPart+=...`/`ErrPart=...` mutation made inside
//  it is invisible to DoIndexAutoClean's own `static AnsiString ErrPart`
//  after the call returns. Harmless in practice: every case that CONSUMES
//  ErrPart (e.g. the `ShowErrorMessage(...,ErrPart)` calls) always rebuilds
//  it fresh, in the SAME call, from the live FTestSuck/BTestSuck.Suck[][].Error
//  grid before reading it back -- nothing relies on it surviving across
//  ticks/calls. Kept exactly as golden declares it (not silently upgraded to
//  a reference).
// =============================================================================
void DoIndexAutoClean_Arm1PickArm2Test(int iSpeed, int iSpeedSY, int iSpeedSZ, int &iContactCount,
                                       bool bDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL],
                                       bool bSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL],
                                       bool bIsSuckICFallDown[MAX_SOCKET_ROW][MAX_SOCKET_COL],
                                       bool bTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL],
                                       AnsiString ErrPart)                      //Jimmychiu 20230710 : Auto Clean 用 Arm1 下料 arm2 測試
{
    static int iCT=0;

    int ret=0, iPos;
    int &Task=iDoIndexAutoCleanTask;
    bool bFlagY1=false, bFlagS2=false, flag1=false;
    bool bHasErr=false, bHasDuplicateErr=false;                                 //kevin 20180716
    AnsiString Str;

    switch(Task)
    {
        case 1:
            W906DIAC_FHOME_INITDOTESTZHOME();
            bInedxCleanFinish[0]=false;                                         //kevin 20170520 (wei) Autoclean index Arm 1完成
            bInedxCleanFinish[1]=false;                                         //kevin 20170520 (wei) Autoclean index Arm 2完成
            Task=100;
            break;
        case 100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, GetMotFunc(__FUNC__,Task)))
            {
                if(TestSocket.HasDefineIC(HAS_CLEAN_IC))                        //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                {
                    Task=3090;
                }
                else if(TestSocket.HasDefineIC(CLEAN_FINISH_IC))                //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                {
                    Task=3150;
                }
                else if(FTestSuck.HasDefineIC(HAS_CLEAN_IC))                    //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                {
                    Task=200;
                }
                else if(FTestSuck.HasDefineIC(CLEAN_FINISH_IC))                 //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                {
                    Task=3750;
                }
                else
                {
                    Task=200;
                }
            }
            break;
        case 200:
            //重置index狀態
            W906DIAC_FHOME_INITDOTESTZHOME();
            iContactCount=0;
            Task=300;
            break;
        case 300:
            if(EP_Install)
            {
                ADAM_WriteVoltage(TestIF.fAutoClean_AireForce);
            }
            bFlagY1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedSY, GetMotFunc(__FUNC__,Task));//kevin 20120517 iSpeed);    //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            bFlagS2=false;

            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
            {
                if(FLCarryKit.UseSiteHasIC() && InSHT1InRT())                   //Sam 20210923 : 修正 AutoClean Index Arm 為送空盤導致 Hang up    //Steven 20220211 : HasRealIC --> HasIC
                {
                    MOT[MInShuttle1].fCanMoveM=false;                           //kevin 20170119 index cleanpad會限制到 shuttle的移動
                    bFlagS2=true;
                }
            }
            else if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                bFlagS2=true;
            }

            if(bFlagY1 && bFlagS2)
            {
                MOT[MInShuttle1].fCanMoveM=false;                               //index與shuttle到達等待位置上，shuttle 1移clean pad
                Task=400;
            }
            break;
        case 400:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick+TestIF_File.iAutoClean_IndexPickOffset, iSpeed, "DoIndexAutoClean_Arm1PickArm2Test_400"))        //wei 20150318 Auto clean Index Pick Offset
            {
                DoIndexAutoCleanDelay.SetMSAndOn(500);
                Task=500;
            }
            break;
        case 500:
            if(DoIndexAutoCleanDelay.Off())
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        bSuckFinish[i][j]=false;                                //Steven 20110301 : 初始化，尚未完成
                        bTestSuckUse[i][j]=false;
                    }
                }
                FTestSuck.ResetAll();                                           //Steven 20160323 : 避免錯誤殘留
                Task=600;
            }
            break;
        case 600:
            flag1=true;
            for(int i=0; i<MAX_Index_Row; i++)                                  //換IC狀態
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FLCarryKit.Item[i][j])
                    {
                        if(FLCarryKit.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           bSuckFinish[i][j]==true)                             //Steven 20110301
                        {
                            if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                               FTestSuck.Item[i][j]==NULL_IC)                   //Steven 20111202 : Retry會重取
                                FTestSuck.Suck[i][j].Normal();                  //Steven 20111201 : 已完成不執行
                            bSuckFinish[i][j]=true;                             //Steven 20110301
                        }
                        else
                        {
                            if(fMain->bAutoCleanTest==true && i==1 && j==2)
                            {
                                FTestSuck.Suck[i][j].Error=true;
                                fMain->bAutoCleanTest=false;
                                bSuckFinish[i][j]=true;
                                return;
                            }
                            #ifdef SOFT_SIMULTE
                            int x=atoi(fMain->edHPY->Text.c_str());
                            int y=atoi(fMain->edHPX->Text.c_str());
                            if(fMain->cbIndexDrop->Checked==true && i==x && j==y)
                            {
                                FTestSuck.Suck[i][j].Error=true;
                                fMain->bAutoCleanTest=false;
                                bSuckFinish[i][j]=true;
                                continue;
                            }
                            #endif
                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                bDuplicateErr[i][j]=false;                      //Steven 20100105
                                bSuckFinish[i][j]=true;                         //Steven 20110301 : 吸嘴完成本次判定
                            }
                            else if(FTestSuck.Suck[i][j].Error)                 //Steven 20110301 : 本次錯誤不判定
                            {
                                bSuckFinish[i][j]=true;
                            }
                            else
                            {
                                flag1=false;                                    //jou 2011-08-16 只要有任何未完成就繼續
                            }
                        }
                    }
                    else
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           FTestSuck.Item[i][j]==NULL_IC)                       //Steven 20111202 : Retry會重取
                            FTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 已完成不執行
                        bSuckFinish[i][j]=true;                                 //Steven 20110301 : 沒有東西的地方要跳過
                    }
                }
            }

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bSuckFinish[i][j]==false)                                //只要有任何未完成就繼續
                        flag1=false;
                }
            }

            if(flag1==true)                                                     //Steven 20110301 : 所有吸嘴皆完成
            {
                for(int i=0; i<MAX_Index_Row; i++)                              //確認吸嘴狀態異常
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            Task=650;
                            return;
                        }
                    }
                }

                if(FLCarryKit.HasRealIC())
                    break;

                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FLCarryKit.Item[i][j])
                        {
                            if(FLCarryKit.Item[i][j]==HAS_NULL_CLEAN_IC)
                            {
                                FTestSuck.Suck[i][j].Normal();
                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                            }
                        }
                        bDuplicateErr[i][j]=false;                              //Steven 20100105
                    }
                }

                if(FLCarryKit.UseSiteHasIC())
                    break;

                Task=700;
            }
            break;
        case 650:                                                               //移動安全位置
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean_Arm1PickArm2Test_650"))
            {
                Task=651;
            }
            break;
        case 651:                                                               //Richard 20230418 : pickupError shutter move to left position
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            iPos=IsNNMode();
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;
                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iPos][j];
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(IniConfig.bD43IndexDropErrorCanRetryandSkip==true && bHasErr==true)
            {
                MOT[MInShuttle1].fCanMoveM=true;
                bAutoCleanShuttle1MoveToLeft=true;
                bAutoCleanShuttle1HasPickErr=true;
                Task=652;
            }
            else
            {
                Task=660;
            }
            break;
        case 652:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
               bAutoCleanShuttle1MoveToLeft)                                    //ChungHung 20131015 fix hangup
            {
                MOT[MInShuttle1].fCanMoveM=true;
                if(InSHT1InLF()!=true)
                {
                    break;
                }
            }
            MOT[MInShuttle1].fCanMoveM=false;
            Task=660;
        case 660:
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            iPos=IsNNMode();
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iPos][j];
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(bHasErr)                                                         //Shuttle上有 確認是否移動??
            {
                bHasErr=false;                                                  //Ifor 20160322 移動至上面
                if(DoInArmMoveToWaitPosByAutoClean()==false)                    //JerryYang 20241118 : fix auto clean hang up    //Steven 20130613 : Index有異常時, In Arm要有先讓功能
                {
                    return ;
                }
                bIsTestSitICFallDown=true;
                if(IniConfig.bIndexPickErrOnlySKIP==true ||                     //kevin 201701103 (Steven) index pick up error only skip
                   IniConfig.bD64IndexPickErrOnlySKIP)                          //JerryYang 20160301 index pick-up error only skip
                    ret=ShowErrorMessage("JAM0312", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart);            //Device Pick-Up Error  //JerryYang 20160511 JAM0301->JAM0312,將IC與Clean pad分開的alarm code分開
                else
                    ret=ShowErrorMessage("JAM0312", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);    //Devicr Pick-Up Error  //JerryYang 20160511 JAM0301->JAM0312,將IC與Clean pad分開的alarm code分開

                if(ret==K_SKIP)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Error)
                            {
                                FTestSuck.MoveSuckData(FLCarryKit, i, j);       //JerryYang 20160223 MARK
                                FTestSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC); //Steven 20260126 : 為了記得座標,但是只能填入 HAS_NULL_CLEAN_IC
                                FTestSuck.Suck[i][j].Normal();
                                bTestSuckUse[i][j]=true;
                            }
                            bDuplicateErr[i][j]=false;
                        }
                    }
                }
                else
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                            if(FTestSuck.Suck[i][j].Error)
                                bDuplicateErr[i][j]=true;
                }
                FTestSuck.ResetAll();                                           //Steven 20160323 : 避免錯誤殘留
                bAutoCleanShuttle1MoveToLeft=false;
                MOT[MInShuttle1].fCanMoveM=true;
            }
            bHasErr=false;
            bAutoCleanShuttle1HasPickErr=false;
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
                {
                    Task=670;
                }
                else
                {
                    if(FLCarryKit.HasRealIC())
                        Task=400;
                    else
                        Task=700;
                }
            }
            break;
        case 670:
            if(InSHT1InRT())
            {
                MOT[MInShuttle1].fCanMoveM=false;
                if(FLCarryKit.HasRealIC())                                      //有IC->Retry
                    Task=400;
                else
                    Task=700;                                                   //無IC->Skip
            }
            break;
        case 700:                                                               //Steven 20160323 : 吸嘴等一下再作動
            DoIndexAutoCleanDelay.SetMSAndOn(500);
            Task=710;
        case 710:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=720;
            }
            break;
        case 720:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean_Arm1PickArm2Test_720"))
            {
                Task=800;
            }
            break;
        case 800:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Suck[i][j].GetStatus()==false)
                        FTestSuck.Suck[i][j].Normal();
                }
            }
            Task=900;
            break;
        case 900:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedSY, GetMotFunc(__FUNC__,Task)))  //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                Task=1000;
            }
            break;
        case 1000:
            W906DIAC_FHOME_INITDOTESTZHOME();
            Task=1150;
            break;
        case 1150:
            if(MOT[MTestZ1].Gali_MotMove(Prod.iAutoCleanZ_Contact[0], iSpeedSZ, "DoIndexAutoClean_Arm1PickArm2Test_1150"))//kevin 20181016 add drop 高低
            {
                flag1=true;
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Item[i][j]>0)
                        {
                            if(FTestSuck.Item[i][j]!=HAS_CLEAN_IC ||
                               FTestSuck.Suck[i][j].Destroy())
                            {
                                TestSocket.MoveSuckData(FTestSuck, i, j);
                            }
                            else if(FTestSuck.Suck[i][j].Error==false)
                            {
                                flag1=false;
                            }
                        }
                    }
                }

                if(flag1==true)
                {
                    bIndexCheckNoStopVaccum=false;
                    bHasICinSocket=true;

                    DoIndexAutoCleanDelay.SetSecAndOn(1.5);
                    Task=1160;
                }
                break;
                }
                break;
        case 1160:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=1400;
            }
            break;
        case 1400:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, GetMotFunc(__FUNC__,Task)))
            {
                if(CheckSocketSensor(0, "DoIndexAutoClean_Arm1PickArm2Test_1400", true, false))  //Jimmychiu 20230821 : Arm1PickArm2Test add Socket Sensor Function
                {
                    Task=1450;
                }
                else
                {
                    bHasICinSocket=false;
                    Task=1500;
                }
            }
            break;
        case 1450:
            if(DoSocketSensorAlarm(__FUNC__, Task)==true)
            {
                Task=1400;                                                      //Steven 20201014 : 修正防止異常後, index arm要放開
            }
            break;
        case 1500:
            if(MOT[MTestY1].Gali_MotMove(Prod.TestY1_Front, iSpeedSY, "DoIndexAutoClean_Arm1PickArm2Test_1500"))          //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                Task=2100;
            }
            break;
        case 2100:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeed, GetMotFunc(__FUNC__,Task)))
            {
                Task=2200;
            }
            break;
        case 2200:
            W906DIAC_FHOME_INITDOTESTZHOME();
            iContactCount=0;
            Task=2300;
            break;
        case 2300:
            if(EP_Install)
            {
                ADAM_WriteVoltage(TestIF.fAutoClean_AireForce);
            }
            bFlagY1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedSY, GetMotFunc(__FUNC__, Task));//kevin 20120517 iSpeed);   //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            if(bFlagY1)
            {
                DoIndexAutoCleanDelay.SetSecAndOn(1.5);
                MOT[MInShuttle2].fCanMoveM=false;
                Task=2310;
            }
            break;
        case 2310:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=2900;
            }
            break;
        case 2900:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedSY, GetMotFunc(__FUNC__, Task)))//kevin 20120517 iSpeed))  //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                Task=3000;
            }
            break;
        case 3000:
            W906DIAC_FHOME_INITDOTESTZHOME();
            if(Prod.iAutoCleanZ_Contact[1]>-1000)                               //Steven 20260130 : Add protection for Auto clean
            {
                Str.sprintf("iAutoCleanZ_Contact[0, 1]=[%d, %d] !!!", Prod.iAutoCleanZ_Contact[0], Prod.iAutoCleanZ_Contact[1]);
                RecordProcess(Str);
                SetTechDataToProd_AutoClean();
            }
            Task=3090;
            break;
        case 3090:
            if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Contact[1], iSpeedSZ, "DoIndexAutoClean_Arm1PickArm2Test_3090"))
            {
                iContactCount++;
                DoIndexAutoCleanDelay.Set0_1SecAndOn(Prod.iAutoClean_ContactTime);  //Sam 20230111 : Smart Auto Clean
                Task=3100;
            }
            break;
        case 3100:
            if(DoIndexAutoCleanDelay.Off())
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].Off();
                if(iContactCount<Prod.iAutoClean_ContactCount)                  //Sam 20230111 : Smart Auto Clean
                {
                    Task=3130;
                }
                else
                {
                    if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    {
                        Task=3150;

                        for(int i=0; i<MAX_Index_Row; i++)                      //JerryYang 20260312 : fix補一併Drop error
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(TestSocket.Item[i][j])
                                {
                                    if(TestSocket.Item[i][j]!=HAS_NULL_CLEAN_IC)
                                        TestSocket.SetItemData(i, j, CLEAN_FINISH_IC);
                                }
                            }
                        }
                    }
                    else
                    {
                        Task=3200;
                    }
                }
            }
            break;
        case 3130:
            if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Shift[1], iSpeedSZ, "DoIndexAutoClean_Arm1PickArm2Test_3130"))
            {
                DoIndexAutoCleanDelay.SetSecAndOn(1.5);
                iCT=0;
                Task=3135;
            }
            break;
        case 3135:
            if(DoIndexAutoCleanDelay.Off())
            {
                bHasICinSocket=true;
                if(CheckSocketSensor(1, "DoIndexAutoClean_Arm1PickArm2Test_3135", true, true))  //Jimmychiu 20230821 : Arm1PickArm2Test add Socket Sensor Function
                {
                    iCT++;
                    if(iCT>=3)
                    {
                        iCT=0;
                        Task=3140;
                    }
                    break;
                }

                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].On();
                bHasICinSocket=false;
                Task=3090;
            }
            break;
        case 3140:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeed, GetMotFunc(__FUNC__,Task)))
            {
                Task=3142;
            }
            break;
        case 3142:
            if(DoSocketSensorAlarm(__FUNC__, Task)==true)
            {
                Task=3130;
            }
            break;
        case 3150:
            if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Shift[1], iSpeedSZ, "DoIndexAutoClean_Arm1PickArm2Test_3150"))
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].On();
                DoIndexAutoCleanDelay.SetSecAndOn(1.5);
                Task=3160;
            }
            break;
        case 3160:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=3400;
            }
            break;
        case 3400:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeed, GetMotFunc(__FUNC__, Task)))
            {
                Task=3600;
            }
            break;
        case 3600:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedSY, GetMotFunc(__FUNC__, Task)))//kevin 20120517 iSpeed))    //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                bHasICinSocket=true;
                if(CheckSocketSensor(0, "DoIndexAutoClean_Arm1PickArm2Test_3600", true, false))  //Jimmychiu 20230821 : Arm1PickArm2Test add Socket Sensor Function
                {
                    Task=3610;
                }
                else
                {
                    bHasICinSocket=false;
                    Task=3650;
                }
            }
            break;
        case 3610:
            if(DoSocketSensorAlarm(__FUNC__, Task)==true)
            {
                Task=3600;
            }
            break;
        case 3650:
            if(MOT[MTestZ1].Gali_MotMove(Prod.iAutoCleanZ_Contact[0], iSpeedSZ, "DoIndexAutoClean_Arm1PickArm2Test_3650"))//kevin 20180717 drop high
            {
                Task=3700;
            }
            break;
        case 3700:
            // AI(W906-DoIndexAutoClean) 20260728: golden AutoClean.cpp:7108-7139 (case 3700)
            // opens with a ~22-line block that is ALREADY dead in golden itself -- 3 lines
            // commented with `//` (a DoInArmMoveToWaitPosByAutoClean() early-return guard, and
            // a bJAM0303NeedOpenChamberDoor set) plus a `/* ... */`-commented JAM0314
            // CC_KYEC_LEE/XILINX/CHEN ShowErrorMessage block, and a trailing `//Task=1500;`.
            // All already inert historical commentary in golden (not executed there either) --
            // intentionally NOT carried forward as dead code here, per this wave's translation.
            flag1=true;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(TestSocket.Item[i][j]>0)
                    {
                        if(TestSocket.Item[i][j]==HAS_NULL_CLEAN_IC)            //Steven 20260126 : fixed for [D58] auto clean
                        {
                            FTestSuck.MoveSuckData(TestSocket, i, j);
                        }
                        else if(FTestSuck.Suck[i][j].Suck())
                        {
                            FTestSuck.MoveSuckData(TestSocket, i, j);
                            FTestSuck.SetItemData(i, j, CLEAN_FINISH_IC);
                        }
                        else
                        {
                            flag1=false;                                        //jou 2011-08-16 只要有任何未完成就繼續
                        }
                    }
                }
            }

            if(flag1==true)
            {
                Task=3750;
            }
            break;
        case 3750:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, GetMotFunc(__FUNC__,Task)))
            {
                Task=3800;
                flag1=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    iPos=IsNNMode();
                    ErrPart=" ";                                                //Steven 20160323 : Auto Clean完成的資料
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Enable       &&
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC     &&
                               bTestSuckUse[i][j]==false)
                            {
                                if(FTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    FTestSuck.Suck[i][j].Normal();              //jou 2012-01-17 重置吸嘴狀態，避免延誤shuttle釋放，也避免要重測而Hang up
                                    flag1=true;
                                    ErrPart+=IndexSuckName[i+iPos][j];
                                    bIsSuckICFallDown[i][j]=true;               //kevin 20150624
                                }
                            }
                        }
                    }
                }

                if(flag1)
                {
                    MOT[MTestZ1].Gali_Command("ST", GetMotFunc(__FUNC__,Task));
                    Task=3760;
                }
            }
            break;
        case 3760:                                                              //Sam 20221220 : 放 Index Arm1
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeed, GetMotFunc(__FUNC__,Task)))
            {
                Task=3770;
            }
            break;
        case 3770:
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;
            if(CosFunction.bJAM0303NeedOpenChamberDoor)                         //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                bIsTestSitICFallDown=true;                                      //kevin 20130706
            if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_XILINX || CUSTOMER_CODE==CC_KYEC_CHEN)
                ret=ShowErrorMessage("JAM0314", K_RETRY, MTestZ1, false, ErrPart);  //Steven 20100129 : Device Drop Error   //JerryYang 20160511 JAM0303->JAM0314,將IC與Clean pad分開的alarm code分開
            else
                ret=ShowErrorMessage("JAM0314", K_SKIP, MTestZ1, false, ErrPart);   //Steven 20100129 : Device Drop Error    //JerryYang 20160511 JAM0303->JAM0314,將IC與Clean pad分開的alarm code分開

            if(ret==K_SKIP)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bIsSuckICFallDown[i][j])                             //kevin 20150624
                            FTestSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);     // 掉料歸零
                    }
                }
            }
            Task=3800;
            break;
        case 3800:                                                              //step go to end
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedSY, GetMotFunc(__FUNC__,Task)))    //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=3810;
                else
                    Task=3840;
            }
            break;
        case 3810:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Place+TestIF_File.iAutoClean_IndexReleaseOffset, iSpeed, "DoIndexAutoClean_Arm1PickArm2Test_3810"))  //Jou 2015-08-22 Auto clean Index Release Offset
            {
                Task=3820;
            }
            break;
        case 3820:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Item[i][j]>0)
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           FTestSuck.Suck[i][j].Destroy())
                        {
                            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)    //Clean Kit
                                FLCarryKit.MoveSuckData(FTestSuck, i, j);
                        }
                    }
                }
            }

            flag1=false;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Item[i][j] && FTestSuck.Suck[i][j].Error==false)
                    {
                        flag1=true;
                        break;
                    }
                }
            }

            if(flag1==false)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            Task=3830;
                            return;
                        }
                    }
                }
            }

            if(FTestSuck.UseSiteHasIC())
                break;

            for(int i=0; i<MAX_Index_Row; i++)                                  //Steven 20100105
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                    bDuplicateErr[i][j]=false;
            Task=3840;
            break;
        case 3830:                                                              //開料異常處理
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];
                        FTestSuck.Suck[i][j].Error=false;                       //Steven 20101229 : 重置位置
                        bDuplicateErr[i][j]=true;                               //Steven 20101229 : 重置位置
                    }
                    else
                    {
                        bDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);   //Vacuum sensor OFF error

            bHasErr=false;
            Task=3820;
            break;
        case 3840:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean 3840"))
            {
                if((iInArmType==e9045_1x3_4 ||                                  //Steven 20220922 : Fixed for 1x3 auto clean
                    iInArmType==e9045_1x3_2_14) &&
                   TestIF_File.iAutoClean_DeveicePices%3==0)
                {
                    FLCarryKit.SetItemData(0, 3, NULL_IC);
                }
                Task=3850;
            }
            break;
        case 3850:
            MOT[MInShuttle1].fCanMoveM=true;
            bInedxCleanFinish[0]=true;                                          //kevin 20170520 (wei) Autoclean index Arm 1完成
            Task=3900;
        case 3900:
            MOT[MInShuttle2].fCanMoveM=true;
            bInedxCleanFinish[1]=true;                                          //kevin 20170520 (wei) Autoclean index Arm 2完成
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                bRunAutoClean=false;                                            //停止Auto Clean動作
                bAutoCleanFinishOnlyUseRTC=true;                                //JerryYang 20161216 (Steven) 停止auto clean後不需要index check
                iAutoClean_IndexContactCount=0;
                CleanSetSpeed(false);
            }
            break;
    }
}
//------------------------------------------------------------------------------
void DoIndexAutoClean()
{
    int &Task=iDoIndexAutoCleanTask;
    bool bFlagY1=false, bFlagS2=false, flag=false, flag1=false;
    int iSpeed, iSpeedSZ, iIndexSpeed, iSpeedSY, iPos;                          //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
    static int iContactCount;
    AnsiString Message;

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
        return;

    if(TestIF_File.iAutoClean_MotorSpeed[2]>=80)
        iIndexSpeed=TestIF_File.iAutoClean_MotorSpeed[2]-20;                    //kevin 20120919
    else
        iIndexSpeed=TestIF_File.iAutoClean_MotorSpeed[2];

    iSpeed=MOT[MTestY1].Motor->PJogHighSpeed*iIndexSpeed/100;
    if(iSpeed>=300000)                                                          //kevin 20120919 GAIL SPEED > 3000000 RC= -2
       iSpeed=300000;
    iSpeedSZ=iSpeed*2;
    if(iSpeedSZ>=300000)
       iSpeedSZ=300000;

    int ret;
    int iRet=0;
    static AnsiString ErrPart="";                                               //Steven 20160323 : Auto Clean完成的資料, 加上static
    bool bHasDuplicateErr=false;
    bool bHasErr=false, bCheckDestroy=false, bCheckSuck=false;                  //kevin 20180716
    static bool bDuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bSuckFinish[MAX_SOCKET_ROW][MAX_SOCKET_COL]  ={{false, false, false, false, false, false, false, false},    //Steven 20110301 : 確認吸嘴完成
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false},
                                                               {false, false, false, false, false, false, false, false}};

    static bool bTestSuckUse[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false,false,false,false,false,false,false,false},
                                                              {false,false,false,false,false,false,false,false},
                                                              {false,false,false,false,false,false,false,false},
                                                              {false,false,false,false,false,false,false,false}};

    static bool bIsSuckICFallDown[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false,false,false,false,false,false,false,false},       //kevin 20150624 重複使用
                                                                   {false,false,false,false,false,false,false,false},
                                                                   {false,false,false,false,false,false,false,false},
                                                                   {false,false,false,false,false,false,false,false}};

    if(bFullViewCheckFinish==false)                                             //JerryYang 20160331 等待FulL view check流程才進行,避免同時對index arm的馬達下指令
        return;

    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
    {
        iSpeedSZ=iSpeed;
        iSpeedSY=iSpeed;
    }
    else
    {
        iSpeedSY=50000;
    }

    if(RunAutoCleanByArmPickArm2Test())                                         //Jimmychiu 20230710 : Auto Clean 用 Arm1 下料 arm2 測試
    {
        DoIndexAutoClean_Arm1PickArm2Test(iSpeed, iSpeedSY, iSpeedSZ, iContactCount,
                                          bDuplicateErr, bSuckFinish,
                                          bIsSuckICFallDown,
                                          bTestSuckUse, ErrPart);
        return;
    }

    if(Task>=400 && Task<=1900 && MOT[MInShuttle1].IsCanMove())
        return;

    if(Task>=2400 && Task<=3900 && MOT[MInShuttle2].IsCanMove())
        return;

    switch(Task)
    {
        case 1:
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                InitialSet();
            }
            W906DIAC_FHOME_INITDOTESTZHOME();
            bInedxCleanFinish[0]=false;                                         //kevin 20170520 (wei) Autoclean index Arm 1完成
            bInedxCleanFinish[1]=false;                                         //kevin 20170520 (wei) Autoclean index Arm 2完成
            Task=100;
            break;
        case 100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean 100"))
            {
                if(CheckAutoCleanCloseSite(0))                                  //Steven 20220929 : 判斷開放Site
                {
                    if(BTestSuck.HasDefineIC(HAS_CLEAN_IC))                     //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                    {
                        Task=2650;
                    }
                    else if(BTestSuck.HasDefineIC(CLEAN_FINISH_IC))             //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                    {
                        Task=3500;
                    }
                    else
                    {
                        Task=2100;
                    }
                }
                else
                {
                    if(CheckAutoCleanCloseSite(0)==false)                       //Steven 20220929 : 判斷開放Site  //GOLDEN QUIRK preserved: redundant re-check of the SAME condition already false in this else-branch (verbatim golden :7468, not simplified away)
                    {
                        if(FTestSuck.HasDefineIC(HAS_CLEAN_IC))                 //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                        {
                            Task=650;
                        }
                        else if(FTestSuck.HasDefineIC(CLEAN_FINISH_IC))         //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                        {
                            Task=1500;
                        }
                        else
                        {
                            Task=200;
                        }
                    }

                    if((CosFunction.bAutoCleanAutoSelIndexArm==false && TestIF.iAutoClean_SelectArm==1) ||
                       (CosFunction.bAutoCleanAutoSelIndexArm==true && TestIF.bCleanIndexOtherArm==false &&
                        TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1))
                    {
                        if(BTestSuck.HasDefineIC(HAS_CLEAN_IC))                 //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                        {
                            Task=2650;
                        }
                        else if(BTestSuck.HasDefineIC(CLEAN_FINISH_IC))         //Steven 20130620 : Auto Clean做完在左右鎖要有分開
                        {
                            Task=3500;
                        }
                        else
                        {
                            Task=2100;
                        }
                    }
                }
            }
            break;
        case 200:
            //重置index狀態
            W906DIAC_FHOME_INITDOTESTZHOME();
            iContactCount=0;
            Task=300;
            break;
        case 300:
            if(EP_Install)
            {
                ADAM_WriteVoltage(TestIF.fAutoClean_AireForce);
            }
            bFlagY1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedSY, "DoIndexAutoClean 300");//kevin 20120517 iSpeed);    //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            bFlagS2=false;

            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
            {
                if(FLCarryKit.UseSiteHasIC() && InSHT1InRT())                   //Sam 20210923 : 修正 AutoClean Index Arm 為送空盤導致 Hang up    //Steven 20220211 : HasRealIC --> HasIC
                {
                    MOT[MInShuttle1].fCanMoveM=false;                           //kevin 20170119 index cleanpad會限制到 shuttle的移動
                    bFlagS2=true;
                }
            }
            else if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                bFlagS2=true;
            }

            if(bFlagY1 && bFlagS2)
            {
                MOT[MInShuttle1].fCanMoveM=false;

                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=400;
                else
                    Task=650;
            }
            break;
        case 400:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Pick+TestIF_File.iAutoClean_IndexPickOffset, iSpeed, "DoIndexAutoClean_Arm1PickArm2Test_400"))        //wei 20150318 Auto clean Index Pick Offset
            {
                DoIndexAutoCleanDelay.SetMSAndOn(500);
                Task=500;
            }
            break;
        case 500:
            if(DoIndexAutoCleanDelay.Off())
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        bSuckFinish[i][j]=false;                                //Steven 20110301 : 初始化，尚未完成
                        bTestSuckUse[i][j]=false;
                    }
                }
                FTestSuck.ResetAll();                                           //Steven 20160323 : 避免錯誤殘留
                Task=600;
            }
            break;
        case 600:
            flag1=true;
            for(int i=0; i<MAX_Index_Row; i++)                                  //換IC狀態
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FLCarryKit.Item[i][j])
                    {
                        if(FLCarryKit.Item[i][j]==HAS_NULL_CLEAN_IC)
                        {
                            FTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 已完成不執行
                            FTestSuck.MoveSuckData(FLCarryKit, i, j);
                            bSuckFinish[i][j]=true;                             //Steven 20110301
                        }
                        else if(bSuckFinish[i][j]==true)
                        {
                            if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                               FTestSuck.Item[i][j]==NULL_IC)                   //Steven 20111202 : Retry會重取
                            {
                                FTestSuck.Suck[i][j].Normal();                  //Steven 20111201 : 已完成不執行
                            }
                        }
                        else
                        {
                            if(fMain->bAutoCleanTest==true && i==1 && j==2)
                            {
                                FTestSuck.Suck[i][j].Error=true;
                                fMain->bAutoCleanTest=false;
                                bSuckFinish[i][j]=true;
                                return;
                            }
                            #ifdef SOFT_SIMULTE
                            int x=atoi(fMain->edHPY->Text.c_str());
                            int y=atoi(fMain->edHPX->Text.c_str());
                            if(fMain->cbIndexDrop->Checked==true && i==x && j==y)
                            {
                                FTestSuck.Suck[i][j].Error=true;
                                fMain->bAutoCleanTest=false;
                                bSuckFinish[i][j]=true;
                                continue;
                            }
                            #endif

                            if(FTestSuck.Suck[i][j].Suck())
                            {
                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                bDuplicateErr[i][j]=false;                      //Steven 20100105
                                bSuckFinish[i][j]=true;                         //Steven 20110301 : 吸嘴完成本次判定
                            }
                            else if(FTestSuck.Suck[i][j].Error)                 //Steven 20110301 : 本次錯誤不判定
                            {
                                bSuckFinish[i][j]=true;
                            }
                            else
                            {
                                flag1=false;                                    //jou 2011-08-16 只要有任何未完成就繼續
                            }
                        }
                    }
                    else
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           FTestSuck.Item[i][j]==NULL_IC)                       //Steven 20111202 : Retry會重取
                        {
                            FTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 已完成不執行
                        }
                        bSuckFinish[i][j]=true;                                 //Steven 20110301 : 沒有東西的地方要跳過
                    }
                }
            }

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bSuckFinish[i][j]==false)                                //只要有任何未完成就繼續
                        flag1=false;
                }
            }

            if(flag1==true)                                                     //Steven 20110301 : 所有吸嘴皆完成
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            Task=650;
                            return;
                        }
                    }
                }

                if(FLCarryKit.HasRealIC())
                    break;

                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC)
                        {
                            FTestSuck.Suck[i][j].Normal();
                        }
                        bDuplicateErr[i][j]=false;
                    }
                }

                if(FLCarryKit.UseSiteHasIC())
                    break;

                Task=700;
            }
            break;
        case 650:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean_650"))
            {
                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=651;
                else
                    Task=900;
            }
            break;
        case 651:                                                               //Richard 20230418 : pickupError shutter move to left position
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            iPos=IsNNMode();

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iPos][j];
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(IniConfig.bD43IndexDropErrorCanRetryandSkip==true &&
               bHasErr==true)
            {
                MOT[MInShuttle1].fCanMoveM=true;
                bAutoCleanShuttle1MoveToLeft=true;
                bAutoCleanShuttle1HasPickErr=true;
                Task=652;
            }
            else
            {
                Task=660;
            }
            break;
        case 652:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
               bAutoCleanShuttle1MoveToLeft)                                    //ChungHung 20131015 fix hangup
            {
                MOT[MInShuttle1].fCanMoveM=true;
                if(InSHT1InLF()!=true)
                {
                    break;
                }
            }
            MOT[MInShuttle1].fCanMoveM=false;
            Task=660;
        case 660:
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            iPos=IsNNMode();
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iPos][j];
                    }
                    else
                    {
                        FTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(bHasErr)
            {
                bHasErr=false;

                if(DoInArmMoveToWaitPosByAutoClean()==false)                    //JerryYang 20241118 : fix auto clean hang up    //Steven 20130613 : Index有異常時, In Arm要有先讓功能
                {
                    return ;
                }
                bIsTestSitICFallDown=true;

                if(IniConfig.bIndexPickErrOnlySKIP==true ||                     //kevin 201701103 (Steven) index pick up error only skip
                   IniConfig.bD64IndexPickErrOnlySKIP)                          //JerryYang 20160301 index pick-up error only skip
                {
                    ret=ShowErrorMessage("JAM0312", K_SKIP, MTestZ1, bHasDuplicateErr, ErrPart); //Device Pick-Up Error             //JerryYang 20160511 JAM0301->JAM0312,將IC與Clean pad分開的alarm code分開
                }
                else
                {
                    ret=ShowErrorMessage("JAM0312", K_SKIP|K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart); //Devicr Pick-Up Error     //JerryYang 20160511 JAM0301->JAM0312,將IC與Clean pad分開的alarm code分開
                }

                if(ret==K_SKIP)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Error)
                            {
                                FTestSuck.MoveSuckData(FLCarryKit, i, j);
                                FTestSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC); //Steven 20250326 : fixed for auto clean
                                FTestSuck.Suck[i][j].Normal();
                                bTestSuckUse[i][j]=true;
                            }
                            bDuplicateErr[i][j]=false;
                        }
                    }
                }
                else
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Error)
                            {
                                bDuplicateErr[i][j]=true;
                            }
                        }
                    }
                }
                FTestSuck.ResetAll();                                           //Steven 20160323 : 避免錯誤殘留
                bAutoCleanShuttle1MoveToLeft=false;
                MOT[MInShuttle1].fCanMoveM=true;
            }
            bHasErr=false;
            bAutoCleanShuttle1HasPickErr=false;
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
                {
                    Task=670;
                }
                else
                {
                    if(FLCarryKit.HasRealIC())
                        Task=400;
                    else
                        Task=700;
                }
            }
            break;
        case 670:
            if(InSHT1InRT())
            {
                MOT[MInShuttle1].fCanMoveM=false;
                if(FLCarryKit.HasRealIC())
                    Task=400;
                else
                    Task=700;
            }
            break;
        case 700:                                                               //Steven 20160323 : 吸嘴等一下再作動
            DoIndexAutoCleanDelay.SetMSAndOn(500);
            Task=710;
        case 710:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=720;
            }
            break;
        case 720:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean_720"))
            {
                Task=800;
            }
            break;
        case 800:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Suck[i][j].GetStatus()==false)
                        FTestSuck.Suck[i][j].Normal();
                }
            }
            Task=900;
            break;
        case 900:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedSY, "DoIndexAutoClean 900"))//kevin 20120517 iSpeed))    //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                Task=1000;
            }
            break;
        case 1000:
            W906DIAC_FHOME_INITDOTESTZHOME();
            Task=1050;
            break;
        case 1050:                                                              //kevin 20180716 drop contract 分是下高或高低
            if(Prod.iAutoClean_ContactMode==0)                                  //Sam 20230111 : Smart Auto Clean //kevin 20180716 direction
            {
                Task=1150;
            }
            else
            {
                if(MOT[MTestZ1].Gali_MotMove(Prod.iAutoCleanZ_Drop[0], iSpeedSZ, "DoIndexAutoClean_1050")) //kevin 20180717 drop high
                    Task=1055;
            }
            break;
        case 1055:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Item[i][j]==HAS_CLEAN_IC)
                    {
                        FTestSuck.Suck[i][j].Off();
                    }
                }
            }
            DoTestYRearDelayAC.SetSecAndOn(Prod.TestZ_Drop_Wait);               //Steven 20140909 : 延遲吸放後再作
            Task=1060;
        case 1060:
            bCheckDestroy=true;
            if(bCheckDestroy==true && DoTestYRearDelayAC.Off())
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_CLEAN_IC)
                            FTestSuck.Suck[i][j].Normal();
                    }
                }
                bIndexCheckNoStopVaccum=false;
                Task=1150;
            }
            break;
        case 1150:
            if(MOT[MTestZ1].Gali_MotMove(Prod.iAutoCleanZ_Contact[0], iSpeedSZ, "DoIndexAutoClean_1150"))//kevin 20181016 add drop 高低
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].Off();

                if(Prod.iAutoClean_ContactMode==0)                              //Sam 20230111 : Smart Auto Clean //kevin 20180716 direction
                {
                    Task=1200;
                    DoIndexAutoCleanDelay.Set0_1SecAndOn(Prod.iAutoClean_ContactTime);  //Sam 20230111 : Smart Auto Clean
                    iContactCount++;
                    TestIF_File.iIndexArmAutoCleanCnt++;                        //Sam 20250820 : AutoClean 在 Index Arm 下壓時作動次數++
                }
                else
                {
                    Task=1160;                                                  //KEVIN 20180724 ADD
                }
            }
            break;
        case 1160:
            if(DeviceForm.VacuumMode==VacuumONMode)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Item[i][j]==HAS_CLEAN_IC)
                        {
                            FTestSuck.Suck[i][j].On();
                            if(INDEX_SUCKER_TYPE==1)
                            {
                                fiosetview->bIndexSuck[0][i][j]=true;
                            }
                        }
                    }
                }
            }

            DoTestYRearDelayAC.SetMSAndOn(100);
            Task=1170;
            break;
        case 1170:
            if(INDEX_SUCKER_TYPE==1)
            {
                bCheckSuck=W906DIAC_FIOSET_PISD1();
            }
            else
            {
                bCheckSuck=true;
            }

            if(bCheckSuck==true && DoTestYRearDelayAC.Off())
            {
                bFTestSuckDrop=false;
                DoIndexAutoCleanDelay.Set0_1SecAndOn(Prod.iAutoClean_ContactTime);  //Sam 20230111 : Smart Auto Clean
                iContactCount++;
                TestIF_File.iIndexArmAutoCleanCnt++;                            //Sam 20250820 : AutoClean 在 Index Arm 下壓時作動次數++
                Task=1200;
            }
            break;
        case 1200:                                                              //在旁邊置停等
            if(DoIndexAutoCleanDelay.Off())
            {
                if(iContactCount<Prod.iAutoClean_ContactCount)                  //Sam 20230111 : Smart Auto Clean
                {
                    Task=1300;
                }
                else
                {
                    if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(FTestSuck.Item[i][j])
                                {
                                    if(FTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC)
                                        FTestSuck.SetItemData(i, j, CLEAN_FINISH_IC);         //Steven 20130701
                                }
                            }
                        }
                    }
                    Task=1350;
                }
            }
            break;
        case 1300:
            if(MOT[MTestZ1].Gali_MotMove(Prod.iAutoCleanZ_Shift[0], iSpeedSZ, "DoIndexAutoClean_1300"))  //kevin 20181016 add drop 高低
            {
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    iPos=IsNNMode();
                    ErrPart=" ";                                                //Steven 20160323 : Auto Clean完成的資料
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            bIsSuckICFallDown[i][j]=false;                      //kevin 20150624
                            if(FTestSuck.Suck[i][j].Enable       &&
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC     &&
                               bTestSuckUse[i][j]==false)
                            {
                                if(FTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    FTestSuck.Suck[i][j].Normal();              //jou 2012-01-17 重置吸嘴狀態，避免延誤shuttle釋放，也避免要重測而Hang up
                                    flag=true;
                                    ErrPart+=IndexSuckName[i+iPos][j];
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case 1300"));

                    if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(FTestSuck.Item[i][j])
                                {
                                    if(FTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC)
                                        FTestSuck.SetItemData(i, j, CLEAN_FINISH_IC);
                                }
                            }
                        }
                    }
                    DoIndexAutoCleanDelay.SetMSAndOn(100);
                    Task=1360;
                    break;
                }

                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].On();

                DoIndexAutoCleanDelay.SetMSAndOn(100);
                Task=1310;
            }
            break;
        case 1310:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=1050;
            }
            break;
        case 1350:
            if(MOT[MTestZ1].Gali_MotMove(Prod.iAutoCleanZ_Shift[0],iSpeedSZ, "DoIndexAutoClean_1350"))   //JerryYang 20181119 (Steven) : fix auto clean 高度異常
            {
                for(int i=0; i<MAX_Index_Row; i++)                              //ChungHung 20150407 add for SCK drop issue
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        bDuplicateErr[i][j]=false;
                        bIsSuckICFallDown[i][j]=false;                          //kevin 20150624
                    }
                }
                Task=1360;
                DoIndexAutoCleanDelay.SetMSAndOn(100);
            }
            break;
        case 1360:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=1400;
            }
            break;
        case 1400:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean 1400"))
            {
                Task=1500;

                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    iPos=IsNNMode();
                    ErrPart=" ";                                                //Steven 20160323 : Auto Clean完成的資料
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FTestSuck.Suck[i][j].Enable       &&
                               FTestSuck.Suck[i][j].SenUsing!="" &&
                               FTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                               FTestSuck.Item[i][j]!=NULL_IC     &&
                               bTestSuckUse[i][j]==false)
                            {
                                if(FTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    FTestSuck.Suck[i][j].Normal();              //jou 2012-01-17 重置吸嘴狀態，避免延誤shuttle釋放，也避免要重測而Hang up
                                    flag=true;
                                    ErrPart+=IndexSuckName[i+iPos][j];
                                    bIsSuckICFallDown[i][j]=true;               //kevin 20150624
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ1].Gali_Command("ST", __FUNC__+AnsiString(",case 1400"));
                    Task=1410;
                }
            }
            break;
        case 1410:                                                              //Sam 20221220 : 放 Index Arm1
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear, iSpeed, __FUNC__+AnsiString("case 1410")))
                Task=1420;
            break;
        case 1420:
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            if(CosFunction.bJAM0303NeedOpenChamberDoor)                         //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                bIsTestSitICFallDown=true;                                      //kevin 20130706

            if(CUSTOMER_CODE==CC_KYEC_LEE ||
               CUSTOMER_CODE==CC_KYEC_XILINX ||
               CUSTOMER_CODE==CC_KYEC_CHEN)
                iRet=ShowErrorMessage("JAM0314", K_RETRY, MTestZ1, false, ErrPart); //Steven 20100129 : Device Drop Error   //JerryYang 20160511 JAM0303->JAM0314,將IC與Clean pad分開的alarm code分開
            else
                iRet=ShowErrorMessage("JAM0314", K_SKIP, MTestZ1, false, ErrPart);  //Steven 20100129 : Device Drop Error    //JerryYang 20160511 JAM0303->JAM0314,將IC與Clean pad分開的alarm code分開

            if(iRet==K_SKIP)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bIsSuckICFallDown[i][j])                             //kevin 20150624
                            FTestSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);     // 掉料歸零
                    }
                }
            }
            Task=1500;
            break;
        case 1500:
            if(MOT[MTestY1].Gali_MotMove(Prod.TestY1_Front, iSpeedSY, "DoIndexAutoClean_1500"))          //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            {
                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=1600;
                else
                    Task=1800;
            }
            break;
        case 1600:
            if(MOT[MTestZ1].Gali_MotMove(Prod.TestZ1_Place+TestIF_File.iAutoClean_IndexReleaseOffset, iSpeed, "DoIndexAutoClean_1600"))  //Jou 2015-08-22 Auto clean Index Release Offset
            {
                Task=1700;
            }
            break;
        case 1700:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Item[i][j])
                    {
                        if(FTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           FTestSuck.Suck[i][j].Destroy())
                        {
                            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)    //Clean Kit
                                FLCarryKit.MoveSuckData(FTestSuck, i, j);
                        }
                    }
                }
            }

            flag=false;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(FTestSuck.Item[i][j] && FTestSuck.Suck[i][j].Error==false)
                    {
                        flag=true;
                        break;
                    }
                }
            }

            if(flag==false)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Suck[i][j].Error)
                        {
                            Task=1750;
                            return;
                        }
                    }
                }
            }

            if(FTestSuck.UseSiteHasIC())
                break;

            for(int i=0; i<MAX_Index_Row; i++)
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                    bDuplicateErr[i][j]=false;
            Task=1800;
            break;
        case 1750:
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            iPos=IsNNMode();
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(FTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i+iPos][j];
                        FTestSuck.Suck[i][j].Error=false;                       //Steven 20101229 : 重置位置
                        bDuplicateErr[i][j]=true;                               //Steven 20101229 : 重置位置
                    }
                    else
                    {
                        bDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ1, bHasDuplicateErr, ErrPart);   //Vacuum sensor OFF error

            bHasErr=false;
            Task=1700;
            break;
        case 1800:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, iSpeed, "DoIndexAutoClean 1800"))
            {
                if((iInArmType==e9045_1x3_4 ||                                  //Steven 20220922 : Fixed for 1x3 auto clean
                    iInArmType==e9045_1x3_2_14) &&
                   TestIF_File.iAutoClean_DeveicePices%3==0)
                {
                    FLCarryKit.SetItemData(0, 3, NULL_IC);
                }
                Task=1900;
            }
            break;
        case 1900:
            MOT[MInShuttle1].fCanMoveM=true;
            bInedxCleanFinish[0]=true;                                          //kevin 20170520 (wei) Autoclean index Arm 1完成

            if((CosFunction.bAutoCleanAutoSelIndexArm==false && TestIF.iAutoClean_SelectArm==2) ||
               (CosFunction.bAutoCleanAutoSelIndexArm==true  && TestIF.bCleanIndexOtherArm==true))
            {
                if(CheckAutoCleanCloseSite(1)==false)                           //Steven 20220929 : 判斷開放Site
                {
                    Task=2200;
                    break;
                }
            }

            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                bRunAutoClean=false;
                bAutoCleanFinishOnlyUseRTC=true;                                //JerryYang 20161216 (Steven) 停止auto clean後不需要index check
                iAutoClean_IndexContactCount=0;
                CleanSetSpeed(false);
            }
            break;
        case 2100:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeed, "DoIndexAutoClean 2100"))
            {
                Task=2200;
            }
            break;
        case 2200:
            W906DIAC_FHOME_INITDOTESTZHOME();
            iContactCount=0;
            Task=2300;
            break;
        case 2300:
            if(EP_Install)
            {
                ADAM_WriteVoltage(TestIF.fAutoClean_AireForce);
            }

            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                             //JimmyChiu 20251101 : add Index Arm Axis
            {
                bFlagY1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle, Prod.TestY2_Rear, iSpeedSY, "DoIndexAutoClean 2300");//kevin 20120517 iSpeed);   //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            }
            else
            {
                bFlagY1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, iSpeedSY, "DoIndexAutoClean 2300");//kevin 20120517 iSpeed);   //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            }
            bFlagS2=false;

            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
            {
                if(BLCarryKit.UseSiteHasIC() && InSHT2InRT())
                {
                    MOT[MInShuttle2].fCanMoveM=false;                           //kevin 20170119 index cleanpad會限制到 shuttle的移動
                    bFlagS2=true;
                }
            }
            else if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                bFlagS2=true;
            }

            if(bFlagY1 && bFlagS2)
            {
                MOT[MInShuttle2].fCanMoveM=false;

                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=2400;
                else
                    Task=2650;
            }
            break;
        case 2400:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Pick+TestIF_File.iAutoClean_IndexPickOffset, iSpeed, "DoIndexAutoClean_2400"))         //wei 20150318 Auto clean Index Pick Offset
            {
                DoIndexAutoCleanDelay.SetMSAndOn(500);
                Task=2500;
            }
            break;
        case 2500:
            if(DoIndexAutoCleanDelay.Off())
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        bSuckFinish[i][j]=false;                                //Steven 20110301 : 初始化，尚未完成
                        bTestSuckUse[i][j]=false;
                    }
                }
                BTestSuck.ResetAll();                                           //Steven 20160323 : 避免錯誤殘留
                Task=2600;
            }
            break;
        case 2600:
            flag1=true;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(BLCarryKit.Item[i][j])
                    {
                        if(BLCarryKit.Item[i][j]==HAS_NULL_CLEAN_IC)
                        {
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 已完成不執行
                            BTestSuck.MoveSuckData(BLCarryKit, i, j);
                            bSuckFinish[i][j]=true;                             //Steven 20110301
                        }
                        else if(bSuckFinish[i][j]==true)
                        {
                            if(BTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                               BTestSuck.Item[i][j]==NULL_IC)                   //Steven 20111202 : Retry會重取
                            {
                                BTestSuck.Suck[i][j].Normal();                  //Steven 20111201 : 已完成不執行
                            }
                        }
                        else
                        {
                            #ifdef SOFT_SIMULTE
                            int x=atoi(fMain->edHPY->Text.c_str());
                            int y=atoi(fMain->edHPX->Text.c_str());
                            if(fMain->cbIndexDrop->Checked==true && i==x && j==y)
                            {
                                BTestSuck.Suck[i][j].Error=true;
                                fMain->bAutoCleanTest=false;
                                bSuckFinish[i][j]=true;
                                continue;
                            }
                            #endif

                            if(BTestSuck.Suck[i][j].Suck())
                            {
                                BTestSuck.MoveSuckData(BLCarryKit, i, j);
                                bDuplicateErr[i][j]=false;                      //Steven 20100105
                                bSuckFinish[i][j]=true;                         //Steven 20110301 : 吸嘴完成本次判定
                            }
                            else if(BTestSuck.Suck[i][j].Error)                 //Steven 20110301 : 本次錯誤不判定
                            {
                                bSuckFinish[i][j]=true;
                            }
                            else
                            {
                                flag1=false;                                    //jou 2011-08-16 只要有任何未完成就繼續
                            }
                        }
                    }
                    else
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           BTestSuck.Item[i][j]==NULL_IC)                       //Steven 20111202 : Retry會重取
                        {
                            BTestSuck.Suck[i][j].Normal();                      //Steven 20111201 : 已完成不執行
                        }
                        bSuckFinish[i][j]=true;                                 //Steven 20110301 : 沒有東西的地方要跳過
                    }
                }
            }

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bSuckFinish[i][j]==false)                                //只要有任何未完成就繼續
                        flag1=false;
                }
            }

            if(flag1==true)                                                     //Steven 20110301 : 所有吸嘴皆完成
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            Task=2650;
                            return;
                        }
                    }
                }

                if(BLCarryKit.HasRealIC())
                    break;

                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC)
                        {
                            BTestSuck.Suck[i][j].Normal();
                        }
                        bDuplicateErr[i][j]=false;
                    }
                }

                if(BLCarryKit.UseSiteHasIC())
                    break;

                Task=2700;
            }
            break;
        case 2650:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, iSpeed, "DoIndexAutoClean_2650"))
            {
                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=2651;
                else
                    Task=2900;
            }
            break;
        case 2651:                                                              //Richard 20230418 : add Index Pick Error Can Retry and Start
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];
                    }
                    else
                    {
                        BTestSuck.Suck[i][j].Error=false;
                    }
                }
            }

            if(IniConfig.bD43IndexDropErrorCanRetryandSkip==true && bHasErr==true)
            {
                MOT[MInShuttle2].fCanMoveM=true;
                bAutoCleanShuttle2MoveToLeft=true;
                bAutoCleanShuttle2HasPickErr=true;
                Task=2652;
            }
            else
            {
                Task=2660;
            }
            break;
        case 2652:
            if(IniConfig.bD43IndexDropErrorCanRetryandSkip &&
               bAutoCleanShuttle2MoveToLeft)
            {
                MOT[MInShuttle2].fCanMoveM=true;
                if(InSHT2InLF()!=true)
                {
                    break;
                }
            }
            MOT[MInShuttle2].fCanMoveM=false;
            Task=2660;
        case 2660:
            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;

            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
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
                bHasErr=false;

                if(CosFunction.bJAM0301NeedOpenChamberDoor)                     //wei : JAM0301 & JAM0302需要開啟Chamber門10秒
                {
                    if(DoInArmMoveToWaitPosByAutoClean()==false)                //JerryYang 20241118 : fix auto clean hang up    //Steven 20130613 : Index有異常時, In Arm要有先讓功能
                    {
                        return ;
                    }
                    bIsTestSitICFallDown=true;
                }

                if(IniConfig.bIndexPickErrOnlySKIP==true ||                     //kevin 201701103 (Steven) index pick up error only skip
                   IniConfig.bD64IndexPickErrOnlySKIP)                          //JerryYang 20160301 index pick-up error only skip
                    ret=ShowErrorMessage("JAM0313", K_SKIP, MTestZ2, bHasDuplicateErr, ErrPart);            //Devicr Pick-Up Error                      //JerryYang 20160511 JAM0302->JAM0313,將IC與Clean pad分開的alarm code分開
                else
                    ret=ShowErrorMessage("JAM0313", K_SKIP|K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);    //Steven 20091123 : Devicr Pick-Up Error    //JerryYang 20160511 JAM0302->JAM0313,將IC與Clean pad分開的alarm code分開

                if(ret==K_SKIP)
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BTestSuck.Suck[i][j].Error)
                            {
                                BTestSuck.MoveSuckData(BLCarryKit, i, j);
                                BTestSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC); //Steven 20250326 : fixed for auto clean
                                BTestSuck.Suck[i][j].Normal();
                                bTestSuckUse[i][j]=true;
                            }
                            bDuplicateErr[i][j]=false;
                        }
                    }
                }
                else
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BTestSuck.Suck[i][j].Error)
                            {
                                bDuplicateErr[i][j]=true;
                            }
                        }
                    }
                }
                BTestSuck.ResetAll();                                           //Steven 20160323 : 避免錯誤殘留
                bAutoCleanShuttle2MoveToLeft=false;
                MOT[MInShuttle2].fCanMoveM=true;
            }

            bHasErr=false;
            bAutoCleanShuttle1HasPickErr=false;
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)                    //Clean Kit
            {
                if(IniConfig.bD43IndexDropErrorCanRetryandSkip)
                {
                    Task=2670;
                }
                else
                {
                    if(BLCarryKit.HasRealIC())
                        Task=2400;
                    else
                        Task=2700;
                }
            }
            break;
        case 2670:
            if(InSHT2InRT())
            {
                MOT[MInShuttle2].fCanMoveM=false;
                if(BLCarryKit.HasRealIC())                                      //有IC->Retry
                    Task=2400;
                else
                    Task=2700;                                                  //無IC->Skip
            }
            break;
        case 2700:                                                              //Steven 20160323 : 吸嘴等一下再作動
            DoIndexAutoCleanDelay.SetMSAndOn(500);
            Task=2710;
        case 2710:
            if(DoIndexAutoCleanDelay.Off())
            {
                Task=2720;
            }
            break;
        case 2720:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Safe, iSpeed, "DoIndexAutoClean_2720"))
            {
                Task=2800;
            }
            break;
        case 2800:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(BTestSuck.Suck[i][j].GetStatus()==false)
                        BTestSuck.Suck[i][j].Normal();
                }
            }
            Task=2900;
            break;
        case 2900:
            bFlagY1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle, iSpeedSY, "DoIndexAutoClean 2900");  //kevin 20120517 iSpeed))  //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致

            if(bFlagY1)
            {
                Task=3000;
            }
            break;
        case 3000:
            W906DIAC_FHOME_INITDOTESTZHOME();
            Task=3050;
            break;
        case 3050:                                                              //kevin 20180716 drop contract 分是下高或高低
            if(Prod.iAutoClean_ContactMode==0)                                  //kevin 20180716 direction  //Sam 20230111 : Smart Auto Clean
            {
                 Task=3090;
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Drop[1], iSpeedSZ, "DoIndexAutoClean_3050")) //kevin 20180717 drop high //Steven 20180920 : #P180916-ATK-H9-04 , Auto Clean-Socket Position Offset does not use for Arm2
                    Task=3055;
            }
            break;
        case 3055:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(BTestSuck.Item[i][j]==HAS_CLEAN_IC)
                    {
                        BTestSuck.Suck[i][j].Off();
                    }
                }
            }
            DoTestYRearDelayAC.SetSecAndOn(Prod.TestZ_Drop_Wait);               // delay 0.3 sec for ic down        //Steven 20140909 : 延遲吸放後再作
            Task=3060;
            break;
        case 3060:
            bCheckDestroy=true;
            if(bCheckDestroy==true && DoTestYRearDelayAC.Off())
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_CLEAN_IC)
                            BTestSuck.Suck[i][j].Normal();
                    }
                }
                bIndexCheckNoStopVaccum=false;
                Task=3090;
            }
            break;
        case 3090:                                                              //socket pos kevin 20180630 change
            if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Contact[1], iSpeedSZ, "DoIndexAutoClean_3090"))
            {                                                                   //kevin 20180630 add clean pad - device thickness
                if(Prod.iAutoClean_ContactMode==0)                              //Sam 20230111 : Smart Auto Clean //kevin 20180716 direction
                {
                    iContactCount++;
                    TestIF_File.iIndexArmAutoCleanCnt++;                        //Sam 20250820 : AutoClean 在 Index Arm 下壓時作動次數++
                    DoIndexAutoCleanDelay.Set0_1SecAndOn(Prod.iAutoClean_ContactTime);  //Sam 20230111 : Smart Auto Clean
                    Task=3100;
                }
                else
                {
                    Task=3095;                                                  //drop mode
                }
            }
            break;
        case 3095:
            if(DeviceForm.VacuumMode==VacuumONMode)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(BTestSuck.Item[i][j]==HAS_CLEAN_IC)
                        {
                            BTestSuck.Suck[i][j].On();
                            if(INDEX_SUCKER_TYPE==1)                            //Steven 20111202
                            {
                                fiosetview->bIndexSuck[1][i][j]=true;
                            }
                        }
                    }
                }
            }

            DoTestYRearDelayAC.SetMSAndOn(100);                                 // delay 0.3 sec for ic down
            Task=3096;
            break;
        case 3096:
            if(INDEX_SUCKER_TYPE==1)
            {
                bCheckSuck=W906DIAC_FIOSET_PISD2();
            }
            else
            {
                bCheckSuck=true;
            }

            if(bCheckSuck==true && DoTestYRearDelayAC.Off())
            {
                bFTestSuckDrop=false;
                iContactCount++;
                TestIF_File.iIndexArmAutoCleanCnt++;                            //Sam 20250820 : AutoClean 在 Index Arm 下壓時作動次數++
                DoIndexAutoCleanDelay.Set0_1SecAndOn(Prod.iAutoClean_ContactTime);  //Sam 20230111 : Smart Auto Clean
                Task=3100;
            }
            break;
        case 3100:
            if(DoIndexAutoCleanDelay.Off())
            {
                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].Off();
                if(iContactCount<Prod.iAutoClean_ContactCount)                  //Sam 20230111 : Smart Auto Clean
                {
                    Task=3200;
                }
                else
                {
                    if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(BTestSuck.Item[i][j])
                                {
                                    if(BTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC)
                                        BTestSuck.SetItemData(i, j, CLEAN_FINISH_IC);         //Steven 20130701
                                }
                            }
                        }
                    }
                    Task=3350;
                }
            }
            break;
        case 3200:                                                              //kevin 20180716 add drop contract
            if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Shift[1], iSpeedSZ, "DoIndexAutoClean_3200"))
            {
                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";                                                //Steven 20160323 : Auto Clean完成的資料
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BTestSuck.Suck[i][j].Enable       &&
                               BTestSuck.Suck[i][j].SenUsing!="" &&
                               BTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC &&
                               bTestSuckUse[i][j]==false)
                            {
                                if(BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    BTestSuck.Suck[i][j].Normal();              //jou 2012-01-17 重置吸嘴狀態，避免延誤shuttle釋放，也避免要重測而Hang up
                                    flag=true;
                                    ErrPart+=IndexSuckName[i][j];
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ2].Gali_Command("ST", __FUNC__+AnsiString(",case 3200"));

                    if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    {
                        for(int i=0; i<MAX_Index_Row; i++)
                        {
                            for(int j=0; j<NEW_MAX_Index_Col; j++)
                            {
                                if(BTestSuck.Item[i][j])
                                {
                                    if(BTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC)
                                        BTestSuck.SetItemData(i, j, CLEAN_FINISH_IC);         //Steven 20130701
                                }
                            }
                        }
                    }
                    DoTestYRearDelayAC.SetMSAndOn(100);
                    Task=3360;
                    break;
                }

                if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
                    SW[SwSocketClean].On();
                DoTestYRearDelayAC.SetMSAndOn(100);
                Task=3210;
            }
            break;
        case 3210:
            if(DoTestYRearDelayAC.Off())
            {
                Task=3050;
            }
            break;
        case 3350:
            if(MOT[MTestZ2].Gali_MotMove(Prod.iAutoCleanZ_Shift[1], iSpeedSZ, "DoIndexAutoClean_3350"))  //JerryYang 20181119 (Steven) : fix auto clean 高度異常     //kevin 20181016 add drop 高低
            {
                for(int i=0; i<MAX_Index_Row; i++)                              //ChungHung 20150407 add for SCK drop issue
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        bDuplicateErr[i][j]=false;
                        bIsSuckICFallDown[i][j]=false;                          //kevin 20150624
                    }
                }
                Task=3360;
                DoTestYRearDelayAC.SetMSAndOn(100);
            }
            break;
        case 3360:
            if(DoTestYRearDelayAC.Off())
            {
                Task=3400;
            }
            break;
        case 3400:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeed, "DoIndexAutoClean 3400"))
            {
                Task=3500;

                flag=false;
                if(LastSet.iRealDummy==REALLY)
                {
                    ErrPart=" ";                                                //Steven 20160323 : Auto Clean完成的資料
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BTestSuck.Suck[i][j].Enable       &&
                               BTestSuck.Suck[i][j].SenUsing!="" &&
                               BTestSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                               BTestSuck.Item[i][j]!=NULL_IC &&
                               bTestSuckUse[i][j]==false)
                            {
                                if(BTestSuck.Suck[i][j].GetStatus()==false)
                                {
                                    BTestSuck.Suck[i][j].Normal();              //jou 2012-01-17 重置吸嘴狀態，避免延誤shuttle釋放，也避免要重測而Hang up
                                    flag=true;
                                    ErrPart+=IndexSuckName[i][j];
                                    bIsSuckICFallDown[i][j]=true;               //kevin 20150624
                                }
                            }
                        }
                    }
                }

                if(flag)
                {
                    MOT[MTestZ2].Gali_Command("ST", __FUNC__+AnsiString(",case 3400"));
                    Task=3410;
                }
            }
            break;
        case 3410:                                                              //Jimmychiu 20211022 : #R210901-ATK-H9-03 , The index Arm move on the shuttle when drop error during auto clean.
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front,Prod.TestY2_Rear, iSpeed, __FUNC__+AnsiString("case 3410")))
                Task=3420;
            break;
        case 3420:
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            if(CosFunction.bJAM0303NeedOpenChamberDoor)                         //Steven : JAM0303 & JAM0403需要開啟Chamber門10秒
                bIsTestSitICFallDown=true;                                      //kevin 20130706

            iRet=ShowErrorMessage("JAM0315", K_SKIP, MTestZ2, false, ErrPart);  //Steven 20100129 : Device Drop Error    //JerryYang 20160511 JAM0304->JAM0315,將IC與Clean pad分開的alarm code分開

            if(iRet==K_SKIP)                                                    //kevin 20150624 add
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(bIsSuckICFallDown[i][j])                             //kevin 20150624
                            BTestSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);     // 掉料歸零
                    }
                }
            }
            Task=3500;
            break;
        case 3500:
            if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                             //JimmyChiu 20220708 : add Index Arm Axis
            {
                flag=MOT[MTestY1].Gali_MotMove(Prod.TestY1_Middle, iSpeedSY, "DoIndexAutoClean_3500");   //kevin 20120517 iSpeed))
            }
            else
            {
                flag=MOT[MTestY2].Gali_MotMove(Prod.TestY2_Rear, iSpeedSY, "DoIndexAutoClean_3500");     //kevin 20120517 iSpeed))  //Isaac 20200203 : iSpeedSY,認為要AutoCleanIndex速度要一致
            }

            if(flag)
            {
                if(TestIF_File.iAutoClean_Tray!=eCKPos_CleanAir)
                    Task=3600;
                else
                    Task=3800;
            }
            break;
        case 3600:
            if(MOT[MTestZ2].Gali_MotMove(Prod.TestZ2_Place+TestIF_File.iAutoClean_IndexReleaseOffset, iSpeed, "DoIndexAutoClean_3600"))  //Jou 2015-08-22 Auto clean Index Release Offset
            {
                Task=3700;
            }
            break;
        case 3700:
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(BTestSuck.Item[i][j])
                    {
                        if(BTestSuck.Item[i][j]==HAS_NULL_CLEAN_IC ||
                           BTestSuck.Suck[i][j].Destroy())
                        {
                            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanKit)    //Clean Kit
                                BLCarryKit.MoveSuckData(BTestSuck, i, j);
                        }
                    }
                }
            }
            flag=false;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(BTestSuck.Item[i][j] &&
                       BTestSuck.Suck[i][j].Error==false)
                    {
                        flag=true;
                        break;
                    }
                }
            }

            if(flag==false)
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(BTestSuck.Suck[i][j].Error)
                        {
                            Task=3750;
                            return;
                        }
                    }
                }
            }

            if(BTestSuck.UseSiteHasIC())
                break;

            for(int i=0; i<MAX_Index_Row; i++)
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                    bDuplicateErr[i][j]=false;
            Task=3800;
            break;
        case 3750:                                                              //開料異常處理
            if(DoInArmMoveToWaitPosByAutoClean()==false)                        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                return;

            ErrPart=" ";
            bHasErr=false;
            bHasDuplicateErr=false;
            for(int i=0; i<MAX_Index_Row; i++)
            {
                for(int j=0; j<NEW_MAX_Index_Col; j++)
                {
                    if(bDuplicateErr[i][j])
                        bHasDuplicateErr=true;

                    if(BTestSuck.Suck[i][j].Error)
                    {
                        bHasErr=true;
                        ErrPart+=IndexSuckName[i][j];
                        BTestSuck.Suck[i][j].Error=false;                       //Steven 20101229 : 重置位置
                        bDuplicateErr[i][j]=true;                               //Steven 20101229 : 重置位置
                    }
                    else
                    {
                        bDuplicateErr[i][j]=false;
                    }
                }
            }

            if(bHasErr)
                ShowErrorMessage("JAM0327", K_RETRY, MTestZ2, bHasDuplicateErr, ErrPart);   //Vacuum sensor OFF error

            bHasErr=false;
            Task=3700;
            break;
        case 3800:
            if(MOT[MTestZ2].Gali_Two_ZAxis_Move(Prod.TestZ2_Safe, iSpeed, "DoIndexAutoClean 3800"))
            {
                if((iInArmType==e9045_1x3_4 ||                                  //Steven 20220922 : Fixed for 1x3 auto clean
                    iInArmType==e9045_1x3_2_14) &&
                   TestIF_File.iAutoClean_DeveicePices%3==0)
                {
                    BLCarryKit.SetItemData(0, 3, NULL_IC);
                }
                Task=3900;
            }
            break;
        case 3900:
            MOT[MInShuttle2].fCanMoveM=true;
            bInedxCleanFinish[1]=true;                                          //kevin 20170520 (wei) Autoclean index Arm 2完成
            if(TestIF_File.iAutoClean_Tray==eCKPos_CleanAir)
            {
                bRunAutoClean=false;
                bAutoCleanFinishOnlyUseRTC=true;                                //JerryYang 20161216 (Steven) 停止auto clean後不需要index check
                iAutoClean_IndexContactCount=0;
                CleanSetSpeed(false);
            }
            break;
    }

    #ifdef DEBUG_AUTO_CLEAN
    static int iOldTask=-1;
    if(iOldTask!=Task)
    {
        Message.sprintf("DoIndexAutoClean, %d, Go to Task, %d", iOldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        iOldTask=Task;
    }
    #endif
}

//==============================================================================
//  W906-AutoCleanCluster (20260722) -- Part H: master orchestrator
//==============================================================================
//------------------------------------------------------------------------------
void DoAutoCleanKit()                                                           //ChungHung 20130701 add 修改AutoClean 流程
{
    AnsiString Message;
    int ret=0;
    int &Task=iDoAutoCleanTask;
    int flag=0, iCleanPadCount=0;
    if((bLockPlaceToShuttleByAutoClean ||
        bLockPickFromShuttleByAutoClean) &&
       bPlaceToCleanKit==false &&
       bPickFromKitByAutoClean==false)                                          //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion    //Ifor 20191104 : fix 雙Arm Auto Clean InArm 資料在Clean Kit Index2 Drop 要InArm 互相互卡 Hangup
        return;

    if(W906_FormShowing("fContact", fContact->fShow)==true && iContactMode==CONTACT_DEVICE_MAP_CHECK)         //Steven 20220510 : For QTI SD Device Map Function  //AI(W906-PAGETAB-Q51) 20260928 [W906] 批2：golden「這個畫面開著嗎」改問頁面表的單一函式 W906_FormShowing（成員照傳；Steven Q51／Q-P3=A 直接生效）
        return;

    static bool bShakeFlag, bVibration;                                         //Steven 20210616 : Auto Clean也要振動

    bool bIsSuckICFallDown[MAX_SOCKET_ROW][MAX_SOCKET_COL]={false}, bHasErr=false;
    AnsiString ErrPart="";
    bool bSupport2Arm;

    if(IniConfig.bF16CheckShuttleSensorBroken && bDoingF16)                     //Steven 20221213 : 確認shuttle 有沒有斷線
        return;

    int iSuckRow=0;                                                             //ChungHung 20141121 add for Use HotPlate AutoClean

    bool bDoAutoCleanAutoAlignment;
    if(USE_IN_Y_IS_AUTO_PITCH==true && CUSTOMER_CODE==CC_ASE_KaohSiung)         //JerryYang 20251218 : IN/OUT ARM支援不同模式      //KenHsieh 20220103 : 新增ASEKH專用
    {
        if(MACHINE_HAS_AUTO_ALIGNMENT_CCD && TestIF.bEnableAutoAlignment==true &&
           (LastSet.iRealDummy==HAS_TRAY || LastSet.iRealDummy==REALLY))        //KenHsieh 20211214 : AOA add AutoClean
        {
            if(lInArmAutoAlignmentCKTimingFlag || bRunInArmAutoAlignment)
            {
                if(iCleanOut==1 && HasICUnderMachine()==false)                  //process clean out ,don't supply new tray
                {
                    return;
                }
                else
                {
                    bDoAutoCleanAutoAlignment=CheckInArmAutoAlignmentCKModeBeUse(lInArmAutoAlignmentCKTimingFlag, true);
                    bRunInArmAutoAlignment=bDoAutoCleanAutoAlignment;
                    return;
                }
            }
        }
        else
        {
            bRunInArmAutoAlignment=false;
        }
    }

    bUse8Picker=false;                                                          //Steven 20201014 : 拆出8吸嘴auto clean
    if(IniConfig.bE43AutoCleanUseHotplate)
    {
        bUse8Picker=true;
    }
    else if(MachineTypeChoice==Type_HT9046_LS ||
            MachineTypeChoice==Type_HT1032)
    {
        if(SubMachineType==Type_None)
            bUse8Picker=true;
        else if(USE_IN_Y_IS_AUTO_PITCH==true)                                   //JerryYang 20251218 : IN/OUT ARM支援不同模式
            bUse8Picker=true;
    }

    if(IniConfig.bE43AutoCleanUseHotplate)
        iSuckRow=0;
    else
        iSuckRow=1;

    if(DoInArmPineRelease()==false)
        return;

    SocketAirCoolingStart();                                                    //jou 2016-04-28 Socket Air Cooling contact count trun on
    switch(Task)
    {
        case 1:
            if(FIX3_FULL_PLACE==Fix3K_UseCylinder &&
               bUseFix3CylinderActive==true)                                    //Ifor 20210226 Fix: Auto Clean Hang up
            {
                InitialFix3CanFullTask();
                Task=2;
            }
            else
            {
                DoStructUnitConvert();
                DoInArm_9045_Type();                                            //Steven 20201014 : 將DoInArm_9045_Type上移,避免DoInArm_9045還沒執行就被使用
                DoInArm_SuckerMap();                                            //JerryYang 20190805 add
                Cylinder[C_HotplateVibration].Off();                            //JerryYang 20191123 fix auto clean過程中震動震動器持續震動
                Cylinder[C_TrayVibration].Off();
                PlaceToCleanList->ClearGroupList();                             //JerryYang 20241219 : initial data避免資料沒清乾淨

                MOT[MInShuttle1].fCanMoveM=true;                                //Steven 20220506 : 避免shuttle被鎖住,導致Auto Clean異常
                MOT[MInShuttle2].fCanMoveM=true;
                // AI(W906-AutoCleanCluster) 20260722: golden is
                // `ZeroMemory(CleanKitRecord, sizeof(CleanKitRecord));` -- a raw
                // memset over a `AnsiString CleanKitRecord[50][50]` array
                // (cmydef.h:5569). Same hazard class as the DoTestIFConvert
                // memcpy discussed above: in THIS tree AnsiString is backed by
                // std::string (a real heap-owning object), so zero-filling its
                // raw bytes bypasses the destructor and corrupts the object's
                // internal SSO/heap-pointer state instead of giving it the
                // empty string golden intends. Unlike DoTestIFConvert, the safe
                // equivalent here is a trivial, scope-faithful one-liner (loop
                // + plain AnsiString assignment) -- not a gate.
                for(int iCKRy=0; iCKRy<50; iCKRy++)
                    for(int iCKRx=0; iCKRx<50; iCKRx++)
                        CleanKitRecord[iCKRy][iCKRx]="";                        //Sam 20230619 : 新增 Clean吸嘴時間 Log
                if(CosFunction.bFullTestBeforeAutoClean &&                      //JerryYang 20160331 SPIL國內要求Auto clean前做 full view check
                   REAL_TIME_CCD==true &&
                   !COM2->bCCDDummyRum)
                {
                    RecordProcess("Start Full View Check...");
                    fContact->InitDoFullViewCheck();
                    bFullViewCheckFinish=false;
                    Task=3;                                                     //full view check
                }
                else
                {
                    bFullViewCheckFinish=true;
                    Task=5;                                                     //auto clean
                }
            }
            break;
         case 2:
            if(UseFix3Cylinder(0)==true)
            {
                Task=1;
            }
            break;
        case 3:
            if(fContact->DoFullViewCheck()==true)
            {
                bFullViewCheckFinish=true;
                Task=5;
            }
            break;
        case 5:
            GetXPitchOfCleanKit();
            RecordProcess("Start Auto Cleaning...");                            //Steven 20130614 : Auto Clean加上紀錄
            bAutoCleaning=true;                                                 //JerryYang 20151109 add for 記錄 AutoClean出現
            SW[SwTesterAirCooling].Off();                                       //jou 2016-04-28 Socket Air Cooling contact count trun on
            iL03SocketAirCoolingCT=0;                                           //jou 2016-04-28 Socket Air Cooling contact count trun on
                                                                                //Ifor 20170425 (wei) Auto Clean Start 傳送次數與Auto Clean End 無法搭配 InitialAutoCleanTask 造成 DoAutoCleanKit 流程異常
            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.DoAutoClean);                            //34     Auto Clean Start
            #ifdef DEBUG_AUTO_CLEAN
                Message.sprintf("\\--------------- DoAutoCleanKit - Start Auto Clean");
                fMain->AddAutoCleanMessage(Message);
            #endif

            iCleanPadCount=TrayHasCleanICCount();                               //wei 20150904
            if(iCleanPadCount>=TestIF_File.iAutoClean_DeveicePices)
            {
                Task=20;
            }
            else
            {
                if((TestIF_File.iTestMode==_6Site2X3 && iCleanPadCount>=6) ||   //Steven 20171220 (Wei) : Fixed for 2x3 Auto Clean Hang Up
                   (TestIF_File.iTestMode==_10Site2X5 && iCleanPadCount>=10))   //KEVIN 20220113 ADD 2x5 site
                {
                    Task=20;
                    break;
                }

                bErrorAutoClean=true;
                Task=2000;
            }
            break;
        case 10:                                                                //Steven 20130620 : 避免Clean Count到的時候，貨到Arm導致Hang Up，回傳值從bool改為int
            InitialShuttleAutoCleanTask();
            InitialIndexAutoCleanTask();
            Task=20;
        case 20:
            if(MoveInArmZToPlateSafe(Task))                                     //in arm z軸移至安全位置
            {
                InitialSet();                                                   //kevin 20150720

                if(CosFunction.bAutoCleanAutoSelIndexArm==false)
                {
                    if(TestIF.iAutoClean_SelectArm!=1)                          //ChungHung 20131218 add for SCK request  //0:Arm1 1:Arm2 2:Arm1 & Arm2
                        MOT[MInShuttle1].fCanMoveL=true;
                    if(TestIF.iAutoClean_SelectArm!=0)
                        MOT[MInShuttle2].fCanMoveL=true;

                    if(TestIF.iAutoClean_SelectArm==1)
                    {
                        Task=2100;
                    }
                    else
                    {
                        Task=100;
                    }

                    if(TestIF_File.iAutoClean_SelectArm==2)                     //wei 20220801 2x3 NN Mode
                    {
                        if(CheckAutoCleanCloseSite(0))                          //Steven 20220929 : 判斷開放Site
                        {
                            Task=2100;
                        }
                        else
                        {
                            Task=100;
                        }
                    }
                }
                else
                {
                    MOT[MInShuttle1].fCanMoveL=true;
                    if(TestIF.bCleanIndexOtherArm==true)
                        MOT[MInShuttle2].fCanMoveL=true;

                    if(TestIF.bCleanIndexOtherArm==false &&
                       TestIF_File.iShuttleMode==1 &&
                       TestIF_File.iShuttle_Sel==1)
                    {
                        Task=2100;
                    }
                    else
                    {
                        Task=100;
                    }
                }
            }
            break;
        case 100:
            if(CheckInArmSuckInitial()==false)                                  //In Arm 吸嘴上如果有 Device 就 Alarm
                break;

            if(InArmSuck.HasIC()==false                &&                       //Steven 20130620 : Auto Clean做一半的糗事要繼續作
               b1ShuttleMoveToLeft==true               &&                       //ChungHung 20131120 AutoClean use Hotplate1
               (FTestSuck.HasDefineIC(HAS_CLEAN_IC)    ||
                FTestSuck.HasDefineIC(CLEAN_FINISH_IC) ||
                FLCarryKit.HasDefineIC(HAS_CLEAN_IC)   ||
                FLCarryKit.HasDefineIC(CLEAN_FINISH_IC)))
            {
                Task=540;
            }
            else
            {
                if(InArmSuck.HasDefineIC(CLEAN_FINISH_IC))
                {
                    Task=1100;
                }
                else if(InArmSuck.HasDefineIC(HAS_CLEAN_IC))
                {
                    InitPlaceToShuttleTask();
                    Task=400;
                }
                else
                {
                    Task=200;
                }
            }
            break;
        case 200:
            DoAutoCleanPickfromCleanKit(euShuttle1, true);                      //initial Task
            Task=300;
            break;
        case 300:
            bPickFromKitByAutoClean=true;                                       //Steven 20210603 : 新增吸取Clean Kit的Flag
            flag=DoAutoCleanPickfromCleanKit(euShuttle1, false);
            if(flag==1)
            {
                bPickFromKitByAutoClean=false;                                  //Steven 20210603 : 新增吸取Clean Kit的Flag
                if(InArmSuck.HasIC()==false)
                {
                    Task=200;                                                   //沒吸到重來
                    if(FLCarryKit.FindNoIC()==false)                            //kevin 20180425 SHUTTLE 有NULL IC
                        Task=530;
                }
                else
                {
                    InitPlaceToShuttleTask();                                   //吸到了放到shuttle上
                    Task=400;
                }
            }
            else if(flag==2)                                                    //Steven 20130620 : 避免Clean Count到的時候，貨到Arm導致Hang Up，回傳值從bool改為int
            {
                bPickFromKitByAutoClean=false;                                  //Steven 20210603 : 新增吸取Clean Kit的Flag
                Task=10;
            }
            break;
        case 400:
            bPlaceToShuttleByAutoClean=true;                                    //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
            if(DoPlaceToShuttle(euShuttle1))
            {
                bPlaceToShuttleByAutoClean=false;                               //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                iShuttleRowKit=GetShuttleState(euShuttle1, bAutoPlace);

                if(USE_PICKER_COUNT==ep1Picker)
                {
                    if(IsFLCarrKitAllHasIC()==false)
                    {
                        Task=200;
                    }
                    else
                    {
                        Task=500;
                    }
                }
                else
                {
                    if(iShuttleRowKit!=0)
                    {
                        Task=200;                                                   //Shuttle 沒放滿要，則Clean Kit吸Device
                    }
                    else
                    {
                        Task=500;
                    }
                }
            }
            break;
        case 500:                                                               //Pick from shuttle ,place to Clean Kit
            if(MoveInArmZToPlateSafe(Task))
            {
                bShakeFlag=false;
                bVibration=false;                                               //Steven 20210616 : Auto Clean也要振動
                if(IniConfig.bF01ShakeShuttleWhenJam==false && IniConfig.bF23ShuttleVibration==false)
                    bShakeFlag=true;
                Task=510;
            }
            break;
        case 510:
            if(CosFunction.bAutoCleanShuttleDisable==true)                      //jou 2013-02-27 Auto Clean disable shuttle sensor detect
            {
                if(IniConfig.bAutoCleanShuttleDisable)                          //jou 2013-02-27 Auto Clean disable shuttle sensor detect
                {
                    Task=515;
                    break;
                }
            }
            else
            {
                if(iAutoCleanShuttle==0)
                {
                    Task=515;
                    break;
                }
            }

            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20250923 : Auto clean for InSht sensor Latch 判定
            {
                InitAutoChkInSHLatchTask();
                Task=512;
                break;
            }

            ret=CheckShuttleSensor_Clean(euShuttle1, bShakeFlag);
            if(ret==1)
            {
                bShakeFlag=false;
                bVibration=false;                                               //JerryYang 20180711 (wei) Shuttle振動後還是置放好不會振動
            }
            else if(ret==3)
            {
                Task=511;                                                       //Steven 20220728 : 應該重執行
                break;
            }
            else if(ret==2)
            {
                if(IniConfig.bF23ShuttleVibration &&                            //Steven 20210616 : Auto Clean也要振動
                   ((IniConfig.bF01ShakeShuttleWhenJam==true && bVibration==false) ||
                     IniConfig.bF01ShakeShuttleWhenJam==false))                 //JerryYang 20180711 (wei) Shuttle振動後還是置放好不會振動
                {
                    if(IniConfig.bF01ShakeShuttleWhenJam==true)
                    {
                        bShakeFlag=false;
                    }
                    else
                    {
                        bShakeFlag=true;
                    }
                    bVibration=true;
                    Task=522;
                    DoVibrateShuttle(euShuttle1, true);                         //JerryYang 20190123 shuttle振動做動
                }
                else if(bShuttleKnock==false)                                   //Jou 2013-03-08 修改敲敲功能
                {
                    if(IniConfig.bF01ShakeShuttleWhenJam==false &&
                       IniConfig.bF23ShuttleVibration==false)                   //kevin 20190731 不使用敲敲
                    {
                        bShakeFlag=true;
                        break;
                    }
                    Task=520;
                    DoShakeShuttle(euShuttle1, true);                           //Steven 20120801 : 修改搖搖功能 (true為初始化)
                }
                else
                {
                    bShakeFlag=true;
                    DoKnockShuttle(euShuttle1, true);                           //jou 2013-07-17 Knock Shuttle(true為初始化)
                    Task=521;
                }
                break;
            }
            else
            {
                Task=515;
            }
            break;
        case 511:                                                               //Steven 20220728 : 應該重執行
            Task=510;
            break;
        case 515:
            MOT[MInShuttle1].fCanMoveL=true;

            if(CosFunction.bAutoCleanAutoSelIndexArm==false)
            {
                if(TestIF.iAutoClean_SelectArm==0)
                {
                    Task=540;
                }
                else if(TestIF.iAutoClean_SelectArm==2)
                {
                    if(CheckAutoCleanCloseSite(1)==false)                       //Steven 20220929 : 判斷開放Site
                    {
                        if(MOT[MMAutoCleanKit].Tray.HasDataIC(HAS_CLEAN_IC))    //Steven 20210315 : 修正雙arm auto clean只有一半有clean pad
                            Task=2100;
                        else
                            Task=540;
                    }
                    else
                    {
                        Task=540;
                    }
                }
            }
            else
            {
                if(TestIF.bCleanIndexOtherArm==false)                           //使用該arm可能auto clean可能one arm or two arm可能
                {
                    Task=540;
                }
                else
                {
                    if(CheckAutoCleanCloseSite(1)==false &&                     //Steven 20220929 : 判斷開放Site
                       MOT[MMAutoCleanKit].Tray.HasDataIC(HAS_CLEAN_IC))        //Steven 20210315 : 修正雙arm auto clean只有一半有clean pad
                        Task=2100;
                    else
                        Task=540;
                }
            }
            break;
        case 520:
            if(DoShakeShuttle(euShuttle1))                                      //Steven 20120801 : 修改搖搖功能
            {
                Task=530;
            }
            break;
        case 521:                                                               //Steven 20210616 : Auto Clean也要振動
            if(DoKnockShuttle(euShuttle1))                                      //Jou 2013-03-08 修改敲敲功能
            {
                Task=530;
            }
            break;
        case 522:                                                               //Steven 20210616 : Auto Clean也要振動
            if(DoVibrateShuttle(euShuttle1))                                    //JerryYang 20190123 shuttle振動做動
            {
                Task=530;
            }
            break;
        case 530:
            if(InSHT1InLF())
            {
                MOT[MInShuttle1].fCanMoveL=false;
                bShuttleShake=false;
                IndexZCanMove[0]=true;
                IndexZCanMove[1]=true;
                CleanSetSpeed(true);
                Task=510;
            }
            break;
        case 540:
            // AI(W906-FLOW-2) 20260928: GATE (W7d-I1) retired -- golden AutoClean.cpp:4895 restored (premise dead, see DoPlaceToShuttle case 100)
            if(MoveInArmXYToShuttle_9045(euShuttle1, 0, ZAxisNotDown, false))
            {
                Task=600;
            }
            break;
        case 600:
            if(FLCarryKit.UseSiteNoIC() &&
               InSHT1InLF()              &&
               FTestSuck.UseSiteNoIC())
            {
                Task=1200;
            }
            else
            {
                if(InSHT1InLF() &&                                              //Steven 20190212 : 修正Hang up
                   bInedxCleanFinish[0])                                        //Steven 20200730 : 修正Hang up
                    iShuttleRowKit=GetShuttleState(euShuttle1, bAutoPick);
                else
                    iShuttleRowKit=0;

                if(iShuttleRowKit!=0 && InSHT1InLF() &&
                   (FLCarryKit.HasType(CLEAN_FINISH_IC) ||                      //JerryYang 20160123 修正做index arm完成後,shuttle只剩下HAS_NULL_IC會hang up //JerryYang 20151124 Fix hang up,Shuttle上有 CLEAN_FINISH_IC才能吸取CleanPad
                    (bInedxCleanFinish[0] && FLCarryKit.HasType(HAS_NULL_CLEAN_IC))))//kevin 20170518 (wei) add 全部IC掉料Hang up
                {
                    MOT[MInShuttle1].fCanMoveL=false;
                    InitPickFromShuttleTask();
                    if(CUSTOMER_CODE==CC_KYEC_LEE ||
                       CosFunction.bHiSiliconFunction)
                    {
                        bShakeFlag=false;
                        if(IniConfig.bF01ShakeShuttleWhenJam==false &&
                           IniConfig.bF23ShuttleVibration==false)
                            bShakeFlag=true;
                        Task=610;
                    }
                    else
                    {
                        Task=800;
                    }
                }
            }
            break;
        case 610:
            if(CosFunction.bAutoCleanShuttleDisable==true)                      //jou 2013-02-27 Auto Clean disable shuttle sensor detect
            {
                if(IniConfig.bAutoCleanShuttleDisable)                          //jou 2013-02-27 Auto Clean disable shuttle sensor detect
                {
                    Task=800;
                    break;
                }
            }
            else
            {
                if(iAutoCleanShuttle==0)
                {
                    Task=800;
                    break;
                }
            }

            ret=CheckShuttleSensor_Clean(euShuttle1, bShakeFlag);
            if(ret==1)
            {
                bShakeFlag=false;
                if(IniConfig.bF01ShakeShuttleWhenJam==false &&
                   IniConfig.bF23ShuttleVibration==false)
                    bShakeFlag=true;
            }
            else if(ret==3)
            {
                break;
            }
            else if(ret==2)
            {
                Task=620;
                bShakeFlag=true;
                DoShakeShuttle(euShuttle1, true);                               //Steven 20120801 : 修改搖搖功能 (true為初始化)
                break;
            }
            else
            {
                Task=800;
            }
            break;
        case 620:
            if(DoShakeShuttle(euShuttle1))                                      //Steven 20120801 : 修改搖搖功能
            {
                Task=630;
            }
            break;
        case 630:
            if(InSHT1InLF())
            {
                MOT[MInShuttle1].fCanMoveL=false;
                bShuttleShake=false;
                IndexZCanMove[0]=true;
                IndexZCanMove[1]=true;
                CleanSetSpeed(true);
                Task=610;
            }
            break;
        case 800:
            bPickFromShuttleByAutoClean=true;                                   //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
            if(DoPickFromShuttle(euShuttle1, iShuttleRowKit))
            {
                if(bUse8Picker && iCloseSiteModeFor2x8==e2x8_STMMode ||         //JerryYang 20191122 STM 8 site Auto clean支援前後資料一起做
                                  iCloseSiteModeFor2x8==e2x8_TW153Mode)
                {
                    if(FLCarryKit.LeftSideNoIC(4)==true && FLCarryKit.RightSideNoIC(4)==false)
                    {
                        iShuttleRowKit=GetShuttleState(euShuttle1, bAutoPick);
                        break;
                    }
                }

                bPickFromShuttleByAutoClean=false;                              //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                Task=1000;
            }
            break;
        case 1000:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=1050;
            }
            break;
        case 1050:
            if(LastSet.iRealDummy==REALLY)
            {
                ptrInSHT=&FLCarryKit;

                if(iShuttleRowKit==1 || iShuttleRowKit==2)                      //Kit 1
                    ret=0;
                else                                                            //kit 2
                    ret=4;

                for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                   //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Suck[i][j].Enable       &&
                           InArmSuck.Suck[i][j].SenUsing!="" &&
                           InArmSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                           InArmSuck.Item[i][j]!=NULL_IC)
                        {
                            if(InArmSuck.Suck[i][j].GetStatus()==false)
                            {
                                bIsSuckICFallDown[i][j]=true;
                                ErrPart+=InArmSuck.Suck[i][j].sName;
                                bHasErr=true;
                            }
                        }
                    }
                }
            }

            if(bHasErr)
            {
                ret=ShowErrorMessage("JAM0128", K_SKIP, MInArmX, false, ErrPart); //Steven 20091123 : Device Drop Error //JerryYang 20160511 JAM0126->JAM0128,將IC與Clean pad分開的alarm code分開

                for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                   //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Suck[i][j].Enable       &&
                           InArmSuck.Suck[i][j].SenUsing!="" &&
                           InArmSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                           InArmSuck.Item[i][j]!=NULL_IC     &&
                           InArmSuck.Suck[i][j].GetStatus()==false)
                        {
                            InArmSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);
                            bIsSuckICFallDown[i][j]=true;                       //ChungHung 20141121 add for Use HotPlate AutoClean
                        }
                    }
                }

                if(SoftStop)
                    fMain->Pause("DoAutoCleanKit 1050");

                Task=1100;                                                      //JerryYang 20160125 Device drop error only skip
            }
            else
            {
                Task=1100;
            }
            break;
        case 1100:
            if(InArmSuck.HasIC()==false)
            {
                Task=600;
                break;
            }
            MOT[MInShuttle1].fCanMoveL=true;                                    //ChungHung 20150410 add because case 2600 lock here need unlock
            DoAutoCleanPlaceToCleanKit(true);
            if(TestIF_File.iAutoClean_Tray==eCKPos_Fix3)                        //JerryYang 20161219 (Steven) 避免Auto clean hang up
            {
                if(iCatchTrayControlManual>=2)                                  // catch tray busy
                    break;
            }
            Task=1200;
            break;
        case 1200:
            bPlaceToCleanKit=true;                                              //Steven 20171204 (Wei) : 避免做一半要放不回原本位置放
            if(DoAutoCleanPlaceToCleanKit(false))
            {
                if(InArmSuck.HasIC()==false)
                {
                    if(FLCarryKit.UseSiteHasIC())                               //JerryYang 20160123 修正做index arm完成後,shuttle只剩下HAS_NULL_IC會hang up
                    {
                        Task=600;
                    }
                    else
                    {
                        if(CosFunction.bAutoCleanAutoSelIndexArm==false)
                        {
                            if(TestIF.iAutoClean_SelectArm!=2)                  //ChungHung 20131218 add for SCK request  //0:Arm1 1:Arm2 2:Arm1 & Arm2
                            {
                                Task=2000;
                            }
                            else
                            {
                                if(CheckAutoCleanCloseSite(1))                  //wei 20220801 2x3 NN Mode   //Steven 20220929 : 判斷開放Site
                                {
                                    Task=2000;
                                }
                                else
                                {
                                    Task=1300;
                                }
                            }
                        }
                        else
                        {
                            if(TestIF.bCleanIndexOtherArm==false ||
                               CheckAutoCleanCloseSite(1)==true)                //Steven 20220929 : 判斷開放Site
                            {
                                Task=2000;
                            }
                            else
                            {
                                Task=1300;
                            }
                        }
                    }
                    bPlaceToCleanKit=false;                                     //Steven 20171204 (Wei) : 避免做一半要放不回原本位置放
                }
                else
                {
                    Task=1000;
                }

                if(Task==1300)                                                  //Steven 20220926 : Auto Clean先做Arm 1後才吸取動作
                {
                    if(BLCarryKit.UseSiteNoIC() &&
                       BTestSuck.UseSiteNoIC())
                    {
                        Task=2100;
                    }
                }
            }
            break;
        case 1300:
            // AI(W906-FLOW-2) 20260928: GATE (W7d-I1) retired -- golden AutoClean.cpp:5156 restored (premise dead, see DoPlaceToShuttle case 100)
            if(MoveInArmXYToShuttle_9045(euShuttle2, 0, ZAxisNotDown, false))
            {
                // AI(W906-AutoCleanCluster) 20260722: golden `fCleaning->
                // CleanPadCountCanSupport2Arm()` -- Wave-14 Part D translated
                // CleanPadCountCanSupport2Arm as a FREE function (fCleaning
                // demoted, same as SetDeviceInTray above); called unqualified.
                bSupport2Arm=CleanPadCountCanSupport2Arm();                     //Steven 20221006 : 修正雙arm auto clean只有一半的clean pad
                if(bSupport2Arm)
                    Task=2600;
                else
                    Task=2100;
            }
            break;
        case 2000:
            if(FLCarryKit.HasType(HAS_CLEAN_IC) || FLCarryKit.HasType(HAS_NULL_CLEAN_IC) ||    //JerryYang 20220216 : 新增保護, 避免auto clean流程結束的時候 suhttle或index arm還有clean pad造成清除clean pad前的訊息
               BLCarryKit.HasType(HAS_CLEAN_IC) || BLCarryKit.HasType(HAS_NULL_CLEAN_IC) ||
               FTestSuck.HasType(HAS_CLEAN_IC)  || FTestSuck.HasType(HAS_NULL_CLEAN_IC) ||
               BTestSuck.HasType(HAS_CLEAN_IC)  || BTestSuck.HasType(HAS_NULL_CLEAN_IC))
            {
                fAllMotorHome=false;
                ShowMyMessage("Auto clean process error, need home!");
                break;
            }

            if(CosFunction.bAutoCleanAutoSelIndexArm==false)
            {
                if(TestIF.iAutoClean_SelectArm!=1)
                    MOT[MInShuttle1].fCanMoveL=true;
                if(TestIF.bCleanIndexOtherArm!=0)
                    MOT[MInShuttle2].fCanMoveL=true;
            }
            else
            {
                MOT[MInShuttle1].fCanMoveL=true;
                if(TestIF.bCleanIndexOtherArm==true)
                    MOT[MInShuttle2].fCanMoveL=true;
            }

            if(IniConfig.bA81WaitSECS==true)                                    //KevinCheng 20250919 : Wait SECS
            {
                Task=2001;
                bWaitSECS=false;
            }
            else
            {
                bRunAutoClean=false;
            }

            if(HasICUnderMachine()==true || HasAnyICInMachine()==true)          //JerryYang 20250305 : fix Initial時auto clean結束沒有做index check
            {
            bAutoCleanFinishOnlyUseRTC=true;                                    //JerryYang 20161216 (Steven) 結束auto clean後只跑RTC檢查Socket,不做index check
            }

            iAutoClean_IndexContactCount=0;
            lAutoClean_TimeCount=0;                                             //jou 20250102 : auto clean triger time count
            if(iCleanOut==1)                                                    //Steven 20140725
                iCheckFinish_ByAutoClean=2;                                     //pig 2011.09.01 AutoClean

            CleanSetSpeed(false);
            iCloseSiteState=CloseSiteState();

            if(bErrorAutoClean)                                                 //JerryYang 20160520 修正auto clean發生error後 直接修改clean kit參數的問題
            {
                ShowErrorMessage("WAR16313", K_RETRY, MMSystem);                //kevin 20200527 add Autoclean count Alarm
                RecordProcess("Error Auto Clean Finish!");                      //Steven 20130614 : Auto Clean加上紀錄
            }
            else
            {
                RecordProcess("Auto Clean Finish!");                            //Steven 20130614 : Auto Clean加上紀錄
            }

            if(bBackupOneCycle_ByAutoClean)                                     //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 完成的時又觸發 AutoClean 動作，AutoClean 完成才會執行 OneCycle Finish
            {
                bBackupOneCycle_ByAutoClean=false;
                NewRecordProcess("MES2115", "ONE CYCLE pressed", "Auto Clean Finish");
                fMain->BtnOneCycleClick(fMain);
            }
            ZeroMemory(iAutoCleanByBinCount, sizeof(iAutoCleanByBinCount));
            ZeroMemory(iAutoCleanBySiteCount, sizeof(iAutoCleanBySiteCount));
            bAutoCleaning=false;                                                //JerryYang 20151109 add for 記錄 AutoClean出現
            // AI(W906-FLOW-2) 20260928: RESTORED two lines down: the drop reason below is dead -- TfContactCT *fContactCT is a real global (cContactCT.cpp:99, method :1527), declared by forms/fContactCT.h (included at the top).  History: AI(W906-AutoCleanCluster) 20260722: golden `fContactCT->ACSmartClearData();`
            // (Sam 20230111 : Smart Auto Clean) -- fContactCT has NO link-visible
            // shared home in this tree (only TU-local #define seams inside
            // csystem.cpp/Automation/auto9045.cpp, verified by grep). Same
            // treatment as the fShowBinSelect drop above: a pure smart-clean UI
            // bookkeeping reset nothing else in this cluster reads back.
            fContactCT->ACSmartClearData();                                     //Sam 20230111 : Smart Auto Clean  //AI(W906-FLOW-2) 20260928: golden :5239 restored
            ReadWriteAutoCleanCount(false);                                     //Steven 20180524 : Fixed for clean count
            SearchCleanNum();
            bIndexCheckState=true;                                              //wei 20150903
            bErrorAutoClean=false;                                              //wei 20150904
            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.AutoCleanFinish);                        //50     Auto Clean Finish
            #ifdef DEBUG_AUTO_CLEAN
                Message.sprintf("DoAutoCleanKit, Auto Clean Finish");
                fMain->AddAutoCleanMessage(Message);
            #endif

            // AI(W906-AutoCleanCluster) 20260722: golden's CC_SPIL_TAICHUNG_LOGIC
            // FTP-upload block (Steven 20140917) -- SAME treatment as
            // InitialAutoCleanTask's identical block above (Wave-14): fFTPClient
            // was demoted away from a singleton by the KYECFTP translation wave
            // (UploadFileToServer2 is now a free function, no `fFTPClient->`
            // surface exists), and SaveJamCodeFile was never translated
            // (verified by grep -- zero hits). Re-introducing the singleton
            // pattern that wave deliberately removed is out of scope here;
            // gated. CUSTOMER_CODE==CC_SPIL_TAICHUNG_LOGIC is a narrow
            // single-customer condition, default unreachable elsewhere.
            //   if(CUSTOMER_CODE==CC_SPIL_TAICHUNG_LOGIC &&
            //      IniConfig.bFTPJamCodeUpload && IniConfig.bEnableFTP)
            //   {
            //       fNote->aJamCodeFilePath=fFTPClient->SaveJamCodeFile(IniConfig.SocketHandlerID, Now(), "MES1608", "Autoclean Finish");
            //       fFTPClient->UploadFileToServer2(IniConfig.FtpUplaodPath, fNote->aJamCodeFilePath);
            //   }

            fLtcSensor->ClearLtcSensor(1);                                      //Sam 20221101 : Latch 清乾淨要確認是否清清乾淨
            fLtcSensor->ClearLtcSensor(0);                                      //Sam 20221101 : Latch 清乾淨要確認是否清清乾淨
            fLtcSensor->SetLtcSensor(0);                                        //JerryYang 20230223 : 清除latch函式的shuttle 1,2  //Steven 20140805 : 將Latch清空，確保沒有問題
            fLtcSensor->SetLtcSensor(1);                                        //JerryYang 20230223 : 清除latch函式的shuttle 1,2
            DoInArm_SuckerMap();                                                //kevin 20161007 Auto Clean開放site 吸嘴使用異常修正
        case 2001:                                                              //KevinCheng 20250919 : Wite SECS
            if(IniConfig.bA81WaitSECS==true)
            {
                SECSGEM_WaitSECS.SetSecAndOn(8);
                Task=2002;
            }
            break;
        case 2002:
            if(bWaitSECS==true)
            {
                bWaitSECS=false;
                bRunAutoClean=false;                                            //KevinCheng 20250922 : 等收到後才能繼續執行程序
                return;
            }
            else
            {
                if(SECSGEM_WaitSECS.Off())
                {
                    bRunAutoClean=false;                                        //KevinCheng 20250922 : 等收到後才能繼續執行程序
                    ShowErrorMessage("WAR0358", 0, MMSystem, false, "");
                }
                return;
            }                                                                   //KevinCheng 20250919 : Wite SECS
            break;
        case 2100:                                                              //Clean Other Arm start
            if(CheckInArmSuckInitial()==false)
                break;

            if(MoveInArmZToPlateSafe(Task))                                     //in arm z軸移至安全位置
            {
                if(InArmSuck.HasIC()==false                &&                   //Steven 20130620 mark: Auto Clean做一半的糗事要繼續作
                   b2ShuttleMoveToLeft==true               &&                   //ChungHung 20131120 AutoClean use Hotplate1
                   (BTestSuck.HasDefineIC(HAS_CLEAN_IC)    ||
                    BTestSuck.HasDefineIC(CLEAN_FINISH_IC) ||
                    BLCarryKit.HasDefineIC(HAS_CLEAN_IC)   ||
                    BLCarryKit.HasDefineIC(CLEAN_FINISH_IC)))
                {
                    Task=2540;
                }
                else
                {
                    if(InArmSuck.HasDefineIC(CLEAN_FINISH_IC))
                    {
                        Task=3100;
                    }
                    else if(InArmSuck.HasDefineIC(HAS_CLEAN_IC))
                    {
                        InitPlaceToShuttleTask();
                        Task=2400;
                    }
                    else
                    {
                        Task=2200;
                    }
                }
            }
            break;
        case 2200:
            DoAutoCleanPickfromCleanKit(euShuttle2, true);                      //initial Task
            Task=2300;
            break;
        case 2300:
            bPickFromKitByAutoClean=true;                                       //Steven 20210603 : 新增吸取Clean Kit的Flag
            flag=DoAutoCleanPickfromCleanKit(euShuttle2, false);
            if(flag==1)
            {
                bPickFromKitByAutoClean=false;                                  //Steven 20210603 : 新增吸取Clean Kit的Flag
                if(InArmSuck.HasIC()==false)
                {
                    Task=2200;
                    if(BLCarryKit.FindNoIC()==false)                            //kevin 20180425 SHUTTLE 有NULL IC
                        Task=2530;
                }
                else
                {
                    InitPlaceToShuttleTask();
                    Task=2400;
                }
            }
            else if(flag==2)                                                    //Steven 20130620 : 避免Clean Count到的時候，貨到Arm導致Hang Up，回傳值從bool改為int
            {
                Task=10;
            }
            break;
        case 2400:
            bPlaceToShuttleByAutoClean=true;                                    //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
            if(DoPlaceToShuttle(euShuttle2))
            {
                bPlaceToShuttleByAutoClean=false;                               //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                if(USE_PICKER_COUNT==ep1Picker)
                {
                    if(IsBLCarrKitAllHasIC()==false)
                    {
                        Task=2200;
                    }
                    else
                    {
                        Task=2500;
                    }
                }
                else
                {
                    iShuttleRowKit=GetShuttleState(euShuttle2, bAutoPlace);
                    if(iShuttleRowKit!=0)
                    {
                        Task=2200;
                    }
                    else
                    {
                        Task=2500;
                    }
                }
            }
            break;
        case 2500:                                                              //Pick from shuttle ,place to Clean Kit
            if(MoveInArmZToPlateSafe(Task))
            {
                bShakeFlag=false;
                bVibration=false;                                               //Steven 20210616 : Auto Clean也要振動
                if(IniConfig.bF01ShakeShuttleWhenJam==false && IniConfig.bF23ShuttleVibration==false)
                    bShakeFlag=true;
                Task=2510;
            }
            break;
        case 2510:
            if(CosFunction.bAutoCleanShuttleDisable==true)                      //jou 2013-02-27 Auto Clean disable shuttle sensor detect
            {
                if(IniConfig.bAutoCleanShuttleDisable)                          //jou 2013-02-27 Auto Clean disable shuttle sensor detect
                {
                    Task=2515;
                    break;
                }
            }
            else
            {
                if(iAutoCleanShuttle==0)                                        //JerryYang 20170327 (Jou) 修正Auto Clean做shuttle 2 Shuttle開放感應disable的問題
                {
                    Task=2515;
                    break;
                }
            }

            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20250923 : Auto clean for InSht sensor Latch 判定
            {
                InitAutoChkInSHLatchTask();
                Task=2512;
                break;
            }

            ret=CheckShuttleSensor_Clean(euShuttle2, bShakeFlag);
            if(ret==1)
            {
                bShakeFlag=false;
                bVibration=false;                                               //JerryYang 20180711 (wei) Shuttle振動後還是置放好不會振動
            }
            else if(ret==3)
            {
                break;
            }
            else if(ret==2)
            {
                if(IniConfig.bF23ShuttleVibration &&                            //Steven 20210616 : Auto Clean也要振動
                   ((IniConfig.bF01ShakeShuttleWhenJam==true && bVibration==false) ||
                     IniConfig.bF01ShakeShuttleWhenJam==false))                 //JerryYang 20180711 (wei) Shuttle振動後還是置放好不會振動
                {
                    if(IniConfig.bF01ShakeShuttleWhenJam==true)
                    {
                        bShakeFlag=false;
                    }
                    else
                    {
                        bShakeFlag=true;
                    }
                    bVibration=true;
                    Task=2522;
                    DoVibrateShuttle(euShuttle2, true);                         //JerryYang 20190123 shuttle振動做動
                }
                else if(bShuttleKnock==false)                                   //Jou 2013-03-08 修改敲敲功能
                {
                    if(IniConfig.bF01ShakeShuttleWhenJam==false &&
                       IniConfig.bF23ShuttleVibration==false)                   //kevin 20190731 不使用敲敲
                    {
                        bShakeFlag=true;
                        break;
                    }
                    Task=2520;
                    DoShakeShuttle(euShuttle2, true);                           //Steven 20120801 : 修改搖搖功能 (true為初始化)
                }
                else
                {
                    bShakeFlag=true;
                    DoKnockShuttle(euShuttle2, true);                           //jou 2013-07-17 Knock Shuttle(true為初始化)
                    Task=2521;
                }
                break;
            }
            else
            {
                Task=2515;
            }
            break;
        case 2515:
            MOT[MInShuttle2].fCanMoveL=true;

            if(TestIF.iAutoClean_SelectArm==2 ||                                //jou 2016-05-11 修正Auto Clean POP function異常
               TestIF_File.bCleanIndexOtherArm)                                 //JerryYang 20160307 使用兩Arm Input arm要等shuttle1完成
            {
                bSupport2Arm=CleanPadCountCanSupport2Arm();                     //Steven 20221006 : 修正雙arm auto clean只有一半的clean pad
                if(bSupport2Arm)
                    Task=2518;
                else
                    Task=2540;
            }
            else
            {
                Task=2540;
            }

            if(Task==2540)                                                      //Steven 20220822 : 判斷Shuttle 1是否還有Clean Pad, 避免Hang up
            {
                if(InArmSuck.HasIC()==false &&
                   (FLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC) ||
                    FLCarryKit.HasDefineIC(HAS_CLEAN_IC)      ||                //Jimmychiu 20221028 add HAS_CLEAN_IC
                    FLCarryKit.HasDefineIC(CLEAN_FINISH_IC)   ||
                    FTestSuck.HasDefineIC(HAS_CLEAN_IC)       ||
                    FTestSuck.HasDefineIC(CLEAN_FINISH_IC)))
                {
                    Task=2518;
                }
            }
            break;
        case 2518:
            if(InArmSuck.HasIC()==false && b1ShuttleMoveToLeft==true &&
               (FLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC)   ||
                FLCarryKit.HasDefineIC(HAS_CLEAN_IC)        ||                  //Jimmychiu 20221028 add HAS_CLEAN_IC
                FLCarryKit.HasDefineIC(CLEAN_FINISH_IC)))
            {
                Task=540;
            }
            else if(InArmSuck.HasIC()==false &&
                    (FLCarryKit.HasDefineIC(HAS_NULL_CLEAN_IC) ||
                     FLCarryKit.HasDefineIC(HAS_CLEAN_IC)      ||               //Jimmychiu 20221028 add HAS_CLEAN_IC
                     FLCarryKit.HasDefineIC(CLEAN_FINISH_IC)   ||
                     FTestSuck.HasDefineIC(HAS_CLEAN_IC)       ||
                     FTestSuck.HasDefineIC(CLEAN_FINISH_IC)))
            {
            }
            else                                                                //Steven 20220922 : Shuttle 1都沒有, 只處理Shuttle 2
            {
                Task=2540;                                                      //Steven 20220926 : 2515 --> 2540
            }
            break;
        case 2520:
            if(DoShakeShuttle(euShuttle2))                                      //Steven 20120801 : 修改搖搖功能
            {
                Task=2530;
            }
            break;
        case 2521:                                                              //Steven 20210616 : Auto Clean也要振動
            if(DoKnockShuttle(euShuttle2))                                      //Jou 2013-03-08 修改敲敲功能
            {
                Task=2530;
            }
            break;
        case 2522:                                                              //Steven 20210616 : Auto Clean也要振動
            if(DoVibrateShuttle(euShuttle2))                                    //JerryYang 20190123 shuttle振動做動
            {
                Task=2530;
            }
            break;
        case 2530:
            if(InSHT2InLF())
            {
                MOT[MInShuttle2].fCanMoveL=false;
                bShuttleShake=false;
                IndexZCanMove[0]=true;
                IndexZCanMove[1]=true;
                InitialSet();                                                   //kevin 20150720
                Task=2510;
            }
            break;
        case 2540:
            // AI(W906-FLOW-2) 20260928: GATE (W7d-I1) retired -- golden AutoClean.cpp:5551 restored (premise dead, see DoPlaceToShuttle case 100)
            if(MoveInArmXYToShuttle_9045(euShuttle2, 0, ZAxisNotDown, false))
            {
                Task=2600;
            }
            break;
        case 2600:
            if(BLCarryKit.UseSiteNoIC() &&
               InSHT2InLF()              &&
               BTestSuck.UseSiteNoIC())
            {
                Task=3200;
            }
            else
            {                                                                   //shuttle 2上有Clean Pad & shuttle 2確定完成放
                if(InSHT2InLF() &&                                              //Steven 20190212 : 修正Hang up
                   bInedxCleanFinish[1])                                        //Steven 20200730 : 修正Hang up
                    iShuttleRowKit=GetShuttleState(euShuttle2, bAutoPick);
                else
                    iShuttleRowKit=0;

                if(iShuttleRowKit!=0 && InSHT2InLF() &&
                   (BLCarryKit.HasType(CLEAN_FINISH_IC) ||                      //JerryYang 20160123 修正做index arm完成後,shuttle只剩下HAS_NULL_IC會hang up //JerryYang 20151124 Fix hang up,Shuttle上有 CLEAN_FINISH_IC才能吸取CleanPad
                   (bInedxCleanFinish[1] && BLCarryKit.HasType(HAS_NULL_CLEAN_IC))))//kevin 20170518 (wei) 全部IC掉料Hang up
                {
                    MOT[MInShuttle2].fCanMoveL=false;
                    InitPickFromShuttleTask();
                    if(CUSTOMER_CODE==CC_KYEC_LEE || CosFunction.bHiSiliconFunction)
                    {
                        bShakeFlag=false;
                        if(IniConfig.bF01ShakeShuttleWhenJam==false && IniConfig.bF23ShuttleVibration==false)
                            bShakeFlag=true;
                        Task=2610;
                    }
                    else
                    {
                        Task=2800;
                    }
                }
                else if(iShuttleRowKit!=0 && bInedxCleanFinish[1] &&
                        BLCarryKit.HasType(HAS_NULL_CLEAN_IC))                  //Frank 20171213 (Steven) modify
                {
                    Task=3200;
                }
            }
            break;
        case 2610:
            if(CosFunction.bAutoCleanShuttleDisable==true)                      //jou 2013-02-27 Auto Clean disable shuttle sensor detect
            {
                if(IniConfig.bAutoCleanShuttleDisable)                          //jou 2013-02-27 Auto Clean disable shuttle sensor detect
                {
                    Task=2800;
                    break;
                }
            }
            else
            {
                if(iAutoCleanShuttle==0)                                        //JerryYang 20170327 (Jou) 修正Auto Clean做shuttle 2 Shuttle開放感應disable的問題
                {
                    Task=2800;
                    break;
                }
            }

            ret=CheckShuttleSensor_Clean(euShuttle2, bShakeFlag);
            if(ret==1)
            {
                bShakeFlag=false;
                if(IniConfig.bF01ShakeShuttleWhenJam==false && IniConfig.bF23ShuttleVibration==false)
                    bShakeFlag=true;
            }
            else if(ret==3)
            {
                break;
            }
            else if(ret==2)
            {
                Task=2620;
                bShakeFlag=true;
                DoShakeShuttle(euShuttle2, true);                               //Steven 20120801 : 修改搖搖功能 (true為初始化)
                break;
            }
            else
            {
                Task=2800;
            }
            break;
        case 2620:
            if(DoShakeShuttle(euShuttle2))                                      //Steven 20120801 : 修改搖搖功能
            {
                Task=2630;
            }
            break;
        case 2630:
            if(InSHT2InLF())
            {
                MOT[MInShuttle2].fCanMoveL=false;
                bShuttleShake=false;
                IndexZCanMove[0]=true;
                IndexZCanMove[1]=true;
                InitialSet();
                Task=2610;
            }
            break;
        case 2800:
            bPickFromShuttleByAutoClean=true;                                   //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
            if(DoPickFromShuttle(euShuttle2, iShuttleRowKit))
            {
                if(bUse8Picker)
                {
                    if(iCloseSiteModeFor2x8==e2x8_STMMode ||                    //JerryYang 20191122 STM 8 site Auto clean支援前後資料一起做
                       iCloseSiteModeFor2x8==e2x8_TW153Mode)
                    {
                        if(BLCarryKit.LeftSideNoIC(4)==true && BLCarryKit.RightSideNoIC(4)==false)
                        {
                            iShuttleRowKit=GetShuttleState(euShuttle2, bAutoPick);
                            break;
                        }
                    }
                }

                bPickFromShuttleByAutoClean=false;                              //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
                Task=3000;
            }
            break;
        case 3000:
            if(MoveInArmZToPlateSafe(Task))
            {
                Task=3050;
                if(CheckInArmSuckICFallDownToHasNullIC(false)==true)            //Steven 20110516 : 修改成整合式Alarm
                {
                    Task=3010;                                                  //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
                }
            }
            break;
        case 3010:
            CheckInArmSuckICFallDownToHasNullIC();                              //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
            Task=3000;
            break;
        case 3050:
            if(LastSet.iRealDummy==REALLY)
            {
                ptrInSHT=&BLCarryKit;

                if(iShuttleRowKit==1 || iShuttleRowKit==2)
                    ret=0;
                else
                    ret=4;

                for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                   //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Suck[i][j].Enable       &&
                           InArmSuck.Suck[i][j].SenUsing!="" &&
                           InArmSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                           InArmSuck.Item[i][j]!=NULL_IC)
                        {
                            if(InArmSuck.Suck[i][j].GetStatus()==false)
                            {
                                bIsSuckICFallDown[i][j]=true;
                                ErrPart+=InArmSuck.Suck[i][j].sName;
                                bHasErr=true;
                            }
                        }
                    }
                }
            }

            if(bHasErr)
            {
                ret=ShowErrorMessage("JAM0128", K_SKIP, MInArmX, false, ErrPart); //Steven 20091123 : Device Drop Error  //JerryYang 20160511 JAM0126->JAM0128,將IC與Clean pad分開的alarm code分開

                for(int i=iSuckRow; i<InArmSuck.iMaxRow; i++)                   //ChungHung 20141121 add for Use HotPlate AutoClean 1--->i
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        if(InArmSuck.Suck[i][j].Enable       &&
                           InArmSuck.Suck[i][j].SenUsing!="" &&
                           InArmSuck.Item[i][j]!=HAS_NULL_CLEAN_IC &&
                           InArmSuck.Item[i][j]!=NULL_IC     &&
                           InArmSuck.Suck[i][j].GetStatus()==false)
                        {
                            InArmSuck.SetItemData(i, j, HAS_NULL_CLEAN_IC);
                            bIsSuckICFallDown[i][j]=true;                       //ChungHung 20141121 add for Use HotPlate AutoClean
                        }
                    }
                }

                if(SoftStop)
                    fMain->Pause("DoAutoCleanKit 3050");

                Task=3100;                                                      //JerryYang 20160125 Device drop error only skip
            }
            else
            {
                Task=3100;
            }
            break;
        case 3100:
            if(InArmSuck.HasIC()==false)
            {
                Task=2600;
                break;
            }
            MOT[MInShuttle2].fCanMoveL=true;                                    //ChungHung 20150410 add because case 2600 lock here need unlock
            DoAutoCleanPlaceToCleanKit(true);
            if(TestIF_File.iAutoClean_Tray==eCKPos_Fix3)                        //JerryYang 20161219 (Steven) 避免Auto clean hang up
            {
                if(iCatchTrayControlManual>=2)                                  // catch tray busy
                    break;
            }
            Task=3200;
            break;
        case 3200:
            if(DoAutoCleanPlaceToCleanKit(false))
            {
                if(InArmSuck.HasIC()==false)
                {
                    if(BLCarryKit.UseSiteHasIC())                               //JerryYang 20160123 修正做index arm完成後,shuttle只剩下HAS_NULL_IC會hang up
                    {
                        Task=2600;
                    }
                    else
                    {
                        Task=2000;
                    }
                }
                else
                {
                    Task=3000;
                }
            }
            break;
    }

    #ifdef DEBUG_AUTO_CLEAN
    static int oldTask=-1;
    if(oldTask!=Task)
    {
        Message.sprintf("DoAutoCleanKit, %d, Go Task, %d", oldTask, Task);
        fMain->AddAutoCleanMessage(Message);
        oldT2ask=Task;
    }
    #endif
}
//------------------------------------------------------------------------------
//  AI(W906-FLOW-2) 20260928: TfHome::InitDoTestZHome -- golden uhome.cpp:4891-4906, line for line
//  (golden uhome.h:74 `void __fastcall InitDoTestZHome();`, declared in forms/fHome.h).
//  Placed here rather than in uhome.cpp (its golden home) only because this change is
//  scoped to AutoClean.cpp + forms/fHome.h; the body can be moved there verbatim.
//  Callers: the ten W906DIAC_FHOME_INITDOTESTZHOME() sites of DoIndexAutoClean /
//  DoIndexAutoClean_Arm1PickArm2Test (golden AutoClean.cpp:6483 ... :8682).
//  MovFlag / bScanFlag / GaliSofDelayCount are plain TMyMotor fields (Motor/mymotor.h:178,
//  :232, :233) -- no driver call, so this is safe even where MOT[].Motor is NULL.
//  TestZTask is written here and read only by State Record (golden main.cpp:9936).
//------------------------------------------------------------------------------
void TfHome::InitDoTestZHome()
{
    TestZTask=1;
    MOT[MTestY1].MovFlag=false;
    MOT[MTestY2].MovFlag=false;
    MOT[MTestZ1].MovFlag=false;
    MOT[MTestZ2].MovFlag=false;
    MOT[MTestY1].bScanFlag=false;
    MOT[MTestY2].bScanFlag=false;
    MOT[MTestZ1].bScanFlag=false;
    MOT[MTestZ2].bScanFlag=false;
    MOT[MTestY1].GaliSofDelayCount=0;
    MOT[MTestY2].GaliSofDelayCount=0;
    MOT[MTestZ1].GaliSofDelayCount=0;
    MOT[MTestZ2].GaliSofDelayCount=0;
}
//------------------------------------------------------------------------------
//  AI(W906-FLOW-5) 20260929: golden AutoClean.cpp:1282-1301 -- the tail of SearchCleanNum (called on :1695,
//  right after golden :1281 `fMain->pnlCleanCount->Caption=Str;`).  Split out only so that no line above
//  moves; the statements are golden's, line for line.  Two spellings differ, both measured:
//    - fMain->pnlCleanCount->Font is the facade's parallel TFont fMain->pnlCleanCountFont (forms/fMain.h:373,
//      new'd at forms/fMain.cpp:130) -- the same idiom FileRW/TestIF_File_Cleaning.cpp:301/:317 uses;
//    - clRed / clNavy are written as their VCL values ($000000FF / $00800000): clNavy is not declared
//      anywhere in this TU's include set (measured with -E), so both are spelled as values.
//  The static iLastAutoCleanAlarmCount is golden's function-local static; this helper has one caller
//  (SearchCleanNum), so it keeps golden's once-per-new-iMin semantics.
//  WAR16313 is golden 906's code ("Reuse WAR16313", Steven 20260427).  V912 renamed it WAR1922
//  (JerryYang 20260814), which is what the V912 copy FileRW_Cleaning_SearchCleanNum
//  (FileRW/TestIF_File_Cleaning.cpp:262) raises -- see the FLOW-5 report: two copies, two statics.
//------------------------------------------------------------------------------
static void W906_SearchCleanNum_Tail(int iMin)
{
    if(iMin>=TestIF_File.iAutoClean_AlarmCount)                                 // golden :1282
    {
        fMain->pnlCleanCountFont->Color=0x000000FF;                             // golden :1284 fMain->pnlCleanCount->Font->Color=clRed;
        //Steven 20260427 : ATK P260427-ATK-H9-01 trigger alarm when reaching AlarmCount
        //                  Reuse WAR16313 (Autoclean count Alarm); replace with dedicated
        //                  WAR16314 after MDB Updater adds the new code.
        static int iLastAutoCleanAlarmCount=-1;
        if(TestIF_File.iAutoClean_AlarmCount>0 && iMin!=iLastAutoCleanAlarmCount)
        {
            iLastAutoCleanAlarmCount=iMin;
            AnsiString sLog;
            sLog.sprintf("Auto Clean Count reached AlarmCount: %d / %d", iMin, TestIF_File.iAutoClean_AlarmCount);
            RecordProcess(sLog);
            ShowErrorMessage("WAR16313", K_RETRY, MMSystem, false);             // golden :1295
        }
    }
    else
    {
        fMain->pnlCleanCountFont->Color=0x00800000;                             // golden :1300 fMain->pnlCleanCount->Font->Color=clNavy;
    }
}
