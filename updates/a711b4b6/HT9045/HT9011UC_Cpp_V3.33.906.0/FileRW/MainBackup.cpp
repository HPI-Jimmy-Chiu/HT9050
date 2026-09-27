// ===========================================================================
//  FileRW/MainBackup.cpp -- golden TfMain::BackupSetupFile（V912 main.cpp:34053-34127）的本體：
//    配方夾的 MD5 檢查碼（SetMD5ByFolder）＋客戶專屬的配方備份。
//
//  AI(W906-FRW-S92) 20260926（Steven 團隊）：新檔。RULINGS_20260926 S92（唯讀普查 S91～S105，S52 範圍）。
//    golden 一律照主 repo 那份 V912：D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（cp950 → UTF-8），
//    行號都是那一份的。
//
//  為什麼要有這支：移植樹 forms/fMain.cpp:458 `void TfMain::BackupSetupFile() {}` 是空殼，但移植樹有 27 個程式碼
//    呼叫點（5 個在 `#if 0` 裡），其中 wb_serve 網頁存檔真的會走到的是 FileRW 的 15 個頁面存檔流程
//    （Setup.Speed／BinSel／Contact／HotPlate／Ld_ULd／Temp_Set／BarCode／Cleaning／QAMode／SetUp／TesterIF／
//    YieldMonitoring／TrayAssignment／TrayForm、HW.VacuumUnit）⇒ 每一頁存檔都照 golden 叫了 BackupSetupFile，
//    但什麼都沒做：配方夾裡的 *.MD5 一直是 BCB6 上次存檔時寫的，網頁改過的配方 MD5 就對不上
//    （20260926 唯讀量這台 D:\HT9045\IniData\Data：有 1 個 MD5 的 195 個配方夾裡 88 個對不上內容）。
//    MD5 的讀者：golden 全樹只有 uLotInfo.cpp:4645 CompareMD5ByFolder（從 Server 下載配方之後比對，S105 範圍）。
//
//  接法（安裝座，跟 forms/fMain.h 的 W906_ClarnDataBody／W906_StateRecordBody 同一個作法與理由）：
//    forms/fMain.cpp（ht9045_forms）不能直接呼叫本檔 —— 本檔只編進 wb_serve；很多 ctest 會把 fMain.o 連進去
//    （TfMain 的 vtable 引用 BackupSetupFile），直接呼叫會讓那些測試連結失敗（FileRW/_fallback.cpp 檔頭記的就是同一件事）。
//    所以 fMain.cpp:458 改成經函式指標轉（指標定義在 fMain.cpp 同一行，預設 0 ＝ 原本的 no-op），
//    wb_serve 開機時明確呼叫 W906_FRW_InstallBackupSetupFile() 裝上（不用 static init 自我登錄：陷阱 #2，forms/fMain.h:1326-1332（該段 :1331））。
//    ⇒ 測試執行檔沒裝，BackupSetupFile 維持 no-op，ctest 不會去改真配方夾。
//    fMain.cpp、wb_serve.cpp、CMakeLists.txt 都是共用檔，本檔不改它們；三段片段見交件報告，**要一起套**：
//      只套 CMakeLists ＋ wb_serve、沒套 fMain.cpp ⇒ W906_BackupSetupFileBody 未定義，wb_serve 連結失敗（看得見）；
//      只套 fMain.cpp ⇒ 指標沒人裝，維持 no-op（跟現在一樣）。
//
//  ⚠ 會碰到的真實檔（每一次頁面存檔）：
//    D:\HT9045\IniData\Data\<目前配方>\*.MD5 —— 先刪掉夾裡全部的 *.MD5，再寫一個 <32 碼 hex>.MD5，內容是夾裡每個
//      *.Data 的完整路徑一行一個（CRLF）。hex ＝ 依 FindFirstFile 順序把夾裡所有 *.Data 串起來的 MD5（Public/HTMD5.cpp:448）。
//      「目前配方」＝ fMain->cbSetupFileName->Text（wb_serve.cpp:4111 開機設、WebRecipeChange.cpp:302 換配方設）。
//      W906_INIDATA_ROOT 可把 DataPath 導開（common.cpp:204）。夾裡沒有 *.Data ⇒ 寫 "No File Exist.MD5"（golden 同）。
//    以下兩段只在客戶條件成立時跑（S25 客戶碼，照 golden 保留、只註記；這台 CUSTOMER_CODE 不是這兩家就不會跑）：
//    D:\HT9045_Backup\SetupFile\<配方>\ —— CosFunction.bUseAutoBackUpSetupFile（只有 FUNC_CC_ASE_M，CosFunction.cpp:1031）
//      且 IniConfig.bA24AutoBackupSetupFile：建資料夾，把配方夾每個檔複製過去（覆蓋）。寫死路徑，沒有導開。
//    D:\SetupFile\ —— CUSTOMER_CODE==CC_ASE_SG：**整個資料夾先刪掉**（DeleteDirectory）再重建，複製目前配方。
//  CC_ASE_KaohSiung：一進來就 return（golden :34058-34059），什麼都不寫。
//
//  跟 golden 不同的地方（前提不在或 C++ 語法，不是改行為）：
//    ① CopyFile → CopyFileA（明寫 ANSI 版，跟 WebRecipeChange.cpp 的 FindFirstFileA／DeleteFileA 同一個慣例）。
//    ② golden 兩個 FindFirst/FindNext 迴圈都沒有 FindClose（BCB6 會漏一個搜尋 handle）；vclcompat 的 TSearchRec
//       解構子會關掉（vclcompat/SysUtils.h:168），所以這裡不漏。
//    ③ 目前配方夾不存在時：golden SetMD5ByFolder 的 SaveToFile 會丟 EFCreateError（存檔鈕的事件處理中斷、跳錯誤框）；
//       vclcompat TStringList::SaveToFile 開不了檔就靜靜 return（vclcompat/TStringList.cpp:262-264）⇒ 不寫、不中斷。
//    ④ 每次呼叫印一行 stdout（golden 沒有），方便對照網頁存檔有沒有走到這裡。
//
//  建置：本檔不屬於 gen_editlist.py／gen_formbridge.py 的產生清單，照 FileRW/MainBoot.cpp 的前例列在 CMakeLists.txt 的
//    add_executable(wb_serve …)。本檔只被 wb_serve 用；不要搬進任何 archive（陷阱 #2）。
// ===========================================================================
#include "forms/fMain.h"            // fMain->cbSetupFileName->Text（golden main.h:875 TComboBox*）
#include "cmydef.h"                 // CUSTOMER_CODE
#include "MachineType.h"            // CC_ASE_KaohSiung（936）／CC_ASE_SG（930）
#include "common.h"                 // DataPath／MyForceDirectories
#include "cpublic.h"                // SetMD5ByFolder（本體 cpublic.cpp:993，golden cpublic.cpp:846）
#include "Config.h"                 // IniConfig.bA24AutoBackupSetupFile
#include "CosFunction.h"            // CosFunction.bUseAutoBackUpSetupFile
#include "Public/ExternFunction.h"  // DeleteDirectory（本體 Public/ExternFunction.cpp:182）
#include "vclcompat/SysUtils.h"     // TSearchRec／FindFirst／FindNext／DirectoryExists

