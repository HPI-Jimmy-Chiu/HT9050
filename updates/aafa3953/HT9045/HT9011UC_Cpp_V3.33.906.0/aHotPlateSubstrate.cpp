// =============================================================================
//  aHotPlateSubstrate.cpp  --  W6.2 in-arm HotPlate substrate (offline sim bodies; the 2-arg MyDBIProcess is a LIVE adapter since P4 -- AI(W906-P4) ★W56, Steven 0928)
//
//  Translation wave: W6.2 (IN-ARM foundation -- shared HP pick/place geometry)
//  Translator: AI(W6.2-INARM) 20260626
//
//  Provides the offline definitions/bodies for the substrate declared in
//  aHotPlateSubstrate.h:
//    (a) the KitSuck grid objects + minimal TMyKitSuck / TMySucker bodies
//    (b) PickFromHPList (uPlateInfo) + uPoint2D, minimally
//    (c) the ainarm2 cursors / arrays + InArmSiteMapData + ZAxisDown
//    (d) the in-arm engine helper stubs (real bodies live in the later ainarm
//        core, W6.x/W7)
//    (e) tRotate / LaserCheckPos + TMyProductionRecord bodies the leaves call
//
//  All bodies are CONSERVATIVE offline defaults over the Sim HAL (except the 2-arg MyDBIProcess adapter, live since P4, ★W56):
//    * vacuum Suck()/sensors report "no IC / not finished" (false)
//    * the team-list (PickFromHPList) is empty -> Get* return false, so the
//      geometry leaves take their "nothing to do" branch deterministically
//    * engine helpers that gate motion return "not finished" (false) / no-op
//  This is faithful to a handler with no HotPlate populated and is exactly the
//  surface the W6.2 canary asserts against.
//
//  Big5: Chinese comments preserved as UTF-8 (cp950).  NO U+FFFD.
// =============================================================================
#include "aHotPlateSubstrate.h"

// -- W5-BarCode INTEGRATE ADD (20260711): real bodies for 18 of the 20
//    TfBarCode methods declared aHotPlateSubstrate.h:623-664 (7 parallel
//    W5-BarCode translate units; see docs/MIGRATION_ROADMAP.md DEFERRED table
//    entry "BarCode/{BarCode,BarCode_Sh1,BarCode_Sh2,BarcodeXML}.cpp" for the
//    still-untranslated ~24,500-line VCL-form core these units extract from).
//    BarCode_Bottom2DID8CCD.{h,cpp} intentionally live at project ROOT (not
//    under BarCode/), per that unit's exact task-brief path -- see its own
//    file banner.
#include "BarCode/BarCode_Helpers.h"          // BarCode_IsSHT2DIDScanFinish
#include "BarCode/BarCode_Bottom2DID.h"       // BarCode_InitBottom2DIDScan / BarCode_DoBottom2DIDScan
#include "BarCode_Bottom2DID8CCD.h"           // BarCode_DoBottom2DID_8CCD_Scan
#include "BarCode/BarCode_Shuttle1_Scan.h"    // InitialBarcodeScanInShuttle1/OutShuttle1, DoBarcode{Scan,Trigger}InShuttle_1, DoBarcodeScanOutShuttle_1, InitialShuttleFloatCheck1, DoShuttleFloatCheck_1
#include "BarCode/BarCode_Shuttle2_Scan.h"    // BarCode_Sh2_* (InitialBarcodeScanIn/OutShuttle2, DoBarcodeTriggerInShuttle_2, DoBarcodeScanOutShuttle_2, InitialShuttleFloatCheck2)
#include "BarCode/BarCode_Shuttle1_CCDScan.h" // BarCode_DoBarcodeCCDInShuttle_1
#include "BarCode/BarCode_Shuttle2_CCDScan.h" // BarCode_DoBarcodeCCDInShuttle_2
// -- W5-BarCode-Final INTEGRATE ADD (20260711): the 2 leftover TfBarCode
//    methods flagged "REMAINING / HANDED OFF" by BarCode_Shuttle2_Scan.h --
//    now delivered by 2 separate hand-off translate units, completing 20/20.
#include "BarCode/BarCode_Shuttle2_ScanRemainder1.h" // BarCode_Sh2_DoBarcodeScanInShuttle_2
#include "BarCode/BarCode_Shuttle2_ScanRemainder2.h" // BarCode_Sh2_DoShuttleFloatCheck_2
//AI(W906-W7-L2) 20260803: TMySucker::Suck() below reads LastSet.iRealDummy to
// reproduce golden's DUMMY early-out (MyKitSuck.cpp:2167).  LastSet already
// arrives in this TU transitively -- BarCode/BarCode_Shuttle1_Scan.h (included
// at :39) includes canary_support.h at its own line 85 -- so this line is a
// literal preprocessing no-op today (guard canary_supportH is already defined).
// It is added only so the dependency survives someone dropping that BarCode
// include, and placed AFTER the BarCode block so include ORDER is unchanged.
#include "canary_support.h"                   // LAST_GENERAL_SET LastSet (.iRealDummy)

//==============================================================================
// AI(W906-W7-L2) 20260803: grid-wide half of the TMySucker::OffDestroy()
// observability seam (declared aHotPlateSubstrate.h, "OBSERVABILITY SEAM"
// block).  Defined BEFORE the KitSuck grid objects below on purpose: both
// initialisers are constant expressions, so these two are CONSTANT-initialised
// and are therefore already live when the grids' dynamic initialisation (the
// TMySucker ctor, which stamps itself with the epoch) runs -- no static
// initialisation-order hazard, regardless of link order.
//==============================================================================
unsigned long W906_TMySucker_OffDestroyEpoch = 1;   // 0 is reserved for "never stamped"
long          W906_TMySucker_OffDestroyTotal = 0;

void W906_TMySucker_OffDestroy_ResetAll()
{
    // Bump, never re-zero: a nozzle whose stamp lags the epoch reads as 0, so one
    // increment retires every per-nozzle count in the tree in O(1).
    ++W906_TMySucker_OffDestroyEpoch;
    W906_TMySucker_OffDestroyTotal = 0;
}

//==============================================================================
//  (a) KitSuck grid objects (golden MyKitSuck.h:357-366) -- offline instances
#if 0   // AI(W906-A4-6) 20260924: 精簡鏡像的全域物件與 TMySucker／TMyKitSuck 方法本體退役 —— 真的定義在 mykitsuck.cpp（golden MyKitSuck.cpp 忠實翻譯，本顆起註冊進建置）
TMyKitSuck InArmSuck;
TMyKitSuck FLCarryKit;
TMyKitSuck BLCarryKit;
TMyKitSuck OutArmSuck;
// -- W6.2c ADD: OUT-ARM KitSuck objects (golden MyKitSuck.h:359/361/366) -------
TMyKitSuck OutArm2Suck;
TMyKitSuck FRCarryKit;
TMyKitSuck BRCarryKit;
// -- W6.3 ADD: TRAY-ARM KitSuck object (golden MyKitSuck.h:367) -----------------
TMyKitSuck CatchTraySuck;
// -- W6.4 ADD: TESTER/INDEX KitSuck objects (golden MyKitSuck.h:363/364/368) -----
TMyKitSuck TestSocket;
TMyKitSuck FTestSuck;
TMyKitSuck BTestSuck;

// ---- TMySucker bodies -------------------------------------------------------
//  Offline: no real vacuum line.  Suck() never reports "finished" (the leaves
//  treat that as "still building vacuum"); On()/Off() are no-ops.
//AI(W906-W7-L2) 20260803: the "On()/Off() are no-ops" and (below) the
// "OnSuck/OnDestroy/OffDestroy/Normal are solenoid no-ops" / "Reset() ... no-op"
// claims in the two pre-existing banners are SUPERSEDED by this wave and kept
// only as history.  Still true: nothing here touches hardware.  Now also true:
// the vacuum-ON line is modelled by an in-object latch (W906_SimVacuumOnBit) so
// GetOnBit() can answer, Normal() composes the way golden composes it
// (MyKitSuck.cpp:2143-2149), Reset() carries golden's real body
// (MyKitSuck.cpp:1855-1860), and OffDestroy() keeps a call count so
// StopAllDestroy (golden ckernel.cpp:189-209) stops being untestable.
//AI(W906-W7-L2) 20260803: Suck() now MAINTAINS the vacuum-ON latch, because it
// and Destroy() are the DOMINANT drivers of that output bit -- measured in this
// tree: `Suck[i][j].Suck()` 123 call sites, `.Destroy()` 140, versus `.On()` 31
// and `.Off()` 25.  Leaving these two latch-blind made GetOnBit() answer false
// on the paths that matter and silently flipped the ckernel "vacuum solenoid
// still ON but the site holds no IC" guard onto its no-residual-vacuum arm.
// RETURN VALUE DELIBERATELY UNCHANGED: offline GetStatus() is hard-false, so
// golden could not reach its own `return true` arms (:2200/:2324/:2341/:2359)
// either -- only the latch write is added.
// GOLDEN, READ THIS PASS (MyKitSuck.cpp:2161-2360):
//   :2167-2169 `if(LastSet.iRealDummy==DUMMY || LastSet.iRealDummy==HAS_TRAY ||
//              (LastSet.iRealDummy==HAS_TRAY && SuckerName.AnsiCompare(
//              "CatchSuck")!=0))` -- the third disjunct is a strict SUBSET of the
//              second, so the whole test reduces to `iRealDummy != REALLY` and is
//              SuckerName-FREE.  That is why this early-out IS exactly
//              reproducible here even though the shim has no SuckerName.
//   :2171-2205 the dummy branch.  Its only IO call is DoOffIO(false) (:2183) --
//              the DESTROY line.  It NEVER writes the vacuum-ON bit.
//   :2207      `if(OnTask!=200)`  -- reproduced; OnTask is a real member here.
//   :2210      `DoOnIO(true);` (//開啟真空) -- the latch SET this repair adds.
//   :2230/:2283 `Normal();` on the retry-exhausted arms -> OffSuck() (:2145) ->
//              DoOnIO(false) (:2102): those two arms CLEAR the bit.
// NOT REPRODUCED -- a real hole, not a tidy one: this body returns false
// unconditionally and never advances OnTask, so it can never reach :2230/:2283.
// After a retry-exhausted suck golden leaves the bit CLEAR while this shim
// leaves it SET; a test that needs that arm must call Normal() itself.
// SIM-DEFAULT WARNING: canary_support.cpp:26 is `LAST_GENERAL_SET LastSet = {0}`
// and DUMMY==0 (cmydef.cpp:267), so out of the box iRealDummy==DUMMY and this
// function takes the early-out -- which is exactly what golden does.  A test
// that wants the bit SET must first set LastSet.iRealDummy=REALLY (==2).
bool TMySucker::Suck()
{
    //AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:2166 `Error=false;` -- an
    // UNCONDITIONAL clear that sits BEFORE the dummy gate at :2167, so it runs on
    // every call including the dummy early-out.  Restored after an independent
    // re-audit found the first landing had dropped it: without it the tree's
    // dominant caller idiom `if(Suck() || Error)` turns from self-clearing into
    // sticky, and a nozzle that errored once would report error forever.
    Error = false;
    if(LastSet.iRealDummy == DUMMY || LastSet.iRealDummy == HAS_TRAY)
        return false;                   // golden MyKitSuck.cpp:2167-2205
    if(OnTask != 200)
        W906_SimVacuumOnBit = true;     // golden MyKitSuck.cpp:2207-2210
    return false;                       // UNCHANGED: offline suck never "finished"
}
//AI(W906-W7-L2) 20260803: golden Destroy() (MyKitSuck.cpp:2362-2440+) writes the
// vacuum-ON bit on four paths and EVERY ONE OF THEM CLEARS IT -- :2395 (dummy
// branch, `DoOnIO(false)`), :2436 (mainline `DoOnIO(false)`, under the :2434
// `if(OffTask!=300)` guard), plus :2507 `Normal()` and :2535/:2581 `OffSuck()`,
// which reach DoOnIO(false) through :2145/:2102.  One guarded clear therefore
// matches golden on every path this body can reach, and that is also why NO
// dummy early-out is attempted: golden's is `iRealDummy==DUMMY ||
// (iRealDummy==HAS_TRAY && SuckerName.AnsiCompare("CatchSuck")!=0)`
// (:2381-2382), and the clear INSIDE it (:2392-2396) is gated on SuckerName
// containing "InArmSuck"/"OutArmSuck" -- so returning early on DUMMY would
// WRONGLY skip the clear for exactly the arm nozzles that are the bulk of this
// tree's callers.  KNOWN GAP: for a non-arm nozzle in DUMMY/HAS_TRAY golden
// leaves the bit alone and this clears it.  Reaching that divergence needs the
// bit already set, which needs an earlier REALLY-mode Suck()/On() -- a mixed-
// mode sequence.  Recorded rather than silently absorbed.
// SECOND, SMALLER DUMMY-MODE DIVERGENCE, disclosed after re-audit: golden's
// dummy-branch clear is reached only on `case 1` of `switch(OffTask)`
// (:2384-2398); :2399 case 2 and :2412 case 50 write no ON bit at all.  This
// body clears on every call with OffTask!=300, with no case discrimination.
// Inert in this tree today because the offline OffTask never advances off 1,
// but it is a divergence, not an equivalence.
bool TMySucker::Destroy()
{
    //AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:2367 `Error=false;` -- same
    // unconditional pre-gate clear as Suck() (:2166), same restoration reason.
    Error = false;
    if(OffTask != 300)
        W906_SimVacuumOnBit = false;    // golden MyKitSuck.cpp:2434-2436
    return false;   // W6.2b: offline destroy never "finished" -> SM holds
}
//AI(W906-W7-L2) 20260803: On()/Off() latch the LOGICAL vacuum-ON solenoid state
// so GetOnBit() has something real to read back, mirroring golden's
// DoOnIO(true)/DoOnIO(false) at MyKitSuck.cpp:2129/2137.
// KNOWN GAP -- AND THE REASON GIVEN FOR IT EARLIER TODAY WAS WRONG, corrected
// here in place.  Golden On() early-outs on `LastSet.iRealDummy==DUMMY`
// (:2121-2122) and on `bLoadInarmAutoHigh==false && HAS_TRAY &&
// SuckerName.AnsiCompare("CatchSuck")!=0` (:2124-2125); golden OnSuck() early-
// outs on the same pair (:2091-2094); golden Off() additionally sets
// Status=false (:2136) and drives the destroy line ON (:2138).  The earlier
// revision of this comment blamed the missing DUMMY gate on LastSet "living in
// canary_support.h, owned by another agent this wave".  That is NOT the reason:
// LastSet is in scope in this TU and Suck() above now uses it.  The real reason
// is CHARTER -- this repair pass was scoped to Suck()/Destroy(), so On()/OnSuck()
// are left as they are and the mismatch is reported rather than fixed blind.
// CONSEQUENCE, STATED PLAINLY: under the sim default iRealDummy==DUMMY, Suck()
// leaves the bit clear (faithful) while On()/OnSuck() set it (unfaithful).  The
// fix is the same one-line `if(LastSet.iRealDummy==DUMMY) return;` guard and
// belongs to whoever lands the real MyKitSuck.  Status and a destroy-line model
// remain genuinely absent from this shim's surface.
void TMySucker::On()      { W906_SimVacuumOnBit = true;  }   //AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:2129 DoOnIO(true)
void TMySucker::Off()     { W906_SimVacuumOnBit = false; }   //AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:2137 DoOnIO(false)
// -- W6.2c ADD: out-arm-touched TMySucker surface (golden MyKitSuck.h) ---------
//    Offline: no real vacuum line.  OnSuck/OnDestroy/OffDestroy/Normal are
//    solenoid no-ops; GetStatus() reports "no IC held" (false) so the out-arm
//    destroy-confirm SM (CheckOutArmDestroyActive case 300) takes its
//    "destroy finished" branch deterministically.
void TMySucker::OnSuck()     { W906_SimVacuumOnBit = true; } //AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:2096 DoOnIO(true)
void TMySucker::OnDestroy()  {}
//AI(W906-W7-L2) 20260803: OffDestroy is StopAllDestroy's ENTIRE payload (golden
// ckernel.cpp:189-209 calls this and nothing else), and offline it drives no
// solenoid -- so without a counter a no-op StopAllDestroy and a faithful one are
// indistinguishable and any test over it is tautological.  The count is the
// observable.  Epoch-stamped so W906_TMySucker_OffDestroy_ResetAll() isolates
// tests in O(1) (see the header block for the design note).
void TMySucker::OffDestroy()
{
    if(W906_OffDestroyStamp != W906_TMySucker_OffDestroyEpoch)
    {
        W906_OffDestroyStamp = W906_TMySucker_OffDestroyEpoch;
        W906_OffDestroyRaw   = 0;
    }
    ++W906_OffDestroyRaw;
    ++W906_TMySucker_OffDestroyTotal;
}
//AI(W906-W7-L2) 20260803: Normal() is COMPOSED exactly as golden composes it --
// golden MyKitSuck.cpp:2143-2149 is `OffSuck(); OffDestroy(); bSuckOK=true;
// bDestroyOK=true;`.  OffSuck's effect is DoOnIO(false) (MyKitSuck.cpp:2102), so
// it is inlined here as the latch clear (OffSuck itself is not part of the shim's
// declared surface).  IMPORTANT for anyone reading the counter: because golden's
// Normal() really does call OffDestroy(), so does this one -- ScanSystemSensor's
// Normal() calls (golden ckernel.cpp:555, 557, 580, 587, 599, 606, 637, 644, 656,
// 663) legitimately bump the count too.  StopAllDestroy never calls Normal(), so
// a StopAllDestroy-only test is unaffected.
void TMySucker::Normal()
{
    W906_SimVacuumOnBit = false;   // == golden OffSuck() -> DoOnIO(false), MyKitSuck.cpp:2102
    OffDestroy();                  // golden MyKitSuck.cpp:2146
}
bool TMySucker::GetStatus()  { return false; }   // offline: vacuum sensor reads "no IC"

