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
//  其餘欄位與 iCateParity 要等關表單的 LoadData 才進執行期）。接著 InitDIOStstus(true)（AI(W906-DIO) 20260925：golden 906 main.cpp:27670-27672，本體 cDIOStatus.cpp）。
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
//
//  AI(W906-FRW-S101) 20260926: RULINGS_20260926 S101 —— Delete 鈕（golden spbDeleteClick :249-257）＝ 檔尾 FileRW_TTLCfg_DeleteOp。
//    golden 按 Delete → 開檔對話框（OpenDialog1：Filter *.ini、InitialDir＝DIOCFGPath）→ 選了檔按 Open 就 DeleteFile，沒有確認框、
//    沒有權限檢查（表單本身由主畫面 sbDioSetClick 的 fSecurity->Insufficient(31) 擋，main.cpp:28663）。網頁版：op=list 列檔（對話框）、
//    op=delete 刪選的檔；說明與守衛在該函式上方。WS 分派片段見交件報告（tools/wb_serve.cpp 共用檔）。
// ===========================================================================
#include "FileRW/TTLCfg.gen.inc"

#include <cstdio>
#include "forms/fMain.h"   // AI(W906-DIO) 20260925: fMain->InitDIOStstus（SaveFlow；golden sbDioSetClick 是 TfMain 成員）。佔用原本的空行
#include "FileRW/_EditPage.h"
#include "Public/cJSON.h"            // AI(W906-FRW-S101) 20260926: FileRW_TTLCfg_DeleteOp（value 解析）
#include "WebBridge/JsonWriter.h"    // AI(W906-FRW-S101) 20260926: FileRW_TTLCfg_DeleteOp（回應）
#include "forms/fSecurity.h"         // AI(W906-FRW-S101) 20260926: fSecurity->Insufficient(31)（golden sbDioSetClick main.cpp:28663）
#include <string>
#include <vector>

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }
int  g_w906DeleteOpenLevel = -1;   // AI(W906-FRW-S101) 20260926: 開頁（Open＝golden FormShow）時的 AccessLevel；-1＝這次開機還沒開過 DIO 頁

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
    g_w906DeleteOpenLevel = AccessLevel;        // AI(W906-FRW-S101) 20260926: Delete 鈕只在表單開著時按得到（FileRW_TTLCfg_DeleteOp 的守衛）
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
        //AI(W906-DIO) 20260925: 接上 golden 906 main.cpp:27670-27672（sbDioSetClick 關表單後）的 InitDIOStstus(true)，取代原本的 ELTodo「not ported」（Steven 0925 08:17 交辦）。
        //  本體 cDIOStatus.cpp（golden main.cpp:24357-24375：TTLCfg → Prod.DIOCfg，再打 TTL_StartData／TTL_Dut／SwClear2／SwClear6）；呼叫點與本體 golden 都是 #ifndef SOFT_SIMULTE，照翻。
        #ifndef SOFT_SIMULTE
        fMain->InitDIOStstus(true);
        #endif
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

