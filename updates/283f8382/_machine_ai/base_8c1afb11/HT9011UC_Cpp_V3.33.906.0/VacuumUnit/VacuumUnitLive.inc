// =============================================================================
//  VacuumUnit/VacuumUnitLive.inc -- the LIVE half of golden TfVacuumUnit (see VacuumUnit/VacuumUnitLive.h).
//
//  AI(W906-VACUNIT-1203) 20260930: new file, #included at the end of VacuumUnit/VacuumUnit.cpp (ht9045_sm) --
//  an .inc, not a new .cpp (the CMakeLists.txt source lists are the laptop's). EastSun 20260930:
//      「vacuunit 所有功能按鈕和內部功能要有所對應，我需要實際上有功能」
//      「針對1203 開啟 vacuunit 時，如果不符合現在程式碼就新創一個1203分支」
//
//  WHAT RUNS HERE IS GOLDEN'S OWN CODE, on real objects:
//      W906_VacuumLiveBuild   golden CreateForm (HT9045.cpp:274) + Initial :38-110 (the TMyVacuumPanel grids)
//      W906_VacuumLiveShow    golden FormShow :221-247 (fShow, iCount=2, RefreshDOIO on every panel)
//      W906_VacuumLiveTick    golden tmr1Timer :255-302 (every 1 s while the page is open)
//      W906_VacuumLiveSetSV   golden TMyVacuumPanel::btnSVClick :385-389
//      W906_VacuumLiveDo      golden TMyVacuumPanel::btnVaccumOnOffOnClick :391-416 (W906_VaccumOnOff)
//      W906_VacuumLiveSetAll  golden TfVacuumUnit::btnSetInArmClick :550-595 (Tags from the DFM, kVuSetAllArms)
//      W906_VacuumLiveReset   golden TfVacuumUnit::sbResetClick :408-429
//  and every hardware touch inside them goes MyLaneIO / TMyVacuumPanel -> the ECAT-VC8 route
//  (VacuumUnit/Vc8Route.h), which fails CLOSED unless the station really is an ECAT-VC8.
//
//  EASTSUN'S RULINGS 20260930 (R1-R3, relayed by the coordinator) and the defaults that still stand:
//    R1  there is NO ECAT-VC8 on this machine's ring today (「沒有，先把軟體做好」): everything here must refuse
//        or report cleanly without one, and the identity check guards READS too (the route does both).
//    R2  FOLLOW GOLDEN: tmr1Timer's InitialThresholdMode() writes the threshold MODE (8000h+VC*10h:02h = 1)
//        from the first refreshing tick after the page opens, with no press, and retries every tick until it
//        succeeds -- golden's own loop, unchanged. It still goes through the fail-closed route, so with no
//        VC8 nothing is written, the panel shows golden's Error5 and the reason is on the page (the route
//        prints / oplogs a repeated identical refusal once a minute, not every second).
//    R3  FOLLOW GOLDEN: SetIOTableByECAT_VC8_Sucker() runs at boot when VCCU_UNIT_TYPE==1 (golden main.cpp
//        :10497-10498), behind MachineType.h W906_VC8_SUCKER_REMAP -- see VacuumUnit.cpp.
//  DEVIATIONS FROM GOLDEN that remain (each recorded here and in the report):
//    R4  golden refreshes every panel every tick, visible or not (only the PAINT checks Visible). Here the
//        tick reads visible panels only -- the same set golden paints.
//    D1  threshold reads (an SDO each) are capped per tick (thrBudget); the rest wait for the next tick
//        (bNeedReadVaccumThreshold stays true). golden reads them all in one tick.
//    D2  the DO read-back (RefreshDOIO) also runs every tick for the visible panels, and after a DO press --
//        golden reads it only in FormShow and then paints its own toggled Down. Reads of monitor samples,
//        no card traffic; the page must not show a coil state it did not read.
//    D3  a press that would turn vacuum and blow ON together on one VC is refused (here and in the route).
//    D4  every write the page causes (the tick's mode write included) is refused while SystemStart /
//        SoftStart is set; golden's form writes whenever it is open.
//  golden's FormShow file read (ReadFile), Caption and window geometry are the PAGE's (the gen.inc's VU_FormShow,
//  run by editlist.get) and are not repeated here.
// =============================================================================
#include "VacuumUnit/VacuumUnitLive.h"
#include "VacuumUnit/Vc8Route.h"
#include <cstdio>
#include <cstring>
#include "W906FormShowing.h"   // AI(W906-MACH1001) 20261001: W906_FormShowing for the two show-state reads below (ctest FShow_Audit); on the old blank line, so no line below moves
namespace {

struct VuLiveRef_ { TMyVacuumPanel* p; int arm; int col; int row; };
VuLiveRef_    g_vu[4 * TOTAL_VACUUM_UNIT * 2];   // at most 8+8 Index + 4+4 In/Out columns, 2 rows
int           g_vuN = 0;
bool          g_vuBuilt = false;
unsigned long g_vuTicks = 0;
int           g_vuThrReads = 0, g_vuThrDeferred = 0;

void VuCopy_(char* d, int n, const char* s)
{
    if (!d || n <= 0) return;
    std::strncpy(d, s ? s : "", (std::size_t)(n - 1));
    d[n - 1] = '\0';
}

// golden TfVacuumUnit::Initial :38-110, as this port's VacuumUnit.cpp Initial() :156-244 runs it (the HT9050 ruling
// included, AI(W906-HT9050-FAM)), with `this` -> `f` and WITHOUT the elVacuumUnit registration (:112-135).
// Initial() itself is left untouched: the page's HTEditList already holds those keys.
void W906_VuInitialPanels_(TfVacuumUnit* f)
{
    if(MachineTypeChoice==Type_HT9046      ||
       MachineTypeChoice==Type_HT9046_LS   ||
       MachineTypeChoice==Type_HT1032      || MachineTypeChoice==Type_HT9050 ||   // AI(W906-HT9050-FAM) 20260925, as Initial()
       USE_46_SUCKER_DB==1)                                                     //HT9046LS sucker_9046.db
    {
        f->iIndexColMax=TOTAL_VACUUM_UNIT;
    }
    else  //HT9045 sucker.db
    {
        f->iIndexColMax=TOTAL_VACUUM_UNIT/2;
    }
    f->iInOutColMax=TOTAL_VACUUM_UNIT/2;

    if(f->iIndexColMax==TOTAL_VACUUM_UNIT)                                      //HT9046LS sucker_9046.db
    {
        for(int iCol=0; iCol<f->iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                f->myPalArm1[iCol][iRow]                  = new TMyVacuumPanel(f,0,iCol,iRow);      //IndexArm1
                f->myPalArm2[iCol][iRow]                  = new TMyVacuumPanel(f,1,iCol,iRow);      //IndexArm2
            }
        }
    }
    else    //HT9045 sucker.db
    {
        for(int iCol=0; iCol<f->iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                f->myPalArm1[iCol][iRow]                  = new TMyVacuumPanel(f,4,iCol,iRow);      //IndexArm1
                f->myPalArm2[iCol][iRow]                  = new TMyVacuumPanel(f,5,iCol,iRow);      //IndexArm2
            }
        }
    }
    for(int iCol=0; iCol<f->iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            f->myPalInArm[iCol][iRow]                 = new TMyVacuumPanel(f,2,iCol,iRow);     //InArm
            f->myPalOutArm[iCol][iRow]                = new TMyVacuumPanel(f,3,iCol,iRow);     //OutArm
        }
    }
}