// ---------------------------------------------------------------------------
//  TMySucker::Sensor -- golden MyKitSuck.cpp:1943-1949.  **完整翻譯，零閘。**
//
//  ⚠ 注意它的 false 臂回的是 **true**，不是 false：
//      沒裝真空感測器的吸嘴，一律當作「感測到料」。
//  這看起來反直覺，但它是 golden 原文，而且有道理 —— 呼叫端用它判斷
//  「這一格是不是空的」，沒有感測器時**保守地假設有料**比假設沒料安全。
//  （uhome.cpp case 2 用它決定要不要把 IC 放回去。）
// ---------------------------------------------------------------------------
bool TMySucker::Sensor()
{
    if(Enable==false)
        return true;                                                            // golden :1946
    else
        return GetStatus();                                                     // golden :1948
}
// -- W6.3 ADD: tray-arm-touched TMySucker surface (golden MyKitSuck.h) ----------
//    Offline: Reset() clears the suck/destroy task (no-op over the Sim HAL).
//    Enable/OnAlarmTime are plain data members (default-init below by the object).
//AI(W906-W7-L2) 20260803: Reset() is now golden's REAL body (MyKitSuck.cpp:1855-
// 1860) instead of an empty one -- it was only empty because ReStart() did not
// exist in the shim yet.  golden also clears iNozzleEvent (:1859); that member is
// not part of this shim's surface, so that one line is the only omission.
// SCOPE: THIS IS AN EXPANSION BEYOND THE W7-L2 RECON CHARTER.  That charter
// covered four members (VacuumOnTime/VacuumOffTime/ReStart/GetOnBit + the
// OffDestroy seam); rewriting Reset() from `{}` to golden's body was NOT in it
// and was taken on this front's own judgement.  It is disclosed, not smuggled:
// the justification is the Error-latch argument below, and the thing that
// actually VALIDATES it is the integrator's full ctest run over the whole wave,
// not any reasoning written here.  If that run is red, start at this function.
// WHY IT STANDS -- CORRECTED 20260803 AFTER RE-AUDIT.  An earlier revision of
// this comment defended the change with a bug-FIX argument: that engines latch
// `...Suck[i][j].Error=true` (it cited ainarm9045_2x4_16.cpp:1348,
// ainarm_SearchPickPlate.cpp:272/339, aoutarm9045_1x1_1.cpp:535) and that an
// empty Reset() left the latch permanently unclearable.  THAT ARGUMENT WAS
// FALSE and is retracted here rather than stacked over: all four cited sites
// sit inside `#ifdef SOFT_SIMULTE`, which this build does not define, so they
// are dead code; and the tree already contains many live `Error=false` clears,
// so the latch was never unclearable.
// The change stands on FIDELITY alone, which needs no argument: golden's whole
// Reset() body is three statements (MyKitSuck.cpp:1855-1860, read cp950 this
// pass) -- `ReStart();` :1857, `Error=false;` :1858, `iNozzleEvent=0;` :1859.
// Two of the three are reproduced below.  The third is NOT: `iNozzleEvent` has
// no member on this offline TMySucker (golden MyKitSuck.h; the shim's member
// list is aHotPlateSubstrate.h:106-313), so it is an honest omission, not an
// oversight.  Adding it would mean adding a field nothing offline reads.
// Callers affected -- ALL EIGHT were OPENED AND READ during this repair pass,
// and each is a bare `....Reset();` statement whose behaviour this change alters:
//   acatchtray.cpp:3347            `CatchTraySuck.Suck[0][0].Reset();`
//   atester_32Site.cpp:3886/3887   `FTestSuck...` / `BTestSuck.Suck[i][j].Reset();`
//   aTester_Front.cpp:972, :1413   `FTestSuck.Suck[i][j].Reset();`
//   aTester_Rear.cpp:1052, :1461   `BTestSuck.Suck[i][j].Reset();`
//   AutoClean/AutoClean.cpp:4229   `InArmSuck.Suck[i][j].Reset();`
// If a suite run shows fallout, deleting ONLY the
// `Error = false;` line below restores the previous (unfaithful) behaviour while
// keeping ReStart() -- but the correct resolution is to fix the test.
void TMySucker::Reset()
{
    ReStart();          // golden MyKitSuck.cpp:1857
    Error = false;      // golden MyKitSuck.cpp:1858
}

//AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:1849-1853 VERBATIM -- golden's own
// body touches no hardware, only the two task cursors, so this is a faithful
// translation rather than an offline approximation.  Consumed by golden
// ckernel.cpp:552-553 (In/OutArmSuck) and :570-571 (F/BTestSuck).
void TMySucker::ReStart()
{
    OnTask  = 1;
    OffTask = 1;
}

//AI(W906-W7-L2) 20260803: golden MyKitSuck.cpp:1862-1888 collapsed onto the
// offline latch.  Golden's two IO-family branches (:1867-1877) both compute the
// SAME logical answer -- the raw output bit -- and :1879-1882 then de-inverts it
// by OnType; since DoOnIO applied that same inversion when the bit was written
// (:1957 vs :1988), the logical value round-trips OnType-free.  The `OnEnable`
// gate is golden's own (:1884-1887) and is reproduced exactly: an unwired vacuum
// line answers false without consulting anything.
bool TMySucker::GetOnBit()
{
    if(OnEnable == false)
        return false;              // golden MyKitSuck.cpp:1884-1887
    return W906_SimVacuumOnBit;    // golden MyKitSuck.cpp:1870/1876 readback, :1879-1882 de-inverted
}

//AI(W906-W7-L2) 20260803: epoch-aware reader -- a count stamped with a retired
// generation reads as 0, which is what makes ResetAll O(1) and leak-proof.
int TMySucker::W906_GetOffDestroyCount() const
{
    if(W906_OffDestroyStamp != W906_TMySucker_OffDestroyEpoch)
        return 0;
    return W906_OffDestroyRaw;
}

//AI(W906-W7-L2) 20260803: per-nozzle isolation (the grid-wide form is
// W906_TMySucker_OffDestroy_ResetAll).  Note it does NOT touch the tree-wide
// total, because the total is only meaningful across a whole ResetAll epoch.
void TMySucker::W906_ResetOffDestroyCount()
{
    W906_OffDestroyStamp = W906_TMySucker_OffDestroyEpoch;
    W906_OffDestroyRaw   = 0;
}

//AI(W906-W7-L2) 20260803: the offline TMySucker ctor USED TO LIVE HERE and was
// moved INLINE into aHotPlateSubstrate.h during this same wave.  Reason, kept at
// the old site so nobody moves it back: TMySucker had no user-declared ctor
// before this wave, so a TU could instantiate one without linking this .cpp.
// An out-of-line ctor made this .cpp a link requirement for every such TU, and
// the wave's first full build failed on exactly that -- tests/
// test_MyProductionRecord.cpp -> `undefined reference to TMySucker::TMySucker()`.
// See the header for the body and for why the stamp seeds with 0 rather than
// with W906_TMySucker_OffDestroyEpoch (which is defined in THIS file).

