// ===========================================================================
//  LogObjects.cpp -- the golden TfMain TMyStringList log objects (cMyDB CSV plan P1; all of them since W7).
//  AI(W906-CSVONLY-P1) 20260926.  AI(W906-LOGOBJ-W7) 20260927: Steven W7 = A.
//
//  Golden 906_0625_Steven main.cpp:1500-1675 (TfMain::TfMain) creates them; main.cpp:11991-12042 (TfMain::FormDestroy)
//  deletes them -- TMyStringList's destructor flushes (Public/MyStringList.cpp; empty -> nothing written).  (The same
//  block is 912 main.cpp:1549-1724, byte for byte.)  Golden never deletes slEventLog (:12042 is commented out),
//  slLotInfolog or slHanaTrayMap[]; kept.
//  V906: W906_CreateLogObjects() runs in the wb_serve boot chain after the configuration / recipe load (golden reads
//  IniConfig.bSPILFunction, Prod.iTrayType and the as*Path globals here); W906_DestroyLogObjects() at shutdown.
//  File names, header rows and SaveType options are golden text; paths too, except that the golden "D:\\HT9045_Log\\X"
//  literals are written as9045LogPath+"\\X" (AI(W906-LOGOBJ-SEAM) 20260927 (St02-E, optional hardening): the same value unless the ctest seam
//  W906_HT9045LOG_ROOT is set, common.cpp:240/:425).  No ctest calls either function, so in every
//  ctest these objects stay nullptr (and the HeaterLog hook stays unset, slAutoSiteMapLog stays the no-op stand-in).
//
//  W7 = A (Steven 20260927): all 24 golden names (58 objects: slHanaTrayMap[eTrayCount], slDewPointLog[3]).
//  What writes for real in wb_serve today: slHeaterLog (HeaterLog, 17 callers, through the hook below), slAutoSiteMapLog
//  (5 in-arm / home callers, through the swap below), sl2DMappingLog (2D sort only; it also removes a latent NULL
//  dereference at asortarm.cpp:3915), slQtyLog.  The other names have no live writer yet.
//  ALSO here: golden SaveEventLog() (see its note below).
//  NOT here: the TStringList members golden creates in the same place (slBundlID, slDupBundlID, slDupUnloadBundlID,
//  slLowYieldAlarm) and InArmSiteMapData.ClearData().
// ===========================================================================
#include "LogObjects.h"

#include "Public/MyStringList.h"
#include "forms/fMain.h"
#include "cmydef.h"        // slEventLog, sl2DMappingLog, slHanaTrayMap[], slGroundManLog, s6TrayName[]
#include "cprod.h"         // Prod.iTrayType
#include "Config.h"        // IniConfig.bSPILFunction
#include "common.h"        // asProductRecordPath, asQtyDataPath, asTorqLogPath, asHPCardPath, asGroundManPath
#include "MachineType.h"   // eTrayCount, tNotUse, tTrayBox
#include "cpublic.h"       // HeaterLog ("Close" at destroy)   //AI(W906-LOGOBJ-W7)
#include <utility>          // std::pair (the State Record log queue at the EOF)   //AI(W906-STATEREC-TS)
#include <vector>           // (same)

static bool g_logObjectsCreated = false;

//AI(W906-LOGOBJ-W7) 20260927: HeaterLog's golden line (906_0625_Steven cpublic.cpp:523
//  `fMain->slHeaterLog->AddTextWithDateTime(NewMess);`) runs through this hook: cpublic.cpp is in ht9045_globals, which
//  has neither fMain nor TMyStringList (narrow ctests link it alone), so its call site (cpublic.cpp:704-706) calls
//  W906_HeaterLogHook, defined at that file's end.  Set at create, cleared first at destroy.
extern void (*W906_HeaterLogHook)(AnsiString);

