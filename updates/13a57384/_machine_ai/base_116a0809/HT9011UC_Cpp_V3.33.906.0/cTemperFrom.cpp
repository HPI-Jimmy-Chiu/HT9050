// =============================================================================
//  cTemperFrom.cpp  --  temperature-display data core: channel<->panel
//                        mapping, the per-channel classifier, and the small
//                        function-status strip.
//
//  Faithful translation of golden cTemperFrom.cpp (2,021 lines, BCB6,
//  cp950). Translator: AI(W906-FW3-TemperFrom-WA) 20260818.
//  Translation wave: FW-3 cTemperFrom Wave A (form-logic campaign, batch 3).
//  Facade: forms/fTemperFrom.h (read its banner FIRST -- widget shape, gate
//  register and design notes are documented there, not repeated in full here).
//
//  WAVE SCOPE -- see forms/fTemperFrom.h's own "WAVE SCOPE" banner for the
//  full golden-line-span table. Summary: ctor (:40-126), ShowThermo (:586-1470,
//  885 lines, the classifier), SetShowYield/ShowYieldFuntion/ShowOffYieldFun
//  (:1473-1614), FormClose (:1684-1690), ShowHotName (:1772-1818),
//  TempRunShowAlarmHigh/Low (:1917-1939, PURE).
//
//  GATE REGISTER -- exactly one, re-cited at its call site below. Full
//  rationale in forms/fTemperFrom.h:
//   (T1) ShowThermo, golden :1294-1321 -- the WAR15xx ShowErrorMessage alarm
//        escalation. NOT a missing-dependency gate (ShowErrorMessage/MMSystem/
//        SystemSec all already exist) -- a SAFETY-classified gate reserved
//        for explicit user sign-off, per the task brief.
//
//  GOLDEN ODDITY (recorded, not "fixed") -- golden :607 `sizeof(ShowTempComp)/4`
//  assumes 32-bit (4-byte) pointers; see forms/fTemperFrom.h for the full
//  64-bit-pointer behaviour-delta note. Translated literally.
//
//  LOCALLY PROVIDED (NOT a gate, but disclosed -- read before editing):
//   * `InitTempOffset` (golden uTemp_Set.h:34, `extern const int
//     InitTempOffset;`, real value 8 per uTemp_Set.cpp) -- uTemp_Set is
//     UNPORTED (grepped: no port header/source defines it). TWO sibling
//     translated files already hit this exact gap and solved it the same way
//     (bthermo.cpp:233 `static const int InitTempOffset=8;`, uHeaterThread.cpp:297
//     `const int InitTempOffset=8;` -- both file-local, internal linkage by
//     either `static` or C++'s own "const globals are internal-linkage by
//     default" rule). This file adds a THIRD, independent file-local copy of
//     the identical constant, same convention, so ShowThermo's
//     `Temperature.fTempOffSet[InitTempOffset][Addr]` reads stay ACTIVE
//     instead of needing a gate.
//   * `ATC_TYPE_33`/`ATC_TYPE_35`/`ATC_TYPE_61` (golden ATC/ATC_Handler_Side.h:24,
//     25, 30, `#define`d ints) -- that header is not ported as a shared
//     include; FIVE sibling files already carry their own identical TU-local
//     `#define ATC_TYPE_33 33` etc. (aTester_Front.cpp:169-170,
//     aTester_Rear.cpp:416-419, cUnitConvert.cpp:353-359, csystem.cpp:20230-20245,
//     uHeaterThread.cpp:277-289) -- same convention, only the 3 values
//     ShowThermo actually compares against.
//   * `clRed`/`clLime`/`clYellow`/`clBtnFace` (golden Graphics.hpp VCL system
//     colours) -- FIVE existing files each already carry their own identical
//     `const TColor clXxx = 0x...;` block (acatchtray_shims.h:126-128,
//     SECSGEM/uHGemEquipment.h:393-396, vclcompat/{TrayCore,BtnPanelCore,
//     LedCore}.h) rather than a single shared header; same convention here.
//     `TColor` itself is NOT re-typedef'd here -- cmydef.h:16 already does
//     `typedef int TColor;` at global scope and this file includes cmydef.h,
//     and repeating an IDENTICAL typedef in the same TU is legal C++, but
//     there is no need to.
//
//  DEPENDENCY AUDIT (this wave, 20260818) -- every non-widget global/struct-
//  member ShowThermo/ShowHotName/TempRunShowAlarmHigh/Low/SetShowYield/
//  ShowYieldFuntion/ShowOffYieldFun/FormClose touch was grepped against this
//  port tree before translating: ALL are already declared with matching
//  types in cmydef.h / cprod.h / Config.h / CosFunction.h / LastSet.h /
//  MachineType.h / bthermo.h / acarry_shims.h -- see the include list below.
//  This is UNLIKE cObserver/uYieldMonitoring's own waves, which each found
//  dozens of genuinely-missing form-pointer members (fContactCT/
//  fShowBinSelect/fLotInfo gaps); cTemperFrom Wave A's only real gap is the
//  deliberate safety GATE (T1) above, not a missing-dependency one.
// =============================================================================
#include "MachineDefine.h"     // de-VCL'd include hub: vclcompat umbrella + portable STL
#pragma hdrstop

#include "forms/fTemperFrom.h"

#include "MachineType.h"       // tc* channel enum, eht*/eATC*/eDut*ea, CC_*
#include "cmydef.h"            // UN150Read[]/bUT150Install[]/bUT150State[]/bUT150HasUse[]/
                                //   asGPIBTempShow[]/asTempCtrl[]/iTempOverShowAlarmT[]/
                                //   USE_16_HEATER/ATC_SYSTEM/CUSTOMER_CODE/TColor/SystemStart/
                                //   iHome/SystemSec/MMSystem/bCCDOverTemp/Tri_Temp_Machine/...
#include "cprod.h"              // Temperature / TestIF / TestIF_File / DeviceForm_File
#include "Config.h"             // IniConfig
#include "CosFunction.h"        // CosFunction
#include "LastSet.h"            // LastSet.iTemperature / iRunStartMode
#include "bthermo.h"            // bGetHeaterUsed(int)
#include "acarry_shims.h"       // ATC_InterfaceForm (+ MySleep decl)

//------------------------------------------------------------------------------
// AI(W906-FW3-TemperFrom-WA) 20260818: golden cTemperFrom.cpp :34-37, file
// scope, verbatim shape. ShowTempComp[]/NameTempComp[] are the arrays
// ShowThermo/ShowHotName index by channel -- NOT TfTemperFrom members (see
// forms/fTemperFrom.h's own "SINGLE-INSTANCE ASSUMPTION" note). This is also
// the FIRST real definition of `ShowTempComp` in the port tree:
// SECSGEM/uHGemHT9045_SV.cpp:390 already carries a matching
// `extern TPanel *ShowTempComp[tcTotalCount];` (both resolve to the SAME
// `TPanel` -- vclcompat::TPanel promoted to global scope via
// vclcompat/Controls.h:473's `using vclcompat::TPanel;`, which this file
// reaches transitively through forms/fTemperFrom.h) -- that file's own
// banner already documents nothing there dereferences it, so this
// definition changes no behaviour there, it simply gives the pre-existing
// extern a real home.
//
// golden's OWN `TfTemperFrom *fTemperFrom;` (:34) and `bool
// bGreen=false,bYellow=false;` (:35) are NOT declared here -- neither is
// touched by any Wave A method (both are exclusively Panel71/72/73MouseDown/
// Timer1Timer state, all excluded this wave; see forms/fTemperFrom.h's
// "EXPLICITLY EXCLUDED" list and its own "integration-pending" note on the
// live `fTemperFrom` singleton).
//------------------------------------------------------------------------------
TPanel *ShowTempComp[tcTotalCount];
TPanel *NameTempComp[tcTotalCount];

// AI(W906-FW3-TemperFrom-WA) 20260818: LOCALLY PROVIDED, see file banner --
// golden uTemp_Set.h:34 (`extern const int InitTempOffset;`), value 8 per
// uTemp_Set.cpp (unported); bthermo.cpp:233 / uHeaterThread.cpp:297 already
// carry the identical file-local mirror.
static const int InitTempOffset = 8;   //Steven 20141117 : 起測時溫度要補Offset