TMyVacuumPanel* VuPanel_(int i) { return (i >= 0 && i < g_vuN) ? g_vu[i].p : 0; }

}  // namespace

bool W906_VacuumLiveBuild(char* why, int whyLen)
{
    if (g_vuBuilt) return true;
    //  golden HT9045.cpp:274 Application->CreateForm(__classid(TfVacuumUnit), &fVacuumUnit). wb_serve does not run
    //  FormsBootstrap's InitForms(); if something did, its object is reused.
    if (fVacuumUnit == 0) fVacuumUnit = new TfVacuumUnit(0);
    TfVacuumUnit* f = fVacuumUnit;
    //  ⚠ THE DFM WIDGETS golden's code reads through the form. TfVacuumUnit's ctor leaves every widget pointer
    //  indeterminate (FormsBootstrap.cpp CAUTION) -- the VCL streamed them from the .dfm. Each one this code path
    //  reads is created here with its DFM design value (golden VacuumUnit.dfm):
    //    TMyVacuumPanel ctor (MyVacuumPanel.cpp:172-196)  pnlCurectVal->Color, edSV->Text, btnSV->Caption
    //    btnSetInArmClick (:556-592)                      the button's Tag, edSetInArm / edSetIndexArm / edSetOutArm->Text
    f->pnlCurectVal = new TPanel();       f->pnlCurectVal->Color = 14540252;     // DFM Color = 14540252
    f->pnlThreshold = new TPanel();       f->pnlThreshold->Color = 14540252;
    f->pnlEvent     = new TPanel();       f->pnlEvent->Color     = 14540252;
    f->edSV         = new TEdit();        f->edSV->Text          = "0";          // DFM (the gen.inc's VU_DfmState agrees)
    f->btnSV        = new TSpeedButton(); f->btnSV->Caption      = "Set";
    f->edSetInArm    = new TEdit();  f->edSetInArm->Text    = "0";
    f->edSetIndexArm = new TEdit();  f->edSetIndexArm->Text = "0";
    f->edSetOutArm   = new TEdit();  f->edSetOutArm->Text   = "0";
    TButton* const btn[3] = { f->btnSetInArm = new TButton(), f->btnSetIndexArm = new TButton(), f->btnSetOutArm = new TButton() };
    int n = 0;
    const TVuSetAllArm* arms = W906_VuSetAllArms(&n);
    for (int k = 0; k < n && k < 3; ++k) { btn[k]->Caption = "Set"; btn[k]->Tag = arms[k].tag; }   // the DFM Tags, ONE table
    for (int c = 0; c < TOTAL_VACUUM_UNIT; ++c)
        for (int r = 0; r < 2; ++r) { f->myPalArm1[c][r] = 0; f->myPalArm2[c][r] = 0; }
    for (int c = 0; c < TOTAL_VACUUM_UNIT / 2; ++c)
        for (int r = 0; r < 2; ++r) { f->myPalInArm[c][r] = 0; f->myPalOutArm[c][r] = 0; }
    f->fShow = false;
    W906_VuInitialPanels_(f);
    g_vuN = 0;
    for (int c = 0; c < f->iIndexColMax; ++c)
        for (int r = 0; r < 2; ++r) {
            VuLiveRef_ a = { f->myPalArm1[c][r], kVuArmIndex1, c, r };  g_vu[g_vuN++] = a;
            VuLiveRef_ b = { f->myPalArm2[c][r], kVuArmIndex2, c, r };  g_vu[g_vuN++] = b;
        }
    for (int c = 0; c < f->iInOutColMax; ++c)
        for (int r = 0; r < 2; ++r) {
            VuLiveRef_ a = { f->myPalInArm[c][r],  kVuArmIn,  c, r };  g_vu[g_vuN++] = a;
            VuLiveRef_ b = { f->myPalOutArm[c][r], kVuArmOut, c, r };  g_vu[g_vuN++] = b;
        }
    g_vuBuilt = true;
    if (why && whyLen > 0) why[0] = '\0';
    std::printf("VacuumUnit live: golden TfVacuumUnit panels built (Initial :38-110, no elVacuumUnit registration): "
                "%d panels, index cols %d, in/out cols %d\n", g_vuN, f->iIndexColMax, f->iInOutColMax);
    return true;
}

