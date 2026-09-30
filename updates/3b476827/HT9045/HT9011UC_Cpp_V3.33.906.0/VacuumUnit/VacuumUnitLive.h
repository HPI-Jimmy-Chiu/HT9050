// =============================================================================
//  VacuumUnit/VacuumUnitLive.h -- the LIVE half of golden TfVacuumUnit, for HW.VacuumUnit.
//
//  AI(W906-VACUNIT-1203) 20260930: new file. EastSun 20260930: 「vacuunit 所有功能按鈕和內部功能要有所對應，
//  我需要實際上有功能」. Plain C++ interface (no VCL, no EtherCAT header) between
//      FileRW/TestIF_File_VacuumUnit.cpp   the page service (WS vacuum.*), a wb_serve TU. It CANNOT include
//                                          VacuumUnit.h: its generated TestIF_File_VacuumUnit.gen.inc redefines
//                                          TOTAL_VACUUM_UNIT / fShow / iIndexColMax for its HTEditList proxies.
//      VacuumUnit/VacuumUnitLive.inc       the implementation, #included at the end of VacuumUnit/VacuumUnit.cpp
//                                          (ht9045_sm): the REAL golden objects -- fVacuumUnit and its
//                                          TMyVacuumPanel myPalArm1/2 / myPalInArm / myPalOutArm -- built by golden
//                                          Initial's panel half (:38-110) WITHOUT its elVacuumUnit registration
//                                          (:112-135; the page's HTEditList already has those keys, and a second
//                                          registration would duplicate them -- TestIF_File_VacuumUnit.cpp:29-30).
//  So the panel address table IS golden's TMyVacuumPanel constructor (MyVacuumPanel.cpp:16-272, ring / station /
//  VC / swap / DO and DI channels), not a copy of it, and every button runs golden's own method.
// =============================================================================
#ifndef VACUUMUNIT_VACUUMUNITLIVE_H
#define VACUUMUNIT_VACUUMUNITLIVE_H

#include <cstring>

// ---------------------------------------------------------------------------
//  The three "Set All Value" buttons share ONE handler, golden btnSetInArmClick, which dispatches on the
//  button's Tag. The Tags come from the golden DFM (VacuumUnit/VacuumUnit.dfm), not from the port:
//      btnSetInArm     :223   no Tag line  -> 0   (InArm)
//      btnSetIndexArm  :250   Tag = 1 (:251)       (Index Arm1 / Arm2)
//      btnSetOutArm    :260   Tag = 2 (:261)       (OutArm)
//  tests/test_vacuum_vc8.cpp asserts this table against tools/dfm2rc/ir_out/VacuumUnit/VacuumUnit.dfm.ir.json.
//  The web page names the arm ("in" / "index" / "out"); an unknown name maps to -1 and is refused.
// ---------------------------------------------------------------------------
struct TVuSetAllArm { const char* arm; const char* button; const char* edit; int tag; int dfmLine; };
inline const TVuSetAllArm* W906_VuSetAllArms(int* n)
{
    static const TVuSetAllArm k[] = {
        { "in",    "btnSetInArm",    "edSetInArm",    0, 223 },
        { "index", "btnSetIndexArm", "edSetIndexArm", 1, 250 },
        { "out",   "btnSetOutArm",   "edSetOutArm",   2, 260 },
    };
    if (n) *n = (int)(sizeof(k) / sizeof(k[0]));
    return k;
}
inline int W906_VuSetAllTag(const char* arm)
{
    int n = 0;
    const TVuSetAllArm* k = W906_VuSetAllArms(&n);
    for (int i = 0; arm && i < n; ++i)
        if (std::strcmp(k[i].arm, arm) == 0) return k[i].tag;
    return -1;
}