namespace {
void W906_HeaterLogBody(AnsiString NewMess)
{
    if(fMain && fMain->slHeaterLog)                                             //AI(W906-LOGOBJ-W7): V906 NULL guard (Steven W7=A)
        fMain->slHeaterLog->AddTextWithDateTime(NewMess);
}

//AI(W906-LOGOBJ-W7) 20260927: fMain->slAutoSiteMapLog is the TfMainSiteMapLog* stand-in (forms/fMain.h:191, built at
//  forms/fMain.cpp:65; a no-op).  Golden's member is a TMyStringList*.  Derive-and-swap (same shape as TfLotInfoLogMemo,
//  forms/fLotInfo.h): wb_serve gets a real TMyStringList behind the stand-in's interface, ctests keep the no-op, and no
//  shared file changes.  The stand-in is restored at destroy.
struct W906_SiteMapLogReal : public TfMainSiteMapLog
{
    TMyStringList *real;
    explicit W906_SiteMapLogReal(TMyStringList *p) : real(p) {}
    ~W906_SiteMapLogReal() override { delete real; }                            // TMyStringList dtor flushes (golden :12002)
    void AddTextWithDateTime(AnsiString S) override { real->AddTextWithDateTime(S); }
};
TfMainSiteMapLog *g_siteMapStandIn = nullptr;
}  // namespace

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
        if(AnsiString(RunInfo.slEventLogFile->Text).AnsiPos(FileName)==0)   //AI(W906-CSVONLY-P1-FIX) 20260926: golden `->Text.AnsiPos(...)`; vclcompat Text is a TextProxy, wrapped like SECSGEM
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
        slEventLog=new TMyStringList(as9045LogPath+"\\EventLogTxt",
                                     "EventLogTxt",
                                     "UnitName, AlarmCode, OccurDateTime, Recovery, StopedTime, Duplicate, Message, ErrPart");
    }
    else
    {
        slEventLog=new TMyStringList(as9045LogPath+"\\EventLogTxt",             //Steven 20161115 : EventLog存成文字檔
                                     "EventLogTxt",
                                     "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe");
    }
    sl2DMappingLog=new TMyStringList(as9045LogPath+"\\2DMapping",               //JerryYang 20230322 : add 2D mapping result
                                 "2DMapping",
                                 "2D Code, Binning, ECID, Device, Lot No., VS-shuttle, VS-unload, RC1-shuttle, RC1-unload, RC2-shuttle, RC2-unload, VS-Error log, RC1-Error log, RC2-Error log");
    self->slMNetLog =new TMyStringList(as9045LogPath+"\\MNetLog",               //Steven 20161115 : MNet Log改新版存檔
                                 "MNetLog",
                                 "");

    self->slUploadFile=new TMyStringList(as9045LogPath+"\\UploadFile",          //Steven 20250716 : 整合上傳的功能
                                 "UploadFile",
                                   "SourcePath, TargetPath, SourceFile, TargetFile");
    self->slUploadFile->MaxLineCount=1;
    self->slUploadFile->FixedFile=true;

    self->slTTLLog  =new TMyStringList(as9045LogPath+"\\TTL_Signal_LOG",        //Steven 20161115 : TTL Log改新版存檔
                                 "TTLLog",
                                 "date, time, Status, SwClearAll, SwAnti-StartSignal, SwStartEnable, SwReserve, SwStart0, SwStart1, SwStart2, SwStart3, SwDut0, SwDut1, SwDut2, SwDut3");
    self->slHeaterLog=new TMyStringList(as9045LogPath+"\\Heater_On_Off_LOG",    //Steven 20161115 : Heater Log改新版存檔
                                  "Heater",
                                  "");
    W906_HeaterLogHook=W906_HeaterLogBody;                                      //AI(W906-LOGOBJ-W7): cpublic.cpp HeaterLog -> slHeaterLog
    self->slJamAlarmLog=new TMyStringList(as9045LogPath+"\\JamAlarmLogTxt",     //RogerYang 20170405 (Steven) 力成只記 Jam Alarm 且需帶入LotID, OperatorID, SetupFileName, RunMode, 另外存csv檔
                                    "JamAlarmLogTxt",
                                    "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Temperature, LotID, OperatorID, SetupFileName, RunMode");
    self->slSocketIdProductData=new TMyStringList(as9045LogPath+"\\SocketIdProductData",                                //Sam 20170516 (wei) 力成增加 SocketId 計數
                                            "SocketIdProductData",
                                            "LotId, SocketID, Count");

    self->slTimeData=new TMyStringList(as9045LogPath+"\\TimeData",              //JerryYang 20180515 (Steven) 記錄Time data
                                 "TimeData",
                                 "Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, JamTime, PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA");

    self->slTimeData->SaveType=TByYear;

    for(int i=0; i<eTrayCount; i++)                                             //Steven 20250414 : HANA ART Function
    {
        slHanaTrayMap[i]=new TMyStringList(as9045LogPath+"\\Hana_TrayMap",
                                           s6TrayName[i].UpperCase(),
                                           "");

        slHanaTrayMap[i]->MaxLineCount=1;
        slHanaTrayMap[i]->bHanaTrayMap=true;
    }

    self->slIndexYMaxMinShift=new TMyStringList(as9045LogPath+"\\IndexPos",     //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值
                                          "IndexMaxMin",
                                          "Date, Time, MaxCMD, MinCMD, MaxYFront, MinYFront, MaxYRear, MinYRear");

    self->slInputDataLog=new TMyStringList(as9045LogPath+"\\JamAlarmLogTxt",    //Sam 20171208 (wei) : PTI Lot End 上報 Input 數量
                                     "JamAlarmLogTxt",
                                     "Date, Time, Input quantity, LotID, OperatorID, SetupFileName, RunMode");

    Buffer1="";
    for(int i=0; i<eTrayCount; i++)
    {
        if(Prod.iTrayType[i]!=tNotUse &&
           Prod.iTrayType[i]!=tTrayBox)
        {
            if(Buffer1=="")
                Str1.sprintf("%s", s6TrayName[i].c_str());                      //AI(W906-LOGOBJ-W7): AnsiString -> .c_str() for the varargs (as P1 below)
            else
                Str1.sprintf(", %s", s6TrayName[i].c_str());
            Buffer1=Buffer1+Str1;
        }
    }
    str=AnsiString("Date, Time, Action, Loading, Total,")+Buffer1;
    self->slQtyLog=new TMyStringList(asQtyDataPath,                             //Sam 20171208 (wei) : PTI Lot End 上報 Input 數量
                               "QtyLog",
                               str);

    self->slQtyLog->SaveType=TByMonth;

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

    {
        TMyStringList *pASM=new TMyStringList(as9045LogPath+"\\ASM",            //Steven 20211209 : 紀錄Auto Site Map動作
                                              "ASMLog",
                                              "Date, Time, HP, HP X, HP Y, Site, Pick up");
        pASM->SaveType=TByDay;
        pASM->MaxLineCount=1;
        g_siteMapStandIn=self->slAutoSiteMapLog;                                //AI(W906-LOGOBJ-W7): the stand-in comes back at destroy
        self->slAutoSiteMapLog=new W906_SiteMapLogReal(pASM);
    }

    self->slLotInfolog=new TMyStringList(as9045LogPath+"\\LotInfo",             //Steven 20221225 : add for CyuEan
                                   "LotInfo",
                                   "Start Time, End Time, Tester OS Ver, Tester ID, Operator, Customer, Test Program, Device Name, Lot No, Sub Lot No, Mode Code, Test Code, Test Bin No, Machine ID, Stage, Step");
    self->slLotInfolog->SaveType=TByYear;

    self->slTriTempDoorlog=new TMyStringList(as9045LogPath+"\\TriTemp\\CheckInputDoor",
                                       "CheckInputDoorLog",
                                       "Date, Time, TaskNow, TaskNext, Action");
    self->slTriTempDoorlog->SaveType=TByHour;                                   //Steven 20230810 : 三溫開關門log

    self->slDewPointLog[0]=new TMyStringList(as9045LogPath+"\\DewPoint_Log",    //Steven 20230810 : 三溫露點計log
                                       "InArm_DewPoint_Log",
                                       "Date, Time, Degree, mA");

    self->slDewPointLog[1]=new TMyStringList(as9045LogPath+"\\DewPoint_Log",
                                       "Index_DewPoint_Log",
                                       "Date, Time, Degree, mA");

    self->slDewPointLog[2]=new TMyStringList(as9045LogPath+"\\DewPoint_Log",
                                       "OutArm_DewPoint_Log",
                                       "Date, Time, Degree, mA");

    self->slDewPointLog[0]->SaveType=TByHour;
    self->slDewPointLog[1]->SaveType=TByHour;
    self->slDewPointLog[2]->SaveType=TByHour;

    self->slRecordRunState=new TMyStringList(as9045LogPath+"\\RunState",        //Ifor 20230420 add:ASEM要求新增Run Status Log Function
                                       "RunStateLog",
                                       "Date, Time, Machine ID, State,  Message");
    self->slRecordRunState->SaveType=TByDay;

    self->slTestLog=new TMyStringList(as9045LogPath+"\\TestLog\\",              //Steven 20210608 : JSCK OEE Function.
                                "IP",
                                "");
    self->slTestLog->SaveType=TByMin;
    self->slTestLog->bFilePathWithDate=false;
    self->slTestLog->bUseFTRT=true;                                             //Steven 20220414 : JSCK OEE檔名要加上FT RT

    self->slTorqueLog=new TMyStringList(asTorqLogPath,                          //KaiHuang 20200513 Add : For ASE 高雄 扭力值存Log
                                  "TorqueLog",
                                  "Date, Time, Position, Arm, Value, AlarmCount, Force, SetValue");                     //kevin 20210421 change "Date, Time, Position, Arm, Value, Delay");
    self->slTorqueLog->SaveType=TByHour;

    self->slTorqueLogNew=new TMyStringList(asTorqLogPath,                       //Steven 20210524 : 連續讀取扭力
                                     "TorqueLogNew",
                                     "Date, Time, Arm, Count, Value");
    self->slTorqueLogNew->SaveType=TByMin;

    self->slTorquMUClog = new TMyStringList(asHPCardPath,                       //Steven 20211111 : 連續讀取扭力
                                      "MCU","");                                //kevin 20211111 : MCU Comman log
    self->slTorquMUClog->SaveType=TByHour;

    slGroundManLog=new TMyStringList(asGroundManPath,                           //KenHsieh 20220728 : 新增GroundMan Value Log
                                     "GroundMan",
                                     "Date, Time, InArm, OutArm, Arm1, Arm2, HP1, HP2, SH1, SH2, InArm_A, InArm_B, InArm_C, InArm_D, InArm_E, InArm_F, InArm_G, InArm_H, OutArm_A, OutArm_B, OutArm_C, OutArm_D, OutArm_E, OutArm_F, OutArm_G, OutArm_H, Loader, Empty, Color, Auto1, Auto2, Auto3");

    slGroundManLog->SaveType=TByHour;

    //AI(W906-CMYDB-P4-D2) 20260927 (Steven D2 = B): golden TfMain::FormShow (906_0625_Steven main.cpp:9411-9416) loads
    //  today's production log back into MemoProductionLog BEFORE anything can call ProductionLog -- ProductionLog saves
    //  the WHOLE memo over the file, so without this load the first save after a restart would truncate today's file.
    //  V906 runs it here, at the end of the boot-time log-object creation (the wb_serve boot chain calls GetTimeInfo()
    //  before this).
    if(IniConfig.bO06SaveLogTimePeriod)                                         //JerryYang 20160105 讀取ProductionLog,放在DoReadLastData之前
    {
        str.sprintf("%s\\%s_%04d%02d%02d.logs", asProductionLogPath.c_str(), IniConfig.SocketHandlerID.c_str(), SystemYear, SystemMonth, SystemDate);
        if(FileExists(str))
            self->MemoProductionLog->Lines->LoadFromFile(str);
    }
}

