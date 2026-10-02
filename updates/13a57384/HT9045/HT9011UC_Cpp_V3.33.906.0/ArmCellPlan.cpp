// =============================================================================
//  ArmCellPlan.cpp  --  see ArmCellPlan.h (contract, sources, rulings).
//
//  AI(W906-ARMCELL) 20261002 [W906]: RULINGS_20261002 #18. Pure: reads only the ArmCellInputs it is given.
//
//  Golden quirks kept (NB2 spec §4.3 numbering; "found" = seen while copying, not in the spec):
//    q1  HotPlate index 0 is HotPlate 2 (golden MoveInArmXYToHotPlatePlace :1854 "iPlate=0 先到2號加熱盤"): area
//        "HotPlate2" reads Plate2, its scale index [1]; "HotPlate1" Plate1, index [0].
//    q2  SetTechDataToProd_InArm overwrites Plate1's per-nozzle Y copies from Plate2 (port cinitial.cpp:14970) -- the
//        movers read only the base slot, so it never reaches a target; nothing to copy.
//    q3  Loader single-nozzle path: the X block pitch is computed and then lost (GetInArmToLoaderPosition_Single
//        recomputes iXPos), only Y keeps it; Out single path: OutArmAddBlockPitch runs twice and X is recomputed in
//        between, so Y gets the block pitch twice, X once (aoutarm.cpp:4013-4044). Both only with bUseTrayBlockMode &&
//        bP06_LoaderUseCarrierTray. OutArmAddBlockPitch reads AutoForm[0] whatever the tray (golden).
//    q4  TransferHotPlateRatio's Y clamp writes X (ainarm2.cpp: `if(abs(*iYPos-iOldY)>iRet) *iXPos=iOldX;`).
//    q5  the Out bin-box / over-limit tests on the global iWhichAuto and AutoCalculateOutArmXClosePitch(iWhichAuto+
//        bOutArmXOverLimit) only feed dOutArmXPitch_1Step -- a multi-nozzle term, 0 under D1; nothing to copy.
//    q6  Prod.ZPlace[eBulkBox] / [eMag*] are never written (golden cinitial.cpp:10005-10040 stop at eFix12) ->
//        BinBox / Magazine Z down is not allowed.
//    q7  the E46 two-offset Loader branch ignores "ep1 adds no pitch" -- it is in the multi-nozzle path only.
//    q8  per-nozzle GetArmX / GetArmY only at Loader (In) and Auto / Fix (Out) single paths; HotPlate, shuttles, Clean
//        Kit, BinBox never add them.
//    f1  (found) ARM_OFFSET::GetArmX/Y return -dArmX/-dArmY when OneByOne is off (cprod.h:209-222), so on those two
//        single paths the table's X / Y offset is taken back out of the base point. golden adds them only for the
//        nozzles going down (bZFlag); here always for the chosen nozzle, so the X/Y target does not depend on zDown.
//    f2  (found) Plate2's Y in TransferHotPlateRatio applies the expansion coefficient only in its last else
//        (ainarm2.cpp, Plate2 non-tri-temp branch); Plate1's Y applies it after the if / else, like X.
//    f3  (found) GetShtRowColPos's NS7000 shift adds for MInShuttle1 and subtracts for every other shuttle (MInShuttle2
//        too); TransferOutShuttleRatio's setup-file branch scales Y with the X factor; its E32 branch uses the IN
//        shuttle scales (LastSet.fInShuttleX/YScale).
//    f4  (found) MoveInArmXYPickCleanKit with bUse8Picker == false adds iARM_Y_PITCH to Y for every nozzle row, and
//        MoveInOutArmZToKitPickPlace then lets only row B go down (AutoClean.cpp:1199-1216) -- kept: such a machine
//        gets no Z down at the Clean Kit for a row-A nozzle.
//    f5  (found) HotPlate place Z with E33 + E88 reads Tech.iInArmZHeightSub[i][j] directly (no _16, no +200) and,
//        with E34, HotPlate2's place offset is InOfsHP1's (port cinitial.cpp:14984-15003).
//  D1 drops (multi-nozzle terms, RULINGS_20261002 #18): (iInArmXBase - iMyCol) * dInArmXPitch_1Step, iVariablePara *
//    dOutArmXPitch_1Step, the HotPlate (2-c)*pitch/3 family, dMovePitchX * (iInArmXBase - c) at the Clean Kit, and the
//    nozzle-row Y terms (iMovePitchY * iInArmOrder, ArmRow1NotICForPlaceToPlate, iOutARM_Y_PITCH, the Clean Kit row-B
//    pitch when bUse8Picker). An automatic Y pitch is refused (v1), so the auto-pitch branches are not copied.
//  [W906] not golden: the area list / refusals (v1), the D1 rule, D2 (job side), Tech-recomputed base points.
// =============================================================================
#include "ArmCellPlan.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace ht9045 {

double ArmCellOffset::ArmX(int r, int c) const
{
    if (r < 0 || r > 3 || c < 0 || c > 7) return 0.0;
    return oneByOne ? posX[r][c] : -x;
}
double ArmCellOffset::ArmY(int r, int c) const
{
    if (r < 0 || r > 3 || c < 0 || c > 7) return 0.0;
    return oneByOne ? posY[r][c] : -y;
}
double ArmCellOffset::PickUpRC(int r, int c) const { return (r < 0 || r > 3 || c < 0 || c > 7) ? 0.0 : pickUpRC[r][c]; }
double ArmCellOffset::PlaceRC(int r, int c) const  { return (r < 0 || r > 3 || c < 0 || c > 7) ? 0.0 : placeRC[r][c]; }

int ArmCellShtStartPos(int itemCount, int centre, int gap)
{
    const int iPitchCount = itemCount - 1;                                      // golden FindCentorPointIndex
    const double dCentorIndex = (double)iPitchCount / 2;
    return centre - (int)(dCentorIndex * gap);                                  // golden GetShtStartPos (int, double, int)
}

float ArmCellFloatQuotient(double n, double d)
{
    double str = 0.00;
    if (d != 0) str = (n / d);
    return (float)str;
}

bool ArmCellIsHt9050(const std::string& gpibModel, bool typeIsHt9050)
{
    return gpibModel == "9050GPIB" || typeIsHt9050;                             // = WebBridgeTags.cpp W906_MainCaption's HT-9050 test (review M1)
}

