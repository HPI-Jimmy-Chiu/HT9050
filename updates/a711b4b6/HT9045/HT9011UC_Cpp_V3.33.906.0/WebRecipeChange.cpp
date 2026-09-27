// =============================================================================
//  WebRecipeChange.cpp -- 主畫面「變更工作檔」（換配方）：golden V912 翻譯 ＋ WS recipe.change 包裝
//
//  AI(W906-RCHG) 20260925（Steven 團隊）：只在 wb_serve 連結（CMakeLists.txt wb_serve 來源清單）。
//    golden = HT9011UC_Code_V3.33.912.0_20260908_Jimmy（行號一律是 V912）：
//      TfMain::cbSetupFileNameDropDown  main.cpp:31154-31173   → op "list"
//      TfMain::LookForFile              main.cpp:9443-9514     → W906_RC_LookForFile()
//      TfMain::cbSetupFileNameChange    main.cpp:25351-25652   → W906_RC_cbSetupFileNameChange()
//      TfMain::ChangeSetUpFile          main.cpp:25655-25816   → W906_RC_ChangeSetUpFile()
//      TfMain::DoReadLastData           main.cpp:9264-9441     → tools/wb_serve.cpp W906_DoReadLastData(false,…)
//                                                                 （與開機同一份函式，bBoot=false）
//      TFormHS::CheckSetupFileData      HS_Function.cpp:3486-3517 → W906_RC_CheckSetupFileData()
//      TfMain::Timer3Timer 的 bNoNeedLoadSteupFile 還原  main.cpp:25945-25949（本檔 op 結尾做）
//
//  守衛（golden 的順序）
//    1. cbSetupFileName->Enabled：golden 由計時器 ShowRunLabel（ckernel.cpp:935 起）呼叫 EnabledSetupFile
//       （main.cpp:29354-29447）設定 —— 運轉中（ckernel.cpp:1084-1086 SystemStart）、機台內有 IC、
//       權限（bSetupFileNameControlByLevel 時 LevelSet.AccessLevel[41]）、RMS／FTP／ERMS 都會把它關掉。
//       移植樹 ShowRunLabel（ckernel.cpp:1940）／EnabledSetupFile（cMainStatus.cpp:203）已照翻，
//       MainProc 每 6 拍跑一次；這裡讀同一個 Enabled，並另外直接看 SystemStart（兩拍之間的空窗）。
//    2. cbSetupFileNameDropDown：fSecurity->Insufficient(9)==false 就 return（main.cpp:31156-31157）。
//       golden 的 Insufficient(9) 預設 bAlarm=true 會跳 WAR1676；網頁版傳 false，拒絕理由回 ack，由頁面顯示。
//    3. 只能選清單裡的名字：golden cbSetupFileNameKeyPress/KeyDown 都把 Key 清成 0（main.cpp:29477-29485），
//       也就是只能從 LookForFile 列出的項目選。
//    4. ChangeSetUpFile 自己的三道：CheckCanChangeRealDummy（:25657）、空字串（:25663）、
//       CheckSetupFileData 檔案少於 8 個（:25669）。
//    5. 確認框：golden 一般路徑沒有（只有 CC_KYEC_LEE 的 Run Start Mode 改變 YES/NO，:25440-25444，客戶專屬先跳過）。
//
//  機台動作（沒有現成通道的不接）：執行到那一行、而且 golden 條件成立時，記進 ack.notWired（含 golden 行號與原因），
//  不假裝做了。客戶專屬條件（CUSTOMER_CODE==CC_xxx）照 Steven 指示先跳過，條件成立時記進 ack.customerSkipped。
// =============================================================================
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "forms/fMain.h"            // fMain->cbSetupFileName／iHasChangeFile／CheckCanChangeRealDummy／各 UI 門面
#include "forms/fSecurity.h"        // fSecurity->Insufficient
#include "forms/fOffSet.h"          // fOffSet->GetOffsetPath（LookForFile）
#include "forms/fLotInfo.h"         // fLotInfo->cbRunMode／SetFirstTrayCheckOnUnloader
#include "forms/fTestCategory.h"    // fTestCategory->AdjFormData
#include "forms/fContactCT.h"       // fContactCT->ShowFormComp
#include "forms/fShowBinSelect.h"   // fShowBinSelect->ShowBinSel／InitShowBinDigital
#include "forms/fTrayMapping.h"     // fTrayMappingForm->ChangeTraySetupFile
#include "forms/fShuttleMove.h"     // fShuttleMove->ReadData（golden :25814；Steven 團隊 20260926）
#include "common.h"                 // DataPath／OffsetPath／AuthPath／GetLastOpenFN／WriteLastDataFN／MyForceDirectories
#include "cmydef.h"                 // SystemStart／iAdaptiveACInterval／bFTPDownlodFinish／… golden 全域
#include "cprod.h"                  // TestIF_File／Temperature／TestMode／LastSet／RunInfo／SaveTestMode／ReadConfigByRecipe
#include "cinitial.h"               // SetWorkParameter
#include "cUnitConvert.h"           // DoStructUnitConvert
#include "bthermo.h"                // ClearAllHotBuffer
#include "csystem.h"                // HasICUnderMachine
#include "aHotPlateSubstrate.h"     // ResetShuttleWhichKit／SetRunStartMode
#include "ainarm_SearchPlacePlate.h"// ResetHotPlateSearchParameter
#include "Motor/mymotor.h"          // MOT[MMTrayY]
#include "ATC/ATCInterface.h"       // ATCInterfaceForm->SendCommToATC7（MainTempMode.cpp 同一條活的通道）
#include "Interface/TesterTCP.h"    // TesterTCP_CopyRecipeToTester（golden fTesterTCP->CopyRecipeToTester）
#include "SECSGEM/SecsEventReport.h"// EventReport（移植樹是 sim 記錄器，SecsEventReport.cpp:15）
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT.SwitchSetupFile
#include "canary_support.h"         // W906_ShowMyMessage_Hook（訊息收進 ack）

