// =============================================================================
//  asendic_Auto.cpp  --  Auto-tray (automatic sort-output) stack-feed SM
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/asendic_Auto.cpp
//                   (2561 decoded lines; `wc -l` on the cp950 decode in THIS
//                    pass -- the last line is a `//---` separator, no trailing
//                    blank).
//  Translation wave: W906-W7-L1 Wave 2 (Auto track)
//  Translator: AI(W906-W7-L1-Auto) 20260802
//
//  FAITHFUL TRANSLATION
//  --------------------
//  Names, 1-based numeric Task states, switch(Task) semantics, every case
//  fall-through and every Sen[]/Cylinder[]/SW[]/MOT[] spelling are preserved
//  VERBATIM, same idiom as asendic_Empty.cpp (W6.1 canary), asendic_Auto2.cpp,
//  asendic_Color.cpp and asendic_Loader.cpp.  ALL 9 golden top-level definitions
//  are translated -- golden asendic_Auto.cpp :58, :71, :2262, :2267, :2294,
//  :2300, :2438, :2483, :2488 (each re-derived from the decoded golden in this
//  pass).  NOTHING in this file is `#if 0`.
//
//  THE BODY REGION BELOW IS BYTE-IDENTICAL TO GOLDEN.  Everything from the
//  anchor line `TQPF_Timer ULDStackDelay        [MAX_AUTO_TRAY];` (golden :50)
//  to the final `//---` separator (golden :2561) is a verbatim copy of the
//  cp950-decoded golden, re-encoded as UTF-8 and nothing else -- 2512 lines,
//  zero diff lines.  That is only possible because every symbol with no ported
//  home is handled ABOVE the anchor, by a TU-local stand-in plus a `#define`,
//  never by editing a call site.
//
//  GOLDEN FALL-THROUGHS PRESERVED (every `case N:` whose preceding statement is
//  not break/return/continue, re-derived by scanning the decoded golden here):
//    (1) DoAutoReceiveBinTray  case 950 -> case 955  (golden :743-:751) -- LIVE.
//        case 950 arms htUnLoaderTimeRelief and then DELIBERATELY runs case
//        955's body in the SAME tick; case 955 immediately re-tests the same
//        `CosFunction.bUnloadCylinerUpRelief && LOAD_Z_USE_MOTOR[3+Pos]==false`
//        guard and `break`s while the 1000 ms relief timer is still running.
//    (2) DoAutoTrackDetectICFloating case 1 -> case 100 (golden :2273-:2276).
//    (3) DoAuto123TrayToRear   case 1 -> case 100 (golden :2311-:2317).
//  There are NO other fall-throughs in this file.  They are golden's and are
//  NOT annotated: inserting `[[fallthrough]]` would break the zero-diff fidelity
//  proof.  MEASURED, not assumed: this file compiles with ZERO warnings under
//  the library flags, because -Wimplicit-fallthrough does not exist in this
//  toolchain (MinGW.org g++ 6.3.0; the warning arrived in GCC 7).  A future
//  compiler bump WILL surface three of them -- that is the expected outcome, not
//  a regression to "fix".
//
//  INCLUDES (BCB6 original vs this tree)
//  -------------------------------------
//  BCB6 pulled: MachineDefine.h, asendic_Auto.h, csystem.h, asendic.h,
//  MyMotor.h, mycylin.h, main.h, mymessbox.h, note.h, mysensor.h, myswitch.h,
//  acatchtray.h, cprod.h, OCR.h, AGV.h, OCRInsp.h, cMyDB.h, uHGemHT9045.h,
//  cmydef.h, aoutarm.h, ProductionInfo.h, BarCode.h, SortingBinTray.h,
//  FixAICCD.h, uTrayEditForm.h, fVATMesFileSys.h, cTrayMapping.h, common.h,
//  SCK_ART.h, AMR.h, uLotInfo.h, cSortCT.h.  Replaced by:
//    Motor/mymotor.h      (MOT[])                          -- W4 HAL
//    mycylin.h            (Cylinder[])                     -- W6.0 HAL gap
//    mysensor.h / myswitch.h (Sen[] / SW[])                -- W4-IO HAL
//    mytray.h             (TMyTray -- Tray.HasIC/FullIC/HowManyBinICInTray)
//    csystem.h            (ReadWriteTrayID / CheckAllAutoTrayEjectFinsh)
//    asendic.h            (TrayMoveIn/Out + TrayCylinMoveIn/Out + TrayMoveStatus,
//                          and -- AI(W906-W7-L1-W3fixB) 20260802 -- AutoCylinderUp/
//                          Middle/Lower, which golden also declares here
//                          (asendic.h:19-21).  Wave 3 moved those three
//                          declarations out of acatchtray_shims.h to match golden.)
//    asendic_Color.h      (ForTERAPOWERCheckColorSensor -- golden reaches it via
//                          asendic.h's god-include, which this tree's asendic.h
//                          subset does not reproduce; golden call sites :363,
//                          :2029, :2074)
//    acatchtray.h         (InitialCoverTrayIDTask)
//    acatchtray_shims.h   (fTrayMapping, AMRUnloadBin, NewRecordProcess).
//                          AI(W906-W7-L1-W3fixB) 20260802: AutoCylinderUp/Middle/
//                          Lower REMOVED from this list -- Wave 3 deleted both the
//                          `{ return true; }` bodies and the declarations from the
//                          acatchtray_shims pair; they now come from asendic.h.
//    FormsFacade.h        (fMain / fLotInfo / fSortCT / fSCKART / fAGV /
//                          fFixAICCD / fProductionInfo)     -- non-VCL satellites
//    canary_support.h     (LastSet, __FUNC__, ShowErrorMessage, ShowMyMessage,
//                          ShowUnloaderTrayMessage, RecordProcess)
//    SECSGEM/SecsEventType.h + SecsEventReport.h  (SECS_EVENT / EventReport)
//    Automation/AMR.h     (the real TTeraPowerAMR `AMR` global)
//    common.h             (aslDupUnloadBundlID)
//    cprod.h/cpublic.h/cmydef.h  (Prod/TrayForm/IniConfig/CosFunction/TestIF_File/
//                          Ld_UldDelayTime/RunInfo/BinSelect/TrayID/enums)
//
//  SOFT_SIMULTE is NOT defined: the #ifndef SOFT_SIMULTE (real-machine) branches
//  are compiled, the #ifdef SOFT_SIMULTE branches are not -- i.e. the REAL logic
//  is translated and driven through the Sim HAL.  Both arms are kept verbatim.
//
//  ONE GOLDEN LINE IS *NOT* REPRODUCED VERBATIM, AND IT IS ABOVE THE ANCHOR
//  -----------------------------------------------------------------------
//  golden :44 `extern SaveUnloaderInfo(int iWhichTray);` is an implicit-int
//  declaration (BCB6 accepted K&R style).  ISO C++ forbids it, so the symbol is
//  handled by the stand-in block below instead.  It sits at golden :44, i.e. SIX
//  lines above the :50 fidelity anchor, so it does not perturb the zero-diff
//  proof.  Likewise golden :46-:47's `extern iAutoTrayData / bOldAutoHasTray`
//  become TU-local storage (stand-in #11) -- also above the anchor.  [AI(W906-FLOW-2) 20260928: both now bind to the real objects -- :44 is a void prototype of cinitial.cpp:5533 at :324, and the :46-:47 externs are restored verbatim at :348-:349]
//
//  TU-LOCAL STAND-INS + `#define` REDIRECTS -- READ THIS BEFORE EDITING
//  --------------------------------------------------------------------
//  Fourteen symbols this file needs have no linkable home in the ported tree.
//  Rather than gate their call sites (which would destroy the verbatim
//  switch(Task) bodies this translation exists to preserve), each is redirected
//  to a transparently-named TU-local stand-in with a `#define` -- exactly the
//  idiom asendic_Loader.cpp (W7-L1 Wave 1) and csystem.cpp already use.  The
//  redirects live BELOW all #includes, so no header declaration is perturbed;
//  they are TU-local, so no duplicate symbol can ever be created; and each names
//  the wave that retires it.
//
//  FOUR OF THE FOURTEEN ARE FULL FAITHFUL TRANSLATIONS, NOT NO-OPS.  That is a
//  deliberate departure from Wave 1's all-no-op set and the reason is measured,
//  not stylistic: `bARTUnloaderUseTwoCylin` alone gates ~10 branches of this
//  file INCLUDING BOTH runtime AutoCylinder argument swaps (golden :784/:788 and
//  :811/:819), so a constant stand-in would make the single highest-risk hazard
//  in this file structurally untestable.  Each of the four is stateless or has
//  this file as its ONLY golden caller (verified by grepping the cp950-decoded
//  golden tree in this pass), so a TU-local copy is behaviourally IDENTICAL to
//  golden rather than an approximation.  The full list, with each one's golden
//  home / why it has no ported home / its offline value / its behaviour cost, is
//  in the block right above the `#define`s.  Do NOT promote any of them into a
//  shared header: acatchtray_shims.h's Wave-0 banner explicitly forbids that for
//  the six golden asendic.cpp free functions, and the concurrent Wave-2 track
//  (asendic_Auto_RT.cpp) is landing in parallel.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  ZERO U+FFFD.
// =============================================================================
#include "asendic_Auto.h"
#include "csystem.h"
//---------------------------------------------------------------------------
#include "asendic.h"
#include "Motor/mymotor.h"
#include "mycylin.h"
#include "mysensor.h"
#include "myswitch.h"
#include "mytray.h"
#include "myTimer.h"
#include "acatchtray.h"                 // InitialCoverTrayIDTask
// AI(W906-W7-L1-W3fixB) 20260802: dropped "AutoCylinder*" from this comment --
// Wave 3 moved those three declarations to asendic.h (included above).
#include "acatchtray_shims.h"           // fTrayMapping / AMRUnloadBin / NewRecordProcess
#include "asendic_Color.h"              // ForTERAPOWERCheckColorSensor (golden :363/:2029/:2074)
#include "aHotPlateSubstrate.h"          // OutArmSuck (golden MyKitSuck.h:366) -- needed by
                                        // stand-in #4 CheckOutArmZ.  Included BEFORE the
                                        // `#define fBarCode` below, so this header's own
                                        // `TfBarCode *fBarCode` declaration is
                                        // processed unperturbed and the macro only ever
                                        // rewrites THIS file's 3 call sites.
#include "Automation/AGV_PortScan.h"    // sOutputBinCode (golden AGV.h:241)
#include "cprod.h"
#include "cmydef.h"
#include "cpublic.h"
#include "common.h"                     // aslDupUnloadBundlID
#include "FormsFacade.h"                // fMain / fLotInfo / fSortCT / fSCKART / fAGV /
                                        // fFixAICCD / fProductionInfo
#include "canary_support.h"             // LastSet, __FUNC__, ShowErrorMessage, etc.
#include "SECSGEM/SecsEventType.h"      // SECS_EVENT      (was uHGemHT9045.h)
#include "SECSGEM/SecsEventReport.h"    // EventReport()   (was uHGemHT9045.h)
#include "Automation/AMR.h"             // AMR             (was AMR.h)

// AI(W906-W7-L1-Auto) 20260802: golden aoutarm.h:18 `int iWhichAuto` (the
// place-target Auto index).  The ported tree DEFINES it exactly once, at
// aoutarm9045.cpp:179 (`int iWhichAuto=0;`), which is compiled into ht9045_sm --
// the same library this file joins -- but NO ported header declares it.  This is
// therefore a DECLARATION of that one existing object, not a new definition and
// not a stand-in, so golden :1922/:1924/:1925 bind to the real global.  A second
// definition here would be a duplicate symbol.  Reported as a surface gap: the
// declaration belongs in a shared header (aoutarm9045.h), not in each consumer.
extern int iWhichAuto;                                                          // golden aoutarm.h:18

