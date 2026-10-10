// =============================================================================
//  cStartCondition.cpp  --  Start Condition / Life Time dialog: current-value
//                            read + widget fill (batch-5 wave, display-only
//                            bucket -- see forms/fStartCondition.h banner
//                            FIRST, it documents the full 35-method
//                            classification, GATE register, and design notes;
//                            not repeated in full here).
//
//  Faithful translation of golden cStartCondition.cpp (1,716 lines, BCB6,
//  cp950). Translator: AI(W906-FW3-StartCond-WA) 20260819.
//  Translation wave: FW-3 cStartCondition Wave A (batch 5, display-only
//  mandate). Facade: forms/fStartCondition.h.
//
//  WAVE SCOPE -- 8 of 35 golden methods (bucket (a), display/read-only; see
//  forms/fStartCondition.h banner for the other 27's classification):
//    TfStartCondition() ctor   golden :40-104
//    FormShow                  golden :106-536  (431 lines; GATE (SC1)/(SC2)/
//                               (SC3)/(SC6), see banner + inline)
//    DoIniDataToForm           golden :1009-1027
//    pgLifeTimeChange          golden :1029-1047 (ENTIRE body GATE (SC6))
//    GetCylinderIndexByName    golden :1479-1487
//    UpdateCylinderScreen      golden :1489-1589 (GATE (SC4))
//    LoadCylinderLife          golden :1591-1653
//    strngrdCylinderViewDrawCell golden :1418-1450 (ENTIRE body GATE (SC5))
//
//  GATE REGISTER -- full rationale in forms/fStartCondition.h, re-cited
//  briefly at each call site below, PLUS one NEW gate found while writing
//  this file (not yet in the header banner at facade-authoring time --
//  recorded here and cross-referenced):
//   (SC1) FormShow :145  -- LastSet.iStartMode=2 hidden write.
//   (SC2) FormShow :171  -- LastSet.iStartMode=1 hidden write.
//   (SC3) FormShow :167  -- fSetup->ReadFile(), fSetup facade has no such method.
//   (SC4) UpdateCylinderScreen :1565-1572 -- self-healing negative-count
//         clamp + SaveCylinderLife() hidden write.
//   (SC5) strngrdCylinderViewDrawCell -- no TCanvas anywhere in vclcompat.
//   (SC6) *** NEW, found this file *** FormShow :323-381 and the ENTIRE body
//         of pgLifeTimeChange (golden :1029-1047) compare
//         `pgLifeTime->ActivePage==tsConditionNN` -- a POINTER-valued
//         `TTabSheet *ActivePage` property. vclcompat::TPageControl
//         (Controls.h:434-441) carries ONLY `int ActivePageIndex`, no
//         `ActivePage` pointer -- the EXACT SAME missing-property class
//         forms/fSpeed.h's GATE (S2) already documents for its own
//         `PageControl1->ActivePage=tsAllSpeed;` (an assignment, no
//         consumer, harmless to drop). HERE the gap is more consequential:
//         it blocks a real READ used by ~40 lines of alarm-count-per-page
//         display logic, not a cosmetic write. Per "跨檔缺口 GATE 不自建
//         shim", NOT worked around by adding a locally-tracked ActivePage
//         field to a `TfStartConditionPageControl` wrapper -- with nothing
//         in this wave's translated scope ever WRITING such a field, it
//         would sit at its default forever and make every `==tsConditionNN`
//         comparison permanently false, which is behaviourally identical to
//         an honest `#if 0` gate but LOOKS like live code. `#if 0`-gated
//         instead, explicitly, at both call sites.
//
//  DEPENDENCY AUDIT (this wave, 20260819) -- every non-widget global/struct
//  member the 8 in-scope methods touch was grepped against this port tree
//  before translating. Confirmed present with matching field/function names:
//    cAuthority.h : authCounterClr[9], GetCountClrAuth(), GetLimitAuth(),
//                   AccessLevel, iDefEngineerLevel, iDefHonPrecLevel.
//    cmydef.h/MachineType.h: CUSTOMER_CODE, CC_SCC/CC_ASE_KaohSiung/
//                   CC_KYEC_LEE/CC_AMD_M/CC_ASE_CL/CC_PTI/CC_TERAPOWER/
//                   CC_HANA_MICRON/CC_ASE_JP/CC_SIGURD_HUKOU (all confirmed
//                   #define'd), TRAY_VIBRATION/VibrationMotor,
//                   rsmInitialStart/rsmCInitialRetest, SystemInitialOK,
//                   eCylName..eCylPrAlarmItemTotal (MachineType.h:1386-1399,
//                   exact match to golden's enum), MaxCylinderItem=321
//                   (mycylin.h:42; was 295 -- AI(W906-F9050-BD) 20261004).
//    mycylin.h/.cpp: `extern class TMyCylinder Cylinder[MaxCylinderItem];`
//                   (mycylin.h:176) with CylinderName/Enable/iOnOffCount/
//                   iTimeOutCount/GetOnTime()/GetOffTime()/GetOnTimeAvg()/
//                   GetOffTimeAvg()/GetOnTimeAlarm()/GetOffTimeAlarm()/
//                   iOnOffCountAlarm/iTimeOutCountAlarm/iResetCount/
//                   sResetTime/OnSenEnable/OffSenEnable/AddOnTime()/
//                   AddOffTime() all present with matching names/signatures.
//    common.h/.cpp: asMachineLifePath ("D:\HT9045\system\MachineLife.ini"),
//                   ReadIniData/ReadWriteIni (both already relied on by 18+
//                   other translated files per forms/fSpeed.h's own
//                   "DISCLOSED, NOT GATED" precedent for the identical
//                   lazy-seed-on-missing-key behaviour -- NOT re-gated here).
//    aHotPlateSubstrate.h: `extern TMyKitSuck TestSocket;` / `extern TMyKitSuck
//                   FTestSuck;` (the 177-TU shape) -- deliberately NOT
//                   mykitsuck.h, per this tree's own two-TMyKitSuck-headers
//                   gotcha (KNOWLEDGE.md); TMyKitSuck::iShtRow/iShtCol/
//                   iMaxRow/iMaxCol confirmed present.
//    cprod.h      : LastSet (bCTClear[2][7], iStartMode, ContactSet[2],
//                   iContactCT[2], strSocketID[4][8], iSocketContactCount[4][8],
//                   iRunStartMode, iLanguageCountry), IniConfig
//                   (bShowFTandRTButtonCanClick, bContactAlmNeedOneCycle read
//                   via CosFunction -- see below, ContactSet[3][2][8],
//                   HeadContactCount[3][2][8], HeadContactCountHistory[3][2][8],
//                   bLifeTimeCount[3], ContactConditionName[3],
//                   iVibratorHP1/Sht1/Sht2/Unloader, bVTESTFunction,
//                   SocketContactSet/Count/CountHistory[4][8]),
//                   TestIF_File (iContactWarningCount[2], iContactAlarmCount[4],
//                   InOutArmLifeCntSet, InArmPickerLifeCnt[2][4],
//                   OutArmPickerLifeCnt[2][4], Arm1PickerLifeCnt[2][8],
//                   Arm2PickerLifeCnt[2][8]), InputLimit (iOffsetXYHigh,
//                   iOffsetZHigh), IndexSuckName[][] all confirmed present.
//    CosFunction.h: CosFunction (bContactAlmNeedOneCycle, bUseHeadContactCount,
//                   bHiSiliconFunction, bUseSocketContactCount,
//                   bSetOffsetLimitToAll, bCylinderOnOffTimeLog).
//  NOT found / gated: fSetup::ReadFile() (GATE SC3), TPageControl::ActivePage
//  (GATE SC6), TCanvas anywhere (GATE SC5).
// =============================================================================
#include "MachineDefine.h"     // de-VCL'd include hub: vclcompat umbrella + portable STL
#pragma hdrstop

