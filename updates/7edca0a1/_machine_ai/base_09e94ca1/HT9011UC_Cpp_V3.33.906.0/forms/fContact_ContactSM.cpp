// =============================================================================
//  forms/fContact_ContactSM.cpp -- E-042 = E-038 Phase B, step B2 (SAFETY): golden's contact-mode switch
//  TfContact::SetContactMode (golden 0618 cContact.cpp:15341-15434).  Step B4 adds the contact master state
//  machine TfContact::DoTestContactFunction (golden :11755-13963) to this TU.
//
//  AI(W906-E042) 20261004 (St01): NEW FILE. Golden source of truth:
//  D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp (cp950, decoded strictly, 0 U+FFFD).
//  Plan: D:\AI_TempFile\st01e-e042-plan-20261004.md (s1.1 row 6, s7 B2).  Generator (re-run is byte-identical):
//  D:\AI_TempFile\st01e-e042-run\gen_contactsm.py.
//
//  *** NO CALLER -- ZERO RUNTIME CHANGE ***
//    golden calls SetContactMode from FormShow :1163, FormClose :1872, rbModeNormalClick :15338 (forms/fContact.h
//    GATE (X-08) / (X-09) / (X-11), none defined in the port) and DoTestContactFunction case 1 :11790 (step B4, not
//    translated; MainProc's call is #if 0, csystem.cpp:31320-31322).  So nothing references this object and the
//    linker does not pull it into wb_serve.  tests/test_indexz_autoheight_1203.cpp [B13] pins "0 live callers
//    outside this TU": that census replaces the linker interlock of GATE (X-10) (forms/fContact.h:1464 still says
//    GATE (X-10): the laptop's line, not claimed by E-042 B2 -- FYI in the B2 report).
//
//  WHY IT IS A SAFETY ITEM: it is a MODE SWITCH (forms/fContact.h:441-446).  It writes the global iContactMode
//  (cmydef.cpp:3346), which MainProc reads live (csystem.cpp:31302: Contact page open && iContactMode != CONTACT_NORMAL
//  -> DoServoOn) and which every contact / auto-height state machine branches on.  It issues no motion, IO, ini
//  write or message itself; [B13] pins that the body stays UI state only.
//
//  VERBATIM: the golden span below is copied byte for byte (cp950 -> UTF-8), no line changed:
//  SetContactMode span: port line = golden line + (-15232).  [B13] pins every code line against golden.
//  V912 (:15566-15696) differs only by FEATURES, so 0618 is kept (the #20 rule is for fixes): 4 x the ASE_CL
//  contact-height greying (Chrischen 20260316; E-030 / RULINGS_20261002 #23-6 already ruled "906, not done") and the
//  VISUAL_DETECTION_TEST arm (mode 12; Q-C open; not in cContact.h).
//
//  HT9050 (PCI-1203 Index Z): golden lets the operator pick any mode here, and so does this copy.  Refusing Manual
//  Height (2), Load Cell (8) and every mode other than 1 / 3 is P8 (W906_IndexZRunRefused, IndexZTorque1203.cpp),
//  run at DoTestContactFunction case 1 AFTER this call and before anything energises (B4; Steven 1004 08:4x).
//  [B13] pins that chain on this function's output.  Panasonic / RS-232 rows: every mode passes, as golden.
//
//  FOR B4: this member reads the TfContact FACADE radios (forms/fContact.h:1089-1229), which the web does not drive.
//  The web drives the C-route copies (FileRW/DeviceForm_File.cpp g_ct3aRb -> DF_rbModeNormalClick ->
//  DF_SetContactMode in FileRW/DeviceForm_File.gen.inc, V912 base incl. mode 12), which also write iContactMode.
//  The facade radios start unchecked (vclcompat TRadioButton(); the facade ctor is fields only), so this member
//  then leaves iContactMode as the web set it (golden: no arm fires) -- [B13] pins that.  B4 decides the bridge.
// =============================================================================
#include "forms/fContact.h"
#include "cContact.h"               // CONTACT_* (CONTACT_TEST since W906-E042 B1, cContact.h:160)
#include "MachineType.h"            // CC_KYEC_CHEN / CC_KYEC_LEE / CC_KYEC_XILINX / CC_MAXIM_THAILAND
#include "cmydef.h"                 // iContactMode (:3107) / CUSTOMER_CODE (:3186)
#include "Config.h"                 // IniConfig.bA16ContactTestDropContact
//  ---- AI(W906-E042) 20261005 (B4, St01): DoTestContactFunction (golden :11752-13963) + Do_LoadCellAutoHigh (:17197-17997) ----
//  Generator: D:\AI_TempFile\st01e-e042-run\gen_b4.py (re-run byte-identical).  STILL NO CALLER: MainProc's call is #if 0
//  (csystem.cpp:31320-31322, step B6); tests/test_indexz_autoheight_1203.cpp [B9] pins 0 live callers of
//  DoTestContactFunction / Do_LoadCellAutoHigh outside the E-042 TUs.  Port changes (each on its golden line, AI(W906-E042)):
//    case 1 :11790  the contact-mode SINGLE ENTRY (V912 DF_SetContactMode on the web's radios, FileRW/DeviceForm_File.cpp EOF);
//    case 1 :11819  run entry: untranslated modes refused on every machine (4 / 5 / 9 / 10 / 11; 12 = TODO E-052), P4 + P8 + P7
//                   (PCI1203 Index Z), W-44 run entry, place-back reset -- all before anything energises;
//    switch :11786  P7 every tick except case 1;  case 900 :13350  the HT9050 place-back step (Steven 1005 09:2x Q101);
//    leaf state machines that are not translated (plan s1.2) -> E042Leaf("name"): message + SystemStart=false +
//    CarlibrationTask=1 (the run stops; nothing is skipped silently).  Do_LoadCellAutoHigh: P7 + case-1 refusal on HT9050.
//    SetContactMode (B2, above) keeps its offset; the spans below carry their own offsets (marker lines).
#include "forms/fContact_AutoHeight.h"  // FileRW_Contact_SetContactModeSingleEntry
#include "IndexZTorque1203.h"       // P4 / P7 / P8 / W-44 / place-back (AI(W906-E042))
#include "MachineDefine.h"
#include "atester.h"
#include "atester_shims.h"
#include "aTester_Front.h"
#include "aArmHeader.h"             // __FUNC__
#include "Motor/mymotor.h"
#include "Motor/HTMotor.h"
#include "mysensor.h"
#include "myswitch.h"
#include "csystem.h"
#include "cprod.h"
#include "cpublic.h"
#include "cUnitConvert.h"
#include "common.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "aHotPlateSubstrate.h"     // THE aHotPlateSubstrate.h TMyKitSuck
#include "FormsFacade.h"            // fMain
#include "canary_support.h"         // ShowMyMessage
#include "mycylin.h"
#include "ckernel.h"
#include "cinitial.h"
#include "MessageDef.h"
#include "Motor/myGALILmotor.h"
#include "acarry_shims.h"
#include "acarry.h"
#include "atester_ProcessCount.h"
#include "forms/fATCHandlerSide.h"
#include "ATC/ATCInterface.h"
#include "forms/fHome.h"
#include "ainarm9045_2x4_16_shims.h" // AutoTeachLoadTrayZ / iInArmZTeachTask
#include "acatchtray.h"             // IsTrayArmAtEmptyOrColor
#include "aoutarm.h"                // MoveOutArmXY_ToFix_Tray_Full
#include "asendic_Loader_RT.h"       // DoAutoLoaderReceive
#include "forms/fShuttleMove.h"     // fShuttleMove (In-shuttle latch teach, golden :2196-2198)
#include <cstdlib>
#include <cmath>
void CheckInArmDestroyActive();     // ainarm9045.h:103 (not included: its InArmLeftSide* default arguments collide in this TU)
bool MoveInArm2XYToShuttle2Wait();  // acatchtray_shims.h:413 (not included: clWhite redeclaration in this TU)
extern int iOutArmZTeachTask;       // REAL def aoutarm_shims.cpp:33 (ht9045_sm), golden AutoTeach.h:235 (as aoutarm.cpp:517)
//  A call of a leaf state machine that is not translated yet: stop the contact run with a message (NOT GOLDEN).
static bool E042Leaf(const char* fn)
{
    ShowMyMessage(AnsiString("Contact run stopped: ") + fn + " is not translated in the port yet (E-042 leaf)",
                  AnsiString("接觸流程已停止: ") + fn + " 尚未移植", "E042Leaf");
    SystemStart=false;
    if(fContactForm) fContactForm->CarlibrationTask=1;
    return false;
}
//  golden 0618 cContact.cpp:15341-15434, verbatim:
void TfContact::SetContactMode()                                                //Steven 20150224 : Auto Contact Test
{
    if(rbModeNormal->Checked)
    {
        iContactMode=CONTACT_NORMAL;
        if(CUSTOMER_CODE==CC_KYEC_CHEN &&
           IniConfig.bA16ContactTestDropContact)                                //wei 20150831
        {
            ChangeContactMode(false);
        }
        Memo1->Lines->Add("CONTACT_NORMAL");
    }
    else if(rbDeviceMapping->Checked)                                           //Steven 20190910 : Qualcomm功能
    {
        iContactMode=CONTACT_DEVICE_MAP_CHECK;
        Memo1->Lines->Add("CONTACT_DEVICE_MAP_CHECK");
    }
    else if(rbAutoHeight->Checked)
    {
        iContactMode=CONTACT_AUTO_GET_HEIGHT;
        Memo1->Lines->Add("CONTACT_AUTO_GET_HEIGHT");
    }
    else if(rbManualHeight->Checked)
    {
        iContactMode=CONTACT_MANUAL_GET_HEIGHT;
        Memo1->Lines->Add("CONTACT_MANUAL_GET_HEIGHT");
    }
    else if(rbContactTest->Checked)
    {
        iContactMode=CONTACT_TEST;
        Memo1->Lines->Add("CONTACT_TEST");
        if(CUSTOMER_CODE==CC_KYEC_CHEN &&
           IniConfig.bA16ContactTestDropContact)                                //wei 20150831
        {
            ChangeContactMode(true);
        }
    }
    else if(rbAutoContactTest->Checked)
    {
        iContactMode=AUTO_CONTACT_TEST;
        Memo1->Lines->Add("AUTO_CONTACT_TEST");
    }
    else if(rbStepContactTest->Checked)                                         //Steven 20150811 : Step by Step Contact Test
    {
        iContactMode=STEP_CONTACT_TEST;
        Memo1->Lines->Add("STEP_CONTACT_TEST");
    }
    else if(rbLoadCellAutoHigh->Checked)                                        // Step by Step Contact Test
    {
        iContactMode=CONTACT_LoadCell_AUTO_GET_HEIGHT;                          //kevin 20190909 add Load cell AutoHigh
        Memo1->Lines->Add("CONTACT_LoadCell_AUTO_GET_HEIGHT");
    }
    else if(rbDeviceLoopTest->Checked)                                          //Ztex 2023.11.19 Add CONTACT_DEVICE_LOOP_TEST
    {
        iContactMode=CONTACT_DEVICE_LOOP_TEST;
        Memo1->Lines->Add("CONTACT_DEVICE_LOOP_TEST");
    }
    else if(rbKTempIndexMove->Checked)                                          //Ztex 2024.03.26 Add Contact Mode K Temperature
    {
        iContactMode=K_TEMP_INDEX_MOVE;
        Memo1->Lines->Add("K_Temp_Index_Move");
        chk_K_Temperature->Checked=true;
    }

    if(CUSTOMER_CODE==CC_KYEC_LEE ||
       CUSTOMER_CODE==CC_KYEC_XILINX ||
       CUSTOMER_CODE==CC_MAXIM_THAILAND)
    {
        if(iContactMode==CONTACT_AUTO_GET_HEIGHT)
        {
            cbOneTouchAutoContactHight->Visible=true;
        }
        else
        {
            cbOneTouchAutoContactHight->Visible=false;
            cbOneTouchAutoContactHight->Checked=false;                          //Ifor 20220803 add 一鍵完成Auto Contact Hight
        }
    }
    else
    {
        cbOneTouchAutoContactHight->Visible=false;
        cbOneTouchAutoContactHight->Checked=false;                              //Ifor 20220803 add 一鍵完成Auto Contact Hight
    }

    if(iContactMode==STEP_CONTACT_TEST)
    {
        chkDailyCorrelation->Enabled=true;
    }
    else
    {
        chkDailyCorrelation->Enabled=false;
        chkDailyCorrelation->Checked=false;
    }
}
//  end of golden :15341-15434 (golden :15435-15454 = ChangeContactMode, already ACTIVE in forms/fContact.cpp:925)

