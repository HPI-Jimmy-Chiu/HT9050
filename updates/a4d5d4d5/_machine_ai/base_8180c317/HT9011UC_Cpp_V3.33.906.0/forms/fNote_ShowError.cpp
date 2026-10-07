// =============================================================================
//  forms/fNote_ShowError.cpp  --  golden ShowErrorMessage's alarm-record half, TfNote::ErrShowToForm and
//                                  CheckRecordJamDrivingRecord   (+ golden ShowMotorErrorMessage at EOF, AI(W906-JAM-STOP) 20260930)
//
//  AI(W906-SHOWERR) 20260929: new file.  RULINGS_20260929 section 5 item 7 = A (user 0929 10:1x "JAM 警報斷電 ->
//    依據建議"): translate golden ShowErrorMessage's alarm record (note.cpp:839-868 -- MyDBIEvent + ProductionLog +
//    ErrShowToForm), which starts writing golden's alarm rows and gives fNote->edUnitName the unit name that
//    TfNote::FormShow's alarm servo-off (note.cpp:2035-2128) keys on.
//  Lines copied from golden 906 HT9011UC_Code_V3.33.906.0_20260618/note.cpp (cp950 -> UTF-8, 0 U+FFFD), in golden's
//  order; every [W906] line says so on the line.
//
//  CALLER: tools/wb_serve.cpp ForwardShowErrorMessage -- the W906_ShowErrorMessage_Hook host (canary_support.cpp:92
//    records the call first, then asks the hook) -- right after W906_AlarmStopLikeGolden (= golden :795-801).  The hook
//    carries only (Code, KCode, Pos); the host passes the bDuplicateErr / errPart that canary_support.cpp recorded
//    (W906_ShowErrorMessage_LastDuplicate / W906_ShowErrorMessage_LastErrPart).  No hook installed (every ctest except
//    NoteShowError / NoteServoOff) = nothing here runs = no behaviour change.
//
//  WHAT GOLDEN WRITES THROUGH THIS HALF (paths are the common.cpp seams -- W906_HT9045LOG_ROOT / W906_SAVEEVENTLOG_ROOT /
//  W906_RMS_ROOT -- which ctest points into machine_log_scratch for every test, AI(W906-ENV-ALL) / AI(W906-TESTGUARD)):
//    MyDBIEvent (cMyDB.cpp:774) -> SaveEventLogInfo (cMyDB.cpp:2078) -> SaveEventTracker (cMyDB.cpp:1960):
//        as9045LogPath "\ASE log\YYYY\MM\DD\<SocketHandlerID>@YYYY_MM_DD_EventTracker.csv" -- one row per alarm
//      SaveEventLogInfo itself: asSaveEventLogPath "\HANDLER LOG_<SocketHandlerID>_YYYY_MM_DD.csv" -- a row for WAR/MES
//        codes; for JAM (iType 0) golden sets bWrite=false (cMyDB.cpp:2133), so only the header line is created
//      and fNote->AlarmType through &fNote->AlarmType: 1 JAM / 2 WAR / 3 other, 0 = not in AlarmCodeList.txt
//        (fMain->AlarmCodeMap, filled at boot by MyDBUpdateDB, wb_serve.cpp:4166)
//    ProductionLog (LogObjects.cpp:357) -> asProductionLogPath "\<SocketHandlerID>_YYYYMMDD.logs", only when
//      IniConfig.bO06SaveLogTimePeriod
//    MyDBIProcess("Exception" / "Message", ...) (cMyDB.cpp:1003) -> the same HANDLER LOG csv (+ ProductionLog)
//
//  KEPT FROM GOLDEN AROUND :839-868 BECAUSE THE RECORD DEPENDS ON THEM (not a wider translation of ShowErrorMessage):
//    :538-542  InitialOK==false -> MyDBIProcess("Exception", Code, errPart) and no record (golden returns before :839)
//    :794      bOpenAllDoor=true -- the companion of :844's KYEC bOpenAllDoor=false (CheckRecordJamDrivingRecord :420);
//              without it that false would stick (the port's only other writers are the note-box panel scan,
//              wb_serve.cpp:7307-7313)
//    :802-813  iDuplicateError (MyDBIEvent's Duplicate column, ErrShowToForm's " (Again!!)")
//    :814-818  an alarm while fNote is up -> MyDBIProcess("Exception", "Alarm at same time: ...") and no record
//  [W906] DIFFERENCES, STATED:
//    * golden returns 0 from ShowErrorMessage at :541 / :817 (no box at all).  The port's host still stops the machine
//      and shows the box in those two cases (pre-existing, outside this item).  Only the record follows golden here:
//      they write the Exception line instead of an alarm row and return false, the host keeps its own AlarmType
//      fallback (wb_serve.cpp W906ModalWaitScope) and the FormShow servo-off does not run for that box.
//    * the two form-visibility reads (:808 fContact->fShow, :814 fNote->fShow) go through W906_FormShowing (the Q51
//      page table, W906FormShowing.h); with no hook installed it returns the member itself, i.e. golden's read.
//    * not in the host and not in this item (unchanged): :544-613 WAR151xx remap (16Site2X8), :615-627 FTP alarm
//      upload, :629-763 ATC alarm handling, :786-793 (ASE report, strBackUpCode, bShowBigDescription), :819-838
//      (TempCode, BundleID / ATC pages) and everything after :868 (ShowErrorUnit, sJamArea / sJamCode, OLP, ASE,
//      SaveJamCodeFile, OEE, SaveEventLog, SaveErrEventLog, ...).
//  GATES (each a missing dependency, reason on the #if line):
//    R1  :848-852  FormHS->RecordLog_HS
//    E1 / E2       inside ErrShowToForm (display only)
//
//  ctest: tests/test_note_showerror.cpp (NoteShowError).
// =============================================================================
#include "forms/fNote.h"             // TfNote / fNote (edErrorCode / Edit3 / edUnitName / AlarmType / fShow)
#include "forms/fMain.h"             // fMain->edWorkTemperBase (MyDBIEvent's Temperature column)
#include "cMyDB.h"                   // MyDBIEvent / MyDBIProcess -- NOT canary_support.h: the two cannot share one TU (cMyDB.h:82-89)
#include "cpublic.h"                 // ProductionLog (cpublic.h:45)
#include "cmydef.h"                  // InitialOK / sAlarmMes / bMaintanceMode / bOpenAllDoor / bAutoRetestJam / bNeedKeyInSkipIC / bCarRecordTimeStart / bUseFTPOneCycle / SwCarRecord / OFF_LINE / CUSTOMER_CODE / USE_AUTO_RETEST
#include "LastSet.h"                 // LastSet.iTester
#include "Config.h"                  // IniConfig.bC09_CarRecord / bSPILFunction
#include "CosFunction.h"             // CosFunction.bUseMRTMode / bUseARTSortCount
#include "MachineType.h"             // CC_KYEC_LEE / CC_Greatek / eartInstall
#include "myswitch.h"                // SW[] (SwCarRecord)
#include "atester_shims.h"           // fContact (TfContactShim, golden cContact.h:667)
#include "W906FormShowing.h"         // W906_FormShowing (the Q51 page-table read of a form's fShow)