void W906_DestroyLogObjects()
{
    if (!g_logObjectsCreated || fMain == 0)
        return;
    g_logObjectsCreated = false;
    TfMain* const self = fMain;
    HeaterLog("Close", false);                                                  // golden FormClose main.cpp:11641 (V906 runs it here, at FormDestroy time: a location deviation)
    ProductionLog("Close");                                                     // golden FormClose main.cpp:11645 (same deviation)   //AI(W906-CMYDB-P4-D2)
    W906_HeaterLogHook=0;                                                       //AI(W906-LOGOBJ-W7): a late HeaterLog becomes a no-op, not a use-after-free
    // golden 906_0625_Steven main.cpp:11991-12042, in golden order
    delete self->slMNetLog;                                                     //Steven 20161115 : MNet Log改新版存檔
    self->slMNetLog=nullptr;
    delete self->slUploadFile;                                                  //Steven 20250716 : 整合上傳的功能
    self->slUploadFile=nullptr;
    delete self->slTTLLog;                                                      //Steven 20161115 : TTL Log改新版存檔
    self->slTTLLog=nullptr;
    delete self->slHeaterLog;                                                   //Steven 20161115 : Heater Log改新版存檔
    self->slHeaterLog=nullptr;
    delete self->AlarmCodeList;                                                 //Steven 20170202 (wei): Fixed for Unknown Alarm Code  //AI(W906-CMYDB-P3) 20260926: created by MyDBUpdateDB
    self->AlarmCodeList=nullptr;
    delete self->UnitNameMap;
    self->UnitNameMap=nullptr;
    delete self->slJamAlarmLog;                                                 //RogerYang 20170405 (Steven) : Jam Alarm Log
    self->slJamAlarmLog=nullptr;
    delete self->slSocketIdProductData;                                         //Sam 20170516 (wei)
    self->slSocketIdProductData=nullptr;
    delete self->slTimeData;                                                    //JerryYang 20180515 (Steven) 記錄Time data
    self->slTimeData=nullptr;
    delete self->slInputDataLog;                                                //Sam 20171208 (wei)
    self->slInputDataLog=nullptr;
    delete self->slQtyLog;                                                      //Sam 20171208 (wei)
    self->slQtyLog=nullptr;
    if(g_siteMapStandIn)                                                        //Steven 20211209 (golden :12002 delete slAutoSiteMapLog)
    {
        delete self->slAutoSiteMapLog;                                          //AI(W906-LOGOBJ-W7): deletes the real TMyStringList (flush) ...
        self->slAutoSiteMapLog=g_siteMapStandIn;                                //   ... and puts the facade's stand-in back
        g_siteMapStandIn=nullptr;
    }
    delete self->slRecordRunState;                                              //Ifor 20230420
    self->slRecordRunState=nullptr;
    delete self->slTorqueLog;                                                   //KaiHuang 20200513
    self->slTorqueLog=nullptr;
    delete self->slTestLog;                                                     //Steven 20210608
    self->slTestLog=nullptr;
    delete self->slTorqueLogNew;                                                //Steven 20210524
    self->slTorqueLogNew=nullptr;
    delete self->slTorquMUClog;                                                 //kevin 20211111
    self->slTorquMUClog=nullptr;
    delete slGroundManLog;                                                      //KenHsieh 20220728
    slGroundManLog=nullptr;
    delete sl2DMappingLog;                                                      //JerryYang 20230322
    sl2DMappingLog=nullptr;
    delete self->slProdRecordLog;                                               //JerryYang 20230721 : Analog要求production record
    self->slProdRecordLog=nullptr;
    delete self->slTriTempDoorlog;                                              //Steven 20230810
    self->slTriTempDoorlog=nullptr;
    delete self->slDewPointLog[2];                                              //Steven 20230810 (golden deletes 2, 1, 0)
    self->slDewPointLog[2]=nullptr;
    delete self->slDewPointLog[1];
    self->slDewPointLog[1]=nullptr;
    delete self->slDewPointLog[0];
    self->slDewPointLog[0]=nullptr;
    delete self->slIndexYMaxMinShift;                                           //Isaac 20201012
    self->slIndexYMaxMinShift=nullptr;
    // never deleted in golden: slLotInfolog, slHanaTrayMap[], slEventLog (:12042 commented out) -- kept
//        delete slEventLog;                                                    //Steven 20161115 : EventLog存成文字檔  (golden :12042, commented out in golden)
}

