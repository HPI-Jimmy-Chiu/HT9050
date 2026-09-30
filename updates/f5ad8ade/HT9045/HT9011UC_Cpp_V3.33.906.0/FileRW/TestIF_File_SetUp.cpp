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
#include "WebReauth.h"             //AI(W906-Q45-B5) 20260930: W906_ReauthOpenJson（下面 ExtraJson 的 extra.auth；本體 WebLogin.cpp 檔尾）；佔用原本的空行
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"

void W906_Main_sbSetupClickTail();  void FileRW_Setup_B8Su7BeforeApply(const cJSON* root, std::vector<std::string>* handled);   //AI(W906-B8-SU7) 20260930 [W906]: SU-7 存檔前重查（本體檔尾；BeforeApply 在匿名 namespace 裡，宣告要放全域）；同一行附加  // AI(W906-FRW-S100) 20260926: FileRW/MainClick.cpp（golden TfMain::sbSetupClick main.cpp:28472-28497）；全域範圍宣告（放進匿名 namespace 會變成另一支）

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
    }   /* AI(W906-Q41) 20260927 (St02-E): (4) Q41 SU-1..SU-6 —— golden DFM 912 cSetUp.dfm 的 OnClick 綁定照 (1)(2) 同規則重播（替身可改、頁面值與伺服器不同 → 套值＋跑 golden 事件、列入 handled）；放在 (1) 之後（模式已換好） */ auto q41Ck = [&](const char* id, bool* v) -> bool { const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, id); const cJSON* c = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr; if (!c || !cJSON_IsBool(c)) return false; *v = cJSON_IsTrue(c) != 0; return true; }; auto q41Ix = [&](const char* id, int* v) -> bool { const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, id); const cJSON* c = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "itemIndex") : nullptr; if (!c || !cJSON_IsNumber(c)) return false; *v = c->valueint; return true; }; auto q41Rg = [&](const char* id, void (*ev)()) { int v = 0; if (!q41Ix(id, &v) || !filerw::ELEditable("TfSetup", id)) return; TRadioGroup* g = EL<TRadioGroup>("TfSetup", id); if (v == g->ItemIndex || v < 0 || v >= g->Items->Count) return; g->ItemIndex = v; ev(); handled->push_back(id); };
    { bool bN = false, bO = false; if (q41Ck("rgShtModeNormal", &bN) && q41Ck("rgShtModeOneSide", &bO) && filerw::ELEditable("TfSetup", "rgShtModeNormal") && filerw::ELEditable("TfSetup", "rgShtModeOneSide")) { TRadioButton* rN = EL<TRadioButton>("TfSetup", "rgShtModeNormal"); TRadioButton* rO = EL<TRadioButton>("TfSetup", "rgShtModeOneSide"); if (bN != rN->Checked || bO != rO->Checked) { rN->Checked = bN; rO->Checked = bO; SU_rgShtModeNormalClick(); handled->push_back("rgShtModeNormal"); handled->push_back("rgShtModeOneSide"); } }   /* SU-1 golden 912 cSetUp.cpp:1272（DFM :70、:80 兩顆 TRadioButton 同一支）*/ q41Rg("rgUseSuckMode", &SU_rgUseSuckModeClick);   /* SU-2 golden :3447（DFM :562）；SU-1 設成不可改時照通用規則丟值 */ struct Q41Ev { const char* id; void (*ev)(); }; static const Q41Ev kQ41[] = { {"Arm1PickArm2Test", &SU_Arm1PickArm2TestClick}, {"cbArm1UseHeat", &SU_Arm1PickArm2TestClick}, {"cbArm1OnlyPlaceArm2TestAndSuck", &SU_Arm1PickArm2TestClick}, {"cbUseSLKClamp", &SU_cbUseSLKClampClick}, {"cbUseTesterDry", &SU_cbUseSLKClampClick}, {"chkOffCenterkit", &SU_chkOffCenterkitClick}, {"chkUseXCenterPitch", &SU_chkOffCenterkitClick}, {"cbEnablePreciser", &SU_chkOffCenterkitClick}, {"cbEnabledPreciserRT", &SU_chkOffCenterkitClick} };   /* SU-3 :4497（DFM :1446／:1471／:1535，golden 三顆都只看 Arm1PickArm2Test）、SU-4 :4670（DFM :1591／:925，都只看 cbUseSLKClamp）、SU-5 :4523（DFM :134／:818／:874／:901） */ for (const Q41Ev& e : kQ41) { bool b = false; if (!q41Ck(e.id, &b) || !filerw::ELEditable("TfSetup", e.id)) continue; TCheckBox* c = EL<TCheckBox>("TfSetup", e.id); if (b != c->Checked) { c->Checked = b; e.ev(); handled->push_back(e.id); } } const AnsiString yOfsBefore = EL<TEdit>("TfSetup", "edYOffset")->Text; const size_t nBefore = handled->size(); q41Rg("rgYPitchOffsetMode", &SU_rgYPitchOffsetModeClick); const cJSON* yo = cJSON_GetObjectItemCaseSensitive(root, "edYOffset"); const cJSON* yt = (yo && cJSON_IsObject(yo)) ? cJSON_GetObjectItemCaseSensitive(yo, "text") : nullptr; if (handled->size() > nBefore && yt && cJSON_IsString(yt) && AnsiString(yt->valuestring) == yOfsBefore) handled->push_back("edYOffset"); }   /* SU-6 :4812（DFM :770）：golden 換模式就改 edYOffset->Text（NN＝10、其他＝檔案值）；頁面沒動過 edYOffset（與開頁值相同）→ 留 golden 設的值 */  FileRW_Setup_B8Su7BeforeApply(root, handled);   /* AI(W906-B8-SU7) 20260930 [W906]：(5) SU-7 RTC 等 6 個勾選框存檔前重查 golden cbEnableRealTimeCCDClick（cSetUp.cpp:4364；檔尾）；同一行附加 */  cJSON_Delete(root);
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
    w.Key("auth").RawValue(W906_ReauthOpenJson("TestIF_File_SetUp", !(CosFunction.bLockRTCByFile && IniConfig.bRTC_Enable==false) && EL<TGroupBox>("TfSetup", "gbRTC")->Enabled, bNeedPassword, !(CosFunction.bLockRTCByFile && IniConfig.bRTC_Enable==false) && EL<TGroupBox>("TfSetup", "gbOcr")->Enabled, bOCRNeedPassword));  w.EndObject();   //AI(W906-Q45-B5) 20260930: Q45 甲 editlist.get 的 extra.auth（設計 q45-web-password.md 3.2.2-①；頁面 ht9045_setup_c_wire.js 看它決定按 Save 時要不要跳登入框）：rtcOff／ocrOff 的 armed＝golden sbUpdateClick :3567-3574 扣掉勾選值（沒被 [RTC Lock by file] 鎖、gbRTC／gbOcr 可用，REAL_TIME_CCD 在 W906_ReauthOpenJson 裡看），armedAtOpen＝golden bNeedPassword／bOCRNeedPassword（開頁 DoIniDataToForm :3176／:3191 設）；同一行
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
    SU_CreateContainerProxies();  { void FileRW_Setup_EvB10ABoot(); FileRW_Setup_EvB10ABoot(); }  { void FileRW_Setup_B8Su7Boot(); FileRW_Setup_B8Su7Boot(); }   //AI(W906-B8-SU7) 20260930 [W906]: SU-7 6 個勾選框的替身＋golden DFM OnClick 登記（檔尾）；同一行附加  //AI(W906-EVB10A) 20260929 [W906]: SU-9 form.event 控制項 sbtExit 的替身（檔尾）；接在同一行
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