// ---- W6.4 ADD: TMyKitSuck ctor (homes the tester decode grid) ----------------
//  Offline-safe init.  The golden TMyKitSuck has many more members; we only home
//  the ones the translated engines actually read.  iShtRow/iShtCol default to a
//  small 2-row x 1-col shuttle-site grid (any real load overwrites them via
//  cinitial), so the decode loops run a deterministic (small, in-bounds) pass and
//  the empty grid yields "no result yet" (-1).  iBinData/cDeviceInf/cSBin start
//  cleared; Item/iWhichSite are int grids left to the loader.
TMyKitSuck::TMyKitSuck()
{
    // -----------------------------------------------------------------------
    //  AI(W906-P1b) 20260920: golden 的 ctor 尾端設 12 個拓撲欄位
    //  （MyKitSuck.cpp:76-87）。移植樹原本只設了 4 個，而且其中兩個的**值不同**。
    //  ⚠ 兩個值不同的地方，原本的註解引用的是**宣告行**不是值的出處：
    //      `iShtRow = 2;   // golden :163`
    //    `MyKitSuck.h:163` 是 `int iShtRow;` —— 純宣告，**沒有預設值**。
    //    golden 的值在 ctor：`MyKitSuck.cpp:83 iShtRow=1;`。
    //    ⇒ 那個 `2` 沒有 golden 依據，是移植樹自己填的。
    //  ⇒ 這一段改成逐行照 golden，順序也照 golden。
    //
    //  ⚠ **這不等於忠實** —— 見 docs/PLAN_START_TO_RUN.md 的 P1b：
    //    golden 在 bring-up 用一個**兩參數的 SetPickerCount 多載**把值推給
    //    carry kit 與 socket（golden ainarm9045.cpp:1476-1495），
    //    那個多載與那五個呼叫點移植樹都還沒翻，所以 `FLCarryKit.iShtRow` 等
    //    在本樹一輩子是 ctor 值。**那是 P1b 的第二段。**
    // -----------------------------------------------------------------------

    // ⚠ `iXStep` / `iYStep`：golden **有這兩個成員**（MyKitSuck.h:169/170），
    //   但它的 **ctor 沒有初始化它們**。對全域物件來說 golden 的值因此是 0。
    //   移植樹設 1，這是**刻意保留的 port 決定**，不是漏抄：
    //     * 它們的語意是「X/Y pitch 要跑幾次」，0 是一個沒有意義的步數
    //     * 真正的值由 `ainarm9045.cpp:3943` 起的那一段依機型寫入，
    //       ctor 值只在那之前有效
    //   ⇒ 留 1，並在這裡寫明它不是 golden 的值。
    iXStep = 1;
    iYStep = 1;
    for(int i=0;i<_MAX_SUCK_ROW_ITEM;i++)
        for(int j=0;j<_MAX_SUCK_COL_ITEM;j++)
        {
            iBinData[i][j]  = -1;     // -1 = no decoded bin yet
            cDeviceInf[i][j]= "";
            cSBin[i][j]     = "";
            // W6.5: zero the shuttle clean-out grids the carry engine writes.
            iWhichAuto[i][j]  = 0;
            bPass[i][j]       = false;
            bNeedReTest[i][j] = false;
            // AI(W906-AutoCleanFoundation) 20260721: golden -1 = "no pending
            // AutoClean pick/place record at this nozzle" sentinel -- every
            // in-scope call site guards on `if(iXpos>=0 && iYpos>=0)` before
            // trusting these, so 0 (a VALID tray coordinate) would be wrong.
            iAutoCleanRecX[i][j] = -1;
            iAutoCleanRecY[i][j] = -1;
            //AI(W906-P1-HASIC) 20260919: golden MyKitSuck.cpp:56 `Item[i][j]=NULL_IC;`
            // 這一行移植樹原本漏了，所以物件要靠呼叫端 ClearAll() 才乾淨。
            // 全域物件零初始化剛好等於 NULL_IC（==0，樹裡多處實證），
            // 但**區域／堆積物件是垃圾值**。
            Item[i][j] = NULL_IC;
        }
    //AI(W906-P1-HASIC) 20260919: golden MyKitSuck.cpp:79-80 `iMaxRow=1; iMaxCol=1;`
    //
    // ⚠ 這兩行在 P1 之前是**無害的缺口**：那時 HasIC/HasRealIC/NoIC 用的是
    //   編譯期陣列上界 _MAX_SUCK_ROW_ITEM/_MAX_SUCK_COL_ITEM。P1 把三支改回
    //   golden 的語意之後，迴圈邊界變成 iMaxRow/iMaxCol，缺口就變成承重的：
    //     全域物件（零初始化）-> iMaxRow=0 -> 迴圈空轉 -> NoIC() 恆 true
    //     區域物件（垃圾值）   -> Item[iMaxRow][iMaxCol] 越界 -> SegFault
    //   20260919 gate 兩種都實測到了（W7_GridOccupancy 是 SegFault，
    //   W7_C1/C2 與兩支 BarCode 是「有 IC 卻說沒有」）。
    //
    // ⚠ AI(W906-P1b) 20260920 更正：上面那段原本寫「golden 的 ctor 還設了
    //   iMotRow/... 那些是**另一批**不忠實，不在 P1 範圍內」。
    //   **那一批就是下面這一段，20260920 補完了。**
    //   ⇒ 那句「不在範圍內」已經過期，留著會讓人以為還有債沒還。

    //  ---- golden MyKitSuck.cpp:76-87，逐行、照順序 -------------------------
    iMotRow      = 1;                                                           // golden :76
    iMotCol      = 1;                                                           // golden :77
    iMaxRow      = 1;                                                           // golden :78
    iMaxCol      = 1;                                                           // golden :79
    //  golden :80 `iMaxCnt=1;` —— ⚠ **本樹的 TMyKitSuck 沒有 iMaxCnt 這個成員**
    //  （aHotPlateSubstrate.h 的門面只帶了被讀到的欄位）。全樹 0 個讀取點，
    //  所以不是「漏設值」而是「這個欄位還沒被帶進來」。
    //  ⇒ 哪一天有人要讀它，要先在 h 裡補宣告，再把這一行解開。
    iPickRow     = 1;                                                           // golden :81
    iPickCol     = 1;                                                           // golden :82
    iShtRow      = 1;                                                           // golden :83  ⚠ 移植樹原本是 2（無 golden 依據）
    iShtCol      = 1;                                                           // golden :84
    iShtCnt      = 1;                                                           // golden :85
    iPickStep    = 1;                                                           // golden :86  Pitch倍數, 13吸嘴就寫2, 14吸嘴就寫3
    iPickKitStep = 1;                                                           // golden :87  ⚠ 移植樹原本是 0
}

// ---- TMyKitSuck bodies (only the called methods) ----------------------------
void TMyKitSuck::ResetAll() {}                              // golden :289 -- reset SuckTask

// ===========================================================================
//  AI(W906-P1-HASIC) 20260919: 三支述詞改回 golden（RULINGS §C12 / 使用者裁決）
//
//  移植樹原本長這樣（三支都不是 golden 的定義）：
//      NoIC()      = !HasRealIC()
//      HasIC()     = 手寫迴圈，只認 HAS_IC || HAS_HOT_IC
//      HasRealIC() = **與 HasIC() 逐字相同的迴圈**
//  ⇒ HasIC 與 HasRealIC 在移植樹塌成同一個實作，而 golden 保證的是
//    `HasIC() == !NoIC()`，移植樹不保證。
//
//  漏掉的狀態（cmydef.h:154-170 的 Item 家族）：在 golden 的定義下，
//  **除了 NULL_IC 以外每一個都讓 HasIC() 回 true**；移植樹只有兩個會。
//  Steven 報的 clean IC（HAS_CLEAN_IC）正是被漏掉的其中一個。
//
//  ★ 這個檔自己就有兩支**寫對了**的對照組：HasRealIC_Left / HasRealIC_Right
//    （本檔 :531 / :557）就是 golden 的三值排除法逐字翻譯，連 `//wei 20160130`
//    都在。敘述與實作在同一個畫面裡互相矛盾，而敘述是對的（§C12.3）。
//
//  ⚠⚠ 扇出：HasIC 1,683 處 / HasRealIC 346 處 / NoIC 78 處 = 2,107 個判斷點。
//    方向是**變得更保守**（更多情況會認為「有 IC」而拒絕動作），安全上是對的，
//    但會讓一些現在跑得過的流程停下來。使用者已裁決承受。
//
//  ⚠ 迴圈邊界也改回 golden 的 `iMaxRow`/`iMaxCol`（原本是陣列上界
//    `_MAX_SUCK_ROW_ITEM`/`_MAX_SUCK_COL_ITEM` = 4x8）。
//    離線風險已量：TMyKitSuck 的 ctor 沒有初始化 Item[][] 與 iMaxRow/iMaxCol，
//    但那些物件是全域 ⇒ 零初始化，而 NULL_IC==0（樹裡多處實證，例
//    aoutarm9045.cpp:2449）。所以 iMaxRow==0 時 golden 的 NoIC() 走空迴圈回 true、
//    HasIC() 回 false —— 與改之前相同，離線不會突然變臉。
// ===========================================================================
bool TMyKitSuck::HasIC()                                                        // golden MyKitSuck.cpp:655
{
    return !NoIC();
}

bool TMyKitSuck::HasType(int IC_TYPE)
{
    for(int i=0;i<_MAX_SUCK_ROW_ITEM;i++)
        for(int j=0;j<_MAX_SUCK_COL_ITEM;j++)
            if(Item[i][j]==IC_TYPE) return true;
    return false;
}

// -- W6.4 ADD: tested-state predicates the test-cycle dispatcher (DoTestY case
//    50) derefs.  Faithful golden bodies (MyKitSuck.cpp :1663/:1685/:1701) -- the
//    AlreadyTest() side effect that clamps over-range bins to TEST_PASS+iTestBin
//    Count is preserved verbatim.
bool TMyKitSuck::AlreadyTest()                                                  // golden MyKitSuck.cpp:1663
{
    bool flag=false;                                                            //Steven 20180907 : 修正避免完全沒IC也回True
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]>=TEST_PASS)                                           // && Item[i][j]<(TEST_PASS+iTestBinCount))//Steven 20200507 : 修正Test time out之後按SKIP,機台會hang up
            {
                flag=true;
            }

            if(Item[i][j]>(TEST_PASS+iTestBinCount-1) &&
               Item[i][j]!=(TEST_PASS+iTestBinCount))                           //Steven 20121112 : RS232支援32Bin 14->iTestBinCount-1, 1016->TEST_PASS+iTestBinCount
            {
                Item[i][j]=TEST_PASS+iTestBinCount;
            }
        }
    }
    return flag;
}
bool TMyKitSuck::AlreadyTestNotIncludeErrorBin()                                //Steven 20200611 : for Murata, 2DID NG不測試  golden :1685
{
    bool flag=false;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]>=TEST_PASS && Item[i][j]<(TEST_PASS+iTestBinCount))
            {
                flag=true;
            }
        }
    }
    return flag;
}
bool TMyKitSuck::PartAlreadyTest()                                              // golden MyKitSuck.cpp:1701
{
    bool flag1=false, flag2=false;

    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_IC)
                flag1=true;

            if(Item[i][j]>=TEST_PASS)
                flag2=false;
        }
    }

    if(flag1 && flag2)
        return true;
    else
        return false;
}

// Carry-kit presence leaves: this is the W6.0-gated "KitSuck grid" predicate
// surface (csystem_predicates.cpp HT9045_KITSUCK_GRID_AVAILABLE).  Offline =
// "no IC present" -> conservative.
//AI(ht9045-v906) 20260629: W7 substrate staging -- replace conservative stub with faithful golden Item-scan (golden MyKitSuck.cpp:288). UseSiteHasIC true iff any use-site cell != NULL_IC over iShtRow x iShtCol. Empty grid (zero-init==NULL_IC) still returns false, behavior-identical to old stub.
bool TMyKitSuck::UseSiteHasIC()
{
    for(int i=0; i<iShtRow; i++)
        for(int j=0; j<iShtCol; j++)
            if(Item[i][j]!=NULL_IC)
                return true;
    return false;
}
//AI(ht9045-v906) 20260629: W7 substrate staging -- replace conservative stub with faithful golden Item-scan (golden MyKitSuck.cpp:297). UseSiteNoIC true iff every use-site cell == NULL_IC over iShtRow x iShtCol. Empty grid (zero-init==NULL_IC) still returns true, behavior-identical to old stub.
bool TMyKitSuck::UseSiteNoIC()
{
    for(int i=0; i<iShtRow; i++)
        for(int j=0; j<iShtCol; j++)
            if(Item[i][j]!=NULL_IC)
                return false;
    return true;
}
// W6.2b1x1: FAITHFUL golden MyKitSuck.cpp:273 -- "all use-sites carry an IC"
// (true iff no NULL_IC over the iShtRow x iShtCol shuttle grid).
bool TMyKitSuck::UseSiteFullIC()                                                //Ifor 20161215
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Item[i][j]==NULL_IC)
            {
                return false;
            }
        }
    }
    return true;
}
bool TMyKitSuck::LeftSideNoIC(int)           { return true;  }
bool TMyKitSuck::RightSideNoIC(int)          { return true;  }
bool TMyKitSuck::ArmUpSideNoIC()             { return true;  }
bool TMyKitSuck::ArmDownSideNoIC()           { return true;  }

void TMyKitSuck::SetItemData(int iRow, int iCol, int data, int /*iTarget*/)
{
    if(iRow<0||iRow>=_MAX_SUCK_ROW_ITEM||iCol<0||iCol>=_MAX_SUCK_COL_ITEM) return;
    Item[iRow][iCol]=data;
}

void TMyKitSuck::CopyFromTray(int iSuckR, int iSuckC, int iSuckData,
                              class TTrayMotor & /*Mot*/, int /*TrayR*/, int /*TrayC*/,
                              int /*iTrayData*/, int /*iTarget*/, bool /*bHP2Inarm*/)
{
    if(iSuckR<0||iSuckR>=_MAX_SUCK_ROW_ITEM||iSuckC<0||iSuckC>=_MAX_SUCK_COL_ITEM) return;
    Item[iSuckR][iSuckC]=iSuckData;                         // faithful: nozzle now carries iSuckData
}

void TMyKitSuck::CopyToTray(int iSuckR, int iSuckC, int iSuckData,
                            class TTrayMotor & /*Mot*/, int /*TrayR*/, int /*TrayC*/,
                            int /*iTrayData*/, int /*iTarget*/)
{
    if(iSuckR<0||iSuckR>=_MAX_SUCK_ROW_ITEM||iSuckC<0||iSuckC>=_MAX_SUCK_COL_ITEM) return;
    Item[iSuckR][iSuckC]=iSuckData;                         // faithful: nozzle cleared to iSuckData
}

