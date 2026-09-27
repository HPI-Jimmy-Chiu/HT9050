// ===========================================================================
//  FileRW/BinSelect_C.cpp -- 結構 BinSelect 的讀寫檔（C 路，golden TfBinSel → <recipe>\Binasgn*.Data）。
//
//  Steven 20260925.  取代 A 形狀 FileRW/BinSelect.cpp（tools/gen_formbridge.py 產物，整合者退役後本檔改名 BinSelect.cpp）。
//  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（BinSelect）。
//
//  Steven 指定的做法（20260925）：TfBinSel 的資料不在表單元件上，而在結構裡 ——
//    * MyBinPanel[tag]（golden vector<TMyBinPanel*>，cBinSel.cpp:974-980 建 7 個；移植樹 TMyBinPanelData）的資料欄位；
//    * 每種 bin 類型一組 TStringList sXxx[tag]（golden ctor :986-1078）。
//  golden 讀：ReadFile(:1121) → ReadFunctionData(tag)(:4855) 把檔案讀進 sXxx[tag]、MyBinPanel[tag]、BinSelect[tag]
//            → mtTrayNameSetColor(tag)(:4081) → InitDataToEdit(tag)(:4190)。
//  golden 存：spbSaveClick(:2252) → SaveOther(:2378)（寫 I/F Error、舊格式 Category、每盤 Pass/Fail／Link／ART／Cate R
//            ← MyBinPanel[tag]） → SaveFunctionData(tag)(:5716)（新格式 ← sXxx[tag]->CommaText）。
//  golden 的編輯（滑鼠事件 mtBinSelectMouseDown/Up、mtTrayNameMouseDown、SetBinTray :3982…）一律改 MyBinPanel[tag]，
//  最後都呼叫 InitDataToEdit(tag)（:4018／:4187）由 MyBinPanel 重算 sXxx[tag]（:4733-4823）。所以：
//    * 頁面不模擬 TMyBinPanel 的 GUI edit；editlist.get 送「7 組 × 各 TStringList ＋ MyBinPanel[tag] 的資料欄位」。
//    * editlist.save 收作用中那一組的 MyBinPanel 欄位 → 套進 fBinSel->MyBinPanel[tag] → golden InitDataToEdit(tag)
//      （＝最後一次編輯的收尾）→ golden spbSaveClick。頁面送的 TStringList 不直接套（TfBinSel 沒有任何地方直接編輯
//      它們：25 組由 InitDataToEdit 從 MyBinPanel 算、sBinTraySetT3PosName 由 SaveFunctionData 內的
//      TransferBinTrayStrToName 算、sBinType 由 ReadFile 算、sMagazineSetup／sAOIBinTraySetting 是讀進來原樣寫回
//      （AOI 由另一張表單 fBinAOISel 存））；存完拿來和 golden 實際寫的值比對，不同的列在 ack.bin.listsDiffer。
//
//  物件共用：sXxx[tag]／MyBinPanel[tag] 是移植樹 fBinSel 的那一份（開機讀檔、Command.cpp、SECS、fShowBinSelect 讀的同一份）；
//  表單層真元件在開機時收養 fBinSel 的同名同型別物件（BS_AdoptPortWidgets）。
//  golden 表單方法由 tools/gen_editlist.py 轉成 FileRW/BinSelect.gen.inc（設定 tools/editlist/BinSelect.py）。
//
//  AI(W906-FRW-S99) 20260926: RULINGS_20260926 S99 —— Normal／Prime 兩顆鈕（golden spbNormalClick :2800／spbPrimeClick :2822
//    → WritePrimeDara :6107，寫 <配方>\Binasgn.Data [BinModel] bPrime）。通道：editlist.save tag=BinSelect，value 多帶
//    "op":"spbNormalClick"|"spbPrimeClick"（見 FileRW_BinSelect_PrimeClick 上方）—— tools/wb_serve.cpp 不用改。
// ===========================================================================
#include "FileRW/BinSelect.gen.inc"

// 產生檔的 #define 只給 golden 方法用；本檔手寫碼一律直接寫 fBinSel->…
#undef MyBinPanel
#undef sBinDoubleContact
#undef sBinConsFail
#undef sBinEnableFail
#undef sBinFailPercent
#undef sBinFailIgnore
#undef sBinCountEnable
#undef sBinCountIgnore
#undef sBinCountNumber
#undef sSpecialBinByArm
#undef sSpecialBinCountByArm
#undef sSpecialBinBySocket
#undef sSpecialBinCountBySocket
#undef sLowYield
#undef sArmYield
#undef sSiteYield
#undef sBinTraySetT3Pos
#undef sBinTraySetT3PosName
#undef sBinType
#undef sBySiteClean
#undef sByBinClean
#undef sT3TrayType
#undef sT6Retest
#undef sT3CateR
#undef sSpecBinBySiteCompareEnable
#undef sSpecBinBySiteCompareIgnore
#undef sSpecBinBySiteComparePercent
#undef sSpecBinByArmPerSiteCompareEnable
#undef sSpecBinByArmPerSiteCompareIgnore
#undef sSpecBinByArmPerSiteComparePercent
#undef sAOIBinTraySetting
#undef sBinTrayLinked
#undef sBinLinked
#undef sMagazineSetup
#undef ReadFile
#undef ReadParam
#undef CheckFix2Tray
#undef CheckOSBin
#undef ARTBinCheck
#undef TransferBinTrayStrToName

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"
#include "WebBridge/JsonWriter.h"

bool FileRW_InOutArmSuckHasIC();   // FileRW/_KitSuck.cpp（golden InArmSuck.HasIC() || OutArmSuck.HasIC()）
extern bool bCanLinkT6[eTrayCount]; // 移植樹 cBinSel.cpp:116（golden cBinSel.cpp:52 TU 內全域；ReadFile :1154-1185 算）

