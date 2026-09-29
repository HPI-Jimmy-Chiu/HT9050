// ===========================================================================
//  FileRW/ContactForce.cpp -- golden TfContactForce（ContactForce.cpp，V912）的 C 路入口：Setup.ContactForce.html 的「讀寫段」。
//  D:\HT9045\system\ContactInfo.ini（系統檔，不跟配方）＋ Gerneral.ini [System] EP 鍵／[Test Arm] dIndexZOffset ＋ SaveLastSetIni。
//
//  //AI(W906-FRW-S57) 20260926: 新檔（Steven 團隊，S57；RULINGS_20260926 檔尾 S57「ContactInfo.ini＋Gerneral.ini 9 鍵＋SLK 表（含 P4
//    dIndexZOffset 讀）」）。設定：tools/editlist/ContactForce.py（每條 replace 附原因）；動態面板：FileRW/ContactForce_Panels.h。
//    未 build、未 e2e（Steven S51）；語法檢查 g++ -fsyntax-only 模擬＋出貨兩組態。頁面補件（web/page/ht9045_contactforce_c.js）尚未做
//    （Steven：網頁建頁往後排、先做讀寫檔），建頁備忘見本檔最後一節。
//
//  golden 方法由 tools/gen_editlist.py 轉成 ContactForce.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    建構子（:421）       ＝ golden CreateForm(TfContactForce)（HT9045.cpp:220）：[SLK Type] CSV → 四個面板 vector、EP 三鍵、
//                            `if(bHasFile) ReadFile(); else WriteFile();`。開機就跑（FileRW_ContactForce_Boot → EnsureCreated；
//                            R15＝B，RULINGS_20260926 S162；S57 原本延到第一次開頁，見下「開機」）。
//    FormShow（:686）     ＝ 開頁（editlist.get）：ReadFile＋頁籤可見／權限＋NS 切換＋tb*／tbD25／edD25 從記憶體（IniConfig／LastSet）填。
//    btSaveClick（:936）  ＝ 存檔鈕（editlist.save）：iContactForceMap／LastSet.dIndexLoadRate ← 畫面 → SaveLastSetIni → WriteFile → ReadFile。
//    ReadFile（:966）     ＝ Gerneral.ini EP 12 鍵（EP_Install!=0）＋[Test Arm] dIndexZOffset 30 鍵（[2][*] 固定階梯）＋ContactInfo.ini 每一筆。
//    WriteFile（:1183）   ＝ ContactInfo.ini 全部段＋LastSet.dIndexLoadRate（記憶體）＋Gerneral.ini EP 8 鍵＋[Test Arm] 30 鍵 → ReadFile。
//  savedMark＝"CF_SaveLastSetIni"（btSaveClick 沒有 return；只有本檔 SaveFlow 的越界拒存會在它之前 return）。
//  reload＝golden FormShow（golden FormClose :917 不讀檔，而且是機台動作：ShowArmAndDeviceForce／DeviceForm.dPress／ADAM_WriteVoltage）。
//
//  ---- 與上一位工程師計畫（scratchpad contactforce/plan.md）不同的地方（以現況為準）-------------------------------------------------
//    * 共用表（ContactForceTables()，adam6024／FileRW/DeviceForm_File.cpp 讀的那一份）**不從本頁的面板發佈**，改成本頁寫檔之後
//      重跑底層載入器 LoadContactForceTables()（ContactForceLoad.cpp，906 的資料半邊）讀剛寫好的檔。理由：RULINGS_20260926 第 26 條
//      「底層照 906、畫面照 912；畫面寫進底層共用結構的欄位以底層為準」—— V912 建構子的 SLKIndClass 跟 [SLK Type]（:453），906 載入器
//      跟 [SLK Type Ind]；從面板發佈會讓「有沒有人開過這一頁」決定機台用哪一份 Ind 表。兩邊經同一個檔同步：同名段（例 Diameter_30.000mm_3）
//      的值一致，SLKClass／DieForce 兩份完全相同（同一個 [SLK Type] CSV）。→ 決策題（見 S57 報告）。
//    * 多寫者：Gerneral.ini EP 四鍵（HSys 也寫）採「沒動過的欄位不把舊值蓋回去」（見 Overlap），不是只警告。→ 決策題，kCF_KeepNewerOverlap 可關。
//
//  ---- 開機與換配方（golden 呼叫點）---------------------------------------------------------------------------------------------
//    開機：golden CreateForm(TfContactForce)（HT9045.cpp:220，THandlerSystem :210／TfYieldMonitoring :215 之後）→ 建構子。
//          FileRW_ContactForce_Boot()（tools/wb_serve.cpp，FileRW_HSys_Boot() 同一行後面）先建替身（DFM 設計期狀態＋存檔讀的替身＋容器），
//          接著跑 golden 建構子本體（EnsureCreated）。開機的底層共用表仍是之後的 LoadContactForceTables()（wb_serve.cpp，InitialHandler
//          之後；S57 在它裡面補了 golden ReadFile :1014-1036 的 dIndexZOffset（P4）與建構子 :542-544 的 EP 三鍵），它讀的就是建構子剛補過
//          ／建好的 ContactInfo.ini。
//AI(W906-FRW-S162) 20260927 [W906] R15＝B（Steven，RULINGS_20260926 S162；S57 決策題 1）：建構子本體從「第一次開頁」改回 golden 時序
//          （開機 CreateForm）。多出來的開機行為：頁面面板物件（四個 vector）開機就建；bUseDynamicKitDiameter 時依 912 [SLK Type]
//          （不是 906 載入器跟的 [SLK Type Ind]）在 D:\HT9045\system\ContactInfo.ini 補 Ind 段預設鍵（這台 [SLK Type]=30,40,60,56,80 →
//          40／60／56／80 各 16 段＝64 段 × LoadRate／ContactOffset），沒有檔時 WriteFile 建檔。細節（含 golden 沒檔時的 EP 8 鍵寫法）見
//          FileRW_ContactForce_Boot() 的註解。S57 原本的安全預設（延到第一次開頁）拿掉；EnsureCreated 留著給「開機沒跑過」的保底。
//    換配方：不讀（ContactInfo.ini 不跟配方；golden DoReadLastData／ChangeSetUpFile 沒有 fContactForce）。
//    golden iosetview.cpp:3451 Tfiosetview::Label120Click 也呼叫 fContactForce->ReadFile()（IO 檢查頁的隱藏標籤）：移植樹沒有那一頁的
//    C 路，這裡提供 FileRW_ContactForce_ReadFile() 給之後接（建構子沒跑過時先跑建構子）。
//
//  ---- 不在本檔（機台動作 → Jimmy；網頁端只顯示 todo）--------------------------------------------------------------------------
//    ADAM_WriteVoltage（btSaveClick :963、FormClose :924、btExitClick :932）、ADAM_DirectWriteData（tb30mm_10kgChange :1381：
//    golden 拖 tb*mm_*kg 或開頁設 Position 時就把數值直接輸出到 EP）、FormClose 的 fContact->ShowArmAndDeviceForce＋DeviceForm.dPress。
// ===========================================================================
#include "FileRW/ContactForce.gen.inc"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "ContactForce.h"            // ContactForceTables()：底層共用表（906 載入器的容器）
#include "ContactForceLoad.h"        // LoadContactForceTables()
#include "FileRW/_EditPage.h"
#include "WebBridge/JsonWriter.h"

