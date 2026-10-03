// =============================================================================
//  FileRW/LotInfo_SECSLotStart.cpp -- golden TfLotInfo::sbSECSLotStartClick（Lot Start 鈕）照翻成一支可呼叫的函式。
//
//  //AI(W906-E020-LI1) 20261002 [W906] (St01): new file -- todo E-020 LI-1 (the laptop's card, TO_STEVEN s4 1001 23:3x on main;
//    St02 E-019 docs/E019_DATA_STATUS_EVENTS_20261001.md row LI-1).
//  golden: V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp:7501-8583 (1083 lines incl. the braces; cp950 ->
//    UTF-8 for reading).  The card's ":7382-8416" is the 906 tree's numbering (HT9011UC_Code_V3.33.906.0_20260618, the tree
//    forms/fLotInfo.h cites: N23UseLotInfoFile :7346-7380 there, this function right after it).  Every cite below is V912.
//  Helpers golden calls from this TU (V912 uLotInfo.cpp, file-static there too): WritePTILotStartTrace :99-120,
//    NormalizePTIRunModeBeforeLotStart :123-155; TfBarCode::JCETUseMakeWhite2DIDList V912 BarCode/BarCode.cpp:11340-11346.
//
//  CALLER.  tools/wb_serve.cpp lot.start arm (:5757-5789, the laptop's, c8e5e3aa) sets edtSysLotID / edtSysOperatorID and then
//    calls this instead of fLotInfo->SetLotStart("wb_serve::lot.start") -- the laptop changes that line, this file does not touch
//    wb_serve.cpp.  One-line call (block-scope extern, the arms' convention):
//      extern bool W906_LotInfo_SECSLotStart(std::string* whyNot, std::vector<std::string>* skipped); std::string lsWhy; const bool lsRan = W906_LotInfo_SECSLotStart(&lsWhy, 0); if (!lsRan) std::printf("lot.start: golden sbSECSLotStartClick refused: %s\n", lsWhy.c_str());
//    and ok = lsRan && RunInfo.bLotStart != 0 (+ "whyNot" in the reply) on the CompleteCommand line.
//  THREAD / MODALS (measured 20261002 on this tree): the arm is in the tick-thread command chain, the same if/else chain as
//    start.run (wb_serve.cpp:5669), inside #ifdef WB_PUMP_1203_CONTROL (:5627-:5792), with NO FormLock held.  Golden's
//    ShowMyMessage -> canary_support.cpp W906_ShowMyMessageEx_Hook -> wb_serve MbWait (:6755, blocks and pumps commands until the
//    browser answers) and ShowErrorMessage -> W906_ShowErrorMessage_Hook (ForwardShowErrorMessage pump), so golden's modal waits
//    run as golden's, like StartFromWeb's.  This file does NOT take FormLock: holding it across a modal wait would freeze every
//    HTTP /api/form read until the operator answers.  Widget writes therefore stay unlocked, as the arm's own two Text writes are.
//  CTEST: canary_support's sims (ShowMyMessage records; ShowErrorMessage returns W906_ShowErrorMessage_SimReturn).
//
//  GATES (golden callee / member missing in the port).  Two kinds, ST01-E 20261002: where golden would RETURN we refuse with
//  whyNot; where golden only ACTS we skip and log (printf + *skipped).  Nothing is invented.
//    LI1-1  :7506  DoPassword() (TfLotInfo, :8654-8685: fMain->cbUserSelectChange / stOperatorClick login dialog, fInput,
//                  fPassword->Label3/4) -- not in the port (forms/fLotInfo.h); the dialog has no web form here.  VTEST VTENG
//                  lot only.  REFUSE.
//    LI1-2  :7560 / :7957  rgSort2DID (TRadioGroup, golden uLotInfo.h:850) -- no such widget in forms/fLotInfo.h.  str2 keeps
//                  the value golden had before the line.  SKIP.
//    LI1-3  :7600  fFTPClient->Download_2DSortingList -- the port's free function (KYECFTP/FTPClient_Transfer.cpp) lives in
//                  ht9045_kyecftp, which wb_serve does not link.  SKIP (the FileExists check right after it stays real).
//    LI1-4  :7609-7748  the 2D sorting list load (fBarCode->list2DSorting / s2DSorting / TransformListToMap) -- TfBarCode
//                  (BarCode/BarCode.h, 21 members) has none of them; the 2D-sort engine itself is gated (asortarm.cpp list2DByLot
//                  / map2DList).  Starting a 2D-sort lot without its list would bin by an empty map.  REFUSE.
//    LI1-5  :7789-7833 / :7869+:7903 / :7963-7995  2DID white list: fBarCode->GetWhite2DIDList / bNeedCheckWhitleList /
//                  iJCETWhitelistSN / list2DWhitle / list2DWhitleResult -- not members of TfBarCode (the BarCode_Shuttle*
//                  globals of the same names are per-TU stand-ins, trap #2).  The two list loads REFUSE; the two
//                  bNeedCheckWhitleList writes SKIP.
//    LI1-6  :8268-8322  LEADYO [N33_1] TestInfo.txt: bCheckOnlyOneFileAndData (golden :14490-) is not in the port and
//                  fMain->cbSetupFileNameChange is a counter stub (forms/fMain.cpp:483) -- running it would claim a recipe
//                  switch that does not happen.  REFUSE.
//    LI1-7  :8216  fMesSystem->RunModeRW -- declared, not defined by design (forms/fMesSystem.h GATE W-09: a call is a link
//                  error).  VTEST_Shanghai repeat-run check.  REFUSE.
//    LI1-8  (removed by E-030, 20261003) V912 :8397 fBarCode->ResetBarcodeCSVForLotStart is 912-only; golden 906 has no
//                  Barcode CSV Compare, so the port follows 906 (Jimmy RULINGS_20261002 #20 / #23 item 6).  The number stays free.
//    LI1-9  :8404-8443  VTEST MES lot download (fMesSystem->LabeledEditOPID / buttonDownloadLotInfor; the download itself is
//                  forms/fMesSystem.h GATE W-02).  REFUSE.
//    LI1-10 :8457-8515  OEE (CosFunction.bOEEFunction): fMonitor->MVCtrl is never constructed (forms/fMonitor.h M-1),
//                  Application->MessageBox has no vclcompat surface (same as forms/fLotInfo.cpp GATE W906-PROD-S117-OEEASK),
//                  fProductionInfo has no OEE_StartLot.  REFUSE.
//    LI1-11 :8530-8532  PANTHER fMain->patFunc / UpdateLotInfoPAT / fMain->machineTime -- none in the port.  SKIP; the
//                  LastSet.SystemAccSecond clear (:8533-8534) stays real.
//    LI1-12 :8559-8562  VTEST bVTESTKitCheckPending / DoVTESTChangeKitCheck -- not in the port (same as forms/fLotInfo.cpp
//                  GATE W906-PROD-S117-S25-VTEST).  SKIP.
//    LI1-13 :8581  FormHS->RecordParameter_TFAMDLog -- the port has no TFormHS instance (FormHS is the SCK_ART_Remainder
//                  stand-in; same as csystem.cpp GATE W906-FLOW-1).  SKIP.
//  (a) LOCAL TRANSLATIONS (pure logic, static here, cited): WritePTILotStartTrace, NormalizePTIRunModeBeforeLotStart,
//    JCETUseMakeWhite2DIDList, and the TRegExpr special-character test (precedent SECSGEM/uHGemHT9045.cpp G1a).
//    golden `btClearBarcodeList->Click()` (:8354) = fLotInfo->btClearBarcodeListClick(), the port's seat for that button
//    (forms/fLotInfo.cpp:6544-6557: vclcompat TControl::Click() is a no-op).
//
//  DEVIATIONS (each also marked at its line)
//    D-1  golden "D:\\HT9045_Log\\..." literals here are written as9045LogPath+"\\..." (common.cpp:48, golden literal unless the
//         ctest seam W906_HT9045LOG_ROOT is set; the LogObjects.cpp / common.cpp:52 precedent), and the LEADYO
//         "D:\\HT9045\\IniData\\Data\\%s" would be DataPath (inside the LI1-6 gate, so moot).  Production values unchanged.
//    D-2  golden `SetLotStart(__FUNC__)` (:8565): BCB6 __FUNC__ inside TfLotInfo::sbSECSLotStartClick = that name; the port's
//         __FUNC__ is __func__ (canary_support.h:45) and would say W906_LotInfo_SECSLotStart, so the golden name is passed.
//    D-3  the public wrapper catches exceptions (whyNot "exception in ...") like the other wb_serve arms; golden VCL would show
//         its exception box.  Return value, whyNot and skipped are the port's report, not golden state.
//  GOLDEN ODDITIES KEPT: the special-character checks blank the field and go on (no return; '!' is rejected but not listed in
//    the message); SIGURD_ChungXing shows "Please Enter lot information!!" and goes on (:8187-8188); JCET sets
//    sbSECSLotEnd->Down=false where every other branch sets true (:8336/:8344); golden deletes its TRegExpr only in the Murata
//    branch (:8140); the white-list Net Drive loop keeps the LAST matching file (:7894-7906, no break) and leaks MyList when no
//    file matches (:7933-7939); QLE builds the file name from a stale sWhiteListProcess (:7878) and SJ sets sWhiteListProcess
//    from SubString(iPos+1, ...) with iPos still 0 (:7885).
//
//  ⚠ REAL FILES this function writes (golden does; ctest redirects every one of them):
//    AuthPath+"config.ini" [Lot Info]           SCC SetLotID("") (SHIP only), SetLotID(:8536), SetLotStart(:8565) -> ReadWriteLotInfo
//    D:\HT9045\system\ArmByLot0..2.dat          SetLotStart -> ArmDataLot[i]->WriteFile() (W906_MACHINERECORD_DIR in ctest)
//    D:\HT9045_Log\2D_SortList\*                2D sort / white list only (delete + Net Drive copy)
//    D:\HT9045_Log\PTI_LotStartTrace\*          PTI only
//    the event log through RecordProcess / slEventLog (SetLotStart)
// =============================================================================
#include "FileRW/LotInfo_SECSLotStart.h"