// ---- W6.2b: members/methods the in-arm ENGINE SMs deref (golden MyKitSuck.h)
//      Offline: the grid is empty (no real vacuum line); predicates report
//      "no IC / finished" so the engine SMs take their deterministic path.
//AI(W906-P1-HASIC) 20260919: golden MyKitSuck.cpp:432 逐字。
// 三值排除法：HAS_NULL_IC / HAS_NULL_CLEAN_IC 算「空吸嘴」、NULL_IC 算「沒東西」，
// **其餘每一個狀態都算 real IC**。與本檔 :531/:557 的 _Left/_Right 同一個形狀。
// 詳細理由見上面 HasIC() 的橫幅。
bool TMyKitSuck::HasRealIC()                                                    // golden MyKitSuck.cpp:432
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j]==HAS_NULL_CLEAN_IC)                              //wei 20160130
                iHasNullICCount++;
            else if(Item[i][j]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
// -- W6.2c(OUT-ARM) batch-3 ADD: golden MyKitSuck.cpp:458/486 VERBATIM (pure Item
//    scan, no HAL). Left-half uses j2=j, right-half uses j2=j+Mid; HAS_NULL_IC /
//    HAS_NULL_CLEAN_IC / NULL_IC are not "real"; any other -> real IC present.
bool TMyKitSuck::HasRealIC_Left(int iRow, int Mid)                              //golden MyKitSuck.cpp:458
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    int j2;
    for(int i=0; i<iRow; i++)
    {
        for(int j=0; j<Mid; j++)
        {
            j2=j;
            if(Item[i][j2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j2]==HAS_NULL_CLEAN_IC)                             //wei 20160130
                iHasNullICCount++;
            else if(Item[i][j2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
bool TMyKitSuck::HasRealIC_Right(int iRow, int Mid)                             //golden MyKitSuck.cpp:486
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    int j2;
    for(int i=0; i<iRow; i++)
    {
        for(int j=0; j<Mid; j++)
        {
            j2=j+Mid;
            if(Item[i][j2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j2]==HAS_NULL_CLEAN_IC)                             //wei 20160130
                iHasNullICCount++;
            else if(Item[i][j2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
//AI(W906-P1-HASIC) 20260919: golden MyKitSuck.cpp:227 逐字。
// 原本是 `return !HasRealIC();` —— 那讓 NoIC/HasIC/HasRealIC 三支互相定義成
// 同一件事。golden 的 NoIC 是獨立的：**每一格都 NULL_IC** 才算沒有 IC。
bool TMyKitSuck::NoIC()                                                         // golden MyKitSuck.cpp:227
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]!=NULL_IC)
                return false;
    return true;
}
//AI(W906-T6-HASNOTTEST) 20260923: golden MyKitSuck.cpp:1032-1041 逐字（MainProc 停機臂要用）。
bool TMyKitSuck::HasNotTestYet()                                                // golden MyKitSuck.cpp:1032
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]==HAS_IC || Item[i][j]==HAS_HOT_IC)                    //ChungHung 20121002 alter
                return true;
    }
    return false;
}
bool TMyKitSuck::IsPickSuckFinish()    { return true; }     // offline: nothing left to suck
bool TMyKitSuck::IsPickDestroyFinish() { return true; }     // offline: nothing left to destroy
bool TMyKitSuck::IsPickFinish()        { return true; }     // offline: pick cycle done
void TMyKitSuck::ClearAll()
{
    for(int i=0;i<_MAX_SUCK_ROW_ITEM;i++)
        for(int j=0;j<_MAX_SUCK_COL_ITEM;j++)
            Item[i][j]=NULL_IC;
}
void TMyKitSuck::SetAllToNullIC()      { ClearAll(); }       // golden :286

// ---------------------------------------------------------------------------
//  TMyKitSuck::SetAllHASIC2ErrorBin -- golden MyKitSuck.cpp:2777-2792.
//  **完整翻譯，零閘** —— 它的五個相依（iMaxRow / iMaxCol / Item[][] /
//  SetItemData / iBinData / bPass / bNeedReTest / iTestBinCount）在本樹都在。
//
//  ⚠ 它**會改變料的判定**：把每一顆 HAS_IC 直接寫成 TEST_PASS+iTestBinCount，
//    也就是最後那個 bin（慣例上的「錯誤 bin」）。這不是顯示，是資料。
//    唯一的呼叫端是 uhome.cpp ProcessMotorHome case 2 的 CC_SCK 臂，
//    而那一臂本來就只在那家客戶的機台上成立。
// ---------------------------------------------------------------------------
void TMyKitSuck::SetAllHASIC2ErrorBin()
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_IC)
            {
                SetItemData(i, j, TEST_PASS+iTestBinCount);                     //Steven 20150203 : Fixed for Sucker Status
                iBinData[i][j]=iTestBinCount;
                bPass[i][j]=false;
                bNeedReTest[i][j]=false;
            }
        }
    }
}

// -- W906-AutoCleanCluster ADD: golden MyKitSuck.cpp:238/306/323 verbatim. -----
#ifndef HT9045_KITSUCK_AUTOCLEAN_CLUSTER_BODIES
#define HT9045_KITSUCK_AUTOCLEAN_CLUSTER_BODIES
bool TMyKitSuck::FindNoIC()                                                     //kevin 20120531
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]==NULL_IC)
                return true;
    return false;
}
bool TMyKitSuck::ArmAll_HasICType(int IC_TYPE1, int IC_TYPE2)                   //Steven 20250420 : fixed for auto clean
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]!=IC_TYPE1 && Item[i][j]!=IC_TYPE2)
                return false;
        }
    }
    return true;
}
bool TMyKitSuck::ShtAll_HasICType(int IC_TYPE1, int IC_TYPE2)                   //這個能用在In / out arm上
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Item[i][j]!=IC_TYPE1 && Item[i][j]!=IC_TYPE2)
                return false;
        }
    }
    return true;
}
#endif

// -- W6.2c(OUT-ARM) ADD: golden MyKitSuck.cpp:369 (Steven 20241017) verbatim.
//    Clears unused nozzles (beyond iShtRow/iShtCol) + maps HAS_NULL_IC->NULL_IC.
#ifndef HT9045_KITSUCK_SETUNUSE_BODY
#define HT9045_KITSUCK_SETUNUSE_BODY
void TMyKitSuck::SetUnuseAndHasNullICToNullIC()                                 //Steven 20241017 : 清除沒用到的料 與 HAS_NULL_IC的料
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(i>=iShtRow ||
               j>=iShtCol ||
               Item[i][j]==HAS_NULL_IC)
            {
                SetItemData(i, j, NULL_IC);
            }
        }
    }
}
#endif

// -- W6.2b(2x4_16) ADD: golden TMyKitSuck methods the 2x4_16 in-arm SMs call.
//    Guarded so a parallel sibling variant that adds the same body does not
//    produce a duplicate definition.
#ifndef HT9045_KITSUCK_2x4_16_BODIES
#define HT9045_KITSUCK_2x4_16_BODIES
void TMyKitSuck::SetAll(int Type)                                              // golden MyKitSuck.cpp ::SetAll
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            SetItemData(i, j, Type);
}
bool TMyKitSuck::All_HasIC()                                                   // golden MyKitSuck.cpp ::All_HasIC (UI bLed/pLed gate dropped offline)
{
    for(int i=0; i<iMaxRow; i++)
        for(int j=0; j<iMaxCol; j++)
            if(Item[i][j]==NULL_IC)
                return false;
    return true;
}
void TMyKitSuck::SetType1ToType2ByPickCol(int Type1, int Type2)               // golden-by-name (2x4_16 dead-by-design; faithful remap)
{
    for(int i=0; i<iPickRow; i++)
        for(int j=0; j<iPickCol; j++)
            if(Item[i][j]==Type1)
                SetItemData(i, j, Type2);
}
#endif
// -- W6.3 ADD: tray-arm-touched TMyKitSuck predicates (golden MyKitSuck.h) ------
//    Offline: the tray-arm shuttle-side suck/destroy have no real vacuum line, so
//    both report "finished" -> DoCatchTray's early-out guard (golden :6017-6018)
//    takes its deterministic "not waiting on suck/destroy" branch.
bool TMyKitSuck::IsShtSuckFinish()     { return true; }      // golden :337
bool TMyKitSuck::IsShtDestroyFinish()  { return true; }      // golden :338

// -- W6.5 ADD: shuttle clean-out / realIC->interface-bin TMyKitSuck methods -----
//    FAITHFUL translations of the golden MyKitSuck.cpp bodies (NOT stubs): they
//    walk Item[i][j] over [iMaxRow][iMaxCol] -- both members already on this
//    substrate.  Offline the global carry-kits have iMaxRow==iMaxCol==0 (value-
//    initialized) so the loops naturally no-op: CountRealIC->0, HasDefineIC->false,
//    SetAllRealIC2InterfaceBin->no write -- the deterministic empty-grid result.
//    With a populated grid (a test seeding iMaxRow/iMaxCol+Item) the bodies behave
//    exactly as golden.
void TMyKitSuck::SetAllRealIC2InterfaceBin()                                    // golden MyKitSuck.cpp:SetAllRealIC2InterfaceBin
{
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]!=NULL_IC && Item[i][j]!=HAS_NULL_IC)
            {
                SetItemData(i, j, TEST_PASS+iTestBinCount);                     //Steven 20150203 : Fixed for Sucker Status
                iBinData[i][j]=iTestBinCount;
                bPass[i][j]=false;
                bNeedReTest[i][j]=false;
            }
        }
    }
}
//------------------------------------------------------------------------------
int TMyKitSuck::CountRealIC()                                                   // golden MyKitSuck.cpp:CountRealIC
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(Item[i][j]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    return iOtherCount;
}
//------------------------------------------------------------------------------
bool TMyKitSuck::HasDefineIC(int IC_TYPE)                                       //Steven 20130620 : 增加Function  (golden MyKitSuck.cpp:HasDefineIC)
{
    bool bHasDefineIC=false;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<iMaxCol; j++)
        {
            if(IC_TYPE==NULL_IC && Item[i][j]==IC_TYPE)                         //Steven 20160530 : fixed for HasDefineIC
            {
                bHasDefineIC=true;
            }
            else
            {
                if(Item[i][j]!=NULL_IC)
                {
                    if(Item[i][j]==IC_TYPE)
                    {
                        bHasDefineIC=true;
                    }
                }
            }
        }
    }
    return bHasDefineIC;
}
#endif  // AI(W906-A4-6) 20260924
//==============================================================================
//  (e) TMyProductionRecord bodies the leaves call. A same-day sibling wave
//      (AI(W906-MyProductionRecord) 20260721) gave Public/MyProductionRecord.cpp
//      real bodies for the pure-logic half of this class (ctor included) --
//      reconciled here: the 8 stand-ins that wave now defines for real were
//      removed from this file (ctor, AddHPRecord, AddInArmHotplatePickRecord,
//      GetInRotationAngRecord, AddPickCleanPad, AddPickCleanPadFormShuttle,
//      AddPlaceCleanPad, Add2DIDRecord) to avoid a multiple-definition link
//      error; only the 3 the real .cpp deliberately still leaves undefined
//      (the heavy Save*/upload half + one golden-dead method) keep their
//      offline no-op here.
//==============================================================================
// AI(W906-W2-PRODLOG) 20260926: AddErrorRecord 空替身退役 —— 真本體在 Public/MyProductionRecord.cpp（GATE G-5 同一顆 commit 解開；兩邊只能留一個，否則重複定義）
void TMyProductionRecord::AddTestRecord(int, int) {}               // W6.2b(2x4_16): golden dead method; offline no-op
// AI(W906-W2-CLEANPADLOG) 20260926: SaveRecordCleanPad 空替身退役 —— 真本體在 Public/MyProductionRecord.cpp（GATE G-6 同一顆 commit 解開）

//==============================================================================
//  IsFLCarrKitAllHasIC / IsBLCarrKitAllHasIC -- EXPORTED by
//  ainarm_SearchPickPlate.h but DEFINED in the golden ainarm core
//  (ainarm9045.cpp / acarry.cpp, W6.x/W7).
//  AI(W6.5-CARRY) 20260626: the W6.5 wave landed acarry.cpp, which carries the
//  REAL golden bodies (acarry.cpp:175 / :197).  Removed the W6.2 placeholder
//  stubs here to resolve the multiple-definition (the real bodies now own them).
//==============================================================================
#if 0 // W6.5: superseded by the real bodies translated into acarry.cpp (:175/:197)
bool IsFLCarrKitAllHasIC() { return false; }                       //ChungHung 20111230
bool IsBLCarrKitAllHasIC() { return false; }                       //ChungHung 20111230
#endif