//------------------------------------------------------------------------------
//  golden note.cpp file-scope data this half writes
//------------------------------------------------------------------------------
static bool bAutoCount_Reset=false;                                             // golden note.cpp:77 (its reader, FormClose :2612-2614, is not ported)   [AI(W906-J5-ACK) 20260930: read now -- the notice ack's FormClose, EOF W906_NoteNoticeAckLikeGolden (golden :2609-2614)]
int iEventID;                                                                   // golden note.cpp:82 (its reader, FormClose :2557 MyDBUEventRecover, is not ported)   [AI(W906-J5-ACK) 20260930: read now -- kept by W906_NoteNoticeCapture, passed to MyDBUEventRecover at the notice ack (EOF)]
int iDuplicateError=0;                                                          // golden note.cpp:84 (its other golden readers -- :1000 SaveErrEventLog, FormClose :2528 -- are not ported; forms/fNote_JamCount.cpp reads canary's LastDuplicate instead)   [AI(W906-J5-ACK) 20260930: FormClose :2528 reads it at the notice ack (EOF), as kept when the notice was posted]
bool W906_ShowErrorMessage_Recorded=false;                                      // [W906] true = the last W906_ShowErrorMessageRecordLikeGolden reached golden :846 (read by tools/wb_serve.cpp W906ModalWaitScope, golden FormShow)
//------------------------------------------------------------------------------
//  golden note.cpp:391-436 (file-scope function; its only golden caller is ShowErrorMessage :844)
//------------------------------------------------------------------------------
void CheckRecordJamDrivingRecord(AnsiString asJamCode)                          //wei 2013-12-09 JAM偵測檢查
{
    AnsiString Code;

    Code=asJamCode.UpperCase();
    if(Code.Pos("JAM")>0)
    {
        if(IniConfig.bC09_CarRecord)
        {
            SW[SwCarRecord].On();
            bCarRecordTimeStart=true;
        }
        else if(CUSTOMER_CODE==CC_KYEC_LEE &&
                (USE_AUTO_RETEST==eartInstall ||
                 CosFunction.bUseMRTMode==true ||                               //wei 20160407 Alarm Message
                 CosFunction.bUseARTSortCount==true))                           //Ifor 20170322 add Jam Skip (改到下個版本)
        {
            if(Code=="JAM0109" || Code=="JAM0112" || Code=="JAM0126" || Code=="JAM0128" ||
               Code=="JAM0201" || Code=="JAM0202" || Code=="JAM0203" || Code=="JAM0210" ||
               Code=="JAM0301" || Code=="JAM0302" || Code=="JAM0303" || Code=="JAM0304" ||
               Code=="JAM0312" || Code=="JAM0313" || Code=="JAM0314" || Code=="JAM0315" ||
               Code=="JAM0508" || Code=="JAM0509" || Code=="JAM0407")           //JerryYang 20160516 add JAM0312~0315 JAM0128
               {
                    if(Code=="JAM0303" || Code=="JAM0304" || Code=="JAM0508" || Code=="JAM0509")
                    {
                        bAutoCount_Reset=true;
                    }
                    bAutoRetestJam=true;                                        //wei 20160302 Jam Skip輸入顆數
                    #ifndef SOFT_SIMULTE
                    bOpenAllDoor=false;                                         //wei 20160407 Alarm 後需要開門確認
                    #endif
               }
        }
    }
    else if(Code=="MES0101")                                                    //Ifor 20171228 : add MES0101 Loader Pick Up Error 處置
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE &&
           (USE_AUTO_RETEST==eartInstall ||
            CosFunction.bUseMRTMode==true ||
            CosFunction.bUseARTSortCount==true))
        {
            bNeedKeyInSkipIC=true;
            bAutoRetestJam=true;
        }
    }
}
//------------------------------------------------------------------------------
//  TfNote::ErrShowToForm -- golden note.cpp:4307-4509 (declared golden note.h:406; here forms/fNote.h:371).
//  MachineType.h:22 leaves DEBUG_NEW_ALARM_DESCRIPTION undefined, so golden's live arm is the #else one (:4399-4508);
//  the #ifdef arm is kept verbatim and is not compiled.  [W906] no __fastcall (same as every forms/fNote.h member).
//  Live here: the fields the port has, in golden's order -- edErrorCode / Edit3 / edUnitName (TfNote members,
//  forms/fNote.h:365-371), the global sAlarmMes (cmydef.cpp:4263) and the Greatek bUseFTPOneCycle reset (:4500).
//  GATE E1 (:4400-4435) and E2 (:4444-4495, and the reDescription lines of :4497-4507): the description text / movie of
//    the old VCL window -- PlayMovie is declared, not defined (GATE (N-9), forms/fNote.h:427), and TfNote has no
//    reDescription / reBigDescription / ShowMessageEdit1 member.  The alarm window is the browser page (its description
//    comes from web/JSON/js/Alarm-description.js), not a TfNote control.  Nothing but display is lost.
//------------------------------------------------------------------------------
void TfNote::ErrShowToForm(AnsiString Code, AnsiString UName, AnsiString Mes, int iMotorErr)   // golden note.cpp:4307 ([W906] no __fastcall)
{
#ifdef DEBUG_NEW_ALARM_DESCRIPTION
    bool bAddToMemo;
    TStringList *strList = new TStringList;
    AnsiString Language="", LanguageEng=Code+"_English", MoviePath;
    strList->LoadFromFile("D:\\HT9045\\Error\\AlarmDescription.ini");

//    _di_IXMLDocument

    MoviePath="D:\\Movie\\"+Code+".avi";                                        //Steven 20100914

    PlayMovie(MoviePath);
    reDescription->Clear();
    edErrorCode->Text="";
    Edit3->Text="";
    edUnitName->Text="";

    edErrorCode->Text=Code;
    Edit3->Text=Code.SubString(4, 2);
    edUnitName->Text=UName;
    ShowMessageEdit1->Text=Mes;                                                 //kevin 20170905 (Steven) add

    if(IniConfig.iUserLanguage==eulKorea)                                       //Steven 20120203 : 分開語言包
    {
        Language=Code+"_Korea";
    }
    else if(IniConfig.iUserLanguage==eulJapan)
    {
        Language=Code+"_Japan";
    }
    else if(IniConfig.iUserLanguage==eulSingapore)                              //Steven 20120607 : 新加坡版
    {
        Language=Code+"_Singapore";
    }
    else if(IniConfig.iUserLanguage==eulEnglish)                                //Steven 20120202 : 純英文版
    {
        Language="";
    }
    else
    {
        Language=Code+"_Chinese";
    }

    bAddToMemo=false;
    for(int i=0; i<strList->Count; i++)
    {
        if(Language!="")
        {
            if(strList->Strings[i].Pos(Language)!=0)
            {
                bAddToMemo=true;
                continue;
            }
        }

        if(strList->Strings[i].Pos(LanguageEng)!=0)
        {
            bAddToMemo=true;
            continue;
        }

        if(bAddToMemo)
        {
            if(strList->Strings[i].Pos("[")!=1)
            {
                reDescription->Lines->Add(strList->Strings[i]);
            }
            else
            {
                bAddToMemo=false;
            }
        }
    }

    if(IniConfig.iUserLanguage==eulKorea)                                       //Steven 20120203 : 分開語言包
    {
        reDescription->Font->Name="Batang";
        reDescription->Font->Charset=HANGEUL_CHARSET;
    }
    else if(IniConfig.iUserLanguage==eulJapan)
    {
        reDescription->Font->Name="Tahoma";
        reDescription->Font->Charset=SHIFTJIS_CHARSET;
    }
    else
    {
        reDescription->Font->Name="Arial";
        reDescription->Font->Charset=DEFAULT_CHARSET;
    }
    strList->Clear();                                                           //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete strList;
#else
#if 0 // GATE(W906-SHOWERR) E1: golden :4400-4435 -- description path, PlayMovie (GATE N-9, not defined) and reDescription / reBigDescription (no TfNote member); display only, the page shows the description
    AnsiString Language, TextPath="", MoviePath="";

    if(CUSTOMER_CODE==CC_KYEC_LEE)                                              //Ifor 20160907 KYEC 要求顯示中英文 Alarm
    {
         Language="Chinese";
    }
    else if(IniConfig.iUserLanguage==eulKorea)                                  //Steven 20120203 : 分開語言包
    {
        Language="Korea";
    }
    else if(IniConfig.iUserLanguage==eulEnglish)                                //Steven 20120202 : 純英文版
    {
        Language="English";
    }
    else if(IniConfig.iUserLanguage==eulSingapore)                              //Steven 20120607 : 新加坡版
    {
        Language="Singapore";
    }
    else
    {
        if(LastSet.iLanguageCountry == 0)                                       //kevin 20170923 (wei) add 中英切換
            Language="English";
        else
            Language="Chinese";
    }

    if(iMotorErr==-1)
        TextPath="D:\\HT9045\\Error\\"+Language+"\\"+Code+".dat";
    else
        TextPath="D:\\HT9045\\Error\\"+Language+"\\MOT"+iMotorErr+".dat";

    MoviePath="D:\\Movie\\"+Code+".avi";                                        //Steven 20100914

    PlayMovie(MoviePath);
    reDescription->Clear();
    fNote->reBigDescription->Clear();                                           //Ifor 20200331 : add
#endif // GATE(W906-SHOWERR) E1
    edErrorCode->Text="";
    Edit3->Text="";
    edUnitName->Text="";

    edErrorCode->Text=Code;
    Edit3->Text=Code.SubString(4,2);
    edUnitName->Text=UName;
    sAlarmMes=Mes;                                                              //wei 20160407 Alarm Message
#if 0 // GATE(W906-SHOWERR) E2: golden :4444-4495 -- ShowMessageEdit1 / reDescription / reBigDescription (no TfNote member); display only
    ShowMessageEdit1->Text=Mes;                                                 //kevin 20170905 (Steven) add

    if(FileExists(TextPath))
    {
        reDescription->Lines->LoadFromFile(TextPath);                           //Steven 20100127 : 新的Alarm訊息專用
        fNote->reBigDescription->Lines->LoadFromFile(TextPath);                 //Ifor 20200331 : add

        if(IniConfig.iUserLanguage==eulKorea)                                   //Steven 20120203 : 分開語言包
        {
            reDescription->Font->Name="Batang";
            reDescription->Font->Charset=DEFAULT_CHARSET;
            fNote->reBigDescription->Font->Name="Batang";                       //Ifor 20200331 : add
            fNote->reBigDescription->Font->Charset=DEFAULT_CHARSET;             //Ifor 20200331 : add
//            reDescription->Font->Charset=HANGEUL_CHARSET;
        }
        else if(IniConfig.iUserLanguage==eulJapan)
        {
            reDescription->Font->Name="Tahoma";
            reDescription->Font->Charset=SHIFTJIS_CHARSET;
            fNote->reBigDescription->Font->Name="Tahoma";                       //Ifor 20200331 : add
            fNote->reBigDescription->Font->Charset=SHIFTJIS_CHARSET;            //Ifor 20200331 : add
        }
        else if(IniConfig.iUserLanguage==eulEnglish)                            //Steven 20120202 : 純英文版
        {
            reDescription->Font->Name="Arial";
            reDescription->Font->Charset=DEFAULT_CHARSET;
            fNote->reBigDescription->Font->Name="Arial";                        //Ifor 20200331 : add
            fNote->reBigDescription->Font->Charset=DEFAULT_CHARSET;             //Ifor 20200331 : add
        }
        else
        {
            reDescription->Font->Charset=DEFAULT_CHARSET;
            fNote->reBigDescription->Font->Charset=DEFAULT_CHARSET;             //Ifor 20200331 : add
        }
    }
    else
    {
        Language="English";
        if(iMotorErr==-1)
            TextPath="D:\\HT9045\\Error\\"+Language+"\\"+Code+".dat";
        else
            TextPath="D:\\HT9045\\Error\\"+Language+"\\MOT"+iMotorErr+".dat";

        if(FileExists(TextPath))
        {
            reDescription->Lines->LoadFromFile(TextPath);                       //Steven 20100127 : 新的Alarm訊息專用
            fNote->reBigDescription->Lines->LoadFromFile(TextPath);             //Ifor 20200331 : add
        }

        reDescription->Font->Name="Arial";
        fNote->reBigDescription->Font->Name="Arial";                            //Ifor 20200331 : add
    }
#endif // GATE(W906-SHOWERR) E2
    (void)iMotorErr;                                                            // [W906] its only readers are the TextPath lines inside E1 / E2

    if(CUSTOMER_CODE==CC_Greatek && bUseFTPOneCycle && Code=="MES1640")         //Sam 20200305 : 增加 FTP 檔案檢查，檢查到 VIE_STOP.txt 就執行 OneCycle
    {
#if 0 // GATE(W906-SHOWERR) E2: golden :4499 reDescription (no TfNote member)
        reDescription->Clear();
#endif // GATE(W906-SHOWERR) E2
        bUseFTPOneCycle=false;
#if 0 // GATE(W906-SHOWERR) E2: golden :4501-4502 reDescription (no TfNote member)
        reDescription->Lines->Add("Please confirm Tester VIE information");
        reDescription->Font->Color=clRed;
#endif // GATE(W906-SHOWERR) E2
    }
    else
    {
#if 0 // GATE(W906-SHOWERR) E2: golden :4506 reDescription (no TfNote member)
        reDescription->Font->Color=clBlack;
#endif // GATE(W906-SHOWERR) E2
    }
#endif
}
//------------------------------------------------------------------------------
//  golden ShowErrorMessage (note.cpp:532-1048) -- the alarm-record half.  See the banner for what is kept and why.
//  Returns true when golden would have reached :846 (MyDBIEvent ran); also left in W906_ShowErrorMessage_Recorded.
//------------------------------------------------------------------------------
bool W906_ShowErrorMessageRecordLikeGolden(AnsiString Code, bool bDuplicateErr, AnsiString errPart)   // [W906] Code / bDuplicateErr / errPart = golden ShowErrorMessage's parameters (note.cpp:532)
{
    W906_ShowErrorMessage_Recorded=false;                                       // [W906]
    if(InitialOK==false)                                                        //Steven 20250310 : Add protection
    {
        MyDBIProcess("Exception", Code, errPart);
        return 0;
    }
    bOpenAllDoor=true;                                                          //wei 20160407 Alarm 後需要開門確認
    if(bDuplicateErr==true)
        iDuplicateError=1;
    #ifndef SOFT_SIMULTE
    else if(LastSet.iTester==OFF_LINE)                                          //Steven 20110121 : 不要把Offline的Jam也給列入報表中
        iDuplicateError=2;
    #endif
    else if(W906_FormShowing("fContact", fContact->fShow)==true)                //Steven 20110209 : Auto Height時也不算   [W906] golden :808 fContact->fShow==true, read through the page table (Q51)
        iDuplicateError=3;
    else if(bMaintanceMode)                                                     //Steven 20251007 : maintance mode for Hana
        iDuplicateError=4;
    else
        iDuplicateError=0;
    if(W906_FormShowing("fNote", fNote->fShow))                                 // [W906] golden :814 if(fNote->fShow), read through the page table (Q51; fNote is a program row, WebPageTable.cpp:133)
    {
        MyDBIProcess("Exception", "Alarm at same time: "+Code, errPart);        //Steven 20250310 : Add protection
        return 0;
    }
    AnsiString Message, UnitName;
    int AlarmID, UnitNo, AxleNo;
    Code=Code.UpperCase();
    if(IniConfig.bC09_CarRecord || CUSTOMER_CODE==CC_KYEC_LEE)
    {
        CheckRecordJamDrivingRecord(Code);                                      //wei 2013-12-09
    }
    iEventID=MyDBIEvent(Code, 0, &AlarmID, &UnitNo, &AxleNo, &fNote->AlarmType, &Message, &UnitName, fMain->edWorkTemperBase->Text, iDuplicateError, errPart);
    ProductionLog(Message, true);                                               //JerryYang 20160217 for 矽品蘇州 Process Record也要存成文字檔
#if 0 // GATE(W906-SHOWERR) R1: golden :848-852 -- FormHS->RecordLog_HS is declared, not defined (forms/fHS.h:693 "GATE (Cat B)"); the same pair is gated at forms/fLotInfo.cpp:6912 (S117-S25-HSLOG).  SPIL customers only (IniConfig.bSPILFunction)
    if(IniConfig.bSPILFunction==true)                                           //Ifor 20160408 矽品客戶要求，發生Alarm上傳一次Log
    {
        bSysLotStart=false;
        FormHS->RecordLog_HS(true);
    }
#endif // GATE(W906-SHOWERR) R1

    if(AlarmID==41)
    {
        MyDBIProcess("Message", "Unknow Alarm Code: "+Code);
    }

    if(errPart==" ")
    {
        if(iDuplicateError==1)  fNote->ErrShowToForm(Code, UnitName, Message+" (Again!!)", -1);
        else                    fNote->ErrShowToForm(Code, UnitName, Message, -1);
    }
    else
    {
        if(iDuplicateError==1)  fNote->ErrShowToForm(Code, UnitName, Message+" : "+errPart+" (Again!!)", -1);
        else                    fNote->ErrShowToForm(Code, UnitName, Message+" : "+errPart, -1);
    }
    W906_ShowErrorMessage_Recorded=true;                                        // [W906]
    return true;                                                                // [W906] golden goes on to ShowErrorUnit (:870) ... ShowModal (:991)
}

