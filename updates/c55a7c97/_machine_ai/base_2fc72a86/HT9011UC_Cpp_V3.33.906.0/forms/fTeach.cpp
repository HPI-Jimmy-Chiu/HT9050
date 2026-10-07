// =============================================================================
//  forms/fTeach.cpp  --  the 48 read-only-direction TfTeach bodies translated
//  by wave FW-TEACH-W25.
//
//  AI(W906-FW-TEACH-W25) 20260826: new file.
//  Golden source of truth: HT9011UC_Code_V3.33.906.0_20260618/uteach.cpp
//  (6,056 lines, cp950, 0 U+FFFD, decoded this wave).
//
//  READ forms/fTeach.h FIRST.  It carries the whole contract: the measured
//  denominator (156 golden definitions), the selection rule (R1)/(R2) and WHY
//  CheckAndReadIniData counts as a WRITE, the 108-row DEFERRED REGISTER, the
//  GOLDEN NOTES (G-a..G-i) and the PORT DEVIATIONS (D-1..D-8).  Not repeated
//  here; this file carries per-statement pointer-backs only.
//
//  ⚠ NO MACHINE ACTION AND NO PERSISTENCE ANYWHERE IN THIS FILE.  Measured,
//  not asserted: across all 48 bodies below the occurrence count of
//  MotorMove / Gali_MotMove / Gali_MovePR / JogP / JogN / Gali_JogP /
//  Gali_JogN / ServoOnOff / Gali_Command / SetSpeed / SetGroup /
//  SetMotorScaleSpeed / InitMOTParameter / InitProcessSingleMotorTask /
//  StopAllMotor / IOBitOn / IOBitOff / Send_Command_TTL / WriteIniData /
//  WriteData / CheckAndReadIniData / SaveToFile / ReadFromFile / fopen /
//  CreateFile is ZERO.  Motor STATUS POLLING (ScanMotorStatus /
//  Gali_ScanMotStatus / ReadPos / ReadEncoderPos / Gali_ReadPos /
//  Gali_ReadEncoderPos) and sensor reads DO occur -- they are queries, see the
//  header's selection rule.
//
//  ⚠ NOT WIRED.  No OnClick/OnChange/OnMouseDown/OnDblClick delegate is
//  assigned for any handler below; stock vclcompat widgets carry no delegate
//  slot (forms/fTemp_Set.h GATE(G-Delegate)).  Nothing in this tree can reach
//  these bodies, which is why the NULL-pointer exposures inherited from golden
//  (header GOLDEN NOTE (G-i)) are unreachable today.
//
//  ⚠ CMAKE: NOT WIRED by this wave.  Register in **ht9045_sm**, NOT
//  ht9045_forms -- the measured reason is in forms/fTeach.h's INTEGRATION
//  REQUIREMENT block.
// =============================================================================
#include "MachineDefine.h"       // de-VCL'd include hub (golden uteach.cpp:1)

#include "forms/fTeach.h"

#include "forms/fQwertyKey.h"    // fQwertyKey, TfQwertyKey::ShowQwertyKey (:370/:406)
#include "cmydef.h"              // N_INTEGER (:289); K_RETRY (:276); MMSystem (:2398);
                                 // M*/C_*/Sn* index enums; INDEX_MOTION_CARD;
                                 // MOTION_CARD_TYPE; SHUTTLE_FLOODGATE; TRAY_ARM_MODE;
                                 // USE_IN_OUT_ARM_X_PITCH/_Y_PITCH; USE_PICKER_COUNT;
                                 // iXpitchMin/MaxX3/X7; IN_OUT_ARM_Y_PITCH_MIN;
                                 // fIndexDownPos; iInArmPitch/iOutArmPitch (:2622-2623);
                                 // InArmZIndex/OutArmZIndex (:3103-3104); TColor (:16)
#include "cprod.h"               // MAX_ARM_Row / MAX_ARM_Col
#include "MachineType.h"         // eUnderCoveyor / ep1Picker / iXPitch16Pick /
                                 // iXYPitch16Picker / iXYPitch16Bd_Be / MotionCard_Contec
#include "Motor/mymotor.h"       // MOT[] (TTrayMotor, :385) + ScanMotorStatus/ReadPos/...
#include "Motor/HTMotor.h"       // iHomeLed (:42), MotorType/PSoftLimitP/PSoftLimitN
#include "mysensor.h"            // Sen[] (:48), TMySensor::IsOn (:42)
#include "mycylin.h"             // Cylinder[] (:176), OffSensor (:120), Status (:73)
#include "csystem.h"             // IsEMGPressed (:147)
#include "canary_support.h"      // ShowMyMessage (:80), ShowErrorMessage (:66)
#include "aHotPlateSubstrate.h"  // InArmSuck (TMyKitSuck, the 177-TU shape, :624)
#include "acarry_shims.h"        // fLtcSensor (TfLtcSensor*, :90)

#include "vclcompat/LedCore.h"   // vclcompat::clYellow (:60) -- DEVIATION (D-6)
#include "vclcompat/SysUtils.h"  // IntToStr (:23)

#include <cstdlib>               // atoi, abs

//------------------------------------------------------------------------------
// golden uteach.cpp:41 -- `TfTeach *fTeach;`.  Zero-initialized, NEVER
// constructed at static init: golden's own pre-CreateForm state (golden
// creates the form in HT9045.cpp, not here).  Same posture as
// forms/fQwertyKey.cpp:41-42.  This is deliberate SIOF avoidance -- see
// forms/fTeach.h CTOR / Init() SPLIT.
//------------------------------------------------------------------------------
TfTeach *fTeach;

// golden uteach.cpp:45 -- `bool Teachshow=false;`
bool Teachshow = false;

