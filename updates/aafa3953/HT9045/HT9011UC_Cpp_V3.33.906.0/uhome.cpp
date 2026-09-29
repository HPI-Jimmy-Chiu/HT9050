// =============================================================================
//  uhome.cpp -- golden uhome.cpp:719-4842 的移植
//
//  AI(W906-HOME-C2) 20260920
//
//  ## 這個檔存在的唯一理由
//
//  P0-4 量到（見 docs/PLAN_START_TO_RUN.md）：網頁按 START 之後 spine 不跑，
//  因為 `MainProc` 每一個 tick 都被歸零臂攔下來 —— `DoHomeProcess()` 裡的
//  `CheckMotorHome()`（csystem.cpp:328）永遠回 false，因為 TOTAL_MOTOR 顆
//  馬達沒有一顆的 `HomeFlag` 被設成 1。
//
//  寫 `HomeFlag` 的 `TMyMotor::MotorHome()`（Motor/mymotor.cpp:1105/1132/1160）
//  **早就翻好而且是活的**。全樹沒有人呼叫它，因為呼叫它的是這個檔裡的
//  `ProcessMotorHome()` case 600。
//
//  ⇒ 這不是「多翻一個函式」，這是**接上唯一缺的那一段**。
//
//  ## 範圍
//
//      golden uhome.cpp:719-4842
//        :719-722    CylinderInitState / OKFlag / SwitchType
//        :724-878    SetCylinderResetStateHomeBegin
//        :879-889    ResetOKDeleyTime / SetHomeSerial
//        :890-1000   PrePushLifterCylinder / PrePushLoaderCylinder
//        :1001-1174  DoTopBtmHome
//        :1175-1179  外部宣告 + hRealCCDTimeOut / hHomeDelay
//        :1180-4842  ★ ProcessMotorHome（3,663 行，73 個 case）
//
//  **不在這一波**：
//    * `ProcessSingleMotorHome`（golden :415-644）—— 本樹已有回 true 的樁
//      （acatchtray_shims.cpp:134）。翻真本體要同時退掉那個樁，屬於另一顆 commit。
//    * `TfHome::InitGali_HomeTask`（golden :646）—— Galil 專屬，本機沒有卡。
//      AI(W906-MT-E3b) 20260925: 已翻（它只寫旗標），連同 `TfHome::GaliMotorServoOff`（golden :4988-5016）與
//      `TfHome::sbAbortHomeClick`（golden :4980-4986），兩個都在本檔檔尾。
//    * golden :1-718 的表單事件與 W906-HOME-C1 已落地的四個 TfHome 方法。
//
//  ## 閘的規矩（與本樹其他波次相同）
//
//  加閘的**唯一**合法理由是「相依不存在」。每一格閘都要寫：
//    (a) 缺什麼（型別/成員/函式，附 golden 出處）
//    (b) 閘住之後機台**實際**少做什麼（用機台的話，不是用程式的話）
//    (c) UN-GATE 的條件
//  能部分閘就不要整段閘。
//
//  ⚠ 閘的位置一律由 **g++** 決定，不由掃描器決定。
//    （memory: symbol-exists-has-three-strengths —— 名字在樹裡出現過
//     只是第一級證據；成員在不在、有沒有被 #if 0 閘掉，只有編譯器知道。）
//
//  ## ⛔ 這個檔會讓機台動
//
//  `ProcessMotorHome()` 直接呼叫 `MOT[i].MotorHome()`、`StopAllMotor()`、
//  `Cylinder[...].Push()/Pop()`。在有硬體的機台上，它就是歸零本身。
//  使用者 20260919 的裁決是「必須接上，不用擔心會動的風險」，
//  所以這裡**不加任何「怕它動」的閘** —— 那不是合法的加閘理由。
// =============================================================================
#include "vclcompat/vcl_compat.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstdio>                   // std::printf / std::fflush（bring-up 守衛的診斷）

#include "uhome.h"

#include "cmydef.h"
#include "cprod.h"
#include "cpublic.h"
#include "MachineType.h"
#include "Config.h"
#include "CosFunction.h"
#include "csystem.h"
#include "csystem_shims.h"
#include "Motor/mymotor.h"
#include "myswitch.h"
#include "mysensor.h"
#include "mycylin.h"
#include "aHotPlateSubstrate.h"
#include "acarry.h"
#include "atester.h"
#include "acatchtray.h"
#include "asendic.h"
#include "asendic_Empty.h"
#include "atester_shims.h"
#include "acatchtray_shims.h"
#include "canary_support.h"
#include "FormsFacade.h"
#include "common.h"
#include "MessageDef.h"
#include "forms/fHome.h"
#include "forms/fNote.h"
#include "AutoClean/AutoClean.h"
#include "RotateKit/aRotateKIT.h"
#include "RotateKit/aRotateKIT_In.h"
#include "RotateKit/aRotateKIT_Out.h"
#include "cinitial.h"
#include "ckernel.h"
#include "asortarm.h"
#include "aoutarm.h"
#include "ainarm_SearchPlacePlate.h"
#include "forms/fAOI.h"
// ⚠ 下面兩個只有**出貨組態**（-DW906_NO_SOFT_SIMULTE=ON）才用得到 ——
//   它們的呼叫點全在 `#ifndef SOFT_SIMULTE` 臂裡。
//   20260920 的教訓：我整輪都用預設（模擬）組態做 -fsyntax-only，
//   所以那些臂**一行都沒編過**，直到雙 gate 的 SHIPPING 側才爆出來
//   （13 個 error：FrmRotate x1、MNetLog x12）。
//   ⇒ 新檔進樹之前要**兩種組態各編一次**，不是只編 F5 那一種。
#include "forms/fRotate.h"        // FrmRotate（#ifndef SOFT_SIMULTE 臂）
#include "Motor/myMN200motor.h"   // MNetLog（同上）
#include "acarry_shims.h"

// ---------------------------------------------------------------------------
//  點名 extern —— **不是閘**，是繞開標頭對撞
//
//  下面這幾個符號在本樹都**存在而且是活的**，只是宣告它們的標頭
//  （ainarm9045.h / acarry_shims.h）與本 TU 已經帶進來的那幾個互相對撞
//  （同函式重複帶預設參數、同名類別兩份佈局…，見上面 include 區塊）。
//  為了一個符號把一整個會對撞的標頭拉進來，換來的是 header 自己爆，
//  而不是我寫的行爆 —— 那比點名 extern 糟。
//
//  ⚠ 點名 extern 的風險是「型別寫錯不會當場發現，會變成連結期或執行期問題」。
//    所以每一行都附出處，而且**逐字照抄那個標頭的宣告**。
// ---------------------------------------------------------------------------
extern int  iMoveToShuttle;                     // ainarm9045.h:35
extern void InitDoInArmAdditionalFunction();    // ainarm9045.h:123
extern void SetAutoSkipCount(int Add);          // ainarm9045.h:84
extern bool IndexZCanMove[2];                   // golden uhome.cpp:49 `extern bool IndexZCanMove[2];`

// golden uhome.h:90 `bool PrePushLoaderCylinder(bool bReset=false);`
//   本檔下面就是它的定義（golden uhome.cpp:967）。預設參數必須在**定義之前**
//   的某個宣告上給，否則 case 1250 的 `PrePushLoaderCylinder()` 會是 too-few-arguments。
//   ⚠ 這一行刻意**不**放進 uhome.h —— csystem.cpp:12295 有
//     `#define PrePushLoaderCylinder W7G3_PrePushLoaderCylinder`，放進標頭會讓那個
//     TU 把宣告本身改名。退那個樁是另一顆 commit 的事。
bool PrePushLoaderCylinder(bool bReset=false);

// golden uhome.cpp:395 `int iSingleMotorHomeTask[TOTAL_MOTOR];`
//   它與 `ProcessSingleMotorHome`（golden :415，本波不翻）同屬單軸歸零那一族。
//   本樹全樹 0 命中，所以在這裡定義；唯一的讀寫者是下面的 DoTopBtmHome。
int iSingleMotorHomeTask[TOTAL_MOTOR];

//==============================================================================
bool  CylinderInitState[CynForHome];                                            //true : Push(), false : Pop()
bool  CylinderInitOKFlag[CynForHome];                                           //true : 不需要復歸, false : 需要復歸
bool  CylinderInitSwitchType[CynForHome];                                       //RogerYang 20250909 : true: Push/Pop  false On/Off
//------------------------------------------------------------------------------
void SetCylinderResetStateHomeBegin()
{
    for(int i=0; i<CynForHome; i++)
    {
        CylinderInitSwitchType[i]=true;                                         //RogerYang 20250909 : true: Push/Pop  false On/Off

        if(LOAD_Z_USE_MOTOR[0]==true &&
           (CynNeedHome[i]==C_Load_Up ||
            CynNeedHome[i]==C_Load_Middle))                                     //JerryYang 20191017 修正loader Z馬達版回home問題
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[1]==true &&
                (CynNeedHome[i]==C_Empty_Up ||
                 CynNeedHome[i]==C_Empty_Middle))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[2]==true &&
                (CynNeedHome[i]==C_Color_Up ||
                 CynNeedHome[i]==C_Color_Middle))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[3]==true &&
                (CynNeedHome[i]==C_Auto1_Up ||
                 CynNeedHome[i]==C_Auto1_Selector))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[4]==true &&
                (CynNeedHome[i]==C_Auto2_Up ||
                 CynNeedHome[i]==C_Auto2_Selector))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[5]==true &&
                (CynNeedHome[i]==C_Auto3_Up ||
                 CynNeedHome[i]==C_Auto3_Selector))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[6]==true &&
                (CynNeedHome[i]==C_Auto4_Up ||
                 CynNeedHome[i]==C_Auto4_Selector))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[7]==true &&
                (CynNeedHome[i]==C_Auto5_Up ||
                 CynNeedHome[i]==C_Auto5_Selector))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(LOAD_Z_USE_MOTOR[8]==true &&
                (CynNeedHome[i]==C_Auto6_Up ||
                 CynNeedHome[i]==C_Auto6_Selector))
        {
            CylinderInitOKFlag[i]=true;
        }
        else if(AUTO3_IS_MAGAZINE==1 &&                                         //JerryYang 20220909 : add magazine
               (CynNeedHome[i]==C_Auto3_Selector  ||
                CynNeedHome[i]==C_Auto3Side_Fixer ||
                CynNeedHome[i]==C_Auto3_Up        ||
                CynNeedHome[i]==C_Auto3LoaderZ_Select))
        {
            CylinderInitOKFlag[i]=true;
        }
        else
        {
            if(Cylinder[CynNeedHome[i]].Enable==true)                           //Steven 20240123 : 改用enable確認氣缸是否要復歸
            {
                CylinderInitOKFlag[i]=false;
            }
            else
            {
                CylinderInitOKFlag[i]=true;
            }
        }

        if(CynNeedHome[i]==C_Auto1Separate  ||
           CynNeedHome[i]==C_Auto2Separate  ||                                  //Steven 20240131 : 歸零的時候要常態Push
           CynNeedHome[i]==C_Auto3Separate  ||                                  //ChungHung 20140701 add Auto Retest
           CynNeedHome[i]==C_LoadCarRFIDRotArmU ||                              //RogerYang 20250828 add for Loader Rotate Arm
           CynNeedHome[i]==C_LoadTrayDetU       ||                              //RogerYang 20250828 add for 殘料檢氣缸
           CynNeedHome[i]==C_LoadTrayDetB       ||                              //RogerYang 20250828 add for 殘料檢氣缸
           (Cylinder[C_TrayXFloodgate1].Enable==true && CynNeedHome[i]==C_TrayXFloodgate1) ||                           //Ztex 2024.04.03 Add Ready Home
           (Cylinder[C_TrayXFloodgate2].Enable==true && CynNeedHome[i]==C_TrayXFloodgate2) ||
           (Cylinder[C_TrayXFloodgate3].Enable==true && CynNeedHome[i]==C_TrayXFloodgate3) ||
           (Cylinder[C_TrayXFloodgate4].Enable==true && CynNeedHome[i]==C_TrayXFloodgate4))
        {
            CylinderInitState[i]=true;                                          //true : Push()
        }
        else
        {
            if(CynNeedHome[i]==C_LoadCarRFIDRotArmD  ||                         //RogerYang 20250909 : true: Push/Pop  false On/Off
               CynNeedHome[i]==C_LoadTrayDetD        ||
               CynNeedHome[i]==C_LoadTrayDetF)
            {
                CylinderInitSwitchType[i]=false;
            }
            CylinderInitState[i]=false;                                         //false : Pop()
        }

        if(MachineTypeChoice==Type_HT1032)                                      //RogerYang 20250523 會影響到9045的AutoZ氣缸(push會往上升)
        {
            if(Enable_PLCSafety_IO==1 &&                                        //Ztex 2024.05.07 Add PLC Safe Door
              (CynNeedHome[i]==C_SafeDoor1Lock ||
               CynNeedHome[i]==C_SafeDoor2Lock ||
               CynNeedHome[i]==C_SafeDoor3Lock ||
               CynNeedHome[i]==C_SafeDoor4Lock ||
               CynNeedHome[i]==C_SafeDoor6Lock ||
               CynNeedHome[i]==C_SafeDoor7Lock ||
               CynNeedHome[i]==C_SafeDoor8Lock
              ))
            {
                CylinderInitState[i]=false;                                     //false : Pop()
            }
            else
            {
                CylinderInitState[i]=true;                                      //true : Push()
            }
        }

        if(CynNeedHome[i]==C_TrayZ_Selector      ||
           CynNeedHome[i]==C_EmptyLoaderZ_Select ||
           CynNeedHome[i]==C_ColorLoaderZ_Select ||
           CynNeedHome[i]==C_LoaderSeparate      ||
           CynNeedHome[i]==C_EmptySeparate       ||
           CynNeedHome[i]==C_ColorSeparate       ||
           CynNeedHome[i]==C_Auto1Separate       ||
           CynNeedHome[i]==C_Auto2Separate       ||
           CynNeedHome[i]==C_Auto3Separate       ||
           CynNeedHome[i]==C_LoaderEdgePush      ||
           CynNeedHome[i]==C_EmptyEdgePush       ||
           CynNeedHome[i]==C_ColorEdgePush       ||
           CynNeedHome[i]==C_Auto1EdgePush       ||
           CynNeedHome[i]==C_Auto2EdgePush       ||
           CynNeedHome[i]==C_Auto3EdgePush       ||
           CynNeedHome[i]==C_Auto1_Selector      ||
           CynNeedHome[i]==C_Auto2_Selector      ||
           CynNeedHome[i]==C_Auto3_Selector
          )
        {
            CylinderInitOKFlag[i]=true;
        }
    }
}
//---------------------------------------------------------------------------
//void SetCylinderResetStateHomeAfter()                                         //Steven 20240123 : 完全不會執行, mark
//{
//    for(int i=0; i<CynForHome; i++)
//        CylinderInitOKFlag[i]=true;
//}
//------------------------------------------------------------------------------
TQPF_Timer ResetOKDeleyTime;
//void WritePanasonicParameter(int Index, unsigned char Command, unsigned Data);
//void ReadPanasonicParameter(int index, unsigned char Command);
void SetHomeSerial(int i)
{
    char str[256];
    fHome->ShowLed(i, 3);
    sprintf(str, "M%2d homeing ....", i+1);
    fHome->ListBox1->Items->Insert(0, str);
}
//------------------------------------------------------------------------------
TQPF_Timer CynPushDeleyTime[MAX_TRACK];
int PrePushLifterCylinderTask[MAX_TRACK]={1};
bool bLoaderUpStatus[MAX_TRACK]={false};
//------------------------------------------------------------------------------
bool PrePushLifterCylinder(int index)                                           //Steven 20240109 : 預先打兩下汽缸
{
    bool ret=false;
    int &Task=PrePushLifterCylinderTask[index];

    if(Prod.iTrayType[index]==tNotUse)
        return true;

    switch(Task)
    {
        case 1:
            bLoaderUpStatus[index]=(Cylinder[iC_Up[index]].OnSensor() ||
                                    Cylinder[iC_Middle[index]].OnSensor());
            Task=100;
        case 100:
            if(bLoaderUpStatus[index]==false)
            {
                Cylinder[iC_Up[index]].On();
            }

            Cylinder[iC_EdgePush[index]].On();
            CynPushDeleyTime[index].SetMSAndOn(1000);
            Task=200;
            break;
        case 200:
            if(CynPushDeleyTime[index].Off())
            {
                if(bLoaderUpStatus[index]==false)
                {
                    Cylinder[iC_Up[index]].Off();
                }

                Cylinder[iC_EdgePush[index]].Off();
                CynPushDeleyTime[index].SetMSAndOn(1000);
                Task=300;
            }
            break;
        case 300:
            if(CynPushDeleyTime[index].Off())
            {
                if(bLoaderUpStatus[index]==false)
                {
                    Cylinder[iC_Up[index]].On();
                }

                Cylinder[iC_EdgePush[index]].On();
                CynPushDeleyTime[index].SetMSAndOn(1000);
                Task=400;
            }
            break;
        case 400:
            if(CynPushDeleyTime[index].Off())
            {
                if(bLoaderUpStatus[index]==false)
                {
                    Cylinder[iC_Up[index]].Off();
                }

                Cylinder[iC_EdgePush[index]].Off();
                CynPushDeleyTime[index].SetMSAndOn(1000);
                Task=500;
            }
            break;
        case 500:
            if(CynPushDeleyTime[index].Off())
            {
                ret=true;
            }
            break;
    }
    return ret;
}
//------------------------------------------------------------------------------
bool PrePushLoaderCylinder(bool bReset)                                         //Steven 20150429 : 預先打兩下Loader汽缸
{
    bool ret=true;

    if(bReset)
    {
        for(int i=0; i<MAX_TRACK; i++)
        {
            PrePushLifterCylinderTask[i]=1;
            bLoaderUpStatus[i]=false;
        }
        return ret;
    }

    if(IniConfig.bP05_LoaderCylinderPreOn==false)
    {
        return ret;
    }

    for(int i=0; i<MAX_TRACK; i++)
    {
        if(AUTO_EMPTY_COLOR==0)                                                 //使用四軌
        {
            if(i==etEmpty || i==etColor)
                continue;
        }

        if(PrePushLifterCylinder(i)==false)
            ret=false;
    }

    return ret;
}
//------------------------------------------------------------------------------
int iTopBtmHome=0;
TQPF_Timer tTopBtmDelay;
//------------------------------------------------------------------------------
bool DoTopBtmHome(bool bFirst)                                                  //Jimmychiu 20240322 : Top & Bottom Inspect
{
    if(bFirst==true)
    {
        iTopBtmHome=1;
        return true;
    }
    else
    {
        //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TOPBTMAOI)
        // 缺相依：`TFrmAOI::ttbInsp`（golden fAOI.h 的 Top/Bottom 檢測分頁物件）
        //   與 `TFrmAOI::GetRotateIs0()`。本樹的 `TFrmAOI`（forms/fAOI.h:106）
        //   是門面，這兩個成員都沒有 —— 整個 Top/Bottom AOI 模組沒翻。
        //
        // 🔴 行為（這一格是三格裡最重的，寫清楚）：
        //   在 `USE_Scanner_AOI_Inspection==eBtnAOI_TopBottomInstall` 的機台上，
        //   歸零會**停在 iHomeStep=305 不動** —— 因為 case 305 是
        //   `if(DoTopBtmHome(false)) iHomeStep=310;`，而閘住之後它永遠回 false。
        //   ⇒ 那種機台按 START 之後歸零不會完成。
        //   **這不是「少做一點事」，是擋住整條線**，所以標 🔴。
        //
        //   ⚠ 但**其他機台不受影響**：case 300 只有在上面那個組態成立時才
        //   跳 305，否則直接跳 310。這台筆電的組態不是 TopBottomInstall。
        //
        // 為什麼不改成 `return true`：那會宣稱「Top/Bottom 機構已經歸好位」，
        //   而它一根都沒動。假成功比卡住危險得多 —— 卡住看得見，假成功看不見。
        //
        // UN-GATE：等 `TFrmAOI` 帶進 `ttbInsp`（DoLockRotate / DoRotate /
        //   DoMoveFixedSeatXY2RotatePos / DoCCDLightDown）與 `GetRotateIs0()`。
#if 0 // GATE (W906-HOME-C2-TOPBTMAOI): 缺相依，見上面的就地註解
        int &Task=iTopBtmHome;
        switch(Task)
        {
            case 1:  //check safe status
                FrmAOI->ttbInsp->DoLockRotate(true);
                SW[SwCCDZBreaker].On();  //Jimmychiu 20240322 : Top & Bottom Inspect
                if(FrmAOI->GetRotateIs0()==false)
                {
                    ShowMyMessage("Please manually rotate the platform to the origin.");
                    return false;
                }
//                iSingleMotorHomeTask[MTopAOIArmX]=1;
//                iSingleMotorHomeTask[MTopAOIArmY]=1;
//                iSingleMotorHomeTask[MTopAOIArmR]=1;
                iSingleMotorHomeTask[MTopAOICCDZ]=1;
                Task=100;
                break;
            case 100://XYZ home
                {
                    bool bZMove=ProcessSingleMotorHome(MTopAOICCDZ);
                    if(bZMove)
                    {
                        bZMove=false;
                        if(MOT[MTopAOICCDZ].Led[iHomeLed])
                        {
//                            fHome->HomeClass[MTopAOIArmX]->THomeFlag=0;
//                            fHome->HomeClass[MTopAOIArmY]->THomeFlag=0;
//                            fHome->HomeClass[MTopAOIArmR]->THomeFlag=0;
                            fHome->HomeClass[MTopAOICCDZ]->THomeFlag=0;
                            fHome->ShowLed(MTopAOICCDZ, 1);
                            return true;
                            Task=200;
                        }
                        else
                        {
                            iSingleMotorHomeTask[MTopAOICCDZ]=1;
                            Task=100;
                        }
                    }
                }
                break;
            case 200:     //move Y
                {
                    bool bZMove=ProcessSingleMotorHome(MTopAOIArmY);
                    if(bZMove)
                    {
                        bZMove=false;
                        if(MOT[MTopAOIArmY].Led[iHomeLed])
                        {
                            Task=300;
                        }
                        else
                        {
                            iSingleMotorHomeTask[MTopAOIArmY]=1;
                            Task=200;
                        }
                    }
                }
                break;
            case 300:
                {
                    bool bZMove=ProcessSingleMotorHome(MTopAOIArmX);
                    if(bZMove)
                    {
                        bZMove=false;
                        if(MOT[MTopAOIArmX].Led[iHomeLed])
                        {
                            Task=400;
                        }
                        else
                        {
                            iSingleMotorHomeTask[MTopAOIArmX]=1;
                            Task=300;
                        }
                    }
                }
                break;
            case 400://move XY
                if(FrmAOI->ttbInsp->DoMoveFixedSeatXY2RotatePos())
                {
                    FrmAOI->ttbInsp->DoLockRotate(false);
                    iSingleMotorHomeTask[MTopAOIArmR]=1;
                    Task=500;
                }
                break;
            case 500://rotate home
                {
                    bool bZMove=ProcessSingleMotorHome(MTopAOIArmR);
                    if(bZMove)
                    {
                        bZMove=false;
                        if(MOT[MTopAOIArmR].Motor->Enable==true)
                        {
                            if(MOT[MTopAOIArmR].Led[iHomeLed])
                            {
                                FrmAOI->ttbInsp->DoRotate(0, true);
                                Task=600;
                            }
                            else
                            {
                                ShowMyMessage("AOI rotate not at Home");
                            }
                        }
                        else
                        {
                            Task=900;
                        }
                    }
                }
                break;
            case 600:  //rotate 0
                if(FrmAOI->ttbInsp->DoRotate(0))
                {
                    Task=900;
                    FrmAOI->ttbInsp->DoLockRotate(true);
                }
                break;
            case 900:  //finish
                fHome->HomeClass[MTopAOIArmX]->THomeFlag=0;
                fHome->HomeClass[MTopAOIArmY]->THomeFlag=0;
                fHome->HomeClass[MTopAOIArmR]->THomeFlag=0;
                fHome->HomeClass[MTopAOICCDZ]->THomeFlag=0;
                SW[SwLightOrg].OnOff(true);
                tTopBtmDelay.SetSecAndOn(20);                                   //Eastsun 20260302 : 新增Time out
                Task=1000;
                break;
            case 1000:
                if(USE_Scanner_AOI_Inspection_FixLight_Z_Axis==1)               //Eastsun 20260410 : AOI Fix Light Z Axis bypass
                    return true;
                #ifdef SOFT_SIMULTE
                return true;
                #endif                                                    //無電動缸直接Pass

                if(Sen[SnLightZINP].IsOff())
                {
                    SW[SwLightOrg].OnOff(false);
                    tTopBtmDelay.SetSecAndOn(20);                               //Eastsun 20260302 : 新增Time out
                    Task=1100;
                }

                if(tTopBtmDelay.Off())                                          //Eastsun 20260302 : 新增Time out
                {
                    ShowMyMessage("Sensor SnLightZINP Off TimeOut");
                }
                break;
            case 1100:
                if(Sen[SnLightZORG].IsOff()==false)
                {
                    SW[SwLightOrg].OnOff(true);
                    return true;
                }

                 if(tTopBtmDelay.Off())                                          //Eastsun 20260302 : 新增Time out
                {
                    ShowMyMessage("Sensor SnLightZORG Off TimeOut");
                }
                break;
        }
#endif // GATE (W906-HOME-C2-TOPBTMAOI)
    }
    return false;
}
//------------------------------------------------------------------------------
extern bool bPlaceToShuttleFirst;
extern bool bPickFromHotplate;
extern bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY);
TQPF_Timer hRealCCDTimeOut;
TQPF_Timer hHomeDelay;

// ===========================================================================
//  TfHome::Show / Close / RotateCheckClear
//  -- golden uhome.cpp:4844 (FormShow) / :4868 (FormClose) / :4851
//
//  ★ **Show/Close 是承重的，不是版面。** golden 的歸零狀態機有 16 處問
//  `fHome->fShow`，其中 case 600（本檔真正驅動馬達的那個迴圈）第一行就是
//      if(fHome->fShow==false) { SoftStop=true; break; }
//  —— `fShow` 是 false 的話整個歸零一步都跑不動，而且會 SoftStop。
//
//  golden 用 VCL 事件維護它：`fHome->Show()` 觸發 `FormShow`，
//  `fHome->Close()` 觸發 `FormClose`。本樹沒有事件迴圈，所以把那兩個
//  事件處理器的**本體**直接寫進同名方法裡。逐行對應，沒有多也沒有少。
// ===========================================================================
void TfHome::Show()                                                             // golden TForm::Show -> TfHome::FormShow (uhome.cpp:4844-4849)
{
    //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-PANEL2)
    // 缺相依：`TfHome::Panel2`（golden uhome.h 的 .dfm TPanel）。
    // 行為：歸零畫面上那塊面板不會被藏起來。純版面，零控制流。
    // UN-GATE：等 TfHome 門面帶進 Panel2（或 web HMI 有對應的區塊）。
#if 0 // GATE (W906-HOME-C2-PANEL2): 缺相依，見上面的就地註解
    Panel2->Visible=false;                                                      // golden :4846
#endif // GATE (W906-HOME-C2-PANEL2)
    fShow =true;  if (W906_FormProgramShowHook) W906_FormProgramShowHook("fHome", true, "uhome.cpp:646 TfHome::Show (golden uhome.cpp:4847)");  /*AI(W906-PAGETAB-Q51) 20260928 [W906] 程式開畫面 ⇒ 頁面表（每個 HMI 自動跳出 Home Monitor，Steven Q-P2=A）；hook 是 0（ctest）＝不做*/                                                                // golden :4847
    fAbort=false;                                                               // golden :4848
}

void TfHome::Close()                                                            // golden TForm::Close -> TfHome::FormClose (uhome.cpp:4868-4872)
{
    fShow=false;  if (W906_FormProgramShowHook) W906_FormProgramShowHook("fHome", false, "uhome.cpp:652 TfHome::Close (golden uhome.cpp:4870)");  /*AI(W906-PAGETAB-Q51) 20260928 [W906] 程式關畫面 ⇒ 頁面表（每個 HMI 自動關掉 Home Monitor）*/                                                                // golden :4870
    //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TIMER1)
    // 缺相依：`TfHome::Timer1`（golden uhome.h 的 .dfm TTimer，只驅動
    //   `TfHome::ScanKey()` —— 掃歸零畫面上的面板按鍵）。
    // 行為：本樹沒有面板按鍵掃描，所以沒有計時器可以關。
    //   ⚠ golden 關它是為了停掉 ScanKey；本樹**從頭到尾就沒有 ScanKey**，
    //   所以這一格不是「少做一件事」，是「那件事在本樹不存在」。
    // UN-GATE：等面板按鍵掃描接進來（那時 Timer1 與 ScanKey 要一起翻）。
#if 0 // GATE (W906-HOME-C2-TIMER1): 缺相依，見上面的就地註解
    Timer1->Enabled=false;                                                      // golden :4871
#endif // GATE (W906-HOME-C2-TIMER1)
}

