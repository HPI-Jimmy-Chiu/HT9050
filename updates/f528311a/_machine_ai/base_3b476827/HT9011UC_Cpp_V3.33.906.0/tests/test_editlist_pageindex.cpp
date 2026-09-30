// =============================================================================
//  test_editlist_pageindex.cpp -- C 路共用層 TPageControl 目前分頁（值種類 "activePageIndex"）
//
//  AI(W906-FRW-S158) 20260927 [W906]  受測：FileRW/_EditList.cpp ProxyStateJson／ELApplyProxies／ELSetPageOrder。
//  背景：上一位同事做 TrayForm form.event（commit 4e74e8b4）時發現 KindOf 沒有 TPageControl 的值種類、PutProxyValue 也不送
//    ActivePageIndex ⇒ 伺服器端永遠停在 DFM 的設計期分頁（FileRW/TrayForm.cpp TA_EvOnTab 的「分頁跳板」只補得到直接點到的控制項）。
//    [1] editlist.get：TPageControl 替身帶 activePageIndex（替身目前值）；Tag 非 0 照舊帶 tag；別種替身不多這個鍵
//    [2] 合法值套回替身（applied 有名稱、notes 空）；和別的元件同一批也照套
//    [3] 不合法 → 只丟這一筆、其餘照套、理由進 notes：超出範圍（下限＝父子表裡的 TTabSheet 數）、負數、不是整數、
//        不是數字（字串／null／布林）、分頁控制或容器停用／看不見（ELOperable）
//    [4] 舊頁面（沒帶 activePageIndex）行為不變：{} → unknown；{"tag":n} → 照舊當 tag 套
//    [5] 範圍只算「同一個表單、父元件是它、型別是 TTabSheet」的替身（別的表單同名、面板子元件不算）
//    [6] ELSetPageOrder 登記後範圍＝登記的頁數（父子表只知道 3 頁時第 4 頁也收；登記 2 頁時第 3 頁不收）
//    [7] 目標分頁 TabVisible=false 照收（golden V912 cOffSet.cpp:3383 會用程式切到藏起來的頁）
//    [8] notes 為 nullptr：理由直接 ELTodo 進這次 session（SessionJson 的 todo）
//    [9] 同一批有型別錯的別的元件 → 整批拒（回 false）：分頁也沒動、notes 也不記
//    [10] 套值不動 Visible／Enabled／Tag（只改 ActivePageIndex）
//  NOT COVERED：FileRW/_EditPage.cpp PageSave 的「先套分頁再跑 beforeApply、400 還原」與 RunPageEvent 的 state 理由補記
//    （要 god-stack：AccessLevel、PageDesc 的 golden 函式）—— 由整合者用 wb_serve 探針驗（交件列了步驟）。
//  只依賴 FileRW/_EditList.cpp ＋ vclcompat ＋ cJSON／JsonWriter（ht9045_webbridge）；HTEditList_RegisterControlName 在
//  Public/HTEditList.cpp（ht9045_sm，god-stack），本檔給一個只記名字的替身（本測試不建 HTEditList，不看 ControlName）。
//  不讀寫任何檔案，秒級。
// =============================================================================
#include "FileRW/_EditList.h"

#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "Public/cJSON.h"

// Public/HTEditList.cpp:214 的替身（見檔頭）：只記下來，[1] 順便驗 ELKeep 有登記名稱
static std::map<TControl*, std::string> g_names;
void HTEditList_RegisterControlName(TControl* Ctrl, const AnsiString& Name)
{
    if (Ctrl) g_names[Ctrl] = Name.c_str();
}

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

using filerw::EL;

static const char* const kF = "TfPgTest";   // 測試用 golden 表單類別
static const char* const kG = "TfPgOther";  // 另一個表單（同名元件）

// 解析 ProxyStateJson 的某個替身的某個鍵；沒有回 nullptr（呼叫端負責 cJSON_Delete(*root)）
static const cJSON* ProxyKey(const char* form, const char* name, const char* key, cJSON** root)
{
    *root = cJSON_Parse(filerw::ProxyStateJson(form).c_str());
    const cJSON* o = *root ? cJSON_GetObjectItemCaseSensitive(*root, name) : nullptr;
    return o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
}

struct Apply {
    bool ok = false;
    std::vector<std::string> applied, unknown, notes;
    std::string err;
};
static Apply Run(const char* form, const char* json, bool withNotes = true)
{
    Apply a;
    a.ok = filerw::ELApplyProxies(form, json, &a.applied, &a.unknown, &a.err, withNotes ? &a.notes : nullptr);
    return a;
}
static bool Has(const std::vector<std::string>& v, const char* s)
{
    for (const std::string& x : v) if (x == s) return true;
    return false;
}
static bool AnyContains(const std::vector<std::string>& v, const char* s)
{
    for (const std::string& x : v) if (x.find(s) != std::string::npos) return true;
    return false;
}

