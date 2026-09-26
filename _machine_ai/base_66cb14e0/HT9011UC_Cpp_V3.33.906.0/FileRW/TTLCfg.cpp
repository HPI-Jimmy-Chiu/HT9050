// ===========================================================================
//  FileRW/TTLCfg.cpp -- TTLCfg 的讀寫檔（C 形狀：具名替身）
//  （<recipe>\<DIO>.ini（IniConfig.bI16TTLSaveInSetupFile）或 DIOCFGPath\<DIO>.ini）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TTLCfg.py（每條 replace 附原因）。
//
//  原本寫成 FileRW/TTLCfg_C.cpp（當時 FileRW/TTLCfg.cpp 是 A 形狀 kBridge_TfDIOFrom）；整合者 20260925 退役 A 形狀後改名。
//
//  golden TfDIOFrom（DIOInterFaceCFG.cpp，912）由 tools/gen_editlist.py 轉成 TTLCfg.gen.inc。本檔另外手寫
//  golden TFTestIF（cTesterIF.cpp）的三段，替身掛在 golden 表單類別 "TFTestIF" 下：
//    TI_InitcbDIOType        golden cTesterIF.cpp:70-106   列 DIOCFGPath\*.ini 當 cbDIOType->Items
//    TI_DoIniDataToForm_DIO  golden cTesterIF.cpp:1089-1105（TFTestIF::DoIniDataToForm 的 DIO 段）
//                            依 TestIF_File.sDioName／iDioMode 選 cbDIOType->ItemIndex（→ Text）
//    FileRW_TTLCfg_TFTestIF_cbDIOTypeChange  golden cTesterIF.cpp:1299-1305（TesterIF 換 DIO 型態）
//  DIO 檔名＝cbDIOType->Text（golden GetDIOFileName :55/:58/:67）—— Steven 20260925 指示，不再用 TestIF_File.sDioName 代替。
//
//  開頁（editlist.get）＝golden 的順序：
//    0. FileRW_TTLCfg_ReadDIOSection() —— golden ReadTestIFFile 的 [DIO] 段（只讀，不回寫 TypeName），開機也呼叫
//    1. TI_InitcbDIOType(false)   —— golden TFTestIF 建構子 cTesterIF.cpp:43（CreateForm HT9045.cpp:189，早於
//       TfDIOFrom 的 :196）。bAlarm=false：golden 開 TfDIOFrom 的呼叫端 main.cpp:28658 sbDioSetClick 本身不呼叫
//       InitcbDIOType，cbDIOType 的內容來自開機建構子那一次（false）。golden 在這裡找不到 DIO 檔會
//       MessageBox＋Application->Terminate()；網頁版改 ELMessage、記 g_dioListLost → 存檔一律拒絕，不結束程式。
//    2. TI_DoIniDataToForm_DIO()  —— golden 開機 main.cpp:9327 FTestIF->ReadTestIFFile()（尾端 :994 DoIniDataToForm）
//    3. DI_FormShow()             —— golden :24（InitData 清空、DIOFileName=""）
//    4. DI_spbLoadClick()         —— golden :238，對話框換成「選本配方目前的 DIO 檔」＝GetDIOFileName()→LoadData→DoIniDataToForm
//  存檔（editlist.save）＝golden spbSaveClick :191；寫了檔之後照 golden 關表單的順序（main.cpp:28668-28669）
//  GetDIOFileName＋LoadData 重讀進 TTLCfg（網頁沒有「關表單」事件；golden 存檔鈕只更新 TTLCfg 的 3 個時間欄位，
//  其餘欄位與 iCateParity 要等關表單的 LoadData 才進執行期）。
//
//  ⚠ 輸入來源（照實）：TestIF_File.sDioName／iDioMode／iTestType 在 golden 只有 TFTestIF::ReadTestIFFile
//    （cTesterIF.cpp:563）讀檔填 —— 移植樹 forms/fTesterIF.cpp:800 GATE (F-5)，沒有其他讀者。
//    Steven 20260925：sDioName／iDioMode 改由本檔 FileRW_TTLCfg_ReadDIOSection()（[DIO] 段，只讀）填；
//    iTestType（[Mode] Tester Type，:573）仍依賴 F-5（移植樹預設 0＝TTL_MODE）。開頁的 ELTodo 回報實際選到的名稱。
//
//  副作用（照 golden，未改）：
//    * GetDIOFileName :56-62：bI16TTLSaveInSetupFile 且配方裡沒有 <DIO>.ini 時，CopyFile 母檔進配方資料夾（開頁就會）。
//    * LoadData :78-82：TTL 模式（移植樹 iTestType 預設 0＝TTL_MODE）且 DIO 檔不存在 → SystemStart=false（安全側）。
//      A 形狀（FileRW/TTLCfg.cpp）選 ELTodo 不清；本檔照 golden 清。Steven 可改判（tools/editlist/TTLCfg.py 檔頭）。
//    * LoadData 寫全域 TTLCfg（執行期那一份）—— golden 開機／關表單同樣寫它。
// ===========================================================================
#include "FileRW/TTLCfg.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// golden InitcbDIOType 找不到任何 DIO 檔（golden 在這裡 Application->Terminate()）→ 網頁版拒絕存檔
bool g_dioListLost = false;

