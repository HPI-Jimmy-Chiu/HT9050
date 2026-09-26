// =============================================================================
//  csystem.h  --  HT9045 system-coordinator INTERFACE (frozen as W6 contract)
//
//  Mirrored from: HT9011UC_Code_V3.33.906.0_20260618/csystem.h
//  Translation wave: W6.0 (freeze the cross-arm decoupling contract)
//  Translator: AI(W6.0-SCAFFOLD) 20260626
//
//  PURPOSE
//  -------
//  csystem.h is FROZEN as the W6 INTERFACE.  The load-bearing part this wave
//  locks is the cross-arm IC-presence PREDICATE API (the ~15 HasIC-family free
//  funcs at "PREDICATE API" below) plus HasICUnderMachine / HasAutoICInMachine /
//  HasAnyICInMachine -- these are the SM-to-SM decoupling contract: state
//  machines never cross-call each other's Do* functions; they only consult
//  these predicates.
//
//  This header is MIRRORED VERBATIM from the golden (every declaration kept, in
//  the same order).  The DEFINITIONS are split:
//    * csystem_predicates.cpp implements ONLY the HasIC family (this wave).
//    * every other body stays in csystem.cpp gated #if 0 // TODO(W6.x/W7).
//  A header costs nothing, so all interface decls stay live.
//
//  Big5 note: golden line 14 (GetMainProcCallCount comment) was already Big5-
//  corrupt in the source (mixed-encoding bytes); it is glossed to ASCII here.
//  Every other Chinese comment decodes cleanly via cp950 and is preserved as
//  UTF-8.  NO U+FFFD is emitted.
//
//  Includes added (golden relied on MachineDefine.h having pulled these first):
//    cmydef.h  -> MAX_AUTO_TRAY (MachineType.h), and the global type universe.
//    myTimer.h -> TQPF_Timer (DoCloseHeadterDelay / hTorqueDelay / ...).
//    vcl_compat.h -> AnsiString / TDateTime.
// =============================================================================
#ifndef csystemH
#define csystemH

#include "vclcompat/vcl_compat.h"   // AnsiString, TDateTime
#include "myTimer.h"                // TQPF_Timer
#include "cmydef.h"                 // MAX_AUTO_TRAY (via MachineType.h) + globals

//  ---- PREDICATE API marker (see lines ~"InputShuttleFrontHasIC" below) ----

#define StationCount  3
//---------------------------------------------------------------------------
void ClearATKBackup();                                                          //AI(ht9045-atk-amr-flow) 20260427 (RogerYang) : clear ATK backup fields
void WriteATKLog(AnsiString sMsg);                                              //AI(ht9045-atk-amr-flow) 20260427 (RogerYang) : ATK file log
void DoTestHeadMotor();
bool CheckIndexIsNormal();
//JerryYang 20260513 : Index arm Z axis safe position check
bool IsAllSuckKitTrayIsEmpty();
void CheckLotEnd();
void MainProc();
unsigned int GetMainProcCallCount();                                            //JerryYang 20260414 : add MainProc call-count log (golden csystem.h:14 comment was Big5-corrupt in source; glossed to ASCII)
TDateTime GetMainProcLastEnterTime();
AnsiString GetMainProcLastEnterTimeString();
double GetMainProcSilentSeconds();
bool IsMainProcAlive(int iTimeoutSec);
void ResetSCK_OEECount();
extern int iDoInArmPitchCHKTask;
extern int iDoOutArmPitchCHKTask;
extern int iReTestStartTask;
extern int iClearSocketFunctionTask;
extern int ReceiveICTrayTask;
extern int CatchTrayTask;
extern int iCatchNewTrayFromBufferTask;                                         //kevin 20120905
extern bool fCatchTrayPrepareToCatch;
extern bool fSendICTrayToBuffer;
extern int iReceiveAutoTrayTask[MAX_AUTO_TRAY];