// golden's threshold keyboard range (MyVacuumPanel.cpp:382 edSVClick / VacuumUnit.cpp:599 edSetInArmClick:
// ShowQwertyKey(..., -116.0, 148.0)) and elVacuumUnit's range (:121-133). The page service refuses outside it.
inline bool W906_VuKpaOk(double kPa) { return kPa == kPa && kPa >= -116.0 && kPa <= 148.0; }

// Which grid a panel is on -- the page's id prefix (hwidgets.js makeVacuumPanel / the gen.inc's proxies).
enum { kVuArmIndex1 = 0, kVuArmIndex2 = 1, kVuArmIn = 2, kVuArmOut = 3 };
inline const char* W906_VuArmPrefix(int arm)
{
    return arm == kVuArmIndex1 ? "myPalArm1" : arm == kVuArmIndex2 ? "myPalArm2" : arm == kVuArmIn ? "myPalInArm" : "myPalOutArm";
}

// One live panel as the page service sees it. Every field is a golden member or what golden paints.
struct TVuLivePanel {
    char          id[24];          // "myPalInArm_0_0" = <prefix>_<iCol>_<iRow>
    int           arm, col, row;
    int           kind;            // golden _iKind (0/1 HT9046LS Index, 2 InArm, 3 OutArm, 4/5 HT9045 Index)
    bool          visible;         // golden GroupBox->Visible (from the page: SetPanelPos / ShowSuckMode)
    int           ring, ip, vc;    // golden SenRing / SenIP / SenVCNo
    int           senPort;         // 128 + VC       (vacuum-OK DI channel)
    int           onPort, offPort; // 16 + VC*2 + swap (vacuum / blow DO channel)
    int           onType, offType;
    int           swap;            // golden iVacuOnOffSwap
    char          cur[24], thr[24], evt[24];   // what golden paints
    unsigned long evtColor;        // TColor (BGR) of the event text
    int           led;             // golden myld1->Value; -1 not read yet
    int           onDown, offDown; // golden bplOn / bplOff->Down; -1 not read
    bool          needThr;         // golden bNeedReadVaccumThreshold
    bool          modeOk;          // golden bInitialThresholdModeOK
    char          sEvent[24];      // golden sEvent ("" = no error)
};

struct TVuLiveState {
    bool          built;           // the live objects exist
    bool          show;            // golden fShow (the live view runs)
    int           iCount;          // golden iCount (FormShow sets 2: two quiet ticks first)
    bool          initialOK;       // golden InitialOK (tmr1Timer's own first guard)
    int           indexCols, inOutCols;   // golden iIndexColMax / iInOutColMax
    unsigned long ticks;           // W906_VacuumLiveTick bodies run
    int           thrReadsLastTick;
    int           thrDeferredLastTick;
};

bool W906_VacuumLiveBuild(char* why, int whyLen);   // once; golden CreateForm + Initial :38-110
int  W906_VacuumLiveCount();
bool W906_VacuumLivePanel(int i, TVuLivePanel* out);
int  W906_VacuumLiveFind(const char* id);           // -1 = none
void W906_VacuumLiveState(TVuLiveState* out);
void W906_VacuumLiveSetVisible(int i, bool visible);
void W906_VacuumLiveShow();                         // golden FormShow's live half (:221-247)
void W906_VacuumLiveClose();                        // golden FormClose's live half (:308-309)
bool W906_VacuumLiveTick(int thrBudget);            // golden tmr1Timer (:255-302), R2 / R4, false = did not run
int  W906_VacuumLiveSetSV(int i, double kPa, char* why, int whyLen);            // golden btnSVClick
int  W906_VacuumLiveDo(int i, bool onButton, bool down, char* why, int whyLen); // golden btnVaccumOnOffOnClick
int  W906_VacuumLiveSetAll(int tag, double kPa, char* why, int whyLen);         // golden btnSetInArmClick
int  W906_VacuumLiveReset(char* why, int whyLen);                               // golden sbResetClick

#endif  // VACUUMUNIT_VACUUMUNITLIVE_H
