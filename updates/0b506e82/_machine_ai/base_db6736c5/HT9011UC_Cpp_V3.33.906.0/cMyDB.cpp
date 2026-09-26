// ===========================================================================
//  cMyDB.cpp  -- GA-1-B4 translation (sqlite-backed logging / query wrapper)
//  Golden ref: D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\cMyDB.cpp (2,047 lines)
//               + cMyDB.h
//
//  BCB6 preamble dropped per migration plan (no portable meaning):
//    #include "MachineDefine.h" / #pragma hdrstop
//
//  sqlite3 ENGINE: this file links against the vendored amalgamation at
//  third_party/sqlite3/ (CMake target `sqlite3`, same version -- 3.7.7.1 -- as
//  golden's sqlite3.h).  Golden's own `//#include "sqlite3.h"` is commented out
//  (it reaches sqlite3.h transitively through a precompiled-header chain in
//  BCB6); here it is a plain, active #include.
//
//  ===========================================================================
//  FUNCTION TALLY (golden cMyDB.h declares 33 free functions with a real body
//  in cMyDB.cpp -- MyDBULotInfo()/MyDBULotInfo(TStringGrid*) are declared but
//  never defined anywhere in golden, see below -- plus 3 file-local helpers
//  [MyDBExecSQL, DoInsertAlarmCode, SaveEventTracker] = 36 bodies total; see
//  the GA-1-B4 report for the authoritative table)
//  ===========================================================================
//  ACTIVE (translated, compiles and runs against the real sqlite3 engine):
//    MyDBCloseDB, MyDBOpenDB, MyDBExecSQL (helper), DoInsertAlarmCode (helper),
//    MyDBITotalLoader, MyDBITimeData, MyDBIUPH, RecordTimeData (Summary-Report
//    sub-block gated, rest active), MyDBIProductionData, MyDBIEvent,
//    MyDBULotEndTime, MyDBULotData, MyDBUEventRecover, MyDBQLotData,
//    MyDBQMotMess, GetMyDBIMessage, MyDBQClearDT, MyDBQTotalLoader,
//    MyDBQAlarmCodeList, MyDBQTimeData, MyDBVACUUM, MyDBVEventFreq,
//    MyDBVProcess, GetAlarmCodeList, MyDBVProcessFilter,
//    RecordChangeLogProcess, GetJameCodeOfAxis, SaveEventTracker (helper,
//    dispatch logic only -- file I/O gated), SaveEventLogInfo (file I/O
//    gated), MyDBUpdateDB (fMain-map priming gated, rest active).
//
//  WHOLE-FUNCTION GATED (no faithful path to compile, ACTIVE stub provided):
//    MyDBVUnitEventCount, MyDBVAxleEventCount -- both take `TChart *Chart`
//    (golden <Chart.hpp> TeeChart VCL component); TChart has no port anywhere
//    in this tree (W7-UI scope).  Stub returns 0 (no chart series to sum).
//
//  HOMECOMING GATED (golden body preserved verbatim in #if 0; a real, reduced,
//  non-golden-faithful stand-in for the SAME external symbol already lives
//  elsewhere in this tree -- defining it again here would be a duplicate
//  definition / link error.  See cMyDB.h's HOMECOMING NOTICE for the full
//  writeup and the GA-1-B4 report for the integrator swap steps):
//    MyDBIProcess     (3-arg, __fastcall) -- real home: SECSGEM/uHGemEquipment.cpp:3475
//    MyDBIProcessNew  (4-arg)             -- real home: canary_support.cpp:453
//    RecordProcess    (2-arg)             -- real home: canary_support.cpp:102
//    NewRecordProcess (3-arg)             -- real home: acatchtray_shims.cpp:132
//
//  NOT CARRIED FORWARD: MyDBULotInfo() / MyDBULotInfo(TStringGrid*) -- golden
//  cMyDB.h declares them but NO definition exists anywhere in the golden tree
//  (whole-tree grep, zero hits beyond the header's own two declaration lines).
//  Dead/vestigial API in golden itself; see cMyDB.h's own note.
//
//  ===========================================================================
//  DEPENDENCY GAPS (narrow #if 0 // TODO(GA1-B4) blocks at each call site;
//  every gate below is individually commented in-place at its call site too):
//  ===========================================================================
//    * forms/fMain.h facade has NO AlarmCodeMap / AlarmCodeList / AlarmCodeIter
//      / UnitNameMap / slProdRecordLog / slTimeData / TimerRecordLoaderDate()
//      members (checked: not present anywhere in the current facade).
//    * `slEventLog` (cmydef.h:117 `extern TMyStringList *slEventLog;`) points
//      at a type that is still an OPAQUE forward declaration tree-wide
//      (cmydef.h:15 `class TMyStringList;`, no member ever defined) -- its
//      AddTextWithDateTime/AddTextWithLineNo extension methods cannot be
//      called against an incomplete type.  The `slEventLog!=NULL` pointer
//      comparisons themselves stay ACTIVE (need no complete type); only the
//      method calls inside are gated.
//    * fContactCT, fSecurity, fConfiguration (general-purpose) -- no port
//      anywhere in this tree (fConfiguration DOES exist, but only as
//      Automation/SCK_ART_Remainder.h's module-scoped W5SckArtRem_ConfigStub;
//      pulling that stub into a general file like this one would be an
//      architecture layering violation, so it is treated as absent here).
//    * forms/fLotInfo.h has edtASECL_LotID but NOT edtASECL_TesterID /
//      cbbASECL_LoginMode / UploadEventLogFile.
//    * asProduct_LoaderPath, as9045LogPath, asSaveEventLogPath, DateSeparator
//      -- no declaration anywhere in the ported tree.
//    * vclcompat::TDateTime has no DayOfWeek() method (FormatDateTime(fmt,dt)
//      DOES exist and is used as the active replacement for every golden
//      `.FormatString(fmt)` call that is otherwise reachable).
//    * vclcompat::TStringGrid deliberately has no ColWidths[] (documented
//      out-of-scope in StringGrid.h -- "NO rendering... ColWidths/RowHeights").
//    * MNetLog (golden Motor/myMN200motor.h) -- no ported home; ADDED a
//      file-local no-op stand-in, same shape/precedent as
//      Motor/mymotor.cpp:54 and AutoClean/AutoClean.cpp:180.
//
//  FAITHFULNESS NOTES (golden quirks preserved, not "fixed"):
//    * MyDBCloseDB: `iCount` is declared but never incremented inside either
//      sqlite3_next_stmt loop, so the `if(iCount>100)` break is dead code
//      (golden :70-113) -- preserved verbatim.
//    * MyDBExecSQL: `int rowID` truncates sqlite3_last_insert_rowid's 64-bit
//      return (golden :148) -- preserved verbatim.
//    * MyDBIEvent: on the `CosFunction.bUseMDB==false` path, `rowID` is
//      returned WITHOUT ever being assigned (golden :605-721, `int rowID;`
//      declared, only ever written inside the `if(CosFunction.bUseMDB)`
//      branch) -- an indeterminate-value return is a genuine golden bug,
//      preserved verbatim (not zero-initialized here).
//    * MyDBIProductionData: `static iJamCnt=0;` (golden :505) is BCB6
//      implicit-int, ill-formed in C++17 -- given an explicit `int` here
//      (behaviour-identical, required for this to compile at all).
//    * MyDBVProcessFilter: returns `count-1` (golden :1541) even though
//      `count` was already used 0-based as the last-written row index one
//      line above -- looks off-by-one, preserved verbatim (not golden's only
//      caller is known, so "fixing" this would be a guess).
//    * GetJameCodeOfAxis / GetAlarmCodeList: `Str.SubString(4, 2)` reads
//      Axis as an always-2-digit substring starting at column 4 of the
//      "TTTUUmmmmmm" AlarmCodeList.txt key spelling -- preserved verbatim.
// ===========================================================================

#include "cMyDB.h"
#include "third_party/sqlite3/sqlite3.h"
#include "cmydef.h"                // SystemYear/.../SaveEventLog/slEventLog; transitively MachineType.h/cprod.h/cpublic.h/Config.h/CosFunction.h/myTimer.h
#include "common.h"                 // asDBPath, ConvertSecondToTime, GetTimeInfo, RespondASECom, ProductionLog, GetOnlyTimeInfoByString, MyForceDirectories, ReadIniData/WriteIniData, SGDToCSV
#include "LastSet.h"                // LastSet
#include "mycylin.h"                // Cylinder[], MaxCylinderItem
#include "forms/fMain.h"            // fMain
#include "forms/fLotInfo.h"         // fLotInfo

#include <cstdio>
#include <cstdlib>

//---------------------------------------------------------------------------
// Steven 20090817
// 對SQLite資料庫的控制  (SQLite database access control layer)
//---------------------------------------------------------------------------

sqlite3 *dbReadOnly;
sqlite3 *dbReadWrite;
AnsiString AlarmUnitNo[iAlarmUnitTotal];                                        //Steven 20231127 : 整理Alarm Unit
AnsiString AlarmUnit[iAlarmUnitTotal]={"Event",                                 //Steven 20231127 : 整理Alarm Unit
                                       "Input Arm",
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
                                       "ESD System",
                                       "Process Log",
                                       "Motion Log",
                                       "Cassette",
                                       "Motor",
                                       "Tray Unloader 4",                       //Steven 20201031 : Add for HT9011
                                       "Tray Unloader 5",
                                       "Tray Unloader 6",
                                       "Fix Tray 4",
                                       "Fix Tray 5",
                                       "Fix Tray 6",
                                       "Cylinder"                               //Steven 20231127 : 氣缸Alarm改成自動生成, 分類31
                                      };

// AI(GA1-B4) 20260804: MNetLog (golden Motor/myMN200motor.h) has no ported
// home yet -- same gap Motor/mymotor.cpp:54 and AutoClean/AutoClean.cpp:180
// already hit, both resolved with a file-local no-op stand-in of the same
// shape.  Kept consistent with that established precedent.
static void MNetLog(AnsiString /*Message*/) {}