namespace {

int Tr(double v) { return (int)v; }   // golden: a double expression assigned to an int -- truncation toward zero (BCB6 and MinGW alike)

std::string Fmt(const char* f, ...)
{
    char b[512];
    va_list ap;
    va_start(ap, f);
    std::vsnprintf(b, sizeof(b), f, ap);
    va_end(ap);
    return b;
}

const char* const kV1Why = "第一版未做（9050 沒有這一區；RULINGS_20261002 第 18 條：第一版只做 9050 有的區）";

struct AreaDef { const char* id; const char* label; int arm; bool v1; };
const AreaDef kAreas[] = {
    { "Loader",      "Loader",        0, true  },
    { "HotPlate1",   "HotPlate 1",    0, false },
    { "HotPlate2",   "HotPlate 2",    0, true  },
    { "InShuttle1",  "In Shuttle 1",  0, true  },
    { "InShuttle2",  "In Shuttle 2",  0, false },
    { "CleanKit",    "Clean Kit",     0, true  },
    { "OutShuttle1", "Out Shuttle 1", 1, true  },
    { "OutShuttle2", "Out Shuttle 2", 1, false },
    { "Auto1",       "Auto 1",        1, true  },
    { "Auto2",       "Auto 2",        1, true  },
    { "Auto3",       "Auto 3",        1, true  },
    { "Auto4",       "Auto 4",        1, false },
    { "Auto5",       "Auto 5",        1, false },
    { "Auto6",       "Auto 6",        1, false },
    { "Fix1",        "Fix 1",         1, true  },
    { "Fix2",        "Fix 2",         1, true  },
    { "Fix3",        "Fix 3",         1, true  },
    { "Fix4",        "Fix 4",         1, false },
    { "Fix5",        "Fix 5",         1, false },
    { "Fix6",        "Fix 6",         1, false },
    { "BinBox",      "Bin Box",       1, false },
    { "Magazine",    "Magazine",      1, false },
};
const int kAreaCount = (int)(sizeof(kAreas) / sizeof(kAreas[0]));

const AreaDef* FindArea(const std::string& id)
{
    for (int i = 0; i < kAreaCount; ++i) if (id == kAreas[i].id) return &kAreas[i];
    return 0;
}

// the arm's Enable Z (the golden Z All Up grid filtered by Enable, NB2 spec §4.1)
int EnabledZ(const ArmCellArmInputs& a, int& r, int& c)
{
    int n = 0;
    r = c = -1;
    for (int i = 0; i < a.motRow && i < 2; ++i)
        for (int j = 0; j < a.motCol && j < 8; ++j)
            if (a.z[i][j].present && a.z[i][j].enable) { if (n == 0) { r = i; c = j; } ++n; }
    return n;
}

// v1 arm-level refusals (RULINGS_20261002 #18: only what HT9050 has). "" = the arm can be planned.
std::string ArmCheck(const ArmCellInputs& in, int arm, int& r, int& c)
{
    const ArmCellArmInputs& a = in.arm[arm];
    const char* name = arm == 0 ? "In Arm" : "Out Arm";
    r = c = -1;
    if (in.aoa)
        return Fmt("%s：AOA 自動對位開著（MACHINE_HAS_AUTO_ALIGNMENT_CCD＋bEnableAutoAlignment）—— 第一版沒有做 AOA 的補償（9050 沒有）", name);
    if (in.pickerUse == kAcPickCyn)
        return Fmt("%s：吸嘴是氣缸（INOUT_ARM_PICKER_USE_MOTOR=0）—— 第一版只做馬達 Z 的手臂（9050）", name);
    if (in.pickerUse == kAcPickMotCyn)
        return Fmt("%s：一支 ZA 帶全部氣缸吸嘴（INOUT_ARM_PICKER_USE_MOTOR=2，eptUseMotCyn）—— 第一版不做（RULINGS_20261002 第 18 條）", name);
    for (std::size_t k = 0; k < a.pitch.size(); ++k)
        if (a.pitch[k].present && a.pitch[k].enable)
            return Fmt("%s：這台有 pitch 軸（%s 在 Mot_Table 是 Enable=1）—— 第一版不做有 pitch 軸的機台（9050 沒有；RULINGS_20261002 第 18 條）",
                       name, a.pitch[k].alias.c_str());
    if (!(a.x.present && a.x.enable) || !(a.y.present && a.y.enable))
        return Fmt("%s：X／Y 軸沒有裝（Mot_Table %s／%s）", name, a.x.alias.empty() ? "?" : a.x.alias.c_str(),
                   a.y.alias.empty() ? "?" : a.y.alias.c_str());
    if (!a.x.is1203 || !a.y.is1203)
        return Fmt("%s：X／Y 不是 PCI1203 軸 —— 第一版只接 PCI1203 的手臂（9050）", name);
    const int n = EnabledZ(a, r, c);
    if (n == 0) return Fmt("%s：這支手臂沒有 Enable 的 Z（Mot_Table）", name);
    if (n > 1)
        return Fmt("%s：有 %d 支 Enable 的 Z —— 多吸嘴第一版不做（9050 只有一支；RULINGS_20261002 第 18 條 D1）", name, n);
    if (!a.z[r][c].is1203)
        return Fmt("%s：%s 不是 PCI1203 軸 —— 第一版只接 PCI1203 的手臂（9050）", name, a.z[r][c].alias.c_str());
    if (a.yAutoPitch)
        return Fmt("%s：自動 Y pitch（USE_%s_ARM_Y_PITCH）—— 第一版不做（9050 是固定 pitch）", name, arm == 0 ? "IN_OUT" : "OUT");
    if (r >= a.maxRow || c >= a.maxCol)
        return Fmt("%s：%s 在吸嘴格 [%d][%d]，超出 golden 每支吸嘴的 Prod 表（iMaxRow×iMaxCol=%d×%d）—— golden 沒有這支吸嘴的高度",
                   name, a.z[r][c].alias.c_str(), r, c, a.maxRow, a.maxCol);
    return std::string();
}

// golden SetTechDataToProd_TrayArm (port cinitial.cpp:5938-5946): !bP06 zeroes the Loader block fields in place,
// and it runs before _InArm / _OutArm.
ArmCellForm LoaderForm(const ArmCellInputs& in)
{
    ArmCellForm f = in.loadForm;
    if (!in.p06) {
        f.blockXItem = 1; f.blockYItem = 1;
        f.blockXStart = 0.0; f.blockYStart = 0.0; f.blockPitchX = 0.0; f.blockPitchY = 0.0;
    }
    return f;
}

// the D2 shuttle axis and its arm-side point (golden InShtInLF / OutSHT1InRT / OutSHT2InRT, port csystem.cpp:18458,
// csystem_predicates.cpp:314-345; the points are golden SetTechDataToProd_Shuttle's Prod.InSHT / OutSHT)
void D2Of(const ArmCellInputs& in, bool inArm, int s, ArmCellAxis& ax, int& target, std::string& what)
{
    if (inArm) {
        ax = in.inShuttle[s];
        target = Tr((double)in.techInShtLeft[s] + in.shLeftPod[s] + in.shuttleTempPos[s][0]);   // Prod.InSHT[s].iLeft
        what = Fmt("In Arm 放料時 In Shuttle %d 停在左邊（golden InSHT%dInLF：Prod.InSHT[%d].iLeft）", s + 1, s + 1, s);
        return;
    }
    //AI(W906-ARMCELL) 20261002: review M1 -- in.ht9050Type is the run-time HT9050 test (ArmCellIsHt9050), not MachineTypeChoice alone:
    //  this HT9050 decodes as Type_HT9046_LS, and its Out Arm picks from MOutShuttle1 / MOutShuttle2 (machines/HT9050/Mot_Table.csv
    //  M17 / M18), not from the In shuttle. The production predicate csystem_predicates.cpp OutSHT1InRT still tests the type only
    //  (910 / golden as translated) -- D2 is [W906] and follows the hardware.
    if (s == 0 && in.ht9050Type) {                                              // golden (910) OutSHT1InRT Type_HT9050 arm (csystem_predicates.cpp:315)
        ax = in.outShuttle[0];
        target = in.techOutShtRight[0];                                         // Prod.OutSHT[0].iRight = Tech.iOutShuttle1Right
        what = "Out Arm 取料時 Out Shuttle 1 停在右邊（HT9050：MOutShuttle1 在 Prod.OutSHT[0].iRight，golden OutSHT1InRT 的 Type_HT9050 分支）";
        return;
    }
    ax = in.inShuttle[s];                                                       // golden OutSHT1InRT else / OutSHT2InRT -> InSHTnInRT
    target = Tr((double)in.techInShtRight[s] + in.shRightPod[s] + in.shuttleTempPos[s][1]);     // Prod.InSHT[s].iRight
    what = Fmt("Out Arm 取料時 Shuttle %d 停在右邊（golden OutSHT%dInRT → InSHT%dInRT：Prod.InSHT[%d].iRight；這台不是 HT9050，golden 看 In Shuttle 那一軸）",
               s + 1, s + 1, s + 1, s);
}

// ---- area evaluation (catalog and plan share it) ----
ArmCellArea EvalArea(const ArmCellInputs& in, const AreaDef& d, const std::string& armWhy, int nr)
{
    ArmCellArea a;
    a.arm = d.arm == 0 ? "in" : "out";
    a.id = d.id; a.label = d.label; a.v1 = d.v1;
    a.zDownAllowed = true;
    const std::string id = d.id;
    std::string why;
    if (id == "Loader") {
        const ArmCellForm lf = LoaderForm(in);
        a.cols = lf.xDivision * lf.blockXItem;                                  // golden MOT[MMTrayY].Tray XItem = XDivision*BlockXItem
        a.rows = lf.yDivision * lf.blockYItem;
        a.installed = lf.xDivision > 0 && lf.yDivision > 0;
        if (!a.installed) why = "Loader：料盤格數是 0（工作檔 Tray.Data 的 X／Y Division 沒有讀到）";
        a.zKind = "pick";
    } else if (id == "HotPlate1" || id == "HotPlate2") {
        const int plate = (id == "HotPlate2") ? 0 : 1;                          // q1: index 0 = HotPlate 2
        a.cols = in.hotPlateForm.xDivision;
        a.rows = in.hotPlateForm.yDivision;
        a.installed = in.plateSelect[plate] && a.cols > 0 && a.rows > 0;
        if (!in.plateSelect[plate])
            why = Fmt("%s：工作檔 HotPlate.Data 的 Using Flag 不含這一盤（Prod.bPlateSelect[%d]=0）", d.label, plate);
        else if (!a.installed) why = Fmt("%s：加熱盤格數是 0", d.label);
        a.zKind = "place";
    } else if (id == "InShuttle1" || id == "InShuttle2") {
        const int s = (id == "InShuttle2") ? 1 : 0;
        const ArmCellAxis& ax = in.inShuttle[s];
        a.cols = in.shtCol; a.rows = in.shtRow;
        if (!ax.present || !ax.enable)
            why = Fmt("%s：Mot_Table 的 %s 沒有裝（%s）", d.label, ax.alias.empty() ? (s ? "MInShuttle2" : "MInShuttle1") : ax.alias.c_str(),
                      ax.present ? "Enable=0" : "沒有這一列");
        else if (in.shuttleMode == 1 && in.shuttleSel != s)
            why = Fmt("%s：工作檔只用 Shuttle %d（HandlerCondition.Data Shuttle Mode=1、Shuttle1 Cancel=%d）", d.label, in.shuttleSel + 1, in.shuttleSel);
        else if (in.shtCol <= 0 || in.shtRow <= 0)
            why = Fmt("%s：Shuttle 的格數是 0（TestSocket.iShtCol／iShtRow）", d.label);
        a.installed = why.empty();
        a.zKind = "place";
        a.zDownWhy = "Shuttle 區：只有 shuttle 停在手臂側（READY 而且在教導點的 gap 內）才降 Z，X／Y 照走（RULINGS_20261002 第 18 條 D2）";
    } else if (id == "CleanKit") {
        a.cols = in.acXDivision; a.rows = in.acYDivision;
        if (!in.acFunction) why = "Clean Kit：Auto Clean 沒開（TestIF iAutoClean_Function=0）";
        else if (in.e43) why = "Clean Kit：E43 Auto Clean 用 HotPlate 1，不是 Clean Kit";
        else if (in.acTray != kAcCKPosCleanKit)
            why = Fmt("Clean Kit：Clean pad 不在 Clean Kit（iAutoClean_Tray=%d%s）", in.acTray, in.acTray == kAcCKPosHP2 ? "＝HotPlate 2" : "");
        else if (a.cols <= 0 || a.rows <= 0) why = "Clean Kit：格數是 0（iAutoClean_X／YDivision）";
        a.installed = why.empty();
        a.zKind = "place";
        if (!in.use8Picker && !in.e43 && nr == 0) {                             // f4: golden MoveInOutArmZToKitPickPlace protection
            a.zDownAllowed = false;
            a.zDownWhy = "Clean Kit：golden MoveInOutArmZToKitPickPlace（AutoClean.cpp:1199-1216）bUse8Picker=false 時只讓 B 排（row 1）的吸嘴降，這支吸嘴在 A 排";
        }
    } else if (id == "OutShuttle1" || id == "OutShuttle2") {
        const int s = (id == "OutShuttle2") ? 1 : 0;
        ArmCellAxis ax; int tgt = 0; std::string what;
        D2Of(in, false, s, ax, tgt, what);
        a.cols = in.shtCol; a.rows = in.shtRow;
        if (!ax.present || !ax.enable)
            why = Fmt("%s：golden 判斷 shuttle 在不在 Out Arm 側看的 %s 沒有裝（Mot_Table %s）", d.label,
                      ax.alias.empty() ? "那一軸" : ax.alias.c_str(), ax.present ? "Enable=0" : "沒有這一列");
        else if (in.shuttleMode == 1 && in.shuttleSel != s)
            why = Fmt("%s：工作檔只用 Shuttle %d（HandlerCondition.Data Shuttle Mode=1、Shuttle1 Cancel=%d）", d.label, in.shuttleSel + 1, in.shuttleSel);
        else if (in.shtCol <= 0 || in.shtRow <= 0)
            why = Fmt("%s：Shuttle 的格數是 0（TestSocket.iShtCol／iShtRow）", d.label);
        a.installed = why.empty();
        a.zKind = "pick";
        a.zDownWhy = "Shuttle 區：只有 shuttle 停在手臂側（READY 而且在教導點的 gap 內）才降 Z，X／Y 照走（RULINGS_20261002 第 18 條 D2）";
    } else if (id.compare(0, 4, "Auto") == 0) {
        const int t = kAcAuto1 + (id[4] - '1');
        a.cols = in.autoForm[t].xDivision; a.rows = in.autoForm[t].yDivision;
        if (in.trayType[t] != kAcTrayAuto)
            why = Fmt("%s：這台沒有這一盤（Prod.iTrayType[eAuto%c]=%d，不是 Auto；Gerneral.ini AUTO_EMPTY_COLOR 決定裝幾個 Auto）", d.label, id[4], in.trayType[t]);
        else if (a.cols <= 0 || a.rows <= 0) why = Fmt("%s：料盤格數是 0", d.label);
        a.installed = why.empty();
        a.zKind = "place";
    } else if (id.compare(0, 3, "Fix") == 0) {
        const int k = id[3] - '1';
        const int t = kAcFix1 + k;
        a.cols = in.autoForm[t].xDivision; a.rows = in.autoForm[t].yDivision;
        if (k >= 3 && in.fixRightIsFix3) a.label = Fmt("Fix %d（＝Fix %d 下半）", k + 1, k - 2);
        if (in.trayType[t] != kAcTrayFix)
            why = Fmt("%s：這台沒有這一盤（Prod.iTrayType[eFix%d]=%d，不是 Fix；工作檔 Use Fix%d Tray）", d.label, k + 1, in.trayType[t], k + 1);
        else if (a.cols <= 0 || a.rows <= 0) why = Fmt("%s：料盤格數是 0", d.label);
        a.installed = why.empty();
        a.zKind = "place";
    } else if (id == "BinBox") {
        a.cols = 1; a.rows = 1;
        a.installed = in.trayType[kAcBulkBox] == kAcTrayBox;
        if (!a.installed) why = "Bin Box：這台沒有裝（Prod.iTrayType[eBulkBox] 不是 Box）";
        a.zDownAllowed = false;
        a.zDownWhy = "Bin Box：golden Prod.ZPlace[eBulkBox] 從來沒被寫過（cinitial.cpp 只寫到 eFix12），降 Z 會到 0";
        a.zKind = "";
    } else if (id == "Magazine") {
        bool mag = false;
        for (int t = kAcMag1; t <= kAcMag14; ++t) if (in.trayType[t] == kAcTrayMag) mag = true;
        a.cols = 0; a.rows = 0;
        a.installed = mag;
        if (!mag) why = "Magazine：這台沒有裝（AUTO3_IS_MAGAZINE=0）";
        a.zDownAllowed = false;
        a.zDownWhy = "Magazine：golden Prod.ZPlace[eMag*] 從來沒被寫過，降 Z 會到 0；而且 Magazine 沒有自己的 X／Y（放到 Auto3 或 buffer）";
        a.zKind = "";
    }
    if (!a.installed) a.why = why;
    else if (!a.v1)   a.why = std::string(d.label) + "：" + kV1Why;
    else if (!armWhy.empty()) a.why = armWhy;
    a.usable = a.installed && a.v1 && armWhy.empty();
    return a;
}

// ---- the formulas (single nozzle r, c) ----
int ZSub(const ArmCellArmInputs& a, int r, int c) { return (r >= 0 && r < 2 && c >= 0 && c < 8) ? a.zHeightSub[r][c] : 0; }
bool IsBase(const ArmCellArmInputs& a, int r, int c) { return r == a.yBase && c == a.xBase; }

// golden InArmAddBlockPitch iXY==2 (ainarm9045.cpp:7682-7697; reads the live LoadForm, only with block mode + bP06)
void InBlockY(const ArmCellInputs& in, int& y, int row)
{
    if (!(in.trayBlockMode && in.p06)) return;
    const ArmCellForm& f = in.loadForm;
    y = Tr((double)y - (double)ArmCellFloatQuotient((double)row, (double)f.yDivision) * f.blockPitchY);
    y = Tr((double)y + (double)ArmCellFloatQuotient((double)row, (double)f.yDivision) * (f.yPitch * f.yDivision));
}

// golden OutArmAddBlockPitch (aoutarm.cpp:3304-3314; AutoForm[0] whatever the tray -- q3)
void OutBlock(const ArmCellInputs& in, int& x, int& y, int row, int col)
{
    if (!(in.trayBlockMode && in.p06)) return;
    const ArmCellForm& f = in.autoForm[0];
    x = Tr((double)x + (double)ArmCellFloatQuotient((double)col, (double)f.xDivision) * f.blockPitchX);
    x = Tr((double)x - (double)ArmCellFloatQuotient((double)col, (double)f.xDivision) * (f.xPitch * f.xDivision));
    y = Tr((double)y - (double)ArmCellFloatQuotient((double)row, (double)f.yDivision) * f.blockPitchY);
    y = Tr((double)y + (double)ArmCellFloatQuotient((double)row, (double)f.yDivision) * (f.yPitch * f.yDivision));
}

// golden TransferHotPlateRatio (ainarm2.cpp:5909-6113), the plate passed explicitly (golden reads iPlacePlate[0]).
// plate 0 = HotPlate 2 (scale index [1]), 1 = HotPlate 1 (index [0]). AOA branch: gated in V906 (GATE k2ai2-G1) = identity.
void HotPlateRatio(const ArmCellInputs& in, int plate, int baseX, int baseY, int& x, int& y)
{
    double fi = 0.0;
    const int iOldX = x, iOldY = y, iRet = 200;
    if (in.aoa) return;
    const int k = (plate == 0) ? 1 : 0;
    if (in.triTempHot) {
        if (in.e30_1Hot && in.workTemperBase >= 26) {
            fi = x - baseX; if (in.e30) fi = fi * in.hpXScaleHot[k]; fi = fi * in.hotPlateExpansion; x = Tr(fi + baseX);
            fi = y - baseY; if (in.e30) fi = fi * in.hpYScaleHot[k]; fi = fi * in.hotPlateExpansion; y = Tr(fi + baseY);
        }
        if (in.e30_2Cold && in.workTemperBase < 26) {
            fi = x - baseX; if (in.e30) fi = fi * in.hpXScaleCold[k]; fi = fi * in.hotPlateExpansion; x = Tr(fi + baseX);
            fi = y - baseY; if (in.e30) fi = fi * in.hpYScaleCold[k]; fi = fi * in.hotPlateExpansion; y = Tr(fi + baseY);
        }
    } else {
        fi = x - baseX;
        if (in.setupFileScale) fi *= in.hpXScaleFile[k];
        else if (in.hotModeUseDiffScale && in.e30_1Hot) fi = fi * in.hpXScaleHot[k];
        else { if (in.e30) fi = fi * in.hpXScale[k]; }
        fi = fi * in.hotPlateExpansion;
        x = Tr(fi + baseX);
        fi = y - baseY;
        if (in.setupFileScale) fi *= in.hpYScaleFile[k];
        else if (in.hotModeUseDiffScale && in.e30_1Hot) fi = fi * in.hpYScaleHot[k];
        else { if (in.e30) fi = fi * in.hpYScale[k]; if (plate == 0) fi = fi * in.hotPlateExpansion; }   // f2: Plate2's Y expands only here
        if (plate != 0) fi = fi * in.hotPlateExpansion;                         // Plate1's Y after the if / else, like X
        y = Tr(fi + baseY);
    }
    if (std::abs(x - iOldX) > iRet) x = iOldX;
    if (std::abs(y - iOldY) > iRet) x = iOldX;                                  // q4: golden writes *iXPos here (sic)
}

// golden TransferInShuttleRatio (ainarm2.cpp:6185-6316), base = Prod.XInArm_ShuttleN_Place[iInArmYBase][iInArmXBase].
void InShuttleRatio(const ArmCellInputs& in, int s, int baseX, int baseY, int& x, int& y)
{
    if (in.aoa) return;                                                         // port GATE (W7e-G1): identity
    double fi = 0.0;
    if (in.triTempHot) {
        if (in.e32_1Hot && in.workTemperBase >= 26) {
            fi = x - baseX; fi *= in.inShtXScaleHot[s]; x = Tr(fi + baseX);
            fi = y - baseY; fi *= in.inShtYScaleHot[s]; y = Tr(fi + baseY);
        }
        if (in.e32_2Cold && in.workTemperBase < 26) {
            fi = x - baseX; fi *= in.inShtXScaleCold[s]; x = Tr(fi + baseX);
            fi = y - baseY; fi *= in.inShtYScaleCold[s]; y = Tr(fi + baseY);
        }
    } else if (in.setupFileScale) {
        fi = x - baseX; fi *= in.inShtXScaleFile[s]; x = Tr(fi + baseX);
        fi = y - baseY; fi *= in.inShtYScaleFile[s]; y = Tr(fi + baseY);
    } else if (in.e32) {
        const bool hot = in.hotModeUseDiffScale && in.e32_1Hot && in.lastSetHot;
        fi = x - baseX; fi *= hot ? in.inShtXScaleHot[s] : in.inShtXScale[s]; x = Tr(fi + baseX);
        fi = y - baseY; fi *= hot ? in.inShtYScaleHot[s] : in.inShtYScale[s]; y = Tr(fi + baseY);
    }
}

// golden TransferAutoRatio (aoutarm.cpp:3328-3372), base = Prod.XStart[t][iOutArmYBase][iOutArmXBase].
void AutoRatio(const ArmCellInputs& in, int t, int baseX, int baseY, int& x, int& y)
{
    if (in.aoa) return;                                                         // v1 refuses AOA before this (golden CheckOutArmXYScaleByAutoTeach)
    double fi = 0.0;
    if (in.triTempHot) {
        if (in.e31_1Hot && in.workTemperBase >= 26) {
            fi = x - baseX; fi *= in.trayXScaleHot[t]; x = Tr(fi + baseX);
            fi = y - baseY; fi *= in.trayYScaleHot[t]; y = Tr(fi + baseY);
        }
        if (in.e31_2Cold && in.workTemperBase < 26) {
            fi = x - baseX; fi *= in.trayXScaleCold[t]; x = Tr(fi + baseX);
            fi = y - baseY; fi *= in.trayYScaleCold[t]; y = Tr(fi + baseY);
        }
    } else if (in.e31) {
        fi = x - baseX; fi *= in.trayXScale[t]; x = Tr(fi + baseX);
        fi = y - baseY; fi *= in.trayYScale[t]; y = Tr(fi + baseY);
    }
}

// golden TransferOutShuttleRatio (aoutarm.cpp:3374-3436), base = Prod.XOutArm_ShuttleN_Pick[iOutArmYBase][iOutArmXBase].
void OutShuttleRatio(const ArmCellInputs& in, int s, int baseX, int baseY, int& x, int& y)
{
    if (in.aoa) return;                                                         // v1 refuses AOA before this
    double fi = 0.0;
    if (in.setupFileScale) {
        fi = x - baseX; fi *= in.outShtXScaleFile[s]; x = Tr(fi + baseX);
        fi = y - baseY; fi *= in.outShtXScaleFile[s]; y = Tr(fi + baseY);       // f3: golden scales Y with the X factor
    } else if (in.e32) {
        fi = x - baseX; fi *= in.inShtXScale[s]; x = Tr(fi + baseX);            // f3: the IN shuttle scales
        fi = y - baseY; fi *= in.inShtYScale[s]; y = Tr(fi + baseY);
    }
}

// golden GetShtRowColStartPos + GetShtRowColPos (port ainarm9045.cpp:630-720) for one shuttle target
void ShtRowColPos(const ArmCellInputs& in, bool inArm, int s, int baseX, int baseY, int col, int row, int& X, int& Y)
{
    int iRow = in.shtRow, iCol = in.shtCol;
    int hx = inArm ? in.arm[0].shtXCenter : in.arm[1].shtXCenter;
    int hy = inArm ? in.arm[0].shtYCenter : in.arm[1].shtYCenter;
    if (in.twoArm32Site) iRow = iRow / 2;
    hx += baseX;
    hy += baseY;
    X = ArmCellShtStartPos(iCol, hx, (int)(in.siteXPitch));
    Y = ArmCellShtStartPos(iRow, hy, (int)(in.siteYPitch) * (-1));              // golden: 反向補償*(-1)
    X += (int)(in.siteXPitch) * col;
    Y -= (int)(in.siteYPitch) * row;
    const bool inShuttle1 = inArm && s == 0;                                    // golden `iTarget==MInShuttle1`
    if (in.ns7000Kit) {
        const int iShiftY = Tr((in.siteYPitch == 0) ? (double)(6000 / 2) : (in.siteYPitch / 2));
        if (inShuttle1) Y += iShiftY; else Y -= iShiftY;                        // f3: every other shuttle subtracts
    } else if (in.twoArm32Site) {
        if (inShuttle1) Y += 1000; else Y -= 1000;
    }
}

// golden SetTechDataToProd_InArm HotPlate base (port cinitial.cpp:14765-14839 iHPXn/iHPYn, :14901-14963 the base slot)
void InPlateBase(const ArmCellInputs& in, int plate, int& bx, int& by)
{
    const ArmCellForm& hf = in.hotPlateForm;
    const int techX = (plate == 0) ? in.inPlate2X : in.inPlate1X;
    const int techY = (plate == 0) ? in.inPlate2Y : in.inPlate1Y;
    const ArmCellOffset& o = (plate == 0) ? in.inOfsHP2 : in.inOfsHP1;
    int hpX = 0, hpY = 0;
    if (in.hotPlatePos0 && !in.ht1032) {
        hpX = techX;
        hpY = techY;
    } else {
        if (in.ht1032 && in.usePickerCount == kAcEp16)
            hpX = Tr((double)(techX + 1000) - hf.xStart * 2 - (hf.xDivision - 1) * hf.xPitch + 9000);
        else
            hpX = Tr((double)(techX + 9000) - hf.xStart * 2 - (hf.xDivision - 1) * hf.xPitch + 9000);
        hpY = Tr((double)(techY - 1000) + hf.yStart * 2 + (hf.yDivision - 1) * hf.yPitch - 1000);
    }
    bx = Tr((double)hpX + hf.xStart - 9000 + o.x);
    if (in.hotPlateMove1CM && !hf.useWideHotplate) {
        if (in.hotPlatePos0) bx = Tr((double)hpX + hf.xStart - 9000 + 1000 + o.x);
        else                 bx = Tr((double)hpX + hf.xStart - 9000 - 1000 + o.x);
    }
    by = Tr((double)hpY - hf.yStart + 1000 + o.y);
}

// golden SetTechDataToProd_InArm Plate place Z of nozzle (r, c) (port cinitial.cpp:14977-15062)
bool InPlatePlaceZ(const ArmCellInputs& in, int plate, int r, int c, int& z, std::string& why)
{
    const ArmCellArmInputs& a = in.arm[0];
    const int iTemp = ZSub(a, r, c);
    const double Z2 = (double)in.inPlateZ;
    const ArmCellOffset& hp1 = in.inOfsHP1;
    const ArmCellOffset& hpN = (plate == 0) ? in.inOfsHP2 : in.inOfsHP1;
    if (in.e33) {
        if (in.e88) {
            if (c >= 4) { why = "HotPlate：E33＋E88 時 golden 讀 Tech.iInArmZHeightSub[i][j]（[2][4]）而這支吸嘴在第 5 欄以後 —— golden 會讀到表外，不算"; return false; }
            const double sub = (double)(r < 2 ? a.zHeightSub[r][c] : 0);   // Tech.iInArmZHeightSub[i][j] (c < 4: the same cell as iTemp)
            z = Tr(Z2 + sub + in.inOfsLoader.PlaceRC(r, c) + (in.e34 ? hp1.place : hpN.place));
        } else {
            z = Tr(Z2 + iTemp + 200 + in.inOfsLoader.PlaceRC(r, c) + (in.e34 ? hp1.place : hpN.place));
        }
    } else {
        z = Tr(Z2 + iTemp + 200 + hpN.PlaceRC(r, c) + (in.e34 ? hp1.place : hpN.place));
    }
    if (in.hotPlateMove1CM && !in.hotPlateForm.useWideHotplate) z += 300;
    return true;
}

// golden SetTechDataToProd_InArm Auto Clean base + place Z (port cinitial.cpp:15079-15176)
void CleanBase(const ArmCellInputs& in, int& bx, int& by)
{
    const ArmCellOffset& oac = in.e43 ? in.inOfsHP1 : in.inOfsAutoClean;        // iAcOfs
    if (in.e43) {
        int p1x = 0, p1y = 0;
        InPlateBase(in, 1, p1x, p1y);                                           // Prod.X/YInArm_Plate1_Pick[base]
        bx = Tr((double)p1x + in.acXStart - in.hotPlateForm.xStart + oac.x);
        by = Tr((double)p1y + in.acYStart - in.hotPlateForm.yStart + oac.y);
    } else if (in.newCleanModeKit) {
        bx = Tr((double)in.inACX + in.acXStart + oac.x + 50);
        by = Tr((double)in.inACY - in.acYStart + oac.y - 90);
        if (in.acUseNSKit) bx = bx + 1500;
    } else {
        bx = Tr((double)in.inACX + in.acXStart - 18000 + oac.x);
        by = Tr((double)in.inACY - in.acYStart - 2250 + oac.y);
        if (in.acUseTray) { bx = bx + 8100; by = by - 400; }
        else if (in.acUseNSKit) bx = bx + 1500;
        if (in.cleanKit16BdBe) {
            bx = Tr((double)in.inACX + in.acXStart + 1680 + in.inOfsAutoClean.x);
            by = Tr((double)in.inACY - in.acYStart - 6000 + in.inOfsAutoClean.y);
        }
    }
}

bool BuildIn(const ArmCellInputs& in, const std::string& id, ArmCellPlan& p, std::string& why)
{
    const ArmCellArmInputs& a = in.arm[0];
    const int r = p.nr, c = p.nc, col = p.col, row = p.row;
    if (id == "Loader") {
        if (in.loaderRatioUngated) {
            why = "Loader：TransferLoaderRatio 已經解閘（MachineType.h W906_TRANSFER_LOADER_RATIO），但 Arm Cell 的複本還沒寫 —— 不算（NB2 規格 §4：三處一起解）";
            return false;
        }
        const ArmCellForm lf = LoaderForm(in);
        const ArmCellOffset& o = in.inOfsLoader;
        p.teachX = in.inLoadX; p.teachY = in.inLoadY; p.teachZ = in.inLoadZ;
        p.baseX = Tr((double)in.inLoadX + lf.xStart + lf.blockXStart - kAcKitPitch + o.x + in.trayKitStartX);
        p.baseY = Tr((double)in.inLoadY - lf.yStart - lf.blockYStart + kAcKitPitch + o.y - in.trayKitStartY);
        p.hasProd = true; p.prodBaseX = in.prodInLoadX; p.prodBaseY = in.prodInLoadY;
        const int xPitch = Tr(in.loadForm.xPitch), yPitch = Tr(in.loadForm.yPitch);   // Prod.LoadForm.iXPitch / iYPitch (cinitial.cpp:15597)
        int x = Tr((double)p.baseX + (double)(col * xPitch));                     // GetInArmToLoaderPosition_Single :8403-8406 (q3: X block lost)
        int y = p.baseY - row * yPitch;
        InBlockY(in, y, row);                                                   // :8408-8409
        x = Tr((double)x + o.ArmX(r, c));                                       // MoveInArmXYToLoader_9045 :9216-9228 (f1)
        y = Tr((double)y + o.ArmY(r, c));
        p.cellX = x; p.cellY = y;                                               // TransferLoaderRatio gated (GATE k4-G4)
        p.xTarget = x; p.yTarget = y;
        int zb = Tr((double)in.inLoadZ + o.pickUp);                             // cinitial.cpp:14853
        if (in.trayThickAdjustZ && (in.e70 || in.userDefFile0Thick)) {
            if (in.loaderTrayType < 0 || in.loaderTrayType > 3) { why = Fmt("Loader：TrayForm.Loader.iTrayType=%d 超出 UserDefForm_File[4]", in.loaderTrayType); return false; }
            double dbTrayThick = in.userDefFileZDepth[in.loaderTrayType] * 100;
            if (dbTrayThick < 600) dbTrayThick = 600.0;
            zb = Tr((double)zb + (dbTrayThick - 635));
        }
        if (in.e88) zb -= 200;
        p.zTarget = IsBase(a, r, c) ? zb : Tr((double)(zb + ZSub(a, r, c)) + o.PickUpRC(r, c));
        if (!o.oneByOne && (o.x != 0.0 || o.y != 0.0))
            p.notes.push_back("Loader 的 InArmOffSet 沒開 OneByOne：golden GetArmX／GetArmY 回 −X／−Y，單吸嘴路徑把表的 X／Y 偏移又扣回去（golden 原樣）");
        return true;
    }
    if (id == "HotPlate1" || id == "HotPlate2") {
        const int plate = (id == "HotPlate2") ? 0 : 1;
        InPlateBase(in, plate, p.baseX, p.baseY);
        p.teachX = plate == 0 ? in.inPlate2X : in.inPlate1X; p.teachY = plate == 0 ? in.inPlate2Y : in.inPlate1Y; p.teachZ = in.inPlateZ;
        p.hasProd = true; p.prodBaseX = in.prodInPlateX[plate]; p.prodBaseY = in.prodInPlateY[plate];
        const int xPitch = Tr(in.hotPlateForm.xPitch), yPitch = Tr(in.hotPlateForm.yPitch);   // Prod.HotPlateForm[0].iXPitch / iYPitch
        int x = p.baseX + xPitch * col;                                         // MoveInArmXYToHotPlatePlace :1854-1863
        int y = p.baseY - yPitch * row;
        p.cellX = x; p.cellY = y;
        HotPlateRatio(in, plate, p.baseX, p.baseY, x, y);                       // :1967 (place: iPlacePlate[0] = plate)
        p.xTarget = x; p.yTarget = y;
        return InPlatePlaceZ(in, plate, r, c, p.zTarget, why);
    }
    if (id == "InShuttle1" || id == "InShuttle2") {
        const int s = (id == "InShuttle2") ? 1 : 0;
        p.teachX = in.inShtX[s]; p.teachY = in.inShtY[s]; p.teachZ = in.inShtZ;
        p.baseX = in.inShtX[s] - in.shuttleTempPos[s][0];                       // cinitial.cpp:15217 / :15229
        p.baseY = in.inShtY[s];
        p.hasProd = true; p.prodBaseX = in.prodInShtX[s]; p.prodBaseY = in.prodInShtY[s];
        int x = 0, y = 0;
        ShtRowColPos(in, true, s, p.baseX, p.baseY, col, row, x, y);            // CheckXYPitch_All_1Pick :258
        const ArmCellOffset& o = in.inOfsInSh[s];                               // GetInArmToShuttleOffset_9045(iSht,0,0,false) = InOfsInSh1/2
        y = y + Tr(o.y);                                                        // GetInArmYToShuttleOffset_9045 returns int
        x = x + Tr(o.x);
        p.cellX = x; p.cellY = y;
        InShuttleRatio(in, s, p.baseX, p.baseY, x, y);                          // :262
        p.xTarget = x; p.yTarget = y;
        const ArmCellOffset& ob = (s == 0 || in.e34) ? in.inOfsInSh[0] : in.inOfsInSh[1];
        const int zb = Tr((double)in.inShtZ + ob.place);                        // cinitial.cpp:15180-15189
        const ArmCellOffset& orc = in.e33 ? in.inOfsLoader : in.inOfsInSh[s];
        p.zTarget = IsBase(a, r, c) ? zb : Tr((double)(zb + ZSub(a, r, c)) + orc.PlaceRC(r, c));
        p.d2 = true;
        D2Of(in, true, s, p.d2Axis, p.d2Target, p.d2What);
        return true;
    }
    if (id == "CleanKit") {
        CleanBase(in, p.baseX, p.baseY);
        p.teachX = in.inACX; p.teachY = in.inACY; p.teachZ = in.e43 ? in.inPlateZ : in.acTeachPickZ;
        p.hasProd = true; p.prodBaseX = in.prodInACX; p.prodBaseY = in.prodInACY;
        const int hx = Tr(in.acHotplateXOffset), hy = Tr(in.acHotplateYOffset);   // AutoClean InitialSet :3617-3618 (int globals)
        int x = Tr((double)p.baseX + 0.0 + in.acXPitch * col + hx);             // MoveInArmXYPickCleanKit :2243-2246 (D1: dMovePitchX term 0)
        int y = 0;
        if (!in.use8Picker) {
            y = Tr((double)p.baseY - row * in.acYPitch + in.armYPitch);         // :2258 (f4: every row; GetYPitchOfCleanKit = TestIF.iARM_Y_PITCH)
            p.notes.push_back(Fmt("Clean Kit：golden bUse8Picker=false ⇒ Y 加 iARM_Y_PITCH=%d（不分吸嘴列，AutoClean.cpp:2257-2258）", in.armYPitch));
        } else {
            y = Tr((double)p.baseY - row * in.acYPitch);                        // :2262 (the row-B pitch: D1)
        }
        y += hy;                                                                // :2264
        p.cellX = x; p.cellY = y;                                               // AOA refused (v1)
        p.xTarget = x; p.yTarget = y;
        const ArmCellOffset& oac = in.e43 ? in.inOfsHP1 : in.inOfsAutoClean;
        const int pb = in.e43 ? Tr((double)in.inPlateZ + 200 + oac.place) : Tr((double)in.acTeachPickZ + 200 + oac.place);
        const ArmCellOffset& orc = in.e33 ? in.inOfsLoader : oac;
        const int zp = IsBase(a, r, c) ? pb : Tr((double)(ZSub(a, r, c) + pb) + orc.PlaceRC(r, c));
        const int iArmPlaceTrayPos = in.acPadThickness + Tr(in.acHotplatePlaceOffset);   // InitialSet :3620 / :3623
        p.zTarget = zp + iArmPlaceTrayPos;                                      // MoveInOutArmZToKitPickPlace :1194-1195
        return true;
    }
    why = "不認得的 In Arm 區：" + id;
    return false;
}

void OutStart(const ArmCellInputs& in, int t, int& sx, int& sy, int& teachX, int& teachY)
{
    const ArmCellForm lf = LoaderForm(in);
    int k = 0;
    const ArmCellOffset* o = 0;
    int tx = 0, ty = 0, formT = t;
    if (t >= kAcAuto1 && t <= kAcAuto6) { k = t - kAcAuto1; tx = in.outAutoX[k]; ty = in.outAutoY[k]; o = &in.outOfsAuto[k]; }
    else {
        k = t - kAcFix1;
        if (k >= 3 && in.fixRightIsFix3) k -= 3;                               // cinitial.cpp:11867-11872: Fix4..6 = Fix_Place[0..2]
        tx = in.outFixX[k]; ty = in.outFixY[k]; o = &in.outOfsFix[k]; formT = kAcFix1 + k;
    }
    teachX = tx; teachY = ty;
    sx = Tr((double)tx + in.autoForm[formT].xStart + lf.blockXStart - kAcKitPitch + o->x + in.trayKitStartX);   // cinitial.cpp:11500 / :11577
    sy = Tr((double)ty - in.autoForm[formT].yStart - lf.blockYStart + kAcKitPitch + o->y - in.trayKitStartY);
}

bool BuildOut(const ArmCellInputs& in, const std::string& id, ArmCellPlan& p, std::string& why)
{
    const ArmCellArmInputs& a = in.arm[1];
    const int r = p.nr, c = p.nc, col = p.col, row = p.row;
    if (id == "OutShuttle1" || id == "OutShuttle2") {
        const int s = (id == "OutShuttle2") ? 1 : 0;
        p.teachX = in.outShtX[s]; p.teachY = in.outShtY[s]; p.teachZ = in.outShtPickZ2;
        p.baseX = in.outShtX[s] + in.shuttleTempPos[s][1];                      // cinitial.cpp:11440 / :11452
        p.baseY = in.outShtY[s];
        p.hasProd = true; p.prodBaseX = in.prodOutShtX[s]; p.prodBaseY = in.prodOutShtY[s];
        int x = 0, y = 0;
        ShtRowColPos(in, false, s, p.baseX, p.baseY, col, row, x, y);           // CheckOutArmXYPitch_All_1Picker :388
        const ArmCellOffset& o = in.outOfsOutSh[s];                             // iOffsetPos = OutOfsOutSh1/2
        y = y + Tr(o.y);
        x = x + Tr(o.x);
        p.cellX = x; p.cellY = y;
        OutShuttleRatio(in, s, p.baseX, p.baseY, x, y);                         // :391
        p.xTarget = x; p.yTarget = y;
        const int zb = Tr((double)in.outShtPickZ2 + o.pickUp);                  // cinitial.cpp:11466-11467
        const ArmCellOffset& orc = in.e33 ? in.outOfsAuto[0] : o;
        p.zTarget = IsBase(a, r, c) ? zb : Tr((double)(zb + ZSub(a, r, c)) + orc.PickUpRC(r, c));   // + iOutShtRetryCount*dRetryDown, retry 0
        p.d2 = true;
        D2Of(in, false, s, p.d2Axis, p.d2Target, p.d2What);
        return true;
    }
    const bool isAuto = id.compare(0, 4, "Auto") == 0, isFix = id.compare(0, 3, "Fix") == 0;
    if (!isAuto && !isFix) { why = "不認得的 Out Arm 區：" + id; return false; }
    const int t = isAuto ? kAcAuto1 + (id[4] - '1') : kAcFix1 + (id[3] - '1');
    int sx = 0, sy = 0;
    OutStart(in, t, sx, sy, p.teachX, p.teachY);
    p.baseX = sx; p.baseY = sy;
    p.hasProd = true; p.prodBaseX = in.prodOutStartX[t]; p.prodBaseY = in.prodOutStartY[t];
    const ArmCellForm& af = in.autoForm[t];
    int y = Tr((double)sy - row * af.yPitch);                                   // GetOutArmToUnLoaderPosition :4006
    int x = Tr((double)sx + col * af.xPitch);                                   // :4009 (ep1 / D1: no iVariablePara term)
    if (in.trayBlockMode) OutBlock(in, x, y, row, col);                         // :4013-4014
    x = Tr((double)sx + col * af.xPitch);                                       // :4028-4031 (q3: X recomputed)
    const ArmCellOffset& oa = isAuto ? in.outOfsAuto[t - kAcAuto1] : in.outOfsFix[t - kAcFix1];
    x = Tr((double)x + oa.ArmX(r, c));                                          // :4033-4041 (f1)
    y = Tr((double)y + oa.ArmY(r, c));
    if (in.trayBlockMode) OutBlock(in, x, y, row, col);                         // :4043-4044 (q3: Y twice)
    p.cellX = x; p.cellY = y;
    AutoRatio(in, t, sx, sy, x, y);                                             // :4046
    p.xTarget = x; p.yTarget = y;
    int zb = 0;
    int thickT = t;
    if (isAuto) {
        const int k = t - kAcAuto1;
        p.teachZ = in.outPlaceZ2;
        zb = Tr((double)in.outPlaceZ2 + (in.e34 ? in.outOfsAuto[0] : in.outOfsAuto[k]).place);   // cinitial.cpp:11536-11539
        const ArmCellOffset& orc = in.e33 ? in.outOfsAuto[0] : in.outOfsAuto[k];
        if (in.trayThickAdjustZ && (in.e70 || in.userDefFile0Thick)) {
            const int tt = in.autoTrayType[thickT];
            if (tt < 0 || tt > 3) { why = Fmt("%s：TrayForm.Auto[%d].iTrayType=%d 超出 UserDefForm_File[4]", id.c_str(), t, tt); return false; }
            double dbTrayThick = in.userDefFileZDepth[tt] * 100;
            if (dbTrayThick < 600) dbTrayThick = 600.0;
            zb = Tr((double)zb + (dbTrayThick - 635));
        }
        p.zTarget = IsBase(a, r, c) ? zb : Tr((double)(zb + ZSub(a, r, c)) + orc.PlaceRC(r, c));
        if (!oa.oneByOne && (oa.x != 0.0 || oa.y != 0.0))
            p.notes.push_back(id + " 的 OutArmOffSet 沒開 OneByOne：golden GetArmX／GetArmY 回 −X／−Y，單吸嘴路徑把表的 X／Y 偏移又扣回去（golden 原樣）");
        return true;
    }
    int k = t - kAcFix1;
    if (k >= 3 && in.fixRightIsFix3) k -= 3;                                   // ZPlace[eFix4..6] = ZOutArm_Fix_Place[0..2]
    const int ft = kAcFix1 + k;
    const ArmCellOffset& ok = in.outOfsFix[k];
    const bool special = (in.fix3FullPlace == 1 && ft == kAcFix2) || (in.fix3FullPlace == 2 && ft == kAcFix3);
    if (special) zb = Tr((double)in.outPlaceFix2Z1 + ok.place);                // cinitial.cpp:11626-11635 / :11647-11656
    else if (in.e34) zb = Tr((double)in.outPlaceFixZ1 + in.outOfsFix[0].place);
    else zb = Tr((double)in.outPlaceFixZ1 + ok.place);
    p.teachZ = special ? in.outPlaceFix2Z1 : in.outPlaceFixZ1;
    thickT = ft;
    if (in.trayThickAdjustZ && (in.e70 || in.userDefFile0Thick)) {
        const int tt = in.autoTrayType[thickT];
        if (tt < 0 || tt > 3) { why = Fmt("%s：TrayForm.Auto[%d].iTrayType=%d 超出 UserDefForm_File[4]", id.c_str(), thickT, tt); return false; }
        double dbTrayThick = in.userDefFileZDepth[tt] * 100;
        if (dbTrayThick < 600) dbTrayThick = 600.0;
        zb = Tr((double)zb + (dbTrayThick - 635));
    }
    const ArmCellOffset& orc = in.e33 ? in.outOfsAuto[0] : ok;
    p.zTarget = IsBase(a, r, c) ? zb : Tr((double)(zb + ZSub(a, r, c)) + orc.PlaceRC(r, c));
    if (!oa.oneByOne && (oa.x != 0.0 || oa.y != 0.0))
        p.notes.push_back(id + " 的 OutArmOffSet 沒開 OneByOne：golden GetArmX／GetArmY 回 −X／−Y，單吸嘴路徑把表的 X／Y 偏移又扣回去（golden 原樣）");
    return true;
}

std::string D1Note(const ArmCellInputs& in, int arm, int r, int c)
{
    if (in.usePickerCount == kAcEp1) return std::string();
    const ArmCellArmInputs& a = in.arm[arm];
    return Fmt("Gerneral.ini USE_PICKER_COUNT=%d（golden 的吸嘴頭 %d×%d），這支手臂只有 %s 一支 Enable 的 Z：照硬體當 1 吸嘴算，"
               "不加多吸嘴的 X／Y 偏移（RULINGS_20261002 第 18 條 D1）", in.usePickerCount, a.motRow, a.motCol,
               (r >= 0 && c >= 0) ? a.z[r][c].alias.c_str() : "?");
}

}  // namespace

