// =============================================================================
//  Motor/mymotor.h  -- TMyMotor / TTrayMotor (PARTIAL W4 translation)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.h
//  Translation wave: W4 (HAL motor layer)
//  Translator: AI(W4) 20260626
//
//  PARTIAL TRANSLATION RATIONALE
//  --------------------------------
//  The full header pulls in two VCL-widget / vendor-SDK headers at global scope
//  that cannot compile without those subsystems:
//
//    1. HTray.h  (#include "elec/myvcl/HTray.h") = VCL TCustomControl subclass
//       (Forms.hpp / Controls.hpp / Classes.hpp).
//       ACTION (W0 form-pointer pattern): dropped include; forward-declared
//       `class TTMyTray` so TTrayMotor::pHTray compiles as a pointer.
//       The single method SetHTrayPanel(TTMyTray*) and data member pHTray are
//       gated with `#if 0 // TODO(W7-UI)`.
//
//    2. myGALILmotor.h  -- pulls Galil <System.hpp> (BCB/Delphi RTL).
//       ACTION: dropped include from header.  All Gali_* methods in TMyMotor
//       that were declared in the header are declared here but their bodies
//       in mymotor.cpp are gated `#if 0 // TODO(W6-Galil)`.
//
//  WHAT IS TRANSLATED AND ACTIVE (W4):
//    - All data members of TMyMotor (motor-routing, scale, lock map, timers).
//    - All motion-routing methods whose bodies need only HTMotor* +
//      already-translated globals (ReadPos, ReadEncoderPos, SetADCRate,
//      SetSpeed, GetSpeed, EnableMotorMove, InitMOTParameter, MotorInitial,
//      Home, HomeReset, MotorHome, GetErrorIndex, GetMotorAlarm,
//      ScanMotorStatus, PCIL132_SetPos/ResetPos/StopMotor, JogP, JogN,
//      ServoOnOff, IsCanMove, SetAlias, SetScreenScale, Lock/UnLock/
//      ClearLock/GetLockCount/GetLockString, SetPanel, SetArmMaxSpeed,
//      IsStartGali_Pr/StartGali_Pr/EndGali_Pr/GetGali_Pr_Result/
//      GetGali_Pr_ER/SetGali_Pr_ER -- these last 6 use Gali_Command which
//      itself is gated; included for signature completeness).
//
//  GATED (`#if 0 // TODO(wave)`):
//    - MotorMoveShuttleShake (still the W4 stub).  MotorMove / MotorMove2SpeedForPicker /
//      MotorMovePosition: LIVE golden bodies since AI(W906-C21-MOTORMOVE) 20260924 (mymotor.cpp tail).
//    - ServoOnOff gated portion (Gali_ branch -- needs myGALILmotor/DMCCommand).
//    - All ~80 Gali_* / Z1Z2 / ISZ / GalilTwoY / Find*Phase methods.
//    - MagazineUp / MagazineDown.
//    - GetRealPos / CompareEncoderPos / CompareCommandPos (encoder helpers
//      call MotorMovePosition internal state + CheckPos/sensor layer).
//    - CheckPos* / CheckYPos* / CheckIndexYPos / Check_Y1Y2* / Check_SHUTTLE_*
//      (Cylinder / Sensor layer; not yet translated).
//    - TrayArmInitial (cinitial / sensor coupling).
//
//  TTrayMotor:
//    - Translated faithfully except SetHTrayPanel / pHTray (VCL widget).
//    - TMyTray Tray member active; mytray.h NOT yet in target Motor/ dir --
//      include path points to project root (../../mytray.h from Motor/).
//      The CMakeLists include path covers this.
//
//  FREE FUNCTIONS / GLOBALS:
//    - extern TTrayMotor MOT[MAX_TRAY_MOTOR] -- active; defined in mymotor.cpp.
//    - All InArmContinuousMove_9045 / OutArm* / SortArm* / PCIL112_* / ZSafe*
//      free functions are declared (for link compatibility) but their bodies
//      are gated TODO(W6-state-machine) in mymotor.cpp.
//    - ZSafePos / ZlimitPos / bPauseSortMotor extern -- active.
// =============================================================================
#ifndef MyMotorH
#define MyMotorH

