# 原文 01

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/forms/fNote_ShowError.cpp`，function `void ShowMotorErrorMessage(AnsiString`。
來源 commit `26a6dcf0b8c1acf106a06e6cf4e700d20053d703`；完整摘錄 SHA256 `eed3fce9e12b32120b539d9dce1e8593910dca25e8f4c080a92987e3ef4f3749`。
此頁保留完整正文及原註解，歷史 golden／nm／test 敘述不是本輪驗證。

```cpp
<!-- preserved-content:start -->
void ShowMotorErrorMessage(AnsiString Code, int MotorAlarmNo, AnsiString errPart)
{
    W906_ShowMotorErrorMessage_LastCode         = Code;                         // [W906] the recorder first (AI(W906-W7-L2) 20260803, kept): tests assert "fired once with this code"
    W906_ShowMotorErrorMessage_LastMotorAlarmNo = MotorAlarmNo;                 // [W906]
    W906_ShowMotorErrorMessage_LastErrPart      = errPart;                      // [W906]
    W906_ShowMotorErrorMessage_Count++;                                         // [W906]
    extern std::string g_W906MotorErrorReason;                                  //AI(W906-ALARM-WHY) 20261003: taken and cleared FIRST (review: the early returns below left it for an unrelated later alarm)
    const std::string w906Reason = g_W906MotorErrorReason;  g_W906MotorErrorReason.clear();
    std::printf("  [ShowMotorErrorMessage] Code=%s MotorAlarmNo=%d errPart=%s\n", Code.c_str(), MotorAlarmNo, errPart.c_str());   // [W906] the stand-in's console line, kept

    SoftStop=false;
    SoftStart=false;
    StopAllMotor();
    MOT[MTestY1].Gali_Command("ST", errPart);
    IndexMotorBreakerOFF();
    fAllMotorHome=false;

    if(InitialOK==false)                                                        //Steven 20250310 : Add protection
    {
        MyDBIProcess("Exception", Code, AnsiString(MotorAlarmNo));
        return;
    }

    if(Code=="WAR")                                                             //Steven 20100830
    {
        ShowErrorMessage("WAR16101", 0, MMSystem, false, AnsiString(MotorAlarmNo));                                     //Motor error !!
        return;
    }

    int Pos=0;
    AnsiString Message, UnitName, AxleName, Str;
    int AlarmID, UnitNo, AxleNo;

    Code=Code+AnsiString(MotorAlarmNo);
    Code=Code.UpperCase();

    iEventID=MyDBIEvent(Code, 0, &AlarmID, &UnitNo, &AxleNo, &fNote->AlarmType, &Message, &UnitName, fMain->edWorkTemperBase->Text);
    if(AlarmID==41)
    {
        MyDBIProcess("Message", "Unknow Alarm Code: "+Code);
    }

    if(UnitNo>0 && UnitNo<=30)                                                  //Steven 20140222 Start: Alarm Code設定權限
        fNote->sJamArea=JamArea[UnitNo-1];
    else
        fNote->sJamArea="";

    fNote->sJamCode=Code;
    ProductionLog(Message, true);                                               //JerryYang 20160217 for 矽品蘇州 EventLog也要存成文字檔

    UnitNo=atoi(Code.SubString(6, 3).c_str());

    if(UnitNo>=MInArmX && UnitNo<=MInArmZH)             Pos=MInArmX;
    else if(UnitNo>=MOutArmX && UnitNo<=MOutArmZH)      Pos=MOutArmX;
    else if(UnitNo<=MTrayX)                             Pos=UnitNo;
    else                                                Pos=MMSystem;

#if 0 // GATE(W906-JAM-STOP) U1: golden :1101 -- ShowErrorUnit (golden note.cpp:4511, the VCL alarm-panel flush: FlushPanel=fNote->palXxx) has no definition in this tree (nm -C wb_serve.exe | grep ShowErrorUnit: none, 20260930) and TfNote has none of its panels; its one state line iPosition=Pos (:4514) has one reader, IsTestSitICFallDown in ScanPannelKey while fNote->fShow, and a kcode==0 note never sets fShow here.  The page flushes the unit itself from the note's position (web/page/ht9045_alarm_motionview.js), which is Pos, passed to the host below
    ShowErrorUnit(Pos);
#endif // GATE(W906-JAM-STOP) U1
    if (!w906Reason.empty()) Message = Message + AnsiString(("  ／ 原因：" + w906Reason).c_str());   //AI(W906-ALARM-WHY) 20261003: EastSun「你跳出錯誤需要出現為什麼錯誤」-- the port's reason (HOME failure / stepper leave / servo-on check) under golden's message
    fNote->ErrShowToForm(Code, MOT[UnitNo].Alias, Message, MotorAlarmNo);

    if(CosFunction.bOLPFunction)                                                //Steven 20141229 : OLP功能
    {
        fAutomation->DoCommandBuffer("ALARM_REQUEST", "", AnsiString(UnitName+":"+Message), fNote->AlarmType, Code);    //Steven 20110210   [W906] fAutomation is TfAutomationShim (atester_shims.h:334), whose DoCommandBuffer is empty (atester_shims.cpp:364) -- as for every OLP call site in this tree
    }
#if 0 // GATE(W906-JAM-STOP) F1: golden :1108-1109 -- fFTPClient->SaveJamCodeFile was never translated (fFTPClient has no port: BarcodeReader.h:44 GATE B-F1, AutoClean.cpp:849-860; nm -C wb_serve.exe | grep SaveJamCodeFile: none, 20260930); ASE_aDate (golden note.cpp:123) has no other reader in the port (golden :912 / :916 are ShowErrorMessage's untranslated ASE block), and vclcompat's AnsiString=TDateTime would format the day number, not BCB's date text.  Lost: the SPIL FTP jam-code trace file of a motor alarm
    ASE_aDate=Now();
    fNote->aJamCodeFilePath=fFTPClient->SaveJamCodeFile(IniConfig.SocketHandlerID, ASE_aDate, Code, AnsiString(UnitName+":"+Message));
#endif // GATE(W906-JAM-STOP) F1
    fNote->KeyCode=0;                                                           //ChungHung 20120927 強制只能秀pause  <----原本未指定任何顯示的組合建所以會依照上次所設定的

    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        Str.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    }
    else
    {
        Str.sprintf("%04d-%02d-%02d", SystemYear, SystemMonth, SystemDate);
        SL->Add(Str);
        Str.sprintf("%02d:%02d:%02d.%03d", SystemHour, SystemMin, SystemSec, SystemMSec);
        SL->Add(Str);
    }

    if(slEventLog!=NULL)                                                        // [W906] see the banner (never false in wb_serve)
    {
    fNote->iAlarmLine=slEventLog->GetLastLine();                                //Steven 20191016 : 紀錄目前Alarm在檔案裡面的行數
    SaveEventLog();
    }
#if 0 // GATE(W906-JAM-STOP) F2: golden :1128-1132 -- the same two missing dependencies as SAFETY-GATE(W906-YESNO-FTCT) (tools/wb_serve.cpp W906_YesNoShowLikeGolden): bAutoRestartAfterFTCTAlarm's definition is inside cmydef.cpp's `#if 0 // TODO(W6)` (cmydef.cpp:6178, nm -C wb_serve.exe: absent, 20260930), and fMain->RENESAS_Server is the stand-in TfMainRENESASServer (forms/fMain.h) with no FTCTManStartUnlock.  RENESAS FT-CT only (TestIF_File.bRENESAS_EnableFTCT): the FT-CT manual-start unlock is not sent
    if(TestIF_File.bRENESAS_EnableFTCT==true)                                   //RogerYang 20251002 : RogerYang 瑞薩FT-CT 解綁manualstart
    {
        bAutoRestartAfterFTCTAlarm=false;
        fMain->RENESAS_Server->FTCTManStartUnlock();
    }
#endif // GATE(W906-JAM-STOP) F2
    if(W906_ShowMotorErrorMessage_Hook)                                         // [W906] golden :1133 fNote->ShowModal(); -- the host posts the note and returns at once (banner)
    {
        W906_ShowMotorErrorMessage_Hook(Code.c_str(), fNote->KeyCode, Pos, MOT[UnitNo].Alias.c_str(), Message.c_str());
        SoftStop=true;                                                          // [W906] golden BtnPauseClick KeyCode==0 :4146 -- the note's only exit, applied now (banner, AI(W906-ALARMSTOP) rule)
        SoftStart=false;                                                        // [W906] golden :4148
    }

    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        SL->Add(fNote->sJamArea);                                               //UnitName
        SL->Add(fNote->sJamCode);                                               //AlarmCode
        SL->Add(Str);                                                           //OccurDateTime
        SL->Add(Recovery);                                                      //Recovery
        SL->Add((unsigned int)PassTime);                                        //StopedTime   [W906] cast: vclcompat has no AnsiString(unsigned long) (the conversion would be ambiguous); same decimal text
        SL->Add(iDuplicateError);                                               //Duplicate
        SL->Add(Message);                                                       //Message
        SL->Add(MotorAlarmNo);                                                  //ErrPart
    }
    else
    {
        SL->Add(fNote->sJamArea);
        SL->Add(fNote->sJamCode);
        SL->Add(Recovery);
        SL->Add((unsigned int)PassTime);                                        // [W906] cast, as above
        SL->Add(iDuplicateError);
        SL->Add(Message);
        SL->Add(MotorAlarmNo);
        SL->Add(GetLastOpenFN());
    }
    if(slEventLog!=NULL)                                                        // [W906] see the banner
    {
    SaveEventLog();
//    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
//        slEventLog->AddTextWithLineNo(SL->CommaText);
//    else
//        slEventLog->AddText(SL->CommaText);

    slEventLog->MyInsertToFile(SL->CommaText, fNote->iAlarmLine-1);             //Steven 20191016 : 紀錄目前Alarm在檔案裡面的行數
    }
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;

    bIndexDropVacuumError=false;                                                //kevin 20190418 避免 inarm 來回跑
    iHome=1;                                                                    //ChungHung 20120927 修改做contact height 時Index馬達錯誤會照成撞機
}

<!-- preserved-content:end -->
```
