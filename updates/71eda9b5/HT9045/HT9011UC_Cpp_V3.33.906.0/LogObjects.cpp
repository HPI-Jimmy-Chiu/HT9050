// ===========================================================================
//  LogObjects.cpp -- the golden TfMain TMyStringList log objects (cMyDB CSV plan P1).
//  AI(W906-CSVONLY-P1) 20260926.
//
//  Golden 912 main.cpp:1550-1724 (TfMain::TfMain) creates them; main.cpp:12508-12550 (TfMain::FormDestroy) deletes
//  them -- TMyStringList's destructor flushes (Public/MyStringList.cpp).  Golden never deletes slEventLog (:12559 is
//  commented out); kept.
//  V906: W906_CreateLogObjects() runs in the wb_serve boot chain after the configuration / recipe load (golden reads
//  IniConfig.bSPILFunction, Prod.iTrayType and the as*Path globals here); W906_DestroyLogObjects() at shutdown.
//  Paths, file names, header rows and SaveType options are golden text.
//
//  SCOPE (github-59 relaying 20260926; user decision #6 "all 27 or cMyDB's 4" not answered yet): only the four cMyDB
//  uses are CONSTRUCTED -- slEventLog (cmydef global), slJamAlarmLog, slTimeData, slProdRecordLog.  forms/fMain.h
//  declares all the golden members (nullptr), so "all 27" later only adds constructions here.  The other golden
//  objects, in golden order, are listed below as comments so the addition is mechanical.
//  ALSO here: golden SaveEventLog() (see its note below).
//  NOT here: slAutoSiteMapLog (the facade keeps its TfMainSiteMapLog stand-in, forms/FormWidgets.h:219) and the
//  TStringList members golden creates in the same place (slBundlID, slLowYieldAlarm, ImpParaCheck ...).
// ===========================================================================
#include "LogObjects.h"

#include "Public/MyStringList.h"
#include "forms/fMain.h"
#include "cmydef.h"        // slEventLog, s6TrayName[]
#include "cprod.h"         // Prod.iTrayType
#include "Config.h"        // IniConfig.bSPILFunction
#include "common.h"        // asProductRecordPath
#include "MachineType.h"   // eTrayCount, tNotUse, tTrayBox

static bool g_logObjectsCreated = false;

//---------------------------------------------------------------------------
//AI(W906-CSVONLY-P1) 20260926: golden cmydef.cpp SaveEventLog (Steven 20200416), the text V906 keeps gated at
//  cmydef.cpp:129-147 ("TODO(W3): TMyStringList body + RunInfo state").  It is defined HERE, in ht9045_db, instead of
//  lifting that gate: cmydef.cpp is in ht9045_globals, which several narrow ctests link without ht9045_sm (where
//  TMyStringList lives, Public/MyStringList.cpp), so the TMyStringList calls would break their links.  Every caller
//  (cMyDB.cpp) is in ht9045_db.  Callers guard slEventLog!=NULL, as golden.  Body = golden text.
void SaveEventLog()                                                             //Steven 20200416 : 整合EventLog存檔
{
    slEventLog->MySaveToFile();
    int iCount=RunInfo.slEventLogFile->Count;
    AnsiString FileName=slEventLog->GetFileName();

    if(iCount==0)
    {
        RunInfo.slEventLogFile->Add(FileName);
    }
    else
    {
        if(AnsiString(RunInfo.slEventLogFile->Text).AnsiPos(FileName)==0)   //AI(W906-CSVONLY-P1-FIX) 20260926: golden `->Text.AnsiPos(...)`; vclcompat Text is a TextProxy, wrapped like SECSGEM/uHGemHT9045.cpp:6084
        {
            RunInfo.slEventLogFile->Add(FileName);
        }
    }
}