// ===== AI(W906-E042) 20261005 (B4): golden 0618 cContact.cpp:11752-13963 (timers WaitTime / iRealTimeCCD + DoTestContactFunction), verbatim: DoTestContactFunction span: port line = golden line + (-11546) =====
//------------------------------------------------------------------------------
TQPF_Timer WaitTime;
TQPF_Timer iRealTimeCCD;                                                        //----- by dell ccd realtime-------------
void TfContact::DoTestContactFunction()   //AI(W906-E042) 20261005 (B4): golden `void __fastcall` -- __fastcall dropped as in forms/fContact.cpp (the declaration has none)
{
    static int iHeaterWaitingTime=0, iCont=0;
    static int iNowSec, iOldSec;
    static int iRetry=0;
    static int iTestContactCount=0;                                             //KevinCheng 20260115 : NV Contact Test
    static bool bf[9]={false, false, false, false, false, false, false, false, false};

    int ret=0;
    int iSiteOn[4]={0, 0, 0, 0};
    int &Task=CarlibrationTask;
    AnsiString Str;
    AnsiString sTime=Now().FormatString("hh:nn:ss");
    TDateTime tSoakTime;                                                        //Frank 20160305 SoakTime prompt
    bool bHasDevice=false;

    if(SoftStop || SystemStart==false)
        return;

    CheckInArmDestroyActive();                                                  //JerryYang 20221118 : add in arm vacuum detect
    SW[SwRKOneCycle].OnOff(bContinueContact);
    //==> Eastsun 20260526 #026-1.24 Ifor 20230608 add:KYEC 要求新增ATC 溫度等待功能
    if(false /*GATE(W906-E042-HS) FormHS->CheckATCTempWait()==true -- AI(W906-E042): no TFormHS instance in the port (FormHS is the SCK_ART stub, forms/fHS.h:64-86); its body is CC_KYEC_LEE-only, so for every other customer this is exact (same gate as csystem.cpp:33374)*/ &&
       CUSTOMER_CODE==CC_KYEC_LEE)             //Eastsun 20260617 always return
    {
        return;
    }
    //<== Eastsun 20260526 #026-1.24

    static bool bRunDailyCorrelation=false;                                     //KaiHuang 20200606

    if(Task!=1 && W906_IndexZDriveFaultStop("DoTestContactFunction")) { fAllMotorHome=false; CarlibrationTask=1; return; }   switch(Task)   //AI(W906-E042) 20261005 (B4) P7 NOT GOLDEN: M14 drive error -> ST + message + exit (re-HOME); case 1 is the run entry below, which refuses without sending anything
    {
        case 1:                                                                                                         //初始化
            Memo1->Lines->Clear();
            if(W906_ContactModeSingleEntryHook) W906_ContactModeSingleEntryHook(-1); else SetContactMode();   //AI(W906-E042) 20261005 (B4) [906] golden 0618 :11790 `SetContactMode();` (0618 :15341-15434 on the form's radios, also in this TU); [V912] DF_SetContactMode (V912 cContact.cpp:15566-15696, incl. mode 12) on the web's radio state, mirrored onto the facade (FileRW/DeviceForm_File.cpp EOF, through W906_ContactModeSingleEntryHook; golden SetContactMode() only if the C-route is not linked); [why] Steven 1003 standing rule + ST01-M 1005: one entry = the mode the web picked                                                                                            //Steven 20150224 : Auto Contact Test
            if(iContactMode==STEP_CONTACT_TEST && chkDailyCorrelation->Checked)                                         //KaiHuang 20200606
            {
                chkDailyCorrelation->Enabled=false;
                bRunDailyCorrelation=true;
                MOT[MMDailyCorrelationKit].Tray.XItem=4;
                MOT[MMDailyCorrelationKit].Tray.YItem=2;
//                MOT[MMDailyCorrelationKit].SetTray(HAS_IC);

                bUseTwoArm=!TestIF.iShuttleMode;
                if(bUseTwoArm)
                    iRunWhichArm=0;
                else
                    iRunWhichArm=TestIF.iShuttle_Sel;
            }
            else
            {
                bRunDailyCorrelation=false;
                bUseTwoArm=false;
                iRunWhichArm=0;
            }
            bDoRTCLearning=false;                                                                                       //Ifor 20260226 add: Contact Mode 執行RTC Learn 不開啟Hot Air

            if(iContactMode==CONTACT_NORMAL)
            {
                ShowMyMessage("Must Exit Contact Form", "請先離開contact畫面");
                SystemStart=false;
                return;
            }
            if(W906_ContactModeUntranslatedRefused(iContactMode, "DoTestContactFunction 1")) { SystemStart=false; return; }   if(W906_IndexZRunRefused(iContactMode, 0, "DoTestContactFunction 1")) { SystemStart=false; return; }   if(W906_IndexZShuttlesHomeRefused("DoTestContactFunction 1")) { SystemStart=false; return; }   W906_IndexZPlaceBackReset();   InitialTestHeadMotorTask();   /*AI(W906-E042) 20261005 (B4) NOT GOLDEN run entry, after golden's CONTACT_NORMAL refusal (:11813-11818) and before anything energises (:11819-11843): untranslated modes on EVERY machine (4 / 5 / 9 / 10 / 11 leaves not ported; 12 VISUAL_DETECTION_TEST TODO E-052: port V912 Task 60); P4 + P8 + P7 (PCI1203 Index Z: CONFIRMED / modes 1 and 3 only / single arm / drive health); W-44 run entry (In / Out shuttle at their taught clear positions); a new run resets the place-back step*/                                                                                  //Steven 20160520 : 移到下面,避免測試中進來被重置

            if(IndexHasIC() ||                                                                                          //JerryYang 20251118 : 機台有IC不能切換模式
               InArmSuck.HasIC() ||
               OutArmSuck.HasIC() ||
               ShuttleHasIC())
            {
                ShowMyMessage("Please finish ONE CYCLE before Contact Test!", "請先完成ONE CYCLE再執行Contact test!");
                return;
            }

            cbContactMode->Enabled=false;                                                                               //Steven 20100406 : 避免做到一半被更換
            cbVacuumMode->Enabled=false;
            if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                                                  //Steven 20210810 : Qualcomm功能
                SetMotorSpeed();                                                                                        //Steven 20210813 : CONTACT_DEVICE_MAP_CHECK速度
            else
                SetAllMotorSpeed(10);
            if(fAllMotorHome==false)                                                                                    //motor not in home
            {
                ShowMyMessage("Must home first", "請先歸零");
                SystemStart=false;
                return ;
            }
            SW[SwTesterAirCooling].On();                                                                                //Steven 20160714
            ADAM_WriteMaxData(true);                                                                                    //Steven 20241014 : 整合auto height輸出壓力
            ZeroMemory(bf, sizeof(bf));

            if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70 &&
               Temperature.bATC7TSDFunction==true)                                                                      //Steven 20161201 : by site TSD for Contact Test
            {
                iSiteOn[0]=0;
                iSiteOn[1]=0;
                iSiteOn[2]=0;
                iSiteOn[3]=0;
                { (void)iSiteOn; } /*GATE(W906-E042-ATC) ATC_InterfaceForm->UseTSD_Function(4, iSiteOn); -- AI(W906-E042): ATC_InterfaceForm is TATC_InterfaceFormShim (acarry_shims.h:115, iATC_MODE_TYPE only); the real TATC_InterfaceForm (forms/fATCHandlerSide.h) has no global -- same gate as atester.cpp R04 / T04 (W906-GB-P2b); HT9050 USE_ATC_MODE=5 = no ATC*/
            }

            if((CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_CL) &&                                         //JerryYang 20250120 : add
               iContactMode==CONTACT_AUTO_GET_HEIGHT &&
               chkTeachInOutArmZ->Checked)                                                                              //kevin 20171205 add auto teach in out arm Z
            {
                AutoTeachLoadTrayZ(true, 0, iInArmZTeachTask);
                AutoTeachLoadTrayZ(true, OutArm, iOutArmZTeachTask);
                bOutarmAutoHigh=true;
                bLoadInarmAutoHigh=true;
            }
            fMain->LightOn();
            if(CUSTOMER_CODE==CC_ASE_SG)
            {
                for(int i=0; i<FTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<FTestSuck.iShtCol; j++)
                    {
                        if(FTestSuck.Suck[i][j].GetStatus())
                        {
                            bHasDevice=true;
                            ShowMyMessage("Please Remove The IC From the Index1 First", "");
                        }
                    }
                }

                for(int i=0; i<BTestSuck.iShtRow; i++)
                {
                    for(int j=0; j<BTestSuck.iShtCol; j++)
                    {
                        if(BTestSuck.Suck[i][j].GetStatus())
                        {
                            bHasDevice=true;
                            ShowMyMessage("Please Remove The IC From the Index2 First", "");
                        }
                    }
                }
            }

            if(bHasDevice==true)
            {
            }
            else
            {
                if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                                                              //Steven 20190910 : Qualcomm功能
                {
                    if(LastSet.iLanguageCountry==1)
                        Memo1->Lines->Add("執行DEVICE_MAP_CHECK");
                    else
                        Memo1->Lines->Add("Run DEVICE MAP CHECK");
                    Task=50;
                }
                else if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum &&                                                   //Steven 20170405 (wei) : No RTC no do fullview check
                        CosFunction.bFullTestBeforeContactHeight)
                {
                    Task=70;
                }
                else
                {
                    if(iContactMode==STEP_CONTACT_TEST)                                                                 //Steven 20150811 : Step by Step Contact Test
                    {
                        if(bRunDailyCorrelation)                                                                        //KaiChen 20200525 ：Daily Correlation Function
                        {
                            Task=77;
                        }
                        else
                        {
                            Task=80;
                        }
                    }
                    else
                    {
                        Task=90;

                        if(iContactMode==CONTACT_AUTO_GET_HEIGHT)
                        {
                            RecordProcess("Start Auto Contact Height Calibration.");
                        }
                        else if(iContactMode==CONTACT_TEST)
                        {
                            RecordProcess("Start Contact Test.");
                        }
                    }
                }
                rgHandlerMode->Enabled=false;                                                                           //Steven 20220616 : Add protection.
            }
            break;
        case 50:                                                                //Steven 20190910 : Qualcomm功能
            E042Leaf("DoDeviceMapCheck") /*GATE(W906-E042-LEAF) DoDeviceMapCheck(true) -- not translated*/;
            Task=55;
            break;
        case 55:
            if(E042Leaf("DoDeviceMapCheck") /*GATE(W906-E042-LEAF) DoDeviceMapCheck() -- not translated*/)
            {
                Task=1800;
            }
            break;
        case 70:
            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("進行Full view檢查");
            }
            else
            {
                Memo1->Lines->Add("Full view check.");
            }
            InitDoFullViewCheck();
            Task=75;
        case 75:
            if(E042Leaf("DoFullViewCheck") /*GATE(W906-E042-LEAF) DoFullViewCheck() -- not translated*/==true)
            {
                if(iContactMode==STEP_CONTACT_TEST)                             //Steven 20150811 : Step by Step Contact Test
                {
                    if(bRunDailyCorrelation)                                    //KaiChen 20200525 ：Daily Correlation Function
                    {
                        Task=77;
                    }
                    else
                    {
                        Task=80;
                    }
                }
                else
                {
                    Task=90;
                }
            }
            break;
        case 77:                                                                //KaiChen 20200525 ：Daily Correlation Function
            E042Leaf("DoStepContactKitDevice") /*GATE(W906-E042-LEAF) DoStepContactKitDevice(true) -- not translated*/;
            Task=78;
            break;
        case 78:                                                                //KaiChen 20200525 ：Daily Correlation Function
            if(E042Leaf("DoStepContactKitDevice") /*GATE(W906-E042-LEAF) DoStepContactKitDevice() -- not translated*/)
            {
//                Task=1900; //測試用
//                break;

                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("Shuttle 至 Index 吸取位。");
                }
                else
                {
                    Memo1->Lines->Add("Shuttle Move TO Index Pick.");
                }
                Task=86;
            }
            break;
        case 80:
            E042Leaf("DoStepContactLoadDevice") /*GATE(W906-E042-LEAF) DoStepContactLoadDevice(true) -- not translated*/;
            Task=85;
            break;
        case 85:
            if(E042Leaf("DoStepContactLoadDevice") /*GATE(W906-E042-LEAF) DoStepContactLoadDevice() -- not translated*/)
            {
                if(USE_TRAY_MAPPING==etmInstall &&
                   TestIF_File.bEnableDeviceRemain)                             //wei 20171128
                {
                    Task=190;                                                   //Sam 20200313 : Add Tray Arm 位置檢查機制
                }
                else
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("使用 T.Start 按鈕，進行再一次的測試。");
                        Memo1->Lines->Add("使用 Step 按鈕，結束測試。");
                    }
                    else
                    {
                        Memo1->Lines->Add("Push T.Start for test again.");
                        Memo1->Lines->Add("Push Step for finish the test.");
                    }
                    Task=86;
                }
            }
            break;
        case 86:                                                                //Steven 20151215 : 當執行一次remote function結束後無法接續再次執行,必須完成所有動作並將loader tray取走後才可再次執行,"客戶要求當input arm將shuttle上的產品放回loader tray後可直接再次編輯並重新開始"
            if(bRunDailyCorrelation)
            {
                Task=90;
                break;
            }

            if(WaitManualStartKey())
            {
                labDelayStatus->Caption="Waiting Test Result";
                StepContactTestLoadTask=sctlEditTray;
                Task=85;
            }
            else if(WaitManualStepKey())
            {
                iTriggerBoostFunction=-1;
                iTriggerBoostFuncBack=-1;
                iBoostFuncStep=5;
                labDelayStatus->Caption="";
                Task=90;
            }
            break;
        case 90:
            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("EP充飽氣");
                Memo1->Lines->Add("Z軸回至安全位置");
            }
            else
            {
                Memo1->Lines->Add("EP Fully charged air");
                Memo1->Lines->Add("Z-axis back to a safe location");
            }
            Task=100;
            break;
        case 100:                                                               //Z1 Z2 回至安全位置 請操作人員放IC 至 Shuttle
            if(INDEX_PRESS_TYPE==e240KG ||                                      //Steven 20110503 : 240KG要加速
               INDEX_PRESS_TYPE==e400KG ||                                      //Steven 20131007 : Index 1.5KW, 400KG
               INDEX_PRESS_TYPE==e260KG ||
               INDEX_PRESS_TYPE==e360KG)
            {
                iSpeedZ=CheckRange(iSpeedZ, 150, 30);
            }
            else
            {
                iSpeedZ=CheckRange(iSpeedZ, 60, 10);
            }
            iSpeed=CheckRange(iSpeed, 60, 30);

            if(IniConfig.bC04EnableTestTempIC)
            {
                iSpeed=CheckRange(atoi(edSpeed->Text.c_str()), 90, 10);
                iSpeedZ=CheckRange(atoi(edSpeedZ->Text.c_str()), 150, 30);
            }

            if(CUSTOMER_CODE==CC_ASE_KaohSiung)
            {
                if(iContactMode==CONTACT_TEST)
                {
                    iSpeed =90;                                                 //kevin 20220816 add contact mode change speed
                    iSpeedZ=150;
                }
            }

            bf[0]=MoveInArmZToPlateSafe(4444);
            bf[1]=MoveOutArmToAutoSafe();
            bf[2]=MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), "DoTestContactFunction");

            if(bf[0] && bf[1] && bf[2])
            {
                ZeroMemory(bf, sizeof(bf));
                if(iContactMode==STEP_CONTACT_TEST)                             //Steven 20150811 : Step by Step Contact Test
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("入料至Shuttle上完成，請按Step");
                    }
                    else
                    {
                        Memo1->Lines->Add("Loading devices to shuttle finish, press Step");
                    }
                }
                else
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("請置放Device至Shuttle上，並按Step");
                    }
                    else
                    {
                        Memo1->Lines->Add("Device to be placed on Shuttle, then Press Step");
                    }
                }
                Task=190;                                                       //Sam 20200313 : Add Tray Arm 位置檢查機制
            }
            break;
        case 190:                                                               //Sam 20200313 : Add Tray Arm 位置檢查機制
            if(IsTrayArmAtEmptyOrColor()==false)
            {
                Task=1;
                fAllMotorHome=false;
                SystemStart=false;
                ShowMyMessage("Tray Arm is not safe position. Must home first", "Tray X 沒有在安全位置上 ,請先歸零");
                return;
            }
            else
            {
                Task=200;
            }
        case 200:                                                                                                       //In/Out Arm 回至安全位置 及 Shuttle 回至入料位置
            rgHandlerMode->Enabled=false;

            if(iContactMode==STEP_CONTACT_TEST &&                                                                       //wei 20161219 Tray Mapping
               USE_TRAY_MAPPING==etmInstall &&
               (TestIF_File.bEnableTrayMap==true ||
                TestIF_File.bEnableDeviceRemain==true) && bRunDailyCorrelation==false)                                  //wei 20171128
            {
                if(bf[0]==false)
                {
                    bf[0]=MoveInArm2XYToShuttle2Wait();
                    if(bf[0]==true)
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("In Arm XY 移至安全位置");
                        }
                        else
                        {
                            Memo1->Lines->Add("In Arm XY move to a safe position");
                        }
                    }
                }

                if(bf[1]==false)
                {
                    bf[1]=MoveOutArmXY_ToFix_Tray_Full();
                    if(bf[1]==true)
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("Out Arm XY 移至安全位置");
                        }
                        else
                        {
                            Memo1->Lines->Add("Out Arm XY move to a safe position");
                        }
                    }
                }
                bf[2]=true;
                bf[3]=true;
            }
            else
            {
                if(bf[0]==false)
                {
                    bf[0]=MOT[MInArmX].MotorMove(Prod.XInArm_Tray_Pick[0][2]);
                    if(bf[0]==true)
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("In Arm X 移至安全位置");
                        }
                        else
                        {
                            Memo1->Lines->Add("In Arm X move to a safe position");
                        }
                    }
                }

                if(bf[1]==false)
                {
                    bf[1]=MOT[MInArmY].MotorMove(Prod.YInArm_Tray_Pick[0][2]-10000);
                    if(bf[1]==true)
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("In Arm Y 移至安全位置");
                        }
                        else
                        {
                            Memo1->Lines->Add("In Arm Y move to a safe position");
                        }
                    }
                }
            }

            if(bf[2]==false)
            {
                bf[2]=MoveOutArmXY_ToFix_Tray_Full();
                if(bf[2]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Out Arm XY 移至安全位置");
                    }
                    else
                    {
                        Memo1->Lines->Add("Out Arm XY move to a safe position");
                    }
                }
            }
            bf[3]=true;

            if(chk_K_Temperature->Checked==true)                                                                        //Ztex 2024.03.26 Add Contact Mode K Temperature
            {
                bf[4]=true;
                bf[5]=true;
            }

            if(bf[4]==false)
            {
                bf[4]=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft);
                if(bf[4]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Shuttle 1 移至等待區");
                    }
                    else
                    {
                        Memo1->Lines->Add("Shuttle 1 move to a waiting position");
                    }
                }
            }

            if(bf[5]==false)
            {
                bf[5]=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft);
                if(bf[5]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Shuttle 2 移至等待區");
                    }
                    else
                    {
                        Memo1->Lines->Add("Shuttle 2 move to a waiting position");
                    }
                }
            }
