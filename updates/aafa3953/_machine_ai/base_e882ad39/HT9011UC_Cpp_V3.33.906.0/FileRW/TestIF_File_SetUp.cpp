// ===========================================================================
//  FileRW/TestIF_File_SetUp.cpp -- TestIF_File 的 TfSetup 半邊（C 形狀：具名替身）
//  （<recipe>\HandlerCondition.Data；存檔順帶 Contact.Data 與 Temperature.Data [ATC]）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_SetUp.py（每條 replace／blocks 附原因）。
//
//  golden TfSetup（cSetUp.cpp，912）由 tools/gen_editlist.py 轉成 TestIF_File_SetUp.gen.inc：
//    建構子（:123）＝ 32 個 site 下拉／列欄標籤／rgSensor1..N 的替身陣列、ScrollBar1->Max；
//    FormShow（:1615）＝開頁（讀檔器＋DoIniDataToForm＋ScrollBar1Change＋權限）；
//    sbUpdateClick（:3492）＝存檔鈕（CHSetError／CheckShuttlePitch／RTC-OCR 密碼 → SaveSetupFile → Contact 重讀 →
//    SECS → SetWorkParameter）。沒有 HTEditList：存檔流程讀的替身全部是 mustSend。
//  結構名不是 TestIF_File：那個 tag 已被 A 形狀 FileRW/TestIF_File.cpp（kBridge_TfSetup）佔用。
//
//  讀檔器接移植樹 fSetup->ReadFile()（原因見設定檔檔頭），所以開機仍由 wb_serve 的 fSetup->Init()＋ReadFile() 讀檔；
//  本檔的 FileRW_Setup_Boot() 只做 golden 建構子（替身），必須在 fSetup->Init() 之後（tSiteMap 由它建）。
//
//  存檔前重播的 golden 事件（Steven 團隊 20260925，PageDesc::beforeApply → BeforeApply()）：
//    頁面送來的值與伺服器端不同時，golden 是「使用者改了 → 觸發事件」，事件會改別的元件的可見／選項：
//      ScrollBar1（Test Mode）  → ScrollBar1Change（:232）：夾限、CompChange（:1203）依新模式重建 Site 格子
//      CoSocketCombo           → CoSocketComboChange（:4785）：前 atoi(Text) 顆 rgSensor 看得見
//      六顆 Site Map 排序鈕     → btnLUpToRDownNClick（:4200）：排序＋IniConfig.iSiteMapDirection（SaveSetupFile :3851 寫）
//    事件跑完才判斷哪些元件可改、套頁面值（新模式的格子、新顯示的 rgSensor 才收得到頁面值）；其餘元件照舊
//    「頁面最後狀態、不觸發事件」。排序鈕的通道：widgets 裡 "<鈕名>":{"click":true}（頁面 ht9045_setup_c_wire.js
//    送最後按的那一顆）—— 走 editlist.save 的 widgets，wb_serve.cpp 不用改；可不可按照替身的可見／可改判斷
//    （golden FormShow :1880-1890 bSiteMappingFastSetDisable 會藏起來），按不到的照通用規則進 ack.ignored。
// ===========================================================================
#include "Public/cJSON.h"             // 放在產生檔之前：產生檔的 #define（ReadFile、GetTestMode…）不能碰到這兩個 header
#include "WebBridge/JsonWriter.h"

#include "FileRW/TestIF_File_SetUp.gen.inc"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"

void W906_Main_sbSetupClickTail();   // AI(W906-FRW-S100) 20260926: FileRW/MainClick.cpp（golden TfMain::sbSetupClick main.cpp:28472-28497）；全域範圍宣告（放進匿名 namespace 會變成另一支）

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// 存檔流程讀的替身：產生器掃出的（kSU_SaveReads）＋ 經指標陣列讀的（產生器的掃描看不到）：
//   TestSiteCH[r][c]（cbAa..cbDh，SaveSetupFile／CHSetError／sbUpdateClick）、MyTempRGBox[i]（rgSensor1..N，SaveSetupFile）。
//   BuildReads() 在 SU_CreateSaveProxies() 之後呼叫（要用 ELFind 判型別）。
std::vector<std::string> g_readNames;
std::vector<const char*> g_reads;

