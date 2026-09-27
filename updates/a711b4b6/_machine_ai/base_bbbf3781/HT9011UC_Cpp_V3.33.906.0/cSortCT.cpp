// =============================================================================
//  cSortCT.cpp  --  golden TfSortCT 會碰 ht9045_sm 的四支方法（顯示與清除）
//
//  Steven 20260925 (Data.SortCT)。Steven 20260925 授權：遇問題依 golden 原碼與經驗自行決斷。
//  golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy/cSortCT.cpp（1,956 行，cp950；行號一律 V912）。
//    解碼：iconv -f cp950 -t utf-8（scratchpad/sortct/cSortCT.cpp.utf8），0 個錯誤。
//
//  本檔翻四支（逐行照 golden，缺的符號 GATE 並註記）：
//    W906Body_ShowLoadingIC       golden :210-279
//    W906Body_ShowLoadingIC_ART   golden :281-343
//    W906Body_ShowSortIC          golden :345-442
//    W906Body_btnClearCountClick  golden :585-708
//  其餘（建構子、_MyCountPanel、FormShow、UpForm、lblTotalClick）只碰 ht9045_globals，在 forms/fSortCT.cpp；
//  沒翻的方法與理由見 forms/fSortCT.cpp 檔頭。
//
//  接法：forms/fSortCT.cpp 的四個 virtual 方法是跳板，查 g_W906_SortCTBodies。本檔在
//    (a) 靜態初始化（s_w906SortCTAutoInstall）與 (b) W906_SortCTInstall()（WebBridgeTags.cpp 每拍呼叫）
//  兩處填入。(b) 同時是**連結錨點**：static archive 的成員只有在有人引用它的符號時才會被抽出
//  （KNOWLEDGE「build 綠證明不了接上了」第 1 種假象），WebBridgeTags.cpp 直接呼叫 W906_SortCTInstall()，
//  所以凡是編 WebBridgeTags.cpp 的執行檔（wb_serve、test_wb_tags）一定連進本檔、本體一定生效。
//
//  GATE 一覽（全部是移植樹缺元件／缺方法；會不會在這台機器走到，逐條寫在各 GATE 處）：
//    G1  ShowSortIC :356/:362  TrayCTPanel[i]／TrayCTART[i]->Caption —— golden 從未指派（NULL），VCL 對 NULL 控制項
//        寫 Caption 是靜默 no-op（TControl.Perform 有 `if Self <> nil`）；C++ 解參考 NULL 會當，所以不執行、只註記。
//    G2  ShowSortIC :407-408   NUMBER_PANEL_TYPE==4 的 HSys.BinDisCtrl->WriteTargetCount —— TMyBinDispCtrl 在本樹只有前置宣告
//        （database.h:303，NULL）。本機 Gerneral.ini NUMBER_PANEL_TYPE=3，走不到。
//    G3  ShowSortIC :411-417   fShowBinSelect->IsPTIRotateFix1Display() —— 力成（CC_PTI）V912 新增的方法，移植樹沒有。客戶專屬。
//    G4  ShowSortIC :425-428   fShowBinSelect->IndexInput／IndexOut／OutArm_input／labInArm_input —— facade 沒有這四個元件（純顯示）。
//    G5  ShowSortIC :439-440   fLotInfo->labLowYieldICCount —— facade 沒有（CosFunction.bLowYieldUseContactCounts 才走）。
//    G6  btnClearCount :612-634  CC_KYEC_LEE 強制登出重登（fMain->cbUserSelect／stOperatorClick…）—— 客戶專屬；
//        本樹沒有登入對話框，照 golden 的結果（登出後 AccessLevel=0 < Supervisor）等同 return，所以 KYEC 一律拒絕。
//    G7  btnClearCount :653-660、:698-701  OEE（CosFunction.bOEEFunction）：EachCycleSecondDo_SaveAndUpdateOEEFiles 不在 facade；
//        ClearOEECount 有，照翻。CosFunction 在 wb_serve 沒載入（WebBridgeTags.cpp CosFunctionLoaded()），兩支都走不到。
//    G8  btnClearCount :694     fContactCT->sgYield->Refresh() —— VCL 重繪；照 MainClarnData.h 偏離 D1 不翻（畫面由 tag 更新）。
//    G9  btnClearCount :695-696 fSCKART->bShow／UpdateCount() —— facade 沒有（SCK 93K ART 視窗開著才走）。
//    G10 btnClearCount :704-707 fBarCode->InitBarcodeRecFile() —— BarCode.h facade 沒有（CosFunction.bBarcodeTrayRecFile 才走）。
//    G11 btnClearCount :637     ShowMyMessageBox_YES_NO —— 本樹沒有 golden 的確認框；換成網頁兩段式確認（見 W906_Confirm*）。
//        golden 那一支會留操作紀錄（mymessbox，AI(ht9045-v899) 20260605 的理由）；本樹的留痕入口本來就不落地
//        （porting-gaps.md 六），這裡不另外補。
// =============================================================================
#include "forms/fSortCT.h"