// =============================================================================
//  AI(W906-SHOWERR) 20260929: TfNote::W906_FormShowServoOff -- golden TfNote::FormShow note.cpp:2035-2128, the alarm
//  servo-off block (FormShow has no return before it; the block's only local, `AnsiString str;`, is golden :1364).
//  RULINGS_20260929 section 5 item 7 = A: the In Arm half first; the Out Arm half together with csystem.cpp DoServoOn
//  GATE H2-G1; the Out Shuttle half (E44) together with GATE H2-G2 / H2-G3 -- DoServoOn is each half's servo-on.
//  HOST: tools/wb_serve.cpp W906ModalWaitScope(0) (golden TfNote::FormShow), just before fShow=true (golden :2167 also
//    comes after this block), and only when W906_ShowErrorMessage_Recorded -- golden reaches FormShow only through
//    ShowModal (:991), never after the :538 / :814 returns.  The port opens no box (no FormShow) for kcode==0 alarms --
//    they do not block, AI(W906-Q30-KZERO) 20260923 -- so those never switch a servo off here (golden: they do).
//  KEY: edUnitName->Text = MyDBIEvent's UnitName via ErrShowToForm = UnitNameMap[the two digits after the prefix]
//    (cMyDB.cpp:885-886, AlarmUnit[] cMyDB.cpp:145): 01 "Input Arm", 02 "Output Arm", 03 "Index Unit", 04 "Input
//    Shuttle", 05 "Output Shuttle".  A code missing from AlarmCodeList.txt has UnitName "" and switches nothing off.
//  RESET PATHS (why no axis stays parked for good):
//    In Arm (bMyServoOffInArm): DoInArm_9045 returns while it is true (ainarm9045.cpp:904-907).  Cleared by DoServoOn's
//      In Arm arm (csystem.cpp:22032-22149 = golden :20742-20859), which MainProc calls every tick of the SystemStart
//      branch (csystem.cpp:31730) and of the contact branches (:31269 ...): START re-energises X/Y, jogs back to
//      iMyServoOffInArmPosX/Y and clears the flag; a Z picker still down -> StopAllMotor + SetInArmHome (home);
//      a CW/CCW limit on (CosFunction.bLimitSensorOnNeedManualMove) -> MES0171 / MES0172 until moved off by hand.
//      Also HOME (uhome.cpp:1073) and DoInArmPineRelease (AutoClean.cpp:3273 / :3282).
//    Out Arm (bMyServoOffOutArm, S2): aoutarm9045.cpp:759-762, aoutarm.cpp:915 / :993 / :1040 / :1101,
//      asortarm.cpp:920-922 and aRotateKIT_Out.cpp:467 / :1319 return while it is true.  Cleared by DoServoOn's Out Arm
//      arm (csystem.cpp:21902-22006 = golden :20619-20722, GATE H2-G1) on START -- the same shape, MES0271 / MES0272,
//      OutArmZSafe -> StopAllMotor + SetOutArmHome -- and by HOME (uhome.cpp:1074).
//    Out Shuttle 1/2 (bMyServoOffOutShuttle1/2, S3): nothing parks on the flag; MOT[MInShuttle1/2] is simply off.
//      Cleared by DoServoOn's Out Shuttle arms (csystem.cpp:22180-22250 / :22282-22351 = golden :20870-20933 /
//      :20952-21014, GATE H2-G2 / H2-G3) on START once both arms are back (bResult[0] && bResult[1]); Z pickers or
//      Index Z not safe -> StopAllMotor + fAllMotorHome=false (HOME needed).  Also HOME (uhome.cpp:1080-1081).
//  [W906] a member of TfNote so golden's text stays verbatim (edUnitName / bMyServoOff... are TfNote members).
//  golden quirk kept: the "Out Shuttle" half reads / switches MOT[MInShuttle1/2] (:2096-2098 / :2118-2120), as DoServoOn's
//    H2-G2 / H2-G3 do (golden :20874 / :20956).
//  ctest: tests/test_note_servooff.cpp (NoteServoOff).
// =============================================================================
#include "Motor/mymotor.h"           // MOT[] / InArmZSafe / OutArmZSafe (mymotor.h:439-440)
#include "cprod.h"                   // Prod.TestZ1_Safe / Prod.TestZ2_Safe
#include "common.h"                  // MySleep (common.h:366)
void TfNote::W906_FormShowServoOff()                                            // [W906] part of golden TfNote::FormShow (note.cpp:1223), split out; declared forms/fNote.h:378
{
    AnsiString str;                                                             //ChungHung 02120720 add
    if(IniConfig.bAlarmNeedServoOff)                                            //Steven 20110802
    {
        if(edUnitName->Text=="Input Arm"      ||                                //In Shuttle異常, In Arm讓開
           edUnitName->Text=="Input Shuttle"  ||
           edUnitName->Text=="Output Shuttle" ||                                //Steven 20220624 : Out shuttle & Index Jam, In arm servo off
           edUnitName->Text=="Index Unit")
        {
            if(bMyServoOffInArm==false &&                                       //ChungHung 20120912 Hangup 解除
               (MOT[MInArmX].IsCanMove() &&
                MOT[MInArmY].IsCanMove()))
            {
                if(InArmZSafe(DETECT_ALL_FLAG)==-1)                             //如果Z軸在上才可以推
                {
                    iMyServoOffInArmPosX=MOT[MInArmX].ReadEncoderPos();
                    iMyServoOffInArmPosY=MOT[MInArmY].ReadEncoderPos();         //要在ServoOff之前
                    MySleep(200);
                    MOT[MInArmX].ServoOnOff(false);
                    MOT[MInArmY].ServoOnOff(false);
                    bMyServoOffInArm=true;
                    str.sprintf("In Arm Servo Off, X=%d, Y=%d", iMyServoOffInArmPosX, iMyServoOffInArmPosY);
                    RecordProcess(str);
                }
            }
        }

#if 1 // GATE(W906-SHOWERR) S2 RETIRED: golden :2060-2079 the Out Arm half -- lands together with csystem.cpp DoServoOn GATE H2-G1 (its servo-on / jog-back); alone, bMyServoOffOutArm would park the Out Arm (aoutarm9045.cpp:759-762) until HOME (RULINGS_20260929 section 5 item 7)  //AI(W906-SHOWERR) 20260929: retired with csystem.cpp DoServoOn GATE H2-G1 (same commit) -- START re-energises and jogs the Out Arm back, HOME clears the flag
        if(edUnitName->Text=="Output Arm" ||                                    //Out Shuttle異常,Out Arm讓開
           edUnitName->Text=="Output Shuttle")                                  //ChungHung 20120723 Output Shuttle Alarm InArm Servo off
        {
            if(bMyServoOffOutArm==false &&                                      //ChungHung 20120912 Hangup 解除
               (MOT[MOutArmX].IsCanMove() &&
                MOT[MOutArmY].IsCanMove()))
            {
                if(OutArmZSafe(DETECT_ALL_FLAG)==-1)                            //如果Z軸在上才可以推
                {
                    iMyServoOffOutArmPosX=MOT[MOutArmX].ReadEncoderPos();
                    iMyServoOffOutArmPosY=MOT[MOutArmY].ReadEncoderPos();       //要在ServoOff之前
                    MySleep(200);
                    MOT[MOutArmX].ServoOnOff(false);
                    MOT[MOutArmY].ServoOnOff(false);
                    bMyServoOffOutArm=true;
                    str.sprintf("Out Arm Servo Off, X=%d, Y=%d", iMyServoOffOutArmPosX, iMyServoOffOutArmPosY);
                    RecordProcess(str);
                }
            }
        }
#endif // GATE(W906-SHOWERR) S2
#if 1 // GATE(W906-SHOWERR) S3 RETIRED: golden :2080-2127 the Out Shuttle half (IniConfig.bE44EnableLoseDeviceOutShuttleServoOff) -- lands together with csystem.cpp DoServoOn GATE H2-G2 / H2-G3 (its servo-on / jog-back); alone, a de-energised shuttle would stay off until HOME  //AI(W906-SHOWERR) 20260929: retired with csystem.cpp DoServoOn GATE H2-G2 / H2-G3 (same commit) -- START re-energises and moves the shuttle back, HOME clears the flag

        if(IniConfig.bE44EnableLoseDeviceOutShuttleServoOff)                    //ChungHung 20140522 add OutShuttle lose devices can servo off start
        {
            if(edErrorCode->Text=="JAM0508")
            {
                if(bMyServoOffOutShuttle1==false &&
                   (MOT[MOutArmX].IsCanMove() &&
                    MOT[MOutArmY].IsCanMove() &&
                    MOT[MInArmX].IsCanMove() &&
                    MOT[MInArmY].IsCanMove() &&
                    MOT[MTestZ1].IsCanMove()))
                {
                    if(OutArmZSafe(DETECT_ALL_FLAG)==-1 &&                      //如果Z軸在上才可以推
                       InArmZSafe(DETECT_ALL_FLAG)==-1 &&
                       MOT[MTestZ1].Gali_ReadEncoderInRandge(Prod.TestZ1_Safe))
                    {
                        iMyServoOffOutShuttle1Pos=MOT[MInShuttle1].ReadEncoderPos();
                        MySleep(200);
                        MOT[MInShuttle1].ServoOnOff(false);
                        bMyServoOffOutShuttle1=true;
                        str.sprintf("Out Shuttle 1 Servo Off, X=%d", iMyServoOffOutShuttle1Pos);
                        RecordProcess(str);
                    }
                }
            }
            else if(edErrorCode->Text=="JAM0509")
            {
                if(bMyServoOffOutShuttle2==false &&
                   (MOT[MOutArmX].IsCanMove() &&
                    MOT[MOutArmY].IsCanMove() &&
                    MOT[MInArmX].IsCanMove() &&
                    MOT[MInArmY].IsCanMove() &&
                    MOT[MTestZ2].IsCanMove()))
                {
                    if(OutArmZSafe(DETECT_ALL_FLAG)==-1 &&                      //如果Z軸在上才可以推
                       InArmZSafe(DETECT_ALL_FLAG)==-1 &&
                       MOT[MTestZ2].Gali_ReadEncoderInRandge(Prod.TestZ2_Safe))
                    {
                        iMyServoOffOutShuttle2Pos=MOT[MInShuttle2].ReadEncoderPos();
                        MySleep(200);
                        MOT[MInShuttle2].ServoOnOff(false);
                        bMyServoOffOutShuttle2=true;
                        str.sprintf("Out Shuttle 2 Servo Off, X=%d", iMyServoOffOutShuttle2Pos);
                        RecordProcess(str);
                    }
                }
            }
        }
#endif // GATE(W906-SHOWERR) S3
    }
}

