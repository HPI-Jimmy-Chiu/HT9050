// ===========================================================================
//  common_ChangeLog.cpp -- AI(W906-CHGLOG) 20260927
//
//  The CHANGE-LOG half of golden's WriteIniData overloads (common.cpp:622-1099)
//  and TempChangeLog (common.cpp:1802-2037), GENERATED from golden (cp950) by
//  tools/chglog/gen_common_changelog.py: every golden line is verbatim except the
//  substitutions marked on their lines.
//
//  WHY A SEPARATE FILE IN ht9045_sm: common.cpp lives in ht9045_core, which
//  links only vclcompat + ht9045_public.  The change-log block needs
//  InitialOK / asTempCtrl / TestIF_File (ht9045_globals), RecordChangeLogProcess
//  (ht9045_db) and fContactForm (ht9045_forms).  The 20260721 attempt to
//  un-gate TempChangeLog inside common.cpp broke the link of test_common /
//  test_ini_helpers (they link ht9045_core alone).  So common.cpp's five
//  WriteIniData bodies call a function POINTER (common.h EOF) at golden's exact
//  spot -- after the old value is read, before the write -- and this file,
//  in the top library, supplies the bodies.  wb_serve installs them at boot
//  (tools/wb_serve.cpp:3858, W906_InstallChangeLogHooks()); tests that want
//  them call the functions directly.  Nothing else changes: a program that
//  does not install the hooks behaves exactly as before this change.
//
//  DEVIATIONS (each also marked on its line):
//   (a) varargs: golden passes AnsiString straight to sprintf (BCB6's AnsiString
//       is one pointer); here .c_str().
//   (b) Item captions: golden reads fCleaning->X / fContact->X / FTestIF->X
//       ->Items->Strings[i].  Those facades do not hold the lists (fCleaning.h
//       has no widgets; FTestIF's list members are empty or gated), so:
//         - lists golden never edits at runtime come from golden's .dfm, verbatim
//           (measured 0927: golden edits Items only in cContact.cpp:205-234
//           (cbContactMode) and cTesterIF.cpp:48-89 (cbGPIBType, cbDIOType));
//         - cbContactMode: the real facade fContactForm (forms/fContact.h, its
//           Init() runs golden :205-235 at wb_serve boot);
//         - cbGPIBType: golden's ctor choice (ASE Kaohsiung list, else .dfm);
//         - cbDIOType: golden InitcbDIOType's listing of DIOCFGPath *.ini, taken
//           when the log line is written instead of when the form was built.
//       An index outside the list gives "".  NB2 R87 measured BCB6 (VCL source
//       + a probe): a TComboBox (cbContactMode, coD41, cbGPIBType, cbDIOType,
//       ContactMode) also gives "" -- same; a TRadioGroup (the other 7) raises
//       EStringListError before the write, so that key is NOT saved.  Whether
//       to reproduce that is NIGHT_REPORT decision 25 (NB2 R87-RANGE); today "".
//   (c) TempChangeLog: golden indexes asTempCtrl / its three local tables with
//       no range check (undefined behaviour on a malformed key); here an
//       out-of-range index leaves Name unchanged.
//   (d) WriteIniData(double), golden common.cpp:891 `ret!=Str` (ret double,
//       Str the "%0.4f" AnsiString): bcc32 5.6.4 builds Variant(ret) !=
//       Variant(Str), a NUMERIC compare (NB2 R87: probe + -S assembly; 106
//       customer double records, 0 with equal sides).  vclcompat would take
//       AnsiString(ret) ("1.5") != "1.5000" as a string compare and log every
//       double write, so the port writes ret!=Str.ToDouble().
//   NOT HERE:
//   - FormHS->RecordChangeLogByLot (the CosFunction.bUseChangeLogByLot tail):
//     customer option, still gated in common.cpp.
//   - Where the record lands: RecordChangeLogProcess -> MyDBIProcess("ChangeLog")
//     reaches the SECSGEM/uHGemEquipment.cpp:3480 stand-in until cMyDB P4
//     (St02 0927 03:06): the old==>new text is counted, not yet written.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "common.h"             // DIOCFGPath; the W906_ChangeLogHook_* pointers
#include "common_ChangeLog.h"
#include "cmydef.h"             // InitialOK, asTempCtrl[tcTotalCount], CUSTOMER_CODE
#include "cprod.h"              // TestIF_File
#include "cpublic.h"            // ConvertToMMType
#include "cMyDB.h"              // RecordChangeLogProcess
#include "MachineType.h"        // _8Site2X4, CC_ASE_KaohSiung
#include "forms/fContact.h"     // fContactForm (the TfContact facade; `fContact` itself is TfContactShim)