//------------------------------------------------------------------------------
// AI(W906-FW-TEACH-W25) 20260826: PORT-ONLY ctor -- FIELDS ONLY.
// Carries golden's ctor lines :257-266 (nine scalar writes) and :276-284 (two
// loops that touch only this object's own bool arrays).  Golden's ctor line
// :267 onward is DEFERRED -- see the ctor's entry in forms/fTeach.h's
// DEFERRED REGISTER for the exact remainder.  Every other field default lives
// as an in-class member initializer in the header (BCB6 TObject zero-fill
// equivalent; plain C++ has none).
//------------------------------------------------------------------------------
TfTeach::TfTeach()
{
    bUseIniFile=false;                                                          // golden :257  Steven 20240501 : Teach改存成ini
    ActiveMotorIndex=-1;                                                        // golden :259
    TwoActiveMotorIndex=-1;                                                     // golden :260
    fShow=false;                                                                // golden :261
    fTechAuto=false;                                                            // golden :262
    iTechIndex=1;                                                               // golden :263
    iTechTask=1;                                                                // golden :264
    bAutoGetHomeSensor2Position=false;                                          // golden :265
    iGetHomeSensor2PositionTask=1;                                              // golden :266

    for(int i=0; i<2; i++)                                                      // golden :276-284
    {
        for(int j=0; j<8; j++)
        {
            bInArmZHome[i][j]=true;                                             // golden :280
            bOutArmZHome[i][j]=true;                                            // golden :281
        }
        bSortArmZHome[i]=true;                                                  // golden :283  RogerYang 20250416 for HT9046AU add
    }
    // golden :258 `AnsiString asString;` is a local of the DEFERRED remainder
    // (its first use is golden :305, the .BMP load block) -- not carried.

    //AI(W906-TEACH-W1) 20260919: golden 的 ctor 在 :342-994 接著建 teach 參數
    // 登錄表。那 654 行放在 forms/fTeachRegistry.cpp（**產生的**，來源
    // tools/gen_teach_registry.py），由這裡呼叫一次 —— 同物件、同順序、
    // 同時機（ctor 結束前），語意與 golden 相同。
    // 拆出去的理由：forms/fTeach.cpp 的既有行號被別處引用，往 ctor 中間塞
    // 654 行會把它們全部推掉（memory: never-mechanically-shift-line-citations）。
    //
    // ⚠ 這一行讓 ctor 從「只寫自己的欄位」變成「會配置記憶體」（440 個 new）。
    //   `fTeach` 仍然**不在靜態初始化期建構**（forms/fTeach.cpp:72 是裸指標，
    //   刻意的 SIOF 迴避），所以這些 new 只會在有人真的 `new TfTeach()` 時跑。
    BuildTechRegistry();                                                        // golden :342-994
}