#include "vclcompat/vcl_compat.h"
#include "vclcompat/SysUtils.h"     // FormatFloat / IntToStr
#include "MachineType.h"            // eTrayCount / e6TrayName / FT RT / rsm* / CC_* / eartInstall / tNotUse / tCID_NFC / pbtContinualLoader / ChangeToPercentage / ChangeToFloat
#include "cmydef.h"                 // iRunStartMode / iWhoTriggerPiggyBack / MMInterface / iTo3Unload / iSECSGEM* / iATR* / ASE_Yield / s6TrayName / fTrayYield / iallSitCount / USE_* / SystemStart
#include "cprod.h"                  // Prod / TestIF / TestIF_File / RunInfo / TrayForm
#include "LastSet.h"                // LastSet（不在 cprod.h）
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "canary_support.h"         // ShowErrorMessage（golden note.h:466）
// 不 include cMyDB.h：它與 canary_support.h 的 RecordProcess／MyDBIProcessNew 預設引數互相重宣告、
// 且 3 參數 MyDBIProcess 與 aHotPlateSubstrate.h:979 的 2 參數版會讓兩參數呼叫模稜兩可（實測 g++ -fsyntax-only）。
// 照 uYieldMonitoring.cpp:160-164 的做法：MyDBIProductionData 單獨前置宣告；MyDBIProcess 用 aHotPlateSubstrate.h 的 2 參數版
// （3 參數版 uHGemEquipment.cpp:3483 本來也只是轉發到它，ChanAction.cpp:207-227 量過）—— 兩條路落點相同。
extern void __fastcall MyDBIProductionData(AnsiString sAction);            // golden cMyDB.h:63 -- def cMyDB.cpp:642
#include "cSocket.h"                // ArmData[3]（GetByBinLowYieldPCA / GetTotalCT）
#include "aHotPlateSubstrate.h"     // InArmSuck / OutArmSuck —— TMyKitSuck 用這一個標頭（KNOWLEDGE：兩個 TMyKitSuck，選錯會讀錯偏移）
#include "Motor/mymotor.h"          // MOT[]（MInRotateKit / MOutRotateKit 的 HasIC）
#include "csystem.h"                // HasICUnderMachine / ShuttleHasIC / IndexHasIC
#include "atester_ProcessCount.h"   // ProcessPiggyBackFunction
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT
#include "SECSGEM/SecsEventReport.h"// EventReport
#include "forms/fMain.h"            // fMain->Clarn_Data / lblAuto1..6TrayCnt
#include "forms/fLotInfo.h"         // fLotInfo->sgATRCount / ShowAMRCategoryBin
#include "forms/fShowBinSelect.h"   // fShowBinSelect->ShowCategoryBin
#include "forms/fSecurity.h"        // fSecurity->Insufficient
#include "forms/fProductionInfo.h"  // fProductionInfo->ClearOEECount

