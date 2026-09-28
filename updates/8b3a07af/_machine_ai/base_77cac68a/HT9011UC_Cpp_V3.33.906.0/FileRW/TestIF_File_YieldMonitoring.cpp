// ===========================================================================
//  FileRW/TestIF_File_YieldMonitoring.cpp -- TestIF_File 的 TfYieldMonitoring 半邊（<recipe>\Tester.Data）讀寫檔，C 形狀。
//
//  Steven 團隊 20260925.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md（TestIF_File）。
//  結構名加後綴 _YieldMonitoring：FileRW/TestIF_File.cpp 是 A 形狀（golden TFTestIF 的 bridge），同一個結構、同一個檔
//  （Tester.Data）的另一半 —— 兩個表單各寫自己的鍵（golden 也是兩支 SaveSetupFile 寫同一個檔）。
//
//  golden TfYieldMonitoring（uYieldMonitoring.cpp）由 tools/gen_editlist.py 轉成 TestIF_File_YieldMonitoring.gen.inc：
//    FormShow（:2581）＝開頁（golden ReadFile（:900，尾 DoIniDataToForm）＋顯示／權限）；
//    btnApplyClick（:3092）＝存檔鈕（A02 守衛 → DoFormToData → CheckSettingNo → SaveSetupFile → SECS）。
//  沒有 HTEditList —— 存檔流程讀的具名替身全部是 mustSend；動態 Category 元件（12×256）不在 mustSend
//  （產生器只掃字面名稱），頁面沒送就用開頁時 golden DoIniDataToForm 填的值。
//  原本的 A 形狀 bridge（tools/formbridge/TfYieldMonitoring.py）未退役（整合者決定）。
//  開機讀檔仍是移植樹 fYieldMonitoring->ReadFile()（uYieldMonitoring.cpp:2676，ReadIniData 全在、只擋 UI）；
//  開頁才跑 golden 912 ReadFile（同 FileRW/ArmSpeed_File.cpp 的做法）。
// ===========================================================================
#include "FileRW/TestIF_File_YieldMonitoring.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

const filerw::PageDesc kPage = {
    "TestIF_File_YieldMonitoring", "TfYieldMonitoring", "Setup.YieldMonitoring.html",
    nullptr, nullptr, 0,
    kYM_SaveReads, (int)(sizeof(kYM_SaveReads) / sizeof(kYM_SaveReads[0])),
    &YM_FormShow, &YM_btnApplyClick, "SaveSetupFile", &YM_ReadFile, &Booted,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfYieldMonitoring 建構（HT9045.cpp CreateForm）：DFM 設計期狀態 → 建構子 → OnCreate（FormCreate，動態元件）
// → 存檔流程讀的替身 → 容器替身與父子
void FileRW_YieldMonitoring_Boot()
{
    if (g_booted) return;
    YM_DfmItems();
    YM_DfmState();
    YM_TfYieldMonitoring();
    YM_FormCreate();
    YM_CreateSaveProxies();
    YM_CreateContainerProxies();
    std::printf("FileRW TestIF_File_YieldMonitoring: TfYieldMonitoring proxies ready (%d save reads, %d dynamic Category widgets) -- golden uYieldMonitoring.cpp\n",
                (int)(sizeof(kYM_SaveReads) / sizeof(kYM_SaveReads[0])), 12 * TEST_MAX_BIN);
    g_booted = true;
}