TComboBox* cbDIOType() { return EL<TComboBox>("TFTestIF", "cbDIOType"); }   // golden cTesterIF.h:23／cTesterIF.dfm:85

// VCL TCustomComboBox 的語意（vclcompat TComboBox 的 Clear／ItemIndex 是裸欄位，不會連動 Text）：
//   Clear() 清 Items 且 Text=""、ItemIndex=-1；設 ItemIndex＝i：範圍內 → Text=Items[i]，範圍外（含 -1）→
//   CB_SETCURSEL 清掉選取與編輯框（Text=""、ItemIndex=-1）。程式設 ItemIndex 不觸發 OnChange（同 VCL）。
void ComboClear(TComboBox* cb)
{
    cb->Clear();
    cb->ItemIndex = -1;
    cb->Text = "";
}
void ComboSetItemIndex(TComboBox* cb, int i)
{
    if (i >= 0 && i < cb->Items->Count) {
        cb->ItemIndex = i;
        cb->Text = cb->Items->Strings[i];
    } else {
        cb->ItemIndex = -1;
        cb->Text = "";
    }
}

// golden cTesterIF.cpp:70  TFTestIF::InitcbDIOType(bool bAlarm)
void TI_InitcbDIOType(bool bAlarm)                            //Steven 20101008 : 連按三下會不見
{
    g_dioListLost = false;
    ComboClear(cbDIOType());                                                    // golden :72 cbDIOType->Clear();
    WIN32_FIND_DATA filedata;                                                   // Structure for file data
    HANDLE filehandle;                                                          // Handle for searching
    AnsiString szFileName;
    filehandle=FindFirstFile((DIOCFGPath + "*.ini").c_str(), &filedata);        //Steven 20100706 Start: 改用讀取資料夾的方式

    if(filehandle!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                strcmp(filedata.cFileName, ".")==0 ||                           /* 不處理隱藏檔及 . 跟 .. */
                strcmp(filedata.cFileName, "..")==0)
                continue;

            if(ExtractFileExt(filedata.cFileName).LowerCase()==".ini")          // 若找到的檔案的副檔名是 .ini
            {
                szFileName=ChangeFileExt(ExtractFileName(filedata.cFileName), "");  // 取出檔名，其實就是把副檔名設成""
                cbDIOType()->Items->Add(szFileName);                            // 將檔名加到 cbDIOType
            }
        } while(FindNextFile(filehandle, &filedata));
    }
    else
    {
        if(bAlarm==false)
        {
            // golden :97-98 Application->MessageBox("DIO data has been lossed, please check!!", "DIO Data loss", MB_OK|MB_TOPMOST);
            //               Application->Terminate();
            // 網頁版（Steven 20260925）：訊息回頁面、不結束程式，這次開頁之後的存檔一律拒絕（g_dioListLost）。
            filerw::ELMessage("DIO data has been lossed, please check!!", "DIO Data loss");
            g_dioListLost = true;
        }
        else
        {
            filerw::ELMessage("DIO data has been lossed, please check!!", "DIO資料遺失，請檢查！！");
        }
        //AI(W906-S12C-TTLCfg) 20260925: golden 的分支看起來相反 —— TFTestIF::FormShow :118 傳
        //  bAlarm=(iTestType==TTL_MODE)，註解「修正不是TTL模式會Alarm」，但 bAlarm==false（非 TTL）走的是
        //  MessageBox＋Terminate，TTL 反而只提示。照翻，不修。
    }
    FindClose(filehandle);
}

