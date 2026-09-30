// =============================================================================
//  test_evb10b_vclclicks.cpp -- 事件批次 B10 part b 的 C 路共用層：VCL「程式設值也會觸發 OnClick」與切分頁事件要的頁序查詢。
//
//  AI(W906-EVB10B) 20260929 [W906]  St01 新檔。派工：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表
//    X-3（R100、R118＝照 BCB）、X-5、CC-L1（Q46）。Steven 20260929「照 BCB 的邏輯」。
//  受測：FileRW/_EditList.cpp 的 ELSetOnClick／ELOnClickOf／ELClickIndex／ELClickChecked（TCheckBox、TRadioButton）／ELClickCount／
//    ELHTEditSetChecked／ELHTEditSetItemIndex（Public/HTEditList.cpp HTEditList_SetClickHooks 裝的那兩支）與 ELActivePage／ELPageIndexOf／
//    ELTabIndexOf（＋既有的 ELSetPageOrder、ELPageIndexRefused、ELChangeCompomentEnabled、ELEditable 一起用）。
//    [1] TRadioGroup（VCL TCustomRadioGroup.SetItemIndex）：值有變且 >=0 才 OnClick、Sender＝那個群組；同值不觸發；夾在 -1..Count-1；
//        設成 -1 不觸發；Items 空的群組夾成 -1
//    [2] 沒登記 OnClick：ELClickIndex 照樣夾值但不觸發、計數不變
//    [3] TCheckBox：值有變就觸發（勾、取消都算）；同值不觸發
//    [4] TRadioButton：false→true 才觸發；true→false 不觸發
//    [5] 巢狀：處理器裡再用 ELClickChecked 設別的元件 ⇒ 那一格的處理器接著跑（VCL 同），順序對
//    [6] 處理器自己改回（golden cbA09Click 的寫法）⇒ 再進一次、值相等就停（不會無限遞迴）
//    [7] HTEditList hook：有登記 ⇒ 照 VCL 觸發；沒登記 ⇒ 純設值、連夾值都不做（跟裝 hook 之前一樣）
//    [8] ELSetOnClick(c, nullptr) 取消登記
//    [9] 頁序查詢：ELActivePage／ELPageIndexOf／ELTabIndexOf（藏起來的頁 TabIndex＝-1、後面的頁往前數；沒有替身的頁算看得見；沒登記頁序＝nullptr／-1）
//    [10] CC-L1 的組合：切到的頁被 ELChangeCompomentEnabled(頁, false) 鎖住 ⇒ 底下的元件 ELEditable＝false；別頁不受影響；
//         bMustEnable=false 時 true 不會把頁打開（golden 同）；ELSetPageOrder 之後 ELPageIndexRefused 的範圍＝登記的頁數
//    [11] R118 的順序（golden cprod.cpp SetCustomerLimitationForConfig：ReadEditTextFromFile 逐筆「設元件 → 元件值寫回變數」，
//         再 InitialDataToEdit 逐筆「變數 → 元件」）：D36 由不勾變勾時 cbD36Click 把畫面 D33 取消、D35 勾上，但 D33／D35 的變數在前面
//         幾筆已經是檔案值，InitialDataToEdit 又設回去 ⇒ 最後畫面＝檔案值，處理器只跑一次；再讀一次（D36 沒變）不再觸發
//  NOT COVERED：產生檔（TrayForm／IniConfig／Temperature .gen.inc）裡 golden 處理器本身的結果、Public/HTEditList.cpp 裡 hook 的呼叫點
//    （要 god-stack；wb_serve 目標編譯＋連結有驗到）。
//  只依賴 FileRW/_EditList.cpp ＋ vclcompat ＋ cJSON／JsonWriter（ht9045_webbridge）；HTEditList_RegisterControlName（Public/HTEditList.cpp，
//  god-stack）由本檔給只記名字的替身（同 test_editlist_pageindex.cpp）。不讀寫任何檔案，秒級。
// =============================================================================
#include "FileRW/_EditList.h"