void InitReceiveICTrayTask();
bool DoReceiveICTray();

void InitialCatchTrayTask();
void DoCDAAir();
void InitCleanOutFunction();
// W7-C1: the REAL faithful body lives in csystem.cpp (golden csystem.cpp:14713-
// 15748). Declared here so callers (the MainProc/DoAllProcess spine + the W7-C1
// verify) bind the single ODR definition. AI(W7C1-Integrate) 20260629.
void DoCleanOutFinishCheck();
// W7-C2: the REAL faithful body lives in csystem.cpp (golden csystem.cpp:12813-
// 14047). Declared here so callers (the MainProc spine + the W7-C2 verify) bind
// the single ODR definition -- the no-op stub in csystem_shims.cpp was removed.
// AI(W7C2-Integrate) 20260701.
void DoOneCycleFinishCheck();
// W7-C2: DoART_AfterCleanOut REAL body (golden csystem.cpp:14049-14711) lives in
// csystem.cpp; declared here so DoCleanOutFinishCheck's call binds to it (the
// W7C1 seam alias was removed). AI(W7C2-Integrate) 20260701.
bool DoART_AfterCleanOut(int &ret);
void InitOneCycle(AnsiString sFunc, bool bQAModeTrayEnd=false);
void TrayEndFunction();
void InitTrayEndFunction();
bool CheckMotorHome();
extern int iInitialStartTask;
void InitAllProcessTask();
void InitDoArmZHome();
bool DoArmZHome();

bool CheckOutArmSuckICFallDown();                                               //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
bool CheckInArmSuckFromLoaderICFallDown();                                      //Steven 20110516 : 修改成整合式Alarm
bool CheckInArmSuckICFallDownToHasNullIC(bool bAlarm=true);                     //Steven 20110516 : 整合只叫一次  //JerryYang 20200422 修正掉料可能檢查不出來的問題, 檢查到有掉料要重新再確認全部吸嘴
bool CheckIndexSuckICFallDownSetToHasNullIC(int iArm);                          //Steven 20110131 : 整合成只叫一次 //Steven 20190904 : 新增for 32site, BTestSuck只能輸入BTestSuck
bool CheckInArmDestroyICFail();                                                 //Steven 20111223 : 檢查破壞錯誤
bool IsInArmCleanOutFinish(int iIsOneCycle=0);
bool IsOutArmCleanOutFinish();
bool IsInArmOneCycleFinish();                                                   //Steven 20131029 : 解決Index Position Error
bool HasICUnderMachine();
bool HasAutoICInMachine();
AnsiString sHasICUnderMachine();                                                //Steven 20250110 : 顯示哪個位置還有IC
AnsiString sHasAnyICInMachine();                                                //Steven 20250110 : 顯示哪個位置還有IC
extern bool HasAnyICInMachine();                                                //kevin 20150914 判斷是否還有tray在 機台
bool HasICUnderHotPlate();                                                      //Steven 20110826
bool MemoryBug();
void CheckSafeDoorForICFallDown();
extern void ScanTrayStatus();
extern bool NewScanTrayStatus(int i);                                           //Ifor 20170405 (wei) add New Scan Tray Status
extern bool ScanFixTrayStatus();                                                //kevin 20160310 判斷OPEN 有設定bin fix tray 有無TRAY
extern void ScanColorFixTrayStatus();                                           //Steven 20141019 : 按下Start才檢查Fix盤上的Color Tray
extern bool SetShuttleMode(bool bAlarm);                                        //Steven 20231103 : for NN mode

bool IsSafeLockCheck();
void DoAvoidIndexMotorFallDown();
bool IsIndexMotorOutOfPower();
void LockIndexMotorAndDoHomeProcess();
void DoHeaterOn();
void ShowICFallDownASlarmMessage(AnsiString S1, AnsiString S2);

void CheckATC6System();                                                         //ChungHung 20141024 add