#include <windows.h>                // CopyFileA
#include <cstdio>

// forms/fMain.cpp:458 的安裝座（片段套上之後在那一行定義，預設 0）。型別要跟那一行一字不差：
// 變數的名字不帶型別，對不上連結器也不會說。AI(W906-FRW-S92) 20260926
extern void (*W906_BackupSetupFileBody)();

namespace {

int g_calls = 0;                    // ④ 觀測用：本次執行叫過幾次

// ---------------------------------------------------------------------------
//  golden main.cpp:34053  void __fastcall TfMain::BackupSetupFile()
//  （V3.27G.531 Ifor 20170620 add Backup Setup File）。golden 本體裡只用 fMain->（不用 this），所以這裡是自由函式。
// ---------------------------------------------------------------------------
void W906_TfMain_BackupSetupFile()
{
    AnsiString sPath="D:\\HT9045_Backup\\";                                    // golden :34055
    AnsiString sFileName=fMain->cbSetupFileName->Text;                          // golden :34056
    AnsiString sSetupFile="";
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20180824   // golden :34058（客戶碼，S25：照翻）
    {
        std::printf("FileRW MainBackup: BackupSetupFile skipped -- CUSTOMER_CODE==CC_ASE_KaohSiung (golden main.cpp:34058-34059)\n");
        return;
    }
    SetMD5ByFolder(DataPath+sFileName);                                         //Steven 20170927 (wei) : 將工作檔加入檢查碼   // golden :34060
    ++g_calls;

    bool bA24Backup=false, bAseSgBackup=false;
    // ---- 客戶碼（S25）：bUseAutoBackUpSetupFile 只有 FUNC_CC_ASE_M 設 true（CosFunction.cpp:1031）。照 golden 保留、只註記。
    if(CosFunction.bUseAutoBackUpSetupFile==true && IniConfig.bA24AutoBackupSetupFile==true)   // golden :34062
    {
        if(MyForceDirectories(sPath)==false)
        {
            return;
        }

        sPath=sPath+"SetupFile\\";
        if(MyForceDirectories(sPath)==false)
        {
            return;
        }

        sPath=sPath+sFileName+"\\";
        if(MyForceDirectories(sPath)==false)
        {
            return;
        }

        sSetupFile=DataPath+sFileName+"\\*.*";
        TSearchRec srFile;
        if(FindFirst(sSetupFile, 0, srFile)==0)
        {
            do
            {
                AnsiString sCopyFile    =DataPath+sFileName+"\\"+srFile.Name;
                AnsiString sBackupFile  =sPath+srFile.Name;
                CopyFileA(sCopyFile.c_str(), sBackupFile.c_str(), false);       // ① golden CopyFile
            }
            while(FindNext(srFile)==0);
        }
        bA24Backup=true;
    }

    // ---- 客戶碼（S25）：CC_ASE_SG。照 golden 保留、只註記。⚠ 會整個刪掉 D:\SetupFile\ 再重建。
    if(CUSTOMER_CODE==CC_ASE_SG)                                                // golden :34095
    {
        sPath="D:\\SetupFile\\";
        if(DirectoryExists(sPath)==true)
        {
            DeleteDirectory(sPath);
            MyForceDirectories(sPath);
        }
        else
        {
            MyForceDirectories(sPath);
        }

        sPath=sPath+sFileName+"\\";
        if(DirectoryExists(sPath)==false)
        {
            MyForceDirectories(sPath);
        }

        sSetupFile=DataPath+sFileName+"\\*.*";
        TSearchRec srFile;
        if(FindFirst(sSetupFile, 0, srFile)==0)
        {
            do
            {
                AnsiString sCopyFile    =DataPath+sFileName+"\\"+srFile.Name;
                AnsiString sBackupFile  =sPath+srFile.Name;
                CopyFileA(sCopyFile.c_str(), sBackupFile.c_str(), false);       // ① golden CopyFile
            }
            while(FindNext(srFile)==0);
        }
        bAseSgBackup=true;
    }                                                                           // golden :34126

    std::printf("FileRW MainBackup: golden TfMain::BackupSetupFile (main.cpp:34053) #%d -- SetMD5ByFolder(%s)%s%s\n",
                g_calls, (DataPath+sFileName).c_str(),
                bA24Backup   ? "; A24 backup -> D:\\HT9045_Backup\\SetupFile\\" : "",
                bAseSgBackup ? "; CC_ASE_SG backup -> D:\\SetupFile\\" : "");
}

}  // namespace

// ---------------------------------------------------------------------------
//  安裝座。wb_serve 開機時呼叫一次（tools/wb_serve.cpp，片段見交件報告；位置在 cbSetupFileName->Text 設好之後）。
//  裝上之後，移植樹既有的 fMain->BackupSetupFile() 呼叫點（見檔頭）全部從 no-op 變成真的寫 MD5。
// ---------------------------------------------------------------------------
void W906_FRW_InstallBackupSetupFile()
{
    W906_BackupSetupFileBody=&W906_TfMain_BackupSetupFile;
    std::printf("FileRW MainBackup: TfMain::BackupSetupFile body installed (golden main.cpp:34053; every page save now "
                "rewrites %s<recipe>\\*.MD5; CUSTOMER_CODE=%d)\n", DataPath.c_str(), CUSTOMER_CODE);
}

bool W906_FRW_BackupSetupFileInstalled() { return W906_BackupSetupFileBody==&W906_TfMain_BackupSetupFile; }
