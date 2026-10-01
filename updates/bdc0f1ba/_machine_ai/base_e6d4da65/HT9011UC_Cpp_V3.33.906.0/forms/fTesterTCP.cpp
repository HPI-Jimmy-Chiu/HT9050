// =============================================================================
//  forms/fTesterTCP.cpp  --  definitions for the fTesterTCP facade
//
//  AI(W906-FW3-TIF1) 20260828: new file, FW wave FW3-TIF1.
//  GOLDEN SOURCE: HT9011UC_Code_V3.33.906.0_20260618/Interface/TesterTCP.cpp
//  (1117 lines) + Interface/TesterTCP.h (267 lines), read with
//  `io.open(p, encoding='cp950')`.
//  SPAN: 23 golden `TfTesterTCP::` member bodies, 1068 span lines
//  (tools/census/wave_preflight.py, 20260828).
//
//  THIS WAVE, against the 23-member denominator:
//     1 ACTIVE-PARTIAL   ctor -- 8 of its 86 golden lines live
//     6 GATED-WITH-BODY  448 golden span lines carried as `#if 0` transcript
//    16 NOT RE-TRANSLATED -- they ALREADY have a port in ht9045_sm, as FREE
//       FUNCTIONS (not as methods of any class), body-by-body verified:
//         Interface/TesterTCP_Socket.cpp  12 x `TesterTCPSocket_*`
//                                         (golden :166-345, :349-613, :615-633)
//                                         -- 4 of them RENAMED to On*, so a
//                                         name-only search under-reports them
//         Interface/TesterTCP.cpp          4 x `TesterTCP_*`
//                                         (golden :660-697, :1058-1115)
//       Re-translating them here would put a SECOND transcript of the same
//       golden text in the same tree. Deliberately not done -- see the
//       "READ THIS FIRST" block in forms/fTesterTCP.h.
//
//  See forms/fTesterTCP.h for the full GATE REGISTER (T-P1)/(T-P2)/(T-1)..
//  (T-7), the ACTIVE evidence, DEVIATIONS D-1..D-4, the FIELD LIST (including
//  what is deliberately NOT declared) and the ZERO-WRITER value-provenance
//  warning about bOutArmVariAuto_OS[0].
//
//  ⚠ EVERY `#if 0` BLOCK BELOW HAS NEVER BEEN COMPILED. The text is golden's
//  own, sliced programmatically out of the cp950 decode (the ONLY edit is
//  dropping `__fastcall`), so it is a faithful TRANSCRIPT -- not verified
//  code. Most of the identifiers it names (RichEdit, GroupBox, SourceMemo,
//  redtSummary, scrlbxSummary, bFlag, cbSite01.., TastCategory, Prod, alTop,
//  clWhite, ssBoth, ...) are NOT members of this facade and/or have no port;
//  un-gating requires supplying them first.
// =============================================================================
#include "forms/fTesterTCP.h"

// AI(W906-FW3-TIF1) 20260828: TfTesterTCP/fTesterTCP were FREE tree-wide --
// same idiom as forms/fPrecaution.cpp:14 / forms/fHotPlate.cpp:24. The ACTIVE
// part of golden's ctor (:27 and :61-67) writes only this object's own fields
// and allocates two vclcompat::TStringList, so this static-init `new` touches
// no global -- no SIOF risk (docs/KNOWLEDGE.md "static-init ctor 不可碰 NULL
// 全域"; the fLaserSensor incident that rule comes from turned 88 of 134
// ctest binaries into SEGFAULTs). Golden's remaining ctor statements DO read
// globals (CUSTOMER_CODE, s6TrayName[]) and are gated -- see (T-1).
TfTesterTCP *fTesterTCP = new TfTesterTCP();