// =============================================================================
//  AI(W906-JAM-STOP) 20260930: ShowMotorErrorMessage -- golden note.cpp:1052-1169 (declared golden note.h:467; here
//  canary_support.h:268).  INBOX 118 "motor JAM does not stop the machine, no alarm shown": every motor JAM path (the
//  encoder range check in Gali_MotMove* Motor/myGALILmotor.cpp:2036 ..., ScanIndexMotorCanMove, ckernel's alarm sweep
//  ckernel.cpp:3915 / :3942, uhome.cpp, csystem.cpp:16013) ends here.  Until today the only definition was the
//  record-only stand-in canary_support.cpp:486-525 (retired in the same commit), which left out golden :1056-1058 and
//  the whole display half: a blocked Z1 printed one line and the next tick re-sent the move.  User rule 20260922: this
//  project ships -- translate what golden does (stop, brake, alarm); gate only a dependency that does not exist.
//  Lines copied from golden 906 HT9011UC_Code_V3.33.906.0_20260618/note.cpp (cp950 -> UTF-8), in golden's order.
//
//  WHERE IT LIVES AND WHY (link analysis, nm 20260930 on build_ship / build_sim): the callers are in ht9045_sm
//    (ckernel.cpp / csystem.cpp / uhome.cpp) and ht9045_motor (Motor/myGALILmotor.cpp); this TU is in ht9045_sm next to
//    golden ShowErrorMessage's record half, because the display half needs cMyDB.h (MyDBIEvent / the 3-arg
//    MyDBIProcess), and cMyDB.h cannot share a TU with canary_support.h (cMyDB.h:82-89).  ONE definition: the stand-in
//    is #if 0'd; test_ga1_cprod compiles canary_support.cpp directly and calls none of the callers.  The recorder
//    (W906_ShowMotorErrorMessage_*, canary_support.cpp:427-437, read by tests/test_gali_route_engine.cpp) is kept and
//    still written first.
//  THE STOP HALF (golden :1054-1059): StopAllMotor() = the no-arg forwarder (aHotPlateSubstrate.cpp:1236) to the golden
//    body StopAllMotor(true) (Motor/myGALILmotor.cpp, golden Motor/myGALILmotor.cpp:4712; AI(W906-STOPALL)), then golden's
//    Galil "ST" on MOT[MTestY1] (TMyMotor::Gali_Command, Motor/myGALILmotor.cpp:1385; no card: golden's own ST clearing,
//    and the INBOX 113 Index Z1 route when installed), then IndexMotorBreakerOFF() (csystem.cpp:19144).  All three run
//    before the InitialOK check, as in golden.
//  THE DISPLAY HALF (golden :1073-1169): all live except the gates below.  golden :1133 fNote->ShowModal() is the host
//    hook W906_ShowMotorErrorMessage_Hook (0 = no host: tests, unattended) -- tools/wb_serve.cpp installs it and posts
//    the note through the SAME path as a kcode==0 ShowErrorMessage notice (ForwardShowErrorMessage's AI(W906-Q30-KZERO)
//    branch: dialog mailbox + alarm ring raise/clear, return at once).  golden :1110 sets KeyCode=0 ("PAUSE only"), and
//    the port's kcode==0 notes do not wait (user ruling 20260923, AI(W906-Q30-KZERO)): the tick thread never waits on
//    the browser here.
//    [W906] DEVIATION, stated: golden's ShowModal returns only through the note's one button, PAUSE (TfNote::BtnPauseClick,   [AI(W906-J5-ACK) 20260930: [AI(W906-I37A) 20261002: INBOX 122 / NB2 R122 correction -- TfNote::Start's KeyCode==0 arm :3683-3803 is NOT a second exit: FormShow hides BtnStart :1542-1545 and the panel START key returns while it is hidden :3024-3029]; the PAUSE exit is now wired at the notice ack -- EOF banner]
//    KeyCode==0 arm, golden note.cpp:4044-4153: SoftStop=true; ReturnCode=0; SoftStart=false; ...).  The port's note does
//    not wait, so that answer's state is applied right after the hook -- the rule of AI(W906-ALARMSTOP) 20260924 (user
//    ruling 1: a notice stops the machine and the operator presses START again, "golden's result, without the key press").
//    Without it SystemStart stays true (golden :1052-1169 never clears it -- it relies on the note), MainProc's
//    fAllMotorHome==false arm (csystem.cpp:31185) sets iHome=1 and its iHome==1 arm (:31556) runs DoHomeProcess -- the
//    machine re-homes by itself on the next tick, before anybody has seen the alarm.
//    Not applied with the answer (the blocking W906-DLG-PAUSE answer sends them since AI(W906-I37A) 20261002, INBOX 123): golden :4149-4151
//    EventReport(DoPause) and SendCommand_ESD(ESD_SYSTEM_STOP), and :4052 DoPassword.
//    Recovery / PassTime (golden :1140-1141 / :1150-1151) are golden FormClose's (not ported): "" and 0.   [AI(W906-J5-ACK) 20260930: FormClose now runs at the notice ack (EOF), after this row was written -- StopedTime stays 0 here (known gap, EOF banner)]
//  GATES (each a missing dependency; the measurement is on the #if line):
//    U1  :1101  ShowErrorUnit(Pos)
//    F1  :1108-1109  ASE_aDate / fFTPClient->SaveJamCodeFile
//    F2  :1128-1132  RENESAS FT-CT unlock
//  [W906] slEventLog guard: golden builds slEventLog in the TfMain ctor, before InitialOK can be true; wb_serve builds it
//    at boot (tools/wb_serve.cpp:4166 W906_CreateLogObjects) before PumpInit sets InitialOK (:4212), so the guard never
//    fires there -- it keeps a ctest that sets InitialOK without the log objects from dereferencing NULL.
//  ctest: tests/test_note_motorerror.cpp (NoteMotorError).
// =============================================================================
#include "Public/MyStringList.h"     // TMyStringList::GetLastLine / MyInsertToFile (slEventLog, cmydef.h:117)
void StopAllMotor();                                                            // aHotPlateSubstrate.h:980 -- forwards to StopAllMotor(true) (Motor/myGALILmotor.cpp; golden's default argument), AI(W906-STOPALL)
void IndexMotorBreakerOFF();                                                    // csystem.h:143 (body csystem.cpp:19144)
int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr, AnsiString errPart);   // canary_support.h:66 (that header cannot share this TU with cMyDB.h)
extern AnsiString W906_ShowMotorErrorMessage_LastCode;                          // canary_support.cpp:427 (the recorder, canary_support.h:275-279)
extern int        W906_ShowMotorErrorMessage_LastMotorAlarmNo;
extern AnsiString W906_ShowMotorErrorMessage_LastErrPart;
extern int        W906_ShowMotorErrorMessage_Count;
//------------------------------------------------------------------------------
//  golden note.cpp file-scope data this function reads
//------------------------------------------------------------------------------
static DWORD PassTime;                                                          // golden note.cpp:79 (its writer, FormClose, is not ported: stays 0)   [AI(W906-J5-ACK) 20260930: written now -- the notice ack's FormClose :2545, EOF]
AnsiString Recovery;                                                            // golden note.cpp:118 (its writer, FormClose, is not ported: stays "")   [AI(W906-J5-ACK) 20260930: written now -- notice capture (FormShow :2130) and ack (FormClose :2548-2549), EOF]
AnsiString JamArea[]={"Input Arm",                                              // golden note.cpp:125-155
                      "Output Arm",
                      "Index Unit",
                      "Input Shuttle",
                      "Output Shuttle",
                      "Empty Tray Arm",
                      "Tester I/F",
                      "Scanner",
                      "Tray Loader",
                      "Empty Tray",
                      "Tray Unloader 1",
                      "Tray Unloader 2",
                      "Tray Unloader 3",
                      "Color Tray",
                      "Temp. Controller",
                      "System",
                      "Fix Tray 1",
                      "Fix Tray 2",
                      "Fix Tray 3",
                      "ESD",
                      "Process",
                      "Motion",
                      "Cassette",
                      "Motor",
                      "Tray Unloader 4",
                      "Tray Unloader 5",
                      "Tray Unloader 6",
                      "Fix Tray 4",
                      "Fix Tray 5",
                      "Fix Tray 6",
                     };