// =============================================================================
//  TU-LOCAL STAND-INS FOR SYMBOLS WITH NO PORTED HOME  (see banner)
//  Each entry: golden home -> why there is nothing to link -> offline value ->
//  the wave that retires the stand-in + its `#define`.
//
//  --- GROUP A: FOUR FULL FAITHFUL TRANSLATIONS (behaviourally identical) -----
//
//   1. bARTUnloaderUseTwoCylin(int Part)     golden asendic.h:24 / asendic.cpp:139
//      A golden asendic.cpp free function.  acatchtray_shims.h's Wave-0 SHIM
//      RETIREMENT DEBT REGISTER states explicitly that this one lands as a REAL
//      translated body in Wave 3 and that "no shim for them exists here to
//      retire, and none should be added" -- so it is stood in TU-locally here.
//      IT IS STATELESS: a 4-term predicate over USE_AUTO_RETEST / eartInstall /
//      UNLOADER_ART[] / bNoAutoZSelect / USE_LdUldCassetteMode, ALL FIVE of which
//      already exist in the ported tree (cmydef.h / MachineType.h).  So the body
//      below is golden's, line for line, and the result is IDENTICAL to golden
//      for every input -- not an offline approximation.
//      WHY IT IS TRANSLATED RATHER THAN STUBBED: it gates ~10 branches of this
//      file, and in particular it is the runtime test that SWAPS parameters 2
//      and 3 of AutoCylinderLower at golden :784/:788 and :811/:819.  A constant
//      stand-in would make both swap arms unreachable and the highest-risk
//      hazard in this file unfalsifiable.
//      RETIRED BY: Wave 3 (asendic.cpp L1a) -- delete the #define and the body.
//   2. DoAutoTrayEdgeCylinderLoop(int iAuto) golden asendic.h:25 / asendic.cpp:1163
//      Same register entry, same reasoning.  THIS FILE IS ITS ONLY GOLDEN CALLER
//      (grepped every .cpp in the cp950-decoded golden tree this pass: exactly
//      two call sites, golden :485 and :495, both here; asendic.cpp contains only
//      the definition).  Its two statics (EdgePushLoop[], bFirst) and its
//      LoopEdgeDelay[] timer array are therefore not shared with anyone, so a
//      TU-local copy is behaviourally identical.  LoopEdgeDelay is golden
//      asendic.cpp file-scope and has no ported home, so it is TU-local too.
//      COST: none.  (Offline IniConfig.bP13EnableAutoTrayEdgePushCylinderLoop is
//      false, so golden's own first line returns false -- but the remaining
//      body is translated so that turning P13 on behaves like golden.)
//      RETIRED BY: Wave 3 (asendic.cpp L1a).
//   3. PushUnLoaderTrayInAverageTime(DWORD st, int pos)
//                                            golden asendic.h:26 / asendic.cpp:1236
//      Same register entry.  ALSO SOLE-CALLER: golden's only call is this file's
//      :1067, so its static 5-deep ring buffer is unshared and the TU-local copy
//      is behaviourally identical.  It writes LastSet.iUnLoaderTraySimulateTime[],
//      which Wave 0 landed for exactly this purpose (canary_support.h) -- and the
//      [3] bound is golden's, including golden's own out-of-bounds write for
//      pos>=3, mirrored verbatim rather than widened.
//      RETIRED BY: Wave 3 (asendic.cpp L1a).
//   4. CheckOutArmZ(bool bMessage)           golden aoutarm.h:53 / aoutarm.cpp:95
//      aoutarm.cpp belongs to W7-A2 and is not translated; the ported tree has
//      only the unrelated CheckOutArmZNeedHome() (Motor/mymotor.h:450).
//      STATELESS: it only reads OutArmSuck / InOutArmPickerUseMotor / MOT[] /
//      Prod.ZOutArmSafe, all present, so the translated body is identical to
//      golden for every caller -- which matters because golden has 7 other
//      callers and a constant would have been wrong for all of them.
//      OFFLINE it returns false whenever OutArmSuck.iPickRow/iPickCol are 0.
//      One cosmetic delta, stated so it is not mistaken for a translation slip:
//      golden's unused `AnsiString S1, S2;` locals are dropped (they would draw
//      -Wunused-variable and have no behaviour).
//      RETIRED BY: the wave that lands aoutarm.cpp (W7-A2).  [AI(W906-FLOW-2) 20260928: RETIRED -- aoutarm.cpp:653, aoutarm.h included at :372]
//
//  --- GROUP B: TEN NO-OP / STORAGE STAND-INS ---------------------------------
//
//   5. SaveUnloaderInfo(int iWhichTray)      golden cinitial.cpp:7754
//      A pure FILE sink (MyForceDirectories + Tray.SaveUnloaderInfo(path)).
//      cinitial.cpp is not translated.  Note the ported mytray.h:93 DOES have a
//      `TMyTray::SaveUnloaderInfo(AnsiString)` MEMBER -- a different function
//      with a different signature; it is not this symbol.  Golden's own
//      declaration (golden :44) is implicit-int K&R and would not compile under
//      ISO C++ regardless.  OFFLINE VALUE: no-op.  COST: golden :1103 stops
//      writing the per-unloader ATK info file.  No SM-visible effect.
//      RETIRED BY: the wave that lands cinitial.cpp.  [AI(W906-FLOW-2) 20260928: RETIRED -- cinitial.cpp:5533 (W906_UNLOADERINFO_ROOT seam)]
//   6. SaveProductionRecord(TMyTray*, AnsiString) golden SortingBinTray.h:46
//   7. SaveTrayRecord(int iAuto)                  golden SortingBinTray.h:47
//      Both are production-log FILE sinks in SortingBinTray.cpp, not translated.
//      OFFLINE VALUE: no-op both.  COST: golden :607 stops writing the per-tray
//      production log and golden :601 stops writing the CC_TSI every-10-trays
//      summary.  Pure diagnostics; neither has an SM-visible effect (the CC_TSI
//      counter LastSet.RecodeTrayCount[] is still incremented and still reset by
//      the translated code around the call).
//      RETIRED BY: the wave that lands SortingBinTray.  [AI(W906-FLOW-2) 20260928: RETIRED -- SortingBinTray.cpp:2699/:2788]
//   8. EditTray(int MotorIndexIndex, int iHasMap=0)  golden uTrayEditForm.h:60
//      A VCL modal tray-editor form.  Same situation, and the same answer, as
//      asendic_Loader.cpp's stand-in #9.  OFFLINE VALUE: no-op.  COST: golden
//      :2471's P54 "error bin on tray" hand-edit step does nothing; the operator
//      message right above it (ShowMyMessage) is still issued.
//      RETIRED BY: the wave that lands uTrayEditForm.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, no EditTray definition anywhere in the tree]
//   9. fBarCode->Write_Device_Info_By_Tray(int,int)  golden BarCode.h:939
//      The ported tree DOES have an `fBarCode` (aHotPlateSubstrate.h:816,
//      `TfBarCode *fBarCode`), but that shim carries only the Bottom-2DID
//      methods -- Write_Device_Info_By_Tray is not one of them, and
//      aHotPlateSubstrate.h belongs to another front, so the method is NOT added
//      there.  A TU-local object supplies it instead and this file deliberately
//      does NOT include aHotPlateSubstrate.h, so the two names never meet in one
//      translation unit.  OFFLINE VALUE: no-op.  COST: golden :1080/:1086/:1096
//      stop appending to the per-tray Barcode device-info file.  The companion
//      counter LastSet.iDevice_Info_By_Tray[] is still incremented by the
//      translated code, so the SM state is unchanged.
//      RETIRED BY: the wave that lands BarCode.cpp's Write_Device_Info_By_Tray.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, BarCode/BarCode.h TfBarCode still has no Write_Device_Info_By_Tray (csystem.cpp G1 / g3-G15 / g3-G16 gate the same method); the stand-in above is still correct]
//  10. fMesSystem->bFirstMaterialsQA        golden Mes/fVATMesFileSys.h:166/:227
//      The whole Mes/ directory (TfMesSystem) is absent from the ported tree.
//      A TU-local object supplies the one field this file touches.
//      OFFLINE VALUE: false -- which is fVATMesFileSys.cpp:157's own initial
//      value, so the read at golden :2225 sees exactly what a freshly
//      constructed golden form would give.  COST: the VTEST first-tray QA alarm
//      (WAR16124, golden :2228) never fires.  Already unreachable anyway: the
//      enclosing guard needs IniConfig.bVTESTFunction, which is offline false.
//      RETIRED BY: the wave that lands Mes/fVATMesFileSys.  [AI(W906-FLOW-2) 20260928: RETIRED -- forms/fMesSystem.cpp:1254]
//  11. int iAutoTrayData[MAX_AUTO_TRAY][MAX_X_ITEM][MAX_Y_ITEM]  golden csystem.cpp:130
//      bool bOldAutoHasTray[MAX_AUTO_TRAY]                       golden csystem.cpp:131
//      A DATA stand-in, not a function.  golden :46-:47 extern them; the ported
//      csystem.cpp has NEITHER (grepped: zero hits tree-wide, so this is a
//      genuinely absent symbol, not a `#if 0`-gated one).  csystem.cpp is an
//      integrator-only file that other fronts are perturbing, so the definitions
//      are NOT added there.  TU-local storage with golden's exact initialisers.
//      COST, stated because it is a real behaviour delta: golden's writer is
//      csystem.cpp:7098-7103 (the continue-start snapshot of each Auto tray's
//      data).  With TU-local storage this file's case-1 continue-start restore
//      (golden :212-:220) reads a snapshot nobody ever writes -- i.e.
//      bOldAutoHasTray is permanently false, so the restore arm is DEAD and the
//      `else` (unlock the fixers + the JAM1110 alarm) always runs instead.  That
//      arm is dead in the ported tree TODAY regardless, because the writer does
//      not exist; the stand-in makes it compile without pretending otherwise.
//      RETIRED BY: the wave that lands csystem.cpp's continue-start snapshot.  [AI(W906-FLOW-2) 20260928: RETIRED -- csystem.cpp:9767 + :11486-:11491]
//  12. GetBundleInfo(int iAuto)             golden cpublic.h:337 / cpublic.cpp:2253
//      DECLARED in the ported cpublic.h:337, but its BODY sits inside
//      cpublic.cpp's `#if 0 // TODO(W7: GetBundleInfo needs cJSON +
//      TestSocket/LotSummary/Prod/fSCKART/fNote + asBundleTrayID/bUnloading)`
//      block starting cpublic.cpp:2249 -- so the symbol does not exist at LINK
//      time even though the three call sites (golden :1748, :1790, :1831)
//      compile fine.  cpublic.cpp is not this agent's file and must not be
//      un-gated.  OFFLINE VALUE: "".  COST: sBundleEndInfo is left empty on the
//      three BundleEnd_IDREAD_AutoN paths, so the SECS bundle-end payload
//      carries no JSON.  No SM-visible effect (nothing in this file reads it
//      back).  RETIRED BY: the wave that un-gates cpublic.cpp's W7 block.  [AI(W906-FLOW-2) 20260928: KEEP -- re-measured, cpublic.cpp:2336 #if 0 (GA1-B3) still closed: TLotSummary unported, fNote->edBundleID missing]
//  13. CheckAllAutoTrayEjectFinsh()         golden csystem.h:270 / csystem.cpp:24617
//      DECLARED in the ported csystem.h:321 but defined NOWHERE (grepped the
//      whole tree: one hit, the declaration).  OFFLINE VALUE: no-op.  COST:
//      golden :2005's half-clean "all Auto trays ejected" roll-up never runs.
//      Already unreachable anyway -- its guard is `if(bEject)`, and the ported
//      `bEject` (acatchtray_shims.cpp:170) is false offline.
//      RETIRED BY: the wave that lands csystem.cpp's eject roll-up.  [AI(W906-FLOW-2) 20260928: RETIRED -- csystem.cpp:25241]
//  14. bool bNeedEject[MAX_AUTO_TRAY]       golden cmydef.cpp:6012 (as gated)
//      A DATA stand-in.  The ported cmydef.h:5938 DECLARES it but the ported
//      cmydef.cpp definition (:6012) sits inside that file's `#if 0` block
//      :5816-6025 -- the SAME block that swallowed `bWaitingAMR`, which
//      asendic_Loader.cpp had to own.  This file is its only consumer in the
//      PORTED tree, but NOT in golden (cShowBinSelect.cpp and csystem.cpp both
//      touch it), so it is stood in TU-locally rather than DEFINED here: owning
//      it would become a duplicate symbol the moment either of those lands, and
//      the concurrent Wave-2 track is writing in parallel.  Its sibling `bEject`
//      from the same golden line pair IS already owned, by acatchtray_shims.cpp:170
//      -- which is where this one belongs too.  Reported as a surface gap.
//      OFFLINE VALUE: all-false (golden's own default -- golden's definition has
//      no initialiser, i.e. zero-init, so this matches exactly).
//      COST: none reachable -- the single write (golden :2004) is inside the
//      same `if(bEject)` guard as #13.
//      RETIRED BY: the wave that un-gates cmydef.cpp :5816-6025.  [AI(W906-FLOW-2) 20260928: RETIRED -- cmydef.cpp:6285]
// =============================================================================
void SaveUnloaderInfo(int iWhichTray);  //AI(W906-FLOW-2) 20260928: golden :44 `extern SaveUnloaderInfo(int iWhichTray);` (implicit int) written as the void prototype of the real body cinitial.cpp:5533 (golden cinitial.cpp:7754); stand-in #5 RETIRED. Was: static void W7L1A_SaveUnloaderInfo(int /*iWhichTray*/)                   {}
#include "SortingBinTray/SortingBinTray.h"  //AI(W906-FLOW-2) 20260928: golden :29; real SaveProductionRecord SortingBinTray.cpp:2699 / SaveTrayRecord :2788; stand-ins #6/#7 RETIRED (:325-:327). Was: static void W7L1A_SaveProductionRecord(TMyTray * /*TrayData*/,
//                                       AnsiString /*Name*/)              {}
//AI(W906-FLOW-2) 20260928: RETIRED stand-in #7. Was: static void W7L1A_SaveTrayRecord(int /*iAuto*/)                          {}
static void W7L1A_EditTray(int /*MotorIndexIndex*/, int /*iHasMap*/ = 0) {}

// --- #9 fBarCode (golden BarCode.h TfBarCode) -------------------------------
struct W7L1A_TfBarCode_AutoShim
{
    void Write_Device_Info_By_Tray(int /*iAuto*/, int /*iTray_Number*/) {}      // golden BarCode.h:939
};
static W7L1A_TfBarCode_AutoShim  W7L1A_fBarCode_obj;
static W7L1A_TfBarCode_AutoShim *W7L1A_fBarCode = &W7L1A_fBarCode_obj;

// --- #10 fMesSystem (golden Mes/fVATMesFileSys.h TfMesSystem) ---------------
#include "forms/fMesSystem.h"  //AI(W906-FLOW-2) 20260928: golden :32 (fVATMesFileSys.h); real `TfMesSystem *fMesSystem` forms/fMesSystem.cpp:1254 (bFirstMaterialsQA set true by forms/fLotInfo.cpp:5586); stand-in #10 RETIRED (:339-:345). Was: struct W7L1A_TfMesSystem_AutoShim
//{
//    bool bFirstMaterialsQA;                                                     // golden fVATMesFileSys.h:166
//    W7L1A_TfMesSystem_AutoShim() : bFirstMaterialsQA(false) {}                   // golden fVATMesFileSys.cpp:157
//};
//static W7L1A_TfMesSystem_AutoShim  W7L1A_fMesSystem_obj;
//static W7L1A_TfMesSystem_AutoShim *W7L1A_fMesSystem = &W7L1A_fMesSystem_obj;

// --- #11 continue-start Auto tray snapshot (golden csystem.cpp:130-131) -----
extern int  iAutoTrayData       [MAX_AUTO_TRAY][MAX_X_ITEM][MAX_Y_ITEM];  //AI(W906-FLOW-2) 20260928: golden :46 verbatim; real csystem.cpp:9767, written by DoReceiveAutoTray csystem.cpp:11486. Was: static int  W7L1A_iAutoTrayData[MAX_AUTO_TRAY][MAX_X_ITEM][MAX_Y_ITEM];
extern bool bOldAutoHasTray     [MAX_AUTO_TRAY];  //AI(W906-FLOW-2) 20260928: golden :47 verbatim; real acatchtray_shims.cpp:211, set true csystem.cpp:11487. Was: static bool W7L1A_bOldAutoHasTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};

// --- #12 / #13 / #14 --------------------------------------------------------
static AnsiString W7L1A_GetBundleInfo(int /*iAuto*/)                    { return AnsiString(""); }
//AI(W906-FLOW-2) 20260928: RETIRED stand-in #13 -- real CheckAllAutoTrayEjectFinsh csystem.cpp:25241 (decl csystem.h:321). Was: static void       W7L1A_CheckAllAutoTrayEjectFinsh()                    {}
//AI(W906-FLOW-2) 20260928: RETIRED stand-in #14 -- real bNeedEject cmydef.cpp:6285 (extern cmydef.h:5944). Was: static bool       W7L1A_bNeedEject[MAX_AUTO_TRAY]={false, false, false, false, false, false};

// --- #1 / #2 / #3 -- STAND-INS RETIRED (W7-L1 Wave 3) -----------------------
// AI(W906-W7-L1-Wave3) 20260802: the TU-local W7L1A_bARTUnloaderUseTwoCylin,
// W7L1A_DoAutoTrayEdgeCylinderLoop (+ its W7L1A_LoopEdgeDelay[] timer array) and
// W7L1A_PushUnLoaderTrayInAverageTime bodies, and the three `#define`s that
// redirected this file's golden call sites onto them, are DELETED.  Wave 3
// landed golden asendic.cpp in full, so all three now have REAL definitions
// (golden asendic.cpp:139-148, :1162-1201, :1236-1266) reachable through
// `#include "asendic.h"` above.  The stand-ins were faithful line-for-line
// copies, so retiring them changes nothing about WHAT runs -- except that
// LoopEdgeDelay[] and PushUnLoaderTrayInAverageTime's static 5-deep ring buffer
// are no longer private to this TU; they are now the single shared instances
// golden has, which is what golden means.
// The retirement was placed here, ABOVE the golden-body anchor, precisely so the
// zero-diff fidelity of the body below is untouched.

// --- #4 CheckOutArmZ -- VERBATIM golden aoutarm.cpp:95-116 ------------------
#include "aoutarm.h"  //AI(W906-FLOW-2) 20260928: golden :25; real CheckOutArmZ aoutarm.cpp:653 (decl aoutarm.h:116) has the same body as stand-in #4 (:372-:391), RETIRED. Was: static bool W7L1A_CheckOutArmZ(bool bMessage)
//{
//    int iMotZ;
//    for(int i=0; i<OutArmSuck.iPickRow; i++)
//    {
//        for(int j=0; j<OutArmSuck.iPickCol; j++)
//        {
//            iMotZ=(InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
//            if(MOT[iMotZ].ReadPos()!=Prod.ZOutArmSafe[i][j])
//            {
//                if(bMessage)
//                {
//                    ShowErrorMessage("WAR0257", 0, MOutArmX, false, MOT[iMotZ].Alias);      //出料手臂的吸嘴 %s 沒有在安全位置
//                }
//                return true;
//            }
//        }
//    }
//    return false;
//}

//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define SaveUnloaderInfo                W7L1A_SaveUnloaderInfo
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define SaveProductionRecord            W7L1A_SaveProductionRecord
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define SaveTrayRecord                  W7L1A_SaveTrayRecord
#define EditTray                        W7L1A_EditTray
#define fBarCode                        W7L1A_fBarCode
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define fMesSystem                      W7L1A_fMesSystem
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define iAutoTrayData                   W7L1A_iAutoTrayData
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define bOldAutoHasTray                 W7L1A_bOldAutoHasTray
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define CheckOutArmZ                    W7L1A_CheckOutArmZ
#define GetBundleInfo                   W7L1A_GetBundleInfo
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define CheckAllAutoTrayEjectFinsh      W7L1A_CheckAllAutoTrayEjectFinsh
//AI(W906-FLOW-2) 20260928: RETIRED. Was: #define bNeedEject                      W7L1A_bNeedEject

// =============================================================================
//  BODY -- VERBATIM golden asendic_Auto.cpp:50-2561 (2512 lines, zero diff).
//  DO NOT reflow, retab, re-indent or "tidy" anything below this line: the
//  fidelity proof for this landing is a zero-line diff against the cp950-decoded
//  golden, anchored on the very next line.
// =============================================================================
TQPF_Timer ULDStackDelay        [MAX_AUTO_TRAY];

int BinTrayTask                 [MAX_AUTO_TRAY]={1, 1, 1, 1, 1, 1};
bool bAutoIsFullNeedStop        [MAX_AUTO_TRAY]={false, false, false, false, false, false};                             //JerryYang 20160810 Unloader滿tray是否需要馬上停機
TQPF_Timer DetectAutoTime       [MAX_AUTO_TRAY];
TQPF_Timer DetectAutoTimePause  [MAX_AUTO_TRAY];
TQPF_Timer ColorSensorDelayTime [MAX_AUTO_TRAY];                                //Sam 20211028 : 增加保護防止 Hange up