// ===========================================================================
//  AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a SU-9（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
//    Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」、20260929「照 BCB 的邏輯」）。
//  golden 關 Setup 視窗有兩條路（cSetUp.dfm 沒改 BorderIcons ⇒ 有 ✕）：
//    (1) Exit 鈕 sbtExit → sbtExitClick（cSetUp.cpp:3476-3490）：AUTO_SENSOR_INSTALL && bSaveNeedHome ⇒ ShowMyMessage("Auto Shuttle Sensor
//        Need Reset!!")、**不關**；否則 OCR lot、bNeedEnterPassword=true、Close() → FormClose（:3363）、fMain->GetCZSiteMap(false)。
//        ⇒ WS form.event（頁面 D:\HT9045\web\page\ht9045_setup_c_wire.js 攔 sbtExit 的 click，送 {"control":"sbtExit","event":"click"}）；
//          處理器由產生器照 golden 轉（TestIF_File_SetUp.gen.inc 檔尾 SU_sbtExitClick、kSU_Events；設定 tools/editlist/TestIF_File_SetUp.py
//          的 events／_EXIT_REPLACE），Close() 那一行＝記 "closed"＋跑 SU_FormClose ⇒ ack.closed＝true 頁面才關視窗。
//    (2) 視窗的 ✕ → 只有 FormClose（:3363；它開頭也查 bSaveNeedHome，查到就 ShowMyMessage 然後 return —— golden 的 ✕ 這時視窗照樣關、
//        只是 ReadFile／fShow=false／Auto Site Map 那段沒跑）。
//        ⇒ 頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge，表在 FileRW/WindowEdgeTails.h）呼叫 FileRW_Setup_WindowEdge：
//          這一次開窗 golden FormShow 跑過、而且 (1) 還沒跑過 FormClose（filerw::PageShownNow）才跑 SU_FormClose。運轉中不跑（R86 同理：
//          golden Setup 是 ShowModal、運轉中打不開）。golden 的訊息（ELMessage）由分派那一邊印到主控台（視窗已經不在了）。
//  ⚠ 跟 golden 不同（寫明）：網頁的 ✕ 與沒有載入 ht9045_setup_c_wire.js 的 Exit 都走 (2)；(1) 被拒時視窗留著，但操作員再按 ✕ 就關得掉
//    （golden 也一樣：✕ 不看 bSaveNeedHome 能不能關）。主畫面的尾段 TfMain::sbSetupClick :28472-28497（W906_Main_sbSetupClickTail）照
//    S100／R85 留在存檔之後，關窗不再跑一次。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