#include "Motor/HTMotor.h"

// mytray.h lives in project root (not Motor/)
#include "mytray.h"             // TMyTray
#include "cprod.h"              // ARM_CONDITION, INDEX_PRESS_TYPE, etc.
#include "myTimer.h"            // TQPF_Timer

// Forward-declare VCL-widget type so pointer members compile without HTray.h
class TTMyTray;                 // AI(W4) 20260626: was #include "HTray.h" (W7-UI)

// Galil header dropped from here (was #include "myGALILmotor.h").
// Gali_* method declarations retained for ABI completeness; bodies gated in .cpp.

#include <map>                  // std::map for mapLockList

#define MAX_DEVICES 100         // Sam 20230707: EtherCAT Shuttle sensor

// ---------------------------------------------------------------------------
// Forward declaration needed for TTrayMotor free-function parameter
// ---------------------------------------------------------------------------
class TTrayMotor;

// ===========================================================================
//  TMyMotor
// ===========================================================================
class TMyMotor
{
private:
    int  RefStart;
    int  RefEnd;
    int  FactStart;
    int  FactEnd;
    double  Scale;
    int  MotorMovePosition(int &Position, int speed, int Target);
    TQPF_Timer ResetTime;
    int iMyHomeTask;
    int iHomeRetryCT;
    TQPF_Timer htWaitHomeSensorOnDelay;
    bool bPanelUse;
    bool bUpDownMove;
    void *PWinCtrl;         // AI(W4) 20260626: was TWinControl* (VCL); gated W7-UI
    int iHomeCount;

protected:
    std::map<AnsiString, AnsiString> mapLockList;    // Steven 20210825: lock list
    std::map<AnsiString, AnsiString>::iterator mapLockIter;

    // Gali helper -- declared for ABI; body gated TODO(W6-Galil)
    AnsiString Gali_GetMOT(int MOT);

    // CheckPos / CheckYPos -- Cylinder/Sensor layer; gated TODO(W6-sensor)
    bool CheckPos(bool Front);
    bool CheckYPos();
    bool CheckPos_nnMode(bool bOpen);

private:
    TQPF_Timer DelayCT;
    int iEncoderCheckCT;
    TQPF_Timer tSh1FloodgateOpenTimeout;    // Ifor 20260401: Floodgate open timeout
    TQPF_Timer tSh2FloodgateOpenTimeout;    // Ifor 20260401: Floodgate open timeout
    bool bSh1FloodgateOpenWaiting;          // Ifor 20260401: Floodgate open timeout flag
    bool bSh2FloodgateOpenWaiting;          // Ifor 20260401: Floodgate open timeout flag

public:
    TMyMotor();
    ~TMyMotor();

    int HomeFlag;
    int Mot_Name;
    AnsiString Alias;
    AnsiString NumberAlias;
    void SetAlias(int iNo, AnsiString Name);

    HTMotor *Motor;
    int   speed;                // run speed
    bool  fCanMove;
    bool  fCanMoveR;
    bool  fCanMoveM;
    bool  fCanMoveL;   bool W906_HasMotorLocks() const { return mapLockList.size()!=0; }   // AI(W906-W4-MOTOR) 20260925: 唯讀存取子（port-only）—— motor.access 要照 MotorMove 的 fCanMove*/mapLockList 互鎖判斷，而 mapLockList 是 protected

    int  TargetPosition;
    int  Position;
    int  EncoderPosition;
    int  ScreenPos;
    int  OldSpeed;

    // AI(W4) 20260626: SetPanel takes void* instead of TWinControl*; the
    // bPanelUse/PWinCtrl fields are kept so MotorMove's panel-update branch
    // compiles; actual VCL Top/Left assignment is gated in MotorMove TODO(W7).
    void SetPanel(void *PCtrl, bool b);
    void SetScreenScale(int s1, int e1, int s2, int e2);
    void SetSpeed(double p, bool bSetJog = false);  // RogerYang 20250729: 9046AU