bool InputShuttleFrontHasIC();
bool InputShuttleRearHasIC();
bool InputShuttleHasIC();
bool OutputShuttleFrontHasIC();
bool OutputShuttleRearHasIC();
bool OutputShuttleHasIC();
bool SortShuttleHasIC();                                                        //RogerYang 20250506 Add for 9046AU
bool ShuttleHasIC();
bool FrontTestHeadHasIC();
bool RearTestHeadHasIC();
bool TestHeadHasIC();
bool TestSocketHasIC();
bool IndexHasIC();
bool IndexHasRealIC();                                                          //Steven 20240217 : Fixed for one cycle / clean out hang up
void IndexMotorBreakerON();
void IndexMotorBreakerOFF();
void MagazineBreakerON();                                                       //JerryYang 20220909 : add magazine
void MagazineBreakerOFF();                                                      //JerryYang 20220909 : add magazine
bool IsSystemPowerOff();
bool IsEMGPressed();
void SetInitialICCheck();
bool DoStepShuttleCheck(int iShuttle);

void DoMotorPowerOn();
void ProcessStatrDigital();
bool HasAreaOverAmbientTemp(int iIndex=0);                                      //kevin 20121224
extern int InArmpitchCHKPos[4];
extern int OutArmpitchCHKPos[4];
bool CheckContactOver();

//bool ArmYCanSuck2IC();                                                          //針對1x2_4 跑 2x2 NN mode
//bool ArmXCanSuck2IC_9045S();
//bool ArmXCanSuck4IC_9045();
//bool ArmXCanSuck4IC_1032();
bool ArmCanSuck4IC(int Direct, bool flag=true);                                 //Steven 20120106 : True = OutArm
bool HisiWorkName(AnsiString sName);                                            //kevin 20180627  海思工作檔命名
bool XPitchIsStand();
bool YPitchIsStand();
bool InSHT1InLF();
bool InSHT2InLF();
bool InShtInLF(int iSht);
bool InSHT1InRT();
bool InSHT2InRT();
bool InShtInRT(int iSht);
bool SortSHTInLF();                                                             //RogerYang 20250510 Add for 9046AU
bool SortSHTInRT();
bool SortShtSureInLF();
bool SortShtSureInRT();
//bool InSHT1InBSRT();
//bool InSHT2InBSRT();
bool OutSHT1InLF();
bool OutSHT2InLF();
bool OutSHT1InRT();
bool OutSHT2InRT();
void ProcessCCDLight();
void ResetHotTime();

void InitialCheckShuttle1SlotTask();
void InitialCheckShuttle2SlotTask();
void InitialCheckShuttle1MustHasICTask();
void InitialCheckShuttle2MustHasICTask();
bool DoCheckShuttle1MustHasIC();
bool DoCheckShuttle2MustHasIC();

bool DoCheckShuttle1EmptyIC();                                                  //kevin 20161108 IN SHUTTLE 退出 檢測是否有殘留IC
bool DoCheckShuttle2EmptyIC();                                                  //kevin 20161108 IN SHUTTLE 退出 檢測是否有殘留IC
void InitialCheckShuttle1ProminentNoHasICTask();                                //kevin 20161108 add for ASE_KH
void InitialCheckShuttle2ProminentNoHasICTask();                                //kevin 20161108 add for ASE_KH

extern bool bHeaterDoorIsOpen[4];
void HeaterDoorIsOpen();

void ShowIndexTime(int Item=0);
void ShowTestTime();

bool CheckAutoHasTray(bool bInitial);                                           //jou 2010-01-25 start : 確認Auto流道上是否有tray盤

void BackupFixTrayData(int TrayIndex);                                          //Steven 20100205 : 暫存Fix資料
void ReadFixTrayBackupData(int TrayIndex);                                      //Steven 20100205 : 讀取Fix的資料
void ClearFixTrayBackupData(int TrayIndex);                                     //Steven 20100205 : 清空Fix的暫存資料