// [W906] golden :1133 fNote->ShowModal() -- the host that shows the note (tools/wb_serve.cpp ForwardShowMotorErrorMessage).
//   KeyCode = fNote->KeyCode (golden :1110 sets 0 = PAUSE only), Pos = golden :1096-1099's unit (golden :1101 ShowErrorUnit),
//   UnitName / Message = what golden :1102 ErrShowToForm puts in edUnitName / ShowMessageEdit1.  0 = no host.  Tick thread only.
void (*W906_ShowMotorErrorMessage_Hook)(const char* Code, int KeyCode, int Pos, const char* UnitName, const char* Message) = 0;
//------------------------------------------------------------------------------
//Steven 20190412 : 修改Motor Alarm Message --> WAR24xxxo (xxx為馬達號碼, 0為Alarm Type)
//------------------------------------------------------------------------------
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

// =============================================================================
//  AI(W906-J5-ACK) 20260930: the operator's answer to a KeyCode==0 note -- INBOX 119 (Jerry J-5, FROM_JERRY 0929 18:24).
//  Golden (906 note.cpp): a KeyCode==0 note waits in ShowModal (ShowErrorMessage :991, ShowMotorErrorMessage :1133)
//  until the operator closes it, and it has ONE exit, PAUSE (AI(W906-I37A) 20261002: INBOX 122 / NB2 R122 -- not two, see START):
//    PAUSE  TfNote::BtnPauseClick (:3826), its KeyCode==0 arm (:4044-4153): SoftStop=true, ReturnCode=0, SoftStart=false,
//           EventReport(DoPause), SendCommand_ESD(ESD_SYSTEM_STOP), Close()
//    START  TfNote::Start (:3527), its KeyCode==0 arm (:3683-3803) is unreachable: FormShow hides BtnStart (:1542-1545),
//           the panel START key returns while it is hidden (:3024-3029); the port matches (dialog-page.js, wb_serve :7369)
//  and PAUSE ends in TfNote::FormClose (:2503-2762).  The port's notice does not wait (user ruling 20260923,
//  AI(W906-Q30-KZERO)).  This is the PAUSE exit, reached from the page's single 確認 button through WS dialog.notifyAck
//  (tools/wb_serve.cpp W906_NoticeAckCommand -> w906dlg::NotifyAckHandle, tools/wb_dialog_mailbox.h).  A web / remote
//  START (start.run, SECS RCMD) can restart the machine while the box is up; the ack then pauses it as golden does
//  (RULINGS_20261002 #5 = NIGHT_REPORT s0 #37 A; it used to be "pause 2", records only).  Nothing here starts motion.
//
//  W906_NoteNoticeCapture -- when a notice is posted (wb_serve ForwardShowErrorMessage, its kcode==0 branch): keeps what
//    FormClose reads from fNote.  Golden reads it at close, and nothing can change it while the note is modal (a new
//    alarm returns at :814-818); the port's notice does not hold the tick, so it is kept here.  Plus golden FormShow's
//    two lines this close depends on: :1526 tNoteTimer start (PassTime), :2130 Recovery="".
//  W906_NoteNoticeAckRefusal -- golden BtnPauseClick's early returns before it closes (the box stays up).
//  W906_NoteNoticeAckLikeGolden -- after the mailbox is idle: BtnPauseClick's KeyCode==0 arm, then FormClose, once.
//
//  WHAT RUNS (the pause code goes back in the ack, w906dlg::NotifyAckOkJson):
//    3 no-golden-note  the record half did not run (InitialOK false :538-542 / an alarm while fNote is up :814-818):
//                      golden shows no note at all, so nothing closes -- only the mailbox is retired.
//    2 skipped-machine-running  RETIRED by AI(W906-I37A) 20261002 (RULINGS_20261002 #5 = A).  It was: SystemStart or SoftStart
//                      true (restarted since the notice) -> records only, no PAUSE.  golden's KeyCode==0 arm has no such
//                      condition (:4146-4152): the machine is paused now and Alarm->Clear() drops what it raised meanwhile, as
//                      golden.  bPress below is always true; the code value 2 stays reserved in w906dlg::NotifyAckOkJson.
//
//    1 already-applied the motor note: ShowMotorErrorMessage's body applied :4146 / :4148 right after posting (this file,
//                      the AI(W906-ALARMSTOP) rule), so SoftStop / SoftStart are not set a second time; the rest as 0.
//    0 applied         the machine is still in the stop the notice put it in: everything below.
//  KNOWN GAPS (not dependency gates -- stated so the ack is not read as "golden's close is complete"):
//    * ShowMotorErrorMessage's EventLogTxt row (this file, after the hook) is written when the note is POSTED with
//      StopedTime (PassTime) 0; golden writes it after ShowModal returns (:1135-1163) with the seconds the note was up.
//      Moving it here would lose the row whenever a notice is never acknowledged (replaced by a newer alarm, wb_serve
//      restarted) -- a user decision, INBOX 119.
//    * ShowErrorMessage's post-ShowModal tail (golden :993-1047: SaveErrEventLog (N-7), the Unloader-full message, ASE,
//      the K_SKIP pause, HSForm) is not translated for any alarm, blocking or notice (this file's banner).
//    * HGem->ReportAlarm at close (:2551-2555) is INBOX 64, as for the blocking close (forms/fNote_JamCount.cpp banner).
//    * a notice replaced by a newer request before anyone acknowledged it never gets this close (golden cannot replace
//      an open note; the port's single-slot mailbox can).
//  ctest: tests/test_notice_ack.cpp (NoticeAck).
// =============================================================================
#include "Interface/InterfaceSYS.h"  // SendCommand_ESD / ESD_SYSTEM_STOP (golden :4151)
#include "SECSGEM/SecsEventType.h"   // SECS_EVENT.DoPause (golden :4150)
#include "SECSGEM/SecsEventReport.h" // EventReport (golden :4150)
#include "myTimer.h"                 // TQPF_Timer (golden note.h:438 tNoteTimer)
bool W906_CheckRecordJamType(AnsiString asJamCode, bool bCheckOnly);          // forms/fNote_JamCount.cpp (golden note.cpp:344 CheckRecordJamType)
void W906_NoteFormCloseAlarmClear();                                           // HAlarm.cpp:345 (golden FormClose :2531 Alarm->Clear())
extern TQPF_Timer hAutoCleanHangUp;                                            // csystem.h:291 (golden csystem.cpp:157)
extern TQPF_Timer tGalilTwoYMoveDelay;                                         // Motor/myGALILmotor.cpp:5432 (golden mymessbox.cpp:384 extern)
void W906_NoteNoticeCapture(const char* requestId, bool motorNote);
const char* W906_NoteNoticeAckRefusal(const char* requestId);
bool W906_NoteNoticeAckLikeGolden(const char* requestId, int* pause, bool* jamCounted, unsigned long* passTimeSec);
namespace {
struct W906NoticeNote                                                           // [W906] fNote as golden FormClose reads it, kept when the notice is posted
{
    bool       valid = false;
    AnsiString requestId;                                                       // the mailbox requestId it belongs to
    AnsiString code;                                                            // fNote->edErrorCode->Text (:2528-2529)
    int        alarmType = 0;                                                   // fNote->AlarmType (:2528)
    int        duplicateError = 0;                                              // iDuplicateError (:2528)
    int        eventId = 0;                                                     // iEventID (:2557)
    bool       goldenNote = false;                                              // golden reaches ShowModal for it (the record half ran)
    bool       answerApplied = false;                                           // :4146 / :4148 already applied (the motor note)
};
W906NoticeNote W906_Notice;
TQPF_Timer     W906_tNoteTimer;                                                 // [W906] golden note.h:438 TfNote::tNoteTimer (TfNote has none here)
}
//------------------------------------------------------------------------------
void W906_NoteNoticeCapture(const char* requestId, bool motorNote)             // [W906] see the banner
{
    W906_Notice.valid          = true;
    W906_Notice.requestId      = AnsiString(requestId ? requestId : "");
    W906_Notice.code           = (fNote != 0) ? fNote->edErrorCode->Text : AnsiString("");
    W906_Notice.alarmType      = (fNote != 0) ? fNote->AlarmType : 0;
    W906_Notice.duplicateError = iDuplicateError;
    W906_Notice.eventId        = iEventID;
    W906_Notice.goldenNote     = motorNote ? true : W906_ShowErrorMessage_Recorded;   // the motor body returns before the host when InitialOK is false (:1061-1065)
    W906_Notice.answerApplied  = motorNote;
    if (W906_Notice.goldenNote)
    {
    W906_tNoteTimer.LatchCycleTimeSec(true);                                    // golden TfNote::FormShow :1526
    Recovery="";                                                                //Steven 20120209 : 每次進來都要初始化   [golden FormShow :2130]
    }
}
//------------------------------------------------------------------------------
//  golden TfNote::BtnPauseClick (note.cpp:3826), the returns before anything closes.  0 = no refusal.
//------------------------------------------------------------------------------
const char* W906_NoteNoticeAckRefusal(const char* requestId)
{
    if (!W906_Notice.valid || W906_Notice.requestId != AnsiString(requestId ? requestId : "")) return 0;   // [W906] not this notice: nothing to refuse (the close below does nothing either)
    if (!W906_Notice.goldenNote) return 0;                                      // [W906] pause 3: golden has no note, no BtnPauseClick
    if (false) return 0;                                                        // [W906] was pause 2 (SystemStart||SoftStart -> no refusal checks); AI(W906-I37A) 20261002: A -- golden refuses whatever the machine state
#if 0 // GATE(W906-J5-ACK) B1: golden :3828-3831 -- fNote->bNeedTCPAlarm (TfNote has none; Command.cpp's readers are comments / #if 0) and bErrPan_err / Pwd (0 definitions in this tree, git grep 20260930): the HandlerResultServer panel lock and the SpecialPanel password are not ported, so neither refuses here
    if(CosFunction.bEnableHandlerResultServer && bNeedTCPAlarm)                 //Sam 20230620 : 面板已經鎖定需要 IT 下命令解鎖
        return;
    if(bErrPan_err==true && Pwd!="")
        return;                                                                 //Richard 2011/2/22 SpecialPanel
#endif // GATE(W906-J5-ACK) B1
     if(CUSTOMER_CODE==CC_ASE_SG && bTesterSendPause==true)
        return "ASE_SG: the tester sent PAUSE (golden note.cpp:3833-3834)";    // golden :3834 return;
    return 0;
}
//------------------------------------------------------------------------------
//  golden TfNote::BtnPauseClick's KeyCode==0 arm (note.cpp:4044-4153) and TfNote::FormClose (note.cpp:2503-2762).
//  Returns true when it ran for this requestId (false: no snapshot for it -- nothing applied).
//------------------------------------------------------------------------------
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
//                sFixBundleID[2]=fNote->edBundleID->Text;
                sBundleEndInfo=GetBundleInfo(8);
                EventReport(SECS_EVENT.BundleEnd_Fix3);
                EventReport(SECS_EVENT.BundleEnd_IDREAD_Fix3);

                slDupUnloadBundlID->Clear();
                slDupUnloadBundlID->LoadFromFile(aslDupUnloadBundlID);
                slDupUnloadBundlID->Add(sUnloadBundleID);
                slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);

                sFixBundleID[2]="";
                RecordProcess("Fix3 Bundle End event report finished.");
                bNeedReportBundleID[eFix3]=false;                               //JerryYang 20250220 : fix AUTO IN OUT
            }
        }
    }