void Initial_Auto_BinTray_Task(int Pos)
{
    BinTrayTask[iAutoIndex[Pos]]=1;
}
//------------------------------------------------------------------------------
TQPF_Timer htUnLoaderTimeDelay      [MAX_AUTO_TRAY];
TQPF_Timer htSwACAutoOffDelay       [MAX_AUTO_TRAY];
TQPF_Timer AutoReceiveBinTrayDelay  [MAX_AUTO_TRAY];
TQPF_Timer RecevieLoopDelay         [MAX_AUTO_TRAY];                            // 2010.08.23 , Joye
TQPF_Timer AutoReceiveBinTrayTimer  [MAX_AUTO_TRAY];
TQPF_Timer htUnLoaderTimeRelief     [MAX_AUTO_TRAY];                            //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
extern bool bAutoEdgePush           [MAX_AUTO_TRAY];
TQPF_Timer DoTrayGoOutNeedDelay[MAX_AUTO_TRAY];
void DoAutoReceiveBinTray(int Pos)
{
    if(Pos<0 || iAutoIndex[Pos]>iAutoRight)
        return;

    Pos=iAutoIndex[Pos];

    int &Task=BinTrayTask[Pos], ret;
    int x, y;
#ifndef SOFT_SIMULTE
    static int  iSenRetry           [MAX_AUTO_TRAY]={0, 0, 0, 0, 0, 0};         //Sam 20220929 : 增加 AutoSideFixer Check Alarm Auto Retry
#endif
    static int  iRecevieLoopCount   [MAX_AUTO_TRAY]={0, 0, 0, 0, 0, 0};         // 2010.08.23 , Joye
    static int  iRetry              [MAX_AUTO_TRAY]={0, 0, 0, 0, 0, 0};         //Steven 20150924 : [SCC]改成陣列
    static bool bRecevieLoop        [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         // 2010.08.23 , Joye
    static bool bAutoDuplicateErr   [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //JerryYang 20200615 ART分離汽缸sensor
    static bool bDupError           [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //Steven 20120208 : Auto Tray沒放好,重複Alarm
    static bool bAlreadyIn          [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //JerryYang 20180628 (wei) : add二段氣缸先on off一次避免爆衝
    static bool bNeedClearData      [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //Ifor 20200620 add: 馬達停止後才清除 Tray Data //Ifor 20200720 Fix:改陣列處理
    static bool bReceiveAutoTray    [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //Ifor 20200620 add: 馬達停止後才清除 Tray Data //Ifor 20200720 Fix:改陣列處理
    static bool bflag               [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //kevin 20180726 (wei) Auto 123 氣缸常態在上
    static bool bHasICFloating      [MAX_AUTO_TRAY]={false, false, false, false, false, false};                         //Sam 20250415 : Unloader 偵測到置偏 IC 退出後再報警
    static AnsiString asColorSensorAlarm;                                       //Sam 20180525 (wei) : 晶兆成 Auto1~3 Color sensor detect By FromEmptyColor
    static bool bAMRFullTray[MAX_AUTO_TRAY]    = {false, false, false, false, false, false};   //Eastsun 20260512 F011 整合 (Phase4-F5T3 KYEC AMR file-local FullTray)

    AnsiString s="", str1, str2, str;                                           //JerryYang 20160810
    bool bFlag=false, bColorFlag=false;                                         //20140903 wei colcr Tray
    int ibuffer=0, iPosY=0, iTest;
    int iPreAlarm[MAX_AUTO_TRAY]={2, 3, 4, 10, 11, 12};                         //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color 10:Auto4 11:Auto5 12:Auto6 13:Fix4 14:Fix5 15:Fix6

    if(USE_ROTATE_KIT==1)                                                       // 2012.06.26 , Joye , Rotate KIT
    {
        if(TrayForm.iRotateKIT_OutputType!=0 && Pos==iRotate_Out_Tray6)
        {
            Cylinder[C_AutoSide_Fixer[Pos]].On();
            Cylinder[C_AutoEdgePush[Pos]].On();
            Cylinder[C_AutoUpPress[Pos]].On();                                  //JerryYang 20190423 新增unloader壓tray
            return;
        }
    }

    if(bAutoTrackCanGoRear==true)                                               //Isaac 20180109 (Steven) : auto123可前進後退
    {
        if(SW[iSwAutoOut[Pos]].Status()==true &&
           SW[iSwAutoIn[Pos]].Status()==false)
        {
            if(DoAutoTrackDetectICFloating(Pos))
            {
                Task=920;
                bAtuoTrayICDetectErr=true;
            }
        }
    }
    else
    {
        if(Sen[SnAutoTrackDetect[Pos]].IsOn() &&                                // detect IC under Tray placed was smooth
           IniConfig.bP50DisabledAutoTrackSensorDetect==false)                  //Sam 20230221 : 矽格中興國桂要求要能關閉
        {
            if(IniConfig.bP59UnloaderICFloattingAlarmAfterExit)                 //Sam 20250415 : Unloader 偵測到置偏 IC 退出後再報警
            {
                bHasICFloating[Pos]=true;
            }
            else
            {
                TrayMoveOut(false, Pos+3);
                ret=ShowErrorMessage(sJAM1110[Pos], K_SKIP, iMMAuto[Pos]);      //JerryYang 20171101 Retry改成Skip避免誤會
                return;
            }
        }
    }

    switch(Task)
    {
        case 1:
            if(LastSet.iRealDummy!=DUMMY &&
               MOT[iMMAuto_Car[Pos]].fHasTray &&
               Sen[SnAutoTrayCar[Pos]].IsOff())                                                                         //Buffer紀錄有Tray Buffer Sensor偵測沒有Tray
            {
                ret=ShowErrorMessage(sJAM1101[Pos], K_SKIP|K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_1");
                if(ret==K_SKIP)
                {
                    CheckHasErrorBinOnTray(iMMAuto_Car[Pos]);                                                           //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                    fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto_Car[Pos]);                                                 //Sam 20240325 : 新增 DamageTrayMapping 功能
                    MOT[iMMAuto_Car[Pos]].ClearTray(__FUNC__);
                }
                break;
            }

            if(LastSet.iRealDummy!=DUMMY &&                                                                             //Sam 20200707 : 避免 Auto Tray 要分離的時候被 Home 導致資料還在 MMAuto1_Car，可能會造成 Clean Out Hang up
               MOT[iMMAuto_Car[Pos]].fHasTray==false &&
               Sen[SnAutoTrayCar[Pos]].IsOn())
            {
                ret=ShowErrorMessage(sMES1120[Pos], K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_2");
                break;
            }

            if(LastSet.iRealDummy!=DUMMY &&                                                                             //Auto紀錄有Tray Auto偵測沒有Tray
               MOT[iMMAuto[Pos]].fHasTray &&
               Sen[SnAutoTrayDetect[Pos]].IsOff())
            {
                if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                    break;
                iRetry[Pos]++;
                if(iRetry[Pos]<30)
                    break;
                if(IniConfig.bSPILFunction==true)                                                                       //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                    ret=ShowErrorMessage(sJAM1108[Pos], K_SKIP|K_RETRY, iMMAuto[Pos]);                                  //wei 20160511 矽品Auto紀錄有Tray Auto偵測沒有Tray 只能Skip
                else
                    ret=ShowErrorMessage(sJAM1108[Pos], K_SKIP|K_RETRY, iMMAuto[Pos]);

                if(ret==K_SKIP)
                {
                    CheckHasErrorBinOnTray(iMMAuto[Pos]);                                                               //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                    fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto[Pos]);                                                     //Sam 20240325 : 新增 DamageTrayMapping 功能
                    MOT[iMMAuto[Pos]].ClearTray(__FUNC__);
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();
                    Cylinder[C_AutoEdgePush[Pos]].Off();
                    Cylinder[C_AutoUpPress[Pos]].Off();                                                                 //JerryYang 20190423 新增unloader壓tray
                }

                if(SoftStop)
                {
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();
                    Cylinder[C_AutoEdgePush[Pos]].Off();
                    Cylinder[C_AutoUpPress[Pos]].Off();                                                                 //JerryYang 20190423 新增unloader壓tray
                    Task=300;
                }
                break;
            }
            iRetry[Pos]=0;

            if(MOT[iMMAuto[Pos]].fHasTray==false &&                                                                     //Auto紀錄沒有Tray Auto偵測有Tray
               Sen[SnAutoTrayDetect[Pos]].IsOn())
            {
                if(LastSet.iRealDummy!=DUMMY &&                                                                       //AI(ht9045-v899) 20260513: dummy run does not enable P18 bypass; keep original tray mismatch handling
                  (IniConfig.bP18FailAutoTrayManual==true ||                                                           //jou 2012-03-16 Fail Auto Tray手動補Tray
                  ((iRunStartMode==FT || iRunStartMode==FT_ART) && TrayForm.bFailAutoTrayManual_FT==true) ||            //Steven 20150116 : 手動移除Auto Fail Bin Tray
                  ((iRunStartMode==RT || iRunStartMode==RT_ART) && TrayForm.bFailAutoTrayManual_RT==true)) &&
                    Prod.iIsFailT6[Pos]==1)                                                                             //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6 //JerryYang 20191111 initial start放tray盤在Auto區會誤發alarm
                {
                }
                else
                {
                    if((LastSet.iRunStartMode==rsmContinuStart || LastSet.iRunStartMode==rsmContinuRetest) && bOldAutoHasTray[Pos]==true)
                    {
                        MOT[iMMAuto[Pos]].fHasTray=true;
                        MOT[iMMAuto[Pos]].SetTray(NULL_IC, __FUNC__);
                        bOldAutoHasTray[Pos]=false;
                        for(x=0; x<MAX_X_ITEM; x++)
                            for(y=0; y<MAX_Y_ITEM; y++)
                                MOT[iMMAuto[Pos]].SetTraySingleData(x, y, iAutoTrayData[Pos][x][y]);
                        break;
                    }
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();                                                              //kevin 20150530 手動取出tray 釋放 氣缸
                    Cylinder[C_AutoEdgePush[Pos]].Off();                                                                //kevin 20150530 手動取出tray 釋放 氣缸
                    Cylinder[C_AutoUpPress[Pos]].Off();                                                                 //JerryYang 20190423 新增unloader壓tray

                    if(IniConfig.bSPILFunction==true)                                                                   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
                        ShowErrorMessage(sJAM1110[Pos], K_RETRY, iMMAuto[Pos]);                                         //wei 20160511 矽品Auto紀錄沒有Tray Auto偵測有Tray 只能Skip
                    else
                        ShowErrorMessage(sJAM1110[Pos], K_RETRY, iMMAuto[Pos]);
                    break;
                }
            }

            if(MOT[iMMAuto_Car[Pos]].fHasTray)                                                                          //Sam 20200707 : 避免 Auto Tray 要分離的時候被 Home 導致資料還在 MMAuto1_Car，可能會造成 Clean Out Hang up
            {
                if(MOT[iMMAuto[Pos]].fHasTray)
                {
                    if(CosFunction.bAutoTrayFullAlarmCanNotSkip)                                                        //Steven 20210219 : Unloader滿盤alarm不能Skip
                        ret=ShowErrorMessage(sMES1120[Pos], K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_3");
                    else
                        ret=ShowErrorMessage(sMES1120[Pos], K_SKIP|K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_3");

                    if(ret==K_SKIP)
                    {
                        CheckHasErrorBinOnTray(iMMAuto_Car[Pos]);                                                       //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                        fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto_Car[Pos]);                                             //Sam 20240325 : 新增 DamageTrayMapping 功能
                        MOT[iMMAuto_Car[Pos]].ClearTray(__FUNC__);
                    }
                }
                else
                {
                    DetectAutoTime[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayArrivalWaitDelay);
                    Task=1210;
                }

                break;
            }
            Task=100;
            break;
        case 100:
            if(MOT[iMMAuto[Pos]].fHasTray==false)                               //Auto沒有Tray
            {
                Cylinder[C_AutoSide_Fixer[Pos]].Off();
                Cylinder[C_AutoEdgePush[Pos]].Off();
                Cylinder[C_AutoUpPress[Pos]].Off();                             //JerryYang 20190423 新增unloader壓tray
            }

            if(IniConfig.bP37bAutoCylinderUP)                                   //kevin 20180726 (wei) Auto 123 氣缸常態在上
                Task=110;
            else
                Task=200;
            break;
        case 110:
            if(IniConfig.bP37bAutoCylinderUP)
            {
                Task=111;
                for(int i=eAuto1; i<=iAutoRight; i++)
                {
                    bflag[i]=false;                                                                                     //升到接盤位置
                    AutoCylinderLower(i, C_Auto_Up[i], C_Auto_Selector[i], true);                                       //Steven 20140409 : AutoCylinderXX Add Reset
                    AutoCylinderMiddle(i, C_Auto_Up[i], C_Auto_Selector[i], true);                                      //升到接盤位置
                    AutoCylinderUp(i, C_Auto_Up[i], C_Auto_Selector[i], true);                                          //升到接盤位置
                }
            }
            break;
        case 111:
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(Sen[SnAutoTrackDetect[i]].IsOn() ||
                   Sen[SnAutoTrayCar[i]].IsOn())                                //軌道上有tray
                {
                    bflag[i]=true;                                              //kevin 20180726 Auto 12 3 up
                    bFlag=true;
                }
            }

            if(bFlag)
                Task=112;
            else
                Task=113;
            break;
        case 112:
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(bflag[i])                                                                                            //kevin 20180726 Auto 12 3 up
                {
                    ibuffer++;
                    if(AutoCylinderMiddle(i, C_Auto_Up[i], C_Auto_Selector[i]))                                         //升到接盤位置
                    {
                        Cylinder[C_AutoZ_Select[i]].On();
                        bflag[i]=false;
                    }
                }
            }

            if(ibuffer==0)
                Task=113;
            break;
        case 113:
            if(AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))       //kevin 20190723 change  //升到接盤位置
                Task=200;
            break;
        case 200:
            if(MOT[iMMAuto[Pos]].fHasTray)                                      //Auto有Tray
                Task=500;
            else                                                                //Auto沒有Tray
                Task=300;
            break;
        case 300:                                                               //等待CatchArm 補Tray
            if(MOT[iMMAuto[Pos]].fHasTray)                                      //取得Tray盤
            {
                if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2 && Pos==0)     //kevin 20120718 load 空tray放auto1
                {
                     Task=500;                                                  //收auto1 tray
                }
                else
                {
                    if(Cylinder[C_AutoEdgePush[Pos]].Enable)
                    {
                        if(bAutoEdgePush[Pos])
                            Task=1600;
                    }
                    else
                    {
                        Task=1600;
                    }
                }
            }
            break;
        case 500:                                                               //Auto偵測有Tray
            if(LastSet.iRealDummy==DUMMY ||
               Sen[SnAutoTrayDetect[Pos]].IsOff()==false)
            {
                if(USE_COLOR_TRAY_SENSOR==1 &&                                  //20140903 wei colcr Tray
                   TrayForm.bColorTray &&
                   LastSet.iRealDummy!=DUMMY &&
                   Sen[SnAutoColorTrayDete[Pos]].Enable)
                {
                    if(CUSTOMER_CODE==CC_TERAPOWER)                             //Sam 20180525 (wei) : 晶兆成 Auto1~3 Color sensor detect By FromEmptyColor
                    {
                        bColorFlag=ForTERAPOWERCheckColorSensor(Pos);
                        if(bColorFlag==true)
                        {
                            asColorSensorAlarm="500";
                            ColorSensorDelayTime[Pos].SetSecAndOn(60);          //Sam 20211028 : 增加保護防止 Hange up
                            Task=1650;                                          //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。
                            break;
                        }
                    }
                    else
                    {
                        bColorFlag=Sen[SnAutoColorTrayDete[Pos]].IsOff();
                        if(bColorFlag==true)                                    //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。
                        {                                                       //Steven 20150427 : For Color Sensor, 滅掉是紅色
                            ret=ShowErrorMessage(sWAR1151[Pos], K_RETRY, iMMAuto[Pos], false, "DoAutoReceiveBinTray_500");
                            break;
                        }
                    }
                }

                ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayArrivalDelay);
                Task=800;
                bDupError[Pos]=false;                                           //Steven 20120208 : Auto Tray沒放好,重複Alarm
            }
            else                                                                //Auto偵測沒有Tray
            {
                if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                    break;
                Task=750;
            }
            break;
        case 750:
            Cylinder[C_AutoSide_Fixer[Pos]].Off();
            Cylinder[C_AutoEdgePush[Pos]].Off();
            Cylinder[C_AutoUpPress[Pos]].Off();                                 //JerryYang 20190423 新增unloader壓tray
            ret=ShowErrorMessage(sJAM1109[Pos], K_SKIP|K_RETRY, iMMAuto[Pos], bDupError[Pos], __FUNC__);
            bDupError[Pos]=true;                                                //Steven 20120208 : Auto Tray沒放好,重複Alarm

            if(ret==K_SKIP)
            {
                CheckHasErrorBinOnTray(iMMAuto[Pos]);                           //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto[Pos]);                 //Sam 20240325 : 新增 DamageTrayMapping 功能
                fProductionInfo->CalTrayICCount(Pos);
                MOT[iMMAuto[Pos]].ClearTray(__FUNC__);
                bDupError[Pos]=false;                                           //Steven 20120208 : Auto Tray沒放好,重複Alarm
                Task=100;
            }
            else
            {
                if(SoftStop==false)
                {
                    Cylinder[C_AutoSide_Fixer[Pos]].On();
                    Cylinder[C_AutoEdgePush[Pos]].On();
                    Cylinder[C_AutoUpPress[Pos]].On();                          //JerryYang 20190423 新增unloader壓tray
                }
                Task=500;
            }
            break;
        case 800:                                                               //將Auto Tray 固定
            bAUTORev[Pos]=false;
            if(ULDStackDelay[Pos].Off())
            {
                if(Cylinder[C_AutoSide_Fixer[Pos]].Push())
                {
                    Cylinder[C_AutoEdgePush[Pos]].On();
                    Cylinder[C_AutoUpPress[Pos]].On();                          //JerryYang 20190423 新增unloader壓tray
                    Task=850;
                }
            }
            break;
        case 850:                                                                                                       //Auto偵測沒有Tray
            if(LastSet.iRealDummy!=DUMMY &&
               Sen[SnAutoTrayDetect[Pos]].IsOff()==true)
            {
                iRetry[Pos]++;
                if(iRetry[Pos]<3000)                                                                                    //Steven 20160530 : 30 --> 3000 for longer delay
                    break;

                if(IniConfig.bNewResetFunction==true && bResetUnLoadTray[Pos]==true)
                {
                    bResetUnLoadTray[Pos]=false;
                    ret=K_SKIP;
                }
                else
                {
                    ret=ShowErrorMessage(sJAM1108[Pos], K_SKIP|K_RETRY, iMMAuto[Pos]);
                }

                if(ret==K_SKIP)
                {
                    CheckHasErrorBinOnTray(iMMAuto[Pos]);                                                               //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                    fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto[Pos]);                                                     //Sam 20240325 : 新增 DamageTrayMapping 功能
                    fProductionInfo->CalTrayICCount(Pos);
                    MOT[iMMAuto[Pos]].ClearTray(__FUNC__);
                    MOT[iMMAuto[Pos]].fHasTray=false;
                    Task=1;
                }
                else if(ret==K_RETRY)
                {
                    Task=1;                                                                                             //Steven 20100225
                }

                if(SoftStop)
                {
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();
                    Cylinder[C_AutoEdgePush[Pos]].Off();
                    Cylinder[C_AutoUpPress[Pos]].Off();                                                                 //JerryYang 20190423 新增unloader壓tray
                }
                break;
            }

            bResetUnLoadTray[Pos]=false;
            iRetry[Pos]=0;
            bAUTORev[Pos]=false;

            if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&                                                                       //kevin 20161028
               TRAY_VIBRATION==NonVibration)                                                                            //Steven 20110627 : new Tray Vibration  //JerryYang 20170531 (wei) 敲tray方式新增震動馬達
            {
                if(Cylinder[C_AutoEdgePush[Pos]].Enable)
                {
                    if(bAutoEdgePush[Pos])
                    {
                        if(DoAutoTrayEdgeCylinderLoop(Pos)==false)                                                      //Steven 20100915 : 跳蛋功能
                        {
                            Cylinder[C_AutoSide_Fixer[Pos]].On();
                            Cylinder[C_AutoEdgePush[Pos]].On();
                            Cylinder[C_AutoUpPress[Pos]].On();                                                          //JerryYang 20190423 新增unloader壓tray
                        }
                    }
                }
            }

            if(IniConfig.bP61UseTrayTap==true)                                                                          //Ztex 2024.11.12 Add P53 Use Tray Tap
            {
                if(Cylinder[C_AutoEdgePush[Pos]].Enable)
                {
                    if(DoAutoTrayEdgeCylinderLoop(Pos)==false)
                    {
                        Cylinder[C_AutoEdgePush[Pos]].On();
                    }
                }
            }

            if(Pos==eAuto1 && TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                                            //kevin 201250718 load 空tray放auto2
            {
                if(MOT[iMMAuto[Pos]].Tray.HasIC()==false)                                                               //kevin 20120914
                    Task=900;
            }
            else if(MOT[iMMAuto[Pos]].Tray.FullIC())                                                                    //Auto滿盤
            {
                if(MOT[MMAuto1+Pos].Tray.HasDataIC(HAS_NULL_IC)==true)                                                  //JerryYang 20250120 : add
                {
                    str1.sprintf("AUTO%d中有空格, 請手動取出tray盤", Pos+1);
                    ShowMyMessage(str1);
                    break;
                }
                iPosY=MOT[MOutArmY].ReadPos();                                                                          //JerryYang 20170919 (Steven) Auto退tray增加保護,避免Out arm Z軸尚未上升就退tray
                if((iPosY<Prod.YOutArm_Shuttle1_Pick[0][2]) &&
                    CheckOutArmZ(false)==true)                                                                          //kevin 20220919 OutArm Shuttle 1 pick hang up
                {
                    break;
                }

                if(AMR.CheckUnloaderCount(Pos))                                                                         //Sam 20240304 : 新增 AMR 功能
                     break;

                if(bARTUnloaderUseTwoCylin(Pos) &&
                   (IniConfig.bA10_AutoReTest || bAutoReTest_ART) &&
                   LastSet.bLoaderTrayCount_ART &&
                   Prod.bART6Tray[Pos])                                                                                 //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
                {
                    LastSet.iUnloaderTrayCount_ART[Pos]++;
                    iAutoTrayCount[Pos]++;                                                                              //wei 20150828

                    fMain->lblAuto1TrayCnt->Caption=LastSet.iUnloaderTrayCount_ART[eAuto1];
                    fMain->lblAuto2TrayCnt->Caption=LastSet.iUnloaderTrayCount_ART[eAuto2];
                    fMain->lblAuto3TrayCnt->Caption=LastSet.iUnloaderTrayCount_ART[eAuto3];
                    fMain->lblAuto4TrayCnt->Caption=LastSet.iUnloaderTrayCount_ART[eAuto4];
                    fMain->lblAuto5TrayCnt->Caption=LastSet.iUnloaderTrayCount_ART[eAuto5];
                    fMain->lblAuto6TrayCnt->Caption=LastSet.iUnloaderTrayCount_ART[eAuto6];
                }

                fMain->edtAuto1->Text=LastSet.iUnloaderTrayCount_ART[eAuto1];
                fMain->edtAuto2->Text=LastSet.iUnloaderTrayCount_ART[eAuto2];
                fMain->edtAuto3->Text=LastSet.iUnloaderTrayCount_ART[eAuto3];
                fMain->edtAuto4->Text=LastSet.iUnloaderTrayCount_ART[eAuto4];
                fMain->edtAuto5->Text=LastSet.iUnloaderTrayCount_ART[eAuto5];
                fMain->edtAuto6->Text=LastSet.iUnloaderTrayCount_ART[eAuto6];

                Task=900;

                //==> Eastsun 20260512 : Phase4-F5T2 KYEC AMR (a-side asendic_Auto.cpp L546-L570)
                if(TrayForm.bEnableAMR)
                {
                    iUnloaderTrayCountCal[Pos]++;
                    TestIF_File.iAMRTrayCount[Pos+3]++;
                    if(asColorCoverTrayID!="")
                    {
                        if(Pos==0)      fLotInfo->labNowAuto1TrayID->Caption=asColorCoverTrayID;
                        else if(Pos==1) fLotInfo->labNowAuto2TrayID->Caption=asColorCoverTrayID;
                        else if(Pos==2) fLotInfo->labNowAuto3TrayID->Caption=asColorCoverTrayID;
                        asColorCoverTrayID="";
                    }

                    if(TrayForm.bEnableAMRLoader==false && !bAMRFullICBin && iTrayFeed==0) //Eastsun20260123
                    {
                        if(iUnloaderTrayCountCal[Pos]>=TrayForm.iFullTrayCount) //盤子疊太高 要叫AGV取料
                        {
                            bNeedCoverTray[Pos]=true;
                            bAMRFullTray[Pos]=true;
                        }
                    }
                }
                //<== Eastsun 20260512

                if(TestIF_File.bUseAutoTrayMap)                                                                         //kevin 20220503 use auto empty tray
                {
                    iUnloaderTrayCountCal[Pos]++;
                    if(iUnloaderTrayCountCal[Pos]>TestIF_File.iUnloaderTrayCount[Pos])
                    {
                        if(TestIF_File.bDoubleUnloadTray)
                        {
                            if(iUnloaderTrayCountCal[Pos]>TestIF_File.iUnloaderTrayCount[Pos]*2)                        //kevin 20220506 add Unload Tray 2  倍的設定
                                iUnloaderTrayCountCal[Pos]=0;                                                           // 滿足條件需要下貨
                        }
                        else
                        {
                            iUnloaderTrayCountCal[Pos]=0;                                                               // 滿足條件需要下貨
                        }
                    }
                }
                else
                {
                    iUnloaderTrayCountCal[Pos]++;
                    if(iUnloaderTrayCountCal[Pos]>=fSCKART->iBundleOutCnt)
                    {
                        bHas1DCoverTray[Pos]=false;
                        bNeedCoverTray[Pos]=false;
                    }
                }

                if(CUSTOMER_CODE==CC_TSI)
                {
                    LastSet.RecodeTrayCount[Pos]++;                                                                     //frank 20200814 : 每10盤記錄一次summary log
                    if(LastSet.RecodeTrayCount[Pos]>=10)
                    {
                        SaveTrayRecord(Pos);
                        LastSet.RecodeTrayCount[Pos]=0;
                    }
                    fMain->lblAuto1TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto1];
                    fMain->lblAuto2TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto2];
                    fMain->lblAuto3TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto3];
                    fMain->lblAuto4TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto4];
                    fMain->lblAuto5TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto5];
                    fMain->lblAuto6TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto6];
                }
                SaveProductionRecord(&MOT[iMMAuto[Pos]].Tray, s6TrayName[Pos]);                                         //Steven 20200330 : production log by unloader tray存檔

                if(IniConfig.bA68_AutoLoadUnload)                                                                       //JerryYang 20250521 : For AMR
                {
                    if(Pos==eAuto1)
                    {
                        bPortIsBusy[ePortAuto1]=true;
                    }
                    else if(Pos==eAuto2)
                    {
                        bPortIsBusy[ePortAuto2]=true;
                    }
                    else if(Pos==eAuto3)
                    {
                        bPortIsBusy[ePortAuto3]=true;
                    }
                }
            }
            break;
        case 900:
            if(USE_LdUldCassetteMode!=1)                                                                                //RogerYang 20260203 : Add for HT9046CR
                bAUTORev[Pos]=true;
            if(IniConfig.bP14EnableAutoTrayRecevieDelayCount==true &&                                                   //wei 20160309 LastSet-->IniConfig
               IniConfig.iP14AutoTrayRecevieDelayCount>0)
            {
                if(TRAY_VIBRATION!=NonVibration)                                                                        //JerryYang 20170531 (wei) 敲tray方式新增震動馬達
                {
                    RecevieLoopDelay[0].SetMSAndOn(IniConfig.iP14AutoTrayRecevieLoopDelayTime*100);                     //ChungHung 20150304 modify 0.1s cyclinder can not active

                    iRecevieLoopCount[0]=0;
                    bRecevieLoop[0]=true;
                }
                else
                {
                    RecevieLoopDelay[Pos].SetMSAndOn(IniConfig.iP14AutoTrayRecevieLoopDelayTime*100);                   //ChungHung 20150304 modify 0.1s cyclinder can not active
                    iRecevieLoopCount[Pos]=0;
                    bRecevieLoop[Pos]=true;
                }

                if(CosFunction.bUnloadCylinerUpRelief &&                                                                //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
                   LOAD_Z_USE_MOTOR[3+Pos]==false)
                {
                    Cylinder[C_Auto_Selector[Pos]].On();
                    Cylinder[C_Auto_Up[Pos]].On();
                    htUnLoaderTimeRelief[Pos].SetMSAndOn(1000);                                                         //Sam 20220927 : 0.2s>1s
                }
                Task=910;
            }
            else
            {
                Task=950;
            }
            break;
        case 910:
            if(TRAY_VIBRATION!=NonVibration)                                                                            //JerryYang 20170531 (wei) 敲tray方式新增震動馬達
            {
                if(RecevieLoopDelay[0].Off())
                {
                    bRecevieLoop[0]=!bRecevieLoop[0];

                    if(bRecevieLoop[0])
                        Cylinder[C_TrayVibration].On();
                    else
                        Cylinder[C_TrayVibration].Off();

                    RecevieLoopDelay[0].SetMSAndOn(IniConfig.iP14AutoTrayRecevieLoopDelayTime*100);                     //ChungHung 20150304 modify 0.1s cyclinder can not active
                    if(bRecevieLoop[0]==false)                                                                          //KaiChen 20190527 ：Fix 退盤震動不會關閉問題
                    {
                        iRecevieLoopCount[0]++;
                    }

                    if(iRecevieLoopCount[0]>IniConfig.iP14AutoTrayRecevieDelayCount)                                    //KaiChen 20190527 ：Fix 退盤震動不會關閉問題
                    {
                        Task=955;                                                                                       //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
                    }
                }
            }
            else
            {
                if(RecevieLoopDelay[Pos].Off())
                {
                    bRecevieLoop[Pos]=!bRecevieLoop[Pos];

                    if(CUSTOMER_CODE!=CC_TSMC_TAINAN)                                                                   //wei 20170116 (jou) TSMC不使用側推氣缸敲Tray
                    {
                        if(bRecevieLoop[Pos])
                            Cylinder[C_AutoEdgePush[Pos]].On();
                        else
                            Cylinder[C_AutoEdgePush[Pos]].Off();
                    }
                    RecevieLoopDelay[Pos].SetMSAndOn(IniConfig.iP14AutoTrayRecevieLoopDelayTime*100);                   //ChungHung 20150304 modify 0.1s cyclinder can not active
                    if(bRecevieLoop[Pos]==true)
                    {
                        iRecevieLoopCount[Pos]++;
                    }

                    if(iRecevieLoopCount[Pos]>=IniConfig.iP14AutoTrayRecevieDelayCount)
                    {
                        Task=955;                                                                                       //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
                    }
                }
            }

            if(CosFunction.bUnloadCylinerUpRelief &&                                                                    //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
               LOAD_Z_USE_MOTOR[3+Pos]==false)
            {
                if(htUnLoaderTimeRelief[Pos].Off())
                {
                    Cylinder[C_Auto_Selector[Pos]].Off();
                    Cylinder[C_Auto_Up[Pos]].Off();
                }
            }
            break;
        case 920:                                                               //Isaac 20180109 (Steven) : auto123可前進後退，auto track go rear
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                TrayArmMotorMove(Prod.iXTrayEmpty);
                if(DoAuto123TrayToRear(Pos))
                {
                    ShowErrorMessage(sJAM1110[Pos], K_RETRY, iMMAuto[Pos]);     //JerryYang 20171101 (wei) Retry改成Skip避免誤會
                    Task=950;                                                   //再次回收Auto Tray盤並檢查
                }
            }
            break;
        case 950:                                                               //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
            if(CosFunction.bUnloadCylinerUpRelief && LOAD_Z_USE_MOTOR[3+Pos]==false)
            {
                Cylinder[C_Auto_Selector[Pos]].On();
                Cylinder[C_Auto_Up[Pos]].On();
                htUnLoaderTimeRelief[Pos].SetMSAndOn(1000);                     //Sam 20220927 : 0.2s>1s
            }
            Task=955;
        case 955:
            if(CosFunction.bUnloadCylinerUpRelief &&                                                                    //Sam 20220916 : 退 Tray 前 Unloader 上升汽缸跑減壓動作防止震盤。
               LOAD_Z_USE_MOTOR[3+Pos]==false)
            {
                if(htUnLoaderTimeRelief[Pos].Off())
                {
                    Cylinder[C_Auto_Selector[Pos]].Off();
                    Cylinder[C_Auto_Up[Pos]].Off();
                }
                else
                {
                    break;
                }
            }

            if(bAtuoTrayICDetectErr==true)                                                                              //Isaac 20180109 (Steven) : auto123可前進後退
            {
                bAtuoTrayICDetectErr=false;
                CatchTrayTask=1200;                                                                                     //TrayArm暫時不要到需換新Tray的Auto上
            }

            Cylinder[C_AutoUpPress[Pos]].Off();                                                                         //JerryYang 20190423 新增unloader壓tray
            Cylinder[C_AutoEdgePush[Pos]].Off();                                                                        //放開Auto Lock
            if(Cylinder[C_AutoSide_Fixer[Pos]].Pop())
            {
                if(USE_LdUldCassetteMode==1)                                                                            //Frank 20251217 add
                {
                    Task=1010;
                }
                else
                {
                    if(bARTUnloaderUseTwoCylin(Pos))
                    {
                        AutoCylinderLower(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos],true);
                    }
                    else
                    {
                        AutoCylinderLower(Pos, C_Auto_Selector[Pos], C_Auto_Up[Pos],true);                              //JerryYang 20250721 : fix non ART 汽缸沒有下降
                    }
                    Task=960;
                }
            }

            if(TrayForm.bMoveAfterTrayGoOut==true &&
               TrayForm.bAutoTrayGoOutNeedDelay[Pos]==true &&
               Pos <=2)
            {
                DoTrayGoOutNeedDelay[Pos].SetSecAndOn(TrayForm.ibAutoTrayGoOutDelayTime);
                Task=956;
            }
            break;
        case 956:                                                               //kevin 20120718  Auto 2 Auto 3  移到AUTO2 取空TRAY
             if(DoTrayGoOutNeedDelay[Pos].Off())
            {
                Task=960;
            }
            break;
        case 960:
            if(bARTUnloaderUseTwoCylin(Pos))
            {
                if(AutoCylinderLower(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))                                        //kevin 20180720 (wei) Auto 123 氣缸在下
                {
                    Task=970;
                    DetectAutoTimePause[Pos].SetMSAndOn(600);                                                           //JerryYang 20170914 (Steven) 200->600ms,避免break再進來所花的時間超過計時器,造成不會發出逾時alarm
                }
            }
            else
            {
                if(AutoCylinderLower(Pos, C_Auto_Selector[Pos], C_Auto_Up[Pos]))                                        //JerryYang 20250721 : fix non ART 汽缸沒有下降
                {
                    Task=970;
                    DetectAutoTimePause[Pos].SetMSAndOn(600);
                }
            }
            break;
        case 970:
            if(DetectAutoTimePause[Pos].Off())
            {
                Task=1000;
            }
            break;
        case 1000:
            if(Sen[SnAutoTrayCar[Pos]].IsOn()==true ||                                                                                                          //kevin 20161129 tray到位sensor 亮起 要alarm 否則 tray夾斷
               Sen[SnAutoPreDete[Pos]].IsOn()==true)                                                                                                            //Steven 20230303 : 多檢查一個Sensor
            {
                ShowErrorMessage(sMES1120[Pos], K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_1000");                                                 //Sam 20200630 : Add Log
            }
            else
            {
                if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bG11ASEReport)                                                                                  //kevin 20210810 read tray id tput new tray
                {
                    TrayID[3+Pos][2]=TrayID[3+Pos][1];                                                                                                          //kevin 20210623  trid move
                    if(((iUnloaderTrayCountCal[Pos]==0 ||
                        (iUnloaderTrayCountCal[Pos]==(IniConfig.iUnloaderTrayCount[Pos]+1)))  && IniConfig.b18bUseAutoTrayMap) ||
                       ((iUnloaderTrayCountCal[Pos]==(IniConfig.iUnloaderTrayCount[Pos]*2+1)) && IniConfig.b18bDoubleUnloadTray))                               //kevin 20220908 fix empty tray
                    {
                        str1.sprintf("<AutoMove>Auto Unloader Auto %d move out cover,%s", Pos+1, TrayID[3+Pos][1]);                                             //kevin 20210623
                        RecordProcess(str1);                                                                                                                    //kevin 20210604 add Auto tray receive srart
                    }
                    else
                    {
                        str1.sprintf("<AutoMove>Auto Unloader Auto %d move out ICs,%d,%s", Pos+1, iOneTrayPickCount[1+Pos], TrayID[3+Pos][1]);                  //kevin 20210623
                        RecordProcess(str1);                                                                                                                    //kevin 20210604 add Auto tray receive srart
                        ReadWriteTrayID(false);                                                                                                                 //kevin 20220618 record Auto 1 2 3 tray Receive
                    }
                }

                if(LastSet.iRealDummy!=DUMMY)                                                                                                                   //JerryYang 20241021 : Unloader增加第二組Sensor檢查是否有cover tray
                {
                    if(Sen[SnAutoHasCoverTray[Pos]].Enable &&
                       Sen[SnAutoHasCoverTray[Pos]].IsOn()==false)
                    {
                        str2.sprintf("Please check there is a COVER TRAY on the top of AUTO%d", Pos+1);                                                         //JerryYang 20250905 : 修正message
                        str.sprintf("請確認AUTO%d上面有Cover tray", Pos+1);
                        ShowMyMessage(str2, str);
                        break;
                    }
                }
                Task=1010;
            }
            break;
        case 1010:                                                              //回收Auto Tray盤
            Cylinder[C_AutoEdgePush[Pos]].Off();
            Cylinder[C_AutoSide_Fixer[Pos]].Off();
            Cylinder[C_AutoUpPress[Pos]].Off();                                 //JerryYang 20190423 新增unloader壓tray
            TrayMoveOut(true, Pos+3);

            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(Pos+3);

            AutoReceiveBinTrayTimer[Pos].LatchCycleTime(true);

            #ifdef SOFT_SIMULTE
            DetectAutoTime[Pos].SetMSAndOn(300);
            Task=1020;
            #else
            if(LastSet.iRealDummy==DUMMY &&
               Sen[SnAutoTrayDetect[Pos]].IsOff()==true)                        //Auto偵測Tray and Dummy
            {
                DetectAutoTime[Pos].SetMSAndOn(2000);
                if(USE_LdUldCassetteMode==1)                                    //RogerYang 20260207 : Add fot 9046CR
                {
                    InitAutoCassetteTrayFeedTask(Pos);
                    Task=1030;
                    break;
                }
                Task=1022;
            }
            else                                                                //設定Delay時間
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    htUnLoaderTimeDelay[Pos].SetMSAndOn(LastSet.iUnLoaderTraySimulateTime[Pos]+1000);
                }
                DetectAutoTime[Pos].SetMSAndOn(2000);
                Task=1020;
            }
            #endif
            break;
        case 1020:
            if(DetectAutoTime[Pos].Off())                                       //Steven 20230303 : 增加一個讓Tray滾出來的Delay
            {
                TrayMoveOut(true, Pos+3);
                if(TRAY_ARM_MODE==eUnderCoveyor)
                    TrayCylinMoveOut(Pos+3);
                DetectAutoTime[Pos].SetMSAndOn(20000);
                DetectAutoTimePause[Pos].SetMSAndOn(600);                       //JerryYang 20170914 (Steven) 200->600ms,避免break再進來所花的時間超過計時器,造成不會發出逾時alarm
                Task=1021;
            }
            break;
        case 1021:
            if(bHandlerPause)                                                                                           //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                Task=1010;
                break;
            }

            if(DetectAutoTimePause[Pos].Off())
            {
                DetectAutoTime[Pos].On();
                DetectAutoTimePause[Pos].On();
                break;
            }
            else
            {
                DetectAutoTimePause[Pos].On();
            }

            TrayMoveOut(true, Pos+3);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(Pos+3);

            if(USE_LdUldCassetteMode==1)                                                                                //Frank 20251217 add
            {
                InitAutoCassetteTrayFeedTask(Pos);
                Task=1030;
                break;
            }

            if(bUseAuto2Empty==0 &&
               (USE_AUTO_RETEST==eartUninstall ||                                                                       //ChungHung 20140317 add Auto Retest
               (USE_AUTO_RETEST==eartInstall && UNLOADER_ART[Pos]==eartUninstall)))                                     //Steven 20161221 (jou) : for SCK only Auto 2 has ART
            {
                if((Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                    Sen[SnAutoUpSafe[Pos]].IsOff()==false) ||
                   (MachineTypeChoice==Type_HT1032 &&
                    Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                    Sen[SnAutoUpSafe[Pos]].IsOff()))
                {
                    DetectAutoTime[Pos].SetMSAndOn(100);
                    DetectAutoTimePause[Pos].SetMSAndOn(150);
                    Task=1025;
                    break;
                }
            }
            else
            {
                if(LastSet.iRealDummy==DUMMY ||
                   (MachineTypeChoice==Type_HT1032 &&
                    Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                    Sen[SnAutoUpSafe[Pos]].IsOff()) ||
                   (Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                    Sen[SnAutoUpSafe[Pos]].IsOff()==false))                                                             // kevin 20120726
                {
                    DetectAutoTime[Pos].SetMSAndOn(100);
                    DetectAutoTimePause[Pos].SetMSAndOn(150);
                    Task=1050;
                    break;
                }
            }

            if(DetectAutoTime[Pos].Off())
            {
                ret=ShowErrorMessage(sJAM1112[Pos], K_RETRY, iMMAuto[Pos], false, "DoAutoReceiveBinTray_1021");
                if(ret==K_RETRY)
                    Task=1010;                                                                                          //JerryYang 20250418 : 1000->1010 避免Tray盤已經遮到減速sensor的時候會誤alarm
            }
            break;
        case 1022:
            TrayMoveOut(true, Pos+3);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(Pos+3);
            if(DetectAutoTime[Pos].Off())
                Task=1052;
            break;
        case 1025:
            if(DetectAutoTimePause[Pos].Off())                                  //有Hang Up風險
            {
                DetectAutoTime[Pos].On();
                DetectAutoTimePause[Pos].On();
                break;
            }
            else
            {
                DetectAutoTimePause[Pos].On();
            }

            if(DetectAutoTime[Pos].Off())
            {
                if((Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                    Sen[SnAutoUpSafe[Pos]].IsOff()==false) ||
                   (MachineTypeChoice==Type_HT1032 &&
                    Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                    (Sen[SnAutoUpSafe[Pos]].Enable==false ||                    //Ztex 2023.12.16 Add HT-1032
                     Sen[SnAutoUpSafe[Pos]].IsOff())))
                {
                    Task=1050;
                }
                else
                {
                    TrayMoveOut(true, Pos+3);
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                        TrayCylinMoveOut(Pos+3);
                    DetectAutoTime[Pos].SetMSAndOn(20000);                      //Steven 20150106 : 改成20000
                    Task=1020;
                }
            }
            break;
        case 1030:                                                              //推出來                                                 //RogerYang 20260207 Add for 9046CR
            if(DoAutoCassetteTrayFeed(Pos))                                     //Frank 20251217 add
            {
                Task=1200;
            }
            break;
        case 1050:
            if(bUseAuto2Empty==0 && bNoAutoZSelect==false &&                                                            //kevin 20181023 (Steven) : 不使用2段分tray氣缸
               (USE_AUTO_RETEST==eartUninstall ||                                                                       //ChungHung 20140317 add Auto Retest
               (USE_AUTO_RETEST==eartInstall && UNLOADER_ART[Pos]==eartUninstall)))                                     //Steven 20161221 (jou) : for SCK only Auto 2 has ART
            {
                if(MachineTypeChoice==Type_HT1032)
                {
                    bFlag=(Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                           (Sen[SnAutoUpSafe[Pos]].Enable==false ||
                            Sen[SnAutoUpSafe[Pos]].IsOff()));                                                           //Ztex 2023.12.16 Add HT-1032
                }
                else
                {
                    bFlag=(Sen[SnAutoTrayCar[Pos]].IsOff()==false &&
                           Sen[SnAutoUpSafe[Pos]].IsOff()==false);
                }
            }
            else
            {
                if(LastSet.iRealDummy==DUMMY)
                    bFlag=true;
                else
                    bFlag=(Sen[SnAutoTrayCar[Pos]].IsOff()==false && Sen[SnAutoUpSafe[Pos]].IsOff()==false);
            }
            #ifdef SOFT_SIMULTE
                bFlag=true;
            #endif

            if(bFlag)                                                                                                   // kevin 20120726
            {
                if(LastSet.iRealDummy!=DUMMY)
                {
                    PushUnLoaderTrayInAverageTime(AutoReceiveBinTrayTimer[Pos].LatchCycleTime(), Pos);
                }
                else
                {
                    if(htUnLoaderTimeDelay[Pos].Off()==false)
                        break;
                }

                if(IniConfig.b18bUseAutoTrayMap)                                                                        //kevin 20220819 use auto empty tray
                {
                    if(IniConfig.b18bDoubleUnloadTray)
                    {
                        if(iUnloaderTrayCountCal[Pos]!=(IniConfig.iUnloaderTrayCount[Pos]+1)||
                           iUnloaderTrayCountCal[Pos]!=(IniConfig.iUnloaderTrayCount[Pos]*2+1))                         //kevin 20220506 add Unload Tray 2  倍的設定
                        {
                            fBarCode->Write_Device_Info_By_Tray(Pos, LastSet.iDevice_Info_By_Tray[Pos]);
                            LastSet.iDevice_Info_By_Tray[Pos]++;
                            iUnloaderTrayCountCal[Pos]++;                                                               //kevin 20220706 add count
                        }
                    }
                    else if(iUnloaderTrayCountCal[Pos]!=(IniConfig.iUnloaderTrayCount[Pos]+1))                          //kevin 20220706 add count
                    {
                        fBarCode->Write_Device_Info_By_Tray(Pos, LastSet.iDevice_Info_By_Tray[Pos]);
                        LastSet.iDevice_Info_By_Tray[Pos]++;
                        iUnloaderTrayCountCal[Pos]++;                                                                   //kevin 20220706 add count
                    }
                }

                if(LastSet.iRealDummy==REALLY           &&
                   TestIF_File.bEnableBarCode==true     &&
                   CosFunction.bBarcodeTrayRecFile==true )                                                              //jou 20190930 : Barcode Tray record file
                {
                    fBarCode->Write_Device_Info_By_Tray(Pos, LastSet.iDevice_Info_By_Tray[Pos]);
                    LastSet.iDevice_Info_By_Tray[Pos]++;
                }

                SaveUnloaderInfo(Pos);                                                                                  //ChungHung 20150205 add for ATK
                AutoReceiveBinTrayTimer[Pos].LatchCycleTime(true);
                Task=1100;
            }
            else
            {
                TrayMoveOut(true, Pos+3);
                if(TRAY_ARM_MODE==eUnderCoveyor)
                    TrayCylinMoveOut(Pos+3);
                Task=1020;
            }
            break;
        case 1052:
            bAtuoTrayICDetectErr=false;                                         //Isaac 20180109 (Steven) : auto123可前進後退
            Task=1200;
            break;
        case 1100:
            if(Sen[SnAutoIsFull[Pos]].IsOn()==true)
            {
                if((CosFunction.bAutoFullDoNotStopImmediately==true ||                                                                                          //JerryYang 20160810 Unloader滿tray是否需要馬上停機
                    (CosFunction.bUseAutoTrayPreAlarm==true &&                                                                                                  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
                     IniConfig.bP33AutoTrayPreAlarm==true)) &&
                     IniConfig.bP53_ForcedScanBinCodeOfUnloader==false &&                                                                                       //JerryYang 20240111 : 開P53功能要停機
                    bAutoIsFullNeedStop[Pos]==false)
                {
                    iUnLoaderCount=Pos+1;
                    str1.sprintf("Auto%d is full with trays, Please take it off.", Pos+1);
                    str2.sprintf("Auto%d 上的Tray盤已滿,請取下Tray盤", Pos+1);
                    ShowUnloaderTrayMessage(str1, str2);
                    bAutoIsFullNeedStop[Pos]=true;
                    Task=1200;
                }
                else if(CosFunction.bFullTrayAlarmAfterUnloadEnd)                                                                                               //Sam 20210602 : Unload 做完後再 Alarm
                {
                    if(fAGV->Use_AMR() &&                                                                                                                       //Steven 20251216 : for AMR
                       iUnloaderTrayCountCal[Pos]-1<fSCKART->iBundleOutCnt)                                                                                     //JerryYang 20250428 : 這個時候TRAY盤還在軌道上, 數量要減1
                    {
                        str1.sprintf("Please check Auto%d full sensor, now only %d trays but sensor is on! Please check over %d tray then sensor on!", Pos+1, iUnloaderTrayCountCal[Pos]-1, fSCKART->iBundleOutCnt);
                        str2.sprintf("請確認Auto%d 滿tray sensor, 軟體計數有%d盤但sensor已亮! 請調整tray超過%d盤時sesnsor才能亮!", Pos+1, iUnloaderTrayCountCal[Pos]-1, fSCKART->iBundleOutCnt);
                        ShowMyMessage(str1, str2);
                    }
                    else
                    {
                        bAutoIsFullNeedStop[Pos]=true;
                        Task=1200;
                    }
                }
                else
                {
                    if(IniConfig.bEnable_SECS_GEM==true)                                                                                                        //Steven 20140528 : Secs Gem
                    {
                        if(fAGV->IsATK_AMR())                                                                                                                   //Steven 20251210 : ATK AMR
                        {
                            sUnloadBundleID =fSortCT->pnlTrayID[Pos]->Caption;
                            iThisPortNo     =Pos+ePortAuto1;
                            iThisPortStatus =iPortStatus[Pos];

                            int iLen=sSVBinAssign[Pos].Length();                //RogerYang 20260403 remove Commas ","
                            AnsiString sGetCommas=sSVBinAssign[Pos].SubString(iLen, iLen);
                            if(sGetCommas==",")
                                sOutputBinCode=sSVBinAssign[Pos].SubString(0 , iLen-1);
                            else
                            sOutputBinCode  =sSVBinAssign[Pos];
//                            sUnloadBundleID=asBundleTrayID[iThisPortNo];
                        }

                        if(Pos==eAuto1)
                            EventReport(SECS_EVENT.Auto1Full);
                        else if(Pos==eAuto2)
                            EventReport(SECS_EVENT.Auto2Full);
                        else if(Pos==eAuto3)
                            EventReport(SECS_EVENT.Auto3Full);
                        else if(Pos==eAuto4)                                                                                                                    //Steven 20230907 : For HT-9011UC
                            EventReport(SECS_EVENT.Auto4Full);
                        else if(Pos==eAuto5)
                            EventReport(SECS_EVENT.Auto5Full);
                        else if(Pos==eAuto6)
                            EventReport(SECS_EVENT.Auto6Full);

                        EventReport(SECS_EVENT.MaximumOutputPortReport);
                    }

                    if(CosFunction.bAutoTrayFullAlarmCanNotSkip)                                                                                                //Steven 20210219 : Unloader滿盤alarm不能Skip
                    {
                        ret=ShowErrorMessage(sMES1120[Pos], K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_1100");                                     //JerryYang 20160909 矽品要求Auto滿料時不能skip
                    }
                    else
                    {
                        ret=ShowErrorMessage(sMES1120[Pos], K_SKIP|K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_1100");
                    }

                    if(ret==K_SKIP)
                    {
                        Task=1200;
                        bAutoIsFullNeedStop[Pos]=false;
                    }
                    else
                    {
                        Task=1010;                                                                                                                              //JerryYang 20180822 alarm後要重新偵測到位sensor
                    }
                }
            }
            else
            {
                Task=1200;
                bAutoIsFullNeedStop[Pos]=false;
            }
            break;
        case 1200:
            if(bHasICFloating[Pos])                                                                                     //Sam 20250415 : Unloader Tray偵測到有置偏IC退出來再報警。
            {
                str1.sprintf("JAM%d10", 11+Pos);                                                                        //JAM1110, JAM1210, JAM1310
                ret=ShowErrorMessage(str1, K_SKIP, MMAuto1_Car+Pos);
                bHasICFloating[Pos]=false;
                break;
            }

            DetectAutoTime[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayArrivalWaitDelay);                                  //kevin 20190326 add auto 退tray 等待時間
            Task=1210;
            TrayMoveOut(true, Pos+3);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveOut(Pos+3);
            break;
        case 1210:
            if(DetectAutoTime[Pos].Off())
            {
                htSwACAutoOffDelay[Pos].SetMSAndOn(1000);                                                               //Sam 20200721 : 延長時間

                if(LastSet.iRealDummy!=DUMMY)                                                                           //Ifor 20200305 : add 不等於Dummy才判斷
                {
                    if(Sen[SnAutoPreDete[Pos]].IsOff()==false &&
                       Sen[SnAutoTrayCar[Pos]].IsOff()==false)                                                          //Ifor 20200110 add: Auto減速與到位Sensor一起亮才可上升，避免Sensor異常
                    {
                        AutoCylinderMiddle(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);                            //Steven 20140409 : AutoCylinderXX Add Reset
                    }
                    else
                    {
                        TrayMoveOut(false, Pos+3);
                        if(TRAY_ARM_MODE==eUnderCoveyor)
                            TrayCylinMoveIn(Pos+3);
                        str1.sprintf("Auto%d Sensor status abnormal please confirm. 1210", Pos+1);                      //Ifor 20200110 add: Auto減速與到位Sensor一起亮才可上升，避免Sensor異常
                        str2.sprintf("Auto%d 請確認是否Sensor異常 1210", Pos+1);
                        ShowUnloaderTrayMessage(str1, str2);
                        break;
                    }
                }
                else
                {
                    AutoCylinderMiddle(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);                                //Steven 20140409 : AutoCylinderXX Add Reset
                }

                if(bARTUnloaderUseTwoCylin(Pos))                                                                        //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
                {
                    Cylinder[C_Auto_Selector[Pos]].On();
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                                 //kevin 20190527 高雄Auto 123 避免翻TRAY
                        DetectAutoTime[Pos].SetMSAndOn(2000);
                    else
                        DetectAutoTime[Pos].SetMSAndOn(1000);
                    bAlreadyIn[Pos]=false;
                }
                else                                                                                                    //JerryYang 20210721 : fix Auto Z motor蹺蹺板版本
                {
                    if(LOAD_Z_USE_MOTOR[Pos+3]==true)
                    {
                        AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);
                    }
                }

                Task=1220;
                bNeedClearData[Pos]=true;                                                                               //Ifor 20200620 add: 馬達停止後才清除 Tray Data
            }
            break;
        case 1215:                                                              //Sam 20211115 : 確保汽缸上前的狀態皮帶是一直再帶動轉
            if(htSwACAutoOffDelay[Pos].Off())
            {
                Task=1220;
            }
            break;
        case 1220:
            #ifndef SOFT_SIMULTE
            if(CUSTOMER_CODE==CC_KYEC_LEE ||
               CUSTOMER_CODE==CC_TERAPOWER)                                                                             //Sam 20211115 : 確保汽缸上前的狀態皮帶是一直再帶動轉
            {
                if(TrayMoveStatus(Pos+3,__FUNC__)==0  &&
                   LastSet.iRealDummy!=DUMMY)                                                                           //Steven 20130930
                {
                    TrayMoveOut(true, Pos+3);
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                        TrayCylinMoveOut(Pos+3);
                    htSwACAutoOffDelay[Pos].SetMSAndOn(2000);
                    Task=1215;
                    break;
                }
            }
            #endif

            if(htSwACAutoOffDelay[Pos].Off())
            {
                if(CUSTOMER_CODE==CC_KYEC_LEE ||                                                                        //Ifor 20190506 : KYEC 新增 Auto Tray Sensor Off才停止AC 馬達運轉，避免AC馬達停止時被皮帶回拉導致翻盤
                   CUSTOMER_CODE==CC_TERAPOWER)                                                                         //Sam 20211115 : Add TPW
                {
                }
                else
                {
                    TrayMoveOut(false, Pos+3);
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                        TrayCylinMoveIn(Pos+3);

                    if(Cylinder[C_AutoTrackFloodgate[Pos]].Enable)
                        Cylinder[C_AutoTrackFloodgate[Pos]].Off();
                }

                if(bNeedClearData[Pos]==true)                                                                           //Ifor 20200620 add: 馬達停止後才清除 Tray Data
                {
                    //==> Eastsun 20260520 整合//Ifor 20230817 add: 退Tray時有HasNullIC報警提示
                    if(bAQL_Sorting_Finish==false)  //Ifor 20231124 add:避免AQL Mode 誤報空穴問題
                        iAutoHasHod[Pos]=MOT[MMAuto1+Pos].HowManyDevice(HAS_NULL_IC);
                    else
                        bAQL_Sorting_Finish=false;  //Ifor 20231124 add:避免AQL Mode 誤報空穴問題
                    //<== Eastsun 20260520 //Ifor 20230817 add: 退Tray時有HasNullIC報警提示
                    bNeedClearData[Pos]=false;
                    //fFixAICCD->bCheckUnloaderHasAiNG(Pos);                    //Sam 20211220 : 整合 Fix AI AOI
                    fProductionInfo->CalTrayICCount(Pos);
                    MOT[iMMAuto_Car[Pos]].InitNewTray(NULL_IC, false, __FUNC__);
                    MOT[iMMAuto_Car[Pos]].MoveTrayAllItem(&MOT[iMMAuto[Pos]]);                                          //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                }
            }

            if(bARTUnloaderUseTwoCylin(Pos))                                                                            //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
            {
                if(bAlreadyIn[Pos]==false)
                {
                    if(DetectAutoTime[Pos].Off()==true)
                    {
                        Cylinder[C_Auto_Selector[Pos]].Off();
                        bAlreadyIn[Pos]=true;
                    }
                    else
                    {
                        bAlreadyIn[Pos]=false;
                    }
                    break;
                }
            }

            if(bARTUnloaderUseTwoCylin(Pos))                                                                            //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
            {
                if(AutoCylinderMiddle(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))                                       //升到接盤位置
                {
                    TrayMoveOut(false, Pos+3);
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                        TrayCylinMoveIn(Pos+3);

                    if(Cylinder[C_AutoTrackFloodgate[Pos]].Enable)
                        Cylinder[C_AutoTrackFloodgate[Pos]].Off();
                    Task=1350;
                }
            }
            else if(bUseAuto2Empty && Pos==eAuto2)                                                                      //kevin 20120907 AUTO2是分離氣缸  start  //ChungHung 20140317 need add AutoRetest
            {
                if(AutoCylinderMiddle(Pos, C_Auto_Selector[Pos], C_Auto_Up[Pos]))                                       //升到接盤位置
                {
                    TrayMoveOut(false, Pos+3);
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                        TrayCylinMoveIn(Pos+3);

                    if(Cylinder[C_AutoTrackFloodgate[Pos]].Enable)
                        Cylinder[C_AutoTrackFloodgate[Pos]].Off();
                    Task=1350;
                }
            }
            else
            {
                if(LOAD_Z_USE_MOTOR[Pos+3]==true)                                                                       //JerryYang 20210721 : fix Auto Z motor蹺蹺板版本
                {
                    if(AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))                                       //JerryYang 20210721 unloader z motor
                    {
                        TrayMoveOut(false, Pos+3);
                        if(TRAY_ARM_MODE==eUnderCoveyor)
                            TrayCylinMoveIn(Pos+3);

                        if(Cylinder[C_AutoTrackFloodgate[Pos]].Enable)
                            Cylinder[C_AutoTrackFloodgate[Pos]].Off();
                        Task=1350;
                    }
                }
                else
                {
                    if(Cylinder[C_Auto_Selector[Pos]].Push())                                                           //kevin 20120907 AUTO2是分離氣缸 end
                        Task=1350;
                }
            }

            ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
            break;
        case 1350:
            if(ULDStackDelay[Pos].Off())
            {
                if(bNeedClearData[Pos]==true)                                   //Ifor 20200620 add: 馬達停止後才清除 Tray Data
                {
                    bNeedClearData[Pos]=false;
                    //fFixAICCD->bCheckUnloaderHasAiNG(Pos);                    //Sam 20211220 : 整合 Fix AI AOI
                    fProductionInfo->CalTrayICCount(Pos);
                    MOT[iMMAuto_Car[Pos]].InitNewTray(NULL_IC, false, __FUNC__);
                    MOT[iMMAuto_Car[Pos]].MoveTrayAllItem(&MOT[iMMAuto[Pos]]);  //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                             //kevin 20210717 auto finish log
                {
                    str1.sprintf("Auto tray unload finish,Auto %d.",Pos+1);
                    RecordProcess(str1);                                        //kevin 20210604 add Auto tray receive finish
                }

                if(LastSet.iRealDummy==DUMMY && bARTUnloaderUseTwoCylin(Pos))   //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
                {
                    Task=1352;
                }
                else if(LastSet.iRealDummy!=DUMMY &&
                       ((bUseAuto2Empty && Pos==1) ||
                        bARTUnloaderUseTwoCylin(Pos)))                          //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
                {
                    Task=1351;
                }
                else
                {
                    if(LOAD_Z_USE_MOTOR[Pos+3]==true)                           //JerryYang 20210721 : fix Auto Z motor蹺蹺板版本
                    {
                        Task=1352;
                    }
                    else
                    {
                        Task=1400;
                    }
                }
            }
            break;
        case 1351:                                                              //kevin 20120907 AUTO2 分離氣缸
            if(LastSet.iRealDummy==DUMMY ||
              Cylinder[C_AutoZ_Select[Pos]].Push())                             //分離氣缸打開
            {
                if(Sen[SnAutoZSelect1[Pos]].Enable ||
                   Sen[SnAutoZSelect2[Pos]].Enable ||
                   Sen[SnAutoZSelect3[Pos]].Enable ||
                   Sen[SnAutoZSelect4[Pos]].Enable)                             //JerryYang 20200615 ART分離汽缸sensor
                {
                    ULDStackDelay[Pos].SetSecAndOn(5);
                    Task=1352;
                }
                else
                {
                    ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                    if(IniConfig.bSPILFunction==true)                           //JerryYang 20220923 : SPIL ART分離汽缸先on/off一次
                    {
                        Task=13520;
                    }
                    else
                    {
                        Task=1352;
                    }
                }
            }
            break;
        case 13520:
            if(ULDStackDelay[Pos].Off())                                        //JerryYang 20220923 : SPIL ART分離汽缸先on/off一次
            {
                if(LastSet.iRealDummy!=DUMMY)
                {
                    Cylinder[C_AutoZ_Select[Pos]].Off();
                    ULDStackDelay[Pos].SetSecAndOn(1);
                }
                Task=13521;
            }
            break;
        case 13521:
            if(ULDStackDelay[Pos].Off())
            {
                if(LastSet.iRealDummy==DUMMY ||
                   Cylinder[C_AutoZ_Select[Pos]].Push())                        //分離氣缸打開
                {
                    if(Sen[SnAutoZSelect1[Pos]].Enable ||
                       Sen[SnAutoZSelect2[Pos]].Enable ||
                       Sen[SnAutoZSelect3[Pos]].Enable ||
                       Sen[SnAutoZSelect4[Pos]].Enable)                         //JerryYang 20200615 ART分離汽缸sensor
                    {
                        ULDStackDelay[Pos].SetSecAndOn(5);
                    }
                    else
                    {
                        ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                    }
                    Task=1352;
                }
            }
            break;
        case 1352:                                                                                                      //kevin 20120907 AUTO2 分離氣缸
            if(Sen[SnAutoZSelect1[Pos]].Enable ||
               Sen[SnAutoZSelect2[Pos]].Enable ||
               Sen[SnAutoZSelect3[Pos]].Enable ||
               Sen[SnAutoZSelect4[Pos]].Enable)                                                                         //JerryYang 20200615 ART分離汽缸sensor
            {
                if(Sen[SnAutoZSelect1[Pos]].IsOff()==false &&
                   Sen[SnAutoZSelect2[Pos]].IsOff()==false &&
                   Sen[SnAutoZSelect3[Pos]].IsOff()==false &&
                   Sen[SnAutoZSelect4[Pos]].IsOff()==false)
                {
                    bAutoDuplicateErr[Pos]=false;
                    AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);                                    //Steven 20140409 : AutoCylinderXX Add Reset
                    Task=1353;
                }
                else if(ULDStackDelay[Pos].Off())
                {
                    ret=ShowErrorMessage(sJAM1114[Pos], K_RETRY, iMMAuto[Pos], bAutoDuplicateErr[Pos]);                 //JerryYang 20171101 (wei) Retry改成Skip避免誤會
                    bAutoDuplicateErr[Pos]=true;
                    Cylinder[C_AutoZ_Select[Pos]].On();
                    ULDStackDelay[Pos].SetSecAndOn(5);
                }
            }
            else if(ULDStackDelay[Pos].Off())
            {
                AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);                                        //Steven 20140409 : AutoCylinderXX Add Reset
                Task=1353;
            }
            break;
        case 1353:                                                              //升到最高位置
            if(bARTUnloaderUseTwoCylin(Pos))                                    //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
            {
                if(AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))   //Steven 20140330 : add Auto Retest
                {
                    ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                    Task=1354;
                }
            }
            else
            {
                if(LOAD_Z_USE_MOTOR[Pos+3]==true)                               //JerryYang 20210721 : fix Auto Z motor蹺蹺板版本
                {
                    if(AutoCylinderUp(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))
                    {
                        ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                        Task=1354;
                    }
                }
                else
                {
                    if(AutoCylinderUp(Pos, C_Auto_Selector[Pos], C_Auto_Up[Pos]))
                    {
                        ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                        Task=1354;
                    }
                }
            }
            break;
        case 1354:
            if(ULDStackDelay[Pos].Off())
            {
                #ifndef SOFT_SIMULTE
                if(LastSet.iRealDummy!=DUMMY)
                {
                    if(Sen[SnAutoPreDete[Pos]].IsOff()==false || Sen[SnAutoTrayCar[Pos]].IsOff()==false ||
                       (Sen[SnAutoUpSafe[Pos]].Enable==true   && Sen[SnAutoUpSafe[Pos]].IsOff()==false))
                    {
                        str1.sprintf("Auto%d Sensor status abnormal please confirm.1354", Pos+1);                       //Ifor 20200107 add: 上升至最高點判斷是否有Tray再下方
                        str2.sprintf("Auto%d 請確認是否Sensor異常 1354", Pos+1);
                        ShowUnloaderTrayMessage(str1, str2);
                        break;
                    }
                }
                #endif
                Task=1355;
            }
            break;
        case 1355:
            if(Cylinder[C_AutoZ_Select[Pos]].Pop())                             //分離氣缸關閉    //Steven 20140408
            {
                ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                Task=1356;
            }
            break;
        case 1356:
            if(ULDStackDelay[Pos].Off())
            {
                #ifndef SOFT_SIMULTE
                if(LastSet.iRealDummy!=DUMMY)
                {
                    if(Sen[SnAutoPreDete[Pos]].IsOff()==false || Sen[SnAutoTrayCar[Pos]].IsOff()==false ||
                       (Sen[SnAutoUpSafe[Pos]].Enable==true   && Sen[SnAutoUpSafe[Pos]].IsOff()==false))
                    {
                        str1.sprintf("Auto%d Sensor status abnormal please confirm. 1356", Pos+1);                      //Ifor 20200107 add:下降前判斷是否有Tray再下方
                        str2.sprintf("Auto%d 請確認是否Sensor異常 1356", Pos+1);
                        ShowUnloaderTrayMessage(str1, str2);
                        break;
                    }
                }
                #endif
                AutoCylinderLower(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos], true);                                     //Steven 20140409 : AutoCylinderXX Add Reset
                if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                                                     //kevin 20220607 add send load count to eKeep call AGV
                {
                    iStackCount[Pos+3]++;                                                                               //kevin 20220527 add Tray Count 0:load 1:empty 2:Color 3:Auto1 4:Auto2 5:Auto3
                    str1="";
                    str1.sprintf("Auto %d Count:%d",Pos+1,iStackCount[3+Pos]);
                    RecordProcess(str1);
                }
                Task=1357;
                if(IniConfig.bP37bAutoCylinderUP)                                                                       //kevin 20180726 (wei) Auto 123 氣缸常態在上
                    Task=1400;
            }
            break;
        case 1357:
            if(bARTUnloaderUseTwoCylin(Pos))                                                                            //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
            {
                if(AutoCylinderLower(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))                                        //降到最低位置
                {
                    ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                    Task=1358;
                }
            }
            else
            {
                if(LOAD_Z_USE_MOTOR[Pos+3]==true)                                                                       //JerryYang 20210721 : fix Auto Z motor蹺蹺板版本
                {
                    if(AutoCylinderLower(Pos, C_Auto_Up[Pos], C_Auto_Selector[Pos]))                                    //降到最低位置
                    {
                        ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                        Task=1358;
                    }
                }
                else
                {
                    if(AutoCylinderLower(Pos, C_Auto_Selector[Pos], C_Auto_Up[Pos]))                                    //降到最低位置
                    {
                        ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_TrayBackDelay);
                        Task=1358;
                    }
                }
            }
            break;
        case 1358:                                                              //kevin 20120907 AUTO2 分離氣缸
            //==> Eastsun 20260512 F011 整合 (a-side asendic_Auto.cpp L1615-L1648 KYEC AMR; bAMRFullTray flag implicit guard)
            if(ULDStackDelay[Pos].Off())
            {
                if(bAMRFullTray[Pos] && !bAMRFullICBin && iTrayFeed==0) //Eastsun20260123 : 加入bAMRFullICBin與iTrayFeed
                {
                    AnsiString str[3];
                    AnsiString asstr;
                    str[0]="P1:0,P2:0,P3:0,P4:1,P5:0,P6:0";
                    str[1]="P1:0,P2:0,P3:0,P4:0,P5:1,P6:0";
                    str[2]="P1:0,P2:0,P3:0,P4:0,P5:0,P6:1";
                    asstr=AMRUnloadBin(Pos+1);                         //Eastsun20260123 : 算錯修正
                    asSupplementBin=str[Pos];
                    bUnLoaderActionFlag[Pos]=true;
                    iUnloaderTrayCountCal[Pos]=0;
                    TestIF_File.asAMRBinSetting[Pos]=asstr;
                    TestIF_File.iAMRTrayCount[Pos+3]-=iAMRCoverTray;
                    EventReport(SECS_EVENT.AGVSupplement);
                    asSupplementBin="";
                    TestIF_File.iAMRTrayCount[Pos+3]=0;
                    TestIF_File.iAMRDeviceCount[Pos+3]=0;
                    bAMRFullTray[Pos]=false;
                    bNeedCoverTray[Pos]=true;
                }

                if(bAMRFullTray[Pos]) bAMRFullTray[Pos]=false;
                Task=1400;                                              //a-side Task=1390 collapsed (no case 1390 in c-side)
            }
            //<== Eastsun 20260512
            break;
        case 1400:
            TrayMoveOut(false, Pos+3);
            if(TRAY_ARM_MODE==eUnderCoveyor)
                TrayCylinMoveIn(Pos+3);

            if(Sen[SnAutoTrayCar[Pos]].IsOn()==true ||
               (Sen[SnAutoUpSafe[Pos]].Enable==true  &&
                Sen[SnAutoUpSafe[Pos]].IsOn()==true))                                                                   //Ifor 20200110 add:Auto 下降前若有Sensor On 需警報
            {
                ret=ShowErrorMessage(sJAM1107[Pos], K_RETRY, iMMAuto_Car[Pos]);
            }
            else                                                                                                        //Sam 20230523 : 修正FullTrayAlarm不停機功能，停不下來問題。
            {
                Task=1420;
                bNeedClearData[Pos]=true;                                                                               //Steven 20250715 : add
                if(CUSTOMER_CODE!=CC_KYEC_LEE && fAGV->IsSPIL_AMR())                                                    //JerryYang 20250521 : For AMR  //Eastsun 20260512 F011 整合 (case 1400 SPIL 區塊對 KYEC 顯式隔離)
                {
                    if(iUnloaderTrayCountCal[Pos]>=fSCKART->iBundleOutCnt)
                    {
                        if(IniConfig.bEnable_SECS_GEM==true)                                                            //Steven 20140528 : Secs Gem
                        {
                            if(Pos==eAuto1)
                            {
                                EventReport(SECS_EVENT.BundleEnd_Auto1);
                                InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO1);
                            }
                            else if(Pos==eAuto2)
                            {
                                EventReport(SECS_EVENT.BundleEnd_Auto2);
                                InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO2);
                            }
                            else if(Pos==eAuto3)
                            {
                                EventReport(SECS_EVENT.BundleEnd_Auto3);
                                InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO3);
                            }
                            else if(Pos==eAuto4)                                                                        //Steven 20230907 : For HT-9011UC
                            {
                                EventReport(SECS_EVENT.BundleEnd_Auto4);
                            }
                            else if(Pos==eAuto5)
                            {
                                EventReport(SECS_EVENT.BundleEnd_Auto5);
                            }
                            else if(Pos==eAuto6)
                            {
                                EventReport(SECS_EVENT.BundleEnd_Auto6);
                            }
                        }

                        if(iLastPortStatus[ePortAuto1+Pos]!=iPortStatus[ePortAuto1+Pos])                                //JerryYang 20250521 : For AMR
                        {
                            iPortStatus[ePortAuto1+Pos]=eFullBundle;
                            iLastPortStatus[ePortAuto1+Pos]=eFullBundle;
//                            EventReport(SECS_EVENT.Auto1PortStatusChanged+Pos);       //JerryYang 20250529 : do not report
                        }
                        Task=1415;
                    }
                }
            }
            break;
        case 1415:                                                              //JerryYang 20240318
            if(fTrayMapping->DoCoverTrayID(iKeyenceCoverTrayID_AUTO1+Pos, true)==true)
            {
                if(Pos==eAuto1)
                {
                    if(asAutoCoverTrayID1=="ERROR" ||
                       asAutoCoverTrayID1=="Error" ||
                       asAutoCoverTrayID1=="NOREAD" ||
                       asAutoCoverTrayID1.AnsiPos("Pass_")!=0 ||
                       asAutoCoverTrayID1.AnsiPos("Fail_")!=0)                  //JerryYang 20240821 : 避免誤刷bin label, 開頭pass或fail要卡掉
                    {
                        asBundleTrayID[ePortAuto1]="ERROR";
                        InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO1);
                        break;
                    }
                    else
                    {
                        asBundleTrayID[ePortAuto1]=asAutoCoverTrayID1;          //這盤是Cover Tray
                    }
                    sBundleEndInfo=GetBundleInfo(Pos);
                    sUnloadBundleID=asBundleTrayID[ePortAuto1];
                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto1);

                    slDupUnloadBundlID->Clear();
                    slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                    if(slDupUnloadBundlID->Find(sUnloadBundleID, iTest))
                    {
                        ShowMyMessage("Bundle End Bundle ID is duplicated!");
                        break;
                    }
                    else
                    {
                        slDupUnloadBundlID->Add(sUnloadBundleID);
                        slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);
                    }

                    if(IniConfig.bA68_AutoLoadUnload)                           //JerryYang 20250521 : For AMR
                    {
                        bPortIsBusy[ePortAuto1]=false;
                        bAskStopPort[ePortAuto1]=true;                          //JerryYang 20250505
                    }

                    iPortStatus[ePortAuto1+Pos]=eReadyToUnload;
                    iLastPortStatus[ePortAuto1+Pos]=eReadyToUnload;