#include "forms/fStartCondition.h"
#include "forms/fSetup.h"           // AI(W906-SETUP-READFILE) 20260922: fSetup for the un-gated ReadFile() at golden :167

#include "cmydef.h"              // CUSTOMER_CODE/CC_*/TRAY_VIBRATION/VibrationMotor/
                                 //   rsmInitialStart/rsmCInitialRetest/SystemInitialOK
#include "cprod.h"               // IniConfig/TestIF_File/InputLimit/IndexSuckName
#include "LastSet.h"             // LastSet (LAST_GENERAL_SET) -- separate header from cprod.h,
                                 //   same as cSpeed.cpp's own dependency audit records
#include "CosFunction.h"         // CosFunction
#include "MachineType.h"        // eCyl*/MaxCylinderItem-adjacent enum
#include "common.h"              // ReadIniData/ReadWriteIni/asMachineLifePath/FormatDateTime
#include "aHotPlateSubstrate.h"  // TestSocket/FTestSuck (TMyKitSuck, 177-TU shape)
#include "cAuthority.h"          // authCounterClr[]/GetCountClrAuth/GetLimitAuth/
                                 //   AccessLevel/iDefEngineerLevel/iDefHonPrecLevel
#include "mycylin.h"             // Cylinder[]/TMyCylinder/MaxCylinderItem
#include "forms/fLotInfo.h"      // fLotInfo->edSocket[][] (DoIniDataToForm cross-form write-through)

//---------------------------------------------------------------------------
//  ctor -- golden :40-104
//---------------------------------------------------------------------------
TfStartCondition::TfStartCondition()
{
    // golden :45-99 builds 4 local TXxx*[][] temp arrays purely to fan values
    // out into 7 different global arrays (SocketID/SocketContactCnt/
    // TestLabelCol/TestLabelRow/TestIDLabelRow/TestCountLabelRow/
    // ClearCntButton) -- this facade's Socket-ID widgets are already named
    // 1:1 members, so only the RUNTIME EFFECT (every Socket-ID widget starts
    // hidden) is reproduced, via local temp arrays scoped to this ctor only
    // (matches the indirection golden itself uses, minus the unneeded global
    // fan-out arrays -- see fStartCondition.h's widget-list DEVIATION;
    // ClearCntButton[][]/btnClearAa.. are bucket (b)-only, not declared).
    TLabel *rowLabels[4]      = {labTestRowA, labTestRowB, labTestRowC, labTestRowD};
    TLabel *rowIDLabels[4]    = {LabelID_RowA, LabelID_RowB, LabelID_RowC, LabelID_RowD};
    TLabel *rowCountLabels[4] = {LabelCnt_RowA, LabelCnt_RowB, LabelCnt_RowC, LabelCnt_RowD};
    TLabel *colLabels[8]      = {labTestColA, labTestColB, labTestColC, labTestColD,
                                  labTestColE, labTestColF, labTestColG, labTestColH};
    TEdit  *socketEdits[4][8] = {
        {editSocketAa, editSocketAb, editSocketAc, editSocketAd, editSocketAe, editSocketAf, editSocketAg, editSocketAh},
        {editSocketBa, editSocketBb, editSocketBc, editSocketBd, editSocketBe, editSocketBf, editSocketBg, editSocketBh},
        {editSocketCa, editSocketCb, editSocketCc, editSocketCd, editSocketCe, editSocketCf, editSocketCg, editSocketCh},
        {editSocketDa, editSocketDb, editSocketDc, editSocketDd, editSocketDe, editSocketDf, editSocketDg, editSocketDh}};
    TPanel *socketPanels[4][8] = {
        {PanelAa, PanelAb, PanelAc, PanelAd, PanelAe, PanelAf, PanelAg, PanelAh},
        {PanelBa, PanelBb, PanelBc, PanelBd, PanelBe, PanelBf, PanelBg, PanelBh},
        {PanelCa, PanelCb, PanelCc, PanelCd, PanelCe, PanelCf, PanelCg, PanelCh},
        {PanelDa, PanelDb, PanelDc, PanelDd, PanelDe, PanelDf, PanelDg, PanelDh}};

    for (int i = 0; i < 4; i++)
    {
        rowLabels[i]->Visible = false;
        rowIDLabels[i]->Visible = false;
        rowCountLabels[i]->Visible = false;
    }
    for (int j = 0; j < 8; j++)
        colLabels[j]->Visible = false;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            socketEdits[i][j]->Visible = false;
            socketPanels[i][j]->Visible = false;
        }
    }

    // golden :102-103 GDI setup (PtrDCCyl=GetDC(...); pCanvasCyl=new TCanvas;)
    // dropped entirely -- see fStartCondition.h's GDI DEVIATION note.
}

