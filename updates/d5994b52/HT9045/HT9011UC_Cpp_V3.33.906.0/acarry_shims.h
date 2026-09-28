// =============================================================================
//  acarry_shims.h  --  W6.5 offline cross-module shims for the SHUTTLE/CARRY engine
//
//  Translation wave: W6.5 (SHUTTLE/CARRY ENGINE -- acarry.cpp)
//  Translator: AI(W6.5-CARRY) 20260626
//
//  WHY THIS FILE EXISTS
//  --------------------
//  The faithfully-transcribed shuttle engine (acarry.cpp) reaches a handful of
//  cross-module subsystems with NO translated home this wave.  Rather than gate
//  hundreds of call sites inside the verbatim switch(Task) SMs (which would
//  obscure the cursor flow we are preserving), this header exposes ONLY the
//  surface the engine dereferences, on transparently-named non-VCL stand-ins,
//  with offline-safe bodies (false / 0 / no-op) in acarry_shims.cpp.  This
//  mirrors the FormsFacade form-pointer cut + the acatchtray_shims / atester_shims
//  approach already proven by the in/out-arm + tray-arm + tester engine waves.
//
//  Each symbol cites its golden home.  When the real subsystem lands (W7:
//  csystem.cpp / LaserSensorShuttle.h / LtcSensor.h / LastSet.h / ATC_Handler_Side
//  / the 9046AU sort-arm), these shims are replaced and the call sites bind to
//  the real bodies unchanged.
//
//  GATED (#if 0): NONE -- every shimmed body is ACTIVE + offline-safe so the
//  shuttle SMs pump over the Sim HAL.
//
//  Big5: Chinese comments decoded via cp950, preserved UTF-8.  ZERO U+FFFD.
// =============================================================================
#ifndef acarry_shimsH
#define acarry_shimsH

#include "vclcompat/vcl_compat.h"   // AnsiString
#include "aHotPlateSubstrate.h"     // TMyKitSuck (for OutSht3Kit) -- golden MyKitSuck.h

// ===========================================================================
//  Win32 thread tokens  -- golden <windows.h> (HThreadCtrlShuttle, acarry.cpp
//  :6940-7229).  The wrapper is converted to a THIN OFFLINE DRIVER (no real OS
//  thread).  Provide portable no-op aliases for the HANDLE/DWORD/LPVOID/CALLBACK/
//  INFINITE/WAIT_* tokens the (kept-verbatim) thread code mentions, so the body
//  compiles WITHOUT pulling <windows.h> name-mangling chaos into the SM TU.
// ===========================================================================
#ifndef HT9045_ACARRY_WIN32_TOKENS
#define HT9045_ACARRY_WIN32_TOKENS
typedef void*          HANDLE;          // golden windows.h
typedef unsigned long  DWORD;           // golden windows.h
typedef void*          LPVOID;          // golden windows.h
#ifndef CALLBACK
#define CALLBACK                        // golden __stdcall -- dropped offline
#endif
#ifndef INFINITE
#define INFINITE 0xFFFFFFFF             // golden windows.h
#endif
#ifndef WAIT_FAILED
#define WAIT_FAILED  0xFFFFFFFF         // golden windows.h
#endif
#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 0x00000102         // golden windows.h
#endif
#endif // HT9045_ACARRY_WIN32_TOKENS

// ===========================================================================
//  TECH `Tech` -- AI(W906-GA1-B1) 20260804: MINIMAL 6-field mirror RETIRED.
//  The real translated TECH (427 fields) + extern Tech now come from
//  LastSet.h/LastSet.cpp; all 6 shim fields matched golden name+type exactly.
//  Offline default-0 semantics preserved (static-storage zero-init).
// ===========================================================================
#include "LastSet.h"

