// ===========================================================================
//  RunStartMode.cpp  --  AI(W906-P10) 20260921
//
//  golden main.cpp 的兩支「開工模式」自由函式，獨立成一個 TU：
//      GetRunStartModeNum   golden main.cpp:240-252     13 行
//      SetRunStartMode      golden main.cpp:363-1115    753 行
//  （`TfMain::SetMainRunStartMode` 73 行是成員函式，放 forms/fMain.cpp）
//
//  ## 為什麼是獨立 TU，而不是塞進樁原本的位置
//
//  樁原本在 aHotPlateSubstrate.cpp:1232 —— `void SetRunStartMode(int) {}`。
//  20260921 用**編譯器**量過（整檔 + golden body 拼成一個 TU，兩組態各一次
//  -fsyntax-only）：golden 本體要 22 個當下解析不到的名字，其中 20 個只差
//  include（fMain / fSCKART / fLotInfo / fBinSel / clRed / clLime …）。
//  把它們塞進 aHotPlateSubstrate.cpp 等於給那個檔加約 15 個 include。
//  前例：P2a 的 ContactForceLoad.cpp 就是為同樣理由開的獨立 TU
//  （CMakeLists.txt:299）。
//
//  ⚠ 宣告仍留在 aHotPlateSubstrate.h:929，所以 36 個呼叫端一行都不用動。
//
//  ## 簽章與 golden 一致，但保留單參數呼叫端
//
//  golden: `void SetRunStartMode(eRunStartMode Mode, AnsiString ModeText)`
//          （Mode==rsmNull 時用 ModeText 決定模式）
//  本樹原樁: `void SetRunStartMode(int)`
//  ⇒ 改成 golden 的雙參數並給預設值，既忠實又不必動呼叫點。
//
//  ## 相依：零個「相依不存在」
//
//  22 個未解析裡：
//    * 20 個在 header 裡有宣告，只差 include
//    * `RecordAutoSiteMapStart` / `RecordAutoSiteMapFinish` 有**真本體**
//      （ainarm2.cpp:1450 起，ACTIVE），只是沒有 header 宣告 -> 本檔補宣告
//  ⇒ 一個閘都沒加。
// ===========================================================================
#include "aHotPlateSubstrate.h"      // SetRunStartMode 的宣告（:929）
#include "cmydef.h"                  // eRunStartMode / rsm* / 全域純量
#include "cprod.h"                   // TestIF_File / IniConfig / Prod
#include "common.h"                  // CheckAndReadIniData 家族
#include "cpublic.h"                 // RecordProcess / GetTimeInfo
#include "csystem.h"                 // HasAnyICInMachine 等狀態述詞
#include "cinitial.h"                // SetWorkParameter
#include "cAuthority.h"              // BinSetting
#include "mysensor.h"                // Sen[]
#include "CosFunction.h"             // CosFunction
#include "canary_support.h"          // ShowMyMessage / __FUNC__
#include "forms/fMain.h"             // fMain（含 TfMainHanaART）
#include "forms/fLotInfo.h"          // fLotInfo
#include "forms/fQAMode.h"           // QABackupStatus
#include "forms/fHandlerSys.h"       // fRPDefault
#include "Automation/SCK_ART_Remainder.h"  // HasICUnderMachine
#include "SECSGEM/SecsEventReport.h" // EventReport
// AI(W906-P10) 20260921: 下面這幾個的宣告**不在**我第一版猜的 header 裡。
//   第一版是用 `git grep -lw <名字> -- '*.h'` 選的，而 grep 會把**註解裡的提及**
//   算成「有」—— 例如 `Automation/SCK_ART.h:40` 只是在註解裡講 fSCKART 的
//   duplicate-symbol 問題，真正的 `extern TfSCKART *fSCKART;` 在 forms/fSCKART.h:177。
//   ⇒ 22 個未解析靠編譯器降到 11 個之後，這 11 個就是被 grep 騙到的那一批。
#include "forms/fSCKART.h"           // extern TfSCKART *fSCKART（:177）
#include "forms/fBinSel.h"           // extern TfBinSel *fBinSel（:745）
#include "forms/fShowBinSelect.h"    // extern TfShowBinSelect *fShowBinSelect（:1133）
#include "forms/fRPDefault.h"        // extern TfRPDefault *fRPDefault（:360）
#include "forms/fQwertyKey.h"        // extern TfQwertyKey *fQwertyKey（:406）
#include "SECSGEM/SecsEventType.h"   // extern struct ETypeStruct SECS_EVENT（:340）
#include "SECSGEM/uHGemEquipment.h"  // const TColor clRed/clLime/clYellow（:395-397）
#include "vclcompat/LedCore.h"     // const TColor clBtnFace —— ⚠ 不是 BtnPanelCore.h：
                                  //   那裡的色彩區塊被 `#ifndef HT9045_W7C1_TCOLOR_SHIM`
                                  //   守著，先被別的 header 設過就整塊跳過。
                                  //   LedCore.h 帶的是共用的 9 個常數（ATC/ATCInterface.h:155 同樣用它）。
// AI(W906-P10) 20260921: `NewRecordProcess` 只補宣告，**不 include
//   acatchtray_shims.h** —— 那個檔在 :129-134 自己定義了一組顏色
//   （guard 是 `HT9045_TCOLOR_SHIM`，**與 vclcompat 那組的
//   `HT9045_W7C1_TCOLOR_SHIM` 不同**），會和上面 uHGemEquipment.h 的
//   `clYellow` 撞成 redefinition。實測：第一版 include 它，全量建置在
//   `acatchtray_shims.h:133` 噴 `redefinition of 'const TColor clYellow'`。
//   簽名逐字抄 golden cMyDB.h:62（含預設值）。
extern void NewRecordProcess(AnsiString AlarmCode, AnsiString S,
                             AnsiString Debug = " ");                           // golden cMyDB.h:62

// AI(W906-P10) 20260921: `clBtnFace` 住在 **vclcompat 命名空間**裡
//   （vclcompat/LedCore.h:63），而 clRed/clLime/clYellow 來自
//   SECSGEM/uHGemEquipment.h:395-397 的**全域**。golden 兩者都寫成不加限定，
//   所以這裡補一個 using，讓本體逐字照 golden，不要改成 vclcompat::clBtnFace。
using vclcompat::clBtnFace;

// AI(W906-P10) 20260921: 這兩支有**真本體**（ainarm2.cpp:1450 / :1462 起，ACTIVE）
//   但全樹沒有 header 宣告，所以只在這裡補宣告，簽名逐字抄 golden ainarm2.h:228/229。
//   不 include ainarm2.h：那會把 in-arm 的整個介面拉進本檔
//   （同 IsNNMode 在 ainarm9045.cpp、MoveOutArmToAutoSafe 在 mymotor.cpp 的處置）。
extern void RecordAutoSiteMapStart();                                           // golden ainarm2.h:228
extern void RecordAutoSiteMapFinish();                                          // golden ainarm2.h:229

//------------------------------------------------------------------------------
//  GetRunStartModeNum -- golden main.cpp:240-252（13 行）
//    SetRunStartMode 在 Mode==rsmNull 時用它把 ModeText 轉成列舉
//    （golden main.cpp:429）。
//------------------------------------------------------------------------------
eRunStartMode GetRunStartModeNum(AnsiString ModeName)                           //Steven 20120609 : 修改Start Mode切換方式
{
    eRunStartMode Ret=rsmNull;
    for(int i=0; i<rsmRunModeTotal; i++)
    {
        if(StartModeName[i]==ModeName)
        {
            Ret=eRunStartMode(i);
            break;
        }
    }
    return Ret;
}