//---------------------------------------------------------------------------
//  FormShow -- golden :106-536
//---------------------------------------------------------------------------
void TfStartCondition::FormShow()
{
    GetCountClrAuth();
    GetLimitAuth();                                                             //JerryYang 20200504 銦片計數下限
    cbLoadingCount_FT->Enabled=authCounterClr[3];
    cbTestCategory_FT->Enabled=authCounterClr[1];
    cbContactCount_FT->Enabled=authCounterClr[4];
    cbSortingCount_FT->Enabled=authCounterClr[6];
    cbScanner_FT     ->Enabled=authCounterClr[2];
    cbAlarmData_FT   ->Enabled=authCounterClr[0];
    cbTimeData_FT    ->Enabled=authCounterClr[7];
    cbLoadingCount_RT->Enabled=authCounterClr[3];
    cbTestCategory_RT->Enabled=authCounterClr[1];
    cbContactCount_RT->Enabled=authCounterClr[4];
    cbSortingCount_RT->Enabled=authCounterClr[6];
    cbScanner_RT     ->Enabled=authCounterClr[2];
    cbAlarmData_RT   ->Enabled=authCounterClr[0];
    cbTimeData_RT    ->Enabled=authCounterClr[7];

    cbLoadingCount_FT->Checked=LastSet.bCTClear[0][0];
    cbTestCategory_FT->Checked=LastSet.bCTClear[0][1];
    cbContactCount_FT->Checked=LastSet.bCTClear[0][2];
    cbSortingCount_FT->Checked=LastSet.bCTClear[0][3];
    cbScanner_FT     ->Checked=LastSet.bCTClear[0][4];
    cbAlarmData_FT   ->Checked=LastSet.bCTClear[0][5];
    cbTimeData_FT    ->Checked=LastSet.bCTClear[0][6];
    cbLoadingCount_RT->Checked=LastSet.bCTClear[1][0];
    cbTestCategory_RT->Checked=LastSet.bCTClear[1][1];
    cbContactCount_RT->Checked=LastSet.bCTClear[1][2];
    cbSortingCount_RT->Checked=LastSet.bCTClear[1][3];
    cbScanner_RT     ->Checked=LastSet.bCTClear[1][4];
    cbAlarmData_RT   ->Checked=LastSet.bCTClear[1][5];
    cbTimeData_RT    ->Checked=LastSet.bCTClear[1][6];

    AnsiString str;

    if(IniConfig.bShowFTandRTButtonCanClick==true &&                            //Steven 20131224 : FT & RT Buttion 可以按
       CUSTOMER_CODE!=CC_SCC)                                                   //Steven 20200309 : SCC要使用
    {
        // GATE (SC1): golden :145 `LastSet.iStartMode=2;` -- hidden write
        // inside a function named "Show". See fStartCondition.h banner.
        #if 0
        LastSet.iStartMode=2;
        #endif
        gbStartMode->Visible=false;
    }
    TEdit *edtInArmPickLifeCnt[2][4]={                                          //JerryYang 20220331 : add吸嘴作動次數計數
                                     {edtInArmA, edtInArmC, edtInArmE, edtInArmG},
                                     {edtInArmB, edtInArmD, edtInArmF, edtInArmH},
                                     };

    TEdit *edtOutArmPickLifeCnt[2][4]={
                                     {edtOutArmA, edtOutArmC, edtOutArmE, edtOutArmG},
                                     {edtOutArmB, edtOutArmD, edtOutArmF, edtOutArmH},
                                     };

    TEdit *edtArm1PickLifeCnt[2][8]={
                                     {edtArm1Aa, edtArm1Ab, edtArm1Ac, edtArm1Ad, edtArm1Ae, edtArm1Af, edtArm1Ag, edtArm1Ah},
                                     {edtArm1Ba, edtArm1Bb, edtArm1Bc, edtArm1Bd, edtArm1Be, edtArm1Bf, edtArm1Bg, edtArm1Bh},
                                     };

    TEdit *edtArm2PickLifeCnt[2][8]={
                                     {edtArm2Aa, edtArm2Ab, edtArm2Ac, edtArm2Ad, edtArm2Ae, edtArm2Af, edtArm2Ag, edtArm2Ah},
                                     {edtArm2Ba, edtArm2Bb, edtArm2Bc, edtArm2Bd, edtArm2Be, edtArm2Bf, edtArm2Bg, edtArm2Bh},
                                     };

    // GATE (SC3) RESOLVED -- AI(W906-SETUP-READFILE) 20260922.
    // The gate's only stated reason was "forms/fSetup.h is a one-field facade
    // (bool fShow only), no ReadFile() method exists".  TfSetup::ReadFile now
    // exists (cSetUp.cpp, golden cSetUp.cpp:2191-3020), so the reason is gone.
    // This also clears SC3's DISCLOSED CONSEQUENCE: FTestSuck.iShtRow/iShtCol
    // are consumed further down this same FormShow (golden :255-288) and were
    // read stale; they are now refreshed from HandlerCondition.Data first.
    fSetup->ReadFile();                                                        // golden cStartCondition.cpp:167

    if(cbStartModeOnlyFT->Checked==true)                                       //KaiChen 20181206 ：Start Run Mode Only FT
    {
        // GATE (SC2): golden :171 `LastSet.iStartMode=1;` -- hidden write.
        #if 0
        LastSet.iStartMode=1;
        #endif
    }

    if(LastSet.iStartMode==0)
        rbStartMode1->Checked=true;
    else if(LastSet.iStartMode==1)
        rbStartMode2->Checked=true;
    else
        rbStartMode3->Checked=true;

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20140918 ASET 不能使用ReTest 功能
    {
        rbStartMode3->Enabled=false;
        rbStartMode2->Checked=true;
    }

    if(CosFunction.bContactAlmNeedOneCycle)                                     //Sam 20241226 : Contact Alarm 需要先做 OneCycle
    {
        sgContactCount->ColCount=4;
        sgContactCount->Width=657;
        if(LastSet.iLanguageCountry==1)
        {
            sgContactCount->Cells[1][0]=" 接觸幾次要發出提示";
            sgContactCount->Cells[2][0]=" 接觸幾次要發出警告";
            sgContactCount->Cells[3][0]=" 目前接觸次數";
        }
        else
        {
            sgContactCount->Cells[1][0]=" Contact times for warning";
            sgContactCount->Cells[2][0]=" Contact times for alarm";
            sgContactCount->Cells[3][0]=" Current Status";
        }
        for(int i=0; i<2; i++)
        {
            if(LastSet.iLanguageCountry==1)
                str.sprintf(" %d 號測試頭接觸次數", i+1);
            else
                str.sprintf(" Head %d Contact ", i+1);
            sgContactCount->Cells[0][1+i]=str;
            sgContactCount->Cells[1][1+i]=" "+AnsiString(TestIF_File.iContactWarningCount[i]);
            sgContactCount->Cells[2][1+i]=" "+AnsiString(LastSet.ContactSet[i]);
            sgContactCount->Cells[3][1+i]=" "+AnsiString(LastSet.iContactCT[i]);                                        //jou 2012-06-05
        }
    }
    else
    {
        sgContactCount->ColCount=3;
        sgContactCount->Width=513;
        if(LastSet.iLanguageCountry==1)
        {
            sgContactCount->Cells[1][0]=" 接觸幾次要發出警告";
            sgContactCount->Cells[2][0]=" 目前接觸次數";
        }
        else
        {
            sgContactCount->Cells[1][0]=" Contact times for alarm";
            sgContactCount->Cells[2][0]=" Current Status";
        }

        for(int i=0; i<2; i++)
        {
            if(LastSet.iLanguageCountry==1)
                str.sprintf(" %d 號測試頭接觸次數", i+1);
            else
                str.sprintf(" Head %d Contact ", i+1);
            sgContactCount->Cells[0][1+i]=str;
            sgContactCount->Cells[1][1+i]=" "+AnsiString(LastSet.ContactSet[i]);
            sgContactCount->Cells[2][1+i]=" "+AnsiString(LastSet.iContactCT[i]);                                        //jou 2012-06-05
        }
    }
    //ChungHung 20160420 add for Head Count Start
    sgHeadCondition1->Cells[1][0]=" Contact times for alarm";
    sgHeadCondition1->Cells[2][0]=" Current Status";
    sgHeadCondition1->Cells[3][0]=" Total Contact Count";

    sgHeadCondition2->Cells[1][0]=" Contact times for alarm";
    sgHeadCondition2->Cells[2][0]=" Current Status";
    sgHeadCondition2->Cells[3][0]=" Total Contact Count";

    sgHeadCondition3->Cells[1][0]=" Contact times for alarm";
    sgHeadCondition3->Cells[2][0]=" Current Status";
    sgHeadCondition3->Cells[3][0]=" Total Contact Count";
    //ChungHung 20160420 add for Head Count End

    for(int i=0; i<FTestSuck.iShtRow; i++)                                      //ChungHung 20160420 add for Head Count
    {
        for(int j=0; j<FTestSuck.iShtCol; j++)
        {
            str.sprintf("Arm%d Head %d Contact ", i+1, j+1);

            if(CUSTOMER_CODE==CC_KYEC_LEE)                                      // || CosFunction.bHiSiliconFunction==true)
            {
                sgHeadCondition1->Cells[1][1+(i*8+j)]=" "+AnsiString(TestIF_File.iContactAlarmCount[0]);
                sgHeadCondition2->Cells[1][1+(i*8+j)]=" "+AnsiString(TestIF_File.iContactAlarmCount[1]);
                sgHeadCondition3->Cells[1][1+(i*8+j)]=" "+AnsiString(TestIF_File.iContactAlarmCount[2]);
            }
            else
            {
                sgHeadCondition1->Cells[1][1+(i*8+j)]=" "+AnsiString(IniConfig.ContactSet[0][i][j]);
                sgHeadCondition2->Cells[1][1+(i*8+j)]=" "+AnsiString(IniConfig.ContactSet[1][i][j]);
                sgHeadCondition3->Cells[1][1+(i*8+j)]=" "+AnsiString(IniConfig.ContactSet[2][i][j]);
            }

            sgHeadCondition1->Cells[0][1+(i*8+j)]=str;

            sgHeadCondition1->Cells[2][1+(i*8+j)]=" "+AnsiString(IniConfig.HeadContactCount[0][i][j]);
            sgHeadCondition1->Cells[3][1+(i*8+j)]=" "+AnsiString(IniConfig.HeadContactCountHistory[0][i][j]);

            sgHeadCondition2->Cells[0][1+(i*8+j)]=str;

            sgHeadCondition2->Cells[2][1+(i*8+j)]=" "+AnsiString(IniConfig.HeadContactCount[1][i][j]);
            sgHeadCondition2->Cells[3][1+(i*8+j)]=" "+AnsiString(IniConfig.HeadContactCountHistory[1][i][j]);

            sgHeadCondition3->Cells[0][1+(i*8+j)]=str;
            sgHeadCondition3->Cells[2][1+(i*8+j)]=" "+AnsiString(IniConfig.HeadContactCount[2][i][j]);
            sgHeadCondition3->Cells[3][1+(i*8+j)]=" "+AnsiString(IniConfig.HeadContactCountHistory[2][i][j]);
        }
    }

    if(CosFunction.bUseHeadContactCount)                                        //Ifor 20160516 京元要求銦片 Life Time 功能
    {
        tsCondition01->TabVisible=IniConfig.bLifeTimeCount[0];                  //Ifor 20190920 : add Config 開啟時才顯是對應的Lift Time Count
        tsCondition02->TabVisible=IniConfig.bLifeTimeCount[1];                  //Ifor 20190920 : add Config 開啟時才顯是對應的Lift Time Count
        tsCondition03->TabVisible=IniConfig.bLifeTimeCount[2];                  //Ifor 20190920 : add Config 開啟時才顯是對應的Lift Time Count
        if(CUSTOMER_CODE==CC_AMD_M)
            TabsSocketID->TabVisible=true;
    }
    else if(CUSTOMER_CODE==CC_ASE_CL ||                                         //JerryYang 20170214 (wei) ASE中壢才顯示Socket contact life time
            CUSTOMER_CODE==CC_PTI ||                                            //Sam 20170526 (jou) PTI要求socket contact計數
            CUSTOMER_CODE==CC_TERAPOWER ||                                      //Sam 20190916 : 晶兆成也記錄測頭 Count
            CUSTOMER_CODE==CC_HANA_MICRON ||                                    //JimmyChiu 20211008 R211005-Hana-H9-01
            CUSTOMER_CODE==CC_ASE_JP)                                           //Sam 20220706 : ASE_JP 也記錄測頭 Count
    {
        tsCondition01->TabVisible=false;
        tsCondition02->TabVisible=false;
        tsCondition03->TabVisible=false;
    }
    else
    {
        tsCondition01->TabVisible=false;
        tsCondition02->TabVisible=false;
        tsCondition03->TabVisible=false;
    }

    TabsSocketID->TabVisible=CosFunction.bUseSocketContactCount;                //Steven 20221216 : Modify for Socket ID
    tsSocketCount->TabVisible=CosFunction.bUseSocketContactCount;               //Sam 20220720 : 新增一組 Socket Count
    tsVibration->TabVisible =(TRAY_VIBRATION==VibrationMotor);                  //Steven 20240904 : 修正Start condition畫面顯示

    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CosFunction.bHiSiliconFunction==true ||
       CosFunction.bUseHeadContactCount==true)                                  //RogerYang 20260515 : 修正無法修改alarm setting
    {
        // GATE (SC6): golden :323-377's active-page switch + :366-381's
        // per-page editContactCountAlarm->Text fill both key off
        // `pgLifeTime->ActivePage==tsConditionNN` -- vclcompat::TPageControl
        // has no pointer-valued ActivePage. See file-head GATE register.
        #if 0
        if(pgLifeTime->ActivePage==tsCondition01)                               //Active Page 在第1頁
        {
            if(IniConfig.bLifeTimeCount[0]==false)                              //O12 功能沒開啟 切換至別頁面顯示
            {
                if(IniConfig.bLifeTimeCount[1]==true)                           //O13 功能開啟 切換Active Page 至第2頁
                {
                    pgLifeTime->ActivePage=tsCondition02;
                }
                else if(IniConfig.bLifeTimeCount[2]==true)                      //O14 功能開啟 切換Active Page 至第3頁
                {
                    pgLifeTime->ActivePage=tsCondition03;
                }
            }
        }
        else if(pgLifeTime->ActivePage==tsCondition02)                          //Active Page 在第2頁
        {
            if(IniConfig.bLifeTimeCount[1]==false)                              //O13 功能沒開啟 切換至別頁面顯示
            {
                if(IniConfig.bLifeTimeCount[0]==true)                           //O12 功能開啟 切換Active Page 至第1頁
                {
                    pgLifeTime->ActivePage=tsCondition01;
                }
                else if(IniConfig.bLifeTimeCount[2]==true)                      //O14 功能開啟 切換Active Page 至第3頁
                {
                    pgLifeTime->ActivePage=tsCondition03;
                }
            }
        }
        else if(pgLifeTime->ActivePage==tsCondition03)                          //Active Page 在第3頁
        {
            if(IniConfig.bLifeTimeCount[2]==false)                              //O14 功能沒開啟 切換至別頁面顯示
            {
                if(IniConfig.bLifeTimeCount[0]==true)                           //O12 功能開啟 切換Active Page 至第1頁
                {
                    pgLifeTime->ActivePage=tsCondition01;
                }
                else if(IniConfig.bLifeTimeCount[1]==true)                      //O13 功能開啟 切換Active Page 至第2頁
                {
                    pgLifeTime->ActivePage=tsCondition02;
                }
            }
        }

        if(pgLifeTime->ActivePage==tsCondition01)
        {
            editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[0]);
        }
        else if(pgLifeTime->ActivePage==tsCondition02)
        {
            editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[1]);
        }
        else if(pgLifeTime->ActivePage==tsCondition03)
        {
            editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[2]);
        }
        else if(pgLifeTime->ActivePage==TabsSocketID)                           //Steven 20200211 : Add socket count for ASE-CL
        {
            editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[3]);
        }
        #endif // GATE (SC6)

        if(CUSTOMER_CODE==CC_SCC || IniConfig.bVTESTFunction)                   //AI(ht9045-config) 20260515 (RogerYang) : SCC/VTEST工程師以上即可修改alarm setting
            editContactCountAlarm->Enabled=(AccessLevel>=iDefEngineerLevel);
        else
            editContactCountAlarm->Enabled=(AccessLevel>=iDefHonPrecLevel);
    }
    else
    {
        labContactCountAlarm->Visible=(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI);
        editContactCountAlarm->Visible=(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI);

        if(CUSTOMER_CODE==CC_ASE_CL || CUSTOMER_CODE==CC_PTI)                   //Steven 20200211 : Add socket count for ASE-CL
        {
            editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[3]);
        }
    }

    tsCondition01->Caption=IniConfig.ContactConditionName[0];
    tsCondition02->Caption=IniConfig.ContactConditionName[1];
    tsCondition03->Caption=IniConfig.ContactConditionName[2];

    // golden :403-404 `Left=75; Top=10;` -- TForm-inherited window position,
    // no port needed: this facade has no window to position and no (a)
    // method reads Left/Top back (contrast forms/fSpeed.h's DEVIATION, which
    // added ad-hoc Left/Top members because ITS FormShow both wrote AND later
    // logic implicitly depended on a stable value; here there is no such
    // consumer). Skipped, not even declared.

    TLabel *rowLabels[4]      = {labTestRowA, labTestRowB, labTestRowC, labTestRowD};
    TLabel *rowIDLabels[4]    = {LabelID_RowA, LabelID_RowB, LabelID_RowC, LabelID_RowD};
    TLabel *rowCountLabels[4] = {LabelCnt_RowA, LabelCnt_RowB, LabelCnt_RowC, LabelCnt_RowD};
    TLabel *colLabels[8]      = {labTestColA, labTestColB, labTestColC, labTestColD,
                                  labTestColE, labTestColF, labTestColG, labTestColH};
    TEdit  *socketEdits[4][8] = {
        {editSocketAa, editSocketAb, editSocketAc, editSocketAd, editSocketAe, editSocketAf, editSocketAg, editSocketAh},
        {editSocketBa, editSocketBb, editSocketBc, editSocketBd, editSocketBe, editSocketBf, editSocketBg, editSocketBh},
        {editSocketCa, editSocketCb, editSocketCc, editSocketCd, editSocketCe, editSocketCf, editSocketCg, editSocketCh},
        {editSocketDa, editSocketDb, editSocketDc, editSocketDd, editSocketDe, editSocketDf, editSocketDg, editSocketDh}};
    TPanel *socketPanels[4][8] = {
        {PanelAa, PanelAb, PanelAc, PanelAd, PanelAe, PanelAf, PanelAg, PanelAh},
        {PanelBa, PanelBb, PanelBc, PanelBd, PanelBe, PanelBf, PanelBg, PanelBh},
        {PanelCa, PanelCb, PanelCc, PanelCd, PanelCe, PanelCf, PanelCg, PanelCh},
        {PanelDa, PanelDb, PanelDc, PanelDd, PanelDe, PanelDf, PanelDg, PanelDh}};

    for(int i=0; i<4; i++)                                                      //JerryYang 20170214 (wei) add for socket ID and contact count
    {
        rowLabels[i]->Visible=false;
        rowIDLabels[i]->Visible=false;
        rowCountLabels[i]->Visible=false;
        for(int j=0; j<8; j++)
        {
            socketEdits[i][j]->Text="";
            socketEdits[i][j]->Visible=false;
            colLabels[j]->Visible=false;
            socketPanels[i][j]->Visible=false;
            // golden also does `ClearCntButton[i][j]->Visible=false;` here --
            // ClearCntButton[][]/btnClearAa.. not declared, bucket (b)-only.
        }
    }

    for(int j=0; j<TestSocket.iShtCol; j++)                                     //JerryYang 20170206
    {
        colLabels[j]->Visible=true;
        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            socketEdits[i][j]->Visible=true;
            socketEdits[i][j]->Enabled=true;
            socketEdits[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
            socketPanels[i][j]->Visible=true;
            socketPanels[i][j]->Enabled=true;
            socketPanels[i][j]->Caption=AnsiString(LastSet.iSocketContactCount[i][j]);
            // golden also does `ClearCntButton[i][j]->Visible=true;` here.
            rowLabels[i]->Visible=true;
            rowIDLabels[i]->Visible=true;
            rowCountLabels[i]->Visible=true;
        }
    }

    palVibratorHP1->Caption=IniConfig.iVibratorHP1;                             //JerryYang 20200612 振動馬達作動時間累計
    palVibratorSht1->Caption=IniConfig.iVibratorSht1;
    palVibratorSht2->Caption=IniConfig.iVibratorSht2;
    palVibratorUnloader->Caption=IniConfig.iVibratorUnloader;

    edtInOutArmPickerAlmCnt->Text=TestIF_File.InOutArmLifeCntSet;               //JerryYang 20220331 : add吸嘴作動次數計數
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<4; j++)
        {
            edtInArmPickLifeCnt[i][j]->Text=TestIF_File.InArmPickerLifeCnt[i][j];
            edtOutArmPickLifeCnt[i][j]->Text=TestIF_File.OutArmPickerLifeCnt[i][j];
        }
    }

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            edtArm1PickLifeCnt[i][j]->Text=TestIF_File.Arm1PickerLifeCnt[i][j];
            edtArm2PickLifeCnt[i][j]->Text=TestIF_File.Arm2PickerLifeCnt[i][j];
        }
    }

    if(CUSTOMER_CODE==CC_SIGURD_HUKOU || CosFunction.bSetOffsetLimitToAll)      //KaiChen 20181206 ：Start Run Mode Only FT
    {
        if(CosFunction.bSetOffsetLimitToAll)                                    //JerryYang 20230215 : ASE-CL 尚智要求一鍵設定Offset limit
        {
            tsFunction->TabVisible=true;
            gbSetOffsetLimit->Visible=true;
        }
        else
        {
            gbSetOffsetLimit->Visible=false;
        }

        if(CUSTOMER_CODE==CC_SIGURD_HUKOU)
        {
            cbStartModeOnlyFT->Visible=true;
            if(LastSet.iRunStartMode==rsmInitialStart ||
                LastSet.iRunStartMode==rsmCInitialRetest)                       //KaiChen 20181211 ：增加卡關 RunStartMode
            {
                tsFunction->TabVisible=true;
            }
            else
            {
                tsFunction->TabVisible=false;
            }
        }
        else
        {
            cbStartModeOnlyFT->Visible=false;
        }
    }
    else
    {
        cbStartModeOnlyFT->Visible=false;
        tsFunction->TabVisible=false;
    }

    if(CosFunction.bCylinderOnOffTimeLog)                                       //JerryYang 20220215 : cylinder作動次數計數
    {
        tsSmartDiagnostic->TabVisible=true;
    }
    else
    {
        tsSmartDiagnostic->TabVisible=false;
    }

    if(CosFunction.bUseSocketContactCount)                                      //Sam 20220720 : 新增一組 Socket Count
    {
        sgSocketCount->Cells[1][0]=" Contact times for alarm";
        sgSocketCount->Cells[2][0]=" Current Status";
        sgSocketCount->Cells[3][0]=" Total Contact Count";

        for(int i=0; i<TestSocket.iShtRow; i++)
        {
            for(int j=0; j<TestSocket.iShtCol; j++)
            {
                str.sprintf("Socket %s Contact ", IndexSuckName[i][j]);
                sgSocketCount->Cells[0][1+(i*TestSocket.iShtCol+j)]=str;
                sgSocketCount->Cells[1][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(IniConfig.SocketContactSet[i][j]);
                sgSocketCount->Cells[2][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(IniConfig.SocketContactCount[i][j]);
                sgSocketCount->Cells[3][1+(i*TestSocket.iShtCol+j)]=" "+AnsiString(IniConfig.SocketContactCountHistory[i][j]);
            }
        }
    }

    if(CosFunction.bSetOffsetLimitToAll)                                        //JerryYang 20230215 : ASE-CL 尚智要求一鍵設定Offset limit
    {
        edtSetXYOffsetLimit->Text=InputLimit.iOffsetXYHigh;
        edtSetZOffsetLimit->Text=InputLimit.iOffsetZHigh;
    }

    fShow=true;
    // golden :534-535 `myLog.Do_Log(...)` is already `//`-commented-out in
    // golden itself -- GOLDEN ODDITY, not translated (matches golden's own
    // dead state, same convention as forms/fTemperFrom.h's
    // TempOverCannotRunShowAlarm precedent).
}

//---------------------------------------------------------------------------
//  DoIniDataToForm -- golden :1009-1027
//---------------------------------------------------------------------------
void TfStartCondition::DoIniDataToForm()
{
    TEdit  *socketEdits[4][8] = {
        {editSocketAa, editSocketAb, editSocketAc, editSocketAd, editSocketAe, editSocketAf, editSocketAg, editSocketAh},
        {editSocketBa, editSocketBb, editSocketBc, editSocketBd, editSocketBe, editSocketBf, editSocketBg, editSocketBh},
        {editSocketCa, editSocketCb, editSocketCc, editSocketCd, editSocketCe, editSocketCf, editSocketCg, editSocketCh},
        {editSocketDa, editSocketDb, editSocketDc, editSocketDd, editSocketDe, editSocketDf, editSocketDg, editSocketDh}};
    TPanel *socketPanels[4][8] = {
        {PanelAa, PanelAb, PanelAc, PanelAd, PanelAe, PanelAf, PanelAg, PanelAh},
        {PanelBa, PanelBb, PanelBc, PanelBd, PanelBe, PanelBf, PanelBg, PanelBh},
        {PanelCa, PanelCb, PanelCc, PanelCd, PanelCe, PanelCf, PanelCg, PanelCh},
        {PanelDa, PanelDb, PanelDc, PanelDd, PanelDe, PanelDf, PanelDg, PanelDh}};

    for(int i=0; i<TestSocket.iShtRow; i++)                                     //JerryYang 20170206
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            socketEdits[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
            socketPanels[i][j]->Caption=AnsiString(LastSet.iSocketContactCount[i][j]);
        }
    }

    for(int i=0; i<TestSocket.iMaxRow; i++)
    {
        for(int j=0; j<TestSocket.iMaxCol; j++)
        {
            fLotInfo->edSocket[i][j]->Text=AnsiString(LastSet.strSocketID[i][j]);
        }
    }
}

