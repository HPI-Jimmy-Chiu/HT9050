// =============================================================================
//  StateRecord_L42_St02.cpp -- card L42 (ST02_V912_VS_V913 s3 L42): golden 913 makes State Record save more for
//  hang-up analysis (RogerYang 20260914 / 20260915 / 20260922, 偉測 HHT-17).
//
//  AI(W906-L42) 20261010 (St02).  cStateRecord.cpp is the laptop's file, so it only got same-line calls (its line count
//  is unchanged); the bodies live here, one free function per golden block, golden text verbatim.  They read machine
//  globals (+ both Index Z positions via Gali_ReadPos, golden 913 :6851 / :6856: offline Motor==NULL -> 0, a 1203 Index ->
//  ReadPos) and write only inside the record folder (NewPath) they are given.  Same library as cStateRecord.cpp
//  (ht9045_sm: root CMakeLists.txt, on the cStateRecord.cpp line), so every target that links cStateRecord.o links this.
//  No file-static helper of cStateRecord.cpp is used.  Not verified on a real machine.
// =============================================================================
#include "StateRecord_L42_St02.h"
#include "vclcompat/vcl_compat.h"  // AnsiString、TStringList
#include "cmydef.h"                // SystemStart
#include "csystem.h"               // HasICUnderMachine／sHasICUnderMachine／HasAnyICInMachine／sHasAnyICInMachine（csystem.cpp :13465-13571）
#include "forms/fAGV.h"            // fAGV->IsATK_AMR()
#include "cprod.h"                 // Prod
#include "Motor/mymotor.h"         // MOT[]、iHomeLed／iInposLed、Gali_ReadPos
#include "aHotPlateSubstrate.h"    // FLCarryKit／BLCarryKit／TestSocket／CatchTraySuck／InArmSiteMapData／iPlacePlate…／PickFromHPList／bHangTimePause —— 與 cStateRecord.cpp 同一個 TMyKitSuck（兩個 TMyKitSuck 的陷阱）
#include "acarry.h"                // b1ShuttleMoveToRight／b2ShuttleMoveToRight
#include "atester.h"               // HangTime

// golden 913 main.cpp:27706-27727 (L42) -- the tail of TfMain::DoStateRecord, after DumpMainFormSnapshot (called from the
//   DumpMainFormSnapshot line of cStateRecord.cpp's W906_DoStateRecordBody, before the background zip job).  golden 913
//   :27704-27705 (OutArmRoundLog_Dump, between the two) is not in the port: aoutarm9045.cpp's OutArmRoundLog is NB2-1 W-188's area.
void W906_L42_SaveMachineMaterial(AnsiString NewPath)
{
    //AI(ht9045-staterecord-analysis) 20260922 (RogerYang) : 落地機內料況帳本。S14F3 與部分 RCMD 以 HasICUnderMachine() 當閘門,
    //                                                        現場常見「目視沒料但機台判定有料」, 過去 State Record 無欄位可判讀 (偉測 HHT-17 20260922)
    try
    {
        TStringList *slMat=new TStringList;
        try
        {
            slMat->Add(AnsiString("SystemStart        = ")+(SystemStart?"true":"false"));
            slMat->Add(AnsiString("HasICUnderMachine  = ")+(HasICUnderMachine()?"true":"false"));
            slMat->Add(AnsiString("    Detail         : ")+sHasICUnderMachine());
            slMat->Add(AnsiString("HasAnyICInMachine  = ")+(HasAnyICInMachine()?"true":"false"));
            slMat->Add(AnsiString("    Detail         : ")+sHasAnyICInMachine());
            slMat->SaveToFile(NewPath+"\\MachineMaterial.txt");
        }
        catch(...)                                                              // golden __finally (C++ has none): delete, then rethrow -- as cStateRecord.cpp DumpMainFormSnapshot :1059
        {
            delete slMat;
            throw;
        }
        delete slMat;
    }
    catch(...)
    {
    }
}