//                    EventReport(SECS_EVENT.Auto1PortStatusChanged+Pos);       //JerryYang 20250529 : do not report
                }
                else if(Pos==eAuto2)
                {
                    if(asAutoCoverTrayID2=="ERROR" ||
                       asAutoCoverTrayID2=="NOREAD" ||
                       asAutoCoverTrayID2.AnsiPos("Pass_")!=0 ||
                       asAutoCoverTrayID2.AnsiPos("Fail_")!=0)                  //JerryYang 20240821 : 避免誤刷bin label, 開頭pass或fail要卡掉
                    {
                        asBundleTrayID[ePortAuto2]="ERROR";
                        InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO2);
                        break;
                    }
                    else
                    {
                        asBundleTrayID[ePortAuto2]=asAutoCoverTrayID2;          //這盤是Cover Tray
                    }
                    sBundleEndInfo=GetBundleInfo(Pos);
                    sUnloadBundleID=asBundleTrayID[ePortAuto2];
                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto2);

                    slDupUnloadBundlID->Clear();
                    slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                    if(slDupUnloadBundlID->Find(sUnloadBundleID, iTest))
                    {
                        ShowMyMessage("Bundle End Bundle ID is duplicated!");
                        break;
                    }
                    else
                    {
                        slDupUnloadBundlID->Add(sUnloadBundleID);
                        slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);
                    }

                    if(IniConfig.bA68_AutoLoadUnload)                           //JerryYang 20250521 : For AMR
                    {
                        bPortIsBusy[ePortAuto2]=false;
                        bAskStopPort[ePortAuto2]=true;                          //JerryYang 20250505
                    }
                    iPortStatus[ePortAuto1+Pos]=eReadyToUnload;
                    iLastPortStatus[ePortAuto1+Pos]=eReadyToUnload;