//---------------------------------------------------------------------------
//  pgLifeTimeChange -- golden :1029-1047
//  GATE (SC6): ENTIRE body compares `pgLifeTime->ActivePage==tsConditionNN` /
//  `==TabsSocketID` -- vclcompat::TPageControl has no pointer-valued
//  ActivePage. See file-head GATE register. Declared (not omitted) so it is
//  ready the moment a future wave adds that property.
//---------------------------------------------------------------------------
void TfStartCondition::pgLifeTimeChange()
{
#if 0
    if(pgLifeTime->ActivePage==tsCondition01)
    {
        editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[0]);
    }
    else if(pgLifeTime->ActivePage==tsCondition02)
    {
        editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[1]);
    }
    else if(pgLifeTime->ActivePage==tsCondition03)
    {
        editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[2]);
    }
    else if(pgLifeTime->ActivePage==TabsSocketID)                               //Steven 20200211 : Add socket count for ASE-CL
    {
        editContactCountAlarm->Text=AnsiString(TestIF_File.iContactAlarmCount[3]);
    }
#endif // GATE (SC6)
}

//---------------------------------------------------------------------------
//  GetCylinderIndexByName -- golden :1479-1487
//---------------------------------------------------------------------------
int TfStartCondition::GetCylinderIndexByName(AnsiString sCylinderName)
{
    for(int i=0; i<MaxCylinderItem; i++)
    {
        if(sCylinderName==Cylinder[i].CylinderName)
            return i;
    }
    return -1;
}

