// =============================================================================
//  forms/fMain_OperateMode.cpp  --  TfMain::UpdateMainOperateMode /
//                                   TfMain::TemperatureEditDisable /
//                                   TfMain::SetNormalOrPrime
//
//  AI(W906-OPMODE) 20260926: new file. Standard C++17 translation of three
//  TfMain members of the BCB6 main form, translated from the 906 golden:
//    HT9011UC_Code_V3.33.906.0_20260618/main.cpp (cp950, read-only)
//      TfMain::UpdateMainOperateMode   main.cpp:12803-13127  (decl main.h:1236)
//      TfMain::TemperatureEditDisable  main.cpp:27047-27088  (decl main.h:1329)
//      TfMain::SetNormalOrPrime        main.cpp:32387-32410  (decl main.h:1475)
//  Facade declarations: forms/fMain.h:557 (UpdateMainOperateMode, virtual) and
//  forms/fMain.h:562 (TemperatureEditDisable / ChangeATCSiteUse /
//  SetNormalOrPrime). ChangeATCSiteUse's body is NOT here (forms/fMain_ATCSiteUse.cpp).
//
//  USER RULING 20260922 (UpdateMainOperateMode): 「都要，全部動作都要執行，此專案
//  就是要上線的」 -- the WHOLE function is translated, INCLUDING the heater relay
//  (SW[SwHeaterRelay].On()/Off() + HeaterLog) and the ATC 7.0 commands
//  (ATCInterfaceForm->SendCommToATC7 ATC_STOP / ATC_SET_TEMP / ATC_RUN). No safety
//  gate was added. A `#if 0` below exists ONLY where the dependency does not exist
//  in the port; every one says what is missing and cites the golden line.
//
//  Text: golden VERBATIM, statement by statement, same order, original author
//  comments carried over (cp950-decoded to UTF-8, zero U+FFFD). The golden lines
//  were copied mechanically from the decoded golden by line number, not retyped.
//  TfMain members stay unqualified; golden's own `fMain->` spellings are kept.
//
//  PORT-ONLY SEAM: the call counter `W906_UpdateMainOperateModeCallCount++` stays in the virtual stub (forms/fMain.cpp:507); this file defines the golden body as TfMain::W906_UpdateMainOperateModeBody() and wb_serve installs it at boot (file end).
//  of UpdateMainOperateMode. It is not golden -- it is the counter the old stub
//  (forms/fMain.cpp:507) had, and the tests rely on it.
//
//  GATE REGISTER (generated from the gate table; full reason on each #if 0 line)
//  --------------------------------------------------------------------------
//   golden lines           kind               what
//   :12809-12810           missing-cpp-state  TemperatureBackup / TemperatureSoakTimeBackup
//   :12818                 missing-dependency labAutomation->Font
//   :12830-12832           missing-cpp-state  fMain->tPSM
//   :12859                 web-display        spbSet
//   :12861-12862           web-display        labSoakTime / labDeg
//   :12882-12886           web-display        imgTempOnOff / lblTemperatureMode
//   :12895                 web-display        spbSet
//   :12897-12898           web-display        labSoakTime / labDeg
//   :12909-12911           web-display        imgTempOnOff
//   :12913-12914           web-display        lblTemperatureMode
//   :12926                 web-display        spbSet
//   :12928-12929           web-display        labSoakTime / labDeg
//   :12934                 web-display        spbSet
//   :12936-12937           web-display        labSoakTime / imgTempOnOff
//   :12942                 web-display        spbSet
//   :12944-12945           web-display        labSoakTime / imgTempOnOff
//   :12953-12956           web-display        labSoakTime / lblTemperatureMode
//   :12958-12959           web-display        imgTempOnOff
//   :12965                 web-display        lblTemperatureMode
//   :12980                 web-display        lblTemperatureMode
//   :12985                 web-display        lblTemperatureMode
//   :12990                 web-display        lblTemperatureMode
//   :12997-12998           web-display        imgTempOnOff
//   :13007                 web-display        spbSet
//   :13046                 web-display        lblTemperatureMode
//   :13050-13053           web-display        lblTemperatureMode
//   :13071                 web-display        lblTemperatureMode
//   :13075-13078           web-display        lblTemperatureMode
//   :27055                 web-display        spbSet
//   :32391-32392           web-display        palPrime / palNormal
//   :32396-32397           web-display        palPrime / palNormal
//   :32401-32402           web-display        palPrime / palNormal
//   :32407-32408           web-display        palPrime / palNormal
//
//  web-display: the BCB main-form widgets spbSet (TSpeedButton), labSoakTime /
//  labDeg / lblTemperatureMode (TLabel), imgTempOnOff (TImage), palPrime /
//  palNormal (TPanel) are not TfMain facade members (Grep forms/fMain.h: 0 hits
//  each). In this architecture their state is OWNED BY THE WEB PAGE; no member,
//  stub or stand-in was added for them. The facade members this code touches
//  (edWorkTemperBase / edSoakTime / edATCAmbientTemper / cbUserSelect /
//  labAutomation / cbSetupFileName) are translated verbatim.
//  Where golden's widget statement is the unbraced body of an `if`
//  (`if(FileExists(S)) imgTempOnOff->...;`, the Hot/Ambient caption if/else) the
//  whole `if` is inside the gate, so the gate cannot re-attach the condition to
//  the next statement; those conditions have no side effects. The `S=BmpPath+...`
//  picture-path assignments stay ACTIVE (plain C++ state, golden order).
//
//  DELTA-1 (golden :12830-12832, the C05 power-save operand of bEMG). Golden sets
//  bEMG=true -- and so does NOT switch SW[SwHeaterRelay] ON in the two ATC 5.1 /
//  6.0 / 7.0 arms -- when C05 power-save has closed the temperature loop
//  (fMain->tPSM.HotModule->bCheckTempClose). The port has no tPSM, so that operand
//  is `false`. Value-identical TODAY: no TPowerSaving instance exists anywhere in
//  the port (no tPSM; PowerSavingMode.cpp's tPowerSaving is never new'd), so the
//  only writer of bCheckTempClose=true (PowerSavingMode.cpp:702, THotModule inside
//  the scan timer) never runs. It becomes a real difference the day power-saving is
//  wired in -- un-gate this operand in the same change.
//
//  V912 DIFFERENCE (reported, NOT applied -- this file is the 906 text):
//    912 main.cpp:13352 (906 :12829) reads
//      (IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff()) ||
//    instead of 906's (Enable_PLCSafety_IO && Sen[SnAllEMG].IsOff()) ||.
//    TemperatureEditDisable (912 :28037-28078) and SetNormalOrPrime
//    (912 :33480-33503) are identical to 906.
//
//  LINK-LAYER NOTE for whoever registers this file in CMake: the body reaches
//  SW[]/Sen[] (myswitch.cpp/mysensor.cpp, ht9045_io), ATCInterfaceForm
//  (ATC/ATCInterface.cpp), ATC_InterfaceForm (acarry_shims.cpp), ShowErrorMessage
//  (canary_support.cpp) and authMainForm/bAuthCriticalPara (cAuthority.cpp) -- all
//  ht9045_sm --, BmpPath (common.cpp, ht9045_core), and HeaterLog (cpublic.cpp) /
//  WriteLastDataFile / ReadLastDataFile (cprod.cpp) in ht9045_globals. ht9045_forms
//  links only vclcompat + ht9045_globals. UpdateMainOperateMode is `virtual`
//  (forms/fMain.h:557), so TfMain's vtable (emitted with forms/fMain.cpp) will
//  reference this body wherever it lives -- the same forms->sm vtable edge the
//  CleanOut / ProcessSensorScan members were made non-virtual to avoid.
//
//  Toolchain: MinGW g++ 6.3+, C++17. UTF-8 without BOM, CRLF.
// =============================================================================

