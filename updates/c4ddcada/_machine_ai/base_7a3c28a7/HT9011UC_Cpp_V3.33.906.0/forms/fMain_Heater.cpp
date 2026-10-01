// =============================================================================
//  forms/fMain_Heater.cpp  --  TfMain::Index16Heater / TfMain::IndexHeatMode /
//                              TfMain::HotplateHeatMode / TfMain::SetTemp
//
//  AI(W906-I01) 20261001 (Ifor01): new file. Standard C++17 translation of four TfMain
//  members of the BCB6 main form, translated from the 906 golden:
//    HT9011UC_Code_V3.33.906.0_20260618/main.cpp (cp950, read-only)
//      TfMain::Index16Heater     main.cpp:18613-20265  (decl main.h:1186)
//      TfMain::IndexHeatMode     main.cpp:20266-20742  (decl main.h:1318)
//      TfMain::HotplateHeatMode  main.cpp:20743-20847  (decl main.h:1319)
//      TfMain::SetTemp           main.cpp:23890-23981  (decl main.h:1321)
//  Work card: docs/handoff/TO_IFOR.md I-01 (Jimmy 1001 11:1x). RULINGS_20261001 #0:
//  golden functions are translated and wired as golden does, heating / IO included.
//  Facade declarations: forms/fMain.h:590 (SetTemp is virtual; the other three are
//  declared on the same line). The golden SetTemp body is W906_SetTempBody below; the
//  virtual SetTemp stays a stub at forms/fMain.cpp:511 (see PORT-ONLY SEAMS).
//
//  PHASE 1 = BODIES. Nothing in the port calls Index16Heater / IndexHeatMode /
//  HotplateHeatMode yet: golden calls IndexHeatMode from TfMain::FormShow (:10722),
//  HotplateHeatMode / IndexHeatMode from TfMain::Timer2Timer (:21528-21643), and
//  starts the heater thread from FormShow (:10138). Those hook points are I-01 phase 2
//  (TO_IFOR.md §4 12:0x: tools/wb_serve.cpp boot segment, WebBridgeTags.cpp:601,
//  HeaterSimTick.cpp's ship arm after I-03).
//  SetTemp is NOT live yet either. Its callers (AutoRetest.cpp:1959 / :1986,
//  SECSGEM/uHGemHT9045.cpp:3682, ProductionInfo/uPAT_Function.cpp:1852) still reach
//  the virtual stub at forms/fMain.cpp:511, which runs W906_SetTempBody only through
//  W906_SetTempHook -- and phase 1 installs that hook nowhere (no wb_serve change).
//  Reason: the body persists the new set point through fTemp_Set->spbSaveClick, whose
//  write tail (uTemp_Set.cpp SAFETY GATE (S2)) and SaveSetupFile (S1) are still gated.
//  Live today, the golden body would reset fHeaterOK / iThermoTask / the hot buffers
//  and return 0, but the second ReadTempFile would read the OLD set point back -- a
//  silent half-success. The hook goes in with phase 2, once S1 / S2 are decided
//  (FROM_IFOR §3 20261001).
//
//  Text: golden VERBATIM, statement by statement, same order, original author
//  comments carried over (cp950-decoded to UTF-8, zero U+FFFD). The golden lines were
//  copied mechanically from the decoded golden by line number (scratchpad generator),
//  not retyped. TfMain members stay unqualified; golden's own `fMain->` spellings are
//  kept. Only `__fastcall` is dropped from the three signatures that had it.
//
//  PORT-ONLY SEAMS (forms/fMain.cpp:511, the virtual stub): W906_SetTemp_Sim
//  (forms/fMain.h:595) non-zero = the value a test wants SetTemp to return without
//  running the body; 0 = W906_SetTempHook's golden body if installed, else 0 (the old
//  stub). W906_InstallSetTemp() at the end of this file installs the hook -- the same
//  forms->sm split as W906_InstallUpdateMainOperateMode (forms/fMain_OperateMode.cpp):
//  TfMain's vtable lives in ht9045_forms and must not reference an ht9045_sm body.
//
//  GATE REGISTER (generated from the gate table; full reason on each #if 0 line)
//  --------------------------------------------------------------------------
//   golden lines           kind               what
//   :19165                 missing-dependency TriTemp_Ch
//   :19781                 missing-dependency TriTemp_Ch
//   :19833                 missing-dependency TriTemp_Ch
//   :19928                 missing-dependency TriTemp_Ch
//   :20223                 missing-dependency TriTemp_Ch
//   :23894                 missing-cpp-state  bChangeTemp
//   :23977                 missing-dependency ATC_InterfaceForm->SendAirMachineStatus
//
//  Toolchain: MinGW g++ 6.3+, C++17. UTF-8 without BOM, CRLF.
// =============================================================================

#include "forms/fMain.h"            // TfMain facade / fMain

#include "cprod.h"                  // Temperature / TestIF_File / DeviceForm_File
#include "cmydef.h"                 // bUT150Install / bUT150HasUse / ATC_SYSTEM / CUSTOMER_CODE / TC401HeaterControl /
                                    // USE_16_HEATER / fHeaterOK / bHeatOKBellowError / SystemStart ...
#include "MachineType.h"            // eTempControll (tc*) / eht* / eATC* / CC_* / test modes
#include "LastSet.h"                // LastSet
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "cpublic.h"                // HeaterLog
#include "csystem.h"                // SetShuttleMode / OpenDUTHeat
#include "mysensor.h"               // Sen[]
#include "myswitch.h"               // SW[]
#include "forms/fTemp_Set.h"        // fTemp_Set (ReadTempFile / spbSaveClick / edWorkTemp / edATCAmbTemp / edSoakTime)
#include "acarry_shims.h"           // ATC_InterfaceForm
#include "atester_shims.h"          // fiosetview / fContact (->fShow) / EPSwitchOnOff / eEPSwBoth
#include "canary_support.h"         // ShowMyMessageBox_YES_NO
#include "bthermo.h"                // iThermoTask / ClearAllHotBuffer
#include "SECSGEM/SecsEventType.h"  // SECS_EVENT
#include "SECSGEM/SecsEventReport.h" // EventReport(unsigned)

// NewRecordProcess: same declaration as cMyDB.h:129 (body cMyDB.cpp, golden cMyDB.cpp:1545-1562). cMyDB.h itself is
// not included: it re-declares canary_support.h's RecordProcess / MyDBIProcessNew with their default arguments
// (canary_support.h:70 / :300), which is an error in one TU. acatchtray_shims.h:439 has a third, differently
// defaulted declaration -- not included either.
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");

#include <cstdlib>                  // atof