// 與 WebBuilder.cpp 同一個理由在這裡直接宣告：cMyDB.h 與 canary_support.h 會給同一個函式重複的預設參數。
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // acatchtray_shims.cpp:152（移植樹是空 shim）
namespace ht9045 { namespace formjson { void FormLock(); void FormUnlock(); } }   // JsonBridge/FormJson.cpp
namespace ht9045 { void SetWebBinSelLoaded(bool loaded); void SetWebTempLoaded(bool loaded); }   // WebBridgeTags.h:237/:244
namespace filerw { void SessionBegin(const std::string& answersJson); std::string SessionJson(); }   // FileRW/_EditList.h:144-145

// tools/wb_serve.cpp：golden TfMain::DoReadLastData 的移植（開機與換配方共用同一份）
void W906_DoReadLastData(bool bBoot, bool& binSelLoaded, bool& tempLoaded);
bool W906_RecipeChainsReady();
void W906_RC_LaserReadLaserFile();   // WebReadChainCalls.cpp（LaserSensor.h 與 ATCInterface.h 各定義一個 TTimer，不能同一個 TU）

namespace {

// golden 全域 bNoNeedLoadSteupFile（main.cpp:25384 設、Timer3Timer :25945-25949 還原 cbSetupFileName->Text）。
//   移植樹沒有 Timer3Timer，這個旗標只在本檔用：op 結尾照 Timer3 的那三行做。
bool bNoNeedLoadSteupFile = false;

struct FormLockGuard {
    FormLockGuard()  { ht9045::formjson::FormLock(); }
    ~FormLockGuard() { ht9045::formjson::FormUnlock(); }
};

// ---- ShowMyMessage 收進 ack（同 WebBuilder.cpp:120-131）----
std::vector<std::pair<std::string, std::string> >* g_msgs = 0;
void (*g_prevHook)(const char*, const char*) = 0;
void CaptureHook(const char* s1, const char* s2)
{
    if (g_msgs) g_msgs->push_back(std::make_pair(std::string(s1 ? s1 : ""), std::string(s2 ? s2 : "")));
    if (g_prevHook) g_prevHook(s1, s2);
}
struct MsgCapture {
    std::vector<std::pair<std::string, std::string> > msgs;
    MsgCapture()  { g_prevHook = W906_ShowMyMessage_Hook; g_msgs = &msgs; W906_ShowMyMessage_Hook = CaptureHook; }
    ~MsgCapture() { W906_ShowMyMessage_Hook = g_prevHook; g_msgs = 0; g_prevHook = 0; }
};

// ---- 檔名：磁碟上是 ANSI（cp950），JSON 是 UTF-8 ----
std::string U8(const AnsiString& a)
{
    const std::string s = a.c_str();
    if (webbridge::IsValidUtf8(s)) return s;
    const int wn = MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return std::string();
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string out((size_t)(n > 0 ? n : 0), '\0');
    if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), wn, &out[0], n, NULL, NULL);
    return out;
}
AnsiString FromU8(const std::string& s)
{
    const int wn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), (int)s.size(), NULL, 0);
    if (wn <= 0) return AnsiString(s.c_str());
    std::wstring w((size_t)wn, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], wn);
    const int n = WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, NULL, 0, NULL, NULL);
    std::string out((size_t)(n > 0 ? n : 0), '\0');
    if (n > 0) WideCharToMultiByte(CP_ACP, 0, w.c_str(), wn, &out[0], n, NULL, NULL);
    return AnsiString(out.c_str());
}

// ---- 報告：沒接的機台動作／跳過的客戶專屬分支（golden 條件成立才記）----
struct Note { std::string golden, what, why; };
struct Rep {
    std::vector<Note> notWired, customer;
    int readChainRuns;
    Rep() : readChainRuns(0) {}
    void gap(const char* g, const char* w, const char* y)  { Note n; n.golden = g; n.what = w; n.why = y; notWired.push_back(n); }
    void cust(const char* g, const char* w)                { Note n; n.golden = g; n.what = w; n.why = "customer-specific branch skipped (Steven 20260925)"; customer.push_back(n); }
};
Rep* g_rep = 0;
void Gap(const char* g, const char* w, const char* y) { if (g_rep) g_rep->gap(g, w, y); }
void Cust(const char* g, const char* w)                 { if (g_rep) g_rep->cust(g, w); }

// golden TfMain::DoReadLastData() —— 移植樹的本體在 tools/wb_serve.cpp（開機也叫它），這裡只是 golden 的呼叫點
void DoReadLastData()
{
    bool binSelLoaded = false, tempLoaded = false;
    W906_DoReadLastData(false, binSelLoaded, tempLoaded);
    ht9045::SetWebBinSelLoaded(binSelLoaded);
    ht9045::SetWebTempLoaded(tempLoaded);
    if (g_rep) ++g_rep->readChainRuns;
}

} // namespace