//---------------------------------------------------------------------------
//  UpdateCylinderScreen -- golden :1489-1589
//  GATE (SC4): golden :1565-1572's self-healing negative-count clamp +
//  SaveCylinderLife() call is #if 0-gated below (a write hidden inside a
//  function named "UpdateScreen"). See file-head GATE register.
//---------------------------------------------------------------------------
void TfStartCondition::UpdateCylinderScreen(bool bRefrash)
{
    static bool flag=true;
    int iRow=0;
    AnsiString s1="",s2="";
    (void)bRefrash;   // golden itself never reads this parameter in the body either
    if(SystemInitialOK==false)
        return;

    if(fShow==false)
        return;

    if(flag)
    {
        flag=false;
        strngrdCylinderView->ColWidths[eCylName]            =140;
        strngrdCylinderView->ColWidths[eCylOnOffCnt]        =70;
        strngrdCylinderView->ColWidths[eCylOnTime]          =60;
        strngrdCylinderView->ColWidths[eCylOffTime]         =60;
        strngrdCylinderView->ColWidths[eCylAvgOnTime]       =70;
        strngrdCylinderView->ColWidths[eCylAvgOffTime]      =70;
        strngrdCylinderView->ColWidths[eCylTimeOutCnt]      =70;
        strngrdCylinderView->ColWidths[eCylOnOffCntAlarm]   =80;
        strngrdCylinderView->ColWidths[eCylOnTimeAlarm]     =70;
        strngrdCylinderView->ColWidths[eCylOffTimeAlarm]    =70;
        strngrdCylinderView->ColWidths[eCylTimeOutCntAlarm] =70;
        strngrdCylinderView->ColWidths[eCylResetCount]      =70;
        strngrdCylinderView->ColWidths[eCylResetTime]       =120;

        if(LastSet.iLanguageCountry==1)
        {
            sCylinderData[eCylName][0]              ="氣缸名稱";
            sCylinderData[eCylOnOffCnt][0]          ="開關次數";
            sCylinderData[eCylOnTime][0]            ="開時間";
            sCylinderData[eCylOffTime][0]           ="關時間";
            sCylinderData[eCylAvgOnTime][0]         ="平均.開時間";
            sCylinderData[eCylAvgOffTime][0]        ="平均.關時間";
            sCylinderData[eCylTimeOutCnt][0]        ="逾時次數";
            sCylinderData[eCylOnOffCntAlarm][0]     ="開關次數警報";
            sCylinderData[eCylOnTimeAlarm][0]       ="開時間警報";
            sCylinderData[eCylOffTimeAlarm][0]      ="關時間警報";
            sCylinderData[eCylTimeOutCntAlarm][0]   ="關時間警報";
            sCylinderData[eCylResetCount][0]        ="重置次數";
            sCylinderData[eCylResetTime][0]         ="重置時間";
        }
        else
        {
            sCylinderData[eCylName][0]              ="Cylinder Name";
            sCylinderData[eCylOnOffCnt][0]          ="OnOff Cnt.";
            sCylinderData[eCylOnTime][0]            ="On T.";
            sCylinderData[eCylOffTime][0]           ="Off T.";
            sCylinderData[eCylAvgOnTime][0]         ="Avg. On T.";
            sCylinderData[eCylAvgOffTime][0]        ="Avg. Off T.";
            sCylinderData[eCylTimeOutCnt][0]        ="TimeOut Cnt.";
            sCylinderData[eCylOnOffCntAlarm][0]     ="OnOff cnt. alarm";
            sCylinderData[eCylOnTimeAlarm][0]       ="On T. alarm";
            sCylinderData[eCylOffTimeAlarm][0]      ="Off T. alarm";
            sCylinderData[eCylTimeOutCntAlarm][0]   ="TimeOut Cnt. alarm";
            sCylinderData[eCylResetCount][0]        ="Reset Count";
            sCylinderData[eCylResetTime][0]         ="Reset T.";
        }
    }

    flag=false;
    static int ct=0;
    if(ct++<10)
        return;
    ct=0;

    iRow=1;
    for(int i=0; i<MaxCylinderItem; i++)
    {
        if(Cylinder[i].Enable)
        {
            sCylinderData[eCylName][iRow]           =Cylinder[i].CylinderName;
            // GATE (SC4): self-healing negative-count clamp + persist.
            #if 0
            if(Cylinder[i].iOnOffCount<0)   //超出變數極限變負值
            {
                s1.sprintf("%s clear PreAlarm count by over",Cylinder[i].CylinderName);
                s2.sprintf("OnOff=%d",Cylinder[i].iOnOffCount);
                NewRecordProcess("", s1, s2);
                Cylinder[i].iOnOffCount=0;
                SaveCylinderLife();
            }
            #else
            (void)s1; (void)s2;   // golden odd-int-underflow display kept verbatim below, see GATE (SC4)
            #endif
            sCylinderData[eCylOnOffCnt][iRow]       =Cylinder[i].iOnOffCount;
            sCylinderData[eCylOnTime][iRow]         =Cylinder[i].GetOnTime();
            sCylinderData[eCylOffTime][iRow]        =Cylinder[i].GetOffTime();
            sCylinderData[eCylAvgOnTime][iRow]      =Cylinder[i].GetOnTimeAvg();
            sCylinderData[eCylAvgOffTime][iRow]     =Cylinder[i].GetOffTimeAvg();
            sCylinderData[eCylTimeOutCnt][iRow]     =Cylinder[i].iTimeOutCount;
            sCylinderData[eCylOnOffCntAlarm][iRow]  =Cylinder[i].iOnOffCountAlarm;
            sCylinderData[eCylOnTimeAlarm][iRow]    =Cylinder[i].GetOnTimeAlarm();
            sCylinderData[eCylOffTimeAlarm][iRow]   =Cylinder[i].GetOffTimeAlarm();
            sCylinderData[eCylTimeOutCntAlarm][iRow]=Cylinder[i].iTimeOutCountAlarm;
            sCylinderData[eCylResetCount][iRow]     =Cylinder[i].iResetCount;
            sCylinderData[eCylResetTime][iRow]      =Cylinder[i].sResetTime;
            iRow++;
        }
    }
    strngrdCylinderView->Refresh();   // GATE (SC5) sibling no-op, see fStartCondition.h
}

