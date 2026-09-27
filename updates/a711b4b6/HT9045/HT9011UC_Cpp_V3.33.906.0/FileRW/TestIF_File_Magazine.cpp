// ===========================================================================
//  FileRW/TestIF_File_Magazine.cpp -- golden TfMagazine（Magazine.cpp，V912）的 C 路入口：
//  <配方>\HandlerCondition.Data [Configuration] 三鍵＋AuthPath+config.ini [Magazine Z Offset] 32 鍵的寫檔段。沒有頁面。
//
//  //AI(W906-FRW-S62) 20260926: 新檔（Steven 團隊，S62）。設定：tools/editlist/TestIF_File_Magazine.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 TestIF_File_Magazine.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    建構子（:44）            ＝ FileRW_Magazine_Boot()：CreateForm（HT9045.cpp:274）。fShow=false、EditInMag[16]／EditOutAuto[16]
//                                指向 32 個輸入框替身。不讀檔、不寫檔。
//    DoIniDataToForm（:3654） ＝ FileRW_Magazine_DoIniDataToForm()：TestIF_File.iMagTraySource／iMagFixTrayType／iMagDisplayOrder＋
//                                fMagazine->iInMagOfs／iOutAutoOfs → 替身。只寫替身。
//                                ⚠ golden DoReadLastData 沒有呼叫它（main.cpp:9381-9411 的 DoIniDataToForm 段沒有 fMagazine）——
//                                golden 只在 FormShow :88、FormClose :354、spbSaveClick :3609 呼叫 → 開機鏈不接，目前沒有呼叫者。
//    spbSaveClick（:3547）    ＝ FileRW_Magazine_spbSaveClick()：golden 存檔鈕。⚠ 目前沒有呼叫者（沒有頁面、沒有 WS 指令、
//                                沒有登記 PageDesc）。寫 HandlerCondition.Data 三鍵（Mag Fix tray Type 三道檢查不過只跳訊息、這一鍵不寫）、
//                                D:\HT9045\config\config.ini [Magazine Z Offset] 32 鍵 → 顯示順序有變就 fShowBinSelect->InitShowBinDigital()
//                                → fMagazine->ReadFile()（移植樹 Magazine.cpp 那一支）→ DoIniDataToForm。golden 這支沒有 BackupSetupFile。
//  讀檔器 ReadFile（:3613）是移植樹 Magazine.cpp 的那一支（逐字、開機鏈 golden :9351／:9357 兩次都已接）；本檔不重複。
//
//  ---- golden 呼叫點 → 本檔函式 --------------------------------------------------------------------------------------
//    開機：CreateForm(TfMagazine)（HT9045.cpp:274）→ FileRW_Magazine_Boot()：可以不接（下面兩支第一次呼叫時自己建）；
//          建構子沒有讀檔、沒有 HTEditList，順序不影響任何值。
//    開機／換配方：DoReadLastData :9351、:9357 fMagazine->ReadFile()（已接，tools/wb_serve.cpp）；golden 沒有 DoIniDataToForm → 不接。
//    開頁：TfMain::sbMagazineClick main.cpp:35060（權限 fSecurity->Insufficient(174)）→ ShowModal → FormShow :82：
//          ReadFile :87 → DoIniDataToForm :88 → 權限／標籤 → iMagFixTrayType==1 時改 MOT 盤面資料（不是讀寫檔，見設定檔檔頭）。沒有頁面，不接。
//
//  ---- 存檔前替身必須已是頁面值 ------------------------------------------------------------------------------------
//    golden 的存檔鈕只有 FormShow 之後才按得到（FormShow :87-88 先 ReadFile＋DoIniDataToForm）。本檔的替身開機只有 DFM 設計期值
//    （32 個輸入框都是 "0"；ReadFile 的預設值是 -3／3）—— ⚠ 替身沒灌過值就呼叫 FileRW_Magazine_spbSaveClick()，會把 32 個 0 寫進
//    config.ini。頁面接上時照 PageDesc 慣例：editlist.get＝FormShow（至少 fMagazine->ReadFile()＋FileRW_Magazine_DoIniDataToForm()）、
//    editlist.save＝套頁面值 → MG_spbSaveClick。
//    ⚠ kMG_SaveReads 只列到三個 TRadioGroup：32 個 Z offset 輸入框是經 EditInMag[i]／EditOutAuto[i] 指標表讀的，產生器掃
//    EL<> 字面值掃不到 —— 登記 PageDesc 時 mustSend 要自己補 edInMagOfs01..16、edOutAuto3Ofs01..16。
// ===========================================================================
#include "FileRW/TestIF_File_Magazine.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TfMagazine 建構（HT9045.cpp:274 CreateForm）：DFM 設計期狀態 → 建構子（:44）→ 存檔流程讀的替身 → 容器替身與父子。冪等。
// 不讀檔、不寫檔。
void FileRW_Magazine_Boot()
{
    if (g_booted) return;
    MG_DfmItems();
    MG_DfmState();
    MG_TfMagazine();
    MG_CreateSaveProxies();
    MG_CreateContainerProxies();
    std::printf("FileRW Magazine: TfMagazine proxies ready (%d save reads + 32 Z-offset edits via EditInMag/EditOutAuto) -- "
                "golden Magazine.cpp ctor :44 (CreateForm HT9045.cpp:274)\n",
                (int)(sizeof(kMG_SaveReads) / sizeof(kMG_SaveReads[0])));
    g_booted = true;
}

// golden TfMagazine::DoIniDataToForm（:3654）。golden 只有 FormShow／FormClose／spbSaveClick 呼叫（開機鏈沒有）→ 目前沒有呼叫者。
void FileRW_Magazine_DoIniDataToForm()
{
    if (!g_booted) FileRW_Magazine_Boot();
    MG_DoIniDataToForm();
}

// golden 存檔鈕 TfMagazine::spbSaveClick（:3547）。目前沒有呼叫者（沒有頁面）；給之後的頁面／WS 指令用。
// ⚠ 呼叫前替身必須已是頁面值（見檔頭）。會寫 <DataPath>\<配方>\HandlerCondition.Data 與 AuthPath+config.ini，最後重讀。
void FileRW_Magazine_spbSaveClick()
{
    if (!g_booted) FileRW_Magazine_Boot();
    MG_spbSaveClick();
}
