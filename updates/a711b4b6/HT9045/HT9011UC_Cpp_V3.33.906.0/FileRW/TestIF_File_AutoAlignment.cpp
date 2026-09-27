// ===========================================================================
//  FileRW/TestIF_File_AutoAlignment.cpp -- golden TfAutoAlignment（AutoAlignment\AutoAlignment.cpp，V912）的 C 路入口：
//  <配方>\HandlerCondition.Data [AutoAlignmrnt] 的寫檔段＋開機 DoIniDataToForm。沒有頁面（Steven 20260926）。
//
//  //AI(W906-FRW-NoPage) 20260926: 新檔（Steven 團隊）。設定：tools/editlist/TestIF_File_AutoAlignment.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 TestIF_File_AutoAlignment.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    建構子（:47）            ＝ FileRW_AutoAlignment_Boot()：CreateForm（HT9045.cpp:270）。Gerneral.ini [Auto_Alignment]
//                                AUTO_ALIGNMENT_CCD1/2_ADRESS／_PORT → 四個輸入框（CheckAndReadIniDataGeneral：鍵不在就照 golden 補寫預設值）。
//    DoIniDataToForm（:1433） ＝ FileRW_AutoAlignment_DoIniDataToForm()：golden DoReadLastData main.cpp:9434（開機與換配方）。
//                                ⚠ 不只是顯示：golden 在這裡把 TestIF_File.iAutoAlignmentTrayEvent |= AutoAlignmentTray_AfterHome、
//                                iAutoAlignmentShuttleHotplateEvent |= AutoAlignmentCK_AfterHome（每次開機／換配方都強制帶「回 Home 後」）。
//    spbSaveClick（:1573）    ＝ FileRW_AutoAlignment_spbSaveClick()：golden 存檔鈕。⚠ 目前沒有呼叫者（沒有頁面、沒有 WS 指令、
//                                沒有登記 PageDesc）。沒有 CCD（MACHINE_HAS_AUTO_ALIGNMENT_CCD==0，本機 Gerneral.ini:158）時 golden 在
//                                CheckAutoAlignmentEvent 之後就 return，不寫檔。
//  讀檔器 ReadFile（:1373）是移植樹 AutoAlignment/AutoAlignment.cpp 的那一支（逐字、開機鏈 golden :9433 已接）；本檔不重複。
//
//  ---- golden 呼叫點 → 本檔函式（呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）------------------------------
//    開機：CreateForm(TfAutoAlignment)（HT9045.cpp:270，TfGroundMan :262 之後）→ FileRW_AutoAlignment_Boot()（要在 LoadMachineConfig 之後）。
//    開機／換配方：DoReadLastData :9432 fAutoAlignment->ReadFile()（已接）→ :9434 FileRW_AutoAlignment_DoIniDataToForm()。
//    main.cpp:9734 的 fAutoAlignment->ReadFile() 在 `#ifdef ASE_KaohSiung`（編譯期客戶版號，移植樹不定義）→ 不接。
// ===========================================================================
#include "FileRW/TestIF_File_AutoAlignment.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TfAutoAlignment 建構（HT9045.cpp:270 CreateForm）：DFM 設計期狀態 → 建構子（:47）→ 存檔流程讀的替身 → 容器替身與父子。冪等。
// ⚠ 會讀、缺鍵時會寫 D:\HT9045\system\Gerneral.ini [Auto_Alignment]（golden CheckAndReadIniDataGeneral；每台機台開機都這樣，
//   不看 MACHINE_HAS_AUTO_ALIGNMENT_CCD）。四個鍵都在的檔不會被改（本機 Gerneral.ini:561-564 都在）。
void FileRW_AutoAlignment_Boot()
{
    if (g_booted) return;
    AA_DfmItems();
    AA_DfmState();
    AA_TfAutoAlignment();
    AA_CreateSaveProxies();
    AA_CreateContainerProxies();
    std::printf("FileRW AutoAlignment: TfAutoAlignment proxies ready (%d save reads) -- golden AutoAlignment.cpp ctor :47 "
                "(Gerneral.ini [Auto_Alignment]): CCD1=%s:%s CCD2=%s:%s MACHINE_HAS_AUTO_ALIGNMENT_CCD=%d\n",
                (int)(sizeof(kAA_SaveReads) / sizeof(kAA_SaveReads[0])),
                EL<TEdit>("TfAutoAlignment", "edtInArmAddress")->Text.c_str(), EL<TEdit>("TfAutoAlignment", "edtInArmPort")->Text.c_str(),
                EL<TEdit>("TfAutoAlignment", "edtOutArmAddress")->Text.c_str(), EL<TEdit>("TfAutoAlignment", "edtOutArmPort")->Text.c_str(),
                (int)MACHINE_HAS_AUTO_ALIGNMENT_CCD);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9434 `fAutoAlignment->DoIniDataToForm();`（開機與換配方，緊接在 :9432 ReadFile 之後）。
void FileRW_AutoAlignment_DoIniDataToForm()
{
    if (!g_booted) FileRW_AutoAlignment_Boot();   // golden 的建構一定在 DoReadLastData 之前（CreateForm 先於 TfMain::FormShow）
    AA_DoIniDataToForm();
    std::printf("autoalign.* DoIniDataToForm (golden main.cpp:9434): TrayEvent=0x%lx ShuttleHotplateEvent=0x%lx "
                "TrayEvent_Z=0x%lx ShuttleHotplateEvent_Z=0x%lx\n",
                (unsigned long)TestIF_File.iAutoAlignmentTrayEvent, (unsigned long)TestIF_File.iAutoAlignmentShuttleHotplateEvent,
                (unsigned long)TestIF_File.iAutoAlignmentTrayEvent_Z, (unsigned long)TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z);
}

// golden 存檔鈕 TfAutoAlignment::spbSaveClick（:1573）。目前沒有呼叫者（沒有頁面）；給之後的頁面／WS 指令用。
// 會寫 <DataPath>\<配方>\HandlerCondition.Data [AutoAlignmrnt]（有 CCD 時），最後重讀（fAutoAlignment->ReadFile）。
void FileRW_AutoAlignment_spbSaveClick()
{
    if (!g_booted) FileRW_AutoAlignment_Boot();
    AA_spbSaveClick();
}