bool CheckIndexAllSuckICFallDown(bool bCheckArm1, bool bCheckArm2);             //Steven 20110725 : 獨立出來
void AutoTrayCylinderFree();                                                    //jou 2010-01-25 start : 釋放Auto Tray上的汽缸

void DoSiteMappingResult();                                                     //jou 2011-03-24 start : Auto Site Mapping
bool DoInArmAutoSiteMapping();
void GetInArm2DIDMapping();                                                     //JerryYang 20250220 : 2DID硬體順序檢查功能
void DoAutoSiteMappingDropError();                                              //Ifor 20210524 add Auto Site Mapping Drop掉料處理
void DoSiteMappingCHK(bool bAdd=true);
void DoJCETSiteMappingCHK(bool bAdd);

void ResetMNet(int iRingNo, AnsiString EngMessage, AnsiString ChtMessage,bool bShowMess);
void DoHotplateEdgeCylinderLoop();                                              //jou 2011-08-09 start : Hotplate也要敲敲敲
void DoLoaderVibrateLoop();                                                     //JerryYang 20191001 loader震動馬達

int CheckPCI_L112State();
bool DoSwCoolingFan(bool OnOff);                                                //20111130  Dell
int CheckIndexConnect();                                                        //20111130  Dell
extern TQPF_Timer DoCloseHeadterDelay;

bool CheckSafeDoorIsClosed();
bool LowTempIdleCheckSafeDoor(void);                                            //Steven 20191016 : For ATC3.3
bool DoServoOn();

void OpenDUTHeat(bool Open);                                                    //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆 ,only for 9046LS
bool HasAreaOverAmbientTemp_DUT(double dTemp);
//------------------------------------------------------------------------------
bool CanChangeData(bool bCleanOut);                                             //kevin 20150525 判斷機台上是否有料可以更改資料 bCleanOut=fale 不包括 LOAD及HOTPLATE
extern bool bScanColorTray;                                                     //Steven 20141019 : 按下Start才檢查Fix盤上的Color Tray

//---------------------------------------------------------------------------
bool IdleCheckSafeDoor(void);                                                   // 2015.01.15 , Joye , Safe door check
bool IdleCheckSafeDoorByCylinder(int iPort , int iBit=-1);                      // 2015.01.15 , Joye , Safe door check
bool IdleCheckSafeDoorByCylinder(int Ring, int IP, int Port, int Bit=-1);       //Steven 20230703 : Add for MotionNet IO check SafeDoor

extern void RecordSafeDoorStates();                                             //Steven 20100129 : Record when door opened
extern bool AllArmZIsSafe();
extern bool DetectAutoTray(int Pos, int *iRetryCnt);                            //JerryYang 20170907 (Steven) Auto Tray偵測 整理為函式
void InitNewFixTray(int i, AnsiString Str, int iAction=0);                      //Steven 20160414 : 整合Fix盤設定
void ClearFixTray(int i, AnsiString Str, int iAction=0);                        //Steven 20160414 : 整合Fix盤設定
extern bool AutoTrayReCheck(int iWhich);                                        //Ifor 20171031 (Steven) add Auto Tray ReCheck