#include <windows.h>
#include <cstdlib>
#include <cstring>

// ---------------------------------------------------------------------------
//  (b) Item captions
// ---------------------------------------------------------------------------
static AnsiString CL_Pick(const char* const* a, int n, int i)
{
    return (i >= 0 && i < n) ? AnsiString(a[i]) : AnsiString("");
}

static AnsiString CL_fCleaning_rgCleanKitType(int i)   // golden uCleaning.dfm:967-968 (object rgCleanKitType at :957)
{
    static const char* const a[] = {"Kit", "Tray"};
    return CL_Pick(a, 2, i);
}

static AnsiString CL_fCleaning_rgAutoCleanSelectArm(int i)   // golden uCleaning.dfm:2887-2889 (object rgAutoCleanSelectArm at :2879)
{
    static const char* const a[] = {"Arm1", "Arm2", "Arm1 & Arm2"};
    return CL_Pick(a, 3, i);
}

static AnsiString CL_fCleaning_ContactMode(int i)   // golden uCleaning.dfm:2992-2993 (object ContactMode at :2977)
{
    static const char* const a[] = {"Direct Contact Mode", "Drop Contact Mode"};
    return CL_Pick(a, 2, i);
}

static AnsiString CL_fContact_coD41(int i)   // golden cContact.dfm:18593-18594 (object coD41 at :18576)
{
    static const char* const a[] = {"Inside socket", "Above Socket"};
    return CL_Pick(a, 2, i);
}

static AnsiString CL_FTestIF_rgInterfaceType(int i)   // golden cTesterIF.dfm:50-53 (object rgInterfaceType at :35)
{
    static const char* const a[] = {"DIO", "GP-IB", "RS232", "TCP/IP"};
    return CL_Pick(a, 4, i);
}

static AnsiString CL_FTestIF_rgBaudRate(int i)   // golden cTesterIF.dfm:439-441 (object rgBaudRate at :425)
{
    static const char* const a[] = {"19200", "9600", "4800"};
    return CL_Pick(a, 3, i);
}

static AnsiString CL_FTestIF_rgBitLength(int i)   // golden cTesterIF.dfm:420-421 (object rgBitLength at :406)
{
    static const char* const a[] = {"7 Bits", "8 Bits"};
    return CL_Pick(a, 2, i);
}

static AnsiString CL_FTestIF_rgParity(int i)   // golden cTesterIF.dfm:381-383 (object rgParity at :367)
{
    static const char* const a[] = {"Even", "Odd", "None"};
    return CL_Pick(a, 3, i);
}

static AnsiString CL_FTestIF_rgStopBit(int i)   // golden cTesterIF.dfm:401-402 (object rgStopBit at :387)
{
    static const char* const a[] = {"1 Bit", "2Bits"};
    return CL_Pick(a, 2, i);
}

static AnsiString CL_FTestIF_cbGPIBType(int i)   // golden TFTestIF ctor cTesterIF.cpp:46-59 (ASE Kaohsiung rebuilds the list) else cTesterIF.dfm:201-210
{
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        static const char* const a[] = {"Advan type1", "256 Bin", "16 Bin", "32 Bin", "SPEA Type", "16 Bin GS", "32 Bin GS", "Advan T6577", "Qrovo Protocol"};
        return CL_Pick(a, 9, i);
    }
    static const char* const d[] = {"Advan type1", "255 Bin", "16 Bin", "32 Bin", "SPEA Type", "16 Bin GS", "32 Bin GS", "Advan T6577", "Qrovo Protocol", "Delta_Castle"};
    return CL_Pick(d, 10, i);
}

