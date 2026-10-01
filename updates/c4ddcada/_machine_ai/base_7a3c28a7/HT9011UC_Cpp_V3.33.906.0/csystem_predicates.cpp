// =============================================================================
//  csystem_predicates.cpp  --  the W6 cross-arm IC-presence PREDICATE API
//
//  Mirrors (partial) of: HT9011UC_Code_V3.33.906.0_20260618/csystem.cpp
//                        (the ~15 HasIC-family bodies, csystem.cpp:964-1047 +
//                         HasICUnderHotPlate at :12664).
//  Translation wave: W6.0 (freeze + implement the SM-to-SM decoupling contract)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  WHY THIS FILE EXISTS
//  --------------------
//  csystem.h is frozen as the W6 INTERFACE.  Its load-bearing part is the
//  cross-arm IC-presence predicate family.  State machines never cross-call each
//  other's Do* functions; they only consult these predicates.  This .cpp gives
//  the frozen interface a linkable definition.
//
//  NOTE: the W6.1 canary (asendic_Empty) exercises ZERO of these predicates
//  (verified: 0 grep hits for any HasIC name in asendic_Empty.cpp).  They are
//  implemented now ONLY to lock the contract for later sub-waves and to keep the
//  frozen interface linkable.
//
//  FAITHFUL CALL-GRAPH, STUBBED LEAVES
//  -----------------------------------
//  The golden leaf predicates read the vacuum-suck / carry-kit grid objects
//  (FLCarryKit / BLCarryKit / FRCarryKit / BRCarryKit / FTestSuck / BTestSuck /
//  TestSocket / OutSht3Kit -- all TMyKitSuck/TMySucker).  That entire grid was
//  DEFERRED by the W4-IO HAL note ("Entire TMyKitSuck data grid ... deferred to
//  W6") and is NOT yet in the target.  So:
//    * the COMPOSITE predicates (InputShuttleHasIC / OutputShuttleHasIC /
//      ShuttleHasIC / TestHeadHasIC / IndexHasIC) delegate to the leaves
//      EXACTLY as the golden does -- the contract logic shape is preserved.
//    * the LEAF predicates' grid access (`Kit.UseSiteHasIC()`) is gated
//      `#if 0 // TODO(W6.x: TMyKitSuck grid)` with a conservative `return false`
//      active fallback.  When the KitSuck grid lands, flip the gate.
//  The IniConfig / TestIF_File arm-2 guards (which DO exist in the target) are
//  kept live so the Rear-variant branch structure stays faithful.
// =============================================================================
#include "csystem.h"
#include "cmydef.h"     // IniConfig, MOT[]-adjacent globals, consts
#include "cprod.h"      // Prod, TestIF_File
//AI(ht9045-v906) 20260629: W7 substrate staging -- with HT9045_KITSUCK_GRID_AVAILABLE flipped ON,
// the leaf predicates now reference the TMyKitSuck carry-kit globals. Pull in their extern decls
// (FLCarryKit/BLCarryKit/FRCarryKit/BRCarryKit/FTestSuck/BTestSuck/TestSocket from aHotPlateSubstrate.h,
// OutSht3Kit from acarry_shims.h). No predicate body changed; these were only unneeded while gated OFF.
#include "aHotPlateSubstrate.h"
#include "acarry_shims.h"
// AI(W64b-Integrate) 20260706: LastSet.iRealDummy (CheckIndexAllSuckICFallDown
// golden guard) needs canary_support.h; not previously included here.
#include "canary_support.h"

//AI(ht9045-v906) 20260629: W7 substrate staging -- TMyKitSuck grid is now translated and
// UseSiteHasIC()/UseSiteNoIC() carry the faithful golden Item-scan (golden MyKitSuck.cpp:288/297);
// HasRealIC()/CountRealIC()/HasDefineIC()/UseSiteFullIC() were already faithful Item-scans.
// Flip the gate ON so the leaves consult the real (seedable) Item grid. The #else conservative-false
// branches are now dead. Empty/zero-init grids are all-NULL_IC, so this is behavior-identical until seeded.
#define HT9045_KITSUCK_GRID_AVAILABLE 1