bool bCheckWinWayATCSite();                                                     //Jimmychiu 20210902 add: ATC Winway
void SocketAirCoolingStart();                                                   //jou 2016-04-28 Socket Air Cooling contact count trun on
void SocketAirCoolingEnd();                                                     //jou 2016-04-28 Socket Air Cooling contact count trun on
void SocketAirCoolingClear();                                                   //jou 2016-04-28 Socket Air Cooling contact count trun on
bool DoCheckShuttle1Prominent();                                                //Alick 20160815 add for 力成 確認IC放置時是否有突出
bool DoCheckShuttle2Prominent();                                                //Alick 20160815 add for 力成 確認IC放置時是否有突出
void InitialCheckShuttle1ProminentTask();                                       ///Alick 20160815 add for 力成 確認IC放置時是否有突出
void InitialCheckShuttle2ProminentTask();                                       ///Alick 20160815 add for 力成 確認IC放置時是否有突出
void SendDataToASE(AnsiString Data);                                            //kevin 20161228 add
extern void SaveFile(AnsiString sPath,AnsiString sData);                        //kevin 20170520 (wei) 記錄 LOG
bool CheckSafeDoor_1();                                                         //KaiChen 20180321 ：矽格-湖口 Chamber Light On By Open Any Door
bool CheckFixTraySensor();                                                      //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
bool CheckFixTray();                                                            //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
void AutoSiteMappingCheckShuttle(bool bWhich32);                                //Ifor 20180116 (Steven) : add Auto Site Mapping Check iWhich Shuttle
bool CheckSetupFinish();                                                        //JerryYang 20180921 Setup Teach功能
void DoBoostFunctionStepCooling();
bool ReadESDDataFile();                                                         //kevin 20180907 SECS READ ESD DATA
bool WriteESDDataFile();                                                        //kevin 20180907 SECS Write ESD DATA
int SaveAllFile(AnsiString SaveFileNm);                                         //kevin 20181109 add Save all File
extern void DoLockLoader();                                                     //JerryYang 20190115 避免Loader tray定位汽缸同時作動會造成跳料
extern void CheckBundleAndRead();
extern void InitialDoLockUnloader(int iAuto);
extern bool DoLockUnloader(int iAuto);                                          //JerryYang 20190115 避免Loader tray定位汽缸同時作動會造成跳料
extern void InitialDoLockLoader();
void DoTestHeadMotorLoadCell();                                                 //kevin 20190306 load cell test
bool TemperatureStorageLog(int iRecord=0);                                      //kevin 20190323 add Steven 20140617 : for 海思
int CheckHotGun();                                                              //kevin 20190621 流量偵測
int CheckHotGunFlow(int iGun);                                                  //KaiChen 20190729 ：Hot Gun Flow
extern bool Check32siteOnlyEnabled24Site();
extern bool Check32siteOnlyEnabled4x4Site();
extern int iLockLoaderTask;
extern AnsiString sHisiAddTempLog(bool bTemp);                                  //kevin 20191016 ASE KH HIS add TEMP LOG
bool beKeepLotNumber();                                                         //kevin 20191018 ASE KH lot ID LOG
bool bBuildFilter(AnsiString aFilePath,AnsiString aModeName);                   //kevin 20191106
void Del_Tree(AnsiString Dir);                                                  //kevin 20191111 add 刪除資料夾檔案
extern int iLockUnloaderTask[MAX_AUTO_TRAY];
bool EmptySocketCheckModeBeUse();
bool RunStartLowSpeedBuzzer(bool bReset);                                       //kevin 20201116 add run 發出警告
bool DoIndex4AxisHome(bool bReset=false);                                       //Isaac 20201012 : index Y超過範圍，做一次Tmode，indexArm四軸先回home
extern TQPF_Timer hTorqueDelay;                                                 //KaiHuang 20201222
void CheckTorqueDelay();                                                        //KaiHuang 20201222
extern TQPF_Timer hAutoCleanHangUp;                                             //Steven 20220702 : 針對Auto Clean的Hang Up偵測
extern bool CheckTTLBoardBitMode();                                             //Isaac 20210309 :TTL RS232兩塊板子
extern void SendTTLRS232CSOTsignal();                                           //Isaac 20210309 :TTL RS232兩塊板子
void ReadWriteBinCountMode(bool bRead);                                         //kevin 20210825 Bin 1  Bin 2...記錄
extern bool CheckNozzleEventFinish();                                           //Steven 20210825 : 吹氣完成才可以歸零
void ReadWriteTrayID(bool bRead);                                               //kevin 20220618 wirite Tray ID
extern bool DoAutoDecayCheck();                                                 //Ifor 20220112 add:Auto Decay 流程整合