//---------------------------------------------------------------------------
//AI(W906-CMYDB-P4-D2) 20260927 (Steven D2 = B): golden ProductionLog, 906_0625_Steven cpublic.cpp:594-622 (declared
//  cpublic.h:45).  V906 keeps the same text gated in cpublic.cpp:778-806 as the reference copy; the live body is HERE,
//  in ht9045_db, for the same reason as SaveEventLog above: cpublic.cpp is in ht9045_globals, which has no fMain
//  (narrow ctests link it alone).  Callers: cMyDB.cpp MyDBIProcess / MyDBIProcessNew (golden 906_0625_Steven cMyDB.cpp:808 / :743; WIP said :752 / :818) and
//  W906_DestroyLogObjects ("Close").  Only .c_str() added for the varargs.
//  Deviation, recorded: VCL TStrings::SaveToFile throws EFCreateError when asProductionLogPath (golden D:\RMS, never
//  created by golden) does not exist; vclcompat's fails silently.
void ProductionLog(AnsiString Message, bool bSaveToFile, AnsiString JamCode)    //JerryYang 20151225 Production log for SPIL 蘇州
{
    AnsiString sFileName;
    AnsiString Str;
    if(IniConfig.bO06SaveLogTimePeriod==false)                                  //JerryYang 20160217 沒開啟就不記Log
        return;
    GetTimeInfo();

    sFileName.sprintf("%s\\%s_%04d%02d%02d.logs", asProductionLogPath.c_str(), IniConfig.SocketHandlerID.c_str(), SystemYear, SystemMonth, SystemDate);
    if(FileExists(sFileName)==false)                                            //JerryYang 20160120 跨日時不重複記錄,先清掉Memo內容
    {
        fMain->MemoProductionLog->Clear();
    }

    if(JamCode=="")
        Str.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d --> %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, Message.c_str());
    else
        Str.sprintf("%04d-%02d-%02d %02d:%02d:%02d.%d %s --> %s", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec, SystemMSec, JamCode.c_str(), Message.c_str());
    fMain->MemoProductionLog->Lines->Add(Str);
    if(fMain->MemoProductionLog->Lines->Count>32768 ||                          //JerryYang 20160303 一天存一個log檔，1024->32768行
       Message=="Close" || bSaveToFile)
    {
        fMain->MemoProductionLog->Lines->SaveToFile(sFileName);
        if(fMain->MemoProductionLog->Lines->Count>32768)                        //JerryYang 20160303 一天存一個log檔，1024->32768行
        {
            fMain->MemoProductionLog->Clear();
        }
    }
}

