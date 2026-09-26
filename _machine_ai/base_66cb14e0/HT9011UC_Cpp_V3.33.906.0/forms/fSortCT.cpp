// =============================================================================
//  forms/fSortCT.cpp  --  definitions for the fSortCT facade
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.cpp by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Body moved VERBATIM.
// =============================================================================
#include "forms/fSortCT.h"

// --- W6.2: TfSortCT --------------------------------------------------------
TfSortCT::TfSortCT()
{
    pnlHP1 = new TfSortCTPanel();
    pnlHP2 = new TfSortCTPanel();
    // -- W6.3 ADD --
    pnlLoad       = new TfSortCTPanel();
    pnlLoadCID    = new TfSortCTPanel();
    pnlCoverTrayD = new TfSortCTPanel();
    for(int i=0;i<6;i++) pnlTrayCnt[i] = new TfSortCTPanel();
    // AI(W906-W7-L1-Wave0) 20260801: golden cSortCT.h:338 pnlTrayID[MAX_AUTO_TRAY]
    // (MAX_AUTO_TRAY==6, golden MachineType.h:396) -- read by asendic_Auto.cpp:1156.
    for(int iW0=0;iW0<6;iW0++) pnlTrayID[iW0] = new TfSortCTPanel();
}
// AI(W906-PT-W3-integrate) 20260808: uRENESAS_Server facade sinks.  Offline
// no-ops; the real bodies (golden cSortCT.cpp:210-279 / :345+) belong to the
// unported cSortCT.cpp wave.  See forms/fSortCT.h for the full behaviour delta
// -- these two are NOT UI-only in golden, so that note is worth reading before
// relying on unload counts or piggy-back triggers in an offline test.
void TfSortCT::ShowLoadingIC() {}
void TfSortCT::ShowSortIC()    {}
TfSortCT *fSortCT = new TfSortCT();