//---------------------------------------------------------------------------
void __fastcall MyDBCloseDB()
{
    int ret=0;
    int iCount=0;

    if(CosFunction.bUseMDB==false)                                              //Steven 20210526 : 部分客戶取消使用MDB
        return;

    ret=sqlite3_close(dbReadOnly);
    if(ret==SQLITE_BUSY)
    {
        sqlite3_stmt * stmt;
        while((stmt=sqlite3_next_stmt(dbReadOnly, NULL))!=NULL)
        {
            sqlite3_finalize(stmt);
            if(iCount>100)
            {
                MNetLog("Close DB dbReadOnly Error! ");
                break;
            }
        }
        ret=sqlite3_close(dbReadOnly);
        if(ret!=SQLITE_OK)
        {
            ;                                                                   //例外處理
        }
    }

    ret=sqlite3_close(dbReadWrite);
    if(ret==SQLITE_BUSY)
    {
        sqlite3_stmt * stmt;
        while((stmt=sqlite3_next_stmt(dbReadWrite, NULL))!=NULL)
        {
            sqlite3_finalize(stmt);
            if(iCount>100)
            {
                MNetLog("Close DB dbReadWrite Error!");
                break;
            }
        }
        ret=sqlite3_close(dbReadWrite);
        if(ret!=SQLITE_OK)
        {
            ;                                                                   //例外處理
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall MyDBOpenDB()
{
    char *errMsg=NULL;
    if(CosFunction.bUseMDB)                                                     //Steven 20210526 : 部分客戶取消使用MDB
    {
        sqlite3_open_v2(asDBPath.c_str(), &dbReadWrite, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
        sqlite3_open_v2(asDBPath.c_str(), &dbReadOnly, SQLITE_OPEN_READONLY, NULL);

        sqlite3_exec(dbReadWrite, "PRAGMA synchronous = OFF", NULL, NULL, &errMsg);         //Steven 20150426 : 紀錄SQL存取速度
        sqlite3_exec(dbReadWrite, "PRAGMA journal_mode = MEMORY", NULL, NULL, &errMsg);

        sqlite3_exec(dbReadOnly, "PRAGMA synchronous = OFF", NULL, NULL, &errMsg);          //Steven 20150426 : 紀錄SQL存取速度
        sqlite3_exec(dbReadOnly, "PRAGMA journal_mode = MEMORY", NULL, NULL, &errMsg);
    }
}
//---------------------------------------------------------------------------
int MyDBExecSQL(AnsiString str)
{
    int ret=0;
    char *errMsg=NULL;
    int rowID=0;

    if(CosFunction.bUseMDB==false)                                              //Steven 20210526 : 部分客戶取消使用MDB
        return 0;

    ret=sqlite3_exec(dbReadWrite, str.c_str(), 0, 0, &errMsg);
    if(ret!=SQLITE_OK)
    {
        sqlite3_free(errMsg);
    }

    if(dbReadWrite!=NULL)
        rowID=sqlite3_last_insert_rowid(dbReadWrite);                          //AI(GA1-B4) 20260804: golden quirk -- 64-bit rowid truncated into `int`, preserved verbatim

    return rowID;
}
//------------------------------------------------------------------------------
void DoInsertAlarmCode(AnsiString Code, AnsiString Message)                     //Steven 20170202 (wei): Fixed for Unknown Alarm Code
{
    AnsiString sQuery, Str;
    int AlarmType=0;
    int UnitNo=atoi(Code.SubString(4, 2).c_str());
    int iCode=atoi(Code.SubString(6, 4).c_str());
    if(Code.AnsiPos("JAM")!=0)
    {
        AlarmType=1;
    }
    else if(Code.AnsiPos("WAR")!=0)
    {
        AlarmType=2;
    }
    else if(Code.AnsiPos("MES")!=0)
    {
        AlarmType=3;
    }

    int ID=AlarmType*100000000+UnitNo*10000000+iCode;                           //Steven 20230531 : Alarm ID 比照SECS GEM 格式共9碼 X    XX     XXXXXX
#if 0 // TODO(GA1-B4): fMain->AlarmCodeMap/AlarmCodeIter/AlarmCodeList not yet
      // ported (forms/fMain.h facade has no AlarmCode* members) -- golden
      // :173-184 in-memory dedup cache + AlarmCodeList.txt text-mirror persist.
    fMain->AlarmCodeIter=fMain->AlarmCodeMap.find(Code);                        //                                                  Type UnitNo Jam Code
    if(fMain->AlarmCodeIter!=fMain->AlarmCodeMap.end())
    {
                                                                                //重複
    }
    else
    {
        fMain->AlarmCodeMap[Code]=Message;
        Str.sprintf("%s=%s", Code, Message);
        fMain->AlarmCodeList->Add(Str);
        MyForceDirectories("D:\\HT9045\\Error\\", "DoInsertAlarmCode");
        fMain->AlarmCodeList->SaveToFile("D:\\HT9045\\Error\\AlarmCodeList.txt");

        sQuery.sprintf("INSERT OR REPLACE INTO 'AlarmList' VALUES(%d,  '%s',  %d,  %d, %d, '%s')", ID, Code, UnitNo, UnitNo*100, AlarmType, Message);
        MyDBExecSQL(sQuery);
    }
#else
    // ACTIVE fallback: golden's in-memory dedup is purely an optimization --
    // "INSERT OR REPLACE" below is idempotent on its own, so DB correctness
    // does not depend on the fMain-side cache.  What IS lost until
    // AlarmCodeMap/List land: the AlarmCodeList.txt text-mirror persistence.
    MyForceDirectories("D:\\HT9045\\Error\\", "DoInsertAlarmCode");
    sQuery.sprintf("INSERT OR REPLACE INTO 'AlarmList' VALUES(%d,  '%s',  %d,  %d, %d, '%s')", ID, Code, UnitNo, UnitNo*100, AlarmType, Message);
    MyDBExecSQL(sQuery);
#endif
}
//---------------------------------------------------------------------------
// 新增資料到表單中  (insert new data into tables)
//---------------------------------------------------------------------------
void __fastcall MyDBITotalLoader(int iLoader)
{
    AnsiString str, sTime;
    str.sprintf("INSERT INTO TotalLoader (LoadCount, OccurDateTime) "
                "VALUES(%d, datetime(CURRENT_TIMESTAMP, 'localtime'))",
                iLoader);
    MyDBExecSQL(str);
    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();

    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        GetTimeInfo();
        sTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        SL->Add("TimeDataTotalLoader");                                         //UnitName
        SL->Add("\t");                                                          //AlarmCode
        SL->Add(sTime);                                                         //OccurDateTime
        SL->Add("\t");                                                          //Recovery
        SL->Add("\t");                                                          //StopedTime
        SL->Add("\t");                                                          //Duplicate
        SL->Add(AnsiString(iLoader));                                           //Message
        SL->Add("\t");                                                          //ErrPart
    }
    else
    {
        SL->Add("TimeDataTotalLoader");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add(AnsiString(iLoader));
        SL->Add("\t");
    }

    if(slEventLog!=NULL)
    {
#if 0 // TODO(GA1-B4): TMyStringList (slEventLog's real type, cmydef.h:15) is
      // still an opaque forward decl tree-wide -- AddTextWithDateTime/
      // AddTextWithLineNo extension methods cannot be called against an
      // incomplete type.  Same gap recurs verbatim in every function below
      // that touches slEventLog; not re-explained after this point.
        if(IniConfig.bSPILFunction==true)                                       //Steven 20240604 : SPIL格式的event log
            slEventLog->AddTextWithLineNo(SL->CommaText);
        else
            slEventLog->AddTextWithDateTime(SL->CommaText);
        SaveEventLog();
#endif
    }
    SaveEventLogInfo("220000000", SL->CommaText, 22, " ");
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;
}
//---------------------------------------------------------------------------
int __fastcall MyDBITimeData(long StartTime, long HomeTime, long ContactTest, long PauseTime, long ProductTime, long JamTime, long PowerOn)    //Steven 20120628 : 顯示機台稼動時間
{
    AnsiString str, sTime;
    str.sprintf("INSERT INTO TimeData (StartTime, HomeTime, ContactTest, PauseTime, ProductTime, JamTime, PowerOn, OccurDateTime) "
                "VALUES(%i, %i, %i, %i, %i, %i, %i, datetime(CURRENT_TIMESTAMP, 'localtime'))",
                StartTime, HomeTime, ContactTest, PauseTime, ProductTime, JamTime, PowerOn);
    int rowID=MyDBExecSQL(str);

    str.sprintf("%i, %i, %i, %i, %i, %i, %i, ",
                StartTime, HomeTime, ContactTest, PauseTime, ProductTime, JamTime, PowerOn);

    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        GetTimeInfo();
        sTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        SL->Add("TimeData");                                                    //UnitName
        SL->Add("\t");                                                          //AlarmCode
        SL->Add(sTime);                                                         //OccurDateTime
        SL->Add("\t");                                                          //Recovery
        SL->Add("\t");                                                          //StopedTime
        SL->Add("\t");                                                          //Duplicate
        SL->Add(str);                                                           //Message
        SL->Add("\t");                                                          //ErrPart
    }
    else
    {
        SL->Add("TimeData");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add(str);
        SL->Add("\t");
    }

    if(slEventLog!=NULL)
    {
#if 0 // TODO(GA1-B4): see MyDBITotalLoader's note above (slEventLog opaque type)
        if(IniConfig.bSPILFunction==true)                                       //Steven 20240604 : SPIL格式的event log
            slEventLog->AddTextWithLineNo(SL->CommaText);
        else
            slEventLog->AddTextWithDateTime(SL->CommaText);
        SaveEventLog();
#endif
    }
    SaveEventLogInfo("220000000", SL->CommaText, 22, " ");
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;
    return rowID;
}
//---------------------------------------------------------------------------
void __fastcall MyDBIUPH(int UPH)                                               //Steven 20190906 : Add UPH in EventLog
{
    AnsiString str, sTime;

    str.sprintf("%i", UPH);

    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        GetTimeInfo();
        sTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        SL->Add("UPH");                                                         //UnitName
        SL->Add("\t");                                                          //AlarmCode
        SL->Add(sTime);                                                         //OccurDateTime
        SL->Add("\t");                                                          //Recovery
        SL->Add("\t");                                                          //StopedTime
        SL->Add("\t");                                                          //Duplicate
        SL->Add(str);                                                           //Message
        SL->Add("\t");                                                          //ErrPart
    }
    else
    {
        SL->Add("UPH");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add(str);
        SL->Add("\t");
    }

    if(slEventLog!=NULL)
    {
#if 0 // TODO(GA1-B4): see MyDBITotalLoader's note above (slEventLog opaque type)
        if(IniConfig.bSPILFunction==true)                                       //Steven 20240604 : SPIL格式的event log
            slEventLog->AddTextWithLineNo(SL->CommaText);
        else
            slEventLog->AddTextWithDateTime(SL->CommaText);
        SaveEventLog();
#endif
    }
    SaveEventLogInfo("220000000", SL->CommaText, 22, " ");
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;
}
//------------------------------------------------------------------------------
// 2: 24Hr
// 3: 12Hr (8:00 ~ 20:00)
//------------------------------------------------------------------------------
void __fastcall RecordTimeData(int iDataType)                                   //JerryYang 20180515 (Steven) 記錄Time data  //第三組改成每12Hr計算一次, 第二組為每24Hr計算一次
{
    AnsiString str;
    long Sum=0;
    int MUBA, MTBA;

    if(IniConfig.bVTESTFunction==true)                                          //jou 20220301 : VTEST by time OEE record
        return;

    // AI(GA1-B4) 20260804: iSaveTime0800/iSaveTime2000 moved inside the gated
    // Summary-Report block below (their only consumers) to avoid an
    // unused-variable warning on the active build path.

    for(int i=0; i<20; i++)
    {
        Sum+=LastSet.BinCT[1][i];
        LastSet.BinCT[1][i]=0;
    }

    if(LastSet.iJamCount[2]!=0)
    {
        MTBA=((LastSet.SystemAccSecond[iDataType][stPauseTime]+
               LastSet.SystemAccSecond[iDataType][stProductTime]+
               LastSet.SystemAccSecond[iDataType][stJamTime])/1000)/LastSet.iJamCount[2];
        MUBA=ChangeToFloatNonPcnt((double)(Sum), (double)(LastSet.iJamCount[2]));
    }
    else
    {
        MTBA=((LastSet.SystemAccSecond[iDataType][stPauseTime]+
               LastSet.SystemAccSecond[iDataType][stProductTime]+
               LastSet.SystemAccSecond[iDataType][stJamTime]))/1000;
        MUBA=Sum;
    }

    str.sprintf("%i,%i,%i,%i,%i,%i,%i,%i,%i,%d, %d",
                LastSet.SystemAccSecond[iDataType][stStartTime]/1000,
                LastSet.SystemAccSecond[iDataType][stHomeTime]/1000,
                LastSet.SystemAccSecond[iDataType][stContactTest]/1000,
                LastSet.SystemAccSecond[iDataType][stPauseTime]/1000,
                LastSet.SystemAccSecond[iDataType][stProductTime]/1000,
                LastSet.SystemAccSecond[iDataType][stJamTime]/1000,
                LastSet.SystemAccSecond[iDataType][stPowerOn]/1000,
                Sum,
                LastSet.iJamCount[2],
                MUBA,
                MTBA);

#if 0 // TODO(GA1-B4): golden :393-476 "Sam 20210107 Summary Report fuction"
      // block gated as one unit -- it needs ALL of: vclcompat::TDateTime's
      // absent DayOfWeek() method; asProduct_LoaderPath (no declaration
      // anywhere in this tree); fObserver->DoProduction_Summary_Report /
      // ->strngrdMDBQuery (not in atester_shims.h's TfObserverShim);
      // fConfiguration->edtN04_Host (general-purpose fConfiguration has no
      // port -- only an Automation-module-scoped stub exists, out of layering
      // scope for this file).  (Its own `.FormatString(fmt)` calls WOULD
      // translate cleanly to the active vclcompat::FormatDateTime(fmt,dt) --
      // that part alone is not the blocker.)
    int iSaveTime0800=80000;                                                    //AM 0800
    int iSaveTime2000=200000;                                                   //PM 0800
    int iNowTime;                                                               //Sam 20210107 : Summary Report fuction
    int iDecDay=0,iWeek=0,iDate;
    bool bIsRecordSummaryReport=false;
    AnsiString asYear, asDate, as1DayDate, as1WeekDate, as1MonthDate, asFileTime, asFilePath, asFolderPath, asSaveFilePath, asDay, asWeek, asMonth;
    int iLoaderCountTemp;
    int iProductTimeTemp;

    AnsiString HostName=fConfiguration->edtN04_Host->Text;

    if(iDataType==3 ||                                                          //Sam 20210107 : Summary Report fuction
       CUSTOMER_CODE==CC_TERAPOWER)                                             //Sam 20210108 : 紀錄整天的結果
    {
        iNowTime=StrToInt(GetOnlyTimeInfoByString());
        bIsRecordSummaryReport=false;
        if(iSaveTime0800+20>iNowTime && iNowTime>iSaveTime0800)                 //08:00:20 - 08:00:00
        {
            iDecDay=-1;                                                         //產能日期要記錄在前一天
            asFileTime="2000-0800";
            bIsRecordSummaryReport=true;                                        //08:00:10 - 08:00:00 之間才需要結檔
        }
        else if(iSaveTime2000+20>iNowTime && iNowTime>iSaveTime2000)            //20:00:20 - 20:00:00
        {
            iDecDay=0;
            asFileTime="0800-2000";
        }
        else if(iSaveTime2000>iNowTime && iNowTime>iSaveTime0800)               //08:00:00 - 20:00:00
        {
            iDecDay=0;
            asFileTime="0800-2000";
        }
        else//iNowTime>iSaveTime2000                                            //20:00:00 - 23:59:59
        {
            iDecDay=1;                                                          //產能日期要記錄在第二天
            asFileTime="2000-0800";
        }

        asYear=FormatDateTime("yyyy", Now()+iDecDay);
        asDate=FormatDateTime("yyyy-mm-dd", Now()+iDecDay);

        asFolderPath.sprintf("%s\\%s", asProduct_LoaderPath, asYear);
        MyForceDirectories(asFolderPath);
        asFilePath.sprintf("%s\\%s\\%s-%s.txt", asProduct_LoaderPath, asYear, asDate, asFileTime);

        iLoaderCountTemp=ReadIniData(asFilePath, "Product", "LoaderCount", 0);
        iProductTimeTemp=ReadIniData(asFilePath, "Product", "ProductTime", 0);
        WriteIniData(asFilePath, "Product", "LoaderCount", LastSet.iLoaderCount+iLoaderCountTemp);
        WriteIniData(asFilePath, "Product", "ProductTime", (unsigned long)((LastSet.SystemAccSecond[iDataType][stProductTime]/1000)+iProductTimeTemp));
        LastSet.iLoaderCount=0;

        if(bIsRecordSummaryReport)
        {
            asDay       =FormatDateTime("mmdd", Now()+iDecDay+1);              //20211102 : 建勳要求的檔名
            asWeek      =FormatDateTime("mmdd", Now()+iDecDay-7);              //20211102 : 建勳要求的檔名
            asMonth     =FormatDateTime("mmdd", Now()+iDecDay-30);             //20211102 : 建勳要求的檔名

            as1DayDate  =FormatDateTime("yyyy-mm-dd", Now()+iDecDay+1);
            as1WeekDate =FormatDateTime("yyyy-mm-dd", Now()+iDecDay-7);
            as1MonthDate=FormatDateTime("yyyy-mm-dd", Now()+iDecDay-30);
            iWeek=(Now()+iDecDay).DayOfWeek()-1;
            iDate=atoi(FormatDateTime("dd", Now()+iDecDay).c_str());
            asFolderPath.sprintf("%s\\%s",IniConfig.asO19_SavePath, FormatDateTime("yyyy", Now())); //Sam 20240103 : 修正跨年度問題
            MyForceDirectories(asFolderPath);

            if(IniConfig.bO19_AutoRecordReportByEveryDay)
            {
                NewRecordProcess("", "bO19_AutoRecordReportByEveryDay", "");        //Sam 20240816 : 新增O19 Log紀錄
                fObserver->DoProduction_Summary_Report(asDate,"08:00:00", as1DayDate, "08:00:00");              //Record Today Report
                asSaveFilePath.sprintf("%s\\%s_%s.csv", asFolderPath, HostName, asDay);     //Sam 20231106 : 新增 SGD 轉 CSV
                SGDToCSV(fObserver->strngrdMDBQuery, ",", ";", asSaveFilePath);             //Sam 20231106 : 新增 SGD 轉 CSV
            }

            if(IniConfig.bO19_AutoRecordReportByEveryWeek)
            {
                if(iWeek==IniConfig.iO19_WeekPeriod)
                {
                    NewRecordProcess("", "bO19_AutoRecordReportByEveryWeek", "");       //Sam 20240816 : 新增O19 Log紀錄
                    fObserver->DoProduction_Summary_Report(as1WeekDate, "08:00:00", as1DayDate, "08:00:00");    //Record Week Report
                    asSaveFilePath.sprintf("%s\\%s_%s_%s.csv", asFolderPath, HostName, asWeek, as1DayDate);     //Sam 20231106 : 新增 SGD 轉 CSV
                    SGDToCSV(fObserver->strngrdMDBQuery, ",", ";", asSaveFilePath);         //Sam 20231106 : 新增 SGD 轉 CSV
                }
            }

            if(IniConfig.bO19_AutoRecordReportByEveryMonth)
            {
                if(iDate==IniConfig.iO19_MonthPeriod+1)
                {
                    NewRecordProcess("", "bO19_AutoRecordReportByEveryMonth", "");      //Sam 20240816 : 新增O19 Log紀錄
                    fObserver->DoProduction_Summary_Report(as1MonthDate, "08:00:00", as1DayDate, "08:00:00");   //Record Month Report
                    asSaveFilePath.sprintf("%s\\%s_%s_%s.csv", asFolderPath, HostName, asMonth, as1DayDate);    //Sam 20231106 : 新增 SGD 轉 CSV
                    SGDToCSV(fObserver->strngrdMDBQuery, ",", ";", asSaveFilePath);         //Sam 20231106 : 新增 SGD 轉 CSV
                }
            }
        }
    }
#endif

    for(int i=0; i<8; i++)
    {
        LastSet.SystemAccSecond[2][i]=0;
        LastSet.SystemAccSecond[3][i]=0;
    }
    LastSet.iJamCount[2]=0;

    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    SL->CommaText=str;
    if(fMain!=NULL)
    {
#if 0 // TODO(GA1-B4): fMain->slTimeData not yet ported (forms/fMain.h facade
      // has no such member) -- golden :491-492 text-mirror persist.
        fMain->slTimeData->AddTextWithDateTime(SL->CommaText);
        fMain->slTimeData->MySaveToFile();
#endif
        SaveEventLogInfo("220000000", SL->CommaText, 22, " ");
    }
    SL->Clear();
    delete SL;
}
//---------------------------------------------------------------------------
void __fastcall MyDBIProductionData(AnsiString sAction)
{
    AnsiString str, Str2, Code, sDateTime="", sTime;
#if 0 // TODO(GA1-B4): fContactCT not yet ported (no declaration anywhere in
      // this tree) -- golden :503 site-yield snapshot.
    fContactCT->SaveSiteYield(sAction);
#endif
#if 0 // TODO(GA1-B4): fMain->TimerRecordLoaderDate() not yet ported
      // (forms/fMain.h facade has no such method) -- golden :504.
    int rowID=fMain->TimerRecordLoaderDate();
#else
    int rowID=0;
#endif
    static int iJamCnt=0;                                                       //AI(GA1-B4) 20260804: golden `static iJamCnt=0;` (:505) is BCB6 implicit-int, ill-formed in C++17; made explicit `int` (behaviour-identical)

    AnsiString str1;
    str1=IniConfig.sLotID;                                                      //JerryYang 20151026 for SPIL_CHINA_SUZHOU 資料要有工作檔名
    if(str1=="" && sAction=="TimePeriod")                                       //JerryYang 20151026 for SPIL_CHINA_SUZHOU 資料要有工作檔名
    {
        str1=fMain->cbSetupFileName->Text;
    }

    if(CUSTOMER_CODE==CC_AnalogDevice_Phil || IniConfig.bMaximFunction==true)  //JerryYang 20230828 : analog要求 jam count存完要清除
    {
        if(LastSet.iJamCount[0]>iJamCnt)
        {
            iJamCnt=LastSet.iJamCount[0]-iJamCnt;
        }
        else if(LastSet.iJamCount[0]<iJamCnt)
        {
            iJamCnt=LastSet.iJamCount[0];
        }
        else
        {
            iJamCnt=0;
        }
    }
    else
    {
        iJamCnt=LastSet.iJamCount[0];
    }

    str.sprintf("INSERT INTO 'Production' ('LotID','OccurDateTime','LoadCount','UnLoadCount',"
                "'Auto1Count','Auto2Count','Auto3Count','Fix1Count','Fix2Count','Fix3Count','Fix4Count','Fix5Count','Fix6Count',"
                "'Action','ID_TotalLoader','JamCount','UPH','MTBR','MUBF') "
                "VALUES ('%s', datetime(CURRENT_TIMESTAMP, 'localtime'), %d, %d,"
                "        %d, %d, %d, %d, %d, %d, %d, %d, %d,"
                "        '%s', %d, %d, %s, '%s', '%s')",
                str1, LastSet.SendCT[0], RunInfo.iUnloadCount,                  //JerryYang 20151026 for SPIL_CHINA_SUZHOU 資料要有工作檔名
                LastSet.BinCT[0][0], LastSet.BinCT[0][1], LastSet.BinCT[0][2],
                LastSet.BinCT[0][3], LastSet.BinCT[0][4], LastSet.BinCT[0][5],
                LastSet.BinCT[0][6], LastSet.BinCT[0][7], LastSet.BinCT[0][8],
                sAction, rowID, iJamCnt, RunInfo.iAvgUPH, RunInfo.MTBA, RunInfo.MUBA);

    MyDBExecSQL(str);

    sDateTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);

    str.sprintf("%s, %s, %d, %d,"
                " %d, %d, %d, %d, %d, %d, %d, %d, %d,"
                "%s, %d, %d, %s, %s, %s, %s",
                str1, sDateTime, LastSet.SendCT[0], RunInfo.iUnloadCount,
                LastSet.BinCT[0][0], LastSet.BinCT[0][1], LastSet.BinCT[0][2],
                LastSet.BinCT[0][3], LastSet.BinCT[0][4], LastSet.BinCT[0][5],
                LastSet.BinCT[0][6], LastSet.BinCT[0][7], LastSet.BinCT[0][8],
                sAction, rowID, iJamCnt, RunInfo.iAvgUPH, RunInfo.MTBA, RunInfo.MUBA, IniConfig.SocketHandlerID);

#if 0 // TODO(GA1-B4): fMain->slProdRecordLog not yet ported (forms/fMain.h
      // facade has no such member) -- golden :559-560 text-mirror persist.
    fMain->slProdRecordLog->AddText(str);                                       //JerryYang 20230721 : Analog要求production record
    fMain->slProdRecordLog->MySaveToFile();
#endif

    iJamCnt=LastSet.iJamCount[0];

    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        GetTimeInfo();
        sTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        SL->Add("Production");                                                  //UnitName
        SL->Add("\t");                                                          //AlarmCode
        SL->Add(sTime);                                                         //OccurDateTime
        SL->Add("\t");                                                          //Recovery
        SL->Add("\t");                                                          //StopedTime
        SL->Add("\t");                                                          //Duplicate
        SL->Add(str);                                                           //Message
        SL->Add("\t");                                                          //ErrPart
    }
    else
    {
        SL->Add("Production");
        SL->Add(sAction);
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add(str);
        SL->Add("\t");
    }

    if(slEventLog!=NULL)
    {
#if 0 // TODO(GA1-B4): see MyDBITotalLoader's note above (slEventLog opaque type)
        if(IniConfig.bSPILFunction==true)                                       //Steven 20240604 : SPIL格式的event log
            slEventLog->AddTextWithLineNo(SL->CommaText);
        else
            slEventLog->AddTextWithDateTime(SL->CommaText);
        SaveEventLog();
#endif
    }
    SaveEventLogInfo("220000000", SL->CommaText, 22, " ");
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;
}
//---------------------------------------------------------------------------
int __fastcall MyDBIEvent(AnsiString AlarmCode, int MotorID, int *AlarmID, int *UnitNo, int *AxleNo, int *Type,
                          AnsiString *Message, AnsiString *UnitName, AnsiString asTemperature, int bDuplicateErr, AnsiString errPart, bool bDate, TDateTime date) //Chunghung 2012 0416 add date
{
    int ret=0, iAlarmCode;
    AnsiString str, Str2, Code;
    int rows, cols;
    char **result;
    char *errMsg=NULL;
    int LotID       =0;
    int rowID;                                                                  //AI(GA1-B4) 20260804: golden quirk -- returned WITHOUT being assigned when CosFunction.bUseMDB==false (golden :605-721); preserved verbatim, not zero-initialized here

    if(CosFunction.bUseMDB)                                                     //Steven 20210526 : 部分客戶取消使用MDB
    {
        str.sprintf("SELECT ID_AlarmList, Message, UnitNo, ID_Axle, Type FROM AlarmList WHERE AlarmCode='%s'", AlarmCode);
        sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
        if(rows==1)                                                             //取得AlarmList的資料
        {
            *AlarmID=atoi(result[5]);
            *Message=result[6];
            *UnitNo =atoi(result[7]);
            *AxleNo =atoi(result[8]);
            *Type   =atoi(result[9]);
        }
        else                                                                    //例外處理
        {
#if 0 // TODO(GA1-B4): fMain->AlarmCodeMap not yet ported (forms/fMain.h facade
      // has no such member) -- golden :628-649 in-memory alarm-code fallback
      // lookup.  ACTIVE fallback below always takes golden's own "not found"
      // branch (the map lookup cannot structurally succeed without the map).
            if(fMain->AlarmCodeMap[AlarmCode]!="")                              //Steven 20170202 (wei): Fixed for Unknown Alarm Code
            {
                *AlarmID=atoi(AlarmCode.SubString(4, AlarmCode.Length()).c_str());
                *UnitNo =atoi(AlarmCode.SubString(4, 2).c_str());
                *AxleNo =0;
                if(AlarmCode.AnsiPos("JAM")==1)
                    *Type=1;
                else if(AlarmCode.AnsiPos("WAR")==1)
                    *Type=2;
                else
                    *Type=3;                                                    //Steven 20211028 : 0 --> 3

                *Message=fMain->AlarmCodeMap[AlarmCode];
            }
            else
            {
                *AlarmID=41;
                *UnitNo =0;
                *AxleNo =0;
                *Type   =0;
                *Message="Unknown Alarm Code";
            }
#else
            *AlarmID=41;
            *UnitNo =0;
            *AxleNo =0;
            *Type   =0;
            *Message="Unknown Alarm Code";
#endif
        }
        sqlite3_free(errMsg);
        sqlite3_free_table(result);
        str.sprintf("SELECT UnitName FROM UnitName WHERE ID_UnitNo=%d", *UnitNo);   //取得Unit Name
        sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
        sqlite3_free(errMsg);
        if(rows==1)                                                             //Steven 20210810 : 避免資料庫壞掉搜尋不到
        {
            *UnitName=AnsiString(result[1]);
        }
        else
        {
            if(AlarmCode.AnsiPos("24")==4)                                      //Steven 20210813 : 針對馬達Alarm, UnitName強制寫成Motor
                *UnitName=AnsiString("Motor");
#if 0 // TODO(GA1-B4): fMain->UnitNameMap not yet ported (forms/fMain.h facade
      // has no such member) -- golden :664-665.  ACTIVE fallback below always
      // takes golden's own final else (`*UnitName=*UnitNo;`).
            else if(*UnitNo<fMain->UnitNameMap->Count && *UnitNo>=0)
                *UnitName=fMain->UnitNameMap->Strings[*UnitNo];
#endif
            else
                *UnitName=*UnitNo;
        }
        str.sprintf("INSERT INTO EventLog (ID_AlarmList, ID_MotorList, ID_LotInfo, OccurDateTime, Duplicate, ErrPart, Temperature) VALUES"
                    "(%d, %d, %d, datetime(CURRENT_TIMESTAMP, 'localtime'), %d, '%s', '%s')", *AlarmID, MotorID, LotID, bDuplicateErr, errPart, asTemperature);
        ret=sqlite3_exec(dbReadWrite, str.c_str(), 0, 0, &errMsg);
        if(ret!=SQLITE_OK){sqlite3_free(errMsg);}

        rowID=sqlite3_last_insert_rowid(dbReadWrite);
        sqlite3_free_table(result);
    }
//    else                                                                      //強制值性下面的，才會在有使用MDB時，也有存到txt檔案裡
//    {
#if 0 // TODO(GA1-B4): fMain->AlarmCodeMap not yet ported -- see the first
      // occurrence's note above (golden :679-698, the unconditional second
      // copy of the same lookup -- "強制執行下面的" means this branch runs
      // regardless of bUseMDB).  ACTIVE fallback: golden's own else (Unknown
      // Alarm Code defaults) -- these are ALREADY the current values of
      // *AlarmID/*UnitNo/*AxleNo/*Type/*Message when bUseMDB was true (set
      // above), so this fallback only actually changes anything on the
      // bUseMDB==false path, where it now matches golden's "not found" shape.
        if(fMain->AlarmCodeMap[AlarmCode]!="")                                  //Steven 20170202 (wei): Fixed for Unknown Alarm Code
        {
            *AlarmID=atoi(AlarmCode.SubString(4, AlarmCode.Length()).c_str());
            *UnitNo =atoi(AlarmCode.SubString(4, 2).c_str());
            *AxleNo =0;
            if(AlarmCode.AnsiPos("JAM")==1)
                *Type=1;
            else if(AlarmCode.AnsiPos("WAR")==1)
                *Type=2;
            else
                *Type=3;                                                        //Steven 20211028 : 0 --> 3

            *Message=fMain->AlarmCodeMap[AlarmCode];
            if(AlarmCode.AnsiPos("24")==4)                                      //Steven 20210813 : 針對馬達Alarm, UnitName強制寫成Motor
                *UnitName=AnsiString("Motor");
            else if(*UnitNo<fMain->UnitNameMap->Count && *UnitNo>=0)
                *UnitName=fMain->UnitNameMap->Strings[*UnitNo];
            else
                *UnitName=*UnitNo;
        }
        else
        {
            *AlarmID=41;
            *UnitNo =0;
            *AxleNo =0;
            *Type   =0;
            *Message="Unknown Alarm Code";
        }
#else
    if(!CosFunction.bUseMDB)
    {
        *AlarmID=41;
        *UnitNo =0;
        *AxleNo =0;
        *Type   =0;
        *Message="Unknown Alarm Code";
    }
#endif
//    }

    // AI(GA1-B4) 20260804: golden `DateSeparator='-';` (:709) sets a BCB6
    // locale-global that only affects later locale-sensitive date PARSING;
    // vclcompat::FormatDateTime below always uses fixed token separators, so
    // this line has no observable effect in the ported build. Dropped (inert).
    if(bDate)
        MyDBULotEndTime(FormatDateTime("yyyy-mm-dd hh:nn:ss", TDateTime(date.Val()+0.00001)));    //比NOW()加一秒  //AI(GA1-B4) 20260804: TDateTime(double) explicit ctor call disambiguates operator+ (TDateTime<->double implicit conversions both ways make `date+0.00001` ambiguous under vclcompat)
    else
        MyDBULotEndTime(FormatDateTime("yyyy-mm-dd hh:nn:ss", TDateTime(Now().Val()+0.00001)));   //比NOW()加一秒

    iAlarmCode=atoi(AlarmCode.SubString(4, AlarmCode.Length()).c_str());
    Code.sprintf("%d%02d%06d", *Type, *UnitNo, iAlarmCode);
    if(AlarmCode.AnsiPos("JAM")!=0)
        SaveEventLogInfo(Code, *Message, 0, " ", errPart);
    else
        SaveEventLogInfo(Code, *Message, 20, " ", errPart);
    return rowID;
}
//---------------------------------------------------------------------------
// AI(GA1-B4) 20260804: MyDBIProcessNew -- HOMECOMING GATED.  Golden body
// preserved verbatim below.  A real, reduced (record + stdout log, no DB
// write) stand-in for this EXACT external symbol already lives in
// canary_support.cpp:453 (declared canary_support.h:396), landed by the
// independent W7-L2 wave on 20260803 -- defining it again here would be a
// duplicate-definition link error.  Un-gating requires the integrator swap
// (see the GA-1-B4 report for the exact steps + nm verification command).
//---------------------------------------------------------------------------
#if 0 // TODO(GA1-B4-integrate): homecoming from canary_support.cpp -- integrator swaps
// AI(W906-FCAudit) 20260818: __fastcall dropped in lockstep with cMyDB.h:82
// (see the note there) -- gated body must not resurrect the mangling mismatch
// against canary_support.cpp:457 when this homecoming gate dissolves.
void MyDBIProcessNew(AnsiString asTable, AnsiString AlarmCode, AnsiString S1, AnsiString S2) //Steven 20161220 : Process加上Alarm Code
{
    int iAlarmCode, iUnitNo;
    AnsiString str, Str2, Code, sTime;

    AnsiString SData="@e02002"+S1+S2;
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)
    {
        if(S1.Pos("ATC"))                                                       //kevin 20210927 ATC log
        {
        }
        else
        {
            RespondASECom(SData);                                               //kevin 20150610 回應 ase Reset finish
        }
    }
    str.sprintf("INSERT INTO %s (%s, Debug, OccurDateTime, AlarmCode) "
                "VALUES('%s', '%s', datetime(CURRENT_TIMESTAMP, 'localtime'), '%s')", asTable, asTable, S1, S2, AlarmCode);
    MyDBExecSQL(str);
    ProductionLog(S1+S2, true);                                                //JerryYang 20160301 for 矽品蘇州 Process Record也要存成文字檔

    TStringList *SL;                                                            //Steven 20161115 : EventLog存成文字檔
    SL=new TStringList();
    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        GetTimeInfo();
        sTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        SL->Add(asTable);                                                       //UnitName
        SL->Add(AlarmCode);                                                     //AlarmCode
        SL->Add(sTime);                                                         //OccurDateTime
        SL->Add("\t");                                                          //Recovery
        SL->Add("\t");                                                          //StopedTime
        SL->Add("\t");                                                          //Duplicate
        SL->Add(S1);                                                            //Message
        SL->Add(S2);                                                            //ErrPart
    }
    else
    {
        SL->Add(asTable);
        SL->Add(AlarmCode);
        SL->Add("\t");
        SL->Add("\t");
        SL->Add("\t");
        SL->Add(S1);
        SL->Add(S2);
    }

    if(slEventLog!=NULL)
    {
        if(IniConfig.bSPILFunction==true)                                       //Steven 20240604 : SPIL格式的event log
            slEventLog->AddTextWithLineNo(SL->CommaText);
        else
            slEventLog->AddTextWithDateTime(SL->CommaText);
        SaveEventLog();
    }
    SL->Clear();                                                                //Ifor 20170603 (wei) TStringList 刪除前先 Clean
    delete SL;
    SL=NULL;                                                                    //kevin 20171213

    iAlarmCode=atoi(AlarmCode.SubString(4, AlarmCode.Length()).c_str());
    iUnitNo=atoi(AlarmCode.SubString(4, 2).c_str());
    Code.sprintf("2%02d%06d", iUnitNo, iAlarmCode);
    SaveEventLogInfo(Code, S1, 22, " ", S2);
}
#endif // MyDBIProcessNew homecoming gate
//---------------------------------------------------------------------------
// AI(GA1-B4) 20260804: MyDBIProcess (3-arg) -- HOMECOMING GATED per task
// brief.  Golden body preserved verbatim below.  The production definition
// currently resides in SECSGEM/uHGemEquipment.cpp:3475 (FastcallFix wave,
// 20260720) -- see cMyDB.h's HOMECOMING NOTICE and the GA-1-B4 report for the
// integrator swap steps.
//---------------------------------------------------------------------------
#if 0 // TODO(GA1-B4-integrate): homecoming from uHGemEquipment.cpp -- integrator swaps
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2)
{
    AnsiString str, Str2, Code, sTime;
    try
    {
        AnsiString SData="@e02002"+S1+S2;
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)
        {
            if(S1.Pos("ATC"))                                                   //kevin 20210927
            {
            }
            else
            {
                RespondASECom(SData);                                           //kevin 20150610 回應 ase Reset finish
            }
        }
        str.sprintf("INSERT INTO %s (%s, Debug, OccurDateTime) "
                    "VALUES('%s', '%s', datetime(CURRENT_TIMESTAMP, 'localtime'))", asTable, asTable, S1, S2);
        MyDBExecSQL(str);
        ProductionLog(S1+S2, true);                                             //JerryYang 20160301 for 矽品蘇州 Process Record也要存成文字檔

        TStringList *SL;                                                        //Steven 20161115 : EventLog存成文字檔
        SL=new TStringList();
        if(IniConfig.bSPILFunction==true)                                       //Steven 20240604 : SPIL格式的event log
        {
            GetTimeInfo();
            sTime.sprintf("%04d-%02d-%02d %02d:%02d:%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
            SL->Add(asTable);                                                   //UnitName
            SL->Add("\t");                                                      //AlarmCode
            SL->Add(sTime);                                                     //OccurDateTime
            SL->Add("\t");                                                      //Recovery
            SL->Add("\t");                                                      //StopedTime
            SL->Add("\t");                                                      //Duplicate
            SL->Add(S1);                                                        //Message
            SL->Add(S2);                                                        //ErrPart
        }
        else
        {
            SL->Add(asTable);
            SL->Add("\t");
            SL->Add("\t");
            SL->Add("\t");
            SL->Add("\t");
            SL->Add(S1);
            SL->Add(S2);
        }

        if(slEventLog!=NULL)
        {
            if(IniConfig.bSPILFunction==true)                                   //Steven 20240604 : SPIL格式的event log
                slEventLog->AddTextWithLineNo(SL->CommaText);
            else
                slEventLog->AddTextWithDateTime(SL->CommaText);
            SaveEventLog();
        }
        SL->Clear();                                                            //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete SL;
        SL =NULL;                                                               //kevin 20171213

        Code.sprintf("220000000");
        SaveEventLogInfo(Code, S1, 22, " ");
    }
    catch(...)
    {
        str="!!";
    }
}
#endif // MyDBIProcess homecoming gate
//---------------------------------------------------------------------------
// 更新表單資料  (update table data)
//---------------------------------------------------------------------------
void __fastcall MyDBULotEndTime(AnsiString EndTime)
{
    int ret=0;

    int rows, cols;
    char **result;
    char *errMsg=NULL;
    AnsiString str;

    if(CosFunction.bUseMDB==false)                                              //Steven 20210526 : 部分客戶取消使用MDB
        return;

    str.sprintf("SELECT ID_LotInfo FROM LotInfo ORDER BY ID_LotInfo DESC LIMIT 1");
    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
    sqlite3_free(errMsg);
    if(rows==1)
    {
        str.sprintf("UPDATE LotInfo SET EndTime='%s' WHERE ID_LotInfo=%s", EndTime, result[1]);
        ret=sqlite3_exec(dbReadWrite, str.c_str(), 0, 0, &errMsg);
        if(ret!=SQLITE_OK){sqlite3_free(errMsg);}
    }
    sqlite3_free_table(result);
}
//---------------------------------------------------------------------------
void __fastcall MyDBULotData(AnsiString TableName, TStringGrid *strGrid)
{
    AnsiString id, name, str;
    id.sprintf("ID_%s", TableName);
    name.sprintf("%sName", TableName);
    for(int i=2; i<strGrid->RowCount; i++)
    {
        str.sprintf("UPDATE %s SET %s='%s' WHERE %s=%d",
                      TableName,
                      name, strGrid->Cells[1][i],
                      id,   atoi(strGrid->Cells[0][i].c_str()));
        MyDBExecSQL(str);
    }
}
//---------------------------------------------------------------------------
void __fastcall MyDBUEventRecover(int RowID, AnsiString Recovery, int StopTime)
{
    AnsiString str;
    str.sprintf("UPDATE EventLog SET StopedTime=%d, Recovery='%s' WHERE ID_EventLog=%d", StopTime, Recovery, RowID);
    MyDBExecSQL(str);
    MyDBULotEndTime(FormatDateTime("yyyy-mm-dd hh:nn:ss", TDateTime(Now().Val()+0.00001)));       //比NOW()加一秒  //AI(GA1-B4) 20260804: see MyDBIEvent's note on this disambiguation
}
//---------------------------------------------------------------------------
// 資料庫查詢  (database query)
//---------------------------------------------------------------------------
void __fastcall MyDBQLotData(AnsiString Query, TStringList *strList)
{
    int rows, cols;
    char *errMsg=NULL;
    char **result;

    sqlite3_get_table(dbReadOnly , Query.c_str(), &result , &rows, &cols, &errMsg);
    for(int i=2; i<=rows; i++)                                                  // 列出所有資料  //AI(GA1-B4) 20260804: golden quirk (verified against real Handler.DB3 data via tests/test_ga1_cmydb.cpp) -- `i` starts at 2, so row index 1 (the FIRST data row, sqlite3_get_table's result[] index 0 being the header row) is silently skipped; only rows 2..rows are ever added. Preserved verbatim -- no other golden caller of this function was in scope to confirm intent.
        strList->Add(result[i*cols]);
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
}
//---------------------------------------------------------------------------
AnsiString __fastcall MyDBQMotMess(int MotorID)
{
    char *errMsg=NULL;
    int rows, cols;
    char **result;
    AnsiString str="";

    AnsiString EventName=AnsiString(SystemYear)+AnsiString(SystemMonth);         //AI(GA1-B4) 20260804: golden quirk -- computed, never used again in this function (:928); preserved verbatim
    str.sprintf("SELECT MotMess FROM MotorAlarmList WHERE ID_MotorList=%d", MotorID);
    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
    if(rows==1) str=result[1];
    else        str="";
    sqlite3_free(errMsg);
    sqlite3_free_table(result);

    return str;
}
//---------------------------------------------------------------------------
AnsiString __fastcall GetMyDBIMessage(AnsiString AlarmCode)                     //Sam 20230218 : 新增用 AlarmCode 去找 Alarm Message 資料
{
    char *errMsg=NULL;
    int rows, cols;
    char **result;
    AnsiString str="";
    AnsiString EventName=AnsiString(SystemYear)+AnsiString(SystemMonth);         //AI(GA1-B4) 20260804: golden quirk -- computed, never used again in this function (:945); preserved verbatim
    str.sprintf("SELECT ID_AlarmList, Message, UnitNo, ID_Axle, Type FROM AlarmList WHERE AlarmCode='%s'", AlarmCode);
    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
    if(rows==1)
    {
        str=result[6];
    }
    else
    {
        str="";
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    return str;
}
//---------------------------------------------------------------------------
AnsiString __fastcall MyDBQClearDT()
{
    char *errMsg=NULL;
    int rows, cols;
    char **result;
    AnsiString str;
    str.sprintf("SELECT ClearDateTime FROM ClearDateTime");
    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);

    if(rows==0 && cols==0)                                                      //jou 2014-10-22 修正 database clear date error
        str="NULL";
    else
        str=result[rows*cols];
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    return str;
}
//---------------------------------------------------------------------------
AnsiString __fastcall MyDBQTotalLoader(AnsiString StartDateTime, AnsiString EndDateTime)
{
    char *errMsg=NULL;
    int rows, cols, iStartCount, iEndCount, iResult;
    char **result;
    AnsiString str;
    str.sprintf(
        "SELECT LoadCount, OccurDateTime "
        "FROM TotalLoader "
        "WHERE                     "
        "    OccurDateTime <= '%s'  "
        "ORDER BY                  "
        "      OccurDateTime DESC  ",
        StartDateTime);

    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
    if(rows>=1) iStartCount=atoi(result[2]);
    else        iStartCount=0;
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    str.sprintf(
        "SELECT LoadCount, OccurDateTime "
        "FROM TotalLoader "
        "WHERE                     "
        "    OccurDateTime <= '%s'  "
        "ORDER BY                  "
        "      OccurDateTime DESC  ",
        EndDateTime);

    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
    if(rows>=1) iEndCount=atoi(result[2]);
    else        iEndCount=iStartCount;

    iResult=iEndCount-iStartCount;
    sqlite3_free(errMsg);
    sqlite3_free_table(result);

    if(iResult>=0)
        return AnsiString(iResult);
    else
        return AnsiString(iEndCount);
}
//---------------------------------------------------------------------------
bool __fastcall MyDBQAlarmCodeList()                                            //Steven 20170202 (wei): Fixed for Unknown Alarm Code
{
    bool bResult=true;
    char *errMsg=NULL;
    int rows, cols;
    char **result;
    AnsiString str;

    str.sprintf("SELECT AlarmCode, Message FROM AlarmList");
    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);

    if(rows==0 && cols==0)
    {
        bResult=false;
    }
    else
    {
        for(int i=1; i<=rows; i++)                                              // 列出所有資料
        {
            str.sprintf("%s=%s", result[i*cols], result[i*cols+1]);
#if 0 // TODO(GA1-B4): fMain->AlarmCodeList not yet ported (forms/fMain.h
      // facade has no such member) -- golden :1043 text-mirror populate.
            fMain->AlarmCodeList->Add(str);
#endif
        }
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    return bResult;
}
//---------------------------------------------------------------------------
void __fastcall MyDBQTimeData(AnsiString StartDateTime, AnsiString EndDateTime, int sgRow, TStringGrid *strGrid)    //Steven 20120628 : 顯示機台稼動時間
{
    char *errMsg=NULL;
    int rows, cols;
    char **result;
    int time;
    AnsiString str;

    str.sprintf(
        "SELECT OccurDateTime, StartTime, HomeTime, ContactTest, PauseTime, ProductTime, JamTime, PowerOn "
        "FROM TimeData "
        "WHERE                     "
        "    OccurDateTime <= '%s'  "
        "ORDER BY                  "
        "      OccurDateTime DESC  ",
        StartDateTime);

    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);

    strGrid->RowCount = strGrid->RowCount + 10;                                 //AI(GA1-B4) 20260804: RowCountProxy has no operator+= (vclcompat/StringGrid.h); rewritten via its int conversion + assignment operator, behaviour-identical
    strGrid->Cells[1][sgRow+1]="Record Time";
    strGrid->Cells[1][sgRow+2]="StartTime";
    strGrid->Cells[1][sgRow+3]="HomeTime";
    strGrid->Cells[1][sgRow+4]="ContactTest";
    strGrid->Cells[1][sgRow+5]="PauseTime";
    strGrid->Cells[1][sgRow+6]="ProductTime";
    strGrid->Cells[1][sgRow+7]="JamTime";
    strGrid->Cells[1][sgRow+8]="PowerOn";
    strGrid->Cells[2][sgRow]="Start Count";
    if(rows==0)
    {
        strGrid->Cells[2][sgRow+1]="No Time Record!!";
    }
    else
    {
        strGrid->Cells[2][sgRow+1]=AnsiString(result[cols+0]);
        strGrid->Cells[2][sgRow+2]=AnsiString(result[cols+1]);
        strGrid->Cells[2][sgRow+3]=AnsiString(result[cols+2]);
        strGrid->Cells[2][sgRow+4]=AnsiString(result[cols+3]);
        strGrid->Cells[2][sgRow+5]=AnsiString(result[cols+4]);
        strGrid->Cells[2][sgRow+6]=AnsiString(result[cols+5]);
        strGrid->Cells[2][sgRow+7]=AnsiString(result[cols+6]);
        strGrid->Cells[2][sgRow+8]=AnsiString(result[cols+7]);
    }

    str.sprintf(
        "SELECT OccurDateTime, StartTime, HomeTime, ContactTest, PauseTime, ProductTime, JamTime, PowerOn "
        "FROM TimeData "
        "WHERE                     "
        "    OccurDateTime <= '%s'  "
        "ORDER BY                  "
        "      OccurDateTime DESC  ",
        EndDateTime);
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);

    strGrid->Cells[3][sgRow]="End Count";
    if(rows==0)
    {
        strGrid->Cells[3][sgRow+1]="No Time Record!!";
    }
    else
    {
        strGrid->Cells[3][sgRow+1]=AnsiString(result[cols+0]);
        strGrid->Cells[3][sgRow+2]=AnsiString(result[cols+1]);
        strGrid->Cells[3][sgRow+3]=AnsiString(result[cols+2]);
        strGrid->Cells[3][sgRow+4]=AnsiString(result[cols+3]);
        strGrid->Cells[3][sgRow+5]=AnsiString(result[cols+4]);
        strGrid->Cells[3][sgRow+6]=AnsiString(result[cols+5]);
        strGrid->Cells[3][sgRow+7]=AnsiString(result[cols+6]);
        strGrid->Cells[3][sgRow+8]=AnsiString(result[cols+7]);
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);

    strGrid->Cells[4][sgRow]="Summary";
    for(int i=2; i<9; i++)
    {
        time=(atoi(strGrid->Cells[3][sgRow+i].c_str())-atoi(strGrid->Cells[2][sgRow+i].c_str()));   //先相減算出區間數量
        if(time>=0)
        {
            if(i==5)
            {
#if 0 // TODO(GA1-B4): fObserver->iPauseTime not yet ported (atester_shims.h's
      // TfObserverShim has no such member) -- golden :1136 MTBF bookkeeping.
                fObserver->iPauseTime=time;                                     //Steven 20170309 (wei) add MTBF(PauseTime/Jam)
#endif
            }
            else if(i==6)
            {
#if 0 // TODO(GA1-B4): fObserver->iProductTime not yet ported -- golden :1140 MTBF bookkeeping.
                fObserver->iProductTime=time;                                   //Isaac 20180417 (Steven) 修正MTBF公式(pause+production+jam)/jamcount
#endif
            }
            else if(i==7)
            {
#if 0 // TODO(GA1-B4): fObserver->iJamTime not yet ported -- golden :1144 MTBF bookkeeping.
                fObserver->iJamTime=time;                                       //Isaac 20180417 (Steven) 修正MTBF公式(pause+production+jam)/jamcount
#endif
            }
            str=ConvertSecondToTime(time);
        }
        else
        {
            if(i==5)
            {
#if 0 // TODO(GA1-B4): fObserver->iPauseTime not yet ported -- see above.
                fObserver->iPauseTime=atoi(strGrid->Cells[3][sgRow+i].c_str());     //Steven 20170309 (wei) add MTBF(PauseTime/Jam)
#endif
            }
            else if(i==6)
            {
#if 0 // TODO(GA1-B4): fObserver->iProductTime not yet ported -- see above.
                fObserver->iProductTime=atoi(strGrid->Cells[3][sgRow+i].c_str());   //Isaac 20180417 (Steven) 修正MTBF公式(pause+production+jam)/jamcount
#endif
            }
            else if(i==7)
            {
#if 0 // TODO(GA1-B4): fObserver->iJamTime not yet ported -- see above.
                fObserver->iJamTime=atoi(strGrid->Cells[3][sgRow+i].c_str());       //Isaac 20180417 (Steven) 修正MTBF公式(pause+production+jam)/jamcount
#endif
            }
            str=ConvertSecondToTime(atoi(strGrid->Cells[3][sgRow+i].c_str()));
        }
        strGrid->Cells[4][sgRow+i]=str;

        time=atoi(strGrid->Cells[2][sgRow+i].c_str());                          //轉換秒數成年月日
        if(time>=0)
            str=ConvertSecondToTime(time);
        else
            str=ConvertSecondToTime(0);
        strGrid->Cells[2][sgRow+i]=str;

        time=atoi(strGrid->Cells[3][sgRow+i].c_str());
        if(time>=0)
            str=ConvertSecondToTime(time);
        else
            str=ConvertSecondToTime(0);
        strGrid->Cells[3][sgRow+i]=str;
    }
}
//---------------------------------------------------------------------------
// 壓縮資料庫  (compact database)
//---------------------------------------------------------------------------
void __fastcall MyDBVACUUM()
{
    if(CosFunction.bUseMDB)                                                     //Steven 20210526 : 部分客戶取消使用MDB
        MyDBExecSQL("VACUUM");
}
//---------------------------------------------------------------------------
// AI(GA1-B4) 20260804: MyDBVUnitEventCount / MyDBVAxleEventCount -- WHOLE-
// FUNCTION GATED.  Both take `TChart *Chart` (golden <Chart.hpp>, the VCL
// TeeChart component) and call ->Series[0]->AddY(...); TChart has no port
// anywhere in this tree (W7-UI scope -- see cMyDB.h's forward decl note).
// Golden bodies preserved verbatim below; ACTIVE stub returns 0 (same
// established idiom as e.g. ainarm9045_2x8_32.cpp's gated-whole-body /
// active-stub pattern).
//---------------------------------------------------------------------------
int __fastcall MyDBVUnitEventCount(AnsiString asQuery, TChart *Chart)
{
#if 0 // TODO(GA1-B4): TChart not ported -- see note above.
    int rows, cols, total=0;
    char *errMsg=NULL;
    char **result;
    sqlite3_get_table(dbReadOnly , asQuery.c_str(), &result , &rows, &cols, &errMsg);
    if(rows!=0)
    {
        for(int i=1; i<=rows; i++)
        {
            Chart->Series[0]->AddY(atoi(result[i*cols+2]), result[i*cols+1], clBlue);
            total+=atoi(result[i*cols+2]);
        }
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    return total;
#else
    (void)asQuery; (void)Chart;
    return 0;
#endif
}
//---------------------------------------------------------------------------
int __fastcall MyDBVAxleEventCount(int min, int max, AnsiString StartTime, AnsiString EndTime, TChart *Chart)
{
#if 0 // TODO(GA1-B4): TChart not ported -- see note above.
    int rows, cols, rows2, cols2, i, total=0;
    char *errMsg=NULL;
    char **result, **result2;
    AnsiString str, str2="%JAM%";
    Chart->Series[0]->Clear();

    str.sprintf("SELECT AxleName FROM AxleName WHERE (ID_AxleNo>=%d) AND (ID_AxleNo<%d)", min, max);
    sqlite3_get_table(dbReadOnly , str.c_str(), &result2, &rows2, &cols2, &errMsg);
    sqlite3_free(errMsg);
    for(i=1; i<=rows2; i++)
    {
        str.sprintf("SELECT                                                                         \n"
                    "      AxleName.ID_AxleNo          AS AxleNo,                                   \n"
                    "      AxleName.AxleName,                                                       \n"
                    "      Count(EventLog.ID_EventLog) AS Count                                     \n"
                    "  FROM                                                                         \n"
                    "      EventLog                                                                 \n"
                    "      INNER JOIN AlarmList  ON (EventLog.ID_AlarmList=AlarmList.ID_AlarmList)  \n"
                    "      INNER JOIN AxleName   ON (AlarmList.ID_Axle    =AxleName.ID_AxleNo)      \n"
                    "  WHERE                                                                        \n"
                    "      (AxleName.AxleName='%s'        AND                                       \n"
                    "      AlarmList.AlarmCode LIKE '%s'  AND                                       \n"
                    "      EventLog.Duplicate=0           AND                                       \n"
                    "      EventLog.OccurDateTime >= '%s' AND                                       \n"
                    "      EventLog.OccurDateTime <= '%s')                                          \n"
                    "  GROUP BY                                                                     \n"
                    "      AxleName.AxleName                                                        \n"
                    "  ORDER BY                                                                     \n"
                    "      AxleName.ID_AxleNo Asc                                                   ", result2[i], str2, StartTime, EndTime);

        sqlite3_get_table(dbReadOnly , str.c_str(), &result , &rows, &cols, &errMsg);
        if(rows>0)
        {
            Chart->Series[0]->AddY(atoi(result[5]), result2[i], clBlue);
            total+=atoi(result[5]);
        }
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result2);
    sqlite3_free_table(result);
    return total;
#else
    (void)min; (void)max; (void)StartTime; (void)EndTime; (void)Chart;
    return 0;
#endif
}
//---------------------------------------------------------------------------
int __fastcall MyDBVEventFreq(AnsiString asQuery, TStringGrid *strGrid)         //Steven 20091222 更新
{
    AnsiString str, tmp="";
    int rows, cols, Count=0;
    int d, h, m, s, tab;
    char *errMsg=NULL;
    char **result;
    sqlite3_get_table(dbReadOnly , asQuery.c_str(), &result , &rows, &cols, &errMsg);
    tab=(cols==7)?1:0;                                                          //Steven 20110715 : byDate的話,要多跳一欄

    if(rows==0)
    {
        strGrid->RowCount=2;
        for(int i=0; i<strGrid->ColCount; i++)
            strGrid->Cells[i][1]="";
        strGrid->Cells[1][1]="No Record!!";
    }
    else
    {
        strGrid->RowCount=rows+1;
        strGrid->Cells[5+tab][0]="Rate";
        for(int i=0; i<=rows; i++)                                              // 列出所有資料
        {
            if(i!=0)
                strGrid->Cells[0][i]=i;

            strGrid->Cells[1][i]=AnsiString(result[i*cols+0]);
            strGrid->Cells[2][i]=AnsiString(result[i*cols+1]);
            strGrid->Cells[3][i]=AnsiString(result[i*cols+2]);
            strGrid->Cells[4][i]=AnsiString(result[i*cols+3]);
            if(tab==1)
            {
                strGrid->Cells[5][i]=AnsiString(result[i*cols+4]);
            }

            if(i!=0) Count+=atoi(result[i*cols+3+tab]);
            if(i==0)
            {
                strGrid->Cells[6+tab][i]=result[i*cols+4+tab];
                strGrid->Cells[7+tab][i]=result[i*cols+5+tab];
            }
            else
            {
                //StopTime
                double f=atof(result[i*cols+4+tab]);
                //iStopTime+=f;
                tmp="";
                h=int(f)/3600;
                m=int(f)/60-h*60;
                s=int(f)%60;
                d=h/24;
                h=h%24;
                if(     d==0)tmp.sprintf("%02d:%02d:%02d", h, m, s);
                else if(d==1)tmp.sprintf("%d day %02d:%02d:%02d", d, h, m, s);
                else         tmp.sprintf("%d days %02d:%02d:%02d", d, h, m, s);
                strGrid->Cells[6+tab][i]=tmp;
                //MTTR
                f=atof(result[i*cols+5+tab]);
                tmp="";
                h=int(f)/3600;
                m=int(f)/60-h*60;
                s=int(f)%60;
                tmp.sprintf("%02d:%02d:%02d", h, m, s);
                strGrid->Cells[7+tab][i]=tmp;
            }
        }

        for(int i=1; i<=rows; i++)                                              //計算出平均發生次數
        {
            if(Count==0)                                                        //Steven 20110418
            {
                strGrid->Cells[5+tab][i]="0.0%";
            }
            else
            {
                strGrid->Cells[5+tab][i]=ChangeToPercentage(atof(result[i*cols+3+tab]), (double)Count);
            }
        }
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    return Count;
}
//---------------------------------------------------------------------------
int __fastcall MyDBVProcess(AnsiString asQuery, TStringGrid *strGrid)
{
    int rows, cols, icount=0;                                                   //JerryYang 20180518 : add
    char *errMsg=NULL;
    char **result;
    sqlite3_get_table(dbReadOnly , asQuery.c_str(), &result , &rows, &cols, &errMsg);
    if(rows>=1)
    {
        strGrid->RowCount=rows+1;
        for(int i=0; i<=rows; i++)                                              // 列出所有資料
        {
            if(i!=0)
                strGrid->Cells[0][i]=i;

            for(int j=0; j<cols; j++)
            {
                if(j==5)
                {
                    strGrid->Cells[j+1][i]=result[i*cols+j];
                    if(strGrid->Cells[j+1][i]=='1')
                    {
                        strGrid->Cells[j+1][i]="Again";
                    }
                    else if(strGrid->Cells[j+1][i]=='2')
                    {
                        strGrid->Cells[j+1][i]="Tester Off Line";
                    }
                    else if(strGrid->Cells[j+1][i]=='3')
                    {
                        strGrid->Cells[j+1][i]="Contact Test";
                    }
                    else
                    {
#if 0 // TODO(GA1-B4): fSecurity not yet ported (no declaration anywhere in
      // this tree) -- golden :1373-1381 custom MTBA-inclusion jam-code filter.
                        if(CosFunction.bIncludeMTBA && i!=0)                    //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
                        {                                                       //Sam 20181031 : IncludeMTBA Bug Fix
                            AnsiString sJamArea=fSecurity->GetJamArea(strGrid->Cells[1][i]);
                            AnsiString sJamCode=fSecurity->GetJamArea(strGrid->Cells[2][i]);
                            if(fSecurity->GetJemIncludeMTBA(sJamArea, sJamCode))
                            {
                                icount++;
                            }
                        }
#endif
                    }
                }
                else
                {
                    strGrid->Cells[j+1][i]=result[i*cols+j];
                }
            }
        }
        strGrid->Cells[0][0]="No.";
    }
    else
    {
        strGrid->RowCount=2;
        for(int i=0; i<strGrid->ColCount; i++)
            strGrid->Cells[i][1]="";
        strGrid->Cells[1][1]="No Record!!";
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    if(CosFunction.bIncludeMTBA)                                                //JerryYang 20180619 (wei) : 新增可自定義Jam code是否列入MTBA計算
    {
        return icount;
    }
    else
    {
        return rows;
    }
}
//---------------------------------------------------------------------------
void  __fastcall GetAlarmCodeList(TStringGrid *strGrid)                         //Steven 20200331 : Alarm code list改用文字檔
{
    AnsiString Code,Mess,Desc;
    AnsiString asQuery;

    // AI(W906-VclGrid-1) 20260820: gate RETIRED -- its premise ("the base has
    // no ColWidths[]") died when the grid consolidation moved the auto-grow
    // ColWidths proxy into vclcompat::TStringGrid itself. Golden :1416-1422
    // live again; still pure column-width cosmetics with no data effect.
    strGrid->ColWidths[0]=50;
    strGrid->ColWidths[1]=50;                                                   //ID_AlarmList
    strGrid->ColWidths[2]=80;                                                   //Alarm Code
    strGrid->ColWidths[3]=80;                                                   //Unit No
    strGrid->ColWidths[4]=80;                                                   //Unit Name
    strGrid->ColWidths[5]=40;                                                   //Type
    strGrid->ColWidths[6]=500;                                                  //Message

    for(int i=0; i<strGrid->RowCount; i++)
    {
        for(int j=0; j<strGrid->ColCount; j++)
        {
            strGrid->Cells[j][i]="";
        }
    }

    strGrid->Cells[0][0]="No.";
    strGrid->Cells[1][0]="ID_AlarmList";                                        //ID_AlarmList
    strGrid->Cells[2][0]="AlarmCode";
    strGrid->Cells[3][0]="UnitNo";
    strGrid->Cells[4][0]="UnitName";
    strGrid->Cells[5][0]="Type";
    strGrid->Cells[6][0]="Message";

    strGrid->Cells[0][1]="1";
    strGrid->Cells[1][1]="41";                                                  //ID_AlarmList
    strGrid->Cells[2][1]="41";
    strGrid->Cells[3][1]="0";
    strGrid->Cells[4][1]="Event";
    strGrid->Cells[5][1]="0";
    strGrid->Cells[6][1]="Unknown Alarm Code";

    AnsiString FileName="D:\\HT9045\\Error\\AlarmCodeList.txt";
    AnsiString Str, Axis, Type;
    int _iAxis, _iType, iNo, iPos;

    TStringList *List=new TStringList();
    if(FileExists(FileName))
    {
        List->LoadFromFile(FileName);
        strGrid->RowCount=List->Count+1;
        for(int i=1; i<List->Count; i++)
        {
            iNo=i+1;
            Str=List->Strings[i];
            iPos=Str.AnsiPos("=");
            Type=Str.SubString(1, 3);
            Axis=Str.SubString(4, 2);
            _iAxis=atoi(Axis.c_str());

            strGrid->Cells[0][iNo]=iNo;
            strGrid->Cells[1][iNo]=Str.SubString(4, iPos-4);
            strGrid->Cells[2][iNo]=Str.SubString(1, iPos-1);
            strGrid->Cells[3][iNo]=_iAxis;                                      //UnitNo
            strGrid->Cells[4][iNo]=AlarmUnit[_iAxis];                           //UnitName //Steven 20231127 : 整理Alarm Unit
            if(Type=="MES")
                _iType=3;
            else if(Type=="JAM")
                _iType=1;
            else if(Type=="WAR")
                _iType=2;
            else
                _iType=0;
            strGrid->Cells[5][iNo]=_iType;
            strGrid->Cells[6][iNo]=Str.SubString(iPos+1, Str.Length());
        }
    }
    List->Clear();
    delete List;
}
//---------------------------------------------------------------------------
int __fastcall MyDBVProcessFilter(AnsiString asQuery, TStringGrid *strGrid)     //Steven 20120222 : 過濾掉Duplicate的訊息
{
    int rows, cols;
    char *errMsg=NULL;
    char **result;
    sqlite3_get_table(dbReadOnly , asQuery.c_str(), &result , &rows, &cols, &errMsg);
    int count=0;
    if(rows>=1)
    {
        strGrid->RowCount=rows+1;
        for(int i=0; i<=rows; i++)                                              // 列出所有資料
        {
            strGrid->Cells[6][count]=result[i*cols+5];                          //先把Duplicate判斷一下
            if(strGrid->Cells[6][count]!='1' && strGrid->Cells[6][count]!='2' && strGrid->Cells[6][count]!='3')
            {
                strGrid->Cells[0][count]=count;
                for(int j=0; j<cols; j++)
                {
                    if(j==5)
                    {
                        if(strGrid->Cells[j+1][count]=='1')
                        {
                            strGrid->Cells[j+1][count]="Again";
                        }
                        else if(strGrid->Cells[j+1][count]=='2')
                        {
                            strGrid->Cells[j+1][count]="Tester Off Line";
                        }
                        else if(strGrid->Cells[j+1][count]=='3')
                        {
                            strGrid->Cells[j+1][count]="Contact Test";
                        }
                    }
                    else
                    {
                        strGrid->Cells[j+1][count]=result[i*cols+j];
                    }
                }
                count++;
            }
        }

        strGrid->Cells[0][0]="No.";
        strGrid->RowCount=count;
    }
    else
    {
        strGrid->RowCount=2;
        for(int i=0; i<strGrid->ColCount; i++)
            strGrid->Cells[i][1]="";
        strGrid->Cells[1][1]="No Record!!";
    }
    sqlite3_free(errMsg);
    sqlite3_free_table(result);
    return count-1;                                                             //AI(GA1-B4) 20260804: golden quirk -- returns count-1 (:1541), preserved verbatim
}
//---------------------------------------------------------------------------
AnsiString ExString="";                                                         //Steven 20151111 : 改成全域變數,避免Alarm與Message按的時候沒紀錄
//---------------------------------------------------------------------------
// AI(GA1-B4) 20260804: NewRecordProcess -- HOMECOMING GATED.  Golden body
// preserved verbatim below.  A real (empty no-op) stand-in for this EXACT
// external symbol already lives in acatchtray_shims.cpp:132 (declared
// acatchtray_shims.h:388) -- defining it again here would be a duplicate-
// definition link error.  Un-gating requires the integrator swap (see the
// GA-1-B4 report).
//---------------------------------------------------------------------------
#if 0 // TODO(GA1-B4-integrate): homecoming from acatchtray_shims.cpp -- integrator swaps
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug)     //Steven 20161220 : Process加上Alarm Code
{
    if(CUSTOMER_CODE==CC_Greatek && IniConfig.bN14_1_EnableOEEFunction==true)   //Sam 20170926 (Steven) : 超豐紀錄操作
    {
        fProductionInfo->SaveMessageHistroy("", S, 0, 0);
    }

    if(CUSTOMER_CODE!=CC_ASE_KaohSiung && IniConfig.bSPILFunction==false)       //wei 20150420 SPIL 要求按幾次按鍵都要顯示   //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        if(S==ExString && S.Pos(" pressed")!=0)
            return;                                                             //2008/06/24 lee avoid same record repeat be record
    }
    ExString=S;
    if(Debug=="")
        Debug=" ";

    MyDBIProcessNew("Process", AlarmCode, S, Debug);                            //Steven 20091004 : 紀錄按鍵被按的時間
}
#endif // NewRecordProcess homecoming gate
//---------------------------------------------------------------------------
// AI(GA1-B4) 20260804: RecordProcess -- HOMECOMING GATED.  Golden body
// preserved verbatim below.  A real (stdout-log) stand-in for this EXACT
// external symbol already lives in canary_support.cpp:102 (declared
// canary_support.h:256, included tree-wide by 120+ TUs) -- defining it again
// here would be a duplicate-definition link error.  Un-gating requires the
// integrator swap (see the GA-1-B4 report).
//---------------------------------------------------------------------------
#if 0 // TODO(GA1-B4-integrate): homecoming from canary_support.cpp -- integrator swaps
void RecordProcess(AnsiString S, AnsiString S2)
{
    if(CUSTOMER_CODE!=CC_ASE_KaohSiung && IniConfig.bSPILFunction==false)       //wei 20150420 SPIL 要求按幾次按鍵都要顯示  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    {
        if(S==ExString && S.Pos(" pressed")!=0)
            return;                                                             //2008/06/24 lee avoid same record repeat be record
    }
    ExString=S;
    MyDBIProcess("Process", S, S2);                                             //Steven 20091004 : 紀錄按鍵被按的時間
}
#endif // RecordProcess homecoming gate
//---------------------------------------------------------------------------
// RecordChangeLogProcess -- NO existing stand-in anywhere in the tree
// (verified: no static/extern/definition-shaped match tree-wide).  Translated
// ACTIVELY.  Its MyDBIProcess("ChangeLog", S, S2) call resolves, at link
// time, to whichever body currently satisfies the `void __fastcall
// MyDBIProcess(AnsiString,AnsiString,AnsiString="")` declaration in cMyDB.h
// -- today that is SECSGEM/uHGemEquipment.cpp's stand-in; after the
// integrator's homecoming swap, it becomes this file's own (now-active)
// definition.  Either way, exactly one definition exists tree-wide.
//---------------------------------------------------------------------------
void RecordChangeLogProcess(AnsiString S, AnsiString S2)                        //wei 20180625 offset Change log紀錄
{
    if(CUSTOMER_CODE!=CC_ASE_KaohSiung && CUSTOMER_CODE!=CC_SPIL_SHINCHU)
    {
        if(S==ExString && S.Pos(" pressed")!=0)
            return;                                                             //2008/06/24 lee avoid same record repeat be record
    }
    ExString=S;
    MyDBIProcess("ChangeLog", S, S2);                                           //Steven 20091004 : 紀錄按鍵被按的時間
}
//---------------------------------------------------------------------------
void GetJameCodeOfAxis(int iAxis, TComboBox *ComboBox)                          //Steven 20140221 : 根據Axis取出對應的Jam Code  //Steven 20200331 : Alarm code list改用文字檔
{
    ComboBox->Clear();
    AnsiString FileName="D:\\HT9045\\Error\\AlarmCodeList.txt";
    AnsiString Key, Value, Str, Axis;
    int _iAxis;
    TStringList *List=new TStringList();
    if(FileExists(FileName))
    {
        List->LoadFromFile(FileName);
        for(int i=0; i<List->Count; i++)
        {
            Str=List->Strings[i];
            Key=Str.SubString(1, Str.AnsiPos("=")-1);
            Value=Str.SubString(Str.AnsiPos("=")+1, Str.Length());
            Axis=Str.SubString(4, 2);
            _iAxis=atoi(Axis.c_str());
            if(iAxis==_iAxis)
            {
                ComboBox->Items->Add(Key+"  :  "+Value);
            }
        }
    }
    List->Clear();
    delete List;
}
//---------------------------------------------------------------------------
AnsiString aBackEventLogFile="";
AnsiString aBackEventLogMessage="";
AnsiString asEventTrackerFile="";
AnsiString asEventTrackerMsg="";
TQPF_Timer tEventLogTimer;
int iPerMinuteNumberofError;
int iRecordEventLogUPH;
//---------------------------------------------------------------------------
void __fastcall SaveEventTracker(AnsiString aAlarmCode, AnsiString aMess, int iType, AnsiString aStatus)
{
    if(aMess.Pos("\t")!=0 && iType!=0)
    {
        return;
    }

    AnsiString aHandler=" ", aRecipe=" ", aLotName=" ", LotInName=" ", aJamStartTime=" ", aJamStartDate=" ", aSubject=" ",
               aTesterID=" ", aSwVersion=" ";
    AnsiString aJamStopTime=" ";
    AnsiString aUPH=" ";
    bool bWrite=true;

//    if(IniConfig.bN22Enable_EventLog==false)  //JerryYang 20190702 Mark掉,功能沒開也要存log  //Steven 20181224 : For ASE-CL
//        return;

    if(fMain!=NULL)
        aRecipe.sprintf("%s", fMain->cbSetupFileName->Text);

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        // (aSiteID, golden-only, unused elsewhere in this reduced body)
    }

    if(fLotInfo!=NULL)
    {
#if 0 // TODO(GA1-B4): fLotInfo->edtASECL_TesterID not yet ported (forms/fLotInfo.h has no such member)
        aTesterID.sprintf("%s", fLotInfo->edtASECL_TesterID->Text);
#endif
        aLotName.sprintf("%s", fLotInfo->edtASECL_LotID->Text);
        aHandler.sprintf("%s", IniConfig.SocketHandlerID);
        // AI(W906-FW-LOTINFO-W30) 20260826: gate RETIRED (golden cMyDB.cpp:1651).
        // cbbASECL_LoginMode landed as a real TComboBox* member in FW-LOTINFO-W27
        // (forms/fLotInfo.h:2133).  The SIBLING gate above stays: edtASECL_TesterID
        // still has ZERO declaration on TfLotInfo -- re-verified 20260826, the only
        // hits in forms/fLotInfo.h are comment text (:67/:172/:343/:1663).
        // EFFECT TODAY: LotInName goes from " " to cbbASECL_LoginMode->Text, which
        // is "" until something assigns it (vclcompat TComboBox::Text is a plain
        // AnsiString, Controls.h:388 -- NOT derived from ItemIndex, and the only
        // writer of that member, fLotInfo.cpp:5006/5010, sets ItemIndex only).
        // The CSV that consumes LotInName is itself still gated below, so this is
        // unobservable today -- opening it now is precisely what stops the future
        // as9045LogPath integrator from silently emitting " " for LOGIN MODE.
        LotInName.sprintf("%s", fLotInfo->cbbASECL_LoginMode->Text);
        aJamStartDate.sprintf("%04d/%d/%d", SystemYear, SystemMonth, SystemDate);
        aJamStartTime.sprintf("%02d:%02d:%02d", SystemHour, SystemMin, SystemSec);
        aSwVersion.sprintf("%s", RunInfo.SoftwareVersion);
    }

    if(     iType==0)   aSubject="ERROR";
    else if(iType==10)  aSubject="START";
    else if(iType==11)  aSubject="END";
    else if(iType==12)  aSubject="INPUT";
    else if(iType==13)  aSubject="PASS";
    else if(iType==14)  aSubject="FAIL";
    else if(iType==15)  aSubject="RATE";
    else if(iType==16)  aSubject="IDLE TIME";
    else if(iType==17)  aSubject="PRODUCTION TIME";
    else if(iType==18)  aSubject="DOWN TIME";
    else if(iType==19)  aSubject="SOCKET";
    else if(iType==20)  aSubject="WARNING";
    else if(iType==21)  aSubject="CHANGE LOG";
    else                aSubject="PROCESS";

    if(iType==0)
    {
        tEventLogTimer.LatchCycleTimeSec(true);
//        bWrite=false;
        aSubject="WARNING";
        iPerMinuteNumberofError++;
        aJamStopTime.sprintf("%02d:%02d:%02d", SystemHour, SystemMin, SystemSec);
    }

    aUPH.sprintf("%d", iRecordEventLogUPH);

