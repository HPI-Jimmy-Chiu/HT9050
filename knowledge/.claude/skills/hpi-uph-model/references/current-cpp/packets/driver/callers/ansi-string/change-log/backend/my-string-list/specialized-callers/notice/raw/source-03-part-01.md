# 原文 03／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`；定位 `W906_NoteNoticeAckLikeGolden`；種類 `complete_cpp_functions`。
來源 commit `c90d8d22bb2d34c532443386469de717aef2e671`；完整摘錄 SHA256 `3af2f5fb72ef93e6b4724de6e21f4d75980b433beea627cd49b753ec0297f166`。
分頁只切閱讀長度；包括原註解、gate、裁決註記，歷史 test 敘述不是本輪實測。

```cpp
<!-- preserved-content:start -->
bool W906_NoteNoticeAckLikeGolden(const char* requestId, int* pauseOut, bool* jamCountedOut, unsigned long* passTimeOut)
{
    int pause=3;
    bool jam=false;
    unsigned long passSec=0;
    const bool mine = W906_Notice.valid && W906_Notice.requestId == AnsiString(requestId ? requestId : "");
    if (mine && W906_Notice.goldenNote)
    {
    W906_Notice.valid=false;                                                    // [W906] exactly once
    const bool bPress = true;                                                   // [W906] banner: 0 / 1; AI(W906-I37A) 20261002: A -- was (SystemStart==false && SoftStart==false), "pause 2" when false
    pause = !bPress ? 2 : (W906_Notice.answerApplied ? 1 : 0);
    const int KeyCode=0;                                                        // [W906] this note's fNote->KeyCode (golden :901 KCode / :1110); the member may already hold a later note's
    const int ReturnCode=0;                                                     // [W906] golden :4147 ReturnCode=0 (TfNote has no ReturnCode; the caller got 0 when the notice was posted)
    //---- TfNote::BtnPauseClick (golden note.cpp:3826) ----
    if (bPress)
    {
    bStartMoveSpeed=false;                                                      //Steven 20231018 : Fixed for G14   [golden :3840]
#if 0 // GATE(W906-J5-ACK) B2: golden :3842-3863 -- fNote->TempCode, pnlLotInfo / tsHandler, edtLotCount, edEQCQty: TfNote has none of them (forms/fNote.h) -- the Lot Info / Bundle ID input checks of the old VCL note; no kcode==0 caller raises MES16111 / MES16112 / MES1712 in this tree with an input to check
    if(pnlLotInfo->Parent==tsHandler)
    {
        if(fNote->TempCode=="MES16112" && atoi(edtLotCount->Text.c_str())==0)
        {
            return;
        }
        else if(fNote->TempCode=="MES16111" && atoi(edEQCQty->Text.c_str())==0)
        {
            return;
        }
    }

    if(fNote->TempCode=="MES1712" || fNote->TempCode=="MES1812" || fNote->TempCode=="MES1912")
    {
        if((fNote->edBundleID->Text.Length()!=12 || (fNote->edBundleID->Text.Length()>3 && fNote->edBundleID->Text[3]!='T')))                                   //RogerYang 20250613 index 2->3
        {
            return;
        }
        else
        {
        }
    }
#endif // GATE(W906-J5-ACK) B2
    // golden :3865-4042 the Select[] loop: a KeyCode==0 note offers no key, so nothing is selected and it is skipped
    //---- if(KeyCode==0) golden :4044 ----
#if 0 // GATE(W906-J5-ACK) B3: golden :4046-4055 -- TfNote::DoPassword / bNeedPassWord / TempCode are not in this tree (forms/fNote.h; the WS dialog.auth answers "not wired", wb_serve.cpp): a note golden guards with a password closes without one -- as for the blocking close   [AI(W906-B28-NOTE) 20261002: the reason above is STALE since St01 D-026 (batch 27, 352e861c) -- dialog.auth IS wired now. This gate still stays closed: the KeyCode==0 password is asked BEFORE this ack path runs (tools/wb_serve.cpp dialog.notifyAck arm -> W906_NoteAuthNoticeGate; WebLogin.cpp EOF Press(sel<0) = golden :4046-4055 incl. the F15 JAM0508/0509 exception), so opening it would ask for the password twice.]
        if(IniConfig.bF15OutShuttleLoseICNeedPWD)                               //ChungHung 20120912 Amkor 需求Shuttle lose ic need password Only Skip
        {
            if((TempCode=="JAM0508" || TempCode=="JAM0509") && Select[0]!=true)
                bNeedPassWord=false;
        }

        if(DoPassword()==false)                                                 //Steven 20101124
        {
            return;
        }
#endif // GATE(W906-J5-ACK) B3
#if 0 // GATE(W906-J5-ACK) B4: golden :4057-4101 -- P53: TfNote::CheckBinCode is declared, not defined (GATE N-10, forms/fNote.h:433); myBinCodeEdit / labCheckBin / bCheckBinOK do not exist; the page's notice has no bin-code input.  Its codes are raised with K_RETRY in this tree (csystem.cpp:12725 / :12772, WebStart.cpp:3501 / :3527), never as a notice
        if(IniConfig.bP53_ForcedScanBinCodeOfUnloader &&                        //JerryYang 20240111 : add P53 function
           LastSet.iTester==ON_LINE)
        {
            if((edErrorCode->Text=="MES1120" ||
               edErrorCode->Text=="MES1220" ||
               edErrorCode->Text=="MES1320" ||
               edErrorCode->Text=="MES1720" ||
               edErrorCode->Text=="MES1820" ||
               edErrorCode->Text=="MES1920" ||
               edErrorCode->Text=="MES1124" ||
               edErrorCode->Text=="MES1224" ||
               edErrorCode->Text=="MES1324" ||
               edErrorCode->Text=="MES1724" ||
               edErrorCode->Text=="MES1824" ||
               edErrorCode->Text=="MES1924" ||
               edErrorCode->Text=="MES1712" ||
               edErrorCode->Text=="MES1812" ||
               edErrorCode->Text=="MES1912") && bCheckBinOK==false)             //JerryYang 20250429 : fix Auto In/Out
            {
                if(CheckBinCode()==true)
                {
                    labCheckBin->Caption="Check bin pass!";
                    labCheckBin->Font->Color=clBlack;
                    bCheckBinOK=true;
                }
                else
                {
                    for(int j=0; j<TEST_MAX_BIN; j++)
                    {
                        myBinCodeEdit[j]->Text="";
                        myBinCodeEdit[j]->Enabled="";                           //JerryYang 20241118 : fix
                        if(myBinCodeEdit[j]->Enabled==true && myBinCodeEdit[j]->Visible==true && myBinCodeEdit[j]->Text=="" && bSetFocus==false)
                        {
                            bSetFocus=true;
                            myBinCodeEdit[j]->SetFocus();
                        }
                    }

                    labCheckBin->Caption="Check bin fail! please keyin the bin again.";
                    labCheckBin->Font->Color=clRed;
                    bCheckBinOK=false;
                    return;
                }
            }
        }
#endif // GATE(W906-J5-ACK) B4
#if 0 // GATE(W906-J5-ACK) B5: golden :4103-4144 -- the Bundle ID / NO ReTest BIN checks read fNote->edBundleID, which the page never fills (the notice has no input), and show lblBundleIDErr / lblBundleUnloadIDuplicted / lblBunIDNotInList (TfNote has none); TempCode / iNoRTBinIdx do not exist.  MES1712 / MES1713 are raised with K_RETRY / K_RETRY|K_SKIP (csystem.cpp:13889-13906), never as a notice
        if(edErrorCode->Text=="MES1712" || edErrorCode->Text=="MES1812" || edErrorCode->Text=="MES1912")
        {
            if((fNote->edBundleID->Text.Length()!=12 || (fNote->edBundleID->Text.Length()>3 && fNote->edBundleID->Text[3]!='T')) ||                             //RogerYang 20250613 index 2->3
              (bUnloading==false &&
              ((iFixTrayCountCal[0]<fSCKART->iBundleOutCnt-2 &&edErrorCode->Text=="MES1712" && sFixBundleID[0]!="" && fNote->edBundleID->Text!=sFixBundleID[0]) ||
              (iFixTrayCountCal[1]<fSCKART->iBundleOutCnt-2 &&edErrorCode->Text=="MES1812" && sFixBundleID[1]!="" && fNote->edBundleID->Text!=sFixBundleID[1]) ||
              (iFixTrayCountCal[2]<fSCKART->iBundleOutCnt-2 &&edErrorCode->Text=="MES1912" && sFixBundleID[2]!="" && fNote->edBundleID->Text!=sFixBundleID[2]))))
            {
                lblBundleIDErr->Visible=true;
                return;
            }
            slDupUnloadBundlID->Clear();
            slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
            if(slDupUnloadBundlID->Find(fNote->edBundleID->Text, iTest))
            {
                lblBundleUnloadIDuplicted->Visible=true;
                return;
            }
        }
        else if(TrayForm.bVTestNoRTBin &&                                       //RogerYang 20250626 偉測不可複測bin功能
                (TempCode=="MES1713"   ||
                 TempCode=="MES1813"   ||
                 TempCode=="MES1913")  &&
                 bSetFocus==false)
        {
            bool bTmp=edBundleID->Text==""?false:edBundleID->Text==TrayForm.asNoRTBinFix[iNoRTBinIdx];
            fMesSystem->bNoRTBinFlag[iNoRTBinIdx]=bTmp;
            if(bTmp==false)                                                     //not Skip，且數值錯誤，不允許離開
            {
                if(bSetFocus==false)
                {
                    bSetFocus=true;
                    edBundleID->SetFocus();
                }

                if(Select[0]==false)
                {
                    lblBunIDNotInList->Visible=true;
                    return;
                }
            }
        }
#endif // GATE(W906-J5-ACK) B5
    if (!W906_Notice.answerApplied)                                             // [W906] pause 1: the motor body already did these two
    {
        SoftStop=true;
        SoftStart=false;
    }
    (void)ReturnCode;                                                           // [W906] golden :4147 ReturnCode=0 (see its declaration)
        if(IniConfig.bEnable_SECS_GEM==true)                                    //Steven 20140528 : Secs Gem
            EventReport(SECS_EVENT.DoPause);                                    // 2     按下 Pause
        SendCommand_ESD(ESD_SYSTEM_STOP);                                       //Steven 20140722
    // golden :4152 fNote->Close() -> TfNote::FormClose
    }
    //---- TfNote::FormClose (golden note.cpp:2503-2762) ----
    if (bPress)
    {
#if 0 // [W906] display, not state: golden :2507-2511 -- the SCK ART panel / btPrintSummary / pnlCorrectionCount of the VCL note (the box is the browser page; the blocking close leaves them out too, tools/wb_serve.cpp ~W906ModalWaitScope)
    fSCKART->palARTCount->Parent=fSCKART->palARTLeft;                           //Steven 20170111 (wei) : Add button for SCK ART
    fSCKART->palARTCount->Align=alTop;
    fSCKART->SettingPanelOnOff(false);
    btPrintSummary->Visible=false;
    pnlCorrectionCount->Visible=false;
#endif
    bShowNoteCleanSocket=false;                                                 //Sam 20230111 : Smart Auto Clean
#if 0 // GATE(W906-J5-ACK) C1: golden :2514-2517 -- TfProductionInfo has no bFTPError (forms/fProductionInfo.h; bFTPError is FormHS's, forms/fHS.h:665)
    if(CosFunction.bOEEFunction)                                                //Steven 20180417 (Jou) : OEE功能
    {
        fProductionInfo->bFTPError = false;
    }
#endif // GATE(W906-J5-ACK) C1
    // golden :2519 fShow=false -- its FormShow :2167 fShow=true never ran for a notice (no wait loop, no W906ModalWaitScope)
#if 0 // GATE(W906-J5-ACK) C2: golden :2520-2521 -- TfNote has no bSendJamCodeToFTP (KYECFTP/FTPClient_Transfer.cpp Gate #3) and no bNeedPassWord
    bSendJamCodeToFTP=false;
    bNeedPassWord=false;
#endif // GATE(W906-J5-ACK) C2
    bAlarmReset=false;                                                          //Steven 20140905 : 紀錄有被按下Alarm Reset
    // golden :2523-2524 *bPtr[i]=Select[i]: nothing is selected on a KeyCode==0 note (every lamp false; :2534-2543 clear them)
    bSECSGEM_NoteAlarm=false;                                                   //Ifor 20170616 (wei) add note From Close時清除SECSGEM Note Alarm 旗標
    bAlarmAfterPreAlarm=false;                                                  //Ifor 20170906 (wei) add 避免 PreAlarm -> Alarm -> SECS GEM Alarm 同時發生造成當機問題
    }
    const bool bLampTrayEndAtClose=false;                                       // [W906] golden :2523-2524 bLampTrayEnd=Select[3]: no key on a KeyCode==0 note
    if(W906_Notice.alarmType==1 && LastSet.iRealDummy==REALLY && !bLampTrayEndAtClose && W906_Notice.duplicateError!=1)   // golden :2528 (AlarmType / iDuplicateError as the note was posted)
        jam=W906_CheckRecordJamType(W906_Notice.code, false);                   // golden :2529 CheckRecordJamType(edErrorCode->Text)
    if (bPress)
    {
    W906_NoteFormCloseAlarmClear();                                             // golden :2531 Alarm->Clear();
    if(iGali_VsSpeed<=0)    iGali_VsSpeed=30000;
    if(iGali_SpSpeed<=0)    iGali_SpSpeed=10000;
    bLampSkip=false;
    bLampRetry=false;
    bLampOneCycle=false;
    bLampCleanOut=false;
    bLampTrayFeed=false;
    bLampTrayEnd=false;
    bLampReset=false;
    bLampHome=false;                                                            //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home

<!-- preserved-content:end -->
```