//------------------------------------------------------------------------------
//  SetRunStartMode -- golden main.cpp:363-1115（753 行）
//------------------------------------------------------------------------------
void SetRunStartMode(eRunStartMode Mode, AnsiString ModeText)                   //當Mode為rsmNull時，用Text決定模式
{
    AnsiString szDir="", sFileName[]={"JOBFILE\\FT", "JOBFILE\\QA", "JOBFILE\\RT"};                                     //kevin 20160928

    if(IniConfig.bUseAutoSiteMapping==true &&                                   //Steven 20231031 : RT不做ASM
       IniConfig.bI21EnableASM==true       &&
       Mode==rsmAutoSiteMap)
    {
        if(IniConfig.bI50_EnableAutoSiteMappingTrigger==true)
        {
            if(IniConfig.bI50_RT==false)
            {
                if(LastSet.iRunStartMode==rsmCInitialRetest ||
                   LastSet.iRunStartMode==rsmContinuRetest)
                {
                    return;
                }
            }
        }
    }

    if(IniConfig.bI50_EnableAutoSiteMappingTrigger==false &&
       IniConfig.bI21RTmodeDonotRunSiteMapping &&                               //Richard 20230427 : RT mode不跑sitemapping
       (Mode==rsmAutoSiteMap ||
       (Mode==rsmNull && ModeText=="Site Mapping Check")))
    {
        if(fMain->palRT->Color==clRed && LastSet.iRunStartMode==rsmAutoSiteMap)
        {
            ModeText="Re-Test Initial Start";
            Mode    =rsmCInitialRetest;
        }

        if(LastSet.iRunStartMode==rsmCInitialRetest)
        {
            Mode=rsmCInitialRetest;
            fMain->cbRunStartMode->ItemIndex=LastSet.iRunStartMode;
        }
        else if(LastSet.iRunStartMode==rsmContinuRetest)
        {
            Mode=rsmContinuRetest;
            fMain->cbRunStartMode->ItemIndex=LastSet.iRunStartMode;
        }
    }

    if(Mode==rsmAutoSiteMap ||                                                  //jou 20220222 : 修正Single site Auto site mapping FT/RT mode 切換錯誤。
       (Mode==rsmNull && ModeText=="Site Mapping Check"))                       //Ifor 20190916 :add Auto Site Mapping 備份Start Mode
    {
        if(fLotInfo->cbRunMode->Visible==true &&                                //Steven 20240220 : Auto Site Map結束後, 增加檢查cbRunMode來切換模式
           fLotInfo->cbRunMode->Text.Pos("FT")>0)
        {
            iAutoSiteMapRunStartMode=0;
        }
        else if(LastSet.iRunStartMode==rsmCInitialRetest ||
                LastSet.iRunStartMode==rsmContinuRetest  ||
                (fLotInfo->cbRunMode->Visible==true &&
                 fLotInfo->cbRunMode->Text.Pos("RT")>0))                        //Steven 20240131 : Auto Site Map結束後, 增加檢查cbRunMode來切換模式
        {
            iAutoSiteMapRunStartMode=1;
        }
        else
        {
            iAutoSiteMapRunStartMode=0;
        }
    }

    if(Mode==rsmNull && ModeText!="")
        Mode=GetRunStartModeNum(ModeText);

    if(IniConfig.bI37_EnableFIFOMode==false && Mode==rsmFIFOMode)               //Steven 20170317 (wei) : 修正Start Mode模式錯誤問題
    {
        Mode=rsmInitialStart;
    }

    if(IniConfig.bI21EnableASM==false && Mode==rsmAutoSiteMap)
    {
        if(HasICUnderMachine()==false && HasAnyICInMachine()==false)            //RogerYang 20260312 : 避免有料時開關site被清空資料，造成後面疊料
        {
            if(LastSet.iRunStartMode==rsmCInitialRetest ||
               LastSet.iRunStartMode==rsmContinuRetest)                         //Steven 20230117 : 修正run mode
            {
                Mode=rsmCInitialRetest;
            }
            else
            {
                Mode=rsmInitialStart;
            }
        }
        else                                                                    //還有料在機台內
        {
            if(LastSet.iRunStartMode==rsmCInitialRetest ||
               LastSet.iRunStartMode==rsmContinuRetest)
            {
                Mode=rsmContinuRetest;
            }
            else
            {
                Mode=rsmContinuStart;
            }
        }
    }

    if(IniConfig.bQAMode==false && Mode==rsmQAMode)
    {
        Mode=rsmInitialStart;
    }

    if(CosFunction.bCanDisableQAMode &&                                         //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
       IniConfig.bA51EnableEQCMode==false && Mode==rsmQAMode)
    {
        Mode=rsmInitialStart;
    }

    if(CosFunction.bUseSCKART)
    {
        if(bCanRunSCKART==true)
        {
            if(Mode==rsmQAMode && IniConfig.bSPILFunction==false)               //Steven 20170830 (wei) : QA mode for ATK ART  //JerryYang 20220923 : SPIL不跑韓國版QA mode
            {
                bQAModeFlag=true;
            }
            else if(Mode==rsmInitial_ART       ||
                    Mode==rsmContinuStart_ART  ||
                    Mode==rsmAutoRetest        ||
                    Mode==rsmContinuRetest_ART ||
                    Mode==rsmContinuRetest     ||                               //Steven 20170513 (jou) : For SCK ART can do manual RT
                    Mode==rsmCInitialRetest)
            {
            }
            else
            {
                Mode=rsmInitial_ART;
            }
        }
        else if(Mode==rsmInitial_ART      ||
                Mode==rsmContinuStart_ART ||
                Mode==rsmAutoRetest       ||
                Mode==rsmContinuRetest_ART)
        {
            Mode=rsmInitialStart;
        }
    }
    else
    {
         if(IniConfig.bA10_AutoReTest==false &&
            (Mode==rsmInitial_ART      ||
             Mode==rsmContinuStart_ART ||
             Mode==rsmAutoRetest       ||
             Mode==rsmContinuRetest_ART))
        {
            Mode=rsmInitialStart;
        }
    }

    if(Mode==rsmInitial_MRT || Mode==rsmContinuStart_MRT || Mode==rsmRetest_MRT)                                        //Ifor 20170504 (wei) add 無使用MRT客戶且測試模式為MRT 強制切至Initial Start
    {
        if(CosFunction.bUseMRTMode==false || TestIF_File.bEnableMRTMode==false)                                         //Ifor 20170504 MRT 功能關閉 Start Mode 強制切換至Initial Start
        {
            Mode=rsmInitialStart;
        }
    }

    if(LastSet.iRunStartMode==rsmAutoSiteMap && Mode!=rsmAutoSiteMap)
    {
        RecordAutoSiteMapFinish();                                              //Steven 20230117 : 修正Auto Site map的訊息
    }

    if(CosFunction.bVerifyMode)                                                 //Sam 20231117 : 整合到 QA 模式
    {
        // ⛔ SAFETY-GATE(W906-P10-QABACKUP) —— 缺相依：`QABackupStatus(bool)` **沒有定義**。
        //
        //  它有宣告（forms/fQAMode.h），但**本體被本樹自己閘住**：
        //  `forms/fQAMode.cpp:413 #if 0 // GATE (Q-F) QABackupStatus -- golden :285-335 (51L)`。
        //  ⇒ 這是「符號存在的三級」裡的第二級問題：編得過、連不起來。
        //    （20260921 實測：不閘的話全量建置噴 13 個
        //      `undefined reference to QABackupStatus(bool)`。）
        //
        //  行為（機台的話）：進/出 **QA 模式**時，**不會備份／還原 Site On-Off 狀態**。
        //    切進 QA 模式前關掉的站，切回來之後不會自動恢復成原本的開關組合。
        //    ⚠ 只有 `CosFunction.bVerifyMode==true` 的機台走得到這裡。
        //
        //  UN-GATE：與 `forms/fQAMode.cpp:413` 的 GATE (Q-F) **同一顆**，
        //    那 51 行解開時本處一起解。
#if 0 // GATE (W906-P10-QABACKUP): 缺相依（本體被 GATE (Q-F) 閘住），見上面的就地註解
        if(LastSet.iRunStartMode!=rsmQAMode && Mode==rsmQAMode)                 //其他模式切換到  QA模式備份目前Site On Off 狀態
            QABackupStatus(true);
        else if(LastSet.iRunStartMode==rsmQAMode && Mode!=rsmQAMode)            //  QA模式切換到其他模式恢復先前Site On Off 狀態
            QABackupStatus(false);
#endif // GATE (W906-P10-QABACKUP)
    }

    // Steven 20260521 : V904.5 P260518-ATK-H9-02 - When leaving QA Mode (any path other than
    // TrayFeed/TrayEnd which have their own restore), restore tester to Lot-Start backup mode.
    // Skips when residual CleanOut still running (iCleanOut!=0) to keep main.cpp:16684 Bin Routing.
    if(bQAModeFinishCleanOut==true &&
       LastSet.iRunStartMode==rsmQAMode &&
       Mode!=rsmQAMode && Mode!=rsmNull &&
       iCleanOut==0)
    {
        IniConfig.bQAModeFirstIn = true;
        fMain->ModifyTester(IniConfig.iBackUpTesterMode);
        if(LastSet.iTester==ON_LINE)
            NewRecordProcess("MES2157", "Change to On_Line", "by SetRunStartMode leaving QAMode, restore to LotStart mode");
        else
            NewRecordProcess("MES2155", "Change to Off_Line", "by SetRunStartMode leaving QAMode, restore to LotStart mode");
        ArmSpeed[InArm].bVariModeFIX = IniConfig.bBackUpInArmMode;
        TrayForm.bAutoFeed           = IniConfig.bBackUpAutoFeed;
        bQAModeFinishCleanOut = false;
        bQAModeQuickCleanOut  = false;
    }

    if(Mode!=rsmNull)                                                           //Steven 20120615
    {
        LastSet.iRunStartMode=int(Mode);
        fMain->cbRunStartMode->Text=StartModeName[Mode];
    }

    if(TestIF_File.iTestMode==SingleSite)                                       //Steven 20120912 : Single Site不需要作Auto Site Mapping
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap)                               //jou 20220222 : 修正Single site Auto site mapping FT/RT mode 切換錯誤。
        {
            if(iAutoSiteMapRunStartMode==0)
            {
                LastSet.iRunStartMode=rsmContinuStart;
                fMain->cbRunStartMode->Text=StartModeName[rsmContinuStart];
            }
            else
            {
                LastSet.iRunStartMode=rsmContinuRetest;
                fMain->cbRunStartMode->Text=StartModeName[rsmContinuRetest];
            }
        }
    }

    if(CosFunction.bUseSCKART || IniConfig.bShowFTandRTButton)                  //Sam 20250122 : 修正 FT/RT 顯示問題。
    {
        fMain->palFT->Visible=true;
        fMain->palRT->Visible=true;
    }
    fMain->palEQC->Visible=(IniConfig.bSPILFunction && CosFunction.bCanDisableQAMode &&IniConfig.bA51EnableEQCMode);    //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode
    fMain->palFT->Color=clBtnFace;
    fMain->palRT->Color=clBtnFace;
    fMain->palEQC->Color=clBtnFace;                                             //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
    fMain->palFT->Caption="FT";
    fMain->palRT->Caption="RT";
    fMain->palEQC->Caption="EQC";                                               //JerryYang 20190701 SPIL要求主畫面可切換EQC mode

    fMain->palEQC->Visible=(IniConfig.bSPILFunction && CosFunction.bCanDisableQAMode &&IniConfig.bA51EnableEQCMode);    //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode

    fMain->palOffLine->Color=clBtnFace;

    if(CUSTOMER_CODE==CC_VTEST_Shanghai)                                        //jou 20230713 : 上海VTEST 張冬冬要求clean out需把loader料清空
        bMustCleanAllTray=true;
    else
        bMustCleanAllTray=false;                                                //ChungHung 20141002 add for KYEC AutoRetest

    LastSet.bLoaderTrayCount_ART=false;                                         //ChungHung 20141002 add for KYEC AutoRetest
    if(IniConfig.bSPILFunction && bCanRunSCKART==true)                          //JerryYang 20220923 : SECS GEM版本ART
    {
        if(Mode==rsmInitial_ART)
        {
            fSCKART->ClearLotInfo();
        }
    }

    if(LastSet.iRunStartMode==rsmContinuStart ||
       LastSet.iRunStartMode==rsmInitialStart ||
       LastSet.iRunStartMode==rsmAutoSiteMap  ||
       LastSet.iRunStartMode==rsmQAMode       )
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap &&
           //IniConfig.bVTESTFunction==true &&                                  //Steven 20230410 : Mark
           IniConfig.bUseAutoSiteMapping && IniConfig.bI21EnableASM && iAutoSiteMapRunStartMode==1)
        {
            fMain->palRT->Color=clRed;
            iRunStartMode=RT;
            iTestRunMode=RT;
        }
        else
        {
            fMain->palFT->Color=clLime;
            iRunStartMode=FT;
            iTestRunMode=FT;
        }

        bForKyecBu3RunART=false;                                                //wei 20151210 //wei 20161118 bRunART-->bForKyecBu3RunART
        iContsFailIgnoreCount=0;
        bIsShowPMAlarmMessage=true;                                             //wei 20160225 20150720 Mylin Modify PM Alarm Show One Time
        if(CUSTOMER_CODE==CC_PTI &&                                             //RogerYang 20170417 LotInfo更新至FT Mode
           IniConfig.bB03_TesterReport==false)                                  //Sam 20240809 : PTI ART 模式
        {
            fLotInfo->cbRunMode->Text="Normal";
        }

        if(CUSTOMER_CODE==CC_SIGURD_PeiXing &&
           LastSet.iRunStartMode==rsmQAMode)                                    //Sam 20240122 : 北興俊堯要求QA要用 RT
        {
            iRunStartMode=RT;
            iTestRunMode=RT;
        }
        else if(LastSet.iRunStartMode==rsmQAMode &&
                IniConfig.bSPILFunction==true)                                  //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
        {
            fMain->palEQC->Color=clYellow;
            fMain->palFT->Color=clBtnFace;
        }

        if(LastSet.iRunStartMode==rsmAutoSiteMap)
            RecordAutoSiteMapStart();                                           //Steven 20230117 : 修正Auto Site map的訊息
    }
    else if(LastSet.iRunStartMode==rsmContinuRetest ||
            LastSet.iRunStartMode==rsmCInitialRetest)
    {
        fMain->palRT->Color=clRed;
        if(CosFunction.bDisableRTBinSet)                                        //wei 20150622 不顯示設定RT Bin set
            iTestRunMode=FT;
        else
            iTestRunMode=RT;
        iRunStartMode=RT;
        bForKyecBu3RunART=false;                                                //wei 20151210 //wei 20161118 bRunART-->bForKyecBu3RunART

        if(CUSTOMER_CODE==CC_PTI &&                                             //RogerYang 20170417 LotInfo更新至FT Mode
           IniConfig.bB03_TesterReport==false)                                  //Sam 20240809 : PTI ART 模式
        {
            fLotInfo->cbRunMode->Text="Re-Test";
        }
    }
    else if(LastSet.iRunStartMode==rsmInitial_ART      ||                       //ChungHung 20141002 add for KYEC AutoRetest
            LastSet.iRunStartMode==rsmContinuStart_ART ||
            LastSet.iRunStartMode==rsmAutoRetest       )
    {
        if(CosFunction.bUseSCKART)                                              //Steven 20161214 (wei) : For SCK ART
        {
            if(IniConfig.bSPILFunction)                                         //JerryYang 20220923 : 矽品版本ART
            {
                fMain->palFT->Visible=true;
                fMain->palRT->Visible=false;
                fMain->palEQC->Visible=true;

                fMain->palFT->Caption="FT-ART";
                fMain->palEQC->Caption="EQC-ART";

                if(fSCKART->sInfo_Stage.Pos("QC")==1)
                {
                    fMain->palEQC->Color=clYellow;
                    fMain->palFT->Color=clBtnFace;
                }
                else
                {
                    fMain->palEQC->Color=clBtnFace;
                    fMain->palFT->Color=clLime;
                }
            }
            else
            {
                if(LastSet.iRunStartMode==rsmInitial_ART)                       //Steven 20170309 (wei) : Fixed for ART
                {
                    fMain->palFT->Color=clLime;
                }
                else if(fSCKART->iFTRTCount==0)
                {
                    fMain->palFT->Color=clLime;
                }
                else
                {
                    fMain->palRT->Color=clRed;
                }
            }
        }
        else
        {
            fMain->palFT->Color=clLime;
        }
        iRunStartMode=FT;

        if(CosFunction.bUseSCKART)                                              //Steven 20161214 (wei) : For SCK ART
            iTestRunMode=FT;
        else
            iTestRunMode=FT_ART;

        if(CosFunction.bAutoRetestGPIBmode==false)                              //jou 2015-10-02 Auto Retest GPIB mode
            bMustCleanAllTray=true;
        else
            TrayForm.bAutoFeed=false;

        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            LastSet.bLoaderTrayCount_ART=true;
//            bForKyecBu3RunART=true;                                           //wei 20151210 //wei 20161118 bRunART-->bForKyecBu3RunART
            if(Sen[SnLoaderTrayHasTray_ART].Enable)                             //Ifor 20191125 : add ART Loader 雙 Sensor mode
            {
                bForKyecBu3RunART=true;
            }
            else
            {
                bForKyecBu3RunART=false;
            }
        }
        else if(CUSTOMER_CODE==CC_AMKOR_Japan)                                  //RogerYang 20251110 : 瑞薩FTCT
        {
            LastSet.bLoaderTrayCount_ART=true;
        }
        else
        {
            bForKyecBu3RunART=false;                                            //wei 20151210 //wei 20161118 bRunART-->bForKyecBu3RunART
        }

        if(LastSet.iRunStartMode==rsmInitial_ART)                               //ChungHung 20150511 modify
        {
            fSCKART->iCurrent93KARTStep=0;
            fSCKART->iCurrentFlexARTStep=0;
            if(CUSTOMER_CODE==CC_KYEC_LEE && IniConfig.bA10_AutoReTest==false)
            {
                ShowMyMessage("No Run ART Mode", "不能生產ART模式");
                SetRunStartMode(rsmInitialStart);
            }
            else
            {
                if(IniConfig.bEnable_SECS_GEM==true)
                    EventReport(SECS_EVENT.InitialArtStart);                    //55     切換成InitART模式成功時
                iRunStartMode=FT;
            }

            if(CUSTOMER_CODE==CC_PTI &&                                         //RogerYang 20170417 LotInfo更新至FT Mode
               IniConfig.bB03_TesterReport)                                     //Sam 20240930 : 修正報表檔名 //Sam 20240809 : PTI ART 模式
            {
                fLotInfo->cbRunMode->Text="1'st";
            }
        }
        else if(LastSet.iRunStartMode==rsmContinuStart_ART)
        {
            //==> Eastsun 20260512 F010 整合
            if(IniConfig.bEnable_SECS_GEM==true)
            {
                if(TrayForm.bEnableAMR)
                {
                    EventReport(SECS_EVENT.ArtReceiveTrayOK);                   //Eastsun 20260414
                }
                EventReport(SECS_EVENT.ReadyForArt);                            //58    //wei 20150826
            }
            //<== Eastsun 20260512 F010 整合

            if(CosFunction.bUseSCKART)                                          //Steven 20170516 (Jou) : Add For SCK ART
            {
                if(TestIF_File.bRENESAS_EnableFTCT==true &&                     //RogerYang 20251018 : FTCT Need Do RT(loader tray to Color)
                    fSCKART->iFTRTCount>0)
                    iRunStartMode=RT;
            }
            else
            {
                if(LastSet.iAutoRetestCount_ART>=1)                             //wei 20150923 add ART計數
                    iRunStartMode=RT;
            }
        }
        else
        {
            if(CosFunction.bDisableRTBinSet)                                    //wei 20150622 不顯示設定RT Bin set
                iRunStartMode=RT;
        }
    }
    else if(LastSet.iRunStartMode==rsmContinuRetest_ART)                        //ChungHung 20141002 add for KYEC AutoRetest
    {
        if(CosFunction.bUseSCKART)                                              //Steven 20161214 (wei) : For SCK ART
        {
            if(IniConfig.bSPILFunction)                                         //JerryYang 20220923 : 矽品版本ART
            {
                fMain->palFT->Visible=true;
                fMain->palRT->Visible=false;
                fMain->palEQC->Visible=true;

                fMain->palFT->Caption="FT-ART";
                fMain->palEQC->Caption="EQC-ART";
                iRunStartMode=RT;

                if(fSCKART->sInfo_Stage.Pos("QC")==1)
                {
                    fMain->palEQC->Color=clYellow;
                    fMain->palFT->Color=clBtnFace;
                }
                else
                {
                    fMain->palEQC->Color=clBtnFace;
                    fMain->palFT->Color=clLime;
                }
            }
            else
            {
                fMain->palRT->Color=clRed;
                iRunStartMode=RT;
            }

            if(TestIF_File.iSCKART_SortMode==1)
            {
                if(CUSTOMER_CODE==CC_SCK)
                {
                    iTestRunMode=RT;
                }
                else
                {
                    iTestRunMode=FT;
                }
            }
            else
            {
                iTestRunMode=RT;
            }
        }
        else if(CosFunction.bDisableRTBinSet)
        {
            fMain->palFT->Color=clLime;
            iRunStartMode=RT;
            iTestRunMode=FT;
        }
        else
        {
            fMain->palRT->Color=clRed;
            iRunStartMode=RT;
            iTestRunMode=RT_ART;
        }

        if(CosFunction.bAutoRetestGPIBmode==false)                              //jou 2015-10-02 Auto Retest GPIB mode
            bMustCleanAllTray=true;
        else
            TrayForm.bAutoFeed=false;

        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            LastSet.bLoaderTrayCount_ART=true;
//            bForKyecBu3RunART=true;                                           //wei 20151210 //wei 20161118 bRunART-->bForKyecBu3RunART
            if(Sen[SnLoaderTrayHasTray_ART].Enable)
            {
                bForKyecBu3RunART=true;
            }
            else
            {
                bForKyecBu3RunART=false;
            }
        }
        else if(CUSTOMER_CODE==CC_AMKOR_Japan)                                  //RogerYang 20251110 : 瑞薩FTCT
        {
            LastSet.bLoaderTrayCount_ART=true;
        }
        else
        {
            bForKyecBu3RunART=false;                                            //wei 20151210 //wei 20161118 bRunART-->bForKyecBu3RunART
        }

        if(IniConfig.bEnable_SECS_GEM==true)                                    //wei 20150630 修改rsmContinuRetest_ART不送SwitchStartMode EVENT
        {
            EventReport(SECS_EVENT.SwitchStartMode);
            //==> Eastsun 20260512 F010 整合
            if(TrayForm.bEnableAMR)
            {
                EventReport(SECS_EVENT.ArtReceiveTrayOK);                       //Eastsun 20260414
            }
            //<== Eastsun 20260512 F010 整合
            EventReport(SECS_EVENT.ReadyForArt);                                //58    //wei 20150826
        }
    }
    else if(LastSet.iRunStartMode==rsmInitial_MRT      ||                       //Ifor 20170316 (wei) add KYEC MRT Mode
            LastSet.iRunStartMode==rsmContinuStart_MRT )
    {
        fMain->palFT->Color=clLime;
        iRunStartMode=FT;
        iTestRunMode=FT_MRT;
        bForKyecBu3RunART=false;
    }
    else if(LastSet.iRunStartMode==rsmRetest_MRT)                               //ChungHung 20141002 add for KYEC AutoRetest
    {
        fMain->palRT->Color=clRed;
        iRunStartMode=RT;
        iTestRunMode=RT_MRT;
        bForKyecBu3RunART=false;
    }

    if(CosFunction.bOffLineBin && LastSet.iTester==OFF_LINE &&                  //ChungHung 20140902 QA mode 一樣變換
       !(LastSet.iRunStartMode==rsmQAMode &&                                    //Steven 20260514 : QA CleanOut keep FT bin table
         (Prod.iQAModeRunType==0 || Prod.iQAModeRunType==2) &&
         bQAModeFinishCleanOut==true))
         {
        //==> Eastsun 20260512 F010 整合
        if(TrayForm.bEnableAMR)
        {
        }
        else
        {
            fMain->palOffLine->Color=clRed;
            iTestRunMode=OffT;
        }
        //<== Eastsun 20260512 F010 整合
        }

    if(LastSet.iRunStartMode!=rsmAutoSiteMap)
    {
        fMain->lbSetOpenBin->Visible=false;
        fMain->edSetOpenBin->Visible=false;
    }
    else
    {
        if(CosFunction.bUSEJCETSiteMapMode==true)                               //jou 2016-10-28 JCET 要求Site Mapping 必須測試到pass bin才能通過
        {
            fMain->lbSetOpenBin->Visible=false;
            fMain->edSetOpenBin->Visible=false;
        }
        else
        {
            fMain->lbSetOpenBin->Visible=true;
            fMain->edSetOpenBin->Visible=true;
        }

        fMain->edSetOpenBin->Text="";
        bSiteMappingCHKOK=false;
        bAutoSiteMapHotICCanPick=false;                                         //Ifor 20171225 (Steven) : add 避免 One Cycle 後執行Site Mapping 發生 Hang up
        bAutoSiteMapHotplateReady=false;                                        //Ifor 20180507 : add 避免One Cycle 後啟動Auto Site Mapping補料異常
        fMain->ReStartAutoSiteMapping(true);
        if(IniConfig.bI21EnableASM)                                             //Steven 20120207 : 只有Auto Site Mapping啟動時才要強制disable
        {
            fMain->cbRunStartMode->Enabled=false;
            fMain->cbbRunModeSel->Enabled=false;
            fBinSel->cbUseMRTMode->Enabled=false;                               //Ifor 20170414 (wei) add 鎖定 mrt 模式不可修改
        }
    }

    if(IniConfig.bQAMode==true && LastSet.iRunStartMode==rsmQAMode)             //從RunStartModeChange移過來-----------------------
    {
        IniConfig.bQAModeFirstIn=true;
    }

    if(IniConfig.bInitialStartNeedAsk)                                          //JerryYang 20180626 (wei) : MicroChip要求initial要給OP確認site mapping
    {
        if(LastSet.iRunStartMode==rsmInitialStart ||
           LastSet.iRunStartMode==rsmCInitialRetest)
            bNeedAskStartMode=true;
        else
            bNeedAskStartMode=false;
    }
    else
    {
        bNeedAskStartMode=false;
    }

    if(IniConfig.bA37LotStartLotEnd==true)                                      //JerryYang 20220923 : 矽品版本ART
    {
        if(LastSet.iRunStartMode==rsmInitialStart ||
           LastSet.iRunStartMode==rsmCInitialRetest)
        {
            fSCKART->ClearLotInfo();
            slDupBundlID->Clear();                                              //JerryYang 20250220 : AUTO IN OUT
            slDupBundlID->SaveToFile(asDupBundleID);

            slDupUnloadBundlID->Clear();
            slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);
        }
    }

    fMain->UpdateMainOperateMode();

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        for(int k=0; k<3; k++)
        {
            szDir=DataPath+sFileName[k]+"\\Binasgn.Data";
            if(FileExists(szDir))                                               //kevin 20160928 ASE_KH FT QA RT BIN資料檔暫時性轉換
            {
                fBinSel->ReadFile(false, true, sFileName[k]);                   //kevin 20160928 //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
            }
        }
    }
    fBinSel->ReadFile(false, false, "");                                        //kevin 20160928 //Steven 20160623 : 當IniConfig.bFTBin2RTBin==true && 讀檔時, RT Bin要跟FT Bin一樣
    fShowBinSelect->ShowBinSel();
    fShowBinSelect->InitShowBinDigital();

    // ⛔ SAFETY-GATE(W906-P10-YIELDREAD) —— golden main.cpp 這一行沒翻。
    //
    //  缺的相依：`TfYieldMonitoring::ReadFile()`。本樹的 `TfYieldMonitoring`
    //  （forms/fYieldMonitoring.h:218）門面沒有這個成員；golden 的本體是
    //  **806 行**，比 P10 主體（839 行）還大一半，而且它自己的相依我沒量過。
    //
    //  行為（機台的話）：切開工模式（FT/RT/EQC/OffLine）時，**良率統計畫面
    //  的數字不會跟著重讀**，會停在切換前的舊值，要等別的地方觸發重讀才更新。
    //  模式切換本身、旗標、畫面顏色、SECS 事件都不受影響。
    //  ⚠ 只有 `IniConfig.bShowFunctionWindow==true` 的機台走得到這裡。
    //
    //  使用者 20260921 08:0x 裁決（INBOX Q21）：**甲 —— 只閘這一行**，
    //  不在這一波補那 806 行。原話：「Q20用甲，P10也用甲」。
    //
    //  UN-GATE：等 `TfYieldMonitoring::ReadFile()` 有真本體（另一個波次）。
    if(IniConfig.bShowFunctionWindow)                                           //jou 2010-12-04
    {
        // GATE(W906-P10-YIELDREAD) RESOLVED -- AI(W906-YM-READFILE) 20260923.
        // The gate's stated un-gate condition was literally "wait until
        // TfYieldMonitoring::ReadFile() has a real body (another wave)".  It now
        // has one (uYieldMonitoring.cpp, golden uYieldMonitoring.cpp:895-1700,
        // 806 lines, all 253 ReadIniData ACTIVE).  So the disclosed behaviour is
        // fixed too: switching run mode (FT/RT/EQC/OffLine) re-reads the yield
        // settings instead of leaving the display on pre-switch values.
        fYieldMonitoring->ReadFile();                                           //jou 2010-12-04
    }

    fMain->LoadStartModePicture();

    if(CUSTOMER_CODE==CC_SCK && Mode==rsmCInitialRetest &&                      //Steven 20170825 (wei) : Add for SCK ART need to setting RT count when select Initial_RT
       fSCKART->iTesterType==0 && bCanRunSCKART)                                //Isaac 20180328 (Steven) 沒ART功能不用輸入RT count
    {
        // AI(W906-P10) 20260921: golden 是 `new TEdit(fMain)` —— VCL 的
        //   TComponent ctor 收一個 owner，由 owner 負責解構。
        //   本樹的 `vclcompat::TEdit`（vclcompat/Controls.h:330）**沒有 owner ctor**，
        //   因為離線門面沒有 VCL 的所有權樹。下一行的 `delete tempEdit;` 是
        //   golden 自己就有的，所以少了 owner 也不會漏記憶體。
        //   ⇒ 這是**有記錄的移植偏離**，不是漏翻。
        TEdit *tempEdit=new TEdit();
        fQwertyKey->ShowQwertyKey(tempEdit, N_INTEGER, 0, true, 0, 1000);
        fSCKART->iFTRTCount=atoi(tempEdit->Text.c_str());
        fSCKART->SetLotStatus(fSCKART->iLOTSTATUS_R);
        fSCKART->AccessFile(false, -1);
        delete tempEdit;
    }

    if(bCanRunSCKART==true)                                                     //Steven 20170321 (wei) : Clear Lot Info when change to Init ART
    {
        if(Mode==rsmInitial_ART)
        {
            if(bQAModeFlag==false)
            {
                fSCKART->iWaitGPIBLotR=0;
                fSCKART->ClearLotInfo();
                fSCKART->DoAutoSocketOff(true);
                if(fMain->hanaART->IsHanaArtAvailable()==true)
                {
                    fMain->hanaART->Clear();
                    int iHD_Mode=IniConfig.iA10_6_HANA_ART_TestMode;            //JimmyChiu 20250214 : For Hana ART
                    fMain->hanaART->SetHandlerWaitingData(IniConfig.sMachineType,
                                                          IniConfig.SocketHandlerID,
                                                          IniConfig.sMachineType,
                                                          iHD_Mode);
                }
            }
        }
        else
        {
            if(Mode==rsmQAMode)
            {
                fSCKART->iWaitGPIBLotR=0;
                fSCKART->ClearLotInfo();
                fSCKART->DoAutoSocketOff(true);
            }
        }
    }

    if(IniConfig.bShowFTandRTButtonCanClick==true)                              //Steven 20131224 : FT & RT Buttion 可以按
    {
        if(Mode==rsmQAMode && IniConfig.bSPILFunction==true)                    //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
            fMain->cbbRunModeSel->Text=StartModeName[5];
        else if(Mode==rsmContinuStart || Mode==rsmContinuRetest)
            fMain->cbbRunModeSel->Text=StartModeName[0];
        else if(Mode==rsmInitialStart || Mode==rsmCInitialRetest)
            fMain->cbbRunModeSel->Text=StartModeName[1];
    }

    if(CosFunction.bAutoCloseSiteWhenRT)
    {
        if(Mode==rsmCInitialRetest)
        {
            if(TestIF_File.iAutoCloseSiteWhenRT)                                //Steven 20200205 : 切到RT的時候,要關閉Socket
            {
                fYieldMonitoring->DoRTAutoSocketOff();                          //Steven 20230814 : 統一自動關Site Function
            }
        }
    }

    fMain->SaveRunMode();                                                       //JerryYang 20161027 把Run mode存起來
    fSCKART->AccessFile(true);                                                  //Steven 20161201 (wei) : For SCK 93K ART
    SetWorkParameter();                                                         //Steven 20170113 (Jou) : 避免Prod.CatData沒有被轉換，導致分Bin異常
    if(CosFunction.bFTRTDifferentDutOnOff ||                                    //JerryYang 20170516 (wei) JSCC要求FT RT要有不同的開關site
       CosFunction.bLastSetInSetUpFile==true)                                   //Steven 20200703 : Add for test mode change while download setup file.
    {
        ReadTestMode();
        fMain->ShowTestHeadComp(false);
    }

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Ifor 20190711 : add Run Mode 改變都需要上報SECS GEM EVEN
    {
        EventReport(SECS_EVENT.SwitchStartMode);
    }

    // ⛔ SAFETY-GATE(W906-P10-MONITORPARAM) —— 缺相依：
    //  `TfRPDefault::ShowMonitoredParameter()` **沒有定義**。
    //  宣告在 `forms/fRPDefault.h:352`，本體被本樹自己閘住：
    //  `forms/fRPDefault.cpp:510 #if 0 // GATE (R-7) ShowMonitoredParameter -- golden :396-415`。
    //
    //  ⚠⚠ 那個閘的理由不只是「沒翻」——`forms/fRPDefault.h:352` 的行尾就寫著
    //    **「WRITES CurrentSetupData.txt」**。那是機台共用的執行期設定檔
    //    （CLAUDE.md 的寫入邊界把它列為預設只讀）。
    //    ⇒ 解它不是翻譯問題，是 write-path 問題，要另外決定。
    //
    //  行為（機台的話）：**只有全智（CC_GIGAS）的機台**受影響 ——
    //    切換開工模式時不會把指定的監控參數寫出到 `CurrentSetupData.txt`。
    //    其他客戶的機台行為完全相同（`CUSTOMER_CODE` 第一項就是 false）。
    //
    //  UN-GATE：與 `forms/fRPDefault.cpp:510` 的 GATE (R-7) 同一顆，
    //    而且要先有 write-path 的裁決。
    if(CUSTOMER_CODE==CC_GIGAS)                                                 //Isaac 20211026 : 全智要把指定參數存出來，Start Mode(LastSet.iRunStartMode)
    {
#if 0 // GATE (W906-P10-MONITORPARAM): 缺相依（本體被 GATE (R-7) 閘住，且會寫 CurrentSetupData.txt）
        fRPDefault->ShowMonitoredParameter();
#endif // GATE (W906-P10-MONITORPARAM)
    }

    //Steven 20211102 : 保持在最下面
    NewRecordProcess("MES2107", "Change Start Mode", fMain->cbRunStartMode->Text);                                      //Steven 20170317 : 換位置紀錄
    NewRecordProcess("ChangeLog", "Change bin mode", BinSetting[iTestRunMode]);                                         //Steven 20170317 : 新增紀錄
    if(LastSet.iRunStartMode==rsmInitialStart ||
       LastSet.iRunStartMode==rsmCInitialRetest)                                //Sam 20231222 : Initial Start 時就要上傳資料
        bHandlerChangeState=true;
    fMain->ChangeStateUploadServer();
}