// ---------------------------------------------------------------------------
//  TfHome::RotateCheckClear -- golden uhome.cpp:4851-4861.  **完整翻譯，零閘。**
//  它的四個相依（IniConfig / TestIF / SW[] / MySleep）在本樹都在而且是活的。
// ---------------------------------------------------------------------------
void TfHome::RotateCheckClear()
{
    //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
    if(IniConfig.bHaveRotateShuttle==true && IniConfig.bRotateShNeedSensorCheck && TestIF.bRotateShuttle)
    {
        //清除SnRotateCheck
        SW[SwRotateCheckClear].On();
        MySleep(100);
        SW[SwRotateCheckClear].Off();
    }
}

// ---------------------------------------------------------------------------
//  TfHome::InitGali_HomeTask -- golden uhome.cpp:646-662.  **完整翻譯，零閘。**
//  AI(W906-MT-E3b) 20260925: 本檔檔頭原本把它列為「Galil 專屬，本機沒有卡」而不翻。
//    量過本體：它只寫記憶體旗標（IndexZCanMove[2]、Index 四軸的 MovFlag／bScanFlag／GaliSofDelayCount），
//    **不下任何 Galil 命令**，四個相依在本樹都在而且是活的（IndexZCanMove 在本檔 :123 點名 extern；
//    其餘三個是 TMyMotor 成員，Motor/mymotor.h:178/:232/:233）。GaliMotorServoOff（本檔檔尾）第二行就叫它。
// ---------------------------------------------------------------------------
void TfHome::InitGali_HomeTask()
{
    IndexZCanMove[0]=true;
    IndexZCanMove[1]=true;
    MOT[MTestY1].MovFlag=false;
    MOT[MTestY2].MovFlag=false;
    MOT[MTestZ1].MovFlag=false;
    MOT[MTestZ2].MovFlag=false;
    MOT[MTestY1].bScanFlag=false;
    MOT[MTestY2].bScanFlag=false;
    MOT[MTestZ1].bScanFlag=false;
    MOT[MTestZ2].bScanFlag=false;
    MOT[MTestY1].GaliSofDelayCount=0;
    MOT[MTestY2].GaliSofDelayCount=0;
    MOT[MTestZ1].GaliSofDelayCount=0;
    MOT[MTestZ2].GaliSofDelayCount=0;
}

