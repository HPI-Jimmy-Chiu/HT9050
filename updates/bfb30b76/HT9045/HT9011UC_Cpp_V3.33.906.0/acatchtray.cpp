// =============================================================================
//  acatchtray.cpp  --  TrayArm ENGINE  (W7 translation)
//
//  Faithful translation of golden  acatchtray.cpp (9142 lines, BCB6, Big5/cp950).
//  Translator: AI(W7-CATCHTRAY) 20260626
//
//  ROLE: this is the TrayArm ENGINE -- it moves trays between Loader / Empty /
//  Color / Auto / Buffer via MTrayX + the CatchTraySuck KitSuck.  The top-level
//  dispatcher DoCatchTray() (a flat switch(CatchTrayTask)) ticks the cycle and
//  calls the lower task SMs by name: case 250->DoCatchFromLoader, 260->
//  DoCatchUnderTray, 400/500/600->CatchNewTrayFromBuffer, 1100->DoPlaceTrayToAuto,
//  2120->DoPlaceToBuffer, 20/50->DoSlapTray.  There is NO machine-type / site-
//  variant if-ladder in this file (verified: 0 hits of iInArmType-style dispatch);
//  the only runtime mode switching is the TRAY_ARM_MODE==eUnderCoveyor/
//  eAboveCoveyor predicate + AUTO3_IS_MAGAZINE / CUSTOMER_CODE flags inside cases,
//  all kept VERBATIM (they evaluate to the offline default branch over the Sim HAL).
//
//  WAVE SCOPE (W7) -- ACTIVE vs satisfied-by-shim:
//
//   ACTIVE (faithful, verbatim formulas / branches / cursor flow / fall-throughs /
//   #ifdef SOFT_SIMULTE blocks -- SOFT_SIMULTE is NOT defined so #ifndef paths run,
//   #ifdef paths stay verbatim but inert):
//     * the top dispatcher DoCatchTray (CatchTrayTask, golden :5993=1) and ALL
//       task SM skeletons (switch + cursor transitions) kept ACTIVE + pumpable:
//         DoCatchFromLoader        (iCatchFromLoaderTask,        golden :1453=1)
//         CatchNewTrayFromBuffer   (iCatchNewTrayFromBufferTask, golden :2407=1)
//         DoPlaceTrayToAuto        (iPlaceTrayToAutoTask,        golden :3717=1)
//         DoPlaceToBuffer          (iPlaceToBufferTask,          golden :5069=1)
//         DoSlapTray               (iSlapTrayTask,               golden :61=1)  -- cursor oracle
//         DoCatchUnderTray         (iCatchUnderTrayTask,         golden :900=1)
//         DoSupportUnderTray       (iSupportUnderTrayTask,       golden :1111=1)
//         DoPlaceBufferTray        (iPlaceBufferTrayTask,        golden :1299=1)
//         C_CatchTray_Fix_Puch/Pop (iCatchTray_Fix_Puch/Pop,     golden :62/63=1)
//         DoLoadCarRotArmReadRFID / CheckCoverTrayID / CatchCoverTray /
//         PlaceCoverTray / ReceiveCoverTray / DoCoverTray (cover-tray subsystem)
//         DoLoadToCassette / DoCassetteToAuto (Boat Carrier)
//     * the 3 GENUINE C fall-throughs are PRESERVED:
//         DoCatchFromLoader case 1 -> 10  (golden :1507-1508, `Task=10;` no break)
//         DoSlapTray        case 200 -> 300 (golden :5928-5930, `Task=300; //break;`)
//         plus the many task-driven `case N: ...` no-break flows kept verbatim.
//     * the pure decision helpers: WhichAutoNeedTray (whole body -- int/array logic
//       over globals; offline takes the default-path returns -- the TrayArm engine
//       OWNS this symbol; the W6.1 canary_support.cpp sim stub was REMOVED to avoid
//       an ODR/link collision), IsLoaderCoverTray, IsPlaceToColor,
//       IsTrayArmMoveAvoidOutArmCrash, IsTrayArmAtEmptyOrColor, MTrayXCanSafeMove,
//       InArmZInSafe, InArmXYZSafe, bCheckNoCatchEmptyTrayToAuto,
//       IsTrayArmCatchTrayFail, AddTrayCount/AddFixTrayCount/ClearAllTrayCount,
//       SwapTrayID, IsOutArmSafe, AvoidOutArm, GetTraySeperateCynState,
//       CatchFromLoaderSetItemData/PlaceToBufferSetItemData/CatchTraySetItemData.
//     * hardware behind the proven Sim HAL: MOT[] (Motor/mymotor.h) / Cylinder[]
//       (mycylin.h) / Sen[] (mysensor.h) / SW[] (myswitch.h) / TrayArmMotorMove /
//       TrayMoveIn / TrayMoveOut (asendic.cpp + mymotor.cpp) / CatchTraySuck
//       KitSuck (aHotPlateSubstrate) / TQPF_Timer (myTimer.h).
//
//   SATISFIED-BY-SHIM (so the engine LINKS + the SMs pump):
//     * acatchtray_shims.{h,cpp} -- the W7 cross-module shim TU: SECS_EVENT /
//       EventReport / fTrayMapping / MyMessageBox / LoadCCD / AMR / TColor /
//       BoolToStr / the Magazine-AMR + in-arm-position + single-motor-home free
//       funcs + iCoverTrayIDTask / iReadCIDAction.  Every method is offline no-op
//       / false / 0 so the chosen branches match the proven canary (no AMR, no
//       Magazine, no TrayMapping CCD/RFID, sensors idle).
//     * the VCL form pointers fMain / fAGV / fSortCT / fLotInfo / fSCKART live in
//       FormsFacade (extended W6.3/W7 with the TrayArm members).
//     * LAST_GENERAL_SET ART/ATK tray-count fields added to the shared canary
//       LastSet shim (canary_support.h).
//     * a few genuinely un-shimmable derefs are wrapped `#if 0 // TODO(W7-UI)`
//       (behaviour-preserving): HSys.BinDisCtrl-> (TMyBinDispCtrl fwd-decl only,
//       BinDisCtrl==NULL offline -> guard is false).
//
//  VCL/Borland conversions: #pragma hdrstop (golden :2) and
//  #pragma package(smart_init) (golden :40) dropped (MinGW).  AnsiString (1-based)
//  + AnsiString::sprintf (56 uses) via vclcompat (kept verbatim).  __FUNC__ via
//  the W6.0 shim.  TQPF_Timer via myTimer.h.  HTimer (golden HTimer.h, gated out
//  of MachineDefine.h) -> minimal offline alias of TQPF_Timer below (same
//  .SetSecAndOn/.Off surface the engine uses).  NO __fastcall / __property /
//  __published / PACKAGE / TStringList in this file (verified 0 hits).
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8.
//  Final gate: ZERO U+FFFD.
// =============================================================================
#include "MachineDefine.h"

#include "acatchtray.h"

#include "MachineType.h"
#include "Motor/mymotor.h"          // MOT[], TMyMotor, TTrayMotor, TrayArmMotorMove
#include "mycylin.h"                // Cylinder[]
#include "mysensor.h"               // Sen[]
#include "myswitch.h"               // SW[]
#include "asendic.h"                // TrayMoveIn / TrayMoveOut
#include "aHotPlateSubstrate.h"     // CatchTraySuck KitSuck + InArmSuck/OutArmSuck + cursors
#include "cprod.h"                  // Prod / TestIF / TestIF_File / TrayForm / ArmSpeed / ...
#include "cpublic.h"                // CosFunction
#include "cmydef.h"                 // global scalar universe + IC consts + tray globals
#include "csystem.h"                // HasICUnderMachine / IndexHasIC / ReadWriteTrayID / DoLockUnloader / iReceiveAutoTrayTask
#include "common.h"                 // asTrayIDByLot
#include "database.h"               // HSys (SYSTEM_MODULAR) + BinDisCtrl
#include "BinDisplay/MyBinDisp.h"   // AI(W906-BinDispInt) 20260820: complete TMyBinDispCtrl (FlashPro/ClearAutoChangingWarn flips)
#include "FormsFacade.h"            // fMain / fAGV / fSortCT / fLotInfo / fSCKART offline stand-ins
#include "canary_support.h"         // LastSet (+ART fields) / ShowErrorMessage / ShowMyMessage / RecordProcess / ShowUnloaderTrayMessage
#include "myTimer.h"                // TQPF_Timer
#include "acatchtray_shims.h"       // W7 cross-module shims: SECS_EVENT/EventReport/fTrayMapping/MyMessageBox/LoadCCD/TColor/BoolToStr/free funcs
#include "Automation/AMR.h"         // AI(W5-Automation-Integrate) 20260710: real TTeraPowerAMR AMR (was TAMRShim in acatchtray_shims)

//==============================================================================
//==  (W7) HTimer  -- golden HTimer.h (a high-precision timer).  HTimer.h is gated
//==  out of the target MachineDefine.h (#if 0 block) so the type is not visible.
//==  The TrayArm engine uses ONE HTimer (DoMagazineGetTrayTime) and only its
//==  .SetSecAndOn / .Off surface, which TQPF_Timer (myTimer.h) provides verbatim.
//==  Alias HTimer to TQPF_Timer so the file-scope `HTimer DoMagazineGetTrayTime;`
//==  compiles with identical offline semantics.  TODO(W7): real HTimer.h.
//==============================================================================
#ifndef HT9045_HTIMER_SHIM
#define HT9045_HTIMER_SHIM
typedef TQPF_Timer HTimer;                                                     // golden HTimer.h -- offline alias of the QPF timer
#endif

//==============================================================================
//==  (W7) eATkTrayFeed PutXxx enum members  -- golden Automation/AGV.h:215-217.
//==  WhichAutoNeedTray compares LastSet.iUnloadFixTray against these three.
//==  (eAtkTfFeedFix=7 / eAtkTfFixToAMR=8 are already in acatchtray_shims.h.)
//==  Ordinals VERBATIM from golden AGV.h enum eATkTrayFeed.
//==============================================================================
#ifndef HT9045_EATKTRAYFEED_PUT_SHIM
#define HT9045_EATKTRAYFEED_PUT_SHIM
enum { eAtkTfPutIDTray = 3, eAtkTfPutCover = 4, eAtkTfPutEmptyTray = 5,           // golden AGV.h:215-217
       eAtkTfFeedFix = 7, eAtkTfFixToAMR = 8 };                                 // golden AGV.h:219-220 (AddTrayCount comparisons)
#endif

//==============================================================================
//==  (W7) module globals golden declares extern in not-yet-ungated cmydef.cpp /
//==  cprod.cpp blocks that the dispatcher reads.  Golden homes noted.  Defined
//==  here offline-safe (this file owns them this wave).  TODO(W7): move to
//==  cmydef.cpp when its blocks are ungated.
//==============================================================================
int  iInArmWaitPosition = 0;                                                   // golden cmydef.cpp (Ifor 20191105) -- in-arm safe-pos state
bool bPlaceToHotplate   = false;                                               // golden cmydef.cpp (jou) -- in-arm placing to hotplate
bool bPlaceShuttle      = false;                                               // golden cmydef.cpp -- in-arm placing to shuttle
int  iInRotateFinish    = 0;                                                   // golden cmydef.cpp (Sam 20190828) -- in-rotate finish state

//==============================================================================
//== AI(W906-CommonWaveFile) 20260721: REMOVED the offline no-op WriteDataToFile
//== stub that used to live here (golden common.h:256 AnsiString overload). Its
//== common.cpp body is un-gated and real as of this wave; keeping this stub
//== would be a GNU ld "multiple definition" link error (both ht9045_core and
//== ht9045_sm meet at final link). This file's own 2 call sites (below) are
//== unchanged and now correctly bind to the real common.cpp implementation --
//== a genuine, harmless behavior upgrade for this file: no-op -> real file
//== write (same extern-then-call pattern BarCode_Bottom2DID8CCD.cpp:36 already
//== used against this stub; it too now binds to the real function).
//==============================================================================
//==  (W7) SetAutoSkipCount  -- golden ainarm9045.h:84 (REAL home: ainarm9045.cpp,
//==  already in lib ht9045_sm).  The engine calls it; declare extern so it binds
//==  to the in-arm engine's real definition (NOT redefined here -> no ODR clash).
//==============================================================================
extern void SetAutoSkipCount(int Add);                                         //Steven 20150217 (golden ainarm9045.h:84)

//==============================================================================
//==  (W7) DoReceiveAutoTray  -- golden (Eastsun 20260515 F020).  Declared extern
//==  in acatchtray.h; its golden home (auto-receive bin-tray AMR flow) is NOT in
//==  scope this wave -> ACTIVE offline stub (false: nothing to receive).  The
//==  AMR cases 410/424 that call it are dormant offline (TrayForm.bEnableAMR
//==  false), so this never drives behaviour.  TODO(W7).
//==============================================================================
#if 0   // PT-W5c RETIRED (DoReceiveAutoTray)
//AI(ht9045-v906) 20260809: PT-W5c -- RETIRED. The real faithful body now lives in csystem.cpp (its golden home); keeping this stand-in is a multiple-definition error, measured in build_0809_w5c. Same convention as csystem_shims.cpp:165.
bool DoReceiveAutoTray(int /*pos*/) { return false; }
#endif

bool AvoidOutArm(AnsiString S,int Task);                                        //Sam 20191120 : add log
bool IsOutArmSafe();
bool IsPlaceToColor(int iStep);                                                 //Sam 20221027 : 整合 P24 功能
int IsLoaderCoverTray();                                                        //Eastsun 20260515 F016 整合:KYEC AMR Loader cover-tray status
void InitialCheckCoverTrayIDTask();                                             //Eastsun 20260515 F011 forward decl: caller-before-def fix
bool CheckCoverTrayID();                                                        //Eastsun 20260515 F011 forward decl: caller-before-def fix

extern void SetUnloaderInfoFile(int iUnloader);                                 //ChungHung 20150205 add for ATK
TQPF_Timer CatchTrayDelay;
extern bool fOutArmDisableManualCylinder;
extern bool fCatchTrayDisableManualCylinder;
bool bHasTrayInEmpty=false;
extern bool bOldAutoHasTray[MAX_AUTO_TRAY];
bool bAutoNeedTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};
bool bAutoEdgePush[MAX_AUTO_TRAY]={false, false, false, false, false, false};
TQPF_Timer DoPlaceTrayToAutoDelay;
TQPF_Timer tCheckC_Color_Fix_OffSensor;                                         //Sam 20200327 : 增加 Time Out
TQPF_Timer tCCDCheckTime;                                                       //kevin 20220401 LOAD CCD  確認是殘留IC
TQPF_Timer DoP43UnloaderCylinderLoop;
TQPF_Timer tGetLdRFIDDelay;                                                     //RogerYang 20250830 : Add RFID
int iSlapTrayTask=1;                                                            //ChungHung 20140701 add AutoRetest
int iCatchTray_Fix_Puch=1;
int iCatchTray_Fix_Pop=1;
static bool bHasEmptyTrayPreAlarm=false;                                        //Ifor 20170417 (wei) add 四軌 Empty Tray PreAlarm
static bool bHascolorTrayPreAlarm=false;                                        //Ifor 20170417 (wei) add 四軌 color Tray PreAlarm
extern void InitDoOutArmTeachAlignmentProcessTask();                            //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
extern bool MoveOutArmToAutoSafe_9045();                                        //Eastsun 20260515 F011 整合:KYEC AMR cover-tray extern decls
extern bool MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false);                    //Eastsun 20260515 F011 整合:KYEC AMR cover-tray extern decls
//------------------------------------------------------------------------------
//回傳: 0=移動中(未到位)  1=到位且encoder正確  2=到位但encoder誤差(已停車)      //AI(ht9045-motor-control) 20260617 (RogerYang) : TrayX 移動到位比照 In/Out Arm 逐次驗證 encoder
//AI(W906-F9050-BD) 20261004: 910 acatchtray.cpp:71-87 verbatim (= V912 :71-87; RogerYang 20260617), the only function the 9050 flow calls
//  that golden 906 lacks (ledger s0 #4) -- not the only 906->910 change it meets: IsOutArmSafe's 910 body differs (note at its call in DoPlaceTrayToAuto_9050 case 1).  Brought in ONLY for the *_9050 functions below (decision D4 = A): golden DoCatchFromLoader keeps
//  TrayArmMotorMove(Prod.iXTrayLoad) (910 / V912 changed that line too, 910 :1575 -- deliberately NOT done here, it is shared by
//  every model).  Note: the 9050 callers test `==1` only, so a 2 (encoder off by >1000 -> stop + RecordProcess) is retried
//  every tick with no alarm (910 as written).
int TrayXMoveCheckEnc(int iTarget)
{
    if(TrayArmMotorMove(iTarget) == false)
        return 0;                                  //尚未到位，維持原流程節奏

    int iEnc = MOT[MTrayX].ReadEncoderPos();       //讀實際 encoder
    if(iTarget > (iEnc + 1000) ||                  //iTorence 判斷
       iTarget < (iEnc - 1000))
    {
        RecordProcess("MTrayX Pos Error Tar=" + AnsiString(iTarget) +
                " Enc=" + AnsiString(iEnc));        //比照 "In Arm Pos Error" log
        MOT[MTrayX].PCIL132_StopMotor();
        return 2;                                   //誤差過大 → 不放行
    }
    return 1;
}
//------------------------------------------------------------------------------
int iDoLoadToCassetteTask=1;                                                    //Frank 20251217 add
void InitDoLoadToCassetteTask()
{
    iDoLoadToCassetteTask=1;
}
//------------------------------------------------------------------------------
bool DoLoadToCassette()                                                         //Frank 20251217 add
{
    int &Task=iDoLoadToCassetteTask;                                            //2013.02.19 Q_Q RTC INDEX MOTION CHANGE

    switch(Task)
    {
        case 1:
            Cylinder[C_TrayY_Fixer].Off();
            Cylinder[C_LoaderEdgePush].Off();
            if(bBoatChangeCasset[0]==false)
            {
                bBoatChangeCasset[0]=true;
                Task=100;
            }
            break;
        case 100:                                                               //TRAY 上升到 STACK
            TrayMoveOut(true, 0);
            if(LastSet.iRealDummy==DUMMY ||
                Sen[SenLoaderCCWDete].IsOff()==false)
            {
                Task=200;
            }
            break;
        case 200:
            if(LastSet.iRealDummy==DUMMY ||
                Sen[SenLoaderCCWDete].IsOff()==true)
            {
                Task=300;
            }
            break;
        case 300:
            if(Cylinder[C_LoaderCarrier].Push())
            {
                Task=400;
            }
            break;
        case 400:
            if(MOT[MLoaderY_CCW].MotorMove(Prod.iLDCassetteRear))
            {
                Task=410;
            }
            break;
        case 410:
            if(MOT[MLoaderY_CCW].MotorMove(Prod.iLDCassetteRearBack))
            {
                Task=420;
            }
            break;
        case 420:
            if(Cylinder[C_LoaderCarrier].Pop() &&
                Sen[SnLoaderSureTray].IsOff() &&
                Sen[SnLoaderPreDete].IsOff())
            {
                TrayMoveOut(false, 0);
                Task=500;
                TestIF_File.iCassetteLDNowLayer++;
                bBoatChangeCasset[0]=false;
                return true;
            }
            break;
        case 500:
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
int iDoCassetteToAutoTask[2]={1, 1};                                            //Frank 20251217 add
void InitDoCassetteToAutoTask(int iPos)
{
    iDoCassetteToAutoTask[iPos]=1;
}
//------------------------------------------------------------------------------
bool DoCassetteToAuto(int iPos)                                                 //Frank 20251217 add
{
    int &Task=iDoCassetteToAutoTask[iPos];
    static bool bYCWMove[2]={false, false};
    static bool bYCCWMove[2]={false, false};
//    static bool bZPushMove[2]={false, false};
    static int iLayerPos;

    if(iPos>1)
        return false;

    switch(Task)
    {
        case 1:
            if(Sen[SnAutoBoatActDetect[iPos]].IsOn())
            {
                if(iPos==0)
                    ShowErrorMessage("WAR1116", 0, MAuto1Z);
                else
                    ShowErrorMessage("WAR1216", 0, MAuto2Z);
                //不能移動
                return false;
            }
            Cylinder[C_AutoSide_Fixer[iPos]].Off();
            Cylinder[C_AutoEdgePush[iPos]].Off();
            bYCWMove[iPos]=false;
            bYCCWMove[iPos]=false;
            Task=10;
            break;
        case 10:
            bYCWMove[iPos]=MOT[MAuto1Y+iPos].MotorMove(Prod.iAutoCassetteFrontBack[iPos]);
            bYCCWMove[iPos]=MOT[MAuto1Y_CCW+iPos].MotorMove(Prod.iAutoCassetteRearBack[iPos]);
            if(bYCWMove[iPos]==true && bYCCWMove[iPos]==true)
            {
                bYCWMove[iPos]=false;
                bYCCWMove[iPos]=false;
                Task=20;
            }
        case 20:
            if(Cylinder[C_AutoCarrier[iPos]].Pop())
            {
                TestIF_File.iCassetteAutoNowLayer[iPos]++;
                iLayerPos=Prod.iAutoCassetteZStart[iPos]+((TestIF_File.iCassetteAutoNowLayer[iPos]-1)*UserDefForm[0].dCassetteZPitch);
                Task=100;
            }
            break;
        case 100:
            if(MOT[iTrayZMotor[iPos+3]].MotorMove(iLayerPos))
            {
                TrayMoveIn(true, iPos+3);
                Task=110;
            }
            break;
        case 110:
            bYCWMove[iPos]=MOT[MAuto1Y+iPos].MotorMove(Prod.iAutoCassetteFront[iPos]);
            if(bYCWMove[iPos]==false)
                break;
            bYCWMove[iPos]=false;
//            bZPushMove[iPos]=false;
            Task=120;
            break;
        case 120:
            bYCWMove[iPos]=MOT[MAuto1Y+iPos].MotorMove(Prod.iAutoCassetteFrontBack[iPos]);
            if(bYCWMove[iPos]==true &&
                Sen[SnAutoBoatActDetect[iPos]].IsOn()==false)
            {
                bYCWMove[iPos]=false;
                DoPlaceTrayToAutoDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);

//                iLayerPos=Prod.iAutoCassetteZStart[iPos]+                     //入空tray的時候不要動料盒，等出料再動
//                      (TestIF_File.iCassetteAutoNowLayer[iPos]*UserDefForm[0].dCassetteZPitch);
                Task=600;
            }
            break;
        case 210:
//            bZPushMove[iPos]=MOT[iTrayZMotor[iPos+3]].MotorMove(iLayerPos);
//            if(bZPushMove[iPos])
//            {
//                bZPushMove[iPos]=false;
//                Task=600;
//            }
            break;
        case 300:
//            TrayMoveIn(true, iPos+3);
//            bYPushMove[iPos]=MOT[Auto推TrayToCassetteFront].MotorMove(到底的位置);
//
//            if(bYPushMove[iPos])
//            {
//                bYPushMove[iPos]=false;
//                Task=400;
//            }
            break;
        case 400:
//            bYPushMove[iPos]=MOT[Auto推TrayToCassetteFront].MotorMove(到底的位置Back);
//
//            if(bYPushMove[iPos])
//            {
//                bYPushMove[iPos]=false;
//                Task=500;
//            }
//            break;
        case 500:
//            if(Sen[SnAutoTrayDetect[iPos]].IsOff()==false)
//            {
//                Cylinder[C_AutoEdgePush[iPos]].On();
//                Cylinder[C_AutoSide_Fixer[iPos]].On();
//                Task=600;
//            }
//            break;
        case 600:
            //檢查是否推到auto區
            if(DoPlaceTrayToAutoDelay.Off() ||
                (LastSet.iRealDummy!=DUMMY &&
                Sen[SnAutoTrayDetect[iPos]].IsOff()==false))
            {
                TrayMoveIn(false, iPos+3);
//                MOT[iMMAuto[iPos]].SetTray(NULL_IC, __FUNC__);
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
bool InArmZInSafe()
{
    int iMotNo;
    for(int i=0; i<InArmSuck.iMotRow; i++)
    {
        for(int j=0; j<InArmSuck.iMotCol; j++)
        {
            iMotNo=(USE_PICKER_COUNT==ep16Picker && InOutArmPickerUseMotor==eptUseMotCyn)?MInArmZA:InArmSuck.Suck[i][j].iMotNo;
            if(MOT[iMotNo].ReadPos()!=ZSafePos)
                return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
bool InArmXYZSafe()                                                             //Steven 20250311 : 確認Tray能移動
{
    bool bRet=false;
    int iPosY;

    if(TRAY_ARM_MODE==eUnderCoveyor)
    {
        bRet=true;
    }
    else if(IsMoveInArm2XYToWait())                                             //JerryYang 20171220 修改in arm安全位置判斷,避免hang up
    {
        bRet=true;
    }
    else
    {
        iPosY=MOT[MInArmY].ReadPos();
        if(iPosY>=Prod.YInArm_Shuttle1_Place[iInArmYBase][iInArmXBase]-300 &&   //Steven 20241130 : 修改in arm安全位置判斷
           InArmZInSafe())
        {
            bRet=true;
        }
    }

    return bRet;
}
//------------------------------------------------------------------------------
bool GetTraySeperateCynState(int iWhichTray, bool IsAuto, bool OnOff)           //RogerYang 20250722 Add for 9046AU
{
    bool rlt=false;
    if(IsAuto)
    {
        int iC_SepaRL=iC_SeparateRL[iWhichTray];                                //Ztex 2023.12.13 Add Double Belt Mode
        int iC_SepaFL=iC_SeparateFL[iWhichTray];                                //Ztex 2023.12.13 Add Double Belt Mode
        int iC_SepaRR=iC_SeparateRR[iWhichTray];                                //Ztex 2023.12.13 Add Double Belt Mode
        int iC_SepaFR=iC_SeparateFR[iWhichTray];                                //Ztex 2023.12.13 Add Double Belt Mode
        if(USE_OUT_SORT_ARM!=eartUninstall)
        {
            if(OnOff==true)
            {
                if(Cylinder[iC_SepaFL].OnSensor() &&
                   Cylinder[iC_SepaFR].OnSensor())
                {
                    rlt=true;
                }
            }
            else
            {
                if(Cylinder[iC_SepaFL].OffSensor() &&
                   Cylinder[iC_SepaFR].OffSensor())
                {
                    rlt=true;
                }
            }
        }
        else
        {
            if(OnOff==true)
            {
                if(Cylinder[iC_SepaRL].OnSensor() && Cylinder[iC_SepaFL].OnSensor() &&                                  //Ztex 2023.12.13 Add Double Belt Mode
                   Cylinder[iC_SepaRR].OnSensor() && Cylinder[iC_SepaFR].OnSensor())
                {
                    rlt=true;
                }
            }
            else
            {
                if(Cylinder[iC_SepaRL].OffSensor() && Cylinder[iC_SepaFL].OffSensor() &&                                //Ztex 2023.12.13 Add Double Belt Mode
                   Cylinder[iC_SepaRR].OffSensor() && Cylinder[iC_SepaFR].OffSensor())
                {
                    rlt=true;
                }
            }
        }
    }
    else                                                                        //Loader Empty Color Loader2
    {
        int SeparateRL[4]={C_LoaderSeparateRL,C_EmptySeparateRL,C_ColorSeparateRL,C_Load2SeparateRL};                   //Ztex 2023.12.13 Add Double Belt Mode
        int SeparateFL[4]={C_LoaderSeparateFL,C_EmptySeparateFL,C_ColorSeparateFL,C_Load2SeparateFL};                   //Ztex 2023.12.13 Add Double Belt Mode
        int SeparateRR[4]={C_LoaderSeparateRR,C_EmptySeparateRR,C_ColorSeparateRR,C_Load2SeparateRR};                   //Ztex 2023.12.13 Add Double Belt Mode
        int SeparateFR[4]={C_LoaderSeparateFR,C_EmptySeparateFR,C_ColorSeparateFR,C_Load2SeparateFR};                   //Ztex 2023.12.13 Add Double Belt Mode
        if(USE_OUT_SORT_ARM!=eartUninstall)
        {
            if(OnOff==true)
            {
                if(Cylinder[SeparateFL[iWhichTray]].OnSensor()&&
                    Cylinder[SeparateFR[iWhichTray]].OnSensor())
                {
                    rlt=true;
                }
            }
            else
            {
                if(Cylinder[SeparateFL[iWhichTray]].OffSensor()&&
                    Cylinder[SeparateFR[iWhichTray]].OffSensor())
                {
                    rlt=true;
                }
            }
        }
        else
        {
            if(OnOff==true)
            {
                if(Cylinder[SeparateRL[iWhichTray]].OnSensor() && Cylinder[SeparateFL[iWhichTray]].OnSensor() &&
                    Cylinder[SeparateRR[iWhichTray]].OnSensor() && Cylinder[SeparateFR[iWhichTray]].OnSensor())
                {
                    rlt=true;
                }
            }
            else
            {
                if(Cylinder[SeparateRL[iWhichTray]].OffSensor() && Cylinder[SeparateFL[iWhichTray]].OffSensor() &&
                    Cylinder[SeparateRR[iWhichTray]].OffSensor() && Cylinder[SeparateFR[iWhichTray]].OffSensor())
                {
                    rlt=true;
                }
            }
        }
    }
    return rlt;
}
//------------------------------------------------------------------------------
HTimer DoMagazineGetTrayTime;                                                   //Eastsun 20260515 F014 整合:KYEC AMR Magazine Get New Tray timer (file-scope HTimer)
int WhichAutoNeedTray()
{
    int iAuto;
    iWhichAutoNeedTray=0;                                                       //Steven 20211230 : 紀錄WhichAutoNeedTray變化
    AnsiString asString;
    static bool bMagazineGetTray=false;                                         //Eastsun 20260515 F014 整合:KYEC AMR Magazine Get New Tray timer (local static)

    if(iTrayFeed==1 && HasICUnderMachine()==false)                              //process Tray Feed ,don't supply new tray
    {
        iWhichAutoNeedTray=0;
        if(fAGV->IsATK_AMR()==true)                                             //Steven 20260202 : for ATK AMR
        {
            if(LastSet.iUnloadFixTray==eAtkTfPutIDTray)
            {
                for(int i=eAuto1; i<=iAutoRight; i++)
                {
                    if(bNeed1DCoverTray[i])                                     //Need 1D Tray
                    {
                        bAutoNeedTray[i]=true;
                        iWhichAutoNeedTray=i+1;
                        return iWhichAutoNeedTray;
                    }
                }
            }
            else if(LastSet.iUnloadFixTray==eAtkTfPutCover)
            {
                for(int i=eAuto1; i<=iAutoRight; i++)
                {
                    if(bNeedCoverTray[i])                                       //Need Cover Tray
                    {
                        bAutoNeedTray[i]=true;
                        iWhichAutoNeedTray=i+1;
                        return iWhichAutoNeedTray;
                    }
                }
            }
            else if(LastSet.iUnloadFixTray==eAtkTfPutEmptyTray)
            {
                for(int i=eAuto1; i<=iAutoRight; i++)
                {
                    if(MOT[iMMAuto[i]].fHasTray==false &&
                       bAutoNeedTray[i]==true)
                    {
                        iWhichAutoNeedTray=i+1;
                        return iWhichAutoNeedTray;
                    }
                    else
                    {
                    }
                }
            }
        }
    }
    else if(iCleanOut==1 && HasICUnderMachine()==false)                         //process clean out ,don't supply new tray
    {
        iWhichAutoNeedTray=0;
    }
    else
    {
        for(int i=eAuto1; i<=iAutoRight; i++)
        {
            iAuto=iAutoIndex[i];
            bAutoNeedTray[iAuto]=false;
        }

        for(int i=0; i<iTestBinCount; i++)                                      //Steven 20121112 : RS232支援32Bin 15 --> iTestBinCount
        {
            if(Prod.iT6CatData[i]>=eAuto1 && Prod.iT6CatData[i]<=iAutoRight)    //jou 2011-08-25 Prod.iT6CatData[i]=-1 over array
            {
                bAutoNeedTray[Prod.iT6CatData[i]]=true;                         //AI(ht9045-catchtray-flow) 20260424 (RogerYang) : fix merge regression, restore bAutoNeedTray for non-AOI machines
            }

            if(Prod.AOICatData[i]>=0 && Prod.AOICatData[i]<=2)                  //Eastsun 20260318 AOI Bin
            {
                bAutoNeedTray[Prod.AOICatData[i]]=true;
            }
        }

        if(fAGV->Use_AMR())                                                     //Steven 20251216 : for AMR
        {
            for(int i=0; i<=iAutoRight; i++)
            {
                if(bNeed1DCoverTray[i])                                         //Need 1D Tray
                {
                    return  i+1;
                }
                else if(bNeedCoverTray[i])                                      //Need Cover Tray
                {
                    return  i+1;
                }

                if(bAutoNeedTray[i]==true)
                {
                    if(bHas1DCoverTray[i]==false)
                    {
                        bNeed1DCoverTray[i]=true;
                        return  i+1;
                    }
                    else if(bHasCoverTray[i]==false)
                    {
                        bNeedCoverTray[i]=true;
                    }
                }
            }
        }
        else if(LastSet.iTester==OFF_LINE && bPurgeOutAllDevice)                //JerryYang 20251020 : 渠梁半清機功能
        {
            for(int i=0; i<=iAutoRight; i++)
            {
                if(bAutoNeedTray[i] && bNeedCoverTray[i])                                      //Need Cover Tray
                {
                    return  i+1;
                }
            }
        }

        //==> Eastsun 20260515 F012 整合:KYEC AMR Magazine full-IC scan, return cover-tray request
        if(bMagGetNewTrayflag==false &&
           bChaneMagTrayflag==false &&
           TrayForm.bEnableAMR && AUTO3_IS_MAGAZINE==1)
        {
            for(int j=0; j<14-1; j++)                                          //Eastsun 20260119 : scan all magazine trays
            {
                if(!MOT[MMMagazineTary1+j].Tray.FullIC())                      //Eastsun 20260119 : skip non-full
                    continue;
                if(!BinSelect[iTestRunMode].bMagazineLink[j] &&                 //both this and next not linked => full
                   !BinSelect[iTestRunMode].bMagazineLink[j+1])
                {
                    bAMRFullICBin=true;
                    bNeedCoverTray[0]=true;
                    return 1;
                }
                else if(BinSelect[iTestRunMode].bMagazineLink[j] &&
                        !BinSelect[iTestRunMode].bMagazineLink[j+1])
                {
                    bAMRFullICBin=true;
                    bNeedCoverTray[0]=true;
                    return 1;
                }
            }

            if(MOT[MMMagazineTary1+13].Tray.FullIC())
            {
                bAMRFullICBin=true;
                bNeedCoverTray[0]=true;
                return 1;
            }
        }

        if(IniConfig.bAutoTrayLink==true)                                       //jou 2012-06-14 Auto Tray Link
        {
            if(bAutoNeedTray[0]==true &&                                        //Auto 2
               Prod.bLinkTo6Tray[1]==true)
            {
                bAutoNeedTray[1]=true;
                if((MOT[MMAuto1].fHasTray==false ||
                    MOT[MMAuto1].Tray.FullIC()) &&
                   MOT[MMAuto2].fHasTray==true &&
                   MOT[MMAuto2].Tray.FullIC()==false)                           //wait Auto 2 Full
                {
                    bAutoNeedTray[0]=false;
                }
            }

            if(bAutoNeedTray[0]==true &&
               Prod.bLinkTo6Tray[1]==true &&
               Prod.bLinkTo6Tray[2]==true)                                      //Auto 3
            {
                if((MOT[MMAuto1].fHasTray==false ||
                    MOT[MMAuto1].Tray.FullIC()) &&
                   MOT[MMAuto3].fHasTray==true &&
                   MOT[MMAuto3].Tray.FullIC()==false)                           //wait Auto 3 Full
                {
                    bAutoNeedTray[0]=false;
                }
            }

            if(bAutoNeedTray[1]==true &&
               Prod.bLinkTo6Tray[2]==true)                                      //Auto 3
            {
                bAutoNeedTray[2]=true;
                if((MOT[MMAuto2].fHasTray==false ||
                    MOT[MMAuto2].Tray.FullIC()) &&
                   MOT[MMAuto3].fHasTray==true &&
                   MOT[MMAuto3].Tray.FullIC()==false)                           //wait Auto 2 Full
                {
                    bAutoNeedTray[1]=false;
                }
            }
        }

        if(Prod.iIfErrorT6<=iAutoCnt-1)                                         //JerryYang 20230918 //jou 2010-01-07 start : R Bin Auto(Unloader) Need Place Tray
        {
            bAutoNeedTray[Prod.iIfErrorT6]=true;
        }

        if(IniConfig.bI37_EnableFIFOMode &&                                     //Steven 20160303 : FIFO Mode
           LastSet.iRunStartMode==rsmFIFOMode &&
           MOT[MMAuto2].fHasTray==false)
        {
            bAutoNeedTray[1]=true;
        }

        //==> Eastsun 20260515 F013 整合:KYEC Dummy Run position check (Auto3 Magazine fix)
        if(CUSTOMER_CODE==CC_KYEC_LEE && bDummyRunPosCheck)
        {
            if(bDummyRunPosCheckCatchTray[0]==false || bDummyRunPosCheckCatchTray[1]==false || bDummyRunPosCheckCatchTray[2]==false)
            {
                for(int i=0; i<3; i++)
                {
                    if(i==2 && AUTO3_IS_MAGAZINE==1)                              //Eastsun 20260325 : fix Dummy mode Auto3=Mag infinite catch
                    {
                        // Auto3 is Magazine: treat as passed
                        bDummyRunPosCheckCatchTray[i]=true;
                        continue;
                    }

                    if(MOT[MMAuto1+i].fHasTray==false)
                    {
                        bDummyRunPosCheckCatchTray[i]=false;
                        return i+1;
                    }
                    else
                    {
                        bDummyRunPosCheckCatchTray[i]=true;
                    }
                }
            }
            else if(bDummyRunPosCheckFinish==true && MOT[MMAuto1].fHasTray==false)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        //<== Eastsun 20260515 F013

        if(AUTO3_IS_MAGAZINE==1)                                                //Sam 20221116 : Magazine TrayArm 自動補 Tray
        {
            if(TestIF_File.iMagTraySource==0)
            {
                if(bMagGetNewTrayflag==false &&
                   ((MOT[MMAuto3].fHasTray==true &&
                     MOT[MMAuto3].Tray.FullIC()==true) ||
                    MOT[MMAuto3].fHasTray==false))                              //JerryYang 20250920 : fixed for Multi bin
                {
                    bAutoNeedTray[2]=true;
                }
            }
        }

        if(IniConfig.b18bUseAutoTrayMap)                                        //kevin 20220819 use auto empty tray
        {
            for(int i=eAuto1; i<=iAutoRight; i++)
            {
                iAuto=iAutoIndex[i];
                if(IniConfig.b18bDoubleUnloadTray &&
                   bAutoNeedTray[iAuto]==true)                                  //kevin 20220506 add Unload Tray 2  倍的設定
                {
                    if(iUnloaderTrayCountCal[iAuto]==(IniConfig.iUnloaderTrayCount[iAuto]+1) ||
                       iUnloaderTrayCountCal[iAuto]==(IniConfig.iUnloaderTrayCount[iAuto]*2+1))                         //kevin 20220506 add Unload Tray 2  倍的設定
                    {
                        if(bAutoNeedTray[iAuto]==true)
                        {
                            iWhichAutoNeedTray=i+1;
                            return iWhichAutoNeedTray;                          //kevin 20220924 change data
                        }
                    }
                    else
                    {
                        if(iUnloaderTrayCountCal[iAuto]==0)
                        {
                            iWhichAutoNeedTray=i+1;
                            return iWhichAutoNeedTray;                          //kevin 20220924 change data
                        }
                    }
                }
                else
                {
                    if(iUnloaderTrayCountCal[iAuto]==0 && bAutoNeedTray[iAuto]==true)
                    {
                        iWhichAutoNeedTray=i+1;
                        return iWhichAutoNeedTray;                              //kevin 20220924 change data
                    }
                }
            }
        }

        for(int i=eAuto1; i<=iAutoRight; i++)
        {
            iAuto=iAutoIndex[i];
            if(bAutoNeedTray[iAuto]==false)
            {
                //==> Eastsun 20260515 F014 整合:KYEC AMR reset Magazine timer flag
                if(CUSTOMER_CODE==CC_KYEC_LEE && i==2) bMagazineGetTray=false;
                //<== Eastsun 20260515 F014
                continue;
            }

            if(LastSet.iRealDummy!=DUMMY &&                                     //AI(ht9045-v899) 20260513: dummy run skip P18 manual Fail Auto Tray refill flow
              (IniConfig.bP18FailAutoTrayManual==true ||                        //jou 2012-03-16 Fail Auto Tray手動補Tray
               ((iRunStartMode==FT || iRunStartMode==FT_ART) && TrayForm.bFailAutoTrayManual_FT==true) ||
               ((iRunStartMode==RT || iRunStartMode==RT_ART) && TrayForm.bFailAutoTrayManual_RT==true)))                 //Steven 20150116 : 手動移除Auto Fail Bin Tray
            {
                if(Prod.iIsPassT6[i]==1)                                        //Steven 20240105 : Prod.bIsPass --> Prod.iIsPassT6
                {
                    if(MOT[iMMAuto[i]].fHasTray==false || MOT[iMMAuto[i]].Tray.FullIC())
                    {
                        iWhichAutoNeedTray=i+1;
                        return iWhichAutoNeedTray;                              //wei 20220412 回傳跳出迴圈
                    }
                }
                else
                {
                    if(MOT[iMMAuto[i]].fHasTray==false &&
                       Sen[SnAutoTrayDetect[iAuto]].IsOn()==false &&            //Steven 20150120 : 等Tray上升
                       MOT[iMMAuto_Car[i]].fHasTray==false)                     //JerryYang 20191111 避免軌道外側放tray時hang up
                    {
                        #ifndef SOFT_SIMULTE
                        ShowErrorMessage(sMES1121[iAuto], K_RETRY, iMMAuto[i], false, "WhichAutoNeedTray");             //There is no tray //MES1121, MES1221, MES1321
                        #endif
                        //Steven 20150924 : [SCC]避免OP按了SKIP又放下一盤, 所以多檢查一次
                        if(Prod.iIsFailT6[i]==1 &&                              //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
                           Sen[SnAutoTrayDetect[iAuto]].IsOff()==false)
                        {
                            if(DoLockUnloader(i)==false)                        //JerryYang 20191210 優化Auto定位方式
                            {
                                iWhichAutoNeedTray=0;
                                QueueTaskList[264].CheckTaskChange();           //Steven 20200821 : 使用Goto也要記錄Task變化
                                return iWhichAutoNeedTray;
                            }
                            MOT[iMMAuto[i]].SetTray(NULL_IC, __FUNC__);
                            bAutoEdgePush[iAuto]=true;                          //ChungHung 20120329 bFailAutoTrayManual
                            bOldAutoHasTray[iAuto]=false;
                        }
                    }
                    else
                    {
                        if(MOT[iMMAuto[i]].fHasTray==false &&
                           Sen[SnAutoTrayDetect[iAuto]].IsOff()==false &&
                           MOT[iMMAuto_Car[i]].fHasTray==false)                 //Steven 20150924 : [SCC]避免OP按了SKIP又放下一盤, 所以多檢查一次  //JerryYang 20191111 避免軌道外側放tray時hang up
                        {
                            if(Prod.iIsFailT6[i]==1 &&                          //Steven 20240105 : Prod.bIsPass --> Prod.iIsFailT6
                               Sen[SnAutoTrayDetect[iAuto]].IsOn()==true)
                            {
                                if(DoLockUnloader(i)==false)                    //JerryYang 20191210 優化Auto定位方式
                                {
                                    iWhichAutoNeedTray=0;
                                    QueueTaskList[264].CheckTaskChange();       //Steven 20200821 : 使用Goto也要記錄Task變化
                                    return iWhichAutoNeedTray;
                                }
                                MOT[iMMAuto[i]].SetTray(NULL_IC, __FUNC__);
                                bAutoEdgePush[iAuto]=true;                      //ChungHung 20120329 bFailAutoTrayManual
                                bOldAutoHasTray[iAuto]=false;
                            }
                        }
                    }
                }
            }
            else if(AUTO3_IS_MAGAZINE==1 && TestIF_File.iMagTraySource==0)      //Sam 20221116 : Magazine TrayArm 自動補 Tray
            {
                if(i==iMagAtAuto)                                               //Auto3
                {
                    if(iOutArmWhichAuto==iMagAtAuto &&
                       MOT[iMMAuto[iMagAtAuto]].fHasTray==false &&              //JerryYang 20250910 : fix magazine
                       iWhichMag!=-1 &&
                       iWhichMag!=iAuto3MagazineIndex &&
                       MOT[iMMgzTray[iWhichMag]].fHasTray==false &&             //JerryYang 20250920 : fixed for Multi bin
                       bMagGetNewTrayflag==false &&
                       bChaneMagTrayflag==false)
                    {
                        return 3;
                    }
                    //==> Eastsun 20260515 F014 整合:KYEC AMR Magazine Get New Tray timer + warn
                    if(TrayForm.bEnableAMR)
                    {
                        if(bResetMagGetTime==true)
                        {
                            bResetMagGetTime=false;
                            bMagazineGetTray=false;
                        }

                        if(OutArmSuck.HasIC()==false)         //Ifor 20240604 add: reset timer to avoid false alarm
                        {
                            bMagazineGetTray=false;
                        }
                        else if(bMagazineGetTray==false)
                        {
                            bMagazineGetTray=true;
                            DoMagazineGetTrayTime.SetSecAndOn(300);
                        }
                        else
                        {
                            if(DoMagazineGetTrayTime.Off())
                            {
                                if(iMagazineStatus>0)         //Ifor 20231130: 0:Normal 1:Full Tray 2:Manual put tray
                                {
                                    if(W906_FormShowing("MyMessageBox", MyMessageBox->fShow)==true)  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（C++ 對話框列＝成員 或 程式狀態；程式狀態只在阻塞等待框跟成員一起設，值不變）
                                    {
                                    }
                                    else
                                    {
                                        iUnLoaderCount=99;
                                        ShowUnloaderTrayMessage("Magazine tray out, please remove the tray in slot.", asMagazineBinFull);
                                    }
                                }
                                else
                                {
                                    ShowMyMessage("Magazine Get New Tray Time Out Error!");
                                }
                                asString.sprintf("iWhichMag :%d ,iAuto3MagazineIndex :%d, bMagazineGetNewTray :%d", iWhichMag, iAuto3MagazineIndex, bMagazineGetNewTray);
                                RecordProcess(asString);
                                bMagazineGetTray=false;
                            }
                        }
                    }
                    //<== Eastsun 20260515 F014
                            }
                else
                {
                    //==> Eastsun 20260515 F015 整合:KYEC AMR rail full-tray CoverTray request (Mag-else branch)
                    if(TrayForm.bEnableAMR && TrayForm.bEnableAMRLoader==false)
                    {
                        if(MOT[iMMAuto[i]].fHasTray==false || MOT[iMMAuto[i]].Tray.FullIC())
                        {
                            if(iUnloaderTrayCountCal[i]>=TrayForm.iFullTrayCount)
                            {
                                bNeedCoverTray[i]=true;
                            }
                            else if(MOT[MTrayX].fHasTray==true &&
                                    (iUnloaderTrayCountCal[i]==(TrayForm.iFullTrayCount-1)))
                            {
                                bNeedCoverTray[i]=true;
                            }

                            iWhichAutoNeedTray=i+1;
                            return iWhichAutoNeedTray;                              //wei 20220412
                        }
                    }
                    else if(MOT[iMMAuto[i]].fHasTray==false ||
                       MOT[iMMAuto[i]].Tray.FullIC())
                    {
                        return i+1;
                    }
                    //<== Eastsun 20260515 F015
                    }
            }
            else
            {
                //==> Eastsun 20260515 F015 整合:KYEC AMR rail full-tray CoverTray request (final-else)
                if(TrayForm.bEnableAMR && TrayForm.bEnableAMRLoader==false)
                {
                    if(MOT[iMMAuto[i]].fHasTray==false || MOT[iMMAuto[i]].Tray.FullIC())
                    {
                        if(iUnloaderTrayCountCal[i]>=TrayForm.iFullTrayCount)
                        {
                            bNeedCoverTray[i]=true;
                        }
                        else if(MOT[MTrayX].fHasTray==true &&
                                (iUnloaderTrayCountCal[i]==(TrayForm.iFullTrayCount-1)))
                        {
                            bNeedCoverTray[i]=true;
                        }

                        iWhichAutoNeedTray=i+1;
                        return iWhichAutoNeedTray;                              //wei 20220412
                    }
                }
                //<== Eastsun 20260515 F015
                else if(MOT[iMMAuto[i]].fHasTray==false ||
                   MOT[iMMAuto[i]].Tray.FullIC())
                {
                    if(MOT[iMMAuto[i]].Tray.FullIC() && bAUTORev[iAuto]==false)
                    {
                        return 0;
                    }
                    iWhichAutoNeedTray=i+1;
                    return iWhichAutoNeedTray;                                  //wei 20220412 回傳跳出迴圈
                }
            }
        }
    }
    QueueTaskList[264].CheckTaskChange();                                       //Steven 20200821 : 使用Goto也要記錄Task變化
    return iWhichAutoNeedTray;
}
//==============================================================================
// 從 LoadStage吸空TRAY流程
//==============================================================================
int iCatchUnderTrayTask=1;
//------------------------------------------------------------------------------
void InitialCatchUnderTray()
{
    iCatchUnderTrayTask=1;
}
//------------------------------------------------------------------------------
TQPF_Timer htCatchUnderTrayTimeDelay;                                           //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
bool DoCatchUnderTray(int iWhichAuto)                                           //夾盤流程
{
    int &Task=iCatchUnderTrayTask;
    static bool bXMove=false;
    static bool bZMove=false;

    AnsiString str;
    AnsiString asPort[3]={"Loader",                 "Empty",                    "Color"};
    int iMOT[3]         ={MMTrayY,                  MMEmpty,                    MMColor};
    int iFixer[3]       ={C_TrayY_Fixer,            C_Empty_Fix,                C_Color_Fix};
    int iSureHasTray[3] ={SnLoaderSureTray,         SenEmptyHasTray,            SenColorHasTray};
    int iSeparate[3]    ={SnLoaderSeparateHasTray,  SnEmptySeparateHasTray,     SnColorSeparateHasTray};
    int iCWDete[3]      ={SenLoaderCCWDete_2,       SenEmptyCCWDete_2,          SenColorCCWDete_2};
    int iTrayXPos[3]    ={Prod.iXTrayLoad,          Prod.iXTrayEmpty,           Prod.iXTrayColor};

//    int iC_SeparateRL[3]={C_LoaderSeparateRL,C_EmptySeparateRL,C_ColorSeparateRL};//Ztex 2023.12.13 Add Double Belt Mode
//    int iC_SeparateFL[3]={C_LoaderSeparateFL,C_EmptySeparateFL,C_ColorSeparateFL};//Ztex 2023.12.13 Add Double Belt Mode
//    int iC_SeparateRR[3]={C_LoaderSeparateRR,C_EmptySeparateRR,C_ColorSeparateRR};//Ztex 2023.12.13 Add Double Belt Mode
//    int iC_SeparateFR[3]={C_LoaderSeparateFR,C_EmptySeparateFR,C_ColorSeparateFR};//Ztex 2023.12.13 Add Double Belt Mode
    bool bSOFT_SIMULTE=false;                                                   //Ztex 2023.07.28 Add bSOFT_SIMULTE
    bool bSeparateHasTray = false;                                              //Ztex 2023.12.13 Add Double Belt Mode
    bool bCheckSnTrayDetect=false;                                              //Ztex 2025.04.01 Add bCheckSnTrayDetect
    #ifdef SOFT_SIMULTE
        bSOFT_SIMULTE=true;
    #endif

    if(DOUBLE_BELT_MODE==1)                                                     //Ztex 2023.12.13 Add Double Belt Mode
        bSeparateHasTray=true;
    else
        bSeparateHasTray=Sen[iSeparate[iWhichAuto]].IsOn() && Sen[iCWDete[iWhichAuto]].IsOff();

    switch(Task)
    {
        case 1:                                                                 //Y推&後牙刷 開  Z托盤 開
            if(LastSet.iRealDummy==DUMMY ||
               bSOFT_SIMULTE==true ||                                           //Ztex 2023.07.28 Add bSOFT_SIMULTE
              (bSeparateHasTray &&                                              //Ztex 2023.12.13 Add Double Belt Mode
               Sen[iSureHasTray[iWhichAuto]].IsOn() &&
               MOT[iMOT[iWhichAuto]].fHasTray))
            {
                if(iWhichAuto!=0)
                {
                    Cylinder[C_LoaderPushBack_Push+iWhichAuto].Off();
                    Cylinder[iFixer[iWhichAuto]].Off();
                }
                else
                {
                    Cylinder[C_TrayY_Fixer].Off();
                    Cylinder[C_LoaderEdgePush].Off();
                }

                bZMove=false;
                bXMove=false;
                Task=50;
            }
            else
            {
                str.sprintf("Please Check %s Port Tray Detect", asPort[iWhichAuto]);
                ShowMyMessage(str);
            }
            break;
        case 50:
            bXMove=TrayArmMotorMove(iTrayXPos[iWhichAuto]);
            if(bXMove)
            {
                if(LastSet.iRealDummy!=DUMMY && Sen[SnTrayArmHasTray].IsOn())
                {
                    ShowMyMessage("Please Check Tray Arm Port Tray Detect");
                }
                else
                {
                    Task=100;
                    bXMove=false;
                    htCatchUnderTrayTimeDelay.SetSecAndOn(5);                   //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
                }
            }
            break;
        case 100:                                                               //Z 上托
            bZMove=MOT[MTrayZ].MotorMove(Prod.iTrayArmZPnP[iWhichAuto]);
            bCheckSnTrayDetect=(Sen[iSureHasTray[iWhichAuto]].IsOn() ||         //Ztex 2025.04.01 Add bCheckSnTrayDetect
                               LastSet.iRealDummy==DUMMY);

            if(bZMove && bCheckSnTrayDetect)
            {
                if(LastSet.iRealDummy!=DUMMY && Sen[SnTrayArmHasTray].IsOff())
                {
                    ShowMyMessage("Please Check Tray Arm Port Tray Disappear");
                }
                else
                {
                    Task=200;
                    bZMove=false;
                    break;
                }
            }

            if(htCatchUnderTrayTimeDelay.Off())                                 //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
            {
                str.sprintf("Please Check %s Port Tray Detect Need On",asPort[iWhichAuto]);
                ShowMyMessage(str);
                htCatchUnderTrayTimeDelay.SetSecAndOn(5);                       //Ztex 2025.04.01 Add bCheckSnTrayDetect
            }
            break;
        case 200:                                                               //汽缸開
            if(bSOFT_SIMULTE ||
               Cylinder[C_LoaderSeparate+iWhichAuto].Push())
            {
                if(DOUBLE_BELT_MODE==1 && bSOFT_SIMULTE==false)                 //Ztex 2023.12.13 Add Double Belt Mode
                {
                    htCatchUnderTrayTimeDelay.SetSecAndOn(10);                  //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
                    Task=300;
                }
                else
                {
                    Task=400;
                }
            }

            break;
        case 300:                                                               //Ztex 2023.12.13 Add Double Belt Mode
            if(bHandlerPause)                                                   //Ztex 2024.03.02 Add Handler Pause Must Recalculate Time
            {
                Task=200;
                break;
            }

            if(GetTraySeperateCynState(iWhichAuto, false, true))                //rogerYang 20250722 add for 9046AU
            {
                Task=400;
                break;
            }

            if(htCatchUnderTrayTimeDelay.Off())                                 //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Ms
            {
                str.sprintf("Please Check %s Separate Tray Detect , Need Open",asPort[iWhichAuto]);
                ShowMyMessage(str);
                htCatchUnderTrayTimeDelay.SetSecAndOn(10);
            }
            break;
        case 400:                                                               //Z 托下
            bZMove=MOT[MTrayZ].MotorMove(0);

            if(bZMove)
            {
                bZMove=false;
                if(LastSet.iRealDummy!=DUMMY && Sen[SnTrayArmHasTray].IsOff())
                {
                    ShowMyMessage("Please Check Tray Arm Port Tray Detect");
                }
                else
                {
                    if(MOT[MTrayZ].Led[iHomeLed]==false)
                    {
                        InitProcessSingleMotorTask(MTrayZ);
                        Task=600;
                        break;
                    }

                    Task=500;
                }
            }
            break;
        case 500:                                                               //分離 夾持
            if(bXMove==false)
            {
                if(iWhichAuto==0)
                    CatchFromLoaderSetItemData();
                bXMove=true;
            }

            if(bXMove)
            {
                bXMove=false;
                Task=700;
            }
            break;
        case 600:
            bZMove=ProcessSingleMotorHome(MTrayZ);
            if(bZMove)
            {
                bZMove=false;
                if(MOT[MTrayZ].Led[iHomeLed])
                {
                    Task=500;
                }
                else
                {
                    InitProcessSingleMotorTask(MTrayZ);
                }
            }
            break;
        case 700:
            #ifdef SOFT_SIMULTE
                return true;
            #else
            if(Cylinder[C_LoaderSeparate+iWhichAuto].Pop())
                return true;
            #endif
    }

    return false;
}
//------------------------------------------------------------------------------
int iSupportUnderTrayTask=1;

//------------------------------------------------------------------------------
void InitialSupportUnderTray()
{
    iSupportUnderTrayTask=1;
}
//------------------------------------------------------------------------------
TQPF_Timer htSupportUnderTrayTimeDelay;                                         //Ztex 2025.04.01 Add bCheckSnTrayDetect
bool DoSupportUnderTray(int iWhichAuto)                                         //補盤流程
{
    int &Task=iSupportUnderTrayTask;
    static bool bZMove=false;

    int iFixer=C_AutoSide_Fixer[iWhichAuto];                                    //JerryYang 20240405 : Fixed for HT9011UC退盤溢位
    int iSureHasTray=SnAutoTrayDetect[iWhichAuto];
    int iSeperate_Cyn=C_AutoSeparate[iWhichAuto];                               //RogerYang 20250602 Add for 9046AU

    int iSeparate=SnAutoSeparate[iWhichAuto];
    int iC_SepaRL=iC_SeparateRL[iWhichAuto];                                    //Ztex 2023.12.13 Add Double Belt Mode
    int iC_SepaFL=iC_SeparateFL[iWhichAuto];                                    //Ztex 2023.12.13 Add Double Belt Mode
    int iC_SepaRR=iC_SeparateRR[iWhichAuto];                                    //Ztex 2023.12.13 Add Double Belt Mode
    int iC_SepaFR=iC_SeparateFR[iWhichAuto];                                    //Ztex 2023.12.13 Add Double Belt Mode
    bool bSOFT_SIMULTE=false;
    bool bSeparateHasTray=false;                                                //Ztex 2023.12.13 Add Double Belt Mode
    AnsiString str;                                                             //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
    AnsiString asPort[3]={"Auto 1","Auto 2","Auto 3"};                          //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
    bool bCheckSnTrayDetect=false;                                              //Ztex 2025.04.01 Add bCheckSnTrayDetect
    #ifdef SOFT_SIMULTE                                                         //Ztex Add SOFT_SIMULTE Pass
        bSOFT_SIMULTE=true;
    #endif

    if(DOUBLE_BELT_MODE==1)                                                     //Ztex 2023.12.13 Add Double Belt Mode
        bSeparateHasTray=true;
    else
        bSeparateHasTray=Sen[iSeparate].IsOff();

    switch(Task)
    {
        case 1:                                                                 //Y推&後牙刷 開 分離 開
            if(bSOFT_SIMULTE)                                                   //Ztex Add SOFT_SIMULTE Pass
            {
                htSupportUnderTrayTimeDelay.SetSecAndOn(5);                     //Ztex 2025.04.01 Add bCheckSnTrayDetect
                Task=110;
                break;
            }
            Cylinder[C_Auto1PushBack_Push+iWhichAuto].Off();
            Cylinder[C_Auto1PushBack_Back+iWhichAuto].On();
            Cylinder[iFixer].Off();
            if(LastSet.iRealDummy==DUMMY ||
              (bSeparateHasTray && Sen[iSureHasTray].IsOff()))                  //Ztex 2023.12.13 Add Double Belt Mode
            {
                if(Cylinder[iSeperate_Cyn].Push())                              //RogerYang 2025060 修正Auto4~6開合氣缸異常
                {
                    bZMove=false;
                    htSupportUnderTrayTimeDelay.SetSecAndOn(10);                //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
                    Task=50;
                }
            }
            break;
        case 50:
            if(Cylinder[C_Auto1PushBack_Push+iWhichAuto].OffSensor())
            {
                Task=100;
            }
            break;
        case 100:
            if(bHandlerPause)                                                   //Ztex 2024.03.02 Add Handler Pause Must Recalculate Time
            {
                Task=1;
                break;
            }

            if(Cylinder[iC_SepaRL].OnSensor() && Cylinder[iC_SepaFL].OnSensor() &&
               Cylinder[iC_SepaRR].OnSensor() && Cylinder[iC_SepaFR].OnSensor())
            {
                htSupportUnderTrayTimeDelay.SetSecAndOn(5);                     //Ztex 2025.04.01 Add bCheckSnTrayDetect
                Task=110;
            }

            if(htSupportUnderTrayTimeDelay.Off())                               //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Ms
            {
                str.sprintf("Please Check %s Separate Tray Detect , Need Open",asPort[iWhichAuto]);
                ShowMyMessage(str);
            }
            break;
        case 110:                                                               //Z 上托
            bZMove=MOT[MTrayZ].MotorMove(Prod.iTrayArmZPnP[3+iWhichAuto]);
            bCheckSnTrayDetect=(Sen[SnAutoTrayDetect[iWhichAuto]].IsOn() ||     //Ztex 2025.04.01 Add bCheckSnTrayDetect
                               LastSet.iRealDummy==DUMMY);
            if(bZMove && bCheckSnTrayDetect)
            {
                Task=200;
                bZMove=false;
                break;
            }

            if(htSupportUnderTrayTimeDelay.Off())                               //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
            {
                str.sprintf("Please Check %s Port Tray Detect Need On",asPort[iWhichAuto]);
                ShowMyMessage(str);
                htSupportUnderTrayTimeDelay.SetSecAndOn(5);                     //Ztex 2025.04.01 Add bCheckSnTrayDetect
            }
            break;
        case 200:                                                               //分離關
            if(bSOFT_SIMULTE ||
               Cylinder[iSeperate_Cyn].Pop())                                   //RogerYang 2025060 修正Auto4~6開合氣缸異常
            {
                if(DOUBLE_BELT_MODE==1 && bSOFT_SIMULTE==false)                 //Ztex 2023.12.13 Add Double Belt Mode
                {
                    htSupportUnderTrayTimeDelay.SetSecAndOn(10);                //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Msg
                    Task=300;
                }
                else
                {
                    Task=400;
                }
            }
            break;
        case 300:                                                               //Ztex 2023.12.13 Add Double Belt Mode
            if(bHandlerPause)                                                   //Ztex 2024.03.02 Add Handler Pause Must Recalculate Time
            {
                Task=200;
                break;
            }

            if(GetTraySeperateCynState(iWhichAuto, true, false))                //rogerYang 20250722 add for 9046AU
            {
                Task=400;
            }

            if(htSupportUnderTrayTimeDelay.Off())                               //Ztex 2024.01.26 Check Auto Separate Sensor & Show Alarm Ms
            {
                str.sprintf("Please Check %s Separate Tray Detect , Need Close",asPort[iWhichAuto]);
                ShowMyMessage(str);
            }
            break;
        case 400:                                                               //Z 托下
            bZMove=MOT[MTrayZ].MotorMove(0);

            if(Sen[SnTrayArmZSafePos].Enable && Sen[SnTrayArmZSafePos].IsOn())
            {
                if(bSeparateHasTray || Sen[iSureHasTray].IsOff())
                {
                                                                                //Tray盤擺放異常
                }
            }

            if(bZMove)
            {
                bZMove=false;
                if(LastSet.iRealDummy!=DUMMY && Sen[SnTrayArmHasTray].IsOn())
                {
                    ShowMyMessage("Please Check Tray Arm Port Tray Detect");
                }
                else
                {
                    if(MOT[MTrayZ].Led[iHomeLed]==false)
                    {
                        InitProcessSingleMotorTask(MTrayZ);
                        Task=600;
                        break;
                    }

                    return true;
                }
            }
            break;
        case 600:
            bZMove=ProcessSingleMotorHome(MTrayZ);
            if(bZMove)
            {
                bZMove=false;
                if(MOT[MTrayZ].Led[iHomeLed])
                {
                    return true;
                }
                else
                {
                    InitProcessSingleMotorTask(MTrayZ);
                }
            }
            break;
    }

    return false;
}
//------------------------------------------------------------------------------
int iPlaceBufferTrayTask=1;
//------------------------------------------------------------------------------
void InitialPlaceBufferTray()
{
    iPlaceBufferTrayTask=1;
}
//------------------------------------------------------------------------------
bool DoPlaceBufferTray(int iWhichAuto)                                          //補盤流程
{
    int &Task=iPlaceBufferTrayTask;
    static bool bZMove=false;

    AnsiString str;
    AnsiString asPort[2]={"Empty", "Color"};

    AnsiString asCode[2]={"JAM1011", "JAM1411"};
    int iMap[2]={MMEmpty, MMColor};

    int iFixer[2]={C_Empty_Fix, C_Color_Fix};
    int iSureHasTray[2]={SenEmptyHasTray, SenColorHasTray};
    int iSeparate[2]={SnEmptySeparateHasTray, SnColorSeparateHasTray};
    int iC_SeparateRL[2]={C_EmptySeparateRL, C_ColorSeparateRL};                //Ztex 2023.12.13 Add Double Belt Mode
    int iC_SeparateFL[2]={C_EmptySeparateFL, C_ColorSeparateFL};                //Ztex 2023.12.13 Add Double Belt Mode
    int iC_SeparateRR[2]={C_EmptySeparateRR, C_ColorSeparateRR};                //Ztex 2023.12.13 Add Double Belt Mode
    int iC_SeparateFR[2]={C_EmptySeparateFR, C_ColorSeparateFR};                //Ztex 2023.12.13 Add Double Belt Mode
    bool bSOFT_SIMULTE=false;
    int ret;
    bool bSeparateHasTray = false;                                              //Ztex 2023.12.13 Add Double Belt Mode
    #ifdef SOFT_SIMULTE                                                         //Ztex Add SOFT_SIMULTE Pass
        bSOFT_SIMULTE=true;
    #endif

    if(DOUBLE_BELT_MODE==1)                                                     //Ztex 2023.12.13 Add Double Belt Mode
        bSeparateHasTray = true;
    else
        bSeparateHasTray = Sen[iSeparate[iWhichAuto]].IsOff();

    switch(Task)
    {
        case 1:                                                                 //Y推&後牙刷 開 分離 開
            Cylinder[C_EmptyPushBack_Back+iWhichAuto].Off();
            Cylinder[C_EmptyPushBack_Push+iWhichAuto].Off();
            Cylinder[iFixer[iWhichAuto]].Off();
            if(bSOFT_SIMULTE ||
               LastSet.iRealDummy==DUMMY ||
               (bSeparateHasTray && Sen[iSureHasTray[iWhichAuto]].IsOff()))     //Ztex 2023.12.13 Add Double Belt Mode
            {
                if(Cylinder[C_EmptySeparate+iWhichAuto].Push())
                {
                    bZMove=false;
                    if(DOUBLE_BELT_MODE==1 && bSOFT_SIMULTE==false)             //Ztex 2023.12.13 Add Double Belt Mode
                        Task=50;
                    else
                        Task=100;
                }
            }
            else
            {
                str.sprintf("Please Check %s Port Tray Detect", asPort[iWhichAuto]);
                ShowMyMessage(str);
            }
            break;
        case 50:                                                                //Ztex 2023.12.13 Add Double Belt Mode
            if(Cylinder[iC_SeparateRL[iWhichAuto]].OnSensor() && Cylinder[iC_SeparateFL[iWhichAuto]].OnSensor() &&
               Cylinder[iC_SeparateRR[iWhichAuto]].OnSensor() && Cylinder[iC_SeparateFR[iWhichAuto]].OnSensor())
            {
                Task=100;
            }
            break;
        case 100:                                                               //Z 上托
            bZMove=MOT[MTrayZ].MotorMove(Prod.iTrayArmZPnP[1+iWhichAuto]);
            if(bZMove)
            {
                if(LastSet.iRealDummy!=DUMMY && Sen[SnTrayArmHasTray].IsOff())
                {
                    ShowMyMessage("Please Check Tray Arm Port Tray Detect");
                }
                else
                {
                    Task=200;
                    bZMove=false;
                }
            }
            break;
        case 200:                                                               //分離關
            if(bSOFT_SIMULTE ||
               Cylinder[C_EmptySeparate+iWhichAuto].Pop())
            {
               if(DOUBLE_BELT_MODE==1 && bSOFT_SIMULTE==false)                  //Ztex 2023.12.13 Add Double Belt Mode
                   Task=300;
               else
                   Task=400;
            }
            break;
        case 300:                                                               //Ztex 2023.12.13 Add Double Belt Mode
            if(Cylinder[iC_SeparateRL[iWhichAuto]].OffSensor() && Cylinder[iC_SeparateFL[iWhichAuto]].OffSensor() &&
               Cylinder[iC_SeparateRR[iWhichAuto]].OffSensor() && Cylinder[iC_SeparateFR[iWhichAuto]].OffSensor())
            {
                Task=400;
            }
            break;
        case 400:                                                               //Z 托下
            bZMove=MOT[MTrayZ].MotorMove(0);

            if(bZMove)
            {
                bZMove=false;
                if(Sen[SnTrayArmHasTray].IsOn() && LastSet.iRealDummy!=DUMMY)   //Ztex 2023.09.20 Check TrayArmHasTray Mode RealDummy
                {
                    ShowMyMessage("Please Check Tray Arm Port Has Tray");
                }
                else
                {
                    if(MOT[MTrayZ].Led[iHomeLed]==false)
                    {
                        InitProcessSingleMotorTask(MTrayZ);
                        Task=600;
                        break;
                    }
                    Task=1000;
                }
            }
            break;
        case 600:
            bZMove=ProcessSingleMotorHome(MTrayZ);
            if(bZMove)
            {
                bZMove=false;
                if(MOT[MTrayZ].Led[iHomeLed])
                {
                    Task=1000;
                }
                else
                {
                    InitProcessSingleMotorTask(MTrayZ);
                }
            }
            break;
        case 1000:
            if(LastSet.iRealDummy!=DUMMY && Sen[iSureHasTray[iWhichAuto]].IsOff())
            {
                ret=ShowErrorMessage(asCode[iWhichAuto], K_RETRY, iMap[iWhichAuto], false, __FUNC__);
                if(ret==K_RETRY)
                    break;
            }
            else
            {
                return true;
            }
    }

    return false;
}
//------------------------------------------------------------------------------
int iCatchFromLoaderTask=1;
void InitialCatchFromLoader()
{
    iCatchFromLoaderTask=1;
    if(USE_TRAY_MAPPING==etmInstall)
        fTrayMapping->WritePickMapLog();                                        //wei 20170119 (Steven) Tray Mapping紀錄是否有吸取IC
}
//------------------------------------------------------------------------------
bool MTrayXCanSafeMove()
{
    if(TRAY_ARM_MODE==eAboveCoveyor &&
       Cylinder[C_TrayX_UpDown].OffSensor() &&
       Cylinder[C_TrayX_UpDown].GetOutBit()==false)
    {
        if(bRunOcrInsp ||                                                       //KenHsieh 20250718 : OCR 讀取時，C_OCRLight_Up 不偵測
          (Cylinder[C_OCRLight_Up].OnSensor() &&                                //JerryYang 20241118 : 增加保護OCR汽缸要在上才能移動
           Cylinder[C_OCRLight_Up].GetOutBit()==false))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if(TRAY_ARM_MODE==eUnderCoveyor &&
            MOT[MTrayZ].Led[iHomeLed])
    {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer DoCatchFromLoaderDelay;
int DoCatchFromLoader()
{
    int ret, pos;
    int &Task=iCatchFromLoaderTask;
    static bool flag[2];
    static int SuckLoaderErrRetryCT=0;
    AnsiString sBuffer="";                                                      //kevin 20210623
    static int iTrayCount=0;                                                    //JerryYang 20220923 : add
    static int iCnt=0;                                                          //Eastsun 20260515 F017 整合:KYEC iTrayDetectCount

    switch(Task)
    {
        case 1:
            if(MTrayXCanSafeMove()==false)
            {
                MOT[MTrayX].PCIL132_StopMotor();
                Task=50;
                break;
            }
            SuckLoaderErrRetryCT=0;
            Task=10;
        case 10:                                                                //Steven 20170922 (wei) : 加上保護
            if(MOT[MMTrayY].Tray.HasIC()==false &&
               MOT[MMTrayY].fHasTray &&
               InArmXYZSafe()==true)                                            //Steven 20250311 : 確認Tray能移動
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MInArmX].fCanMove=false;
                    MOT[MInArmY].fCanMove=false;
                }

                if(TrayArmMotorMove(Prod.iXTrayLoad))
                {
                    if(IniConfig.bEnable_SECS_GEM==true)                        //wei 20150525 : Secs Gem  add Tray Test Finish
                        EventReport(SECS_EVENT.TrayTestFinish);
                    SetAutoSkipCount(0);                                        //Steven 20150429 : Fixed for Auto Skip
                    bFirstRecordLoaderData=true;                                //Steven 20150429 : Fixed for Auto Skip
                    iAse_LoadTrayEndTotalTime=0;                                //kevin 20170418 (wei) Auto tray end one tray
                    Cylinder[C_TrayY_Fixer].Off();
                    Cylinder[C_LoaderEdgePush].Off();
                    Cylinder[C_LoaderUpPress].Off();                            //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
                    DoCatchFromLoaderDelay.SetMSAndOn(3000);
                    Task=100;
                    break;
                }
            }
            else
            {
                MOT[MTrayX].PCIL132_StopMotor();
                Task=50;
            }
            break;
        case 50:
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                Task=1;
                return 3;
            }
            break;
        case 90:
            Cylinder[C_TrayX_UpDown].On();
            if(LastSet.iRealDummy==DUMMY && Sen[SnLoaderSureTray].IsOn())
            {
                if(Sen[SnLoaderSureTray].IsOn())
                {
                    Task=100;                                                   // dummy ,but has tray
                }
                else
                {
                    Task=95;
                    DoCatchFromLoaderDelay.SetMSAndOn(2000);
                }
            }
            else
            {
                Task=100;
            }
            break;
        case 95:
            if(DoCatchFromLoaderDelay.Off())
                Task=100;
            break;
        case 100:
            if(LastSet.iRealDummy==DUMMY && Sen[SnLoaderSureTray].IsOff())                                              //dummy run
            {
                Task=110;
                DoCatchFromLoaderDelay.SetMSAndOn(1000);
                Cylinder[C_TrayX_UpDown].On();
                if(IniConfig.bC03UseCatchTray==false)                                                                   //use catch Tray Fix
                    CatchTraySuck.Suck[0][0].On();
                MOT[MMTrayY_Car].fCanMove=false;
                break;
            }

            if(Cylinder[C_LoaderUpPress].Enable==true)                                                                  //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
            {
                if(Sen[SnLoaderUpPress].IsOn()==true)
                {
                    if(DoCatchFromLoaderDelay.Off()==true)
                    {
                        ShowMyMessage("Please make sure the loader press cylinder is opened. (SnLoaderUpPress))");
                        break;
                    }
                    break;
                }
            }

            Cylinder[C_TrayY_Fixer].Off();
            Cylinder[C_LoaderEdgePush].Off();
            Cylinder[C_LoaderUpPress].Off();                                                                            //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray

            Cylinder[C_TrayX_UpDown].On();
            if(IniConfig.bC03UseCatchTray==false)
            {
                CatchTraySuck.Suck[0][0].On();
            }
            else                                                                                                        //kevin 20170923 (wei) 夾tray機構鬆開
            {
                if(USE_AUTO_RETEST==eartInstall &&
                   (USE_CATCH_TRAY_MODEL==3 ||                                                                          //Steven 20170623 (wei) : Add for catch tray with cover
                    Cylinder[C_TrayCover].Enable))                                                                      //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    C_CatchTray_Fix_Pop(true);
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].Reset();
                }
            }

            MOT[MMTrayY_Car].fCanMove=false;
            bAutoSkiplog=false;                                                                                         //kevin 20170905 (wei) load tray  catch
            if(IniConfig.bC03UseCatchTray==false)                                                                       //use catch Tray
            {
                Task=120;
                CatchTrayDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);                                            //Steven 20120731 : 減少夾Tray時間, 增加UPH
            }
            else
            {
                if(IniConfig.bSPILFunction &&
                   IniConfig.bC03UseCatchTray &&
                   IniConfig.bA10_AutoReTest)                                                                           //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
                {
                    CatchTrayDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT+1.5);
                }
                else
                {
                    CatchTrayDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);                                        //Steven 20120731 : 減少夾Tray時間, 增加UPH
                }
                Task=145;
            }
            break;
        case 110:
            if(DoCatchFromLoaderDelay.Off())
            {
                CatchFromLoaderSetItemData();                                   //Sam 20220712 : 整合 CatchFromLoader SetItemData
                Task=305;
            }
            break;
        case 120:
            if(CatchTraySuck.Suck[0][0].GetStatus())
            {
                Task=150;
                break;
            }

            if(CatchTrayDelay.Off())
                Task=150;
            break;
        case 145:
            if(CatchTrayDelay.Off())
            {
                if(IniConfig.bC03UseCatchTray==false)
                {
                    Task=150;
                }
                else
                {                                                               //kevin 20170923 (wei) 確保夾tray鬆開
                    if(USE_AUTO_RETEST==eartInstall &&
                       (USE_CATCH_TRAY_MODEL==3 ||                              //Steven 20170623 (wei) : Add for catch tray with cover
                        Cylinder[C_TrayCover].Enable))                          //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    {
                        if(C_CatchTray_Fix_Pop())
                        {
                            Cylinder[C_TrayCover].Off();                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                            Task=150;
                        }
                    }
                    else
                    {
                        if(Cylinder[C_CatchTray_Fix].Pop())
                        {
                            Task=150;
                        }
                    }
                }
            }
            break;
        case 150:
            flag[0]=false;
            flag[1]=false;
            if(USE_AUTO_RETEST==eartInstall &&
               (USE_CATCH_TRAY_MODEL==3 ||                                      //Steven 20170623 (wei) : Add for catch tray with cover
                Cylinder[C_TrayCover].Enable))                                  //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
            {
                Cylinder[C_TrayCover].Off();
                C_CatchTray_Fix_Puch(true);
            }
            Task=155;
        case 155:
            bHasTrayInEmpty=false;
            if(IniConfig.bC03UseCatchTray)                                      //use catch Tray Fix
            {
                if(flag[0]==false)
                {
                    if(LastSet.iRealDummy==DUMMY)                               //Steven 20100618
                    {
                        flag[0]=true;
                    }
                    else
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                          (USE_CATCH_TRAY_MODEL==3 ||                           //Steven 20170623 (wei) : Add for catch tray with cover
                           Cylinder[C_TrayCover].Enable))                       //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            flag[0]=C_CatchTray_Fix_Puch();
                        }
                        else
                        {
                            Cylinder[C_CatchTray_Fix].On();                     //wei 20150729
                            CatchTrayDelay.SetSecAndOn(1);
                            Task=156;
                        }
                    }
                }

                if(flag[0])
                {
                    if(CosFunction.bThickTrayNoNeedCover &&
                       UserDefForm[0].ZDepth>1500)                              //Steven 20200723 : 太厚的Tray不能蓋蓋子
                    {
                    }
                    else
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                           Cylinder[C_TrayCover].Enable)                        //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                            Cylinder[C_TrayCover].On();
                        CatchTrayDelay.SetSecAndOn(0.5);                        //kevin 20150623 確保上蓋下來
                    }
                    Task=156;
                }
            }
            else
            {
                if(CatchTraySuck.Suck[0][0].Suck())
                {
                    CatchFromLoaderSetItemData();                               //Sam 20220712 : 整合 CatchFromLoader SetItemData
                    Task=300;
                    break;
                }

                if(CatchTraySuck.Suck[0][0].Error)
                {
                    Cylinder[C_TrayX_UpDown].Off();
                    Task=170;
                    break;
                }
            }
            break;
        case 156:                                                               //kevin 20150623 確保上蓋下來
            if(CatchTrayDelay.Off())
            {
                Task=160;
            }
            break;
        case 160:
            #ifndef SOFT_SIMULTE
            if(IsTrayArmCatchTrayFail())                                        //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                if(IniConfig.bP19CatchTrayUpThenCheck)                          //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
                {
                    CatchTrayDelay.SetSecAndOn(0.5);
                    Cylinder[C_TrayX_UpDown].Off();
                    Task=161;
                }
                else
                {
                    if(USE_AUTO_RETEST==eartInstall &&
                       (USE_CATCH_TRAY_MODEL==3 ||                              //Steven 20170623 (wei) : Add for catch tray with cover
                        Cylinder[C_TrayCover].Enable))                          //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    {
                        Cylinder[C_CatchTray_FixOn].Off();
                        Cylinder[C_CatchTray_FixOff].On();
                        Cylinder[C_TrayCover].Off();                            //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    }
                    else
                    {
                        Cylinder[C_CatchTray_Fix].Off();
                    }
                    Cylinder[C_TrayX_UpDown].Off();
                    Task=170;
                }
            }
            else
            #endif
            {
                Task=165;
            }
            break;
        case 161:                                                               //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
            if(CatchTrayDelay.Off())
            {
                Task=162;
            }
            break;
        case 162:                                                               //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
            if(IsTrayArmCatchTrayFail())                                        //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                if(USE_AUTO_RETEST==eartInstall &&
                   (USE_CATCH_TRAY_MODEL==3 ||                                  //Steven 20170623 (wei) : Add for catch tray with cover
                    Cylinder[C_TrayCover].Enable))                              //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    Cylinder[C_CatchTray_FixOn].Off();
                    Cylinder[C_CatchTray_FixOff].On();
                    Cylinder[C_TrayCover].Off();                                //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].Off();
                }
                Task=163;
            }
            else
            {
                Task=165;
            }
            break;
        case 163:                                                               //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
            if(SuckLoaderErrRetryCT<ArmSpeed[TrayArm].iRetryCT)                 //Steven 20220620 : Catch tray from loader can auto retry.
            {
                SuckLoaderErrRetryCT++;
                ret=K_RETRY;
            }
            else
            {
                ret=ShowErrorMessage("JAM0604", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_163");
                SuckLoaderErrRetryCT=0;
            }

            if(ret==K_SKIP &&
               Sen[SnLoaderSureTray].IsOff() &&
               Sen[SnLoaderPreDete].IsOff())                                    //jou 2010-06-22 防呆Loader tray skip需再偵測
            {
                MOT[MTrayX].fHasTray=false;
                CatchTraySuck.iWhichTray=-1;                                    //Steven 20220331 : 紀錄Tray從哪來的
                CatchTraySuck.SetItemData(0, 0, NULL_IC);
                MOT[MTrayX].iIsCoverTray=MOT[MMTrayY].iIsCoverTray;
                MOT[MTrayX].sTrayID     =MOT[MMTrayY].sTrayID;
                MOT[MMTrayY].ClearTray(__FUNC__);
                if(CosFunction.bShowHPICCount)                                  //Steven 20221228 : 計算加熱盤IC數量
                {
                    fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                }
                MOT[MMTrayY_Car].fCanMove=true;
                fEmptyCanSupplyNewTray=true;                                    //kevin 20110429 讓empty下降一個tray避免hang up
                Task=230;
                break;
            }
            else if(ret==K_RETRY)
            {
                Cylinder[C_CatchTray_Fix].Off();
                Task=1;                                                         //wei 20150729 修改CatchTray Retry
            }

            if(SoftStop)
                Cylinder[C_TrayX_UpDown].Off();
            Task=90;
            break;
        case 165:
            if(IsTrayArmCatchTrayFail())                                        //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                bHasTrayInEmpty=true;
                Task=163;
                break;
            }
            else
            {
                bHasTrayInEmpty=false;
            }

            CatchFromLoaderSetItemData();                                       //Sam 20220712 : 整合 CatchFromLoader SetItemData
            Task=300;
            break;
        case 170:
            if(SuckLoaderErrRetryCT<ArmSpeed[TrayArm].iRetryCT)                 //Steven 20220620 : Catch tray from loader can auto retry.
            {
                SuckLoaderErrRetryCT++;
                ret=K_RETRY;
            }
            else
            {
                SuckLoaderErrRetryCT=0;
                if(IniConfig.bC03UseCatchTray)                                  //use catch Tray Fix
                    ret=ShowErrorMessage("JAM0604", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_170");
                else
                    ret=ShowErrorMessage("JAM0601", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_170");
            }

            if(ret==K_SKIP &&
               Sen[SnLoaderSureTray].IsOff() &&
               Sen[SnLoaderPreDete].IsOff())                                    //jou 2010-06-22 防呆Loader tray skip需再偵測
            {
                MOT[MTrayX].fHasTray=false;
                CatchTraySuck.iWhichTray=-1;                                    //Steven 20220331 : 紀錄Tray從哪來的
                CatchTraySuck.SetItemData(0, 0, NULL_IC);
                MOT[MMTrayY].ClearTray(__FUNC__);
                if(CosFunction.bShowHPICCount)                                  //Steven 20221228 : 計算加熱盤IC數量
                {
                    fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                }
                MOT[MMTrayY_Car].fCanMove=true;

                fEmptyCanSupplyNewTray=true;                                    //kevin 20110429 讓empty下降一個tray避免hang up
                Task=230;
            }
            else if(ret==K_RETRY)
            {
                Cylinder[C_CatchTray_Fix].Off();
                Task=10;                                                        //wei 20150729 修改CatchTray Retry
            }

            if(SoftStop)
                Cylinder[C_TrayX_UpDown].Off();
            break;
        case 230:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=240;
            else
                break;
        case 240:
            if(MTrayXCanSafeMove()==false)
            {
                Task=230;
                break;
            }
            else
            {
                Task=250;
            }
        case 250:
            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bCatchTrayFinishAction=false;                               //KenHsieh 20250724 : for OCR
                    Task=1;
                    return 2;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bCatchTrayFinishAction=false;                               //KenHsieh 20250724 : for OCR
                    Task=1;
                    return 2;
                }
            }
            break;
        case 300:
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
                   IniConfig.bG11ASEReport)                                                                                                                     //kevin 20210810 read tray id tput new tray
                {
                    iTrayTotal[0]=ASE_InTrayNum;                                                                                                                //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
                    TrayID[0][2]=TrayID[0][1];                                                                                                                  //kevin 20210707 load tray move empty
                    TrayID[0][1]="";
                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                        sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty Tray Start,%d,%s", iTrayTotal[0], iOneTrayPickCount[0], TrayID[0][2]);          //kevin 20210623 add log
                    else
                        sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty2 Tray Start,%d,%s", iTrayTotal[0], iOneTrayPickCount[0], TrayID[0][2]);         //kevin 20210623 add log
                    RecordProcess(sBuffer);
                    iOneTrayPickCount[0]=0;
                }
                Task=301;
                CatchTrayDelay.SetSecAndOn(0.5);
            }
            break;
        case 301:
            if(CatchTrayDelay.Off())
            {
                if(Sen[SnLoaderSureTray].IsOn() ||
                   (TestIF.bOcrFunction==false &&
                    Sen[SnLoaderPreDete].IsOn()))                                                                       //kevin 20150606 Tray未吸起來
                {
                    ret=ShowErrorMessage("JAM0616", K_RETRY, MMTrayY, false, "DoCatchFromLoader_301");                  //kevin 20170923 load track sensor error

                    if(ret==K_RETRY)
                    {
                        return false;
                    }
                }

                if(IniConfig.bKoreaFunction==false)
                    RecordProcess("Load Tray exchange for tray finished.");                                             //kevin 20170614 (wei) log tray

                if(IniConfig.bC03UseCatchTray)
                {
                    if(USE_AUTO_RETEST==eartInstall &&                                                                  //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道
                       Cylinder[C_TrayCover].Enable)                                                                    //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    {
                        MOT[MMTrayY_Car].fCanMove=true;
                        if(Cylinder[C_CatchTray_FixOn].OnSensor()==false)                                               //kevin 20150606
                        {
                            Task=400;
                            break;
                        }
                        else
                        {
                            MOT[MMTrayY_Car].fCanMove=true;
                            Task=310;
                            break;
                        }
                    }
                    #ifndef SOFT_SIMULTE
                    else if(bHasTrayInEmpty && IsTrayArmCatchTrayFail())                                                //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
                    {
                        Cylinder[C_CatchTray_Fix].Off();
                        Cylinder[C_CatchTray_FixOn].Off();
                        Cylinder[C_CatchTray_FixOff].On();
                        Task=400;
                    }
                    #endif
                    else
                    {
                        MOT[MMTrayY_Car].fCanMove=true;
                        Task=310;
                    }
                }
                else
                {
                    MOT[MMTrayY_Car].fCanMove=true;
                    if(CatchTraySuck.Suck[0][0].Enable &&
                       CatchTraySuck.Suck[0][0].GetStatus()==false)
                        Task=400;
                    else
                        Task=310;
                }
            }
            break;
        case 305:
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                MOT[MMTrayY_Car].fCanMove=true;
                Task=310;
            }
            break;
        case 310:
            Task=500;
            break;
        case 400:
            ret=ShowErrorMessage("JAM0613", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_400");                    //wei 20150805 JAM0611==>JAM0613
            if(ret==K_RETRY)
            {
                MOT[MMTrayY].SetTray(NULL_IC, __FUNC__);                                                                //jou 20171024 (wei) : 修正catch tray吸到後起來真空沒on hang up
                Task=1;
                if(CosFunction.bShowHPICCount)                                                                          //Steven 20221228 : 計算加熱盤IC數量
                {
                    fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                }
            }
            else
            {
                MOT[MMTrayY_Car].fCanMove=true;
                MOT[MTrayX].fHasTray=false;
                CatchTraySuck.iWhichTray=-1;                                                                            //Steven 20220331 : 紀錄Tray從哪來的
                CatchTraySuck.SetItemData(0, 0, NULL_IC);
                MOT[MMTrayY].ClearTray(__FUNC__);
                if(CosFunction.bShowHPICCount)                                                                          //Steven 20221228 : 計算加熱盤IC數量
                {
                    fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                }
                fEmptyCanSupplyNewTray=true;
                Task=410;
                break;
            }

            if(SoftStop)
                Cylinder[C_TrayX_UpDown].Off();
            break;
        case 410:
            if(Cylinder[C_TrayX_UpDown].Pop())
                Task=240;
            break;
        case 500:
            if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                   //kevin 20120726
                Task=501;                                                       //放空tray到 auto1
            else
                Task=550;
            break;
        case 502:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=502;
            else
                break;
        case 501:                                                               //kevin 20120718  Auto 2 Auto 3
            if(MTrayXCanSafeMove()==false)
            {
                Task=502;
                break;
            }

            if(IsOutArmSafe())
            {
                iCatchTrayControlManual=2;
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
            }
            else
            {
                if(IniConfig.bP56TrayArmWaitAtColorTrack ||                     //Steven 20240516 : Tray Arm等待位置改到Color
                   (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))         //wei 20150925 待機位置改道 Color
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                    {
                        MOT[MInArmX].fCanMove=true;
                        MOT[MInArmY].fCanMove=true;

                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                }
                else
                {
                    if(TrayArmMotorMove(Prod.iXTrayEmpty))
                    {
                        MOT[MInArmX].fCanMove=true;
                        MOT[MInArmY].fCanMove=true;

                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                }
                break;
            }

            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;

            if(LastSet.iRealDummy!=DUMMY &&
               IniConfig.bC03UseCatchTray==false)
            {
                if(CatchTraySuck.Suck[0][0].Enable && CatchTraySuck.Suck[0][0].GetStatus()==false)
                {
                    //==> Eastsun 20260515 F017 整合:KYEC iTrayDetectCount debounce
                    iCnt++;
                    if(CUSTOMER_CODE!=CC_KYEC_LEE || iCnt>iTrayDetectCount)
                    {
                        iCnt=0;
                    //<== Eastsun 20260515 F017
                        ret=ShowErrorMessage("JAM0610", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_501");
                        if(ret==K_SKIP)
                        {
                            MOT[MTrayX].fHasTray=false;
                            Task=1;
                            return true;
                        }
                //==> Eastsun 20260515 F017
                    }
                //<== Eastsun 20260515 F017
                }
                //==> Eastsun 20260515 F017 整合:KYEC iCnt else reset
                else if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    iCnt=0;
                }
                //<== Eastsun 20260515 F017
            }

            DoPlaceTrayToAutoDelay.SetSecAndOn(0.1);                            //jou 2012-06-05 0.5sec -> 0.1sec 增加UPH
            Task=503;
            break;
        case 503:                                                               //kevin 20120718  Auto 2 Auto 3  移到AUTO2 取空TRAY
             if(DoPlaceTrayToAutoDelay.Off())
            {
                if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    Task=550;
                }
                else if(IsOutArmSafe() &&
                        MOT[MOutArmX].fCanMove==false &&
                        MOT[MOutArmY].fCanMove==false)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                    Task=550;
                }
                else
                {
                    Task=501;
                }
            }
            break;
        case 550:                                                               //ChungHung 20140701 add AutoRetest
            if(USE_AUTO_RETEST==eartInstall &&
               IniConfig.bA12ClearLoaderDevice &&
               Cylinder[C_TrayCover].Enable)                                    //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道
            {
                DoSlapTray(true);
                bNeedSlapTray=true;
                Task=560;
            }
            else
            {
                Task=570;
            }
            break;
        case 560:                                                               //ChungHung 20140701 add AutoRetest
            if(DoSlapTray())
            {
                bNeedSlapTray=false;
                Task=570;
            }
            break;
        case 570:
            if(MTrayXCanSafeMove()==false)
            {
                Task=600;
                break;
            }
            iTrayCount++;                                                       //JerryYang 20220923 : add
            if(CatchTraySuck.Item[0][0]==3)
            {
                pos=Prod.iXTrayAuto[0];                                         //kevin 20120726 移到auto 1
            }
            else if(CatchTraySuck.Item[0][0]==2 ||
                    (TestIF_File.bRENESAS_EnableFTCT==true &&                   //RogerYang 20251018 : FT/RT place to color by trayassing setting
                     TrayForm.LoaderToEmptyColor[iRunStartMode]==1))
            {
                pos=Prod.iXTrayColor;
            }
            else
            {
                pos=Prod.iXTrayEmpty;
            }

            if(CatchTraySuck.Item[0][0]==2 || CatchTraySuck.Item[0][0]==3)      //JerryYang 20210506 add移動前偵測buffer區是否滿tray
            {
                if(Sen[SnEmptyTrayIsFull1].IsOn())
                {
                    if(iTrayCount>=100)
                    {
                        iTrayCount=0;
                        ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoCatchTray");
                    }
                    return false;
                }
                else if(Sen[SnEmptyTrayIsFull2].IsOn())                         //color
                {
                    if(iTrayCount>=100)
                    {
                        iTrayCount=0;
                        ShowErrorMessage("MES1420", K_RETRY, MMColor, false, "DoCatchTray");
                    }
                    return false;
                }
            }
            else
            {
                if(Sen[SnEmptyTrayIsFull1].IsOn())
                {
                    if(iTrayCount>=100)
                    {
                        iTrayCount=0;
                        ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoCatchTray");
                    }
                    return false;
                }
            }
            iTrayCount=0;

            if(LastSet.iRealDummy!=DUMMY && IniConfig.bC03UseCatchTray==false && MOT[MTrayX].fHasTray)
            {
                if(CatchTraySuck.Suck[0][0].Enable && CatchTraySuck.Suck[0][0].GetStatus()==false)
                {
                    //==> Eastsun 20260515 F017 整合:KYEC iTrayDetectCount debounce
                    iCnt++;
                    if(CUSTOMER_CODE!=CC_KYEC_LEE || iCnt>iTrayDetectCount)
                    {
                        iCnt=0;
                    //<== Eastsun 20260515 F017
                        ret=ShowErrorMessage("JAM0610", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_570");
                        if(ret==K_RETRY)
                            MOT[MTrayX].fHasTray=true;
                        else if(ret==K_SKIP)
                            MOT[MTrayX].fHasTray=false;
                //==> Eastsun 20260515 F017
                    }
                //<== Eastsun 20260515 F017
                }
                //==> Eastsun 20260515 F017 整合:KYEC iCnt else reset
                else if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    iCnt=0;
                }
                //<== Eastsun 20260515 F017
            }
            else
            {
                if(MOT[MTrayX].fHasTray && IsTrayArmCatchTrayFail())            //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
                {
                    //==> Eastsun 20260515 F017 整合:KYEC iTrayDetectCount debounce
                    iCnt++;
                    if(CUSTOMER_CODE!=CC_KYEC_LEE || iCnt>iTrayDetectCount)
                    {
                        iCnt=0;
                    //<== Eastsun 20260515 F017
                        ret=ShowErrorMessage("JAM0610", K_SKIP|K_RETRY, MTrayX, false, "DoCatchFromLoader_570");
                        if(ret==K_RETRY)
                            MOT[MTrayX].fHasTray=true;
                        else if(ret==K_SKIP)
                            MOT[MTrayX].fHasTray=false;
                //==> Eastsun 20260515 F017
                    }
                //<== Eastsun 20260515 F017
                }
                //==> Eastsun 20260515 F017 整合:KYEC iCnt else reset
                else if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    iCnt=0;
                }
                //<== Eastsun 20260515 F017
            }

            if(TrayArmMotorMove(pos))
            {
                if(INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR)           //wei 20150925 待機位置改道 Color
                {
                    MOT[MInArmX].fCanMove=false;
                    MOT[MInArmY].fCanMove=false;
                }
                else
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                }
                bDoTrayDeviceCheck=false;                                       //wei 20170317 (steven) Device Remain 殘料檢測

                if(ArmSpeed_File[InArm].bAutoSKIP==1 && bASkStart==false)       //kevin 20170606 (wei) tray end 燈滅
                    bLampTrayEnd=false;
                Task=1;
                return 1;
            }
            break;
        case 600:
            if(Cylinder[C_TrayX_UpDown].Pop())
                Task=500;
            break;
    }
    return 0;
}
//------------------------------------------------------------------------------
//AI(W906-F9050-BD) 20261004: F9050-BD batch D (ledger H064) -- 910 acatchtray.cpp:2434-2603 (Eastsun 0819; ref/frank-910-9050 2275f78c),
//  line by line: InitialCatchFromLoader_9050 / DoCatchFromLoader_9050 (MLoaderZ + C_TrayZ_Selector + C_MobileTrayTableSelect,
//  task 1-60) and InitialCatchFromEmpty_9050 / DoCatchFromEmpty_9050 (MEmptyZ, task 1-70) with their file-scope state.
//  Called ONLY from the MachineTypeChoice==Type_HT9050 arms of DoCatchTray (cases 200 / 250 / 6100); 9050GPIB still decodes
//  as Type_HT9046_LS (database.cpp:517) => never called today.  DoCatchFromLoader_9050 returns 0 / 1 / 3, never 2, so
//  DoCatchTray's 'fail catch 1 tray' path (ret==2 -> 100) does not exist for 9050 (910 as written).
//  Pending Frank/Jimmy (TO_FRANK s4 1001 18:1x; WAITING_REPLIES W-01 Q2): the layer counters start at -1 and index
//  Prod.TrayZ_Up/Down[MAX_TRACK=9] / EmptyZ_Up/Down[20] outside their bounds (910 defect (a); fields in 910's order, so those reads hit 910's neighbours);
//  the NEW Z teach fields (TrayZ_Down / _Home, EmptyZ_*) are never assigned (0), TrayZ_Up[0..8] = golden teach + 200 (cprod_9050.inc head); a motor with Enable=0 'arrives' at once in a machine build (910 defect (e)).  Translated as written.
int iCatchFromLoader9050Task=1;
void InitialCatchFromLoader_9050()
{
    iCatchFromLoader9050Task=1;
}
int DoCatchFromLoader_9050()
{
    int &Task=iCatchFromLoader9050Task;
    switch(Task)
    {
        case 1:                                                                 //Step 1: MTrayX 移到 Loader X 位置
            if(MTrayXCanSafeMove()==false)
            {
                MOT[MTrayX].PCIL132_StopMotor();
                Task=1;
                return 3;
            }
            if(TrayXMoveCheckEnc(Prod.iXTrayLoad)==1)
            {
                if(IniConfig.bEnable_SECS_GEM==true)
                    EventReport(SECS_EVENT.TrayTestFinish);
                if(MOT[MTrayX].fHasTray)
                {
                    Task=10;
                }
                else
                Task=10;
            }
            break;
        case 10:
            if(Cylinder[C_LoaderEdgeClip].Pop() &&
               Cylinder[C_LoaderEdgePush].Pop())
            {
                if(MOT[MLoaderZ].MotorMove(Prod.TrayZ_Up[iLoaderLayerCount_9050])==1)
                    Task=20;
            }
            break;
        case 20:
            Cylinder[C_TrayZ_Selector].On();
            MOT[MLoaderZ].ClearTray(__FUNC__);
            iLoaderLayerCount_9050++;
            Task=30;
            break;
        case 30:
            if(Cylinder[C_LoaderEdgeClip].Pop() &&
               Cylinder[C_LoaderEdgePush].Pop() &&
               Cylinder[C_TrayZ_Selector].Push() &&
               Cylinder[C_MobileTrayTableSelect].Push())
            {
                if(MOT[MLoaderZ].MotorMove(Prod.TrayZ_Up[iLoaderLayerCount_9050] + Prod.iCatchExtraLift_9050)==1)
                {
                    Task=40;
                }
            }
            break;
        case 40:
            Cylinder[C_MobileTrayTableSelect].Off();
            MOT[MTrayX].fHasTray = true;
            MOT[MTrayX].iIsCoverTray = MOT[MMTrayZ].iIsCoverTray;
            MOT[MTrayX].sTrayID      = MOT[MMTrayZ].sTrayID;
            MOT[MMTrayZ].ClearTray(__FUNC__);
            Task=50;
            break;
        case 50:
            if(Cylinder[C_MobileTrayTableSelect].Pop())
            {
                if(MOT[MLoaderZ].MotorMove(Prod.TrayZ_Down[iLoaderLayerCount_9050])==1)
                {
                    Task=60;
                }
            }
            break;
        case 60:                                                                
            {
                int xTarget = (IniConfig.bP56TrayArmWaitAtColorTrack ||
                               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))
                              ? Prod.iXTrayColor
                              : Prod.iXTrayEmpty;
                if(TrayArmMotorMove(xTarget))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bCatchTrayFinishAction=false;
                    Task=1;
                    return 1;
                }
            }
            break;
    }
    return 0;
}

//==============================================================================
// Eastsun 20260819 : 9050 Empty 區 TrayArm 取盤 (機構同 Loader)
//==============================================================================
//AI(W906-F9050-BD) 20261004: 910 :2529-2531 verbatim.  iEmptyLayerCountDetect_9050 is never used (910 as written).
int iEmptyLayerCount_9050        = -1;
int iEmptyLayerCountDetect_9050  = 0;
int iCatchFromEmpty9050Task      = 1;
void InitialCatchFromEmpty_9050()
{
    iCatchFromEmpty9050Task = 1;
}
//AI(W906-F9050-BD) 20261004: NO CALLER, as in 910 (910 defect (c); Frank 1001 17:1x 「暫時先不動」): 9050 Empty picks still go through golden
//  CatchNewTrayFromBuffer, and golden DoAutoEmpty still runs -- two models of one Empty area.  Note: no iEmptyLayerCount_9050
//  change on a pick (W-01 Q2 (c), pending).
int DoCatchFromEmpty_9050()
{
    int &Task = iCatchFromEmpty9050Task;
    switch(Task)
    {
        case 1:                                                                 //Step 1: MTrayX 移到 Empty X
            if(MTrayXCanSafeMove()==false)
            {
                MOT[MTrayX].PCIL132_StopMotor();
                Task=1;
                return 3;
            }
            if(TrayXMoveCheckEnc(Prod.iXTrayEmpty)==1)
                Task=10;
            break;
        case 10:                                                                //Step 2: Edge Pop + Select Push + MZ 到交接位
            if(Cylinder[C_EmptyEdgeClip].Pop() &&
               Cylinder[C_EmptyEdgePush].Pop() &&
               Cylinder[C_MobileTrayTableSelect].Push())
            {
                if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Up[iEmptyLayerCount_9050])==1)
                    Task=20;
            }
            break;
        case 20:                                                                //Step 3: Selector Push + MZ 再上升
            if(Cylinder[C_EmptyEdgeClip].Pop() &&
               Cylinder[C_EmptyEdgePush].Pop() &&
               Cylinder[C_EmptyLoaderZ_Select].Push() &&
               Cylinder[C_MobileTrayTableSelect].Push())
            {
                if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Up[iEmptyLayerCount_9050] + Prod.iEmptyCatchExtraLift_9050)==1)
                    Task=30;
            }
            break;
        case 30:                                                                //Step 4: 盤所有權移交 MMEmpty -> MTrayX
            Cylinder[C_MobileTrayTableSelect].Off();
            MOT[MTrayX].fHasTray     = true;
            MOT[MTrayX].iIsCoverTray = MOT[MMEmpty].iIsCoverTray;
            MOT[MTrayX].sTrayID      = MOT[MMEmpty].sTrayID;
            MOT[MMEmpty].ClearTray(__FUNC__);
            Task=40;
            break;
        case 40:                                                                //Step 5: MZ 回下方待命
            if(Cylinder[C_MobileTrayTableSelect].Pop())
            {
                if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Down[iEmptyLayerCount_9050])==1)
                    Task=70;
            }
            break;
        case 70:                                                                //Step 6: TrayArm 帶盤離開
            {
                int xTarget = (IniConfig.bP56TrayArmWaitAtColorTrack ||
                               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))
                              ? Prod.iXTrayColor
                              : Prod.iXTrayEmpty;
                if(TrayArmMotorMove(xTarget))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bCatchTrayFinishAction=false;
                    Task=1;
                    return 1;
                }
            }
            break;
    }
    return 0;
}
//==============================================================================
// 從 Color1 Color 2 Color 3 吸空TRAY流程
//==============================================================================
void SwapTrayID(bool bToCatch, int iAuto)
{
    if(fAGV->IsATK_AMR())                                                       //Steven 20251210 : ATK AMR
    {
        if(bToCatch)
        {
            if(iAuto==etColor)
            {
                sCIDOnTrayArm           =fSortCT->pnlCoverTrayD->Caption;
                MOT[MTrayX].sTrayID     =fSortCT->pnlCoverTrayD->Caption;
                MOT[MTrayX].iIsCoverTray=IS_ID_TRAY;
                fSortCT->pnlCoverTrayD->Caption="";
            }
            else
            {
                sCIDOnTrayArm           ="";
                MOT[MTrayX].sTrayID     ="";
                MOT[MTrayX].iIsCoverTray=NULL_IC;
            }
        }
        else
        {
            if(iAuto==etEmpty)
            {
                MOT[MMEmpty].sTrayID     =MOT[MTrayX].sTrayID;
                MOT[MMEmpty].iIsCoverTray=MOT[MTrayX].iIsCoverTray;
            }
            else if(iAuto==etColor)
            {
                MOT[MMColor].sTrayID            =MOT[MTrayX].sTrayID;
                MOT[MMColor].iIsCoverTray       =MOT[MTrayX].iIsCoverTray;
                fSortCT->pnlCoverTrayD->Caption =MOT[MTrayX].sTrayID;
            }
            else                                                                //if(iAuto==etAuto2)
            {
                MOT[MMAuto2].sTrayID     =MOT[MTrayX].sTrayID;
                MOT[MMAuto2].iIsCoverTray=MOT[MTrayX].iIsCoverTray;
            }
        }
    }
}
//------------------------------------------------------------------------------
int iCatchNewTrayFromBufferTask=1;
void InitialCatchNewTrayFromBufferTask()
{
    iCatchNewTrayFromBufferTask=1;
    bTrayArmIsPickColor=false;                                                  //JerryYang 20250113 : Color上方TrayID
}
//------------------------------------------------------------------------------
TQPF_Timer CatchNewTrayFromBufferDelay;
TQPF_Timer tCatchNewTrayTimer;
int CatchNewTrayFromBuffer(int iWhichAuto)                                      //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm //1,2,3
{
    #ifdef SOFT_SIMULTE
        bool bFailAlarm=false;
    #else
        bool bFailAlarm=true;                                                   //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm
    #endif

    static bool flag[2];
    int ret, pos;
    static int iRetryCT=0;
    int &Task=iCatchNewTrayFromBufferTask;
    static int iTrayCount=0;
    static int bPickFromColor=0;                                                //kevin 20120718   0:empty 1:color  2:Auto2
    static int SuckEmptyErrRetryCT=0;
    static bool bColorHasDuplicateError=false;                                  //Steven 20120220 : Color Tray重複Alarm Flag
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
    static bool bAuto2HasDuplicateError=false;                                  //kevin  2012726 : Auto2 Tray重複Alarm Flag

    switch(Task)
    {
        case 1:
            SuckEmptyErrRetryCT=0;                                                                                      //Steven 20220602 : move up
            if(AUTO_EMPTY_COLOR!=0)                                                                                     //軌道是自動或手動
            {
                bIsCatchingFromBuffer=true;                                                                             //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中
                Task=2100;
                break;
            }
            else
            {
                if(fAGV->Use_AMR())                                                                                     //Steven 20251216 : for AMR
                {
                    if(bNeed1DCoverTray[0] || bNeed1DCoverTray[1] || bNeed1DCoverTray[2])
                    {
                        ret=1;
                    }
                    else
                    {
                        ret=0;
                    }
                }
                else if(CosFunction.bSpecialP24 &&                                                                      //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                        IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray)                                             //Alick 20160630 add 矽格北興用，開啟SKIP功能，一律從EMPTY供料
                {
                    ret=0;
                }
                else if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                                                        //Sam 20230220 : 移除客戶碼 //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
                {
                    ret=1;
                }
                else
                {
                    if(TrayForm.LodareType==0)
                        ret=TrayForm.LoaderToEmptyColor[iRunStartMode];
                    else
                        ret=TrayForm.AutoFromEmptyColor[iRunStartMode][iWhichAuto-1];
                }

                if(ret==0)
                {
                    if(Sen[SnEmptyTrayHasTray1].IsOff()==false &&
                       Sen[SnEmptyTrayIsLock1].IsOff()==false)
                    {
                        bColorHasDuplicateError=false;                                                                  //Steven 20120220 : Color Tray重複Alarm Flag
                        Task=2100;
                    }
                    else
                    {
                        iTrayCount++;
                        if(Sen[SnEmptyTrayIsLock1].IsOn()==false)
                        {
                            if(iTrayCount>=100)
                            {
                                iTrayCount=0;
                                ShowErrorMessage("MES1023", K_RETRY, MMEmpty, false, "CatchNewTrayFromBuffer");         //marc check
                            }
                        }
                        else if(Sen[SnEmptyTrayHasTray1].IsOn()==false)
                        {
                            if(LastSet.iRealDummy==DUMMY)
                            {
                                Task=2100;
                            }
                            else
                            {
                                if(CosFunction.bSpecialP24 &&                                                           //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                                   IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray)                                  //wei 20180309 (Steven) P24 Empty tray no tray no alarm
                                {
                                    if(iTrayCount>=100)
                                    {
                                        iTrayCount=0;
                                        ShowErrorMessage("MES1021", K_RETRY, MMEmpty, false, "CatchNewTrayFromBuffer");
                                    }
                                }
                                else
                                {
                                    if(bCheckNoCatchEmptyTrayToAuto()==false)
                                    {
                                        if(iTrayCount>=100)
                                        {
                                            iTrayCount=0;
                                            ShowErrorMessage("MES1021", K_RETRY, MMEmpty, false, "CatchNewTrayFromBuffer");
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                else
                {
                    if(ret==1)
                    {
                        if(Sen[SnEmptyTrayHasTray2].IsOff()==false &&
                           Sen[SnEmptyTrayIsLock1].IsOff()==false)
                        {
                            bColorHasDuplicateError=false;                                                              //Steven 20120220 : Color Tray重複Alarm Flag
                            Task=2100;
                        }
                        else
                        {
                            iTrayCount++;
                            if(Sen[SnEmptyTrayIsLock1].IsOn()==false)
                            {
                                if(iTrayCount>=100)
                                {
                                    iTrayCount=0;
                                    ShowErrorMessage("MES1023", K_RETRY, MMColor, bColorHasDuplicateError, __FUNC__);   //Steven 20120925 : JAM1409 -> MES1023
                                    bColorHasDuplicateError=true;                                                       //Steven 20120220 : Color Tray重複Alarm Flag
                                }
                            }
                            else if(Sen[SnEmptyTrayHasTray2].IsOn()==false)
                            {
                                if(LastSet.iRealDummy==DUMMY)
                                    Task=2100;
                                else
                                {
                                    if(iTrayCount>=100)
                                    {
                                        iTrayCount=0;
                                        ShowErrorMessage("MES1421", K_RETRY, MMColor, false, "CatchNewTrayFromBuffer");
                                    }
                                }
                            }
                            break;
                        }
                    }
                    else                                                                                                //kevin 20120718 Auto2
                    {
                        Task=2100;
                    }
                }
                break;
            }
        case 2100:
            if((iWhichAuto==2 || iWhichAuto==3) &&
               TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==2)           //kevin 20120718  Auto 2 Auto 3
                Task=2101;
            else
                Task=2149;
            break;
        case 2101:                                                              //kevin 20120718  Auto 2 Auto 3
            if(MTrayXCanSafeMove()==false)
            {
                Task=2102;
                break;
            }

            if(MOT[MOutArmX].fCanMove==false || MOT[MOutArmY].fCanMove==false)
            {
            }
            else
            {
                if(iCatchTrayControlManual==0)
                    break;
                iCatchTrayControlManual=2;
            }

            if(IsOutArmSafe())                                                  //
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
            }
            else
            {
                if(IniConfig.bP56TrayArmWaitAtColorTrack ||                     //Steven 20240516 : Tray Arm等待位置改到Color
                   (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))         //wei 20150925 待機位置改道 Color
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                }
                else
                {
                    if(TrayArmMotorMove(Prod.iXTrayEmpty))
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                }
                break;
            }

            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;
            DoPlaceTrayToAutoDelay.SetSecAndOn(0.1);                            //jou 2012-06-05 0.5sec -> 0.1sec 增加UPH
            Task=2103;
            break;
        case 2102:                                                              //kevin 20120718  Auto 2 Auto 3
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //在上面
                Task=2101;
            break;
        case 2103:                                                              //kevin 20120718  Auto 2 Auto 3  移到AUTO2 取空TRAY
            if(DoPlaceTrayToAutoDelay.Off())
            {
                if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    Task=2149;
                }
                else if(IsOutArmSafe() &&
                        MOT[MOutArmX].fCanMove==false &&
                        MOT[MOutArmY].fCanMove==false)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                    Task=2149;
                }
                else
                {
                    Task=2101;
                }
            }
            break;
        case 2149:
            if(MTrayXCanSafeMove()==false)
            {
                Task=2150;
                break;
            }

            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                if(CheckInArmFinishAllPickerAction())                           //Steven 20171226 (Wei) : 修改in arm讓開的flag
                {
                    NeedWaitTrayArm=true;                                       //wei 20151002
                    if(InArmXYZSafe()==true)                                    //Steven 20250311 : 確認Tray能移動
                    {
                        MOT[MInArmX].fCanMove=false;
                        MOT[MInArmY].fCanMove=false;
                        Task=2199;
                    }
                }
            }
            else
            {
                Task=2199;
            }
            break;
        case 2150:                                                              //Steven 20231026 : 針對TrayArm動作加上保護
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //在上面
                Task=2100;
            break;
        case 2199 :
            if(fAGV->Use_AMR())                                                 //Steven 20251216 : for AMR
            {
                if(bNeed1DCoverTray[0] || bNeed1DCoverTray[1] || bNeed1DCoverTray[2])
                {
                    ret=1;
                }
                else
                {
                    ret=0;
                }
            }
            else if(CosFunction.bSpecialP24 &&                                  //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                    IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray)         //Alick 20160630 add 矽格北興用，開啟SKIP功能，一律從EMPTY供料
            {
                ret=0;
            }
            else if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                    //Sam 20230220 : 移除客戶碼   //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
            {
                ret=1;
            }
            else
            {
                if(TrayForm.LodareType==0)                                      //Loader Type : same
                    ret=TrayForm.LoaderToEmptyColor[iRunStartMode];
                else                                                            //Loader Type : Different
                    ret=TrayForm.AutoFromEmptyColor[iRunStartMode][iWhichAuto-1];
            }

            if(ret==0)
            {
                pos=Prod.iXTrayEmpty;
                bPickFromColor=0;                                               //kevin 20120718
            }
            else
            {
                if(ret==1)
                    pos=Prod.iXTrayColor;
                else
                    pos=Prod.iXTrayAuto[1];                                     //kevin 20120718 Auto 2

                bPickFromColor=ret;                                             //kevin 20120718
            }

            if(USE_TRAY_MAPPING==1 &&                                           //JerryYang 20250113 : Color上方TrayID
               TestIF_File.bEnableTrayID2==true &&
               USE_KEYENCE_EMPTY==3 &&                                          //3: Keyence at Color
               bPickFromColor==1 &&
               MOT[MMColor].fHasTray==false &&
               MOT[MTrayX].Motor->ReadPos()==Prod.iXTrayEmpty)
            {
                break;
            }

            if(MTrayXCanSafeMove()==false)                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=2150;                                                      //Steven 20231026 : 針對TrayArm動作加上保護
                break;
            }

            if(USE_TRAY_MAPPING==1 &&
               TestIF_File.bEnableTrayID2==true &&
               USE_KEYENCE_EMPTY==3 &&
               MOT[MMColor].fHasTray==false)                                    //JerryYang 20250113 : Color上方TrayID
            {
                MOT[MTrayX].MotorMove(Prod.iXTrayEmpty);
                break;
            }

            if(IniConfig.bC03UseCatchTray && ret==1)                            //JerryYang 20220111 : 避免夾tray版本tray arm移動到color位置風管會與output arm干涉
            {
                if(IsOutArmSafe())
                {
                    //==> Eastsun 20260515 F018 整合:KYEC BU5 OutArm motor protect
                    if(CUSTOMER_CODE==CC_KYEC_LEE && USE_BU5_Function==true)
                    {
                        MOT[MOutArmX].ScanMotorStatus();
                        MOT[MOutArmY].ScanMotorStatus();
                        if(MOT[MOutArmX].Led[iInposLed]==true || MOT[MOutArmY].Led[iInposLed]==true)
                            break;
                    }
                    //<== Eastsun 20260515 F018
                    if(TRAY_ARM_MODE==eAboveCoveyor)
                    {
                        MOT[MOutArmX].fCanMove=false;
                        MOT[MOutArmX].PCIL132_StopMotor();
                        MOT[MOutArmY].fCanMove=false;
                        MOT[MOutArmY].PCIL132_StopMotor();
                    }

                    if(TrayArmMotorMove(pos))
                    {
                        Task=2200;
                    }
                }
                else
                {
                    int iTrayArmPosX =MOT[MTrayX].ReadPos();
                    if(MOT[MOutArmX].fCanMove==false &&
                       MOT[MOutArmY].fCanMove==false &&
                       MOT[MInShuttle1].CompareCommandPos(Prod.InSHT[0].iRight, 2)==1 &&
                       iTrayArmPosX<=Prod.iXTrayColor)                          //kevin 20220916 Shuttle 1X2 hange up 讓OUT ARM 做完到等待點
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                }
            }
            else
            {
                if(TrayArmMotorMove(pos))
                {
                    if(ret==2)                                                  //kevin 20120718
                    {
                        MOT[MOutArmX].fCanMove=false;
                        MOT[MOutArmY].fCanMove=false;
                        iCatchTrayControlManual=0;                              //kevin 20120726
                    }
                    Task=2200;
                }
            }
            break;
        case 2160:
            tCatchNewTrayTimer.LatchCycleTimeSec(true);
            CatchNewTrayFromBufferDelay.SetMSAndOn(4000);
            Task=2165;
            break;
        case 2165:
            if(tCatchNewTrayTimer.LatchCycleTimeSec()>1)
            {
                Task=2160;
                break;
            }

            if(MOT[MMEmpty].fHasTray==false && Sen[SenEmptyHasTray].IsOn())
            {
                if(CatchNewTrayFromBufferDelay.Off())
                {
                    Task=2200;
                }
            }
            else
            {
                Task=2200;
            }
            break;
        case 2180:
            tCatchNewTrayTimer.LatchCycleTimeSec(true);
            CatchNewTrayFromBufferDelay.SetMSAndOn(4000);
            Task=2185;
            break;
        case 2185:
            if(tCatchNewTrayTimer.LatchCycleTimeSec()>1)
            {
                Task=2180;
                break;
            }

            if(MOT[MMColor].fHasTray==false && Sen[SenColorHasTray].IsOn())     // avoid hang
            {
                if(CatchNewTrayFromBufferDelay.Off())
                {
                    Task=2200;
                }
            }
            else
            {
                Task=2200;
            }
            break;
        case 2190:
            tCatchNewTrayTimer.LatchCycleTimeSec(true);
            CatchNewTrayFromBufferDelay.SetMSAndOn(4000);
            Task++;
            break;
        case 2191:
            if(tCatchNewTrayTimer.LatchCycleTimeSec()>1)
            {
                Task=2190;
                break;
            }

            if(MOT[MMAuto2].fHasTray==false && Sen[SnAuto2TrayDetect].IsOn())   // avoid hang     kevin 20120905
            {
                if(CatchNewTrayFromBufferDelay.Off())
                {
                    Task=2200;
                }
            }
            else
            {
                Task=2200;
            }
            break;
        case 2200:
            if(AUTO_EMPTY_COLOR!=0)
            {
                if(bPickFromColor==1)
                {
                    if(MOT[MMColor].fHasTray==false)
                    {
                        if(iReceiveColorTray==0 &&                                                                      //Sam 20200831 : Fix buffer tray can not supply new tray hang up
                           fColorCanSupplyNewTray==false)
                        {
                            ShowMyMessage("Color can not supply new tray error, reset flag");
                            fColorCanSupplyNewTray=true;
                        }
                        Task=2100;
                        return 2;
                    }

                    if(USE_TRAY_MAPPING==1 &&                                                                           //JerryYang 20250113 : Color上方TrayID
                       TestIF_File.bEnableTrayID2==true &&
                       USE_KEYENCE_EMPTY==3)
                    {
                        if(MOT[MMColor].fHasTray==true && MOT[MMColor].Tray.cTrayID=="")
                        {
                            ShowMyMessage("Color Tray ID is Empty! Please take out the tray of Color");
                            Task=2800;                                                                                  //JerryYang 20240709 : 強制拿掉TRAY盤
                            break;
                        }
                    }

                    if(LastSet.iRealDummy &&
                       MOT[MMColor].fHasTray &&
                       Sen[SenColorHasTray].IsOff() &&
                       Sen[SenColorCWDete].IsOff())                                                                     //Eliot 2008_07_09
                    {                                                                                                   //Steven 20120905 : 避免Tray吸取後又掉回去
                        ret=ShowErrorMessage("JAM1430", K_RETRY|K_SKIP, MMColor, bColorHasDuplicateError, __FUNC__);    //Color tray miss error!
                        if(ret==K_RETRY)
                        {
                            bColorHasDuplicateError=true;                                                               //Steven 20120220 : Empty Tray重複Alarm Flag
                            Task=2160;
                        }
                        else
                        {
                            bColorHasDuplicateError=false;                                                              //Steven 20120220 : Empty Tray重複Alarm Flag
                            MOT[MMColor].ClearTray(__FUNC__);
                            Task=1;
                        }
                        break;
                    }
                    else
                    {
                        bColorHasDuplicateError=true;
                    }
                }
                else if(bPickFromColor==0)
                {
                    if(MOT[MMEmpty].fHasTray==false)
                    {
                        if(iReceiveEmptyTray==0 &&                                                                      //Sam 20200831 : Fix buffer trya can not supply new tray hang up
                           fEmptyCanSupplyNewTray==false)
                        {
                            ShowMyMessage("Empty can not supply new tray error, reset flag");
                            fEmptyCanSupplyNewTray=true;
                        }
                        Task=2100;
                        return 2;
                    }

                    if(USE_TRAY_MAPPING==etmInstall &&                                                                  //USE_KEYENCE_EMPTY 0 ccd ;1 keyence auto 2; 2 keyence Empty
                       TestIF_File.bEnableTrayID2==true &&
                       bEmptyReadIDOK==false &&
                       (USE_KEYENCE_EMPTY==0 || USE_KEYENCE_EMPTY==2))                                                  //wei 20180808 MR Tray ID 2
                    {
                        #ifndef SOFT_SIMULTE
                        if(LastSet.iRealDummy!=DUMMY)                                                                   //wei 20200604 空跑不讀取Empty Tray ID
                        #endif
                        {
                            InitialTrayID2Task();
                            Task=4000;
                            break;
                        }
                    }

                    if(LastSet.iRealDummy &&
                       MOT[MMEmpty].fHasTray &&
                       Sen[SenEmptyHasTray].IsOff() &&
                       Sen[SenEmptyCWDete].IsOff())                                                                     //Eliot 2008_07_09
                    {                                                                                                   //Steven 20120905 : 避免Tray吸取後又掉回去
                        ret=ShowErrorMessage("JAM1030", K_RETRY|K_SKIP, MMEmpty, bEmptyHasDuplicateError, __FUNC__);    //Empty tray miss error!
                        if(ret==K_RETRY)
                        {
                            bEmptyHasDuplicateError=true;                                                               //Steven 20120220 : Empty Tray重複Alarm Flag
                            Task=2160;
                        }
                        else
                        {
                            bEmptyHasDuplicateError=false;                                                              //Steven 20120220 : Empty Tray重複Alarm Flag
                            MOT[MMEmpty].ClearTray(__FUNC__);
                            bEmptyReadIDOK=false;
                            Task=1;
                        }
                        break;
                    }
                    else
                    {
                        bEmptyHasDuplicateError=false;                                                                  //Steven 20120220 : Empty Tray重複Alarm Flag
                    }
                }
            }

            if(TrayForm.AutoFromEmptyColor[iRunStartMode][iWhichAuto-1]==2)                                             //kevin 20120726 Auto2 送空tray auto3 強至用 auto2空tray
            {
                if(MOT[MMAuto2].fHasTray==false || MOT[MMAuto2_Car].fHasTray)                                           //避免汽缸尚未動作完
                {
                    Task=2100;
                    return 3;
                }

                if(LastSet.iRealDummy && MOT[MMAuto2].fHasTray &&
                   Sen[SnAuto2TrayDetect].IsOff() &&
                   Sen[SnAuto2CWPreDetect].IsOff())                                                                     //Eliot 2008_07_09
                {
                    ret=ShowErrorMessage("WAR1230", K_RETRY|K_SKIP, MMAuto2, bAuto2HasDuplicateError, __FUNC__);        //Auto 2 tray miss error!   //jou 2014-04-21 JAM%d30 -> WAR%d30
                    if(ret==K_RETRY)
                    {
                        bAuto2HasDuplicateError=true;
                        Task=2160;
                    }
                    else
                    {
                        bAuto2HasDuplicateError=false;
                        MOT[MMAuto2].ClearTray(__FUNC__);
                        fAuto2CanSupplyNewTray=true;                                                                    // 載入新空tray
                        Task=1;
                        return 2;                                                                                       //kevin 20120906
                    }
                    break;
                }
                else
                {
                    bAuto2HasDuplicateError=true;                                                                       //Steven 20120220 : Empty Tray重複Alarm Flag
                }
            }

            if(bPickFromColor==0)                                                                                       //empty    //kevin 20120912  start   //Steven 20121030 : 下降前要把固定鉤放開
            {
                Cylinder[C_Empty_Fix].Off();
            }
            else if(bPickFromColor==1)                                                                                  //color
            {
                Cylinder[C_Color_Fix].Off();
                if(USE_TRAY_MAPPING==1 && USE_KEYENCE_EMPTY==3 &&
                   TestIF_File.bEnableTrayID2)                                                                          //JerryYang 20250113 : Color上方TrayID
                {
                    bTrayArmIsPickColor=true;
                }
                else
                {
                    bTrayArmIsPickColor=false;
                }
            }
            else                                                                                                        //auto2
            {
                Cylinder[C_Auto2Side_Fixer].Off();
                Cylinder[C_Auto2EdgePush].Off();
            }

            if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                InitialCatchUnderTray();
                Task=2700;
            }
            else if(IniConfig.bC03UseCatchTray==false)
            {
                Cylinder[C_TrayX_UpDown].On();
                CatchTraySuck.Suck[0][0].On();
                Task=2300;
            }
            else
            {
                Task=2250;
            }

            if(IniConfig.bSPILFunction &&
               IniConfig.bC03UseCatchTray &&
               IniConfig.bA10_AutoReTest)                                                                               //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
            {
                CatchNewTrayFromBufferDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT+1.5);
            }
            else
            {
                CatchNewTrayFromBufferDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);
            }
            break;
        case 2250:
            if(LastSet.iRealDummy==DUMMY)                                       //Steven 20120417 : Dummy不要夾起來
            {
                ;
            }
            else if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                InitialCatchUnderTray();
                Task=2700;
            }
            else
            {
                if(bPickFromColor==0)                                           //EMPTY
                {
                    if(Sen[SenEmptyCWDete].IsOff())                             //Eliot 2008_07_09
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                           (USE_CATCH_TRAY_MODEL==3 ||                          //Steven 20170623 (wei) : Add for catch tray with cover
                            Cylinder[C_TrayCover].Enable))                      //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            Cylinder[C_CatchTray_FixOn].Off();
                            Cylinder[C_CatchTray_FixOff].On();
                            Cylinder[C_TrayCover].Off();                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        }
                        else
                        {
                            Cylinder[C_CatchTray_Fix].Off();
                        }
                    }
                }
                else
                {
                    if(Sen[SenEmptyCWDete].IsOff())                             //Eliot 2008_07_09
                    {
                        if(USE_AUTO_RETEST==eartInstall &&                      //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道
                           Cylinder[C_TrayCover].Enable)                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            Cylinder[C_CatchTray_FixOn].Off();
                            Cylinder[C_CatchTray_FixOff].On();
                            Cylinder[C_TrayCover].Off();                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        }
                        else
                        {
                            Cylinder[C_CatchTray_Fix].Off();
                        }
                    }
                }
            }
            Cylinder[C_TrayX_UpDown].On();                                      //kevin 20150606

            CatchTrayDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);        //Sam 20200311 : TrayArm 下降 才開始計數，避免機台 Delay 導致夾爪提前夾
            Task=2260;
            break;
        case 2260:
            if(CatchTrayDelay.Off())                                            //Sam 20200311 : TrayArm 下降 才開始計數，避免機台 Delay 導致夾爪提前夾
                Task=2300;
            break;
        case 2300:
            if(CatchNewTrayFromBufferDelay.Off())
            {
                flag[0]=false;
                flag[1]=false;
                Task=2310;
                if(USE_AUTO_RETEST==eartInstall &&
                   (USE_CATCH_TRAY_MODEL==3 ||                                  //Steven 20170623 (wei) : Add for catch tray with cover
                    Cylinder[C_TrayCover].Enable))                              //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    C_CatchTray_Fix_Puch(true);
                    Cylinder[C_TrayCover].Off();
                }
                iRetryCT=0;
            }
            break;
        case 2310:
            if(LastSet.iRealDummy==DUMMY &&
               Sen[SenEmptyHasTray].IsOff())
                CatchTraySuck.Suck[0][0].OnAlarmTime=1;
            else
                CatchTraySuck.Suck[0][0].OnAlarmTime=100;

            bHasTrayInEmpty=false;
            if(IniConfig.bC03UseCatchTray)                                      //use catch Tray Fix
            {
                if(flag[0]==false)
                {
                    if(LastSet.iRealDummy==DUMMY)                               //Steven 20100618
                    {
                        flag[0]=true;
                    }
                    else
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                           (USE_CATCH_TRAY_MODEL==3 ||                          //Steven 20170623 (wei) : Add for catch tray with cover
                            Cylinder[C_TrayCover].Enable))                      //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            flag[0]=C_CatchTray_Fix_Puch();
                        }
                        else
                        {
//                          flag[0]=Cylinder[C_CatchTray_Fix].Push();
                            Cylinder[C_CatchTray_Fix].On();                     //wei 20150729 增加氣缸sensor判斷
                            CatchTrayDelay.SetSecAndOn(1);
                            Task=2340;
                        }
                    }
                }

                if(flag[0])
                {
                    if(CosFunction.bThickTrayNoNeedCover &&
                       UserDefForm[0].ZDepth>1500)                              //Steven 20200723 : 太厚的Tray不能蓋蓋子
                    {
                    }
                    else
                    {
                        if(USE_AUTO_RETEST==eartInstall && Cylinder[C_TrayCover].Enable)
                            Cylinder[C_TrayCover].On();                         //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        CatchTrayDelay.SetSecAndOn(0.5);                        //kevin 20150623 確保上蓋下來
                    }
                    Task=2340;
                }
            }
            else
            {
                if(CatchTraySuck.Suck[0][0].Suck())
                {
                    Task=2500;
                    break;
                }

                if(CatchTraySuck.Suck[0][0].Error)
                {
                    if(LastSet.iRealDummy==DUMMY)
                    {
                        Task=2500;
                    }
                    else
                    {
                        iRetryCT++;
                        if(iRetryCT>5)
                        {
                            Task=2400;
                        }
                        else
                        {
                            CatchTraySuck.Suck[0][0].Reset();
                        }
                    }
                }
            }
            break;
        case 2340:                                                              //kevin 20150623 確保上蓋下來
            if(CatchTrayDelay.Off())
            {
                Task=2350;
            }
            break;
        case 2350:
            if(IsTrayArmCatchTrayFail())                                                                                //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                if(IniConfig.bP19CatchTrayUpThenCheck)                                                                  //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
                {
                    CatchTrayDelay.SetSecAndOn(0.5);
                    Cylinder[C_TrayX_UpDown].Off();
                    Task=2351;
                }
                else
                {
                    if(USE_AUTO_RETEST==eartInstall &&
                       (USE_CATCH_TRAY_MODEL==3 ||                                                                      //Steven 20170623 (wei) : Add for catch tray with cover
                        Cylinder[C_TrayCover].Enable))                                                                  //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    {
                        Cylinder[C_CatchTray_FixOn].Off();
                        Cylinder[C_CatchTray_FixOff].On();
                        Cylinder[C_TrayCover].Off();                                                                    //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    }
                    else
                    {
                        Cylinder[C_CatchTray_Fix].Off();
                    }

                    CatchTrayDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);                                        //Steven 20120731 : 減少夾Tray時間, 增加UPH
                    Task=2360;
                }
                break;
            }
            else
            {
                Task=2355;
            }
        case 2351:                                                              //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
            if(CatchTrayDelay.Off())
            {
                Task=2352;
            }
            break;
        case 2352:                                                              //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
            if(IsTrayArmCatchTrayFail())                                        //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                if(USE_AUTO_RETEST==eartInstall &&
                   (USE_CATCH_TRAY_MODEL==3 ||                                  //Steven 20170623 (wei) : Add for catch tray with cover
                    Cylinder[C_TrayCover].Enable))                              //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    Cylinder[C_CatchTray_FixOn].Off();
                    Cylinder[C_CatchTray_FixOff].On();
                    Cylinder[C_TrayCover].Off();                                //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].Off();
                }
                Task=2353;
            }
            else
            {
                Task=2355;
            }
            break;
        case 2353:                                                              //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
            if(bFailAlarm)
            {
                Cylinder[C_TrayX_UpDown].Off();
                if(SuckEmptyErrRetryCT<ArmSpeed[TrayArm].iRetryCT)              //Steven 20220602 : Add alarm for catch tray
                {
                    SuckEmptyErrRetryCT++;
                    Task=2200;
                    break;
                }
                ShowErrorMessage("JAM0601", K_RETRY, MTrayX, false, "CatchNewTrayFromBuffer_2353");
                Task=1;
//                if(bMustFromColorBuffer)                                      //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm Mark
//                    return 1;
            }
            else
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                {
                    Task=1;
                    AddTrayCount(iWhichAuto-1);                                 //Steven 20251029 : outputtray 數量
                    return 1;
                }
            }
            break;
        case 2355:
            if(IsTrayArmCatchTrayFail())                                        //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                bHasTrayInEmpty=true;
            }
            else
            {
                bHasTrayInEmpty=false;
            }

            Task=2500;
            break;
        case 2360:
            if(CatchTrayDelay.Off())
            {
                if(bFailAlarm)
                {
                    Cylinder[C_TrayX_UpDown].Off();
                    if(SuckEmptyErrRetryCT<ArmSpeed[TrayArm].iRetryCT)                                                  //Steven 20220602 : Add alarm for catch tray
                    {
                        SuckEmptyErrRetryCT++;
                        Task=2200;
                        break;
                    }
                    ShowErrorMessage("JAM0604", K_RETRY, MTrayX, false, "CatchNewTrayFromBuffer_2360");                 //Ifor 20200227 :  JAM0601->JAM0604 alarm錯誤訊息修正
                    Task=1;
//                    if(bMustFromColorBuffer)                                  //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm Mark
//                        return 1;
                }
                else
                {
                    if(Cylinder[C_TrayX_UpDown].Pop())
                    {
                        Task=1;
                        AddTrayCount(iWhichAuto-1);                                                                     //Steven 20251029 : outputtray 數量
                        return 1;
                    }
                }
            }
            break;
        case 2400:
            if(bFailAlarm)
            {
                Cylinder[C_TrayX_UpDown].Off();
                if(SuckEmptyErrRetryCT<ArmSpeed[TrayArm].iRetryCT)              //Steven 20220602 : 3 --> ArmSpeed[TrayArm].iRetryCT
                {
                    SuckEmptyErrRetryCT++;
                    Task=2200;
                    break;
                }
                SuckEmptyErrRetryCT=0;
                ShowErrorMessage("JAM0601", K_RETRY, MTrayX, false, "CatchNewTrayFromBuffer_2400");
                Task=1;                                                         //retry;

                if(bPickFromColor==1)                                           //kevin 20120718
                    MOT[MMColor].ClearTray(__FUNC__);                           //jou 2014-04-23 ClearTray()拿到alarm之後,修正pick up error 不會alarm hang up
                else if(bPickFromColor==0)                                      //kevin 20120718
                    MOT[MMEmpty].ClearTray(__FUNC__);
                else
                    MOT[MMAuto2].ClearTray(__FUNC__);                           //kevin 20120718

//                if(bMustFromColorBuffer)                                      //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm Mark
//                    return 1;
            }
            else
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                {
                    Task=1;
                    AddTrayCount(iWhichAuto-1);                                 //Steven 20251029 : outputtray 數量
                    return 1;
                }
            }
            break;
        case 2450:
            if(Cylinder[C_TrayX_UpDown].Pop())
                Task=1;
            break;
        case 2500:
            if(IniConfig.bC03UseCatchTray)                                      //use catch Tray Fix
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                    Task=2550;
            }
            else
            {
                Task=2600;
                tCheckC_Color_Fix_OffSensor.SetSecAndOn(5);                     //Sam 20200327 : 增加 Time Out
            }
            break;
        case 2550:
            if(IsTrayArmCatchTrayFail())                                                                                //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
            {
                if(USE_AUTO_RETEST==eartInstall &&
                   (USE_CATCH_TRAY_MODEL==3 ||                                                                          //Steven 20170623 (wei) : Add for catch tray with cover
                    Cylinder[C_TrayCover].Enable))                                                                      //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    Cylinder[C_CatchTray_FixOn].Off();
                    Cylinder[C_CatchTray_FixOff].On();
                    Cylinder[C_TrayCover].Off();                                                                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].Off();
                }
                Cylinder[C_TrayX_UpDown].Off();
                ShowErrorMessage("JAM0604", K_RETRY, MTrayX, false, "CatchNewTrayFromBuffer_2550");                     //jou 2016-01-08 JAM0601->JAM0604 alarm錯誤訊息修正
                Task=1;
            }
            else
            {
                if(bNewCatchTrayblock)
                {
                    if(Cylinder[C_CatchTray_FixOn].OnSensor() ||                                                        //kevin 20200512 夾tray遮版削短
                       LastSet.iRealDummy==DUMMY)                                                                       //kevin 20210824 add Dummy mode
                    {
                        Task=2600;
                    }
                    else
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                           (USE_CATCH_TRAY_MODEL==3 ||                                                                  //Steven 20170623 (wei) : Add for catch tray with cover
                            Cylinder[C_TrayCover].Enable))                                                              //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            Cylinder[C_CatchTray_FixOn].Off();
                            Cylinder[C_CatchTray_FixOff].On();
                            Cylinder[C_TrayCover].Off();                                                                //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        }
                        else
                        {
                            Cylinder[C_CatchTray_Fix].Off();
                        }
                        Cylinder[C_TrayX_UpDown].Off();
                        ShowErrorMessage("JAM0604", K_RETRY, MTrayX, false, "CatchNewTrayFromBuffer_2550");             //jou 2016-01-08 JAM0601->JAM0604 alarm錯誤訊息修正
                        Task=1;
                    }
                }
                else
                {
                    Task=2600;
                }
                tCheckC_Color_Fix_OffSensor.SetSecAndOn(5);                                                             //Sam 20200327 : 增加 Time Out
            }
            break;
        case 2600:                                                                                                      //Sam 20210610 : 修正 Tray 上升時被停止再重新啟動 Timer Out 誤報警 JAM1103 問題
            if(bPickFromColor==0 && Cylinder[C_Empty_Fix].OffSensor()==true ||
               bPickFromColor==1 && Cylinder[C_Color_Fix].OffSensor()==true)
            {
                Task=2700;
            }
            else if(tCheckC_Color_Fix_OffSensor.Off())
            {
                if(bPickFromColor==0)
                {
                    ShowErrorMessage("JAM1003", K_RETRY, MEmptyY, false, "CatchNewTrayFromBuffer_2600");                //Sam 20221122 : 修正 Alarm Code
                }
                else
                {
                    ShowErrorMessage("JAM1403", K_RETRY, MColorY, false, "CatchNewTrayFromBuffer_2600");
                }
                tCheckC_Color_Fix_OffSensor.SetSecAndOn(5);                                                             //Sam 20200327 : 增加 Time Out
                break;
            }
            else
            {
                break;
            }
        case 2700:
            if((TRAY_ARM_MODE==eAboveCoveyor && Cylinder[C_TrayX_UpDown].Pop()) ||
               (TRAY_ARM_MODE==eUnderCoveyor && DoCatchUnderTray(1+bPickFromColor)))
            {
                if(LastSet.iRealDummy!=DUMMY && IniConfig.bC03UseCatchTray==false)
                {
                    if(CatchTraySuck.Suck[0][0].Enable &&                                                                                                       //kevin 20120912  tray arm Tray 掉落處理
                       CatchTraySuck.Suck[0][0].GetStatus()==false)
                    {
                        if(USE_TRAY_MAPPING==1 && USE_KEYENCE_EMPTY==3 &&
                           TestIF_File.bEnableTrayID2==true)                                                                                                    //JerryYang 20240423 add
                        {
                            ret=ShowErrorMessage("JAM0601", K_SKIP, MTrayX, false, "CatchNewTrayFromBuffer_2600");
                        }
                        else
                        {
                            ret=ShowErrorMessage("JAM0601", K_RETRY|K_SKIP, MTrayX, false, "CatchNewTrayFromBuffer_2600");
                        }

                        if(ret==K_RETRY)
                        {
                            if(AUTO_EMPTY_COLOR!=0)
                            {
                                if(bPickFromColor==0)                                                                                                           //empty
                                {
                                    if(Sen[SenEmptyHasTray].IsOff() ||
                                       Sen[SenEmptyCWDete].IsOff())                                                                                             //Eliot 2008_07_09
                                    {
                                        ShowErrorMessage("JAM1030", K_RETRY, MMEmpty, false, "CatchNewTrayFromBuffer");                                         //kevin 20120912
                                        break;
                                    }
                                    else
                                    {
                                        SuckEmptyErrRetryCT=3;
                                        Task=2400;
                                        break;
                                    }
                                }

                                if(bPickFromColor==1)                                                                                                           //color
                                {
                                    if(Sen[SenColorHasTray].IsOff() ||
                                       Sen[SenColorCWDete].IsOff())                                                                                             //Eliot 2008_07_09
                                    {
                                        ShowErrorMessage("JAM1430", K_RETRY, MMColor, false, "CatchNewTrayFromBuffer");                                         //kevin 20120912
                                        break;
                                    }
                                    else
                                    {
                                        SuckEmptyErrRetryCT=3;
                                        Task=2400;
                                        break;
                                    }
                                }
                            }

                            if(bPickFromColor==2)
                            {
                                if(Sen[SnAuto2TrayDetect].IsOff() ||
                                   Sen[SnAuto2CWPreDetect].IsOff())                                                                                             //Eliot 2008_07_09
                                {
                                    ShowErrorMessage("WAR1230", K_RETRY, MMAuto2, false, "CatchNewTrayFromBuffer");                                             //kevin 20120912   //jou 2014-04-21 JAM%d30 -> WAR%d30
                                    break;
                                }
                            }
                            SuckEmptyErrRetryCT=3;
                            Task=2400;
                            break;
                        }
                        else
                        {
                            if(AUTO_EMPTY_COLOR!=0)
                            {
                                if(bPickFromColor==0)                                                                                                           //empty
                                {
                                    MOT[MMEmpty].ClearTray(__FUNC__);
                                    fEmptyCanSupplyNewTray=true;
                                    //MOT[MMEmpty].SetTray(NULL_IC);            //Steven 20120905 : 取Tray Hang Up
                                }
                                else if(bPickFromColor==1)                                                                                                      //color
                                {
                                    if(USE_TRAY_MAPPING==1 && USE_KEYENCE_EMPTY==3 &&
                                       TestIF_File.bEnableTrayID2==true)                                                                                        //JerryYang 20240423 add
                                    {
                                        Task=2800;
                                        break;
                                    }
                                    MOT[MMColor].ClearTray(__FUNC__);
                                    fColorCanSupplyNewTray=true;
                                    //MOT[MMColor].SetTray(NULL_IC);            //Steven 20120905 : 取Tray Hang Up
                                }
                            }

                            if(bPickFromColor==2)                                                                                                               //empty
                            {
                                MOT[MMAuto2].ClearTray(__FUNC__);
                                fAuto2CanSupplyNewTray=true;                                                                                                    // 載入新空tray
                            }
                            bEmptyReadIDOK=false;
                            Task=1;
                            return 2;
                        }
                    }
                }

                if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                                                                                                     //Sam 20230220 : 移除客戶碼   //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
                    ret=1;
                else if(TrayForm.LodareType==0)
                    ret=TrayForm.LoaderToEmptyColor[iRunStartMode];
                else
                    ret=TrayForm.AutoFromEmptyColor[iRunStartMode][iWhichAuto-1];

                CatchTraySuck.SetItemData(0, 0, 1+ret);

//                if(bPickFromColor==1)                                           //kevin 20120718
//                    SwapTrayID(true, etLoader);
//                if(bMustFromColorBuffer)                                      //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm
//                    ret=1;
                MOT[MTrayX].fHasTray=true;

                if(AUTO_EMPTY_COLOR!=0)
                {
                    if(bPickFromColor==1)                                                                                                                       //kevin 20120718
                    {
                        CatchTraySuck.iWhichTray=2;                                                                                                             //Steven 20220331 : 紀錄Tray從哪來的

                        if(USE_TRAY_MAPPING==1 &&                                                                                                               //JerryYang 20250113 : Color上方TrayID
                            TestIF_File.bEnableTrayID2==true &&
                            USE_KEYENCE_EMPTY==3)
                        {
                            MOT[MTrayX].Tray.cTrayID=MOT[MMColor].Tray.cTrayID;
                        }
                        LastSet.iColorTrayCount_ART--;
                        SwapTrayID(true, etColor);
                        NewRecordProcess("MES0652", "Tray Arm - Pick tray from Color", "CatchNewTrayFromBuffer");                                               //Steven 20220331 : Tray Arm動作加上紀錄
                        MOT[MMColor].ClearTray(__FUNC__);
                    }
                    else if(bPickFromColor==0)                                                                                                                  //kevin 20120718
                    {
                        CatchTraySuck.iWhichTray=1;                                                                                                             //Steven 20220331 : 紀錄Tray從哪來的
                        LastSet.iEmptyTrayCount_ART--;
                        SwapTrayID(true, etEmpty);
                        NewRecordProcess("MES0651", "Tray Arm - Pick tray from Empty", "CatchNewTrayFromBuffer");                                               //Steven 20220331 : Tray Arm動作加上紀錄
                        MOT[MMEmpty].ClearTray(__FUNC__);
                    }
                    else
                    {
                        CatchTraySuck.iWhichTray=4;                                                                                                             //Steven 20220331 : 紀錄Tray從哪來的
                        LastSet.iUnloaderTrayCount_ART[1]--;
                        SwapTrayID(true, etAuto2);
                        NewRecordProcess("MES0654", "Tray Arm - Pick tray from Auto 2", "CatchNewTrayFromBuffer");                                              //Steven 20220331 : Tray Arm動作加上紀錄
                        MOT[MMAuto2].ClearTray(__FUNC__);                                                                                                       //kevin 20120718
                    }
                    bTrayArmIsPickColor=false;
                }
                bHasTrayInEmpty=false;
                if(AUTO3_IS_MAGAZINE==1 && iWhichAuto==iMagAtAuto)                                                                                              //JerryYang 20230509 : Magazine production log
                {
                    AddTrayCount(iWhichAuto-1);                                                                                                                 //Steven 20251029 : outputtray 數量
                }
                else
                {
                    AddTrayCount(iWhichAuto-1);                                                                                                                 //Steven 20251029 : outputtray 數量
                }

                if(fAGV->Use_AMR() &&                                                                                                                           //Steven 20251216 : for AMR
                   CatchTraySuck.iWhichKit==1)
                {
                    for(int i=0; i<=iAutoRight; i++)
                    {
                        if(bNeed1DCoverTray[i])                                                                                                                 //Need 1D Tray
                        {
                            ShowMyMessage("Need Cover tray but pick from Empty.");
                        }
                    }
                }

                return 1;
            }
            break;
        case 2800:
            if(Sen[SenColorCarHasTray].IsOff()==false || Sen[SenColorCCWDete].IsOff()==false ||
               Sen[SenColorHasTray].IsOff()==false || Sen[SenColorCWDete].IsOff()==false)
            {
                ShowMyMessage("Please remove the tray from Color stack!");
            }
            else
            {
                bTrayArmIsPickColor=false;
                MOT[MMColor].ClearTray(__FUNC__);
                fColorCanSupplyNewTray=true;
                bEmptyReadIDOK=false;
                Task=1;
                return 2;
            }
            break;
        case 4000:
            if(USE_KEYENCE_EMPTY==2)
            {
                if(fTrayMapping->DoTrayIDKeyence2(0))                           //wei 20180808 MR Tray ID 2
                {
                    bEmptyReadIDOK=true;
                    Task=2199;
                }
                else if(TestIF_File.bEnableTrayID2==false)
                {
                    bEmptyReadIDOK=true;
                    Task=2199;
                }
            }
            else
            {
                if(fTrayMapping->DoTrayID2CCD())                                //wei 20180808 MR Tray ID 2
                {
                    bEmptyReadIDOK=true;
                    Task=2199;
                }
                else if(TestIF_File.bEnableTrayID2==false)
                {
                    bEmptyReadIDOK=true;
                    Task=2199;
                }
            }
            break;
    }
    return 0;
}
//AI(W906-F9050-BD) 20261004: F9050-BD batch D (ledger H065) -- 910 acatchtray.cpp:3961-4302 (Eastsun 0819; Frank 1002 `static int pos`),
//  line by line: DoPlaceTrayToEmpty_9050 (task 1-500), DoPlaceTrayToAuto_9050 (MOutArmX/Y + MTrayX + MAuto1~3Z, task 1-700),
//  DoPlaceToBuffer_9050 (Empty / Color(=Empty on 9050) / Auto1) and their Initial*.  Called ONLY from the Type_HT9050 arms of
//  DoCatchTray (cases 1100 / 2120) => never called today.  One deliberate deviation: case 700 of DoPlaceTrayToAuto_9050 (below).
//  Pending Frank/Jimmy: 910 defects (a) (layer indices), (b) (DoPlaceTrayToAuto_9050(0) below), (e) (Enable=0 axes).
//==============================================================================
// Eastsun 20260819 : 9050 Empty 區 TrayArm 放盤 (catch 反向動作)
//==============================================================================
int iPlaceToEmpty9050Task = 1;
void InitialPlaceToEmpty_9050()
{
    iPlaceToEmpty9050Task = 1;
}
int DoPlaceTrayToEmpty_9050()
{
    int &Task = iPlaceToEmpty9050Task;
    switch(Task)
    {
        case 1:                                                                 //Step 1: MTrayX 帶盤到 Empty X
            if(MTrayXCanSafeMove()==false)
            {
                MOT[MTrayX].PCIL132_StopMotor();
                Task=1;
                return 3;
            }
            if(TrayXMoveCheckEnc(Prod.iXTrayEmpty)==1)
            {
                //AI(W906-F9050-BD) 20261004: MOT[MEmptyZ].fHasTray is never written anywhere (910 or V906; golden reads MOT[MMEmptyZ]) => case 100/110 are
                //  unreachable in practice and a place adds 1 layer, not 2 (Frank01 1001 21:22 Q2 (3); W-01 Q2 pending).  As 910.
                if(MOT[MEmptyZ].fHasTray)
                {
                    Task=100;
                }
                else
                {
                    Task=200;
                }

            }
            break;
        case 100:
            if(Cylinder[C_EmptyEdgeClip].Pop() &&
               Cylinder[C_EmptyEdgePush].Pop())
            {
                if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Up[iEmptyLayerCount_9050])==1)
                {
                    Task=110;
                }
            }
            break;
        case 110:
            Cylinder[C_EmptyLoaderZ_Select].On();
            MOT[MMEmpty].ClearTray(__FUNC__);
            iEmptyLayerCount_9050++;
            Task=200;
            break;
        case 200:
            if(Cylinder[C_EmptyEdgeClip].Pop() &&
               Cylinder[C_EmptyEdgePush].Pop() &&
               Cylinder[C_EmptyLoaderZ_Select].Push())
            {
                if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Up[iEmptyLayerCount_9050] + Prod.iEmptyCatchExtraLift_9050)==1)
                {
                    Task=300;
                }
            }
            break;
        case 300:
            Cylinder[C_MobileTrayTableSelect].On();
            MOT[MTrayX].ClearTray(__FUNC__);
            iEmptyLayerCount_9050++;
            Task=400;
            break;
        case 400:
            if(Cylinder[C_EmptyEdgeClip].Pop() &&
               Cylinder[C_EmptyEdgePush].Pop() &&
               Cylinder[C_EmptyLoaderZ_Select].Push() &&
               Cylinder[C_MobileTrayTableSelect].Push())
            {
                if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Down[iEmptyLayerCount_9050])==1)
                {
                    Task=500;
                }
            }
            break;
        case 500:
            if(MOT[MEmptyZ].MotorMove(Prod.EmptyZ_Down[iEmptyLayerCount_9050])==1)
            {
                if(Cylinder[C_MobileTrayTableSelect].Pop())
                {
                   int xTarget = (IniConfig.bP56TrayArmWaitAtColorTrack ||
                               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))
                              ? Prod.iXTrayColor
                              : Prod.iXTrayEmpty;
                    if(TrayArmMotorMove(xTarget))
                    {
                        MOT[MInArmX].fCanMove=true;
                        MOT[MInArmY].fCanMove=true;
                        bCatchTrayFinishAction=false;
                        Task=1;
                        return 1;
                    }
                }
            }
            break;


    }
    return 0;
}

int iPlaceToAuto9050Task = 1;
void InitialPlaceToAuto_9050()
{
    iPlaceToAuto9050Task = 1;
}
int DoPlaceTrayToAuto_9050(int iAutoTarget)
{
    int idx = iAutoTarget - 1;                                                  //1-based -> 0-based
    if(idx < 0 || idx > 2) return 0;

    // IO 陣列 (idx 0=Auto1, 1=Auto2, 2=Auto3)
    static const int MotZ[3]     = { MAuto1Z,               MAuto2Z,               MAuto3Z              };
    static const int MMMot[3]    = { MMAuto1,               MMAuto2,               MMAuto3              };
    static const int EdgeClip[3] = { C_Auto1EdgeClip,       C_Auto2EdgeClip,       C_Auto3EdgeClip      };
    static const int EdgePush[3] = { C_Auto1EdgePush,       C_Auto2EdgePush,       C_Auto3EdgePush      };
    static const int Sel[3]      = { C_Auto1LoaderZ_Select, C_Auto2LoaderZ_Select, C_Auto3LoaderZ_Select};

    int* pAutoZ_Up[3]    = { Prod.Auto1Z_Up,   Prod.Auto2Z_Up,   Prod.Auto3Z_Up   };
    int* pAutoZ_Down[3]  = { Prod.Auto1Z_Down, Prod.Auto2Z_Down, Prod.Auto3Z_Down };
    int  iExtraLift[3]   = { Prod.iAuto1CatchExtraLift_9050,
                             Prod.iAuto2CatchExtraLift_9050,
                             Prod.iAuto3CatchExtraLift_9050 };
    int* piLayerCount[3] = { &iAuto1LayerCount_9050,
                             &iAuto2LayerCount_9050,
                             &iAuto3LayerCount_9050 };
    int &iLayer = *piLayerCount[idx];

    int &Task = iPlaceToAuto9050Task;
    switch(Task)
    {
        case 1:                                                                 //Step 1: MTrayX 帶盤到 Auto[idx] X,判斷 stack 現況
            if(MTrayXCanSafeMove()==false)
            {
                MOT[MTrayX].PCIL132_StopMotor();
                Task=1;
                return 3;
            }

            if(IsOutArmSafe())                                                  //AI(W906-F9050-BD) 20261004: 906->910 DIVERGENCE, pending Jimmy before the type switch (like D4: a Type_HT9050-only copy of 910's margin, or accept golden; the shared helper is NOT changed, every model calls it).  910's IsOutArmSafe last branch (910 acatchtray.cpp:4340-4359, RogerYang 20260626) lets Out Arm Y stand down to YOutArm_Shuttle1_Pick[0][2]-iMargin, iMargin = 500 when OutArmSuck.iPickRow==2 or iOutArmYBase!=0, else 3500 (500 for CC_AMKOR_Korea); this tree links golden's body, -3500 in every case (IsOutArmSafe :4448-4453 = golden 906 acatchtray.cpp:3751-3756).  Frank's machines/HT9050/sim_9378 (INSTALL_OCR=0, USE_TRAY_MAPPING=0, C_TrayCover Enable=0) takes that branch, so the F-01b trace comparison can differ here.  Measured (every function the F9050-BD code calls, two calls deep, 910 2275f78c vs golden 906): the other bodies that differ are customer- or safety-PLC-only (ShowErrorMessage / CheckRecordJamDrivingRecord: KYEC; IOInputBit in MyLaneIo.cpp / myio.cpp and TMyPLC_IO_Modbus::IsOn: the PLC input register range).
            {
                //==> Eastsun 20260515 F018 整合:KYEC BU5 OutArm motor protect
                if(CUSTOMER_CODE==CC_KYEC_LEE && USE_BU5_Function==true)
                {
                    MOT[MOutArmX].ScanMotorStatus();
                    MOT[MOutArmY].ScanMotorStatus();
                    if(MOT[MOutArmX].Led[iInposLed]==true || MOT[MOutArmY].Led[iInposLed]==true)
                        break;
                }
                //<== Eastsun 20260515 F018
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
            }
            else
            {
                if(IniConfig.bP56TrayArmWaitAtColorTrack ||                     //Steven 20240516 : Tray Arm等待位置改到Color
                   (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))         //wei 20150925 待機位置改道 Color
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                    {
                        if(TRAY_ARM_MODE==eAboveCoveyor)
                        {
                            MOT[MOutArmX].fCanMove=true;
                            MOT[MOutArmY].fCanMove=true;
                        }
                    }
                }
                else
                {
                    if(TrayArmMotorMove(Prod.iXTrayEmpty))
                    {
                        if(TRAY_ARM_MODE==eAboveCoveyor)
                        {
                            MOT[MOutArmX].fCanMove=true;
                            MOT[MOutArmY].fCanMove=true;
                        }
                    }
                }
                break;
            }

            if(TrayXMoveCheckEnc(Prod.iXTrayAuto[idx])==1)
            {
                //AI(W906-F9050-BD) 20261004: as DoPlaceTrayToEmpty_9050: MOT[MAuto1~3Z].fHasTray is never written => case 100/110 unreachable in practice. As 910.
                if(MOT[MotZ[idx]].fHasTray)
                    Task=100;                                                   //已有 stack -> 先把現有撥開
                else
                    Task=200;                                                   //空 stack -> 直接放
            }
            break;
        case 100:                                                               //Step 2a: Edge Pop + M<Auto>Z 上升到 Up[iLayer]
            if(Cylinder[EdgeClip[idx]].Pop() &&
               Cylinder[EdgePush[idx]].Pop())
            {
                if(MOT[MotZ[idx]].MotorMove(pAutoZ_Up[idx][iLayer])==1)
                    Task=110;
            }
            break;
        case 110:                                                               //Step 2b: Selector On, MM<Auto> ClearTray, iLayer++
            Cylinder[Sel[idx]].On();
            MOT[MMMot[idx]].ClearTray(__FUNC__);
            iLayer++;
            Task=200;
            break;
        case 200:                                                               //Step 3: Edge Pop + Selector Push, M<Auto>Z 上升 Up[iLayer]+ExtraLift
            if(Cylinder[EdgeClip[idx]].Pop() &&
               Cylinder[EdgePush[idx]].Pop() &&
               Cylinder[Sel[idx]].Push())
            {
                if(MOT[MotZ[idx]].MotorMove(pAutoZ_Up[idx][iLayer] + iExtraLift[idx])==1)
                    Task=300;
            }
            break;
        case 300:                                                               //Step 4: MobileTblSel On, MTrayX ClearTray, iLayer++
            Cylinder[C_MobileTrayTableSelect].On();
            MOT[MTrayX].ClearTray(__FUNC__);
            iLayer++;
            Task=400;
            break;
        case 400:                                                               //Step 5: Edge Pop + Selector Push + MobileTblSel Push, M<Auto>Z 下降 Down[iLayer]
            if(Cylinder[EdgeClip[idx]].Pop() &&
               Cylinder[EdgePush[idx]].Pop() &&
               Cylinder[Sel[idx]].Push() &&
               Cylinder[C_MobileTrayTableSelect].Push())
            {
                //AI(W906-F9050-BD) 20261004: as 910 (:4193): the step comment above says Down[iLayer], the code moves to Up[iLayer-1] (pending Frank).
                if(MOT[MotZ[idx]].MotorMove(pAutoZ_Up[idx][iLayer-1])==1)
                    Task=500;
            }
            break;
        case 500:
            Cylinder[Sel[idx]].Off();
            MOT[MMMot[idx]].fHasTray = true;
            MOT[MMMot[idx]].SetTray(NULL_IC, __FUNC__);
            Task = 600;
            break;
        case 600:
            if(Cylinder[Sel[idx]].Pop())
            {
                if(MOT[MotZ[idx]].MotorMove(pAutoZ_Down[idx][iLayer])==1)
                {
                    if(Cylinder[EdgeClip[idx]].Push() &&
                       Cylinder[EdgePush[idx]].Push())
                    Task=700;
                }
            }
            break;
        case 700:                                                               //Step 6: M<Auto>Z 下降 Down[iLayer] 再確認 + MobileTblSel Pop + TrayArm 離開
            if(MOT[MotZ[idx]].MotorMove(pAutoZ_Down[idx][iLayer])==1)
            {
                if(Cylinder[C_MobileTrayTableSelect].Pop())
                {
                    int xTarget = Prod.iXTrayEmpty;                             //目前只有empty
                    if(TrayArmMotorMove(xTarget))
                    {
                        //AI(W906-F9050-BD) 20261004: DEVIATION from 910 :4222 (decided): 910 writes `MOT[MOutArmY].fCanMove=true;` twice
                        //  (:4222-4223).  Frank 1004 17:45 「這不是改解開Auto Z 而是解開Out Arm 的X/Y軸的移動Flag，因為Auto Z移動到位後才能動Out Arm X/Y」
                        //  => the first line is a typo for MOutArmX: unlock X and Y, pairing case 1's lock (910 :4116-4119) (FROM_FRANK s3
                        //  1004 17:45, 4e003922; the 910 tree itself is unchanged, Frank01 asking).  910 原文：MOT[MOutArmY].fCanMove=true;
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                        bCatchTrayFinishAction=false;
                        Task=1;
                        return 1;
                    }
                }
            }
            break;

    }
    return 0;
}

int iPlaceToBuffer9050Task       = 1;

//AI(W906-F9050-BD) 20261004: NO CALLER in 910 or here (nor InitialPlaceToEmpty_9050 / InitialPlaceToAuto_9050 / InitialCatchFromEmpty_9050):
//  HOME / InitAllProcessTask never reset these cursors, so an aborted sequence resumes mid-way (910 as written; tests pin it).
void InitialPlaceToBuffer_9050()
{
    iPlaceToBuffer9050Task       = 1;
    InitialPlaceToEmpty_9050();
    InitialPlaceToAuto_9050();
}

bool DoPlaceToBuffer_9050()
{
    int &Task = iPlaceToBuffer9050Task;
    static int pos=0;                                                           //Frank 20261002 : 改 static，case 1 算好的 pos 要留到下一圈 case 10 用（原本每圈重設成 0，Tray X 會先跑到 0）
    switch(Task)
    {
        case 1:
            PlaceToBufferSetItemData();                                         //沿用 HT9045 helper 設 Item[0][0]
            if(CatchTraySuck.Item[0][0]==1)
            {
                pos=Prod.iXTrayEmpty;
            }
            else if(CatchTraySuck.Item[0][0]==2)
            {
                //pos=Prod.iXTrayColor;                                         //9050 沒有Color
                pos=Prod.iXTrayEmpty;
            }
            else
            {
                pos=Prod.iXTrayAuto[0];                                         //kevin 20120726 移到auto 1
            }
            Task=10;
            break;
        case 10:
            if(TrayArmMotorMove(pos))
            {
                MOT[MMTrayY_Car].fCanMove=true;                                 //kevin 20150618 夾tray 在下十 home 造成 load無法進tray
                if(CatchTraySuck.Item[0][0] == 1)                                   //目的 = Empty
                {
                    Task = 100;
                }
                else if(CatchTraySuck.Item[0][0] == 2)
                {
                    Task = 100;
                }
                else
                {
                    Task = 200;
                }
            }
            break;
        case 100:                                                               //Step 2a: 放盤到 Empty
            if(DoPlaceTrayToEmpty_9050() == 1)
            {
                Task = 1;
                return true;
            }
            break;
        case 200:                                                               //按照之前架構移到auto 1
            //AI(W906-F9050-BD) 20261004: HAZARD kept as 910 (defect (b); Frank 1004 17:34 「先暫時不用動」, decision D3 / NIGHT_REPORT s0 #105 pending):
            //  DoPlaceTrayToAuto_9050 counts targets from 1 and returns 0 for idx<0 before its switch, so with Tray.Data ToBuffer=2
            //  ([3]AUTO1 -> CatchTraySuck.Item[0][0]==3) this case never finishes: CatchTrayTask stays 2120, no alarm, and HOME does not
            //  clear it (InitialPlaceToBuffer_9050 has no caller).  Pinned by tests/test_flow9050_tray.cpp [H2120].
            if(DoPlaceTrayToAuto_9050(0) == 1)
            {
                Task = 1;
                return true;
            }
            break;
    }
    return false;
}
//==============================================================================
// 把空TRAY 放到 Auto1 Auto2 Auto3 流程
//==============================================================================
int iPlaceTrayToAutoTask=1;
void InitialPlaceTrayToAutoTask()
{
    iPlaceTrayToAutoTask=1;
}
//------------------------------------------------------------------------------
bool IsOutArmSafe()
{
    int iPosY;
    iPosY=MOT[MOutArmY].ReadPos();

    if(TRAY_ARM_MODE==eUnderCoveyor)
        return true;

    if(INSTALL_OCR!=eocrUninstal)                                               //wei 20170713 避免讓位不夠去撞到
    {
        if(iPosY>=(Prod.YOutArm_Shuttle1_Pick[0][2]) ||
           MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1)            //iPosY==Prod.iOutArmSafeY)    //Steven 20240110 : 避免比對異常
            return true;
    }
    else if(USE_TRAY_MAPPING==1 &&
            (USE_IN_OUT_ARM_Y_PITCH==iXYPitchVariable ||
             USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb))                               //KaiHuang 20200811 : Out Arm 退不夠多,Tray Arm 會勾到
    {
        if(iPosY>=Prod.iOutArmSafeY)
            return true;
    }
    else if(Cylinder[C_TrayCover].Enable==true ||
            USE_TRAY_MAPPING==etmInstall)                                       //Sam 20191120 Fix Issue
    {
        if(iPosY>=(Prod.YOutArm_Shuttle1_Pick[0][2]-1900) ||
           MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1)            //iPosY==Prod.iOutArmSafeY)    //Steven 20240110 : 避免比對異常
            return true;
    }
    else
    {
        if(iPosY>=(Prod.YOutArm_Shuttle1_Pick[0][2]-3500) ||
           MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1)            //iPosY==Prod.iOutArmSafeY)    //Steven 20240110 : 避免比對異常
            return true;
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer hPlaceTrayToAutoTimer[3];
TQPF_Timer AvoidOutArmDelay;
bool bTranArmWaitFlag=true;
bool AvoidOutArm(AnsiString S, int Task)
{
    if(IsOutArmSafe())                                                          //
    {
        if(TRAY_ARM_MODE==eAboveCoveyor)
        {
            MOT[MOutArmX].fCanMove=false;
            MOT[MOutArmY].fCanMove=false;
            MOT[MOutArmX].PCIL132_StopMotor();
            MOT[MOutArmY].PCIL132_StopMotor();
        }
        bTranArmWaitFlag=true;
        return false;
    }
    else
    {
        MOT[MTrayX].PCIL132_StopMotor();
        if(bTranArmWaitFlag)
        {
            AvoidOutArmDelay.SetMSAndOn(10000);
            bTranArmWaitFlag=false;
            return true;
        }

        if(AvoidOutArmDelay.Off()==false)
        {
            return true;
        }
        bTranArmWaitFlag=true;

        MOT[MOutArmX].fCanMove=false;
        MOT[MOutArmX].PCIL132_StopMotor();
        MOT[MOutArmY].fCanMove=false;
        MOT[MOutArmY].PCIL132_StopMotor();
        MOT[MTrayX].PCIL132_StopMotor();
        fMain->DoStateRecord(0, false);                                         //KenHsieh 20230116 : 區分手動或自動(sbclick -> Function)
        ShowErrorMessage("WAR0630", 0, MTrayX, false, "AvoidOutArm_"+S+"_"+IntToStr(Task));                             //Sam 20200630 : Add Log //Tray Arm Position Error
        iHome=1;
        fAllMotorHome=false;
        return true;
    }
}
//------------------------------------------------------------------------------
bool DoPlaceTrayToAuto(int AutoTarget)
{
    static int count=0, iCnt=0;                                                 //JerryYang 20220923 : add
    static bool flag2[2]={false, false};                                        //kevin 20150812
    static bool bDupError[MAX_AUTO_TRAY]={false, false, false, false, false, false};                                    //Steven 20120208 : Auto Tray沒放好,重複Alarm
    static int iCountForP48=0;

    AutoTarget--;
    if(AutoTarget<0 || AutoTarget>iAutoCnt)
        return false;

    int iTemp       =0;
    int ret         =0;
    int &Task       =iPlaceTrayToAutoTask;
    int iFixer      =C_AutoSide_Fixer[AutoTarget];
    int iEdgePush   =C_AutoEdgePush[AutoTarget];
    int iUpPress    =C_AutoUpPress[AutoTarget];
    bool bflag1=false, bflag2=false, bflag3=false, bflag4=false;
    TColor cPtr[2]  ={clWhite, clYellow};
    AnsiString str1 ="";
    // AI(W906-BinDispInt) 20260820: gate RETIRED -- TMyBinDispCtrl's real
    // data-layer base landed (BinDisplay/MyBinDisp.{h,cpp}, in ht9045_sm).
    // Golden's own !=NULL guard keeps this dead offline (BinDisCtrl stays
    // NULL until InstallColorBinDisplay is wired), so the flip is
    // behaviour-preserving today and golden-faithful the day it is not.
    if(HSys.BinDisCtrl!=NULL)
        HSys.BinDisCtrl->FlashPro(AutoTarget);                                      //Eastsun 20260514

    if(Task==1150)                                                              //標配,改以sensor Enable作判斷
    {
        if(LastSet.iRealDummy!=DUMMY)
        {
            if(CosFunction.bThickTrayNoNeedCover && UserDefForm[0].ZDepth>1500)                                         //Steven 20200723 : 太厚的Tray不能蓋蓋子
            {
            }
            else
            {
                if(Sen[SnUnLoaderFloating].Enable &&                            //Steven 20110704 : Unloader置偏偵測
                   Sen[SnUnLoaderFloating].IsOn())
                {
                    ShowErrorMessage(sJAM1109[AutoTarget], K_RETRY, iMMAuto[AutoTarget], bDupError[AutoTarget], "DoPlaceTrayToAuto_1");
                    bDupError[AutoTarget]=true;                                 //Steven 20120208 : Auto Tray沒放好,重複Alarm
                    return false;
                }
            }
        }
    }

    switch(Task)
    {
        case 1:
            if(USE_LdUldCassetteMode==1)                                        //RogerYang 20260203 : Add for HT9046CR
            {
                InitDoCassetteToAutoTask(AutoTarget);
                DoPlaceTrayToAutoDelay.SetSecAndOn(15);
                Task=10000;
            }
            else
            {
                Task=100;
            }
            break;
        case 100:
            if(TRAY_ARM_MODE==eAboveCoveyor &&
               MTrayXCanSafeMove()==false)
            {
                Task=150;
                break;
            }

            if(TRAY_ARM_MODE==eAboveCoveyor &&
               (MOT[MOutArmX].fCanMove==false ||
                MOT[MOutArmY].fCanMove==false))
            {
            }
            else
            {
                if(iCatchTrayControlManual==0)
                    break;
                iCatchTrayControlManual=2;
            }

            if(IsOutArmSafe())
            {
                //==> Eastsun 20260515 F018 整合:KYEC BU5 OutArm motor protect
                if(CUSTOMER_CODE==CC_KYEC_LEE && USE_BU5_Function==true)
                {
                    MOT[MOutArmX].ScanMotorStatus();
                    MOT[MOutArmY].ScanMotorStatus();
                    if(MOT[MOutArmX].Led[iInposLed]==true || MOT[MOutArmY].Led[iInposLed]==true)
                        break;
                }
                //<== Eastsun 20260515 F018
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
            }
            else
            {
                if(IniConfig.bP56TrayArmWaitAtColorTrack ||                     //Steven 20240516 : Tray Arm等待位置改到Color
                   (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))         //wei 20150925 待機位置改道 Color
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                    {
                        if(TRAY_ARM_MODE==eAboveCoveyor)
                        {
                            MOT[MOutArmX].fCanMove=true;
                            MOT[MOutArmY].fCanMove=true;
                        }
                    }
                }
                else
                {
                    if(TrayArmMotorMove(Prod.iXTrayEmpty))
                    {
                        if(TRAY_ARM_MODE==eAboveCoveyor)
                        {
                            MOT[MOutArmX].fCanMove=true;
                            MOT[MOutArmY].fCanMove=true;
                        }
                    }
                }
                break;
            }

            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;

            if(LastSet.iRealDummy!=DUMMY && IniConfig.bC03UseCatchTray==false)
            {
                if(CatchTraySuck.Suck[0][0].Enable && CatchTraySuck.Suck[0][0].GetStatus()==false)
                {
                    //==> Eastsun 20260515 F017 整合:KYEC iTrayDetectCount debounce
                    iCnt++;
                    if(CUSTOMER_CODE!=CC_KYEC_LEE || iCnt>iTrayDetectCount)
                    {
                        iCnt=0;
                    //<== Eastsun 20260515 F017
                        ret=ShowErrorMessage("JAM0610", K_SKIP|K_RETRY, MTrayX, false, "DoPlaceTrayToAuto_100");
                        if(ret==K_SKIP)
                        {
                            MOT[MTrayX].fHasTray=false;
                            Task=1;
                            return true;
                        }
                //==> Eastsun 20260515 F017
                    }
                //<== Eastsun 20260515 F017
                }
                //==> Eastsun 20260515 F017 整合:KYEC iCnt else reset
                else if(CUSTOMER_CODE==CC_KYEC_LEE)
                {
                    iCnt=0;
                }
                //<== Eastsun 20260515 F017
            }

            DoPlaceTrayToAutoDelay.SetSecAndOn(0.1);                            //jou 2012-06-05 0.5sec -> 0.1sec 增加UPH
            bEmptyReadIDOK=false;
            Task=200;
            break;
        case 150:
            if(Cylinder[C_TrayX_UpDown].Pop())
                Task=100;
            break;
        case 200:
            if(DoPlaceTrayToAutoDelay.Off())
            {
                fAuto2CanSupplyNewTray=true;
                Task=210;
                iCnt=0;                                                         //JerryYang 20220923 : add
            }
            else                                                                //Steven 20121008 : 加速Tray Arm
            {
                break;
            }
            break;
        case 210:
            if(USE_TRAY_MAPPING==etmInstall &&                                  //USE_KEYENCE_EMPTY 0 ccd ;1 keyence auto 2; 2 keyence Empty
               TestIF_File.bEnableTrayID2==true &&                              //wei 20180808 MR Tray ID 2
               bEmptyReadIDOK==false &&
               USE_KEYENCE_EMPTY==1)
            {
                #ifndef SOFT_SIMULTE
                if(LastSet.iRealDummy!=DUMMY)                                   //wei 20200604 空跑不讀取Empty Tray ID
                #endif
                {
                    InitialTrayID2Task();
                    Task=220;
                    break;
                }
            }
            else
            {
                Task=250;
            }
            break;
        case 220:
            if(fTrayMapping->DoTrayIDKeyence2(1))                               //wei 20180808 MR Tray ID 2
            {
                bEmptyReadIDOK=true;
                Task=210;
            }
            else if(TestIF_File.bEnableTrayID2==false)
            {
                bEmptyReadIDOK=true;
                Task=210;
            }
            break;
        case 240:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
            {
                break;
            }
            else
            {
                Task=250;
            }
        case 250:
            if((TRAY_ARM_MODE==eAboveCoveyor &&
                MTrayXCanSafeMove()==false) ||                                  //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
               bAtuoTrayICDetectErr==true)                                      //Isaac 20180109 (Steven) : auto123可前進後退，感測IC偏置後，trayarm先不要移到Auto上方
            {
                Task=240;
                break;
            }

            if(TrayForm.bMoveAfterTrayGoOut==true &&
               MOT[MMAuto1+AutoTarget].fHasTray==true)                          //JerryYang 20241002 : SPIL要求功能 Trray arm等AUTO TRAY退到外面才能移動
            {
                break;
            }

            #ifndef SOFT_SIMULTE
            if(MOT[MTrayX].fHasTray && IsTrayArmCatchTrayFail())                //JerryYang 20200926 新增tray arm移動中掉落偵測
            {
                iCnt++;
                if(iCnt>100)                                                    //JerryYang 20220923 : add
                {
                    iCnt=0;
                    ret=ShowErrorMessage("JAM0610", K_SKIP|K_RETRY, MTrayX, false, "DoPlaceTrayToAuto_250");
                    if(ret==K_SKIP)
                    {
                        MOT[MTrayX].fHasTray=false;
                        Task=1;
                        return true;
                    }
                }
            }
            else
            {
                iCnt=0;                                                         //JerryYang 20220923 : add
            }
            #endif

            if(USE_TRAY_MAPPING==1 &&                                           //JerryYang 20250113 : Color上方TrayID
               TestIF_File.bEnableTrayID2==true &&
               USE_KEYENCE_EMPTY==3)
            {
                if(LastSet.iRealDummy!=DUMMY && IniConfig.bC03UseCatchTray==false)
                {
                    if(CatchTraySuck.Suck[0][0].Enable && CatchTraySuck.Suck[0][0].GetStatus()==false)
                    {
                        ret=ShowErrorMessage("JAM0610", K_SKIP, MTrayX, false, "DoPlaceTrayToAuto_100");
                        if(ret==K_SKIP)
                        {
                            MOT[MTrayX].fHasTray=false;
                            Task=1;
                            return true;
                        }
                    }
                }
            }

            if(TrayArmMotorMove(Prod.iXTrayAuto[AutoTarget]))
            {
                if(MOT[iMMAuto[AutoTarget]].fHasTray==false)
                {
                    #ifndef SOFT_SIMULTE
                    if(USE_TRAY_MAPPING==1 &&                                   //JerryYang 20250113 : Color上方TrayID
                       TestIF_File.bEnableTrayID2==true &&
                       USE_KEYENCE_EMPTY==3)
                    {
                        if(Sen[SnAuto1TrayDetect+AutoTarget].IsOff()==false)
                        {
                            str1.sprintf("Please remove the tray from AUTO%d", AutoTarget+1);
                            ShowMyMessage(str1);
                            break;
                        }
                    }
                    #endif
                    Cylinder[iUpPress].Off();                                   //JerryYang 20190423 新增unloader壓tray
                    Task=300;
                    if(TRAY_ARM_MODE==eUnderCoveyor)
                    {
                        InitialSupportUnderTray();
                        Task=260;
                    }
                    else
                    {
                        break;                                                  //Sam 20240826 : 補break;
                    }
                }
                else
                {
                    break;                                                      //Steven 20121008 : 加速Tray Arm
                }
            }
            else
            {
                break;                                                          //Steven 20121008 : 加速Tray Arm
            }
        case 260:
            if(DoSupportUnderTray(AutoTarget))
            {
                if(IniConfig.bSPILFunction &&
                   IniConfig.bC03UseCatchTray &&
                   IniConfig.bA10_AutoReTest)                                                                           //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
                    DoPlaceTrayToAutoDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn+1.5);                                 //Steven 20110720 : 等待Hand Down Time
                else
                    DoPlaceTrayToAutoDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);                                     //Steven 20120731 : 減少夾Tray時間, 增加UPH
                Task=510;
            }
            break;
        case 300:
            if(AvoidOutArm(__FUNC__, Task))                                                                                                                     //Sam 20191120 : add log
                break;

            if(SW[SwAutoCCW[AutoTarget]].Status())
                break;

            #ifndef SOFT_SIMULTE
            if(LastSet.iRealDummy!=DUMMY)                                                                                                                       //Sam 20221116 : Magazine TrayArm 自動補 Tray
            {
                if(AUTO3_IS_MAGAZINE==1 && AutoTarget==2)
                {
                    if(Cylinder[C_CatchMagazineTray].OnSensor() ||
                       Sen[SnAuto3EdgePush].IsOn() ||
                       Cylinder[C_Auto3EdgePush].Status)                                                                                                        //放下去前要檢查 CatchMagazineTrayr夾爪收起來 & 側推汽缸收起來
                        break;

                    if(iAuto3MagazineIndex!=-1)
                        break;
                }
            }
            #endif
            if(AUTO3_IS_MAGAZINE==1 && AutoTarget==2)
            {
                if(bChaneMagTrayflag)
                    break;
            }

            if(Sen[SnAutoUpPress[AutoTarget]].IsOn())                                                                                                           //JerryYang 20190423 新增unloader壓tray
            {
                if(DoPlaceTrayToAutoDelay.Off())
                {
                    ShowMyMessage("Unloader up press pop error", "Unloader 上壓汽缸放開異常");
                    Cylinder[iUpPress].Off();                                                                                                                   //JerryYang 20190423 新增unloader壓tray
                    DoPlaceTrayToAutoDelay.SetSecAndOn(5);
                    break;
                }
            }

            if(Cylinder[C_TrayX_UpDown].Push())
            {
                count=0;
                CatchTraySuck.Suck[0][0].Off();                                                                                                                 //kevin 20110601 先將tray關真空避免tray因alarm從空而掉下來
                if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bG11ASEReport)                                                                                  //kevin 20210810 read tray id tput new tray//kevin 20210623 add color log
                {
                    TrayID[3+AutoTarget][1]=TrayID[3+AutoTarget][0];                                                                                            //kevin 20210623 TriD move //kevin 20210623 TriD move

                    if(IniConfig.bP56TrayArmWaitAtColorTrack ||                                                                                                 //Steven 20240516 : Tray Arm等待位置改到Color
                       (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR) ||                                                                                   //wei 20150925 待機位置改道 Color
                        (TrayForm.LodareType==1 && TrayForm.AutoFromEmptyColor[iRunStartMode][AutoTarget]==1))
                        str1.sprintf("<AutoMove>Empty2 Tray %d move to Auto%d Tray finish,%s ", iTrayTotal[2], AutoTarget+1, TrayID[3+AutoTarget][1]);          //color tray
                    else
                        str1.sprintf("<AutoMove>Empty Tray %d move to Auto%d Tray finish,%s ", iTrayTotal[1], AutoTarget+1, TrayID[2][1]);                      //Empty tray

                    ReadWriteTrayID(false);                                                                                                                     //kevin 20220618 record color tray id move Auto 1 2 3
                }
                iOneTrayPickCount[1+AutoTarget]=0;                                                                                                              //kevin 20210623 Auto 1 2 3 放數量清除
                RecordProcess(str1);                                                                                                                            //kevin 20210604 add tray arm put on finish
                if(IniConfig.bSPILFunction &&
                   IniConfig.bC03UseCatchTray &&
                   IniConfig.bA10_AutoReTest)                                                                                                                   //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
                {
                    DoPlaceTrayToAutoDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT+1.5);                                                                    //Sam 20250207 : 修正 TrayArm Down auto delay time //Steven 20110720 : 等待Hand Down Time
                }
                else
                {
                    DoPlaceTrayToAutoDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);                                                                        //Steven 20120731 : 減少夾Tray時間, 增加UPH
                }
                Task=400;
            }
            else                                                                                                                                                //Steven 20121008 : 加速Tray Arm
            {
                break;
            }
        case 400:                                                               //jou 2010-09-06 start : 當Tray放置偏斜時,左右30條,嘗試讓他滑入
            if(USE_AUTO_RETEST==eartInstall)
            {
                CatchTraySuck.Suck[0][0].Off();                                 //jou 2011-08-11 要把真空關閉，破壞打開
                Task=500;
            }
            else
            {
                if(DoPlaceTrayToAutoDelay.Off() ||
                   (LastSet.iRealDummy!=DUMMY &&
                    Sen[SnAutoTrayDetect[AutoTarget]].IsOn()))                  //Steven 20121008 : 加速Tray Arm
                {
                    if(LastSet.iRealDummy!=DUMMY &&
                       Sen[SnAutoTrayDetect[AutoTarget]].IsOff())
                    {
                        if(Cylinder[C_TrayX_UpDown].Pop())
                        {
                            Task=401;
                        }
                    }
                    else
                    {
                        CatchTraySuck.Suck[0][0].Off();                         //jou 2011-08-11 要把真空關閉，破壞打開
                        Task=500;
                    }
                }
            }
            break;
        case 401:
            if(LastSet.iRealDummy!=DUMMY && Sen[SnAutoTrayDetect[AutoTarget]].IsOff())
            {
                if(USE_TRAY_MAPPING==1 &&                                       //JerryYang 20250113 : Color上方TrayID
                   TestIF_File.bEnableTrayID2==true &&
                   USE_KEYENCE_EMPTY==3)
                {
                    ret=ShowErrorMessage(sJAM1111[AutoTarget], K_SKIP, iMMAuto[AutoTarget], bDupError[AutoTarget], "DoPlaceTrayToAuto_401");
                }
                else
                {
                    ret=ShowErrorMessage(sJAM1111[AutoTarget], K_RETRY|K_SKIP, iMMAuto[AutoTarget], bDupError[AutoTarget], "DoPlaceTrayToAuto_401");
                }
                bDupError[AutoTarget]=true;                                     //Steven 20120208 : Auto Tray沒放好,重複Alarm
                if(ret==K_SKIP)                                                 //kevin 20150605  tray 掉了
                {
                    MOT[MTrayX].fHasTray=false;
                    Task=1;
                    if(USE_TRAY_MAPPING==1 &&                                   //JerryYang 20250113 : Color上方TrayID
                       TestIF_File.bEnableTrayID2==true &&
                       USE_KEYENCE_EMPTY==3)
                    {
                        if(Sen[SnAuto1TrayDetect+AutoTarget].IsOn())            //按SKIP又檢查到有TRAY
                        {
                            Task=402;
                            break;
                        }
                    }
                    return true;
                }
                else
                {
                    Task=250;                                                   //ChungHung 20140628 Fix 如果Tray detect sensor 沒亮就算Retry 也不會在動作 此時catch Tray 位放開夾爪
                    break;
                }
            }
            else
            {
                Task=500;
            }
            break;
        case 402:
            if(Sen[SnAuto1TrayDetect+AutoTarget].IsOn())                        //按SKIP又檢查到有TRAY, 必須移除tray盤
            {
                str1.sprintf("Please remove the tray from AUTO%d", AutoTarget+1);
                ShowMyMessage(str1);
            }
            else
            {
                Task=1;
                return true;
            }
            break;
        case 500:
            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;

            flag2[0]=false;
            flag2[1]=false;
            if(USE_AUTO_RETEST==eartInstall &&
               (USE_CATCH_TRAY_MODEL==3 ||                                      //Steven 20170623 (wei) : Add for catch tray with cover
                Cylinder[C_TrayCover].Enable))                                  //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
            {
                C_CatchTray_Fix_Pop(true);
            }
            else
            {
                Cylinder[C_CatchTray_Fix].Reset();
            }
            Task=510;
        case 510:
            if(AvoidOutArm(__FUNC__, Task))                                                                             //Sam 20191120 : add log
                break;

            if(DoPlaceTrayToAutoDelay.Off())
            {
                if(CatchTraySuck.iWhichTray!=-1)                                                                        //Steven 20220331 : 紀錄Tray從哪來的
                    str1.sprintf("DoPlaceTrayToAuto - From %s", sTrayPosName[CatchTraySuck.iWhichTray]);
                else
                    str1.sprintf("DoPlaceTrayToAuto - From Unkonwn");

                if(IniConfig.bC03UseCatchTray ||
                   TRAY_ARM_MODE==eUnderCoveyor)
                {
                    if(LastSet.iRealDummy==DUMMY ||                                                                     //Steven 20120417 : Dummy不要夾起來
                       TRAY_ARM_MODE==eUnderCoveyor)
                    {
                        flag2[0]=true;
                    }
                    else if(flag2[0]==false)
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                           (USE_CATCH_TRAY_MODEL==3 ||                                                                  //Steven 20170623 (wei) : Add for catch tray with cover
                            Cylinder[C_TrayCover].Enable))                                                              //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            Cylinder[C_TrayCover].Off();                                                                //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                            flag2[0]=C_CatchTray_Fix_Pop();
                        }
                        else
                        {
                            flag2[0]=Cylinder[C_CatchTray_Fix].Pop();
                        }
                    }

                    if(flag2[0])
                    {
                        if(AUTO3_IS_MAGAZINE==1)                                                                        //JerryYang 20221215 : add Magazine
                        {
                            if(AutoTarget==iMagAtAuto)                                                                  //AUTO3
                            {
                                iAuto3MagazineIndex=iWhichMag;
                            }
                        }

                        MOT[iMMAuto[AutoTarget]].SetTray(NULL_IC, __FUNC__);

                        if(USE_TRAY_MAPPING==1 &&                                                                       //JerryYang 20250113 : Color上方TrayID
                           TestIF_File.bEnableTrayID2==true &&
                           USE_KEYENCE_EMPTY==3)
                        {
                            MOT[MMAuto1+AutoTarget].Tray.cTrayID=MOT[MTrayX].Tray.cTrayID;
                        }

                        if(TestIF_File.bCheckTrayIDBylot)
                        {
                            fTrayMapping->listTrayIDByLot->Clear();
                            fTrayMapping->listTrayIDByLot->Add(MOT[MMAuto1+AutoTarget].Tray.cTrayID);
                            WriteDataToFile(asTrayIDByLot, fTrayMapping->listTrayIDByLot->Text);
                        }

                        if(MOT[MTrayX].iIsCoverTray==IS_ID_TRAY)
                        {
                            InitialCoverTrayIDTask(iKeyenceCoverTrayID_AUTO1+AutoTarget);
                            fTrayMapping->DoCoverTrayID(iKeyenceCoverTrayID_AUTO1+AutoTarget);
                            MOT[iMMAuto[AutoTarget]].sTrayID=MOT[MTrayX].sTrayID;
                            iThisPortNo=eAuto1+AutoTarget;
                            asBundleTrayID[iThisPortNo]=MOT[MTrayX].sTrayID;

                            if(IniConfig.bEnable_SECS_GEM==true)
                            {
                                if(AutoTarget==eAuto1)
                                {
                                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto1);
                                }
                                else if(AutoTarget==eAuto2)
                                {
                                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto2);
                                }
                                else if(AutoTarget==eAuto3)
                                {
                                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto3);
                                }
                                else if(AutoTarget==eAuto4)
                                {
                                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto4);
                                }
                                else if(AutoTarget==eAuto5)
                                {
                                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto5);
                                }
                                else if(AutoTarget==eAuto6)
                                {
                                    EventReport(SECS_EVENT.BundleEnd_IDREAD_Auto6);
                                }

                                EventReport(SECS_EVENT.UnloaderTrayIDReadOK);
                            }
                        }

                        MOT[MTrayX].ClearTray("DoPlaceTrayToAuto 510");

                        AMR.SupplyCover(AutoTarget);                                                                    //Sam 20240304 : 新增 AMR 功能
                        SetUnloaderInfoFile(AutoTarget);                                                                //ChungHung 20150205 add for ATK
                        bOldAutoHasTray[AutoTarget]=false;                                                              //Sam 20230322 : 避免 Tray 資料異常疊料。
                        if(CUSTOMER_CODE!=CC_ASE_KaohSiung &&                                                           //kevin 20150601 不是高雄TRAY需留一空排空
                           CosFunction.bUseSCKART==false &&
                           CUSTOMER_CODE!=CC_KYEC_LEE)  /*Eastsun 20260515 F018 整合:KYEC ART skip last row*/                                                               //Steven 20161201 : For SCK 93K ART
                        {
                            if(USE_AUTO_RETEST==eartInstall &&
                               IniConfig.bA10_AutoReTest &&
                               LastSet.bLoaderTrayCount_ART &&                                                          //ChungHung 20141002 add for KYEC AutoRetest
                               Prod.bART6Tray[AutoTarget] &&
                               LastSet.iRunStartMode==rsmContinuStart_ART)
                            {
                                for(int i=0; i<MOT[iMMAuto[AutoTarget]].Tray.XItem; i++)
                                    MOT[iMMAuto[AutoTarget]].SetTraySingleData(i, MOT[iMMAuto[AutoTarget]].Tray.YItem-1,HAS_NULL_IC);
                            }
                        }

                        iTemp=CatchTraySuck.Item[0][0]-1;                                                               //Steven 20150911 : 避免記憶體破壞
                        if(iTemp<0)
                            iTemp=0;

                        if(AutoTarget==0)
                        {
                            NewRecordProcess("MES0673", "Tray Arm - place tray to Auto 1", str1);                       //Steven 20220331 : Tray Arm動作加上紀錄
                            fMain->mtAuto1->Color=cPtr[iTemp];
                        }
                        else if(AutoTarget==1)
                        {
                            NewRecordProcess("MES0674", "Tray Arm - place tray to Auto 2", str1);                       //Steven 20220331 : Tray Arm動作加上紀錄
                            fMain->mtAuto2->Color=cPtr[iTemp];
                        }
                        else if(AutoTarget==2)
                        {
                            NewRecordProcess("MES0675", "Tray Arm - place tray to Auto 3", str1);                       //Steven 20220331 : Tray Arm動作加上紀錄
                            fMain->mtAuto3->Color=cPtr[iTemp];
                        }
                        CatchTraySuck.iWhichTray=-1;                                                                    //Steven 20220331 : 紀錄Tray從哪來的
                        CatchTraySuck.SetItemData(0, 0, NULL_IC);
                        CatchTrayDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);                                         //Steven 20120731 : 減少夾Tray時間, 增加UPH
                        if(TRAY_ARM_MODE==eAboveCoveyor)
                        {
                            if(CUSTOMER_CODE==CC_AMKOR_Japan)
                            {
                                Task=1000;                                                                              //Sam 20250417 : 客戶要求測推後勾上升前也都不要動作 //Sam 20250326 : 客戶要求不要偷跑
                                break;
                            }
                            else
                            {
                                Cylinder[C_TrayX_UpDown].Off();
                            }
                        }
                        count=0;

                        if(TRAY_ARM_MODE==eUnderCoveyor)                                                                //Ztex 2024.03.25 Add Reduce tray exit time
                        {
                            Cylinder[iFixer].On();
                            Cylinder[iEdgePush].On();
                            MOT[iMMAuto[AutoTarget]].SetTray(NULL_IC, __FUNC__);
                            Task=530;
                        }
                        else
                        {
                            Task=520;
                        }
//                        break;                                                //Steven 20121008 : 加速Tray Arm
                    }
                }
                else
                {
                    Cylinder[iFixer].On();

                    MOT[iMMAuto[AutoTarget]].SetTray(NULL_IC, __FUNC__);
                    MOT[iMMAuto[AutoTarget]].Tray.cTrayID=MOT[MTrayX].Tray.cTrayID;
                    MOT[MTrayX].fHasTray=false;
                    MOT[MTrayX].ClearTray(__FUNC__);

                    if(TestIF_File.bCheckTrayIDBylot)
                    {
                        fTrayMapping->listTrayIDByLot->Clear();
                        fTrayMapping->listTrayIDByLot->Add(MOT[MMAuto1+AutoTarget].Tray.cTrayID);
                        WriteDataToFile(asTrayIDByLot, fTrayMapping->listTrayIDByLot->Text);
                    }

                    if(AUTO3_IS_MAGAZINE)                                                                               //JerryYang 20221215 : add Magazine
                    {
                        if(AutoTarget==2)                                                                               //AUTO3
                        {
                            iAuto3MagazineIndex=iWhichMag;
                        }
                    }
                    SetUnloaderInfoFile(AutoTarget);                                                                    //ChungHung 20150205 add for ATK

                    bOldAutoHasTray[AutoTarget]=false;

                    iTemp=CatchTraySuck.Item[0][0]-1;                                                                   //Steven 20150911 : 避免記憶體破壞
                    if(iTemp<0)
                        iTemp=0;

                    if(AutoTarget==0)
                    {
                        NewRecordProcess("MES0673", "Tray Arm - place tray to Auto 1", str1);                           //Steven 20220331 : Tray Arm動作加上紀錄
                        fMain->mtAuto1->Color=cPtr[iTemp];
                    }
                    else if(AutoTarget==1)
                    {
                        NewRecordProcess("MES0674", "Tray Arm - place tray to Auto 2", str1);                           //Steven 20220331 : Tray Arm動作加上紀錄
                        fMain->mtAuto2->Color=cPtr[iTemp];
                    }
                    else if(AutoTarget==2)
                    {
                        NewRecordProcess("MES0675", "Tray Arm - place tray to Auto 3", str1);                           //Steven 20220331 : Tray Arm動作加上紀錄
                        fMain->mtAuto3->Color=cPtr[iTemp];
                    }
                    CatchTraySuck.Suck[0][0].Off();                                                                     // keep purge
                    CatchTraySuck.iWhichTray=-1;                                                                        //Steven 20220331 : 紀錄Tray從哪來的
                    CatchTraySuck.SetItemData(0, 0, NULL_IC);

                    DoPlaceTrayToAutoDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);                                     //Steven 20110719 Start: 要有吹氣的Delay
                    Task=900;
                    count=0;
                    break;
                }

                if(IniConfig.bC03UseCatchTray==false)
                {
                    if(CatchTraySuck.Suck[0][0].Error)
                    {
                        Task=600;
                        break;
                    }
                    else
                    {
                        break;                                                                                          //Steven 20121008 : 加速Tray Arm
                    }
                }
                else
                {
                    break;                                                                                              //Steven 20121008 : 加速Tray Arm
                }
            }
            else                                                                                                        //Steven 20121008 : 加速Tray Arm
            {
                break;
            }
        case 520:
            if(AvoidOutArm(__FUNC__, Task))                                     //JerryYang 20191210 優化Auto定位方式,側推->後勾->側推+後勾
                break;

            Cylinder[iFixer].On();
            if(CatchTrayDelay.Off())
            {
                Task=530;
                CatchTrayDelay.SetSecAndOn(0.5);
            }
            break;
        case 530:
            if(AvoidOutArm(__FUNC__, Task))
                break;

            if(TRAY_ARM_MODE==eUnderCoveyor)                                    //Ztex 2024.03.25 Add Reduce tray exit time
            {
                Task=1000;
                break;
            }
            Cylinder[iFixer].Off();
            if(CatchTrayDelay.Off())
            {
                Task=540;
                CatchTrayDelay.SetSecAndOn(0.5);
            }
            break;
        case 540:
            if(AvoidOutArm(__FUNC__, Task))
                break;

            Cylinder[iFixer].On();
            Cylinder[iEdgePush].On();
            if(CatchTrayDelay.Off())
                Task=1000;
            break;
        case 600:
            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;
            ShowErrorMessage("JAM0627", K_RETRY, MTrayX, false, "DoPlaceTrayToAuto_600");
            if(SoftStop)
                Cylinder[C_TrayX_UpDown].Off();
            Task=300;
            break;
        case 900:                                                               //Steven 20110719 Start: 要有吹氣的Delay
            Cylinder[iFixer].On();
            Cylinder[iEdgePush].On();
            if(DoPlaceTrayToAutoDelay.Off())
                Task=1000;
            break;
        case 1000:
            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;
            bAutoEdgePush[AutoTarget]=false;
            if(CUSTOMER_CODE==CC_AMKOR_Japan)
            {
                                                                                //Sam 20250417 : 客戶要求測推後勾上升前也都不要動作
            }
            else
            {
                Cylinder[iFixer].On();
                Cylinder[iEdgePush].On();
            }

            if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                if(IniConfig.bP48UnloaderCylinderLoop)                          //JimmyChiu 20230512 Unloader cylinder loop
                {
                    iCountForP48=0;
                    Task=1030;
                    break;
                }
                else
                {
                    Task=1040;
                }
            }
            else if(Cylinder[C_TrayX_UpDown].Pop())
            {
                CatchTraySuck.Suck[0][0].Normal();

                MOT[MOutArmX].fCanMove=false;
                MOT[MOutArmX].PCIL132_StopMotor();
                MOT[MOutArmY].fCanMove=false;
                MOT[MOutArmY].PCIL132_StopMotor();
                if(IniConfig.bP48UnloaderCylinderLoop)                          //JimmyChiu 20230512 Unloader cylinder loop
                {
                    iCountForP48=0;
                    Task=1030;
                    break;
                }
                else
                {
                    Task=1040;
                }
            }
            break;
        case 1030:
            if(IniConfig.bP48UnloaderCylinderLoop)                              //JimmyChiu 20230512 Unloader cylinder loop
            {
                if(DoP43UnloaderCylinderLoop.Off())
                {
                    if(iCountForP48%2==0)
                    {
                        if(iCountForP48>=(IniConfig.iP48UnloaderCylinderLoopTimes*2))
                        {
                            Cylinder[iFixer].On();
                            Cylinder[iEdgePush].On();
                            Task=1040;
                        }
                        else
                        {
                            Cylinder[iFixer].On();
                            Cylinder[iEdgePush].Off();
                        }
                    }
                    else
                    {
                        Cylinder[iFixer].Off();
                        Cylinder[iEdgePush].On();
                    }
                    iCountForP48++;
                    DoP43UnloaderCylinderLoop.SetSecAndOn(IniConfig.iP48UnloaderCylinderLoopDelay);
                }
            }
            else
            {
                Task=1040;
            }
            break;
        case 1040:
            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
                TrayArmMotorMove(Prod.iXTrayColor);
            else
                TrayArmMotorMove(Prod.iXTrayEmpty);

            if(TRAY_ARM_MODE==eUnderCoveyor)                                    //Ztex 2024.03.25 Add Reduce tray exit time
            {
                Task=1060;
                break;
            }
            else if(AUTO3_IS_MAGAZINE==1 && AutoTarget==2)
            {
                DoPlaceTrayToAutoDelay.SetSecAndOn(1.5);                        //Steven 20120731 : 減少夾Tray時間, 增加UPH
                Task=1070;
                break;
            }
            else if(Cylinder[iEdgePush].Enable)
            {
                Cylinder[iFixer].Off();
                Cylinder[iEdgePush].On();
                Task=1050;
                break;
            }
            Task=1100;
            break;
        case 1050:
            if(AUTO3_IS_MAGAZINE==1)                                            //Richard 20231031 : fix magazine auto3 cylinder error
            {
                Cylinder[iFixer].On();
                    Task=1060;
                break;
            }
            else
            {
                if(Cylinder[iFixer].Pop())
                {
                    Cylinder[iFixer].On();
                    Task=1060;
                }
            }
            break;
        case 1060:
            if(AUTO3_IS_MAGAZINE==1)                                                                                    //Richard 20231031 : fix magazine auto3 cylinder error
            {
                Cylinder[iEdgePush].On();
                DoPlaceTrayToAutoDelay.SetSecAndOn(Ld_UldDelayTime.ULD_FixTrayDely);                                    //Steven 20120731 : 減少夾Tray時間, 增加UPH
                bAutoEdgePush[AutoTarget]=true;
                Task=1070;
                break;
            }
            else
            {
                if(Cylinder[iFixer].Push())
                {
                    Cylinder[iEdgePush].On();
                    DoPlaceTrayToAutoDelay.SetSecAndOn(Ld_UldDelayTime.ULD_FixTrayDely);                                //Steven 20120731 : 減少夾Tray時間, 增加UPH
                    bAutoEdgePush[AutoTarget]=true;
                    Task=1070;
                }
            }
            break;
        case 1070:
            if(DoPlaceTrayToAutoDelay.Off())
            {
                Cylinder[iUpPress].On();                                        //JerryYang 20190423 新增unloader壓tray
                Task=1100;
            }
            break;
        case 1100:
            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;

            #ifndef SOFT_SIMULTE
                bflag1=Sen[SnAutoTrayDetect[AutoTarget]].IsOff();
                bflag2=Sen[SnAutoFixCyPush[AutoTarget]].IsOn();
                bflag3=Sen[SnAutoEdgePush[AutoTarget]].IsOn();
                bflag4=Cylinder[iFixer].OffSensor();                            //wei 20241011 新增後勾氣缸off sensor,避免氣缸沒作動

                if(AUTO3_IS_MAGAZINE==1 &&  AutoTarget==iMagAtAuto)
                {
                    bflag4=false;
                }

            #else
                bflag1=false;
                bflag2=false;
                bflag3=false;
                bflag4=false;
            #endif

            if(LastSet.iRealDummy!=DUMMY &&
               (bflag1 || bflag2 || bflag3 || bflag4))
            {
                if(bflag1)
                {
                    str1.sprintf("%s must off. (DoPlaceTrayToAuto_1100)", Sen[SnAutoTrayDetect[AutoTarget]].Name);
                }
                else if(bflag2)                                                                                                                                 //Steven 20241108 : 增加顯示sensor狀態
                {
                    str1.sprintf("%s must off. (DoPlaceTrayToAuto_1100)", Sen[SnAutoFixCyPush[AutoTarget]].Name);
                }
                else if(bflag3)
                {
                    str1.sprintf("%s must off. (DoPlaceTrayToAuto_1100)", Sen[SnAutoEdgePush[AutoTarget]].Name);
                }
                else
                {
                    str1.sprintf("%s must off. (DoPlaceTrayToAuto_1100)", Cylinder[iFixer].OffSensorName);
                }

                Cylinder[iFixer].Off();
                Cylinder[iEdgePush].Off();
                Cylinder[iUpPress].Off();                                                                                                                       //JerryYang 20190423 新增unloader壓tray

                if(USE_TRAY_MAPPING==1 &&                                                                                                                       //JerryYang 20250113 : Color上方TrayID
                   TestIF_File.bEnableTrayID2==true &&
                   USE_KEYENCE_EMPTY==3)                                                                                                                        //USE_KEYENCE_EMPTY 0 ccd ;1 keyence auto 2; 2 keyence Empty; 3 Keyence Color
                {
                    ret=ShowErrorMessage(sJAM1111[AutoTarget], K_SKIP, iMMAuto[AutoTarget], bDupError[AutoTarget], str1);                                       //Sam 20200630 : Add Log
                }
                else
                {
                    ret=ShowErrorMessage(sJAM1111[AutoTarget], K_RETRY|K_SKIP, iMMAuto[AutoTarget], bDupError[AutoTarget], str1);                               //Sam 20200630 : Add Log
                }
                bDupError[AutoTarget]=true;                                                                                                                     //Steven 20120208 : Auto Tray沒放好,重複Alarm
                if(ret==K_SKIP)                                                                                                                                 //Steven 20100225
                {
                    MOT[iMMAuto[AutoTarget]].ClearTray(__FUNC__);
                    MOT[iMMAuto[AutoTarget]].fHasTray=false;
                    bDupError[AutoTarget]=false;                                                                                                                //Steven 20120208 : Auto Tray沒放好,重複Alarm
                    Task=1150;                                                                                                                                  //kevin 20110423 按skip補tray
                    break;
                }
                Task=1200;
                break;
            }
            else
            {
                bDupError[AutoTarget]=false;                                                                                                                    //Steven 20120208 : Auto Tray沒放好,重複Alarm
            }
            Task=1140;
            break;
        case 1130:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=1140;
            else
                break;
        case 1140:
            if(MTrayXCanSafeMove()==false)                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=1130;
                break;
            }
            else
            {
                Task=1150;
            }
        case 1150:
            bEmptyReadIDOK=false;
            if(MOT[MMAuto1+AutoTarget].fHasTray==false && Sen[SnAuto1TrayDetect+AutoTarget].IsOn())                     //按SKIP又檢查到有TRAY
            {
                str1.sprintf("Please remove the tray from AUTO%d", AutoTarget+1);
                ShowMyMessage(str1);
                break;
            }

            if(IsOutArmSafe())
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
            }
            else
            {
                if(IniConfig.bP56TrayArmWaitAtColorTrack ||                                                             //Steven 20240516 : Tray Arm等待位置改到Color
                   (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))                                                 //wei 20150925 待機位置改道 Color
                    TrayArmMotorMove(Prod.iXTrayColor);
                else
                    TrayArmMotorMove(Prod.iXTrayEmpty);
                break;
            }

            if(AvoidOutArm(__FUNC__, Task))                                                                             //Sam 20191120 : add log
                break;

            if(MACHINE_HAS_AUTO_ALIGNMENT_CCD &&                                                                        //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
               TestIF.bEnableAutoAlignment==true &&                                                                     //ChungHung 20210113 add for Alignment CCD start
               (LastSet.iRealDummy==HAS_TRAY || LastSet.iRealDummy==REALLY))
            {
                if(CheckOutArmAutoAlignmentTrayModeBeUse(AutoAlignmentTray_EveryTray))
                {
                    InitDoOutArmTeachAlignmentProcessTask();
                    bAutoNeedTrayMustFinish=true;
                    bAlignmentChangeUnloadTray[AutoTarget]=true;
                }

                if(bAutoNeedTrayMustFinish && WhichAutoNeedTray()!=0)
                {
                    Task=1;
                    return true;
                }
            }

            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                                                                 //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))                                                     //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    if(IniConfig.b18bUseAutoTrayMap)                                                                    //kevin 20220819 use auto unload add empty tray
                    {
                        Task=1160;
                    }
                    else
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                        MOT[MInArmX].fCanMove=true;
                        MOT[MInArmY].fCanMove=true;
                        NeedWaitTrayArm=false;                                                                          //wei 20151002
                        iCatchTrayControlManual=0;
                        Task=1;
                        if(bMagazineGetNewTray==true && AutoTarget==2)                                                  //JerryYang 20250920 : fixed for Multi bin
                            bMagazineGetNewTray=false;
                        return true;
                    }
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    if(IniConfig.b18bUseAutoTrayMap)                                                                    //kevin 20220819 use auto unload add empty tray
                    {
                        Task=1160;
                    }
                    else
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                        iCatchTrayControlManual=0;
                        Task=1;
                        if(bMagazineGetNewTray==true && AutoTarget==2)                                                  //JerryYang 20250920 : fixed for Multi bin
                            bMagazineGetNewTray=false;

                        if(fAGV->Use_AMR())                                                                             //Steven 20251216 : for AMR
                        {
                            for(int iAutoTarget=0; iAutoTarget<3; iAutoTarget++)
                            {
                                if(MOT[iMMAuto[iAutoTarget]].fHasTray==true)
                                {
                                    if(bNeed1DCoverTray[iAutoTarget])
                                    {
                                        MOT[iMMAuto[iAutoTarget]].InitNewTray(HAS_IC, false, __FUNC__);
                                        bAutoEdgePush[iAutoTarget]=true;
                                        bNeed1DCoverTray[iAutoTarget]=false;
                                        bHas1DCoverTray[iAutoTarget]=true;
                                        bNeedCoverTray[iAutoTarget]=true;
                                        str1.sprintf("Support New 1D Cover Tray to %s", s6TrayName[iAutoTarget]);
                                        RecordProcess(str1);
                                    }
                                    else if(bNeedCoverTray[iAutoTarget])
                                    {
                                        MOT[iMMAuto[iAutoTarget]].InitNewTray(HAS_IC, false, __FUNC__);
                                        bAutoEdgePush[iAutoTarget]=true;
                                        bNeedCoverTray[iAutoTarget]=false;
                                        bHasCoverTray[iAutoTarget]=true;
                                        str1.sprintf("Support New Empty Tray to %s", s6TrayName[iAutoTarget]);
                                        RecordProcess(str1);
                                    }
                                }
                            }
                            bAUTORev[AutoTarget]=false;
                        }
                        else if(LastSet.iTester==OFF_LINE && bPurgeOutAllDevice)//JerryYang 20251020 : 渠梁半清機功能

                        {
                            for(int iAutoTarget=0; iAutoTarget<3; iAutoTarget++)
                            {
                                if(bNeedCoverTray[iAutoTarget])
                                {
                                    MOT[MMAuto1+iAutoTarget].InitNewTray(HAS_IC, false, __FUNC__);
                                    bAutoEdgePush[iAutoTarget]=true;
                                    bNeedCoverTray[iAutoTarget]=false;
                                    bHasCoverTray[iAutoTarget]=true;
                                    RecordProcess("Support New Empty Tray");
                                }
                            }
                        }

                        return true;
                    }
                }
            }
            break;
        case 1160:                                                                                                                                              //kevin 20220819 add for empty tray
            if(iUnloaderTrayCountCal[AutoTarget]==0 || bAutoReceTray)                                                                                           //kevin 20220908 add 退料TRAY 補空TRAY
            {
                MOT[iMMAuto[AutoTarget]].InitNewTray(HAS_IC, false, __FUNC__);                                                                                  //FULL TRAY RECE TRAY
                bAutoEdgePush[AutoTarget]=true;
                if(bAutoReceTray)                                                                                                                               //kevin 20220908 add 退料TRAY 補空TRAY
                {
                    iUnloaderTrayCountCal[AutoTarget]=0;
                    bAutoReceTray=false;
                    str1.sprintf("WAR0910%d", 7+AutoTarget);                                                                                                    //kevin 20220915 add AGV error
                    ShowErrorMessage(str1, K_RETRY, iMMAuto[AutoTarget]);                                                                                       //kevin 20220908 UNLOAD TRAY finish call AGV
                }
            }
            else if(IniConfig.b18bDoubleUnloadTray)
            {
                if(iUnloaderTrayCountCal[AutoTarget]==(IniConfig.iUnloaderTrayCount[AutoTarget]+1) ||
                   iUnloaderTrayCountCal[AutoTarget]==(IniConfig.iUnloaderTrayCount[AutoTarget]*2+1))                                                           //kevin 20220506 add Unload Tray 2  倍的設定
                {
                    MOT[iMMAuto[AutoTarget]].InitNewTray(HAS_IC, false, __FUNC__);
                    bAutoEdgePush[AutoTarget]=true;

                    if((iUnloaderTrayCountCal[AutoTarget]+1)==IniConfig.iUnloaderTrayCount[AutoTarget]*2)
                    {
                        iUnloaderTrayCountCal[AutoTarget]=0;                                                                                                    //KEVIN 20220601 Add 需call AGV搬走
                        str1.sprintf("<CoverTrayAuto>Cover Tray Move To Auto%d, Tray Count %d ",AutoTarget+1,IniConfig.iUnloaderTrayCount[AutoTarget]+1);       //kevin 20220819 add log
                        RecordProcess(str1);
                    }
                }
            }
            else
            {
                if(iUnloaderTrayCountCal[AutoTarget]==(IniConfig.iUnloaderTrayCount[AutoTarget]+1))                                                             //kevin 20220706 add Unload Tray 2  倍的設定
                {
                    MOT[iMMAuto[AutoTarget]].InitNewTray(HAS_IC, false, __FUNC__);
                    bAutoEdgePush[AutoTarget]=true;
                    iUnloaderTrayCountCal[AutoTarget]=0;                                                                                                        //kevin 20220601 Add 需call AGV搬走
                    str1.sprintf("<CoverTrayAuto>Cover Tray Move To Auto%d, Tray Count %d ",AutoTarget,IniConfig.iUnloaderTrayCount[AutoTarget]+1);             //kevin 20210623
                    RecordProcess(str1);
                    str1.sprintf("WAR0910%d",7+AutoTarget);                                                                                                     //kevin 20220915 add AGV error
                    ShowErrorMessage(str1, K_RETRY, iMMAuto[AutoTarget]);                                                                                       //kevin 20220908 UNLOAD TRAY finish call AGV
                }
            }

            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                                                                                                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))                                                                                             //wei 20150925 待機位置改道 Color
            {
                MOT[MOutArmX].fCanMove=true;
                MOT[MOutArmY].fCanMove=true;
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                NeedWaitTrayArm=false;                                                                                                                          //wei 20151002
            }
            else
            {
                MOT[MOutArmX].fCanMove=true;
                MOT[MOutArmY].fCanMove=true;
            }
            iCatchTrayControlManual=0;
            Task=1;

            if(bMagazineGetNewTray==true && AutoTarget==2)                                                                                                      //JerryYang 20250920 : fixed for Multi bin
                bMagazineGetNewTray=false;
            return true;
        case 1200:
            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;

            Cylinder[iFixer].On();
            Cylinder[iEdgePush].On();
            Cylinder[iUpPress].On();                                            //JerryYang 20190423 新增unloader壓tray

            CatchTrayDelay.SetSecAndOn(2);
            Task=1300;
            break;
        case 1300:
            if(AvoidOutArm(__FUNC__, Task))                                     //Sam 20191120 : add log
                break;

            if(CatchTrayDelay.Off())
                Task=1100;
            break;
        case 10000:                                                             //RogerYang 20260203 : Add for HT9046CR
            if(DoCassetteToAuto(AutoTarget))
            {
                Task=400;
            }
    }
    return false;
}
//==============================================================================
// 把空TRAY 放到 Color 1,Color 2,Color 3 流程
//==============================================================================
int iPlaceToBufferTask=1;
void InitialPlaceToBufferTask()
{
    iPlaceToBufferTask=1;
}
//------------------------------------------------------------------------------
TQPF_Timer DoPlaceToBufferDelay;
TQPF_Timer DoPlaceToBufferTimeOut;                                              //Sam 20220714 : 新增加 Time Out
bool DoPlaceToBuffer()
{
    int ret,pos;
    int &Task=iPlaceToBufferTask;
    static int iTrayCount=0, iCnt=0;                                            //JerryYang 20220923 : add
    static bool bEmptyColorFullTray=false;
    static bool bEmptyHasDuplicateError=false;                                  //Steven 20120220 : Empty Tray重複Alarm Flag
    static bool bColorHasDuplicateError=false;                                  //Steven 20120220 : Color Tray重複Alarm Flag
    static bool bAuto1HasDuplicateError=false;                                  //kevin 20120718  : Auto1 Tray重複Alarm Flag
#ifndef SOFT_SIMULTE
    bool bEmptyStackNotReady=false;                                             //Sam 20220714 : 新增加 Time Out
    bool bColorStackNotReady=false;                                             //Sam 20220714 : 新增加 Time Out
#endif
    AnsiString str1, str2;
   //kevin 20110601 整個函數修改
    switch(Task)
    {
        case 1:
            if(AUTO_EMPTY_COLOR==0 &&
               Sen[SnEmptyTrayIsLock1].IsOff()==true)
            {
                iTrayCount++;
                if(iTrayCount>=200)
                {
                    iTrayCount=0;
                    ShowErrorMessage("MES1050", K_RETRY, MMEmpty, false, "DoPlaceToBuffer");
                }
                return false;
            }
            else
            {
                Task=2100;                                                      //kevin 2011/06/02
                iCnt=0;                                                         //JerryYang 20220923 : add
            }
        case 2100:
            if(MTrayXCanSafeMove()==false)
            {
                Task=2150;
                break;
            }
            #ifndef SOFT_SIMULTE
            if(MOT[MTrayX].fHasTray && IsTrayArmCatchTrayFail())                //JerryYang 20200926 新增tray arm移動中掉落偵測
            {
                iCnt++;
                if(iCnt>100)
                {
                    iCnt=0;
                    ret=ShowErrorMessage("JAM0610", K_SKIP|K_RETRY, MTrayX, false, "DoPlaceTrayToBuffer_2100");
                    if(ret==K_SKIP)
                    {
                        MOT[MTrayX].fHasTray=false;
                        Task=1;
                        return true;
                    }
                }
            }
            else
            {
                iCnt=0;
            }
            #endif
            PlaceToBufferSetItemData();                                         //Sam 20221027 : 整合 CatchTray SetItemData
            if(CatchTraySuck.Item[0][0]==1)
            {
                pos=Prod.iXTrayEmpty;
            }
            else if(CatchTraySuck.Item[0][0]==2)                                //kevin 20120726
            {
                pos=Prod.iXTrayColor;
            }
            else
            {
                pos=Prod.iXTrayAuto[0];                                         //kevin 20120726 移到auto 1
            }

            if(IniConfig.bP04ColorIsEmptyUnloader)
            {
                pos=Prod.iXTrayColor;
            }

            if(IniConfig.bP38UseEmptyFullPutColor &&
               CatchTraySuck.Item[0][0]==1)                                     //wei 20170504 Use Empty Full Put Color
            {
                if(Sen[SnEmptyTrayIsFull1].IsOn()==true)
                {
                    CatchTraySuck.SetItemData(0, 0, 2);                         // to black
                    pos=Prod.iXTrayColor;
                }
            }

            if(TrayArmMotorMove(pos))
            {
                MOT[MMTrayY_Car].fCanMove=true;                                 //kevin 20150618 夾tray 在下十 home 造成 load無法進tray
                Task=2250;
            }
            break;
        case 2150:
            if(TRAY_ARM_MODE==eAboveCoveyor)
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                    Task=2100;
            }
            else
            {
                InitProcessSingleMotorTask(MTrayZ);
                Task=2160;
            }
            break;
        case 2160:
            if(ProcessSingleMotorHome(MTrayZ))
            {
                if(MOT[MTrayZ].Led[iHomeLed])
                {
//                    SetMotorScaleSpeed(MTrayZ, ArmSpeed[TrayArm].iBodySP);
                    Task=2100;
                }
                else
                {
                    Task=2150;
                }
            }
            break;
        case 2250:
            if(CatchTraySuck.Item[0][0]==3)                                     //kevin 20121003 load 放到 auto1
            {
                if(MOT[MMAuto1].fHasTray==false && MOT[MMAuto1_Car].fHasTray==false)
                    Task=2251;
            }
            else
            {
                Task=2251;
            }
            break;
        case 2251:                                                              //kevin 20121003 load 放到 auto1
            DoPlaceToBufferTimeOut.SetSecAndOn(300.0);                          //Sam 20220714 : 新增加 Time Out
            Task=2260;
        case 2260:                                                              //kevin 20121003 load 放到 auto1
            //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
            //==>
            #ifndef SOFT_SIMULTE
            if((CosFunction.bManuallyRemoveForceInColor &&
                IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT) ||
                CosFunction.bManuallyRemoveForceInColor==false)
            {
                if(AUTO_EMPTY_COLOR!=0)
                {
                    bColorStackNotReady=(CatchTraySuck.Item[0][0]==2 &&
                                         (MOT[MMColor].fHasTray || MOT[MMColorZ].fHasTray || MOT[MMColor_Car].fHasTray || bLoadNewColorTrayToCarStart || bUnLoadNewColorToStackStart));
                    bEmptyStackNotReady=(CatchTraySuck.Item[0][0]==1 &&
                                         (MOT[MMEmpty].fHasTray || MOT[MMEmptyZ].fHasTray || MOT[MMEmpty_Car].fHasTray || bLoadNewEmptyTrayToCarStart || bUnLoadNewEmptyToStackStart));

                    if(bColorStackNotReady || bEmptyStackNotReady)
                    {
                        if(DoPlaceToBufferTimeOut.Off())
                        {
                            if(bColorStackNotReady)
                            {
                                str1.sprintf("%d, %s, %s, %s, %s, %s", CatchTraySuck.Item[0][0],
                                                                        BoolToStr(MOT[MMColor].fHasTray),
                                                                        BoolToStr(MOT[MMColorZ].fHasTray),
                                                                        BoolToStr(MOT[MMColor_Car].fHasTray),
                                                                        BoolToStr(bLoadNewColorTrayToCarStart),
                                                                        BoolToStr(bUnLoadNewColorToStackStart));
                            }
                            else
                            {
                                str1.sprintf("%d, %s, %s, %s, %s, %s", CatchTraySuck.Item[0][0],
                                                                        BoolToStr(MOT[MMEmpty].fHasTray),
                                                                        BoolToStr(MOT[MMEmptyZ].fHasTray),
                                                                        BoolToStr(MOT[MMEmpty_Car].fHasTray),
                                                                        BoolToStr(bLoadNewEmptyTrayToCarStart),
                                                                        BoolToStr(bUnLoadNewEmptyToStackStart));
                            }
                            NewRecordProcess("", "DoPlaceToBuffer Time Out 2260", str1);
                            DoPlaceToBufferTimeOut.SetSecAndOn(300.0);
                        }
                        break;
                    }
                }
            }
            #endif
            //<==
            //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)

            Task=2270;

            if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                Task=2280;
                InitialPlaceBufferTray();
                break;
            }
        case 2270:
            if(Cylinder[C_TrayX_UpDown].Push())
            {
                Task=2300;
                if(IniConfig.bSPILFunction &&
                   IniConfig.bC03UseCatchTray &&
                   IniConfig.bA10_AutoReTest)                                                                           //JerryYang 20220923 : for矽品ART TRAY ARM上下汽缸調整較慢，下降delay time太短會alarm
                {
                    DoPlaceToBufferDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT+1.5);                              //Steven 20110720 : 等待Hand Down Time
                }
                else
                {
                    DoPlaceToBufferDelay.SetSecAndOn(ArmSpeed[TrayArm].dTAHeadDownIT);                                  //Steven 20110720 : 等待Hand Down Time
                }

                if(USE_AUTO_RETEST==eartInstall)                                                                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                    C_CatchTray_Fix_Pop(true);
            }
            break;
        case 2280:
            if(CatchTraySuck.Item[0][0]==1)
                ret=0;
            else if(CatchTraySuck.Item[0][0]==2)
                ret=1;

            if(DoPlaceBufferTray(ret))
            {
                Task=2300;
            }
            break;
        case 2300:
            if(DoPlaceToBufferDelay.Off())                                      //Steven 20110720 : 等待Hand Down Time
            {
                if(IniConfig.bC03UseCatchTray)
                {
                    if(LastSet.iRealDummy==DUMMY)                               //Steven 20120417 : Dummy不要夾起來
                    {
                        Task=2350;
                    }
                    else
                    {
                        if(USE_AUTO_RETEST==eartInstall &&
                           (USE_CATCH_TRAY_MODEL==3 ||                          //Steven 20170623 (wei) : Add for catch tray with cover
                            Cylinder[C_TrayCover].Enable))                      //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                        {
                            if(C_CatchTray_Fix_Pop())
                            {
                                Cylinder[C_TrayCover].Off();                    //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                                Task=2350;
                            }
                            else
                            {
                                break;                                          //Sam 20250324 : 修正夾爪還沒完成就一直重複Delay
                            }
                        }
                        else
                        {
                            if(Cylinder[C_CatchTray_Fix].Pop())
                            {
                                Task=2350;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    CatchTraySuck.Suck[0][0].Off();
                    Task=2350;
                }
                DoPlaceToBufferDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);   //Steven 20110720 : 等待Counter Air On Time
            }
            break;
       case 2350:
            if(DoPlaceToBufferDelay.Off())
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                {
                    CatchTraySuck.Suck[0][0].Normal();                                                                                                          //Steven 20110720 : 等升上來才關閉吹氣
                    bEmptyColorFullTray=false;
                    Task=2351;
                    break;
                }
            }
            break;
        case 2351:
            if(LastSet.iRealDummy!=DUMMY)                                                                                                                       //color  kevin 20120914
            {
                if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                                                                                               //kevin 201250718 load 空tray放auto 1
                {
                    Cylinder[C_Auto1Side_Fixer].On();
                }
                else if(CatchTraySuck.Item[0][0]==2 ||
                        IniConfig.bP04ColorIsEmptyUnloader)                                                                                                     // pick from color
                {
                    if(AUTO_EMPTY_COLOR==0)                                                                                                                     //使用四軌
                    {
                        if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&
                           IniConfig.bP32EmptyColorTrayPreAlarm==true)                                                                                          //Ifor 20170421 (wei) add 四軌 Color Tray Pre Alarm
                        {
                            if(Sen[SnEmptyTrayIsFull2].IsOn()==true)
                            {
                                if(bHascolorTrayPreAlarm==false)
                                {
                                    bHascolorTrayPreAlarm=true;
                                    if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)                                                                                             //Alarm 前若form有開啟先關閉  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                        MyMessageBox->Close();
                                    iUnLoaderCount=8;                                                                                                           // 必須不為0 Handler才不停機
                                    str1.sprintf("Color Tray is full with trays, Please take it off");
                                    str2.sprintf("Color上的Tray盤已滿,請取下Tray盤");
                                    RunInfo.iPreAlarmPosition=9;                                                                                                //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color 10:Auto4 11:Auto5 12:Auto6 13:Fix4 14:Fix5 15:Fix6
                                    ShowUnloaderTrayMessage(str1, str2);
                                }
                            }
                            else
                            {
                                bHascolorTrayPreAlarm=false;
                            }
                        }
                        else
                        {
                            if(Sen[SnEmptyTrayIsFull2].IsOn())                                                                                                  //color  kevin 20120914
                            {
                                ShowErrorMessage("MES1420", K_RETRY, MMColor, false, "DoPlaceToBuffer");
                                break;
                            }
                            bHascolorTrayPreAlarm=false;
                        }

                        if(Sen[SnEmptyTrayHasTray2].IsOff())                                                                                                    //沒有偵測到放tray
                        {
                            ret=ShowErrorMessage("JAM1411", K_RETRY, MMColor, bColorHasDuplicateError, __FUNC__);                                               //Steven : 20120925 JAM1409 -> JAM1411
                            bColorHasDuplicateError=true;                                                                                                       //Steven 20120220 : Color Tray重複Alarm Flag
                            if(ret==K_RETRY)
                            {
                                Task=2251;
                                break;
                            }
                        }
                    }
                    else
                    {
                        Cylinder[C_Color_Fix].On();
                    }
                }
                else
                {
                    if(AUTO_EMPTY_COLOR==0)                                                                                                                     //使用四軌
                    {
                        if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                                                                                      //Ifor 20170421 (wei) add 四軌 Empty Tray Pre Alarm
                           IniConfig.bP32EmptyColorTrayPreAlarm==true)
                        {
                            if(Sen[SnEmptyTrayIsFull1].IsOn()==true)
                            {
                                if(bHasEmptyTrayPreAlarm==false)
                                {
                                    bHasEmptyTrayPreAlarm=true;
                                    if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)                                                                                             //Alarm 前若form有開啟先關閉  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                        MyMessageBox->Close();
                                    iUnLoaderCount=8;                                                                                                           // 必須不為0 Handler才不停機
                                    str1.sprintf("Empty Tray is full with trays, Please take it off");
                                    str2.sprintf("Empty 上的Tray盤已滿,請取下Tray盤");
                                    RunInfo.iPreAlarmPosition=8;                                                                                                //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color 10:Auto4 11:Auto5 12:Auto6 13:Fix4 14:Fix5 15:Fix6
                                    ShowUnloaderTrayMessage(str1, str2);
                                }
                            }
                            else
                            {
                                bHasEmptyTrayPreAlarm=false;
                            }
                        }
                        else
                        {
                            if(IniConfig.bP38UseEmptyFullPutColor==false && Sen[SnEmptyTrayIsFull1].IsOn())
                            {
                                ret=ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoPlaceToBuffer");
                                if(ret==K_RETRY)
                                {
                                    bEmptyColorFullTray=true;
                                }
                                break;
                            }
                            bHasEmptyTrayPreAlarm=false;
                        }

                        if(Sen[SnEmptyTrayHasTray1].IsOff())                                                                                                    //沒有偵測到放tray
                        {
                            if(bEmptyColorFullTray==false)
                            {
                                ret=ShowErrorMessage("JAM1011", K_RETRY, MMEmpty, bEmptyHasDuplicateError, __FUNC__);                                           //Steven : 20120925 JAM1009 -> JAM1011
                                bEmptyHasDuplicateError=true;                                                                                                   //Steven 20120220 : Empty Tray重複Alarm Flag
                                if(ret==K_RETRY)
                                {
                                    Task=2251;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        Cylinder[C_Empty_Fix].On();
                    }
                }
            }
            Task=2352;
            //DoPlaceToBufferDelay.SetSecAndOn(ArmSpeed[TrayArm].dCTAirOn);     //Sam 20250324 : Mark  //Steven 20110720 : 等待Counter Air On Time
            break;
        case 2352:
            //if(DoPlaceToBufferDelay.Off())                                    //Sam 20250324 : Mark
            {
                if(LastSet.iRealDummy!=DUMMY)                                                                                                                   //color  kevin 20120914
                {
                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                                                                                           //kevin 201250718 load 空tray放auto 1
                    {
                        if(Sen[SnAuto1TrayDetect].IsOff())                                                                                                      //沒有偵測到放tray
                        {
                            ret=ShowErrorMessage("JAM1111", K_RETRY, MMAuto1, bAuto1HasDuplicateError, __FUNC__);                                               //Steven : 20131226 JAM1411 -> JAM1111
                            bAuto1HasDuplicateError=true;                                                                                                       //Steven 20120220 : Auto1 Tray重複Alarm Flag
                            Cylinder[C_Auto1Side_Fixer].Off();
                            if(ret==K_RETRY)
                            {
                                Task=2251;
                                break;
                            }
                        }
                        else
                        {
                            Task=2600;
                        }
                    }
                    else if(CatchTraySuck.Item[0][0]==2 ||
                            IniConfig.bP04ColorIsEmptyUnloader)                                                                                                 // pick from color
                    {
                        if(AUTO_EMPTY_COLOR==0)                                                                                                                 //使用四軌
                        {
                            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                                                                                  //Ifor 20170421 (wei) add 四軌 Empty Tray Pre Alarm
                               IniConfig.bP32EmptyColorTrayPreAlarm==true)
                            {
                                if(Sen[SnEmptyTrayIsFull2].IsOn()==true)
                                {
                                    if(bHascolorTrayPreAlarm==false)
                                    {
                                        bHascolorTrayPreAlarm=true;
                                        if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)                                                                                         //Alarm 前若form有開啟先關閉  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                            MyMessageBox->Close();
                                        iUnLoaderCount=8;                                                                                                       // 必須不為0 Handler才不停機
                                        str1.sprintf("Color Tray is full with trays, Please take it off");
                                        str2.sprintf("Color上的Tray盤已滿,請取下Tray盤");
                                        RunInfo.iPreAlarmPosition=9;                                                                                            //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color
                                        ShowUnloaderTrayMessage(str1, str2);
                                    }
                                }
                                else
                                {
                                    bHascolorTrayPreAlarm=false;
                                }
                            }
                            else
                            {
                                if(Sen[SnEmptyTrayIsFull2].IsOn())                                                                                              //color  kevin 20120914
                                {
                                    ShowErrorMessage("MES1420", K_RETRY, MMColor, false, "DoPlaceToBuffer");
                                    break;
                                }
                                bHascolorTrayPreAlarm=false;
                            }

                            if(Sen[SnEmptyTrayHasTray2].IsOff())                                                                                                //沒有偵測到放tray
                            {
                                ret=ShowErrorMessage("JAM1411", K_RETRY, MMColor, bColorHasDuplicateError, __FUNC__);                                           //Steven : 20120925 JAM1409 -> JAM1411
                                bColorHasDuplicateError=true;                                                                                                   //Steven 20120220 : Color Tray重複Alarm Flag
                                Cylinder[C_Color_Fix].Off();
                                if(ret==K_RETRY)
                                {
                                    Task=2251;
                                    break;
                                }
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                        else
                        {
                            if(Sen[SenColorHasTray].IsOff())                                                                                                    //沒有偵測到放tray
                            {
                                ret=ShowErrorMessage("JAM1411", K_RETRY, MMColor, bColorHasDuplicateError, __FUNC__);                                           //Steven : 20120925 JAM1409 -> JAM1411
                                bColorHasDuplicateError=true;                                                                                                   //Steven 20120220 : Color Tray重複Alarm Flag
                                Cylinder[C_Color_Fix].Off();
                                if(ret==K_RETRY)
                                {
                                    Task=2251;
                                    break;
                                }
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                    }
                    else
                    {
                        if(AUTO_EMPTY_COLOR==0)                                                                                                                 //使用四軌
                        {
                            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                                                                                  //Ifor 20170421 (wei) add 四軌 Empty Tray Pre Alarm
                               IniConfig.bP32EmptyColorTrayPreAlarm==true)
                            {
                                if(Sen[SnEmptyTrayIsFull1].IsOn()==true)
                                {
                                    if(bHasEmptyTrayPreAlarm==false)
                                    {
                                        bHasEmptyTrayPreAlarm=true;
                                        if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)                                                                                         //Alarm 前若form有開啟先關閉  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                                            MyMessageBox->Close();
                                        iUnLoaderCount=8;                                                                                                       // 必須不為0 Handler才不停機
                                        str1.sprintf("Empty Tray is full with trays, Please take it off");
                                        str2.sprintf("Empty 上的Tray盤已滿,請取下Tray盤");
                                        RunInfo.iPreAlarmPosition=8;                                                                                            //Ifor 20171024 (wei) : add Pre Alarm Position 1:LD 2:Auto1 3:Auto2 4:Auto3 5:Fix1 6:Fix2 7:Fix3 8:Empty 9:Color
                                        ShowUnloaderTrayMessage(str1, str2);
                                    }
                                }
                                else
                                {
                                    bHasEmptyTrayPreAlarm=false;
                                }
                            }
                            else
                            {
                                if(IniConfig.bP38UseEmptyFullPutColor==false && Sen[SnEmptyTrayIsFull1].IsOn())
                                {
                                    ret=ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoPlaceToBuffer");
                                    if(ret==K_RETRY)
                                    {
                                        bEmptyColorFullTray=true;
                                    }
                                    break;
                                }
                                bHasEmptyTrayPreAlarm=false;
                            }

                            if(Sen[SnEmptyTrayHasTray1].IsOff())                                                                                                //沒有偵測到放tray
                            {
                                if(bEmptyColorFullTray==false)
                                {
                                    ret=ShowErrorMessage("JAM1011", K_RETRY, MMEmpty, bEmptyHasDuplicateError, __FUNC__);                                       //Steven : 20120925 JAM1009 -> JAM1011
                                    bEmptyHasDuplicateError=true;                                                                                               //Steven 20120220 : Empty Tray重複Alarm Flag
                                    Cylinder[C_Empty_Fix].Off();
                                    if(ret==K_RETRY)
                                    {
                                        Task=2251;
                                        break;
                                    }
                                }
                                else
                                {
                                    Task=2600;
                                }
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                        else
                        {
                            if(Sen[SenEmptyHasTray].IsOff())                                                                                                    //沒有偵測到放tray
                            {
                                ret=ShowErrorMessage("JAM1011", K_RETRY, MMEmpty, bEmptyHasDuplicateError, __FUNC__);                                           //Steven : 20120925 JAM1009 -> JAM1011
                                bEmptyHasDuplicateError=true;                                                                                                   //Steven 20120220 : Empty Tray重複Alarm Flag
                                Cylinder[C_Empty_Fix].Off();
                                if(ret==K_RETRY)
                                {
                                    Task=2251;
                                    break;
                                }
                            }
                            else
                            {
                                Task=2600;
                            }
                        }
                    }
                }
                else
                {
                    Task=2600;
                }
            }
            break;
        case 2500:
            if(Cylinder[C_TrayX_UpDown].Pop())                                                                                                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=2600;
            else
                break;
        case 2600:
            if(MTrayXCanSafeMove()==false)                                                                                                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=2500;
                break;
            }

            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                                                                                                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))                                                                                             //wei 20150925 待機位置改道 Color
                TrayArmMotorMove(Prod.iXTrayColor);
            else
                TrayArmMotorMove(Prod.iXTrayEmpty);

            if(CatchTraySuck.iWhichTray!=-1)                                                                                                                    //Steven 20220331 : 紀錄Tray從哪來的
                str1.sprintf("DoPlaceToBuffer - From %s", sTrayPosName[CatchTraySuck.iWhichTray]);
            else
                str1.sprintf("DoPlaceToBuffer - From Unkonwn");

            if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                                                                                                   //kevin 201250718 load 空tray放auto2
            {
                bAuto1HasDuplicateError=false;                                                                                                                  //Steven 20120220 : Empty Tray重複Alarm Flag
                LastSet.iUnloaderTrayCount_ART[1]++;
                if(CosFunction.bLoaderTrayToAuto1)
                {
                    MOT[MMAuto1].InitNewTray(NULL_IC, false, __FUNC__);
                    MOT[MMAuto1].fHasTray=true;
                    SwapTrayID(true, etAuto1);
                    NewRecordProcess("MES0673", "Tray Arm - place tray to Auto 1", str1);                                                                       //Steven 20220331 : Tray Arm動作加上紀錄
                }
                else
                {
                    MOT[MMAuto2].InitNewTray(NULL_IC, false, __FUNC__);
                    MOT[MMAuto2].fHasTray=true;
                    SwapTrayID(false, etAuto2);
                    NewRecordProcess("MES0674", "Tray Arm - place tray to Auto 2", str1);                                                                       //Steven 20220331 : Tray Arm動作加上紀錄
                }
            }
            else
            {
                if(IniConfig.bP04ColorIsEmptyUnloader)
                {
                    bColorHasDuplicateError=false;                                                                                                              //Steven 20120220 : Color Tray重複Alarm Flag
                    LastSet.iColorTrayCount_ART++;
                    MOT[MMColor].InitNewTray(NULL_IC, false, __FUNC__);
                    MOT[MMColor].fHasTray=true;
                    SwapTrayID(false, etColor);
                    iReceiveColorTray=1;
                    Cylinder[C_Color_Fix].Off();                                                                                                                //kevin 20150606
                    NewRecordProcess("MES0672", "Tray Arm - place tray to Color", str1);                                                                        //Steven 20220331 : Tray Arm動作加上紀錄
                }
                else
                {
//                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)           //jou 2014-06-10 修正Catch Tray放置Color Tray時設置錯的Tray 位置
                    if(CatchTraySuck.Item[0][0]==1)                                                                                                             //Steven 20260204 : fixed for miss set data
                    {
                        bEmptyHasDuplicateError=false;                                                                                                          //Steven 20120220 : Empty Tray重複Alarm Flag
                        LastSet.iEmptyTrayCount_ART++;
                        MOT[MMEmpty].InitNewTray(NULL_IC, false, __FUNC__);
                        MOT[MMEmpty].fHasTray=true;
                        SwapTrayID(false, etEmpty);
                        fEmptyCanSupplyNewTray=true;
                        Cylinder[C_Empty_Fix].Off();
                        NewRecordProcess("MES0671", "Tray Arm - place tray to Empty", str1);                                                                    //Steven 20220331 : Tray Arm動作加上紀錄
                    }
                    else
                    {
                        bColorHasDuplicateError=false;
                        LastSet.iColorTrayCount_ART++;
                        MOT[MMColor].InitNewTray(NULL_IC, false, __FUNC__);
                        MOT[MMColor].fHasTray=true;
                        SwapTrayID(false, etColor);
                        fColorCanSupplyNewTray=true;
                        Cylinder[C_Color_Fix].Off();
                        //==> Eastsun 20260515 F018 整合:KYEC Magazine Tray Arm Pick From Color
                        if(CUSTOMER_CODE==CC_KYEC_LEE && AUTO3_IS_MAGAZINE==1 && CatchTraySuck.iWhichKit==2)
                        {
                            if(MOT[MOutArmX].fCanMove==false || MOT[MOutArmY].fCanMove==false)
                            {
                                MOT[MOutArmX].fCanMove=true;
                                MOT[MOutArmY].fCanMove=true;
                                RecordProcess("Out Arm Can not Move:DoPlaceToBuffer");
                            }
                        }
                        //<== Eastsun 20260515 F018
                        NewRecordProcess("MES0672", "Tray Arm - place tray to Color", str1);                                                                    //Steven 20220331 : Tray Arm動作加上紀錄
                    }
                }
            }

            if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray ||
               IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT)                                                                                                //Sam 20230220 : 移除客戶碼
            {
                if(MOT[MTrayX].Tray.iNeedManualRemoved>=0)                                                                                                      //Sam 20230317 : 修正 P24 功能
                {
                    if(MOT[MTrayX].Tray.iNeedManualRemoved==0)
                        MOT[MMEmpty].Tray.iNeedManualRemoved=0;
                    else
                        MOT[MMColor].Tray.iNeedManualRemoved=1;
                    MOT[MTrayX].Tray.iNeedManualRemoved=-1;
                }
            }

            CatchTraySuck.iWhichTray=-1;                                                                                                                        //Steven 20220331 : 紀錄Tray從哪來的
            CatchTraySuck.SetItemData(0, 0, NULL_IC);
            MOT[MTrayX].fHasTray=false;
            Task=3000;
            break;
        case 2900:
            if(Cylinder[C_TrayX_UpDown].Pop())                                                                                                                  //Steven 20231026 : 針對TrayArm動作加上保護
            {
                //==> Eastsun 20260515 F016 整合:KYEC AMR cover-tray ID dispatch
                if(TrayForm.bEnableAMR && IsLoaderCoverTray())
                    Task=3010;
                else
                //<== Eastsun 20260515 F016
                Task=3000;
            }
            else
            {
                break;
            }
        //==> Eastsun 20260515 F016 整合:KYEC AMR cover-tray ID check chain (3010->3020->3030->3000)
        case 3010:
            Task=3100;
            if((TestIF_File.iAMRLDNowTrayCount-iLoaderTrayCountAMRCal)<3)
            {
                if(TrayForm.bIDTrayOrder[TestIF_File.iAMRLDNowTrayCount-iLoaderTrayCountAMRCal])
                {
                    InitialCheckCoverTrayIDTask();
                    Task=3020;
                }
            }

            if(TrayForm.bEnableAMR)
            {
                if(iLoaderTrayCountAMRCal==TestIF_File.iAMRLDNowTrayCount)
                {
                    TestIF_File.iAMRLDNowTrayCount=TestIF_File.iAMRLDSECSTrayCount;
                    iLoaderTrayCountAMRCal=0;
                }
            }

            if(Task==3100) Task=3000;
            break;
        case 3020:
            if(CheckCoverTrayID())
            {
                Task=3030;
            }
            break;
        case 3030:
            #ifdef SOFT_SIMULTE
            Task=3000;
            break;
            #endif
            if(asColorCoverTrayID!="")
            {
                fLotInfo->labNowLoaderTrayID->Caption=asColorCoverTrayID;
                EventReport(SECS_EVENT.AGVLdID);
                asColorCoverTrayID="";
                Task=3000;
            }
            break;
        //<== Eastsun 20260515 F016
        case 3000:
            if(MTrayXCanSafeMove()==false)                                                                                                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=2900;
                break;
            }

            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                                                                                                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))                                                                                             //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                                                                                           //kevin 201250718 load 空tray放auto 1
                    {
                        MOT[MOutArmX].fCanMove=true;                                                                                                            //OUT ARM 可以移動
                        MOT[MOutArmY].fCanMove=true;
                    }
                    MOT[MInArmX].fCanMove=true;                                                                                                                 //OUT ARM 可以移動
                    MOT[MInArmY].fCanMove=true;
                    iCatchTrayControlManual=0;
                    Task=1;
                    return true;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)                                                                                           //kevin 201250718 load 空tray放auto 1
                    {
                        MOT[MOutArmX].fCanMove=true;                                                                                                            //OUT ARM 可以移動
                        MOT[MOutArmY].fCanMove=true;
                    }
                    iCatchTrayControlManual=0;
                    Task=1;
                    return true;
                }
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer HSlapTrayDelay;
bool DoSlapTray(bool bInit)                                                     //ChungHung 20140701 add AutoRetest
{
    int &Task=iSlapTrayTask;
    int pos=0;
    static int iCoverCount=0;

    if(bInit)
    {
        iSlapTrayTask=1;
        return false;
    }

    switch(Task)
    {
        case 1:
            if(InArmXYZSafe()==true)                                            //Steven 20250311 : 確認Tray能移動
            {
                MOT[MInArmX].fCanMove=false;
                MOT[MInArmY].fCanMove=false;
                Task=5;
            }
            break;
        case 5:
            if(Cylinder[C_TrayX_UpDown].Pop())
                Task=10;
            break;
        case 10:                                                                //確認Tray Arm 朝下
            if(Cylinder[C_TurnTrayArm].Pop())
                Task=20;                                                        //ChungHung 20140814 add for AutoRetest Lock
            break;
        case 20:                                                                //ChungHung 20140814 add for AutoRetest Lock
            if(Cylinder[C_TurnTrayArmLock].Push())
                Task=50;
            break;
        case 50:                                                                                                        //確認翻轉機構 在安全位置
            if(TrayArmMotorMove(Prod.iXTrayClean+IniConfig.iCleanLoaderOffset), false)                                  //Sam 20241206 : Tray Arm 新增 Teach 保護 //wei 20150826 翻轉機構位置修改
            {
                if(Cylinder[C_TurnTrayArmLock].Pop())                                                                   //ChungHung 20140814 add for AutoRetest Lock
                    Task=100;
            }
            break;
        case 100:                                                               //Push 翻轉氣缸
            if(Cylinder[C_TurnTrayArm].Push())
            {
                iCoverCount=0;
                Task=200;
            }
            break;
        case 200:                                                               //Cover 拍拍 5次
            Cylinder[C_TrayCover].Off();
            HSlapTrayDelay.Set0_1SecAndOn(10);
            Task=300;
            //break;
        case 300:
            if(HSlapTrayDelay.Off())
            {
                if(Cylinder[C_TrayCover].Push())
                {
                    iCoverCount++;
                    if(iCoverCount>=IniConfig.iClearLoaderCount)                //wei 20150810 拍拍Tray次數設定
                        Task=400;
                    else
                        Task=200;
                }
            }
            break;
        case 400:
            if(Cylinder[C_TurnTrayArm].Pop())
                Task=450;                                                       //ChungHung 20140814 add for AutoRetest Lock
            break;
        case 450:                                                               //ChungHung 20140814 add for AutoRetest Lock
            if(Cylinder[C_TurnTrayArmLock].Push())
                Task=500;
            break;
        case 500:                                                               //動作完成
            if(MTrayXCanSafeMove()==false)
            {
                Task=300;
                break;
            }

            if(CatchTraySuck.Item[0][0]==3)
            {
                 pos=Prod.iXTrayAuto[0];                                        //kevin 20120726 移到auto 1
            }
            else if(CatchTraySuck.Item[0][0]==2)
            {
                pos=Prod.iXTrayColor;
            }
            else
            {
                 pos=Prod.iXTrayEmpty;
            }

            if(TrayArmMotorMove(pos))
            {
                /*if(LastSet.bHT8040_SACNNER)                                   //Steven 20140409 : Mark Scanner
                {
                    MOT[MOutArmX].fCanMove=true;
                    MOT[MOutArmY].fCanMove=true;
                }
                else  */
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                }
                //Task=1;
                return true;
            }
            break;
    }
    return false;
}
//==============================================================================
// 吸空 TRAY 手臂主流程
//==============================================================================
int CatchTrayTask=1;
void InitialCatchTrayTask()
{
    CatchTrayTask=1;
    bDoTrayDeviceCheck=false;                                                   //wei 20170317 (steven) Device Remain 殘料檢測
}
//------------------------------------------------------------------------------
void DoCatchTray()
{
    static int Target=0, TargetBuf=0, iWhichAuto=0;
    static int iTrayCount=0;
    static bool bLoaderAlreadyRetry=false;

    int ret, &Task=CatchTrayTask;
    int iPosY;
    int ipos=0;                                                                 //wei 20170317 (steven) Device Remain 殘料檢測
    int iPickfrom=1;
    bool flag1, flag2;
    AnsiString sBuffer="";
    AnsiString str="";
    int iBinPos=0;

    if(iPauseBackUp!=-1 &&
       IniConfig.bC03UseCatchTray==false &&                                     //Steven 20220113 : 針對夾Tray加上判斷
       CatchTraySuck.IsShtSuckFinish()==true &&                                 //ChungHung 20110901 add
       CatchTraySuck.IsShtDestroyFinish()==true)
        return;

    if(bEject)                                                                  //JerryYang 20251020 : 渠梁半清機功能
        return;

    QueueTaskList[11].CheckTaskChange();                                        //Steven 20200821 : 使用Goto也要記錄Task變化
    switch(Task)
    {
        case 1:
            iTrayCount++;
            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                  //Ifor 20170419 (wei) add 開啟 Empty/Color Tray PreAlarm 功能不做計數
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
            }
            else
            {
                if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0 &&
                   IniConfig.bP04ColorIsEmptyUnloader==false)
                {
                    if(IniConfig.bP38UseEmptyFullPutColor==false && Sen[SnEmptyTrayIsFull1].IsOn())
                    {
                        if(iTrayCount>=100)
                        {
                            iTrayCount=0;
                            ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoCatchTray");
                        }
                        return;
                    }
                    else if(IniConfig.bP38UseEmptyFullPutColor)
                    {
                        if(Sen[SnEmptyTrayIsFull1].IsOn() && Sen[SnEmptyTrayIsFull2].IsOn())
                        {
                            if(iTrayCount>=100)
                            {
                                iTrayCount=0;
                                ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoCatchTray_1");
                            }
                            return;
                        }
                    }
                }
                else if(TrayForm.LoaderToEmptyColor[iRunStartMode]==1 &&
                        IniConfig.bP04ColorIsEmptyUnloader==true)
                {
                    if(Sen[SnEmptyTrayIsFull2].IsOn())                          //color
                    {
                        if(iTrayCount>=100)
                        {
                            iTrayCount=0;
                            ShowErrorMessage("MES1420", K_RETRY, MMColor, false, "DoCatchTray");
                        }
                        return;
                    }
                }
            }

            iTrayCount=0;
            if(TRAY_ARM_MODE==eUnderCoveyor)                                    //Frank 20230425
            {
                Task=50;
            }
            else
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                {
                    Task=10;
                }
            }
            break;
        case 10:
            bIsPlacingToBuffer=false;                                           //JerryYang 20250828 : fix color誤退TRAY
            bIsCatchingFromBuffer=false;                                        //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中
            if(MTrayXCanSafeMove()==false || bOutarmAutoHigh)                   //kevin 20171115 (wei) add Autoteach outarm Z
            {
                Task=25;
            }
            else
            {
                if(IniConfig.bP56TrayArmWaitAtColorTrack ||                     //Steven 20240516 : Tray Arm等待位置改到Color
                   (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))         //wei 20150925 待機位置改道 Color
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                        Task=20;
                }
                else
                {
                    if(TrayArmMotorMove(Prod.iXTrayEmpty))
                        Task=20;
                }
            }
            break;
        case 15:
            if(Cylinder[C_TrayX_UpDown].Pop())
                Task=10;
            break;
        case 20:
            if(CatchTraySuck.Suck[0][0].Enable &&
               IniConfig.bC03UseCatchTray==false)                               //jou 2013-07-19 夾Tray不用確認吸嘴Sensor
            {
                flag1=CatchTraySuck.Suck[0][0].GetStatus();

                if(MOT[MTrayX].fHasTray)
                    flag2=true;
                else
                    flag2=false;

                if(flag1!=flag2)
                {
                    if(Cylinder[C_TrayX_UpDown].Pop())
                    {
                        if(flag1)
                            Task=30;
                        else
                            Task=40;
                    }
                    break;
                }
            }
            DoSlapTray(true);                                                   //ChungHung 20140701 add Auto Retest
            Task=50;
            break;
        case 30:
            ShowErrorMessage("WAR0611", K_RETRY, MTrayX, false, "DoCatchTray_30");
            Task=10;
            break;
        case 40:
            if(ShowErrorMessage("WAR0611", K_SKIP|K_RETRY, MTrayX, false, "DoCatchTray_40")==K_SKIP)
            {
                CatchTraySuck.iWhichTray=-1;                                    //Steven 20220331 : 紀錄Tray從哪來的
                CatchTraySuck.SetItemData(0, 0, NULL_IC);
                MOT[MTrayX].fHasTray=false;
                MOT[MMTrayY_Car].fCanMove=true;
                fEmptyCanSupplyNewTray=true;                                    //Steven 20110406
            }
            Task=10;
            break;
        case 50:
            if(MOT[MTrayX].fHasTray)
            {
                if(bNeedSlapTray)                                               //ChungHung 20140701 add Auto Retest
                {
                    if(DoSlapTray())
                    {
                        bNeedSlapTray=false;
                    }
                }
                else
                {
                    Task=1000;                                                  // check hear,because color tray check
                }
            }
            else
            {
                Task=100;
            }
            break;
        case 100:
//            iPosX=MOT[MInArmX].ReadPos();                                     //Steven 20241018 : 安全點不需要比較X軸
            iPosY=MOT[MInArmY].ReadPos();

            if(CUSTOMER_CODE==CC_Greatek || IniConfig.bSIGURDFunction)                                                  //Sam 20201223 : 暫時 Mark 確認問題
            {
            }
            else
            {
                if(((TrayForm.iManualRemoveLoader!=0 &&
                     bLoaderHasSkip==true) ||                                                                           //Ifor 20200422 : add 編輯Tray需手動移除Tray
                   bNeedManualRemoveTray==true) &&                                                                      //Ifor 20191022 : add 避免InArm因移動速度慢導致未移動至SH2就停下來
                   iInArmWaitPosition==3)                                                                               //Ifor 20191105 : add InArm 安全位置判斷
                {
                    if(                                                                                                 //iPosX>Prod.XInArm_Shuttle2_Place[iInArmYBase][iInArmXBase]      || //Steven 20241018 : 安全點不需要比較X軸
                       iPosY<Prod.YInArm_Shuttle2_Place[iInArmYBase][iInArmXBase]-3000)
                    {
                        break;
                    }
                }
            }

            if(MOT[MMTrayY].Tray.HasIC()==false &&
               MOT[MMTrayY].fHasTray==true &&
               bPlaceToHotplate==false &&                                                                               //jou 20180329 (Steven) : 放hotplate位置導致catch tray 與 in arm互卡 hang up
               InArmXYZSafe()==true)                                                                                    //Steven 20250311 : 確認Tray能移動
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)                                                                        //jou 2011-05-31 start : 需要放在前面,不然會被WhichAutoNeedTray跳到300 hang up.
                {                                                                                                       //jou 20180329 (Steven) : 放hotplate位置導致catch tray 與 in arm互卡 hang up
                    MOT[MInArmX].PCIL132_StopMotor();
                    MOT[MInArmY].PCIL132_StopMotor();
                    MOT[MInArmX].fCanMove=false;
                    MOT[MInArmY].fCanMove=false;
                }

                if(iAse_LoadTrayEndTotalTime==1)                                                                        //kevin 20170418 (wei) 有吸到空位置
                    iAse_LoadTrayEndTotalTime=2;                                                                        //kevin 20170320 (wei) 使用AUTO TRAY END 時間記數

                if(IniConfig.bE62TryPickLastRow &&
                   TrayForm.Loader.Direction!=iBackupLoadDir)                                                           //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
                {
                    TrayForm.Loader.Direction=iBackupLoadDir;
                }

                if(fTrayMapping->IsRunDeviceRemainLaser() &&
                   fTrayMapping->bDoTrayDeviceCheckFromLoader==true)                                                    //JimmyChiu 20220219 : 殘料檢查 From Loader
                {
                    Task=140;
                    break;
                }

                if(CosFunction.bOneByOneWhenPickErrAtLoader &&                                                          //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
                   bNeedOneByOnePickInArm)
                {
                    ArmSpeed_File[InArm].bVariModeFIX=IniConfig.bBackUpInArmMode;
                    ArmSpeed[InArm].bVariModeFIX=IniConfig.bBackUpInArmMode;
                    bNeedOneByOnePickInArm=false;
                }

                bTryPickLastRowIng=false;
                bASkStart=false ;                                                                                       //kevin 20170427 (wei) AutoSkip 換TRAY 需清為0
                if((bCanRunSCKART==true ||                                                                              //Steven 20161201 (wei) : For SCK 93K ART
                    IniConfig.bA65_BundleIDList ||
                    IniConfig.bP57LoaderAutoCleanOutByInputCT) &&                                                       //Sam 20250605 : Loader Count AutoCleanOut
                   bLoaderHasSkip==true)
                {
                    bool bCountOK=(LastSet.SendCT[0]==LastSet.iP57_InputCT && LastSet.iP57_InputCT!=0)?true:false;      //Sam 20250605 : Loader Count AutoCleanOut
                    if(bLoaderHasSuck &&
                       (LastSet.iSCKARTInputCT==fSCKART->iInputCount ||                                                 //Steven 20170126 (wei) : 當入料數量少於設定值 //Steven 20170327 : Fixed for ART count
                        bCountOK))
                    {
                        SetAutoSkipCount(0);                                                                            //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
                        bLoaderAlreadyRetry=false;
                        ret=K_TRAY_END;                                                                                 //Steven 20170720 (wei) : 最後說都不要叫了
                        #ifndef SOFT_SIMULTE
                        bFirstRecordLoaderData=false;                                                                   //Steven 20141019 : Tray End也要重置Auto Skip狀態
                        #endif
                        bLoaderHasSkip=false;                                                                           //Steven 20141019 : Tray End也要重置Auto Skip狀態
                        bNeedManualRemoveTray=false;                                                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                        Task=200;
                        break;
                    }
                    else
                    {
                        if(TrayForm.iManualRemoveLoader!=0 &&
                           bLoaderHasSkip==true)
                        {
                            if(TestIF_File.bRENESAS_EnableFTCT==true)                                                   //RogerYang 20251021 : 瑞薩FTCT模式下，數量不同，且Loader已無tray還是要跳
                            {
                                if(LastSet.iSCKARTInputCT==fSCKART->iInputCount)
                                    Task=200;
                                else
                                    Task=3000;
                            }
                            else
                            {
                                Task=3000;
                            }
                        }
                        else
                        {
                            Task=200;
                        }
                        break;
                    }
                }
                else if(ArmSpeed[InArm].bAutoSKIP==true &&
                        IniConfig.bRecordSkipPosition==true &&
                        TrayForm.iManualRemoveLoader==0 &&
                        bFirstRecordLoaderData==true &&
                        bLoaderHasSkip==true &&
                        CUSTOMER_CODE!=CC_ASE_KaohSiung &&                                                              //kevin 20170209 (wei) add 高雄不發alarm
                        CUSTOMER_CODE!=CC_TERAPOWER)                                                                    //Sam 20240304 : 新增 AMR 功能
                {
                    SetAutoSkipCount(0);                                                                                //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
                    bLoaderAlreadyRetry=false;
                    ret=ShowErrorMessage("MES0101", K_RETRY|K_TRAY_END, MInArmX, false, "DoCatchTray");                 //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home start

                    if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray==true ||                                      //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查
                       IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT)                                                //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
                    {
                        if(CosFunction.bSpecialP24)                                                                     //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                        {
                            MOT[MMTrayY].Tray.iNeedManualRemoved=1;                                                     //to color alarm
                        }
                        else
                        {
                            if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                                MOT[MMTrayY].Tray.iNeedManualRemoved=0;                                                 //to empty alarm
                            else
                                MOT[MMTrayY].Tray.iNeedManualRemoved=1;                                                 //to color alarm
                        }
                    }

                    if(ret==K_RETRY)
                    {
                        #ifndef SOFT_SIMULTE
                        bFirstRecordLoaderData=false;
                        #endif
                        for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                        {
                            for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                            {
                                if(MOT[MMTrayY].Tray.BufferData[i][j]==HAS_SKIP_IC)
                                {
                                    MOT[MMTrayY].SetTraySingleData(i, j, HAS_IC);
                                    MOT[MMTrayY].SetTrayBufferSingleData(i, j, NULL_IC);
                                }
                                else
                                {
                                    MOT[MMTrayY].SetTraySingleData(i, j, NULL_IC);
                                }
                            }
                        }

                        MOT[MMTrayY_Car].fCanMove=true;
                        MOT[MInArmX].fCanMove=true;
                        MOT[MInArmY].fCanMove=true;
                        bLoaderHasSkip=false;
                        bNeedManualRemoveTray=false;                                                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                        if(CosFunction.bShowHPICCount)                                                                  //Steven 20221228 : 計算加熱盤IC數量
                        {
                            fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                        }
                        Task=1;
                        break;
                    }
                    else
                    {
                        #ifndef SOFT_SIMULTE
                        bFirstRecordLoaderData=false;                                                                   //Steven 20141019 : Tray End也要重置Auto Skip狀態
                        #endif
                        bLoaderHasSkip=false;                                                                           //Steven 20141019 : Tray End也要重置Auto Skip狀態
                        bNeedManualRemoveTray=false;                                                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                        Task=200;
                        break;
                    }
                }
                else if(IniConfig.bE63RetryPickLoader &&                                                                //JerryYang 20200422 改成E63功能選項
                        ArmSpeed[InArm].bAutoSKIP==true &&
                        IniConfig.bRecordSkipPosition==true &&                                                          //JerryYang 20200206 數量小於5 ea要自動回去吸
                        TrayForm.iManualRemoveLoader==1 &&
                        bFirstRecordLoaderData==true &&
                        bLoaderHasSkip==true &&
                        MOT[MMTrayY].Tray.HowManyICInBuffer(HAS_SKIP_IC)>0 &&
                        MOT[MMTrayY].Tray.HowManyICInBuffer(HAS_SKIP_IC)<=5 &&
                        bLoaderAlreadyRetry==false)
                {
                    SetAutoSkipCount(0);                                                                                //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
                    bLoaderAlreadyRetry=true;
                    bPickUpHomeFinish=false;
                    for(int i=0; i<MOT[MMTrayY].Tray.XItem; i++)
                    {
                        for(int j=0; j<MOT[MMTrayY].Tray.YItem; j++)
                        {
                            if(MOT[MMTrayY].Tray.BufferData[i][j]==HAS_SKIP_IC)
                            {
                                MOT[MMTrayY].SetTraySingleData(i, j, HAS_IC);
                                MOT[MMTrayY].SetTrayBufferSingleData(i, j, NULL_IC);
                            }
                            else
                            {
                                MOT[MMTrayY].SetTraySingleData(i, j, NULL_IC);
                            }
                        }
                    }

                    if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray==true ||                                      //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查
                       IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT)                                                //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
                    {
                        if(CosFunction.bSpecialP24)                                                                     //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                        {
                            MOT[MMTrayY].Tray.iNeedManualRemoved=1;                                                     //to color alarm
                        }
                        else
                        {
                            if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                                MOT[MMTrayY].Tray.iNeedManualRemoved=0;                                                 //to empty alarm
                            else
                                MOT[MMTrayY].Tray.iNeedManualRemoved=1;                                                 //to color alarm
                        }
                    }

                    if(CosFunction.bShowHPICCount)                                                                      //Steven 20221228 : 計算加熱盤IC數量
                    {
                        fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
                    }

                    MOT[MMTrayY_Car].fCanMove=true;
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    Task=1;
                    break;
                }
                else if((TrayForm.iManualRemoveLoader!=0 &&
                         bLoaderHasSkip==true)           ||                                                             //Steven 20101019
                        (USE_TRAY_MAPPING!=etmUninstall &&
                                             TestIF_File.bEnableTrayMap &&
                                             TestIF_File.bEnableSuckMapCheck && bCheckTrayDevice) ||                    //wei 20170302 (Steven) Tray map data error
                        (USE_TRAY_MAPPING!=etmUninstall &&
                                             TestIF_File.bEnableDeviceRemain && bTrayHaveDevice)  ||                    //wei 20170317 (steven) Device Remain 殘料檢測
                        bNeedManualRemoveTray==true                                                )                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                {
                    bLoaderAlreadyRetry=false;
                    Task=3000;
                    break;
                }
                else
                {
                    bLoaderAlreadyRetry=false;
                    bAutoSkipCntOver=false;                                                                             //JerryYang 20210420 : 矽品蘇州要求auto skip數量小於設定值要由工程師排除
                    Task=200;
                }
            }
            else
            {
                if(USE_LdUldCassetteMode==1)                                                                            //Frank 20251217 add
                {
                    Target=WhichAutoNeedTray();
//                    InitDoCassetteToAutoTask(Target);
//                    Task=450;
                    if(Target!=0)                                                                                       //RogerYang 20260203 : Add for HT9046CR
                    {
                        Task=1100;
                    }
                }
                else
                {
                    Task=300;
                }
            }
            break;
        case 140:                                                               //JimmyChiu 20220219 : 殘料檢查 From Loader
            if(fTrayMapping->IsRunDeviceRemainLaser() &&
               fTrayMapping->bDoTrayDeviceCheckFromLoader)                      //JimmyChiu 20220219 : 殘料檢查 From Loader
            {
                fTrayMapping->DoLaserScanFromLoader(fTrayMapping->eAOIType_TrayDeviceCheckLaser,true);
                fTrayMapping->bDoTrayDeviceCheckFromLoader=true;
                Task=145;
            }
            else
            {
                Task=100;
            }
            break;
        case 145:                                                               //JimmyChiu 20220219 : 殘料檢查 From Loader
            if(fTrayMapping->IsRunDeviceRemainLaser() &&
               fTrayMapping->bDoTrayDeviceCheckFromLoader)                      //JimmyChiu 20220219 : 殘料檢查 From Loader
            {
                if(fTrayMapping->DoLaserScanFromLoader(fTrayMapping->eAOIType_TrayDeviceCheckLaser, false, true)==true)
                {
                    if(fTrayMapping->asErrorLaserScan!="" ||
                       fTrayMapping->GetTrayDeviceCheckIsRemainIC())
                    {
                        fTrayMapping->yieldRemainIC.AddYieldData(false);
                        if(fTrayMapping->DoAutoSkip(fTrayMapping->eAOIType_TrayDeviceCheckLaser)==true)
                        {
                            Task=100;                                           //pass繼續跑
                            fTrayMapping->bDoTrayDeviceCheckFromLoader=false;
                            break;
                        }
                        else if(fTrayMapping->CheckRunAutoRetry(fTrayMapping->eAOIType_TrayDeviceCheckLaser))
                        {
                            if(fTrayMapping->asErrorLaserScan!="")
                            {
                                sBuffer=fTrayMapping->asErrorLaserScan;
                            }
                            else
                            {
                                sBuffer="Tray has IC";
                            }
                            RecordProcess(AnsiString().sprintf("Auto Retry count:%d ,Error:%s", fTrayMapping->iAutoRetryCount, sBuffer));
                            Task=140;                                           //retry
                            break;
                        }
                        else if(fTrayMapping->asErrorLaserScan!="")
                        {
                            ret=ShowErrorMessage("WAR0888", K_RETRY|K_SKIP, MInArmX, 0, __FUNC__);
                            if(ret==K_RETRY)
                            {
                                Task=140;                                       //retry
                                break;
                            }
                            else                                                //K_SKIP
                            {
                            }
                        }
                        else
                        {
                            ShowErrorMessage("WAR0889", 0, MInArmX, 0, __FUNC__);
                        }
                        Task=147;                                               //remove tray
                        fTrayMapping->bDoTrayDeviceCheckFromLoader=false;
                        break;
                    }

                    Task=100;                                                   //正常繼續跑
                    fTrayMapping->bDoTrayDeviceCheckFromLoader=false;
                    fTrayMapping->yieldRemainIC.AddYieldData(true);
                }
            }
            else
            {
                Task=100;
                fTrayMapping->bDoTrayDeviceCheckFromLoader=false;
            }
            break;
        case 147:                                                               //remove tray manually
            if(Sen[SnLoaderSureTray].IsOn())
            {                                                                   //jou 2010-12-20 修正alarm時,就放開汽缸
                Cylinder[C_TrayY_Fixer].Off();                                  //Open Fix Supply Try Fix Cylinder
                Cylinder[C_LoaderEdgePush].Off();
                Cylinder[C_LoaderUpPress].Off();                                //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray

                if(IniConfig.bKoreaFunction)                                    //jou 2010-12-15 ASE Korea 要按Skip
                {
                    ShowErrorMessage("MES0922", K_SKIP, MMTrayY, false, "DoCatchTray");
                }
                else
                {
                    ShowErrorMessage("MES0922", K_RETRY, MMTrayY, false, "DoCatchTray");
                }
            }
            else
            {
                Task=3100;
            }
            break;
        case 150:                                                               //kevin 20220401 loadTRAY MAP
            ret=LoadCCD->LoadCCDData();

            if(ret==0)
            {
                Task=200;
            }
            else if(tCCDCheckTime.Off() || ret!=999)                            //LOAD EMPTY TRAY 有殘留IC需手動取出
            {
                if(ret!=999)
                    Task=170;
                else
                    Task=160;
            }
            break;
        case 160:                                                               //kevin 20220401 loadTRAY MAP  X Y Wait
            if(MoveInArm2XYToWait())
            {
                bNeedManualRemoveTray=true;
                MOT[MTrayX].PCIL132_StopMotor();
                ret =ShowErrorMessage("WAR09104", K_SKIP|K_RETRY, MMTrayY);
                if(ret == K_RETRY)
                {
                    LoadCCD->InitialLoadCCD();
                    tCCDCheckTime.SetSecAndOn(30);                              //kevin 20220401 LOAD CCD  確認是殘留IC
                    Task=150;
                }
                else
                {
                    Task=3000;
                }
                break;
            }
            break;
        case 170:                                                               //kevin 20220401 loadTRAY MAP  X Y Wait
            if(MoveInArm2XYToWait())
            {
                bNeedManualRemoveTray=true;
                MOT[MTrayX].PCIL132_StopMotor();
                ShowErrorMessage("WAR09105", K_RETRY, MMTrayY);
                Task=3000;
                break;
            }
            break;
        case 200:
            bTrayHaveDevice=false;
            if(USE_TRAY_MAPPING==etmInstall &&
               TestIF_File.bEnableDeviceRemain==true &&                         //wei 20170317 (steven) Device Remain 殘料檢測
               LastSet.iRealDummy!=DUMMY)                                       //Sam 20181204 : 修正空跑時殘料檢測誤報警
            {
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                Task=6000;
                break;
            }
            else
            {
                if(USE_LdUldCassetteMode==1)                                    //Frank 20251217 add
                {
                    InitDoLoadToCassetteTask();
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    Task=240;
                }
                else if(TRAY_ARM_MODE==eUnderCoveyor)
                {
                    InitialCatchUnderTray();
                    Task=260;
                }
                else
                {
                    //==> Eastsun 20260818 : 9050 uses InitialCatchFromLoader_9050() for its own FSM   //AI(W906-F9050-BD) 20261004: H068, 910 acatchtray.cpp:7263-7268 verbatim; today 9050GPIB decodes as Type_HT9046_LS (database.cpp:517) => InitialCatchFromLoader() as before
                    if((MachineTypeChoice==Type_HT9050))
                        InitialCatchFromLoader_9050();
                    else
                    //<== Eastsun 20260818
                    InitialCatchFromLoader();
                    //Task=250;
                    //AI(W906-TIF912) 20260925: golden 912 :6959-6964 (Prevent Drop: wait in 245 while InArm holds a real IC)
                    if(TestIF_File.bPreventDropfunction && InArmSuck.HasRealIC())  //Eastsun 20260710整合
                        Task=245;
                    else
                        Task=250;
                }
                break;
            }                                                                   //RogerYang 20260402 : Add
        case 240:
            if(DoLoadToCassette())                                              //回推                                 //Frank 20251217 add
            {
                MOT[MMTrayY].ClearTray(__FUNC__);
                if(TestIF_File.iCassetteLDNowLayer>UserDefForm_File[0].iCassetteZItem)
                {
                    ShowMyMessage("Loader Cassette Finish ,Please Change New One");
                    TestIF_File.iCassetteLDNowLayer=0;
                }
                bBoatChangeCasset[0]=false;
                Task=1;
            }
            break;
        case 245:                                                               //Eastsun 20260710整合   //AI(W906-TIF912) 20260925: golden 912 :6981-6998, verbatim
            //  Motor calls here (fCanMove / PCIL132_StopMotor on MInArmX/Y) are the
            //  same TMyMotor members cases 170/200 of this switch already drive
            //  live -- no new hardware surface.  Entered only when
            //  TestIF_File.bPreventDropfunction (SingleSite recipes; golden
            //  cSetUp.cpp:2566-2577) and InArmSuck holds a real IC.
            if(TRAY_ARM_MODE==eAboveCoveyor)
            {
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
            }
            if(InArmSuck.HasRealIC()==false)
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MInArmX].PCIL132_StopMotor();
                    MOT[MInArmY].PCIL132_StopMotor();
                    MOT[MInArmX].fCanMove=false;
                    MOT[MInArmY].fCanMove=false;
                }
                Task=250;
            }
            break;
        case 250:
            bCatchTrayFinishAction=true;
            iCatchTrayControlManual=0;
            //==> Eastsun 20260818 : 9050 catch-tray via MLoaderZ + C_MobileTrayTableSelect   //AI(W906-F9050-BD) 20261004: H069, 910 :7311-7316 verbatim; DoCatchFromLoader_9050 returns 0 / 1 / 3, never 2 (no 'fail catch 1 tray' path for 9050, 910 as written)
            if((MachineTypeChoice==Type_HT9050))
                ret=DoCatchFromLoader_9050();
            else
            //<== Eastsun 20260818
            ret=DoCatchFromLoader();
            if(ret==1)                                                                                                  // success catch 1 tray
            {
                NewRecordProcess("MES0650", "Tray Arm - Pick tray from Loader", "DoCatchFromLoader");                   //Steven 20220331 : Tray Arm動作加上紀錄
                if((TrayForm.bAutoFeed==false ||                                                                        //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
                    LastSet.bCleanOut_ART) &&                                                                           //ChungHung 20141002 add for KYEC AutoRetest
                   bLoaderNoTrayAutoCleanOut==true &&
                   MOT[MMTrayY_Car].fHasTray==false)
                {
                    if(IniConfig.bLastLoaderAutoCleanOut==false &&                                                      //Steven 20120207 : 不要自動退Tray
                       IniConfig.bA08LastLoaderAutoCleanOutAndCheckAgain==false)                                        //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                    {
                        ret=ShowErrorMessage("MES0920", K_RETRY|K_CLEAN_OUT, MMTrayY_Car, false, "DoCatchTray_250");    //Steven 20100202
                        if(ret==K_CLEAN_OUT)
                        {
                            fMain->CleanOut("DoCatchTray 1");
                        }
                        else
                        {
                            bSecondTimeLoaderCheckNoTray=false;                                                         //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
                            bLoaderNoTrayAutoCleanOut=false;
                        }
                    }
                    else
                    {
                        fMain->CleanOut("DoCatchTray 2");
                    }
                }

                if(TrayForm.LoaderToEmptyColor[iRunStartMode]==2)
                    Task=2120;
                else
                    Task=2000;
            }
            else if(ret==2)                                                                                             // fail catch 1 tray
            {
                Task=100;
            }
            else if(ret==3)
            {
                Task=5200;                                                                                              //Sam 20200204 : 修正 Hang up
            }
            break;
        case 260:
            if(DoCatchUnderTray(0))
            {
                Task=2000;
            }
            break;
        case 300:                                                               // no tray
            if(TRAY_ARM_MODE==eAboveCoveyor &&
               MTrayXCanSafeMove()==false)
            {
                Task=350;
                break;
            }

            if(USE_TRAY_MAPPING==etmInstall &&
               (TestIF_File.bEnableTrayMap==true ||
                TestIF_File.bEnableTrayID==true) &&
               bLoaderTrayAction==true)                                         //wei 20161219 (Steven) Tray Mapping
            {
                MOT[MInArmX].fCanMove=false;
                MOT[MInArmY].fCanMove=false;
                Task=4000;
            }
            else
            {
                Target=WhichAutoNeedTray();                                     //which auto need tray    //JerryYang 20180927 (Steven) : 避免Tray arm在empty跟color來回跑
                if(USE_TRAY_MAPPING==1 &&                                       //JerryYang 20250113 : Color上方TrayID
                   TestIF_File.bEnableTrayID2==true &&
                   USE_KEYENCE_EMPTY==3)
                {
                    if(MOT[MTrayX].MotorMove(Prod.iXTrayEmpty))
                        Task=400;
                }
                else if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                //Sam 20230220 : 移除客戶碼  //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                        Task=400;
                }
                else if(IniConfig.bP56TrayArmWaitAtColorTrack ||                //Steven 20240516 : Tray Arm等待位置改到Color
                        (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR) ||  //wei 20150925 待機位置改道 Color
                        (Target!=0 && TrayForm.LodareType==1 &&
                         TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1]==1))
                {
                    if(TrayArmMotorMove(Prod.iXTrayColor))
                        Task=400;
                }
                else
                {
                    if(TrayArmMotorMove(Prod.iXTrayEmpty))
                        Task=400;
                }
            }
            break;
        case 350:
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                Task=300;
            }
            break;
        case 400:                                                               // auto1,auto2,auto3 need tray ?
            Target=WhichAutoNeedTray();                                         //which auto need tray
            if(Target==0)
            {
                iCatchTrayControlManual=0;
                Task=100;
            }
            else
            {
                if(TrayForm.bEnableAMR && bNeedCoverTray[Target-1])                    //Eastsun 20260515 F010 AMR cover tray
                {
                    InitialCoverTrayTask();
                    Task=405;
                    if(bAMRFullICBin && Target==1)
                    {
                        iReceiveAutoTrayTask[0]=1;
                        if(iUnloaderTrayCountCal[0]>2 || MOT[MMAuto1].Tray.HasIC())
                        {
                            Task=410;
                        }
                        else
                        {
                            Task=410;
                            iWhichAuto=SearchNeedAMRUnload(false);
                            if(iWhichAuto!=-1)
                            {
                                InitialCoverTrayTask();
                                InitialDoMagazineAMRTrayFeed();
                                Task=424;
                            }
                        }
                    }
                }
                else
                {
                InitialCatchNewTrayFromBufferTask();
                if(TrayForm.LodareType!=0 &&                                    //Steven 20170628 (Wei) : 修正死雞死雞死雞~~
                   (Target==2 || Target==3) &&
                   TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1])
                {
                    Task=600;
                    break;
                }
                bCatchTrayFinishAction=true;                                    //wei 20161219 (Steven) Tray Mapping
                Task=500;
            }
            }
            break;
        //==> Eastsun 20260515 F020 (AMR cases 405-440: full-bin processing)
        case 405:
            if(DoCoverTray(Target))
            {
                bNeedCoverTray[Target-1]=false;
                Task=400;
            }
            break;
        case 410:
            if(DoReceiveAutoTray(0))
            {
                str.sprintf("P1:0,P2:0,P3:0,P4:%d,P5:0,P6:0", SearchNeedAMRUnloadCount());

                asSupplementBin=str;

                bUnLoaderActionFlag[0]=true;
                iUnloaderTrayCountCal[0]=0;
                TestIF_File.asAMRBinSetting[0]=AMRUnloadBin(1);

                TestIF_File.iAMRTrayCount[3]-=iAMRCoverTray;
                EventReport(SECS_EVENT.AGVSupplement);

                asSupplementBin="";
                TestIF_File.iAMRTrayCount[3]=0;
                TestIF_File.iAMRDeviceCount[3]=0;
                TestIF_File.asAMRBinSetting[0]="";
                InitialCoverTrayTask();
                Task=430;
            }
            break;
        case 420:
            if(DoCoverTray(1))
            {
                InitialDoMagazineAMRTrayFeed();
                Task=425;
            }
            break;
        case 424:
            if(DoReceiveAutoTray(0))
            {
                Task=425;
            }
            break;
        case 425:
            if(DoMagazineAMRTrayFeed(iWhichAuto))
            {
                iBinPos=BinSelect[iTestRunMode].iCatDataT3Pos[iWhichAuto];
                int iWhichTray=iBinPos-e3PosMag1;
                TestIF_File.iAMRDeviceCount[3]=TestIF_File.iAMRMagzineDeviceCount[iWhichTray];
                str.sprintf("P1:0,P2:0,P3:0,P4:%d,P5:0,P6:0", SearchNeedAMRUnloadCount());
                for(int k=iWhichTray; k<(14-iWhichTray); k++)
                {
                    if(k==iWhichTray)
                    {
                        if(BinSelect[iTestRunMode].bMagazineLink[k]==false)
                        {
                            TestIF_File.iAMRMagzineDeviceCount[k]=0;
                        }
                    }
                    else if(BinSelect[iTestRunMode].bMagazineLink[k])
                    {
                        TestIF_File.iAMRMagzineDeviceCount[k]=0;
                    }
                    else
                    {
                        break;
                    }
                }

                asSupplementBin=str;
                str=AMRUnloadBin(iBinPos);
                bUnLoaderActionFlag[0]=true;
                iUnloaderTrayCountCal[0]=0;
                TestIF_File.asAMRBinSetting[0]=str;

                TestIF_File.iAMRTrayCount[3]-=iAMRCoverTray;
                EventReport(SECS_EVENT.AGVSupplement);

                asSupplementBin="";
                TestIF_File.iAMRTrayCount[3]=0;
                TestIF_File.iAMRDeviceCount[3]=0;
                TestIF_File.asAMRBinSetting[0]="";
                Task=430;
            }
            break;
        case 430:
            if(bUnLoaderActionFlag[0]==false)
            {
                Task=440;
            }
            else
            {
                fMain->Pause("AMR");
                str.sprintf("Auto %d Full, Wait for AMR",iWhichAuto+1);
                RecordProcess(str);
                #ifdef SOFT_SIMULTE
                bUnLoaderSECSActionFlag[0]=true;
                fLotInfo->InitialUnLoaderTask(0);
                bUnLoaderSECSActionFlag[1]=true;
                fLotInfo->InitialUnLoaderTask(1);
                bUnLoaderSECSActionFlag[2]=true;
                fLotInfo->InitialUnLoaderTask(2);
                #endif
            }
                    break;
        case 440:
            iWhichAuto=SearchNeedAMRUnload(false);
            if(iWhichAuto!=-1)
            {
                InitialCoverTrayTask();
                InitialDoMagazineAMRTrayFeed();
                Task=420;
                }
            else
            {
                bAMRFullICBin=false;
                bNeedCoverTray[0]=true;
                Task=400;
            }
            break;
        //<== Eastsun 20260515 F020 (AMR cases 405-440)
        case 500:
            if(AUTO_EMPTY_COLOR!=0 ||                                                                                   //Steven 20251211
               Sen[SnEmptyTrayIsLock1].IsOff()==false)
            {
                ret=CatchNewTrayFromBuffer(Target);                                                                     //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm
                if(ret==1)
                {
                    Task=1000;
                }
                else if(ret==2)
                {
                    Task=1;

                    bIsCatchingFromBuffer=false;                                                                        //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中

                    if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                                                         //Sam 20230220 : 移除客戶碼   //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
                        iPickfrom=1;
                    else if(TrayForm.LodareType==0)                                                                     //jou 2011-05-31 start : 開啟iManualRemoveLoader模式下,emtpy沒有tray,auto又需要tray時,hang up
                        iPickfrom=TrayForm.LoaderToEmptyColor[iRunStartMode];
                    else
                        iPickfrom=TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1];

                    if((TrayForm.iManualRemoveLoader==1 &&
                        bLoaderHasSkip==true) ||
                       bNeedManualRemoveTray==true)                                                                     //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                    {
                        if(iPickfrom==0 && MOT[MMEmpty].fHasTray==false)
                        {
                            Task=100;
                            break;
                        }
                        else if(iPickfrom==1 && MOT[MMColor].fHasTray==false)
                        {
                            Task=100;
                            break;
                        }
                        else if(iPickfrom==2 && MOT[MMAuto2].fHasTray==false)
                        {
                            Task=100;
                            break;
                        }
                    }

                    if(AUTO_EMPTY_COLOR!=0)                                                                             //jou 2010-03-19 start : loader選empty,auto選color時,6軌道tray arm會來回跑
                    {
                        Target=WhichAutoNeedTray();                                                                     //which auto need tray
                        if(Target!=0)
                        {
                            Task=400;
                        }
                    }
                }
            }
            else
            {
                if(Cylinder[C_TrayX_UpDown].Pop())
                {
                    if(AUTO_EMPTY_COLOR!=0)
                        ShowErrorMessage("JAM1003", K_RETRY, MMEmpty, false, "DoCatchTray_500");                        //Steven 20120925 : JAM1001 -> JAM1003
                    else
                        ShowErrorMessage("MES1050", K_RETRY, MMEmpty, false, "DoCatchTray_500");
                    InitialCatchNewTrayFromBufferTask();                                                                //kevin 20110721  避免托盤SENSOR誤動作 夾TRAY沒吸到TRAY移到AUTO
                }
            }
            break;
        case 600:
            ret=CatchNewTrayFromBuffer(Target);                                 //Sam 20230220 : 拿掉 bMustFromColorBuffer & bFailAlarm
            if(ret==1)
            {
                Task=1000;
            }
            else if(ret==2)
            {
                Task=1;

                bIsCatchingFromBuffer=false;                                    //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中

                if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                     //Sam 20230220 : 移除客戶碼  //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
                    iPickfrom=1;
                else if(TrayForm.LodareType==0)                                 //jou 2011-05-31 start : 開啟iManualRemoveLoader模式下,emtpy沒有tray,auto又需要tray時,hang up
                    iPickfrom=TrayForm.LoaderToEmptyColor[iRunStartMode];
                else
                    iPickfrom=TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1];

                if((TrayForm.iManualRemoveLoader==1 &&
                    bLoaderHasSkip==true) ||
                   bNeedManualRemoveTray==true)                                 //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                {
                    if(iPickfrom==2 && MOT[MMAuto2].fHasTray==false)
                    {
                        Task=100;
                        break;
                    }
                }

                if(AUTO_EMPTY_COLOR!=0)                                         //jou 2010-03-19 start : loader選empty,auto選color時,6軌道tray arm會來回跑
                {
                    Target=WhichAutoNeedTray();                                 //which auto need tray
                    if(Target!=0)
                    {
                        Task=400;
                    }
                }
            }
            break;
        case 1000:                                                              // has tray
            bIsCatchingFromBuffer=false;                                        //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中

            TargetBuf=WhichAutoNeedTray();                                      //jou 2015-08-25 Target -> TargetBuf Color Tray Place to Auto1(Pass) Tray
            if(TargetBuf==0)                                                    // no any auto need tray
            {
                Task=2000;                                                      // place to buffer
                break;
            }

            if(fAGV->Use_AMR() &&                                               //Steven 20251216 : for AMR
               CatchTraySuck.iWhichKit==1)                                      //Empty
            {
                for(int i=0; i<=iAutoRight; i++)
                {
                    if(bNeed1DCoverTray[TargetBuf-1])                           //Need 1D Tray
                    {
                        ShowMyMessage("Need Cover tray but pick from Empty.");
                        Task=2000;                                              // place to buffer
                        break;
                    }
                }
            }

            Task=1100;
            iCatchTrayControlManual=0;
            InitialPlaceTrayToAutoTask();

            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                  //Ifor 20170419 (wei) add Empty/Color Tray 補Tray至Auto需關閉 Pre Alaem
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
                if(Sen[SnEmptyTrayIsFull1].IsOn()==false && bHasEmptyTrayPreAlarm==true)
                {
                    bHasEmptyTrayPreAlarm=false;
                    if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)                             //Alarm 前若form有開啟先關閉  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                        MyMessageBox->Close();
                }

                if(Sen[SnEmptyTrayIsFull2].IsOn()==false && bHascolorTrayPreAlarm==true)
                {
                    bHascolorTrayPreAlarm=false;
                    if(W906_FormShowing("MyMessageBox", MyMessageBox->Visible)==true)                             //Alarm 前若form有開啟先關閉  //AI(W906-FSHOW-E3) 20260929 [W906] ST01-E3：改問頁面表（MyMessageBox->Visible 在移植樹沒人設成真；接上後只有 C++ 阻塞等待框開著時為真，這條工作流程那時不會跑，值不變）
                        MyMessageBox->Close();
                }
            }
            break;
        case 1100:
            if(Target==0)                                                       //kevin 20160715
            {
                Task=1200;
            }
            //==> Eastsun 20260819 : 9050 dispatch place-to-auto (single param fn)   //AI(W906-F9050-BD) 20261004: H070, 910 :7743-7747 verbatim
            else if((MachineTypeChoice==Type_HT9050)
                    ? DoPlaceTrayToAuto_9050(Target)==1
                    : DoPlaceTrayToAuto(Target))
            //<== Eastsun 20260819
            {
                // AI(W906-BinDispInt) 20260820: gate RETIRED -- same ground
                // as the FlashPro flip above (real base landed, golden's own
                // !=NULL guard preserves offline behaviour).
                if(HSys.BinDisCtrl!=NULL)
                    HSys.BinDisCtrl->ClearAutoChangingWarn(Target-1); //Eastsun 20260513 : 顯示器閃爍功能
                ret=WhichAutoNeedTray();
                if(USE_LdUldCassetteMode==1)                                    //RogerYang 20260203 : Add for HT9046CR
                {
                    if(ret!=0)
                    {
                        Target=ret;
                        break;
                    }
                    {
                        Task=1200;
                    }
                }
                else
                {
                    if(ret!=0)
                    {
                        Task=400;
                    }
                    else
                    {
                        Task=1200;
                    }
                }
            }
            break;
        case 1150:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=1200;
            else
                break;
        case 1200:
            if(TRAY_ARM_MODE==eAboveCoveyor &&
               MTrayXCanSafeMove()==false)                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=1150;
                break;
            }
            Target=WhichAutoNeedTray();                                         //kevin 20160920 tray arm 上有TRAY Hang up
            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    MOT[MOutArmX].fCanMove=true;
                    MOT[MOutArmY].fCanMove=true;
                    if(IniConfig.bP04ColorIsEmptyUnloader)
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                    bCatchTrayFinishAction=false;                               //wei 20161005 移動到等待位置後，才可以去讀取OCR
                    iCatchTrayControlManual=0;
                    Task=10;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    MOT[MOutArmX].fCanMove=true;
                    MOT[MOutArmY].fCanMove=true;
                    if(IniConfig.bP04ColorIsEmptyUnloader)
                    {
                        MOT[MOutArmX].fCanMove=true;
                        MOT[MOutArmY].fCanMove=true;
                    }
                    bCatchTrayFinishAction=false;                               //wei 20161005 移動到等待位置後，才可以去讀取OCR
                    iCatchTrayControlManual=0;
                    Task=10;
                }
            }
            break;
        case 2000:
            InitialPlaceToBufferTask();
            if(TrayForm.LoaderToEmptyColor[iRunStartMode]!=2)                   //kevin 20120726
                iCatchTrayControlManual=0;
            if(SUPPORT_2_EMPTY_EMPTY)
            {
                if(IniConfig.bP04ColorIsEmptyUnloader)
                {
                    if(MOT[MTrayX].fHasTray==false)
                    {
                        Task=1;
                        break;
                    }

                    if(MOT[MMColor].fHasTray || MOT[MMColor_Car].fHasTray)
                        break;
                }
                else
                {
                    if(MOT[MMEmpty1].fHasTray || MOT[MMEmpty1_Car].fHasTray)
                        break;
                }
            }
            Task=2100;
            break;
        case 2100:
            if(SUPPORT_2_EMPTY_EMPTY==false)
            {
                if(AUTO_EMPTY_COLOR!=0)
                {
                    if(IsPlaceToColor(0))                                                                                                                       //Sam 20221027 : 整合 P24 功能
                    {
                        if(iReceiveColorTray==1)                                                                                                                //jou 980619 /**/ source speed up release tray
                            break;

                        if(MOT[MMColor_Car].fHasTray)
                        {
                            if(TrayForm.bEnableAMR &&                           //Eastsun 20260515 整合
                               (IsLoaderCoverTray()>0 || bNeedCoverTray[0] ||
                               bNeedCoverTray[1] || bNeedCoverTray[2]))
                            {
                                Target=0;
                            }
                            else if(MOT[MTrayX].Tray.iNeedManualRemoved>=0)  //Sam 20230317 : 修正 P24 功能
                            {
                                Target=0;
                            }
                            else
                            {
                                Target=WhichAutoNeedTray();
                            }

                            if(Target>0)                                                                                                                        // now any auto need tray
                            {
                                if(CUSTOMER_CODE==CC_TERAPOWER &&                                                                                               //Sam 20200420 : 晶兆成修正 RT 時，Loader 吸起來(紅Tray)剛好 Auto 剛好需要補 Tray(需要黑Tray時)放錯問題
                                   IniConfig.bA10_AutoReTest &&
                                   TestIF_File.bSCKART_EnableART &&
                                   fSCKART->iFTRTCount!=0)                                                                                                      //Sam 20191113 : TCP ART 晶兆成 RT 時強制將 Load 做完的搬到 Color
                                {
                                    if(TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1]==1)                                                                 //Auto 需要紅 Tray 才能補
                                    {
                                        Task=1000;
                                        fColorCanSupplyNewTray=true;
                                    }
                                }
                                else if(TrayForm.LodareType==0 ||
                                        TrayForm.LoaderToEmptyColor[iRunStartMode]==TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1])
                                {
                                    Task=1000;
                                    fColorCanSupplyNewTray=true;                                                                                                //KEVIN 20221005 ADD CHANGE
                                    if(CUSTOMER_CODE==CC_ASE_KaohSiung &&
                                       IniConfig.bG11ASEReport)                                                                                                 //kevin 20220924 add read tray id put new tray   load 要放AUTO TRAY
                                    {
                                        if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                                        {
                                            sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty Tray Finish,%s",iTrayTotal[0],TrayID[0][2]);                //kevin 20210623  0:loader 1:Empty  2:color  Tray 入軌道數量
                                            RecordProcess(sBuffer);
                                            TrayID[2][1]=TrayID[0][2];                                                                                          //kevin 20220923  load 放TRAY至 empty TRAY TRAY ID 資料交換
                                            sBuffer.sprintf("<AutoMove>Empty Tray ID,%s",TrayID[2][1]);                                                         //kevin 20220923  load 放TRAY至 empty TRAY TRAY ID 資料交換
                                        }
                                        else
                                        {
                                            sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty2 Tray Finish,%s",iTrayTotal[0],TrayID[0][2]);               //kevin 20210623 add log
                                            RecordProcess(sBuffer);
                                            TrayID[2][1]=TrayID[0][2];                                                                                          //kevin 20220923  load 放TRAY至 COLOR TRAY TRAY ID 資料交換
                                            sBuffer.sprintf("<AutoMove>Empty2 Tray ID,%s",TrayID[2][1]);                                                        //kevin 20220923  load 放TRAY至 COLOR TRAY TRAY ID 資料交換
                                        }
                                        RecordProcess(sBuffer);
                                        TrayID[0][2]="";
                                    }
                                }
                            }
                            else
                            {
                                if(MOT[MTrayX].fHasTray &&
                                   fColorCanSupplyNewTray==true)                                                                                                //Steven 20130930
                                {
                                    if(iReceiveColorTray!=2)                                                                                                    //Steven 20131025 : 三片Tray會夾斷
                                    {
                                        fColorCanSupplyNewTray=false;
                                        iReceiveColorTray=1;
                                        InitAutoColorReceiveTask();
                                    }
                                }
                            }
                            break;
                        }

                        if(Sen[SenColorHasTray].IsOn() && Sen[SenColorCWDete].IsOn())
                        {
                            MOT[MMColor].fHasTray=true;
                            #ifdef SOFT_SIMULTE
                            MOT[MMColor].fHasTray=false;
                            #endif
                        }

                        #ifndef SOFT_SIMULTE

                        if(SW[SwACColorCW].Status()==true ||
                           iReceiveColorTray!=0)                                                                                                                //Steven 20130917 : Double Tray問題,測試 Color
                        {
                            break;
                        }
                        #endif

                        if(MOT[MMColor].fHasTray)
                        {
                            if(iReceiveColorTray!=2)                                                                                                            //Steven 20131025 : 三片Tray會夾斷
                            {
                                fColorCanSupplyNewTray=false;
                                iReceiveColorTray=1;
                                InitAutoColorReceiveTask();
                            }
                            break;
                        }
                    }
                    else
                    {
                        if(iReceiveEmptyTray==1)
                            break;

                        if(MOT[MMEmpty_Car].fHasTray)                                                                                                           //jou 980619 /**/ source speed up release tray
                        {
                            if(TrayForm.bEnableAMR &&                           //Eastsun 20260515 整合
                               (IsLoaderCoverTray()>0 || bNeedCoverTray[0] ||
                               bNeedCoverTray[1] || bNeedCoverTray[2]))
                            {
                                Target=0;
                            }
                            else if(MOT[MTrayX].Tray.iNeedManualRemoved>=0)  //Sam 20230317 : 修正 P24 功能
                            {
                                Target=0;
                            }
                            else
                            {
                                Target=WhichAutoNeedTray();
                            }

                            if(Target>0)                                                                                                                        // now any auto need tray
                            {
                                if(TrayForm.LodareType==0 ||
                                   TrayForm.LoaderToEmptyColor[iRunStartMode]==TrayForm.AutoFromEmptyColor[iRunStartMode][Target-1])
                                {
                                    Task=1000;
                                    fEmptyCanSupplyNewTray=true;
                                    if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bG11ASEReport)                                                              //kevin 20220924 add read tray id put new tray   load 要放AUTO TRAY
                                    {
                                        if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                                        {
                                            sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty Tray Finish,%s",iTrayTotal[0],TrayID[0][2]);                //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
                                        }
                                        else
                                        {
                                            sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty2 Tray Finish,%s",iTrayTotal[0],TrayID[0][2]);               //kevin 20210623 add log
                                            RecordProcess(sBuffer);
                                            TrayID[1][1]=TrayID[0][2];                                                                                          //kevin 20220923  load 放TRAY至 COLOR TRAY TRAY ID 資料交換
                                            sBuffer.sprintf("<AutoMove>Empty2 Tray ID,%s",TrayID[1][1]);                                                        //kevin 20220923  load 放TRAY至 COLOR TRAY TRAY ID 資料交換
                                        }
                                        RecordProcess(sBuffer);
                                        TrayID[0][2]="";
                                    }
                                }
                            }
                            else
                            {
                                if(MOT[MTrayX].fHasTray &&
                                   fEmptyCanSupplyNewTray==true)                                                                                                //Steven 20130930
                                {
                                    if(iReceiveEmptyTray!=2)                                                                                                    //Steven 20131025 : 三片Tray會夾斷
                                    {
                                        fEmptyCanSupplyNewTray=false;
                                        iReceiveEmptyTray=1;
                                        InitAutoEmptyReceiveTask();
                                    }
                                }
                            }
                            break;
                        }

                        if(Sen[SenEmptyHasTray].IsOn() &&
                           Sen[SenEmptyCWDete].IsOn())                                                                                                          //Sam 20211118 : 修正  || >> &&
                        {
                            MOT[MMEmpty].fHasTray=true;

                            #ifdef SOFT_SIMULTE
                            MOT[MMEmpty].fHasTray=false;
                            #endif
                        }

                        #ifndef SOFT_SIMULTE
                        if(SW[SwACEmptyCW].Status() ==true ||
                           iReceiveEmptyTray!=0)                                                                                                                //Steven 20130917 : Double Tray問題,測試 Q4
                        {
                            break;
                        }
                        #endif

                        if(MOT[MMEmpty].fHasTray)
                        {
                            if(iReceiveEmptyTray!=2)                                                                                                            //Steven 20131025 : 三片Tray會夾斷
                            {
                                fEmptyCanSupplyNewTray=false;
                                iReceiveEmptyTray=1;
                                InitAutoEmptyReceiveTask();
                            }
                            break;
                        }
                    }
                }
            }

            if(CosFunction.bUseEmptyColorTrayPreAlarm==true &&                                                                                                  //Ifor 20170419 (wei) add Empty/Color Tray PreAlarm 後要放Tray時要Alarm停機
               IniConfig.bP32EmptyColorTrayPreAlarm==true)
            {
                if(Sen[SnEmptyTrayIsFull1].IsOn()==true && bHasEmptyTrayPreAlarm==true)
                {
                    ShowErrorMessage("MES1020", K_RETRY, MMEmpty, false, "DoCatchTray");
                    break;
                }
                else
                {
                    bHasEmptyTrayPreAlarm=false;
                }

                if(Sen[SnEmptyTrayIsFull2].IsOn()==true && bHascolorTrayPreAlarm==true)
                {
                    ShowErrorMessage("MES1420", K_RETRY, MMColor, false, "DoCatchTray");
                    break;
                }
                else
                {
                    bHascolorTrayPreAlarm=false;
                }
            }

            if(MOT[MTrayX].Tray.iNeedManualRemoved==1 &&
               IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT)                                                                                                //Sam 20221027 : 整合 P24 功能
            {
                if(iReceiveColorTray!=0)
                    break;
            }
            bIsPlacingToBuffer=true;                                                                                                                            //JerryYang 20250828 : fix color誤退TRAY
            Task=2120;
        case 2120:
            //==> Eastsun 20260819 : 9050 dispatch central routing   //AI(W906-F9050-BD) 20261004: H071, 910 :8087-8091 verbatim; HAZARD kept: with Tray.Data ToBuffer=2 a 9050 stays in this case without an alarm (DoPlaceToBuffer_9050 case 200, NIGHT_REPORT s0 #105)
            if((MachineTypeChoice==Type_HT9050)
                ? DoPlaceToBuffer_9050()
                : DoPlaceToBuffer())
            //<== Eastsun 20260819
            {
                MOT[MTrayX].Tray.bMustToEmpty=false;                                                                                                            //Sam 20230317 : 修正 P24 功能
                bCatchTrayFinishAction=false;
                bIsPlacingToBuffer=false;                                                                                                                       //JerryYang 20250828 : fix color誤退TRAY
                if(IniConfig.bP25EmptyColorNoSuppleAutoNoLoadEmpty ||                                                                                           //kevin 20151102
                   MOT[MMEmpty].Tray.iNeedManualRemoved>=0 ||                                                                                                   //Sam 20230317 : 修正 P24 功能
                   MOT[MMColor].Tray.iNeedManualRemoved>=0)
                {
                    if(IsPlaceToColor(1))                                                                                                                       //Sam 20221027 : 整合 P24 功能
                    {
                        if((TrayForm.LodareType==1 &&
                            TrayForm.LoaderToEmptyColor[iRunStartMode]==0 &&
                            TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto1]==0 &&
                            TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==0 &&
                            TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto3]==0) ||                                                                           //kevin 20151102 load to color no supply Auto1-3 //JerryYang 20160309 修正開啟[P25]但Loader type選擇same時，不該退出的tray盤會被退出
                            MOT[MMColor].Tray.iNeedManualRemoved>=0)                                                                                            //Sam 20230317 : 修正 P24 功能
                        {
                            fColorCanSupplyNewTray=false;
                            iReceiveColorTray=1;
                            InitAutoColorReceiveTask();
                        }
                    }
                    else
                    {
                        if((TrayForm.LodareType==1 &&
                            TrayForm.LoaderToEmptyColor[iRunStartMode]==0 &&
                            TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto1]==1 &&
                            TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto2]==1 &&
                            TrayForm.AutoFromEmptyColor[iRunStartMode][eAuto3]==1) ||                                                                           //kevin 20151102 load to empty no supply Auto1-3 //JerryYang 20160309 修正開啟[P25]但Loader type選擇same時，不該退出的tray盤會被退出
                           MOT[MMEmpty].Tray.iNeedManualRemoved>=0)                                                                                             //Sam 20230317 : 修正 P24 功能
                        {
                            fEmptyCanSupplyNewTray=false;
                            iReceiveEmptyTray=1;
                            InitAutoEmptyReceiveTask();
                        }
                    }
                }

                if(CUSTOMER_CODE==CC_ASE_KaohSiung && IniConfig.bG11ASEReport)                                                                                  //kevin 20210810 read tray id tput new tray
                {
                    if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)
                    {
                        sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty Tray Finish,%s",iTrayTotal[0],TrayID[0][2]);                                    //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
                    }
                    else
                    {
                        sBuffer.sprintf("<AutoMove>Loader Tray %d move to Empty2 Tray Finish,%s",iTrayTotal[0],TrayID[0][2]);                                   //kevin 20210623 add log
                        RecordProcess(sBuffer);
                        TrayID[2][1]=TrayID[0][2];                                                                                                              //kevin 20220923  load 放TRAY至 COLOR TRAY TRAY ID 資料交換
                        sBuffer.sprintf("<AutoMove>Empty2 Tray ID,%s",TrayID[2][1]);                                                                            //kevin 20220923  load 放TRAY至 COLOR TRAY TRAY ID 資料交換
                    }
                    TrayID[0][2]="";
                    RecordProcess(sBuffer);
                }
                Task=2130;
            }
            break;
        case 2130:
            if(CosFunction.bOpenDoorCheckLoaderAfterTrayEnd &&                  //Jimmychiu 20250826 : Open Door Check Loader After TrayEnd
               AUTO_EMPTY_COLOR==0 &&
               bNeedManualCheckEmptyTray)
            {
                ShowMyMessage("Please check the tray on Empty and Color mamually.");
            }
            else
            {
                Task=2140;
            }
            break;
        case 2140:
            if(AUTO_EMPTY_COLOR==0 &&
              (IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray ||
               IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT) &&             //Sam 20230317 修正 P24 功能
              (MOT[MMEmpty].Tray.iNeedManualRemoved>=0 ||
               MOT[MMColor].Tray.iNeedManualRemoved>=0))
            {
                #ifdef SOFT_SIMULTE
                MOT[MMColor].Tray.iNeedManualRemoved=-1;                        //Sam 20230317 : 修正 P24 功能
                MOT[MMEmpty].Tray.iNeedManualRemoved=-1;
                if(CosFunction.bSpecialP24)                                     //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                {
                     ShowMyMessage("Please remove the tray on Color mamually.", "請將Color 上的 Tray 手動取走 !!");
                }
                else
                {
                     ShowMyMessage("Please remove the tray on Empty and Color mamually.", "請將Empty和Color 上的 Tray 手動取走 !!");
                }
                Task=2160;
                #else
                if(Sen[SnEmptyTrayIsLock1].IsOff())
                {
                    ShowMyMessage("Please put Empty and Color to correct position!!", "請將Empty & Color 推至定位 !!");
                }

                if(((Sen[SnEmptyTrayHasTray1].IsOn() ||
                     Sen[SnEmptyTrayHasTray2].IsOn()) &&
                    CosFunction.bSpecialP24==false) ||                          //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                    (Sen[SnEmptyTrayHasTray2].IsOn() &&                         //Alick 20161207 add for 矽格只判斷color
                     CosFunction.bSpecialP24))                                  //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                {
                    if(CosFunction.bSpecialP24)                                 //Sam 20230320 : 整合 P24 北興版為獨立功能碼    //Alick 20161207 add for 矽格修正顯示文字
                        ShowMyMessage("Please remove the tray on Color mamually.", "請將Color 上的 Tray 手動取走 !!");
                    else
                        ShowMyMessage("Please remove the tray on Empty and Color mamually.", "請將Empty & Color 上的 Tray 手動取走 !!");
                    Task=2140;
                    break;
                }
                else
                {
                    if((Sen[SnEmptyTrayHasTray1].IsOff() &&
                        Sen[SnEmptyTrayHasTray2].IsOff() &&
                        Sen[SnEmptyTrayIsLock1].IsOn()   &&
                        CosFunction.bSpecialP24==false) ||                      //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                       (Sen[SnEmptyTrayHasTray2].IsOff() &&
                        Sen[SnEmptyTrayIsLock1].IsOn() &&                       //Alick 20161207 add for 矽格只判斷color是否取走就可
                        CosFunction.bSpecialP24))                               //Sam 20230320 : 整合 P24 北興版為獨立功能碼
                    {
                        MOT[MMColor].Tray.iNeedManualRemoved=-1;                //Sam 20230317 : 修正 P24 功能
                        MOT[MMEmpty].Tray.iNeedManualRemoved=-1;
                        Task=2160;
                    }
                    break;
                }
                #endif
            }
            else
            {
                Task=2160;
            }
            break;
        case 2160:
            fEmptyCanSupplyNewTray=true;
            fColorCanSupplyNewTray=true;
            if(iAseTrayEnd!=0)                                                  //kevin 20150925
            {
                iAseTrayEnd=0;
                RespondASECom("@e02112Done");                                   //kevin 20150925 回應 ase TrayEnd finish
            }
            Task=1;
            break;
        //Steven 20101019 Start
        case 3000:
            if((TrayForm.iManualRemoveLoader==1 &&                                                                      //Steven 20170919 (wei) : 修改判斷式
//                bLoaderHasSuck==true &&                                       //JerryYang 20241015 : Mark
               ((IniConfig.bA65_BundleIDList==false) ||
               (IniConfig.bA65_BundleIDList &&
                TestIF_File.bSCKART_LotDeviceCheck &&
                fSCKART->iInputCount!=0 &&
                fSCKART->CheckLoadingCount()==0 &&
                bLoaderHasSuck==true))) ||                                                                              //JerryYang 20250220 : fix AUTO IN OUT
                bNeedManualRemoveTray==true)                                                                            //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
            {
                if(CUSTOMER_CODE==CC_Greatek &&                                                                         //Sam 20190731 : 超豐正賢要求 ReTest Mode 不用手動移除 Tray
                   (LastSet.iRunStartMode==rsmContinuRetest ||
                    LastSet.iRunStartMode==rsmCInitialRetest))
                {
                    bLoaderHasSkip=false;
                    bNeedManualRemoveTray=false;                                                                        //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                    Task=200;
                    break;
                }

                Cylinder[C_TrayY_Fixer].Off();                                                                          //Open Fix Supply Try Fix Cylinder
                Cylinder[C_LoaderEdgePush].Off();                                                                       //jou 2010-12-20 修正alarm時,就放開汽缸
                Cylinder[C_LoaderUpPress].Off();                                                                        //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray

                if(IniConfig.bKoreaFunction)                                                                            //jou 2010-12-15 ASE Korea 要按Skip
                {
                    ShowErrorMessage("MES0922", K_SKIP, MMTrayY, false, "DoCatchTray");
                }
                else
                {
                    if(IniConfig.bSPILFunction && CUSTOMER_CODE!=CC_SPIL_CS &&
                       ArmSpeed[InArm].bAutoSKIP==true &&
                       IniConfig.bRecordSkipPosition==true &&
                       bLoaderHasSkip==true &&
                       bAutoSkipCntOver==false)                                                                         //JerryYang 20210420 : 矽品蘇州要求auto skip數量小於設定值要由工程師排除
                    {
                        ret=ShowErrorMessage("MES0103", K_SKIP, MInArmX, false, "DoCatchTray");                         //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home start
                    }
                    else if(CosFunction.bNextTrayNeedManualRemoveTray==true &&
                            CosFunction.bMaualRemoveTray3Alarm==true)                                                   //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
                    {
                        ret=MaualRemoveTray3Alarm.GetTakePickUpError();
                        ShowErrorMessage(AnsiString().sprintf("MES0922%d", ret), K_RETRY, MMTrayY, false, __FUNC__);    //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
                    }
                    else
                    {
                        ShowErrorMessage("MES0922", K_RETRY, MMTrayY, false, "DoCatchTray");
                    }
                }
                iInArmWaitPosition=0;                                                                                   //Ifor 20210209 add:旗標未清除導致Tray Arm 不作動
                Task=3100;
            }
            else
            {
                if((TestIF_File.bSCKART_LotDeviceCheck &&
                    fSCKART->iInputCount!=0 &&
                    (fSCKART->CheckLoadingCount()==1 ||
                     (IniConfig.bA65_BundleIDList &&
                      fSCKART->CheckLoadingCount()==2))))
                {
                }

                if(TrayForm.iManualRemoveLoader==2)                                                                     //Steven 20130819 : 加入開門檢查的功能
                    ShowErrorMessage("MES0923", K_RETRY, MMTrayY, false, "DoCatchTray");
                bLoaderHasSkip=false;
                bMustCoverIDTray=true;                                                                                  //JerryYang 20240318 : 下一盤必須是Cover Tray
                bNeedManualRemoveTray=false;                                                                            //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                Task=200;
            }
            break;
        case 3010:
            if(Sen[SnLoaderSureTray].IsOff()==false)                            //Tray沒被拿走就繼續叫
            {
                Task=3000;
            }
            else
                Task=3100;
            break;
        case 3100:
            #ifdef SOFT_SIMULTE
            if(fTrayMapping->cbBarCodeSimulate->Checked)
            {
                MOT[MMTrayY].ClearTray(__FUNC__);
                MOT[MMTrayY_Car].fCanMove=true;
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                bLoaderHasSkip=false;
                bNeedManualRemoveTray=false;                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
                SetAutoSkipCount(0);                                            //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
                bCheckTrayDevice=false;                                         //wei 20170302 Tray map data error
                bDoTrayDeviceCheck=false;                                       //wei 20170317 (steven) Device Remain 殘料檢測
                bTrayHaveDevice=false;
                Task=1;
            }
            else
            {
                if(MOT[MMTrayY].Tray.HasIC()==false)
                {
                    MOT[MMTrayY].ClearTray(__FUNC__);
                }
                bNeedManualRemoveTray=false;                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray  //Steven 20220517 : 換位置
                MOT[MMTrayY_Car].fCanMove=true;
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                bLoaderHasSkip=false;

                bCheckTrayDevice=false;                                         //wei 20170302 (Steven) Tray map data error
                bDoTrayDeviceCheck=false;                                       //wei 20170317 (steven) Device Remain 殘料檢測
                bTrayHaveDevice=false;
                Task=1;
            }
            #else
            if(Sen[SnLoaderSureTray].IsOff()==false)                            //Tray沒被拿走就繼續叫
            {
                if(MOT[MMTrayY].Tray.HasIC())                                   //Steven 20140818 : for Auto Skip, IC擺在後半盤
                {
                    MOT[MMTrayY_Car].fCanMove=true;
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bLoaderHasSkip=false;
                    bCheckTrayDevice=false;                                     //wei 20170302 (Steven) Tray map data error
                    bDoTrayDeviceCheck=false;                                   //wei 20170317 (steven) Device Remain 殘料檢測
                    bTrayHaveDevice=false;
                    bNeedManualRemoveTray=false;                                //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray  //Steven 20220517 : add
                    Task=1;
                }
                else
                {
                    Task=3000;
                }
            }
            else
            {
                if(TrayForm.bAutoFeed==false &&                                 //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
                   bLoaderNoTrayAutoCleanOut==true &&
                   MOT[MMTrayY_Car].fHasTray==false)
                {
                    fMain->CleanOut("DoCatchTray 3");
                }

                MOT[MMTrayY].ClearTray(__FUNC__);
                MOT[MMTrayY_Car].fCanMove=true;
                MOT[MInArmX].fCanMove=true;
                MOT[MInArmY].fCanMove=true;
                bLoaderHasSkip=false;
                bNeedManualRemoveTray=false;                                    //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray

                if(CosFunction.bNextTrayNeedManualRemoveTray==true)             //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
                {
                    if(MaualRemoveTray3Alarm.GetTakePickUpError()==2)
                    {
                        bLoaderHasSkip=false;
                        MaualRemoveTray3Alarm.Clear();
                    }
                    else
                    {
                        bLoaderHasSkip=true;
                        MaualRemoveTray3Alarm.DoRemoveTray();                   //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
                    }
                }

                if(TrayForm.bEnableAMR)                                         //Eastsun 20260519 整合
                {
                    iLoaderTrayCountCal++;
                    iLoaderTrayCountAMRCal++;
                    RecordProcess("AMR iLoaderTrayCountCal++ (2)");
                }

                SetAutoSkipCount(0);                                            //Steven 20150217 : 顯示Auto Skip的數量, 1=++, 0=清空計數
                bCheckTrayDevice=false;                                         //wei 20170302 (Steven) Tray map data error
                bDoTrayDeviceCheck=false;                                       //wei 20170317 (steven) Device Remain 殘料檢測
                bTrayHaveDevice=false;
                Task=1;
            }
            #endif
            if(CosFunction.bShowHPICCount)                                      //Steven 20221228 : 計算加熱盤IC數量
            {
                fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
            }
            break;
        case 4000:                                                              //wei 20161219 (Steven) Tray Mapping
            if(TestIF_File.bEnableTrayID)
            {
                InitialTrayIDTask();
                if(USE_COVER_TRAYID==tCID_NFC)                                  //Steven 20251211 : ATK AMR
                {
                    if(TestIF_File.bEnableTrayMap)
                        Task=5000;
                    else
                        Task=5200;
                }
                else if(USE_KEYENCE_LOADER==1)
                {
                    Task=4200;
                }
                else
                {
                    Task=4100;
                }
            }
            else
            {
                Task=5000;
            }
            break;
        case 4100:
            if(fTrayMapping->DoTrayIDCCD())
            {
                if(TestIF_File.bEnableTrayMap)
                    Task=5000;
                else
                    Task=5200;
            }
            else if(TestIF_File.bEnableTrayID==false)
            {
                if(TestIF_File.bEnableTrayMap)
                    Task=5000;
                else
                    Task=5200;
            }
            break;
        case 4200:                                                              //20200824
            if(fTrayMapping->DoTrayIDKeyence())
            {
                if(TestIF_File.bEnableTrayMap)
                {
                    Task=5000;
                }
                else
                {
                    Task=5200;
                }
            }
            else if(TestIF_File.bEnableTrayID==false)
            {
                if(TestIF_File.bEnableTrayMap)
                    Task=5000;
                else
                    Task=5200;
            }
            break;
        case 5000:
            InitialTrayMapTask(iTray_Map);                                      //Sam 20200323 : Modify Tray Function
            InitInArmTask();                                                    //KaiChen 20200316 :修正執行TrayMap, InArm到HotPlate待機, 造成誤放ic在HotPlate上
            if(TestIF_File.bEnableTrayMap==false)
            {
                Task=5200;
            }
            else
            {
                Task=5100;
            }
            break;
        case 5100:
            if(fTrayMapping->DoTrayMapCCD(iTray_Map))                           //Sam 20200323 : Modify Tray Function
            {
                Task=5200;
            }
            break;
        case 5150:
            if(Cylinder[C_TrayX_UpDown].Pop())                                  //Steven 20231026 : 針對TrayArm動作加上保護
                Task=5200;
            else
                break;
        case 5200:
            if(MTrayXCanSafeMove()==false)                                      //Ifor 20170626 (wei) Tray Arm Move 前判斷是否可移動
            {
                Task=5150;
                break;
            }

            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bLoaderTrayAction=false;
                    Task=300;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    MOT[MInArmX].fCanMove=true;
                    MOT[MInArmY].fCanMove=true;
                    bLoaderTrayAction=false;
                    Task=300;
                }
            }
            break;
        case 6000:
            if(InArmSuck.IsPickDestroyFinish() &&
               bPlaceToHotplate==false &&
               bPlaceShuttle==false &&
               iInRotateFinish!=1)                                              //Sam 20190828 : Fix Rotate & DeviceRemain 打架
            {
                bDoTrayDeviceCheck=true;                                        //wei 20170317 (steven) Device Remain 殘料檢測
                Task=6010;
            }
            else if(WhichAutoNeedTray()!=0)                                     //Sam 20190624 : 有開啟殘料檢功能時，當 Load等待 InArm  清完料再檢查殘料的時候，Unload 剛好需要補盤，造成 Hange Up 修正。
            {
                Task=300;
            }
            break;
        case 6010:
            if(WhichAutoNeedTray()!=0)                                          //Sam 20190624 : 有開啟殘料檢功能時，當 Load等待 InArm  清完料再檢查殘料的時候，Unload 剛好需要補盤，造成 Hange Up 修正。
            {
                Task=300;
                break;
            }

            if(bPlaceToHotplate==true ||
               bPlaceShuttle==true ||
               iInRotateFinish==1)                                              //Sam 20190828 : Fix Rotate & DeviceRemain 打架
            {
                break;
            }
            else if(MoveInArm2XYToShuttle2Wait())                               //MoveInArm2XYToWait
            {
                InitialTrayMapTask(iDeviceRemain);                              //Sam 20200323 : Modify Tray Function
                MOT[MInArmX].fCanMove=false;
                MOT[MInArmY].fCanMove=false;
                Task=6030;
            }
            break;
        case 6030:
            if(fTrayMapping->DoTrayMapCCD(iDeviceRemain))                       //Sam 20200323 : Modify Tray Function
            {
                Task=6100;
            }
            else if(TestIF_File.bEnableDeviceRemain==false)
            {
                Task=6100;
            }
            break;
        case 6100:
            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                ipos=Prod.iXTrayColor;
            }
            else
            {
                ipos=Prod.iXTrayEmpty;
            }

            if(TrayArmMotorMove(ipos))
            {
                if(bTrayHaveDevice)
                {
                    Task=100;
                }
                else
                {
                    //==> Eastsun 20260818 : 9050 second-path init   //AI(W906-F9050-BD) 20261004: H072, 910 :8591-8596 verbatim
                    if((MachineTypeChoice==Type_HT9050))
                        InitialCatchFromLoader_9050();
                    else
                    //<== Eastsun 20260818
                    InitialCatchFromLoader();
                    bTrayHaveDevice=false;
                    Task=250;
                }
            }
            break;
    }
}
//------------------------------------------------------------------------------
//==> Eastsun 20260515 F016 整合:KYEC AMR Loader cover-tray helper
int IsLoaderCoverTray()
{
    if(TrayForm.bEnableAMR)
    {
        if(iLoaderTrayCountAMRCal>(TestIF_File.iAMRLDNowTrayCount-iAMRCoverTray))
        {
            return 100;
        }
    }

    return 0;
}
//--------------------------------------------------------------------------------
//<== Eastsun 20260515 F016
bool IsPlaceToColor(int iStep)                                                  //Sam 20221027 : 整合 P24 功能
{
    if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                                 //Sam 20230220 : 移除客戶碼   //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
    {
        if(MOT[MTrayX].Tray.bMustToEmpty)                                       //Sam 20230317 : 修正 P24 功能
            return false;
        else
            return true;
    }

    if(IniConfig.bP04ColorIsEmptyUnloader)
        return true;

    //==> Eastsun 20260515 F016 整合:KYEC AMR cover-tray force PlaceToColor by iTrayOrder
    if(TrayForm.bEnableAMR && IsLoaderCoverTray())
    {
        if(TrayForm.iTrayOrder[TestIF_File.iAMRLDNowTrayCount-iLoaderTrayCountAMRCal]==1)
            return true;
        else
            return false;
    }//<== Eastsun 20260515 F016

    if((TrayForm.LoaderToEmptyColor[iRunStartMode]==1 &&
        IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray==false &&
        IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT==false) ||             //Sam 20221027 : 整合 P24 功能
        (iStep==0 && MOT[MTrayX].Tray.iNeedManualRemoved==1) ||                 //Catch Tray 放 Tray 前判斷    //Sam 20230317 : 修正 P24 功能
        (iStep==1 && MOT[MMColor].Tray.iNeedManualRemoved==1))                  //Catch Tray 放 Tray 後判斷    //Sam 20230317 : 修正 P24 功能
    {
        return true;
    }
    else if(TrayForm.LoaderToEmptyColor[iRunStartMode]==1)                      //Jimmychiu 20240514 : 修正Color動作異常
    {
        return true;
    }

    if(MOT[MTrayX].iIsCoverTray==IS_ID_TRAY)                                    //JerryYang 20240318
    {
        return true;
    }

    if(CUSTOMER_CODE==CC_TERAPOWER &&
       IniConfig.bA10_AutoReTest &&
       TestIF_File.bSCKART_EnableART &&
       fSCKART->iFTRTCount!=0)                                                  //Sam 20191113 : TCP ART 晶兆成 RT 時強制將 Load 做完的搬到 Color
    {
        return true;
    }
    else
    {
        return false;
    }
}
//------------------------------------------------------------------------------
bool IsTrayArmMoveAvoidOutArmCrash()
{
    MOT[MTrayX].ScanMotorStatus();
    if(TRAY_ARM_MODE==eUnderCoveyor)
        return false;

    if(MOT[MTrayX].ReadPos()>=Prod.iXTrayEmpty &&
       MOT[MTrayX].Led[iInposLed])
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool IsTrayArmAtEmptyOrColor()
{
    int iPos=MOT[MTrayX].ReadPos();

    if(TRAY_ARM_MODE==eUnderCoveyor)
        return true;

    if(iPos<Prod.iXTrayColor+100 &&
       iPos>Prod.iXTrayEmpty-100)
        return true;
    else
        return false;
}
//------------------------------------------------------------------------------
bool C_CatchTray_Fix_Puch(bool bInitial)                                        //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
{
    int &iTask=iCatchTray_Fix_Puch;
    static bool bflag=false;
    if(bInitial)
    {
        bflag=false;
        iTask=1;
        return true;
    }

    switch(iTask)
    {
        case 1:
            Cylinder[C_CatchTray_FixOn].Reset();
            Cylinder[C_CatchTray_FixOn].Off();
            Cylinder[C_CatchTray_FixOff].Off();
            bflag=false;
            iTask=100;
        case 100:
            Cylinder[C_CatchTray_FixOff].Off();
            if(bflag==false)
                bflag=Cylinder[C_CatchTray_FixOn].Push();

            if(bNewCatchTrayblock)                                                                                      //kevin 20200512 夾tray遮版削短
            {
                if(bflag)
                {
                    if(Cylinder[C_CatchTray_FixOn].OnSensor())
                        return true;
                    else
                        ShowErrorMessage("JAM0604", K_RETRY, MTrayX, false, "CatchNewTrayFromBuffer_2550");             //jou 2016-01-08 JAM0601->JAM0604 alarm錯誤訊息修正
                }
            }
            else if(bflag)
            {
                return true;
            }
            break;
    }

    return false;
}
//------------------------------------------------------------------------------
bool C_CatchTray_Fix_Pop(bool bInitial)                                         //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
{
    int &iTask=iCatchTray_Fix_Pop;
    static bool bflag=false;
    if(bInitial)
    {
        bflag=false;
        iTask=1;
        return true;
    }

    switch(iTask)
    {
        case 1:
            Cylinder[C_CatchTray_FixOn].Off();
            Cylinder[C_CatchTray_FixOff].Off();
            Cylinder[C_CatchTray_FixOff].Reset();
            bflag=false;
            iTask=100;
        case 100:
            Cylinder[C_CatchTray_FixOn].Off();
            if(bflag==false)
                bflag=Cylinder[C_CatchTray_FixOff].Push();

            if(bflag)
                return true;

            break;
    }

    return false;
}
//------------------------------------------------------------------------------
bool bCheckNoCatchEmptyTrayToAuto()
{
    int iCount=0;

    if(TrayForm.LodareType==1)                                                  //Ifor 20170421 (wei) add 避免Loader Type 選擇Same
    {
        for(int i=0; i<3; i++)
        {
            if(TrayForm.AutoFromEmptyColor[iRunStartMode][i]==1)
            {
                iCount++;
            }
        }

        if(iCount==3)
        {
            return true;
        }
    }

    return false;
}
//------------------------------------------------------------------------------
bool IsTrayArmCatchTrayFail()                                                   //JerryYang 20200926 包成函式,偵測tray arm夾tray是否異常
{
    #ifdef SOFT_SIMULTE
        return false;
    #else
    if(LastSet.iRealDummy!=DUMMY && IniConfig.bC03UseCatchTray)
    {
        switch(USE_CATCH_TRAY_MODEL)
        {
            case 0:                                                             //4 Sensor
                if(Cylinder[C_CatchTray_Fix].OnStatus()==false   ||             //wei 20150729 增加氣缸sensor判斷
                    (Sen[SnCatchTrayFix1On].IsOn()==true &&
                     Sen[SnCatchTrayFix2On].IsOn()==true))
                {
                    return true;
                }
                break;
            case 1:                                                             //2 Sensor
                if(Cylinder[C_CatchTray_Fix].OnStatus()==false)
                {
                    return true;
                }
                break;
            case 2:                                                             //2 Sensor(ART)
            case 4:                                                             //2 Sensor(ART & No Cover)  //Steven 20241211 : add
                if(Cylinder[C_CatchTray_FixOn].OnStatus()==false)               //wei 20150415 前後夾Alarm
                {
                    return true;
                }
                break;
            case 3:                                                             //4 Sensor(ART)   //Steven 20170623 (wei) : Add for catch tray with cover
                if(Cylinder[C_CatchTray_FixOn].OnStatus()==false   ||           //wei 20150729 增加氣缸sensor判斷
                    (Sen[SnCatchTrayFix1On].IsOn()==true &&
                     Sen[SnCatchTrayFix2On].IsOn()==true))
                {
                    return true;
                }
                break;
        }
    }
    return false;
    #endif
}
//------------------------------------------------------------------------------
void CatchFromLoaderSetItemData()                                               //Sam 20220712 : 整合 CatchFromLoader SetItemData
{
    CatchTraySuck.iWhichTray=0;                                                 //Steven 20220331 : 紀錄Tray從哪來的

    if(IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray ||                    //Alick 20160630 add 矽格北興用
       IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT)                        //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    {
        if(MOT[MMTrayY].Tray.iNeedManualRemoved>=0)                             //Sam 20230317 : 修正 P24 功能
        {
            MOT[MTrayX].Tray.iNeedManualRemoved=MOT[MMTrayY].Tray.iNeedManualRemoved;
            MOT[MMTrayY].Tray.iNeedManualRemoved=-1;
        }
    }
    else if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                            //Sam 20230116 : 修正矽格湖口 P39 Tray Issue
    {
        if(MOT[MMTrayY].Tray.bMustToEmpty)                                      //Sam 20230317 : 修正 P24 功能
        {
            MOT[MTrayX].Tray.bMustToEmpty=true;
            MOT[MMTrayY].Tray.bMustToEmpty=false;
        }
    }
    MOT[MTrayX].fHasTray=true;

    MOT[MTrayX].iIsCoverTray=MOT[MMTrayY].iIsCoverTray;                         //JerryYang 20240318
    MOT[MTrayX].sTrayID     =MOT[MMTrayY].sTrayID;

    if(MOT[MMTrayY].iIsCoverTray==IS_ID_TRAY)
    {
        fSortCT->pnlLoadCID->Caption="";
    }

    if(CosFunction.bShowHPICCount)                                              //Steven 20221228 : 計算加熱盤IC數量
    {
        fSortCT->pnlLoad->Caption=MOT[MMTrayY].Tray.HowManyIC();
    }
    //==> Eastsun 20260515 F016 整合:KYEC AMR loader tray count increment (before CatchTraySetItemData)
    if(TrayForm.bEnableAMR)
    {
        iLoaderTrayCountCal++;
        iLoaderTrayCountAMRCal++;
        RecordProcess("AMR iLoaderTrayCountCal++ (1)");
    }
    //<== Eastsun 20260515 F016
    CatchTraySetItemData();

    MOT[MMTrayY].ClearTray(__FUNC__);                                           //JerryYang 20240318 : 移到下面
    //==> Eastsun 20260515 F016 整合:KYEC AMR loader tray count rollover
    if(iLoaderTrayCountCal==TestIF_File.iAMRLDNowTrayCount)
    {
        iLoaderTrayCountCal=0;
    }
    //<== Eastsun 20260515 F016
    }
//------------------------------------------------------------------------------
void PlaceToBufferSetItemData()                                                 //Sam 20221027 : 整合 CatchTray SetItemData
{
    CatchTraySetItemData();
}
//------------------------------------------------------------------------------
void CatchTraySetItemData()                                                     //Sam 20221027 : 整合 CatchTray SetItemData
{
    //==> Eastsun 20260515 F016 整合:KYEC AMR cover-tray ID dispatch (parallel with fAGV->Use_AMR())
    if(TrayForm.bEnableAMR && IsLoaderCoverTray()>0)
    {
        if(TrayForm.bIDTrayOrder[TestIF_File.iAMRLDNowTrayCount-iLoaderTrayCountAMRCal]==0)
            CatchTraySuck.SetItemData(0, 0, 2);     // to Color
        else
            CatchTraySuck.SetItemData(0, 0, 1);     // to Empty
    }
    //<== Eastsun 20260515 F016
    else if(fAGV->Use_AMR() &&                                                  //Steven 20251216 : for AMR
       MOT[MTrayX].iIsCoverTray==IS_ID_TRAY)                                    //Cover Tray
    {
        CatchTraySuck.SetItemData(0, 0, 2);                                     // to Color
    }
    else if(CosFunction.bSpecialP24 &&                                          //Sam 20230320 : 整合 P24 北興版為獨立功能碼
           (IniConfig.bP24SkipEventNeedRemoveEmptyAndColorTray ||               //Alick 20160630 add 矽格北興用
            IniConfig.bP24SkipEventNeedRemoveColorTrayForIDT))                  //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    {
        if(MOT[MTrayX].Tray.iNeedManualRemoved==1)                              //Sam 20230317 : 修正 P24 功能
            CatchTraySuck.SetItemData(0, 0, 2);                                 // to Color
        else
            CatchTraySuck.SetItemData(0, 0, 1);                                 // to Empty
    }
    else if(IniConfig.bP39LoaderHasSkipPlaceToEmpty)                            //Sam 20230220 : 移除客戶碼   //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
    {
        if(MOT[MTrayX].Tray.bMustToEmpty)                                       //Sam 20230317 : 修正 P24 功能
            CatchTraySuck.SetItemData(0, 0, 1);                                 // to Empty
        else
            CatchTraySuck.SetItemData(0, 0, 2);                                 // to color
    }
    else if(CUSTOMER_CODE==CC_TERAPOWER &&
            IniConfig.bA10_AutoReTest &&
            TestIF_File.bSCKART_EnableART &&
            fSCKART->iFTRTCount!=0)                                             //Sam 20191113 : TCP ART 晶兆成 RT 時強制將 Load 做完的搬到 Color
    {
        CatchTraySuck.SetItemData(0, 0, 2);                                     // to color
    }
    else
    {
        if(TrayForm.LoaderToEmptyColor[iRunStartMode]==1)
            CatchTraySuck.SetItemData(0, 0, 2);                                 // to color
        else if(TrayForm.LoaderToEmptyColor[iRunStartMode]==0)                  //kevin 20120726
            CatchTraySuck.SetItemData(0, 0, 1);                                 // to black
        else
            CatchTraySuck.SetItemData(0, 0, 3);                                 // to Auto1 kevin 20120726
    }
}
//==============================================================================
// Loader Car 旋轉臂讀RDIF主流程
//==============================================================================
TQPF_Timer tNFCDelay;
void InitialLDRotArmReadRFIDTask()
{
    if(iReadCIDAction==ePortLoader)
        iReadCIDAction=ePortTotal;
    iCoverTrayIDTask[iKeyenceCoverTrayID_LoaderCar]=1;
}
//------------------------------------------------------------------------------
bool DoLoadCarRotArmReadRFID(bool bAlarm)
{
    static int iPosTmp=0;
    static int iCount=0;
    static bool bMotOk=false, bCynOk=false, bReadOK=false;

    int ret;
    int &Task=iCoverTrayIDTask[iKeyenceCoverTrayID_LoaderCar];
    AnsiString sBuffer="";

    switch(Task)
    {
        case 1:
            if(fAGV->IsATK_AMR() &&
               fTrayMapping->ldRFID->bCommConnect==false)
            {
                fTrayMapping->btnLdRFIDConn->Click();
            }
            iCount=0;
            Task=2;
        case 2:
            if(iReadCIDAction!=ePortTotal)
                break;
        case 3:
            iReadCIDAction=ePortLoader;
            bMotOk=false;
            bCynOk=false;
            bReadOK=false;
            iPosTmp=0;
            Task=100;
            asTrayIDDataCorverLoader="";
            tNFCDelay.SetSecAndOn(0.2);
            break;
        case 100:
            if(CylinderUp(C_Load_Up) && tNFCDelay.Off())
            {
                Task=500;
            }
            else
            {
                break;
            }
        case 500:
            bMotOk=false;
            bCynOk=false;
            if(MOT[MLdCarRotArm].Led[iHomeLed]==false)
            {
                //SetMotorScaleSpeed(MLdCarRotArm, ArmSpeed[TrayArm].iBodySP);    //暫用TrayArm的速度
                Task=1000;
            }
            else
            {
                fTrayMapping->btnLdRFIDClear1->Click();
                Cylinder[C_LoadCarRFIDRotArmD].Off();
                Task=2000;
            }
            break;
        case 1000:                                                              //上抬
            Cylinder[C_LoadCarRFIDRotArmD].Off();
            Cylinder[C_LoadCarRFIDRotArmU].On();
            if(bMotOk==false)
                bMotOk=MOT[MLdCarRotArm].MotorMove(0);

            if(bCynOk==false)
                bCynOk=Cylinder[C_LoadCarRFIDRotArmD].OffSensor();

            if(bMotOk && bCynOk)
            {
                Task=500;
            }
            break;
        case 2000:                                                              //放下
            Cylinder[C_LoadCarRFIDRotArmD].On();
            Cylinder[C_LoadCarRFIDRotArmU].Off();
            bCynOk=Cylinder[C_LoadCarRFIDRotArmD].OnSensor();
            if(bCynOk &&
               fTrayMapping->ldRFID->bClearCmd1==true)
            {
                fTrayMapping->ldRFID->sBlockID1="";
                fTrayMapping->ldRFID->sUID1="";
                fTrayMapping->ldRFID->bReadUID1=false;
                fTrayMapping->btnReadUID1->Click();
                Task=2500;
            }
            break;
//        case 2100:
//            if(fTrayMapping->ldRFID->bReadUID==true)
//            {
//                fTrayMapping->btnLdRFIDReadBlock->Click();
//                Task=2500;
//            }
//            break;
        case 2500:
            bMotOk=false;
            bCynOk=false;
            iPosTmp=MOT[MLdCarRotArm].Motor->PSoftLimitN+100;
//            if(iPosTmp<MOT[MLdCarRotArm].Motor->PSoftLimitN)                    //超過軟體極限
//                Task=4000;
//            else
                Task=3000;

            tNFCDelay.SetSecAndOn(0.2);
            fTrayMapping->btnReadUID1->Click();
//            break;
        case 3000:                                                              //一次走5mm 往下找
            if(fTrayMapping->ldRFID->bReadUID1==true ||
               Sen[SnLoadCarRFIDSW].IsOn()==true)
            {
                bMotOk=true;
                MOT[MLdCarRotArm].PCIL132_StopMotor();
            }
            else
            {
                if(tNFCDelay.Off())
                {
                    tNFCDelay.SetSecAndOn(0.2);
                    fTrayMapping->btnReadUID1->Click();
                }
                bMotOk=MOT[MLdCarRotArm].MotorMove(iPosTmp);
            }

            if(bMotOk)
            {
                bMotOk=false;
                bCynOk=false;
                if(fTrayMapping->ldRFID->bReadUID1==true)
                {
                    bReadOK=true;
                    tNFCDelay.SetSecAndOn(0.1);
                    Task=3100;
                }
                else if(Sen[SnLoadCarRFIDSW].IsOn()==true)                      //QQQ need alarm
                {
                    bReadOK=false;
                    asTrayIDDataCorverLoader="NOREAD";                          //AI(W906-E034) 20261003: 0618 acatchtray.cpp:8403 `=="NOREAD"` is a no-op (the ID stays ""); 0625 :8403 / V912 :9088 (function rewritten) `="NOREAD"`; #20 exception (Steven 1003 standing rule; Q82)
                    tNFCDelay.SetSecAndOn(0.1);
                    Task=4000;
                }
//                else
//                {
//                    fTrayMapping->btnReadUID1->Click();
//                    Task=2500;
//                }
            }
            break;
        case 3100:
            if(tNFCDelay.Off())
            {
                fTrayMapping->btnLdRFIDReadBlock1->Click();
                Task=4000;
            }
            else
            {
                break;
            }
        case 4000:                                                              //上抬
            Cylinder[C_LoadCarRFIDRotArmD].Off();
            Cylinder[C_LoadCarRFIDRotArmU].On();

            if(bMotOk==false)
                bMotOk=MOT[MLdCarRotArm].MotorMove(0);

            if(bCynOk==false)
                bCynOk=Cylinder[C_LoadCarRFIDRotArmD].OffSensor();

            if(bMotOk && bCynOk)
            {
                tNFCDelay.SetSecAndOn(0.1);
                if(bReadOK==true)
                {
                    fTrayMapping->btnLdRFIDReadBlock1->Click();
                    Task=4100;
                }
                else
                {
                    asTrayIDDataCorverLoader="NOREAD";                          //AI(W906-E034) 20261003: 0618 acatchtray.cpp:8444 `=="NOREAD"` is a no-op (the ID stays ""); 0625 :8444 / V912 :9166 (function rewritten) `="NOREAD"`; #20 exception (Steven 1003 standing rule; Q82)
                    Task=6000;
                }
            }
            break;
        case 4100:
            if(CylinderLower(C_Load_Up) && tNFCDelay.Off())
            {
                Task=5000;
            }
            else
            {
                break;
            }
        case 5000:
            if(bReadOK==true && fTrayMapping->ldRFID->sBlockID1!="")
            {
                asTrayIDDataCorverLoader=fTrayMapping->ldRFID->sBlockID1;
                fSortCT->pnlLoadCID->Caption=asTrayIDDataCorverLoader;
                iThisPortNo=ePortLoader;
                asBundleTrayID[ePortLoader]=asTrayIDDataCorverLoader;

                if(IniConfig.bEnable_SECS_GEM==true)
                    EventReport(SECS_EVENT.BundleCompleteIDRead);

                iPosTmp=0;
                if(iReadCIDAction==ePortLoader)
                    iReadCIDAction=ePortTotal;
                return true;
            }
            else
            {
                iCount++;
                if(iCount>2)
                {
                    Task=6000;
                    bAlarm=true;
                }
                else
                {
                    Task=3;
                }
            }
            break;
        case 6000:
            if(bAlarm==true)
            {
                if(IniConfig.bEnable_SECS_GEM==true &&
                   fAGV->IsATK_AMR())                                           //Steven 20251210 : ATK AMR
                {
                    iThisPortNo=ePortLoader;
                    iThisPortStatus=iPortStatus[ePortLoader];
                    EventReport(SECS_EVENT.LoaderTrayIDReadFail);
                }

                ret=ShowErrorMessage("WAR16120", K_RETRY|K_SKIP, MTrayX, false);
            }
            else
            {
                ret=K_RETRY;
            }

            if(ret==K_RETRY)
            {
                Task=3;
            }
            else
            {
                asTrayIDDataCorverLoader="NOREAD";                              //AI(W906-E034) 20261003: 0618 acatchtray.cpp:8512 `=="NOREAD"` is a no-op (the ID stays ""); 0625 :8512 / V912 :9245 (function rewritten) `="NOREAD"`; #20 exception (Steven 1003 standing rule; Q82)
                if(iReadCIDAction==ePortLoader)
                    iReadCIDAction=ePortTotal;
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
void AddFixTrayCount()                                                          //Steven 20251029 : outputtray 數量
{
    if(fMain->hanaART->IsHanaArtAvailable()==false ||
       fLotInfo->edtSysLotID->Text=="")
        return;

    if(LastSet.iRealDummy==HAS_TRAY || LastSet.iRealDummy==REALLY)
    {
        for(int i=iFixMin; i<=iFixMax; i++)
        {
            int iFix=iAutoIndex[i];
            if(iBinTray[i])
            {
                if(Sen[SnFixedTrayDetect[iFix]].IsOn() && LastSet.iRealDummy!=DUMMY)
                {
                    AddTrayCount(i);                                            //Steven 20251219 : Haoutputtray 數量
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void AddTrayCount(int iAuto)                                                    //Steven 20251029 : outputtray 數量
{
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        if(LastSet.bLoaderTrayCount_ART==false)
        {
            LastSet.iUnloaderTrayCount_ART[iAuto]++;
        }
    }
    else
    {
        if(fAGV->IsATK_AMR()==true &&                                           //RogerYang 20260403 : fix sorting do not add auto count
           (LastSet.iUnloadFixTray==eAtkTfFeedFix ||
           LastSet.iUnloadFixTray==eAtkTfFixToAMR))
        {
        }
        else
        {
            LastSet.iUnloaderTrayCount_ART[iAuto]++;
        }
    }

    if(fAGV->IsATK_AMR())                                                       //Steven 20251210 : ATK AMR
    {
        fSortCT->pnlTrayCnt[iAuto]->Caption=AnsiString(LastSet.iUnloaderTrayCount_ART[iAuto]);
    }

    if(fLotInfo->edtSysLotID->Text!="")
        fMain->hanaART->AddNewTrayHead(iAuto);                                  //Steven 20250414 : HANA ART Function
}
//------------------------------------------------------------------------------
void ClearAllTrayCount()                                                        //Steven 20251029 : outputtray 數量
{
    if(CUSTOMER_CODE==CC_KYEC_LEE)
    {
        if(LastSet.bLoaderTrayCount_ART==false)
        {
            for(int i=0; i<eTrayCount; i++)
            {
                LastSet.iUnloaderTrayCount_ART[i]=0;
            }
        }
    }
    else
    {
        for(int i=0; i<eTrayCount; i++)
        {
            LastSet.iUnloaderTrayCount_ART[i]=0;
        }
    }

    if(fAGV->IsATK_AMR())                                                       //Steven 20251210 : ATK AMR
    {
        for(int i=0; i<MAX_AUTO_TRAY; i++)
        {
            fSortCT->pnlTrayCnt[i]->Caption="0";
        }
    }
}
//------------------------------------------------------------------------------
//==> Eastsun 20260515 F011 整合:KYEC AMR cover-tray subsystem (9 funcs)
//==============================================================================
// 從 LoadStage吸空TRAY流程
//==============================================================================
int CheckCoverTrayIDTask=0;
void InitialCheckCoverTrayIDTask()
{
    CheckCoverTrayIDTask=1;
}
//------------------------------------------------------------------------------
bool CheckCoverTrayID()
{
    //static int Target=0, TargetBuf=0;  //Eastsun 20260522 W8080
    int iBufferTaryPoa[2]={Prod.iXTrayColor, Prod.iXTrayEmpty};
    int &Task=CheckCoverTrayIDTask;
    #ifdef SOFT_SIMULTE
    return true;
    #endif
    switch(Task)
    {
        case 1:
            if(TrayArmMotorMove(iBufferTaryPoa[TrayForm.iReaderPos]))
            {
                InitialCoverTrayIDTask(iKeyenceCoverTrayID_Color);
                Task=100;
            }
            break;
        case 100:
            if(TrayForm.iReaderPos==0)
            {
                if(Sen[SenEmptyHasTray].IsOn() && Sen[SenEmptyCWDete].IsOn())
                {
                    Task=200;
                }
            }
            else if(TrayForm.iReaderPos==1)
            {
                if(Sen[SenColorHasTray].IsOn() && Sen[SenColorCWDete].IsOn())
                {
                    Task=200;
                }
            }
            break;
        case 200:
            if(fTrayMapping->DoCoverTrayIDKeyence(iKeyenceCoverTrayID_Color)==true)
            {
                return true;
            }
            break;
    }

    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer CatchCoverTrayDelay;
int CatchCoverTrayTask=0;
void InitialCatchCoverTrayTaskTask()
{
    CatchCoverTrayTask=1;
}
//------------------------------------------------------------------------------
int CatchCoverTray(int iWhichAuto)
{
    //static int Target=0, TargetBuf=0;  //Eastsun 20260522 W8080
    int iBufferTaryPoa[2]={Prod.iXTrayEmpty, Prod.iXTrayColor};
    //int iCylinder[5];  //Eastsun 20260522 W8080
    int &Task=CatchCoverTrayTask;  //ret removed  //Eastsun 20260522 W8080
    switch(Task)
    {
        case 1:
            if(IsOutArmSafe())
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
                Task=1000;
            }
            else
            {
                MOT[MOutArmX].fCanMove=true;
                MOT[MOutArmY].fCanMove=true;
                Task=100;
            }
            break;
        case 100:
            if(MoveOutArmToAutoSafe_9045())
            {
                Task=200;
            }
            break;
        case 200:
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
                Task=1000;
            }
            break;
        case 1000:
            if(TrayArmMotorMove(iBufferTaryPoa[iWhichAuto]))
            {
                Task=1200;
            }
            break;
        case 1200:
            if(iWhichAuto==eEmpty)
                Cylinder[C_Empty_Fix].Off();
            else if(iWhichAuto==eColor)
                Cylinder[C_Color_Fix].Off();

            Task=1300;
            break;
        case 1300:
            if(USE_AUTO_RETEST==eartInstall && Cylinder[C_TrayCover].Enable) //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
            {
                Cylinder[C_CatchTray_FixOn].Off();
                Cylinder[C_CatchTray_FixOff].On();
                Cylinder[C_TrayCover].Off(); //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
            }
            else
            {
                Cylinder[C_CatchTray_Fix].Off();
            }

            Cylinder[C_TrayX_UpDown].On();
            CatchCoverTrayDelay.SetMSAndOn(1000);
            Task=1400;
            break;
        case 1400:
            if(CatchCoverTrayDelay.Off())
            {
                #ifdef SOFT_SIMULTE                                             //Eastsun 20260116
                Cylinder[C_TrayCover].Enable = true;
                #endif
                if(USE_AUTO_RETEST==eartInstall && Cylinder[C_TrayCover].Enable)
                {
                    Cylinder[C_CatchTray_FixOn].Reset();
                    Cylinder[C_CatchTray_FixOn].Off();
                    Cylinder[C_CatchTray_FixOff].Off();
                    Cylinder[C_TrayCover].Off();
                    Task=1500;
                }
            }
            break;
        case 1500:
            if(Cylinder[C_CatchTray_FixOn].Push())
            {
                Cylinder[C_TrayCover].On();
                Task=1600;
            }
            break;
        case 1600:
            if(IsTrayArmCatchTrayFail())
            {
                Task=10000;
            }
            else if(Cylinder[C_TrayX_UpDown].Pop())
            {
                Task=1700;
            }
            break;
        case 1700:
            CatchTraySuck.SetItemData(0, 0, 1+iWhichAuto);
            MOT[MTrayX].fHasTray=true;
            if(AUTO_EMPTY_COLOR)
            {
                if(iWhichAuto==eColor)//kevin 20120718
                {
                    NewRecordProcess("MES0652", "Tray Arm - Pick tray from Color", "CatchNewTrayFromBuffer");   //Steven 20220331 : Tray Arm動作加上紀錄
                    MOT[MMColor].ClearTray(__FUNC__);
                }
                else if(iWhichAuto==eEmpty)//kevin 20120718
                {
                    NewRecordProcess("MES0651", "Tray Arm - Pick tray from Empty", "CatchNewTrayFromBuffer");   //Steven 20220331 : Tray Arm動作加上紀錄
                    MOT[MMEmpty].ClearTray(__FUNC__);
                }
            }

            if(AUTO3_IS_MAGAZINE==1 && iWhichAuto==3)   //JerryYang 20230509 : Magazine production log
            {
                ASE_OutTrayNum[iWhichMag+eMag1]++;
                LastSet.iOutTrayNum[iWhichAuto-1]=ASE_OutTrayNum[iWhichMag+eMag1];
            }
            else
            {
                ASE_OutTrayNum[iWhichAuto-1]++;//kevin 20140918 outputtray 數量
                LastSet.iOutTrayNum[iWhichAuto-1]=ASE_OutTrayNum[iWhichAuto-1];
            }
            return 1;
        case 10000:
            return 2;
    }
    return 0;
}
//------------------------------------------------------------------------------
TQPF_Timer PlaceCoverTrayDelay;
int PlaceCoverTrayTask=0;
void InitialPlaceCoverTrayTask()
{
    PlaceCoverTrayTask=1;
}
//------------------------------------------------------------------------------
bool PlaceCoverTray(int iWhichAuto)
{
    //static int Target=0, TargetBuf=0;  //Eastsun 20260522 W8080
    //int iBufferTaryPoa[2]={Prod.iXTrayEmpty, Prod.iXTrayColor};  //Eastsun 20260522 W8080
    //int iCylinder[5];  //Eastsun 20260522 W8080
    int &Task=PlaceCoverTrayTask;  //ret removed  //Eastsun 20260522 W8080
    switch(Task)
    {
        case 1:
            if(IsOutArmSafe())
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
                Task=1000;
            }
            else
            {
                MOT[MOutArmX].fCanMove=true;
                MOT[MOutArmY].fCanMove=true;
                Task=100;
            }
            break;
        case 100:
            if(MoveOutArmToAutoSafe_9045())
            {
                Task=200;
            }
            break;
        case 200:
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                if(TRAY_ARM_MODE==eAboveCoveyor)
                {
                    MOT[MOutArmX].fCanMove=false;
                    MOT[MOutArmX].PCIL132_StopMotor();
                    MOT[MOutArmY].fCanMove=false;
                    MOT[MOutArmY].PCIL132_StopMotor();
                }
                Task=1000;
            }
            break;
        case 1000:
            if(TrayArmMotorMove(Prod.iXTrayAuto[iWhichAuto]))
            {
                Task=1200;
            }
            break;
        case 1200:
            if(MOT[MMAuto1_Car+iWhichAuto].fHasTray==false && MOT[MMAuto1+iWhichAuto].fHasTray==false)
            {
                Cylinder[C_TrayX_UpDown].On();
                PlaceCoverTrayDelay.SetMSAndOn(1500);
                Task=1300;
            }
            else
            {
                    DoAutoReceiveBinTray(iWhichAuto);
                }
            break;
        case 1300:
            if(PlaceCoverTrayDelay.Off())
            {
                if(USE_AUTO_RETEST==eartInstall && Cylinder[C_TrayCover].Enable) //ChungHung 20150529 fix ATK AutoRetest 只有安裝軌道 //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                {
                    Cylinder[C_CatchTray_FixOn].Off();
                    Cylinder[C_CatchTray_FixOff].On();
                    Cylinder[C_TrayCover].Off(); //ChungHung 20140624 add AutoRetest catch Tray use 2 Output
                }
                else
                {
                    Cylinder[C_CatchTray_Fix].Off();
                }

                Task=1400;
            }
            break;
        case 1400:
            if(Cylinder[C_TrayX_UpDown].Pop())
            {
                CatchTraySuck.iWhichKit=-1;  //Steven 20220331 : 紀錄Tray從哪來的
                CatchTraySuck.SetItemData(0, 0, NULL_IC);
                MOT[MTrayX].fHasTray=false;
                MOT[MMAuto1+iWhichAuto].SetTray(HAS_IC, __FUNC__);
                bAutoEdgePush[iWhichAuto]=true;
                Task=1500;
            }
            break;
        case 1500:
            if(TrayArmMotorMove(Prod.iXTrayEmpty))
            {
                MOT[MOutArmX].fCanMove=true;//OUT ARM 可以移動
                MOT[MOutArmY].fCanMove=true;
                Task=1600;
            }
            break;
        case 1600:
            if(MOT[MMAuto1_Car+iWhichAuto].fHasTray==false && MOT[MMAuto1+iWhichAuto].fHasTray==false)
            {
                return true;
            }
            else
            {
                    DoAutoReceiveBinTray(iWhichAuto);
                }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
TQPF_Timer ReceiveCoverTrayDelay;
int ReceiveCoverTrayTask=0;
void InitialReceiveCoverTrayTask()
{
    ReceiveCoverTrayTask=1;
}
//------------------------------------------------------------------------------
bool ReceiveCoverTray(int iWhichAuto)
{
    AnsiString str;
    int &Task=ReceiveCoverTrayTask;  //ret removed  //Eastsun 20260522 W8080
    switch(Task)
    {
        case 1:
            if(Cylinder[C_Auto1_Selector+iWhichAuto].OffStatus()==false)
            {
                Task=5000;
            }
            else if(Sen[SnAuto1PreDete+iWhichAuto].IsOff()==false ||
                    Sen[SnAuto1_Tray_Car+iWhichAuto].IsOff()==false ||
                    (Sen[SnAutoUpSafedetect0+iWhichAuto].Enable && Sen[SnAutoUpSafedetect0+iWhichAuto].IsOff()==false) ||
                    (Sen[SnAuto1_Detect+iWhichAuto].Enable && Sen[SnAuto1_Detect+iWhichAuto].IsOff()==false))
            {
                str.sprintf("Please Remove Auto %d Tray", iWhichAuto+1);
                ShowMyMessage(str);
            }
            else
            {
                AutoCylinderLower(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto,true);
                Task=100;
            }
            break;
        case 100:
            if(AutoCylinderLower(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto))
            {
                Task=200;
            }
            break;
        case 200:
            TrayMoveOut(true, iWhichAuto+3);
            if(Sen[SnAuto1PreDete+iWhichAuto].IsOff()==false && Sen[SnAuto1_Tray_Car+iWhichAuto].IsOff()==false)//Ifor 20200110 add: Auto減速與到位Sensor一起亮才可上升，避免Sensor異常
            {
                AutoCylinderMiddle(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto, true);    //Steven 20140409 : AutoCylinderXX Add Reset
                Task=300;
            }
            break;
        case 300:
            if(AutoCylinderMiddle(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto))
            {
                TrayMoveOut(false, iWhichAuto+3);
                Task=400;
            }
            break;
        case 400:
            if(Cylinder[C_Auto1LoaderZ_Select+iWhichAuto].Push())
            {
                AutoCylinderUp(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto, true);
                Task=500;
            }
            break;
        case 500:
            if(AutoCylinderUp(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto))
            {
                Task=600;
            }
            break;
        case 600:
            if(Cylinder[C_Auto1LoaderZ_Select+iWhichAuto].Pop()) //分離氣缸關閉    //Steven 20140408
            {
                AutoCylinderLower(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto, true);
                Task=700;
            }
            break;
        case 700:
            if(AutoCylinderLower(iWhichAuto, C_Auto1_Up+iWhichAuto, C_Auto1_Selector+iWhichAuto))
            {
                return true;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------
int CoverTrayTask=1;
void InitialCoverTrayTask()
{
    CoverTrayTask=1;
}
//------------------------------------------------------------------------------
bool DoCoverTray(int iAuto)
{
    static int Target=0;  //TargetBuf=0 removed  //Eastsun 20260522 W8080
    int ret, &Task=CoverTrayTask;
    AnsiString str;

    int iWhichAuto=iAuto-1;

    if(TrayForm.bEnableAMR) //Eastsun 20260420合code
    {
         if(bLoaderLockActionFlag[1]==false)
             DoAutoEmpty();
         if(bLoaderLockActionFlag[2]==false)
             DoAutoColor();
    }
//    if(ReceiveCoverTray(iWhichAuto))
//    {
//        iUnloaderTrayCountCal[iWhichAuto]++;
//        if(asColorCoverTrayID!="")
//        {
//            if(iWhichAuto==0)
//                fLotInfo->labNowAuto1TrayID->Caption=asColorCoverTrayID;
//            else if(iWhichAuto==1)
//                fLotInfo->labNowAuto2TrayID->Caption=asColorCoverTrayID;
//            else if(iWhichAuto==2)
//                fLotInfo->labNowAuto3TrayID->Caption=asColorCoverTrayID;
//
//            asColorCoverTrayID="";
//        }
//    }
    switch(Task)
    {
        case 1:
            if(IsOutArmSafe())
            {
                bByPassAutoLock=true;
                if(MOT[MMAuto1+iWhichAuto].fHasTray==false && MOT[MMAuto1_Car+iWhichAuto].fHasTray==false)
                {
                    Target=0;
                    Task=100;
                }
            }
            break;
        case 100:
            if(TrayForm.bIDTrayOrder[Target]==eUse1DCoverTray)
            {
                InitialCheckCoverTrayIDTask();
                Task=1000;
            }
            else
            {
                InitialCatchCoverTrayTaskTask();
                Task=1500;
            }
            break;
        case 1000:
            if(CheckCoverTrayID())
            {
                InitialCatchCoverTrayTaskTask();
                Task=1500;
            }
            break;
        case 1500:
            ret=CatchCoverTray(TrayForm.iTrayOrder[Target]);
            if(ret==1)
            {
                InitialPlaceCoverTrayTask();
                Task=1550;
            }
            else if(ret==2)
            {
                //error
            }
            break;
        case 1550:
            if(bUnLoaderActionFlag[iWhichAuto]==false)
            {
                InitialPlaceCoverTrayTask();
                Task=1600;
            }
            else
            {
                fMain->Pause("DoCoverTray");
                str.sprintf("Auto %d Full, Wait for AMR",iWhichAuto+1);
                RecordProcess(str);
                if(TrayForm.bEnableAMR) //Eastsun 20260420合code
                {
                    #ifdef SOFT_SIMULTE
                    bUnLoaderSECSActionFlag[0]=true;
                    fLotInfo->InitialUnLoaderTask(0);
                    bUnLoaderSECSActionFlag[1]=true;
                    fLotInfo->InitialUnLoaderTask(1);
                    bUnLoaderSECSActionFlag[2]=true;
                    fLotInfo->InitialUnLoaderTask(2);
                    #endif
                }
            }
            break;
        case 1600:
            if(PlaceCoverTray(iWhichAuto))
            {
                Target++;//Eastsun 20260420合code
                Task=1700;
            }
            break;
        case 1700:
            //Target++;
            if(TrayForm.iTrayOrder[Target]==eNoUse || Target>=2)
            {
                bByPassAutoLock=false;
                //if(MOT[MMAuto1_Car+iWhichAuto].fHasTray==false && MOT[MMAuto1+iWhichAuto].fHasTray==false)
                   //&& MOT[MMEmpty].fHasTray==true &&  MOT[MMColor].fHasTray==true) //Eastsun 20260116 : trayFeed 就不要看有沒有盤了
                    return true;
            }
            else
            {
                Task=100;
            }
            break;
    }
    return false;
}
//------------------------------------------------------------------------------

//<== Eastsun 20260515 F011