// ===========================================================================
//  fLtcSensor (TfLtcSensor*)  -- golden LtcSensor.h.  The shuttle Y-Latch
//  residual / stack / auto-latch checks read the per-sensor latch buffers.
//  MINIMAL mirror of exactly the touched surface: LatchDataCnt0..9 (count) +
//  LatchDataTable0..9[1024] (positions) + Get/Set/ClearLtcSensor(int).  Offline:
//  every count = 0 (no latch events) -> the residual loops find nothing -> the
//  "no remain IC" fall-through, and Get/Set/Clear are no-ops.
// ===========================================================================
class TfLtcSensor
{
public:
    int LatchDataCnt0, LatchDataCnt1, LatchDataCnt2, LatchDataCnt3, LatchDataCnt4;  // golden LtcSensor.h
    int LatchDataCnt5, LatchDataCnt6, LatchDataCnt7, LatchDataCnt8, LatchDataCnt9;  // golden LtcSensor.h
    int LatchDataTable0[1024], LatchDataTable1[1024], LatchDataTable2[1024];        // golden LtcSensor.h
    int LatchDataTable3[1024], LatchDataTable4[1024], LatchDataTable5[1024];        // golden LtcSensor.h
    int LatchDataTable6[1024], LatchDataTable7[1024], LatchDataTable8[1024];        // golden LtcSensor.h
    int LatchDataTable9[1024];                                                      // golden LtcSensor.h
    int  GetLtcSensor(int iWhich);     // golden LtcSensor.h -- offline: 0
    void SetLtcSensor(int iWhich);     // golden LtcSensor.h -- offline: no-op
    void ClearLtcSensor(int iWhich);   // golden LtcSensor.h -- offline: clear counts
    TfLtcSensor();
};
extern TfLtcSensor *fLtcSensor;        // golden LtcSensor.h: extern TfLtcSensor *fLtcSensor

// ===========================================================================
//  OutSht3Kit (TMyKitSuck)  -- golden MyKitSuck.h (9046AU sort-shuttle kit).
//  Do_Auto_SHT3 reads OutSht3Kit.UseSiteHasIC() / .iShtRow / .iShtCol.  It is
//  never dispatched offline (USE_OUT_SORT_ARM==eartUninstall) but must link.
//  Reuse the proven TMyKitSuck (aHotPlateSubstrate.h); offline grid = all NULL_IC.
//  NOTE: csystem_predicates.cpp references OutSht3Kit only behind
//  #if HT9045_KITSUCK_GRID_AVAILABLE (OFF) -> no real definition there, so this
//  is the SOLE definition (owned in acarry_shims.cpp).
// ===========================================================================
extern TMyKitSuck OutSht3Kit;          // golden MyKitSuck.h (9046AU sort kit)

// ===========================================================================
//  ATC_InterfaceForm / ATC_Data  -- golden ATC_Handler_Side.h.  DoFloodGateClose
//  reads ATC_InterfaceForm->iATC_MODE_TYPE and ATC_Data.dChillerSetTemp.  Offline
//  there is no ATC link: iATC_MODE_TYPE!=36 -> DoFloodGateClose returns false
//  (no flood-gate close), the deterministic safe branch.
// ===========================================================================
class TATC_InterfaceFormShim
{
public:
    int iATC_MODE_TYPE;            // golden ATC_Handler_Side.h -- offline: 0 (!=36)
    TATC_InterfaceFormShim();
};
extern TATC_InterfaceFormShim *ATC_InterfaceForm;   // golden ATC_Handler_Side.h

struct TATC_DataShim
{
    double dChillerSetTemp;        // golden ATC_Handler_Side.h -- offline: 0
    TATC_DataShim();
};
extern TATC_DataShim ATC_Data;     // golden ATC_Handler_Side.h

// ===========================================================================
//  Cross-module FUNCTIONS the shuttle engine calls that have NO translated home
//  this wave (golden csystem.cpp / LaserSensorShuttle.h / aoutarm / main).  All
//  ACTIVE offline shims; offline-default values match the proven canary posture
//  (no floating, no HTTP, no sort-arm, shuttle empty -> "nothing to do").
// ===========================================================================
//  shuttle-floating detectors (golden LaserSensorShuttle.h:16-18)
//  AI(W906-PT-W3-integrate) 20260808: SHIMS RETIRED -- these three now declare the
//  REAL bodies that landed with OmronLaser/LaserSensorShuttle.cpp in PT-W3
//  (CheckShtFloating :656 / UseInArmCheckShtFloating :1070 /
//  UseOutArmCheckShtFloating :1368; golden :455 / :942 / :1240).  The bodies that
//  used to live in acarry_shims.cpp:80-82 are gone.
//    Two separate things were wrong with the old shim block, and only the first
//  one announced itself:
//    (1) CheckShtFloating had the SAME signature as the real body, so the link
//        broke loudly (`multiple definition of CheckShtFloating(int, bool)`).
//    (2) The two Use* shims declared only TWO parameters where golden declares
//        THREE (`bool bSetGold=false`, golden LaserSensorShuttle.h:17-18).  That
//        is not a collision, it is a DIFFERENT OVERLOAD -- so it linked silently
//        and acarry.cpp's five call sites (:4601/:4606/:4941/:4946/:6718) bound
//        to a stub that always returned false while the real 294-line engine sat
//        unused next to it.  Restoring the third parameter is what actually fixes
//        that; it was never going to show up as a link error.
//  Signatures below are byte-for-byte golden LaserSensorShuttle.h:16-18, so the
//  existing call sites keep compiling unchanged and now reach the real bodies
//  with bSetGold defaulting to false -- which is golden's own default.
//  NOTE: the defaults are given here AND in OmronLaser/LaserSensorShuttle.h.  C++
//  forbids repeating a default argument in one scope, so no TU may include both.
//  Checked this pass: no OmronLaser/*.cpp includes acarry_shims.h and
//  aHotPlateSubstrate.h does not pull it in, so nothing does today.
bool CheckShtFloating(int iSht, bool bReset=false);                         // golden LaserSensorShuttle.h:16
bool UseInArmCheckShtFloating(int iSht, bool bReset=false, bool bSetGold=false);   // golden :17
bool UseOutArmCheckShtFloating(int iSht, bool bReset=false, bool bSetGold=false);  // golden :18