// D-P1（FileRW/ContactForce_Panels.h 檔頭）：golden 每個面板 trackbar 的 OnChange 都只更新自己的 edtLoadRate*；全部重算一次效果相同
static void CF_PanelTrackBarsChanged()
{
    for (std::size_t i = 0; i < SLKClass.size(); ++i) { SLKClass[i]->trckbrDiameter_Change(); SLKClass[i]->trckbrDiameter_NSChange(); }
    for (std::size_t i = 0; i < SLKIndClass.size(); ++i) SLKIndClass[i]->trckbrDiameterInd_Change();
    for (std::size_t i = 0; i < DieForceSLKClass.size(); ++i) DieForceSLKClass[i]->trckbrDieForceDiameter_Change();
    for (std::size_t i = 0; i < DieForceOneByOneSLKClass.size(); ++i) DieForceOneByOneSLKClass[i]->trckbrDieForceOneByOneDiameter_Change();
}

namespace {
bool g_booted = false;     // FileRW_ContactForce_Boot（替身）做完
bool g_created = false;    // golden 建構子（CF_TfContactForce）跑過
bool Booted() { return g_booted; }

// ---- 多寫者：Gerneral.ini [System] EP 四鍵（HSys 的 THandlerSystem::SaveSystemSet 也寫，FileRW/HSys.gen.inc WriteIniDataGeneral）----
// golden 兩張表單都在開頁（FormShow）時讀檔、存檔時寫「畫面上的值」—— 頁面開著的期間另一頁存了新值，本頁再存會把開頁時的舊值蓋回去。
// golden 的 HandlerSystem 是 ShowModal、單一操作者，機會很小；網頁可以同時開好幾個分頁，機會變大（golden 沒有的風險）。
// 安全預設（kCF_KeepNewerOverlap=true）：存檔時「這一欄頁面沒改過（＝開頁時的值）而檔案已經被別人改了」→ 用檔案現在的字串寫回（等於不動），
//   ack messages 說明；頁面改過而檔案也被改了（雙方都改）→ 照 golden 寫頁面值（後寫的贏）並警告。false＝照 golden（寫頁面值）＋警告。
// _1032 四鍵與 [Test Arm] 30 鍵只有本頁寫（golden grep 20260926：dIndexZOffset 只在 ContactForce.cpp 寫；EP_*_1032 移植樹只有本頁與
//   ContactForceLoad.cpp 讀），不套這條規則。
const bool kCF_KeepNewerOverlap = true;
struct Overlap {
    const char* widget;
    const char* key;
    AnsiString openText;     // 開頁（FormShow 之後）替身的字
    AnsiString openFile;     // 開頁時檔案裡的字
    bool openHad;
};
Overlap g_ovl[4] = {
    {"edMaxKpa",    "EP_MAXKPA",        "", "", false},
    {"edMaxMpaFB",  "EP_MAXA",          "", "", false},
    {"edMinMpa",    "EP_MINMPA",        "", "", false},
    {"edtMinMpaFB", "EP_MINA_FeedBack", "", "", false},
};
const char* const kMissing = "<<W906-FRW-S57 missing>>";

// 純讀（不補鍵）：Gerneral.ini 是 write-through（vclcompat/IniFiles.cpp:299），讀到的是檔案現況
AnsiString GeneralRaw(const char* key, bool* had)
{
    const AnsiString v = ReadIniData(asGeneralPath, AnsiString("System"), AnsiString(key), AnsiString(kMissing));
    *had = (v != kMissing);
    return *had ? v : AnsiString("");
}

void TakeSnapshot()
{
    for (Overlap& o : g_ovl) {
        o.openText = EL<TEdit>("TfContactForce", o.widget)->Text;
        o.openFile = GeneralRaw(o.key, &o.openHad);
    }
}

void ApplyOverlapRule()
{
    for (Overlap& o : g_ovl) {
        bool hadNow = false;
        const AnsiString now = GeneralRaw(o.key, &hadNow);
        if (!o.openHad || !hadNow) continue;
        if (now == o.openFile || std::fabs(atof(now.c_str()) - atof(o.openFile.c_str())) < 1e-12) continue;   // 檔案沒被別人改
        TEdit* e = EL<TEdit>("TfContactForce", o.widget);
        const bool touched = !(e->Text == o.openText);
        AnsiString en, zh;
        if (!touched && kCF_KeepNewerOverlap) {
            en.sprintf("Gerneral.ini [System] %s was changed by another page after this page was opened (%s -> %s); "
                       "this page did not change it, so the newer value %s is kept", o.key, o.openFile.c_str(), now.c_str(), now.c_str());
            zh.sprintf("Gerneral.ini [System] %s 在本頁開啟後被其他頁面改過（%s → %s）；本頁沒有改這一欄，保留較新的 %s，不把舊值蓋回去",
                       o.key, o.openFile.c_str(), now.c_str(), now.c_str());
            e->Text = now;
        } else if (!touched) {
            en.sprintf("Gerneral.ini [System] %s was changed by another page (%s -> %s) and is overwritten with the value shown when this page "
                       "was opened (%s), as golden does", o.key, o.openFile.c_str(), now.c_str(), e->Text.c_str());
            zh.sprintf("Gerneral.ini [System] %s 已被其他頁面改成 %s，本頁照 golden 寫回開頁時的 %s", o.key, now.c_str(), e->Text.c_str());
        } else {
            en.sprintf("Gerneral.ini [System] %s was changed by another page (%s -> %s) and also on this page (%s); this page's value is written "
                       "(last writer wins, as golden)", o.key, o.openFile.c_str(), now.c_str(), e->Text.c_str());
            zh.sprintf("Gerneral.ini [System] %s 其他頁面改成 %s、本頁也改成 %s：照 golden 寫本頁的值（後存的為準）",
                       o.key, now.c_str(), e->Text.c_str());
        }
        filerw::ELMessage(en, zh);
    }
}

// 底層共用表：本頁寫檔之後重跑 906 載入器（見檔頭「與上一位工程師計畫不同的地方」）。它也重讀 EP_* 與 dIndexZOffset（同一個檔，值與
// 本頁 ReadFile 剛讀的一樣）。
void RefreshShared(const char* why)
{
    const int n = LoadContactForceTables();
    SlkForceTables& t = ContactForceTables();
    std::printf("FileRW ContactForce: LoadContactForceTables() after %s -> %d entries (SLK=%u Ind=%u DieForce=%u DieForce1by1=%u, bLoaded=%d)\n",
                why, n, (unsigned)t.SLKClass.size(), (unsigned)t.SLKIndClass.size(), (unsigned)t.DieForceSLKClass.size(),
                (unsigned)t.DieForceOneByOneSLKClass.size(), (int)t.SLKClass.bLoaded);
}

// ---- mustSend：產生器掃出的 24 個（kCF_SaveReads）＋ golden WriteFile 經指標讀、產生器掃不到的 ----
//   IndexZOffsetEdit[0..1][0..14]（:1372，ReadFile :972-980 指到 edtArm1Offset_01…edtArm2Offset_15）與四種面板的
//   trackbar／offset 欄（:1285-1293、:1306-1308、:1320-1322、:1333-1335、:1351-1353；條件照 golden WriteFile）。
//   面板是建構子之後才有 → EnsureCreated 之後重算一次。
std::vector<std::string> g_mustNames;
std::vector<const char*> g_must;
filerw::PageDesc* g_page = nullptr;

void RebuildMustSend()
{
    g_mustNames.clear();
    for (const char* n : kCF_SaveReads) g_mustNames.push_back(n);
    for (int i = 1; i <= 2; ++i)
        for (int j = 1; j <= 15; ++j) {
            char b[32];
            std::snprintf(b, sizeof(b), "edtArm%dOffset_%02d", i, j);
            g_mustNames.push_back(b);
        }
    if (CosFunction.bUseDynamicKitDiameter) {                                   // golden :1191
        for (THTSLKClass* p : SLKClass)
            for (const auto& r : p->ids)
                if (r.first == "trckbrDiameter" || r.first == "trckbrDiameter_NS" || r.first == "edtHotOffset" ||
                    r.first == "edtContactOffset" || r.first == "edtContactOffset_NS")
                    g_mustNames.push_back(r.second);
        for (THTSLKIndClass* p : SLKIndClass)
            for (const auto& r : p->ids)
                if (r.first == "trckbrDiameterInd" || r.first == "edtContactOffsetInd") g_mustNames.push_back(r.second);
        if (INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI)                               // golden :1296／:1312
            for (THTDieForceOneByOneSLKClass* p : DieForceOneByOneSLKClass)
                for (const auto& r : p->ids)
                    if (r.first == "trckbrDieForceOneByOneDiameter" || r.first == "edtDieForceOneByOneContactOffset")
                        g_mustNames.push_back(r.second);
    }
    if (INSTALL_DOUBLE_EP == 1 || INSTALL_DOUBLE_EP == DOUBLE_EP_MULTI)         // golden :1346
        for (THTDieForceSLKClass* p : DieForceSLKClass)
            for (const auto& r : p->ids)
                if (r.first == "trckbrDieForceDiameter" || r.first == "edtDieForceContactOffset") g_mustNames.push_back(r.second);
    g_must.clear();
    for (const std::string& s : g_mustNames) g_must.push_back(s.c_str());
    if (g_page) { g_page->saveReads = g_must.data(); g_page->nSaveReads = (int)g_must.size(); }
}

// golden 建構子（開機 FileRW_ContactForce_Boot 就跑；R15＝B，見檔頭「開機」）。回 true＝這一次才建。when＝誰叫的（log 用）。
//AI(W906-FRW-S162) 20260927 [W906] 加 when 參數：開機之後其他呼叫點（開頁／存檔／iosetview ReadFile）只剩保底，正常不會再建
bool EnsureCreated(const char* when)
{
    if (g_created) return false;
    g_created = true;
    CF_TfContactForce();
    RebuildMustSend();
    std::printf("FileRW ContactForce: golden TfContactForce ctor (ContactForce.cpp:421) ran at %s -- bUseDynamicKitDiameter=%d "
                "bHasFile=%d SLK=%u Ind=%u DieForce=%u DieForce1by1=%u (mustSend %d)\n",
                when, (int)CosFunction.bUseDynamicKitDiameter, (int)bHasFile, (unsigned)SLKClass.size(), (unsigned)SLKIndClass.size(),
                (unsigned)DieForceSLKClass.size(), (unsigned)DieForceOneByOneSLKClass.size(), (int)g_must.size());
    return true;
}

// PageDesc::formShow：golden FormShow（建構子開機已跑；沒跑過才先跑 —— 保底）
void FormShowFlow()
{
    const bool created = EnsureCreated("first page open (fallback: boot did not run it)");
    CF_FormShow();
    // 建構子沒有檔時會 WriteFile 建檔；FormShow 的 ReadFile 在 KYEC 30→28／60→58 轉換時也會 WriteFile（:1142）→ 底層表跟著新檔
    if (created || filerw::ELMarked("WriteFile")) RefreshShared(created ? "ctor (first page open, fallback)" : "FormShow WriteFile (KYEC remap)");
    TakeSnapshot();
}

// PageDesc::reload：golden 沒寫檔（本檔 SaveFlow 拒存）→ 替身還原成檔案／記憶體值（golden FormShow）
//AI(W906-FRW-S90) 20260926: 不重拍 EP 四鍵快照 —— 快照只在開頁與寫檔之後（同 FileRW/HSys.cpp 的理由：reload 後重拍，頁面再送時
//   「開頁時的值」會變成別人的新值，多寫者規則失效）
void Reload()
{
    CF_FormShow();
}

// PageDesc::saveFlow：golden btSaveClick（前後各一段本檔的守衛）
void SaveFlow()
{
    filerw::ELMark("CF_SaveFlow");
    EnsureCreated("save (fallback: boot did not run it)");
    // 越界拒存（偏離 golden，見 tools/editlist/ContactForce.py 的 WriteFile 守衛）：golden 在 SaveLastSetIni 之後的 WriteFile 才會當，
    //   這裡在寫任何檔之前就擋（連 config.ini 都不寫），比 golden「寫了一半才當」保守
    if (CosFunction.bUseDynamicKitDiameter) {
        const unsigned int need = (bHasFile == false) ? 4u : ((EP_Install == 5) ? 8u : 4u);
        if (SLKClass.size() < need) {
            AnsiString en, zh;
            en.sprintf("ContactForce save refused: ContactInfo.ini [SLK Type] has %u valid diameters, golden WriteFile needs at least %u "
                       "(golden ContactForce.cpp:1195-1258 would crash)", (unsigned)SLKClass.size(), need);
            zh.sprintf("ContactForce 存檔拒絕：ContactInfo.ini [SLK Type] 有效口徑只有 %u 個，golden WriteFile 至少要 %u 個（golden 在這裡會當機）",
                       (unsigned)SLKClass.size(), need);
            filerw::ELMessage(en, zh);
            return;
        }
    }
    ApplyOverlapRule();
    CF_btSaveClick();
    RefreshShared("save");
    TakeSnapshot();
}

void PanelJson(webbridge::JsonWriter& w, const std::vector<std::pair<std::string, std::string> >& ids, const AnsiString& sDiameter,
               int iTag, bool bShow, double dDiameter, double dMinForce, double dMaxForce)
{
    w.Key("sDiameter").String(sDiameter.c_str());
    w.Key("iTag").Number((wb_int64)iTag);
    w.Key("bShow").Bool(bShow);
    w.Key("dDiameter").Number(dDiameter);
    w.Key("dMinForce").Number(dMinForce);
    w.Key("dMaxForce").Number(dMaxForce);
    w.Key("ids").BeginObject();
    for (const auto& r : ids) w.Key(r.first).String(r.second);
    w.EndObject();
}

void ListJson(webbridge::JsonWriter& w, const char* name, TStringList* sl)
{
    w.Key(name);
    if (!sl) { w.Null(); return; }
    w.BeginArray();
    for (int i = 0; i < sl->Count; ++i) w.String(sl->Strings[i].c_str());
    w.EndArray();
}

// PageDesc::extraJson：通用 proxies 帶不到的東西 —— 頁面依 panels 建四個 ScrollBox 裡的動態面板（替身的值／可見／可改在 proxies）
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("created").Bool(g_created);
    w.Key("fileName").String(FileName.c_str());
    w.Key("bHasFile").Bool(bHasFile);
    w.Key("bUseDynamicKitDiameter").Bool(CosFunction.bUseDynamicKitDiameter);
    w.Key("EP_Install").Number((wb_int64)EP_Install);
    w.Key("INSTALL_DOUBLE_EP").Number((wb_int64)INSTALL_DOUBLE_EP);
    w.Key("iIndEPCnt").Number((wb_int64)iIndEPCnt);
    w.Key("customerCode").Number((wb_int64)CUSTOMER_CODE);
    // golden 的 TStringList（建構子 :494-500／:604-624；slSLKType／slSLKTypeVisible 建構子尾端就 delete 了，同 golden 為 null）
    w.Key("lists").BeginObject();
    ListJson(w, "slSLKType", slSLKType);
    ListJson(w, "slSLKTypeVisible", slSLKTypeVisible);
    ListJson(w, "slSLKTypeInd", slSLKTypeInd);
    ListJson(w, "slSLKTypeIndVisible", slSLKTypeIndVisible);
    ListJson(w, "slDieForceSLKType", slDieForceSLKType);
    ListJson(w, "slDieForceSLKTypeVisible", slDieForceSLKTypeVisible);
    ListJson(w, "slDieForceOneByOneSLKType", slDieForceOneByOneSLKType);
    ListJson(w, "slDieForceOneByOneSLKTypeVisible", slDieForceOneByOneSLKTypeVisible);
    w.EndObject();
    // 四種動態面板（golden 父容器：scrlbxDynamicKit／scrlbxDynamicKitInd／scrlbxDieForceDynamicKit／scrlbxDieForceOneByOneDynamicKit）
    w.Key("panels").BeginObject();
    w.Key("SLKClass").BeginObject().Key("parent").String("scrlbxDynamicKit").Key("items").BeginArray();
    for (THTSLKClass* p : SLKClass) {
        w.BeginObject();
        PanelJson(w, p->ids, p->sDiameter, p->iTag, p->bShow, p->dDiameter, p->dMinForce, p->dMaxForce);
        w.Key("dLoadRate").Number(p->dLoadRate).Key("dLoadRate_NS").Number(p->dLoadRate_NS).Key("dHotOffset").Number(p->dHotOffset)
         .Key("dContactOffset").Number(p->dContactOffset).Key("dContactOffset_NS").Number(p->dContactOffset_NS);
        w.EndObject();
    }
    w.EndArray().EndObject();
    w.Key("SLKIndClass").BeginObject().Key("parent").String("scrlbxDynamicKitInd").Key("items").BeginArray();
    for (THTSLKIndClass* p : SLKIndClass) {
        w.BeginObject();
        PanelJson(w, p->ids, p->sDiameter, p->iTag, p->bShow, p->dDiameter, p->dMinForce, p->dMaxForce);
        w.Key("dLoadRate").Number(p->dLoadRate).Key("dContactOffset").Number(p->dContactOffset);
        w.EndObject();
    }
    w.EndArray().EndObject();
    w.Key("DieForceSLKClass").BeginObject().Key("parent").String("scrlbxDieForceDynamicKit").Key("items").BeginArray();
    for (THTDieForceSLKClass* p : DieForceSLKClass) {
        w.BeginObject();
        PanelJson(w, p->ids, p->sDiameter, p->iTag, p->bShow, p->dDiameter, p->dMinForce, p->dMaxForce);
        w.Key("dLoadRate").Number(p->dLoadRate).Key("dContactOffset").Number(p->dContactOffset);
        w.EndObject();
    }
    w.EndArray().EndObject();
    w.Key("DieForceOneByOneSLKClass").BeginObject().Key("parent").String("scrlbxDieForceOneByOneDynamicKit").Key("items").BeginArray();
    for (THTDieForceOneByOneSLKClass* p : DieForceOneByOneSLKClass) {
        w.BeginObject();
        PanelJson(w, p->ids, p->sDiameter, p->iTag, p->bShow, p->dDiameter, p->dMinForce, p->dMaxForce);
        w.Key("dLoadRate").Number(p->dLoadRate).Key("dContactOffset").Number(p->dContactOffset);
        w.EndObject();
    }
    w.EndArray().EndObject();
    w.EndObject();
    // 記憶體裡機台在用的值（本頁 ReadFile／btSaveClick 寫的全域；探針／除錯核對）
    w.Key("machine").BeginObject();
    w.Key("EP").BeginObject()
     .Key("EP_MAXKPA").Number(EP_MAXKPA).Key("EP_MAXAFB").Number(EP_MAXAFB).Key("EP_MINMPA").Number(EP_MINMPA).Key("EP_MinAFB").Number(EP_MinAFB)
     .Key("EP_MAXKPA_1032").Number(EP_MAXKPA_1032).Key("EP_MAXAFB_1032").Number(EP_MAXAFB_1032)
     .Key("EP_MINMPA_1032").Number(EP_MINMPA_1032).Key("EP_MinAFB_1032").Number(EP_MinAFB_1032)
     .Key("EPDual_MAXKPA").Number(EPDual_MAXKPA).Key("EPDual_MAXAFB").Number(EPDual_MAXAFB)
     .Key("EPDual_MINMPA").Number(EPDual_MINMPA).Key("EPDual_MinAFB").Number(EPDual_MinAFB)
     .EndObject();
    w.Key("dIndexZOffset").BeginArray();
    for (int i = 0; i < 3; ++i) {
        w.BeginArray();
        for (int j = 0; j < 15; ++j) w.Number(dIndexZOffset[i][j]);
        w.EndArray();
    }
    w.EndArray();
    w.Key("iContactForceMap").BeginArray();
    for (int i = 0; i < 4; ++i) w.BeginArray().Number((wb_int64)IniConfig.iContactForceMap[i][0]).Number((wb_int64)IniConfig.iContactForceMap[i][1]).EndArray();
    w.EndArray();
    w.Key("dIndexLoadRate").BeginArray();
    for (int i = 0; i < 3; ++i) {
        w.BeginArray();
        for (int j = 0; j < 4; ++j) w.Number(LastSet.dIndexLoadRate[i][j]);
        w.EndArray();
    }
    w.EndArray();
    w.EndObject();
    // 底層共用表（906 載入器，adam6024／Setup.Contact 用的那一份）
    {
        SlkForceTables& t = ContactForceTables();
        w.Key("shared").BeginObject()
         .Key("source").String("LoadContactForceTables() (ContactForceLoad.cpp, 906 data half; SLKIndClass follows [SLK Type Ind])")
         .Key("SLK").Number((wb_int64)t.SLKClass.size()).Key("Ind").Number((wb_int64)t.SLKIndClass.size())
         .Key("DieForce").Number((wb_int64)t.DieForceSLKClass.size()).Key("DieForce1by1").Number((wb_int64)t.DieForceOneByOneSLKClass.size())
         .Key("bLoaded").Bool(t.SLKClass.bLoaded)
         .Key("iSlkTypeIndTokens").Number((wb_int64)t.iSlkTypeIndTokens).Key("iDieForceTypeTokens").Number((wb_int64)t.iDieForceTypeTokens)
         .EndObject();
    }
    w.Key("overlap").BeginObject().Key("keepNewer").Bool(kCF_KeepNewerOverlap).Key("keys").BeginArray();
    for (const Overlap& o : g_ovl) w.String(o.key);
    w.EndArray().EndObject();
    w.Key("notWired").BeginArray()
     .String("golden ContactForce.cpp:963/:924/:932 ADAM_WriteVoltage and :1381 ADAM_DirectWriteData (EP output, machine action -> Jimmy)")
     .String("golden FormClose :917-924 fContact->ShowArmAndDeviceForce + DeviceForm.dPress (web page has no close event; Setup.Contact recomputes on its own open)")
     .String("golden never writes EPDual_MAXKPA/EPDual_MAXAFB/EPDual_MINMPA/EPDual_MinAFB (edMaxKpaDual.. are display-only; kept as golden)")
     .EndArray();
    w.EndObject();
    return w.Str();
}