// ---------------------------------------------------------------------------
//  golden :24-109.  ACTIVE-PARTIAL: golden :27 + :61-67 are live below; golden
//  :28-59 (the 4x32 widget mirror arrays) and :69-108 (dynamic TGroupBox/TMemo
//  construction + the rgUnloader visibility pass) are GATE (T-1) and appear as
//  transcript further down this file.
// ---------------------------------------------------------------------------
TfTesterTCP::TfTesterTCP()
{
    OSRecipe="";                                                                //Steven 20230116 : OS測試機傳送工作檔名
    // GATE (T-1)a -- golden :28-59 (widget mirror arrays), transcript below.

    SocketTCPIPReceiveList          =new TStringList;
    SummaryHead                     =new TStringList;
    bEnableTCPIPChannelConnect      =true;
    SimulateStart                   =false;
    bTCPError                       =false;
    SocketTCPIPReceiveList->Clear();
    SummaryHead->Clear();

    // GATE (T-1)b -- golden :69-108 (per-tray TGroupBox/TMemo creation and the
    // rgUnloader visibility pass), transcript below.
}

// ===========================================================================
//  GATE REGISTER -- translated golden bodies, deliberately NOT COMPILED.
// ===========================================================================

#if 0 // GATE (T-1)a -- golden :28-59, the ctor's widget mirror arrays.
      // (a) static-init safety, (b) 128 undeclared per-site widgets,
      // (c) plSite/cbSiteOn/labOcr/cbSimulateBin arrays are not members here.
{
    AnsiString Str;
    TComboBox *tempComboBox[32]={cbSite01, cbSite02, cbSite03, cbSite04, cbSite05, cbSite06, cbSite07, cbSite08,
                                 cbSite09, cbSite10, cbSite11, cbSite12, cbSite13, cbSite14, cbSite15, cbSite16,
                                 cbSite17, cbSite18, cbSite19, cbSite20, cbSite21, cbSite22, cbSite23, cbSite24,
                                 cbSite25, cbSite26, cbSite27, cbSite28, cbSite29, cbSite30, cbSite31, cbSite32};

    TPanel *tempPanel[32]={plSite01, plSite02, plSite03, plSite04, plSite05, plSite06, plSite07, plSite08,
                           plSite09, plSite10, plSite11, plSite12, plSite13, plSite14, plSite15, plSite16,
                           plSite17, plSite18, plSite19, plSite20, plSite21, plSite22, plSite23, plSite24,
                           plSite25, plSite26, plSite27, plSite28, plSite29, plSite30, plSite31, plSite32};

    TCheckBox *tempCheckBox[32]={cbSiteOn01, cbSiteOn02, cbSiteOn03, cbSiteOn04, cbSiteOn05, cbSiteOn06, cbSiteOn07, cbSiteOn08,
                                 cbSiteOn09, cbSiteOn10, cbSiteOn11, cbSiteOn12, cbSiteOn13, cbSiteOn14, cbSiteOn15, cbSiteOn16,
                                 cbSiteOn17, cbSiteOn18, cbSiteOn19, cbSiteOn20, cbSiteOn21, cbSiteOn22, cbSiteOn23, cbSiteOn24,
                                 cbSiteOn25, cbSiteOn26, cbSiteOn27, cbSiteOn28, cbSiteOn29, cbSiteOn30, cbSiteOn31, cbSiteOn32};

    TLabel *tempLabel[32]={labOcr01, labOcr02, labOcr03, labOcr04, labOcr05, labOcr06, labOcr07, labOcr08,
                           labOcr09, labOcr10, labOcr11, labOcr12, labOcr13, labOcr14, labOcr15, labOcr16,
                           labOcr17, labOcr18, labOcr19, labOcr20, labOcr21, labOcr22, labOcr23, labOcr24,
                           labOcr25, labOcr26, labOcr27, labOcr28, labOcr29, labOcr30, labOcr31, labOcr32};

    for(int i=0; i<32; i++)
    {
        cbSimulateBin[i]=tempComboBox[i];
        cbSimulateBin[i]->ItemIndex=0;
        plSite[i]=tempPanel[i];
        plSite[i]->Caption="--";
        cbSiteOn[i]=tempCheckBox[i];
        cbSiteOn[i]->Checked=false;
        labOcr[i]=tempLabel[i];
        labOcr[i]->Caption="";
    }
}
#endif // GATE (T-1)a

