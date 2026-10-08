// MainTempMode.cpp
// Standard C++ translation of TfMain::ChangeTempMode (BCB6 main.cpp, V3.33.906.0_20260618).
// Source of truth: HT9011UC_Code_V3.33.906.0_20260618/main.cpp:21749-21925 (cp950)
//
// AI(W906-FW-CMD-E) 20260821: new file. CHARTER -- this is main.cpp's TEMPERATURE-MODE
// SWITCH METHOD, TfMain::ChangeTempMode, NOT a calc-core file. It borrows MainCalcCore.cpp's
// banner SHAPE only (one file, one golden-cited function, per-block golden line citations);
// unlike MainCalcCore's pure functions, ChangeTempMode reads/writes real machine state
// (LastSet.iTemperature, Temperature.*, TestMode.iTemperatureMode, the
// bEnableTempHeaterOkShowLight/bEnableTempLess30degShowLight UI-light flags) and forwards to
// real hardware-adjacent calls (ATCInterfaceForm->SendCommToATC7, fTemp_Set->ReadTempFile).
// Every dereference site below is either ACTIVE (the port already has a real, non-stub
// definition of the callee -- verified by grep, 20260821) or wrapped in a SAFETY GATE (#if 0)
// citing the golden line and the absent symbol. Full per-site table in this wave's own report.
//
// THIS TASK'S SCOPE: close the two golden TfMain:: facade gaps -- ChangeTempMode itself, plus
// the sibling edATCAmbientTemper widget forms/fMain.h needed -- that were blocking Command.cpp's
// WriteSetTempStatus_SIGURD / WriteSetSoakTimeStatus_SIGURD GATE REGISTER items 4/5/6 -- NOT to
// un-gate those Command.cpp call sites. That remains a SEPARATE, later task; see this file's
// per-GATE comments below for what un-gating will still need (fSetup->ReadUseSuckModeFile /
// fOffSet->ReadFile / COM2->ATCInitialTask are ALL still absent tree-wide, independent of this
// file landing).
//
// Toolchain: MinGW g++ 6.3+, C++17.

#include "forms/fMain.h"

#include "cprod.h"                  // Temperature / TestIF_File / TestMode / LevelSet /
                                     // SaveTestMode() / SaveTempMode() / SaveTempModeByDLL() --
                                     // transitively pulls Config.h (IniConfig), MachineType.h
                                     // (eTestMode) and CosFunction.h (CosFunction)
#include "cmydef.h"                  // AccessLevel / SystemStart / MMPlate1 / MMPlate2 / MMSystem /
                                     // Tempture_Hot / Tempture_AmbientHot / Tri_Temp_Machine /
                                     // USE_IN_OUT_ARM_Y_PITCH / iXYPitch16Picker / iXYPitch16Bd_Be /
                                     // bEnableTempHeaterOkShowLight / bEnableTempLess30degShowLight
#include "aHotPlateSubstrate.h"      // InArmSuck (TMyKitSuck); transitively pulls Motor/mymotor.h
                                     // for MOT[]/TTrayMotor::HasIC()
#include "canary_support.h"          // LastSet / ShowErrorMessage
#include "csystem.h"                 // sHasICUnderMachine() / ResetHotTime() / IndexHasIC() /
                                     // ShuttleHasIC()
#include "bthermo.h"                 // ClearAllHotBuffer()
#include "ATC/ATCInterface.h"        // ATCInterfaceForm / SendCommToATC7 / ATC_SET_TEMP / ATC_RUN /
                                     // ATC_STOP (real, ACTIVE)
#include "forms/fSetup.h"            //AI(W906-FW-SETUP-C) 20260821: fSetup->ReadUseSuckModeFile
                                     // (real since 6e2d75e; eager instance, forms/fSetup.cpp:21)
#include "forms/fTemp_Set.h"         // fTemp_Set->ReadTempFile(bool) (real, ACTIVE; NULL-guarded,
                                     // same idiom as Command.cpp's WriteSetTempStatus_SIGURD)