#include "MachineType.h"        // SOFT_SIMULTE (:64), CC_*, rsm*, eTrayCount
#include "forms/fLotInfo.h"     // TfLotInfo / fLotInfo (read only; the shared form file is not edited)
#include "forms/fMesSystem.h"   // fMesSystem->CheckVTENGmode / bDownloadLotInforFlag / bNoRTBinFlag
#include "cmydef.h"             // CUSTOMER_CODE, AccessLevel, iDefSupervisorLevel, OFFLINE_ALARM, bReadLotInfoFromART, iByBinCnt,
                                //   iExceptAutoCnt, sTotalLotID, sWhiteListLotID/Process, bNoRTBinFixFlag, iAMD_Function, SPIL_FOR_QLE
#include "cprod.h"              // TestIF_File, TestIF, RunInfo, TrayForm, dtStartLot
#include "Config.h"             // IniConfig
#include "CosFunction.h"        // CosFunction
#include "LastSet.h"            // LastSet
#include "common.h"             // as9045LogPath, asBarCodeLogPath, as2DWhiteListLog(Name), MyForceDirectories, WriteDataToFile
#include "canary_support.h"    // ShowMyMessage, ShowErrorMessage, RecordProcess
#include "cpublic.h"            // GetTimeInfo

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// csystem.h:105 / :109 -- not included (only these two; same as FileRW/MainClick.cpp:78 / :995).
bool HasICUnderMachine();
bool HasAnyICInMachine();

namespace {

// ---- the port's report (not golden state) ------------------------------------------------------------------------------
bool LI1_Refuse(std::string* whyNot, int goldenLine, const AnsiString& text)
{
    std::printf("[LotStart] golden uLotInfo.cpp:%d returns -- %s\n", goldenLine, text.c_str());
    if (whyNot) *whyNot = text.c_str();
    return false;
}

AnsiString LI1_Two(const AnsiString& s1, const AnsiString& s2)                 // the "S1 | S2" form ShowMyMessage logs
{
    return s2.Length() > 0 ? s1 + " | " + s2 : s1;
}

void LI1_Skip(std::vector<std::string>* skipped, const char* text)
{
    std::printf("[LotStart] skipped (gated) %s\n", text);
    if (skipped) skipped->push_back(text);
}

// D-1: golden "D:\\HT9045_Log" + tail.
AnsiString LI1_Log(const char* tail)
{
    return as9045LogPath + tail;
}

// BCB6 SysUtils IncludeTrailingPathDelimiter = IncludeTrailingBackslash.  Every TU that needs it carries its own copy
// (forms/fLotInfo.cpp:123-132 names the others); this one too.
AnsiString IncludeTrailingPathDelimiter(const AnsiString& p)
{
    return IncludeTrailingBackslash(p);
}

// ---- (a) golden uLotInfo.cpp:99-120 WritePTILotStartTrace ----------------------------------------------------------------
void WritePTILotStartTrace(TfLotInfo *pLotInfo, AnsiString Stage, AnsiString Detail)
{
    if(CUSTOMER_CODE!=CC_PTI || pLotInfo==NULL)
        return;

    GetTimeInfo();

    AnsiString Path, FileName, Log, ProcLog;
    Path.sprintf("%s\\PTI_LotStartTrace\\%04d_%02d", as9045LogPath, SystemYear, SystemMonth);   // golden "D:\\HT9045_Log\\PTI_LotStartTrace\\%04d_%02d" (D-1)
    MyForceDirectories(Path);
    FileName.sprintf("%s\\%04d_%02d_%02d.txt", Path, SystemYear, SystemMonth, SystemDate);

    Log.sprintf("%04d-%02d-%02d,%02d:%02d:%02d.%03d,Stage=%s,AccessLevel=%d,StartMode=%d,RunMode=%s,LotID=%s,OPID=%s,Tester=%d,TestType=%d,%s",
                SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec,
                Stage, AccessLevel, LastSet.iRunStartMode, pLotInfo->cbRunMode->Text,
                pLotInfo->edtSysLotID->Text, pLotInfo->edtSysOperatorID->Text,
                LastSet.iTester, TestIF.iTestType, Detail);
    WriteDataToFile(FileName.c_str(), Log.c_str());

    ProcLog.sprintf("PTI LotStart Trace: %s, RunMode=%s, %s", Stage, pLotInfo->cbRunMode->Text, Detail);
    RecordProcess(ProcLog);
}

// ---- (a) golden uLotInfo.cpp:123-155 NormalizePTIRunModeBeforeLotStart -----------------------------------------------------
void NormalizePTIRunModeBeforeLotStart(TfLotInfo *pLotInfo)
{
    if(CUSTOMER_CODE!=CC_PTI || pLotInfo==NULL || pLotInfo->cbRunMode->Text!="")
        return;

    AnsiString OldRunMode=pLotInfo->cbRunMode->Text;
    AnsiString NewRunMode="Normal";
    AnsiString Detail;

    if(IniConfig.bB03_TesterReport)
    {
        NewRunMode="1'st";
    }
    else if(LastSet.iRunStartMode==rsmContinuRetest ||
            LastSet.iRunStartMode==rsmCInitialRetest)
    {
        NewRunMode="Re-Test";
    }

    pLotInfo->cbRunMode->Text=NewRunMode;
    pLotInfo->cbRunMode->ItemIndex=-1;
    for(int i=0; i<pLotInfo->cbRunMode->Items->Count; i++)
    {
        if(pLotInfo->cbRunMode->Items->Strings[i]==NewRunMode)
        {
            pLotInfo->cbRunMode->ItemIndex=i;
            break;
        }
    }

    Detail.sprintf("OldRunMode=%s,NewRunMode=%s", OldRunMode, NewRunMode);
    WritePTILotStartTrace(pLotInfo, "NormalizeRunMode", Detail);
}

// ---- (a) golden BarCode/BarCode.cpp:11340-11346 TfBarCode::JCETUseMakeWhite2DIDList ---------------------------------------
//  Pure logic over four flags the port has (cprod.h TestIF_File, CosFunction.h).  Translated here instead of on TfBarCode
//  (BarCode/BarCode.h is another owner's file); same function, same four flags.
bool JCETUseMakeWhite2DIDList()                                                 //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
{
    return (TestIF_File.bEnableBarCode==true &&
            CosFunction.bMakeWhite2DIDList==true &&
            TestIF_File.bChkMakeWhite2DIDList==true &&
            TestIF_File.b2DIDAllowList==true);
}

// ---- (a) golden :8021-8022 TRegExpr: Expression "([<>/!|:\"\*\?])" ------------------------------------------------------
//  One group holding one character class.  BCB6 compiles "\*" as '*' and "\?" is '?', so the class is < > / ! | : " * ?.
//  regex->Exec() searches the whole InputString and MatchLen[0] is the length of the whole match = 1; "" never matches.
//  Golden tests `Text.AnsiPos("\\")!=0 || regex->Exec()` and then `len!=0 || Text.AnsiPos("\\")!=0`; when the backslash test
//  is true Exec() is skipped and MatchLen[0] is stale, but the second test is true anyway -- so the pair is exactly
//  "has a backslash or one of the nine".  Returns golden's len (0 / 1).
int RegexMatchLen(const AnsiString& s)
{
    static const char kSet[] = "<>/!|:\"*?";
    return std::strpbrk(s.c_str(), kSet) != 0 ? 1 : 0;
}

// ---- golden TfLotInfo::sbSECSLotStartClick, V912 uLotInfo.cpp:7501-8583 --------------------------------------------------
//  `this` of golden = L (== fLotInfo; golden spells both `fLotInfo->x` and bare `x`, the same object).
bool SbSECSLotStartClick(TfLotInfo* L, std::string* whyNot, std::vector<std::string>* skipped)
{
    if(IniConfig.bVTESTFunction==true &&                                        //RogerYang 20260327 : VTENGrecipe/VTEErecipe lot號開批需要OP以上權限   // :7503
        fMesSystem->CheckVTENGmode(fLotInfo->edtSysLotID->Text)==true)
    {
        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-1) -- golden :7506-7509, see the file header.
#if 0 // GATE (W906-E020-LI1-1)
        if(DoPassword()==false)
        {
            return;
        }
#else
        return LI1_Refuse(whyNot, 7506, "GATE (W906-E020-LI1-1): VTEST VTENG lot needs golden TfLotInfo::DoPassword (uLotInfo.cpp:8654, login dialog) -- not ported, lot start refused");
#endif // GATE (W906-E020-LI1-1)
    }

    bReadLotInfoFromART=false;                                                  // :7512
    int ret=0, ret2=0, iBin, iIndex=0, iPos=0, temp;
    //    static int iCount;                                                    // :7514 -- only the LI1-4 gated block uses it (kept there)

    AnsiString str1="", str2="", str3="", str4="", str5="", strPath="", str="", strSort="", strLotID="",sID="", sFileName="";
    char cSort[1024];
    AnsiString sDeviceID="",sBin="";
    (void)iBin; (void)temp; (void)str3; (void)str4; (void)str5; (void)str; (void)strSort; (void)strLotID; (void)sID;   //AI(W906-E020-LI1) 20261002 [W906]: used only inside the gated blocks (no unused warnings)
    (void)sDeviceID; (void)sBin;
    memset(cSort, '\0', sizeof(cSort));
    ZeroMemory(iByBinCnt, sizeof(iByBinCnt));
    ZeroMemory(iExceptAutoCnt, sizeof(iExceptAutoCnt));
    L->mmo2DLotInfo->Clear();
    dtStartLot=Now();                                                           // :7523