//  In-shuttle left/right position predicates -- (int) overloads (golden csystem.cpp).
//  The (void) variants InSHT1InLF/InSHT2InLF/InSHT1InRT/InSHT2InRT already live in
//  csystem_predicates.cpp; these int-dispatch overloads route to them.
bool InShtInLF(int iSht);                                           // golden csystem.cpp -- routes to InSHT1/2InLF
bool InShtInRT(int iSht);                                           // golden csystem.cpp -- routes to InSHT1/2InRT

//  9046AU sort-shuttle position predicates (golden csystem.cpp -- 9046AU only)
bool SortSHTInLF();                                                 // golden csystem.cpp -- offline: false
bool SortSHTInRT();                                                 // golden csystem.cpp -- offline: false

//  in-shuttle step / residual / prominent checks (golden csystem.cpp).  Offline:
//  step-check passes (true), residual/must-have/prominent find nothing (false).
bool DoStepShuttleCheck(int iShuttle);                             // golden csystem.cpp -- offline: true (no step pending)
bool DoCheckShuttle1EmptyIC();                                     // golden csystem.cpp -- offline: true (empty OK)
bool DoCheckShuttle2EmptyIC();                                     // golden csystem.cpp -- offline: true
bool DoCheckShuttle1MustHasIC();                                   // golden csystem.cpp -- offline: true
bool DoCheckShuttle2MustHasIC();                                   // golden csystem.cpp -- offline: true
bool DoCheckShuttle1Prominent();                                   // golden csystem.cpp -- offline: true (not prominent)
bool DoCheckShuttle2Prominent();                                   // golden csystem.cpp -- offline: true
void InitialCheckShuttle1MustHasICTask();                          // golden csystem.cpp -- offline: no-op
void InitialCheckShuttle2MustHasICTask();                          // golden csystem.cpp -- offline: no-op
void InitialCheckShuttle1ProminentNoHasICTask();                  // golden csystem.cpp -- offline: no-op
void InitialCheckShuttle2ProminentNoHasICTask();                  // golden csystem.cpp -- offline: no-op
void InitialCheckShuttle1ProminentTask();                          // golden csystem.cpp -- offline: no-op
void InitialCheckShuttle2ProminentTask();                          // golden csystem.cpp -- offline: no-op

//  32-site enable predicates (golden csystem.cpp) -- offline: false (24Site/4x4 off)
bool Check32siteOnlyEnabled24Site();                               // golden csystem.cpp -- offline: false
bool Check32siteOnlyEnabled4x4Site();                             // golden csystem.cpp -- offline: false

//  out-arm let-shuttle-pass + bottom-2DID HTTP + sort-arm safe-move (golden
//  aoutarm / main / 9046AU).  Offline: out-arm already away (true), HTTP OK
//  (K_OK), sort-arm safe-move no-op.
bool MoveOutArmXY_ToShuttleAlarmArea();                            // golden aoutarm -- offline: away (true)
int  SendHTTPRequest(int iSht);                                    // golden main -- offline: 0 (success, not the -1 fail path)
//AI(ht9045-v906) 20260809: PT-W5c -- return type corrected void -> bool to match golden
// asortarm.h:37, whose real body (asortarm.cpp:663) now wins the link. The offline no-op
// here was `void`, which asortarm.cpp:494 had already flagged as a mismatch; C++ mangling
// ignores the return type, so the two disagreed on the contract while still colliding as
// one symbol. acarry.cpp:8295/8302 discard the result, so correcting it changes nothing
// at those call sites. Declaration only -- acarry.cpp does not include asortarm.h.
bool MoveSortArmToAutoSafe();                                     // golden asortarm.h:37

