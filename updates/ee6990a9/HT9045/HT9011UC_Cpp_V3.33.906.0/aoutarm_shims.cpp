// =============================================================================
//  aoutarm_shims.cpp  --  W6.2c-OUT cross-module / engine OFFLINE shims
//
//  Integrate-phase home (added ONCE) for the out-arm ENGINE free-functions and
//  globals that the FIRST batch of 6 translated out-arm site variants
//  (aoutarm9045_{1x1_1,1x2_2,1x2_4,1x3_2_14,1x3_4,1x4_2}.cpp) call but whose real
//  bodies live in golden modules not yet translated in this tree
//  (aoutarm.cpp / aoutarm9045.cpp engine helpers, Magazine.cpp, aRotateKIT_Out.cpp,
//  AutoTeach.cpp, FixAICCD.cpp, cSortCT.cpp, ...).
//
//  Each variant TU forward-declares these behind its own #ifndef guard (verbatim
//  golden signatures); here we DEFINE them ONCE, non-static, offline-safe
//  (return false/0/no-op/sane default), so an executable linking ht9045_sm
//  resolves them.  Faithful to the golden OFFLINE end-state -- no HW touched.
//  TODO(W7): replace with the real engine bodies once aoutarm.cpp / aoutarm9045.cpp
//  engine helpers + Magazine / Rotate / AutoTeach / FixAICCD land.
//
//  Translator: AI(ht9045-v899) 20260627
//  Big5 source comments preserved as UTF-8; ZERO U+FFFD.
// =============================================================================
#include "MachineType.h"            // MAX_ARM_Row / MAX_ARM_Col
#include "vclcompat/vcl_compat.h"   // AnsiString (for OutArmPickShuttleAlarm signature)

//==============================================================================
//  Engine SM cursors / flags (golden aoutarm9045.cpp / aoutarm.h globals).
//  Offline: zero-initialized; the SMs read/advance them.  bOutSuckShtDupErr is
//  the per-nozzle duplicate-error grid (Steven 20110216).
//==============================================================================
// AI(W906-PT-W4-integrate) 20260809: 32 STAND-IN DEFINITION(S) RETIRED FROM HERE.
//   aoutarm.cpp (golden's own home for all of them) landed in wave PT-W4 and is
//   registered in ht9045_sm, so both definitions were in libht9045_sm.a and every
//   executable linking it failed with `multiple definition of ...`. The linker named
//   each one, which is also the proof the signatures match exactly -- a decorated-name
//   collision cannot happen otherwise.
//   Retired here: iPickFromShuttle1Task, iPickFromShuttle2Task, OutArmTask, bOutArmManualStepPress, TransferOutShuttleRatio, PickFromShuttle, bOutShtwaitPick, ShowOutputShuttleDataMiss, SwapShuttleDataToOutArm, OutArmPickShuttleAlarm, CheckTesterZ, CheckShuttleICPos, CheckDuplicateBarCode, SetOutArmNeedDestory, InitPickFromShuttle1Task, InitPickFromShuttle2Task, InitPlaceToAutoTask, OutArmAddSpeed, OutArmSubSpeed, OutArmAddSpeedDisplay, OutArmSubSpeedDisplay, CheckOutArmInitState, CheckOutArmCleanOut, IsCatchTrayReadySupplyNewTray, SetFixTrayFullIC, SetOutArmHome, iPlaceToSortShtTask, InitPlaceToSortShtTask, AutoCalculateOutArmXClosePitch, AutoCalculateOutArmYClosePitch, dOutArmXPitch_1Step, iOutArmYPosition
//   BEHAVIOUR: these were offline defaults (return true/false/0/no-op); the real bodies
//   run golden's actual logic, so out-arm paths that used to short-circuit now execute.
//   That is the point of the wave, and it is why this wave was measured on its own.
int  iOutShtRetryCount     = 0;                                                // golden aoutarm9045.cpp -- offline retry counter
int  iOutArmZTeachTask     = 0;                                                // golden AutoTeach.h:235 -- offline cursor
bool bOutSuckShtDupErr[MAX_ARM_Row][MAX_ARM_Col] = {{false,false,false,false},{false,false,false,false}}; // golden aoutarm9045.cpp (Steven 20110216)