#if 0 // TODO(GA1-B4): as9045LogPath not yet ported (no declaration anywhere
      // in this tree) -- golden :1683-1721 ASE-log CSV write body gated as a
      // whole (Target/fopen/fputs/fclose all chain off this one missing path
      // global).  (void)-silenced above: aHandler/aRecipe/aTesterID/aLotName/
      // LotInName/aJamStartDate/aJamStartTime/aSwVersion/aJamStopTime/aUPH/
      // bWrite are all computed but, with this block gated, go unconsumed.
    AnsiString aTargetPath, aPath, str, str1, Target;
    FILE *Fp;
    aPath=as9045LogPath+"\\ASE log";
    aTargetPath.sprintf("%s\\%04d\\%02d\\%02d\\", aPath, SystemYear, SystemMonth, SystemDate);
    MyForceDirectories(aTargetPath, __FUNC__);
    MyForceDirectories(asSaveEventLogPath, __FUNC__);
    str.sprintf("%s@%04d_%02d_%02d_EventTracker", IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate);
    Target=aTargetPath+str+".csv";

    if(FileExists(Target)==false)
    {
        Fp=fopen(Target.c_str(), "a+");
        if(Fp==NULL)
            return;
        str1.sprintf("SWVERSION,SITEID,PROJECTID,MACHINEID,DATE,TIME,LOT NAME,LOGIN MODE,KEYWORD,ERROR CODE,ERROR MESSAGE,MESSAGE\n");
        fputs(str1.c_str(), Fp);
    }
    else
    {
        Fp=fopen(Target.c_str(), "a+");
        if(Fp==NULL)
            return;
    }

