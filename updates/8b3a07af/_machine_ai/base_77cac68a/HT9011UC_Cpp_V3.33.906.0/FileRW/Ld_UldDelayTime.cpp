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

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

HTEditList** const kLists[] = {&elUdUld};
const char* const kListNames[] = {"elUdUld"};

const filerw::PageDesc kPage = {
    "Ld_UldDelayTime", "TfLd_ULd", "Setup.Ld_ULd.html",
    kLists, kListNames, 1,
    kLU_SaveReads, (int)(sizeof(kLU_SaveReads) / sizeof(kLU_SaveReads[0])),
    &LU_FormShow, &LU_spbSaveClick, "SaveSetupFile", &LU_ReadFile, &Booted,
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