filerw::PageDesc kPage = {
    "ContactForce", "TfContactForce", "Setup.ContactForce.html",
    nullptr, nullptr, 0,
    kCF_SaveReads, (int)(sizeof(kCF_SaveReads) / sizeof(kCF_SaveReads[0])),
    &FormShowFlow, &SaveFlow, "CF_SaveLastSetIni", &Reload, &Booted,
    nullptr, &ExtraJson,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfContactForce 建構（HT9045.cpp:220 CreateForm）：替身半邊（DFM 設計期狀態 → 存檔流程讀的替身 → 容器替身與父子）＋ golden 建構子
// 本體（EnsureCreated → CF_TfContactForce，V912 ContactForce.cpp:421-653）。冪等。
//AI(W906-FRW-S162) 20260927 [W906] R15＝B（Steven，RULINGS_20260926 S162）：建構子本體開機就跑（S57 原本「不讀不寫檔、延到第一次開頁」）。
//   呼叫點不變：tools/wb_serve.cpp 開機行 FileRW_HSys_Boot()（golden CreateForm(THandlerSystem) HT9045.cpp:210）之後、FileRW_ACTForm_Boot()
//   （TACTForm :221）之前 ＝ golden CreateForm 的相對位置，wb_serve 的呼叫不用改（本檔只在 wb_serve 連，ctest 不連）。
//   前提：LoadMachineConfig 之後 —— 建構子讀 CosFunction.bUseDynamicKitDiameter、CUSTOMER_CODE、EP_Install、INSTALL_DOUBLE_EP、iIndEPCnt、
//   IniConfig.bSPILFunction（HSys.ReadGeneralIni 填好）。TestIF_File.iTestMode（iIndEPCnt==8 那一支）此時還沒讀配方：golden 同
//   （DoReadLastData 在 TfMain::FormShow，所有 CreateForm 之後）。
//   會寫的檔（golden 行為，照翻，只在 bUseDynamicKitDiameter 時）：
//     * D:\HT9045\system\ContactInfo.ini（golden :428 字面路徑，--dry 不重導 —— 同 ContactForceLoad.cpp:26 的載入器）：
//       有檔 → ReadFile 的 CheckAndReadIniData 補缺鍵：[SLK Type] Type／Visible（＋INSTALL_DOUBLE_EP>0 的 DieForceType／DieForceVisible）、
//       Diameter_<d>mm 五鍵、DieForceDiameter_<d>mm 兩鍵，以及依 [SLK Type] 的 Ind 段 Diameter_<d>mm_<0..15>（MULTI：_Arm1_1..4／_Arm2_1..4）
//       的 LoadRate／ContactOffset。906 載入器跟的是 [SLK Type Ind]，所以 [SLK Type] 裡有、[SLK Type Ind] 沒有的口徑是這次新增的段。
//       沒有檔 → WriteFile 建檔（全部段）＋ Gerneral.ini [System] EP 8 鍵。CC_KYEC_LEE 的 30→28／60→58 轉換也會 WriteFile。
//     * Gerneral.ini（asGeneralPath，--dry 會重導）：ReadFile 的 EP 12 鍵（EP_Install!=0）＋[Test Arm] dIndexZOffset 30 鍵缺鍵補寫 ——
//       之後的 LoadContactForceTables() 本來就會補同一批，不是新增。
//     ⚠ golden 沒檔那一次（照翻，看起來錯）：WriteFile 寫的 EP 8 鍵是畫面替身的字 —— edMaxKpa_1032／edMaxMpaFB_1032／edMinMpa_1032
//       是建構子 :572-574 從 EP_MAXKPA_1032 等全域填的，那時全域還是 cmydef 初值 0（golden 只有 ReadFile :994-997 讀它們，而建構子沒檔時
//       ReadFile 在 WriteFile 尾端 :1375 才跑）⇒ Gerneral.ini EP_MAXKPA_1032／EP_MAXA_1032／EP_MINMPA_1032 會被寫成 0（原本有值也蓋掉）；
//       EP_MINA_FeedBack／EP_MINA_FeedBack_1032 寫 DFM 的 0.968。[Test Arm] 30 鍵因 IndexZOffsetEdit 還是 NULL 不寫（gen.inc 守衛，golden 這裡 AV）。
//   底層共用表：這裡不 RefreshShared —— 開機鏈稍後（InitialHandler 之後）的 LoadContactForceTables() 讀的就是剛補過／建好的檔。
//   建構子裡的 ELTodo／ELMark 開機沒有頁面 session 可回 → 開一個 session 收、印到 log、清掉（同 FileRW_BarCode_BootReadFile）。
void FileRW_ContactForce_Boot()
{
    if (g_booted) return;
    CF_DfmItems();
    CF_DfmState();
    CF_CreateSaveProxies();
    CF_CreateContainerProxies();
    // 產生器的父子表只收「golden 方法用到的替身」與它們的祖先；scrlbxDieForceOneByOneDynamicKit 沒有被任何 TfContactForce 方法直接用到
    // （只在面板建構子 :229-230 當 Parent）→ 照 golden DFM 補（其餘三個 ScrollBox 產生器已有，重設一次值相同）
    static const char* const kPanelParents[][2] = {
        {"scrlbxDynamicKit", "tsDynamicKit"},
        {"scrlbxDynamicKitInd", "TabSheet1"},
        {"scrlbxDieForceDynamicKit", "tsDieForceDynamicKit"},
        {"scrlbxDieForceOneByOneDynamicKit", "tsDieForceOneByOneKit"},
        {"tsDieForceOneByOneKit", "PageControl1"},
    };
    EL<TControl>("TfContactForce", "scrlbxDieForceOneByOneDynamicKit");        // golden TScrollBox（vclcompat 沒有 → TControl，同產生器）
    EL<TTabSheet>("TfContactForce", "tsDieForceOneByOneKit");
    filerw::ELSetParents("TfContactForce", kPanelParents, (int)(sizeof(kPanelParents) / sizeof(kPanelParents[0])));
    g_page = &kPage;
    RebuildMustSend();
    g_booted = true;
    std::printf("FileRW ContactForce: TfContactForce proxies ready (%d save reads before panels)\n", (int)g_must.size());
    //AI(W906-FRW-S162) 20260927 [W906] R15＝B：golden CreateForm(TfContactForce) HT9045.cpp:220 → 建構子本體，開機就跑（見上面註解）
    filerw::SessionBegin("");
    EnsureCreated("boot (golden CreateForm HT9045.cpp:220)");
    //   ⚠ 這兩個 SessionBegin 會清掉當下的 editlist session —— 只在開機呼叫（wb_serve 開機行）；FileRW_ContactForce_ReadFile 的保底呼叫
    //   只有在開機沒跑過本函式時才會走到這裡。WriteFile 的越界守衛（SLKClass 不足 4 筆）也會先記 "WriteFile" 再 return（todo 在 session 裡）。
    std::printf("FileRW ContactForce: boot ctor %s system\\ContactInfo.ini (%s); boot tables still come from LoadContactForceTables() "
                "later in the boot chain; session=%s\n",
                CosFunction.bUseDynamicKitDiameter ? (filerw::ELMarked("WriteFile") ? "ran WriteFile on" : "ran ReadFile on (missing keys seeded)")
                                                   : "did not touch",
                CosFunction.bUseDynamicKitDiameter ? (filerw::ELMarked("WriteFile") ? "no file -> create, or KYEC 30->28/60->58 remap; see session todo"
                                                                                   : "file existed")
                                                   : "bUseDynamicKitDiameter=0",
                filerw::SessionJson().c_str());
    filerw::SessionBegin("");
}

// golden fContactForce->ReadFile()（iosetview.cpp:3451 Tfiosetview::Label120Click）給之後接的入口：建構子沒跑過先跑（golden 開機就建好了；
// R15＝B 之後開機的 FileRW_ContactForce_Boot 也跑了，這裡只剩保底）
void FileRW_ContactForce_ReadFile()
{
    if (!g_booted) FileRW_ContactForce_Boot();
    if (!EnsureCreated("iosetview ReadFile (fallback: boot did not run it)")) CF_ReadFile();
    RefreshShared("ReadFile (iosetview Label120Click)");
    TakeSnapshot();
}

// ===========================================================================
//  建頁備忘（web/page/Setup.ContactForce.html 已存在、純靜態；照 FileRW/GroundMan.cpp ↔ web/page/ht9045_groundman_c.js 的做法補）：
//    * 開頁：WS editlist.get tag=ContactForce → proxies（靜態元件＋動態面板的值／可見／可改）、mustSend、extra（panels／lists／machine／shared）。
//    * 動態面板：依 extra.panels.<類別>.items 在 extra.panels.<類別>.parent 那個 ScrollBox（頁面已有 id）裡建 DOM，元件 id＝items[].ids 的值；
//      群組標題＝proxies[ids.gbLoadRate…].caption 沒有（TGroupBox 不帶 caption）→ 用標籤 lbl* 的 caption，或頁面照 golden 格式自己組
//      （"Load rate of %s mm"）。
//    * 存檔：WS editlist.save tag=ContactForce，widgets 要含 mustSend 全部（含 30 個 edtArm*Offset_*、面板 trackbar／offset），
//      trackbar 送 {position}、edit 送 {text}；ack.messages 會帶多寫者說明、ack.session.todo 帶 ADAM 未接。
//    * 小鍵盤範圍（golden）：edtHotOffset* ±0.5（2 位）、edtContactOffset*／edtArm*Offset_* ±10（2 位）、edMaxKpa* 400～950 整數、
//      edMinMpa* -1～10（3 位）、edMaxMpaFB*／edtMinMpaFB* 0～6（3 位）、edD25_* ±0.5（3 位）。
//    * Button1（電壓換算）：頁面 JS 算 dMaxVol=((dMid-dMin)/5)*9+dMin（golden :1591-1602，不存檔）。
//    * web/page/ht9045_wire_engine.js 的 GOLDEN_BRIDGE 要加一筆（Jimmy 登記的檔 → 片段見 S57 報告）。
// ===========================================================================