namespace {

bool g_booted = false;
bool Booted() { return g_booted; }

// ---------------------------------------------------------------------------
//  7 種 bin 類型（MachineType.h eBinType；golden ctor :974-980 的名稱與分頁）
// ---------------------------------------------------------------------------
struct TagMeta { int tag; const char* name; const char* tab; int pageIndex; };
const TagMeta kTags[eBinTypeTotal] = {
    {eBinRT,      "RT",      "tsRetest",  1},
    {eBinFT,      "FT",      "tsNormal",  0},
    {eBinOffLine, "OffLine", "tsOffline", 2},
    {eBinRT_ART,  "RT_ART",  "tsArtRT",   4},
    {eBinFT_ART,  "FT_ART",  "tsArtFT",   3},
    {eBinRT_MRT,  "RT_MRT",  "tsMrtRT",   6},
    {eBinFT_MRT,  "FT_MRT",  "tsMrtFT",   5},
};
const TagMeta* MetaOfTag(int tag) {
    for (int i = 0; i < eBinTypeTotal; ++i) if (kTags[i].tag == tag) return &kTags[i];
    return nullptr;
}

// golden ReadFile :1127-1152 的 SavePath[tag]（讀哪個檔；SavePath[0] 依 bA02BinModelPrime／bFTBin2RTBin 改）
std::string ReadFileOf(int tag) {
    static const char* const kSavePath[eBinTypeTotal] = {"BinasgnOff.Data", "Binasgn.Data", "BinasgnOff-Line.Data",
        "BinasgnOff_ART.Data", "Binasgn_ART.Data", "Binasgn_MRT_RT.Data", "Binasgn_MRT.Data"};
    if (tag == eBinRT) {
        if (IniConfig.bA02BinModelPrime)
            return (IniConfig.bFTBin2RTBin == true && (iBinModelPrime == 0 || CosFunction.bDisableRTBinSet))
                       ? "Binasgn.Data" : "BinasgnOff.Data";
        return (IniConfig.bFTBin2RTBin == true || CosFunction.bDisableRTBinSet) ? "Binasgn.Data" : "BinasgnOff.Data";
    }
    return (tag >= 0 && tag < eBinTypeTotal) ? kSavePath[tag] : "";
}
// golden ReadFile :1234-1256：這個 tag 會不會被讀（沒讀的 tag 的清單是建構子初值）
bool ReadFileLoads(int tag) {
    if (CosFunction.bUseMRTMode == false && (tag == eBinRT_MRT || tag == eBinFT_MRT)) return false;
    if (USE_AUTO_RETEST == 0 && (tag == eBinRT_ART || tag == eBinFT_ART)) return false;
    if (USE_AUTO_RETEST == 1 && CosFunction.bUseSCKART && (tag == eBinRT_ART || tag == eBinFT_ART)) return false;
    return true;
}
// golden SaveOther :2386-2441：ActivePageIndex → 寫哪個 tag
int SaveTagOfPageIndex(int idx) {
    switch (idx) {
        case 1: return eBinRT;
        case 2: return eBinOffLine;
        case 3: return eBinFT_ART;
        case 4: return eBinRT_ART;
        case 5: return eBinFT_MRT;
        case 6: return eBinRT_MRT;
        default: return eBinFT;
    }
}

// ---------------------------------------------------------------------------
//  清單（golden cBinSel.h:234-286）
// ---------------------------------------------------------------------------
enum ListKind { LK_Derived, LK_Computed, LK_PassThrough, LK_ReadOnly };
struct ListDesc { const char* name; TStringList** (*arr)(); ListKind kind; const char* note; };
#define BS_LIST(n) [] () -> TStringList** { return fBinSel->n; }
const ListDesc kLists[] = {
    // SaveFunctionData(:5716) 寫的 30 組，照 golden 寫檔順序
    {"sBinTraySetT3Pos",        BS_LIST(sBinTraySetT3Pos),        LK_Derived,     "InitDataToEdit :4733 <- BackT6PosTray"},
    {"sBinTraySetT3PosName",    BS_LIST(sBinTraySetT3PosName),    LK_Computed,    "SaveFunctionData TransferBinTrayStrToName(tag) <- sBinTraySetT3Pos"},
    {"sBinDoubleContact",       BS_LIST(sBinDoubleContact),       LK_Derived,     "InitDataToEdit :4734 <- i2Contact"},
    {"sBinType",                BS_LIST(sBinType),                LK_Computed,    "ReadFile :1586-1596 <- BinSelect[tag]（移植樹 ReadFile 仍是 906 的 Prod.bIsPassBin 算法，見報告）"},
    {"sBinConsFail",            BS_LIST(sBinConsFail),            LK_Derived,     "InitDataToEdit :4735 <- bConFail"},
    {"sBinEnableFail",          BS_LIST(sBinEnableFail),          LK_Derived,     "InitDataToEdit :4736 <- bPersentEnable"},
    {"sBinFailPercent",         BS_LIST(sBinFailPercent),         LK_Derived,     "InitDataToEdit :4737 <- dPersentNumber"},
    {"sBinFailIgnore",          BS_LIST(sBinFailIgnore),          LK_Derived,     "InitDataToEdit :4738 <- iPersentIgnore"},
    {"sBinCountEnable",         BS_LIST(sBinCountEnable),         LK_Derived,     "InitDataToEdit :4739 <- bCountEnable"},
    {"sBinCountIgnore",         BS_LIST(sBinCountIgnore),         LK_Derived,     "InitDataToEdit :4740 <- iCountIgnore"},
    {"sBinCountNumber",         BS_LIST(sBinCountNumber),         LK_Derived,     "InitDataToEdit :4741 <- iCountNumber"},
    {"sSpecialBinByArm",        BS_LIST(sSpecialBinByArm),        LK_Derived,     "InitDataToEdit :4742 <- bSpecialBinByArm"},
    {"sSpecialBinCountByArm",   BS_LIST(sSpecialBinCountByArm),   LK_Derived,     "InitDataToEdit :4743 <- iSpecialBinCountByArm"},
    {"sSpecialBinBySocket",     BS_LIST(sSpecialBinBySocket),     LK_Derived,     "InitDataToEdit :4744 <- bSpecialBinBySocket"},
    {"sSpecialBinCountBySocket",BS_LIST(sSpecialBinCountBySocket),LK_Derived,     "InitDataToEdit :4745 <- iSpecialBinCountBySocket"},
    {"sLowYield",               BS_LIST(sLowYield),               LK_Derived,     "InitDataToEdit :4746 <- bLowYield"},
    {"sArmYield",               BS_LIST(sArmYield),               LK_Derived,     "InitDataToEdit :4747 <- bArmYield"},
    {"sSiteYield",              BS_LIST(sSiteYield),              LK_Derived,     "InitDataToEdit :4748 <- bSiteYield"},
    {"sBySiteClean",            BS_LIST(sBySiteClean),            LK_Derived,     "InitDataToEdit :4749 <- iAutoCleanByBin（golden 名稱對調，照翻）"},
    {"sByBinClean",             BS_LIST(sByBinClean),             LK_Derived,     "InitDataToEdit :4750 <- iAutoCleanBySite（golden 名稱對調，照翻）"},
    {"sSpecBinBySiteCompareEnable",        BS_LIST(sSpecBinBySiteCompareEnable),        LK_Derived, "InitDataToEdit :4752"},
    {"sSpecBinBySiteCompareIgnore",        BS_LIST(sSpecBinBySiteCompareIgnore),        LK_Derived, "InitDataToEdit :4753"},
    {"sSpecBinBySiteComparePercent",       BS_LIST(sSpecBinBySiteComparePercent),       LK_Derived, "InitDataToEdit :4754"},
    {"sSpecBinByArmPerSiteCompareEnable",  BS_LIST(sSpecBinByArmPerSiteCompareEnable),  LK_Derived, "InitDataToEdit :4755"},
    {"sSpecBinByArmPerSiteCompareIgnore",  BS_LIST(sSpecBinByArmPerSiteCompareIgnore),  LK_Derived, "InitDataToEdit :4756"},
    {"sSpecBinByArmPerSiteComparePercent", BS_LIST(sSpecBinByArmPerSiteComparePercent), LK_Derived, "InitDataToEdit :4757"},
    {"sBinTrayLinked",          BS_LIST(sBinTrayLinked),          LK_Derived,     "InitDataToEdit :4759 <- bT6Link"},
    {"sBinLinked",              BS_LIST(sBinLinked),              LK_Derived,     "InitDataToEdit :4761-4823 <- sBinTraySetT3Pos + sBinTrayLinked"},
    {"sMagazineSetup",          BS_LIST(sMagazineSetup),          LK_PassThrough, "ReadFunctionData 讀進來、SaveFunctionData 原樣寫回（TfBinSel 不編輯）"},
    {"sAOIBinTraySetting",      BS_LIST(sAOIBinTraySetting),      LK_PassThrough, "ReadFunctionData 讀進來、SaveFunctionData 原樣寫回（由 fBinAOISel 編輯與存檔）"},
    // ReadFile :1557-1621 算的、TfBinSel 不存檔（SECS 用）
    {"sT3TrayType",             BS_LIST(sT3TrayType),             LK_ReadOnly,    "ReadFile :1602 <- BinSelect[tag].iStackDefFailCate"},
    {"sT6Retest",               BS_LIST(sT6Retest),               LK_ReadOnly,    "ReadFile :1611 <- BinSelect[tag].bAutoRetest"},
    {"sT3CateR",                BS_LIST(sT3CateR),                LK_ReadOnly,    "ReadFile :1603 <- BinSelect[tag].bCateR"},
};
#undef BS_LIST
const int kNLists = (int)(sizeof(kLists) / sizeof(kLists[0]));
const char* KindName(ListKind k) {
    switch (k) {
        case LK_Derived: return "derived";
        case LK_Computed: return "computed";
        case LK_PassThrough: return "passThrough";
        default: return "readOnly";
    }
}

// ---------------------------------------------------------------------------
//  MyBinPanel[tag] 的資料欄位（golden TMyBinPanel，cBinSel.cpp:173-213；移植樹 forms/fBinSel.h TMyBinPanelData）
//  per-bin 陣列送 iTestBinCount 筆、per-tray 陣列送 eTrayCount 筆。
//  BackT6PosTray[bin][j]（golden 一個 bin 只會有一個 j 是 1：SetBinTray :3993-4005 保證；ReadFunctionData 也只設一個）
//  以 BackT6PosTrayRow[bin] = 那個 j（eBinNotUse..eBinSetTotal-1），沒有則 -1 交換。
// ---------------------------------------------------------------------------
enum FieldType { FT_Bool, FT_Int, FT_UInt, FT_Double };
enum FieldSpan { FS_Bin, FS_Tray };
struct FieldDesc { const char* name; FieldType type; FieldSpan span; void* (*ptr)(TMyBinPanelData*); };
#define BS_F(n) [] (TMyBinPanelData* p) -> void* { return (void*)p->n; }
const FieldDesc kFields[] = {
    // per-tray（SaveOther :2699-2738 寫）
    {"iT6IsFail",   FT_Int,  FS_Tray, BS_F(iT6IsFail)},
    {"bT6Link",     FT_Bool, FS_Tray, BS_F(bT6Link)},
    {"bT6ART",      FT_Bool, FS_Tray, BS_F(bT6ART)},
    {"bT6CateR",    FT_Bool, FS_Tray, BS_F(bT6CateR)},
    // per-bin（SaveOther :2653-2689 寫舊格式；InitDataToEdit 算成 sXxx 由 SaveFunctionData 寫新格式）
    {"bScan",                   FT_Bool,   FS_Bin, BS_F(bScan)},
    {"i2Contact",               FT_Int,    FS_Bin, BS_F(i2Contact)},
    {"bConFail",                FT_Bool,   FS_Bin, BS_F(bConFail)},
    {"bPersentEnable",          FT_Bool,   FS_Bin, BS_F(bPersentEnable)},
    {"iPersentIgnore",          FT_Int,    FS_Bin, BS_F(iPersentIgnore)},
    {"dPersentNumber",          FT_Double, FS_Bin, BS_F(dPersentNumber)},
    {"bCountEnable",            FT_Bool,   FS_Bin, BS_F(bCountEnable)},
    {"iCountIgnore",            FT_Int,    FS_Bin, BS_F(iCountIgnore)},
    {"iCountNumber",            FT_Int,    FS_Bin, BS_F(iCountNumber)},
    {"bSpecialBinByArm",        FT_Bool,   FS_Bin, BS_F(bSpecialBinByArm)},
    {"iSpecialBinCountByArm",   FT_UInt,   FS_Bin, BS_F(iSpecialBinCountByArm)},
    {"bSpecialBinBySocket",     FT_Bool,   FS_Bin, BS_F(bSpecialBinBySocket)},
    {"iSpecialBinCountBySocket",FT_UInt,   FS_Bin, BS_F(iSpecialBinCountBySocket)},
    {"bLowYield",               FT_Bool,   FS_Bin, BS_F(bLowYield)},
    {"bArmYield",               FT_Bool,   FS_Bin, BS_F(bArmYield)},
    {"bSiteYield",              FT_Bool,   FS_Bin, BS_F(bSiteYield)},
    // per-bin，只經 InitDataToEdit → sXxx → SaveFunctionData 存
    {"iAutoCleanByBin",                    FT_Int,    FS_Bin, BS_F(iAutoCleanByBin)},
    {"iAutoCleanBySite",                   FT_Int,    FS_Bin, BS_F(iAutoCleanBySite)},
    {"bSpecBinBySiteCompareEnable",        FT_Bool,   FS_Bin, BS_F(bSpecBinBySiteCompareEnable)},
    {"iSpecBinBySiteCompareIgnore",        FT_Int,    FS_Bin, BS_F(iSpecBinBySiteCompareIgnore)},
    {"dSpecBinBySiteComparePercent",       FT_Double, FS_Bin, BS_F(dSpecBinBySiteComparePercent)},
    {"bSpecBinByArmPerSiteCompareEnable",  FT_Bool,   FS_Bin, BS_F(bSpecBinByArmPerSiteCompareEnable)},
    {"iSpecBinByArmPerSiteCompareIgnore",  FT_Int,    FS_Bin, BS_F(iSpecBinByArmPerSiteCompareIgnore)},
    {"dSpecBinByArmPerSiteComparePercent", FT_Double, FS_Bin, BS_F(dSpecBinByArmPerSiteComparePercent)},
};
#undef BS_F
const int kNFields = (int)(sizeof(kFields) / sizeof(kFields[0]));
int SpanLen(FieldSpan s) { return s == FS_Tray ? (int)eTrayCount : iTestBinCount; }

void PutList(webbridge::JsonWriter& w, TStringList* l) {
    w.BeginArray();
    for (int i = 0; l && i < l->Count; ++i) w.String(AnsiString(l->Strings[i]).c_str());
    w.EndArray();
}

void PutPanel(webbridge::JsonWriter& w, TMyBinPanelData* p) {
    w.BeginObject();
    w.Key("iErrorT6").Number((wb_int64)p->iErrorT6);
    for (int f = 0; f < kNFields; ++f) {
        const FieldDesc& d = kFields[f];
        void* a = d.ptr(p);
        w.Key(d.name).BeginArray();
        for (int i = 0; i < SpanLen(d.span); ++i) {
            switch (d.type) {
                case FT_Bool:   w.Bool(((bool*)a)[i]); break;
                case FT_Int:    w.Number((wb_int64)((int*)a)[i]); break;
                case FT_UInt:   w.Number((wb_int64)((unsigned int*)a)[i]); break;
                case FT_Double: w.Number(((double*)a)[i]); break;
            }
        }
        w.EndArray();
    }
    w.Key("BackT6PosTrayRow").BeginArray();
    for (int i = 0; i < iTestBinCount; ++i) {
        int row = -1;
        for (int j = eBinNotUse; j < eBinSetTotal; ++j)
            if (p->BackT6PosTray[i][j]) { row = j; break; }   // golden SaveOther :2667-2687 同樣取第一個
        w.Number((wb_int64)row);
    }
    w.EndArray();
    w.EndObject();
}

// 這個 tag 的 bin 資料能不能改：golden MyBinPanel[tag]->Panel->Enabled（FormShow :1726-1779）且分頁本身可改
// （FormShow :1843-1875 AccessLevel／critical para 停用 tsX；cbTestModeChange TabVisible）
bool TagEditable(int tag) {
    const TagMeta* m = MetaOfTag(tag);
    return m && bBinPanelEnabled[tag] && filerw::ELEditable("TfBinSel", m->tab);
}

// golden UNLOADER_ART[Y]==eartInstall（mtTrayNameMouseDown :2954、mtTrayNameSetColor :4143）。golden 只在 Y<=iAutoRight
// 時讀它（兩處都先判斷）；移植樹 UNLOADER_ART 長度是 MAX_AUTO_TRAY（cmydef.h:2986），不是 eTrayCount → 範圍外一律 false。
bool UnloaderArtInstalled(int t) {
    return t >= 0 && t <= iAutoRight && t < MAX_AUTO_TRAY && UNLOADER_ART[t] == eartInstall;
}

// ---------------------------------------------------------------------------
//  bin.rules：golden 編輯規則取決的機台設定（頁面 ht9045_binsel_wire.js buildRules() 的 RULE_KEYS）。
//  每一項照 golden 的原式從移植樹同名全域算；沒輸出的項目頁面照舊推定並列在畫面上方。
//  客戶專屬條件（Steven 20260925 決定先跳過）：只在 CUSTOMER_CODE 不是那個客戶時輸出（那時該條件恆為 false，值是確定的）；
//  是那個客戶時不輸出，讓頁面推定並列出。
// ---------------------------------------------------------------------------
void PutRules(webbridge::JsonWriter& w) {
    w.Key("rules").BeginObject();
    // Prod.iTrayType[i]==tNotUse（mtTrayNameSetColor :4092、bCheckTrayCanUse :2849）
    w.Key("trayNotUse").BeginArray();
    for (int i = 0; i < eTrayCount; ++i) w.Bool(Prod.iTrayType[i] == tNotUse);
    w.EndArray();
    // TfBinSel::bCheckTrayCanUse（golden :2844-2877；移植樹 cBinSel.cpp:4208 同式）
    w.Key("trayCanUse").BeginArray();
    for (int i = 0; i < eTrayCount; ++i) w.Bool(fBinSel->bCheckTrayCanUse(i));
    w.EndArray();
    // bCanLinkT6[]：ReadFile :1154-1185 算（移植樹 cBinSel.cpp:1436-1453 同式，全域 cBinSel.cpp:116）。FormShow 的 ReadFile 剛算過。
    w.Key("canLinkT6").BeginArray();
    for (int i = 0; i < eTrayCount; ++i) w.Bool(bCanLinkT6[i]);
    w.EndArray();
    // UNLOADER_ART[i]==eartInstall（:2954、:4143）
    w.Key("unloaderArt").BeginArray();
    for (int i = 0; i < eTrayCount; ++i) w.Bool(UnloaderArtInstalled(i));
    w.EndArray();
    w.Key("bulkBox").Number((wb_int64)eBulkBox);                                           // :3057 Y-eBinSetting==eBulkBox
    w.Key("multiColorFail").Bool(IniConfig.bG07MultiColorForFailBin);                      // :2906
    w.Key("artInstall").Bool(USE_AUTO_RETEST == eartInstall);                             // :2950、:4137
    w.Key("sltSummary").Bool(IniConfig.bA38_SLT_Summary);                                  // :2970、:4138
    w.Key("sckArt").Bool(CosFunction.bUseSCKART);                                          // :2968、:4155
    w.Key("sckArtSortMode1").Bool(TestIF_File.iSCKART_SortMode == 1);                      // :2969、:4156
    w.Key("iAutoRight").Number((wb_int64)iAutoRight);                                      // :2952、:4140
    // mtTrayNameSetColor :4119-4120：bEnableQASampling && iT6Pos==iQASamplingT3Pos（頁面 t+1===qaT3Pos，-1 = 不用 QA）
    w.Key("qaT3Pos").Number((wb_int64)(TestIF_File.bEnableQASampling ? TestIF_File.iQASamplingT3Pos : -1));
    // SeteDoubleContact :3350（CC_ASE_CL 另要權限 132 :3321，頁面未模擬）／InitDataToEdit :4314
    w.Key("multiDoubleContact").Bool(Prod.bD22SupportMultiDoubleContact);
    w.Key("doubleContactMax").Number((wb_int64)(IniConfig.iD22DoubleContactCount + 2));    // :3352 ShowQwertyKey 上限
    w.Key("fromYieldForm").Bool(CosFunction.bByBinAlarmFromYieldForm);                     // :3074、:4338
    // :3066-3072、:4614-4616：Compare 六列只在 bBySiteByBinPercentCompare && !bByBinAlarmFromYieldForm 時可用
    w.Key("cmpActive").Bool(CosFunction.bBySiteByBinPercentCompare && !CosFunction.bByBinAlarmFromYieldForm);
    w.Key("lowYieldByBin").Bool(Prod.bLowYieldAlarmByBin);                                 // :4531、:4581、:4599
    w.Key("autoClean").Bool(TestIF_File.iAutoClean_Function != 0);                         // :3740、:3755、:4546
    // CheckCountSettingPassword :6717-6725（本 TU 的 bCountSetPassed，FormShow 每次設 false）
    w.Key("countNeedsPassword").Bool(CosFunction.bYieldSettingNeedPassword && !bCountSetPassed);
    w.Key("oneCycle").Bool(CosFunction.bOneCycleCanChangeContinuesFailBin);               // :3038、:3110、:3298、:6489
    // :3112-3115、:6491-6494：InArmSuck.HasIC() || OutArmSuck.HasIC() || ShuttleHasIC() || IndexHasIC()
    w.Key("icInMachine").Bool(FileRW_InOutArmSuckHasIC() || ShuttleHasIC() || IndexHasIC());
    w.Key("bin1CanNotInFix").Bool(CosFunction.bBin1CanNotInFix);                           // SetBinTray :3987
    w.Key("posFix1Row").Number((wb_int64)(eBinSetting + ePosFix1 - 1));                    // SetBinTray :3989 iStartY>=eBinSetting+ePosFix1-1
    w.Key("offLineTag").Number((wb_int64)OffT);                                            // :3987 tag!=OffT
    w.Key("iFixRight").Number((wb_int64)iFixRight);                                        // CancelErrorBinClick :6130、rg_FixBinBoxClick :6142
    w.Key("testRunModeTag").Number((wb_int64)iTestRunMode);                                // btnSetAll2NotUseClick :6425 SetBinTray(iTestRunMode)
    // :3083-3093：bUseYieldControlFunction 時 Y 1..24 要權限 148（Insufficient 的 bAlarm 傳 false：只取值，不跳訊息）
    w.Key("lockFuncRows").Bool(CosFunction.bUseYieldControlFunction && fSecurity->Insufficient(148, false) == false);
    // :3085-3089 Korea 149；:3097-3102 另有 CC_SCK && bEnableRms（客戶專屬，跳過）→ CC_SCK 時不輸出
    if (CUSTOMER_CODE != CC_SCK)
        w.Key("lockTrayRows").Bool(CosFunction.bUseYieldControlFunction && IniConfig.bKoreaFunction &&
                                   fSecurity->Insufficient(149, false) == false);
    // SeteConsFail :3383：CC_KYEC_LEE && bEnablePEModel==false（客戶專屬，跳過）→ 不是 KYEC 時恆 false
    if (CUSTOMER_CODE != CC_KYEC_LEE) w.Key("kyecForceConsFail").Bool(false);
    // cbTestMode->Items：DFM 設計期選項 ＋ 建構子 :1080-1098 依設定增刪（BS_DfmItems → BS_TfBinSel，開機時建）
    w.Key("modeItems").BeginArray();
    for (int i = 0; i < fBinSel->cbTestMode->Items->Count; ++i)
        w.String(AnsiString(fBinSel->cbTestMode->Items->Strings[i]).c_str());
    w.EndArray();
    w.EndObject();
}

std::string BinJson() {
    webbridge::JsonWriter w;
    w.BeginObject();
    const int active = fBinSel->PageControl1->ActivePageIndex;
    w.Key("activePageIndex").Number((wb_int64)active);
    w.Key("activeTag").Number((wb_int64)SaveTagOfPageIndex(active));
    w.Key("iTestBinCount").Number((wb_int64)iTestBinCount);
    w.Key("eTrayCount").Number((wb_int64)eTrayCount);
    w.Key("eBinNotUse").Number((wb_int64)eBinNotUse);
    w.Key("eBinSetting").Number((wb_int64)eBinSetting);
    w.Key("eBinSetTotal").Number((wb_int64)eBinSetTotal);
    w.Key("s6TrayName").BeginArray();
    for (int i = 0; i < eTrayCount; ++i) w.String(s6TrayName[i].c_str());
    w.EndArray();
    w.Key("s3TrayName").BeginArray();
    for (int i = 0; i < e3TrayCount; ++i) w.String(s3TrayName[i].c_str());
    w.EndArray();
    w.Key("iTo3PosUnload").BeginArray();
    for (int i = 0; i < ePosTrayCount; ++i) w.Number((wb_int64)iTo3PosUnload[i]);
    w.EndArray();
    w.Key("listKinds").BeginObject();
    for (int k = 0; k < kNLists; ++k)
        w.Key(kLists[k].name).BeginObject().Key("kind").String(KindName(kLists[k].kind)).Key("source").String(kLists[k].note).EndObject();
    w.EndObject();
    PutRules(w);
    w.Key("tags").BeginArray();
    for (int t = 0; t < eBinTypeTotal; ++t) {
        const TagMeta& m = kTags[t];
        w.BeginObject();
        w.Key("tag").Number((wb_int64)m.tag);
        w.Key("name").String(m.name);
        w.Key("tab").String(m.tab);
        w.Key("pageIndex").Number((wb_int64)m.pageIndex);
        w.Key("file").String(ReadFileOf(m.tag));
        w.Key("loaded").Bool(ReadFileLoads(m.tag));
        w.Key("panelEnabled").Bool(bBinPanelEnabled[m.tag]);
        w.Key("editable").Bool(TagEditable(m.tag));
        w.Key("lists").BeginObject();
        for (int k = 0; k < kNLists; ++k) {
            w.Key(kLists[k].name);
            PutList(w, kLists[k].arr()[m.tag]);
        }
        w.EndObject();
        w.Key("panel");
        PutPanel(w, fBinSel->MyBinPanel[m.tag]);
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    return w.Str();
}

// ---------------------------------------------------------------------------
//  存檔：頁面送的 bin 資料先完整驗證（型別、長度、範圍），存進 g_pending；golden 存檔流程（saveFlow）裡才套用。
// ---------------------------------------------------------------------------
struct PendingPanel {
    int tag = -1;
    int iErrorT6 = 0;
    std::vector<std::vector<double> > fields;   // kFields 順序；bool 以 0/1
    std::vector<int> row;                        // BackT6PosTrayRow
    std::vector<std::pair<std::string, std::vector<std::string> > > lists;   // 頁面送的清單（只比對，不套用）
};
struct Pending {
    bool active = false;
    std::vector<PendingPanel> panels;
    // 結果（併進 ack.bin）
    int savedTag = -1;
    bool applied = false;
    std::string reason;
    std::vector<std::string> ignoredTags, passwordRefused, listsDiffer, listsNotSent;
};
Pending g_pending;
bool g_shown = false;
int g_shownLevel = -1;

bool ParsePanel(const cJSON* tagObj, PendingPanel* out, std::string* err) {
    const cJSON* jt = cJSON_GetObjectItemCaseSensitive(tagObj, "tag");
    if (!jt || !cJSON_IsNumber(jt) || jt->valueint < 0 || jt->valueint >= eBinTypeTotal) { *err = "bin.tags[].tag must be 0..6"; return false; }
    out->tag = jt->valueint;
    const std::string where = std::string("bin tag ") + MetaOfTag(out->tag)->name + ": ";
    const cJSON* p = cJSON_GetObjectItemCaseSensitive(tagObj, "panel");
    if (!p || !cJSON_IsObject(p)) { *err = where + "panel object missing"; return false; }
    const cJSON* e = cJSON_GetObjectItemCaseSensitive(p, "iErrorT6");
    if (!e || !cJSON_IsNumber(e) || e->valuedouble != std::floor(e->valuedouble) || e->valueint < 0 || e->valueint >= eTrayCount) {
        *err = where + "panel.iErrorT6 must be an integer 0..eTrayCount-1"; return false;
    }
    out->iErrorT6 = e->valueint;
    out->fields.assign(kNFields, std::vector<double>());
    for (int f = 0; f < kNFields; ++f) {
        const FieldDesc& d = kFields[f];
        const cJSON* a = cJSON_GetObjectItemCaseSensitive(p, d.name);
        const int n = SpanLen(d.span);
        if (!a || !cJSON_IsArray(a) || cJSON_GetArraySize(a) != n) {
            *err = where + "panel." + d.name + " must be an array of " + std::to_string(n); return false;
        }
        for (const cJSON* x = a->child; x; x = x->next) {
            double v;
            if (d.type == FT_Bool) {
                if (!cJSON_IsBool(x)) { *err = where + "panel." + d.name + " items must be true/false"; return false; }
                v = cJSON_IsTrue(x) ? 1 : 0;
            } else {
                if (!cJSON_IsNumber(x)) { *err = where + "panel." + d.name + " items must be numbers"; return false; }
                v = x->valuedouble;
                if (d.type != FT_Double && v != std::floor(v)) { *err = where + "panel." + d.name + " items must be integers"; return false; }
                if (d.type == FT_UInt && (v < 0 || v > 4294967295.0)) { *err = where + "panel." + d.name + " items must be 0..4294967295"; return false; }
                if (d.type == FT_Int && (v < -2147483648.0 || v > 2147483647.0)) { *err = where + "panel." + d.name + " out of int range"; return false; }
            }
            out->fields[f].push_back(v);
        }
    }
    const cJSON* r = cJSON_GetObjectItemCaseSensitive(p, "BackT6PosTrayRow");
    if (!r || !cJSON_IsArray(r) || cJSON_GetArraySize(r) != iTestBinCount) {
        *err = where + "panel.BackT6PosTrayRow must be an array of iTestBinCount"; return false;
    }
    for (const cJSON* x = r->child; x; x = x->next) {
        if (!cJSON_IsNumber(x) || x->valuedouble != std::floor(x->valuedouble) ||
            !(x->valueint == -1 || (x->valueint >= eBinNotUse && x->valueint < eBinSetTotal))) {
            *err = where + "panel.BackT6PosTrayRow items must be -1 or eBinNotUse..eBinSetTotal-1"; return false;
        }
        out->row.push_back(x->valueint);
    }
    const cJSON* ls = cJSON_GetObjectItemCaseSensitive(tagObj, "lists");
    if (ls) {
        if (!cJSON_IsObject(ls)) { *err = where + "lists must be an object"; return false; }
        for (const cJSON* it = ls->child; it; it = it->next) {
            bool known = false;
            for (int k = 0; k < kNLists; ++k) if (it->string == std::string(kLists[k].name)) known = true;
            if (!known) { *err = where + "unknown list " + it->string; return false; }
            if (!cJSON_IsArray(it)) { *err = where + "lists." + it->string + " must be an array of strings"; return false; }
            std::vector<std::string> v;
            for (const cJSON* x = it->child; x; x = x->next) {
                if (!cJSON_IsString(x)) { *err = where + "lists." + it->string + " must be an array of strings"; return false; }
                v.push_back(x->valuestring);
            }
            out->lists.push_back(std::make_pair(std::string(it->string), v));
        }
    }
    return true;
}

// golden 只有 mtTrayNameMouseDown（:2879-3008）改得到托盤的 Link／Retest／CateR 欄；那裡的設定閘在網頁端只是畫面判斷，
// 後端照 golden 再擋一次（只擋「改了」的盤：檔案裡已經是那樣的值不擋，否則設定改過之後整頁都存不了）。
// 每個盤先過 bCheckTrayCanUse（:2890）。不擋的（golden 另有路徑改得到，見報告）：bin 指派到 tNotUse／BulkBox／Link 盤
// （mtTrayItemMouseUp :6473-6533 點列名把全部 bin 設到該列，沒有這些檢查）、iErrorT6（CancelErrorBinClick :6124 直接設 iFixRight）。
int FieldIndex(const char* name) {
    for (int f = 0; f < kNFields; ++f) if (std::string(kFields[f].name) == name) return f;
    return -1;
}
bool ValidateTrayEdits(const PendingPanel& pp, TMyBinPanelData* p, std::string* err) {
    const std::string where = std::string("bin tag ") + MetaOfTag(pp.tag)->name + ": ";
    const int fLink = FieldIndex("bT6Link"), fArt = FieldIndex("bT6ART"), fCateR = FieldIndex("bT6CateR");
    for (int t = 0; t < eTrayCount; ++t) {
        const std::string tray = std::string("tray ") + std::to_string(t) + " (" + s6TrayName[t].c_str() + ")";
        const bool link = pp.fields[fLink][t] != 0, art = pp.fields[fArt][t] != 0, cateR = pp.fields[fCateR][t] != 0;
        const bool linkChanged = link != p->bT6Link[t];
        const bool artOn = art && !p->bT6ART[t];
        const bool cateROn = cateR && !p->bT6CateR[t];
        if (!linkChanged && !artOn && !cateROn) continue;
        if (!fBinSel->bCheckTrayCanUse(t)) {                                    // :2890
            *err = where + tray + " cannot be edited: golden bCheckTrayCanUse() is false (tNotUse / magazine / QA sampling tray, cBinSel.cpp:2844)";
            return false;
        }
        if (linkChanged && !bCanLinkT6[t]) {                                    // :2921 if(bCanLinkT6[Y]==true)
            *err = where + tray + " cannot be linked: golden bCanLinkT6[] is false (Auto1 / Fix1 / BulkBox / Auto tray with bAutoTrayLink off, cBinSel.cpp ReadFile:1154-1185)";
            return false;
        }
        if (artOn && !(USE_AUTO_RETEST == eartInstall && t <= iAutoRight && UnloaderArtInstalled(t))) {   // :2950-2961
            *err = where + tray + " Retest cannot be turned on: golden needs USE_AUTO_RETEST installed, an Auto tray and UNLOADER_ART installed for it (cBinSel.cpp:2950-2961)";
            return false;
        }
        if (cateROn && !((USE_AUTO_RETEST == eartInstall && CosFunction.bUseSCKART && TestIF_File.iSCKART_SortMode == 1) ||
                         IniConfig.bA38_SLT_Summary)) {                         // :2967-2983
            *err = where + tray + " CateR cannot be turned on: golden needs SCK ART sort mode 1 or A38 SLT summary (cBinSel.cpp:2967-2983)";
            return false;
        }
    }
    return true;
}

// golden CheckCountSettingPassword（cBinSel.cpp:6717，AI rf360 20260814）：Count 三列（SeteCountEnable／Ignore／Number
// :3530-3604）改值要密碼。網頁端沒有密碼框 → 視同密碼錯誤（ELPasswordRefused），那三列保留原值。
bool CountChangedNeedsPassword(const PendingPanel& pp, TMyBinPanelData* p) {
    if (CosFunction.bYieldSettingNeedPassword == false) return false;   // golden :6722
    if (bCountSetPassed == true) return false;                          // golden :6725（本 TU 的 FormShow 每次設 false）
    for (int f = 0; f < kNFields; ++f) {
        const std::string n = kFields[f].name;
        if (n != "bCountEnable" && n != "iCountIgnore" && n != "iCountNumber") continue;
        void* a = kFields[f].ptr(p);
        for (int i = 0; i < iTestBinCount; ++i) {
            const double cur = kFields[f].type == FT_Bool ? (((bool*)a)[i] ? 1 : 0) : ((int*)a)[i];
            if (cur != pp.fields[f][i]) return true;
        }
    }
    return false;
}

void ApplyPanel(const PendingPanel& pp, TMyBinPanelData* p, bool keepCount) {
    p->iErrorT6 = pp.iErrorT6;
    for (int f = 0; f < kNFields; ++f) {
        const FieldDesc& d = kFields[f];
        const std::string n = d.name;
        if (keepCount && (n == "bCountEnable" || n == "iCountIgnore" || n == "iCountNumber")) continue;
        void* a = d.ptr(p);
        for (int i = 0; i < SpanLen(d.span); ++i) {
            const double v = pp.fields[f][i];
            switch (d.type) {
                case FT_Bool:   ((bool*)a)[i] = v != 0; break;
                case FT_Int:    ((int*)a)[i] = (int)v; break;
                case FT_UInt:   ((unsigned int*)a)[i] = (unsigned int)v; break;
                case FT_Double: ((double*)a)[i] = v; break;
            }
        }
    }
    for (int i = 0; i < iTestBinCount; ++i) {
        for (int j = eBinNotUse; j < eBinSetTotal; ++j) p->BackT6PosTray[i][j] = 0;
        if (pp.row[i] >= eBinNotUse) p->BackT6PosTray[i][pp.row[i]] = 1;
    }
}

// golden cbTestModeChange（:2147-2230）的「文字 → 分頁」對照（TabVisible＋ActivePageIndex）。
// 不含它開頭的 ReadFile(:2151)：golden 那一行發生在使用者改下拉選單「當下」（丟掉前一個分頁未存的編輯）；
// 頁面開頁時已拿到 7 組的檔案值，只送作用中那一組 —— 等同 golden 換頁後再編輯。這裡若再 ReadFile，會把
// PageSave 剛套上的表單元件（CancelErrorBin、rg_FixBinBox…，收養的同一物件）改回檔案值。
void ApplyTestModeTab() {
    TComboBox* cb = filerw::EL<TComboBox>("TfBinSel", "cbTestMode");
    static const struct { const char* text; int idx; } kMap[] = {
        {"Re-Test", 1}, {"Off-Line", 2}, {"ART Re-Test", 4}, {"ART Normal", 3}, {"Normal", 0}, {"MRT Re-Test", 6}, {"MRT Normal", 5}};
    static const char* const kTab[7] = {"tsNormal", "tsRetest", "tsOffline", "tsArtFT", "tsArtRT", "tsMrtFT", "tsMrtRT"};
    // golden 讀 cbTestMode->Text（VCL 選項改變時 Text 跟著變）。頁面只送 itemIndex 時 ELApplyProxies 不動 Text →
    // Text 對不上任何選項才用 Items[ItemIndex]
    AnsiString t = cb->Text;
    bool known = false;
    for (std::size_t k = 0; k < sizeof(kMap) / sizeof(kMap[0]); ++k) if (t == AnsiString(kMap[k].text)) known = true;
    if (!known && cb->ItemIndex >= 0 && cb->ItemIndex < cb->Items->Count) t = AnsiString(cb->Items->Strings[cb->ItemIndex]);
    for (std::size_t k = 0; k < sizeof(kMap) / sizeof(kMap[0]); ++k) {
        if (!(t == AnsiString(kMap[k].text))) continue;
        for (int i = 0; i < 7; ++i) filerw::EL<TTabSheet>("TfBinSel", kTab[i])->TabVisible = (i == kMap[k].idx);
        fBinSel->PageControl1->ActivePageIndex = kMap[k].idx;
        return;
    }
    // golden 沒有 else：文字不在清單上就不換頁（沿用開頁時 cbTestModeChange 設的分頁）
}

// PageDesc::saveFlow：golden 存檔鈕，前面套上頁面的 bin 資料
void SaveFlow() {
    ApplyTestModeTab();
    const int tag = SaveTagOfPageIndex(fBinSel->PageControl1->ActivePageIndex);
    g_pending.savedTag = tag;
    const PendingPanel* mine = nullptr;
    for (std::size_t i = 0; i < g_pending.panels.size(); ++i) {
        if (g_pending.panels[i].tag == tag) mine = &g_pending.panels[i];
        else g_pending.ignoredTags.push_back(MetaOfTag(g_pending.panels[i].tag)->name);
    }
    if (!mine) {
        g_pending.reason = "page sent no bin data for the active tag -- golden saves the values loaded by FormShow";
    } else if (!TagEditable(tag)) {
        g_pending.reason = "bin panel of the active tag is disabled (golden MyBinPanel[tag]->Panel->Enabled / tab Enabled) -- page values dropped";
    } else {
        TMyBinPanelData* p = fBinSel->MyBinPanel[tag];
        const bool keepCount = CountChangedNeedsPassword(*mine, p);
        if (keepCount) {
            filerw::ELPasswordRefused("CheckCountSettingPassword (Count Bin/Ignore/Number, cBinSel.cpp:6717)");
            g_pending.passwordRefused.push_back("bCountEnable");
            g_pending.passwordRefused.push_back("iCountIgnore");
            g_pending.passwordRefused.push_back("iCountNumber");
        }
        ApplyPanel(*mine, p, keepCount);
        fBinSel->InitDataToEdit(tag);   // golden 每個編輯的收尾（SetBinTray :4018、mtTrayNameSetColor :4187）
        g_pending.applied = true;
    }
    BS_spbSaveClick();
    // golden 實際寫的清單（spbSaveClick 最後 ReadFile 重讀回來的值）和頁面送的比對
    if (mine && filerw::ELMarked("SaveFunctionData")) {
        for (std::size_t k = 0; k < mine->lists.size(); ++k) {
            for (int i = 0; i < kNLists; ++i) {
                if (mine->lists[k].first != kLists[i].name) continue;
                TStringList* l = kLists[i].arr()[tag];
                bool same = l && l->Count == (int)mine->lists[k].second.size();
                for (int j = 0; same && j < l->Count; ++j) same = AnsiString(l->Strings[j]) == AnsiString(mine->lists[k].second[j].c_str());
                if (!same) g_pending.listsDiffer.push_back(kLists[i].name);
            }
        }
    }
}

// PageDesc::reload：golden 沒寫檔時關頁 FormClose（:2137-2144）的 ReadFile
void Reload() { fBinSel->ReadFile(false, false, AnsiString("")); }

const filerw::PageDesc kPage = {
    "BinSelect", "TfBinSel", "Setup.BinSel.html",
    nullptr, nullptr, 0,
    kBS_SaveReads, (int)(sizeof(kBS_SaveReads) / sizeof(kBS_SaveReads[0])),
    &BS_FormShow, &SaveFlow, "SaveFunctionData", &Reload, &Booted,
};
// 不用 filerw::PageRegistrar：通用 editlist.get／save 不帶 bin 資料。wb_serve 對 tag=BinSelect 走本檔的
// FileRW_BinSelect_Page／FileRW_BinSelect_Save（同 Teach）。

// 移植樹 fBinSel 的同名同型別表單元件（forms/fBinSel.h）→ 替身就是那個物件（產生器的 adopt 認不得 `new TLabel()`）。
// 移植樹 ReadFile／ReadWriteMRTMode(0)／ReadWriteSpecialFunction(false)／ReadPrimeDara 寫的就是這些物件，
// 所以開頁後替身的值＝檔案值。sgSpecificBin 型別不同（golden TStringGrid → filerw::ELStringGrid），不收養。
void BS_AdoptPortWidgets() {
    filerw::ELKeep("TfBinSel", "Label1", fBinSel->Label1);
    filerw::ELKeep("TfBinSel", "spbSave", fBinSel->spbSave);
    filerw::ELKeep("TfBinSel", "palSpecificBin", fBinSel->palSpecificBin);
    filerw::ELKeep("TfBinSel", "CancelErrorBin", fBinSel->CancelErrorBin);
    filerw::ELKeep("TfBinSel", "chkShow0Xbin", fBinSel->chkShow0Xbin);
    filerw::ELKeep("TfBinSel", "rg_FixBinBox", fBinSel->rg_FixBinBox);
    filerw::ELKeep("TfBinSel", "ed_FixBinBoxAlarmCount", fBinSel->ed_FixBinBoxAlarmCount);
    filerw::ELKeep("TfBinSel", "cbUseMRTMode", fBinSel->cbUseMRTMode);
    filerw::ELKeep("TfBinSel", "cbbAutoSiteMap", fBinSel->cbbAutoSiteMap);
    filerw::ELKeep("TfBinSel", "cbbASMPassBin", fBinSel->cbbASMPassBin);
    filerw::ELKeep("TfBinSel", "cbOutShtLoseICSetErrUntilOneCycle", fBinSel->cbOutShtLoseICSetErrUntilOneCycle);
    filerw::ELKeep("TfBinSel", "cbIndexDropErrSetErrUntilOneCycle", fBinSel->cbIndexDropErrSetErrUntilOneCycle);
    filerw::ELKeep("TfBinSel", "cbTestMode", fBinSel->cbTestMode);
    filerw::ELKeep("TfBinSel", "PageControl1", fBinSel->PageControl1);
    filerw::ELKeep("TfBinSel", "labWarning", fBinSel->labWarning);
    // 收養的下拉／選項在移植樹建構子（靜態初始化時，設定檔還沒讀）已加過選項 → 清掉，照 golden DFM＋建構子重建
    fBinSel->cbTestMode->Items->Clear();
    fBinSel->cbbAutoSiteMap->Items->Clear();
    fBinSel->cbbASMPassBin->Items->Clear();
    fBinSel->rg_FixBinBox->Items->Clear();
}

}  // namespace

// golden TfBinSel 建構（HT9045.cpp CreateForm）：收養 → DFM 設計期狀態 → 建構子（cbTestMode 選項）→ 存檔讀的替身。
// wb_serve：放在開機讀檔鏈 fBinSel->ReadParam(); fBinSel->ReadFile(...) 之前（設定檔已載入，CosFunction 旗標已定）。
void FileRW_BinSelect_Boot()
{
    if (g_booted) return;
    BS_AdoptPortWidgets();
    BS_DfmItems();
    BS_DfmState();
    BS_CreateSaveProxies();
    BS_CreateContainerProxies();
    // golden cBinSel.dfm:394 sgSpecificBin ColCount = 33（產生器的 DFM 狀態不帶 ColCount）
    filerw::EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->grid.ColCount = 33;
    BS_TfBinSel();
    std::printf("FileRW BinSelect: TfBinSel proxies ready (%d save reads, cbTestMode %d items) -- golden cBinSel.cpp\n",
                (int)(sizeof(kBS_SaveReads) / sizeof(kBS_SaveReads[0])), fBinSel->cbTestMode->Items->Count);
    g_booted = true;
}

// WS editlist.get tag=BinSelect：呼叫端持 FormLock、在主迴圈。
//   回 _EditPage 的 {struct, form, booted, lists:{}, proxies, session, mustSend} ＋ "bin":{…}（見 BinJson）。
int FileRW_BinSelect_Page(std::string* json)
{
    std::string base;
    const int st = filerw::PageJson(kPage, &base);   // golden FormShow（ReadParam／ReadPrimeDara／ReadFile／權限）
    if (st != 200) { *json = base; return st; }
    g_shown = true;
    g_shownLevel = AccessLevel;
    const std::size_t end = base.find_last_of('}');
    if (end == std::string::npos) { *json = "internal: PageJson returned no object"; return 500; }
    *json = base.substr(0, end) + ",\"bin\":" + BinJson() + "}";
    return 200;
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S99) 20260926: golden TfBinSel::spbNormalClick（V912 cBinSel.cpp:2800-2820）／spbPrimeClick（:2822-2842）。
//    兩顆鈕只在 IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime 時看得見（golden SetPrimeButton :6058-6090，開頁
//    FormShow → ReadPrimeDara :6099 就跑一次）。按下 → 確認框「Change Binning to Normal/Prime mode?」→ YES：WritePrimeDara
//    （:6107-6116）立刻寫 <DataPath><配方>\Binasgn.Data [BinModel] bPrime＝0／1、改 iBinModelPrime → ReadFile(false,false,"")
//    重讀全部 7 組 bin 設定（RT 那一組讀哪個檔跟著 iBinModelPrime 變，ReadFileOf 同一條規則）→ tsRetest／cbTestMode 的 Enabled。
//    golden 沒有權限檢查、沒有 SystemStart 檢查（表單開著、鈕看得見就能按），照翻。
//  通道：WS editlist.save tag=BinSelect，value＝{"op":"spbNormalClick"|"spbPrimeClick","answers":{…},"widgets":{}}
//    （wb_serve 的 editlist.save 分支要求 widgets 是物件，送空物件即可；不看 widgets／bin／actions）。
//    確認框走 filerw::ELAsk：第一次不帶答案 → 回 session.asked（golden 當 NO：SetPrimeButton 後 return，沒寫檔）；頁面照 golden 字樣
//    問完、帶 answers {"Change Binning to Prime mode?":1} 重送才寫。
//  守衛（不信任前端）：開頁過（同一個 AccessLevel，FileRW_BinSelect_Save 開頭那兩條）；按鈕替身可按（filerw::ELEditable：
//    自己與每一層容器 Enabled 且 Visible —— SetPrimeButton 條件不成立時看不見 → 409，什麼都不動）。
//  ⚠ ReadFile 會把 7 組 bin 設定重讀成檔案值（golden 同）：頁面上還沒存的 bin 編輯會被丟掉 —— 頁面收到回應要用 binAfter 重畫。
//  與 C 路存檔的關係：存檔鈕（SaveOther :2783）也寫 [BinModel] bPrime，寫的是記憶體 iBinModelPrime；本函式寫檔同時改記憶體
//    （golden :6113-6114），之後存檔寫的是新值，不會蓋回舊值。
//  回 200 時 *ack＝{"op","executed","before","after","file","goldenLine","session","proxies","binAfter"}；executed＝真的寫了檔。
// ---------------------------------------------------------------------------
static int FileRW_BinSelect_PrimeClick(const std::string& op, const std::string& answersJson, std::string* ack, std::string* err)
{
    const bool normal = (op == "spbNormalClick");
    const char* btn = normal ? "spbNormal" : "spbPrime";
    if (!filerw::ELEditable("TfBinSel", btn)) {
        *err = std::string(btn) + " cannot be pressed on this machine/page state (golden SetPrimeButton cBinSel.cpp:6058-6090 shows it only when "
               "IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime; or its container is disabled) -- nothing done";
        return 409;
    }
    AnsiString S=GetLastOpenFN();
    AnsiString szDir;
    szDir.sprintf("%s%s\\Binasgn.Data", DataPath.c_str(), S.c_str());           // golden :6112（同 BS_WritePrimeDara，只給回應用）
    filerw::SessionBegin(answersJson);
    const int before = iBinModelPrime;
    if (normal) BS_spbNormalClick();                                            // golden cBinSel.cpp:2800
    else        BS_spbPrimeClick();                                             // golden cBinSel.cpp:2822
    const int after = iBinModelPrime;
    const bool wrote = (after != before);                                       // golden 只有 WritePrimeDara 會改 iBinModelPrime（開頭 return 保證 before!=Mode）
    std::printf("editlist.save BinSelect op=%s -> %s (iBinModelPrime %d -> %d)\n", op.c_str(),
                wrote ? "wrote Binasgn.Data [BinModel] bPrime" : "not written", before, after);

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    w.Key("executed").Bool(wrote);
    w.Key("before").Number((wb_int64)before);
    w.Key("after").Number((wb_int64)after);
    w.Key("file").String(szDir.c_str());
    w.Key("goldenLine").String(normal ? "V912 cBinSel.cpp:2800-2820 spbNormalClick -> WritePrimeDara :6107-6116"
                                      : "V912 cBinSel.cpp:2822-2842 spbPrimeClick -> WritePrimeDara :6107-6116");
    w.Key("session").RawValue(filerw::SessionJson());
    w.Key("proxies").RawValue(filerw::ProxyStateJson("TfBinSel"));
    w.Key("binAfter").RawValue(BinJson());
    w.EndObject();
    *ack = w.Str();
    return 200;
}

// WS editlist.save tag=BinSelect：value 整包 JSON 字串
//   {"widgets":{名稱:{…}}, "answers":{…}, "actions":["btnSettingSpecificBinClick"]?,
//    "bin":{"tags":[{"tag":n, "panel":{…}, "lists":{…}?}]}?}
int FileRW_BinSelect_Save(const std::string& valueJson, std::string* ack, std::string* err)
{
    if (!g_booted) { *err = "BinSelect edit lists are not booted"; return 409; }
    if (!g_shown || g_shownLevel != AccessLevel) {
        *err = "reload page: open the page (editlist.get BinSelect = golden FormShow) with the current access level before saving";
        return 409;
    }
    cJSON* root = cJSON_Parse(valueJson.c_str());
    if (!root || !cJSON_IsObject(root)) { if (root) cJSON_Delete(root); *err = "value is not a JSON object"; return 400; }
    if (const cJSON* jop = cJSON_GetObjectItemCaseSensitive(root, "op")) {     //AI(W906-FRW-S99) 20260926: Normal／Prime 鈕（見 FileRW_BinSelect_PrimeClick）
        const std::string op = (cJSON_IsString(jop) && jop->valuestring) ? jop->valuestring : "";
        const cJSON* ja0 = cJSON_GetObjectItemCaseSensitive(root, "answers");
        char* sa = (ja0 && cJSON_IsObject(ja0)) ? cJSON_PrintUnformatted(ja0) : nullptr;
        const std::string answers0 = sa ? sa : "{}";
        if (sa) cJSON_free(sa);
        cJSON_Delete(root);
        if (op != "spbNormalClick" && op != "spbPrimeClick") { *err = "op: only \"spbNormalClick\" / \"spbPrimeClick\" are supported"; return 400; }
        return FileRW_BinSelect_PrimeClick(op, answers0, ack, err);
    }
    const cJSON* jw = cJSON_GetObjectItemCaseSensitive(root, "widgets");
    const cJSON* ja = cJSON_GetObjectItemCaseSensitive(root, "answers");
    const cJSON* jact = cJSON_GetObjectItemCaseSensitive(root, "actions");
    const cJSON* jbin = cJSON_GetObjectItemCaseSensitive(root, "bin");
    if (!jw || !cJSON_IsObject(jw)) { cJSON_Delete(root); *err = "value must be {\"widgets\":{...},...}"; return 400; }
    Pending pend;
    bool openSpecificBin = false;
    if (jact) {
        if (!cJSON_IsArray(jact)) { cJSON_Delete(root); *err = "actions must be an array"; return 400; }
        for (const cJSON* a = jact->child; a; a = a->next) {
            if (!cJSON_IsString(a) || std::string(a->valuestring) != "btnSettingSpecificBinClick") {
                cJSON_Delete(root); *err = "actions: only \"btnSettingSpecificBinClick\" is supported"; return 400;
            }
            openSpecificBin = true;
        }
    }
    if (jbin) {
        const cJSON* tags = cJSON_IsObject(jbin) ? cJSON_GetObjectItemCaseSensitive(jbin, "tags") : nullptr;
        if (!tags || !cJSON_IsArray(tags)) { cJSON_Delete(root); *err = "bin must be {\"tags\":[...]}"; return 400; }
        for (const cJSON* t = tags->child; t; t = t->next) {
            PendingPanel pp;
            std::string e;
            if (!cJSON_IsObject(t) || !ParsePanel(t, &pp, &e)) { cJSON_Delete(root); *err = e.empty() ? "bin.tags[] must be objects" : e; return 400; }
            for (std::size_t i = 0; i < pend.panels.size(); ++i)
                if (pend.panels[i].tag == pp.tag) { cJSON_Delete(root); *err = "bin.tags[]: tag sent twice"; return 400; }
            // 只驗會套用的那種情況（可改的 tag）；不可改的 tag SaveFlow 會整組丟掉
            if (TagEditable(pp.tag) && !ValidateTrayEdits(pp, fBinSel->MyBinPanel[pp.tag], &e)) { cJSON_Delete(root); *err = e; return 400; }
            pend.panels.push_back(pp);
        }
    }
    char* s1 = cJSON_PrintUnformatted(jw);
    char* s2 = (ja && cJSON_IsObject(ja)) ? cJSON_PrintUnformatted(ja) : nullptr;
    const std::string widgets = s1 ? s1 : "{}";
    const std::string answers = s2 ? s2 : "{}";
    if (s1) cJSON_free(s1);
    if (s2) cJSON_free(s2);
    cJSON_Delete(root);

    // golden btnSettingSpecificBinClick（:6166）：使用者打開了 Specific Bin 面板（它底下的 sgSpecificBin／cbOutSht…／
    // cbIndexDrop… 才改得到）。golden 是切換；這裡只在伺服器端目前是關的時候重播「打開」那一下，避免重送時又關掉。
    if (openSpecificBin && fBinSel->palSpecificBin->Visible == false) BS_btnSettingSpecificBinClick();

    g_pending = pend;
    g_pending.active = true;
    std::string pageAck;
    const int st = filerw::PageSave(kPage, widgets, answers, &pageAck, err);
    Pending res = g_pending;
    g_pending = Pending();
    if (st != 200) return st;

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("tag").Number((wb_int64)res.savedTag);
    w.Key("name").String(res.savedTag >= 0 ? MetaOfTag(res.savedTag)->name : "");
    w.Key("file").String(res.savedTag >= 0 ? ReadFileOf(res.savedTag) : "");
    w.Key("applied").Bool(res.applied);
    if (!res.reason.empty()) w.Key("reason").String(res.reason);
    w.Key("ignoredTags").BeginArray();
    for (std::size_t i = 0; i < res.ignoredTags.size(); ++i) w.String(res.ignoredTags[i]);
    w.EndArray();
    w.Key("passwordRefused").BeginArray();
    for (std::size_t i = 0; i < res.passwordRefused.size(); ++i) w.String(res.passwordRefused[i]);
    w.EndArray();
    w.Key("listsDiffer").BeginArray();
    for (std::size_t i = 0; i < res.listsDiffer.size(); ++i) w.String(res.listsDiffer[i]);
    w.EndArray();
    w.EndObject();
    const std::size_t end = pageAck.find_last_of('}');
    if (end == std::string::npos) { *err = "internal: PageSave returned no object"; return 500; }
    // 存檔後（golden spbSaveClick 的 ReadFile 重讀）的 7 組值一起回給頁面，頁面不必再開一次頁
    *ack = pageAck.substr(0, end) + ",\"bin\":" + w.Str() + ",\"binAfter\":" + BinJson() + "}";
    return 200;
}