//---------------------------------------------------------------------------
//  LoadCylinderLife -- golden :1591-1653
//  DEVIATION: golden's own `fStartCondition->strngrdCylinderView` self-
//  reference (:1649-1651) is translated using the plain unqualified
//  `strngrdCylinderView` member -- see fStartCondition.h's
//  INTEGRATION-PENDING note (no live `fStartCondition` global stood up yet).
//---------------------------------------------------------------------------
void TfStartCondition::LoadCylinderLife()
{
    static bool bfirst=true;
    AnsiString s="";
    int iRow=0;
    double dOnTime=0.0,dOffTime=0.0;
    for(int i=0; i<MaxCylinderItem; i++)
    {
        if(Cylinder[i].Enable)
        {
            //Life
            s.sprintf("%s_OnOffCount",Cylinder[i].CylinderName);
            Cylinder[i].iOnOffCount=ReadIniData(asMachineLifePath, "CylinderLife",s,0);
            s.sprintf("%s_TimeOutCount",Cylinder[i].CylinderName);
            Cylinder[i].iTimeOutCount=ReadIniData(asMachineLifePath, "CylinderLife",s,0);

            //Alarm
            s.sprintf("%s_OnOffCountAlarm",Cylinder[i].CylinderName);
            Cylinder[i].iOnOffCountAlarm=ReadWriteIni(asMachineLifePath, "CylinderAlarm",s,3000000,true,true,3000000,5000000);
            s.sprintf("%s_OnTimeAlarm",Cylinder[i].CylinderName);
            Cylinder[i].dOnTimeAlarm=ReadIniData(asMachineLifePath, "CylinderAlarm",s,999.99);
            s.sprintf("%s_OffTimeAlarm",Cylinder[i].CylinderName);
            Cylinder[i].dOffTimeAlarm=ReadIniData(asMachineLifePath, "CylinderAlarm",s,999.99);
            s.sprintf("%s_TimeOutCountAlarm",Cylinder[i].CylinderName);
            Cylinder[i].iTimeOutCountAlarm=ReadWriteIni(asMachineLifePath, "CylinderAlarm",s,1000,true,true,1000,10000);

            //Reset Time
            s.sprintf("%s_ResetCount",Cylinder[i].CylinderName);
            Cylinder[i].iResetCount=ReadWriteIni(asMachineLifePath, "CylinderResetCount",s,0,0,true,0,1000);
            s.sprintf("%s_ResetTime",Cylinder[i].CylinderName);
            Cylinder[i].sResetTime=ReadIniData(asMachineLifePath, "CylinderResetTime",s,FormatDateTime("yyyy-mm-dd hh:nn:ss", Now()));   // DEVIATION: golden `Now().FormatString(...)` -- vclcompat::TDateTime has no FormatString method; FormatDateTime(fmt,dt) is the ported equivalent (same substitution already used by cContactCT.cpp:1381)

            //On/Off Time Record
            for(int j=0; j<5 ;j++)
            {
                s.sprintf("%s_OnTime_%d",Cylinder[i].CylinderName,j);
                dOnTime=ReadIniData(asMachineLifePath, "OnTimeRecord",s,0.0);
                if(dOnTime!=0.0)
                    Cylinder[i].AddOnTime(dOnTime);

                s.sprintf("%s_OffTime_%d",Cylinder[i].CylinderName,j);
                dOffTime=ReadIniData(asMachineLifePath, "OffTimeRecord",s,0.0);
                if(dOffTime!=0.0)
                    Cylinder[i].AddOffTime(dOffTime);

                if(j==0)
                {
                    Cylinder[i].dOnTime=dOnTime;
                    Cylinder[i].dOffTime=dOffTime;
                }
            }
            iRow++;
        }
    }

    if(bfirst)
    {
        bfirst=false;
        strngrdCylinderView->RowCount=iRow+1;
        strngrdCylinderView->Height  =20*iRow+2;
        strngrdCylinderView->ColCount=eCylPrAlarmItemTotal;
    }
}