#include "forms/fMain.h"            // TfMain facade / fMain

#include "cprod.h"                  // Temperature / RunInfo / LevelSet / WriteLastDataFile / ReadLastDataFile
#include "cmydef.h"                 // CUSTOMER_CODE / Sn*EMG / SwHeaterRelay / Tempture_* / AccessLevel /
                                    // iDefEngineerLevel / fHeaterOK / bHeatOKBellowError / iBinModelPrime /
                                    // bEnablePEModel / Tri_Temp_Machine / Enable_PLCSafety_IO / ATC_SYSTEM / MMSystem
#include "MachineType.h"            // CC_* customer codes / eNewATCSystem
#include "Config.h"                 // IniConfig
#include "CosFunction.h"            // CosFunction
#include "canary_support.h"         // LastSet / ShowErrorMessage
#include "cpublic.h"                // HeaterLog
#include "common.h"                 // BmpPath
#include "cAuthority.h"             // authMainForm[] / bAuthCriticalPara[]
#include "mysensor.h"               // Sen[]
#include "myswitch.h"               // SW[]
#include "acarry_shims.h"           // ATC_InterfaceForm (TATC_InterfaceFormShim, ->iATC_MODE_TYPE)
#include "ATC/ATCInterface.h"       // ATCInterfaceForm->SendCommToATC7 / ATC_STOP / ATC_RUN / ATC_SET_TEMP
#include "forms/fATCHandlerSide.h"  // ATC_TYPE_51 / _60 / _70 (golden ATC_Handler_Side.h #defines).  LAST on
                                    //   purpose, same as TesterComm/Handler/HandlerGpibMsg.cpp:164: its
                                    //   ATC_* macros then touch no other header

