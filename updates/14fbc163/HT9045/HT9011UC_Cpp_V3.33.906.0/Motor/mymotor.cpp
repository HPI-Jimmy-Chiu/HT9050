// =============================================================================
//  Motor/mymotor.cpp  -- TMyMotor / TTrayMotor implementation (PARTIAL W4)
//
//  Translated from: HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp
//  Translation wave: W4 (HAL motor layer)
//  Translator: AI(W4) 20260626
//
//  ACTIVE (W4): ctor/dtor, SetAlias, IsCanMove, SetScreenScale, ReadPos,
//    ReadEncoderPos, SetADCRate, SetSpeed, GetSpeed, EnableMotorMove,
//    InitMOTParameter, MotorInitial, Home, HomeReset, MotorHome,
//    GetMotorAlarm, ScanMotorStatus, GetErrorIndex, PCIL132_SetPos/ResetPos/
//    StopMotor, JogP(int)/JogN(int), ServoOnOff (SMC/generic branch only),
//    SetArmMaxSpeed, IsStartGali_Pr/StartGali_Pr/EndGali_Pr/GetGali_Pr_Result/
//    GetGali_Pr_ER/SetGali_Pr_ER (use Gali_Command stub),
//    GetRealPos, CompareEncoderPos, CompareCommandPos, GetRotatorBacklash,
//    Lock/UnLock/ClearLock/GetLockCount/GetLockString,
//    SetPanel.
//    TTrayMotor: ctor, HasIC, HasRealIC, HasCleanPad, SetTrayBinData,
//    SetTraySingleData, SetTraySiteMap, SetNullIcToHasNullIc/HasIc,
//    SetTrayBufferSingleData, Refresh, InitNewTray (full), ClearTray,
//    InitEmptyTray, SetHTrayPanel, SetTray, UpHalfIsFull, DownHalfIsFull,
//    WhichBufferIsFull, MoveTrayAllItem, HowManyDevice(x2),
//    MoveTrayData.
//    Global array MOT[300], ZSafePos, ZlimitPos, bPauseSortMotor.
//
//  GATED `#if 0 // TODO(wave)`:
//    - (AI(W906-C21-MOTORMOVE) 20260924) MotorMovePosition / MotorMove /
//      MotorMove2SpeedForPicker are NO LONGER gated: golden bodies live at the FILE TAIL.
//    - (AI(W906-AMB-L2) 20260929) MotorMoveShuttleShake is NO LONGER a stub: golden :5198-5316 is live at :910.
//    - Gali_* method bodies (require myGALILmotor / DMCCommand).
//    - ServoOnOff Gali_ branch (INDEX_MOTION_CARD==0).
//    - InArmZSafe, CheckInArmZNeedHome, ShowIndexMotorError, CheckTestZ*,
//      IndexPosMonitor free functions (Sensor/Cylinder/VCL coupling).
//    - All InArmContinuousMove_9045 / OutArm* / SortArm* / PCIL112_* / ZSafe*
//      / Tray helper free functions (state-machine wave).
//    - GetRealPos Contec direction branch (uses MotionCard_Contec).
//    - MyMNetLine myLine[] (requires MyMNet/mn_open_all; MN200 band).
//
//  AI(W906-PT-W3-integrate) 20260808: the old "NOTE on MNetLog" that stood here
//  is now obsolete and has been removed with the stub it described -- the real
//  `bool MNetLog(AnsiString)` landed with Motor/myMN200motor.cpp (:2494, golden
//  :2146-2156) in PT-W3 and is declared by myMN200motor.h:192, which this file
//  now includes.  Same declaration golden itself sees here.
// =============================================================================

#include "MachineDefine.h"
#include "Motor/mymotor.h"
#include "MachineType.h"    // ChangeToFloatNonPcnt, MOTION_CARD_TYPE constants, etc.
#include "cpublic.h"
#include "cmydef.h"         // sIC_Type[], iYRegNum, bTestSiteUse[], etc.
// AI(W906-PT-W3-integrate) 20260808: golden mymotor.cpp:27 `#include "myMN200motor.h"`
//   -- restored, needed for MyMNetLine/MAXRing so golden's `myLine[MAXRing]` definition
//   below (golden :43) can live where golden puts it.  Same relative order as golden
//   (after cmydef.h, golden :19).
#include "Motor/myMN200motor.h"   // MyMNetLine, MAXRing (golden myMN200motor.h:78-92 / :96)

// ---------------------------------------------------------------------------
//  AI(W906-PT-W3-integrate) 20260808: RETIRED the local
//  `static void MNetLog(AnsiString) {}` stub that used to sit here.  It had to
//  go: myMN200motor.h:192 declares `extern bool MNetLog(AnsiString)` (golden
//  :192 verbatim), so once this file started including that header the two
//  declarations differ only in return type -- an ambiguating redeclaration, a
//  hard compile error, not a silent shadow.  The real body is
//  Motor/myMN200motor.cpp:2494 (golden :2146-2156), registered in ht9045_motor.
//  Behaviour note: that body is itself GATED (golden writes fMain->slMNetLog,
//  which has no port) so it does nothing and returns true unconditionally --
//  which is what every golden code path returns too (golden :2155).  The three
//  call sites in this file (:1199/:1222/:1244, golden :1200/:1447/:1477)
//  discard the result, so both the no-op behaviour and the void->bool signature
//  change are non-events for them.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Module-level constants / globals
// ---------------------------------------------------------------------------
#define RESET_TIMES 900

#define PNP_DONE    0   // order finished
#define PNP_DOING   1   // order in progress

int ZSafePos  =  20;        // Steven 20220207: In/Out Arm Z safe position
int ZlimitPos = -3200;      // Ifor 20221026:   In/Out Arm limit position
// AI(W906-PT-W3-integrate) 20260808: golden Motor/mymotor.cpp:43
//   `MyMNetLine myLine[MAXRing];  //Isaac 20181212 (Steven) : Baud Rate防呆功能`
//   -- restored, and the file-head DEFERRED list above amended accordingly.  It was
//   deferred as "requires MyMNet/mn_open_all", but that is wrong about the DEFINITION:
//   the array is plain storage, and it is Motor/myMN200motor.cpp (landed in PT-W3) that
//   needs the vendor SDK, not this line.  myMN200motor.h:181 declares it and
//   myMN200motor.cpp reads/writes it at :1208/:1209/:2449/:2451/:2453, so without this
//   definition that unit does not link.  Golden keeps it here, in exactly this position
//   (between ZlimitPos and the `extern bool CheckOutSuckICFallDown` declaration), so the
//   port keeps it here too rather than inventing a new home.
MyMNetLine myLine[MAXRing];                                                     //Isaac 20181212 (Steven) : Baud Rate防呆功能

bool bPauseInMotor   = false;
bool bPauseOutMotor  = false;
bool bPauseSortMotor = false;   // RogerYang 20250510: 9046AU

// PCIL112 XY move task counters (used by free-function stubs)
int iPCIL112_SortArmXYMoveTask = 1;    // RogerYang 20250512: 9046AU
int iPCIL112_InArmXYMoveTask   = 1;
int iPCIL112_OutArmXYMoveTask  = 1;

// InArm/OutArm Z move task indices (extern'd in mymotor.h)
int iInArmZMoveTask  = 1;
int iOutArmZMoveTask = 1;

// AI(W906-P0-6) 20260921: 這個樁**退休了** —— 真本體翻在本檔尾端
//   （golden Motor/mymotor.cpp:5662-5880，219 行）。
//   放尾端而不是原地，理由是相依：本體要 `Cylinder[]`（mycylin.h）與
//   `ShowMyMessage`（canary_support.h），這兩個 header 在本檔是 :2764 之後
//   才 include 的。golden 自己也把它放在檔尾（:5662）。
//
// ⛔ 它為什麼非翻不可：`{ return false; }` 讓 `uhome.cpp:3722` 的 `flag2`
//   永遠不成立 ⇒ 歸零 case 1310 每次逾時重來。
//   20260921 實測（spine_poll_probe --seconds 240）：5 個完整循環
//   `1→…→1310(x54)→1→…`，一次都沒到 1520。

// Encoder tolerances (module-local, matches original)
static const int iTorence = 10;
static int iEncoderTorence = 500;
static int iCheckZ         = 4000;

// Bit mask table (used by some Gali helpers; kept for completeness)
// Suppress unused-variable: table is referenced in gated TODO(W6-Galil) helpers.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static unsigned char bMask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#pragma GCC diagnostic pop

// Global motor array
class TTrayMotor MOT[MAX_TRAY_MOTOR];

// ---------------------------------------------------------------------------
//  GetScale helper (private to this TU; used by SetScreenScale / ctor)
// ---------------------------------------------------------------------------
static double GetScale(int s1, int e1, int s2, int e2)
{
    double S = (double)(s2 - e2);
    if (S == 0)
        return 1.0;
    double P = (double)(s1 - e1);
    return P / S;
}

// ===========================================================================
//  TMyMotor
// ===========================================================================

// ---------------------------------------------------------------------------
TMyMotor::TMyMotor()
{
    RefStart  = 2;
    RefEnd    = 0;
    FactStart = 4;
    FactEnd   = 0;
    fCanMove  = true;
    fCanMoveR = true;
    fCanMoveM = true;
    fCanMoveL = true;
    Scale     = GetScale(RefStart, RefEnd, FactStart, FactEnd);
    bIsFullIC  = false;
    bIsEmptyIC = false;
    MovFlag    = false;
    bPanelUse  = false;
    bUpDownMove = false;
    Gali_MotorAlarm   = false;
    GaliSofDelayCount = 0;
    bScanFlag         = false;
    iGali_SingalHomeTask = 1;
    mapLockList.clear();    // Steven 20210825: lock list

    // Isaac 20201110: Index Y find motor phase
    iGali_FindZPhaseTask[0] = 1;
    iGali_FindZPhaseTask[1] = 1;
    iGali_FindZPhaseTask[2] = 1;
    iGali_FindZPhaseTask[3] = 1;

    Mot_Name  = 0;
    fCMD      = false;
    iOldPos   = 0;
    bSh1FloodgateOpenWaiting = false;   // Ifor 20260401
    bSh2FloodgateOpenWaiting = false;   // Ifor 20260401
    iCheckStatusCT = 0;
    GaliScanServo  = true;
    iEncoderCheckCT = 100;
    bCheckEncoderEveryTime = false;
    bShowMotorMove = false;
    iLastRotatorDirP = true;            // RogerYang 20260113

    if (MOTION_CARD_TYPE == MotionCard_SYN)
    {
        iEncoderTorence = 1000;
    }
    else
    {
        iEncoderTorence = 1000;
    }

    if (INDEX_PRESS_TYPE == e160KG && CUSTOMER_CODE == CC_JCET)
    {
        iCheckZ = 11000;
    }
    // AI(ht9045-v899) 20260512: CASE-20260507-001 CC_GIGAS Z2 delay -> iCheckZ 11000
    else if (CUSTOMER_CODE == CC_GIGAS)
    {
        iCheckZ = 11000;
    }
    else if (INDEX_PRESS_TYPE == e85KG  || INDEX_PRESS_TYPE == e240KG ||
             INDEX_PRESS_TYPE == e120KG || INDEX_PRESS_TYPE == e160KG)
    {
        iCheckZ = 9000;
    }
    else
    {
        iCheckZ = 4000;
    }
}

// ---------------------------------------------------------------------------
TMyMotor::~TMyMotor()
{
    mapLockList.clear();    // Steven 20210825
}

// ---------------------------------------------------------------------------
void TMyMotor::SetPanel(void *PCtrl, bool b)
{
    bUpDownMove = b;
    bPanelUse   = true;
    PWinCtrl    = PCtrl;
}

// ---------------------------------------------------------------------------
void TMyMotor::SetAlias(int iNo, AnsiString Name)
{
    Alias   = Name;
    Mot_Name = iNo;
    NumberAlias.sprintf("[%02d] %s", iNo, Name);
}

// ---------------------------------------------------------------------------
bool TMyMotor::IsCanMove()
{
    if (fCanMove && fCanMoveR && fCanMoveM && fCanMoveL)
        return true;
    return false;
}

// ---------------------------------------------------------------------------
void TMyMotor::SetScreenScale(int s1, int e1, int s2, int e2)
{
    RefStart  = s1;  RefEnd   = e1;
    FactStart = s2;  FactEnd  = e2;
    Scale     = GetScale(RefStart, RefEnd, FactStart, FactEnd);
    ScreenPos = (int)(Scale * (Position - FactStart)) + RefStart;
}

// ---------------------------------------------------------------------------
int TMyMotor::ReadPos()
{
    if (Motor != NULL && Motor->Enable)
    {
        Position        = Motor->ReadPos();
        EncoderPosition = Motor->ReadEncoderPos();
    }
    else
    {
        EncoderPosition = Position;
    }
    ScreenPos = (int)(Scale * (Position - FactStart)) + RefStart;
    return Position;
}

// ---------------------------------------------------------------------------
int TMyMotor::ReadEncoderPos()
{
    if (Motor != NULL && Motor->Enable)
    {
        Position        = Motor->ReadPos();
        EncoderPosition = Motor->ReadEncoderPos();
    }
    else
    {
        EncoderPosition = Position;
    }
    return EncoderPosition;
}

// ---------------------------------------------------------------------------
void TMyMotor::SetADCRate(int Scale)
{
    double fPersent = 0.0;
    double Acc, Dec;

    if (Scale == 100)
    {
        Acc = Motor->GetAccDataBase();
        Dec = Motor->GetDecDataBase();
    }
    else
    {
        fPersent = double(Scale) / 100.0;
        if (CardType == "PCI1203")  // RogerYang 20250415: PCI1203
        {
            Acc = Motor->GetAccDataBase() * fPersent;
            Dec = Motor->GetDecDataBase() * fPersent;
        }
        else
        {
            Acc = ChangeToFloatNonPcnt((double)(Motor->GetAccDataBase()), (double)(fPersent));
            Dec = ChangeToFloatNonPcnt((double)(Motor->GetDecDataBase()), (double)(fPersent));
        }
    }
    Motor->SetAcc(double(Acc));
    Motor->SetDec(double(Dec));
}

// ---------------------------------------------------------------------------
void TMyMotor::SetSpeed(double p, bool bSetJog)
{
    int s;
    if (Motor != NULL && Motor->Enable)
    {
        if (Mot_Name == MTestY1 || Mot_Name == MTestZ1 ||
            Mot_Name == MTestZ2 || Mot_Name == MTestY2)
        {
            // Index axis -- Galil card controls speed directly; skip here
        }
        else
        {
            if (p >= 100)
                p = 100;

            if (Mot_Name == MLoaderZ || Mot_Name == MEmptyZ  || Mot_Name == MColorZ  ||
                Mot_Name == MAuto1Z  || Mot_Name == MAuto2Z  || Mot_Name == MAuto3Z  ||
                Mot_Name == MAuto4Z  || Mot_Name == MAuto5Z  || Mot_Name == MAuto6Z)
            {
                s = (Motor->PJogHighSpeed - Motor->PJogLowSpeed) * (int)p / 100 + Motor->PJogLowSpeed;
            }
            else
            {
                s = Motor->PJogHighSpeed * (int)p / 100;
            }
            Motor->SetSpeed(s, bSetJog);    // RogerYang 20250729
            speed = s;
        }
    }
    else
    {
        speed = (int)p;
    }
}

// ---------------------------------------------------------------------------
int TMyMotor::GetSpeed()
{
    if (Motor != NULL && Motor->Enable)
        return Motor->ReadSpeed();
    else
        return speed;
}

// ---------------------------------------------------------------------------
void TMyMotor::EnableMotorMove()
{
    fCanMove  = true;
    fCanMoveR = true;
    fCanMoveM = true;
    fCanMoveL = true;
}

// ---------------------------------------------------------------------------
void TMyMotor::InitMOTParameter()
{
    fCMD = false;
    GaliSofDelayCount = 0;
    bScanFlag = false;
    MovFlag   = false;
}

// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- MotorMovePosition
//  golden HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp:549-860  (312 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  AI(W906-C21-MOTORMOVE) 20260924: this gate stays as the verbatim reference.
//  The W4 stub that used to follow it is RETIRED in place; the LIVE body is now
//  golden's, translated at the FILE TAIL (see the C21 tail banner).
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN MotorMovePosition Motor/mymotor.cpp:549-860
int TMyMotor::MotorMovePosition(int &Position, int speed, int Tar)
{
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return -1;
    }

    int ret=0;
    int Pos=Tar;
    int iEncoder=0;
    AnsiString S1="", S2="", S3="";
    int iGap=2;                                                                 //Sam 20230621 : Gap容許誤差改為1>2 //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條

    if(Motor!=NULL &&                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->Enable)                                                           // has true motor
    {
        if(Tar>=Motor->PSoftLimitP)
        {
            S1=AnsiString("The target position of ")+Alias+AnsiString(" over positive soft limit !");
            S2=Alias+AnsiString("的目標位置超過正向軟體極限!");
            S3.sprintf("%d > %d", Tar, Motor->PSoftLimitP);
            ShowMyMessage(S1, S2, S3);
            return -2;
        }

        if(Tar<=Motor->PSoftLimitN)
        {
            S1=AnsiString("The target position of ")+Alias+AnsiString(" below negative soft limit !");
            S2=Alias+AnsiString("的目標位置低於負向軟體極限!");
            S3.sprintf("%d <= %d", Tar, Motor->PSoftLimitN);
            ShowMyMessage(S1, S2, S3);
            return -3;
        }

        Position=ReadPos();                                                     //calculate GearRate Pos

        if(Position!=Tar)                                                       //RogerYang 20260113 : 用來紀錄Rotator最近一次旋轉方向
        {
            iLastRotatorDirP=(Tar>Position)?true:false;
        }

        if(fCMD==false && Motor->MotionDone()==false)
        {
            return 0;
        }
        else if(fCMD==false)
        {
            GetRealPos(&Pos);                                                   //pos will change to gear ration value
            #ifndef USE_CompareCommandPos
            if(Tar==Position)
            #else
            if((Motor->GearRatio>2 && Motor->GearRatio<=5)  &&
                ((Mot_Name>=MInArmZA && Mot_Name<=MInArmZH) ||
                 (Mot_Name>=MOutArmZA && Mot_Name<=MOutArmZH)))                 //Jimmychiu 20230216 : 吸嘴高度
            {
                iGap=5;
            }

            if(CompareCommandPos(Tar, iGap)==1)                                 //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
            #endif
            {
                if(Motor->PServoAlarmOn)
                {
                    ScanMotorStatus();
                    if(Led[iInposLed]==true)
                    {
                        return 0;
                    }
                    else
                    {
                        InitMOTParameter();
                        return 1;                                               // -4 --> 1
                    }
                }
                else
                {
                    InitMOTParameter();
                    return 1;
                }
            }
//            if(Motor->Direction==false)
//                Pos=-Pos;

            if(Mot_Name==MInArmY || Mot_Name==MOutArmY)                         //Steven 20181129 : 短距離的移動把加減速縮短一半
            {
                if(AUTO_EMPTY_COLOR>=3)
                {
                    if(Mot_Name==MOutArmY && (abs(Tar-Position)<6500))
                        ret=Motor->MoveToPosShortDisSlowSP(Pos);
                    else
                        ret=Motor->MoveToPos(Pos);
                }
                else
                {
                    if(abs(Tar-Position)<6500)
                        ret=Motor->MoveToPosShortDistance(Pos);
                    else
                        ret=Motor->MoveToPos(Pos);
                }
            }
            else if(Mot_Name==MInShuttle1 || Mot_Name==MInShuttle2)
            {
                if(SHUTTLE_FLOODGATE==1)                                        //Ifor 20260327 add:避免Shuttle 移動時閘門未完全開啟導致撞機
                {
                    if(Mot_Name==MInShuttle1)
                    {
                        Cylinder[C_Shuttle1Floodgate].Off();
                        Cylinder[C_OutShuttle1Floodgate].Off();
                        if(Cylinder[C_Shuttle1Floodgate].OffSensor()==false ||
                           Cylinder[C_OutShuttle1Floodgate].OffSensor()==false)
                        {                                                       //Ifor 20260401 add: Floodgate open timeout
                            if(!bSh1FloodgateOpenWaiting)
                            {
                                tSh1FloodgateOpenTimeout.SetSecAndOn(3);
                                bSh1FloodgateOpenWaiting=true;
                            }
                            else if(tSh1FloodgateOpenTimeout.Off())
                            {
                                tSh1FloodgateOpenTimeout.SetSecAndOn(3);           //Ifor 20260401 add: re-arm for next warning
                                ShowMyMessage("Shuttle Floodgate open timeout! Check cylinder sensor.");
                            }
                            return -1;
                        }
                        bSh1FloodgateOpenWaiting=false;                            //Ifor 20260401 add: reset on success
                    }
                    else
                    {
                        Cylinder[C_Shuttle2Floodgate].Off();
                        Cylinder[C_OutShuttle2Floodgate].Off();
                        if(Cylinder[C_Shuttle2Floodgate].OffSensor()==false ||
                           Cylinder[C_OutShuttle2Floodgate].OffSensor()==false)
                        {                                                       //Ifor 20260401 add: Floodgate open timeout
                            if(!bSh2FloodgateOpenWaiting)
                            {
                                tSh2FloodgateOpenTimeout.SetSecAndOn(3);
                                bSh2FloodgateOpenWaiting=true;
                            }
                            else if(tSh2FloodgateOpenTimeout.Off())
                            {
                                tSh2FloodgateOpenTimeout.SetSecAndOn(3);           //Ifor 20260401 add: re-arm for next warning
                                ShowMyMessage("Shuttle Floodgate open timeout! Check cylinder sensor.");
                            }
                            return -1;
                        }
                        bSh2FloodgateOpenWaiting=false;                            //Ifor 20260401 add: reset on success
                    }
                }

                if(abs(Tar-Position)<5000)
                    ret=Motor->MoveToPosShortDistance(Pos);
                else
                    ret=Motor->MoveToPos(Pos);
            }
            else
            {
                ret=Motor->MoveToPos(Pos);
            }

            if(ret==0)
            {
            }

            fCMD=true;
        }
        else
        {
            if(Motor->MotionDone())
            {
                Position=Tar;
                if(Motor->PServoAlarmOn)
                {
                    ScanMotorStatus();
                    if(Led[iInposLed]==true)
                    {
                        return 0;
                    }
                    else
                    {
                        InitMOTParameter();
                        iEncoderCheckCT++;
                        if(bCheckEncoderEveryTime==true || iEncoderCheckCT>100)
                        {
                            iEncoderCheckCT=0;
                            iEncoder=ReadEncoderPos();
                            if((Tar-iEncoderTorence)>iEncoder ||                //JerryYang 20180706 : 修改Encoder到位容許範圍
                               (Tar+iEncoderTorence)<iEncoder)
                            {
                                S1.sprintf("MOT=%s, Tar=%d, Encoder=%d", Alias, Tar, iEncoder);
                                ShowErrorMessage("WAR1639", 0, MMSystem, 0, S1);   //Motor encoder error, check encoder cable
                                return -5;
                            }

                            if(Motor->PServoAlarmOn)
                            {
                                if(Mot_Name==MInRotateKit ||                    //Sam 20190811 : 防止 Rotate 旋轉完後剛好位置剛好落在原點上面導致誤報警。
                                   Mot_Name==MOutRotateKit)
                                {
                                    return 1;
                                }

                                if(iEncoder>2000 || iEncoder<-2000)
                                {
                                    ScanMotorStatus();
                                    if((Mot_Name==MInArmY ||                    //JerryYang 20191210 fix auto clean時後排到shuttle row A跳出home sensor error
                                        Mot_Name==MOutArmY) &&
                                       iEncoder>2000)
                                    {
                                    }
                                    else
                                    {
                                        if(Led[1]==true)
                                        {
                                            S1.sprintf("MOT=%s Home sensor error!!", Alias);
                                            ShowMyMessage(S1, "");
                                            return -6;
                                        }
                                    }
                                }
                            }

                            return 1;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }
                else
                {
                    InitMOTParameter();

                    //AI(ht9045-v899) 20260505: MLoaderY 工作位置 iMLoaderYCarPos=-505 會誤觸發此防護，僅對 MLoaderY 豁免，其餘馬達維持原有 home sensor 防護
                    if(Position<=-500 && Mot_Name!=MLoaderY)                                          //kevin 20140121 Z軸 home sensor 損壞
                    {
                        if(Mot_Name==MInRotateKit ||                            //Sam 20190811 : 防止 Rotate 旋轉完後剛好位置剛好落在原點上面導致誤報警。
                           Mot_Name==MOutRotateKit)
                        {
                            return 1;
                        }
                        ScanMotorStatus();
                        if(Led[1]==true)
                        {
                            S1.sprintf("MOT=%s Home sensor error!!", Alias);
                            ShowMyMessage(S1, "");

                            if(Mot_Name==MInArmPitch   || Mot_Name==MInArmPitchX2 ||
                               Mot_Name==MInArmPitchX3 || Mot_Name==MInArmPitchX4 ||
                               Mot_Name==MInArmPitchY)
                            {
                                SetInArmHome();
                            }

                            if(Mot_Name==MOutArmPitch   || Mot_Name==MOutArmPitchX2 ||
                               Mot_Name==MOutArmPitchX3 || Mot_Name==MOutArmPitchX4 ||
                               Mot_Name==MOutArmPitchY)
                            {
                                SetOutArmHome();
                            }
                            return -6;
                        }
                    }
                    return 1;
                }
            }
        }
        return 0;
    }

    #ifndef SOFT_SIMULTE
        Position=Tar;
        return 1;
    #else
        if(Position==Tar)
        {
            fCMD=false;
            return 1;
        }
        else
        {
            if(speed<=0)                                                        //Steven 20210730 : 修正軟體模擬的最小速度
                speed=100;

            if(Position>Tar)
            {
                fCMD=true;
                Position-=speed;
                if(Position<=Tar)
                {
                    fCMD=false;
                    Position=Tar;
                    return 1;
                }
            }
            else
            {
                fCMD=true;
                Position+=speed;
                if(Position>=Tar)
                {
                    fCMD=false;
                    Position=Tar;
                    return 1;
                }
            }
        }
        return 0;
    #endif
}
#endif // AI-W6C-GOLDEN-END MotorMovePosition Motor/mymotor.cpp:549-860

// ---------------------------------------------------------------------------
//  MotorMovePosition -- AI(W906-C21-MOTORMOVE) 20260924: W4 樁已退休
//  （原本 :708-740 的 33 行原地改成這段註解，本檔後面的行號一格不動）。
//
//  ⛔ 真本體在本檔尾端（檔尾橫幅「AI(W906-C21-MOTORMOVE) 20260924」下，golden :539-860 逐字）。
//  放尾端的理由同 AI(W906-P0-6) / P0-7 / B1：本體要 ShowMyMessage / ShowErrorMessage
//  （canary_support.h）、Cylinder[]（mycylin.h）、SetInArmHome（aHotPlateSubstrate.h），
//  那幾個 header 在本檔 :2992-2997 才 include；SetOutArmHome 的 extern 在 :3012。
//
//  退休的樁長這樣（留作史料）：
//    if (Motor == NULL || !Motor->Enable) { Position = Tar; return 1; }  // W7 OFFLINE 捷徑
//    return 0;                                                           // 真驅動：永遠「移動中」
//  ⇒ 在馬達 Enable 的機器上，經 MotorMove 的每一軸都停在原地、每一拍回 0。
//
//  回傳碼（golden :540-547，不變）：
//   1 = Move Success  |  0 = Moving  | -1 = Safe Door Opened
//  -2 = Target > LimitP | -3 = Target < LimitN | -4 = PServoAlarmOn
//  -5 = WAR1639 Motor encoder error | -6 = MOT Home sensor error
//
//  !Enable 的行為見檔尾橫幅〈!Enable 的終端〉：golden 的 #ifndef SOFT_SIMULTE 終端
//  （Position=Tar; return 1）兩個組態都用；golden 的 SOFT_SIMULTE 逐拍步進在移植樹
//  500 ms tick 下不收斂，原文以 #if 0 保存在檔尾本體的原位。
// ---------------------------------------------------------------------------











// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- MotorMove
//  golden HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp:871-962  (92 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  AI(W906-C21-MOTORMOVE) 20260924: this gate stays as the verbatim reference.
//  The W4 stub that used to follow it is RETIRED in place; the LIVE body is now
//  golden's, translated at the FILE TAIL (see the C21 tail banner).
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN MotorMove Motor/mymotor.cpp:871-962
int TMyMotor::MotorMove(int p)
{
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor==NULL ||                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return -1;
    }

    //jou 2010-12-23 保護兩次命令會造成撞機
    if(p!=iOldPos)
    {
        fCMD=false;
        iOldPos=p;
        bSh1FloodgateOpenWaiting=false;                                         //Ifor 20260401 add: reset timeout when target changes
        bSh2FloodgateOpenWaiting=false;                                         //Ifor 20260401 add: reset timeout when target changes
    }

    int ret=0;
    int iCommandPos;
    int iGap=1;

    if(fCanMove ==false  ||
       fCanMoveR==false  ||
       fCanMoveM==false  ||
       fCanMoveL==false  ||
       mapLockList.size()!=0)  //Klutter 20210817 加入鎖馬達機制                //Steven 20210825 : 吹氣完成才可以歸零
    {
        PCIL132_StopMotor();

        if((Mot_Name==MInShuttle1 || Mot_Name==MInShuttle2) && (Motor->GearRatio>1))
        {
            iCommandPos=ReadPos();
            if(iCommandPos>p+iGap)      //超出+
            {
                return -2;
            }
            else if(iCommandPos<p-iGap)
            {
                return -3;             //低於-
            }
            else
            {
                fCMD=false;
                return 1;              //正常等於
            }
        }
        else
        {
            if(p==ReadPos())
            {
                fCMD=false;
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    ret=MotorMovePosition(Position, speed, p);
    if(bShowMotorMove==true)
    {
        ScreenPos=(int)(Scale*(Position-FactStart))+RefStart;
        if(bPanelUse)
        {
            if(bUpDownMove)
            {
                if(abs(PWinCtrl->Top-ScreenPos)>2)
                    PWinCtrl->Top=ScreenPos;
            }
            else
            {
                if(abs(PWinCtrl->Left-ScreenPos)>2)
                    PWinCtrl->Left=ScreenPos;
            }
        }
    }

    if(ret==1)
    {
        fCMD=false;
        return 1;
    }
    else
    {
        return ret;
    }
}
#endif // AI-W6C-GOLDEN-END MotorMove Motor/mymotor.cpp:871-962

// ---------------------------------------------------------------------------
//  MotorMove / MotorMove2SpeedForPicker -- AI(W906-C21-MOTORMOVE) 20260924: 兩支 W4 樁已退休
//  （原本 :848-890 的 43 行原地改成這段註解，行號不位移）。
//
//  ⛔ 真本體在本檔尾端（檔尾橫幅「AI(W906-C21-MOTORMOVE) 20260924」下）：
//      MotorMove                  golden :861-962（上面 :753-845 的 #if 0 抄本是同一段原文）
//      MotorMove2SpeedForPicker   golden :963-1000
//
//  退休的兩支樁（留作史料）：
//    MotorMove(p)：
//      if (Motor == NULL || !Motor->Enable)
//          { Position = p; ScreenPos = ...; fCMD = false; return 1; }  // 第一行就回成功
//      return 0;                                                       // 真驅動：永遠「移動中」
//    MotorMove2SpeedForPicker(FinalPos, ARM, bIsLoader)：同上，回 true / false，ARM 不讀。
//  ⇒ 舊捷徑在**安全門與 fCanMove*/mapLockList 互鎖之前**就回成功；真機上則一步都不動。
//
//  現在（檔尾本體）：
//    - Motor==NULL           -> -1（golden :876-880）。舊樁回 1。
//    - 安全門 / 互鎖         -> golden 語意，對 Enable 與 !Enable 軸一視同仁（golden 本來就如此）。
//    - Enable（真機）        -> golden 硬體區塊：軟體極限、MotionDone、GetRealPos、
//                               CompareCommandPos、MoveToPos*、Floodgate、encoder 檢查。
//    - !Enable（模擬 / 未裝）-> golden 的 #ifndef SOFT_SIMULTE 終端（一次到位），兩組態皆然；
//                               理由見檔尾橫幅〈!Enable 的終端〉。
//
//  本檔 :4613-4616（B1 家族橫幅）原本把這兩支列為「仍是樁」，已同步改寫。
// ---------------------------------------------------------------------------


















// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- MotorMoveShuttleShake
//  golden HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp:5198-5316  (119 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
//AI(W906-AMB-L2) 20260929: UN-GATED (RULINGS_20260929 #11). The copy below is now the LIVE body and the W4
//  stub that followed it is retired in place (comment after the "#endif"; lines from MotorInitial on do not move).
//  Deps re-measured with g++ -fsyntax-only (SIM + -DW906_NO_SOFT_SIMULTE): only two errors, both handled here --
//  bShuttleShake undeclared -> extern below; PWinCtrl is void* -> GATE(C21-1), same as MotorMove (file-tail banner (2)).
//  One port-side edit: Motor==NULL added to the first check, golden's own MotorMove form (golden :876-877, JimmyChiu 20250306).
// ===========================================================================
extern bool bShuttleShake;                                                      //AI(W906-AMB-L2) 20260929: = golden mymotor.cpp:5197 verbatim (the W6c copy started at :5198; review 20260929); golden ainarm2.h:84 declares it too; port csystem_shims.h:73, defined csystem_shims.cpp:38 (ht9045_sm, same lib as ShowMyMessage/MyDBIProcess this file already uses)
//#if 0 // AI-W6C-GOLDEN-BEGIN MotorMoveShuttleShake Motor/mymotor.cpp:5198-5316   //AI(W906-AMB-L2) 20260929: gate retired -- this verbatim golden copy (golden mymotor.cpp:5198-5316) is now the LIVE body; the W4 stub below it is retired in place. Every dependency exists in this TU (see the banner above).
bool TMyMotor::MotorMoveShuttleShake(int p)                                     //JerryYang 20190628 shuttle shake專用command
{
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor==NULL || Motor->CheckIsSafeDoorOpen())                             //Jimmychiu 20221013 safedoor判斷整合Function   //AI(W906-AMB-L2) 20260929: port adds "Motor==NULL ||" in golden's MotorMove form (golden :876-877) -- golden has no NULL test here because InitialMotorParameter never leaves MOT[i].Motor NULL (wb_serve runs it too); only the NULL-driver test HAL can reach this with NULL. The retired stub returned true for NULL; this returns false (not arrived), as golden MotorMove reports -1.
    {
        return false;
    }

    if(bShuttleShake==false)
        return false;
    if(Mot_Name==MInShuttle1 || Mot_Name==MInShuttle2)
    {
    }
    else
    {
        return false;
    }

    if(Mot_Name==MInShuttle1)
    {
        if(p==Prod.InSHT[0].iLeft+SHSpeed_File.iShakeDistance*100 || p==Prod.InSHT[0].iLeft)    //Sam 20250326 : 新增 Shake 條件設定
        {
        }
        else
        {
            return false;
        }
    }
    else if(Mot_Name==MInShuttle2)
    {
        if(p==Prod.InSHT[1].iLeft+SHSpeed_File.iShakeDistance*100 || p==Prod.InSHT[1].iLeft)    //Sam 20250326 : 新增 Shake 條件設定
        {
        }
        else
        {
            return false;
        }
    }

    if(p!=iOldPos)                                                              //jou 2010-12-23 保護兩次命令會造成撞機
    {
        fCMD=false;
        iOldPos=p;
    }

    int ret=0;
    int iCommandPos;
    int iGap=1;

    if(fCanMove ==false  ||
//       fCanMoveR==false  ||
       fCanMoveM==false  ||
       fCanMoveL==false  ||                                                     //Klutter 20210817 加入鎖馬達機制
       mapLockList.size()!=0)                                                   //Steven 20210825 : 吹氣完成才可以歸零
    {
        PCIL132_StopMotor();

        if((Mot_Name==MInShuttle1 || Mot_Name==MInShuttle2) && (Motor->GearRatio>1))
        {
            iCommandPos=ReadPos();
            if(iCommandPos>p+iGap)                                              //超出+
            {
                return false;
            }
            else if(iCommandPos<p-iGap)
            {
                return false;                                                   //低於-
            }
            else
            {
                fCMD=false;
                return true;                                                    //正常等於
            }
        }
        else
        {
            if(p==ReadPos())
            {
                fCMD=false;
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    ret=MotorMovePosition(Position, speed, p);
    if(bShowMotorMove==true)
    {
        ScreenPos=(int)(Scale*(Position-FactStart))+RefStart;
        if(bPanelUse)
        {
#if 0 // GATE(C21-1) AI(W906-AMB-L2) 20260929: same gate as MotorMove -- PWinCtrl is void* here (mymotor.h:106, golden TWinControl*); measured: g++ -fsyntax-only reports 'void*' is not a pointer-to-object type on these 4 lines. bPanelUse stays false (ctor :165; every SetPanel() call, cinitial.cpp:14080-14147 incl. MInShuttle1/2 :14146-14147, sits inside #if 0 GATE n5-G3 cinitial.cpp:14077-14570; renumbered AI(W906-HTRAY) 20260930). Display only. See file-tail C21 banner (2).
            if(bUpDownMove)
            {
                if(abs(PWinCtrl->Top-ScreenPos)>2)
                    PWinCtrl->Top=ScreenPos;
            }
            else
            {
                if(abs(PWinCtrl->Left-ScreenPos)>2)
                    PWinCtrl->Left=ScreenPos;
            }
#endif // GATE(C21-1)
        }
    }

    if(ret==1)
    {
        fCMD=false;
        return true;
    }
    else
    {
        return false;
    }
}
//#endif // AI-W6C-GOLDEN-END MotorMoveShuttleShake Motor/mymotor.cpp:5198-5316   //AI(W906-AMB-L2) 20260929: gate retired (matching #if 0 above)

// ---------------------------------------------------------------------------
//  MotorMoveShuttleShake -- AI(W906-AMB-L2) 20260929: W4 stub RETIRED in place (the golden body above is live).
//  The old 20-line stub block became these 12 lines, absorbing the 8 lines added above, so MotorInitial and
//  everything after it keep their line numbers.
//  The retired stub, kept as history:
//    if (Motor == NULL || !Motor->Enable) { Position = p; ScreenPos = ...; return true; }  // W7 OFFLINE shortcut
//    return false;   // TODO(W6-state-machine): the real driver was never commanded
//  => on Enable motors it never returned true, so DoShakeShuttle's essMoveRight (ainarm2.cpp:6664) could not
//     advance; on !Enable motors it "arrived" before golden's bShuttleShake / target / fCanMove* / mapLockList
//     checks. Now both follow golden; !Enable reaches MotorMovePosition's one-shot terminal (file-tail C21
//     banner (1)).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void TMyMotor::MotorInitial()
{
    // AI(W906-HOME-C2) 20260920: 補上與手足一致的 NULL 守衛。
    //
    //   `Home()`（:1049）與 `MotorHome()`（:1076）第一行都是
    //       if (Motor == NULL || Motor->CheckIsSafeDoorOpen()) return ...;
    //   只有 `MotorInitial()` 沒有，所以它是這一族裡唯一會 SIGSEGV 的那個。
    //
    //   ⚠ 這不是「golden 有這個守衛」—— golden 沒有，因為 golden 的
    //     `MOT[i].Motor` 由 `InitialMotorParameter()`（cinitial.cpp:3739）
    //     在啟動時無條件建好，**永遠非 NULL**。這是**移植樹自己**的狀態：
    //     本樹允許「還沒 bring-up」的 TU 存在（測試、wb_publish…）。
    //
    //   實測觸發（20260920）：`ProcessMotorHome()` case 1 對 164 顆馬達呼叫
    //   這一支；在沒跑過 `InitialMotorParameter()` 的行程裡，第 0 顆就爆。
    //   堆疊：MainProc -> W906_MainProcHomeDispatch -> DoHomeProcess
    //        -> ProcessMotorHome -> MotorInitial -> HTMotor::SetHomeobjectTask
    //
    //   ⇒ 改成與手足相同的「NULL 就是離線、什麼都不做」。
    //     本樹既有的 HAL 慣例就是這樣，而在 handler 上「當掉」比「no-op」糟。
    //   ⚠ 但下面三行**照常執行**（iMyHomeTask/ResetTime/HomeFlag 是 TMyMotor
    //     自己的狀態，與硬體無關）—— 只跳過那一次驅動層呼叫。
    if (Motor != NULL)
        Motor->SetHomeobjectTask(1);
    iMyHomeTask = 1;
    ResetTime.Set0_1SecAndOn(RESET_TIMES);
    HomeFlag = 0;
}

// ---------------------------------------------------------------------------
bool TMyMotor::Home(void)
{
    // **NOTE: CheckIsSafeDoorOpen is safety-critical in the real Handler.**
    if (Motor == NULL || Motor->CheckIsSafeDoorOpen())
        return false;

    Motor->iHomePitch = 200;
    GetRealPos(&Motor->iHomePitch);
    fCMD = false;
    SetADCRate(100);
    return Motor->HomeObject();
}

// ---------------------------------------------------------------------------
void TMyMotor::HomeReset()
{
    Motor->SetHomeobjectTask(1);
}

// ---------------------------------------------------------------------------
//  MotorHome state machine
//
//  The Galil branch (INDEX_MOTION_CARD==0 && MTestY1..MTestY2) calls
//  Gali_ScanAlarmStatus() / Gali_ScanMotStatus() / Gali_Command().
//  Those are gated TODO(W6-Galil); the SMC/generic else-branch is active.
//  In the W4 sim build INDEX_MOTION_CARD defaults to non-zero (from
//  cmydef.h constant), so the Galil branch is never entered at runtime.
// ---------------------------------------------------------------------------
std::string g_W906MotorErrorReason;              //AI(W906-ALARM-WHY) 20261003: EastSun「你跳出錯誤需要出現為什麼錯誤」-- the reason the NEXT motor jam note shows under golden's message (forms/fNote_ShowError.cpp ShowMotorErrorMessage reads and clears it); "" = golden's message only
int  (*g_W906PreHomeHook)(int motIndex, char* why, int whyLen) = 0;  //AI(W906-HOME-STEPLEAVE) 20261003: WebMotorAccess MotorAccessStepperLeaveOrigin for MOT[motIndex] (1 home / 0 wait / -1 alarm); WebMotorAccessLive.cpp registers it, 0 = home as before
void (*g_W906MotorHomeNote)(const char*) = 0;  //AI(W906-HOME-WHY) 20261003: why MotorHome failed, into the op log (tools/wb_serve.cpp registers it; HOME line)
static void W906_MotorHomeWhy(const TMyMotor& m, int ret, const char* why)
{
    char b[256];
    std::snprintf(b, sizeof(b), "%s MotorHome returns %d: %s (alarm LED %d, servo LED %d, origin LED %d)",
                  m.Alias.c_str(), ret, why, (int)m.Led[iAlarmLed], (int)m.Led[iServoOn], (int)m.Led[iHomeLed]);
    std::printf("[HOME] %s\n", b);
    if (g_W906MotorHomeNote) g_W906MotorHomeNote(b);
    if (ret != 0) g_W906MotorErrorReason = b;    //AI(W906-ALARM-WHY) 20261003: uhome's jam popup for this failure shows it
}

int TMyMotor::MotorHome(bool Flag)
{
    if (Motor == NULL || Motor->CheckIsSafeDoorOpen())
        return 0;

    int ret = 0;
    int &Task = iMyHomeTask;

    if (Motor->Enable)
    {
        ScanMotorStatus();
        if (GetMotorAlarm())
        {
            ret = GetErrorIndex();
            if (ret < 2 || ret > 5)
            {
                Task = 1;
                W906_MotorHomeWhy(*this, 3, "motor alarm during the home (GetMotorAlarm)");   //AI(W906-HOME-WHY) 20261003
                return 3;
            }
        }
    }

    switch (Task)
    {
        case 1:
            if (Motor->Enable == false)
            {
                Task     = 1;
                Position = 0;
                HomeFlag = 1;
                iLastRotatorDirP = true;    // RogerYang 20260113
                return 1;
            }
            HomeReset();
            Task = g_W906PreHomeHook ? 5 : 10;                                  //AI(W906-HOME-STEPLEAVE) 20261003: 5 = leave the origin first (SW3D steppers), then 10
            iHomeRetryCT = 3;
            Motor->bW906HomeTrusted = false; Motor->bW906HomeFault = false;   //AI(W906-HOME-FAILVISIBLE) 20261003: a new home starts clean
            if (g_W906PreHomeHook) { char w[8]; g_W906PreHomeHook(-1 - Mot_Name, w, (int)sizeof(w)); }   //  -1-mi = reset that motor's half-done leave
            break;

        case 5:                                                                 //AI(W906-HOME-STEPLEAVE) 20261003: EastSun ruling 1003 -- the shared
            {                                                                   //  MotorAccessStepperLeaveOrigin (the same function the single HOME uses)
                char why[256] = "";
                const int lv = g_W906PreHomeHook ? g_W906PreHomeHook(Mot_Name, why, (int)sizeof(why)) : 1;
                if (lv == 0) break;                                             //  still moving off the origin
                if (lv < 0)
                {
                    Task = 1;
                    HomeFlag = 2;
                    W906_MotorHomeWhy(*this, 2, why[0] ? why : "the stepper could not leave its origin");
                    return 2;                                                   //  uhome's motor-alarm path (that axis's jam popup, HOME stopped)
                }
                Task = 10;
            }
            break;

        case 10:
            {
                const bool hd = Home();
                if (Motor->bW906HomeFault)                                      //AI(W906-HOME-FAILVISIBLE) 20261003: the routed 1203 home failed
                {                                                               //  (never sent / ERROR_STOP / no HOMING) -- was a silent wait
                    Motor->bW906HomeFault = false;
                    Task = 1;
                    HomeFlag = 2;
                    W906_MotorHomeWhy(*this, 2, "the 1203 home failed (never sent / ERROR_STOP / READY without HOMING -- see the ROUTE HomeDone line)");   //AI(W906-HOME-WHY) 20261003
                    return 2;                                                   // uhome's motor-alarm path (jam popup, HOME stopped)
                }
                if (!hd) break;
            }
            {
                Task = 20;
                htWaitHomeSensorOnDelay.SetMSAndOn(300);

                if (Led[iServoOn] == false)
                {
                    Task = 1;
                    W906_MotorHomeWhy(*this, 4, "home done but the servo LED is off");   //AI(W906-HOME-WHY) 20261003
                    return 4;
                }
            }
            break;

        case 20:
            if (Motor->bW906HomeTrusted || (W906_Ht9050OrgHome(Mot_Name) != -2 ? W906_Ht9050OrgHome(Mot_Name) == 1 : Motor->HomeFlag()))   //AI(W906-HT9050-ORG-ENG) 20261001: golden `Motor->HomeFlag()`；HT9050＋1203＝原點訊號 low（mymotor.h）  AI(W906-HOME-FAILVISIBLE) 20261003: + a DS402 home the drive reported done counts (it ends off the switch; Motor Test never checks the lamp either)
            {
                const bool w906Trusted = Motor->bW906HomeTrusted;  Motor->bW906HomeTrusted = false;   //AI(W906-FULLHOME-ZSAFE) 20261005: NB2-1 R231 -- remembered for the ZSafePos step below
                Task = 1;
                HomeFlag  = 1;
                fCanMove  = true;
                fCanMoveR = true;
                fCanMoveM = true;
                fCanMoveL = true;
                iLastRotatorDirP = true;    // RogerYang 20260113
                { extern bool W906_FullHomeZSafeWanted(const TMyMotor&, bool); if (W906_FullHomeZSafeWanted(*this, w906Trusted)) { Task = 40; HomeFlag = 0; break; } }  return 1;   //AI(W906-FULLHOME-ZSAFE) 20261005: NB2-1 R231 -- MachineType.h W906_HT9050_FULLHOME_ZSAFE (default OFF = return 1 here as before): an HT9050 arm Z the drive homed goes on to ZSafePos (Task 40, helpers at the end of this file)
            }
            else if (htWaitHomeSensorOnDelay.Off())
            {
                if (iHomeRetryCT)
                {
                    iHomeRetryCT--;
                    Task = 30;
                }
                else
                {
                    HomeFlag = 2;
                    W906_MotorHomeWhy(*this, 2, "origin check failed after 3 retries (not a trusted DS402 done, origin LED off)");   //AI(W906-HOME-WHY) 20261003
                    return 2;
                }
            }
            if (Task == 30 && g_W906MotorHomeNote) W906_MotorHomeWhy(*this, 0, "origin LED off 0.3 s after the home -- retry");   //AI(W906-HOME-WHY) 20261003
            break;

        case 40: { extern int W906_FullHomeZSafeTick(TMyMotor&); if (W906_FullHomeZSafeTick(*this) == 0) break; Task = 1; HomeFlag = 1; return 1; }  case 30:   //AI(W906-FULLHOME-ZSAFE) 20261005: NB2-1 R231 -- Task 40 = golden ProcessSingleMotorHome case 500 `if(MotorMove(ZSafePos)) HomeFlag=true` (only reached with W906_HT9050_FULLHOME_ZSAFE)
            if (Motor->Enable == false)
            {
                Task     = 1;
                Position = 0;
                HomeFlag = 1;
                return 1;
            }
            HomeReset();
            Task = 10;
            break;
    }

    if (Flag)
        ResetTime.Set0_1SecAndOn(RESET_TIMES);

    if (ResetTime.Off())
    {
        HomeFlag = 2;
        W906_MotorHomeWhy(*this, 2, "home timeout (ResetTime)");   //AI(W906-HOME-WHY) 20261003
        return 2;
    }
    return 0;
}

// ---------------------------------------------------------------------------
bool TMyMotor::GetMotorAlarm()
{
    if (Motor != NULL)
    {
        if (Mot_Name == MTestY1 || Mot_Name == MTestZ1 ||
            Mot_Name == MTestZ2 || Mot_Name == MTestY2)
        {
            // Galil branch: TODO(W6-Galil)
            { extern bool W906_GaliRouteOwns(int); if (W906_GaliRouteOwns(Mot_Name)) Gali_ScanAlarmStatus(); }   //AI(W906-INDEXZ-1203) 20260929: golden :1771 restored for the routed axis only (Motor/GaliRoute.h); the other Galil axes keep this TODO (F7, separate)
            // return Gali_MotorAlarm;
            return Gali_MotorAlarm;     // default false in W4 sim
        }
        else
        {
            return Motor->GetAlarm();
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
int (*W906_Ht9050OrgHomeHook)(int motIndex) = 0;   //AI(W906-HT9050-ORG-ENG) 20261001: mymotor.h（wb_serve 開機裝；沒裝＝-2＝golden 原樣）

void TMyMotor::ScanMotorStatus()
{
    if (Motor != NULL)
    {
        if (INDEX_MOTION_CARD == 0 &&
            (Mot_Name == MTestY1 || Mot_Name == MTestZ1 ||
             Mot_Name == MTestZ2 || Mot_Name == MTestY2))
        {
            // Galil branch: TODO(W6-Galil)
            { extern bool W906_GaliRouteOwns(int); if (W906_GaliRouteOwns(Mot_Name)) Gali_ScanMotStatus(); }   //AI(W906-INDEXZ-1203) 20260929: golden :1789 restored for the routed axis only (Motor/GaliRoute.h); the other Galil axes keep this TODO (F7, separate)
        }
        else
        {
            Motor->ScanMotorStatus(Led);
        }
        //AI(W906-HT9050-ORG-ENG) 20261001: HT9050＋1203：HOME 燈＝1203 原點訊號 low（兩個分支之後，各家 ScanMotorStatus 寫的值不會蓋掉它；mymotor.h）
        { const int w906Home = W906_Ht9050OrgHome(Mot_Name); if (w906Home != -2) Led[iHomeLed] = (w906Home == 1); }
    }
}

// ---------------------------------------------------------------------------
int TMyMotor::GetErrorIndex()
{
    if (Led[iAlarmLed] && Led[iServoalarmLed] && Led[iInposLed])
        return 0;
    else if (Led[iAlarmLed] && Led[iServoalarmLed])
        return 1;
    else if (Led[iAlarmLed] && Led[iCwLed])
        return 2;
    else if (Led[iAlarmLed] && Led[iCcwLed])
        return 3;
    else if (Led[iCwLed] || Led[iCcwLed])
        return 6;
    else if (Led[iAlarmLed] && Motor->PServoAlarmOn)
        return 7;
    else if (Led[iAlarmLed] && Led[iSoftcwLed])
        return 4;
    else if (Led[iAlarmLed] && Led[iSoftccwLed])
        return 5;
    else
        return 9;
}

// ---------------------------------------------------------------------------
void TMyMotor::PCIL132_SetPos(int Pos)
{
    if (Motor != NULL && Motor->Enable == false)
    {
        Position = 0;
    }
    else
    {
        if (Motor == NULL || Motor->Enable == false)
            return;
        Motor->SetCommand(Pos);
        Motor->SetPosition(Pos);
    }
}

// ---------------------------------------------------------------------------
void TMyMotor::PCIL132_ResetPos()   // Steven 20110628: reset CommandPos on ServoOn
{
    if (Motor == NULL || Motor->Enable == false)
        return;

    extern bool W906_Is1203Motor(const HTMotor*); if (MOTION_CARD_TYPE == MotionCard_SYN && !W906_Is1203Motor(Motor))   //AI(W906-SVON-SYNCARD) 20261005: NB2-1 R230 (laptop W-60) -- golden :1838-1856 picks the arm by the GLOBAL card type because TMySYNTEKMotor::ResetPos(p) ignores p and copies the encoder into the command (golden mySYNTEKmotor.cpp:673-681): both arms mean "command := encoder". A PCIE-1203 row is a TMyEtherCatMotor, whose ResetPos(p) WRITES p: on HT9050 (Gerneral.ini MOTION_CARD_TYPE=0) the SYN arm wrote 0, the route refused it (Q1: not within 1 pulse of the encoder) and the servo-ON sync never happened. A 1203 row takes the encoder arm, as on every non-SYN machine; every other class keeps golden's dispatch (helper at the end of this file)
    {
        Motor->ResetPos();
    }
    else
    {
        int p = Motor->ReadEnCoderRealPos();
        if (Motor->Direction)
            p = -p;
        Motor->ResetPos(p);
    }
}

// ---------------------------------------------------------------------------
void TMyMotor::PCIL132_StopMotor()
{
    if (Motor != NULL && Motor->Enable)
    {
        if (Mot_Name == MTestY1 || Mot_Name == MTestZ1 ||
            Mot_Name == MTestZ2 || Mot_Name == MTestY2)
        {
            return;
        }
        fCMD = false;
        Motor->DecStop();
    }
}

// ---------------------------------------------------------------------------
void TMyMotor::JogP(int /*Speed*/)
{
    if (Motor == NULL || Motor->CheckIsSafeDoorOpen())
        return;
    Motor->JogP();
}

// ---------------------------------------------------------------------------
void TMyMotor::JogN(int /*Speed*/)
{
    if (Motor == NULL || Motor->CheckIsSafeDoorOpen())
        return;
    Motor->JogN();
}

// ---------------------------------------------------------------------------
//  ServoOnOff -- Galil branch (INDEX_MOTION_CARD==0, MTestY1..MTestY2)
//  calls Gali_Command; gated TODO(W6-Galil).  SMC/generic branch translated.
// ---------------------------------------------------------------------------
void TMyMotor::ServoOnOff(bool IsOn)
{
    if (Motor != NULL && Motor->Enable)
    {
        if (INDEX_MOTION_CARD == 0 &&
            Mot_Name >= MTestY1 && Mot_Name <= MTestY2)
        {
            extern bool W906_GaliRouteOwns(int); if (W906_GaliRouteOwns(Mot_Name)) {   //AI(W906-INDEXZ-1203) 20260929: golden :1906-1926 restored for the routed axis only (HT9050 M14, Motor/GaliRoute.h); TODO(W6-Galil) stays for the other Galil axes (F7)
            AnsiString S[] = {"X","Y","Z","W"};                                    //AI(W906-INDEXZ-1203) 20260929: golden :1906
            Gali_ScanMotStatus();                                                  //AI(W906-INDEXZ-1203) 20260929: golden :1913
            if(IsOn) { if(Led[iServoOn]==false) Gali_Command("SH"+S[Mot_Name-MTestY1], __func__); }   //AI(W906-INDEXZ-1203) 20260929: golden :1914-1918 (sevro on)
            else     { if(Led[iServoOn]) { Gali_Command("AB1", __func__); Gali_Command("MO"+S[Mot_Name-MTestY1], __func__); } } }   //AI(W906-INDEXZ-1203) 20260929: golden :1919-1926 (AB1 = stop all, then servo off)
        }
        else
        {
            Motor->SetServoOn(IsOn);
            if (Motor->PServoAlarmOn)
            {
                if (IsOn)
                {
                    // MySleep(200) gated -- TODO(W6-sleep)
                    PCIL132_ResetPos();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void TMyMotor::SetArmMaxSpeed()
{
    Motor->SetArmMaxSpeed();
}

// ---------------------------------------------------------------------------
//  GetRealPos -- converts logical position to hardware pulses via GearRatio
//
//  The Contec direction-flip branch (MOTION_CARD_TYPE==MotionCard_Contec) is
//  translated faithfully; MotionCard_Contec is defined in MachineType.h.
// ---------------------------------------------------------------------------
void TMyMotor::GetRealPos(int *iPos)
{
    TargetPosition = *iPos;
    int    p  = *iPos;
    double r  = 0.0;

    if (Motor->GearRatio == 0.0)
        Motor->GearRatio = 1.0;

    r = Motor->GearRatio;
    int p1 = 0, p2 = 0;
    p1 = ChangeToFloatNonPcnt((double)(double(p)), (double)(r));
    p2 = (int)(double(p1) * r);

    if (p2 < p)
    {
        while (1)
        {
            p1++;
            p2 = (int)(double(p1) * r);
            if (p2 >= p)
                break;
        }
    }
    else if (p2 > p)
    {
        while (1)
        {
            p1--;
            p2 = (int)(double(p1) * r);
            if (p2 <= p)
                break;
        }
    }
    *iPos = p1;

    if (Mot_Name == MTestY1 || Mot_Name == MTestZ1 ||
        Mot_Name == MTestZ2 || Mot_Name == MTestY2)
    {
        // Galil axes: no direction flip
    }
    else
    {
        if (MOTION_CARD_TYPE == MotionCard_Contec)
        {
            if (Motor->Direction)
                *iPos = -*iPos;
            else
                *iPos = *iPos;
        }
    }
}

// ---------------------------------------------------------------------------
int TMyMotor::CompareEncoderPos(int iPos, int iGap)     // Isaac 20201217
{
    int iEncoderPos = ReadEncoderPos();
    if (iPos - iGap <= iEncoderPos && iEncoderPos <= iPos + iGap)
        return 1;
    else
        return -1;
}

// ---------------------------------------------------------------------------
int TMyMotor::CompareCommandPos(int iPos, int iGap)     // Isaac 20201217
{
    int iCommandPos = ReadPos();
    if (iPos - iGap <= iCommandPos && iCommandPos <= iPos + iGap)
        return 1;
    else
        return -1;
}

// ---------------------------------------------------------------------------
//  GetRotatorBacklash  (RogerYang 20260113: Rotator backlash)
// ---------------------------------------------------------------------------
int TMyMotor::GetRotatorBacklash(int iGoalPos, bool bInRotator, int iTechData)
{
    int iBacklash = 0;
    int iProdData = 0;
    if (bInRotator == true)
    {
        iProdData = (iTechData == 0) ? Prod.iIn_iRotateA_Backlash : iTechData;
        if (iGoalPos > MOT[MInRotateKit].ReadPos() &&
            MOT[MInRotateKit].iLastRotatorDirP == false)
        {
            iBacklash = iProdData;
        }
        else if (iGoalPos < MOT[MInRotateKit].ReadPos() &&
                 MOT[MInRotateKit].iLastRotatorDirP == true)
        {
            iBacklash = -iProdData;
        }
    }
    else
    {
        iProdData = (iTechData == 0) ? Prod.iOut_iRotateA_Backlash : iTechData;
        if (iGoalPos > MOT[MOutRotateKit].ReadPos() &&
            MOT[MOutRotateKit].iLastRotatorDirP == false)
        {
            iBacklash = iProdData;
        }
        else if (iGoalPos < MOT[MOutRotateKit].ReadPos() &&
                 MOT[MOutRotateKit].iLastRotatorDirP == true)
        {
            iBacklash = -iProdData;
        }
    }
    return iBacklash;
}

// ---------------------------------------------------------------------------
//  Lock / UnLock / ClearLock / GetLockCount / GetLockString
//  (Steven 20210825: lock list to prevent concurrent motor access)
// ---------------------------------------------------------------------------
int TMyMotor::GetLockCount()
{
    return (int)mapLockList.size();
}

void TMyMotor::Lock(AnsiString MotorAlias, AnsiString FunctionName, int /*Task*/)
{
    mapLockList[MotorAlias] = FunctionName;
}

void TMyMotor::UnLock(AnsiString MotorAlias, AnsiString /*FunctionName*/)
{
    mapLockIter = mapLockList.find(MotorAlias);
    if (mapLockIter != mapLockList.end())
        mapLockList.erase(mapLockIter);
}

void TMyMotor::ClearLock()
{
    mapLockList.clear();
}

AnsiString TMyMotor::GetLockString(int Index)
{
    AnsiString sRet = "";
    int iIdx = 0;
    for (mapLockIter = mapLockList.begin();
         mapLockIter != mapLockList.end(); ++mapLockIter)
    {
        if (iIdx == Index)
        {
            sRet = mapLockIter->first + ":" + mapLockIter->second;
            break;
        }
        iIdx++;
    }
    return sRet;
}

// ---------------------------------------------------------------------------
//  Gali_* method stubs -- declared for ABI; all bodies gated TODO(W6-Galil)
// ---------------------------------------------------------------------------
// =============================================================================
//  ★ AI(W906-P0-5) 20260920: 這一批 `Gali_*` 樁**退休**（`#if 0`，文字留著當史料）
//
//  使用者 20260920 22:1x 裁決（INBOX Q18）：「依據建議執行」＝ 選項 A。
//
//  ## 為什麼
//
//  `Motor/myGALILmotor.cpp` 有這 48 個方法的**真本體**，兩者同在
//  `libht9045_motor.a`。GNU ld 只在需要解某個符號時才抽取成員，所以只要沒有
//  任何 TU 同時逼出這兩個 `.obj`，重複就一直潛伏著 —— 一旦逼出來，
//  連結會噴 `multiple definition of TMyMotor::Gali_*`。
//
//  20260920 它從「潛在風險」變成「擋住主線」：
//  歸零狀態機（`uhome.cpp` 的 `ProcessMotorHome`）跑到 **case 700** 停住，
//  等的是 `MOT[MTestZ1].Gali_Two_ZAxis_Move(...)`，而它連到的是下面那個
//  `{ return (Motor==NULL); }` 的樁 —— sim 馬達非 NULL ⇒ 恆 false ⇒ 卡死。
//  實測 `iHomeStep` 序列：`…600→650→600→700(x43 停住)`。
//
//  ⚠ 這台機器的 Index **真的用 Galil**：`system/Gerneral.ini:144
//    INDEX_MOTION_CARD=0`。所以 golden 在 case 700 呼叫 Galil 函式不是筆誤。
//
//  ## 退樁是 1:1 的乾淨交換（`nm` 實測，不是推論）
//
//      myGALILmotor.cpp.obj 定義的 TMyMotor:: 符號      48
//      其中 mymotor.cpp.obj 也定義的                     48（全部）
//      只有 myGALILmotor.cpp 有的                         0
//      myGALILmotor.cpp.obj 的未定義符號                193
//      其中在 libht9045_*.a / libvclcompat.a 找不到的     13（全是 libc/Win32）
//
//  ⇒ 沒有符號會消失，也不會帶進新的專案相依。
//
//  ## ⚠ 行為會變，而且是刻意的
//
//  下面幾個樁帶著 W6.4-TESTER 20260626 加的「離線快捷」：
//      Gali_Two_ZAxis_Move / GalilTwoY_Move / ISNormal  -> `return (Motor==NULL);`
//  它們的註解說那是為了「讓 test-head SM 在 Sim HAL 上 pump」。
//
//  ★ **但那個判準從來沒有生效過**：本樹的「離線」不是 `Motor==NULL`，
//    是「`Motor` 是 `TMySimMotor`」。wb_serve 與每一支帶 Sim HAL 的測試裡
//    `Motor` 都非 NULL，所以那三個快捷一律回 false。
//
//  退樁之後這幾條走 `myGALILmotor.cpp` 的真本體，而那裡有 **14 處
//  `SOFT_SIMULTE` 條件編譯**（:1354 / :3616 / :3673 / :3972…），
//  也就是 Galil 層自己就有模擬路徑。
//  真本體開頭已由 `gali_guards.py` 補上
//  `if(Motor==NULL) return <樁的離線值>;` —— 值逐個抄自下面這些樁，
//  所以 `Motor==NULL` 時的可觀察行為與退樁前**逐位元組相同**。
//
//  ## ⛔ 沒有一起退的（**不是疏漏，而且 v1 的腳本在這裡是錯的**）
//
//      :1549 Gali_MotHome_HighSpeed
//          myGALILmotor.cpp **沒有**它的真本體 -> 這個樁是全樹唯一的定義。
//          退掉它 = 留下「有宣告、沒定義」的方法。今天沒有呼叫端所以連結
//          不會馬上壞，但 RogerYang 20250410 為 9046AU 加的那條路接上就會炸。
//          ⇒ 它**夾在退休區間中間**，所以下面拆成兩段 `#if 0`。
//
//      :1582+ IsStartGali_Pr / StartGali_Pr / EndGali_Pr /
//             GetGali_Pr_Result / GetGali_Pr_ER
//          同樣不在那 48 個裡；它們在退休區間**之外**，本來就沒被動到。
//
//  ⚠ v1 的橫幅寫「Gali_MotHome_HighSpeed 沒有一起退」但程式**退了它** ——
//    敘述與程式不一致。改的時候兩邊都要改。
// =============================================================================
#if 0 // GALI-STUB RETIRED (W906-P0-5): 真本體在 Motor/myGALILmotor.cpp
AnsiString TMyMotor::Gali_GetMOT(int /*MOT*/) { return ""; }
void TMyMotor::Gali_ScanMotStatusTIMO() {}
void TMyMotor::Gali_ScanMotStatus()     {}
void TMyMotor::Gali_ScanAlarmStatus()   {}
bool TMyMotor::Gali_MotMove(int, int, AnsiString)            { return false; }
bool TMyMotor::Gali_MovePR(int, int)                         { return false; }
bool TMyMotor::Gali_MotMove2(int, int, int)                  { return false; }
bool TMyMotor::Gali_MotMoveNoWait(int, int, int, bool)       { return false; }
bool TMyMotor::Gali_MotMoveSkipEncoder(int, int)             { return false; }
bool TMyMotor::Z1UpZ2Down1(int)                              { return false; }
bool TMyMotor::Z1DownZ2Up1(int)                              { return false; }
bool TMyMotor::Z1UpZ2Down2(int, bool)                        { return false; }
bool TMyMotor::Z1DownZ2Up2(int, bool)                        { return false; }
bool TMyMotor::Z1UpZ2Down(int, bool, bool)                   { return false; }
bool TMyMotor::Z1DownZ2Up(int, bool, bool)                   { return false; }
long TMyMotor::Gali_Command(AnsiString, AnsiString)          { return 0; }
long TMyMotor::Gali_ReadPos()                                { return 0; }
long TMyMotor::Gali_ReadEncoderPos()                         { return 0; }
void TMyMotor::Gali_MotHome(AnsiString)                      {}
#endif // GALI-STUB RETIRED (W906-P0-5)
// ---------------------------------------------------------------------------
//  ⛔ 下面這一行**刻意留在 `#if 0` 外面**，見上面橫幅的「沒有一起退的」。
//     `myGALILmotor.cpp` 沒有 `Gali_MotHome_HighSpeed` 的真本體，
//     這個樁是全樹唯一的定義。退掉它 = 有宣告沒定義。
// ---------------------------------------------------------------------------
void TMyMotor::Gali_MotHome_HighSpeed(AnsiString, int)       {}
#if 0 // GALI-STUB RETIRED (W906-P0-5): 真本體在 Motor/myGALILmotor.cpp
void TMyMotor::Gali_MotHomeFindZ(AnsiString)                 {}
bool TMyMotor::Gali_SingalHome(bool)                         { return false; }
bool TMyMotor::Gali_FindZPhase()                             { return false; }
// AI(W6.4-TESTER) 20260626: Motor==NULL offline fast-path -- no vendor backend
// means there is no hardware to wait on, so the Galil-Z move reports COMPLETE
// immediately (matches the W4 HAL design: offline ReadPos preserves Position,
// ScanMotorStatus is a no-op).  Lets the test-head SM (DoTestHeadMotor) pump
// over the Sim HAL; with a real HTMotor* attached the body is gated TODO(W6-Galil).
bool TMyMotor::Gali_Two_ZAxis_Move(int,int,AnsiString,bool,int) { return (Motor==NULL); }
void TMyMotor::Gali_JogP(int)          {}
void TMyMotor::Gali_JogPSetup(int)     {}
void TMyMotor::Gali_JogPAndCount(int, int) {}
void TMyMotor::Gali_JogN(int)          {}
void TMyMotor::Gali_JogNSetup(int)     {}
void TMyMotor::Gali_JogNAndCount(int, int) {}
bool TMyMotor::ISNormal()                 { return (Motor==NULL); }   // AI(W6.4-TESTER) 20260626: offline (no backend) -> axis treated normal/in-position
bool TMyMotor::ISZ1Up_Z2Down()            { return false; }
bool TMyMotor::ISZ1Down_Z2Up()            { return false; }
bool TMyMotor::ISZ1Up_Z2DownNoWait()      { return false; }
bool TMyMotor::ISZ1Down_Z2UpNoWait()      { return false; }
bool TMyMotor::Gali_ReadEncoderInRandge(long)            { return false; }
bool TMyMotor::Gali_ReadEncoderInRandgeNoWait(long)      { return false; }
bool TMyMotor::Gali_ReadEncoderOver(long)                { return false; }
bool TMyMotor::Gali_ReadEncoderMaxRandge(long)           { return false; }
bool TMyMotor::Gali_ReadEncoderInRandgeMinLimit(long)    { return false; }
bool TMyMotor::GalilTwoY_Move(int,int,int,AnsiString)    { return (Motor==NULL); }   // AI(W6.4-TESTER) 20260626: offline (no backend) -> Y move reports complete
bool TMyMotor::Gali_ReadEncoderBelowCheckHeight(long)    { return false; }
bool TMyMotor::Gali_nnMode_Z1Z2_Down(int, bool)          { return false; }
bool TMyMotor::Gali_nnMode_Z1Z2_Up(int, bool)            { return false; }
bool TMyMotor::ISZ1Down_Z2Down()                         { return false; }
bool TMyMotor::ISZ1Up_Z2Up()                             { return false; }
#endif // GALI-STUB RETIRED (W906-P0-5)

bool TMyMotor::IsStartGali_Pr()
{
    if (Gali_Command("MG _XQ", __func__) == -1)
        return false;
    return true;
}
void TMyMotor::StartGali_Pr() { Gali_Command("XQ",  __func__); }
void TMyMotor::EndGali_Pr()   { Gali_Command("ST",  __func__); }

long TMyMotor::GetGali_Pr_Result()
{
    AnsiString str;
    if (Mot_Name == MTestY1)      str = "E1=";
    else if (Mot_Name == MTestZ1) str = "E2=";
    else if (Mot_Name == MTestZ2) str = "E3=";
    else if (Mot_Name == MTestY2) str = "E4=";
    return Gali_Command(str, __func__) * (long)Motor->GearRatio;
}

void TMyMotor::GetGali_Pr_ER(long &ERA, long &ERB, long &ERC, long &ERD)
{
    ERA = Gali_Command("ERA=?", __func__) * (long)MOT[MTestY1].Motor->GearRatio;
    ERB = Gali_Command("ERB=?", __func__) * (long)MOT[MTestZ1].Motor->GearRatio;
    ERC = Gali_Command("ERC=?", __func__) * (long)MOT[MTestZ2].Motor->GearRatio;
    if (USE_INDEX_ARM_AXES == IndexArm_3_Axis)
        ERD = 0;
    else
        ERD = Gali_Command("ERD=?", __func__) * (long)MOT[MTestY1].Motor->GearRatio;
}

void TMyMotor::SetGali_Pr_ER(long ERA, long ERB, long ERC, long ERD)
{
    AnsiString str;
    ERA = (ERA * 100) / (long)(MOT[MTestY1].Motor->GearRatio * 100);
    ERB = (ERB * 100) / (long)(MOT[MTestZ1].Motor->GearRatio * 100);
    ERC = (ERC * 100) / (long)(MOT[MTestZ2].Motor->GearRatio * 100);
    if (USE_INDEX_ARM_AXES == IndexArm_3_Axis)
        ERD = 0;
    else
        ERD = (ERD * 100) / (long)(MOT[MTestY1].Motor->GearRatio * 100);
    str.sprintf("ER%ld,%ld,%ld,%ld", ERA, ERB, ERC, ERD);
    Gali_Command(str, __func__);
}

// ---------------------------------------------------------------------------
//  Stubs for methods gated TODO(W6-sensor/state-machine)
// ---------------------------------------------------------------------------
// AI(W906-P0-5) 20260920: 同上面那一批 —— 真本體在 myGALILmotor.cpp。
#if 0 // GALI-STUB RETIRED (W906-P0-5): 真本體在 Motor/myGALILmotor.cpp
bool TMyMotor::CheckPos(bool)                                    { return false; }
bool TMyMotor::CheckYPos()                                       { return false; }
bool TMyMotor::CheckPos_nnMode(bool)                             { return false; }
#endif // GALI-STUB RETIRED (W906-P0-5)
void TMyMotor::MagazineUp()                                      {}
void TMyMotor::MagazineDown()                                    {}
// ===========================================================================
//  AI(W906-HOME-Y) 20260923: CheckYPosWhenZDown / CheckArmPosInRange / CheckArmPosArrival
//  真本體 —— golden Motor/mymotor.cpp:5007-5195 逐字（cp950 讀入，只轉 EOL）。
//
//  原本這三個是 W4 波次留下的 `{ return false; }` 空樁；Motor/myGALILmotor.cpp:148
//  的 SATISFIED-BY-SUBSTRATE 清單卻把它們寫成「已經是真的」，所以一直沒人補。
//
//  後果（20260923 gdb 實測）：這台 CosFunction.bIndexProtect=true，
//  Gali_MotMove（myGALILmotor.cpp:~1953）每次都被 CheckYPosWhenZDown()==false 擋回；
//  USE_INDEX_ARM_AXES==IndexArm_3_Axis 時 GalilTwoY_Move 就是 Gali_MotMove，
//  所以 ProcessMotorHome（uhome.cpp case 710 的 GalilTwoY_Move(-10,10,...)）永遠
//  停在 fHome->iHomeStep=710 ⇒ 按 START 觸發的歸零永遠不會完成，START／PAUSE 都測不了。
//  BCB 在 SOFT_SIMULTE 下能歸零完成，是因為 golden 這裡是真本體：
//  Y 軸移動前讀 Z 軸編碼器，Z 沒有在下方就放行（return true）。
//
//  #ifdef INDEX_PROTECT_TMOVE 的臂照抄；該巨集在 MachineType.h 是註解掉的，
//  所以裡面的 RecordIndexPosition / bOverRangeDoTMode 不會被編譯（golden 同樣如此）。
// ===========================================================================
#include "cContact.h"                      // CONTACT_AUTO_GET_HEIGHT（golden 經 cmydef/MachineDefine 取得）
bool TMyMotor::CheckYPosWhenZDown(int Pos, int iOrgPos, AnsiString asErrorFunc)
{
    int iZPos=0;
    static int iRetryCnt[4]={0, 0, 0, 0};

    if(CosFunction.bIndexProtect==true)                                         //Steven 20180319 (jou) : 加入Index Y軸移動前確認Z軸位置保護
    {
        if(Mot_Name==MTestY1)
        {
            iZPos=MOT[MTestZ1].Gali_ReadEncoderPos();
            if(iZPos<-200)
            {
                if(iRetryCnt[0]<100)
                {
                    iRetryCnt[0]++;
                    return false;
                }
                ShowIndexMotorError(asErrorFunc+AnsiString("_Y1"));
                return false;
            }
            iRetryCnt[0]=0;
        }
        else if(Mot_Name==MTestY2)
        {
            iZPos=MOT[MTestZ2].Gali_ReadEncoderPos();
            if(iZPos<-200)
            {
                if(iRetryCnt[1]<100)
                {
                    iRetryCnt[1]++;
                    return false;
                }
                ShowIndexMotorError(asErrorFunc+AnsiString("_Y2"));
                return false;
            }
            iRetryCnt[1]=0;
        }
        else if(Mot_Name==MTestZ1)                                              //JerryYang 20180411 (jou) Z軸移動前確認Y軸位置保護
        {
            if(Pos>0)                                                           //前面Pos有加負號，所以>0是指Z軸往下要檢查Y軸
            {
                int iEncoderPos1=MOT[MTestY1].Gali_ReadEncoderPos();
                if(CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Front, IniConfig.GaliPosRange))  //JerryYang 20230309 : 修正index arm移動保護
                {
                    iRetryCnt[2]=0;
                    if(iContactMode!=CONTACT_AUTO_GET_HEIGHT &&                 //Steven 20231109 : Auto Height的時候, 不檢查高度
                       CheckArmPosInRange(iOrgPos, Prod.TestZ1_Pick-1000, 300)==false)
                    {
                        ShowIndexMotorError(asErrorFunc+AnsiString("_Z1_TooLowOnShuttle"), true);
                        return false;
                    }
                }
                else if(CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Middle, IniConfig.GaliPosRange))  //JerryYang 20230309 : 修正index arm移動保護
                {
                    iRetryCnt[2]=0;
                }
                else
                {
                    iRetryCnt[2]++;
                    #ifdef INDEX_PROTECT_TMOVE
                    if(iRetryCnt[2]<50)                                         //Ifor 20240719 : Add index Y protection
                    {
                        return false;
                    }

                        if(bOverRangeDoTMode==false)
                            RecordIndexPosition(1, 3);
                        bOverRangeDoTMode=true;                                 //觸發做Tmode
                    #else
                    if(iRetryCnt[2]<100)
                    {
                        return false;
                    }
                    ShowIndexMotorError(asErrorFunc+AnsiString("_Z1"), true);
                        #endif
                    iRetryCnt[2]=0;
                    return false;
                }
            }
        }
        else if(Mot_Name==MTestZ2)
        {
            if(Pos>0)                                                           //前面Pos有加負號，所以>0是指Z軸往下要檢查Y軸
            {
                if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                         //JimmyChiu 20220708 : add Index Arm Axis
                {
                    int iEncoderPos1=MOT[MTestY1].Gali_ReadEncoderPos();
                    if(CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Front,  IniConfig.GaliPosRange))  //JerryYang 20230309 : 修正index arm移動保護  //Jimmychiu 20230628 : 修正index arm移動保護
                    {
                        iRetryCnt[2]=0;
                    }
                    else if(CheckArmPosArrival(iEncoderPos1, Prod.TestY1_Middle, IniConfig.GaliPosRange))  //JerryYang 20230309 : 修正index arm移動保護
                    {
                        iRetryCnt[2]=0;
                        if(iContactMode!=CONTACT_AUTO_GET_HEIGHT &&             //Steven 20231109 : Auto Height的時候, 不檢查高度
                           CheckArmPosInRange(iOrgPos, Prod.TestZ2_Pick-1000, 300)==false)
                        {
                            ShowIndexMotorError(asErrorFunc+AnsiString().sprintf("_Z2_TooLowOnShuttle_OrgPos:%d",iOrgPos), true);
                            return false;
                        }
                    }
                    else
                    {
                        if(iRetryCnt[2]<100)
                        {
                            iRetryCnt[2]++;
                            return false;
                        }
                        iRetryCnt[2]=0;
                        ShowIndexMotorError(asErrorFunc+AnsiString("_Z2"), true);
                        return false;
                    }
                }
                else
                {
                    int iEncoderPos1=MOT[MTestY2].Gali_ReadEncoderPos();
                    if(CheckArmPosArrival(iEncoderPos1, Prod.TestY2_Rear, IniConfig.GaliPosRange))  //JerryYang 20230309 : 修正index arm移動保護
                    {
                        iRetryCnt[3]=0;
                        if(iContactMode!=CONTACT_AUTO_GET_HEIGHT &&             //Steven 20231109 : Auto Height的時候, 不檢查高度
                           CheckArmPosInRange(iOrgPos, Prod.TestZ2_Pick-1000, 300)==false)
                        {
                            ShowIndexMotorError(asErrorFunc+AnsiString().sprintf("_Z2_TooLowOnShuttle_OrgPos:%d",iOrgPos));
                            return false;
                        }
                    }
                    else if(CheckArmPosArrival(iEncoderPos1, Prod.TestY2_Middle, IniConfig.GaliPosRange))  //JerryYang 20230309 : 修正index arm移動保護
                    {
                        iRetryCnt[3]=0;                                         //Jimmychiu 20230710 : 修正index arm移動保護
                    }
                    else
                    {
                        #ifdef INDEX_PROTECT_TMOVE
                        if(iRetryCnt[3]>50)
                        {
                            if(bOverRangeDoTMode==false)
                                RecordIndexPosition(2, 3);
                            bOverRangeDoTMode=true;                             //觸發做Tmode
                        }
                        #else
                        if(iRetryCnt[3]<100)
                        {
                            iRetryCnt[3]++;
                            return false;
                        }
                        #endif
                        iRetryCnt[3]=0;
                        ShowIndexMotorError(asErrorFunc+AnsiString("_Z2"), true);
                        return false;
                    }
                }
            }
        }
    }
    return true;
}
//----------------------------------------------------------------------------
bool TMyMotor::CheckArmPosInRange(int iNowPos, int iMin, int iMax)
{
    int iTemp;
    if(iMin>iMax)
    {
        iTemp=iMax;
        iMax=iMin;
        iMin=iTemp;
    }

    if(iMin<=iNowPos &&                                                         //JerryYang 20230309 : 修正index arm移動保護
       iNowPos<=iMax)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//----------------------------------------------------------------------------
bool TMyMotor::CheckArmPosArrival(int iNowPos, int iDestination, int iTolerance)
{
    if(CheckArmPosInRange(iNowPos, iDestination-iTolerance, iDestination+iTolerance))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool TMyMotor::CheckIndexYPos(bool)                              { return false; }
bool TMyMotor::Check_SHUTTLE_FLOODGATE_Staste(int)               { return false; }
void TMyMotor::TrayArmInitial()                                  {}
bool TMyMotor::Check_Y1Y2_TargetPosWillCrash(int,int)           { return false; }
bool TMyMotor::CheckY1Y2TargetPos(int,AnsiString)                { return false; }
bool TMyMotor::Check_Y1_TargetPosWillCrash(int,AnsiString)      { return false; }
bool TMyMotor::Check_Y2_TargetPosWillCrash(int,AnsiString)      { return false; }
bool TMyMotor::Check_Y1_TargetPosInTeachPos(int,AnsiString)     { return false; }
bool TMyMotor::Check_Y2_TargetPosInTeachPos(int,AnsiString)     { return false; }

// ===========================================================================
//  TTrayMotor
// ===========================================================================

TTrayMotor::TTrayMotor()
{
    fHasTray = false;
    fHTary   = false;
    pHTray   = NULL;    // AI(W4): VCL widget not available; always NULL in W4 sim
    Tray.SetXYItem(1, 1);
    ClearTray(__func__);
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetHTrayPanel(TTMyTray *ptr)
{
    //AI(W906-HTRAY) 20260930: golden body restored verbatim (golden Motor/mymotor.cpp:1500-1504).  fHTary is MACHINE STATE, not a widget
    //  flag: SetTray (:2389-2397 = golden :1506-1514) fills the IC grid only `if(fHTary)`, so the old no-op left every supplied tray
    //  "fHasTray=true, 0 IC" (INBOX 121, docs/AMBIENT_STALL_HTRAY_20260930.md).  ptr is NULL in this tree (no TTMyTray widget) and that is
    //  safe: no live code dereferences pHTray (every `pHTray->` is commented out or inside #if 0 :2074-2323; HasIC :1965 only compares it).
    fHTary=true;
    pHTray=ptr;
}

// ---------------------------------------------------------------------------
bool TTrayMotor::HasIC()    // kevin 20130509
{
    if (fHasTray == false)
        return false;

    if (pHTray != NULL)
    {
        // pHTray->Name comparison -- gated (VCL; W7)
        if (Tray.HasIC())
            return true;
    }
    else if (Tray.HasIC())
    {
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
bool TTrayMotor::HasCleanPad()  // kevin 20150505
{
    if (fHasTray == false)
        return false;
    // pHTray name checks gated (VCL; W7) -- always false in W4 sim
    return false;
}

// ---------------------------------------------------------------------------
bool TTrayMotor::HasRealIC()
{
    if (fHasTray == false)
        return false;
    if (Tray.HasRealIC())
        return true;
    return false;
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetTrayBinData(int x, int y, int data, AnsiString iInfo)
{
    Tray.Data[x][y] = data;
    if (fHTary)
    {
        if (data >= 1000)
            data -= 1000;
        // pHTray->SetCellColorIndex(x, y, data);   // TODO(W7-UI)
        // pHTray->SetCellNumber(x, y, iInfo);       // TODO(W7-UI)
    }
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetTraySingleData(int x, int y, int data, int iTarget)
{
    Tray.Data[x][y]    = data;
    Tray.iTarget[x][y] = iTarget;
    if (fHTary)
    {
        if (data >= 1000)
            data -= 1000;
        // pHTray->SetCellColorIndex(x, y, data);   // TODO(W7-UI)
    }
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetTraySiteMap(int x, int y, int iSiteMap)     // Steven 20220510
{
    SetTraySingleData(x, y, HAS_IC);
    Tray.iWhichSite[x][y] = iSiteMap;
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetNullIcToHasNullIc()
{
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = 0; j < Tray.YItem; j++)
            if (Tray.Data[i][j] == NULL_IC)
                SetTraySingleData(i, j, HAS_NULL_IC);
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetNullIcToHasIc()    // Sam 20240424
{
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = 0; j < Tray.YItem; j++)
            if (Tray.Data[i][j] == NULL_IC)
                SetTraySingleData(i, j, HAS_IC);
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetTrayBufferSingleData(int x, int y, int data)
{
    Tray.BufferData[x][y] = data;
}

// ---------------------------------------------------------------------------
void TTrayMotor::Refresh()
{
    if (fHTary)
    {
        // pHTray->XBlockItem = Tray.XBItem; ... TODO(W7-UI)
    }
}

// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- InitNewTray
//  golden HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp:1192-1439  (248 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN InitNewTray Motor/mymotor.cpp:1192-1439
void TTrayMotor::InitNewTray(int data, bool bShowSiteMapFlag, AnsiString Func)
{
    int iCount;
    Tray.ClearData();                                                           //Steven 20200619 : 加上保護
    Tray.SetData(data);

    AnsiString Str;
    Str.sprintf("Initial new tray [%s] with IC type %s by function %s ", Alias, sIC_Type[data], Func);
    MNetLog(Str);

    if(fHTary)
    {
        pHTray->XBlockItem=Tray.XBItem;                                         //Frank 20160928 add Subtray Function
        pHTray->YBlockItem=Tray.YBItem;

        pHTray->XBlockWidth=Tray.XBWidth;
        pHTray->YBlockWidth=Tray.YBWidth;

        pHTray->XItem=Tray.XItem;
        pHTray->YItem=Tray.YItem;

        bShowSiteMap=bShowSiteMapFlag;
        if(bShowSiteMap)                                                        //Steven 20170302 (wei) : FIFO MODE
        {
            if(IniConfig.bI37_LockLoaderDirection)
            {
                iDirection=IniConfig.iI37_LockLoaderDirection;
            }
            else
            {
                iDirection=TrayForm.Loader.Direction;
            }
            iSiteCount=GetSiteCount();
            for(int i=0; i<MAX_SOCKET_ROW; i++)
            {
                for(int j=0; j<MAX_SOCKET_COL; j++)
                {
                    if(TestIF_File.iSiteMap[i][j]>0)                            //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
                    {
                        if(TestIF_File.iShuttleMode==1)                         //單arm       //Isaac 20210821 : FIFO開site修正
                            bSiteHasTurnOn[TestIF_File.iSiteMap[i][j]]=bTestSiteUse[TestIF_File.iShuttle_Sel][i][j];
                        else                                                    //雙arm
                            bSiteHasTurnOn[TestIF_File.iSiteMap[i][j]]=bTestSiteUse[0][i][j];
                    }
                }
            }
        }

        for(int i=0; i<Tray.XItem; i++)
        {
            for(int j=0; j<Tray.YItem; j++)
            {
                pHTray->SetCellColorIndex(i, j, data);
                if(bShowSiteMap==false)                                         //Steven 20170302 (wei) : FIFO MODE
                    pHTray->SetCellNumber(i, j, "");
            }
        }

        if(bShowSiteMap)                                                        //Steven 20170302 (wei) : FIFO MODE
        {
            iCount=1;
            // ----   左至右,上至下
            //  /
            // --->
            if(iDirection==0)
            {
                for(int j=0; j<Tray.YItem; j++)
                {
                    for(int i=0; i<Tray.XItem; i++)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // ----   右至左,上至下
            //  \\
            // <---
            else if(iDirection==1)
            {
                for(int j=0; j<Tray.YItem; j++)
                {
                    for(int i=Tray.XItem-1; i>=0; i--)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // --->   左至右,下至上
            //  \\
            // ----
            else if(iDirection==2)
            {
                for(int j=Tray.YItem-1; j>=0; j--)
                {
                    for(int i=0; i<Tray.XItem; i++)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // <---   右至左,下至上
            //   /
            // ----
            else if(iDirection==3)
            {
                for(int j=Tray.YItem-1; j>=0; j--)
                {
                    for(int i=Tray.XItem-1; i>=0; i--)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // |   | 上至下, 左至右
            // | / |
            // |   V
            else if(iDirection==4)
            {
                for(int i=0; i<Tray.XItem; i++)
                {
                    for(int j=0; j<Tray.YItem; j++)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // |   ^ 下至上, 左至右
            // | \\|
            // |   |
            else if(iDirection==5)
            {
                for(int i=0; i<Tray.XItem; i++)
                {
                    for(int j=Tray.YItem-1; j>=0; j--)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // |   | 上至下, 右至左
            // | \\|
            // V   |
            else if(iDirection==6)
            {
                for(int i=Tray.XItem-1; i>=0; i--)
                {
                    for(int j=0; j<Tray.YItem; j++)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
            // ^   | 下至上, 右至左
            // | / |
            // |   |
            else if(iDirection==7)
            {
                for(int i=Tray.XItem-1; i>=0; i--)
                {
                    for(int j=Tray.YItem-1; j>=0; j--)
                    {
                        while(bSiteHasTurnOn[iCount]==false)
                        {
                            iCount++;
                            if(iCount>iSiteCount)
                                iCount=1;
                        };
                        pHTray->SetCellNumber(i, j, iCount);
                        Tray.iWhichSite[i][j]=iCount;
                        iCount++;
                        if(iCount>iSiteCount)
                            iCount=1;
                    }
                }
            }
        }
    }
}
#endif // AI-W6C-GOLDEN-END InitNewTray Motor/mymotor.cpp:1192-1439

// ---------------------------------------------------------------------------
void TTrayMotor::InitNewTray(int data, bool bShowSiteMapFlag, AnsiString Func)
{
    int iCount;
    Tray.ClearData();
    Tray.SetData(data);

    AnsiString Str;
    Str.sprintf("Initial new tray [%s] with IC type %s by function %s ",
                Alias.c_str(), sIC_Type[data].c_str(), Func.c_str());
    MNetLog(Str);

    if (fHTary)
    {
        // VCL pHTray layout updates TODO(W7-UI)
        bShowSiteMap = bShowSiteMapFlag;
        if (bShowSiteMap)
        {
            // iDirection / iSiteCount / bSiteHasTurnOn setup -- needs
            // IniConfig / TrayForm / GetSiteCount / bTestSiteUse -> TODO(W6)
            // iCount used here in the full body; unused in the W4 stub.   //AI(W906-HTRAY) 20260930: this branch is REACHED now (fHTary is set at boot): FIFO mode (asendic_Loader.cpp:1304 / :1351) still gets no Tray.iWhichSite (golden :1215-1437), which Find_InArm_Single_FIFO_SiteOrder matches (ainarm9045.cpp:9939-10072) -- pre-existing gap, INBOX 121
        }
    }
    (void)iCount;   // W4 stub: iCount used in gated FIFO site-map loop TODO(W6)
}

// ---------------------------------------------------------------------------
void TTrayMotor::ClearTray(AnsiString Func)
{
    AnsiString Str;
    if (fHasTray)
    {
        Str.sprintf("Clear tray [%s] by function %s ", Alias.c_str(), Func.c_str());
        MNetLog(Str);
    }

    fHasTray      = false;
    iIsCoverTray  = NULL_IC;    // JerryYang 20240318
    sTrayID       = "";
    Tray.ClearData();
    Tray.cTrayID  = "";         // JerryYang 20250120

    if (fHTary)
    {
        // pHTray layout reset TODO(W7-UI)
    }
    sUnloaderAlarmMsg = "";     // Jimmychiu 20240902
}

// ---------------------------------------------------------------------------
void TTrayMotor::InitEmptyTray(AnsiString Func)
{
    Tray.ClearData();
    AnsiString Str;
    Str.sprintf("Initial empty tray [%s] by function %s ", Alias.c_str(), Func.c_str());
    MNetLog(Str);

    if (fHTary)
    {
        // pHTray layout TODO(W7-UI)
    }
}

// ---------------------------------------------------------------------------
void TTrayMotor::SetTray(int data, AnsiString Func)
{
    fHasTray = true;
    if (fHTary)
    {
        fHasTray = true;
        InitNewTray(data, false, Func);
    }
}

// ---------------------------------------------------------------------------
bool TTrayMotor::UpHalfIsFull()
{
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = 0; j < (Tray.YItem / 2); j++)
            if (Tray.Data[i][j] == NULL_IC)
                return false;
    return true;
}

// ---------------------------------------------------------------------------
bool TTrayMotor::DownHalfIsFull()
{
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = (Tray.YItem / 2); j < Tray.YItem; j++)
            if (Tray.Data[i][j] == NULL_IC)
                return false;
    return true;
}

// ---------------------------------------------------------------------------
int TTrayMotor::WhichBufferIsFull()     // JerryYang 20221215: Magazine
{
    int iCnt[5] = {0, 0, 0, 0, 0};
    for (int k = 0; k < 5; k++)
    {
        iCnt[k] = 0;
        for (int i = 0; i < Tray.XItem; i++)
            for (int j = 0; j < Tray.YItem; j++)
                if (j >= k * iYRegNum && j < (k + 1) * iYRegNum &&
                    Tray.Data[i][j] == NULL_IC)
                    iCnt[k]++;

        if (iCnt[k] < 8)
            return k * 3;
    }
    return -1;
}

// ---------------------------------------------------------------------------
void TTrayMotor::MoveTrayAllItem(class TTrayMotor *Source)   // Sam 20240108
{
    if (Source->fHasTray == false)
        return;
    fHasTray = true;

    int MinRow = (Source->Tray.XItem >= Tray.XItem) ? Tray.XItem : Source->Tray.XItem;
    int MinCol = (Source->Tray.YItem >= Tray.YItem) ? Tray.YItem : Source->Tray.YItem;

    for (int i = 0; i < MinRow; i++)
    {
        for (int j = 0; j < MinCol; j++)
        {
            SetTraySingleData(i, j, Source->Tray.Data[i][j]);
            Tray.iWhichSite[i][j]   = Source->Tray.iWhichSite[i][j];
            Tray.iNeedRotAng[i][j]  = Source->Tray.iNeedRotAng[i][j];
            Tray.iCurrRotAng[i][j]  = Source->Tray.iCurrRotAng[i][j];
            Tray.iWhichIndex[i][j]  = Source->Tray.iWhichIndex[i][j];
            Tray.iBinCode[i][j]     = Source->Tray.iBinCode[i][j];
            Tray.BufferData[i][j]   = Source->Tray.BufferData[i][j];
            Tray.iTarget[i][j]      = Source->Tray.iTarget[i][j];
            Tray.iCleanCount[i][j]  = Source->Tray.iCleanCount[i][j];
            Tray.bFliped[i][j]      = Source->Tray.bFliped[i][j];
            Tray.cDeviceInf[i][j]   = Source->Tray.cDeviceInf[i][j];
            Tray.b2DIDNG[i][j]      = Source->Tray.b2DIDNG[i][j];
            Tray.cReDeviceInf[i][j] = Source->Tray.cReDeviceInf[i][j];
            Tray.iAOIResult[i][j]   = Source->Tray.iAOIResult[i][j];  // Sam 20240325
        }
    }
    sUnloaderAlarmMsg = Source->sUnloaderAlarmMsg;  // Jimmychiu 20240902
    Source->ClearTray(__func__);
}

// ---------------------------------------------------------------------------
int TTrayMotor::HowManyDevice(int iType)    // Ifor 20160829
{
    int iCT = 0;
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = 0; j < Tray.YItem; j++)
            if (Tray.Data[i][j] == iType)
                iCT++;
    return iCT;
}

// ---------------------------------------------------------------------------
int TTrayMotor::HowManyDevice()             // Steven 20190627
{
    int iCT = 0;
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = 0; j < Tray.YItem; j++)
            if (Tray.Data[i][j] != NULL_IC && Tray.Data[i][j] != HAS_NULL_IC)
                iCT++;
    return iCT;
}

// ---------------------------------------------------------------------------
void TTrayMotor::MoveTrayData(TTrayMotor &TrayMotor)     // JerryYang 20221215: Magazine
{
    MoveTrayAllItem(&TrayMotor);
}

// ---------------------------------------------------------------------------
//  TTrayMotor stubs for methods with VCL/sensor coupling (gated TODO)
// ---------------------------------------------------------------------------
bool TTrayMotor::SearchHasEmpryToPlace(int)     { return false; }
bool TTrayMotor::TrayFeedHasIC()                { return false; }
void TTrayMotor::SetHasNullIcToNullIc()
{
    for (int i = 0; i < Tray.XItem; i++)
        for (int j = 0; j < Tray.YItem; j++)
            if (Tray.Data[i][j] == HAS_NULL_IC)
                SetTraySingleData(i, j, NULL_IC);
}
bool TTrayMotor::HasOnlyDataICAndNullIC(int DataType)
{
    return Tray.HasOnlyDataICAndNullIC(DataType);
}

// ===========================================================================
//  Free function stubs  -- state-machine / sensor coupling; TODO(W6)
// ===========================================================================
// AI(W906-B1-INARM) 20260924: RETIRED stub InArmContinuousMove_9045 -- real body at end of file (golden Motor/mymotor.cpp:2886-3325)
// AI(W906-B1-INARM) 20260924: RETIRED stub InArmPitchMove -- real body at end of file (golden Motor/mymotor.cpp:2586-2717)
// AI(W906-B1-INARM) 20260924: RETIRED stub SetInArmPitchSpeed -- real body at end of file (golden Motor/mymotor.cpp:2560-2584)
// AI(W906-B1-INARM) 20260924: RETIRED stub InArmCynMove -- real body at end of file (golden Motor/mymotor.cpp:2721-2749)
// AI(W906-B1-INARM) 20260924: RETIRED stub InArmZMoveDown -- real body at end of file (golden Motor/mymotor.cpp:2751-2832)
// AI(W906-B1-INARM) 20260924: RETIRED stub InArmZMoveUp -- real body at end of file (golden Motor/mymotor.cpp:2834-2882)

// AI(W906-P0-7) 20260921: OutArm 連續移動家族的五個樁**退休了** ——
//   真本體翻在本檔尾端（golden Motor/mymotor.cpp 共 731 行）：
//       OutArmPitchMove            golden :3327-3454  128 行
//       OutArmCynMove              golden :3938-3965   28 行
//       OutArmZMoveDown            golden :3967-4044   78 行
//       OutArmZMoveUp              golden :4046-4090   45 行
//       OutArmContinuousMove_9045  golden :4094-4545  452 行
//   放尾端的理由與 TrayArmMotorMove 相同：本體要 `MyMessageBox`
//   （mymessbox_shim.h）與 `fMotorTest`（forms/fMotorTest.h），
//   那兩個 header 在本檔是 :2764 之後才 include 的。
//
// ⛔ 為什麼五支要一起退：`OutArmContinuousMove_9045` 的**成功路徑會呼叫另外四支**
//   （golden :4499 `bFlag[0]=OutArmZMoveUp(...)`、:4517 `bFlag[3]=OutArmPitchMove(...)`、
//    :4525 `bFlag[4]=OutArmCynMove(...)`、:4531 `bRet=OutArmZMoveDown(...)`）。
//   只翻它一支，那四個樁照樣回 false ⇒ 它永遠到不了 `return true`。
//   ⚠ 這不是預防性的一起退 —— 是讀過三個 `return true` 的路徑才決定的。
//
// ⚠ **InArm 那一組鏡像的五支（golden 732 行）當時刻意沒動** —— AI(W906-B1-INARM) 20260924 已翻（本檔尾端，連同 PCIL112_InArmXYMove 那一對）。P0-7 當時的理由：
//   歸零 case 1540 的 flag1/flag3 走的是 `MOT[MInArmX/Y].MotorMove(...)`，
//   不是 `InArmContinuousMove_9045`。它們是另一個波次，要有自己的量測。

bool SortArmContinuousMove(int,int,int,bool[][MAX_ARM_Col],int[][MAX_ARM_Col],bool,bool) { return false; }
bool SortArmPitchMove(int)             { return false; }
void InitPCIL112_SortArmXYMoveTask()   {}
int  PCIL112_SortArmXYMove(int,int)    { return 0; }
// AI(W906-ZSAFE) 20260927: RETIRED stub sSortArmZHomeState（以前回常數）—— real body at end of file (golden :6336-6358)
bool SortArmZMoveDown(bool[][MAX_ARM_Col],int[][MAX_ARM_Col],bool,bool) { return false; }
bool SortArmZMoveUp(int, bool)         { return false; }

bool CatchMgzTrayMove(int)             { return false; }
// AI(W906-PT-W3-integrate) 20260808: `void OpenPCI132Card(bool) {}` RETIRED from this
//   stub block.  Motor/myMN200motor.cpp landed in wave PT-W3 with golden's real 295-line
//   body (port :1154, golden Motor/myMN200motor.cpp:855) and both are in ht9045_motor, so
//   keeping this one produced `multiple definition of OpenPCI132Card(bool)`.  Same shape
//   as PT-W2's uPlateInfo retirement: the stub was standing in for an unported unit, and
//   the unit arrived.  NOTE this is a real behaviour change on the MN200 ring path --
//   ring bring-up now actually runs instead of returning immediately.
// AI(W906-B20-SERVOON) 20261001: RETIRED stub ServoOnAllMOT (was `{}`) -- real body at end of file (golden Motor/mymotor.cpp:4609-4621; census 129 E-BOOT-002)
// AI(W906-ZSAFE) 20260927: RETIRED stub OutArmZSafe（以前回常數）—— real body at end of file (golden :2280-2323)
// AI(W906-ZSAFE) 20260927: RETIRED stub InArmZSafe（以前回常數）—— real body at end of file (golden :2154-2194)
// AI(W906-ZSAFE) 20260927: RETIRED stub SortArmZSafe（以前回常數）—— real body at end of file (golden :2325-2365)
// AI(W906-ZSAFE) 20260927: RETIRED stub sInArmZHomeState（以前回常數）—— real body at end of file (golden :2196-2220)
// AI(W906-ZSAFE) 20260927: RETIRED stub sOutArmZHomeState（以前回常數）—— real body at end of file (golden :2222-2247)
// AI(W906-B1-INARM) 20260924: RETIRED stub InitPCIL112_InArmXYMoveTask -- real body at end of file (golden Motor/mymotor.cpp:4640-4646)
// AI(W906-B1-INARM) 20260924: RETIRED stub PCIL112_InArmXYMove -- real body at end of file (golden Motor/mymotor.cpp:4648-4712)
// AI(W906-AMB-L2) 20260929: RETIRED stub InitPCIL112_OutArmXYMoveTask (was `{}`) -- real body at end of file (golden Motor/mymotor.cpp:4714-4720)
// AI(W906-AMB-L2) 20260929: RETIRED stub PCIL112_OutArmXYMove (was `{ return 0; }` = PNP_DONE, "arrived" with X/Y never commanded) -- real body at end of file (golden Motor/mymotor.cpp:4722-4821)
// AI(W906-ZSAFE) 20260927: RETIRED stub CheckOutArmZNeedHome（以前回常數）—— real body at end of file (golden :2250-2278)
// AI(W906-ZSAFE) 20260927: RETIRED stub CheckInArmZNeedHome（以前回常數）—— real body at end of file (golden :2127-2152)
// AI(W906-IDXERR) 20260927: RETIRED empty stub ShowIndexMotorError —— real body at end of file (golden :261-299)
AnsiString SaveLog(AnsiString)         { return ""; }
void RecordIndexPositionError(AnsiString,bool,bool,bool,bool,long*) {}
bool TrayArmContinuousMoveForOCR(int,int) { return false; }
bool TrayMoveHome()                    { return false; }
bool ShuttleSensorContinuousMove(int,int,bool) { return false; }
int  CheckOutArmDestory()              { return 0; }
// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- RecordIndexPosition
//  golden HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp:5318-5424  (107 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN RecordIndexPosition Motor/mymotor.cpp:5318-5424
void RecordIndexPosition(int iArm,int Part)                                     //Isaac 20200922 : 紀錄indexArmY encoder值和command值
{
    AnsiString Strtemp="",StrType="";
    AnsiString sFileName="",sFileName2="",StrIndexLog="";
    GetTimeInfo();

    int y1CmdPos     = MOT[MTestY1].Gali_ReadPos();
    int y1EncoderPos = MOT[MTestY1].Gali_ReadEncoderPos();
    int y2CmdPos     = (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?0:MOT[MTestY2].Gali_ReadPos();
    int y2EncoderPos = (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?0:MOT[MTestY2].Gali_ReadEncoderPos();
    int z1CmdPos     = MOT[MTestZ1].Gali_ReadPos();
    int z1EncoderPos = MOT[MTestZ1].Gali_ReadEncoderPos();
    int z2CmdPos     = MOT[MTestZ2].Gali_ReadPos();
    int z2EncoderPos = MOT[MTestZ2].Gali_ReadEncoderPos();
    if(Part==3)                                                                 //Error
    {
        if(iArm==0)                                                             //Handler status Halt
        {
            StrType="Halt";
            sFileName2.sprintf("%s\\%04d\\%02d_IndexPosLog\\IndexArmHalt_%04d%02d%02d%02d%02d%02d.logs", "D:\\HT9045_Log\\IndexPos",SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        }
        else if(iArm==3)                                                        //Gali_Two_ZAxis_Move
        {
            StrType="Gali_Two_ZAxis_Move";
            sFileName2.sprintf("%s\\%04d\\%02d_IndexPosLog\\IndexArm12_%04d%02d%02d%02d%02d%02d.logs", "D:\\HT9045_Log\\IndexPos",SystemYear, SystemMonth, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        }
        else                                                                    //Arm1,Arm2
        {
            StrType.sprintf("IndexArm%d",iArm);
            sFileName2.sprintf("%s\\%04d\\%02d_IndexPosLog\\IndexArm%d_%04d%02d%02d%02d%02d%02d.logs", "D:\\HT9045_Log\\IndexPos",SystemYear, SystemMonth, iArm, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
        }

        Strtemp.sprintf("%s :Y1CMD:%d, Y1POS:%d, Teach F:%d, M:%d, Y2CMD:%d, Y2POS:%d, Teach M:%d, R:%d, Z1CMD:%d, Z1POS:%d, Z2CMD:%d, Z2POS:%d",
                StrType,
                y1CmdPos, y1EncoderPos, Prod.TestY1_Front, Prod.TestY1_Middle,
                y2CmdPos, y2EncoderPos, Prod.TestY2_Middle, Prod.TestY2_Rear,
                z1CmdPos, z1EncoderPos,
                z2CmdPos, z2EncoderPos);
        fMain->AddIndexPosLog(Strtemp);
        fMain->AddIndexPosLog("Save Log", true);
    }
    else                                                                        //Part : Shuttle==0 or Socket==1
    {
        if(fMain->cbBoth->Checked)                                              //Record Both Arm1 and Arm2
        {
            if (iArm == 1)
            {
                if (Part == 0)  // Shuttle
                {
                    Strtemp.sprintf("Arm1 on Shuttle : Teach:%d, Y1CMD:%d, Y1POS:%d, Y2CMD:%d, Y2POS:%d",
                                    Prod.TestY1_Front, y1CmdPos, y1EncoderPos, y2CmdPos, y2EncoderPos);
                }
                else if (Part == 1)  // Socket
                {
                    Strtemp.sprintf("Arm1 on Socket : Teach:%d, Y1CMD:%d, Y1POS:%d, Y2CMD:%d, Y2POS:%d",
                                    Prod.TestY1_Middle, y1CmdPos, y1EncoderPos, y2CmdPos, y2EncoderPos);
                }
            }
            else  // iArm == 2
            {
                if (Part == 0)  // Shuttle
                {
                    Strtemp.sprintf("Arm2 on Shuttle : Teach:%d, Y1CMD:%d, Y1POS:%d, Y2CMD:%d, Y2POS:%d",
                                    Prod.TestY2_Rear, y1CmdPos, y1EncoderPos, y2CmdPos, y2EncoderPos);
                }
                else if (Part == 1)  // Socket
                {
                    Strtemp.sprintf("Arm2 on Socket : Teach:%d, Y1CMD:%d, Y1POS:%d, Y2CMD:%d, Y2POS:%d",
                                    Prod.TestY2_Middle, y1CmdPos, y1EncoderPos, y2CmdPos, y2EncoderPos);
                }
            }
        }
        else if (fMain->cbArm1->Checked && iArm == 1)  // Record Arm1 only
        {
            if (Part == 0)  // Shuttle
            {
                Strtemp.sprintf("Arm1 on Shuttle : Teach:%d, Y1CMD:%d, Y1POS:%d",
                                Prod.TestY1_Front, y1CmdPos, y1EncoderPos);
            }
            else if (Part == 1)  // Socket
            {
                Strtemp.sprintf("Arm1 on Socket : Teach:%d, Y1CMD:%d, Y1POS:%d",
                                Prod.TestY1_Middle, y1CmdPos, y1EncoderPos);
            }
        }
        else if (fMain->cbArm2->Checked && iArm == 2)  // Record Arm2 only
        {
            if (Part == 0)  // Shuttle
            {
                Strtemp.sprintf("Arm2 on Shuttle : Teach:%d, Y2CMD:%d, Y2POS:%d",
                                Prod.TestY2_Rear, y2CmdPos, y2EncoderPos);
            }
            else if (Part == 1)  // Socket
            {
                Strtemp.sprintf("Arm2 on Socket : Teach:%d, Y2CMD:%d, Y2POS:%d",
                                Prod.TestY2_Middle, y2CmdPos, y2EncoderPos);
            }
        }
        else
        {
            return;
        }

        fMain->AddIndexPosLog(Strtemp);
    }
    return;
}
#endif // AI-W6C-GOLDEN-END RecordIndexPosition Motor/mymotor.cpp:5318-5424

void RecordIndexPosition(int, int)     {}
// ===========================================================================
//AI(W906-PT-W6c) 20260810: GOLDEN TEXT RESTORED (GATED) -- EncoderTeachingMaxMinCount
//  golden HT9011UC_Code_V3.33.906.0_20260618/Motor/mymotor.cpp:5426-5499  (74 lines)
//  Census scored this function "translated" on name match only; the LIVE body
//  below is an abbreviated stand-in and golden's text existed nowhere in the
//  tree. The block inside the gate is golden's body transcribed VERBATIM
//  (cp950 -> UTF-8; byte-exact when re-encoded to cp950) and is INACTIVE.
//  The LIVE body that follows is UNCHANGED and remains the only active
//  definition -- net behaviour delta = 0. NOTHING was added inside the gate,
//  so a later un-gate is mechanical.
// ===========================================================================
#if 0 // AI-W6C-GOLDEN-BEGIN EncoderTeachingMaxMinCount Motor/mymotor.cpp:5426-5499
void EncoderTeachingMaxMinCount(int iRecordArm)                                 //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值
{
    int check1=0, check2=0;
    int iEncoderPos=0;

    if(iRecordArm==1)                                                           //Arm1
    {
        iEncoderPos=MOT[MTestY1].Gali_ReadEncoderPos();

        check1=abs(iEncoderPos-Prod.TestY1_Front);
        check2=abs(iEncoderPos-Prod.TestY1_Middle);

        if(check1>5000)                                                         //Middle
        {
            if(check2>iMaxTeachY1M)
            {
                iMaxTeachY1M=check2;
            }
            else if(check2<iMinTeachY1M)
            {
                iMinTeachY1M=check2;
            }
        }
        else                                                                    //Front
        {
            if(check1>iMaxTeachY1F)
            {
                iMaxTeachY1F=check1;
            }
            else if(check1<iMinTeachY1F)
            {
                iMinTeachY1F=check1;
            }
        }
    }
    else if(iRecordArm==2)                                                      //Arm2
    {
        if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)
        {
            iEncoderPos=0;
        }
        else
        {
            iEncoderPos=MOT[MTestY2].Gali_ReadEncoderPos();
        }

        check1=abs(iEncoderPos-Prod.TestY2_Middle);
        check2=abs(iEncoderPos-Prod.TestY2_Rear);

        if(check1>5000)                                                         //Rear
        {
            if(check2>iMaxTeachY2R)
            {
                iMaxTeachY2R=check2;
            }
            else if(check2<iMinTeachY2R)
            {
                iMinTeachY2R=check2;
            }
        }
        else                                                                    //Middle
        {
            if(check1>iMaxTeachY2M)
            {
                iMaxTeachY2M=check1;
            }
            else if(check1<iMinTeachY2M)
            {
                iMinTeachY2M=check1;
            }
        }
    }
    return;
}
#endif // AI-W6C-GOLDEN-END EncoderTeachingMaxMinCount Motor/mymotor.cpp:5426-5499

void EncoderTeachingMaxMinCount(int)   {}
void InitialMaxMinValue(AnsiString)    {}
void TrigerIndexAxisHome()             {}

// ===========================================================================
//  AI(pt-wave) 20260811  PT-W7n  --  BANNER EXTENSION (append-only block)
//
//  ROLE
//    Eight golden FREE FUNCTIONS of this same golden unit plus thirteen of its
//    file-scope globals.  None of the eight is a method; all are TU-scope in
//    golden (golden declares CheckTestZ/1/2 in Motor/myGALILmotor.cpp:52-54 and
//    the other five NOWHERE -- their only callers are inside golden
//    Motor/mymotor.cpp itself).  Who pumps them, in golden:
//      CheckTestZ / CheckTestZ1 / CheckTestZ2   <- 14 sites in golden
//          Motor/myGALILmotor.cpp (Gali_MotMove2 / Gali_MotMoveNoWait /
//          Z1UpZ2Down1/2 / Z1DownZ2Up1/2 / Gali_SingalHome / Gali_FindZPhase /
//          Gali_nnMode_Z1Z2_Down/_Up).  Index-arm four-axis command-vs-encoder
//          agreement gate: a FALSE return makes the Galil layer refuse the move.
//      IndexPosMonitor                          <- golden index-arm callers;
//          pure diagnostic sampler, return value is unconditionally false.
//      CheckInArmZHomeSensor_2x8                <- golden :3104
//          (InArmContinuousMove_9045)
//      CheckOutArmZHomeSensor_2x8               <- golden :4328
//          (OutArmContinuousMove_9045)
//      CheckSortArmZHomeSensor_2x8              <- golden :6220
//          (SortArmContinuousMove)
//          All three: picker-Z HOME-SENSOR SANITY gate taken while the arm is
//          about to descend.  FALSE means 'home sensor says home but the encoder
//          says below zero' -> stop X and Y, tell the operator, re-home.
//      ChangePosition                           <- golden :2937/:2947/:2958/:2968
//          (InArm) and :3500/:3512/:3525/:3537 + :4149/:4161/:4174/:4186 (OutArm)
//          Magnetic-scale linear-interpolation position remap.
//
//  WAVE SCOPE  (one line per golden function assigned to this wave)
//    ACTIVE  CheckTestZ                    golden Motor/mymotor.cpp:67
//    ACTIVE  CheckTestZ1                   golden Motor/mymotor.cpp:117
//    ACTIVE  CheckTestZ2                   golden Motor/mymotor.cpp:173
//    ACTIVE  IndexPosMonitor               golden Motor/mymotor.cpp:228
//            (body ACTIVE; the two fMain->tMonitorIndex->Add() DIAGNOSTIC SINK
//             lines are GATED -- see GATE(PT-W7n-1).  Return value unaffected.)
//    ACTIVE  CheckInArmZHomeSensor_2x8     golden Motor/mymotor.cpp:2394
//    ACTIVE  CheckSortArmZHomeSensor_2x8   golden Motor/mymotor.cpp:2424
//    ACTIVE  CheckOutArmZHomeSensor_2x8    golden Motor/mymotor.cpp:2453
//    ACTIVE  ChangePosition                golden Motor/mymotor.cpp:2485
//    -- globals (storage only; no ctor below reaches any of the 18 NULL globals) --
//    ACTIVE  iIDLETime                     golden :2556
//    ACTIVE  InArmIdle                     golden :2557
//    ACTIVE  bInArmZMove[][]               golden :2720
//    ACTIVE  InArmCylinderDelayTimer       golden :2884   (HTimer -> TQPF_Timer)
//    ACTIVE  OutArmIdle                    golden :3456
//    ACTIVE  bOutArmZMove[][]              golden :3937
//    ACTIVE  OutArmCylinderDelayTimer      golden :4092   (HTimer -> TQPF_Timer)
//    ACTIVE  iTrayXTask                    golden :5653
//    ACTIVE  iTrayOldPos                   golden :5654
//    ACTIVE  SortArmIdle                   golden :5907
//    ACTIVE  iSortArmZMoveTask             golden :5908
//    ACTIVE  bSortArmZMove[][]             golden :5909
//    ACTIVE  SortArmCylinderDelayTimer     golden :6069   (HTimer -> TQPF_Timer)
//
//  HTimer -> TQPF_Timer SUBSTITUTION (golden :2884 / :4092 / :6069)
//    Golden's HTimer is D:\HT9045\elec\Component\htimer.h -- a BCB6 *component
//    package* OUTSIDE the version tree, never translated (same class of finding as
//    halarm.h, recorded at canary_support.h:220-254).  The ONLY `HTimer` this tree
//    has is atester_shims.h:463
//        struct HTimer { bool Off(){ return true; } void SetSecAndOn(double){} };
//    whose Off() is HARD-CODED true.  Typing these three on it would compile, link
//    clean, and make every cylinder dwell expire INSTANTLY -- a silent motion-timing
//    loss, not a visible one.  TQPF_Timer (myTimer.h) is the tree's established
//    substitute for golden timers in exactly this position -- acatchtray.cpp:114,
//    CanBus/cMyDNM100UD.cpp:85, MyPLC/MyPLC_IO_Modbus.cpp:49 -- and its Off() is a
//    real QueryPerformanceCounter deadline test.  Golden itself uses TQPF_Timer for
//    the sibling idle timers a few lines away (:2557 / :3456 / :5907), so the two
//    families already coexist in golden's own text.
//    SURFACE DELTA, stated because it is NOT zero: golden HTimer offers at least
//    Off()/SetSecAndOn(double) (the two members atester_shims.h bothered to mirror);
//    TQPF_Timer offers Off()/SetSecAndOn(double)/SetMSAndOn/SetUSAndOn/
//    Set0_1SecAndOn/On/SetSec/SetMS/SetUS/LatchCycleTime*.  Every member golden
//    calls on these three objects exists; the extra members are additive.
//    BEHAVIOUR DELTA on a virgin object: TQPF_Timer::Off() reads rEnd, which the
//    ctor does NOT initialise, so Off() BEFORE any Set*AndOn() is indeterminate.
//    All three globals below have ZERO call sites in this tree today (measured --
//    see the report), so nothing polls them unarmed.
//
//  GATE REGISTER
//  ------------------------------------------------------------------------
//  GATE(PT-W7n-1) -- `fMain->tMonitorIndex->Add(str);`
//                    golden Motor/mymotor.cpp:242 and :254 (2 sites, both inside
//                    IndexPosMonitor).  Everything else in that function is ACTIVE.
//    (a) GOLDEN LINE / WHAT IT IS
//        golden main.h:1391 `TStringList *tMonitorIndex;` -- a member of TfMain,
//        allocated at golden main.cpp:2227 (`tMonitorIndex = new TStringList();`),
//        drained at golden main.cpp:28550-28551 (SaveToFile then Clear), freed at
//        golden main.cpp:11850-11854 (which guards `if(tMonitorIndex!=NULL)`).
//    (b) WHY THE OFFLINE DEFAULT IS FAITHFUL
//        Golden's IndexPosMonitor declares `bool bRet=false;` at :230 and NEVER
//        assigns it again -- `return bRet` at :258 is UNCONDITIONALLY false on
//        every path, in golden, on a real machine.  So the RETURN VALUE, the only
//        thing a caller can act on, is bit-identical with the Add() present or
//        absent.  The Add() is a pure DIAGNOSTIC SINK: an append-only string log
//        that nothing in golden reads back for a decision.  Skipping it cannot
//        change control flow anywhere.  This is NOT the weaker claim that 'the
//        degraded value happens to be equivalent' -- there is no degraded value
//        here; the observable (bRet) is literally golden's own constant.
//    (c) HOW REAL-MACHINE BEHAVIOUR DIFFERS
//        On a real machine, every sample where the index-arm Z encoder sits below
//        Prod.All_TestZ_Test_Safe appends one line -- "Z1,<z>,Y1,<y>" or
//        "Z2,<z>,Y2,<y>" -- to an in-memory TStringList that golden later writes
//        to disk (golden main.cpp:28550).  Offline that forensic trail is not
//        recorded.  Nothing else differs.
//    ABSENCE CLAIM + COMMAND + TIME  (re-run immediately before hand-off)
//        rg -n "tMonitorIndex" D:/HT9045/HT9011UC_Cpp_V3.33.906.0
//        -> 0 hits.  Run twice: 2026-08-11 03:20 UTC (before writing) and again
//           at hand-off; the second timestamp is in this wave's report.
//        `fMain` DOES exist in this tree (forms/fMain.h facade) -- the absence
//        claim is about the MEMBER tMonitorIndex only, which is why the gate is
//        on the member access and not on fMain.
//    GA-3 HAND-OFF NOTE (trap 4).  When a tMonitorIndex facade lands, do NOT just
//        delete the #if 0.  Golden's line here is UNGUARDED against NULL while
//        golden's own teardown at main.cpp:11850 proves the pointer IS nullable,
//        and in this tree the object would be born NULL like the other late-`new`
//        globals.  Re-activate as `if(fMain && fMain->tMonitorIndex)` at the CALL
//        SITE -- exactly the shape the fLaserSensor/elLaser incident mandates.
//  ------------------------------------------------------------------------
//  NON-GATE 1 -- golden's `#ifdef DEBUG` (:149-154) and `#ifdef DEGBU` (:204-209)
//    blocks are transcribed VERBATIM, golden's typo `DEGBU` included (it is
//    golden's own misspelling of DEBUG at golden :204, so that block is
//    unreachable in EVERY build golden has ever produced).  This build defines
//    neither macro (checked: CMakeLists.txt add_compile_definitions carries only
//    _WIN32_WINNT / WINVER / MN200DLL_EXPORTS / DLLDIR_EX; no -DDEBUG anywhere),
//    so both blocks are inert.  Same treatment as the sibling copy at
//    Motor/myGALILmotor.cpp:832+, deliberately, so the two texts stay diffable.
//    LANDMINE, stated rather than hidden: a `-DDEBUG` build WOULD try to compile
//    `fMain->lbEnCoder0->Caption=S;` and fail -- fMain has no lbEnCoder0/1 in this
//    tree.  That is already true of Motor/myGALILmotor.cpp today.
//  ------------------------------------------------------------------------
//  NON-GATE 2 -- `MOT[iMotNo].Motor->Enable` (golden :2408, :2438, :2470) is kept
//    UNGUARDED, and that is a DELIBERATE FAIL-LOUD choice, not an oversight.
//    MOT[].Motor is NULL in this tree offline: the construction ladder is still
//    behind GATE(W5a-G) in cinitial.cpp, as AutoClean/AutoClean.cpp:909-928 records
//    (that gate exists because retiring the acatchtray_shims no-ops took ctest
//    128 -> 127 with 27-AutoClean SEGFAULTing in both Debug and Release).
//    Adding `MOT[iMotNo].Motor &&` here would make the predicate FALSE offline,
//    i.e. 'no home-sensor fault found', i.e. these three functions would return
//    TRUE and the arm would be cleared to descend.  An always-true home sensor is
//    precisely the silent safety loss this wave is required to avoid; a SEGFAULT
//    is the loud alternative and is therefore the correct one.  Consistent with
//    the rest of the tree, which keeps the same unguarded deref at acarry.cpp:7730/
//    :7781/:8324, aoutarm.cpp:3003/:3060, asortarm.cpp:2238/:2295, ckernel.cpp:3863
//    and cinitial.cpp:3846.  ALL THREE FUNCTIONS HAVE ZERO CALL SITES IN THIS TREE
//    TODAY, so nothing can reach the deref yet -- see this wave's report, trap 1.
//  ------------------------------------------------------------------------
//  NON-GATE 3 -- ShowIndexMotorError / RecordIndexPositionError (called by
//    CheckTestZ/1/2) resolve to the EMPTY STUBS already living in THIS SAME TU at
//    Motor/mymotor.cpp:2252 and :2254.  Not introduced here and not retired here.
//    Consequence, to be explicit: the retry-ceiling ALARM arm of CheckTestZ*
//    (>150 / >100 consecutive failures) is behaviourally SILENT in this build --
//    no operator dialog, no position record.  The load-bearing half is untouched:
//    `iRetryCT` still resets and the function still returns FALSE, which is what
//    the 14 Galil call sites branch on.
//  ------------------------------------------------------------------------
//  NON-GATE 4 -- ShowMyMessage (called by the three home-sensor checks) resolves
//    to canary_support.cpp's RECORDING SIM in ht9045_sm, not to golden's modal VCL
//    dialog.  Golden BLOCKS the operator there; the sim records S1 and returns
//    immediately.  The control-flow half -- the two PCIL132_StopMotor() calls that
//    precede it and the `return false` that follows it -- is fully ACTIVE, so the
//    caller still learns the arm must not descend.  Pre-existing substrate
//    property of the whole tree, restated here because these are SAFETY functions.
// ===========================================================================

// ---------------------------------------------------------------------------
//  AI(pt-wave) 20260811 PT-W7n: includes for THIS BLOCK ONLY, placed here rather
//  than at the file head because this wave is append-only on an existing mirror.
//  Legal at file scope, and it keeps the pre-existing 2471 lines byte-untouched.
//
//  common.h             -- MySleepEx (golden common.h:260; golden reaches it via
//                          its own `#include "common.h"` at golden :15)
//  canary_support.h     -- ShowMyMessage (golden mymessbox.h:58, golden :11) and
//                          __FUNC__ (BCB6 builtin -> __func__).  Same header
//                          Motor/myGALILmotor.cpp:579 uses for the same two.
//  aHotPlateSubstrate.h -- TMyKitSuck + InArmSuck / OutArmSuck / OutArm2Suck
//                          + SetInArmHome.
//
//  TRAP-5 CHECK ON aHotPlateSubstrate.h -- THIS TREE HAS **TWO** TMyKitSuck.
//    (i) aHotPlateSubstrate.h:365 and (ii) mykitsuck.h:274.  They have DIFFERENT
//    LAYOUTS, and mykitsuck.cpp:205-207 defines a SECOND InArmSuck / FLCarryKit /
//    FRCarryKit.  Picking the wrong one links perfectly cleanly and then reads
//    Suck[i][j].iMotNo at the wrong offset -- i.e. these functions would stop a
//    random motor.  WHICH ONE IS ACTUALLY LINKED, measured (not assumed):
//      nm --defined-only -C build_0811_w7h_rel/libht9045_sm.a
//        -> aHotPlateSubstrate.cpp.obj: 00031b80 B InArmSuck
//        -> aHotPlateSubstrate.cpp.obj: 00029060 B OutArmSuck
//        -> aHotPlateSubstrate.cpp.obj: 00026200 B OutArm2Suck
//      and mykitsuck.cpp is in NO archive (it has no entry in CMakeLists.txt).
//      So aHotPlateSubstrate.h is the header whose layout matches the objects we
//      link against.  Both measurements 2026-08-11.
//    aHotPlateSubstrate.h:132 is the `int iMotNo;` this block reads (golden
//    MyKitSuck.h:138); aHotPlateSubstrate.h:419-420 are iMotRow / iMotCol
//    (golden MyKitSuck.h:156-157).
// ---------------------------------------------------------------------------
#include "common.h"                 // MySleepEx  (golden common.h:260)
#include "canary_support.h"         // ShowMyMessage (golden mymessbox.h:58) + __FUNC__
#include "aHotPlateSubstrate.h"     // TMyKitSuck InArmSuck/OutArmSuck/OutArm2Suck + SetInArmHome
#include "mycylin.h"                 // AI(W906-P0-6) 20260921: Cylinder[] / TMyCylinder -- TrayArmMotorMove 的 floodgate 狀態機（golden :5793 起）
#include "mymessbox_shim.h"         // AI(W906-P0-7) 20260921: MyMessageBox->Visible/Close() -- OutArmContinuousMove_9045 的三個 Discrepancy 分支（golden :4346/:4365/:4382）
#include "forms/fMotorTest.h"       // AI(W906-P0-7) 20260921: fMotorTest->mmo5..mmo8（TMemo*） -- light-scale log（golden :4155/:4167/:4180/:4192）
// AI(W906-P0-7) 20260921: 只補**這一個**宣告，簽名逐字抄 aoutarm.h:115。
//   不 include aoutarm.h：那會把 out-arm 的整個介面拉進本檔，
//   風險遠大於這件事本身（同 IsNNMode 在 ainarm9045.cpp 的處置）。
bool MoveOutArmToAutoSafe();                                                    // golden aoutarm.h:52 / 本樹 aoutarm.h:115（本體 aoutarm.cpp:858）
bool W906_FormShowing(const char* goldenForm, bool member);   //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：頁面表的單一函式（與 W906FormShowing.h／csystem.h:440 同一個宣告，本體 csystem.cpp:30049）；本檔不 include csystem.h，放在原本的空行，其後行號不動
// AI(pt-wave) 20260811 PT-W7n: golden Motor/mymotor.cpp:64-65 sit here, just above
//   CheckTestZ.  :64 is `extern void SetInArmHome(bool bPrecisorNeedHome=false);`
//   -- NOT re-emitted: aHotPlateSubstrate.h:915 already carries that exact
//   declaration WITH the default argument, and C++ forbids repeating a default
//   argument for the same parameter in one translation unit (hard error, not a
//   silent shadow).  Same signature, same default, same meaning; the only thing
//   lost is a duplicate line.  :65 has no default argument, so it is transcribed
//   verbatim.  golden :63 `extern HAlarm *Alarm;` is NOT in this wave's scope and
//   HAlarm has no port (see canary_support.h:220-254).
extern void SetOutArmHome();
//==============================================================================
// golden Motor/mymotor.cpp:67-115  (CheckTestZ)
bool CheckTestZ(AnsiString sFunc)                                               //Steven 20141007 : 換位置
{
    AnsiString Str;
    static int iRetryCT=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    long lPos[4]={0, 0, 0, 0};                                                  //kevin 20150915

    lPos[0]=MOT[MTestZ1].Gali_ReadPos();
    if(MOT[MTestZ1].Gali_ReadEncoderInRandgeMinLimit(lPos[0]))
        flag1=true;

    lPos[1]=MOT[MTestZ2].Gali_ReadPos();
    if(MOT[MTestZ2].Gali_ReadEncoderInRandgeMinLimit(lPos[1]))
        flag2=true;

    lPos[2]=MOT[MTestY1].Gali_ReadPos();
    if(MOT[MTestY1].Gali_ReadEncoderInRandgeMinLimit(lPos[2]))
        flag3=true;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        lPos[3]=0;
        flag4=true;
    }
    else
    {
        lPos[3]=MOT[MTestY2].Gali_ReadPos();
        if(MOT[MTestY2].Gali_ReadEncoderInRandgeMinLimit(lPos[3]))
            flag4=true;
    }

    if(flag1 && flag2 && flag3 && flag4)
    {
        iRetryCT=0;
        return true;
    }
    else
    {
        iRetryCT++;
        if(iRetryCT>150)                                                        //kevin 20130312100
        {
            Str.sprintf("CheckTestZ(%s)", sFunc);
            ShowIndexMotorError(Str);
            iRetryCT=0;                                                         //kevin 20110628 發生alarm 需清為0否則要關程式
            RecordIndexPositionError(Str, flag1, flag2, flag3, flag4, &lPos[0]); //kevin 20150915 record
        }
        return false;
    }
}
//==============================================================================
// golden Motor/mymotor.cpp:117-171  (CheckTestZ1)
bool CheckTestZ1(AnsiString sFunc)                                              //Steven 20141007 : 換位置
{
    static int iRetryCT=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    long lPos[4]={0, 0, 0, 0};                                                  //kevin 20150915
    AnsiString S;

    lPos[0]=MOT[MTestZ1].Gali_ReadPos();
    if(MOT[MTestZ1].Gali_ReadEncoderInRandgeMinLimit(lPos[0]))
        flag1=true;

    flag2=true;
    lPos[1]=0;

    lPos[2]=MOT[MTestY1].Gali_ReadPos();
    if(MOT[MTestY1].Gali_ReadEncoderInRandgeMinLimit(lPos[2]))
        flag3=true;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        lPos[3]=0;
        flag4=true;
    }
    else
    {
        lPos[3]=MOT[MTestY2].Gali_ReadPos();
        if(MOT[MTestY2].Gali_ReadEncoderInRandgeMinLimit(lPos[3]))
            flag4=true;
    }

    if(flag1 && flag2 && flag3 && flag4)
    {
#ifdef DEBUG
        long Pos=MOT[MTestZ1].Gali_ReadPos();
        long Pos1=MOT[MTestZ1].Gali_ReadEncoderPos();
        S.sprintf("%d, %d, %d, %d", Pos, Pos1, Pos-Pos1, iRetryCT);
        fMain->lbEnCoder0->Caption=S;
#endif
        iRetryCT=0;
        return true;
    }
    else
    {
        iRetryCT++;
        MySleepEx(5, true);
        if(iRetryCT>100)
        {
            S.sprintf("CheckTestZ1(%s)", sFunc);
            ShowIndexMotorError(S);
            iRetryCT=0;
            RecordIndexPositionError(S, flag1, flag2, flag3, flag4, &lPos[0]); //kevin 20150915 record
        }
        return false;
    }
}
//==============================================================================
// golden Motor/mymotor.cpp:173-226  (CheckTestZ2)
bool CheckTestZ2(AnsiString sFunc)                                              //Steven 20141007 : 換位置
{
    static int iRetryCT=0;
    bool flag1=false, flag2=false, flag3=false, flag4=false;
    long lPos[4]={0, 0, 0, 0};                                             //kevin 20150915
    AnsiString S;

    flag1=true;

    lPos[1]=MOT[MTestZ2].Gali_ReadPos();
    if(MOT[MTestZ2].Gali_ReadEncoderInRandgeMinLimit(lPos[1]))
        flag2=true;

    lPos[2]=MOT[MTestY1].Gali_ReadPos();
    if(MOT[MTestY1].Gali_ReadEncoderInRandgeMinLimit(lPos[2]))
        flag3=true;

    if(USE_INDEX_ARM_AXES==IndexArm_3_Axis)                                     //JimmyChiu 20220708 : add Index Arm Axis
    {
        lPos[3]=0;
        flag4=true;
    }
    else
    {
        lPos[3]=MOT[MTestY2].Gali_ReadPos();
        if(MOT[MTestY2].Gali_ReadEncoderInRandgeMinLimit(lPos[3]))
            flag4=true;
    }

    if(flag1 && flag2 && flag3 && flag4)
    {
#ifdef DEGBU
        long Pos=MOT[MTestZ2].Gali_ReadPos();
        long Pos1=MOT[MTestZ2].Gali_ReadEncoderPos();
        S.sprintf("%d, %d, %d, %d", Pos, Pos1, Pos-Pos1, iRetryCT);
        fMain->lbEnCoder1->Caption=S;
#endif
        iRetryCT=0;
        return true;
    }
    else
    {
        iRetryCT++;
        MySleepEx(5, true);
        if(iRetryCT>100)
        {
            S.sprintf("CheckTestZ2(%s)", sFunc);
            ShowIndexMotorError(S);
            iRetryCT=0;
            RecordIndexPositionError(S, flag1, flag2, flag3, flag4, &lPos[0]);  //kevin 20150915 record
        }
        return false;
    }
}
//---------------------------------------------------------------------------
// golden Motor/mymotor.cpp:228-259  (IndexPosMonitor)
//  GOLDEN DEFECTS PRESERVED, NOT FIXED (all three reported):
//   (1) `bool Z1, Z2, Y1, Y2;` (:231) are assigned from Gali_ReadEncoderPos(),
//       which returns `long` -- so every non-zero encoder position collapses to
//       true==1 and zero collapses to false==0 BEFORE the `Z1<Prod.All_TestZ_
//       Test_Safe` comparison at :239/:251 and before the %d in the sprintf.
//       The whole predicate is therefore `0or1 < All_TestZ_Test_Safe` in golden
//       too.  Kept exactly: the types stay `bool`.
//   (2) Y2 (:231) is read at :253 on the USE_INDEX_ARM_AXES!=IndexArm_4_Axis
//       path without ever being written (:248-249 only writes it on the 4-axis
//       path) -- an indeterminate read, in golden.  Kept exactly.
//   (3) bRet (:230) is never assigned, so :258 always returns false.  Kept.
bool IndexPosMonitor(int IsZ1Down)
{
    bool bRet=false;
    bool Z1, Z2, Y1, Y2;
    AnsiString str;

    if(IsZ1Down==1)                                                             //Index Arm 1
    {
        Z1=MOT[MTestZ1].Gali_ReadEncoderPos();
        Y1=MOT[MTestY1].Gali_ReadEncoderPos();

        if(Z1<Prod.All_TestZ_Test_Safe)
        {
            str.sprintf("Z1,%d,Y1,%d", Z1, Y1);
#if 0 // GATE(PT-W7n-1a): golden Motor/mymotor.cpp:242 -- VERBATIM golden text below; see GATE REGISTER
            fMain->tMonitorIndex->Add(str);
#endif // GATE(PT-W7n-1a)
        }
    }
    else if(IsZ1Down==2)                                                        //Index Arm 2
    {
        Z2=MOT[MTestZ2].Gali_ReadEncoderPos();
        if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)
            Y2=MOT[MTestY2].Gali_ReadEncoderPos();

        if(Z2<Prod.All_TestZ_Test_Safe)
        {
            str.sprintf("Z2,%d,Y2,%d", Z2, Y2);
#if 0 // GATE(PT-W7n-1b): golden Motor/mymotor.cpp:254 -- VERBATIM golden text below; see GATE REGISTER
            fMain->tMonitorIndex->Add(str);
#endif // GATE(PT-W7n-1b)
        }
    }

    return bRet;
}
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2392  (the declaration golden puts immediately above
//   CheckInArmZHomeSensor_2x8; transcribed verbatim -- no default argument, so it
//   cannot clash with asortarm.h:111's identical declaration.)
extern void SetSortArmHome();                                                   //RogerYang 20250510 Add for 9046AU
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2394-2422  (CheckInArmZHomeSensor_2x8)
bool CheckInArmZHomeSensor_2x8(bool ZNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col])   //Steven for HT1032
{
    int iMotNo;
    int iPos;
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMotNo=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            MOT[iMotNo].fCMD=false;
            if(ZNeedDown && ZDownSel[i][j])
            {
                MOT[iMotNo].ScanMotorStatus();
                iPos=MOT[iMotNo].ReadEncoderPos();
                if(MOT[iMotNo].Motor->Enable && MOT[iMotNo].Led[iHomeLed] && iPos<0)
                {
                    MOT[MInArmX].PCIL132_StopMotor();
                    MOT[MInArmY].PCIL132_StopMotor();
                    ShowMyMessage(MOT[iMotNo].NumberAlias+" Home sensor error, if suck is down, maybe sensor fail!",
                                  MOT[iMotNo].NumberAlias+"歸零sensor錯誤；如果吸嘴在下方，可能是sensor壞掉", __FUNC__);
                    SetInArmHome();
                    return false;
                }
            }
        }
    }

    return true;
}
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2424-2451  (CheckSortArmZHomeSensor_2x8)
bool CheckSortArmZHomeSensor_2x8(bool ZNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col])  //RogerYang 20250510 Add for 9046AU
{
    int iMotNo;
    int iPos;
    for(int i=0; i<OutArm2Suck.iMotRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iMotCol; j++)
        {
            iMotNo=OutArm2Suck.Suck[i][j].iMotNo;
            MOT[iMotNo].fCMD=false;
            if(ZNeedDown && ZDownSel[i][j])
            {
                MOT[iMotNo].ScanMotorStatus();
                iPos=MOT[iMotNo].ReadEncoderPos();
                if(MOT[iMotNo].Motor->Enable && MOT[iMotNo].Led[iHomeLed] && iPos<0)
                {
                    MOT[MOutSortX].PCIL132_StopMotor();
                    MOT[MOutSortY].PCIL132_StopMotor();
                    ShowMyMessage(MOT[iMotNo].NumberAlias+" Home sensor error, if suck is down, maybe sensor fail!",
                                  MOT[iMotNo].NumberAlias+"歸零sensor錯誤；如果吸嘴在下方，可能是sensor壞掉", __FUNC__);
                    SetSortArmHome();
                    return false;
                }
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2453-2483  (CheckOutArmZHomeSensor_2x8)
bool CheckOutArmZHomeSensor_2x8(bool ZNeedDown, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col])  //Steven for HT1032
{
    int iMotNo;
    int iPos;
    for(int i=0; i<OutArmSuck.iMotRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMotCol; j++)
        {
            if(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)
                iMotNo=MOutArmZA;
            else
                iMotNo=OutArmSuck.Suck[i][j].iMotNo;
            MOT[iMotNo].fCMD=false;
            if(ZNeedDown && ZDownSel[i][j])
            {
                MOT[iMotNo].ScanMotorStatus();
                iPos=MOT[iMotNo].ReadEncoderPos();
                if(MOT[iMotNo].Motor->Enable && MOT[iMotNo].Led[iHomeLed] && iPos<0)
                {
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].PCIL132_StopMotor();
                    ShowMyMessage(MOT[iMotNo].NumberAlias+" Home sensor error, if suck is down, maybe sensor fail!",
                                  MOT[iMotNo].NumberAlias+"歸零sensor錯誤；如果吸嘴在下方，可能是sensor壞掉", __FUNC__);
                    SetOutArmHome();
                    return false;
                }
            }
        }
    }
    return true;
}
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2485-2554  (ChangePosition)
//  GOLDEN DEFECTS PRESERVED, NOT FIXED (both reported):
//   (1) `return dPos6;` at golden :2518 and :2549 returns a DOUBLE from an INT
//       function -> implicit truncation toward zero.  Kept exactly.
//   (2) `iMagneticScalePos[iMove][j+1]` with j running to 999 reads index 1000
//       of a [16][1000] array (cmydef.h:4576) on the last iteration -- a
//       one-element OUT-OF-BOUNDS read, in golden.  Kept exactly.
//   Division note: :2505 and :2536 divide a double by dPos1 (a double), so these
//   are FLOATING divisions in golden and stay floating here.  There is no int/int
//   anywhere in this function that could be wrongly 'improved'.
int ChangePosition(int Pos, int iMoveTable)                                     //Frank 20180102 add
{
    double dPos1=0.0, dPos2=0.0, dPos3=0.0, dPos4=0.0, dPos5=0.0, dPos6=0.0;
    int j=0;
    int iMove=iMoveTable*2;

    if(iMoveTable%2==0)
    {                  //往負
        for(j=0; j<1000; j++)
        {
            if(Pos>=iMagneticScalePos[iMove][j+1] && Pos<=iMagneticScalePos[iMove][j])
            {
                if(iMagneticScalePos[iMove][j+1]-iMagneticScalePos[iMove][j]!=0)
                {
                    dPos1=iMagneticScalePos[iMove+1][j+1]-iMagneticScalePos[iMove+1][j];
                    dPos2=iMagneticScalePos[iMove][j+1]-iMagneticScalePos[iMove][j];
                    dPos3=(Pos);

                    if(dPos1!=0)
                    {
                        dPos4=(dPos3-(double)iMagneticScalePos[iMove+1][j])/dPos1;
                        dPos5=dPos2*dPos4;
                        dPos6=dPos5+iMagneticScalePos[iMove][j];
                    }
                    else
                    {
                        dPos6=0.0;
                    }
                }
                else
                {
                    dPos6=0.0;
                }
                return dPos6;
            }
        }
    }
    else
    {                 //往正
        for(j=0; j<1000; j++)
        {
            if(iMagneticScalePos[iMove][j+1]>=Pos && Pos>=iMagneticScalePos[iMove][j])
            {
                if(iMagneticScalePos[iMove][j]-iMagneticScalePos[iMove][j+1]!=0)
                {
                    dPos1=iMagneticScalePos[iMove+1][j+1]-iMagneticScalePos[iMove+1][j];
                    dPos2=iMagneticScalePos[iMove][j+1]-iMagneticScalePos[iMove][j];
                    dPos3=(Pos);

                    if(dPos1!=0)
                    {
                        dPos4=(dPos3-iMagneticScalePos[iMove+1][j])/dPos1;
                        dPos5=dPos2*dPos4;
                        dPos6=dPos5+iMagneticScalePos[iMove][j];
                    }
                    else
                    {
                        dPos6=0.0;
                    }
                }
                else
                {
                    dPos6=0.0;
                }
                return dPos6;
            }
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2556-2557  (iIDLETime, InArmIdle)
const int iIDLETime=30;
TQPF_Timer InArmIdle;
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2720  (bInArmZMove).  golden's :2719 iInArmZMoveTask
//   is NOT re-emitted: this port already defines it at Motor/mymotor.cpp:105.
//   Divergence recorded for the main loop: the port initialises it to 1, golden
//   :2719 initialises it to -1.  Pre-existing, NOT touched by this wave.
bool bInArmZMove[MAX_ARM_Row][MAX_ARM_Col];
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:2884  (InArmCylinderDelayTimer; HTimer -> TQPF_Timer)
TQPF_Timer InArmCylinderDelayTimer;                                             //AI(pt-wave) 20260811 PT-W7n: golden `HTimer InArmCylinderDelayTimer;` (:2884)
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:3456  (OutArmIdle)
TQPF_Timer OutArmIdle;
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:3937  (bOutArmZMove).  golden's :3936 iOutArmZMoveTask
//   is NOT re-emitted: this port already defines it at Motor/mymotor.cpp:106
//   (same 1 vs -1 divergence noted above; pre-existing, NOT touched here).
bool bOutArmZMove[MAX_ARM_Row][MAX_ARM_Col];
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:4092  (OutArmCylinderDelayTimer; HTimer -> TQPF_Timer)
TQPF_Timer OutArmCylinderDelayTimer;                                            //AI(pt-wave) 20260811 PT-W7n: golden `HTimer OutArmCylinderDelayTimer;` (:4092)
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:5653-5654  (iTrayXTask, iTrayOldPos)
int iTrayXTask;
int iTrayOldPos;
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:5907-5909  (SortArmIdle, iSortArmZMoveTask, bSortArmZMove)
TQPF_Timer SortArmIdle;                                                         //RogerYang 20250510 Add for 9046AU
int  iSortArmZMoveTask=-1;
bool bSortArmZMove[MAX_ARM_Row][MAX_ARM_Col];
//------------------------------------------------------------------------------
// golden Motor/mymotor.cpp:6069  (SortArmCylinderDelayTimer; HTimer -> TQPF_Timer)
TQPF_Timer SortArmCylinderDelayTimer;                                           //AI(pt-wave) 20260811 PT-W7n: golden `HTimer SortArmCylinderDelayTimer;`  //RogerYang 20250512 Add for 9046AU (:6069)
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  TrayArmMotorMove -- golden Motor/mymotor.cpp:5662-5880（219 行）
//  AI(W906-P0-6) 20260921
//
//  ## 為什麼是它
//
//  歸零 `case 1310`（uhome.cpp）要 flag1..flag10 + flag14[0..3] + flag18[0..2]
//  全 true 才走到 1520，而 `flag2 = TrayArmMotorMove(Prod.iXTrayEmpty)`
//  在本樹連到的是 `Motor/mymotor.cpp:109` 的一行式樁 `{ return false; }`。
//  ⇒ 歸零永遠停在 1310、逾時、從第 1 步重來。這是 Q18 退掉 Galil 樁之後
//    浮上來的**下一個**阻擋（退樁前是卡在 700）。
//
//  ## 兩種組態差很多，看清楚再改
//
//      #ifdef SOFT_SIMULTE      -> 整個本體就是 `MOT[MTrayX].MotorMove(p)` 一行
//      #else                    -> 四個 floodgate 氣缸的 Task 狀態機 1->100->200->300
//
//  ⚠ 但**前面那段 teach 保護在兩種組態都會跑**（它在 #ifdef 外面），
//    而且 `bCheckPos` 預設是 true（mymotor.h:466），所以 uhome 那個單參數
//    呼叫會走到它。它會在 teach 點不合理時 `ShowMyMessage` 然後回 false。
//    ⇒ 如果翻完之後 1310 仍然卡住，**先看 `_spine_poll_raw.log` 有沒有
//      "TrayArm moves %d to the left error"** —— 那是教導資料的問題，
//      不是翻譯的問題，兩者處置完全不同。
//
//  ## 相依
//
//  * `Cylinder[]` / `C_TrayXFloodgate1..4` -> 本檔新增 `#include "mycylin.h"`
//    （`C_TrayXFloodgate*` 的常數本來就在 cmydef.h:508，缺的是 `Cylinder[]` 本身）
//  * `iTrayXTask` / `iTrayOldPos` -> **本檔早就有**（檔尾，標著 golden :5653-5654）
//  * `Prod` / `Tech` / `MOT` / `MachineTypeChoice` / `SubMachineType` /
//    `TRAY_ARM_MODE` / `eAboveCoveyor` / `USE_LdUldCassetteMode` /
//    `USE_OUT_SORT_ARM` / `eartUninstall` / `Type_HT90*` -> 都已存在
//  ⇒ **零個「相依不存在」，所以一個閘都沒加。**
//------------------------------------------------------------------------------
bool TrayArmMotorMove(int p, bool bCheckPos)                                    //bOnOff = true Door Open , bOnOff = false Door Off
{
    bool bResult=false;
    //Sam 20241206 : Tray Arm 新增 Teach 保護
    //==>
    AnsiString sMsg="";
    int iSafePos=0;
    int iTrayArmTarget=-1;
    if(p==Prod.iXTrayLoad ||
       p==Prod.iXTrayLoad_ART)
    {
        iTrayArmTarget=eTrayArmLoader;
    }
    else if(p==Prod.iXTrayEmpty)
    {
        iTrayArmTarget=eTrayArmEmpty;
    }
    else if(p==Prod.iXTrayColor)
    {
        iTrayArmTarget=eTrayArmColor;
    }
    else if(p==Prod.iXTrayAuto[0] ||
            p==Prod.iXTrayAuto[1] ||
            p==Prod.iXTrayAuto[2] ||
            p==Prod.iXTrayAuto[3] ||
            p==Prod.iXTrayAuto[4] ||
            p==Prod.iXTrayAuto[5] ||
            p==Prod.iXTrayAuto_ART[0] ||
            p==Prod.iXTrayAuto_ART[1] ||
            p==Prod.iXTrayAuto_ART[2] ||
            p==Prod.iXTrayAuto_ART[3] ||
            p==Prod.iXTrayAuto_ART[4] ||
            p==Prod.iXTrayAuto_ART[5])
    {
        iTrayArmTarget=eTrayArmAuto;
    }

    if(USE_LdUldCassetteMode==1)                                                //KenHsieh 20251220 : Cassette mode safe pos is Empty real pos +1000
    {
        iSafePos=Tech.iTrayXEmpty+6800+1000;
    }
    else if(MachineTypeChoice==Type_HT9045)
    {
        iSafePos=45742;                                                         //此用 HT9045機台 Staterecord empty & color 的 Teach 點取中心。
    }
    else if((MachineTypeChoice==Type_HT9046)) //AI(W906-HT9050-FAM) 20260925: 0926 拿掉 +Type_HT9050（AI(W906-HT9050-AS-LS) 20260926：RULINGS_20260926 第 25 條，HT9050 解碼成 HT9046_LS，這一處照 LS 走）
    {
        iSafePos=49750;                                                         //此用 HT9046機台 Staterecord empty & color 的 Teach 點取中心。
    }
    else if(MachineTypeChoice==Type_HT9045_12Site)
    {
    }
//    else if(MachineTypeChoice==Type_HT1032)
//    {
//
//    }
//    else if(MachineTypeChoice==Type_HT7080)
//    {
//
//    }
    else if(MachineTypeChoice==Type_HT9046_LS ||
            MachineTypeChoice==Type_HT9050)                                     //AI(W906-HT9050-TYPE) 20261004: 9050GPIB decodes to Type_HT9050 again (database.cpp); it kept this LS arm's Tray Arm station guard while it was decoded as Type_HT9046_LS -- without it iSafePos=0 switches the whole check off in auto-run (HT9050 = HT9046 family, RULINGS_20260925 #5)
    {
        if(SubMachineType==Type_HT9046AU ||                                     //Steven 20240822 : For HT-9046AU
                USE_OUT_SORT_ARM!=eartUninstall)                                //RogerYang 20250514 Add for 9046AU
        {
            iSafePos=Tech.iTrayXEmpty+6500;
        }
        else
        {
            iSafePos=(Tech.iTrayXEmpty+Tech.iTrayXColor)/2+6500;                //Steven 20250122 : 改用Teaching位子判斷
        }
//        iSafePos=63800;                                                         //此用 HT9046LS機台 Staterecord empty & color 的 Teach 點取中心。
        /*
        if(SubMachineType==Type_HT9046LA)

        else if(SubMachineType==Type_HT9016C)

        else if(SubMachineType==Type_HT9046AU ||                                //Steven 20240822 : For HT-9046AU
                USE_OUT_SORT_ARM!=eartUninstall)

        else if(SubMachineType==Type_HT9046CR)                                  //Steven 20241001 : For HT-9046CR

        else
        */
    }

    if(iSafePos>0 && bCheckPos)
    {
        if(USE_LdUldCassetteMode==1 &&
           Tech.iTrayXEmpty!=Tech.iTrayXColor)                                  //KenHsieh 20251220 : Cassette mode empty & color pos need same
        {
            sMsg.sprintf("TrayArm empty & color pos need same.");
            ShowMyMessage(sMsg);
            return bResult;
        }
        else if(iTrayArmTarget==eTrayArmLoader ||
           iTrayArmTarget==eTrayArmEmpty)
        {
            if(p>iSafePos)
            {
                sMsg.sprintf("TrayArm moves %d to the left error. Please check loader and empty teach position.", p);
                ShowMyMessage(sMsg);
                return bResult;
            }
        }
        else if(iTrayArmTarget==eTrayArmColor ||
                iTrayArmTarget==eTrayArmAuto)
        {
            if(p<iSafePos)
            {
                 sMsg.sprintf("TrayArm moves %d to the right error. Please check color and auto teach position.", p);
                 ShowMyMessage(sMsg);
                 return bResult;
            }
        }
        else
        {
            sMsg.sprintf("TrayArm moves %d unknown error. Please check TrayArm teach position.", p);
            ShowMyMessage(sMsg);
            return bResult;
        }
    }
    //<==
    //Sam 20241206 : Tray Arm 新增 Teach 保護

    #ifdef SOFT_SIMULTE
        bResult=MOT[MTrayX].MotorMove(p);
    #else
        static bool bCyflag[4]={false, false, false, false};
        int &Task=iTrayXTask;
        if(TRAY_ARM_MODE==eAboveCoveyor)
        {
            bResult=MOT[MTrayX].MotorMove(p);
        }
        else if(USE_LdUldCassetteMode==1)                                       //RogerYang 20260203 : Add for HT9046CR 移動保護
        {
            if(p==Prod.iXTrayColor ||
                p==Prod.iXTrayEmpty)
            {
                bResult=MOT[MTrayX].MotorMove(p);
            }
            else
            {
                bResult=true;
            }
        }
        else
        {
            switch(Task)
            {
                case 1:
                    for(int i=0; i<4; i++)
                        bCyflag[i]=false;

                    if(iTrayOldPos!=p)
                    {
                        Task=100;
                    }
                    else
                    {
                        bResult=true;
                    }
                    break;
                case 100:
                    if(bCyflag[0]==false)
                        bCyflag[0]=(Cylinder[C_TrayXFloodgate1].Enable==false || Cylinder[C_TrayXFloodgate1].Push());
                    if(bCyflag[1]==false)
                        bCyflag[1]=(Cylinder[C_TrayXFloodgate2].Enable==false || Cylinder[C_TrayXFloodgate2].Push());
                    if(bCyflag[2]==false)
                        bCyflag[2]=(Cylinder[C_TrayXFloodgate3].Enable==false || Cylinder[C_TrayXFloodgate3].Push());
                    if(bCyflag[3]==false)
                        bCyflag[3]=(Cylinder[C_TrayXFloodgate4].Enable==false || Cylinder[C_TrayXFloodgate4].Push());

                    if(bCyflag[0] && bCyflag[1] && bCyflag[2] && bCyflag[3])
                    {
                        Task=200;
                    }
                    break;
                case 200:
                    if((Cylinder[C_TrayXFloodgate1].Enable==true && Cylinder[C_TrayXFloodgate1].OffSensor()==true) ||
                       (Cylinder[C_TrayXFloodgate2].Enable==true && Cylinder[C_TrayXFloodgate2].OffSensor()==true) ||
                       (Cylinder[C_TrayXFloodgate3].Enable==true && Cylinder[C_TrayXFloodgate3].OffSensor()==true) ||
                       (Cylinder[C_TrayXFloodgate4].Enable==true && Cylinder[C_TrayXFloodgate4].OffSensor()==true))
                    {
                        for(int i=0; i<4; i++)
                            bCyflag[i]=false;
                        Task=100;
                        break;
                    }

                    if(MOT[MTrayX].MotorMove(p))
                    {
                        for(int i=0; i<4; i++)
                            bCyflag[i]=false;
                        Task=300;
                    }
                    break;
                case 300:
                    if(bCyflag[0]==false)
                        bCyflag[0]=(Cylinder[C_TrayXFloodgate1].Enable==false || Cylinder[C_TrayXFloodgate1].Pop());
                    if(bCyflag[1]==false)
                        bCyflag[1]=(Cylinder[C_TrayXFloodgate2].Enable==false || Cylinder[C_TrayXFloodgate2].Pop());
                    if(bCyflag[2]==false)
                        bCyflag[2]=(Cylinder[C_TrayXFloodgate3].Enable==false || Cylinder[C_TrayXFloodgate3].Pop());
                    if(bCyflag[3]==false)
                        bCyflag[3]=(Cylinder[C_TrayXFloodgate4].Enable==false || Cylinder[C_TrayXFloodgate4].Pop());

                    if(bCyflag[0] && bCyflag[1] && bCyflag[2] && bCyflag[3])
                    {
                        bResult=true;
                        iTrayOldPos=p;
                        Task=1;
                    }
                    break;
            }
        }
    #endif
    return bResult;
}

//==============================================================================
//  OutArm 連續移動家族 -- golden Motor/mymotor.cpp（五支，共 731 行）
//  AI(W906-P0-7) 20260921
//
//  ## 為什麼
//
//  歸零 `uhome.cpp` case 1540 要 flag1..flag6 全 true。逐個排除之後只剩 flag2：
//      flag2 = MoveOutArmXY_ToFix_Tray_Full()      aoutarm.cpp:869
//            -> 尾巴是 `if(OutArmContinuousMove_9045(...)) return true; return false;`
//            -> 而本樹連到的是 `Motor/mymotor.cpp:2322` 的一行式樁 `{ return false; }`
//  ⇒ 20260921 實測：`…→1520→1530→1540(x381)` 卡死（240 秒探針，末端連續 381 次）。
//
//  排除另外五個 flag 的依據（不是猜的）：
//    flag4 `AUTO_SENSOR_INSTALL`   預設 0             cmydef.cpp:3223 -> else 補 true
//    flag6 `USE_OUT_SORT_ARM`      預設 eartUninstall cmydef.cpp:3138 -> else 補 true
//    flag5 已被 GATE (W906-HOME-C2-MAGAZINE) 補 true
//    flag1/flag3 是 `MotorMove`，它在 `Motor==NULL || !Motor->Enable` 時回 1（成功）
//
//  ## 相依：零個缺
//
//  用**編譯器**判（`Motor/mymotor.cpp` 全文 ＋ golden 五支 body 拼成一個 TU，
//  兩組態各跑一次 `-fsyntax-only`）。未解析的只有三個，而且三個都**存在**，
//  只是本檔沒 include：
//      MoveOutArmToAutoSafe  -> aoutarm.h:115（本體 aoutarm.cpp:858）
//      MyMessageBox          -> mymessbox_shim.h:28
//      fMotorTest            -> forms/fMotorTest.h:983（mmo5..mmo8 是 TMemo*，:803-806）
//  ⇒ **一個閘都沒加。**
//
//  ⚠ 不要用 `git grep` 判這件事。第一版我用 grep，18 個嫌疑符號全回報「有」；
//    而 20260920 的 `CheckSTMMode` 全樹命中都在註解行裡（golden 要的是成員函式）。
//  ⚠ 探針的**取樣點**也會騙人：第一版只取 `#include` 行當前綴，於是
//    `bPauseOutMotor`(:96) / `iTorence`(:113) / `bOutArmZMove` /
//    `OutArmCylinderDelayTimer`（都在本檔）被誤報成缺相依 —— 14 個裡 11 個是假的。
//    正確做法是拿**真檔全文**當前綴。
//==============================================================================
//----- golden Motor/mymotor.cpp:3327-3454  OutArmPitchMove -----
bool OutArmPitchMove(int Vari[X_PITCH_COUNT], int YVari, bool bInit)            //Steven for HT1032
{
    static bool bMoveClose=false;
    static bool bFirstIn=true;
    static int  iFlag[PITCH_COUNT];
    bool bFlag;
    int iResult=0;
    int iPos=0;
    #ifndef SOFT_SIMULTE
    long iCurrPos[4]={MOT[MOutArmPitch  ].Motor->ReadPos(),
                      MOT[MOutArmPitchX2].Motor->ReadPos(),
                      MOT[MOutArmPitchX3].Motor->ReadPos(),
                      MOT[MOutArmPitchX4].Motor->ReadPos()};

    int iDevPos[4];
    BYTE bDevNo[4]={MOT[MOutArmPitch  ].Motor->iPortID,
                    MOT[MOutArmPitchX2].Motor->iPortID,
                    MOT[MOutArmPitchX3].Motor->iPortID,
                    MOT[MOutArmPitchX4].Motor->iPortID};
    #endif

    if(USE_PICKER_COUNT==ep1Picker)
        return 1;

    if(bInit || bFirstIn)
    {
        ZeroMemory(iFlag, sizeof(iFlag));
        if(bFirstIn)
        {
            iPos=MOT[MOutArmPitch].ReadPos();
            bMoveClose=(Vari[0]<iPos);                                          //目前位置大於目標位置    //內縮
            if(USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //Steven for HT1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
               USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.06 Add HT-1032
            {
                #ifdef SOFT_SIMULTE                                             //Ztex Add SOFT_SIMULTE Pass
                    iFlag[1]=MOT[MOutArmPitch  ].MotorMove(Vari[0]);
                    iFlag[2]=MOT[MOutArmPitchX2].MotorMove(Vari[1]);
                    iFlag[3]=MOT[MOutArmPitchX3].MotorMove(Vari[2]);
                    iFlag[4]=MOT[MOutArmPitchX4].MotorMove(Vari[3]);
                #else
                    iDevPos[0]=Vari[0]-iCurrPos[0];
                    iDevPos[1]=Vari[1]-iCurrPos[1];
                    iDevPos[2]=Vari[2]-iCurrPos[2];
                    iDevPos[3]=Vari[3]-iCurrPos[3];
                    MOT[MOutArmPitch  ].GetRealPos(&iDevPos[0]);
                    MOT[MOutArmPitchX2].GetRealPos(&iDevPos[1]);
                    MOT[MOutArmPitchX3].GetRealPos(&iDevPos[2]);
                    MOT[MOutArmPitchX4].GetRealPos(&iDevPos[3]);
                    long lDevPos[4]={iDevPos[0], iDevPos[1], iDevPos[2], iDevPos[3]};
                    MOT[MOutArmPitch].Motor->SetGroup(2, 4, bDevNo);
                    MOT[MOutArmPitch].Motor->LineNMove(bDevNo, lDevPos, 4);
                #endif
            }
        }
        bFirstIn=false;
        return 0;
    }

    if(USE_OUT_ARM_Y_PITCH==iXYPitchVariable ||                              //JerryYang 20251218 : IN/OUT ARM支援不同模組
       USE_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        if(iFlag[0]==0)
            iFlag[0]=MOT[MOutArmPitchY ].MotorMove(YVari);                      //ChungHung 20131231 alter AutoYPitch
        if(iFlag[1]==0)
            iFlag[1]=MOT[MOutArmPitch  ].MotorMove(Vari[0]);
        if(iFlag[2]==0)
            iFlag[2]=MOT[MOutArmPitchX2].MotorMove(Vari[1]);                    //Steven 20131002 : XY變距
        iFlag[3]=1;
        iFlag[4]=1;
    }
    else if(USE_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                         //Steven for HT1032  //JerryYang 20251218 : IN/OUT ARM支援不同模組
            USE_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        if(iFlag[4]==0)
            iFlag[4]=MOT[MOutArmPitchY].MotorMove(YVari);

        if(iFlag[0]==0)
        {
            #ifndef SOFT_SIMULTE
            bFlag=MOT[MOutArmPitch].Motor->MotionDone();
            #else
            bFlag=true;
            #endif
            if(bFlag)
                iFlag[0]=1;
        }
        iFlag[1]=1;
        iFlag[2]=1;
        iFlag[3]=1;
    }
    else
    {
        if(iFlag[0]==0)
            iFlag[0]=MOT[MOutArmPitch].MotorMove(Vari[0]);
        iFlag[1]=1;
        iFlag[2]=1;
        iFlag[3]=1;
        iFlag[4]=1;
    }

    for(int i=0; i<PITCH_COUNT; i++)
    {
        if(iFlag[i]<0)                                                          //判斷是不是有Alarm
            iResult=iFlag[i];
    }

    if(iResult==0)                                                              //沒Alarm就確認是不是走完了
    {
        for(int i=0; i<PITCH_COUNT; i++)
        {
            if(iFlag[i]==0)
                return 0;
        }
        #ifndef SOFT_SIMULTE
        if(abs(Vari[0]-iCurrPos[0])>5)                                          //Ztex 20240104 : Fixed for pitch move
        {
            bFirstIn=true;
            return 0;
        }
        #endif
        bFirstIn=true;
        ZeroMemory(iFlag, sizeof(iFlag));
        iResult=1;
    }
    bFirstIn=true;
    ZeroMemory(iFlag, sizeof(iFlag));
    return iResult;
}
//----- golden Motor/mymotor.cpp:3938-3965  OutArmCynMove -----
bool OutArmCynMove(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col])                     //Steven for HT1032
{
    if(InOutArmPickerUseMotor!=eptUseMotCyn)
        return true;

    int iCyn;
    bool bFlag=true;
    for(int i=0; i<OutArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMaxCol; j++)
        {
            iCyn=OutArmSuck.Suck[i][j].iMotNo;
            if(i<OutArmSuck.iPickRow && j<OutArmSuck.iPickCol)                  //有用到的才做判斷
            {
                if(ZDownSel[i][j])
                    Cylinder[iCyn].On();
                else
                    Cylinder[iCyn].Off();
            }
            else                                                                //沒用到的一律往上打
            {
                Cylinder[iCyn].Off();
            }
        }
    }

    return bFlag;
}
//----- golden Motor/mymotor.cpp:3967-4044  OutArmZMoveDown -----
bool OutArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bLoader, bool bPreOn)//Steven for HT1032
{   extern bool W906_DropCylOpen(int); if(!W906_DropCylOpen(1)) return false;   //AI(W906-DROPCYL) 20261005: NB2-1 R229 (W-64) -- OUT_ARM_DROP_CYL on: the four anti-drop cylinders open (Pop, _Off in place) before this Out Arm Z goes down (St01 W-56 safe order); off = true at once (golden). mykitsuck.cpp EOF
    int iMot, iCyn;
    bool bFlag=true;
    if(iOutArmZMoveTask==-1 || iOutArmZMoveTask==2)
    {
        ZeroMemory(bOutArmZMove, sizeof(bOutArmZMove));
        iOutArmZMoveTask=1;
    }

    if(InOutArmPickerUseMotor==eptUseMotCyn)
    {
        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                iCyn=OutArmSuck.Suck[i][j].iMotNo;
                if(i<OutArmSuck.iPickRow && j<OutArmSuck.iPickCol)              //有用到的才做判斷
                {
                    if(ZDownSel[i][j])
                    {
                        if(bPreOn)
                            OutArmSuck.Suck[i][j].On();

                        Cylinder[iCyn].On();
                    }
                    else
                    {
                        Cylinder[iCyn].Off();
                    }
                }
                else                                                            //沒用到的一律往上打
                {
                    Cylinder[iCyn].Off();
                }
            }
        }

        bFlag=MOT[MOutArmZA].MotorMove2SpeedForPicker(iZPos[0][0], &ArmSpeed[OutArm], bLoader);
    }
    else
    {
        for(int i=0; i<OutArmSuck.iMotRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMotCol; j++)
            {
                iMot=OutArmSuck.Suck[i][j].iMotNo;
                if(i<OutArmSuck.iPickRow && j<OutArmSuck.iPickCol)              //有用到的才做判斷
                {
                    if(ZDownSel[i][j])
                    {
                        if(bPreOn)
                            OutArmSuck.Suck[i][j].On();
                        if(bOutArmZMove[i][j]==false)
                            bOutArmZMove[i][j]=MOT[iMot].MotorMove2SpeedForPicker(iZPos[i][j], &ArmSpeed[OutArm], bLoader);      //Steven 20140217 : 兩段速移動  //JerryYang 20190729 二段速功能可選擇only at loader
                    }
                    else
                    {
                        bOutArmZMove[i][j]=true;
                    }
                }
                else
                {
                    bOutArmZMove[i][j]=true;
                }

                if(bOutArmZMove[i][j]==false)
                    bFlag=false;
            }
        }
    }

    if(bFlag)
    {
        iOutArmZMoveTask=-1;
    }
    return bFlag;
}
//----- golden Motor/mymotor.cpp:4046-4090  OutArmZMoveUp -----
bool OutArmZMoveUp(int iZPos, bool bLoader)                                     //Steven for HT1032
{
    int iMot, iCyn;
    bool bFlag=true;
    if(iOutArmZMoveTask==-1 || iOutArmZMoveTask==1)
    {
        ZeroMemory(bOutArmZMove, sizeof(bOutArmZMove));
        iOutArmZMoveTask=2;
    }

    if(InOutArmPickerUseMotor==eptUseMotCyn)
    {
        for(int i=0; i<OutArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMaxCol; j++)
            {
                iCyn=OutArmSuck.Suck[i][j].iMotNo;
                Cylinder[iCyn].Off();
            }
        }

        bFlag=MOT[MOutArmZA].MotorMove2SpeedForPicker(iZPos, &ArmSpeed[OutArm], bLoader);
    }
    else
    {
        for(int i=0; i<OutArmSuck.iMotRow; i++)
        {
            for(int j=0; j<OutArmSuck.iMotCol; j++)
            {
                iMot=OutArmSuck.Suck[i][j].iMotNo;
                if(bOutArmZMove[i][j]==false)
                    bOutArmZMove[i][j]=MOT[iMot].MotorMove2SpeedForPicker(iZPos, &ArmSpeed[OutArm], bLoader);      //Steven 20140217 : 兩段速移動  //JerryYang 20190729 二段速功能可選擇only at loader

                if(bOutArmZMove[i][j]==false)
                    bFlag=false;
            }
        }
    }

    if(bFlag)
    {
        iOutArmZMoveTask=-1;
    }
    return bFlag;
}
//----- golden Motor/mymotor.cpp:4094-4545  OutArmContinuousMove_9045 -----
bool OutArmContinuousMove_9045(int X, int Y, int Vari[X_PITCH_COUNT], int YVari, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool ZNeedDown, bool bLoader)  //Steven for HT1032
{
    // ⛔ AI(W906-P0-7) 20260921: NULL 守衛 —— golden 這裡**沒有**，這是移植樹加的。
    //
    //  golden :4121 第一件事就是 `MOT[MOutArmX].Motor->MotionDone()`，無守衛。
    //  golden 可以，因為它的 `InitialMotorParameter()` 一定掛得到 driver。
    //
    //  **MEASURED, not predicted**：翻完這一家族之後 `W6_2_OutArmCore` SEGFAULT，
    //  堆疊是 main -> DoOutArmIonFanGiveWay -> MoveOutArm2XYToDecayTeach ->
    //  OutArmContinuousMove_9045，崩在上面那一行。測試行程不掛馬達，
    //  所以 `MOT[].Motor` 是 NULL。
    //
    //  ⚠ 這**不是**「怕機台會動」的閘，是這棵樹既有的前例：
    //    `aoutarm.cpp:869 MoveOutArmXY_ToFix_Tray_Full()` 開頭一模一樣，
    //    而它的橫幅把理由寫完整了 ——「回 false 是 **golden 自己可達**的路徑
    //    （本函式底下就有多處 return false），呼叫端把 false 當成
    //    『這輪還沒移到，下個 cycle 再試』，那正是一台 X 軸沒有 driver 的機器
    //    所處的狀態」。
    //
    //  ⚠ **對生產路徑零影響**：`wb_serve` 裡 164 個馬達全部非 NULL ——
    //    這可以從已測事實演繹：`ProcessMotorHome` 入口只要有任何一個是 NULL
    //    就印 "refusing -- machine not brought up" 並 return，而它 20260921
    //    實測命中 427 次。⇒ 守衛在生產路徑上恆為 false，一行都不改變行為。
    //
    //  UN-GUARD：等測試夾具也掛上離線 driver（`Motor/mySimMotor.cpp` 的
    //  TMySimMotor 就是為此存在）。那是 PT-W5a「三部分」的第 3 部分，
    //  Q18 用離線守衛取代掉了 Gali 那一側，但 out-arm 這一側還沒做。
    if(MOT[MOutArmX].Motor==NULL || MOT[MOutArmY].Motor==NULL)
        return false;

    AnsiString str=AnsiString("");
    AnsiString str1=AnsiString(""), str2=AnsiString("");
    int iCheckX=0, iCheckY=0, iEncodeX=0, iEncodeY=0;
    int iXPosdiffer=0, iYPosdiffer=0, iXpos=0, iXMSPos=0, iYpos=0, iYMSPos=0;
    int ret=PNP_DOING;
    int iX=X, iY=Y;
    bool bRet;
    static bool bFirstIn=true;
    static bool bFlag[8];
    static int  iOutArmAllZSafe=1;
    static int  iPitchMoveOk=0;
    static bool bXYMoveFinish=false;
    static bool bXYPosErrorFlag=false;

    static int iLastPosX=-99999;
    static int iLastPosY=-99999;

//    static int iXPosEncoer=0, iYPosEncoder=0;

    if(iLastPosX!=X || iLastPosY!=Y)
    {
        iLastPosX=X;
        iLastPosY=Y;
        iOutArmAllZSafe=1;
        ZeroMemory(bFlag, sizeof(bFlag));
        if(MOT[MOutArmX].Motor->MotionDone()==false)
            MOT[MOutArmX].PCIL132_StopMotor();
        if(MOT[MOutArmY].Motor->MotionDone()==false)
            MOT[MOutArmY].PCIL132_StopMotor();
    }

    if(bFirstIn)
    {
        bFirstIn=false;
        ZeroMemory(bFlag, sizeof(bFlag));
        ZeroMemory(bOutArmZMove, sizeof(bOutArmZMove));
        OutArmPitchMove(Vari, YVari, true);
    }

    //KaiChen 20171228 ：Log Light Scale Data
    //==>
    int iXO=0, iYO=0, iLogCount=500;
    iXO=iX;
    iYO=iY;
    AnsiString S1="";

    if(IniConfig.bA27EnableLightScale==true)
    {
        iCheckX=MOT[MOutArmX].ReadPos();
        iCheckY=MOT[MOutArmY].ReadPos();

        if(iX>iCheckX)
        {
            iX=ChangePosition(iX, 4);
            if(IniConfig.bA27_1LogEnableLightScaleData==true &&
               iLogLightScaleCount_OutArmX1<iLogCount &&
               bLogLightScale_OutArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iXO, iX, iXO-iX);
                if(fMotorTest!=0) fMotorTest->mmo5->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_OutArmX1++;
            }
        }
        else
        {
            iX=ChangePosition(iX, 5);
            if(IniConfig.bA27_1LogEnableLightScaleData==true &&
               iLogLightScaleCount_OutArmX2<iLogCount &&
               bLogLightScale_OutArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iXO, iX, iXO-iX);
                if(fMotorTest!=0) fMotorTest->mmo6->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_OutArmX2++;
            }
        }

        if(iY>iCheckY)
        {
            iY=ChangePosition(iY, 6);
            if(IniConfig.bA27_1LogEnableLightScaleData==true &&
               iLogLightScaleCount_OutArmY1<iLogCount &&
               bLogLightScale_OutArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iYO, iY, iYO-iY);
                if(fMotorTest!=0) fMotorTest->mmo7->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_OutArmY1++;
            }
        }
        else
        {
            iY=ChangePosition(iY, 7);
            if(IniConfig.bA27_1LogEnableLightScaleData==true &&
               iLogLightScaleCount_OutArmY2<iLogCount &&
               bLogLightScale_OutArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iYO, iY, iYO-iY);
                if(fMotorTest!=0) fMotorTest->mmo8->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_OutArmY2++;
            }
        }

        bLogLightScale_OutArm=true;
    }
    //<==
    //KaiChen 20171228 ：Log Light Scale Data

    if(iX>=MOT[MOutArmX].Motor->PSoftLimitP ||                                  //jou 980313 safe protect
       iX<=MOT[MOutArmX].Motor->PSoftLimitN)
    {
        str.sprintf("X=%d", iX);
        ShowErrorMessage("WAR0254", 0, MOutArmX, 0, str);                       //"Out Arm X axis motor will out of limit !"
        return false;
    }

    if(iY>=MOT[MOutArmY].Motor->PSoftLimitP || iY<=MOT[MOutArmY].Motor->PSoftLimitN)
    {
        str.sprintf("Y=%d", iY);
        ShowErrorMessage("WAR0255", 0, MOutArmY, 0, str);                       //"Out Arm Y axis motor will out of limit !"
        return false;
    }

    if(MOT[MOutArmX].fCanMove==false || MOT[MOutArmY].fCanMove==false)
    {
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].PCIL132_StopMotor();
        iOutArmAllZSafe=1;
        return false;
    }

    if(MOT[MOutArmX].Motor->Enable && MOT[MOutArmY].Motor->Enable)
    {
        if(MOT[MOutArmX].Motor->Direction==false)
            iX=-iX;
        if(MOT[MOutArmY].Motor->Direction==false)
            iY=-iY;

        if(bPauseOutMotor)
        {
            iOutArmAllZSafe=1;
            bPauseOutMotor=false;
            OutArmIdle.SetSecAndOn(iIDLETime);
            if(IniConfig.bA62bUseStopMachineArmHome==true)                      //Ztex 2024.10.30 Add Use Stop Machine In/Out Arm Need To Home
            {
                MOT[MOutArmX].PCIL132_StopMotor();
                MOT[MOutArmY].PCIL132_StopMotor();
                iOutArmAllZSafe=1;
                iPitchMoveOk=0;
                SetOutArmHome();
            }
        }

        switch(iOutArmAllZSafe)
        {
            case 1:
                if(MoveOutArmToAutoSafe())
                {
                    iPitchMoveOk=0;
                    OutArmPitchMove(Vari, YVari, true);
                    OutArmCynMove(ZDownSel);
                    OutArmCylinderDelayTimer.SetMSAndOn(ArmSpeed_File[OutArm].dCylinderDelay*1000);
                    OutArmIdle.SetSecAndOn(iIDLETime);
                    if(InOutArmPickerUseMotor!=eptUseMotCyn)
                        iOutArmAllZSafe=100;
                    else
                        iOutArmAllZSafe=50;
                    bXYMoveFinish=false;
                    ZeroMemory(bOutArmZMove, sizeof(bOutArmZMove));

                    InitPCIL112_OutArmXYMoveTask();                             //jou 2011-04-03
                    if(bXYPosErrorFlag)
                    {
                        bFlag[0]=false;
                        bFlag[1]=false;
                        iOutArmAllZSafe=150;
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            case 50:
                if(OutArmCylinderDelayTimer.Off())
                    iOutArmAllZSafe=100;
                else
                    break;
            case 100:
                if(iPitchMoveOk==0)
                    iPitchMoveOk=OutArmPitchMove(Vari, YVari);

                if(iPitchMoveOk>0)
                {
                    iOutArmAllZSafe=200;
                }
                else if(iPitchMoveOk<0)
                {
                }
                break;
            case 150:
                if(bFlag[0]==false)
                    bFlag[0]=MOT[MOutArmX].MotorMove(X+100);
                if(bFlag[1]==false)
                    bFlag[1]=MOT[MOutArmY].MotorMove(Y+100);
                if(bFlag[0] && bFlag[1])
                {
                    bFlag[0]=false;
                    bFlag[1]=false;
                    bXYPosErrorFlag=false;
                    iOutArmAllZSafe=100;
                }
                break;
            case 200:
                if(bXYMoveFinish)
                {
                    if(ZNeedDown)
                    {
                        MOT[MOutArmX].ScanMotorStatus();
                        MOT[MOutArmY].ScanMotorStatus();

                        if(MOT[MOutArmX].Led[iInposLed]==true ||
                           MOT[MOutArmY].Led[iInposLed]==true)                  //Isaac 20170721 (wei) Z axis protection
                            return false;

                        bRet=OutArmZMoveDown(ZDownSel, iZPos, bLoader);
                        OutArmCylinderDelayTimer.SetMSAndOn(ArmSpeed_File[OutArm].dCylinderDelay*1000);
                        if(bRet==false)
                            return false;
                    }

                    iOutArmAllZSafe=1;
                    InitPCIL112_OutArmXYMoveTask();                             //jou 2011-04-03

                    if(CheckOutArmZHomeSensor_2x8(ZNeedDown, ZDownSel)==false)
                    {
                        return false;
                    }

                    if(IniConfig.bA22MagneticScale==true)                       //Frank 20161109 add 磁性尺
                    {
                        iXpos=MOT[MOutArmX].Motor->ReadPos();
                        iXMSPos=MOT[MOutArmXScale].Motor->ReadPos();
                        iYpos=MOT[MOutArmY].Motor->ReadPos();
                        iYMSPos=MOT[MOutArmYScale].Motor->ReadPos();

                        iXPosdiffer=abs(iXpos - iXMSPos);
                        iYPosdiffer=abs(iYpos - iYMSPos);

                        if(iXPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100 &&
                           iYPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                        {
                            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                MyMessageBox->Close();

                            str1.sprintf("Out Arm X & Y Magnetic Scale Discrepancy Encoder Over Setting");
                            str2.sprintf("Out Arm X & Y 磁性尺與編碼器差超出設定");

                            if(iXPosdiffer>IniConfig.dA22MagneticScaleStopRunRange*100 ||
                               iYPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                            {
                                iUnLoaderCount=0;
                            }
                            else
                            {
                                iUnLoaderCount=9;
                            }
                            ShowUnloaderTrayMessage(str1, str2);
                        }
                        else if(iXPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                        {
                            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                MyMessageBox->Close();
                            str1.sprintf("Out Arm X Magnetic Scale Discrepancy Encoder Over Setting");
                            str2.sprintf("Out Arm X 磁性尺與編碼器差超出設定");

                            if(iXPosdiffer>IniConfig.dA22MagneticScaleStopRunRange)
                            {
                                iUnLoaderCount=0;
                            }
                            else
                            {
                                iUnLoaderCount=9;
                            }
                            ShowUnloaderTrayMessage(str1, str2);
                        }
                        else if(iYPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                        {
                            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                MyMessageBox->Close();
                            str1.sprintf("Out Arm Y Magnetic Scale Discrepancy Encoder Over Setting");
                            str2.sprintf("Out Arm Y 磁性尺與編碼器差超出設定");
                            if(iYPosdiffer>IniConfig.dA22MagneticScaleStopRunRange)
                            {
                                iUnLoaderCount=0;
                            }
                            else
                            {
                                iUnLoaderCount=9;
                            }
                            ShowUnloaderTrayMessage(str1, str2);
                        }
                    }

                    if(OutArmCylinderDelayTimer.Off())
                    {
                        iOutArmAllZSafe=1;
                        iPitchMoveOk=0;
                        bLogLightScale_OutArm=false;                            //KaiChen 20171228 ：Log Light Scale Data
                        return true;
                    }
                    else
                    {
                        iOutArmAllZSafe=210;
                        break;
                    }
                }

                if(iPitchMoveOk==0)                                             //Steven 20150718 : Fixed for Y-Pitch
                {
                    iOutArmAllZSafe=1;
                }
                break;
            case 210:
                if(OutArmCylinderDelayTimer.Off())
                {
                    iOutArmAllZSafe=1;
                    iPitchMoveOk=0;
                    bLogLightScale_InArm=false;                                 //KaiChen 20171228 ：Log Light Scale Data
                    return true;
                }
                break;
            default :
                iOutArmAllZSafe=1;
                return false;
        }

        if(bXYPosErrorFlag)
            return false;

        if(bXYMoveFinish==false &&                                              //Steven 20231214 : 避免重複進去
           iPitchMoveOk>=0)                                                     //避免Pitch Alarm時, XY還在動
            ret=PCIL112_OutArmXYMove(X, Y);                                     //jou 2011-04-03
        if(ret==PNP_DONE)
        {
            if(iPitchMoveOk>0)                                                  //Steven 20150718 : Fixed for Y-Pitch
            {
                if(USE_MAGNETIC_SCALE==false)                                   //Ifor 20180622 (Steven) : 避免位置錯誤
                {
                    iCheckX=MOT[MOutArmX].ReadPos();
                    iCheckY=MOT[MOutArmY].ReadPos();

                    if((X!=iCheckX) || (Y!=iCheckY))
                    {
                        iEncodeX=MOT[MOutArmX].ReadEncoderPos();                //Steven 20110314 : 比對Encoder位置
                        iEncodeY=MOT[MOutArmY].ReadEncoderPos();                //Steven 20110314 : 比對Encoder位置

                        str1.sprintf(" Move Finish:%d ", bXYMoveFinish);        //Ifor 20181127 : add  bXYMoveFinish Log
                        MNetLog(" iX="+AnsiString(iX)+" iCheckX="+AnsiString(iCheckX)+" iEncodeX="+AnsiString(iEncodeX)+
                                " iY="+AnsiString(iY)+" iCheckY="+AnsiString(iCheckY)+" iEncodeY="+AnsiString(iEncodeY)+ " Out Arm Pos Error"+ str1);   //Steven 20110402   //Ifor 20181127 : add  bXYMoveFinish Log
                        if(X>(iEncodeX+iTorence) || X<(iEncodeX-iTorence) ||
                           Y>(iEncodeY+iTorence) || Y<(iEncodeY-iTorence))      //Steven 20110314 : 比對Encoder位置
                        {
                            iOutArmAllZSafe=1;
                            iPitchMoveOk=0;
                            MOT[MOutArmX].PCIL132_StopMotor();
                            MOT[MOutArmY].PCIL132_StopMotor();
                            bXYPosErrorFlag=true;
                            bXYMoveFinish=false;                                //Ifor 20181127 : add Encoder位置比對異常時，設定bXYMoveFinish為False避免誤動作
                            return false;
                        }
                    }
                }

                bXYMoveFinish=true;

                if(ZNeedDown)
                {
                    MOT[MOutArmX].ScanMotorStatus();
                    MOT[MOutArmY].ScanMotorStatus();

                    if(MOT[MOutArmX].Led[iInposLed]==true ||
                       MOT[MOutArmY].Led[iInposLed]==true)                      //Isaac 20170721 (wei) Z axis protection
                        return false;

                    bRet=OutArmZMoveDown(ZDownSel, iZPos, bLoader);
                }
                return false;
            }
        }

        if(OutArmIdle.Off())
        {
            MOT[MOutArmX].PCIL132_StopMotor();
            MOT[MOutArmY].PCIL132_StopMotor();
            iOutArmAllZSafe=1;
            iPitchMoveOk=0;
            SetOutArmHome();
            OutArmIdle.SetSecAndOn(iIDLETime);
        }
    }
    else
    {
        //動作模擬
        if(bFlag[0]==false)
            bFlag[0]=OutArmZMoveUp(ZSafePos, bLoader);

        if(bFlag[0])
        {
            if(bFlag[1]==false)
            {
                OutArmPitchMove(Vari, YVari, true);
                InitPCIL112_OutArmXYMoveTask();
                bFlag[1]=true;
            }

            if(bFlag[2]==false)
            {
                ret=PCIL112_OutArmXYMove(X, Y);                                 //Steven 20240603 : [E77]改放到PCIL112_OutArmXYMove裡面
                bFlag[2]=(ret==PNP_DONE);
            }

            if(bFlag[3]==false)
                bFlag[3]=OutArmPitchMove(Vari, YVari);
        }

        if(bFlag[0] && bFlag[1] && bFlag[2] && bFlag[3])
        {
            if(InOutArmPickerUseMotor==eptUseMot)                               //Steven for HT1032
                bFlag[4]=true;
            else
                bFlag[4]=OutArmCynMove(ZDownSel);
        }

        if(bFlag[0] && bFlag[1] && bFlag[2] && bFlag[3] && bFlag[4])
        {
            if(ZNeedDown)
                bRet=OutArmZMoveDown(ZDownSel, iZPos, bLoader);
            else
                bRet=OutArmZMoveUp(ZSafePos, bLoader);

            if(bRet==false)
                return false;

            ZeroMemory(bFlag, sizeof(bFlag));
            ZeroMemory(bOutArmZMove, sizeof(bOutArmZMove));
            bLogLightScale_OutArm=false;                                        //KaiChen 20171228 ：Log Light Scale Data
            return true;
        }
    }
    return false;
}

//==============================================================================
//  InArm 連續移動家族 -- golden Motor/mymotor.cpp（六支 ＋ PCIL112 InArm 那一對，共 829 行）
//  AI(W906-B1-INARM) 20260924
//
//  ## 為什麼（使用者 B1：「F5 驗收沒有持續跳進 void DoInArm_9045()」）
//
//  主迴圈 20260924 gdb 實測：歸零後 Index 呼叫 IndexAlarmInArmAway()
//  （atester.cpp:297，經 aTester_Front.cpp:7613 DoTestYFront）把
//  bInArmNeedToSafePos 設 true；DoInArm()（ainarm2.cpp:5745-5760）於是每隔一個
//  tick 就走 MoveInArm2XYToWait()（ainarm2.cpp:5059）
//    -> InArmContinuousMove_9045(Prod.iInArmSafeX, Prod.iInArmSafeY, ...)
//    -> 本樹連到的是本檔 :2520 原本的一行式樁 `{ return false; }`
//  ⇒ in-arm 一步都不動（X 停在 20、目標 1326；Y 停在 -10000、目標 -48898），
//    旗標永遠不清，DoInArm_9045() 再也進不去。
//
//  ## 翻了什麼（本體逐字照 golden，cp950 -> UTF-8）
//
//      SetInArmPitchSpeed            golden :2560-2584   25 行
//      InArmPitchMove                golden :2586-2717  132 行
//      InArmCynMove                  golden :2721-2749   29 行
//      InArmZMoveDown                golden :2751-2832   82 行
//      InArmZMoveUp                  golden :2834-2882   49 行
//      InArmContinuousMove_9045      golden :2886-3325  440 行
//      InitPCIL112_InArmXYMoveTask   golden :4640-4646    7 行
//      PCIL112_InArmXYMove           golden :4648-4712   65 行
//  （行範圍用「去掉註解與字串後的大括號配對」量；同一支工具量 P0-7 那五支，
//    結果與 P0-7 記錄的 :3327-3454 / :3938-3965 / :3967-4044 / :4046-4090 /
//    :4094-4545 逐一相同。）
//
//  ⛔ 為什麼連 PCIL112 那一對也翻（任務原本只列六支）：
//    本檔 :2571 的舊樁 `int PCIL112_InArmXYMove(int,int) { return 0; }` —— 而本檔
//    :78 `#define PNP_DONE 0`。也就是說那個樁回的是**「XY 已到位」**，卻一步都沒命令。
//    InArmContinuousMove_9045 的兩個分支都被它帶錯：
//    - Motor->Enable 為真（golden :3003-3274）：成功路徑（:3211-3248）拿它當 XY 走完的
//      訊號，接著比 ReadPos 與 X/Y；差超過 iTorence（本檔 :120 = 10，與 golden :51 同值）
//      就設 bXYPosErrorFlag、走 case 150 去 MotorMove(X+100)，下一輪再比又差 100 ——
//      **永遠到不了 `return true`**。
//    - Motor->Enable 為假（「動作模擬」分支，golden :3275-3323）：:3294
//      `bFlag[2]=(ret==PNP_DONE)` 第一輪就成立 ⇒ 回 true，但 X/Y **從沒被命令過** ——
//      假成功，in-arm 位置停在原地。依 P0-7 的推論（MotorMove 在 !Enable 時回 1，
//      歸零 1540 因此過關），F5/SIM 的馬達是 !Enable，走的就是這條；本波未實測 wb_serve。
//    這一對在全樹**沒有其他呼叫端**（git grep：只有本檔與 mymotor.h:445-446），
//    所以翻它只影響 InArm 連續移動本身。
//    ⚠ OutArm 那一對（本檔 :2572-2573 `InitPCIL112_OutArmXYMoveTask` /
//      `PCIL112_OutArmXYMove`，golden :4714-4720 / :4722-4821）當時是**同一種樁**，
//      P0-7 翻的 OutArmContinuousMove_9045 呼叫的就是它。B1 沒有動（會改變歸零 case 1540 路徑）；
//      AI(W906-AMB-L2) 20260929 已照 golden 翻在本檔尾端（RULINGS_20260929 第 11 條），:2572-2573 改成退休註解。
//
//  ## 本體放尾端、宣告只補兩個
//
//  理由同 P0-7：本體要 `MyMessageBox`（mymessbox_shim.h）與 `fMotorTest`
//  （forms/fMotorTest.h；InArm 用 mmo1..mmo4，:799-802），那兩個 header 在本檔
//  :2996-2997 才 include。golden 經 cinitial.h（golden mymotor.cpp:17）拿到的
//  SetMotorScaleSpeed / SetMotorAccelSpeed 只補宣告、不 include 本樹的 cinitial.h
//  （同 P0-7 對 MoveOutArmToAutoSafe 的處置）。本體在 cinitial.cpp:13474 / :16730，
//  nm 量到在 libht9045_sm.a（cinitial.cpp.obj，T）。
//
//  golden 夾在這幾支之間的全域都**已存在**，不重出：
//    :2556-2557 iIDLETime / InArmIdle            -> 本檔「golden :2556-2557」那段
//    :2558      extern MoveInArmZToPlateSafe      -> aHotPlateSubstrate.h:947（本體 ainarm2.cpp:4634）
//    :2719      iInArmZMoveTask                   -> 本檔 :105（⚠ 初值 1，golden -1，既有差異，未動；
//               見下「刻意保留的差異」）
//    :2720      bInArmZMove                       -> 本檔「golden :2720」那段
//    :2884      InArmCylinderDelayTimer           -> 本檔「golden :2884」那段（HTimer -> TQPF_Timer，PT-W7n）
//
//  ## 相依：用編譯器判，不用 grep
//
//  本檔全文 ＋ golden 八支本體逐字拼成一個 TU，SIM（預設）與 REAL
//  （-DW906_NO_SOFT_SIMULTE）兩組態各跑一次 `g++ -fsyntax-only`。錯誤只有三種：
//    SetMotorAccelSpeed / SetMotorScaleSpeed 未宣告  -> 補宣告（本體存在，見上）
//    MyDBIProcess 參數太多（golden :4680 是 3 參數）  -> 見下 GATE(B1-1)
//  ⇒ 只有一個閘，而且不是「相依不存在」的閘，是「形狀不同」的閘：
//    GATE(B1-1) golden :4680 `MyDBIProcess("Motion", StrE, "");`
//      本 TU 看得到的只有 2 參數版（aHotPlateSubstrate.h:979，本體 aHotPlateSubstrate.cpp:1275）。
//      3 參數版**存在**於 cMyDB.h:81（__fastcall，本體在 ht9045_secsgem），但 include 它
//      會替 motor 庫拉進 secsgem 相依 —— 本樹對同一件事已有兩個前例都選了不拉：
//      asortarm.cpp:2476-2480 GATE(13)、ainarm2.cpp:4596/:4667-4669 GATE W7E-1。
//      **這裡 golden 的第 3 參數是空字串**，所以 2 參數呼叫帶的內容與 golden 完全相同，
//      連前例那種「把第 3 參數摺進訊息」都不需要。
//
//  ## 移植樹加的一段：InArmContinuousMove_9045 開頭的 NULL 守衛（量測之後才加）
//
//  第一版照 golden 不加守衛，ctest `AutoClean` 由 Passed 變 SEGFAULT；gdb 堆疊
//  main -> DoAutoCleanPickfromCleanKit -> MoveInArm2XYToShuttle2Wait ->
//  InArmContinuousMove_9045（golden :2910 無守衛解參考 `MOT[MInArmX].Motor`，測試不掛馬達）。
//  於是加上與 OutArm 孿生（AI(W906-P0-7)）逐字同形的守衛；理由與「對生產路徑零影響」
//  的推論寫在守衛本身。回 false 正是舊樁給的答案，測試行程行為與改前相同。
//
//  ## 刻意保留的差異 / golden 怪處（全部照翻，本體內不加註，好讓對 golden 的 diff
//  只剩上面那段守衛與 GATE(B1-1)）
//
//   (1) InArmPitchMove :2617 算了 bMoveClose 卻從不讀（它本身沒呼叫 SetInArmPitchSpeed；
//       本樹 git grep SetInArmPitchSpeed 在本檔之外是 0 個呼叫端）。
//   (2) InArmPitchMove :2704-2708（僅 REAL 組態）位置差 >5 回 **-1**；OutArm 孿生 :3441-3445 回 0。
//   (3) InArmPitchMove :2613 bInit 分支先問 MotionDone() 才下群組移動；OutArm 孿生沒有這一問。
//       :2636 SetGroup(1,...) vs OutArm :3376 SetGroup(2,...)（各臂一個群組號，不是怪處）。
//   (4) InArmContinuousMove case 1 -> 50 -> 100 沒有 break，是 golden 刻意的 fall-through（OutArm 同）。
//   (5) case 100 :3057 `iPitchMoveOk==2` 分支：InArmPitchMove 不會回 2（只回 0/1/-1/負的 alarm），
//       golden 的死分支，照留。
//   (6) 磁性尺 :3128-3129 在「X、Y 都超過 Keep」的分支裡又用 `iYPosdiffer>Keep*100` 判 Stop，
//       恆真 ⇒ iUnLoaderCount 恆為 0；:3147 / :3163 拿 StopRunRange 比卻沒乘 100（Keep 有乘）。
//       與 OutArm 孿生同形，照留。
//   (7) 模擬分支（Motor->Enable==false）:3288 在第一次 pitch 初始化後就 `return false`；OutArm 沒有。
//   (8) InArm 連續移動從不清 bInArmZMove（OutArm 孿生在 :4131 / :4262 / :4539 清 bOutArmZMove）。
//   (9) InArmZMoveDown 馬達分支 :2801-2819 把「有用到的才判斷」整段註解掉 ⇒ 沒用到的吸嘴只要
//       ZDownSel 為 true 也會下；OutArm 孿生 :4014 沒註解。照留。
//  (10) PCIL112_InArmXYMove :4668 有 `CUSTOMER_CODE!=CC_ASE_KaohSiung` 豁免，OutArm 孿生 :4743 沒有。
//  (11) iInArmZMoveTask 初值：本檔 :105 是 1、golden :2719 是 -1（既有差異，本波未動）。
//       實際等價：第一次呼叫 InArmZMoveDown 時 golden 會清 bInArmZMove 並設 1，本樹不清但
//       值已是 1；第一次呼叫 InArmZMoveUp 兩邊都清並設 2。bInArmZMove 是靜態儲存（初值全 0），
//       全樹只有本檔在寫它（git grep）。
//
//  ## 仍然是樁、但新本體會走到的相依（不在本波範圍，列給主迴圈）
//    InArmZSafe(int)  本檔 :2566 `{ return -1; }` —— -1 = 「所有 Z 都安全」。
//      PCIL112_InArmXYMove :4663 的 Z home sensor 保護因此永遠不觸發（放行方向）。
//      MoveInArmZToPlateSafe（ainarm2.cpp:4634，case 1 的入口）也吃它。
//    （已解 —— AI(W906-C21-MOTORMOVE) 20260924）TMyMotor::MotorMove / MotorMove2SpeedForPicker / MotorMovePosition
//      的真驅動路徑原本是 W4 樁（回 0 / false）；C21 已照 golden :539-1000 翻在本檔尾端，
//      原本 :721 / :851 / :874 的三支樁原地改成退休註解（行號不位移）。!Enable 仍一次到位，
//      但現在先過 golden 的安全門與 fCanMove*/mapLockList 互鎖（檔尾橫幅〈!Enable 的終端〉）。
//==============================================================================
// AI(W906-B1-INARM) 20260924: golden 經 cinitial.h 取得（golden mymotor.cpp:17）；只補宣告，簽名逐字抄 golden cinitial.h:50-51。
void SetMotorScaleSpeed(int Index, int ScaleSpeed);                             // golden cinitial.h:50 / 本樹 cinitial.h:184（本體 cinitial.cpp:13474）
void SetMotorAccelSpeed(int Index, int ADCSpeed);                               // golden cinitial.h:51 / 本樹 cinitial.h:256（本體 cinitial.cpp:16730）
//----- golden Motor/mymotor.cpp:2560-2584  SetInArmPitchSpeed -----
void SetInArmPitchSpeed(bool bMoveClose)
{
    if(bMoveClose)                                                              //內縮
    {
        SetMotorAccelSpeed(MInArmPitch, 100);
        SetMotorScaleSpeed(MInArmPitch, ArmSpeed[InArm].iVariSP);
        SetMotorAccelSpeed(MInArmPitchX2, 100);
        SetMotorScaleSpeed(MInArmPitchX2, double(ArmSpeed[InArm].iVariSP)*0.90);
        SetMotorAccelSpeed(MInArmPitchX3, 100);
        SetMotorScaleSpeed(MInArmPitchX3, double(ArmSpeed[InArm].iVariSP)*0.80);
        SetMotorAccelSpeed(MInArmPitchX4, 100);
        SetMotorScaleSpeed(MInArmPitchX4, double(ArmSpeed[InArm].iVariSP)*0.70);
    }
    else
    {
        SetMotorAccelSpeed(MInArmPitchX4, 100);
        SetMotorScaleSpeed(MInArmPitchX4, double(ArmSpeed[InArm].iVariSP));
        SetMotorAccelSpeed(MInArmPitchX3, 100);
        SetMotorScaleSpeed(MInArmPitchX3, double(ArmSpeed[InArm].iVariSP)*0.80);
        SetMotorAccelSpeed(MInArmPitchX2, 100);
        SetMotorScaleSpeed(MInArmPitchX2, double(ArmSpeed[InArm].iVariSP)*0.70);
        SetMotorAccelSpeed(MInArmPitch, 100);
        SetMotorScaleSpeed(MInArmPitch, double(ArmSpeed[InArm].iVariSP)*0.60);
    }
}
//----- golden Motor/mymotor.cpp:2586-2717  InArmPitchMove -----
int InArmPitchMove(int Vari[X_PITCH_COUNT], int YVari, bool bInit)              //Steven for HT1032
{
    static bool bMoveClose=false;
    static bool bFirstIn=true;
    static int  iFlag[PITCH_COUNT];
    bool bFlag;
    int iResult=0;
    int iPos=0;
    #ifndef SOFT_SIMULTE
    long iCurrPos[X_PITCH_COUNT]={MOT[MInArmPitch  ].Motor->ReadPos(),
                      MOT[MInArmPitchX2].Motor->ReadPos(),
                      MOT[MInArmPitchX3].Motor->ReadPos(),
                      MOT[MInArmPitchX4].Motor->ReadPos()};

    int iDevPos[X_PITCH_COUNT];
    BYTE bDevNo[X_PITCH_COUNT]={MOT[MInArmPitch  ].Motor->iPortID,
                    MOT[MInArmPitchX2].Motor->iPortID,
                    MOT[MInArmPitchX3].Motor->iPortID,
                    MOT[MInArmPitchX4].Motor->iPortID};
    #endif

    if(USE_PICKER_COUNT==ep1Picker)
        return 1;

    if(bInit || bFirstIn)
    {
        ZeroMemory(iFlag, sizeof(iFlag));
        bFlag=MOT[MInArmPitch].Motor->MotionDone();
        if(bFlag==true)
        {
            iPos=MOT[MInArmPitch].ReadPos();
            bMoveClose=(Vari[0]<iPos);
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      //Steven for HT1032
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.06 Add HT-1032
            {
                #ifdef SOFT_SIMULTE                                             //Ztex Add SOFT_SIMULTE Pass
                    iFlag[1]=MOT[MInArmPitch  ].MotorMove(Vari[0]);
                    iFlag[2]=MOT[MInArmPitchX2].MotorMove(Vari[1]);
                    iFlag[3]=MOT[MInArmPitchX3].MotorMove(Vari[2]);
                    iFlag[4]=MOT[MInArmPitchX4].MotorMove(Vari[3]);
                #else
                    iDevPos[0]=Vari[0]-iCurrPos[0];
                    iDevPos[1]=Vari[1]-iCurrPos[1];
                    iDevPos[2]=Vari[2]-iCurrPos[2];
                    iDevPos[3]=Vari[3]-iCurrPos[3];
                    MOT[MInArmPitch  ].GetRealPos(&iDevPos[0]);
                    MOT[MInArmPitchX2].GetRealPos(&iDevPos[1]);
                    MOT[MInArmPitchX3].GetRealPos(&iDevPos[2]);
                    MOT[MInArmPitchX4].GetRealPos(&iDevPos[3]);
                    long lDevPos[4]={iDevPos[0], iDevPos[1], iDevPos[2], iDevPos[3]};
                    MOT[MInArmPitch].Motor->SetGroup(1, 4, bDevNo);
                    MOT[MInArmPitch].Motor->LineNMove(bDevNo, lDevPos, 4);
                #endif
            }
        }
        bFirstIn=false;
        return 0;
    }

    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
       USE_IN_OUT_ARM_Y_PITCH==iXYPitchIn_Bb_Out_Bc)                            //Ztex 2024.02.24 Add HT-1132
    {
        if(iFlag[0]==0)
            iFlag[0]=MOT[MInArmPitchY ].MotorMove(YVari);                       //ChungHung 20131231 alter AutoYPitch
        if(iFlag[1]==0)
            iFlag[1]=MOT[MInArmPitch  ].MotorMove(Vari[0]);
        if(iFlag[2]==0)
            iFlag[2]=MOT[MInArmPitchX2].MotorMove(Vari[1]);                     //Steven 20131002 : XY變距
        iFlag[3]=1;
        iFlag[4]=1;
    }
    else if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                         //Steven for HT1032
            USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                            //Ztex 2023.12.06 Add HT-1032
    {
        if(iFlag[4]==0)
            iFlag[4]=MOT[MInArmPitchY].MotorMove(YVari);

        if(iFlag[0]==0)
        {
            #ifndef SOFT_SIMULTE
            bFlag=MOT[MInArmPitch].Motor->MotionDone();
            #else
            bFlag=true;
            #endif
            if(bFlag)
                iFlag[0]=1;
        }
        iFlag[1]=1;
        iFlag[2]=1;
        iFlag[3]=1;
    }
    else
    {
        if(iFlag[0]==0)
            iFlag[0]=MOT[MInArmPitch].MotorMove(Vari[0]);
        iFlag[1]=1;
        iFlag[2]=1;
        iFlag[3]=1;
        iFlag[4]=1;
    }

    for(int i=0; i<PITCH_COUNT; i++)
    {
        if(iFlag[i]<0)  //判斷是不是有Alarm
            iResult=iFlag[i];
    }

    if(iResult==0)      //沒Alarm就確認是不是走完了
    {
        for(int i=0; i<PITCH_COUNT; i++)
        {
            if(iFlag[i]==0)
            {
                return 0;
            }
        }

        #ifndef SOFT_SIMULTE
        if(abs(Vari[0]-iCurrPos[0])>5)
        {
            bFirstIn=true;
            return -1;
        }
        #endif
        bFirstIn=true;
        ZeroMemory(iFlag, sizeof(iFlag));
        iResult=1;
    }
    bFirstIn=true;
    ZeroMemory(iFlag, sizeof(iFlag));
    return iResult;
}
//----- golden Motor/mymotor.cpp:2721-2749  InArmCynMove -----
bool InArmCynMove(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col])                      //Steven for HT1032
{
    if(InOutArmPickerUseMotor!=eptUseMotCyn)
        return true;

    bool bFlag=true;
    int iCyn;

    for(int i=0; i<InArmSuck.iMaxRow; i++)
    {
        for(int j=0; j<InArmSuck.iMaxCol; j++)
        {
            iCyn=InArmSuck.Suck[i][j].iMotNo;
            if(i<InArmSuck.iPickRow && j<InArmSuck.iPickCol)                    //有用到的才做判斷
            {
                if(ZDownSel[i][j])
                    Cylinder[iCyn].On();
                else
                    Cylinder[iCyn].Off();
            }
            else                                                                //沒用到的一律往上打
            {
                Cylinder[iCyn].Off();
            }
        }
    }

    return bFlag;
}
//----- golden Motor/mymotor.cpp:2751-2832  InArmZMoveDown -----
bool InArmZMoveDown(bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool bLoader, bool bPreOn)//Steven for HT1032
{   extern bool W906_DropCylOpen(int); if(!W906_DropCylOpen(0)) return false;   //AI(W906-DROPCYL) 20261005: NB2-1 R229 (W-64) -- IN_ARM_DROP_CYL on: the four anti-drop cylinders open (Pop, _Off in place) before this In Arm Z goes down (St01 W-56 safe order); off = true at once (golden). mykitsuck.cpp EOF
    int iMot, iCyn;
    bool bFlag=true;
    if(iInArmZMoveTask==-1 || iInArmZMoveTask==2)                               //JerryYang 20230820
    {
        ZeroMemory(bInArmZMove, sizeof(bInArmZMove));
        iInArmZMoveTask=1;
    }

    if(InOutArmPickerUseMotor==eptUseMotCyn)
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                iCyn=InArmSuck.Suck[i][j].iMotNo;
                if(i<InArmSuck.iPickRow && j<InArmSuck.iPickCol)                //有用到的才做判斷
                {
                    if(ZDownSel[i][j])
                    {
                        if(bPreOn)
                            InArmSuck.Suck[i][j].On();

                        bInArmZMove[i][j]=true;
                        Cylinder[iCyn].On();
                    }
                    else
                    {
                        bInArmZMove[i][j]=true;
                        Cylinder[iCyn].Off();
                    }
                }
                else                                                            //沒用到的一律往上打
                {
                    bInArmZMove[i][j]=true;
                    Cylinder[iCyn].Off();
                }
            }
        }

        bFlag=MOT[MInArmZA].MotorMove2SpeedForPicker(iZPos[0][0], &ArmSpeed[InArm], bLoader);
    }
    else
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                iMot=InArmSuck.Suck[i][j].iMotNo;
//                if(i<InArmSuck.iPickRow && j<InArmSuck.iPickCol)                //有用到的才做判斷
//                {
                    if(ZDownSel[i][j])
                    {
                        if(bPreOn)
                            InArmSuck.Suck[i][j].On();

                        if(bInArmZMove[i][j]==false)
                            bInArmZMove[i][j]=MOT[iMot].MotorMove2SpeedForPicker(iZPos[i][j], &ArmSpeed[InArm], bLoader);      //Steven 20140217 : 兩段速移動  //JerryYang 20190729 二段速功能可選擇only at loader
                    }
                    else
                    {
                        bInArmZMove[i][j]=true;
                    }
//                }
//                else
//                {
//                    bInArmZMove[i][j]=true;
//                }

                if(bInArmZMove[i][j]==false)
                    bFlag=false;
            }
        }
    }

    if(bFlag)
    {
        iInArmZMoveTask=-1;
    }
    return bFlag;
}
//----- golden Motor/mymotor.cpp:2834-2882  InArmZMoveUp -----
bool InArmZMoveUp(int iZPos, bool bLoader)                                      //Steven for HT1032
{
    int iMot, iCyn;
    bool bFlag=true;
    if(iInArmZMoveTask==-1 || iInArmZMoveTask==1)                               //JerryYang 20230820
    {
        ZeroMemory(bInArmZMove, sizeof(bInArmZMove));
        iInArmZMoveTask=2;
    }

    if(InOutArmPickerUseMotor==eptUseMotCyn)
    {
        for(int i=0; i<InArmSuck.iMaxRow; i++)
        {
            for(int j=0; j<InArmSuck.iMaxCol; j++)
            {
                iCyn=InArmSuck.Suck[i][j].iMotNo;
                if(bInArmZMove[i][j]==false)
                {
                    Cylinder[iCyn].Off();
                    bInArmZMove[i][j]=true;
                }
            }
        }

        bFlag=MOT[MInArmZA].MotorMove2SpeedForPicker(iZPos, &ArmSpeed[InArm], bLoader);
    }
    else
    {
        for(int i=0; i<InArmSuck.iMotRow; i++)
        {
            for(int j=0; j<InArmSuck.iMotCol; j++)
            {
                iMot=InArmSuck.Suck[i][j].iMotNo;
                if(bInArmZMove[i][j]==false)
                    bInArmZMove[i][j]=MOT[iMot].MotorMove2SpeedForPicker(iZPos, &ArmSpeed[InArm], bLoader);      //Steven 20140217 : 兩段速移動  //JerryYang 20190729 二段速功能可選擇only at loader

                if(bInArmZMove[i][j]==false)
                    bFlag=false;
            }
        }
    }

    if(bFlag)
    {
        iInArmZMoveTask=-1;
    }
    return bFlag;
}
//----- golden Motor/mymotor.cpp:2886-3325  InArmContinuousMove_9045 -----
bool InArmContinuousMove_9045(int X, int Y, int Vari[X_PITCH_COUNT], int YVari, bool ZDownSel[MAX_ARM_Row][MAX_ARM_Col], int iZPos[MAX_ARM_Row][MAX_ARM_Col], bool ZNeedDown, bool bLoader)     //Steven for HT1032
{
    // ⛔ AI(W906-B1-INARM) 20260924: NULL 守衛 —— golden 這裡**沒有**，這是移植樹加的，
    //   與 OutArm 孿生（本檔 OutArmContinuousMove_9045 開頭，AI(W906-P0-7)）逐字同形。
    //
    //  golden :2910 第一件事就是 `MOT[MInArmX].Motor->MotionDone()`，無守衛。
    //
    //  **MEASURED, not predicted**：翻完這一家族、還沒加守衛時 ctest `AutoClean`
    //  SEGFAULT（基準那一輪它是 Passed）。gdb 堆疊：main -> DoAutoCleanPickfromCleanKit
    //  -> MoveInArm2XYToShuttle2Wait -> InArmContinuousMove_9045，崩在本函式裡。
    //  測試行程不掛馬達，所以 `MOT[].Motor` 是 NULL。
    //
    //  回 false 是 golden 自己可達的路徑（本函式底下多處 return false），呼叫端把它當
    //  「這輪還沒移到，下個 cycle 再試」—— 也正是舊樁 `{ return false; }` 給的答案，
    //  所以測試行程看到的行為與改前完全相同。
    //  ⚠ 對生產路徑零影響：理由同 OutArm 孿生那段（wb_serve 能走到歸零就代表 164 個
    //    馬達全部非 NULL，ProcessMotorHome 入口只要有一個 NULL 就拒絕）⇒ 守衛恆為 false。
    //  UN-GUARD：等測試夾具掛上離線 driver（Motor/mySimMotor.cpp 的 TMySimMotor）。
    if(MOT[MInArmX].Motor==NULL || MOT[MInArmY].Motor==NULL)
        return false;
    AnsiString str=AnsiString("");
    AnsiString str1=AnsiString(""), str2=AnsiString("");
    int iCheckX=0, iCheckY=0, iEncodeX=0, iEncodeY=0;
    int iXPosdiffer=0, iYPosdiffer=0, iXpos=0, iXMSPos=0, iYpos=0, iYMSPos=0;
    int ret=PNP_DOING;
    int iX=X, iY=Y;
    bool bRet;
    static bool bFirstIn=true;
    static bool bFlag[8];
    static int  iInArmAllZSafe=1;
    static int  iPitchMoveOk=0;
    static bool bXYMoveFinish=false;
    static bool bXYPosErrorFlag=false;

    static int iLastPosX=-99999;
    static int iLastPosY=-99999;
    if(iLastPosX!=X || iLastPosY!=Y)
    {
        iLastPosX=X;
        iLastPosY=Y;
        iInArmAllZSafe=1;
        ZeroMemory(bFlag, sizeof(bFlag));
        if(MOT[MInArmX].Motor->MotionDone()==false)
            MOT[MInArmX].PCIL132_StopMotor();
        if(MOT[MInArmY].Motor->MotionDone()==false)
            MOT[MInArmY].PCIL132_StopMotor();
    }

    if(bFirstIn)
    {
        bFirstIn=false;
        ZeroMemory(bFlag, sizeof(bFlag));
        InArmPitchMove(Vari, YVari, true);
    }

    //KaiChen 20171228 ：Log Light Scale Data
    //==>
    int iXO=0, iYO=0, iLogCount=500;
    iXO=iX;
    iYO=iY;
    AnsiString S1="";

    if(IniConfig.bA27EnableLightScale==true)
    {
        iCheckX=MOT[MInArmX].ReadPos();
        iCheckY=MOT[MInArmY].ReadPos();

        if(iX>iCheckX)
        {
            iX=ChangePosition(iX, 0);
            if(IniConfig.bA27_1LogEnableLightScaleData==true && iLogLightScaleCount_InArmX1<iLogCount && bLogLightScale_InArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iXO, iX, iXO-iX);
                if(fMotorTest!=0) fMotorTest->mmo1->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_InArmX1++;
            }
        }
        else
        {
            iX=ChangePosition(iX, 1);
            if(IniConfig.bA27_1LogEnableLightScaleData==true && iLogLightScaleCount_InArmX2<iLogCount && bLogLightScale_InArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iXO, iX, iXO-iX);
                if(fMotorTest!=0) fMotorTest->mmo2->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_InArmX2++;
            }
        }

        if(iY>iCheckY)
        {
            iY=ChangePosition(iY, 2);
            if(IniConfig.bA27_1LogEnableLightScaleData==true && iLogLightScaleCount_InArmY1<iLogCount && bLogLightScale_InArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iYO, iY, iYO-iY);
                if(fMotorTest!=0) fMotorTest->mmo3->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_InArmY1++;
            }
        }
        else
        {
            iY=ChangePosition(iY, 3);
            if(IniConfig.bA27_1LogEnableLightScaleData==true && iLogLightScaleCount_InArmY2<iLogCount && bLogLightScale_InArm==false)
            {
                S1.sprintf("Old, %d, New, %d, [ %d ]", iYO, iY, iYO-iY);
                if(fMotorTest!=0) fMotorTest->mmo4->Lines->Add(S1);   //AI(W906-B20-NULLFORM) 20261001: INBOX 134 -- nothing in this tree creates fMotorTest (forms/fMotorTest.cpp:99); golden logs into the Motor Test page memo, which the web page reads the same way (WebMotorAccessLive.cpp:956) -- no form, no log, no crash
                iLogLightScaleCount_InArmY2++;
            }
        }

        bLogLightScale_InArm=true;
    }

    if(iX>=MOT[MInArmX].Motor->PSoftLimitP || iX<=MOT[MInArmX].Motor->PSoftLimitN)
    {
        str.sprintf("X=%d", iX);
        ShowErrorMessage("WAR0154", 0, MInArmX, 0, str);                        //"In Arm X axis motor will out of limit !"
        return false;
    }

    if(iY>=MOT[MInArmY].Motor->PSoftLimitP || iY<=MOT[MInArmY].Motor->PSoftLimitN)
    {
        str.sprintf("Y=%d", iY);
        ShowErrorMessage("WAR0155", 0, MInArmY, 0, str);                        //"In Arm Y axis motor will out of limit !"
        return false;
    }

    //simulate en
    if(MOT[MInArmX].fCanMove==false || MOT[MInArmY].fCanMove==false)
    {
        MOT[MInArmX].PCIL132_StopMotor();
        MOT[MInArmY].PCIL132_StopMotor();
        iInArmAllZSafe=1;
        return false;
    }

    if(MOT[MInArmX].Motor->Enable && MOT[MInArmY].Motor->Enable)
    {
        if(bPauseInMotor)
        {
            iInArmAllZSafe=1;
            bPauseInMotor=false;
            InArmIdle.SetSecAndOn(iIDLETime);
            if(IniConfig.bA62bUseStopMachineArmHome==true)                      //Ztex 2024.10.30 Add Use Stop Machine In/Out Arm Need To Home
            {
                MOT[MInArmX].PCIL132_StopMotor();
                MOT[MInArmY].PCIL132_StopMotor();
                iInArmAllZSafe=1;
                iPitchMoveOk=0;
                SetInArmHome();
            }
        }

        switch(iInArmAllZSafe)
        {
            case 1:
                if(MoveInArmZToPlateSafe(5555))
                {
                    iPitchMoveOk=0;
                    InArmPitchMove(Vari, YVari, true);
                    InArmIdle.SetSecAndOn(iIDLETime);
                    if(InOutArmPickerUseMotor!=eptUseMotCyn)
                        iInArmAllZSafe=100;
                    else
                        iInArmAllZSafe=50;
                    bXYMoveFinish=false;
                    InArmCynMove(ZDownSel);
                    InArmCylinderDelayTimer.SetMSAndOn(ArmSpeed_File[InArm].dCylinderDelay*1000);
                    InitPCIL112_InArmXYMoveTask();                              //jou 2011-04-03
                    if(bXYPosErrorFlag)
                    {
                        bFlag[0]=false;
                        bFlag[1]=false;
                        iInArmAllZSafe=150;
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            case 50:
                if(InArmCylinderDelayTimer.Off())
                    iInArmAllZSafe=100;
                else
                    break;
            case 100:
                if(iPitchMoveOk==0)
                    iPitchMoveOk=InArmPitchMove(Vari, YVari);

                if(iPitchMoveOk==2)                                             //未變Pitch 則需重新動作
                {
                    iInArmAllZSafe=1;
                }
                else if(iPitchMoveOk>0)                                         //ChungHung 20131231 alter AutoYPitch
                {
                    iInArmAllZSafe=200;
                }
                else if(iPitchMoveOk<0)                                         //Alarm了
                {
                    iInArmAllZSafe=1;
                }
                break;
            case 150:
                if(bFlag[0]==false)
                    bFlag[0]=MOT[MInArmX].MotorMove(X+100);
                if(bFlag[1]==false)
                    bFlag[1]=MOT[MInArmY].MotorMove(Y+100);
                if(bFlag[0] && bFlag[1])
                {
                    bFlag[0]=false;
                    bFlag[1]=false;
                    bXYPosErrorFlag=false;
                    iInArmAllZSafe=100;
                }
                break;
            case 200:
                if(bXYMoveFinish)
                {
                    if(ZNeedDown)
                    {
                        MOT[MInArmX].ScanMotorStatus();
                        MOT[MInArmY].ScanMotorStatus();

                        if(MOT[MInArmX].Led[iInposLed]==true ||
                           MOT[MInArmY].Led[iInposLed]==true)                   //Isaac 20170721 (wei) Z axis protection
                            return false;

                        bRet=InArmZMoveDown(ZDownSel, iZPos, bLoader);
                        InArmCylinderDelayTimer.SetMSAndOn(ArmSpeed_File[InArm].dCylinderDelay*1000);
                        if(bRet==false)
                            return false;
                    }

                    iInArmAllZSafe=1;
                    InitPCIL112_InArmXYMoveTask();                              //jou 2011-04-03

                    if(CheckInArmZHomeSensor_2x8(ZNeedDown, ZDownSel)==false)
                    {
                        return false;
                    }

                    if(IniConfig.bA22MagneticScale==true)                       //Frank 20161109 add 磁性尺
                    {
                        iXpos=MOT[MInArmX].Motor->ReadPos();
                        iXMSPos=MOT[MInArmXScale].Motor->ReadPos();
                        iYpos=MOT[MInArmY].Motor->ReadPos();
                        iYMSPos=MOT[MInArmYScale].Motor->ReadPos();

                        iXPosdiffer=abs(iXpos - iXMSPos);
                        iYPosdiffer=abs(iYpos - iYMSPos);

                        if(iXPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100 &&
                           iYPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                        {
                            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                MyMessageBox->Close();

                            str1.sprintf("In Arm X & Y Magnetic Scale Discrepancy Encoder Over Setting");
                            str2.sprintf("In Arm X & Y 磁性尺與編碼器差超出設定");

                            if(iXPosdiffer>IniConfig.dA22MagneticScaleStopRunRange*100 ||
                               iYPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                            {
                                iUnLoaderCount=0;
                            }
                            else
                            {
                                iUnLoaderCount=9;
                            }
                            ShowUnloaderTrayMessage(str1, str2);
                        }
                        else if(iXPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                        {
                            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                MyMessageBox->Close();

                            str1.sprintf("In Arm X Magnetic Scale Discrepancy Encoder Over Setting");
                            str2.sprintf("In Arm X 磁性尺與編碼器差超出設定");

                            if(iXPosdiffer>IniConfig.dA22MagneticScaleStopRunRange)
                            {
                                iUnLoaderCount=0;
                            }
                            else
                            {
                                iUnLoaderCount=9;
                            }
                            ShowUnloaderTrayMessage(str1, str2);
                        }
                        else if(iYPosdiffer>IniConfig.dA22MagneticScaleKeepRunRange*100)
                        {
                            if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                MyMessageBox->Close();
                            str1.sprintf("In Arm Y Magnetic Scale Discrepancy Encoder Over Setting");
                            str2.sprintf("In Arm Y 磁性尺與編碼器差超出設定");
                            if(iYPosdiffer>IniConfig.dA22MagneticScaleStopRunRange)
                            {
                                iUnLoaderCount=0;
                            }
                            else
                            {
                                iUnLoaderCount=9;
                            }
                            ShowUnloaderTrayMessage(str1, str2);
                        }
                    }

                    if(InArmCylinderDelayTimer.Off())
                    {
                        iPitchMoveOk=0;
                        bLogLightScale_InArm=false;                             //KaiChen 20171228 ：Log Light Scale Data
                        iInArmAllZSafe=1;
                        return true;
                    }
                    else
                    {
                        iInArmAllZSafe=210;
                        break;
                    }
                }

                if(iPitchMoveOk==0)
                {
                    iInArmAllZSafe=1;
                }
                break;
            case 210:
                if(InArmCylinderDelayTimer.Off())
                {
                    iPitchMoveOk=0;
                    iInArmAllZSafe=1;
                    bLogLightScale_InArm=false;                                 //KaiChen 20171228 ：Log Light Scale Data
                    return true;
                }
                break;
            default :
                iInArmAllZSafe=1;
                return false;
        }

        if(bXYPosErrorFlag)
            return false;

        if(bXYMoveFinish==false &&                                              //Steven 20231214 : 避免重複進去
           iPitchMoveOk>=0)                                                     //避免Pitch Alarm時, XY還在動
        {
            ret=PCIL112_InArmXYMove(X, Y);
        }

        if(ret==PNP_DONE)
        {
            if(iPitchMoveOk>0)                                                  //Steven 20150718 : Fixed for Y-Pitch
            {
                if(USE_MAGNETIC_SCALE==false)                                   //Ifor 20180622 (Steven) : 避免位置錯誤
                {
                    iCheckX=MOT[MInArmX].ReadPos();
                    iCheckY=MOT[MInArmY].ReadPos();

                    if((X!=iCheckX) || (Y!=iCheckY))
                    {
                        iEncodeX=MOT[MInArmX].ReadEncoderPos();                 //Steven 20110314 : 比對Encoder位置
                        iEncodeY=MOT[MInArmY].ReadEncoderPos();                 //Steven 20110314 : 比對Encoder位置

                        str1.sprintf(" Move Finish:%d ",bXYMoveFinish);         //Ifor 20181127 : add  bXYMoveFinish Log
                        MNetLog(" iX="+AnsiString(iX)+" iCheckX="+AnsiString(iCheckX)+" iEncodeX="+AnsiString(iEncodeX)+
                                " iY="+AnsiString(iY)+" iCheckY="+AnsiString(iCheckY)+" iEncodeY="+AnsiString(iEncodeY)+ " In Arm Pos Error" + str1);   //Steven 20110402 //Ifor 20181127 : add  bXYMoveFinish Log
                        if(X>(iEncodeX+iTorence) || X<(iEncodeX-iTorence) ||
                           Y>(iEncodeY+iTorence) || Y<(iEncodeY-iTorence))      //Steven 20110314 : 比對Encoder位置
                        {
                            iInArmAllZSafe=1;
                            iPitchMoveOk=0;
                            MOT[MInArmX].PCIL132_StopMotor();
                            MOT[MInArmY].PCIL132_StopMotor();
                            bXYPosErrorFlag=true;
                            bXYMoveFinish=false;                                //Ifor 20181127 : add Encoder位置比對異常時，設定bXYMoveFinish為False避免誤動作
                            return false;
                        }
                    }
                }

                bXYMoveFinish=true;

                if(ZNeedDown)
                {
                    MOT[MInArmX].ScanMotorStatus();
                    MOT[MInArmY].ScanMotorStatus();

                    if(MOT[MInArmX].Led[iInposLed]==true ||
                       MOT[MInArmY].Led[iInposLed]==true)                       //Isaac 20170721 (wei) Z axis protection
                        return false;

                    bRet=InArmZMoveDown(ZDownSel, iZPos, bLoader);
                }
                return false;
            }
        }

        if(InArmIdle.Off())
        {
            MOT[MInArmX].PCIL132_StopMotor();
            MOT[MInArmY].PCIL132_StopMotor();
            iInArmAllZSafe=1;
            iPitchMoveOk=0;
            SetInArmHome();
            InArmIdle.SetSecAndOn(iIDLETime);
        }
    }
    else
    {
        //動作模擬
        if(bFlag[0]==false)
            bFlag[0]=InArmZMoveUp(ZSafePos, bLoader);

        if(bFlag[0])
        {
            if(bFlag[1]==false)
            {
                InArmPitchMove(Vari, YVari, true);
                InitPCIL112_InArmXYMoveTask();
                bFlag[1]=true;
                return false;
            }

            if(bFlag[2]==false)
            {
                ret=PCIL112_InArmXYMove(X, Y);                                  //Steven 20240603 : [E77]改放到PCIL112_OutArmXYMove裡面
                bFlag[2]=(ret==PNP_DONE);
            }

            if(bFlag[3]==false)
                bFlag[3]=InArmPitchMove(Vari, YVari);
        }

        if(bFlag[0] && bFlag[1] && bFlag[2] && bFlag[3])
        {
            if(InOutArmPickerUseMotor==eptUseMot)                               //Steven for HT1032
                bFlag[4]=true;
            else
                bFlag[4]=InArmCynMove(ZDownSel);
        }

        if(bFlag[0] && bFlag[1] && bFlag[2] && bFlag[3] && bFlag[4])
        {
            if(ZNeedDown)
                bRet=InArmZMoveDown(ZDownSel, iZPos, bLoader);
            else
                bRet=InArmZMoveUp(ZSafePos, bLoader);

            if(bRet==false)
                return false;

            ZeroMemory(bFlag, sizeof(bFlag));
            bLogLightScale_InArm=false;                                         //KaiChen 20171228 ：Log Light Scale Data
            return true;
        }
    }
    return false;
}
//----- golden Motor/mymotor.cpp:4640-4646  InitPCIL112_InArmXYMoveTask -----
void  InitPCIL112_InArmXYMoveTask()
{
    iPCIL112_InArmXYMoveTask=1;
    MOT[MInArmX].fCMD=false;
    MOT[MInArmY].fCMD=false;
    MOT[MInArmPitch].fCMD=false;
}
//----- golden Motor/mymotor.cpp:4648-4712  PCIL112_InArmXYMove -----
int PCIL112_InArmXYMove(int iXComPos, int iYComPos)                             //jou 2011-04-03
{
    static int iCount=0;                                                        //Ifor 20220715 add:同一次移動中發生三次異常報警
    static int iXPos=0, iYPos=0;
    static int iInArmFlag[2]={0, 0};

    AnsiString StrE, StrC;
    int &iTask=iPCIL112_InArmXYMoveTask;

    if(MOT[MInArmX].CompareEncoderPos(iXComPos, 2)==1 &&                        //Steven 20231214 : CompareCommandPos --> CompareEncoderPos
       MOT[MInArmY].CompareEncoderPos(iYComPos, 2)==1)
    {
        return PNP_DONE;
    }

    int iPos=InArmZSafe(DETECT_SENSOR_FLAG);
    if(iPos!=-1)                                                                //jou 20220113 : 增加in & out arm移動保護
    {
        StrE.sprintf("In arm XY move, %s axis home sensor off alarm", MOT[iPos].NumberAlias);
        StrC.sprintf("In arm XY移動, %s軸home sensor異常警報", MOT[iPos].NumberAlias);
        if(USE_ARM_PROTECTION==true && CUSTOMER_CODE!=CC_ASE_KaohSiung)         //kevin 20220901 ASE_KH close          //Steven 20220314 : In Our Arm Z Sensor保護加上開關
        {
            ShowMyMessage(StrE, StrC);
            return PNP_DOING;
        }
        else
        {
            if(iXPos!=iXComPos || iYPos!=iYComPos)
            {
                iXPos=iXComPos;
                iYPos=iYComPos;

#if 0 // GATE(B1-1) AI(W906-B1-INARM) 20260924 -- golden :4680 的 MyDBIProcess 是 3 參數（golden cMyDB.h）；本 TU 看得到的是 2 參數（aHotPlateSubstrate.h:979）。見檔尾家族橫幅。
                MyDBIProcess("Motion", StrE, "");
#else
                MyDBIProcess("Motion", StrE);                                   // GATE(B1-1): golden 第 3 參數是 ""，2 參數呼叫帶的內容與 golden 相同 —— 同 asortarm.cpp:2479 / ainarm2.cpp:4669 的處置
#endif
                iCount++;
                if(iCount>=3)
                {
                    iCount=0;
                    ShowMyMessage(StrE, StrC);
                }
                return PNP_DOING;                                               //Steven 20240424 : move down
            }
        }
    }

    switch(iTask)
    {
        case 1:
            iInArmFlag[0]=0;
            iInArmFlag[1]=0;
            iCount=0;
            iTask=100;
        case 100:
            if(iInArmFlag[0]==0)
                iInArmFlag[0]=MOT[MInArmX].MotorMove(iXComPos);
            if(iInArmFlag[1]==0)
                iInArmFlag[1]=MOT[MInArmY].MotorMove(iYComPos);
            if(iInArmFlag[0]>0 && iInArmFlag[1]>0)
            {
                iCount=0;
                return PNP_DONE;
            }
            break;
    }
    return PNP_DOING;
}

//==============================================================================
//  AI(W906-C21-MOTORMOVE) 20260924
//
//  ## 為什麼（使用者 20260924：「空殼就必須解決」；V906 要上真機）
//
//  本檔原本 :721 / :851 / :874 的 MotorMovePosition / MotorMove / MotorMove2SpeedForPicker
//  是 W4 樁：Motor==NULL 或 !Enable 時把 Position 設成目標、回成功（W7 OFFLINE 捷徑，
//  模擬組態靠它跑）；**馬達 Enable 時直接 `return 0;` / `return false;`**。
//  ⇒ 真機（出貨組態 -DW906_NO_SOFT_SIMULTE=ON、Mot_Table Enable=1）上，經 MotorMove 的
//    每一軸都一步不動、每一拍回「移動中」。非測試 .cpp 裡 `.MotorMove(` 出現 320 次（計數含註解與 #if 0）。
//
//  ## 翻了什麼（本體逐字照 golden，cp950 -> UTF-8，由腳本直接從 golden 位元組轉出）
//
//      MotorMovePosition          golden :539-860   （含 :540-547 回傳碼說明）
//      MotorMove                  golden :861-962   （含 :862-869 回傳碼說明）
//      MotorMove2SpeedForPicker   golden :963-1000
//
//  本檔 :393-705 / :753-845 的 #if 0 golden 抄本保留作對照；原本的三支樁在原位改成退休註解。
//  本體放尾端：理由同 P0-6 / P0-7 / B1 —— ShowMyMessage / ShowErrorMessage（canary_support.h）、
//  Cylinder[]（mycylin.h）、SetInArmHome（aHotPlateSubstrate.h）在本檔 :2992-2997 才 include，
//  SetOutArmHome 的 extern 在 :3012。
//
//  ## 與 golden 的差異 —— 只有兩處，本體內都標了 AI(W906-C21-MOTORMOVE) 20260924
//
//  (1) 〈!Enable 的終端〉MotorMovePosition 的 SOFT_SIMULTE 臂（golden :824-858）。
//      golden 在 !Enable 時跳過整個硬體區塊（`if(Motor!=NULL && Motor->Enable)`），落到：
//        #ifndef SOFT_SIMULTE   Position=Tar; return 1;             <- 出貨組態：一次到位
//        #else                  每呼叫一次只走 `speed` 個 count     <- 模擬組態：逐拍步進
//      步進在移植樹不收斂：!Enable 時 SetSpeed 把百分比原樣存進 speed（本檔 :351 / golden :510，
//      典型 50~100；speed<=0 時 golden 用 100）；移植樹一拍 500 ms（RULINGS_20260917 B13），
//      golden 的 MainProc 約 1 ms 一拍。B1 實測的 InArm Y 一趟 -10000 -> -48898 = 38,898 count，
//      speed=100 要 389 拍 = 194.5 秒；而 ProcessMotorHome case 1310 約 30 秒就重來 ——
//      AI(W906-SIMSTEP)（Motor/myGALILmotor.cpp 檔尾）用 gdb 量過同一個演算法「要走幾十分鐘」。
//      ⇒ 依任務規則：保留捷徑，**只限 !Enable**。模擬臂改成與出貨臂同一個終端
//        （fCMD=false; Position=Tar; return 1;），golden 步進原文以 #if 0 保存在原位。
//        另一條路是 SIMSTEP 的 ×500 補償 —— 那要使用者裁決，本波沒走。
//      ⚠ 與舊捷徑的差別（刻意的：這就是 golden 的路徑，只有終端換掉）：
//        - 舊捷徑在 MotorMove **第一行**就回 1，不看安全門也不看互鎖。現在 !Enable 軸先走完
//          golden MotorMove 的前段：安全門（-1）、iOldPos／fCMD／floodgate 旗標重置、
//          fCanMove/R/M/L 與 mapLockList 互鎖（PCIL132_StopMotor ＋ 位置比對，未到位回 0），
//          然後進 MotorMovePosition（再問一次安全門）才落到終端。
//          模擬組態的軸全是 Enable=false（cinitial.cpp InitialMotorParameter 的 #ifdef SOFT_SIMULTE），
//          而安全門回呼只掛給 Enable 的軸（cinitial.cpp:4555），所以 HTMotor::CheckIsSafeDoorOpen()
//          回 `Enable==true` = false —— 模擬時安全門不擋；互鎖則照 golden 生效。
//        - Motor==NULL 回 **-1**（golden :876-880）；舊捷徑回 1。golden 的 InitialMotorParameter
//          保證開機後非 NULL（本樹 wb_serve.cpp:2861 也跑它），NULL 只剩「沒 bring-up 的測試行程」。
//        - ScreenPos 只在 bShowMotorMove 時更新（golden :935-951）；舊捷徑每次都算。
//          MOT[].ScreenPos 在 Motor/ 之外沒有讀者（git grep '\.ScreenPos'）。
//
//  (2) GATE(C21-1) MotorMove 的面板區塊（golden :940-949，PWinCtrl->Top / ->Left）。
//      本樹 PWinCtrl 是 void*（mymotor.h:106；golden 是 TWinControl*）。相依不存在用編譯器判：
//      把 golden 原文放開，g++ -fsyntax-only 報 "'void*' is not a pointer-to-object type"。
//      TWinControl 在本樹有三份互撞的定義（forms/fIoSetView.h:103 的說明），不引入。
//      行為影響 0：bPanelUse 只有 SetPanel() 會設 true，ctor（本檔 :165）設 false，
//      而 SetPanel() 全樹 0 個呼叫端（git grep 'SetPanel('）。
//
//  ## golden 怪處（照翻，不修；改行為要使用者決定）
//
//   (a) MotorMove2SpeedForPicker 回 bool 卻 `return iFlag;`（int，golden :999）：MotorMove 的
//       -1..-6 全部變成 true。安全門開（-1）時 InArmZMoveDown / OutArmZMoveDown 會當成「Z 已到位」。
//       golden :967 jou 20170811 把 iFlag 從 bool 改成 int，回傳型別沒跟著改。
//   (b) MotorMove 的呼叫端很多寫 `if(MOT[x].MotorMove(p))`（真值判斷），-1..-6 同樣被當成到位。
//   (c) 二段速 `FinalPos+ARM->dTwoSpeedDistance*100` 是 double，傳進 MotorMove(int) 時截斷（golden 同）。
//   (d) MotorMovePosition 的第一個安全門檢查不判 Motor==NULL（golden :554）。本體只經 MotorMove
//       （已判 NULL）進來；MotorMoveShuttleShake（AI(W906-AMB-L2) 20260929 起是 golden 本體，本檔 :910）第一行也先判 Motor==NULL 才呼叫它。
//   (e) Enable 軸沒掛安全門回呼時，CheckIsSafeDoorOpen() 回 true（HTMotor.cpp:122，golden 同）——
//       MotorMove 回 -1。真機由 InitialMotorParameter 掛上 IdleCheckSafeDoor（cinitial.cpp:4555）。
//
//  ## 本體用到、但仍是樁或與 golden 不同的東西（本波不動，列給主迴圈）
//    - ScanMotorStatus()（本檔 :1230-1246）：Galil 分支（INDEX_MOTION_CARD==0 的 MTestY1..MTestY2）
//      `Gali_ScanMotStatus();` 仍是註解（golden :1789 有呼叫；真本體已在 myGALILmotor.cpp:1604）。
//      MotorMovePosition 只在 PServoAlarmOn 時呼叫它；Index 軸平常走 Gali_* 不走 MotorMove。
//    - （不是樁，列出來是因為本體在別的 TU）SetInArmHome() / SetOutArmHome()：nm 量到 T 在
//      libht9045_sm.a 的 ainarm2.cpp.obj / aoutarm.cpp.obj。只在 home sensor error 分支呼叫。
//    - SetSpeed()（本檔 :320-353）：(H-L)*(int)p/100 用整數算；golden :497/:501 用 double 再截斷。
//      p 是整數且非負時兩者相同（MotorMove2SpeedForPicker 傳的 iZSP / iTwoSpeed 都是 int）。
//    - Lock()（本檔 :1501）：key 用 MotorAlias、覆寫；golden :5531 用 "alias_func"、不覆寫。
//      對 MotorMove 的 `mapLockList.size()!=0` 等價：本樹只有 Lock 與 ClearLock 兩種寫入者，
//      UnLock 全樹 0 個呼叫端（git grep '\.UnLock('）。
//    - （已解 —— AI(W906-AMB-L2) 20260929）MotorMoveShuttleShake()：原本 :1028 的 W4 樁已原地退休（:1033-1044），
//      golden :5198-5316 抄本（本檔 :909-1031）解閘成本體，經本檔尾端的 MotorMovePosition 移動。
//==============================================================================
//----- golden Motor/mymotor.cpp:539-860  MotorMovePosition -----  //AI(W906-C21-MOTORMOVE) 20260924
//------------------------------------------------------------------------------
// 1 : Move Success
// 0 : Moving
//-1 : Safe Door Opened
//-2 : Target > Limit P
//-3 : Target < Limit N
//-4 : PServoAlarmOn
//-5 : WAR1639 Motor encoder error, check encoder cable
//-6 : MOT Home sensor error!!
//------------------------------------------------------------------------------
int TMyMotor::MotorMovePosition(int &Position, int speed, int Tar)
{
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return -1;
    }

    int ret=0;
    int Pos=Tar;
    int iEncoder=0;
    AnsiString S1="", S2="", S3="";
    int iGap=2;                                                                 //Sam 20230621 : Gap容許誤差改為1>2 //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條

    if(Motor!=NULL &&                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->Enable)                                                           // has true motor
    {
        if(Tar>=Motor->PSoftLimitP)
        {
            S1=AnsiString("The target position of ")+Alias+AnsiString(" over positive soft limit !");
            S2=Alias+AnsiString("的目標位置超過正向軟體極限!");
            S3.sprintf("%d > %d", Tar, Motor->PSoftLimitP);
            ShowMyMessage(S1, S2, S3);
            return -2;
        }

        if(Tar<=Motor->PSoftLimitN)
        {
            S1=AnsiString("The target position of ")+Alias+AnsiString(" below negative soft limit !");
            S2=Alias+AnsiString("的目標位置低於負向軟體極限!");
            S3.sprintf("%d <= %d", Tar, Motor->PSoftLimitN);
            ShowMyMessage(S1, S2, S3);
            return -3;
        }

        Position=ReadPos();                                                     //calculate GearRate Pos

        if(Position!=Tar)                                                       //RogerYang 20260113 : 用來紀錄Rotator最近一次旋轉方向
        {
            iLastRotatorDirP=(Tar>Position)?true:false;
        }

        if(fCMD==false && Motor->MotionDone()==false)
        {
            return 0;
        }
        else if(fCMD==false)
        {
            GetRealPos(&Pos);                                                   //pos will change to gear ration value
            #ifndef USE_CompareCommandPos
            if(Tar==Position)
            #else
            if((Motor->GearRatio>2 && Motor->GearRatio<=5)  &&
                ((Mot_Name>=MInArmZA && Mot_Name<=MInArmZH) ||
                 (Mot_Name>=MOutArmZA && Mot_Name<=MOutArmZH)))                 //Jimmychiu 20230216 : 吸嘴高度
            {
                iGap=5;
            }

            if(CompareCommandPos(Tar, iGap)==1)                                 //Isaac 20201217 : 若齒輪比大於1，換算有機會和目標位置差1條
            #endif
            {
                if(Motor->PServoAlarmOn)
                {
                    ScanMotorStatus();
                    if(Led[iInposLed]==true)
                    {
                        return 0;
                    }
                    else
                    {
                        InitMOTParameter();
                        return 1;                                               // -4 --> 1
                    }
                }
                else
                {
                    InitMOTParameter();
                    return 1;
                }
            }
//            if(Motor->Direction==false)
//                Pos=-Pos;

            if(Mot_Name==MInArmY || Mot_Name==MOutArmY)                         //Steven 20181129 : 短距離的移動把加減速縮短一半
            {
                if(AUTO_EMPTY_COLOR>=3)
                {
                    if(Mot_Name==MOutArmY && (abs(Tar-Position)<6500))
                        ret=Motor->MoveToPosShortDisSlowSP(Pos);
                    else
                        ret=Motor->MoveToPos(Pos);
                }
                else
                {
                    if(abs(Tar-Position)<6500)
                        ret=Motor->MoveToPosShortDistance(Pos);
                    else
                        ret=Motor->MoveToPos(Pos);
                }
            }
            else if(Mot_Name==MInShuttle1 || Mot_Name==MInShuttle2)
            {
                if(SHUTTLE_FLOODGATE==1)                                        //Ifor 20260327 add:避免Shuttle 移動時閘門未完全開啟導致撞機
                {
                    if(Mot_Name==MInShuttle1)
                    {
                        Cylinder[C_Shuttle1Floodgate].Off();
                        Cylinder[C_OutShuttle1Floodgate].Off();
                        if(Cylinder[C_Shuttle1Floodgate].OffSensor()==false ||
                           Cylinder[C_OutShuttle1Floodgate].OffSensor()==false)
                        {                                                       //Ifor 20260401 add: Floodgate open timeout
                            if(!bSh1FloodgateOpenWaiting)
                            {
                                tSh1FloodgateOpenTimeout.SetSecAndOn(3);
                                bSh1FloodgateOpenWaiting=true;
                            }
                            else if(tSh1FloodgateOpenTimeout.Off())
                            {
                                tSh1FloodgateOpenTimeout.SetSecAndOn(3);           //Ifor 20260401 add: re-arm for next warning
                                ShowMyMessage("Shuttle Floodgate open timeout! Check cylinder sensor.");
                            }
                            return -1;
                        }
                        bSh1FloodgateOpenWaiting=false;                            //Ifor 20260401 add: reset on success
                    }
                    else
                    {
                        Cylinder[C_Shuttle2Floodgate].Off();
                        Cylinder[C_OutShuttle2Floodgate].Off();
                        if(Cylinder[C_Shuttle2Floodgate].OffSensor()==false ||
                           Cylinder[C_OutShuttle2Floodgate].OffSensor()==false)
                        {                                                       //Ifor 20260401 add: Floodgate open timeout
                            if(!bSh2FloodgateOpenWaiting)
                            {
                                tSh2FloodgateOpenTimeout.SetSecAndOn(3);
                                bSh2FloodgateOpenWaiting=true;
                            }
                            else if(tSh2FloodgateOpenTimeout.Off())
                            {
                                tSh2FloodgateOpenTimeout.SetSecAndOn(3);           //Ifor 20260401 add: re-arm for next warning
                                ShowMyMessage("Shuttle Floodgate open timeout! Check cylinder sensor.");
                            }
                            return -1;
                        }
                        bSh2FloodgateOpenWaiting=false;                            //Ifor 20260401 add: reset on success
                    }
                }

                if(abs(Tar-Position)<5000)
                    ret=Motor->MoveToPosShortDistance(Pos);
                else
                    ret=Motor->MoveToPos(Pos);
            }
            else
            {
                ret=Motor->MoveToPos(Pos);
            }

            if(ret==0)
            {
            }

            fCMD=true;
        }
        else
        {
            if(Motor->MotionDone())
            {
                Position=Tar;
                if(Motor->PServoAlarmOn)
                {
                    ScanMotorStatus();
                    if(Led[iInposLed]==true)
                    {
                        return 0;
                    }
                    else
                    {
                        InitMOTParameter();
                        iEncoderCheckCT++;
                        if(bCheckEncoderEveryTime==true || iEncoderCheckCT>100)
                        {
                            iEncoderCheckCT=0;
                            iEncoder=ReadEncoderPos();
                            if((Tar-iEncoderTorence)>iEncoder ||                //JerryYang 20180706 : 修改Encoder到位容許範圍
                               (Tar+iEncoderTorence)<iEncoder)
                            {
                                S1.sprintf("MOT=%s, Tar=%d, Encoder=%d", Alias, Tar, iEncoder);
                                ShowErrorMessage("WAR1639", 0, MMSystem, 0, S1);   //Motor encoder error, check encoder cable
                                return -5;
                            }

                            if(Motor->PServoAlarmOn)
                            {
                                if(Mot_Name==MInRotateKit ||                    //Sam 20190811 : 防止 Rotate 旋轉完後剛好位置剛好落在原點上面導致誤報警。
                                   Mot_Name==MOutRotateKit)
                                {
                                    return 1;
                                }

                                if(iEncoder>2000 || iEncoder<-2000)
                                {
                                    ScanMotorStatus();
                                    if((Mot_Name==MInArmY ||                    //JerryYang 20191210 fix auto clean時後排到shuttle row A跳出home sensor error
                                        Mot_Name==MOutArmY) &&
                                       iEncoder>2000)
                                    {
                                    }
                                    else
                                    {
                                        if(Led[1]==true)
                                        {
                                            S1.sprintf("MOT=%s Home sensor error!!", Alias);
                                            ShowMyMessage(S1, "");
                                            return -6;
                                        }
                                    }
                                }
                            }

                            return 1;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }
                else
                {
                    InitMOTParameter();

                    //AI(ht9045-v899) 20260505: MLoaderY 工作位置 iMLoaderYCarPos=-505 會誤觸發此防護，僅對 MLoaderY 豁免，其餘馬達維持原有 home sensor 防護
                    if(Position<=-500 && Mot_Name!=MLoaderY)                                          //kevin 20140121 Z軸 home sensor 損壞
                    {
                        if(Mot_Name==MInRotateKit ||                            //Sam 20190811 : 防止 Rotate 旋轉完後剛好位置剛好落在原點上面導致誤報警。
                           Mot_Name==MOutRotateKit)
                        {
                            return 1;
                        }
                        ScanMotorStatus();
                        if(Led[1]==true)
                        {
                            S1.sprintf("MOT=%s Home sensor error!!", Alias);
                            ShowMyMessage(S1, "");

                            if(Mot_Name==MInArmPitch   || Mot_Name==MInArmPitchX2 ||
                               Mot_Name==MInArmPitchX3 || Mot_Name==MInArmPitchX4 ||
                               Mot_Name==MInArmPitchY)
                            {
                                SetInArmHome();
                            }

                            if(Mot_Name==MOutArmPitch   || Mot_Name==MOutArmPitchX2 ||
                               Mot_Name==MOutArmPitchX3 || Mot_Name==MOutArmPitchX4 ||
                               Mot_Name==MOutArmPitchY)
                            {
                                SetOutArmHome();
                            }
                            return -6;
                        }
                    }
                    return 1;
                }
            }
        }
        return 0;
    }

    #ifndef SOFT_SIMULTE
        Position=Tar;
        return 1;
    #else
        //AI(W906-C21-MOTORMOVE) 20260924: 模擬臂改用與出貨臂（上面 golden :821-823）同一個終端 —— 見檔尾橫幅〈!Enable 的終端〉。
        //  golden 這裡每呼叫一次只走 speed 個 count（原文在下面 #if 0）；移植樹一拍 500 ms，
        //  InArm Y 一趟 38,898 count 以 speed=100 要 389 拍 = 194.5 秒 ⇒ 歸零／B1 永遠等不到。
        //  fCMD=false 是 golden 步進到位時做的事（:841 / :852），一併保留。
        fCMD=false;                                                             //AI(W906-C21-MOTORMOVE) 20260924
        Position=Tar;                                                           //AI(W906-C21-MOTORMOVE) 20260924
        return 1;                                                               //AI(W906-C21-MOTORMOVE) 20260924
    #if 0 // AI(W906-C21-MOTORMOVE) 20260924: golden :825-858 VERBATIM（SOFT_SIMULTE 逐拍步進；500 ms tick 下不收斂，見上）
        if(Position==Tar)
        {
            fCMD=false;
            return 1;
        }
        else
        {
            if(speed<=0)                                                        //Steven 20210730 : 修正軟體模擬的最小速度
                speed=100;

            if(Position>Tar)
            {
                fCMD=true;
                Position-=speed;
                if(Position<=Tar)
                {
                    fCMD=false;
                    Position=Tar;
                    return 1;
                }
            }
            else
            {
                fCMD=true;
                Position+=speed;
                if(Position>=Tar)
                {
                    fCMD=false;
                    Position=Tar;
                    return 1;
                }
            }
        }
        return 0;
    #endif // AI(W906-C21-MOTORMOVE) 20260924: golden :825-858
    #endif
}
//----- golden Motor/mymotor.cpp:861-962  MotorMove -----  //AI(W906-C21-MOTORMOVE) 20260924
//------------------------------------------------------------------------------
// 1 : Move Success
// 0 : Moving
//-1 : Safe Door Opened
//-2 : Target > Limit P
//-3 : Target < Limit N
//-4 : PServoAlarmOn
//-5 : WAR1639 Motor encoder error, check encoder cable
//-6 : MOT Home sensor error!!
//------------------------------------------------------------------------------
int TMyMotor::MotorMove(int p)
{
    //******************************************************************************
    //  注意!! CheckIsSafeDoorOpen為Handler 安全門相關, 修改時要小心!!
    //******************************************************************************
    if(Motor==NULL ||                                                           //JimmyChiu 20250306 : 避免還沒初始化就操作馬達
       Motor->CheckIsSafeDoorOpen())                                            //Jimmychiu 20221013 safedoor判斷整合Function
    {
        return -1;
    }

    //jou 2010-12-23 保護兩次命令會造成撞機
    if(p!=iOldPos)
    {
        fCMD=false;
        iOldPos=p;
        bSh1FloodgateOpenWaiting=false;                                         //Ifor 20260401 add: reset timeout when target changes
        bSh2FloodgateOpenWaiting=false;                                         //Ifor 20260401 add: reset timeout when target changes
    }

    int ret=0;
    int iCommandPos;
    int iGap=1;

    if(fCanMove ==false  ||
       fCanMoveR==false  ||
       fCanMoveM==false  ||
       fCanMoveL==false  ||
       mapLockList.size()!=0)  //Klutter 20210817 加入鎖馬達機制                //Steven 20210825 : 吹氣完成才可以歸零
    {
        PCIL132_StopMotor();

        if((Mot_Name==MInShuttle1 || Mot_Name==MInShuttle2) && (Motor->GearRatio>1))
        {
            iCommandPos=ReadPos();
            if(iCommandPos>p+iGap)      //超出+
            {
                return -2;
            }
            else if(iCommandPos<p-iGap)
            {
                return -3;             //低於-
            }
            else
            {
                fCMD=false;
                return 1;              //正常等於
            }
        }
        else
        {
            if(p==ReadPos())
            {
                fCMD=false;
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    ret=MotorMovePosition(Position, speed, p);
    if(bShowMotorMove==true)
    {
        ScreenPos=(int)(Scale*(Position-FactStart))+RefStart;
        if(bPanelUse)
        {
#if 0 // GATE(C21-1) AI(W906-C21-MOTORMOVE) 20260924: PWinCtrl 在本樹是 void*（mymotor.h:106，golden TWinControl*）；g++ -fsyntax-only 報 'void*' is not a pointer-to-object type。bPanelUse 恆 false（SetPanel 0 個呼叫端）。見檔尾橫幅 (2)。
            if(bUpDownMove)
            {
                if(abs(PWinCtrl->Top-ScreenPos)>2)
                    PWinCtrl->Top=ScreenPos;
            }
            else
            {
                if(abs(PWinCtrl->Left-ScreenPos)>2)
                    PWinCtrl->Left=ScreenPos;
            }
#endif // GATE(C21-1)
        }
    }

    if(ret==1)
    {
        fCMD=false;
        return 1;
    }
    else
    {
        return ret;
    }
}
//----- golden Motor/mymotor.cpp:963-1000  MotorMove2SpeedForPicker -----  //AI(W906-C21-MOTORMOVE) 20260924
//---------------------------------------------------------------------------
bool TMyMotor::MotorMove2SpeedForPicker(int FinalPos, ARM_CONDITION *ARM, bool bIsLoader)     //Steven 20140217 : 兩段速移動  //JerryYang 20190729 二段速功能可選擇only at loader
{
    int iCurrPos;
    int iFlag=0;                                                                //jou 20170811 (Steven) int -> bool int Flag=false;

    if(ARM->iTwoSpeedMove==0 || (ARM->iTwoSpeedMove==1 && ARM->bTwoSpeedOnlyLoader==true && bIsLoader==false) || FinalPos>-100)  //JerryYang 20190729 二段速功能可選擇only at loader
    {
        iFlag=MotorMove(FinalPos);
    }
    else
    {
        iCurrPos=ReadPos();
        if(FinalPos==iCurrPos)                                                  //到位了
        {
            SetADCRate(ARM->iACDCZSP);
            SetSpeed(ARM->iZSP);
            iFlag=1;
        }
        else
        {
            if(iCurrPos>=FinalPos+ARM->dTwoSpeedDistance*100)                   //第一段
            {
                SetADCRate(ARM->iACDCZSP);
                SetSpeed(ARM->iZSP);
                iFlag=MotorMove(FinalPos+ARM->dTwoSpeedDistance*100);
            }

            if(iFlag==1 || iCurrPos<=FinalPos+ARM->dTwoSpeedDistance*100)       //第二段
            {
                SetADCRate(ARM->iTwoADC);
                SetSpeed(ARM->iTwoSpeed);
                iFlag=MotorMove(FinalPos);
            }
        }
    }
    return iFlag;
}

// ===========================================================================
//  AI(W906-ZSAFE) 20260927: golden Z 軸互鎖一族照翻（golden Motor/mymotor.cpp:2127-2365、:6336-6358）—— NB2 R105 ①
//  以前是一行樁：InArmZSafe／OutArmZSafe／SortArmZSafe 一律回 -1（＝「所有 Z 都在原點、位置不為負」＝安全），
//  CheckInArmZNeedHome／CheckOutArmZNeedHome 一律回 0（＝「0 號馬達要回原點」），sIn/Out/SortArmZHomeState 回空字串。
//  後果：飛梭／Sort 飛梭／出料臂／START 前的 Z 檢查永遠放行（acarry.cpp:4170 Do_Auto_SHT1 等，入料臂 Z 還在下面時飛梭照樣動）；
//  反方向：START 時 csystem.cpp:31205 每次都多做一次入料臂／出料臂回原點，SortingBinTray 的 X／Y 移動一律被擋。
//  golden 的判斷：iFlag&1 ＝馬達 Enable 時讀 home LED（ScanMotorStatus → Led[iHomeLed]），沒亮就回那顆馬達號；
//  iFlag&2 ＝ReadPos()<0 就回那顆馬達號；全部通過才回 -1。模擬組態下 Motor->Enable 關的軸第 1 種整段跳過（golden 就是這樣）。
//  相依全在（NB2 以 -fsyntax-only 探針量過缺口 0）；不加閘。
//  ⚠ 和 golden 唯一的差別：讀 MOT[iMotNo].Motor->Enable 之前先擋 Motor==NULL（8 處，同行）。移植樹的慣例（golden 自己 :759 JimmyChiu 20250306「避免還沒初始化就操作馬達」、TMyMotor::ReadPos）都擋；正式機開機就建好每一顆 MOT[].Motor，行為不變；沒建馬達物件的 ctest（InArm 一系列、AutoClean）以前靠樁不碰 MOT，照 golden 不擋會 SegFault（第一輪 gate 量到 9 支）。
// ===========================================================================
// ---- golden Motor/mymotor.cpp:2127-2365 ----
int CheckInArmZNeedHome()                                                       //ChungHung 20140605 add Fix Shuttle hit In/OutArm
{
    bool bInLedFlag=false;
    int iMotNo;

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMotNo=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
            {
                if(MOT[iMotNo].Position==Prod.ZInArmSafe[i][j])
                {
                    MOT[iMotNo].ScanMotorStatus();
                    bInLedFlag=W906_ZSAFE_ORIGIN(iMotNo, MOT[iMotNo].Led[iHomeLed]);   //AI(W906-S26-R1) 20261004: MachineType.h W906_DS402_ZSAFE_HOMEFLAG -- OFF (default) = exactly golden's lamp; ON = also a DS402-homed 1203 Z standing at command 0 (S-26 Appendix A R1)
                    if(bInLedFlag==false)
                    {
                        return iMotNo;
                    }
                }
            }
        }
    }
    return -1;
}
//------------------------------------------------------------------------------
int InArmZSafe(int iFlag)                                                       // -1 = safe
{
    bool bInLedFlag[MAX_ARM_Row][MAX_ARM_Col];
    bool bInPosFlag[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(bInLedFlag, sizeof(bInLedFlag));
    ZeroMemory(bInPosFlag, sizeof(bInPosFlag));
    int iMotNo;

    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMotNo=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            if(iFlag & 1)
            {
                if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
                {
                    MOT[iMotNo].ScanMotorStatus();
                    bInLedFlag[i][j]=W906_ZSAFE_ORIGIN(iMotNo, MOT[iMotNo].Led[iHomeLed]);   //AI(W906-S26-R1) 20261004: MachineType.h W906_DS402_ZSAFE_HOMEFLAG -- OFF (default) = exactly golden's lamp; ON = also a DS402-homed 1203 Z standing at command 0 (S-26 Appendix A R1)
                    if(bInLedFlag[i][j]==false)
                    {
                        return iMotNo;
                    }
                }
            }

            if(iFlag & 2)
            {
                if(MOT[iMotNo].ReadPos()>=0)
                {
                    bInPosFlag[i][j]=true;
                }
                else
                {
                    return iMotNo;
                }
            }
        }
    }
    return -1;
}
//------------------------------------------------------------------------------
AnsiString sInArmZHomeState()                                                   //Sam 20230707 : 新增 InOutArm Z Home前Home sensor 狀態
{
    AnsiString sRet="", s="";
    int iMotNo;
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            if(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)
                iMotNo=MInArmZA;
            else
                iMotNo=InArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
            {
                MOT[iMotNo].ScanMotorStatus();
                if(MOT[iMotNo].Led[iHomeLed]==false)
                {
                    s=InArmSuck.Suck[i][j].sName+"_Off";
                    sRet+=s+" ";
                }
            }
        }
    }
    return sRet;
}
//------------------------------------------------------------------------------
AnsiString sOutArmZHomeState()                                                  //Sam 20230707 : 新增 InOutArm Z Home前Home sensor 狀態
{
    AnsiString sRet="", s="";
    int iMotNo;
    for(int i=0; i<OutArmSuck.iMotRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMotCol; j++)
        {
            if(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)
                iMotNo=MOutArmZA;
            else
                iMotNo=OutArmSuck.Suck[i][j].iMotNo;

            if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
            {
                MOT[iMotNo].ScanMotorStatus();
                if(MOT[iMotNo].Led[iHomeLed]==false)
                {
                    s=OutArmSuck.Suck[i][j].sName+"_Off";
                    sRet+=s+" ";
                }
            }
        }
    }
    return sRet;
}
//------------------------------------------------------------------------------
//ChungHung 20140605 add Fix Shuttle hit In/OutArm
int CheckOutArmZNeedHome()
{
    bool bOutLedFlag=false;
    int iMotNo;
    for(int i=0; i<OutArmSuck.iMotRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMotCol; j++)
        {
            if(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)
                iMotNo=MOutArmZA;
            else
                iMotNo=OutArmSuck.Suck[i][j].iMotNo;

            if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
            {
                if(MOT[iMotNo].Position==Prod.ZOutArmSafe[i][j])
                {
                    MOT[iMotNo].ScanMotorStatus();
                    bOutLedFlag=W906_ZSAFE_ORIGIN(iMotNo, MOT[iMotNo].Led[iHomeLed]);   //AI(W906-S26-R1) 20261004: MachineType.h W906_DS402_ZSAFE_HOMEFLAG -- OFF (default) = exactly golden's lamp; ON = also a DS402-homed 1203 Z standing at command 0 (S-26 Appendix A R1)
                    if(bOutLedFlag==false)
                    {
                        return iMotNo;
                    }
                }
            }
        }
    }
    return -1;
}
//------------------------------------------------------------------------------
int OutArmZSafe(int iFlag)                                                      // -1 = safe
{
    bool bOutLedFlag[MAX_ARM_Row][MAX_ARM_Col];
    bool bOutPosFlag[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(bOutLedFlag, sizeof(bOutLedFlag));
    ZeroMemory(bOutPosFlag, sizeof(bOutPosFlag));
    int iMotNo;
    for(int i=0; i<OutArmSuck.iMotRow; i++)
    {
        for(int j=0; j<OutArmSuck.iMotCol; j++)
        {
            if(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)
                iMotNo=MOutArmZA;
            else
                iMotNo=OutArmSuck.Suck[i][j].iMotNo;

            if(iFlag & 1)
            {
                if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
                {
                    MOT[iMotNo].ScanMotorStatus();
                    bOutLedFlag[i][j]=W906_ZSAFE_ORIGIN(iMotNo, MOT[iMotNo].Led[iHomeLed]);   //AI(W906-S26-R1) 20261004: MachineType.h W906_DS402_ZSAFE_HOMEFLAG -- OFF (default) = exactly golden's lamp; ON = also a DS402-homed 1203 Z standing at command 0 (S-26 Appendix A R1)
                    if(bOutLedFlag[i][j]==false)
                    {
                        return iMotNo;
                    }
                }
            }

            if(iFlag & 2)
            {
                if(MOT[iMotNo].ReadPos()>=0)
                {
                    bOutPosFlag[i][j]=true;
                }
                else
                {
                    return iMotNo;
                }
            }
        }
    }
    return -1;
}
//------------------------------------------------------------------------------
int SortArmZSafe(int iFlag)                                                     //RogerYang 20250510 Add for 9046AU
{
    bool bSortLedFlag[MAX_ARM_Row][MAX_ARM_Col];
    bool bSortPosFlag[MAX_ARM_Row][MAX_ARM_Col];
    ZeroMemory(bSortLedFlag, sizeof(bSortLedFlag));
    ZeroMemory(bSortPosFlag, sizeof(bSortPosFlag));
    int iMotNo;
    for(int i=0; i<OutArm2Suck.iMotRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iMotCol; j++)
        {
            iMotNo=OutArm2Suck.Suck[i][j].iMotNo;

            if(iFlag & 1)
            {
                if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
                {
                    MOT[iMotNo].ScanMotorStatus();
                    bSortLedFlag[i][j]=MOT[iMotNo].Led[iHomeLed];
                    if(bSortLedFlag[i][j]==false)
                    {
                        return iMotNo;
                    }
                }
            }

            if(iFlag & 2)
            {
                if(MOT[iMotNo].ReadPos()>=0)
                {
                    bSortPosFlag[i][j]=true;
                }
                else
                {
                    return iMotNo;
                }
            }
        }
    }
    return -1;
}
// ---- golden Motor/mymotor.cpp:6336-6358 ----
AnsiString sSortArmZHomeState()                                                  //RogerYang 20250512 Add for 9046AU
{
    AnsiString sRet="", s="";
    int iMotNo;
    for(int i=0; i<OutArm2Suck.iMotRow; i++)
    {
        for(int j=0; j<OutArm2Suck.iMotCol; j++)
        {
            iMotNo=OutArm2Suck.Suck[i][j].iMotNo;

            if(MOT[iMotNo].Motor!=NULL && MOT[iMotNo].Motor->Enable)   /* AI(W906-ZSAFE): NULL 保護（見段頭）*/
            {
                MOT[iMotNo].ScanMotorStatus();
                if(MOT[iMotNo].Led[iHomeLed]==false)
                {
                    s=OutArm2Suck.Suck[i][j].sName+"_Off";
                    sRet+=s+" ";
                }
            }
        }
    }
    return sRet;
}

// ===========================================================================
//  AI(W906-IDXERR) 20260927: golden ShowIndexMotorError（Motor/mymotor.cpp:261-299）逐行照翻（NB2 R105 P1，46 個呼叫者）：
//  Index 四軸位置出錯時：fAllMotorHome=false、iHome=1、StopAllMotor()、讀 Y1／Y2／Z1／Z2 的命令與編碼器位置（或跟教導點比），
//  跳「Index Position error, Index 4 Axis Need home」並 SoftStop。以前是空的 ⇒ Index 位置出錯時機台照樣繼續跑（錯誤被吞掉）。
//  移植樹沒有的相依才閘（GATE(W906-IDXERR)）。
// ===========================================================================
void ShowIndexMotorError(AnsiString Debug, bool bCompareTeachPos)               //Steven 20180319 (Jou) : 增加傳入的Function名稱,方便Debug
{
    AnsiString str,str1;
//    if(CheckOutArmDestory()==0)                                               //kevin 20180119 add     //Steven 20210608 : Mark.
    {
        fAllMotorHome=false;
        iHome=1;
        StopAllMotor();
        int y1CmdPos     = MOT[MTestY1].Gali_ReadPos();
        int y1EncoderPos = MOT[MTestY1].Gali_ReadEncoderPos();
        int y2CmdPos     = (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?0:MOT[MTestY2].Gali_ReadPos();
        int y2EncoderPos = (USE_INDEX_ARM_AXES==IndexArm_3_Axis)?0:MOT[MTestY2].Gali_ReadEncoderPos();
        int z1CmdPos     = MOT[MTestZ1].Gali_ReadPos();
        int z1EncoderPos = MOT[MTestZ1].Gali_ReadEncoderPos();
        int z2CmdPos     = MOT[MTestZ2].Gali_ReadPos();
        int z2EncoderPos = MOT[MTestZ2].Gali_ReadEncoderPos();
        if(bCompareTeachPos==false)                                             // 原來，encoder 和 command 比較
        {
            str.sprintf("Y1 CMD:%d, POS:%d; Y2 CMD:%d, POS:%d, \n\rZ1 CMD:%d, POS:%d; Z2 CMD:%d, POS:%d, Function:%s",
                        y1CmdPos, y1EncoderPos,
                        y2CmdPos, y2EncoderPos,
                        z1CmdPos, z1EncoderPos,
                        z2CmdPos, z2EncoderPos,
                        Debug);
        }
        else                                                                    // encoder 和 Teaching 點比較
        {
            str.sprintf("Y1: POS:%d, Teach F:%d, M:%d;\n\rY2: POS:%d, Teach M:%d, R:%d,\n\rZ1 POS:%d; Z2 POS:%d, \n\rFunction:%s",
                        y1EncoderPos, Prod.TestY1_Front, Prod.TestY1_Middle,
                        y2EncoderPos, Prod.TestY2_Middle, Prod.TestY2_Rear,
                        z1EncoderPos,
                        z2EncoderPos,
                        Debug);
        }
        bShowIndexMotorError=true;                                              //Isaac 20201012 : encoder和command/Teaching點比較，show alarm視窗變大
        ShowMyMessage("Index Position error, Index 4 Axis Need home", str, str);
        SoftStop=true;
    }
}

//==============================================================================
//  AI(W906-AMB-L2) 20260929 -- PCIL112 OutArm XY pair, golden Motor/mymotor.cpp:4714-4720 / :4722-4821
//
//  ## Why (RULINGS_20260929 #11: ambient In/Out Arm, Shuttle, Fix/Auto, Loader/Empty/Color really wired)
//  The stubs at :2572-2573 were `{}` and `{ return 0; }`, and :78 `#define PNP_DONE 0` -- so every Out Arm
//  XY move reported "arrived" without commanding MOutArmX / MOutArmY. The only caller is
//  OutArmContinuousMove_9045 (this file: Enable branch :4386, simulation branch :4462; re-init :4214 /
//  :4276 / :4456), which git grep finds on 38 non-comment lines in 28 files (#if 0 not excluded -- a grep,
//  not a census), e.g. homing case 1540 (uhome.cpp:4098 MoveOutArmXY_ToFix_Tray_Full -> aoutarm.cpp:971; :4092 is in #if 0,
//  review 20260929). git grep: no other caller of the pair (golden also calls it from AutoAlignment.cpp:5504/:5507, not ported).
//
//  ## What: both bodies verbatim from golden (cp950 -> UTF-8, script-extracted from golden bytes).
//  Placed at the file tail for the same reason as the InArm twin (AI(W906-B1-INARM), :5402-5476):
//  ShowMyMessage (canary_support.h) and the 2-arg MyDBIProcess (aHotPlateSubstrate.h) are only
//  included at :2993-2994.
//
//  ## Dependencies re-measured (g++ -fsyntax-only, SIM and -DW906_NO_SOFT_SIMULTE; git grep for homes)
//    OutArmZSafe(int)                real body at this file's tail (AI(W906-ZSAFE), golden :2280-2323); decl mymotor.h:439
//    CompareEncoderPos / ReadEncoderPos / MotorMove   TMyMotor members (mymotor.h:258 / :269; MotorMove = C21 golden body)
//    USE_ARM_PROTECTION              cmydef.h:5205, defined cmydef.cpp:5445, read database.cpp:1632 / :1634
//    IniConfig.bE77_OutamrCMotion    Config.h:646
//    Tech.iOutArmShuttle1Y / .iOutArmAuto2X   LastSet.h:645 / :652 (extern TECH Tech, LastSet.h:1158)
//    iPCIL112_OutArmXYMoveTask       this file :102 (golden :61, same initial value 1)
//  The only textual difference from golden is GATE(B1-1) around the MyDBIProcess call (see there):
//  same content as golden, same handling as PCIL112_InArmXYMove.
//
//  ## Golden oddities kept as-is (fidelity; changing them is a user decision)
//   (1) No `CUSTOMER_CODE!=CC_ASE_KaohSiung` exemption on the Z-sensor protection (the InArm twin has it,
//       golden :4668; OutArm golden :4743 does not).
//   (2) iOutArmFlag[] is int, but case 300/400 test it with `==false` / truthiness and assign `false`; so a
//       MotorMove error code (-1..-6, truthy) counts as "done" in case 300/400, while case 100 needs `>0`.
//   (3) E77 detour: case 300 falls into 400, and 400 is the last case, so once both detour moves finish the
//       call returns PNP_DOING; the XY move itself runs on the next call through case 100.
//
//  ## Behaviour notes (measured by reading, not run)
//   - !Enable axes (SIM build): ReadEncoderPos returns Position (this file :276-288) and MotorMove lands on the
//     one-shot terminal (C21 banner (1)), so the pair converges in one call (two more with E77 on).
//   - The Z home-sensor protection (OutArmZSafe) now guards this path, as in golden.
//==============================================================================
//----- golden Motor/mymotor.cpp:4714-4720  InitPCIL112_OutArmXYMoveTask -----
void InitPCIL112_OutArmXYMoveTask()
{
    iPCIL112_OutArmXYMoveTask=1;
    MOT[MOutArmX].fCMD=false;
    MOT[MOutArmY].fCMD=false;
    MOT[MOutArmPitch].fCMD=false;
}
//----- golden Motor/mymotor.cpp:4722-4821  PCIL112_OutArmXYMove -----
extern int g_W906OutArmRule10;    //AI(W906-HT9050-RULE10) 20261004: defined after this function
static bool W906_OutArmSmallYIn(bool bOn);
int PCIL112_OutArmXYMove(int iXComPos, int iYComPos)
{
    static int iCount=0;                                                        //Ifor 20220715 add:同一次移動中發生三次異常報警
    static int iYEncoder=0;
    static int iXPos=0, iYPos=0;
    static int iOutArmFlag[2]={0, 0};

    AnsiString StrE, StrC;
    int &iTask=iPCIL112_OutArmXYMoveTask;

    if(MOT[MOutArmX].CompareEncoderPos(iXComPos, 2)==1 &&                       //Steven 20231214 : CompareCommandPos --> CompareEncoderPos
       MOT[MOutArmY].CompareEncoderPos(iYComPos, 2)==1)
    {
        return PNP_DONE;
    }

    int iPos=OutArmZSafe(DETECT_SENSOR_FLAG);
    if(iPos!=-1)
    {
        StrE.sprintf("Out arm XY move, %s axis home sensor off alarm", MOT[iPos].NumberAlias);
        StrC.sprintf("Out arm XY移動, %s軸home sensor異常警報", MOT[iPos].NumberAlias);
        if(USE_ARM_PROTECTION==true)                                            //Steven 20220314 : In Our Arm Z Sensor保護加上開關
        {
            ShowMyMessage(StrE, StrC);
            return PNP_DOING;
        }
        else
        {
            if(iXPos!=iXComPos || iYPos!=iYComPos)
            {
                iXPos=iXComPos;
                iYPos=iYComPos;
#if 0 // GATE(B1-1) AI(W906-AMB-L2) 20260929 -- golden :4754 MyDBIProcess is 3-arg (golden cMyDB.h); this TU sees only the 2-arg one (aHotPlateSubstrate.h:981). Same shape as PCIL112_InArmXYMove above.
                MyDBIProcess("Motion", StrE, "");
#else
                MyDBIProcess("Motion", StrE);                                   // GATE(B1-1): golden 3rd arg is "" -- the 2-arg overload forwards (S1, S2, "") to the golden body (aHotPlateSubstrate.cpp:1249); same as PCIL112_InArmXYMove
#endif
                iCount++;
                if(iCount>=3)
                {
                    iCount=0;
                    ShowMyMessage(StrE, StrC);
                }
                return PNP_DOING;                                               //Steven 20240424 : move down
            }
        }
    }

    switch(iTask)
    {
        case 1:
            iOutArmFlag[0]=0;
            iOutArmFlag[1]=0;
            iCount=0;
            if(IniConfig.bE77_OutamrCMotion)                                    //Steven 20240603 : [E77]改放到PCIL112_OutArmXYMove裡面
            {
                iYEncoder=MOT[MOutArmY].ReadEncoderPos();
                if((iYEncoder>(Tech.iOutArmShuttle1Y-3000) && iYComPos<(Tech.iOutArmShuttle1Y-3000)) ||
                   (iYEncoder<(Tech.iOutArmShuttle1Y-3000) && iYComPos>(Tech.iOutArmShuttle1Y-3000))) //Y 在FIX區, 要移動到AUTO區
                {
                    iTask=300;
                    return PNP_DOING;
                }
            }
            if(g_W906OutArmRule10==1)                                           //AI(W906-HT9050-RULE10) 20261004: see case 500
            {
                iTask=500;
                return PNP_DOING;
            }
            iTask=100;
        case 100:
            if(iOutArmFlag[0]==0)
                iOutArmFlag[0]=MOT[MOutArmX].MotorMove(iXComPos);
            if(iOutArmFlag[1]==0)
                iOutArmFlag[1]=MOT[MOutArmY].MotorMove(iYComPos);
            if(g_W906OutArmRule10==2)                                           //AI(W906-HT9050-RULE10-2) 20261004: EastSun「吸料的時候請Off()」-- the move to the Out Shuttle pick: C_OutArmSmallY Off with the XY move,
            {                                                                   //  and the XY is only "done" (so the nozzle goes down) once its Off sensor is on
                Cylinder[C_OutArmSmallY].Off();
                if(iOutArmFlag[0]>0 && iOutArmFlag[1]>0 && !W906_OutArmSmallYIn(false))
                    break;
            }
            if(iOutArmFlag[0]>0 && iOutArmFlag[1]>0)
            {
                iCount=0;
                return PNP_DONE;
            }
            break;
        case 300:
            if(iOutArmFlag[0]==false)
                iOutArmFlag[0]=MOT[MOutArmX].MotorMove(Tech.iOutArmAuto2X);
            if(iOutArmFlag[0])
            {
                iOutArmFlag[0]=false;
                iTask=400;
            }
            else
            {
                return PNP_DOING;
            }
        case 400:
            if(iOutArmFlag[0]==false)
                iOutArmFlag[0]=MOT[MOutArmY].MotorMove(iYComPos);
            if(iOutArmFlag[0])
            {
                iOutArmFlag[0]=false;
                iOutArmFlag[1]=false;
                iTask=100;
            }
            else
            {
                return PNP_DOING;
            }
            break;   //AI(W906-HT9050-RULE10) 20261004: golden falls out of the switch here (case 400 is last); the new case 500 below must not be entered
        //AI(W906-HT9050-RULE10) 20261004: EastSun 1004 rule 10「吸取完後Out Arm ZA到Home點後，接下來將Y軸移動至Out Arm Auto1的位置，同時將C_OutArmSmallY
        //  這個汽缸...，都到定位後再移動Out Arm X軸到Auto1的位置，然後在下降吸嘴讓IC交換到Tray上」; AI(W906-HT9050-RULE10-2) 20261004 the cylinder's
        //  direction, EastSun「要在放置Auto1的時候與Out Arm Y 一起動作，那時候C_OutArmSmallY需下On()動作」/「吸料的時候請Off()」(case 100 above).
        //  Entered from case 1 only when the caller set g_W906OutArmRule10=1 (aoutarm9045.cpp DoMoveOutArmXYToPlace_9045: HT9050 and the place
        //  tray is Auto1). The Out Arm ZA's HOME lamp is already required above on every tick (OutArmZSafe, HT9050 ORG rule). Y moves and
        //  C_OutArmSmallY goes On together; only when Y is in place AND the cylinder's On sensor is on, case 100 moves X (Y is already
        //  there). Other machines / other trays: never entered.
        case 500:
            Cylinder[C_OutArmSmallY].On();
            if(iOutArmFlag[1]==0)
                iOutArmFlag[1]=MOT[MOutArmY].MotorMove(iYComPos);
            if(iOutArmFlag[1]>0 && W906_OutArmSmallYIn(true))
            {
                iOutArmFlag[0]=0;
                iTask=100;
                return PNP_DOING;
            }
            break;
    }
    return PNP_DOING;
}
int g_W906OutArmRule10=0;   //AI(W906-HT9050-RULE10) 20261004: 1 = XY to Auto1 (Y + C_OutArmSmallY On first, then X); 2 = XY to the Out Shuttle pick (C_OutArmSmallY Off before "done"); 0 = golden. Set by aoutarm9045.cpp around one OutArmContinuousMove_9045 call
// true = C_OutArmSmallY's On (bOn) / Off sensor is on. While waiting, ~1000 calls -> one message (then keeps waiting).
static bool W906_OutArmSmallYIn(bool bOn)
{
    static int s_iWait=0;
    if(bOn ? Cylinder[C_OutArmSmallY].OnSensor() : Cylinder[C_OutArmSmallY].OffSensor()) { s_iWait=0; return true; }
    if(++s_iWait>1000)
    {
        s_iWait=0;
        if(bOn) ShowMyMessage("Out Arm: C_OutArmSmallY not on (On sensor), X can not move to Auto1", "出料臂：C_OutArmSmallY 汽缸沒有到 On（On 感測器沒亮），X 軸不能移往 Auto1");
        else    ShowMyMessage("Out Arm: C_OutArmSmallY not off (Off sensor), Z can not go down to pick", "出料臂：C_OutArmSmallY 汽缸沒有放開（Off 感測器沒亮），Z 軸不能下去吸料");
    }
    return false;
}
//------------------------------------------------------------------------------
// AI(W906-B20-SERVOON) 20261001: census 129 (e) E-BOOT-002 -- golden Motor/mymotor.cpp:4609-4621 ServoOnAllMOT, called once at boot by
//   golden TfMain::FormShow main.cpp:10165 (the port: tools/wb_serve.cpp:4163, right after GetHotPlateYHalfPos = golden :10164).
//   DEVIATION by RULINGS_20261001 #10 (Jimmy 1001 13:2x "照建議" = B): a PCIE-1203 axis is skipped -- its servo stays with EastSun's
//   Motor Power On (Motor Test). cinitial.cpp:3998 builds exactly those axes as TMyEtherCatMotor (CardModel "PCI1203"), so the class
//   is the test, as Motor/EcatMotorRoute.cpp:58 / :71 already do. HT9050's 14 enabled axes are all 1203 => this machine boots as before.
//   Port-only: a NULL MOT[i].Motor is skipped (ctest builds MOT[] without InitialMotor). Without a card the other classes' SetServoOn
//   only return an error (Motor/vendor_offline_*.cpp; MN200 logs it through MNetLog) -- no box, no wait.
#include "Motor/myEthercatmotor.h"     // AI(W906-B20-SERVOON) 20261001: TMyEtherCatMotor (the 1203 test below)
void ServoOnAllMOT()
{
    for(int i=0; i<TOTAL_MOTOR; i++)
    {
        if(i>=MTestY1 && i<=MTestY2)
            continue;
        if(MOT[i].Motor==NULL)                                                  // AI(W906-B20-SERVOON): port-only guard, see above
            continue;
        if(MOT[i].Motor->Enable==false)
            continue;
        if(dynamic_cast<TMyEtherCatMotor*>(MOT[i].Motor)!=0)                    // AI(W906-B20-SERVOON): RULINGS_20261001 #10 -- a 1203 axis keeps EastSun's Motor Power On
            continue;

        MOT[i].Motor->SetServoOn(true);                                         //servo on
        MySleep(2);
    }
}
//------------------------------------------------------------------------------
#if defined(W906_DS402_ZSAFE_HOMEFLAG)
//AI(W906-S26-R1) 20261004: the "Z at its origin" rule of MachineType.h W906_DS402_ZSAFE_HOMEFLAG (S-26 Appendix A R1, DEFAULT OFF --
//  read the switch's note there).  Reached only through W906_ZSAFE_ORIGIN in InArmZSafe / OutArmZSafe (iFlag & 1) and
//  CheckInArmZNeedHome / CheckOutArmZNeedHome above.  Not golden: golden trusts the lamp alone.  Every fact is read from the engine
//  route's sample (Motor/EcatMotorRoute.h, the same reads TMyEtherCatMotor makes), never from a timer; no route installed (every SIM
//  build, every ctest but S26_R1_ZSafe_On) -> false -> the lamp alone.  At the end of the file so no line above moves.
//AI(W906-S26-R1-REVIEW) 20261005: laptop review R1-1 -- HomeFlag + command 0 alone is not a physical "Z is up": TMyMotor::HomeFlag
//  survives a motor or system power drop (csystem.cpp DoSystem SnSystemPower / SnMotorPower off clear only fAllMotorHome = golden
//  csystem.cpp:4348 / :4384) and, on a ServoAlarmOn=0 row (machines/HT9050/Mot_Table.csv M03 MInArmZA, M22 MOutArmZA), a drive alarm
//  (ScanAllMotorStatus clears HomeFlag only when PServoAlarmOn==1, golden :4052 / :4078); and the card's command position does not
//  follow an axis that moved while servo-off or slipped (WebMotorAccess.cpp W5B-6 note).  So the rule now ALSO needs:
//    (a) fAllMotorHome -- golden's own "a full HOME ended and nothing voided it since": cleared by both power-drop arms above, the EMG
//        LED arm and the alarm raise of ScanAllMotorStatus, the EMG / out-of-power arm (IsIndexMotorOutOfPower ->
//        LockIndexMotorAndDoHomeProcess), CheckMotorPowerShutDown, the relay / servo-ON / breaker buttons
//        (JsonBridge/IoBtnPanelClick.cpp), the Teach / Motor Test pages, Initial Start and every full HOME start (uhome.cpp);
//    (b) the same sample shows SVON on and neither ALM nor EMG (AX_MOTION_IO_*, EtherCAT/vendor/AdvMotDrv.h:2037 / :2024 / :2029);
//    (c) the ENCODER (actPos) is at the home as well as the command: |actPos| <= 1 pulse.
//  Any of them missing -> false -> the lamp alone (golden).  ctest S26_R1_ZSafe_On case 9 has one case per condition.
#include "Motor/EcatMotorRoute.h"
#include <cmath>
namespace {
struct W906_ZSafeEcatAddr : TMyEtherCatMotor {   // the route's key = TMyEtherCatMotor's OWN protected iBoardID / iPortID (shorts, set by
    static short TMyEtherCatMotor::* Board() { return &W906_ZSafeEcatAddr::iBoardID; }   //  its constructor from the Mot_Table address);
    static short TMyEtherCatMotor::* Port()  { return &W906_ZSafeEcatAddr::iPortID; }    //  they SHADOW HTMotor's iBoardID / iPortID, which
};                                                                               //  this class never sets -- same reach as Motor/EcatMotorRoute.cpp EcatAddr
const unsigned long kW906ZsAlm  = 0x00000002ul;   // AX_MOTION_IO_ALM  (EtherCAT/vendor/AdvMotDrv.h:2024; measured 20260911: 1 on every ERROR_STOP axis)
const unsigned long kW906ZsEmg  = 0x00000040ul;   // AX_MOTION_IO_EMG  (:2029; golden TMyEtherCatMotor::GetAlarm counts it, mask 0x3004e)
const unsigned long kW906ZsSvOn = 0x00004000ul;   // AX_MOTION_IO_SVON (:2037; TMyMotor::MotorHome case 10 already needs it at the end of a home)
}  // namespace
bool W906_Ds402ZAtOrigin(int motIndex, bool lamp)
{
    if (lamp) return true;                                                      // golden: the lamp
    if (motIndex < 0 || motIndex >= MAX_TRAY_MOTOR) return false;
    if (!fAllMotorHome) return false;                                           // (a) a full HOME ended and no power drop / alarm / EMG / manual page voided it
    TMyMotor& m = MOT[motIndex];
    if (!(m.CardType == "PCI1203") || m.HomeFlag != 1) return false;           // a Mot_Table PCI1203 row, homed
    TMyEtherCatMotor* e = dynamic_cast<TMyEtherCatMotor*>(m.Motor);            // cinitial.cpp builds that row as a TMyEtherCatMotor (NULL -> 0)
    if (e == 0) return false;
    const int board = e->*W906_ZSafeEcatAddr::Board(), port = e->*W906_ZSafeEcatAddr::Port();
    TEcatAxisRead s;
    if (!W906_EcRead(board, port, s)) return false;                             // no route / not claimed / no valid sample
    if (s.pending || s.state != kEcStaReady) return false;                      // a sample after the last command, standing still
    if ((s.motionIO & kW906ZsSvOn) == 0 || (s.motionIO & (kW906ZsAlm | kW906ZsEmg)) != 0) return false;   // (b) servo on, no drive alarm, no EMG
    if (W906_EcHomeCardSide(board, port)) return false;                         // a card-side home zeroes ON the switch: the lamp decides
    return std::fabs(s.cmdPos) <= 1.0 && std::fabs(s.actPos) <= 1.0;          // (c) command AND encoder at the DS402 home (command never rewritten, Q1)
}
#endif
//------------------------------------------------------------------------------
//AI(W906-SVON-SYNCARD) 20261005: NB2-1 R230 (laptop W-60, St01 W-47) -- used by TMyMotor::PCIL132_ResetPos above. The class is the
//  test, as ServoOnAllMOT / W906_Ds402ZAtOrigin / Motor/EcatMotorRoute.cpp already do: cinitial.cpp builds a Mot_Table CardModel
//  "PCI1203" row as a TMyEtherCatMotor. Here (not inline there) because this file includes Motor/myEthercatmotor.h only above
//  ServoOnAllMOT. A NULL motor is not one (PCIL132_ResetPos has returned before that anyway).
bool W906_Is1203Motor(const HTMotor* m)
{
    return dynamic_cast<const TMyEtherCatMotor*>(m) != 0;
}
//AI(W906-FULLHOME-ZSAFE) 20261005: NB2-1 R231 (laptop W-63; docs/nb2_assist/RD5軟體_HT9050_W63原點燈極性覆核_20261005_063800.md on
//  v906/nb2-assist) -- MachineType.h W906_HT9050_FULLHOME_ZSAFE, DEFAULT OFF. HT9050's In / Out Arm Z home in the drive (DS402 method
//  28), which stops at the switch edge on its uncovered side; TMyMotor::MotorHome case 20 takes that home (bW906HomeTrusted) with the
//  origin lamp OFF, so after a full HOME the Teach page refuses X / Y (golden IsCanQuickJogMove reads the lamp) until that Z is homed
//  alone: the single home (golden uhome.cpp ProcessSingleMotorHome case 500, WebMotorAccess.cpp TickHomes phase 1) goes on to
//  ZSafePos, inside the lit zone (machine oplog 20261004: lit from +28, ZSafePos 50). ON = every engine home of such an axis ends
//  there as well: case 20 goes to Task 40 instead of returning 1, Task 40 calls MotorMove(ZSafePos) until it answers non-zero (golden
//  case 500 `if(MOT[Index].MotorMove(ZSafePos))` -- -1 door / -2 / -3 count as done there too), HomeFlag 0 while moving (as golden
//  case 500), then 1. Bounded by MotorHome's own ResetTime. Only for golden case 500's arm-Z names, an enabled axis, a home the 1203
//  route reported done (bW906HomeTrusted) and the HT9050 origin rule installed (W906_Ht9050OrgHome != -2 = wb_serve on a 9050GPIB
//  machine, a 1203 axis). OFF = MotorHome returns 1 at case 20 exactly as before. ctest FullHomeZSafe_On / FullHomeZSafe_Default.
#if defined(W906_HT9050_FULLHOME_ZSAFE)
static bool W906_IsGoldenArmZ(int i)   // golden uhome.cpp ProcessSingleMotorHome case 300 -> 500 list, in its order
{
    return i == MInArmZA || i == MInArmZB || i == MInArmZC || i == MInArmZD || i == MInArmZE || i == MInArmZF || i == MInArmZG ||
           i == MInArmZH || i == MOutArmZA || i == MOutArmZB || i == MOutArmZC || i == MOutArmZD || i == MOutArmZE ||
           i == MOutArmZF || i == MOutArmZG || i == MOutArmZH || i == MInArmZAe || i == MInArmZAf || i == MInArmZAg ||
           i == MInArmZAh || i == MInArmZBe || i == MInArmZBf || i == MInArmZBg || i == MInArmZBh || i == MOutArmZAe ||
           i == MOutArmZAf || i == MOutArmZAg || i == MOutArmZAh || i == MOutArmZBe || i == MOutArmZBf || i == MOutArmZBg ||
           i == MOutArmZBh || i == MOutSortAa || i == MOutSortAb;
}
#endif
bool W906_FullHomeZSafeWanted(const TMyMotor& m, bool trusted)
{
#if defined(W906_HT9050_FULLHOME_ZSAFE)
    if (!trusted || m.Motor == NULL || !m.Motor->Enable || !W906_IsGoldenArmZ(m.Mot_Name)) return false;
    return W906_Ht9050OrgHome(m.Mot_Name) != -2;
#else
    (void)m; (void)trusted;
    return false;
#endif
}
int W906_FullHomeZSafeTick(TMyMotor& m)
{
    return m.MotorMove(ZSafePos) != 0 ? 1 : 0;   // golden case 500: any non-zero answer = done
}