// ---------------------------------------------------------------------------
//  AI(W906-D4) 20260928 (St02-E): the D4 seat -- golden TfMain::ChangeTesterConnect's Off-Line -> On-Line "forced back to
//  Operator" block (golden 906_0625_Steven main.cpp:12104-12131; 912 :12621-12648).  Caller: forms/fMain.cpp TfMain::
//  ChangeTesterConnect (:1148, declared on :1056).  Body: WebLogin.cpp W906_WebLoginForceOperator (wb_serve only), which installs
//  itself here at static init.  Defined HERE, not in fMain.cpp, so that the fMain.cpp lines can be held or dropped alone and
//  wb_serve still links; ht9045_db is in every link that has ht9045_forms (fMain.cpp already needs cMyDB.cpp NewRecordProcess;
//  every RESCAN group with ht9045_forms has ht9045_db).  0 = not installed = nothing happens (every ctest but WebLogin_ForceOperator).
// ---------------------------------------------------------------------------
void (*W906_WebLoginForceOperatorHook)() = 0;   // AI(W906-ELA-REV) 20260928: called from the main-loop thread (TfMain::ChangeTesterConnect); the body takes the FormJson lock
// AI(W906-D1D7) 20260928 (St02-E): the D1 / D2 / D3 / D6 / D7 seats, same reason and same place as the D4 one above
//   (LogObjects.h has the declarations).  Bodies: TesterComm/Handler/HandlerTesterConnect.cpp, installed by
//   W906_TesterConnectRulesInstall() from W906_TesterCommInit (wb_serve).  0 = not installed = today's port behaviour.
bool (*W906_CtcD1AccessHook)(bool bRemote) = 0;
bool (*W906_CtcD2IcRefuseHook)(int Mode, bool Msg) = 0;
bool (*W906_CtcD3ManualSortHook)() = 0;
void (*W906_CtcD6To2DSortHook)() = 0;
void (*W906_CtcD7AsmOnLineHook)() = 0;
// AI(W906-GB-P8) 20260928 (St02-E helper): the B1 seats -- golden this->Handle / HVisionWnd / SendMessage(HVisionWnd, WM_COPYDATA) for
//   TfMain::GetTTLState (Command.cpp), same reason and same place as the D-seats above (ht9045_db is in every link that
//   has ht9045_forms, so every link that has Command.cpp's ht9045_sm).  Declared in TesterComm/TesterWndSeat.h, included
//   here so the definitions are checked against it.  Installed by W906_TesterCommInit (TesterComm/Handler/TesterCommWiring.cpp,
//   wb_serve only).  0 = not installed = no send and the token fields keep their value (the port before B1).
#include "TesterComm/TesterWndSeat.h"
HWND (*W906_TesterHandlerWndHook)() = 0;
HWND (*W906_TesterBridgeWndHook)() = 0;
void (*W906_TesterSendToBridgeHook)(COPYDATASTRUCT* pcp) = 0;
// ---------------------------------------------------------------------------
//  AI(W906-STATEREC-TS) 20260928 (St02-E, laptop-approved claim): the State Record worker thread (cStateRecord.cpp
//  RunStateRecordJob :1418-1509) no longer calls RecordProcess itself -- since P4 that is the golden body (cMyDB.cpp:1893: the
//  global ExString + MyDBIProcess into the shared log objects), which races the tick thread.  The worker queues (S, S2) here
//  under a CRITICAL_SECTION and touches no Handler global; the tick thread (tools/wb_serve.cpp:4575, every tick) drains the queue
//  and calls the golden RecordProcess in order.  Latency <= one tick.  A line can only be lost if the process exits before the
//  next tick.  ctests don't run the wb_serve tick, so queued lines just stay in memory there.  Here (ht9045_db) because every
//  link that has cStateRecord.cpp has this file.  The queue object is never destroyed (the worker may still run at exit).
// ---------------------------------------------------------------------------
void RecordProcess(AnsiString S, AnsiString S2);   // cMyDB.cpp:1893 (golden); declared here without cMyDB.h (its default-argument clash)
namespace {
struct W906StateRecordLogQueue
{
    CRITICAL_SECTION cs;
    std::vector<std::pair<AnsiString, AnsiString> > items;
    W906StateRecordLogQueue() { InitializeCriticalSection(&cs); }
};
W906StateRecordLogQueue* const g_w906SrQueue = new W906StateRecordLogQueue();   // static init, before any worker thread exists
struct W906SrLock
{
    explicit W906SrLock(CRITICAL_SECTION* c) : cs(c) { EnterCriticalSection(cs); }
    ~W906SrLock() { LeaveCriticalSection(cs); }
    CRITICAL_SECTION* cs;
};
}  // namespace
void W906_StateRecordLogLater(AnsiString S, AnsiString S2)
{
    W906SrLock lock(&g_w906SrQueue->cs);
    g_w906SrQueue->items.push_back(std::make_pair(S, S2));
}
void W906_StateRecordDrainLog()
{
    std::vector<std::pair<AnsiString, AnsiString> > batch;
    {
        W906SrLock lock(&g_w906SrQueue->cs);
        batch.swap(g_w906SrQueue->items);
    }
    for (size_t i = 0; i < batch.size(); ++i)
        RecordProcess(batch[i].first, batch[i].second);
}