    void EnableMotorMove();

    // AI(W906-C21-MOTORMOVE) 20260924: MotorMove / MotorMove2SpeedForPicker = golden bodies
    // (mymotor.cpp tail); MotorMoveShuttleShake is still the W4 stub.
    int  MotorMove(int p);
    bool MotorMoveShuttleShake(int p);
    bool MotorMove2SpeedForPicker(int FinalPos, ARM_CONDITION *ARM, bool bIsLoader = false);

    void MotorInitial();
    void ScanMotorStatus();

    int  MotorHome(bool);
    int  ReadPos();
    int  GetSpeed();
    bool GetMotorAlarm();
    int  GetErrorIndex();

    bool Led[10];
    bool Home2Led;
    bool MovFlag;

    bool Home(void);
    int  HomeTask;
    void HomeReset();

    AnsiString CardType;            // RogerYang 20250411: ECAT Motor reinit
    // MagazineUp/Down: gated TODO(W6-sensor) -- declarations kept for ABI
    void MagazineUp();
    void MagazineDown();

    bool bIsFullIC;
    bool bIsEmptyIC;

    // -----------------------------------------------------------------------
    //  Galil-specific members -- declared for ABI; all bodies gated
    //  TODO(W6-Galil) in mymotor.cpp (requires myGALILmotor / DMCCommand)
    // -----------------------------------------------------------------------
    int  GailSpeed;
    bool Gali_MotorAlarm;
    void Gali_ScanMotStatusTIMO();
    void Gali_ScanMotStatus();
    void Gali_ScanAlarmStatus();
    bool Gali_MotMove(int Pos, int Speed, AnsiString _Func = "");
    bool Gali_MovePR(int Pos, int Speed);
    bool Gali_MotMove2(int Pos, int Speed, int SpeedDec);
    bool Gali_MotMoveNoWait(int Pos, int Speed, int iNeedDelayTime, bool bCheckZ = true);
    bool Gali_MotMoveSkipEncoder(int Pos, int Speed);
    bool Z1UpZ2Down1(int Speed);
    bool Z1DownZ2Up1(int Speed);
    bool Z1UpZ2Down2(int Speed, bool bPickErr);
    bool Z1DownZ2Up2(int Speed, bool bPickErr);
    bool Z1UpZ2Down(int Speed, bool TMode, bool bPickErr);
    bool Z1DownZ2Up(int Speed, bool TMode, bool bPickErr);

    long Gali_Command(AnsiString str, AnsiString sFunc = "");
    long Gali_ReadPos();
    long Gali_ReadEncoderPos();

    void Gali_MotHome(AnsiString HomeAxis);
    void Gali_MotHome_HighSpeed(AnsiString HomeAxis, int Speed);    // RogerYang 20250410: 9046AU
    void Gali_MotHomeFindZ(AnsiString HomeAxis);                    // ChungHung 20141016: find Z
    bool Gali_SingalHome(bool IndexZFirstHome = false);             // RogerYang 20161116
    bool Gali_FindZPhase();                                         // Ifor 20170817: Find Z Phase

    bool Gali_Two_ZAxis_Move(int Pos, int Speed, AnsiString sFunc = "", bool bTwoPos = false, int Pos2 = 0);
    void Gali_JogP(int Speed);
    void Gali_JogPSetup(int Speed);
    void Gali_JogPAndCount(int Speed, int Count);
    void Gali_JogN(int Speed);
    void Gali_JogNSetup(int Speed);
    void Gali_JogNAndCount(int Speed, int Count);

    int  IndexPickLimit;
    bool bScanFlag;
    int  GaliSofDelayCount;
    bool ISNormal();
    bool ISZ1Up_Z2Down();
    bool ISZ1Down_Z2Up();
    bool ISZ1Up_Z2DownNoWait();
    bool ISZ1Down_Z2UpNoWait();