#if 0 // GATE (T-1)b -- golden :69-108, the ctor's per-tray TGroupBox/TMemo
      // construction. Needs ->Name/->Parent/->Height/->Align/->Font/
      // ->ScrollBars/->Color (none exist on vclcompat::TGroupBox:308 or
      // vclcompat::TMemo:367), plus scrlbxSummary (TScrollBox, no port),
      // s6TrayName[]/CUSTOMER_CODE (globals read at static-init time),
      // alTop/alNone/alClient/clWhite/clBlack/ssBoth.
{
    for(int iAuto=0; iAuto<eTrayCount; iAuto++)
    {
        Str.sprintf("GroupBox_%s", s6TrayName[iAuto]);
        bFlag[iAuto]=true;
        if(CUSTOMER_CODE==CC_JSCC_OS &&                                         //RogerYang 20260128 : 田揚志畫面修改需求
            Str.Pos("Fix")!=0)
        {
            bFlag[iAuto]=false;
        }

        GroupBox[iAuto]         =new TGroupBox(scrlbxSummary);
        GroupBox[iAuto]->Name   =Str;
        GroupBox[iAuto]->Caption=s6TrayName[iAuto];
        GroupBox[iAuto]->Parent =scrlbxSummary;
        GroupBox[iAuto]->Height =100;
        GroupBox[iAuto]->Top    =iAuto*100;
        GroupBox[iAuto]->Align  =alTop;
        GroupBox[iAuto]->Font->Name="Courier New";
        GroupBox[iAuto]->Font->Color=clWhite;

        Str.sprintf("RichEdit_%s", s6TrayName[iAuto]);
        RichEdit[iAuto]         =new TMemo(GroupBox[iAuto]);
        RichEdit[iAuto]->Name   =Str;
        RichEdit[iAuto]->Parent =GroupBox[iAuto];
        RichEdit[iAuto]->Align  =alClient;
        RichEdit[iAuto]->Color  =clWhite;                                       //RogerYang 20260128 : 田揚志畫面修改需求
        RichEdit[iAuto]->Font->Name="Courier New";
        RichEdit[iAuto]->Font->Color=clBlack;                                   //RogerYang 20260128 : 田揚志畫面修改需求
        RichEdit[iAuto]->ScrollBars=ssBoth;
        RichEdit[iAuto]->Clear();
    }

    rgUnloader->ItemIndex=1;
    for(int i=0; i<eTrayCount; i++)
    {
        GroupBox[i]->Visible=false;
        GroupBox[i]->Align=alNone;
    }
    GroupBox[rgUnloader->ItemIndex-1]->Visible=true;
    GroupBox[rgUnloader->ItemIndex-1]->Align=alClient;
}
#endif // GATE (T-1)b

#if 0 // GATE (T-2) FormDestroy -- golden :111-123. TRANSITIVE on (T-1): the
      // whole body deletes objects the gated ctor block never allocated.
void TfTesterTCP::FormDestroy(TObject *Sender)
{
    SocketTCPIPReceiveList->Clear();
    delete SocketTCPIPReceiveList;
    SummaryHead->Clear();
    delete SummaryHead;

    for(int iAuto=0; iAuto<eTrayCount; iAuto++)
    {
        delete RichEdit[iAuto];
        delete GroupBox[iAuto];
    }
}
#endif // GATE (T-2)

