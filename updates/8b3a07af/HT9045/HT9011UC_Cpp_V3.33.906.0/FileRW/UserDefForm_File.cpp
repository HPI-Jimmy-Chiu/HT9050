// ===========================================================================
//  FileRW/UserDefForm_File.cpp -- 結構 UserDefForm_File 的讀寫檔（A＋C：elTrayForm＋逐鍵 → <recipe>\Tray.Data）。
//
//  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（UserDefForm_File）。
//
//  golden TfTrayForm（cTrayForm.cpp）由 tools/gen_editlist.py 轉成 UserDefForm_File.gen.inc：
//    建構子（:28）＝ elTrayForm 的註冊；FormShow（:141）＝開頁（ReadFile＋DoIniDataToForm＋權限）；
//    spbSaveClick（:610）＝存檔鈕（A02 守衛 → SaveSetupFile（elTrayForm＋Name／Memo／Bin Box…逐鍵）→ ReadFile → SECS）。
//  移植樹其他程式還在用 fTrayForm 的真元件（uPAT_Function、csystem／fBuilder 的 fTrayForm->SaveSetupFile），
//  所以名稱型別對得上的元件「收養」移植樹那個物件（TF_AdoptPortWidgets），兩邊共用同一份值。
//  本檔只放開機與 C 路頁面描述（FileRW/_EditPage.h）。
//
//  開機：golden HT9045.cpp:192 CreateForm(TfTrayForm)。tools/wb_serve.cpp 的 tray.* 讀檔鏈原本呼叫
//  fTrayForm->Init()（移植樹以真元件註冊）＋ fTrayForm->ReadFile()，改呼叫本檔的
//  FileRW_TrayForm_Boot()＋FileRW_TrayForm_ReadFile()（golden 建構子與 ReadFile，元件有名稱）。
// ===========================================================================
#include "FileRW/UserDefForm_File.gen.inc"

#include <cstdio>
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbTrayFormClickTail（FileRW/MainClick.cpp；Q41 第 3 項 TF-5）；佔用原本的空行
#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

HTEditList** const kLists[] = {&elTrayForm};
const char* const kListNames[] = {"elTrayForm"};
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbTrayFormClick 關窗尾段，R85）；佔用原本的空行
const filerw::PageDesc kPage = {
    "UserDefForm_File", "TfTrayForm", "Setup.TrayForm.html",
    kLists, kListNames, 1,
    kTF_SaveReads, (int)(sizeof(kTF_SaveReads) / sizeof(kTF_SaveReads[0])),
    &TF_FormShow, &SaveFlow, "SaveSetupFile", &TF_ReadFile, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &TF_spbSaveClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段）；同一行改寫
};
filerw::PageRegistrar g_reg(&kPage);
// AI(W906-FRW-S157) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157）—— 頁面選 cbTrayType1／2／3 一筆
//   → golden cbTrayType1Change（cTrayForm.cpp:570）→ ShowTypePage(Tag, ItemIndex)（:580，填那一型的 10 個欄位）。
//   表 kTF_Events 由 tools/gen_editlist.py 產生（UserDefForm_File.gen.inc 檔尾），本體 FileRW/_EditPage.cpp RunPageEvent。
filerw::PageEventsRegistrar g_evreg("UserDefForm_File", kTF_Events, (int)(sizeof(kTF_Events) / sizeof(kTF_Events[0])));
}  // namespace

// golden TfTrayForm 建構（HT9045.cpp:192）
void FileRW_TrayForm_Boot()
{
    if (g_booted) return;
    if (!elTrayForm) elTrayForm = new HTEditList;                               // golden main.cpp:1487（TfMain 建構子）
    TF_AdoptPortWidgets();
    TF_DfmItems();
    TF_DfmState();
    TF_TfTrayForm();
    TF_CreateSaveProxies();
    TF_CreateContainerProxies();
    std::printf("FileRW UserDefForm_File: elTrayForm registered by name (%d entries) -- golden TfTrayForm ctor cTrayForm.cpp:28\n",
                elTrayForm->FEditList->Count);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:8898 fTrayForm->ReadFile()
void FileRW_TrayForm_ReadFile()
{
    if (g_booted) TF_ReadFile();
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 TF-5（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」
//    的延伸）—— PageDesc::saveFlow 包一層：golden 存檔鈕 spbSaveClick（cTrayForm.cpp:610）跑完之後，補 golden 主畫面
//    TfMain::sbTrayFormClick 的關窗尾段（V912 main.cpp:28411-28413：DoStructUnitConvert、SetWorkParameter；:28413 ShowTrayDeviceDir
//    是主畫面方向圖、不翻；本體 FileRW/MainClick.cpp W906_Main_sbTrayFormClickTail，運轉中不跑 R86）。照 FileRW/Ld_UldDelayTime.cpp 的做法：
//      (1) A02 權限不足（"closed"）—— golden :616 Close() → FormClose（cTrayForm.cpp:559：ReadFile、DoIniDataToForm、fShow=false）
//          → ShowModal 回來 → 尾段：與 golden 相同。
//      (2) 真的寫了檔（"SaveSetupFile"）—— golden 存完視窗還開著、等 Exit 才跑 FormClose＋尾段；網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
//          FormClose 不跑（頁面還開著；存檔鈕最後已 ReadFile，cTrayForm.cpp:631）。
//    差別：golden「開頁、沒存就關」也跑尾段；這裡不跑（值沒變，結果相同）。
//    Q40 的事件表（上面 g_evreg，FileRW/_EditPage.cpp RunPageEvent）不經 saveFlow，不受影響。
//    描述檔 tools/editlist/UserDefForm_File.py 不改、gen.inc 不重產。
// ---------------------------------------------------------------------------
namespace {
void SaveFlow()
{
    TF_spbSaveClick();                                                          // golden cTrayForm.cpp:610
    if (filerw::ELMarked("closed")) {
        TF_FormClose();                                                         // golden cTrayForm.cpp:559
        if (const char* w = W906_Main_sbTrayFormClickTail()) filerw::ELTodo(w);   // golden main.cpp:28411-28413
    } else if (filerw::ELMarked("SaveSetupFile")) {
        if (const char* w = W906_Main_sbTrayFormClickTail()) filerw::ELTodo(w);   // golden main.cpp:28411-28413（時機見上）
    }
}
}  // namespace