static AnsiString CL_FTestIF_cbDIOType(int i)   // golden TFTestIF::InitcbDIOType cTesterIF.cpp:69-106 (its list, built on demand)
{
    AnsiString szFileName, out = "";
    int n = 0;
    WIN32_FIND_DATAA filedata;
    HANDLE filehandle = FindFirstFileA((DIOCFGPath + "*.ini").c_str(), &filedata);
    if(filehandle!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if((filedata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)!=0 ||
                strcmp(filedata.cFileName, ".")==0 ||
                strcmp(filedata.cFileName, "..")==0)
                continue;

            if(ExtractFileExt(filedata.cFileName).LowerCase()==".ini")
            {
                szFileName=ChangeFileExt(ExtractFileName(filedata.cFileName), "");
                if(n==i)
                    out=szFileName;
                n++;
            }
        } while(FindNextFileA(filehandle, &filedata));
        FindClose(filehandle);
    }
    // golden's else-branch (MessageBox "DIO data has been lossed" + Terminate) belongs to building the form, not to a log line
    return out;
}

static AnsiString CL_fContact_cbContactMode(int i)   // golden cContact.cpp:205-235 via the facade that runs it (TfContact::Init)
{
    if(fContactForm==NULL || i<0 || i>=fContactForm->cbContactMode->Items->Count)
        return "";
    return fContactForm->cbContactMode->Items->Strings[i];
}