//==============================================================================
//  Shuttle offset / ratio geometry helpers (golden aoutarm9045.cpp engine).
//  Offline: no offset/ratio configured -> identity / zero.
//==============================================================================
#if 0   // PT-W8 RETIRED (GetOutArmToShuttleOffset_9045): real translated body now live
int  GetOutArmToShuttleOffset_9045(int, int, int, bool) { return 0; }          // golden aoutarm9045.cpp:1211 -- offline: 0
#endif
#if 0   // PT-W8 RETIRED (GetOutArmYToShuttleOffset_9045): real translated body now live
int  GetOutArmYToShuttleOffset_9045(int, int)           { return 0; }          // golden aoutarm9045.cpp -- offline: 0
#endif
#if 0   // PT-W8 RETIRED (GetOutArmXToShuttleOffset_9045): real translated body now live
int  GetOutArmXToShuttleOffset_9045(int, int)           { return 0; }          // golden aoutarm9045.cpp -- offline: 0
#endif
#if 0   // PT-W8 RETIRED (GetVariableYOutShuttleData): real translated body now live
int  GetVariableYOutShuttleData()                       { return 0; }          // golden aoutarm9045.cpp (ChungHung 20131231) -- offline: 0
#endif

//==============================================================================
//  Shuttle pick / data-swap / alarm surface (golden aoutarm.cpp / aoutarm9045.cpp)
//==============================================================================
// (W6.2c-OUT batch-3) The temp MoveOutArmToShuttleIncludeZ_9045_2x4_4 stub that lived
// here is REMOVED -- aoutarm9045_2x4_4.cpp is now translated/added live and DEFINES the
// real symbol (golden aoutarm9045_2x4_4.cpp:486); keeping the stub = duplicate symbol.

// SetOutArmNeedDestory: 4-arg (golden aoutarm9045.cpp) + 5-arg (RogerYang 20250516
// 9046AU, default bPlace).  Both overloads referenced across the variant set.
// AI(W906-ARM1) 20260927: RETIRED 4-arg stub SetOutArmNeedDestory —— golden 只有 aoutarm.cpp 的 5 參數那支（bPlace=false），variant 的原型已改（NB2 R105 ③）

//==============================================================================
//  Out-arm task init / speed / state predicates (golden aoutarm9045.cpp / aoutarm.h)
//==============================================================================
#if 0   // PT-W8 RETIRED (CheckOutArmToTask50): real translated body now live
bool CheckOutArmToTask50(int)                      { return true; }            // golden aoutarm9045.h:49 -- offline: reached task50
#endif
#if 0   // PT-W5c RETIRED (IsOutArmCleanOutFinish)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool IsOutArmCleanOutFinish()                      { return true; }            // golden aoutarm9045.cpp -- offline: clean-out finished
#endif

// CheckOutArmCleanOut: variants split between the 0-arg (golden aoutarm.h:58) and
// the (int Task=50) overload -> define BOTH non-static.  Offline: stay/continue
// the normal pick flow (return the no-clean sentinel 1140 / pass-through Task).
// AI(W906-ARM1) 20260927: RETIRED 0-arg stub CheckOutArmCleanOut —— golden 只有 aoutarm.cpp 的 (int Task=50) 那支，variant 的原型已改（NB2 R105 ⑦）