//---------------------------------------------------------------------------
//  G11：網頁兩段式確認（port-only）。
//  golden :637 `ShowMyMessageBox_YES_NO("Clear Sort Count?", "確定要清空計數？")` 是一個模態框。
//  本樹沒有它，而 wb_serve 單執行緒不能在命令裡等對話框（tools/wb_serve.cpp act.* 分派註解）。所以：
//    第 1 段（詢問）：s_w906ConfirmAnswer=0 → 本體跑 golden 的守衛直到這一行，記下「走到確認框」後當作 NO 返回；
//    第 2 段（確認）：瀏覽器照 golden 字樣問完、使用者按 YES 才送 → s_w906ConfirmAnswer=1 → 本體重跑守衛、這一行回 YES。
//  守衛在兩段都照 golden 跑（不信任前端：指令進 handler 後重新過檢查）。程式呼叫（Lot End／SECS／MES 傳 Sender!=btnClearCount）
//  golden 本來就不走確認框（:635 bManualClear），本機制不影響它們。
//---------------------------------------------------------------------------
namespace {
int  s_w906ConfirmAnswer  = 0;      // 1 = YES（golden ShowMyMessageBox_YES_NO 回 1）；其他 = NO
bool s_w906ConfirmReached = false;
int  s_w906ClearExit      = 0;      // 本體從哪一個 return 出去（W906_SortCTClearExit 的值）；純觀測，不影響流程
}

enum W906_SortCTClearExit {
    kSortClrNone = 0, kSortClrSystemStart, kSortClrNotAuthorized, kSortClrTsmcIcInMachine, kSortClrAseClIcInMachine,
    kSortClrKyecReauth, kSortClrConfirmNo, kSortClrSystemStartAfterConfirm, kSortClrDone
};

static int W906_ShowMyMessageBox_YES_NO(AnsiString /*S1*/, AnsiString /*S2*/)
{
    s_w906ConfirmReached = true;
    return (s_w906ConfirmAnswer == 1) ? 1 : 0;
}
#define ShowMyMessageBox_YES_NO W906_ShowMyMessageBox_YES_NO

//---------------------------------------------------------------------------
//  ShowLoadingIC -- golden cSortCT.cpp:210-279（逐行）
//---------------------------------------------------------------------------
void TfSortCT::W906Body_ShowLoadingIC()
{
    bool bContinuousLoaderAlarm=false;
    pnlLoader->Caption=LastSet.SendCT[0];

    if(iRunStartMode==FT)                                                       //jou 2010-11-17 start :Piggy-Back Functions
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //Steven 20110506 : Auto Site Mapping時不啟動
        {
            LastSet.SendCT[2]=0;
            if((USE_AUTO_RETEST==eartInstall &&
                (bAutoReTest_ART ||
                 IniConfig.bA10_AutoReTest)) ||
                 CosFunction.bUseARTSortCount)                                  //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
            {
                LastSet.SendCT_ART[2]=0;
            }
        }
        else
        {
            if(TestIF.bContinuousLoader==true)
            {
                if(LastSet.SendCT[2]>=int(TestIF.iContinuousLoaderCount))
                {
                    LastSet.SendCT[2]=0;                                        //20141001 wei add
                    if(Prod.iCountAlarmAction==0)                               //20141001 wei add
                    {
                        bContinuousLoaderAlarm=true;
                    }
                    else
                    {
                        iWhoTriggerPiggyBack=pbtContinualLoader;                //Steven 20111207 : 誰觸發了Piggy Back
                        ProcessPiggyBackFunction();                             //Steven 20110725 : 整合成function
                    }
                }
            }
        }
    }
    else if(iRunStartMode==RT)
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //Steven 20110506 : Auto Site Mapping時不啟動
        {
            LastSet.SendCT[2]=0;
        }
        else
        {
            if(TestIF.bContinuousLoader_RT==true)
            {
                if(LastSet.SendCT[2]>=int(TestIF.iContinuousLoaderCount_RT))
                {
                    LastSet.SendCT[2]=0;                                        //20141001 wei add
                    if(Prod.iCountAlarmAction==0)                               //20141001 wei add
                    {
                        bContinuousLoaderAlarm=true;
                    }
                    else
                    {
                        iWhoTriggerPiggyBack=pbtContinualLoader;                //Steven 20111207 : 誰觸發了Piggy Back
                        ProcessPiggyBackFunction();                             //Steven 20110725 : 整合成function
                    }
                }
            }
        }
    }

    if(bContinuousLoaderAlarm)                                                  //20141001 wei add
    {
        ShowErrorMessage("WAR07324", 0, MMInterface, false);
    }
}

