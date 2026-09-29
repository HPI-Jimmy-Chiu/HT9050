// ===========================================================================
//  cMyDB.h  -- GA-1-B4 translation (sqlite-backed logging/query wrapper layer)
//  Golden ref: D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618\cMyDB.h (+ cMyDB.cpp,
//              2,047 lines).  Golden header banner (kept as semantic comment,
//              Big5 -> UTF-8):
//    MyDBC : Creat Table
//    MyDBI : Insert Record into Table
//    MyDBU : Update Record into Table
//    MyDBD : Drop Table or Delete Record
//    MyDBQ : Query Data from Table
//    MyDBV : View All Record in Table
//
//  BCB6 preamble dropped per migration plan (no portable meaning):
//    #include <Grids.hpp>   -> vclcompat/StringGrid.h  (TStringGrid)
//    #include <Chart.hpp>   -> NOT ported (TeeChart VCL component, W7-UI scope).
//                              `class TChart;` stays an opaque forward decl so
//                              the two report-chart signatures below remain
//                              declarable; their BODIES are gated in cMyDB.cpp
//                              (see MyDBVUnitEventCount / MyDBVAxleEventCount
//                              there, and the GA-1-B4 report for detail).
//
//  ============================================================================
//  HOMECOMING NOTICE -- DONE (AI(W906-CMYDB-P4) 20260927 (St02-E); Steven P4: D1=A, D3=A)
//  ============================================================================
//  Four golden cMyDB.h/.cpp symbols used to have a real, externally-linked,
//  non-golden-faithful STAND-IN body elsewhere in this tree.  P4 deleted all of
//  them in ONE commit and lifted the four #if 0 homecoming gates in cMyDB.cpp:
//
//    MyDBIProcess    (3-arg, __fastcall) -- was SECSGEM/uHGemEquipment.cpp:3480
//                        (FastcallFix forwarder to the 2-arg sink); the golden
//                        body is live: golden 906_0625_Steven cMyDB.cpp:789-855
//    MyDBIProcessNew (4-arg)             -- was canary_support.cpp:537 (record +
//                        stdout); the golden body is live: golden :724-787.
//                        D3 = A: NO __fastcall here, in canary_support.h:300
//                        or on the body (golden has it; the three V906
//                        spellings are kept consistent instead -- see the
//                        AI(W906-FCAudit) note below).
//    RecordProcess   (2-arg, S2 default) -- was canary_support.cpp:117 (stdout);
//                        the golden body is live: golden :1564-1573
//    NewRecordProcess(3-arg)             -- was acatchtray_shims.cpp:152 (empty);
//                        the golden body is live: golden :1545-1562 (its Greatek
//                        branch calls fProductionInfo->SaveMessageHistroy, D4)
//
//  The 2-arg MyDBIProcess(S1, S2) (aHotPlateSubstrate.cpp, 228 call sites) is
//  NOT golden (golden 906 has only the 3-arg one, S2=""): D1 = A keeps it as an
//  ADAPTER that forwards to the golden 3-arg body as (S1, S2, "") -- the binding
//  a golden 2-argument call gets.  It keeps its W906_MyDBIProcess_* counter.
//  Two anonymous-namespace 3-arg folders (Public/MyProductionRecord.cpp:1154,
//  KYECFTP/FTPClient_Transfer.cpp:87) still fold (S1, S2, S3) into
//  (S1, S2+" : "+S3) before the adapter -- a recorded deviation.
//
//  Since P4 these bodies WRITE: EventLogTxt (slEventLog, when built), HANDLER
//  LOG_*.csv (asSaveEventLogPath), *_EventTracker.csv (as9045LogPath + ASE log),
//  the ProductionLog day file (O06) and the Greatek history csv.  ctests reach
//  them only through the env seams (docs/CMYDB_PORT_LEDGER.md; ctest
//  MyDB_P4_Containment).  A new caller includes THIS header, or canary_support.h
//  / acatchtray_shims.h -- never two of them in one TU (each gives these names
//  default arguments; declaring a default twice is ill-formed).
//  RecordChangeLogProcess never had a stand-in; it now reaches the golden
//  MyDBIProcess body instead of the old forwarder.  History: the GA-1-B4 report.
// ===========================================================================
#ifndef cMyDBH
#define cMyDBH