    //AI(ht9045-v899) 20260528: block PTI operator Lot Start while tester is offline
    if(CUSTOMER_CODE==CC_PTI &&                                                 // :7526
       OFFLINE_ALARM &&
       LastSet.iTester==OFF_LINE &&
       AccessLevel<iDefSupervisorLevel)
    {
        WritePTILotStartTrace(L, "OfflineOperatorBlocked", "Reason=AccessLevelBelowSupervisor");
        ShowMyMessage("Please check the tester mode is connected"); //AI(ht9045-v899) 20260529: clarify PTI offline tester mode prompt
        return LI1_Refuse(whyNot, 7533, "Please check the tester mode is connected");
    }

    //AI(ht9045-v899) 20260525: normalize PTI RunMode before local validation blocks TesterTCP Lot Start
    NormalizePTIRunModeBeforeLotStart(L);                                       // :7537

    if(CosFunction.bSortingBy2DList==true &&                                    // :7539
       LastSet.iTester==_2D_SORT &&
       TestIF_File.bSortingBy2DIDList==true)
    {
        str2.sprintf("%s\\SortBy2DID_%s.csv", LI1_Log("\\2D_SortList"), sTotalLotID);                         // :7543  golden "D:\\HT9045_Log\\2D_SortList" (D-1)
        if(HasICUnderMachine() && HasAnyICInMachine())                          //JerryYang 20240111 : add
        {
        }
        else
        {
            if(FileExists(str2))                                                //Steven 20160505 : 加上保護, 不然開程式會跳Error
            {
                DeleteFile(str2);
            }
        }

        if(L->N23UseLotInfoFile()==false)                                       // :7555 (it shows its own message, golden N23UseLotInfoFile :7494)
            return LI1_Refuse(whyNot, 7556, LI1_Two("The Lot info for 2DID sorting is missing", "找不到2DID sorting用的Lot info"));

        if(IniConfig.iN23DownloadMethod==3)                                     //手動   // :7558
        {
            //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-2) -- golden :7560-7561, rgSort2DID not in the port.
#if 0 // GATE (W906-E020-LI1-2)
            if(rgSort2DID->ItemIndex==1)                                        //手動選檔案
                str2=edSort2DIDBinFile->Text;
#endif // GATE (W906-E020-LI1-2)
            LI1_Skip(skipped, "GATE (W906-E020-LI1-2): golden uLotInfo.cpp:7560-7561 rgSort2DID->ItemIndex==1 -> str2=edSort2DIDBinFile->Text not run (rgSort2DID widget not ported)");
        }
        else if(IniConfig.iN23DownloadMethod==2)                                //SECS/GEM
        {
        }
        else if(IniConfig.iN23DownloadMethod==1)                                //Net Drive   // :7566
        {
            //----------------------
            //開始下載檔案
            //----------------------
            do
            {
                if(!DirectoryExists(LI1_Log("\\2D_SortList\\")))                // golden "D:\\HT9045_Log\\2D_SortList\\" (D-1)
                {
                    MyForceDirectories(LI1_Log("\\2D_SortList\\"));
                }
                str1.sprintf("%sSortBy2DID_%s.csv", IncludeTrailingPathDelimiter(IniConfig.sN23DownloadDrivePath), L->edtSysLotID->Text);
                str2.sprintf("%s\\SortBy2DID_%s.csv", LI1_Log("\\2D_SortList"), L->edtSysLotID->Text);       // (D-1)

                ret=CopyFile(str1.c_str(), str2.c_str(), false);                //複製一份新的
                ::Sleep(100);

                if(ret==0)
                {
                    ret2=ShowErrorMessage("WAR1684", K_RETRY|K_SKIP, MMSystem, 0, str1);                                //下載失敗，重試
                    if(ret2==K_SKIP)
                        break;
                }
                else
                {
                    ret2=0;
                }
            }
            while(ret2==1);
        }
        else                                                                    //FTP   // :7596
        {
            strPath=IniConfig.cN23FtpDownloadPath;
            str1.sprintf("%s.csv", L->edtSysLotID->Text);
            //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-3) -- golden :7600, see the file header.
#if 0 // GATE (W906-E020-LI1-3)
            fFTPClient->Download_2DSortingList(strPath, str1);
#endif // GATE (W906-E020-LI1-3)
            LI1_Skip(skipped, "GATE (W906-E020-LI1-3): golden uLotInfo.cpp:7600 fFTPClient->Download_2DSortingList not run (ht9045_kyecftp is not linked into wb_serve)");
            str2.sprintf("%s\\%s.csv", "D:\\RMS", L->edtSysLotID->Text);
        }

        if(IniConfig.iN23DownloadMethod==2)                                     //SECS/GEM   // :7604
        {
        }
        else if(FileExists(str2))                                               //Steven 20160505 : 加上保護, 不然開程式會跳Error
        {
            //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-4) -- golden :7609-7748, see the file header.
#if 0 // GATE (W906-E020-LI1-4)
            fSCKART->sLotID=edtSysLotID->Text;
            fBarCode->list2DSorting->Clear();
            fBarCode->list2DSorting->LoadFromFile(str2);
//            fBarCode->list2DSorting->Sort();

            iCount=0;
            sl2DMappingLog->Clear();                                            //JerryYang 20230322 : add 2D mapping result
            sl2DMappingLog->Add("2D Code, Binning, ECID, Device, Lot No., VS-shuttle, VS-unload, RC1-shuttle, RC1-unload, RC2-shuttle, RC2-unload");

            for(int i=0; i<fBarCode->list2DSorting->Count; i++)
            {
                if(CUSTOMER_CODE==CC_AMKOR_Korea)
                {
                    str3=fProductionInfo->GetCSVLineData(5, fBarCode->list2DSorting->Strings[i]);
                    str4=fProductionInfo->GetCSVLineData(11, fBarCode->list2DSorting->Strings[i]);

                    if(str3=="" || str3=="Unit_ID")
                        continue;

                    iBin=atoi(str4.c_str())-48;
                    str5.sprintf("%s=%d", str3, iBin);
                    fBarCode->list2DSorting->Strings[iCount]=str5;
                    iCount++;
                }
                else if(CUSTOMER_CODE==CC_JCET)                                 //RogerYang 20251208 : JCET 2D FT1白名單/FT2比對功能
                {
                }
                else
                {
                    str=fBarCode->list2DSorting->Strings[i];
                    str4=StringReplace(str, '"', "", TReplaceFlags()<<rfReplaceAll);

                    sDeviceID="";                                               //JerryYang 20230322 : add 2D mapping result
                    sBin="";
                    if(str4.Pos(",")>0)
                    {
                        sDeviceID=str4.SubString(1, str4.Pos(",")-1);
                        sBin=str4.SubString(str4.Pos(",")+1, str4.Length());
                    }

                    if(sDeviceID!="" && sBin!="")
                    {
                        sl2DMappingLog->Add(sDeviceID+","+sBin+",,"+edtDevice->Text+","+edtCusLotID->Text+",,,,,,");
                    }
                    str5=StringReplace(str4, " ", "_", TReplaceFlags()<<rfReplaceAll);

                    fBarCode->s2DSorting->CommaText=str5;
                    str1=StringReplace(str, ",", "=", TReplaceFlags()<<rfReplaceAll);
                    strSort=StringReplace(str1, '"', "", TReplaceFlags()<<rfReplaceAll);
                    fBarCode->list2DSorting->Strings[i]=strSort;

                    int iBin=StrToIntDef(fBarCode->s2DSorting->Strings[1], iTestBinCount);
                    if(iBin>=0 && iBin<iTestBinCount)
                    {
                        iByBinCnt[iBin]++;
                    }
                }
            }

            fBarCode->TransformListToMap();                                     //Steven 20240515 : modified for 2D sort
            for(int i=0; i<6; i++)                                              //JerryYang 20240927 : 2D SORT模式Output arm放料避免空洞
            {
                for(int j=0; j<iTestBinCount; j++)
                {
                    temp=Prod.iT6CatData[j];

                    if(temp<=0)
                        continue;

                    if(i==temp-1)
                    {
                        iExceptAutoCnt[i]+=iByBinCnt[j];
                    }
                }
            }

            if(!DirectoryExists("D:\\HT9045_Log\\2D_MappingResult\\"))          //JerryYang 20230322 : add 2D mapping result
            {
                MyForceDirectories("D:\\HT9045_Log\\2D_MappingResult\\");
            }

            str3.sprintf("D:\\HT9045_Log\\2D_MappingResult\\%s_%s_%s_VS_Result.csv", edtSysLotID->Text, edtCusLotID->Text, edtCusDevGrp->Text);                 //JerryYang 20230322 : add 2D mapping result
            if(FileExists(str3)==false)
                sl2DMappingLog->SaveToFile(str3);
            int sum=0, Sum_ART=0;
//            double f=0.0, f1=0.0;
            for(int i=0; i<eTrayCount; i++)
            {
                sum     +=LastSet.BinCT    [0][iTo3Unload[i]];
                Sum_ART +=LastSet.BinCT_ART[0][iTo3Unload[i]];                  //wei 20150923 add ART計數
            }
            int iTemp=0;

            for(int i=0; i<iTestBinCount+1; i++)                                //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
            {
                if(LastSet.iTester==_2D_SORT)
                {
                    fShowBinSelect->StrGrdCategory->Cells[0][1+i]="";
                    fShowBinSelect->StrGrdCategory->Cells[1][1+i]="";
                    fShowBinSelect->StrGrdCategory->Cells[2][1+i]="";
                    fShowBinSelect->StrGrdCategory->Cells[3][1+i]="";
                    if(iByBinCnt[i]>0 || i==iTestBinCount)
                    {
                        if(i==iTestBinCount)
                        {
                            fShowBinSelect->StrGrdCategory->Cells[0][1+iTemp]="Error Bin";
                        }
                        else
                        {
                            fShowBinSelect->StrGrdCategory->Cells[0][1+iTemp]="BIN "+AnsiString(i);
                        }
                        fShowBinSelect->StrGrdCategory->Cells[2][1+iTemp]=LastSet.iBinData32[0][i];
                        fShowBinSelect->StrGrdCategoryART->Cells[1][1+iTemp]=LastSet.iBinData32_ART[0][i];              //wei 20150923 add ART計數
                        fShowBinSelect->StrGrdCategory->Cells[1][1+iTemp]=iByBinCnt[i];
                        if(sum>0)
                        {
//                            f=ChangeToFloat((double)LastSet.iBinData32[0][i], (double)sum);
                            fShowBinSelect->StrGrdCategory->Cells[3][1+iTemp]=ChangeToPercentage((double)LastSet.iBinData32[0][i], (double)sum);
                        }
                        else
                        {
                            fShowBinSelect->StrGrdCategory->Cells[3][1+iTemp]="0.00%";
                        }

                        if((USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest) || CosFunction.bUseARTSortCount)                                         //wei 20150923 add ART計數  //Ifor 20170316 (wei) add MRT Mode
                        {
                            if(Sum_ART>0)
                            {
//                                f1=ChangeToFloat((double)LastSet.iBinData32_ART[0][i], (double)Sum_ART);
                                fShowBinSelect->StrGrdCategoryART->Cells[2][1+iTemp]=ChangeToPercentage((double)LastSet.iBinData32_ART[0][i], (double)Sum_ART);
                            }
                            else
                            {
                                fShowBinSelect->StrGrdCategoryART->Cells[2][1+iTemp]="0.00%";
                            }
                        }
                        iTemp++;
                    }
                }
            }
#else
            return LI1_Refuse(whyNot, 7609, "GATE (W906-E020-LI1-4): 2D sorting list " + str2 + " found, but loading it needs TfBarCode::list2DSorting / s2DSorting / TransformListToMap (not ported) -- lot start refused");
#endif // GATE (W906-E020-LI1-4)
        }
        else
        {
            ShowMyMessage("The 2DID sorting list is missing", "找不到2DID sorting list");                             // :7752
            return LI1_Refuse(whyNot, 7753, LI1_Two("The 2DID sorting list is missing", "找不到2DID sorting list"));
        }
    }
    else
    {
        if(TestIF_File.b2DIDAllowList)                                          // :7758
        {
            if(L->N23UseLotInfoFile()==false)
                return LI1_Refuse(whyNot, 7761, LI1_Two("The Lot info for 2DID sorting is missing", "找不到2DID sorting用的Lot info"));

            if(L->cbRunMode->Text=="" ||                                        //Steven 20250811 : Add run mode check
               L->cbRunMode->ItemIndex==-1)                                     //JerryYang 20230322 : add 2D mapping result
            {
                L->sbSECSLotEnd->Down=true;
                ShowMyMessage("Please select the run mode in lot info!!");
                return LI1_Refuse(whyNot, 7768, "Please select the run mode in lot info!!");
            }
            else if(L->cbRunMode->Text!="CORR")                                 //JerryYang 20241104 : 支援2DID白名單功能   // :7770
            {
                str2.sprintf("%s\\2D_SortList\\Search2DIDByLot.txt", as9045LogPath);                                    // :7772  golden "D:\\HT9045_Log\\2D_SortList\\Search2DIDByLot.txt" (D-1)
                if(FileExists(str2))                                            //Steven 20160505 : 加上保護, 不然開程式會跳Error
                {
                    DeleteFile(str2);
                }

                if(IniConfig.iN23DownloadMethod==0)                             // :7778
                {
                    if(JCETUseMakeWhite2DIDList()==true)                        //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
                    {
                        if(L->edtSysLotID->Text.Length()<3)                     //RogerYang 20260225 : 修改白名單路徑 \\客戶代碼(批次前3碼)\\批次_FTx_csv
                        {
                            AnsiString sMsg;
                            sMsg.sprintf("批號 %s 異常，無法取得客戶代碼(前3碼)", L->edtSysLotID->Text);
                            ShowMyMessage(sMsg);
                            return LI1_Refuse(whyNot, 7787, sMsg);
                        }
                        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-5) -- golden :7789-7833, see the file header.
#if 0 // GATE (W906-E020-LI1-5)
                        TStringList *list2DTemp=new TStringList();
                        AnsiString sLocTmp;
                        AnsiString sCust=edtSysLotID->Text.SubString(0,3);
                        //檢查是否有相同檔名的list, 如果有就是續測. 下載後接續存檔
                        bool rlt=fBarCode->GetWhite2DIDList(edtSysLotID->Text);
                        fBarCode->bNeedCheckWhitleList=
                            (cbRunMode->Text=="FT1" || cbRunMode->Text.Pos("QA")>0)?false:true;     //RogerYang 20260615 : 新增QA      //FT1不用執行
                        if(cbRunMode->Text!="FT1" &&
                            cbRunMode->Text.Pos("QA")==0 &&                     //RogerYang 20260615 : 新增QA
                            rlt==false)                                         //非FT1要確認有沒有FT1的白名單檔案
                        {
                            ShowMyMessage("The 2DID sorting list is missing", "找不到2DID sorting list");
                            return;
                        }
                        str2.sprintf("%s%s\\%s\\%s_FT1.csv", sWhite2DIDListLoc, sCust, edtSysLotID->Text, edtSysLotID->Text);                                   //RogerYang 20260225 : 修改白名單路徑 \\客戶代碼(批次前3碼)\\批次_FTx_csv
                        if(cbRunMode->Text=="FT1")                              //取得iJCETWhitelistSN作為塞入序號(PartID)用
                        {
                            if(rlt==true)
                            {
                                list2DTemp->LoadFromFile(str2);
                            }
                        }
                        else                                                    //如果不是FT1 也要下載當下runmode的名單，僅作為紀錄，不比對
                        {
                            sLocTmp.sprintf("%s%s\\%s\\%s_%s.csv", sWhite2DIDListLoc, sCust, edtSysLotID->Text, edtSysLotID->Text, cbRunMode->Text);            //RogerYang 20260225 : 修改白名單路徑 \\客戶代碼(批次前3碼)\\批次_FTx_csv
                            rlt=fBarCode->GetWhite2DIDList(edtSysLotID->Text, cbRunMode->Text);
                            if(rlt==true)
                            {
                                list2DTemp->LoadFromFile(sLocTmp);
                            }
                        }

                        if(list2DTemp->Count>1)                                 //有資料，根據_FTx.csv取得最後一筆資料的partID
                        {
                            TStringList* sRowList=new TStringList();
                            sRowList->Delimiter=',';
                            sRowList->DelimitedText=list2DTemp->Strings[list2DTemp->Count-1];
                            fBarCode->iJCETWhitelistSN=sRowList->Strings[sRowList->Count-1].ToIntDef(0);                //在最後一個欄位
                            delete sRowList;
                        }
                        else
                        {
                            fBarCode->iJCETWhitelistSN=0;
                        }
                        delete list2DTemp;
#else
                        return LI1_Refuse(whyNot, 7793, "GATE (W906-E020-LI1-5): JCET 2DID white list download needs TfBarCode::GetWhite2DIDList / bNeedCheckWhitleList / iJCETWhitelistSN (not ported) -- lot start refused");
#endif // GATE (W906-E020-LI1-5)
                    }
                }
                else if(IniConfig.iN23DownloadMethod==1)                        //JerryYang 20250320 : 2DID白名單功能 Net Drive   // :7836
                {
                    //----------------------
                    //開始下載檔案
                    //----------------------
                    do
                    {
                        MyForceDirectories(asBarCodeLogPath);
                        WIN32_FIND_DATA filedata1;                              // Structure for file data
                        HANDLE filehandle1;                                     // Handle for searching
                        AnsiString szFileName;
                        filehandle1=FindFirstFile((IniConfig.sN23_4_URL + "*.txt").c_str(), &filedata1);
                        TStringList *MyList=new TStringList();
                        MyList->Clear();
                        if(filehandle1!=INVALID_HANDLE_VALUE)
                        {
                            do
                            {
                                /* 不處理隱藏檔及 . 跟 .. */
                                if((filedata1.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                                    strcmp(filedata1.cFileName, ".")==0 ||
                                    strcmp(filedata1.cFileName, "..")==0)
                                    continue;

                                if(ExtractFileExt(filedata1.cFileName).LowerCase()==".txt")                             // 若找到的檔案的副檔名是 .txt
                                {
                                    szFileName=ChangeFileExt(ExtractFileName(filedata1.cFileName), "");                 // 取出檔名，其實就是把副檔名設成""
                                    MyList->Add(szFileName);
                                }
                            } while(FindNextFile(filehandle1, &filedata1));
                            FindClose(filehandle1);
                        }

                        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-5) -- golden :7869, see the file header.
#if 0 // GATE (W906-E020-LI1-5)
                        fBarCode->bNeedCheckWhitleList=true;
#endif // GATE (W906-E020-LI1-5)
                        LI1_Skip(skipped, "GATE (W906-E020-LI1-5): golden uLotInfo.cpp:7869 fBarCode->bNeedCheckWhitleList=true not run (not a TfBarCode member in the port)");

                        if(MyList->Count>0)
                        {
                            iIndex=-1;
                            if(SPIL_FOR_QLE==1)     //JerryYang 20260409 : QLE 白名單檔名
                            {
                                iPos=L->edtSysLotID->Text.AnsiPos("-");
                                sWhiteListLotID=L->edtSysLotID->Text.SubString(1, iPos-1);
                                sFileName.sprintf("%s_%s", sWhiteListLotID, sWhiteListProcess);
                            }
                            else if(CUSTOMER_CODE==CC_SJ_Semiconductor)              //RogerYang 20260125 : Add for SJSM LotID format
                            {
                                sWhiteListLotID=L->edtSysLotID->Text;
                                sWhiteListProcess=L->cbRunMode->Text;
                                sFileName.sprintf("%s_%s", sWhiteListLotID, sWhiteListProcess);
                                sWhiteListProcess=L->edtSysLotID->Text.SubString(iPos+1, L->edtSysLotID->Text.Length());
                            }
                            else
                            {
                                sFileName=L->edtSysLotID->Text;
                                sWhiteListLotID=L->edtSysLotID->Text;
                                sWhiteListProcess=L->edtSysLotID->Text;
                            }

                            for(int i=0; i<MyList->Count; i++)
                            {
                                if(AnsiString(MyList->Strings[i]).AnsiPos(sFileName)!=0)   // AnsiString(...): vclcompat Strings[] is a proxy without AnsiPos
                                {
                                    iIndex=i;
                                    as2DWhiteListLog=LI1_Log("\\2DBarCode\\CheckResult\\")+MyList->Strings[iIndex]+"_CheckResult_"+Now().FormatString("yyyymmddhhnnss")+".txt";   // golden "D:\\HT9045_Log\\2DBarCode\\CheckResult\\" (D-1)
                                    as2DWhiteListLogName=MyList->Strings[iIndex]+"_CheckResult_"+Now().FormatString("yyyymmddhhnnss")+".txt";
                                    if(AnsiString(MyList->Strings[i]).AnsiPos("FirstStage")!=0)
                                    {
                                        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-5) -- golden :7903.
#if 0 // GATE (W906-E020-LI1-5)
                                        fBarCode->bNeedCheckWhitleList=false;
#endif // GATE (W906-E020-LI1-5)
                                        LI1_Skip(skipped, "GATE (W906-E020-LI1-5): golden uLotInfo.cpp:7903 fBarCode->bNeedCheckWhitleList=false (FirstStage) not run");
                                    }
                                }
                            }

                            if(iIndex>=0)
                            {
                                str1.sprintf("%s%s.txt", IniConfig.sN23_4_URL, MyList->Strings[iIndex]);
                                str2.sprintf("%s\\%s.txt", asBarCodeLogPath, MyList->Strings[iIndex]);   //RogerYang 20260527 : Fix
                                delete MyList;

                                if(FileExists(str2))
                                {
                                    DeleteFile(str2);
                                }

                                ret=CopyFile(str1.c_str(), str2.c_str(), false);                                        //複製一份新的
                                ::Sleep(100);

                                if(ret==0)
                                {
                                    ret2=ShowErrorMessage("WAR1684", K_RETRY|K_SKIP, MMSystem, 0, str1);                //下載失敗，重試
                                    if(ret2==K_SKIP)
                                        break;
                                }
                                else
                                {
                                    ret2=0;
                                }
                            }
                            else
                            {
                                // golden oddity kept: MyList is not deleted on this path (:7933-7939)
                                str1.sprintf("%s Can not find file %s", IniConfig.sN23_4_URL, sFileName);
                                ret2=ShowErrorMessage("WAR1684", K_RETRY|K_SKIP, MMSystem, 0, str1);                    //下載失敗，重試
                                if(ret2==K_SKIP)
                                    break;
                            }
                        }
                        else
                        {
                            delete MyList;
                            str1.sprintf("%s Can not find any txt file", IniConfig.sN23_4_URL);
                            ret2=ShowErrorMessage("WAR1684", K_RETRY|K_SKIP, MMSystem, 0, str1);                        //下載失敗，重試
                            if(ret2==K_SKIP)
                                break;
                        }
                    }
                    while(ret2==1);
                }
                else if(IniConfig.iN23DownloadMethod==2)                        //SECS/GEM
                {
                }
                else if(IniConfig.iN23DownloadMethod==3)                        //手動   // :7955
                {
                    //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-2) -- golden :7957-7958, rgSort2DID not in the port.
#if 0 // GATE (W906-E020-LI1-2)
                    if(rgSort2DID->ItemIndex==1)                                //手動選檔案
                        str2=edSort2DIDBinFile->Text;
#endif // GATE (W906-E020-LI1-2)
                    LI1_Skip(skipped, "GATE (W906-E020-LI1-2): golden uLotInfo.cpp:7957-7958 rgSort2DID->ItemIndex==1 -> str2=edSort2DIDBinFile->Text not run (rgSort2DID widget not ported)");
                }

                if(FileExists(str2))                                            //Steven 20160505 : 加上保護, 不然開程式會跳Error   // :7961
                {
                    //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-5) -- golden :7963-7995, see the file header.
#if 0 // GATE (W906-E020-LI1-5)
                    fBarCode->list2DWhitle->Clear();

                    fBarCode->list2DWhitle->LoadFromFile(str2);
                    fBarCode->list2DWhitleResult->LoadFromFile(str2);

                    if(fBarCode->JCETUseMakeWhite2DIDList()==true)              //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能
                    {
                        for(int i=1; i<fBarCode->list2DWhitle->Count; i++)      //避開title
                        {
                            iPos=fBarCode->list2DWhitle->Strings[i].AnsiPos(",");
                            if(iPos>1)
                            {
                                sID=fBarCode->list2DWhitle->Strings[i].SubString(1, iPos-1);
                                if(sID!="2DID")
                                {
                                    fBarCode->list2DWhitle->Strings[i]=sID;
                                }
                            }
                        }
                    }
                    else
                    {
                        for(int i=0; i<fBarCode->list2DWhitle->Count; i++)
                        {
                            iPos=fBarCode->list2DWhitle->Strings[i].AnsiPos("_");
                            if(iPos>1)
                            {
                                sID=fBarCode->list2DWhitle->Strings[i].SubString(1, iPos-1);
                                fBarCode->list2DWhitle->Strings[i]=sID;
                            }
                        }
                    }
                    fBarCode->list2DWhitle->Sort();
#else
                    return LI1_Refuse(whyNot, 7963, "GATE (W906-E020-LI1-5): 2DID white list " + str2 + " found, but loading it needs TfBarCode::list2DWhitle / list2DWhitleResult (not ported) -- lot start refused");
#endif // GATE (W906-E020-LI1-5)
                }
                else
                {
                    if(CUSTOMER_CODE==CC_JCET)                                  //RogerYang 20251210 : JCET 2D FT1白名單/FT2比對功能   // :7999
                    {
                        if(JCETUseMakeWhite2DIDList()==true &&
                            L->cbRunMode->Text!="FT1" &&
                            L->cbRunMode->Text.Pos("QA")==0)                    //RogerYang 20260615 : 新增QA
                        {
                            ShowMyMessage("The 2DID sorting list is missing", "找不到2DID sorting list");
                            return LI1_Refuse(whyNot, 8006, LI1_Two("The 2DID sorting list is missing", "找不到2DID sorting list"));
                        }
                    }
                    else
                    {
                        L->btnASECL_LotStart->Down=false;                       // :8011
                        return LI1_Refuse(whyNot, 8012, "(golden returns without a message) 2DID allow list on, Run Mode " + L->cbRunMode->Text + ": no white list file " + str2);
                    }
                }
            }
        }
    }

    AnsiString Msg, MsgE;                                                       // :8019
    int len=0;
    // golden :8021-8023: TRegExpr *regex=new TRegExpr; Expression="([<>/!|:\"\*\?])"; InputString=edtSysLotID->Text -- (a) RegexMatchLen
    if(L->edtSysLotID->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->edtSysLotID->Text)!=0)                         // :8024
    {
        len=RegexMatchLen(L->edtSysLotID->Text);                                //取出第一個找到符合Expression的字串長度
        if(len!=0 || L->edtSysLotID->Text.AnsiPos("\\")!=0)
        {
            ShowMyMessage("Lot ID can not use special charater \\/:*?\"<>|", L->edtSysLotID->Text);
            L->edtSysLotID->Text="";
        }
    }

    if(L->edtSysOperatorID->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->edtSysOperatorID->Text)!=0)               // :8035
    {
        len=RegexMatchLen(L->edtSysOperatorID->Text);                           //取出第一個找到符合Expression的字串長度
        if(len!=0 || L->edtSysOperatorID->Text.AnsiPos("\\")!=0)
        {
            ShowMyMessage("OP ID can not use special charater \\/:*?\"<>|", L->edtSysOperatorID->Text);
            L->edtSysOperatorID->Text="";
        }
    }

    if(L->cbRunMode->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->cbRunMode->Text)!=0)                             // :8046
    {
        len=RegexMatchLen(L->cbRunMode->Text);                                  //取出第一個找到符合Expression的字串長度
        if(len!=0 || L->cbRunMode->Text.AnsiPos("\\")!=0)
        {
            ShowMyMessage("Run Mode can not use special charater \\/:*?\"<>|", L->cbRunMode->Text);
            L->cbRunMode->Text="";
        }
    }

    #ifndef SOFT_SIMULTE                                                        // :8056 -- compiled in the SHIP configuration only
    if(CUSTOMER_CODE==CC_SCC)                                                   //Steven 20200302 : SCC楊恩民說輸入字串5~30個字元
    {
        if(L->edtSysLotID->Text.Length()<5 || L->edtSysLotID->Text.Length()>30)
        {
            L->SetLotID("");
        }

        if(L->edtSysOperatorID->Text.Length()<5 || L->edtSysOperatorID->Text.Length()>30)
        {
            L->edtSysOperatorID->Text="";
        }
    }
    #endif

    if(IniConfig.bEnable_SECS_GEM==false && TestIF.iTestType==TCP_IP_MODE)      //Steven 20230302 : Add for OS Tester   // :8071
    {
        if(L->edtSysLotID->Text=="" || L->edtSysOperatorID->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter LotID and Operator ID!!");
            return LI1_Refuse(whyNot, 8077, "Please Enter LotID and Operator ID!!");
        }
    }

    //==> Eastsun 20260527 整合#027-2.MR.U5 BarcodeRecipe required :KYEC
    if(TestIF_File.bEnableBarCode==true &&                                      // :8082
       BAR_CODE_INSTALL==ebctUseCCDMode &&
       TestIF_File.bBarCodeMultiRecipe==true)
    {
        if(L->edtBarcodeRecipe->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter Barcode Recipe!!");
            return LI1_Refuse(whyNot, 8090, "Please Enter Barcode Recipe!!");
        }
    }
    //<== Eastsun 20260527 #027-2.MR.U5

    if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200409 : Murata 2DID比對功能   // :8095
    {
        if(L->edPage->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->edPage->Text)!=0)                               //Steven 20200818 : 增加正規表達式判斷, 避免存檔例外
        {
            len=RegexMatchLen(L->edPage->Text);                                 //取出第一個找到符合Expression的字串長度
            if(len!=0 || L->edPage->Text.AnsiPos("\\")!=0)
            {
                ShowMyMessage("Page can not use special charater \\/:*?\"<>|", L->edPage->Text);
                L->edPage->Text="";
            }
        }

        if(L->edtLine->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->edtLine->Text)!=0)                             // :8109
        {
            len=RegexMatchLen(L->edtLine->Text);                                //取出第一個找到符合Expression的字串長度
            if(len!=0 || L->edtLine->Text.AnsiPos("\\")!=0)
            {
                ShowMyMessage("Line ID can not use special charater \\/:*?\"<>|", L->edtLine->Text);
                L->edtLine->Text="";
            }
        }

        if(L->edtProcessName->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->edtProcessName->Text)!=0)               // :8120
        {
            len=RegexMatchLen(L->edtProcessName->Text);                         //取出第一個找到符合Expression的字串長度
            if(len!=0 || L->edtProcessName->Text.AnsiPos("\\")!=0)
            {
                ShowMyMessage("Process Name can not use special charater \\/:*?\"<>|", L->edtProcessName->Text);
                L->edtProcessName->Text="";
            }
        }

        if(L->edtProduct->Text.AnsiPos("\\")!=0 || RegexMatchLen(L->edtProduct->Text)!=0)                       // :8131
        {
            len=RegexMatchLen(L->edtProduct->Text);                             //取出第一個找到符合Expression的字串長度
            if(len!=0 || L->edtProduct->Text.AnsiPos("\\")!=0)
            {
                ShowMyMessage("Product Name can not use special charater \\/:*?\"<>|", L->edtProduct->Text);
                L->edtProduct->Text="";
            }
        }
        //  golden :8140 `delete regex;` -- the only delete (golden leaks the TRegExpr on every other path); no object here.

        if(L->edtLine->Text=="" || L->edtProcessName->Text=="" || L->edtProduct->Text=="")                     // :8142
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter Line and Process and Product Name!!");
            return LI1_Refuse(whyNot, 8146, "Please Enter Line and Process and Product Name!!");
        }

        if(L->edtSysLotID->Text=="" ||                                          // :8149
           L->edPage->Text=="" ||
           L->edtSysOperatorID->Text=="" ||
           L->cbRunMode->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter lot information!!");
            return LI1_Refuse(whyNot, 8156, "Please Enter lot information!!");
        }
    }

    if(CUSTOMER_CODE==CC_TSI)                                                   //frank 20200814 : 每10盤記錄一次summary log   // :8160
    {
        if(L->edtSysLotID->Text=="" ||
           L->edtSysOperatorID->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter LotID and Operator ID!!");
            return LI1_Refuse(whyNot, 8167, "Please Enter LotID and Operator ID!!");
        }

        if(RunInfo.bLotStart==false)
        {
            for(int i=0; i<10; i++)
                LastSet.TrayCount[i]=0;
        }
    }
    else if(CUSTOMER_CODE==CC_Greatek)                                          //Sam 20171101 (wei) : 超豐不用檢查 OPID and LotID   // :8176
    {
    }
    else if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                                 //KaiChen 20200618 ：矽格中興，不檢查RunMode   // :8179
    {
        if(L->edtSysLotID      ->Text==""  ||
           L->edtSysOperatorID ->Text==""  ||
           L->edCustomerLotId  ->Text==""  ||                                   //Sam 20220223 : 矽格中興廠新增 Lot 資料
           L->coStation        ->Text==""  ||                                   //Sam 20220223 : 矽格中興廠新增 Lot 資料
           L->edStationNum     ->Text=="")                                      //Sam 20220223 : 矽格中興廠新增 Lot 資料
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter lot information!!");                    // golden oddity kept: no return here (:8188)
        }
    }
    else if(IniConfig.bVTESTFunction==true)                                     // :8191
    {
        if(L->edtSysLotID->Text=="" ||
           L->edtSysOperatorID->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter LotID and Operator ID!!");
            return LI1_Refuse(whyNot, 8198, "Please Enter LotID and Operator ID!!");
        }

        if(RunInfo.bLotStart==true &&                                           // :8201
           fMesSystem->bDownloadLotInforFlag==true)
        {
            return LI1_Refuse(whyNot, 8204, "(golden returns without a message) VTEST: a lot is already started and its MES lot info is already downloaded (RunInfo.bLotStart && fMesSystem->bDownloadLotInforFlag)");
        }

        if(CUSTOMER_CODE==CC_VTEST_Shanghai)                                    // :8207
        {
            if(L->edtSysOperatorID->Text.Length()<4)
            {
                L->sbSECSLotEnd->Down=true;
                ShowMyMessage("OP ID 輸入小於4個字","OP ID Length less than 4");
                return LI1_Refuse(whyNot, 8213, LI1_Two("OP ID 輸入小於4個字", "OP ID Length less than 4"));
            }

            //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-7) -- golden :8216-8222, see the file header.
#if 0 // GATE (W906-E020-LI1-7)
            if(fMesSystem->RunModeRW(true, edtSysLotID->Text, cbRunMode->Text)==1)                                      //jou 20210823 : lot start增加保護避免重複執行run mode
            {
                Msg.printf("Lot: %s run mode %s 重複執行", edtSysLotID->Text, cbRunMode->Text);
                MsgE.printf("Lot: %s run mode %s Repeat execution", edtSysLotID->Text, cbRunMode->Text);
                ShowMyMessage(Msg, MsgE);
                return ;
            }
#else
            return LI1_Refuse(whyNot, 8216, "GATE (W906-E020-LI1-7): VTEST_Shanghai repeat-run check needs fMesSystem->RunModeRW (forms/fMesSystem.h GATE W-09, not defined) -- lot start refused");
#endif // GATE (W906-E020-LI1-7)
        }
    }
    else if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS ||                            //Steven 20230213 : For SJSemi OS Tester   // :8225
            CUSTOMER_CODE==CC_XINITECH)
    {
        if(IniConfig.bEnable_SECS_GEM==false && TestIF.iTestType==TCP_IP_MODE)  //Steven 20230213 : For SJSemi OS Tester
        {
            if(L->edtSysLotID->Text=="" ||
               L->edtSysOperatorID->Text=="")
            {
                L->sbSECSLotEnd->Down=true;
                ShowMyMessage("Please Enter LotID and Operator ID!!");
                return LI1_Refuse(whyNot, 8235, "Please Enter LotID and Operator ID!!");
            }
        }

        if(USE_RFID_READER && L->pnlLoader->Caption=="")                        // :8239
        {
            ShowMyMessage("Please Enter Tray ID!!");
            return LI1_Refuse(whyNot, 8242, "Please Enter Tray ID!!");
        }
    }
    else if((IniConfig.bSPILFunction==true ||                                   // :8245
             CUSTOMER_CODE==CC_SJ_Semiconductor) &&
            CosFunction.bSortingBy2DList==true &&
            LastSet.iTester==_2D_SORT &&
            TestIF_File.bSortingBy2DIDList==true)
    {
        if(L->edtSysLotID->Text=="" ||
           L->edtSysOperatorID->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter LotID and Operator ID!!");
            return LI1_Refuse(whyNot, 8256, "Please Enter LotID and Operator ID!!");
        }
        else if(L->cbRunMode->Text=="" ||
                L->cbRunMode->ItemIndex==-1)                                    //JerryYang 20230322 : add 2D mapping result
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please select the run mode in lot info!!");
            return LI1_Refuse(whyNot, 8263, "Please select the run mode in lot info!!");
        }
    }
    else if(CUSTOMER_CODE==CC_LEADYO)                                           //KenHsieh 20230406 : 新增OCR Data + Bin Log功能   // :8266
    {
        if(IniConfig.bN33_1_NetChangeFileAndData)                               //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
        {
            //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-6) -- golden :8270-8321, see the file header.
#if 0 // GATE (W906-E020-LI1-6)
            TStringList *ProductionInfo=new TStringList;
            AnsiString asProdPath="D:\\HT9045_Log\\Production_Info";
            AnsiString S="", asPath="", asSetupFile="", asLotID="", asFileName="";

            MyForceDirectories(asProdPath);

            if(bCheckOnlyOneFileAndData()==false)
            {
                sbSECSLotEnd->Down=true;
                return;
            }

            asPath=asProdPath+"\\TestInfo.txt";
            ProductionInfo->Clear();
            ProductionInfo->LoadFromFile(asPath);

            if(LastSet.iRunStartMode==rsmInitialStart)
            {
                asSetupFile=ProductionInfo->Strings[21];
                asSetupFile=asSetupFile.SubString(asSetupFile.Pos(":")+1, asSetupFile.Length()-asSetupFile.Pos(":"));   //冒號後是檔名
                if(asSetupFile=="")
                {
                    sbSECSLotEnd->Down=true;
                    ShowMyMessage("Setup File name is NULL, Please check TestInfo.txt");
                    ProductionInfo->Clear();
                    delete ProductionInfo;
                    return;
                }

                S.sprintf("D:\\HT9045\\IniData\\Data\\%s", asSetupFile);        //檢查工作檔是否存在
                if(DirectoryExists(S)==false)
                {
                    sbSECSLotEnd->Down=true;
                    ShowMyMessage("Setup File is not exist, Please check TestInfo.txt");
                    ProductionInfo->Clear();
                    delete ProductionInfo;
                    return;
                }

                if(fMain->cbSetupFileName->Text!=asSetupFile)
                {
                    fMain->cbSetupFileName->Text=asSetupFile;
                    fMain->cbSetupFileNameChange(fMain);
                }
            }

            asLotID=ProductionInfo->Strings[0];
            asLotID=asLotID.SubString(asLotID.Pos(":")+1, asLotID.Length()-asLotID.Pos(":"));
            edtSysLotID->Text=asLotID;

            ProductionInfo->Clear();
            delete ProductionInfo;
#else
            return LI1_Refuse(whyNot, 8276, "GATE (W906-E020-LI1-6): LEADYO [N33_1] needs golden bCheckOnlyOneFileAndData (uLotInfo.cpp:14490) and a real recipe switch (fMain->cbSetupFileNameChange is a stub here) -- lot start refused");
#endif // GATE (W906-E020-LI1-6)
        }

        if(L->edtSysLotID->Text=="")                                            // :8324
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter LotID!!");
            return LI1_Refuse(whyNot, 8328, "Please Enter LotID!!");
        }
    }
    else if(CUSTOMER_CODE==CC_JCET)                                             //RogerYang 20251215 : JCET 2D FT1白名單/FT2比對功能   // :8331
    {
        if(JCETUseMakeWhite2DIDList() &&
            L->edtCusLotID->Text=="")
        {
            L->sbSECSLotEnd->Down=false;                                        // golden oddity kept: false here, true in every other branch
            ShowMyMessage("Please Enter Cust. Lot ID!!");
            return LI1_Refuse(whyNot, 8338, "Please Enter Cust. Lot ID!!");
        }

        if(L->edtSysLotID->Text=="" ||
                L->edtSysOperatorID->Text=="")
        {
            L->sbSECSLotEnd->Down=false;
            ShowMyMessage("Please Enter LotID and Operator ID!!");
            return LI1_Refuse(whyNot, 8346, "Please Enter LotID and Operator ID!!");
        }
    }
    else                                                                        // :8349
    {
        if(IniConfig.bSPILFunction==true && TestIF_File.b2DIDAllowList &&       //JerryYang 20250320 : 2DID白名單功能
           IniConfig.iN23DownloadMethod!=2)                                     //JerryYang 20241104 : 支援2DID白名單功能
        {
            L->btClearBarcodeListClick();                                       //JerryYang 20250327 : add   // golden btClearBarcodeList->Click() (:8354), the port's seat (forms/fLotInfo.cpp:6544)
            if(L->edtSysLotID->Text=="")
            {
                ShowMyMessage("Please Enter LotID!!");
                return LI1_Refuse(whyNot, 8358, "Please Enter LotID!!");
            }

            if(fLotInfo->cbRunMode->Text=="")                                   //JerryYang 20250521 : add
            {
                ShowMyMessage("Please Enter Run Mode!!");
                return LI1_Refuse(whyNot, 8364, "Please Enter Run Mode!!");
            }
        }
        else if(CUSTOMER_CODE==CC_XINYUN)                                       //RogerYang 20260610 : XINYUN SECS開批只卡LotID   // :8367
        {
            if(L->edtSysLotID->Text=="")
            {
                L->sbSECSLotEnd->Down=true;
                ShowMyMessage("Please Enter LotID!!");
                return LI1_Refuse(whyNot, 8373, "Please Enter LotID!!");
            }
        }
        else if(L->edtSysLotID->Text=="" ||                                     // :8376
                L->edtSysOperatorID->Text=="")
        {
            L->sbSECSLotEnd->Down=true;
            ShowMyMessage("Please Enter LotID and Operator ID!!");
            return LI1_Refuse(whyNot, 8381, "Please Enter LotID and Operator ID!!");
        }
        else if(L->cbRunMode->Text=="" && CUSTOMER_CODE!=CC_SIGURD_ChungXing && L->cbRunMode->Visible)                  //Sam 20210623 : 修正 Bug  //KaiChen 20200618 ：矽格中興，不檢查RunMode  //Steven 20200306 : SCC要求按下Lot End的時候, 資料要清空   // :8383
        {
            if(IniConfig.bSPILFunction==true)                                   //JerryYang 20220923 : 20220927 : 矽品不檢查run mode
            {
            }
            else
            {
                L->sbSECSLotEnd->Down=true;
                ShowMyMessage("Please select run mode!!");
                return LI1_Refuse(whyNot, 8392, "Please select run mode!!");
            }
        }
    }

    //AI(W906-E030A) 20261003 [W906] (St01): V912 uLotInfo.cpp:8397 (the fBarCode->ResetBarcodeCSVForLotStart call, Ifor 20260625, Barcode CSV Compare log / cache reset) is 912-only -- golden 906 has no Barcode CSV Compare at all (0 hits in D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003]), so per Jimmy RULINGS_20261002 #20 / #23 item 6 the line, its gate and its LI1-8 skip are gone (906 uLotInfo.cpp:8272 -> :8274 goes straight from the run-mode check to the VTEST block).

    if(IniConfig.bVTESTFunction==true)                                          //jou 20200409 : VTest Mes system   // :8398
    {
        if((IniConfig.bEnableRms || IniConfig.bEnableFTP) && IniConfig.bCheckFile)
        {
            if(fMesSystem->CheckVTENGmode(L->edtSysLotID->Text)==false)         // :8402
            {
                //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-9) -- golden :8404-8443, see the file header.
#if 0 // GATE (W906-E020-LI1-9)
                fMesSystem->LabeledEditLotNo->Text=edtSysLotID->Text;
                fMesSystem->LabeledEditOPID->Text=edtSysOperatorID->Text;
                fMesSystem->buttonDownloadLotInfor->Click();

//                #ifdef BETA_VTestSummaryFile                                  //RogerYang 20250809 偉測Summary文件修改
                TStringList *Strlist = new TStringList();
                Strlist->Delimiter = ',';
                if(edtProcessName->Text=="")                                    //RogerYang 20250809 Summary文件產出必須要有資料，否則不允許開批
                    Strlist->Add("產品型號(CustPart)");
                if(edtProduct->Text=="")
                    Strlist->Add("客戶批次(CustLotNum)");
                if(sVTestInternalLot=="")
                    Strlist->Add("廠內批次(LotNum)");
                if(cbProcess->Text=="")
                    Strlist->Add("測試站點(WipStep)");
                if(cbTestTimes->Text=="")
                    Strlist->Add("測試次數");

                if(Strlist->Count>0)
                {
                    str5=Strlist->CommaText+" 資料不可為空，請檢察MES(MesFileLog)下載資料是否正確!!";
                    ShowMyMessage(str5);
                    Strlist->Clear();
                    delete Strlist;
                    return ;
                }
                Strlist->Clear();
                delete Strlist;
//                #endif

                if(fMesSystem->bDownloadLotInforFlag==false)
                {
                    edtSysLotID->Enabled=true;
                    edtSysOperatorID->Enabled=true;
                    cbProcess->Enabled=true;
                    cbTestTimes->Enabled=true;                                  //RogerYang 20250809 偉測Summary文件修改
                    edtProcessName->Enabled=true;
                    edtProduct->Enabled=true;
                    return;
                }
#else
                return LI1_Refuse(whyNot, 8404, "GATE (W906-E020-LI1-9): VTEST lot start needs the MES lot-info download (fMesSystem->buttonDownloadLotInfor, forms/fMesSystem.h GATE W-02) -- lot start refused");
#endif // GATE (W906-E020-LI1-9)
            }
        }
        else
        {
            bNoRTBinFixFlag[0]=TrayForm.asNoRTBinFix[0]==""?false:true;         //RogerYang 20250814 偉測不可複測bin功能   // :8448
            bNoRTBinFixFlag[1]=TrayForm.asNoRTBinFix[1]==""?false:true;         //如果開批不用下載，這裡直接看TrayForm設定決定要不要做
            bNoRTBinFixFlag[2]=TrayForm.asNoRTBinFix[2]==""?false:true;         //bNoRTBinFixFlag - 該Fix是否啟用不可複測Bin
        }
        fMesSystem->bNoRTBinFlag[0]=TrayForm.asNoRTBinFix[0]==""?true:false;    //RogerYang 20250604 偉測不可複測bin功能   // :8452
        fMesSystem->bNoRTBinFlag[1]=TrayForm.asNoRTBinFix[1]==""?true:false;    //bNoRTBinFlag - 判斷當下是否已經掃過條碼，空值代表不需要掃碼輸入
        fMesSystem->bNoRTBinFlag[2]=TrayForm.asNoRTBinFix[2]==""?true:false;
    }

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能   // :8457
    {                                                                           //Sam 20170719 (Steven) 移植超豐 OEE 功能 form HT-7045
        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-10) -- golden :8459-8514, see the file header.
