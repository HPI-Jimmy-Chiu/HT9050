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
#include "FileRW/MainClickTail.h"   // AI(W906-FRW-S158) 20260927 [W906]: W906_Main_sbYieldClickTail（FileRW/MainClick.cpp；Q41 第 3 項 YM-5）；佔用原本的空行
#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }
void SaveFlow();   // AI(W906-FRW-S158) 20260927 [W906]: 定義在檔尾（golden 存檔鈕＋TfMain::sbYieldClick 關窗尾段，R85）；佔用原本的空行
const filerw::PageDesc kPage = {
    "TestIF_File_YieldMonitoring", "TfYieldMonitoring", "Setup.YieldMonitoring.html",
    nullptr, nullptr, 0,
    kYM_SaveReads, (int)(sizeof(kYM_SaveReads) / sizeof(kYM_SaveReads[0])),
    &YM_FormShow, &SaveFlow, "SaveSetupFile", &YM_ReadFile, &Booted,   // AI(W906-FRW-S158) 20260927 [W906]: saveFlow &YM_btnApplyClick → &SaveFlow（檔尾：存檔鈕之後補關窗尾段）；同一行改寫
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

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]: Q41 第 3 項 YM-5（decisions-pending R85＝A：RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」
//    的延伸）—— PageDesc::saveFlow 包一層：golden 存檔鈕 btnApplyClick（uYieldMonitoring.cpp:3114）跑完之後，補 golden 主畫面
//    TfMain::sbYieldClick 的關窗尾段（V912 main.cpp:28510-28512：DoStructUnitConvert、SetWorkParameter、SetStartModeData；本體
//    FileRW/MainClick.cpp W906_Main_sbYieldClickTail，運轉中不跑 R86）。照 FileRW/Ld_UldDelayTime.cpp 的做法：
//      (1) A02 權限不足（"closed"）—— golden :3120 Close() → FormClose（uYieldMonitoring.cpp:3288：只有 fShow=false）→ ShowModal 回來
//          → 尾段：與 golden 相同。A02 在 DoFormToData（:3125）之前 return，TestIF_File 沒被頁面值動過，不必另外還原
//          （頁面替身由 PageSave 接著跑的 YM_ReadFile 還原：FileRW/_EditPage.cpp PageSave 的 `if (!saved) d.reload();`）。
//      (2) 真的寫了檔（"SaveSetupFile"）—— golden 存完視窗還開著、等 Exit／OK 才跑 FormClose＋尾段；網頁 Exit 不送伺服器 → 存完就跑（S107-1）。
//    差別：golden「開頁、沒存就關」也跑尾段；這裡不跑（值沒變，結果相同）。golden btnOkClick（:3190，ReadFile＋主畫面 Yield 狀態＋Close
//    → 同一個尾段）是盤點 YM-3，另案。
//    ⚠ SetStartModeData 會寫檔、記事件（RunMode.txt、MES2107／ChangeLog、lastdata.dat，見 FileRW/MainClick.cpp 第二批檔頭）——
//      golden 一次開關記一次，網頁每存一次記一次。
//    描述檔 tools/editlist/TestIF_File_YieldMonitoring.py 不改、gen.inc 不重產。
// ---------------------------------------------------------------------------
namespace {
void SaveFlow()
{
    YM_btnApplyClick();                                                         // golden uYieldMonitoring.cpp:3114
    if (filerw::ELMarked("closed")) {
        YM_FormClose();                                                         // golden uYieldMonitoring.cpp:3288
        if (const char* w = W906_Main_sbYieldClickTail()) filerw::ELTodo(w);    // golden main.cpp:28510-28512
    } else if (filerw::ELMarked("SaveSetupFile")) {
        if (const char* w = W906_Main_sbYieldClickTail()) filerw::ELTodo(w);    // golden main.cpp:28510-28512（時機見上）
    }
}
}  // namespace