namespace {
filerw::PageEventsRegistrar g_evreg("TestIF_File_SetUp", kSU_Events, (int)(sizeof(kSU_Events) / sizeof(kSU_Events[0])));
}  // namespace

// FileRW_Setup_Boot（SU_CreateContainerProxies 同一行）：form.event 控制項的替身。sbtExit 只出現在 golden 處理器那一行（已轉成 ";"），
//   產生器不建它的替身；RunPageEvent 找不到替身會回 handler-failed（同 FileRW/TestIF_File_YieldMonitoring.cpp EvBoot 的理由）。
//   DFM 父層 Panel2（cSetUp.dfm:1171 > :1319）已在產生的 kSU_ParentOf 裡。
void FileRW_Setup_EvB10ABoot()
{
    EL<TSpeedButton>("TfSetup", "sbtExit");                                    // golden cSetUp.h TSpeedButton *sbtExit
    for (std::size_t i = 0; i < sizeof(kSU_Events) / sizeof(kSU_Events[0]); ++i)
        if (!filerw::ELFind("TfSetup", kSU_Events[i].control))
            std::printf("FileRW TestIF_File_SetUp: WARNING form.event control %s has no proxy (add it to FileRW_Setup_EvB10ABoot)\n",
                        kSU_Events[i].control);
}