bool ProcessMotorHome(bool Flag2)
{
#ifndef SOFT_SIMULTE
    int iTrayArmPos;
#endif
    int ibuffer=0;
    int ret, iRef, iCT;
    int iPos = 33736;
    int iPitch_Move[4]={0,0,0,0};                                               //Ztex 2023.12.15 Add Pitch X Home Twice
    int Ang45=0, Ang90=0, buffer=0;                                             //kevin 20131003 讀取馬達轉90度pluse
    char str[256];
    bool bSensor=false;
    bool bATCSiteTest[32];                                                      //Ifor 20160510 add ATC Test Site
    AnsiString sBuffer;
    AnsiString strxxx="", Str;

    static int iInRotateCount=1;
    static int iOutRotateCount=1;
    static int Index_Z1_FindZPosition=0;                                        //Isaac 20201110 : Index Y find motor phase
    static int Index_Z2_FindZPosition=0;
    static int Index_Y1_FindZPosition=0;
    static int Index_Y2_FindZPosition=0;
    static bool flag1, flag2, flag3, flag4, flag5, flag6, flag[MAX_ARM_Row][MAX_ARM_Col], flag11;
    static bool flag7 ,flag8;                                                   //2013-04-12    Dell :旋轉站;馬達版
    static bool flag9, flag10;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
    static bool flag12, flag13;                                                 //Isaac 20201110 : Index Y find motor phase
    static bool flag14[4]={false, false, false, false};                         //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    static bool bCyflag[4];
    static bool bCheckInArmX=false;
    static bool bNeedPutBack=false;
    static bool IndexY1=false, IndexZ1=false, IndexY2=false, IndexZ2=false;     //kevin 20161214 (Steven) Increase Hone Speed
    static bool bflag[MAX_AUTO_TRAY]={false, false, false, false, false, false};                                        //kevin 20180726 Auto 12 3 up
    static bool bFindMotorPhaseEveryGoHomeProcess=false;
    static bool bPitchHome_Twice=false;                                         //Ztex 2023.12.15 Add Pitch X Home Twice
    static bool bPitchHome[2]={false,false};                                    //Ztex 2023.12.15 Add Pitch X Home Twice
    static bool bOCRHomeActionPass=false;                                       //Frank 20250214 add
    static bool bIsYCarMoving=false;                                            //RogerYang 20251029 : 修正Loader Y motor判斷有無tray可能被機構誤觸發
    static bool flag18[3]={false, false, false};                                //Ifor 20251220 add:Boat Z

    // -----------------------------------------------------------------------
    //  AI(W906-HOME-C2) 20260920: 上面這些 golden 區域變數在**這個組態**下
    //  「寫了沒人讀」。宣告與賦值一律保留（golden 原文，解閘時原地就能用），
    //  這裡只用 (void) 讓 -Wunused-but-set-variable 安靜。
    //
    //  ⚠ 逐個說明，因為「沒人讀」有三種完全不同的原因，混在一起就看不出來：
    //
    //   (A) 讀取點被本檔的閘吃掉了 —— 解閘就會回來
    //        iPitch_Move   -> GATE (W906-HOME-C2-PITCHX2)
    //        bATCSiteTest  -> GATE (W906-HOME-C2-ATCTESTEND)
    //
    //   (B) 讀取點在 `#ifdef DEBUG_HOME` 裡，而本樹沒有定義那個巨集
    //        IndexY1 / IndexZ1 / IndexY2 / IndexZ2
    //        （golden 用它們做「快速回 Home」的 Galil 單軸捷徑）
    //
    //   (C) ★ **golden 自己就沒有讀它** —— 不是移植造成的
    //        iPos            golden 只有 `iPos+=5000;`（uhome.cpp 全檔唯一一處）
    //        bCyflag         golden 的讀取點自己被註解掉了
    //                        （`// if(bCyflag[0]==false)` 那一段）
    //        iInRotateCount / iOutRotateCount  只在 case 1 各寫一次 =1
    //        bNeedPutBack    只在 case 1 寫一次 =false
    //        bIsYCarMoving   寫了四次，讀取點在 golden 的 Loader Y 臂裡，
    //                        而那一臂落在 `#ifndef SOFT_SIMULTE`（本組態關著）
    //        ⇒ 這一組**不可以「順手清掉」** —— 它們是 golden 的原文，
    //          而且 (C) 與 (A)/(B) 的解法完全不同：(A)/(B) 會自己回來，
    //          (C) 要的是回 golden 問「本來想做什麼」。
    // -----------------------------------------------------------------------
    (void)iPos; (void)iPitch_Move; (void)bATCSiteTest;
    (void)IndexY1; (void)IndexZ1; (void)IndexY2; (void)IndexZ2;
    (void)bCyflag; (void)iInRotateCount; (void)iOutRotateCount;
    (void)bNeedPutBack; (void)bIsYCarMoving;

    // -----------------------------------------------------------------------
    //  ★ AI(W906-HOME-C2) 20260920: 機台沒 bring-up 就不要跑歸零
    //
    //  golden 不需要這一條：`InitialMotorParameter()`（cinitial.cpp:3739）
    //  在啟動時無條件建好 164 顆 `HTMotor`，所以 `MOT[i].Motor` 永遠非 NULL，
    //  `fHome->HomeClass` 也永遠有 164 列。
    //
    //  **本樹不是那樣**：它允許「沒 bring-up」的行程存在（單元測試、
    //  wb_publish…）。而本函式有 **20 處 `MOT[...].Motor->`** 與整個
    //  case 600 的 `fHome->HomeClass[i]`。
    //
    //  實測（20260920，退掉假 seam 之後）：`tests/test_w6_6_hub` 直接 SIGSEGV
    //      MainProc -> W906_MainProcHomeDispatch -> DoHomeProcess
    //               -> ProcessMotorHome -> `MOT[i].Motor->Enable`   <- NULL
    //
    //  ⇒ 與其在 20 個地方偏離 golden，在**入口問一次**。
    //
    //  ⚠ 回 `false` 的語意是「歸零序列這一 tick 還沒結束」——
    //    與 golden 每個未完成 tick 的回傳值相同，**不會假裝歸零成功**。
    //    （回 true 才會，那正是 20260920 上午 Q13 假 seam 的問題。）
    //
    //  ⚠ 它在真實機台上**永遠不成立**，所以不是「怕機台會動」的閘 ——
    //    是把一個本樹特有的未定義行為變成有定義的拒絕。
    // -----------------------------------------------------------------------
    //  ⚠⚠ 要掃**每一顆**，不能只問一顆。
    //    第一版只問 `MOT[MInArmX].Motor == NULL`，結果 test_w6_6_hub 照樣
    //    SIGSEGV（`mov (%eax),%eax` with eax==0）—— 因為那個行程**有些**
    //    馬達掛了 backend、有些沒有。golden 的不變式是「每一顆都有」，
    //    所以守衛也要問「每一顆都有」。
    //    memory: unreachable-claim-must-test-the-same-expression ——
    //    要量的是 case 1 真正會碰的那個運算式（`MOT[i].Motor`，i 走完全部）。
    {
        int nNullMotor = 0;
        for(int i=0; i<TOTAL_MOTOR; i++)
            if(MOT[i].Motor == NULL) ++nNullMotor;

        if(nNullMotor > 0 || fHome->HomeClass.size() < (unsigned)TOTAL_MOTOR)
        {
            static bool s_noBringUpWarned = false;
            if(!s_noBringUpWarned)
            {
                s_noBringUpWarned = true;
                std::printf("ProcessMotorHome: refusing -- machine not brought up "
                            "(%d/%d motors have no backend, HomeClass=%u/%d). "
                            "InitialMotorParameter() has not run in this process.\n",
                            nNullMotor, TOTAL_MOTOR,
                            (unsigned)fHome->HomeClass.size(), TOTAL_MOTOR);
                std::fflush(stdout);
            }
            return false;
        }
    }

    if(fHome->fAbort)
    {
        fHome->fAbort=false;
        SoftStop=true;
        fHome->iHomeStep=1;
        //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-HOMELOG)
        // 缺相依：`HomeLog(AnsiString)` 的**定義**在本樹被閘住 ——
        //   cpublic.cpp:746 的本體整個落在 `#if 0 // TODO(GA1-B3)` 裡，
        //   阻塞原因是它第一行就寫 `fMain->MemoHome->Lines->Add(...)`，
        //   而 TfMain 門面沒有 MemoHome（與本檔的 MEMOHOME 閘同一個缺口）。
        //   宣告還在（cpublic.h:40），所以編得過、連不起來 ——
        //   memory: symbol-exists-has-three-strengths 的第一級 vs 第二級。
        // 行為：歸零的逐步流程紀錄不會寫進 `D:\HT9045_Log\HomeLog\Home_Home.logs`。
        //   ⚠ 那個檔是事後追「歸零卡在第幾步」的主要證據來源。
        //   本樹的替代品是 `fHome->ListBox1`（同樣逐步記，而且是活的），
        //   只是它在記憶體裡、不落盤。
        // UN-GATE：TfMain 門面帶進 MemoHome（會同時解掉 cpublic.cpp 那 7 支 log 中的
        //   HomeLog 與 ProductionLog 兩支）。
        #if 0 // GATE (W906-HOME-C2-HOMELOG): 缺相依，見上面的就地註解
        HomeLog("fHome->fAbort");                                               //Kevin  20110525
        #endif // GATE (W906-HOME-C2-HOMELOG)
        return true;
    }
    try
    {
        sBuffer.sprintf("%d", fHome->iHomeStep);
        //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-HOMELOG)
        // 缺相依：`HomeLog(AnsiString)` 的**定義**在本樹被閘住 ——
        //   cpublic.cpp:746 的本體整個落在 `#if 0 // TODO(GA1-B3)` 裡，
        //   阻塞原因是它第一行就寫 `fMain->MemoHome->Lines->Add(...)`，
        //   而 TfMain 門面沒有 MemoHome（與本檔的 MEMOHOME 閘同一個缺口）。
        //   宣告還在（cpublic.h:40），所以編得過、連不起來 ——
        //   memory: symbol-exists-has-three-strengths 的第一級 vs 第二級。
        // 行為：歸零的逐步流程紀錄不會寫進 `D:\HT9045_Log\HomeLog\Home_Home.logs`。
        //   ⚠ 那個檔是事後追「歸零卡在第幾步」的主要證據來源。
        //   本樹的替代品是 `fHome->ListBox1`（同樣逐步記，而且是活的），
        //   只是它在記憶體裡、不落盤。
        // UN-GATE：TfMain 門面帶進 MemoHome（會同時解掉 cpublic.cpp 那 7 支 log 中的
        //   HomeLog 與 ProductionLog 兩支）。
        #if 0 // GATE (W906-HOME-C2-HOMELOG): 缺相依，見上面的就地註解
        HomeLog(sBuffer);                                                       //Kevin  20110525
        #endif // GATE (W906-HOME-C2-HOMELOG)
    }
    catch(...)
    {
        MyDBIProcess("Exception", "ProcessMotorHome");
        //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-MEMOHOME)
        // 缺相依：`TfMain::MemoHome`（golden main.h 的歸零訊息 TMemo）。
        // 本樹的 TfMain 門面沒有這個欄位。
        // 行為：`iHomeStep` 變成非整數時（golden 認為可能發生，所以包了 try/catch）
        //   畫面上不會多一行 "fHome->iHomeStep != int"。
        //   ⚠ **同一個 catch 裡的 `MyDBIProcess` 照留**，所以這件事**仍然會進資料庫**，
        //   只是不會出現在畫面備忘欄。⇒ 沒有任何診斷資訊消失。
        // UN-GATE：等 TfMain 門面帶進 MemoHome（或 web HMI 有對應的訊息面板）。
        #if 0 // GATE (W906-HOME-C2-MEMOHOME): 缺相依，見上面的就地註解
        fMain->MemoHome->Lines->Add("fHome->iHomeStep != int");
        #endif // GATE (W906-HOME-C2-MEMOHOME)
    }
    #ifdef DEBUG_HOME                                                           //wei (Steven) 20170317 快速回Home開關

    if(fHome->iHomeStep>=650 &&                                                 //kevin 20161214 (Steven) Increase Hone Speed
       fHome->iHomeStep<=1250 &&
       (IndexY1==false || IndexZ1==false || IndexY2==false || IndexZ2==false))
    {
        if(IndexY1==false)
        {
            if(MOT[MTestY1].Gali_SingalHome())
            {
                IndexY1=true;
                fHome->ListBox1->Items->Insert(0, "Index Y1 home finish.");
            }
        }

        if(IndexY2==false)
        {
            if(MOT[MTestY2].Gali_SingalHome())
            {
                IndexY2=true;
                fHome->ListBox1->Items->Insert(0, "Index Y2 home finish.");
            }
        }

        if(IndexZ1==false)
        {
            if(MOT[MTestZ1].Gali_SingalHome())
            {
                IndexZ1=true;
                fHome->ListBox1->Items->Insert(0, "Index Z1 home finish.");
            }
        }

        if(IndexZ2==false)
        {
            if(MOT[MTestZ2].Gali_SingalHome())
            {
                IndexZ2=true;
                fHome->ListBox1->Items->Insert(0, "Index Z2 home finish.");
            }
        }
    }
    #endif

    if(IniConfig.bP35TrayArm && bPushHomeDetect)                                //kevin 20171006 (wei) tray arm home 需遮住sensor
    {
        if(Sen[SnTrayArmSafePos].IsOff())
        {
            ShowMyMessage("Please move tray arm on safe sensor");
            return false;
        }
    }
    bPushHomeDetect=false;

    switch(fHome->iHomeStep)
    {
        case 1:
            iReadCIDAction=ePortTotal;
            iMoveToShuttle=0;
            bIsYCarMoving=false;                                                //RogerYang 20251029 : 修正Loader Y motor判斷有無tray可能被機構誤觸發

            for(int i=0; i<TOTAL_MOTOR; i++)                                    //JerryYang 20250429 : 進入HOME流程強制清除home旗標, 避免沒做完home又按start會接著繼續跑
            {
                MOT[i].MotorInitial();
                MOT[i].HomeFlag=0;
                fHome->HomeClass[i]->THomeFlag=1;
                if(MOT[i].Motor->Enable==false)
                {
                    fHome->HomeClass[i]->THomeFlag=0;
                    MOT[i].HomeFlag=1;
                }
            }

            #ifndef SOFT_SIMULTE
            if(CosFunction.bUSEJCETSiteMapMode &&
               IniConfig.bI21EnableASM==true &&
               InArmSiteMapData.iP!=-1)                                         //Steven 20211209 : 紀錄Site map資料
            {
                Str.sprintf("%d, %d, %d, %d, %s, ASM_Home", InArmSiteMapData.iP, InArmSiteMapData.iPlateR, InArmSiteMapData.iPlateC, InArmSiteMapData.iSuckR, InArmSuck.Suck[InArmSiteMapData.iSuckR][InArmSiteMapData.iSuckC].sName);
                fMain->slAutoSiteMapLog->AddTextWithDateTime(Str);              //Steven 20211209 : 紀錄Auto Site Map動作
                RecordProcess(Str);
                fMain->DoStateRecord(0, false);                                 //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)
            }
            #endif

            if(SW[SwAirOff].Enable==true)                                       //Steven 20221215 : Power saving for vacuum pump
                SW[SwAirOff].On();

            HPPlaceLog.InitialPosition();                                       //Steven 20211110 : 記錄放料到加熱盤的位置
            bUnloading=false;                                                   //JerryYang 20250227 : fix
            if(CheckNozzleEventFinish()==false)                                 //Steven 20210825 : 吹氣完成才可以歸零
            {
                return false;
            }

            bSuckingFlagZ1=true;                                                //Steven 20240916 : index下降到shuttle吸放料
            bSuckingFlagZ2=true;
            InitDoInArmAdditionalFunction();
            InitDoOutArmAdditionalFunction();
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TRAYSTEP)
            // 缺相依：`dmTrayMotor`（golden Motor/TrayStepMotor.h 的 TTrayStepMotor 實例）。
            // 本樹沒有這個全域 —— `cSpeed.cpp:29/:68` 的 GATE (S5) 就是為了同一個理由存在的，
            // 不是本波新發現。
            // 行為：Loader 入 Tray 的步進馬達**不會在歸零時重寫速度表**。
            //   golden 的註解說這一步是「換位置寫入速度, 避免動作異常」——
            //   ⇒ 在用步進馬達送 Tray 的機台上，歸零後那顆馬達沿用上一次的速度設定。
            //   沒有步進 Tray 馬達的機台（含這台筆電）完全不受影響。
            // UN-GATE：等 Motor/TrayStepMotor 落地並建立 dmTrayMotor 全域（與 cSpeed.cpp
            //   的 GATE (S5) 同一個條件，兩處要一起解）。
            #if 0 // GATE (W906-HOME-C2-TRAYSTEP): 缺相依，見上面的就地註解
            dmTrayMotor->StartSetSpeed();                                       //Steven 20210318 : Loader入Tray改步進, 換位置寫入速度, 避免動作異常
            #endif // GATE (W906-HOME-C2-TRAYSTEP)

            bCatchTrayFinishAction=false;                                       //JerryYang 20241118 : fix
            InitialIndexSocketCheckTask();
            bNeedPutBack=false;
            bDoingF16=false;                                                    //Steven 20221213 : 確認shuttle 有沒有斷線

            if(asMotorDatabaseErr!="")                                          //jou 20180814 (Steven) : 增加Motor database 異常警示
            {
                ShowMyMessage(asMotorDatabaseErr);
                return false;
            }

            if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                      //Ifor 20181127 : add RTC TIME SYNC
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-RTCTIMESYNC)
                // 缺相依：`TCOM2Shim` 缺 `sRealTimeCom_Send[]`、`rtTIMESYNC`、`SendCommToVision()`
                // （golden rs232.h 的即時 CCD（RTC）通訊介面）。整個 RTC 通訊層沒翻。
                // 行為：歸零時**不會把 handler 的系統時間送給 RTC CCD 對時**。
                //   後果是 CCD 那端的照片時戳與 handler 的 log 時戳可能有偏差，
                //   查影像對照生產紀錄時會對不起來。不影響任何動作或互鎖。
                //   ⚠ 這一格與下面幾個 RTC 閘是同一個缺口的不同落點
                //   （TIMESYNC / rtHome / InitRealTimeCCDPara / bRealTimeCom_ReceiveOK）。
                // UN-GATE：等 COM2 的 RealTimeCom 介面翻進來（rs232.h 那一族），四格一起解。
                #if 0 // GATE (W906-HOME-C2-RTCTIMESYNC): 缺相依，見上面的就地註解
                COM2->sRealTimeCom_Send[COM2->rtTIMESYNC].sprintf("@TIMESYNC:,%s",FormatDateTime("MM/DD/YYYY HH:NN:SS AM/PM", Now())+ "+");                     // RTC僅收@到+的資料，"+"必需放在資料最後面
                COM2->SendCommToVision(COM2->rtTIMESYNC, true);
                #endif // GATE (W906-HOME-C2-RTCTIMESYNC)

            }

            Cylinder[C_TrayZ_Selector     ].Off();                              //Steven 20240123 : 歸零前先把分離氣缸關起來
            Cylinder[C_EmptyLoaderZ_Select].Off();
            Cylinder[C_ColorLoaderZ_Select].Off();
            for(int i=0; i<MAX_AUTO_TRAY; i++)
            {
                Cylinder[C_AutoZ_Select[i]].Off();
            }

            for(int i=0; i<4; i++)
                Temperature.iATCCurrentFailCount[i]=0;                          //Steven 20151123 : Continue Fail Temp Offset for ATC

            if(CosFunction.bOutShtLoseICInArmAway)                              //JerryYang 20200519 out shuttle lose IC, in arm要讓位
            {
                if(bSht1LoseICErr)
                {
                    bSht1LoseICErr=false;
                    ShowOutputShuttleICStatus(0);
                    StartDetectMotorSensor(0);                                  //清除
                    b1ShuttleMoveToLeft=false;
                    return false;
                }

                if(bSht2LoseICErr)
                {
                    bSht2LoseICErr=false;
                    ShowOutputShuttleICStatus(1);
                    StartDetectMotorSensor(1);                                  //清除
                    b2ShuttleMoveToLeft=false;
                    return false;
                }
            }

            MoveInRotateToDegreeAtSameTime(0, true);                            //Steven 20170425 (wei) : Add rotate motor
            MoveOutRotateToDegreeAtSameTime(0, true);
            iInRotateCount=1;
            iOutRotateCount=1;

            PrePushLoaderCylinder(true);                                        //Steven 20150429 : 預先打兩下Loader汽缸
            bZ1PickShuttle=false;                                               //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            bZ2PickShuttle=false;                                               //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
            bZ1Isdownflag=false;                                                //Isaac 20180307 index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖，回home後重置 //Steven 20180522 : true --> false, 解除Hang up
            bZ2Isdownflag=false;                                                //Isaac 20180307 index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖，回home後重置
            fMain->SendMSG_CMD(MSG_CMD_HandlerHomeStart);                       //Steven 20150304 : Add GPIB LOG
            fNote->bMyServoOffInArm=false;                                      //Steven 20110804 : 歸零後要重置狀態
            fNote->bMyServoOffOutArm=false;                                     //Steven 20110804 : 歸零後要重置狀態
            NeedWaitTrayArm        =false;                                      //wei 20151002
            bIndexPickErrShtStayRight1=false;                                   //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
            bIndexPickErrShtStayRight2=false;                                   //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
            bIndexPickUpErrorWaitRetry=false;

            fNote->bMyServoOffOutShuttle1=false;                                //ChungHung 20140522 add OutShuttle lose devices can servo off
            fNote->bMyServoOffOutShuttle2=false;                                //ChungHung 20140522 add OutShuttle lose devices can servo off
            bDoOverDrive           =false;                                      //Steven 20151207 : OverDrive for TSMC
            bDoReContact           =false;                                      //Steven 20151207 : Recontact for TSMC
            bAutoCleaning=false;                                                //Isaac 20171006 (wei) ： AMKOR_Philippines machine status Bit16 是否在做autoclean
            bNeedWaitRTCAutoVerify =false;                                      //jou 2014-06-24 RTC 自動進行Model驗證
            bRTCAutoVerifyControlEP=false;                                      //jou 2014-06-24 RTC 自動進行Model驗證
            bWaitOutArmCheckCylin=false;                                        //jou 20240131 : 修正out arm 與 auto tray互卡衝突hang up
            bIsPlacingToBuffer=false;                                           //JerryYang 20250828 : fix color誤退TRAY
            bIsCatchingFromBuffer=false;                                        //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中

            if(bShuttle1Pause)                                                  //Sam 20250520 : 修正 D42 功能觸發時按 Home 死機
            {
                sBuffer.printf("D42 function is stopped by HOME action shuttle1");
                MyDBIProcess("Message", sBuffer);
                bShuttle1Pause=false;
            }

            if(bShuttle2Pause)
            {
                sBuffer.printf("D42 function is stopped by HOME action shuttle2");
                MyDBIProcess("Message", sBuffer);
                bShuttle2Pause=false;
            }

            bArm1NeedSuck=false;                                                //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
            bArm2NeedSuck=false;
            bD44Arm1CheckVacOn=false;
            bD44Arm2CheckVacOn=false;
            iIndexTakeDeviceChk1=0;                                             //JerryYang 20190325 fix回home後hang up
            iIndexTakeDeviceChk2=0;

            MyDBIProcessNew("Motion", "WAR2207", "Do process motor home start.");                                       //jou 2010-11-23
            bMoveInArm2XYToWaitOk=true;                                         //Steven 20131025 : 要先動到安全位置才可以再到別的地方
            bIndexDropVacuumError=false;                                        //kevin 20190418 避免 inarm 來回跑
            bArm1Delay=false;                                                   //JerryYang 20200903 : 修正ARM 1預熱ARM2測試功能流程
            bArm2Delay=false;
            bHalfViewVerifyNeedAboveSocket=false;
            DoSetupSystemToProd();                                              //JerryYang 20220215 : RTC Auto Verify half view check

            bTriggerRTC_AutoSTD=false;                                          //JerryYang 20240829 : RTC Auto STD
            fLotInfo->cbRTCASTD->Enabled=true;

            bInArmHasHotIC=false;                                               //Sam 20211012 : Debug 用
            bLoadNewEmptyTrayToCarStart=false;                                  //Sam 20211119 : 放 Tray 增加保護
            bUnLoadNewEmptyToStackStart=false;                                  //Sam 20211119 : 放 Tray 增加保護
            bLoadNewColorTrayToCarStart=false;                                  //Sam 20211119 : 放 Tray 增加保護
            bUnLoadNewColorToStackStart=false;                                  //Sam 20211119 : 放 Tray 增加保護

            bAutoCleanShuttle1MoveToLeft=false;                                 //Sam 20250728 : 避免 AutoClean Drop Alarm 未完成時 Home Hange up
            bAutoCleanShuttle1HasPickErr=false;
            bAutoCleanShuttle2MoveToLeft=false;
            bAutoCleanShuttle2HasPickErr=false;

            if(LastSet.iRealDummy==REALLY)                                      //JerryYang 20170815 (Steven) In arm A吸嘴上IC大於36mm時X軸回homeIC會撞到基座,要求強制將IC取下才能回home
            {
                if(DeviceForm.XDimension>=3600 || UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1)
                {
                    if(USE_PICKER_COUNT==ep1Picker)                             //Ifor 20251106 add:單吸嘴模組Out Arm 回home IC會撞到強制將IC取下才能回home
                    {
                        if(InArmSuck.Suck[0][0].GetStatus()==true)
                        {
                            ShowMyMessage("Please take off the device of in arm Vacuum A");
                            if(CUSTOMER_CODE==CC_ASE_SG)
                                InArmSuck.SetItemData(0, 0, NULL_IC);
                            return false;
                        }
                    }
                    else
                    {
                        if(InArmSuck.Suck[0][0].GetStatus()==true)
                        {
                            ShowMyMessage("Please take off the device of in arm Vacuum A");
                            return false;
                        }

                        if(InArmSuck.Suck[1][0].GetStatus()==true)
                        {
                            ShowMyMessage("Please take off the device of in arm Vacuum B");
                            return false;
                        }

                        if(CUSTOMER_CODE==CC_ASE_SG)                            //Ifor 20251120 add:ASE SG 要求InArm第二支吸嘴需要取走
                        {
                            InArmSuck.SetItemData(0, 0, NULL_IC);               //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                            InArmSuck.SetItemData(1, 0, NULL_IC);               //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE

                            if(InArmSuck.Suck[0][1].GetStatus()==true)
                            {
                                ShowMyMessage("Please take off the device of in arm Vacuum C");

                                return false;
                            }

                            if(InArmSuck.Suck[1][1].GetStatus()==true)
                            {
                                ShowMyMessage("Please take off the device of in arm Vacuum D");

                                return false;
                            }
                            InArmSuck.SetItemData(0, 1, NULL_IC);               //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                            InArmSuck.SetItemData(1, 1, NULL_IC);               //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE

                            if(bUseAxExPicker()==true)
                            {
                                if(InArmSuck.Suck[0][2].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of in arm Vacuum E");
                                    return false;
                                }

                                if(InArmSuck.Suck[1][2].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of in arm Vacuum F");

                                    return false;
                                }
                                InArmSuck.SetItemData(0, 2, NULL_IC);           //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                                InArmSuck.SetItemData(1, 2, NULL_IC);           //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                            }
                            else if(bUseAxxGPicker()==true)
                            {
                                if(InArmSuck.Suck[0][3].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of in arm Vacuum G");
                                    return false;
                                }

                                if(InArmSuck.Suck[1][3].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of in arm Vacuum H");
                                    return false;
                                }
                                InArmSuck.SetItemData(0, 3, NULL_IC);           //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                                InArmSuck.SetItemData(1, 3, NULL_IC);           //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                            }
                        }
                    }
                }

                if(DeviceForm.XDimension>=4000 || UserDefForm_File[TrayForm.Loader.iTrayType].XDivision==1)
                {
                    if(USE_PICKER_COUNT==ep1Picker)                             //Ifor 20251106 add:單吸嘴模組Out Arm 回home IC會撞到強制將IC取下才能回home
                    {
                        if(OutArmSuck.Suck[0][0].GetStatus()==true)
                        {
                            ShowMyMessage("Please take off the device of out arm Vacuum A");
                            if(CUSTOMER_CODE==CC_ASE_SG)
                                OutArmSuck.SetItemData(0, 0, NULL_IC);
                            return false;
                        }
                    }
                    else
                    {
                        if(CUSTOMER_CODE==CC_ASE_SG)                            //Ifor 20251120 add:ASE SG 要求OutArm第三支吸嘴需要取走
                        {
                            if(TestIF_File.iTestMode==SingleSite)
                            {
                                if(OutArmSuck.Suck[0][0].GetStatus()==true)
                                {
                                    if(TestIF_File.bSingleUseOtherSuck==true)
                                        ShowMyMessage("Please take off the device of out arm Vacuum E");                //Ifor 20200108 Fix: In Arm => Out Arm
                                    else
                                        ShowMyMessage("Please take off the device of out arm Vacuum G");                //Ifor 20200108 Fix: In Arm => Out Arm
                                    return false;
                                }

                                if(OutArmSuck.Suck[1][0].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of out arm Vacuum H");
                                    return false;
                                }
                                OutArmSuck.SetItemData(0, 0, NULL_IC);          //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                                OutArmSuck.SetItemData(1, 0, NULL_IC);          //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                            }
                            else if(TestIF_File.iTestMode==DualSite)
                            {
                                if(OutArmSuck.Suck[0][0].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of out arm Vacuum A");                    //Ifor 20200108 Fix: In Arm => Out Arm
                                    return false;
                                }

                                if(OutArmSuck.Suck[1][0].GetStatus()==true)
                                {
                                    ShowMyMessage("Please take off the device of out arm Vacuum B");
                                    return false;
                                }
                                OutArmSuck.SetItemData(0, 0, NULL_IC);          //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                                OutArmSuck.SetItemData(1, 0, NULL_IC);          //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE

                                if(bUseAxExPicker()==true)
                                {
                                    if(OutArmSuck.Suck[0][1].GetStatus()==true)
                                    {
                                        ShowMyMessage("Please take off the device of out arm Vacuum E");                //Ifor 20200108 Fix: In Arm => Out Arm
                                        return false;
                                    }

                                    if(OutArmSuck.Suck[1][1].GetStatus()==true)
                                    {
                                        ShowMyMessage("Please take off the device of out arm Vacuum F");
                                        return false;
                                    }
                                }
                                else if(bUseAxxGPicker()==true)
                                {
                                   if(OutArmSuck.Suck[0][1].GetStatus()==true)
                                    {
                                        ShowMyMessage("Please take off the device of out arm Vacuum G");                //Ifor 20200108 Fix: In Arm => Out Arm
                                        return false;
                                    }

                                    if(OutArmSuck.Suck[1][1].GetStatus()==true)
                                    {
                                        ShowMyMessage("Please take off the device of out arm Vacuum H");
                                        return false;
                                    }
                                }
                                OutArmSuck.SetItemData(0, 1, NULL_IC);          //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                                OutArmSuck.SetItemData(1, 1, NULL_IC);          //Ifor 20250523 add:避免IC拔掉後報DROP的JAM CODE
                            }
                        }
                        else
                        {
                            if(OutArmSuck.Suck[0][3].GetStatus()==true)
                            {
                                ShowMyMessage("Please take off the device of out arm Vacuum G");                        //Ifor 20200108 Fix: In Arm => Out Arm
                                return false;
                            }

                            if(OutArmSuck.Suck[1][3].GetStatus()==true)
                            {
                                ShowMyMessage("Please take off the device of out arm Vacuum H");
                                return false;
                            }
                        }
                    }
                }
            }

            if(USE_LdUldCassetteMode==1)
            {
                if(Sen[SnLoaderBoatActDetect].IsOn())
                {
                    ShowErrorMessage("MES0921", 0, MMTrayZ);
                    return false;
                }

                if(Sen[SnAuto1BoatActDetect].IsOn())
                {
                    ShowErrorMessage("WAR1116", 0, MAuto1Z);
                    return false;
                }

                if(Sen[SnAuto2BoatActDetect].IsOn())
                {
                    ShowErrorMessage("WAR1216", 0, MAuto2Z);
                    return false;
                }
            }

            if(LastSet.iRealDummy!=DUMMY)
            {
                if(IniConfig.bP37bAutoCylinderUP)                               //kevin 20200504 add for Auto 123 氣缸常態在上
                {
                    for(int i=eAuto1; i<=iAutoRight; i++)
                    {
                        int iAuto=iAutoIndex[i];
                        if(LastSet.iRealDummy!=DUMMY && MOT[iMMAuto_Car[i]].fHasTray)
                        {
                            if(IniConfig.bP37bAutoCylinderUP)                   //kevin 20180726 (wei) Auto 123 氣缸常態在上
                            {
                                Cylinder[C_Auto_Up[iAuto]].Off();
                                Cylinder[C_Auto_Selector[iAuto]].Off();         //kevin 20120718 修改輸入氣缸因代號位置不同共用程式
                                ShowMyMessage("Please take out all trays from Auto-unloader #1,#2,#3 ");
                            }

                            if(Sen[SnAutoTrayCar[iAuto]].IsOff())
                                MOT[iMMAuto_Car[i]].fHasTray=false;
                        }
                    }
                }
            }

            if(TestIF_File.bUseSLKClamp==true)                                  //JerryYang 20160526 Socket clamp跟SKL Clamp都夾著時要把socket clamp放開
            {
                if(Sen[SnSocketClampPull1].IsOff() || Sen[SnSocketClampPull2].IsOff() ||
                   Sen[SnSocketClampPush1].IsOn()  || Sen[SnSocketClampPush2].IsOn()  )
                {
                    ShowMyMessage("Socket clamp must unclamp !!", "Socket clamp必須打開才能回Home");
                    fHome->fAbort=false;
                    SystemStart=false;
                    fHome->iHomeStep=1;
                    SoftStop=true;
                    return true;
                }
            }

            if(AUTO3_IS_MAGAZINE==1)                                            //JerryYang 20220909 : add magazine
            {
                if(bChaneMagTrayflag || bMagGetNewTrayflag)
                {
                    if(Sen[SnAuto3TrayDetect].IsOn() ||
                       Sen[SnMagazineTrackDetect].IsOn() ||
                       Sen[SnMagazineTrackDetect2].IsOn())                      //Sam 20221116 : Magazine TrayArm 自動補 Tray)
                    {
                        ShowMyMessage("Please remove the tray from AUTO3, check SnAuto3TrayDetect/SnMagazineTrackDetect/SnMagazineTrackDetect2");
                        return false;
                    }

                    if(iMagChangeStep==1)                                       //表示正在從AUTO3夾TRAY移動到Magazine中
                    {
                        if(MOT[MMAuto3_Car].fHasTray && MOT[MMAuto3].fHasTray==true)                                    //正在AUTO3夾到Magazine
                        {
                            iMagChangeStep=0;
                            Str.sprintf("Please remove the tray on Magazine slot%d and AUTO3", iAuto3MagazineIndex+1);
                            ShowMyMessage(Str);
                            iAuto3MagazineIndex=-1;
                            iWhichMag=-1;
                            MOT[MMAuto3].ClearTray(__FUNC__);
                            MOT[MMAuto3_Car].fHasTray=false;
                        }
                    }
                    else if(iMagChangeStep==2)                                  //表示正在從Magazine夾TRAY移動到AUTO3中
                    {
                        if(MOT[MMAuto3].fHasTray==false && MOT[MMAuto3_Car].fHasTray==true && MOT[iMMgzTray[iWhichMag]].fHasTray==true)
                        {
                            iMagChangeStep=0;
                            MOT[iMMgzTray[iWhichMag]].ClearTray(__FUNC__);
                            Str.sprintf("Please remove the tray on Magazine slot%d and AUTO3", iWhichMag+1);
                            ShowMyMessage(Str);
                            iWhichMag=-1;
                            iAuto3MagazineIndex=-1;
                        }
                    }
                    bChaneMagTrayflag=false;
                    iWhichMag=-1;
                    iAuto3MagazineIndex=-1;
                }
            }

            if(INSTALL_OCR_YMot==eocrYMotInstal)                                //Frank 20250214 add
            {
                if(CUSTOMER_CODE!=CC_ARDENTEC &&                                //AI(ht9045-v899) 20260504: ARDENTEC OCR YMot Home Check bypass, no mechanical interference
                   MOT[MMTrayY].fHasTray && Sen[SnLoaderSureTray].IsOn() && Cylinder[C_Load_Middle].OffStatus()==false)
                {
                    ShowMyMessage("Loader cylinder up , can not do homing!");
                }
            }

            if(AUTO3_IS_MAGAZINE==1)                                            //pig 2016.06.29 MgzTrayCatchCynHome end
            {                                                                   // 2011.01.21 , Joye , Auto 3 Magazine {
                Cylinder[C_Auto3EdgePush].Off();                                //kevin 20140603
                Cylinder[C_CatchMagazineTray].Off();                            //kevin 20140603
                if(Sen[SnMagazineTrackDetect].Enable &&
                   Sen[SnMagazineTrackDetect].IsOn()==true)                     //Sam 20221116 : Magazine TrayArm 自動補 Tray
                {                                                               // Alarm : Track has Tray , Magazine Can not Home
                    ShowMyMessage("SnMagazineTrackDetect is on, can not do homing!");

                    fHome->iHomeStep    = 1;
                    fHome->fAbort       = false;
                    SystemStart         = false;
                    SoftStop            = true;
                    return true;
                }

                if(Sen[SnMagazineTrackDetect2].Enable && Sen[SnMagazineTrackDetect2].IsOn()==true)
                {                                                               // Alarm : Track has Tray , Magazine Can not Home
                    ShowMyMessage("SnMagazineTrackDetect2 is on, can not do homing!");

                    fHome->iHomeStep    = 1;
                    fHome->fAbort       = false;
                    SystemStart         = false;
                    SoftStop            = true;
                    return true;
                }
            }

            bATC_SITE_2ND_CHECK[0]=false;                                       //Ifor 20160509 add ATC 測試時開啟第二點溫度監控
            bATC_SITE_2ND_CHECK[1]=false;
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-ATCTESTEND)
            // 缺相依：`TATC_InterfaceFormShim::SiteTesting()`（New ATC 介面）與
            // `ATCInterfaceForm`（HonPrec ATC 介面的全域，golden ATCInterface.h）。
            // 本樹有 `ATC_InterfaceForm` 門面但沒有這兩個。
            // 行為：歸零時**不會通知 ATC「這些測試站的測試結束了」**。
            //   * New ATC：不送 SiteTesting(全 false)，ATC 端可能還以為某些站在測試中。
            //   * HonPrec ATC：不送 SendTestEnd(0)/(1)。
            //   ⇒ 在接 ATC 溫控的機台上，歸零後 ATC 的站別狀態不會被清乾淨。
            //   沒有接 ATC 的機台（含這台筆電）完全不受影響。
            //   ⚠ 這一格是**通知**，不是互鎖 —— 沒有任何 handler 側的拒絕動作被跳過。
            // UN-GATE：等 ATC/ATCInterface.h 的 ATCInterfaceForm 全域與 SiteTesting 落地。
            //   （那個標頭本身與本 TU 的其他標頭有 clWhite 對撞，解閘時要一併處理。）
            #if 0 // GATE (W906-HOME-C2-ATCTESTEND): 缺相依，見上面的就地註解
            if(ATC_SYSTEM==eNewATCSystem)
            {
                for(int i=0; i<iATC_Use_Heat_Count; i++)                        //Ifor 20160516 add Index 測試前開啟 ATC 第二點溫度偵測
                    bATCSiteTest[i]=false;                                      //Ifor 20160516 修改ATC Heat 設定數
                ATC_InterfaceForm->SiteTesting(iATC_Use_Heat_Count, bATCSiteTest);
            }
            else if(ATC_SYSTEM==eATCHonPrecType)
            {
                ATCInterfaceForm->SendTestEnd(0);
                ATCInterfaceForm->SendTestEnd(1);
            }
            #endif // GATE (W906-HOME-C2-ATCTESTEND)


            IsTest=false;                                                       //JerryYang 20160318 增加防護 回Home時要重置GPIB
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-GPIBADAM)
            // 缺相依：
            //   * `TfMain::RunTestProgram(bool)` —— GPIB 的「測試程式執行中」旗標（golden main.h）
            //   * `Open_ADAM_6024()` / `Close_ADAM_6024()` —— ADAM-6024 類比模組的連線開關
            //     （golden adam6024.h；本樹 forms/fIoSetView.h:239-240 只在註解裡提到它們）
            // 行為：
            //   * 歸零時**不會通知 GPIB 端「測試程式停了」**。上一行 `IsTest=false` 照留，
            //     所以 handler 自己的狀態仍然被重置 —— 少的只是往 tester 送的那一次通知。
            //   * **ADAM-6024 不會被重新連線**。golden 在這裡關再開，是為了把 CKD FCM
            //     的連線洗一次；閘住之後沿用既有連線。
            //   ⚠ 連帶影響：golden 緊接著用 `Open_ADAM_6024()==false && iRealDummy==REALLY`
            //     當**連線失敗就不准歸零**的互鎖（下面那個 if）。那個 if 一併閘住 ——
            //     ⇒ **在 REALLY 模式、ADAM-6024 斷線的機台上，golden 會擋住歸零，本樹不會。**
            //     這是本批唯一一個**少掉互鎖**的閘，所以特別寫出來。
            //     沒裝 ADAM-6024 的機台不受影響（golden 那個 if 本來也不會成立）。  //AI(W906-FLOW-5) 20260929: CORRECTION -- the sentence above is wrong (golden re-read): Open_ADAM_6024(IP,Num) returns false in a non-SOFT_SIMULTE build when EP_Install==0 (golden adam6024.cpp:387-390), when ADAMTCP_Open fails (:273-282), and when the module does not answer the UDP AI-range read while CHECK_EP_SETTING!=0 (:292-296); only a SOFT_SIMULTE build returns true (:391-392).  So in REALLY mode golden ALSO stops HOME ("Adam Connect Error and Stop Home") on a machine whose ADAM-6024 is absent or silent.  HT9050: EP_Install=3, CHECK_EP_SETTING=1.
            // UN-GATE：等 adam6024 的 Open/Close 落地 + TfMain 門面帶進 RunTestProgram。  //AI(W906-FLOW-5) 20260929: re-measured, both dependencies still absent: (1) Open_ADAM_6024 / Close_ADAM_6024 need the ADAMTCP transport, which has NO binding in this tree -- 0 ADAMTCP_* declarations or definitions, no copy of the vendor header ADAMTCP.h, golden links ADAMTCPbc.lib (HT9045.bpr:153), a Borland OMF library (first byte 0xF0) MinGW cannot link, and no LoadLibrary adapter; the PE32 DLL exists only in the BCB6 drop D:/HT9045/EXE/ADAMTCP.dll.  How to bind it is open question Q8c-4 (docs/nb2_assist/PENDING_JIMMY.md:33).  (2) TfMain::RunTestProgram has 0 definitions in this tree.
            #if 0 // GATE (W906-HOME-C2-GPIBADAM): 缺相依，見上面的就地註解
            fMain->RunTestProgram(false);
            Close_ADAM_6024();
            #endif // GATE (W906-HOME-C2-GPIBADAM)

            if(false &&                                                        // AI(W906-HOME-C2) 20260920: 原為 `Open_ADAM_6024()==false &&`，見上面 GATE (W906-HOME-C2-GPIBADAM)
               LastSet.iRealDummy==REALLY)                                      //Jimmychiu 20230804 : 整合全部連線檢查
            {
                ShowMyMessage("Adam Connect Error and Stop Home");
                return false;
            }

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-ALIGNCCD)
            // 缺相依：`fAutoAlignment`（golden 的 CCD Auto-Alignment 表單全域）與常數
            // `LOAD_FILE`。兩者在本樹**全樹 0 命中** —— 整個 Auto-Alignment CCD 模組沒翻。
            // 行為：歸零時**不會連線 Alignment CCD、也不會把目前工作檔名推給它**。
            //   ⇒ 在 `MACHINE_HAS_AUTO_ALIGNMENT_CCD` 的機台上，CCD 端沿用上一次的
            //   工作檔，換料號之後對位參數可能是舊的。
            //   沒有 Alignment CCD 的機台（含這台筆電）完全不受影響。
            // UN-GATE：等 Auto-Alignment 模組（TcpipOpen / ChangeFileName / InArmSendCommand）
            //   落地。本檔另有一處同族的閘（case 400 的 ResetOKDeleyTime 臂）。
            #if 0 // GATE (W906-HOME-C2-ALIGNCCD): 缺相依，見上面的就地註解
            if(MACHINE_HAS_AUTO_ALIGNMENT_CCD)                                  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
            {
                fAutoAlignment->TcpipOpen();
                if(CUSTOMER_CODE!=CC_ASE_KaohSiung)
                {
                    fAutoAlignment->ChangeFileName(fMain->cbSetupFileName->Text);
                }
                else
                {
                    fAutoAlignment->ChangeFileName(TestIF_File.AutoAlignmentFileName);                                  //Kenhsieh 20211122 : 修改無法讀取fileName問題
                    fAutoAlignment->InArmSendCommand(LOAD_FILE);                //Kenhsieh 20211006 : 回Home時更新Alignment工作檔
                }
            }
            #endif // GATE (W906-HOME-C2-ALIGNCCD)


            if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                      //----- by dell ccd realtime-------------
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-RTCINIT)
                // 缺相依：`TfLotInfo::RTCChangeFile(bool)` 與 `TCOM2Shim::InitRealTimeCCDPara()`
                // （golden uLotInfo.h / rs232.h 的即時 CCD 初始化）。同 RTCTIMESYNC 的缺口。
                // 行為：歸零時**不會重新載入 RTC CCD 的工作檔、也不會重設它的參數**。
                //   ⇒ 有 RTC CCD 的機台上，CCD 沿用上一次的 model 與參數。
                // ⚠ **`hRealCCDTimeOut.SetSecAndOn(10)` 刻意留在閘外面**（見下一行）——
                //   那是本樹自己的計時器，它有沒有被起算會影響 case 650 的逾時判斷。
                //   閘掉它會讓一個「從來沒起算過」的計時器參與判斷，那比留著更難預測。
                // UN-GATE：等 RTC 介面（rs232.h 那一族）與 fLotInfo 的 RTCChangeFile 落地。
                #if 0 // GATE (W906-HOME-C2-RTCINIT): 缺相依，見上面的就地註解
                fLotInfo->RTCChangeFile(false);                                 //Steven 20110826 : Real Time CCD - 初始化工作檔
                COM2->InitRealTimeCCDPara();
                #endif // GATE (W906-HOME-C2-RTCINIT)
                hRealCCDTimeOut.SetSecAndOn(10);                                //JerryYang 20250602 : 2sec to 10sec  //ChungHung 20140514 fix in homeing and contact make time out
            }

            if(INSTALL_OCR!=eocrUninstal && TestIF.bOcrFunction)                //Steven 20120716 : OCR
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-OCRINIT)
                // 缺相依：`TfOCR::SetOcrFileName(AnsiString)` 與 `TfOCR::InitialOCRPara()`
                // （golden OCR.h）。本樹的 TfOCR 門面沒有這兩個成員。
                // 行為：歸零時**不會把目前工作檔名推給 OCR、也不會重設 OCR 參數**。
                //   ⇒ 有 OCR 的機台上，OCR 沿用上一次的設定；換料號後讀字參數可能是舊的。
                // ⚠ **`bOCRHomeActionPass=true` 留在閘外面**：它是本檔 case 600 的
                //   `if(bOCRHomeActionPass)` 讀的旗標（那一段會關 OCR 光源）。
                //   把它一起閘掉會讓光源永遠不關，那是**多一個副作用**而不是少一個。
                #if 0 // GATE (W906-HOME-C2-OCRINIT): 缺相依，見上面的就地註解
                fOCR->SetOcrFileName(fMain->cbSetupFileName->Text);
                fOCR->InitialOCRPara();                                         //ChungHung 20120830 add OCR Function add
                #endif // GATE (W906-HOME-C2-OCRINIT)

                bOCRHomeActionPass=true;                                        //Frank 20250214 add
            }

            if(IniConfig.bHaveRotateShuttle==true && TestIF.bRotateShuttle)     //kevin 20110531 旋轉SHUTTLE 鎖最高速度
            {
                MOT[MInShuttle1].Motor->PJogHighSpeed   =IniConfig.iPJogHighSpeed;
                MOT[MInShuttle1].Motor->InitSpeed       =IniConfig.iInitSpeed;
                MOT[MInShuttle2].Motor->PJogHighSpeed   =IniConfig.iPJogHighSpeed;
                MOT[MInShuttle2].Motor->InitSpeed       =IniConfig.iInitSpeed;
            }
            else
            {
                MOT[MInShuttle1].Motor->InitSpeed       =iInitSpeedSh1;         //kevin 20110531 旋轉SHUTTLE 鎖最高速度
                MOT[MInShuttle1].Motor->PJogHighSpeed   =iPJogHighSpeedSh1;
                MOT[MInShuttle2].Motor->InitSpeed       =iInitSpeedSh2;
                MOT[MInShuttle2].Motor->PJogHighSpeed   =iPJogHighSpeedSh2;
            }

            if(TestIF_File.iAutoClean_Tray!=eCKPos_HP2 &&                       //Steven 20130620 Start : Auto Clean到一半按歸零要繼續跑
               CUSTOMER_CODE!=CC_ASE_KaohSiung)                                 //kevin 20150806
            {
                if(bRunAutoClean==true)
                {
                    ResetAutoClean();
                    hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);
                }

                InitialAutoCleanTask();
                InitialShuttleAutoCleanTask();
                InitialIndexAutoCleanTask();
                InitPickFromShuttleTask();
            }

            if(fContact->IsRun2DCheck()==true)                                  //JerryYang 20250220 : 2DID硬體順序檢查功能
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-2DIDMAP)
                // 缺相依：`TfContactShim::RestoreData_2D_Check()` 與 `Do2DIDMapCheck(bool)`
                // （golden cContact.h）。本樹的 fContact 是門面（atester_shims.h:157），沒有這兩個。
                // 行為：歸零時**不做 2DID 硬體順序檢查、也不還原它的資料**。
                //   ⚠ 這一格**少掉一個 early-return**：golden 在 `RestoreData_2D_Check()==true`
                //   時直接 `return false`（這一 tick 不往下走，等資料還原完）。
                //   閘住之後那個等待不存在，歸零會直接往下。
                //   ⇒ 只影響有開 `fContact->IsRun2DCheck()` 的機台（2DID 硬體順序檢查功能）。
                //   外層的 `if(fContact->IsRun2DCheck()==true)` 照留，所以沒開的機台不受影響。
                // UN-GATE：等 TfContact 真本體落地（同 csystem.cpp 的 W906-HOME-W1-CONTACTFN 閘）。
                #if 0 // GATE (W906-HOME-C2-2DIDMAP): 缺相依，見上面的就地註解
                if(fContact->RestoreData_2D_Check()==true)                      //JerryYang 20250428 : fix 2DID map
                {
                    return false;
                }
                fContact->Do2DIDMapCheck(true);
                #endif // GATE (W906-HOME-C2-2DIDMAP)

            }

            if(CUSTOMER_CODE==CC_SIGURD_HUKOU ||                                //jou 2012-03-07 矽格希望歸完零，只有當下在測試的IC丟到error bin。
               CUSTOMER_CODE==CC_SIGURD_ChungXing ||                            //KaiChen 20201112 ：矽格-中興，強制執行
               IniConfig.bI02HomeSetSocketICToErrBin)                           //JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin
            {
                if(CUSTOMER_CODE==CC_KYEC_LEE &&                                //Ifor 20190218 : add KYEC 要求海思專版需做Self Test時不丟至Err Bin
                   CosFunction.bHiSiliconFunction==true &&
                   (bNeedSendATCRunSelfTest==true ||
                    bNeedWaitATCRunSelfTestFinish==true))
                {
                }
                else
                {
                    if(bUseTwoArm32Site==true)
                    {
                        iCT=FTestSuck.CountRealIC();
                        sBuffer.printf("Do Home Index Arm1 Place to Error bin : Device=%d;", iCT);
                        MyDBIProcess("Message", sBuffer);
                        FTestSuck.SetAllRealIC2InterfaceBin();
                        iCT=BTestSuck.CountRealIC();
                        sBuffer.printf("Do Home Index Arm2 Place to Error bin : Device=%d;", iCT);
                        MyDBIProcess("Message", sBuffer);
                        BTestSuck.SetAllRealIC2InterfaceBin();
                    }
                    else
                    {
                        if(fFrontNeedTest==true)
                        {
                            iCT=FTestSuck.CountRealIC();
                            sBuffer.printf("Do Home Index Arm1 Place to Error bin : Device=%d;", iCT);
                            MyDBIProcess("Message", sBuffer);
                            FTestSuck.SetAllRealIC2InterfaceBin();
                        }
                        else if(fRearNeedTest==true)
                        {
                            iCT=BTestSuck.CountRealIC();
                            sBuffer.printf("Do Home Index Arm2 Place to Error bin : Device=%d;",iCT);
                            MyDBIProcess("Message", sBuffer);
                            BTestSuck.SetAllRealIC2InterfaceBin();
                        }
                    }
                }
            }
            else if(IniConfig.bA03UseAfterHomeCarryAndSuckIcToRBin)
            {
                if(CUSTOMER_CODE==CC_KYEC_LEE &&
                   CosFunction.bHiSiliconFunction==true &&                      //Ifor 20190218 : add KYEC 要求海思專版需做Self Test時不丟至Err Bin
                   (bNeedSendATCRunSelfTest==true ||
                    bNeedWaitATCRunSelfTestFinish==true))
                {
                }
                else
                {
                    if(CUSTOMER_CODE==CC_SCK)
                    {
                        FTestSuck.SetAllHASIC2ErrorBin();
                        BTestSuck.SetAllHASIC2ErrorBin();
                        TestSocket.SetAllHASIC2ErrorBin();
                    }
                    else
                    {
                        iCT=FRCarryKit.CountRealIC();
                        if(iCT>0)
                        {
                            sBuffer.printf("Do Home Out Shuttle1 Place to Error bin : Device=%d;",iCT);
                            MyDBIProcess("Message", sBuffer);
                        }
                        FRCarryKit.SetAllRealIC2InterfaceBin();
                        iCT=BRCarryKit.CountRealIC();
                        if(iCT>0)
                        {
                            sBuffer.printf("Do Home Out Shuttle2 Place to Error bin : Device=%d;",iCT);
                            MyDBIProcess("Message", sBuffer);
                        }
                        BRCarryKit.SetAllRealIC2InterfaceBin();
                        iCT=FTestSuck.CountRealIC();
                        if(iCT>0)
                        {
                            sBuffer.printf("Do Home Index Arm1 Place to Error bin : Device=%d;",iCT);
                            MyDBIProcess("Message", sBuffer);
                        }
                        FTestSuck.SetAllRealIC2InterfaceBin();
                        iCT=BTestSuck.CountRealIC();
                        if(iCT>0)
                        {
                            sBuffer.printf("Do Home Index Arm2 Place to Error bin : Device=%d;",iCT);
                            MyDBIProcess("Message", sBuffer);
                        }
                        BTestSuck.SetAllRealIC2InterfaceBin();
                        TestSocket.SetAllRealIC2InterfaceBin();
                        iCT=OutArmSuck.CountRealIC();
                        if(iCT>0)
                        {
                            sBuffer.printf("Do Home Output Arm Place to Error bin : Device=%d;",iCT);
                            MyDBIProcess("Message", sBuffer);
                        }

                        for(int i=0; i<OutArmSuck.iPickRow; i++)
                        {
                            for(int j=0; j<OutArmSuck.iPickCol; j++)
                            {
                                if(OutArmSuck.Item[i][j]!=NULL_IC && OutArmSuck.Item[i][j]!=HAS_NULL_IC)
                                {
                                    OutArmSuck.iWhichAuto[i][j]=Prod.iIfErrorT6;
                                    OutArmSuck.Item[i][j]=TEST_PASS+iTestBinCount;
                                    OutArmSuck.iBinData[i][j]=iTestBinCount;
                                    OutArmSuck.bPass[i][j]=false;
                                    OutArmSuck.bNeedReTest[i][j]=false;
                                }
                            }
                        }

                        if(CUSTOMER_CODE==CC_ASE_SG)                            //Ifor 20251120 add:ASE 要求A03 InArm InShuttle 也要丟Error Bin
                        {
                            iCT=FLCarryKit.CountRealIC();
                            if(iCT>0)
                            {
                                sBuffer.printf("Do Home In Shuttle1 Place to Error bin : Device=%d;",iCT);
                                MyDBIProcess("Message", sBuffer);
                            }
                            FLCarryKit.SetAllRealIC2InterfaceBin();
                            iCT=BLCarryKit.CountRealIC();
                            if(iCT>0)
                            {
                                sBuffer.printf("Do Home In Shuttle2 Place to Error bin : Device=%d;",iCT);
                                MyDBIProcess("Message", sBuffer);
                            }
                            BLCarryKit.SetAllRealIC2InterfaceBin();

                            iCT=InArmSuck.CountRealIC();
                            if(iCT>0)
                            {
                                sBuffer.printf("Do Home Input Arm Place to Error bin : Device=%d;",iCT);
                                MyDBIProcess("Message", sBuffer);
                            }
                            for(int i=0; i<MAX_ARM_Row; i++)
                            {
                                for(int j=0; j<MAX_ARM_Col; j++)
                                {
                                    if(InArmSuck.Item[i][j]!=NULL_IC && InArmSuck.Item[i][j]!=HAS_NULL_IC)
                                    {
                                        InArmSuck.iWhichAuto[i][j]=Prod.iIfErrorT6;
                                        InArmSuck.Item[i][j]=TEST_PASS+iTestBinCount;
                                        InArmSuck.iBinData[i][j]=iTestBinCount;
                                        InArmSuck.bPass[i][j]=false;
                                        InArmSuck.bNeedReTest[i][j]=false;
                                    }
                                }
                            }
                        }
                    }
                    bPlaceToShuttleFirst=true;
                }
            }

            fMain->ReStartAutoSiteMapping(true);                                //jou 2011-03-24 start : Auto Site Mapping

            bNowDoInterFaceErrorStep=false;
            bContactTimeOverStep=false;
            iDoInterFaceErrorStepTask=1;

            bCheckInArmX=false;
            for(int i=0; i<int(fHome->HomeClass.size()); i++)
            {
                fHome->HomeClass[i]->THomeOrder=fHome->HomeClass[i]->HomeOrder;
            }
            IndexZCanMove[0]=true;
            IndexZCanMove[1]=true;
            IndexMotorBreakerOFF();
            MagazineBreakerOFF();                                               //JerryYang 20220909 : add magazine
            InOutArmZBreakerOFF();                                              //add One sucker with rotate
            LDCarRotArmZBreakerOFF();                                           //RogerYang 20250828 add for Loader Rotate Arm
            CassetteBreakerOFF();                                               //Ifor 20251216 add:Boat Carrier
            ResetOKDeleyTime.Set0_1SecAndOn(2);
            //AI(ht9045-v899) 20260505: 預防中途 Home (生產過程 hangup 後按 Home All) 時，
            //  C_LoaderUpPress 仍 ON 導致台車移動撞壓桿。僅對裝有 MLoaderY 滑軌台車的機型 (eocrYMotInstal) 動作，
            //  避免影響其他機型既有行為。C_LoaderUpPress 固定鎖在機台，台車要移動必須先退開。
            if(INSTALL_OCR_YMot==eocrYMotInstal)
                Cylinder[C_LoaderUpPress].Off();
            if(MOT[MMTrayY].fHasTray==false &&
               LastSet.iRunStartMode!=rsmAutoRetest)                            //ChungHung 20140625 AutoRetest 不能鎖
            {
                if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                         Cylinder[C_TrayY_Fixer].On();
                    }
                    else
                    {
                        Cylinder[C_LoaderPushBack_Back].On();
                        Cylinder[C_LoaderPushBack_Push].Off();
                    }
                }
                else
                {
                    Cylinder[C_TrayY_Fixer].On();
                }
                Cylinder[C_LoaderEdgePush].On();
            }

            if(TRAY_ARM_MODE==eUnderCoveyor &&
                USE_OUT_SORT_ARM==eartUninstall)                                //rogerYang 20250722 add for 9046AU
            {
                for(int i=0; i<3; i++)
                    TrayCylinMoveIn(3+i);
            }

            SocketAirCoolingStart();                                            //jou 2016-04-28 Socket Air Cooling contact count trun on

            if(bPickFromHotplate ||                                             //jou 2010-01-19 start : 如果資料未轉移過來,需把IC放回去
               bPickFromLoader)                                                 //Steven 20210903 : 修正Loader吸到一半歸零要放回去
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        bSensor=InArmSuck.Suck[i][j].Sensor();
                        if(InArmSuck.Item[i][j]==NULL_IC && bSensor==true)
                        {
                            flag[i][j]=false;
                        }
                        else
                        {
                            flag[i][j]=true;
                        }
                    }
                }

                if(bPickFromHotplate)                                           //Steven 20210903 : 修正Loader吸到一半歸零要放回去
                {                                                               //Steven 20190815 : 修正加熱盤吸取到一半按下歸零,會導致記憶體破壞, Hang up
                    if(iHotWhichShuttle[iPickPlate[0]][iPickPlateX[0]][iPickPlateY[0]]==-1)
                    {
                        if(InArmSuck.Item[0][0]!=NULL_IC)
                        {
                            iHotWhichShuttle[iPickPlate[0]][iPickPlateX[0]][iPickPlateY[0]]=InArmSuck.iWhichShtPickFor32;
                        }
                    }

                    if(iHotWhichKit[iPickPlate[0]][iPickPlateX[0]][iPickPlateY[0]]==-1)
                    {
                        if(InArmSuck.Item[0][0]!=NULL_IC)
                        {
                            iHotWhichKit[iPickPlate[0]][iPickPlateX[0]][iPickPlateY[0]]=InArmSuck.iWhichKitPickFor32;
                        }
                    }
                }
            }

            iSpeedY=(50000*2);                                                  //Ifor 20150703 加快Y移動速度
            if(INDEX_PRESS_TYPE==e240KG ||                                      //jou 2011-12-23 從TfMain::TfMain改放到做Home就要重新設定一次
               INDEX_PRESS_TYPE==e400KG ||
               INDEX_PRESS_TYPE==e260KG ||
               INDEX_PRESS_TYPE==e360KG ||
               INDEX_PRESS_TYPE==e500KG)                                        //kevin 20160725 Add 500 KG
            {
                iSpeedFast=50000*4;
                iSpeedSlow=30000*4;
            }
            else
            {
                iSpeedFast=50000;
                iSpeedSlow=30000;
            }

            for(int i=0; i<2; i++)                                              //Steven 20170425 (wei) : Add rotate motor
            {
                for(int j=0; j<4; j++)
                {
                    MOT[MInRotateKit].Tray.iCurrRotAng[i][j]=0;
                    MOT[MOutRotateKit].Tray.iCurrRotAng[i][j]=0;
                }
            }

            if(MOT[MMPlate1].HasIC()==false && MOT[MMPlate2].HasIC()==false)    //Steven 20220117 : 歸零且加熱盤上面沒有東西的話, 變數要重置
            {
                iPickPlate[0]=0;
                iPickPlateX[0]=0;
                iPickPlateY[0]=0;
                iPickPlate[1]=0;
                iPickPlateX[1]=0;
                iPickPlateY[1]=0;
            }

            if(IniConfig.iI22TestTimeOutOption==3 && bI22_NeedHomeDelay)
            {
                hHomeDelay.SetSecAndOn(IniConfig.fI22HomeDelay);
            }

            TestIntervalsBoostTime.LatchCycleTime(true);                        //JerryYang 20181122 (Steven) :  (Steven) : 將不同function計時器分開
            fHome->iHomeStep=2;
            if(SHUTTLE_FLOODGATE==1)                                            //Ztex 2023.06.02 Add Check_SHUTTLE_FLOODGATE_Staste
            {
                #ifndef SOFT_SIMULTE
                Cylinder[C_Shuttle1Floodgate].Off();
                Cylinder[C_Shuttle2Floodgate].Off();
                Cylinder[C_OutShuttle1Floodgate].Off();                         //Ifor 20240620 add:Out Shuttle Floodgate
                Cylinder[C_OutShuttle2Floodgate].Off();                         //Ifor 20240620 add:Out Shuttle Floodgate
                if(Cylinder[C_Shuttle1Floodgate].GetOutBit()==true ||
                   Cylinder[C_Shuttle2Floodgate].GetOutBit()==true      ||
                   Cylinder[C_OutShuttle1Floodgate].GetOutBit()==true   ||      //Ifor 20240620 add:Out Shuttle Floodgate
                   Cylinder[C_OutShuttle2Floodgate].GetOutBit()==true   )       //Ifor 20240620 add:Out Shuttle Floodgate
                    return false;
                #endif
            }
            break;
        case 2:                                                                 //ChungHung 20140514 fix in homeing and contact make time out
            if(CUSTOMER_CODE==CC_ASE_SG)
            {
                iCheckShuttleSensor=1;
            }

            if(IniConfig.iI22TestTimeOutOption==3 && bI22_NeedHomeDelay)
            {
                if(hHomeDelay.Off()==true)
                {
                    bI22_NeedHomeDelay=false;
                }
                else
                {
                    break;
                }
            }

            if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)                      //wait RTC change file finish
            {
                if(fLotInfo->bRTCChangeFileFinish==false)
                {
                    if(hRealCCDTimeOut.Off())
                        ShowMyMessage("RTC Change File TimeOut");
                    break;
                }
            }
            SW[SwMotorRelay].Off();
            if(SubMachineType==Type_HT9046AU)
                ResetOKDeleyTime.SetSecAndOn(6);                                //RogerYang 20250411 Add for 9046AU
            else
                ResetOKDeleyTime.SetSecAndOn(3);

            if(bPickFromHotplate ||                                             //jou 2010-01-19 start : 如果資料未轉移過來,需把IC放回去
               bPickFromLoader)                                                 //Steven 20210903 : 修正Loader吸到一半歸零要放回去
            {
                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        if(flag[i][j]==false)
                        {
                            flag[i][j]=InArmSuck.Suck[i][j].Destroy();
                        }
                    }
                }

                for(int i=0; i<2; i++)
                {
                    for(int j=0; j<4; j++)
                    {
                        if(flag[i][j]==false)
                        {
                            break;
                        }
                    }
                }
            }
            bPickFromLoader=false;                                              //Steven 20210903 : 修正Loader吸到一半歸零要放回去
            bWaitRotateFinish=false;                                            //KevinCheng 20260318 : Rotate流程未完，不可關閉。

            if(bArm1SuckComplete==false)                                        //Steven 20210824 : index歸零時要把IC放回去
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(FTestSuck.Item[i][j]!=NULL_IC)                       //直接把IC資料放回去
                        {
                            FLCarryKit.MoveSuckData(FTestSuck, i, j);
                        }

                        if(FLCarryKit.Item[i][j]!=NULL_IC &&
                           FLCarryKit.Item[i][j]!=HAS_NULL_IC)                  //有料的地方都要吹
                        {
                            FTestSuck.Suck[i][j].Off();
                        }
                    }
                }
            }

            if(bArm2SuckComplete==false)                                        //Steven 20210824 : index歸零時要把IC放回去
            {
                for(int i=0; i<MAX_Index_Row; i++)
                {
                    for(int j=0; j<NEW_MAX_Index_Col; j++)
                    {
                        if(BTestSuck.Item[i][j]!=NULL_IC)                       //直接把IC資料放回去
                        {
                            BLCarryKit.MoveSuckData(BTestSuck, i, j);
                        }

                        if(BLCarryKit.Item[i][j]!=NULL_IC &&
                           BLCarryKit.Item[i][j]!=HAS_NULL_IC)                  //有料的地方都要吹
                        {
                            BTestSuck.Suck[i][j].Off();
                        }
                    }
                }
            }

            fHome->iHomeStep=3;
            break;
        case 3:
            if(ResetOKDeleyTime.Off())
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-ALIGNCCD)
                // 缺相依：同本檔 case 1 那一格 —— `fAutoAlignment` 與 `LOAD_FILE` 全樹 0 命中。
                // 行為：case 3 這一次的「回 Home 時更新 Alignment 工作檔」不會發生。
                //   與 case 1 那一格是同一個模組的兩個落點，解閘時兩處一起。
                #if 0 // GATE (W906-HOME-C2-ALIGNCCD): 缺相依，見上面的就地註解
                if(MACHINE_HAS_AUTO_ALIGNMENT_CCD)                              //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
                {
                    fAutoAlignment->InArmSendCommand(LOAD_FILE);
                }
                #endif // GATE (W906-HOME-C2-ALIGNCCD)


                if(bArm1SuckComplete==false)                                    //Steven 20210824 : index歸零時要把IC放回去
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(FLCarryKit.Item[i][j]!=NULL_IC &&
                               FLCarryKit.Item[i][j]!=HAS_NULL_IC)              //有料的地方都要吹
                            {
                                FTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                }
                bArm1SuckComplete=true;

                if(bArm2SuckComplete==false)                                    //Steven 20210824 : index歸零時要把IC放回去
                {
                    for(int i=0; i<MAX_Index_Row; i++)
                    {
                        for(int j=0; j<NEW_MAX_Index_Col; j++)
                        {
                            if(BLCarryKit.Item[i][j]!=NULL_IC &&
                               BLCarryKit.Item[i][j]!=HAS_NULL_IC)              //有料的地方都要吹
                            {
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                }
                bArm2SuckComplete=true;

                if(Cylinder[C_Shuttle_Knocker_1].Enable)                        //Steven 20130108 : 避免蝦頭敲敲汽缸打開撞斷
                    Cylinder[C_Shuttle_Knocker_1].Off();
                if(Cylinder[C_Shuttle_Knocker_2].Enable)
                    Cylinder[C_Shuttle_Knocker_2].Off();
                if(USE_AUTO_RETEST==eartInstall)                                //ChungHung 20140814 add
                    Cylinder[C_TurnTrayArmLock].Off();

                DoMotorPowerOn();
                if(SubMachineType==Type_HT9046AU)                               //RogerYang 20250411 Add for 9046AU
                    ResetOKDeleyTime.SetSecAndOn(4);
                else
                    ResetOKDeleyTime.SetSecAndOn(2);                            //JerryYang 20230522 : 0.3s -> 2s, Power on過幾秒再解magazin煞車
                fHome->iHomeStep=4;
                if(MOT[MMTrayY].fHasTray==false)
                {
                    Cylinder[C_TrayY_Fixer].Off();
                    Cylinder[C_LoaderEdgePush].Off();
                }
            }
            break;
        case 4:
            if(ResetOKDeleyTime.Off())
            {
                fHome->iHomeStep=5;
            }
            break;
        case 5:
            if(ResetOKDeleyTime.Off()==false)
                break;

            fHome->ListBox1->Clear();
            MOT[MTestY1].Gali_Command("SH", __FUNC__);                          //servo On Gail Motor
            ResetOKDeleyTime.SetSecAndOn(0.3);
            SetNoiseDelay=false;
            fHome->iHomeStep=10;
            break;
        case 10:
            if(ResetOKDeleyTime.Off())
                fHome->iHomeStep=20;
            break;
        case 20:
            MOT[MTestZ1].Gali_Command("ST", __FUNC__);                          // claer all motor command
            StopAllDestroy();
            for(int i=0; i<TOTAL_MOTOR; i++)
                fHome->ShowLed(i, 0);
            fHome->Show();
            //AI(W906-MT-FIX1) 20260926: EastSun ruling 20260926 -- a brake group is released only while its 1203 axes are servo-ON
            //  (W906_BrakeReleaseOK, csystem.h); golden releases here unconditionally.  A held group is released later by
            //  DoSystem's G05 idle pass once its drives are servo-ON (HOME cannot drive that axis before -- the safe direction).
            if(W906_BrakeReleaseOK("Index", "HOME start"))        IndexMotorBreakerON();
            if(W906_BrakeReleaseOK("Magazine", "HOME start"))     MagazineBreakerON();                                                //JerryYang 20220909 : add magazine
            if(W906_BrakeReleaseOK("InOutArmZ", "HOME start"))    InOutArmZBreakerON();                                               //add One sucker with rotate
            if(W906_BrakeReleaseOK("LDCarRotArmZ", "HOME start")) LDCarRotArmZBreakerOn();                                            //RogerYang 20250828 add for Loader Rotate Arm
            if(W906_BrakeReleaseOK("Cassette", "HOME start"))     CassetteBreakerON();                                                //Ifor 20251216 add:Boat Carrier
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                SW[SwAutoCCW[i]].Off();
                SW[SwAutoCW[i]].Off();
            }

            fHome->iHomeStep=100;
            #ifdef SOFT_SIMULTE
                Cylinder[C_TrayX_UpDown].Off();
                fHome->iHomeStep=300;
            #endif
            break;
        case 100:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }
            ResetOKDeleyTime.SetSecAndOn(0.2);
            fHome->iHomeStep=200;
            break;
        case 200:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }

            if(ResetOKDeleyTime.Off())
            {
                fHome->iHomeStep=250;
                fHome->ListBox1->Items->Insert(0, "Tray catcher cylinder up ....");
            }
            break;
        case 250:
            if(Cylinder[C_TrayX_UpDown].Pop())
                fHome->iHomeStep=270;
            break;
        case 270:
            ResetOKDeleyTime.SetSecAndOn(0.1);
            fHome->iHomeStep=280;
            break;
        case 280:
            if(ResetOKDeleyTime.Off())
                fHome->iHomeStep=300;
            break;
        case 300:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }
            SetCylinderResetStateHomeBegin();
            fHome->ResetAllMotorLed();
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                MOT[i].MotorInitial();
                MOT[i].HomeFlag=0;
                fHome->HomeClass[i]->THomeFlag=1;
                if(MOT[i].Motor->Enable==false)
                {
                    fHome->HomeClass[i]->THomeFlag=0;
                    MOT[i].Position=0;
                    MOT[i].fCanMoveL=true;
                    MOT[i].fCanMoveR=true;
                    MOT[i].fCanMoveM=true;
                    MOT[i].fCanMove=true;
                    MOT[i].MotorMove(MOT[i].Position);
                    MOT[i].HomeFlag=1;
                    fHome->ShowLed(i, 1);
                }
            }
            MOT[MTrayX].TrayArmInitial();

            fHome->HomeClass[MInArmXScale]->THomeFlag=0;                        //Steven 20160426 : 磁性尺
            fHome->HomeClass[MInArmYScale]->THomeFlag=0;
            fHome->HomeClass[MOutArmXScale]->THomeFlag=0;
            fHome->HomeClass[MOutArmYScale]->THomeFlag=0;

            //AI(W906-MT-E3b) 20260925: GATE (W906-HOME-C2-GALIHOMETASK) 解閘 —— 兩個理由都過期了：
            //   InitGali_HomeTask() 已照 golden :646-662 翻在本檔（只寫旗標、不下 Galil 命令），
            //   609 個 Gali_* 重複符號的樁 20260920 已退役（Motor/mymotor.cpp:1601 GALI-STUB RETIRED）。以下是原註。
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-GALIHOMETASK)
            // 缺相依：`TfHome::InitGali_HomeTask()`（golden uhome.cpp:646）。
            // 它把 Galil 卡四顆 Index 馬達（MTestY1/Y2/Z1/Z2）的 `iGali_SingalHomeTask`
            // 重設成 1，也就是「Galil 的單軸歸零任務重新開始」。
            // 行為：**只有裝 Galil 運動卡的機台受影響**（`INDEX_MOTION_CARD==0`）。
            //   那種機台上，Index 四軸的 Galil 歸零任務不會被重設，沿用上一次的 task 值。
            //   本樹的 Galil 層（`Motor/myGALILmotor.cpp`）還沒接上 —— 參見
            //   `forms/fHome.cpp` 檔頭記的 609 個 `TMyMotor::Gali_*` 重複符號問題：
            //   Galil 真本體與 `mymotor.cpp` 的樁今天無法同時進同一個連結。
            //   非 Galil 機台（含這台筆電，`MOTION_CARD_TYPE=1`）完全不受影響。
            // UN-GATE：等 Galil 波次（它要先解決那 609 個重複符號）。
            fHome->InitGali_HomeTask();


            //Isaac 20201012 : index Y超過範圍，做一次Tmode
            //=>
            bOverRangeDoTMode=false;
            bOverRange4Indexhome=false;
            bTriger4Indexhome=false;
            iIndexOverRangeCount=0;
            //<=
            //Isaac 20201012 : index Y超過範圍，做一次Tmode
            if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall)
            {
                DoTopBtmHome(true);
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TOPBTMAOI)
                // 缺相依：`TFrmAOI::ttbInsp`（同本檔 DoTopBtmHome 那一格）。
                // 行為：Top/Bottom AOI 的 CCD 光源不會在歸零前降下。
                //   ⚠ 上一行 `DoTopBtmHome(true)` 照留 —— 它的 `bFirst==true` 臂只做
                //   `iTopBtmHome=1; return true;`，不碰 FrmAOI，所以是活的。
                //   下一行 `iHomeStep=305` 也照留，於是流程仍然走到 case 305，
                //   然後卡在那裡（見 DoTopBtmHome 那一格的 🔴 說明）。
                #if 0 // GATE (W906-HOME-C2-TOPBTMAOI): 缺相依，見上面的就地註解
                FrmAOI->ttbInsp->DoCCDLightDown(false, true);
                #endif // GATE (W906-HOME-C2-TOPBTMAOI)
                fHome->iHomeStep=305;
            }
            else
            {
                fHome->iHomeStep=310;
            }
            MotorTask=FIRST_HOME;                                               //第一階段回Home
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                if(fHome->HomeClass[i]->THomeFlag &&
                   fHome->HomeClass[i]->THomeOrder==MotorTask)                  //Z軸先回Home
                {
                    fHome->ShowLed(i, 3);
                    sprintf(str, "M%2d homeing ....", i+1);
                    fHome->ListBox1->Items->Insert(0, str);
                }
            }
            break;
        case 305:                                                               //Jimmychiu 20240307 : Loader Tray改用步進馬達
            if(DoTopBtmHome(false))
            {
                fHome->iHomeStep=310;
            }
            break;
        case 310:                                                               //是否需要讀取扭力值
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TORQUE)
            // 缺相依：`InitWriteAndCheckMotorTorqueTask()`（全樹 0 命中）與
            // `TCOM2Shim::iWriteAndCheckMotorTorque(int,int)`（golden rs232.h ——
            // 透過 COM2 對 Index 伺服下扭力值並讀回確認）。整個扭力讀寫層沒翻。
            // 行為：**歸零時不再做 Index Z 軸的扭力設定與確認**。
            //   golden 在這裡把 MTestZ1/Z2 的扭力設成 80 再設成 300 並逐次確認，
            //   讀不到就報 alarm。閘住之後那些設定與確認都不發生 ——
            //   ⇒ Index 下壓的扭力沿用伺服器內部的既有值。
            //   ⚠ **這一格連帶拿掉一個 alarm**：golden 在扭力讀不回來時會報錯擋住歸零。
            //   ⚠ 只影響 `SW[SwReadTorue].Enable` 的機台（要讀扭力值的配置）。
            //     本格把 `if` 整個換成走 else，也就是**直接跳到 case 350**，
            //     與那些機台上「不需要讀扭力值」的路徑完全相同。
            // UN-GATE：等 COM2 的 iWriteAndCheckMotorTorque 與 InitWriteAndCheckMotorTorqueTask
            //   翻進來，本檔三個落點（case 310/320/322）一起解。
            // AI(W906-R28TORQ) 20260925: 條件成立，三個落點照上面那句一起解（golden uhome.cpp:2491-2566，逐字）。
            //   兩個相依都翻了：TCOM2Shim::iWriteAndCheckMotorTorque（rs232.cpp，golden rs232.cpp:1847）與
            //   InitWriteAndCheckMotorTorqueTask（rs232.cpp，golden rs232.cpp:1838）；使用者 20260925 裁決第 6 條
            //   「Index Z 扭力上限對齊 BCB6」。歸零時照 golden 把兩支 Z 的扭力上限寫回 300 並讀回確認；
            //   寫不進／讀不回就報「Motor MTestZ1/Z2 torque set error!!」並中止歸零（golden 的行為）。
            //   ⚠ 只在 SW[SwReadTorue].Enable 的機台走（IO_Table 有配 SwReadTorue；SOFT_SIMULTE 下 cinitial.cpp 把它關掉）。
            //   ⚠ 以前 case 320/322 補的 ret=1 是「沒做卻回成功」—— 已拿掉。
            if(SW[SwReadTorue].Enable)
            {
                InitWriteAndCheckMotorTorqueTask();
                fHome->iHomeStep=320;
                fHome->ListBox1->Items->Insert(0, "MTestZ1 set Torque 80...");
            }
            else
            {
                fHome->iHomeStep=350;
            }
            break;
        case 320:                                                               //Arm 1 扭力讀取是否正常
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TORQUE)
            // 缺相依：`TCOM2Shim::iWriteAndCheckMotorTorque(int,int)` 與
            // `InitWriteAndCheckMotorTorqueTask()`（同 case 310 那一格）。
            // 行為：這一個 case 的扭力設定與確認整段不做。
            //   ⚠ 把 `ret` 直接設成 1（＝golden 的「讀回正常」），流程照走，
            //     否則 case 320/322 會卡死等一個永遠不會來的值。
            //     **補的值是「成功」，而它其實沒做** —— 與 PITCHX2 那一格同一種取捨，
            //     理由也相同：卡死比「少做一件事」嚴重。
            // AI(W906-R28TORQ) 20260925: 解閘（見 case 310 的說明）；補的 ret=1 拿掉 —— golden uhome.cpp:2510
            ret=COM2->iWriteAndCheckMotorTorque(0, 300);

            if(ret==1)
            {
                fHome->ListBox1->Items->Insert(0, "MTestZ1 set Torque 300...");                                         //kevin 20130425
                // AI(W906-R28TORQ) 20260925: 解閘（見 case 310 的說明）—— golden uhome.cpp:2515
                InitWriteAndCheckMotorTorqueTask();

                if(bContaceTorque==false &&
                   IniConfig.bKoreaFunction==false &&
                   CUSTOMER_CODE!=CC_TSMC_TAINAN &&
                   IniConfig.bSPILFunction==false)                              //JerryYang 20230204 : Add SPIL support回傳GPIB force指令
                {
                    asArmForce1="";                                             //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
                }
                fHome->iHomeStep=322;
            }
            else if(ret==2)
            {
                ShowMyMessage("Motor MTestZ1 torque set error!!", "MTestZ1馬達壓力設定錯誤", "ProcessMotorHome 320");
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return true;
            }
            break;
        case 322:                                                               //Arm 2 扭力讀取是否正常
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
                ret=1;
            }

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-TORQUE)
            // 缺相依：`TCOM2Shim::iWriteAndCheckMotorTorque(int,int)` 與
            // `InitWriteAndCheckMotorTorqueTask()`（同 case 310 那一格）。
            // 行為：這一個 case 的扭力設定與確認整段不做。
            //   ⚠ 把 `ret` 直接設成 1（＝golden 的「讀回正常」），流程照走，
            //     否則 case 320/322 會卡死等一個永遠不會來的值。
            //     **補的值是「成功」，而它其實沒做** —— 與 PITCHX2 那一格同一種取捨，
            //     理由也相同：卡死比「少做一件事」嚴重。
            // AI(W906-R28TORQ) 20260925: 解閘（見 case 310 的說明）；補的 ret=1 拿掉 —— golden uhome.cpp:2543
            ret=COM2->iWriteAndCheckMotorTorque(1, 300);

            if(ret==1)
            {
                // AI(W906-R28TORQ) 20260925: 解閘（見 case 310 的說明）—— golden uhome.cpp:2547
                InitWriteAndCheckMotorTorqueTask();

                fHome->ListBox1->Items->Insert(0, "MTestZ2 set Torque 300...");                                         //kevin 20130425

                if(bContaceTorque==false &&
                   IniConfig.bKoreaFunction==false &&
                   CUSTOMER_CODE!=CC_TSMC_TAINAN &&
                   IniConfig.bSPILFunction==false)                              //JerryYang 20230204 : Add SPIL support回傳GPIB force指令
                {
                    asArmForce2="";                                             //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
                }
                fHome->iHomeStep=350;
            }
            else if(ret==2)
            {
                ShowMyMessage("Motor MTestZ2 torque set error!!", "MTestZ2馬達壓力設定錯誤", "ProcessMotorHome 322");
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return true;
            }
            break;
        case 350:
            fHome->iHomeStep=375;
            MOT[MTestY1].iGali_SingalHomeTask=1;
            MOT[MTestY2].iGali_SingalHomeTask=1;
            MOT[MTestZ1].iGali_SingalHomeTask=1;
            MOT[MTestZ2].iGali_SingalHomeTask=1;
            MOT[MTestZ1].iGali_FindZPhaseTask[1]=1;                             //Ifor 20170817 (wei) add Z Phase Task
            MOT[MTestZ2].iGali_FindZPhaseTask[2]=1;                             //Ifor 20170817 (wei) add Z Phase Task
            MOT[MTestY1].iGali_FindZPhaseTask[0]=1;                             //Isaac 20201110 : Index Y find motor phase
            MOT[MTestY2].iGali_FindZPhaseTask[3]=1;                             //Isaac 20201110 : Index Y find motor phase

            if(IniConfig.bP37bAutoCylinderUP)                                   //kevin 20180726 Auto 123 氣缸常態在上
            {                                                                   //kevin 20180726  Auto 123 氣缸常態在上
                fHome->iHomeStep=360;
                for(int i=eAuto1; i<=iAutoRight; i++)
                {
                    bflag[i]=false;                                             //升到接盤位置
                    AutoCylinderLower(i, C_Auto_Up[i], C_Auto_Selector[i], true);                                       //Steven 20140409 : AutoCylinderXX Add Reset
                    AutoCylinderMiddle(i, C_Auto_Up[i], C_Auto_Selector[i],true);                                       //升到接盤位置
                    AutoCylinderUp(i, C_Auto_Up[i], C_Auto_Selector[i],true);   //升到接盤位置
                }
            }
            break;
        case 360:                                                               //kevin 20180726  Auto 123 氣缸常態在上
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(Sen[SnAutoTrackDetect[i]].IsOn() ||
                   Sen[SnAutoTrayCar[i]].IsOn())                                //軌道上有tray
                {
                    bflag[i]=true;                                              //kevin 20180726 Auto 12 3 up
                    flag1=true;
                }
            }

            if(flag1)                                                           //kevin 20180720  Auto 123 氣缸常態在上
                fHome->iHomeStep=361;
            else
                fHome->iHomeStep=362;
            break;
        case 361:
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(bflag[i])                                                    //kevin 20180726 Auto 12 3 up
                {
                    ibuffer++;
                    if(AutoCylinderMiddle(i, C_Auto_Up[i], C_Auto_Selector[i]))                                         //升到接盤位置
                    {
                       Cylinder[C_AutoZ_Select[i]].On();
                       bflag[i]=false;
                    }
                }
            }

            if(ibuffer==0)
                fHome->iHomeStep=362;
            break;
        case 362:
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(AutoCylinderUp(i, C_Auto_Up[i], C_Auto_Selector[i]))         //升到接盤位置
                {
                    Cylinder[C_AutoZ_Select[i]].Off();
                    bflag[i]=true;
                }
            }

            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                if(bflag[i])                                                    //升到接盤位置
                {
                   ibuffer++;
                }
            }

            if(ibuffer==3)
                fHome->iHomeStep=375;
            break;
        case 375:                                                               //所有Z軸回Home中
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }
            flag1=true;
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                ret=0;
                if(fHome->HomeClass[i]->THomeFlag &&
                   fHome->HomeClass[i]->THomeOrder==MotorTask)
                {
                    if(INDEX_MOTION_CARD==0 && (i==MTestZ1 || i==MTestZ2))      //Index的Z軸         //Steven 20210623 : Index使用Galil
                    {
                        if(MOT[i].Motor->Enable)
                        {
                            if(MOT[i].Led[iCcwLed] || MOT[i].Led[iCwLed])
                            {
                                iRef=MOT[i].GetErrorIndex();
                                if(iRef==9) iRef=7;
                                JamCode=MotorIndexToJamCode(i);
                                ShowMotorErrorMessage(JamCode, iRef+1);
                                fHome->fAbort=false;
                                SoftStop=true;
                                return true;
                            }

                            if(MOT[i].Gali_SingalHome(true)==true)              //RogerYang 20161116 Index Z 第一次回Home, flag=true
                            {
                                if(MOT[i].Led[iServoOn]==false)
                                {
                                    StopAllMotor();
                                    JamCode=MotorIndexToJamCode(i);
                                    ShowMotorErrorMessage(JamCode, 0);
                                    fHome->fAbort=false;
                                    SoftStop=true;
                                    return true;
                                }

                                fHome->HomeClass[i]->THomeFlag=0;
                                fHome->ShowLed(i, 1);
                                sprintf(str, "M%02d home finish.", i+1);
                                fHome->ListBox1->Items->Insert(0, str);
                            }
                        }
                        else
                        {
                            fHome->HomeClass[i]->THomeFlag=0;
                            fHome->ShowLed(i, 1);
                            sprintf(str, "M%02d home finish.", i+1);
                            fHome->ListBox1->Items->Insert(0, str);
                        }
                    }
                    else
                    {
                        if(i==MInArmY)
                        {
                            if(MOT[MInArmPitch].HomeFlag==1)
                            {
                                if(bCheckInArmX==true)
                                {
                                    ret=0;
                                    MOT[MInArmX].ScanMotorStatus();
                                    if(MOT[MInArmX].Motor->HomeFlag())
                                    {
                                        MOT[MInArmX].PCIL132_StopMotor();
                                        bCheckInArmX=false;
                                    }
                                }
                                else
                                {
                                    ret=MOT[i].MotorHome(flag1);
                                }
                            }
                            else
                            {
                                ret=0;
                            }
                        }
                        else
                        {
                            ret=MOT[i].MotorHome(flag1);
                        }
                    }

                    if(MOT[i].HomeFlag==0 && ret==2)
                    {
                        StopAllMotor();
                        fHome->ShowLed(i, 2);
                        fHome->iHomeStep=1;
                        SystemStart=false;
                        iRef=MOT[i].GetErrorIndex();
                        if(iRef==9) iRef=7;
                        JamCode=MotorIndexToJamCode(i);
                        ShowMotorErrorMessage(JamCode, iRef+1);
                        fHome->fAbort=false;
                        SoftStop=true;
                        return true;
                    }

                    if(ret==4)
                    {
                        StopAllMotor();
                        fHome->ShowLed(i, 2);
                        JamCode=MotorIndexToJamCode(i);
                        ShowMotorErrorMessage(JamCode, 0);
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        return true;
                    }
                    else if(ret==3)
                    {
                        StopAllMotor();
                        fHome->ShowLed(i, 2);
                        iRef=MOT[i].GetErrorIndex();
                        if(iRef==9) iRef=6;
                        sprintf(str, "Mot%04d", 5000+i*10+iRef);
                        JamCode=MotorIndexToJamCode(i);
                        ShowMotorErrorMessage(JamCode, iRef+1);
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        return true;
                    }
                    else if(ret==1 && MOT[i].HomeFlag==1)
                    {
                        MOT[i].HomeFlag=1;
                        fHome->HomeClass[i]->THomeFlag=0;
                        fHome->ShowLed(i, 1);
                        sprintf(str, "M%02d home finish", i+1);
                        fHome->ListBox1->Items->Insert(0, str);
                        MOT[i].Position=0;
                    }
                    else
                    {
                        flag1=false;
                    }
                }

                if(!fHome->ShowMotorHomePos(i))
                {
                    return true;
                }
            }

            if(flag1)
            {
                if(MotorTask==FIRST_HOME)
                {
                    Cylinder[C_FixTray_FullPlace].Off();                        //ChungHung 20140313 add Fix3 can Full Tray
                    if(CheckInArmSuckICFallDownToHasNullIC()==true)             //JerryYang 20200424 回home時新增掉料偵測
                    {
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        return true;
                    }

                    if(CheckOutArmSuckICFallDown()==true)                       //JerryYang 20200424 回home時新增掉料偵測
                    {
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        return true;
                    }

                    for(int i=0; i<4; i++)
                    {
                        if(Cylinder[C_TrayXFloodgate1+i].Enable)
                        {
                            Cylinder[C_TrayXFloodgate1+i].On();
                            bCyflag[i]=false;
                        }
                        else
                        {
                            bCyflag[i]=true;
                        }
                    }

                    MotorTask=SECOND_HOME;
                    fHome->iHomeStep=400;
                }
            }
            break;
        case 400:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }

            for(int i=0; i<CynForHome; i++)                                     //RogerYang 20250909 : 雙動氣缸 pop側先關
            {
                if(CylinderInitSwitchType[i]==false)
                {
                    Cylinder[CynNeedHome[i]].Off();
                    CylinderInitOKFlag[i]=true;
                }
            }

            flag1=true;
            for(int i=0; i<CynForHome; i++)
            {
                if(CylinderInitOKFlag[i]==false)
                {
                    if(CylinderInitState[i])
                    {
                        if(Cylinder[CynNeedHome[i]].Push())
                            CylinderInitOKFlag[i]=true;
                        else
                            flag1=false;
                    }
                    else
                    {
                        if(Cylinder[CynNeedHome[i]].Pop())
                            CylinderInitOKFlag[i]=true;
                        else
                            flag1=false;
                    }
                }
            }