#include "vclcompat/vcl_compat.h"   // AnsiString, TStringList, TDateTime, Now(), ...
#include "vclcompat/StringGrid.h"  // vclcompat::TStringGrid
#include "vclcompat/Controls.h"    // vclcompat::TComboBox
#include "myTimer.h"               // TQPF_Timer

using vclcompat::TStringGrid;
using vclcompat::TComboBox;

// TeeChart VCL component (golden <Chart.hpp>) -- NOT ported, W7-UI scope.
// Opaque forward decl only; see the homecoming/gating notice above.
class TChart;

//---------------------------------------------------------------------------
// New Alarm/Log data insertion into tables
//---------------------------------------------------------------------------
int  __fastcall MyDBIEvent(AnsiString AlarmCode, int MotorID, int *AlarmID, int *UnitNo, int *AxleNo, int *Type, AnsiString *Message, AnsiString *UnitName, AnsiString asTemperature=" ", int bDuplicateErr=0, AnsiString errPart=" ", bool bDate=false, TDateTime date=0); //Chunghung 2012 0416 add date
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2=""); // golden cMyDB.h:20 -- body cMyDB.cpp (golden :789-855), live since P4 (see HOMECOMING NOTICE above)
// AI(W906-FCAudit) 20260818: __fastcall dropped -- golden cMyDB.h:21 has it,
// but this declaration, canary_support.h:300 and the body (cMyDB.cpp, live
// since P4) all spell it without; a TU that saw a fastcall declaration
// would reference a differently-mangled symbol that nothing defines.
// Steven P4 D3 = A (20260927): keep all three without __fastcall.  Probe:
// including cMyDB.h + canary_support.h in one TU errors either order, so no
// TU sees both declarations.  The vcl_compat.h FastcallFix rule is lockstep:
// flip all three together, or none.
void MyDBIProcessNew(AnsiString asTable, AnsiString AlarmCode, AnsiString S1, AnsiString S2=" "); //Steven 20161220 : Process加上Alarm Code -- body cMyDB.cpp (golden :724-787), live since P4; no __fastcall (D3 = A)
void __fastcall MyDBITotalLoader(int iLoader);
int  __fastcall MyDBITimeData(long StartTime, long HomeTime, long ContactTest, long PauseTime, long ProductTime, long JamTime, long PowerOn);
void __fastcall MyDBIUPH(int UPH);                                              //Steven 20190906 : Add UPH in EventLog
void __fastcall RecordTimeData(int iDataType);                                  //JerryYang 20180515 記錄Time data
void __fastcall MyDBIProductionData(AnsiString sAction);
//---------------------------------------------------------------------------
// Table update
//---------------------------------------------------------------------------
void __fastcall MyDBULotEndTime(AnsiString EndTime);
void __fastcall MyDBULotData(AnsiString TableName, TStringGrid *strGrid);
void __fastcall MyDBUEventRecover(int RowID, AnsiString Recovery, int StopTime);
void __fastcall MyDBUpdateDB();                                                 //所有資料庫更新事項

//---------------------------------------------------------------------------
// Database query
//---------------------------------------------------------------------------
void        __fastcall MyDBQLotData(AnsiString Query, TStringList *strList);    //將LotData的內容存成StringList並回傳
AnsiString  __fastcall MyDBQMotMess(int MotorID);                               //回傳MotorAlarm的Message
bool        __fastcall MyDBQAlarmCodeList();                                    //Steven 20170202 (wei): Fixed for Unknown Alarm Code
AnsiString  __fastcall MyDBQClearDT();                                          //回傳最近一次的資料清除時間
AnsiString  __fastcall MyDBQTotalLoader(AnsiString StartDateTime, AnsiString EndDateTime);
void        __fastcall MyDBQTimeData(AnsiString StartDateTime, AnsiString EndDateTime, int sgRow, TStringGrid *strGrid);