#include <cstdio>
#include <string>
#include <vector>

// Public/HTEditList.cpp 的替身：本測試不建 HTEditList
void HTEditList_RegisterControlName(TControl* Ctrl, const AnsiString& Name)
{
    (void)Ctrl;
    (void)Name;
}

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

using filerw::EL;

static const char* const kF = "TfClickTest";

static std::vector<std::string> g_log;   // 處理器被呼叫的順序
static TControl* g_lastSender = nullptr;

static void OnRg(TControl* s)  { g_log.push_back("rg");  g_lastSender = s; }
static void OnCb(TControl* s)  { g_log.push_back("cb");  g_lastSender = s; }
static void OnRb(TControl* s)  { g_log.push_back("rb");  g_lastSender = s; }
// [5] rgNest 的處理器把 cbNest 勾上（巢狀）
static void OnRgNest(TControl* s)
{
    g_log.push_back("rgNest");
    (void)s;
    filerw::ELClickChecked(EL<TCheckBox>(kF, "cbNest"), true);
}
static void OnCbNest(TControl* s) { g_log.push_back("cbNest"); (void)s; }
// [6] golden cbA09Click 的形狀：跟「記憶體值」不同就改回（改回 ⇒ VCL 再進一次，這次值相等就 return）
static bool g_memA09 = false;
static void OnA09(TControl* s)
{
    (void)s;
    g_log.push_back("a09");
    TCheckBox* c = EL<TCheckBox>(kF, "cbA09");
    if (c->Checked == g_memA09) return;
    filerw::ELClickChecked(c, !c->Checked);   // 機台有料：改回
}
// [11] golden cbD36Click（cConfiguration.cpp:6551）
static bool g_d33WhenD36Fired = true;
static void OnD36(TControl* s)
{
    (void)s;
    g_log.push_back("d36");
    if (EL<TCheckBox>(kF, "cbD36")->Checked) {
        EL<TCheckBox>(kF, "cbD33")->Checked = false;
        EL<TCheckBox>(kF, "cbD35")->Checked = true;
        g_d33WhenD36Fired = EL<TCheckBox>(kF, "cbD33")->Checked;
    }
}

static void Items(TRadioGroup* g, int n)
{
    g->Items->Clear();
    for (int i = 0; i < n; ++i) g->Items->Add(AnsiString("item") + AnsiString(i));
}