extern TQPF_Timer hLotStartTimeOut;                                             //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
extern TQPF_Timer hLotEndTimeOut;
extern bool CanYieldAlarmRemainInSHT();                                         //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
int WhichMagazineBufferIsFull();                                                //JerryYang 20220909 : add magazine
bool MagazineBufferIsEmpty(int iMag);                                           //JerryYang 20220909 : add magazine
bool CheckAllMagazineBufferIsEmpty();                                           //JerryYang 20220909 : add magazine
void CheckMagazineBufferNeedClear();                                            //JerryYang 20220909 : add magazine
bool IsTriTempFixDoorLockCheck();                                               //Ztex 2023.04.19 Add HT-1032 TriTemp Function
bool bCheckPLCConnet();                                                         //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門
bool bCheckPLCAllSafedoorAndEMGEnable();
void InitDoInShZHome();
bool DoInShZHome(int iShuttle);
void InOutArmZBreakerON();                                                      //add One sucker with rotate
void InOutArmZBreakerOFF();                                                     //add One sucker with rotate
void CheckEmptyColorTrayAfterTrayEnd();                                         //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
void LDCarRotArmZBreakerOn();                                                   //RogerYang 20250828 add for Loader Rotate Arm
void LDCarRotArmZBreakerOFF();                                                  //RogerYang 20250828 add for Loader Rotate Arm
void CassetteBreakerON();                                                       //Ifor 20251216 add:Boat Carrier
void CassetteBreakerOFF();                                                      //Ifor 20251216 add:Boat Carrier
extern void bScanLoadPortState_Analog(int iPos);
extern void bScanUnLoadPortState_Analog(int iPos);
extern void bScanFixPortState_Analog(int iPos);
extern void CheckAllAutoTrayEjectFinsh();                                       //JerryYang 20251020 : 渠梁半清機功能
//===========================================================================
typedef struct
{
     AnsiString IP_ADDR[StationCount];
//    AnsiString MachName;
    int     Port[StationCount];
    int     ScanInterval;
    int     Alarm[StationCount][3];
    int     ProxOnOff[StationCount][3];
    bool    bStationActive[StationCount];

    double  Max_DecayTime[StationCount];                                        //Ifor 20150916 SIMCO Decay Time Setting //Ifor 20151123 Simco Decay 單位修改 int => double
    double  Min_DecayTime[StationCount];
    double  Decay_Interval[StationCount];

    int     Decay_WaitingTime;                                                  //Ifor 20150925 新增 Decay Test 延遲秒數
    int     iESDReporTimer;                                                     //Ifor 20160308 新增 ESD Report Data 秒數

    bool    DecayProxOnOff[StationCount][3];

    int     HandlerType;                                                        //0: D:\HT9045  1: D:\newtesthander

    int     Alarm_Continuous_Time;                                              //Ifor 20150831 :新增Alarm允許時間與次數
    int     Alarm_Occurrences;
    int     Alarm_StartTime[StationCount][3];
    int     Alarm_HappenCount[StationCount][3];
    bool    Alarm_FirstTime[StationCount][3];
    bool    Alarm_Enable;
    int     Alarm_MaxOccurrencesCount;                                          //Steven 20161104 : 最大Alarm發生次數

    int     Decay_Permissible_Time;                                             //Ifor 20151020 :新增Decay 離子消散允許時間

    int     Kasuga_ScanInterval;                                                //Ifor 20150729 Kasuga 變數新增
    int     Kasuga_Alarm;
    int     Kasuga_Alarm_Value;
    int     Kasuga_Alarm_Count;
    bool    bKasugaActive;
    bool    bKasugaAlarmReport;
    AnsiString Kasuga_IP_ADDR;
    bool    bSendDataToCustomer;                                                //kevin 20151028 傳送ESD資料給客戶
    int     iCustomerPort;                                                      //kevin 20151028 Servo Port
    AnsiString   PassWord;                                                      //kevin 20160408
    bool    Remot;                                                              //kevin 20170310 (Steven) 離子BAR
    bool    SvaveFilebyDay;                                                     //kevin 20180816 存記錄 一天存一筆
}ESD_GENERAL_SET;                                                               //ESD_GENERAL.ScanInterval
extern ESD_GENERAL_SET ESD_GENERAL;                                             //kevin 20180907 add ESD Data