// golden cTesterIF.cpp:1089-1105  TFTestIF::DoIniDataToForm 的 DIO 段（Steven 20101008）
void TI_DoIniDataToForm_DIO()
{
    if(TestIF_File.sDioName!="")
    {
        for(int i=0; i<cbDIOType()->Items->Count; i++)
        {
            if(TestIF_File.sDioName==cbDIOType()->Items->Strings[i])
            {
                ComboSetItemIndex(cbDIOType(), i);                              // golden :1095 cbDIOType->ItemIndex=i;
                break;
            }
        }
    }
    else
    {
        if(TestIF_File.iDioMode<0)  //Steven 20110105
            TestIF_File.iDioMode=0;
        ComboSetItemIndex(cbDIOType(), TestIF_File.iDioMode);                   // golden :1104 cbDIOType->ItemIndex=TestIF_File.iDioMode;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// golden TFTestIF::ReadTestIFFile（cTesterIF.cpp:563）的 [DIO] 段 —— 只讀（Steven 20260925：「另外補一支只讀 [DIO] 段的讀檔器」）。
//   填 TestIF_File.iDioMode（:707-710）與 TestIF_File.sDioName（:818-854）。ReadTestIFFile 其餘部分仍是 GATE F-5。
//   ⚠ 不照 golden 回寫：:847 在 [DIO] TypeName 不存在時 WriteIniData(szDir,"DIO","TypeName",…) 補鍵 —— 這裡只在記憶體算出
//     同一個值，不寫檔，printf＋ELTodo 回報「golden 會寫」。
//   TestIF_File.iTestType 不在 [DIO]：它在 [Mode] "Tester Type"（:573），且 :575-579 超出 rgInterfaceType 項數時回寫 1
//     （GPIB_MODE）—— 不在本函式範圍，仍依賴 F-5（移植樹預設 0＝TTL_MODE）。
//   開頁（Open）與開機（wb_serve，golden main.cpp:9327 的位置）都呼叫。
// ---------------------------------------------------------------------------
void FileRW_TTLCfg_ReadDIOSection()
{
    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";//, szDir2;
    szDir=DataPath+S;
    szDir+="\\Tester.Data";                                                     // golden :566-570

    TestIF_File.iDioMode                 =ReadIniData(szDir, "DIO", "Type",             0);   // golden :707

    if(TestIF_File.iDioMode<0)                                                  // golden :709-710
        TestIF_File.iDioMode=0;

    //Steven 20101008 : Start
    if(CheckIniData(szDir, "DIO", "TypeName")==false)                           // golden :819
    {
        WIN32_FIND_DATA filedata1;                                              // Structure for file data
        HANDLE filehandle1;                                                     // Handle for searching
        AnsiString szFileName;
        filehandle1=FindFirstFile((DIOCFGPath + "*.ini").c_str(), &filedata1);
        TStringList *MyList=new TStringList();
        if(filehandle1!=INVALID_HANDLE_VALUE)
        {
            do
            {
                /* 不處理隱藏檔及 . 跟 .. */
                if((filedata1.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                    strcmp(filedata1.cFileName, ".")==0 ||
                    strcmp(filedata1.cFileName, "..")==0)
                    continue;

                if(ExtractFileExt(filedata1.cFileName).LowerCase()==".ini")     // 若找到的檔案的副檔名是 .ini
                {
                    szFileName=ChangeFileExt(ExtractFileName(filedata1.cFileName), "");  // 取出檔名，其實就是把副檔名設成""
                    MyList->Add(szFileName);
                }
            } while(FindNextFile(filehandle1, &filedata1));
            FindClose(filehandle1);
        }

        // golden :846 TestIF_File.sDioName=MyList->Strings[TestIF_File.iDioMode];
        //AI(W906-S12C-TTLCfg) 20260925: golden 沒檢查範圍 —— iDioMode 超出清單（或 DIOCFGPath 沒有 *.ini）時 BCB
        //  TStringList 丟 EStringListError（ReadTestIFFile 中斷）。這裡改成不取值、sDioName 維持原值並回報，不讓伺服器崩。
        if(TestIF_File.iDioMode<MyList->Count)
        {
            TestIF_File.sDioName=MyList->Strings[TestIF_File.iDioMode];
        }
        else
        {
            AnsiString e;
            e.sprintf("golden cTesterIF.cpp:846 MyList->Strings[%d] out of range (%d DIO files in DIOCFGPath) -- golden throws "
                      "EStringListError here; TestIF_File.sDioName left as \"%s\"", TestIF_File.iDioMode, MyList->Count,
                      TestIF_File.sDioName);
            std::printf("FileRW TTLCfg: %s\n", e.c_str());
            filerw::ELTodo(e.c_str());
        }
        // golden :847 WriteIniData(szDir, "DIO", "TypeName", TestIF_File.sDioName); —— 只讀：不回寫（Steven 20260925）
        {
            AnsiString t;
            t.sprintf("read-only [DIO] reader: %s has no [DIO] TypeName -- golden cTesterIF.cpp:847 would WRITE "
                      "TypeName=%s into it; NOT written (sDioName computed in memory only)", szDir, TestIF_File.sDioName);
            std::printf("FileRW TTLCfg: %s\n", t.c_str());
            filerw::ELTodo(t.c_str());
        }
        MyList->Clear();                                                        //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete MyList;
    }
    else
    {
        TestIF_File.sDioName             =ReadIniData(szDir, "DIO", "TypeName", AnsiString(""));   // golden :853
    }
    //Steven 20101008 : End
}

namespace {

// 開頁（editlist.get）：順序見檔頭
void Open()
{
    FileRW_TTLCfg_ReadDIOSection();             // golden main.cpp:9327 ReadTestIFFile 的 [DIO] 段（只讀，見上）
    TI_InitcbDIOType(false);                    // golden cTesterIF.cpp:43（TFTestIF 建構子）
    TI_DoIniDataToForm_DIO();                   // golden main.cpp:9327 ReadTestIFFile → cTesterIF.cpp:994 DoIniDataToForm
    {
        AnsiString t;
        t.sprintf("DIO file from recipe Tester.Data [DIO] (read-only reader; rest of ReadTestIFFile is GATE F-5): "
                  "sDioName=\"%s\" iDioMode=%d -> cbDIOType->Text=\"%s\" (ItemIndex %d of %d in DIOCFGPath); "
                  "TestIF_File.iTestType ([Mode] Tester Type) is still not read (port default 0 = TTL_MODE)",
                  TestIF_File.sDioName, TestIF_File.iDioMode, cbDIOType()->Text, cbDIOType()->ItemIndex,
                  cbDIOType()->Items->Count);
        filerw::ELTodo(t.c_str());
    }
    DI_FormShow();                              // golden DIOInterFaceCFG.cpp:24
    DI_spbLoadClick();                          // golden DIOInterFaceCFG.cpp:238（對話框＝GetDIOFileName）
}

// 存檔（editlist.save）
void SaveFlow()
{
    if (g_dioListLost) {
        // golden 走不到這裡（InitcbDIOType 已 Terminate）；網頁版拒絕，不寫 DIOCFGPath\.ini 這種空檔名
        filerw::ELMessage("refused: DIO data has been lossed (no *.ini in DIOCFGPath) -- golden terminates the program at "
                          "TFTestIF::InitcbDIOType; nothing written",
                          "DIO 資料遺失（DIOCFGPath 沒有 *.ini）：golden 會結束程式，網頁版拒絕存檔，未寫檔");
        return;
    }
    DI_spbSaveClick();                          // golden DIOInterFaceCFG.cpp:191
    if (filerw::ELMarked("DIOFileWritten")) {
        // golden main.cpp:28667-28669 sbDioSetClick：ShowModal 回來（關表單）後
        AnsiString S="";
        S=DI_GetDIOFileName();                                                  //Steven 20180626 (wei) : TTL設定存到工作檔裡面
        DI_LoadData(S);
        // golden main.cpp:28670-28672 #ifndef SOFT_SIMULTE InitDIOStstus(true)（main.cpp:25063：TTLCfg → Prod.DIOCfg，
        // 再依設定打 TTL_StartData／TTL_Dut／SwClear2／SwClear6 輸出）—— 移植樹沒有 InitDIOStstus，且是硬體輸出
        // Steven 20260925：維持 ELTodo，列為 Jimmy 的待辦
        filerw::ELTodo("golden main.cpp:28670-28672 InitDIOStstus(true) (TTLCfg -> Prod.DIOCfg + TTL start/DUT/clear outputs) "
                       "not ported -- Prod.DIOCfg and the TTL outputs are NOT refreshed after save");
    }
}

filerw::PageDesc g_page = {
    "TTLCfg", "TfDIOFrom", "Config.DIOInterFaceCFG.html",
    nullptr, nullptr, 0,
    kDI_SaveReads, (int)(sizeof(kDI_SaveReads) / sizeof(kDI_SaveReads[0])),
    &Open, &SaveFlow, "DIOFileWritten",
    &DI_FormClose,   // 沒寫檔時還原替身：golden FormClose :264（fShow=false＋DoIniDataToForm，JerryYang 20250411「避免誤存檔」）
    &Booted,
};
filerw::PageRegistrar g_reg(&g_page);
}  // namespace

// golden TfDIOFrom 建構（HT9045.cpp:196 CreateForm）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身。
// golden TFTestIF（HT9045.cpp:189，更早）的 cbDIOType 替身也在這裡建；它的內容（InitcbDIOType）每次開頁重跑。
// 開機讀檔：golden main.cpp:9415-9416 GetDIOFileName＋LoadData —— 本檔不做（移植樹開機路徑沒有，整合者決定）。
void FileRW_TTLCfg_Boot()
{
    if (g_booted) return;
    DI_DfmItems();
    DI_DfmState();
    DI_TfDIOFrom();
    DI_CreateSaveProxies();
    DI_CreateContainerProxies();
    cbDIOType();
    std::printf("FileRW TTLCfg: TfDIOFrom proxies ready (%d save reads) -- golden DIOInterFaceCFG.cpp\n",
                (int)(sizeof(kDI_SaveReads) / sizeof(kDI_SaveReads[0])));
    g_booted = true;
}

// golden cTesterIF.cpp:1299  TFTestIF::cbDIOTypeChange —— TesterIF 頁換 DIO 型態時（將來 TFTestIF 的 C 形狀呼叫；
// 呼叫前 cbDIOType 替身已是新選的值）。本檔目前沒有呼叫端。
void FileRW_TTLCfg_TFTestIF_cbDIOTypeChange()
{
    AnsiString S;
    S=DI_GetDIOFileName();                                                      //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    DI_LoadData(S);
    ;   // golden :1304 ShowTTLState()：TFTestIF 的 lstTTL 摘要清單（畫面；移植樹 GATE F-11）
}