// =============================================================================
//  golden TfMain::LookForFile()  main.cpp:9443-9514
// =============================================================================
void W906_RC_LookForFile()
{
    AnsiString sOldDir="", sNewDir="", str3;
    static bool bHasOffsetData=true;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         // :9447-9454 客戶專屬（bShowFirst 只有這一段用，移植樹沒有）
        Cust("main.cpp:9447-9454", "CC_ASE_KaohSiung: ShowMyMessage(\"change level!!\") when AccessLevel<=LevelSet.AccessLevel[9]");
    fMain->cbSetupFileName->Clear();                                            // :9455

    if(!DirectoryExists(OffsetPath))                                            // :9457 檢查Offset資料夾是否存在，不存在就建立一個新的
    {
        bHasOffsetData=false;                                                   //Steven 20100927 Start: 把Offset換位置
        MyForceDirectories(OffsetPath);
    }

    if(!DirectoryExists(DataPath))                                              // :9463
    {
        ShowMyMessage("File Path Lost!!");
        return;
    }
    else
    {
        WIN32_FIND_DATAA filedata;                                              // Structure for file data
        HANDLE filehandle;                                                      // Handle for searching
        filehandle=FindFirstFileA((DataPath+"*").c_str(), &filedata);

        if(filehandle!=INVALID_HANDLE_VALUE)
        {
            do
            {
                /* 不處理隱藏檔及 . 跟 .. */
                if((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                    strcmp(filedata.cFileName, ".")==0 ||
                    strcmp(filedata.cFileName, "..")==0)
                {
                    continue;
                }
                else if(filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)   //如果該檔案為資料夾
                {
                    sNewDir=fOffSet->GetOffsetPath(filedata.cFileName);
                    fMain->cbSetupFileName->Items->Add(ExtractFileName(filedata.cFileName));   // 將檔名加到 cbSetupFileName
                    //Steven 20100927 Start: 把Offset換位置
                    if(bHasOffsetData==false)                                   //Offset資料夾不存在，表示要把Offset移動過來
                    {
                        sOldDir=DataPath+filedata.cFileName;
                        MyForceDirectories(sNewDir, "TfMain::LookForFile");
                        MoveFileA(AnsiString(sOldDir+"\\Position Offset.Data").c_str(), AnsiString(sNewDir+"\\Position Offset.Data").c_str());
                        MoveFileA(AnsiString(sOldDir+"\\Position Offset Hot.Data").c_str(), AnsiString(sNewDir+"\\Position Offset Hot.Data").c_str());
                    }

                    if(FileExists(DataPath+filedata.cFileName+"\\123aaa.ini"))  //幽靈檔案，砍掉
                    {
                        DeleteFileA(AnsiString(DataPath+filedata.cFileName+"\\123aaa.ini").c_str());
                    }
                    //Steven 20100927 End
                }
            } while(FindNextFileA(filehandle, &filedata));
            FindClose(filehandle);
        }
    }
    //Ifor 20160425 工作檔切換需重新設定ATC參數
    bATCTempAdjustmentOffset=false;                                             // :9510
    bSetTempChange=true;                                                        // :9511
    //Steven 20100706 End
    // :9513 bShowFirst=false; —— 只給 :9447 的 CC_ASE_KaohSiung 分支用，客戶專屬先跳過，移植樹沒有這個全域
}

// =============================================================================
//  golden TFormHS::CheckSetupFileData(AnsiString strSetupFile)  HS_Function.cpp:3486-3517
//    移植樹沒有 TFormHS 的這一支（forms/fHS.h:710 只有宣告，GATE Cat E），所以翻在這裡。
// =============================================================================
static bool W906_RC_CheckSetupFileData(AnsiString strSetupFile)                 //Ifor 20160822 add 判斷Setup File 檔案是否遺失 Start
{
    bool bHasError=false;

    int iCount=0;                                                               // golden TStringList *lstFiles（只用 Count）
    AnsiString strSetupFileDir="";
    strSetupFileDir.sprintf("%s%s", DataPath.c_str(), strSetupFile.c_str());

    WIN32_FIND_DATAA sr;                                                        // golden FindFirst(strSetupFileDir+"\\*.*", faAnyFile, sr)
    HANDLE h=FindFirstFileA((strSetupFileDir+"\\*.*").c_str(), &sr);
    if(h!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if(strcmp(sr.cFileName, ".")!=0 && strcmp(sr.cFileName, "..")!=0)
                iCount++;
        }while(FindNextFileA(h, &sr));
        FindClose(h);
    }

    if(iCount<8)
    {
        InitialOK=true;                                                         //Ifor 20160822 讓Message 可以顯示（wb_serve 開機後本來就是 true）
        bHasError=true;
        if(fMain->iHasChangeFile!=9)
        {
            ShowMyMessage("Setup file error, Please check\r\n"+strSetupFileDir+"\r\n"+"Whether the missing files");
        }
        fMain->iHasChangeFile=2;
    }
    return bHasError;
}