//                    EventReport(SECS_EVENT.Auto1PortStatusChanged+Pos);
                }
                else if(Pos==eAuto3)
                {
                    if(asAutoCoverTrayID3=="ERROR" ||
                       asAutoCoverTrayID3=="NOREAD" ||
                       asAutoCoverTrayID3.AnsiPos("Pass_")!=0 ||
                       asAutoCoverTrayID3.AnsiPos("Fail_")!=0)                  //JerryYang 20240821 : 避免誤刷bin label, 開頭pass或fail要卡掉
                    {
                        asBundleTrayID[ePortAuto3]="ERROR";
                        InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO3);
                        break;
                    }
                    else
                    {
                        asBundleTrayID[ePortAuto3]=asAutoCoverTrayID3;          //這盤是Cover Tray
                    }
                    sBundleEndInfo=GetBundleInfo(Pos);
                    sUnloadBundleID=asBundleTrayID[ePortAuto3];
                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto3);

                    slDupUnloadBundlID->Clear();
                    slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                    if(slDupUnloadBundlID->Find(sUnloadBundleID, iTest))
                    {
                        ShowMyMessage("Bundle End Bundle ID is duplicated!");
                        break;
                    }
                    else
                    {
                        slDupUnloadBundlID->Add(sUnloadBundleID);
                        slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);
                    }

                    if(IniConfig.bA68_AutoLoadUnload)                           //JerryYang 20250521 : For AMR
                    {
                        bPortIsBusy[ePortAuto3]=false;
                        bAskStopPort[ePortAuto3]=true;                          //JerryYang 20250505
                    }
                    iPortStatus[ePortAuto1+Pos]=eReadyToUnload;
                    iLastPortStatus[ePortAuto1+Pos]=eReadyToUnload;