//==============================================================================
//  Input shuttle (front carry-kit FLCarryKit)
//==============================================================================
#if 0   // PT-W5c RETIRED (InputShuttleFrontHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InputShuttleFrontHasIC()
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (FLCarryKit.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): FLCarryKit.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (InputShuttleRearHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InputShuttleRearHasIC()
{
    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 supply light for CMOS
       TestIF_File.bForEgisTecTest==true     ||                                 //Steven 20140922 : Arm2 used for fingerprint test
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                         //kevin 20150127 Arm1 press, arm2 test
        TestIF_File.bArm1PickPlaceArm2Test==true))                              //Ifor 20200811 Fix: needs both conditions
    {
        return false;
    }

#if HT9045_KITSUCK_GRID_AVAILABLE
    return (BLCarryKit.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): BLCarryKit.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (InputShuttleHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InputShuttleHasIC()
{
    return (InputShuttleFrontHasIC() || InputShuttleRearHasIC());
}
#endif
//==============================================================================
//  Output shuttle (front/rear carry-kit FRCarryKit / BRCarryKit)
//==============================================================================
#if 0   // PT-W5c RETIRED (OutputShuttleFrontHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool OutputShuttleFrontHasIC()
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (FRCarryKit.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): FRCarryKit.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (OutputShuttleRearHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool OutputShuttleRearHasIC()
{
    if(IniConfig.bIndexArm2SupplyLight==true ||                                 //jou 2012-10-19 Index Arm 2 supply light for CMOS
       TestIF_File.bForEgisTecTest==true     ||                                 //Steven 20140922 : Arm2 used for fingerprint test
       (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                         //kevin 20150127 Arm1 press, arm2 test
        TestIF_File.bArm1PickPlaceArm2Test==true))                              //Ifor 20200811 Fix: needs both conditions
    {
        return false;
    }
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (BRCarryKit.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): BRCarryKit.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (OutputShuttleHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool OutputShuttleHasIC()
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (FRCarryKit.UseSiteHasIC() || BRCarryKit.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): FRCarryKit/BRCarryKit.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (ShuttleHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool ShuttleHasIC()
{
    return (InputShuttleHasIC() || OutputShuttleHasIC());
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (SortShuttleHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool SortShuttleHasIC()                                                         //RogerYang 20250506 Add for 9046AU
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (OutSht3Kit.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): OutSht3Kit.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
//  Test head / socket (FTestSuck / BTestSuck / TestSocket)
//==============================================================================
#if 0   // PT-W5c RETIRED (FrontTestHeadHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool FrontTestHeadHasIC()
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (FTestSuck.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): FTestSuck.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (RearTestHeadHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool RearTestHeadHasIC()
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (BTestSuck.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): BTestSuck.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (TestHeadHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool TestHeadHasIC()
{
    return (FrontTestHeadHasIC() || RearTestHeadHasIC());
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (TestSocketHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool TestSocketHasIC()
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (TestSocket.UseSiteHasIC());
#else
    return false;   // TODO(W6.x): TestSocket.UseSiteHasIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (IndexHasIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool IndexHasIC()
{
    return (TestHeadHasIC() || TestSocketHasIC());
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (IndexHasRealIC)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool IndexHasRealIC()                                                           //Steven 20240217 : Fixed for one cycle / clean out hang up
{
#if HT9045_KITSUCK_GRID_AVAILABLE
    return (FTestSuck.HasRealIC() || BTestSuck.HasRealIC() || TestSocket.HasRealIC());
#else
    return false;   // TODO(W6.x): FTestSuck/BTestSuck/TestSocket.HasRealIC() (TMyKitSuck grid)
#endif
}
#endif
//==============================================================================
//  HasICUnderHotPlate (golden csystem.cpp:12664)
//  Golden walks the HotPlate occupancy via MOT[MHotPlate*].HasIC()/Prod hotplate
//  state.  The hotplate MOT-index map + Prod hotplate-occupancy accessor is not
//  wired this wave; conservative false until W6.x.
//==============================================================================
#if 0   // PT-W5c RETIRED (HasICUnderHotPlate)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool HasICUnderHotPlate()                                                       //Steven 20110826
{
    return false;   // TODO(W6.x): MOT[MHotPlate*].HasIC() / Prod hotplate occupancy
}
#endif
//==============================================================================
//  Cross-arm aggregate predicates (declared in csystem.h; golden bodies are the
//  big sHasICUnderMachine / HasAutoICInMachine surface, gated in csystem.cpp).
//  Provide the boolean aggregates the contract names; conservative until W6.x.
//==============================================================================
#if 0   // PT-W5c RETIRED (HasICUnderMachine)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool HasICUnderMachine()
{
    // TODO(W6.x): golden aggregates Loader/HotPlate/Shuttle/Index occupancy.
    return (ShuttleHasIC() || IndexHasIC() || HasICUnderHotPlate());
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (HasAutoICInMachine)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool HasAutoICInMachine()
{
    // TODO(W6.x): golden walks Auto-tray IC occupancy (MOT[]/Prod).
    return false;
}
#endif
//==============================================================================
#if 0   // PT-W5c RETIRED (HasAnyICInMachine)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool HasAnyICInMachine()                                                        //kevin 20150914 : whether any tray still in machine
{
    // TODO(W6.x): golden aggregates every IC-bearing position.
    return (HasICUnderMachine() || HasAutoICInMachine());
}
#endif
//==============================================================================
//  W6.2 additions: frozen csystem predicates/helpers the shared HP geometry
//  leaves (ainarm_SearchPickPlate/PlacePlate) consult.  These are declared in
//  csystem.h; the golden bodies live in csystem.cpp (gated #if 0).  Offline sim
//  bodies follow -- conservative, matching a handler with shuttles not docked
//  and the safe door closed.  AI(W6.2-INARM) 20260626.
//==============================================================================
//  InSHT1InLF / InSHT2InLF (golden csystem.cpp:413/498): the input shuttle is in
//  its left-feed position.  Offline there is no shuttle motion -> not in LF.
#if 0   // PT-W5c RETIRED (InSHT1InLF)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InSHT1InLF() { return false; }
#endif
#if 0   // PT-W5c RETIRED (InSHT2InLF)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InSHT2InLF() { return false; }
#endif
//------------------------------------------------------------------------------
//  InSHT1InRT / InSHT2InRT (golden csystem.cpp:458/542): the shuttle is in its
//  right-transfer position.  The golden body reads MOT[MInShuttle*].Led[iInposLed]
//  (the in-position LED) + the Offset-change-right encoder compare.  Offline
//  there is no shuttle motion docked at the right station -> not in RT.
//  OutSHT1InRT / OutSHT2InRT (golden csystem.cpp:699/701) DELEGATE verbatim to
//  the InSHT*InRT predicates.  Added because the out-arm ENGINE
//  (DoOutArmIonFanGiveWay case 1/5, aoutarm9045.cpp:1325/1361) consults them and
//  they had no offline home.  AI(W6.2c-OUTARM) 20260626.
#if 0   // PT-W5c RETIRED (InSHT1InRT)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InSHT1InRT() { return false; }
#endif
#if 0   // PT-W5c RETIRED (InSHT2InRT)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool InSHT2InRT() { return false; }
#endif
bool OutSHT1InRT() { return InSHT1InRT(); }                                      // golden csystem.cpp:699
bool OutSHT2InRT() { return InSHT2InRT(); }                                      // golden csystem.cpp:701
// AI(W64b-Integrate) 20260706: OutSHT1InLF/OutSHT2InLF (golden csystem.cpp:698,700)
// were declared in csystem.h but had no defined body anywhere in the translated
// tree -- discovered as an undefined-reference link error while integrating
// aTester_Front.cpp/aTester_Rear.cpp (case 1: `if(OutSHT1InLF()==false)`).
// Golden bodies are trivial one-line delegates; translated verbatim (not a gate).
bool OutSHT1InLF() { return InSHT1InLF(); }                                      // golden csystem.cpp:698
bool OutSHT2InLF() { return InSHT2InLF(); }                                      // golden csystem.cpp:700
//------------------------------------------------------------------------------
// AI(W64b-Integrate) 20260706: CheckIndexAllSuckICFallDown (golden csystem.cpp:2417,
// "Steven 20110725 修改檢查方式") -- declared in csystem.h but had no defined body
// anywhere in the translated tree; discovered as an undefined-reference link error
// while integrating aTester_Front.cpp/aTester_Rear.cpp (case 1/200 drop-detect
// calls).  Translated FAITHFULLY (not a gate/stub) over the already-translated
// FTestSuck/BTestSuck substrate + bIndex1Suck/bIndex2Suck (cmydef.h) +
// LastSet.iRealDummy/REALLY.  SOFT_SIMULTE not defined -> the #ifndef body is LIVE,
// exactly as golden.
#if 0   // PT-W5c RETIRED (CheckIndexAllSuckICFallDown)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckIndexAllSuckICFallDown(bool bCheckArm1, bool bCheckArm2)              //Steven 20110725 : 修改檢查方式
{
    bool bFail=false;
    #ifndef SOFT_SIMULTE
    if(LastSet.iRealDummy==REALLY)
    {
        for(int i=0; i<FTestSuck.iShtRow; i++)
        {
            for(int j=0; j<FTestSuck.iShtCol; j++)
            {
                if(bCheckArm1 && bIndex1Suck==false)                            //kevin 20220105 Index 在下壓時建立 pause 保護條件
                {
                    if(FTestSuck.Suck[i][j].Enable       &&
                       FTestSuck.Suck[i][j].SenUsing!="" &&
                       FTestSuck.Item[i][j]!=HAS_NULL_IC &&
                       FTestSuck.Item[i][j]!=NULL_IC)
                    {
                        if(FTestSuck.Suck[i][j].GetStatus()==false)
                        {
                            if(INDEX_SUCKER_TYPE==1)
                                FTestSuck.Suck[i][j].Normal();
                            bFail=true;                                         //Steven 20110707 : 只要有一個True就True
                        }
                    }
                }

                if(bCheckArm2 && bIndex2Suck==false)                            //kevin 20220105 Index 在下壓時建立 pause 保護條件
                {
                    if(BTestSuck.Suck[i][j].Enable       &&
                       BTestSuck.Suck[i][j].SenUsing!="" &&
                       BTestSuck.Item[i][j]!=HAS_NULL_IC &&
                       BTestSuck.Item[i][j]!=NULL_IC)
                    {
                        if(BTestSuck.Suck[i][j].GetStatus()==false)
                        {
                            if(INDEX_SUCKER_TYPE==1)
                                BTestSuck.Suck[i][j].Normal();
                            bFail=true;                                         //Steven 20110707 : 只要有一個True就True
                        }
                    }
                }
            }
        }
    }
    #endif
    return bFail;
}
#endif
//------------------------------------------------------------------------------
//  CheckSafeDoorIsClosed (golden csystem.cpp:2599): offline (no PLC / no door
//  sensor) the door is treated as CLOSED so the #ifndef SOFT_SIMULTE guard the
//  leaves call is non-blocking.
#if 0   // PT-W5c RETIRED (CheckSafeDoorIsClosed)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckSafeDoorIsClosed() { return true; }
#endif
//------------------------------------------------------------------------------
//  DoAutoSiteMappingDropError (golden csystem.cpp:23361): only acts when
//  bRunAutoSiteMapping==true.  Offline ASM is off -> the golden body is a no-op;
//  keep the guard shape faithful (re-reads bRunAutoSiteMapping at the call).
#if 0   // PT-W5f RETIRED (DoAutoSiteMappingDropError)
//AI(ht9045-v906) 20260810: PT-W5f -- RETIRED. csystem.cpp wave 2 landed the real faithful body in its golden home; keeping this stand-in is a multiple-definition error, measured in build_0810_w5f.
void DoAutoSiteMappingDropError()
{
    // TODO(W6.x/W7): full ASM drop-error recovery (InitInArmTask + grid reset)
    // lands with the ainarm core.  Offline ASM is off, so this is a no-op.
}
#endif
//==============================================================================
//  W7-C1 ADD: csystem.h predicates the end-of-lot CLEAN OUT drain finish-check
//  (DoCleanOutFinishCheck, csystem.cpp) consults in its ACTIVE path.  These had
//  NO offline home before W7-C1 (the empty stub referenced neither); the real
//  body does, so translate them FAITHFULLY here over the existing MOT[]/kit
//  substrate.  AI(W7C1-Integrate) 20260629.
//------------------------------------------------------------------------------
//  AllArmZIsSafe -- golden csystem.cpp:343.  Every in/out-arm Z motor must be at
//  a non-negative (safe/up) position.  Offline a fresh kit has iMotRow/iMotCol==0
//  so the loop is empty -> returns true (all Z safe), which is the faithful
//  "arms parked up" posture for an idle/empty handler.  The Sim motor ReadPos()
//  is >=0 once homed, so a homed offline machine is also safe.
#if 0   // PT-W5c RETIRED (AllArmZIsSafe)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool AllArmZIsSafe()                                                            // golden csystem.cpp:343
{
    #ifdef SOFT_SIMULTE
    return true;
    #else
    int iMotNoIn, iMotNoOut;
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMotNoIn =(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            iMotNoOut=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MOutArmZA:OutArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMotNoIn].ReadPos()<0)                                       //Steven 20210713 : Prod.ZInArmSafe[i][j] --> 0
                return false;
            if(MOT[iMotNoOut].ReadPos()<0)                                      //Steven 20210713 : Prod.ZOutArmSafe[i][j] --> 0
                return false;
        }
    }
    return true;
    #endif
}
#endif
//------------------------------------------------------------------------------
//  CheckIndexIsNormal -- golden csystem.cpp:12676.  The two test-head Z motors
//  must be within their safe encoder window and not moving.  Translated VERBATIM
//  (the golden has the well-known MTestZ1.MovFlag double-OR typo at :12697-12698;
//  preserved exactly).  Offline the Sim Gali_ReadEncoderInRandge returns true at
//  the home/safe window and MovFlag is false when idle -> a parked index reads
//  Normal.
#if 0   // PT-W5c RETIRED (CheckIndexIsNormal)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool CheckIndexIsNormal()                                                       // golden csystem.cpp:12676
{
    if(MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe)==false  ||
       MOT[MTestZ2].Gali_ReadEncoderInRandge(Prod.TestZ2_Safe)==false)
    {
        if(MOT[MTestZ1].Gali_ReadEncoderInRandge(0)==false  ||
           MOT[MTestZ2].Gali_ReadEncoderInRandge(0)==false)
        {
            return false;
        }
    }

    if(IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear)                    //marc 2007/10/11 start
    {
        if(MOT[MTestY1].Gali_ReadEncoderInRandge(Prod.TestY1_Middle)==false)
        {
            return false;
        }
    }

    if(MOT[MTestZ1].MovFlag ||
       MOT[MTestZ1].MovFlag ||                                                  //golden double-OR on MTestZ1 (verbatim)
       MOT[MTestY1].MovFlag)
    {
        return false;
    }
    return true;
}
#endif
//------------------------------------------------------------------------------
//  hAutoCleanHangUp -- golden csystem.cpp:157 (TQPF_Timer global, Steven
//  20220702).  Declared extern in csystem.h; the C1 finish-check arms it
//  (SetSecAndOn) on the auto-clean-at-finish branch.  Single ODR definition
//  here (no other TU defines the csystem timer globals yet).
TQPF_Timer hAutoCleanHangUp;                                                    // golden csystem.cpp:157
//------------------------------------------------------------------------------
//  AI(W906-PT-W4-integrate) 20260809: ESD_GENERAL -- golden csystem.cpp:155, i.e.
//  TWO LINES ABOVE hAutoCleanHangUp above, which is why it belongs here: this file
//  is already the tree's home for golden csystem.cpp's file-scope globals, and the
//  port's own csystem.cpp never reached that region.
//  WHY IT IS NEEDED NOW: SECSGEM/uHGemHT9045_EC.cpp landed in wave PT-W4 with 26
//  ACTIVE registrations that take the address of ESD_GENERAL's members.  csystem.h:367
//  declares it `extern` (faithfully -- golden declares it there too), and NO .cpp in
//  this tree defined it, so the unit carried an undefined reference.  That reference
//  was INVISIBLE while EC.cpp.obj sat unextracted in an archive nobody pulled from --
//  found with `nm --undefined-only` against every archive, not by a failing build.
//  SAFE AS A FILE-SCOPE OBJECT: ESD_GENERAL_SET (csystem.h:325-366) is a plain
//  aggregate -- ints, bools and AnsiStrings, no user ctor and no pointer that anything
//  must `new` -- so zero-initialisation at static-init time touches nothing else, and
//  it cannot trip the plan-section-8 NULL-global hazard.  Golden populates it in
//  csystem.cpp:22424+ (ReadESDSetData), which this port has not reached; until then
//  every field reads as its zero, exactly as golden reads before that call.
ESD_GENERAL_SET ESD_GENERAL;                                                    // golden csystem.cpp:155 (kevin 20180907 add ESD Data)
//==============================================================================