//    if(asEventTrackerMsg!=str)
//    {
//        if(fLotInfo!=NULL)
//            fLotInfo->UploadEventLogFile(asEventTrackerMsg);
//        asEventTrackerMsg=str;
//    }

    str.sprintf("%s,%s,%s,%s,%04d/%02d/%02d,%s,%s,%s,%s,%s,\"%s\",%s\n",
                    aSwVersion, "" /*aSiteID*/, aRecipe, IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate, aJamStartTime,
                    aLotName, LotInName, aSubject, aAlarmCode, aMess, "" /*aMessage*/);

    if(bWrite==true)
        fputs(str.c_str(), Fp);
    else
        asEventTrackerMsg=str;
    fclose(Fp);
#else
    (void)aHandler; (void)aRecipe; (void)aLotName; (void)LotInName; (void)aJamStartDate;
    (void)aJamStartTime; (void)aTesterID; (void)aSwVersion; (void)aJamStopTime; (void)aUPH; (void)bWrite; (void)aStatus; (void)aAlarmCode;
#endif
}
//---------------------------------------------------------------------------
void __fastcall SaveEventLogInfo(AnsiString aAlarmCode, AnsiString aMess, int iType, AnsiString aStatus, AnsiString sErrPart)
{
    AnsiString aHandler=" ", aRecipe=" ", aLotName=" ", LotInName=" ", aJamStartTime=" ", aJamStartDate=" ", aSubject=" ", Target, aJamReStartTime=" ", aDownTime=" ", aUPH=" ", aTesterID=" ";
    AnsiString str, str1;
    AnsiString aJamStopTime=" ";
    bool bWrite=true;

//    if(IniConfig.bN22Enable_EventLog==false)                                  //JerryYang 20190702 Mark掉,功能沒開也要存log  //Steven 20181224 : For ASE-CL
//        return;
    if(aAlarmCode!="220000000")
        SaveEventTracker(aAlarmCode, aMess, iType, aStatus);

    if(fMain!=NULL)
        aRecipe.sprintf("%s", fMain->cbSetupFileName->Text);

    if(fLotInfo!=NULL)
    {
#if 0 // TODO(GA1-B4): fLotInfo->edtASECL_TesterID not yet ported -- see SaveEventTracker's note above.
        aTesterID.sprintf("%s", fLotInfo->edtASECL_TesterID->Text);
#endif
        aLotName.sprintf("%s", fLotInfo->edtASECL_LotID->Text);
        aHandler.sprintf("%s", IniConfig.SocketHandlerID);
        // AI(W906-FW-LOTINFO-W30) 20260826: gate RETIRED (golden cMyDB.cpp:1744) --
        // see SaveEventTracker's note above for the member citation and the
        // edtASECL_TesterID sibling that stays gated.
        // ⚠ THIS SITE IS NOT UNOBSERVABLE, unlike the SaveEventTracker one: the
        // asSaveEventLogPath #else arm below (:2113-2120) DOES consume LotInName,
        // building aBackEventLogMessage whenever bWrite==false (iType==0, i.e. every
        // alarm).  aBackEventLogMessage has no reader anywhere in this tree
        // (grepped 20260826: cMyDB.h:137 extern + cMyDB.cpp:1891/2111/2115 writes
        // only), so the delta is confined to one write-only in-memory global -- no
        // file, no machine, no outward command.  Golden puts the combo text there.
        LotInName.sprintf("%s", fLotInfo->cbbASECL_LoginMode->Text);
        aJamStartDate.sprintf("%04d/%d/%d", SystemYear, SystemMonth, SystemDate);
        aJamStartTime.sprintf("%02d:%02d:%02d", SystemHour, SystemMin, SystemSec);
    }

    if(     iType==0)   aSubject="ERROR";
    else if(iType==10)  aSubject="START";
    else if(iType==11)  aSubject="END";
    else if(iType==12)  aSubject="INPUT";
    else if(iType==13)  aSubject="PASS";
    else if(iType==14)  aSubject="FAIL";
    else if(iType==15)  aSubject="RATE";
    else if(iType==16)  aSubject="IDLE TIME";
    else if(iType==17)  aSubject="PRODUCTION TIME";
    else if(iType==18)  aSubject="DOWN TIME";
    else if(iType==19)  aSubject="SOCKET";
    else if(iType==20)  aSubject="WARNING";
    else if(iType==21)  aSubject="CHANGE LOG";
    else                aSubject="PROCESS";

    if(iType==0)
    {
        tEventLogTimer.LatchCycleTimeSec(true);
        bWrite=false;
        aSubject="WARNING";
        iPerMinuteNumberofError++;
        aJamStopTime.sprintf("%02d:%02d:%02d", SystemHour, SystemMin, SystemSec);
    }

    aUPH.sprintf("%d", iRecordEventLogUPH);

#if 0 // TODO(GA1-B4): asSaveEventLogPath not yet ported (no declaration
      // anywhere in this tree) -- golden :1774-1809 handler-log CSV write
      // body gated as a whole (MyForceDirectories/Target/FileExists/fopen all
      // chain off this one missing path global).
    FILE *Fp;
    MyForceDirectories(asSaveEventLogPath, __FUNC__);
    str.sprintf("HANDLER LOG_%s_%04d_%02d_%02d", IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate);
    Target=asSaveEventLogPath+"\\"+str+".csv";

    if(FileExists(Target)==false)
    {
        Fp=fopen(Target.c_str(), "a+");
        if(Fp==NULL)
            return;
        str1.sprintf("SiteID,ProjectID,TesterID,DATE,TIME,LOT NAME,LOGIN MODE,STOP TIME,RESTART TIME,DOWN TIME,KEYWORD,STATUS,ERROR CODE,ERROR TYPE,ERROR MESSAGE,MESSAGE\n");  //pig 20180413 AddASECLTestLogInfor
        fputs(str1.c_str(), Fp);
    }
    else
    {
        Fp=fopen(Target.c_str(), "a+");
        if(Fp==NULL)
            return;
    }

#if 0 // TODO(GA1-B4): fLotInfo->UploadEventLogFile not yet ported (forms/fLotInfo.h has no such member)
    if(aBackEventLogFile!=str)
    {
        if(fLotInfo!=NULL)
            fLotInfo->UploadEventLogFile(aBackEventLogFile);
        aBackEventLogFile=str;
    }
#endif

    str.sprintf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    aHandler, aRecipe, aTesterID, aJamStartDate, aJamStartTime, aLotName, LotInName,     //pig 20180413 AddASECLTestLogInfor
                    aJamStopTime, aJamReStartTime, aDownTime,
                    aSubject, aStatus, aAlarmCode, aSubject, aMess, sErrPart);

    if(bWrite==true)
        fputs(str.c_str(), Fp);
    else
        aBackEventLogMessage=str;
    fclose(Fp);