// =============================================================================
//  AI(W906-A-OEE-ASECL) 20260929 (St02-E): two small golden bodies that St01's FileRW/MainRecord.cpp calls (its S113 gates
//  :187 / :202 / :212 / :245).  ST01-M job (A), FROM_STEVEN §4 19:46.
//  Out-of-line member definitions, kept in this St02 file of ht9045_db (always linked with ht9045_forms), so the owners'
//  files barely change: forms/fLotInfo.h:2216 gains the SaveASECLTestLogInfo declaration (St01).  TfMesSystem::SetOEEState
//  is already declared-not-defined at forms/fMesSystem.h:686 (the laptop's GATE list); its body is its own block below,
//  added only with the laptop's OK.
//  The includes are here, not at the file head, so no line above moves (LogObjects.cpp:101 / :107 / :130 / :241 / :288
//  are cited elsewhere).
//  Both are customer-only after their first statement (VTEST / ASE-CL).  The guard is live; the rest is golden VERBATIM
//  behind an S25 gate (RULINGS_20260925 S25: 客戶專屬條件先暫時跳過，註記就好), with what it would still need noted.
// =============================================================================
#include "forms/fLotInfo.h"

// golden 906_0625_Steven uLotInfo.cpp:10840-10967 (V912 :11021-)
void TfLotInfo::SaveASECLTestLogInfo()
{
    if(CUSTOMER_CODE!=CC_ASE_CL)                                                //JerryYang 20230204 : add
        return;

#if 0   // GATE (S25-ASECL): customer-only past the guard -- golden :10841-10849 (its locals, only used below) and
        //   :10852-10966 VERBATIM.  It writes D:/HT9045/system/ASECL_SUMMARY.ini and <asSaveEventLogPath>/TEST SUMMARY_*.csv.
        //   It would also still need TfLotInfo::aBackTestSummaryFile (golden uLotInfo.h:1323).  Everything else it uses
        //   exists in the port: edtASECL_LotID / cbbASECL_LoginMode / edtASECL_TesterID / slASECLTestInfor
        //   (forms/fLotInfo.h), UploadEventLogFile (St02 A5), TastCategory (cSocket.cpp:174), iPerMinuteNumberofError
        //   (cMyDB.h:141), TestSocket, LastSet.bUseTestSocket, WriteIniDataNoLog / ReadIniData / MyForceDirectories.
    AnsiString Target, S, Target1;
    FILE *Fp;
    AnsiString str, str1, str3;
    int iCT=0;
    AnsiString aTestCT="", aLotName, LotInName;
    AnsiString aTestDate, aTestTime, aRecipe, aHandler, aTesterID;
    static int iTotalOld=0, iPassOld=0, iFailOld=0;                             //KaiChen 20200316 :Add
    int iTotal=0;                                                               //KaiChen 20200316 :Add
    AnsiString sPathName;

    sPathName.sprintf("D:\\HT9045\\system\\ASECL_SUMMARY.ini");
    if(FileExists(sPathName)==false)                                            //檢查檔案
    {
        WriteIniDataNoLog(sPathName, "SUMMARY",    "iTotalOld",      0);
        WriteIniDataNoLog(sPathName, "SUMMARY",    "iPassOld",       0);
        WriteIniDataNoLog(sPathName, "SUMMARY",    "iFailOld",       0);
    }

    iTotalOld   =ReadIniData(sPathName, "SUMMARY",    "iTotalOld",   iTotalOld);
    iPassOld    =ReadIniData(sPathName, "SUMMARY",    "iPassOld",    iPassOld);
    iFailOld    =ReadIniData(sPathName, "SUMMARY",    "iFailOld",    iFailOld);

    MyForceDirectories(asSaveEventLogPath);
    aLotName  = edtASECL_LotID->Text;
    LotInName = cbbASECL_LoginMode->Text;
    str.sprintf("TEST SUMMARY_%s_%04d_%02d_%02d", IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate);
    Target = asSaveEventLogPath+"\\"+str+".csv";

    if(FileExists(Target)==false)
    {
        Fp=fopen(Target.c_str(),"a+");
        if(Fp==NULL)
            return;
        str1.sprintf("SiteID,ProjectCode,TesterID,DATE,TIME,LOT NAME,LOGIN MODE,INPUT,PASS,FAIL,ERROR#,Socket Usage,Socket Usage#\n");
        fputs(str1.c_str(), Fp);
    }
    else
    {
        Fp=fopen(Target.c_str(), "a+");
        if(Fp==NULL)
            return;
    }

    if(aBackTestSummaryFile!=AnsiString(str))
    {
        UploadEventLogFile(aBackTestSummaryFile);
        aBackTestSummaryFile=AnsiString(str);
    }

    slASECLTestInfor->Clear();
    TastCategory.UpdataCount(true);                                             //Steven 20250514 : 統一計算數量
    iTotal=TastCategory.iTotalSocket-iTotalOld;                                 //KaiChen 20200316 :Add

    aRecipe     = fMain->cbSetupFileName->Text;                                 //JerryYang 20190702 fix log
    aHandler    = IniConfig.SocketHandlerID;
    aTesterID   = edtASECL_TesterID->Text;
    aTestDate   = Now().FormatString("yyyy/mm/dd");
    aTestTime   = Now().FormatString("hh:nn:ss");

    slASECLTestInfor->Add(aHandler);                                            //SiteID
    slASECLTestInfor->Add(aRecipe);                                             //ProjectCode
    slASECLTestInfor->Add(aTesterID);                                           //TesterID
    slASECLTestInfor->Add(aTestDate);                                           //DATE
    slASECLTestInfor->Add(aTestTime);                                           //TIME
    slASECLTestInfor->Add(aLotName);                                            //LOT NAME
    slASECLTestInfor->Add(LotInName);                                           //LOGIN MODE
    slASECLTestInfor->Add(TastCategory.iTotalSocket);
    slASECLTestInfor->Add(TastCategory.iPassSocket);
    slASECLTestInfor->Add(TastCategory.iTotalSocket-TastCategory.iPassSocket);

    if(iTotal<0)
    {
        slASECLTestInfor->Add(TastCategory.iTotalSocket);
        slASECLTestInfor->Add(TastCategory.iPassSocket);
        slASECLTestInfor->Add(TastCategory.iTotalSocket-TastCategory.iPassSocket);
    }
    else                                                                        //KaiChen 20200316 :Add
    {
        slASECLTestInfor->Add(TastCategory.iTotalSocket-iTotalOld);
        slASECLTestInfor->Add(TastCategory.iPassSocket-iPassOld);
        slASECLTestInfor->Add(TastCategory.iTotalSocket-TastCategory.iPassSocket-iFailOld);
    }
    iTotalOld   =TastCategory.iTotalSocket;
    iPassOld    =TastCategory.iPassSocket;
    iFailOld    =TastCategory.iTotalSocket-TastCategory.iPassSocket;
    WriteIniDataNoLog(sPathName, "SUMMARY",    "iTotalOld",      iTotalOld);
    WriteIniDataNoLog(sPathName, "SUMMARY",    "iPassOld",       iPassOld);
    WriteIniDataNoLog(sPathName, "SUMMARY",    "iFailOld",       iFailOld);

    slASECLTestInfor->Add(iPerMinuteNumberofError);                             //Error

    for(int i=0; i<TestSocket.iShtRow; i++)
    {
        for(int j=0; j<TestSocket.iShtCol; j++)
        {
            if(LastSet.bUseTestSocket[0][i][j]==true)
            {
                if(i==0 && j==0)
                    aTestCT=aTestCT+"1";
                else
                    aTestCT=aTestCT+"1";                                        //JerryYang 20190625 fix log
                iCT++;
            }
            else
            {
                if(i==0 && j==0)
                    aTestCT=aTestCT+"0";
                else
                    aTestCT=aTestCT+"0";                                        //JerryYang 20190625 fix log
            }
        }
    }

    slASECLTestInfor->Add(iCT);                                                 //Socket CT
    slASECLTestInfor->Add(aTestCT);                                             //Socket

    fputs(slASECLTestInfor->CommaText.c_str(), Fp);
    fputs("\n", Fp);                                                            //JerryYang 20190625 fix log
    fclose(Fp);

//    TastCategory.iTotalSocket = 0;
//    TastCategory.iPassSocket  = 0;
    iPerMinuteNumberofError   = 0;
#endif
}