void BuildReads()
{
    // 容器（gbRTC／gbOcr：golden 存檔只讀 ->Enabled，是開頁 FormShow 設的伺服器端狀態，不是頁面值）不列必送
    for (const char* n : kSU_SaveReads) {
        TControl* c = filerw::ELFind("TfSetup", n);
        if (dynamic_cast<TGroupBox*>(c) || dynamic_cast<TPanel*>(c) || dynamic_cast<TTabSheet*>(c)) continue;
        g_readNames.push_back(n);
    }
    for (int r = 0; r < MAX_SOCKET_ROW; ++r)
        for (int c = 0; c < MAX_SOCKET_COL; ++c) {
            std::string s("cb");
            s += char('A' + r);
            s += char('a' + c);
            g_readNames.push_back(s);
        }
    for (int i = 0; i < iSnSocketCnt; ++i) g_readNames.push_back("rgSensor" + std::to_string(i + 1));
    for (const std::string& s : g_readNames) g_reads.push_back(s.c_str());
}

// golden 全域表 SiteData[]（每個 Test Mode 的 X×Y 格數，CompChange :1203 依它決定哪些 Site 格子看得見、幾個 CH）。
// golden 在 SYSTEM_MODULAR 建構子（database.cpp:48）呼叫 InitialMemory()（cmydef.cpp:5824）填這張表（:5879-5898）；
// 移植樹那個建構子整段 #if 0（移植樹 database.cpp:139 TODO(wave)），wb_serve 也沒有呼叫 InitialMemory()（它還會
// ZeroMemory 一大堆執行期陣列，開機晚期呼叫會把已讀的值清掉）→ SiteData 全 0 → CompChange 一個格子都不顯示 →
// CHSetError（:1292）回 "Test Site Not Select"，Setup 頁永遠存不了檔（實測 20260925：FormShow 時 SiteData[13]=0x0）。
// 這裡只補這張常數表，逐行照 golden；表若已被別人填過（SingleSite.Cnt!=0）就不動。
// ⚠ SiteData 是全域：填了之後移植樹其他讀它的程式（Command.cpp、forms/fLotInfo.cpp、FileRW/TestIF_File.cpp）也拿到 golden 值。
void SeedSiteData()
{
    if (SiteData[SingleSite].Cnt != 0) return;
    SiteData[SingleSite].SetData(1, 1);
    SiteData[DualSite].SetData(2, 1);
//    SiteData[DualSiteBS].SetData(2, 1);
    SiteData[TriSite1X3].SetData(3, 1);
    SiteData[QualSite1X4].SetData(4, 1);
    SiteData[DualSite2x1].SetData(1, 2);
    SiteData[QualSite2X2].SetData(2, 2);
    SiteData[QualSite2X2N].SetData(2, 2);                                       //Frank 20200520 2X2NN Mode
//    SiteData[QualSite2X2BS].SetData(2, 2);
    SiteData[_6Site2X3].SetData(3, 2);
    SiteData[_6Site2X3N].SetData(3, 2);                                         //Steven 20220425 : 2X3NN Mode
    SiteData[_8Site2X4].SetData(4, 2);
    SiteData[_10Site2X5].SetData(5, 2);
    SiteData[_12Site2X6].SetData(6, 2);
    SiteData[_16Site2X8].SetData(8, 2);
    SiteData[_16Site4X4].SetData(4, 4);                                         //Sam 20190226 : 16Site4X4
    SiteData[_32Site4X8N].SetData(8, 4);
    SiteData[_32Site4X8M].SetData(8, 4);
    SiteData[_8Site1X4].SetData(4, 1);
    SiteData[_8Site2X4N].SetData(4, 2);                                         //Wei 20231211 : 2X4NN Mode
    std::printf("FileRW TestIF_File_SetUp: SiteData[] seeded from golden InitialMemory (cmydef.cpp:5879-5898) -- "
                "port SYSTEM_MODULAR ctor is gated, InitialMemory() never ran\n");
}