const char* FileRW_Setup_WindowEdge(bool open)
{
    static std::string s_what;
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfSetup proxies are not booted";
    if (!filerw::PageShownNow("TestIF_File_SetUp"))
        return "not run: golden FormShow did not run in this window-open, or the Exit button (form.event sbtExitClick) / A02 save already ran golden FormClose";
    SU_FormClose();                                                             // golden cSetUp.cpp:3363（✕：VCL Close → OnClose）
    s_what = filerw::ELMarked("closeRefused")
        ? "ran golden TfSetup::FormClose (cSetUp.cpp:3363): CC_ASE_KaohSiung CHSetError -> golden Action=caNone (would keep the form open) -- the web window is already closed"
        : (AUTO_SENSOR_INSTALL && bSaveNeedHome)
            ? "ran golden TfSetup::FormClose (cSetUp.cpp:3363): AUTO_SENSOR_INSTALL && bSaveNeedHome -> golden ShowMyMessage(\"Auto Shuttle Sensor Need Reset!!\") and return (ReadFile / Auto Site Map part not run, same as golden)"
            : "ran golden TfSetup::FormClose (cSetUp.cpp:3363): ReadFile, fShow=false, Auto Site Map clean-up when ASM is off, bNeedPassword=false";
    return s_what.c_str();
}

// ===========================================================================
//  //AI(W906-B8-SU7) 20260930 [W906]：B8 SU-7（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「SU-7」；Steven 20260928
//    「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）—— RTC 等 6 個勾選框（Setup.SetUp）的點擊檢查與改回。
//  golden V912 cSetUp.cpp:4364-4399 TfSetup::cbEnableRealTimeCCDClick；cSetUp.dfm 6 個元件的 OnClick 都綁它（kSu7Boxes，DFM 順序）。
//    處理器由產生器轉（tools/editlist/TestIF_File_SetUp.py 的 _SU7_*，產生檔 SU_cbEnableRealTimeCCDClick、kSU_Events 的 6 列）：
//      (a) :4366-4375 取消 RTC、[D55] 開著且 [M01-09] 沒開 ⇒ 改回勾選、ShowMyMessage("Must cancel ... [D55] ...")、return
//      (b) :4377-4388 RTC 正在跑（COM2->bCCDDummyRum==false）：IndexStatus!=Z1_Z2_Normal ⇒ 「* Turn off need Z1 Z2 up!」＋改回勾選；
//          否則 COM2->SendCommToVision(rtInspEnd) —— 這一半沒有移植（RTC 視覺序列埠，B8 P-4），產生檔記 ack.todo、不送
//      (c) :4390-4397 RTC 關著：fMain->CheckCanChangeRealDummy()==false（cMainStatus.cpp:323 的活本體：Plate1／2、Shuttle、Index、
//          In／Out Arm 有料）⇒ 改回不勾＋「* Turn On need Clean-Out !」。⚠ 不是 Automation/auto9045.cpp:207-216 那個「先回 true」替身。
//    SOFT_SIMULTE：讀檔器把 bCCDDummyRum 固定成 true（golden cSetUp.cpp:2811-2812，移植樹 cSetUp.cpp:1056）⇒ SIM 只會走 (c)。
//  兩條進來的路：
//    ① 點擊當下：頁面 D:\HT9045\web\page\ht9045_setup_c_wire.js (8) 送 WS form.event {"form":"TfSetup","control":"<6 個之一>",
//       "event":"click","checked":<新值>}（tag "TestIF_File_SetUp"）→ RunPageEvent 先設 Checked（VCL 使用者點一下＝Toggle）再跑處理器；
//       ack.changed 帶回改回的勾選與 lbShowMessage 的字。
//    ② 存檔時再查一次（下面 FileRW_Setup_B8Su7BeforeApply，BeforeApply (5)）：頁面沒送 form.event 就按 Save（舊頁面、或直接改勾選）時，
//       editlist.save 會把 RTC 勾選直接寫進替身、跳過 (a)(b)(c)。照 TA-3（FileRW/TrayForm.cpp BeforeApply (1)）的做法：頁面值跟伺服器端
//       不同 ⇒ 照 VCL 點一下＋跑 golden 處理器，被改回就記一句訊息（golden 只有 lbShowMessage，頁面存完會重讀、FormShow :1899 把它藏起來）。
//       這是機台狀態（有沒有料、Index Z 位置）的互鎖 —— 不信任前端，存檔一定重查。
//  VCL「程式設 Checked 值有變也觸發 OnClick」（B8 P-8、R118，FileRW/_EditList.h ELClickChecked）：處理器裡改回勾選那三行會再進一次處理器
//    （值相等就停）。⚠ golden 在 [D55] 開、[M01-09] 沒開、RTC 關著、機台有料時兩條改回互相觸發、永遠停不下來（(a) 設 true → (c) 設 false →
//    (a) 設 true …；golden BCB6 會一路遞迴到堆疊用完）。移植樹不能讓 wb_serve 當掉 ⇒ 巢狀超過 kSu7MaxNest 層就停下來、把勾選設回機台目前的
//    RTC 狀態（!bCCDDummyRum，不觸發 OnClick）、記訊息與 todo。正常的 golden 路徑最多巢狀 2 層（例 (a) → (b) 或 (c) 值相等就停）。
//  ⚠ 跟 golden 不同（寫明，不在這一列做）：
//    * golden 其他地方程式設這 6 格也會觸發這一支 —— DoIniDataToForm :3175（開頁 FormShow、關頁 FormClose、本檔 Reload）、sbUpdateClick
//      :3579／:3602（Q45 重新登入失敗把 RTC 勾回去；B5 SU_DoPassword 那一段）。它們在移植樹是純設值（沒有開整個結構的 'vcl_clicks'，
//      開了 Q41 SU-1～6 的開頁／存檔也會多跑事件）。那幾次 golden 處理器的效果只有 lbShowMessage 與送 rtInspEnd（沒移植）。
//    * form.event 運轉中一律拒收（FileRW/_FormEvent.cpp:77-81）；golden Setup 是 ShowModal、運轉中打不開，同一個結果。
//  測試：ctest B8_Su7_RtcClick（tests/test_b8_su7_rtcclick.cpp）。
// ===========================================================================
#include <algorithm>