    bool Gali_ReadEncoderInRandge(long checkpos);
    bool Gali_ReadEncoderInRandgeNoWait(long checkpos);
    bool Gali_ReadEncoderOver(long checkpos);
    bool Gali_ReadEncoderMaxRandge(long checkpos);
    bool Gali_ReadEncoderInRandgeMinLimit(long checkpos);
    int  iGali_SingalHomeTask;
    int  iGali_FindZPhaseTask[4];       // Ifor 20170817 / Isaac 20201110
    bool GalilTwoY_Move(int YPos, int Y2Pos, int Speed, AnsiString sFunc = "");
    bool IsCanMove();
    bool Gali_ReadEncoderBelowCheckHeight(long checkpos);

    bool Gali_nnMode_Z1Z2_Down(int Speed, bool TMode);     // 2012-10-22 Dell HT9046LS
    bool Gali_nnMode_Z1Z2_Up(int Speed, bool TMode);
    bool ISZ1Down_Z2Down();
    bool ISZ1Up_Z2Up();

    // GetRealPos / Compare* -- gated TODO(W6-sensor) -- declarations kept for ABI
    void GetRealPos(int *iPos);
    int  CompareEncoderPos(int iPos, int iGap);
    int  CompareCommandPos(int iPos, int iGap);

    int  iOldPos;
    bool fCMD;
    void PCIL132_SetPos(int Pos);
    void PCIL132_StopMotor();
    void PCIL132_ResetPos();
    void JogP(int Speed);
    void JogN(int Speed);
    void ServoOnOff(bool IsOn);
    int  ReadEncoderPos();
    int  iEncoderPos;
    void InitMOTParameter();
    void SetADCRate(int Scale);
    int  iCheckStatusCT;
    int  SimulateSpeed;
    bool GaliScanServo;
    void SetArmMaxSpeed();
    bool bZ1Z2Exute;
    bool bCheckEncoderEveryTime;
    bool bShowMotorMove;

    bool IsStartGali_Pr();
    void StartGali_Pr();
    void EndGali_Pr();
    long GetGali_Pr_Result();
    void SetGali_Pr_ER(long ERA, long ERB, long ERC, long ERD);
    void GetGali_Pr_ER(long &ERA, long &ERB, long &ERC, long &ERD);

    int  GetLockCount();
    void Lock(AnsiString MotorAlias, AnsiString FunctionName, int Task);
    void UnLock(AnsiString MotorAlias, AnsiString FunctionName);
    void ClearLock();
    AnsiString GetLockString(int Index);

    // Check* methods -- Cylinder/Sensor layer; gated TODO(W6-sensor) -- ABI only
    bool CheckYPosWhenZDown(int Pos, int iOrgPos, AnsiString asErrorFunc);
    bool CheckArmPosInRange(int iNowPos, int iMin, int iMax);
    bool CheckArmPosArrival(int iNowPos, int iDestination, int iTolerance);
    bool CheckIndexYPos(bool Front);
    bool Check_SHUTTLE_FLOODGATE_Staste(int iIndex);

    // TrayArmInitial -- gated TODO(W6-state-machine)
    void TrayArmInitial();

    bool Check_Y1Y2_TargetPosWillCrash(int iY1_RelPos, int iY2_RelPos);
    bool CheckY1Y2TargetPos(int Pos, AnsiString asErrorFunc);
    bool Check_Y1_TargetPosWillCrash(int iY1TarPos, AnsiString sFunc);
    bool Check_Y2_TargetPosWillCrash(int iY2TarPos, AnsiString sFunc);
    bool Check_Y1_TargetPosInTeachPos(int iY1TarPos, AnsiString sFunc);
    bool Check_Y2_TargetPosInTeachPos(int iY2TarPos, AnsiString sFunc);

    bool iLastRotatorDirP;                                      // RogerYang 20260113: Rotator
    int  GetRotatorBacklash(int iGoalPos, bool bInRotator, int iTechData = 0); // RogerYang 20260113
};

