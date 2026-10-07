// =============================================================================
//  BarCode/BarCode_Bottom2DID.h  --  1-CCD Bottom 2DID scan engine
//                            (TfBarCode::InitBottom2DIDScan /
//                             TfBarCode::DoBottom2DIDScan real bodies)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/BarCode/BarCode.cpp
//      TfBarCode::InitBottom2DIDScan(bool bClear2DID)   golden :6961-7005 (45 lines)
//      TfBarCode::DoBottom2DIDScan()                    golden :7135-8208 (1074 lines)
//  Golden decl:  BarCode/BarCode.h:920-ish (InitBottom2DIDScan declared nearby,
//                same header).  aHotPlateSubstrate.h:626-627 already stubs the
//                TfBarCode delegation targets:
//                    void InitBottom2DIDScan();   // golden BarCode.h : init bottom-2DID scan
//                    bool DoBottom2DIDScan();      // golden BarCode.h : 1-CCD bottom 2DID scan
//
//  Translation wave: W5-BarCode (BarCode module, Bottom-2DID 1-CCD unit)
//  Translator: AI(W5-BarCode-Bottom2DID-Translate) 20260711
//
//  PATH NOTE (discrepancy vs. task brief, golden/established-convention wins --
//  see translate report item 8): the task brief asked for a ROOT-level
//  `HT9011UC_Cpp_V3.33.906.0\BarCode_Bottom2DID.{h,cpp}`.  By the time this unit
//  ran, 3 sibling W5-BarCode units had already landed their files under a
//  `BarCode\` subdirectory (BarCode_Helpers.{h,cpp}, BarCode_Shuttle1_Scan.h,
//  BarCode_Shuttle2_Scan.{h,cpp}), matching this project's own file-mirroring
//  convention (KNOWLEDGE.md: translated files mirror the golden relative path
//  -- golden BarCode.cpp lives in a `BarCode\` folder too).  Placed here
//  instead, for consistency with those 3 already-existing sibling files (and
//  to let this unit `#include` their headers by bare filename, same-directory
//  quoted-include resolution).  Flagged for the integrate agent.
//
//  SCOPE
//  -----
//  This unit translates ONLY TfBarCode::InitBottom2DIDScan and
//  TfBarCode::DoBottom2DIDScan (the 1-CCD bottom-mounted 2D-barcode scan state
//  machine).  DoBottom2DID_8CCD_Scan (golden BarCode.cpp:9940, the 8-CCD sibling
//  variant) and the 14 in/out-shuttle 2DID-scan TfBarCode methods are OUT
//  OF SCOPE for this unit (translated by other parallel units in this same W5
//  BarCode wave -- confirmed present alongside this unit: BarCode_Helpers.cpp
//  [TfHIKROBOTcmd/uCCDUnloaderClip/IsSHT2DIDScanFinish], BarCode_Shuttle1_Scan.*
//  [5 of 7 In/Out-Shuttle-1 methods], BarCode_Shuttle2_Scan.* [5 of 7
//  In/Out-Shuttle-2 methods], BarCode_Shuttle1_CCDScan.h / _Shuttle2_CCDScan.h
//  [DoBarcodeCCDInShuttle_1/_2, in progress at the time this unit ran]).
//  BarCode.cpp / BarCode_Sh1.cpp / BarCode_Sh2.cpp / BarcodeXML.cpp themselves
//  are NOT mirrored by this file (deliberately -- multiple parallel units read
//  from those same golden files this wave).
//
//  GATED DEPENDENCIES (golden TfBarCode members/helpers NOT part of this unit's
//  assignment, and not yet available anywhere in the translated tree):
//    * fNote->t2DCode / fNote->bMyServoOffInArm      -- TfNote VCL form, no home
//        anywhere in the migrated tree (every existing fNote-> reference in
//        ainarm9045.cpp/aoutarm9045.cpp/atester_32Site.cpp/cpublic.cpp is itself
//        inside a `#if 0` block) -- gated here the same way, NOT a new global.
//    * fLotInfo->mtBarcodeInShLotInfo / ->sgBarcode  -- FormsFacade::TfLotInfo
//        exists but has no such member yet (cosmetic UI only) -- gated inline,
//        matching the sibling BarCode_Shuttle2_Scan.cpp's
//        Gated_SetLotInfoBarcodeGridCell convention.
//    * ClientSocket_Bottom_* / memoCCDCommLog        -- SendCCDCommand's real
//        body (golden BarCode.cpp:5305) needs a vclcompat TClientSocket shim
//        (not yet built, same as the BarCode-wide instruction) + a VCL TMemo;
//        gated to a logging-only stand-in (see SendCCDCommand in the .cpp).
//    * Tech.iBottom2DIDInX/InY (LastSet.h fields not yet in the acarry_shims.h
//        TECH shim) + the fNote/InArmContinuousMove_9045 motor-move body --
//        MoveInArmXY_ToBottom2DID (golden :7006, NOT one of our 2 assigned
//        methods) is gated wholesale; conservative default: "arrived" (true),
//        matching this project's Sim-motor "instantaneous move" convention.
//
//  SHARED BarCode-class STATE -- reused, NOT redefined (see sibling headers):
//    iBarCode1_1 / iBarCode1_2   -- BarCode_Shuttle1_Scan.h (enum, ==0/==1)
//    iBarCode2_1 / iBarCode2_2   -- BarCode_Shuttle2_Scan.h (extern const int,
//                                   canonical definition in .cpp, ==2/==3)
//    map2DList / map2DIter       -- BarCode_Shuttle2_Scan.h (extern, canonical
//                                   definition in .cpp).  NOTE: Shuttle1_Scan.h
//                                   ALSO declares (not yet defines -- its .cpp
//                                   does not exist yet) the identical externs;
//                                   once that sibling's .cpp lands, the
//                                   integrate agent must verify only ONE TU
//                                   defines these (currently Shuttle2_Scan.cpp).
//    iShtDupilcateFailCnt[2]     -- BarCode_Shuttle2_Scan.h (canonical def in
//                                   .cpp).  Bottom2DID (this unit) and the
//                                   Shuttle1 cluster BOTH use index [0] (a
//                                   genuine golden coupling: bottom-mounted vs.
//                                   shuttle-mounted CCD are mutually-exclusive
//                                   hardware installs, so only one is ever
//                                   actually active at a time) -- see this
//                                   unit's translate report item 6.
//  This unit OWNS the canonical definition (in BarCode_Bottom2DID.cpp) of every
//  OTHER shared symbol below -- confirmed by grep against every sibling file
//  that exists at the time this unit ran (none of them reference these names).
//  If a FUTURE unit (e.g. whoever finishes the still-incomplete remainder of
//  DoBarcodeScanInShuttle_1/2 or DoShuttleFloatCheck_1/2, which golden also
//  touches map2DShuttleList/mapShtIter per BarCode.h) needs any of these, it
//  MUST #include this header and reference these externs, not redefine them.
// =============================================================================
#ifndef BarCode_Bottom2DIDH
#define BarCode_Bottom2DIDH