int W906_VacuumLiveCount() { return g_vuBuilt ? g_vuN : 0; }

bool W906_VacuumLivePanel(int i, TVuLivePanel* out)
{
    TMyVacuumPanel* p = VuPanel_(i);
    if (!p || !out) return false;
    std::memset(out, 0, sizeof(*out));
    std::snprintf(out->id, sizeof(out->id), "%s_%d_%d", W906_VuArmPrefix(g_vu[i].arm), g_vu[i].col, g_vu[i].row);
    out->arm = g_vu[i].arm; out->col = g_vu[i].col; out->row = g_vu[i].row;
    out->kind = p->W906_Kind();
    out->visible = p->GroupBox && p->GroupBox->Visible;
    out->ring = p->SenRing; out->ip = p->SenIP; out->vc = p->SenVCNo; out->senPort = p->SenPort;
    out->onPort = p->OnPort; out->offPort = p->OffPort; out->onType = p->OnType; out->offType = p->OffType;
    out->swap = p->W906_Swap();
    VuCopy_(out->cur, (int)sizeof(out->cur), p->sW906Cur.c_str());
    VuCopy_(out->thr, (int)sizeof(out->thr), p->sW906Thr.c_str());
    VuCopy_(out->evt, (int)sizeof(out->evt), p->sW906Event.c_str());
    out->evtColor = p->ulW906EventColor;
    out->led = p->iW906Led; out->onDown = p->iW906OnDown; out->offDown = p->iW906OffDown;
    out->needThr = p->W906_NeedReadThreshold();
    out->modeOk = p->bInitialThresholdModeOK;
    VuCopy_(out->sEvent, (int)sizeof(out->sEvent), p->sEvent.c_str());
    return true;
}