// ===========================================================================
//  TTrayMotor
// ===========================================================================
class TTrayMotor : public TMyMotor
{
private:

protected:
    bool     fHTary;
    // AI(W4) 20260626: pHTray gated -- TTMyTray is a VCL TCustomControl;
    // HTray.h not included.  The pointer and SetHTrayPanel are kept behind
    // #if 0 so existing bodies that test fHTary still compile (fHTary=false
    // in the W4 sim build).
#if 0 // TODO(W7-UI): restore when HTray.h / VCL TCustomControl is available
    TTMyTray *pHTray;
#else
    TTMyTray *pHTray;   // NULL in W4 sim build; HTray.h fwd-decl only
#endif

public:
    TTrayMotor();
    TMyTray  Tray;
    int      iDirection;        // Steven 20170302: FIFO MODE
    int      iSiteCount;        // Steven 20170302: FIFO MODE
    bool     bShowSiteMap;      // Steven 20170302: FIFO MODE
    bool     fHasTray;

    // AI(W4) 20260626: SetHTrayPanel signature kept; body is a no-op in W4
    // (pHTray stays NULL; all fHTary branches are skipped).
    // Restore real implementation when HTray.h is available (W7-UI).
    void SetHTrayPanel(TTMyTray *ptr);

    void InitNewTray(int data, bool bShowSiteMapFlag, AnsiString Func);
    void InitEmptyTray(AnsiString Func);
    void ClearTray(AnsiString Func);
    void SetTray(int data, AnsiString Func);

    void MoveTrayAllItem(class TTrayMotor *Source);     // Sam 20240108
    bool HasIC();
    bool HasRealIC();
    bool FullIC()                   { return Tray.FullIC(); }
    void SetTraySingleData(int x, int y, int data, int iTarget = 0);
    void SetTraySiteMap(int x, int y, int iSiteMap);    // Steven 20220510
    void SetTrayBinData(int x, int y, int data, AnsiString iInfo);
    void SetNullIcToHasNullIc();
    void SetNullIcToHasIc();        // Sam 20240424
    void SetTrayBufferSingleData(int x, int y, int data);

    bool UpHalfIsFull();
    bool DownHalfIsFull();
    int  WhichBufferIsFull();       // JerryYang 20221215: Magazine
    void Refresh();

    bool SearchHasEmpryToPlace(int iY);
    bool TrayFeedHasIC();
    bool HasCleanPad();
    int  HowManyDevice(int iType);
    int  HowManyDevice();
    void SetHasNullIcToNullIc();
    bool HasOnlyDataICAndNullIC(int DataType);
    void MoveTrayData(TTrayMotor &TrayMotor);           // JerryYang 20221215: Magazine
    int  iIsCoverTray;
    AnsiString sUnloaderAlarmMsg;                       // Jimmychiu 20240902
    AnsiString sTrayID;
};

// ---------------------------------------------------------------------------
//  Global motor array  (defined in mymotor.cpp)
// ---------------------------------------------------------------------------
#define MAX_TRAY_MOTOR 300
extern class TTrayMotor MOT[MAX_TRAY_MOTOR];

// ---------------------------------------------------------------------------
//  Free functions  -- declared for ABI; all bodies gated TODO(W6-state-machine)
//  in mymotor.cpp unless otherwise noted.
// ---------------------------------------------------------------------------
// ZSafe extern globals (defined in mymotor.cpp -- active)
extern int ZSafePos;
extern int ZlimitPos;
extern bool bPauseSortMotor;    // RogerYang 20250510: 9046AU

bool InArmContinuousMove_9045(int X, int Y, int Vari[X_PITCH_COUNT], int YVari,
                               bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col],
                               int  iZPos[MAX_ARM_Row][MAX_ARM_Col],
                               bool ZNeedDown, bool bLoader = false);