// 六顆 Site Map 排序鈕（golden cSetUp.dfm，OnClick 全部＝btnLUpToRDownNClick，方向＝Tag）
const char* const kSortBtn[6] = {
    "btnLUpToRDownZ",   // Tag 0（DFM 沒寫 Tag）  cSetUp.dfm:1907  Left_Top To Right     → iSiteMapDirection 5
    "btnLDownToRUpZ",   // Tag 1                 cSetUp.dfm:2010  Left_Bottom To Right  → 10
    "btnRUpToLDownZ",   // Tag 2                 cSetUp.dfm:2114  Right_Top To Left     → 9
    "btnRDownToLUpZ",   // Tag 3                 cSetUp.dfm:2218  Right_Bottom To Left  → 6
    "btnLUpToRDownN",   // Tag 4                 cSetUp.dfm:1803  Left_Top To Bottom    → 7
    "btnRUpToLDownN",   // Tag 5                 cSetUp.dfm:2322  Right_Top To Bottom   → 11
};
bool IsSortBtn(const char* n)
{
    for (const char* b : kSortBtn) if (n && std::strcmp(n, b) == 0) return true;
    return false;
}

// 頁面送回的下拉值由 ELApplyProxies 以裸欄位寫進替身（ItemIndex／Text 不連動）→ golden 存檔前補上 VCL
// csDropDownList 的不變式（ItemIndex 在 -1..Count-1、Text＝Items[ItemIndex]），同產生檔的 SU_CbSetIndex：
// 頁面選到伺服器端沒有的選項（例：網頁 Site 格子的 CH 數比 golden CompChange 多）＝ VCL 選不到 → -1。
void NormalizeCombos()
{
    for (int r = 0; r < MAX_SOCKET_ROW; ++r)
        for (int c = 0; c < MAX_SOCKET_COL; ++c) SU_CbSetIndex(TestSiteCH[r][c], TestSiteCH[r][c]->ItemIndex);
    TComboBox* co = EL<TComboBox>("TfSetup", "CoSocketCombo");
    SU_CbSetIndex(co, co->ItemIndex);
}

// PageDesc::beforeApply（見檔頭與 _EditPage.h）：主 session 20260925 決斷——ScrollBar1 與開頁值不同 → 先照 golden
// 觸發 ScrollBar1Change（重建格子）再套 site 格子的值；CoSocketCombo 不同 → 先跑 CoSocketComboChange 再判斷 rgSensor
// 能不能改、套值；排序鈕 {"click":true} → btnLUpToRDownNClick。順序：捲軸 → CoSocketCombo → 排序鈕（排序要作用在
// 新模式的格子上；排序設的 ItemIndex 之後仍被頁面送來的格子值蓋過＝頁面最後狀態，留下的是 iSiteMapDirection）。
// 每一個都先看替身現在可不可改（同 PageSave 丟值的判斷）；不可改的不處理，留給通用規則丟掉（ack.ignored）。
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root) return;
    // (1) golden TScrollBar：使用者拉動 → Position 變（VCL 夾在 Min..Max）→ OnChange=ScrollBar1Change
    const cJSON* sb = cJSON_GetObjectItemCaseSensitive(root, "ScrollBar1");
    const cJSON* pos = (sb && cJSON_IsObject(sb)) ? cJSON_GetObjectItemCaseSensitive(sb, "position") : nullptr;
    if (pos && cJSON_IsNumber(pos) && filerw::ELEditable("TfSetup", "ScrollBar1")) {
        filerw::ELTrackBar* bar = EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1");
        const int before = bar->Position;
        int v = pos->valueint;                      // VCL 先把 Position 夾在 Min..Max，值有變才觸發 OnChange
        if (v < (int)bar->Min) v = bar->Min;
        if (v > (int)bar->Max) v = bar->Max;
        // 有事件就整個交給 golden：ScrollBar1Change 自己會再改 Position（iMax 夾限、跳過不允許的模式），
        // 最後的 Position 是 golden 的結果，不能再被頁面的原始值蓋掉 → 列入 handled。
        // 夾限後沒變＝沒有事件：留給通用規則（SetPosition(v, false) 一樣夾回原值）。
        if (v != before) {
            bar->Position = pos->valueint;          // ELTrackBar::SetPosition(v, true)
            handled->push_back("ScrollBar1");
        }
    }
    // (2) golden TComboBox（csDropDownList）：使用者選別的項 → ItemIndex／Text 變 → OnChange=CoSocketComboChange
    const cJSON* co = cJSON_GetObjectItemCaseSensitive(root, "CoSocketCombo");
    const cJSON* idx = (co && cJSON_IsObject(co)) ? cJSON_GetObjectItemCaseSensitive(co, "itemIndex") : nullptr;
    if (idx && cJSON_IsNumber(idx) && filerw::ELEditable("TfSetup", "CoSocketCombo")) {
        TComboBox* cb = EL<TComboBox>("TfSetup", "CoSocketCombo");
        if (idx->valueint != cb->ItemIndex) {
            SU_CbSetIndex(cb, idx->valueint);
            SU_CoSocketComboChange();
            handled->push_back("CoSocketCombo");
        }
    }
    // (3) golden TSpeedButton OnClick=btnLUpToRDownNClick（Sender＝那一顆，方向＝Tag）；照頁面送來的順序
    for (const cJSON* it = root->child; it; it = it->next) {
        if (!IsSortBtn(it->string)) continue;
        const cJSON* click = cJSON_IsObject(it) ? cJSON_GetObjectItemCaseSensitive(it, "click") : nullptr;
        if (!cJSON_IsTrue(click) || !filerw::ELEditable("TfSetup", it->string)) continue;
        SU_btnLUpToRDownNClick(EL<TSpeedButton>("TfSetup", it->string));
        handled->push_back(it->string);
    }
    cJSON_Delete(root);
}