//==============================================================================
//  AI(W906-PT-W5-g1) 20260809 -- declarations for the eight csystem.cpp bodies
//  this wave's group g1 lands that golden does NOT declare in csystem.h and that
//  NO port header declared either (verified 2026-08-09: a whole-tree grep of all
//  14,172 headers found zero declarations for any of the eight).  Golden reaches
//  them through cmydef.h / its own file-local externs -- e.g.
//  atester_32Site.cpp:421 and asortarm.cpp:757 each carry their OWN
//  `extern ...` line for CheckTestSuckICOn / CheckSuckInitialStatus, which is
//  exactly the symptom of a missing shared declaration.  Declared HERE because
//  csystem.h is the header this group is allowed to edit and because these are
//  csystem.cpp bodies; wording is golden's own definition signature verbatim.
//  DEVIATION FROM GOLDEN, stated plainly: golden puts none of these eight in
//  csystem.h.  Nothing already declared in this header is touched.
//  `class TMyKitSuck &` is golden's own elaborated-type-specifier form (golden
//  csystem.cpp:2465/:2493/:2522) -- it forward-declares ::TMyKitSuck without
//  committing this header to either of the tree's two TMyKitSuck definitions
//  (aHotPlateSubstrate.h / mykitsuck.h), so it cannot pick the wrong layout.
//  NO default arguments are introduced by any line below.
//==============================================================================
bool DoIonFanAutoClean();                                                       //Isaac 20210609 : IO觸發IonFan清針
bool CheckTestSuckICOn(class TMyKitSuck &Ptr, int iR, int iC);
int  CheckSuckInitialStatus(class TMyKitSuck &Ptr, int iR, int iC);
int  CheckTestSuckStatus(class TMyKitSuck &Ptr, int iR, int iC);                //Sam 20221019 : 矽格北興 Contact 模式初始檢查吸嘴上有 IC 就報警。
bool CheckLoaderSafeDoor();
bool CheckUnLoadSafeDoor();
bool CheckFixTraySafeDoor();
bool CountMotorPowerDelay();

//==============================================================================
//  AI(W906-MT-E3b) 20260925 -- engine side of Motor Test / Teach (EastSun rulings R8/R9 20260925).
//  PORT-ONLY names (golden has none of these); bodies in csystem.cpp.  Rationale per item at the
//  definitions.  All hook pointers start NULL; ctest never registers them, and every engine call
//  site is NULL-safe, so ht9045_sm links and behaves as before without the wb_serve side.
//==============================================================================
// Hook pointers the wb_serve side registers (WebMotorAccessLive.cpp or tools/wb_serve.cpp):
//   W906_Stop1203AllHook(why)     -- stop every 1203 axis the monitor opened.  Called right after the
//                                    golden StopAllMotor() in TfHome::GaliMotorServoOff and in
//                                    VerifyMotorAction (golden StopAllMotor cannot reach those axes).
//   W906_FormFShowHook(form)      -- "is this golden form's web window open" (fMotorTest / fTeach);
//                                    register ht9045::WebWindowRegistryFShowPolicy.
//   W906_MotorHomingHook()        -- a web HOME job is running (golden fTeach/fMotorTest->btnHome->Down).
//   W906_MotorMovingHook(mi)      -- -1: not a hook-owned motor (golden test runs); 0: not moving;
//                                    1: moving.  Must claim every 1203 axis (golden MotionDone on a
//                                    TMyEtherCatMotor here is always false == "moving").
//   W906_MotorAllBtnUpHook(why)   -- golden fTeach->AllBtnUp()+fMotorTest->AllBtnUp()+bSingleHome=false:
//                                    end the web jobs whose golden buttons those raise.
// THREADING: every caller of these hooks (MainProc -> VerifyMotorAction, DoSystem ->
//   CheckMotorPowerShutDown, MainProc's stop arm -> TfHome::sbAbortHomeClick, TfHome::GaliMotorServoOff)
//   runs on the single tick thread; the hooks are called there and must not block.  Register them once,
//   on the tick thread, before the first MainProc() of the pump.  W906_GetMotorLockState() is written
//   there too, so read it only from tick-thread producers (the /api/struct bodies are).
extern void (*W906_Stop1203AllHook)(const char* why);
extern bool (*W906_FormFShowHook)(const char* goldenForm);
extern bool (*W906_MotorHomingHook)();
extern int  (*W906_MotorMovingHook)(int motorIndex);
extern void (*W906_MotorAllBtnUpHook)(const char* why);