#if 0 // GATE (W906-E020-LI1-10)
        if(IniConfig.bC11UseMonitorView && fMonitor->MVCtrl->IsConnect()==false)                                        //Sam 20170925 : 檢查攝影機有沒有連線  //JimmyChiu 20220125 優先檢測功能是否開啟
        {
            ShowMyMessage("Please Check Monitor Connect!!!");
            return;
        }

        if(Application->MessageBox("Are You Sure Start Lot?", "Start Lot?", MB_YESNO)!=IDYES)
        {
            return;
        }

        //AI(ht9045-v912) 20260911: 用既有的 pasErrorMsg 接住內層原因；原本這裡只印 Please Check MO or Machine ID，把真正的原因丟掉
        AnsiString asInnerReason="";
        //AI(ht9045-v912) 20260914: CASE-20260914-001 開批途中的未攔截例外會把整條流程掀掉，
        //而 TfMain::AppException 只寫 EventLog 不顯示任何東西，操作員按下去像沒反應。
        //在此攔住並走既有的失敗訊息路徑，控制流與原本失敗相同(一樣 return、不繼續開批)。
        //只包在 bOEEFunction 區塊內(全樹僅 FUNC_CC_Greatek 設 true)，不影響其他客戶與 SECS S2F41 路徑。
        bool bOEEStartLotOK=false;
        bool bStartLotException=false;
        try
        {
            bOEEStartLotOK=fProductionInfo->OEE_StartLot(false, &asInnerReason); //Steven 20250520 : QQQ
        }
        catch(Exception &e)
        {
            bOEEStartLotOK=false;
            bStartLotException=true;
            if(asInnerReason!="")
                asInnerReason=asInnerReason+" / ";
            asInnerReason=asInnerReason+"Exception : "+e.Message;
        }
        catch(...)
        {
            bOEEStartLotOK=false;
            bStartLotException=true;
            if(asInnerReason!="")
                asInnerReason=asInnerReason+" / ";
            asInnerReason=asInnerReason+"Exception : Unknown exception";
        }
        if(bOEEStartLotOK==false)
        {
            lb_PIOEELotStatus->Caption="Production Start Lot Fail!";
            AnsiString sMsg="Production Start Lot Fail!";
            if(asInnerReason=="")
                sMsg="Production Start Lot Fail!#Please Check MO or Machine ID";
            //AI(ht9045-v912) 20260914: CASE-20260914-001 例外中斷給可查代號與白話說明；
            //S3 一律以原始英文字面開頭，既有的 EventLog 檢索不會失效
            if(bStartLotException==true)
                ShowMyMessage("【LOT-01】Start Lot 被程式異常中斷",
                              "開批流程跑到一半發生程式異常而停止，本批沒有開成功。"
                              "請保留畫面、按 State Record 收快照並通知工程師；異常內容已記在事件記錄。",
                              "Production Start Lot Fail! LOT-01 exception : "+asInnerReason);
            else
                ShowMyMessage(sMsg, asInnerReason, "Production Start Lot Fail! Please Check MO or Machine ID : "+asInnerReason);
            return;
        }