#include <cstdlib>                  // atoi

// ---------------------------------------------------------------------------
// TfMain::UpdateMainOperateMode -- golden main.cpp:12803-13127 (325 lines)
// ---------------------------------------------------------------------------
void TfMain::W906_UpdateMainOperateModeBody()                                 //AI(W906-OPMODE) 20260926: golden `void __fastcall TfMain::UpdateMainOperateMode()` 的本體。虛擬的 TfMain::UpdateMainOperateMode 留在 forms/fMain.cpp:507（ht9045_forms，不可依賴 ht9045_sm）經 W906_UpdateMainOperateModeHook 呼叫這裡；hook 由 wb_serve 開機裝（本檔尾 W906_InstallUpdateMainOperateMode）
{
    //AI(W906-OPMODE) 20260926: 呼叫計數（PORT-ONLY SEAM）留在 forms/fMain.cpp:507 的樁裡 —— 沒裝 hook 的 ctest 照舊只計數
    AnsiString S;

    edWorkTemperBase->Text      =Temperature.fWorkTemperBase;
    edSoakTime->Text            =Temperature.fSoakTime;
#if 0 // TODO(W906-OPMODE): TemperatureBackup / TemperatureSoakTimeBackup (golden main.h:1188-1189, AnsiString TfMain data members) have no TfMain facade member -- Grep over the port tree (*.h,*.cpp): 0 hits; their only golden readers are edSoakTimeMouseDown / edWorkTemperBaseMouseDown (main.cpp:25764 / :25855, web-owned edit handlers) -- golden main.cpp:12809-12810
    TemperatureBackup           =Temperature.fWorkTemperBase;                   //kevin 20160908
    TemperatureSoakTimeBackup   =Temperature.fSoakTime;                         //kevin 20160908
#endif

    if(CUSTOMER_CODE==CC_AMKOR_Korea)                                           //ChungHung 20120413 add
    {
        if(cbUserSelect->Text=="HonPrec")
        {
            labAutomation->Visible=true;
            labAutomation->Caption="Server Connected";
#if 0 // TODO(W906-OPMODE): labAutomation->Font -- missing dependency (not web-display): labAutomation IS a facade member (forms/fMain.h:280) but vclcompat TPanel has no Font and the facade has no parallel labAutomationFont (the fMain.h:1204-1205 pnlCleanCountFont pattern) -- golden main.cpp:12818
            labAutomation->Font->Color=clBlue;
#endif
        }
        else
        {
            labAutomation->Visible=false;
        }
    }

    bool bEMG=false;
    if(Sen[SnFrontLeftEMG].IsOff() || Sen[SnFrontRightEMG].IsOff() ||
       Sen[SnRearLeftEMG ].IsOff() || Sen[SnRearRightEMG ].IsOff() ||           //緊停被按下時
       (IsSafePLCIOInstall() && Sen[SnAllEMG].IsOff()) ||                        //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門  AI(W906-W217) 20261010 (Ifor01): golden 913 main.cpp:13496
#if 0 // TODO(W906-OPMODE): fMain->tPSM (golden main.h:1339 `TPowerSaving tPSM;`) is not a TfMain facade member -- Grep "tPSM" over the port tree: only gated precedents (csystem.cpp G05, WebStart.cpp W906-ST-W5-D, forms/fMain.cpp W906-HOME-W1-PSM); PowerSavingMode.h:156 tPowerSaving is a different object and is never new'd -- golden main.cpp:12830-12832
       (IniConfig.bPowerSaveFunction==true &&
        IniConfig.bC05_PowerSaveTemp==true &&
        fMain->tPSM.HotModule->bCheckTempClose==true))                          //Ifor 20230705 add: EMG || 省電模式不啟動 SwHeaterRelay
#else
       false)                                                                   //AI(W906-OPMODE) 20260926: the gated power-save operand contributes nothing (X || false == X); see banner DELTA-1