void ArmCellBuildCatalog(const ArmCellInputs& in, ArmCellCatalog& out)
{
    out = ArmCellCatalog();
    out.zSafePos = in.zSafePos;
    int nr[2] = { -1, -1 };
    for (int arm = 0; arm < 2; ++arm) {
        const ArmCellArmInputs& a = in.arm[arm];
        int r = -1, c = -1;
        out.armWhy[arm] = ArmCheck(in, arm, r, c);
        out.x[arm] = a.x; out.y[arm] = a.y;
        for (int i = 0; i < a.motRow && i < 2; ++i)
            for (int j = 0; j < a.motCol && j < 8; ++j)
                if (a.z[i][j].present && a.z[i][j].enable) {
                    ArmCellNozzleInfo n; n.i = i; n.j = j; n.z = a.z[i][j];
                    out.nozzles[arm].push_back(n);
                }
        int rr = -1, cc = -1;
        if (EnabledZ(a, rr, cc) == 1) nr[arm] = rr;
        if (out.armWhy[arm].empty()) out.note[arm] = D1Note(in, arm, r, c);
    }
    for (int i = 0; i < kAreaCount; ++i)
        out.areas.push_back(EvalArea(in, kAreas[i], out.armWhy[kAreas[i].arm], nr[kAreas[i].arm]));
}