//---------------------------------------------------------------------------
//  ShowLoadingIC_ART -- golden cSortCT.cpp:281-343（逐行）
//---------------------------------------------------------------------------
void TfSortCT::W906Body_ShowLoadingIC_ART()
{
    bool bContinuousLoaderAlarm=false;
    pnlLoadingART->Caption=LastSet.SendCT_ART[0];

    if(iRunStartMode==FT)                                                       //jou 2010-11-17 start :Piggy-Back Functions
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //Steven 20110506 : Auto Site Mapping時不啟動
        {
           LastSet.SendCT_ART[2]=0;
        }
        else
        {
            if(TestIF.bContinuousLoader==true)
            {
                if(LastSet.SendCT_ART[2]>=int(TestIF.iContinuousLoaderCount))
                {
                    LastSet.SendCT_ART[2]=0;                                    //20141001 wei add
                    if(Prod.iCountAlarmAction==0)                               //20141001 wei add
                    {
                        bContinuousLoaderAlarm=true;
                    }
                    else
                    {
                        iWhoTriggerPiggyBack=pbtContinualLoader;                //Steven 20111207 : 誰觸發了Piggy Back
                        ProcessPiggyBackFunction();                             //Steven 20110725 : 整合成function
                    }
                }
            }
        }
    }
    else if(iRunStartMode==RT)
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //Steven 20110506 : Auto Site Mapping時不啟動
        {
            LastSet.SendCT_ART[2]=0;
        }
        else
        {
            if(TestIF.bContinuousLoader_RT==true)
            {
                if(LastSet.SendCT_ART[2]>=int(TestIF.iContinuousLoaderCount_RT))
                {
                    LastSet.SendCT_ART[2]=0;                                    //20141001 wei add
                    if(Prod.iCountAlarmAction==0)                               //20141001 wei add
                    {
                        bContinuousLoaderAlarm=true;
                    }
                    else
                    {
                        iWhoTriggerPiggyBack=pbtContinualLoader;                //Steven 20111207 : 誰觸發了Piggy Back
                        ProcessPiggyBackFunction();                             //Steven 20110725 : 整合成function
                    }
                }
            }
        }
    }

    if(bContinuousLoaderAlarm)                                                  //20141001 wei add
    {
        ShowErrorMessage("WAR07324", 0, MMInterface, false);
    }
}

