// =============================================================================
//  cCounterClear.cpp  --  TfCounterClear (golden cCounterClear.cpp, 504 lines)
//
//  AI(W906-FW-SecCC) 20260819: new file. See forms/fCounterClear.h for the
//  full translation-status table, GATE REGISTER, ctor safety argument and
//  STUB COLLISION search. This file only carries per-statement pointer-back
//  comments to that banner.
// =============================================================================
#include "forms/fCounterClear.h"

#include "vclcompat/vcl_compat.h"   // AnsiString, FileExists, DeleteFile
#include "cSocket.h"                 // ArmData[3]/ArmDataLot[3]/ArmHistory[3], TastCategory
#include "cAuthority.h"              // authCounterClr[9], GetCountClrAuth()
#include "cMyDB.h"                   // MyDBIProcess, MyDBIProductionData
#include "cprod.h"                   // BinSelect[8], TestIF_File, Prod, iTestRunMode, ...
#include "cpublic.h"
#include "cmydef.h"                  // CUSTOMER_CODE + CC_*, iTo3Unload, iByBinTotal, iSVByBinCount, ...
#include "MachineType.h"             // eTrayCount, TEST_MAX_BIN, MAX_SOCKET_ROW/COL, eAuto1
#include "Config.h"                   // IniConfig
#include "CosFunction.h"               // CosFunction
#include "LastSet.h"                    // LastSet
#include "forms/fMain.h"                 // fMain->Clarn_Data (real); GATE (CC3) evidence
#include "forms/fProductionInfo.h"        // fProductionInfo -- GATE (CC2) evidence (type visible for the comment)