#include "vclcompat/vcl_compat.h"   // AnsiString / TStringList
#include "BarCode_Shuttle1_Scan.h"  // iBarCode1_1 / iBarCode1_2 (enum, ==0/==1)
#include "BarCode_Shuttle2_Scan.h"  // iBarCode2_1/iBarCode2_2/map2DList/map2DIter/
                                    // iShtDupilcateFailCnt[2] (canonical defs) +
                                    // BAR_CODE_COUNT
#include "myTimer.h"                // TQPF_Timer
#include <map>

// ---------------------------------------------------------------------------
//  The two TfBarCode delegation targets (see aHotPlateSubstrate.h:626-627
//  for the exact stub signatures these must match).
// ---------------------------------------------------------------------------
void BarCode_InitBottom2DIDScan(bool bClear2DID=true);   // golden BarCode.cpp:6961
bool BarCode_DoBottom2DIDScan();                          // golden BarCode.cpp:7135

// ---------------------------------------------------------------------------
//  Bottom2DID-cluster-only timers (golden BarCode.h:872-874) -- used ONLY
//  within DoBottom2DIDScan/DoBottom2DID_8CCD_Scan (0 hits in BarCode_Sh1.cpp/
//  BarCode_Sh2.cpp, confirmed by grep); this unit owns the canonical def.
// ---------------------------------------------------------------------------
extern TQPF_Timer Bottom2DPosDelay;         // golden BarCode.h:872
extern TQPF_Timer Bottom2DExposureDelay;    // golden BarCode.h:873
extern TQPF_Timer Bottom2DDelay;            // golden BarCode.h:874