#else
        return LI1_Refuse(whyNot, 8459, "GATE (W906-E020-LI1-10): OEE lot start needs fMonitor->MVCtrl, the Yes/No box and fProductionInfo->OEE_StartLot (not ported) -- lot start refused");
#endif // GATE (W906-E020-LI1-10)
    }

    if(CUSTOMER_CODE==CC_Murata)                                                //Steven 20200624 : Lot ID讀到DayDate, 自動轉成YYMMDD   // :8517
    {
        AnsiString Date, Year;
        Year.sprintf("%04d", SystemYear);
        Year=Year.SubString(3, 2);
        Date.sprintf("%s%02d%02d", Year, SystemMonth, SystemDate);
        if(L->edtSysLotID->Text.AnsiPos("DAYDATE")!=0)
        {
            L->edtSysLotID->Text=StringReplace(L->edtSysLotID->Text, "DAYDATE", Date, TReplaceFlags()<<rfReplaceAll);
        }
    }
    else if(CUSTOMER_CODE==CC_PANTHER)                                          // :8528
    {
        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-11) -- golden :8530-8532, see the file header.
#if 0 // GATE (W906-E020-LI1-11)
        fMain->patFunc->SetStartLotTime(dtStartLot);
        UpdateLotInfoPAT();
        fMain->machineTime.StartLot();                                          //Jimmychiu 20250916 : 新增機台運作狀態紀錄