int W906_VacuumLiveFind(const char* id)
{
    TVuLivePanel lp;
    for (int i = 0; id && i < W906_VacuumLiveCount(); ++i)
        if (W906_VacuumLivePanel(i, &lp) && std::strcmp(lp.id, id) == 0) return i;
    return -1;
}

void W906_VacuumLiveState(TVuLiveState* out)
{
    if (!out) return;
    std::memset(out, 0, sizeof(*out));
    out->built = g_vuBuilt;
    out->initialOK = InitialOK;
    out->ticks = g_vuTicks;
    out->thrReadsLastTick = g_vuThrReads;
    out->thrDeferredLastTick = g_vuThrDeferred;
    if (g_vuBuilt && fVacuumUnit) {
        out->show = W906_FormShowing("fVacuumUnit", fVacuumUnit->fShow);   // AI(W906-MACH1001) 20261001: FShow_Audit -- the member, or the page table saying the browser has the window open (W906FormShowing.h); ctest = the member
        out->iCount = fVacuumUnit->iCount;
        out->indexCols = fVacuumUnit->iIndexColMax;
        out->inOutCols = fVacuumUnit->iInOutColMax;
    }
}

// golden SetPanelPos(fShow) (:157/:161, In/Out Visible) and ShowSuckMode (:170-186, Index Visible from FTestSuck) are
// the page's (the gen.inc runs both in VU_FormShow; this port's ShowSuckMode is GATE-d on the TMyKitSuck include
// conflict). Their result -- each proxy's Visible -- is copied here panel by panel.
void W906_VacuumLiveSetVisible(int i, bool visible)
{
    TMyVacuumPanel* p = VuPanel_(i);
    if (p && p->GroupBox) p->GroupBox->Visible = visible;
}

void W906_VacuumLiveShow()
{
    TfVacuumUnit* f = fVacuumUnit;
    if (!g_vuBuilt || !f) return;
    f->fShow=true;                                    // golden FormShow :221
    // golden :222 tmr1->Enabled=true -> the page's 1 s vacuum.get polls drive W906_VacuumLiveTick
    f->iCount=2;                                      // golden :224
    //IndexArm12                                        golden :227-236
    for(int iCol=0; iCol<f->iIndexColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            f->myPalArm1[iCol][iRow]->RefreshDOIO();

            f->myPalArm2[iCol][iRow]->RefreshDOIO();
        }
    }

    //In/OutArm                                         golden :238-247
    for(int iCol=0; iCol<f->iInOutColMax; iCol++)
    {
        for(int iRow=0; iRow<2; iRow++)
        {
            f->myPalInArm[iCol][iRow]->RefreshDOIO();

            f->myPalOutArm[iCol][iRow]->RefreshDOIO();
        }
    }
}

void W906_VacuumLiveClose()
{
    if (g_vuBuilt && fVacuumUnit) fVacuumUnit->fShow=false;   // golden FormClose :308-309 (tmr1 off, fShow=false)
}

namespace {
// golden's per-panel refresh, as tmr1Timer calls it (golden :266-270 Index / :284-288 In/Out), with R4 / D1 / D2
// applied (see the banner). InitialThresholdMode() is golden's call, every tick (R2 = follow golden).
void VuRefreshOne_(TMyVacuumPanel* p, bool inOut, int& budget)
{
    if (!p || !p->GroupBox || !p->GroupBox->Visible) return;          // R4
    p->RefreshCurrectVal();
    if (p->sEvent=="" && p->W906_NeedReadThreshold()) {               // D1: RefreshThresholdVal would read
        if (budget > 0) { --budget; ++g_vuThrReads; p->RefreshThresholdVal(); }
        else ++g_vuThrDeferred;
    }
    p->RefreshVaccumIO();
    if (!inOut) p->RefreshEvent();                                     // golden :269 / :275 (Index: Event, then Mode)
    p->InitialThresholdMode();                                         // golden :270 / :276 / :287 / :293 (R2)
    if (inOut) p->RefreshEvent();                                      // golden :288 / :294 (In/Out: Mode, then Event)
    p->RefreshDOIO();                                                  // D2
}
}  // namespace