// ===========================================================================
//  AI(W906-W2-SETTESTRUNMODE) 20260926: SetTestRunMode —— golden main.cpp:1117-1338（222 行）逐行照搬。
//  W1 普查（docs/CHANNEL_CENSUS_20260924.md 第 46 行）：TfMain::ModifyTester 是空殼（forms/fMain.cpp），
//  而 golden 的 ModifyTester（main.cpp:12056-12062）就是「改 LastSet.iTester → SetTestRunMode() → LoadTestModePicture()」；
//  這支在移植樹不存在（全樹 0 處），run.*／runmode.* 6 個 tag 也因為開機沒人設定 iTestRunMode 而維持 null。
//  放這支檔的理由同檔頭：它在 golden 緊接在 SetRunStartMode（:363-1115）之後，用的是同一批 fMain／fSCKART／fLotInfo 元件。
//  golden main.cpp:233 是它的 extern 宣告；移植樹的呼叫端（ModifyTester）用區塊範圍的 extern 宣告。
// ===========================================================================
void SetTestRunMode()                                                           //JerryYang 20230204 : 更新iTestRunMode
{
    if(LastSet.iRunStartMode==rsmContinuStart ||
       LastSet.iRunStartMode==rsmInitialStart ||
       LastSet.iRunStartMode==rsmAutoSiteMap  ||
       LastSet.iRunStartMode==rsmQAMode       )
    {
        if(LastSet.iRunStartMode==rsmAutoSiteMap &&
            //IniConfig.bVTESTFunction==true &&                                 //Steven 20230410 : Mark
           IniConfig.bUseAutoSiteMapping && IniConfig.bI21EnableASM && iAutoSiteMapRunStartMode==1)
        {
            fMain->palRT->Color=clRed;
            iRunStartMode=RT;
            iTestRunMode=RT;
        }
        else
        {
            fMain->palFT->Color=clLime;
            iRunStartMode=FT;
            iTestRunMode=FT;
        }

        if(CUSTOMER_CODE==CC_PTI &&                                             //RogerYang 20170417 LotInfo更新至FT Mode
           IniConfig.bB03_TesterReport==false)                                  //Sam 20240809 : PTI ART 模式
        {
            fLotInfo->cbRunMode->Text="Normal";
        }

        if(LastSet.iRunStartMode==rsmQAMode && IniConfig.bSPILFunction==true)   //JerryYang 20190701 SPIL要求主畫面可切換EQC mode
        {
            fMain->palEQC->Color=clYellow;
            fMain->palFT->Color=clBtnFace;
        }

        if(IniConfig.bA60EnableAMR==true)                                       //Sam 20240827 : 新增 AMR 功能
            TrayForm.bAutoFeed=true;
    }
    else if(LastSet.iRunStartMode==rsmContinuRetest ||
            LastSet.iRunStartMode==rsmCInitialRetest)
    {
        fMain->palRT->Color=clRed;
        if(CosFunction.bDisableRTBinSet)                                        //wei 20150622 不顯示設定RT Bin set
            iTestRunMode=FT;
        else
            iTestRunMode=RT;
        iRunStartMode=RT;

        if(CUSTOMER_CODE==CC_PTI &&                                             //RogerYang 20170417 LotInfo更新至FT Mode
           IniConfig.bB03_TesterReport==false)                                  //Sam 20240809 : PTI ART 模式
        {
            fLotInfo->cbRunMode->Text="Re-Test";
        }

        if(IniConfig.bA60EnableAMR==true)                                       //Sam 20240827 : 新增 AMR 功能
            TrayForm.bAutoFeed=true;
    }
    else if(LastSet.iRunStartMode==rsmInitial_ART      ||                       //ChungHung 20141002 add for KYEC AutoRetest
            LastSet.iRunStartMode==rsmContinuStart_ART ||
            LastSet.iRunStartMode==rsmAutoRetest       )
    {
        if(CosFunction.bUseSCKART)                                              //Steven 20161214 (wei) : For SCK ART
        {
            if(IniConfig.bSPILFunction)                                         //JerryYang 20220923 : 矽品版本ART
            {
                fMain->palFT->Visible=true;
                fMain->palRT->Visible=false;
                fMain->palEQC->Visible=true;

                fMain->palFT->Caption="FT-ART";
                fMain->palEQC->Caption="EQC-ART";

                if(fSCKART->sInfo_Stage.Pos("QC")==1)
                {
                    fMain->palEQC->Color=clYellow;
                    fMain->palFT->Color=clBtnFace;
                }
                else
                {
                    fMain->palEQC->Color=clBtnFace;
                    fMain->palFT->Color=clLime;
                }
            }
            else
            {
                if(LastSet.iRunStartMode==rsmInitial_ART)                       //Steven 20170309 (wei) : Fixed for ART
                {
                    fMain->palFT->Color=clLime;
                }
                else if(fSCKART->iFTRTCount==0)
                {
                    if(fMain->hanaART->IsHanaArtAvailable())                    //Steven 20250414 : HANA ART Function
                    {
                        if(fMain->hanaART->IsPrimeTest())
                            fMain->palFT->Color=clLime;
                        else
                            fMain->palRT->Color=clRed;
                    }
                    else
                    {
                        fMain->palFT->Color=clLime;
                    }
                }
                else
                {
                    fMain->palRT->Color=clRed;
                }
            }
        }
        else
        {
            fMain->palFT->Color=clLime;
        }
        iRunStartMode=FT;

        if(CosFunction.bUseSCKART)                                              //Steven 20161214 (wei) : For SCK ART
            iTestRunMode=FT;
        else
            iTestRunMode=FT_ART;
    }
    else if(LastSet.iRunStartMode==rsmContinuRetest_ART)                        //ChungHung 20141002 add for KYEC AutoRetest
    {
        if(CosFunction.bUseSCKART)                                              //Steven 20161214 (wei) : For SCK ART
        {
            if(IniConfig.bSPILFunction)                                         //JerryYang 20220923 : 矽品版本ART
            {
                fMain->palFT->Visible=true;
                fMain->palRT->Visible=false;
                fMain->palEQC->Visible=true;

                fMain->palFT->Caption="FT-ART";
                fMain->palEQC->Caption="EQC-ART";
                iRunStartMode=RT;

                if(fSCKART->sInfo_Stage.Pos("QC")==1)
                {
                    fMain->palEQC->Color=clYellow;
                    fMain->palFT->Color=clBtnFace;
                }
                else
                {
                    fMain->palEQC->Color=clBtnFace;
                    fMain->palFT->Color=clLime;
                }
            }
            else
            {
                fMain->palRT->Color=clRed;
                iRunStartMode=RT;
            }

            if(TestIF_File.iSCKART_SortMode==1)
            {
                if(CUSTOMER_CODE==CC_SCK)
                {
                    iTestRunMode=RT;
                }
                else
                {
                    iTestRunMode=FT;
                }
            }
            else
            {
                iTestRunMode=RT;
            }
        }
        else if(CosFunction.bDisableRTBinSet)
        {
            fMain->palFT->Color=clLime;
            iRunStartMode=RT;
            iTestRunMode=FT;
        }
        else
        {
            fMain->palRT->Color=clRed;
            iRunStartMode=RT;
            iTestRunMode=RT_ART;
        }
    }
    else if(LastSet.iRunStartMode==rsmInitial_MRT      ||                       //Ifor 20170316 (wei) add KYEC MRT Mode
            LastSet.iRunStartMode==rsmContinuStart_MRT )
    {
        fMain->palFT->Color=clLime;
        iRunStartMode=FT;
        iTestRunMode=FT_MRT;
    }
    else if(LastSet.iRunStartMode==rsmRetest_MRT)                               //ChungHung 20141002 add for KYEC AutoRetest
    {
        fMain->palRT->Color=clRed;
        iRunStartMode=RT;
        iTestRunMode=RT_MRT;
    }

    if(CosFunction.bOffLineBin && LastSet.iTester==OFF_LINE &&                  //ChungHung 20140902 QA mode 一樣變換
       !(LastSet.iRunStartMode==rsmQAMode &&                                    //Steven 20260514 : QA CleanOut keep FT bin table
         (Prod.iQAModeRunType==0 || Prod.iQAModeRunType==2) &&
         bQAModeFinishCleanOut==true))
    {
        fMain->palOffLine->Color=clRed;
        iTestRunMode=OffT;
    }

    if(LastSet.iRunStartMode!=rsmAutoSiteMap)
    {
        fMain->lbSetOpenBin->Visible=false;
        fMain->edSetOpenBin->Visible=false;
    }
    else
    {
        if(CosFunction.bUSEJCETSiteMapMode==true)                               //jou 2016-10-28 JCET 要求Site Mapping 必須測試到pass bin才能通過
        {
            fMain->lbSetOpenBin->Visible=false;
            fMain->edSetOpenBin->Visible=false;
        }
        else
        {
            fMain->lbSetOpenBin->Visible=true;
            fMain->edSetOpenBin->Visible=true;
        }
        fMain->edSetOpenBin->Text="";
    }
}