// =============================================================================
//  golden TfMain::ChangeSetUpFile(AnsiString FileName)  main.cpp:25655-25816
// =============================================================================
static int W906_RC_ChangeSetUpFile(AnsiString FileName)
{
    if(fMain->CheckCanChangeRealDummy()==false)                                 // :25657
    {
        fMain->iHasChangeFile=2;
        return 1;
    }

    if(FileName=="")                                                            // :25663
    {
        fMain->iHasChangeFile=2;
        return 1;
    }

    if(W906_RC_CheckSetupFileData(FileName))                                    // :25669 FormHS->CheckSetupFileData —— Ifor 20160822 切換工作擋時判斷Setup File 檔案數量是否短少
    {
        fMain->iHasChangeFile=2;
        return 1;
    }

    if(CUSTOMER_CODE==CC_JCET)                                                  // :25675-25685 客戶專屬（JCET 強制開 Socket Sensor）先跳過
    {
        Cust("main.cpp:25675-25685", "CC_JCET: bFirstTimeEnableSocketSensor=true (JCET_FOR_EVAN==1 or C08/D65)");
    }
    else
    {
        bFirstTimeEnableSocketSensor=false;                                     // :25688 一般路徑
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           // :25691-25694
        Cust("main.cpp:25691-25694", "CC_KYEC_XILINX: bNoChangeContactHeight=true");

    static AnsiString OldWorkfile="";                                           // :25696
    AnsiString Str = FileName;
    if(MACHINE_HAS_AUTO_ALIGNMENT_CCD)                                          // :25698 KenHsieh 20210813 : add CCD AUTO ALIGNMENT
    {
        Gap("main.cpp:25698-25720", "Auto Alignment CCD: VerifyNeedDoAlignment(AutoAlignmentTray_ChangeSetupFile, ...) / l*AutoAlignment*Flag reset",
            "CCD auto-alignment -- csystem.cpp:2530 VerifyNeedDoAlignment is a no-op stub; the flag block is part of the same alignment state machine");
    }
    OldWorkfile=Str;                                                            // :25721

    fMain->cbSetupFileName->Text=FileName;                                      // :25722
    WriteLastDataFN(FileName);                                                  // :25723 D:\HT9045\SetUp.inf
    DoReadLastData();                                                           // :25724

    Gap("main.cpp:25726", "fTesterTCP->OSRecipe=\"\"", "TfTesterTCP::OSRecipe member is not in the port (Interface/TesterTCP.cpp has free functions only)");
    NewRecordProcess("MES2158", "Change Set Up File", FileName);                // :25727（移植樹 NewRecordProcess 是空 shim，acatchtray_shims.cpp:152）

    if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS ||                                 // :25729-25734
       CUSTOMER_CODE==CC_XINITECH)
        Cust("main.cpp:25729-25734", "CC_SJ_Semiconductor_OS / CC_XINITECH: fStartCondition->ClearPickerCount() + sbHeadCondition1Save->Click()");

    if(Temperature.iMachineTempMode==0)                                         // :25736
    {
        LastSet.iTemperature=Tempture_Hot;
        NewRecordProcess("MES2151", "Change Hot Mode", " ");
        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();
        }
    }
    else if(Temperature.iMachineTempMode==1)                                    // :25746
    {
        LastSet.iTemperature=Tempture_Ambient;
        NewRecordProcess("MES2153", "Change Ambient Mode", " ");
        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();
        }
    }
    fMain->UpdateMainOperateMode();                                             // :25756（移植樹門面計數 stub，forms/fMain.cpp）
    DoStructUnitConvert();                                                      // :25757
    SetWorkParameter();                                                         // :25758 Steven 20120130 : 存檔後要重新load參數
    ClearAllHotBuffer();                                                        // :25759

    fTestCategory->AdjFormData();                                               // :25761
    fContactCT->ShowFormComp();                                                 // :25762
    fShowBinSelect->ShowBinSel();                                               // :25763
    fShowBinSelect->InitShowBinDigital();                                       // :25764
    fMain->LoadTestModePicture();                                               // :25765（門面 stub）
    // :25766 fShowMessage->sgdSpeedView->Repaint(); —— 純 VCL 重畫，網頁沒有這張表

    W906_RC_LaserReadLaserFile();                                               // :25768 fLaserSensor->ReadLaserFile() Steven 20140228 : 雷射測距功能
    fMain->ShowTestHeadComp(false);                                             // :25769 kevin 20150202 主畫面site 跟著工作檔改變（門面 stub）
    ResetHotPlateSearchParameter();                                             // :25770
    { extern void SetTestRunMode(); SetTestRunMode(); }                         // :25771 JerryYang 20230130 : fix offline沒有切換  //AI(W906-RCHG) 20260926: Jimmy 0dcc9c2c 已翻 SetTestRunMode（RunStartMode.cpp:938），Gap 換成 golden 呼叫——修「換配方來回後 cbTestMode 由 Re-Test 變 Off-Line」
    DoReadLastData();                                                           // :25772 Jimmychiu 20250121 : Avoid changing the recipe if the bin settings remain unchanged.
    MOT[MMTrayY].fHasTray=false;                                                // :25773 jou 2012-06-19 修正HasICUnderMachine誤判

    if(INSTALL_OCR!=eocrUninstal)                                               // :25775-25778
        Gap("main.cpp:25775-25778", "fOCR->edFileName->Text=cbSetupFileName->Text", "TfOCR is not in the port");

    ResetShuttleWhichKit();                                                     // :25780 Steven 20160213 : 解決Input擺放順序問題
    Gap("main.cpp:25781", "myLog.Save_SiteStatusLog()", "TMyLog::Save_SiteStatusLog is translated (handlerlog.cpp:277) but the global myLog has no definition in wb_serve (and handlerlog.cpp needs SaveEventLog(), also undefined) -- site on/off log D:\HT9045_Log\ChangeLog not written");

    if(CUSTOMER_CODE==CC_Greatek)                                               // :25783-25786
        Cust("main.cpp:25783-25786", "CC_Greatek: fRPDefault->spbReplyRPDefaultClick");
    if(CUSTOMER_CODE==CC_KYEC_LEE || CUSTOMER_CODE==CC_KYEC_JCTHIU)             // :25788-25792
        Cust("main.cpp:25788-25792", "CC_KYEC_LEE / CC_KYEC_JCTHIU: ShowMyMessage(\"DownLoad Setup File Please Execute:\", ...)");
    if(CUSTOMER_CODE==CC_ASE_SG)                                                // :25794-25797
        Cust("main.cpp:25794-25797", "CC_ASE_SG: BackupSetupFile()");

    Gap("main.cpp:25798", "dmTrayMotor->StartSetSpeed()", "tray step-motor speed write (Loader tray stepper) -- dmTrayMotor is not in the port; MOTOR ACTION");
    Gap("main.cpp:25799", "GetCZSiteMap(false)", "sends the site map to the GPIB bridge -- not in the port; TESTER COMM");
    if(CosFunction.bManualSteplAutoTeach && IniConfig.bA56EnableAutoTeachFunciton && IniConfig.bN14_21_SetUpConfiguration==false)   // :25800-25801
        Gap("main.cpp:25800-25801", "fAutoTeach->DoAutoTeachStart()", "TfAutoTeach is not in the port; MOTION");
    bHandlerChangeState=true;                                                   // :25802 Sam 20230511 : 機台資料變更後須上傳 FTP
    Gap("main.cpp:25803", "ChangeStateUploadServer()", "FTP upload of the machine state -- not in the port; NETWORK");
    if(CosFunction.bOLPFunction)                                                // :25804 Sam 20230921 : Bin 設定錯誤不能啟動
    {
        LastSet.OLPSetBinErr[0]=0;
        LastSet.OLPSetBinErr[1]=0;
        LastSet.OLPSetBinErr[2]=0;
    }

    TesterTCP_CopyRecipeToTester(FileName);                                     // :25811 Steven 20250612 : for OS Tester.（本體自己看 IniConfig.bN06_CopyTesterFile）

    if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                                     // :25813-25814
        fShuttleMove->ReadData();   // Steven 團隊 20260926 (S12-C ShuttleMove)：golden :25814（本體 forms/fShuttleMove.cpp，golden ShuttleMove.cpp:2252）；S47：只有 latch 機台讀（golden 條件照舊）

    return 0;
}