//==============================================================================
//  (b) uPoint2D + uPlateInfo (PickFromHPList) -- empty offline team list
//==============================================================================
//==============================================================================
//  AI(W906-PT-W2-integrate) 20260807: THE uPlateInfo STAND-IN IS RETIRED HERE.
//
//  What stood in this space was an offline stub of uPlateInfo -- 15 members, all
//  `return false;` / no-op, plus the four globals PickFromHPList /
//  PlaceToCleanList / sHPPickRec / sHPPickRecException.  It existed because
//  uPlateInfo's real golden home, Public/HTEditList.cpp, had no port.
//
//  It does now: wave PT-W2 translated Public/HTEditList.cpp, which defines all
//  32 uPlateInfo members -- a strict SUPERSET of the 15 stubbed here -- and the
//  same four globals.  Keeping both produced exactly what you would expect:
//  `multiple definition of uPlateInfo::...` from the linker, in every executable
//  that pulls both objects out of libht9045_sm.a.
//
//  The stub is deleted rather than the real one because this is the direction
//  the whole port moves in: a SATISFIED-BY-SUBSTRATE entry is a placeholder that
//  retires the moment the golden unit lands.  (aoutarm9045.cpp:175 carries the
//  same note for its five SortingBinTray statics, still pending.)
//
//  BEHAVIOUR DELTA, STATED PLAINLY: every caller that used to get the offline
//  "team list is always empty" answer -- GetHPFirstTeam* -> false, ExtractFirstTeam
//  -> NULL, the Set/Add/Update/Save/Clear family -> no-op -- now runs golden's
//  real list logic against a genuinely empty list at startup.  That is the
//  intended direction (real code, real data), but it is a real change: any test
//  that was pinned to the stub's unconditional false is now pinned to golden's
//  behaviour instead.
//
//  uPoint2D's three bodies are retired with it.  (An earlier draft of this note
//  claimed uPoint2D "stays here" because golden declares it in HTEditList.h --
//  wrong: golden puts the DECLARATION in the .h and the BODIES in
//  HTEditList.cpp:3118-3134, which the port translates faithfully at
//  Public/HTEditList.cpp:2712-2724.  The linker said so before this note did.)
//
//  WHAT THE STUB WAS ALSO SILENTLY DOING -- AND WHY THIS BLOCK EXISTS.
//  Retiring the stub bodies turned three green tests (AutoClean,
//  W6_2_InArmCanary, W6_2_InArmSearch) into SEGFAULTs, which is the useful part:
//  the stub had been hiding a real hole.  Golden declares both globals as bare
//  uninitialised pointers -- Public/HTEditList.cpp:56 `uPlateInfo *PickFromHPList;`
//  and :59 `uPlateInfo *PlaceToCleanList;` -- and CONSTRUCTS them somewhere else
//  entirely: main.cpp:2149-2150, `PickFromHPList = new uPlateInfo();` /
//  `PlaceToCleanList = new uPlateInfo();`.  The port translates :56/:59
//  faithfully (Public/HTEditList.cpp:204/:207), so both are NULL -- and main.cpp
//  is one of the 163 golden units with no port at all yet, so nothing ever runs
//  those two lines.  Every `PickFromHPList->...` in the ported ainarm9045_*
//  family was therefore a NULL dereference waiting to happen; the old stub
//  concealed it by pointing the names at its own file-static objects.
//
//  This block does what golden main.cpp:2149-2150 does, and nothing more.  It
//  is a stand-in for an unported unit, marked as such, and it retires the moment
//  main.cpp lands -- exactly like LoadMachineConfig() in database.cpp, which
//  stands in for golden's SYSTEM_MODULAR ctor for the same reason.
//
//  ON STATIC-INITIALISATION ORDER: standard C++ gives no cross-TU ordering
//  guarantee, so this is only safe because both pointers are read from ordinary
//  runtime functions (the arm state machines), never from another TU's static
//  initialiser -- checked, not assumed.  Doing it here rather than by
//  initialising the pointers at their definition keeps golden's own :56/:59
//  declarations byte-identical.
//==============================================================================
namespace {
struct HPListBootstrap
{
    HPListBootstrap()
    {
        // AI(W906-BA-MW6) 20260911: was `new uPlateInfo()`, which from THIS TU's
        // view of the facade allocated 1 byte for a 24-byte constructor -- twice
        // per boot. The factory allocates in the TU that sees the real class.
        if(PickFromHPList  == NULL) PickFromHPList  = W906_NewPlateInfo();  // golden main.cpp:2149
        if(PlaceToCleanList== NULL) PlaceToCleanList= W906_NewPlateInfo();  // golden main.cpp:2150
    }
};
HPListBootstrap g_hpListBootstrap;
}
//==============================================================================

//==============================================================================
//  (c) ainarm2.h cursors / arrays (golden ainarm2.h) -- definitions
//==============================================================================
const bool ZAxisDown = true;                               // golden ainarm2.h:42 (Z 下降)

int iPickPlate [2] = {0,0},  iPickPlateX [2] = {0,0},  iPickPlateY [2] = {0,0};
int iPlacePlate[2] = {0,0},  iPlacePlateX[2] = {0,0},  iPlacePlateY[2] = {0,0};

// AI(W906-AutoCleanFoundation) 20260721: golden ainarm2.cpp:91/101/105-107 --
// verified genuinely absent from the target tree (grepped) before adding.
int  iAutoCleanStart = 0;                                   // golden ainarm2.cpp:91
int  iAutoCleanPickPlateX;                                  // golden ainarm2.cpp:105 (int-static default-0)
int  iAutoCleanPickPlateY;                                  // golden ainarm2.cpp:106
int  iAutoCleanUseXPitch = 0;                               // golden ainarm2.cpp:107
bool bInArmSuckActive[MAX_ARM_Row][MAX_ARM_Col] = {{false,false,false,false},{false,false,false,false}}; // golden ainarm2.cpp:101
bool bPlaceToCleanKit = false;                              // golden ainarm2.cpp:49
// AI(W906-AutoCleanCluster) 20260722: golden ainarm2.cpp:92 -- verified genuinely
// absent from the target tree (grepped) before adding.
int  iAutoCleanNum = 0;                                     // golden ainarm2.cpp:92

bool InArmSuckUse[MAX_ARM_Row][MAX_ARM_Col]          = {{false}};
bool bPickFromHotplate                               = false;
bool bInArmCheckDestroyACT[MAX_ARM_Row][MAX_ARM_Col] = {{false}};

int iHotCount = 0;                                         // golden ainarm2.h:77
int iHotPlateCount [2][50][50] = {{{0}}};
int iHotWhichKit   [2][50][50] = {{{0}}};
int iHotWhichShuttle[2][50][50]= {{{0}}};

bool bPitchOver12000 = false;                              // golden ainarm2.h:162
//AI(W6.2c-INARM-batch3) 20260626: single definition of the ainarm2 debug latch
// (golden ainarm2.cpp:52).  Cleared by DoInArm_9045_2x3_6_14 / _2x4_4; offline
// default false, no hardware side effect.  extern in aHotPlateSubstrate.h.
bool bInArmHasHotIC = false;                               // golden ainarm2.cpp:52 (Sam 20211012)

strAUTOSITEMAP InArmSiteMapData = { -1, -1, -1, -1, -1 };  //Steven 20211209 : 紀錄Site map資料

//==============================================================================
//  (e) Rotate / Laser micro-shims
//==============================================================================
tRotateShim tRotate = {};   //AI(W906-FRW-NoPage) 20260926: 原本 `= { false }`（只有 ActiveRotate 一個初值）；tRotateShim 改成 golden 完整 TRotate（forms/fRotate.h）後
                            //  成員從 3 個變 25 個，`{ false }` 會對其餘每個成員各報一次 -Wmissing-field-initializers。`{}` 是同一個值（全部零初始化、ActiveRotate=false）

TInLaserCheck::TInLaserCheck(int _iP, int _iPlateC, int _iPlateR)
    : iP(_iP), iPlateC(_iPlateC), iPlateR(_iPlateR) {}
// AI(W906-PT-W3-integrate) 20260808: the stand-in DEFINITION of LaserCheckPos that
//   used to sit here (`std::vector<TInLaserCheck*> LaserCheckPos;`) is RETIRED.
//   golden defines it in OmronLaser/LaserSensorInArm.cpp:32 and that unit landed in
//   PT-W3 (port :111), so both definitions were in libht9045_sm.a and every
//   executable linking it failed with `multiple definition of 'LaserCheckPos'`.
//   The declaration in aHotPlateSubstrate.h:845 STAYS: ~30 ainarm* leaves reach
//   LaserCheckPos through this header alone and none of them may be edited here.
//
//   ⚠ ODR DEBT, DELIBERATELY LEFT AND WRITTEN DOWN -- this is the same shape as
//   the TMyKitSuck trap this wave recorded, and it needs its own wave to close.
//   `class TInLaserCheck` is defined TWICE with DIFFERENT MEMBER NAMES:
//     aHotPlateSubstrate.h:836-844   iP / iPlateC / iPlateR   (out-of-line ctor, above)
//     OmronLaser/LaserSensorInArm.h:34-46  iPlate / iX / iY   (inline ctor)
//   Why it is not silently wrong TODAY (checked field by field, not assumed):
//   both are exactly three `int`s in the same declaration order, and the producers
//   and consumer agree semantically -- the ainarm leaves push
//   `new TInLaserCheck(plate, col, row)` and LaserSensorInArm.cpp reads
//   `iPlate / iX / iY`, and in this codebase X is the column and Y the row.  So the
//   layouts coincide and the fields line up.  It is still an ODR violation that
//   LINKS CLEANLY, i.e. exactly the class of defect that stays invisible until a
//   member is added to one copy.  Closing it means the ~30 leaves taking
//   OmronLaser/LaserSensorInArm.h instead -- a substrate-homecoming wave, sibling
//   to the mykitsuck.cpp one (see CMakeLists.txt's PT-W3 note).

//==============================================================================
//  per-site close-site-mode selectors (golden ainarm9045_2x6_8.h / _2x8_8.h)
//==============================================================================
//AI(W6.2c-INARM-batch4) 20260626: the placeholder DEFINITIONS of iCloseSiteModeFor2x6
// (was =0, e2x6Standard) and iCloseSiteModeFor2x8 (was =0, e2x8Standard) were REMOVED.
// The per-site variants ainarm9045_2x6_8.cpp (iCloseSiteModeFor2x6=e2x6Standard) and
// ainarm9045_2x8_8.cpp (iCloseSiteModeFor2x8=e2x8Standard) now own the REAL defs; the
// extern decls in aHotPlateSubstrate.h satisfy consumers (acarry.cpp / ainarm2.cpp).
// Values identical (both e*Standard==0) so numeric behavior is unchanged.
//==============================================================================
//  XPHSuckToSht_2x8_8_OutArm (golden aoutarm9045_2x8_8.h:7 / aoutarm9045_2x8_8.cpp:332,
//  JerryYang 20250711).
//AI(W6.2c-OUT-batch3) 20260627: ownership TRANSFERRED -- the out-arm 2x8_8 wave has
//  landed ACTIVE this batch and aoutarm9045_2x8_8.cpp now OWNS the REAL golden def
//  (full special-關SITE pick map). Removed the offline zero-init placeholder here to
//  avoid a multiple-definition link error; the extern in aHotPlateSubstrate.h still
//  serves the substrate-only in-arm consumers (ainarm9045.cpp / inarm 2x8_8 family).
//==============================================================================

//==============================================================================
//  (A) [W6.2b] in-arm ENGINE cursors owned by ainarm2.cpp (not-yet-translated).
//      init to 1 (golden InitInArmTask()).  Defined here so ainarm9045.cpp's
//      central pick SM (`int &Task=iPickFromLoadStageTask`) and DoInArm_9045
//      (reads iArmTask) link.  Golden ainarm2.h:97/101.
//==============================================================================
int iArmTask               = 1;
int iPickFromLoadStageTask = 1;
bool bPickFromLoader       = false;     //golden ainarm2.h:54

//==============================================================================
//  (A2) [W6.2c] OUT-ARM ENGINE cursors owned by aoutarm2.cpp (not-yet-translated).
//      init to 1 (golden InitOutArmTask/InitPlaceToAutoTask/InitialFix3CanFullTask).
//      Defined here so aoutarm9045.cpp's DoOutArmPlaceToAuto_9045
//      (`int &Task=iPlaceToAutoTask`) and DoOutArmAfterPlaceToAuto link.
//      Golden aoutarm.h:49/15/51.
//==============================================================================
// AI(W906-PT-W4-integrate) 20260809: 3 STAND-IN DEFINITION(S) RETIRED FROM HERE.
//   aoutarm.cpp (golden's own home for all of them) landed in wave PT-W4 and is
//   registered in ht9045_sm, so both definitions were in libht9045_sm.a and every
//   executable linking it failed with `multiple definition of ...`. The linker named
//   each one, which is also the proof the signatures match exactly -- a decorated-name
//   collision cannot happen otherwise.
//   Retired here: iPlaceToAutoTask, iDoOutArmAfterPlaceToAutoTask, iFix3CanFullTask
//   BEHAVIOUR: these were offline defaults (return true/false/0/no-op); the real bodies
//   run golden's actual logic, so out-arm paths that used to short-circuit now execute.
//   That is the point of the wave, and it is why this wave was measured on its own.

//==============================================================================
//  (B) [W6.2b] per-variant close-site selector for 1x4 (golden ainarm9045_1x4_4.h)
//==============================================================================
//AI(W6.2c-INARM-batch3) 20260626: definition moved to ainarm9045_1x4_4.cpp:111
// (`int iCloseSiteModeFor1x4=e1x4Standard;`) -- that variant landed ACTIVE this
// batch and now OWNS the symbol.  Removed the duplicate substrate definition to
// avoid a multiple-definition link error; the extern in aHotPlateSubstrate.h:531
// still serves the substrate-only consumers (ainarm9045.cpp / inarm tests).

//==============================================================================
//  (D) fBarCode -- AI(W906-BC-READFILE) 20260923: the class is now TfBarCode and
//      its bodies MOVED to BarCode/BarCode.cpp, golden's own file for them.
//      Same library (ht9045_sm), same object, same forwarding targets -- this
//      TU simply no longer owns them.  Declaration: BarCode/BarCode.h, reached
//      from aHotPlateSubstrate.h exactly where the class used to be declared.
//==============================================================================

//==============================================================================
//  HP pick/place "part OK" / retry flags.  These are EXPORTED by
//  ainarm_SearchPickPlate.h but the golden DEFINES them in ainarm9045.cpp (the
//  in-arm core, W6.x/W7).  Define them here for W6.2 so the leaf modules link.
//  AI(W6.2-INARM) 20260626.  Golden homes: ainarm9045.cpp.
//==============================================================================
bool bPickFormHotplatePartOK         = false;   //ChungHung 20120112
bool bInArmToPickHotPlatePos         = false;   //ChungHung 20110323
bool bPickFormHotplateRetry          = false;   //Ifor 20160616
bool bInArmTryPickFromHotPlateFinish = false;   //ChungHung 20120206

