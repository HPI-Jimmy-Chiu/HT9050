// ===========================================================================
//  FileRW/Ld_UldDelayTime.cpp -- 結構 Ld_UldDelayTime 的讀寫檔（C 類 HTEditList：elUdUld → <recipe>\UdUld.Data）。
//
//  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（Ld_UldDelayTime）。
//
//  golden TfLd_ULd（cLd_ULd.cpp）由 tools/gen_editlist.py 轉成 Ld_UldDelayTime.gen.inc（元件改成具名替身）：
//    建構子（:22）＝ elUdUld 的註冊；FormShow（:87）＝開頁（ReadFile＋權限）；
//    spbSaveClick（:179）＝存檔鈕（A02 守衛 → SaveSetupFile → elUdUld->SaveEditTextToFile → ReadFile → SECS）。
//  本檔只放開機與 C 路頁面描述（FileRW/_EditPage.h）。
//
//  開機順序照 golden：HT9045.cpp:191 CreateForm(TfLd_ULd) 在 :207 TfConfiguration 之前 ——
//  HTEditList 同 (區段,鍵) 第一筆生效，順序不能反。FileRW_IniConfig_Boot 經 W906_LdUldInitOnce（cSpeed.cpp）
//  呼叫本檔的 FileRW_LdUld_Boot。原本移植樹的 fLd_ULd->Init() 以真元件註冊（頁面拿不到名稱），不再使用。
// ===========================================================================
#include "FileRW/Ld_UldDelayTime.gen.inc"

#include <cstdio>
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbLdUldClickTail（FileRW/MainClick.cpp；Q41 第 3 項 LU-2）；佔用原本的空行
#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbLdUldClick 關窗尾段，R85）；佔用原本的空行
HTEditList** const kLists[] = {&elUdUld};
const char* const kListNames[] = {"elUdUld"};

const filerw::PageDesc kPage = {
    "Ld_UldDelayTime", "TfLd_ULd", "Setup.Ld_ULd.html",
    kLists, kListNames, 1,
    kLU_SaveReads, (int)(sizeof(kLU_SaveReads) / sizeof(kLU_SaveReads[0])),
    &LU_FormShow, &SaveFlow, "SaveSetupFile", &LU_ReadFile, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &LU_spbSaveClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段）；同一行改寫
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfLd_ULd 建構（HT9045.cpp:191）：DFM 設計期狀態 → 建構子（elUdUld->Add…）→ 存檔流程讀的替身
void FileRW_LdUld_Boot()
{
    if (g_booted) return;
    if (!elUdUld) elUdUld = new HTEditList;                                     // golden main.cpp:1486（TfMain 建構子）
    LU_DfmItems();
    LU_DfmState();
    LU_TfLd_ULd();
    LU_CreateSaveProxies();
    LU_CreateContainerProxies();
    std::printf("FileRW Ld_UldDelayTime: elUdUld registered by name (%d entries) -- golden TfLd_ULd ctor cLd_ULd.cpp:22\n",
                elUdUld->FEditList->Count);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:8921 fLd_ULd->ReadFile()
void FileRW_LdUld_ReadFile()
{
    if (g_booted) LU_ReadFile();
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 LU-2（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」
//    的延伸）—— PageDesc::saveFlow 包一層：golden 存檔鈕 spbSaveClick（cLd_ULd.cpp:179）跑完之後，補 golden 主畫面
//    TfMain::sbLdUldClick 的關窗尾段（V912 main.cpp:28454 DoStructUnitConvert；本體 FileRW/MainClick.cpp W906_Main_sbLdUldClickTail，
//    運轉中不跑 R86）。照 FileRW/Temperature.cpp SaveFlow 的做法：
//      (1) A02 權限不足（"closed"）—— golden :185 Close() → FormClose（cLd_ULd.cpp:170：ReadFile、rbTemp->SetFocus、fShow=false）
//          → ShowModal 回來 → 尾段：與 golden 相同。
//      (2) 真的寫了檔（"SaveSetupFile"）—— golden 存完視窗還開著、等 Exit 才跑尾段；網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
//    差別：golden「開頁、沒存就關」也跑尾段；這裡不跑（值沒變，DoStructUnitConvert 的結果相同）。
//    描述檔 tools/editlist/Ld_UldDelayTime.py 不改、gen.inc 不重產。
// ---------------------------------------------------------------------------
namespace {
void SaveFlow()
{
    LU_spbSaveClick();                                                          // golden cLd_ULd.cpp:179
    if (filerw::ELMarked("closed")) {
        LU_FormClose();                                                         // golden cLd_ULd.cpp:170
        if (const char* w = W906_Main_sbLdUldClickTail()) filerw::ELTodo(w);    // golden main.cpp:28454
    } else if (filerw::ELMarked("SaveSetupFile")) {
        if (const char* w = W906_Main_sbLdUldClickTail()) filerw::ELTodo(w);    // golden main.cpp:28454（時機見上）
    }
}
}  // namespace

// ===========================================================================
//  AI(W906-EVB10C) 20260929 [W906]：事件批次 B10 part c（FileRW/WindowEdgeTails.h 第 7 列；Steven 20260928「如果已經有移植, 就接上」、
//    20260929「照 BCB 的邏輯」）。golden TfLd_ULd::FormClose（V912 cLd_ULd.cpp:170-177）：ReadFile(); rbTemp->SetFocus(); fShow=false;
//    ＝產生檔的 LU_FormClose（SetFocus＝頁面焦點，產生檔已閘）。重讀 <配方>\UdUld.Data（elUdUld）＝丟掉這一次開窗裡沒存的改動。
//  頁面表的關窗邊緣（FileRW/MainClick.cpp W906_EvB10A_WindowEdge）呼叫：這一次開窗 golden FormShow 跑過、FormClose 還沒跑過
//    （filerw::PageCloseEdgeRefused；A02 的 Close() 上面 SaveFlow 已經跑過 LU_FormClose ⇒ 不跑第二次）才跑；運轉中不跑（ShowModal main.cpp:28453）。
//  主畫面尾段 sbLdUldClick :28454 照 R85／S107-1 留在存檔之後（上面 SaveFlow），關窗不再跑。
// ===========================================================================
#include "FileRW/WindowEdgeTails.h"

const char* FileRW_LdUld_WindowEdge(bool open)
{
    if (open) return "no open-edge action";
    if (!g_booted) return "not run: TfLd_ULd proxies are not booted";
    if (const char* no = filerw::PageCloseEdgeRefused("Ld_UldDelayTime")) return no;
    LU_FormClose();                                                             // golden cLd_ULd.cpp:170
    return "ran golden TfLd_ULd::FormClose (cLd_ULd.cpp:170-177): ReadFile (UdUld.Data), fShow=false (rbTemp->SetFocus is page focus)";
}