// =============================================================================
//  golden TfMain::cbSetupFileNameChange(TObject *Sender)  main.cpp:25351-25652
// =============================================================================
static void W906_RC_cbSetupFileNameChange()
{
    int mode=0;
    int ret=0;
    AnsiString str1="", str2="";
    iAdaptiveACInterval=-1;                                                     //Sam 20240726 : AI Clean
    mode=W906_RC_ChangeSetUpFile(fMain->cbSetupFileName->Text);                 // :25357

    bFTPDownlodFinish=false;                                                    //JerryYang 20220215 : 沒download完成不能start
    if(CUSTOMER_CODE==CC_SCK)                                                   // :25361-25375
        Cust("main.cpp:25361-25375", "CC_SCK: elConfig [Monitor] checkboxes -> true, SaveEditTextToFile(config.ini), SendCommand_EventLog");

    if(CosFunction.bFTPFunction)                                                // :25377-25380 Steven 20110305
        Gap("main.cpp:25379", "fLotInfo->btSaveSetupFile->Visible=false", "btSaveSetupFile is not on the fLotInfo facade (screen only)");

    if(mode==1)                                                                 // :25382
    {
        bNoNeedLoadSteupFile=true;                                              //Steven 20101221
        fMain->iHasChangeFile=2;
    }

    if(IniConfig.bEnableCCDUSETCPIP)                                            // :25388-25389
        Gap("main.cpp:25389", "CCDInterfaceForm->Edit1->Text=cbSetupFileName->Text", "CCD interface form Edit1 is not in the port; CCD COMM");

    if(IniConfig.bUseAutoSiteMapping)                                           //jou 2011-03-24 start : Auto Site Mapping   :25391
    {
        if(CUSTOMER_CODE==CC_SCC && RunInfo.bLotStart==true)
        {
        }
        else
        {
            if(IniConfig.bI21EnableASM)                                         //Steven 20110502
            {
                if(TestIF_File.iTestMode==SingleSite)                           //Steven 20130610 : Single Site不做Auto Site Mapping
                {
                    if(IniConfig.bVTESTFunction==true &&                        //Steven 20221220 : for VTest switch to RT
                       fLotInfo->cbRunMode->Text.Pos("RT")>0)
                    {
                        fMain->SetMainRunStartMode(rsmContinuRetest);
                    }
                    else
                    {
                        fMain->SetMainRunStartMode(rsmContinuStart);
                    }
                }
                else
                {
                    for(int i=0; i<MAX_SOCKET_ROW; i++)
                    {
                        for(int j=0; j<MAX_SOCKET_COL; j++)
                            iAutoSiteMap[i][j]=0;                               //kevin 20150114
                    }

                    if(IniConfig.bVTESTFunction==true)                          //Steven 20221220 : for VTest switch to RT
                    {
                        if(fLotInfo->cbRunMode->Text.Pos("RT")>0)
                            iAutoSiteMapRunStartMode=1;
                        else
                            iAutoSiteMapRunStartMode=0;
                    }
                    fMain->SetMainRunStartMode(rsmAutoSiteMap);
                }
            }
            else
            {
                if(CUSTOMER_CODE==CC_KYEC_LEE &&
                   iOldRunStartMode!=rsmContinuStart)
                {
                    // :25431-25456 客戶專屬（KYEC 的 AMR／MRT 判斷與 YES/NO 確認框）先跳過；照一般路徑 ret=1
                    Cust("main.cpp:25431-25456", "CC_KYEC_LEE: AMR / MRT check + ShowMyMessageBox_YES_NO(\"Run Start Mode Change ...\")");
                    ret=1;
                }
                else
                {
                    ret=1;
                }

                if(ret==1)
                {
                    if(IniConfig.bVTESTFunction==true &&                        //Steven 20221220 : for VTest switch to RT
                       fLotInfo->cbRunMode->Text.Pos("RT")>0)
                    {
                        fMain->SetMainRunStartMode(rsmContinuRetest);
                    }
                    else
                    {
                        fMain->SetMainRunStartMode(rsmContinuStart);            //kevin 20150108 AUTO SITE map不是強致模式切換工作檔 initial
                    }
                }
                else
                {
                    SetRunStartMode((eRunStartMode)iOldRunStartMode);           //kevin 20150108 ... //Ifor 20170216 (wei) 避免FTP下載後，選擇不切換模式導致未更換Bin別設定
                }
            }
        }
        // ⓘ 移植樹 TfMain::SetMainRunStartMode 是門面的「documented GAP」空殼（forms/fMain.h:184-190），
        //   golden 的 run start mode 切換本體不在這裡執行 —— 見 ack.notWired。
        Gap("main.cpp:25391-25486", "fMain->SetMainRunStartMode(...)", "TfMain::SetMainRunStartMode is a documented no-op GAP stub on the port facade (forms/fMain.h:184-190); RUN-MODE SWITCH, left for Jimmy");
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN)                                           // :25488-25492
        Cust("main.cpp:25488-25492", "CC_TSMC_TAINAN: SetRunStartMode(rsmInitialStart) + Clarn_Data(1, \"TSMC_SetupFileNameChange\")");
    else if(IniConfig.bAMDFunction)                                             // :25493-25496（AMD 客戶功能）
        Cust("main.cpp:25493-25496", "IniConfig.bAMDFunction (AMD): SetRunStartMode(rsmInitialStart)");

    if(CosFunction.bLastSetInSetUpFile)                                         //Steven 20111019   :25498
    {
        fMain->ShowTestHeadComp(false);
        fMain->LoadRunModePicture();
    }

    Gap("main.cpp:25504-25506", "fAutomation->AmkorSendMessage(0); MySleep(10); fAutomation->AmkorSendMessage(1)", "Amkor site-map monitoring message -- TfAutomation::AmkorSendMessage is not in the port; HOST COMM");

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem   :25508
    {
        if(bNoSendSiteOnOff==false)
            EventReport(SECS_EVENT.SwitchSetupFile);                            // 移植樹 EventReport 是 sim 記錄器（SecsEventReport.cpp:15），不送主機
        if(CUSTOMER_CODE==CC_KYEC_LEE ||
           CUSTOMER_CODE==CC_KYEC_CHEN ||
           CUSTOMER_CODE==CC_KYEC_JCTHIU ||
           CUSTOMER_CODE==CC_KYEC_XILINX)
            Cust("main.cpp:25512-25517", "KYEC: bSECSGEMNoSendEC=true");
    }
    // :25520-25522 #ifdef ASE_KaohSiung fBuilder->bSaveAsJobFile(...) —— 客戶巨集，本建置沒定義

    AnsiString asData1="", asData2="";                                          // :25524-25528
    asData1.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[0], Temperature.bATC7ChannelEnabled[1]);
    asData2.sprintf("%d,%d", Temperature.bATC7ChannelEnabled[2], Temperature.bATC7ChannelEnabled[3]);
    ATCInterfaceForm->SendCommToATC7(ATC_CH_ENABLED, asData1, asData2);          // 只有 ATC7 用戶端連著才真的送（ATC/ATCInterface.cpp:2165）
    ATCInterfaceForm->SendCommToATC7(ATC_RUN, "", "");

    if(BAR_CODE_INSTALL==ebctUseCCDMode && TestIF_File.bEnableBarCode)          // :25530-25533
        Gap("main.cpp:25532", "fBarCode->Change2DSetupFile()", "BarCode/BarCode.cpp:491 GATE(G-BC-TAIL): Change2DSetupFile is not on the partial TfBarCode; CCD COMM");

    if(USE_TRAY_MAPPING!=etmUninstall &&                                        // :25535-25540
       (TestIF_File.bEnableTrayMap ||                                           //wei 20161219 Tray Mapping
        TestIF_File.bEnableDeviceRemain))                                       //wei 20170317 (steven) Device Remain 殘料檢測
    {
        fTrayMappingForm->ChangeTraySetupFile();
    }

    if(IniConfig.bB02_HanderMajorMaintenanceRecordFunction &&                   // :25542-25549
       CUSTOMER_CODE==CC_Greatek)
        Cust("main.cpp:25542-25549", "CC_Greatek: fObserver->ShowModal() (Major Maintenance Record)");