#endif // GATE(W906-J5-ACK) C9
// [W906] SCOPE(W906-J5-ACK): golden :2751-2758 -- L46 AStream "compress one cycle" restarts the machine (fMain->Start; the port's equivalent is StartFromWeb).  The dependency exists; starting motion from an acknowledgement is outside INBOX 119 -- a user decision  [kept as comment lines, not #if 0: tools/start_sites_census.py counts a gated fMain->Start( as a start site (ctest START_SitesCensus pins 34 / 30 / 4)]
//    if(IniConfig.bL46_AStreamErrorCompressOnecycle)                             //Ztex 2024.10.01 Add AStream Error Compress Onecycle
//    {
//        if(iAStreamErrorCompressOnecycle==3)
//        {
//            iAStreamErrorCompressOnecycle=0;
//            fMain->Start("bL46_AStreamErrorCompressOnecycle");
//        }
//    }
    hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);                          //Steven 20220823 : 機台有暫停就要重新計算
    tGalilTwoYMoveDelay.SetSecAndOn(60);
#if 0 // GATE(W906-J5-ACK) C10: golden :2761 -- bPLCFlag has no definition in this tree (git grep 20260930: 0 hits)
    bPLCFlag=false;                                                             //KenHsieh 20250307 : fix PLC safedoor 通訊延遲問題