// PageDesc::saveFlow：golden 存檔鈕 sbUpdateClick，前面補 VCL 下拉不變式（見 NormalizeCombos）
void SaveFlow()
{
    NormalizeCombos();
    SU_sbUpdateClick();
    // AI(W906-FRW-S100) 20260926: RULINGS_20260926 S100 —— golden 主畫面 TfMain::sbSetupClick（V912 main.cpp:28457）在 fSetup->ShowModal()
    //   回來（Setup 視窗關掉）之後跑的尾段 :28472-28497（DoStructUnitConvert、SetWorkParameter…；_8Site1X4 時改寫 Contact.Data 4 鍵，
    //   本體與怪行為說明在 FileRW/MainClick.cpp W906_Main_sbSetupClickTail）。網頁沒有「關頁」事件，照 S88（FileRW/Temperature.cpp
    //   MainTempOffsetTail）的做法：
    //     (1) A02 權限不足 —— golden sbUpdateClick :3498 Close() → 視窗關掉 → 尾段：與 golden 相同。
    //     (2) 真的寫了檔（SaveSetupFile）—— golden 存完視窗還開著，要等 Exit 才跑尾段；網頁 Exit 不送伺服器，改在存完就跑。
    //   差別：golden「開 Setup、沒存就關」也會跑尾段（_8Site1X4 時照樣寫那 4 鍵）；這裡不跑。觸發時機待 Steven（交件報告 S100）。
    if (filerw::ELMarked("closed") || filerw::ELMarked("SaveSetupFile"))
        W906_Main_sbSetupClickTail();                                           // golden main.cpp:28472
}

// PageDesc::extraJson：golden CompChange（:1203）執行期重建的 Site 下拉選項（"- - -"、"CH 1".."CH n"；看不見的格子是空的）。
// 頁面（ht9045_setup_c_wire.js）照這份重建 cbAa..cbDh 的選項 —— DFM 的選項（"1".."8"）不是 golden 執行期的選項。
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("items").BeginObject();
    for (int r = 0; r < MAX_SOCKET_ROW; ++r)
        for (int c = 0; c < MAX_SOCKET_COL; ++c) {
            std::string n("cb");
            n += char('A' + r);
            n += char('a' + c);
            TStringList* it = TestSiteCH[r][c]->Items;
            w.Key(n).BeginArray();
            for (int i = 0; i < it->Count; ++i) w.String(it->Strings[i].c_str());
            w.EndArray();
        }
    w.EndObject();
    w.Key("iSiteMapDirection").Number((wb_int64)IniConfig.iSiteMapDirection);   // golden 記憶體值（ReadFile :2326 讀檔）
    w.EndObject();
    return w.Str();
}