// =============================================================================
//  (b) Pure predicates -- the interlock guards themselves.
// =============================================================================
//------------------------------------------------------------------------------
//  golden :1036-1221.  GOLDEN NOTE (G-e): the `#ifdef SOFT_SIMULTE` early
//  `return true;` and the missing return after `#endif` are golden's own
//  shape, kept verbatim.  SOFT_SIMULTE is undefined in this tree
//  (MachineType.h:48), i.e. the shipping-build path.
//  Every ScanMotorStatus / Gali_ScanMotStatus call below is a STATUS QUERY,
//  not motion (see this file's banner).
//------------------------------------------------------------------------------
bool TfTeach::IsCanQuickJogMove()
{
    #ifdef SOFT_SIMULTE
        if(!W906_TeachBodyInSim(ActiveMotorIndex)) return true;                 // golden :1039 `return true;` —— AI(W906-W5-b) 20260925: W5B-3 被移動的是真的會動的 1203 軸時不跳過本體（forms/fTeach.h 檔尾）
    #endif                                                                      // golden :1040 是 #else —— AI(W906-W5-b) 20260925: 本體兩種組態都編
    {
        if(ActiveMotorIndex==-1)                                                // golden :1042
            return false;

        if(ActiveMotorIndex==MInArmX || ActiveMotorIndex==MInArmY)               // golden :1045
        {
            for(int i=0; i<MAX_ARM_Row; i++)                                    // golden :1047  Steven 20240601 : 修正for Teach
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    W906_TeachScanMotorStatus(InArmZIndex[i][j]);                           // golden :1051  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    if(W906_TeachHomeLed(InArmZIndex[i][j])==false)                         // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    {
                        ShowMyMessage("Please let InArm Z at home position first!!", "請先讓InArm的Z軸在home的位置上!!", "Teach");
                        return false;
                    }
                }
            }
        }

        if(ActiveMotorIndex==MOutArmX || ActiveMotorIndex==MOutArmY)             // golden :1061
        {
            for(int i=0; i<MAX_ARM_Row; i++)
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    W906_TeachScanMotorStatus(OutArmZIndex[i][j]);                          // golden :1067  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    if(W906_TeachHomeLed(OutArmZIndex[i][j])==false)                        // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    {
                        ShowMyMessage("Please let OutArm Z at home position first!!", "請先讓OutArm的Z軸在home的位置上!!", "Teach");
                        return false;
                    }
                }
            }
        }

        if(ActiveMotorIndex==MInShuttle1 || ActiveMotorIndex==MInShuttle2)       // golden :1077
        {
            W906_TeachGaliScanMotStatus(MTestZ1);                                   // golden :1079  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            if(W906_TeachHomeLed(MTestZ1)==false)                                   // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            {
                ShowMyMessage("Please let MTestZ1 at home position first!!", "請先讓MTestZ1在home的位置上!!", "Teach");
                return false;
            }
            W906_TeachGaliScanMotStatus(MTestZ2);                                   // golden :1085  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            if(W906_TeachHomeLed(MTestZ2)==false)                                   // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            {
                ShowMyMessage("Please let MTestZ2 at home position first!!", "請先讓MTestZ2在home的位置上!!", "Teach");
                return false;
            }

            for(int i=0; i<MAX_ARM_Row; i++)                                    // golden :1092
            {
                for(int j=0; j<MAX_ARM_Col; j++)
                {
                    W906_TeachScanMotorStatus(InArmZIndex[i][j]);                           // golden :1096  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    if(W906_TeachHomeLed(InArmZIndex[i][j])==false)                         // golden :1097  Steven 20091004 Start: Shuttle move need check In/Out Arm Z position  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    {
                        ShowMyMessage("Please let InArm Z at home position first!!", "請先讓InArm的Z軸在home的位置上!!", "Teach");
                        return false;
                    }
                    W906_TeachScanMotorStatus(OutArmZIndex[i][j]);                          // golden :1102  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    if(W906_TeachHomeLed(OutArmZIndex[i][j])==false)                        // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                    {
                        ShowMyMessage("Please let OutArm Z at home position first!!", "請先讓OutArm的Z軸在home的位置上!!", "Teach");
                        return false;
                    }
                }
            }
        }

        if(INDEX_MOTION_CARD==0 && (ActiveMotorIndex==MTestY1 ||                // golden :1112
                                    ActiveMotorIndex==MTestY2))                 // Steven 20210623 : Index使用Galil
        {
            W906_TeachGaliScanMotStatus(MTestZ1);                                   // golden :1115  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            if(W906_TeachHomeLed(MTestZ1)==false)                                   // golden :1116  Steven 20091004 Start : Move Test Arm need check Z first  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            {
                ShowMyMessage("Please let MTestZ1 at home position first!!", "請先讓MTestZ1在home的位置上!!", "Teach");
                return false;
            }
            W906_TeachGaliScanMotStatus(MTestZ2);                                   // golden :1121  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            if(W906_TeachHomeLed(MTestZ2)==false)                                   // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            {
                ShowMyMessage("Please let MTestZ2 at home position first!!", "請先讓MTestZ2在home的位置上!!", "Teach");
                return false;
            }
        }

        if(ActiveMotorIndex==MTrayX)                                            // golden :1129
        {
            if(TRAY_ARM_MODE==eUnderCoveyor)                                    // golden :1131
            {
                if(W906_TeachHomeLed(MTrayZ)==false)                                    // golden :1133  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                {
                    ShowMyMessage("Please let TrayArm Z at home position first!!", "請先讓TrayArm的Z軸在home的位置上!!", "Teach");
                    return false;
                }

                if((Cylinder[C_TrayXFloodgate1].Enable==true && Cylinder[C_TrayXFloodgate1].OffSensor()==true ) ||   // golden :1139
                   (Cylinder[C_TrayXFloodgate2].Enable==true && Cylinder[C_TrayXFloodgate2].OffSensor()==true ) ||
                   (Cylinder[C_TrayXFloodgate3].Enable==true && Cylinder[C_TrayXFloodgate3].OffSensor()==true ) ||
                   (Cylinder[C_TrayXFloodgate4].Enable==true && Cylinder[C_TrayXFloodgate4].OffSensor()==true))
                {
                    ShowMyMessage("Tray X Floodgate Need Open", "Tray X 閘門需打開!!");
                    return false;
                }
            }
            else
            {
                // GOLDEN NOTE (G-f): `.Status` (commanded) here vs `.OffSensor()`
                // (sensed) everywhere else in this function.  Golden's own
                // inconsistency, preserved.
                if(Cylinder[C_TrayX_UpDown].Status)                             // golden :1150
                {
                    ShowMyMessage("Tray X can't move when cylinder is at down position!!", "氣缸在下始點不能進行寸動!!", "Teach");
                    return false;
                }

                W906_TeachScanMotorStatus(MInArmY);                                     // golden :1156  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                if(W906_TeachHomeLed(MInArmY)==false)                                   // golden :1157  Steven 20091006 Start: Before move Tray Arm, In/OutArm Y must home  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                {
                    ShowMyMessage("Please let InArm Y at home position first!!", "請先讓InArm的Y軸在home的位置上!!", "Teach");
                    return false;
                }
                W906_TeachScanMotorStatus(MOutArmY);                                    // golden :1162  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                if(W906_TeachHomeLed(MOutArmY)==false)                                  // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                {
                    ShowMyMessage("Please let OutArm Y at home position first!!", "請先讓OutArm的Y軸在home的位置上!!", "Teach");
                    return false;
                }
            }
        }

        if(ActiveMotorIndex==MMagazine)                                         // golden :1171  JerryYang 20220909 : add magazine
        {
            if(W906_TeachHomeLed(MCatchMgzTray)==false)                             // golden :1173  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
            {
                ShowMyMessage("Please let Magagzine Y In Axis at home position first!!", "請先讓Magazine Y In軸在home的位置上!!", "Teach");
                return false;
            }

            if(Sen[SnMagazineTrackDetect].IsOn()==true ||                       // golden :1179
               Sen[SnMagazineTrackDetect2].IsOn()==true)                        // Sam 20221116 : Magazine TrayArm 自動補 Tray
            {
                ShowMyMessage("Magazine Track sensor is On, please remove the tray!!", "Magezine軌道sensor偵測到tray盤，請先取出tray盤!!", "Teach");
                return false;
            }
        }

        if(CheckShuttleCanMove(ActiveMotorIndex)==false)                        // golden :1187  Ztex 2023.06.02 Add Check_SHUTTLE_FLOODGATE_Staste
        {
            return false;
        }

        if(ActiveMotorIndex==MInArmPitchY || ActiveMotorIndex==MOutArmPitchY)   // golden :1192  Ztex 2024.09.05 Add 16 Picker Check Has In Safe Pos
        {
            if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||                      // golden :1194
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)
            {
                for(int i=0; i<InArmSuck.iMaxRow; i++)                          // golden :1197
                {
                    for(int j=0; j<InArmSuck.iMaxCol; j++)
                    {
                        W906_TeachScanMotorStatus(InArmZIndex[i][j]);                           // golden :1201  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                        if(W906_TeachHomeLed(InArmZIndex[i][j])==false)                         // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                        {
                            ShowMyMessage("Please let InArm Z at home position first!!", "請先讓InArm的Z軸在home的位置上!!", "Teach");
                            return false;
                        }
                        W906_TeachScanMotorStatus(OutArmZIndex[i][j]);                          // golden :1207  AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                        if(W906_TeachHomeLed(OutArmZIndex[i][j])==false)                        // AI(W906-W5-b) 20260925: W5B-3 1203 軸讀監看器
                        {
                            // GOLDEN NOTE (G-e): golden :1210 prints the INARM
                            // message on the OUTARM branch.  Copy/paste slip in
                            // golden, preserved verbatim.
                            ShowMyMessage("Please let InArm Z at home position first!!", "請先讓InArm的Z軸在home的位置上!!", "Teach");
                            return false;
                        }
                    }
                }
            }
        }

        return W906_Teach3AxesHomeOk(ActiveMotorIndex);                         // golden :1218 `return true;` —— //AI(W906-TEACH-3AXES) 20261001: EastSun 1001「三軸都幫我加 在合適的地方」：MOutShuttle1／2、MCCDY 的非 golden 互鎖（本檔檔尾；其他軸＝true，與 golden 相同）
    }
    // golden :1219 #endif —— AI(W906-W5-b) 20260925: 移到 :139（本體兩種組態都編）
}
//------------------------------------------------------------------------------
//  golden :2457-2468
//------------------------------------------------------------------------------
bool TfTeach::CheckCanMove()
{
    if(IsEMGPressed())                                                          // golden :2459
    {
        ShowMyMessage("EMG Stop", "");
        return false;
    }
    else
    {
        return true;
    }
}
//------------------------------------------------------------------------------
//  golden :5428-5484.  Ztex 2023.06.02 Add Check_SHUTTLE_FLOODGATE_Staste
//  Golden dispatches on the RAW LITERALS 11 / 12 with `//MInShuttle1` /
//  `//MInShuttle2` comments rather than on the enums themselves; kept
//  verbatim (changing them to MInShuttle1/MInShuttle2 would be a behaviour
//  change if the enum ever moves, which is exactly the kind of silent
//  divergence this port must not introduce on its own initiative).
//------------------------------------------------------------------------------
bool TfTeach::CheckShuttleCanMove(int iMotorIndex)
{
    int iActiveMotorIndex = iMotorIndex;                                        // golden :5430
    bool bReturnStatus = false;
    switch(iActiveMotorIndex)
    {
        case 11:                                                                // golden :5434  MInShuttle1
        {
            if(SHUTTLE_FLOODGATE==1)
            {
               if(Cylinder[C_Shuttle1Floodgate].OffSensor()==false  &&          // golden :5438
                  Cylinder[C_OutShuttle1Floodgate].OffSensor()==false)          // Ifor 20240620 add:Out Shuttle Floodgate
               {
                    ShowErrorMessage("WAR16435", K_RETRY, MMSystem);            // golden :5441
                    bReturnStatus = false;
               }
               else
               {
                  bReturnStatus = true;
               }
            }
            else
            {
                bReturnStatus = true;
            }
        }
        break;
        //========================================
        case 12:                                                                // golden :5456  MInShuttle2
        {
            if(SHUTTLE_FLOODGATE==1)
            {
                 if(Cylinder[C_Shuttle2Floodgate].OffSensor()==false)           // golden :5460
                 {
                        ShowErrorMessage("WAR16436", K_RETRY, MMSystem);        // golden :5462
                        bReturnStatus = false;
                 }
                 else
                 {
                    bReturnStatus = true;
                 }
            }
            else
            {
                bReturnStatus = true;
            }
        }
        break;
        //========================================
        default:
        {
            bReturnStatus = true;                                               // golden :5479
        }
        break;
    }
    return bReturnStatus;
}