void W906_CreateLogObjects()
{
    if (g_logObjectsCreated || fMain == 0)
        return;
    g_logObjectsCreated = true;
    TfMain* const self = fMain;     // golden runs this inside TfMain::TfMain; the TfMain members are self->X here
    AnsiString Buffer1, Str1, str;

    if(IniConfig.bSPILFunction==true)                                           //Steven 20240604 : SPIL格式的event log
    {
        slEventLog=new TMyStringList("D:\\HT9045_Log\\EventLogTxt",
                                     "EventLogTxt",
                                     "UnitName, AlarmCode, OccurDateTime, Recovery, StopedTime, Duplicate, Message, ErrPart");
    }
    else
    {
        slEventLog=new TMyStringList("D:\\HT9045_Log\\EventLogTxt",             //Steven 20161115 : EventLog存成文字檔
                                     "EventLogTxt",
                                     "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe");
    }
    // golden :1562 sl2DMappingLog, :1565 slMNetLog, :1569 slUploadFile, :1575 slTTLLog, :1578 slHeaterLog -- not yet (scope)

    self->slJamAlarmLog=new TMyStringList("D:\\HT9045_Log\\JamAlarmLogTxt",     //RogerYang 20170405 (Steven) 力成只記 Jam Alarm 且需帶入LotID, OperatorID, SetupFileName
                                    "JamAlarmLogTxt",
                                    "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Temperature, LotID, OperatorID, SetupFileName, RunMode");
    // golden :1584 slSocketIdProductData -- not yet (scope)

    self->slTimeData=new TMyStringList("D:\\HT9045_Log\\TimeData",              //JerryYang 20180515 (Steven) 記錄Time data
                                 "TimeData",
                                 "Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, JamTime, PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA");

    self->slTimeData->SaveType=TByYear;
    // golden :1594 slHanaTrayMap[], :1604 slIndexYMaxMinShift, :1608 slInputDataLog, :1635 slQtyLog -- not yet (scope)

    Buffer1="";
    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse &&
           Prod.iTrayType[i]!=tTrayBox)
        {
            if(Buffer1=="")
                Str1.sprintf("%sCount", s6TrayName[i].c_str());                 //AI(W906-CSVONLY-P1) 20260926: AnsiString -> .c_str() for the varargs
            else
                Str1.sprintf(", %sCount", s6TrayName[i].c_str());
            Buffer1=Buffer1+Str1;
        }
    }
    str.sprintf("LotID, OccurDateTime, LoadCount, UnLoadCount, %s, Action, ID_TotalLoader, JamCount, UPH, MTBR, MUBF, MachineID", Buffer1.c_str());
    self->slProdRecordLog=new TMyStringList(asProductRecordPath,                //JerryYang 20230721 : Analog要求production record
                               "ProductionRecordLog",
                               str);
    // golden :1659 slAutoSiteMapLog (stand-in, see banner), :1665 slLotInfolog, :1670 slTriTempDoorlog,
    //        :1675-1689 slDewPointLog[3], :1693 slRecordRunState, :1698 slTestLog, :1706 slTorqueLog,
    //        :1711 slTorqueLogNew, :1716 slTorquMUClog, :1720 slGroundManLog -- not yet (scope)
}

void W906_DestroyLogObjects()
{
    if (!g_logObjectsCreated || fMain == 0)
        return;
    g_logObjectsCreated = false;
    TfMain* const self = fMain;
    // golden 912 main.cpp:12508-12550, in golden order, for the objects created above
    delete self->slJamAlarmLog;                                                 //RogerYang 20170405 (Steven) : Jam Alarm Log
    self->slJamAlarmLog=nullptr;
    delete self->slTimeData;                                                    //JerryYang 20180515 (Steven) 記錄Time data
    self->slTimeData=nullptr;
    delete self->slProdRecordLog;                                               //JerryYang 20230721 : Analog要求production record
    self->slProdRecordLog=nullptr;
//        delete slEventLog;                                                    //Steven 20161115 : EventLog存成文字檔  (golden :12559, commented out in golden)
}