//            if(bCyflag[0]==false)
//                bCyflag[0]=(Cylinder[C_TrayXFloodgate1].Enable==false || Cylinder[C_TrayXFloodgate1].Push());
//            if(bCyflag[1]==false)
//                bCyflag[1]=(Cylinder[C_TrayXFloodgate2].Enable==false || Cylinder[C_TrayXFloodgate2].Push());
//            if(bCyflag[2]==false)
//                bCyflag[2]=(Cylinder[C_TrayXFloodgate3].Enable==false || Cylinder[C_TrayXFloodgate3].Push());
//            if(bCyflag[3]==false)
//                bCyflag[3]=(Cylinder[C_TrayXFloodgate4].Enable==false || Cylinder[C_TrayXFloodgate4].Push());
//
//            if(!(bCyflag[0] && bCyflag[1] && bCyflag[2] && bCyflag[3]))
//                flag1=false;

            if(MOT[MTrayX].fHasTray ||                                          //jou 2012-10-26 修正歸零catch tray 會把tray丟在中途
               (LastSet.iRunStartMode==rsmAutoRetest &&
                Cylinder[C_CatchTray_FixOn].OnStatus()) ||
               (USE_AUTO_RETEST==eartInstall &&                                 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                Cylinder[C_CatchTray_FixOn].OnStatus()))                        //kevin 20160920
            {
                if(USE_AUTO_RETEST==eartInstall)                                //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    if(Cylinder[C_CatchTray_FixOn].OnSensor()==false)           //kevin 20160920 沒有tray
                    {
                        MOT[MTrayX].fHasTray=false;
                        Cylinder[C_CatchTray_FixOn].Off();
                        Cylinder[C_CatchTray_FixOff].On();
                    }
                    else
                    {
                        if(MOT[MTrayX].fHasTray==false)                         //JerryYang 20251014 : tray arm沒資料但是偵測到有tray盤的時候跳一下alarm
                        {
                            Str.sprintf("Tray arm detect a tray on it. Please check if there is a tray!(C_CatchTray_FixOn_On)");
                            ShowMyMessage(Str);
                        }
                        MOT[MTrayX].fHasTray=true;                              //ChungHung 20141208 add for AutoRetest mode 中途關程式 catch Tray 要搬到loader
                        Cylinder[C_CatchTray_FixOn].On();
                        Cylinder[C_CatchTray_FixOff].Off();
                    }
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].On();
                }
            }
            else
            {
                if(USE_AUTO_RETEST==eartInstall)                                //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    Cylinder[C_CatchTray_FixOn].Off();
                    Cylinder[C_CatchTray_FixOff].On();
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].Off();
                }
            }

            if(flag1)
            {
                if(USE_AUTO_RETEST==eartInstall)                                //ChungHung 20140814 add
                {
                    if(Cylinder[C_TurnTrayArmLock].Push())
                        fHome->iHomeStep=500;
                }
                else                                                            //Steven 20140827
                {
                    fHome->iHomeStep=500;
                }
            }
            break;
        case 500:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }
            fHome->iHomeStep=600;
            MotorTask=SECOND_HOME;
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                if(fHome->HomeClass[i]->THomeFlag &&
                   fHome->HomeClass[i]->THomeOrder==MotorTask)
                {
                    fHome->ShowLed(i, 3);
                    sprintf(str, "M%2d homeing ....", i+1);
                    fHome->ListBox1->Items->Insert(0, str);
                }
            }

            if(USE_PICKER_COUNT==ep16Picker &&
               USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                         //Ztex 2023.12.15 Add Pitch X Home Twice
            {
                bPitchHome_Twice=true;
                bPitchHome[0]=false;
                bPitchHome[1]=false;
                fHome->iHomeStep=510;
            }

            #ifdef DEBUG_HOME                                                   //wei (Steven) 20170317 快速回Home開關

            IndexY1=false;                                                      //kevin 20161214 (Steven) Increase Hone Speed
            IndexY2=false;
            IndexZ1=false;
            IndexZ2=false;
            #endif
            break;
        case 510:                                                               //Ztex 2023.12.15 Add Pitch X Home Twice
            for(int i=0;i<4;i++)
                iPitch_Move[i]=0;
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-PITCHX2)
            // 缺相依：`TfHome::MoveInArmPitch_X(int*,bool)` / `MoveOutArmPitch_X(int*,bool)`
            // （golden uhome.cpp:5100 附近，"Ztex 2023.12.15 Add Pitch X Home Twice"）。
            // 那兩支本身在 golden 的 `ProcessMotorHome` 之外（:4842 之後），本波的抽取
            // 範圍是 :719-4842，**沒有包含它們**。
            // 行為：`USE_PICKER_COUNT==ep16Picker && USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be`
            //   的機台上，In/Out Arm 的 Pitch X **不會做第二次歸零**。
            //   golden 加這一步是為了修「Pitch X 移動位置錯誤」。
            //   ⚠ 這裡把 `bPitchHome[0]/[1]` 直接設成 true（見下面那兩行），
            //     讓流程照樣往下走 —— 否則 case 510 會永遠等不到而卡死，
            //     那比「少做第二次歸零」嚴重得多。
            //     **這是本批唯一一處「閘住之後補一個值讓流程繼續」**，所以寫明：
            //     補的值是 `true`＝「這一步做完了」，而它其實沒做。
            //   16-picker 以外的機台（含這台筆電）根本不走這一段。
            // UN-GATE：把 golden uhome.cpp :4843 之後的 MoveInArmPitch_X / MoveOutArmPitch_X
            //   一起翻進來（它們是本檔的下一波）。
            #if 0 // GATE (W906-HOME-C2-PITCHX2): 缺相依，見上面的就地註解
            if(bPitchHome[0]==false)
            {
                bPitchHome[0]=fHome->MoveInArmPitch_X(iPitch_Move, true);
            }

            if(bPitchHome[1]==false)
            {
                bPitchHome[1]=fHome->MoveOutArmPitch_X(iPitch_Move, true);
            }
            #endif // GATE (W906-HOME-C2-PITCHX2)
            bPitchHome[0]=true;   // 見上面 GATE (W906-HOME-C2-PITCHX2)
            bPitchHome[1]=true;   // 同上

            if(bPitchHome[0]==true && bPitchHome[1]==true)
            {
                bPitchHome[0]=false;
                bPitchHome[1]=false;
                fHome->iHomeStep=520;
            }

            break;
        case 520:                                                               //Ztex 2023.12.15 Add Pitch X Home Twice
            for(int i=0;i<4;i++)
                iPitch_Move[i]=-200;
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-PITCHX2)
            // 缺相依：同 case 510 那一格（`TfHome::MoveIn/OutArmPitch_X`，golden :4843 之後）。
            // 行為：Pitch X 的第二次歸零（-200 這一段）不做；旗標補 true 讓流程往下。
            #if 0 // GATE (W906-HOME-C2-PITCHX2): 缺相依，見上面的就地註解
            if(bPitchHome[0]==false)
            {
                bPitchHome[0]=fHome->MoveInArmPitch_X(iPitch_Move, true);
            }

            if(bPitchHome[1]==false)
            {
                bPitchHome[1]=fHome->MoveOutArmPitch_X(iPitch_Move, true);
            }
            #endif // GATE (W906-HOME-C2-PITCHX2)
            bPitchHome[0]=true;   // 見上面 GATE (W906-HOME-C2-PITCHX2)
            bPitchHome[1]=true;   // 同上

            if(bPitchHome[0]==true && bPitchHome[1]==true)
            {
                bPitchHome[0]=false;
                bPitchHome[1]=false;
                fHome->iHomeStep=600;
            }
            break;
        case 600:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }
            flag1=true;
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                if(fHome->HomeClass[i]->THomeFlag &&
                   fHome->HomeClass[i]->THomeOrder==MotorTask)
                {
                    ret=0;
                    if(INDEX_MOTION_CARD==0 && (i==MTestY1 || i==MTestY2))      //Steven 20210623 : Index使用Galil
                    {
                        if(MOT[i].Motor->Enable)
                        {
                            #ifdef DEBUG_HOME                                   //wei (Steven) 20170317 快速回Home開關
                            if(MOT[i].Led[iCcwLed] ||
                               MOT[i].Led[iCwLed] &&
                               MOT[i].Led[iHomeLed])                            //kevin 20170209 add
                            {
                                iRef=MOT[i].GetErrorIndex();
                                if(iRef==9) iRef=7;
                                JamCode=MotorIndexToJamCode(i);
                                ShowMotorErrorMessage(JamCode, iRef+1);
                                fHome->fAbort=false;
                                SoftStop=true;
                                return true;
                            }
                            #else

                            #endif
                            if(MOT[i].Gali_SingalHome())
                            {
                                if(MOT[i].Led[iServoOn]==false)
                                {
                                    StopAllMotor();
                                    JamCode=MotorIndexToJamCode(i);
                                    ShowMotorErrorMessage(JamCode, 8);
                                    fHome->fAbort=false;
                                    SoftStop=true;
                                    return true;
                                }

                                fHome->HomeClass[i]->THomeFlag=0;
                                fHome->ShowLed(i, 1);
                                sprintf(str, "M%02d home finish.", i+1);
                                fHome->ListBox1->Items->Insert(0, str);
                                MOT[i].Position=0;
                            }
                            else if(MOT[i].Led[iAlarmLed])                      //Steven 20230708 : Galil歸零要可以Alarm
                            {
                                iRef=MOT[i].GetErrorIndex();
                                if(iRef==9)
                                    iRef=7;
                                JamCode=MotorIndexToJamCode(i);
                                ShowMotorErrorMessage(JamCode, iRef+1);
                                fHome->fAbort=false;
                                SoftStop=true;
                                return true;
                            }
                        }
                        else
                        {
                            MOT[i].Position=0;
                            MOT[i].HomeFlag=1;
                            fHome->HomeClass[i]->THomeFlag=0;
                            fHome->ShowLed(i, 1);
                            sprintf(str, "M%02d home finish.", i+1);
                            fHome->ListBox1->Items->Insert(0, str);
                        }
                    }
                    else
                    {
                        ret=MOT[i].MotorHome(flag1);
                    }

                    if(i==MLoaderY)                                             //RogerYang 20251105 : 修正撞機問題
                    {
                        if(INSTALL_OCR_YMot==eocrYMotUninstal)                  //RogerYang 20250909 : 只要是Motor就要回home//Frank 20250214 add
                        {
                            MOT[i].HomeFlag=1;
                            ret=1;
                            MOT[i].PCIL132_SetPos(0);
                        }

                        if(INSTALL_OCR!=eocrUninstal &&
                           CosFunction.bTrayOCR)                                //wei 20150925
                        {
                            if(bOCRHomeActionPass)
                            {
                                Cylinder[C_OCRLight_Up].Off();
                                if(IniConfig.OCRLightChange)                    //wei 20181225 光源auto change
                                {
                                    fOCR->ChangeLightValue(1, 0);
                                    fOCR->ChangeLightValue(2, 0);
                                }
                                bOCRHomeActionPass=false;
                            }
                        }
                    }

                    if(MOT[i].HomeFlag==0 && ret==2)
                    {
                        StopAllMotor();
                        fHome->ShowLed(i, 2);
                        fHome->iHomeStep=1;
                        SystemStart=false;
                        iRef=MOT[i].GetErrorIndex();
                        if(iRef==9) iRef=7;
                        JamCode=MotorIndexToJamCode(i);
                        ShowMotorErrorMessage(JamCode, iRef+1);
                        fHome->fAbort=false;
                        SoftStop=true;
                        return true;
                    }

                    if(ret==4)
                    {
                        StopAllMotor();
                        fHome->ShowLed(i, 2);
                        JamCode=MotorIndexToJamCode(i);
                        ShowMotorErrorMessage(JamCode, 8);
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        return true;
                    }
                    else if(ret==3)
                    {
                        StopAllMotor();
                        fHome->ShowLed(i, 2);
                        iRef=MOT[i].GetErrorIndex();
                        if(iRef==9) iRef=6;
                        JamCode=MotorIndexToJamCode(i);                         //alarm message problem
                        ShowMotorErrorMessage(JamCode, iRef+1);
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        return true;
                    }
                    else if(ret==1 && MOT[i].HomeFlag==1)
                    {
                        MOT[i].HomeFlag=1;
                        fHome->HomeClass[i]->THomeFlag=0;
                        fHome->ShowLed(i, 1);
                        sprintf(str, "M%02d home finish.", i+1);
                        fHome->ListBox1->Items->Insert(0, str);
                        MOT[i].Position=0;
                    }
                    else
                    {
                        flag1=false;
                    }
                }

                if(!fHome->ShowMotorHomePos(i))
                    return true;
            }

            if(Sen[SnMagazineTrackDetect].IsOn()==true)                         //JerryYang 20220909 : add magazine
            {
                // Alarm : Track has tray can not up/down
                ShowMyMessage("SnMagazineTrackDetect is on, Magazine can not move");
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return false;
            }

            if(Sen[SnMagazineTrackDetect2].IsOn()==true)                        //Sam 20221116 : Magazine TrayArm 自動補 Tray
            {                                                                   // Alarm : Track has tray can not up/down
                ShowMyMessage("SnMagazineTrackDetect2 is on, Magazine can not move");
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return false;
            }

            if(Cylinder[C_OCRLight_Up].OnSensor()==true)
                flag11=true;

            if(flag1 && flag11)
            {
                if(MotorTask==FIRST_HOME)
                {
                    for(int i=0; i<TOTAL_MOTOR; i++)
                        if(fHome->HomeClass[i]->THomeFlag &&
                           fHome->HomeClass[i]->THomeOrder==MotorTask)
                            fHome->ShowLed(i, 3);
                    MotorTask=SECOND_HOME;
                }
                else if(MotorTask==SECOND_HOME)
                {
                    if(USE_PICKER_COUNT==ep16Picker &&                          //Ztex 2023.12.15 Add Pitch X Home Twice  //Ztex 2024.01.14 Modify Pitch X Move Pos Error Issue
                       USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be &&
                       bPitchHome_Twice==true)
                    {
                        for(int i=0;i<4;i++)
                            iPitch_Move[i]=-500;

                        //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-PITCHX2)
                        // 缺相依：`TfHome::MoveInArmPitch_X(int*,bool)` / `MoveOutArmPitch_X(int*,bool)`
                        // （golden uhome.cpp:5100 附近，"Ztex 2023.12.15 Add Pitch X Home Twice"）。
                        // 那兩支本身在 golden 的 `ProcessMotorHome` 之外（:4842 之後），本波的抽取
                        // 範圍是 :719-4842，**沒有包含它們**。
                        // 行為：`USE_PICKER_COUNT==ep16Picker && USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be`
                        //   的機台上，In/Out Arm 的 Pitch X **不會做第二次歸零**。
                        //   golden 加這一步是為了修「Pitch X 移動位置錯誤」。
                        //   ⚠ 這裡把 `bPitchHome[0]/[1]` 直接設成 true（見下面那兩行），
                        //     讓流程照樣往下走 —— 否則 case 510 會永遠等不到而卡死，
                        //     那比「少做第二次歸零」嚴重得多。
                        //     **這是本批唯一一處「閘住之後補一個值讓流程繼續」**，所以寫明：
                        //     補的值是 `true`＝「這一步做完了」，而它其實沒做。
                        //   16-picker 以外的機台（含這台筆電）根本不走這一段。
                        // UN-GATE：把 golden uhome.cpp :4843 之後的 MoveInArmPitch_X / MoveOutArmPitch_X
                        //   一起翻進來（它們是本檔的下一波）。
                        #if 0 // GATE (W906-HOME-C2-PITCHX2): 缺相依，見上面的就地註解
                        if(bPitchHome[0]==false)
                        {
                            bPitchHome[0]=fHome->MoveInArmPitch_X(iPitch_Move, false);
                        }

                        if(bPitchHome[1]==false)
                        {
                            bPitchHome[1]=fHome->MoveOutArmPitch_X(iPitch_Move, false);
                        }
                        #endif // GATE (W906-HOME-C2-PITCHX2)
                        bPitchHome[0]=true;   // 見上面 GATE (W906-HOME-C2-PITCHX2)
                        bPitchHome[1]=true;   // 同上

                        if(bPitchHome[0]==true && bPitchHome[1]==true)
                        {
                            bPitchHome_Twice=false;
                            for(int i=0;i<TOTAL_MOTOR;i++)
                            {
                                if(i==MInArmPitch  || i==MInArmPitchX2  || i==MInArmPitchX3  || i==MInArmPitchX4 ||
                                   i==MOutArmPitch || i==MOutArmPitchX2 || i==MOutArmPitchX3 || i==MOutArmPitchX4)
                                {
                                    fHome->HomeClass[i]->THomeFlag=1;
                                }
                            }
                            fHome->iHomeStep=600;
                            break;
                        }
                    }
                    else
                    {
                        MotorTask=THREE_HOME;
                        fHome->iHomeStep=650;
                        //----- by dell ccd realtime-------------
                        //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-RTCHOME)
                        // 缺相依：`TCOM2Shim::SendCommToVision()` / `rtHome`（同本檔的 RTCTIMESYNC 閘）。
                        // 行為：**不通知 RTC CCD「handler 要歸零了」**。
                        //   ⚠ `hRealCCDTimeOut.SetSecAndOn(2)` 刻意留在閘外面：它是本樹自己的計時器，
                        //     case 650 會讀它。讓它照常起算，case 650 的逾時判斷才有意義。
                        #if 0 // GATE (W906-HOME-C2-RTCHOME): 缺相依，見上面的就地註解
                        if(REAL_TIME_CCD==true)                                 //不管有沒有開RTC都要歸零
                        {
                            COM2->SendCommToVision(COM2->rtHome, true);
                        }
                        #endif // GATE (W906-HOME-C2-RTCHOME)
                        if(REAL_TIME_CCD==true)
                        {
                            hRealCCDTimeOut.SetSecAndOn(2);                     // 見上面 GATE (W906-HOME-C2-RTCHOME)
                        }
                        //---------------------------------------
                        #ifdef DEBUG_HOME                                       //wei (Steven) 20170317 快速回Home開關
                        MOT[MTestY1].iGali_SingalHomeTask=1;                    //kevin 20161214 (Steven) Increase Hone Speed
                        MOT[MTestY2].iGali_SingalHomeTask=1;
                        MOT[MTestZ1].iGali_SingalHomeTask=1;
                        MOT[MTestZ2].iGali_SingalHomeTask=1;
                        MOT[MTestZ1].iGali_FindZPhaseTask[1]=1;                 //Ifor 20170817 (wei) add Z Phase Task
                        MOT[MTestZ2].iGali_FindZPhaseTask[2]=1;                 //Ifor 20170817 (wei) add Z Phase Task
                        MOT[MTestY1].iGali_FindZPhaseTask[0]=1;                 //Isaac 20201110 : Index Y find motor phase
                        MOT[MTestY2].iGali_FindZPhaseTask[3]=1;                 //Isaac 20201110 : Index Y find motor phase
                        #endif
                    }
                }
                else if(MotorTask==THREE_HOME)
                {
                    fHome->iHomeStep=700;

                    for(int i=0; i<4; i++)
                    {
                        if(Cylinder[C_TrayXFloodgate1+i].Enable)
                        {
                            bCyflag[i]=false;
                        }
                        else
                        {
                            bCyflag[i]=true;
                        }
                    }
                }
            }
            break;
        case 650:                                                               //Tray Arm歸零
            //----- by dell ccd realtime-------------
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-RTCHOME)
            // 缺相依：`TCOM2Shim` 缺 `bRealTimeCom_ReceiveOK[]` / `rtHome` /
            // `OpenRTCComPortAgain()` / `SendCommToVision()`（同上）。
            // 行為：case 650 **不再等 RTC CCD 回覆歸零完成**。
            //   ⚠ 這一格拿掉一個 `break`（golden 在沒收到回覆時中止這一個 tick，
            //     等下一次再問）。閘住之後 case 650 直接往下走。
            //   ⚠ 也拿掉 `WAR0338 RTC Home Error!` 這個 alarm ——
            //     RTC 連不上時 golden 會重開 COM port 並報警，本樹不會。
            //   ⇒ 只影響 `REAL_TIME_CCD==true` 的機台。沒有 RTC 的機台走不到這一段。
            #if 0 // GATE (W906-HOME-C2-RTCHOME): 缺相依，見上面的就地註解
            if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum)
            {
                if(COM2->bRealTimeCom_ReceiveOK[COM2->rtHome]==false)
                {
                    if(hRealCCDTimeOut.Off())
                    {
                        if(COM2->OpenRTCComPortAgain())                         //ChungHung 20121005 add
                            ShowErrorMessage("WAR0338", 0, MMIndex, 0, __FUNC__);                                       //RTC Home Error!
                        COM2->SendCommToVision(COM2->rtHome, true);
                        hRealCCDTimeOut.SetSecAndOn(2);
                    }
                    break;
                }
            }
            #endif // GATE (W906-HOME-C2-RTCHOME)

            //---------------------------------------
            if(CheckInArmSuckICFallDownToHasNullIC()==true)                     //JerryYang 20200424 回home時新增掉料偵測
            {
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return true;
            }

            if(CheckOutArmSuckICFallDown()==true)                               //JerryYang 20200424 回home時新增掉料偵測
            {
                fHome->fAbort=false;
                SystemStart=false;
                fHome->iHomeStep=1;
                SoftStop=true;
                return true;
            }
            MotorTask=THREE_HOME;
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                if(fHome->HomeClass[i]->THomeFlag &&
                   fHome->HomeClass[i]->THomeOrder==MotorTask)
                {
                    fHome->ShowLed(i, 3);
                }
            }
            fHome->iHomeStep=600;
            break;
        case 700:
            #ifdef DEBUG_HOME                                                   //wei (Steven) 20170317 快速回Home開關
            fHome->iHomeStep=710;
            #else
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 700"))                       //Steven 20140828 : 歸零後檢查Index位置    //JerryYang 20160905 10 --> Prod.TestZ1_Safe
                fHome->iHomeStep=710;
            #endif
            break;
        case 710:
            #ifdef DEBUG_HOME                                                   //wei (Steven) 20170317 快速回Home開關
            fHome->iHomeStep=750;                                               //kevin 20161214 (Steven) Increase Hone Speed
            #else
            if(MOT[MTestY1].GalilTwoY_Move(-10, 10, 60000, "ProcessMotorHome 710"))                                     //Steven 20140828 : 歸零後檢查Index位置
            {
                fHome->iHomeStep=750;
            }
            #endif
            break;
        case 750:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }
//            SetCylinderResetStateHomeAfter();                                 //Steven 20240123 : 完全不會執行, mark
            fHome->iHomeStep=800;
            break;
        case 800:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }

            flag1=true;
            if(flag1)
            {
                fHome->iHomeStep=900;
            }
            break;
        case 900:
            if(fHome->fShow==false)
            {
                flag1=false;
                SoftStop=true;
                break;
            }
            fHome->iHomeStep=1000;
            fHome->ListBox1->Items->Insert(0, "Cylinder Auto 1 down...");
            fHome->ListBox1->Items->Insert(0, "Cylinder Auto 2 down...");
            fHome->ListBox1->Items->Insert(0, "Cylinder Auto 3 down...");
            if(AUTO_EMPTY_COLOR>=3)
            {
                fHome->ListBox1->Items->Insert(0, "Cylinder Auto 4 down...");
                fHome->ListBox1->Items->Insert(0, "Cylinder Auto 5 down...");
            }

            if(AUTO_EMPTY_COLOR>=4)
            {
                fHome->ListBox1->Items->Insert(0, "Cylinder Auto 6 down...");
            }
            flag1=false;
            flag2=false;
            flag3=false;
            flag4=false;
            flag5=false;
            flag6=false;
            break;
        case 1000:
            if(fHome->fShow==false)
            {
                SoftStop=true;
                break;
            }

            if(IniConfig.bP37bAutoCylinderUP)                                   //kevin 20180726 Auto 123 氣缸常態在上
            {                                                                   //kevin 20180726 (wei) Auto 123 Z氣缸常態在上
                flag1=true;
                flag2=true;
                flag3=true;
                flag4=true;
                flag5=true;
                flag6=true;
            }
            else
            {
                if(flag1==false)
                {
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                    {
                        if(USE_OUT_SORT_ARM!=eartUninstall)                     //rogerYang 20250722 add for 9046AU
                        {
                             Cylinder[C_TrayY_Fixer].Off();
                        }
                        else
                        {
                            Cylinder[C_LoaderPushBack_Back].Off();
                            Cylinder[C_LoaderPushBack_Push].On();
                        }
                    }
                    else
                    {
                        Cylinder[C_TrayY_Fixer].Off();
                    }
                    Cylinder[C_LoaderEdgePush].Off();
                    Cylinder[C_Auto1_Up].Off();                                 //ChungHung 20140526 add
                    if(Cylinder[C_Auto1_Selector].Pop())
                        flag1=true;
                }

                if(flag2==false)
                {
                    Cylinder[C_Auto2_Up].Off();
                    if(Cylinder[C_Auto2_Selector].Pop())
                        flag2=true;
                }

                if(flag3==false)
                {
                    if(AUTO3_IS_MAGAZINE==1)                                    //JerryYang 20220909 : add magazine
                    {
                        flag3=true;
                    }
                    else
                    {
                        Cylinder[C_Auto3_Up].Off();                             //ChungHung 20140526 add
                        if(Cylinder[C_Auto3_Selector].Pop())
                            flag3=true;
                    }
                }

                if(AUTO_EMPTY_COLOR>=3)
                {
                    if(flag4==false)
                    {
                        Cylinder[C_Auto4_Up].Off();
                        if(Cylinder[C_Auto4_Selector].Pop())
                            flag4=true;
                    }

                    if(flag5==false)
                    {
                        Cylinder[C_Auto5_Up].Off();
                        if(Cylinder[C_Auto5_Selector].Pop())
                            flag5=true;
                    }

                    if(AUTO_EMPTY_COLOR>=4)
                    {
                        if(flag6==false)
                        {
                            Cylinder[C_Auto6_Up].Off();
                            if(Cylinder[C_Auto6_Selector].Pop())
                                flag6=true;
                        }
                    }
                    else
                    {
                        flag6=true;
                    }
                }
                else
                {
                    flag4=true;
                    flag5=true;
                    flag6=true;
                }
            }

            if(flag1 && flag2 && flag3)
                fHome->iHomeStep=1100;
            break;
        case 1100:
            if(fHome->fShow==false)
            {
                fHome->iHomeStep=1300;
                break;
            }

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-PANEL2)
            // 缺相依：`TfHome::Panel2`（golden uhome.h 的 .dfm TPanel）。同 TfHome::Show()
            // 那一格。純版面：golden 在歸零走到 case 1100 時把那塊面板顯示出來。
            // 行為：歸零畫面上那塊面板不會出現。零控制流。
            #if 0 // GATE (W906-HOME-C2-PANEL2): 缺相依，見上面的就地註解
            fHome->Panel2->Visible=true;
            #endif // GATE (W906-HOME-C2-PANEL2)
            fHome->iHomeStep=1200;
            ResetOKDeleyTime.SetSecAndOn(0.1);
            fLtcSensor->ClearLtcSensor(1);                                      //Sam 20221101 : Latch 清除都要確認是否清清乾淨
            fLtcSensor->ClearLtcSensor(0);                                      //Sam 20221101 : Latch 清除都要確認是否清清乾淨
            iOutShuttle1HasICErrRetryCnt=0;
            iOutShuttle2HasICErrRetryCnt=0;

            bIn_ICRotationCompleteOnKit=false;                                  //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
            bOut_ICRotationCompleteOnKit=false;
            bIn_XYMoveFinishOnRotationKit=false;                                //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
            bOut_XYMoveFinishOnRotationKit=false;

            if(CosFunction.bUseAfterHomeShtChkLoseICNeedSlow)                   //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
            {
                bAfterHomeShtChkLoseICNeedSlow[0]=true;
                bAfterHomeShtChkLoseICNeedSlow[1]=true;
            }
            break;
        case 1200:
            if(fHome->fShow==false)
            {
                fHome->iHomeStep=1300;
                break;
            }