int main()
{
    // ---- 替身：pnlMain ⊃ pgRun ⊃ {ts0, ts1, ts2}（頁序照登記，但父子表是字母序也不影響「個數」）；ts1 ⊃ pnlInTab；edA 在 pnlMain
    //      pgRun 還有一個非 TTabSheet 的子元件 lblOnPc（VCL 不會有，但產生器的父子表照 DFM 收；不算分頁）
    TPanel* pnlMain = EL<TPanel>(kF, "pnlMain");
    TPageControl* pg = EL<TPageControl>(kF, "pgRun");
    EL<TTabSheet>(kF, "ts0");
    EL<TTabSheet>(kF, "ts1");
    TTabSheet* ts2 = EL<TTabSheet>(kF, "ts2");
    EL<TPanel>(kF, "pnlInTab");
    EL<TLabel>(kF, "lblOnPc");
    TEdit* edA = EL<TEdit>(kF, "edA");
    TCheckBox* cbA = EL<TCheckBox>(kF, "cbA");
    static const char* const kPairs[][2] = {
        {"pgRun", "pnlMain"}, {"ts2", "pgRun"}, {"ts0", "pgRun"}, {"ts1", "pgRun"},
        {"pnlInTab", "ts1"}, {"lblOnPc", "pgRun"}, {"edA", "pnlMain"}, {"cbA", "pnlMain"},
    };
    filerw::ELSetParents(kF, kPairs, (int)(sizeof(kPairs) / sizeof(kPairs[0])));
    // 另一個表單：同名 pgRun，但只有 1 頁
    TPageControl* pgG = EL<TPageControl>(kG, "pgRun");
    EL<TTabSheet>(kG, "tsOnly");
    static const char* const kPairsG[][2] = {{"tsOnly", "pgRun"}};
    filerw::ELSetParents(kG, kPairsG, 1);
    // 第三個分頁控制：父子表裡一頁都沒有
    TPageControl* pgNone = EL<TPageControl>(kF, "pgNone");

    std::printf("[1] editlist.get: ProxyStateJson carries activePageIndex for TPageControl\n");
    {
        cJSON* r = nullptr;
        const cJSON* v = ProxyKey(kF, "pgRun", "activePageIndex", &r);
        CHECK(v && cJSON_IsNumber(v) && v->valueint == 0);                 // vclcompat 建構子 0
        cJSON_Delete(r);
        pg->ActivePageIndex = 2;
        v = ProxyKey(kF, "pgRun", "activePageIndex", &r);
        CHECK(v && cJSON_IsNumber(v) && v->valueint == 2);
        cJSON_Delete(r);
        CHECK(ProxyKey(kF, "pgRun", "tag", &r) == nullptr);                // Tag 0：照舊不帶
        cJSON_Delete(r);
        CHECK(ProxyKey(kF, "pgRun", "visible", &r) != nullptr);            // 原有的鍵還在
        cJSON_Delete(r);
        pg->Tag = 7;
        v = ProxyKey(kF, "pgRun", "tag", &r);
        CHECK(v && v->valueint == 7);                                       // Tag 非 0：照舊帶
        cJSON_Delete(r);
        pg->Tag = 0;
        CHECK(ProxyKey(kF, "ts1", "activePageIndex", &r) == nullptr);      // TTabSheet 不帶
        cJSON_Delete(r);
        CHECK(ProxyKey(kF, "edA", "activePageIndex", &r) == nullptr);      // TEdit 不帶
        cJSON_Delete(r);
        CHECK(ProxyKey(kF, "pnlMain", "activePageIndex", &r) == nullptr);  // TPanel 不帶
        cJSON_Delete(r);
        CHECK(g_names[pg] == "pgRun");                                      // ELKeep 照舊登記名稱
    }

    std::printf("[2] valid activePageIndex is applied\n");
    {
        pg->ActivePageIndex = 1;
        Apply a = Run(kF, "{\"pgRun\":{\"activePageIndex\":0}}");
        CHECK(a.ok && Has(a.applied, "pgRun") && a.notes.empty() && a.unknown.empty());
        CHECK(pg->ActivePageIndex == 0);
        a = Run(kF, "{\"edA\":{\"text\":\"12.5\"},\"pgRun\":{\"activePageIndex\":2},\"cbA\":{\"checked\":true}}");
        CHECK(a.ok && a.applied.size() == 3 && a.notes.empty());
        CHECK(pg->ActivePageIndex == 2 && edA->Text == "12.5" && cbA->Checked);
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":1.0}}");               // 1.0 是整數
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 1);
        cJSON* r = nullptr;
        const cJSON* v = ProxyKey(kF, "pgRun", "activePageIndex", &r);    // 套完 get 回來就是頁面送的
        CHECK(v && v->valueint == 1);
        cJSON_Delete(r);
    }

    std::printf("[3] invalid activePageIndex: only that entry is dropped, reason in notes\n");
    {
        pg->ActivePageIndex = 1;
        static const char* const kBad[] = {
            "{\"pgRun\":{\"activePageIndex\":3},\"edA\":{\"text\":\"B3\"}}",       // 父子表 3 頁 ⇒ 0..2
            "{\"pgRun\":{\"activePageIndex\":-1},\"edA\":{\"text\":\"B-1\"}}",
            "{\"pgRun\":{\"activePageIndex\":1.5},\"edA\":{\"text\":\"B15\"}}",
            "{\"pgRun\":{\"activePageIndex\":\"1\"},\"edA\":{\"text\":\"Bs\"}}",
            "{\"pgRun\":{\"activePageIndex\":null},\"edA\":{\"text\":\"Bn\"}}",
            "{\"pgRun\":{\"activePageIndex\":true},\"edA\":{\"text\":\"Bb\"}}",
            "{\"pgRun\":{\"activePageIndex\":1e300},\"edA\":{\"text\":\"Be\"}}",
        };
        static const char* const kText[] = {"B3", "B-1", "B15", "Bs", "Bn", "Bb", "Be"};
        static const char* const kWhy[] = {"outside 0..2", "outside 0..2", "not a whole number", "not a number", "not a number",
                                           "not a number", "outside 0..2"};
        for (int i = 0; i < 7; ++i) {
            Apply a = Run(kF, kBad[i]);
            std::printf("    case %d: %s\n", i, a.notes.empty() ? "(no note)" : a.notes[0].c_str());
            CHECK(a.ok && !Has(a.applied, "pgRun") && Has(a.applied, "edA") && a.unknown.empty());
            CHECK(pg->ActivePageIndex == 1 && edA->Text == kText[i]);
            CHECK(a.notes.size() == 1 && AnyContains(a.notes, "TfPgTest.pgRun") && AnyContains(a.notes, kWhy[i]) &&
                  AnyContains(a.notes, "keeps tab 1"));
        }
        // 父子表一頁都沒有
        pgNone->ActivePageIndex = 0;
        Apply a = Run(kF, "{\"pgNone\":{\"activePageIndex\":0}}");
        CHECK(a.ok && a.applied.empty() && a.notes.size() == 1 && AnyContains(a.notes, "knows no tab sheet"));
        // 分頁控制自己停用／看不見、容器停用 ⇒ golden 使用者換不了頁
        pg->Enabled = false;
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");
        CHECK(a.ok && a.applied.empty() && pg->ActivePageIndex == 1 && AnyContains(a.notes, "disabled or hidden"));
        pg->Enabled = true;
        pg->Visible = false;
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");
        CHECK(a.ok && a.applied.empty() && pg->ActivePageIndex == 1 && AnyContains(a.notes, "disabled or hidden"));
        pg->Visible = true;
        pnlMain->Enabled = false;
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");
        CHECK(a.ok && a.applied.empty() && pg->ActivePageIndex == 1 && AnyContains(a.notes, "disabled or hidden"));
        pnlMain->Enabled = true;
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");                 // 全部恢復 ⇒ 收
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 2 && a.notes.empty());
    }

    std::printf("[4] old pages (no activePageIndex key): behaviour unchanged\n");
    {
        pg->ActivePageIndex = 2;
        Apply a = Run(kF, "{\"pgRun\":{}}");
        CHECK(a.ok && Has(a.unknown, "pgRun") && a.applied.empty() && a.notes.empty() && pg->ActivePageIndex == 2);
        a = Run(kF, "{\"pgRun\":{\"visible\":false,\"enabled\":true}}");   // 頁面回送 get 的狀態鍵：不歸頁面管
        CHECK(a.ok && Has(a.unknown, "pgRun") && pg->Visible && pg->ActivePageIndex == 2);
        a = Run(kF, "{\"pgRun\":{\"tag\":5}}");                              // 舊規則：帶 tag 的非值元件照 tag 套
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->Tag == 5 && pg->ActivePageIndex == 2);
        pg->Tag = 0;
        a = Run(kF, "{\"pgRun\":\"x\"}");                                    // 不是物件：照舊 unknown
        CHECK(a.ok && Has(a.unknown, "pgRun"));
    }

    std::printf("[5] tab count only counts this form's TTabSheet children of this page control\n");
    {
        pgG->ActivePageIndex = 0;
        Apply a = Run(kG, "{\"pgRun\":{\"activePageIndex\":1}}");           // TfPgOther.pgRun 只有 1 頁
        CHECK(a.ok && a.applied.empty() && pgG->ActivePageIndex == 0 && AnyContains(a.notes, "TfPgOther.pgRun") &&
              AnyContains(a.notes, "outside 0..0"));
        a = Run(kG, "{\"pgRun\":{\"activePageIndex\":0}}");
        CHECK(a.ok && Has(a.applied, "pgRun") && a.notes.empty());
        CHECK(pg != pgG);                                                     // 兩個表單各自一個替身
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");                  // TfPgTest.pgRun 仍是 3 頁（lblOnPc 不算）
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 2);
    }

    std::printf("[6] ELSetPageOrder overrides the parent-table lower bound\n");
    {
        static const char* const kOrder4[] = {"ts0", "ts1", "ts2", "tsNotAProxy"};   // DFM 有、產生器沒建替身的第 4 頁
        filerw::ELSetPageOrder(kF, "pgRun", kOrder4, 4);
        Apply a = Run(kF, "{\"pgRun\":{\"activePageIndex\":3}}");
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 3 && a.notes.empty());
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":4}}");
        CHECK(a.ok && a.applied.empty() && pg->ActivePageIndex == 3 && AnyContains(a.notes, "outside 0..3") &&
              AnyContains(a.notes, "golden DFM tab order"));
        static const char* const kOrder2[] = {"ts0", "ts1"};                         // 重新登記：取代，不累加
        filerw::ELSetPageOrder(kF, "pgRun", kOrder2, 2);
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");
        CHECK(a.ok && a.applied.empty() && pg->ActivePageIndex == 3 && AnyContains(a.notes, "outside 0..1"));
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":1}}");
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 1);
        static const char* const kOrder3[] = {"ts0", "ts1", "ts2"};
        filerw::ELSetPageOrder(kF, "pgRun", kOrder3, 3);
        Apply b = Run(kG, "{\"pgRun\":{\"activePageIndex\":1}}");              // 別的表單的同名元件不受影響
        CHECK(b.ok && b.applied.empty() && AnyContains(b.notes, "outside 0..0"));
    }

    std::printf("[7] a TabVisible=false target tab is accepted\n");
    {
        ts2->TabVisible = false;
        Apply a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2}}");
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 2 && a.notes.empty());
        CHECK(!ts2->TabVisible);                                              // 不動分頁的 TabVisible
        ts2->TabVisible = true;
    }

    std::printf("[8] notes == nullptr: reasons go straight to the session todo\n");
    {
        filerw::SessionBegin("");
        pg->ActivePageIndex = 0;
        Apply a = Run(kF, "{\"pgRun\":{\"activePageIndex\":9}}", false);
        CHECK(a.ok && a.applied.empty() && a.notes.empty() && pg->ActivePageIndex == 0);
        const std::string sj = filerw::SessionJson();
        std::printf("    session: %s\n", sj.c_str());
        CHECK(sj.find("TfPgTest.pgRun activePageIndex 9 ignored") != std::string::npos);
        filerw::SessionBegin("");                                             // SessionBegin 清掉（所以 PageSave／RunPageEvent 要傳 notes）
        CHECK(filerw::SessionJson().find("pgRun") == std::string::npos);
    }

    std::printf("[9] another widget with a wrong type refuses the whole batch (tab untouched, no note)\n");
    {
        pg->ActivePageIndex = 0;
        edA->Text = "keep";
        Apply a = Run(kF, "{\"pgRun\":{\"activePageIndex\":2},\"edA\":{\"text\":5}}");
        CHECK(!a.ok && a.err.find("edA.text") != std::string::npos);
        CHECK(pg->ActivePageIndex == 0 && edA->Text == "keep" && a.notes.empty() && a.applied.empty());
        a = Run(kF, "{\"pgRun\":{\"activePageIndex\":7},\"edA\":{\"text\":5}}");   // 不合法的分頁＋整批拒：也不記
        CHECK(!a.ok && a.notes.empty() && pg->ActivePageIndex == 0);
    }

    std::printf("[10] applying the tab touches ActivePageIndex only\n");
    {
        pg->Tag = 0;
        pg->ActivePageIndex = 0;
        Apply a = Run(kF, "{\"pgRun\":{\"activePageIndex\":1,\"visible\":false,\"enabled\":false,\"tag\":4}}");
        CHECK(a.ok && Has(a.applied, "pgRun") && pg->ActivePageIndex == 1);
        CHECK(pg->Visible && pg->Enabled && pg->Tag == 0);                    // 頁面帶來的其他鍵不歸這個種類
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