//                    EventReport(SECS_EVENT.Auto1PortStatusChanged+Pos);       //JerryYang 20250529 : do not report
                }

                Task=1420;
                bNeedClearData[Pos]=true;
            }
            break;
        case 1420:
            if(IniConfig.bA68_AutoLoadUnload)                                   //JerryYang 20251013 : fix for AMR
            {
                if((Pos==eAuto1 && bAskStopPort[ePortAuto1]==true) ||
                   (Pos==eAuto2 && bAskStopPort[ePortAuto2]==true) ||
                   (Pos==eAuto3 && bAskStopPort[ePortAuto3]==true))
                {
                    break;
                }
            }

            if(CosFunction.bFullTrayAlarmAfterUnloadEnd)                        //Sam 20210602 : Unload 做完後再 Alarm
            {
                if((bAutoIsFullNeedStop[Pos] && Sen[SnAutoIsFull[Pos]].IsOn() &&
                    IniConfig.bA68_AutoLoadUnload==false) ||                    //JerryYang 20251013 : fix for AMR
                   (fAGV->Use_AMR() &&                                          //Steven 20251216 : for AMR
                    iUnloaderTrayCountCal[Pos]>=fSCKART->iBundleOutCnt))        //JerryYang 20240821 : 修正誤ALARM
                {
                    if(IniConfig.bA68_AutoLoadUnload)                           //JerryYang 20251013 : fix for AMR
                    {
                    }
                    else
                    {
                        ShowErrorMessage(sMES1120[Pos], K_RETRY, iMMAuto_Car[Pos], false, "DoAutoReceiveBinTray_1400");
                    }

                    if(Sen[SnAutoIsFull[Pos]].IsOn() ||                         //JerryYang 20250428 : 滿tray sensor跟has tray sensor都要判斷
                       (Sen[SnAutoTrayHasTray[Pos]].Enable==true &&
                        Sen[SnAutoTrayHasTray[Pos]].IsOn()))
                    {
                        str1.sprintf("Please take off the bundle tray of AUTO%d", Pos+1);
                        ShowMyMessage(str1);
                        break;
                    }

                    if(iUnloaderTrayCountCal[Pos]>=fSCKART->iBundleOutCnt)
                    {
                        iUnloaderTrayCountCal[Pos]=0;
                    }
                    break;
                }

                bAutoIsFullNeedStop[Pos]=false;                                 //Sam 20210602 : Unload 做完後再 Alarm
            }

            if(IniConfig.bP22EnableFirstTrayNeedAlarm &&                        //ChungHung 20140521 add for ATK
               bFirstTrayNeedAlarm)
            {
                if(IniConfig.iEnableFirstTrayNeedAlarmNum==Pos)
                {
                    bFirstTrayNeedAlarm=false;
                    str1.sprintf("Auto%d First Tray", Pos+1);                   //Hung : 20140603 : Pos --> Pos+1
                    ShowMyMessage(str1);
                }
            }
            iAuto_CarCarDeviceCnt[Pos]=MOT[iMMAuto_Car[Pos]].HowManyDevice();   //Sam 20250331 : Unloader Auto 退出時上面的 IC 數量。
            CheckHasErrorBinOnTray(iMMAuto_Car[Pos]);                           //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
            fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto_Car[Pos]);                 //Sam 20240325 : 新增 DamageTrayMapping 功能
            if(CosFunction.bNeedAlarmAfterUnloaderFull &&
               MOT[iWhichAuto].sUnloaderAlarmMsg!="")                           //Jimmychiu 20240902 : Need Alarm After Unloader Full
            {
                ShowMyMessage(MOT[iWhichAuto].sUnloaderAlarmMsg);
                MOT[iWhichAuto].sUnloaderAlarmMsg="";
            }

            if(bNeedClearData[Pos]==true)                                       //Ifor 20200620 add: 馬達停止後才清除 Tray Data
            {
                bNeedClearData[Pos]=false;
                if(MOT[iMMAuto_Car[Pos]].fHasTray)                              //Steven 20250715 : 避免瘋狂存檔
                    MOT[iMMAuto_Car[Pos]].ClearTray(__FUNC__);
            }
            PitchCylinderState[Pos]=CY_POP;

            AMR.UnloaderOutCount(Pos);                                          //Sam 20240304 : 新增 AMR 功能

            if(USE_LdUldCassetteMode==1)                                        //Frank 20251217 add
            {
                Task=1500;
            }
            else if((bUseAuto2Empty && Pos==1) ||
               bARTUnloaderUseTwoCylin(Pos))                                    //Sam 20220916 : 整合 ART Unload 上升汽缸判斷式
            {
                PitchCylinderState[Pos]=0;
                Task=1500;
            }
            else if(Cylinder[C_Auto_Selector[Pos]].Pop())
            {
                PitchCylinderState[Pos]=0;
                Task=1500;
            }
            ULDStackDelay[Pos].SetSecAndOn(Ld_UldDelayTime.ULD_LiftDownDelay);
            break;
        case 1500:
            if(CosFunction.bUseAutoTrayPreAlarm==true &&                        //Ifor 20170420 (wei) Fix Auto Tray PreAlarm 時機異常(第一次滿Tray不Alarm)
               IniConfig.bP33AutoTrayPreAlarm==true)
            {
               if((Sen[SnAutoIsFull[Pos]].IsOn()==true ||
                   (Sen[SnAutoIsPreAlarm[Pos]].Enable &&                        //wei 20170802 Pre alarm sensor
                    Sen[SnAutoIsPreAlarm[Pos]].IsOn()==true)) &&
                   bAutoIsFullNeedStop[Pos]==false)
                {
                    iUnLoaderCount=8;
                    str1.sprintf("Auto%d is full with trays, Please take it off.", Pos+1);
                    str2.sprintf("Auto%d 上的Tray盤已滿,請取下Tray盤", Pos+1);
                    RunInfo.iPreAlarmPosition=iPreAlarm[Pos];                   //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color 10:Auto4 11:Auto5 12:Auto6 13:Fix4 14:Fix5 15:Fix6
                    ShowUnloaderTrayMessage(str1, str2);
                    bAutoIsFullNeedStop[Pos]=true;
                }
            }

            bReceiveAutoTray[Pos]=true;
            if(ULDStackDelay[Pos].Off())
            {
                if(IniConfig.bEnable_SECS_GEM==true)
                {
                    if(Pos==eAuto1)
                        EventReport(SECS_EVENT.Auto1Unloadtray);
                    else if(Pos==eAuto2)
                        EventReport(SECS_EVENT.Auto2Unloadtray);
                    else if(Pos==eAuto3)
                        EventReport(SECS_EVENT.Auto3Unloadtray);
                    else if(Pos==eAuto4)                                        //Steven 20230907 : For HT-9011UC
                        EventReport(SECS_EVENT.Auto4Unloadtray);
                    else if(Pos==eAuto5)
                        EventReport(SECS_EVENT.Auto5Unloadtray);
                    else if(Pos==eAuto6)
                        EventReport(SECS_EVENT.Auto6Unloadtray);
                }

                if(CosFunction.bFirstTrayCheckOnUnloader==true &&               //Jimmychiu 20251205 : First Tray Check On Unloader
                   IniConfig.bP62FirstTrayCheckOnUnloader==true &&
                   fLotInfo->cbFirstTrayCheckOnUnloader->Checked==true)
                {
                    if((Pos==eAuto1 && IniConfig.bP62Auto1==true) ||
                       (Pos==eAuto2 && IniConfig.bP62Auto2==true) ||
                       (Pos==eAuto3 && IniConfig.bP62Auto3==true))
                    {
                        str1.sprintf("Auto%d First tray check after change kit \r\n Please call EE on-site routine check", Pos+1);
                        ShowMyMessage(str1);
                    }
                }
                iAuto_CarCarDeviceCnt[Pos]=0;                                   //Sam20250331 : Unloader Auto 退出時上面的 IC 數量。

                if(bEject)                                                      //JerryYang 20251020 : 渠梁半清機功能
                {
                    bNeedEject[Pos]=false;
                    CheckAllAutoTrayEjectFinsh();
                }
                Task=300;
            }
            break;
        case 1550:                                                              //Sam 20221007 : 電磁閥 On 沒作用，重新 Off 延遲一下再 On
            if(AutoReceiveBinTrayDelay[Pos].Off())
                Task=1600;
            break;
        case 1600:                                                                                                                                              //Auto 偵測有Tray
            if(LastSet.iRealDummy==DUMMY ||
               Sen[SnAutoTrayDetect[Pos]].IsOff()==false)                                                                                                       // has tray
            {
                if(USE_COLOR_TRAY_SENSOR==1 &&
                   TrayForm.bColorTray &&
                   LastSet.iRealDummy!=DUMMY &&
                   Sen[SnAutoColorTrayDete[Pos]].Enable)                                                                                                        //20140903 wei colcr Tray
                {
                    if(CUSTOMER_CODE==CC_TERAPOWER)                                                                                                             //Sam 20180525 (wei) : 晶兆成 Auto1~3 Color sensor detect By FromEmptyColor
                    {
                        bColorFlag=ForTERAPOWERCheckColorSensor(Pos);
                        if(bColorFlag==true)
                        {
                            asColorSensorAlarm="1600";
                            ColorSensorDelayTime[Pos].SetSecAndOn(60);                                                                                          //Sam 20211028 : 增加保護防止 Hange up
                            Task=1650;                                                                                                                          //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。
                            break;
                        }
                    }
                    else
                    {
                        bColorFlag=Sen[SnAutoColorTrayDete[Pos]].IsOff();
                        if(bColorFlag==true)                                                                                                                    //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。 //Steven 20150427 : For Color Sensor, 滅掉是紅色
                        {
                            ret=ShowErrorMessage(sWAR1151[Pos], K_RETRY, iMMAuto[Pos], false, "DoAutoReceiveBinTray_1600");                                     //WAR1151, WAR1251, WAR1351
                            break;
                        }
                    }
                }

                Cylinder[C_AutoSide_Fixer[Pos]].On();                                                                                                           //Lock Tray
                Cylinder[C_AutoEdgePush[Pos]].On();
                Cylinder[C_AutoUpPress[Pos]].On();                                                                                                              //JerryYang 20190423 新增unloader壓tray
                Task=1700;
            }
            else
            {
                Cylinder[C_AutoSide_Fixer[Pos]].On();                                                                                                           //Auto 偵測沒有Tray Lock
                Cylinder[C_AutoEdgePush[Pos]].On();
                Cylinder[C_AutoUpPress[Pos]].On();                                                                                                              //JerryYang 20190423 新增unloader壓tray
                DetectAutoTime[Pos].SetMSAndOn(1500);                                                                                                           //等待1.5s
                Task=1620;
            }
            break;
        case 1620:
            if(LastSet.iRealDummy==DUMMY ||
               Sen[SnAutoTrayDetect[Pos]].IsOff()==false)                                                                                                       // has tray
            {
                if(USE_COLOR_TRAY_SENSOR==1 &&
                   TrayForm.bColorTray &&
                   LastSet.iRealDummy!=DUMMY &&
                   Sen[SnAutoColorTrayDete[Pos]].Enable)                                                                                                        //20140903 wei colcr Tray
                {
                    if(CUSTOMER_CODE==CC_TERAPOWER)                                                                                                             //Sam 20180525 (wei) : 晶兆成 Auto1~3 Color sensor detect By FromEmptyColor
                    {
                        bColorFlag=ForTERAPOWERCheckColorSensor(Pos);
                        if(bColorFlag==true)
                        {
                            asColorSensorAlarm="1620";
                            ColorSensorDelayTime[Pos].SetSecAndOn(60);                                                                                          //Sam 20211028 : 增加保護防止 Hange up
                            Task=1650;                                                                                                                          //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。
                            break;
                        }
                    }
                    else
                    {
                        bColorFlag=Sen[SnAutoColorTrayDete[Pos]].IsOff();
                        if(bColorFlag==true)                                                                                                                    //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。//Steven 20150427 : For Color Sensor, 滅掉是紅色
                        {
                            ret=ShowErrorMessage(sWAR1151[Pos], K_RETRY, iMMAuto[Pos], false, "DoAutoReceiveBinTray_1620");                                     //WAR1151, WAR1251, WAR1351
                            break;
                        }
                    }
                }

                Cylinder[C_AutoSide_Fixer[Pos]].On();                                                                                                           //Auto 偵測到有Tray
                Cylinder[C_AutoEdgePush[Pos]].On();
                Cylinder[C_AutoUpPress[Pos]].On();                                                                                                              //JerryYang 20190423 新增unloader壓tray
                Task=1700;
            }
            else
            {                                                                                                                                                   //Auto 偵測沒有Tray
                if(MOT[MTrayX].ReadPos()>=Prod.iXTrayAuto[0])
                    break;

                if(DetectAutoTime[Pos].Off())
                {
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();
                    Cylinder[C_AutoEdgePush[Pos]].Off();
                    Cylinder[C_AutoUpPress[Pos]].Off();                                                                                                         //JerryYang 20190423 新增unloader壓tray
                    ret=ShowErrorMessage(sJAM1108[Pos], K_SKIP|K_RETRY, iMMAuto[Pos]);
                    if(ret==K_SKIP)
                    {
                        CheckHasErrorBinOnTray(iMMAuto[Pos]);                                                                                                   //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
                        fFixAICCD->bCheckUnloaderHasAiNG(iMMAuto[Pos]);                                                                                         //Sam 20240325 : 新增 DamageTrayMapping 功能
                        fProductionInfo->CalTrayICCount(Pos);
                        MOT[iMMAuto[Pos]].ClearTray(__FUNC__);
                        Task=1;
                    }
                }
            }
            break;
        case 1650:                                                                                                                                              //Sam 20180704 : color sensor 偵測到異常時，等 Tray Arm 離開後並 unlock tray 再報警。
            if(MOT[MTrayX].ReadPos()>Prod.iXTrayAuto[0]-10000 &&
               ColorSensorDelayTime[Pos].Off()==false)                                                                                                          //Sam 20211028 : 增加保護防止 Hange up
                break;

            Cylinder[C_AutoSide_Fixer[Pos]].Off();                                                                                                              //Unlock Tray
            Cylinder[C_AutoEdgePush[Pos]].Off();
            Cylinder[C_AutoUpPress[Pos]].Off();                                                                                                                 //JerryYang 20190423 新增unloader壓tray

            ret=ShowErrorMessage(sWAR1151[Pos], K_RETRY, iMMAuto[Pos], false, "DoAutoReceiveBinTray_1650_"+asColorSensorAlarm);                                 //WAR1151, WAR1251, WAR1351

            Cylinder[C_AutoSide_Fixer[Pos]].On();
            Cylinder[C_AutoEdgePush[Pos]].On();
            Cylinder[C_AutoUpPress[Pos]].On();                                                                                                                  //JerryYang 20190423 新增unloader壓tray
            Task=1700;
            break;
        case 1700:
            Cylinder[C_AutoEdgePush[Pos]].On();
            if(Cylinder[C_AutoSide_Fixer[Pos]].Push())
            {
                Cylinder[C_AutoUpPress[Pos]].On();                              //JerryYang 20190423 新增unloader壓tray
                AutoReceiveBinTrayDelay[Pos].SetSecAndOn(3.0);                  //Sam 20220718 : 增加 Delay Time 避免誤報警
                Task=1710;
            }
            break;
        case 1710:                                                              //kevin 20170413 (wei) add 汽缸沒動作需發ALARM
            #ifdef SOFT_SIMULTE
                Task=1;
                if(IniConfig.bP42AlarmWhenExitTrayComplete==true &&
                   bReceiveAutoTray[Pos]==true)                                 //Ifor 20240525 add: 客戶要求Auto每盤退出後要Alarm 停機
                {
                    str1.sprintf("Auto%d Tray has finished exiting.", Pos+1);
                    str2.sprintf("Auto%d Tray 已完成退出", Pos+1);
                    ShowMyMessage(str1,str2);
                }
            #else
            if(LastSet.iRealDummy!=DUMMY &&                                     //Steven 20220523 : Dummy mode 軌道氣缸不Alarm
               Cylinder[C_AutoSide_Fixer[Pos]].OffSensor())                     //kevin 20170413 cylinder  error
            {
                Cylinder[C_AutoSide_Fixer[Pos]].On();                           //Sam 20220816 : 增加保護避免誤報警
                if(AutoReceiveBinTrayDelay[Pos].Off())                          //Sam 20220718 : 增加 Delay Time 避免誤報警
                {
                    if(CosFunction.bAutoSideFixerCheckAlarmAutoRetry)           //Sam 20220929 : 增加 AutoSideFixer Check Alarm Auto Retry
                        iSenRetry[Pos]++;
                    if(CosFunction.bAutoSideFixerCheckAlarmAutoRetry==false ||
                       iSenRetry[Pos]>=10)                                      //Sam 20220929 : 增加 AutoSideFixer Check Alarm Auto Retry
                    {
                        ShowErrorMessage(sJAM1103[Pos], K_RETRY, iMMAuto[Pos], bDupError[Pos], __FUNC__);
                        Task=1600;
                    }
                    else                                                        //Sam 20221007 : 電磁閥 On 沒作用，重新 Off 延遲一下再 On
                    {
                        AutoReceiveBinTrayDelay[Pos].SetSecAndOn(5.0);
                        Task=1550;
                    }
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();
                    Cylinder[C_AutoEdgePush[Pos]].Off();
                    Cylinder[C_AutoUpPress[Pos]].Off();                         //JerryYang 20190423 新增unloader壓tray
                    bDupError[Pos]=true;
                }
                break;
            }

            if(IniConfig.bP42AlarmWhenExitTrayComplete==true &&
               bReceiveAutoTray[Pos]==true)                                     //Ifor 20240525 add: 客戶要求Auto每盤退出後要Alarm 停機
            {
                str1.sprintf("Auto%d Tray has finished exiting.", Pos+1);
                str2.sprintf("Auto%d Tray 已完成退出", Pos+1);
                ShowMyMessage(str1, str2);
            }

            if(CosFunction.bAutoSideFixerCheckAlarmAutoRetry &&
               iSenRetry[Pos]>0)                                                //Sam 20220929 : 增加 AutoSideFixer Check Alarm Auto Retry
            {
                s.sprintf("C_Auto%d_Side_Fixer_OffSensor() check retry count=%d", Pos+1, iSenRetry[Pos]);
                NewRecordProcess("Motion", s, "DoAutoReceiveBinTray()");
                iSenRetry[Pos]=0;
            }
            #endif
            if(IniConfig.bVTESTFunction==true)                                  //jou 20230207 : VTEST 首盤請執行送檢，通知QA確認
            {
                Task=1720;
                break;
            }

            bReceiveAutoTray[Pos]=false;
            Task=1;
            break;
        case 1720:
            if(bArm1NeedSuck==true || bArm2NeedSuck==true)
                break;

            if(bReceiveAutoTray[Pos]==true)
            {
                if(IniConfig.bVTESTFunction==true)
                {
                    if(BinSelect[iTestRunMode].iStackDefFailCate[Pos]==0)
                    {
                        if(bOutarmDropError==true)                              //jou 20241024 : out arm drop error show alarm提醒檢查出Tray是否疊料
                        {
                            ShowMyMessage("pass bin auto/fix區域有丟料警報,料盤需要檢查物料是否疊料","needs to be checked to see if the IC is stacked.");
                            bOutarmDropError=false;
                        }

                        if(fMesSystem->bFirstMaterialsQA==true)
                        {
                            fMesSystem->bFirstMaterialsQA=false;
                            ShowErrorMessage("WAR16124", K_RETRY, MMAuto1+Pos, false, __FUNC__);
                        }
                    }
                }

                if(CUSTOMER_CODE==CC_VTEST_Shanghai)
                {
                    LastSet.RecodeTrayCount[Pos]++;

                    fMain->lblAuto1TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto1];
                    fMain->lblAuto2TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto2];
                    fMain->lblAuto3TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto3];
                    fMain->lblAuto4TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto4];
                    fMain->lblAuto5TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto5];
                    fMain->lblAuto6TrayCnt->Caption=LastSet.RecodeTrayCount[eAuto6];

                    if(LastSet.RecodeTrayCount[Pos]>=TrayForm.iUnloadTrayCount)
                    {
                        LastSet.RecodeTrayCount[Pos]=0;

                        str1.sprintf("Auto%d is full with trays, Please take it off.", Pos+1);
                        str2.sprintf("Auto%d 上的Tray盤已滿,請取下Tray盤", Pos+1);
                        ShowMyMessage(str1,str2);
                    }
                }
            }

            bReceiveAutoTray[Pos]=false;
            Task=1;
            break;
    }
}
int iAutoTrackDetectICFloatingTask[MAX_AUTO_TRAY]={1, 1, 1, 1, 1, 1};
//------------------------------------------------------------------------------
void InitAutoTrackDetectICFloatingTask(int Pos)
{
    iAutoTrackDetectICFloatingTask[Pos]=1;
}
//------------------------------------------------------------------------------
bool DoAutoTrackDetectICFloating(int Pos)                                       //Isaac 20180109 (Steven) : auto123可前進後退
{
    int &Task=iAutoTrackDetectICFloatingTask[Pos];
    AnsiString str1;
    switch(Task)
    {
        case 1:
            InitAutoTrackDetectICFloatingTask(Pos);
            Task=100;
        case 100:
            if(Sen[SnAutoTrackDetect[Pos]].IsOn() &&                            // detect IC under Tray placed was smooth
               IniConfig.bP50DisabledAutoTrackSensorDetect==false)              //Sam 20230221 : 矽格中興國桂要求要能關閉
            {
                TrayMoveOut(false, Pos+3);
                Task=1;
                return true;                                                    //fail, auto tray go rear and showalarm
            }
            break;
    }
    return false;                                                               //pass
}