//==============================================================================
//  (d) in-arm engine helper stubs (real bodies live in the later ainarm core; the 2-arg MyDBIProcess below is a live adapter since P4, ★W56;
//      W6.x/W7).  Offline defaults keep the geometry leaves linkable and the
//      canary deterministic.
//==============================================================================
//  [W6.2b PRE-STEP] 11 stubs REMOVED from here -- ainarm9045.cpp now REAL-defines
//  them (avoid ODR/link collision):
//    bUseAxExPicker, bUseAxxGPicker, InArmLeftSideHasIC, InArmLeftSideNoIC,
//    InspectInArmPosition, IsCheckInArmDestroyActiveFinish,
//    DoInArm_9045_SuckerMap, SetShuttleToHasNullIC_9045, AddInArmPickerCount,
//    GetInArmPitchX_9045, GetInArmPitchY_9045.
//  The remaining stubs below stay -- their real bodies live in not-yet-translated
//  modules (ainarm2.cpp / cmydef.cpp / motor speed ctrl, W7).
//==============================================================================
#if 0   // PT-W7e-part2 RETIRED (ResetShuttleWhichKit): real translated body now in ainarm2.cpp
void ResetShuttleWhichKit() {}                             //golden ainarm2.h:133
#endif
#if 0   // PT-W7e-part2 RETIRED (CloseSiteState): real translated body now in ainarm2.cpp
int  CloseSiteState(bool /*bPlace*/) { return 0; }         // no site closed offline
#endif
// RowCanDualSite (golden ainarm2.cpp:1554, ChungHung 20150528).  W6.2c batch-4: the
// 2x2_8_Hot place-to-HP SM derefs it (GetPlaceHotPlate_4/_8/_8All).  The golden body
// reads TestIF.iTestMode/iUseSuckMode + ArmCanSuck4IC(0) + HotPlateForm.XDivision and
// returns false on every branch EXCEPT the ArmCanSuck4IC(0)==false early-out (which
// the Sim HAL does not model); 3 of 4 returns are false.  Offline: false (the
// dominant/faithful outcome -- no dual-site over the Sim Suck grid).
#if 0   // PT-W7e-part2 RETIRED (RowCanDualSite): real translated body now in ainarm2.cpp
bool RowCanDualSite() { return false; }                    //golden ainarm2.cpp:1554
#endif
#if 0   // PT-W7e-part2 RETIRED (InitInArmTask): real translated body now in ainarm2.cpp
void InitInArmTask()  {}
#endif
// AI(W906-P10) 20260921: 這個樁**退休了** —— 真本體在新的 TU
//   `RunStartMode.cpp`（golden main.cpp:363-1115，753 行）。
//   為什麼不留在這裡：golden 本體要 22 個本檔看不到的名字，
//   塞進來等於給本檔加約 15 個 include。理由與前例寫在那個檔的檔頭。
//   ⚠ 宣告仍在 aHotPlateSubstrate.h:929，所以 36 個呼叫端一行都沒動。
#if 0   // PT-W7e-part2 RETIRED (TransferHotPlateRatio): real translated body now in ainarm2.cpp
void TransferHotPlateRatio(bool, int *, int *) {}          //Steven 20110324 : ratio xform no-op
#endif
#if 0   // PT-W7e-part2 RETIRED (MoveInArmZToPlateSafe): real translated body now in ainarm2.cpp
bool MoveInArmZToPlateSafe(int) { return true; }           // offline: Z reaches safe immediately
#endif
#if 0   // PT-W7e-part2 RETIRED (AdjustShuttlePlaceOrder): real translated body now in ainarm2.cpp
void AdjustShuttlePlaceOrder(int) {}
#endif
#if 0   // PT-W7e-part2 RETIRED (SetInArmHome): real translated body now in ainarm2.cpp
void SetInArmHome(bool /*bPrecisorNeedHome*/) {}   // AI(pt-wave) 20260811: golden arity (ainarm2.h:136); still a no-op stub, golden real body ainarm2.cpp:1266 not yet translated
#endif
#if 0   // PT-W7e-part2 RETIRED (InArmSubSpeed): real translated body now in ainarm2.cpp
void InArmSubSpeed() {}
#endif
#if 0   // PT-W7e-part2 RETIRED (InArmAddSpeed): real translated body now in ainarm2.cpp
void InArmAddSpeed() {}
#endif
void StopAllMotor() { extern void StopAllMotor(bool bIndexCanStop); StopAllMotor(true); }  // AI(W906-STOPALL) 20260924: 空殼改轉呼叫已翻好的真本體（Motor/myGALILmotor.cpp，golden Motor/myGALILmotor.cpp:4712；golden 的 no-arg 呼叫就是預設參數 true）。使用者 20260924 裁決「統一，全部呼叫點照 golden 真停」—— 在這之前 PAUSE（WebStart.cpp:3841）、ckernel、csystem 等約 20 個呼叫點都只呼叫到空殼
// AI(W906-CMYDB-P4) 20260927 (St02-E), Steven P4 D1=A: the 2-arg MyDBIProcess is an ADAPTER now -- golden 906 has
//   no 2-arg overload (its 2-argument calls bind the 3-arg one with S2 = "", golden cMyDB.h:20), so this forwards
//   (S1, S2) as (asTable=S1, S1=S2, S2="") to the golden body in cMyDB.cpp (golden cMyDB.cpp:789-855; ht9045_db).
//   The W906_* observation seam stays (test_FTPClient_EventHandlers and others read it).  Was a counting no-op.
int        W906_MyDBIProcess_Count = 0;
AnsiString W906_MyDBIProcess_LastS1;
AnsiString W906_MyDBIProcess_LastS2;
void W906_MyDBIProcess_Reset() { W906_MyDBIProcess_Count=0; W906_MyDBIProcess_LastS1=""; W906_MyDBIProcess_LastS2=""; }
extern void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2);   // golden cMyDB.h:20 (no default: this TU also sees the 2-arg one)
void MyDBIProcess(AnsiString S1, AnsiString S2)
{
    W906_MyDBIProcess_Count++;  W906_MyDBIProcess_LastS1=S1;  W906_MyDBIProcess_LastS2=S2;
    MyDBIProcess(S1, S2, AnsiString(""));                                      // golden: a 2-arg call = (asTable, S1, "")
}

//==============================================================================
//  (E) [W6.2b1x1] ainarm2.h engine shims the in-arm per-site VARIANT SMs call.
//      Shared across every in-arm variant; defined ONCE here.  golden homes
//      cited per symbol.  Cursor-init + small row-scan helpers are FAITHFUL
//      translations; HAL/UI-bound bodies are conservative offline stubs whose
//      terminal value keeps the variant SM cursor flowing (never hangs).
//      AI(W6.2b-INARM-1x1_1) 20260626.
//==============================================================================

// -- data ---------------------------------------------------------------------
// AI(W906-A4-6) 20260924 退役（mykitsuck.cpp 定義）：TMyKitSuck  ptrInSHTBackup;              // golden MyKitSuck.h:380
// AI(W906-A4-6) 20260924 退役（mykitsuck.cpp 定義，golden 預設 NULL，執行期由 AutoClean 指派）：TMyKitSuck *ptrInSHT = &FLCarryKit;
                                         //   GetShuttleState_1x1_1's ptrInSHT->ArmUpSideAllTypeIC() is non-null offline.
// AI(W906-A4-6) 20260924 退役（mykitsuck.cpp 定義）：TMyKitSuck  OutArmSuckBackup;            // golden MyKitSuck.h (SetInOutArmParameter_* target)
int  iInXPToSht[X_PITCH_COUNT]                = {0};   // golden ainarm2.h:36
int  iZPosToSht[MAX_ARM_Row][MAX_ARM_Col]     = {{0}}; // golden ainarm2.h:37
bool bZFlgToSht[MAX_ARM_Row][MAX_ARM_Col]     = {{false}}; // golden ainarm2.h:38
TQPF_Timer InArmReleaseDelayToHot;       // golden ainarm2.h:45
TQPF_Timer InArmReleaseDelay;            // golden ainarm2.h:46
TQPF_Timer MyInArmAtShuttleTimer;        // golden ainarm2.h:47
TQPF_Timer hInArmYpitchHomeTimer;        // golden ainarm2.h:212 (offline: TQPF_Timer, see .h note)
// -- W6.2c(OUT-ARM) ADD: out-arm sibling of hInArmYpitchHomeTimer.  Golden
//    aoutarm.h:213 (HTimer); exposed offline as TQPF_Timer (same Off()/
//    SetSecAndOn() surface), mirroring the in-arm precedent above.  Single
//    definition; the 6 out-arm variants forward-declare `extern TQPF_Timer`.
TQPF_Timer hOutArmYpitchHomeTimer;       // golden aoutarm.h:213 (offline: TQPF_Timer)
int  iBackInArmHotCount  = 0;            // golden ainarm2.h:76
bool InArmXMoveSafe      = false;        // golden ainarm2.h:83
bool bPlaceToShuttle2Step = false;       // golden ainarm2.h:56
int  iInArmPlaceToHotPlateTask    = 1;   // golden ainarm2.h (cursor; InitInArmPlaceToHotPlateTask sets 1)
int  iInArmPlaceToShuttleTask     = 1;   // golden ainarm2.h (cursor; InitInArmPlaceToShuttleTask sets 1)
int  iInArmTryPickFromHotPlateTask = 1;  // golden ainarm2.h (cursor; InitInArmTryPickFromHotPlateTask sets 1)
int  iBackupPlate = 0, iBackupPlateC = 0, iBackupPlateR = 0; // golden ainarm2.h (HP-check backup pos)
// flag1_1x2_2[2] (golden ainarm2.cpp:41 / ainarm2.h:9): plain process-state flag
// pair shared by the 1x2_2 / 1x2_2_14 / 1x2_4_Hot in-arm SMs (HotPlate XDivision==3
// path).  Offline-safe: zero-init, no hardware.  AI(W6.2c-INARM) 20260626.
bool flag1_1x2_2[2] = {false,false};                          // golden ainarm2.cpp:41

