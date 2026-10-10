// =============================================================================
//  forms/fSCKART.h  --  non-VCL stand-in for golden's fSCKART form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Content moved VERBATIM apart
//  from the F0-e virtualisation; TfSortCTPanel / TfLotInfoEdit now come from
//  forms/FormWidgets.h.  Facade-wide contract: see forms/fMain.h.
//
//  RECONCILIATION DEBT THIS FILE IS PART OF -- read before extending it.
//  AI(W906-W7-F2fix) 20260729: COUNT CORRECTED FROM FOUR TO FIVE.  This banner
//  used to say "FOUR places ... Do NOT invent a fifth" while listing
//  csystem.cpp's TWO independent seam structs as a single bullet; plan SS3-C4
//  also says "four" but its four OMIT this file's TfSCKART.  Neither four is the
//  union.  Re-derived this pass by grepping every declaration of the overlapping
//  golden-TfSCKART field names across the whole ported tree: golden has exactly
//  ONE fSCKART object; the ported tree spreads its state over FIVE declaration
//  sites, and the "fifth" the old banner forbade inventing already existed when
//  it was written.
//    1. this TfSCKART (the real global `fSCKART`),
//    2. Automation/SCK_ART.h's `SckArtState`,
//    3. Automation/SCK_ART_Remainder.h's `SckArtRemainderState`
//         -- since W7-F2 this one `: public SckArtState`, so it is the ONLY site
//            that no longer duplicates the 9 overlapping fields,
//    4. csystem.cpp's `W7C1_TfSCKARTSeam`  (shadow fields + its own embedded
//         `SckArtState core`),
//    5. csystem.cpp's `W7C2_TfSCKARTSeam`  (ditto).
//  Because #4 and #5 each EMBED a SckArtState, the number of live copies of
//  those 9 fields at runtime is higher still than five.
//  Do NOT invent a SIXTH.  Plan SS6-F2 also warns these may already have
//  diverged BEHAVIOURALLY, so the merge is analysis work, not mechanical
//  aliasing -- and any divergence found must be reported, not quietly
//  reconciled.  Two such divergences ARE now on record and pinned: see
//  Automation/SCK_ART_Remainder.h's "WHY A SEPARATE SckArtRemainderState" block
//  for the full enumeration, and tests/test_w7_f2_sckart_state.cpp for the pins.
//
//  AI(W906-W7-L1-Wave0) 20260801: BANNER AMENDED IN THE SAME WRITE THAT WIDENED
//  THE OVERLAP -- recorded here rather than left for someone to re-discover.
//  This pass adds 7 golden TfSCKART fields + 1 golden method to the class below
//  for the asendic_* tray SM family (plus 2 PORT-ONLY observability seams, which
//  are not golden state and are excluded from every count here).
//  TWO of the 7 new fields are ALSO SckArtState fields,
//  so the TfSCKART-vs-SckArtState field intersection grows.  Measured this pass
//  by listing both field sets rather than quoting an earlier figure:
//    * BEFORE: TfSCKART carried {iInputJamCnt, iFTRTCount, iInputCount,
//      iCurrent93KARTStep, sLotID, sProcessCode}; SckArtState (Automation/
//      SCK_ART.h) carries {sLOTSTATUS, iCurrentStatus, iTesterType, iInputCount,
//      iFTRTCount, iManualRejectCnt, iNeedRT, dCurrYield, iCurrent93KARTStep,
//      bLdCntExdInputCnt, bBackUpInArmMode, bBackUpOutArmMode}.  Intersection =
//      3 fields (iFTRTCount, iInputCount, iCurrent93KARTStep).
//    * AFTER this pass: + iNeedRT and + iTesterType.  Intersection = 5 fields.
//      The remaining 6 additions (iBundleInCnt, iBundleOutCnt, sBundleList,
//      bFirstFullSkip, iLOTSTATUS_W, and the SetLotStatus method) have no
//      SckArtState counterpart, so they widen nothing.
//  AND ONE NEW DELIBERATE VALUE DIVERGENCE, STATED LOUDLY: this class's
//  iTesterType ctor reproduces golden's real two-part initialisation
//  (SCK_ART.cpp:42 sets 0, then :102-114 sets 1 for every CUSTOMER_CODE that is
//  not CC_SCK), whereas SckArtState's ctor deliberately keeps the bare 0 and is
//  PINNED there by tests/test_w7_f2_sckart_state.cpp with its own disclosed-
//  divergence note (Automation/SCK_ART.cpp:40-50).  The two therefore now differ
//  ON PURPOSE.  Do NOT "reconcile" them by changing either side without reading
//  both notes: SckArtState's 0 is a recorded, test-pinned divergence awaiting a
//  deliberate fix, and this class's value is what golden actually constructs.
//  Do NOT invent a SIXTH declaration site; extend THIS class instead.
// =============================================================================
#ifndef FORMS_FSCKART_H
#define FORMS_FSCKART_H

