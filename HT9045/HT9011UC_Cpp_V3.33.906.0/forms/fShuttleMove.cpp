// =============================================================================
//  forms/fShuttleMove.cpp  --  definitions for the fShuttleMove facade
//
//  AI(W906-W7-L2) 20260803: NEW FILE.  See forms/fShuttleMove.h for the golden
//  provenance (ShuttleMove.h:14 / :40 / :110 / :121) and for the per-field
//  offline-default justification.
// =============================================================================
#include "forms/fShuttleMove.h"
#include "cprod.h"     // Prod.iInSH1SenICAddPos / iInSH2SenICAddPos   AI(W906-SM-READDATA) 20260925
#include "common.h"    // ReadIniData, GetLastOpenFN, DataPath         AI(W906-SM-READDATA) 20260925

// --- W906-W7-L2 ADD: TfShuttleMove (first home, see forms/fShuttleMove.h) ---
//
// bShuttleRetry(false) mirrors golden's own construction-time value
// (ShuttleMove.cpp:63, inside `__fastcall TfShuttleMove::TfShuttleMove` at :59)
// and the value FormShow re-imposes on every open (ShuttleMove.cpp:87).  It is
// the branch selection documented on the member: it keeps golden ckernel.cpp's
// WaitManualRetryKey on its `return false` arm (:148) instead of the
// auto-confirm arm (:144-146).
//
// btRetry is heap-allocated in the ctor body -- same shape as
// forms/fCleaning.cpp's edCleaningCount and forms/fMain.cpp's widget members.
// Its Caption/Visible defaults come from vclcompat::TButton / TControl and are
// NOT overridden here; the header explains why for each (Caption is write-only
// at ckernel.cpp:138/:140; Visible's false already matches golden's
// ShuttleMove.dfm:794 + ShuttleMove.cpp:97).
TfShuttleMove::TfShuttleMove() : bShuttleRetry(false), fShow(false), iInShtLtcPercent(80)   // AI(W906-ST-S2-fShow-B1) 20260917: offline, no form is on screen -- see the header   // AI(W906-SM-READDATA) 20260925: iInShtLtcPercent(80) = golden MyCCLinkSensor.cpp:42
{
    btRetry = new vclcompat::TButton();
}

// The facade global lives for the whole process, exactly like every other
// forms/ singleton here (fNote, fCleaning, fOCR, ...), so the ctor's `new` is
// never paired with a delete -- matching the established convention rather
// than introducing a lone owning-dtor idiom in one file.  Golden defines the
// same global at ShuttleMove.cpp:38 (`TfShuttleMove *fShuttleMove;`, filled in
// by the VCL form loader); offline it is constructed eagerly so that
// ckernel.cpp's 4 dereferences never see a null pointer.
TfShuttleMove *fShuttleMove = new TfShuttleMove();

// AI(W906-SM-READDATA) 20260925 (Steven 團隊, C 路 ShuttleMove): golden TfShuttleMove::ReadData
// (ShuttleMove.cpp:2252-2264, KenHsieh 20250722), line for line.  The only substitution is the
// receiver of the third read: golden fCCLink->iInShtLtcPercent -> this->iInShtLtcPercent (see the
// header: TfCCLink is not ported).  The two commented-out reads at :2262-2263 stay commented out,
// exactly as golden has them.
void TfShuttleMove::ReadData()                                                  //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
{
    AnsiString S, szDir="";

    S=GetLastOpenFN();
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);

    Prod.iInSH1SenICAddPos=ReadIniData(szDir, "Shuttle", "InSH1SenICAddPos", 0);
    Prod.iInSH2SenICAddPos=ReadIniData(szDir, "Shuttle", "InSH2SenICAddPos", 0);
    iInShtLtcPercent=ReadIniData(szDir, "Shuttle", "InShtLtcPercent", 80);      //KenHsieh 20251128 : InSht Latch add percent for auto teach   (golden: fCCLink->iInShtLtcPercent)
//    Prod.iInSH1SenICDetectZ1=ReadIniData(szDir, "Shuttle", "InSH1SenICDetectZPos", 0);
//    Prod.iInSH2SenICDetectZ1=ReadIniData(szDir, "Shuttle", "InSH2SenICDetectZPos", 0);
}

// AI(W906-SM-READDATA) 20260926: free-function entry for tools/wb_serve.cpp (its boot chain calls golden
// main.cpp:11741 `fShuttleMove->ReadData()` without pulling this header into that TU).
void W906_ShuttleMoveReadData() { fShuttleMove->ReadData(); }