// -- TMyKitSuck methods (golden MyKitSuck.cpp) --------------------------------
#if 0   // AI(W906-A4-6) 20260924: 精簡鏡像的 TMyKitSuck 方法與 CopyInitSuck 退役（mykitsuck.cpp 有 golden 版）。原註：FAITHFUL: small HAL-free row scan (golden MyKitSuck.cpp:881).
bool TMyKitSuck::ArmUpSideAllTypeIC(int IC_TYPE, int iOffset, int iCol)         //Steven 20220930
{
    for(int j=0; j<iCol; j++)
    {
        if(Item[0][j+iOffset]!=IC_TYPE)
            return false;
    }
    return true;
}
// FAITHFUL: twin of ArmUpSideAllTypeIC, scans the down-side row Item[1][..]
// (golden MyKitSuck.cpp:971, Steven 20220930).  AI(W6.2c-INARM-batch2) 20260626.
bool TMyKitSuck::ArmDownSideAllTypeIC(int IC_TYPE, int iOffset, int iCol)       //Steven 20220930
{
    for(int j=0; j<iCol; j++)
    {
        if(Item[1][j+iOffset]!=IC_TYPE)
            return false;
    }
    return true;
}
//----------------------------------------------------------------------------
//AI(W6.2c-INARM-batch3) 20260626: golden MyKitSuck.cpp:514 (Steven 20221005).
// FAITHFUL per-row "has the specified IC?" query -- pure Item[iRow][*] scan, no
// HAL.  Added for GetShuttleState_1x4_4 / _2x3_6 / _All_1Pick (batch-3) which
// call InArmSuck.RowHasDefineIC(0/1, NULL_IC).  Body verbatim from golden.
bool TMyKitSuck::RowHasDefineIC(int iRow, int IC_TYPE)                          //Steven 20221005 : 針對上下排判斷有沒有指定的IC
{
    bool bHasDefineIC=false;
    for(int j=0; j<iMaxCol; j++)
    {
        if(IC_TYPE==NULL_IC && Item[iRow][j]==IC_TYPE)
        {
            bHasDefineIC=true;
        }
        else
        {
            if(Item[iRow][j]!=NULL_IC)
            {
                if(Item[iRow][j]==IC_TYPE)
                {
                    bHasDefineIC=true;
                }
            }
        }
    }

    return bHasDefineIC;
}
// FAITHFUL Item-grid side-scans (golden MyKitSuck.cpp:730/:768/:853/:943).
// Used by the 1x2_4_Hot in-arm place-to-shuttle SM (DoInArmPlaceToShuttle).
// Pure offline scan over the present Item grid -- no HAL.  Declared in
// aHotPlateSubstrate.h (left=true default lives there, NOT here).
// AI(W6.2c-INARM-1x2_4_Hot) 20260626.
bool TMyKitSuck::ArmLeftSideHaveRealIC(int MiddleValue)
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;
    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<MiddleValue; j++)
        {
            if(Item[i][j*2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][j*2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
bool TMyKitSuck::ArmRightSideHaveRealIC(int MiddleValue)                        //For 1x2 & 2x2使用的
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;

    for(int i=0; i<iMaxRow; i++)
    {
        for(int j=0; j<MiddleValue; j++)
        {
            if(Item[i][1+j*2]==HAS_NULL_IC)
                iHasNullICCount++;
            else if(Item[i][1+j*2]==NULL_IC)
                iNullICCount++;
            else
                iOtherCount++;
        }
    }

    if(iOtherCount)                                                             //JerryYang 20170703 (Steven) Fix 2x2_8 Hang up
        return true;
    else
        return false;
}
bool TMyKitSuck::ArmUpSideHaveRealIC(bool left)                                 //ChungHung 20130708 add left=true
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;

    int offset=0;
    if(left)
        offset=0;
    else
        offset=4;

    for(int j=0; j<4; j++)
    {
        if(Item[0][j+offset]==HAS_NULL_IC)                                      //ChungHung 20130708 add left=true
            iHasNullICCount++;
        else if(Item[0][j+offset]==NULL_IC)                                     //ChungHung 20130708 add left=true
            iNullICCount++;
        else
            iOtherCount++;
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
bool TMyKitSuck::ArmDownSideHaveRealIC(bool left)                               //ChungHung 20130708 add left=true;
{
    int iHasNullICCount=0;
    int iNullICCount=0;
    int iOtherCount=0;

    int offset=0;
    if(left)
        offset=0;
    else
        offset=4;

    for(int j=0; j<4; j++)
    {
        if(Item[1][j+offset]==HAS_NULL_IC)
            iHasNullICCount++;
        else if(Item[1][j+offset]==NULL_IC)
            iNullICCount++;
        else
            iOtherCount++;
    }

    if(iOtherCount)
        return true;
    else
        return false;
}
// FAITHFUL "NULL_IC -> HAS_NULL_IC" grid promotion (golden MyKitSuck.cpp:340-352,
// Steven 20150203 : Fixed for Sucker Status).  W6.2c batch-4: used by the 2x2_8_Hot
// in-arm place-to-shuttle SM (FLCarryKit/BLCarryKit, 4 sites).  Pure Item-grid scan
// over iShtRow/iShtCol -- no HAL.
void TMyKitSuck::SetNullIcToHasNullIc()
{
    for(int i=0; i<iShtRow; i++)
    {
        for(int j=0; j<iShtCol; j++)
        {
            if(Item[i][j]==NULL_IC)
            {
                SetItemData(i, j, HAS_NULL_IC);                                 //Steven 20150203 : Fixed for Sucker Status
            }
        }
    }
}
// AI(W906-P1b) 20260920: golden MyKitSuck.cpp:218-225 的兩參數多載，逐行照翻。
//   ⚠ 它**刻意只設 5 個欄位** —— iPickStep / iPickKitStep / iShtKitStep 不動。
//     那不是漏寫：這個多載的用途是「把 shuttle 的 Row/Col 幾何推給別的 kit」，
//     pitch 與 kit-step 屬於手臂自己的組態，不該被 shuttle 幾何覆蓋。
void TMyKitSuck::SetPickerCount(int _iPickRow, int _iPickCol)                   //Steven 20241002 : TestSuck補上 Row / Col
{
    iPickRow    =_iPickRow;                                                     // golden :220
    iPickCol    =_iPickCol;                                                     // golden :221
    iShtRow     =_iPickRow;                                                     // golden :222
    iShtCol     =_iPickCol;                                                     // golden :223
    iShtCnt     =iShtRow*iShtCol;                                               // golden :224
}

// FAITHFUL topology setter (golden MyKitSuck.cpp:206).
void TMyKitSuck::SetPickerCount(int _iPickRow, int _iPickCol, int _iShtRow, int _iShtCol, int _iPickStep, int _iKitStep, int _iShtStep)
{
    iPickRow    =_iPickRow;
    iPickCol    =_iPickCol;
    iShtRow     =_iShtRow;
    iShtCol     =_iShtCol;
    iShtCnt     =iShtRow*iShtCol;
    iPickStep   =_iPickStep;                                                    //Pitch倍數, 13吸嘴就寫2, 14吸嘴就寫3
    iPickKitStep=_iKitStep;
    iShtKitStep =_iShtStep;
}

// -- functions ----------------------------------------------------------------
// CopyInitSuck (golden MyKitSuck.cpp:1111): the golden body copies ~30 grid
// members; the offline substrate TMyKitSuck carries the subset the in-arm SMs
// actually read.  Conservative offline body: copy the present members so the
// OutArmSuck<->OutArmSuckBackup round-trip in SetInOutArmParameter_1x1_1 is
// data-faithful for those fields.
void CopyInitSuck(TMyKitSuck *Source, TMyKitSuck *Target, int SourceR, int SourceC, int TargetR, int TargetC)
{
    if(Source==0 || Target==0) return;
    Target->Item     [TargetR][TargetC] = Source->Item     [SourceR][SourceC];
    Target->iWhichSite[TargetR][TargetC]= Source->iWhichSite[SourceR][SourceC];
    Target->iBinData [TargetR][TargetC] = Source->iBinData [SourceR][SourceC];
    Target->bPass    [TargetR][TargetC] = Source->bPass    [SourceR][SourceC];
    Target->bNeedReTest[TargetR][TargetC]=Source->bNeedReTest[SourceR][SourceC];
    Target->cDeviceInf[TargetR][TargetC]= Source->cDeviceInf[SourceR][SourceC];
    Target->cSBin    [TargetR][TargetC] = Source->cSBin    [SourceR][SourceC];
}

// AI(W906-AutoCleanFoundation-Review) 20260722: golden MyKitSuck.cpp:1503-1561 --
// FAITHFUL line-for-line translation (every field golden copies/clears, in
// golden's order; see aHotPlateSubstrate.h's TMyKitSuck member comments for
// each field's golden home).  Used by the AutoClean shuttle-place/pick leaves
// (CleanPad_PlaceToShuttle / CleanPad_PickFromShuttle / DoAutoCleanKit-family)
// to move one nozzle's full record between two TMyKitSuck grids (e.g.
// InArmSuck -> FLCarryKit).  Calls the real SetItemData (not a raw Item[]=)
// exactly as golden does, so any future SetItemData side-effect stays wired.
// This is a MOVE not a copy: golden clears the Source slot after copying (via
// Source.SetItemData(..., NULL_IC) plus a matching per-field reset) so callers
// rely on the source grid going empty as a side-effect -- see review finding
// that a prior revision of this function stopped after the target-side copy
// and silently dropped golden's entire source-clear tail, leaving stale data
// in the source slot.  Golden's bLed/pLed UI sync (MyKitSuck.cpp:1532-1538) is
// omitted: this tree's TMyKitSuck has no bLed/pLed members (UI LED gate
// already dropped offline elsewhere, see All_HasIC).
void TMyKitSuck::MoveSuckDataDiff(TMyKitSuck &Source, int SourceR, int SourceC, int TargetR, int TargetC)
{
    SetItemData(TargetR, TargetC, Source.Item[SourceR][SourceC]);

    iWhichSite[TargetR][TargetC]     = Source.iWhichSite[SourceR][SourceC];
    iWhichAuto[TargetR][TargetC]     = Source.iWhichAuto[SourceR][SourceC];
    iWhichIndex[TargetR][TargetC]    = Source.iWhichIndex[SourceR][SourceC];
    iCurrRotAng[TargetR][TargetC]    = Source.iCurrRotAng[SourceR][SourceC];
    iNeedRotAng[TargetR][TargetC]    = Source.iNeedRotAng[SourceR][SourceC];

    bPass[TargetR][TargetC]          = Source.bPass[SourceR][SourceC];
    iCleanCount[TargetR][TargetC]    = Source.iCleanCount[SourceR][SourceC];
    bFliped[TargetR][TargetC]        = Source.bFliped[SourceR][SourceC];
    iBinData[TargetR][TargetC]       = Source.iBinData[SourceR][SourceC];
    iBinDataBackUp[TargetR][TargetC] = Source.iBinDataBackUp[SourceR][SourceC];

    iAutoCleanRecX[TargetR][TargetC] = Source.iAutoCleanRecX[SourceR][SourceC];
    iAutoCleanRecY[TargetR][TargetC] = Source.iAutoCleanRecY[SourceR][SourceC];
    cDeviceInf[TargetR][TargetC]     = Source.cDeviceInf[SourceR][SourceC];
    b2DIDNG[TargetR][TargetC]        = Source.b2DIDNG[SourceR][SourceC];
    cReDeviceInf[TargetR][TargetC]   = Source.cReDeviceInf[SourceR][SourceC];
    cSBin[TargetR][TargetC]          = Source.cSBin[SourceR][SourceC];
    bQATray[TargetR][TargetC]        = Source.bQATray[SourceR][SourceC];
    iAOIResult[TargetR][TargetC]     = Source.iAOIResult[SourceR][SourceC];

    PordRec[TargetR][TargetC].asBuffer->CommaText = Source.PordRec[SourceR][SourceC].asBuffer->CommaText;
    PordRec[TargetR][TargetC].bUse = Source.PordRec[SourceR][SourceC].bUse;

    Source.SetItemData(SourceR, SourceC, NULL_IC);

    Source.iWhichSite[SourceR][SourceC]      = -1;
    Source.iWhichAuto[SourceR][SourceC]      = -1;
    Source.iWhichIndex[SourceR][SourceC]     = -1;

    Source.bPass[SourceR][SourceC]           = false;
    Source.iCleanCount[SourceR][SourceC]     = 0;
    Source.bFliped[SourceR][SourceC]         = false;

    Source.iBinData[SourceR][SourceC]        = -1;
    Source.iAutoCleanRecX[SourceR][SourceC]  = -1;
    Source.iAutoCleanRecY[SourceR][SourceC]  = -1;

    Source.cDeviceInf[SourceR][SourceC]      = "";
    Source.cReDeviceInf[SourceR][SourceC]    = "";
    Source.cSBin[SourceR][SourceC]           = "";
    Source.b2DIDNG[SourceR][SourceC]         = false;

    Source.iCurrRotAng[SourceR][SourceC]     = 0;
    Source.iNeedRotAng[SourceR][SourceC]     = 0;
    Source.bQATray[SourceR][SourceC]         = false;
    Source.iAOIResult[SourceR][SourceC]      = 0;
}

// AI(W906-AutoCleanCluster) 20260728: golden MyKitSuck.cpp:1443-1501 -- FAITHFUL
// line-for-line translation (same field set/order as MoveSuckDataDiff above,
// PLUS the default-arg same-position resolution PLUS the extra
// Source.PordRec[][].InitialRecord() call golden's MoveSuckData has that
// MoveSuckDataDiff does NOT -- see this method's declaration comment in
// aHotPlateSubstrate.h for the full "4th discovered gap" rationale and the
// GOLDEN QUIRK citation). Used by DoIndexAutoClean/DoIndexAutoClean_Arm1PickArm2Test
// (AutoClean/AutoClean.cpp) to move one nozzle's full record between two
// TMyKitSuck grids at the SAME row/col (e.g. FTestSuck<->FLCarryKit).
void TMyKitSuck::MoveSuckData(TMyKitSuck &Source, int SourceR, int SourceC, int TargetR, int TargetC)
{
    if(TargetR==-1 || TargetC==-1)
    {
        TargetR=SourceR;
        TargetC=SourceC;
    }

    SetItemData(TargetR, TargetC, Source.Item[SourceR][SourceC]);
    iWhichSite[TargetR][TargetC]     = Source.iWhichSite[SourceR][SourceC];
    iWhichAuto[TargetR][TargetC]     = Source.iWhichAuto[SourceR][SourceC];
    iCurrRotAng[TargetR][TargetC]    = Source.iCurrRotAng[SourceR][SourceC];
    iNeedRotAng[TargetR][TargetC]    = Source.iNeedRotAng[SourceR][SourceC];
    iWhichIndex[TargetR][TargetC]    = Source.iWhichIndex[SourceR][SourceC];

    bPass[TargetR][TargetC]          = Source.bPass[SourceR][SourceC];
    iCleanCount[TargetR][TargetC]    = Source.iCleanCount[SourceR][SourceC];
    bFliped[TargetR][TargetC]        = Source.bFliped[SourceR][SourceC];
    iBinData[TargetR][TargetC]       = Source.iBinData[SourceR][SourceC];
    iBinDataBackUp[TargetR][TargetC] = Source.iBinDataBackUp[SourceR][SourceC];

    iAutoCleanRecX[TargetR][TargetC] = Source.iAutoCleanRecX[SourceR][SourceC];
    iAutoCleanRecY[TargetR][TargetC] = Source.iAutoCleanRecY[SourceR][SourceC];
    cDeviceInf[TargetR][TargetC]     = Source.cDeviceInf[SourceR][SourceC];
    cReDeviceInf[TargetR][TargetC]   = Source.cReDeviceInf[SourceR][SourceC];
    cSBin[TargetR][TargetC]          = Source.cSBin[SourceR][SourceC];
    b2DIDNG[TargetR][TargetC]        = Source.b2DIDNG[SourceR][SourceC];
    bQATray[TargetR][TargetC]        = Source.bQATray[SourceR][SourceC];
    iAOIResult[TargetR][TargetC]     = Source.iAOIResult[SourceR][SourceC];

    PordRec[TargetR][TargetC].asBuffer->CommaText = Source.PordRec[SourceR][SourceC].asBuffer->CommaText;
    PordRec[TargetR][TargetC].bUse   = Source.PordRec[SourceR][SourceC].bUse;

    Source.SetItemData(SourceR, SourceC, NULL_IC);
    Source.PordRec[SourceR][SourceC].InitialRecord();   // golden MyKitSuck.cpp:1474 -- MoveSuckDataDiff has NO equivalent call (golden quirk)

    Source.iWhichSite[SourceR][SourceC]      = -1;
    Source.iWhichAuto[SourceR][SourceC]      = -1;
    Source.iWhichIndex[SourceR][SourceC]     = -1;

    Source.bPass[SourceR][SourceC]           = false;
    Source.iCleanCount[SourceR][SourceC]     = 0;
    Source.bFliped[SourceR][SourceC]         = false;

    Source.iBinData[SourceR][SourceC]        = -1;
    Source.iAutoCleanRecX[SourceR][SourceC]  = -1;
    Source.iAutoCleanRecY[SourceR][SourceC]  = -1;

    Source.cDeviceInf[SourceR][SourceC]      = "";
    Source.cReDeviceInf[SourceR][SourceC]    = "";
    Source.cSBin[SourceR][SourceC]           = "";
    Source.b2DIDNG[SourceR][SourceC]         = false;

    Source.iCurrRotAng[SourceR][SourceC]     = 0;
    Source.iNeedRotAng[SourceR][SourceC]     = 0;
    Source.bQATray[SourceR][SourceC]         = false;
    Source.iAOIResult[SourceR][SourceC]      = 0;
}
#endif  // AI(W906-A4-6) 20260924
// ResetInToShtFlag (golden ainarm2.cpp:124): zero the in->shuttle pitch/Z flags.
void ResetInToShtFlag()
{
    for(int i=0;i<X_PITCH_COUNT;i++) iInXPToSht[i]=0;
    for(int i=0;i<MAX_ARM_Row;i++)
        for(int j=0;j<MAX_ARM_Col;j++){ iZPosToSht[i][j]=0; bZFlgToSht[i][j]=false; }
}

// cursor-init helpers (golden ainarm2.cpp) -- FAITHFUL.
#if 0   // PT-W7e-part2 RETIRED (InitInArmPickFromHotPlateTask340): real translated body now in ainarm2.cpp
void InitInArmPickFromHotPlateTask340() { iInArmPickFromHotPlateTask=340; }     // golden :628
#endif
void InitInArmPickFromHotPlateTask50()                                          // golden :633
{
    if(bPickFormHotplateRetry==true)                                            //Ifor 20160616
        iInArmPickFromHotPlateTask=190;
    else
        iInArmPickFromHotPlateTask=50;
}
void InitInArmPlaceToShuttleTask()                                              // golden :647
{
    iInArmPlaceToShuttleTask=1;
    if(TestIF_File.UseRotateForHT7000HPKit==false)                              //Sam 20250428
        iInRotateFinish=2;                                                      //Ifor 20211220
}
#if 0   // PT-W7e-part2 RETIRED (InitInArmPlaceToHotPlateTask): real translated body now in ainarm2.cpp
void InitInArmPlaceToHotPlateTask()    { iInArmPlaceToHotPlateTask=1;   }       // golden :1539
#endif
#if 0   // PT-W7e-part2 RETIRED (InitInArmPlaceToHotPlateTask400): real translated body now in ainarm2.cpp
void InitInArmPlaceToHotPlateTask400() { iInArmPlaceToHotPlateTask=400; }       // golden :1544
#endif
#if 0   // PT-W7e-part2 RETIRED (InitInArmPlaceToHotPlateTask100): real translated body now in ainarm2.cpp
void InitInArmPlaceToHotPlateTask100() { iInArmPlaceToHotPlateTask=100; }       // golden :1549
#endif
#if 0   // PT-W7e-part2 RETIRED (InitInArmTryPickFromHotPlateTask): real translated body now in ainarm2.cpp
void InitInArmTryPickFromHotPlateTask(){ iInArmTryPickFromHotPlateTask=1; }     // golden :613
#endif

// BackupPlacePos / RestorePlacePos (golden ainarm2.cpp:2185/:2192) -- FAITHFUL.
void BackupPlacePos()
{
    iBackupPlate =iPlacePlate[0];
    iBackupPlateC=iPlacePlateX[0];
    iBackupPlateR=iPlacePlateY[0];
}
void RestorePlacePos()
{
    iPlacePlate[0] =iBackupPlate;
    iPlacePlateX[0]=iBackupPlateC;
    iPlacePlateY[0]=iBackupPlateR;
}

// speed-display nudgers (golden ainarm2.cpp:2934/:2939) -- FAITHFUL.
#if 0   // PT-W7e-part2 RETIRED (InArmAddSpeedDisplay): real translated body now in ainarm2.cpp
void InArmAddSpeedDisplay() { iInArmSpeed1++; }                                 //KaiChen 20171225
#endif
#if 0   // PT-W7e-part2 RETIRED (InArmSubSpeedDisplay): real translated body now in ainarm2.cpp
void InArmSubSpeedDisplay() { iInArmSpeed1--; }                                 //KaiChen 20171225
#endif

// InitArmPickFromLoadStageTask (golden ainarm2.cpp:1016): the golden body resets
// the central pick cursor + several Loader-search statics (HAL-bound).  Offline:
// reset the cursor so the engine's DoInArmPickFromLoadStage_9045 SM restarts.
#if 0   // PT-W7e-part2 RETIRED (InitArmPickFromLoadStageTask): real translated body now in ainarm2.cpp
void InitArmPickFromLoadStageTask() { iPickFromLoadStageTask=1; }
#endif

// SetInArm_Unuse_SuckToNullICForHP (golden ainarm2.cpp:320): turns unused-site
// nozzles into NULL_IC before HP place.  Offline conservative no-op (the Sim grid
// is driven by the engine pick SM; nothing to reclassify offline).
#if 0   // PT-W7e-part2 RETIRED (SetInArm_Unuse_SuckToNullICForHP): real translated body now in ainarm2.cpp
void SetInArm_Unuse_SuckToNullICForHP() {}
#endif

// W6.2c: cross-variant in-arm shims with NO live golden home.  AI(W6.2c-INARM) 20260626.
// Zteach (golden InOutArmZteach.h): AutoTeach Z form.  Offline stand-in; fShow==false
// so the variant SMs take the non-teach path verbatim.  Single canonical def.
TfInOutArmZteach_Facade  g_ZteachFacade;
TfInOutArmZteach_Facade *Zteach = &g_ZteachFacade;                               // golden InOutArmZteach.h:Zteach
// DoCheckAutoSiteMappingPosition (golden ainarm2.cpp:2812): drives the JCET/ASE
// Auto-Site-Mapping step on a HW-bound site-map check.  Offline-safe no-op (no
// site-map HW; bRunAutoSiteMapping path is inert offline).
#if 0   // PT-W7e-part2 RETIRED (DoCheckAutoSiteMappingPosition): real translated body now in ainarm2.cpp
void DoCheckAutoSiteMappingPosition() {}
#endif
// CheckClearAllHotICThenPickLoadIC (golden ainarm2.cpp:533): true iff the HP grid
// still holds >= iLimit real ICs (so "clear all hot IC before picking load IC").
// Offline conservative: report not-satisfied so the SM takes the normal pick path.
#if 0   // PT-W7e-part2 RETIRED (CheckClearAllHotICThenPickLoadIC): real translated body now in ainarm2.cpp
bool CheckClearAllHotICThenPickLoadIC() { return false; }
#endif

// AdjustShuttleWhichKitOrder (golden ainarm2.cpp:849): re-orders which Shuttle/Kit
// the in-arm targets next.  Offline no-op (single-site: order is invariant).
#if 0   // PT-W7e-part2 RETIRED (AdjustShuttleWhichKitOrder): real translated body now in ainarm2.cpp
void AdjustShuttleWhichKitOrder() {}
#endif

// AdjustShtOrderWhenPlaceToSht (golden ainarm2.cpp:772): post-place shuttle-order
// adjust.  Offline no-op (single-site).
#if 0   // PT-W7e-part2 RETIRED (AdjustShtOrderWhenPlaceToSht): real translated body now in ainarm2.cpp
void AdjustShtOrderWhenPlaceToSht(int /*iMode*/) {}
#endif

// SetInArmNeedDestory (golden ainarm2.cpp:3897): marks a nozzle as needing a
// destroy (blow-off) before/after place.  Offline no-op (no real vacuum line;
// TMySucker::Destroy() already reports "not finished" so the SM is consistent).
#if 0   // PT-W7e-part2 RETIRED (SetInArmNeedDestory): real translated body now in ainarm2.cpp
void SetInArmNeedDestory(bool /*bPlace*/, int /*iShtRow*/, int /*iShtCol*/, int /*iRow*/, int /*iCol*/) {}
#endif

// TransferInShuttleRatio (golden ainarm2.cpp:1972): in-shuttle software gear-ratio
// X/Y transform.  Offline: identity (gear ratio applies a 1:1 mapping when the
// ratio params are unset).  Pointers are caller-owned and non-null at the call site.
#if 0   // PT-W7e-part2 RETIRED (TransferInShuttleRatio): real translated body now in ainarm2.cpp
void TransferInShuttleRatio(int /*iSht*/, int * /*iXPos*/, int * /*iYPos*/, int /*iRow*/, int /*iCol*/) {}
#endif

// CheckInArmFloating (golden OmronLaser/LaserSensorInArm.h:30): laser float check.
// Offline: no laser -> report "finished/ok" (true) so the place SM advances.
// AI(W906-PT-W3-integrate) 20260808: `bool CheckInArmFloating(bool) { return true; }`
//   RETIRED.  OmronLaser/LaserSensorInArm.cpp landed in wave PT-W3 with golden's real
//   body (port :400, golden OmronLaser/LaserSensorInArm.cpp:321) and both are in
//   ht9045_sm, so keeping this stub produced `multiple definition`.  BEHAVIOUR CHANGE,
//   and it is the faithful direction: the stub answered "not floating" unconditionally;
//   the real body walks the in-arm laser readings per position.  aHotPlateSubstrate.h:1069
//   keeps the `extern bool CheckInArmFloating(bool bReset=false);` declaration -- the
//   default argument lives there, matching golden LaserSensorInArm.h:30.

// SetShuttlefCanMoveL (golden ainarm2.cpp:3910): set per-shuttle can-move-left
// interlock flag.  Offline: write the engine-visible MOT[] flag faithfully so the
// in-arm/shuttle handshake is consistent.
#if 0   // PT-W7e-part2 RETIRED (SetShuttlefCanMoveL): real translated body now in ainarm2.cpp
void SetShuttlefCanMoveL(int iShuttle, bool bCanMoveL, AnsiString /*sFun*/, AnsiString /*sTask*/)
{
    int mot = (iShuttle==0) ? MInShuttle1 : MInShuttle2;
    MOT[mot].fCanMoveL = bCanMoveL;
}
#endif

// InitInOCRWaitTask / OCRMoveInArm2XYToWait (golden ainarm2.cpp:3950/:3955): OCR
// tray-arm wait handshake.  Offline: no OCR -> the move "completes" immediately.
#if 0   // PT-W7e-part2 RETIRED (InitInOCRWaitTask): real translated body now in ainarm2.cpp
void InitInOCRWaitTask() {}
#endif
#if 0   // PT-W7e-part2 RETIRED (OCRMoveInArm2XYToWait): real translated body now in ainarm2.cpp
bool OCRMoveInArm2XYToWait() { return true; }
#endif

// IsHotPlateCheckFinsih (golden ainarm2.cpp:2199): scans the HP grids vs a
// site-mode count to decide whether the HP try-pick sweep is done.  The golden
// body reads HotPlateForm / TestIF.iTestMode / MOT[MMPlate*] (HAL+form bound).
// Offline conservative: report finished (true) so the try-pick sweep terminates
// rather than looping over a HotPlate that is never populated offline.
#if 0   // PT-W7e-part2 RETIRED (IsHotPlateCheckFinsih): real translated body now in ainarm2.cpp
bool IsHotPlateCheckFinsih() { return true; }
#endif

// CheckInArmSuckInitial (golden ainarm2.cpp:1207): integrates all suck-init errors
// into one alarm; reads the vacuum sensors.  Offline: vacuum is idealized -> no
// init error -> report ready (true) so DoInArm_9045_1x1_1 case 10 proceeds.
#if 0   // PT-W7e-part2 RETIRED (CheckInArmSuckInitial): real translated body now in ainarm2.cpp
bool CheckInArmSuckInitial() { return true; }
#endif

// SetInArmUseSuckToHasTrySuckIC (golden ainarm2.cpp:261): mark in-use nozzles
// HAS_TRY_SUCK_IC for the HotPlate try-pick.  Offline conservative: the single
// 1-picker path (the only one this single-site variant drives) sets nozzle (0,0).
#if 0   // PT-W7e-part2 RETIRED (SetInArmUseSuckToHasTrySuckIC): real translated body now in ainarm2.cpp
void SetInArmUseSuckToHasTrySuckIC(int /*iSht*/, int /*iKit*/)
{
    InArmSuck.SetItemData(0, 0, HAS_TRY_SUCK_IC);                               //JerryYang 20251013 (offline subset of golden)
}
#endif

// DisableAutoSiteMapWhenCleanOut (golden ainarm2.cpp:2786): clears the auto-site-
// map run flag when Loader has no IC during clean-out.  Offline no-op (auto site
// map is not exercised offline).
#if 0   // PT-W7e-part2 RETIRED (DisableAutoSiteMapWhenCleanOut): real translated body now in ainarm2.cpp
void DisableAutoSiteMapWhenCleanOut() {}
#endif

// SetMotorSpeed (golden cinitial.cpp:5022): pushes the configured motor speed
// table to the controllers.  Offline no-op (no real controllers).
#if 0   // PT-W7a RETIRED (SetMotorSpeed)
//AI(ht9045-v906) 20260810: PT-W7a -- RETIRED. cinitial.cpp now holds golden's real body (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0810_w7a.
void SetMotorSpeed() {}
#endif

// DoInArmAutoSiteMapping (declared csystem.h:200): auto-site-map step pump.
// Offline: not running auto-site-map -> false (DoInArm_9045_1x1_1 falls through).
#if 0   // PT-W5f RETIRED (DoInArmAutoSiteMapping)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error, measured in build_0810_w5f.
bool DoInArmAutoSiteMapping() { return false; }
#endif

// ZAxisNotDown (golden ainarm2.h:43): the "Z stays up" flag passed to
// MoveInArmXYToWaitTrayArm.  Sibling of ZAxisDown (already defined above).
const bool ZAxisNotDown = false;

// CheckInArmDestroyICFail (golden csystem.cpp / declared csystem.h:88): checks
// whether a blown-off (destroyed) IC is stuck.  Offline: no real vacuum -> no
// destroy failure -> report ok (true) so the place SM proceeds.
#if 0   // PT-W5c RETIRED (CheckInArmDestroyICFail)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckInArmDestroyICFail() { return true; }
#endif
