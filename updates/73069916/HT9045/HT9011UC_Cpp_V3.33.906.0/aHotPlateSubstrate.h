// =============================================================================
//  aHotPlateSubstrate.h  --  W6.2 in-arm HotPlate substrate shim
//
//  Translation wave: W6.2 (IN-ARM foundation -- shared HP pick/place geometry)
//  Translator: AI(W6.2-INARM) 20260626
//
//  WHY THIS FILE EXISTS
//  --------------------
//  The two shared HP-geometry leaf modules translated this wave --
//  ainarm_SearchPickPlate.cpp (mirrors golden MyKitSuck.h + ainarm2.h + HTEditList.h)
//  ainarm_SearchPlacePlate.cpp
//  pump a substrate that the proven W6.0/W6.1 scaffold deliberately DEFERRED:
//
//    (a) TMyKitSuck InArmSuck / FLCarryKit / BLCarryKit / OutArmSuck
//        -- golden MyKitSuck.h:357.  This is exactly the "KitSuck grid"
//           csystem_predicates.cpp documents as the W6.x dependency
//           (HT9045_KITSUCK_GRID_AVAILABLE 0).  W6.2 is the wave that
//           introduces it.  Mirrored MINIMALLY: only the members these two
//           modules deref are declared (documented inline); offline bodies are
//           no-ops / conservative defaults over the Sim HAL.
//
//    (b) PickFromHPList (uPlateInfo*) -- golden Public/HTEditList.h:191/244.
//        The HP pick/place "team list".  Mirrored MINIMALLY to the called
//        surface (GetHPFirstTeam* / SetArrPlateXY / SetPlateSuck /
//        DataForwardAndNextTeam / GetHPFirstTeamToList / SaveFile).  Needs
//        uPoint2D (golden HTEditList.h:323) + uBasicPickPlace base.
//
//    (c) ainarm2.h cursors / arrays -- golden ainarm2.h:62-114:
//        iPickPlate[2]/iPickPlateX[2]/iPickPlateY[2]/iPlacePlate[2]/...,
//        iInArmPickFromHotPlateTask, iHotWhichShuttle/iHotWhichKit/
//        iHotPlateCount[2][50][50], InArmSuckUse[][], bPickFromHotplate,
//        InArmSiteMapData (strAUTOSITEMAP), bInArmCheckDestroyACT[][],
//        ZAxisDown.  Mirrored from the golden ainarm2.h declarations verbatim.
//
//    (d) extern decls (bodies live in the later ainarm core, W6.x/W7) for the
//        in-arm engine helpers the leaves call:
//        bUseAxExPicker / bUseAxxGPicker (golden ainarm9045.h),
//        CloseSiteState / InitInArmTask / SetRunStartMode /
//        TransferHotPlateRatio / InspectInArmPosition /
//        MoveInArmZToPlateSafe / IsCheckInArmDestroyActiveFinish /
//        DoInArm_9045_SuckerMap / SetShuttleToHasNullIC_9045 /
//        AdjustShuttlePlaceOrder / SetInArmHome / AddInArmPickerCount /
//        InArmSubSpeed / InArmAddSpeed / GetInArmPitchX_9045 /
//        GetInArmPitchY_9045 / GetInRotationAngRecord-on-record /
//        StopAllMotor / MyDBIProcess.
//        (InArmContinuousMove_9045 already has a W4 sim stub in Motor/mymotor.h.)
//        (DoAutoSiteMappingDropError already declared in csystem.h.)
//        Offline sim bodies are in aHotPlateSubstrate.cpp (except MyDBIProcess 2-arg: a live adapter to the golden cMyDB body since P4 -- AI(W906-P4) ★W56, Steven 0928).
//
//    (e) Rotate / Laser micro-shims the place-swap leaf derefs:
//        tRotate.ActiveRotate (golden aRotateKIT.h), USE_ROTATE_KIT /
//        iRotate_Type / eInOutArm1Motor already in cmydef.h/MachineType.h,
//        TInLaserCheck + LaserCheckPos (golden -- laser-distance check list).
//
//  SCOPE: strictly the members the two modules deref.  No scope creep.  When the
//  real MyKitSuck.h / HTEditList.h / ainarm2.h / aRotateKIT.h land (W6.x/W7),
//  these shims are replaced and csystem_predicates.cpp may flip
//  HT9045_KITSUCK_GRID_AVAILABLE to 1.
//
//  Big5: every Chinese comment decoded cleanly via cp950 and is preserved as
//  UTF-8.  NO U+FFFD is emitted.
// =============================================================================
#ifndef aHotPlateSubstrateH
#define aHotPlateSubstrateH

#include "vclcompat/vcl_compat.h"   // AnsiString
#include "cmydef.h"                 // MAX_ARM_Row/Col, X_PITCH_COUNT, IC consts
#include "Motor/mymotor.h"          // class TTrayMotor (CopyFromTray/CopyToTray)
#include "Public/MyProductionRecord.h"  // TMyProductionRecord (golden by-value PordRec)
#include "myTimer.h"                // TQPF_Timer (W6.5: TMyKitSuck::TSoakTime)

// ---------------------------------------------------------------------------
//  TList  -- golden VCL Classes.hpp TList (pointer array).  Not in vcl_compat;
//  HasHotReadyIC_9045() does `new TList()` + ->Count / ->Items[i] / ->Clear().
//  MINIMAL mirror of the touched surface (Steven 20240311 list walk).
// ---------------------------------------------------------------------------
#ifndef HT9045_TLIST_SHIM
#define HT9045_TLIST_SHIM
#include <vector>
class TList
{
    std::vector<void*> v;
public:
    int    Count;                                   // VCL TList::Count (property)
    TList() : Count(0) { Items.pv=&v; }
    int    Add(void* p)     { v.push_back(p); Count=(int)v.size(); return Count-1; }
    void   Clear()          { v.clear(); Count=0; }
    // VCL spells element access `list->Items[i]`.  Provide an Items proxy so the
    // golden `(uPoint2D*)lsPoint2D->Items[i]` reads compile unchanged.
    struct ItemsProxy {
        const std::vector<void*>* pv;
        void* operator[](int i) const { return (*pv)[i]; }
    } Items;
};
#endif // HT9045_TLIST_SHIM

#include "mykitsuck.h"   // AI(W906-A4-6) 20260924: 吸嘴類別改用 golden 的完整佈局（mykitsuck.h = golden MyKitSuck.h）；下面的精簡鏡像退役（兩份同名不同佈局 = A4-6 的 ODR 隱患）
//  (a) TMyKitSuck grid  -- golden MyKitSuck.h:14/151
//      MINIMAL mirror: only the members ainarm_SearchPickPlate.cpp /
//      ainarm_SearchPlacePlate.cpp dereference.  _MAX_SUCK_ROW/COL kept = golden.
// ============================================================================
#define _MAX_SUCK_ROW_ITEM 4
#define _MAX_SUCK_COL_ITEM 8

#if 0   // AI(W906-A4-6) 20260924: 精簡鏡像 TMySucker／TMyKitSuck 退役（缺 IO 接線欄位與 DoOnIO／DoOffIO／OffSuck…，真機上吸嘴不會真的開關真空）。移植樹獨有的觀測接縫與 SetType1ToType2ByPickCol 已搬進 mykitsuck.h。原註：---- TMySucker (golden MyKitSuck.h:14) -- only the touched surface
class TMySucker
{
public:
    bool       Error;           // golden :57   -- pick/place vacuum error flag
    int        OnDelayTime;     // golden :64   -- HP vacuum on-delay (centi-sec)
    AnsiString sName;           // golden :158  -- nozzle position label
    // -- W6.2b-canary ADD: CheckInArmSuckICFallDownToHasNullIC (csystem.cpp,
    //    golden csystem.cpp:1677) reads .SenUsing!="" to decide whether this
    //    vacuum line has a sensor wired before drop-detecting.  golden
    //    MyKitSuck.h:81 verbatim.  Offline-safe: plain data, default empty ->
    //    the drop-detect loop skips every nozzle (no sensor configured).
    AnsiString SenUsing;        // golden :81   -- the vacuum sensor name this nozzle uses ("" = none)

    bool Suck();                // golden :89   -- vacuum ON / destroy OFF (returns "suck finished")
    bool Destroy();             // golden :90   -- destroy(blow) ON (returns "destroy finished")  // W6.2b: ProcessSCKARTLoadingCount case 1
    void On();                  // golden :91   -- vacuum solenoid ON
    void Off();                 // golden :92   -- vacuum solenoid OFF

    // -- W6.2c ADD: members the OUT-ARM ENGINE (aoutarm9045.cpp) derefs ----------
    //    golden MyKitSuck.h member names verbatim.  Added only because the
    //    out-arm destroy-confirm / safe-move / init-state code reads them.
    void OnSuck();              // golden :95   -- vacuum-on (suck) solenoid
    void OnDestroy();           // golden :97   -- destroy(blow) solenoid ON
    void OffDestroy();          // golden :98   -- destroy(blow) solenoid OFF
    void Normal();              // golden :99   -- return nozzle to idle/normal
    bool GetStatus();           // golden :103  -- read vacuum sensor (true=still holding)
    // AI(W906-HOME-C2) 20260920: golden MyKitSuck.h:104 `bool Sensor();`
    //   它是 GetStatus() 的四行包裝：沒裝（Enable==false）就當**有料**（回 true），
    //   裝了才真的問感測器。golden MyKitSuck.cpp:1943-1949。
    //   第一個消費者：uhome.cpp 的 ProcessMotorHome case 2（歸零時把
    //   In Arm 吸嘴上的 IC 放回去之前，先確認哪幾格真的有料）。
    bool Sensor();              // golden :104
    int  iMotNo;                // golden :138  -- the Z motor index for this nozzle
    int  iMyRow;                // golden :139  -- physical row of this nozzle
    int  iMyCol;                // golden :140  -- physical col of this nozzle

    // -- W6.3 ADD: members the TRAY-ARM ENGINE (acatchtray.cpp) derefs ----------
    //    golden MyKitSuck.h member names verbatim.  CatchTraySuck.Suck[0][0] reads
    //    .Enable (whether this vacuum line is configured) + .OnAlarmTime (vacuum-on
    //    alarm window, centi-sec) and calls Reset() (golden MyKitSuck.h).
    bool Enable;                // golden :55   -- vacuum line configured/installed
    int  OnAlarmTime;           // golden :63   -- vacuum-on alarm window (centi-sec)
    void Reset();               // golden :94   -- reset this nozzle's suck/destroy task

    // -- W6.2b(2x4_16) ADD: the in-arm place-to-shuttle SM reads the per-nozzle
    //    "needs destroy(blow-off)" flag (golden MyKitSuck.h:120/124, inline body).
    //    Offline-safe: the flag is plain data, default-false; no HW touched.
#ifndef HT9045_SUCKER_NEEDDESTROY_ADDED
#define HT9045_SUCKER_NEEDDESTROY_ADDED
    bool bNeedDestroy;          // golden MyKitSuck.h:120
    bool GetNeedDestroyStatus() { return bNeedDestroy; }   // golden MyKitSuck.h:124
#endif