#include "forms/FormWidgets.h"

// ===========================================================================
//  TfSCKART -- non-VCL stub (golden Automation/SCK_ART.h).  iInputJamCnt is
//  only incremented (inside `if(CosFunction.bUseSCKART)`, default false).
// ===========================================================================
class TfSCKART
{
public:
    // -- W906-P10 ADD (20260921): SetRunStartMode 會用到的三個 ----------------
    //    量過 golden 本體實際怎麼用（不是照抄宣告）：
    //      sInfo_Stage      -> .Pos("QC")    （字串，判斷是否 QC 階段）
    //      iLOTSTATUS_R     -> SetLotStatus(iLOTSTATUS_R) 的引數
    //      DoAutoSocketOff  -> DoAutoSocketOff(true)
    //    ⚠ `DoAutoSocketOff` 轉呼既有的自由函式 `SckArt_DoAutoSocketOff`
    //      （若之後 SCK ART 子系統翻進來，這裡換成真本體）。
    AnsiString  sInfo_Stage;                 // [DATA]   golden SCK_ART.h
    int         iLOTSTATUS_R;                // [DATA]   golden SCK_ART.h   //AI(W906-W132) 20261007 (St02-E): golden SCK_ART.h:256; the ctor sets 4 now (golden SCK_ART.cpp:47), before it was never set
    virtual void DoAutoSocketOff(bool) {}    // [METHOD] golden -- offline no-op
    int iInputJamCnt;                              // [DATA] golden SCK_ART.h:288
    // -- W6.3 ADD: members the TRAY-ARM ENGINE (acatchtray.cpp) derefs -----------
    int iFTRTCount;                                // [DATA] golden SCK_ART.h (FT/RT count; CC_TERAPOWER ART)
    int iInputCount;                               // [DATA] golden SCK_ART.h (ART input count)
    virtual int  CheckLoadingCount();              // [METHOD] golden SCK_ART.h -- offline: 0 (no ART loading mismatch; golden compares ==0/==1/==2)
    // -- W6.5 ADD: member the SHUTTLE ENGINE (acarry.cpp) derefs ----------------
    virtual void AddOutputJamCnt(int row, int col, int ret, int iBinOnCarryKit=0);  // [METHOD] golden Automation/SCK_ART.h:302 -- offline: no-op (inside if(bUseSCKART), default false)
    // -- W5-Automation INTEGRATE ADD: members Automation/AMR.cpp + HANA_ART.cpp derefs --
    int  iCurrent93KARTStep;                       // [DATA] golden SCK_ART.h:263 -- needed by both AMR.cpp and HANA_ART.cpp
    AnsiString sLotID;                              // [DATA] golden SCK_ART.h:237
    AnsiString sProcessCode;                        // [DATA] golden SCK_ART.h:238
    virtual void DoARTLotStart(AnsiString _sLotID, AnsiString _sProcessCode, int _iLotCount);  // [METHOD] golden SCK_ART.h:347 -- offline no-op
    virtual void AccessFile(bool bRead, int iAccess=-1);    // [METHOD] golden SCK_ART.h:271 -- offline no-op
    TfSortCTPanel *palLotNumber;                    // [DATA] golden SCK_ART.h:71  (TPanel* lot-number caption)
    TfSortCTPanel *palTestCnt;                      // [DATA] golden SCK_ART.h:70  (TPanel* FT/RT test-count caption)
    TfSortCTPanel *palRTTryCnt;                     // [DATA] golden SCK_ART.h:75  (TPanel* RT-try-count caption)
    TfSortCTPanel *pnlProcessCode;                  // [DATA] golden SCK_ART.h:181 (TPanel* process-code caption)
    TfLotInfoEdit *edlRTTryCnt;                     // [DATA] golden SCK_ART.h:78  (TEdit* RT-try-count edit)
    // AI(W906-W7-L1-Wave0) 20260801: W7-L1 Wave-0 ADD -- 7 golden data members +
    // 1 golden method the asendic_* tray SM family dereferences, plus 2 port-only
    // observability seams.  Golden lines re-read from the
    // cp950-decoded golden Automation/SCK_ART.h this pass.  See the amended
    // RECONCILIATION DEBT banner at the top of this file for the SckArtState
    // overlap this widens (3 fields -> 5) and for the deliberate iTesterType
    // value divergence it introduces.
    int        iBundleInCnt;                        // [DATA] golden SCK_ART.h:343 -- read by asendic_Loader.cpp:835/:1384/:1386/:1453
    int        iBundleOutCnt;                       // [DATA] golden SCK_ART.h:344 -- read at 7 asendic_Auto.cpp sites
                                    //   (:597/:1138/:1140/:1141/:1686/:1879/:1898); offline default 0
    AnsiString sBundleList;                         // [DATA] golden SCK_ART.h:342 -- read into a TStringList CommaText
                                    //   (asendic_Loader.cpp:1409/:1582); offline default ""
    bool       bFirstFullSkip;                      // [DATA] golden SCK_ART.h:351 (RogerYang 20251112) -- write-only from
                                    //   asendic_Loader.cpp:2645; offline default false
    int        iNeedRT;                             // [DATA] golden SCK_ART.h:285 -- read by asendic_Loader.cpp:2748.
                                    //   ALSO an SckArtState field -- see the banner amendment above; the two are
                                    //   separate storage and this pass does not merge them.
    // iTesterType: THE CTOR VALUE IS LOAD-BEARING AND INVERTS A BRANCH.  golden
    // SCK_ART.cpp:42 sets 0, then :102-114 runs `if(CUSTOMER_CODE==CC_SCK){...}
    // else { iTesterType=1; }`, so on every non-SCK machine golden constructs 1 and
    // `if(fSCKART->iTesterType==0)` at golden asendic_Loader_RT.cpp:808 is FALSE.
    // A bare 0 default would invert that branch.  forms/fSCKART.cpp reproduces the
    // conditional; see its own note for the static-initialisation caveat.
    int        iTesterType;                         // [DATA] golden SCK_ART.h:249 (0: Flex, 1: 93K)
    int        iLOTSTATUS_W;                        // [DATA] golden SCK_ART.h:253; golden ctor SCK_ART.cpp:44 sets 1.
                                    //   Passed as SetLotStatus's argument at golden asendic_Loader_RT.cpp:809.
    // SetLotStatus -- golden SCK_ART.h:269, body SCK_ART.cpp:639-667.  Its offline
    // body is NOT a bare no-op: facade contract rule 4 forbids an unobservable
    // sink, so it records into the two W906_ seams below.
    // AI(W906-W132) 20261007 (St02-E): iCurrentStatus + iLOTSTATUS_A added (golden SCK_ART.h:251 / :258) for the ART
    // LOTRETESTCLEARED arms (TesterComm/Handler/HandlerGpibMsg.cpp G2 / G4 / G6, golden 0618 main.cpp:15461-15525).
    // iCurrentStatus is ALSO an SckArtState field: intersection 8 -> 9, separate storage, NOT merged (separate card).
    // AI(W906-W214) 20261010 (St02-E): W-214 SCKART-UNIFY -- sLOTSTATUS lives HERE: TfSCKART is golden's ONE fSCKART and the canonical lot state;
    // SetLotStatus runs golden SCK_ART.cpp:641-663 via W906_SckArtLotStatusName_St02 (end of forms/fSCKART.cpp; cbLotStatus->ItemIndex :664 has no widget), and csystem.cpp's W7C1 / W7C2 seams forward their SetLotStatus here and read this object's status / constants at every call site.
    AnsiString sLOTSTATUS;                          // [DATA] golden SCK_ART.h:236; golden ctor SCK_ART.cpp:51 sets "NONE" -- AI(W906-W214) 20261010 (St02-E)
    int        iCurrentStatus;                      // [DATA] golden SCK_ART.h:251 -- VCL zero-init 0; written by SetLotStatus (golden SCK_ART.cpp:665)
    int        iLOTSTATUS_A;  int iLOTSTATUS_T; int iLOTSTATUS_L; int iLOTSTATUS_F;   // [DATA] golden SCK_ART.h:258; golden ctor SCK_ART.cpp:49 sets 6  [_T / _L / _F: golden SCK_ART.h:254 / :255 / :257; ctor :45 / :46 / :48 sets 2 / 3 / 5 -- AI(W906-W214) 20261010 (St02-E)]
    virtual void SetLotStatus(int iStatus);  virtual AnsiString GetLotStatus() { return sLOTSTATUS; }   // [METHOD] golden SCK_ART.h:269 (body SCK_ART.cpp:639-667)  [GetLotStatus: golden SCK_ART.h:270, body SCK_ART.cpp:669-672 -- AI(W906-W214) 20261010 (St02-E)]
    int        W906_SetLotStatus_LastArg;           // [PORT-ONLY SEAM] last iStatus passed; -1 when never called
                                    //   (same settable/observable-seam idiom as forms/fMain.h's W906_* members)
    int        W906_SetLotStatus_Count;             // [PORT-ONLY SEAM] call count -- distinguishes "never called"
                                    //   from "called with 0"
    // AI(W906-PT-W3-integrate) 20260808: 9 golden members + 1 golden field the
    // PT-W3 unit Automation/uRENESAS_Server.cpp dereferences (its own
    // "FACADE ADDITIONS NEEDED" banner, uRENESAS_Server.cpp:63-109, is the
    // measurement; every golden line below re-read from the cp950-decoded
    // golden Automation/SCK_ART.{h,cpp} this pass).  This extends declaration
    // site #1 per the RECONCILIATION DEBT banner above -- NOT a sixth site.
    // Overlap bookkeeping: iLotCount / iManualStart / iCurrentFlexARTStep /
    // bLdCntExdInputCnt are ALSO SckArtState or SckArtRemainderState fields,
    // so the TfSCKART-vs-SckArtState intersection grows 5 -> 8 (adding
    // iCurrentFlexARTStep, bLdCntExdInputCnt to the SckArtState overlap;
    // iLotCount/iManualStart overlap SckArtRemainderState).  Separate storage,
    // deliberately NOT merged -- same posture as iNeedRT above.
    int        iLotCount;                           // [DATA] golden SCK_ART.h:246 -- lot qty; golden relies on VCL
                                    //   zero-init (no ctor assignment); read/written by uRENESAS_Server case 20000/30000
    int        iManualStart;                        // [DATA] golden SCK_ART.h:349 (RogerYang 20250918 FT-CT) -- VCL zero-init
    AnsiString sLotStartTime;                       // [DATA] golden SCK_ART.h:241; golden ctor SCK_ART.cpp:41 sets ""
    int        iCurrentFlexARTStep;                 // [DATA] golden SCK_ART.h:264 -- VCL zero-init
    int        iWaitGPIBLotR;                       // [DATA] golden SCK_ART.h:259; golden ctor SCK_ART.cpp:53 sets 0
    int        iOutputJamCnt;                       // [DATA] golden SCK_ART.h:289; golden ctor SCK_ART.cpp:55 sets 0
                                    //   (pairs with the already-real iInputJamCnt above)
    bool       bLdCntExdInputCnt;                   // [DATA] golden SCK_ART.h:350 (RogerYang 20250918 FT-CT) -- VCL zero-init
    int        iLOTSTATUS_NONE;                     // [DATA] golden SCK_ART.h:252; golden ctor SCK_ART.cpp:43 sets 0.
                                    //   Passed to SetLotStatus by ClearLotInfo (golden SCK_ART.cpp:853)
    // ClearLotInfo -- golden SCK_ART.h:274, body SCK_ART.cpp:837-923.  Offline
    // body is a FAITHFUL SUBSET: it performs every golden assignment whose
    // target exists on this facade or on the real LastSet/LotSummary globals,
    // and records each skipped golden statement in the .cpp at the exact spot
    // it would run (facade contract rule 4: no unobservable divergence).
    virtual void ClearLotInfo();                    // [METHOD] golden SCK_ART.h:274 (body SCK_ART.cpp:837-923)
    // CheckNeedRT -- golden SCK_ART.h:291, body SCK_ART.cpp:1072-1230: the full
    // RT-decision engine (reads dCurrYield / iManualRejectCnt / fMain->hanaART /
    // ArmSpeed backups -- none of which live on this facade).  Offline: no-op
    // that leaves iNeedRT UNCHANGED, stated loudly here because golden always
    // rewrites iNeedRT to 0/1/2.  The engine belongs to the SCK_ART.cpp
    // completion wave (PT-W5, campaign plan section 3: 3,876 missing lines) --
    // porting it piecemeal onto the facade would fork it.
    virtual void CheckNeedRT();                     // [METHOD] golden SCK_ART.h:291 (body SCK_ART.cpp:1072-1230)
    TfSCKART();
    virtual ~TfSCKART() {}
};
extern TfSCKART *fSCKART;

#endif // FORMS_FSCKART_H