namespace {
const char* const kSu7Boxes[6] = {"cbOutUseBackRow", "cbInUseBackRow", "cbEnableRealTimeCCD",       // golden cSetUp.dfm:1052／:1082／:1121
                                  "cbOcrFunction", "cbdisibleinitialcheck", "cbSocketSensor"};   // :1559／:1654／:1669
const int kSu7MaxNest = 4;   // 巢狀 OnClick 的上限（golden 正常路徑最多 2 層）
int g_su7Nest = 0;

// VCL TCustomCheckBox.SetState 值有變 → Click → OnClick＝cbEnableRealTimeCCDClick（golden DFM 6 格同一支，Sender 不讀）
void Su7VclOnClick(TControl*)
{
    if (g_su7Nest >= kSu7MaxNest) {
        TCheckBox* rtc = EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD");
        const bool now = !COM2->bCCDDummyRum;
        rtc->Checked = now;                                                     // 純設值：不再觸發 OnClick
        filerw::ELMessage(AnsiString("Enable Real Time CCD: BCB6 cbEnableRealTimeCCDClick keeps putting the box back and forth here ([D55] on, "
                                     "[M01-09] off, RTC off, IC in the machine: golden recurses without end). Stopped; the box is left at the "
                                     "machine's current RTC state (") + (now ? "ON" : "OFF") + "). Clean out the machine or cancel [D55] first.",
                          AnsiString("Enable Real Time CCD：BCB6 cbEnableRealTimeCCDClick 在這裡會一直來回改勾選（[D55] 開、[M01-09] 沒開、RTC 關、"
                                     "機台有料：golden 會無限遞迴）。已停止，勾選留在機台目前的 RTC 狀態（") + (now ? "開" : "關") +
                          "）。請先 Clean-Out 或取消 [D55]。");
        filerw::ELTodo("golden cSetUp.cpp:4371 / :4394 cbEnableRealTimeCCDClick re-enters itself without end (VCL SetState -> Click) when [D55] is on, "
                       "[M01-09] is off, RTC is off and the machine has IC -- stopped after kSu7MaxNest nested OnClicks, box = !COM2->bCCDDummyRum");
        std::printf("FileRW TestIF_File_SetUp: SU-7 cbEnableRealTimeCCDClick nested %d times -- golden endless recursion stopped, RTC box = %d\n",
                    g_su7Nest, (int)now);
        return;
    }
    struct Nest { Nest() { ++g_su7Nest; } ~Nest() { --g_su7Nest; } } nest;
    SU_cbEnableRealTimeCCDClick();                                              // golden cSetUp.cpp:4364
}
}  // namespace