//#ifdef Carry4
//            if(bf[6]==false)
//                bf[6]=MOT[MOutShuttle1].MotorMove(Prod.OutSHT[0].iLeft);
//            if(bf[7]==false)
//                bf[7]=MOT[MOutShuttle2].MotorMove(Prod.OutSHT[1].iLeft);
//#else
            bf[6]=true;
            bf[7]=true;
//#endif
            if(bf[0] && bf[1] && bf[2] && bf[3] && bf[4] && bf[5] && bf[6] && bf[7])
            {
                if(SHT_FLOATING_CHK==1 &&                                                                               //Steven 20160920 : IC置偏檢查
                   TestIF_File.bEnableShtFloatChk &&
                   cbSFCAutoTune->Checked)
                {
                    palSFCInformation->Caption="Please take out devices from shuttle!!";
                    {} /*GATE(W906-E042-UI) palSFCInformation->Font->Color=clRed; -- AI(W906-E042): text colour only; vclcompat TPanel has no Font (forms/fContact.h facade)*/
                    palSFCInformation->Visible=true;
                    E042Leaf("fBarCode->InitialSFCAutoTune1") /*GATE(W906-E042-LEAF) fBarCode->InitialSFCAutoTune1(false) -- not translated*/;
                    E042Leaf("fBarCode->InitialSFCAutoTune2") /*GATE(W906-E042-LEAF) fBarCode->InitialSFCAutoTune2(false) -- not translated*/;
                    Task=205;
                }
                else if(iContactMode==STEP_CONTACT_TEST &&
                        USE_TRAY_MAPPING==etmInstall &&
                        (TestIF_File.bEnableTrayMap==true ||                                                            //wei 20161219 Tray Mapping
                         TestIF_File.bEnableDeviceRemain==true) && bRunDailyCorrelation==false)                         //wei 20171128
                {
                    if(TestIF_File.bEnableDeviceRemain==true)
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("Tray Remain自動學習開始--沒有IC");
                        }
                        else
                        {
                            Memo1->Lines->Add("Tray Remain Auto Tune Start--No IC");
                        }
                        E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(2, true) -- not translated*/;                                                    //Sam 20200323 : Modify Tray Function
                    }
                    else if(TestIF_File.bEnableTrayMap==true)
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("Tray Map自動學習開始--沒有IC");
                        }
                        else
                        {
                            Memo1->Lines->Add("Tray Map Auto Tune Start--No IC");
                        }
                        E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(0, true) -- not translated*/;
                    }
                    Task=211;
                }
                else if(iContactMode==CONTACT_DEVICE_LOOP_TEST)                                                         //Ztex 2023.11.19 Add CONTACT_DEVICE_LOOP_TEST
                {
                    Task=2300;
                }
                else if(iContactMode==K_TEMP_INDEX_MOVE)                                                                //Ztex 2024.03.26 Add Contact Mode K Temperature
                {
                    Task=2400;
                }
                else
                {
                    Task=218;
                }
            }
            break;
        case 205:
            if(WaitManualStepKey())
            {
                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)            //Use Signal shuttle 2
                {
                    palSFCInformation->Caption="Tuning shuttle 2 without IC!!";
                    Task=207;
                }
                else
                {
                    palSFCInformation->Caption="Tuning shuttle 1 without IC!!";
                    Task=206;
                }
                {} /*GATE(W906-E042-UI) palSFCInformation->Font->Color=clBlue; -- AI(W906-E042): text colour only; vclcompat TPanel has no Font (forms/fContact.h facade)*/
            }
            break;
        case 206:
            if(E042Leaf("fBarCode->DoSFCAutoTune_1") /*GATE(W906-E042-LEAF) fBarCode->DoSFCAutoTune_1(false) -- not translated*/==true)
            {
                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)            //Use Signal shuttle
                {
                    Task=208;
                }
                else
                {
                    palSFCInformation->Caption="Tuning shuttle 2 without IC!!";
                    {} /*GATE(W906-E042-UI) palSFCInformation->Font->Color=clBlue; -- AI(W906-E042): text colour only; vclcompat TPanel has no Font (forms/fContact.h facade)*/
                    Task=207;
                }
            }
            break;
        case 207:
            if(E042Leaf("fBarCode->DoSFCAutoTune_2") /*GATE(W906-E042-LEAF) fBarCode->DoSFCAutoTune_2(false) -- not translated*/==true)
            {
                palSFCInformation->Caption="Please put devices to shuttle!!";
                {} /*GATE(W906-E042-UI) palSFCInformation->Font->Color=clRed; -- AI(W906-E042): text colour only; vclcompat TPanel has no Font (forms/fContact.h facade)*/
                E042Leaf("fBarCode->InitialSFCAutoTune1") /*GATE(W906-E042-LEAF) fBarCode->InitialSFCAutoTune1(true) -- not translated*/;
                E042Leaf("fBarCode->InitialSFCAutoTune2") /*GATE(W906-E042-LEAF) fBarCode->InitialSFCAutoTune2(true) -- not translated*/;
                Task=208;
            }
            break;
        case 208:
            if(WaitManualStepKey())
            {
                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)            //Use Signal shuttle
                {
                    palSFCInformation->Caption="Tuning shuttle 2 with IC!!";
                }
                else
                {
                    palSFCInformation->Caption="Tuning shuttle 1 with IC!!";
                }
                {} /*GATE(W906-E042-UI) palSFCInformation->Font->Color=clBlue; -- AI(W906-E042): text colour only; vclcompat TPanel has no Font (forms/fContact.h facade)*/
                Task=209;
            }
            break;
        case 209:
            if(E042Leaf("fBarCode->DoSFCAutoTune_1") /*GATE(W906-E042-LEAF) fBarCode->DoSFCAutoTune_1(true) -- not translated*/==true)
            {
                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)            //Use Signal shuttle
                {
                    palSFCInformation->Visible=false;
                    Task=218;
                }
                else
                {
                    palSFCInformation->Caption="Tuning shuttle 2 with IC!!";
                    {} /*GATE(W906-E042-UI) palSFCInformation->Font->Color=clBlue; -- AI(W906-E042): text colour only; vclcompat TPanel has no Font (forms/fContact.h facade)*/
                    Task=210;
                }
            }
            break;
        case 210:
            if(E042Leaf("fBarCode->DoSFCAutoTune_2") /*GATE(W906-E042-LEAF) fBarCode->DoSFCAutoTune_2(true) -- not translated*/==true)
            {
                palSFCInformation->Visible=false;
                Task=218;
            }
            break;
        case 211:
            if(TestIF_File.bEnableDeviceRemain==true)
            {
                if(E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(2) -- not translated*/)                       //Sam 20200323 : Modify Tray Function
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Tray Remain自動學習結束--沒有IC，並按Step或Retry");
                    }
                    else
                    {
                        Memo1->Lines->Add("Tray Remain Auto Tune End--No IC ,then Press Step or Press Retry");
                    }
                    Task=212;
                }
            }
            else if(TestIF_File.bEnableTrayMap==true)
            {
                if(E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(0) -- not translated*/)                       //wei 20161219 Tray Mapping
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Tray Map自動學習結束--沒有IC，並按Step或Retry");
                    }
                    else
                    {
                        Memo1->Lines->Add("Tray Map Auto Tune End--No IC ,then Press Step or Press Retry");
                    }
                    Task=212;
                }
            }
            break;
        case 212:
            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    Task=218;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    Task=218;
                }
            }
            break;
        case 218:
            if(IniConfig.bG06HomeinitialCheckZ1)                                //kevin 20190227 add check socket ic
            {
                bContractModeCheckPushZ1=true;                                  //kevin 20190227 add 前檢查是否有CI放在COCKET 造成機構損壞 按z1 確認
                Task=2180;
            }
            else
            {
                E042Leaf("DoIndecxCHECkFunction") /*GATE(W906-E042-LEAF) DoIndecxCHECkFunction(true) -- not translated*/;                                    //kevin 20220426  add index check initial
                Task=2190;
            }
            break;
        case 2180:
            if(bContractModeCheckPushZ1 ==false)
            {
                E042Leaf("DoIndecxCHECkFunction") /*GATE(W906-E042-LEAF) DoIndecxCHECkFunction(true) -- not translated*/;                                    //kevin 20220426  add index check initial
                Task=2190;
            }
            break;
        case 2190:
            if(iContactMode==STEP_CONTACT_TEST && bRunDailyCorrelation)         //KaiHuang 20200606
            {
//                Button12Click(this);
            }

            if(iContactMode==STEP_CONTACT_TEST)
            {
                if(E042Leaf("DoIndecxCHECkFunction") /*GATE(W906-E042-LEAF) DoIndecxCHECkFunction(false) -- not translated*/)                                //kevin 20220426   Contact mode 需強制INDEX CHECK
                    Task=2195;
            }
            else
            {
                Task=2195;
            }
            break;
        case 2195:
            if(iContactMode==STEP_CONTACT_TEST &&
               (TestIF_File.bEnableDeviceRemain==true ||
                TestIF_File.bEnableTrayMap==true) &&
               WaitManualRetryKey())
            {
                if(LastSet.iLanguageCountry==1)
                {
                    if(TestIF_File.bEnableDeviceRemain==true)
                        Memo1->Lines->Add("Tray Remain自動學習開始--沒有IC");
                    else if(TestIF_File.bEnableTrayMap==true)
                        Memo1->Lines->Add("Tray Mapping 自動取像開始--");
                }
                else
                {
                    if(TestIF_File.bEnableDeviceRemain==true)
                        Memo1->Lines->Add("Tray Remain Auto Tune Start--No IC");
                    else if(TestIF_File.bEnableTrayMap==true)
                        Memo1->Lines->Add("Tray Map Auto Grab Start--");
                }
                E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(2, true) -- not translated*/;                                                            //Sam 20200323 : Modify Tray Function
                E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(0, true) -- not translated*/;                                                            //Sam 20200323 : Modify Tray Function
                Task=211;
                fMain->Pause("DoTestContactFunction 2195");
            }
            else if(WaitManualStepKey() ||
                  (IniConfig.bSPILFunction && cbTestContactMode->Checked==true))                                        //KevinCheng 20260115 : NV Contact Test
            {
                if(iContactMode==STEP_CONTACT_TEST &&
                   (TestIF_File.bEnableDeviceRemain==true ||
                    TestIF_File.bEnableTrayMap==true))                                                                  //wei 20171128
                {
                    Task=2150;
                }
                else if(iContactMode==CONTACT_LoadCell_AUTO_GET_HEIGHT)                                                 //kevin 20190909 add Load cell AutoHigh
                {
                    Inital_ZTask();
                    Task=2160;
                }
                else
                {
                    Inital_ZTask();
                    if(CosFunction.bContactShowOffset &&
                       iContactMode==CONTACT_AUTO_GET_HEIGHT)                                                           //2014-03-14    Dell
                    {
                        edContactOffsetArm1->Text=0;
                        edContactOffsetArm2->Text=0;
                    }

                    if(LastSet.iRealDummy==REALLY &&
                       cbTeachInSHSen->Checked &&
                       In_Shuttle_Auto_Latch==eInSHAutoLtc &&
                       iContactMode==CONTACT_AUTO_GET_HEIGHT)                   //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
                    {
                        iAutoTeachInSHLtcStatus=eTeachSHLtcNomal;
                        Memo1->Lines->Add("Auto teach In Shuttle 1 & 2 Latch sensor");
                        Task=2196;
                    }
                    else
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("偵測 Shuttle 1 & 2 Device是否置偏");
                        }
                        else
                        {
                            Memo1->Lines->Add("Shuttle 1 & 2 detect Device is tilted?");
                        }
                        Task=219;
                    }
                }
            }
            break;
        case 2196:                                                              //判斷做的Shuttle   //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
            if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==1)
            {
                E042Leaf("fShuttleMove->iShuttleMoveTask (In-shuttle latch teach)"); /*GATE(W906-E042-LEAF) fShuttleMove->iShuttleMoveTask=1; -- forms/fShuttleMove.h has no iShuttleMoveTask; the latch teach (golden :2196-2198) is a leaf (S-45), refused on HT9050 by P8*/
                Task=2198;
                break;
            }
            else
            {
                E042Leaf("fShuttleMove->iShuttleMoveTask (In-shuttle latch teach)"); /*GATE(W906-E042-LEAF) fShuttleMove->iShuttleMoveTask=1; -- forms/fShuttleMove.h has no iShuttleMoveTask; the latch teach (golden :2196-2198) is a leaf (S-45), refused on HT9050 by P8*/
                Task=2197;
            }
            break;
        case 2197:                                                              //Auto teach Inshuttle1 sensor  //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
            if(iAutoTeachInSHLtcStatus==eTeachSHLtcNomal)
                E042Leaf("fShuttleMove->DoInShuttleChkStackAction") /*GATE(W906-E042-LEAF) fShuttleMove->DoInShuttleChkStackAction(0) -- not translated*/;

            if(iAutoTeachInSHLtcStatus==eTeachSH1LtcFin)
            {
                iAutoTeachInSHLtcStatus=eTeachSHLtcNomal;
                if(TestIF.iShuttleMode==1 && TestIF.iShuttle_Sel==0)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("偵測 Shuttle 1 & 2 Device是否置偏");
                    }
                    else
                    {
                        Memo1->Lines->Add("Shuttle 1 & 2 detect Device is tilted?");
                    }
                    Task=219;
                    break;
                }
                else
                {
                    E042Leaf("fShuttleMove->iShuttleMoveTask (In-shuttle latch teach)"); /*GATE(W906-E042-LEAF) fShuttleMove->iShuttleMoveTask=1; -- forms/fShuttleMove.h has no iShuttleMoveTask; the latch teach (golden :2196-2198) is a leaf (S-45), refused on HT9050 by P8*/
                    Task=2198;
                }
            }
            else if(iAutoTeachInSHLtcStatus==eTeachSH1LtcAlarm)
            {
                E042Leaf("fShuttleMove->iShuttleMoveTask (In-shuttle latch teach)"); /*GATE(W906-E042-LEAF) fShuttleMove->iShuttleMoveTask=1; -- forms/fShuttleMove.h has no iShuttleMoveTask; the latch teach (golden :2196-2198) is a leaf (S-45), refused on HT9050 by P8*/
                iAutoTeachInSHLtcStatus=eTeachSHLtcNomal;
            }
            break;
        case 2198:                                                              //Auto teach Inshuttle2 sensor  //KenHsieh 20250909 : AutoHigh 使用Auto teach In SH latch sensor
            if(iAutoTeachInSHLtcStatus==eTeachSHLtcNomal)
                E042Leaf("fShuttleMove->DoInShuttleChkStackAction") /*GATE(W906-E042-LEAF) fShuttleMove->DoInShuttleChkStackAction(1) -- not translated*/;

            if(iAutoTeachInSHLtcStatus==eTeachSH2LtcFin)
            {
                iAutoTeachInSHLtcStatus=eTeachSHLtcNomal;
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("偵測 Shuttle 1 & 2 Device是否置偏");
                }
                else
                {
                    Memo1->Lines->Add("Shuttle 1 & 2 detect Device is tilted?");
                }
                Task=219;
            }
            else if(iAutoTeachInSHLtcStatus==eTeachSH2LtcAlarm)
            {
                E042Leaf("fShuttleMove->iShuttleMoveTask (In-shuttle latch teach)"); /*GATE(W906-E042-LEAF) fShuttleMove->iShuttleMoveTask=1; -- forms/fShuttleMove.h has no iShuttleMoveTask; the latch teach (golden :2196-2198) is a leaf (S-45), refused on HT9050 by P8*/
                iAutoTeachInSHLtcStatus=eTeachSHLtcNomal;
            }
            break;
        case 219:                                                               //確認InShuttle Sensor 是否正常 device 是否擺好
            if(LastSet.iRealDummy==REALLY &&
               In_Shuttle_Auto_Latch==eInSHAutoLtc)                             //KenHsieh 20250811 : InSht sensor latch for contact
            {
                InitChkInSHLatchTask();                                         //KenHsieh 20250811 : InSht sensor latch for contact
                Task=230;
            }
            else if(CheckShuttleSensor_9045(0) /*AI(W906-E042): golden default argument 0 (ainarm9045.h:100) spelled out -- acarry_shims.h:213 redeclares it without the default*/)                                  //Frank 20160305 SoakTime prompt
            {
                if(CosFunction.bContactTestWaitSoakTime &&
                   LastSet.iTemperature==Tempture_Hot)                          //wei 20160329 Contact Test Wait SoakTime
                {
                    iOldSec=0;
                    iCont=0;
                    palSoakTimeWating->Top=60;
                    palSoakTimeWating->Left=28;
                    palSoakTimeWating->Visible=true;
                    Task=220;
                }
                else
                {
                    Task=290;
                }
            }
            break;
        case 220:                                                               //Frank 20160305 SoakTime prompt
            if(WaitManualStepKey()==false)
            {
                iHeaterWaitingTime=atoi(fMain->edSoakTime->Text.c_str());
                tSoakTime=Now();
                DecodeTime(tSoakTime, SystemHour, SystemMin, SystemSec, SystemMSec);

                iNowSec=SystemSec;
                if(iNowSec==iOldSec)
                    return;

                if((iNowSec!=iOldSec && iCont<iHeaterWaitingTime))
                {
                    iOldSec=iNowSec;
                    iCont++;
                    Str=AnsiString(iHeaterWaitingTime-iCont);
                    palSoakTime->Caption=Str;
                }
                else
                {
                    Task=290;
                }
            }
            else
            {
                Task=290;
            }
            break;
        case 230:                                                               //KenHsieh 20250811 : InSht sensor latch for contact
            if(E042Leaf("CheckInShuttleSensor_Latch_Contact") /*GATE(W906-E042-LEAF) CheckInShuttleSensor_Latch_Contact() -- not translated*/)
            {
                if(CosFunction.bContactTestWaitSoakTime && LastSet.iTemperature==Tempture_Hot)                        //wei 20160329 Contact Test Wait SoakTime
                {
                    iOldSec=0;
                    iCont=0;
                    palSoakTimeWating->Top=60;
                    palSoakTimeWating->Left=28;
                    palSoakTimeWating->Visible=true;
                    Task=220;
                }
                else
                {
                    Task=290;
                }
            }
            break;
        case 290:
            palSoakTimeWating->Visible=false;                                                                           //Frank 20160305 SoakTime prompt
            iRetry=0;
            if(TestIF.iShuttleMode==1 || (bRunDailyCorrelation && bUseTwoArm && iRunWhichArm==1))                       //Use Signal shuttle //jou 2010-03-09 start : 只使用單一Arm時
            {
                if(TestIF.iShuttle_Sel==1 || (bRunDailyCorrelation && iRunWhichArm==1))                                 //shuttle 2
                {
                    EPSwitchOnOff(eEPSwArm2);
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("取得Shuttle 2 高度，或吸取Shuttle 2 Device");
                    }
                    else
                    {
                        Memo1->Lines->Add("get Shuttle 2 height position or pick up Shuttle 2 Device");
                    }
                    Task=330;
                    break;
                }
            }

            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("取得Shuttle 1 高度，或吸取Shuttle 1 Device");
            }
            else
            {
                Memo1->Lines->Add("get Shuttle 1 height position or pick up Shuttle 1 Device");
            }

            if((CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_CL) &&                                         //JerryYang 20250120 : add
               iContactMode==CONTACT_AUTO_GET_HEIGHT && chkTeachInOutArmZ->Checked)                                     //kevin 20171205 add auto teach in out arm Z
                Task=295;
            else
                Task=300;
            break;
        case 295:                                                               //kevin 20171103 (wei) Inarm Auto High //kevin 20170929 test4 不讓z 軸往下
            if(AutoTeachLoadTrayZ(false, InArm, iInArmZTeachTask))
            {
                Task=296;
            }
            break;
        case 296:                                                               //kevin 20171103 (wei) Outarm Z Auto High
            if(AutoTeachLoadTrayZ(false,OutArm,iOutArmZTeachTask))
            {
                MoveOutArmXY_ToFix_Tray_Full();
                Task=297;
            }
            break;
        case 297:                                                               //kevin 20171103 (wei) Outarm Z Auto High
            if(MoveOutArmXY_ToFix_Tray_Full())
            {
                bOutarmAutoHigh=false;
                bLoadInarmAutoHigh=false;
                Task=300;
            }
            break;
        case 300:                                                               //Index1 吸取 Shuttle1 device
            if(DoZ1PickFromShuttle())
            {
                Inital_ZTask();
                Task=320;
            }
            break;
        case 320:                                                                                                       //取得目前位置並重新寫入軸卡
            if((CUSTOMER_CODE==CC_GIGAS ||
                CUSTOMER_CODE==CC_JCET) &&                                                                              //RogerYang 20260130 : add JCET do check socket sensor when autoheight
                E042Leaf("DoSocketSensorCheckRemainIC") /*GATE(W906-E042-LEAF) DoSocketSensorCheckRemainIC() -- not translated*/==true)                                                                    //Isaac 20220120 : 全智要求在contact test和autoheight前，用socket sensor檢查socket是否有殘料，預防IC壓傷
                break;

            iRetry=0;
            EPSwitchOnOff(eEPSwBoth);
            if(TestIF.iShuttleMode==1|| (bRunDailyCorrelation && bUseTwoArm && iRunWhichArm==0))                        //Use Signal shuttle //jou 2010-03-09 start : 只使用單一Arm時
            {
                if(TestIF.iShuttle_Sel==0 || (bUseTwoArm && iRunWhichArm==0))                                           //shuttle 1
                {
                    Task=340;
                    break;
                }
            }

            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("取得Shuttle 2 高度，或吸取Shuttle 2 Device");
            }
            else
            {
                Memo1->Lines->Add("get Shuttle 2 height position or pick up Shuttle 2 Device");
            }
            Task=330;
            break;
        case 330:                                                               //Index2 吸取 Shuttle2 device
            if(DoZ2PickFromShuttle())
            {
                Inital_ZTask();
                Task=335;
            }
            break;
        case 335:                                                                                                       //取得目前位置並重新寫入軸卡
            if((CUSTOMER_CODE==CC_GIGAS ||
                CUSTOMER_CODE==CC_JCET) &&                                                                              //RogerYang 20260130 : add JCET do check socket sensor when autoheight
                E042Leaf("DoSocketSensorCheckRemainIC") /*GATE(W906-E042-LEAF) DoSocketSensorCheckRemainIC() -- not translated*/==true)                                                                    //Isaac 20220120 : 全智要求在contact test和autoheight前，用socket sensor檢查socket是否有殘料，預防IC壓傷
                break;

                iRetry=0;

            if(TestIF.iShuttleMode==1 || (bRunDailyCorrelation && bUseTwoArm && iRunWhichArm==1))                       //Use Signal shuttle //jou 2010-03-09 start : 只使用單一Arm時
            {
                if(TestIF.iShuttle_Sel==1 || (bRunDailyCorrelation && iRunWhichArm && iRunWhichArm==1))                 //shuttle 2
                {
                    for(int i=0; i<FTestSuck.iMaxRow; i++)
                    {
                        for(int j=0; j<FTestSuck.iMaxCol; j++)
                        {
                            if(i>=FTestSuck.iShtRow ||                                                                  //沒用到的不要開
                               j>=FTestSuck.iShtCol)
                            {
                                FTestSuck.Suck[i][j].Normal();
                                BTestSuck.Suck[i][j].Normal();
                            }
                        }
                    }
                    Inital_ZTask();
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                    }
                    else
                    {
                        Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                    }
                    Task=799;                                                   //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
                    break;
                }
            }
            Task=340;
            break;
        case 340:                                                               //確認Index Arm 是否有吸取到Device 並依照選擇模式進行動作
            for(int i=0; i<FTestSuck.iMaxRow; i++)                              //沒用到的不要開
            {
                for(int j=0; j<FTestSuck.iMaxCol; j++)
                {
                    if(i>=FTestSuck.iShtRow ||
                       j>=FTestSuck.iShtCol)
                    {
                        FTestSuck.Suck[i][j].Normal();
                        BTestSuck.Suck[i][j].Normal();
                    }
                }
            }

            Inital_ZTask();
            if(IniConfig.bD11NoIcSkipAutoHeight)                                //Steven 20110726 : Shuttle沒IC時,該Arm不要Auto Height
            {
                if(FrontTestHeadHasIC()==false)                                 //如果Arm1沒有吸到IC
                {
                    if(TestIF.iShuttleMode==1)                                  //Use Signal shuttle
                    {
                        if(TestIF.iShuttle_Sel==0)                              //只開Shuttle 1且沒有IC
                        {
                            if(LastSet.iLanguageCountry==1)
                            {
                                Memo1->Lines->Add("Index Arm 1 沒有Device, 略過Auto Height流程");
                            }
                            else
                            {
                                Memo1->Lines->Add("Index Arm 1 has no device, skip auto height.");
                            }
                            Task=805;
                        }
                        else                                                    //只開Shuttle 2且沒有IC
                        {
                            if(RearTestHeadHasIC()==false)                      //如果Arm2沒有吸到IC
                            {
                                if(LastSet.iLanguageCountry==1)
                                {
                                    Memo1->Lines->Add("Index Arm 2 沒有Device, 略過Auto Height流程");
                                }
                                else
                                {
                                    Memo1->Lines->Add("Index Arm 2 has no device, skip auto height.");
                                }
                                Task=805;
                            }
                            else
                            {
                                if(LastSet.iLanguageCountry==1)
                                {
                                    Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                                    Memo1->Lines->Add("按one cylce(亮燈後) -> Step，可上下arm測試");
                                }
                                else
                                {
                                    Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                                    Memo1->Lines->Add("press one cylce(After light) -> Step can up-down arm test");
                                }
                            }
                            Task=799;                                           //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
                        }
                    }
                    else
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("Index Arm 1 沒有Device, 略過Auto Height流程");
                        }
                        else
                        {
                            Memo1->Lines->Add("Index Arm 1 has no device, skip auto height.");
                        }

                        if(RearTestHeadHasIC()==false)                          //如果Arm2沒有吸到IC
                        {
                            if(LastSet.iLanguageCountry==1)
                            {
                                Memo1->Lines->Add("Index Arm 2 沒有Device, 略過Auto Height流程");
                            }
                            else
                            {
                                Memo1->Lines->Add("Index Arm 2 has no device, skip auto height.");
                            }
                            Task=805;
                        }
                        else
                        {
                            if(LastSet.iLanguageCountry==1)
                            {
                                Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                                Memo1->Lines->Add("按one cylce(亮燈後) -> Step，可上下arm測試");
                            }
                            else
                            {
                                Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                                Memo1->Lines->Add("press one cylce(After light) -> Step can up-down arm test");
                            }
                            Task=799;                                           //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
                        }
                    }
                    break;
                }
            }

            if(iContactMode==AUTO_CONTACT_TEST)                                 //Steven 20150224 : Auto Contact Test
            {
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("開始執行自動測試驗證流程");
                }
                else
                {
                    Memo1->Lines->Add("Start Auto Contact Test Procedure.");
                }
                E042Leaf("Do_AutoContactTest") /*GATE(W906-E042-LEAF) Do_AutoContactTest(true) -- not translated*/;
                Task=1500;
            }
            else if(iContactMode==CONTACT_TEST &&
                    bUseTwoArm32Site==true)
            {
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("測試全部的Index Devices");
                }
                else
                {
                    Memo1->Lines->Add("Test all devices on index Arm.");
                }

                Task=500;
            }
            else
            {
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("取得index Arm 1 contect高度，或測試index Arm 1 device");
                }
                else
                {
                    Memo1->Lines->Add("get index Arm 1 contect height position or test index Arm 1 device");
                }

                Task=400;
            }
            break;
        case 400:                                                                                                       //取得Index1 Socket 高度
            SW[SwTesterAirCooling].On();                                                                                //Steven 20160714

            if(bUniversalkitflag[0]==true)                                                                              //Frank 20171030 (Steven) add Floating Shuttle Read Torque
            {
                Inital_ZTask();

                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("Index Arm 1 Device 吸取異常, 略過Auto Height流程");
                }
                else
                {
                    Memo1->Lines->Add("Index Arm 1 has no device, skip auto height.");
                }

                if(IniConfig.bIndexArm2SupplyLight==true ||                                                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   TestIF_File.bForEgisTecTest==true     ||                                                             //Steven 20140922 : Arm2當作指紋測試
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                     //kevin 20150127 Arm1 下壓 arm2 測試
                    TestIF_File.bArm1PickPlaceArm2Test==true))                                                          //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    if(iContactMode==CONTACT_TEST)
                    {
                        Task=805;
                    }
                    else
                    {
                        ADAM_WriteVoltage(60);                                                                          //Lee 2008_01_09
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                            Memo1->Lines->Add("按one cylce(亮燈後) -> Step，可上下arm測試");
                        }
                        else
                        {
                            Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                            Memo1->Lines->Add("press one cylce(After light) -> Step can up-down arm test");
                        }
                        Task=799;                                               //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
                    }
                    break;
                }

                if(IniConfig.bD11NoIcSkipAutoHeight)                                                                    //Steven 20110726 : Shuttle沒IC時,該Arm不要Auto Height
                {                                                                                                       //Steven 20240807 : 往下移動
                    if(RearTestHeadHasIC()==false)                                                                      //如果Arm2沒有吸到IC
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("Index Arm 2 沒有Device, 略過Auto Height流程");
                        }
                        else
                        {
                            Memo1->Lines->Add("Index Arm 2 has no device, skip auto height.");
                        }
                        Task=805;
                        break;
                    }
                }

                if(TestIF.iShuttleMode==1)                                                                              //Use Signal shuttle    //jou 2010-03-09 start : 只使用單一Arm時
                {
                    if(TestIF.iShuttle_Sel==0)                                                                          //shuttle 1
                    {
                        Task=805;
                        break;
                    }
                }

                ADAM_WriteVoltage(60);                                                                                  //Lee 2008_01_09
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                    Memo1->Lines->Add("按one cylce(亮燈後) -> Step，可上下arm測試");
                }
                else
                {
                    Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                    Memo1->Lines->Add("press one cylce(After light) -> Step can up-down arm test");
                }
                Task=799;                                                       //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                                                                    //Frank 20221122 : 2DID sorting for ATK
            {
                Inital_ZTask();
                Task=805;
                break;
            }

            if(Do_Z1_AutoGetHeight() && bUniversalkitflag[0]==false)
            {
                if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                   IniConfig.bD01EnableReadTorque)                                                                      //Frank 20170626 add Xilinx 浮動Shuttle Kit 強制開啟[D01]
                {
                    ret=ShowMyMessageBox_YES_NO("Sure To Save Z1 Torue Value?", "確定要紀錄Z1 Torue值嗎？");
                    if(ret!=2)
                    {
                        palTorqueArm1->Caption=PnlTorue0->Caption;
                    }
                }

                Inital_ZTask();

                if(IniConfig.bIndexArm2SupplyLight==true ||                                                             //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   TestIF_File.bForEgisTecTest==true     ||                                                             //Steven 20140922 : Arm2當作指紋測試
                   (IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                                                     //kevin 20150127 Arm1 下壓 arm2 測試
                    TestIF_File.bArm1PickPlaceArm2Test==true))                                                          //Ifor 20200811 Fix: Arm1 Pick Place Arm2Test 需卡兩個條件
                {
                    if(iContactMode==CONTACT_TEST)
                    {
                        Task=805;
                    }
                    else
                    {
                        ADAM_WriteVoltage(60);                                                                          //Lee 2008_01_09
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                            Memo1->Lines->Add("按one cylce(亮燈後) -> Step，可上下arm測試");
                        }
                        else
                        {
                            Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                            Memo1->Lines->Add("press one cylce(After light) -> Step can up-down arm test");
                        }
                        Task=799;                                               //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
                    }
                    break;
                }

                if(IniConfig.bD11NoIcSkipAutoHeight)                                                                    //Steven 20110726 : Shuttle沒IC時,該Arm不要Auto Height
                {                                                                                                       //Steven 20240807 : 往下移動
                    if(RearTestHeadHasIC()==false)                                                                      //如果Arm2沒有吸到IC
                    {
                        if(LastSet.iLanguageCountry==1)
                        {
                            Memo1->Lines->Add("Index Arm 2 沒有Device, 略過Auto Height流程");
                        }
                        else
                        {
                            Memo1->Lines->Add("Index Arm 2 has no device, skip auto height.");
                        }
                        Task=805;
                        break;
                    }
                }

                if(TestIF.iShuttleMode==1 || (bUseTwoArm && iRunWhichArm==0))                                           //Use Signal shuttle  //jou 2010-03-09 start : 只使用單一Arm時
                {
                    if(TestIF.iShuttle_Sel==0 || (bUseTwoArm && iRunWhichArm==0))                                       //shuttle 1
                    {
                        Task=805;
                        break;
                    }
                }

                ADAM_WriteVoltage(60);                                                                                  //Lee 2008_01_09
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("取得index Arm 2 contect高度，或測試index Arm 2 device");
                    Memo1->Lines->Add("按one cylce(亮燈後) -> Step，可上下arm測試");
                }
                else
                {
                    Memo1->Lines->Add("get index Arm 2 contect height position or test index Arm 2 device");
                    Memo1->Lines->Add("press one cylce(After light) -> Step can up-down arm test");
                }
                Task=799;                                                       //AI(general) 20260328 (Rogeryang) : route through case 799 for socket sensor check before Z2 auto height
            }
            break;
        case 500:                                                               //Steven 20140512: For HT-9047
            SW[SwTesterAirCooling].On();                                        //Steven 20160714
            if(E042Leaf("Do_ContactTest_32Site") /*GATE(W906-E042-LEAF) Do_ContactTest_32Site() -- not translated*/)
            {
                Inital_ZTask();
                Task=805;
            }
            break;
        case 799:                                                               //AI(general) 20260328 (Rogeryang) : one-time socket sensor check gate before Z2 auto height, avoid false WAR0322 during descent
            if(CUSTOMER_CODE==CC_JCET &&
                E042Leaf("DoSocketSensorCheckRemainIC") /*GATE(W906-E042-LEAF) DoSocketSensorCheckRemainIC() -- not translated*/==true)
            break;
            Task=800;
            break;
        case 800:
            SW[SwTesterAirCooling].On();                                        //Steven 20160714

            if(bUniversalkitflag[1]==true)
            {
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("Index Arm 2 Device 吸取異常, 略過Auto Height流程");
                }
                else
                {
                    Memo1->Lines->Add("Index Arm 2 has no device, skip auto height.");
                }

                Inital_ZTask();
                Task=805;
            }

            if(CosFunction.bSortingBy2DList==true &&
               LastSet.iTester==_2D_SORT &&
               TestIF_File.bSortingBy2DIDList==true)                            //Frank 20221122 : 2DID sorting for ATK
            {
                Inital_ZTask();
                Task=805;
                break;
            }

            if(Do_Z2_AutoGetHeight() && bUniversalkitflag[1]==false)
            {
                if(CUSTOMER_CODE==CC_KYEC_XILINX &&
                   IniConfig.bD01EnableReadTorque)                              //Frank 20170626 (Steven) add Xilinx 浮動Shuttle Kit 強制開啟[D01]
                {
                    ret=ShowMyMessageBox_YES_NO("Sure To Save Z2 Torue Value?", "確定要紀錄Z2 Torue值嗎？");
                    if(ret!=2)
                    {
                        palTorqueArm2->Caption=PnlTorue1->Caption;
                    }
                }

                if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&              //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
                   TestIF_File.bArm1PickPlaceArm2Test==true &&
                   FTestSuck.HasRealIC()==true)
                {
                    Inital_Z1PickFromSocketTask();
                    Task=802;
                    break;
                }

                Inital_ZTask();
                Task=805;
            }
            break;
        case 802:
            if(E042Leaf("DoZ1PickFromSocket") /*GATE(W906-E042-LEAF) DoZ1PickFromSocket() -- not translated*/)                                            //JerryYang 20250515 : 一丟一測Auto height把IC丟到SOCKET
            {
                Inital_ZTask();
                Task=805;
            }
            break;
        case 805:
            SW[SwTesterAirCooling].On();                                        //Steven 20160714
            if(iContactMode==CONTACT_AUTO_GET_HEIGHT)
            {
                bAutoHighFinish=true;

                if(TestIF.iShuttleMode==1 &&                                    //Use Signal shuttle //jou 2010-03-09 start : 只使用單一Arm時
                   IniConfig.bIndexArm2SupplyLight==false &&                    //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
                   TestIF_File.bForEgisTecTest==false     &&                    //Steven 20140922 : Arm2當作指紋測試
                   TestIF_File.bArm1PickPlaceArm2Test==false)                   //kevin 20150127 Arm1 下壓 arm2 測試
                {
                    if(TestIF.iShuttle_Sel==0)                                  //shuttle 1
                    {
                        edReleaseHeight2->Text=edReleaseHeight1->Text;
                        edPickUp2->Text=edPickUp1->Text;

                        edContactHeight2->Text="-50.0";                         //Steven 20110506 : 強制設成-50mm
                    }
                    else                                                        //shuttle 2
                    {
                        edReleaseHeight1->Text=edReleaseHeight2->Text;
                        edPickUp1->Text=edPickUp2->Text;
                        edContactHeight1->Text="-50.0";                         //Steven 20110506 : 強制設成-50mm
                    }
                }

                if(CosFunction.bSortingBy2DList==true &&
                   LastSet.iTester==_2D_SORT &&
                   TestIF_File.bSortingBy2DIDList==true)                        //Frank 20221122 : 2DID sorting for ATK
                {
                    edContactHeight2->Text="-50.0";                             //Steven 20110506 : 強制設成-50mm
                    edContactHeight1->Text="-50.0";                             //Steven 20110506 : 強制設成-50mm
                }
            }
            Task=1100;
            break;
        case 810:                                                               //Eliot 2007_12_03 Start
            WaitTime.SetMSAndOn(1000);
            Task=1120;
            break;
        case 820:
            if(WaitTime.Off())
            {
                Task=800;
            }
            break;                                                              //Eliot 2007_12_03 End
        case 1100:
            SW[SwTesterAirCooling].On();                                                                                //Steven 20160714
            if(bContinueContact &&
               (iContactMode==CONTACT_TEST ||
                iContactMode==CONTACT_AUTO_GET_HEIGHT))
            {
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("取得index Arm 1 contect高度，或測試index Arm 1 device");
                }
                else
                {
                    Memo1->Lines->Add("get index Arm 1 contect height position or test index Arm 1 device");
                }
                Task=400;
            }
            else
            {
                if(REAL_TIME_CCD && COM2->bCCDDummyRum==false && cbRTCModel->Checked==true)                             //Steven 20110827 : Real Time CCD - ROI Learning
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("進行ROI Learning動作");
                    }
                    else
                    {
                        Memo1->Lines->Add("Doing real time model learning...");
                    }

                    if(CosFunction.bRTCAutoTuning && cbRTCAutoTuning->Checked)                                          //Sam 20230419 : 新增 RTC Auto Tuning 功能
                    {
                        if(LastSet.iLanguageCountry==1)
                            Memo1->Lines->Add("執行 RTC Auto Tuning.");
                        else
                            Memo1->Lines->Add("Run RTC Auto Tuning.");
                        InitRTCAutoTuning();
                        Task=950;
                    }
                    else
                    {
                        InitROILearningTask();
                        Task=1000;
                    }
                    bDoRTCLearning=true;                                                                                //Ifor 20260226 add: Contact Mode 執行RTC Learn 不開啟Hot Air
                }
                else
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("index arm 放置device回shuttle上");
                    }
                    else
                    {
                        Memo1->Lines->Add("index arm place device to shuttle");
                    }
                    Task=900;
                }
            }
            Inital_ZTask();
            break;
        case 900:
            bDoRTCLearning=false;                                               //Ifor 20260226 add: Contact Mode 執行RTC Learn 不開啟Hot Air
            { const int pb=W906_IndexZPlaceBackPrep("DoTestContactFunction 900"); if(pb<0) { fAllMotorHome=false; CarlibrationTask=1; return; } if(pb==0) break; }   if(DoZPlaceToShuttle())   //AI(W906-E042) 20261005 (B4) NOT GOLDEN, HT9050 only: Steven 1005 09:2x Q101: HT9050 has no Index Y; the In shuttle returns to iRight before place-back (preconditions Z1 at its safe height + Out shuttle X at OutSHT[0].iRight, else alarm + ST + exit, nothing moves); golden case 1700 returns it to iLeft
            {
                Inital_ZTask();
                if(iContactMode==CONTACT_DEVICE_MAP_CHECK)                      //Steven 20210810 : Qualcomm功能
                    SetMotorSpeed();                                            //Steven 20210813 : CONTACT_DEVICE_MAP_CHECK速度
                else
                    SetAllMotorSpeed(10);
                for(int i=3; i<8; i++)
                    bf[i]=false;

                Task=1700;
            }
            break;
        case 950:                                                               //Sam 20230419 : 新增 RTC Auto Tuning 功能
            if(E042Leaf("DoRTCAutoTuning") /*GATE(W906-E042-LEAF) DoRTCAutoTuning() -- not translated*/)
            {
                InitROILearningTask();
                Task=1000;
            }
            break;
        case 1000:                                                                                                      //Steven 20110827 : Real Time CCD - ROI Learning
            if(E042Leaf("Do_ROILearning") /*GATE(W906-E042-LEAF) Do_ROILearning() -- not translated*/)
            {
                if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum && LastSet.iRealDummy==REALLY &&                          //RTC auto verify
                   CosFunction.bRTCAutoModelVerify==true && IniConfig.bD36EnableRTCAutoModelVerify==true &&
                   cbContactMode->ItemIndex!=DropPlaceShiftContact &&
                   CosFunction.bRTCHalfViewAutoVerify)
                {
                    if(TestIF.iShuttleMode==1)                                                                          //Use Signal shuttle
                    {
                        if(TestIF.iShuttle_Sel==1 &&
                           TestIF.bArm1PickPlaceArm2Test==false)                                                        //shuttle 2
                        {
                            if(BTestSuck.UseSiteHasIC() &&
                               BTestSuck.AlreadyTest()==false)
                            {
                                if(SendSiteMapToRTC(false, 2)==BTestSuck.CountRealIC())
                                {
                                    bNeedWaitRTCAutoVerify=true;
                                    bNeedWaitContactTestAutoVerify=true;
                                    bRTCAutoModelVerifyFirstTime=false;
                                    RecordProcess("Start RTC Auto Verification");
                                    Memo1->Lines->Add("Start RTC Auto Verification");
                                    DoBRTCAutoModelVerify(true);
                                    Task=1300;
                                    break;
                                }
                            }
                        }
                        else if(TestIF.iShuttle_Sel==0)
                        {
                            if(FTestSuck.UseSiteHasIC() &&
                               FTestSuck.AlreadyTest()==false)                                                          //shuttle 1
                            {
                                if(SendSiteMapToRTC(false, 1)==FTestSuck.CountRealIC())
                                {
                                    bNeedWaitRTCAutoVerify=true;
                                    bNeedWaitContactTestAutoVerify=true;
                                    bRTCAutoModelVerifyFirstTime=false;
                                    RecordProcess("Start RTC Auto Verification");
                                    Memo1->Lines->Add("Start RTC Auto Verification");
                                    DoFRTCAutoModelVerify(true);
                                    Task=1200;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        if(BTestSuck.UseSiteHasIC() &&
                           BTestSuck.AlreadyTest()==false)                                                              //shuttle 2
                        {
                            if(SendSiteMapToRTC(false, 2)==BTestSuck.CountRealIC())
                            {
                                bNeedWaitRTCAutoVerify=true;
                                bNeedWaitContactTestAutoVerify=true;
                                bRTCAutoModelVerifyFirstTime=false;
                                RecordProcess("Start RTC Auto Verification");
                                Memo1->Lines->Add("Start RTC Auto Verification");
                                DoBRTCAutoModelVerify(true);
                                Task=1300;
                                break;
                            }
                        }
                    }
                    ShowMyMessage("There is no enough device, SKIP RTC Auto Verification");
                    bNeedWaitContactTestAutoVerify=true;                                                                //JerryYang 20241220 : No enough device alarm的時候要重新做
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("index arm 放置device回shuttle上");
                    }
                    else
                    {
                        Memo1->Lines->Add("index arm place device to shuttle");
                    }
                    Task=900;
                }
                else
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("RealTime CCD 自動建立Real Time Modal完成!!!");
                        Memo1->Lines->Add("index arm 放置device回shuttle上");
                    }
                    else
                    {
                        Memo1->Lines->Add("RealTime CCD Auto Builder Real Time Modal Finish!!!");
                        Memo1->Lines->Add("index arm place device to shuttle");
                    }
                    Task=900;
                }
            }
            break;
        case 1200:
            if(DoFRTCAutoModelVerify(false))                                    //Arm1 RTC Auto verify
            {
                bNeedWaitRTCAutoVerify=false;
                if(bPickUpErrReAutoVerify)
                {
                    bNeedWaitContactTestAutoVerify=true;
                }
                else
                {
                    bNeedWaitContactTestAutoVerify=false;
                }

                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("RTC Auto Verification 完成!!!");
                    Memo1->Lines->Add("index arm 放置device回shuttle上");
                }
                else
                {
                    Memo1->Lines->Add("RTC Auto Verification finish!!!");
                    Memo1->Lines->Add("index arm place device to shuttle");
                }
                Task=900;
            }
            break;
        case 1300:
            if(DoBRTCAutoModelVerify(false))                                                                            //Arm2 RTC Auto verify
            {
                bNeedWaitRTCAutoVerify=false;
                if(bPickUpErrReAutoVerify)
                {
                    bNeedWaitContactTestAutoVerify=true;
                }
                else
                {
                    bNeedWaitContactTestAutoVerify=false;
                }

                if(REAL_TIME_CCD==true && !COM2->bCCDDummyRum && LastSet.iRealDummy==REALLY &&                          //RTC auto verify
                   CosFunction.bRTCAutoModelVerify==true && IniConfig.bD36EnableRTCAutoModelVerify==true &&
                   DeviceForm.ContactMode!=DropPlaceShiftContact &&
                   CosFunction.bRTCHalfViewAutoVerify)
                {
                    if(FTestSuck.HasIC() && FTestSuck.AlreadyTest()==false)                                             //shuttle 1
                    {
                        if(SendSiteMapToRTC(false,1)==FTestSuck.CountRealIC())
                        {
                            bNeedWaitRTCAutoVerify=true;
                            bNeedWaitContactTestAutoVerify=true;
                            bRTCAutoModelVerifyFirstTime=false;
                            RecordProcess("Start RTC Auto Verification");
                            Memo1->Lines->Add("Start RTC Auto Verification");
                            DoFRTCAutoModelVerify(true);
                            Task=1200;
                            break;
                        }
                    }
                }

                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("RTC Auto Verification 完成!!!");
                    Memo1->Lines->Add("index arm 放置device回shuttle上");
                }
                else
                {
                    Memo1->Lines->Add("RTC Auto Verification finish!!!");
                    Memo1->Lines->Add("index arm place device to shuttle");
                }
                Task=900;
            }
            break;
        case 1500:                                                              //Steven 20150224 : Auto Contact Test
            if(E042Leaf("Do_AutoContactTest") /*GATE(W906-E042-LEAF) Do_AutoContactTest() -- not translated*/)
            {
                Task=900;
            }
            break;
        case 1700:
            if(bf[3]==false)
            {
                bf[3]=MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front_EndWaitPos, Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), "DoTestContactFunction 1700");   //981118 jou Y1 +2000 easy change kit      //JimmyChiu 20211028 : All speed can set by speed setting.
                if(bf[3]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("index arm 回安全位置");
                    }
                    else
                    {
                        Memo1->Lines->Add("index arm move to safe position");
                    }
                }
            }

            if(bf[4]==false)
            {
                bf[4]=MOT[MInShuttle1].MotorMove(Prod.InSHT[0].iLeft);
                if(bf[4]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("shuttle 1 退回放置位置");
                    }
                    else
                    {
                        Memo1->Lines->Add("shuttle 1 Back placement");
                    }
                }
            }

            if(bf[5]==false)
            {
                bf[5]=MOT[MInShuttle2].MotorMove(Prod.InSHT[1].iLeft);
                if(bf[5]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("shuttle 2 退回放置位置");
                    }
                    else
                    {
                        Memo1->Lines->Add("shuttle 2 Back placement");
                    }
                }
            }