// ---------------------------------------------------------------------------
// TfMain::Index16Heater -- golden main.cpp:18613-20265 (1653 lines)
// ---------------------------------------------------------------------------
void TfMain::Index16Heater(bool bHeaterMode)                                    //Steven 20111207 : 獨立加熱開關
{
    bool bMode[2]={true, true}, bL17HeaterOnWhenCloseSite[2]={false, false};    //Steven 20161218 : Fixed for L17
    bool bUseArm1PnPMode=false;                                                 //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱

    for(int i=tcHead1; i<=tcHead4; i++)                                         //Ifor 20160418 清除記憶體資料，避免多Site 切換 少Site 出現異常
        bUT150Install[i]=false;

    for(int i=tcAa1; i<=tcBd2; i++)
        bUT150Install[i]=false;

    for(int i=tcAe1; i<=tcBh2; i++)                                             //Steven 20140923 : Index使用EJ1N版32組加熱器   //wei  20150213
        bUT150Install[i]=false;

    if(TC401HeaterControl==NoHeater)                                            //Steven 20171227 (Wei) : Add for HT-9045L
    {
        return;
    }

    if(Temperature.bSLKNoHeatUp)                                                //Steven 20230221 : Amb Ctr mode, SLK no heat up
    {
        return;
    }

    SetShuttleMode(false);                                                      //Steven 20231103 : for NN mode

    if(bHeaterMode)
    {
        bL17HeaterOnWhenCloseSite[0]=true;
        bL17HeaterOnWhenCloseSite[1]=true;
        if(IniConfig.bD30EnableSiteModeSelect && TestIF_File.iShuttleMode==1)   //kevin 20200501 mark L17 關SITE 遲續加熱
        {
            if(TestIF_File.iShuttle_Sel==0)                                     //Front Arm Only
            {
                bMode[1]=false;
                bL17HeaterOnWhenCloseSite[1]=false;                             //Steven 20161218 : Fixed for L17
                if(IniConfig.bL17HeadHeaterOnWhenCloseSite==false)              //kevin 20200501 add close Site 不加熱
                    bL17HeaterOnWhenCloseSite[0]=false;
            }
            else if(TestIF_File.iShuttle_Sel==1)                                //Rear Arm Only
            {
                bMode[0]=false;
                bL17HeaterOnWhenCloseSite[0]=false;                             //Steven 20161218 : Fixed for L17
                if(IniConfig.bL17HeadHeaterOnWhenCloseSite==false)              //kevin 20200501 add close Site 不加熱
                    bL17HeaterOnWhenCloseSite[1]=false;
            }
        }
        else
        {
            if(IniConfig.bL17HeadHeaterOnWhenCloseSite==false)                  //kevin 20200501 add close Site 不加熱
            {
                bL17HeaterOnWhenCloseSite[0]=false;
                bL17HeaterOnWhenCloseSite[1]=false;
            }
        }

        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                      //jou 2015-05-28 修正 UseArm1PickPlaceArm2Test mode Arm2 不會開啟加熱
           TestIF_File.bArm1PickPlaceArm2Test==true)
        {
            bUseArm1PnPMode=true;                                               //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱
            if((ATC_SYSTEM==eNewATCSystem || ATC_SYSTEM==eATCHonPrecType || ATC_SYSTEM==eWinWay) && Temperature.bATCActiveCooling==true)                        //Steven 20160906 : ATC與[L17]衝突  //Jimmychiu 20210906
            {
            }
            else
            {
                bMode[0]=true;
                bMode[1]=true;
            }
        }

        if(USE_16_HEATER==eht16Heater       ||
           USE_16_HEATER==eht16HeaterEJ1N   ||
           USE_16_HEATER==eht32HeaterEJ1N   ||                                  //Steven 20140923 : Index使用EJ1N版32組加熱器
           USE_16_HEATER==eht32HeaterKT4H   ||                                  //Steven 20150211 : Index使用KT4H版32組加熱器
           USE_16_HEATER==eht16HeaterDTME08 ||                                  //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
           USE_16_HEATER==eht32HeaterDTME08 )                                   //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
        {                                                                       //需要搭配TfTemp_Set::SetTempPanelCaption()做修改
            if(TestIF_File.iTestMode==SingleSite)
            {
                if(ATC_SYSTEM>=eATC60 && Temperature.bATCActiveCooling==true)   //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;                              //Ifor 20160127 :ATC 目前使用四組控制器 若有增加需新增
                    if(bUseArm1PnPMode==true)                                   //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱
                    {
                        bUT150Install[tcAa2]=LastSet.bUseTestSocket[0][0][0];
                    }
                    else
                    {
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                    }

                    if(Temperature.bMultiZoneEnable)                            //wei 20240617 Multi Zone
                    {
                        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==0) || TestIF_File.iShuttleMode==0)                                         //Index 1
                        {
                            bUT150Install[tcAa1]=Temperature.bZoneTempEnable[0];
                            bUT150Install[tcAb1]=Temperature.bZoneTempEnable[1];
                            bUT150Install[tcAc1]=Temperature.bZoneTempEnable[2];
                            bUT150Install[tcAd1]=Temperature.bZoneTempEnable[3];
                        }
                        else
                        {
                            bUT150Install[tcAa1]=false;
                            bUT150Install[tcAb1]=false;
                            bUT150Install[tcAc1]=false;
                            bUT150Install[tcAd1]=false;
                        }

                        if((TestIF_File.iShuttleMode==1 && TestIF_File.iShuttle_Sel==1) || TestIF_File.iShuttleMode==0)                                         //Index 2
                        {
                            bUT150Install[tcAa2]=Temperature.bZoneTempEnable[0];
                            bUT150Install[tcAb2]=Temperature.bZoneTempEnable[1];
                            bUT150Install[tcAc2]=Temperature.bZoneTempEnable[2];
                            bUT150Install[tcAd2]=Temperature.bZoneTempEnable[3];
                        }
                        else
                        {
                            bUT150Install[tcAa2]=false;
                            bUT150Install[tcAb2]=false;
                            bUT150Install[tcAc2]=false;
                            bUT150Install[tcAd2]=false;
                        }
                    }
                }
                else
                {
                    if(Temperature.bATC70Active)                                //Eliot 2015_0105
                    {
                        bMode[0]=false;
                        bMode[1]=false;
                    }

                    if(TestIF_File.b2CableLayoutKit)                            //JerryYang 20160826 single site兩條線版本(使用一支加熱棒)
                    {
                        bUT150Install[tcAa1]=bMode[0];
                        bUT150Install[tcAa2]=bMode[1];
                    }
                    else
                    {
                        bUT150Install[tcAa1]=bMode[0];
                        bUT150Install[tcBa1]=bMode[0];
                        bUT150Install[tcAa2]=bMode[1];
                        bUT150Install[tcBa2]=bMode[1];
                    }
                }
            }
            else if(TestIF_File.iTestMode==DualSite)                            //1x2
            {
                if(TestIF_File.bUse1x3SiteKit &&                                //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                   Temperature.bATCActiveCooling==true)
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcAb1]=false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=false;
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if((ATC_SYSTEM==eNewATCSystem ||                           //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType ||
                    ATC_SYSTEM==eWinWay) &&
                   Temperature.bATCActiveCooling==true)                         //Jimmychiu 20210906
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;                              //Ifor 20160127 :ATC 目前使用四組控制器 若有增加需新增
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                    if(bUseArm1PnPMode==true)
                    {
                        bUT150Install[tcAa2]=LastSet.bUseTestSocket[0][0][0];
                        bUT150Install[tcAb2]=LastSet.bUseTestSocket[0][0][1];
                    }
                    else
                    {
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                    }
                }
                else if(ATC_SYSTEM>eATC30 &&                                    //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
                        ATC_SYSTEM!=eNonChamber   &&                            //Steven 20140314 : For HT9045WA
                        (Temperature.bATCActiveCooling==true ||
                        (Temperature.bATCActiveCooling==false &&
                         LastSet.iTemperature==Tempture_Ambient)))              //Steven 20120712 : 常溫沒開ATC,不顯示溫度
                {
                    ;
                }
                else
                {
                    if(Temperature.bATC70Active)                                //Eliot 2015_0105
                    {
                        bMode[0]=false;
                        bMode[1]=false;
                    }

                    if(TestIF_File.bUse1x3SiteKit)                              //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAb1]=false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else if(TestIF_File.b2CableLayoutKit)                       //JerryYang 20160826 1x2 兩條線版本使用兩支加熱棒
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                            (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))       //jou 2015-10-15  : 16溫控器 1條線版本
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][0][1] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][0][1] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)   //Site Aa
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }
            }
            else if(TestIF_File.iTestMode==TriSite1X3)                          //Frank 20160329 add for 1x3_4
            {
                bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];           //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
            }
            else if(TestIF_File.iTestMode==QualSite1X4)                         //1x4
            {
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 修正 ATC 1*4 溫度顯示異常問題
                   Temperature.bATCActiveCooling==true)
                {
                    //Ifor 20160418 : 32 site ATC
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                    if(bUseArm1PnPMode==true)                                   //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱
                    {
                        bUT150Install[tcAa2]=LastSet.bUseTestSocket[0][0][0];
                        bUT150Install[tcAb2]=LastSet.bUseTestSocket[0][0][1];
                        bUT150Install[tcAc2]=LastSet.bUseTestSocket[0][0][2];
                        bUT150Install[tcAd2]=LastSet.bUseTestSocket[0][0][3];
                    }
                    else
                    {
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                    }
                }
                else if(TestIF_File.b2CableLayoutKit)                           //Steven 20150724 : 16溫控器 2條線版本
                {
                    if(CUSTOMER_CODE==CC_ATEC &&
                       DeviceForm_File.iHeadDeviceCT==3)                        //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //jou 2015-10-15  : 16溫控器 1條線版本
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
            }
            else if(TestIF_File.iTestMode==DualSite2x1)                         //wei 20170417 (Steven) : Fixed for Dual Site 2x1
            {
                //ChungHung 20130910 alter for SCK can close site by Index
                bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];           //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
            }
            else if(TestIF_File.iTestMode==QualSite2X2)                         //2x2
            {
                if(TestIF_File.bSquare_OctalKit ||                              //Steven 20141224 : 2x2Site使用8Site Kit
                   TestIF_File.b2x2Use16SiteKit)                                //Steven 20191113 : 2x2Site使用16Site Kit
                {
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if(CUSTOMER_CODE==CC_ATEC && TestIF_File.b2CableLayoutKit && DeviceForm_File.iHeadDeviceCT==3)     //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) || (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))  //JerryYang 20181207 2x2新增一條線版本
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
//                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
//                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
//                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
//                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    if(bUseArm1PnPMode==true)                                   //Ifor 20191126 : add Arm1 Pnp Mode Arm2 需開加熱
                    {
                        bUT150Install[tcAa2]=LastSet.bUseTestSocket[0][0][0];
                        bUT150Install[tcBa2]=LastSet.bUseTestSocket[0][1][0];
                        bUT150Install[tcAb2]=LastSet.bUseTestSocket[0][0][1];
                        bUT150Install[tcBb2]=LastSet.bUseTestSocket[0][1][1];
                    }
                    else
                    {
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }
            }
            else if(TestIF_File.iTestMode==QualSite2X2N)                        //Frank 20200520 2X2NN Mode
            {
                if(TestIF_File.bUse1x3SiteKit &&                                //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                   Temperature.bATCActiveCooling==true)
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcAb1]=false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=false;
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if((ATC_SYSTEM==eNewATCSystem ||                           //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType ||
                    ATC_SYSTEM==eWinWay) &&
                   Temperature.bATCActiveCooling==true)                         //Jimmychiu 20210906
                {                                                               //Ifor 20160127 :ATC 目前使用四組控制器 若有增加需新增
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                }
                else if(ATC_SYSTEM>eATC30 &&                                    //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
                        ATC_SYSTEM!=eNonChamber   &&                            //Steven 20140314 : For HT9045WA
                        (Temperature.bATCActiveCooling==true ||
                        (Temperature.bATCActiveCooling==false &&
                         LastSet.iTemperature==Tempture_Ambient)))              //Steven 20120712 : 常溫沒開ATC,不顯示溫度
                {
                    ;
                }
                else
                {
                    if(Temperature.bATC70Active)                                //Eliot 2015_0105
                    {
                        bMode[0]=false;
                        bMode[1]=false;
                    }
                    else if(TestIF_File.bUse1x3SiteKit)                         //KevinCheng 20260109 : 1x2Site and 2x2 NN mode 使用1x3Site Kit
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAb1]=false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }
            }
            else if(TestIF_File.iTestMode==_6Site2X3)                           //ChungHung 20140115 add for 2x3_6
            {
                //ChungHung 20130910 alter for SCK can close site by Index
                bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];           //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                bUT150Install[tcAd1]=false;
                bUT150Install[tcBd1]=false;
                bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];           //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                bUT150Install[tcAd2]=false;
                bUT150Install[tcBd2]=false;
            }
            else if(TestIF_File.iTestMode==_6Site2X3N)                          //Steven 20220425 : 2X3NN Mode
            {
                if((ATC_SYSTEM==eNewATCSystem ||                                //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType) &&
                   Temperature.bATCActiveCooling==true)
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;                              //Ifor 20160127 :ATC 目前使用四組控制器 若有增加需新增
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                }
                else if(ATC_SYSTEM>eATC30 &&                                    //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
                        ATC_SYSTEM!=eNonChamber   &&                            //Steven 20140314 : For HT9045WA
                        (Temperature.bATCActiveCooling==true ||
                        (Temperature.bATCActiveCooling==false &&
                         LastSet.iTemperature==Tempture_Ambient)))              //Steven 20120712 : 常溫沒開ATC,不顯示溫度
                {
                    ;
                }
                else
                {
                    if(Temperature.bATC70Active)                                //Eliot 2015_0105
                    {
                        bMode[0]=false;
                        bMode[1]=false;
                    }

                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
            }
            else if(TestIF_File.iTestMode==_8Site2X4)
            {
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                         //Ifor 20170622 (wei) add ATC 溫度獨立判斷
                {
                    if(TestIF_File.bOctal_16Kit &&                              //JerryYang 20220810 : 8 site SLK支援16site SLK
                       TestIF_File.dSiteXPitch<=40 &&
                       (USE_16_HEATER==eht32HeaterEJ1N ||
                        USE_16_HEATER==eht32HeaterKT4H))
                    {
                        bUT150Install[tcAa1]=false;
                        bUT150Install[tcAb1]=false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAg1]=false;
                        bUT150Install[tcAh1]=false;
                        bUT150Install[tcBa1]=false;
                        bUT150Install[tcBb1]=false;
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBg1]=false;
                        bUT150Install[tcBh1]=false;

                        bUT150Install[tcAa2]=false;
                        bUT150Install[tcAb2]=false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAg2]=false;
                        bUT150Install[tcAh2]=false;
                        bUT150Install[tcBa2]=false;
                        bUT150Install[tcBb2]=false;
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBg2]=false;
                        bUT150Install[tcBh2]=false;
                    }
                    else
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:false;
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:false;
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:false;
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:false;
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:false;
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:false;
                    }

                    if(ATC_InterfaceForm->iATC_MODE_TYPE==61)                   //Ztex 2023.12.31 for HT-1032 AT
                    {
                        for(int i=0; i<32; i++)
                        {
#if 0 // GATE(W906-I01) missing-dependency: TriTemp_Ch -- golden `extern int TriTemp_Ch[ATC_MAX_SITE]`（HT-1032 三溫機的通道表）移植樹沒有定義（MainCalcCore.h:69、forms/fTemp_Set.h:218 都記成缺）；外層是 iATC_MODE_TYPE==61（HT-1032 AT）才進的大括號 for，閘掉這一行後迴圈是空的、沒有副作用 (golden :19165)
                            bUT150HasUse[TriTemp_Ch[i]]=bUT150Install[TriTemp_Ch[i]];
#endif
                        }
                    }
                }
                else if(CUSTOMER_CODE==CC_ATEC &&
                        TestIF_File.b2CableLayoutKit &&
                        DeviceForm_File.iHeadDeviceCT==4)                       //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1] ||
                                          LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3] ||
                                          LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) ||
                        (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))           //Sam 20210524 2x4 新增一條線版本
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1] ||
                                          LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3] ||
                                          LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if(TestIF_File.bNS8000CS)                                  //Steven 20120606 : 16溫控器 8Site使用Hontech頭
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||         //JerryYang 20160613 修正使用NS8000 4組加熱器只對應4個site 造成關site時會加熱異常
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=false;
                    bUT150Install[tcBc1]=false;
                    bUT150Install[tcAd1]=false;
                    bUT150Install[tcBd1]=false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1] ||         //JerryYang 20160613 修正使用NS8000 4組加熱器只對應4個site 造成關site時會加熱異常
                                          LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1] ||
                                          LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3] ||         //JerryYang 20190429 0->1, fix加熱對應錯誤
                                          LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3] ||
                                          LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=false;
                    bUT150Install[tcBc2]=false;
                    bUT150Install[tcAd2]=false;
                    bUT150Install[tcBd2]=false;
                }
                else if(TestIF_File.dSiteXPitch<=30)                            //Steven 20160621 : 2x4 XPitch 30mm
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=false;
                    bUT150Install[tcBc1]=false;
                    bUT150Install[tcAd1]=false;
                    bUT150Install[tcBd1]=false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];            //JerryYang 20190429 0->1, fix加熱對應錯誤
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=false;
                    bUT150Install[tcBc2]=false;
                    bUT150Install[tcAd2]=false;
                    bUT150Install[tcBd2]=false;
                }
                else if(TestIF_File.bOctal_12Kit &&                             //Sam 20180419 (jou) : 修正開啟 "Octal site use 12 Site Layout Kit" 加熱棒對應的位置
                        (USE_16_HEATER==eht32HeaterEJ1N ||
                         USE_16_HEATER==eht32HeaterKT4H ||
                         USE_16_HEATER==eht32HeaterDTME08))
                {
                    bUT150Install[tcAa1]=false;
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAf1]=false;
                    bUT150Install[tcBa1]=false;
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBf1]=false;

                    bUT150Install[tcAa2]=false;
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAf2]=false;
                    bUT150Install[tcBa2]=false;
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBf2]=false;
                }
                else if(TestIF_File.bOctal_16Kit &&                             //JerryYang 20220810 : 8 site SLK支援16site SLK
                        TestIF_File.dSiteXPitch<=40 &&
                        (USE_16_HEATER==eht32HeaterEJ1N ||
                         USE_16_HEATER==eht32HeaterKT4H))
                {
                    bUT150Install[tcAa1]=false;
                    bUT150Install[tcAb1]=false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAg1]=false;
                    bUT150Install[tcAh1]=false;
                    bUT150Install[tcBa1]=false;
                    bUT150Install[tcBb1]=false;
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBg1]=false;
                    bUT150Install[tcBh1]=false;

                    bUT150Install[tcAa2]=false;
                    bUT150Install[tcAb2]=false;
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAg2]=false;
                    bUT150Install[tcAh2]=false;
                    bUT150Install[tcBa2]=false;
                    bUT150Install[tcBb2]=false;
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBg2]=false;
                    bUT150Install[tcBh2]=false;
                }
                else
                {
                    //ChungHung 20130910 alter for SCK can close site by Index
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
            }
            else if(TestIF_File.iTestMode==_8Site2X4N)                          //Wei 20231211 : 2X4NN Mode
            {
                if((ATC_SYSTEM==eNewATCSystem ||                                //Ifor 20160128 新增 New ATC System 、 ATC2.0 開關 Site設定
                    ATC_SYSTEM==eATCHonPrecType) &&
                   Temperature.bATCActiveCooling==true)
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;                              //Ifor 20160127 :ATC 目前使用四組控制器 若有增加需新增
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                }
                else if(ATC_SYSTEM>eATC30 &&                                    //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
                        ATC_SYSTEM!=eNonChamber   &&                            //Steven 20140314 : For HT9045WA
                        (Temperature.bATCActiveCooling==true ||
                        (Temperature.bATCActiveCooling==false &&
                         LastSet.iTemperature==Tempture_Ambient)))              //Steven 20120712 : 常溫沒開ATC,不顯示溫度
                {
                    ;
                }
                else
                {
                    if(Temperature.bATC70Active)                                //Eliot 2015_0105
                    {
                        bMode[0]=false;
                        bMode[1]=false;
                    }

                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];       //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
            }
            else if(TestIF_File.iTestMode==_10Site2X5)                          //wei 20190614 10 site
            {
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 新增 ATC功能開啟時才切換bUT150Install開關設定      // ATC未新增
                   Temperature.bATCActiveCooling==true)
                {
                    if(TestIF_File.bUse32Heater==false)                         //kevin 20170705 add  LS Kit 1對2 layout kit
                    {
                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        //ChungHung 20141224 add for ATK 12 site close site close heater
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][4] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][4] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        //ChungHung 20141224 add for ATK 12 site close site close heater
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][4] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][4] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else
                    {
                        //Ifor 20160418 : 24 site ATC
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:false;
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:false;
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:false;
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:false;
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:false;
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:false;

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:false;
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:false;
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:false;
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:false;
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:false;
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:false;
                    }
                }
                else
                {
                    if((USE_16_HEATER==eht32HeaterEJ1N ||
                        USE_16_HEATER==eht32HeaterKT4H )  &&                    //wei 20150213
                       (TestIF_File.bUse32Heater ||                             //Steven 20140923 : Index使用EJ1N版32組加熱器
                        TestIF_File.b12SiteUse10Heater))
                    {
                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                        /*if(TestIF_File.bUse32Heater)                          //Mark 2x5不需要用
                        {
                            //Steven 20170203 (wei): Fixed for Direct Heater On Off
                            bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        }
                        else
                        {
                            bUT150Install[tcAf1]=false;
                            bUT150Install[tcBf1]=false;
                            bUT150Install[tcAf2]=false;
                            bUT150Install[tcBf2]=false;
                        }                                */
                    }
                    else
                    {
                        if(TestIF_File.bUse32Heater)                            //ChungHung 20141224 add for ATK 12 site close site close heater
                        {                                                       //Steven 20170203 (wei): Fixed for Direct Heater On Off
                            bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                            //ChungHung 20141224 add for ATK 12 site close site close heater
                            bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        }
                        else
                        {
                            bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][4] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][4] )?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                            //ChungHung 20141224 add for ATK 12 site close site close heater
                            bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][4] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][4] )?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        }
                    }
                }
            }
            else if(TestIF_File.iTestMode==_12Site2X6)
            {
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160514 新增 ATC功能開啟時才切換bUT150Install開關設定
                   Temperature.bATCActiveCooling==true)
                {
                    if(TestIF_File.bUse32Heater==false)                         //kevin 20170705 add  LS Kit 1對2 layout kit
                    {
                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        //ChungHung 20141224 add for ATK 12 site close site close heater
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=false;
                        bUT150Install[tcBd1]=false;

                        //ChungHung 20141224 add for ATK 12 site close site close heater
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][4] || LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][4] || LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=false;
                        bUT150Install[tcBd2]=false;
                    }
                    else
                    {
                        if(TestIF_File.b2x6Use2x8SitSLK)                        //Steven 20240807 : 12Site使用16Site Kit
                        {
                            bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                            bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                            bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                            bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                            bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:false;
                            bUT150Install[tcAg1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:false;
                            bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:false;
                            bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:false;
                            bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:false;
                            bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:false;
                            bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:false;
                            bUT150Install[tcBg1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:false;

                            bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                            bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                            bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                            bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                            bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:false;
                            bUT150Install[tcAg2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:false;
                            bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:false;
                            bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:false;
                            bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:false;
                            bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:false;
                            bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:false;
                            bUT150Install[tcBg2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:false;
                        }
                        else                                                    //Ifor 20160418 : 24 site ATC
                        {
                            bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                            bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                            bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                            bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                            bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:false;
                            bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:false;
                            bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:false;
                            bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:false;
                            bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:false;
                            bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:false;
                            bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:false;
                            bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:false;

                            bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                            bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                            bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                            bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                            bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:false;
                            bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:false;
                            bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:false;
                            bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:false;
                            bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:false;
                            bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:false;
                            bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:false;
                            bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:false;
                        }
                    }
                }
                else
                {
                    if((USE_16_HEATER==eht32HeaterEJ1N ||
                        USE_16_HEATER==eht32HeaterKT4H ||
                        USE_16_HEATER==eht32HeaterDTME08) &&                    //wei 20150213
                       (TestIF_File.bUse32Heater ||
                        TestIF_File.b12SiteUse10Heater))                        //Steven 20140923 : Index使用EJ1N版32組加熱器
                    {
                        if(TestIF_File.b2x6Use2x8SitSLK)                        //Steven 20240807 : 12Site使用16Site Kit
                        {
                            bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                            bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                            bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                            bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                            bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:false;
                            bUT150Install[tcAg1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:false;
                            bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:false;
                            bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:false;
                            bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:false;
                            bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:false;
                            bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:false;
                            bUT150Install[tcBg1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:false;

                            bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                            bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                            bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                            bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                            bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:false;
                            bUT150Install[tcAg2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:false;
                            bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:false;
                            bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:false;
                            bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:false;
                            bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:false;
                            bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:false;
                            bUT150Install[tcBg2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:false;
                        }
                        else                                                    //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        {
                            bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                            bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                            bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                            if(TestIF_File.bUse32Heater)                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                            {
                                bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                                bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                                bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                                bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                            }
                            else
                            {
                                bUT150Install[tcAf1]=false;
                                bUT150Install[tcBf1]=false;
                                bUT150Install[tcAf2]=false;
                                bUT150Install[tcBf2]=false;
                            }
                        }
                    }
                    else
                    {
                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        //ChungHung 20141224 add for ATK 12 site close site close heater
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=false;
                        bUT150Install[tcBd1]=false;

                        //ChungHung 20141224 add for ATK 12 site close site close heater
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][4] || LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][4] || LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=false;
                        bUT150Install[tcBd2]=false;
                    }
                }
            }
            else if(TestIF_File.iTestMode==_16Site2X8)                          //2x8  //Eliot 2009_12_25
            {
                if(ATC_SYSTEM==eNewATCSystem &&                                 //Ifor 20160418 新增 New ATC System 開關 bUT150Install 設定
                   Temperature.bATCActiveCooling==true)                         //Ifor 20160514 新增 ATC功能開啟時才切換bUT150Install開關設定
                {
                    if(TestIF_File.bUse32Heater==false)                         //kevin 20170705 add  LS Kit 1對2
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][6] || LastSet.bUseTestSocket[0][0][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][6] || LastSet.bUseTestSocket[0][1][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][4] || LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][4] || LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][6] || LastSet.bUseTestSocket[1][0][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][6] || LastSet.bUseTestSocket[1][1][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                    else                                                        //Ifor 20160418 : 32 site ATC
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:false;
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:false;
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:false;
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:false;
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:false;
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:false;
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:false;

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:false;
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:false;
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:false;
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:false;
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:false;
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:false;
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:false;

                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:false;
                        bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:false;
                        bUT150Install[tcAg1]=(LastSet.bUseTestSocket[0][0][6])?bMode[0]:false;
                        bUT150Install[tcAh1]=(LastSet.bUseTestSocket[0][0][7])?bMode[0]:false;
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:false;
                        bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:false;
                        bUT150Install[tcBg1]=(LastSet.bUseTestSocket[0][1][6])?bMode[0]:false;
                        bUT150Install[tcBh1]=(LastSet.bUseTestSocket[0][1][7])?bMode[0]:false;

                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:false;
                        bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:false;
                        bUT150Install[tcAg2]=(LastSet.bUseTestSocket[1][0][6])?bMode[1]:false;
                        bUT150Install[tcAh2]=(LastSet.bUseTestSocket[1][0][7])?bMode[1]:false;
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:false;
                        bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:false;
                        bUT150Install[tcBg2]=(LastSet.bUseTestSocket[1][1][6])?bMode[1]:false;
                        bUT150Install[tcBh2]=(LastSet.bUseTestSocket[1][1][7])?bMode[1]:false;

                        if(ATC_InterfaceForm->iATC_MODE_TYPE==61)               //Ztex 2023.12.31 for HT-1032 AT
                        {
                            for(int i=0; i<32; i++)
                            {
#if 0 // GATE(W906-I01) missing-dependency: TriTemp_Ch -- golden `extern int TriTemp_Ch[ATC_MAX_SITE]`（HT-1032 三溫機的通道表）移植樹沒有定義（MainCalcCore.h:69、forms/fTemp_Set.h:218 都記成缺）；外層是 iATC_MODE_TYPE==61（HT-1032 AT）才進的大括號 for，閘掉這一行後迴圈是空的、沒有副作用 (golden :19781)
                                bUT150HasUse[TriTemp_Ch[i]]=bUT150Install[TriTemp_Ch[i]];
#endif
                            }
                        }
                    }
                }
                else
                {
                    if((USE_16_HEATER==eht32HeaterEJ1N  ||
                        USE_16_HEATER==eht32HeaterKT4H  ||                      //Steven 20150211 : Index使用KT4H版32組加熱器
                        USE_16_HEATER==eht32HeaterDTME08) &&
                       TestIF_File.bUse32Heater)                                //Steven 20140923 : Index使用EJ1N版32組加熱器
                    {                                                           //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][0][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAg1]=(LastSet.bUseTestSocket[0][0][6])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAh1]=(LastSet.bUseTestSocket[0][0][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][1][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBg1]=(LastSet.bUseTestSocket[0][1][6])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBh1]=(LastSet.bUseTestSocket[0][1][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[1][0][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAf2]=(LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAg2]=(LastSet.bUseTestSocket[1][0][6])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAh2]=(LastSet.bUseTestSocket[1][0][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[1][1][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBf2]=(LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBg2]=(LastSet.bUseTestSocket[1][1][6])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBh2]=(LastSet.bUseTestSocket[1][1][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                        if(ATC_InterfaceForm->iATC_MODE_TYPE==61)               //Ztex 2023.12.31 for HT-1032 AT
                        {
                            for(int i=0; i<32; i++)
                            {
#if 0 // GATE(W906-I01) missing-dependency: TriTemp_Ch -- golden `extern int TriTemp_Ch[ATC_MAX_SITE]`（HT-1032 三溫機的通道表）移植樹沒有定義（MainCalcCore.h:69、forms/fTemp_Set.h:218 都記成缺）；外層是 iATC_MODE_TYPE==61（HT-1032 AT）才進的大括號 for，閘掉這一行後迴圈是空的、沒有副作用 (golden :19833)
                                bUT150HasUse[TriTemp_Ch[i]]=bUT150Install[TriTemp_Ch[i]];
#endif
                            }
                        }
                    }
                    else                                                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][0][6] || LastSet.bUseTestSocket[0][0][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][1][6] || LastSet.bUseTestSocket[0][1][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[1][0][0] || LastSet.bUseTestSocket[1][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[1][1][0] || LastSet.bUseTestSocket[1][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[1][0][2] || LastSet.bUseTestSocket[1][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[1][1][2] || LastSet.bUseTestSocket[1][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[1][0][4] || LastSet.bUseTestSocket[1][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[1][1][4] || LastSet.bUseTestSocket[1][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[1][0][6] || LastSet.bUseTestSocket[1][0][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[1][1][6] || LastSet.bUseTestSocket[1][1][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }
            }
            else if(TestIF_File.iTestMode==_16Site4X4)                          //Sam 20190226 : 16Site4X4 //2x4
            {
                if(ATC_SYSTEM==eNewATCSystem &&
                   Temperature.bATCActiveCooling==true)                         //Ifor 20170622 (wei) add ATC 溫度獨立判斷
                {
                    if(TestIF_File.bOctal_16Kit &&
                       TestIF_File.dSiteXPitch<=40 &&
                       (USE_16_HEATER==eht32HeaterEJ1N ||
                        USE_16_HEATER==eht32HeaterKT4H))                        //JerryYang 20220810 : 8 site SLK支援16site SLK
                    {
                        bUT150Install[tcAa1]=false;
                        bUT150Install[tcAb1]=false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][2][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAg1]=false;
                        bUT150Install[tcAh1]=false;
                        bUT150Install[tcBa1]=false;
                        bUT150Install[tcBb1]=false;
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBg1]=false;
                        bUT150Install[tcBh1]=false;

                        bUT150Install[tcAa2]=false;
                        bUT150Install[tcAb2]=false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[0][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAf2]=(LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAg2]=false;
                        bUT150Install[tcAh2]=false;
                        bUT150Install[tcBa2]=false;
                        bUT150Install[tcBb2]=false;
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[0][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBf2]=(LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBg2]=false;
                        bUT150Install[tcBh2]=false;
                    }
                    else
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0])?bMode[0]:false;
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][3][0])?bMode[0]:false;
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][1])?bMode[0]:false;
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][3][1])?bMode[0]:false;
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][2])?bMode[0]:false;
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][2])?bMode[0]:false;
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][3])?bMode[0]:false;
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][3])?bMode[0]:false;

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0])?bMode[1]:false;
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][1][0])?bMode[1]:false;
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][1])?bMode[1]:false;
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][1][1])?bMode[1]:false;
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][2])?bMode[1]:false;
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][2])?bMode[1]:false;
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][3])?bMode[1]:false;
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][3])?bMode[1]:false;
                    }

                    if(ATC_InterfaceForm->iATC_MODE_TYPE==61)                   //Ztex 2023.12.31 for HT-1032 AT
                    {
                        for(int i=0; i<32; i++)
                        {
#if 0 // GATE(W906-I01) missing-dependency: TriTemp_Ch -- golden `extern int TriTemp_Ch[ATC_MAX_SITE]`（HT-1032 三溫機的通道表）移植樹沒有定義（MainCalcCore.h:69、forms/fTemp_Set.h:218 都記成缺）；外層是 iATC_MODE_TYPE==61（HT-1032 AT）才進的大括號 for，閘掉這一行後迴圈是空的、沒有副作用 (golden :19928)
                            bUT150HasUse[TriTemp_Ch[i]]=bUT150Install[TriTemp_Ch[i]];
#endif
                        }
                    }
                }
                else if(CUSTOMER_CODE==CC_ATEC && TestIF_File.b2CableLayoutKit && DeviceForm_File.iHeadDeviceCT==4)     //JerryYang 20191003 for 艾科 支援7000的2個感溫點SLK, Mars說用兩條線的選項
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][2][1] ||
                                          LastSet.bUseTestSocket[0][3][0] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][2][3] ||
                                          LastSet.bUseTestSocket[0][3][2] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if((IniConfig.bL30Use1CableLayoutKitByConfig==false && TestIF_File.b1CableLayoutKit) || (IniConfig.bL30Use1CableLayoutKitByConfig==true && IniConfig.bL30Use1CableLayoutKit))  //Sam 20210524 2x4 新增一條線版本
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][2][1] ||
                                          LastSet.bUseTestSocket[0][3][0] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][2][3] ||
                                          LastSet.bUseTestSocket[0][3][2] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else if(TestIF_File.bNS8000CS)                                  //Steven 20120606 : 16溫控器 8Site使用Hontech頭
                {                                                               //ChungHung 20130910 alter for SCK can close site by Index
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][2][1] ||         //JerryYang 20160613 修正使用NS8000 4組加熱器只對應4個site 造成關site時會加熱異常
                                          LastSet.bUseTestSocket[0][3][0] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][2][1] ||
                                          LastSet.bUseTestSocket[0][3][0] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][2][3] ||
                                          LastSet.bUseTestSocket[0][3][2] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][2][3] ||
                                          LastSet.bUseTestSocket[0][3][2] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=false;
                    bUT150Install[tcBc1]=false;
                    bUT150Install[tcAd1]=false;
                    bUT150Install[tcBd1]=false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||         //JerryYang 20160613 修正使用NS8000 4組加熱器只對應4個site 造成關site時會加熱異常
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1] ||
                                          LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||         //JerryYang 20190429 0->1, fix加熱對應錯誤
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3] ||
                                          LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=false;
                    bUT150Install[tcBc2]=false;
                    bUT150Install[tcAd2]=false;
                    bUT150Install[tcBd2]=false;
                }
                else if(TestIF_File.dSiteXPitch<=30)                            //Steven 20160621 : 2x4 XPitch 30mm
                {                                                               //ChungHung 20130910 alter for SCK can close site by Index
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][3][0] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][3][2] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=false;
                    bUT150Install[tcBc1]=false;
                    bUT150Install[tcAd1]=false;
                    bUT150Install[tcBd1]=false;
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];            //JerryYang 20190429 0->1, fix加熱對應錯誤
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=false;
                    bUT150Install[tcBc2]=false;
                    bUT150Install[tcAd2]=false;
                    bUT150Install[tcBd2]=false;
                }
                else if(TestIF_File.bOctal_12Kit &&                             //Sam 20180419 (jou) : 修正開啟 "Octal site use 12 Site Layout Kit" 加熱棒對應的位置
                        (USE_16_HEATER==eht32HeaterEJ1N ||
                         USE_16_HEATER==eht32HeaterKT4H ||
                         USE_16_HEATER==eht32HeaterDTME08))
                {
                    bUT150Install[tcAa1]=false;
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAf1]=false;
                    bUT150Install[tcBa1]=false;
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBf1]=false;

                    bUT150Install[tcAa2]=false;
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAe2]=(LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAf2]=false;
                    bUT150Install[tcBa2]=false;
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBe2]=(LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBf2]=false;
                }
                else if(TestIF_File.bOctal_16Kit &&                             //JerryYang 20220810 : 8 site SLK支援16site SLK
                        TestIF_File.dSiteXPitch<=40 &&
                        (USE_16_HEATER==eht32HeaterEJ1N ||
                         USE_16_HEATER==eht32HeaterKT4H))
                {
                    bUT150Install[tcAa1]=false;
                    bUT150Install[tcAb1]=false;
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][2][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAg1]=false;
                    bUT150Install[tcAh1]=false;
                    bUT150Install[tcBa1]=false;
                    bUT150Install[tcBb1]=false;
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBg1]=false;
                    bUT150Install[tcBh1]=false;

                    bUT150Install[tcAa2]=false;
                    bUT150Install[tcAb2]=false;
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAe2]=(LastSet.bUseTestSocket[0][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAf2]=(LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAg2]=false;
                    bUT150Install[tcAh2]=false;
                    bUT150Install[tcBa2]=false;
                    bUT150Install[tcBb2]=false;
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBe2]=(LastSet.bUseTestSocket[0][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBf2]=(LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBg2]=false;
                    bUT150Install[tcBh2]=false;
                }
                else
                {
                    if(TestIF_File.dSiteYPitch<50.0 &&
                       TestIF_File.bUse32Heater==false)
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][2][1] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][2][3] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][0][1] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][0][3] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                        sTempsite[tcAa1]= IntToStr(TestIF_File.iSiteMap[2][0])+","+IntToStr(TestIF_File.iSiteMap[3][0]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcBa1]= IntToStr(TestIF_File.iSiteMap[2][1])+","+IntToStr(TestIF_File.iSiteMap[3][1]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcAb1]= IntToStr(TestIF_File.iSiteMap[2][2])+","+IntToStr(TestIF_File.iSiteMap[3][2]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcBb1]= IntToStr(TestIF_File.iSiteMap[2][3])+","+IntToStr(TestIF_File.iSiteMap[3][3]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcAa2]= IntToStr(TestIF_File.iSiteMap[0][0])+","+IntToStr(TestIF_File.iSiteMap[1][0]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcBa2]= IntToStr(TestIF_File.iSiteMap[0][1])+","+IntToStr(TestIF_File.iSiteMap[1][1]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcAb2]= IntToStr(TestIF_File.iSiteMap[0][2])+","+IntToStr(TestIF_File.iSiteMap[1][2]);                                        //kevin 20190928 add 溫度error 秀site編號
                        sTempsite[tcBb2]= IntToStr(TestIF_File.iSiteMap[0][3])+","+IntToStr(TestIF_File.iSiteMap[1][3]);                                        //kevin 20190928 add 溫度error 秀site編號
                    }
                    else
                    {
                        bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];   //KenHsieh 20251004 : 補上L17 //kevin 20191109 change ARM 2  //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                        bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];   //KenHsieh 20251004 : 補上L17 //kevin 20191109      ARM 1
                        bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }
            }
            else if(TestIF_File.iTestMode==_32Site4X8N)                         //Steven 20140512 : For HT-9047
            {
                if(TestIF_File.dSiteYPitch<50.0 && TestIF_File.bUse32Heater==false)
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];            //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][2][1] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][2][3] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][4] || LastSet.bUseTestSocket[0][3][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][2][5] || LastSet.bUseTestSocket[0][3][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][6] || LastSet.bUseTestSocket[0][3][6])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][2][7] || LastSet.bUseTestSocket[0][3][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][0][1] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][0][3] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][1][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][0][5] || LastSet.bUseTestSocket[0][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][6] || LastSet.bUseTestSocket[0][1][6])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][0][7] || LastSet.bUseTestSocket[0][1][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                    sTempsite[tcAa1]= IntToStr(TestIF_File.iSiteMap[2][0])+","+IntToStr(TestIF_File.iSiteMap[3][0]);    //kevin 20190928 add 溫度error 秀site編號
                    sTempsite[tcBa1]= IntToStr(TestIF_File.iSiteMap[2][1])+","+IntToStr(TestIF_File.iSiteMap[3][1]);    //kevin 20190928 add 溫度error 秀site編號
                    sTempsite[tcAb1]= IntToStr(TestIF_File.iSiteMap[2][2])+","+IntToStr(TestIF_File.iSiteMap[3][2]);
                    sTempsite[tcBb1]= IntToStr(TestIF_File.iSiteMap[2][3])+","+IntToStr(TestIF_File.iSiteMap[3][3]);
                    sTempsite[tcAc1]= IntToStr(TestIF_File.iSiteMap[2][4])+","+IntToStr(TestIF_File.iSiteMap[3][4]);
                    sTempsite[tcBc1]= IntToStr(TestIF_File.iSiteMap[2][5])+","+IntToStr(TestIF_File.iSiteMap[3][4]);
                    sTempsite[tcAd1]= IntToStr(TestIF_File.iSiteMap[2][6])+","+IntToStr(TestIF_File.iSiteMap[3][6]);
                    sTempsite[tcBd1]= IntToStr(TestIF_File.iSiteMap[2][7])+","+IntToStr(TestIF_File.iSiteMap[3][7]);
                    sTempsite[tcAa2]= IntToStr(TestIF_File.iSiteMap[0][0])+","+IntToStr(TestIF_File.iSiteMap[1][0]);
                    sTempsite[tcBa2]= IntToStr(TestIF_File.iSiteMap[0][1])+","+IntToStr(TestIF_File.iSiteMap[1][1]);
                    sTempsite[tcAb2]= IntToStr(TestIF_File.iSiteMap[0][2])+","+IntToStr(TestIF_File.iSiteMap[1][2]);
                    sTempsite[tcBb2]= IntToStr(TestIF_File.iSiteMap[0][3])+","+IntToStr(TestIF_File.iSiteMap[1][3]);
                    sTempsite[tcAc2]= IntToStr(TestIF_File.iSiteMap[0][4])+","+IntToStr(TestIF_File.iSiteMap[1][4]);
                    sTempsite[tcBc2]= IntToStr(TestIF_File.iSiteMap[0][5])+","+IntToStr(TestIF_File.iSiteMap[1][5]);
                    sTempsite[tcAd2]= IntToStr(TestIF_File.iSiteMap[0][6])+","+IntToStr(TestIF_File.iSiteMap[1][6]);
                    sTempsite[tcBd2]= IntToStr(TestIF_File.iSiteMap[0][7])+","+IntToStr(TestIF_File.iSiteMap[1][7]);
                }
                else if(TestIF_File.dSiteYPitch>=50.0 && TestIF_File.bUse32Heater==false)                               //Sam 20191020 : 32 Site 加熱問題 Fix
                {
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0] || LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][2] || LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][4] || LastSet.bUseTestSocket[0][2][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][6] || LastSet.bUseTestSocket[0][2][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][3][0] || LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][3][2] || LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][4] || LastSet.bUseTestSocket[0][3][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][6] || LastSet.bUseTestSocket[0][3][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0] || LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][2] || LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][4] || LastSet.bUseTestSocket[0][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][6] || LastSet.bUseTestSocket[0][0][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][1][0] || LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][1][2] || LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][4] || LastSet.bUseTestSocket[0][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][6] || LastSet.bUseTestSocket[0][1][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                }
                else
                {
                    //Steven 20170203 (wei): Fixed for Direct Heater On Off
                    bUT150Install[tcAa1]=(LastSet.bUseTestSocket[0][2][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAb1]=(LastSet.bUseTestSocket[0][2][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAc1]=(LastSet.bUseTestSocket[0][2][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcAd1]=(LastSet.bUseTestSocket[0][2][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBa1]=(LastSet.bUseTestSocket[0][3][0])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBb1]=(LastSet.bUseTestSocket[0][3][1])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBc1]=(LastSet.bUseTestSocket[0][3][2])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                    bUT150Install[tcBd1]=(LastSet.bUseTestSocket[0][3][3])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                    bUT150Install[tcAa2]=(LastSet.bUseTestSocket[0][0][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAb2]=(LastSet.bUseTestSocket[0][0][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAc2]=(LastSet.bUseTestSocket[0][0][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcAd2]=(LastSet.bUseTestSocket[0][0][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBa2]=(LastSet.bUseTestSocket[0][1][0])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBb2]=(LastSet.bUseTestSocket[0][1][1])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBc2]=(LastSet.bUseTestSocket[0][1][2])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    bUT150Install[tcBd2]=(LastSet.bUseTestSocket[0][1][3])?bMode[1]:bL17HeaterOnWhenCloseSite[1];

                    if((USE_16_HEATER==eht32HeaterEJ1N  ||
                        USE_16_HEATER==eht32HeaterKT4H  ||                      //Steven 20150211 : Index使用KT4H版32組加熱器
                        USE_16_HEATER==eht32HeaterDTME08) &&
                       TestIF_File.bUse32Heater)                                //Steven 20140923 : Index使用EJ1N版32組加熱器
                    {
                        //Steven 20170203 (wei): Fixed for Direct Heater On Off
                        bUT150Install[tcAe1]=(LastSet.bUseTestSocket[0][2][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAf1]=(LastSet.bUseTestSocket[0][2][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAg1]=(LastSet.bUseTestSocket[0][2][6])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcAh1]=(LastSet.bUseTestSocket[0][2][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBe1]=(LastSet.bUseTestSocket[0][3][4])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBf1]=(LastSet.bUseTestSocket[0][3][5])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBg1]=(LastSet.bUseTestSocket[0][3][6])?bMode[0]:bL17HeaterOnWhenCloseSite[0];
                        bUT150Install[tcBh1]=(LastSet.bUseTestSocket[0][3][7])?bMode[0]:bL17HeaterOnWhenCloseSite[0];

                        bUT150Install[tcAe2]=(LastSet.bUseTestSocket[0][0][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAf2]=(LastSet.bUseTestSocket[0][0][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAg2]=(LastSet.bUseTestSocket[0][0][6])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcAh2]=(LastSet.bUseTestSocket[0][0][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBe2]=(LastSet.bUseTestSocket[0][1][4])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBf2]=(LastSet.bUseTestSocket[0][1][5])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBg2]=(LastSet.bUseTestSocket[0][1][6])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                        bUT150Install[tcBh2]=(LastSet.bUseTestSocket[0][1][7])?bMode[1]:bL17HeaterOnWhenCloseSite[1];
                    }
                }

                if(ATC_InterfaceForm->iATC_MODE_TYPE==61)                       //Ztex 2023.12.31 for HT-1032 AT
                {
                    for(int i=0; i<32; i++)
                    {
#if 0 // GATE(W906-I01) missing-dependency: TriTemp_Ch -- golden `extern int TriTemp_Ch[ATC_MAX_SITE]`（HT-1032 三溫機的通道表）移植樹沒有定義（MainCalcCore.h:69、forms/fTemp_Set.h:218 都記成缺）；外層是 iATC_MODE_TYPE==61（HT-1032 AT）才進的大括號 for，閘掉這一行後迴圈是空的、沒有副作用 (golden :20223)
                        bUT150HasUse[TriTemp_Ch[i]]=bUT150Install[TriTemp_Ch[i]];
#endif
                    }
                }
            }
        }
        else if(USE_16_HEATER==eht4Heater)
        {
            if(TestIF_File.iTestMode==SingleSite &&
               TestIF_File.bSingleHeater==true)                                 //JerryYang 20161013 新增Single site一支加熱棒模式
            {
                bUT150Install[tcHead1]=bMode[0];
                bUT150Install[tcHead2]=false;
                bUT150Install[tcHead3]=bMode[1];
                bUT150Install[tcHead4]=false;
            }
            else
            {
                bUT150Install[tcHead1]=bMode[0];
                bUT150Install[tcHead2]=bMode[0];
                bUT150Install[tcHead3]=bMode[1];
                bUT150Install[tcHead4]=bMode[1];
            }
            for(int i=tcAa1; i<=tcBd2; i++)
                bUT150Install[i]=false;
        }

        if(IniConfig.bD58UseArm1PickPlaceArm2Test==true &&                      //Ifor 20190815 : Arm1 Pick Place 無安裝加熱片 不加熱
           TestIF_File.bArm1PickPlaceArm2Test==true     &&
           TestIF_File.bArm1UseHeat==false              )
        {
            for(int i=tcAa1; i<=tcBd1; i++)
                bUT150Install[i]=false;

            for(int i=tcAe1; i<=tcBh1; i++)
                bUT150Install[i]=false;
        }
    }
}
//******************************************************************************
//
//  注意!! Index16Heater為Handler針對Direct Heater的開關, 修改時要小心!!
//
//******************************************************************************