#if 0 // GATE (T-3) FormShow -- golden :125-164. HIDDEN WRITE x2:
      // :147-149 CheckAndReadIniData writes the default back when the key is
      // missing (common.cpp:1567-1569 -- NOT the pure-read ReadIniData), and
      // :144 MyForceDirectories creates directories under DataPath /
      // sSaveByMachine, i.e. the shared production parameter tree.
      // NOTE golden's own brace indentation at :136-143 is misaligned; kept
      // verbatim (it is syntactically fine, just ugly).
void TfTesterTCP::FormShow(TObject *Sender)
{
    edTCPIP_Address->Text=TestIF_File.asTester_Address;
    edTCPIP_Port->Text=TestIF_File.iTester_Port;

    if(CUSTOMER_CODE==CC_JSCC_OS)                                               //RogerYang 20260128 : 田揚志畫面修改需求
    {
        pgcTcpIp->ActivePageIndex=3;

        AnsiString szDir="";                                                    //RogerYang 20260210 : 田揚志需求，有設定的才可以一次放全部，OS報表要照Tray盤放料順序顯示
        LastFileName=GetLastOpenFN();
        if(CosFunction.bSaveArmSpeedByMachine && IniConfig.bA57_1SaveArmSpeedByMachine)
    {
            szDir=sSaveByMachine;
        }
        else
        {
            szDir.sprintf("%s%s", DataPath, LastFileName);
    }
        MyForceDirectories(szDir);
        szDir+="\\ArmCondition.Data";

        bOutArmVariAuto_OS[0]=CheckAndReadIniData(szDir, "Output Arm", "JSCC_OS_Use Auto1", 1);   //default auto 1 enable
        bOutArmVariAuto_OS[1]=CheckAndReadIniData(szDir, "Output Arm", "JSCC_OS_Use Auto2", 0);
        bOutArmVariAuto_OS[2]=CheckAndReadIniData(szDir, "Output Arm", "JSCC_OS_Use Auto3", 0);
        chkAuto1_OS->Checked=bOutArmVariAuto_OS[0];
        chkAuto2_OS->Checked=bOutArmVariAuto_OS[1];
        chkAuto3_OS->Checked=bOutArmVariAuto_OS[2];
    }
    else
    {
        pgcTcpIp->ActivePageIndex=0;
        for(int iAuto=0; iAuto<eTrayCount; iAuto++)
        {
            GroupBox[iAuto]->Visible=
                (Prod.iTrayType[iAuto]!=tNotUse && bFlag[iAuto]==true);             //RogerYang 20260128 : 田揚志畫面修改需求
        }
    }
    fShow=true;
}
#endif // GATE (T-3)

#if 0 // GATE (T-4) rgUnloaderClick -- golden :635-658. MISSING MEMBERS:
      // GroupBox[i]->Height / ->Align and alTop/alNone/alClient have no port.
      // Also depends on the GATED (T-1) ctor to populate GroupBox[].
void TfTesterTCP::rgUnloaderClick(TObject *Sender)
{
    if(rgUnloader->ItemIndex==0)
    {
        for(int i=0; i<eTrayCount; i++)
        {
            GroupBox[i]->Visible=(Prod.iTrayType[i]!=tNotUse && bFlag[i]==true);//RogerYang 20260128 : 田揚志畫面修改需求
            GroupBox[i]->Height=100;
            GroupBox[i]->Top=i*100;
            GroupBox[i]->Align=alTop;
        }
    }
    else
    {
        for(int i=0; i<eTrayCount; i++)
        {
            GroupBox[i]->Visible=false;
            GroupBox[i]->Align=alNone;
        }

        GroupBox[rgUnloader->ItemIndex-1]->Visible=true;
        GroupBox[rgUnloader->ItemIndex-1]->Align=alClient;
    }
}
#endif // GATE (T-4)

#if 0 // GATE (T-5) PlaceOSTestResultToTray -- golden :699-739. WRITES DISK
      // (:738 SaveToFile), DELETES a file (:718 DeleteFile), creates
      // directories (:713 MyForceDirectories).