// =============================================================================
//  (c) Status read -> display.
// =============================================================================
//------------------------------------------------------------------------------
//  golden :1322-1357.  GOLDEN NOTE (G-a): the `static` LED table is a
//  FUNCTION-LOCAL STATIC initialized from MEMBER pointers -- it latches
//  whichever instance first enters the function.  Kept verbatim, `static`
//  included.  TALed -> TfLedValue per DEVIATION (D-4).
//------------------------------------------------------------------------------
void TfTeach::ScanNowMotorStatus()
{
    static TfLedValue *pLed[10]={ALed1, ALed2, ALed3, ALed4, ALed5, ALed6, ALed7, ALed8, ALed9, ALed10};   // golden :1324
    if(ActiveMotorIndex==-1)                                                    // golden :1325
    {
        for(int i=0; i<10; i++)
            pLed[i]->Value=false;
        return;
    }

    if(INDEX_MOTION_CARD==0 && (ActiveMotorIndex==MTestY1 || ActiveMotorIndex==MTestZ1 ||    // golden :1332
                                ActiveMotorIndex==MTestZ2 || ActiveMotorIndex==MTestY2))     // Steven 20210623 : Index使用Galil
    {
        MOT[ActiveMotorIndex].Gali_ScanMotStatus();                             // golden :1335
        edtNowPosition->Text=MOT[ActiveMotorIndex].Gali_ReadPos();              // golden :1336
        pnlEncoderPos->Caption=MOT[ActiveMotorIndex].Gali_ReadEncoderPos();     // golden :1337
    }
    else
    {
        MOT[ActiveMotorIndex].ScanMotorStatus();                                // golden :1341
        edtNowPosition->Text=MOT[ActiveMotorIndex].ReadPos();                   // golden :1342
        if(MOT[ActiveMotorIndex].Motor->MotorType==1 || MOT[ActiveMotorIndex].Motor->MotorType==3)   // golden :1343
            pnlEncoderPos->Caption= MOT[ActiveMotorIndex].ReadEncoderPos();
        else
            pnlEncoderPos->Caption= MOT[ActiveMotorIndex].ReadPos();
    }

    for(int i=0; i<10; i++)                                                     // golden :1349
        pLed[i]->Value=MOT[ActiveMotorIndex].Led[i];

/*                                                                                 golden :1352-1356 -- golden's own commented-out block, carried verbatim
    if(ActiveMotorIndex==MInArmPitch || ActiveMotorIndex==MOutArmPitch ||
       ActiveMotorIndex==MInArmPitchY || ActiveMotorIndex==MOutArmPitchY)
        ALed10->Value=MOT[ActiveMotorIndex].Home2Led;
*/
}
//------------------------------------------------------------------------------
//  golden :3474-3515.  GOLDEN NOTE (G-b): `iTechSelMot[0]>=0` passes in the
//  zero state because 0 is a valid motor index -- golden's own zero-fill
//  exposure, reproduced exactly (header member initializer `{0,0}`).
//------------------------------------------------------------------------------
void TfTeach::GetTechPos()
{
    if(ActiveMotorIndex==-1)                                                    // golden :3476
        return;

    if(ActiveFlag[0])                                                           // golden :3479
    {
        if(ActiveMotorIndex==MTestZ1 || ActiveMotorIndex==MTestZ2)
        {
            EditPtr->Text=MOT[ActiveMotorIndex].Gali_ReadEncoderPos();          // golden :3483
        }
        else
        {
            if(MOTION_CARD_TYPE==MotionCard_Contec && MOT[ActiveMotorIndex].Motor->MotorType==0)   // golden :3487
            {
                EditPtr->Text=MOT[ActiveMotorIndex].ReadPos();
            }
            else
            {
                EditPtr->Text=MOT[ActiveMotorIndex].ReadEncoderPos();
            }
        }
    }

    if(ActiveFlag[1])                                                           // golden :3498
    {
        if(INDEX_MOTION_CARD==0 && (TwoActiveMotorIndex==MTestY1 || TwoActiveMotorIndex==MTestZ1 ||   // golden :3500
                                    TwoActiveMotorIndex==MTestZ2 || TwoActiveMotorIndex==MTestY2))    // Steven 20210623 : Index使用Galil
        {
            if(OldIndex>0)                                                      // golden :3503
                EditPtr->Text=MOT[OldIndex].Gali_ReadEncoderPos();
            EditPtr1->Text=MOT[TwoActiveMotorIndex].Gali_ReadEncoderPos();
        }
        else
        {
            if(iTechSelMot[0]>=0)                                               // golden :3509
                EditPtr->Text=MOT[iTechSelMot[0]].ReadEncoderPos();
            if(iTechSelMot[1]>=0)                                               // golden :3511
                EditPtr1->Text=MOT[iTechSelMot[1]].ReadEncoderPos();
        }
    }
}
//------------------------------------------------------------------------------
//  golden :3697-3700.  `TObject *Sender` dropped, unread (DEVIATION (D-2)).
//------------------------------------------------------------------------------
void TfTeach::btnRecordY1Click()
{
    GetTechPos();                                                               // golden :3699
}