#include "forms/fMesSystem.h"   // AI(W906-A-OEE-ASECL) 20260929 (St02-E): TfMesSystem::SetOEEState below (declared forms/fMesSystem.h:686)
#include "Config.h"             // IniConfig.bVTESTFunction

// golden 906_0625_Steven Mes/fVATMesFileSys.cpp:3223-3260 (V912 :3227-3264)
void TfMesSystem::SetOEEState(int iState)
{
    if(IniConfig.bVTESTFunction==false)
        return;

#if 0   // GATE (S25-VTEST): customer-only past the guard -- golden :3228-3259 VERBATIM.  It would also still need
        //   TfMesSystem::WriteOEEState (golden :3214-3221, which calls AddOEEState, GATE W-17), TfMesSystem::iOEEStateChangeCnt
        //   (golden fVATMesFileSys.h:215), TfLotInfo::rgOEEState (golden uLotInfo.h:796) and TfLotInfo::labOEEState (golden uLotInfo.h:794,
        //   read and written by WriteOEEState :3216-3219).  FT is in the port (cprod.cpp:84 / cprod.h:3252); iRunStartMode is cmydef.h:3115.
    switch(iState)
    {
        case stStartTime:
            if(fLotInfo->rgOEEState->ItemIndex>1)
                return;

            if(iRunStartMode==FT)
                WriteOEEState("State : Running");
            else
                WriteOEEState("State : Retest");
            fLotInfo->rgOEEState->Enabled=false;
            break;
        case stPauseTime:
            if(fLotInfo->rgOEEState->ItemIndex>=1)
                return;

            WriteOEEState("State : Stop");
            fLotInfo->rgOEEState->Enabled=true;
            break;
        case stJamTime:
            if(fLotInfo->rgOEEState->ItemIndex>=1)
                return;

            WriteOEEState("State : Jam");
            fLotInfo->rgOEEState->Enabled=false;
            break;
    }

    iOEEStateChangeCnt++;                                                       //RogerYang 20250529 加入保護，曾發生狀態莫名其妙一直切換導致LOG暴增
    fLotInfo->rgOEEState->ItemIndex=0;
    if(iOEEStateChangeCnt>0)
        iOEEStateChangeCnt--;
#else
    (void)iState;
#endif
}
// AI(W906-W143) 20261007 (St02-E): LogObjects.h -- golden fMain->slMNetLog for MNetLog.cpp (W-143); null before W906_CreateLogObjects / after W906_DestroyLogObjects.
TMyStringList* W906_MNetLogObj() { return fMain != nullptr ? fMain->slMNetLog : nullptr; }
// AI(W906-W150) 20261007 (St02-E): LogObjects.h:27 -- W-150 slice 1 (skill hpi-mnetlog-split s7): the golden TfMain log objects for callers outside main.cpp; null before W906_CreateLogObjects.
TMyStringList* W906_TimeDataLogObj()   { return fMain != nullptr ? fMain->slTimeData      : nullptr; }   // golden cMyDB.cpp:491-492
TMyStringList* W906_ProdRecordLogObj() { return fMain != nullptr ? fMain->slProdRecordLog : nullptr; }   // golden cMyDB.cpp:559-560
TMyStringList* W906_TestLogObj()       { return fMain != nullptr ? fMain->slTestLog       : nullptr; }   // golden cObserver.cpp:2228, cprod.cpp:2909-2910
TMyStringList* W906_JamAlarmLogObj()   { return fMain != nullptr ? fMain->slJamAlarmLog   : nullptr; }   // golden note.cpp:6777 (TfNote::SaveErrEventLog: not translated yet, forms/fNote_ShowError.cpp N-7)
TMyStringList* W906_TorqueLogObj()     { return fMain != nullptr ? fMain->slTorqueLog     : nullptr; }   // golden rs232.cpp:1783 (+ BarCode.cpp / OCR.cpp / rs232.cpp :3250 / :3899 callers, not translated yet)
TMyStringList* W906_TorqueLogNewObj()  { return fMain != nullptr ? fMain->slTorqueLogNew  : nullptr; }   // golden rs232.cpp:4513 / :4599 (TCOM2::TimerHPCardTimer, not translated yet)
// AI(W906-W150) 20261008 (St02-E): LogObjects.h:27 -- W-150 slice 2 (skill hpi-mnetlog-split s7): null before W906_CreateLogObjects / after W906_DestroyLogObjects.
TMyStringList* W906_TTLLogObj()               { return fMain != nullptr ? fMain->slTTLLog            : nullptr; }   // golden 0618 cpublic.cpp:510 (TTLLog -> TTLLog.cpp)
TMyStringList* W906_IndexYMaxMinShiftLogObj() { return fMain != nullptr ? fMain->slIndexYMaxMinShift : nullptr; }   // golden 0618 cpublic.cpp:1589-1596 (LogIndexMaxMinPos -> IndexPosLog.cpp), mymotor.cpp:5515 (InitialMaxMinValue, machine side)
TMyStringList* W906_QtyLogObj()               { return fMain != nullptr ? fMain->slQtyLog            : nullptr; }   // golden V912 main.cpp:15458-15648 (S11 Lot End qty, JsonBridge/actions/MainClarnData.cpp QtyLog)
// AI(W906-W175) 20261008 (St02-E): LogObjects.h:27 -- golden fMain->slLowYieldAlarm (main.h:1487 TStringList*; `new TStringList()` in the TfMain ctor, golden 0618
//   main.cpp:1642, so it exists from the start and is never null; golden deletes it at FormClose :12003 -- here it lives for the process).
//   Not a log: the low-yield alarm codes ("AlarmCode,ErrPart") DoLowYieldAlarm adds (atester_ProcessCount.cpp:344-346), CleanOut reads (cCleanOut.cpp:315-321)
//   and DoOneCycleFinishCheck shows and clears (csystem.cpp W7C2_FMAIN_SLLOWYIELD :4750-4756 / :5087-5093).  Not tied to W906_CreateLogObjects.
vclcompat::TStringList* W906_LowYieldAlarmList() { static vclcompat::TStringList* const p = new vclcompat::TStringList(); return p; }
// AI(W906-W150) 20261008 (St02-E): LogObjects.h:27 -- W-150 slice 4: TempCtrl/TriTemp.cpp's W7TT_TMyStringList stand-in (:330) forwards here,
//   forms/fLotInfo.cpp W906_slLotInfolog (:6639) prefers fMain->slLotInfolog.  Null before W906_CreateLogObjects / after W906_DestroyLogObjects.
TMyStringList* W906_TriTempDoorLogObj()   { return fMain != nullptr ? fMain->slTriTempDoorlog : nullptr; }   // golden 0618 main.cpp:1621-1624; TriTemp.cpp :1950 ... :3269
TMyStringList* W906_DewPointLogObj(int i) { return (fMain != nullptr && i >= 0 && i < 3) ? fMain->slDewPointLog[i] : nullptr; }   // golden 0618 main.cpp:1626-1640; TriTemp.cpp :149 / :163
TMyStringList* W906_LotInfoLogObj()       { return fMain != nullptr ? fMain->slLotInfolog : nullptr; }       // golden 0618 main.cpp:1616-1619; uLotInfo.cpp:1996-1997