//---------------------------------------------------------------------------
//  strngrdCylinderViewDrawCell -- golden :1418-1450
//  GATE (SC5): ENTIRE body -- the BrushColor/FontColor decision has no
//  consumer besides MyDrawText(pCanvasCyl,...), and vclcompat has no
//  TCanvas anywhere. Same class as forms/fContactCT.h's GATE (C1) /
//  forms/fYieldMonitoring.h's GATE (Y3). Declared with trimmed (ACol, ARow)
//  signature (Sender/Rect/State dropped, same DEVIATION as
//  fContactCT.h's sgYieldDrawCell) so it is ready the moment a TCanvas lands.
//---------------------------------------------------------------------------
void TfStartCondition::strngrdCylinderViewDrawCell(int ACol, int ARow)
{
#if 0
    AnsiString s="";
    TColor BrushColor=clBtnFace;
    TColor FontColor=clBlack;
    pCanvasCyl->Handle=PtrDCCyl;
    pCanvasCyl->Font->Style = TFontStyles() << fsBold;
    s=sCylinderData[ACol][ARow];
    if(ARow==0 || ACol==0)  //標題
    {
        if(ACol>=eCylOnOffCntAlarm)
            BrushColor=clAppWorkSpace;      //Alarm Setting 用其他顏色
        else
            BrushColor=clBtnFace;
    }
    else
    {
        if((ACol==eCylOnOffCnt   && atoi(sCylinderData[eCylOnOffCnt][ARow].c_str())   > atoi(sCylinderData[eCylOnOffCntAlarm][ARow].c_str())) ||
           (ACol==eCylAvgOnTime  && atof(sCylinderData[eCylAvgOnTime][ARow].c_str())  > atof(sCylinderData[eCylOnTimeAlarm][ARow].c_str()))   ||
           (ACol==eCylAvgOffTime && atof(sCylinderData[eCylAvgOffTime][ARow].c_str()) > atof(sCylinderData[eCylOffTimeAlarm][ARow].c_str()))  ||
           (ACol==eCylTimeOutCnt && atoi(sCylinderData[eCylTimeOutCnt][ARow].c_str()) > atoi(sCylinderData[eCylTimeOutCntAlarm][ARow].c_str())))
        {
            BrushColor=clYellow;    //超出異常範圍顯示黃色
        }
        else
        {
            BrushColor=clWhite;
        }
    }
    MyDrawText(pCanvasCyl, Rect, s.c_str(), BrushColor,FontColor);
#else
    (void)ACol; (void)ARow;   // GATE (SC5) -- no TCanvas, see banner
#endif
}