// =============================================================================
//  (a) Pure computation / string assembly.
// =============================================================================
//------------------------------------------------------------------------------
//  golden :2014-2021 / :2023-2030 / :2032-2039 / :2041-2048.
//  JimmyChiu 20220708 : add Index Arm Axis
//  `Str.sprintf("%s ...", Str, ...)` passes a RAW AnsiString to %s -- the BCB6
//  idiom.  vclcompat's variadic sprintf handles it (vclcompat/AnsiString.h:
//  150 + conv() :186), so the golden text stands unchanged.
//------------------------------------------------------------------------------
AnsiString TfTeach::GetMinXPitchCaptionName(AnsiString Str)
{
    if(Str=="")                                                                 // golden :2016
        Str.sprintf("%0.0fmm", GetMinXPitchMM());
    else
        Str.sprintf("%s %0.0fmm", Str, GetMinXPitchMM());
    return Str;
}
//------------------------------------------------------------------------------
AnsiString TfTeach::GetMaxXPitchCaptionName(AnsiString Str)                     // golden :2023-2030
{
    if(Str=="")
        Str.sprintf("%0.0fmm", GetMaxXPitchMM());
    else
        Str.sprintf("%s %0.0fmm", Str, GetMaxXPitchMM());
    return Str;
}
//------------------------------------------------------------------------------
AnsiString TfTeach::GetMinYPitchCaptionName(AnsiString Str)                     // golden :2032-2039
{
    if(Str=="")
        Str.sprintf("%0.0fmm", GetMinYPitchMM());
    else
        Str.sprintf("%s %0.0fmm", Str, GetMinYPitchMM());
    return Str;
}
//------------------------------------------------------------------------------
AnsiString TfTeach::GetMaxYPitchCaptionName(AnsiString Str)                     // golden :2041-2048
{
    if(Str=="")
        Str.sprintf("%0.0fmm", GetMaxYPitchMM());
    else
        Str.sprintf("%s %0.0fmm", Str, GetMaxYPitchMM());
    return Str;
}
//------------------------------------------------------------------------------
double TfTeach::GetMinXPitchMM()                                                // golden :2050-2056
{
    if(USE_IN_OUT_ARM_X_PITCH==iXPitch16Pick)                                   // golden :2052  Ztex 2024.02.29 Add HT-1032AT
        return (double)(iXpitchMinX7*0.01);
    else
        return (double)(iXpitchMinX3*0.01);
}
//------------------------------------------------------------------------------
double TfTeach::GetMaxXPitchMM()                                                // golden :2058-2064
{
    if(USE_IN_OUT_ARM_X_PITCH==iXPitch16Pick)                                   // golden :2060  Ztex 2024.02.29 Add HT-1032AT
        return (double)(iXpitchMaxX7*0.01);
    else
        return (double)(iXpitchMaxX3*0.01);
}
//------------------------------------------------------------------------------
double TfTeach::GetMinYPitchMM()                                                // golden :2066-2069
{
    return (double)(IN_OUT_ARM_Y_PITCH_MIN*0.01);                               // golden :2068
}
//------------------------------------------------------------------------------
//  GOLDEN NOTE (G-c): the config value is DELIBERATELY ignored.
//------------------------------------------------------------------------------
double TfTeach::GetMaxYPitchMM()                                                // golden :2071-2074
{
    return 60.0;                                                                // golden :2073  //(double)(IN_OUT_ARM_Y_PITCH_MAX*0.01);   //Jimmychiu 20221208 依照已出機機台都是儲存Pitch=60
}
//------------------------------------------------------------------------------
//  golden :3702-3725
//------------------------------------------------------------------------------
AnsiString TfTeach::CheckEditNumber(int iMax, int iMin, AnsiString asCurrent)
{
    int iCurrent=atoi(asCurrent.c_str());                                       // golden :3704

    if(iMin>iMax)                                                               // golden :3706  Steven 20140505 : 要確保MAX > MIN
    {
        int Temp=iMax;
        iMax=iMin;
        iMin=Temp;
    }

    if(iCurrent>iMax)                                                           // golden :3713
    {
        return AnsiString(iMax);
    }
    else if(iCurrent<iMin)
    {
        return AnsiString(iMin);
    }
    else
    {
        return asCurrent;
    }
}

// =============================================================================
//  (d) UI state only.  All `TObject *Sender` dropped, unread (DEVIATION (D-2)).
// =============================================================================
//------------------------------------------------------------------------------
//  golden :5321-5329.  Steven 20230815 : Add for MotorTest
//  DEVIATION (D-6) for clYellow / the 0x00DFD9CC literal.
//------------------------------------------------------------------------------
void TfTeach::LockAllButton(bool bLock)
{
    PageControl2->Enabled   =!bLock;                                            // golden :5323
    btnMoveN->Enabled       =!bLock;
    btnMoveP->Enabled       =!bLock;
    btnHome->Enabled        =!bLock;
    btnMoveTo->Enabled      =!bLock;
    pnlStop->Color=(bLock)?vclcompat::clYellow:(TColor)0x00DFD9CC;              // golden :5328
}
//------------------------------------------------------------------------------
//  golden :5331-5343.  Steven 20230815 : Add for MotorTest
//------------------------------------------------------------------------------
void TfTeach::AllBtnUp()
{
    btnJogN->Down    =false;                                                    // golden :5333
    btnJogP->Down    =false;
    btnHome->Down    =false;
    btnMoveP->Down   =false;
    btnMoveN->Down   =false;
    btnMoveTo->Down  =false;
    if(PtrGoBtn020!=NULL)                                                       // golden :5339
        PtrGoBtn020->Down=false;
    if(PtrGoBtn140!=NULL)                                                       // golden :5341
        PtrGoBtn140->Down=false;
}
//------------------------------------------------------------------------------
//  golden :4295-4298.  DEVIATION (D-5) for ->ActivePage.
//------------------------------------------------------------------------------
void TfTeach::Label2Click()
{
    PageControl2->ActivePage=tsAxleCtrl;                                        // golden :4297
}
//------------------------------------------------------------------------------
//  golden :3625-3628 -- golden's body is a single COMMENTED-OUT statement, so
//  the method is a no-op in golden too.  Carried, comment included, so the
//  ledger shows it was read and is genuinely empty (not "forgotten").
//------------------------------------------------------------------------------
void TfTeach::PageControl2Change()
{
//    TabSheet20->Visible=false;                                                // golden :3627
}
//------------------------------------------------------------------------------
//  golden :4710-4714
//------------------------------------------------------------------------------
void TfTeach::btClearMemoClick()
{
    Memo1->Lines->Clear();                                                      // golden :4712
    Memo2->Lines->Clear();                                                      // golden :4713
}
//------------------------------------------------------------------------------
//  golden :2098-2102
//------------------------------------------------------------------------------
void TfTeach::btnSetToClick()
{
    btnSetTo->Down=false;                                                       // golden :2100
    EditPtr->Text=edtNowPosition->Text;                                         // golden :2101
}
//------------------------------------------------------------------------------
//  golden :2200-2203
//------------------------------------------------------------------------------
void TfTeach::btnSetToOffsetClick()
{
    EditPtr->Text=edtSetToOffset->Text;                                         // golden :2202
}
//------------------------------------------------------------------------------
//  golden :3044-3047
//------------------------------------------------------------------------------
void TfTeach::pnlEncoderPosDblClick()
{
    EditPtr->Text=pnlEncoderPos->Caption;                                       // golden :3046
}