// golden 913 main.cpp:6835-6863 (L42) -- TfMain::SaveTaskList, Pattern #31 rows after the bCheckShuttle2Flag row (called from
//   cStateRecord.cpp's former blank line after it, before b1ShuttleMoveToLeft).
void W906_L42_TaskListPattern31(TStringList *sList)
{
    AnsiString Str="";

    //AI(ht9045-staterecord-analysis) 20260922 (RogerYang) : Pattern #31 判別要件, 原本全部不在 StateRecord 裡
    //==>
    {
#if 0 // GATE(W906-L42-1) iShtChkFlagEscapeCT = golden 913 acarry.cpp:143 (913-only Pattern #31 escape counter); not in the port (git grep 0; 0618 / 912 have none)
        extern int  iShtChkFlagEscapeCT[2];                                     //acarry.cpp
#endif // GATE(W906-L42-1)
        extern bool IndexZCanMove[2];                                           //Motor\myGALILmotor.cpp

        Str.sprintf("b1ShuttleMoveToRight, %s", (b1ShuttleMoveToRight)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        Str.sprintf("b2ShuttleMoveToRight, %s", (b2ShuttleMoveToRight)?AnsiString("true"):AnsiString("false"));
        sList->Add(Str);

        Str.sprintf("IndexZCanMove[0]/[1], %d / %d", (IndexZCanMove[0])?1:0, (IndexZCanMove[1])?1:0);
        sList->Add(Str);

        Str.sprintf("MTestZ1 MovFlag/ReadPos/Safe/HomeLed/InPosLed, %d / %d / %d / %d / %d",
                    (MOT[MTestZ1].MovFlag)?1:0, (int)MOT[MTestZ1].Gali_ReadPos(), Prod.TestZ1_Safe,
                    (MOT[MTestZ1].Led[iHomeLed])?1:0, (MOT[MTestZ1].Led[iInposLed])?1:0);
        sList->Add(Str);

        Str.sprintf("MTestZ2 MovFlag/ReadPos/Safe/HomeLed/InPosLed, %d / %d / %d / %d / %d",
                    (MOT[MTestZ2].MovFlag)?1:0, (int)MOT[MTestZ2].Gali_ReadPos(), Prod.TestZ2_Safe,
                    (MOT[MTestZ2].Led[iHomeLed])?1:0, (MOT[MTestZ2].Led[iInposLed])?1:0);
        sList->Add(Str);

#if 0 // GATE(W906-L42-1) the row of that counter -- not invented
        Str.sprintf("ShtChkFlagEscapeCT SHT1/SHT2, %d / %d", iShtChkFlagEscapeCT[0], iShtChkFlagEscapeCT[1]);
        sList->Add(Str);
#endif // GATE(W906-L42-1)
    }
    //<==
}

// golden 913 main.cpp:7079-7116 (L42) -- TfMain::SaveTaskList, FLCarryKit / BLCarryKit / TestSocket after the BTestSuck dump
//   (called from the closing-brace line of cStateRecord.cpp's BTestSuck loop, before the "//<==" of that block).
void W906_L42_TaskListInShuttleSocket(TStringList *sList)
{
    AnsiString Str="";

    Str="FLCarryKit";                                                           //AI(ht9045-staterecord-analysis) 20260914 (RogerYang) : 補dump In Shuttle與TestSocket—AutoClean case2000 guard與CleanOut IC檢查看的是這三個,原本只dump Out Shuttle
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(FLCarryKit.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="BLCarryKit";
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(BLCarryKit.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }

    Str="TestSocket";
    sList->Add(Str);
    Str="";
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<8; j++)
        {
            Str+=IntToStr(TestSocket.Item[i][j])+",";
        }
        sList->Add(Str);
        Str="";
    }
}

