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
//  HOMECOMING NOTICE (read before adding a caller of any of these four names)
//  ============================================================================
//  Four golden cMyDB.h/.cpp symbols ALREADY have a real, externally-linked,
//  non-golden-faithful STAND-IN body living elsewhere in this tree, landed by
//  earlier waves that had no cMyDB.cpp to home them in yet:
//
//    MyDBIProcess    (3-arg, __fastcall) -> SECSGEM/uHGemEquipment.cpp:3475
//                        (task-brief-specified homecoming; FastcallFix wave,
//                        20260720)
//    MyDBIProcessNew (4-arg)             -> canary_support.cpp:453
//                        (discovered independently this pass; W7-L2 wave,
//                        20260803.  NOTE: golden declares this WITH __fastcall
//                        -- canary_support.h:396's declaration and
//                        canary_support.cpp:453's definition do NOT have
//                        __fastcall.  This is the SAME mismatch class the
//                        FastcallFix wave fixed for MyDBIProcess itself
//                        [vclcompat/vcl_compat.h:24-54] -- flagged here, not
//                        fixed here: canary_support.{h,cpp} are outside this
//                        wave's write boundary.  Because this header keeps the
//                        golden (fastcall) spelling for faithfulness AND my own
//                        definition stays gated #if 0, and nothing active in
//                        cMyDB.cpp calls MyDBIProcessNew, the mismatch is
//                        latent/undisturbed by this file -- not exercised.)
//    RecordProcess   (2-arg, S2 default) -> canary_support.cpp:102
//                        (discovered independently this pass; W7-L2 wave,
//                        20260803; signature/callconv already consistent with
//                        golden -- no fastcall mismatch)
//    NewRecordProcess(3-arg)             -> acatchtray_shims.cpp:132
//                        (discovered independently this pass; signature/
//                        callconv already consistent with golden)
//
//  My faithful golden-body translations of all four sit in cMyDB.cpp wrapped
//  in `#if 0 // TODO(GA1-B4-integrate): homecoming from <file> -- integrator
//  swaps`.  DO NOT remove those #if 0 guards without ALSO deleting the stand-in
//  body at the file named above -- doing both at once is the "homecoming swap"
//  (see the GA-1-B4 report for the exact steps + nm verification command).
//  RecordChangeLogProcess has NO existing stand-in anywhere in the tree, so it
//  is translated ACTIVELY (not gated) below/in the .cpp.
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
void __fastcall MyDBIProcess(AnsiString asTable, AnsiString S1, AnsiString S2=""); // see HOMECOMING NOTICE above -- real body lives in SECSGEM/uHGemEquipment.cpp today
// AI(W906-FCAudit) 20260818: __fastcall dropped -- the LIVE definition
// (canary_support.cpp:457) has none, so this decl referenced a fastcall-
// mangled symbol nothing defines. Latent only today (the sole cMyDB.h-side
// caller, cMyDB.cpp:1817, sits inside the NewRecordProcess homecoming gate;
// probe: including cMyDB.h+canary_support.h in one TU errors either order,
// so no TU ever saw both decls). Golden says __fastcall, but the vcl_compat.h
// FastcallFix rule is lockstep-with-definition; when the body homecomes here,
// keep both sides fastcall-free or flip both together.
void MyDBIProcessNew(AnsiString asTable, AnsiString AlarmCode, AnsiString S1, AnsiString S2=" "); //Steven 20161220 : Process加上Alarm Code -- see HOMECOMING NOTICE above (real body: canary_support.cpp)
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
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");//Steven 20161220 : Process加上Alarm Code -- see HOMECOMING NOTICE above (real body: acatchtray_shims.cpp)
void RecordProcess(AnsiString S, AnsiString S2="");                             // see HOMECOMING NOTICE above (real body: canary_support.cpp)
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