int main()
{
    std::printf("[1] TRadioGroup ELClickIndex\n");
    {
        TRadioGroup* g = EL<TRadioGroup>(kF, "rgA");
        Items(g, 3);
        g->ItemIndex = 0;
        filerw::ELSetOnClick(g, &OnRg);
        CHECK(filerw::ELOnClickOf(g) == &OnRg);
        g_log.clear();
        const long c0 = filerw::ELClickCount();
        filerw::ELClickIndex(g, 1);
        CHECK(g->ItemIndex == 1);
        CHECK(g_log.size() == 1 && g_log[0] == "rg");
        CHECK(g_lastSender == g);
        CHECK(filerw::ELClickCount() == c0 + 1);
        filerw::ELClickIndex(g, 1);                                  // 同值
        CHECK(g_log.size() == 1);
        filerw::ELClickIndex(g, 5);                                  // 夾成 Count-1
        CHECK(g->ItemIndex == 2);
        CHECK(g_log.size() == 2);
        filerw::ELClickIndex(g, -1);                                 // -1：VCL 只取消舊的那顆，不 Click
        CHECK(g->ItemIndex == -1);
        CHECK(g_log.size() == 2);
        filerw::ELClickIndex(g, -7);                                 // 夾成 -1，已經是 -1
        CHECK(g->ItemIndex == -1);
        CHECK(g_log.size() == 2);
        filerw::ELClickIndex(g, 0);
        CHECK(g->ItemIndex == 0 && g_log.size() == 3);
        TRadioGroup* e = EL<TRadioGroup>(kF, "rgEmpty");             // Items 空的
        e->ItemIndex = 0;
        filerw::ELSetOnClick(e, &OnRg);
        filerw::ELClickIndex(e, 0);
        CHECK(e->ItemIndex == -1);
        CHECK(g_log.size() == 3);
    }

    std::printf("[2] no registered OnClick\n");
    {
        TRadioGroup* g = EL<TRadioGroup>(kF, "rgPlain");
        Items(g, 2);
        g->ItemIndex = 0;
        g_log.clear();
        const long c0 = filerw::ELClickCount();
        filerw::ELClickIndex(g, 9);
        CHECK(g->ItemIndex == 1);                                    // 照 VCL 夾
        CHECK(g_log.empty());
        CHECK(filerw::ELClickCount() == c0);
        CHECK(filerw::ELOnClickOf(g) == nullptr);
    }

    std::printf("[3] TCheckBox ELClickChecked\n");
    {
        TCheckBox* c = EL<TCheckBox>(kF, "cbA");
        c->Checked = false;
        filerw::ELSetOnClick(c, &OnCb);
        g_log.clear();
        filerw::ELClickChecked(c, true);
        CHECK(c->Checked && g_log.size() == 1 && g_lastSender == c);
        filerw::ELClickChecked(c, true);
        CHECK(g_log.size() == 1);
        filerw::ELClickChecked(c, false);                            // 取消也觸發（TCustomCheckBox.SetState）
        CHECK(!c->Checked && g_log.size() == 2);
    }

    std::printf("[4] TRadioButton ELClickChecked\n");
    {
        TRadioButton* r = EL<TRadioButton>(kF, "rbA");
        r->Checked = false;
        filerw::ELSetOnClick(r, &OnRb);
        g_log.clear();
        filerw::ELClickChecked(r, true);
        CHECK(r->Checked && g_log.size() == 1 && g_lastSender == r);
        filerw::ELClickChecked(r, false);                            // true→false 不 Click（TRadioButton.SetChecked）
        CHECK(!r->Checked && g_log.size() == 1);
    }

    std::printf("[5] nested\n");
    {
        TRadioGroup* g = EL<TRadioGroup>(kF, "rgNest");
        Items(g, 2);
        g->ItemIndex = 0;
        EL<TCheckBox>(kF, "cbNest")->Checked = false;
        filerw::ELSetOnClick(g, &OnRgNest);
        filerw::ELSetOnClick(EL<TCheckBox>(kF, "cbNest"), &OnCbNest);
        g_log.clear();
        filerw::ELClickIndex(g, 1);
        CHECK(g_log.size() == 2 && g_log[0] == "rgNest" && g_log[1] == "cbNest");
        CHECK(EL<TCheckBox>(kF, "cbNest")->Checked);
    }

    std::printf("[6] handler puts itself back (golden cbA09Click shape)\n");
    {
        TCheckBox* c = EL<TCheckBox>(kF, "cbA09");
        c->Checked = false;
        g_memA09 = false;
        filerw::ELSetOnClick(c, &OnA09);
        g_log.clear();
        filerw::ELClickChecked(c, true);                             // 跟記憶體不同 → 改回 → 再進一次 → 相等 return
        CHECK(!c->Checked);
        CHECK(g_log.size() == 2);
    }

    std::printf("[7] HTEditList hooks\n");
    {
        TCheckBox* reg = EL<TCheckBox>(kF, "cbHookReg");
        TCheckBox* plain = EL<TCheckBox>(kF, "cbHookPlain");
        reg->Checked = false;
        plain->Checked = false;
        filerw::ELSetOnClick(reg, &OnCb);
        g_log.clear();
        filerw::ELHTEditSetChecked(reg, true);
        filerw::ELHTEditSetChecked(plain, true);
        CHECK(reg->Checked && plain->Checked);
        CHECK(g_log.size() == 1 && g_lastSender == reg);
        TRadioGroup* gr = EL<TRadioGroup>(kF, "rgHookReg");
        TRadioGroup* gp = EL<TRadioGroup>(kF, "rgHookPlain");
        Items(gr, 3);
        Items(gp, 3);
        gr->ItemIndex = 0;
        gp->ItemIndex = 0;
        filerw::ELSetOnClick(gr, &OnRg);
        g_log.clear();
        filerw::ELHTEditSetItemIndex(gr, 9);                         // 登記的：照 VCL 夾＋觸發
        filerw::ELHTEditSetItemIndex(gp, 9);                         // 沒登記的：跟以前一樣純設值（不夾）
        CHECK(gr->ItemIndex == 2 && g_log.size() == 1);
        CHECK(gp->ItemIndex == 9);
    }

    std::printf("[8] unregister\n");
    {
        TCheckBox* c = EL<TCheckBox>(kF, "cbA");
        filerw::ELSetOnClick(c, nullptr);
        CHECK(filerw::ELOnClickOf(c) == nullptr);
        g_log.clear();
        filerw::ELClickChecked(c, !c->Checked);
        CHECK(g_log.empty());
    }

    std::printf("[9] tab order queries\n");
    {
        TPageControl* pc = EL<TPageControl>(kF, "pcT");
        CHECK(filerw::ELActivePage(kF, "pcT") == nullptr);           // 沒登記頁序
        CHECK(filerw::ELPageIndexOf(kF, "pcT", "t1") == -1);
        static const char* const kPages[] = {"t0", "t1", "t2", "t3"};
        EL<TTabSheet>(kF, "t0");
        EL<TTabSheet>(kF, "t1");
        EL<TTabSheet>(kF, "t2");                                      // t3 沒有替身（轉出的程式沒碰過）
        filerw::ELSetPageOrder(kF, "pcT", kPages, 4);
        pc->ActivePageIndex = 2;
        CHECK(filerw::ELActivePage(kF, "pcT") == filerw::ELFind(kF, "t2"));
        pc->ActivePageIndex = 3;
        CHECK(filerw::ELActivePage(kF, "pcT") == nullptr);           // 那一頁沒有替身
        pc->ActivePageIndex = 7;
        CHECK(filerw::ELActivePage(kF, "pcT") == nullptr);           // 超出範圍
        CHECK(filerw::ELPageIndexOf(kF, "pcT", "t2") == 2);
        CHECK(filerw::ELPageIndexOf(kF, "pcT", "t9") == -1);
        CHECK(filerw::ELTabIndexOf(kF, "pcT", "t2") == 2);
        EL<TTabSheet>(kF, "t1")->TabVisible = false;
        CHECK(filerw::ELTabIndexOf(kF, "pcT", "t1") == -1);          // 自己藏起來
        CHECK(filerw::ELTabIndexOf(kF, "pcT", "t2") == 1);           // 前面少一頁
        CHECK(filerw::ELPageIndexOf(kF, "pcT", "t2") == 2);          // 頁序不看 TabVisible
        EL<TTabSheet>(kF, "t1")->TabVisible = true;
    }

    std::printf("[10] CC-L1 lock on tab change\n");
    {
        static const char* const kPages[] = {"tsX0", "tsX1", "tsX2"};
        TPageControl* pc = EL<TPageControl>(kF, "pcLock");
        EL<TTabSheet>(kF, "tsX0");
        EL<TTabSheet>(kF, "tsX1");
        EL<TTabSheet>(kF, "tsX2");
        EL<TEdit>(kF, "edOn1");
        EL<TEdit>(kF, "edOn2");
        static const char* const kPar[][2] = {{"tsX0", "pcLock"}, {"tsX1", "pcLock"}, {"tsX2", "pcLock"},
                                               {"edOn1", "tsX1"}, {"edOn2", "tsX2"}};
        filerw::ELSetParents(kF, kPar, 5);
        filerw::ELSetPageOrder(kF, "pcLock", kPages, 3);
        CHECK(filerw::ELPageIndexRefused(kF, "pcLock", 2).empty());
        CHECK(!filerw::ELPageIndexRefused(kF, "pcLock", 3).empty());  // 精確範圍 0..2
        const bool auth[3] = {true, false, true};
        pc->ActivePageIndex = 1;                                       // 使用者點到第 1 頁（RunPageEvent 先換頁）
        filerw::ELChangeCompomentEnabled(filerw::ELActivePage(kF, "pcLock"), auth[pc->ActivePageIndex]);   // golden pcConfigChange 的形狀
        CHECK(!EL<TTabSheet>(kF, "tsX1")->Enabled);
        CHECK(!filerw::ELEditable(kF, "edOn1"));
        CHECK(filerw::ELEditable(kF, "edOn2"));
        pc->ActivePageIndex = 2;
        filerw::ELChangeCompomentEnabled(filerw::ELActivePage(kF, "pcLock"), auth[pc->ActivePageIndex]);
        CHECK(EL<TTabSheet>(kF, "tsX2")->Enabled);
        CHECK(!filerw::ELEditable(kF, "edOn1"));                       // 已鎖的頁不會因為切走而打開（golden：只有 FormShow 的 bMustEnable 會開）
        pc->ActivePageIndex = 1;
        filerw::ELChangeCompomentEnabled(filerw::ELActivePage(kF, "pcLock"), true);   // bMustEnable=false：true 不會打開
        CHECK(!EL<TTabSheet>(kF, "tsX1")->Enabled);
    }

    std::printf("[11] R118 order: ReadEditTextFromFile then InitialDataToEdit (golden cprod.cpp:2837 / :2922)\n");
    {
        TCheckBox* d33 = EL<TCheckBox>(kF, "cbD33");
        TCheckBox* d35 = EL<TCheckBox>(kF, "cbD35");
        TCheckBox* d36 = EL<TCheckBox>(kF, "cbD36");
        filerw::ELSetOnClick(d36, &OnD36);                            // golden DFM：只有 D36 有 OnClick（D33／D35 沒有）
        d33->Checked = d35->Checked = d36->Checked = false;           // 開機前：DFM 預設
        bool var33 = false, var35 = false, var36 = false;             // IniConfig 變數
        const bool file33 = true, file35 = false, file36 = true;      // config.ini：D33、D36 都勾（R118 的例子）
        struct Item { TCheckBox* c; bool* var; bool file; } items[3] = {{d33, &var33, file33}, {d35, &var35, file35}, {d36, &var36, file36}};
        for (int pass = 0; pass < 2; ++pass) {                         // 開機一次、開頁（FormShow :4625 ReadLastSetIni）再一次
            g_log.clear();
            for (Item& it : items) { filerw::ELHTEditSetChecked(it.c, it.file); *it.var = it.c->Checked; }   // ReadEditTextFromFile
            for (Item& it : items) filerw::ELHTEditSetChecked(it.c, *it.var);                                 // InitialDataToEdit
            if (pass == 0) {
                CHECK(g_log.size() == 1);                              // D36 不勾→勾：cbD36Click 跑一次
                CHECK(!g_d33WhenD36Fired);                             // 那一刻畫面的 D33 被取消
            } else {
                CHECK(g_log.empty());                                  // 第二次 D36 沒變：不觸發
            }
            CHECK(d33->Checked == file33 && var33 == file33);           // 最後畫面＝檔案值（同一次讀檔的 InitialDataToEdit 設回去）
            CHECK(d35->Checked == file35 && var35 == file35);
            CHECK(d36->Checked && var36);
        }
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
// AI(W906-S09-Q3) 20260930 (St02-E): FileRW/_EditList.cpp now takes the FormJson lock in FileRW_ProxyChecked / FileRW_ProxySet* (St01 R1);
//   this test compiles _EditList.cpp without JsonBridge/FormJson.cpp, so it gives the lock itself (single-threaded:
//   a no-op), as test_b8_os5_sortbuttons.cpp:62-65 does.
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }
