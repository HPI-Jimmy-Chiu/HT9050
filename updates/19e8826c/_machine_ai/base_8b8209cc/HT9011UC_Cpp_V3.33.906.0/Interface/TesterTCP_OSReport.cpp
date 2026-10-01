// =============================================================================
//  Interface/TesterTCP_OSReport.cpp  --  see the .h for scope and WHY this lives in ht9045_sm.
//
//  AI(W906-G023) 20261001 (St02-E).  Golden 906_0625_Steven Interface/TesterTCP.cpp:699-1056, read
//  with cp950.  Bodies are golden text; every port-only change is marked //AI(W906-G023):
//    P1  the golden literal "D:\\HT9045_Log" -> as9045LogPath (common.cpp:240, golden common.cpp:48
//        = "D:\\HT9045_Log"; W906_HT9045LOG_ROOT is the ctest seam, unset = the golden bytes).
//        Precedent: SortingBinTray.cpp:2702 / :2806 (W906-FLOW-2), common.cpp:244 / :295.
//    P2  golden's form widgets SourceMemo (TMemo, dfm :2846), RichEdit[] (TMemo, ctor :76) and
//        redtSummary (TRichEdit, dfm :2877) are TU-local stand-ins below; SummaryHead is the real
//        facade member fTesterTCP->SummaryHead (forms/fTesterTCP.h:361, allocated by the active ctor).
//    P3  redtSummary->Lines->SaveToFile writes RTF, like golden: dfm :2877 sets no PlainText, so VCL
//        TRichEditStrings streams SF_RTF.  Shape copied from a BCB6 output on this machine
//        (D:\HT9045_Log\OS_Summary\202510\OS_Summary_11111111111_2025_10_28_17_05_37.TXT, 3290 B).
//    P4  StringsProxy -> AnsiString(...) where golden passes Strings[i] to a varargs sprintf
//        (golden :885-887).  Without it the proxy object goes through '...' and %s prints its bytes
//        (g++ -Wconditionally-supported confirms; precedent TesterComm/Gpib/GpibCommands.cpp:94).
//    P5  the one Big5 literal (golden :1027) is written as \x bytes so the file gets golden's cp950
//        bytes, not UTF-8 (this source file is UTF-8).
//    P6  W58 Q5 (Steven 20260929): the B05 network-share copy is skipped in SIM when the path is a share
//        (W906_SimNetPathBlocked, common.cpp end of file); SHIP = golden.
// =============================================================================
#include "Interface/TesterTCP_OSReport.h"

#include <fstream>

#include "MachineDefine.h"
#include "MachineType.h"        // eTrayCount, tNotUse, ChangeToPercentage
#include "cprod.h"              // TestIF_File, TestIF, Prod, RunInfo (RUN_INFO::SaveJamRateByLot), IniConfig, CheckFileExist
#include "cmydef.h"             // TCP_IP_MODE, s6TrayName, iOneTrayPickCount, iTestBinCount, SystemYear..SystemSec
#include "common.h"             // MyForceDirectories, as9045LogPath, W906_SimNetPathBlocked
#include "aHotPlateSubstrate.h" // OutArmSuck, TestSocket (mykitsuck.h, TMyKitSuck::PordRec / iShtRow / iMaxRow ...)
#include "cSocket.h"            // TastCategory (TEST_CATEGORY)
#include "FormsFacade.h"        // fLotInfo (lbledtCustomer, edtSysOperatorID), fMain (cbSetupFileName)
#include "forms/fTesterTCP.h"   // fTesterTCP->SummaryHead

namespace {

// AI(W906-G023) P3: golden TRichEdit's Lines.  SaveToFile hides TStringList::SaveToFile (non-virtual, so the
// static type decides), the way VCL's TRichEditStrings replaces TStrings::SaveToFile.
class TW906RichEditLines : public TStringList {
public:
    void SaveToFile(const AnsiString& path) const;
};
class TW906RichEdit {
public:
    TW906RichEditLines *Lines;
    TW906RichEdit() : Lines(new TW906RichEditLines()) {}
};

// \ { } escaped; a byte >= 0x80 as \'hh (lower-case hex).  ASCII-only text matches the BCB6 sample byte
// for byte; non-ASCII (a Chinese Customer / Program name) is NOT verified against RichEdit -- HUMAN_REVIEW.
std::string W906RtfText(const AnsiString& s)
{
    static const char hex[] = "0123456789abcdef";
    std::string out;
    const std::string& in = s.str();
    for (size_t i = 0; i < in.size(); ++i) {
        const unsigned char c = (unsigned char)in[i];
        if (c == '\\' || c == '{' || c == '}') { out += '\\'; out += (char)c; }
        else if (c >= 0x80) { out += "\\'"; out += hex[c >> 4]; out += hex[c & 15]; }
        else out += (char)c;
    }
    return out;
}

void TW906RichEditLines::SaveToFile(const AnsiString& path) const
{
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) return;
    out << "{\\rtf1\\ansi\\ansicpg950\\deff0\\deflang1033\\deflangfe1028{\\fonttbl{\\f0\\fnil\\fcharset136 Courier New;}}\r\n";
    out << "\\viewkind4\\uc1\\pard\\lang1028\\f0\\fs16 ";
    for (int i = 0; i < Count; ++i) {
        if (i > 0) out << "\\par ";
        out << W906RtfText(GetString(i)) << "\r\n";
    }
    out << "\\par \r\n\\par }\r\n";
    out.put('\0');                       // the BCB6 sample ends with one NUL after "\par }\r\n"
}

// AI(W906-G023) P2: golden TfTesterTCP widgets (golden TesterTCP.h:199 SourceMemo, :201 redtSummary,
// :244 RichEdit[eTrayCount]).  Allocated on first use, not at static init.
TMemo         *SourceMemo = 0;
TW906RichEdit *redtSummary = 0;
TMemo         *RichEdit[eTrayCount] = {};

void W906OSReportWidgets()
{
    if (SourceMemo != 0) return;
    SourceMemo  = new TMemo();
    redtSummary = new TW906RichEdit();
    for (int i = 0; i < eTrayCount; i++) RichEdit[i] = new TMemo();
}

} // namespace

//------------------------------------------------------------------------------
// golden TfTesterTCP::PlaceOSTestResultToTray, Interface/TesterTCP.cpp:699-739
//------------------------------------------------------------------------------
void TesterTCP_PlaceOSTestResultToTray(int iSuckRow, int iSuckCol, int iTrayRow, int iTrayCol, int iAuto)
{
    if(TestIF_File.iTestType!=TCP_IP_MODE)
        return;
    W906OSReportWidgets();                                                      //AI(W906-G023) P2

    AnsiString sSourceFileName, sTargetFileName, Str1, Str2;
    int iTotalCh        =TestSocket.iShtRow*TestSocket.iShtCol;
    int iTesterCh       =OutArmSuck.PordRec[iSuckRow][iSuckCol].GetSiteNo();
    int iContactIndex   =OutArmSuck.PordRec[iSuckRow][iSuckCol].GetOrderOfContact();

    SourceMemo->Lines->Clear();
    RichEdit[iAuto]->Lines->Clear();
    sTargetFileName.sprintf("%s\\OS_TestReport\\%s_TestReport.TXT", as9045LogPath, s6TrayName[iAuto]);            //AI(W906-G023) P1: golden "D:\\HT9045_Log\\OS_TestReport\\%s_TestReport.TXT"
    sSourceFileName.sprintf("%s\\OSTestResult\\Device%06d_%02d.TXT", as9045LogPath, iContactIndex, iTesterCh-1);  //AI(W906-G023) P1: golden "D:\\HT9045_Log\\OSTestResult\\Device%06d_%02d.TXT"
    MyForceDirectories(as9045LogPath+"\\OS_TestReport", "ProcessOSPrint");    //AI(W906-G023) P1: golden "D:\\HT9045_Log\\OS_TestReport"

    if(CheckFileExist(sSourceFileName)==true)
    {
        SourceMemo->Lines->LoadFromFile(sSourceFileName);
        DeleteFile(sSourceFileName);
    }
    else
    {
        Str1.sprintf("can not find file %s", sSourceFileName);
        SourceMemo->Lines->Add(Str1);
    }

    if(CheckFileExist(sTargetFileName)==true)
        RichEdit[iAuto]->Lines->LoadFromFile(sTargetFileName);

    //RogerYang 20260210 : change the test seq value to Tested from Device, and the Device shown the serier number on a tray.
    Str1.sprintf("===========================    Device:%d  X:%d  Y:%d  Tested:%d    ====================================",
        iOneTrayPickCount[1+iAuto], iTrayRow+1, iTrayCol+1, (iTotalCh*iContactIndex+iTesterCh));

    RichEdit[iAuto]->Lines->Add(Str1);
    for(int i=0; i<SourceMemo->Lines->Count; i++)
    {
        RichEdit[iAuto]->Lines->Add(SourceMemo->Lines->Strings[i]);
    }
    RichEdit[iAuto]->Lines->SaveToFile(sTargetFileName);
}

//------------------------------------------------------------------------------
// golden TfTesterTCP::ProcessOSPrint, Interface/TesterTCP.cpp:741-962
//------------------------------------------------------------------------------
void TesterTCP_ProcessOSPrint(bool bViewOnly)                                   //Steven 20250515 : 整合Open Short測試報表
{
    W906OSReportWidgets();                                                      //AI(W906-G023) P2
    TStringList *SummaryHead=fTesterTCP->SummaryHead;                           //AI(W906-G023) P2: golden member TfTesterTCP::SummaryHead (TesterTCP.h:246)

    int iSiteCh=0;
    int iTotalCh=TestSocket.iShtRow*TestSocket.iShtCol;
    int iSiteTotalCt=0;
    int iSitePassCt=0;
    int iSiteFailCt=0;
    AnsiString sPassYield;
    AnsiString sFailYield;
    AnsiString asFileName, asFolderName, TargetFile, SourceFile, ServerFile;
    AnsiString sTemp, str, s, sTotal, sTempT, sTempP, sTempF, sP, sF;
    redtSummary->Lines->Clear();
    SummaryHead->Clear();
    TastCategory.UpdataCount(false);                                            //Steven 20250514 : 統一計算數量

    asFolderName.sprintf("%s\\OS_Summary\\%04d%02d", as9045LogPath, SystemYear, SystemMonth);   //AI(W906-G023) P1: golden "D:\\HT9045_Log\\OS_Summary\\%04d%02d"
    MyForceDirectories(asFolderName, "ProcessOSPrint");

    ServerFile.sprintf("%s\\OS_Summary_%s_%04d_%02d_%02d_%02d_%02d_%02d.TXT",
                                                    IniConfig.sB05_OSReportPath,
                                                    RunInfo.LotNo,
                                                    SystemYear, SystemMonth, SystemDate,
                                                    SystemHour, SystemMin, SystemSec);

    asFileName.sprintf("%s\\OS_Summary_%s_%04d_%02d_%02d_%02d_%02d_%02d.TXT",
                                                    asFolderName,
                                                    RunInfo.LotNo,
                                                    SystemYear, SystemMonth, SystemDate,
                                                    SystemHour, SystemMin, SystemSec);

    if(RunInfo.LotStartTime=="2020-01-01 00:00:00")
        sPassYield.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    else
        sPassYield=RunInfo.LotStartTime;

    if(RunInfo.LotEndTime=="")
        sFailYield.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    else
        sFailYield=RunInfo.LotEndTime;

    if(fLotInfo->lbledtCustomer->Text=="")
        sTemp=" ";
    else
        sTemp=fLotInfo->lbledtCustomer->Text;

    if(fLotInfo->edtSysOperatorID->Text=="")
        sTotal=" ";
    else
        sTotal=fLotInfo->edtSysOperatorID->Text;

    SummaryHead->Add(str.sprintf("============================ SUMMARY REPORT ============================"));
//    SummaryHead->Add(str.sprintf(" "                                     ));                              //RogerYang 20260530 : 田揚志要求格式修改
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Lot#:"),       RunInfo.LotNo));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Start:"),      sPassYield));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("End:"),        sFailYield));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Customer:"),   sTemp));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Program:"),    fMain->cbSetupFileName->Text));
//    SummaryHead->Add(str.sprintf("%-34s ",    AnsiString("LoadBoard ID:")));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Operator ID:"),sTotal));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Machine ID:"), IniConfig.SocketHandlerID));
    SummaryHead->Add(str.sprintf("%-34s %d",  AnsiString("Input:"),      TastCategory.iTotalSocket));
    SummaryHead->Add(str.sprintf("%-34s %d",  AnsiString("Pass:"),       TastCategory.iPassSocket));
    SummaryHead->Add(str.sprintf("%-34s %d",  AnsiString("Fail:"),       TastCategory.iFailSocket));
//    SummaryHead->Add(str.sprintf("%-34s %d",  AnsiString("Reject:"),     TastCategory.iRejectCount));     //RogerYang 20260530 : 田揚志要求格式修改
    SummaryHead->Add(str.sprintf("%-34s %d",  AnsiString("Open:"),       TastCategory.iUnloadCnt[1]));
    SummaryHead->Add(str.sprintf("%-34s %d",  AnsiString("Short:"),      TastCategory.iUnloadCnt[2]));
    SummaryHead->Add(str.sprintf("%-34s %s",  AnsiString("Yield:"),      ChangeToPercentage(TastCategory.iPassSocket, TastCategory.iTotalSocket)));
//    SummaryHead->Add(str.sprintf(" "                                     ));                              //RogerYang 20260530 : 田揚志要求格式修改

    for(int i=0; i<SummaryHead->Count; i++)
        redtSummary->Lines->Add(SummaryHead->Strings[i]);

    redtSummary->Lines->Add(str.sprintf("============================= BY TRAY COUNT ============================"));
    redtSummary->Lines->Add(str.sprintf(" "                                     ));

    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            sTemp.sprintf("%-34s %d", s6TrayName[i]+AnsiString(":"), TastCategory.iUnloadCnt[i]);
            redtSummary->Lines->Add(sTemp);
        }
    }

    redtSummary->Lines->Add(str.sprintf(" "                                     ));
    redtSummary->Lines->Add(str.sprintf("============================= BY SITE COUNT ============================"));
    redtSummary->Lines->Add(str.sprintf(" "                                     ));

    str.sprintf("%-34s", "Total Tested DUT Count:");                            //Total Tested DUT Count: DUT1 DUT2 DUT3 DUT4 SUM
    for(int i=0; i<iTotalCh; i++)
    {
        s.sprintf("DUT%d", i+1);
        sTemp.sprintf(" %-20s", s);
        str+=sTemp;
    }
    sTemp.sprintf(" %-20s", "SUM");
    str+=sTemp;
    redtSummary->Lines->Add(str);

    TStringList *sListTotal=new TStringList();
    TStringList *sListPass =new TStringList();
    TStringList *sListFail =new TStringList();
    sListTotal->Clear();
    sListPass->Clear();
    sListFail->Clear();
    for(int i=0; i<TestSocket.iMaxRow; i++)
    {
        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            sListTotal->Add("0");
            sListPass->Add("0(0.00\%)");
            sListFail->Add("0(0.00\%)");
        }
    }

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(TestIF.iSiteMap[i][j]>0)
            {
                iSiteCh=TestIF.iSiteMap[i][j]-1;
                iSiteTotalCt=TastCategory.iBySiteTotal[iSiteCh];
                iSitePassCt =TastCategory.iBySitePass[iSiteCh];
                iSiteFailCt =TastCategory.iBySiteFail[iSiteCh];

                if(iSiteTotalCt>0)
                {
                    sPassYield.sprintf("%d(%s)", iSitePassCt, ChangeToPercentage(iSitePassCt, iSiteTotalCt));
                    sFailYield.sprintf("%d(%s)", iSiteFailCt, ChangeToPercentage(iSiteFailCt, iSiteTotalCt));
                    sListTotal->Strings[iSiteCh]=AnsiString().sprintf("%d", iSiteTotalCt);
                    sListPass->Strings[iSiteCh] =AnsiString().sprintf("%s", sPassYield);
                    sListFail->Strings[iSiteCh] =AnsiString().sprintf("%s", sFailYield);
                }
            }
        }
    }

    sTotal.sprintf("%-34s", "Total");                                           //Total DUT1 DUT2 DUT3 DUT4 SUM
    sPassYield.sprintf("%-34s", "PASS");                                        //PASS DUT1 DUT2 DUT3 DUT4 SUM
    sFailYield.sprintf("%-34s", "FAIL");                                        //FAIL DUT1 DUT2 DUT3 DUT4 SUM

    for(int i=0; i<iTotalCh; i++)
    {
        sTempT.sprintf(" %-20s", AnsiString(sListTotal->Strings[i]));          //AI(W906-G023) P4: golden passes Strings[i] straight to %s
        sTempP.sprintf(" %-20s", AnsiString(sListPass->Strings[i]));           //AI(W906-G023) P4
        sTempF.sprintf(" %-20s", AnsiString(sListFail->Strings[i]));           //AI(W906-G023) P4

        sTotal+=sTempT;
        sPassYield+=sTempP;
        sFailYield+=sTempF;
    }

    sP.sprintf("%d(%s)", TastCategory.iPassSocket, ChangeToPercentage(TastCategory.iPassSocket, TastCategory.iTotalSocket));
    sF.sprintf("%d(%s)", TastCategory.iFailSocket, ChangeToPercentage(TastCategory.iFailSocket, TastCategory.iTotalSocket));
    sTempT.sprintf(" %-20d", TastCategory.iTotalSocket);
    sTempP.sprintf(" %-20s", sP);
    sTempF.sprintf(" %-20s", sF);

    sTotal+=sTempT;
    sPassYield+=sTempP;
    sFailYield+=sTempF;

    redtSummary->Lines->Add(sTotal);
    redtSummary->Lines->Add(sPassYield);
    redtSummary->Lines->Add(sFailYield);
    redtSummary->Lines->Add(str.sprintf(" "                                     ));
    redtSummary->Lines->Add(str.sprintf("============================= BY BIN COUNT ============================="));
    redtSummary->Lines->Add(str.sprintf(" "                                     ));

    str.sprintf("%-34s", "HW BIN Count:");                                      //HW BIN Count: DUT1 DUT2 DUT3 DUT4 SUM
    for(int i=0; i<iTotalCh; i++)
    {
        s.sprintf("DUT%d", i+1);
        sTemp.sprintf(" %-20s", s);
        str+=sTemp;
    }
    sTemp.sprintf(" %-20s", "SUM");
    str+=sTemp;
    redtSummary->Lines->Add(str);

    for(int iCat=0; iCat<iTestBinCount; iCat++)
    {
        s.sprintf("BIN %d", iCat);
        sTotal.sprintf("%-34s", s);
        for(int iDut=0; iDut<iTotalCh; iDut++)
        {
            sPassYield.sprintf("%d(%s)", TastCategory.iBySiteCate[iDut][iCat], ChangeToPercentage(TastCategory.iBySiteCate[iDut][iCat], TastCategory.iTotalCategory[iCat]));
            sTempT.sprintf(" %-20s", sPassYield);
            sTotal+=sTempT;
        }

        sPassYield.sprintf("%d(%s)", TastCategory.iTotalCategory[iCat], ChangeToPercentage(TastCategory.iTotalCategory[iCat], TastCategory.iTotalSocket));
        sTempT.sprintf(" %-20s", sPassYield);
        sTotal+=sTempT;
        redtSummary->Lines->Add(sTotal);
    }

    s.sprintf("REJECT");
    sTotal.sprintf("%-34s", s);
    for(int iDut=0; iDut<iTotalCh; iDut++)
    {
        sPassYield.sprintf("%d(%s)", TastCategory.iBySiteCate[iDut][iTestBinCount], ChangeToPercentage(TastCategory.iBySiteCate[iDut][iTestBinCount], TastCategory.iTotalCategory[iTestBinCount]));
        sTempT.sprintf(" %-20s", sPassYield);
        sTotal+=sTempT;
    }

    sPassYield.sprintf("%d(%s)", TastCategory.iTotalCategory[iTestBinCount], ChangeToPercentage(TastCategory.iTotalCategory[iTestBinCount], TastCategory.iTotalSocket));
    sTempT.sprintf(" %-20s", sPassYield);
    sTotal+=sTempT;
    redtSummary->Lines->Add(sTotal);

    if(bViewOnly==false)
    {
//        if(IniConfig.bB05_OSReport)                                             //AI(ht9045-config) 20260520 (RogerYang) : 恢復Summary上傳網盤
//            redtSummary->Lines->SaveToFile(ServerFile);
        redtSummary->Lines->SaveToFile(asFileName);                             //AI(W906-G023) P3: RTF, like golden's TRichEdit
    }

    // golden leaks sListTotal / sListPass / sListFail here (no delete) -- kept, three small lists per Lot End.
    TesterTCP_ProcessOSTrayData(bViewOnly);                                     // golden :960 ProcessOSTrayData(bViewOnly)
    RunInfo.SaveJamRateByLot();                                                 //Steven 20200415 : SCC要By Lot Jam Rate
}