// ---------------------------------------------------------------------------
//  TempChangeLog -- golden common.cpp:1802-2037 (Ifor 20190930)
// ---------------------------------------------------------------------------
AnsiString __fastcall TempChangeLog(AnsiString Group, AnsiString Name)          //Ifor 20190930 : add Display temperature switch Site Name   // golden common.cpp:1802-2037
{
    int iSiteAdd=0;
    AnsiString asATCTempName[32]=   {"Aa1", "Ab1", "Ac1", "Ad1",
                                     "Ae1", "Af1", "Ag1", "Ah1",
                                     "Ba1", "Bb1", "Bc1", "Bd1",
                                     "Be1", "Bf1", "Bg1", "Bh1",
                                     "Aa2", "Ab2", "Ac2", "Ad2",
                                     "Ae2", "Af2", "Ag2", "Ah2",
                                     "Ba2", "Bb2", "Bc2", "Bd2",
                                     "Be2", "Bf2", "Bg2", "Bh2"
                                    };

    AnsiString asIniDelayName[10]=  {"Every first devices",
                                     "After ShowAlarm Message",
                                     "After Auto Clean Function",
                                     "When happen tested time below",
                                     "After Open Heat Door",
                                     "When Press Stop Over",
                                     "When No Full Site",
                                     "EOT monitor time",
                                     "OTD unlock",
                                     "SOT monitor time"
                                    };

    AnsiString asAutoCleanSpeed[4]= {"Auto Clean Input Arm Speed",
                                     "Auto Clean Shuttle Speed",
                                     "Auto Clean Index Arm Speed",
                                     "Auto Clean Input Arm Z Speed"
                                    };

    if(Group.Pos("AmbientHotLowOffSet")     ==1     ||                          //Ifor 20190930 : add Temp Change Site
       Group.Pos("AmbientHotMidOffSet")     ==1     ||
       Group.Pos("Low OffSet")              ==1     ||
       Group.Pos("Mid. OffSet")             ==1     ||
       Group.Pos("High OffSet")             ==1     ||
       Group.Pos("User OffSet")             ==1     ||
       Group.Pos("SingleTempLimit")         ==1     ||
       Group.Pos("Init Temp OffSet")        ==1     ||
       Group.Pos("TestOverTime Temp OffSet")==1     )
    {
        if(Name.Pos("CH")==1)                                                   //Ifor 20191015 : Fix 溫度校正曲線中非CH參數會發生異常導致無法寫入檔案
        {
            iSiteAdd=atoi(Name.SubString(3,Name.Length()-2).c_str());
            if(Group.Pos("SingleTempLimit")==1)                                 //Ifor 20190930 : add Single Temp Limit 不需-1
            {
                if(iSiteAdd>=0 && iSiteAdd<tcTotalCount) Name=asTempCtrl[iSiteAdd];   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked
            }
            else
            {
                if(iSiteAdd-1>=0 && iSiteAdd-1<tcTotalCount) Name=asTempCtrl[iSiteAdd-1];   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked
            }
        }
    }
    else if(Group.Pos("ATC")==1 && Name.Pos("ATCTempOffset")==1)
    {
        iSiteAdd=atoi(Name.SubString(15,Name.Length()-15).c_str());

        if(TestIF_File.iTestMode==_8Site2X4 && TestIF_File.bOctal_16Kit==true)  //JerryYang 20230828 : fix change log
        {
            if(iSiteAdd>=2)                                                     //避免小於0
            {
                iSiteAdd-=2;
            }
        }

        if(iSiteAdd>=0 && iSiteAdd<32) Name="OffSet_"+asATCTempName[iSiteAdd];   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked
    }
    else if(Group.Pos("InitialMode")==1)
    {
        if(Name.Pos("iInitialDelay")==1)
        {
            if(Name.Pos("iInitialDelay_")==1)
            {
                iSiteAdd=atoi(Name.SubString(15,Name.Length()-14).c_str())-1;
            }
            else
            {
                iSiteAdd=0;
            }
            if(iSiteAdd>=0 && iSiteAdd<10) Name=asIniDelayName[iSiteAdd];   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked
        }
        else if(Name.Pos("dInitialDelay_10")==1)
        {
            iSiteAdd=atoi(Name.SubString(15,Name.Length()-14).c_str())-1;
            if(iSiteAdd>=0 && iSiteAdd<10) Name=asIniDelayName[iSiteAdd];   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked
        }
        else if(Name=="bEveryFirstDeviceUseInitialDelay")
        {
            Name="bEvery First Device Use Initial Delay";
        }
        else if(Name=="bAfterShowAlarmMessageUseInitialDelay")
        {
            Name="bAfter Show Alarm Message Use Initial Delay";
        }
        else if(Name=="bWhenHappenTestedTimeBelowUseInitialDelay")
        {
            Name="bWhen Happen Tested Time Below Use Initial Delay";
        }
        else if(Name=="bAfterAutoCleanFunctionUseInitialDelay")
        {
            Name="bAfter Auto Clean Function Use Initial Delay";
        }
        else if(Name=="bAfterOpenHeatDoorUseInitialDelay")
        {
            Name="bAfter Open Heat Door Use Initial Delay";
        }
        else if(Name=="bWhenPressStopOverUseInitialDelay")
        {
            Name="bWhen Press Stop Over Use Initial Delay";
        }
        else if(Name=="bTestFinishToNextTestOver")
        {
            Name="bEOT monitor time Use Initial Delay";
        }
        else if(Name=="bTestStartToNextTestStart")
        {
            Name="bSOT monitor time Use Initial Delay";
        }
        else if(Name=="bOTDUnlockDelay")
        {
            Name="bOTD Unlock Use Initial Delay";
        }
        else if(Name=="iEveryFirstDeviceUseInitialDelay")
        {
            Name="When happen tested time below Trigger time (Sec)";
        }
        else if(Name=="iWhenPressStopOver")
        {
            Name="When Press Stop Over Trigger time (Sec)";
        }
        else if(Name=="iTestFinishToNextTestOver")
        {
            Name="EOT monitor time Trigger time (Sec)";
        }
        else if(Name=="dTeststartToNextTestStart")
        {
            Name="SOT monitor time Trigger time (Sec)";
        }
//        else if(Name=="")
//        {
//            Name="";
//        }
    }
    else if(Group.Pos("Time")==1)
    {
        if(Name=="Stary Delay")
        {
            Name="Start Delay";
        }
        else if(Name=="Initial Stary Delay")
        {
            Name="Initial Start Delay";
        }
        else if(Name=="Initial Stary Delay CT")
        {
            Name="Initial Start Delay Count";
        }
    }
    else if(Group.Pos("Mode")==1)
    {
        if(Name=="fSocketInitialICCheckPositionOffset")
        {
            Name="fSocket Initial IC Check Position Offset";
        }
        else if(Name=="iSocketInitialICCheckPosition")
        {
            Name="iSocket Initial IC Check Position";
        }
    }
    else if(Group.Pos("Configuration")==1)
    {
        if(Name.Pos("iAutoClean_MotorSpeed[")==1)
        {
            iSiteAdd=atoi(Name.SubString(23,1).c_str());
            if(iSiteAdd>=0 && iSiteAdd<4) Name=asAutoCleanSpeed[iSiteAdd];   //AI(W906-CHGLOG) 20260927: index guard (file banner (c)); golden indexes unchecked
        }
        else if(Name=="iAutoClean_AlarmCount")
        {
            Name="Auto Clean Alarm Count";
        }
        else if(Name=="iAutoClean_iPadThickness")
        {
            Name="Auto Clean Clean Pad Deviation";
        }
        else if(Name=="iAutoClean_ContactCleanHeight")
        {
            Name="Auto Clean Socket Position Offset";
        }
        else if(Name=="iAutoClean_IndexPickOffset")
        {
            Name="Auto Clean Index to Shuttle Pick Offset";
        }
        else if(Name=="iAutoClean_IndexReleaseOffset")
        {
            Name="Auto Clean Index to Shuttle Release Offset";
        }
        else if(Name=="iAutoClean_Shuttle1PickOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 Pick Offset";
        }
        else if(Name=="iAutoClean_Shuttle1PlaceOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 Place Offset";
        }
        else if(Name=="iAutoClean_Shuttle1XOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 X Offset";
        }
        else if(Name=="iAutoClean_Shuttle1YOffset")
        {
            Name="Auto Clean In Arm to Shuttle1 Y Offset";
        }
        else if(Name=="iAutoClean_Shuttle2PickOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 Pick Offset";
        }
        else if(Name=="iAutoClean_Shuttle2PlaceOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 Place Offset";
        }
        else if(Name=="iAutoClean_Shuttle2XOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 X Offset";
        }
        else if(Name=="iAutoClean_Shuttle2YOffset")
        {
            Name="Auto Clean In Arm to Shuttle2 Y Offset";
        }
        else if(Name=="ShuttlePitchOffset")
        {
            Name="Auto Clean In Arm Shuttle Pitch Offset";
        }
    }
    return Name;
}