void GetJameCodeOfAxis(int iAxis, TComboBox *ComboBox);                         //Steven 20140221 : 根據Axis取出對應的Jam Code
AnsiString __fastcall GetMyDBIMessage(AnsiString AlarmCode);                    //Sam 20230218 : 新增用 AlarmCode 去找 Alarm Message 資料
//---------------------------------------------------------------------------
// Grid/report views
//---------------------------------------------------------------------------
int         __fastcall MyDBVEventFreq(AnsiString asQuery, TStringGrid *strGrid);//次數統計
int         __fastcall MyDBVAxleEventCount(int min, int max, AnsiString StartTime, AnsiString EndTime, TChart *Chart);//將結果直接畫到Chart上 -- BODY GATED (no TChart port, see file head)
int         __fastcall MyDBVUnitEventCount(AnsiString asQuery, TChart *Chart);  // BODY GATED (no TChart port, see file head)
int         __fastcall MyDBVProcess(AnsiString asQuery, TStringGrid *strGrid);  //取得所有按鍵的紀錄
int         __fastcall MyDBVProcessFilter(AnsiString asQuery, TStringGrid *strGrid);  //過濾掉Duplicate的訊息
void        __fastcall GetAlarmCodeList(TStringGrid *strGrid);                  //Steven 20200331 : Alarm code list改用文字檔
//---------------------------------------------------------------------------
// Misc
//---------------------------------------------------------------------------
void        __fastcall MyDBVACUUM();                                            //壓縮資料庫
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");//Steven 20161220 : Process加上Alarm Code -- body cMyDB.cpp (golden :1545-1562), live since P4
void RecordProcess(AnsiString S, AnsiString S2="");                             // body cMyDB.cpp (golden :1564-1573), live since P4
void RecordChangeLogProcess(AnsiString S, AnsiString S2="");                    //wei 20180625 offset Change log紀錄 -- NO existing stand-in; translated ACTIVELY
void __fastcall MyDBOpenDB();
void __fastcall MyDBCloseDB();

void __fastcall SaveEventLogInfo(AnsiString aAlarmCode, AnsiString aMess, int iType, AnsiString aStatus, AnsiString sErrPart="");   //Steven 20200116 : 從uLotInfor改到cMyDB, 避免uLotInfo被解構後沒辦法存取
extern AnsiString aBackEventLogFile;
extern AnsiString aBackEventLogMessage;
extern AnsiString asEventTrackerFile;
extern AnsiString asEventTrackerMsg;
extern TQPF_Timer tEventLogTimer;
extern int iPerMinuteNumberofError;
extern int iRecordEventLogUPH;
extern AnsiString ExString;

#define iAlarmUnitTotal 32                                                      //Steven 20231127 : 整理Alarm Unit
extern AnsiString AlarmUnit[iAlarmUnitTotal];
extern AnsiString AlarmUnitNo[iAlarmUnitTotal];
//---------------------------------------------------------------------------
//  AI(GA1-B4) 20260804: golden cMyDB.h ALSO declares an overload set
//  `MyDBULotInfo()` / `MyDBULotInfo(TStringGrid*)` (golden cMyDB.h:30-31) that
//  has NO definition anywhere in the entire golden tree (verified: whole-tree
//  binary grep for "MyDBULotInfo" across every .cpp/.h under
//  HT9011UC_Code_V3.33.906.0_20260618 hits ONLY this header's own two
//  declaration lines -- zero call sites, zero bodies).  It is dead/vestigial
//  API in golden itself, not a translation gap.  NOT carried forward here --
//  declaring a function with intentionally no definition anywhere would only
//  invite a future "why is this unimplemented" bug report for something golden
//  never implemented either.
//---------------------------------------------------------------------------
#endif