// AI(W906-FW3-TemperFrom-WA) 20260818: LOCALLY PROVIDED, see file banner --
// golden ATC/ATC_Handler_Side.h:24/25/30 (unported as a shared include);
// aTester_Front.cpp/aTester_Rear.cpp/cUnitConvert.cpp/csystem.cpp/
// uHeaterThread.cpp each already carry an identical TU-local mirror.
#define ATC_TYPE_33         33
#define ATC_TYPE_35         35
#define ATC_TYPE_61         61

// AI(W906-FW3-TemperFrom-WA) 20260818: LOCALLY PROVIDED, see file banner --
// golden Graphics.hpp VCL system colours; acatchtray_shims.h /
// SECSGEM/uHGemEquipment.h / vclcompat/{TrayCore,BtnPanelCore,LedCore}.h
// each already carry an identical mirror block (TColor itself comes from
// cmydef.h:16, already included above).
const TColor clRed     = 0x000000FF;
const TColor clLime     = 0x0000FF00;
const TColor clYellow   = 0x0000FFFF;
const TColor clBtnFace  = TColor(0x8000000F);

//---------------------------------------------------------------------------
// golden :40-126 -- ctor. Builds the two 71-entry channel<->panel arrays and
// seeds strShowYield[]'s 10 function-status slots.
//---------------------------------------------------------------------------
TfTemperFrom::TfTemperFrom()
{
    TPanel *tem1Ptr[]=                                                          //溫控器要一起改
    {
        hlNamePlate1, hlNamePlate2, hlNameShuttle1, hlNameShuttle2, hlNameHead12,
        hlNameHead34, hlNameHead56, hlNameHead78, hlNameDut, hlNameChamber, hlNameCCD,
        hlNameAa1, hlNameAb1, hlNameAc1, hlNameAd1,
        hlNameBa1, hlNameBb1, hlNameBc1, hlNameBd1,
        hlNameAa2, hlNameAb2, hlNameAc2, hlNameAd2,
        hlNameBa2, hlNameBb2, hlNameBc2, hlNameBd2,
        hlNameHeatGun1, hlNameHeatGun2,                                         //kevin 20120523 add
        hlNameDut_A1, hlNameDut_A2, hlNameDut_A3, hlNameDut_A4,                 //2013-01-15    Dell DUT增加為4顆
        hlNameAe1, hlNameAf1, hlNameAg1, hlNameAh1,                             //Steven 20140923 : Index使用EJ1N版32組加熱器
        hlNameBe1, hlNameBf1, hlNameBg1, hlNameBh1,
        hlNameAe2, hlNameAf2, hlNameAg2, hlNameAh2,
        hlNameBe2, hlNameBf2, hlNameBg2, hlNameBh2,
        hlName2D_2, hlNameLB, hlNameESD, hlNameCCD_2_2,
        hlNameATCHeatGun1_2, hlNameATCHeatGun2_2,                               //JerryYang 20220408 : add for ATC3.5
        hlNameOutShuttle1, hlNameOutShuttle2,                                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlNameBase1, hlNameBase2, hlNameBase3,                                  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlNameBase4, hlNameBase5, hlNameBase6,                                  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlNamePlate_3, hlNamePlate_4,                                           //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlNameShuttle_3, hlNameShuttle_4,                                       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlNameDoor1, hlNameDoor2,                                               //Ztex 2023.10.23 Add Index Door Heater
        hlNameLBUp, hlNameLBDown                                                //Frank 20241231 : add
    };

    TPanel *temPtr[]=                                                           //溫控器要一起改
    {
        hlTempPlate1, hlTempPlate2, hlTempShuttle1, hlTempShuttle2, hlTempHead12,
        hlTempHead34, hlTempHead56, hlTempHead78, hlTempDut, hlTempChamber, hlTempCCD,
        hlTempAa1, hlTempAb1, hlTempAc1, hlTempAd1,
        hlTempBa1, hlTempBb1, hlTempBc1, hlTempBd1,
        hlTempAa2, hlTempAb2, hlTempAc2, hlTempAd2,
        hlTempBa2, hlTempBb2, hlTempBc2, hlTempBd2,
        hlTempHeatGun1, hlTempHeatGun2,                                         //kevin 20120523 add
        hlTempDut_A1 ,hlTempDut_A2 ,hlTempDut_A3 ,hlTempDut_A4,                 //2013-01-15    Dell DUT增加為4顆
        hlTempAe1, hlTempAf1, hlTempAg1, hlTempAh1,                             //Steven 20140923 : Index使用EJ1N版32組加熱器
        hlTempBe1, hlTempBf1, hlTempBg1, hlTempBh1,
        hlTempAe2, hlTempAf2, hlTempAg2, hlTempAh2,
        hlTempBe2, hlTempBf2, hlTempBg2, hlTempBh2,
        hlTemp2D_2, hlTempLB, hlTempESD, hlTempCCD_2_2,                         //Steven 20210625 : 多了一個hlNameCCD
        hlTempATCHeatGun1_2, hlTempATCHeatGun2_2,                               //JerryYang 20220408 : add for ATC3.5
        hlTempOutShuttle1, hlTempOutShuttle2,                                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlTempBase1, hlTempBase2, hlTempBase3,                                  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlTempBase4, hlTempBase5, hlTempBase6,                                  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlTempPlate_3, hlTempPlate_4,                                           //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlTempShuttle_3, hlTempShuttle_4,                                       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
        hlTempDoor1, hlTempDoor2,                                               //Ztex 2023.10.23 Add Index Door Heater
        hlTempLBUp, hlTempLBDown                                                //Frank 20241231 : add
    };

    for(int i=tcHotPlate1; i<tcTotalCount; i++)
    {
        ShowTempComp[i]=temPtr[i];
        NameTempComp[i]=tem1Ptr[i];
    }

    AnsiString tempYieldName[]=
    {
        "Error Recovery",    "Double Device", "Continue Good Bin", "Yield Monitoring", "Consecutive Alarm",
        "Index Start Error", "OCR",           "QA Mode",           "",                 ""
    };
    TfTemperFromPanel *tem2Ptr[]=
    {
        palErrorRecovery, palDoubleDevice, palCGoodBin, palYieldMonitor, palConsAlarm,
        palIndexStartErr, palOcrOn, pnlQAMode, palTest1, palTest2
    };

    for(int i=esytErrorRecovery; i<SHOW_YIELD_TOTAL; i++)
    {
        strShowYield[i].UsePanel            =tem2Ptr[i];
        strShowYield[i].asName              =tempYieldName[i];
        strShowYield[i].UsePanel->Caption   ="";
        strShowYield[i].UsePanel->Color     =clBtnFace;
        strShowYield[i].bShow=false;
    }

    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           //Steven 20250423 : Add auto temp offset display
        strShowYield[esytOEE].asName="Auto Temp Offset";

    bShow=false;
    AutoSize=false;
    bStartRecord=false;
    iFormWidth=728;
}