#endif // GATE (W906-E020-LI1-11)
        LI1_Skip(skipped, "GATE (W906-E020-LI1-11): golden uLotInfo.cpp:8530-8532 PANTHER fMain->patFunc->SetStartLotTime / UpdateLotInfoPAT / fMain->machineTime.StartLot not run (not ported)");
        for(int i=0; i<8; i++)
            LastSet.SystemAccSecond[0][i]=0;
    }
    L->SetLotID(L->edtSysLotID->Text, false);                                   // :8536

//    if(IniConfig.bVTESTFunction)                                                //AI(ht9045-config) 20260726 (RogerYang) : 診斷:開批四閘門值
//    {   ... golden :8538-8554, commented out in golden (diagnostic log + the older O12-by-recipe gate) ...
//    }
    if(IniConfig.bVTESTFunction &&                                              //AI(ht9045-config) 20260726 (RogerYang) : 開批後alarm顯示(取消診斷log)   // :8555
        CosFunction.bUseHeadContactCount &&
        IniConfig.bLifeTimeCount[0])
    {
        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-12) -- golden :8559-8562, see the file header.
#if 0 // GATE (W906-E020-LI1-12)
        if(IniConfig.bEnable_SECS_GEM)
            bVTESTKitCheckPending = true;
        else
            DoVTESTChangeKitCheck();