// ---------------------------------------------------------------------------
// TfMain::IndexHeatMode -- golden main.cpp:20266-20742 (477 lines)
// ---------------------------------------------------------------------------
void TfMain::IndexHeatMode()                                                   //AI(W906-I01) 20261001: golden `void __fastcall TfMain::IndexHeatMode()`
{
    bool bHeaterMode=false;                                                     //Steven 20120411
    if(W906_FormShowing("fiosetview", fiosetview->fShow)==false)                                                //Steven 20111124 : 方便試機  //AI(W906-I01) 20261001: golden「IO 畫面開著嗎」改問頁面表（W906_FormShowing，csystem.h；成員照傳，同 adam6024.cpp:941）
    {                                                                           //Steven 20110725 Start: 風扇改二段速
        if(Temperature.iIndexHeatMode==HeadOnly ||
           LastSet.iTemperature==Tempture_Ambient ||
           Temperature.iIndexHeatMode==HeadSocket)                              //ChungHung 20150423 add HeadSocket      //Head Only
        {
            if(SW[SwHeaterFanSpeed].Enable==true)                               //Steven 20110725 : 風扇改二段速
            {
                if(ESD_Monitor==false)                                          //Steven 20140109 : 搭配3M EM Aware要轉快的
                    SW[SwHeaterFanSpeed].OnOff(true);                           //慢速
                else
                    SW[SwHeaterFanSpeed].OnOff(false);                          //快速
            }
        }
        else
        {
            if(SW[SwHeaterFanSpeed].Enable==true)
                SW[SwHeaterFanSpeed].OnOff(false);                              //快速
        }
    }

    if(Temperature.iIndexHeatMode==HeadOnly)                                    //Head Only //Steven 20090926 Start: Index Heating Mode
    {
        if(ATC_SYSTEM>eATC60 && ATC_SYSTEM!=eNonChamber)                        //20141204 ChungHung ???? //2014-05-30    Dell    for ATC6.0
        {
            if((ATC_SYSTEM==eNewATCSystem ||                                    //Ifor 20161118 整合New ATC && ATC2.0 開啟Heat顯示
                ATC_SYSTEM==eATCHonPrecType  ||
                ATC_SYSTEM==eWinWay) &&
               Temperature.bATCActiveCooling==true)
            {
                bHeaterMode=true;
            }
            else if((TestIF_File.iTestMode==DualSite ||
                     TestIF_File.iTestMode==SingleSite) &&
                    Temperature.bATCActiveCooling==true &&
                    Temperature.bATCTemperatureSet==false)
            {
                bHeaterMode=false;                                              //Steven 20120411
            }
            else
            {
                if((TestIF_File.iTestMode==_8Site2X4   ||
                    TestIF_File.iTestMode==QualSite2X2 ||
                    TestIF_File.iTestMode==_8Site1X4   ||                       //ChungHung 20150528 add for 海思 _8Site1x4
                    TestIF_File.iTestMode==QualSite1X4 ||
                    TestIF_File.iTestMode==DualSite    ||
                    TestIF_File.iTestMode==DualSite2x1 ||
                    TestIF_File.iTestMode==_16Site4X4) &&                       //Sam 20190226 : 16Site4X4
                   (TestIF_File.bNS7000CS==true ||
                    TestIF_File.bHontechLayoutKit2x2==true ||
                    TestIF_File.bNS7000kit==true) &&
                   LastSet.iTemperature==Tempture_Ambient)                      //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
                {
                    bHeaterMode=false;                                          //Steven 20120411
                }
                else
                {
                    bHeaterMode=true;                                           //Steven 20120411
                }
            }
        }
        else
        {
            bHeaterMode=true;                                                   //Steven 20120411
        }

        OpenDUTHeat(false);                                                     //2013-01-15    Dell DUT(Socket Base)增加為4顆
        if(ATC_SYSTEM>eATC60 ||                                                 //20141204 ChungHung add for ATC3.0
           (IniConfig.bSPILFunction==true ||                                    //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction //ChungHung 20120210 : 常溫,矽品說不要顯示chamber溫度
            CUSTOMER_CODE==CC_AMKOR_Korea  ||                                   //Steven 20160215 : ATK 說不要顯示Chamber溫度
            CUSTOMER_CODE==CC_SIGURD_HUKOU))                                    //Alick 20160628 矽格湖口HeadOnly不要顯示Chamber溫度
        {
            bUT150Install[tcChamber]=false;
        }
        else if(ATC_SYSTEM==eNonChamber)                                        //Steven 20140314 : For HT9045WA
        {
            bUT150Install[tcChamber]=false;
        }
        else
        {
            if(CUSTOMER_CODE==CC_KYEC_LEE)
            {
                bUT150Install[tcChamber]=false;                                 //Ifor 20191105 : KYEC HeadOnly 不顯示 Chamber 溫度
            }
            else
            {
                bUT150Install[tcChamber]=true;
            }
        }
    }
    else if(Temperature.iIndexHeatMode==ChamberOnly)                            //Chamber Only
    {
        bHeaterMode=false;                                                      //Steven 20120411
        OpenDUTHeat(false);                                                     //2013-01-15    Dell DUT(Socket Base)增加為4顆
        bUT150Install[tcChamber]=true;
    }
    else if(Temperature.iIndexHeatMode==HeadChamber)                            //Head + Chamber
    {
        bHeaterMode=true;                                                       //Steven 20120411
        OpenDUTHeat(false);                                                     //2013-01-15    Dell DUT(Socket Base)增加為4顆
        bUT150Install[tcChamber]=true;
    }
    else if(Temperature.iIndexHeatMode==SocketChamber)                          //Socket + Chamber
    {
        bHeaterMode=false;                                                      //Steven 20120411
        if(iSocketBaseTempCount==eDut4ea || iSocketBaseTempCount==eDut2ea)      //kevin 20130321 : SOCKET BASE 溫控器一個改4個
        {
            OpenDUTHeat(true);                                                  //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆
            bUT150Install[tcSocket]=false;
        }
        else
        {
            OpenDUTHeat(false);                                                 //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆
            bUT150Install[tcSocket]=true;
        }

        if(ATC_SYSTEM==eNonChamber)                                             //Steven 20140314 : For HT9045WA
        {
            bUT150Install[tcChamber]=false;
        }
        else if(CUSTOMER_CODE==CC_HONPREC_QC && Temperature.bByPassChamber)     //KenHsieh 20230301 : By Pass Chamber
        {
            bUT150Install[tcChamber]=false;
        }
        else if(LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot)                      //kevin 20140918 常溫加熱 恆溫控制
        {
            bUT150Install[tcChamber]=true;
        }
        else if(ATC_SYSTEM>eATC60 ||                                            //20141204 ChungHung add for ATC3.0 //ChungHung 20120210 : 常溫,矽品說不要顯示chamber溫度)
                IniConfig.bSPILFunction==true)                                  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
        {
            bUT150Install[tcChamber]=false;
        }
        else if(CUSTOMER_CODE==CC_SCS)
        {
            bUT150Install[tcChamber]=true;
        }
    }
    else if(Temperature.iIndexHeatMode==HeadSocket)                             //jou 2012-03-12 假如ATC模式只支援Head mode & Head + Socket mode
    {
        if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                        //20141204 ChungHung add for ATC3.0   //2014-05-30    Dell    for ATC6.0
        {
            if((ATC_SYSTEM==eNewATCSystem ||                                    //Ifor 20161118 整合New ATC && ATC2.0 開啟Heat顯示 移除(TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite)條件
                ATC_SYSTEM==eATCHonPrecType  ||
                ATC_SYSTEM==eWinWay) &&
               Temperature.bATCActiveCooling==true)                             //Jimmychiu 20210906
            {
                bHeaterMode=true;
            }
            else if((TestIF_File.iTestMode==DualSite ||
                     TestIF_File.iTestMode==SingleSite) &&
                    Temperature.bATCActiveCooling==true &&
                    Temperature.bATCTemperatureSet==false)
            {
                bHeaterMode=false;                                              //Steven 20120411
            }
            else
            {
                if((TestIF_File.iTestMode==_8Site2X4   ||
                    TestIF_File.iTestMode==QualSite2X2 ||
                    TestIF_File.iTestMode==_8Site1X4   ||                       //ChungHung 20150528 add for 海思 _8Site1x4
                    TestIF_File.iTestMode==QualSite1X4 ||
                    TestIF_File.iTestMode==DualSite    ||
                    TestIF_File.iTestMode==DualSite2x1 ||
                    TestIF_File.iTestMode==_16Site4X4) &&                       //Sam 20190226 : 16Site4X4
                   (TestIF_File.bNS7000CS==true ||
                    TestIF_File.bHontechLayoutKit2x2==true ||
                    TestIF_File.bNS7000kit==true) &&
                   LastSet.iTemperature==Tempture_Ambient)                      //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
                {
                    bHeaterMode=false;                                          //Steven 20120411
                }
                else
                {
                    bHeaterMode=true;                                           //Steven 20120411
                }
            }
        }
        else if(MachineTypeChoice==Type_HT9046_LS &&
                SubMachineType==Type_HT9016C)                                   //Sam 20230628 : 修正空跑不判斷加熱
        {
            if(LastSet.iRealDummy==REALLY)
                bHeaterMode=true;
            else
                bHeaterMode=false;
        }
        else
        {
            bHeaterMode=true;                                                   //Steven 20120411
        }

        if(iSocketBaseTempCount==eDut4ea || iSocketBaseTempCount==eDut2ea)      //kevin 20130321 : SOCKET BASE 溫控器一個改4個
        {
            OpenDUTHeat(true);                                                  //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆
            bUT150Install[tcSocket]=false;
        }
        else
        {
            OpenDUTHeat(false);                                                 //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆
            //bUT150Install[tcSocket]=true;
            if((CosFunction.bHiSiliconFunction) &&
               ((LastSet.iTemperature==Tempture_AmbientHot &&
                 Temperature.fWorkTemperBase<=30) ||
                LastSet.iTemperature==Tempture_Ambient))                        //kevin 20191227 HIS 常溫不用DUT
                bUT150Install[tcSocket]=false;                                  //kevin 20200130
            else
                bUT150Install[tcSocket]=true;
        }
        bUT150Install[tcChamber]=false;
    }
    else if(Temperature.iIndexHeatMode==HeadChamberSocket)                      //kevin 20131209 add  Head + Chamber +Socket
    {
        if(ATC_SYSTEM>eATC30 && ATC_SYSTEM!=eNonChamber)                        //20141204 ChungHung add for ATC3.0  //2014-05-30    Dell    for ATC6.0
        {
            if((TestIF_File.iTestMode==DualSite ||
                TestIF_File.iTestMode==SingleSite) &&
               Temperature.bATCActiveCooling==true &&
               Temperature.bATCTemperatureSet==false)
            {
                bHeaterMode=false;                                              //Steven 20120411
            }
            else
            {
                if((TestIF_File.iTestMode==_8Site2X4    ||
                    TestIF_File.iTestMode==QualSite2X2  ||
                    TestIF_File.iTestMode==_8Site1X4    ||                      //ChungHung 20150528 add for 海思 _8Site1x4
                    TestIF_File.iTestMode==QualSite1X4  ||
                    TestIF_File.iTestMode==DualSite     ||
                    TestIF_File.iTestMode==DualSite2x1  ||
                    TestIF_File.iTestMode==_16Site4X4) &&                       //Sam 20190226 : 16Site4X4
                   (TestIF_File.bNS7000CS==true ||
                    TestIF_File.bHontechLayoutKit2x2==true  ||
                    TestIF_File.bNS7000kit==true) &&
                   LastSet.iTemperature==Tempture_Ambient)                      //jou 2015-12-10 SCS 要求 Hontech Layout kit要選擇Hontech.
                {
                    bHeaterMode=false;                                          //Steven 20120411
                }
                else
                {
                    bHeaterMode=true;                                           //Steven 20120411
                }
            }
        }
        else
        {
            bHeaterMode=true;                                                   //Steven 20120411
        }

        if(ATC_SYSTEM!=eNonChamber)                                             //Steven 20140314 : For HT9045WA
            bUT150Install[tcChamber]=true;

        if(iSocketBaseTempCount==eDut4ea || iSocketBaseTempCount==eDut2ea)      //kevin 20130321 : SOCKET BASE 溫控器一個改4個
        {
            OpenDUTHeat(true);                                                  //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆
            bUT150Install[tcSocket]=false;
        }
        else
        {
            OpenDUTHeat(false);                                                 //kevin 20130321  2013-01-15    Dell DUT(Socket Base)增加為4顆
            //bUT150Install[tcSocket]=true;
            if(CosFunction.bHiSiliconFunction &&
               ((LastSet.iTemperature==Tempture_AmbientHot &&
                 Temperature.fWorkTemperBase<=30) ||
                 LastSet.iTemperature==Tempture_Ambient))                       //kevin 20200130 HIS 常溫不用DUT
                bUT150Install[tcSocket]=false;
            else
                bUT150Install[tcSocket]=true;
        }
    }

    if(Index_ESDAir &&
       ((LastSet.iTemperature==Tempture_AmbientHot &&
         Temperature.fWorkTemperBase>30) ||
        LastSet.iTemperature==Tempture_Hot))                                    //kevin 20200207 add index ESD temp
    {
        bUT150Install[tcIndexESD]=true;
    }
    else
    {
        bUT150Install[tcIndexESD]=false;
    }

    if(INSTALL_HEAT_GUN>0 && Temperature.bActiveHeatGun)
    {
        bUT150Install[tcHeatGun1]=true;                                         //kevin 20120523
        bUT150Install[tcHeatGun2]=true;                                         //kevin 20120523
    }
    else
    {
        bUT150Install[tcHeatGun1]=false;                                        //kevin 20120523
        bUT150Install[tcHeatGun2]=false;                                        //kevin 20120523
    }

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        ;
    }
    else
    {
        if(INSTALL_ATC_HEAT_GUN>0 && (Temperature.bATCActiveCooling || CUSTOMER_CODE==CC_HONPREC_QC))                   //JerryYang 20220408 : add for ATC3.5
        {
            bUT150Install[tcATCHotAir1]=true;
            bUT150Install[tcATCHotAir2]=true;
        }
        else
        {
            bUT150Install[tcATCHotAir1]=false;
            bUT150Install[tcATCHotAir2]=false;
        }
    }

    if(ATC_SYSTEM!=eATCUninstall && LastSet.iTemperature==Tempture_Ambient)     //Steven 20160301 : HT-9045HW針對常溫時不顯示浮動頭溫度。
    {
        if(ATC_SYSTEM==eNonChamber)
        {
            if(TestIF_File.bNS7000kit || TestIF_File.bNS7000CS)                 //Steven 20160401 : HT-9045HW針對常溫時NS7000Layout kit不顯示浮動頭溫度。
                bHeaterMode=false;
        }
        else if(Temperature.bATC70Active==false && Temperature.bATCActiveCooling==false)
        {
            if(TestIF_File.bNS7000kit || TestIF_File.bNS7000CS)
                bHeaterMode=false;
        }
    }

    Index16Heater(bHeaterMode);                                                 //Steven 20111207 : 獨立加熱開關

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        if(IniConfig.bD30EnableSiteModeSelect &&
           TestIF_File.iShuttleMode==1)
        {
            if(TestIF_File.iShuttle_Sel==0)                                     //Front Arm Only    //開關Shuttle
            {
                bUT150Install[tcShuttle1]=true;
                bUT150Install[tcShuttle2]=true;
                bUT150Install[tcShuttle3]=true;
                bUT150Install[tcShuttle4]=true;
                bUT150HasUse[tcShuttle1]=true;
                bUT150HasUse[tcShuttle2]=true;
                bUT150HasUse[tcShuttle3]=false;
                bUT150HasUse[tcShuttle4]=false;
            }
            else if(TestIF_File.iShuttle_Sel==1)
            {
                bUT150Install[tcShuttle1]=true;
                bUT150Install[tcShuttle2]=true;
                bUT150Install[tcShuttle3]=true;
                bUT150Install[tcShuttle4]=true;
                bUT150HasUse[tcShuttle1]=false;
                bUT150HasUse[tcShuttle2]=false;
                bUT150HasUse[tcShuttle3]=true;
                bUT150HasUse[tcShuttle4]=true;
            }
        }
        else
        {
            bUT150Install[tcShuttle1]=true;
            bUT150Install[tcShuttle2]=true;
            bUT150Install[tcShuttle3]=true;
            bUT150Install[tcShuttle4]=true;
            bUT150HasUse[tcShuttle1]=true;
            bUT150HasUse[tcShuttle2]=true;
            bUT150HasUse[tcShuttle3]=true;
            bUT150HasUse[tcShuttle4]=true;
        }

        if(AirStream_Select==1)                                                 //開關AirStream
        {
            bUT150Install[tcATCHotAir1]=true;
            bUT150Install[tcATCHotAir2]=true;
            if(Temperature.bEnableArm_1_Air==true || Temperature.bEnableArm_2_Air==true)
                bUT150HasUse[tcATCHotAir1] =true;
            else
                bUT150HasUse[tcATCHotAir1] =false;

            if(Temperature.bEnableSocket_Air==true)
                bUT150HasUse[tcATCHotAir2] =true;
            else
                bUT150HasUse[tcATCHotAir2] =false;
        }
        else
        {
            bUT150Install[tcATCHotAir1]=false;
            bUT150Install[tcATCHotAir2]=false;
            bUT150HasUse[tcATCHotAir1] =false;
            bUT150HasUse[tcATCHotAir2] =false;
        }
        /*
        if(INDEXDOORHEATER==1 && Temperature.bUseTriTempHeater_Ini[3]==true)    //Ztex 2023.10.23 Add Index Door Heater
        {
            bUT150Install[tcDoor1]=true;
            bUT150Install[tcDoor2]=true;
            bUT150HasUse[tcDoor1] =true;
            bUT150HasUse[tcDoor2] =true;
        }
        else
        {
            bUT150Install[tcDoor1]=false;
            bUT150Install[tcDoor2]=false;
            bUT150HasUse[tcDoor1] =false;
            bUT150HasUse[tcDoor2] =false;
        }
        */
        bUT150Install[tcCCD]  =false;
        bUT150Install[tcCCD_2]=false;
        bUT150Install[tc2D]   =false;
        bUT150Install[tcLB]   =false;
        bUT150Install[tcChamber]=false;
    }
    else                                                                        //Steven 20090929 : Can use only one arm
    {
        if(IniConfig.bI03AmbientTempControl &&
           LastSet.iTemperature==Tempture_AmbientHot &&
           Temperature.bShuttleNoHeatUp==true)                                  //Steven 20180815 : Amb Ctr mode, shuttle no heat up
        {
            bUT150Install[tcShuttle1]=false;                                    //KEVIN 20180724 ADD SHUTTLE 不加熱
            bUT150Install[tcShuttle2]=false;                                    //KEVIN 20180724 ADD SHUTTLE 不加熱
        }
        else
        {
            if(IniConfig.bD30EnableSiteModeSelect &&
               TestIF_File.iShuttleMode==1)
            {
                if(TestIF_File.iShuttle_Sel==0)                                 //Front Arm Only
                {
                    bUT150Install[tcShuttle1]=true;
                    bUT150Install[tcShuttle2]=false;                            //kevin 20120524
                }
                else if(TestIF_File.iShuttle_Sel==1)
                {
                    bUT150Install[tcShuttle1]=false;                            //kevin 20120524
                    bUT150Install[tcShuttle2]=true;
                }
            }
            else
            {
                bUT150Install[tcShuttle1]=true;
                bUT150Install[tcShuttle2]=true;
            }
        }
    }

    if(W906_FormShowing("fContact", fContact->fShow)==false)  //AI(W906-I01) 20261001: golden「Contact 畫面開著嗎」改問頁面表（W906_FormShowing，csystem.h；成員照傳，同 ckernel.cpp:237）
    {
        if(DeviceForm_File.ContactMode==DirectContactSoftEP ||
           DeviceForm_File.ContactMode==DropContactSoftEP)                      //kevin 20130608 EP 用電磁閥控制 arm1 arm2
        {
        }
        else
        {
            EPSwitchOnOff(eEPSwBoth);
        }
    }

    //----- by dell ccd realtime-------------Steven 20110811
    bUT150Install[tcCCD]=(ATC_SYSTEM<=eATC60)?REAL_TIME_CCD:false;              //2014-05-30    Dell    for ATC6.0
    bUT150Install[tcCCD_2]=(ATC_SYSTEM<=eATC60)?(REAL_TIME_CCD && RTC_TemperNumber==2):false;                           //Isaac 20201217 : RTC CCD增加第二組感溫
    bUT150Install[tc2D] =(CCD2_TEMPER==true);                                   //wei 20160524 2D溫度    //Ifor 20190503 :add 2D溫度有安裝才顯示
    bUT150Install[tcLB] =(LB_TEMP && Temperature.bLBTempFunction);              //Steven 20181023 : LB溫度

    bUT150Install[tcLBUp]   =LB_TEMP_UpDown;                                    //Frank 20241231 : add
    bUT150Install[tcLBDown] =LB_TEMP_UpDown;

    //Steven 20241001 : 保持在最下面------------------------
    if(CosFunction.bAmbientNoShowTemp &&
       LastSet.iTemperature==Tempture_Ambient)                                  //Steven 20150518 : 蘇州矽品要求常溫不顯示溫度
    {
        for(int i=0; i<tcTotalCount; i++)
            bUT150Install[i]=false;
    }
}
//------------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TfMain::HotplateHeatMode -- golden main.cpp:20743-20847 (105 lines)
// ---------------------------------------------------------------------------
void TfMain::HotplateHeatMode()                                                   //AI(W906-I01) 20261001: golden `void __fastcall TfMain::HotplateHeatMode()`
{
    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        if(LastSet.iTemperature==Tempture_AmbientHot)                           //開關HotPlate
        {
            if(bUT150Install[tcHotPlate1]==true || bUT150Install[tcHotPlate2]==true ||
               bUT150Install[tcHotPlate3]==true || bUT150Install[tcHotPlate4]==true)
            {
                bUT150Install[tcHotPlate1]=false;
                bUT150Install[tcHotPlate2]=false;
                bUT150Install[tcHotPlate3]=false;
                bUT150Install[tcHotPlate4]=false;
                bUT150HasUse[tcHotPlate1]=false;
                bUT150HasUse[tcHotPlate2]=false;
                bUT150HasUse[tcHotPlate3]=false;
                bUT150HasUse[tcHotPlate4]=false;
            }
        }
        else if(LastSet.iTemperature==Tempture_Hot)
        {
            if(HotPlateForm.iPlateSelect==1)
            {
                bUT150Install[tcHotPlate1]=true;
                bUT150Install[tcHotPlate2]=true;
                bUT150Install[tcHotPlate3]=true;
                bUT150Install[tcHotPlate4]=true;
                bUT150HasUse[tcHotPlate1]=true;
                bUT150HasUse[tcHotPlate2]=true;
                bUT150HasUse[tcHotPlate3]=false;
                bUT150HasUse[tcHotPlate4]=false;
            }
            else if(HotPlateForm.iPlateSelect==2)
            {
                bUT150Install[tcHotPlate1]=true;
                bUT150Install[tcHotPlate2]=true;
                bUT150Install[tcHotPlate3]=true;
                bUT150Install[tcHotPlate4]=true;
                bUT150HasUse[tcHotPlate1]=false;
                bUT150HasUse[tcHotPlate2]=false;
                bUT150HasUse[tcHotPlate3]=true;
                bUT150HasUse[tcHotPlate4]=true;
            }
            else
            {
                bUT150Install[tcHotPlate1]=true;
                bUT150Install[tcHotPlate2]=true;
                bUT150Install[tcHotPlate3]=true;
                bUT150Install[tcHotPlate4]=true;
                bUT150HasUse[tcHotPlate1]=true;
                bUT150HasUse[tcHotPlate2]=true;
                bUT150HasUse[tcHotPlate3]=true;
                bUT150HasUse[tcHotPlate4]=true;
            }
        }
    }
    else
    {
        if(HotPlateForm.iPlateSelect==1)
        {
            bUT150Install[tcHotPlate2]=false;
            bUT150Install[tcHotPlate1]=true;
        }
        else if(HotPlateForm.iPlateSelect==2)
        {
            bUT150Install[tcHotPlate1]=false;
            bUT150Install[tcHotPlate2]=true;
        }
        else
        {
            bUT150Install[tcHotPlate1]=true;
            bUT150Install[tcHotPlate2]=true;
        }
    }

    if(IniConfig.bTemp25degControl==true)                                       //jou 2014-06-07 Temperature 25 deg. control
    {
        if((TestIF_File.iTestMode==_8Site2X4 ||
            TestIF_File.iTestMode==_16Site2X8 ||
            TestIF_File.iTestMode==_16Site4X4) &&                               //Sam 20190226 : 16Site4X4
           (LastSet.iTemperature==Tempture_Hot||
            LastSet.iTemperature==Tempture_AmbientHot) &&                       //kevin 20180811 (Steven) : add 恆溫控制
            Temperature.iIndexHeatMode==HeadOnly &&
            Temperature.fSoakTime==0 &&
            Temperature.fWorkTemperBase<=25.0)
        {
            bUT150Install[tcHotPlate1]=false;
            bUT150Install[tcHotPlate2]=false;
        }
    }

    if(IniConfig.bI03AmbientTempControl &&
       LastSet.iTemperature==Tempture_AmbientHot)                               //kevin 20140918 恆溫控制
    {
        bUT150Install[tcHotPlate1]=false;
        bUT150Install[tcHotPlate2]=false;
        //========================================
        if(Temperature.bShuttleNoHeatUp==true)                                  //Steven 20180815 : Amb Ctr mode, shuttle no heat up
        {
            bUT150Install[tcShuttle1]=false;                                    //KEVIN 20180724 ADD SHUTTLE 不加熱
            bUT150Install[tcShuttle2]=false;
        }
    }
}
//------------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TfMain::SetTemp -- golden main.cpp:23890-23981 (92 lines)
// ---------------------------------------------------------------------------
int TfMain::W906_SetTempBody(bool bAsk, double fWorkTemp, double fSoakTime)                                                   //AI(W906-I01) 20261001: golden `int __fastcall TfMain::SetTemp(bool bAsk, double fWorkTemp, double fSoakTime)`（本體改名 W906_SetTempBody；虛擬的 TfMain::SetTemp 是 forms/fMain.cpp:511 的樁，經 W906_SetTempHook 呼叫這裡）
{
    int ret;
    AnsiString str;
#if 0 // GATE(W906-I01) missing-cpp-state: bChangeTemp -- golden TfMain 資料成員（main.h:1211），移植樹 TfMain facade 沒有（forms/fMain.h 0 筆）；golden 讀它的 main.cpp:3233（配 bStopChangeTempVisible，移植樹 0 筆）與 :24043-24114 都還沒翻 -- 翻那幾支的人加成員並解開這一行 (golden :23894)
    bChangeTemp=false;
#endif

    if(SystemStart)
        return 1;

    if(bAsk==true)
    {
        if(CUSTOMER_CODE==CC_Greatek || bRefreshFunction)                       //kevin 20181206
        {                                                                       //jou 2011-08-24 改用 ShowMyMessageBox_YES_NO 避免畫面被蓋到下面去
            bRefreshFunction=false;
            ret=1;                                                              //Sam 20171006 (wei) : 超豐不顯示提示，直接 OK //Sam 20180402 (wei) : 超豐溫度不顯示提示 Bug
        }
        else
        {
            ret=ShowMyMessageBox_YES_NO("Temperature set to?", "溫度重新設定？");
        }

        if(ret==2)
        {
            if(LastSet.iTemperature==Tempture_Ambient &&
               Temperature.bATCActiveCooling)                                   //wei 20151013  by Setup File ATC Ambient Temp set
            {
                str.sprintf("Set Temperture=%4.2f, Soak Time=%4.2f", IniConfig.dATCAmbientTemperature, 0.0);
                edATCAmbientTemper->Text=IniConfig.dATCAmbientTemperature;
            }
            else
            {
                str.sprintf("Set Temperture=%4.2f, Soak Time=%4.2f", Temperature.fWorkTemperBase, Temperature.fSoakTime);
                edWorkTemperBase->Text=Temperature.fWorkTemperBase;
            }
            edSoakTime->Text=Temperature.fSoakTime;
            NewRecordProcess("MES2132", "Change work tepmearture and soak time", str);
            return 1;
        }

        str.sprintf("Set Temperture=%4.2f, Soak Time=%4.2f", fWorkTemp, fSoakTime);
        NewRecordProcess("MES2132", "Change work tepmearture and soak time", str);
        iTemperatureOk=1;                                                       //kevin 20150914

        if(dOldWorkTemp!=fWorkTemp)                                             //Ifor 20161220 修改溫度與上次不同才紀錄
        {
            str.sprintf("%s -> %s\n", FormatFloat("0.0", dOldWorkTemp), FormatFloat("0.0", fWorkTemp));
            NewRecordProcess("MES2132", "Temperature Default Change", str);
            dOldWorkTemp = fWorkTemp;
        }

        if(dOldSockTime!=fSoakTime)                                             //Ifor 20161220 修改soak Time 與上次不同才紀錄，時間不要有小數點
        {
            str.sprintf("%s -> %s\n", FormatFloat("0", dOldSockTime), FormatFloat("0", fSoakTime));
            NewRecordProcess("MES2132", "Temperature SoakTime Change", str);
            dOldSockTime = fSoakTime;
        }

        NewRecordProcess("MES2130", "Temperature Wait");                        //kevin 20150914
    }

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
        EventReport(SECS_EVENT.SwitchTempData);                                 //46     Change Temp Default and Soak Time

    fTemp_Set->ReadTempFile(true);
    if(LastSet.iTemperature==Tempture_Ambient && Temperature.bATCActiveCooling)                                         //wei 20151013  by Setup File ATC Ambient Temp set
        fTemp_Set->edATCAmbTemp->Text=fWorkTemp;
    else
        fTemp_Set->edWorkTemp->Text=fWorkTemp;
    fTemp_Set->edSoakTime->Text=fSoakTime;
    fTemp_Set->spbSaveClick(NULL);  //AI(W906-I01) 20261001: golden 傳 this（VCL 的 TfMain*->TObject*）；移植樹 TfMain 沒有 TObject 基底，spbSaveClick 不看 Sender（同 Command.cpp:8658 的既有寫法）
    fTemp_Set->ReadTempFile(true);
    if(LastSet.iTemperature==Tempture_Ambient && Temperature.bATCActiveCooling)                                         //wei 20151013  by Setup File ATC Ambient Temp set
        edATCAmbientTemper->Text=fWorkTemp;
    else
        edWorkTemperBase->Text=fWorkTemp;
    edSoakTime->Text=fSoakTime;
    fHeaterOK=false;
    bHeatOKBellowError=false;                                                   //jou 2014-06-12 修正偶發性秀低溫異常
    if(CosFunction.bNotClearAllHotBuffer==false)                                //JerryYang 20151226 For 矽格 由GPIB設定完溫度後，會馬上再問一次溫度。所以不清除暫存溫度
    {
        ClearAllHotBuffer();
    }
    iThermoTask=1;
    bATCTempAdjustmentOffset=false;
    bSetTempChange=true;                                                        //Ifor 20190306 : add Package Offset 三點校正
    if(AirStream_Select==1 && Tri_Temp_Machine==1)                              //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
#if 0 // GATE(W906-I01) missing-dependency: ATC_InterfaceForm->SendAirMachineStatus -- 全域 ATC_InterfaceForm 是 TATC_InterfaceFormShim（acarry_shims.h:109，只有 iATC_MODE_TYPE）；真的 TATC_InterfaceForm（forms/fATCHandlerSide.h）沒接上。只有 HT-1032 三溫機（AirStream_Select==1 && Tri_Temp_Machine==1）會走到，外層大括號 if 保留 (golden :23977)
        ATC_InterfaceForm->SendAirMachineStatus(1, Temperature.fSetTempature2AirMachine*10, Temperature.dSetIndexAirstreamTemp*10);
#endif
    }
    return 0;
}
//------------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// PORT-ONLY: the SetTemp hook. The virtual TfMain::SetTemp stays a stub in
// forms/fMain.cpp:511 (ht9045_forms, whose vtable must not reach ht9045_sm);
// W906_InstallSetTemp() installs it -- NOT called by wb_serve yet (I-01 phase 1: only tests/test_heater_chain.cpp:96; AI(W906-B21-I01NOTE) 20261001) (same pattern as
// W906_InstallUpdateMainOperateMode, forms/fMain_OperateMode.cpp). Not installed
// (wb_serve today, every other ctest) = the old stub: SetTemp returns W906_SetTemp_Sim.
// ---------------------------------------------------------------------------
extern int (*W906_SetTempHook)(TfMain*, bool, double, double);   // forms/fMain.cpp:511
static int W906_SetTempThunk(TfMain* m, bool bAsk, double fWorkTemp, double fSoakTime) { return m != 0 ? m->W906_SetTempBody(bAsk, fWorkTemp, fSoakTime) : 1; }
void W906_InstallSetTemp() { W906_SetTempHook = &W906_SetTempThunk; }