// FileRW_Setup_Boot（SU_CreateContainerProxies 同一行）：6 個勾選框的替身與 golden DFM 的 OnClick（VCL 從 DFM 載入時就綁好）。
//   只有 ELClickChecked（處理器裡改回勾選的三行）會觸發它；頁面送的值（ELApplyProxies）與 form.event 控制項自己的值不經過 OnClick
//   （使用者點的那一下由事件表上的處理器跑一次，_EditList.h 的規則）。
void FileRW_Setup_B8Su7Boot()
{
    for (const char* id : kSu7Boxes) filerw::ELSetOnClick(EL<TCheckBox>("TfSetup", id), &Su7VclOnClick);
}

// BeforeApply (5)（頁面值跟伺服器端不同的才重播；點不到的留給 PageSave 丟掉＝ack.ignored；型別不對留給 PageSave 回 400）
void FileRW_Setup_B8Su7BeforeApply(const cJSON* root, std::vector<std::string>* handled)
{
    if (!root || !handled) return;
    bool ran = false;
    for (const char* id : kSu7Boxes) {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, id);
        const cJSON* c = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
        if (!c || !cJSON_IsBool(c) || !filerw::ELOperable("TfSetup", id)) continue;
        TCheckBox* x = EL<TCheckBox>("TfSetup", id);
        const bool want = cJSON_IsTrue(c) != 0;
        if (x->Checked == want) continue;       // 值一樣＝golden 沒有這一下（頁面送過 form.event 時，伺服器端已經是處理器跑過的值）
        x->Checked = want;                       // VCL 使用者點一下：Toggle → OnClick
        SU_cbEnableRealTimeCCDClick();           // golden cSetUp.cpp:4364
        handled->push_back(id);
        ran = true;
    }
    if (!ran) return;
    // 處理器會改 cbEnableRealTimeCCD（點別的勾選框也會，例 (a)／(c)）⇒ 它最後的值是 golden 的結果，不再被頁面送來的值蓋掉
    const cJSON* ro = cJSON_GetObjectItemCaseSensitive(root, "cbEnableRealTimeCCD");
    const cJSON* rc = (ro && cJSON_IsObject(ro)) ? cJSON_GetObjectItemCaseSensitive(ro, "checked") : nullptr;
    if (!rc || !cJSON_IsBool(rc)) return;
    if (std::find(handled->begin(), handled->end(), std::string("cbEnableRealTimeCCD")) == handled->end())
        handled->push_back("cbEnableRealTimeCCD");
    const bool page = cJSON_IsTrue(rc) != 0;
    TCheckBox* rtc = EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD");
    if (rtc->Checked == page) return;
    TLabel* lb = EL<TLabel>("TfSetup", "lbShowMessage");
    const AnsiString why = lb->Visible ? lb->Caption : AnsiString("");
    filerw::ELMessage(AnsiString("Setup save: Enable Real Time CCD was put back to ") + (rtc->Checked ? "ON" : "OFF") +
                          " by BCB6 cbEnableRealTimeCCDClick (cSetUp.cpp:4364)" + (why.IsEmpty() ? AnsiString("") : AnsiString(": ") + why),
                      AnsiString("Setup 存檔：Enable Real Time CCD 照 BCB6 cbEnableRealTimeCCDClick（cSetUp.cpp:4364）改回") +
                          (rtc->Checked ? "勾選" : "不勾") + (why.IsEmpty() ? AnsiString("") : AnsiString("：") + why));
}
