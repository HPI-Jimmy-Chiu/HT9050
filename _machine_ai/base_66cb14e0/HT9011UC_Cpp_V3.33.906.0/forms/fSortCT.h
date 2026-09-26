// =============================================================================
//  forms/fSortCT.h  --  non-VCL stand-in for golden's fSortCT form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Content moved VERBATIM apart
//  from the F0-e virtual destructor; TfSortCTPanel is now a typedef onto
//  vclcompat::TPanel (see forms/FormWidgets.h).  Facade-wide contract: see
//  forms/fMain.h.
// =============================================================================
#ifndef FORMS_FSORTCT_H
#define FORMS_FSORTCT_H

#include "forms/FormWidgets.h"

// ===========================================================================
//  TfSortCT -- non-VCL stub (golden cSortCT.h).  pnlHP1/pnlHP2 are TPanel* in
//  the golden; the leaves only assign ->Caption an AnsiString (HowManyIC()).
//  `fSortCT->pnlHP1->Caption=...` compiles against the unified TPanel stand-in.
//  Used inside `if(CosFunction.bShowHPICCount)` (default false).
// ===========================================================================
class TfSortCT
{
public:
    TfSortCTPanel *pnlHP1;
    TfSortCTPanel *pnlHP2;
    // -- W6.3 ADD: tray panels the TRAY-ARM ENGINE (acatchtray.cpp) derefs -------
    //    golden cSortCT.h TPanel* members; the catchtray engine only assigns
    //    ->Caption (AnsiString; HowManyIC() returns int -> AnsiString int-ctor).
    TfSortCTPanel *pnlLoad;                       // [DATA] golden cSortCT.h:59  (loader IC-count panel)
    TfSortCTPanel *pnlLoadCID;                    // [DATA] golden cSortCT.h:265 (loader cover-ID panel)
    TfSortCTPanel *pnlCoverTrayD;                 // [DATA] golden cSortCT.h:280 (color cover-tray-ID panel)
    TfSortCTPanel *pnlTrayCnt[6];                 // [DATA] golden cSortCT.h:337 (per-Auto tray-count panels)
    // AI(W906-W7-L1-Wave0) 20260801: golden cSortCT.h:338 `TPanel
    // *pnlTrayID[MAX_AUTO_TRAY];` -- the per-Auto tray-ID caption panels.
    // asendic_Auto.cpp:1156 reads `fSortCT->pnlTrayID[Pos]->Caption` (read-only in
    // this family), so the ctor must allocate all 6.
    // DIMENSION: golden's MAX_AUTO_TRAY is 6 (golden MachineType.h:396, read this
    // pass).  Written as the literal 6 rather than the macro because forms/
    // headers include only forms/FormWidgets.h -> vclcompat/{vcl_compat,Controls,
    // StringGrid}.h and MAX_AUTO_TRAY is not visible there -- the same convention
    // the sibling pnlTrayCnt[6] one line up already follows.
    TfSortCTPanel *pnlTrayID[6];                  // [DATA] golden cSortCT.h:338 (TPanel*[MAX_AUTO_TRAY==6])
    // -- AI(W906-PT-W3-integrate) 20260808 ADD: the 2 methods the PT-W3 unit
    //    Automation/uRENESAS_Server.cpp calls exactly as golden does (its own
    //    "FACADE ADDITIONS NEEDED" banner, uRENESAS_Server.cpp:106-108, is the
    //    measurement).  golden cSortCT.cpp is UNPORTED as a whole -- there is no
    //    cSortCT.cpp in this tree -- so both bodies belong to that future wave and
    //    are offline no-ops here (forms/fSortCT.cpp).
    //
    //    BEHAVIOUR DELTA, STATED BECAUSE IT IS NOT NEUTRAL (unlike most facade
    //    sinks in this family, these two golden bodies are NOT UI-only):
    //      * ShowLoadingIC (golden cSortCT.cpp:210-279) resets LastSet.SendCT[2]
    //        (and SendCT_ART[2]) on Auto-Site-Map runs, and on the
    //        TestIF.bContinuousLoader / bContinuousLoader_RT arms either raises
    //        WAR07324 or fires ProcessPiggyBackFunction() with
    //        iWhoTriggerPiggyBack=pbtContinualLoader.  Offline: the piggy-back
    //        trigger and that alarm do not happen.
    //      * ShowSortIC (golden cSortCT.cpp:345+) aggregates LastSet.BinCT[0][]
    //        into RunInfo.iUnloadCount / iUnloadCount_ART, and sets iSECSGEMPass /
    //        iSECSGEMFail / iATRPassCount / iATRFailCount / iATRTotalCount.
    //        Offline: those counters keep whatever value they already held, so a
    //        test that expects unload counts to move after a sort must drive them
    //        directly rather than via this call.
    //    NOTE for whoever lands that wave: csystem.cpp:2820-2821 gates these same
    //    two golden calls behind its own TU-local W7C2_FSORTCT_SHOWLOADING() /
    //    W7C2_FSORTCT_SHOWSORT() macros, whose stated premise ("these specific
    //    members are absent") stops being true with this ADD.  Behaviour there is
    //    unchanged either way today (both routes are no-ops), so nothing is
    //    silently wrong -- but those two macros are now retirable and should go
    //    when the real bodies land, or they will hide them.
    virtual void ShowLoadingIC();                 // [METHOD] golden cSortCT.h:326 (body cSortCT.cpp:210-279)
    virtual void ShowSortIC();                    // [METHOD] golden cSortCT.h:327 (body cSortCT.cpp:345+)
    TfSortCT();
    virtual ~TfSortCT() {}
};
extern TfSortCT *fSortCT;

#endif // FORMS_FSORTCT_H