// ===========================================================================
//  AI(W906-RSMODE) 20260927: golden TfMain::cbRunStartModeChange（main.cpp:23726-23825）逐行照翻 —— 主畫面「起動模式」下拉選單改值時的處理：
//  運轉中不做；AMKOR China／QUALCOMM 切 RT 要密碼；切 Initial Start 時機台裡有 IC 就改回原值並提示；然後 SetRunStartMode(rsmNull, 文字)
//  （本檔上面，golden main.cpp:363-1115）、Initial Start 重置 ATC 自檢、ATK 首盤警報、Auto Retest、SECS 事件、ASE 批次檔名、LotInfo 良率／AMR 重新整理。
//  和 golden 不同：自由函式（golden 是 TfMain 成員）⇒ cbRunStartMode 改成 fMain->cbRunStartMode；移植樹沒有的相依才閘（GATE(W906-RSMODE)）。
//  呼叫者：tools/wb_serve.cpp 的 WS 指令 main.runStartMode（先把 fMain->cbRunStartMode 的 Text／ItemIndex 設成網頁選的值，再呼叫本函式 = golden 的 OnChange）。
//  動作流程對照（INBOX 第 49 列）要照真機的操作順序：真機紀錄 17:29:53 改 Initial Start 才按 START。
// ===========================================================================
#include "AutoRetest.h"            // DoAutoRetest（golden :23797）
// AI(W906-RSMODE) 20260927: golden 寫 `cbRunStartMode->ItemIndex=LastSet.iRunStartMode;` 把下拉改回原值 —— VCL 的 TComboBox 設 ItemIndex 會
//  連帶把 Text 換成那一項（下一行 SetRunStartMode(rsmNull, cbRunStartMode->Text) 讀的就是它）；vclcompat::TComboBox 的 ItemIndex／Text 是兩個
//  獨立欄位，不補這一步就會變成「畫面說改回去了，其實照新選的模式跑」。清單有內容就照 VCL 取 Items[ItemIndex]（超出範圍＝空字串）；
//  清單是空的（移植樹 TfMain::SetStartModeData 目前是空殼，forms/fMain.cpp:456）就取 StartModeName[ItemIndex] —— golden 最常見的清單
//  （Continuous Start／Initial Start／Re-Test Continuous／Re-Test Initial Start）位置跟 enum 編號一樣，golden 自己也這樣假設（本檔 :149／:154）。
static void W906_RsmComboSyncText()
{
    TfLotInfoRunMode* cb = fMain->cbRunStartMode;
    const int i = cb->ItemIndex;
    if (cb->Items != 0 && cb->Items->GetCount() > 0)
        cb->Text = (i >= 0 && i < cb->Items->GetCount()) ? AnsiString(cb->Items->Strings[i]) : AnsiString("");
    else
        cb->Text = (i >= 0 && i < rsmRunModeTotal) ? StartModeName[i] : AnsiString("");
}
void W906_CbRunStartModeChange()
{
    if(SystemStart)
        return;
    bChangeModeING=true;                                                        //JerryYang 20250120 : modify
#if 0 // GATE(W906-RSMODE) golden :23731-23747 —— AMKOR China／QUALCOMM 切 RT 要輸密碼：fPassword 在移植樹從來沒建（forms/fPassword.cpp:29，NULL），fQwertyKey->ShowQwertyKey 是會卡住等輸入的畫面鍵盤；客戶專屬、排最後（這兩家以外的機台不走這段）
    if((CUSTOMER_CODE==CC_AMKOR_China ||
        CUSTOMER_CODE==CC_QUALCOMM) &&                                          //JerryYang 20170412 (Steven) add QUALCOMM
       IniConfig.bFTBin2RTBin==false)                                           //jou 20170214 (Steven) : RunStartMode need keyin Password
    {
        if(fMain->cbRunStartMode->Text=="Re-Test Continuous" ||
           fMain->cbRunStartMode->Text=="Re-Test Initial Start")
        {
            fPassword->edPassword->Text="";
            fQwertyKey->ShowQwertyKey(fPassword->edPassword, N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD);
            if(asRunStartModePassword!=fPassword->edPassword->Text)
            {
                fMain->cbRunStartMode->ItemIndex=LastSet.iRunStartMode; W906_RsmComboSyncText();   // AI(W906-RSMODE): VCL ItemIndex 連帶改 Text
                ShowMyMessage("FT -> RT 切換模式密碼錯誤","FT -> RT switch mode wrong password");
                return;
            }
        }
    }
#endif // GATE(W906-RSMODE)

//    if(IniConfig.bShowFTandRTButtonCanClick==true)                              //kevin 20180413 add onecycle no detect //Steven 20131224 : FT & RT Buttion 可以按
    {
        if(fMain->cbRunStartMode->Text=="Initial Start" ||
           fMain->cbRunStartMode->Text=="Re-Test Initial Start")                       //JerryYang 20171024 (wei) 避免Unloader有IC時切成Initial start會發生疊料
        {
            if(IniConfig.bFTBin2RTBin)                                          //Steven 20210202 : 當Bin一樣時, One Cycle之後要可以切換FT/RT
            {
                if(InArmSuck.HasIC() ||
                   OutArmSuck.HasIC() ||
                   ShuttleHasIC() ||
                   IndexHasIC() ||
                   AllArmZIsSafe()==false)
                {
                    fMain->cbRunStartMode->ItemIndex=LastSet.iRunStartMode; W906_RsmComboSyncText();   // AI(W906-RSMODE): VCL ItemIndex 連帶改 Text
                    ShowMyMessage("Auto區有tray盤, 請先執行tray feed", "There is a tray in Auto, please do tray feed");
                }
            }
            else
            {
                if(HasAnyICInMachine())
                {
                    fMain->cbRunStartMode->ItemIndex=LastSet.iRunStartMode; W906_RsmComboSyncText();   // AI(W906-RSMODE): VCL ItemIndex 連帶改 Text
                    ShowMyMessage("Auto區有tray盤, 請先執行tray feed", "There is a tray in Auto, please do tray feed");
                    return;
                }
            }
        }
    }

    SetRunStartMode(rsmNull, fMain->cbRunStartMode->Text);
    if(fMain->cbRunStartMode->Text=="Initial Start")                                   //Ifor 20160908Re-Test Initial Start 不做 ATC Self Test
        bInitialATCSelfTest=true;                                               //Ifor 20160829 切換至Initial Start 要重置 ATC Self Test 狀態

    if(IniConfig.bP22EnableFirstTrayNeedAlarm)                                  //ChungHung 20140521 add for ATK
    {
        if(LastSet.iRunStartMode==rsmInitialStart ||
           LastSet.iRunStartMode==rsmCInitialRetest)
            bFirstTrayNeedAlarm=true;
        else
            bFirstTrayNeedAlarm=false;
    }
    else
    {
        bFirstTrayNeedAlarm=false;
    }

    if(LastSet.iRunStartMode==rsmAutoRetest)                                    //Steven 20140409 : Auto Retest
    {
        DoAutoRetest(true);
    }

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.SwitchStartMode);

    if(CUSTOMER_CODE==CC_Greatek)                                               //Steven 20160531 start: 移動到上面
    {
                                                                                //Sam 20201007 : 超豐改到 Lot Start 在做
    }
    else
    {
        ASET_StartTimeNAME  =Now().FormatString("yyyymmdd_hhnn");               //kevin 20170417 (wei) 檔名
#if 0 // GATE(W906-RSMODE) golden :23810-23810 —— ASECommute（golden ASE 通勤排程結構）移植樹沒有 ⇒ ASET_ScheduleNAME 維持原值；ASE 客戶專屬
        ASET_ScheduleNAME   =ASECommute.Schedule;                               //kevin 20141020
#endif // GATE(W906-RSMODE)
        ASET_FileNAME       =ASET_ScheduleNAME+"_"+PC_NAME+"_"+ASET_StartTimeNAME+".csv";
        ASE_InTrayNum       =0;
        iLoadTrayCount      =0;
        if(fMain->hanaART->IsHanaArtAvailable()==false)
            LastSet.iASEContact =0;                                             //kevin 20190808 kevin 20141020 ASE 記錄此批一CONTRACT 次數
    }

    if(IniConfig.bASE_Report)                                                   //kevin 20140918 高雄日月光IC履歷記錄
    {
        bReceiveSchedule=true;
    }
    bChangeModeING=false;                                                       //JerryYang 20250120 : modify
    fLotInfo->RefreshYieldMonitor();
    fLotInfo->RefreshAMR();                                                     //Sam 20240304 : 新增 AMR 功能
}