int  InArmPitchMove(int Vari[X_PITCH_COUNT], int YVari, bool bInit = false);
void SetInArmPitchSpeed(bool bMoveClose);
extern int iInArmZMoveTask;
bool InArmCynMove(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col]);
bool InArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col],
                    int  iZPos[MAX_ARM_Row][MAX_ARM_Col],
                    bool bLoader = false, bool bPreOn = false);
bool InArmZMoveUp(int iZPos, bool bLoader = false);

bool OutArmContinuousMove_9045(int X, int Y, int Vari[X_PITCH_COUNT], int YVari,
                                bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col],
                                int  iZPos[MAX_ARM_Row][MAX_ARM_Col],
                                bool ZNeedDown, bool bLoader = false);
bool OutArmPitchMove(int Vari[X_PITCH_COUNT], int YVari, bool bInit = false);
extern int iOutArmZMoveTask;
bool OutArmCynMove(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col]);
bool OutArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col],
                     int  iZPos[MAX_ARM_Row][MAX_ARM_Col],
                     bool bLoader = false, bool bPreOn = false);
bool OutArmZMoveUp(int iZPos, bool bLoader = false);

// RogerYang 20250510: 9046AU SortArm
bool SortArmContinuousMove(int X, int Y, int Vari,
                            bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col],
                            int  iZPos[MAX_ARM_Row][MAX_ARM_Col],
                            bool ZNeedDown, bool bLoader = false);
bool SortArmPitchMove(int Vari);
void InitPCIL112_SortArmXYMoveTask();
int  PCIL112_SortArmXYMove(int iXComPos, int iYComPos);
AnsiString sSortArmZHomeState();
bool SortArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col],
                      int  iZPos[MAX_ARM_Row][MAX_ARM_Col],
                      bool bLoader = false, bool bPreOn = false);
bool SortArmZMoveUp(int iZPos, bool bLoader = false);

bool CatchMgzTrayMove(int FinalPos);    // Sam 20221116: Magazine TrayArm

void OpenPCI132Card(bool bfirst);       // ChungHung 20111101
void ServoOnAllMOT();
int  OutArmZSafe(int iFlag);
int  InArmZSafe(int iFlag);
int  SortArmZSafe(int iFlag);           // RogerYang 20250510: 9046AU
AnsiString sInArmZHomeState();          // Sam 20230707
AnsiString sOutArmZHomeState();         // Sam 20230707

void InitPCIL112_InArmXYMoveTask();
int  PCIL112_InArmXYMove(int iXComPos, int iYComPos);
void InitPCIL112_OutArmXYMoveTask();
int  PCIL112_OutArmXYMove(int iXComPos, int iYComPos);

int  CheckOutArmZNeedHome();            // ChungHung 20140605
int  CheckInArmZNeedHome();
void ShowIndexMotorError(AnsiString Debug = "", bool bCompareTeachPos = false);

AnsiString SaveLog(AnsiString FileNamePath);   // kevin 20150909
void RecordIndexPositionError(AnsiString sFunction,
                               bool A1, bool A2, bool A3, bool A4,
                               long *Pos);
bool TrayArmContinuousMoveForOCR(int X, int Y);    // wei 20150923
bool TrayMoveHome();                                // wei 20150923
bool ShuttleSensorContinuousMove(int SH1, int SH2, bool Result);   // wei 20160914
int  CheckOutArmDestory();                         // kevin 20180119
void RecordIndexPosition(int iArm, int Part);      // Isaac 20200922
void EncoderTeachingMaxMinCount(int iRecordArm);   // Isaac 20201012
void InitialMaxMinValue(AnsiString StrTask = "");  // Isaac 20201012
void TrigerIndexAxisHome();                        // Isaac 20201012
extern bool TrayArmMotorMove(int p, bool bCheckPos = true); // Sam 20241206

// AI(W906-SIMSTEP) 20260924: 模擬 Galil 步進倍率 —— 定義在 Motor/myGALILmotor.cpp 檔尾（在那裡才確定看得到 SOFT_SIMULTE）。
int W906_SimGalilStepScale();

#endif // MyMotorH