#endif // GATE(W906-J5-ACK) C10
    }
    passSec=(unsigned long)PassTime;
    (void)KeyCode;
    }
    else if (mine)
    {
    W906_Notice.valid=false;                                                    // [W906] pause 3: golden shows no note -- nothing to close
    }
    if (pauseOut)      *pauseOut=pause;
    if (jamCountedOut) *jamCountedOut=jam;
    if (passTimeOut)   *passTimeOut=passSec;
    return mine;
}

//------------------------------------------------------------------------------
//  AI(W906-I37A) 20261002: INBOX 123 (NB2 R122 s2; RULINGS_20261002 #5, Q-C = with the SOFT_SIMULTE key records) -- a BLOCKING alarm
//  answered with PAUSE.  golden TfNote::BtnPauseClick (906 note.cpp:3826; V912 :3866), its Select[] arm for the selected key:
//    :3840  bStartMoveSpeed=false;                (the panel path, wb_serve.cpp:7380, already did it; the web path had not)
//    :3980-3982  ReturnCode=KeyComp[i]; SoftStop=true; SoftStart=false;   -- the callers (wb_serve.cpp :622-624 / :691) do these
//    :3983-4034  #ifdef SOFT_SIMULTE: the per-key EventLog row + EventReport(DoSkip / DoRetry / ...)
//    :4036-4038  EventReport(SECS_EVENT.DoPause) (SECS on) and SendCommand_ESD(ESD_SYSTEM_STOP)   (906 :4036-4038 / V912 :4076-4078)
//    then Close() -> FormClose: the callers' W906_NoteJamCountOnClose / W906_NoteFormCloseAlarmClear, after this call (golden order).
//  k = the answered key (K_SKIP ...), mapped back to golden's i through the same KeyComp table (:3836).  The two callers are the
//  only blocking PAUSE exits in the tree (the web answer and the panel IO key).  The notice (KeyCode==0) PAUSE is
//  W906_NoteNoticeAckLikeGolden above.  Nothing here moves anything (the machine stopped when the alarm was posted).
//------------------------------------------------------------------------------
void W906_NoteBlockingPauseLikeGolden(int k)
{
    void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);   // cMyDB.h:129 (golden cMyDB.h:62); all three given here
    bStartMoveSpeed=false;                                                      //Steven 20231018 : Fixed for G14   [golden :3840]