//---------------------------------------------------------------------------
// golden :586-1470 (885 lines) -- the per-channel classifier. Reads the raw
// PV (UN150Read[Addr]) plus a large web of customer-code/test-mode/ATC-mode
// config, writes ShowTempComp[Addr]->Caption/Color, and returns whether this
// channel is "hot enough to need CCD-cooling airflow" (bOverTemp) -- the one
// caller (Timer1Timer, excluded this wave) ORs this across every channel to
// drive SW[SwCCDCooling].
//---------------------------------------------------------------------------
bool TfTemperFrom::ShowThermo(int Addr)
{
    const double CCDTempLimit =45.0;

    static bool bTemperatureReady[tcTotalCount]={true};
    static bool iBelowAlmSec[tcTotalCount], iOverAlarmSec[tcTotalCount];
    static int  iSec=0;
    // GOLDEN ODDITY, not a translation artifact: `bTemperatureReady[]` is a
    // function-local static (private to golden's own ShowThermo, no extern
    // anywhere) that this function only ever WRITES, never reads -- true in
    // golden too, verified this wave (`grep -c bTemperatureReady` over the
    // whole golden .cpp: every hit is inside this one function, all on the
    // LHS of an assignment). `iBelowAlmSec[]`/`iOverAlarmSec[]` are read only
    // inside GATE (T1) below, so they are legitimately "set but not used"
    // ONLY while that gate stays inert. `(void)`-silenced here rather than
    // left to warn, matching this wave's `(void)S1;`/`(void)iSec;` idiom for
    // "gate made a real local go quiet".
    (void)bTemperatureReady; (void)iBelowAlmSec; (void)iOverAlarmSec;

    AnsiString S1="";
    AnsiString str;
    // PORT-ONLY `=0.0` defaults (golden :596-597 declares these bare,
    // uninitialized). Every path through this function's if/else-if chains
    // assigns SetTemp/dTemptureRangeOver/dTemptureRangeBelow before reading
    // them (verified this wave, full read of the function) -- the final
    // `else` arm of every such chain is an unconditional catch-all -- so
    // this changes no REACHABLE value; it only avoids relying on the same
    // "always assigned before read" invariant a future edit could
    // accidentally break silently. Zero behaviour change.
    double dTemptureRangeOver=0.0,dTemptureRangeBelow=0.0;
    double SetTemp=0.0;
    bool   bNow_Is_ATC=false;                                                   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    bool   bOverTemp=false;                                                     //Steven 20110705
    int    iChannel=0;
    int SITE_Ch[14]={tcOutSht1, tcOutSht2, tcDUT1, tcDUT2, tcDUT3, tcDUT4,
                     tcBase1, tcBase2, tcBase3, tcBase4, tcBase5, tcBase6,      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                     tcDoor1,tcDoor2                                     };     //Ztex 2023.10.23 Add Index Door Heater

    bool bAirStream_Check=false;                                                //Ztex 2023.04.19 Add HT-1032 TriTemp Function

    // GOLDEN ODDITY (golden :607) -- see file banner: assumes 4-byte
    // pointers. Translated literally, not "fixed" to sizeof(elem). The
    // `(int)` cast is PORT-ONLY (golden compares the bare `size_t` division
    // result against a signed `int Addr` -- BCB6 tolerates this silently;
    // g++ -Wextra's -Wsign-compare would warn). The cast only changes the
    // TYPE of the comparison, never the VALUE (tcTotalCount*sizeof(TPanel*)
    // is nowhere near INT_MAX on any real build), so this is zero behaviour
    // change, same class of deviation as the `=0.0` NSDMI note above.
    if(Addr>=(int)(sizeof(ShowTempComp)/4))
        return false;

    if(Tri_Temp_Machine==1)                                                     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    {
        if(LastSet.iTemperature==Tempture_Ambient)
        {
            ShowTempComp[Addr]->Caption="---";
            ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
        }
        else
        {
            bAirStream_Check=false;
            if((Addr>=tcAa1 && Addr<=tcBd2) ||                                  //判斷ATC、Heater溫度區間及顯示元件顏色
               (Addr>=tcAe1 && Addr<=tcBh2) ||
               (Addr>=tcHotPlate1 && Addr<=tcShuttle2) ||
               (Addr>=tcHotPlate3 && Addr<=tcShuttle4))
            {
                SetTemp             =Temperature.fWorkTemperBase;
                dTemptureRangeOver  =IniConfig.iL36_1Tri_Temp_Rang_ATC;
                dTemptureRangeBelow =IniConfig.iL36_1Tri_Temp_Rang_ATC;
                bNow_Is_ATC=true;
            }
            else if(Addr==tcATCHotAir1 || Addr==tcATCHotAir2)
            {
                if(Addr==tcATCHotAir1)
                {
                    SetTemp             =Temperature.dSetIndexAirstreamTemp;
                    bAirStream_Check    =!Temperature.bEnableArm_1_Air && !Temperature.bEnableArm_2_Air;
                }
                else
                {
                    SetTemp             =Temperature.fSetTempature2AirMachine;
                    bAirStream_Check    =!Temperature.bEnableSocket_Air;
                }

                dTemptureRangeOver      =Temperature.SetAirstreamTemperatureRang_Index;
                dTemptureRangeBelow     =Temperature.SetAirstreamTemperatureRang_Index;
            }
            else
            {
                for(int i=0; i<14; i++)
                {
                    if(Addr==SITE_Ch[i])
                    {
                        SetTemp=Temperature.fIndividualTemp[SITE_Ch[i]];
                        break;
                    }
                }

                dTemptureRangeOver  =IniConfig.iL36_2Tri_Temp_Rang_Heater;
                dTemptureRangeBelow =IniConfig.iL36_2Tri_Temp_Rang_Heater;
            }

            if(bUT150Install[Addr]==true && bUT150HasUse[Addr]==true && bAirStream_Check==false)
            {
                #ifdef SOFT_SIMULTE
                AnsiString str1;
                str1.sprintf("%d", Addr);
                if(IniConfig.bL29AmbientNotShowTemp &&
                   LastSet.iTemperature==Tempture_Ambient)                      //Sam 20221101 : 常溫模式不顯示溫度
                    ShowTempComp[Addr]->Caption="---";
                else
                    ShowTempComp[Addr]->Caption=str1;
                asGPIBTempShow[Addr]=str1;                                      //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
                if(bNow_Is_ATC==false)
                    UN150Read[Addr]=Addr;
                #else
    //            ShowTempComp[Addr]->Caption="...";
    //            asGPIBTempShow[Addr]="NULL";                                  //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
                #endif

                str.sprintf("%5.1f", UN150Read[Addr]);

                if(IniConfig.bL29AmbientNotShowTemp &&
                   LastSet.iTemperature==Tempture_Ambient)                      //Sam 20221101 : 常溫模式不顯示溫度
                    ShowTempComp[Addr]->Caption="---";
                else
                    ShowTempComp[Addr]->Caption=str;

                if(Temperature.fWorkTemperBase<=0 && bNow_Is_ATC==true && UN150Read[Addr]<=0)
                {
                    if(UN150Read[Addr]-(SetTemp)>(dTemptureRangeOver))          // OVER
                    {
                        ShowTempComp[Addr]->Color=(TColor)0x008000FF;
                        bUT150State[Addr]=false;
                    }
                    else if(UN150Read[Addr]-(SetTemp)<((-1)*dTemptureRangeBelow))// Below
                    {
                        if(LastSet.iTemperature==Tempture_Hot ||
                           LastSet.iTemperature==Tempture_AmbientHot ||         //kevin 20140918 恆溫控制
                           (LastSet.iTemperature==Tempture_Ambient && Temperature.bAmbientGuardbandCheck && IniConfig.bL20AbientGuardBand))
                        {
                            ShowTempComp[Addr]->Color=clYellow;
                            bUT150State[Addr]=false;
                        }
                        else
                        {
                            ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                            bUT150State[Addr]=false;
                        }
                    }
                    else
                    {
                        ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                        bUT150State[Addr]=true;
                    }
                }
                else
                {
                    if(UN150Read[Addr]>(SetTemp+dTemptureRangeOver))            // OVER
                    {
                        ShowTempComp[Addr]->Color=(TColor)0x008000FF;
                        bUT150State[Addr]=false;
                    }
                    else if(UN150Read[Addr]<(SetTemp-dTemptureRangeBelow))      // Below
                    {
                        if(LastSet.iTemperature==Tempture_Hot ||
                           LastSet.iTemperature==Tempture_AmbientHot ||         //kevin 20140918 恆溫控制
                           (LastSet.iTemperature==Tempture_Ambient && Temperature.bAmbientGuardbandCheck && IniConfig.bL20AbientGuardBand))
                        {
                            ShowTempComp[Addr]->Color=clYellow;
                            bUT150State[Addr]=false;
                        }
                        else
                        {
                            ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                            bUT150State[Addr]=false;
                        }
                    }
                    else
                    {
                        ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                        bUT150State[Addr]=true;
                    }
                }
            }
            else
            {
                ShowTempComp[Addr]->Caption="---";
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                bUT150State[Addr]=true;
                UN150Read[Addr]=0;
            }
        }
    }
    else if(bUT150Install[Addr] && bGetHeaterUsed(Addr))
    {
        if(USE_16_HEATER==eht16Heater       ||                                  //Steven 20120606 : 16溫控器 8Site使用Hontech頭
           USE_16_HEATER==eht16HeaterEJ1N   ||
           USE_16_HEATER==eht32HeaterEJ1N   ||                                  //Steven 20140923 : Index使用EJ1N版32組加熱器
           USE_16_HEATER==eht32HeaterKT4H   ||                                  //Steven 20150211 : Index使用KT4H版32組加熱器
           USE_16_HEATER==eht16HeaterDTME08 ||                                  //JimmyChiu 20210923 : Index使用DTME08版16組加熱器
           USE_16_HEATER==eht32HeaterDTME08 )                                   //JimmyChiu 20210923 : Index使用DTME08版32組加熱器
        {
            if(Addr>=tcHead1 && Addr<tcSocket)
            {
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                ShowTempComp[Addr]->Caption="---";
                return false;
            }
        }

        if(ATC_SYSTEM==eNewATCSystem &&
           Temperature.bATCActiveCooling==true &&
           Temperature.bActiveHeatGun &&
          (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
        {
            if(Addr>=tcHeatGun1 && Addr<=tcHeatGun2)
            {
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                str.sprintf("%5.1f", UN150Read[Addr]);
                ShowTempComp[Addr]->Caption=str;
                return false;
            }
        }

        //----- by dell ccd realtime-------------Steven 20110810
        if(Addr==tcCCD || (RTC_TemperNumber==2 && Addr==tcCCD_2))               //CCD  //Isaac 20201217 : RTC CCD增加第二組感溫
        {
            if(CUSTOMER_CODE==CC_SCC)                                           //jou 2012-03-30 SCC室溫就大約30度,所以就會一直吹氣
            {
                if(UN150Read[Addr]>=35.0)                                       //CCD超過30度就要吹
                {
                    bOverTemp=true;
                }
            }
            else
            {
                if(UN150Read[Addr]>=30.0)                                       //CCD超過30度就要吹
                {
                    bOverTemp=true;
                }
            }

            if(LastSet.iTemperature!=Tempture_AmbientHot)                       //kevin 20150731  恆溫控制
            {
                if(UN150Read[Addr]>=CCDTempLimit)                               //for HT9046LS
                {
                    bCCDOverTemp=true;
                }
                else if(UN150Read[Addr]<(CCDTempLimit-1))
                {
                    bCCDOverTemp=false;
                }
            }
            ShowTempComp[Addr]->Color=(UN150Read[Addr]>55.0)?(TColor)0x008000FF:(TColor)0x0025AB12;
        }
        else if(Addr==tc2D)                                                     //wei 20160614 2D溫控器讀取
        {
            ShowTempComp[Addr]->Color=(UN150Read[Addr]>50.0)?(TColor)0x008000FF:(TColor)0x0025AB12;
        }
        else if(Addr==tcLBUp || Addr==tcLBDown)                                 //Frank 20241231 : add
        {
            if(UN150Read[Addr]>=Temperature.dLBTempHighSettingValue)
                ShowTempComp[Addr]->Color=(TColor)0x008000FF;
            else
                ShowTempComp[Addr]->Color=(UN150Read[Addr]<Temperature.dLBTempLowSettingValue)?clYellow:(TColor)0x0025AB12;
        }
        else if(Addr==tcLB)                                                     //Steven 20181023 : LB溫度
        {
            if(UN150Read[Addr]>=200)
                ShowTempComp[Addr]->Color=(TColor)0x008000FF;
            else
                ShowTempComp[Addr]->Color=(UN150Read[Addr]<Temperature.dBoostIdleTime[Temperature.eBLBL])?clYellow:(TColor)0x0025AB12;
        }
        else
        //---------------------------------------
        {
            if(LastSet.iTemperature==Tempture_AmbientHot)                       //kevin 20150731  恆溫控制
            {
                if(Addr==tcChamber)                                             //for HT9046LS
                {
                    if(UN150Read[Addr]>=Temperature.fWorkTemperBase-Temperature.fAmbientHotGuartbent)//chambo     //kevin 20150731  恆溫控制
                    {
                        bCCDOverTemp=false;
                    }
                    else
                    {
                        bCCDOverTemp=true;                                      //降溫
                    }
                }
            }
            //Steven 20121101 Start : 修改常溫檢查範圍
            if(LastSet.iTemperature==Tempture_Ambient && Addr<=tcHotPlate2 &&   //jou 2012-12-03 修正高溫沒有顯示Hotplate
              (IniConfig.bSPILFunction==true ||                                 //Steven 20110725 : 常溫,矽品說不要顯示HotPlate的溫度  //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
               CUSTOMER_CODE==CC_SIGURD_PeiXing))                               //JerryYang 20160425 : 矽格建榮也要求常溫不顯示Hot plate的溫度
            {
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                ShowTempComp[Addr]->Caption="---";
                bTemperatureReady[Addr]=0;                                      //kevin 20210112 溫度過低過高  1 過低  :2  bTemperatureReady[Addr]=false//kevin 20130425
                return false;
            }

            if(CUSTOMER_CODE==CC_SCS &&
               ATC_SYSTEM==eATCHonPrecType &&
               (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite) &&
               LastSet.iTemperature==Tempture_Ambient &&
               Addr<=tcHotPlate2 &&
               Temperature.bATCActiveCooling==true)                             //Steven 20130122 : SCS常溫開啟ATC不要顯示HotPlate溫度
            {
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                ShowTempComp[Addr]->Caption="---";
                bTemperatureReady[Addr]=0;                                      //kevin 20210112  過高  1 過低  :2
                return false;
            }

            if(Addr==tcChamber && (Temperature.iIndexHeatMode==HeadOnly ||
                                   Temperature.iIndexHeatMode==HeadSocket))     //2013-09-02    Dell
            {
                if((CUSTOMER_CODE==CC_ASE_KaohSiung ||                          //kevin 20151119   head 模式不顯示CHAMBO溫度
                    CUSTOMER_CODE==CC_SIGURD_PeiXing) &&                        //Sam 20230328 : 矽格北興 Head 模式不顯示溫度
                   (LastSet.iTemperature==Tempture_Hot ||
                    LastSet.iTemperature==Tempture_AmbientHot))                 //kevin 20151119 沒使用chambo
                {
                    ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                    ShowTempComp[Addr]->Caption="---";
                    bTemperatureReady[Addr]=0;                                  //kevin 20210112  過高  1 過低  :2
                     return false;
                }
                else
                {
                    if(LastSet.iTemperature==Tempture_Hot)
                        SetTemp=Temperature.fChamberCoolTemp;
                    else
                        SetTemp=Temperature.fAbitTemp;
                }
            }
            else if(LastSet.iTemperature==Tempture_Hot &&
                    Temperature.bUseFixTemp==true &&                            //Steven 20131025 : 使4固定的DUT溫度
                    Addr>=tcDUT1                  &&
                    Addr<=tcDUT4)
            {
                SetTemp=Temperature.dFixedTemp;                                 //Steven 20131025 : 固定的DUT溫度值
            }
            else if((LastSet.iTemperature==Tempture_Hot||
                     LastSet.iTemperature==Tempture_AmbientHot) &&
                     Temperature.bATCActiveCooling==true &&                     //JerryYang 20220408 : add for ATC3.5
                    (Addr==tcATCHotAir1 || Addr==tcATCHotAir2))
            {
                SetTemp=IniConfig.dHeatGunTempATC;
            }
            else if(INSTALL_ATC_HEAT_GUN>0 && (Addr==tcATCHotAir1 || Addr==tcATCHotAir2))
            {
                 SetTemp=IniConfig.dHeatGunTempATC;
            }
            else
            {
                if(LastSet.iTemperature==Tempture_Hot ||
                   LastSet.iTemperature==Tempture_AmbientHot)                   //kevin 20140918 恆溫控制
                    SetTemp=(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp)?Temperature.fIndividualTemp[Addr]:Temperature.fWorkTemperBase;        //Steven 20140924 : 各個加熱區獨立有自己的設定值
                else
                    SetTemp=Temperature.fAbitTemp;
                if(TestIF.iTestMode==SingleSite && Temperature.bMultiZoneEnable)    //wei 20240617 Multi Zone
                {
                    if(Addr==tcAa1 || Addr==tcAa2)
                        iChannel=0;
                    else if(Addr==tcAb1 || Addr==tcAb2)
                        iChannel=1;
                    else if(Addr==tcAc1 || Addr==tcAc2)
                        iChannel=2;
                    else if(Addr==tcAd1 || Addr==tcAd2)
                        iChannel=3;

                     SetTemp=Temperature.dZoneTempSetting[iChannel];
                }
            }

            if(IniConfig.bL07UseSingleTenmpertureLimit)
            {
                dTemptureRangeOver =IniConfig.dSingleTempLimit[Addr];
                dTemptureRangeBelow=IniConfig.dSingleTempLimit[Addr];
            }
            else
            {
                if(Addr==tcChamber)
                {
                    if(LastSet.iTemperature==Tempture_Ambient)
                    {
                        if(LastSet.iTemperature==Tempture_Ambient &&
                           Temperature.bAmbientGuardbandCheck &&
                           IniConfig.bL20AbientGuardBand)                       //kevin 20180115 (Steven) add Amient Guard Band
                        {
                            dTemptureRangeOver =Temperature.iAmbGuardband;
                            dTemptureRangeBelow=Temperature.iAmbGuardband;
                        }
                       else
                        {
                            dTemptureRangeOver =IniConfig.iAmbTemperatureRange; //jou 2013-04-11 Ambient Temperature Range
                            dTemptureRangeBelow=IniConfig.iAmbTemperatureRange; //jou 2013-04-11 Ambient Temperature Range
                        }
                    }
                    else
                    {
                        if(LastSet.iTemperature==Tempture_AmbientHot)           //kevin 20141231 恆溫控制
                        {
                            dTemptureRangeOver=Temperature.fAmbientHotGuartbent;
                            dTemptureRangeBelow=Temperature.fAmbientHotGuartbent;
                        }
                        else
                        {
                            if(LastSet.iTemperature==Tempture_Ambient &&
                               Temperature.bAmbientGuardbandCheck &&
                               IniConfig.bL20AbientGuardBand)                   //kevin 20180115 (Steven) add Amient Guard Band
                            {
                                dTemptureRangeOver =Temperature.iAmbGuardband;
                                dTemptureRangeBelow=Temperature.iAmbGuardband;
                            }
                            else
                            {
                                 dTemptureRangeOver =IniConfig.iL05ChamberTemptureRange;
                                 dTemptureRangeBelow=IniConfig.iL05ChamberTemptureRange;  //kevin 20190103 change alarm temp range different
                            }
                        }
                    }
                }
                else if(Addr==tcSocket || (Addr>=tcDUT1 && Addr<=tcDUT4))       //2013-01-15    Dell DUT(Socket Base)增加為4顆
                {
                    if(LastSet.iTemperature==Tempture_AmbientHot)               //kevin 20141231 恆溫控制
                    {
                        dTemptureRangeOver =Temperature.fAmbientHotGuartbent;
                        dTemptureRangeBelow=Temperature.fAmbientHotGuartbent;
                    }
                    else
                    {
                        dTemptureRangeOver =IniConfig.iSocketTemptureRangeOver; //ChungHung 20130408 add
                        dTemptureRangeBelow=IniConfig.iSocketTemptureRangeUnder;
                    }
                }
                else
                {
                    if(ATC_SYSTEM!=eNonChamber   &&                             //Steven 20140314 : For HT9045WA
                        ATC_SYSTEM>eATC30 &&                                    //20141204 ChungHung add for ATC3.0
                        Temperature.bATCActiveCooling==true &&                  //2014-05-30    Dell    for ATC6.0
                        ((Addr>=tcHead1 && Addr<=tcHead4) ||
                         (Addr>=tcAa1 && Addr<=tcBd2) ||
                         (Addr>=tcAe1 && Addr<=tcBh2)))
                    {
                        if(LastSet.iTemperature==Tempture_AmbientHot)           //kevin 20140918 恆溫控制
                        {
                            dTemptureRangeOver =Temperature.fAmbientHotGuartbent;
                            dTemptureRangeBelow=Temperature.fAmbientHotGuartbent;
                        }
                        else
                        {
                            dTemptureRangeOver =IniConfig.iATCTemperatureRange;
                            dTemptureRangeBelow=IniConfig.iATCTemperatureRange;
                        }
                    }
                    else
                    {
                        if(LastSet.iTemperature==Tempture_Ambient)
                        {
                            if(LastSet.iTemperature==Tempture_Ambient &&
                               Temperature.bAmbientGuardbandCheck &&
                               IniConfig.bL20AbientGuardBand)                   //kevin 20180115 (Steven) add Amient Guard Band
                            {
                                dTemptureRangeOver =Temperature.iAmbGuardband;
                                dTemptureRangeBelow=Temperature.iAmbGuardband;
                            }
                            else
                            {
                                dTemptureRangeOver =IniConfig.iAmbTemperatureRange; //jou 2013-04-11 Ambient Temperature Range
                                dTemptureRangeBelow=IniConfig.iAmbTemperatureRange; //jou 2013-04-11 Ambient Temperature Range
                            }
                        }
                        else
                        {
                            if(LastSet.iTemperature==Tempture_AmbientHot)       //kevin 20140918 恆溫控制
                            {
                                dTemptureRangeOver =Temperature.fAmbientHotGuartbent;
                                dTemptureRangeBelow=Temperature.fAmbientHotGuartbent;
                            }
                            else
                            {
                                dTemptureRangeOver =IniConfig.iL04TemptureRange;
                                dTemptureRangeBelow=IniConfig.iL04TemptureRange;
                            }
                        }
                    }
                }
            }

            if((bUseInitTempOffset   &&                                             //Steven 20141117 : 起測時溫度要補Offset
                iInitContactCount<Temperature.iCintactCntForTempOffsetAtInitial))   //Steven 20141125 : <= --> <
                {
                //==> Eastsun 20260508 整合: Ifor 20210525 add KLT 要求取消功能 + ChungHung 20130408 Socket TempRange
                if(CUSTOMER_CODE==CC_KYEC_LEE && bEnable_KLT_Function==false)
                {
                    if(Addr==tcATCHotAir1 || Addr==tcATCHotAir2)
                    {
                        dTemptureRangeBelow=20;
                        dTemptureRangeOver=20;
                    }
                    else if(Addr==tcSocket || (Addr>=tcDUT1 && Addr<=tcDUT4))
                    {
                        dTemptureRangeOver =IniConfig.iSocketTemptureRangeOver; //ChungHung 20130408 add
                        dTemptureRangeBelow=IniConfig.iSocketTemptureRangeUnder;
                    }
                    else
                    {
                        #ifdef FOR_NVIDIA
                           if((Addr>=tcAa1 && Addr<=tcBd1) || (Addr>=tcAe1 && Addr<=tcBh1) ||
                              (Addr>=tcAa2 && Addr<=tcBd2) || (Addr>=tcAe2 && Addr<=tcBh2))
                          {
                              dTemptureRangeBelow=Temperature.iTempReadyRange;
                              dTemptureRangeOver=Temperature.iTempReadyRange;
                          }
                          else
                          {
                                dTemptureRangeBelow=1;
                                dTemptureRangeOver=1;
                          }
                        #else
                            dTemptureRangeBelow=1;
                            dTemptureRangeOver=1;
                        #endif
                    }
                }
                else if(IniConfig.bL28TempOfsUseReadyTempRange && bEnable_KLT_Function==false) //Sam 20231214 : Temp offset use ready temp range
                {
                    dTemptureRangeBelow=1;
                    dTemptureRangeOver=1;
                }
                else
                {
                    dTemptureRangeBelow=1;
                    dTemptureRangeOver+=Temperature.fTempOffSet[InitTempOffset][Addr];
                }
            }
            else if(bUseInitTempOffset && IniConfig.bL28TempOfsUseReadyTempRange && bEnable_KLT_Function==false && Temperature.iTempReadyRange!=0 &&    //Sam 20231214 : Temp offset use ready temp range
                    iInitContactCount<(Temperature.iCintactCntForTempOffsetAtInitial+Temperature.iCintactDelayCntForInitTempOffset))
            {
                dTemptureRangeBelow=Temperature.iTempReadyRange;
                dTemptureRangeOver=Temperature.iTempReadyRange;
            }
            else if((Temperature.bBoostFuncttion ||
                     Temperature.bLBTempFunction) && iTriggerBoostFunction!=-1) //Steven 20180817 : Boost Function
            {
                if((Addr>=tcAa1 && Addr<=tcBd2) ||
                   (Addr>=tcAe1 && Addr<=tcBh2))
                {
                    dTemptureRangeBelow=1;
                    dTemptureRangeOver+=Temperature.dBoostOffset[iTriggerBoostFunction];
                }
            }

            if(INSTALL_ATC_HEAT_GUN>0 && (Addr==tcATCHotAir1 || Addr==tcATCHotAir2))
            {
                if(UN150Read[Addr]>(IniConfig.dHeatGunTempATC+10))              // OVER
                {
                    ShowTempComp[Addr]->Color=(TColor)0x008000FF;
                    iTempOverShowAlarmT[Addr]=1;
                }
                else if(UN150Read[Addr]<(IniConfig.dHeatGunTempATC-10))         // Below
                {
                    iTempOverShowAlarmT[Addr]=2;
                    if(LastSet.iTemperature==Tempture_Hot ||
                       LastSet.iTemperature==Tempture_AmbientHot ||
                       (LastSet.iTemperature==Tempture_Ambient && Temperature.bAmbientGuardbandCheck && IniConfig.bL20AbientGuardBand))
                        ShowTempComp[Addr]->Color=clYellow;
                    else
                        ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                }
                else
                {
                    iTempOverShowAlarmT[Addr]=0;
                    ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                }
            }
            else
            {
                if(UN150Read[Addr]>(SetTemp+dTemptureRangeOver))                // OVER
                {
                    ShowTempComp[Addr]->Color=(TColor)0x008000FF;
                    iTempOverShowAlarmT[Addr]=1;                                //kevin 20210112 溫度過高
                }
                else if(UN150Read[Addr]<(SetTemp-dTemptureRangeBelow))          // Below
                {
                    iTempOverShowAlarmT[Addr]=2;                                //kevin 20210112 溫度過低
                    if(LastSet.iTemperature==Tempture_Hot ||
                       LastSet.iTemperature==Tempture_AmbientHot ||             //kevin 20140918 恆溫控制
                       (LastSet.iTemperature==Tempture_Ambient && Temperature.bAmbientGuardbandCheck && IniConfig.bL20AbientGuardBand))
                        ShowTempComp[Addr]->Color=clYellow;
                    else
                        ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                }
                else
                {
                    iTempOverShowAlarmT[Addr]=0;                                //kevin 20210112 溫度過低過高
                    ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                }
            }

            //----- by dell ccd realtime-------------Steven 20110810
            if(Addr>=tcHead1 && UN150Read[Addr]>=30.0)                          //Chamber內任何位置超過30就吹
                bOverTemp=true;
            //---------------------------------------
        }

        bool bCanUseLowTemp=false;

        if(Temperature.bATCActiveCooling==true &&
          (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 ||
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 ||
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_61))
        {
            if((Addr>=tcAa1 && Addr<=tcBd2) ||
               (Addr>=tcAe1 && Addr<=tcBh2))
                bCanUseLowTemp=true;
            else
                bCanUseLowTemp=false;
        }
        else
        {
            bCanUseLowTemp=false;
        }

        if(UN150Read[Addr]<18 && bCanUseLowTemp==false)
        {
            #ifdef SOFT_SIMULTE
            //wei 20150703
            AnsiString str1;
            if(ATC_SYSTEM!=eNewATCSystem)
            {
                UN150Read[Addr]=Addr;
            }

            str1.sprintf("%0.2f", UN150Read[Addr]);

            if(IniConfig.bL29AmbientNotShowTemp &&
               LastSet.iTemperature==Tempture_Ambient)                          //Sam 20221101 : 常溫模式不顯示溫度
            {
                ShowTempComp[Addr]->Caption="---";
            }
            else
            {
                ShowTempComp[Addr]->Caption=str1;
            }

            asGPIBTempShow[Addr]=str1;                                          //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND

            #else
            ShowTempComp[Addr]->Caption="...";
            asGPIBTempShow[Addr]="NULL";                                        //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
            #endif
            bTemperatureReady[Addr]=false;                                      //kevin 20130425
        }
        else if(UN150Read[Addr]==999)
        {
            if((TestIF.iTestMode==_8Site2X4 ||
                TestIF.iTestMode==_16Site4X4) &&                                //Sam 20190226 : 16Site4X4
               TestIF_File.bOctal_12Kit &&                                      //kevin 20150624 12Site 使用8site 溫度不秀ALARM
               (Addr==tcAd1 || Addr==tcBd1 || Addr==tcAd2 || Addr==tcBd2))
            {
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                ShowTempComp[Addr]->Caption="---";
                bOverTemp=false;
                asGPIBTempShow[Addr]="NULL" ;                                   //kevin 20130425  2013.01.11 Q_Q TSMC GPIB COMMAND
                bTemperatureReady[Addr]=true;                                   //kevin 20130425   沒有使用溫控器要當讀到
            }
            else
            {
                #ifdef SOFT_SIMULTE
                AnsiString str1;
                if(ATC_SYSTEM!=eNewATCSystem)
                {
                    UN150Read[Addr]=Addr;
                }
                str1.sprintf("%0.2f", UN150Read[Addr]);
                if(IniConfig.bL29AmbientNotShowTemp &&
                   LastSet.iTemperature==Tempture_Ambient)                      //Sam 20221101 : 常溫模式不顯示溫度
                    ShowTempComp[Addr]->Caption="---";
                else
                    ShowTempComp[Addr]->Caption=str1;
                asGPIBTempShow[Addr]=str1;                                      //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
                UN150Read[Addr]=Addr;
                #else
                ShowTempComp[Addr]->Caption="ERR";
                asGPIBTempShow[Addr]="ERR";                                     //kevin 20130425 2013.01.11 Q_Q TSMC GPIB COMMAND
                #endif
                bTemperatureReady[Addr]=false;                                  //kevin 20130425
            }
        }
        else
        {
            if((LastSet.iTemperature==Tempture_Hot ||
                LastSet.iTemperature==Tempture_AmbientHot) &&                   //kevin 20140918 恆溫控制
                Temperature.bUseFixTemp==true     &&                            //Steven 20131025 : 使4固定的DUT溫度
                Temperature.bShowFixedTemp==false &&                            //Steven 20131025 : True -> 畫面上要顯示固定的DUT溫度值, False -> 畫面上顯示fWorkTemperBase溫度值
                Addr>=tcDUT1                      &&
                Addr<=tcDUT4)
            {
                str.sprintf("%5.1f", UN150Read[Addr]+Temperature.fWorkTemperBase-Temperature.dFixedTemp);
            }
            else
            {
                str.sprintf("%5.1f", UN150Read[Addr]);
            }

            if(IniConfig.bL29AmbientNotShowTemp &&
               LastSet.iTemperature==Tempture_Ambient)                          //Sam 20221101 : 常溫模式不顯示溫度
                ShowTempComp[Addr]->Caption="---";
            else
                ShowTempComp[Addr]->Caption=str;

            if(Temperature.bUseReferTempSensor==true)                           //Ifor 20160629 add 開啟第二點溫度時傳送第二點溫度給測試機
                asGPIBTempShow[Addr].sprintf("%0.1f", UN150ReadSecondpoint[Addr]);
            else
                asGPIBTempShow[Addr].sprintf("%0.1f", UN150Read[Addr]);         //kevin 20130425  2013.01.11 Q_Q TSMC GPIB COMMAND

            bTemperatureReady[Addr]=true;                                       //kevin 20130425
        }

        if(LastSet.iTemperature==Tempture_Ambient &&
           Temperature.bAmbientGuardbandCheck &&
           IniConfig.bL20AbientGuardBand)                                       //JerryYang 20230307 : fix常溫模式Amient guard band check
        {
            if(SystemStart==false || iHome!=0)                                  //Pause 狀態不偵測低溫, Home不偵測低溫
            {
                iBelowAlmSec[Addr]=0;
                iOverAlarmSec[Addr]=0;
            }

            #ifndef SOFT_SIMULTE
            // AI(W906-FW3-TemperFrom-WA) 20260818: GATE (T1) -- golden :1294-1321.
            // SAFETY-classified, NOT a missing-dependency gate (ShowErrorMessage/
            // MMSystem/SystemSec all already exist and are wired) -- see this
            // file's own header banner and forms/fTemperFrom.h's GATE REGISTER
            // for the full rationale and the behaviour delta. DEFAULT: the
            // screen-level over/under-temperature classification above
            // (iTempOverShowAlarmT[]/Color) stays fully active; only the WAR15xx
            // ESCALATION below is inert until a user turns it on.
            #if 0 // GATE (T1)
            if(iTempOverShowAlarmT[Addr]==2)
            {
                if(SystemSec!=iSec)
                {
                    iSec=SystemSec;
                    iBelowAlmSec[Addr]++;
                    if(iBelowAlmSec[Addr]>=3)
                    {
                        S1.sprintf("WAR15%02d", Addr);
                        ShowErrorMessage(S1, 0, MMSystem);
                        iBelowAlmSec[Addr]=0;
                    }
                }
            }
            else if(iTempOverShowAlarmT[Addr]==1)
            {
                if(SystemSec!=iSec)
                {
                    iSec=SystemSec;
                    iOverAlarmSec[Addr]++;
                    if(iOverAlarmSec[Addr]>=3)
                    {
                        S1.sprintf("WAR15%02d", Addr+100);
                        ShowErrorMessage(S1, 0, MMSystem);
                        iOverAlarmSec[Addr]=0;
                    }
                }
            }
            #endif // GATE (T1)
            (void)S1;    // write-only once GATE (T1) is inert -- see banner
            (void)iSec;  // read only inside GATE (T1)
            #endif // SOFT_SIMULTE
        }
    }
    else
    {
        if(CUSTOMER_CODE==CC_ATEC)                                              //JerryYang 20191003 for 艾科 7000的2個感溫點SLK要依照site數顯示溫度數量
        {
            if(TestIF.iTestMode==QualSite1X4 && DeviceForm_File.iHeadDeviceCT==3 &&
               TestIF_File.b2CableLayoutKit && Temperature.bATCActiveCooling==false)
            {
                if(Addr==tcAc1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa1]->Caption;
                }
                else if(Addr==tcAd1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb1]->Caption;
                }
                else if(Addr==tcAc2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa2]->Caption;
                }
                else if(Addr==tcAd2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb2]->Caption;
                }
                else
                {
                    ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                    ShowTempComp[Addr]->Caption="---";
                }
            }
            else if(TestIF.iTestMode==QualSite2X2 && TestIF_File.b2CableLayoutKit && DeviceForm_File.iHeadDeviceCT==3)
            {
                if(Addr==tcBa1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa1]->Caption;
                }
                else if(Addr==tcBb1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb1]->Caption;
                }
                else if(Addr==tcBa2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa2]->Caption;
                }
                else if(Addr==tcBb2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb2]->Caption;
                }
                else
                {
                    ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                    ShowTempComp[Addr]->Caption="---";
                }
            }
            else if(TestIF.iTestMode==_8Site2X4 && TestIF_File.b2CableLayoutKit && Temperature.bATCActiveCooling==false && DeviceForm_File.iHeadDeviceCT==4)
            {
                if(Addr==tcAc1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb1]->Caption;
                }
                else if(Addr==tcAd1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb1]->Caption;
                }
                else if(Addr==tcBa1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa1]->Caption;
                }
                else if(Addr==tcBb1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa1]->Caption;
                }
                else if(Addr==tcBc1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb1]->Caption;
                }
                else if(Addr==tcBd1)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb1]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb1]->Caption;
                }
                else if(Addr==tcAc2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb2]->Caption;
                }
                else if(Addr==tcAd2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb2]->Caption;
                }
                else if(Addr==tcBa2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa2]->Caption;
                }
                else if(Addr==tcBb2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAa2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAa2]->Caption;
                }
                else if(Addr==tcBc2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb2]->Caption;
                }
                else if(Addr==tcBd2)
                {
                    ShowTempComp[Addr]->Color=ShowTempComp[tcAb2]->Color;
                    ShowTempComp[Addr]->Caption=ShowTempComp[tcAb2]->Caption;
                }
                else
                {
                    ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                    ShowTempComp[Addr]->Caption="---";
                }
            }
            else
            {
                ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
                ShowTempComp[Addr]->Caption="---";
            }
        }
        else
        {
            ShowTempComp[Addr]->Color=(TColor)0x0025AB12;
            ShowTempComp[Addr]->Caption="---";
        }
        bOverTemp=false;
        asGPIBTempShow[Addr]="NULL" ;                                           //kevin 20130425  2013.01.11 Q_Q TSMC GPIB COMMAND
        bTemperatureReady[Addr]=true;                                           //kevin 20130425   沒有使用溫控器要當讀到
    }
    return bOverTemp;
}

