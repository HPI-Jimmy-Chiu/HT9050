// ===========================================================================
//  FileRW/ArmSpeed_File.cpp -- ArmSpeed_File 族（ArmSpeed_File[]／SHSpeed_File／MGSpeed_File）的讀寫檔
//  （<recipe>\ArmCondition.Data；A10 存到 machine 時 sSaveByMachine）。
//
//  Steven 20260924.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（ArmSpeed_File）。
//
//  golden TfSpeed（cSpeed.cpp）由 tools/gen_editlist.py 轉成 ArmSpeed_File.gen.inc（元件改成具名替身）：
//    FormShow（:41）＝開頁（ReadFile＋DoIniDataToForm＋權限）；spbSaveClick（:1433）＝存檔鈕（逐鍵 WriteIniData
//    ＋ReadWriteFile(false)＋存後 ReadFile／SetWorkParameter…）。沒有 HTEditList —— 存檔流程讀的替身全部是
//    mustSend（頁面必須送回開頁時拿到的值）。
//  原本的 A 形狀 bridge（tools/formbridge/TfSpeed.py）只能顯示、不能存，已退役（tools/formbridge/_retired/）。
//  開機讀檔仍是移植樹 fSpeed->ReadFile()（cSpeed.cpp W906_BootReadHotPlateAndSpeed，與 golden 讀法逐行相同）。
// ===========================================================================
#include "FileRW/ArmSpeed_File.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

const filerw::PageDesc kPage = {
    "ArmSpeed_File", "TfSpeed", "Setup.Speed.html",
    nullptr, nullptr, 0,
    kSP_SaveReads, (int)(sizeof(kSP_SaveReads) / sizeof(kSP_SaveReads[0])),
    &SP_FormShow, &SP_spbSaveClick, "spbSaveClick", &SP_ReadFile, &Booted,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfSpeed 建構（HT9045.cpp CreateForm）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身
void FileRW_Speed_Boot()
{
    if (g_booted) return;
    SP_DfmItems();
    SP_DfmState();
    SP_TfSpeed();
    SP_CreateSaveProxies();
    SP_CreateContainerProxies();
    std::printf("FileRW ArmSpeed_File: TfSpeed proxies ready (%d save reads) -- golden cSpeed.cpp\n",
                (int)(sizeof(kSP_SaveReads) / sizeof(kSP_SaveReads[0])));
    g_booted = true;
}