#ifndef SOFT_SIMULTE
    if(BAR_CODE_INSTALL!=ebctUninstall &&                                       // :25551-25566
       TestIF_File.bEnableBarCode)
        Gap("main.cpp:25551-25566", "barcode reader re-connect (InitialBarcodeScanChangeFile / TimerBarcodeChangeFile / btBarcodeChangeFileDisConnect)", "barcode reader connection is not in the port; DEVICE COMM");
#endif

    if(JCET_FOR_EVAN!=0)                                                        // :25568-25572
        Cust("main.cpp:25568-25572", "JCET_FOR_EVAN: RunICModeChange(true, REALLY) + ChangeTesterConnect(ON_LINE)");

    if(CosFunction.bUseMRTMode==true)                                           // :25574-25575
        fMain->SetStartModeData();                                              // 門面 stub

    if(CosFunction.bRTCAutoModelVerify==true &&                                 // :25577-25582 jou 2014-06-24 RTC 自動進行Model驗證
       IniConfig.bD36EnableRTCAutoModelVerify==true &&
       bNeedWaitRTCAutoVerify==false)
    {
        bRTCAutoModelVerifyFirstTime=true;
    }

    if(TestIF_File.iTestType==TTL_MODE &&                                       // :25584-25588
       (TTL_CARD_TYPE==2 || TTL_CARD_TYPE==3))
        Gap("main.cpp:25587", "fMain->CloseGpibProgram(__FUNC__)", "closes the external RS232/GPIB bridge program -- not in the port; EXTERNAL PROCESS");

    if(CosFunction.bManualSteplAutoTeach &&                                     // :25590-25598
       IniConfig.bA56EnableAutoTeachFunciton)
    {
        IniConfig.bA30SetupTeachFunction=true;                                  // :25593（記憶體）
        Gap("main.cpp:25594-25597", "ShowMyMessage(\"Can't Run Auto Alignment Mode When The Machine Has IC!\") / fAutoTeach->StartAutoTeach(true)", "TfAutoTeach is not in the port; MOTION");
    }

    if(fMain->iHasChangeFile==0 || fMain->iHasChangeFile==9)                    // :25600
    {
        fMain->iHasChangeFile=1;
    }

    if(CUSTOMER_CODE==CC_GIGAS)                                                 // :25605-25609
        Cust("main.cpp:25605-25609", "CC_GIGAS: fRPDefault->ShowMonitoredParameter() + RunBatchCopyRecipe(...)");
    ReadConfigByRecipe();                                                       //JimmyChiu 20220601 : config儲存跟隨recipe   :25610

    if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall)                  // :25612-25616（&& ScannerAOIIF.bEnabledPositionByAOI==false —— V912 新欄位，移植樹 SYSTEM_SCANNER_AOI_IF 沒有）
        Gap("main.cpp:25615", "FrmAOI->SendCommand(\"@GetGrabPos+\", 0)", "Scanner AOI is not in the port; DEVICE COMM");

    for(int i=0; i<eTrayCount; i++)                                             //JerryYang 20231218 : P53防混功能   :25618
    {
        sStackBinTemp[i]="";
    }

    if(IniConfig.bF33_Check2DHardware)                                          //JerryYang 20250220 : 2DID硬體順序檢查功能   :25623
    {
        if((BAR_CODE_INSTALL==ebctUseCCDMode     ||
            BAR_CODE_INSTALL==ebctInShtIntel     ||
            BAR_CODE_INSTALL==ebctEtherNetCCD) &&
          TestIF_File.bEnableBarCode)
        {
            if(InArmSuck.iShtRow==2)
                Gap("main.cpp:25631-25632", "bRun2DCheck=true; fContact->Do2DIDMapCheck(true)", "2D-ID hardware order check -- Do2DIDMapCheck is not on the port TfContact; DEVICE COMM");
        }
    }

    // :25638-25645 cbSetupFileName->Width=400/293 —— 純 VCL 版面，網頁不照做

    if(CosFunction.bFirstTrayCheckOnUnloader==true &&                           //Jimmychiu 20251205 : First Tray Check On Unloader   :25647
       IniConfig.bP62FirstTrayCheckOnUnloader==true)
    {
        //AI(ht9045-v899) 20260525: sync PASS auto positions after workfile switch
        fLotInfo->SetFirstTrayCheckOnUnloader();
    }
    (void)ret; (void)str1; (void)str2;
}