// =============================================================================
//  (e) Coordinate arithmetic from UI values -- compute only, never written back.
// =============================================================================
//------------------------------------------------------------------------------
//  golden :2956-2989.  ChungHung 20140116 沒有用到 (golden's own dead-code marker).
//  GOLDEN NOTE (G-d): `r` is left uninitialized on the two early-return paths;
//  those paths return before reading it.  `/3000.0` is a real (double)
//  division, not the integer-division hazard of memory
//  ht9045-divide-safety-int-to-float-regression.
//------------------------------------------------------------------------------
int TfTeach::GetPitchOffSet(int WhichPitch, int Pos)
{
    double m, r;                                                               // golden :2958
    int InArm50, InArm80, OutArm50, OutArm80;
    InArm50 =atoi(setEditInXPitch40->Text.c_str());                            // golden :2960
    InArm80 =atoi(setEditInXPitch120->Text.c_str());
    OutArm50=atoi(setEditOutXPitch40->Text.c_str());
    OutArm80=atoi(setEditOutXPitch120->Text.c_str());
    if(WhichPitch==MInArmPitch)                                                // golden :2964
    {
        if(InArm50==0 || InArm80==0 || InArm50 == InArm80)
        {
            return (Pos-iInArmPitch);
        }
        else
        {
            m=(InArm80-InArm50)/3000.0;                                        // golden :2972
            r=(double)InArm50+m*((double)Pos-5000.0);
        }
    }
    else
    {
        if(OutArm50==0 || OutArm80==0 || OutArm50==OutArm80)
        {
            return (Pos-iOutArmPitch);
        }
        else
        {
            m=(OutArm80-OutArm50)/3000.0;                                      // golden :2984
            r=(double)OutArm50+m*((double)Pos-5000.0);
        }
    }
    return (int)r;                                                             // golden :2988
}
//------------------------------------------------------------------------------
//  golden :5147-5157.  KenHsieh 20210813 : add CCD AUTO ALIGNMENT /
//  JimmyChiu 20220908 add Pickup Error Placement.
//  Reads two motor positions, subtracts the taught shuttle-2 coordinates and
//  writes the DIFFERENCE into two display edits.  Nothing is persisted.
//------------------------------------------------------------------------------
void TfTeach::SetComputeInSh2Click()
{
    int iNowInarmX=MOT[MInArmX].ReadPos();                                      // golden :5149
    int iNowInarmY=MOT[MInArmY].ReadPos();                                      // golden :5150
    int iSh2X=(setEditInSht2X->Text=="")?0:atoi(setEditInSht2X->Text.c_str());  // golden :5151
    int iSh2Y=(setEditInSht2Y->Text=="")?0:atoi(setEditInSht2Y->Text.c_str());  // golden :5152
    int iOffsetX=iNowInarmX-iSh2X;                                              // golden :5153
    int iOffsetY=iNowInarmY-iSh2Y;
    setEditInarmPlacementXOffsetByBasicSuck->Text=IntToStr(iOffsetX);           // golden :5155
    setEditInarmPlacementYOffsetByBasicSuck->Text=IntToStr(iOffsetY);           // golden :5156
}

// =============================================================================
//  (f) Read-only data listing.
// =============================================================================
//------------------------------------------------------------------------------
//  golden :4753-4777.  Lists the Y-latch capture buffers into the two memos.
//  `iShuttleSelect` is golden's FILE-STATIC (uteach.cpp:4709) set by the
//  DEFERRED btnSht1GoLatchClick / btnSht2GoLatchClick, so offline it stays 0
//  and both arms are skipped -- golden's own pre-latch state, not a port
//  behaviour.  Kept as a file-static here for the same reason.
//------------------------------------------------------------------------------
static int iShuttleSelect=0;                                                    // golden :4709

void TfTeach::btnGetLatchClick()
{
    if(iShuttleSelect==1)                                                       // golden :4755
    {
        for(int i=0; i<fLtcSensor->LatchDataCnt3; i++)
        {
            Memo1->Lines->Add(AnsiString(abs(fLtcSensor->LatchDataTable3[i]))); // golden :4759
        }
        for(int i=0; i<fLtcSensor->LatchDataCnt2; i++)
        {
            Memo2->Lines->Add(AnsiString(abs(fLtcSensor->LatchDataTable2[i]))); // golden :4763
        }
    }
    else if(iShuttleSelect==2)                                                  // golden :4766
    {
        for(int i=0; i<fLtcSensor->LatchDataCnt0; i++)
        {
            Memo1->Lines->Add(AnsiString(abs(fLtcSensor->LatchDataTable0[i]))); // golden :4770
        }
        for(int i=0; i<fLtcSensor->LatchDataCnt1; i++)
        {
            Memo2->Lines->Add(AnsiString(abs(fLtcSensor->LatchDataTable1[i]))); // golden :4774
        }
    }
}