    // -- W6.2c(OUT-ARM) ADD: the out-arm pick SM reads/writes the per-nozzle
    //    "needs suck(vacuum)" flag (golden MyKitSuck.h:119/121/123, inline bodies).
    //    Offline-safe: plain data, default-false; no HW touched.
#ifndef HT9045_SUCKER_NEEDSUCK_ADDED
#define HT9045_SUCKER_NEEDSUCK_ADDED
    bool bNeedSuck;             // golden MyKitSuck.h:119
    void SetNeedSuck(bool Value) { bNeedSuck = Value; }    // golden MyKitSuck.h:121
    bool GetNeedSuckStatus()     { return bNeedSuck; }     // golden MyKitSuck.h:123
#endif

    // ========================================================================
    // AI(W906-W7-L2) 20260803: the golden TMySucker surface ckernel.cpp needs,
    // plus the ONE observability seam that makes StopAllDestroy falsifiable.
    // Guarded so a parallel sibling wave editing this header cannot
    // double-declare the block.
    //
    // VERIFIED CONSUMERS (every line below was read in golden ckernel.cpp):
    //   * ScanSystemSensor's PAUSE (SoftStop) branch snapshots each nozzle's
    //     learned vacuum on/off time into LastSet and then ReStart()s the
    //     nozzle -- golden ckernel.cpp:547-553 (In/OutArmSuck),
    //     :565-571 (F/BTestSuck), :613-614 (CatchTraySuck).
    //   * The index "vacuum solenoid still ON but the site holds no IC" guard
    //     reads GetOnBit() -- golden ckernel.cpp:578, :585, :597, :604 (PAUSE
    //     branch) and :635, :642, :654, :661 (the SystemStart==false sweep).
    //   * StopAllDestroy calls OffDestroy() and nothing else --
    //     golden ckernel.cpp:189-209.
    // ========================================================================
#ifndef HT9045_SUCKER_W7L2_ADDED
#define HT9045_SUCKER_W7L2_ADDED
    // ---- (a1) learned vacuum durations -------------------------------------
    // golden MyKitSuck.h:42-43.  These are NOT static config: golden
    // PushOnTime/PushOffTime (MyKitSuck.cpp:1805-1847) rolls a 20-sample average
    // into them at runtime, and Suck()/Destroy() arm the TOn/TOff timers from
    // them (MyKitSuck.cpp:2177 / :2389, each with its own runaway clamp).
    //
    // OFFLINE DEFAULT 120 / 70 -- deliberately NOT zero.  These are golden's own
    // ctor values (MyKitSuck.cpp:1779-1780).  Zero would make the PAUSE snapshot
    // loop at golden ckernel.cpp:547-550 copy 0 into an already-zeroed LastSet
    // grid, i.e. "the snapshot ran" and "the snapshot never ran" would produce
    // identical state -- the same unfalsifiable shape the OffDestroy seam below
    // exists to close.  With 120/70 the loop transfers a distinctive value and a
    // test can prove it executed.  DWORD comes from myTimer.h -> <windows.h>
    // (already included by this header).
    DWORD VacuumOnTime;         // golden MyKitSuck.h:42 (ctor 120, MyKitSuck.cpp:1779)
    DWORD VacuumOffTime;        // golden MyKitSuck.h:43 (ctor  70, MyKitSuck.cpp:1780)

    // ---- (a2) ReStart ------------------------------------------------------
    // golden MyKitSuck.h:102; body MyKitSuck.cpp:1849-1853 is literally
    // `OnTask=1; OffTask=1;` -- it rewinds the suck/destroy micro state machines
    // to step 1 and touches NO hardware.  The offline body is therefore the
    // golden body VERBATIM (FAITHFUL, not a stub).
    void ReStart();             // golden MyKitSuck.h:102
    // golden MyKitSuck.h:17-18.  PRIVATE in golden; public here because the shim
    // is uniformly public AND because these two ints are ReStart's ONLY effect --
    // hiding them would leave ReStart with no observable at all.
    // OFFLINE DEFAULT 1 for both = golden's ctor value (MyKitSuck.cpp:1741-1742),
    // i.e. "suck/destroy state machine parked at step 1, nothing in flight".
    // BRANCH NOTE: 1 is also ReStart's own result, so from a virgin grid ReStart
    // is a no-change; a test proves the golden ckernel.cpp:552-553 / :570-571
    // calls actually ran by parking a non-1 sentinel in OnTask/OffTask first.
    int  OnTask;                // golden MyKitSuck.h:17
    int  OffTask;               // golden MyKitSuck.h:18

    // ---- (a3) GetOnBit -----------------------------------------------------
    // golden MyKitSuck.h:105; body MyKitSuck.cpp:1862-1888.  Golden reads the
    // vacuum-ON solenoid's OUTPUT bit back off the IO card (:1870 MotionNet/
    // PCI1203, :1876 ISA/PCI1735U/PLC), de-inverts it by OnType
    // (:1879-1882 `OnType==TYPE_A ? ret : !ret`), and returns FALSE outright when
    // the line is not wired (:1884-1887).
    // OFFLINE: there is no IO card, so the readback is served from the latch
    // below, which the solenoid writers keep.  CORRECTED 20260803: an earlier
    // revision of this sentence said the latch reproduces "exactly the round
    // trip golden performs" through DoOnIO().  It does not, and the true census
    // is on the latch member itself below -- golden drives that bit from EIGHT
    // member functions, most of which this shim reproduces only partially and
    // one of which (CheckIsFallDown) it does not declare at all.  DoOnIO() is
    // faithfully summarised: MyKitSuck.cpp:1951-2018, `DoOnIO(true)` energises
    // (:1955 `if(bOn==true)` -> :1968 IOBitOn for TYPE_A) and `DoOnIO(false)`
    // de-energises (:1986 else -> :1999 IOBitOff), with the A/B sense inverted
    // in the else-arms (:2007/:2013).  OnType is
    // deliberately NOT mirrored: DoOnIO applies the A/B inversion on the way out
    // (:1957 vs :1988) and GetOnBit removes it on the way back in, so the LOGICAL
    // value is OnType-independent -- a mirrored OnType could provably never
    // change an answer, and a field that cannot change an answer is noise.
    bool GetOnBit();            // golden MyKitSuck.h:105
    // golden MyKitSuck.h:70 -- "is the vacuum-ON line wired at all".
    // OFFLINE DEFAULT false = golden's own ctor value (MyKitSuck.cpp:1756), and
    // it is a BRANCH SELECTION, not neutrality: with OnEnable=false GetOnBit()
    // returns false, so `FTestSuck.Item[i][j]==NULL_IC &&
    // FTestSuck.Suck[i][j].GetOnBit()` at golden ckernel.cpp:597-598 (and its 7
    // siblings :578, :585, :604, :635, :642, :654, :661) short-circuits FALSE.
    // ScanSystemSensor therefore does NOT call Suck[i][j].Normal() and does NOT
    // clear CloseSuckok, so the `if(CloseSuckok==false && INDEX_SUCKER_TYPE==1)`
    // arm at golden ckernel.cpp:672-675 is NOT taken and bIndexPlaceIcCheck stays
    // as-is (the index place-IC alarm stays armed).  A test that wants the OTHER
    // arm sets OnEnable=true and energises the nozzle -- On()/OnSuck() suffice,
    // but Suck() ALSO needs LastSet.iRealDummy=REALLY, because Suck() reproduces
    // golden's dummy early-out (MyKitSuck.cpp:2167) and the sim default is
    // DUMMY (canary_support.cpp:26 `LastSet = {0}`, DUMMY==0 cmydef.cpp:267).
    bool OnEnable;              // golden MyKitSuck.h:70
    // Offline stand-in for the vacuum-ON output bit GetOnBit reads back.  Holds
    // the LOGICAL solenoid state (post-de-inversion, see the OnType note above).
    //
    // CORRECTED 20260803.  An earlier revision of this comment claimed the
    // writers were On()/OnSuck()/Off()/Normal() and that golden's DoOnIO census
    // was complete at four sites (2096/2102/2129/2137).  IT IS NOT.  Every line
    // of golden MyKitSuck.cpp (all 2894) was scanned this pass; the vacuum-ON
    // bit is driven from FOURTEEN sites in EIGHT member functions (OnSuck,
    // OffSuck, On, Off, Normal, Suck, Destroy, CheckIsFallDown):
    //   DIRECT DoOnIO():
    //     :2096 OnSuck()          SET     -- shim: reproduced (no DUMMY gate)
    //     :2102 OffSuck()         CLEAR   -- shim: inlined into Normal()
    //     :2129 On()              SET     -- shim: reproduced (no DUMMY gate)
    //     :2137 Off()             CLEAR   -- shim: reproduced
    //     :2210 Suck()            SET     -- shim: reproduced, WITH golden's
    //                                        :2167 dummy gate and :2207 OnTask
    //                                        guard
    //     :2395 Destroy()         CLEAR   -- dummy branch, SuckerName-gated
    //     :2436 Destroy()         CLEAR   -- mainline, :2434 OffTask!=300 guard
    //     :2694 CheckIsFallDown() CLEAR   -- NOT on this shim's surface at all
    //   INDIRECT, via Normal() -> OffSuck() -> :2102:
    //     :2145 Normal()   :2230 Suck()   :2283 Suck()   :2507 Destroy()
    //   INDIRECT, via a direct OffSuck() call:
    //     :2535 Destroy()  :2581 Destroy()
    // The two Suck() clears (:2230/:2283) and the three Destroy() clears
    // (:2507/:2535/:2581) sit on retry-exhausted arms that the offline bodies
    // -- which return false unconditionally and never advance OnTask/OffTask --
    // cannot reach.  Those are documented holes, not silent ones; see the two
    // bodies in aHotPlateSubstrate.cpp.
    // Default false = "nothing energised yet".
    bool W906_SimVacuumOnBit;

    // ---- (b) OBSERVABILITY SEAM: per-nozzle OffDestroy() call counter -------
    // golden StopAllDestroy (ckernel.cpp:189-209) does NOTHING except call
    // OffDestroy() over a fixed nozzle set.  Offline there is no destroy
    // solenoid, so OffDestroy() previously had NO effect whatsoever -- meaning a
    // no-op stub of StopAllDestroy and a faithful translation of it produce
    // byte-identical state, and any test written against it is tautological.
    // This counter is the seam that makes it falsifiable.  Same class of seam as
    // canary_support.h's W906_ShowErrorMessage_Count (canary_support.h:250);
    // implemented HERE, on the class, because it must be PER NOZZLE -- the
    // interesting property of StopAllDestroy is WHICH nozzles it reaches (the
    // golden iMaxRow/iMaxRow bound below reaches a strict subset of the grid),
    // not merely how many calls happened in total.
    //
    // RESETTABLE IN O(1) VIA AN EPOCH: W906_TMySucker_OffDestroy_ResetAll()
    // (declared after this class) bumps a global generation number; any nozzle
    // whose stamp is stale reads as 0.  No per-instance registry is needed, so
    // nozzle grids added by later waves are covered automatically and no test can
    // leak a count into the next test.
    int  W906_GetOffDestroyCount() const;  // this nozzle's count since the last reset
    void W906_ResetOffDestroyCount();      // isolate ONE nozzle
    unsigned long W906_OffDestroyStamp;    // generation the raw count belongs to
    int           W906_OffDestroyRaw;      // raw count -- read via W906_GetOffDestroyCount()