// golden 913 main.cpp:7140-7166 (L42) -- TfMain::SaveTaskList, ATK customer rows after the CleanKitTime dump, before
//   MainProcMonitor (called from cStateRecord.cpp's former blank line between them).  golden's customer condition fAGV->IsATK_AMR() kept.
void W906_L42_TaskListATKFixFull(TStringList *sList)
{
    AnsiString Str="";

    if(fAGV->IsATK_AMR())                                                       //AI(ht9045-atk-amr-flow) 20260915 (RogerYang) : Fix-Full Swap診斷區--讓StateRecord單獨即可解析全貌
    {                                                                           //  20260908兩次停機時, 核心旗標iCatchTrayControlManual與TrayArm持盤狀態都只能靠推論
#if 0 // GATE(W906-L42-2) iATKFixFullTask / iATKFixFullLane / bATKFixFullHold = golden 913 cmydef.cpp:5738-5740 (ATK Fix-Full swap); not in the port (git grep 0)
        Str.sprintf("ATKFixFull, Task=%d, Lane=%d, Hold=%s, iCatchTrayControlManual=%d",
                    iATKFixFullTask, iATKFixFullLane,
                    bATKFixFullHold ? "true" : "false", iCatchTrayControlManual);
        sList->Add(Str);
#endif // GATE(W906-L42-2)

        Str.sprintf("ATKFixFull TrayArm, MOT[MTrayX].fHasTray=%s, CatchTraySuck.iWhichTray=%d, iWhichKit=%d",
                    MOT[MTrayX].fHasTray ? "true" : "false",
                    CatchTraySuck.iWhichTray, CatchTraySuck.iWhichKit);
        sList->Add(Str);

#if 0 // GATE(W906-L42-3) bAutoWaitAMR / bAutoWaitAMRSeenOn (golden 913 cmydef.cpp:5736-5737) and ATKFixFullMinFreeCells (golden 913 csystem.cpp:7716); not in the port (git grep 0)
        for(int i=0; i<3; i++)
        {
            Str.sprintf("ATKFixFull Auto%d, Need1D=%d Has1D=%d NeedCover=%d HasCover=%d WaitAMR=%d SeenOn=%d"
                        ", BinCT=%d FixBinCT=%d TrayCntCal=%d TrayCnt_ART=%d FixFreeCells=%d",
                        i+1,
                        bNeed1DCoverTray[i] ? 1 : 0, bHas1DCoverTray[i] ? 1 : 0,
                        bNeedCoverTray[i]   ? 1 : 0, bHasCoverTray[i]   ? 1 : 0,
                        bAutoWaitAMR[i]     ? 1 : 0, bAutoWaitAMRSeenOn[i] ? 1 : 0,
                        LastSet.BinCT[0][iTo3Unload[eAuto1+i]],
                        LastSet.BinCT[0][iTo3Unload[eFix1+i]],
                        iUnloaderTrayCountCal[i], LastSet.iUnloaderTrayCount_ART[i],
                        ATKFixFullMinFreeCells(eFix1+i));
            sList->Add(Str);
        }
#endif // GATE(W906-L42-3)
    }
}