//#ifdef Carry4
//            bf[6]=MOT[MOutShuttle1].MotorMove(Prod.OutSHT[0].iLeft);
//            bf[7]=MOT[MOutShuttle2].MotorMove(Prod.OutSHT[1].iLeft);
//#else
            bf[6]=true;
            bf[7]=true;
//#endif
            if(bf[3] && bf[4] && bf[5] && bf[6] && bf[7])
            {
                if(iContactMode==STEP_CONTACT_TEST)                                                                                                             //Steven 20150811 : Step by Step Contact Test
                {
                    if(bRunDailyCorrelation)                                                                                                                    //KaiChen 20200525 ：Daily Correlation Function
                    {
                        Task=1900;
                    }
                    else
                    {
                        Task=2000;
                    }
                }
                else
                {
                    Task=1800;
                }
            }
            break;
        case 1800:

            if(iContactMode==STEP_CONTACT_TEST && bRunDailyCorrelation)         //KaiHuang  20200606 : For ASE-CL DC
            {
                if(bUseTwoArm)
                {
                    if(iRunWhichArm==0)
                    {
                        iRunWhichArm=1;
                        Task=86;
//                        Task=1900; //測試用
                        break;
                    }
                    else
                    {
                        bRunDailyCorrelation=false;
                        chkDailyCorrelation->Enabled=true;
                        chkDailyCorrelation->Checked=false;
                    }
                }
                else
                {
                    bRunDailyCorrelation=false;
                    chkDailyCorrelation->Enabled=true;
                    chkDailyCorrelation->Checked=false;
                }
                MOT[MMDailyCorrelationKit].ClearTray(__FUNC__);
                FLCarryKit.SetHasNullIcToNullIc();
                BLCarryKit.SetHasNullIcToNullIc();
            }
            SW[SwTesterAirCooling].Off();                                       //Steven 20160714
            Memo1->Lines->Add("Finish!!");
            iWhichArmDown=0;

            if(IniConfig.bSPILFunction &&                                       //KevinCheng 20260115 : NV Contact Test
                cbTestContactMode->Checked==true)
            {
                iTestContactCount++;
                lblNowCount->Caption=iTestContactCount;
                if(iTestContactCount<atoi(edTestContactCount->Text.c_str()))
                {
                    Task=190;
                    break;
                }
                else
                {
                    iTestContactCount=0;
                }
            }

            if(IniConfig.bIndexArm2SupplyLight==false ||                        //jou 2012-10-19 Index Arm 2
               TestIF_File.bArm1PickPlaceArm2Test==false)                       //kevin 20150127 Arm1 下壓 arm2 測試
            {
                cbContactMode->Enabled=true;                                    //Steven 20100406 : 避免做到一半被更換
                cbVacuumMode->Enabled=true;
            }

            fMain->Pause("DoTestContactFunction 1800");
            rgHandlerMode->Enabled=true;
            InitialTestHeadMotorTask();                                         //jou 2011-11-29 做完重新initial index task

            if(iContactMode!=CONTACT_TEST)                                      //ChungHung 20130715 add ATK 顯示上次AutoHeight的值
            {
                edContactBackUp1->Text=edContactHeight1->Text;
                edContactBackUp2->Text=edContactHeight2->Text;

                edOrgPick1->Text=edPickUp1->Text;                               //ChungHung 20140516 add Show 上次Shuttle Auto Height 的值
                edOrgPick2->Text=edPickUp2->Text;
            }

            if(CUSTOMER_CODE==CC_KYEC_XILINX &&                                 //jou 20170407 (wei) : Xilinx Universal Change Kit function Auto Height finish offset 清除為0
               iContactMode==CONTACT_AUTO_GET_HEIGHT &&
               IniConfig.bChangeKitNoHardStop==true)
            {
                edShtPickOffset1->Text=0;
                edShtPickOffset2->Text=0;
                edContactOffsetArm1->Text=0;
                edContactOffsetArm2->Text=0;
            }

            if(IniConfig.bD63CheckIndexZHomeToZPhaseDistanceRange==true)        //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
            {
                if(bZ1ModifyDistanceRef==true)
                {
                    WriteIniData(asGeneralPath,"IndexDriver", "Index_Z1_Home_Position", iZ1ModifyDistanceRef);

                    Str.sprintf("%s Auto height save Index_Z1_Home_Position as %d",sTime,iZ1ModifyDistanceRef);
                    SaveFile(asIndexZphasePath, Str);

                    bZ1ModifyDistanceRef=false;
                }

                if(bZ2ModifyDistanceRef==true)
                {
                    WriteIniData(asGeneralPath,"IndexDriver", "Index_Z2_Home_Position", iZ2ModifyDistanceRef);

                    Str.sprintf("%s Auto height save Index_Z2_Home_Position as %d",sTime,iZ2ModifyDistanceRef);
                    SaveFile(asIndexZphasePath, Str);

                    bZ2ModifyDistanceRef=false;
                }
            }

            if(chk_K_Temperature->Checked==true)                                //Ztex 2024.03.26 Add Contact Mode K Temperature
            {
                rbKTempIndexMove->Visible=true;
                chk_K_Temperature->Checked=false;
            }
            Task=1;
            break;
        case 1900:
            E042Leaf("DoStepContactUnKitDevice") /*GATE(W906-E042-LEAF) DoStepContactUnKitDevice(true) -- not translated*/;                                     //Steven 20150811 : Step by Step Contact Test
            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("IC放回Kit");
            }
            else
            {
                Memo1->Lines->Add("Put devices back to kit");
            }
            Task=1910;
            break;
        case 1910:
            if(E042Leaf("DoStepContactUnKitDevice") /*GATE(W906-E042-LEAF) DoStepContactUnKitDevice() -- not translated*/)
            {
                {
                    Task=2150;
                }
            }
            break;
        case 2000:
            E042Leaf("DoStepContactUnloadDevice") /*GATE(W906-E042-LEAF) DoStepContactUnloadDevice(true) -- not translated*/;                                    //Steven 20150811 : Step by Step Contact Test
            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("IC放回Loader");
            }
            else
            {
                Memo1->Lines->Add("Put devices back to loader");
            }
            Task=2100;
            break;
        case 2100:
            if(E042Leaf("DoStepContactUnloadDevice") /*GATE(W906-E042-LEAF) DoStepContactUnloadDevice() -- not translated*/)
            {
                if(USE_TRAY_MAPPING==etmInstall &&
                   TestIF_File.bEnableTrayMap==true)                            //wei 20161219 Tray Mapping
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Tray Map自動學習開始--有IC");
                    }
                    else
                    {
                        Memo1->Lines->Add("Tray Map Auto Tune Start--Has IC");
                    }
                    bf[0]=false;
                    bf[1]=false;
                    Task=2010;
                }
                else
                {
                    Task=2150;
                }
            }
            break;
        case 2010:
            if(bf[0]==false)
            {
                bf[0]=MoveInArm2XYToWait();
                if(bf[0]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("In Arm XY 移至安全位置");
                    }
                    else
                    {
                        Memo1->Lines->Add("In Arm XY move to a safe position");
                    }
                }
            }

            if(bf[1]==false)
            {
                bf[1]=MoveOutArmXY_ToFix_Tray_Full();
                if(bf[1]==true)
                {
                    if(LastSet.iLanguageCountry==1)
                    {
                        Memo1->Lines->Add("Out Arm XY 移至安全位置");
                    }
                    else
                    {
                        Memo1->Lines->Add("Out Arm XY move to a safe position");
                    }
                }
            }

            if(bf[0] && bf[1])
            {
                E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(1, true) -- not translated*/;
                Task=2050;
            }
            break;
        case 2050:
            if(E042Leaf("fTrayMapping->DoTrayMapAutoTuneCCD") /*GATE(W906-E042-LEAF) fTrayMapping->DoTrayMapAutoTuneCCD(1) -- not translated*/)                           //wei 20161219 Tray Mapping
            {
                if(LastSet.iLanguageCountry==1)
                {
                    Memo1->Lines->Add("Tray Map自動學習結束--有IC");
                }
                else
                {
                    Memo1->Lines->Add("Tray Map Auto Tune End--Has IC");
                }
                Task=2060;
            }
            break;
        case 2060:
            if(IniConfig.bP56TrayArmWaitAtColorTrack ||                         //Steven 20240516 : Tray Arm等待位置改到Color
               (INSTALL_OCR!=eocrUninstal && CosFunction.bTrayOCR))             //wei 20150925 待機位置改道 Color
            {
                if(TrayArmMotorMove(Prod.iXTrayColor))
                {
                    Task=2150;
                }
            }
            else
            {
                if(TrayArmMotorMove(Prod.iXTrayEmpty))
                {
                    Task=2150;
                }
            }
            break;
        case 2150:                                                              //Sam 20200317 : Tray Map teach 增加 Loader 退 Tray 機制
            if(USE_AUTO_RETEST==eartInstall)
            {
                iReceiveLoaderTray=2;
                Task=2151;
            }
            else
            {
                Task=2155;
            }
            break;
        case 2151:
            if(iReceiveLoaderTray==0)
            {
                Task=1800;
            }
            else
            {
                DoAutoLoaderReceive();
            }
            break;                                                              //Sam 20200317 : Tray Map teach 增加 Loader 退 Tray 機制
        case 2155:
            if(iContactMode==STEP_CONTACT_TEST && bRunDailyCorrelation)
            {
                Task=1800;
                break;
            }

            if(LastSet.iLanguageCountry==1)
            {
                Memo1->Lines->Add("請取走Loader上的Tray盤, 然後按下START");
            }
            else
            {
                Memo1->Lines->Add("Please remove the tray on loader, manually. Then push Start");
            }

            Task=2200;
            if(TRAY_ARM_MODE==eUnderCoveyor)
            {
                Cylinder[C_LoaderPushBack_Back].Off();
                Cylinder[C_LoaderPushBack_Push].On();
            }
            else
            {
                Cylinder[C_TrayY_Fixer].Off();                                  //Open Fix Supply Try Fix Cylinder
            }
            Cylinder[C_LoaderEdgePush].Off();
            Cylinder[C_LoaderUpPress].Off();                                    //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
            MOT[MMTrayY].ClearTray(__FUNC__);
            break;
        case 2160:
            if(Do_LoadCellAutoHigh(0))                                          //kevin 20190909 add Load cell AutoHigh
            {
                Inital_ZTask();
                Task=2170;
            }
            break;
        case 2170:
            if(Do_LoadCellAutoHigh(1))                                          //kevin 20190909 add Load cell AutoHigh
            {
                Inital_ZTask();
                Task=2200;
            }
            break;
        case 2200:
            if(Sen[SnLoaderSureTray].IsOn() ||
               Sen[SnLoaderPreDete].IsOn())
            {
                ShowMyMessage("Please remove the tray on loader, manually. Then push Start", "請取走Loader上的Tray盤, 然後按下START");
            }
            else
            {
                Task=1800;
            }
            break;
            //Ztex 2023.11.19 Add CONTACT_DEVICE_LOOP_TEST ==>
        case 2300:
            if(WaitManualStepKey())
            {
                E042Leaf("DoContactDeviceLoopTest") /*GATE(W906-E042-LEAF) DoContactDeviceLoopTest(true) -- not translated*/;
                Task=2310;
            }
            break;
        case 2310:
            if(E042Leaf("DoContactDeviceLoopTest") /*GATE(W906-E042-LEAF) DoContactDeviceLoopTest(false) -- not translated*/)
            {
                Task=1;
                ShowMyMessage("Contact Device Loop Test Finish", "Contact Device Loop Test 完成");
            }
            break;
            //Ztex 2023.11.19 Add CONTACT_DEVICE_LOOP_TEST <==

            //Ztex 2024.03.26 Add Contact Mode K Temperature ==>
        case 2400:
            if(WaitManualStepKey())
            {
                E042Leaf("DoContactKTemperatureTest") /*GATE(W906-E042-LEAF) DoContactKTemperatureTest(true) -- not translated*/;
                Task=2410;
            }
            break;
        case 2410:
            if(E042Leaf("DoContactKTemperatureTest") /*GATE(W906-E042-LEAF) DoContactKTemperatureTest(false) -- not translated*/)
            {
                Task=1;
                ShowMyMessage("Contact K Temperature Finish", "Contact K Temperature 完成");
            }
            break;
            //Ztex 2024.03.26 Add Contact Mode K Temperature <==
    }
}