bool W906_VacuumLiveTick(int thrBudget)
{
    TfVacuumUnit* f = fVacuumUnit;
    if (!g_vuBuilt || !f) return false;
    if(InitialOK==false || W906_FormShowing("fVacuumUnit", f->fShow)==false)           // golden :257   AI(W906-MACH1001) 20261001: FShow_Audit -- W906_FormShowing (the member, or the page table); the tick only reads, fail closed without a VC8
        return false;
    ++g_vuTicks;
    g_vuThrReads = 0; g_vuThrDeferred = 0;
    int budget = thrBudget;
    if(f->iCount<=0)
    {
        //Arm2/Arm1
        for(int iCol=0; iCol<f->iIndexColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                VuRefreshOne_(f->myPalArm1[iCol][iRow], false, budget);
                VuRefreshOne_(f->myPalArm2[iCol][iRow], false, budget);
            }
        }
        //In/OutArm
        for(int iCol=0; iCol<f->iInOutColMax; iCol++)
        {
            for(int iRow=0; iRow<2; iRow++)
            {
                VuRefreshOne_(f->myPalInArm[iCol][iRow], true, budget);
                VuRefreshOne_(f->myPalOutArm[iCol][iRow], true, budget);
            }
        }
    }
    else
    {
        f->iCount--;
    }
    return true;
}

int W906_VacuumLiveSetSV(int i, double kPa, char* why, int whyLen)
{
    TMyVacuumPanel* p = VuPanel_(i);
    if (!p) { VuCopy_(why, whyLen, "沒有這個面板"); return -1; }
    AnsiString s;
    s.sprintf("%g", kPa);
    p->edSV->Text = s;                                 // what golden's keyboard leaves in edSV (MyVacuumPanel.cpp:382)
    p->W906_ClickSV();                                 // golden btnSVClick :385-389
    VuCopy_(why, whyLen, "");
    return 0;
}

int W906_VacuumLiveDo(int i, bool onButton, bool down, char* why, int whyLen)
{
    TMyVacuumPanel* p = VuPanel_(i);
    if (!p) { VuCopy_(why, whyLen, "沒有這個面板"); return -1; }
    p->RefreshDOIO();                                  // the read-back this press is decided against
    const int other = onButton ? p->iW906OffDown : p->iW906OnDown;
    if (down && other != 0) {                          // D3
        VuCopy_(why, whyLen, other < 0 ? "另一顆（吸／破真空）的輸出讀不到——不知道它是不是開著，所以不開這一顆"
                                       : "同一個 VC 的另一顆（吸／破真空）正開著——吸真空與破真空不同時開（偏離 golden，已記錄），先關掉它");
        return (int)kVc8RcRefused;
    }
    const int ret = p->W906_VaccumOnOff(onButton, down);   // golden btnVaccumOnOffOnClick :391-416
    p->RefreshDOIO();                                  // D2: paint the card's read-back
    VuCopy_(why, whyLen, ret == 0 ? "" : W906_Vc8LastWhy());
    return ret;
}

int W906_VacuumLiveSetAll(int tag, double kPa, char* why, int whyLen)
{
    TfVacuumUnit* f = fVacuumUnit;
    if (!g_vuBuilt || !f) { VuCopy_(why, whyLen, "VacuumUnit 沒有建立"); return -1; }
    TButton* b = tag == 0 ? f->btnSetInArm : tag == 1 ? f->btnSetIndexArm : tag == 2 ? f->btnSetOutArm : 0;
    TEdit*   e = tag == 0 ? f->edSetInArm  : tag == 1 ? f->edSetIndexArm  : tag == 2 ? f->edSetOutArm  : 0;
    if (!b || !e || b->Tag != tag) { VuCopy_(why, whyLen, "不認得的 Set All 按鈕（Tag）——拒絕"); return -1; }
    AnsiString s;
    s.sprintf("%g", kPa);
    e->Text = s;                                       // what golden's keyboard leaves in edSet*Arm (VacuumUnit.cpp:599)
    f->btnSetInArmClick(b);                            // golden :550-595, Ptr->Tag from the DFM
    VuCopy_(why, whyLen, "");
    return 0;
}

int W906_VacuumLiveReset(char* why, int whyLen)
{
    TfVacuumUnit* f = fVacuumUnit;
    if (!g_vuBuilt || !f) { VuCopy_(why, whyLen, "VacuumUnit 沒有建立"); return -1; }
    f->sbResetClick(0);                                // golden :408-429 (bInitialThresholdModeOK=false: the next tick writes the mode, R2)
    VuCopy_(why, whyLen, "");
    return 0;
}