#endif
    {
        bEMG=true;
    }

    if(Temperature.bATCActiveCooling &&
       LastSet.iTemperature==Tempture_Ambient)                                  //wei 20151013  by Setup File ATC Ambient Temp set
    {
        if(ATC_SYSTEM==eNewATCSystem &&
          (ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60 ||                    //Ifor 20160527 ATC6.0 不可關閉 Heat Relay
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_51 ||                    //wei 20170217 (Steven) add ATC5.1
           ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70))                     //Ifor 20170328 Add ATC7.0
        {
            if(bEMG==false &&                                                   //Ifor 20230705 add: EMG || 省電模式不啟動 SwHeaterRelay
               SW[SwHeaterRelay].Status()==false)                               //Ifor 20170329 (wei) add 避免 Heater Relay 重複On的問題
            {
                SW[SwHeaterRelay].On();
                HeaterLog("UpdateMainOperateMode_1", true);
            }
        }
        else
        {
            SW[SwHeaterRelay].Off();
            HeaterLog("UpdateMainOperateMode_1", false);                        //Steven 20151123 : Log for Heater Relay
        }

        edWorkTemperBase->Visible=false;
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12859
        spbSet          ->Visible=true;
#endif
        edSoakTime      ->Visible=false;
#if 0 // TODO(W906-OPMODE): labSoakTime / labDeg are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12861-12862
        labSoakTime     ->Visible=false;
        labDeg          ->Visible=true;
#endif
        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                     //kevin 20190129 add ATC 3.1 kaohsiung use abient control
        {
            edATCAmbientTemper->Visible=false;
        }
        else
        {
            edATCAmbientTemper->Visible=true;
            edATCAmbientTemper->Text=IniConfig.dATCAmbientTemperature;
        }

        if(CUSTOMER_CODE==CC_TERAPOWER)                                         //Sam 20181025 : For 晶兆成常溫圖片變更
        {
            S=BmpPath+"NoTempe_1.bmp";
        }
        else
        {
            S=BmpPath+"NoTempe.bmp";
        }

#if 0 // TODO(W906-OPMODE): imgTempOnOff / lblTemperatureMode are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12882-12886
        if(FileExists(S))
            imgTempOnOff ->Picture->LoadFromFile(S);
        imgTempOnOff     ->Enabled=authMainForm[6];                             //Steven 20090731
        lblTemperatureMode->Caption="ATC Mode";                                 //Ifor 20160527 ATC Mode
        lblTemperatureMode->Font->Color=clGreen;
#endif

        ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");                     //Eliot 2015_0105 //Steven 20151111 : For ATC 7.0
    }
    else if(LastSet.iTemperature==Tempture_Ambient)
    {
        SW[SwHeaterRelay].Off();
        HeaterLog("UpdateMainOperateMode_2", false);                            //Steven 20151123 : Log for Heater Relay
        edWorkTemperBase->Visible=false;
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12895
        spbSet          ->Visible=false;
#endif
        edSoakTime      ->Visible=false;
#if 0 // TODO(W906-OPMODE): labSoakTime / labDeg are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12897-12898
        labSoakTime     ->Visible=false;
        labDeg          ->Visible=false;
#endif
        edATCAmbientTemper->Visible=false;
        if(CUSTOMER_CODE==CC_TERAPOWER)                                         //Sam 20181025 : For 晶兆成常溫圖片變更
        {
            S=BmpPath+"NoTempe_1.bmp";
        }
        else
        {
            S=BmpPath+"NoTempe.bmp";
        }

#if 0 // TODO(W906-OPMODE): imgTempOnOff is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12909-12911
        if(FileExists(S))
            imgTempOnOff->Picture->LoadFromFile(S);
        imgTempOnOff    ->Enabled=authMainForm[6];                              //Steven 20090731
#endif

#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12913-12914
        lblTemperatureMode->Caption="Ambient Mode";
        lblTemperatureMode->Font->Color=clGreen;
#endif

        ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");                     //Eliot 2015_0105
    }
    else
    {
        //DoHeaterOn();
        if(CosFunction.bUseIndividulTempSet &&
           Temperature.bUseIndividualTemp)                                      //Steven 20140924 : 各個加熱區獨立有自己的設定值
            edWorkTemperBase->Visible=false;
        else
            edWorkTemperBase->Visible=true;
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12926
        spbSet      ->Visible=true;
#endif
        edSoakTime  ->Visible=true;
#if 0 // TODO(W906-OPMODE): labSoakTime / labDeg are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12928-12929
        labSoakTime ->Visible=true;
        labDeg      ->Visible=true;
#endif
        edATCAmbientTemper ->Visible=false;
        if(authMainForm[6])                                                     //Steven 20090731
        {
            edWorkTemperBase->Enabled=true;
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12934
            spbSet          ->Visible=true;
#endif
            edSoakTime      ->Enabled=true;
#if 0 // TODO(W906-OPMODE): labSoakTime / imgTempOnOff are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12936-12937
            labSoakTime     ->Enabled=true;
            imgTempOnOff    ->Enabled=true;
#endif
        }
        else
        {
            edWorkTemperBase->Enabled=false;
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12942
            spbSet          ->Visible=false;
#endif
            edSoakTime      ->Enabled=false;
#if 0 // TODO(W906-OPMODE): labSoakTime / imgTempOnOff are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12944-12945
            labSoakTime     ->Enabled=false;
            imgTempOnOff    ->Enabled=false;
#endif
        }

        if(LastSet.iTemperature==Tempture_AmbientHot)                           //kevin 20140918 恆溫控制
        {
            edSoakTime  ->Enabled=false;                                        //kevin 20141219 恆溫沒有sock time
            edSoakTime  ->Text="0";                                             //kevin 20141219
            edSoakTime  ->Visible=false;
#if 0 // TODO(W906-OPMODE): labSoakTime / lblTemperatureMode are display state owned by the web page (no TfMain facade member) -- golden main.cpp:12953-12956
            labSoakTime ->Visible=false;
            //lblTemperatureMode->Caption="Ambient_Control_mode";               //kevin 20141219
            lblTemperatureMode->Caption="None HotPlate_mode";                   //kevin 20180811 change k8
            lblTemperatureMode->Font->Color=clBlue;
#endif
            S=BmpPath+"LowTempe.bmp";
#if 0 // TODO(W906-OPMODE): imgTempOnOff is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12958-12959
            if(FileExists(S))
                imgTempOnOff->Picture->LoadFromFile(BmpPath+"LowTempe.bmp");
#endif
        }
        else
        {
            if(Temperature.bATCActiveCooling==true)                             //Ifor 20160527 顯示ATC控溫
            {
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12965
                lblTemperatureMode->Caption="ATC Mode";
#endif
                if(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_60   ||          //Ifor 20170328 (wei) add 程式開啟無啟動 SwHeaterRelay 導致ATC發生異常
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_51  ||
                   ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_70  )
                {
                    if(bEMG==false &&                                           //Ifor 20230705 add: EMG || 省電模式不啟動 SwHeaterRelay
                       SW[SwHeaterRelay].Status()==false)                       //Ifor 20170329 (wei) add 避免 Heater Relay 重複On的問題
                    {
                        SW[SwHeaterRelay].On();
                        HeaterLog("UpdateMainOperateMode_1", true);
                    }
                }
            }
            else
            {
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12980
                lblTemperatureMode->Caption="Hot Mode";
#endif
            }

            if(Tri_Temp_Machine==1 && Temperature.fWorkTemperBase<25)
            {
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12985
                lblTemperatureMode->Font->Color=clBlue;
#endif
                S=BmpPath+"LowTempe.bmp";
            }
            else
            {
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12990
                lblTemperatureMode->Font->Color=clRed;
#endif
                if(IniConfig.bKoreaFunction==true)                              //Steven 20150914 : Add for ATK
                    S=BmpPath+"Tempe_1.bmp";
                else
                    S=BmpPath+"Tempe.bmp";
            }

#if 0 // TODO(W906-OPMODE): imgTempOnOff is display state owned by the web page (no TfMain facade member) -- golden main.cpp:12997-12998
            if(FileExists(S))
                imgTempOnOff->Picture->LoadFromFile(S);
#endif
        }
        fHeaterOK=false;
        if(CUSTOMER_CODE!=CC_ASE_KaohSiung)                                     //kevin 20141015   會造成溫度過低不會發alarm
           bHeatOKBellowError=false;                                            //jou 2014-06-12 修正偶發性秀低溫異常
        TemperatureEditDisable();

        if(CUSTOMER_CODE==CC_ASE_CL)                                            //Steven 20110309
        {
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:13007
            spbSet->Caption="";
#endif
        }

        if(Temperature.bATC70Active==true)                                      //Eliot 2015_0105
        {
            if(LastSet.iTemperature==Tempture_Hot)                              //Steven 20151111 : For ATC7.0 修改判斷式
            {
                ATCInterfaceForm->SendCommToATC7(ATC_SET_TEMP, Temperature.fWorkTemperBase, "");
                ATCInterfaceForm->SendCommToATC7(ATC_RUN, "", "");
            }
            else
            {
                ATCInterfaceForm->SendCommToATC7(ATC_STOP, "", "");
            }
        }
    }

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20151216 海思專用版本 顯示ATC溫度
    {
        AnsiString StrRef="";
        AnsiString aFileName_HS="";
        int iTemp_HS=0;

        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            aFileName_HS=fMain->cbSetupFileName->Text;                          //Ifor 20160102 :判斷工作檔是否為HISI專用
            if(aFileName_HS.Pos("9203")==1 ||                                   //Ifor 20170424 (wei) KYEC 要求海思版本新增客戶碼
               aFileName_HS.Pos("5611")==1 ||
               aFileName_HS.Pos("9287")==1 ||
               aFileName_HS.Pos("3971")==1 ||
               aFileName_HS.Pos("9606")==1 ||
               aFileName_HS.Pos("KL")==1   ||                                   //Ifor 20200914 Fix: HISI => KL
               aFileName_HS.Pos("9378")==1 )                                    //Ifor 20190711 : KYEC 要求新增客戶代碼9378
            {
                if(Temperature.bATCActiveCooling==true)
                {
                    StrRef=aFileName_HS.SubString(24, 3);
                    iTemp_HS=atoi(StrRef.c_str());
                    StrRef="ATC_"+IntToStr(iTemp_HS)+"C";
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:13046
                    lblTemperatureMode->Caption=StrRef;
#endif
                }
                else
                {
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:13050-13053
                    if(LastSet.iTemperature==Tempture_Hot)                      //Ifor 20160427 修改ATC機台關閉ATC生產溫度模式顯示
                        lblTemperatureMode->Caption="Hot Mode";
                    else
                        lblTemperatureMode->Caption="Ambient Mode";
#endif
                }
            }
        }
        else if(CUSTOMER_CODE==CC_SPIL_SHINCHU ||                               //Ifor 20160406 矽品海思專用版本不判斷Setup File Name
                CUSTOMER_CODE==CC_SPIL_TAICHUNG_LOGIC ||
                CUSTOMER_CODE==CC_SPIL_CHINA_SUZHOU)
        {
            if(Temperature.bATCActiveCooling==true)                             //Ifor 20160406 矽品海思溫度顯示判斷
            {
                if(LastSet.iTemperature==Tempture_Ambient)
                {
                    StrRef="ATC_"+FloatToStr(IniConfig.dATCAmbientTemperature)+"C";
                }
                else
                {
                    StrRef="ATC_"+FloatToStr(Temperature.fWorkTemperBase)+"C";
                }
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:13071
                lblTemperatureMode->Caption=StrRef;
#endif
            }
            else
            {
#if 0 // TODO(W906-OPMODE): lblTemperatureMode is display state owned by the web page (no TfMain facade member) -- golden main.cpp:13075-13078
                if(LastSet.iTemperature==Tempture_Hot)                          //Ifor 20160427 修改ATC機台關閉ATC生產溫度模式顯示
                    lblTemperatureMode->Caption="Hot Mode";
                else
                    lblTemperatureMode->Caption="Ambient Mode";
#endif
            }
        }

        if(CUSTOMER_CODE==CC_KYEC_LEE)
        {
            if(bEnablePEModel==true)
            {
                edATCAmbientTemper->Enabled=true;                               //Ifor 20160411 海思版本強制關閉常溫溫度修改反灰    //Ifor 20170807 (wei) Hisi_V02.01 版本開放更改 PE模式下
                edWorkTemperBase->Enabled=true;                                 //Ifor 20160411 海思版本強制關閉工作溫度修改反灰    //Ifor 20170807 (wei) Hisi_V02.01 版本開放更改 PE模式下
                edSoakTime->Enabled=true;                                       //Ifor 20160822 海思版本強制關閉Soak Time 修改反灰  //Ifor 20170807 (wei) Hisi_V02.01 版本開放更改 PE模式下
            }
            else
            {
                edATCAmbientTemper->Enabled=false;                              //Ifor 20160411 海思版本強制關閉常溫溫度修改反灰    //Ifor 20170807 (wei) Hisi_V02.01 版本開放更改 PE模式下
                edWorkTemperBase->Enabled=false;                                //Ifor 20160411 海思版本強制關閉工作溫度修改反灰    //Ifor 20170807 (wei) Hisi_V02.01 版本開放更改 PE模式下
                edSoakTime->Enabled=false;                                      //Ifor 20160822 海思版本強制關閉Soak Time 修改反灰  //Ifor 20170807 (wei) Hisi_V02.01 版本開放更改 PE模式下
            }
        }
        else
        {
            edATCAmbientTemper->Enabled=true;
            edWorkTemperBase->Enabled=true;
            edSoakTime->Enabled=true;
        }
    }

    if(CosFunction.bLotStartLockCriticalPara && RunInfo.bLotStart)              //JerryYang 20220311 : ATP鎖定Critical parameter
    {
        if(bAuthCriticalPara[0])
        {
            edWorkTemperBase->Enabled=false;
        }

        if(bAuthCriticalPara[1])
        {
            edSoakTime->Enabled=false;
        }
    }

    SetNormalOrPrime();
    WriteLastDataFile();
    ChangeATCSiteUse();                                                         //Steven 20120523 : ATC

    bool bRetrun=false;
    bRetrun=ReadLastDataFile();

    if(bRetrun==false)
        ShowErrorMessage("WAR1681", 0, MMSystem, 0, "ReadLastDataFile");
}
//------------------------------------------------------------------------------
// TfMain::TemperatureEditDisable -- golden main.cpp:27047-27088 (42 lines)
// ---------------------------------------------------------------------------
//AI(W906-OPMODE) 20260926: golden `void __fastcall` -- __fastcall dropped (facade convention, forms/fMain.h:562)
void TfMain::TemperatureEditDisable()                                           //Steven 20110421
{
    if(IniConfig.bShowLotInfo)                                                  //Steven 20110113
    {
        if(IniConfig.bEnableRms && AccessLevel<=iDefEngineerLevel)              //jou 2014-06-19 Security Have 5 Level 1->iDefEngineerLevel
        {
            edWorkTemperBase->Enabled=false;
            edSoakTime->Enabled=false;
#if 0 // TODO(W906-OPMODE): spbSet is display state owned by the web page (no TfMain facade member) -- golden main.cpp:27055
            spbSet->Caption="";
#endif
        }
        else
        {
            if(CUSTOMER_CODE==CC_TERAPOWER)                                     //Sam 20181126 : 晶兆成權限不足溫度修改反白
            {
                edWorkTemperBase->Enabled=(AccessLevel<LevelSet.AccessLevel[7])?false:true;
                edSoakTime      ->Enabled=(AccessLevel<LevelSet.AccessLevel[7])?false:true;
            }
            else
            {
                edWorkTemperBase->Enabled=true;
                edSoakTime->Enabled=true;
            }
        }
    }

    if(CosFunction.bHiSiliconFunction==true)                                    //Ifor 20170905 :海思專版可使用獨立密碼修改溫度
    {
        if(CUSTOMER_CODE==CC_KYEC_LEE)                                          //Ifor 20170905 :京元海思專版使用PE模式修改溫度
        {
            if(bEnablePEModel==false)
            {
                edWorkTemperBase->Enabled=false;
                edSoakTime->Enabled=false;
            }
        }
        else
        {
            edWorkTemperBase->Enabled=true;
            edSoakTime->Enabled=true;
        }
    }
}
//------------------------------------------------------------------------------
// TfMain::SetNormalOrPrime -- golden main.cpp:32387-32410 (24 lines)
// ---------------------------------------------------------------------------
void TfMain::SetNormalOrPrime()                                                 //Steven 20160629
{
    if(IniConfig.bFTBin2RTBin && IniConfig.bA02BinModelPrime)
    {
#if 0 // TODO(W906-OPMODE): palPrime / palNormal are display state owned by the web page (no TfMain facade member) -- golden main.cpp:32391-32392
        palPrime->Visible=true;
        palNormal->Visible=true;
#endif

        if(iBinModelPrime && IniConfig.bA02BinModelPrime)
        {
#if 0 // TODO(W906-OPMODE): palPrime / palNormal are display state owned by the web page (no TfMain facade member) -- golden main.cpp:32396-32397
            palPrime->Color=clRed;
            palNormal->Color=clBtnFace;
#endif
        }
        else
        {
#if 0 // TODO(W906-OPMODE): palPrime / palNormal are display state owned by the web page (no TfMain facade member) -- golden main.cpp:32401-32402
            palPrime->Color=clBtnFace;
            palNormal->Color=clLime;
#endif
        }
    }
    else
    {
#if 0 // TODO(W906-OPMODE): palPrime / palNormal are display state owned by the web page (no TfMain facade member) -- golden main.cpp:32407-32408
        palPrime->Visible=false;
        palNormal->Visible=false;
#endif
    }
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
//AI(W906-OPMODE) 20260926: 安裝點。forms/fMain.cpp 在 ht9045_forms（最底層，CMakeLists 明訂不可依賴 ht9045_sm），而上面的本體要用 ht9045_sm 的
//   ATCInterfaceForm／fTemp_Set／ChangeATCSiteUse 等 ⇒ 虛擬的 TfMain::UpdateMainOperateMode 留在 fMain.cpp（計數＋經 hook 呼叫），
//   本體在這裡（ht9045_sm）。跟 St01 S92 的 W906_BackupSetupFileBody 同一套：wb_serve 開機呼叫 W906_InstallUpdateMainOperateMode()；
//   沒呼叫的程式（每一支 ctest）＝原本的計數樁，不會切加熱器繼電器、不會送 ATC 命令、不會寫 lastdata／config.ini。
static void W906_UpdateMainOperateModeThunk(TfMain* m) { if (m != 0) m->W906_UpdateMainOperateModeBody(); }
static void W906_ChangeATCSiteUseThunk(TfMain* m) { if (m != 0) m->ChangeATCSiteUse(); }   // AI(W906-OPMODE-2) 20260926: forms/fMain.cpp Home（golden main.cpp:7069）經這個 hook
void W906_InstallUpdateMainOperateMode() { W906_UpdateMainOperateModeHook = &W906_UpdateMainOperateModeThunk;  W906_ChangeATCSiteUseHook = &W906_ChangeATCSiteUseThunk; }
