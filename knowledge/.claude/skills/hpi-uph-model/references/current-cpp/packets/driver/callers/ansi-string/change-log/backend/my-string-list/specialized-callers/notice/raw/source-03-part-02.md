# 原文 03／分頁 2

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`；定位 `W906_NoteNoticeAckLikeGolden`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `3af2f5fb72ef93e6b4724de6e21f4d75980b433beea627cd49b753ec0297f166`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
    bLampTrain=false;
    bLampFix=false;                                                             //kevin 20130312
    SW[SwManualZ1].Off();
    }
    PassTime=W906_tNoteTimer.LatchCycleTimeSec();                               // golden :2545 PassTime=tNoteTimer.LatchCycleTimeSec();
#if 0 // GATE(W906-J5-ACK) C3: golden :2546 -- bScanBinLabel has no definition in this tree (git grep 20260930: 0 hits)
    bScanBinLabel=false;                                                        //JerryYang 20250429 : fix Auto In/Out
#endif // GATE(W906-J5-ACK) C3
    if(KeyCode==0)                                                              //Steven : 沒有KeyCode的話,不用Recovery
        Recovery="";
#if 0 // [W906] INBOX 64: golden :2551-2555 -- the SECS alarm-clear report on close; the blocking close does not send it either (forms/fNote_JamCount.cpp banner), both land together
    if(CUSTOMER_CODE!=CC_KYEC_LEE && IniConfig.bEnable_SECS_GEM==true)          //JerryYang 20170504 (Steven) SPIL騰清說解除alarm也需上報alarm report
    {
        bool bIsJam=CheckRecordJamType(edErrorCode->Text, true);
        HGem->ReportAlarm(edErrorCode->Text, bIsJam, iDuplicateError, ShowMessageEdit1->Text, true, Recovery);          //kevin 20170905 (Steven) 換成Edit 可以看後面字串
    }
#endif
    MyDBUEventRecover(W906_Notice.eventId, Recovery, PassTime);                 //Steven 20090819   [golden :2557; iEventID as the note was posted]
    if (bPress)
    {
    // golden :2558 fNote->Reset() -- its state line is :2673 edErrorCode->Text="" below; the rest is the VCL form
#if 0 // GATE(W906-J5-ACK) C4: golden :2559-2561 -- StopMovie is declared, not defined (GATE N-9, forms/fNote.h:428); palShtSensorSOP is VCL; bShowSpecialPan has no definition (git grep 20260930: 0 hits)
    StopMovie();
    palShtSensorSOP->Visible=false;                                             //Steven 20100319
    bShowSpecialPan=false;                                                      // 2011.04.11 , Q_Q
#endif // GATE(W906-J5-ACK) C4
    bEnterTestIF=true;

    lHandlerStopTime.LatchCycleTime(true);                                      //jou 2014-09-21 Show Handler Stop Time
#if 0 // [W906] not reached: golden :2569-2584 act only on fNote->ReturnCode==K_SKIP, and a KeyCode==0 note returns 0 (:4147)
    if(IniConfig.bA67TriggerOneCycleWhenAlarm)                                  //JerryYang 20241028 : 矽品彰化要求 特定ALARM要觸發ONE CYCLE
    {
        if(fNote->ReturnCode==K_SKIP)
        {
            if(edErrorCode->Text=="JAM0126" || edErrorCode->Text=="JAM0203" || edErrorCode->Text=="MES0101" ||
                edErrorCode->Text=="JAM0301" || edErrorCode->Text=="JAM0302" ||
                edErrorCode->Text=="JAM0303" || edErrorCode->Text=="JAM0304" ||
                edErrorCode->Text=="JAM0508" || edErrorCode->Text=="JAM0509" ||
                edErrorCode->Text=="JAM0305" || edErrorCode->Text=="JAM0306" ||
                edErrorCode->Text=="JAM0401" || edErrorCode->Text=="JAM0404" ||
                edErrorCode->Text=="JAM0201" || edErrorCode->Text=="JAM0202")
            {
                fMain->BtnOneCycleClick(fMain);
            }
        }
    }
#endif
#if 0 // GATE(W906-J5-ACK) C5: golden :2586-2605 -- TfProductionInfo has no ACM_WriteMsgAndCallExe (git grep 20260930: 0 hits); Greatek OEE N14_14 only
    if(CosFunction.bOEEFunction &&
       IniConfig.bN14_14_AlarmCtrlMachine)                                      //Sam 20210412 : 超豐新增軟體開啟時要丟訊息
    {
        sMessage.sprintf("Alarm,%s,%s", edErrorCode->Text, ShowMessageEdit1->Text);
        if(fNote->ReturnCode==K_SKIP)
            sMessage+=",Action:Skip";
        else if(fNote->ReturnCode==K_RETRY)
            sMessage+=",Action:Retry";
        else if(fNote->ReturnCode==K_TRAY_FEED)
            sMessage+=",Action:TrayFeed";
        else if(fNote->ReturnCode==K_TRAY_END)
            sMessage+=",Action:TrayEnd";
        else if(fNote->ReturnCode==K_CLEAN_OUT)
            sMessage+=",Action:CleanOut";
        else if(fNote->ReturnCode==K_ONECYCLE)
            sMessage+=",Action:OneCycle";
        else if(fNote->ReturnCode==K_RESET)
            sMessage+=",Action:Reset";
        fProductionInfo->ACM_WriteMsgAndCallExe(sMessage);
    }
#endif // GATE(W906-J5-ACK) C5
    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20160310 海思版本不顯示   [golden :2607]
    {
        if(bAutoRetestJam &&
           (ReturnCode==K_SKIP ||                                               // [W906] golden fNote->ReturnCode (0 here, :4147)
            bNeedKeyInSkipIC ||
            bAutoCount_Reset==true))                                            //wei 20160302 Jam Skip輸入顆數   [golden :2609-2612]
        {
            bAutoCount_Reset=false;                                             // golden :2614
#if 0 // [W906] not reached (ReturnCode 0) and GATE(W906-J5-ACK) C6: golden :2615-2647 -- ShowMyInputSkip has no definition in this tree (git grep 20260930: 0 hits)
            if(fNote->ReturnCode==K_SKIP ||
               fNote->ReturnCode==K_RESET)                                      //Ifor 20171228 : add 非 SKIP與RESET按鈕不累加 SKIP COUNT
            {
                if(bNeedKeyInSkipIC)
                {
                    Str=ShowMyInputSkip("請輸入Tray上遺漏IC顆數","Skip IC Count:");
                    if(Str=="NA")
                        return;
                    else
                        bNeedKeyInSkipIC=false;
                }

                Str.printf("%s Skip IC count : %d", sAlarmMes, iJamSkipICCount);
                RecordProcess(Str.c_str());

                iJamSkipIC=iJamSkipICCount;
                EventReport(SECS_EVENT.JamSkipICCount);                         //wei 20160503 Jam Skip IC Count

                if(CUSTOMER_CODE==CC_KYEC_LEE &&
                   (CosFunction.bUseMRTMode ||                                  //Ifor 20170413 add MRT Mode Use ART Skip IC Count顯示 改到下個版本
                    (USE_AUTO_RETEST==eartInstall && IniConfig.bA10_AutoReTest && bCleanSkipICCount==false) ||
                    (CosFunction.bUseARTSortCount==true && bCleanSkipICCount==false)))
                {
                    iJamSkipICCount=iJamSkipICCount+atoi(fShowBinSelect->StrARTSkipICCount->Cells[1][LastSet.iAutoRetestCount_ART+1].c_str());
                    fShowBinSelect->StrARTSkipICCount->Cells[1][LastSet.iAutoRetestCount_ART+1]=iJamSkipICCount;
                }

                for(int j=1; j<21; j++)
                {
                    iSkipICCount=iSkipICCount+atoi(fShowBinSelect->StrARTSkipICCount->Cells[1][j].c_str());
                }
                fShowBinSelect->StrARTSkipICCount->Cells[1][21]=iSkipICCount;
            }
#endif
            bNeedKeyInSkipIC=false;                                             //Ifor 20171228 : add 避免下次Alarm 出現 輸入IC的視窗   [golden :2648]
        }
    }
    iJamSkipICCount=0;
    sAlarmMes="";                                                               //wei 20160407 Alarm Message
    bAutoRetestJam=false;
    bOpenAllDoor=true;
#if 0 // [W906] display, not state: golden :2655-2662 -- the 2D-code grid of the VCL note
    for(int i=0; i<InArmSuck.iShtRow; i++)
    {
        for(int j=0; j<InArmSuck.iShtCol; j++)
        {
            fNote->t2DCode->SetCellNumber(i, j, "");
            fNote->t2DCode->SetCellColorIndex(i, j, 0);
        }
    }
#endif
    bInArmNeedToSafePos=false;                                                  //Steven 20171204 : 加上保護,避免Flag沒轉換造成Hang Up
#if 0 // GATE(W906-J5-ACK) C7: golden :2666-2671 -- fFTPClient has no port (BarcodeReader.h:44 GATE B-F1)
    if(CosFunction.bDownloadUpdateAutomatically &&
       IniConfig.bN32_CheckAtTrayFeedFinish)                                    //Sam 20220824 : FTP 自動下載安裝更新包
    {
        if(edErrorCode->Text=="MES1644")
            fFTPClient->DownloadUpdateAutomatically();
    }
#endif // GATE(W906-J5-ACK) C7
    if (fNote != 0) fNote->edErrorCode->Text="";                                // golden :2673 edErrorCode->Text="";
#if 0 // GATE(W906-J5-ACK) C8: golden :2675-2679 -- TfNote has no bCloseShowMsg (its only writer, Command.cpp:17513, is #if 0); and its ShowMyMessage would block in a wait loop, which this ack must never enter
    if(bCloseShowMsg)                                                           //Sam 20230426 : 通知系統 Handler 已經密碼鎖定
    {
        ShowMyMessage("!!!Please follow the SOP to deal with the found IC. and it is strictly forbiddento dispose of the found IC without permission!!!<Private handling will cause mixing problems> ", "!!!拾獲IC請遵循SOP處理，嚴禁私自處理拾獲IC!!!<私自處理會有混料問題產生>");
        bCloseShowMsg=false;
    }
#endif // GATE(W906-J5-ACK) C8
#if 0 // GATE(W906-J5-ACK) C9: golden :2681-2749 -- fNote->TempCode does not exist (forms/fNote.h), and the Bundle ID it reports comes from fNote->edBundleID, which the page never fills.  MES1712 / 1812 / 1912 are K_RETRY alarms in this tree (csystem.cpp:13889-13894), never a notice
    if(fNote->TempCode=="MES1712" || fNote->TempCode=="MES1812" || fNote->TempCode=="MES1912")
    {
        if(fNote->TempCode=="MES1712")                                          //Bundle 12= 10 + 1 black +1 2D cover tray
        {
            sFixBundleID[0]=fNote->edBundleID->Text;
            if(iFixTrayCountCal[0]>=fSCKART->iBundleOutCnt-2 || bUnloading==true)
            {
                sUnloadBundleID=fNote->edBundleID->Text;
//                sFixBundleID[0]=fNote->edBundleID->Text;
                sBundleEndInfo=GetBundleInfo(6);
                EventReport(SECS_EVENT.BundleEnd_Fix1);
                EventReport(SECS_EVENT.BundleEnd_IDREAD_Fix1);
                iFixTrayCountCal[0]=0;

                slDupUnloadBundlID->Clear();
                slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                slDupUnloadBundlID->Add(sUnloadBundleID);
                slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);

                sFixBundleID[0]="";

                RecordProcess("Fix1 Bundle End event report finished.");

                bNeedReportBundleID[eFix1]=false;                               //JerryYang 20250220 : fix AUTO IN OUT
            }
        }
        else if(fNote->TempCode=="MES1812")
        {
            sFixBundleID[1]=fNote->edBundleID->Text;
            if(iFixTrayCountCal[1]>=fSCKART->iBundleOutCnt-2 || bUnloading==true)
            {
                sUnloadBundleID=fNote->edBundleID->Text;
//                sFixBundleID[1]=fNote->edBundleID->Text;
                sBundleEndInfo=GetBundleInfo(7);
                EventReport(SECS_EVENT.BundleEnd_Fix2);
                EventReport(SECS_EVENT.BundleEnd_IDREAD_Fix2);

                slDupUnloadBundlID->Clear();
                slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                slDupUnloadBundlID->Add(sUnloadBundleID);
                slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);

                sFixBundleID[1]="";
                RecordProcess("Fix2 Bundle End event report finished.");
                bNeedReportBundleID[eFix2]=false;                               //JerryYang 20250220 : fix AUTO IN OUT
            }
        }
        else if(fNote->TempCode=="MES1912")
        {
            sFixBundleID[2]=fNote->edBundleID->Text;
            if(iFixTrayCountCal[2]>=fSCKART->iBundleOutCnt-2 || bUnloading==true)
            {
                sUnloadBundleID=fNote->edBundleID->Text;

<!-- preserved-content:end -->
```
