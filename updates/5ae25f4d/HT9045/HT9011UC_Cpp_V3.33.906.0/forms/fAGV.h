// =============================================================================
//  forms/fAGV.h  --  non-VCL stand-in for golden's fAGV form pointer
//
//  AI(W906-W7-F0) 20260728: split out of FormsFacade.h by the W7-F0 refactor
//  (docs/W7_UI_ARCHITECTURE_PLAN.md SS6-F0-d).  Content moved VERBATIM apart
//  from the F0-e virtualisation (virtual dtor + virtual methods) and the widget
//  types now coming from forms/FormWidgets.h.  See forms/fMain.h for the
//  facade-wide contract every form in this directory obeys.
//
//  Original translation wave: W6.1 (Empty-tray canary) ;
//  EXTENDED W5-Final-AGV_E84 INTEGRATE ; W6.3.
// =============================================================================
#ifndef FORMS_FAGV_H
#define FORMS_FAGV_H

#include "forms/FormWidgets.h"

// ---------------------------------------------------------------------------
//  TfAGV -- non-VCL stand-in.  Started at W6.1 as ONE hardcoded-false method
//  (IsATK_AMR, the Empty-tray canary's only form-pointer deref).
//  AI(W906-W7-F0-fix) 20260728: that "DUMMY non-AMR path" note is stale and is
//  removed -- since W5-Automation-Integrate all 3 predicates delegate to the
//  REAL translated bodies in Automation/AGV_predicates.cpp (compiled into
//  ht9045_forms alongside this form, see CMakeLists.txt).  They still evaluate
//  false on the untouched offline baseline, but by CONFIG (USE_COVER_TRAYID
//  defaults tCIDNotUse / IniConfig.bA65_BundleIDList defaults false), not by a
//  hardcoded return -- see forms/fAGV.cpp.
// ---------------------------------------------------------------------------
class TfAGV
{
public:
    virtual bool IsSPIL_AMR();       // [METHOD] golden AGV.h:204 -- W5-Automation INTEGRATE: real (Automation/AGV_predicates.cpp)
    virtual bool IsATK_AMR();        // [METHOD] golden AGV.h:205 -- ditto (real)
    virtual bool Use_AMR();  bool bATK_AMR_DoLotEndSent = false;          // [METHOD] golden AGV.h:206 -- W6.3 ADD; ditto (real)   //AI(W906-SETLOTSTATE) 20261001: + [DATA] golden AGV.h:200 bATK_AMR_DoLotEndSent (ATK AMR: DoLotEnd already sent at Final Lot End; written by TfMain::SetLotState, read by golden uLotInfo.cpp:2020-2022); false = golden VCL zero-init
    // -- W5-Final-AGV_E84 INTEGRATE ADD: member the E84 loader/unloader
    //    handshake engine (Automation/AGV_E84.cpp) derefs -----------------------
    TfMainMemo *mmE84Log;            // [DATA] golden AGV.h:85 (TMemo* E84 log) -- reuse existing TfMainMemo/TfMainMemoLines shape (meShuttle1/2's shape)
    // AI(W906-FW-QWKEY6) 20260824: the 3 MouseDown handlers the original
    // AGV_PortScan wave omitted (unspellable TMouseButton/TShiftState +
    // fQwertyKey then had no port). Signatures trimmed to what each golden
    // body reads (Sender only). Bodies: Automation/AGV_PortScan.cpp append
    // block. Live, latent until a real event stream wires OnMouseDown.
    void edAuto1CountMouseDown(TObject *Sender);             // golden AGV.cpp:1133-1137
    void edE84_1_TP1MouseDown(TObject *Sender);              // golden AGV.cpp:1139-1143
    void edAGVWorryingwattingtimeMouseDown(TObject *Sender); // golden AGV.cpp:1309-1313
    TfAGV();
    virtual ~TfAGV() {}
};

extern TfAGV *fAGV;     // golden: extern PACKAGE TfAGV *fAGV; (AGV.h:210)

#endif // FORMS_FAGV_H