//==============================================================================
//  Tray / Magazine / Rotate / catch-tray cross-module surface
//==============================================================================
// AI(W906-HOME-C2) 20260920: `CheckRotateOutNotFinish()` 的假樁已刪 ——
//   真本體在 RotateKit/aRotateKIT_Out.cpp:4522，而那個檔的檔頭（:150）
//   本來就寫著「that shim line MUST be deleted in the same commit」。
//   在 uhome.cpp 進樹之前沒有 TU 同時逼出這兩個 .obj，所以重複符號一直潛伏。
//   行為：從「恆回 false（rotate 已完成）」變成真的問 rotate 狀態。
#if 0   // PT-W8 RETIRED (DoFixTrayFullAlarm): real translated body now live
bool DoFixTrayFullAlarm()                          { return true; }            // golden aoutarm9045.h:47 -- offline: alarm handled
#endif
#if 0   // PT-W8 RETIRED (VerifyTrayStatus): real translated body now live
int  VerifyTrayStatus()                            { return 3300; }            // golden aoutarm9045.h:39 -- offline: 3300 (proceed to place)
#endif
#if 0   // PT-W8 RETIRED (SearchTrayToPlace_Magazine): real translated body now live
int  SearchTrayToPlace_Magazine()                  { return 0; }               // golden aoutarm9045.h:63 (int) -- AI(W906-W7-A2) 20260728: fix return-type ODR violation -- this def and 14 of the 27 site-variant TUs (aoutarm9045_{1x2_2,1x2_4,1x4_2,1x4_4,1x4_4S,1x4_4_Back,1x4_8,2x2_4,2x2_4_14,2x2_4_23,2x2_8,2x4_16,2x5_8,S_1x4_4}.cpp) forward-declared this `void`, disagreeing with the other 13 variants' `int` and with golden's `int` (aoutarm9045.h:63, def aoutarm9045.cpp:1420). Harmless today only because the one golden call site that consumes the return value (aoutarm9045.cpp:1369 `return SearchTrayToPlace_Magazine();`) is not yet translated into this tree's aoutarm9045.cpp -- unified to `int`/`return 0` now so that future translation of that call site does not inherit a live ODR trap. AI(W906-A2-followup) 20260728: corrected gloss -- golden's real not-found path returns Prod.iIfErrorT6 (golden aoutarm9045.cpp:1558), NOT 0; golden never returns a literal 0 from this function's real body (paths return 2, iSortTrayIndex[k], OutArmSuck.iWhichAuto[i][j], or Prod.iIfErrorT6 -- golden :1453,1469,1530,1535,1540,1546,1552,1558). `return 0` here is an arbitrary offline placeholder, not golden's not-found value; if ever consumed downstream (currently all 27 ported call sites discard the return value) it would read as Auto1, matching this file's own SearchTrayToPlace_9045 stand-in at aoutarm9045.cpp:1501 (`return 0; // golden :2451 -> iWhichAuto=0 (Auto1)`) -- there is no other Search* stand-in in this file, and the only Verify* stand-in (VerifyTrayStatus, line 94 above) returns 3300, not 0.
#endif
bool CheckPlaceToMagazineTray(int)                 { return true; }            // golden Magazine.h -- offline: place OK
bool DoPickFromMagazineBuffer()                    { return true; }            // golden Magazine.h -- offline: pick OK
#if 0   // PT-W5c RETIRED (AutoTrayReCheck)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool AutoTrayReCheck(int)                          { return true; }            // golden aoutarm9045.cpp (Ifor 20171031) -- offline: re-check OK
#endif

//==============================================================================
//  Fix-AI-CCD form stand-in -- RETIRED FROM THIS FILE.
//
//  AI(W906-W7-L1-Wave0) 20260801: the TU-local `class TfFixAICCD`, its method
//  body and the `TfFixAICCD *fFixAICCD = 0;` global that used to sit here have
//  MOVED to forms/fFixAICCD.{h,cpp} (in ht9045_forms, which ht9045_sm PUBLIC-links
//  -- so every caller still resolves).  Three things changed with the move and all
//  three are deliberate:
//    1. The global is now a REAL object, not a null pointer.  The old comment's
//       "never derefed offline" was true only of the out-arm variants; golden
//       asendic_Auto.cpp derefs fFixAICCD UNCONDITIONALLY on main paths (:153,
//       :184, :248, :404, :454, :1920, :2113), so the null would have been a
//       crash for the W7-L1 Auto translation, not a benign stub.
//    2. The class gained golden's second method, bCheckUnloaderHasAiNG(int)
//       (golden FixAICCD.h:147) -- which golden declares returning VOID, not bool.
//    3. The four TU-local copies of this declaration (aoutarm9045_1x2_2.cpp,
//       _1x2_4.cpp, _2x6_8.cpp and the differently-typed one in _1x3_4.cpp) were
//       retired in the same pass; they had to be, because FormsFacade.h now
//       includes forms/fFixAICCD.h and those TUs include FormsFacade.h.
//  This file itself does NOT include FormsFacade.h and no longer mentions
//  TfFixAICCD, so nothing further is needed here.
//==============================================================================