//AI(W906-MT-FIX1) 20260926: EastSun ruling 20260926 on G16 -- "對應軸 Servo On 才放" (release a brake group only while the
//   axes it holds are servo-ON).  golden releases the Index / Magazine / In-Out-Arm Z / Loader-rotate Z / Cassette brakes
//   without looking at the drives; golden could, because its start-up InitMotor servo-ONs every axis.  Here the 1203 axes
//   belong to EastSun's monitor and nothing servo-ONs them by itself.
//   W906_BrakeServoOnHook(group, why)  -- registered by the wb_serve side (WebMotorAccessLive.cpp): true = every enabled
//        PCI1203 axis of that brake group is opened by the monitor and its SVON bit is on; false + why otherwise.
//        group is one of "Index", "Magazine", "InOutArmZ", "LDCarRotArmZ", "Cassette".
//   W906_BrakeReleaseOK(group, site)   -- guard in front of every golden *BreakerON() release (G16 CountMotorPowerDelay,
//        G05 DoSystem idle pass, uhome HOME): hook NULL (ctest) -> true (golden); else false while bMotorPowerState is
//        false (motor power just cut -- SnMotorPower lags) or the hook says no.  Prints once per group on each change.
extern bool (*W906_BrakeServoOnHook)(const char* group, AnsiString* why);
bool W906_BrakeReleaseOK(const char* group, const char* site);

// member || W906_FormFShowHook(goldenForm)  (same union as WebStart.cpp's W906_FShow)
bool W906_FormFShow(const char* goldenForm, bool member);

// VerifyMotorAction's screen state (golden fMotorTest/fTeach->LockAllButton + fTeach->labLock),
// for the runtime "lock" block.  locked: MoveN/MoveP/LoopMove disabled + pnlStop yellow (golden
// LockAllButton(true)); false: pnlStop 0x00DFD9CC.  labLock*: golden fTeach->labLock Visible/Caption.
struct W906MotorLockState
{
    bool          locked;
    bool          labLockVisible;
    AnsiString    labLockCaption;       // "*Lock by Homeing" / "*Lock by M03 moveing" (golden spelling)
    unsigned long unlockCount;          // times the lock ended (door open or 2 s without motion)
    AnsiString    lastUnlockWhy;
    W906MotorLockState() : locked(false), labLockVisible(false), unlockCount(0) {}
};
const W906MotorLockState& W906_GetMotorLockState();

// Cross-tick form of DoMotorPowerOn() (golden csystem.cpp:19102-19126), same outputs and same 1 s:
// call Begin() once, then Step() once per tick until it returns true (== golden's `break`).
void W906_DoMotorPowerOnBegin();
bool W906_DoMotorPowerOnStep();
#endif