// AI(W906-FW-CMD-E) 20260821: same "declare just the one symbol needed, don't
// pull the whole header" posture Command.cpp already established (Command.cpp
// :271-282, and again :284-296 for MyMessageBox): NewRecordProcess's real
// definition lives in acatchtray_shims.cpp (external linkage, reachable at
// link time), but #include "acatchtray_shims.h" or "cMyDB.h" here would
// reintroduce THEIR OWN NewRecordProcess/RecordProcess declarations, which
// collide on default arguments in the same TU (Command.cpp's own comment cites
// the exact reproduction, `g++ -fsyntax-only`, "default argument given for
// parameter 3"). Declared directly here instead, byte-for-byte matching
// Command.cpp:282's signature.
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug=" ");

// ---------------------------------------------------------------------------
// TfMain::ChangeTempMode
//   BCB6 source: main.cpp:21749-21925 (177 lines)      //JerryYang 20181126 add bSetTempByDLL
// ---------------------------------------------------------------------------
int TfMain::ChangeTempMode(int Mode, bool Msg, bool bRefresh, bool bGPIB, bool bSetTempByDLL)
{
    AnsiString S;
    // golden :21752-21753                                                     //Steven 20110518
    if(SystemStart==true)
        return 1;

    // golden :21755-21774 -- any live IC under the machine blocks a mode switch,
    // UNLESS bHighModeCanOffTemp is set and we are already in room-temp mode (0),
    // in which case golden just resets the hot-time watchdog and falls through
    // to keep evaluating.                                                     //kevin 20120501  autoclean
    if(IndexHasIC() || ShuttleHasIC() || InArmSuck.HasIC() ||
       MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())
    {
        if(IniConfig.bHighModeCanOffTemp && Temperature.iMachineTempMode==0)    //Steven 20110524 : 加熱模式要可以隨時關加熱
        {
            ResetHotTime();
        }
        else
        {
            if(Mode==10)
            {
                if(Msg)
                {
                    S=AnsiString("ChangeTempMode : ")+sHasICUnderMachine();     //Steven 20250110 : 顯示哪個位置還有IC
                    ShowErrorMessage("MES1646", 0, MMSystem, false, S);         //Must finish [Clean out]!!
                }
            }
            return 1;
        }
    }

    // golden :21776-21812 -- Mode==10 is the "toggle" caller (GPIB SETTEMP_/UI
    // click). iMachineTempMode==2 skips this whole re-entry guard, matching
    // golden's own bare `!=2` check verbatim (no other iMachineTempMode==2
    // meaning is documented at this call site -- translated literally, not
    // reinterpreted).
    if(Temperature.iMachineTempMode!=2)
    {
        if(Mode==10)
        {
            switch(Temperature.iMachineTempMode)
            {
                case 0:
                    if(IniConfig.bHighModeCanOffTemp &&                         //JerryYang 20251201 : 啟用OnlyRoomOrHot要鎖定,無法切模式
                      IniConfig.bOnlyRoomOrHot==false)                          //Steven 20110524 : 加熱模式要可以隨時關加熱
                    {
                        ResetHotTime();
                    }
                    else
                    {
                        if(Msg==true)
                            ShowErrorMessage("MES15191", 0, MMSystem, false, "ChangeTempMode");
                    }
                    break;
                case 1:
                    if(Msg==true)
                        ShowErrorMessage("MES15192", 0, MMSystem, false, "ChangeTempMode");
                    break;
            }

            if(IniConfig.bHighModeCanOffTemp &&
               IniConfig.bOnlyRoomOrHot==false &&                               //JerryYang 20251201 : 啟用OnlyRoomOrHot要鎖定,無法切模式
               Temperature.iMachineTempMode==0)                                 //Steven 20110524 : 加熱模式要可以隨時關加熱
            {
                ResetHotTime();
            }
            else
            {
                return 1;
            }
        }
    }

    // golden :21814-21922 -- the real mode-flip block, gated on access level /
    // bRefresh / bGPIB (SIGURD's GPIB SETTEMP_ path always passes bGPIB=true,
    // per Command.cpp's own GATE #5 citation).
    if(AccessLevel>=LevelSet.AccessLevel[7] ||
       bRefresh ||                                                              //kevin 20110401
       bGPIB)                                                                   //JerryYang 20151228 GPIB設定溫度不卡權限
    {
        if(Mode==10)
        {
           if(IniConfig.bI03AmbientTempControl)                                 //kevin 20140918 恆溫控制
            {
                LastSet.iTemperature=LastSet.iTemperature+1;
                if(LastSet.iTemperature==2)                                     //Steven 20180816 : 修正模式切換
                    LastSet.iTemperature=3;
                else if(LastSet.iTemperature>3)
                    LastSet.iTemperature=0;
            }
            else
            {
                LastSet.iTemperature=Tempture_Hot-LastSet.iTemperature;
            }
        }
        else
        {
            LastSet.iTemperature=Mode;
        }

        if(Tri_Temp_Machine==1 && LastSet.iTemperature==0)                      //Ztex 2024.02.21 Add Change Temp When Shuttle Auto Shift Pos
            Temperature.fWorkTemperBase=25;

        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();
        }

        if(CosFunction.bFTPDownLoadTempModeBySetupFile==true)                   //Ifor 20180212 (Steven) : add FTP DownLoad Temp Mode By SetupFile
        {
            SaveTempMode();
        }

        if(CosFunction.bDLLCommands && bSetTempByDLL)
        {
            SaveTempModeByDLL();
        }

        if(LastSet.iTemperature==Tempture_Hot)
        {
            NewRecordProcess("MES2151", "Change Hot Mode");

            if(CosFunction.bTempHeaterOkShowLight)                              //jou 20180529 : Temperature Heater Ok Show Light
            {
                bEnableTempHeaterOkShowLight=true;
            }
        }
        else if(LastSet.iTemperature==Tempture_AmbientHot)                      //kevin 20140918 常溫加熱 恆溫控制
        {
            NewRecordProcess("MES2152", "Change Ambient Hot Mode");
        }
        else
        {
            NewRecordProcess("MES2153", "Change Ambient Mode");

            if(CosFunction.bTempLess30degShowLight)                             //jou 2014-12-19 Temperature Less 30 deg. Show Light
            {
                bEnableTempLess30degShowLight=true;
            }

            //AI(W906-FW-SETUP-C) 20260821: GATE(dep-fSetup-ReadUseSuckModeFile)
            // UNLOCKED at both sites -- the premise died with FW-SETUP-B (commit
            // 6e2d75e): TfSetup::ReadUseSuckModeFile now has a real body
            // (cSetUp.cpp, golden :2133-2146) and that wave's audit measured it
            // READ-ONLY (per-recipe HandlerCondition.Data via plain ReadIniData;
            // no missing-key write-back, no MyForceDirectories, no WriteIniData).
            // fSetup is a REAL eager instance (forms/fSetup.cpp:21), no guard
            // needed. Golden :21886/:21893 restored verbatim.
            switch(TestIF_File.iTestMode)                                       //jou 2010-09-03 加熱暫時先關閉使用1x2-4,1x4-8,2x2-8
            {
                case DualSite:                                                  //1x2
                case QualSite1X4:                                               //1x4
                case QualSite2X2:                                               //2x2
                case DualSite2x1:                                               //2x1
                case _8Site1X4:                                                 //ChungHung 20150528 add for 海思 _8Site1x4
                    fSetup->ReadUseSuckModeFile();
                    break;
                case _8Site2X4:
                case _16Site4X4:
                    if(USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Picker ||              //Steven for HT1032
                       USE_IN_OUT_ARM_Y_PITCH==iXYPitch16Bd_Be)                 //Ztex 2023.12.06 Add HT-1032
                    {
                        fSetup->ReadUseSuckModeFile();
                    }
                    break;
            }
        }
        UpdateMainOperateMode();
        if(CosFunction.bNotClearAllHotBuffer==false)                            //JerryYang 20151226 For 矽格 由GPIB設定完溫度後，會馬上再問一次溫度。所以不清除暫存溫度
        {
            ClearAllHotBuffer();
        }

        // golden :21904 -- SAFETY GATE (dep-fOffSet-ReadFile). `fOffSet->ReadFile()`
        // (golden cOffSet.h, TfOffSet member) has NO port anywhere in this tree:
        // forms/fOffSet.h declares only UseAutoOffsetFunction/UseInArmSetupTeach
        // (`grep -n "ReadFile" forms/fOffSet.h` -- 0 hits, re-run 20260821). Void
        // call in golden (re-reads offset/teach data from disk into the Offset
        // dialog after a LastSet write) -- no return value is lost.
#if 0 // GATE(dep-fOffSet-ReadFile): golden :21904
        fOffSet->ReadFile();                                                    //jou 2010-01-15 offset需放在lastset讀取之後
#endif // GATE(dep-fOffSet-ReadFile)
        if(Temperature.bATC70Active==true)                                      //Eliot 2015_0105
        {
            if(LastSet.iTemperature==Tempture_Hot ||                            //Steven 20151111 : For ATC7.0 修改判斷式
               LastSet.iTemperature==Tempture_AmbientHot)                       //kevin 20180811 (Steven) : add 恆溫控制
            {
                ATCInterfaceForm->SendCommToATC7(ATC_SET_TEMP, Temperature.fWorkTemperBase, "");
                ATCInterfaceForm->SendCommToATC7(ATC_RUN, "", "");
            }
            else
            {
                ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");
            }
        }
        else
        {
            // golden :21920 -- SAFETY GATE (dep-COM2-ATCInitialTask), ESTABLISHED
            // precedent: bthermo.cpp:3424-3431 (GATE W5 G18) and
            // uTemp_Set.cpp:3338-3346 both already gate this exact same absent
            // member for the exact same reason -- `COM2` is atester_shims.h's
            // TCOM2Shim (golden rs232.h:203 TCOM2 is unported), which exposes only
            // bCCDDummyRum/DoReleaseAndInspEnd/ATCAlarmSenCheck/InitReadTorueTask/
            // GetReadTorueTask/iWriteAndCheckMotorTorque/OpenRTCComPortAgain/
            // SendCommToVision/bRealTimeCom_ReceiveOK/rtSiteMap/rtFullTNG/rtFullTOK
            // -- NOT ATCInitialTask (`grep -n "ATCInitialTask" .` tree-wide,
            // 20260821 -- the only 2 call sites are the 2 pre-existing #if 0 gates
            // just cited; 0 real/ACTIVE hits anywhere). ACTIVE DEFAULT: no ATC
            // re-init is kicked when bATC70Active is false, same posture those two
            // precedents already accepted.
#if 0 // GATE(dep-COM2-ATCInitialTask): golden :21920
            COM2->ATCInitialTask();                                             //jou 2012-03-14 ATC
#endif // GATE(dep-COM2-ATCInitialTask)
        }
    }
    // golden :21923 -- `fTemp_Set` is a never-`new`'d global pointer in this port
    // (forms/fTemp_Set.h:1544 / uTemp_Set.cpp:220 `TfTemp_Set *fTemp_Set;`, zero-init,
    // same as every other never-streamed VCL form pointer in this tree). NULL-guarded
    // here, same idiom Command.cpp's own WriteSetTempStatus_SIGURD already uses for
    // this identical pointer (GATE REGISTER item 3's resolution, commit c60e9f4) --
    // golden's own line has no guard, but golden's `fTemp_Set` is always a live VCL
    // form instance by construction, which this port cannot assume.
    if(fTemp_Set)
    {
        fTemp_Set->ReadTempFile(true);
    }
    return 0;
}