//==============================================================================
//  (W6.2c-OUT batch-3) 9046AU SORT-ARM/SORT-SHUTTLE engine surface (golden
//  aoutarm9045.cpp / aoutarm.h, RogerYang 20250513-20250710).  Called by
//  aoutarm9045_2x4_8 / 2x8_8 only under USE_OUT_SORT_ARM != eartUninstall, which
//  is never true offline -> these branches are inert; defined here ONCE so
//  ht9045_sm links.  Faithful offline end-state (no AU sort dispatch).
//==============================================================================
#if 0   // PT-W8 RETIRED (NeedPlaceToSort): real translated body now live
bool NeedPlaceToSort()                                         { return false; } // golden aoutarm9045.h (9046AU) -- offline: no AU dispatch
#endif
#if 0   // PT-W5c RETIRED (GetSortArmToSortShuttleOffset)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
int  GetSortArmToSortShuttleOffset()                           { return 0; }   // golden aoutarm9045.cpp (9046AU) -- offline: 0
#endif
#if 0   // PT-W5c RETIRED (GetSortArmPitchX)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
int  GetSortArmPitchX(int /*iMovePitchX*/, int /*iOffsetPos*/) { return 0; }   // golden aoutarm9045.cpp (9046AU) -- offline: 0
#endif
#if 0   // PT-W5c RETIRED (GetSortShuttleStatus_Pick)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
void GetSortShuttleStatus_Pick(int /*iZPos*/[][MAX_ARM_Col], bool /*bZDown*/, bool /*bZFlag*/[][MAX_ARM_Col]) {} // golden aoutarm9045.cpp (9046AU) -- offline: leave caller arrays untouched (zeroed by caller)
#endif
#if 0   // PT-W5c RETIRED (IsCheckSortArmDestroyActiveFinish)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
bool IsCheckSortArmDestroyActiveFinish(int /*iXPos*/, int /*iYPos*/) { return true; } // golden aoutarm9045.cpp (9046AU) -- offline: finished
#endif
#if 0   // PT-W5c RETIRED (DoSortArmSuckPreOn)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
void DoSortArmSuckPreOn(int /*iKit*/, int /*iXPos*/, int /*iYPos*/, bool /*bZDown*/) {} // golden aoutarm9045.cpp (9046AU) -- offline: no-op
#endif
#if 0   // PT-W5c RETIRED (CheckSortArmXYPitch_2x4_8)
//AI(ht9045-v906) 20260809: PT-W5c phase 2 -- RETIRED. PRE-EXISTING latent ODR violation, not this wave: the real body always existed, but its archive member was never extracted until csystem.cpp entered the link. Real body wins.
bool CheckSortArmXYPitch_2x4_8(int * /*iX*/, int * /*iY*/, int /*iMovePitchX*/) { return true; } // golden aoutarm9045_2x4_8.cpp (9046AU; engine def) -- offline: pitch OK
#endif
#if 0   // PT-W8 RETIRED (SetSortShuttleStatus_Place): real translated body now live
void SetSortShuttleStatus_Place(int /*iShtRow*/, int /*iShtCol*/, int /*iSuckRow*/, int /*iSuckCol*/) {} // golden aoutarm9045.h:33 (9046AU) -- offline: no-op
#endif

//==============================================================================
//  (W6.2c-OUT batch-3) AutoTeach close-pitch geometry (golden aoutarm.h AutoTeach
//  surface).  Used by aoutarm9045_2x8_8 close-site placement; offline returns 0 /
//  inert so the normal (non-auto-teach) placement path is taken.
//==============================================================================

//==============================================================================
//  (W6.2c-OUT batch-3) Out-rotate kit (golden aRotateKIT_Out.h, Ifor 20251215).
//  Used by aoutarm9045_All_1Picker rotate cases 110/120; offline: rotate "done".
//==============================================================================
// AI(W906-HOME-C2) 20260920: `MoveOutRotateToDegreeAtSameTime()` 的假樁已刪 ——
//   真本體在 RotateKit/aRotateKIT_Out.cpp:4307（golden :3813-3912），
//   同 D-2 的指示（aRotateKIT_Out.cpp:154）。
//   行為：從「恆回 true（轉到位了）」變成真的驅動旋轉站並回報是否到位。