// =============================================================================
//  (g) fQwertyKey numeric-keypad launchers -- 22, golden run :5159-5319.
//      Golden shape: ONE `fQwertyKey->ShowQwertyKey((TEdit *)Sender, iFunction,
//      iDP, bCheckRange, min, max);` call (two of the 22 branch first).
//      `Sender` kept and typed as golden's cast target (DEVIATION (D-3)).
//      On the min/max argument order see GOLDEN NOTES (G-g)/(G-h): golden's
//      house style puts the LARGER bound in the parameter named `min`, and
//      CheckRange (MachineType.h:1524-1544) opens with `if(Maximum<Minimum)`
//      so both orders clamp identically.  Every call below is verbatim,
//      argument for argument.
//      GOLDEN NOTE (G-i): `fQwertyKey` and `MOT[x].Motor` are both NULL
//      offline.  Unreachable today (NOT WIRED); a wiring wave must resolve
//      them first.
// =============================================================================
//------------------------------------------------------------------------------
void TfTeach::edtMoveToClick(TEdit *Sender)                                     // golden :5159-5165
{
    if(ActiveMotorIndex==-1)                                                    // golden :5161
        return;

    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[ActiveMotorIndex].Motor->PSoftLimitP, MOT[ActiveMotorIndex].Motor->PSoftLimitN);   // golden :5164
}
//------------------------------------------------------------------------------
void TfTeach::setEditZ1AClick(TEdit *Sender)                                    // golden :5167-5170
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 100, -3000);          // golden :5169
}
//------------------------------------------------------------------------------
void TfTeach::setEditInXPitch40Click(TEdit *Sender)                             // golden :5172-5175
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInArmPitch].Motor->PSoftLimitP, MOT[MInArmPitch].Motor->PSoftLimitN);             // golden :5174
}
//------------------------------------------------------------------------------
void TfTeach::setEditInY15Click(TEdit *Sender)                                  // golden :5177-5180
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInArmPitchY].Motor->PSoftLimitP, MOT[MInArmPitchY].Motor->PSoftLimitN);           // golden :5179
}
//------------------------------------------------------------------------------
void TfTeach::setEditOutXPitch40Click(TEdit *Sender)                            // golden :5182-5185
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MOutArmPitch].Motor->PSoftLimitP, MOT[MOutArmPitch].Motor->PSoftLimitN);           // golden :5184
}
//------------------------------------------------------------------------------
void TfTeach::setEditOutY15Click(TEdit *Sender)                                 // golden :5187-5190
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MOutArmPitchY].Motor->PSoftLimitP, MOT[MOutArmPitchY].Motor->PSoftLimitN);         // golden :5189
}
//------------------------------------------------------------------------------
void TfTeach::setEditIndex1ToSocketYClick(TEdit *Sender)                        // golden :5192-5195
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MTestY1].Motor->PSoftLimitP, MOT[MTestY1].Motor->PSoftLimitN);                     // golden :5194
}
//------------------------------------------------------------------------------
void TfTeach::setEditIndex2ToSocketYClick(TEdit *Sender)                        // golden :5197-5201
{
    if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                                     // golden :5199  JimmyChiu 20220708 : add Index Arm Axis
        fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MTestY2].Motor->PSoftLimitP, MOT[MTestY2].Motor->PSoftLimitN);                 // golden :5200
}
//------------------------------------------------------------------------------
void TfTeach::setEditIndex1ToSht1ZClick(TEdit *Sender)                          // golden :5203-5206
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 0, fIndexDownPos*100);   // golden :5205
}
//------------------------------------------------------------------------------
void TfTeach::setEditInSht1LeftClick(TEdit *Sender)                             // golden :5208-5211
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInShuttle1].Motor->PSoftLimitP, MOT[MInShuttle1].Motor->PSoftLimitN);             // golden :5210
}
//------------------------------------------------------------------------------
void TfTeach::setEditInSht2LeftClick(TEdit *Sender)                             // golden :5213-5216
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInShuttle2].Motor->PSoftLimitP, MOT[MInShuttle2].Motor->PSoftLimitN);             // golden :5215
}
//------------------------------------------------------------------------------
void TfTeach::setEditInXClick(TEdit *Sender)                                    // golden :5218-5221
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInArmX].Motor->PSoftLimitP, MOT[MInArmX].Motor->PSoftLimitN);                     // golden :5220
}
//------------------------------------------------------------------------------
void TfTeach::setEditInYClick(TEdit *Sender)                                    // golden :5223-5226
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInArmY].Motor->PSoftLimitP, MOT[MInArmY].Motor->PSoftLimitN);                     // golden :5225
}
//------------------------------------------------------------------------------
void TfTeach::setEditRotateAClick(TEdit *Sender)                                // golden :5228-5231
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MInRotateKit].Motor->PSoftLimitP, MOT[MInRotateKit].Motor->PSoftLimitN);           // golden :5230
}
//------------------------------------------------------------------------------
void TfTeach::edtEditRotateInBacklashClick(TEdit *Sender)                       // golden :5233-5236  RogerYang 20260113 : Rotator新增背隙補償
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 500, 0);              // golden :5235
}
//------------------------------------------------------------------------------
//  GOLDEN NOTE (G-h): (500, 2500) -- see the header.
//------------------------------------------------------------------------------
void TfTeach::edShtCheckRangeClick(TEdit *Sender)                               // golden :5238-5241
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 500, 2500);           // golden :5240
}
//------------------------------------------------------------------------------
void TfTeach::SetEditPickLoaderClick(TEdit *Sender)                             // golden :5243-5249
{
    if(USE_PICKER_COUNT==ep1Picker)                                             // golden :5245
        fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, -500, -7000);     // golden :5246
    else
        fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, -500, -3000);     // golden :5248
}
//------------------------------------------------------------------------------
void TfTeach::setEditInZSafeHeightClick(TEdit *Sender)                          // golden :5251-5254
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, 0, 200);              // golden :5253
}
//------------------------------------------------------------------------------
void TfTeach::setEditOutXClick(TEdit *Sender)                                   // golden :5256-5259
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MOutArmX].Motor->PSoftLimitP, MOT[MOutArmX].Motor->PSoftLimitN);                   // golden :5258
}
//------------------------------------------------------------------------------
void TfTeach::setEditOutYClick(TEdit *Sender)                                   // golden :5261-5264
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MOutArmY].Motor->PSoftLimitP, MOT[MOutArmY].Motor->PSoftLimitN);                   // golden :5263
}
//------------------------------------------------------------------------------
void TfTeach::setFix3LClick(TEdit *Sender)                                      // golden :5266-5269
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MFix3Full].Motor->PSoftLimitP, MOT[MFix3Full].Motor->PSoftLimitN);                 // golden :5268
}
//------------------------------------------------------------------------------
//  GOLDEN NOTE (G-g): golden mixes a soft limit with the literal -5000 here.
//------------------------------------------------------------------------------
void TfTeach::setEditTrayXClick(TEdit *Sender)                                  // golden :5271-5274
{
    fQwertyKey->ShowQwertyKey(Sender, N_INTEGER, 0, true, MOT[MTrayX].Motor->PSoftLimitP, -5000);   // golden :5273
}
//------------------------------------------------------------------------------
//==============================================================================
//  AI(W906-W5-b) 20260925: W5-b 覆核 W5B-3 —— IsCanQuickJogMove 本體讀狀態的來源（宣告與理由在 forms/fTeach.h 檔尾）。
//  放在檔尾而不是 IsCanQuickJogMove 前面：本檔行號被別處引用（never-mechanically-shift-line-citations），本體那 26 行是同行替換。
//==============================================================================
bool (*W906_TeachLive1203Hook)(int motIndex) = 0;
int  (*W906_TeachHomeLedHook)(int motIndex)  = 0;