#ifdef SOFT_SIMULTE
            MOT[MTrayX].SetSpeed(100);
#else
            MOT[MTrayX].SetSpeed(15);
#endif
            if(ResetOKDeleyTime.Off() || fHome->fShow==false)
            {
                flag1 =false;
                flag2 =false;
                flag3 =false;                                                   //Steven 20110503
                flag4 =false;                                                   //Steven 20110503
                flag5 =false;
                flag6 =false;
                flag7 =false;                                                   //2013-04-12    Dell :旋轉站;馬達版
                flag8 =false;                                                   //2013-04-12    Dell :旋轉站;馬達版
                flag9 =false;                                                   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
                flag10=false;
                PrePushLoaderCylinder(true);                                    //Steven 20150429 : 預先打兩下Loader汽缸
                fHome->iHomeStep=1250;

                MOT[MTestY1].iGali_SingalHomeTask=1;
                MOT[MTestY2].iGali_SingalHomeTask=1;
                MOT[MTestZ1].iGali_SingalHomeTask=1;
                MOT[MTestZ2].iGali_SingalHomeTask=1;
                MOT[MTestZ1].iGali_FindZPhaseTask[1]=1;                         //Ifor 20170817 (wei) add Z Phase Task
                MOT[MTestZ2].iGali_FindZPhaseTask[2]=1;                         //Ifor 20170817 (wei) add Z Phase Task
                MOT[MTestY1].iGali_FindZPhaseTask[0]=1;                         //Isaac 20201110 : Index Y find motor phase
                MOT[MTestY2].iGali_FindZPhaseTask[3]=1;                         //Isaac 20201110 : Index Y find motor phase

                if(USE_MAGNETIC_SCALE)
                {
                    MOT[MInArmXScale].Motor->ResetPos(0);                       //Steven 20160426 : 磁性尺
                    MOT[MInArmYScale].Motor->ResetPos(0);
                    MOT[MOutArmXScale].Motor->ResetPos(0);
                    MOT[MOutArmYScale].Motor->ResetPos(0);
                    MOT[MInArmXScale].HomeFlag=1;
                    MOT[MInArmYScale].HomeFlag=1;
                    MOT[MOutArmXScale].HomeFlag=1;
                    MOT[MOutArmYScale].HomeFlag=1;
                }
            }
            break;
        case 1250:
            #ifdef DEBUG_HOME                                                   //wei (Steven) 20170317 快速回Home開關
            for(int i=0; i<TOTAL_MOTOR; i++)                                    //kevin 20170414 (wei) add show motor pos
                fHome->ShowMotorHomePos(i);

            if(IndexY1==true && IndexZ1==true && IndexY2==true && IndexZ2==true)                                        //kevin 20161214 (Steven) Increase Hone Speed
            {
                fHome->iHomeStep=1260;
            }
            #else
            if(flag1==false)
            {
                if(MOT[MTestY1].Gali_SingalHome())
                {
                    flag1=true;
                    fHome->ListBox1->Items->Insert(0, "Index Y1 home finish.");
                }
            }

            if(USE_INDEX_ARM_AXES==IndexArm_4_Axis)                             //Jimmychiu 20230117 TestY2 not homing in 3 axes
            {
                if(flag2==false)
                {
                    if(MOT[MTestY2].Gali_SingalHome())
                    {
                        flag2=true;
                        fHome->ListBox1->Items->Insert(0, "Index Y2 home finish.");
                    }
                }
            }
            else
            {
                flag2=true;
            }

            if(flag3==false)
            {
                if(MOT[MTestZ1].Gali_SingalHome())
                {
                    flag3=true;
                    fHome->ListBox1->Items->Insert(0, "Index Z1 home finish.");
                }
            }

            if(flag4==false)
            {
                if(MOT[MTestZ2].Gali_SingalHome())
                {
                    flag4=true;
                    fHome->ListBox1->Items->Insert(0, "Index Z2 home finish.");
                }
            }

            if(flag1==true && flag2==true && flag3==true && flag4==true)
            {
                flag1=false;
                flag2=false;
                flag3=false;
                flag4=false;

                if(IniConfig.bD13CheckIndexHomeSensor)                          //Steven 20140828 : 歸零後檢查Index位置
                {
                    fHome->iHomeStep=2000;
                }
                else if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true)                                       //Isaac 20201110 : Index Y find motor phase
                {
                    if(IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess==true)
                    {
                        if(bZ1ModifyDistanceRef==true || bZ2ModifyDistanceRef==true)
                        {
                            fHome->iHomeStep=1300;
                        }
                        else
                        {
                            fHome->iHomeStep=3000;
                        }
                    }
                    else if(IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess==false &&
                            bFindMotorPhaseEveryGoHomeProcess==false)
                    {
                        if(bZ1ModifyDistanceRef==true || bZ2ModifyDistanceRef==true)
                        {
                            fHome->iHomeStep=1300;
                        }
                        else
                        {
                            fHome->iHomeStep=3000;
                        }
                    }
                    else
                    {
                        fHome->iHomeStep=1300;
                    }
                }
                else
                {
                    fHome->iHomeStep=1300;
                }

                ResetOKDeleyTime.SetMSAndOn(30000);                             //Kevin  20110525
            }
            #endif
            break;
        case 1260:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 1260"))                      //Steven 20140828 : 歸零後檢查Index位置    //JerryYang 20160905 10 --> Prod.TestZ1_Safe
            {
                fHome->iHomeStep=1270;
            }
            break;
        case 1270:
            if(MOT[MTestY1].GalilTwoY_Move(-10, 10, 60000, "ProcessMotorHome 1270"))                                    //Steven 20140828 : 歸零後檢查Index位置
            {
                if(IniConfig.bD13CheckIndexHomeSensor)                          //Steven 20140828 : 歸零後檢查Index位置
                {
                    fHome->iHomeStep=2000;
                }
                else if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true)                                       //Isaac 20201110 : Index Y find motor phase
                {
                    if(IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess==true)
                    {
                        if(bZ1ModifyDistanceRef==true || bZ2ModifyDistanceRef==true)
                        {
                            fHome->iHomeStep=1300;
                        }
                        else
                        {
                            fHome->iHomeStep=3000;
                        }
                    }
                    else if(IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess==false &&
                            bFindMotorPhaseEveryGoHomeProcess==false)
                    {
                        if(bZ1ModifyDistanceRef==true ||
                           bZ2ModifyDistanceRef==true)
                        {
                            fHome->iHomeStep=1300;
                        }
                        else
                        {
                            fHome->iHomeStep=3000;
                        }
                    }
                    else
                    {
                        fHome->iHomeStep=1300;
                    }
                }
                else
                {
                    fHome->iHomeStep=1300;
                }
                IndexY1=false;
                IndexY2=false;
                IndexZ1=false;
                IndexZ2=false;

                ResetOKDeleyTime.SetMSAndOn(30000);                             //Kevin 20110525
            }
            break;
        case 1300:                                                              //Steven 20200206 : 增加歸零的保護機制
            flag1 =false;
            flag2 =false;
            flag3 =false;                                                       //Steven 20110503
            flag4 =false;                                                       //Steven 20110503
            flag5 =false;
            flag6 =false;
            flag7 =false;                                                       //2013-04-12    Dell :旋轉站;馬達版
            flag8 =false;                                                       //2013-04-12    Dell :旋轉站;馬達版
            flag9 =false;                                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
            flag10=false;
            for(int j=0; j<4; j++)                                              //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
                flag14[j]=false;
            for(int j=0; j<3; j++)                                              //Ifor 20251220 add:Boat Z
                flag18[j]=false;
            fHome->iHomeStep=1310;
        case 1310:                                                              //歸零完成，移動到等待位置
#ifdef SOFT_SIMULTE
            MOT[MInArmY].SetSpeed(100);
            MOT[MOutArmY].SetSpeed(100);
            MOT[MInShuttle1].SetSpeed(10);
            MOT[MInShuttle2].SetSpeed(10);
            MOT[MOutSortSht].SetSpeed(50);                                      //rogerYang 20250508 add for 9046AU
            MOT[MOutSortY].SetSpeed(100);
#else
            MOT[MInArmX].SetSpeed(10);
            MOT[MInArmY].SetSpeed(10);
            MOT[MOutArmX].SetSpeed(10);
            MOT[MOutArmY].SetSpeed(10);
            MOT[MInShuttle1].SetSpeed(10);
            MOT[MInShuttle2].SetSpeed(10);
            MOT[MTrayX].SetSpeed(15);
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-ROTATESPEED)
            // 缺相依：`TFrmRotate::SetInRotateSpeed(int,int)` /
            //   `SetOutRotateSpeed(int,int)`（golden RotateKit/fRotate.h）。
            //   本樹的 `TFrmRotate`（forms/fRotate.h:16）是門面，沒有這兩個。
            // 行為：**只有出貨組態（SOFT_SIMULTE 關閉）走得到這一段** ——
            //   它是「歸零收尾把各軸速度調回慢速」那一批裡的旋轉站兩行。
            //   閘住的後果：有旋轉站的機台，歸零之後 In/Out 旋轉馬達的速度
            //   **沿用上一段動作留下的值**，而不是被調回 70/100。
            //   ⚠ 同一段裡其他 18 行 `MOT[...].SetSpeed(...)` 都是活的，
            //     所以只有旋轉站這一項沒調到。
            // UN-GATE：等 TFrmRotate 門面帶進那兩個 setter。
#if 0 // GATE (W906-HOME-C2-ROTATESPEED): 缺相依，見上面的就地註解
            FrmRotate->SetInRotateSpeed(70, 100);                               //Steven 20170425 (wei) : Add rotate motor
            FrmRotate->SetOutRotateSpeed(70, 100);
#endif // GATE (W906-HOME-C2-ROTATESPEED)
            MOT[MAOIKit].SetSpeed(20);                                          //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
            MOT[MMagazine].SetSpeed(100);                                       //JerryYang 20220909 : add magazine
            MOT[MCatchMgzTray].SetSpeed(100);                                   //JerryYang 20220909 : add magazine
            MOT[MOutSortSht].SetSpeed(10);                                      //rogerYang 20250508 add for 9046AU
            MOT[MOutSortX].SetSpeed(10);
            MOT[MOutSortY].SetSpeed(10);
            MOT[MInSh1LtcSenZ1].SetSpeed(10);                                   //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
            MOT[MInSh1LtcSenZ2].SetSpeed(10);
            MOT[MInSh2LtcSenZ1].SetSpeed(10);
            MOT[MInSh2LtcSenZ2].SetSpeed(10);

            if(USE_LdUldCassetteMode==1)
            {
                MOT[MTrayZ].SetSpeed(100);                                      //Ifor 20251220 add:Boat Z
                MOT[MAuto1Z].SetSpeed(100);
                MOT[MAuto2Z].SetSpeed(100);

                MOT[MLoaderY].SetSpeed(100);
                MOT[MAuto1Y].SetSpeed(100);
                MOT[MAuto2Y].SetSpeed(100);
                MOT[MLoaderY_CCW].SetSpeed(100);
                MOT[MAuto1Y_CCW].SetSpeed(100);
                MOT[MAuto2Y_CCW].SetSpeed(100);
            }

#endif
            if(INSTALL_OCR_YMot==eocrYMotInstal)                                //Frank 20250214 add
                MOT[MLoaderY].SetSpeed(100);

            if(MachineTypeChoice==Type_HT9046_LS)
                iPos+=5000;

            if(flag1==false)
                flag1=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front_EndWaitPos, Prod.TestY2_Rear, 60000, "ProcessMotorHome 1310");                              //981118 jou Y1 +2000 easy change kit

            if(flag2==false)
            {
                if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
                    flag2=TrayArmMotorMove(Prod.iXTrayColor);
                else
                    flag2=TrayArmMotorMove(Prod.iXTrayEmpty);
            }

            if(flag3==false)
                flag3=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft+1000);     //Steven 20230109 : 修正Shuttle在正負一時可能會死雞
            if(flag4==false)
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-VTESTPOS)
                // 缺相依：`TfLotInfo::rgHomeStopPos`（golden uLotInfo.h 的 TRadioGroup ——
                // V-TEST 模式下「歸零後停在哪個位置」的選擇）。本樹的 TfLotInfo 門面沒有它。
                // ⚠ **刻意不補這個成員**：補了就得給 `ItemIndex` 一個預設值，而那個值會
                //   **選走一條分支**（0 / 1 / -1 各有不同行為）。golden 的值來自 .dfm，
                //   本樹沒有任何地方載入 .dfm 的屬性 —— 猜一個值等於在關鍵路徑上編造行為。
                //   （memory: ht9045-v906-ung.. 「解 gate 前查值從哪來」。）
                // 行為：**只有 `IniConfig.bVTESTFunction==true` 的機台受影響**。
                //   那些機台上，歸零後的停留位置一律走 golden 的 else（一般位置），
                //   不再依 V-TEST 的選擇停在 Shuttle2 左/右。
                //   沒開 V-TEST 的機台（含這台筆電）行為完全一樣 —— `&&` 的第一項就是 false。
                // UN-GATE：等 fLotInfo 的 rgHomeStopPos 有**真的值來源**（web HMI 或 .dfm 載入），
                //   本檔六個落點一起解。
                #if 0 // GATE (W906-HOME-C2-VTESTPOS): 缺相依，見上面的就地註解
                if(IniConfig.bVTESTFunction==true && fLotInfo->rgHomeStopPos->ItemIndex!=-1)
                {
                   if(fLotInfo->rgHomeStopPos->ItemIndex==1)
                      flag4=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iRight);
                   else
                      flag4=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft);
                }
                else
                #endif // GATE (W906-HOME-C2-VTESTPOS)
                if(false)   // 見上面 GATE (W906-HOME-C2-VTESTPOS)：一律走 else
                {
                }
                else
                {
                    flag4=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft+1000);
                }
            }