#else
    if(bWrite==false)
        aBackEventLogMessage.sprintf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                    aHandler, aRecipe, aTesterID, aJamStartDate, aJamStartTime, aLotName, LotInName,
                    aJamStopTime, aJamReStartTime, aDownTime,
                    aSubject, aStatus, aAlarmCode, aSubject, aMess, sErrPart);
    (void)Target; (void)str1; (void)aUPH;
#endif
}
//---------------------------------------------------------------------------
void __fastcall MyDBUpdateDB()
{
    AnsiString Str, KeyString, ValueString, Date;                               //Steven 20170202 (wei): Fixed for Unknown Alarm Code
    AnsiString TextPath;
    TStringList *slAlarmDescrpt=new TStringList();

#if 0 // TODO(GA1-B4): fMain->AlarmCodeList / fMain->UnitNameMap / fMain->AlarmCodeMap
      // not yet ported (forms/fMain.h facade has no such members) -- golden
      // :1816-1845 in-memory alarm-code cache priming.  The rest of this
      // function (ALTER/DELETE table maintenance, VACUUM, and the
      // DoInsertAlarmCode calls below) does not depend on this cache -- see
      // DoInsertAlarmCode's own note (INSERT OR REPLACE is idempotent without it).
    fMain->AlarmCodeList=new TStringList();
    fMain->UnitNameMap=new TStringList();
    int iPos;
    if(FileExists("D:\\HT9045\\Error\\AlarmCodeList.txt"))
    {
        fMain->AlarmCodeList->LoadFromFile("D:\\HT9045\\Error\\AlarmCodeList.txt");
    }
    else
    {
        if(MyDBQAlarmCodeList())
        {
            fMain->AlarmCodeList->SaveToFile("D:\\HT9045\\Error\\AlarmCodeList.txt");
        }
    }

    for(int i=0; i<iAlarmUnitTotal; i++)                                        //Steven 20231127 : 整理Alarm Unit
    {
        fMain->UnitNameMap->Add(AlarmUnit[i]);
    }

    for(int i=0; i<fMain->AlarmCodeList->Count; i++)
    {
        Str=fMain->AlarmCodeList->Strings[i];
        iPos=Str.AnsiPos("=");
        KeyString=Str.SubString(1, iPos-1);
        ValueString=Str.SubString(iPos+1, Str.Length());
        fMain->AlarmCodeMap[KeyString]=ValueString;
    }
#else
    MyDBQAlarmCodeList();
#endif

    MyDBExecSQL("ALTER TABLE 'Message' ADD COLUMN 'AlarmCode' CHAR");           //Steven 20161220 : Process加上Alarm Code
    MyDBExecSQL("ALTER TABLE 'Motion' ADD COLUMN 'AlarmCode' CHAR");
    MyDBExecSQL("ALTER TABLE 'Process' ADD COLUMN 'AlarmCode' CHAR");

    Date.sprintf("%04d-%02d", SystemYear-1, SystemMonth);                       //Steven 20170209 : 減少MDB的檔案大小
    MyDBExecSQL("DELETE FROM EventLog      WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM Motion        WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM Message       WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM Process       WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM TotalLoader   WHERE OccurDateTime < '"+Date+"%'");
    Date.sprintf("%04d-%02d", SystemYear-2, SystemMonth);
    MyDBExecSQL("DELETE FROM TimeData      WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM Production    WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM LotInfo       WHERE OccurDateTime < '"+Date+"%'");
    MyDBExecSQL("DELETE FROM ClearDateTime WHERE OccurDateTime < '"+Date+"%'");
    MyDBVACUUM();

    //以上不要亂動-------------------------
    //
    //以下為新增的Alarm Code
    for(int i=0; i<MaxCylinderItem; i++)                                        //Steven 20231127 : 氣缸Alarm改成自動生成, 分類31
    {
        if(Cylinder[i].CylinderName!="")
        {
            KeyString.sprintf("JAM31%03d", i);
            TextPath.sprintf("D:\\HT9045\\Error\\English\\%s.dat", KeyString);
            ValueString.sprintf("Cylinder %s push or pop error!", Cylinder[i].CylinderName);

            DoInsertAlarmCode(KeyString, ValueString);
            if(FileExists(TextPath)==false)
            {
                slAlarmDescrpt->Clear();
                Str.sprintf("Cylinder %s push or pop error!", Cylinder[i].CylinderName);
                slAlarmDescrpt->Add(Str);
                slAlarmDescrpt->Add("");
                Str.sprintf("- The action speed of cylinder might too slow, please adjust the pressure regulating valve!");
                slAlarmDescrpt->Add(Str);

                if(Cylinder[i].Enable)
                {
                    Str.sprintf("- The solenoid valve of cylinder (O%d0%d%d%d) might not work well, please check it.",
                                                                    Cylinder[i].OutRing, Cylinder[i].OutIP, Cylinder[i].OutPort, Cylinder[i].OutBit);
                }
                else
                {
                    Str.sprintf("- The solenoid valve of cylinder might not work well, please check it.");
                }
                slAlarmDescrpt->Add(Str);

                if(Cylinder[i].OnSenEnable)
                {
                    Str.sprintf("- The on sensor of cylinder (I%d1%d%d%d) might not work well, please check it.",
                                                                    Cylinder[i].OnSenRing, Cylinder[i].OnSenIP, Cylinder[i].OnSenPort, Cylinder[i].OnSenBit);
                }
                else
                {
                    Str.sprintf("- The on sensor of cylinder might not work well, please check it.");
                }

                slAlarmDescrpt->Add(Str);
                if(Cylinder[i].OffSenEnable)
                {
                    Str.sprintf("- The off sensor of cylinder (I%d1%d%d%d) might not work well, please check it.",
                                                                    Cylinder[i].OffSenRing, Cylinder[i].OffSenIP, Cylinder[i].OffSenPort, Cylinder[i].OffSenBit);
                }
                else
                {
                    Str.sprintf("- The off sensor of cylinder might not work well, please check it.");
                }
                slAlarmDescrpt->Add(Str);
                slAlarmDescrpt->SaveToFile(TextPath);
            }
        }
    }

    if(slAlarmDescrpt!=NULL)
        slAlarmDescrpt->Clear();
    delete slAlarmDescrpt;

    for(int i=0; i<tcTotalCount; i++)                                           //Steven 20250110 : 溫度Alarm改成自動生成, 分類15
    {
        KeyString.sprintf("WAR15%02d", i);
        ValueString.sprintf("%s temperature OverLow!", asTempCtrl[i]);
        DoInsertAlarmCode(KeyString, ValueString);

        KeyString.sprintf("WAR151%02d", i);
        ValueString.sprintf("%s temperature OverHigh!", asTempCtrl[i]);
        DoInsertAlarmCode(KeyString, ValueString);
    }

    DoInsertAlarmCode("MES2821",    "No tray on fix tray 4");
    DoInsertAlarmCode("MES2921",    "No tray on fix tray 5");
    DoInsertAlarmCode("MES3021",    "No tray on fix tray 6");

    DoInsertAlarmCode("MES1713",   "Please scan the NO ReTest BIN ID of Fix1!");//RogerYang 20250604 偉測不可複測bin功能
    DoInsertAlarmCode("MES1813",   "Please scan the NO ReTest BIN ID of Fix2!");
    DoInsertAlarmCode("MES1913",   "Please scan the NO ReTest BIN ID of Fix3!");
    DoInsertAlarmCode("MES2813",   "Please scan the NO ReTest BIN ID of Fix4!");
    DoInsertAlarmCode("MES2913",   "Please scan the NO ReTest BIN ID of Fix5!");
    DoInsertAlarmCode("MES3013",   "Please scan the NO ReTest BIN ID of Fix6!");
    DoInsertAlarmCode("WAR2098",   "Loader Ion Gun Alarm!!");                  //Ifor 20230427 add:Loader Ionizer Gun
    DoInsertAlarmCode("MES16334",  "Loader Optical Gate Abnormal Alarm!!");    //Ifor 20230427 add:LD/ULD light gate
    DoInsertAlarmCode("WAR04200",   "In shuttle 1 Barcode Compare Error!!");
    DoInsertAlarmCode("WAR04201",   "In shuttle 2 Barcode Compare Error!!");

    DoInsertAlarmCode("WAR04202",   "In shuttle 1 Remote Barcode Compare Error!!");
    DoInsertAlarmCode("WAR04203",   "In shuttle 2 Remote Barcode Compare Error!!");

    DoInsertAlarmCode("WAR04204",   "In shuttle 1 Barcode Check Sum Error!!");
    DoInsertAlarmCode("WAR04205",   "In shuttle 2 Barcode Check Sum Error!!");

    DoInsertAlarmCode("WAR04206",   "In shuttle 1 Pin 1 inspection Error!!");
    DoInsertAlarmCode("WAR04207",   "In shuttle 2 Pin 1 inspection Error!!");
    DoInsertAlarmCode("WAR04208", "In Shuttle 1 Pin1 Receive Time Out");        //Eastsun 20260515 F022: D1 add
    DoInsertAlarmCode("WAR04209", "In Shuttle 2 Pin1 Receive Time Out");        //Eastsun 20260515 F022: D1 add

    DoInsertAlarmCode("WAR16500",   "Local Recipe Switching Failed!!");    //Ifor 20230427 add:LD/ULD light gate
    DoInsertAlarmCode("WAR07451",   "Barcode Pin1 Function Enabled!");      //Ifor 20240528 add:Pin1 Function
    DoInsertAlarmCode("MES16119", "Chiller water leakage alarm!");

    DoInsertAlarmCode("WAR0916", "There is a tray on Loader (the entrance)!");
    DoInsertAlarmCode("WAR1016", "There is a tray on Empty (the entrance)!");
    DoInsertAlarmCode("WAR1116", "There is a tray on Auto 1 (the entrance)!");
    DoInsertAlarmCode("WAR1216", "There is a tray on Auto 2 (the entrance)!");
    DoInsertAlarmCode("WAR1316", "There is a tray on Auto 3 (the entrance)!");
    DoInsertAlarmCode("WAR2516", "There is a tray on Auto 4 (the entrance)!");
    DoInsertAlarmCode("WAR2616", "There is a tray on Auto 5 (the entrance)!");
    DoInsertAlarmCode("WAR2716", "There is a tray on Auto 6 (the entrance)!");
    DoInsertAlarmCode("WAR1416", "There is a tray on Color (the entrance)!");

    DoInsertAlarmCode("WAR15403", "Soak time setting error!");
    DoInsertAlarmCode("WAR15404", "Work temperature setting error!");

    DoInsertAlarmCode("WAR2080", "Bin Trolley Ion fan Alarm!!");
    DoInsertAlarmCode("WAR2081", "Bin Trolley Ion fan Power Alarm!!");

    DoInsertAlarmCode("MES1181", "Auto 1 C_Auto1_Selector_Off Error!!");
    DoInsertAlarmCode("MES1281", "Auto 2 C_Auto2_Selector_Off Error!!");
    DoInsertAlarmCode("MES1381", "Auto 3 C_Auto3_Selector_Off Error!!");
    DoInsertAlarmCode("MES2581", "Auto 4 C_Auto4_Selector_Off Error!!");
    DoInsertAlarmCode("MES2681", "Auto 5 C_Auto5_Selector_Off Error!!");
    DoInsertAlarmCode("MES2781", "Auto 6 C_Auto6_Selector_Off Error!!");

    DoInsertAlarmCode("MES0917", "Loader SnLoaderUpSafedetect Error!!!!");
    DoInsertAlarmCode("MES1117", "Auto 1 SnAuto1_Selector_Off Error!!!!");
    DoInsertAlarmCode("MES1217", "Auto 2 SnAuto2_Selector_Off Error!!!!");
    DoInsertAlarmCode("MES1317", "Auto 3 SnAuto3_Selector_Off Error!!!!");
    DoInsertAlarmCode("MES2517", "Auto 4 SnAuto4_Selector_Off Error!!!!");
    DoInsertAlarmCode("MES2617", "Auto 5 SnAuto5_Selector_Off Error!!!!");
    DoInsertAlarmCode("MES2717", "Auto 6 SnAuto6_Selector_Off Error!!!!");

    DoInsertAlarmCode("WAR07362", "SECS GEM consecutive failure!");

    DoInsertAlarmCode("WAR16446", "Chamber Dry Air not enough");

    DoInsertAlarmCode("WAR0354", "RTC Alarm Arm 1 NG");     //wei 20221222 RTC ARM Error
    DoInsertAlarmCode("WAR0355", "RTC Alarm Arm 2 NG");     //wei 20221222 RTC ARM Error
    DoInsertAlarmCode("WAR0356", "RTC Alarm CCD Stop Sensor ON,Error type time out.");  //wei 20221222 RTC ARM Error
    DoInsertAlarmCode("WAR16132", "Run Check function has been turn off!");

    DoInsertAlarmCode("MES0736", "Received Tester PAUSE Request > Refer To Internal System Error Message!");

    DoInsertAlarmCode("WAR07402", "Site Mapping Check Fail! Must Pass Bin,need Do again!"); //kevin 20210730
    DoInsertAlarmCode("WAR16126", "Auto site mapping function is OFF!");        //JerryYang 20230327 : Add / WAR16118->WAR16126 Steven 20260331

    DoInsertAlarmCode("WAR14061", "Color tray IC remain communication timeout!");
    DoInsertAlarmCode("WAR14062", "Color tray IC remain ROI check error!");
    DoInsertAlarmCode("WAR14063", "Color tray IC remain laser initial fail!");
    DoInsertAlarmCode("WAR14064", "Color tray IC remain grab error!");
    DoInsertAlarmCode("WAR14065", "Color tray IC remain inspection error!");
    DoInsertAlarmCode("WAR14066", "Color tray IC remain detect error!");

    DoInsertAlarmCode("WAR07403", "Initial Site Mapping Detect Continue Fail!");
    DoInsertAlarmCode("WAR07404", "Idle Site Mapping Detect Continue Fail!");
    DoInsertAlarmCode("WAR07405", "Interval Site Mapping Detect Continue Fail!");
    DoInsertAlarmCode("WAR07406", "Manual Site Mapping Detect Continue Fail!");

    DoInsertAlarmCode("WAR07460", "Indium contact count exceeded! Please replace indium sheet!");  //AI(ht9045-config) 20260507 (RogerYang) : VTEST銦片壽命報警

//    DoInsertAlarmCode(4085, "WAR0485", 7, "2DID is not on the allow list!");
    DoInsertAlarmCode("WAR0732", "Test arm 2DID is not on the allow list!");
//    DoInsertAlarmCode(4086, "WAR0486", 7, "In Shuttle 2 2DID is not on the allow list!");

    DoInsertAlarmCode("MES0105", "Device pick-up error on the tray");
    DoInsertAlarmCode("MES0106", "Device pick-up error on the tray");

    DoInsertAlarmCode("JAM0128", "In Arm Clean pad drop error");
    DoInsertAlarmCode("JAM0203", "Device drop error (Output arm)");

    DoInsertAlarmCode("WAR1487", "Tray ID Read Error on Color Stack!");
    DoInsertAlarmCode("WAR16339", "Tray ID Read Duplicate Error!");

    DoInsertAlarmCode("WAR0354", "Cotact count over setting, please change the socket!");

    DoInsertAlarmCode("JAM1158", "Auto 1 UpSafe Sensor Not In Position!");      //AI(general) 20260323 (RogerYang) : UpSafe sensor 未到位5秒
    DoInsertAlarmCode("JAM1258", "Auto 2 UpSafe Sensor Not In Position!");
    DoInsertAlarmCode("JAM1358", "Auto 3 UpSafe Sensor Not In Position!");
    DoInsertAlarmCode("JAM2558", "Auto 4 UpSafe Sensor Not In Position!");
    DoInsertAlarmCode("JAM2658", "Auto 5 UpSafe Sensor Not In Position!");
    DoInsertAlarmCode("JAM2758", "Auto 6 UpSafe Sensor Not In Position!");
}
//---------------------------------------------------------------------------