bool ArmCellBuildPlan(const ArmCellInputs& in, const ArmCellRequest& q, ArmCellPlan& p, std::string& why)
{
    p = ArmCellPlan();
    why.clear();
    int arm = -1;
    if (q.arm == "in") arm = 0;
    else if (q.arm == "out") arm = 1;
    else { why = "Arm Cell：arm 要是 in 或 out（收到 '" + q.arm + "'）"; return false; }
    const AreaDef* d = FindArea(q.area);
    if (d == 0) { why = "Arm Cell：不認得的區 '" + q.area + "'"; return false; }
    if (d->arm != arm) { why = std::string("Arm Cell：") + d->label + " 不是 " + (arm == 0 ? "In" : "Out") + " Arm 的區"; return false; }
    int r = -1, c = -1;
    const std::string armWhy = ArmCheck(in, arm, r, c);
    int er = -1, ec = -1;
    const int nEnabled = EnabledZ(in.arm[arm], er, ec);
    const ArmCellArea area = EvalArea(in, *d, armWhy, nEnabled == 1 ? er : -1);
    if (!area.usable) { why = area.why.empty() ? std::string(d->label) + "：不能用" : area.why; return false; }
    const ArmCellArmInputs& a = in.arm[arm];
    if (!q.nozzle.empty() && q.nozzle != a.z[r][c].alias) {
        why = Fmt("Arm Cell：吸嘴 '%s' 不是這支手臂 Enable 的 Z（只有 %s）", q.nozzle.c_str(), a.z[r][c].alias.c_str());
        return false;
    }
    if (q.col < 0 || q.col >= area.cols || q.row < 0 || q.row >= area.rows) {
        why = Fmt("%s：格子 (%d, %d) 超出範圍（欄 1..%d、列 1..%d）", d->label, q.col + 1, q.row + 1, area.cols, area.rows);
        return false;
    }
    p.arm = q.arm; p.area = q.area; p.label = area.label; p.zKind = area.zKind;
    p.armIndex = arm; p.col = q.col; p.row = q.row; p.nr = r; p.nc = c;
    p.x = a.x; p.y = a.y; p.z = a.z[r][c]; p.nozzle = p.z.alias;
    for (int i = 0; i < a.motRow && i < 2; ++i)
        for (int j = 0; j < a.motCol && j < 8; ++j)
            if (a.z[i][j].present && a.z[i][j].enable) p.zLift.push_back(a.z[i][j]);
    p.zSafe = in.zSafePos;
    p.zDownAllowed = area.zDownAllowed;
    p.zDownWhy = area.zDownWhy;
    p.d1Note = D1Note(in, arm, r, c);
    const bool ok = (arm == 0) ? BuildIn(in, q.area, p, why) : BuildOut(in, q.area, p, why);
    if (!ok) return false;
    if (!p.d1Note.empty()) p.notes.insert(p.notes.begin(), p.d1Note);
    return true;
}

}  // namespace ht9045