// ---------------------------------------------------------------------------
//  The five WriteIniData change-log blocks
// ---------------------------------------------------------------------------
// golden WriteIniData bool (common.cpp:622-689): the change-log block :637-671, verbatim; locals from :626/:628.
void W906_ChangeLog_Bool(AnsiString FileName, AnsiString Group, AnsiString Name, bool ret, bool bValue, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)
{
    AnsiString StrChangeName="";                                                //Ifor 20190930 : add Display temperature switch Site Name   // golden :626
    bool bStr2HasFind=false;                                                    //Ifor 20191021 : 整理Even Log   // golden :628
    if(ret!=bValue && InitialOK==true)                                          //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        StrChangeName=TempChangeLog(Group,Name);                                //Ifor 20190930 : add Display temperature switch Site Name
        if(FileName.Pos("Offset")==0)                                           //Ifor 20191004 : add Change Log 是否為Offset 資料
        {
            Str1.sprintf("%s_%s change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
        }
        else
        {
            Str1.sprintf("%s_%s Offset change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
        }

        if(FileName.Pos("HandlerCondition.Data")!=0)
        {
            if(Group=="Configuration")
            {
                bStr2HasFind=true;
                if(Name=="bAutoClean_UseTray")
                {
                    Str2.sprintf("%s ==> %s", CL_fCleaning_rgCleanKitType(ret).c_str(), CL_fCleaning_rgCleanKitType(bValue).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                }
                else
                {
                    bStr2HasFind=false;
                }
            }
        }

        if(bStr2HasFind==false)
        {
            Str2.sprintf("%d==>%d", ret, bValue);
        }
        RecordChangeLogProcess(Str1.c_str(), Str2.c_str());                     //wei 20180625 offset Change log紀錄
        bHasChange=true;
    }
}

// golden WriteIniData int (common.cpp:691-873): the change-log block :706-855, verbatim; locals from :695/:697.
void W906_ChangeLog_Int(AnsiString FileName, AnsiString Group, AnsiString Name, int ret, int Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)
{
    AnsiString StrChangeName="";                                                //Ifor 20190930 : add Display temperature switch Site Name   // golden :695
    bool bStr2HasFind=false;                                                    //Ifor 20191021 : 整理Even Log   // golden :697
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        if(FileName.Pos("config.ini")>1 && Group=="O_Count" && Name.Pos("O_1")==1)
        {
                                                                                //Ifor 20191017 : Head Contact Count Change 不記錄
        }
        else
        {
            StrChangeName=TempChangeLog(Group,Name);                            //Ifor 20190930 : add Display temperature switch Site Name

            if(FileName.Pos("Offset")==0)                                       //Ifor 20191004 : add Change Log 是否為Offset 資料
            {
                Str1.sprintf("%s_%s change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
            }
            else
            {
                Str1.sprintf("%s_%s Offset change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
            }

            if(FileName.Pos("Contact.Data")!=0)
            {
                if(Group=="Mode")
                {
                    bStr2HasFind=true;
                    if(Name=="Contact")
                    {
                        Str2.sprintf("%s ==> %s", CL_fContact_cbContactMode(ret).c_str(), CL_fContact_cbContactMode(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else if(Name=="iSocketInitialICCheckPosition")
                    {
                        Str2.sprintf("%s ==> %s", CL_fContact_coD41(ret).c_str(), CL_fContact_coD41(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else
                    {
                        bStr2HasFind=false;
                    }
                }
            }
            else if(FileName.Pos("Tester.Data")!=0)
            {
                if(Group=="Mode")
                {
                    bStr2HasFind=true;
                    if(Name=="Tester Type")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_rgInterfaceType(ret).c_str(), CL_FTestIF_rgInterfaceType(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else
                    {
                        bStr2HasFind=false;
                    }
                }
                else if(Group=="GP-IB")
                {
                    bStr2HasFind=true;
                    if(Name=="Type")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_cbGPIBType(ret).c_str(), CL_FTestIF_cbGPIBType(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else
                    {
                        bStr2HasFind=false;
                    }
                }
                else if(Group=="DIO")
                {
                    bStr2HasFind=true;
                    if(Name=="Type")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_cbDIOType(ret).c_str(), CL_FTestIF_cbDIOType(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else
                    {
                        bStr2HasFind=false;
                    }
                }
                else if(Group=="RS-232C")
                {
                    bStr2HasFind=true;
                    if(Name=="Baud Rate")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_rgBaudRate(ret).c_str(), CL_FTestIF_rgBaudRate(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else if(Name=="Bit Length")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_rgBitLength(ret).c_str(), CL_FTestIF_rgBitLength(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else if(Name=="Parity")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_rgParity(ret).c_str(), CL_FTestIF_rgParity(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else if(Name=="Stop Bit")
                    {
                        Str2.sprintf("%s ==> %s", CL_FTestIF_rgStopBit(ret).c_str(), CL_FTestIF_rgStopBit(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else
                    {
                        bStr2HasFind=false;
                    }
                }
            }
            else if(FileName.Pos("HandlerCondition.Data")!=0)
            {
                if(Group=="Configuration")
                {
                    bStr2HasFind=true;
                    if(Name=="iAutoClean_SelectArm")
                    {
                        Str2.sprintf("%s ==> %s", CL_fCleaning_rgAutoCleanSelectArm(ret).c_str(), CL_fCleaning_rgAutoCleanSelectArm(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else if(Name=="iAutoClean_iPadThickness"        ||
                            Name=="iAutoClean_DropHigh"             ||
                            Name=="iAutoClean_ContactShiftHeight"   ||
                            Name=="iAutoClean_ContactCleanHeight"   ||
                            Name=="iAutoClean_IndexPickOffset"      ||
                            Name=="iAutoClean_IndexReleaseOffset"   ||
                            Name=="iAutoClean_Shuttle1PickOffset"   ||
                            Name=="iAutoClean_Shuttle1PlaceOffset"  ||
                            Name=="iAutoClean_Shuttle1XOffset"      ||
                            Name=="iAutoClean_Shuttle1YOffset"      ||
                            Name=="iAutoClean_Shuttle2PickOffset"   ||
                            Name=="iAutoClean_Shuttle2PlaceOffset"  ||
                            Name=="iAutoClean_Shuttle2XOffset"      ||
                            Name=="iAutoClean_Shuttle2YOffset"      )
                    {
                        Str2.sprintf("%smm ==> %smm", AnsiString(ConvertToMMType(ret)).c_str(), AnsiString(ConvertToMMType(Value)).c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
                    }
                    else if(Name=="iAutoClean_ContactMode")
                    {
                        Str2.sprintf("%s ==> %s", CL_fCleaning_ContactMode(ret).c_str(), CL_fCleaning_ContactMode(Value).c_str());   //AI(W906-CHGLOG) 20260927: golden reads the VCL widget's Items; here CL_<form>_<widget>() (file banner (b))
                    }
                    else if(Name=="iAutoClean_ContactTime")
                    {
                        Str2.sprintf("%0.1fs ==> %0.1fs", (double)ret/10, (double)Value/10);
                    }
                    else
                    {
                        bStr2HasFind=false;
                    }
                }
            }

            if(bStr2HasFind==false)
            {
                Str2.sprintf("%d==>%d", ret, Value);
            }
            RecordChangeLogProcess(Str1.c_str(), Str2.c_str());                 //wei 20180625 offset Change log紀錄
            bHasChange=true;
        }
    }
}

// golden WriteIniData ul (common.cpp:974-1019): the change-log block :988-1002, verbatim; locals from :978.
void W906_ChangeLog_ULong(AnsiString FileName, AnsiString Group, AnsiString Name, unsigned long ret, unsigned long Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)
{
    AnsiString StrChangeName="";                                                //Ifor 20190930 : add Display temperature switch Site Name   // golden :978
    if(ret!=Value && InitialOK==true)                                           //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        StrChangeName=TempChangeLog(Group, Name);                               //Ifor 20190930 : add Display temperature switch Site Name
        if(FileName.Pos("Offset")==0)                                           //Ifor 20191004 : add Change Log 是否為Offset 資料
        {
            Str1.sprintf("%s_%s change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
        }
        else
        {
            Str1.sprintf("%s_%s Offset change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
        }
        Str2.sprintf("%d==>%d", ret, Value);
        RecordChangeLogProcess(Str1.c_str(), Str2.c_str());                     //wei 20180625 offset Change log紀錄
        bHasChange=true;
    }
}

// golden WriteIniData dbl (common.cpp:875-972): the change-log block :891-954, verbatim; locals from :879/:881.
void W906_ChangeLog_Double(AnsiString FileName, AnsiString Group, AnsiString Name, double ret, double Value, AnsiString Str, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)
{
    AnsiString StrChangeName="";                                                //Ifor 20190930 : add Display temperature switch Site Name   // golden :879
    bool bStr2HasFind=false;                                                    //Ifor 20191021 : 整理Even Log   // golden :881
    if(ret!=Str.ToDouble() && InitialOK==true)                                             //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常   //AI(W906-CHGLOG) 20260927: bcc32 5.6.4 compiles ret!=Str as Variant(ret)!=Variant(Str), a NUMERIC compare (NB2 R87, measured: 1.5 vs "1.5000" equal); vclcompat would compare AnsiString(ret)="1.5" with "1.5000" as strings and log every double write
    {
        StrChangeName=TempChangeLog(Group,Name);                                //Ifor 20190930 : add Display temperature switch Site Name
        if(FileName.Pos("Offset")==0)                                           //Ifor 20191004 : add Change Log 是否為Offset 資料
        {
            Str1.sprintf("%s_%s change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
        }
        else
        {
            Str1.sprintf("%s_%s Offset change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
        }

        if(StrChangeName=="Contact" &&
           (Group=="Test Arm1" || Group=="Test Arm2"))                          //kevin 20210519 Add : For ASE 高雄 扭力值存Log 修改 INDEX OFFSET
        {
            if(TestIF_File.bEnableReadAndCheckTorque)                           //kevin 20210804
            {
                bResetArm1Value=true;                                           //修改index contact offset 參數 重新設定扭力標準值
                bResetArm2Value=true;
            }
        }

        if(FileName.Pos("Contact.Data")!=0)                                     //Ifor 20191015 : add Kit Diameter switch Name
        {
            if(Group=="Mode")
            {
                bStr2HasFind=true;
                if(Name=="Kit Diameter")
                {
                    Str2.sprintf("%0.1fmm ==>%0.1fmm", ret*10, Value*10);
                }
                else
                {
                    bStr2HasFind=false;
                }
            }
        }
        else if(FileName.Pos("HandlerCondition.Data")!=0)
        {
            if(Group=="Configuration")
            {
                bStr2HasFind=true;
                if(Name=="ShuttlePitchOffset"       ||
                   Name=="dAutoClean_XStart_Kit"    ||
                   Name=="dAutoClean_XPitch_Kit"    ||
                   Name=="dAutoClean_YStart_Kit"    ||
                   Name=="dAutoClean_YPitch_Kit"    )
                {
                    Str2.sprintf("%smm ==> %smm", AnsiString(ConvertToMMType(ret)).c_str(), AnsiString(ConvertToMMType(Value)).c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
                }
                else
                {
                    bStr2HasFind=false;
                }
            }
        }

        if(bStr2HasFind==false)
        {
            Str2.sprintf("%0.4f==>%0.4f", ret, Value);
        }
        RecordChangeLogProcess(Str1.c_str(), Str2.c_str());                     //wei 20180625 offset Change log紀錄
        bHasChange=true;
    }
}

// golden WriteIniData str (common.cpp:1021-1099): the change-log block :1055-1081, verbatim; locals from :1025/:1027/:1028/:1029.
void W906_ChangeLog_Str(AnsiString FileName, AnsiString Group, AnsiString Name, AnsiString ret, AnsiString Value, AnsiString& Str1, AnsiString& Str2, bool& bHasChange)
{
    AnsiString StrChangeName="";                                                //Ifor 20190930 : add Display temperature switch Site Name   // golden :1025
    bool bStrIsFloat1=false;   // golden :1027
    bool bStrIsFloat2=false;   // golden :1028
    double freg=0;   // golden :1029
    // golden :1053-1054
    bStrIsFloat1=TryStrToFloat(ret.c_str(), freg);
    bStrIsFloat2=TryStrToFloat(Value.c_str(), freg);
    if(InitialOK==true)                                                         //Ifor 20190919 : add 避免程式開啟時因客戶要求強制開啟功能寫入時發生異常
    {
        if((bStrIsFloat1==true && bStrIsFloat2==true && atof(ret.c_str())!=atof(Value.c_str())) ||
           (bStrIsFloat1==false && bStrIsFloat2==false && ret!=Value))
        {
            StrChangeName=TempChangeLog(Group,Name);                            //Ifor 20190930 : add Display temperature switch Site Name
            if(FileName.Pos("Offset")==0)                                       //Ifor 20191004 : add Change Log 是否為Offset 資料
            {
                Str1.sprintf("%s_%s change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
            }
            else
            {
                Str1.sprintf("%s_%s Offset change Value",Group.c_str() , StrChangeName.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
            }

            if(FileName.Pos("Contact.Data")!=0 && Group=="Mode" && Name=="fSocketInitialICCheckPositionOffset")
            {
                Str2.sprintf("%0.2fmm ==> %0.2fmm", atof(ret.c_str()), atof(Value.c_str()));
            }
            else
            {
                Str2.sprintf("%s==>%s", ret.c_str(), Value.c_str());   //AI(W906-CHGLOG) 20260927: AnsiString passed through sprintf ... gets .c_str() (a class object cannot go through varargs)
            }
            RecordChangeLogProcess(Str1, Str2);                                 //wei 20180625 offset Change log紀錄
            bHasChange=true;
        }
    }
}

// ---------------------------------------------------------------------------
void W906_InstallChangeLogHooks()
{
    W906_ChangeLogHook_Bool  = W906_ChangeLog_Bool;
    W906_ChangeLogHook_Int   = W906_ChangeLog_Int;
    W906_ChangeLogHook_ULong = W906_ChangeLog_ULong;
    W906_ChangeLogHook_Double = W906_ChangeLog_Double;
    W906_ChangeLogHook_Str   = W906_ChangeLog_Str;
}