//------------------------------------------------------------------------------
// golden TfTesterTCP::ProcessOSTrayData, Interface/TesterTCP.cpp:964-1056
//------------------------------------------------------------------------------
void TesterTCP_ProcessOSTrayData(bool bViewOnly)
{
    W906OSReportWidgets();                                                      //AI(W906-G023) P2
    TStringList *SummaryHead=fTesterTCP->SummaryHead;                           //AI(W906-G023) P2

    AnsiString asFolderName, ServerFile, asFileName, SourceFile, str;
    AnsiString asAutoName[eTrayCount], asAutoServer[eTrayCount];
    asFolderName.sprintf("%s\\OS_Summary\\%04d%02d", as9045LogPath, SystemYear, SystemMonth);   //AI(W906-G023) P1: golden "D:\\HT9045_Log\\OS_Summary\\%04d%02d"
    MyForceDirectories(asFolderName, "ProcessOSPrint");

    ServerFile.sprintf("%s\\OS_Pin_All_%s_%04d_%02d_%02d_%02d_%02d_%02d.TXT",
                                                    IniConfig.sB05_OSReportPath,
                                                    RunInfo.LotNo,
                                                    SystemYear, SystemMonth, SystemDate,
                                                    SystemHour, SystemMin, SystemSec);

    asFileName.sprintf("%s\\OS_Pin_All_%s_%04d_%02d_%02d_%02d_%02d_%02d.TXT",
                                                    asFolderName,
                                                    RunInfo.LotNo,
                                                    SystemYear, SystemMonth, SystemDate,
                                                    SystemHour, SystemMin, SystemSec);

    for(int iAuto=0; iAuto<eTrayCount; iAuto++)
    {
        asAutoName[iAuto].sprintf("%s\\OS_Pin_%s_%s_%04d_%02d_%02d_%02d_%02d_%02d.TXT",
                                                        asFolderName,
                                                        s6TrayName[iAuto],
                                                        RunInfo.LotNo,
                                                        SystemYear, SystemMonth, SystemDate,
                                                        SystemHour, SystemMin, SystemSec);

        asAutoServer[iAuto].sprintf("%s\\OS_Pin_%s_%s_%04d_%02d_%02d_%02d_%02d_%02d.TXT",
                                                        IniConfig.sB05_OSReportPath,
                                                        s6TrayName[iAuto],
                                                        RunInfo.LotNo,
                                                        SystemYear, SystemMonth, SystemDate,
                                                        SystemHour, SystemMin, SystemSec);
    }

    SourceMemo->Clear();
    for(int i=0; i<SummaryHead->Count; i++)                                     //加入summary的標頭
        SourceMemo->Lines->Add(SummaryHead->Strings[i]);

    for(int iAuto=0; iAuto<eTrayCount; iAuto++)
    {
        RichEdit[iAuto]->Clear();

        if(Prod.iTrayType[iAuto]!=tNotUse)
        {
            SourceFile.sprintf("%s\\OS_TestReport\\%s_TestReport.TXT", as9045LogPath, s6TrayName[iAuto]);   //AI(W906-G023) P1: golden "D:\\HT9045_Log\\OS_TestReport\\%s_TestReport.TXT"
            if(CheckFileExist(SourceFile))
            {
                RichEdit[iAuto]->Lines->LoadFromFile(SourceFile);
                SourceMemo->Lines->Add(str.sprintf("===================================    %s    ==================================================", s6TrayName[iAuto]));
                SourceMemo->Lines->Add(str.sprintf(" "                          ));

                for(int i=0; i<RichEdit[iAuto]->Lines->Count; i++)
                {
                    SourceMemo->Lines->Add(RichEdit[iAuto]->Lines->Strings[i]);
                }
                SourceMemo->Lines->Add(str.sprintf(" "                          ));
            }

            for(int i=0; i<SummaryHead->Count; i++)                             //AI(ht9045-config) 20260520 (RogerYang) : 無料Auto仍產生報表
                RichEdit[iAuto]->Lines->Insert(i, SummaryHead->Strings[i]);
            RichEdit[iAuto]->Lines->Insert(SummaryHead->Count, " ");
            RichEdit[iAuto]->Lines->Insert(SummaryHead->Count+1, "Fail Pin,,,,===============\xA4U\xAD\xAD,         \xA4W\xAD\xAD        ,\xB6q\xB4\xFA\xAD\xC8========");   //AI(W906-G023) P5: golden Big5 bytes of "Fail Pin,,,,===============下限,         上限        ,量測值========"

            if(bViewOnly==false)
            {
                if(IniConfig.bB05_OSReport &&                                   //AI(ht9045-config) 20260530 (RogerYang) : 只上傳Auto2+Auto3
                    (s6TrayName[iAuto]=="Auto2" ||
                        s6TrayName[iAuto]=="Auto3"))
                {
                    if (!W906_SimNetPathBlocked(asAutoServer[iAuto])) RichEdit[iAuto]->Lines->SaveToFile(asAutoServer[iAuto]);   //AI(W906-G023) P6: W58 Q5
                }

                if(s6TrayName[iAuto].Pos("Fix")==0)                             //AI(ht9045-config) 20260530 (RogerYang) : Fix不存本地
                    RichEdit[iAuto]->Lines->SaveToFile(asAutoName[iAuto]);
            }

            if(CheckFileExist(SourceFile))                                      //AI(ht9045-config) 20260520 (RogerYang) : 清除暫存
            {
                RichEdit[iAuto]->Clear();
                RichEdit[iAuto]->Lines->SaveToFile(SourceFile);
            }
        }
    }

    if(bViewOnly==false)
    {
//        if(IniConfig.bB05_OSReport)                                             //AI(ht9045-config) 20260520 (RogerYang) : 恢復All上傳網盤
//            SourceMemo->Lines->SaveToFile(ServerFile);
        SourceMemo->Lines->SaveToFile(asFileName);
    }
}