    // Offline ctor.  Only the members declared in THIS W7-L2 block are set.
    //
    // WHY THE OTHER SHIM MEMBERS ARE LEFT ALONE (deliberate, not an oversight):
    // every TMySucker in this tree lives inside a namespace-scope TMyKitSuck, so
    // it is zero-initialised before any constructor runs, and the already-landed
    // waves were written against those zeros.  Adopting golden's ctor values for
    // them -- e.g. golden OnAlarmTime=100 (MyKitSuck.cpp:1745), OnDelayTime=0
    // (:1747), Enable=false (:1739) -- would change behaviour under code this wave
    // has not read (acatchtray reads CatchTraySuck.Suck[0][0].Enable /
    // .OnAlarmTime).  That audit belongs to whoever lands the real MyKitSuck.
    //
    //AI(W906-W7-L2) 20260803 DEFINED INLINE, ON PURPOSE -- do not move it back to
    // the .cpp.  TMySucker previously had NO user-declared constructor, so a TU
    // could instantiate one with no link edge at all.  Giving it an out-of-line
    // ctor silently added `aHotPlateSubstrate.cpp` as a link requirement to every
    // such TU, and the first full build of this wave proved it: tests/
    // test_MyProductionRecord.cpp constructs a TMySucker directly and died with
    // `undefined reference to TMySucker::TMySucker()`.  Inline here restores the
    // header-only property.  It also dictates the ONE deviation below: the stamp
    // is seeded with the reserved 0 rather than with W906_TMySucker_OffDestroyEpoch
    // (declared 10 lines further down, and DEFINED in the .cpp -- reading it here
    // would just re-create the same link edge under a different symbol name).
    // 0 is exactly equivalent: the epoch starts at 1, so a 0 stamp reads as STALE,
    // and stale means "count 0" -- which is what a freshly constructed nozzle is.
    TMySucker()
    {
        VacuumOnTime  = 120;    // golden MyKitSuck.cpp:1779
        VacuumOffTime = 70;     // golden MyKitSuck.cpp:1780
        OnTask        = 1;      // golden MyKitSuck.cpp:1741
        OffTask       = 1;      // golden MyKitSuck.cpp:1742
        OnEnable      = false;  // golden MyKitSuck.cpp:1756
        // offline-only: no solenoid has been energised yet, so the readback latch
        // is clear.  (golden has no counterpart -- it reads the real IO card.)
        W906_SimVacuumOnBit  = false;
        W906_OffDestroyStamp = 0;   // reserved "never stamped" -- see the note above
        W906_OffDestroyRaw   = 0;
    }
#endif // HT9045_SUCKER_W7L2_ADDED
};

// AI(W906-W7-L2) 20260803: grid-wide half of the OffDestroy seam above.
// The epoch is bumped (never re-zeroed) by ResetAll, so every per-nozzle count
// in the whole tree -- InArmSuck / OutArmSuck / F,BTestSuck / CatchTraySuck and
// any grid a later wave adds -- goes stale in one O(1) step.  The total is the
// cheap assertion for "StopAllDestroy touched exactly N nozzles"; the per-nozzle
// counts are the assertion for "and they were THESE nozzles".
#ifndef HT9045_SUCKER_W7L2_SEAM_GLOBALS
#define HT9045_SUCKER_W7L2_SEAM_GLOBALS
extern unsigned long W906_TMySucker_OffDestroyEpoch;  // current generation (starts at 1)
extern long          W906_TMySucker_OffDestroyTotal;  // OffDestroy() calls tree-wide since the last reset
void W906_TMySucker_OffDestroy_ResetAll();            // bump the epoch + clear the total
#endif // HT9045_SUCKER_W7L2_SEAM_GLOBALS