bool W906_TeachBodyInSim(int motIndex)
{
    // 沒裝掛鉤 ＝ golden 原樣（SOFT_SIMULTE 直接 return true）
    return W906_TeachLive1203Hook != 0 && W906_TeachLive1203Hook(motIndex);
}

static bool W906_TeachIs1203(int motIndex)
{
    return W906_TeachHomeLedHook != 0 && W906_TeachHomeLedHook(motIndex) >= -1;   //AI(W906-TEACH-ZDISABLED) 20261001: was `!= -2`; -3（表上 Enable=0 的非 1203 軸）也不是 1203 ⇒ golden 掃描照跑
}

void W906_TeachScanMotorStatus(int motIndex)
{
    if (!W906_TeachIs1203(motIndex)) MOT[motIndex].ScanMotorStatus();          // golden；1203 軸的狀態由監看器每拍輪詢
}

void W906_TeachGaliScanMotStatus(int motIndex)
{
    if (!W906_TeachIs1203(motIndex)) MOT[motIndex].Gali_ScanMotStatus();       // golden；同上
}

bool W906_TeachHomeLed(int motIndex)
{
    const int s = W906_TeachHomeLedHook ? W906_TeachHomeLedHook(motIndex) : -2;
    if (s == -3) return true;                                                   //AI(W906-TEACH-ZDISABLED) 20261001: EastSun 1001「如果 testz2 enable 是0 那就不要擋testz2」—— Mot_Table Enable=0 的非 1203 軸不檢查（偏離 golden：golden 讀 Led[iHomeLed]，而 INDEX_MOTION_CARD==0 的 Galil 分支不管表上 Enable 一律開 MTestZ2、無卡時 HOME 燈永遠不寫 ⇒ 飛梭永遠被擋）。Enable=1 的軸、表上沒有的軸照舊（fail-closed）
    if (s == -2) return MOT[motIndex].Led[iHomeLed];                            // 非 1203：golden 原本讀的
    return s == 1;                                                              // 1203：監看器 ORG；不明（-1）＝不在原點（fail-closed）
}

//==============================================================================
//  AI(W906-TEACH-3AXES) 20261001: EastSun 1001「三軸都幫我加 在合適的地方」—— 非 golden 的互鎖（宣告在 forms/fTeach.h 檔尾）。
//  教導頁新增了 MOutShuttle1／MOutShuttle2／MCCDY 的教導點（forms/fTeachRegistry.cpp E1，W906 擴充），這三軸從此會在教導頁被
//  jog／Move／Go。golden IsCanQuickJogMove 沒有它們的分支（一路掉到 :1218 `return true;`）⇒ 在 golden 每一道檢查（含
//  CheckShuttleCanMove 的飛梭閘門）都過了之後，再加這一道（IsCanQuickJogMove 那一行同行替換，本檔行號不動）：
//    * MOutShuttle1／MOutShuttle2：MTestZ1、MTestZ2、In Arm Z、Out Arm Z 都要在原點 —— 照 golden 同函式 In Shuttle 分支（:1077-1108）的寫法；
//    * MCCDY：MTestZ1、MTestZ2 在原點 —— 照 golden 同函式 Index Y 分支（:1112-1125）的寫法。
//  原點判斷是同一支 W906_TeachHomeLed：1203 軸讀監看器 ORG（套 HT9050 的 1203 ORG 規則）、狀態不明＝不在原點（fail-closed）；
//  Mot_Table Enable=0 的非 1203 軸不檢查（AI(W906-TEACH-ZDISABLED)，所以 HT9050 的 MTestZ2／MN200 Z 不擋）。訊息照 golden 的原字串。
//  其他軸一律 true（＝golden）。
//==============================================================================
bool W906_Teach3AxesHomeOk(int motIndex)
{
    if (motIndex == MOutShuttle1 || motIndex == MOutShuttle2)
    {
        W906_TeachGaliScanMotStatus(MTestZ1);
        if (W906_TeachHomeLed(MTestZ1) == false)
        {
            ShowMyMessage("Please let MTestZ1 at home position first!!", "請先讓MTestZ1在home的位置上!!", "Teach");
            return false;
        }
        W906_TeachGaliScanMotStatus(MTestZ2);
        if (W906_TeachHomeLed(MTestZ2) == false)
        {
            ShowMyMessage("Please let MTestZ2 at home position first!!", "請先讓MTestZ2在home的位置上!!", "Teach");
            return false;
        }
        for (int i = 0; i < MAX_ARM_Row; i++)
        {
            for (int j = 0; j < MAX_ARM_Col; j++)
            {
                W906_TeachScanMotorStatus(InArmZIndex[i][j]);
                if (W906_TeachHomeLed(InArmZIndex[i][j]) == false)
                {
                    ShowMyMessage("Please let InArm Z at home position first!!", "請先讓InArm的Z軸在home的位置上!!", "Teach");
                    return false;
                }
                W906_TeachScanMotorStatus(OutArmZIndex[i][j]);
                if (W906_TeachHomeLed(OutArmZIndex[i][j]) == false)
                {
                    ShowMyMessage("Please let OutArm Z at home position first!!", "請先讓OutArm的Z軸在home的位置上!!", "Teach");
                    return false;
                }
            }
        }
    }
    if (motIndex == MCCDY)
    {
        W906_TeachGaliScanMotStatus(MTestZ1);
        if (W906_TeachHomeLed(MTestZ1) == false)
        {
            ShowMyMessage("Please let MTestZ1 at home position first!!", "請先讓MTestZ1在home的位置上!!", "Teach");
            return false;
        }
        W906_TeachGaliScanMotStatus(MTestZ2);
        if (W906_TeachHomeLed(MTestZ2) == false)
        {
            ShowMyMessage("Please let MTestZ2 at home position first!!", "請先讓MTestZ2在home的位置上!!", "Teach");
            return false;
        }
    }
    return true;
}