// golden 913 main.cpp:7310-7427 (L42) -- TfMain::SaveDecisionVariables section 5b (ASM arm selection, dispatch cursor and
//   dispatched table, hot-plate ledger), between section 5 and section 6 (called from cStateRecord.cpp's former blank line; it runs
//   inside SaveDecisionVariables' try, as golden).  The card names :7346-7483; :7310-7345 (RogerYang 20260707) is the head of
//   the same 5b section -- its fNeedToCheckASM table is what the 20260914 fAlreadyCheckASM rows are read against (:7387).
void W906_L42_DecisionASM(TStringList *sList)
{
        AnsiString Str="";

        // ===== 5b. ASM 排臂狀態（開單arm 診斷用）=====
        sList->Add("# === ASM Arm Selection ===");

        //RogerYang 20260707 : ASM 目標 shuttle（0=Arm1, 1=Arm2）；單臂時應等於 iShuttle_Sel
        Str.sprintf("iAutoSiteMapHPToSht, %d", iAutoSiteMapHPToSht);
        sList->Add(Str);

        //RogerYang 20260707 : 關掉的那隻臂是否仍被排進 ASM；Arm2-only 時 Arm1 這列應全 0
        Str="Prod.fNeedToCheckASM (1=需做 ASM)";
        sList->Add(Str);
        for(int k=0; k<2; k++)
        {
            Str.sprintf("  Arm%d:", k+1);
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<8; j++)
                    Str+=IntToStr((int)Prod.fNeedToCheckASM[k][i][j])+",";
            }
            sList->Add(Str);
        }

        //RogerYang 20260707 : fNeedToCheckASM 的來源；由 DoInArm_SuckerMapForCloseArm 重建，用來看是否殘留改前設定
        Str="Prod.fInArmSuck4x8 (1=使用該站)";
        sList->Add(Str);
        for(int k=0; k<2; k++)
        {
            Str.sprintf("  Arm%d:", k+1);
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<8; j++)
                    Str+=IntToStr((int)Prod.fInArmSuck4x8[k][i][j])+",";
            }
            sList->Add(Str);
        }


        //AI(ht9045-asm-flow) 20260831 (RogerYang) : HHT-20 雙臂ASM死結定案所需 -- bAutoSiteMapWaitTestResult 一旦假置位, DoInArmAutoSiteMapping 首行就 return, InArm 全身不動
        Str.sprintf("bAutoSiteMapWaitTestResult, %s", bAutoSiteMapWaitTestResult?"true":"false");
        sList->Add(Str);

        Str.sprintf("bRunAutoSiteMapping, %s", bRunAutoSiteMapping?"true":"false");
        sList->Add(Str);

        Str.sprintf("bSiteMappingCHKOK, %s", bSiteMappingCHKOK?"true":"false");
        sList->Add(Str);

        Str.sprintf("iDoSiteMappingStep, %d", iDoSiteMappingStep);
        sList->Add(Str);

        //AI(ht9045-asm-flow) 20260914 (RogerYang) : ASM 派工游標與步序 -- 追「ASM 重啟後跳過第一站」必要的五個值
        //AI : 派工只在 iAutoSiteCurrStep != iDoSiteMappingStep 時發生, 且會把游標當下那站的 fNeedToCheckASM 清成 false
        Str.sprintf("iAutoSiteCurrStep, %d", iAutoSiteCurrStep);
        sList->Add(Str);

        Str.sprintf("iAutoSiteMapInArmRow/Col/Col_8, %d / %d / %d", iAutoSiteMapInArmRow, iAutoSiteMapInArmCol, iAutoSiteMapInArmCol_8);
        sList->Add(Str);

        Str.sprintf("iAutoSiteMapCount, %d", iAutoSiteMapCount);
        sList->Add(Str);

        Str.sprintf("iAutoSiteMapHPToKit, %d", iAutoSiteMapHPToKit);
        sList->Add(Str);

        Str.sprintf("iAutoSiteMapSiteNo, %d", iAutoSiteMapSiteNo);
        sList->Add(Str);

        Str.sprintf("iResetSiteMappingStep, %d", iResetSiteMappingStep);
        sList->Add(Str);

        Str.sprintf("Prod.bInitialAutoSiteMap, %s", Prod.bInitialAutoSiteMap?"true":"false");
        sList->Add(Str);

        Str.sprintf("InArmSiteMapData P/R/C/SuckR/SuckC, %d / %d / %d / %d / %d",
                    InArmSiteMapData.iP, InArmSiteMapData.iPlateR, InArmSiteMapData.iPlateC,
                    InArmSiteMapData.iSuckR, InArmSiteMapData.iSuckC);
        sList->Add(Str);

        Str="Prod.fAlreadyCheckASM (1=本輪已派工)";                                      //AI : 與 fNeedToCheckASM 對照看哪一站被空耗
        sList->Add(Str);
        for(int k=0; k<2; k++)
        {
            Str.sprintf("  Arm%d:", k+1);
            for(int i=0; i<2; i++)
            {
                for(int j=0; j<8; j++)
                    Str+=IntToStr((int)Prod.fAlreadyCheckASM[k][i][j])+",";
            }
            sList->Add(Str);
        }

        Str.sprintf("bAutoSiteMapHasPickHP, %s", bAutoSiteMapHasPickHP?"true":"false");         //AI : 與 bAutoSiteMapHotplateSave 兩旗標錯序 = 帳本錯亂5(孤兒源)
        sList->Add(Str);

        Str.sprintf("bAutoSiteMapHotplateSave, %s", bAutoSiteMapHotplateSave?"true":"false");
        sList->Add(Str);

        Str.sprintf("bAutoSiteMapHotplateReady, %s", bAutoSiteMapHotplateReady?"true":"false");
        sList->Add(Str);

        //AI(ht9045-inarm-flow) 20260831 (RogerYang) : HHT-80 放料撞料(WAR0150 + Swap error)定案所需 -- 搜尋失敗時這三個值是未驗證殘留
        Str.sprintf("iPlacePlate[0]/X/Y, %d / %d / %d", iPlacePlate[0], iPlacePlateX[0], iPlacePlateY[0]);
        sList->Add(Str);

        Str.sprintf("iPickPlate[0]/X/Y, %d / %d / %d", iPickPlate[0], iPickPlateX[0], iPickPlateY[0]);
        sList->Add(Str);

        //AI(ht9045-inarm-flow) 20260831 (RogerYang) : 熱盤帳實對帳 -- group數 vs 盤上真料數, 兩者對不起來就是孤兒/幽靈
        Str.sprintf("PickFromHPList group count, %d", (PickFromHPList!=NULL)?PickFromHPList->GetHPSuckGroupCount():-1);
        sList->Add(Str);

        Str.sprintf("HP RealIC(HowManyIC) P0/P1, %d / %d", MOT[MMPlate1].Tray.HowManyIC(), MOT[MMPlate2].Tray.HowManyIC());
        sList->Add(Str);

        Str.sprintf("HP HasIC(含HAS_NULL_IC佔位) P0/P1, %s / %s",
                    MOT[MMPlate1].Tray.HasIC()?"true":"false",
                    MOT[MMPlate2].Tray.HasIC()?"true":"false");
        sList->Add(Str);
        sList->Add("");
}