//------------------------------------------------------------------------------
int iAuto123TrayToRearTask[MAX_AUTO_TRAY]={1, 1, 1, 1, 1, 1};
TQPF_Timer hAuto123TrayToRear[MAX_AUTO_TRAY];
TQPF_Timer hAuto123TrayToRearForDummy[MAX_AUTO_TRAY];
//------------------------------------------------------------------------------
void InitAuto123TrayToRearTask(int Pos)
{
    iAuto123TrayToRearTask[Pos]=1;
}
//---- 收空TRAY-----------------------------------------------------------------
TQPF_Timer tAuto123TrayToRearTimer[MAX_AUTO_TRAY];
bool DoAuto123TrayToRear(int Pos)
{
    static bool bAutoHasDuplicateError[MAX_AUTO_TRAY]={false, false, false, false, false, false};                       //Steven 20120220 : Empty Tray重複Alarm Flag

    int &Task       =iAuto123TrayToRearTask[Pos];
    int ret         =0;
    bool bFlag      =false;
    AnsiString Str  ="";

    switch(Task)
    {
        case 1:
            hAuto123TrayToRearForDummy[Pos].SetMSAndOn(5000);
            hAuto123TrayToRear[Pos].SetMSAndOn(20000);
            Cylinder[C_AutoEdgePush[Pos]].Off();
            Cylinder[C_AutoUpPress[Pos]].Off();                                 //JerryYang 20190423 新增unloader壓tray
            Task=100;
        case 100:
            if(Cylinder[C_AutoSide_Fixer[Pos]].Pop())                           //ChungHung 20140625 add
            {
                Task=200;
            }
            break;
        case 200:
            if(bHandlerPause)                                                                                                                                   //Steven 20190123 : 紀錄Handler被暫停, 重置Timer
            {
                Task=1;
                break;
            }

            if(MOT[iMMAuto[Pos]].fHasTray && MOT[iMMAuto_Car[Pos]].fHasTray)                                                                                    //Steven 20130917 : Double Tray問題,測試 Q4
            {
                Task=1;
                return true;
            }

            TrayMoveIn(true, Pos+3);
            if(Sen[SnAutoTrayDetect[Pos]].IsOff()==false &&
               Sen[SnAutoCWPreDetect[Pos]].IsOff()==false)                                                                                                      //Sam 20220919 : 增加保護
            {
                Task=450;
                break;
            }

            if(hAuto123TrayToRearForDummy[Pos].Off())
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    Task=450;
                    break;
                }
            }

            if(hAuto123TrayToRear[Pos].Off())
            {
                ret=ShowErrorMessage(sJAM1101[Pos], K_SKIP|K_RETRY, iMMAuto_Car[Pos], bAutoHasDuplicateError[Pos], "DoAutoTrayToRear_200");                     //Steven 20150108 : JAM1001 -> JAM1101, JAM1201, JAM1301
                if(ret==K_RETRY)
                {
                    bAutoHasDuplicateError[Pos]=true;                                                                                                           //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=1;
                }
                else
                {
                    fProductionInfo->CalTrayICCount(Pos);
                    bAutoHasDuplicateError[Pos]=false;                                                                                                          //Steven 20120220 : Empty Tray重複Alarm Flag
                    MOT[iMMAuto_Car[Pos]].ClearTray(__FUNC__);
                    Task=1;
                    return true;
                }
            }
            break;
        case 300:
            if(Cylinder[C_AutoSide_Fixer[Pos]].Push())
            {
                Cylinder[C_AutoUpPress[Pos]].On();                              //JerryYang 20190423 新增unloader壓tray
                Task=400;
            }
            break;
        case 400:
            #ifndef SOFT_SIMULTE
                bFlag=(Sen[SnAutoFixCyPush[Pos]].IsOn() ||
                       Cylinder[C_AutoSide_Fixer[Pos]].OffSensor());                                                                                            //wei 20241011 新增後勾氣缸off sensor,避免氣缸沒作動
            #else
                bFlag=false;
            #endif

            if(bFlag)
            {
                if(LastSet.iRealDummy==DUMMY)
                {
                    bAutoHasDuplicateError[Pos]=false;                                                                                                          //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=450;
                }
                else
                {
                    Cylinder[C_AutoSide_Fixer[Pos]].Off();
                    Cylinder[C_AutoUpPress[Pos]].Off();                                                                                                         //JerryYang 20190423 新增unloader壓tray
                    ret=ShowErrorMessage(sJAM1103[Pos], K_RETRY, iMMAuto[Pos], bAutoHasDuplicateError[Pos], "DoAutoTrayToRear_400");                            //Steven 20150108 : JAM1003 -> JAM1103, JAM1203, JAM1303
                    bAutoHasDuplicateError[Pos]=false;                                                                                                          //Steven 20120220 : Empty Tray重複Alarm Flag
                    Task=300;
                    break;
                }
            }
            else
            {
                bAutoHasDuplicateError[Pos]=false;                                                                                                              //Steven 20120220 : Empty Tray重複Alarm Flag
                Task=500;
            }
            break;
        case 450:
            Cylinder[C_AutoEdgePush[Pos]].Off();
            hAuto123TrayToRear[Pos].SetSecAndOn(Ld_UldDelayTime.LD_TrayArrivalDely);
            Task=460;
            break;
        case 460:
            if(hAuto123TrayToRear[Pos].Off())
            {
                TrayMoveIn(false, Pos+3);
                Task=500;
            }
            break;
        case 500:
            if(Cylinder[C_AutoSide_Fixer[Pos]].Push())
            {
                Task=600;
            }
            break;
        case 600:
            if(Cylinder[C_AutoSide_Fixer[Pos]].Pop())
            {
                Task=1;
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
void CheckHasErrorBinOnTray(int iPos)                                           //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin
{
    if(IniConfig.P54UnloaderTaryCheckHasErrorBinIC)
    {
        AnsiString str1="", asString="";
        AnsiString sAuto_CarName[MAX_AUTO_TRAY]={"Auto1_Car", "Auto2_Car", "Auto3_Car", "Auto4_Car", "Auto5_Car", "Auto6_Car"};

        for(int i=0; i<eTrayCount; i++)                                         //Sam 20240423 : 修正 9011 P54 功能。
        {
            if(iPos==iMMAuto[i] && asString=="")
            {
                asString=s6TrayName[i];
            }
        }

        if(asString=="")
        {
            for(int i=0; i<MAX_AUTO_TRAY; i++)
            {
                 if(iPos==iMMAuto_Car[i])
                 {
                    asString=sAuto_CarName[i];
                 }
            }
        }

        if(asString!="")
        {
            int iErrorIC=MOT[iPos].Tray.HowManyBinICInTray(TEST_PASS+iTestBinCount);
            if(iErrorIC>0)
            {
                str1.sprintf("Unloader %s has error bin %d ic. please check.", asString, iErrorIC);
                ShowMyMessage(str1,"");
                EditTray(iPos,3);
            }
        }
        else
        {
            ShowMyMessage("Unloader not support [P54] Function", "");
        }
    }
}
//------------------------------------------------------------------------------
int iDoAutoCassetteTrayFeed[2]={1, 1};                                          //Frank 20251217 add
TQPF_Timer hAutoCassetteTrayFeed;
void InitAutoCassetteTrayFeedTask(int iPos)
{
    iDoAutoCassetteTrayFeed[iPos]=1;
}
//------------------------------------------------------------------------------
bool DoAutoCassetteTrayFeed(int iPos)                                           //Frank 20251217 add
{
    if(iPos>1)
        return false;
    int &Task=iDoAutoCassetteTrayFeed[iPos];
    static bool bYCCWPushMove[2]={false, false};
    static bool bYCWPushMove[2]={false, false};
    switch(Task)
    {
        case 1:
            if(Sen[SnAutoPreDete[iPos]].IsOff()==false ||                       //Tray盤剛剛始退
                LastSet.iRealDummy==DUMMY)
            {
                bYCCWPushMove[iPos]=false;
                bYCWPushMove[iPos]=false;
                hAutoCassetteTrayFeed.SetSecAndOn(2);
                Task=100;
            }
            break;
        case 100:
            if(hAutoCassetteTrayFeed.Off() && Sen[SnAutoPreDete[iPos]].IsOn()==false)                                   //Tray整盤退出Sensor外
            {
                Task=150;
            }
            break;
        case 150:
                //Chech Loader推TrayToCassette 馬達位置
            bYCCWPushMove[iPos]=MOT[MAuto1Y_CCW+iPos].MotorMove(Prod.iAutoCassetteRearBack[iPos]);
            bYCWPushMove[iPos]=MOT[MAuto1Y+iPos].MotorMove(Prod.iAutoCassetteFrontBack[iPos]);
            if(bYCCWPushMove[iPos]&&bYCWPushMove[iPos])
            {
                bYCCWPushMove[iPos]=false;
                bYCWPushMove[iPos]=false;
                Task=200;
            }
            break;
        case 200:
            if(Cylinder[C_AutoCarrier[iPos]].Push())
            {
                Task=300;
            }
            break;
        case 300:
            bYCCWPushMove[iPos]=MOT[MAuto1Y_CCW+iPos].MotorMove(Prod.iAutoCassetteRear[iPos]);
            if(bYCCWPushMove[iPos])
            {
                bYCCWPushMove[iPos]=false;
                hAutoCassetteTrayFeed.SetSecAndOn(15);
                Task=400;
            }
            break;
        case 400:
            bYCCWPushMove[iPos]=MOT[MAuto1Y_CCW+iPos].MotorMove(Prod.iAutoCassetteRearBack[iPos]);
            if(bYCCWPushMove[iPos]==true)
            {
                bYCCWPushMove[iPos]=false;
                Cylinder[C_AutoCarrier[iPos]].Off();                            //回到原本位置後下降
                Task=500;
            }
            break;
        case 500:
            if(Cylinder[C_AutoCarrier[iPos]].Pop())
            {
                return true;
            }
            else if(hAutoCassetteTrayFeed.Off())
            {
                ShowErrorMessage(sJAM1108[iPos], K_SKIP|K_RETRY, iMMAuto[iPos]);
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