//---------------------------------------------------------------------------
//  ShowSortIC -- golden cSortCT.cpp:345-442（逐行，GATE G1-G5）
//---------------------------------------------------------------------------
void TfSortCT::W906Body_ShowSortIC()
{
    unsigned int Sum=0, Sum_ART=0;
    double Sum2=0.0;
    int ipass=0, iFail=0;                                                       //wei 20160923 Secs Gem 回傳Pass/Fail顆數
    int ipass_ATR=0, iFail_ATR=0;                                               //wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數

    for(int i=0; i<eTrayCount; i++)                                             //JerryYang 20220909 : 10->eTrayCount  //kevin 20160819  //kevin 20110901使用FIX分2TRAY
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            // GATE G1（golden :356）`TrayCTPanel[i]->Caption=LastSet.BinCT[0][iTo3Unload[i]];` —— TrayCTPanel[] 在 golden 從未指派
            //   （forms/fSortCT.h private 區的註解），VCL 對 NULL 控制項寫 Caption 是靜默 no-op；這裡不執行（C++ 會解參考 NULL）。
            if(TrayCTPanel[i]!=NULL) TrayCTPanel[i]->Caption=LastSet.BinCT[0][iTo3Unload[i]];   // 恆為 NULL ⇒ 永不執行，與 golden 可觀察結果相同
            Sum+=LastSet.BinCT[0][iTo3Unload[i]];
            if((USE_AUTO_RETEST==eartInstall &&
               (bAutoReTest_ART || IniConfig.bA10_AutoReTest)) ||
                CosFunction.bUseARTSortCount)                                   //Ifor 20170315 (wei) add 新增使用ART Sort Count 計數功能
            {
                if(TrayCTART[i]!=NULL) TrayCTART[i]->Caption=LastSet.BinCT_ART[0][iTo3Unload[i]];   // GATE G1（golden :362），同上
                Sum_ART  +=LastSet.BinCT_ART[0][iTo3Unload[i]];
            }

            if(Prod.iIsPassT6[i]==1)                                            //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6     //wei 20160923 Secs Gem 回傳Pass/Fail顆數
            {
                ipass    +=LastSet.BinCT    [0][iTo3Unload[i]];
                ipass_ATR+=LastSet.BinCT_ART[0][iTo3Unload[i]];                 //wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
            }
            else
            {
                iFail    +=LastSet.BinCT    [0][iTo3Unload[i]];
                iFail_ATR+=LastSet.BinCT_ART[0][iTo3Unload[i]];                 //wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
            }
        }
    }
    RunInfo.iUnloadCount=Sum;
    RunInfo.iUnloadCount_ART=Sum_ART;                                           //kevin 20150615 ART
    iSECSGEMPass=ipass;                                                         //wei 20160923 Secs Gem 回傳Pass/Fail顆數
    iSECSGEMFail=iFail;                                                         //wei 20160923 Secs Gem 回傳Pass/Fail顆數

    iATRPassCount[iATRFtRtMode]=ipass_ATR;                                      //wei 20170119 (Steven) TSMC Secs Gem 回傳Pass/Fail顆數
    iATRFailCount[iATRFtRtMode]=iFail_ATR;
    iATRTotalCount[iATRFtRtMode]=Sum_ART;
    fLotInfo->sgATRCount->Cells[iATRFtRtMode+1][1]=iATRPassCount[iATRFtRtMode];
    fLotInfo->sgATRCount->Cells[iATRFtRtMode+1][2]=iATRFailCount[iATRFtRtMode];
    fLotInfo->sgATRCount->Cells[iATRFtRtMode+1][3]=iATRTotalCount[iATRFtRtMode];

    AnsiString AYield="";                                                       //kevin 20170816 (Steven) add 傳送YIELD 給ASE
    ASE_Yield[0]="";

    for(int i=0; i<eTrayCount; i++)                                             //JerryYang 20220909 : 10->eTrayCount  //kevin 20110901使用FIX分2TRAY
    {
        if(Prod.iTrayType[i]!=tNotUse)
        {
            RunInfo.sT6AutoYield[i]             =ChangeToPercentage(LastSet.BinCT[0][iTo3Unload[i]], Sum);
            myCountPanel[i].pnlYield->Caption   =RunInfo.sT6AutoYield[i];
            myCountPanel[i].pnlCount->Caption   =LastSet.BinCT[0][iTo3Unload[i]];                                       //Steven 20230108 : fixed display
            fTrayYield[i]                       =ChangeToFloat(LastSet.BinCT[0][iTo3Unload[i]], Sum);
            RunInfo.sT6AutoYield_ART[i]         =ChangeToPercentage(LastSet.BinCT_ART[0][iTo3Unload[i]], Sum_ART);
            myCountPanel[i].pnlYieldART->Caption=RunInfo.sT6AutoYield_ART[i];                                           //kevin 20150615 ART
            myCountPanel[i].pnlCountART->Caption=LastSet.BinCT_ART[0][iTo3Unload[i]];                                   //Steven 20230108 : fixed display

            AYield.sprintf("%s=%s,", s6TrayName[i], myCountPanel[i].pnlYield->Caption);                                 //kevin 20170816 (Steven) add 傳送YIELD 給ASE
            ASE_Yield[0]+=AYield;                                               //kevin 20170816 (Steven) add 傳送YIELD 給ASE
            // GATE G2（golden :407-408，NUMBER_PANEL_TYPE==4 才走；本機 =3）：
            //   HSys.BinDisCtrl->WriteTargetCount(iTo3Unload[i]+3, LastSet.BinCT[0][iTo3Unload[i]]);
            //   TMyBinDispCtrl 在本樹只有前置宣告、HSys.BinDisCtrl 恆 NULL（database.h:303）—— 呼叫會當。
        }
        //AI(ht9045-v912) 20260923: CASE-PTI-20260923-002 力成被 rotate 鎖住的 Fix1 仍比照 V899.37 顯示百分比與數量(只寫畫面，不計入總數、不送 ASE/Bin 顯示器)
        // GATE G3（golden :411-417，客戶專屬 CC_PTI）：`else if(i==eFix1 && fShowBinSelect->IsPTIRotateFix1Display())` ——
        //   IsPTIRotateFix1Display 是 V912 20260923 新增的 TfShowBinSelect 方法，移植樹沒有。只寫畫面，不影響任何計數。
    }

    pnlTotal->Caption   =RunInfo.iUnloadCount;
    pnlTotalART->Caption=RunInfo.iUnloadCount_ART;

    fShowBinSelect->ShowCategoryBin();

    // GATE G4（golden :425-428，純顯示）：fShowBinSelect->IndexInput／IndexOut／OutArm_input／labInArm_input->Caption
    //   =LastSet.iIndexInputOutPut[0..3] —— forms/fShowBinSelect.h 沒有這四個元件。

    if(TestIF_File.bLowYieldAlarmByBin)
    {
        Sum2 =ArmData[0]->GetByBinLowYieldPCA();
        Sum2+=ArmData[1]->GetByBinLowYieldPCA();

        pnlYield->Caption   =FormatFloat("0.00%", Sum2/2.0);                    //Steven 20141125
        pnlYieldART->Caption=FormatFloat("0.00%", Sum2/2.0);                    //kevin 20150615
    }

    if(CosFunction.bLowYieldUseContactCounts)                                   //Sam 20221020 : LowYield 改使用 ContactCounts 的資料來計算
    {
        // GATE G5（golden :440）：fLotInfo->labLowYieldICCount->Caption=IntToStr(ArmData[2]->GetTotalCT()); —— facade 沒有這個元件。
    }
    fLotInfo->ShowAMRCategoryBin();                                             //Sam 20240304 : 新增 AMR 功能
}