// AI(W906-RCHG) 20260925（Steven 團隊）：golden TfMain::DoReadLastData main.cpp:9416-9417（V912）
//   S=fDIOFrom->GetDIOFileName(); fDIOFrom->LoadData(S);   //Steven 20180626 (wei) : TTL設定存到工作檔裡面
// 開機與換配方共用的讀檔鏈（tools/wb_serve.cpp W906_DoReadLastData）在 FTestIF->DoIniDataToForm（:9388，
// FileRW_TesterIF_DoIniDataToForm 會選好 TFTestIF cbDIOType 替身）之後呼叫。
// ⚠ 會寫檔：DI_GetDIOFileName 在 IniConfig.bI16TTLSaveInSetupFile 而配方夾沒有 <DIO型態>.ini 時，從 DIOCFGPath 複製一份進去
//   （golden DIOInterFaceCFG.cpp:56-63，照翻）。DIO 檔遺失時 DI_LoadData 走 filerw::ELMessage（golden ShowMyMessage）。
void FileRW_TTLCfg_DoReadLastDataLoad()
{
    FileRW_TTLCfg_Boot();
    AnsiString S="";
    S=DI_GetDIOFileName();                                                      //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    DI_LoadData(S);
}

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S101) 20260926: golden TfDIOFrom::spbDeleteClick（V912 DIOInterFaceCFG.cpp:249-257）的網頁入口。
//    golden 本體（DI_spbDeleteClick，TTLCfg.gen.inc，逐行）：OpenDialog1->Title="Select file to delete" → if(OpenDialog1->Execute())
//      DeleteFile(OpenDialog1->FileName) → spbDelete->Down=false。沒有確認框（對話框按 Open 就是確認）、沒有權限檢查、沒有 SystemStart
//      檢查、刪完不重讀（刪掉的若是目前使用中的 DIO 檔，golden 下次關 DIO 表單／換配方 LoadData 時才發現：TTL 模式會 SystemStart=false
//      ＋訊息，LoadData :78-82）—— 照翻。
//    對話框（golden OpenDialog1，DFM :789-795：Filter '*.ini|*.ini'、InitialDir＝DIOCFGPath，FormShow :26 再設一次）換成兩步：
//      value={"op":"list"}               → 列 DIOCFGPath 底下的 *.ini（不含隱藏檔，同 TI_InitcbDIOType 的規則），不動任何東西。
//      value={"op":"delete","file":"<檔名>.ini"} → 選了檔按 Open：跑 golden 本體（DeleteFile）。
//    守衛（不信任前端，每次重查；前兩條是移植樹加的，golden 靠「表單開著、使用者在畫面前」隱含成立）：
//      P1 開過 DIO 頁（editlist.get tag=TTLCfg ＝ golden FormShow），而且是同一個 AccessLevel → 否則 page-not-open。
//      P2 fSecurity->Insufficient(31,false)（golden 開這個表單的主畫面鈕 sbDioSetClick :28663 的權限）→ 否則 not-authorized。
//         ⚠ 偏離：golden 只在開表單時檢查一次；這裡按 Delete 時再查（等級沒變時結果相同）。列給 Steven（交件報告 S101）。
//      P3 spbDelete 的替身可按（filerw::ELEditable，自己與 GroupBox4 都 Enabled 且 Visible）。
//      P4 檔名：只收單純檔名（不含 \ / : 與 ".."）、副檔名 .ini、而且在 P0 的清單裡 ——
//         ⚠ 偏離：golden 的對話框可以換資料夾、刪任何一個 .ini（甚至打任意檔名）；網頁版只開放 DIOCFGPath 底下現有的 *.ini。
//    ⚠ 會刪的真實檔：D:\HT9045\iniData\DioCfg\<檔名>.ini（DIOCFGPath，common.cpp:227）—— 是全機台共用的 DIO 型態清單
//      （TFTestIF cbDIOType 的選項就是這個資料夾的 *.ini）。IniConfig.bI16TTLSaveInSetupFile 時配方夾裡的 <DIO>.ini 不在清單內
//      （golden 對話框的預設資料夾也不是那裡）。
//  *ok：list 成功、或 delete 跑完 golden 本體 ＝ true；守衛擋下／參數錯 ＝ false。呼叫端持 FormLock、在主迴圈（同 editlist.*）。
// ---------------------------------------------------------------------------
namespace {
void W906_ListDioIni(std::vector<std::string>* out)
{
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile((DIOCFGPath + "*.ini").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
            (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0)                 // 同 TI_InitcbDIOType（golden cTesterIF.cpp:84-88）
            continue;
        if (ExtractFileExt(fd.cFileName).LowerCase() != ".ini") continue;       // *.ini 的萬用字元也會配到 8.3 短名（例 .ini~）
        out->push_back(fd.cFileName);
    } while (FindNextFile(h, &fd));
    FindClose(h);
}

std::string W906_DelRefuse(const char* op, const char* guard, const char* goldenLine, const std::string& detail)
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(false);
    w.Key("op").String(op);
    w.Key("guard").String(guard);
    w.Key("goldenLine").String(goldenLine);
    w.Key("detail").String(detail);
    w.EndObject();
    std::printf("ttlcfg.op %s -> refused (%s)\n", op, guard);
    return w.Str();
}

bool W906_SameName(const std::string& a, const std::string& b)                 // Windows 檔名不分大小寫
{
    return AnsiString(a.c_str()).LowerCase() == AnsiString(b.c_str()).LowerCase();
}
}  // namespace