// ---- TMyKitSuck (golden MyKitSuck.h:151) -- MINIMAL mirror ------------------
//  PordRec[][] is TMyProductionRecord (Public/MyProductionRecord.h); the leaves
//  call AddErrorRecord / AddInArmHotplatePickRecord / AddHPRecord /
//  GetInRotationAngRecord on it.
class TMyKitSuck
{
public:
    // data grids the leaves read/write (golden member names verbatim)
    TMySucker           Suck   [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :155
    int                 Item   [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :180
    int                 iWhichSite[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :188
    TMyProductionRecord PordRec[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :246

    // -- W6.2b ADD: members the in-arm ENGINE (ainarm9045.cpp) SMs deref --------
    //    golden MyKitSuck.h member names verbatim.  Added only because the
    //    engine task SMs / dispatch ladder read them; offline-safe defaults.
    int  iMaxRow;               // :159  (DoInArm_9045 guard `i<InArmSuck.iMaxRow`)
    // additional-function SM (DoInArmAdditionalFunction) "already done" flags
    bool bAlreadyDieClean;      // :220  (Die Clean done)
    bool bAlreadyPreciser;      // :221  (Preciser done)
    bool bAlready2DID;          // :222  (Bottom 2DID done)
    bool bAlreadyRotate;        // :223  (Rotate done)
    // pick SM / DoInArm_9045 predicates
    bool HasRealIC();           // :312  (any nozzle carries a real IC)
    // -- W6.2c(OUT-ARM) batch-3 ADD: golden MyKitSuck.h:294-295 (bodies .cpp:458/486).
    //    Left-/right-half real-IC predicate, used by DoOutArm_9045_2x5_8/2x6_8/2x8_8
    //    (FR/BRCarryKit.HasRealIC_Left/Right). Pure Item-grid scan, no HAL.
    bool HasRealIC_Left(int iRow, int Mid);   // :294
    bool HasRealIC_Right(int iRow, int Mid);  // :295
    bool NoIC();                // :316  (no nozzle carries any IC)
    //AI(W906-T6-HASNOTTEST) 20260923: golden MyKitSuck.h:302 / .cpp:1032 —— MainProc 停機臂的
    // CosFunction.bStopMustTestTimeOut 用它判斷 TestSocket 上還有沒有未測的 IC。只掃 Item 格，不碰硬體。
    bool HasNotTestYet();
    bool IsPickSuckFinish();    // :360  (all pick-suck done)
    bool IsPickDestroyFinish(); // :362  (all pick-destroy done)
    bool IsPickFinish();        // :364  (pick cycle finished)
    void ClearAll();            // :290  (clear the whole grid)
    void SetAllToNullIC();      // :286  (set every nozzle to NULL_IC)
    // -- W6.2c(OUT-ARM) ADD: golden MyKitSuck.cpp:369 (Steven 20241017): clear
    //    unused nozzles + HAS_NULL_IC -> NULL_IC over the pick grid.  Item-grid
    //    scan only (no HAL).  Used by DoOutArm_9045_<v> after FR/BRCarryKit pick.
    void SetUnuseAndHasNullICToNullIC();  // :279

    // scalar topology / shuttle-kit selectors (golden :167-205)
    int  iMaxCol;               // :160
    int  iModeX;                // :169
    int  iPickRow;              // :161
    int  iPickCol;              // :162
    int  iShtRow;               // :163
    int  iShtKitStep;           // :166
    int  iWhichSht;             // :200
    int  iWhichKit;             // :201
    int  iWhichShtPickFor32;    // :203
    int  iWhichKitPickFor32;    // :204
    int  HotCount;              // :205
    int  iWhichShuttleBackup;   // :234
    int  iWhichKitBackup;       // :235

    // -- W6.2c ADD: members the OUT-ARM ENGINE (aoutarm9045.cpp) derefs ----------
    //    golden MyKitSuck.h member names verbatim.  Added only because the
    //    out-arm safe-move / cell-pos / additional-fn SM read them.
    int  iMotRow;               // :156  (MoveOutArmToAutoSafe_9045 motor-grid walk)
    int  iMotCol;               // :157  (MoveOutArmToAutoSafe_9045 motor-grid walk)
    int  iPickStep;             // :166  (GetOutArmToShtCellPos col step)
    bool bAlreadyAOI;           // :351  (DoOutArmAdditionalFunction AOI done flag)
    bool bAlreadyFixAI;         // :353  (DoOutArmAdditionalFunction FixAI done flag)

    // -- W6.3 ADD: members the TRAY-ARM ENGINE (acatchtray.cpp) derefs ----------
    //    golden MyKitSuck.h member names verbatim.  CatchTraySuck.iWhichTray (15x:
    //    記錄Tray從哪來的) + IsShtSuckFinish()/IsShtDestroyFinish() (DoCatchTray
    //    early-out guard, golden acatchtray.cpp:6017-6018).
    int  iWhichTray;            // :194  (which tray this kit picked: 0 loader/1 empty/2 color/4 auto2)
    bool IsShtSuckFinish();     // :337  (shuttle-side suck finished -- offline true)
    bool IsShtDestroyFinish();  // :338  (shuttle-side destroy finished -- offline true)

    // -- W6.4 ADD: members the TESTER/INDEX ENGINE (atester.cpp) derefs ----------
    //    golden MyKitSuck.h member names verbatim.  The result-decode consumer
    //    (GetTesterResult / ProcessTestResult / CheckIndexArmInitState) walks the
    //    shuttle-site result grid via iShtRow/iShtCol (iShtRow already above) and
    //    stores decoded bin / barcode / sub-bin into iBinData / cDeviceInf / cSBin.
    int  iShtCol;               // :164  (decode loop column bound)
    int  iBinData [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM];  // :189  (decoded bin number)
    AnsiString cDeviceInf[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :228 (per-site 2DID/barcode)
    AnsiString cSBin     [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :230 (per-site software-bin label)

    // -- W6.5 ADD: members the SHUTTLE/CARRY ENGINE (acarry.cpp) derefs ----------
    //    golden MyKitSuck.h member names verbatim.  The shuttle clean-out /
    //    realIC->interface-bin conversion (Do_Auto_SHT1/2 + DoCheckShuttle*EmptyIC)
    //    writes iWhichAuto/bPass/bNeedReTest, reads the soak timer TSoakTime, and
    //    calls SetAllRealIC2InterfaceBin/CountRealIC/HasDefineIC.  All grids
    //    offline-zeroed in the ctor; the methods carry the FAITHFUL golden bodies
    //    (they touch only iMaxRow/iMaxCol/Item/iBinData -- all present here).
    int  iWhichAuto  [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :178 (per-site Auto-bin target)
    bool bPass       [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :187 (per-site pass flag)
    bool bNeedReTest [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // :188 (per-site retest flag)
    TQPF_Timer  TSoakTime;      // :242  (Index Pick up need wait Soak Time)
    void SetAllRealIC2InterfaceBin();   // :276  (faithful golden body)
    int  CountRealIC();                 // :315  (faithful golden body)
    bool HasDefineIC(int IC_TYPE);      // :297  (faithful golden body)

    TMyKitSuck();               // ctor: home iShtRow/iShtCol + init grids offline-safe

    // methods the leaves call (golden signatures preserved) -------------------
    void ResetAll();                                                    // :289
    bool HasIC();                                                       // :310
    bool HasType(int);                                                  // :324
    bool AlreadyTest();                                                 // :318 (W6.4: DoTestY case 50)
    bool AlreadyTestNotIncludeErrorBin();                              // :319 (W6.4)
    bool PartAlreadyTest();                                            // :320 (W6.4: DoTestY case 50)
    bool UseSiteHasIC();                                               // :340 (=W6.0 gated predicate leaf)
    bool UseSiteNoIC();                                                // :341
    bool UseSiteFullIC();                                             // :339 (W6.2b1x1: FAITHFUL -- iShtRow/iShtCol scan)
    bool LeftSideNoIC(int MiddleValue);                                // :349
    bool RightSideNoIC(int MiddleValue);                               // :348
    bool ArmUpSideNoIC();                                              // :352
    bool ArmDownSideNoIC();                                            // :353
    // -- W6.2b1x1 ADD: members the in-arm 1x1_1 variant SMs deref.  golden
    //    MyKitSuck.h names verbatim.  ArmUpSideAllTypeIC (golden :334) is a small
    //    HAL-free row scan -> FAITHFUL body.  SetPickerCount(7-arg, golden :250)
    //    is a topology setter -> FAITHFUL body (writes the present scalar members;
    //    iShtCnt/iPickKitStep added below because the golden body assigns them).
    bool ArmUpSideAllTypeIC(int IC_TYPE, int iOffset, int iCol);       // :334 (Steven 20220930)
    // -- W6.2c batch-2 ADD: twin of ArmUpSideAllTypeIC; golden MyKitSuck.h:335
    //    (Steven 20220930).  FAITHFUL down-side (Item[1][..]) row scan.  First
    //    ACTIVE consumer: GetShuttleState_2x2_4 / _2x2_4_14 (the gated 2x8_32
    //    sibling also derefs it).  AI(W6.2c-INARM-batch2) 20260626.
    bool ArmDownSideAllTypeIC(int IC_TYPE, int iOffset, int iCol);     // :335 (Steven 20220930)
    // -- W6.2c batch-3 ADD: per-row "has the specified IC?" query; golden
    //    MyKitSuck.h:298 `bool RowHasDefineIC(int iRow, int IC_TYPE);` (Steven
    //    20221005).  FAITHFUL pure Item[iRow][*] scan (no HAL).  First ACTIVE
    //    consumers: GetShuttleState_1x4_4 / _2x3_6 / _All_1Pick (InArmSuck.
    //    RowHasDefineIC(0/1, NULL_IC)); the gated 2x8_32 sibling also derefs it.
    //    AI(W6.2c-INARM-batch3) 20260626.
    bool RowHasDefineIC(int iRow, int IC_TYPE);                        // :298 (Steven 20221005)
    void SetPickerCount(int _iPickRow, int _iPickCol, int _iShtRow,
                        int _iShtCol, int _iPickStep, int _iKitStep,
                        int _iShtStep);                                // :250
    // AI(W906-P1b) 20260920: golden 的**兩參數多載**（MyKitSuck.cpp:218，
    //   Steven 20241002「TestSuck補上 Row / Col」）。
    //   它與七參數版不同：`iShtRow/iShtCol` 直接抄 `iPickRow/iPickCol`，
    //   而且**不碰** iPickStep / iPickKitStep / iShtKitStep。
    //   唯一的呼叫端是 golden `ainarm9045.cpp:1471 SetArmRowCount()`，
    //   它把 InArmSuck 的 shuttle 幾何推給 carry kit / TestSuck / TestSocket。
    //   ⚠ 沒有這個多載，那幾個物件的 iShtRow/iShtCol **一輩子是 ctor 值**，
    //     而 golden 那邊是跟著機型 layout 走的。
    void SetPickerCount(int _iPickRow, int _iPickCol);                 // golden MyKitSuck.cpp:218
    int  iShtCnt;                // :165 (=iShtRow*iShtCol; SetPickerCount writes it).
                                 //       iPickKitStep already declared below (W6.2b 2x4_16).
    void SetItemData(int iRow, int iCol, int data, int iTarget=-1);    // :283
    // AI(W906-HOME-C2) 20260920: golden MyKitSuck.h:277 `void SetAllHASIC2ErrorBin();`
    //   把這個 kit 上每一顆 HAS_IC 的料改判成「錯誤 bin」（TEST_PASS+iTestBinCount）
    //   並清掉 pass / 待重測旗標。golden MyKitSuck.cpp:2777-2792。
    //   第一個消費者：uhome.cpp ProcessMotorHome case 2 的 `CUSTOMER_CODE==CC_SCK`
    //   臂 —— 那家客戶要求歸零時把測試站上的殘料直接判成錯誤 bin，而不是逐顆回收。
    void SetAllHASIC2ErrorBin();                                      // :277
    void CopyFromTray(int iSuckR, int iSuckC, int iSuckData, class TTrayMotor &Mot,
                      int TrayR, int TrayC, int iTrayData, int iTarget=-1,
                      bool bHP2Inarm=false);                            // :276
    void CopyToTray  (int iSuckR, int iSuckC, int iSuckData, class TTrayMotor &Mot,
                      int TrayR, int TrayC, int iTrayData, int iTarget=-1); // :277

    // -- W6.2b(2x4_16) ADD: golden TMyKitSuck members the 2x4_16 in-arm SMs deref.
    //    All are genuine golden MyKitSuck.h API (cited per line); guarded so a
    //    parallel sibling variant editing this header does not double-declare.
#ifndef HT9045_KITSUCK_2x4_16_ADDED
#define HT9045_KITSUCK_2x4_16_ADDED
    int  iXStep;                // golden MyKitSuck.h:169 (X pitch 要設幾格)
    int  iYStep;                // golden MyKitSuck.h:170 (Y pitch 要設幾格)
    int  iPickKitStep;          // golden MyKitSuck.h:167
    void SetAll(int Type);      // golden MyKitSuck.h:274 / .cpp -- set every nozzle to Type
    bool All_HasIC();           // golden MyKitSuck.h:307 / .cpp -- every used nozzle has IC
    // golden has the type1->type2 remap; in golden it is referenced only by the
    // 2x4_16 family and never defined (dead-by-design); we supply the obvious
    // faithful body (remap Item==Type1 -> Type2 over the pick grid) offline-safe.
    void SetType1ToType2ByPickCol(int Type1, int Type2); // golden-by-name (2x4_16-local)
#endif

    // -- W6.2c(1x2_4_Hot) ADD: golden TMyKitSuck "have real IC" side-scan API the
    //    1x2_4_Hot in-arm place-to-shuttle SM derefs (golden mykitsuck.cpp).  All
    //    are genuine golden API (cited per line); guarded so a parallel sibling
    //    variant editing this header does not double-declare.  FAITHFUL bodies are
    //    in aHotPlateSubstrate.cpp (Item-grid scans only -- no HAL).
#ifndef HT9045_KITSUCK_HAVEREALIC_ADDED
#define HT9045_KITSUCK_HAVEREALIC_ADDED
    bool ArmUpSideHaveRealIC(bool left=true);    // golden mykitsuck.cpp (ChungHung 20130708 left=true)
    bool ArmDownSideHaveRealIC(bool left=true);  // golden mykitsuck.cpp (ChungHung 20130708 left=true)
    bool ArmLeftSideHaveRealIC(int MiddleValue); // golden mykitsuck.cpp
    bool ArmRightSideHaveRealIC(int MiddleValue);// golden mykitsuck.cpp (For 1x2 & 2x2)
#endif

    // -- W6.2c(2x2_8_Hot) ADD: golden TMyKitSuck "NULL_IC -> HAS_NULL_IC" grid
    //    promotion the 2x2_8_Hot in-arm place-to-shuttle SM derefs (4 call sites:
    //    FLCarryKit x2 / BLCarryKit x2).  Genuine golden API (MyKitSuck.h:280 /
    //    MyKitSuck.cpp:340-352).  FAITHFUL body in aHotPlateSubstrate.cpp (pure
    //    Item-grid scan over iShtRow/iShtCol -- no HAL).  Guarded so a parallel
    //    sibling variant editing this header does not double-declare.  NOTE: a
    //    same-named method also exists on TTrayMotor (Motor/mymotor.h:359) -- a
    //    DIFFERENT class; this one is TMyKitSuck (FLCarryKit/BLCarryKit).
#ifndef HT9045_KITSUCK_SETNULLIC2HASNULLIC_ADDED
#define HT9045_KITSUCK_SETNULLIC2HASNULLIC_ADDED
    void SetNullIcToHasNullIc();                 // golden MyKitSuck.h:280 (Steven 20150203)
#endif

    // AI(W906-AutoCleanFoundation) 20260721: golden MyKitSuck.h data members the
    // AutoClean foundation wave's HAL-only helpers (RestoreCleanKitData /
    // CleanPad_PlaceToShuttle / CleanPad_PickFromShuttle / DoPlaceToKitSwapData /
    // PickFromCleanKit / PlaceToCleanKit / MoveInArmZ_Shuttle_Pick) deref, plus
    // the MoveSuckDataDiff move method golden MyKitSuck.cpp:1503-1561 they call.
    // Every field below is genuine golden MyKitSuck.h API (cited per line); pure
    // data, zero HAL. Guarded so a parallel sibling wave does not double-declare.
#ifndef HT9045_KITSUCK_AUTOCLEAN_ADDED
#define HT9045_KITSUCK_AUTOCLEAN_ADDED
    int  iNeedSuck      [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :185 (0=不需要吸, 其他=IC type)
    int  iAutoCleanRecX [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :244 (jou 2013-03-13)
    int  iAutoCleanRecY [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :245
    int  iWhichIndex    [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :184 (ChungHung 20150205, ATK)
    int  iNeedRotAng    [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :179
    int  iCurrRotAng    [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :180
    bool bQATray        [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :181
    int  iCleanCount    [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :182 (TMyKitSuck's OWN copy -- distinct from TMyTray::iCleanCount, different class)
    bool bFliped        [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :183
    int  iBinDataBackUp [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :190 (Sam 20180612)
    AnsiString cReDeviceInf[_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :229
    bool b2DIDNG        [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :234 (Steven 20200611)
    int  iAOIResult     [_MAX_SUCK_ROW_ITEM][_MAX_SUCK_COL_ITEM]; // golden :235 (Sam 20240325)
    // golden MyKitSuck.h:269 -- MOVES one nozzle's full record (Item + every grid
    // above + PordRec[][].asBuffer->CommaText/bUse) from Source[SourceR][SourceC]
    // into this[TargetR][TargetC], then clears the Source slot back to empty.
    // FAITHFUL body in aHotPlateSubstrate.cpp.
    void MoveSuckDataDiff(class TMyKitSuck &Source, int SourceR, int SourceC, int TargetR, int TargetC);
    // AI(W906-AutoCleanCluster) 20260728: golden MyKitSuck.h:268 -- the SAME-
    // POSITION sibling overload (TargetR/TargetC default to -1, meaning "use
    // SourceR/SourceC"). DISCOVERED GAP (4th, beyond the 3 named in this
    // wave's task brief): DoIndexAutoClean/DoIndexAutoClean_Arm1PickArm2Test
    // call `FTestSuck.MoveSuckData(FLCarryKit, i, j)` (2-target-arg form)
    // pervasively and LOAD-BEARINGLY (it is how a clean IC actually moves
    // between FLCarryKit/FTestSuck/TestSocket/BLCarryKit/BTestSuck as the
    // clean cycle progresses -- unlike the other TUs that already worked
    // around this same gap with a local no-op stub (aTester_Rear.cpp's
    // W64bT2_MoveSuckData / aTester_Front.cpp's W64B_MoveSuckData /
    // atester_32Site.cpp's W5_32S_MOVESUCKDATA), a no-op here would silently
    // break AutoClean's own item-tracking state, contradicting this project's
    // "never silently drop functionality" rule). ZERO new dependencies: every
    // field below is already real (identical set MoveSuckDataDiff already
    // moves, faithfully, right above). Ported for REAL, matching golden
    // MyKitSuck.cpp:1443-1501 body exactly (FAITHFUL body in
    // aHotPlateSubstrate.cpp) -- same treatment this wave's task brief
    // explicitly authorizes for SetTechDataToProd_AutoClean (land it for real
    // when it has zero further dependencies, rather than stub it).
    // GOLDEN QUIRK preserved: MoveSuckData ALSO calls
    // `Source.PordRec[SourceR][SourceC].InitialRecord()` right after clearing
    // the source slot -- MoveSuckDataDiff does NOT do this (verified by
    // reading both golden bodies side by side, MyKitSuck.cpp:1443-1501 vs
    // :1503-1561). A real, golden-verified asymmetry between the two
    // "identical-looking" methods, not a translation slip.
    void MoveSuckData(class TMyKitSuck &Source, int SourceR, int SourceC, int TargetR=-1, int TargetC=-1);
#endif

    // AI(W906-AutoCleanCluster) 20260722: golden TMyKitSuck members the AutoClean
    // pick/place-engine cluster (DoAutoCleanPickfromCleanKit / DoAutoCleanKit /
    // DoShuttle1/2AutoClean) derefs, additive to the Wave-14 AutoClean-foundation
    // set above. Every one is genuine golden MyKitSuck.h/.cpp API (cited per
    // line), pure Item-grid scans -- zero HAL, same idiom as ArmUpSideAllTypeIC/
    // RowHasDefineIC above.  Guarded so a parallel sibling wave does not
    // double-declare.
#ifndef HT9045_KITSUCK_AUTOCLEAN_CLUSTER_ADDED
#define HT9045_KITSUCK_AUTOCLEAN_CLUSTER_ADDED
    // golden MyKitSuck.cpp:306 (Steven 20250420) -- "every used nozzle over the
    // FULL pick grid (iMaxRow/iMaxCol) is one of the two given IC types".
    bool ArmAll_HasICType(int IC_TYPE1, int IC_TYPE2);   // golden MyKitSuck.h (mykitsuck.cpp:306)
    // golden MyKitSuck.cpp:323 -- same predicate scoped to the SHUTTLE-side grid
    // (iShtRow/iShtCol) -- "this one is used on In/Out arm" per its own comment.
    bool ShtAll_HasICType(int IC_TYPE1, int IC_TYPE2);   // golden MyKitSuck.h (mykitsuck.cpp:323)
    // golden MyKitSuck.cpp:238 (kevin 20120531) -- "is there an empty (NULL_IC)
    // slot anywhere over the FULL pick grid". Genuinely missing (grepped) before
    // this addition; needed by DoAutoCleanKit's FLCarryKit/BLCarryKit.FindNoIC()
    // call sites (golden AutoClean.cpp:2688/3339).
    bool FindNoIC();                                     // golden MyKitSuck.h:291 (mykitsuck.cpp:238)
#endif
};
#endif  // AI(W906-A4-6) 20260924: 精簡鏡像退役區段結束；下面的 extern 與 mykitsuck.h 的宣告型別相同（重複宣告合法）
extern TMyKitSuck InArmSuck;     // golden MyKitSuck.h:357
extern TMyKitSuck FLCarryKit;    // golden MyKitSuck.h:358
extern TMyKitSuck BLCarryKit;    // golden MyKitSuck.h:360
extern TMyKitSuck OutArmSuck;    // golden MyKitSuck.h:366
// -- W6.2c ADD: the OUT-ARM KitSuck objects the out-arm engine SMs read --------
extern TMyKitSuck OutArm2Suck;   // golden MyKitSuck.h:366 (HT-9046AU sort arm; DoOutArmAfterPlaceToAuto case 5000)
extern TMyKitSuck FRCarryKit;    // golden MyKitSuck.h:359 (DoOutArmIonFanGiveWay FRCarryKit.HasIC())
extern TMyKitSuck BRCarryKit;    // golden MyKitSuck.h:361 (DoOutArmIonFanGiveWay BRCarryKit.HasIC())
// -- W6.3 ADD: the TRAY-ARM KitSuck object the catchtray engine SMs read --------
extern TMyKitSuck CatchTraySuck; // golden MyKitSuck.h:367 (TrayArm dual-suck grid; DoCatchTray/DoCatchFromLoader/...)
// -- W6.4 ADD: the TESTER/INDEX KitSuck objects the tester engine SMs read -------
extern TMyKitSuck TestSocket;    // golden MyKitSuck.h:368 (the test-result grid; decode/ProcessTestResult target)
extern TMyKitSuck FTestSuck;     // golden MyKitSuck.h:363 (front test-head suck grid)
extern TMyKitSuck BTestSuck;     // golden MyKitSuck.h:364 (rear  test-head suck grid)

// ============================================================================
//  (b) uPlateInfo (PickFromHPList)  -- golden HTEditList.h
//      uPoint2D is ALREADY declared in cmydef.h (W0-TAIL stub, golden HTEditList.h
//      :323, same {int X; int Y;} layout); its ctor/Clear bodies were deferred --
//      aHotPlateSubstrate.cpp supplies them for W6.2.  Do NOT redeclare it here.
// ============================================================================
// AI(W906-AutoCleanFoundation) 20260721: golden HTEditList.h:128 -- opaque
// forward decl only.  uPlateInfo::ExtractFirstTeam() (below) returns
// uHPSuckTeam*; MoveInOutArmZToKitPickPlace (AutoClean.cpp) only null-checks
// the returned pointer and never dereferences it, so the full uHPSuckGroup/
// uHPSuckTeam list-bookkeeping class body stays out of scope for this wave
// (real translation is a separate, larger future front) -- an incomplete
// type is sufficient and honest about what is/isn't implemented.
class uHPSuckTeam;

// AI(W906-AutoCleanCluster-review) 20260723: GAP, not a settled invariant --
// every method below (AddHPSuckGroup/UpdateHPSuckGroup/SetArrPlateXY/
// GetHPFirstTeam*/ExtractFirstTeam/ClearGroupList) is an unconditional no-op;
// this class holds no TList at all, so "the team list is always empty" is
// true ONLY because nothing can ever write to it -- not because of any real
// offline invariant. Golden's HotPlate-place branch in DoAutoCleanKit
// (AutoClean.cpp:3960/3993/4893, gated by IniConfig.bE43AutoCleanUseHotplate,
// a real runtime feature flag, not offline-only) depends on
// GetHPFirstTeamMotUse() returning TRUE to find where to place/pick; since it
// permanently returns false here, that branch would unconditionally fail
// (`ShowMyMessage(...); return false;`) the moment bE43AutoCleanUseHotplate=
// true is exercised. Zero live effect today (DoAutoCleanKit's only call site
// is still `#if 0`-gated in csystem.cpp per the W7 wiring TODO), and
// test_AutoClean.cpp only exercises bE43AutoCleanUseHotplate=false -- but a
// real HPSuckGroupList/HPSuckTeamList implementation (golden
// Public/HTEditList.cpp:2454/2617-2628 ClearTeamList/ClearGroupList,
// :2459-2467 AddHPSuckGroup) must land here before any bE43AutoCleanUseHotplate
// =true configuration can work, once the W7 gate above is ever removed.
// uPlateInfo: MINIMAL mirror -- only the called methods (golden HTEditList.h:244)
class uPlateInfo
{
public:
    uPlateInfo();
    ~uPlateInfo();
    // 取得第一筆資料的全部吸嘴資料
    bool GetHPFirstTeam(int *iP, int (*PlateR)[MAX_ARM_Col],
                        int (*PlateC)[MAX_ARM_Col], bool (*Suck)[MAX_ARM_Col]);
    // 取得第一筆資料的加熱盤座標與 Shuttle 位置
    bool GetHPFirstTeamPlate(int &iP, int &iR, int &iC, int &iSht, int &iKit);
    // 取得第一筆資料中, 吸嘴資料是否需要交換
    bool GetHPFirstTeamSuckUse(bool bSuck[MAX_ARM_Row][MAX_ARM_Col]);
    // 取得第一筆資料中, 吸嘴 Z 軸使用與否的資料
    bool GetHPFirstTeamMotUse(bool bSuck[MAX_ARM_Row][MAX_ARM_Col]);
    // 取得第一筆資料, 並轉換成陣列
    bool GetHPFirstTeamToList(int *iP, TList* lsPoint2D);
    // 確認是否需要多次吸料, 並刪除舊資料
    bool DataForwardAndNextTeam();
    // 編輯已儲存資料
    bool SetPlateSuck(int _iP, int _iRow, int _iCol, bool bSuck);
    void SetArrPlateXY(int iSuckRow, int iSuckCol, int _iP, int iPlateR,
                       int iPlateC, int _iSite=-1);
    void SaveFile(AnsiString sFileName);
    // -- W6.2b1x1 ADD: HP-suck-group recorders the in-arm HP-place SM
    //    (DoPlaceToHotPlate_9045_1x1_1) calls (golden HTEditList.h:202/204).
    //    Offline: list bookkeeping no-ops (the placed-group record is not
    //    consumed offline; the SM only needs the calls to be linkable/no-throw).
    void AddHPSuckGroup();                                              // golden HTEditList.h:202
    void UpdateHPSuckGroup(int iP, int iR, int iC, int iSht, int iKit); // golden HTEditList.h:204
    // AI(W906-AutoCleanFoundation) 20260721: golden HTEditList.h:233 -- "pull the
    // first team out of the group list, if any" (uHPSuckGroup ownership move).
    // Offline: matches the sibling GetHPFirstTeam* determinism (team list is
    // always empty offline) -> always NULL, i.e. "no team to extract".
    uHPSuckTeam* ExtractFirstTeam();
    // AI(W906-AutoCleanCluster) 20260722: golden HTEditList.h -- "throw away the
    // whole group list" (sibling of AddHPSuckGroup/UpdateHPSuckGroup above).
    // DoAutoCleanKit calls PlaceToCleanList->ClearGroupList() once, right at its
    // Task==1 entry (golden AutoClean.cpp:4509), to guarantee no stale group
    // survives a restart. Offline: same "list bookkeeping no-op" idiom as its
    // AddHPSuckGroup/UpdateHPSuckGroup siblings -- see the GAP note on the
    // class banner above: this is a no-op because nothing populates the list,
    // not a settled offline invariant.
    void ClearGroupList();   int GetHPSuckGroupCount();                 // golden HTEditList.h   //AI(W906-W188) 20261009 (NB2-1): golden HTEditList.h -- declaration only; the one definition is Public/HTEditList.cpp (W-188 #3 orphan log)
};

extern uPlateInfo *PickFromHPList;   // golden HTEditList.h:294

// AI(W906-AutoCleanFoundation) 20260721: golden HTEditList.h:247 -- a SEPARATE
// uPlateInfo* instance from PickFromHPList (golden main.cpp:2150 `new`s each
// independently). AutoClean's shuttle-place/pick geometry (MoveInOutArmZToKitPickPlace/
// MoveInArmXYPickCleanKit/PlaceToCleanKit) needs its own team list, distinct
// from the in-arm HotPlate engine's PickFromHPList. NOTE: csystem.cpp's W7C1
// wave already defines an UNRELATED same-named macro `PlaceToCleanList` scoped
// to its own translation unit only (a throwaway ->ResetFile()-only seam,
// csystem.cpp:936-942) -- that macro does not leak into other TUs, so it does
// not collide with this real extern (any TU that includes THIS header, like
// AutoClean.cpp, sees the genuine uPlateInfo* below, not csystem.cpp's local stub).
extern uPlateInfo *PlaceToCleanList; // golden HTEditList.h:247

// AI(W906-BA-MW6) 20260911: the `class uPlateInfo` ABOVE in this header is a
// ZERO-DATA-MEMBER FACADE (sizeof == 1). The real one is at
// Public/HTEditList.h:321 (x64 sizeof == 24).
// ** NEVER WRITE `new uPlateInfo()` FROM THIS HEADER'S POINT OF VIEW ** -- that
// allocates one byte and hands it to a constructor that writes 24, which is
// MW-6: heap corruption twice on every boot.
// Always use this factory; its body lives in Public/HTEditList.cpp, which sees
// the real class. The return type does not participate in name mangling, so
// both declarations refer to the same symbol _Z18W906_NewPlateInfov.
uPlateInfo *W906_NewPlateInfo();

extern AnsiString sHPPickRec;            // golden HTEditList.h:16 (used in SOFT_SIMULTE-gated SaveFile)
extern AnsiString sHPPickRecException;   // golden HTEditList.h:17

// ============================================================================
//  (c) ainarm2.h cursors / arrays  -- golden ainarm2.h
// ============================================================================
extern const bool ZAxisDown;                                            // golden ainarm2.h:42

extern int iPickPlate[2],  iPickPlateX[2],  iPickPlateY[2];             // golden ainarm2.h:62
extern int iPlacePlate[2], iPlacePlateX[2], iPlacePlateY[2];           // golden ainarm2.h:63

extern bool InArmSuckUse[MAX_ARM_Row][MAX_ARM_Col];                    // golden ainarm2.h:67
extern bool bPickFromHotplate;                                          // golden ainarm2.h:53
extern bool bInArmCheckDestroyACT[MAX_ARM_Row][MAX_ARM_Col];           // golden ainarm2.h:39
// W6.2c ADD: 1x2_* HotPlate XDivision==3 flag pair shared by 1x2_2/1x2_2_14/
// 1x2_4_Hot in-arm SMs.  Single canonical decl (def in aHotPlateSubstrate.cpp);
// guarded so a sibling variant's local extern is harmless.  AI(W6.2c-INARM) 20260626.
#ifndef HT9045_flag1_1x2_2_DECLARED
#define HT9045_flag1_1x2_2_DECLARED
extern bool flag1_1x2_2[2];                                            // golden ainarm2.cpp:41 / ainarm2.h:9
#endif

extern int iHotCount;                                                   // golden ainarm2.h:77 (同一次 Shuttle 的 IC 計數)
extern int iHotPlateCount  [2][50][50];                                 // golden ainarm2.h:78
extern int iHotWhichKit    [2][50][50];                                 // golden ainarm2.h:83
extern int iHotWhichShuttle [2][50][50];                                // golden ainarm2.h:84

// AI(W906-AutoCleanFoundation) 20260721: golden ainarm2.h:87/175-177 (REAL def:
// ainarm2.cpp) -- the AutoClean pick-plan cursors SearchCleanKitRowCol /
// SearchCleanKitUpDown / MoveInArmXYPickCleanKit / PickFromCleanKit read+write,
// and the per-nozzle "am I committed to this AutoClean pick?" grid the same
// functions (+ MoveInArmZToShuttlePlace) gate on. Verified absent from the
// target tree before adding (grepped) -- genuinely missing, not a duplicate.
extern int  iAutoCleanStart;                       // golden ainarm2.h:87  (REAL def ainarm2.cpp:91, =0)
extern int  iAutoCleanPickPlateX, iAutoCleanPickPlateY; // golden ainarm2.h:176 (REAL def ainarm2.cpp:105-106)
extern int  iAutoCleanUseXPitch;                   // golden ainarm2.h:177 (REAL def ainarm2.cpp:107, =0)
extern bool bInArmSuckActive[MAX_ARM_Row][MAX_ARM_Col]; // golden ainarm2.h:175 (REAL def ainarm2.cpp:101, all false)
extern bool bPlaceToCleanKit;                      // golden ainarm2.h:57 (REAL def ainarm2.cpp:49, =false) -- Steven 20171204 (Wei)
// AI(W906-AutoCleanCluster) 20260722: golden ainarm2.h:88 (REAL def ainarm2.cpp:92,
// `int iAutoCleanNum=0;`) -- "which AutoClean pad position is currently the
// least-used" cursor SearchiAutoCleanNum computes and
// DoAutoCleanPickfromCleanKit resets on Restart. ainarm2.cpp/.h are NOT this
// wave's scope (out-of-bounds file), so -- matching the identical pattern
// already used for iAutoCleanStart/iAutoCleanPickPlateX/Y/iAutoCleanUseXPitch/
// bInArmSuckActive/bPlaceToCleanKit above -- homed here instead. Verified
// absent from the target tree before adding (grepped): genuinely missing.
extern int  iAutoCleanNum;                         // golden ainarm2.h:88 (REAL def ainarm2.cpp:92, =0)

extern bool bPitchOver12000;                                            // golden ainarm2.h:162 (jou 20100120)

//AI(W6.2c-INARM-batch3) 20260626: ainarm2 debug latch the in-arm variant SMs
// clear (golden ainarm2.cpp:52 `bool bInArmHasHotIC=false;` / decl ainarm2.h:60).
// First ACTIVE consumers: DoInArm_9045_2x3_6_14 + DoInArm_9045_2x4_4 (both set it
// false; pure debug flag, NO offline hardware effect).  Defined once in
// aHotPlateSubstrate.cpp.  The HT9045_SUBSTRATE_bInArmHasHotIC guard matches the
// guarded forward-decl those variant .cpp files carry, so theirs collapses here.
#ifndef HT9045_SUBSTRATE_bInArmHasHotIC
#define HT9045_SUBSTRATE_bInArmHasHotIC
extern bool bInArmHasHotIC;                                             // golden ainarm2.h:60 (Sam 20211012 : Debug 用)
#endif

// strAUTOSITEMAP (golden ainarm2.h:10) + InArmSiteMapData (golden ainarm2.h:34)
typedef struct                                                          //Sam 20181201 : Auto Alignment
{
    int iP;
    int iPlateR;
    int iPlateC;
    int iSuckR;
    int iSuckC;
    void ClearData()
    {
        iP=-1;
        iPlateR=-1;
        iPlateC=-1;
        iSuckR=-1;
        iSuckC=-1;
    };
    void UpdateData(int _iP, int _iPlateR, int _iPlateC, int _iSuckR, int _iSuckC)
    {
        iP      =_iP;
        iPlateR =_iPlateR;
        iPlateC =_iPlateC;
        iSuckR  =_iSuckR;
        iSuckC  =_iSuckC;
    };
}strAUTOSITEMAP;
extern strAUTOSITEMAP InArmSiteMapData;                                 //Steven 20211209 : 紀錄Site map資料

// ============================================================================
//  (e) Rotate / Laser micro-shims  -- golden aRotateKIT.h + laser-distance list
// ============================================================================
//  tRotate.ActiveRotate -- the only member the place-swap leaf reads.
//  //AI(W906-FRW-NoPage) 20260926: tRotateShim 退役成 golden 完整型別 TRotate 的別名（golden RotateKit/fRotate.h:28-59，
//    逐字搬到 forms/fRotate.h）。原本這裡只有 ActiveRotate／RotateDutDate[4][4][8]／bRotateUseRTmode 三個成員；
//    golden TFrmRotate::fRotate_ReadFile（Rotate.Data → tRotate，現在是 FileRW/Rotate.gen.inc）要寫全部成員。
//    三個舊成員名稱與型別不變，既有讀者（ainarm9045*.cpp、aRotateKIT*.cpp、cinitial.cpp）照舊編；
//    結構變大 → 所有 include 本檔的 TU 要重編（標頭相依會處理）。定義仍在 aHotPlateSubstrate.cpp:1077
//    （初值 `= { false }` 改成同值的 `= {}`：ActiveRotate=false、其餘成員零初始化）。
//    aRotateKIT_In.cpp:338-394／aRotateKIT_Out.cpp 的 GATE 巨集（W906RKIN_DUTNUM 等「tRotateShim 沒有這個成員」）
//    前提因此過期；要不要解開是 Jimmy 的底層流程，本次不動。
#include "forms/fRotate.h"      // struct TRotate、extern TRotate tRotate（無其他相依）
typedef TRotate tRotateShim;    // golden RotateKit/fRotate.h (TRotate tRotate)
extern tRotateShim tRotate;

//  TInLaserCheck + LaserCheckPos -- golden laser-distance check list.
//  Place-swap leaf only does LaserCheckPos.push_back(new TInLaserCheck(iP,iC,iR)).
class TInLaserCheck
{
public:
    TInLaserCheck(int iP, int iPlateC, int iPlateR);
    int iP;
    int iPlateC;
    int iPlateR;
};
#include <vector>
extern std::vector<TInLaserCheck*> LaserCheckPos;

// ============================================================================
//  (d) in-arm engine helper externs  (bodies live in the later ainarm core,
//      W6.x/W7; offline sim bodies are in aHotPlateSubstrate.cpp)
// ============================================================================
// These overlap with ainarm9045.h (the engine's public header, where they now
// have REAL bodies + default args).  When ainarm9045.h is already included
// (e.g. in ainarm9045.cpp) skip the redeclaration so the default args are not
// given twice.  The already-translated leaves include only this header and rely
// on the InArmLeftSide* default arg (=2), so keep the decls for them.
#ifndef ainarm9045H
extern bool bUseAxExPicker();                                           //golden ainarm9045.h
extern bool bUseAxxGPicker();                                           //golden ainarm9045.h
extern bool InArmLeftSideHasIC(int iRow=2);                             //golden ainarm9045.h:109
extern bool InArmLeftSideNoIC(int iRow=2);                              //golden ainarm9045.h:108
#endif
extern void ResetShuttleWhichKit();                                     //golden ainarm2.h:133 (Steven 20140710)

// per-site close-site-mode selectors referenced by HotPlateYPitchCanPutAll().
// Golden homes: ainarm9045_2x6_8.h / ainarm9045_2x8_8.h (per-site modules, W6.x/W7).
// e2x8OneByOne already lives in target MachineType.h; only these are missing:
extern int iCloseSiteModeFor2x6;        //golden ainarm9045_2x6_8.h:23 (Steven 20240417)  (REAL def: ainarm9045_2x6_8.cpp)
extern int iCloseSiteModeFor2x8;        //golden ainarm9045_2x8_8.h:28 (JerryYang 20190729) (REAL def: ainarm9045_2x8_8.cpp)
//AI(W6.2c-INARM-batch4) 20260626: e2x6OneByOne now lives in the FULL `enum e2x6Mode`
// owned by ainarm9045_2x6_8.h (the per-site variant landed ACTIVE this batch).
// Keep this bare e2x6OneByOne=2 placeholder ONLY for TUs that include
// aHotPlateSubstrate.h but NOT ainarm9045_2x6_8.h (acarry.cpp:3252 +
// ainarm_SearchPlacePlate.cpp:302).  Guard it out when the variant header is
// present (it is included BEFORE this header in ainarm9045_2x6_8.cpp) so the
// enumerator is not redeclared; value (2) is identical so numeric behavior is
// unchanged.  (Mirrors the e1x4CloseAbAc / ainarm9045_1x4_4H guard above.)
#ifndef ainarm9045_2x6_8H
enum { e2x6OneByOne = 2 };              //golden ainarm9045_2x6_8.h:10 (enum e2x6Mode)
#endif
// -- W6.2c(2x2_8_Hot) ADD: engine-sibling row-dual-site predicate the 2x2_8_Hot
//    place-to-HP SM derefs (3 sites: GetPlaceHotPlate_4/_8/_8All).  Golden home
//    ainarm2.cpp:1554 (ainarm2.h).  FAITHFUL offline body in aHotPlateSubstrate.cpp
//    returns false (the dominant golden outcome: 3 of 4 returns are false; the only
//    `true` early-out is gated on ArmCanSuck4IC(0)==false which the Sim HAL does not
//    model).  2x2_8_Hot.cpp also forward-declares this under #ifndef
//    ROWCANDUALSITE_DECLARED so it compiles standalone; the two coexist (same proto).
extern bool RowCanDualSite();                                          //golden ainarm2.cpp:1554 (ChungHung 20150528)
// -- W6.2c(2x8_8) ADD: out-arm 2x8 special-close-site map WRITTEN-TO by the in-arm
//    2x8_8 CheckSTMMode_2x8_8 (5 sites).  Golden home aoutarm9045_2x8_8.h:7 /
//    defined aoutarm9045_2x8_8.cpp:31 (JerryYang 20250711) -- the out-arm 2x8_8
//    wave is NOT translated yet, so home an OFFLINE definition here (zero-init) so
//    the in-arm family links now; when the out-arm 2x8_8 file lands it OWNS the def
//    and this offline one is removed.  e2x8ModeTotal comes from MachineType.h.
extern int XPHSuckToSht_2x8_8_OutArm[e2x8ModeTotal][2][8];             //golden aoutarm9045_2x8_8.h:7 (JerryYang 20250711)
extern int  CloseSiteState(bool bPlace=true);                          //golden ainarm2.h:156
extern void InitInArmTask();
// AI(W906-P10) 20260921: 簽章改回 **golden 的**（main.cpp:363）：
//   `void SetRunStartMode(eRunStartMode Mode, AnsiString ModeText)`
//   —— `Mode==rsmNull` 時用 `ModeText` 決定模式（golden :429 走 GetRunStartModeNum）。
//
//   ⚠ 原本是 `SetRunStartMode(int iMode)`，那是配合空樁的簡化。
//     實測全樹 36 個活呼叫端**全部傳 `rsm*` 列舉、全部單參數**
//     （rsmAutoSiteMap 14 / rsmInitial_ART 13 / rsmContinuRetest_ART 12 …），
//     所以加上預設值之後**呼叫端一行都不用動**。
//   ⚠ 這個改動是**連結期**才看得到的：改簽章而沒改宣告時，
//     `-fsyntax-only` 兩組態都回 0 error，全量建置才噴 98 個
//     `undefined reference to SetRunStartMode(int)`。
extern void SetRunStartMode(eRunStartMode Mode = rsmNull, AnsiString ModeText = "");
extern void TransferHotPlateRatio(bool bPick, int *iX, int *iY);        //Steven 20110324
extern void InspectInArmPosition(int iMot, int iSuckR, int iSuckC,
                                 int iPlateR, int iPlateC, bool bPlace);
extern bool MoveInArmZToPlateSafe(int Task);
extern bool IsCheckInArmDestroyActiveFinish();                          //ChungHung 20111229
extern void DoInArm_9045_SuckerMap();                                   //Steven 20220531
extern void SetShuttleToHasNullIC_9045(int iSht, int iKit);
extern void AdjustShuttlePlaceOrder(int iReset=-1);
// AI(pt-wave) 20260811 PT-W7e: arity corrected to golden ainarm2.h:136
//   `extern void SetInArmHome(bool bPrecisorNeedHome=false);`
//   The body here is still the zero-behaviour no-op stub (aHotPlateSubstrate.cpp:1067); only the
//   SIGNATURE changes, so this is behaviour-neutral. It is needed because golden calls it BOTH
//   ways -- bare at golden ainarm2.cpp:1060/:1073/:1091/:1198 and with an argument at :3152 --
//   and PT-W7e landed the :3152 caller, which failed to link against the zero-arity stub.
//   Changing the single canonical declaration (rather than adding a second overload) is what
//   keeps every existing bare call unambiguous.
//   golden REAL body is ainarm2.cpp:1266 and is still NOT translated.
extern void SetInArmHome(bool bPrecisorNeedHome=false);
extern void AddInArmPickerCount(int iSuckR, int iSuckC);
extern void InArmSubSpeed();
extern void InArmAddSpeed();
// NOTE (W6.2b): GetInArmPitchX_9045 / GetInArmPitchY_9045 / bUseAxExPicker /
//   bUseAxxGPicker / InArmLeftSideHasIC / InArmLeftSideNoIC / InspectInArmPosition
//   / IsCheckInArmDestroyActiveFinish / DoInArm_9045_SuckerMap /
//   SetShuttleToHasNullIC_9045 / AddInArmPickerCount are now REAL-DEFINED in
//   ainarm9045.cpp (the in-arm engine).  Their offline stub bodies were removed
//   from aHotPlateSubstrate.cpp to avoid ODR/link collisions.  The extern decls
//   for those symbols remain valid (same prototypes); the two pitch helpers the
//   already-translated leaves call are forward-declared here so they keep
//   compiling against the engine definition:
#ifndef ainarm9045H   // default args live in ainarm9045.h; leaves pass all args explicitly
extern int  GetInArmPitchX_9045(int iMovePitchX, int i, int iOffsetPos);        //golden ainarm9045.h:23 (REAL home: ainarm9045.cpp)
extern int  GetInArmPitchY_9045(int iMovePitchY, int iOffsetPos);               //golden ainarm9045.h:25 (REAL home: ainarm9045.cpp)
#endif
extern void StopAllMotor();
extern void MyDBIProcess(AnsiString S1, AnsiString S2);
extern int        W906_MyDBIProcess_Count;   // AI(W906-GA1-B2-integrate) 20260804: observation seam
extern AnsiString W906_MyDBIProcess_LastS1;
extern AnsiString W906_MyDBIProcess_LastS2;
void W906_MyDBIProcess_Reset();

// ============================================================================
//  (A) [W6.2b] in-arm ENGINE cursors owned by not-yet-translated ainarm2.cpp.
//      The central pick SM binds `int &Task=iPickFromLoadStageTask`; DoInArm_9045
//      guards read iArmTask.  Golden ainarm2.h:97/101.  init to 1 (matching
//      golden InitInArmTask()).  Defined in aHotPlateSubstrate.cpp.
// ============================================================================
extern int iArmTask;                    //golden ainarm2.h:97  : master in-arm SM cursor
extern int iPickFromLoadStageTask;      //golden ainarm2.h:101 : central pick SM cursor
extern bool bPickFromLoader;            //golden ainarm2.h:54  : "Loader 吸取完成" flag (ProcessSCKARTLoadingCount)

// ============================================================================
//  (A2) [W6.2c] OUT-ARM ENGINE cursors owned by not-yet-translated aoutarm2.cpp.
//      DoOutArmPlaceToAuto_9045 binds `int &Task=iPlaceToAutoTask`;
//      DoOutArmAfterPlaceToAuto binds iDoOutArmAfterPlaceToAutoTask; the place SM
//      reads iWhichAuto/iWhichBuff/iWhichMag (golden aoutarm.h:18-20 -- but those
//      are ALREADY in cmydef.h, so only the three task cursors + the Fix3 cursor
//      live here).  init to 1 (golden InitOutArmTask/InitPlaceToAutoTask).
//      Golden aoutarm.h:49/15/51.  Defined in aHotPlateSubstrate.cpp.
// ============================================================================
extern int iPlaceToAutoTask;               //golden aoutarm.h:49  : central out-place SM cursor
extern int iDoOutArmAfterPlaceToAutoTask;  //golden aoutarm.h:15  : after-place SM cursor
extern int iFix3CanFullTask;               //golden aoutarm.h:51  : Fix3 full-tray SM cursor

// ============================================================================
//  (B) [W6.2b] per-variant close-site selector for 1x4, mirroring the
//      iCloseSiteModeFor2x6 / 2x8 pattern above.  GetJStep() reads it.
//      Golden home: ainarm9045_1x4_4.h:11/19 (per-site module, W7).
// ============================================================================
extern int iCloseSiteModeFor1x4;        //golden ainarm9045_1x4_4.h:11
//AI(W6.2c-INARM-batch3) 20260626: e1x4Mode now lives in ainarm9045_1x4_4.h (the
// per-site variant landed ACTIVE this batch and owns the FULL e1x4Mode enum +
// the iCloseSiteModeFor1x4 definition).  Keep this bare e1x4CloseAbAc=3 constant
// ONLY for TUs that include aHotPlateSubstrate.h but NOT ainarm9045_1x4_4.h
// (ainarm9045.cpp:476 + the W6.2 inarm tests).  Guard it out when the variant
// header is present so the enumerator is not redeclared (golden value preserved).
#ifndef ainarm9045_1x4_4H
enum { e1x4CloseAbAc = 3 };             //golden ainarm9045_1x4_4.h:19
#endif

// ============================================================================
//  (D) fBarCode -- golden BarCode.h's TfBarCode.
//      AI(W906-BC-READFILE) 20260923: the class used to be declared HERE, under
//      the invented name `TfBarCode_Shim`.  It now lives in its golden home,
//      BarCode/BarCode.h, under its golden name.  The include stays at this
//      exact spot so the 221 TUs that reach fBarCode through this header are
//      unaffected -- not one of them changed.
// ============================================================================
#include "BarCode/BarCode.h"    // class TfBarCode + extern TfBarCode *fBarCode

// ============================================================================
//  (E) [W6.2b1x1] ainarm2.h engine shims the per-site in-arm VARIANT SMs call.
//      These symbols live in the not-yet-translated ainarm2.cpp (W7) and are
//      SHARED across every in-arm variant; they are therefore defined ONCE here
//      (single TU -> no ODR collision when sibling variant files are translated
//      in the same parallel batch).  Each cites its golden home; every body is
//      offline-safe.  Guards are belt-and-braces against a future direct ainarm2
//      translation also declaring them.  AI(W6.2b-INARM-1x1_1) 20260626.
// ============================================================================
#ifndef HT9045_AINARM2_INARM_SHIMS
#define HT9045_AINARM2_INARM_SHIMS

// -- data (golden ainarm2.h) -------------------------------------------------
extern TMyKitSuck *ptrInSHT;            // golden MyKitSuck.h:379 (Auto Clean Kit use; GetShuttleState_1x1_1 derefs)
extern TMyKitSuck  ptrInSHTBackup;      // golden MyKitSuck.h:380
extern TMyKitSuck  OutArmSuckBackup;    // golden MyKitSuck.h (SetInOutArmParameter_* CopyInitSuck target)
extern int  iInXPToSht[X_PITCH_COUNT];  // golden ainarm2.h:36
extern int  iZPosToSht[MAX_ARM_Row][MAX_ARM_Col];   // golden ainarm2.h:37
extern bool bZFlgToSht[MAX_ARM_Row][MAX_ARM_Col];   // golden ainarm2.h:38
extern TQPF_Timer InArmReleaseDelayToHot;           // golden ainarm2.h:45 (JerryYang 20160127)
extern TQPF_Timer InArmReleaseDelay;                // golden ainarm2.h:46
extern TQPF_Timer MyInArmAtShuttleTimer;            // golden ainarm2.h:47 (Steven 20151201)
extern int  iBackInArmHotCount;         // golden ainarm2.h:76
extern bool InArmXMoveSafe;             // golden ainarm2.h:83
extern bool bPlaceToShuttle2Step;       // golden ainarm2.h:56 (Steven 20160721)
// golden type is HTimer (cpublic.h fwd); offline we use TQPF_Timer (same Off()/
// SetSecAndOn() surface the SM calls) to avoid a cross-include of atester_shims.h.
extern TQPF_Timer hInArmYpitchHomeTimer; // golden ainarm2.h:212 (kevin 20180822 Ypitch)
// -- W6.2c(OUT-ARM) ADD: out-arm Ypitch-home timer sibling (golden aoutarm.h:213,
//    HTimer); offline TQPF_Timer.  Defined once in aHotPlateSubstrate.cpp.
#ifndef HT9045_OUTARM_YPITCH_TIMER_DECLARED
#define HT9045_OUTARM_YPITCH_TIMER_DECLARED
extern TQPF_Timer hOutArmYpitchHomeTimer; // golden aoutarm.h:213 (offline TQPF_Timer)
#endif
// per-site task cursors the variant SMs bind `int &Task=...` to (golden ainarm2.h)
extern int  iInArmPlaceToHotPlateTask;  // golden ainarm2.h (DoPlaceToHotPlate_9045_* cursor)
extern int  iInArmPlaceToShuttleTask;   // golden ainarm2.h (DoInArmPlaceToShuttle_9045_* cursor)
extern int  iInArmTryPickFromHotPlateTask; // golden ainarm2.h (DoInArmTryPickFromHotPlate_* cursor)
extern int  iInArmPickFromHotPlateTask;  // golden ainarm2.h (DoInArmPickFromHotPlate_* cursor; DEFINED in ainarm_SearchPickPlate.cpp)
// HotPlate-check backup-position scalars (golden ainarm2.h; BackupPlacePos/RestorePlacePos)
extern int  iBackupPlate, iBackupPlateC, iBackupPlateR;

// -- functions (golden ainarm2.cpp) ------------------------------------------
extern void CopyInitSuck(TMyKitSuck *Source, TMyKitSuck *Target,
                         int SourceR, int SourceC, int TargetR, int TargetC); // golden MyKitSuck.cpp:1111
extern void SetInArmNeedDestory(bool bPlace, int iShtRow, int iShtCol, int iRow, int iCol); // golden ainarm2.h:230
extern void TransferInShuttleRatio(int iSht, int *iXPos, int *iYPos, int iRow, int iCol);   // golden ainarm2.h:159
extern void ResetInToShtFlag();                                              // golden ainarm2.h:40
// AI(pt-wave) 20260811 PT-W7e: the two declarations the wave owed. Both bodies landed in
// ainarm2.cpp this wave and are UNREACHABLE without these; verified against golden ainarm2.h
// (:104 and :216) rather than transcribed from a report, and the default arguments are
// golden's own -- dropping them would break every existing zero/one-arg call.
extern void ChangeHotPlateData(bool bSwapSht=false);                         // golden ainarm2.h:104
extern bool EnableTraymapCheckFunction(int iCheck=0);                        // golden ainarm2.h:216
// AI(pt-wave) 20260811 PT-W7e-part2: two more declarations the wave owed, and the reason is a
//   general one worth keeping: retiring a stub also removes the DECLARATION that other TUs in the
//   same build were relying on. AutoClean.cpp had 4 live calls and csystem.cpp:6943 one more, all
//   satisfied by the stub sitting in AutoClean.cpp itself; retiring it broke them with
//   "not declared in this scope" rather than a link error. Checked all 50 retired stubs for this
//   role -- 48 already had a header declaration, these 2 did not.
extern void DoInArm_SuckerMap();                                             // golden ainarm2.h:148
extern int  LoadTrayCanUse8Suck();                                           // golden ainarm2.h:200
extern void SetInArm_Unuse_SuckToNullICForHP();                              // golden ainarm2.h:153
extern void AdjustShuttleWhichKitOrder();                                    // golden ainarm2.h:130
extern bool CheckInArmFloating(bool bReset=false);                           // golden OmronLaser/LaserSensorInArm.h:30
extern void SetShuttlefCanMoveL(int iShuttle, bool bCanMoveL, AnsiString sFun, AnsiString sTask=""); // golden ainarm2.h:231
extern void AdjustShtOrderWhenPlaceToSht(int iMode);                         // golden ainarm2.h:132
extern void InArmAddSpeedDisplay();                                          // golden ainarm2.h:206
extern void InArmSubSpeedDisplay();                                          // golden ainarm2.h:207
extern void InitInOCRWaitTask();                                             // golden ainarm2.h:232
extern bool OCRMoveInArm2XYToWait();                                         // golden ainarm2.h:233
extern bool IsHotPlateCheckFinsih();                                         // golden ainarm2.h:166
extern void BackupPlacePos();                                               // golden ainarm2.h:164
extern void RestorePlacePos();                                              // golden ainarm2.h:165
extern void InitInArmPickFromHotPlateTask340();                             // golden ainarm2.cpp:628
extern void InitInArmPickFromHotPlateTask50();                              // golden ainarm2.cpp:633
extern void InitInArmPlaceToShuttleTask();                                  // golden ainarm2.cpp:647
extern void InitInArmPlaceToHotPlateTask();                                 // golden ainarm2.cpp:1539
extern void InitInArmPlaceToHotPlateTask400();                              // golden ainarm2.cpp:1544
extern void InitInArmPlaceToHotPlateTask100();                              // golden ainarm2.cpp:1549
extern void InitArmPickFromLoadStageTask();                                 // golden ainarm2.cpp:1016
extern void InitInArmTryPickFromHotPlateTask();                             // golden ainarm2.cpp:613
extern bool CheckInArmSuckInitial();                                        // golden ainarm2.cpp:1207
extern void SetInArmUseSuckToHasTrySuckIC(int iSht, int iKit);              // golden ainarm2.cpp:261
extern void DisableAutoSiteMapWhenCleanOut();                               // golden ainarm2.cpp:2786
extern void SetMotorSpeed();                                                // golden cinitial.cpp:5022
extern bool DoInArmAutoSiteMapping();                                       // golden (declared csystem.h:200)
extern void SearchPlateToPlace();                                           // golden ainarm2.h:112 (DEFINED in ainarm_SearchPlacePlate.cpp)
extern void InitInArmPickFromHotPlateTask();                                // golden ainarm2.h:115 (DEFINED in ainarm_SearchPickPlate.cpp)
extern void SetInArmUseSuckToHasNullIC(int iSht, int iKit);                 // golden ainarm2.h:151 (DEFINED in ainarm9045.cpp engine)
// in-arm phase flags the DoInArm_9045_1x1_1 master SM toggles (golden ainarm2.h
// :51/:53; DEFINED in acatchtray.cpp this wave).  Declared here so the variant TU sees them.
extern bool bPlaceToHotplate;                                               // golden ainarm2.h:51
extern bool bPlaceShuttle;                                                  // golden ainarm2.h:53
extern int  iInRotateFinish;                                               // golden (DEFINED in acatchtray.cpp)
extern bool bHangTimePause;                                                // golden (also declared atester.h:43; DEFINED in ainarm9045.cpp)
extern const bool ZAxisNotDown;                                             // golden ainarm2.h:43 (sibling of ZAxisDown)
// CheckInArmDestroyICFail is declared in csystem.h:88; redeclared here so the
// substrate TU can DEFINE the offline body (no built csystem.cpp definition).
extern bool CheckInArmDestroyICFail();                                      // golden csystem.cpp (Steven 20111223)

// ----------------------------------------------------------------------------
//  W6.2c batch-1: cross-variant in-arm shim symbols the 1x2_2 / 1x2_2_14 /
//  1x2_4_Hot / 1x3_2_14 / 1x3_4 SMs deref but that had no single shared home.
//  Reusing existing offline definitions where they exist; defining the genuinely
//  missing ones ONCE (aHotPlateSubstrate.cpp).  AI(W6.2c-INARM) 20260626.
// ----------------------------------------------------------------------------
// These offline shim symbols are already DEFINED in the 2x4_16 shim TU
// (ainarm9045_2x4_16_shims.cpp).  Declare the exact ones the W6.2c variants deref
// HERE (NOT by including ainarm9045_2x4_16_shims.h, which carries default-arg
// decls that collide with the engine's own definitions in ainarm9045.cpp).
extern void AdjustShuttlePlaceOrder_AutoSiteMapping();                      // golden ainarm2.h (def ainarm9045_2x4_16_shims.cpp)
extern void InitInArmTryPickFromHotPlateTask100();                          // golden ainarm2.h (def ainarm9045_2x4_16_shims.cpp)
extern void DoJudgeInputShuttleNeedChangeToNullIC();                        // golden (def ainarm9045_2x4_16_shims.cpp)
extern DWORD MyTickCount();                                                 // golden common.h:259 (def ainarm9045_2x4_16_shims.cpp)
// AI(W906-FW-YMSwap) 20260818: TfYieldMonitoring_2x4_16 RETIRED -- the live
// fYieldMonitoring is the REAL TfYieldMonitoring facade (forms/
// fYieldMonitoring.h, FW-3 Wave A), whose DoAutoCloseSite is a DOCUMENTED
// no-op until its own behaviour-change wave -- the ~26 arm-variant call
// sites keep today's exact behaviour. Same macro guard kept so the sibling
// mirror in ainarm9045_2x4_16_shims.h stays double-include-safe.
#ifndef HT9045_YIELDMON_FACADE_DECLARED
#define HT9045_YIELDMON_FACADE_DECLARED
#include "forms/fYieldMonitoring.h"
#endif // HT9045_YIELDMON_FACADE_DECLARED
// Shuttle-soak start ticks (golden atester.h:81/:83; DEFINED in atester.cpp).
extern DWORD dwStartShuttle1Soak;                                           // golden atester.h:81
extern DWORD dwStartShuttle2Soak;                                           // golden atester.h:83
// AutoTeach Z form pointer (golden InOutArmZteach.h: TfInOutArmZteach* Zteach).
// Only Zteach->fShow is read (false offline).  Minimal offline facade; single
// canonical decl + def (aHotPlateSubstrate.cpp).  Guarded so the 1x2_2 local
// stand-in (now removed) cannot double-declare.
#ifndef HT9045_Zteach_DECLARED
#define HT9045_Zteach_DECLARED
struct TfInOutArmZteach_Facade { bool fShow; TfInOutArmZteach_Facade():fShow(false){} };
extern TfInOutArmZteach_Facade *Zteach;                                     // golden InOutArmZteach.h:Zteach
#endif
// ainarm2 helpers with NO live golden home (golden ainarm2.cpp:2812 / :533).
// Offline-safe: the Auto-Site-Mapping side-effect is skipped; the "clear-all-hot-IC
// then pick-load-IC" gate reports not-satisfied (conservative "nothing to do").
extern void DoCheckAutoSiteMappingPosition();                               // golden ainarm2.h:198 / ainarm2.cpp:2812
extern bool CheckClearAllHotICThenPickLoadIC();                             // golden ainarm2.h:168 / ainarm2.cpp:533

#endif // HT9045_AINARM2_INARM_SHIMS

#endif // aHotPlateSubstrateH