// 沒寫檔時把替身還原成檔案值：golden FormClose（:3363）的 ReadFile() ＋ DoIniDataToForm()（FormClose 其餘的 Auto Site Mapping
// 清除／SetRunStartMode 是關表單的機台副作用，不在「還原替身」的範圍，不跑）。BeforeApply 重播過的事件也由這裡還原：
// DoIniDataToForm 設回檔案的 Test Mode（值變了 → ScrollBar1Change 重建格子）、CoSocketCombo／rgSensor；讀檔器重讀
// SiteMapDirection（移植樹 cSetUp.cpp:543）。
void Reload()
{
    SU_PortReadFile();
    SU_DoIniDataToForm();
}

filerw::PageDesc g_page = {
    "TestIF_File_SetUp", "TfSetup", "Setup.SetUp.html",
    nullptr, nullptr, 0,
    kSU_SaveReads, (int)(sizeof(kSU_SaveReads) / sizeof(kSU_SaveReads[0])),
    &SU_FormShow, &SaveFlow, "SaveSetupFile", &Reload, &Booted,
    &BeforeApply, &ExtraJson,
};
filerw::PageRegistrar g_reg(&g_page);
}  // namespace

// golden TfSetup 建構（HT9045.cpp:184 CreateForm）。前提：fSetup->Init() 已跑（tSiteMap）。
void FileRW_Setup_Boot()
{
    if (g_booted) return;
    SeedSiteData();
    // ScrollBar1：golden TScrollBar（DFM Max=8、OnChange=ScrollBar1Change）。產生器把 TScrollBar 當 TControl，
    // 這裡先建成 filerw::ELTrackBar（VCL 語意：Position 夾在 Min..Max、值變了觸發 OnChange），之後產生碼的
    // EL<TControl>("TfSetup","ScrollBar1") 拿到的是同一個物件。
    {
        filerw::ELTrackBar* sb = EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1");
        sb->DfmInit(0, 8, 0);
        sb->OnChange = &SU_ScrollBar1Change;
    }
    SU_DfmItems();
    SU_DfmState();
    SU_TfSetup();
    // rgSensor1..N（golden 建構子動態產生，DFM 沒有）的容器：golden Parent=scrlbxSocketSensor（DFM 父＝grpSocketSensor）
    {
        static std::string kids[iSnSocketCnt];
        static const char* pairs[iSnSocketCnt][2];
        for (int i = 0; i < iSnSocketCnt; ++i) {
            kids[i] = "rgSensor" + std::to_string(i + 1);
            pairs[i][0] = kids[i].c_str();
            pairs[i][1] = "scrlbxSocketSensor";
        }
        EL<TControl>("TfSetup", "scrlbxSocketSensor");
        filerw::ELSetParents("TfSetup", pairs, iSnSocketCnt);
    }
    SU_CreateSaveProxies();
    SU_CreateContainerProxies();
    // 六顆排序鈕的 DFM Tag（btnLUpToRDownNClick 讀 Sender->Tag 當方向）；產生器的 DFM 設計期狀態不帶 Tag
    for (int k = 0; k < 6; ++k) EL<TSpeedButton>("TfSetup", kSortBtn[k])->Tag = k;
    BuildReads();
    g_page.saveReads = g_reads.data();
    g_page.nSaveReads = (int)g_reads.size();
    std::printf("FileRW TestIF_File_SetUp: TfSetup proxies ready (%d save reads) -- golden cSetUp.cpp ctor :123\n",
                g_page.nSaveReads);
    g_booted = true;
}

// ---- golden fContact->DutCount()／ReadFile()：移植樹 fContact 是 TfContactShim（atester_shims.h），
//      真表單是 fContactForm（forms/fContact.h:1634）。
#undef ReadFile   // 設定檔 members 的 #define ReadFile SU_PortReadFile 只給 golden 轉出來的程式用
#include "forms/fContact.h"

void FileRW_Setup_ContactDutCount() { fContactForm->DutCount(); }   // golden cSetUp.cpp:3633
void FileRW_Setup_ContactReadFile() { fContactForm->ReadFile(); }   // golden cSetUp.cpp:3642