std::string FileRW_TTLCfg_DeleteOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    std::string op, file;
    {
        cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
        if (root == 0 || !cJSON_IsObject(root)) {
            if (root) cJSON_Delete(root);
            return W906_DelRefuse("?", "bad-payload", "", "value 要是 JSON 物件：{\"op\":\"list\"} 或 {\"op\":\"delete\",\"file\":\"<檔名>.ini\"}");
        }
        const cJSON* jo = cJSON_GetObjectItemCaseSensitive(root, "op");
        const cJSON* jf = cJSON_GetObjectItemCaseSensitive(root, "file");
        if (jo && cJSON_IsString(jo) && jo->valuestring) op = jo->valuestring;
        if (jf && cJSON_IsString(jf) && jf->valuestring) file = jf->valuestring;
        cJSON_Delete(root);
    }
    if (op != "list" && op != "delete")
        return W906_DelRefuse(op.c_str(), "bad-op", "", "op 只收 list／delete");
    if (!g_booted)
        return W906_DelRefuse(op.c_str(), "not-booted", "", "TTLCfg 替身還沒建（FileRW_TTLCfg_Boot）");
    // P1
    if (g_w906DeleteOpenLevel < 0 || g_w906DeleteOpenLevel != AccessLevel)
        return W906_DelRefuse(op.c_str(), "page-not-open", "V912 DIOInterFaceCFG.cpp:24 FormShow",
                              "reload page: open the DIO page (editlist.get TTLCfg = golden FormShow) with the current access level first");
    // P2
    if (fSecurity->Insufficient(31, false) == false)
        return W906_DelRefuse(op.c_str(), "not-authorized", "V912 main.cpp:28663 sbDioSetClick",
                              "fSecurity->Insufficient(31)（DIO Set 權限，system\\levelset.dat）不足 —— golden 開不了這個表單");
    // P3
    if (!filerw::ELEditable("TfDIOFrom", "spbDelete"))
        return W906_DelRefuse(op.c_str(), "button-disabled", "V912 DIOInterFaceCFG.dfm:401 spbDelete",
                              "spbDelete（或它的容器 GroupBox4）目前不能按");

    std::vector<std::string> files;
    W906_ListDioIni(&files);
    const std::string inUse = std::string(cbDIOType()->Text.c_str()) + ".ini";  // 目前 TFTestIF 選的 DIO 型態（golden GetDIOFileName :55/:58/:67 用的名字）

    if (op == "list") {
        webbridge::JsonWriter w;
        w.BeginObject();
        w.Key("executed").Bool(true);
        w.Key("op").String("list");
        w.Key("title").String("Select file to delete");                        // golden :251
        w.Key("dir").String(DIOCFGPath.c_str());                                // golden FormShow :26 InitialDir
        w.Key("filter").String("*.ini");                                        // golden DFM :790
        w.Key("inUse").String(inUse);
        w.Key("files").BeginArray();
        for (std::size_t i = 0; i < files.size(); ++i) w.String(files[i]);
        w.EndArray();
        w.EndObject();
        if (ok) *ok = true;
        return w.Str();
    }

    // P4
    if (file.empty() || file.find_first_of("\\/:") != std::string::npos || file.find("..") != std::string::npos)
        return W906_DelRefuse("delete", "bad-file", "V912 DIOInterFaceCFG.cpp:252-254",
                              "file 要是 DIOCFGPath 底下的單純檔名（不含路徑）");
    if (ExtractFileExt(AnsiString(file.c_str())).LowerCase() != ".ini")
        return W906_DelRefuse("delete", "bad-file", "V912 DIOInterFaceCFG.dfm:790 Filter *.ini", "只收 .ini（golden 對話框的 Filter）");
    std::string picked;
    for (std::size_t i = 0; i < files.size(); ++i)
        if (W906_SameName(files[i], file)) { picked = files[i]; break; }
    if (picked.empty())
        return W906_DelRefuse("delete", "not-in-list", "V912 DIOInterFaceCFG.cpp:252",
                              "DIOCFGPath 底下沒有這個 .ini（網頁版只刪對話框清單裡的檔；golden 的對話框可以換資料夾，這裡不開放）");

    const AnsiString full = DIOCFGPath + AnsiString(picked.c_str());
    W906_DeleteDialogExecuted = true;                                           // golden OpenDialog1->Execute() 回 true
    W906_DeleteDialogFileName = full;                                           // golden OpenDialog1->FileName
    DI_spbDeleteClick();                                                        // golden DIOInterFaceCFG.cpp:249-257
    W906_DeleteDialogExecuted = false;                                          // 回到安全預設：其他呼叫者走到這支都當「對話框取消」
    W906_DeleteDialogFileName = "";
    const bool gone = !FileExists(full);
    std::printf("ttlcfg.op delete %s -> %s\n", full.c_str(), gone ? "deleted" : "STILL THERE (DeleteFile failed)");

    std::vector<std::string> after;
    W906_ListDioIni(&after);
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("executed").Bool(gone);
    w.Key("op").String("delete");
    w.Key("file").String(full.c_str());
    w.Key("wasInUse").Bool(W906_SameName(picked, inUse));                      // 刪的是目前使用中的 DIO 型態檔（golden 不擋、不提示）
    w.Key("goldenLine").String("V912 DIOInterFaceCFG.cpp:249-257 spbDeleteClick");
    w.Key("deleted").BeginArray();
    if (gone) w.String(full.c_str());
    w.EndArray();
    w.Key("filesAfter").BeginArray();
    for (std::size_t i = 0; i < after.size(); ++i) w.String(after[i]);
    w.EndArray();
    w.EndObject();
    if (ok) *ok = true;                                                         // golden 本體跑完（DeleteFile 失敗 golden 也不報，executed 照實）
    return w.Str();
}