//---------------------------------------------------------------------------
//  btnClearCountClick -- golden cSortCT.cpp:585-708（逐行，GATE G6-G11）
//  寫檔：只有 fMain->Clarn_Data(8) 那一條（JsonBridge/actions/MainClarnData.h 檔頭）：
//    D:\HT9045\system\lastdata.dat、lastdata_backup.dat（WriteLastDataFile），
//    D:\HT9045_Log\QtyData\YYYYMM\<PC>_<date>.csv（QtyLog；本行程第 2 次 Clarn_Data 起才寫）。
//  s_w906ClearExit 的指定是 port-only 觀測（標 [W906]），不改變流程。
//---------------------------------------------------------------------------
void TfSortCT::W906Body_btnClearCountClick(TObject *Sender)
{
    s_w906ClearExit=kSortClrNone;                                               // [W906]
    if(SystemStart)
    {
        s_w906ClearExit=kSortClrSystemStart;                                    // [W906]
        return;
    }

    //AI(ht9045-v899) 20260605: Sender==btnClearCount 為人工按鈕觸發,程式呼叫(Lot End/SECS/MES)傳入其他物件,後續清除提示僅人工觸發才跳
    bool bManualClear=(Sender==btnClearCount);

    if(CUSTOMER_CODE!=CC_Greatek)                                               //Sam 201700915 (Steven) : 超豐清除不用權限
    {
        if(fSecurity->Insufficient(108)==false)                                 //wei 20151022 Bin Clean Count權限設定
        {
            s_w906ClearExit=kSortClrNotAuthorized;                              // [W906]
            return;
        }
    }

    if(CUSTOMER_CODE==CC_TSMC_TAINAN && HasICUnderMachine()==true)              //wei 20160923 機台內有IC不能清除 Count
    {
        s_w906ClearExit=kSortClrTsmcIcInMachine;                                // [W906]
        return;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //JerryYang 20250120 : add
    {
        if(InArmSuck.HasIC()          || OutArmSuck.HasIC()         ||
           ShuttleHasIC()             || IndexHasIC()               ||
           MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC())
        {
            s_w906ClearExit=kSortClrAseClIcInMachine;                           // [W906]
            return;
        }
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20191008 : add KYEC 清除 Count 需刷Barcode & 權限
    {
        // GATE G6（golden :614-633，客戶專屬）：ReEnterBarcode[0]=false; fMain->cbUserSelect->ItemIndex=0; AccessLevel=0;
        //   fMain->ChangeLevelAttr(); ...->Caption="Operator"; if(FileExists(pwPath)) fMain->cbUserSelectChange(NULL);
        //   else fMain->stOperatorClick(fMain);  —— 強制登出再叫出登入框（刷 Barcode）。本樹沒有登入對話框，
        //   golden 在「沒有重新登入」時的結果是 AccessLevel=0 < iDefSupervisorLevel ⇒ 下面那一行 return。
        //   為了不在 web 上默默登出所有人，這裡不做登出，只保留那個結果：KYEC 一律 return。
        s_w906ClearExit=kSortClrKyecReauth;                                     // [W906]
        return;                                                                 // golden :630-633 的結果（見上）
    }
    else if(bManualClear && CUSTOMER_CODE!=CC_Greatek)                          //AI(ht9045-v899) 20260605: 僅人工按鈕觸發才提示,程式呼叫不跳詢問避免誤清
    {
        if(ShowMyMessageBox_YES_NO("Clear Sort Count?", "確定要清空計數？")!=1)  //AI(ht9045-v899) 20260605: 改用 ShowMyMessageBox_YES_NO 留下操作紀錄
        {
            s_w906ClearExit=kSortClrConfirmNo;                                  // [W906]
            return;
        }
        else
        {
            if(SystemStart)                                                     //JerryYang 20180510 (Steven) 加上保護避免Start時清除count
            {
                s_w906ClearExit=kSortClrSystemStartAfterConfirm;                // [W906]
                return;
            }
        }
    }

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.DoClearCount);                                   // 5     按下 Clear Count

    MyDBIProductionData("Clear Sorting Count");                                 //Steven 20140816 : Production Data

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        if(IniConfig.bN14_1_EnableOEEFunction==true &&
           IniConfig.iN14_1_OEERecordCycleTime>0)
        {
            // GATE G7（golden :658）：fProductionInfo->EachCycleSecondDo_SaveAndUpdateOEEFiles(true); —— facade 沒有這個方法。
        }
    }

    fMain->Clarn_Data(8, "btnClearCountClick");

    //AI(ht9045-clearcount-flow) 20260731 (RogerYang) : P260729-ATK-H9-01 Clear Count 補清 Auto/Fix Tray Count
    if(bManualClear                         &&                                  // 僅人工按鈕(程式呼叫傳fSortCT不清)
       USE_COVER_TRAYID==tCID_NFC           &&                                  // 僅Tray Count欄有顯示的NFC/ATK機型
       LastSet.iUnloadFixTray==0            &&                                  // eAtkTfInit=0,不在ATK feed-out期(保backup/restore配對)
       LastSet.iRunStartMode!=rsmAutoRetest &&                                  // 不在ART回流期(保RT收盤盤數判斷)
       HasICUnderMachine()==false)                                              // 機台流程內無在途料
    {
        for(int i=0; i<MAX_AUTO_TRAY; i++)
        {
            LastSet.iUnloaderTrayCount_ART[i]=0;
            pnlTrayCnt[i]->Caption="0";
        }

        fMain->lblAuto1TrayCnt->Caption=0;                                      // 對齊Initial Start清除行為(csystem.cpp CheckContinusStartIsReady)
        fMain->lblAuto2TrayCnt->Caption=0;
        fMain->lblAuto3TrayCnt->Caption=0;
        fMain->lblAuto4TrayCnt->Caption=0;
        fMain->lblAuto5TrayCnt->Caption=0;
        fMain->lblAuto6TrayCnt->Caption=0;

        MyDBIProcess("Process", "Auto Tray Count has been cleared!!");           // log留痕,格式對齊其他ClearCount訊息
    }
    else if(bManualClear && USE_COVER_TRAYID==tCID_NFC)                         //AI(ht9045-clearcount-flow) 20260806 (RogerYang) : H9-01 驗證輔助-守門未過時留原因, 現場可判讀為何Tray Count沒清
    {
        AnsiString sBlk;
        sBlk.sprintf("Auto Tray Count NOT cleared (guard): iUnloadFixTray=%d, RunStartMode=%d, HasICUnderMachine=%d",
            LastSet.iUnloadFixTray, LastSet.iRunStartMode, (int)HasICUnderMachine());
        MyDBIProcess("Process", sBlk);
    }

    // GATE G8（golden :694）fContactCT->sgYield->Refresh(); —— VCL 重繪（MainClarnData.h 偏離 D1 同理由）。
    // GATE G9（golden :695-696）if(fSCKART->bShow) fSCKART->UpdateCount(); —— forms/fSCKART.h 沒有 bShow／UpdateCount。

    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        fProductionInfo->ClearOEECount();
    }
    iallSitCount=0;                                                             //kevin 20180720 (wei) add all site fail count

    if(CosFunction.bBarcodeTrayRecFile==true)                                   //jou 20190930 : Barcode Tray record file
    {
        // GATE G10（golden :706）fBarCode->InitBarcodeRecFile(); —— BarCode/BarCode.h facade 沒有這個方法。
    }
    s_w906ClearExit=kSortClrDone;                                               // [W906]
}