#endif // GATE (W906-E020-LI1-12)
        LI1_Skip(skipped, IniConfig.bEnable_SECS_GEM
                 ? "GATE (W906-E020-LI1-12): golden uLotInfo.cpp:8560 bVTESTKitCheckPending=true not run (member not ported)"
                 : "GATE (W906-E020-LI1-12): golden uLotInfo.cpp:8562 DoVTESTChangeKitCheck() not run (not ported)");
    }

    L->SetLotStart("TfLotInfo::sbSECSLotStartClick");                           //Steven 20250515 : 整合Open Short測試報表   // :8565 golden SetLotStart(__FUNC__) (D-2)

    if(CUSTOMER_CODE==CC_TSMC_TAINAN && IniConfig.bEnable_SECS_GEM==true)       //wei 20160517 TSMC lot卡關   // :8567
    {
        ShowMyMessage("Please pressed Start , RUN !!");
    }

    if(CosFunction.bFirstTrayCheckOnUnloader==true &&                           //Jimmychiu 20251205 : First Tray Check On Unloader   // :8572
       IniConfig.bP62FirstTrayCheckOnUnloader==true &&
       IniConfig.bP62AlwaysEnabledAtLotStart==true)
    {
        L->SetFirstTrayCheckOnUnloader();
    }

    if(IniConfig.bAMDFunction && L->edtSysLotID->Enabled && iAMD_Function==2)   //Ifor 20231220 add 避免點一次記錄一次   // :8579
    {
        //AI(W906-E020-LI1) 20261002 [W906] (St01): GATE (W906-E020-LI1-13) -- golden :8581, see the file header.
#if 0 // GATE (W906-E020-LI1-13)
        FormHS->RecordParameter_TFAMDLog();
#endif // GATE (W906-E020-LI1-13)
        LI1_Skip(skipped, "GATE (W906-E020-LI1-13): golden uLotInfo.cpp:8581 FormHS->RecordParameter_TFAMDLog not run (no TFormHS instance in the port)");
    }
    (void)Msg; (void)MsgE; (void)len;                                           //AI(W906-E020-LI1) 20261002 [W906]: Msg / MsgE only in the LI1-7 gate; len is golden's write-only local
    return true;                                                                // :8583
}

}  // namespace