// ---------------------------------------------------------------------------
//  SYMBOLS THIS UNIT OWNS THE CANONICAL DEFINITION OF (defined once in
//  BarCode_Bottom2DID.cpp; sane defaults mirror golden zero-init / ctor
//  defaults).  Shared ONLY within the Bottom2DID cluster (this unit + the
//  sibling 8-CCD unit's DoBottom2DID_8CCD_Scan, golden BarCode.cpp:9940) unless
//  noted otherwise.
// ---------------------------------------------------------------------------
extern int  iBottom2DID_8CCDTask;   // golden BarCode.h:771 (written =1 by InitBottom2DIDScan only)
extern bool bNeedScanBottom2D[2][4];// golden BarCode.h:918
extern int  iBottomKit;             // golden BarCode.h:922
extern bool bBottom2DNeedMoveInArm; // golden BarCode.h:923

// -- shared across ALL CCD-scan methods in golden (Bottom2DID + shuttle 2DID +
//    8CCD) -- but NOT yet referenced by any sibling unit that exists at the
//    time this unit ran (verified by grep); this unit owns the canonical
//    definition for now.  Flagged in the translate report for the integrate
//    agent to watch for a future collision.
extern bool bCCDBarcodeExposureOK  [BAR_CODE_COUNT];   // golden BarCode.h:742
extern bool bCCDBarcodeGetResultOK [BAR_CODE_COUNT];   // golden BarCode.h:744
extern bool bGetSE9                [BAR_CODE_COUNT];   // golden BarCode.h:748
extern bool bStepError             [BAR_CODE_COUNT];   // golden BarCode.h:856
extern int  iCurrentStep           [BAR_CODE_COUNT];   // golden BarCode.h:855
extern AnsiString s2DIDYield;                          // golden BarCode.h:917
extern std::map<AnsiString, AnsiString> map2DShuttleList[BAR_CODE_COUNT]; // golden BarCode.h:850
extern TStringList *list2DByLot;                        // golden BarCode.h:836 (also
                                                         // `fBarCode->list2DByLot` in golden --
                                                         // TfBarCode has no such member yet;
                                                         // exposed here as a bare global instead.
                                                         // See translate report item 6.)

// AI(W906-ST-W7-H) 20260915: same treatment as list2DByLot above, same reason.
//   golden BarCode.h:839 `TStringList *listError2DID;` is a TfBarCode member;
//   TfBarCode has no such member, so it is exposed here as a bare global,
//   next to its sibling list2DByLot (golden :836) rather than in the consumer TU.
//   Home chosen to match golden's own adjacency -- the future consumer
//   `b2DIDIsInsideToErrorBin` (golden BarCode.cpp:8268) will live in BarCode.cpp,
//   not in WebStart.cpp, so the list must not be file-local to the translator.
//   ⚠ NOTHING READS IT YET: both consumers (b2DIDIsInsideList /
//   b2DIDIsInsideToErrorBin) are unported; their four call sites in atester.cpp
//   are all inside golden-verbatim `#if 0`. Loading it today feeds nobody.
//   That is deliberate and recorded, not an oversight.
extern TStringList *listError2DID;                      // golden BarCode.h:839

#endif // BarCode_Bottom2DIDH