void TfTesterTCP::PlaceOSTestResultToTray(int iSuckRow, int iSuckCol, int iTrayRow, int iTrayCol, int iAuto)
{
    if(TestIF_File.iTestType!=TCP_IP_MODE)
        return;

    AnsiString sSourceFileName, sTargetFileName, Str1, Str2;
    int iTotalCh        =TestSocket.iShtRow*TestSocket.iShtCol;
    int iTesterCh       =OutArmSuck.PordRec[iSuckRow][iSuckCol].GetSiteNo();
    int iContactIndex   =OutArmSuck.PordRec[iSuckRow][iSuckCol].GetOrderOfContact();

    SourceMemo->Lines->Clear();
    RichEdit[iAuto]->Lines->Clear();
    sTargetFileName.sprintf("D:\\HT9045_Log\\OS_TestReport\\%s_TestReport.TXT", s6TrayName[iAuto]);
    sSourceFileName.sprintf("D:\\HT9045_Log\\OSTestResult\\Device%06d_%02d.TXT", iContactIndex, iTesterCh-1);
    MyForceDirectories("D:\\HT9045_Log\\OS_TestReport", "ProcessOSPrint");

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
#endif // GATE (T-5)

#if 0 // GATE (T-6) ProcessOSPrint -- golden :741-962 (222L). WRITES DISK
      // (:957 SaveToFile), creates directories (:757), TRANSITIVE on (T-7)
      // at :960. `redtSummary` is a golden TRichEdit* and `class TRichEdit`
      // has ZERO port in this tree (0 hits, 20260828) -- second, independent
      // gate.
void TfTesterTCP::ProcessOSPrint(bool bViewOnly)                     //Steven 20250515 : 整合Open Short測試報表
{
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

    asFolderName.sprintf("D:\\HT9045_Log\\OS_Summary\\%04d%02d", SystemYear, SystemMonth);
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
        sTempT.sprintf(" %-20s", sListTotal->Strings[i]);
        sTempP.sprintf(" %-20s", sListPass->Strings[i]);
        sTempF.sprintf(" %-20s", sListFail->Strings[i]);

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
        redtSummary->Lines->SaveToFile(asFileName);
    }

    ProcessOSTrayData(bViewOnly);
    RunInfo.SaveJamRateByLot();                                                 //Steven 20200415 : SCC要By Lot Jam Rate
}
#endif // GATE (T-6)

#if 0 // GATE (T-7) ProcessOSTrayData -- golden :964-1056. WRITES DISK four
      // times (:1035/:1039/:1045/:1054 SaveToFile), creates directories
      // (:969). Depends on RichEdit[]/SourceMemo from the GATED (T-1) ctor.
void TfTesterTCP::ProcessOSTrayData(bool bViewOnly)
{
    AnsiString asFolderName, ServerFile, asFileName, SourceFile, str;
    AnsiString asAutoName[eTrayCount], asAutoServer[eTrayCount];
    asFolderName.sprintf("D:\\HT9045_Log\\OS_Summary\\%04d%02d", SystemYear, SystemMonth);
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
            SourceFile.sprintf("D:\\HT9045_Log\\OS_TestReport\\%s_TestReport.TXT", s6TrayName[iAuto]);
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
            RichEdit[iAuto]->Lines->Insert(SummaryHead->Count+1, "Fail Pin,,,,===============下限,         上限        ,量測值========");

            if(bViewOnly==false)
            {
                if(IniConfig.bB05_OSReport &&                                   //AI(ht9045-config) 20260530 (RogerYang) : 只上傳Auto2+Auto3
                    (s6TrayName[iAuto]=="Auto2" ||
                        s6TrayName[iAuto]=="Auto3"))
                {
                    RichEdit[iAuto]->Lines->SaveToFile(asAutoServer[iAuto]);
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
#endif // GATE (T-7)