//---------------------------------------------------------------------------
//  安裝（port-only）
//---------------------------------------------------------------------------
namespace {
const TfSortCTBodies kW906SortCTBodies = {
    &TfSortCT::W906Body_ShowLoadingIC,
    &TfSortCT::W906Body_ShowLoadingIC_ART,
    &TfSortCT::W906Body_ShowSortIC,
    &TfSortCT::W906Body_btnClearCountClick,
};
// (a) 靜態初始化：本物件一被連進執行檔，main() 之前就裝好（g_W906_SortCTBodies 是常數初始化的 NULL 指標，
//     不受跨 TU 動態初始化順序影響）。
struct W906SortCTAutoInstall { W906SortCTAutoInstall() { g_W906_SortCTBodies = &kW906SortCTBodies; } };
W906SortCTAutoInstall s_w906SortCTAutoInstall;
}

// (b) 連結錨點＋冪等安裝：WebBridgeTags.cpp 每拍呼叫（見本檔檔頭）。回傳值給呼叫端確認本體已裝。
bool W906_SortCTInstall()
{
    g_W906_SortCTBodies = &kW906SortCTBodies;
    return g_W906_SortCTBodies != 0;
}

//---------------------------------------------------------------------------
//  網頁 Clear 鈕的 C++ 端（port-only）：兩段式確認的一段。
//    confirmed=false：跑 golden 守衛到確認框為止（或 Greatek 這種 golden 不問的客戶 → 直接執行完）
//    confirmed=true ：守衛重跑，確認框回 YES，執行 golden 本體
//  回傳：*exitCode = W906_SortCTClearExit；*confirmReached = 本次是否走到確認框。
//  JSON 包裝在 WebSortCT.cpp（wb_serve 專用，可用 WebBridge 的 JsonWriter；本檔在 ht9045_sm，不連 ht9045_webbridge）。
//---------------------------------------------------------------------------
void W906_SortCTClearRun(bool confirmed, int* exitCode, bool* confirmReached)
{
    s_w906ConfirmAnswer  = confirmed ? 1 : 0;
    s_w906ConfirmReached = false;
    fSortCT->btnClearCountClick(fSortCT->btnClearCount);                        // golden 的人工按鈕路徑（Sender==btnClearCount ⇒ bManualClear）
    s_w906ConfirmAnswer  = 0;                                                   // 回到安全預設：任何其他呼叫者走到確認框都當 NO
    if (exitCode)       *exitCode       = s_w906ClearExit;
    if (confirmReached) *confirmReached = s_w906ConfirmReached;
}