//  Do_Auto_SHT1AsOutArmIsRotater -- golden acarry.h declares it but the golden
//  .cpp NEVER defines it (decl-only).  Provide an inert active stub so the header
//  declaration resolves if anything links against it.
void Do_Auto_SHT1AsOutArmIsRotater();                             // golden acarry.h decl-only -- offline: no-op

// ===========================================================================
//  IN-ARM symbols the shuttle engine calls (golden ainarm9045.h).  ainarm9045.h
//  is NOT included by acarry.cpp (its InArmLeftSideNoIC/HasIC default-arg
//  declarations clash with aHotPlateSubstrate.h's), so the exact 5 entry points
//  acarry references are forward-declared here.  All have REAL bodies in
//  ainarm9045.cpp (W6.2b) -- these are plain forward decls, NOT shims.
// ===========================================================================
bool CheckShuttleSensor_9045(int CheckItem);                      // golden ainarm9045.h:100 (real body in ainarm9045.cpp)
int  CheckOneCycleAction(int iTask);                              // golden ainarm9045.h:73
int  CheckInShuttleSensor_Latch(int iShuttle, int iMoveAct, bool bAlarm); // golden ainarm9045.h:140
void InitAutoChkInSHLatchTask();                                  // golden ainarm9045.h:139
void InitInArmTask();                                             // golden ainarm9045.h / aHotPlateSubstrate.h

// ===========================================================================
//  Cross-module SYMBOLS that already have a LINK-VISIBLE definition in a SIBLING
//  translated TU but whose DECLARING header acarry.cpp does NOT include (pulling
//  those headers in would drag clashing default-arg redecls).  Plain forward
//  declarations -- NOT shims; the real bodies bind at link time:
//    * IsNNMode           -- defined atester_shims.cpp (golden cinitial.h:60)
//    * SetMotorScaleSpeed -- defined acatchtray_shims.cpp (golden cinitial.h:50)
// ===========================================================================
int  IsNNMode();                                  // golden cinitial.h:60  (body in atester_shims.cpp)
void SetMotorScaleSpeed(int Index, int ScaleSpeed); // golden cinitial.h:50 (body in acatchtray_shims.cpp)

// ===========================================================================
//  Cross-module SYMBOLS with NO translated home anywhere in the link set this
//  wave -> ACTIVE offline shims OWNED here (acarry_shims.cpp):
//    * MoveOutArmToAutoSafe -- golden aoutarm.h:52 (the file-local `static`
//      MoveOutArmToAutoSafe in aoutarm9045.cpp is NOT link-visible; the carry
//      engine needs the cross-module variant).  Offline: out-arm reaches the
//      safe position immediately -> true.
//    * MySleep / MySleepEx  -- golden common.h:260-261.  AI(W906-CommonCompletion)
//      20260721: common.cpp now provides the REAL bodies (this wave un-gated
//      them); the offline no-op stand-in DEFINITIONS this file used to own were
//      removed (see acarry_shims.cpp).  These two lines are now just forward
//      declarations for this header's own callers -- same signatures as
//      common.h:365-366, one real definition in common.cpp.
//    * SystemNG             -- golden global bool (main-side VCL form unit, no
//      translated home).  Offline false -> the `if(SystemNG) return;` SM guards
//      proceed (system not in NG).
//    * iCloseSiteStep_2x8   -- golden ainarm9045_2x8_8.h:27 (extern int, 2x8
//      close-site pitch step).  Offline 0 -> the 2x8 close-site geometry collapses
//      to the no-offset baseline.
// ===========================================================================
bool  MoveOutArmToAutoSafe();                     // golden aoutarm.h:52 -- offline: safe reached (true)  [AI(W906-FLOW-2) 20260928: superseded -- body live since 216f7bd8: aoutarm.cpp:858 -> MoveOutArmToAutoSafe_9045 (golden :55-155), false until every nozzle reads safe]
DWORD MySleepEx(DWORD dwMilliseconds, bool bAlertable); // golden common.h:260 -- real body in common.cpp
void  MySleep(DWORD dwMilliseconds);              // golden common.h:261 -- real body in common.cpp
extern bool SystemNG;                             // golden main-side global -- offline: false
extern int  iCloseSiteStep_2x8;                   // golden ainarm9045_2x8_8.h:27 -- offline: 0

#endif // acarry_shimsH