//#ifdef Carry4
//            if(flag5==false)
//                flag5=MOT[MOutShuttle1].MotorMove(Prod.OutSHT[0].iRight);
//            if(flag6==false)
//                flag6=MOT[MOutShuttle2].MotorMove(Prod.OutSHT[1].iRight);
//#else
//            flag5=true;                                                       //Frank 20250214 add
//            flag6=true;
//#endif
            if(USE_OUT_SORT_ARM!=eartUninstall)                                 //rogerYang 20250508 add for 9046AU
            {
                if(flag5==false)
                    flag5=MOT[MOutSortSht].MotorMove(Prod.SortSHT.iRight-1000);
            }
            else if(INSTALL_OCR_YMot==eocrYMotInstal)
            {
                if(MOT[MMTrayY].fHasTray)
                {
                    flag5=MOT[MLoaderY].MotorMove(Prod.iMLoaderYSurePos);
                }
                else
                {
                    #ifndef SOFT_SIMULTE
                    if(Sen[SnLoaderSureTray].IsOff()==false &&                  //RogerYang 20251029 : 修正Loader Y motor判斷有無tray可能被機構誤觸發
                       MOT[MLoaderY].Led[iHomeLed]==true &&
                       bIsYCarMoving==false)
                    {
                        ShowMyMessage("Loader carrier has tray , please remove that");
                    }
                    else
                    #endif
                    {
                        flag5=MOT[MLoaderY].MotorMove(Prod.iMLoaderYCarPos);
                        bIsYCarMoving=true;
                    }
                }
            }
            else
            {
                flag5=true;
            }
            flag6=true;

            if(USE_ROTATE_KIT==1 && (iRotate_Type==e1MotRotate ||
                                     iRotate_Type==e1MotRotate1Dut ||           //kevin 20130722  馬達版 ROTATE需歸零 //kevin 20130415
                                     iRotate_Type==eInOutArm1Motor))            //add One sucker with rotate
            {
                buffer =SetMotorResolution(Ang45, Ang90, true);                 //kevin 20131003
                flag7  =MOT[MInRotateKit].MotorMove(buffer);
                buffer =SetMotorResolution(Ang45, Ang90, false);                //kevin 20131003
                flag8  =MOT[MOutRotateKit].MotorMove(buffer);
            }
            else if(USE_ROTATE_KIT==1 && (iRotate_Type==e1MotRotate1Dut ||      //JerryYang 20230204 : fix rotate 1 motor 1 dut
                                          iRotate_Type==e4MotRotate ||
                                          iRotate_Type==e8MotRotate ||
                                          iRotate_Type==e2MotRotate2Dut))       //Steven 20170329 : Add individual rotate motor
            {
                flag7=MoveInRotateToDegreeAtSameTime(0);
                flag8=MoveOutRotateToDegreeAtSameTime(0);
            }
            else
            {
                flag7=true;                                                     //kevin 20130415
                flag8=true;
            }

            if(USE_AOI_Inspection)                                              //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
            {
                flag9=MOT[MAOIKit].MotorMove(Prod.iTopViewKit_Zup);             //20140918 wei  flag8修正為 flag9
            }
            else
            {
                flag9=true;                                                     //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
            }

            flag10=PrePushLoaderCylinder();                                     //Steven 20150429 : 預先打兩下Loader汽缸

            if(In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
            {
                flag14[0]=MOT[MInSh1LtcSenZ1].MotorMove(Prod.iInSH1SenICDetectZ1+Prod.iInSH1SenICAddPos);
                flag14[1]=MOT[MInSh1LtcSenZ2].MotorMove(Prod.iInSH1SenICDetectZ2+Prod.iInSH1SenICAddPos);
                flag14[2]=MOT[MInSh2LtcSenZ1].MotorMove(Prod.iInSH2SenICDetectZ1+Prod.iInSH2SenICAddPos);
                flag14[3]=MOT[MInSh2LtcSenZ2].MotorMove(Prod.iInSH2SenICDetectZ2+Prod.iInSH2SenICAddPos);
            }
            else
            {
                for(int j=0; j<4; j++)
                    flag14[j]=true;
            }

            if(USE_LdUldCassetteMode==1)                                        //Ifor 20251220 add:Boat Z
            {
                flag18[0]=MOT[MTrayZ].MotorMove(9500);
                flag18[1]=MOT[MAuto1Z].MotorMove(9500);
                flag18[2]=MOT[MAuto2Z].MotorMove(9500);
            }
            else
            {
                for(int j=0; j<3; j++)
                    flag18[j]=true;
            }

            if(ResetOKDeleyTime.Off())                                          //Kevin  20110525 start
            {
                if(flag1==false)
                    sBuffer+=" MTestY1,";
                if(flag2==false)
                    sBuffer+=" MTrayX,";
                if(flag3==false)
                    sBuffer+=" MInShuttle1,";
                if(flag4==false)
                    sBuffer+=" MInShuttle2,";
                sBuffer+=" Home Time Out";
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-HOMELOG)
                // 缺相依：`HomeLog(AnsiString)` 的**定義**在本樹被閘住 ——
                //   cpublic.cpp:746 的本體整個落在 `#if 0 // TODO(GA1-B3)` 裡，
                //   阻塞原因是它第一行就寫 `fMain->MemoHome->Lines->Add(...)`，
                //   而 TfMain 門面沒有 MemoHome（與本檔的 MEMOHOME 閘同一個缺口）。
                //   宣告還在（cpublic.h:40），所以編得過、連不起來 ——
                //   memory: symbol-exists-has-three-strengths 的第一級 vs 第二級。
                // 行為：歸零的逐步流程紀錄不會寫進 `D:\HT9045_Log\HomeLog\Home_Home.logs`。
                //   ⚠ 那個檔是事後追「歸零卡在第幾步」的主要證據來源。
                //   本樹的替代品是 `fHome->ListBox1`（同樣逐步記，而且是活的），
                //   只是它在記憶體裡、不落盤。
                // UN-GATE：TfMain 門面帶進 MemoHome（會同時解掉 cpublic.cpp 那 7 支 log 中的
                //   HomeLog 與 ProductionLog 兩支）。
                #if 0 // GATE (W906-HOME-C2-HOMELOG): 缺相依，見上面的就地註解
                HomeLog(sBuffer);
                #endif // GATE (W906-HOME-C2-HOMELOG)
                fHome->iHomeStep=1;
                break;
            }

            if(flag1 && flag2 && flag3 && flag4 &&                              //kevin 20130415
               flag5 && flag6 && flag7 && flag8 &&
               flag9 &&                                                         //20140918  wei  增加  flag9
               flag10 &&                                                        //Steven 20150429 : 預先打兩下Loader汽缸
               flag14[0] && flag14[1] && flag14[2] && flag14[3] &&              //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
               flag18[0] && flag18[1] && flag18[2])                             //Ifor 20251220 add: Boat Z
            {
                bIsYCarMoving=false;                                            //RogerYang 20251029 : 修正Loader Y motor判斷有無tray可能被機構誤觸發
                #ifndef SOFT_SIMULTE
                if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
                    iTrayArmPos=Prod.iXTrayColor;
                else
                    iTrayArmPos=Prod.iXTrayEmpty;

                MOT[MTrayX].ScanMotorStatus();
                if(MOT[MTrayX].Motor->Enable)
                {
                    if(bCyflag[0]==false)
                        bCyflag[0]=(Cylinder[C_TrayXFloodgate1].Enable==false || Cylinder[C_TrayXFloodgate1].Pop());
                    if(bCyflag[1]==false)
                        bCyflag[1]=(Cylinder[C_TrayXFloodgate2].Enable==false || Cylinder[C_TrayXFloodgate2].Pop());
                    if(bCyflag[2]==false)
                        bCyflag[2]=(Cylinder[C_TrayXFloodgate3].Enable==false || Cylinder[C_TrayXFloodgate3].Pop());
                    if(bCyflag[3]==false)
                        bCyflag[3]=(Cylinder[C_TrayXFloodgate4].Enable==false || Cylinder[C_TrayXFloodgate4].Pop());

                    if(!(bCyflag[0] && bCyflag[1] && bCyflag[2] && bCyflag[3]))
                        break;

                    if(((iTrayArmPos-100>=MOT[MTrayX].ReadEncoderPos()) &&
                        (MOT[MTrayX].ReadEncoderPos()>=iTrayArmPos+100)) ||
                         MOT[MTrayX].Led[iHomeLed])
                    {
                        ShowMyMessage("Tray arm is not at safe position", "Tray Arm不在安全位置上");
                        fHome->fAbort=false;
                        SystemStart=false;
                        fHome->iHomeStep=1;
                        SoftStop=true;
                        break;
                    }
                }
                #endif
                flag1=false;
                flag2=false;
                flag3=false;
                flag4=false;                                                    //RogerYang 20250510 Add for 9046AU
                fHome->iHomeStep=1520;
            }
            break;
        case 1520:
            #ifdef DEBUG_HOME                                                   //wei (Steven) 20170317 快速回Home開關
                flag1=true;                                                     //Steven 20260612 : Fix == to = (was comparison, not assignment)
            #else
            if(flag1==false)                                                    //Steven 20230109 : 修正Shuttle在正負一時可能會死雞
                flag1=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 1520");               //JerryYang 20160905 10 --> Prod.TestZ1_Safe
            #endif
            if(flag2==false)
                flag2=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft);
            if(flag3==false)
                flag3=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft);

            if(USE_OUT_SORT_ARM!=eartUninstall)                                 //rogerYang 20250508 add for 9046AU
            {
                if(flag4==false)
                    flag4=MOT[MOutSortSht].MotorMove(Prod.SortSHT.iRight);
            }
            else
                flag4=true;

            if(flag1 && flag2 && flag3 && flag4)                                //rogerYang 20250508 add for 9046AU
            {
                flag1=false;
                flag2=false;
                flag3=false;
                flag4=false;                                                    //wei 20161206 Auto Shuttle Sensor 回Home
                flag5=false;
                flag6=false;                                                    //RogerYang 20250510 Add for 9046AU
                bAutoShuttleHome=true;                                          //wei 20161206 Auto Shuttle Sensor 回Home
                fHome->iHomeStep=1530;
            }
            break;
        case 1530:                                                              //Steven 20200206 : 增加歸零的保護機制
            if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)
            {
                if(MOT[MTrayX].ReadEncoderPos()<=Prod.iXTrayColor-100)
                {
                    fHome->iHomeStep=1300;
                    break;
                }
            }
            else
            {
                if(MOT[MTrayX].ReadEncoderPos()<=Prod.iXTrayEmpty-100)
                {
                    fHome->iHomeStep=1300;
                    break;
                }
            }
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-MAGAZINE)
            // 缺相依：`InitialMagazineUpDown()` / `DoMagazineUpDown(int,int)`
            // （golden Magazine.h）。本樹全樹 0 命中 —— Magazine（料匣自動補 Tray）沒翻。
            // 行為：**只有 `AUTO3_IS_MAGAZINE==1` 的機台受影響**。
            //   那些機台上，歸零不會重設 Magazine 的升降任務、也不會把 Magazine 降到位。
            //   ⚠ 下面那一格把 `flag5` 補成 true（＝「Magazine 動作完成」），
            //     否則 case 1540 會卡在等 Magazine。
            //   非 Magazine 機台（含這台筆電）走的是 golden 自己的 `else { flag5=true; }`，
            //   行為完全相同。
            // UN-GATE：等 Magazine 模組翻進來，本檔兩個落點一起解。
            #if 0 // GATE (W906-HOME-C2-MAGAZINE): 缺相依，見上面的就地註解
            InitialMagazineUpDown();                                            //JerryYang 20220909 : add magazine
            #endif // GATE (W906-HOME-C2-MAGAZINE)

            fHome->iHomeStep=1540;
        case 1540:
            if(flag1==false)
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-VTESTPOS)
                // 缺相依：`TfLotInfo::rgHomeStopPos`（golden uLotInfo.h 的 TRadioGroup ——
                // V-TEST 模式下「歸零後停在哪個位置」的選擇）。本樹的 TfLotInfo 門面沒有它。
                // ⚠ **刻意不補這個成員**：補了就得給 `ItemIndex` 一個預設值，而那個值會
                //   **選走一條分支**（0 / 1 / -1 各有不同行為）。golden 的值來自 .dfm，
                //   本樹沒有任何地方載入 .dfm 的屬性 —— 猜一個值等於在關鍵路徑上編造行為。
                //   （memory: ht9045-v906-ung.. 「解 gate 前查值從哪來」。）
                // 行為：**只有 `IniConfig.bVTESTFunction==true` 的機台受影響**。
                //   那些機台上，歸零後的停留位置一律走 golden 的 else（一般位置），
                //   不再依 V-TEST 的選擇停在 Shuttle2 左/右。
                //   沒開 V-TEST 的機台（含這台筆電）行為完全一樣 —— `&&` 的第一項就是 false。
                // UN-GATE：等 fLotInfo 的 rgHomeStopPos 有**真的值來源**（web HMI 或 .dfm 載入），
                //   本檔六個落點一起解。
                #if 0 // GATE (W906-HOME-C2-VTESTPOS): 缺相依，見上面的就地註解
                if(IniConfig.bVTESTFunction==true && fLotInfo->rgHomeStopPos->ItemIndex==0)
                    flag1=MoveInArm2XYToShuttle2Wait();                         //Steven 20110719 : In Arm不要呆在加熱盤上面拷
                else
                #endif // GATE (W906-HOME-C2-VTESTPOS)
                if(false)   // 見上面 GATE (W906-HOME-C2-VTESTPOS)：一律走 else
                    ;
                else
                    flag1=MOT[MInArmX].MotorMove(Prod.XInArm_Tray_Pick[0][2]+6000);                                     //Steven 20110719 : In Arm不要呆在加熱盤上面拷
            }

            if(flag3==false)                                                    //Steven 20110719 : In Arm不要呆在加熱盤上面拷
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-VTESTPOS)
                // 缺相依：`TfLotInfo::rgHomeStopPos`（golden uLotInfo.h 的 TRadioGroup ——
                // V-TEST 模式下「歸零後停在哪個位置」的選擇）。本樹的 TfLotInfo 門面沒有它。
                // ⚠ **刻意不補這個成員**：補了就得給 `ItemIndex` 一個預設值，而那個值會
                //   **選走一條分支**（0 / 1 / -1 各有不同行為）。golden 的值來自 .dfm，
                //   本樹沒有任何地方載入 .dfm 的屬性 —— 猜一個值等於在關鍵路徑上編造行為。
                //   （memory: ht9045-v906-ung.. 「解 gate 前查值從哪來」。）
                // 行為：**只有 `IniConfig.bVTESTFunction==true` 的機台受影響**。
                //   那些機台上，歸零後的停留位置一律走 golden 的 else（一般位置），
                //   不再依 V-TEST 的選擇停在 Shuttle2 左/右。
                //   沒開 V-TEST 的機台（含這台筆電）行為完全一樣 —— `&&` 的第一項就是 false。
                // UN-GATE：等 fLotInfo 的 rgHomeStopPos 有**真的值來源**（web HMI 或 .dfm 載入），
                //   本檔六個落點一起解。
                #if 0 // GATE (W906-HOME-C2-VTESTPOS): 缺相依，見上面的就地註解
                if(IniConfig.bVTESTFunction==true && fLotInfo->rgHomeStopPos->ItemIndex==0)
                    flag3=true;
                else
                #endif // GATE (W906-HOME-C2-VTESTPOS)
                if(false)   // 見上面 GATE (W906-HOME-C2-VTESTPOS)：一律走 else
                    ;
                else
                    flag3=MOT[MInArmY].MotorMove(Prod.YInArm_Tray_Pick[0][2]-10000);
            }

            if(flag2==false)
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-VTESTPOS)
                // 缺相依：`TfLotInfo::rgHomeStopPos`（golden uLotInfo.h 的 TRadioGroup ——
                // V-TEST 模式下「歸零後停在哪個位置」的選擇）。本樹的 TfLotInfo 門面沒有它。
                // ⚠ **刻意不補這個成員**：補了就得給 `ItemIndex` 一個預設值，而那個值會
                //   **選走一條分支**（0 / 1 / -1 各有不同行為）。golden 的值來自 .dfm，
                //   本樹沒有任何地方載入 .dfm 的屬性 —— 猜一個值等於在關鍵路徑上編造行為。
                //   （memory: ht9045-v906-ung.. 「解 gate 前查值從哪來」。）
                // 行為：**只有 `IniConfig.bVTESTFunction==true` 的機台受影響**。
                //   那些機台上，歸零後的停留位置一律走 golden 的 else（一般位置），
                //   不再依 V-TEST 的選擇停在 Shuttle2 左/右。
                //   沒開 V-TEST 的機台（含這台筆電）行為完全一樣 —— `&&` 的第一項就是 false。
                // UN-GATE：等 fLotInfo 的 rgHomeStopPos 有**真的值來源**（web HMI 或 .dfm 載入），
                //   本檔六個落點一起解。
                #if 0 // GATE (W906-HOME-C2-VTESTPOS): 缺相依，見上面的就地註解
                if(IniConfig.bVTESTFunction==true &&
                   fLotInfo->rgHomeStopPos->ItemIndex==1)
                    flag2=MoveOutArm2XYToShuttle2Wait();
                else
                #endif // GATE (W906-HOME-C2-VTESTPOS)
                if(false)   // 見上面 GATE (W906-HOME-C2-VTESTPOS)：一律走 else
                    ;
                else
                    flag2=MoveOutArmXY_ToFix_Tray_Full();
            }

            if(AUTO_SENSOR_INSTALL)                                             //wei 20161206 Auto Shuttle Sensor 回Home
            {
                if(flag4==false)
                {
                    flag4=DoMoveShuttleSensor();
                }
            }
            else
            {
                flag4=true;
            }

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-MAGAZINE)
            // 缺相依：同上（`DoMagazineUpDown`）。
            // 行為：`AUTO3_IS_MAGAZINE==1` 的機台上，`flag5` 一律補 true
            //   （golden 在非 Magazine 機台上寫的就是 `flag5=true`）。
            #if 0 // GATE (W906-HOME-C2-MAGAZINE): 缺相依，見上面的就地註解
            if(AUTO3_IS_MAGAZINE==1)                                            //JerryYang 20220909 : add magazine
            {
                flag5=DoMagazineUpDown(0, 3);
            }
            else
            {
                flag5=true;
            }
            #endif // GATE (W906-HOME-C2-MAGAZINE)
            flag5=true;   // 見上面 GATE (W906-HOME-C2-MAGAZINE)

            if(USE_OUT_SORT_ARM!=eartUninstall)                                 //rogerYang 20250508 add for 9046AU
            {
                if(flag6==false)
                {
                    flag6=MoveSortArmXYToSortShtWait();
                }
            }
            else
                flag6=true;

            if(flag1 && flag2 &&                                                //JerryYang 20220909 : add magazine
               flag3 && flag4 && flag5 &&                                       //Steven 20110719 : In Arm不要呆在加熱盤上面拷
               flag6)                                                           //RogerYang 20250510 Add for 9046AU
            {
                flag1=false;
                flag2=false;
                bAutoShuttleHome=false;                                         //wei 20161206 Auto Shuttle Sensor 回Home
                fHome->iHomeStep=1545;
            }
            break;
        case 1545:
            if(AUTO3_IS_MAGAZINE==1)                                            //JerryYang 20220909 : add magazine
            {
                flag1=CatchMgzTrayMove(Prod.iCatchMazTray_Rear);
            }
            else
            {
                flag1=true;
            }

            if(flag1)
            {
                flag1=false;
                fHome->iHomeStep=1550;
            }
            break;
        case 1550:
            if(fHome->fShow)
                fHome->Close();
            fHome->iHomeStep=1600;
            break;
        case 1600:
            for(int i=0; i<TOTAL_MOTOR; i++)
            {
                MOT[i].fCanMove=true;
                MOT[i].fCanMoveL=true;
                MOT[i].fCanMoveM=true;
                MOT[i].fCanMoveR=true;
            }

            IndexStatus=0;
            fHome->iHomeStep=1;
            SoftStart=false;
            TestSocket.ClearAll()  ;
            if(FTestSuck.HasRealIC()==false)    FTestSuck.ClearAll();
            if(BTestSuck.HasRealIC()==false)    BTestSuck.ClearAll();
            if(FLCarryKit.HasRealIC()==false)   FLCarryKit.ClearAll();
            if(BLCarryKit.HasRealIC()==false)   BLCarryKit.ClearAll();
            if(FRCarryKit.HasRealIC()==false)   FRCarryKit.ClearAll();
            if(BRCarryKit.HasRealIC()==false)   BRCarryKit.ClearAll();
            if(ShuttleHasIC() || FTestSuck.UseSiteHasIC() || BTestSuck.UseSiteHasIC())
            {
                iOneCycle=1;
                fMain->DebugOneCycleHotPlate("ProcessMotorHome");               //Sam 20210915 : 增加 OneCycle Hotpalte Debug Log
                _bHomeNeedOnecycle=true;
            }
            else
            {
                iOneCycle=0;
                _bHomeNeedOnecycle=false;
            }
            IndexStatus=Z1_Z2_Normal;
            SetInitialICCheck();

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20150507
            {
                if(MOT[MTrayX].fHasTray==false &&
                   USE_AUTO_RETEST==eartInstall &&
                   Cylinder[C_CatchTray_FixOn].OnStatus())                      //kevin 20150616
                {
                     ShowErrorMessage("WAR0614", K_RETRY, MTrayX);              //手動取下tray    //wei 20150805 WAR0612==>WAR0614
                     return false;
                }
            }

            if(LastSet.iRealDummy!=DUMMY &&
               LastSet.iRunStartMode!=rsmAutoRetest)                            //ChungHung 20140625 AutoRetest 不能鎖
            {
                if(MOT[MMTrayY].fHasTray)
                {
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                    {
                        if(USE_OUT_SORT_ARM!=eartUninstall)                     //rogerYang 20250722 add for 9046AU
                        {
                            Cylinder[C_TrayY_Fixer].On();
                            Cylinder[C_LoaderEdgePush].On();
                        }
                        else
                        {
                            Cylinder[C_LoaderPushBack_Back].On();
                            Cylinder[C_LoaderPushBack_Push].Off();
                        }
                    }
                    else
                    {
                        Cylinder[C_TrayY_Fixer].On();
                        Cylinder[C_LoaderEdgePush].On();
                    }
                }

                if(MOT[MMAuto1].fHasTray)
                {
                    Cylinder[C_Auto1Side_Fixer].On();
                    Cylinder[C_Auto1EdgePush].On();
                    Cylinder[C_Auto1UpPress].On();                              //JerryYang 20190423 新增unloader壓tray
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                        Cylinder[C_Auto1Separate].Pop();
                    }
                }

                if(MOT[MMAuto2].fHasTray)
                {
                    Cylinder[C_Auto2Side_Fixer].On();
                    Cylinder[C_Auto2EdgePush].On();
                    Cylinder[C_Auto2UpPress].On();
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                        Cylinder[C_Auto2Separate].Pop();
                    }
                }

                if(MOT[MMAuto3].fHasTray)
                {
                    Cylinder[C_Auto3Side_Fixer].On();
                    Cylinder[C_Auto3EdgePush].On();
                    Cylinder[C_Auto3UpPress].On();
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                        Cylinder[C_Auto3Separate].Pop();
                    }
                }

                if(MOT[MMAuto4].fHasTray)                                       //Steven 20230907 : For HT-9011UC
                {
                    Cylinder[C_Auto4Side_Fixer].On();
                    Cylinder[C_Auto4EdgePush].On();
                    Cylinder[C_Auto4UpPress].On();
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                        Cylinder[C_Auto4Separate].Pop();
                    }
                }

                if(MOT[MMAuto5].fHasTray)
                {
                    Cylinder[C_Auto5Side_Fixer].On();
                    Cylinder[C_Auto5EdgePush].On();
                    Cylinder[C_Auto5UpPress].On();
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                        Cylinder[C_Auto5Separate].Pop();
                    }
                }

                if(MOT[MMAuto6].fHasTray)
                {
                    Cylinder[C_Auto6Side_Fixer].On();
                    Cylinder[C_Auto6EdgePush].On();
                    Cylinder[C_Auto6UpPress].On();
                    if(USE_OUT_SORT_ARM!=eartUninstall)                         //rogerYang 20250722 add for 9046AU
                    {
                        Cylinder[C_Auto6Separate].Pop();
                    }
                }
            }
            PitchCylinderState[0]=0;
            PitchCylinderState[1]=0;
            PitchCylinderState[2]=0;
            StopAllMotor();                                                     //Steven 20220309 : 拿掉false

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-ZTEACHAUTOPOS)
            // 缺相依：`TfInOutArmZteach_Facade::InitAutoPosTask()`
            // （golden 的 In/Out Arm Z 自動教導表單方法）。本樹的門面沒有它。
            // 行為：歸零完成時**不重設自動教導的 task 游標**。
            //   ⇒ 有在用 Z 軸自動教導的機台上，下一次進教導畫面時 task 可能停在上一次的值。
            //   純教導流程，與生產路徑無關。
            // UN-GATE：等 Zteach 門面帶進 InitAutoPosTask。
            #if 0 // GATE (W906-HOME-C2-ZTEACHAUTOPOS): 缺相依，見上面的就地註解
            Zteach->InitAutoPosTask();                                          //Frank 20171213 (Steven) : Hone 需Inital Task
            #endif // GATE (W906-HOME-C2-ZTEACHAUTOPOS)

            fHome->RotateCheckClear();
            InitialIndexAutoCleanTask();                                        //kevin 20120601 Autoclean
            MyDBIProcessNew("Motion", "WAR2208", "Do process motor home finish.");                                      //jou 2010-11-23
            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-HOMELOG)
            // 缺相依：`HomeLog(AnsiString)` 的**定義**在本樹被閘住 ——
            //   cpublic.cpp:746 的本體整個落在 `#if 0 // TODO(GA1-B3)` 裡，
            //   阻塞原因是它第一行就寫 `fMain->MemoHome->Lines->Add(...)`，
            //   而 TfMain 門面沒有 MemoHome（與本檔的 MEMOHOME 閘同一個缺口）。
            //   宣告還在（cpublic.h:40），所以編得過、連不起來 ——
            //   memory: symbol-exists-has-three-strengths 的第一級 vs 第二級。
            // 行為：歸零的逐步流程紀錄不會寫進 `D:\HT9045_Log\HomeLog\Home_Home.logs`。
            //   ⚠ 那個檔是事後追「歸零卡在第幾步」的主要證據來源。
            //   本樹的替代品是 `fHome->ListBox1`（同樣逐步記，而且是活的），
            //   只是它在記憶體裡、不落盤。
            // UN-GATE：TfMain 門面帶進 MemoHome（會同時解掉 cpublic.cpp 那 7 支 log 中的
            //   HomeLog 與 ProductionLog 兩支）。
            #if 0 // GATE (W906-HOME-C2-HOMELOG): 缺相依，見上面的就地註解
            HomeLog("Close");                                                   //Kevin  20110525
            #endif // GATE (W906-HOME-C2-HOMELOG)
            if(iAseHome!=0)                                                     //kevin 20150925
            {
                iAseHome=0;
                RespondASECom("@e02104Done");                                   //kevin 20150415 回應 ase Home finish
            }

            bHomeUnlock=false;                                                  //kevin 20131218
            SetAutoSkipCount(0);                                                //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
            fMain->SendMSG_CMD(MSG_CMD_HandlerHomeFinish);                      //Steven 20150304 : Add GPIB LOG

            if(USE_Scanner_AOI_Inspection)                                      //Richard 20220817:Add Scanner_AOI add check Big Size IC Function
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-AOISENDCMD)
                // 缺相依：`TFrmAOI::SendCommand(AnsiString,int)`（golden fAOI.h）。
                // 本樹的 TFrmAOI 門面（forms/fAOI.h:106）沒有這個方法。
                // 行為：`USE_Scanner_AOI_Inspection` 的機台上，歸零後**不向 AOI 要
                //   `@GetGrabPos`**（抓取位置與 pitch 數）。
                //   ⇒ AOI 端沿用上一次的抓取位置。沒有 Scanner AOI 的機台不受影響。
                // UN-GATE：等 TFrmAOI 帶進 SendCommand。
                #if 0 // GATE (W906-HOME-C2-AOISENDCMD): 缺相依，見上面的就地註解
                FrmAOI->SendCommand("@GetGrabPos+", 0);                         //Expect Return@GetGrabPos,PitchXCnt,PitchYCnt,PitchX,PitchY +
                #endif // GATE (W906-HOME-C2-AOISENDCMD)

            }

            if(TestIF_File.bEnableBarCode)                                      //Steven 20160823 : 歸零後也要清空蝦頭的2DID
            {
                fBarCode->InitialBarcodeScanInShuttle1();
                fBarCode->InitialBarcodeScanInShuttle2();
            }

            if(SHT_FLOATING_CHK==1 && TestIF_File.bEnableShtFloatChk)           //Steven 20160920 : IC置偏檢查
            {
                fBarCode->InitialShuttleFloatCheck1();
                fBarCode->InitialShuttleFloatCheck2();
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-CCDCMD)
                // 缺相依：`TfBarCode` 缺 `SendCCDCommand(int,AnsiString,AnsiString)`、
                // `iBarCode1_1/1_2/2_1/2_2`（四台 CCD 的通道編號）、`InitialSFCAutoTune1/2()`、
                // `SetSFCCheckStepCount()`、`iSFCTotalMoveStep`（golden BarCode.h）。
                // 本樹的 fBarCode 是門面，只有 `InitialBarcodeScanInShuttle1/2()` 與
                // `InitialShuttleFloatCheck1/2()` 那一類任務重設，**沒有對 CCD 送字串的通道**。
                // 行為：歸零後**不向四台 2DID/置偏檢查 CCD 送指令**：
                //   * 不清 CCD 的接收緩衝（`E9,1,<步數>`）
                //   * 不做 SFC（IC 置偏檢查）的 auto-tune 重設與步數設定
                //   * 2D / Pin1 模式切換（`@FN2D+` / `@FNPN+`）不會送出
                //   ⇒ 有 2DID CCD 的機台上，CCD 沿用上一次的模式與緩衝內容；
                //     若上一批是 Pin1 模式、這一批要 2D，**CCD 不會自己切回去**。
                //   ⚠ 前面那兩行 `fBarCode->InitialBarcodeScanInShuttle1/2()` 與
                //     `InitialShuttleFloatCheck1/2()` **是活的，照留** —— handler 這一側的
                //     掃描任務仍然會被重設。少掉的只有往 CCD 送的那幾筆。
                // UN-GATE：等 BarCode 模組的 SendCCDCommand 與四個通道編號翻進來，
                //   本檔三個落點（SFC 區塊 + Pin1 切換的兩個臂）一起解。
                #if 0 // GATE (W906-HOME-C2-CCDCMD): 缺相依，見上面的就地註解
                fBarCode->InitialSFCAutoTune1(false);
                fBarCode->InitialSFCAutoTune2(false);
                fBarCode->SetSFCCheckStepCount();
                Str.sprintf("E9,1,%d", fBarCode->iSFCTotalMoveStep);
                fBarCode->SendCCDCommand(fBarCode->iBarCode1_1, "Clear buffer", Str);
                fBarCode->SendCCDCommand(fBarCode->iBarCode1_2, "Clear buffer", Str);
                fBarCode->SendCCDCommand(fBarCode->iBarCode2_1, "Clear buffer", Str);
                fBarCode->SendCCDCommand(fBarCode->iBarCode2_2, "Clear buffer", Str);
                #endif // GATE (W906-HOME-C2-CCDCMD)

            }

            //==> Eastsun 20260526 #026-4.PinN.P-N6 Pin1 mode switch :KYEC
            if(CosFunction.b2DUsePinInspection) //Ifor 20230531 add:2D/Pin1 模式切換
            {
                //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-CCDCMD)
                // 缺相依：同上（`SendCCDCommand` / `iBarCode1_1`）。
                // 行為：2D ↔ Pin1 的模式切換指令不會送給 CCD。
                //   ⚠ `Str` 的賦值一起閘掉 —— 它在這個 if 之外沒有讀者，
                //     留著只會是一個寫了沒人用的值。
                #if 0 // GATE (W906-HOME-C2-CCDCMD): 缺相依，見上面的就地註解
                if(TestIF_File.b2DUsePinInspection==true)
                {
                    Str="@FNPN+";
                    fBarCode->SendCCDCommand(fBarCode->iBarCode1_1, "Switch To Pin1.", Str);
                }
                else
                {
                    Str="@FN2D+";
                    fBarCode->SendCCDCommand(fBarCode->iBarCode1_1, "Switch To 2D.", Str);
                }
                #endif // GATE (W906-HOME-C2-CCDCMD)

            }
            //<== Eastsun 20260526 #026-4.PinN.P-N6

            if(INSTALL_OCR!=eocrUninstal || BAR_CODE_INSTALL!=ebctUninstall)    //kevin 20211101 home check 2D use
            {
                if(TestIF_File.bOcrFunction || TestIF_File.bEnableBarCode)      //Steven 20160912 : 避免沒開啟2D function
                    fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);
                else
                    fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);

                if(TestIF_File.b2DUsePinInspection)                             //Ifor 20240528 add:Pin1 Function
                    fMain->SendMSG_CMD(MSG_CMD_EnablePin1Function);
                else
                    fMain->SendMSG_CMD(MSG_CMD_DisablePin1Function);
            }

            //AI(W906-HOME-C2) 20260920: GATE (W906-HOME-C2-VTESTPOS)
            // 缺相依：`TfLotInfo::rgHomeStopPos`（golden uLotInfo.h 的 TRadioGroup ——
            // V-TEST 模式下「歸零後停在哪個位置」的選擇）。本樹的 TfLotInfo 門面沒有它。
            // ⚠ **刻意不補這個成員**：補了就得給 `ItemIndex` 一個預設值，而那個值會
            //   **選走一條分支**（0 / 1 / -1 各有不同行為）。golden 的值來自 .dfm，
            //   本樹沒有任何地方載入 .dfm 的屬性 —— 猜一個值等於在關鍵路徑上編造行為。
            //   （memory: ht9045-v906-ung.. 「解 gate 前查值從哪來」。）
            // 行為：**只有 `IniConfig.bVTESTFunction==true` 的機台受影響**。
            //   那些機台上，歸零後的停留位置一律走 golden 的 else（一般位置），
            //   不再依 V-TEST 的選擇停在 Shuttle2 左/右。
            //   沒開 V-TEST 的機台（含這台筆電）行為完全一樣 —— `&&` 的第一項就是 false。
            // UN-GATE：等 fLotInfo 的 rgHomeStopPos 有**真的值來源**（web HMI 或 .dfm 載入），
            //   本檔六個落點一起解。
            // （這一處是歸零完成時把選擇清成 -1，也就是「下一次不指定」。
            //   閘住它沒有殘留副作用 —— 那個成員在本樹根本不存在。）
            #if 0 // GATE (W906-HOME-C2-VTESTPOS): 缺相依，見上面的就地註解
            if(IniConfig.bVTESTFunction==true)
                fLotInfo->rgHomeStopPos->ItemIndex=-1;
            #endif // GATE (W906-HOME-C2-VTESTPOS)

            if(CosFunction.bFTPFunction)                                        //Ifor 20231101 add:FTP Function
            {
                if(IniConfig.bEnableFTP==true)
                {
                    fMain->SendMSG_CMD(MSG_CMD_EnableFTPFunction);
                }
                else
                {
                    fMain->SendMSG_CMD(MSG_CMD_DisableFTPFunction);
                }
            }
            return true;
        case 2000:                                                              //Steven 20140828 Start: 歸零後檢查Index位置
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 2000"))                      //JerryYang 20160905 10 --> Prod.TestZ1_Safe
                fHome->iHomeStep=2010;
            break;
        case 2010:
            if(MOT[MTestY1].GalilTwoY_Move(-10, 10, 60000, "ProcessMotorHome 2010"))
            {
                fHome->iHomeStep=2020;
                ResetOKDeleyTime.SetMSAndOn(200);                               //JerryYang 20160418 100->200,延長delay 時間,避免已經移動到home點卻沒偵測到
            }
            break;
        case 2020:
            if(ResetOKDeleyTime.Off())
            {
                MOT[MTestZ1].ScanMotorStatus();
                MOT[MTestZ2].ScanMotorStatus();
                MOT[MTestY1].ScanMotorStatus();
                MOT[MTestY2].ScanMotorStatus();
                fHome->iHomeStep=2050;
            }
            break;
        case 2050:
            flag1=true;

            fHome->ListBox1->Items->Insert(0, "Start check Index home sensor...");
        #ifndef SOFT_SIMULTE                                                    //JerryYang 20170202 (Steven) 避免軟體模擬時啟用D13造成無法回home
            if(MOT[MTestZ1].Led[iHomeLed]==false)
            {
                MNetLog("Index Z1 not at home! (2000)");
                flag1=false;
            }

            if(MOT[MTestZ2].Led[iHomeLed]==false)
            {
                MNetLog("Index Z2 not at home! (2000)");
                flag1=false;
            }

            if(MOT[MTestY1].Led[iHomeLed]==false)
            {
                MNetLog("Index Y1 not at home! (2000)");
                flag1=false;
            }

            if(MOT[MTestY2].Led[iHomeLed]==false)
            {
                MNetLog("Index Y2 not at home! (2000)");
                flag1=false;
            }
        #endif
            if(flag1==false)
            {
                RecordProcess("Index find home fail, try again! (2000)");
                fHome->iHomeStep=1;
            }
            else
            {
                fHome->iHomeStep=2060;                                          //kevin 20210908 index arm 沒有在 Shuttle Up
            }
            break;
        case 2060:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Rear, 60000, "ProcessMotorHome 1310"))        //981118 jou Y1 +2000 easy change kit
                fHome->iHomeStep=2100;                                          //kevin 20210908 index arm 沒有在 Shuttle Up
            break;
        case 2100:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(-10, 30000, "ProcessMotorHome 2100"))                                   //Steven 20140828
            {
                flag1=true;
                ResetOKDeleyTime.SetMSAndOn(100);
                fHome->iHomeStep=2150;
            }
            break;
        case 2150:
            if(ResetOKDeleyTime.Off())
            {
                fHome->ListBox1->Items->Insert(0, "Check Index Z must off");
                MOT[MTestZ1].ScanMotorStatus();
                MOT[MTestZ2].ScanMotorStatus();
                MOT[MTestY1].ScanMotorStatus();
                MOT[MTestY2].ScanMotorStatus();
            #ifndef SOFT_SIMULTE                                                //JerryYang 20170202 (Steven) 避免軟體模擬時啟用D13造成無法回home
                if(MOT[MTestZ1].Led[iHomeLed]==true)
                {
                    MNetLog("Index Z1 not away home! (2100)");
                    flag1=false;
                }

                if(MOT[MTestZ2].Led[iHomeLed]==true)
                {
                    MNetLog("Index Z2 not away home! (2100)");
                    flag1=false;
                }
            #endif
                if(flag1==false)
                {
                    RecordProcess("Index find home fail, try again! (2100)");
                    fHome->iHomeStep=1;
                }
                else
                {
                    ResetOKDeleyTime.SetMSAndOn(100);
                    fHome->iHomeStep=2200;
                }
            }
            break;
        case 2200:
            if(ResetOKDeleyTime.Off() && MOT[MTestZ1].Gali_Two_ZAxis_Move(-500, 30000, "ProcessMotorHome 2200"))
            {
                fHome->ListBox1->Items->Insert(0, "Check Index Z must on");
                ResetOKDeleyTime.SetMSAndOn(100);
                fHome->iHomeStep=2300;
            }
            break;
        case 2300:
            if(ResetOKDeleyTime.Off() && MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 2300"))
            {
                ResetOKDeleyTime.SetMSAndOn(200);
                fHome->iHomeStep=2400;
            }
            break;
        case 2400:
            if(ResetOKDeleyTime.Off())
            {
                flag1=true;
                MOT[MTestZ1].ScanMotorStatus();
                MOT[MTestZ2].ScanMotorStatus();
                MOT[MTestY1].ScanMotorStatus();
                MOT[MTestY2].ScanMotorStatus();
            #ifndef SOFT_SIMULTE                                                //JerryYang 20170202 (Steven) 避免軟體模擬時啟用D13造成無法回home
                if(MOT[MTestZ1].Led[iHomeLed]==false)
                {
                    MNetLog("Index Z1 not at home! (2400)");
                    flag1=false;
                }

                if(MOT[MTestZ2].Led[iHomeLed]==false)
                {
                    MNetLog("Index Z2 not at home! (2400)");
                    flag1=false;
                }
            #endif
                if(flag1==false)
                {
                    RecordProcess("Index find home fail, try again! (2400)");
                    fHome->iHomeStep=1;
                }
                else
                {
                    fHome->ListBox1->Items->Insert(0, "Check Index Z Ok");
                    fHome->ListBox1->Items->Insert(0, "Check Index Y must off");
                    fHome->iHomeStep=2500;
                }
            }
            break;
        case 2500:
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 2500"))                      //JerryYang 20160905 10 --> Prod.TestZ1_Safe
            {
                fHome->iHomeStep=2600;
            }
            break;
        case 2600:
            if(MOT[MTestY1].GalilTwoY_Move(10, -10, 60000, "ProcessMotorHome 2600"))
            {
                flag1=true;
                ResetOKDeleyTime.SetMSAndOn(200);
                fHome->iHomeStep=2650;
            }
            break;
        case 2650:
            if(ResetOKDeleyTime.Off())
            {
                MOT[MTestZ1].ScanMotorStatus();
                MOT[MTestZ2].ScanMotorStatus();
                MOT[MTestY1].ScanMotorStatus();
                MOT[MTestY2].ScanMotorStatus();
            #ifndef SOFT_SIMULTE                                                //JerryYang 20170202 (Steven) 避免軟體模擬時啟用D13造成無法回home
                if(MOT[MTestY1].Led[iHomeLed]==true)
                {
                    MNetLog("Index Y1 not away home! (2600)");
                    flag1=false;
                }

                if(MOT[MTestY2].Led[iHomeLed]==true)
                {
                    MNetLog("Index Y2 not away home! (2600)");
                    flag1=false;
                }
            #endif
                if(flag1==false)
                {
                    RecordProcess("Index find home fail, try again! (2600)");
                    fHome->iHomeStep=1;
                }
                else
                {
                    ResetOKDeleyTime.SetMSAndOn(100);
                    fHome->iHomeStep=2700;
                }
            }
            break;
        case 2700:
            if(ResetOKDeleyTime.Off() && MOT[MTestY1].GalilTwoY_Move(500, -500, 60000, "ProcessMotorHome 2700"))
            {
                fHome->ListBox1->Items->Insert(0, "Check Index Y must on");
                ResetOKDeleyTime.SetMSAndOn(100);
                fHome->iHomeStep=2800;
            }
            break;
        case 2800:
            if(ResetOKDeleyTime.Off() && MOT[MTestY1].GalilTwoY_Move(-10, 10, 60000, "ProcessMotorHome 2800"))
            {
                ResetOKDeleyTime.SetMSAndOn(200);
                fHome->iHomeStep=2900;
            }
            break;
        case 2900:
            if(ResetOKDeleyTime.Off())
            {
                flag1=true;
                MOT[MTestZ1].ScanMotorStatus();
                MOT[MTestZ2].ScanMotorStatus();
                MOT[MTestY1].ScanMotorStatus();
                MOT[MTestY2].ScanMotorStatus();
            #ifndef SOFT_SIMULTE                                                //JerryYang 20170202 (Steven) 避免軟體模擬時啟用D13造成無法回home
                if(MOT[MTestY1].Led[iHomeLed]==false)
                {
                    MNetLog("Index Y1 not at home! (2900)");
                    flag1=false;
                }

                if(MOT[MTestY2].Led[iHomeLed]==false)
                {
                    MNetLog("Index Y2 not at home! (2900)");
                    flag1=false;
                }
            #endif
                if(flag1==false)
                {
                    RecordProcess("Index find home fail, try again! (2900)");
                    fHome->iHomeStep=1;
                }
                else
                {
                    fHome->ListBox1->Items->Insert(0, "Check Index Y OK");
                    if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true)                                        //Isaac 20201110 : Index Y find motor phase
                    {
                        if(IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess==true)
                        {
                            if(bZ1ModifyDistanceRef==true || bZ2ModifyDistanceRef==true)
                            {
                                fHome->iHomeStep=1300;
                            }
                            else
                            {
                                fHome->iHomeStep=3000;
                            }
                        }
                        else if(IniConfig.bD63_1FindMotorPhaseEveryGoHomeProcess==false &&
                                bFindMotorPhaseEveryGoHomeProcess==false)
                        {
                            if(bZ1ModifyDistanceRef==true || bZ2ModifyDistanceRef==true)
                            {
                                fHome->iHomeStep=1300;
                            }
                            else
                            {
                                fHome->iHomeStep=3000;
                            }
                        }
                        else
                        {
                            fHome->iHomeStep=1300;
                        }
                    }
                    else
                    {
                        fHome->iHomeStep=1300;
                    }
                    ResetOKDeleyTime.SetMSAndOn(30000);
                }
            }
            break;
        case 3000:                                                              //Ifor 20170817 (wei) add Find Index Z Phase Start
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, 30000, "ProcessMotorHome 3000"))                      //移動至安全位置
            {
                fHome->iHomeStep=3050;                                          //Isaac 20201110 : Index Y find motor phase
                Index_Z1_FindZPosition=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z1_Home_Position", 120);        //Ifor 讀取到find phase的預備位置
                Index_Z2_FindZPosition=ReadIniData(asGeneralPath, "IndexDriver", "Index_Z2_Home_Position", 120);

                if(Index_Z1_FindZPosition>0)
                    Index_Z1_FindZPosition=-(Index_Z1_FindZPosition-10);
                if(Index_Z2_FindZPosition>0)
                    Index_Z2_FindZPosition=-(Index_Z2_FindZPosition-10);

                if(Index_Z1_FindZPosition<-3000)
                {
                    ShowMyMessage("Index Z1 pos is too low!Plz set Index_Z1_Home_Position as 120 in Gerneral.ini file", "Index Z1 馬達尋相開始位置過低!請至Gerneral.ini 回復Index_Z1_Home_Position參數為120");
                    return false;
                }

                if(Index_Z2_FindZPosition<-3000)
                {
                    ShowMyMessage("Index Z2 pos is too low!Plz set Index_Z2_Home_Position as 120 in Gerneral.ini file", "Index Z2 馬達尋相開始位置過低!請至Gerneral.ini 回復Index_Z2_Home_Position參數為120");
                    return false;
                }
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3050:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Middle_Home, Prod.TestY2_Rear, 400000, "ProcessMotorHome 3050"))                                         //kevin 20171012 (wei) add home    //Ifor 20170822 Middle 需使用Teach位置避免關ARM 數值為0   60000=>400000 //Isaac 20201110 : Index Y find motor phase
            {
                fHome->iHomeStep=3060;
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3060:                                                              //Isaac 20201110 : Index Y find motor phase
            if(MOT[MTestZ1].Gali_MotMoveNoWait(Index_Z1_FindZPosition, 50000, 0))                                       //移動上一次紀錄位置-20 30000 =>50000
            {
                fHome->iHomeStep=3100;
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3100:
            if(MOT[MTestZ1].Gali_FindZPhase())
            {
                fHome->ListBox1->Items->Insert(0, "Index Z1 Phase finish.");
                fHome->iHomeStep=3250;
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            if(fHome->HomeClass[MTestZ1]->Visible==true &&
               atoi(fHome->HomeClass[MTestZ1]->edPos->Text.c_str())>3000)       //Ifor 20170907 add Z Phase 保護避免過低壓到Socket
            {
                ShowErrorMessage("WAR0307", K_RETRY, MMSystem);
            }
            break;
        case 3250:
            if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Prod.TestY2_Middle_Home, 400000, "ProcessMotorHome 3250"))                                        //kevin 20171012 (wei) add home  //Ifor 20170822 Middle 需使用Teach位置避免關ARM 數值為0     //Isaac 20201110 : Index Y find motor phase，60000->400000
            {
                fHome->iHomeStep=3260;
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3260:                                                              //Isaac 20201110 : Index Y find motor phase
            if(MOT[MTestZ2].Gali_MotMoveNoWait(Index_Z2_FindZPosition, 50000, 0))                                       //移動上一次紀錄位置-20 30000 =>50000
            {
                fHome->iHomeStep=3300;
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3300:
            if(MOT[MTestZ2].Gali_FindZPhase())
            {
                fHome->ListBox1->Items->Insert(0, "Index Z2 Phase finish.");
                fHome->iHomeStep=3400;
                ResetOKDeleyTime.SetMSAndOn(30000);
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            if(fHome->HomeClass[MTestZ2]->Visible==true &&
               atoi(fHome->HomeClass[MTestZ2]->edPos->Text.c_str())>3000)       //Ifor 20170907 add Z Phase 保護避免過低壓到Socket
            {
                ShowErrorMessage("WAR0308", K_RETRY, MMSystem);
            }
            break;
        case 3400:
            if(MOT[MTestY1].GalilTwoY_Move(0, 0, 400000, "ProcessMotorHome 3400"))                                      //Isaac 20201110 : Index Y find motor phase
            {
                if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true)    //Isaac : Find Y phase
                {
                    Index_Y1_FindZPosition=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y1_Home_Position", 20);
                    Index_Y2_FindZPosition=ReadIniData(asGeneralPath, "IndexDriver", "Index_Y2_Home_Position", 20);

                    if(Index_Y1_FindZPosition<0)
                    {
                        Index_Y1_FindZPosition=-Index_Y1_FindZPosition;
                    }

                    if(Index_Y2_FindZPosition>0)
                    {
                        Index_Y2_FindZPosition=-Index_Y2_FindZPosition;
                    }
                    Index_Y1_FindZPosition=(Index_Y1_FindZPosition-10);
                    Index_Y2_FindZPosition=(Index_Y2_FindZPosition+10);

                    if(Index_Y1_FindZPosition<=-20 || Index_Y1_FindZPosition>=3000)
                    {
                        ShowMyMessage("Index Y1 file is not correct!Plz set Index_Y1_Home_Position as 20 in Gerneral.ini file", "Index Y1 馬達尋相開始位置異常!請至Gerneral.ini 回復Index_Y1_Home_Position參數為20");
                        return false;
                    }

                    if(Index_Y2_FindZPosition>=20 || Index_Y2_FindZPosition<=-3000)
                    {
                        ShowMyMessage("Index Y2 file is not correct!Plz set Index_Y2_Home_Position as 20 in Gerneral.ini file", "Index Y2 馬達尋相開始位置異常!請至Gerneral.ini 回復Index_Y2_Home_Position參數為20");
                        return false;
                    }
                    fHome->iHomeStep=3600;
                    ResetOKDeleyTime.SetMSAndOn(200);
                }
                else
                {
                    fHome->iHomeStep=1300;
                    ResetOKDeleyTime.SetMSAndOn(30000);
                }
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3600:                                                              //Isaac 20201110 : Index Y find motor phase
            if(ResetOKDeleyTime.Off())
            {
                if(MOT[MTestY1].GalilTwoY_Move(Index_Y1_FindZPosition, Index_Y2_FindZPosition, 60000, "ProcessMotorHome 3600"))
                {
                    fHome->iHomeStep=3610;
                    fHome->ListBox1->Items->Insert(0, "Index Y Start Find Phase.");
                    flag12=false;
                    flag13=false;
                }
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            break;
        case 3610:                                                              //Isaac 20201110 : Index Y find motor phase
            if(flag12==false)
                flag12=MOT[MTestY1].Gali_FindZPhase();
            if(flag13==false)
                flag13=MOT[MTestY2].Gali_FindZPhase();

            if(flag12==true && flag13==true)
            {
                fHome->ListBox1->Items->Insert(0, "Index Y1 and Y2 Phase Both Finish.");
                bFindMotorPhaseEveryGoHomeProcess=true;
                fHome->iHomeStep=1300;
                ResetOKDeleyTime.SetMSAndOn(30000);
                flag12=false;
                flag13=false;
            }
            fHome->ShowMotorHomePos(MTestZ1);
            fHome->ShowMotorHomePos(MTestZ2);
            fHome->ShowMotorHomePos(MTestY1);
            fHome->ShowMotorHomePos(MTestY2);
            if(fHome->HomeClass[MTestY1]->Visible==true &&
               atoi(fHome->HomeClass[MTestY1]->edPos->Text.c_str())>4000)       //Ifor 20170907 add Z Phase 保護避免過低壓到Socket
            {
                ShowErrorMessage("WAR03314", K_RETRY, MMSystem);
            }

            if(fHome->HomeClass[MTestY2]->Visible==true &&
               atoi(fHome->HomeClass[MTestY2]->edPos->Text.c_str())<-4000)      //Ifor 20170907 add Z Phase 保護避免過低壓到Socket
            {
                ShowErrorMessage("WAR03315", K_RETRY, MMSystem);
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
//  TfHome::GaliMotorServoOff -- golden uhome.cpp:4988-5016, statement for statement.
//
//  AI(W906-MT-E3b) 20260925: golden's "motor power OFF" (Steven 20230712 -- it cuts SwMotorRelay/SwServerON
//  and HOLDS every brake group, the fix for 'SwServoOn.Off must hold the Z brake').  Callers now live:
//  csystem.cpp CheckMotorPowerShutDown ([Power Off] key, GATE G29 lifted) and DoOneCycleFinishCheck
//  (W7C2_FHOME_SERVOOFF); the web Motor Power OFF path calls it through the MotorAccess backend.
//  Faithful, including what golden does NOT do (lead default 20260925, recorded for the operator):
//    * no MOT[].HomeFlag is cleared -- after power-off the axes still count as homed (golden);
//    * no servo-off is sent to any axis -- power is removed by the relay (golden).
//  ONE port-only line: golden StopAllMotor() walks the golden MOT[] objects and cannot reach the axes the
//  1203 monitor opened, so W906_Stop1203AllHook (csystem.h, NULL in ctest) is called right after it.
//  The Galil "ST" is golden's; on HT9050 there is no Galil card (bGali_CardInstall false, no DMC command).
//------------------------------------------------------------------------------
void TfHome::GaliMotorServoOff(AnsiString sFunc)                                //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
{
    StopAllMotor();
    if(W906_Stop1203AllHook != 0)                                               // AI(W906-MT-E3b) 20260925: port-only, see above
        W906_Stop1203AllHook((AnsiString("GaliMotorServoOff - ")+sFunc).c_str());
    InitGali_HomeTask();
    MOT[MTestY1].Gali_Command("ST", __FUNC__);
    MOT[MTestY1].MovFlag=false;
    MOT[MTestY2].MovFlag=false;
    MOT[MTestZ1].MovFlag=false;
    MOT[MTestZ2].MovFlag=false;
    MOT[MTestY1].bScanFlag=false;
    MOT[MTestY2].bScanFlag=false;
    MOT[MTestZ1].bScanFlag=false;
    MOT[MTestZ2].bScanFlag=false;
    MOT[MTestY1].GaliSofDelayCount=0;
    MOT[MTestZ1].GaliSofDelayCount=0;
    MOT[MTestZ2].GaliSofDelayCount=0;
    MOT[MTestY2].GaliSofDelayCount=0;
    SystemStart=false;
    SW[SwMotorRelay].Off();
    SW[SwServerON].Off();
    bMotorPowerState=false;
    IndexMotorBreakerOFF();
    MagazineBreakerOFF();                                                       //JerryYang 20220909 : add magazine
    InOutArmZBreakerOFF();                                                      //add One sucker with rotate
    LDCarRotArmZBreakerOFF();                                                   //RogerYang 20250828 add for Loader Rotate Arm
    CassetteBreakerOFF();                                                       //Ifor 20251216 add:Boat Carrier
    fAllMotorHome=false;
    RecordProcess(AnsiString("GaliMotorServoOff - ")+sFunc);
}
//------------------------------------------------------------------------------
//  TfHome::sbAbortHomeClick -- golden uhome.cpp:4980-4986, statement for statement.
//
//  AI(W906-MT-E3b) 20260925: translated so that golden's engine caller (csystem.cpp MainProc's stop arm,
//  SAFETY-GATE(W906-T6-ABORTHOME), golden csystem.cpp:18923-18924 `if(fHome->fShow) fHome->sbAbortHomeClick(fHome);`)
//  runs again: a STOP while the home sequence is on screen aborts the home the golden way -- every motor
//  stopped, motor power cut, every brake held (GaliMotorServoOff), fAbort=true, and the home "form" closed
//  (fShow=false, which is what ProcessMotorHome's 16 `fHome->fShow==false` checks read).
//  ONE line gated, missing dependency: `sbAbortHome->Down=false;` -- TfHome has no sbAbortHome widget; the
//  Abort button lives on the web Home Monitor page (it is not a toggle there).  UN-GATE when the facade grows
//  the button.  Sender is unused, as in golden.
//------------------------------------------------------------------------------
void TfHome::sbAbortHomeClick(void * /*Sender*/)
{
    GaliMotorServoOff("sbAbortHomeClick");                                      //Steven 20230712 : 修正SwServoOn.Off時, 要抓住Z煞車
    fAbort=true;
#if 0 // GATE (W906-MT-E3b-ABORTBTN): 缺相依 TfHome::sbAbortHome（golden .dfm TSpeedButton），見上面的就地註解
    sbAbortHome->Down=false;                                                    // golden :4984
#endif // GATE (W906-MT-E3b-ABORTBTN)
    Close();
}

// ===========================================================================
//  AI(W906-SMHOME) 20260927: golden 單軸回原點（uhome.cpp:397-401 InitProcessSingleMotorTask、:415-644 ProcessSingleMotorHome）逐行照翻：
//  一次回一顆馬達的原點（iSingleMotorHomeTask[Index] 狀態機，本檔 :137 已定義）。本檔檔頭（:31-32）當初把它留給「另一顆 commit」，就是這一顆。
//  以前 acatchtray_shims.cpp 的兩支樁：InitProcessSingleMotorTask 什麼都不做、ProcessSingleMotorHome 一律回 true（＝「那顆已經回好了」）⇒
//  27 個呼叫者（DoTopBtmHome 的 AOI 四軸、Tray／Lifter 等單軸回原點）以為回好了，實際上沒動。
//  移植樹沒有的相依才閘（GATE(W906-SMHOME)）。
// ===========================================================================
// ---- golden uhome.cpp:58 與 :396（檔案層計時器；移植樹 0 命中，同檔 tTopBtmDelay／hHomeDelay 是同一種先例）----
TQPF_Timer SingleHomeDelay;
TQPF_Timer SMotorHomeDeleyTime[TOTAL_MOTOR];
// ---- golden uhome.cpp:397-401 ----
void InitProcessSingleMotorTask(int Index)
{
    iSingleMotorHomeTask[Index]=1;
    fHome->iHomeStep=1;
}
// ---- golden uhome.cpp:415-644 ----
bool ProcessSingleMotorHome(int Index)
{
    int ret, iRef;
    int &iTask=iSingleMotorHomeTask[Index];
    switch(iTask)
    {
        case 1:                                                                 //檢查傳入的index是否在範圍內
            if(Index<0 || Index>=TOTAL_MOTOR)
            {
                ShowMyMessage("Error motor index!", "錯誤的馬達索引表", "ProcessSingleMotorHome");
                return true;
            }
            iTask=200;
            break;
        case 200:                                                               //檢查該馬達是否有被Enable
            MOT[Index].MotorInitial();
            if(MOT[Index].Motor->Enable==false)
            {
                MOT[Index].HomeFlag=1;
                MOT[Index].Position=0;                                          //Sam 20211009 : 修正軟體模擬 Home 失敗問題
                iTask=1;
                return true;
            }
            iTask=300;
            break;
        case 300:
            ret=MOT[Index].MotorHome(0);
            if(MOT[Index].HomeFlag==0 && ret==2)
            {
                MOT[Index].PCIL132_StopMotor();
                iTask=1;
                iRef=MOT[Index].GetErrorIndex();
                if(iRef==9) iRef=7;
                JamCode=MotorIndexToJamCode(Index);
                ShowMotorErrorMessage(JamCode, iRef+1);
                iTask=1;
                return false;
            }
            else if(ret==3)
            {
                iRef=MOT[Index].GetErrorIndex();
                if(iRef==9) iRef=6;
                JamCode=MotorIndexToJamCode(Index);
                if(fNote->fShow==false)                                         //Jimmychiu : 20221122 avoid double Alarm to stop
                    ShowMotorErrorMessage(JamCode, iRef+1);
                iTask=1;
                return false;
            }
            else if(ret==1 && MOT[Index].HomeFlag)
            {
                if(Index==MInArmZA ||
                   Index==MInArmZB ||
                   Index==MInArmZC ||
                   Index==MInArmZD ||
                   Index==MInArmZE ||
                   Index==MInArmZF ||
                   Index==MInArmZG ||
                   Index==MInArmZH ||
                   Index==MOutArmZA ||
                   Index==MOutArmZB ||
                   Index==MOutArmZC ||
                   Index==MOutArmZD ||
                   Index==MOutArmZE ||
                   Index==MOutArmZF ||
                   Index==MOutArmZG ||
                   Index==MOutArmZH ||
                   Index==MInArmZAe ||
                   Index==MInArmZAf ||
                   Index==MInArmZAg ||
                   Index==MInArmZAh ||
                   Index==MInArmZBe ||
                   Index==MInArmZBf ||
                   Index==MInArmZBg ||
                   Index==MInArmZBh ||
                   Index==MOutArmZAe ||
                   Index==MOutArmZAf ||
                   Index==MOutArmZAg ||
                   Index==MOutArmZAh ||
                   Index==MOutArmZBe ||
                   Index==MOutArmZBf ||
                   Index==MOutArmZBg ||
                   Index==MOutArmZBh ||
                   Index==MOutSortAa ||                                         //RogerYang 20250402 :for HT9046AU Add
                   Index==MOutSortAb)                                           //RogerYang 20250402 :for HT9046AU Add
                {
                    MOT[Index].HomeFlag=false;
                    iTask=500;
                    return false;
                }

                if(USE_MAGNETIC_SCALE)
                {
                    iTask=600;
                }
                else
                {
                    iTask=1;
                    return true;
                }
            }
            else if(ret==2)
            {
                MOT[Index].MotorInitial();
                iTask=400;
                break;
            }
            break;
        case 400:
            ret=MOT[Index].MotorHome(0);
            if(MOT[Index].HomeFlag==2 && ret==2)
            {
                MOT[Index].PCIL132_StopMotor();
                iTask=1;
                iRef=MOT[Index].GetErrorIndex();
                if(iRef==9) iRef=7;
                JamCode=MotorIndexToJamCode(Index);
                ShowMotorErrorMessage(JamCode, iRef+1);
                iTask=1;
                return false;
            }
            else if(ret==3)
            {
                iRef=MOT[Index].GetErrorIndex();
                if(iRef==9) iRef=6;
                JamCode=MotorIndexToJamCode(Index);
                ShowMotorErrorMessage(JamCode,iRef+1);
                iTask=1;
                return false;
            }
            else if(ret==1 && MOT[Index].HomeFlag)
            {
                if(Index==MInArmZA ||
                   Index==MInArmZB ||
                   Index==MInArmZC ||
                   Index==MInArmZD ||
                   Index==MInArmZE ||
                   Index==MInArmZF ||
                   Index==MInArmZG ||
                   Index==MInArmZH ||
                   Index==MOutArmZA ||
                   Index==MOutArmZB ||
                   Index==MOutArmZC ||
                   Index==MOutArmZD ||
                   Index==MOutArmZE ||
                   Index==MOutArmZF ||
                   Index==MOutArmZG ||
                   Index==MOutArmZH ||
                   Index==MInArmZAe ||                                          //Ztex 2023.12.25 For HT-1032AT
                   Index==MInArmZAf ||
                   Index==MInArmZAg ||
                   Index==MInArmZAh ||
                   Index==MInArmZBe ||
                   Index==MInArmZBf ||
                   Index==MInArmZBg ||
                   Index==MInArmZBh ||
                   Index==MOutArmZAe ||
                   Index==MOutArmZAf ||
                   Index==MOutArmZAg ||
                   Index==MOutArmZAh ||
                   Index==MOutArmZBe ||
                   Index==MOutArmZBf ||
                   Index==MOutArmZBg ||
                   Index==MOutArmZBh ||                                         //Ztex 2023.12.25 For HT-1032AT
                   Index==MOutSortAa ||                                         //RogerYang 20250402 :for HT9046AU Add
                   Index==MOutSortAb)                                           //RogerYang 20250402 :for HT9046AU Add
                {
                    MOT[Index].HomeFlag=false;
                    iTask=500;
                    return false;
                }

                if(USE_MAGNETIC_SCALE)
                {
                    iTask=600;
                }
                else
                {
                    iTask=1;
                    return true;
                }
            }
            break;
        case 500:                                                               //只有In / Out Arm吸嘴
            if(MOT[Index].MotorMove(ZSafePos))
            {
                iTask=1;
                MOT[Index].HomeFlag=true;
                return true;
            }
            break;
        case 600:                                                               //Steven 20160426 : 磁性尺
            if(Index==MInArmX || Index==MInArmY || Index==MOutArmX || Index==MOutArmY)
                SingleHomeDelay.SetSecAndOn(0.5);
            else
                SingleHomeDelay.SetSecAndOn(0.01);
            iTask=610;
            break;
        case 610:
            if(SingleHomeDelay.Off())
            {
                if(Index==MInArmX)
                {
                    MOT[MInArmXScale].Motor->ResetPos(MOT[MInArmX].ReadEncoderPos());
                    MOT[MLightScale].Motor->ResetPos(MOT[MInArmX].ReadEncoderPos());
                }

                if(Index==MInArmY)
                {
                    MOT[MInArmYScale].Motor->ResetPos(MOT[MInArmY].ReadEncoderPos());
                    MOT[MLightScale].Motor->ResetPos(MOT[MInArmY].ReadEncoderPos());
                }

                if(Index==MOutArmX)
                {
                    MOT[MOutArmXScale].Motor->ResetPos(MOT[MOutArmX].ReadEncoderPos());
                    MOT[MLightScale].Motor->ResetPos(MOT[MOutArmX].ReadEncoderPos());
                }

                if(Index==MOutArmY)
                {
                    MOT[MOutArmYScale].Motor->ResetPos(MOT[MOutArmY].ReadEncoderPos());
                    MOT[MLightScale].Motor->ResetPos(MOT[MOutArmY].ReadEncoderPos());
                }
                iTask=1;
                return true;
            }
            break;
    }
    return false;
}
