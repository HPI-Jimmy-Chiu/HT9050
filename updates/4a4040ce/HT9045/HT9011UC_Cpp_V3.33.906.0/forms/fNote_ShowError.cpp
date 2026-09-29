// =============================================================================
//  forms/fNote_ShowError.cpp  --  golden ShowErrorMessage's alarm-record half, TfNote::ErrShowToForm and
//                                  CheckRecordJamDrivingRecord
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
static bool bAutoCount_Reset=false;                                             // golden note.cpp:77 (its reader, FormClose :2612-2614, is not ported)
int iEventID;                                                                   // golden note.cpp:82 (its reader, FormClose :2557 MyDBUEventRecover, is not ported)
int iDuplicateError=0;                                                          // golden note.cpp:84 (its other golden readers -- :1000 SaveErrEventLog, FormClose :2528 -- are not ported; forms/fNote_JamCount.cpp reads canary's LastDuplicate instead)
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