// golden 913 main.cpp:7474-7484 (L42) -- TfMain::SaveDecisionVariables section 9, the NN-mode hang-up watchdog rows after
//   bRunAutoClean (called from cStateRecord.cpp's former blank line after it).
void W906_L42_DecisionHangTime(TStringList *sList)
{
        AnsiString Str="";

        Str.sprintf("bHangTimePause, %s", (bHangTimePause)?AnsiString("true"):AnsiString("false"));  //AI(ht9045-staterecord-analysis) 20260823g (RogerYang) : Pattern #21 診斷--NN mode看門狗(CheckTwoArm32SiteSuckHangup)的進入條件之一; 為true代表看門狗被擋住不會報警。0709擷取包因為沒記這個變數而無法判定
        sList->Add(Str);

        Str.sprintf("TestISTimeOut, %s", (TestISTimeOut)?AnsiString("true"):AnsiString("false"));    //AI(ht9045-staterecord-analysis) 20260823g (RogerYang) : 同上, 看門狗第二個閘門
        sList->Add(Str);

        Str.sprintf("HangTime.Off(), %s", (HangTime.Off())?AnsiString("true"):AnsiString("false"));  //AI(ht9045-staterecord-analysis) 20260823g (RogerYang) : 看門狗計時是否已到期(true=已到期)
        sList->Add(Str);

        Str.sprintf("Prod.iHangupMaxTime, %.0f", Prod.iHangupMaxTime);                               //AI(ht9045-staterecord-analysis) 20260823g (RogerYang) : 逾時門檻秒數
        sList->Add(Str);
}