#ifdef SOFT_SIMULTE
    {
    const int KeyComp[]={K_SKIP, K_RETRY, K_TRAY_FEED, K_TRAY_END, K_CLEAN_OUT, K_RESET, K_HOME, K_TRAIN,K_ONECYCLE};   // golden :3836
    int i=-1;
    for (int j=0; j<(int)(sizeof(KeyComp)/sizeof(KeyComp[0])); j++) if (KeyComp[j]==k) { i=j; break; }   // [W906] golden's loop index
                if(     i==0)
                {
                    NewRecordProcess("MES2120", "SKIP pressed", "Note_BtnPauseClick");
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoSkip);                         //29     按下 Skip
                }
                else if(i==1)
                {
                    NewRecordProcess("MES2121", "RETRY pressed", "Note_BtnPauseClick");
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoRetry);                        //28     按下 Retry
                }
                else if(i==2)
                {
                    NewRecordProcess("MES2118", "TRAY FEED pressed", "Note_BtnPauseClick");
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoTrayFeed);                     //32     按下 Tray Feed
                }
                else if(i==3)
                {
                    NewRecordProcess("MES2119", "TRAY END pressed", "Note_BtnPauseClick");
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoTrayEnd);                      //31     按下 Tray End
                }
                else if(i==4)
                {
                    NewRecordProcess("MES2114", "CLEAN OUT pressed", "Note_BtnPauseClick");
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoCleanOut);                     // 4     按下 Clean Out
                }
                else if(i==5)
                {
                    NewRecordProcess("MES2113", "RESET pressed", "Note_BtnPauseClick");
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoReset);                        //33     按下 Reset
                }
                else if(i==6)
                {
                    NewRecordProcess("MES2122", "HOME & Retry pressed", "Note_BtnPauseClick");                          //ChungHung HT9045 2011/12/13
                    if(IniConfig.bEnable_SECS_GEM==true)                        //Steven 20140528 : Secs Gem
                        EventReport(SECS_EVENT.DoHome);                         //25     按下 Home
                }
                else if(i==7)
                {
                    NewRecordProcess("MES2123", "Auto Training", "Note_BtnPauseClick");
                }
                else if(i==8)
                {
                    NewRecordProcess("MES2124", "Fix pressed", "Note_BtnPauseClick");
                }
    }
#endif
            if(IniConfig.bEnable_SECS_GEM==true)                                //Steven 20140528 : Secs Gem
                EventReport(SECS_EVENT.DoPause);                                // 2     按下 Pause
            SendCommand_ESD(ESD_SYSTEM_STOP);                                   //Steven 20140722
}