// ---- public entry (the port's report around golden; D-3) ----------------------------------------------------------------
bool W906_LotInfo_SECSLotStart(std::string* whyNot, std::vector<std::string>* skipped)
{
    if (whyNot) whyNot->clear();
    if (skipped) skipped->clear();
    if (fLotInfo == 0) {
        if (whyNot) *whyNot = "fLotInfo is null -- lot start is not armed in this binary";
        return false;
    }
    std::printf("[LotStart] golden TfLotInfo::sbSECSLotStartClick (V912 uLotInfo.cpp:7501-8583): LotID=\"%s\" OperatorID=\"%s\" RunMode=\"%s\"\n",
                fLotInfo->edtSysLotID->Text.c_str(), fLotInfo->edtSysOperatorID->Text.c_str(), fLotInfo->cbRunMode->Text.c_str());
    bool ran = false;
    try {
        ran = SbSECSLotStartClick(fLotInfo, whyNot, skipped);
    } catch (...) {
        ran = false;
        if (whyNot) *whyNot = "exception in golden TfLotInfo::sbSECSLotStartClick";
        std::printf("[LotStart] exception in golden TfLotInfo::sbSECSLotStartClick\n");
    }
    std::printf("[LotStart] %s (RunInfo.bLotStart=%d, RunInfo.LotNo=\"%s\")\n", ran ? "ran to the end (SetLotStart called)" : "returned early",
                (int)RunInfo.bLotStart, RunInfo.LotNo.c_str());
    return ran;
}