//---------------------------------------------------------------------------
TfCounterClear *fCounterClear = new TfCounterClear();   // see forms/fCounterClear.h CTOR SAFETY note -- unconditional, cContactCT-shape
//---------------------------------------------------------------------------
TfCounterClear::TfCounterClear()                                                 // golden :24-28
{
    bHasFile = true;
}
//---------------------------------------------------------------------------
void TfCounterClear::WriteCTInfo()                                               // golden :30-38 //Steven 20110801 : 改用結構方式存檔
{
    for (int i = 0; i < 3; i++)                                                  //Steven 20140614 : 改成For迴圈
    {
        ArmData[i]->WriteFile();
        ArmDataLot[i]->WriteFile();
        ArmHistory[i]->WriteFile();
    }
}
//---------------------------------------------------------------------------
void TfCounterClear::ReadCTInfo()                                                // golden :40-48 //Steven 20110801 : 改用結構方式存檔
{
    for (int i = 0; i < 3; i++)                                                  //Steven 20140614 : 改成For迴圈
    {
        ArmData[i]->ReadFile();
        ArmDataLot[i]->ReadFile();
        ArmHistory[i]->ReadFile();
    }
}
//---------------------------------------------------------------------------
void TfCounterClear::cbSelectAllMouseUp(TObject *Sender)                         // golden :50-77 (Button/Shift/X/Y dropped, unused -- same DEVIATION class as forms/fContactCT.h's sgYieldMouseDown)
{
    if (cbSelectAll->Checked)
    {
        if (authCounterClr[0]) cbAlarmData->Checked       = true;
        if (authCounterClr[1]) cbTestCategory->Checked    = true;
        if (authCounterClr[2]) cbScanner->Checked         = true;
        if (authCounterClr[3]) cbLoadingCount->Checked    = true;
        if (authCounterClr[4]) cbContactCountCurr->Checked = true;
        if (authCounterClr[5]) cbContactCountHis->Checked  = true;
        if (authCounterClr[6]) cbSortingCount->Checked    = true;
        if (authCounterClr[7]) cbTimeData->Checked        = true;
        cbSelectAll->Caption = "UnSelect All";
    }
    else
    {
        if (authCounterClr[0]) cbAlarmData->Checked       = false;
        if (authCounterClr[1]) cbTestCategory->Checked    = false;
        if (authCounterClr[2]) cbScanner->Checked         = false;
        if (authCounterClr[3]) cbLoadingCount->Checked    = false;
        if (authCounterClr[4]) cbContactCountCurr->Checked = false;
        if (authCounterClr[5]) cbContactCountHis->Checked  = false;
        if (authCounterClr[6]) cbSortingCount->Checked    = false;
        if (authCounterClr[7]) cbTimeData->Checked        = false;
        cbSelectAll->Caption = "Select All";
    }
}
//---------------------------------------------------------------------------
void TfCounterClear::cbAlarmDataMouseUp(TObject *Sender)                         // golden :79-88 (CheckBox/Button/Shift/X/Y trimmed to the one param the body reads -- Sender IS read, cast to the clicked TCheckBox)
{
    TCheckBox *CheckBox = (TCheckBox *)Sender;
    if (!CheckBox->Checked)
    {
        cbSelectAll->Checked = false;
        cbSelectAll->Caption = "Select All";
    }
}
//---------------------------------------------------------------------------
//Steven 20090731 : Use Auth to Enable/Disable Component
//---------------------------------------------------------------------------
void TfCounterClear::FormShow(TObject *Sender)                                   // golden :92-114
{
    GetCountClrAuth();
    cbAlarmData->Enabled        = authCounterClr[0];
    cbTestCategory->Enabled     = authCounterClr[1];
    cbScanner->Enabled          = authCounterClr[2];
    cbLoadingCount->Enabled     = authCounterClr[3];
    cbContactCountCurr->Enabled = authCounterClr[4];
    cbContactCountHis->Enabled  = authCounterClr[5];
    cbSortingCount->Enabled     = authCounterClr[6];
    cbTimeData->Enabled         = authCounterClr[7];
    if (!authCounterClr[0]) cbAlarmData->Checked        = false;
    if (!authCounterClr[1]) cbTestCategory->Checked     = false;
    if (!authCounterClr[2]) cbScanner->Checked          = false;
    if (!authCounterClr[3]) cbLoadingCount->Checked     = false;
    if (!authCounterClr[4]) cbContactCountCurr->Checked = false;
    if (!authCounterClr[5]) cbContactCountHis->Checked  = false;
    if (!authCounterClr[6]) cbSortingCount->Checked     = false;
    if (!authCounterClr[7]) cbTimeData->Checked         = false;
    // NOTE: golden also sets `Top=30; Left=300;` here (TForm position) --
    // this facade models no form-level geometry (same posture as forms/
    // fContactCT.h / forms/fYieldMonitoring.h / forms/fObserver.h, none of
    // which declare Left/Top/Width/Height on the form object itself).
    fShow = true;
}
//---------------------------------------------------------------------------
void TfCounterClear::ClearCount(int ClearType)                                   // golden :116-392
{
    AnsiString BatFile = getenv("W906_BINCOUNT_PATH") ? getenv("W906_BINCOUNT_PATH") : "D:\\HT9045\\system\\BinCount.txt";   //kevin 20210825 清除Bin 數量統計   //AI(W906-PROD-S111) 20260926: golden 字面；W906_BINCOUNT_PATH 有設時跟 ReadWriteBinCountMode（csystem.cpp）同一個轉開（ctest），沒設＝golden
    int iCate = 0;
    switch (ClearType)
    {
        case ctAlarmData:
            break;
        case ctTesterCategory:
            // GATE (CC2): fProductionInfo->CalculateNowArmSiteBinQty(true)
            // (golden :125) -- see forms/fCounterClear.h banner. TfProductionInfo
            // has no such member.
//#if 0   //AI(W906-S09-B1) 20260929: GATE (CC2) retired -- TfProductionInfo::CalculateNowArmSiteBinQty is forms/fProductionInfo.h:328 (body forms/fProductionInfo.cpp:943, already live at cContactCT.cpp:1069); golden 906_0625_Steven cCounterClear.cpp:125
            fProductionInfo->CalculateNowArmSiteBinQty(true);
//#endif   //AI(W906-S09-B1) 20260929: see :131
            for (int i = 0; i < MAX_SOCKET_ROW; i++)
            {
                for (int j = 0; j < MAX_SOCKET_COL; j++)
                {
                    for (int k = 0; k < iTestBinCount; k++)                      //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
                    {
                        ArmData[0]->SetBinCT(i, j, k, 0);                       //Steven 20140509 Modify
                        ArmData[1]->SetBinCT(i, j, k, 0);
                        ArmData[2]->SetBinCT(i, j, k, 0);
                        iByBinTotal[k] = 0;                                     //kevin 20180703 (wei) bin 數量 Bin total[0]
                    }
                    ArmData[0]->SetIFErr(i, j, 0);
                    ArmData[1]->SetIFErr(i, j, 0);
                    ArmData[2]->SetIFErr(i, j, 0);
                }
            }

            TastCategory.UpdataCount(true);                                     //Steven 20250514 : 統一計算數量
            // GATE (CC1) OPENED 20260926 —— AI(W906-PROD-S111)（Steven 團隊，RULINGS_20260926 S111／S120-2「照 golden 補齊」）：
            //   原本擋它的理由是舊波次任務說明的寫入禁區清單（system\ 等一律 #if 0），已過期；照 golden :146-152 刪 BinCount.txt。
            //   路徑：上面 :121（golden 字面；W906_BINCOUNT_PATH 有設時轉開，同 ReadWriteBinCountMode）。何時刪：每一次 ClearCount(ctTesterCategory)
            //   —— counterclear.exe 勾 Tester Category、act.counterClear.exe testerCategory、Clarn_Data 的 Tag 1/2/4/5/6/7/8 與 Tag 0（依 bCTClear）
            //   （例：SortCT 的 Clear Count＝Clarn_Data(8)）。刪掉後由下一次 ReadWriteBinCountMode 重建（golden 同）。原本的 #if 0／#endif 改成下面兩行註解，行數不變。
// #if 0（GATE CC1 原本的開頭）
            try
            {
                if (FileExists(BatFile))
                    DeleteFile(BatFile);
            }
            catch (...)
            {
                MyDBIProcess("Process", "Delete File BinCount.txt Error");
            };
// #endif（GATE CC1 原本的結尾）
            MyDBIProcess("Process", "Tester category has been cleared!!");
            // GATE (CC2): fProductionInfo->UpdateControlBinCount(true) (golden :154) -- see above.
//#if 0   //AI(W906-S09-B1) 20260929: GATE (CC2) retired -- UpdateControlBinCount is forms/fProductionInfo.h:332 (body .cpp:1042, already live at cContactCT.cpp:1093); golden cCounterClear.cpp:154
            fProductionInfo->UpdateControlBinCount(true);                       //Sam 20200525 : Control Bin
//#endif   //AI(W906-S09-B1) 20260929: see :170
            break;
        case ctLoadingCounts:
            LastSet.iJamCount[0] = 0;
            LastSet.iJamCount[1] = 0;
            LastSet.SendCT[0] = 0;
            LastSet.SendCT[1] = 0;                                                //jou 2010-08-13 計算jam rate,改為Tray Feed當一次記錄
            LastSet.SendCT[2] = 0;                                                //jou 2010-11-17 Piggy-Back Functions
            LastSet.SendCT[3] = 0;                                                //Steven 20110420 : Piggy-Back Functions - Loading Count
            LastSet.iIndexCount = 0;                                              //wei 20141201 Low Yield Auto Clean

            if (USE_AUTO_RETEST == eartInstall && bAutoReTest_ART)                 //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
            {
                if (LastSet.iRunStartMode >= rsmInitial_ART && LastSet.iRunStartMode <= rsmAutoRetest)
                {
                    LastSet.SendCT_ART[0] = 0;
                    LastSet.SendCT_ART[1] = 0;
                    LastSet.SendCT_ART[2] = 0;
                    LastSet.SendCT_ART[3] = 0;
                }
            }
            MyDBIProcess("Process", "Loading Count has been cleared!!");
            break;
        case ctContactCounts:
            for (int i = 0; i < MAX_SOCKET_ROW; i++)
            {
                for (int j = 0; j < MAX_SOCKET_COL; j++)
                {
                    ArmData[0]->SetFailCT(i, j, 0);                             //Steven 20140509 : Modify
                    ArmData[0]->SetPassCT(i, j, 0);
                    ArmData[1]->SetFailCT(i, j, 0);
                    ArmData[1]->SetPassCT(i, j, 0);
                    ArmData[2]->SetFailCT(i, j, 0);
                    ArmData[2]->SetPassCT(i, j, 0);
                }
            }
            MyDBIProcess("Process", "Contact Count(Total) has been cleared!!");
            break;
        case ctContactCountsHis:
            for (int i = 0; i < MAX_SOCKET_ROW; i++)
            {
                for (int j = 0; j < MAX_SOCKET_COL; j++)
                {
                    ArmHistory[0]->SetFailCT(i, j, 0);                          //Steven 20140509 : Modify
                    ArmHistory[0]->SetPassCT(i, j, 0);
                    ArmHistory[1]->SetFailCT(i, j, 0);
                    ArmHistory[1]->SetPassCT(i, j, 0);
                    ArmHistory[2]->SetFailCT(i, j, 0);
                    ArmHistory[2]->SetPassCT(i, j, 0);
                }
            }
            MyDBIProcess("Process", "Contact Count(History) has been cleared!!");
            break;
        case ctTraySortCount:
            // GATE (CC2): fProductionInfo->CalculateNowUnloaderTrayQty(true) (golden :208) -- see above.
//#if 0   //AI(W906-S09-B1) 20260929: GATE (CC2) retired -- CalculateNowUnloaderTrayQty is forms/fProductionInfo.h:162 (body .cpp:276); golden cCounterClear.cpp:208
            fProductionInfo->CalculateNowUnloaderTrayQty(true);
//#endif   //AI(W906-S09-B1) 20260929: see :227
            for (int i = 0; i < eTrayCount; i++)                                     //JerryYang 20220909 : 10->eTrayCount
            {
                LastSet.RecodeTrayCount[i] = 0;                                   //frank 20200814 : 每10盤記錄一次summary log
                LastSet.BinCT[0][iTo3Unload[i]] = 0;                              //Steven 20180516 : 1切板以後,所以直接清除
                LastSet.BinCT[2][iTo3Unload[i]] = 0;
                LastSet.BinCT[3][iTo3Unload[i]] = 0;
                if (USE_AUTO_RETEST == eartInstall && bAutoReTest_ART)             //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
                {
                    LastSet.BinCT_ART[0][iTo3Unload[i]] = 0;
                    LastSet.BinCT_ART[2][iTo3Unload[i]] = 0;
                    LastSet.BinCT_ART[3][iTo3Unload[i]] = 0;
                }
            }

            if (CosFunction.bSpecailLowYeild)                                    //Sam 20210505 : PTI 要求分兩段 Low Yeild
                LowYieldSpecialInitail();

            for (int i = 0; i < TEST_MAX_BIN; i++)
            {
                LastSet.iBinData32[0][i] = 0;
                iSVByBinCount[i] = 0;                                             //JerryYang 20250120 : add
                LastSet.iBinData32[2][i] = 0;
                LastSet.iBinData32[3][i] = 0;
                if (USE_AUTO_RETEST == eartInstall && bAutoReTest_ART)             //kevin 20150615
                {
                    LastSet.iBinData32_ART[0][i] = 0;
                    LastSet.iBinData32_ART[2][i] = 0;
                    LastSet.iBinData32_ART[3][i] = 0;
                }
            }

            if (CosFunction.bHWBinBox && iHWFix_BinBox == 1)                       //kevin 20160822 error bin box 清除
            {
                LastSet.iBinBoxCount = 0;
            }
            break;
        case ctTimeData:
            for (int k = 0; k < 2; k++)                                              //Steven 20190714 : 4-->3
            {
                for (int i = 0; i < 8; i++)
                    LastSet.SystemAccSecond[k][i] = 0;                            //第三組改成每12Hr計算一次, 第二組為每24Hr計算一次
            }
            MyDBIProcess("Process", "Time Data has been cleared!!");
            break;
        case ctBinCount:
            for (int k = 0; k < 4; k++)
            {
                for (int i = 0; i < TEST_MAX_BIN; i++)
                {
                    LastSet.iBinData32[k][i] = 0;
                    iSVByBinCount[i] = 0;                                         //JerryYang 20250120 : add
                }
            }
            break;
        case ctIndexCount:                                                      //kevin 20130125 記錄 index帶著ic數量 index位置shuttle 數量
            for (int k = 0; k < 4; k++)
            {
                LastSet.iIndexInputOutPut[k] = 0;
            }
            break;
        case ctAutoRetestCount:                                                 //kevin 20150530 AutoRETEST
            if (USE_AUTO_RETEST == eartInstall &&
                (bAutoReTest_ART || IniConfig.bA10_AutoReTest))                  //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
            {
                if (CUSTOMER_CODE == CC_KYEC_LEE)
                {
                    if (LastSet.iRunStartMode >= rsmInitial_ART &&
                        LastSet.iRunStartMode <= rsmAutoRetest)
                    {
                        LastSet.SendCT_ART[0] = 0;                                //kevin 20150615  ART 計數清除
                    }
                }
                else
                {
                    LastSet.SendCT_ART[0] = 0;                                    //kevin 20150615  ART 計數清除
                }
            }

            for (int k = 0; k < 4; k++)
            {
                if (k == 1)                                                        //Steven 20190715 : 第1組改成每24hr清一次
                {
                }
                else
                {
                    for (int i = 0; i < eTrayCount; i++)
                    {
                        if (CosFunction.bUseARTSortCount == false ||               //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
                            (LastSet.iRunStartMode >= rsmInitial_ART &&
                             LastSet.iRunStartMode <= rsmAutoRetest))              //Ifor 20180110 (Steven) : add rsmInitial_ART 需清除Sort Count
                        {
                            LastSet.BinCT_ART[k][iTo3Unload[i]] = 0;              //kevin 20150615  ART 計數清除
                        }

                        if (bART_needRT2)                                        //kevin 20150717 多fail tray 清除0
                        {
                            if (k == 0 && (BinSelect[4].iStackDefFailCate[i] > 0))    //有設定Fail tray
                            {
                                LastSet.BinCT[k][iTo3Unload[i]] = 0;
                                if (USE_AUTO_RETEST == eartInstall &&
                                    (bAutoReTest_ART ||
                                     IniConfig.bA10_AutoReTest))                 //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
                                {
                                    if (CosFunction.bUseARTSortCount == false ||   //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
                                        (LastSet.iRunStartMode >= rsmInitial_ART &&
                                         LastSet.iRunStartMode <= rsmAutoRetest))  //Ifor 20180110 (Steven) : add rsmInitial_ART 需清除Sort Count
                                    {
                                        LastSet.BinCT_ART[k][iTo3Unload[i]] = 0;
                                    }
                                }
                            }
                        }
                        else if (k == 0 && (BinSelect[4].bAutoRetest[i]))          //有設定autoretest
                        {
                            LastSet.BinCT[k][iTo3Unload[i]] = 0;
                            if (USE_AUTO_RETEST == eartInstall &&
                                (bAutoReTest_ART ||
                                 IniConfig.bA10_AutoReTest))                     //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
                                LastSet.BinCT_ART[k][iTo3Unload[i]] = 0;
                        }
                    }

                    for (int i = 0; i < TEST_MAX_BIN; i++)
                    {
                        if (CosFunction.bUseARTSortCount == false ||
                            (LastSet.iRunStartMode >= rsmInitial_ART &&            //Ifor 20180110 (Steven) : add rsmInitial_ART 需清除Sort Count
                             LastSet.iRunStartMode <= rsmAutoRetest))              //Ifor 20171016 (wei) : add Auto Sort Count 計數功能 僅用SECS GEM 下命令清除
                        {
                            LastSet.iBinData32_ART[k][i] = 0;
                        }

                        if (TestIF_File.bPass[i] || Prod.bIsPassBin[i])          //20150706autoRetest 設定pass bin 清到本 PASS TRAY BIN RT不需清除
                            continue;

                        if (IniConfig.bA60EnableAMR)                             //Sam 20240827 : 新增 AMR 功能
                        {
                            if (k == 2)
                            {
                                iCate = BinSelect[iTestRunMode].iCatDataT3Pos[i];
                                if (iCate >= 1 && iCate <= 9)
                                {
                                    if (BinSelect[iTestRunMode].bCateR[iCate - 1])
                                    {
                                        LastSet.iBinData32[k][i] = 0;
                                        iSVByBinCount[i] = 0;         //JerryYang 20250120 : add
                                    }
                                }
                            }
                            else
                            {
                                LastSet.iBinData32[k][i] = 0;
                                iSVByBinCount[i] = 0;                 //JerryYang 20250120 : add
                            }
                        }
                        else
                        {
                            LastSet.iBinData32[k][i] = 0;
                            iSVByBinCount[i] = 0;                     //JerryYang 20250120 : add
                        }
                    }
                }
            }

            if (CUSTOMER_CODE == CC_PTI && IniConfig.bB03_TesterReport)            //Sam 20240809 : PTI ART 模式
            {
                ArmData[2]->ClearALLCT();
                TastCategory.UpdataCount(true);                                 //Steven 20250514 : 統一計算數量
            }
            break;
        case ctFailBinCount:                                                    //Frank 20241114 : Add
            for (int i = eAuto1; i <= iAutoRight; i++)
            {
                if (Prod.iIsFailT6[i] == 1)
                {
                    LastSet.BinCT[0][i] = 0;
                    LastSet.BinCT[2][i] = 0;
                    LastSet.BinCT[3][i] = 0;
                }
            }
            break;
    }

    bRefreshCount = true;                                                         //Stteve 20250219 : 清完數量後的顯示要delay一下
}
//---------------------------------------------------------------------------
void TfCounterClear::spbExeClick(TObject *Sender)                                // golden :394-450
{
    // GATE (CC3): the CC_KYEC_LEE re-auth branch (golden :396-418) touches
    // fMain->cbUserSelect/ChangeLevelAttr/cbUserSelectChange/stOperatorClick/
    // btLogin/spbUserName -- forms/fMain.h has none of the first five (only
    // ChangeLevelAttr is real). Gated as a whole (see forms/fCounterClear.h
    // banner for why a partial evaluation would not be faithful).
#if 0
    if (CUSTOMER_CODE == CC_KYEC_LEE)                                              //Ifor 20191008 : add KYEC 清除 Count 需檢Barcode & 權限
    {
        ReEnterBarcode[0] = false;
        fMain->cbUserSelect->ItemIndex = 0;
        AccessLevel = 0;
        fMain->ChangeLevelAttr();
        fMain->cbUserSelect->Text = "Operator";
        fMain->spbUserName->Caption = "Operator";
        fMain->btLogin->Caption = "Login";
        if (FileExists(pwPath))
        {
            fMain->cbUserSelectChange(NULL);
        }
        else
        {
            fMain->stOperatorClick(fMain);
        }

        if (AccessLevel < iDefSupervisorLevel)
        {
            return;
        }
    }
#endif

    MyDBIProductionData("Clear Count executed");
    fMain->Clarn_Data(10, "Manual clear count");

    if (cbAlarmData->Checked)
        ClearCount(ctAlarmData);

    if (cbTestCategory->Checked)
        ClearCount(ctTesterCategory);

    if (cbLoadingCount->Checked)
    {
        ClearCount(ctLoadingCounts);
        ClearCount(ctIndexCount);                                               //kevin 20130125 記錄 index帶著ic數量 index位置shuttle 數量
    }

    if (cbContactCountCurr->Checked)
        ClearCount(ctContactCounts);

    if (cbContactCountHis->Checked)
        ClearCount(ctContactCountsHis);

    if (cbSortingCount->Checked)
        ClearCount(ctTraySortCount);

    if (cbTimeData->Checked)
        ClearCount(ctTimeData);

    fMain->Clarn_Data(10, "Manual clear count done");
    MyDBIProcess("Process", "Counter Clear has been executed!!");
    spbExe->Down = false;
}
//---------------------------------------------------------------------------
void TfCounterClear::spbExitClick(TObject *Sender)                               // golden :452-456
{
    spbExit->Down = false;
    // DEVIATION: golden `Close();` (VCL TForm::Close) -- see forms/
    // fCounterClear.h banner (ATCInterface.cpp:446 precedent).
    FormClose();
}
//---------------------------------------------------------------------------
void TfCounterClear::AutoClear()                                                 // golden :458-485
{
    if (IniConfig.bDownLoadAutoCountClear == false)
        return;

    GetCountClrAuth();

    if (authCounterClr[0])
        ClearCount(ctAlarmData);

    if (authCounterClr[1])
        ClearCount(ctTesterCategory);

    if (authCounterClr[3])
        ClearCount(ctLoadingCounts);

    if (authCounterClr[4])
        ClearCount(ctContactCounts);

    if (authCounterClr[5])
        ClearCount(ctContactCountsHis);

    if (authCounterClr[6])
        ClearCount(ctTraySortCount);

    if (authCounterClr[7])
        ClearCount(ctTimeData);
}
//---------------------------------------------------------------------------
void TfCounterClear::FormClose()                                                 // golden :487-491 (Sender/TCloseAction dropped, see banner)
{
    fShow = false;
}
//---------------------------------------------------------------------------
void TfCounterClear::LowYieldSpecialInitail()                                    // golden :493-503 //Sam 20210505 : PTI 要求分兩段 Low Yeild
{
    bLowYeildAlarmSpecial = false;
    bLowYeildAlarmSpecial1stPass = false;
    for (int i = 0; i < eTrayCount; i++)
    {
        LastSet.BinCT_PTI[0][iTo3Unload[i]] = 0;
        LastSet.BinCT_PTI[2][iTo3Unload[i]] = 0;
        LastSet.BinCT_PTI[3][iTo3Unload[i]] = 0;
    }
}
//---------------------------------------------------------------------------