// ===== AI(W906-E042) 20261005 (B4): golden 0618 cContact.cpp:17197-17997 (Do_LoadCellAutoHigh), verbatim: LoadCell span: port line = golden line + (-14777) =====
bool TfContact::Do_LoadCellAutoHigh(int iIndex)
{
    static int kg, Counter, Pos, PosY1, iStepSpeed, iManualSpeed=100;
    static int vkg;                                                             //Steven 20100604 : For Mitsubishi
    static bool bZ1Z2Press, bEPLeakage=false;
    static int iRecordZ1Pos=0,iRecordZ1Pos1=0,iRecordZ1Pos2=0;
    static bool bRecordZ1Pos=false;

    int &Task=Z_Height_Task, ret;
    double fIndexDownPos_LoadCell=-100;
    TEdit *tempEdit[]={edPinCount, edForcePerPinN, edForcePerPinG};
    ShowMainScreenPresure(0);                                                   //kevin 20130605 read Torque send gpib use

    if(W906_IndexZDriveFaultStop("Do_LoadCellAutoHigh")) { fAllMotorHome=false; CarlibrationTask=1; return false; }   switch(Task)   //AI(W906-E042) 20261005 (B4) P7 NOT GOLDEN (PCI1203 Z1 SHIP only)
    {
        case 1: if(W906_IndexZRunRefused(iContactMode, iIndex, "Do_LoadCellAutoHigh 1")) { SystemStart=false; CarlibrationTask=1; return false; }   /*AI(W906-E042) 20261005 (B4) P4+P8 NOT GOLDEN: Load Cell (mode 8) is refused on a PCI-1203 Index Z (Steven 1004 08:4x; it always runs Z2 after Z1)*/                                                                                                          //Index1 確認Index Device 是否掉料 Z1回安全位置
            if(MOT[MTestZ1].Gali_Two_ZAxis_Move(Prod.TestZ1_Safe, GotIndexZSpeed(iSpeedZ*1000), __FUNC__))              //JimmyChiu 20211028 : All speed can set by speed setting.
            {
                if(iIndex==0)
                    palArm1Height->Caption=0;
                else
                    palArm2Height->Caption=0;
                bRecordZ1Pos=false;
                Task=100;
            }
            break;
        case 100:                                                                                                                                               // IndexY1 移至Socket IndexY2 移至Shuttle2上方
            if(iIndex==0)
            {
                if(MOT[MTestY1].GalilTwoY_Move(Teach.iLoadCellY1 , Prod.TestY2_Rear, GotIndexYSpeed(iSpeed*3000), __FUNC__))                                    //Arm 1 Load Cell Y Pos       //JimmyChiu 20211028 : All speed can set by speed setting.
                    Task=110;
            }
            else
            {
                if(MOT[MTestY1].GalilTwoY_Move(Prod.TestY1_Front, Teach.iLoadCellY2, GotIndexYSpeed(iSpeed*3000), __FUNC__))                                    //Arm 2 Load Cell Y Pos        //JimmyChiu 20211028 : All speed can set by speed setting.
                    Task=110;
            }
            break;
        case 110:                                                               //依照個模式執行動作 CONTACT_TEST--->2900   CONTACT_AUTO_GET_HEIGHT--->530
            if(IniConfig.iD17_UseHardwareHeightToContact==2)                    //Steven 20231025 : 不充氣K高
            {
                ADAM_WriteMaxData(false);                                       //Steven 20241014 : 整合auto height輸出壓力
            }
            else
            {
                ADAM_WriteMaxData(true);
            }

            for(int i=0; i<3; i++)                                              //Steven 20100624 : K高過程不可以改變
               tempEdit[i]->Enabled=false;

            Task=120;
            break;
        case 120:                                                                                                       //IndexZ1移至StandBy位置後設定扭力值
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_MotMove(Teach.iLoadCellZ1Down -100, GotIndexZSpeed(iSpeedZ*1000)))                 //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    InitWriteAndCheckMotorTorqueTask();
                    kg=GetAutoHeightMaxKGTorque();                                                                      //Steven 20170720 (wei) : for low contact force
                    Task=121;
                    iManualSpeed=100;                                                                                   //Steven 20100208 :10->100 加快速度
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(Teach.iLoadCellZ2Down -100, GotIndexZSpeed(iSpeedZ*1000)))                 //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    InitWriteAndCheckMotorTorqueTask();
                    kg=GetAutoHeightMaxKGTorque();                                                                      //Steven 20170720 (wei) : for low contact force
                    Task=121;
                    iManualSpeed=100;                                                                                   //Steven 20100208 :10->100 加快速度
                }
            }
            break;
        case 121:
            bZ1Z2Press=false;
            Task=122;
            break;
        case 122:                                                               //確認扭力值設定完成
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                             //Panasonic
            {
                ret=COM2->iWriteAndCheckMotorTorque(0, kg);
            }
            else
            {
                vkg=kgTranToMitsubishikg(kg);
                ret=COM2->iWriteAndCheckMotorTorque(0, vkg);
            }

            if(ret==1)
            {
                Task=130;
                Counter=0;
                iStepSpeed=10;
            }
            else if(ret==2)
            {
                if(iIndex==0)
                    ShowMyMessage("Index Z1 Motor torque set error", "馬達扭力設定錯誤!", "Do_Z1_LoadCell 122");
                 else
                    ShowMyMessage("Index Z2 Motor torque set error", "馬達扭力設定錯誤!", "Do_Z2_LoadCell 122");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 130:                                                               //讀取扭力值並顯示
            COM2->InitReadTorueTask();
            if(iIndex==0)
            {
                fMain->chkReadTorque1->Checked=true;
                fMain->chkReadTorque2->Checked=false;
                fMain->edTorue0->Text="";
                bReadMCU1=true;                                                 //kevin 20220225 read MCU DATA
            }
            else
            {
                fMain->chkReadTorque1->Checked=false;
                fMain->chkReadTorque2->Checked=true;
                fMain->edTorue1->Text="";
                bReadMCU2=true;                                                 //kevin 20220225 read MCU DATA
            }
            Task=150;
        case 150:                                                                                                       //讀取扭力值與設定值比較後EP洩氣並取得IndexZ 高度
            #ifndef SOFT_SIMULTE
             if(iIndex==0)
             {
                if(fMain->edTorue0->Text=="")
                    break;
             }
             else
             {
                if(fMain->edTorue1->Text=="")
                    break;
             }
            #else
                fMain->edTorue0->Text=30;
            #endif
            if(iIndex==0)
            {
                Pos=MOT[MTestZ1].Gali_ReadPos();
                edLoadCellHeight1->Text=ConvertTouMType(MOT[MTestZ1].Gali_ReadEncoderPos());
                TorqueData=atoi(fMain->edTorue0->Text.c_str());
            }
            else
            {
                Pos=MOT[MTestZ2].Gali_ReadPos();
                edLoadCellHeight2->Text=ConvertTouMType(MOT[MTestZ2].Gali_ReadEncoderPos());
                TorqueData=atoi(fMain->edTorue1->Text.c_str());
            }

            if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
            {
                PnlTorue0->Caption="";
                if(TorqueData>=kg-6)                                                                                    // 2010.05.19 , Joye
                {
                    TorqueData=kg;
                }
            }

            if(INDEX_DRIVER_TYPE==Panasonic_DRIVER)                                                                     //Panasonic
                TorqueData=abs(TorqueData);

            if(TorqueData>=kg ||
               (iIndex==0 && atof(edLoadCellHeight1->Text.c_str())<=fIndexDownPos_LoadCell) ||
               (iIndex==1 && atof(edLoadCellHeight2->Text.c_str())<=fIndexDownPos_LoadCell))
            {
                if(IniConfig.bD10ManualHeightComptibleWithNS && iContactMode==CONTACT_MANUAL_GET_HEIGHT)
                {
                    if(iIndex==0)
                        MOT[MTestZ1].Gali_Command("ST", __FUNC__);
                    else
                        MOT[MTestZ2].Gali_Command("ST", __FUNC__);
                    Task=160;
                }
                else
                {
                    Counter++;
                    if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                        PnlTorue0->Caption=TorqueData;

                    //if(Counter>=10)                                           // avoid torque on only short time
                    //{
                        Counter=0;

                        if(iIndex==0)
                        {
                            Pos=MOT[MTestZ1].Gali_ReadPos();
                            MOT[MTestZ1].Gali_Command("ST", __FUNC__);

                            if(bRecordZ1Pos==false)                                                                     //取得充飽氣的高度
                            {
                                iRecordZ1Pos1=MOT[MTestZ1].Gali_ReadEncoderPos();
                                edLoadCellHeight1->Text=ConvertTouMType(iRecordZ1Pos1);

                                if(atof(edLoadCellHeight1->Text.c_str())<=fIndexDownPos_LoadCell)
                                {
                                    iRecordZ1Pos1=fIndexDownPos_LoadCell*100;
                                    edLoadCellHeight1->Text=fIndexDownPos_LoadCell;
                                    ShowMyMessage("Attention!! Over Z1 LoadCell contact high! Be sure!","注意!!超過Z1 Load Cell contact高度!需確認!");
                                }

                                if(EP_Install==3 || EP_Install==5)
                                {
                                    if(IniConfig.bD26EnableEPEncoderRange==true)                                        //ChungHung 20111217
                                    {
                                        ADAM_Rang(IniConfig.iD26EPEncoderRange);
                                        if(EP_Install==5)
                                            bEPLeakage=ADAM_Alarm(0);
                                        else
                                            bEPLeakage=ADAM_Alarm();
                                    }
                                    else
                                    {
                                        bEPLeakage=false;
                                    }
                                }
                                else
                                {
                                    if(Sen[SnEPAlarm].Enable==true)
                                    {
                                        bEPLeakage=Sen[SnEPAlarm].IsOff();                                              //Steven 20110622 : 檢查EP有沒有漏
                                    }
                                    else
                                    {
                                        bEPLeakage=false;
                                    }
                                }
                                ADAM_WriteVoltage(0);
                                MySleep(3000);
                                ADAM_WriteVoltage(0);
                                bRecordZ1Pos=true;
                                Task=200;
                            }
                            else                                                                                        //取得洩氣的高度
                            {
                                iRecordZ1Pos2=MOT[MTestZ1].Gali_ReadEncoderPos();
                                edLoadCellHeight1->Text=ConvertTouMType(iRecordZ1Pos2);
                                Task=200;
                            }
                        }
                        else
                        {
                            Pos=MOT[MTestZ2].Gali_ReadPos();
                            MOT[MTestZ2].Gali_Command("ST", __FUNC__);

                            if(bRecordZ1Pos==false)                                                                     //取得充飽氣的高度
                            {
                                iRecordZ1Pos1=MOT[MTestZ2].Gali_ReadEncoderPos();
                                edLoadCellHeight2->Text=ConvertTouMType(iRecordZ1Pos1);

                                if(atof(edLoadCellHeight2->Text.c_str())<=fIndexDownPos_LoadCell)
                                {
                                    iRecordZ1Pos1=fIndexDownPos_LoadCell*100;
                                    edLoadCellHeight2->Text=fIndexDownPos_LoadCell;
                                    ShowMyMessage("Attention!! Over Z2 LoadCell contact high! Be sure!", "注意!!超過Z2 Load Cell contact高度!需確認!");
                                }

                                if(EP_Install==3 || EP_Install==5)
                                {
                                    if(IniConfig.bD26EnableEPEncoderRange==true)                                        //ChungHung 20111217
                                    {
                                        ADAM_Rang(IniConfig.iD26EPEncoderRange);
                                        if(EP_Install==5)
                                            bEPLeakage=ADAM_Alarm(1);
                                        else
                                            bEPLeakage=ADAM_Alarm();
                                    }
                                    else
                                    {
                                        bEPLeakage=false;
                                    }
                                }
                                else
                                {
                                    if(Sen[SnEPAlarm].Enable==true)
                                    {
                                        bEPLeakage=Sen[SnEPAlarm].IsOff();                                              //Steven 20110622 : 檢查EP有沒有漏
                                    }
                                    else
                                    {
                                        bEPLeakage=false;
                                    }
                                }
                                ADAM_WriteVoltage(0);
                                MySleep(3000);
                                ADAM_WriteVoltage(0);
                                bRecordZ1Pos=true;
                                Task=200;
                            }
                            else                                                                                        //取得洩氣的高度
                            {
                                iRecordZ1Pos2=MOT[MTestZ2].Gali_ReadEncoderPos();
                                edLoadCellHeight2->Text=ConvertTouMType(iRecordZ1Pos2);
                                Task=200;
                            }
                        }
                   /* }
                    else
                    {
                        Task=130;
                    }*/
                }
            }
            else
            {
                Counter=0;
                Task=160;
            }
            break;
        case 160:
            if(iIndex==0)
            {
                PosY1=MOT[MTestY1].Gali_ReadPos();
                PosY1-=Teach.iLoadCellY1  ;
                if(abs(PosY1)>10)
                {
                    ShowMyMessage("Index Y1 Load Cell Motor Position error", "馬達Y1位置錯誤!", "Do_Z1_LoadCellHeight 160");
                    return false;
                }

                if(MOT[MTestZ1].Gali_ReadPos()>(Teach.iLoadCellZ1Down+750))                                             //kevin 20140612 add start
                {
                    Task=161;
                    return false;
                }
                else
                {
                    if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                    {
                        if(TorqueData>3)
                        {
                            if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos-iStepSpeed/10, GotIndexZSpeed(iSpeedZ*1000)))   //JimmyChiu 20211028 : All speed can set by speed setting.
                                Task=130;
                        }
                        else
                        {
                            if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos-iStepSpeed,GotIndexZSpeed(iSpeedZ*1000)))       //JimmyChiu 20211028 : All speed can set by speed setting.
                                Task=130;
                        }
                    }
                    else
                    {
                        if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos-iStepSpeed, GotIndexZSpeed(iSpeedZ*1000)))          //JimmyChiu 20211028 : All speed can set by speed setting.
                        {
                            MOT[MTestZ1].Gali_ScanMotStatus();
                            if(MOT[MTestZ1].Led[iInposLed]==false)
                            {
                                palArm1Height->Caption=Pos-iStepSpeed;
                                Task=130;
                            }
                        }
                    }
                }
            }
            else
            {
                PosY1=MOT[MTestY2].Gali_ReadPos();
                PosY1=PosY1-Teach.iLoadCellY2;
                if(abs(PosY1)>10)
                {
                    ShowMyMessage("Index Y2 Load Cell Motor Position error", "馬達Y1位置錯誤!", "Do_Z2_LoadCellHeight 160");
                    return false;
                }

                if(MOT[MTestZ2].Gali_ReadPos()>(Teach.iLoadCellZ2Down+750))                                             //kevin 20140612 add start
                {
                    Task=161;
                    return false;
                }
                else
                {
                    if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER)
                    {
                        if(TorqueData>3)
                        {
                            if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos-iStepSpeed/10, GotIndexZSpeed(iSpeedZ*1000)))   //JimmyChiu 20211028 : All speed can set by speed setting.
                                Task=130;
                        }
                        else
                        {
                            if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos-iStepSpeed,GotIndexZSpeed(iSpeedZ*1000)))       //JimmyChiu 20211028 : All speed can set by speed setting.
                                Task=130;
                        }
                    }
                    else
                    {
                        if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos-iStepSpeed, GotIndexZSpeed(iSpeedZ*1000)))          //JimmyChiu 20211028 : All speed can set by speed setting.
                        {
                            MOT[MTestZ2].Gali_ScanMotStatus();
                            if(MOT[MTestZ2].Led[iInposLed]==false)
                            {
                                palArm2Height->Caption=Pos-iStepSpeed;
                                Task=130;
                            }
                        }
                    }
                }
            }
            break;
        case 161:                                                                                                       //kevin 20140612  上升方便取料
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_MotMove(-200, GotIndexZSpeed(iSpeedZ*1000)))                                       //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    Task=160;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(-200, GotIndexZSpeed(iSpeedZ*1000)))                                       //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    Task=160;
                }
            }
            break;

        case 200:                                                               //取得IndexZ1目前位置往上加3000
            if(iIndex==0)
                Pos=MOT[MTestZ1].Gali_ReadEncoderPos()+3000;
            else
                Pos=MOT[MTestZ2].Gali_ReadEncoderPos()+3000;

            if(IniConfig.iD17_UseHardwareHeightToContact==2)                    //Steven 20231025 : 不充氣K高
            {
                iRecordZ1Pos=iRecordZ1Pos1+100;
            }
            else if(DeviceForm_File.dKitDiameter<=2.5 &&                        //kevin 20170804 (Steven) 20mm Auto Height
                    INDEX_PRESS_TYPE!=e85KG)                                    //Frank 20250214 add
            {
                iRecordZ1Pos=iRecordZ1Pos1+100;
            }
            else
            {
                if(IniConfig.iD17_UseHardwareHeightToContact==1)                //Steven 20170411 (wei) : SCK的SIP怕刮傷,所以Contact Height使用硬體高度
                {
                    iRecordZ1Pos=iRecordZ1Pos1;
                }
                else if(bEPLeakage==true)                                       //jou 2011-09-29 如果沒有設定alarm值,高度都不減會太高,所以改-20
                {
                    iRecordZ1Pos=iRecordZ1Pos1-20;
                }
                else
                {
                    iRecordZ1Pos=iRecordZ1Pos1-100;
                }
            }
            Task=210;
            break;
        case 210:                                                                                                       //IndexZ1往上移動至目標位置
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                             //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm1Height->Caption=IntToStr(Pos);
                    InitWriteAndCheckMotorTorqueTask();
                    edLoadCellHeight1->Text=ConvertTouMType(Pos);

                    ADAM_WriteVoltage(DeviceForm.dPress);
                    //Lee 2008_01_09 start
                    ADAM_WriteVoltage(0);
                    MySleepEx(1000, false);
                    ADAM_WriteVoltage(DeviceForm.dPress);
                    //Lee 2008_01_09 end
                    Task=300;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMoveSkipEncoder(Pos, GotIndexZSpeed(iSpeedZ*1000)))                             //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm2Height->Caption=IntToStr(Pos);
                    InitWriteAndCheckMotorTorqueTask();
                    edLoadCellHeight2->Text=ConvertTouMType(Pos);

                    ADAM_WriteVoltage(DeviceForm.dPress);
                    //Lee 2008_01_09 start
                    ADAM_WriteVoltage(0);
                    MySleepEx(1000, false);
                    ADAM_WriteVoltage(DeviceForm.dPress);
                    //Lee 2008_01_09 end
                    Task=300;
                }
            }
            break;
        case 300:                                                               //jou 2010-06-21 start : auto high加一段程式驗證是否有沒有掉O-Ring
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 120);                        //寫入為大容許扭力值 120%//jou 981128
            if(ret==1)
            {
                Task=310;
            }
            else if(ret==2)
            {
                if(iIndex==0)
                    ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_LoadCellHeight 300");
                else
                    ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_LoadCellHeight 300");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 310:                                                                                                       //IndexZ1 往下移至目標位置
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_MotMove(iRecordZ1Pos, GotIndexZSpeed(iSpeedZ*1000)))                               //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm1Height->Caption = IntToStr(iRecordZ1Pos);
                    edLoadCellHeight1->Text=ConvertTouMType(iRecordZ1Pos);

                    InitWriteAndCheckMotorTorqueTask();

                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    bReadMCU1=true;                                                                                     //kevin 20220225 read MCU DATA
                    Counter=0;
                    Task=320;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(iRecordZ1Pos, GotIndexZSpeed(iSpeedZ*1000)))                               //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm2Height->Caption =IntToStr(iRecordZ1Pos);
                    edLoadCellHeight2->Text=ConvertTouMType(iRecordZ1Pos);

                    InitWriteAndCheckMotorTorqueTask();

                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    bReadMCU2=true;                                                                                     //kevin 20220225 read MCU DATA
                    Counter=0;
                    Task=320;
                }
            }
            break;
        case 320:                                                               //確認扭力是否超過 125%
            #ifdef SOFT_SIMULTE
                if(iIndex==0)
                    fMain->edTorue0->Text=30;
                else
                    fMain->edTorue1->Text=30;
            #endif
            if(iIndex==0)
            {
                if(fMain->edTorue0->Text!="")
                {
                    MySleep(500);
                    if(abs(atoi(fMain->edTorue0->Text.c_str()))>=125)           //Frank 20150317 120->125
                    {
                        if(Counter>=5)
                        {
                            ShowMyMessage("Z1 Motor Auto LoadCell error, check EP Value!", "Z1馬達自動取得高度錯誤,請檢查EP是否漏氣!", "Do_Z2_LoadCell High 320");
                            InitWriteAndCheckMotorTorqueTask();
                            MOT[MTestZ1].iGali_SingalHomeTask=1;
                            Task=905;
                            break;
                        }
                        Counter++;
                        COM2->InitReadTorueTask();
                        fMain->chkReadTorque1->Checked=true;
                        fMain->chkReadTorque2->Checked=false;
                        fMain->edTorue0->Text="";
                        bReadMCU1=true;                                         //kevin 20220225 read MCU DATA
                    }
                    else
                    {
                        InitWriteAndCheckMotorTorqueTask();
                        Task=330;
                    }
                }
            }
            else
            {
                if(fMain->edTorue1->Text!="")
                {
                    MySleep(500);
                    if(abs(atoi(fMain->edTorue1->Text.c_str()))>=125)           //Frank 20150317 120->125
                    {
                        if(Counter>=5)
                        {
                            ShowMyMessage("Z2 Motor Auto LoadCell error, check EP Value!", "Z2馬達自動取得高度錯誤,請檢查EP是否漏氣!", "Do_Z2_LoadCell High 320");
                            InitWriteAndCheckMotorTorqueTask();
                            MOT[MTestZ2].iGali_SingalHomeTask=1;
                            Task=905;
                            break;
                        }
                        Counter++;
                        COM2->InitReadTorueTask();
                        fMain->chkReadTorque1->Checked=false;
                        fMain->chkReadTorque2->Checked=true;
                        fMain->edTorue0->Text="";
                    }
                    else
                    {
                        InitWriteAndCheckMotorTorqueTask();
                        Task=330;
                    }
                }
            }
            break;
        case 330:                                                               //Lee 2007_0208    //寫入扭力值300%
            if(USE_IO_CHANGE_TOQUE==true)                                       //jou 2012-06-21 Enable index I/O Change Toque
            {
                SW[SwIndexChangeToque1].Off();
                SW[SwIndexChangeToque2].Off();
            }

            ret=COM2->iWriteAndCheckMotorTorque(0, 300);                        //jou 981128
            if(ret==1)
            {
                Task=340;
            }
            else if(ret==2)
            {
                if(iIndex==0)
                    ShowMyMessage("Index Z1 Motor torque set error", "Z1馬達扭力設定錯誤!", "Do_Z1_Loadcell Height 330");
                else
                    ShowMyMessage("Index Z2 Motor torque set error", "Z2馬達扭力設定錯誤!", "Do_Z2_Loadcell Height 330");
                fAllMotorHome=false;
                CarlibrationTask=1;
                return false;
            }
            break;
        case 340:                                                                                                       //jou 981128  IndexZ1往下移至目標位置
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_MotMove(iRecordZ1Pos, GotIndexZSpeed(iSpeedZ*1000)))
                {
                    palArm1Height->Caption =IntToStr(iRecordZ1Pos);
                    InitWriteAndCheckMotorTorqueTask();
                    edLoadCellHeight1->Text=ConvertTouMType(iRecordZ1Pos);
                    fMain->edTorue0->Text="999";
                    Task=350;

                    for(int i=0; i<3; i++)                                                                              //Steven 20100624 : K高過程不可以改變
                        tempEdit[i]->Enabled=true;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(iRecordZ1Pos, GotIndexZSpeed(iSpeedZ*1000)))                               //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm2Height->Caption = IntToStr(iRecordZ1Pos);
                    InitWriteAndCheckMotorTorqueTask();
                    edLoadCellHeight2->Text=ConvertTouMType(iRecordZ1Pos);

                    fMain->edTorue1->Text="999";
                    Task=350;

                    for(int i=0; i<3; i++)                                                                              //Steven 20100624 : K高過程不可以改變
                        tempEdit[i]->Enabled=true;
                }
            }
            break;
        case 350:
            if(WaitManualStartKey())                                            //Steven 20100225
            {
                labDelayStatus->Caption="Waiting Test Result";
                fMain->SendMSG_CMD(MSG_CMD_ContactTestArm1);                    //Steven 20150304 : Add GPIB LOG
                {} /*GATE(W906-E042-UI) fTestCategory->BringToFront(); -- AI(W906-E042): window z-order only; TfTestCategory has no base class / BringToFront (forms/fTestCategory.h:257 D-2)*/                                  //Steven 20100823
                if(IniConfig.bC04EnableTestTempIC)
                    {} /*GATE(W906-E042-UI) fDynamicTemp->BringToFront(); -- AI(W906-E042): window z-order only; fDynamicTemp has no global (forms/fContact.h:458-461 X-13)*/                               //Steven 20120810
                iSetupTask=1;
                IsTest=true;                                                    //Steven 20110920
                Task=360;
                break;
            }

            if(WaitManualStepKey())
            {
                iTriggerBoostFunction=-1;
                iTriggerBoostFuncBack=-1;
                iBoostFuncStep=5;
                InitWriteAndCheckMotorTorqueTask();
                if(iIndex==0)
                    MOT[MTestZ1].iGali_SingalHomeTask=1;
                else
                    MOT[MTestZ2].iGali_SingalHomeTask=1;

                SW[SwRKManualTStart].Off();

                labDelayStatus->Caption="";
                Task=490;                                                       //jou 981128
            }

            if(iIndex==0)
            {
                if(fMain->edTorue0->Text!="")
                {
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=true;
                    fMain->chkReadTorque2->Checked=false;
                    fMain->edTorue0->Text="";
                    bReadMCU1=true;                                             //kevin 20220225 read MCU DATA
                }
            }
            else
            {
                if(fMain->edTorue1->Text!="")
                {
                    COM2->InitReadTorueTask();
                    fMain->chkReadTorque1->Checked=false;
                    fMain->chkReadTorque2->Checked=true;
                    fMain->edTorue1->Text="";
                    bReadMCU2=true;                                             //kevin 20220225 read MCU DATA
                }
            }
            break;
        case 360:
            if(DoSetupTest(iIndex))
            {
                Task=350;
                break;
            }

            if(WaitManualStepKey())
            {
                InitWriteAndCheckMotorTorqueTask();
                MOT[MTestZ1].iGali_SingalHomeTask=1;
                SW[SwRKManualTStart].Off();
                labDelayStatus->Caption="";
                Task=490;                                                       //jou 981128
            }
            break;
        case 490:
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_SingalHome())
                {
                    palArm1Height->Caption=0;
                    Task=2100;
                }
                else                                                            //Steven 20100208
                {
                    if(MOT[MTestZ1].iGali_SingalHomeTask==900)
                        MOT[MTestZ1].iGali_SingalHomeTask=500;
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_SingalHome())
                {
                    palArm2Height->Caption=0;
                    Task=2100;
                }
                else                                                            //Steven 20100208
                {
                    if(MOT[MTestZ2].iGali_SingalHomeTask==900)
                        MOT[MTestZ2].iGali_SingalHomeTask=500;
                }
            }
            break;

        case 2100:
            fMain->chkReadTorque1->Checked=false;
            fMain->chkReadTorque2->Checked=false;
            if(iIndex==0)
            {
                if(MOT[MTestZ1].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm1Height->Caption=0;
                    Task=2200;                                                  //Steven 20110829 : 升上來後才檢查IC掉料
                }
            }
            else
            {
                if(MOT[MTestZ2].Gali_MotMove(0, GotIndexZSpeed(iSpeedZ*1000)))  //JimmyChiu 20211028 : All speed can set by speed setting.
                {
                    palArm2Height->Caption=0;
                    Task=2200;                                                  //Steven 20110829 : 升上來後才檢查IC掉料
                }
            }
            break;
        case 2200:
            if(bContinueContact)
            {
                Task=1;
                return false;
            }
            return true;                                                        //AutoHeight 完成
    }
    return false;
}
// ===== end of the LoadCell span (golden :17997) =====