// =============================================================================
//  WS recipe.change（tools/wb_serve.cpp 分派）
//    value = {"op":"list"}                      golden cbSetupFileNameDropDown（:31154-31173）
//    value = {"op":"change","name":"<配方名>"}  golden 選了清單項目 → cbSetupFileNameChange（:25351）
// =============================================================================
static void WriteState(webbridge::JsonWriter& w)
{
    w.Key("current").String(U8(fMain->cbSetupFileName->Text));
    w.Key("setupInf").String(U8(GetLastOpenFN()));
    w.Key("enabled").Bool(fMain->cbSetupFileName->Enabled && !SystemStart);
    w.Key("comboEnabled").Bool(fMain->cbSetupFileName->Enabled);
    w.Key("systemStart").Bool(SystemStart);
    w.Key("accessLevel").Number((wb_int64)AccessLevel);
    w.Key("iHasChangeFile").Number((wb_int64)fMain->iHasChangeFile);
}

std::string W906_RecipeChangeOp(const std::string& payloadJson, bool* ok)
{
    if (ok) *ok = false;
    std::string op, name;
    {
        cJSON* root = cJSON_Parse(payloadJson.c_str());
        const cJSON* jop = root ? cJSON_GetObjectItemCaseSensitive(root, "op") : 0;
        const cJSON* jnm = root ? cJSON_GetObjectItemCaseSensitive(root, "name") : 0;
        if (jop && cJSON_IsString(jop) && jop->valuestring) op = jop->valuestring;
        if (jnm && cJSON_IsString(jnm) && jnm->valuestring) name = jnm->valuestring;
        if (root) cJSON_Delete(root);
    }

    FormLockGuard lock;
    MsgCapture cap;
    Rep rep;
    g_rep = &rep;
    filerw::SessionBegin("");
    std::string guard, guardGolden, exc;
    bool changed = false;
    int mode = -1;
    const AnsiString previous = fMain->cbSetupFileName->Text;

    try {
        if (op != "list" && op != "change") {
            guard = "bad-op"; guardGolden = "value must be {\"op\":\"list\"} or {\"op\":\"change\",\"name\":\"...\"}";
        } else if (!W906_RecipeChainsReady()) {
            guard = "boot-read-chain-not-run"; guardGolden = "tools/wb_serve.cpp W906_DoReadLastData(true) did not run (no active recipe at boot)";
        } else if (SystemStart) {
            guard = "SystemStart"; guardGolden = "golden ckernel.cpp:1084-1086 ShowRunLabel: SystemStart -> EnabledSetupFile(false)";
        } else if (!fMain->cbSetupFileName->Enabled) {
            guard = "cbSetupFileName-disabled";
            guardGolden = "golden main.cpp:29354-29447 EnabledSetupFile (IC in machine / level [41] / RMS / FTP / ERMS), set by ShowRunLabel ckernel.cpp:935-1726";
        } else if (fSecurity == 0 || fSecurity->Insufficient(9, false) == false) {
            guard = "not-authorized"; guardGolden = "golden main.cpp:31156-31157 cbSetupFileNameDropDown: fSecurity->Insufficient(9)==false -> return (WAR1676 Insufficient privileges)";
        } else {
            // golden cbSetupFileNameDropDown :31159-31167 bUseBarCoderChangeSetupFile（C16，條碼槍輸入工作檔）
            if (CosFunction.bUseBarCoderChangeSetupFile && IniConfig.bC16UseBarCoderChangeSetupFile)
                Gap("main.cpp:31159-31167", "InputBarcodeNumber(\"Input SetupFile:\") + cbSetupFileNameChange", "barcode-reader input dialog is not in the port; the web list is offered instead");
            AnsiString temp=fMain->cbSetupFileName->Text;                       // :31169
            W906_RC_LookForFile();                                              // :31170
            fMain->cbSetupFileName->Text=temp;                                  // :31171
            if (op == "change") {
                const AnsiString want = FromU8(name);
                int idx = -1;
                for (int i = 0; i < fMain->cbSetupFileName->Items->Count; i++)
                    if (fMain->cbSetupFileName->Items->Strings[i] == want) { idx = i; break; }
                if (want == "") {
                    guard = "empty-name"; guardGolden = "value.name is empty";
                } else if (idx < 0) {
                    guard = "not-in-list";
                    guardGolden = "golden main.cpp:29477-29485 cbSetupFileNameKeyPress/KeyDown Key=0: only an item LookForFile listed (IniData\\Data\\<folder>) can be selected";
                } else {
                    // VCL：選了清單項目 → ItemIndex／Text 改變 → OnChange（main.dfm:10564 OnChange = cbSetupFileNameChange）
                    fMain->cbSetupFileName->ItemIndex = idx;
                    fMain->cbSetupFileName->Text = fMain->cbSetupFileName->Items->Strings[idx];
                    bNoNeedLoadSteupFile = false;
                    fMain->W906_cbSetupFileNameChangeCallCount++;   // 門面的呼叫計數（forms/fMain.cpp:483 同一個計數器）
                    W906_RC_cbSetupFileNameChange();
                    mode = bNoNeedLoadSteupFile ? 1 : 0;
                    if (bNoNeedLoadSteupFile)                                   // golden Timer3Timer :25945-25949
                    {
                        fMain->cbSetupFileName->Text=GetLastOpenFN();
                        bNoNeedLoadSteupFile=false;
                    }
                    changed = (mode == 0);
                }
            }
        }
    } catch (const std::exception& e) {
        exc = std::string("exception in golden recipe change: ") + e.what();
    } catch (...) {
        exc = "exception in golden recipe change (non-std)";
    }
    g_rep = 0;

    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    if (!guard.empty()) { w.Key("guard").String(guard); w.Key("guardGolden").String(guardGolden); }
    if (!exc.empty()) w.Key("exception").String(exc);
    if (op == "change") {
        w.Key("requested").String(name);
        w.Key("previous").String(U8(previous));
        w.Key("mode").Number((wb_int64)mode);          // golden ChangeSetUpFile 回傳：0＝換好、1＝被守衛擋下（combo 退回 setup.inf 的名字）
        w.Key("changed").Bool(changed);
        w.Key("readChainRuns").Number((wb_int64)rep.readChainRuns);   // golden ChangeSetUpFile 叫 DoReadLastData 兩次（:25724、:25772）
    }
    w.Key("items").BeginArray();
    for (int i = 0; i < fMain->cbSetupFileName->Items->Count; i++) w.String(U8(fMain->cbSetupFileName->Items->Strings[i]));
    w.EndArray();
    WriteState(w);
    w.Key("messages").BeginArray();
    for (size_t i = 0; i < cap.msgs.size(); ++i) {
        w.BeginObject();
        w.Key("s1").String(U8(AnsiString(cap.msgs[i].first.c_str())));
        w.Key("s2").String(U8(AnsiString(cap.msgs[i].second.c_str())));
        w.EndObject();
    }
    w.EndArray();
    w.Key("notWired").BeginArray();
    for (size_t i = 0; i < rep.notWired.size(); ++i) {
        w.BeginObject(); w.Key("golden").String(rep.notWired[i].golden); w.Key("what").String(rep.notWired[i].what);
        w.Key("why").String(rep.notWired[i].why); w.EndObject();
    }
    w.EndArray();
    w.Key("customerSkipped").BeginArray();
    for (size_t i = 0; i < rep.customer.size(); ++i) {
        w.BeginObject(); w.Key("golden").String(rep.customer[i].golden); w.Key("what").String(rep.customer[i].what); w.EndObject();
    }
    w.EndArray();
    w.Key("session").RawValue(filerw::SessionJson());   // FileRW 讀檔器記下的 golden 訊息／待辦（例：DIO 檔遺失）
    w.EndObject();
    filerw::SessionBegin("");

    const bool good = guard.empty() && exc.empty() && (op == "list" || mode == 0);
    if (ok) *ok = good;
    std::printf("recipe.change op=%s%s%s -> %s  current=%s  chainRuns=%d notWired=%u customerSkipped=%u messages=%u\n",
                op.c_str(), op == "change" ? " name=" : "", op == "change" ? name.c_str() : "",
                !exc.empty() ? exc.c_str() : !guard.empty() ? guard.c_str() : mode == 1 ? "refused-by-ChangeSetUpFile" : "ok",
                fMain->cbSetupFileName->Text.c_str(), rep.readChainRuns,
                (unsigned)rep.notWired.size(), (unsigned)rep.customer.size(), (unsigned)cap.msgs.size());
    return w.Ok() ? w.Str() : std::string("{\"guard\":\"json-writer-misuse\"}");
}