// =============================================================================
//  MAIN-LOOP INTEGRATION NOTE -- csystem.cpp:4851-4854 TU-local seam
//  --------------------------------------------------------------------------
//  csystem.cpp defines (file-scope, invisible to every other TU):
//      struct W7C1_TfCounterClearSeam { void LowYieldSpecialInitail(){} };
//      static W7C1_TfCounterClearSeam *W7C1_fCounterClear = new W7C1_TfCounterClearSeam();
//      #define fCounterClear          W7C1_fCounterClear
//  This #define makes EVERY occurrence of the bare token `fCounterClear`
//  inside csystem.cpp (after line 4854) resolve to the seam, not to the real
//  global this file now defines -- textual substitution, not name lookup, so
//  simply linking this .cpp in does NOT reconnect csystem.cpp's call sites.
//  Two call shapes exist there today:
//    (a) csystem.cpp:5921 `fCounterClear->LowYieldSpecialInitail();` -- this
//        line sits at plain top level (verified: no enclosing #if/#ifdef
//        between csystem.cpp:5142 [W7C1_SEAM end] and :9879 [next gate open],
//        20260819), so it is LIVE TODAY and silently calls the seam's
//        EMPTY no-op instead of the real body -- a behaviour gap that exists
//        NOW, not introduced by this file. (csystem.cpp:10410's second
//        `LowYieldSpecialInitail()` call sits inside the :9879-:11117
//        `#if 0` GOLDEN VERBATIM block and is inert.)
//    (b) the 17 `fCounterClear->ClearCount(...)` textual occurrences (see
//        forms/fCounterClear.h banner's grep count) -- ALL 17 are already
//        inert today: 8 fall inside csystem.cpp's own two `#if 0` GOLDEN
//        VERBATIM ranges (:3070-:4212 and :11180-:11881, both confirmed by
//        directive-nesting count this wave), 6 are comment-only text inside
//        cShowBinSelect.cpp's GATE (B1) banner (no compiled call exists
//        there), 2 are Command.cpp's ClearSortCountByDLL (one explanatory
//        comment plus one `#if 0`-gated real call), and 1 is a comment
//        fragment inside Automation/auto9045.cpp's OWN separate TU-local
//        shim call (`W5FA_FCounterClear.ClearCount(ctTimeData);`, a
//        DIFFERENT stand-in object, not this seam) -- so ClearCount has NO
//        live caller anywhere yet; only (a) is a live behaviour gap.
//    (c) csystem.cpp:6442 `#define W7C2_FCOUNTER_WRITECTINFO() do{}while(0)`
//        (one real call site, csystem.cpp:7544, LIVE, one-cycle history
//        write skipped today) is a THIRD, independent no-op macro for
//        WriteCTInfo -- not part of the W7C1 struct at all.
//
//  RECOMMENDATION for the main-loop integration pass that registers this
//  file in CMakeLists (out of this wave's scope -- the task brief forbids
//  touching csystem.cpp/CMakeLists here): follow the ALREADY-PROVEN pattern
//  csystem.cpp:6441 used for the sibling fYieldMonitoring seam (AI(W906-FW-
//  YMSwap-enable) 20260818, "the gate's premise died... The macro now
//  forwards to golden's own call"):
//    1. Retire csystem.cpp:4851-4854's `W7C1_TfCounterClearSeam` /
//       `#define fCounterClear` (its one member, LowYieldSpecialInitail, is
//       now real and takes zero arguments identically) -- csystem.cpp:5921
//       then calls the REAL fCounterClear->LowYieldSpecialInitail() with NO
//       further code change at that call site, only the seam's removal.
//    2. Change csystem.cpp:6442's `W7C2_FCOUNTER_WRITECTINFO()` macro body
//       from `do{}while(0)` to `do{ fCounterClear->WriteCTInfo(); }while(0)`
//       (exact mechanical mirror of :6441's existing ClearYieldCount swap).
//    3. ClearCount's 17 sites need NO seam-retirement work (all already
//       inert per (b) above) -- un-gating csystem.cpp's own two GOLDEN
//       VERBATIM ranges is a separate, much larger main-loop decision
//       (G4-3/G4-4) outside this wave and outside this recommendation's
//       scope.
// =============================================================================
