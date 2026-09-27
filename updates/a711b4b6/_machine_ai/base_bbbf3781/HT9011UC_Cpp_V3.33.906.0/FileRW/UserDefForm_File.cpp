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

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

HTEditList** const kLists[] = {&elTrayForm};
const char* const kListNames[] = {"elTrayForm"};

const filerw::PageDesc kPage = {
    "UserDefForm_File", "TfTrayForm", "Setup.TrayForm.html",
    kLists, kListNames, 1,
    kTF_SaveReads, (int)(sizeof(kTF_SaveReads) / sizeof(kTF_SaveReads[0])),
    &TF_FormShow, &TF_spbSaveClick, "SaveSetupFile", &TF_ReadFile, &Booted,
};
filerw::PageRegistrar g_reg(&kPage);
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