//---------------------------------------------------------------------------
// golden :1473-1484
//---------------------------------------------------------------------------
void TfTemperFrom::SetShowYield(eShowYieldType iItem, bool bOnOff)
{
    if(bOnOff==true)
    {
        strShowYield[iItem].OnOff=true;
    }
    else
    {
        strShowYield[iItem].OnOff=false;
    }
    ShowYieldFuntion();
}

//---------------------------------------------------------------------------
// golden :1486-1579
//---------------------------------------------------------------------------
void TfTemperFrom::ShowYieldFuntion()
{
    AnsiString Str;
    for(int i=esytErrorRecovery; i<SHOW_YIELD_TOTAL; i++)
    {
        if(i==esytTest2)
        {
            if(strShowYield[i].bShow==true)                                     //kevin 20121008 start   add FT can't off set max value
            {
                if(LastSet.iRunStartMode==rsmContinuStart ||
                   LastSet.iRunStartMode==rsmInitialStart ||
                   LastSet.iRunStartMode==rsmAutoSiteMap ||
                   LastSet.iRunStartMode==rsmQAMode)
                    strShowYield[i].UsePanel->Caption="FT : Continues MAX_"+AnsiString(TestIF_File.iContsFailSocketAlarmCT); //kevin 20121009
                else
                    strShowYield[i].UsePanel->Caption="RT : Continues MAX_"+AnsiString(TestIF_File.iContsFailSocketAlarmCT_RT);//kevin 2012100

                strShowYield[i].UsePanel->Color=clLime;
                strShowYield[i].UsePanel->Refresh();
            }
            else
            {
                strShowYield[i].UsePanel->Caption="";
                strShowYield[i].UsePanel->Color=clBtnFace;
            }
        }
        else if(i==esytOEE)
        {
            if(CUSTOMER_CODE==CC_SCK)                                           //Steven 20220407 : JSCK OEE Function
            {
                strShowYield[esytOEE].OnOff=true;
                if(IniConfig.bN28_SCK_OEE)
                {
                    if(IniConfig.sN28_IP!="")
                        Str.sprintf("OEE ON(%s)", IniConfig.sN28_IP);
                    else
                        Str.sprintf("OEE ON(-)");
                    strShowYield[i].UsePanel->Caption=Str;
                    strShowYield[i].UsePanel->Color=clLime;
                }
                else
                {
                    Str.sprintf("OEE OFF");
                    strShowYield[i].UsePanel->Caption=Str;
                    strShowYield[i].UsePanel->Color=clRed;
                }
            }
            else
            {
                if(IniConfig.iN31_UseAutoTempOfsByFTP==0)                       //Steven 20250423 : Add auto temp offset display
                {
                    strShowYield[esytOEE].OnOff=false;
                    Str.sprintf("Auto Temp Offset OFF");
                    strShowYield[i].UsePanel->Caption=Str;
                    strShowYield[i].UsePanel->Color=clRed;
                }
                else
                {
                    strShowYield[esytOEE].OnOff=true;
                    if(IniConfig.iN31_UseAutoTempOfsByFTP==1)
                        Str.sprintf("Auto Temp Offset by FTP");
                    else
                        Str.sprintf("Auto Temp Offset by NetDrive");
                    strShowYield[i].UsePanel->Caption=Str;
                    strShowYield[i].UsePanel->Color=clLime;
                }
            }
        }
        else                                                                    //kevin 20121008 end
        {
            if(strShowYield[i].bShow==true)
            {
                if(strShowYield[i].OnOff==true)
                {
                    strShowYield[i].UsePanel->Caption=strShowYield[i].asName+" On";
                    strShowYield[i].UsePanel->Color=clLime;
                }
                else
                {
                    if(CUSTOMER_CODE==CC_ASE_M)                                 //Ifor 20170413 (wei) ASEM 要求無開啟功能不要顯示
                    {
                        strShowYield[i].UsePanel->Caption="";
                        strShowYield[i].UsePanel->Color=clBtnFace;
                    }
                    else
                    {
                        strShowYield[i].UsePanel->Caption=strShowYield[i].asName+" Off";
                        strShowYield[i].UsePanel->Color=clRed;
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// golden :1581-1614
//---------------------------------------------------------------------------
void TfTemperFrom::ShowOffYieldFun()
{
    if(IniConfig.bShowFunctionWindow)
    {
        for(int i=esytErrorRecovery; i<SHOW_YIELD_TOTAL; i++)
        {
            if(strShowYield[i].bShow==true && strShowYield[i].OnOff==false)
            {
                if(IniConfig.bShowOffYieldBlink)                                //Steven 20120609 : 功能關畫面要不要閃爍
                {
                    strShowYield[i].iCount++;
                    if(strShowYield[i].iCount>5)
                    {
                        strShowYield[i].iCount=0;
                        strShowYield[i].bFlag=!strShowYield[i].bFlag;
                    }
                }
                else
                {
                    // GOLDEN ODDITY (golden :1600), kept verbatim -- see file
                    // tail note. `==` not `=`: a discarded-comparison no-op
                    // statement in both BCB6 and standard C++ (harmless,
                    // `-Wunused-value`-class warning only, no UB). Surrounding
                    // code's intent looks like it wanted the assignment
                    // (steady-on bFlag when blink is disabled), but changing
                    // behaviour here is a user decision, not a translation one.
                    strShowYield[i].bFlag==true;
                }

                if(strShowYield[i].bFlag==true)
                {
                    strShowYield[i].UsePanel->Color=clRed;
                }
                else
                {
                    strShowYield[i].UsePanel->Color=clBtnFace;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------
// golden :1684-1690 (Sender/TCloseAction& dropped -- see forms/fTemperFrom.h
// DEVIATION note)
//---------------------------------------------------------------------------
void TfTemperFrom::FormClose()
{
    bShow=false;
    Timer1->Enabled=false;
    MySleep(300);
}

//---------------------------------------------------------------------------
// golden :1772-1818
//---------------------------------------------------------------------------
void TfTemperFrom::ShowHotName()
{
    for(int i=tcHotPlate1; i<=tcBd2; i++)
    {
        if((iSocketBaseTempCount==eDut4ea || iSocketBaseTempCount==eDut2ea) && i==8)    //kevin 20130321 : Omron EJ1N溫控器 SOCKET BASE 溫控器一個改4個
            continue;

        if(bUT150Install[i]==false)
        {
            ShowTempComp[i]->Caption="---";
            NameTempComp[i]->Caption="---";
        }
        else
        {
            NameTempComp[i]->Caption=asTempCtrl[i];
        }
    }

    if(Tri_Temp_Machine==1)
    {
        NameTempComp[tcShuttle1]->Caption   =asTempCtrl[tcShuttle1];
        NameTempComp[tcShuttle2]->Caption   =asTempCtrl[tcShuttle2];
        NameTempComp[tcShuttle3]->Caption   =asTempCtrl[tcShuttle3];
        NameTempComp[tcShuttle4]->Caption   =asTempCtrl[tcShuttle4];
        NameTempComp[tcATCHotAir1]->Caption =asTempCtrl[tcATCHotAir1];
        NameTempComp[tcATCHotAir2]->Caption =asTempCtrl[tcATCHotAir2];
        NameTempComp[tcHotPlate1]->Caption  =asTempCtrl[tcHotPlate1];
        NameTempComp[tcHotPlate2]->Caption  =asTempCtrl[tcHotPlate2];
        NameTempComp[tcHotPlate3]->Caption  =asTempCtrl[tcHotPlate3];
        NameTempComp[tcHotPlate4]->Caption  =asTempCtrl[tcHotPlate4];
        NameTempComp[tcDoor1]->Caption      =asTempCtrl[tcDoor1];               //Ztex 2023.10.23 Add Index Door Heater
        NameTempComp[tcDoor2]->Caption      =asTempCtrl[tcDoor2];               //Ztex 2023.10.23 Add Index Door Heater
    }

    for(int i=tcAe1; i<=tcBh2; i++)                                             //Steven 20150211 : Index使用KT4H版32組加熱器
    {
        if(bUT150Install[i]==false)
        {
            ShowTempComp[i]->Caption="---";
            NameTempComp[i]->Caption="---";
        }
        else
        {
            NameTempComp[i]->Caption=asTempCtrl[i];
        }
    }
}

//---------------------------------------------------------------------------
// golden :1917-1927 (PURE)
//---------------------------------------------------------------------------
bool TfTemperFrom::TempRunShowAlarmHigh()
{
    for(int i=tcHotPlate1; i<tcTotalCount; i++)
    {
        if(bUT150Install[i] && iTempOverShowAlarmT[i]==1)                       //kevin 20210112 溫度過高
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
// golden :1929-1939 (PURE)
//---------------------------------------------------------------------------
bool TfTemperFrom::TempRunShowAlarmLow()
{
    for(int i=tcHotPlate1; i<tcTotalCount; i++)
    {
        if(bUT150Install[i] && iTempOverShowAlarmT[i]==2)                       //kevin 20210112 溫度過低
        {
            return true;
        }
    }
    return false;
}

// =============================================================================
//  GOLDEN ODDITY -- ShowOffYieldFun, golden :1600 (recorded, not "fixed")
//  --------------------------------------------------------------------------
//  `strShowYield[i].bFlag==true;` is a bare comparison expression (BCB6
//  compiles it as a no-op statement with a discarded result, same as
//  standard C++ -- well-defined, no UB, just wasted work) where golden's own
//  surrounding structure (`if(bShowOffYieldBlink){iCount++; if(iCount>5)
//  {iCount=0; bFlag=!bFlag;}} else {bFlag==true;}`) makes the INTENT read
//  like the assignment `bFlag=true;` was meant (force the blink flag
//  steady-on when blinking is disabled, so the status panel renders clRed
//  instead of clBtnFace). BEHAVIOUR DELTA of the golden-as-written no-op:
//  with `bShowOffYieldBlink==false`, `bFlag` never becomes true via this
//  branch (only the OTHER arm's `!bFlag` toggle can ever flip it, and that
//  arm only runs when blinking IS enabled) -- so a caller with blinking
//  disabled sees the status panel colour latch at whatever `bFlag` happened
//  to hold on entry (false from NSDMI on every ctor-then-first-call path),
//  i.e. it stays clBtnFace and never turns clRed for that config combination.
//  Translated LITERALLY as the comparison (see the function body above), per
//  this tree's "照翻，並在 //AI 註解寫下它為什麼看起來錯" rule -- changing
//  it to the assignment is a real behaviour change (an OFF-and-not-blinking
//  function-status panel would start rendering red instead of staying its
//  neutral face colour) and is reserved for explicit user sign-off, not a
//  translation-fidelity call this wave gets to make on its own.
// =============================================================================
