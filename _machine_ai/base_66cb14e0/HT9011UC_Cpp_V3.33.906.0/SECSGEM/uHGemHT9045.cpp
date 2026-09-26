//---------------------------------------------------------------------------
//  SECSGEM/uHGemHT9045.cpp -- permanent home for HT9045Gem::HT9045Gem ctor
//
//  Translation wave: W906-uHGemHT9045-Bucket0 (skeleton swap-back)
//  Translator: AI(W906-uHGemHT9045-Bucket0) 20260721
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemHT9045.cpp
//                 (ctor's first two lines only -- see uHGemHT9045.h's own
//                 file-head note for why the rest of this ~9200-line file
//                 remains out of scope for this wave).
//
//  This ctor body is copied VERBATIM from the now-deleted
//  uHGemHT9045_Shim.cpp (W906-SysModWire 20260720), NOT reconstructed from
//  golden -- golden's own ctor (uHGemHT9045.cpp:66-69) lacks the ActiveWire
//  rebind below (a deliberate port-specific addition, see uHGemClass.h:
//  139-151) and instead seeds a ~300-line EventDescription[] string table
//  (golden :71-368) this class does not carry members for (see uHGemHT9045.h's
//  own "DATA MEMBERS" note -- that population remains future-bucket work,
//  not this wave's, regardless of that data-member decision).
//
//  golden ctor body (uHGemHT9045.cpp:66-69), for reference only -- NOT what
//  is implemented below:
//    HT9045Gem::HT9045Gem(AnsiString Path, THGem *HGemTmp)   // Tmp
//    {
//        HGemPtr=HGemTmp;
//        HandlerPath=Path;
//        ... ~300 lines of EventDescription[...] string-table seeding ...
//    }
//---------------------------------------------------------------------------
#include "SECSGEM/uHGemHT9045.h"
#include "SECSGEM/uHGemEquipment.h"   // THGem complete type (needed for HGemTmp->WireCodec)

// ===========================================================================
//  GOLDEN VERBATIM PAIR -- HT9045Gem::HT9045Gem()
//  GATED : golden SECSGEM/uHGemHT9045.cpp:66-369 (304 lines), inert reference text.
//  LIVE  : the HT9045Gem::HT9045Gem() body immediately AFTER the #endif below.  It is UNCHANGED by
//          this wave -- net behaviour change is ZERO.
//  WHY   : the census scored this "translated" because a same-named LIVE body
//          existed, without comparing SIZE.  Golden's 304 lines were NOWHERE in
//          the tree -- lost text, not deferred behaviour.  Now the text EXISTS
//          and is auditable, so a later un-gate is mechanical.
//  NOTE  : the LIVE ctor is 17 lines (HGemPtr/HandlerPath + a port-only ActiveWire
//          rebind).  Golden's ~300-line EventDescription[] string table -- the whole
//          SECS event-name universe -- was NOWHERE in the tree.  It is here now.
//  RULES : nothing inside the gate is fixed, renamed, reflowed or reindented;
//          it is golden's bytes.  Nothing it references had to be made to
//          exist -- no stub, declaration or header was added for it.
//  SHAPE : same pair shape as csystem.cpp MainProc / atester.cpp (PT-W6a/W6b)
//          golden-verbatim gates.
// ===========================================================================
#if 0 // GOLDEN VERBATIM -- golden SECSGEM/uHGemHT9045.cpp:66-369 (304 lines).  GATE G-PTW6c-HT9045Gem.  NOT COMPILED: the ACTIVE HT9045Gem::HT9045Gem() is the body immediately after this #endif.
HT9045Gem::HT9045Gem(AnsiString Path, THGem *HGemTmp)                           // Tmp
{
    HGemPtr=HGemTmp;
    HandlerPath=Path;

    EventDescription[SECS_EVENT.DoStart]            ="1 Start Pressed";
    EventDescription[SECS_EVENT.DoPause]            ="2 Pause Pressed";
    EventDescription[SECS_EVENT.DoOneCycle]         ="3 OneCycle Pressed";
    EventDescription[SECS_EVENT.DoCleanOut]         ="4 CleanOut Pressed";
    EventDescription[SECS_EVENT.DoClearCount]       ="5 ClearCount Pressed";
    EventDescription[SECS_EVENT.DoLotStart]         ="6 Lot Start";
    EventDescription[SECS_EVENT.DoLot]              ="7 Lot";
    EventDescription[SECS_EVENT.DoLotEnd]           ="8 Lot End";
    EventDescription[SECS_EVENT.SwitchRunMode]      ="9 Switch Real Dummy Mode";
    EventDescription[SECS_EVENT.SwitchTesterMode]   ="10 Switch Tester Online";
    EventDescription[SECS_EVENT.SwitchProduction]   ="11 Switch Production Mode";
    EventDescription[SECS_EVENT.SwitchEngineer]     ="12 Switch Engineer Mode";
    EventDescription[SECS_EVENT.SwitchTemperature]  ="13 Switch Temperature Mode";
    EventDescription[SECS_EVENT.SwitchStartMode]    ="14 Switch StartMode";
    EventDescription[SECS_EVENT.SwitchSetupFile]    ="15 Switch Setup File";
    EventDescription[SECS_EVENT.SwitchUser]         ="16 Switch UserLevel";
    EventDescription[SECS_EVENT.EnterTool]          ="17 Enter Tool Page";
    EventDescription[SECS_EVENT.EnterConfig]        ="18 Enter Maintenance Page";
    EventDescription[SECS_EVENT.EnterOffset]        ="19 Enter Offset Page";
    EventDescription[SECS_EVENT.EnterSpeed]         ="20 Enter Speed Page";
    EventDescription[SECS_EVENT.EnterIO]            ="21 Enter IO Page";
    EventDescription[SECS_EVENT.EnterMessage]       ="22 Enter Message Page";
    EventDescription[SECS_EVENT.EnterDebug]         ="23 Enter Debug Page";
    EventDescription[SECS_EVENT.DoExit]             ="24 Exit Pressed";
    EventDescription[SECS_EVENT.DoHome]             ="25 Home Pressed";
    EventDescription[SECS_EVENT.GetTestResult]      ="26 Get Test Result";
    EventDescription[SECS_EVENT.RunStatus]          ="27 Change Machine State";
    EventDescription[SECS_EVENT.DoRetry]            ="28 Retry Pressed";
    EventDescription[SECS_EVENT.DoSkip]             ="29 Skip Pressed";
    EventDescription[SECS_EVENT.DoAlarmReset]       ="30 Alarm Reset Pressed";
    EventDescription[SECS_EVENT.DoTrayEnd]          ="31 Tray End Pressed";
    EventDescription[SECS_EVENT.DoTrayFeed]         ="32 Tray Feed Pressed";
    EventDescription[SECS_EVENT.DoReset]            ="33 Reset Pressed";
    EventDescription[SECS_EVENT.DoAutoClean]        ="34 Auto Clean Start";
    EventDescription[SECS_EVENT.Auto1Full]          ="35 Auto1 Full";
    EventDescription[SECS_EVENT.Auto2Full]          ="36 Auto2 Full";
    EventDescription[SECS_EVENT.Auto3Full]          ="37 Auto3 Full";
    EventDescription[SECS_EVENT.Fix1Full]           ="38 Fix1 Full";
    EventDescription[SECS_EVENT.Fix2Full]           ="39 Fix2 Full";
    EventDescription[SECS_EVENT.Fix3Full]           ="40 Fix3 Full";
    EventDescription[SECS_EVENT.OneCycleFinish]     ="41 One Cycle Finish";
    EventDescription[SECS_EVENT.CleanOutFinish]     ="42 Clean Out Finish";
    EventDescription[SECS_EVENT.DownloadRecipe]     ="43 DownLoadRecipe";
    EventDescription[SECS_EVENT.SiteOnOff]          ="44 Site On Off";
    EventDescription[SECS_EVENT.ArmOnOff]           ="45 Arm On Off";
    EventDescription[SECS_EVENT.SwitchTempData]     ="46 Change Temp Defaultand Soak Time";
    EventDescription[SECS_EVENT.SwitchSpeed]        ="47 Change HandlerSpeed";
    EventDescription[SECS_EVENT.ChangeEC]           ="48 Change EC";
    EventDescription[SECS_EVENT.TrayFeedFinish]     ="49 Tray Feed Finish";
    EventDescription[SECS_EVENT.AutoCleanFinish]    ="50 Auto Clean Finish";
    EventDescription[SECS_EVENT.SiteMappingStart]   ="51 Site Mapping Start";
    EventDescription[SECS_EVENT.SiteMappingEnd]     ="52 Site Mapping End";
    EventDescription[SECS_EVENT.UPHRecordStart]     ="53 UPH Record Start";
    EventDescription[SECS_EVENT.UPHRecordEnd]       ="54 UPH Record End";
    EventDescription[SECS_EVENT.InitialArtStart]    ="55 Initial ART Start";
    EventDescription[SECS_EVENT.TesterFT]           ="56 Change Tester Program to FT";
    EventDescription[SECS_EVENT.TesterRT]           ="57 Change Tester Program to RT";
    EventDescription[SECS_EVENT.ReadyForArt]        ="58 Ready for ART";

    EventDescription[SECS_EVENT.ArtReceiveTrayOK]       ="59 ART Receive Tray OK";
    EventDescription[SECS_EVENT.ArtReceiveTraySTART]    ="60 ART Receive Tray START";
    EventDescription[SECS_EVENT.ArtRTFinish]            ="61 RT Finish";
    EventDescription[SECS_EVENT.ArtTrayFeedFinish]      ="62 ART Finish";
    EventDescription[SECS_EVENT.ArtFTFinish]            ="63 FT Finish";
    EventDescription[SECS_EVENT.DownLoadRecipeByFTPOK]  ="64 DownLoad Recipe by FTP OK";
    EventDescription[SECS_EVENT.DownLoadRecipeByFTPNG]  ="65 DownLoad Recipe by FTP NG";
    EventDescription[SECS_EVENT.LoadTrayFinish]         ="66 Load Tray Finish";
    EventDescription[SECS_EVENT.TrayTestFinish]         ="67 Tray Test Finish";
    EventDescription[SECS_EVENT.AutoCleanClearCount]    ="68 Auto Clean Clear Count";
    EventDescription[SECS_EVENT.SiteMappingStop]        ="69 Site Mapping Stop";
    EventDescription[SECS_EVENT.BarcodeReaderEnter]     ="70 Barcode Reader Enter";
    EventDescription[SECS_EVENT.OTDLock]                ="71 OTD Lock";
    EventDescription[SECS_EVENT.OTDUnLock]              ="72 OTD UnLock";
    EventDescription[SECS_EVENT.MymessboxOK]            ="73 Mymessbox OK";
    EventDescription[SECS_EVENT.RemoteProgramClose]     ="74";
    EventDescription[SECS_EVENT.ChangeTesterPrgToEQC]   ="75";
    EventDescription[SECS_EVENT.DoStartHasIC]           ="76 Start Pressed HasIC";                                      //Ifor 20151118 非第一次啟動
    EventDescription[SECS_EVENT.ReadCurrentESDData]     ="77 Read Current ESD Data";                                    //Ifor 20160302 Time event, after setting time will sent report automatically
    EventDescription[SECS_EVENT.JamSkipICCount]         ="78 Jam Skip IC Count";                                        //wei 20160503 Jam Skip IC Count
    EventDescription[SECS_EVENT.REVERSED79]             ="79 ";
    EventDescription[SECS_EVENT.ReadNowHandlerData]     ="80 Read Now Handler Data";                                    //Ifor 20160330 KYEC 要求一個Even 即可
    EventDescription[SECS_EVENT.ReadATCTemperature]     ="81 Read ATC Temperature";
    EventDescription[SECS_EVENT.ReadATCRefTemperature]  ="82 Read ATC Ref Temperature";
    EventDescription[SECS_EVENT.ReadNowEPPenconder]     ="83 Read Now EP Penconder";
    EventDescription[SECS_EVENT.RunStatus_FT]           ="84 Run Status FT";    //wei 20170104
    EventDescription[SECS_EVENT.RunStatus_RT]           ="85 Run Status RT";    //wei 20170104
    EventDescription[SECS_EVENT.MapNoArmHasIC]          ="86 Map No Device Arm Has Device";                             //wei 20170104
    EventDescription[SECS_EVENT.MapHasICArmRetry]       ="87 Map Has Device Arm Error Retry";                           //wei 20170104
    EventDescription[SECS_EVENT.MapHasICArmSkip]        ="88 Map Has Device Arm Error Skip";                            //wei 20170104
    EventDescription[SECS_EVENT.PreAlarmMessage]        ="89 Pre Alarm Message";                                        //Ifor 20171024 : add Pre Alarm Position

    EventDescription[SECS_EVENT.GetTestResultAndBarcode]="90 Get TestResult And Barcode";                               //7045
    EventDescription[SECS_EVENT.SECSOffline]            ="91 SECS/GEM Offline";                                         //7045
    EventDescription[SECS_EVENT.SECSOnline]             ="92 SECS/GEM Online";  //7045
    EventDescription[SECS_EVENT.SECSOnlineRemote]       ="93 SECS/GEM Online Remote";                                   //7045
    EventDescription[SECS_EVENT.TransferBlocked]        ="94 Transfer Blocked";
    EventDescription[SECS_EVENT.CassetteLoadComplete]   ="95 Cassette Load Complete";
    EventDescription[SECS_EVENT.CassetteIDReadComplete] ="96 Cassette ID Read Complete";
    EventDescription[SECS_EVENT.ReadyToProcessComplete] ="97 Ready To Process Complete";
    EventDescription[SECS_EVENT.ReadyToCarrierOutLot]   ="98 Ready To Carrier Out Lot";
    EventDescription[SECS_EVENT.CassetteOutComplete]    ="99 Cassette Out Complete";
    EventDescription[SECS_EVENT.CassetteUnclamped]      ="100 Cassette Unclamped";
    EventDescription[SECS_EVENT.ReadyToUnload]          ="101 Ready To Unload";
    EventDescription[SECS_EVENT.UnloadComplete]         ="102 Unload Complete";
    EventDescription[SECS_EVENT.ReadyToCarrierOutTray]  ="103 Ready To Carrier Out Tray";
    EventDescription[SECS_EVENT.ReadyToCombinePass]     ="104 Ready To Combine Pass";
    EventDescription[SECS_EVENT.ReadyToCombineFail]     ="105 Ready To Combine Fail";
    EventDescription[SECS_EVENT.MachineNoStart]         ="106 Machine No Start";
    EventDescription[SECS_EVENT.ReadyToCombinePassLotEnd]="107 Ready To Combine Pass Lot End";
    EventDescription[SECS_EVENT.DoCSTLotStart]          ="108 Cassette Lot Start";
    EventDescription[SECS_EVENT.DieCountFailMessageClose]="109 Die Count Fail Message Close";
    EventDescription[SECS_EVENT.CleanOutTrayFeedFinish] ="110 Clean Out Tray Feed Finish";
    EventDescription[SECS_EVENT.MapNoICArmAutoSkip]     ="111 Map No Device Arm Auto Skip";
    EventDescription[SECS_EVENT.MRRunModeChange]        ="112 MR Run Mode Change";
    EventDescription[SECS_EVENT.AccessModeChange]       ="113 Access Mode Change";
    EventDescription[SECS_EVENT.SoftwareBin]            ="114 Software Bin";
    EventDescription[SECS_EVENT.TrayIDChange]           ="115 Tray ID Change";  //wei 20180130
    EventDescription[SECS_EVENT.ReadyToLoadNoLot]       ="116 Ready To Load No Lot";
    EventDescription[SECS_EVENT.ReadyToLoadNoTray]      ="117 Ready To Load No Tray";
    EventDescription[SECS_EVENT.ReadyToLoadNoCassette]  ="118 Ready To Load No Cassette";
    EventDescription[SECS_EVENT.ART_SRQKIND2_FTLOTSTART]  ="119 ART SRQKIND2 FT LOTSTART";
    EventDescription[SECS_EVENT.ART_SRQKIND4_RTLOTSTART]  ="120 ART SRQKIND4 RT LOTSTART";
    EventDescription[SECS_EVENT.ART_SRQKIND8_LOTEND]      ="121 ART SRQKIND6 LOTEND";
    EventDescription[SECS_EVENT.ART_SRQKIND10_FINALLOTEND]="122 FINAL LOTEND";
    EventDescription[SECS_EVENT.SafeDoorOnOff]            ="123 Safe Door On Off";
    EventDescription[SECS_EVENT.SaveRecipe]               ="124 Save Recipe";   //JerryYang 20200527 新增Save Recipe Event

    EventDescription[SECS_EVENT.EESUGOffestSelect]        ="125 EESUG Offest Select";
    EventDescription[SECS_EVENT.EESUGOffestModify]        ="126 EESUG Offest Modify";
    EventDescription[SECS_EVENT.Backtonormal]             ="127 Back To Normal";
    EventDescription[SECS_EVENT.TestStart]                ="128 Test Start";
    EventDescription[SECS_EVENT.TestFinish]               ="129 Test Finish";
    EventDescription[SECS_EVENT.MaterialReceive]          ="130 Material Receive";

    EventDescription[SECS_EVENT.SlotMapCountOK]           ="131 Slot Map Count OK";                                     //KaiChen 20200716 ：OHT
    EventDescription[SECS_EVENT.CHECK_IN]                 ="132 CHECK IN";      //KaiChen 20200716 ：OHT
    EventDescription[SECS_EVENT.CHECK_OUT]                ="133 CHECK OUT";     //KaiChen 20200716 ：OHT
    EventDescription[SECS_EVENT.ReadyToCombineFailLotEnd] ="134 Ready To Combine Fail Lot End";                         //KaiChen 20210222 ：OHT
    EventDescription[SECS_EVENT.ReadyToOHTLotEnd]         ="135 Ready To OHT Lot End";                                  //KaiChen 20210303 ：OHT

    EventDescription[SECS_EVENT.Auto1Unloadtray]          ="136 Auto 1 Unloading tray";                                 //Steven 20210716 : Auto 退盤的事件
    EventDescription[SECS_EVENT.Auto2Unloadtray]          ="137 Auto 2 Unloading tray";                                 //Steven 20210716 : Auto 退盤的事件
    EventDescription[SECS_EVENT.Auto3Unloadtray]          ="138 Auto 3 Unloading tray";                                 //Steven 20210716 : Auto 退盤的事件
    EventDescription[SECS_EVENT.DoVisualSortLotStart]     ="139 Click lot start button for visual sort mode";           //JerryYang 20220927 : for SPIL Visual sorting Lot start
    EventDescription[SECS_EVENT.PreLoadTray]              ="140 Prepare Load Tray";                                     //KenHsieh 20220923 : add Tray Map Throw IC Function
    EventDescription[SECS_EVENT.GemControlStateChange]    ="141 GEM Control State Change";                              //Ifor 20221018 add:GEM Control State Change Report

    EventDescription[SECS_EVENT.Auto4Unloadtray]          ="145 Auto 4 Unloading tray";                                 //Steven 20230907 : For HT-9011UC
    EventDescription[SECS_EVENT.Auto5Unloadtray]          ="146 Auto 5 Unloading tray";
    EventDescription[SECS_EVENT.Auto6Unloadtray]          ="147 Auto 6 Unloading tray";
    EventDescription[SECS_EVENT.Auto4Full]                ="148 Auto 4 Full";
    EventDescription[SECS_EVENT.Auto5Full]                ="149 Auto 5 Full";
    EventDescription[SECS_EVENT.Auto6Full]                ="150 Auto 6 Full";
    EventDescription[SECS_EVENT.Fix4Full]                 ="151 Fix 4 Full";
    EventDescription[SECS_EVENT.Fix5Full]                 ="152 Fix 5 Full";
    EventDescription[SECS_EVENT.Fix6Full]                 ="153 Fix 6 Full";

    EventDescription[SECS_EVENT.LoadNoTray]               ="154 Loader hasn't tray";                                    //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Loader空盤事件
    EventDescription[SECS_EVENT.LoadFullTray]             ="155 Loader full of tray";                                   //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Loader滿盤事件
    EventDescription[SECS_EVENT.LoadOnlyOneTray]          ="156 Loader only one tray";                                  //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Loader剩一盤事件
    EventDescription[SECS_EVENT.Loader_ReadyToUnload]     ="157 Loader ready to unload";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Loader 退 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Loader_FinishUnload]      ="158 Loader finish unload";                                  //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Loader 退 TRAY 事件(結束)
    EventDescription[SECS_EVENT.Empty_PreLoadTray]        ="159 Empty Prepare Load Tray";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 入料事件
    EventDescription[SECS_EVENT.EmptyOnlyOneTray]         ="160 Empty only one tray";                                   //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 剩一盤事件
    EventDescription[SECS_EVENT.EmptyNoTray]              ="161 Empty hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 空盤事件
    EventDescription[SECS_EVENT.EmptyFullTray]            ="162 Empty full of tray";                                    //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 滿盤事件
    EventDescription[SECS_EVENT.Color_PreLoadTray]        ="163 Color Prepare Load Tray";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 入料事件
    EventDescription[SECS_EVENT.ColorOnlyOneTray]         ="164 Color only one tray";                                   //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 剩一盤事件
    EventDescription[SECS_EVENT.ColorNoTray]              ="165 Color hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 空盤事件
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto1]     ="166 Empty put tray to Auto1";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto1 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto2]     ="167 Empty put tray to Auto2";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto2 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto3]     ="168 Empty put tray to Auto3";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto3 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto4]     ="169 Empty put tray to Auto4";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto1 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto5]     ="170 Empty put tray to Auto5";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto2 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Empty_PutTrayToAuto6]     ="171 Empty put tray to Auto6";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto3 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto1]    ="172 Empty put cover to Auto1";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto1 上蓋事件
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto2]    ="173 Empty put cover to Auto2";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto2 上蓋事件
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto3]    ="174 Empty put cover to Auto3";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto3 上蓋事件
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto4]    ="175 Empty put cover to Auto4";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto1 上蓋事件
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto5]    ="176 Empty put cover to Auto5";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto2 上蓋事件
    EventDescription[SECS_EVENT.Empty_PutCoverToAuto6]    ="177 Empty put cover to Auto6";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 補Auto3 上蓋事件
    EventDescription[SECS_EVENT.Color_PutTrayToAuto1]     ="178 Color put tray to Auto1";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto1 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Color_PutTrayToAuto2]     ="179 Color put tray to Auto2";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto2 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Color_PutTrayToAuto3]     ="180 Color put tray to Auto3";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto3 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Color_PutTrayToAuto4]     ="181 Color put tray to Auto4";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto1 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Color_PutTrayToAuto5]     ="182 Color put tray to Auto5";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto2 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Color_PutTrayToAuto6]     ="183 Color put tray to Auto6";                               //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto3 TRAY 事件(開始)
    EventDescription[SECS_EVENT.Color_PutCoverToAuto1]    ="184 Color put cover to Auto1";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto1 上蓋事件
    EventDescription[SECS_EVENT.Color_PutCoverToAuto2]    ="185 Color put cover to Auto2";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto2 上蓋事件
    EventDescription[SECS_EVENT.Color_PutCoverToAuto3]    ="186 Color put cover to Auto3";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto3 上蓋事件
    EventDescription[SECS_EVENT.Color_PutCoverToAuto4]    ="187 Color put cover to Auto4";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto1 上蓋事件
    EventDescription[SECS_EVENT.Color_PutCoverToAuto5]    ="188 Color put cover to Auto5";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto2 上蓋事件
    EventDescription[SECS_EVENT.Color_PutCoverToAuto6]    ="189 Color put cover to Auto6";                              //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 補Auto3 上蓋事件
    EventDescription[SECS_EVENT.Auto1_LoadTrayFinish]     ="190 Auto1 load tray finish";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto1 定位TRAY事件(結束)
    EventDescription[SECS_EVENT.Auto2_LoadTrayFinish]     ="191 Auto2 load tray finish";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto2 定位TRAY事件(結束)
    EventDescription[SECS_EVENT.Auto3_LoadTrayFinish]     ="192 Auto3 load tray finish";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto3 定位TRAY事件(結束)
    EventDescription[SECS_EVENT.Auto4_LoadTrayFinish]     ="193 Auto4 load tray finish";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto1 定位TRAY事件(結束)
    EventDescription[SECS_EVENT.Auto5_LoadTrayFinish]     ="194 Auto5 load tray finish";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto2 定位TRAY事件(結束)
    EventDescription[SECS_EVENT.Auto6_LoadTrayFinish]     ="195 Auto6 load tray finish";                                //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto3 定位TRAY事件(結束)
    EventDescription[SECS_EVENT.Auto1_ReadyToUnload]      ="196 Auto1 ready to unload";                                 //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto1 退TRAY事件
    EventDescription[SECS_EVENT.Auto2_ReadyToUnload]      ="197 Auto2 ready to unload";                                 //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto2 退TRAY事件
    EventDescription[SECS_EVENT.Auto3_ReadyToUnload]      ="198 Auto3 ready to unload";                                 //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto3 退TRAY事件
    EventDescription[SECS_EVENT.Auto4_ReadyToUnload]      ="199 Auto4 ready to unload";                                 //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto1 退TRAY事件
    EventDescription[SECS_EVENT.Auto5_ReadyToUnload]      ="200 Auto5 ready to unload";                                 //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto2 退TRAY事件
    EventDescription[SECS_EVENT.Auto6_ReadyToUnload]      ="201 Auto6 ready to unload";                                 //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto3 退TRAY事件
    EventDescription[SECS_EVENT.Auto1NoTray]              ="202 Auto1 hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto1 空盤事件
    EventDescription[SECS_EVENT.Auto2NoTray]              ="203 Auto2 hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto2 空盤事件
    EventDescription[SECS_EVENT.Auto3NoTray]              ="204 Auto3 hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto3 空盤事件
    EventDescription[SECS_EVENT.Auto4NoTray]              ="205 Auto4 hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto1 空盤事件
    EventDescription[SECS_EVENT.Auto5NoTray]              ="206 Auto5 hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto2 空盤事件
    EventDescription[SECS_EVENT.Auto6NoTray]              ="207 Auto6 hasn't tray";                                     //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Auto3 空盤事件
    EventDescription[SECS_EVENT.ColorFullTray]            ="208 Color full of tray";                                    //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 滿盤事件
    EventDescription[SECS_EVENT.TrayEndFinish]            ="209 Tray End Finish";                                       //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Tray End Finish
    EventDescription[SECS_EVENT.Empty_FinishUnload]       ="210 Empty finish unload";                                   //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Empty 退Tray 事件(結束)
    EventDescription[SECS_EVENT.Color_FinishUnload]       ="211 Color finish unload";                                   //KenHsieh 20230918 : ASEKH_K1 & K3 事件新增_Color 退Tray 事件(結束)

    EventDescription[SECS_EVENT.PowerSavingStart]         ="212 Energy Saving Start";                                   //Ifor 20240402 add:SECS/GEM Power Saving Mode
    EventDescription[SECS_EVENT.PowerSavingEnd]           ="213 Energy Saving End";                                     //Ifor 20240402 add:SECS/GEM Power Saving Mode
    EventDescription[SECS_EVENT.Reserved_03]              ="214 Reserved_03";
    EventDescription[SECS_EVENT.Reserved_04]              ="215 Reserved_04";
    EventDescription[SECS_EVENT.Reserved_05]              ="216 Reserved_05";

    EventDescription[SECS_EVENT.LoadPortStatusChanged]    ="217 LoadPortStatusChanged";
    EventDescription[SECS_EVENT.EmptyPortStatusChanged ]  ="218 EmptyPortStatusChanged";
    EventDescription[SECS_EVENT.ColorPortStatusChanged]   ="219 ColorPortStatusChanged";
    EventDescription[SECS_EVENT.Auto1PortStatusChanged]   ="220 Auto1PortStatusChanged";
    EventDescription[SECS_EVENT.Auto2PortStatusChanged]   ="221 Auto2PortStatusChanged";
    EventDescription[SECS_EVENT.Auto3PortStatusChanged]   ="222 Auto3PortStatusChanged";
    EventDescription[SECS_EVENT.Fix1PortStatusChanged]    ="223 Fix1PortStatusChanged";
    EventDescription[SECS_EVENT.Fix2PortStatusChanged]    ="224 Fix2PortStatusChanged";
    EventDescription[SECS_EVENT.Fix3PortStatusChanged]    ="225 Fix3PortStatusChanged";
    EventDescription[SECS_EVENT.Auto4PortStatusChanged]   ="226 Auto4PortStatusChanged";
    EventDescription[SECS_EVENT.Auto5PortStatusChanged]   ="227 Auto5PortStatusChanged";
    EventDescription[SECS_EVENT.Auto6PortStatusChanged]   ="228 Auto6PortStatusChanged";
    EventDescription[SECS_EVENT.Fix4PortStatusChanged]    ="229 Fix4PortStatusChanged";
    EventDescription[SECS_EVENT.Fix5PortStatusChanged]    ="230 Fix5PortStatusChanged";
    EventDescription[SECS_EVENT.Fix6PortStatusChanged]    ="231 Fix6PortStatusChanged";
    EventDescription[SECS_EVENT.Reserved_21]              ="232 Reserved_21";
    EventDescription[SECS_EVENT.Reserved_22]              ="233 Reserved_22";
    EventDescription[SECS_EVENT.SafetyDoorOpen]           ="234 SafetyDoorOpen";
    EventDescription[SECS_EVENT.SafetyDoorClosed]         ="235 SafetyDoorClosed";
    EventDescription[SECS_EVENT.LoadPortBundleArrived]    ="236 LoadPortBundleArrived";
    EventDescription[SECS_EVENT.LoadPortBundleRead]       ="237 LoadPortBundleRead";
    EventDescription[SECS_EVENT.RemoteStart]              ="238 RemoteStart]";
    EventDescription[SECS_EVENT.UnexpectedBundleIDRead]   ="239 UnexpectedBundleIDRead";
    EventDescription[SECS_EVENT.UnexpectedUNITIDRead]     ="240 UnexpectedUNITIDRead";
    EventDescription[SECS_EVENT.BundleCompleteProcessed]  ="241 BundleCompleteProcessed";
    EventDescription[SECS_EVENT.BundleCompleteIDRead]     ="242 BundleCompleteIDRead";
    EventDescription[SECS_EVENT.NoCoverTray_2DID]         ="243 NoCoverTray_2DID";
    EventDescription[SECS_EVENT.NoCoverTray_Normal]       ="244 NoCoverTray_Normal";
    EventDescription[SECS_EVENT.BundleEnd_Auto1]          ="245 BundleEnd_Auto1";                                       //Auto1
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Auto1]   ="246 BundleEnd_IDREAD_Auto1";
    EventDescription[SECS_EVENT.BundleEnd_Auto2]          ="247 BundleEnd_Auto2";                                       //Auto2
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Auto2]   ="248 BundleEnd_IDREAD_Auto2";
    EventDescription[SECS_EVENT.ProcessEnd]               ="249 ProcessEnd";    //Process end
    EventDescription[SECS_EVENT.DoStartAutoHeight]        ="250 START Auto contact height";                             //Ifor 20240311 add: START Auto contact height
    EventDescription[SECS_EVENT.BundleEnd_Auto3]          ="251 BundleEnd_Auto3";                                       //Auto3
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Auto3]   ="252 BundleEnd_IDREAD_Auto3";
    EventDescription[SECS_EVENT.BundleEnd_Auto4]          ="253 BundleEnd_Auto4";                                       //Auto4
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Auto4]   ="254 BundleEnd_IDREAD_Auto4";
    EventDescription[SECS_EVENT.BundleEnd_Auto5]          ="255 BundleEnd_Auto5";                                       //Auto5
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Auto5]   ="256 BundleEnd_IDREAD_Auto5";
    EventDescription[SECS_EVENT.BundleEnd_Auto6]          ="257 BundleEnd_Auto6";                                       //Auto6
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Auto6]   ="258 BundleEnd_IDREAD_Auto6";
    EventDescription[SECS_EVENT.BundleEnd_Fix1]           ="259 BundleEnd_Fix1";                                        //Fix1
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Fix1]    ="260 BundleEnd_IDREAD_Fix1";
    EventDescription[SECS_EVENT.BundleEnd_Fix2]           ="261 BundleEnd_Fix2";                                        //Fix2
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Fix2]    ="262 BundleEnd_IDREAD_Fix2";
    EventDescription[SECS_EVENT.BundleEnd_Fix3]           ="263 BundleEnd_Fix3";                                        //Fix3
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Fix3]    ="264 BundleEnd_IDREAD_Fix3";
    EventDescription[SECS_EVENT.BundleEnd_Fix4]           ="265 BundleEnd_Fix4";                                        //Fix4
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Fix4]    ="266 BundleEnd_IDREAD_Fix4";
    EventDescription[SECS_EVENT.BundleEnd_Fix5]           ="267 BundleEnd_Fix5";                                        //Fix5
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Fix5]    ="268 BundleEnd_IDREAD_Fix5";
    EventDescription[SECS_EVENT.BundleEnd_Fix6]           ="269 BundleEnd_Fix6";                                        //Fix6
    EventDescription[SECS_EVENT.BundleEnd_IDREAD_Fix6]    ="270 BundleEnd_IDREAD_Fix6";
    EventDescription[SECS_EVENT.LoaderTrayState]            ="271 LoaderTrayState";
    EventDescription[SECS_EVENT.AGVSupplement]              ="272 AGVSupplement";
    EventDescription[SECS_EVENT.AGVLDUnLDStatus]            ="273 AGVLDUnLDStatus";
    EventDescription[SECS_EVENT.SECSGEMConsecutiveFailure]  ="274 SECSGEMConsecutiveFailure";
    EventDescription[SECS_EVENT.Loader_Buffer_HasTray]      ="275 Loader_Buffer_HasTray";                               //Sam 20250605 : Loader buffer status change event
    EventDescription[SECS_EVENT.Loader_Buffer_NoTray]       ="276 Loader_Buffer_NoTray";
    EventDescription[SECS_EVENT.OutputPort1BinCode]         ="277 OutputPort1BinCode";
    EventDescription[SECS_EVENT.OutputPort2BinCode]         ="278 OutputPort2BinCode";
    EventDescription[SECS_EVENT.OutputPort3BinCode]         ="279 OutputPort3BinCode";
    EventDescription[SECS_EVENT.OutputPort4BinCode]         ="280 OutputPort4BinCode";
    EventDescription[SECS_EVENT.OutputPort5BinCode]         ="281 OutputPort5BinCode";
    EventDescription[SECS_EVENT.OutputPort6BinCode]         ="282 OutputPort6BinCode";
    EventDescription[SECS_EVENT.MaterialModeChange]         ="283 MaterialModeChange";
    EventDescription[SECS_EVENT.PortStateUpdated]           ="284 PortStateUpdated";
    EventDescription[SECS_EVENT.UnloaderTrayIDReadOK]       ="285 UnloaderTrayIDReadOK";
    EventDescription[SECS_EVENT.UnloaderTrayIDReadFail]     ="286 UnloaderTrayIDReadFail";
    EventDescription[SECS_EVENT.LoaderTrayIDReadFail]       ="287 LoaderTrayIDReadFail";
    EventDescription[SECS_EVENT.MaximumOutputPortReport]    ="288 MaximumOutputPortReport";
    EventDescription[SECS_EVENT.RunCheckRequest]            ="289 RunCheckRequest";//Eastsun 20260515 F018
    EventDescription[SECS_EVENT.AGVLDUnLDFinish]            ="290 AGVLDUnLDFinish";//Eastsun 20260512 F009 KYEC AMR LDUnLD Finish
    EventDescription[SECS_EVENT.AGVLdID]                    ="291 AGVLdID";//Eastsun 20260513 F016 KYEC AMR cover-tray ID
    EventDescription[SECS_EVENT.DoSecsGemIndexFail]         ="292 DoSecsGemIndexFail";//Eastsun 20260515 F018 KYEC Index Fail
};
#endif // GOLDEN VERBATIM -- golden SECSGEM/uHGemHT9045.cpp:66-369  (GATE G-PTW6c-HT9045Gem, end)
HT9045Gem::HT9045Gem(AnsiString Path, THGem *HGemTmp)
{
    HGemPtr=HGemTmp;
    HandlerPath=Path;
    // AI(W906-SysModWire) 20260720: PORT-ONLY -- bind the receive/send codec
    // to the live THGem engine when one is supplied (golden's single-codec
    // reality; golden re-points HGemPtr the same way in AddSV/AddEC,
    // uHGemHT9045_SV.cpp:61 / _EC.cpp:54, once the real override layer
    // lands). NULL-tolerant: golden's own static-init call site passes a
    // still-NULL HGem (SYSTEM_MODULAR is a global -> its ctor runs during
    // static init, before THGem HGem is ever assigned -- see design brief
    // §1.1) -- ProcessReceiceData's dispatch entry re-binds ActiveWire
    // defensively on every dispatch (see that method's own comment) so this
    // is not the only re-bind point, just the earliest opportunistic one.
    if (HGemTmp != NULL)
        ActiveWire = &HGemTmp->WireCodec;
}

//==============================================================================
//  APPEND BLOCK -- ROLE: HT9045Gem's registration / override group
//  (the small siblings of AddSV/AddEC).  APPEND-ONLY: nothing above this line
//  was read-modified-written by this wave; three sibling agents are appending
//  their own groups to this same file concurrently.
//
//  Translator: AI(W906-PT-W5-registration) 20260809
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemHT9045.cpp,
//                 decoded cp950.  Every Chinese comment below is TRANSCRIBED
//                 verbatim from golden (lifted byte-for-byte by script, not
//                 retyped), never paraphrased.
//
//  WAVE SCOPE -- one line per golden function
//    * HT9045Gem::ReloadParameter   golden :371-380 (10 lines)  ACTIVE  + gate [R1]
//    * HT9045Gem::AddAlarmList      golden :382-435 (54 lines)  ACTIVE  + gate [R2]
//    * HT9045Gem::AddCEID           golden :437-448 (12 lines)  ACTIVE  (no gate)
//    * HT9045Gem::AddReprot         golden :450-464 (15 lines)  ACTIVE  (no gate)
//                                   golden misspells "Report" -- KEPT AS IS
//    * HT9045Gem::LookForFile       golden :466-472 ( 7 lines)  ACTIVE  (no gate)
//
//  GATE REGISTER
//  ---------------------------------------------------------------------------
//  [R1] golden :373-374 -- ReloadParameter's leading
//       `if(CUSTOMER_CODE==CC_ASE_KaohSiung) SaveAllFile(GetLastOpenFN());`
//       WHY THE DEFAULT IS FAITHFUL: it is not a choice.  `SaveAllFile` is
//       DECLARED at csystem.h:268 and has NO DEFINITION anywhere in this tree
//       (grep + timestamp in this wave's report; docs/
//       W7_UI_ARCHITECTURE_PLAN.md:354 independently states the same).
//       Calling it is a LINK error that `-fsyntax-only` cannot see.  Golden's
//       body lives in csystem.cpp, which is registered in `ht9045_sm`, and
//       ht9045_secsgem must NOT link ht9045_sm (the documented CMake cycle,
//       CMakeLists.txt:602-649) -- so even a future csystem.cpp translation
//       does not by itself make this call reachable from this archive.
//       `GetLastOpenFN()` is NOT the problem: it is real (common.cpp:1235,
//       archive ht9045_core, which ht9045_secsgem does link).  It is inside
//       the gate only because it is the argument of the un-linkable call.
//       BEHAVIOUR DELTA ON A REAL MACHINE: confined to CUSTOMER_CODE ==
//       CC_ASE_KaohSiung (936, MachineType.h:308).  On an ASE-Kaohsiung
//       machine golden persists the ENTIRE current recipe to disk (SaveAllFile
//       of the last-opened recipe name) every time the host makes the handler
//       reload parameters -- e.g. right after an accepted S2F15 equipment-
//       constant change -- BEFORE refreshing the four UI pictures.  This port
//       refreshes the pictures and writes nothing, so on that one customer an
//       accepted host parameter change is lost at the next power cycle
//       (RAM-only).  For every other CUSTOMER_CODE the branch never runs, so
//       the delta is exactly zero -- including in this port as it stands,
//       whose CUSTOMER_CODE default is 0 (cmydef.cpp:3412).
//
//  [R2] golden :403 -- AddAlarmList's `HGem->strGrdAlarm->ColWidths[7]=50;`
//       WHY THE DEFAULT IS FAITHFUL: vclcompat::TStringGrid deliberately has
//       NO ColWidths[] member at all -- "NO rendering, NO FixedRows/FixedCols/
//       Options/OnDrawCell/OnSelectCell/ColWidths/RowHeights"
//       (vclcompat/StringGrid.h:19-20).  That is a declared out-of-scope of
//       the widget shim, not a missing translation, and this tree ALREADY
//       gates golden's identical column-width cosmetics the same way at
//       cMyDB.cpp:1657 -- inside GetAlarmCodeList, the function golden calls
//       on the immediately preceding line (:402).
//       BEHAVIOUR DELTA ON A REAL MACHINE: cosmetic only.  The "Enable"
//       column of the uHGemEquipment alarm StringGrid renders at the grid's
//       default width instead of 50 px.  No cell CONTENT changes (:404's
//       `Cells[7][0]="Enable"` is ACTIVE), no AlarmData.def byte changes, no
//       SECS wire byte changes.  In this port there is no rendering at all,
//       so the delta only becomes visible if a real VCL grid is ever put back.
//
//  NO OTHER GATE IN THIS GROUP.  Every external body this group calls is real
//  and LIVE, and each was checked to be OUTSIDE every `#if 0` region rather
//  than merely present in the file:
//    * SECSGEM/uHGemEquipment.cpp (SAME archive, ht9045_secsgem):
//        THGem::SetAlamData :1051, ReadAlamData :1066, WriteAlamData :1138,
//        SetCEIDContent :2267, SetReportIDContent :2373,
//        ReadEventReportData :3010, SaveEventReportData :3048
//    * cMyDB.cpp:1653  GetAlarmCodeList(TStringGrid*)  -- archive ht9045_db,
//        which ht9045_secsgem links (CMakeLists.txt:1214).
//    * forms/fMain.cpp  LoadTestModePicture :287 / LoadRunModePicture :335 /
//        UpdateMainOperateMode :338 / LoadStartModePicture :339 /
//        LookForFile :340, and the `cbSetupFileName` TComboBox stand-in --
//        archive ht9045_forms, which ht9045_secsgem links (same line).
//        `fMain` itself is a live object (`TfMain *fMain = new TfMain();`,
//        forms/fMain.cpp:385), NOT one of the 18 NULL globals in
//        docs/PT_CAMPAIGN_PLAN.md section 8.
//    * common.cpp:1235 GetLastOpenFN -- archive ht9045_core.  Referenced only
//      from inside gate [R1]'s `#if 0` arm, so it creates no link edge today.
//
//  EXPIRED ABSENCE CLAIM FOUND IN THE TREE -- for the main loop.
//  docs/W7_UI_ARCHITECTURE_PLAN.md:354 still says AddAlarmList is blocked by
//  "Wall 3" because `GetAlarmCodeList` is "全樹未翻" (untranslated tree-wide).
//  THAT CLAIM EXPIRED ON 20260807: cMyDB.cpp was translated earlier and was
//  finally registered by PT-W2 (CMakeLists.txt:805-818), and cMyDB.cpp:1653 is
//  a live body.  AddAlarmList is therefore NOT blocked by Wall 3.  That plan
//  line ALSO still names `SaveAllFile` as "已宣告無定義" (declared, undefined)
//  -- that half is STILL TRUE and is exactly gate [R1].
//
//  CALLER PRECONDITION -- THE ONE RUNTIME HAZARD.  It is NOT a translation
//  defect and is deliberately NOT papered over with a guard.  AddAlarmList
//  and AddReprot dereference the global `HGem`; AddAlarmList, AddCEID and
//  AddReprot dereference the inherited `HGemPtr`.  Golden's own call sites run
//  long after both are live.  In this port `THGem *HGem = NULL;`
//  (SECSGEM/uHGemEquipment.cpp:3521) and stays NULL until the still-unported
//  main.cpp constructs the engine, and `HGemPtr` is whatever database.cpp:187
//  handed the ctor -- i.e. that same NULL.  SO: DO NOT CALL AddAlarmList /
//  AddCEID / AddReprot until `HGem` is a live THGem.
//
//  CALL-SITE CENSUS -- CORRECTED 20260809 16:34 (the first version of this
//  paragraph said "nothing in the tree calls any of these five today"; that
//  was true at 16:10 and had EXPIRED by 16:34, because sibling append-blocks
//  landed into THIS SAME FILE while this block was being written.  Recorded
//  here rather than quietly edited, because this is the campaign's
//  absence-claims-expire failure mode firing in real time.)
//    * AddAlarmList / AddCEID / AddReprot -- still ZERO call sites of any
//      kind.  tests/test_uHGemClass.cpp:178 calls all three, but on a plain
//      base `HTGem g;`, NOT on an HT9045Gem, so it keeps hitting
//      uHGemClass.h:229-231's inline `{}` and is unaffected by these
//      overrides.  The NULL-deref hazard above is therefore still latent, not
//      live.
//    * ReloadParameter -- HAS A LIVE CALLER, and it is not new code:
//      SECSGEM/uHGemClass.cpp:4263, the tail of the already-un-gated
//      `HTGem::S2F15_UpdateNewEquipmentConstant()`.  That is a virtual call,
//      so on an HT9045Gem it now reaches THIS body where it previously hit
//      uHGemClass.h:232's inline `{}`.  Safe: ReloadParameter touches neither
//      HGem nor HGemPtr, only `fMain`, which is live
//      (forms/fMain.cpp:385).  This is also the exact host path gate [R1]'s
//      behaviour delta describes -- an accepted S2F15 equipment-constant
//      change.  (Note golden ALSO overrides S2F15_UpdateNewEquipmentConstant
//      at golden uHGemHT9045.h:352; a sibling wave is landing that override
//      into this file, which changes WHICH S2F15 body reaches
//      ReloadParameter, not the delta itself.)
//    * LookForFile -- HAS FOUR LIVE CALLERS, all inside this same file, all
//      added by the sibling "W906-uHGemHT9045-S7program" append-block after
//      this block's own grep was run.  Safe for the same reason
//      (fMain-only).
//  A NULL guard was
//  considered and REJECTED: it would invent a control-flow path golden does
//  not have in three of the five bodies, and it would convert a loud wiring
//  bug into a silent "zero alarms / zero CEIDs / zero reports registered".
//  The sibling SECSGEM/uHGemHT9045_SV.cpp:416-425 DID add such a guard, but
//  only because its free-function form was already rewriting golden's own
//  `HGemPtr=HGem;` first line -- these three bodies have no such line to hang
//  a guard on.  ReloadParameter and LookForFile touch neither pointer and are
//  safe to call today.
//
//  DEPENDENCY DELTA -- EventDescription[].  AddCEID passes
//  `EventDescription[i]` as SetCEIDContent's `CeidAlias`.  Golden declares
//  that member at uHGemHT9045.h:342 and SEEDS all ~292 entries in its ctor
//  (golden uHGemHT9045.cpp:71-368); this port's ctor deliberately does not
//  (see this file's own head note).  This wave adds the MEMBER -- it is a
//  declaration AddCEID needs, added at golden's own relative position -- but
//  NOT the seeding, because the ctor is another bucket's scope.
//  BEHAVIOUR DELTA ON A REAL MACHINE: every CEID is still registered with the
//  correct CEID number, the correct single ReportID and the correct
//  Mode/EquDefault, so S2F37 enable / S6F11 event reporting are unaffected;
//  only the human-readable alias is "" instead of e.g. "1 DoStart", which
//  shows up as empty CEID names in an S1F24 CollectionEventNamelist reply and
//  in THGem's strGrdCEID alias column.  Retiring it is one edit in the ctor,
//  not here.
//
//  GOLDEN BUGS PRESERVED, NOT FIXED
//  (a) AddReprot golden :457-460 -- the loop runs `i` over the whole CEID
//      range but the call inside passes the LITERAL 1, never `i`:
//      `SetReportIDContent(1, ...)`.  So golden re-defines ReportID 1 some
//      292 times instead of one ReportID per event.  KEPT verbatim.
//  (b) AddCEID golden :445 -- the trailing comment ("Ceid=1 ,有 1個 ReportID
//      ,分別是 1") no longer describes the code beside it (the CEID is `i`).
//      KEPT verbatim.
//  (c) AddAlarmList golden :409/:413/:415 -- `AlarmCount` is latched from
//      `->RowCount` AFTER GetAlarmCodeList resized the grid, but BEFORE
//      `HGem->ReadAlamData()` which can resize it again; the :415 loop then
//      runs to the stale `AlarmCount`.  KEPT verbatim.  NOTE the port-only
//      consequence (AI(W906-W3-6b) 20260925: GONE -- vclcompat now follows BCB6 grids.pas, which is {$R-}: an
//      out-of-range Cells[][] reads "" / stores silently, exactly like golden; the text below is history)
//      (vclcompat/StringGrid.h:43-50, a deliberate design decision of that
//      shim), so if that stale bound ever over-runs, this port terminates
//      where golden would have shown a VCL exception dialog.
//==============================================================================

// Includes for this block only, appended rather than merged into the file head
// so that no existing line is touched (three sibling agents are appending to
// this same file).  All are include-guarded, so a sibling block repeating any
// of them is a no-op.
#include "MachineDefine.h"                  // golden :1 (verbatim); golden's `#pragma hdrstop` dropped
#include "MachineType.h"                    // golden :6  -- CC_ASE_KaohSiung (:308)
#include "cmydef.h"                         // golden :24 -- CUSTOMER_CODE
#include "cMyDB.h"                          // golden :22 -- GetAlarmCodeList(TStringGrid*)
#include "CosFunction.h"                    // PORT: CosFunction.bEnable_SECS_GEM (golden reaches
                                            //   `CosFunction` transitively via its cprod.h/cmydef.h chain)
#include "SECSGEM/SecsEventType.h"          // PORT: SECS_EVENT -- extracted out of golden's own
                                            //   uHGemHT9045.h into a standalone unit in this port
#include "FormsFacade.h"                    // PORT equivalent of golden :8 "main.h" -- fMain

//------------------------------------------------------------------------------
void HT9045Gem::ReloadParameter()
{
// GATE [R1] -- golden :373-374.  `SaveAllFile` is declared (csystem.h:268) and
// defined NOWHERE in this tree, and its golden home csystem.cpp sits in ht9045_sm,
// which ht9045_secsgem may not link.  Golden text kept verbatim below.  Retirement
// is one edit once SaveAllFile has a body reachable from this archive: flip the arms.
#if 0
    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 2018905 add
        SaveAllFile(GetLastOpenFN());                                           //kevin 20181114
#else
    // R1 DEFAULT: both golden lines skipped.  Delta is ASE-Kaohsiung-only (see
    // the GATE REGISTER above); no state this port can observe is changed.
#endif

    fMain->LoadTestModePicture();
    fMain->UpdateMainOperateMode();
    fMain->LoadRunModePicture();
    fMain->LoadStartModePicture();
}
//------------------------------------------------------------------------------
void HT9045Gem::AddAlarmList()
{
    if(CosFunction.bEnable_SECS_GEM==true)                                      //Ifor 20220719 add 有SECS GEM 功能即載入資料避免後續開啟造成異常
    {
        AnsiString Code;
        int AlarmCount;

//Steven 20200331 : Alarm code list改用文字檔
//==>
//        asQuery="SELECT  ID_AlarmList,      "
//                "        AlarmCode,         "
//                "        UnitNo,            "
//                "        UnitName.UnitName, "
//                "        Type,              "
//                "        Message            "
//                "FROM AlarmList             "
//                "INNER JOIN UnitName ON     "
//                "(AlarmList.UnitNo = UnitName.ID_UnitNo)";
//        MyDBVProcess(asQuery, HGem->strGrdAlarm);

        GetAlarmCodeList(HGem->strGrdAlarm);
        // AI(W906-VclGrid-1) 20260820: GATE [R2] RETIRED -- same ground as
        // cMyDB.cpp's GetAlarmCodeList flip: the grid consolidation gave the
        // base class the ColWidths proxy, so golden :403 is live again.
        HGem->strGrdAlarm->ColWidths[7]=50;                                     //Enable
        HGem->strGrdAlarm->Cells[7][0]="Enable";
        HGem->strGrdAlarm->Cells[8][0]="AlarmID";
        HGem->strGrdAlarm->Cells[9][0]="Class";
        HGem->strGrdAlarm->Cells[10][0]="Position";

        AlarmCount=HGem->strGrdAlarm->RowCount;
//<==
//Steven 20200331 : Alarm code list改用文字檔

        HGem->ReadAlamData();

        for(int i=1; i<AlarmCount; i++)                                         //20140213  wei   KYEC SECS/GEM
        {
            int iType, iPos;
            int iAlarmCode;

            iType=atoi(HGem->strGrdAlarm->Cells[5][i].c_str());
            if(iType<0)
                iType=0;

            iPos=atoi(HGem->strGrdAlarm->Cells[3][i].c_str());
            iAlarmCode=atoi(HGem->strGrdAlarm->Cells[2][i].SubString(4, HGem->strGrdAlarm->Cells[2][i].Length()).c_str());

            Code.sprintf("%d%02d%06d", iType, iPos, iAlarmCode);

            HGem->SetAlamData(i, Code, AnsiString(iType), HGem->strGrdAlarm->Cells[6][i], HGem->strGrdAlarm->Cells[4][i]);
        }

        HGemPtr->WriteAlamData();
        HGemPtr->ReadEventReportData();
    }
}
//------------------------------------------------------------------------------
void HT9045Gem::AddCEID()
{
    if(CosFunction.bEnable_SECS_GEM==true)                                      //Ifor 20220719 add 有SECS GEM 功能即載入資料避免後續開啟造成異常
    {
        int EquDefault=1;
        for(int i=SECS_EVENT.DoStart; i<SECS_EVENT.TotalEvent; i++)
        {
            unsigned CEDIDContent[]={i};
            HGemPtr->SetCEIDContent(i, EventDescription[i], sizeof(CEDIDContent)/sizeof(unsigned), CEDIDContent, EquDefault);                                   // Ceid=1 ,有 1個 ReportID ,分別是 1
        }
    }
}
//------------------------------------------------------------------------------
void HT9045Gem::AddReprot()
{
    if(CosFunction.bEnable_SECS_GEM==true)                                      //Ifor 20220719 add 有SECS GEM 功能即載入資料避免後續開啟造成異常
    {
        int EquDefault=1;
        unsigned ReportIDContent[]={1027};                                      //System Time

        for(int i=SECS_EVENT.DoStart; i<SECS_EVENT.TotalEvent; i++)
        {
            HGemPtr->SetReportIDContent(1, sizeof(ReportIDContent)/sizeof(unsigned), ReportIDContent, EquDefault);
        }

        HGem->SaveEventReportData();
    }
}
//------------------------------------------------------------------------------
void HT9045Gem::LookForFile()
{
    AnsiString Str=fMain->cbSetupFileName->Text;
    fMain->cbSetupFileName->Clear();
    fMain->LookForFile();
    fMain->cbSetupFileName->Text=Str;
}
//------------------------------------------------------------------------------
//  END of the PT-W5 "registration" append block.
//------------------------------------------------------------------------------
//===========================================================================
//  WAVE: W906-uHGemHT9045-S7program   Translator: AI(W906-uHGemHT9045-S7program) 20260809
//  ROLE: golden's SECS Stream-7 process-program (recipe) family on HT9045Gem --
//        the host<->handler recipe conversation: load grant (S7F2), recipe
//        download/install (S7F4), recipe upload (S7F6), and the four
//        formatted-process-program entry points (S7F23/24/25/26).
//
//  WAVE SCOPE (one line per golden function)
//  ---------------------------------------------------------------------------
//    HT9045Gem::S7F2_ProcessProgramLoadGrant                    golden :4397   GATED (whole body, degraded ACTIVE arm)
//    HT9045Gem::S7F4_ProcessProgramAcknowledge                  golden :4478   GATED (whole body, degraded ACTIVE arm)
//    HT9045Gem::S7F6_ProcessProgramData                         golden :5326   GATED (whole body, degraded ACTIVE arm)
//    HT9045Gem::ProcessS7F23FromatReceipe                       golden :5826   ACTIVE (fully faithful)
//    HT9045Gem::S7F24_FormattedProcessProgramSendAcknowledge     golden :5929   ACTIVE (fully faithful)
//    HT9045Gem::ProcessS7F25FromatReceipe                       golden :5943   ACTIVE (fully faithful)
//    HT9045Gem::S7F26_FormattedProcessProgramData               golden :6025   ACTIVE (fully faithful)
//
//  NOT IN THIS WAVE: the SECOND overload
//  `S7F6_ProcessProgramData(AnsiString FileName)` (golden :5605-5825, "Steven
//  20210510 : S7F6 upload work-file"). Golden declares both (uHGemHT9045.h:357
//  and :358); only the no-arg one is this group's assignment, so HTGem's own
//  inline `{}` (uHGemClass.h:279) keeps serving the 1-arg form. Do not read this
//  file's coverage as "S7F6 done".
//
//  GATE REGISTER
//  ---------------------------------------------------------------------------
//  Every gate below is a WHOLE-BODY gate: the `#if 0` arm carries golden's text
//  byte-for-byte (extracted from the cp950 golden, not retyped), the `#else` arm
//  is the ACTIVE degraded body. Sub-deltas inside each ACTIVE arm are numbered
//  and cited inline at the exact statement they replace.
//
//  [G1] golden :4397-4476  S7F2_ProcessProgramLoadGrant -- whole body.
//       WHY NOT FAITHFUL: two blockers, both inside the HCACK ladder.
//         G1a golden :4414-4417 / :4427-4430 -- `TRegExpr` (the Borland
//             TRegExpr component) has NO home anywhere in this ported tree
//             (absence claim A1). SUBSTITUTED, not dropped: the ACTIVE arm scans
//             PPID for the identical literal character set golden's pattern
//             encodes, so the HCACK=8 decision is preserved. The derivation is
//             spelled out at the ACTIVE statement so it can be checked by eye.
//             DELTA on a real machine: none for this fixed pattern. Two
//             incidental differences are disclosed at the statement (golden
//             leaks the TRegExpr object; golden only evaluates the pattern once
//             control reaches the third rung, the ACTIVE arm scans
//             unconditionally -- the scan has no side effects so the HCACK
//             actually sent is unchanged).
//         G1b golden :4431-4466 -- the nine machine-state rungs (HCACK 9..17)
//             read MOT[] (Motor/mymotor.h:385, defined Motor/mymotor.cpp ->
//             library ht9045_motor), InArmSuck/OutArmSuck/FLCarryKit/
//             BLCarryKit/FRCarryKit/BRCarryKit (aHotPlateSubstrate.h, defined
//             aHotPlateSubstrate.cpp -> ht9045_sm) and IndexHasIC() (csystem.h
//             :140, defined csystem_predicates.cpp -> ht9045_sm). Those symbols
//             ALL EXIST -- this is NOT an absence claim -- but pulling them in
//             would (i) add ht9045_secsgem -> ht9045_motor object-level
//             undefined references that no consumer of this archive declares
//             today, and (ii) force this TU to choose between the TWO headers
//             that declare the same suck globals with DIFFERENT layouts
//             (aHotPlateSubstrate.h vs mykitsuck.h; mykitsuck.cpp is
//             deliberately unregistered -- CMakeLists.txt:2099 -- so picking the
//             wrong one links clean and reads the wrong offsets). Left GATED.
//             DELTA on a real machine: SAFETY-RELEVANT. Golden REFUSES the
//             recipe-load grant (HCACK 9..17) while an IC or a tray is still
//             held by the tray arm / either hot plate / the in-arm suck / the
//             out-arm suck / either in-shuttle kit / either out-shuttle kit /
//             the index / the in-rotate station / the out-rotate station. The
//             ACTIVE arm cannot see any of that and falls through to HCACK=0
//             (GRANTED), so a host can be told it may load a recipe with
//             material still in the machine. The two rungs that survive
//             (SystemStart -> 6, empty PPID -> 7) are the reachable ones.
//
//  [G2] golden :4478-5321  S7F4_ProcessProgramAcknowledge -- whole body.
//       WHY NOT FAITHFUL: this is the recipe INSTALLER. Its body writes into
//       hard-coded absolute machine paths (D:\HT9045\IniData\Data\<PPID> at
//       golden :4525, D:\HT9045\IniData\Offset\<PPID> at :4749), shells out to
//       d:\HT9045\7z.exe (:4869, :4894), and rewrites the live
//       Temperature/Contact/HotPlate/UdUld/ArmCondition/HandlerCondition .Data
//       files of the CURRENT work-file (:4982-5292). That is exactly the gate
//       this campaign keeps closed. On top of that it needs facade members with
//       no ported home:
//         G2a fOffSet->GetOffsetPath()   golden :4553 / :4558 / :4562 / :4566
//         G2b fLotInfo->coLevelMode->Text golden :4961 ; fLotInfo->
//             ClearAllSetupFile() golden :5306
//         G2c fSetup->bFirstTime          golden :5298 / :5300
//       -- see the ABSENCE CLAIMS block. Adding them is outside this wave's
//       write boundary (forms/*.h is not this agent's to edit).
//       ACTIVE ARM: golden's OUTER wire parse is kept in shape (L,2 -> PPID
//       ASCII -> PPBODY ASCII|BINARY, including golden's own `new char [len+1]`
//       / `delete[]` pair), the install is skipped, and the reply is golden's
//       OWN "could not store the file" acknowledge -- LocalAcknowledge(7, 4, 1),
//       ACKC7=1 -- lifted from golden's own fopen-failure path at :4858. It is
//       deliberately NOT golden's success ack at :5315 (ACKC7=0): nothing was
//       stored and claiming 0 would misinform the host. The malformed-message
//       path (S9F7_IllegalData("S7,F3 Data Format error !!!"), golden :5320) is
//       preserved verbatim.
//       DELTA on a real machine: a host recipe download is parsed and then
//       REJECTED with ACKC7=1 instead of installed. No directory is created, no
//       .Data file is rewritten, no 7z.exe is launched, LastSet.bHasDownloadFile
//       is not set, and bRefreshFunction / LookForFile() are not run.
//       G2d: golden :4506-4515's CUSTOMER_CODE==CC_MAXIM_THAILAND read-order
//       variant (that host wraps PPID in one extra L) is also gated; the ACTIVE
//       arm always takes golden's `else` arm, so an Analog/Maxim Thailand host's
//       S7F3 would misparse. Moot while the install is gated, recorded because
//       it is a real difference.
//
//  [G3] golden :5326-5603  S7F6_ProcessProgramData() -- whole body.
//       WHY NOT FAITHFUL: two independent blockers.
//         G3a THGem has no `FileListBox3` and no `memoPPBody` member (golden
//             :5405 / :5406 / :5415-5447 / :5473). Both are real golden
//             __published widgets on the SECS form; the ported partial THGem
//             models FileListBox1 and FileListBox2 only (absence claim A2).
//         G3b the body zips the live work-file directory to <DataPath><PPID>.zip
//             via d:\HT9045\7z.exe (:5528-5536), copies
//             "C:\Program Files\7-Zip\7z.exe" onto d:\HT9045\7z.exe when absent
//             (:5530), and DeleteFile()s under DataPath (:5524 / :5592) --
//             machine-tree writes, gate stays closed.
//       ACTIVE ARM: golden's PPID read and its DirectoryExists guard are kept
//       (READ-only, including golden's hard-coded D:\HT9045\IniData\Data\%s --
//       reported), then the reply is golden's OWN "cannot produce the file"
//       form: InitLocalHead(7,6,0); DataItemOut(0, HType.LIST_TYPE, NULL);
//       SendLocalData(); -- lifted from golden's own open()==-1 path at
//       :5563-5566, i.e. an S7F6 with an empty L,0 body = "no process program".
//       Golden's two TStringList leaks on the early-return paths (TempString2 is
//       new'ed at :5338 and NOT deleted before the :5350 return, nor before the
//       :5602 S9F7 fall-through) are PRESERVED, not fixed -- they are golden
//       bugs and this is a translation, not a cleanup.
//       DELTA on a real machine: a host recipe-upload request receives an empty
//       S7F6 (no recipe bytes) instead of the packed work-file, and the
//       IniConfig.bN07_6EnableUploadOSRecipe OS-tester side copy never runs.
//
//  NO GATE: ProcessS7F23FromatReceipe / ProcessS7F25FromatReceipe are almost
//  entirely commented out IN GOLDEN ITSELF (golden :5844-5925 and :5961-6021 are
//  one /* */ block each; :5828-5838 and :5945-5960 are `//` lines). Their live
//  code is 4 statements and 1 statement respectively and needs nothing this TU
//  lacks, so both are ACTIVE and faithful, golden's dead text carried over
//  verbatim. S7F24 / S7F26 are 13 lines each and likewise fully faithful.
//
//  ABSENCE CLAIMS (re-run at hand-off -- see this wave's report for timestamps)
//  ---------------------------------------------------------------------------
//   A1  TRegExpr has no home in this tree.
//       grep -rn "TRegExpr" --include=*.h --include=*.cpp . | grep -v "^./build"
//       -> only this block's own gate text.
//   A2  THGem has no FileListBox3 / memoPPBody member.
//       grep -n "FileListBox3\|memoPPBody" SECSGEM/uHGemEquipment.h  -> 0 hits
//   A3  TfOffSet has no GetOffsetPath.
//       grep -n "GetOffsetPath" forms/fOffSet.h                      -> 0 hits
//   A4  TfLotInfo has no coLevelMode and no ClearAllSetupFile.
//       grep -n "coLevelMode\|ClearAllSetupFile" forms/fLotInfo.h    -> 0 hits
//   A5  TfSetup has no bFirstTime.
//       grep -n "bFirstTime" forms/fSetup.h                          -> 0 hits
//
//  MECHANICAL RENAME (this file family's established Design-D idiom, see
//  uHGemClass.h's ActiveWire member and uHGemClass.cpp:365-368): every golden
//  `HGemPtr->DataItemIn / GetDataItemLenAndType /
//  GetDataItemLenAndTypeAndDelete / InitLocalHead / DataItemOut / SendLocalData
//  / LocalAcknowledge / StringOut` becomes `ActiveWire->...`. Direct precedent
//  for this exact S,F family: HTGem::S7F2_ProcessProgramLoadGrant,
//  uHGemClass.cpp:3032-3059. ActiveWire is never NULL (HTGem's ctor points it at
//  its own by-value WireCodec), which also keeps these bodies safe while
//  HGemPtr is still NULL.
//===========================================================================

// AI(W906-uHGemHT9045-S7program) 20260809: `HSys` / SYSTEM_MODULAR::MyGem --
// golden S7F26_FormattedProcessProgramData:6028 calls
// `HSys.MyGem->ProcessS7F25FromatReceipe()`. database.h is the ONE new include
// this group needs; it is already a proven include in this same library
// (SECSGEM/uHGemEquipment.cpp includes it for the identical HSys.MyGem seam --
// see the ht9045_secsgem -> ht9045_db edge, CMakeLists.txt:1214), so it adds no
// link edge. Placed HERE rather than in the file-head include block because
// three sibling agents are appending to this same file concurrently and that
// block is not this agent's to rewrite.
#include "database.h"

// AI(W906-uHGemHT9045-S7program) 20260809: two file-scope declarations instead
// of pulling cmydef.h + csystem.h into this TU. Both are copied EXACTLY from the
// headers that own them, so they cannot drift in type:
//     cmydef.h:221   extern bool SystemStart;
//     csystem.h:105  bool HasICUnderMachine();
// Same local-declaration idiom uHGemClass.cpp already uses for MyDBIProcess, and
// that aoutarm9045_1x2_2.cpp:176 uses for HasICUnderMachine itself. cmydef.h WAS
// evaluated as a full include (it would have made golden's
// CUSTOMER_CODE==CC_MAXIM_THAILAND branch in S7F4 faithful) and DECLINED: that
// branch sits inside a body whose whole reason for being gated is the
// machine-path writes, so the include would widen this TU's surface for a
// difference that is unreachable anyway. Recorded as gate G2d.
// LINK NOTE: HasICUnderMachine() is defined in csystem_predicates.cpp
// (ht9045_sm). This is NOT a new edge for this archive -- uHGemClass.cpp (same
// library) already calls it bare from
// HTGem::S2F16_NewEquipmentConstantSendAcknowledge (uHGemClass.cpp:1639/:1647),
// and every target that extracts a uHGemHT9045/uHGemClass object does so inside
// a $<LINK_GROUP:RESCAN,...> that already lists ht9045_sm.
extern bool SystemStart;
bool HasICUnderMachine();

//---------------------------------------------------------------------------
// [S7,F2] Process Program Load Grant (PPGNT) -- HT9045Gem override.
// GATE [G1], golden SECSGEM/uHGemHT9045.cpp:4397-4476. Full rationale and the
// behaviour delta: see the GATE REGISTER at the top of this block.
//---------------------------------------------------------------------------
#if 0
// ---- GOLDEN VERBATIM (uHGemHT9045.cpp:4397-4476) -- DO NOT EDIT ----------
int HT9045Gem::S7F2_ProcessProgramLoadGrant()
{
    unsigned char HCACK=1;
    int ret, len;
    unsigned char Type;
    AnsiString PPID;

    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        ret=HGemPtr->GetDataItemLenAndType(len, Type);
        if(ret==1 && Type==HType.ASCII_TYPE)
        {
            HGemPtr->DataItemIn(len, Type, PPID);
            HGemPtr->GetDataItemLenAndTypeAndDelete(len, Type);
        }
    }

    TRegExpr * regex    =new TRegExpr;
    regex->Expression   ="[/|\\*|\\?|:|\"|>|<|\\|]";                            //不等於裡面指定的文字
    regex->InputString=PPID;

    //Steven 20220422 : 加強S7F2_ProcessProgramLoadGrant的回覆
    if(SystemStart==true)                                                       //20140124 wei
    {
        HCACK=6;
    }
    else if(PPID.Length()<=0)
    {
        HCACK=7;
    }
    else if(regex->Exec())
    {
        HCACK=8;
    }
    else if(MOT[MMTrayZ].fHasTray || MOT[MMTrayY].fHasTray)
    {
        HCACK=9;
    }
    else if(MOT[MMPlate1].HasIC() || MOT[MMPlate2].HasIC())
    {
        HCACK=10;
    }
    else if(InArmSuck.HasIC())
    {
        HCACK=11;
    }
    else if(OutArmSuck.HasIC())
    {
        HCACK=12;
    }
    else if(FLCarryKit.UseSiteHasIC() || BLCarryKit.UseSiteHasIC())
    {
        HCACK=13;
    }
    else if(FRCarryKit.UseSiteHasIC() || BRCarryKit.UseSiteHasIC())
    {
        HCACK=14;
    }
    else if(IndexHasIC())
    {
        HCACK=15;
    }
    else if(MOT[MInRotateKit].HasIC())
    {
        HCACK=16;
    }
    else if(MOT[MOutRotateKit].HasIC())
    {
        HCACK=17;
    }
    else
    {
        HCACK=0;
    }

    HGemPtr->InitLocalHead(7, 2, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
    HGemPtr->SendLocalData();
    return HCACK;
}
#else
int HT9045Gem::S7F2_ProcessProgramLoadGrant()
{
    unsigned char HCACK=1;
    int ret, len;
    unsigned char Type;
    AnsiString PPID;

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        ret=ActiveWire->GetDataItemLenAndType(len, Type);
        if(ret==1 && Type==HType.ASCII_TYPE)
        {
            ActiveWire->DataItemIn(len, Type, PPID);
            ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type);
        }
    }

    // GATE G1a -- golden :4414-4417 built a TRegExpr over PPID whose pattern
    //   text is   [/|\*|\?|:|"|>|<|\|]   and golden :4427 used regex->Exec() as
    //   the third rung of the HCACK ladder. Golden's own trailing comment on
    //   that Expression line (:4415), transcribed verbatim:
    //       //不等於裡面指定的文字
    //   TRegExpr has no home in this tree (absence claim A1), so the pattern is
    //   evaluated directly instead of being dropped.
    //   DERIVATION, so a reviewer can check this without a regex engine: inside
    //   a bracket expression '|' is an ORDINARY character and \* \? \| are
    //   escaped literals, so the class is exactly the 8-character SET
    //        /   |   *   ?   :   "   >   <
    //   and nothing else. Note what is NOT in it: the backslash. Golden does
    //   not reject a PPID containing a backslash even though PPID is used as a
    //   directory name at :4525. That gap is GOLDEN'S and is preserved here,
    //   not fixed.
    //   Two incidental differences from golden, disclosed rather than hidden:
    //   (1) golden never deletes `regex` -- a leak on every S7F1. This arm
    //       allocates nothing, so the leak is gone; that is an improvement, not
    //       a faithful reproduction. (2) golden only evaluates the pattern once
    //       control reaches the third rung; this scan runs unconditionally. The
    //       scan has no side effects, so the HCACK actually sent is unchanged.
    bool bPPIDHasIllegalChar=false;
    {
        static const char cPPIDIllegalSet[8]={'/', '|', '*', '?', ':', '"', '>', '<'};
        for(int i=0; i<8; i++)
        {
            if(PPID.Pos(cPPIDIllegalSet[i])>0)
            {
                bPPIDHasIllegalChar=true;
                break;
            }
        }
    }

    //Steven 20220422 : 加強S7F2_ProcessProgramLoadGrant的回覆
    if(SystemStart==true)                                                       //20140124 wei
    {
        HCACK=6;
    }
    else if(PPID.Length()<=0)
    {
        HCACK=7;
    }
    else if(bPPIDHasIllegalChar)                                                // GATE G1a: golden :4427 `else if(regex->Exec())`
    {
        HCACK=8;
    }
    // GATE G1b -- golden :4431-4466's nine machine-state rungs are SKIPPED
    //   here: MOT[MMTrayZ]/MOT[MMTrayY].fHasTray -> 9,
    //   MOT[MMPlate1]/MOT[MMPlate2].HasIC() -> 10, InArmSuck.HasIC() -> 11,
    //   OutArmSuck.HasIC() -> 12, FLCarryKit/BLCarryKit.UseSiteHasIC() -> 13,
    //   FRCarryKit/BRCarryKit.UseSiteHasIC() -> 14, IndexHasIC() -> 15,
    //   MOT[MInRotateKit].HasIC() -> 16, MOT[MOutRotateKit].HasIC() -> 17.
    //   SAFETY-RELEVANT DELTA: control now falls through to HCACK=0 (GRANTED)
    //   even with material still in the machine. See GATE REGISTER entry G1b
    //   for which library owns each symbol and why the reference is withheld.
    else
    {
        HCACK=0;
    }

    ActiveWire->InitLocalHead(7, 2, 0);
    ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
    ActiveWire->SendLocalData();
    return HCACK;
}
#endif

//---------------------------------------------------------------------------
// [S7,F4] Process Program Acknowledge (ACKC7) -- HT9045Gem override.
// GATE [G2], golden SECSGEM/uHGemHT9045.cpp:4478-5321 (844 lines). This is the
// recipe INSTALLER; see the GATE REGISTER at the top of this block.
//---------------------------------------------------------------------------
#if 0
// ---- GOLDEN VERBATIM (uHGemHT9045.cpp:4478-5321) -- DO NOT EDIT ----------
void HT9045Gem::S7F4_ProcessProgramAcknowledge()                                //Steven 20140911 : Modify S7F4 for HT9045
{
//L,2
//    1. <PPID>
//    2. <PPBODY>    int ret,len;
    unsigned char Type;
    char *PPBODY;
    FILE *stream;
    HINSTANCE hInstance;
    AnsiString PPID, S, Str, PPIDFolder, str1, str2, str3, sDir, strSetupfile, sOffsetOrg, sOffset, sLastSetup;
    AnsiString NewFolder, NewFile, OrgFolder, OrgFile;

    AnsiString sConfigPath=FileInfo().PathCombin(AuthPath, "Security_new.def");
    bool b2Dsort=false;
    int ret, len;
    int iStart, iEnd;                                                           //, iRNPos;
    bool bHasSetUpFile=false, bNeedCover=false;                                 //JerryYang 20170126 (Steven) 是否已存在此工作檔
    double fTempUserOffset[tcTotalCount], fContactHeight[20], dB2DOffsetX, dB2DOffsetY, dATCOffsetBackup[32];
    int iIndexHeatingMode;
    int iShuttleMode[2]={0, 0},iAutoClean[24];                                  //JerryYang 20191024 15->24
    AnsiString strCleanCnt[20][10];                                             //KenHsieh 20230518 : Auto Clean count不覆蓋
    AnsiString asCleanCt;                                                       //KenHsieh 20230518 : Auto Clean count不覆蓋

    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        ret=HGemPtr->GetDataItemLenAndType(len, Type);
        if(ret==1 && Type==HType.ASCII_TYPE)
        {
            if(CUSTOMER_CODE==CC_MAXIM_THAILAND)                                //Ifor 20251018 add:Analog 泰國客戶要求PPID 需要多一層L
            {
                HGemPtr->GetDataItemLenAndType(len, Type);
                ret=HGemPtr->DataItemIn(len, Type, PPID);                       // PPID=filename;
            }
            else
            {
                HGemPtr->DataItemIn(len, Type, PPID);                           // PPID=filename;
                ret=HGemPtr->GetDataItemLenAndType(len, Type);
            }

            if(ret==1 && (Type==HType.ASCII_TYPE ||
                          Type==HType.BINARY_TYPE))                             //JerryYang 20230221 : S7F4可以接收Binary
            {
                PPBODY=new char [len+1];                                        //JerryYang 20190411 因為有結束位元，長度+1

                if(HGemPtr->DataItemIn(len, Type, PPBODY))
                {
                    RecordProcess("S7F4_ProcessProgramAcknowledge");
                    PPIDFolder.sprintf("D:\\HT9045\\IniData\\Data\\%s", PPID);

                    if(DirectoryExists(PPIDFolder)==false)
                    {
                        HGemPtr->StringOut("S7F4, Make folder :"+PPID);
                        MyForceDirectories(PPIDFolder, "S7F4_ProcessProgramAcknowledge");
                        bHasSetUpFile=false;                                    //JerryYang 20170126 (Steven) 本機不存在欲Download的setup file
                    }
                    else
                    {
                        HGemPtr->StringOut("S7F4, Has folder :"+PPID);
                        bHasSetUpFile=true;                                     //JerryYang 20170126 (Steven) 已存在此工作檔
                    }

                    if(bHasSetUpFile)                                           //JerryYang 20170126 (Steven) 把要還原的資料先備份
                    {
                        strSetupfile=PPID;
                    }
                    else
                    {
                        strSetupfile=GetLastOpenFN();
                    }

                    //----------------------
                    //檢查Offset檔
                    //----------------------
                    if(IniConfig.bE45_AllSetupFileUseOneFile==false)            //JerryYang 20220324 : 如果沒有offset就從當下的工作檔複製過來
                    {
                        NewFolder=FileInfo().PathCombin(fOffSet->GetOffsetPath(PPID), PPID);
                        MyForceDirectories(NewFolder);
                        NewFile  =FileInfo().PathCombin(NewFolder, "Position Offset.Data");
                        if(!FileExists(NewFile))                                //如果沒有就從當下的工作檔複製過來
                        {
                            OrgFile=FileInfo().PathCombin(fOffSet->GetOffsetPath(strSetupfile), "Position Offset.Data");
                            CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                        }

                        NewFolder=FileInfo().PathCombin(fOffSet->GetOffsetPath(PPID), PPID);
                        NewFile  =FileInfo().PathCombin(NewFolder, "Position Offset Hot.Data");
                        if(!FileExists(NewFile))                                //如果沒有就從當下的工作檔複製過來
                        {
                            OrgFile=FileInfo().PathCombin(fOffSet->GetOffsetPath(strSetupfile), "Position Offset Hot.Data");
                            CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                        }
                    }
                    //----------------------
                    //把溫度Offset資料備份
                    //----------------------
                    OrgFolder=FileInfo().PathCombin(DataPath, strSetupfile);
                    NewFolder=DataPath;

                    str1=FileInfo().PathCombin(OrgFolder, "Temperature.Data");
                    for(int i=0; i<tcTotalCount; i++)
                    {
                        str2.printf("CH%d", i+1);
                        fTempUserOffset[i]=ReadIniData(str1, "User OffSet", str2, 0.0);
                    }
                    iIndexHeatingMode=CheckAndReadIniData(str1, "Index",   "Heating Mode",   0);                        //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式

                    if(ATC_SYSTEM>eATC60)                                       //JerryYang 20220324 : 複製ATC溫度offset
                    {
                        if(ATC_SYSTEM!=eNonChamber)
                        {
                            if(CosFunction.bUseOldATCTempOffset==true)          //JerryYang 20190123 ATC Offset UI使用舊版
                            {
                            }
                            else
                            {
                                for(int i=0; i<32; i++)
                                {
                                    str2.sprintf("ATCTempOffset[%d]", i);
                                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)         //kevin 20180215 (Steven) ADD atc
                                        dATCOffsetBackup[i]=CheckRange(ReadIniData(str1, "ATC", str2, 0.0), 60.0, -60.0);
                                    else
                                        dATCOffsetBackup[i]=CheckRange(ReadIniData(str1, "ATC", str2, 0.0), (double)InputLimit.iTempHigh, (double)InputLimit.iTempLow);
                                }
                            }
                        }
                    }
                    MySleep(100);

                    //----------------------
                    //把HotPlate資料備份
                    //----------------------
                    OrgFile=FileInfo().PathCombin(OrgFolder, "HotPlate.Data");
                    NewFile=FileInfo().PathCombin(NewFolder, "HotPlate.Data");
                    ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                    MySleep(100);

                    //----------------------
                    //把Contact高度備份
                    //----------------------
                    str1=FileInfo().PathCombin(OrgFolder, "Contact.Data");
                    fContactHeight[0]=ReadIniData(str1, "Test Arm1", "Pick Up", 0.0);
                    fContactHeight[1]=ReadIniData(str1, "Test Arm1", "Contact", 0.0);
                    fContactHeight[2]=ReadIniData(str1, "Test Arm1", "Drop",    0.0);
                    fContactHeight[3]=ReadIniData(str1, "Test Arm1", "Place",   0.0);
                    fContactHeight[4]=ReadIniData(str1, "Test Arm2", "Pick Up", 0.0);
                    fContactHeight[5]=ReadIniData(str1, "Test Arm2", "Contact", 0.0);
                    fContactHeight[6]=ReadIniData(str1, "Test Arm2", "Drop",    0.0);
                    fContactHeight[7]=ReadIniData(str1, "Test Arm2", "Place",   0.0);
                    fContactHeight[8]=ReadIniData(str1, "Torque Control", "Pin Number",    0.0);
                    fContactHeight[9]=ReadIniData(str1, "Torque Control", "Force Per Pin", 0.0);
                    fContactHeight[19]=ReadIniData(str1,"Torque Control", "Torque",        0.0);

                    fContactHeight[10]=ReadIniData(str1, "Wait Time", "Drop Wait",   1.0);
                    fContactHeight[11]=ReadIniData(str1, "Wait Time", "Drop Speed",  1.0);
                    fContactHeight[12]=ReadIniData(str1, "Mode", "Contact",                          0.0);
                    fContactHeight[13]=ReadIniData(str1, "Mode", "Vacuum",                           0.0);
                    fContactHeight[14]=ReadIniData(str1, "Mode", "Dummy Contact",                    0.0);
                    fContactHeight[15]=ReadIniData(str1, "Mode", "Head Device Mode",                 0.0);
                    fContactHeight[16]=ReadIniData(str1, "Mode", "Kit Diameter",                     3.0);
                    fContactHeight[17]=ReadIniData(str1, "Mode", "Suck Shuttle Device After Tested", 0.0);
                    fContactHeight[18]=ReadIniData(str1, "Mode", "Shuttle Waiting Out Site Chamber", 0.0);

                    //----------------------
                    //把Ld/Uld速度備份
                    //----------------------
                    OrgFile=FileInfo().PathCombin(OrgFolder, "UdUld.Data");
                    NewFile=FileInfo().PathCombin(NewFolder, "UdUld.Data");
                    ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                    MySleep(100);

                    //----------------------
                    //把速度設定備份
                    //----------------------
                    OrgFile=FileInfo().PathCombin(OrgFolder, "ArmCondition.Data");
                    NewFile=FileInfo().PathCombin(NewFolder, "ArmCondition.Data");
                    ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                    MySleep(100);

                    //----------------------
                    //備份開啟單一Shuttle選項
                    //----------------------
                    str1=FileInfo().PathCombin(OrgFolder, "HandlerCondition.Data");
                    iShuttleMode[0]=ReadIniData(str1, "Configuration", "Shuttle Mode", 0);
                    iShuttleMode[1]=ReadIniData(str1, "Configuration", "Shuttle1 Cancel", 0);

                    //----------------------
                    //備份Auto Clean選項
                    //----------------------                                    //Steven 20161116 : ATC說要加上Auto Clean
                    str1=FileInfo().PathCombin(OrgFolder, "HandlerCondition.Data");
                    iAutoClean[0]=ReadIniData(str1,  "Configuration",  "iAutoClean_Function",            0);
                    iAutoClean[1]=ReadIniData(str1,  "Configuration",  "iAutoClean_SelectArm",           0);
                    iAutoClean[2]=ReadIniData(str1,  "Configuration",  "iAutoClean_ContactShiftHeight",  1000);
                    iAutoClean[3]=ReadIniData(str1,  "Configuration",  "iAutoClean_ContactCleanHeight",  0);
                    iAutoClean[4]=ReadIniData(str1,  "Configuration",  "iAutoClean_IndexPickOffset",     0);            //wei 20150318 Auto clean Index Pick Offset
                    iAutoClean[5]=ReadIniData(str1,  "Configuration",  "iAutoClean_IndexReleaseOffset",  0);            //Jou 2015-08-22 Auto clean Index Release Offset
                    iAutoClean[6]=ReadIniData(str1,  "Configuration",  "HotplatlXOffset",                0.0);          //JerryYang 20161122 補上Auto Clean參數
                    iAutoClean[7]=ReadIniData(str1,  "Configuration",  "HotplatlYOffset",                0.0);
                    iAutoClean[8]=ReadIniData(str1,  "Configuration",  "HotplatlPickOffset",             0.0);
                    iAutoClean[9]=ReadIniData(str1,  "Configuration",  "HotplatlPlaceOffset",            0.0);
                    iAutoClean[10]=ReadIniData(str1, "Configuration",  "HotplatlPitchOffset",            0.0);
                    iAutoClean[11]=ReadIniData(str1, "Configuration",  "iAutoClean_MotorSpeed[0]",       10);
                    iAutoClean[12]=ReadIniData(str1, "Configuration",  "iAutoClean_MotorSpeed[1]",       10);
                    iAutoClean[13]=ReadIniData(str1, "Configuration",  "iAutoClean_MotorSpeed[2]",       10);
                    iAutoClean[14]=ReadIniData(str1, "Configuration",  "iAutoClean_MotorSpeed[3]",       10);
                    iAutoClean[15]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle1PickOffset",  -200);         //JerryYang 20191003 add auto clean offset
                    iAutoClean[16]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle1PlaceOffset", 0);
                    iAutoClean[17]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle1XOffset",     0);
                    iAutoClean[18]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle1YOffset",     0);
                    iAutoClean[19]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle2PickOffset",  -200);
                    iAutoClean[20]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle2PlaceOffset", 0);
                    iAutoClean[21]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle2XOffset",     0);
                    iAutoClean[22]=ReadIniData(str1, "Configuration",  "iAutoClean_Shuttle2YOffset",     0);
                    iAutoClean[23]=ReadIniData(str1, "Configuration",  "ShuttlePitchOffset"        ,     0.0);

                    for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)         //KenHsieh 20230518 : Auto Clean count不覆蓋
                    {
                        for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
                        {
                            asCleanCt.sprintf("iAutoCleanPad_CountTime_%d_%d", Y, X);
                            strCleanCnt[X][Y]=ReadIniData(str1, "Configuration", asCleanCt, AnsiString(0));
                        }
                    }
                    //----------------------
                    //備份Bottom 2D offset
                    //----------------------
                    str1=FileInfo().PathCombin(OrgFolder, "HandlerCondition.Data");
                    dB2DOffsetX=ReadIniData(str1, "Configuration", "Bottom 2D Offset X",  0.0);
                    dB2DOffsetY=ReadIniData(str1, "Configuration", "Bottom 2D Offset Y",  0.0);

                    if(IniConfig.bN07_7SendRecipeAsBinary==false)               //Steven 20230710 : 工作檔使用二進制上傳下載
                    {
                        S=FileInfo().PathCombin(PPIDFolder, PPID+AnsiString(".ini"));
                        TStringList *TempPPBody;
                        TempPPBody= new TStringList();
                        TempPPBody->Clear();
                        TempPPBody->Text=PPBODY;
                        TempPPBody->SaveToFile(S);

                        AnsiString TempString,TempString3;
                        TStringList *TempString2;
                        TempString2= new TStringList();

                        Str="";
                        TempString=TempPPBody->Text;

                        int ilength =0;
                        AnsiString sTemp;
                        if(CUSTOMER_CODE==CC_ASE_KaohSiung)                     //kevin 20181101 add fast Get file 10 sec
                        {
                            TempString3=TempString;

                            while(TempString3.Length()!=0)
                            {
                                ilength=TempString3.Length();
                                iEnd   =TempString3.AnsiPos(";");
                                if(iEnd==0)                                     //last data
                                {
                                    sTemp =TempString3;
                                    TempString3="";
                                }
                                else
                                {
                                    sTemp =TempString3.SubString(0, iEnd-1);
                                    TempString3=TempString3.SubString(iEnd+1,ilength);
                                }
                                iStart=sTemp.AnsiPos("[");                      //先找到第一筆
                                iEnd  =sTemp.AnsiPos("]");
                                Str   =sTemp.SubString(iStart+1, iEnd-iStart-1);
                                sTemp.Delete(1, iEnd);                          //取出內容資料砍掉[檔名.Data]
                                TempString2->Text = StringReplace(sTemp, "\\r\\n", "\r\n", TReplaceFlags()<<rfReplaceAll);
                                if(Str.UpperCase()=="POSITION OFFSET.DATA" || Str.UpperCase()=="POSITION OFFSET HOT.DATA")
                                    S.sprintf("D:\\HT9045\\IniData\\Offset\\%s\\%s", PPID, Str);                        //kevin 20181107
                                else
                                    S.sprintf("%s\\%s", PPIDFolder, Str);

                                TempString2->SaveToFile(S);                     //TReplaceFlags
                            }
                        }
                        else
                        {
                            iStart=TempString.AnsiPos("[");                     //先找到第一筆
                            iEnd  =TempString.AnsiPos("]");
                            Str   =TempString.SubString(iStart+1, iEnd-iStart-1);
                            TempString.Delete(1, iEnd);

                            while(TempString.Length()!=0)
                            {
                                iEnd  =TempString.AnsiPos(".Data]");
                                if(iEnd!=0)                                     //找出檔案名稱
                                {
                                    iStart=iEnd-1;
                                    while(TempString.SubString(iStart, 1)!="[")
                                    {
                                        iStart--;
                                        if(iStart==1)
                                            break;
                                    }
                                    TempString2->Text=TempString.SubString(1, iStart-1);
                                }
                                else
                                {
                                    TempString2->Text=TempString.SubString(1, TempString.Length());
                                }

                                S=FileInfo().PathCombin(PPIDFolder, Str);

                                int iSize=0;
                                char *buff;
                                AnsiString asInsterData1;
                                AnsiString asInsterData2;
                                TMemoryStream* pms = new TMemoryStream();
                                TMemoryStream* newpms = new TMemoryStream();
                                pms->Write((void*)TempString2->Text.c_str(), TempString2->Text.Length());
                                iSize=pms->Size;
                                pms->Position=0;
                                buff=new char[iSize+1];
                                pms->Read(buff, iSize);
                                newpms->Position=0;
                                asInsterData1="\r";
                                asInsterData2="\n";
                                for(int i=0; i<iSize; i++)
                                {
                                    if(i+1<iSize && buff[i]=='\\' && buff[i+1]=='r')
                                    {
                                        newpms->Write(asInsterData1.c_str(), asInsterData1.Length());
                                        i++;
                                    }
                                    else if(i+1<iSize && buff[i]=='\\' && buff[i+1]=='n')
                                    {
                                        newpms->Write(asInsterData2.c_str(), asInsterData2.Length());
                                        i++;
                                    }
                                    else
                                    {
                                        newpms->Write(buff+i,1);
                                    }
                                }
                                TempString2->Text=AnsiString((char*)newpms->Memory).SubString(1, newpms->Size);

                                try                                             //AI(Copilot) 20260320 (RogerYang) : 捕捉寫檔例外，僅記錄失敗log，不中斷原流程
                                {
                                    TempString2->SaveToFile(S);
                                }
                                catch(Exception &exception)
                                {
                                    HGemPtr->StringOut("S7F4, SaveToFile fail :"+S);
                                    HGemPtr->StringOut("S7F4, SaveToFile err  :"+exception.Message);
                                }

                                delete pms;
                                delete newpms;
                                delete[] buff;                                  //JerryYang 20240821 : modify
                                buff=NULL;
                                TempString2->Clear();
                                Str=TempString.SubString(iStart+1, iEnd-iStart-1+5);
                                if(iEnd!=0)
                                    TempString.Delete(1, iEnd+5);
                                else
                                    break;
                            }
                        }
                        bRefreshFunction=true;                                  //kevin 20181101
                        LookForFile();
                        TempString2->Clear();                                   //Ifor 20170603 (wei) TStringList 刪除前先 Clean
                        delete TempString2;
                        TempString2 =NULL;

                        TempPPBody->Clear();                                    //Ifor 20170603 (wei) TStringList 刪除前先 Clean
                        delete TempPPBody;
                        TempPPBody =NULL;
                    }
                    else
                    {
                        NewFolder=FileInfo().PathCombin(DataPath, PPID);
                        str1=FileInfo().PathCombin(DataPath, PPID+AnsiString(".zip"));
                        DeleteFile(str1);
                        HGemPtr->StringOut("S7F4, DeleteFile :"+str1);
                        if((stream=fopen(str1.c_str(), "wb"))==NULL)            //Steven 20240809 : S --> str1
                        {
                            HGemPtr->StringOut("S7F4, fopen fail :"+str1);
                            HGemPtr->LocalAcknowledge(7, 4, 1);
                            delete[] PPBODY;
                            PPBODY=NULL;                                        //kevin 20180517
                            return;
                        }

                        fwrite((char *)&PPBODY[0], 1, len, stream);
                        fclose(stream);
                        HGemPtr->StringOut("S7F4, fwrite done :"+str1);

                        str1="e \""+NewFolder+".zip\" -o\""+NewFolder+"\\\" -y";
                        hInstance=ShellExecute(this, "open", "d:\\HT9045\\7z.exe", str1.c_str(), NULL, SW_HIDE);
                        if(int(hInstance)<=32)
                        {
                            ;
                        }
                        MySleep(500);                                           //Steven 20140609 : 1000 --> 100
                        str1=FileInfo().PathCombin(DataPath, PPID+AnsiString(".zip"));
                        HGemPtr->StringOut("S7F4, DeleteFile :"+str1);
                        DeleteFile(str1.c_str());
                    }
                    bRefreshFunction=true;                                      //kevin 20181101
                    LookForFile();
//                    delete[] PPBODY;                                          //Steven 20160912 : Add delete for save memory
//                    PPBODY=NULL;                                              //kevin 20180517

                    if(IniConfig.bN07_6EnableUploadOSRecipe)
                    {
                        if(IniConfig.bN07_6CompressedFile)                      //JimmyChiu 20250214 : For Hana ART
                        {
                            NewFolder=FileInfo().PathCombin(DataPath, PPID);
                            str1=FileInfo().PathCombin(NewFolder, "OS_Setting.zip");                                    //Steven 20230710 : OS測試機的工作檔也要上傳
                            str2=FileInfo().PathCombin(IniConfig.sN07_6OSRecipePath, PPID+"ini");
                            if(FileExists(str1))
                            {
                                str3="e \""+str1+"\" -o\""+str2+"\\\" -y";
                                hInstance=ShellExecute(this, "open", "d:\\HT9045\\7z.exe", str3.c_str(), NULL, SW_HIDE);
                                if(int(hInstance)<=32)
                                {
                                    ;
                                }
                                MySleep(500);
                            }
                        }
                        else
                        {
                            OrgFolder=FileInfo().PathCombin(DataPath, PPID);
                            NewFolder=FileInfo().PathCombin(IniConfig.sN07_6OSRecipePath, PPID);
                            OrgFile=FileInfo().PathCombin(OrgFolder, "OS_Setting.Data");
                            NewFile=FileInfo().PathCombin(NewFolder, "OS_Setting.ini");                                 //Steven 20230710 : OS測試機的工作檔也要上傳
                            ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                        }
                    }

                    NewFolder=FileInfo().PathCombin(DataPath, PPID);
                    OrgFolder=DataPath;

                    if(IniConfig.bA30SetupTeachFunction)                        //JerryYang 20180921 Setup Teach功能
                    {
                        int iTempPinNum=0;
                        double dXDeimen=0.0, dYDeimen=0.0;
                        str1=FileInfo().PathCombin(NewFolder, "Contact.Data");

                        iTempPinNum=ReadIniData(str1, "Torque Control", "Pin Number",    0);
                        dXDeimen=ReadIniData(str1, "Torque Control", "X Dimension",   0.0);
                        dYDeimen=ReadIniData(str1, "Torque Control", "Y Dimension",   0.0);

                        if(DeviceForm_File.iPinCT!=iTempPinNum || DeviceForm_File.XDimension!=dXDeimen || DeviceForm_File.YDimension!=dYDeimen)
                        {
                            LastSet.bNeedSetupTeach=true;
                            WriteLastDataFile();
                            for(int i=0; i<5; i++)
                            {
                                iInArmPickPlaceCnt[i]=0;
                                bInArmStop[i]=false;
                            }

                            for(int i=0; i<8; i++)                              //Out arm部分，預設全部檢查
                            {
//                                iOutArmPickPlaceCnt[i]=0;                     //JerryYang 20230523 : 沒在用, Mark掉
                                bOutArmStop[i]=false;
                            }
                        }
                    }

                    if(IniConfig.bSPILFunction)                                 //JerryYang 20230322 : //矽品中山廠封裝是by機台上傳工作檔,要download
                    {
                        str1=FileInfo().PathCombin(NewFolder, "HandlerCondition.Data");
                        b2Dsort=ReadIniData(str1, "Configuration", "Sorting By 2DID List", false);
                    }
                    //JerryYang 20170126 (Steven) 不覆蓋的要進行還原
                    //==>
                    //----------------------
                    //不覆蓋就是必須要還原
                    // 0 : 不覆蓋
                    // 1 : 要覆蓋
                    //----------------------

                    //----------------------
                    //把溫度Offset資料還原
                    //----------------------
                    if(CosFunction.bDownloadRecipeLevelMode)                    //jou 2016-01-06 download recipe 增加權限模式選擇
                    {
                        if(fLotInfo->coLevelMode->Text!="Normal")
                            bNeedCover=true;
                        else
                            bNeedCover=false;
                    }
                    else
                    {
                        bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Temp Offset", true);
                    }

                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 20180831 add
                    {
                        bNeedCover=true;
                    }
                    else if(IniConfig.bSPILFunction || CUSTOMER_CODE==CC_ASE_CL)                                        //JerryYang 20250120 : modify                            //JerryYang 20220923 : add SPIL
                    {
                        bNeedCover=false;
                    }

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "Temperature.Data");
                        for(int i=0; i<tcTotalCount; i++)                       //ChungHung 20140804 add 修正 RMS 原本 只還原到前十個 uer Offset
                        {
                            str2.printf("CH%d", i+1);
                            WriteIniData(str1, "User OffSet", str2, fTempUserOffset[i]);
                        }
                    }

                    if(ATC_SYSTEM>eATC60)                                       //JerryYang 20220324 : 複製ATC溫度offset
                    {
                        if(ATC_SYSTEM!=eNonChamber)
                        {
                            if(CosFunction.bUseOldATCTempOffset==true)          //JerryYang 20190123 ATC Offset UI使用舊版
                            {
                            }
                            else
                            {
                                for(int i=0; i<32; i++)
                                {
                                    str2.sprintf("ATCTempOffset[%d]", i);
                                    WriteIniData(str1, "ATC", str2, dATCOffsetBackup[i]);
                                }
                            }
                        }
                    }

                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Index Heat Mode", true);                    //Steven 20180420 (Jou) : JCET吳如春說不覆蓋Index加熱模式
                    if(bNeedCover==false)
                    {
                        str1=FileInfo().PathCombin(NewFolder, "Temperature.Data");
                        WriteIniData(str1, "Index",   "Heating Mode",   iIndexHeatingMode);
                    }

                    //----------------------
                    //把Contact高度還原
                    //----------------------
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact High", false);                      //預設不覆蓋
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 20180831 add
                    {
                        bNeedCover=true;
                    }
                    else if(IniConfig.bSPILFunction)                            //JerryYang 20220923 : add SPIL
                    {
                        bNeedCover=false;
                        if(b2Dsort==true)                                       //JerryYang 20230322 : //矽品中山廠封裝是by機台上傳工作檔,要download
                        {
                            bNeedCover=true;
                        }
                    }

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "Contact.Data");
                        WriteIniData(str1, "Test Arm1", "Pick Up", fContactHeight[0]);
                        WriteIniData(str1, "Test Arm1", "Contact", fContactHeight[1]);
                        if(IniConfig.bSPILFunction==false)                      //JerryYang 20200812 : SPIL緯平要求將drop offset移除,不要當機差參數
                        {
                            WriteIniData(str1, "Test Arm1", "Drop",    fContactHeight[2]);
                            WriteIniData(str1, "Test Arm2", "Drop",    fContactHeight[6]);
                        }
                        WriteIniData(str1, "Test Arm1", "Place",   fContactHeight[3]);
                        WriteIniData(str1, "Test Arm2", "Pick Up", fContactHeight[4]);
                        WriteIniData(str1, "Test Arm2", "Contact", fContactHeight[5]);
                        WriteIniData(str1, "Test Arm2", "Place",   fContactHeight[7]);
                    }

                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact Force", true);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20180831 add
                       IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                    {
                        bNeedCover=true;
                    }

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "Contact.Data");
                        WriteIniData(str1, "Torque Control", "Pin Number",    fContactHeight[8]);
                        WriteIniData(str1, "Torque Control", "Force Per Pin", fContactHeight[9]);
                        WriteIniData(str1, "Torque Control", "Torque"       , fContactHeight[19]);
                    }

                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Contact Mode", true);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20180831 add
                       IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                        bNeedCover=true;

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "Contact.Data");
                        WriteIniData(str1, "Wait Time", "Drop Wait",                    fContactHeight[10]);
                        WriteIniData(str1, "Wait Time", "Drop Speed",                   fContactHeight[11]);
                        WriteIniData(str1, "Mode", "Contact",                           fContactHeight[12]);
                        WriteIniData(str1, "Mode", "Vacuum",                            fContactHeight[13]);
                        WriteIniData(str1, "Mode", "Dummy Contact",                     fContactHeight[14]);
                        WriteIniData(str1, "Mode", "Head Device Mode",                  fContactHeight[15]);
                        WriteIniData(str1, "Mode", "Kit Diameter",                      fContactHeight[16]);
                        WriteIniData(str1, "Mode", "Suck Shuttle Device After Tested",  fContactHeight[17]);
                        WriteIniData(str1, "Mode", "Shuttle Waiting Out Site Chamber",  fContactHeight[18]);
                    }

                    //----------------------
                    //把HotPlate資料還原
                    //----------------------
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "HotPlate", false);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20180831 add
                       IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                        bNeedCover=true;

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        OrgFile=FileInfo().PathCombin(OrgFolder, "HotPlate.Data");
                        NewFile=FileInfo().PathCombin(NewFolder, "HotPlate.Data");
                        ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                        MySleep(100);
                    }

                    OrgFile=FileInfo().PathCombin(OrgFile, "HotPlate.Data");
                    DeleteFile(OrgFile.c_str());
                    MySleep(100);

                    //----------------------
                    //把Ld/Uld速度還原
                    //----------------------
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Load Unload", false);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20180831 add
                       IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                    {
                        bNeedCover=true;
                    }

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        OrgFile=FileInfo().PathCombin(OrgFolder, "UdUld.Data");
                        NewFile=FileInfo().PathCombin(NewFolder, "UdUld.Data");
                        ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                        MySleep(100);
                    }
                    OrgFile=FileInfo().PathCombin(OrgFolder, "UdUld.Data");
                    DeleteFile(OrgFile.c_str());
                    MySleep(100);

                    //----------------------
                    //把速度設定還原
                    //----------------------
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Speed Setting", true);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20180831 add
                       IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                        bNeedCover=true;

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        OrgFile=FileInfo().PathCombin(OrgFolder, "ArmCondition.Data");
                        NewFile=FileInfo().PathCombin(NewFolder, "ArmCondition.Data");
                        ret=CopyFile(OrgFile.c_str(), NewFile.c_str(), false);
                        MySleep(100);
                    }

                    OrgFile=FileInfo().PathCombin(OrgFolder, "ArmCondition.Data");
                    DeleteFile(OrgFile.c_str());
                    MySleep(100);

                    //----------------------
                    //還原開啟單一Shuttle選項
                    //----------------------
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Shuttle Mode", false);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung ||                       //kevin 20180831 add
                       IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                        bNeedCover=true;

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "HandlerCondition.Data");
                        WriteIniData(str1, "Configuration", "Shuttle Mode", iShuttleMode[0]);
                        WriteIniData(str1, "Configuration", "Shuttle1 Cancel", iShuttleMode[1]);
                    }
                    //----------------------
                    //還原Auto Clean選項
                    //----------------------                                    //Steven 20161116 : ATC說要加上Auto Clean
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Auto Clean", true);
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 20180831 add
                        bNeedCover=true;
                    else if(CUSTOMER_CODE==CC_SCC || IniConfig.bSPILFunction)   //Steven 20190712 : 李國旗說要寫死不覆蓋  //JerryYang 20220923 : add SPIL
                        bNeedCover=false;

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "HandlerCondition.Data");
                        if(IniConfig.bSPILFunction==true)                       //JerryYang 20191003 矽品只還原auto clean offset
                        {
                            WriteIniData(str1, "Configuration", "iAutoClean_ContactShiftHeight",  iAutoClean[2]);

                            bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Auto Clean Contact Height", false);                                         //JerryYang 20241019 : 矽品二林 耀仁要求Auto clean高度可選擇不覆蓋
                            if(bNeedCover==false)
                            {
                                WriteIniData(str1, "Configuration", "iAutoClean_ContactCleanHeight",  iAutoClean[3]);
                            }
                            WriteIniData(str1, "Configuration", "iAutoClean_IndexPickOffset",     iAutoClean[4]);
                            WriteIniData(str1, "Configuration", "iAutoClean_IndexReleaseOffset",  iAutoClean[5]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PickOffset",  iAutoClean[15]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PlaceOffset", iAutoClean[16]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1XOffset",     iAutoClean[17]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1YOffset",     iAutoClean[18]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PickOffset",  iAutoClean[19]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PlaceOffset", iAutoClean[20]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2XOffset",     iAutoClean[21]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2YOffset",     iAutoClean[22]);
                            WriteIniData(str1, "Configuration", "ShuttlePitchOffset",             iAutoClean[23]);
                        }
                        else if(CUSTOMER_CODE==CC_SCC)                          //JerryYang 20190919 : 曹沖說除了Height，其他的都需要正常download覆蓋本機參數
                        {
                            WriteIniData(str1, "Configuration", "iAutoClean_ContactShiftHeight",  iAutoClean[2]);
                            WriteIniData(str1, "Configuration", "iAutoClean_ContactCleanHeight",  iAutoClean[3]);
                            WriteIniData(str1, "Configuration", "iAutoClean_IndexPickOffset",     iAutoClean[4]);       //wei 20150318 Auto clean Index Pick Offset
                            WriteIniData(str1, "Configuration", "iAutoClean_IndexReleaseOffset",  iAutoClean[5]);       //Jou 2015-08-22 Auto clean Index Release Offset
                            WriteIniData(str1, "Configuration", "HotplatlXOffset",                iAutoClean[6]);       //JerryYang 20161122 補上Auto Clean參數
                            WriteIniData(str1, "Configuration", "HotplatlYOffset",                iAutoClean[7]);
                            WriteIniData(str1, "Configuration", "HotplatlPickOffset",             iAutoClean[8]);
                            WriteIniData(str1, "Configuration", "HotplatlPlaceOffset",            iAutoClean[9]);
                            WriteIniData(str1, "Configuration", "HotplatlPitchOffset",            iAutoClean[10]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PickOffset",  iAutoClean[15]);      //JerryYang 20170411 (Steven) 補上auto clean offset
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PlaceOffset", iAutoClean[16]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1XOffset",     iAutoClean[17]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1YOffset",     iAutoClean[18]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PickOffset",  iAutoClean[19]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PlaceOffset", iAutoClean[20]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2XOffset",     iAutoClean[21]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2YOffset",     iAutoClean[22]);
                            WriteIniData(str1, "Configuration", "ShuttlePitchOffset",             iAutoClean[23]);
                        }
                        else
                        {
                            WriteIniData(str1, "Configuration", "iAutoClean_Function",            iAutoClean[0]);
                            WriteIniData(str1, "Configuration", "iAutoClean_SelectArm",           iAutoClean[1]);
                            WriteIniData(str1, "Configuration", "iAutoClean_ContactShiftHeight",  iAutoClean[2]);
                            WriteIniData(str1, "Configuration", "iAutoClean_ContactCleanHeight",  iAutoClean[3]);
                            WriteIniData(str1, "Configuration", "iAutoClean_IndexPickOffset",     iAutoClean[4]);       //wei 20150318 Auto clean Index Pick Offset
                            WriteIniData(str1, "Configuration", "iAutoClean_IndexReleaseOffset",  iAutoClean[5]);       //Jou 2015-08-22 Auto clean Index Release Offset
                            WriteIniData(str1, "Configuration", "HotplatlXOffset",                iAutoClean[6]);       //JerryYang 20161122 補上Auto Clean參數
                            WriteIniData(str1, "Configuration", "HotplatlYOffset",                iAutoClean[7]);
                            WriteIniData(str1, "Configuration", "HotplatlPickOffset",             iAutoClean[8]);
                            WriteIniData(str1, "Configuration", "HotplatlPlaceOffset",            iAutoClean[9]);
                            WriteIniData(str1, "Configuration", "HotplatlPitchOffset",            iAutoClean[10]);
                            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[0]",       iAutoClean[11]);
                            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[1]",       iAutoClean[12]);
                            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[2]",       iAutoClean[13]);
                            WriteIniData(str1, "Configuration", "iAutoClean_MotorSpeed[3]",       iAutoClean[14]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PickOffset",  iAutoClean[15]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1PlaceOffset", iAutoClean[16]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1XOffset",     iAutoClean[17]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle1YOffset",     iAutoClean[18]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PickOffset",  iAutoClean[19]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2PlaceOffset", iAutoClean[20]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2XOffset",     iAutoClean[21]);
                            WriteIniData(str1, "Configuration", "iAutoClean_Shuttle2YOffset",     iAutoClean[22]);
                            WriteIniData(str1, "Configuration", "ShuttlePitchOffset",             iAutoClean[23]);
                        }
                    }

                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Cleaning Count", bNeedCover);               //KenHsieh 20230518 : Auto Clean count不覆蓋
                    if(bNeedCover==false)                                       //KenHsieh 20230518 : Auto Clean count不覆蓋
                    {
                        str1=FileInfo().PathCombin(NewFolder, "HandlerCondition.Data");
                        for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
                        {
                            for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
                            {
                                asCleanCt.sprintf("iAutoCleanPad_CountTime_%d_%d", Y, X);
                                WriteIniData(str1, "Configuration", asCleanCt, strCleanCnt[X][Y]);
                            }
                        }
                    }

                    //----------------------
                    //還原Bottom 2D offset
                    //----------------------
                    bNeedCover=CheckAndReadIniData(sConfigPath, "Network", "Bottom 2D offset", true);
                    if(IniConfig.bSPILFunction)                                 //JerryYang 20220923 : add SPIL
                    {
                        bNeedCover=false;
                        if(b2Dsort==true)                                       //JerryYang 20230322 : //矽品中山廠封裝是by機台上傳工作檔,要download
                        {
                            bNeedCover=true;
                        }
                    }

                    if(bNeedCover==false)                                       //不覆蓋就要還原
                    {
                        str1=FileInfo().PathCombin(NewFolder, "HandlerCondition.Data");
                        WriteIniData(str1, "Configuration", "Bottom 2D Offset X",     dB2DOffsetX);
                        WriteIniData(str1, "Configuration", "Bottom 2D Offset Y",     dB2DOffsetY);
                    }
                    //----------------------
                    //檢查Offset檔
                    //----------------------
                    str1.sprintf("%s%s\\Position Offset.Data", OffsetPath, PPID);
                    if(!FileExists(str1))                                       //如果沒有就從當下的工作檔複製過來  //JerryYang 20210119 : fix機台點位offset問題
                    {
                        str2.sprintf("%s%s\\Position Offset.Data", OffsetPath, GetLastOpenFN());
                        sDir.sprintf("%s%s\\", OffsetPath, PPID);
                        MyForceDirectories(sDir);
                        CopyFile(str2.c_str(), str1.c_str(), false);
                    }

                    str1.sprintf("%s%s\\Position Offset Hot.Data", OffsetPath, PPID);
                    if(!FileExists(str1))                                       //如果沒有就從當下的工作檔複製過來
                    {
                        str2.sprintf("%s%s\\Position Offset Hot.Data", OffsetPath, GetLastOpenFN());
                        sDir.sprintf("%s%s\\", OffsetPath, PPID);
                        MyForceDirectories(sDir);
                        CopyFile(str2.c_str(), str1.c_str(), false);
                    }
                    MySleep(100);
                }
                delete[] PPBODY;                                                //Steven 20160912 : Add delete for save memory
                PPBODY=NULL;                                                    //kevin 20180517
                if(strSetupfile==PPID)                                          //Steven 20220818 : Fixed for bD31RTCChangeRecipeNeedreCreateModel
                    fSetup->bFirstTime=true;
                else
                    fSetup->bFirstTime=false;

                if(CosFunction.bKeepOnly1SetupFile)
                {
                    try
                    {
                        fLotInfo->ClearAllSetupFile(PPID);                      //Steven 20210917 : Add for 下載完工作檔後, 只留一個就好
                    }
                    catch(...)
                    {
                    };
//                    fMain->PPID=PPID;
//                    fMain->bNeedClearFile=true;
                }
                LastSet.bHasDownloadFile=true;                                  //Steven 20200415 : 下載工作檔的Flag
                HGemPtr->LocalAcknowledge(7, 4, 0);                             //Steven 20211013 : 換位置
                return;
            }
        }
    }
    S9F7_IllegalData("S7,F3 Data Format error !!!");
}
#else
void HT9045Gem::S7F4_ProcessProgramAcknowledge()                                //Steven 20140911 : Modify S7F4 for HT9045
{
//L,2
//    1. <PPID>
//    2. <PPBODY>
    unsigned char Type;
    char *PPBODY;
    AnsiString PPID;
    int ret, len;

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        ret=ActiveWire->GetDataItemLenAndType(len, Type);
        if(ret==1 && Type==HType.ASCII_TYPE)
        {
            // GATE G2d -- golden :4506-4515 branched on
            //   CUSTOMER_CODE==CC_MAXIM_THAILAND (Ifor 20251018: that host wraps
            //   PPID in one extra L) and read the item header FIRST. cmydef.h is
            //   deliberately not included in this TU (see the extern block above
            //   for why), so this arm always takes golden's own `else` path --
            //   the one every other CUSTOMER_CODE takes.
            ActiveWire->DataItemIn(len, Type, PPID);                             // PPID=filename;
            ret=ActiveWire->GetDataItemLenAndType(len, Type);

            if(ret==1 && (Type==HType.ASCII_TYPE ||
                          Type==HType.BINARY_TYPE))                             //JerryYang 20230221 : S7F4可以接收Binary
            {
                PPBODY=new char [len+1];                                        //JerryYang 20190411 因為有結束位元，長度+1

                if(ActiveWire->DataItemIn(len, Type, PPBODY))
                {
                    // GATE G2 BODY -- golden :4524-5293 is the entire install and
                    //   NONE of it runs: RecordProcess, MyForceDirectories on
                    //   D:\HT9045\IniData\Data\<PPID>, the Position-Offset
                    //   seeding under D:\HT9045\IniData\Offset\<PPID>, the
                    //   Temperature / Contact / HotPlate / UdUld / ArmCondition /
                    //   HandlerCondition read-back-and-restore block, the 7z.exe
                    //   shell-outs, the OS-recipe side copy, the Setup-Teach
                    //   invalidation, and the b2Dsort / bNeedCover per-customer
                    //   override ladder.
                    ActiveWire->StringOut("S7F4, install GATED (port) -- PPID:"+PPID);
                }
                delete[] PPBODY;                                                //Steven 20160912 : Add delete for save memory
                PPBODY=NULL;                                                    //kevin 20180517
                // GATE G2a/G2b/G2c -- golden :5297-5314 (fSetup->bFirstTime;
                //   CosFunction.bKeepOnly1SetupFile ->
                //   fLotInfo->ClearAllSetupFile(PPID); LastSet
                //   .bHasDownloadFile=true) are skipped: no ported home for
                //   those facade members (absence claims A3 / A4 / A5).
                // GATE G2 REPLY -- golden :5315 sends ACKC7=0 (accepted).
                //   Nothing was stored here, so this arm sends golden's OWN
                //   store-failure acknowledge instead, lifted from golden's
                //   fopen-failed path at :4858: LocalAcknowledge(7, 4, 1).
                ActiveWire->LocalAcknowledge(7, 4, 1);
                return;
            }
        }
    }
    S9F7_IllegalData("S7,F3 Data Format error !!!");
}
#endif

// ---------------------------------------------------------------------------
// V 1.0
// S7,F6 This message is used to transfer a process program.
// ---------------------------------------------------------------------------
// GATE [G3], golden SECSGEM/uHGemHT9045.cpp:5326-5603. See the GATE REGISTER.
// ---------------------------------------------------------------------------
#if 0
// ---- GOLDEN VERBATIM (uHGemHT9045.cpp:5326-5603) -- DO NOT EDIT ----------
void HT9045Gem::S7F6_ProcessProgramData()                                       //Steven 20140911 : Modify S7F6 for HT9045
{
    unsigned char Type;
    AnsiString S, Str, str1, str2, str3;
    char PPID[1024];
    int ret, len;
    FILE *stream;
    HINSTANCE hInstance;
    int handle, length;
    char *PPBODY;

    TStringList *TempString2;                                                   //JerryYang 20241118 : 優化S7F5時間
    TempString2= new TStringList();
    TempString2->Clear();

    ret=HGemPtr->GetDataItemLenAndType(len, Type);
    if(ret==1 && Type==HType.ASCII_TYPE)
    {
        HGemPtr->DataItemIn(1024, Type, PPID);                                  // PPID=filename;
        S.sprintf("D:\\HT9045\\IniData\\Data\\%s", PPID);

        if(DirectoryExists(S)==false)
        {
            S9F7_IllegalData("S7,F5 PPID Not Exists!!!");
            return;
        }

        if(IniConfig.bN07_6EnableUploadOSRecipe)
        {
            if(IniConfig.bN07_6CompressedFile)                                  //JimmyChiu 20250214 : For Hana ART
            {
                str1.sprintf("%s%s\\OS_Setting.zip", DataPath, PPID);           //Steven 20230710 : OS測試機的工作檔也要上傳
                str2.sprintf("%s%s.ini", IniConfig.sN07_6OSRecipePath, PPID);
                if(FileExists(str2))
                {
                    Str.sprintf("S7F6, step 2 : Copy OS file from %s to %s   ", str2, str1);
                    HGemPtr->StringOut(Str);
                    str3.sprintf("a -tzip \"%s\" \"%s\"", str1, str2);
                    hInstance=ShellExecute(this, "open", "d:\\HT9045\\7z.exe", str3.c_str(), NULL, SW_HIDE);
                    HGemPtr->StringOut("S7F6, Zip File");
                    MySleep(IniConfig.dN07_7_DelayTime*1000);                   //Steven 20240105 : add delay
                    HGemPtr->StringOut("S7F6, step 3 : Zip File Done");
                }
                else
                {
                    Str.sprintf("S7F6, step 1 : OS File %s does not exists!", str2);
                    HGemPtr->StringOut(Str);
                }
            }
            else
            {
                str1.sprintf("%s%s\\OS_Setting.data", DataPath, PPID);          //Steven 20230710 : OS測試機的工作檔也要上傳
                str2.sprintf("%s%s.ini", IniConfig.sN07_6OSRecipePath, PPID);
                if(FileExists(str2))
                {
                    Str.sprintf("S7F6, step 2 : Copy OS file from %s to %s   ", str2, str1);
                    HGemPtr->StringOut(Str);
                    CopyFile(str2.c_str(), str1.c_str(), false);
                    MySleep(IniConfig.dN07_7_DelayTime*1000);                   //Steven 20240105 : add delay
                    LogSoftwareOnTime("S7F6, step 3 : Copy File Done");
                }
                else
                {
                    Str.sprintf("S7F6, step 1 : OS File %s does not exists!", str2);
                    HGemPtr->StringOut(Str);
                }
            }

            if(FileExists(str1))
            {
                HGemPtr->StringOut("S7F6, copy OS file ok");
            }
            else
            {
                HGemPtr->StringOut("S7F6, copy OS file fail");
            }
            HGemPtr->SaveSECSGEMTextToLog();
        }

        HGemPtr->FileListBox3->Directory=S;
        HGemPtr->memoPPBody->Clear();
        TStringList *TempString;
        TempString= new TStringList();
        AnsiString sBuffer="", sBuffer1="";                                     //kevin 20180823 add ASE KH

        if(IniConfig.bN07_7SendRecipeAsBinary==false)                           //Steven 20230710 : 工作檔使用二進制上傳下載
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung || CUSTOMER_CODE==CC_ASE_CL)     //JerryYang 20250120 : modify                                 //kevin 20180830 add
            {
                for(int i=0; i<HGemPtr->FileListBox3->Items->Count; i++)
                {
                    TempString->Clear();
                    TempString->LoadFromFile(S+"\\"+HGemPtr->FileListBox3->Items->Strings[i]);
                    Str.sprintf("[%s]", HGemPtr->FileListBox3->Items->Strings[i]);
                    sBuffer.sprintf("%s%s;", Str,TempString->Text);
                    sBuffer1+=sBuffer;
                }
                S.sprintf("D:\\HT9045\\IniData\\Offset\\%s", PPID);
                if(DirectoryExists(S)==false)
                {
                    S9F7_IllegalData("S7,F5 PPID OFFSET Not Exists!!!");
                    return;
                }

                HGemPtr->FileListBox3->Directory=S;
                for(int i=0; i<HGemPtr->FileListBox3->Items->Count; i++)        //kevin 20180830 add copy offset data
                {
                    TempString->Clear();
                    TempString->LoadFromFile(S+"\\"+HGemPtr->FileListBox3->Items->Strings[i]);
                    Str.sprintf("[%s]", HGemPtr->FileListBox3->Items->Strings[i]);
                    if(i==HGemPtr->FileListBox3->Items->Count-1)
                        sBuffer.sprintf("%s%s", Str,TempString->Text);
                    else
                        sBuffer.sprintf("%s%s;", Str,TempString->Text);
                    sBuffer1+=sBuffer;
                }

                HGemPtr->memoPPBody->Lines->Add(sBuffer1);
            }
            else
            {
                for(int i=0; i<HGemPtr->FileListBox3->Items->Count; i++)
                {
                    TempString->Clear();
                    TempString->LoadFromFile(S+"\\"+HGemPtr->FileListBox3->Items->Strings[i]);
                    Str.sprintf("[%s]", HGemPtr->FileListBox3->Items->Strings[i]);
                    TempString2->Add(Str);                                      //JerryYang 20241118 : 優化S7F5時間
                    TempString2->Add(TempString->Text);
                    TempString2->Add("");
                }
            }

            if(TempString2->Count<1000)                                         //JerryYang 20241118 : 優化S7F5時間
            {
                for(int i=0; i<TempString2->Count; i++)
                {
                    Str=TempString2->Strings[i];
                    Str=Str.Trim();
                    if(Str=="")
                    {
                        TempString2->Delete(i);
                        i--;
                        if(i<0)
                            i=0;
                    }
                }
            }
            str1.sprintf("S7F6, Trim() %d", HGemPtr->memoPPBody->Lines->Count);
            HGemPtr->StringOut(str1);

            char *buff;                                                         //JerryYang 20241118 : 優化S7F5時間
            int iSize=0;
            AnsiString asInsterData1;
            AnsiString asInsterData2;
            Str=TempString2->Text;
            TMemoryStream* pms = new TMemoryStream();
            TMemoryStream* newpms = new TMemoryStream();
            pms->Write((void*)Str.c_str(), Str.Length());
            iSize=pms->Size;
            pms->Position=0;
            buff=new char[iSize+1];
            pms->Read(buff,iSize);
            newpms->Position=0;
            asInsterData1="\\r";
            asInsterData2="\\n";
            for(int i=0; i<iSize; i++)
            {
                if(buff[i]=='\r' )
                {
                    newpms->Write(asInsterData1.c_str(), asInsterData1.Length());
                }
                else if(buff[i]=='\n' )
                {
                    newpms->Write(asInsterData2.c_str(), asInsterData2.Length());
                }
                else
                {
                    newpms->Write(buff+i,1);
                }
            }
            Str=AnsiString((char*)newpms->Memory).SubString(1, newpms->Size);
            delete pms;
            delete newpms;
            delete[] buff;                                                      //JerryYang 20240821 : modify
            buff=NULL;

            HGemPtr->InitLocalHead(7, 6, 0);
            HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
            HGemPtr->DataItemOut(strlen(PPID), HType.ASCII_TYPE, PPID);
            HGemPtr->DataItemOut(HType.ASCII_TYPE, Str);
            HGemPtr->SendLocalData();
            HGemPtr->StringOut("S7F6, DataItemOut");
        }
        else
        {
            //先刪除舊的壓縮檔
            //jou 2012-12-14 system改採用DeleteFile
            str1.sprintf("%s%s.zip", DataPath, PPID);
            DeleteFile(str1.c_str());
            MySleep(IniConfig.dN07_7_DelayTime*1000);                           //Steven 20240105 : add delay

            //把7Zip準備好
            if(FileExists("d:\\HT9045\\7z.exe")==false)
            {
                CopyFile("C:\\Program Files\\7-Zip\\7z.exe", "d:\\HT9045\\7z.exe" , false);                             //Ifor 20170321 FALSE => false
            }

            //把工作檔壓縮
            //jou 2012-12-14 system改採用ShellExecute
            str1.sprintf("a -tzip \"%s%s.zip\" \"%s%s\\*.*\"", DataPath, PPID, DataPath, PPID);
            hInstance=ShellExecute(this, "open", "d:\\HT9045\\7z.exe", str1.c_str(), NULL, SW_HIDE);
            MySleep(IniConfig.dN07_7_DelayTime*1000);                           //Steven 20240105 : add delay
            HGemPtr->StringOut("S7F6, Zip File");
            if(int(hInstance)<=32)
            {
                //可能是壓縮失敗
                HGemPtr->StringOut("Zip recipe fail!");
                HGemPtr->InitLocalHead(7, 6, 0);
                HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
                HGemPtr->StringOut("S7F6, Zip File fail");
            }

            S.sprintf("%s%s.zip", DataPath, PPID);

            MySleep(IniConfig.dN07_7_DelayTime*1000);                           //Steven 20240105 : add delay
            if(FileExists(S))
            {
                HGemPtr->StringOut(AnsiString("S7F6, Has Zip File:")+S);
            }
            else
            {
                HGemPtr->StringOut(AnsiString("S7F6, Can not found Zip File")+S);
            }

            handle=open(S.c_str(), O_RDONLY);
            if(handle==-1)
            {
                HGemPtr->InitLocalHead(7, 6, 0);
                HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
                HGemPtr->SendLocalData();
                HGemPtr->StringOut("S7F6, open File fail");
            }
            else
            {
                length=filelength(handle);
                HGemPtr->StringOut(AnsiString("S7F6, File Length:")+AnsiString(length));
                PPBODY=new char [length];
                close(handle);
                if((stream=fopen(S.c_str(), "rb"))==NULL)
                {
                    delete[] PPBODY;
                }
                else
                {
                    fread(PPBODY, 1, length, stream);
                    fclose(stream);
                    HGemPtr->InitLocalHead(7, 6, 0);
                        HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
                            HGemPtr->DataItemOut(strlen(PPID), HType.ASCII_TYPE, PPID);
                        HGemPtr->DataItemOut(length, HType.BINARY_TYPE, PPBODY);
                    HGemPtr->SendLocalData();
                    delete[] PPBODY;
                    HGemPtr->StringOut("S7F6, DataItemOut");
                }
            }
            str1.sprintf("%s%s.zip", DataPath, PPID);                           //最後要把壓縮檔給刪除。
            DeleteFile(str1.c_str());
        }
        LookForFile();
        TempString->Clear();                                                    //Ifor 20170603 (wei) TStringList 刪除前先 Clean
        delete TempString;
        TempString2->Clear();                                                   //JerryYang 20241118 : 優化S7F5時間
        delete TempString2;

        return;
    }
    S9F7_IllegalData("S7,F5 Data Format error !!!");
}
#else
void HT9045Gem::S7F6_ProcessProgramData()                                       //Steven 20140911 : Modify S7F6 for HT9045
{
    unsigned char Type;
    AnsiString S;
    char PPID[1024];
    int ret, len;

    TStringList *TempString2;                                                   //JerryYang 20241118 : 優化S7F5時間
    TempString2= new TStringList();
    TempString2->Clear();

    ret=ActiveWire->GetDataItemLenAndType(len, Type);
    if(ret==1 && Type==HType.ASCII_TYPE)
    {
        ActiveWire->DataItemIn(1024, Type, PPID);                               // PPID=filename;
        // HARD-CODED ABSOLUTE MACHINE PATH, golden :5345, KEPT: this is a READ
        // (DirectoryExists) only -- it creates nothing and writes nothing.
        // Reported in this wave's hand-off. GOLDEN QUIRK PRESERVED: PPID is an
        // uninitialised char[1024] and golden formats it with %s without ever
        // checking that DataItemIn terminated it.
        S.sprintf("D:\\HT9045\\IniData\\Data\\%s", PPID);

        if(DirectoryExists(S)==false)
        {
            S9F7_IllegalData("S7,F5 PPID Not Exists!!!");
            return;                                                             // GOLDEN BUG PRESERVED: TempString2 leaks here (golden :5350 returns without deleting it)
        }

        // GATE G3 BODY -- golden :5353-5593 is skipped in full: the
        //   IniConfig.bN07_6EnableUploadOSRecipe OS-recipe side copy and its
        //   7z.exe zip of <DataPath><PPID>\OS_Setting.*, the FileListBox3-driven
        //   per-file read of the work-file directory into memoPPBody /
        //   TempString2 with its CR/LF <-> literal-"\r\n" rewrite, and the
        //   binary branch that zips <DataPath><PPID> via d:\HT9045\7z.exe
        //   (copying it out of "C:\Program Files\7-Zip" first when absent),
        //   reads the .zip back with open() / filelength() / fopen() / fread()
        //   and DeleteFile()s it afterwards. Blockers: THGem has no
        //   FileListBox3 / memoPPBody (absence claim A2) and every remaining
        //   branch writes into the live machine tree.
        // REPLY: golden's OWN "cannot produce the file" form, lifted verbatim
        //   from its open()==-1 path at :5563-5566 -- an S7F6 whose body is an
        //   empty L,0, i.e. "no process program".
        ActiveWire->StringOut("S7F6, upload GATED (port) -- replying empty L,0");
        ActiveWire->InitLocalHead(7, 6, 0);
        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        ActiveWire->SendLocalData();
        ActiveWire->StringOut("S7F6, open File fail");

        LookForFile();
        TempString2->Clear();                                                   //JerryYang 20241118 : 優化S7F5時間
        delete TempString2;

        return;
    }
    S9F7_IllegalData("S7,F5 Data Format error !!!");                            // GOLDEN BUG PRESERVED: TempString2 leaks on this path too (golden :5602)
}
#endif

//------------------------------------------------------------------------------
// ProcessS7F23FromatReceipe -- golden SECSGEM/uHGemHT9045.cpp:5826-5927.
// ACTIVE, faithful. Golden misspells "Format" as "Fromat" in the identifier;
// kept. Golden's own body is 4 live statements plus a ~80-line /* */ block of
// disabled code (:5844-5925) -- that block is golden's text and is carried over
// verbatim, NOT deleted and NOT a port-side gate. The only change is the
// Design-D rename HGemPtr-> -> ActiveWire-> on the one LIVE wire call (golden
// :5839). RENAME DISCIPLINE, stated because it is easy to get wrong: golden's
// disabled /* */ block also contains 20 `HGemPtr->` lines and they are LEFT AS
// `HGemPtr->` -- renaming inside golden's own dead text would edit golden's text
// for no compiled benefit. Same rule applied to ProcessS7F25FromatReceipe below,
// where every line is dead and therefore nothing was renamed at all.
// GOLDEN QUIRK, PRESERVED: the live code CONSUMES an L,4 header
// (GetDataItemLenAndTypeAndDelete) and then returns 0 = "accepted" without
// reading any of the 4 items, so its caller S7F24 acknowledges HCACK=0 for a
// message it never actually decoded. Golden's behaviour, not a defect
// introduced here.
//------------------------------------------------------------------------------
int HT9045Gem::ProcessS7F23FromatReceipe()                                      //寫入到檔案
{
//    double dData;
//    int    iData;
    int ret, SVlen;                                                             //,i, j,k,ret1,ret2,len;

//    char PPID[256];
//    char GemSoft[256];
//    char GemMDSN[256];
//    char CCODE[256];
//    AnsiString S,S1;
//    bool bSearchOK=false;

    ret=ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen,HType.LIST_TYPE);
    if(ret!=1)
        return 2;
    if(SVlen!=4)
        return 2;                                                               // format error
/*
    HGemPtr->DataItemIn(256, HType.ASCII_TYPE,PPID);                            //
    HGemPtr->DataItemIn(256, HType.ASCII_TYPE,GemMDSN);                         //
    HGemPtr->DataItemIn(256, HType.ASCII_TYPE,GemSoft);                         //
    S=PPID;
    S=S.UpperCase();
    for(i=0; i<fMain->cbSetupFileName->Items->Count; i++)
    {
        S1=fMain->cbSetupFileName->Items->Strings[i];
        S1=S1.UpperCase();
        if(S==S1)
        {
            bSearchOK=true;
            break;
        }
    }

    if(bSearchOK==false)
        return 4;                                                               // PPID not found

    ret=HGemPtr->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE);
    if(ret!=1)
        return 2;

    S1=CurrentDir+AnsiString("\\data\\")+S+AnsiString(".ini");
    TIniFile *IniFile = new TIniFile(S1);

    for(i=0; i<SVlen; i++)
    {
        ret=HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL);
        if(ret!=1)
        {
            delete IniFile;
            return 2;
        }

        ret=HGemPtr->DataItemIn(256, HType.ASCII_TYPE, CCODE);                  //
        if(ret!=1)
        {
            delete IniFile;
            return 2;
        }

        if(     strcmp(CCODE, "Molding")==0)
        {
            ret=HGemPtr->DataItemIn(13, HType.LIST_TYPE, NULL);
            if(ret!=1)
            {
                delete IniFile;
                return 2;
            }

            HGemPtr->DataItemIn(1, HType.INT_4_TYPE, &iData);
            IniFile->WriteInteger("TabMolding", "edMotorMoldingSpeed", iData);
            HGemPtr->DataItemIn(1, HType.INT_4_TYPE, &iData);
            IniFile->WriteInteger("TabMolding", "edMotorDownDelay", iData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMoldCynDelay1", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMoldCynDelay2", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMoldCynDelay3", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMoldCynDelay4", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMoldCynDelay5", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMoldingTime", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMold1_1_SetTorque", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMold1_2_SetTorque", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMold2_1_SetTorque", dData);
            HGemPtr->DataItemIn(1, HType.FT_8_TYPE,  &dData);
            IniFile->WriteFloat("TabMolding", "edMold2_2_SetTorque", dData);
            HGemPtr->DataItemIn(1, HType.INT_4_TYPE, &iData);
            IniFile->WriteInteger("TabMolding", "edtMoldCount", iData);
        }
    }
    delete IniFile;
*/
    return 0;
}
//------------------------------------------------------------------------------
// S7F24_FormattedProcessProgramSendAcknowledge -- golden :5929-5941.
// ACTIVE, faithful (Design-D rename only). HasICUnderMachine() and SystemStart
// come from the two file-scope declarations at the top of this block.
//------------------------------------------------------------------------------
int HT9045Gem::S7F24_FormattedProcessProgramSendAcknowledge()
{
    unsigned char HCACK=1;
    if(HasICUnderMachine()==true || SystemStart==true)                          //20140124 wei
        HCACK=1;
    else
        HCACK=ProcessS7F23FromatReceipe();

    ActiveWire->InitLocalHead(7, 24, 0);
    ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
    ActiveWire->SendLocalData();
    return HCACK;
}
//------------------------------------------------------------------------------
// ProcessS7F25FromatReceipe -- golden SECSGEM/uHGemHT9045.cpp:5943-6023.
// ACTIVE, faithful. Golden's misspelling kept. EVERY statement in golden is
// already commented out (:5945-5960 as `//` lines, :5961-6021 as one /* */
// block), so golden's live body is exactly `return 0;` -- carried over with
// golden's dead text intact. Zero renames were needed (nothing live to rename).
//------------------------------------------------------------------------------
int HT9045Gem::ProcessS7F25FromatReceipe()                                      //從檔案讀取
{
//    double dData;
//    int    iData;
//    int ret,SVlen,i, j,k,ret1,ret2,len;
//
//    char PPID[256];
//    char GemSoft[256];
//    char GemMDSN[256];
//    char CCODE[256];
//    AnsiString S,S1;
//    bool bSearchOK=false;

//    ret=HGem->GetDataItemLenAndTypeAndDelete(SVlen,HType.LIST_TYPE);
//    if( ret!=1)
//        return 2;
//    if(SVlen!=4)
//        return 2;    // format error
/*
    HGemPtr->DataItemIn(256   ,HType.ASCII_TYPE,PPID);                          //

    S=PPID;
    S=S.UpperCase();
    for(i=0; i<fMain->cbSetupFileName->Items->Count; i++)                       //cb_WorkFile
    {
        S1=fMain->cbSetupFileName->Items->Strings[i];
        S1=S1.UpperCase();
        if(S==S1)
        {
            bSearchOK=true;
            break;
        }
    }

    if(bSearchOK==false)
        return 4;                                                               // PPID not found

    S1=CurrentDir+AnsiString("\\data\\")+S+AnsiString(".ini");
    TIniFile *IniFile = new TIniFile(S1);

    HGemPtr->InitLocalHead(7, 26, 2);
    HGemPtr->DataItemOut(4, HType.LIST_TYPE,NULL);                              // L,4
    HGemPtr->DataItemOut(S.Length(),          HType.ASCII_TYPE, S.c_str()         );                                    //    1. <PPID>
    HGemPtr->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemMDLN   );                //    1. <MDLN>
    HGemPtr->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemSOFTREV);                //    2. <SOFTREV>
    HGemPtr->DataItemOut(1, HType.LIST_TYPE, NULL);                             // L,1
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);                             // L,4
    HGemPtr->DataItemOut(strlen("Molding"), HType.ASCII_TYPE,"Molding");
    HGemPtr->DataItemOut(13, HType.LIST_TYPE,NULL);                             // L,13
    iData=IniFile->ReadInteger("TabMolding", "edMotorMoldingSpeed", 0);
    HGemPtr->DataItemOut(1, HType.INT_4_TYPE, &iData);                          //fSetup->edMotorMoldingSpeed
    iData=IniFile->ReadInteger("TabMolding", "edMotorDownDelay", 0);
    HGemPtr->DataItemOut(1, HType.INT_4_TYPE, &iData);                          //fSetup->edMotorDownDelay
    dData=IniFile->ReadFloat("TabMolding", "edMoldCynDelay1", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMoldCynDelay1
    dData=IniFile->ReadFloat("TabMolding", "edMoldCynDelay2", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMoldCynDelay2
    dData=IniFile->ReadFloat("TabMolding", "edMoldCynDelay3", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMoldCynDelay3
    dData=IniFile->ReadFloat("TabMolding", "edMoldCynDelay4", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMoldCynDelay4
    dData=IniFile->ReadFloat("TabMolding", "edMoldCynDelay5", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMoldCynDelay5
    dData=IniFile->ReadFloat("TabMolding", "edMoldingTime", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMoldingTime
    dData=IniFile->ReadFloat("TabMolding", "edMold1_1_SetTorque", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMold1_1_SetTorque
    dData=IniFile->ReadFloat("TabMolding", "edMold1_2_SetTorque", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMold1_2_SetTorque
    dData=IniFile->ReadFloat("TabMolding", "edMold2_1_SetTorque", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMold2_1_SetTorque
    dData=IniFile->ReadFloat("TabMolding", "edMold2_2_SetTorque", 0.0);
    HGemPtr->DataItemOut(1, HType.FT_8_TYPE,  &dData);                          //fSetup->edMold2_2_SetTorque
    iData=IniFile->ReadInteger("TabMolding", "edtMoldCount", 0);
    HGemPtr->DataItemOut(1, HType.INT_4_TYPE, &iData);                          //fSetup->edtMoldCount
    HGemPtr->SendLocalData();

    delete IniFile;
*/
    return 0;
}
//------------------------------------------------------------------------------
// S7F26_FormattedProcessProgramData -- golden :6025-6037.
// ACTIVE, faithful (Design-D rename only).
// GOLDEN SHAPE KEPT DELIBERATELY: golden dispatches through the global
// `HSys.MyGem->ProcessS7F25FromatReceipe()` rather than calling its own
// `this->ProcessS7F25FromatReceipe()`. In production those are the same object
// (database.cpp's SystemModularInitial assigns `HSys.MyGem = new HT9045Gem`), so
// golden's indirection is a no-op -- but it is golden's text and it is kept.
// RISK RECORDED, NOT PAPERED OVER: golden has no NULL guard, and on this port
// HSys.MyGem stays NULL until SystemModularInitial() runs (main.cpp is
// unported), so calling this method on a standalone HT9045Gem would dereference
// NULL. This is NOT a static-init hazard (nothing here runs before main), and no
// test in tests/ calls S7F26 today. Left faithful.
//------------------------------------------------------------------------------
int HT9045Gem::S7F26_FormattedProcessProgramData()
{
    unsigned char HCACK=1;
    HCACK=HSys.MyGem->ProcessS7F25FromatReceipe();

    if(HCACK!=0)
    {
        ActiveWire->InitLocalHead(7, 26, 0);
        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        ActiveWire->SendLocalData();
    }
    return HCACK;
}
//---------------------------------------------------------------------------
//  END of W906-uHGemHT9045-S7program append block
//---------------------------------------------------------------------------

//==============================================================================
//  APPEND BLOCK -- ROLE: HT9045Gem's equipment-constant CHECK/UPDATE pair, the
//  S5F6 alarm-data list, the S14F4 2DID bin-code intake, S125F4 level-setting
//  acknowledge, S110F5 customer-name list, and the async-RCMD drain hook.
//  APPEND-ONLY: nothing above this line was read-modify-written by this group;
//  three sibling agents are appending their own groups to this same file
//  concurrently (the "registration" group's block sits immediately above).
//
//  Translator: AI(W906-PT-W5-S2F15-S5-S14) 20260809
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemHT9045.cpp,
//                 decoded cp950.  Every Chinese comment below is TRANSCRIBED
//                 verbatim from golden, never paraphrased or re-authored.
//
//  WAVE SCOPE -- one line per golden function
//    * HT9045Gem::S2F15_CheckNewEquipmentConstant       golden :483-690   (208)  ACTIVE + gate [E1]
//    * HT9045Gem::S2F15_UpdateNewEquipmentConstant      golden :692-1104  (413)  ACTIVE + gates [E2][E3][E4][E5][E7][E8] (6)
//    * HT9045Gem::CheckAndExecuteAsyncRCMD              golden :1111-1142 ( 32)  ACTIVE + gate [A2]
//    * HT9045Gem::S5F6_ListAlarmData                    golden :4191-4395 (205)  ACTIVE, NO GATE
//    * HT9045Gem::S14F4_Get2DID_BinCode                 golden :6042-6152 (111)  ACTIVE + gate [D2]
//    * HT9045Gem::S110F5_RequestCustomerNameList        golden :6155-6161 (  7)  ACTIVE, NO GATE
//    * HT9045Gem::S125F4_LevelSettingChangeAcknowledge  golden :6176-6214 ( 39)  ACTIVE + gate [L1]
//  "ACTIVE + gate" = the FUNCTION BODY is live and reachable; only the named
//  sub-blocks inside it are gated.  No function in this group is gated whole.
//  TEN `#if 0` regions remain: [E1][E2][E3][E4][E5][E7][E8][A2][D2][L1].
//
//  AI(W906-PT-W5b-integrate) 20260809 -- A WHOLE DELTA CLASS THIS REGISTER MISSED,
//  added after the adversarial audit pointed it out.  Everything above documents the
//  `#if 0` gates.  But there are also golden calls that are ACTIVE and UNGATED here and
//  still do nothing, because they land on `forms/` facade stubs.  Those are invisible in
//  a gate register by construction -- there is no gate to look at -- and one of them
//  removes a golden REFUSAL path, which is the kind of thing that must not be discovered
//  by a machine:
//    * `fMain->CanChangeSite(...)` -- the ported facade is
//      `return W906_CanChangeSite_Sim`, default **true** (forms/fMain.cpp:336).  Golden's
//      `return 2` ("Denied. Busy") is therefore DEAD, so a host S2F15 write to the
//      site-on/off ECIDs is accepted in states where golden would have refused it.
//      Fail-PERMISSIVE, i.e. the wrong direction for an authority check.
//    * `fMain->cbSetupFileNameChange(fMain)` -- offline counter stub (forms/fMain.cpp:326);
//      see the CORRECTION inside [A2] below.
//    * plus three more ungated fMain calls that resolve to offline stubs.
//  NONE of this is a translation defect -- golden calls these, and so do we; the facade
//  is what is degraded.  It is recorded here because "no gate" reads as "no delta", and
//  for CanChangeSite that reading would be dangerous.
//  [E6], [A1] and [D1] were DRAFTED as gates and RETIRED before hand-off -- see
//  the REACHABILITY CORRECTION immediately below the register.
//
//  ---------------------------------------------------------------------------
//  WIRE-CODEC OBJECT SUBSTITUTION (applies to every body below; NOT a gate)
//  ---------------------------------------------------------------------------
//  Golden reaches the byte-level SECS codec through TWO spellings that are the
//  same live object in practice -- the member `HGemPtr` and the global `HGem`
//  (both `THGem*`); this file's group mixes them freely (e.g. S5F6 golden :4201
//  uses HGemPtr-> for the first read and HGem-> for every read after it).  This
//  port keeps the EXACT precedent already set by the direct sibling
//  SECSGEM/uHGemClass.cpp for its own translations of these same golden bodies
//  (see uHGemClass.cpp:2522-2529 for S5F6 and :4116-4134 for S2F15):
//    * receive/send wire primitives -- GetDataItemLenAndType(AndDelete) /
//      DataItemIn / InitLocalHead / DataItemOut / SendLocalData /
//      LocalAcknowledge / SendInvalidDataMessageToHost -- go through
//      `ActiveWire->` (uHGemClass.h:223), which IS `&HGemPtr->WireCodec` once a
//      live THGem was supplied to the ctor (uHGemHT9045.cpp:46-47) and is the
//      HTGem instance's own by-value codec otherwise.
//    * everything that is genuinely a THGem MEMBER with no codec home stays
//      `HGemPtr->` exactly as golden wrote it: strGrdAlarm, GetAlarmIndex,
//      GemMDLN.
//  This collapses golden's HGem/HGemPtr pair onto the one instance -- the same
//  "deliberate, harmless simplification since golden only ever has one live
//  THGem singleton in practice" already recorded at uHGemClass.cpp:4450-4454.
//  CONSEQUENCE WORTH KNOWING: it also makes the wire half of these bodies
//  reachable in a standalone (HGemPtr==NULL) test, which golden's spelling is
//  not.  The `HGemPtr->` half is NOT so protected -- see CALLER PRECONDITION.
//
//  Two further mechanical (non-behavioural) substitutions, both with precedent:
//    * `_atoi64` -> `strtoll` (Borland RTL name absent under MinGW; identical
//      precedent uHGemClass.cpp:2630/:2645, same wave family).
//    * golden's unqualified `string` / `stringstream` / `std::replace` in
//      S2F15_Update case 3540 -> `std::string` / `std::stringstream`.  Golden
//      compiled these unqualified under BCB6's headers; ISO C++ needs the
//      qualification.  golden already writes `std::replace` qualified on the
//      very next line, so this is purely restoring consistency.
//
//  ---------------------------------------------------------------------------
//  GATE REGISTER
//  ---------------------------------------------------------------------------
//  [E1] golden :685-688 -- S2F15_Check's trailing
//       `if(iECID>=37800 && iECID<=37887) ReadESDDataFile();`
//       WHY THE DEFAULT IS FAITHFUL: not a choice.  `ReadESDDataFile()` is
//       DECLARED at csystem.h:266 and has NO DEFINITION ANYWHERE in this tree
//       (grep + timestamp in this wave's report).  A call is a link error that
//       -fsyntax-only cannot see.  The `if` itself is kept ACTIVE-looking only
//       inside the #if 0 arm; the #else arm evaluates nothing.
//       BEHAVIOUR DELTA ON A REAL MACHINE: golden re-reads the ESD ini
//       (D:\ESD_Program\System\General.ini) into the ESD parameter block whenever the host's
//       S2F15 batch touched an ECID in the ESD band 37800..37887, so a
//       subsequent S2F14/S2F30 read-back reflects the file.  This port skips
//       that refresh, so the ESD band reads back the values held in RAM.  Note
//       this is the CHECK phase (no value has been written yet) -- golden's own
//       ordering means the refresh happens BEFORE the matching UPDATE call.
//
//  [E2] golden :829-841 -- S2F15_Update case 1006's whole
//       `if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS){...}` block (LOTNUMBER /
//       LOTSTART / GETOSSETUP pushes to the OS tester).
//  [E3] golden :843-851 -- case 1007's identical shape (OPERATORID push).
//       WHY THE DEFAULT IS FAITHFUL for both: golden's `fTesterTCP` (golden
//       TesterTCP.h `extern PACKAGE TfTesterTCP *fTesterTCP;`) HAS NO PORTED
//       TYPE AND NO PORTED OBJECT AT ALL -- there is no `class TfTesterTCP`, no
//       shim class, and no `fTesterTCP` pointer anywhere in the ported tree; the
//       ported subset of that form is a free-function + state-struct slice
//       (Interface/TesterTCP_Socket.h, whose own :157-159 says it models "the
//       (single) golden TfTesterTCP instance's data members", not the form
//       pointer).  The blocker is therefore the ABSENT TYPE, not an archive
//       boundary -- and that distinction matters, see the REACHABILITY
//       CORRECTION below: archive boundaries around ht9045_sm are NOT a valid
//       gate reason in this file.  Note the OTHER two symbols in these blocks
//       are real and reachable and are inside the gate ONLY as
//       arguments/guards of the un-portable call:
//       fLotInfo->edtSysLotID / edtSysOperatorID (forms/fLotInfo.h:29/:69,
//       archive ht9045_forms) and iRunStartMode/RT (cmydef.cpp:3340 /
//       cprod.cpp:83).
//       BEHAVIOUR DELTA ON A REAL MACHINE: confined to CUSTOMER_CODE ==
//       CC_SJ_Semiconductor_OS (792, MachineType.h:164) AND
//       TestIF.iTestType == TCP_IP_MODE (3, cmydef.h:64).  On such a machine
//       golden, at the moment the host writes ECID 1006 (lot ID) / 1007
//       (operator ID), forwards LOTNUMBER+LOTSTART+GETOSSETUP / OPERATORID over
//       the tester TCP link so the OS tester starts the same lot.  This port
//       stores the EC value (SetECValue already ran, golden :780) but pushes
//       nothing, so the tester is never told -- the lot would have to be started
//       on the tester by hand.  Every other customer code / test-interface type
//       is bit-identical.
//
//  [E4] golden :930 -- case 1701's `fBarCode->ReadFile();`
//       WHY THE DEFAULT IS FAITHFUL: `fBarCode` DOES exist in this port, but as
//       `TfBarCode` (aHotPlateSubstrate.h:975-1017), a deliberately
//       bounded stand-in whose member list is the scan/trigger family only --
//       it has NO `ReadFile()` member, so golden's call does not even name
//       something that exists here.  THE MISSING MEMBER IS THE WHOLE REASON:
//       its object (aHotPlateSubstrate.cpp, CMakeLists.txt:1466, archive
//       ht9045_sm) IS reachable -- see the REACHABILITY CORRECTION below -- so
//       this gate would be retired by adding ReadFile() to the shim, not by any
//       link change.  The three lines ABOVE the gate in
//       the same case (GetLastOpenFN / MyForceDirectories / WriteIniData of
//       "Check 2DID Allow List Function") are ALL ACTIVE -- the .Data file IS
//       written.
//       BEHAVIOUR DELTA ON A REAL MACHINE: the ini write happens, the in-memory
//       barcode-form re-read does not.  So after the host sets ECID 1701, the
//       persisted HandlerCondition.Data is correct but the running barcode
//       module keeps its previous "Check 2DID Allow List Function" value until
//       the next recipe load re-reads the file -- i.e. the setting takes effect
//       late rather than immediately.
//
//  [E5] golden :1013-1021 -- cases 3616 / 3617 / 3677, the three
//       `fBinSel->Save(<ecid>, eBinFT);` error-bin persist calls.
//       WHY THE DEFAULT IS FAITHFUL: `fBinSel` (golden cBinSel.h) has NO ported
//       home of any kind -- grep of the whole ported tree finds the name only
//       inside build/ dfm2rc layout GENERATORS and one unrelated TU-local
//       mention, never a class or an object (grep + timestamp in the report).
//       The #else arm KEEPS all three case LABELS with a bare `break;` so the
//       switch shape, and the fact that these ECIDs are recognised-but-inert,
//       stay visible in the code rather than silently falling off the switch.
//       BEHAVIOUR DELTA ON A REAL MACHINE: the EC value itself is stored
//       (SetECValue already ran), but golden ALSO flushes the FT bin-selection
//       table to its file at that instant.  This port does not, so an error-bin
//       change made by the host is lost at the next power cycle unless some
//       other path saves the bin table.  Sorting behaviour during the current
//       lot is unaffected.
//
//  [E6] golden :1051-1054 -- case 3540's
//       `if(HasICUnderMachine()==false) fMain->ShowTestHeadComp(true); else
//        fMain->ShowTestHeadComp(false);`
//       WHY THE DEFAULT IS `ShowTestHeadComp(true)`: `HasICUnderMachine()` has a
//       real body (csystem_predicates.cpp:204) but that file is registered in
//       ht9045_sm (CMakeLists.txt:1448), and ht9045_secsgem deliberately does
//       NOT link ht9045_sm (CMakeLists.txt:1214; re-adding that edge is proven
//       to make configure fail, :1207-1213).  So the call is a link error from
//       THIS archive today -- and it is NOT fixed by a future translation, it is
//       fixed by an archive/layering decision the main loop owns.  `false` (no
//       IC anywhere in the machine) is this tree's established offline posture
//       for exactly this predicate family (see forms/fMain.h:439-449, where
//       CanChangeSite's offline default is "nothing blocks it"), so the gate
//       takes golden's `==false` arm.
//       BEHAVIOUR DELTA ON A REAL MACHINE: only when the host pushes a new site
//       map (ECID 3540) WHILE ICs are still inside the machine.  Golden then
//       calls ShowTestHeadComp(FALSE) -- redraw without refreshing from the new
//       map, per its own comment "有開SITE 才恢復畫面關SITE -> 開SITE" -- while
//       this port calls ShowTestHeadComp(TRUE), i.e. it refreshes the head
//       display from the freshly-written map even though the parts in flight
//       were tested under the old one.  Everything the site map actually
//       CONTROLS (TestIF_File.iSiteMap + the HandlerCondition.Data writes above)
//       is identical either way; the delta is the on-screen head picture.  In
//       this port ShowTestHeadComp is itself a no-op facade member
//       (forms/fMain.h:156), so today the delta is nil in either arm.
//
//  [E7] golden :1080-1084 -- the tail `if(bspeed){ fSpeed->DoIniDataToForm(); }`
//       WHY THE DEFAULT IS FAITHFUL: `fSpeed` (golden cSpeed.h) has NO ported
//       home -- same evidence class as fBinSel above (grep + timestamp).  The
//       `bspeed` flag itself, and the whole 60-case speed/retry/destroy ECID
//       label list that sets it plus its MyForceDirectories(szDir) side effect
//       (golden :932-1006), are ACTIVE.
//       BEHAVIOUR DELTA ON A REAL MACHINE: every one of those ~60 motion
//       parameters (arm/shuttle/index/tray speeds, retry counts, destroy
//       intervals, two-speed move-down distances...) is still stored by
//       SetECValue and the recipe directory is still created, but the Speed
//       form's in-memory copy is not re-loaded from the ini, so motion code
//       reading its values THROUGH that form keeps the old numbers until the
//       next recipe load.  Golden's own next line is already commented out
//       (`//fSpeed->SaveSetupFile(szDir);`), which is kept commented.
//
//  [E8] golden :1087-1102 -- the WHOLE trailing `if/else if/else if` chain on
//       ECID (4801..4832 temperature band, 37800..37887 ESD band, ==3545).
//       Gated as ONE block because it is one if/else chain and ALL THREE arms
//       are independently unreachable:
//         arm 1 needs fTemp_Set->DoIniDataToForm(true) and
//                fBuilder->bSaveAllFillOrFile("Temperature",0) -- neither
//                `fTemp_Set` nor `fBuilder` has a ported class/object (grep +
//                timestamp; the only hits are build/ layout generators);
//         arm 2 needs WriteESDDataFile() -- declared csystem.h:267, DEFINED
//                NOWHERE (same class of finding as [E1]).  THAT ALONE is the
//                blocker: its companion SendCommand_ESD() (body
//                Interface/InterfaceSYS.cpp, CMakeLists.txt:1484, ht9045_sm) IS
//                reachable, see the REACHABILITY CORRECTION below;
//         arm 3 needs fSetup->ScrollBar1Change / DoIniDataToForm / sbUpdateClick;
//                `fSetup` DOES exist here (forms/fSetup.h:76) but its facade is
//                deliberately a ONE-FIELD class (`bool fShow;`, :69) and that
//                file's own head note :20-25 states growing it past measured
//                need is refused -- so none of the three methods exists.
//       `ReloadParameter()` on golden :1086, immediately before this chain, is
//       ACTIVE (it is a same-class virtual; the "registration" sibling group
//       translated the real override).
//       BEHAVIOUR DELTA ON A REAL MACHINE, per band:
//         4801..4832 -- host changes a per-site temperature EC: golden reloads
//           the Temp_Set form from the ini and then writes the whole
//           "Temperature" section back through fBuilder; this port stores the EC
//           and does neither, so the temperature UI/file lag behind.
//         37800..37887 -- host changes an ESD EC: golden writes the ESD data
//           file AND tells the external ESD program to re-read it
//           (ESD_Read_ESD_SettDATA); this port does neither, so the ESD unit
//           keeps running on its previous settings.  Together with [E1] this
//           means the ESD band is fully inert in this port -- stored, never
//           applied.
//         3545 -- golden re-runs the Setup form's scrollbar/reload/update
//           cascade; this port stores the EC only.
//
//  [E6] RETIRED -- see the REACHABILITY CORRECTION below.  golden :1051-1054 is
//       ACTIVE.  Also: [E9] never existed as a gate; the ID was reserved in an
//       earlier draft of this register and is deliberately left unused.
//
//  [A1] RETIRED -- see the REACHABILITY CORRECTION below.  golden :1115's full
//       two-term guard `if(SystemStart || HasICUnderMachine()) return;` is
//       ACTIVE, so the deferred-RCMD-while-ICs-are-loaded hazard the draft gate
//       would have introduced does NOT exist in the delivered code.
//
//  [D1] RETIRED -- see the REACHABILITY CORRECTION below.  golden :6052's full
//       two-term guard is ACTIVE.
//
//  [A2] golden :1137 -- `HGemPtr->EventReport(1, SECS_EVENT.SwitchSetupFile);`
//       DEFAULT: the free `EventReport(SECS_EVENT.SwitchSetupFile);`
//       WHY: the ported `class THGem` has NO `EventReport` member at all (grep +
//       timestamp).  Golden's OWN free function of that name is literally
//       `void EventReport(unsigned Ceid){ HGem->EventReport(1, Ceid); }`
//       (golden UsecegemMainFrom.cpp:191, quoted in
//       SECSGEM/SecsEventReport.h:13-14), so calling it is golden's own
//       forwarder, and SecsEventReport.cpp is in THIS archive
//       (CMakeLists.txt:1126) -- no new link edge, no new archive.
//       BEHAVIOUR DELTA ON A REAL MACHINE -- READ THIS, IT IS A STAND-IN:
//       that free function in this port is a SIM counter, not the engine: it
//       records g_SimLastEventReportCeid / g_SimEventReportCount and sends
//       nothing (SecsEventReport.h:24-27,:57-62).  So the host does NOT receive
//       the S6F11 that tells it the deferred PP_SELECT finished; it would have
//       to poll.
//       CORRECTION -- AI(W906-PT-W5b-integrate) 20260809, from the adversarial audit:
//       this entry USED to claim "the recipe switch itself (the two fMain calls above)
//       and the RecordProcess audit line are real."  THAT WAS WRONG about the switch.
//       Only the `cbSetupFileName->Text = PPID` FIELD WRITE is real; the second call,
//       `fMain->cbSetupFileNameChange(fMain)`, resolves to an offline counter stub
//       (`forms/fMain.cpp:326`, declared "offline: no-op" at forms/fMain.h:403-405).
//       So a deferred PP_SELECT does NOT switch recipe in this port -- the delta is the
//       WHOLE switch, not merely the missing S6F11.  The RecordProcess line is real.
//       Retiring [A2] is one line once
//       THGem gains EventReport (or once the free function is rewired to it) --
//       and at that moment the Sim call site becomes correct without further
//       edits, which is why this arm was preferred over gating the call away.
//
//  [D2] golden :6108 -- `HCACK=Process_2DID_BinCode(CommandStr);`
//       DEFAULT: `HCACK=3;` (golden's own "at least one parameter is invalid"
//       code, used by the two sibling `else` arms of the very same `if`).
//       WHY: `Process_2DID_BinCode` is golden's own extern-declared free
//       function (golden :6040) and has NO DEFINITION ANYWHERE in this ported
//       tree -- not even a declaration (grep + timestamp).  Taking its address
//       or calling it is a link error -fsyntax-only cannot see.
//       BEHAVIOUR DELTA ON A REAL MACHINE: a WELL-FORMED S14F3
//       GetAttr(SUBSTRATETYPE) that golden would decode into the 2DID->bin XML
//       map and acknowledge with the decoder's own code (0 on success) is
//       NAK'd with HCACK=3 here, and no bin map is loaded.  2DID-based sorting
//       therefore cannot be provisioned over SECS in this port.
//
//  [L1] golden :6210-6213 -- S125F4's trailing
//       `if(HCACK==1) S9F7_IllegalData(...); else fSecurity->SetLevelSet();`
//       DEFAULT: keep the `if` arm, drop the `else`.
//       WHY: `fSecurity` (golden cSecurity.h) has NO ported class or object, and
//       `SetLevelSet` has ZERO hits of any kind in the ported tree (grep +
//       timestamp).  S9F7_IllegalData is a real inherited virtual
//       (uHGemClass.h:290) and stays ACTIVE.
//       BEHAVIOUR DELTA ON A REAL MACHINE: the level values ARE stored
//       (SetECValue ran for every LSID/LEVEL pair) and the S125F4 acknowledge IS
//       sent, but the Security form never re-applies them, so the operator
//       authority levels in the running UI keep their previous values until the
//       security file is re-read.  On the HCACK==1 path there is no delta at all.
//
//  ---------------------------------------------------------------------------
//  REACHABILITY CORRECTION -- READ BEFORE WRITING A GATE IN THIS FILE
//  ---------------------------------------------------------------------------
//  THREE GATES IN THIS GROUP ([E6], [A1], [D1]) WERE DRAFTED ON A FALSE CLAIM AND
//  RETIRED BEFORE HAND-OFF.  The false claim was:
//      "HasICUnderMachine()'s body is in csystem_predicates.cpp -> archive
//       ht9045_sm; ht9045_secsgem does not link ht9045_sm (CMakeLists.txt:1214,
//       cycle proof :1207-1213); therefore calling it from this TU is a link
//       error."
//  The first two facts are true.  THE CONCLUSION IS NOT, and this tree already
//  disproves it in two independent ways:
//    1. uHGemClass.cpp -- the SAME archive -- `#include "csystem.h"` at :672 and
//       calls HasICUnderMachine() BARE AND UNGATED at :1639 and :1647 (inside
//       HTGem::S2F16_NewEquipmentConstantSendAcknowledge).  That has been in the
//       tree for waves with the build green.
//    2. Every executable that links ht9045_secsgem links it inside a
//       `$<LINK_GROUP:RESCAN, ht9045_sm, ht9045_secsgem, ...>` that ALSO lists
//       ht9045_sm -- all 18 such targets, tests/CMakeLists.txt:291/:324/:348/
//       :377/:404/:469/:499/:548/:576/:606/:637/:673/:712/:744/:776/:810/:839/
//       :868.  This is the same object-file-level back-edge CMakeLists.txt:
//       1180-1199 already documents for database.cpp <-> HT9045Gem, and the
//       RESCAN group is its documented, applied fix.
//  SO: "the body lives in ht9045_sm" is NOT, by itself, a valid gate reason in
//  this file.  A valid reason is one of: (a) the symbol has NO DEFINITION
//  ANYWHERE (gates [E1] ReadESDDataFile, [E8] arm 2 WriteESDDataFile, [D2]
//  Process_2DID_BinCode); (b) the TYPE or OBJECT does not exist in the ported
//  tree at all (gates [E2]/[E3] fTesterTCP, [E5] fBinSel, [E7] fSpeed, [E8]
//  arm 1 fTemp_Set/fBuilder, [L1] fSecurity); (c) the object exists but the
//  MEMBER golden calls does not (gates [E4] fBarCode->ReadFile, [E8] arm 3
//  fSetup's three methods, [A2] THGem::EventReport).  Every remaining gate in
//  the register above is in exactly one of those three categories, re-checked
//  after this correction.
//
//  ---------------------------------------------------------------------------
//  CALLER PRECONDITION (shared with the sibling block above -- same hazard)
//  ---------------------------------------------------------------------------
//  S5F6_ListAlarmData and S110F5_RequestCustomerNameList dereference `HGemPtr`
//  for genuine THGem members (strGrdAlarm / GetAlarmIndex / GemMDLN).  In this
//  port HGemPtr is whatever database.cpp:187 handed the ctor, and the global
//  `THGem *HGem = NULL;` stays NULL until the still-unported main.cpp builds the
//  engine.  DO NOT CALL those two until a live THGem exists.  No NULL guard was
//  added: golden has no such branch, and inventing one turns a loud wiring bug
//  into a silent "zero alarms reported" -- the identical reasoning the sibling
//  block records at its own CALLER PRECONDITION.  The other five bodies in this
//  group touch only ActiveWire / fMain / plain globals and are safe to call
//  today (ActiveWire is never NULL -- HTGem's ctor points it at its own
//  by-value WireCodec).
//
//  NOTHING IN THIS GROUP RUNS BEFORE main().  No object with static storage
//  duration is defined by this block, and HT9045Gem is only ever heap-built
//  from SYSTEM_MODULAR::SystemModularInitial (database.cpp:187), so none of
//  these bodies can touch one of the 18 NULL globals of
//  docs/PT_CAMPAIGN_PLAN.md section 8 during static init.
//
//  ---------------------------------------------------------------------------
//  GOLDEN BUGS PRESERVED, NOT FIXED (all verified by direct read of golden)
//  ---------------------------------------------------------------------------
//  (a) S5F6 golden :4280-4281 and :4292-4293 -- `unsigned int ALID;` (4 bytes)
//      is handed to `DataItemOut(1, HType.INT_8_TYPE, &ALID)`, and that codec
//      path does `__int64 *ptr=(__int64*)P; j=ptr[0];`
//      (SECSGEM/SecsWireCodec.cpp:639-647 with DataSize 8, :438-439) -- an
//      8-byte read out of a 4-byte object, i.e. a 4-byte stack over-read whose
//      high half becomes whatever is adjacent, and the ALARM ID GOES ON THE WIRE
//      AS AN 8-BYTE ITEM WITH GARBAGE IN THE TOP HALF.  Golden's own S5F6 sibling
//      in the base class uses a real 64-bit variable for the same item
//      (uHGemClass.cpp:2645), and golden's own SVlen==0 branch of THIS function
//      (:4385) uses UINT_4_TYPE for the same `ALID`.  KEPT VERBATIM -- both
//      sites -- because "do not fix a golden bug" is this campaign's rule.  A
//      future fixer wants `unsigned __int64 ALID;`, which changes no wire shape
//      on the two INT_8 sites and only widens the :4385 UINT_4 store.
//  (b) S5F6 golden :4290 -- `DataItemOut(0, HType.BINARY_TYPE, &ALCD)` in the
//      unknown-alarm branch: length 0, so the ALCD byte assigned on the line
//      above is never transmitted (a genuine zero-length SECS-II item).  Same
//      quirk the base class already records at uHGemClass.cpp:2663-2673.  KEPT.
//  (c) S5F6 golden :4215 -- `Type` is RE-READ inside the branch that already
//      validated it.  If the second read returns a type outside the six numeric
//      ones, all six `else if`s miss, `ret` keeps the 1 from :4215, `if(ret==1)`
//      is taken, and the code indexes whichever of the six pointers was never
//      allocated.  KEPT verbatim (the base sibling has the same shape).
//  (d) S5F6 golden :4201/:4393 -- when the FIRST GetDataItemLenAndType fails,
//      golden falls straight out of the `if` to `SendLocalData()` and transmits
//      whatever the local buffer happens to hold, instead of answering
//      S9F7_IllegalData as the base class does (uHGemClass.cpp:2802-2805).
//      KEPT.
//  (e) S2F15_Update golden :702/:1087 -- `unsigned int ECID` is declared
//      uninitialised and READ AFTER the loop (`if(ECID>=4801...)`).  With
//      EClen==0 the loop body never runs and the post-loop chain reads an
//      indeterminate value.  KEPT (the chain is inside gate [E8], so today the
//      read is only the `ECID>=4801` comparison itself).
//  (f) S2F15_Update golden :777/:789 vs :624 -- the ASCII branch here DOES
//      `delete[] Str` (:789), but S2F15_Check's ASCII branch (:623-633) leaks
//      it on every call, and golden even shows the fix commented out (":631
//      //  delete[] Str;  ... 先mark").  BOTH kept exactly as golden has them.
//  (g) S2F15_Update golden :1035-1039 -- `while(ss >> temp){ iArry[k]=temp; k++; }`
//      writes into a 40-int array with NO bound check, from a host-supplied
//      comma list.  A host sending >40 site-map values overruns the stack array.
//      KEPT verbatim.  (The consumer loop below it is correctly bounded by
//      MAX_SOCKET_ROW*MAX_SOCKET_COL == 32, MachineType.h:391-392.)
//  (h) S2F15_Update golden :853-866 -- case 1513's `else return -1;` abandons
//      the REST of the host's EC batch mid-list (the outer `for` never finishes),
//      leaving earlier pairs applied and later pairs untouched, with no rollback.
//      KEPT.
//  (i) S2F15_Check golden :539/:685 -- `iECID` is likewise declared
//      uninitialised, but the `EClen<1 -> return -1` guard at :519 means the
//      loop always runs at least once, so unlike (e) this one cannot actually be
//      read indeterminate.  Noted for symmetry only; nothing changed.
//  (j) S125F4 golden :6183-:6207 -- `LocalAcknowledge` sits INSIDE the
//      `GetDataItemLenAndTypeAndDelete(...)==1` block, so on a read failure the
//      host gets NO S125F4 reply at all, while HCACK stays 0 and the tail
//      therefore takes the (gated) fSecurity branch as if everything succeeded.
//      KEPT.
//  (k) S125F4 golden :6189-:6194 -- `DataItemIn(len,Type,&LSID)` /
//      `&LEVEL` pass `int*` for whatever Type the host sent; a UINT_8/FT_8 item
//      would have the codec write 8 bytes into a 4-byte int.  Same class as (a).
//      KEPT.
//  (l) S14F4 golden :6135-:6145 -- the `if(HCACK==0){...} else {...}` emits
//      byte-for-byte IDENTICAL bodies in both arms.  KEPT (not collapsed).
//  (m) S14F4 golden :6050/:6131 -- `new char[10000000]` (10 MB) is `delete[]`d
//      only on the path that reaches :6131; the early `HCACK=4` path at :6054
//      returns at :6151 WITHOUT deleting, leaking 10 MB per rejected request.
//      KEPT verbatim.  Gate [D1] makes that path RARER in this port than in
//      golden, which is a side effect worth naming, not a fix.
//==============================================================================

// Includes for this block only, appended rather than merged into the file head
// so that no existing line is touched (three sibling agents append to this same
// file).  All are include-guarded, so repeating any of a sibling's is a no-op.
#include "MachineType.h"                    // golden :6  -- CC_*/ebctUninstall/eBinFT/MAX_SOCKET_*
#include "cprod.h"                          // golden :11 -- TestIF/TestIF_File/Temperature/
                                            //   DeviceForm_File/IniConfig, and the `RT` constant
#include "cmydef.h"                         // golden :24 -- SystemStart/BAR_CODE_INSTALL/BOTTOM_2DID/
                                            //   iRunStartMode/bRefreshFunction/bTestSiteUse/Tempture_Hot
#include "LastSet.h"                        // PORT split of golden's cprod.h chain -- LastSet
#include "common.h"                         // golden :18 -- GetLastOpenFN/MyForceDirectories/
                                            //   WriteIniData/AuthPath
#include "cMyDB.h"                          // golden :22 -- RecordProcess
#include "csystem.h"                        // golden :15 -- HasICUnderMachine().  ADDED LATE, see the
                                            //   REACHABILITY CORRECTION note in the GATE REGISTER: three
                                            //   gates were RETIRED once this was proven linkable.  The
                                            //   direct precedent is uHGemClass.cpp:672 (same include, same
                                            //   archive) with two live calls at :1639/:1647.
#include "FormsFacade.h"                    // PORT equivalent of golden :8 "main.h" + :17 "uLotInfo.h"
                                            //   -- fMain, fLotInfo
#include "SECSGEM/SecsEventType.h"          // PORT: SECS_EVENT (golden declares it in uHGemHT9045.h)
#include "SECSGEM/SecsEventReport.h"        // PORT: free EventReport(unsigned) -- gate [A2]
#include <sstream>                          // golden :3 (verbatim)
#include <string>
#include <algorithm>
#include <cstdlib>
#include "BarCode/BarCode.h"                // AI(W906-BC-READFILE) 20260923: fBarCode->ReadFile() -- GATE [E4] retired

//==============================================================================
//  V 1.0
//     ret
//     0   = Acknowledge
//     1   = Denied. At least one constant does not exist
//     2   = Denied. Busy
//     3   = Denied. At least one constant out of range
//    >3   = Other equipment-specific error
//    4-63   Reserved
//==============================================================================
int HT9045Gem::S2F15_CheckNewEquipmentConstant()                                //wei 20170417 (Steven) add S2F15
{
    int EClen, i, len, ret=0;
    unsigned char Type;

    __int64  int8EC;                                                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    int   int4EC;
    short int2EC;
    char  int1EC;
    unsigned __int64 uint8EC;                                                   //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    unsigned int   uint4EC;
    AnsiString ECID;
    int iECID;
    unsigned short uint2EC;
    unsigned char  uint1EC;
    float          floatEC;
    double         doubleEC;
//    void *P;

    /*
        L,n
        1. L,2
        1. <ECID1>
        2. <ECV1>
        2. L,2
        .
        .
        n. L,2
        1. <ECIDn>
        2. <ECVn>
    */

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(EClen, Type)!=1)
        return -1;
    if(Type!=HType.LIST_TYPE)
        return -1;
    if(EClen<1)
        return -1;
    for(i=0; i<EClen; i++)
    {
        if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)!=1)
            return -1;
        if(Type!=HType.LIST_TYPE || len!=2)
            return -1;

        ret=1;
        if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
        {
            if(ActiveWire->DataItemIn(len, Type, ECID)!=1)
                return -1;
        }
        else
        {
            return -1;
        }

        iECID=atoi(ECID.c_str());

        if(iECID==1530 || iECID==1531)
        {
            if((LastSet.iTemperature==Tempture_Hot && fMain->CanChangeSite(true)==false) ||
               (LastSet.iTemperature!=Tempture_Hot && fMain->CanChangeSite(false)==false))
            {
                return 2;
            }
        }
        else if(iECID==1513)
        {
            if(BAR_CODE_INSTALL!=ebctUninstall &&
               (TestIF_File.bEnableBarCode ||
               (BOTTOM_2DID && TestIF_File.bEnableBottom2D)) &&
                TestIF_File.bSortingBy2DIDList==true)                           //Steven 20190422 : Bottom 2DID
            {
            }
            else
            {
                return -1;
            }
        }

        if(ActiveWire->GetDataItemLenAndType(len, Type)==1)
        {
            if(Type==HType.UINT_1_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &uint1EC)==1)
                    ret=CheckECValue(ECID, &uint1EC);
                else
                    return -1;
            }
            else if(Type==HType.UINT_2_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &uint2EC)==1)
                    ret=CheckECValue(ECID, &uint2EC);
                else
                    return -1;
            }
            else if(Type==HType.UINT_4_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &uint4EC)==1)
                    ret=CheckECValue(ECID, &uint4EC);
                else
                    return -1;
            }
            else if(Type==HType.UINT_8_TYPE)                                    //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                if(ActiveWire->DataItemIn(1, Type, &uint8EC)==1)
                    ret=CheckECValue(ECID, &uint8EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_1_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &int1EC)==1)
                    ret=CheckECValue(ECID, &int1EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_2_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &int2EC)==1)
                    ret=CheckECValue(ECID, &int2EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_4_TYPE)
            {
                if(ActiveWire->DataItemIn(1, Type, &int4EC)==1)
                    ret=CheckECValue(ECID, &int4EC);
                else
                    return -1;
            }
            else if(Type==HType.INT_8_TYPE)                                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                if(ActiveWire->DataItemIn(1, Type, &int8EC)==1)
                    ret=CheckECValue(ECID, &int8EC);
                else
                    return -1;
            }
            else if(Type==HType.ASCII_TYPE)
            {
                char *Str;
                Str=new char [(size_t)len+100];
                if(ActiveWire->DataItemIn(len, Type, Str)==1)
                {
                    ret=CheckECValue(ECID, Str);
                }
                else
                {
//                    delete[] Str;                                             //Steven 20160912 : Add delete for save memory, 先mark
                    return -1;
                }
            }
            else if(Type==HType.BINARY_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                    ret=CheckECValue(ECID, &int1EC);
                else
                    return -1;
            }
            else if(Type==HType.BOOLEAN_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                    ret=CheckECValue(ECID, &int1EC);
                else
                    return -1;
            }
            else if(Type==HType.FT_4_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &floatEC)==1)
                    ret=CheckECValue(ECID, &floatEC);
                else
                    return -1;
            }
            else if(Type==HType.FT_8_TYPE)
            {
                if(ActiveWire->DataItemIn(len, Type, &doubleEC)==1)
                {
                    ret=CheckECValue(ECID, &doubleEC);
                }
                else
                {
                    return -1;
                }
            }
            else                                                                // error format (SECS-II ASCII code ir correct ,but ITRI is failure,need confirm with ITRI
            {
                ActiveWire->SendInvalidDataMessageToHost("error format");
                return -1;
            }

            if(ret!=0)
            {
                return ret;
            }
        }
        else
        {
            ActiveWire->SendInvalidDataMessageToHost("GetDataItemLenAndType Error");
            return -1;
        }
    }

// GATE [E1] -- golden :685-688.  ReadESDDataFile() is declared (csystem.h:266)
// and DEFINED NOWHERE in this tree; the call is a link error -fsyntax-only cannot
// see.  Golden text kept verbatim below.  Retirement: flip the arms once
// ReadESDDataFile has a body in a CMakeLists-registered .cpp this archive links.
#if 0
    if(iECID>=37800 && iECID<=37887)                                            //kevin 20181206  20180914 (Steven) : add read ESD data D:\ESD_Program\System\General.ini
    {
        ReadESDDataFile();
    }
#else
    // E1 DEFAULT: the ESD-band file refresh is skipped; `iECID` is otherwise
    // unused after the loop, so nothing else changes.  See the GATE REGISTER.
    (void)iECID;
#endif
    return 0;
}
//------------------------------------------------------------------------------
int HT9045Gem::S2F15_UpdateNewEquipmentConstant()
{
    int EClen, i, x, y, len, z;
    unsigned char Type;

    __int64  int8EC;                                                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    int   int4EC;
    short int2EC;
    char  int1EC;
    unsigned __int64 uint8EC;                                                   //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
    unsigned int   uint4EC, ECID;
    unsigned short uint2EC;
    unsigned char  uint1EC;
    float          floatEC;
    double         doubleEC;
    AnsiString S, FileName="", SitMapData="", SitMapDataBuffer="", SData="", sIni="";                                   //kevin 20181107 add
//    int iArry[64]={0};
//    void *P;
    bool b151920=false;
    bool bspeed=false;                                                          //kevin 20181210 add speed
    FileName=GetLastOpenFN();
    AnsiString szDir="";
    AnsiString str;

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(EClen, Type)==1)
    {
        if(Type==HType.LIST_TYPE)
        {
            for(i=0; i<EClen; i++)
            {
                if(ActiveWire->GetDataItemLenAndTypeAndDelete(len, Type)==1)
                {
                    if(Type!=HType.LIST_TYPE || len!=2)
                        break;
                    // get ECID

                    ActiveWire->GetDataItemLenAndType(len, Type);
                    ActiveWire->DataItemIn(len, Type, S);
                    ECID=(unsigned)atoi(S.c_str());

                    ActiveWire->GetDataItemLenAndType(len, Type);

                    if(Type==HType.UINT_1_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint1EC)==1)
                            SetECValue(ECID, &uint1EC);
                    }
                    else if(Type==HType.UINT_2_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint2EC)==1)
                            SetECValue(ECID, &uint2EC);
                    }
                    else if(Type==HType.UINT_4_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint4EC)==1)
                            SetECValue(ECID, &uint4EC);
                    }
                    else if(Type==HType.UINT_8_TYPE)                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        if(ActiveWire->DataItemIn(1, Type, &uint8EC)==1)
                            SetECValue(ECID, &uint8EC);
                    }
                    else if(Type==HType.INT_1_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int1EC)==1)
                            SetECValue(ECID, &int1EC);
                    }
                    else if(Type==HType.INT_2_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int2EC)==1)
                            SetECValue(ECID, &int2EC);
                    }
                    else if(Type==HType.INT_4_TYPE)
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int4EC)==1)
                            SetECValue(ECID, &int4EC);
                    }
                    else if(Type==HType.INT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        if(ActiveWire->DataItemIn(1, Type, &int8EC)==1)
                            SetECValue(ECID, &int8EC);
                    }
                    else if(Type==HType.ASCII_TYPE)
                    {
                        char *Str;
                        Str=new char [(size_t)len+100];
                        if(ActiveWire->DataItemIn(len, Type, Str)==1)
                        {
                            SetECValue(ECID, Str);
                            SData= Str;                                         //kevin 20181116 add
                        }

                        if(ECID==3540)                                          //kevin 20181107 add  SITE MAP
                        {
                            SitMapData=Str;
                            SitMapDataBuffer=Str;
                        }
                        delete[] Str;                                           //kevin 20181116 add
                    }
                    else if(Type==HType.BINARY_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                            SetECValue(ECID, &int1EC);
                    }
                    else if(Type==HType.BOOLEAN_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &int1EC)==1)
                        {
//                            if(int1EC==0x00)                                  //Steven 20230815 : 將ASCII轉成CHAR避免0x00或0x01造成不可視字元
//                                int1EC='0';
//                            else
//                                int1EC='1';
                            SetECValue(ECID, &int1EC);
                        }
                    }
                    else if(Type==HType.FT_4_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &floatEC)==1)
                            SetECValue(ECID, &floatEC);
                    }
                    else if(Type==HType.FT_8_TYPE)
                    {
                        if(ActiveWire->DataItemIn(len, Type, &doubleEC)==1)
                            SetECValue(ECID, &doubleEC);
                    }
                    else                                                        // error format (SECS-II ASCII code ir correct ,but ITRI is failure,need confirm with ITRI
                    {
                        ActiveWire->SendInvalidDataMessageToHost("error format");
                        return -1;
                    }
                    int ia=0, iArry[40];
                    int temp, k=0;
                    AnsiString sPath="";                                        //JerryYang 20250613 Add

                    switch(ECID)
                    {
                        case 1006:
// GATE [E2] -- golden :829-841.  `fTesterTCP` (golden TfTesterTCP form pointer) has
// NO ported object; the ported slice is a free-function/state-struct subset in
// ht9045_sm (Interface/TesterTCP_Socket.cpp, CMakeLists.txt:1496), an archive this
// one must not link.  fLotInfo->edtSysLotID and iRunStartMode/RT ARE real -- they
// are inside the gate only as arguments of the un-linkable call.
#if 0
                            if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS)
                            {
                                if(TestIF.iTestType==TCP_IP_MODE)                                                       //Steven 20230213 : For SJSemi OS Tester
                                {
                                    if(iRunStartMode==RT)
                                        str.sprintf("LOTNUMBER,%s,RT,", fLotInfo->edtSysLotID->Text);
                                    else
                                        str.sprintf("LOTNUMBER,%s,FT,", fLotInfo->edtSysLotID->Text);
                                    fTesterTCP->SendTCPIPCommand(0, "LOTNUMBER", str);
                                    fTesterTCP->SendTCPIPCommand(0, "LOTSTART", "LOTSTART");
                                    fTesterTCP->SendTCPIPCommand(0, "Get OS Setup", "GETOSSETUP");                      //Steven 20230505 : 取得OS Tester資訊
                                }
                            }
#else
                            // E2 DEFAULT: no tester-TCP push.  The EC value itself was
                            // already stored by SetECValue above.  See GATE REGISTER.
#endif
                            break;
                        case 1007:
// GATE [E3] -- golden :843-851.  Same `fTesterTCP` finding as [E2].
#if 0
                            if(CUSTOMER_CODE==CC_SJ_Semiconductor_OS)
                            {
                                if(TestIF.iTestType==TCP_IP_MODE)               //Steven 20230213 : For SJSemi OS Tester
                                {
                                    str.sprintf("OPERATORID,%s,", fLotInfo->edtSysOperatorID->Text);
                                    fTesterTCP->SendTCPIPCommand(0, "OPERATORID", str);
                                }
                            }
#else
                            // E3 DEFAULT: no OPERATORID push; value stored, not forwarded.
#endif
                            break;
                        case 1513:
                            if(BAR_CODE_INSTALL!=ebctUninstall &&
                               (TestIF_File.bEnableBarCode ||                   //Ifor 20190129 : add Cognex EtherNet 通訊
                               (BOTTOM_2DID && TestIF_File.bEnableBottom2D)) &&
                                TestIF_File.bSortingBy2DIDList==true)           //Steven 20190422 : Bottom 2DID
                            {
                                fMain->ChangeTesterConnect(LastSet.iTester, false, true);
                                fMain->LoadRunModePicture();
                                fMain->SetStartModeData();
                            }
                            else
                            {
                                return -1;
                            }
                            break;
                        case 1514:
                            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                 //kevin 20181116 add
                            {
                                if(LastSet.iTemperature==0)
                                    Temperature.iMachineTempMode=1;             //Ambient
                                else if(LastSet.iTemperature==1)
                                    Temperature.iMachineTempMode=0;             //Hot;
                                else if(LastSet.iTemperature==3)
                                    Temperature.iMachineTempMode=3;             //Ambient_HOT
                            }
                            break;
                        case 1519:                                              //kevin 20181206 add
                            fMain->edWorkTemperBase->Text=doubleEC;
                            b151920=true;
                            break;
                        case 1520:
                            fMain->edSoakTime->Text=doubleEC;
                            b151920=true;
                            break;
                        case 1530:
                            for(x=0; x<MAX_SOCKET_ROW; x++)
                            {
                                for(y=0; y<MAX_SOCKET_COL; y++)
                                {
                                    z=x*MAX_SOCKET_COL+y;
                                    if(z<fMain->tSiteOnOff[0]->Count)
                                        LastSet.bUseTestSocket[0][x][y]=atoi(fMain->tSiteOnOff[0]->GetString(z).c_str());   // golden: ->Strings[z].c_str() -- vclcompat StringsProxy has no c_str (established substitution, mycylin.cpp:826/844, uHGemEquipment.cpp:1254)
                                    else
                                        LastSet.bUseTestSocket[0][x][y]=false;
                                    bTestSiteUse[0][x][y]=LastSet.bUseTestSocket[0][x][y];                              //wei 20170417 (Steven) 顯示問題
                                    if(IniConfig.bA09_ByArmCloseSite==false)
                                    {
                                        LastSet.bUseTestSocket[1][x][y]=LastSet.bUseTestSocket[0][x][y];
                                        bTestSiteUse[1][x][y]=LastSet.bUseTestSocket[0][x][y];
                                    }
                                }
                            }
                            fMain->ShowTestHeadComp(false);
                            break;
                        case 1531:
                            for(x=0; x<MAX_SOCKET_ROW; x++)
                            {
                                for(y=0; y<MAX_SOCKET_COL; y++)                                                         //Eliot 2009_12_25 4->8
                                {
                                    z=x*MAX_SOCKET_COL+y;
                                    if(z<fMain->tSiteOnOff[1]->Count)
                                        LastSet.bUseTestSocket[1][x][y]=atoi(fMain->tSiteOnOff[1]->GetString(z).c_str());   // golden: ->Strings[z].c_str() -- same substitution as case 1530 above
                                    else
                                        LastSet.bUseTestSocket[1][x][y]=false;

                                    bTestSiteUse[1][x][y]=LastSet.bUseTestSocket[1][x][y];                              //wei 20170417 (Steven) 顯示問題
                                }
                            }
                            fMain->ShowTestHeadComp(false);
                            break;
                        case 1701:                                              //JerryYang 20250320 : add ECID
                            sIni=GetLastOpenFN();
                            szDir.sprintf("%s%s", DataPath, sIni);
                            MyForceDirectories(szDir);
                            szDir+="\\HandlerCondition.Data";

                            WriteIniData(szDir, "Configuration", "Check 2DID Allow List Function", int1EC);
// GATE [E4] RETIRED -- AI(W906-BC-READFILE) 20260923.  The gate's own note said
// it "would be retired by adding ReadFile() to the shim, not by any link change",
// and that is what happened: golden BarCode/BarCode.cpp:663-1041 is now
// TfBarCode::ReadFile (BarCode/BarCode.cpp, same library ht9045_sm).  The class
// is also no longer called a shim -- it carries golden's own name now.
// BEHAVIOUR CHANGE, deliberate and golden-faithful: the ini WRITE above was
// already ACTIVE, but the in-memory barcode module was NOT re-read, so an
// EC 1701 change only took effect at the next recipe load.  It now takes effect
// immediately, which is what golden :930 does.
                            fBarCode->ReadFile();
                            break;
                        case 2501:                                              //Input Arm vacuum wait time
                        case 2502:                                              //Index Arm vacuum wait time
                        case 2503:                                              //Output Arm vacuum wait time
                        case 2504:                                              //Tray Arm vacuum wait time
                        case 2511:                                              //Input Arm destroy wait time
                        case 2512:                                              //Index Arm destroy wait time
                        case 2513:                                              //Output Arm destroy wait time
                        case 2514:                                              //Tray Arm destroy wait time
                        case 8501:                                              //In Arm Speed
                        case 8502:                                              //Shuttle Speed
                        case 8503:                                              //Index Arm Speed
                        case 8504:                                              //Out Arm Speed
                        case 8505:                                              //Tray Arm Speed
                        case 8506:                                              //Elevator Speed
                        case 8507:                                              //Shuttle 2 Speed
                        case 8508:                                              //In Rotate Speed
                        case 8509:                                              //Out Rotate Speed
                        case 8510:                                              //In Arm Z Speed
                        case 8511:                                              //In Arm X Pitch Speed
                        case 8512:                                              //In Arm Y Pitch Speed
                        case 8513:                                              //Out Arm Z Speed
                        case 8514:                                              //Out Arm X Pitch Speed
                        case 8515:                                              //Out Arm Y Pitch Speed
                        case 8527:                                              //Step Shuttle
                        case 8528:                                              //Tray Arm Retry Count
                        case 8529:                                              //Tray Arm Head Down Time
                        case 8530:                                              //Auto Speed
                        case 8534:                                              //Index Retry Count
                        case 8535:                                              //Index Retry Down Distance
                        case 8536:                                              //Index Destroy Again Interval Time
                        case 8537:                                              //Index Destroy Again Count
                        case 8538:                                              //Socket Device Floating Check
                        case 8540:                                              //In Arm ADC Speed
                        case 8541:                                              //Shuttle ADC Speed
                        case 8542:                                              //Index Arm ADC Speed
                        case 8543:                                              //Out Arm ADC Speed
                        case 8544:                                              //Tray Arm ADC Speed
                        case 8545:                                              //Elevator ADC Speed
                        case 8546:                                              //Shuttle 2 ADC Speed
                        case 8547:                                              //In Rotate ADC Speed
                        case 8548:                                              //Out Rotate ADC Speed
                        case 8549:                                              //In Arm Z ADC Speed
                        case 8550:                                              //In Arm X Pitch ADC Speed
                        case 8551:                                              //In Arm Y Pitch ADC Speed
                        case 8552:                                              //Out Arm Z ADC Speed
                        case 8553:                                              //Out Arm X Pitch ADC Speed
                        case 8560:                                              //In Arm Retry Count
                        case 8561:                                              //In Arm Retry Down Distance
                        case 8562:                                              //In Arm Destroy Again Interval Time
                        case 8563:                                              //In Arm Destroy Again Count
                        case 8564:                                              //In Arm Pitch Function
                        case 8565:                                              //In Arm Destroy Check Time
                        case 8566:                                              //In Arm Destroy Check Need Pause
                        case 8567:                                              //In Arm Two Speed Move Down
                        case 8568:                                              //In Arm Two Speed Move Down Speed
                        case 8569:                                              //In Arm Two Speed Move Down ADC
                        case 8570:                                              //In Arm Two Speed Move Down Distance
                        case 8571:                                              //In Arm Auto Skip
                        case 8572:                                              //In Arm Auto Skip Count
                        case 8573:                                              //In Arm Wait On Shuttle Time
                        case 8574:                                              //Out Arm Retry Count
                        case 8575:                                              //Out Arm Retry Down Distance
                        case 8576:                                              //Out Arm Destroy Again Interval Time
                        case 8577:                                              //Out Arm Destroy Again Count
                        case 8578:                                              //Out Arm Pitch Function
                        case 8579:                                              //Out Arm Destroy Check Time
                        case 8580:                                              //Out Arm Destroy Check Need Pause
                        case 8581:                                              //Out Arm Two Speed Move Down
                        case 8582:                                              //Out Arm Two Speed Move Down Speed
                        case 8583:                                              //Out Arm Two Speed Move Down ADC
                        case 8584:                                              //Out Arm Two Speed Move Down Distance
                            bspeed=true;                                        //kevin 20181210 add speed
                            szDir.sprintf("%s%s", DataPath, FileName);
                            MyForceDirectories(szDir);
                            break;
                        case 2622:
                            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                 //kevin 20181116 add
                            {
                                DeviceForm_File.ForcePerPinN=DeviceForm_File.ForcePerPinG/1000.0*9.8;
                            }
                            break;
// GATE [E5] -- golden :1013-1021.  `fBinSel` (golden cBinSel.h) has NO ported class
// or object anywhere in this tree.  The #else arm KEEPS all three case labels so the
// ECIDs stay visibly recognised-but-inert instead of silently leaving the switch.
#if 0
                        case 3616:
                            fBinSel->Save(3616, eBinFT);                        //kevin 20181120       //error bin
                            break;
                        case 3617:
                            fBinSel->Save(3617, eBinFT);                        //kevin 20181120
                            break;
                        case 3677:
                            fBinSel->Save(3677, eBinFT);                        //kevin 20181120
                            break;
#else
                        case 3616:
                            // E5 DEFAULT: EC stored; FT bin table not flushed to file.
                            break;
                        case 3617:
                            // E5 DEFAULT (same).
                            break;
                        case 3677:
                            // E5 DEFAULT (same).
                            break;
#endif
                        case 35032:                                             //JerryYang 20250613 Add
                            sPath=AuthPath+"config.ini";
                            WriteIniData(sPath, "Function", "bA68_AutoLoadUnload", IniConfig.bA68_AutoLoadUnload);
                            break;
                        case 3540:                                                                                      //kevin 20181107 add sitemap
                            szDir.sprintf("%s%s", DataPath, S);
                            MyForceDirectories(szDir);
                            szDir+="\\HandlerCondition.Data";

                            std::string sData= SitMapData.c_str();
                            std::replace(sData.begin(), sData.end(), ',', ' ');                                         // replace ':' by ' '
                            std::stringstream ss(sData);

                            while(ss >> temp)
                            {
                                iArry[k]=temp;
                                k++;
                            }
                            for(int i=0; i<MAX_SOCKET_ROW; i++)
                            {
                                for(int j=0; j<MAX_SOCKET_COL; j++)
                                {
                                    TestIF_File.iSiteMap[i][j]= iArry[ia];
                                    ia++;
                                    str.sprintf("Site %c%c", i+'A', j+'a');                                             //kevin 20180312 j+'A' ==> j+'a'
                                    WriteIniData(szDir, "Configuration", str, TestIF_File.iSiteMap[i][j]);
                                }
                            }

// [E6] RETIRED before hand-off -- NOT a gate.  It was drafted as one on the claim
// that HasICUnderMachine() (body csystem_predicates.cpp:204, archive ht9045_sm) is
// not linkable from ht9045_secsgem.  THAT CLAIM WAS FALSE and is now disproven in
// this same tree: uHGemClass.cpp:672 includes csystem.h and calls
// HasICUnderMachine() bare and UNGATED at :1639 and :1647, in THIS archive, and
// every test target that links ht9045_secsgem does so inside a
// $<LINK_GROUP:RESCAN, ht9045_sm, ht9045_secsgem, ...> that also lists ht9045_sm
// (tests/CMakeLists.txt:291/:324/:348/... -- all 18 such targets).  golden's own
// two-branch code is therefore ACTIVE and faithful below.
                            if(HasICUnderMachine()==false)                                                              //kevin 20181108有開SITE 才恢復畫面關SITE -> 開SITE
                                fMain->ShowTestHeadComp(true);
                            else
                                fMain->ShowTestHeadComp(false);
                            break;
                    }
                }
                else
                {
                    return -1;
                }
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    if(b151920)
    {
        bRefreshFunction=true;                                                  //kevin 20181101
        fMain->SetTemp(true, atof(fMain->edWorkTemperBase->Text.c_str()), atof(fMain->edSoakTime->Text.c_str()));
    }

// GATE [E7] -- golden :1080-1084.  `fSpeed` (golden cSpeed.h) has NO ported class or
// object.  `bspeed` and every case label that sets it stay ACTIVE.
#if 0
    if(bspeed)                                                                  //kevin 20181210 add speed
    {
        fSpeed->DoIniDataToForm();
        //fSpeed->SaveSetupFile(szDir);
    }
#else
    // E7 DEFAULT: the ~60 motion ECIDs are stored and the recipe directory is
    // created, but the Speed form's in-memory copy is not reloaded.
    (void)bspeed;
#endif

    ReloadParameter();
// GATE [E8] -- golden :1087-1102.  ONE gate for the whole trailing if/else chain:
// arm 1 needs fTemp_Set + fBuilder (no ported class/object), arm 2 needs
// WriteESDDataFile (declared csystem.h:267, defined NOWHERE) + SendCommand_ESD
// (body Interface/InterfaceSYS.cpp -> ht9045_sm, unlinkable here), arm 3 needs three
// TfSetup methods the deliberately one-field facade does not have
// (forms/fSetup.h:69 + its :20-25 "growing the facade past measured need" refusal).
// ReloadParameter() on the line above is ACTIVE.
#if 0
    if(ECID>=4801 && ECID<=4832)                                                //kevin 20180914 (Steven) : add read ESD data
    {
        fTemp_Set->DoIniDataToForm(true);                                       //Steven 20110930 : 得在fSetup後面
        fBuilder->bSaveAllFillOrFile("Temperature",0);
    }
    else if(ECID>=37800 && ECID<=37887)                                         //kevin 20181206 //kevin 20180914 add read ESD data
    {
        WriteESDDataFile();
        SendCommand_ESD(ESD_Read_ESD_SettDATA);                                 //kevin 20180914
    }
    else if(ECID==3545)
    {
        fSetup->ScrollBar1Change(NULL);
        fSetup->DoIniDataToForm();
        fSetup->sbUpdateClick(NULL);
    }
#else
    // E8 DEFAULT: all three post-apply refresh cascades skipped.  NOTE golden bug
    // (e) in the banner -- `ECID` is read here even when the loop never ran.
    (void)ECID;
#endif
    return 0;
}
//------------------------------------------------------------------------------
// CheckAndExecuteAsyncRCMD()
// 說明：供機台進入 Idle（SystemStart==false 且 !HasICUnderMachine()）後呼叫。
//       若有先前以 HCACK=4 接受的非同步指令，則在此執行並透過 S6F11 通知主機完成。
// 呼叫時機：建議放在 Stop() / OneCycleFinish / Idle 狀態切換點。（Ifor 20260423）
//------------------------------------------------------------------------------
void HT9045Gem::CheckAndExecuteAsyncRCMD()
{
    if(!m_AsyncRCMD.bPending)
        return;
// [A1] RETIRED before hand-off -- NOT a gate.  See [E6]'s retirement note: the
// "HasICUnderMachine() is not linkable from this archive" claim it rested on was
// FALSE (uHGemClass.cpp:1639/:1647, same archive, ungated).  golden's full
// two-term guard is ACTIVE below, so the deferred RCMD is correctly refused while
// ICs remain in the machine -- which was the single most safety-relevant delta
// this group had drafted.
    if(SystemStart || HasICUnderMachine())
        return;

    m_AsyncRCMD.bPending = false;

    // --- PP_SELECT 非同步：等待機台 Idle 後切換工作檔 ---
    if(m_AsyncRCMD.sCommand.AnsiPos("PP_SELECT") == 1 ||
       m_AsyncRCMD.sCommand.AnsiPos("PP-SELECT") == 1)
    {
        AnsiString PPID = m_AsyncRCMD.sParam;
        bool bHasFile = false;
        for(int i = 0; i < fMain->cbSetupFileName->Items->Count; i++)
        {
            if(PPID != "" && PPID == fMain->cbSetupFileName->Items->Strings[i])
                bHasFile = true;
        }

        if(bHasFile)
        {
            fMain->cbSetupFileName->Text = PPID;
            fMain->cbSetupFileNameChange(fMain);
            // 以 S6F11（CEID=SwitchSetupFile）通知主機非同步指令已完成
// GATE [A2] -- golden :1137.  The ported `class THGem` has NO EventReport member.
// DEFAULT calls golden's OWN free forwarder of the same name
// (`void EventReport(unsigned Ceid){ HGem->EventReport(1, Ceid); }`, golden
// UsecegemMainFrom.cpp:191) -- BUT in this port that free function is a SIM COUNTER
// that sends nothing (SECSGEM/SecsEventReport.h:24-27), so no S6F11 reaches the host.
// This is a documented STAND-IN, not an equivalent.
#if 0
            HGemPtr->EventReport(1, SECS_EVENT.SwitchSetupFile);
#else
            EventReport(SECS_EVENT.SwitchSetupFile);                             // A2 DEFAULT: Sim counter, NOT a real S6F11
#endif
            RecordProcess("[S2F42 Async] PP_SELECT executed: " + PPID);
        }
    }
    // --- 其他非同步指令可在此繼續 else if 擴充 ---
}
//------------------------------------------------------------------------------
void HT9045Gem::S5F6_ListAlarmData()                                            //Steven 20150520 : 修正S5F6 ListAlarmData
{
    int SVlen, ret;
    unsigned char Type;
    AnsiString S;

    unsigned char ALCD=0x80;
    unsigned int  ALID;
    AnsiString    ALTX;

    if(ActiveWire->GetDataItemLenAndType(SVlen, Type)==1)
    {
        if(SVlen!=0)
        {
            if(Type==HType.UINT_8_TYPE || Type==HType.UINT_4_TYPE || Type==HType.UINT_2_TYPE ||
               Type==HType.INT_8_TYPE  || Type==HType.INT_4_TYPE  || Type==HType.INT_2_TYPE)                            //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
            {
                unsigned __int64 *uint8Ptr;
                unsigned *uint4Ptr;
                unsigned short *uint2Ptr;
                __int64  *int8Ptr;
                int      *int4Ptr;
                short    *int2Ptr;

                ret=ActiveWire->GetDataItemLenAndType(SVlen, Type);
                if(ret!=1)
                {
                    S9F7_IllegalData("S5,F5 Data Format error !!!");
                    return;
                }

                if(Type==HType.UINT_8_TYPE)                                     //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                {
                    uint8Ptr=new unsigned __int64 [SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.UINT_8_TYPE, uint8Ptr);
                }
                else if(Type==HType.UINT_4_TYPE)
                {
                    uint4Ptr=new unsigned [SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.UINT_4_TYPE, uint4Ptr);
                }
                else if(Type==HType.UINT_2_TYPE)
                {
                    uint2Ptr=new unsigned short [SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.UINT_2_TYPE, uint2Ptr);
                }
                else if(Type==HType.INT_8_TYPE)                                 //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                {
                    int8Ptr=new __int64[SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.INT_8_TYPE, int8Ptr);
                }
                else if(Type==HType.INT_4_TYPE)
                {
                    int4Ptr=new int[SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.INT_4_TYPE, int4Ptr);
                }
                else if(Type==HType.INT_2_TYPE)
                {
                    int2Ptr=new short[SVlen];
                    ret=ActiveWire->DataItemIn(SVlen, HType.INT_2_TYPE, int2Ptr);
                }

                if(ret==1)
                {
                    ActiveWire->InitLocalHead(5, 6, 0);
                    ActiveWire->DataItemOut(SVlen, HType.LIST_TYPE, NULL);
                    for(int i=0; i<SVlen; i++)
                    {
                        if(Type==HType.UINT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            S=(long long)uint8Ptr[i];                           // (long long) cast: vclcompat AnsiString has no exact `unsigned long long` operator= (identical precedent uHGemClass.cpp:2604-2610)
                        else if(Type==HType.UINT_4_TYPE)
                            S=uint4Ptr[i];
                        else if(Type==HType.UINT_2_TYPE)
                            S=uint2Ptr[i];
                        else if(Type==HType.INT_8_TYPE)                         //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                            S=int8Ptr[i];
                        else if(Type==HType.INT_4_TYPE)
                            S=int4Ptr[i];
                        else if(Type==HType.INT_2_TYPE)
                            S=int2Ptr[i];

                        int j=HGemPtr->GetAlarmIndex(S);

                        if(j>=0)
                        {
                            ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                            ALCD=atoi(HGemPtr->strGrdAlarm->Cells[7][j].c_str())+0x80;
                            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &ALCD);     // mode

                            ALID=(unsigned)strtoll(HGemPtr->strGrdAlarm->Cells[8][j].c_str(), NULL, 10);   // golden: _atoi64 (MinGW substitution); the (unsigned) makes golden's own 64->32 store explicit
                            ActiveWire->DataItemOut(1, HType.INT_8_TYPE, &ALID);      // ID -- GOLDEN BUG (a): 8-byte item from a 4-byte object, KEPT

                            ALTX=HGemPtr->strGrdAlarm->Cells[6][j];                // message
                            ActiveWire->DataItemOut(HType.ASCII_TYPE, ALTX);
                        }
                        else
                        {
                            ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                            ALCD=0x80;
                            ActiveWire->DataItemOut(0, HType.BINARY_TYPE, &ALCD);     // mode -- GOLDEN BUG (b): zero length, ALCD never transmitted, KEPT

                            ALID=42;
                            ActiveWire->DataItemOut(1, HType.INT_8_TYPE, &ALID);      // ID -- GOLDEN BUG (a) again, KEPT

                            ALTX="Unknown Alarm Code";                          // message
                            ActiveWire->DataItemOut(HType.ASCII_TYPE, ALTX);
                        }
                    }
                    ActiveWire->SendLocalData();
                    if(Type==HType.UINT_8_TYPE)                                 //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] uint8Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint8Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_4_TYPE)
                    {
                        delete[] uint4Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint4Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_2_TYPE)
                    {
                        delete[] uint2Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint2Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.INT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] int8Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int8Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_4_TYPE)
                    {
                        delete[] int4Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int4Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_2_TYPE)
                    {
                        delete[] int2Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int2Ptr=NULL;                                           //kevin 20180517
                    }

                    return;
                }
                else
                {
                    S9F7_IllegalData("S5,F5 Data Format error !!!");
                    if(Type==HType.UINT_8_TYPE)                                 //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] uint8Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint8Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_4_TYPE)
                    {
                        delete[] uint4Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint4Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.UINT_2_TYPE)
                    {
                        delete[] uint2Ptr;                                      //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        uint2Ptr=NULL;                                          //kevin 20180517
                    }
                    else if(Type==HType.INT_8_TYPE)                             //Steven 20140911 : 修正INT_8_TYPE & UINT_8_TYPE
                    {
                        delete[] int8Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int8Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_4_TYPE)
                    {
                        delete[] int4Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int4Ptr=NULL;                                           //kevin 20180517
                    }
                    else if(Type==HType.INT_2_TYPE)
                    {
                        delete[] int2Ptr;                                       //Ifor 20170603 (wei) 修改陣列刪除方式 delete ==> delete[]
                        int2Ptr=NULL;                                           //kevin 20180517
                    }
                    return;
                }
            }
            else
            {
                S9F7_IllegalData("S5,F5 Data Format error !!!");
            }
        }
        else
        {
            ActiveWire->InitLocalHead(5, 6, 0);
            ActiveWire->DataItemOut(HGemPtr->strGrdAlarm->RowCount-1, HType.LIST_TYPE, NULL);
            for(int i=1; i<HGemPtr->strGrdAlarm->RowCount; i++)
            {
                ActiveWire->DataItemOut(3, HType.LIST_TYPE, NULL);
                ALCD=atoi(HGemPtr->strGrdAlarm->Cells[7][i].c_str())+0x80;
                ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &ALCD);                 // mode

                ALID=atoi(HGemPtr->strGrdAlarm->Cells[8][i].c_str());
                ActiveWire->DataItemOut(1, HType.UINT_4_TYPE, &ALID);                 // ID

                ALTX=HGemPtr->strGrdAlarm->Cells[6][i];                            // message
                ActiveWire->DataItemOut(HType.ASCII_TYPE, ALTX);
            }
        }
    }

    ActiveWire->SendLocalData();                                                // GOLDEN BUG (d): also reached when the FIRST read failed, KEPT
    return;
}
//---------------------------------------------------------------------------
// golden :6040 `extern int Process_2DID_BinCode(AnsiString s2DID_Source);`
// (//Steven 20231111 : 調整XML讀取) is DELIBERATELY NOT re-declared here: the
// function has no definition anywhere in this ported tree, and an unused extern
// declaration that nothing can resolve is exactly the "taking the address of an
// undefined function" hazard this campaign has already paid for.  Its one call
// site is gate [D2] below, where golden's own text keeps the call.
//------------------------------------------------------------------------------
void HT9045Gem::S14F4_Get2DID_BinCode()                                         //JerryYang 20230322 : Get 2DID bin map
{
    unsigned char HCACK=1;
    char *CommandStr;
    AnsiString PPID, S;
    int ret, len;
    unsigned char Type;

    CommandStr=new char [10000000];                                             //JerryYang 20221020 test

// [D1] RETIRED before hand-off -- NOT a gate.  See [E6]'s retirement note (the
// link-reachability claim was FALSE).  golden's full two-term guard is ACTIVE, and
// the sibling S7-program group in this same file relies on the identical guard
// ungated at S7F24_FormattedProcessProgramSendAcknowledge.
    if(HasICUnderMachine()==true || SystemStart==true)
    {
        HCACK=4;
    }
    else
    {
        if(ActiveWire->DataItemIn(1, HType.LIST_TYPE, NULL)==1)
        {
            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
            {
                ActiveWire->GetDataItemLenAndType(len, Type);
                if(Type==HType.ASCII_TYPE)
                {
                    ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                    if(ret==-1)
                        S="";
                    else
                        S=CommandStr;
                }
                else
                {
                    S="";
                }

                if(S!="")
                {
                    if(ActiveWire->DataItemIn(1, HType.LIST_TYPE, NULL)==1)
                    {
                        if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                        {
                            ActiveWire->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                S=CommandStr;
                                if(ret==-1)
                                {
                                    S="";
                                }
                                else
                                {
                                    S=CommandStr;
                                    S=S.UpperCase();

                                    if(S=="SUBSTRATETYPE")                      //JerryYang 20200330
                                    {
                                        ActiveWire->GetDataItemLenAndType(len, Type);
                                        if(Type==HType.ASCII_TYPE)
                                        {
                                            ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                            if(ret==-1)
                                            {
                                                HCACK=3;
                                            }
                                            else
                                            {
// GATE [D2] -- golden :6108.  `Process_2DID_BinCode` is golden's own extern free
// function and has NO definition anywhere in this ported tree.  DEFAULT is HCACK=3,
// golden's own "at least one parameter is invalid" code, already used by the two
// sibling `else` arms of this very `if`.  Delta: a well-formed SUBSTRATETYPE request
// is NAK'd and no 2DID bin map is loaded.
#if 0
                                                HCACK=Process_2DID_BinCode(CommandStr);                                 //Steven 20231111 : 調整XML讀取
#else
                                                HCACK=3;                                                                // D2 DEFAULT: decoder unavailable
#endif
                                            }
                                        }
                                        else
                                        {
                                            HCACK=3;
                                        }
                                    }
                                    else
                                    {
                                        HCACK=3;
                                    }
                                }
                            }
                            else
                            {
                                HCACK=3;
                            }
                        }
                    }
                }
            }
        }
        delete[] CommandStr;                                                    //Steven 20160912 : Add delete for save memory
        CommandStr=NULL;
        ActiveWire->InitLocalHead(14, 4, 0);

        if(HCACK==0)                                                            // GOLDEN BUG (l): both arms identical, KEPT
        {
            ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
            ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        }
        else
        {
            ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
            ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
        }

        ActiveWire->SendLocalData();
        return;
    }
    return;                                                                     // GOLDEN BUG (m): the HCACK=4 path returns without delete[] CommandStr, KEPT
}
//------------------------------------------------------------------------------
//pig 2014.07.04 ASEM SECS GEM start
void HT9045Gem::S110F5_RequestCustomerNameList()
{
    ActiveWire->InitLocalHead(110, 5, 1);
    ActiveWire->DataItemOut(1, HType.LIST_TYPE, NULL);
    ActiveWire->DataItemOut(HType.ASCII_TYPE, HGemPtr->GemMDLN);
    ActiveWire->SendLocalData();
}
//------------------------------------------------------------------------------
//L,n
//    1. L,2
//        1. <LSID1>
//        2. <LEVEL1>
//    2. L,2
//        1. <LSID2>
//        2. <LEVEL2>
//    .
//    .
//    n. L,2
//        1. <LSIDn>
//        2. <LEVELn>
//------------------------------------------------------------------------------
void HT9045Gem::S125F4_LevelSettingChangeAcknowledge()                          //Steven 20150605 : S125F3 LevelSettingChangeRequest
{
    int SVLen, len;
    unsigned char HCACK=0;
    unsigned char Type;
    int LSID, LEVEL;

    if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVLen, Type)==1)               //wei 20150630
    {
        if(SVLen!=0 && Type==HType.LIST_TYPE)
        {
            for(int i=0; i<SVLen; i++)
            {
                if( ActiveWire->DataItemIn( 2,HType.LIST_TYPE,NULL)==1)
                {
                    ActiveWire->GetDataItemLenAndType(len,Type);                 //取得資料長度與格式
                    ActiveWire->DataItemIn( len,Type,&LSID);                     //取值 -- GOLDEN BUG (k): int* for any host Type, KEPT
                    ActiveWire->GetDataItemLenAndType(len,Type);                 //取得資料長度與格式
                    ActiveWire->DataItemIn( len,Type,&LEVEL);                    //取值
                    SetECValue((unsigned)LSID, &LEVEL);
                }
                else
                {
                    HCACK=1;
                }
            }
        }
        else
        {
            HCACK=1;
        }
        ActiveWire->LocalAcknowledge(125, 4, HCACK);                             // GOLDEN BUG (j): inside the read-ok block only, KEPT
    }

// GATE [L1] -- golden :6210-6213.  `fSecurity` (golden cSecurity.h) has NO ported
// class or object, and `SetLevelSet` has zero hits of any kind in this tree.  The
// S9F7_IllegalData arm is a real inherited virtual (uHGemClass.h:290) and stays ACTIVE.
#if 0
    if(HCACK==1)
        S9F7_IllegalData("S125,F3 Data Format error !!!");
    else
        fSecurity->SetLevelSet();
#else
    if(HCACK==1)
        S9F7_IllegalData("S125,F3 Data Format error !!!");
    // L1 DEFAULT: levels are stored and the acknowledge is sent, but the Security
    // form never re-applies them (see GATE REGISTER).
#endif
}
//------------------------------------------------------------------------------
//  END of the PT-W5 "S2F15-S5-S14" append block.
//------------------------------------------------------------------------------

//==============================================================================
//  APPEND BLOCK -- ROLE: HT9045Gem's S2F42 host-command dispatcher
//
//  Translation wave: PT-W5-S2F42 (group S2F42)
//  Translator: AI(W906-PT-W5-S2F42) 20260809
//  Golden source: HT9011UC_Code_V3.33.906.0_20260618/SECSGEM/uHGemHT9045.cpp
//                 (read with python encoding='cp950'; every Chinese comment
//                 below is TRANSCRIBED from that decode, never re-authored)
//
//  WAVE SCOPE -- one line per golden function
//    ACTIVE (gated per ARM / per statement, see GATE REGISTER):
//      HT9045Gem::S2F42_Host_Command_Acknowledge   golden :1146-4189 (3044 lines)
//    Also carried over: golden's two file-scope forward declarations that sit
//    immediately in front of that body (golden :1144-1145) -- see FORWARD
//    DECLARATIONS below.
//
//  WHAT THIS FUNCTION IS
//    golden's S2F41 Host-Command dispatcher: one `if / else if` ladder over
//    ~60 remote-command names, each arm poking machine state and each arm
//    setting HCACK, then ONE common tail that replies S2F42 <L,2 <B HCACK>
//    <L,0>>.  The ladder's own final `else` sets HCACK=1 ("command does not
//    exist").  THAT FALL-THROUGH IS THE PORT'S GATING MECHANISM for whole
//    arms: a gated arm is simply not in the ladder, so its command reaches
//    golden's own final else and the host is told HCACK=1 -- the truth (the
//    port does not implement it) rather than a false HCACK=0.
//
//  PORT-MECHANICAL RENAMES (not gates -- no behaviour change)
//    1. golden `HGemPtr->` / `HGem->` for the WIRE-CODEC primitives
//       (DataItemIn / DataItemInNew / GetDataItemLenAndType /
//       GetDataItemLenAndTypeAndDelete / InitLocalHead / DataItemOut /
//       SendLocalData / StringOut) became `ActiveWire->`.  This is this
//       file-family's OWN established rule, not a new invention: those eight
//       live on SecsWireCodec (SECSGEM/SecsWireCodec.h:338-404) in this port,
//       NOT on THGem, and uHGemClass.cpp's INTEGRATE-WAVE 5/6 notes (that
//       file's head, "unified to `ActiveWire->`") applied exactly this rule to
//       golden's identical `HGemPtr->`/`HGem->` duality.  It also removes a
//       NULL deref this port would otherwise have: `THGem *HGem = NULL;`
//       (SECSGEM/uHGemEquipment.cpp:3521), whereas ActiveWire always points at
//       a real codec (HTGem's three ctors all do `ActiveWire=&WireCodec;`,
//       uHGemClass.cpp:858/867/875).  Golden text is preserved VERBATIM inside
//       every `#if 0` arm, so those still read `HGemPtr->`/`HGem->`.
//    2. golden :2549 `fMain->cbSetupFileName->Items->Text.AnsiPos(PPID)` ->
//       `AnsiString(fMain->cbSetupFileName->Items->Text).AnsiPos(PPID)`.
//       In this port `TStringList::Text` is a proxy object
//       (vclcompat/TStringList.h:112-120 TextProxy) with an
//       `operator AnsiString()` but no AnsiPos member, so the explicit
//       conversion is required to compile.  Same value, same comparison.
//    3. golden :3520 `if(S3==0 || S3==1 || S3==2 || S3==3)` ->
//       `if(S3==AnsiString(0) || ... )`.  THIS PRESERVES GOLDEN'S BEHAVIOUR,
//       it does not change it: in BCB6 `AnsiString==0` picks the
//       `AnsiString(int)` converting ctor (exact match) and therefore compares
//       against "0"/"1"/"2"/"3".  In this port `operator==(const AnsiString&,
//       const char*)` (vclcompat/AnsiString.h:216) is a BETTER match for the
//       literal 0 (standard pointer conversion beats a user-defined
//       conversion), so the literal golden text would silently compare against
//       "" four times instead.  Writing AnsiString(0) keeps BCB6 semantics.
//       (golden's own construct is a latent oddity -- the very next lines
//       re-test with S3.Pos("0") etc. -- KEPT, not fixed.)
//
//  GOLDEN BUGS KEPT (not fixed, per the translation contract)
//    B1. golden :1497 and :1565 `if(iLoadPort>=1 || iLoadPort<=15)` -- `||`
//        where `&&` was meant, so the range test is ALWAYS true and
//        bAskStopPort[iLoadPort] / iPortStatus[iLoadPort] can be indexed out
//        of range (both arrays are [ePortTotal]).  KEPT verbatim, ACTIVE.
//    B2. golden :2853 declares `unsigned char uint1EC;` uninitialised and
//        :2923 reads it on the path where no "Action" CPNAME arrived.  KEPT.
//    B3. golden :2036 and golden :2522 are the SAME arm,
//        `S.AnsiPos("REMOTE_SAVE")==1`.  The second is unreachable dead code.
//        KEPT verbatim (including its odd leading indentation at :2522).
//    B4. golden reuses SECS_GEM_PPSIGNALTOWER_CONTROL_flag as a scratch
//        "SVlen==0" marker in THREE unrelated arms (AUTHORITY_CHECK :2875,
//        START_LOT :3605, START_AGV :3760) that have nothing to do with the
//        signal tower -- copy-paste residue.  KEPT (all three gated, G25/G36/
//        G40, because the global has no definition in this port).
//    B5. golden's own misspellings are kept everywhere.
//
//  FORWARD DECLARATIONS (golden :1144-1145)
//    Both are carried over verbatim.  NEITHER has a definition anywhere in
//    this port (see GATE G29 / GATE G35 for the call sites, which ARE gated),
//    so they are declaration-only: legal C++, zero link demand.  Do not add a
//    call to either without also providing a body.
//
//  INCLUDE CONFLICT -- RESOLVED 20260824 (FW-G24) by narrowing the seam:
//  TMyMessageBoxShim extracted to mymessbox_shim.h; this TU includes only
//  that. The clYellow/NewRecordProcess duplications in acatchtray_shims.h
//  itself remain (probe-verified still real), just no longer in our path.
//  G24/G26/G45 are OPENED. Original note kept below for the record.
//  INCLUDE CONFLICT THIS BLOCK COULD NOT WORK AROUND (integration TODO)
//    `MyMessageBox` (golden mymessbox.h) exists in this port ONLY as
//    `TMyMessageBoxShim` in acatchtray_shims.h:319-327, and that header CANNOT
//    be included by this TU: it re-defines `const TColor clYellow`
//    (acatchtray_shims.h:128) which SECSGEM/uHGemEquipment.h:396 -- included
//    at the very top of this file, line 30, not by this block -- already
//    defines, and it re-states NewRecordProcess's default arguments that
//    cMyDB.h:121 already gave.  Both are hard errors in the same TU.  So the
//    three MyMessageBox uses are gated (G24 / G26 / G45) even though a real
//    stand-in object EXISTS and is non-NULL (acatchtray_shims.cpp:98 news it
//    at static-init).  Un-gating them needs the clYellow / NewRecordProcess
//    duplication in acatchtray_shims.h reconciled -- another file, i.e. the
//    main loop's serialized job, not this group's.
//
//  LINK-CLOSURE / STAND-IN DISCLOSURE (trap 3 -- "it links" proves nothing)
//    Every external symbol the ACTIVE code below calls has a body in a
//    CMakeLists-registered .cpp.  Four of those bodies are STAND-INS, not
//    golden's real behaviour, and that materially changes what the ACTIVE arms
//    do on a real machine:
//      * SetRunStartMode(int)   -> aHotPlateSubstrate.cpp:1063 `{}` -- a NO-OP.
//        Nine arms (CONTINUE_RETEST_ART / CONTINUE_START_ART / RETEST_MRT /
//        INITIAL_START_MRT / CONTINUE_START_MRT / INITIAL_START / AUTOSITEMAP /
//        AUTO_RETEST / INITIAL_START_ART) reply HCACK=0 and set NO run mode.
//      * RecordProcess(...)     -> canary_support.cpp:103 (printf to stdout).
//        golden's DB-writing body is gated at cMyDB.cpp:1828 precisely because
//        the canary stand-in already satisfies the symbol -- so nothing warns.
//      * NewRecordProcess(...)  -> acatchtray_shims.cpp:132 `{}` (no-op).
//      * HasICUnderMachine()    -> csystem_predicates.cpp:204, a PARTIAL
//        aggregate (ShuttleHasIC||IndexHasIC||HasICUnderHotPlate); golden's
//        full Loader/HotPlate/Auto-tray walk is gated inside csystem.cpp.  So
//        every "machine still has material?" guard below is optimistic.
//      * All fMain->/fLotInfo->/fSCKART->/fSortCT-> methods are the offline
//        facade bodies in forms/*.cpp (each documented "offline: no-op" at its
//        own declaration).  fMain->Start/Reset/CleanOut/Home/BtnOneCycleClick/
//        BtnTrayEndClick/BtnPauseClick/cbSetupFileNameChange/ChangePassword do
//        NOT move the machine in this port.
//    NULL-POINTER EXPOSURE (runtime, not link): `slBundlID`/`slDupBundlID`/
//    `slDupUnloadBundlID` are `TStringList*` defined NULL (cmydef.cpp:122-124)
//    and golden news them in main.cpp, which is unported -- so the TRAYCHKOK /
//    SET_BUNDLE_INFO / CLEAR_LOT_INFO arms will fault if reached before
//    something allocates them.  Same for `fNote`/`fLotInfo`/`fSCKART`/`fMain`
//    themselves.  This is NOT a static-init hazard (trap 4): this body only
//    ever runs from a received S2F41, long after main(); HT9045Gem itself is
//    heap-built from SYSTEM_MODULAR::SystemModularInitial (database.cpp:187).
//
//  TWO-HEADERS TRAP (trap 5): `InArmSuck`/`OutArmSuck` are declared BOTH in
//    aHotPlateSubstrate.h:624/627 and in mykitsuck.h:452/460, over two
//    DIFFERENT `class TMyKitSuck` layouts.  This block includes
//    aHotPlateSubstrate.h -- the one whose definitions actually link
//    (aHotPlateSubstrate.cpp:80/83); mykitsuck.cpp is DELIBERATELY NOT
//    REGISTERED (CMakeLists.txt:2099).  Same choice as the sibling
//    uHGemHT9045_EC.cpp block.
//
//  GATE REGISTER -- 48 gates.  Missing-in-port surfaces, listed once:
//    forms with NO port object at all : fContactCT, fFTPClient,
//        fTemp_Set, FormBarcodeReader   [fShowBinSelect REMOVED 20260916 -- real]
//    port object exists, member does NOT:
//        fLotInfo : sbSECSLotStartClick, ClearAllSetupFile, edCustomerDevice,
//                   btnASECL_LotStartClick, btnASECL_LotStart, cbTestTimes,
//                   SetAQLMode, InitialLoaderTask, CheckActionFlag
//        fNote    : ReturnCode, Close(), BtnPauseClick()
//        fAGV     : bATK_AMR_DoHostLotStart, bATKAMR_GET_LOTORDER0_Ready
//        fSCKART  : sInfo_Step, sInfo_CurrQty
//        fSortCT  : ShowLoadingIC_ART   (only ShowLoadingIC exists)
//        fObserver: labBundleID, labBundlIn, labBundOut  (TfObserverShim,
//                   atester_shims.h:340-402)
//        fProductionInfo : CalculateNowArmSiteBinQty, UpdateControlBinCount
//        fYieldMonitoring: everything (the facade is one method,
//                   aHotPlateSubstrate.h:1125 DoAutoCloseSite only)
//        fConfiguration  : cbN07_EnableHostStart (W5SckArtRem_ConfigStub,
//                   Automation/SCK_ART_Remainder.h:594-599, has mmoN04_IP only)
//        THGem    : EventReport(unsigned,unsigned) (golden
//                   uHGemEquipment.h:552) -- absent from the port's THGem
//        THGemSpeedButton : Click() (uHGemEquipment.h:486-490 has Enabled only)
//    reachable object, header now reachable 20260824: MyMessageBox (mymessbox_shim.h)
//    declared-but-DEFINED-NOWHERE globals: SECS_GEM_PPMUSIC_CONTROL_flag,
//        iSECS_GEM_PPMUSIC_CONTROL_CLASS, SECS_GEM_PPSIGNALTOWER_CONTROL_flag,
//        iSECS_GEM_PPSIGNALTOWER_CONTROL_{RED,GREEN,YELLOW} (extern'd only at
//        ckernel.cpp:1272-1278; the sibling uHGemHT9045_SV.cpp:459 gate [G3]
//        found the same hole and gated it too)
//    functions with NO body in the tree: ProcessLotInfo (golden BarcodeXML.cpp,
//        untranslated -- there is no BarcodeXML.cpp in the port at all),
//        S2F42_SET_2DID_BIN_CODE (no match anywhere)
//
//    id   golden lines   gated                                    port default
//    ---  -------------  ---------------------------------------  ---------------------------------
//    G01  1194-1394      WHOLE ARM  LOT_START (CC_AMKOR_Korea)    falls to final else -> HCACK=1
//    G02  1682           TRAYCHKOK   fObserver->labBundleID       label not painted
//    G03  1727           TRAYCHKOK   fSCKART->sInfo_Step          step string not stored
//    G04  1750           TRAYCHKOK   fSCKART->sInfo_CurrQty       qty string not stored
//    G05  1878           CLEAN_*_COUNT fShowBinSelect refresh     OPENED 20260916 (A1)
//    G06  1900-1932      CLEAN_*_COUNT whole CC_JCET block        JCET yield/CT clear does not run
//    G07  1951-1954      CLEAN_*_COUNT fShowBinSelect skip grid   OPENED 20260916
//    G08  1955           CLEAN_*_COUNT fSortCT->ShowLoadingIC_ART ART load panel not repainted
//    G09  1957           CLEAN_*_COUNT fShowBinSelect refresh     OPENED 20260916
//    G10  2117-2121      RETRY       fNote dialog dismissal       HCACK=0, dialog left open
//    G11  2126-2130      TRAY END    fNote dialog dismissal       HCACK=0, dialog left open
//    G12  2151-2215      WHOLE ARM  DOWNLOAD_RECIPE_BY_FTP        falls to final else -> HCACK=1
//    G13  2222-2223      CLEAN_OUT   fNote dialog dismissal       HCACK=0, dialog left open
//    G14  2292-2296      WHOLE ARM  ONLINE_LOCAL                  falls to final else -> HCACK=1
//    G15  2297-2301      WHOLE ARM  ONLINE_REMOTE                 falls to final else -> HCACK=1
//    G16  2407-2435      WHOLE ARM  PP_MUSIC                      falls to final else -> HCACK=1
//    G17  2436-2479      WHOLE ARM  PP_SIGNALTOWER                falls to final else -> HCACK=1
//    G18  2480-2495      WHOLE ARM  AUTO_CLEAN                    RULED CLOSED 20260916
//    G19  2564-2565      PP_SELECT   fLotInfo->ClearAllSetupFile   old setup files kept
//    G20  2831           PP_SELECT   fConfiguration checkbox       ini written, UI box not ticked
//    G21  2845-2850      WHOLE ARM  LOTSTART                      falls to final else -> HCACK=1
//    G22  2856-2859      AUTHORITY_CHECK FormBarcodeReader close   barcode form left open
//    G23  2861-2864      AUTHORITY_CHECK fPassword close           OPENED 20260824 (FW-QWKEY4)
//    G24  2866-2869      AUTHORITY_CHECK MyMessageBox close        OPENED 20260824 (FW-G24)
//    G25  2875           AUTHORITY_CHECK PPSIGNALTOWER flag clear  flag not cleared (undefined sym)
//    G26  2921           AUTHORITY_CHECK MyMessageBox->fShow test  OPENED 20260824 (FW-G24)
//    G27  3029           SET_TEST_FLOW fLotInfo->edCustomerDevice  device not stored
//    G28  3031-3032      SET_TEST_FLOW ASE-CL lot-start button     HCACK=0 but NO lot start
//    G29  3110           SET_LOT_INFO  ProcessLotInfo()            bHasFile=false -> HCACK=3
//    G30  3198           SET_BUNDLE_INFO fObserver->labBundleID    label not painted
//    G31  3223           SET_BUNDLE_INFO fObserver->labBundlIn     label not painted
//    G32  3252           SET_BUNDLE_INFO fObserver->labBundOut     label not painted
//    G33  3285           SET_BUNDLE_INFO fObserver->labBundOut     label not painted
//    G34  3422-3431      DEVTEMPOFFSETADJUST fTemp_Set save/read   HCACK=1;break (golden's own fail)
//    G35  3588           SET_2DID_BIN_CODE S2F42_SET_2DID_BIN_CODE HCACK=3 (golden's own bad-param)
//    G36  3605           START_LOT   PPSIGNALTOWER flag clear      flag not cleared (undefined sym)
//    G37  3657-3671      START_LOT   TEST_TIMES CPNAME branch      falls to arm's else -> HCACK=1
//    G38  3688-3689      START_LOT   fLotInfo->sbSECSLotStartClick HCACK=0 but NO lot start
//    G39  3732-3739      START_AQL   fLotInfo->SetAQLMode          HCACK=1 (golden's own fail arm)
//    G40  3760           START_AGV   PPSIGNALTOWER flag clear      flag not cleared (undefined sym)
//    G41  3778           START_AGV   InitialLoaderTask(0)          Loader action flag set, no task
//    G42  3787           START_AGV   InitialLoaderTask(1)          Empty  action flag set, no task
//    G43  3792           START_AGV   InitialLoaderTask(2)          Color  action flag set, no task
//    G44  3877-3881      SKIP        fNote dialog dismissal        HCACK=0, dialog left open
//    G45  3886-3889      TERMINAL_DISPLAY MyMessageBox close       OPENED 20260824 (FW-G24)
//    G46  3990           SET_RECIPE  HGemPtr->EventReport CEID 15  recipe switched, host NOT notified
//    G47  4049           START       fLotInfo->CheckActionFlag     AMR start flag set, no dispatch
//    G48  4112-4113      PAUSE/STOP  fNote pause+close (MAXIM)     fMain->BtnPauseClick still runs
//
//  ARMS THAT ARE FULLY LIVE (no gate at all): CANCEL_INPUT_TRAY, TRY_RFID_READ,
//    LOT_END, LOT_PRE_END, DISCHARGE_OUTPUT_PORT, DISCHARGE_OUTPUT_ALL_PORT,
//    ALARM_NOTIFY, STOP_LOAD_PORT, RESTART_LOAD_PORT, ONE_CYCLE, RESET,
//    TRAYCHKNG, BINDCHKNG, UNBINDCHKNG, CLEAR_SOCKET_COUNT,
//    CONTINUE_RETEST_ART, CONTINUE_START_ART, RETEST_MRT, INITIAL_START_MRT,
//    CONTINUE_START_MRT, INITIAL_START, REMOTE_SAVE, AUTOSITEMAP, AUTO_RETEST,
//    TRAY_FEED, INITIAL_START_ART, SWITCH_TO_FT, SWITCH_TO_RT, RESUME,
//    REMOTE_START, HOME, HALT, PP_PASSWORD, CLEAR_LOT_INFO, LOTORDER,
//    TRAY_MAP, STOP_LOT, RECIPEVERIFYACK/RUNCHECKEND, SET_RECIPE (except its
//    event report, G46), and the final else.  "Live" means the arm's own code
//    runs; see the STAND-IN DISCLOSURE above for what that code actually does
//    offline.
//
//  SEVEN ARMS ARE OFF THE LADDER ENTIRELY (whole-arm gates): LOT_START(AMKOR),
//    DOWNLOAD_RECIPE_BY_FTP, ONLINE_LOCAL, ONLINE_REMOTE, PP_MUSIC,
//    PP_SIGNALTOWER, AUTO_CLEAN, LOTSTART -- eight, counting LOTSTART, which is
//    a different command from LOT_START.  Each now answers HCACK=1.
//==============================================================================

// AI(W906-PT-W5-S2F42) 20260809: includes this block needs.  Several are
// already pulled in above by sibling append blocks; a repeat include is a
// no-op (header guards) and is spelled out here so this block is readable and
// movable on its own.
#include "MachineDefine.h"                  // golden :1 (verbatim); golden's `#pragma hdrstop` dropped
#include "MachineType.h"                    // golden :6  -- CC_* customer codes, rsm* run-start modes,
                                            //               ePort*/eLoad*/eReadyToUnload, eTrayCount, TEST_MAX_BIN
#include "cprod.h"                          // golden :11 -- RunInfo / TestIF / TrayForm / Prod
#include "csystem.h"                        // golden :15 -- HasICUnderMachine / ShuttleHasIC / IndexHasIC
#include "cmydef.h"                         // golden :24 -- the int/bool/AnsiString global block, K_* return keys,
                                            //               MOT index consts, iPortStatus/bPortIsBusy/bAskStopPort
#include "cSocket.h"                        // golden :26 -- ArmData[] (TArm::ClearALLCT)
#include "cMyDB.h"                          // golden :22 -- RecordProcess / NewRecordProcess declarations
#include "Config.h"                         // PORT equivalent of golden :37 "cConfiguration.h" -- IniConfig
#include "CosFunction.h"                    // PORT: CosFunction (golden reaches it via the cprod.h/cmydef.h chain)
#include "LastSet.h"                        // PORT: LastSet (golden reaches it transitively via main.h/cmydef.h)
#include "common.h"                         // PORT: AuthPath / ReadWriteIni / asDupBundleID / aslDupUnloadBundlID
#include "Motor/mymotor.h"                  // PORT: MOT[] + TTrayMotor::HasIC() (golden gets these via cmydef.h)
#include "aHotPlateSubstrate.h"             // PORT: InArmSuck / OutArmSuck (the 177-TU TMyKitSuck -- see TWO-HEADERS
                                            //       TRAP above) + SetRunStartMode
#include "AutoRetest.h"                     // PORT: DoAutoRetest (golden reaches it via csystem.h/main.h)
#include "FormsFacade.h"                    // PORT equivalent of golden :8 "main.h" (fMain), :17 "uLotInfo.h"
#include "forms/fPassword.h"            // AI(W906-FW-QWKEY4) 20260824: fPassword extern (real since FW-QWKEY1 fc08e09) -- GATE G23 opened, Visible/Close() on the facade
#include "mymessbox_shim.h"             // AI(W906-FW-G24) 20260824: MyMessageBox via the narrow seam -- G24/G26/G45 opened (full acatchtray_shims.h still conflicts, see its note)
#include "forms/fShowBinSelect.h"   // AI(W906-G05) 20260916: fShowBinSelect extern (:1133) -- GATE G05 opened. [this line used to be FormsFacade.h's continuation: ":17 uLotInfo.h (fLotInfo), :45 SCK_ART.h (fSCKART), cSortCT.h (fSortCT)"]
#include "SECSGEM/SecsEventType.h"          // PORT: SECS_EVENT (golden declares it in uHGemHT9045.h itself)
#include "SECSGEM/uHGemEquipment.h"         // THGem complete type (HGemPtr->StringOut, ->WireCodec via ActiveWire)
// NOT INCLUDED ON PURPOSE: "acatchtray_shims.h" (MyMessageBox) -- see the
// INCLUDE CONFLICT note in the banner above; it cannot coexist with
// SECSGEM/uHGemEquipment.h + cMyDB.h in one TU.

//------------------------------------------------------------------------------
extern bool ProcessLotInfo(AnsiString S2);
extern int S2F42_SET_2DID_BIN_CODE(AnsiString s2DID_Source);
int HT9045Gem::S2F42_Host_Command_Acknowledge()
{
    unsigned char HCACK=1;
//    char CommandStr[1024];
    char *CommandStr;
//    CommandStr=new char [10000000];                                           //JerryYang 20241205 : 字串陣列大小改成動態

    char str[256];
    AnsiString PPID, sLotNo, sLOTCount, str2="";
    int ret, SVlen, i, j, len, iHead, iArm, iLoadPort=0;
    double dOffset=0.0;
    bool bHasFile;
    unsigned char Type;
    AnsiString S, S1, S2, S3, sLotInfo, strLotID, sLotCnt, str1, sDCC, sOP, sTrayID, sYield, sHBin, sTrayQty;
//    ZeroMemory(CommandStr, sizeof(CommandStr));
    ZeroMemory(str, sizeof(str));

    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)                        // 需要補充多重 Command
    {
        ActiveWire->GetDataItemLenAndType(len, Type);
        CommandStr=new char [len+1];                                            //JerryYang 20241205 : 字串陣列大小改成動態
//        ZeroMemory(CommandStr, sizeof(CommandStr));
        memset(CommandStr, 0, len+1);                                           //JerryYang 20250325 : fix

        if(Type==HType.ASCII_TYPE)
        {
            ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
            if(ret==-1)
                S="";
        }
        else
        {
            S="";
        }
        S=CommandStr;
        S=S.UpperCase();

        delete[] CommandStr;                                                    //Steven 20160912 : Add delete for save memory
        CommandStr=NULL;

        if(S.AnsiPos("CANCEL_INPUT_TRAY")==1)                                   //Command to discharge input tray
        {
            HCACK=0;
        }
        else if(S.AnsiPos("TRY_RFID_READ")==1)                                  //Command to retry reading input cover tray ID
        {
            HCACK=0;                                                        //Steven 20260415 : stub, acknowledge
        }
#if 0   // ===== GATE G01 -- golden SECSGEM/uHGemHT9045.cpp:1194-1394 =====
//   WHY GATED : golden calls fLotInfo->sbSECSLotStartClick(fLotInfo) (:1373) and
//               fAGV->bATK_AMR_DoHostLotStart / fAGV->bATKAMR_GET_LOTORDER0_Ready
//               (:1375/:1376).  None of the three exists: forms/fLotInfo.h has no
//               sbSECSLotStartClick, forms/fAGV.h (whole class, :29-40) has neither
//               flag.  Without them this arm cannot do the one thing it exists for.
//   DELTA     : CC_AMKOR_Korea LOT_START is no longer in the ladder, so the command
//               reaches golden's own final else (:4155) and the host is answered
//               HCACK=1 "command does not exist" instead of HCACK=0 + a lot start.
//               On a real AMKOR machine the host lot-start handshake FAILS CLEANLY
//               rather than silently acknowledging a start that never happened.
        else if(CUSTOMER_CODE==CC_AMKOR_Korea &&
                S.AnsiPos("LOT_START")==1)                                      //Command to process loaded tray
        {
//        <L:2
//            <A:9 LOT_START>
//            <L:8
//                <L:2
//                    <A:6 LOT_NO>
//                    <A:38 AZ1FC5337LT-C5337M4.0101#SL3@k3tv93368>
//                >
//                <L:2
//                    <A:3 DCC>
//                    <A:0>
//                >
//                <L:2
//                    <A:14 OPERATION_CODE>
//                    <A:4 7582>
//                >
//                <L:2
//                    <A:8 UNIT_QTY>
//                    <A:4 5849>
//                >
//                <L:2
//                    <A:7 TRAY_ID>
//                    <A:9 RT0000658>
//                >
//                <L:2
//                    <A:9 LOT_YIELD>
//                    <A:4 99.5>
//                >
//                <L:2
//                    <A:13 HARD_BIN_INFO>
//                    <A:56 BIN01,BIN02,BIN03,BIN04,BIN05,BIN06,BIN22,CM,CV,OS,OTHER>
//                >
//                <L:2
//                    <A:15 OUTPUT_TRAY_QTY>
//                    <A:2 32>
//                >
//            >
//        >

//            unsigned char  uint1EC;
            if(HGem->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)                                         //wei 20150630
            {
                HCACK=0;
                for(i=0; i<SVlen; i++)
                {
                    if(HGem->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                    {
                        HGem->GetDataItemLenAndType(len, Type);
                        HGem->DataItemIn(len, Type, str);
                        S1=str;
                        if(S1=="LOT_NO")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                strLotID=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="DCC")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sDCC=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="OPERATION_CODE")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sOP=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="UNIT_QTY")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sLotCnt=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="TRAY_ID")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sTrayID=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="LOT_YIELD")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sYield=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="HARD_BIN_INFO")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sHBin=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else if(S1=="OUTPUT_TRAY_QTY")
                        {
                            HGemPtr->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                CommandStr=new char [len+1];
                                memset(CommandStr, 0, len+1);
                                ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                sTrayQty=CommandStr;
                                delete[] CommandStr;
                                CommandStr=NULL;
                            }
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                    else
                    {
                        HCACK=2;
                    }
                }

                if(HCACK==0)
                {
//                    fLotInfo->edtSysLotID->Text     =strLotID;                  //RogerYang 20260401 : add
//                    fLotInfo->edtDevice->Text       =sDCC;
//                    sDCC_ATK                        =sDCC;
                    fLotInfo->edtSysOperatorID->Text=sOP;                       //RogerYang 20260401 : add
                    fLotInfo->cbRunMode->Text       =strLotID;                  //RogerYang 20260401 : add
//                    fLotInfo->lbledtTestBinNo->Text =sHBin;
//                    fSCKART->iLotCount              =atoi(sLotCnt.c_str());

//                    int iLotCnt=atoi(sLotCnt.c_str());
//                    fSCKART->DoARTLotStart(strLotID, "", iLotCnt);
                    fLotInfo->sbSECSLotStartClick(fLotInfo);
                    RecordProcess("SECS/GEM LOTSTART!");
                    fAGV->bATK_AMR_DoHostLotStart=true;                         //RogerYang 20260401 : add
                    if(fAGV->bATKAMR_GET_LOTORDER0_Ready==true)
                    {
                        LastSet.bWaitStartLotAutoRetestGPIB=true;
                        fAGV->bATKAMR_GET_LOTORDER0_Ready=false;
                    }

                    if(SystemStart==false)
                    {
                        fMain->Start("SECS GEM RCMD : START");
//                        HCACK=0;
                    }
                    else
                    {
//                        HCACK=3;                                              //RogerYang 20260402 : no return 3 when start
                    }
                    HCACK=0;
                }
            }
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G01
        else if(CUSTOMER_CODE==CC_AMKOR_Korea &&
                S.AnsiPos("LOT_END")==1)                                        //End signal command to discharge all trays after Lot processing
        {
            HCACK=0;                                                        //Steven 20260415 : stub, acknowledge
        }
        else if(S.AnsiPos("LOT_PRE_END")==1)                                    //Command to discharge trays when max tray quantity is reached during Lot processing
        {
//        <L:2
//            <A:11 LOT_PRE_END>
//            <L:1
//                <L:2
//                    <A:14 OUTPUT_PORT_NO>
//                    <A:1 3>
//                >
//            >
//        >
            HCACK=0;                                                        //Steven 20260415 : stub, acknowledge
        }
        else if(S.AnsiPos("DISCHARGE_OUTPUT_PORT")==1)                          //DISCHARGE_OUTPUT_PORT
        {
            HCACK=0;                                                        //Steven 20260415 : stub, acknowledge
        }
        else if(S.AnsiPos("DISCHARGE_OUTPUT_ALL_PORT")==1)                      //All discharge commands for the Output Port that must be discharged after Lot End (CEID 5000 occurs for each port)
        {
//      <L:2
//          <A:25 DISCHARGE_OUTPUT_ALL_PORT>
//          <L:0>
//      >
            HCACK=0;                                                        //Steven 20260415 : stub, acknowledge
        }
        else if(S.AnsiPos("ALARM_NOTIFY")==1)                                   //Command to display alarm from host on equipment
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
            {
                for(i=0; i<SVlen; i++)
                {
                    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                    {
                        ActiveWire->GetDataItemLenAndType(len, Type);
                        CommandStr=new char [len+1];
                        memset(CommandStr, 0, len+1);
                        ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                        if(ret==-1)
                            S2="";
                        S2=CommandStr;
                        delete[] CommandStr;
                        CommandStr=NULL;

                        ActiveWire->GetDataItemLenAndType(len, Type);
                        CommandStr=new char [len+1];
                        memset(CommandStr, 0, len+1);
                        ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                        if(ret==-1)
                            S3="";
                        S3=CommandStr;
                        delete[] CommandStr;
                        CommandStr=NULL;

                        if(S2=="HOST_ALARM_DESCRIPTION")
                        {
                            SecsAlarmMessage->Add(S3);
                            HCACK=0;
                        }
                    }
                }
            }
        }
        else if(S.AnsiPos("STOP_LOAD_PORT")==1)                                 //JerryYang 20250505 : AGV
        {
            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
            {
                ActiveWire->GetDataItemLenAndType(len, Type);
                CommandStr=new char [len+1];                                    //JerryYang 20241205 : 字串陣列大小改成動態
//                ZeroMemory(CommandStr, sizeof(CommandStr));
                memset(CommandStr, 0, len+1);                                   //JerryYang 20250325 : fix
                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                if(ret==-1)
                    S2="";
                S2=CommandStr;
                delete[] CommandStr;                                            //Steven 20160912 : Add delete for save memory
                CommandStr=NULL;

//                int iPort=0;

                if(S2=="PortID")
                {
                    ActiveWire->GetDataItemLenAndType(len, Type);
                    CommandStr=new char [len+1];                                //JerryYang 20241205 : 字串陣列大小改成動態
//                    ZeroMemory(CommandStr, sizeof(CommandStr));
                    memset(CommandStr, 0, len+1);                               //JerryYang 20250325 : fix
                    ActiveWire->GetDataItemLenAndType(len,Type);                      //取得資料長度與格式
                    if(Type==HType.INT_4_TYPE)
                    {
                        ret=ActiveWire->DataItemIn(len, HType.INT_4_TYPE, &iLoadPort);
                        if(ret==-1)
                            iLoadPort=0;
                        S3=CommandStr;

//                        iLoadPort=atoi(S3.c_str());

                        delete[] CommandStr;                                    //Steven 20160912 : Add delete for save memory
                        CommandStr=NULL;
                        if(iLoadPort>=1 || iLoadPort<=15)
                        {
                            if(bPortIsBusy[iLoadPort]==false && fNote->fShow==false)                                    //RogerYang 20250617 add do not ready if Alarm Shown
                            {
                                bAskStopPort[iLoadPort]=true;
                                if(iLoadPort==ePortLoader)
                                {
                                    iPortStatus[iLoadPort]=eLoadReadyToLoad;
                                }
                                else if(iLoadPort==ePortAuto1 || iLoadPort==ePortAuto2 ||iLoadPort==ePortAuto3)
                                {
                                    iPortStatus[iLoadPort]=eReadyToUnload;
                                }
                                HCACK=0;
                            }
                            else
                            {
                                HCACK=1;
                            }
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else
            {
                HCACK=1;
            }
        }
        else if(S.AnsiPos("RESTART_LOAD_PORT")==1)                              //JerryYang 20250505 : AGV
        {
            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
            {
                ActiveWire->GetDataItemLenAndType(len, Type);
                CommandStr=new char [len+1];                                    //JerryYang 20241205 : 字串陣列大小改成動態
//                ZeroMemory(CommandStr, sizeof(CommandStr));
                memset(CommandStr, 0, len+1);                                   //JerryYang 20250325 : fix
                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                if(ret==-1)
                    S2="";
                S2=CommandStr;
                delete[] CommandStr;                                            //Steven 20160912 : Add delete for save memory
                CommandStr=NULL;

                if(S2=="PortID")
                {
                    ActiveWire->GetDataItemLenAndType(len, Type);
                    CommandStr=new char [len+1];                                //JerryYang 20241205 : 字串陣列大小改成動態

                    memset(CommandStr, 0, len+1);                               //JerryYang 20250325 : fix

                    ActiveWire->GetDataItemLenAndType(len,Type);                      //取得資料長度與格式
                    if(Type==HType.INT_4_TYPE)
                    {
                        ret=ActiveWire->DataItemIn(len, HType.INT_4_TYPE, &iLoadPort);
                        if(ret==-1)
                            iLoadPort=0;
                        S3=CommandStr;

                        delete[] CommandStr;                                    //Steven 20160912 : Add delete for save memory
                        CommandStr=NULL;
                        if(iLoadPort>=1 || iLoadPort<=15)
                        {
                            bAskStopPort[iLoadPort]=false;
                            HCACK=0;
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else
            {
                HCACK=1;
            }
        }
//        else if(S.AnsiPos("PAUSE")==1 || S.AnsiPos("STOP")==1)                  //Steven 20221218 : S2F41字串判斷從原本的S=="" 改成 S.AnsiPos()==1
//        {
//            if(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND)
//            {
//                if(fNote->fShow)
//                {
//                    fNote->BtnPauseClick(fNote);
//                    fNote->Close();
//                }
//
//                fMain->BtnPauseClick(fMain);
//                HCACK=0;
//            }
//            else
//            {
//                fMain->BtnPauseClick(fMain);
//                if(IniConfig.bEnable_SECS_GEM==true &&
//                   IniConfig.bRCMDStart==true &&
//                   bPhysicalStart==true)                                        //Wei 20150815 : Fixed for  RCMD Start
//                {
//                    bPhysicalStart=false;
//                }
//
//                if(CUSTOMER_CODE==CC_ASE_CL)                                    //RogerYang 20260413 Add
//                {
//                    bSECSPause=true;                                            //JerryYang 20250120 : modify
//                }
//
//                if(CosFunction.RunCheckWhenRecPause )                           //JerryYang 20250120 : modify
//                {
//                    bNeedDoRunCheck=true;
//                }
//
//                if(SystemStart==false)
//                {
//                    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                      //KenHsieh 20220913 : ASEKH K1要求回傳2
//                        HCACK=2;
//                    else if(IniConfig.bVTESTFunction)                           //RogerYang 20260608 : Add for VTEST SECS
//                        HCACK=5;                                                //5 = Rejected, Already inDesired Condition
//                    else
//                        HCACK=1;
//                }
//                else
//                {
//                    HCACK=0;
//                }
//            }
//        }
        else if(S.AnsiPos("ONE_CYCLE")==1 || S.AnsiPos("ONE CYCLE")==1)         //20140124 wei
        {
            bSECSOneCycleComm=true;                                             //RogerYang 20170705 (wei) add SECS/GEN OneCycle命令旗標
            fMain->BtnOneCycleClick(fMain);
            RecordProcess("SECS/GEM One Cycle!");
            HCACK=0;
        }
        else if(S.AnsiPos("RESET")==1)                                          //Steven 20220714 : Add RCMD RESET
        {
            fMain->Reset("RESET by SECS GEM S2F42");
            HCACK=0;
        }
        else if(S.AnsiPos("TRAYCHKOK")==1)                                      //JerryYang 20250207 : add
        {
            if(HasICUnderMachine()==true || SystemStart==true)
            {
                HCACK=1;
            }
            else
            {
                if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
                {
                    if(SVlen==4)
                    {
                        fSCKART->ClearLotInfo();                                //JerryYang 20250731 : add
                        for(i=0; i<SVlen; i++)
                        {
                            if(ActiveWire->DataItemIn(2,HType.LIST_TYPE,NULL)==1)
                            {
                                ActiveWire->GetDataItemLenAndType(len,Type);          //取得資料長度與格式
                                if(Type==HType.ASCII_TYPE)
                                {
                                    ActiveWire->DataItemIn(len,Type,str);             //取值
                                    S1=str;
                                    if(i==0)
                                    {
                                        if(S1=="COVERTRAYLIST")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len, Type);                                     //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S2);                                         //取值

                                                S2=StringReplace(S2, ";", ",", TReplaceFlags()<<rfReplaceAll);
                                                slBundlID->Clear();
                                                slBundlID->CommaText=S2;

                                                fSCKART->sBundleList=slBundlID->CommaText;
#if 0   // ===== GATE G02 -- golden SECSGEM/uHGemHT9045.cpp:1682 =====
//   WHY GATED : fObserver is the TfObserverShim stand-in (atester_shims.h:340-402);
//               it has no labBundleID member.
//   DELTA     : The bundle-ID list is still stored (fSCKART->sBundleList, :1681, runs);
//               only the on-screen label is not painted.  No control decision changes.
                                                fObserver->labBundleID->Caption=fSCKART->sBundleList;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G02
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else if(i==1)
                                    {
                                        if(S1=="LOTID")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S3);                                         //取值

                                                fSCKART->sLotID.sprintf("%s", S3);
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else if(i==2)
                                    {
                                        if(S1=="STEP")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len, Type, S3);                                        //取值
#if 0   // ===== GATE G03 -- golden SECSGEM/uHGemHT9045.cpp:1727 =====
//   WHY GATED : forms/fSCKART.h TfSCKART (:74-176) has no sInfo_Step member (the
//               sInfo_* family lives on a DIFFERENT class,
//               Automation/SCK_ART_Remainder.h:712).
//   DELTA     : HCACK=0 is still set on the next line (:1728), so the host still gets
//               an ack, but the STEP value from the host is dropped.  Anything
//               downstream that reads the ART step sees the previous value.
                                                fSCKART->sInfo_Step=S3;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G03
                                                HCACK=0;
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else if(i==3)
                                    {
                                        if(S1=="QTY")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len, Type);                                     //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S3);                                         //取值
#if 0   // ===== GATE G04 -- golden SECSGEM/uHGemHT9045.cpp:1750 =====
//   WHY GATED : forms/fSCKART.h TfSCKART has no sInfo_CurrQty member (same class split
//               as GATE G03).
//   DELTA     : The numeric quantity IS still applied -- :1752/:1753 set
//               fSCKART->iLotCount and ->iInputCount, which are the fields the rest of
//               the port reads.  Only the string mirror is dropped.
                                                fSCKART->sInfo_CurrQty=S3;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G04

                                                fSCKART->iLotCount  =atoi(S3.c_str());
                                                fSCKART->iInputCount=atoi(S3.c_str());

                                                str1.sprintf("Lot start: %s, %d", fSCKART->sLotID, fSCKART->iLotCount);
                                                RecordProcess(str1);            //Steven 20190722 : add TSV log
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }

                                    fMain->Clarn_Data(8, "Clean by TRAY CHECK");                                        //JerryYang 20250307

                                    if(TrayForm.bSpecTrayCnt)
                                    {
                                        fSCKART->iBundleInCnt=TrayForm.iFullTrayCount;
                                        fSCKART->iBundleOutCnt=TrayForm.iInputTrayCount;
                                        iBundleIn=TrayForm.iFullTrayCount;
                                        iBundleOut=TrayForm.iInputTrayCount;
                                    }
                                    fSCKART->AccessFile(false, 1);
                                }
                            }
                            else
                            {
                                HCACK=1;
                                break;
                            }
                        }
                    }
                    else
                    {
                        HCACK=2;
                    }
                }
            }
        }
        else if(S.AnsiPos("TRAYCHKNG")==1)                                      //JerryYang 20250207 : add
        {
            bTRAYCHKNG=true;
            HCACK=0;
        }
        else if(S.AnsiPos("BINDCHKNG")==1)                                      //JerryYang 20250207 : add
        {
            bBindChkNG=true;
            HCACK=0;
        }
        else if(S.AnsiPos("UNBINDCHKNG")==1)                                    //JerryYang 20250207 : add
        {
            bUnbindChkNG=true;
            HCACK=0;
        }
        else if(S.AnsiPos("CLEAR_SOCKET_COUNT")==1)                                   //JerryYang 20250825 : add
        {
            for(int i=0; i<4; i++)
            {
                for(int j=0; j<8; j++)
                {
                    LastSet.iSocketContactCount[i][j]=0;
                }
            }
            HCACK=0;
            bWaitSECS=true;                                                      //KevinC 20250919 : Wait SECS
        }
        else if(S.AnsiPos("CONTINUE_RETEST_ART")==1)                            //ChungHung 20150511 modify
        {
            if(MOT[MMPlate1].HasIC()       || MOT[MMPlate2].HasIC()         ||
                InArmSuck.HasIC()          || OutArmSuck.HasIC()            ||
                ShuttleHasIC()             || IndexHasIC()                  ||
                MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()   )    //2013-04-12    Dell
            {
                HCACK=2;
            }
            else
            {
                SetRunStartMode(rsmContinuRetest_ART);
                HCACK=0;
            }
        }
        else if(S.AnsiPos("CONTINUE_START_ART")==1)                             //ChungHung 20150511 modify
        {
            if(MOT[MMPlate1].HasIC()       || MOT[MMPlate2].HasIC()         ||
                InArmSuck.HasIC()          || OutArmSuck.HasIC()            ||
                ShuttleHasIC()             || IndexHasIC()                  ||
                MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()   )    //2013-04-12    Dell
            {
                HCACK=2;
            }
            else
            {
                SetRunStartMode(rsmContinuStart_ART);
                HCACK=0;
            }
        }
        else if(S.AnsiPos("CLEAN_AUTO_SORT_COUNT")==1 || S.AnsiPos("CLEAN_SORT_COUNT")==1)                              //Ifor 20171016 : add RETEST AUTO SORT COUNT
        {
            if(CUSTOMER_CODE==CC_ASE_CL)                                        //JerryYang 20250120 : modify
            {
                if(InArmSuck.HasIC()          || OutArmSuck.HasIC()         ||
                   ShuttleHasIC()             || IndexHasIC()               ||
                   MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC() )   //2013-04-12    Dell
                {
                    HCACK=2;
                }
//                else if(SystemStart==true)
//                {
//                    HCACK=4;
//                }
                else
                {
//                    bRefreshFunction=true;  //kevin 20181220 add
//                    fMain->Clarn_Data(8, "S2F42 Clear Count");

                    for(i=0; i<TEST_MAX_BIN; i++)
                    {
                        iSVByBinCount[i]=0;
                    }
                    iSV_ErrBinCnt=0;
// ===== GATE G05 -- golden SECSGEM/uHGemHT9045.cpp:1878 -- OPENED 20260916 =====
// AI(W906-G05) 20260916: GATE G05 OPENED by user ruling (docs/RULINGS_20260916.md A1).
//   WAS GATED : "fShowBinSelect has NO port object at all (no header declares
//               it)."  That premise EXPIRED.  forms/fShowBinSelect.h declares
//               `extern TfShowBinSelect *fShowBinSelect;` at :1133 and
//               cShowBinSelect.cpp:160 defines it; the method is declared at
//               forms/fShowBinSelect.h:1023 and that file's banner at :59
//               records `ShowCategoryBin golden :1701-2052 ACTIVE`.
//   DELTA NOW : none -- golden's call is restored verbatim.  What the gate used
//               to cost was the category-bin grid repaint after a host
//               CLEAN_AUTO_SORT_COUNT / CLEAN_SORT_COUNT; the counters above
//               (golden :1873-1877) always ran and HCACK=0 was always
//               returned, so nothing host-visible changes -- display only.
//   REQUIRES  : #include "forms/fShowBinSelect.h", added at :4463 in this
//               block's include list (it replaced an orphaned continuation
//               comment, so no line number in this file moves).
//   SIBLINGS  : G07 / G09 quote the SAME expired premise and are display-only
//               too, but were NOT in the A1 ruling and stay closed.  G18
//               quotes it as well; its action starts an auto-clean CYCLE from
//               a remote host -- machine-moving, needs its own ruling.
                    fShowBinSelect->ShowCategoryBin();

                    HCACK=0;
                }
            }
            else
            {
                if(InArmSuck.HasIC()          || OutArmSuck.HasIC()         ||
                    ShuttleHasIC()             || IndexHasIC()               ||
                    MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC() )  //2013-04-12    Dell
                {
                    HCACK=2;
                }
                else
                {
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                         //kevin 2018905 add
                    {
                        bRefreshFunction=true;                                  //kevin 20181220 add
                        fMain->Clarn_Data(8, "S2F42 Clear Count");
                    }
                    else
                    {
#if 0   // ===== GATE G06 -- golden SECSGEM/uHGemHT9045.cpp:1900-1932 =====
//   WHY GATED : the CC_JCET block needs fContactCT->sgYield (:1903/:1931),
//               fProductionInfo->CalculateNowArmSiteBinQty / ->UpdateControlBinCount
//               (:1905/:1914) and six fYieldMonitoring members (:1913/:1915-:1919).
//               fContactCT has no port object; forms/fProductionInfo.h has neither
//               method; the fYieldMonitoring facade (aHotPlateSubstrate.h:1125) is one
//               method (DoAutoCloseSite) and carries none of them.
//   DELTA     : For CUSTOMER_CODE==CC_JCET the yield counters, site-yield alarm
//               counters, ArmData CT and the Low-Yield-AutoClean state are NOT cleared
//               by this command (golden clears all of them).  The ART counters that
//               follow at :1933-1950 ARE still cleared for every customer including
//               JCET, and HCACK=0 is still returned -- so a JCET host is told the
//               clear succeeded while the yield side kept its old numbers.
                        if(CUSTOMER_CODE==CC_JCET)                              //JerryYang 20210127 : 長電葉宏偉要求清除sort count跟yield計數
                        {
                            fMain->Clarn_Data(8, "btnClearCountClick");
                            fContactCT->sgYield->Refresh();

                            fProductionInfo->CalculateNowArmSiteBinQty(true);
                            for(int k=0; k<3; k++)
                            {
                                ArmData[k]->ClearALLCT();                       //2012-01-03    Dell fix 當按下Count Clear,在Tester Category的I/F Error數值錯誤
                            }

                            //Site Yield Alarm(%)
                            for(i=0; i<32; i++)
                                fYieldMonitoring->bShowSiteYield[i]=false;
                            fProductionInfo->UpdateControlBinCount(true);       //Sam 20200525 : Control Bin
                            fYieldMonitoring->iFailAlarmSiteMaxYieldIntervalCount=0;                                    //jou 2014-08-14 Site Compare Low Yield alarm
                            fYieldMonitoring->iFailAlarmSiteYieldIntervalCount=0;
                            fYieldMonitoring->iAutoClean_FailAlarmSiteYieldIntervalCount=0;
                            fYieldMonitoring->ClearYieldCount();                //Steven 20140830 : Yield相關的Alarm, 要清掉全部的Ignore的Count重算
                            fYieldMonitoring->ClearAutoSiteOffStatus();         //Steven 20200409 : 修正清除count之後,不能開site的問題

                            LastSet.iIndexCount=0;                              //wei 20141201 Low Yield Auto Clean 重置
                            iLowYieldCloseCount=0;
                            bStandardYield=false;
                            for(i=0; i<4; i++)                                  //KEVIN 201050424 FIX
                            {
                                for(j=0; j<8; j++)
                                {
                                    iStandardYield[i][j]=0;
                                }
                            }
                            fContactCT->sgYield->Refresh();
                        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G06
                        for(int k=0; k<4; k++)
                        {
                            if(k==1)                                            //Steven 20190715 : 第1組為每天24hr清一次
                            {
                            }
                            else
                            {
                                LastSet.SendCT_ART[k]=0;
                                for(int i=0; i<eTrayCount; i++)
                                {
                                    LastSet.BinCT_ART[k][iTo3Unload[i]]=0;
                                }
                                for(int j=0; j<TEST_MAX_BIN; j++)
                                {
                                    LastSet.iBinData32_ART[k][j]=0;
                                }
                            }
                        }
// ===== GATE G07 -- golden SECSGEM/uHGemHT9045.cpp:1951-1954 -- OPENED 20260916 =====
// AI(W906-G07) 20260916: OPENED with G09, one wave after G05 (user ruling).
//   WAS GATED : "fShowBinSelect has NO port object (see GATE G05)".  Expired --
//               forms/fShowBinSelect.h:1133 externs it, cShowBinSelect.cpp:160
//               defines it, StrARTSkipICCount is declared at :881.
//   DELTA NOW : none.  The cost used to be the per-tray ART skip-count grid
//               cells not being blanked -- display only, no host-visible change.
                        for(int i=1; i<eTrayCount; i++)                         //Ifor 20240313 21 => eTrayCount
                        {
                            fShowBinSelect->StrARTSkipICCount->Cells[1][i]="";
                        }
#if 0   // ===== GATE G08 -- golden SECSGEM/uHGemHT9045.cpp:1955 =====
//   WHY GATED : forms/fSortCT.h declares ShowLoadingIC() (:71) and ShowSortIC() (:72)
//               but NOT golden's ShowLoadingIC_ART().  Calling the similarly-named
//               ShowLoadingIC() instead would be inventing a different behaviour, so
//               the call is gated rather than substituted.
//   DELTA     : The ART loading-count panel is not repainted.  Display only.
//               fSortCT->ShowSortIC() on the next line IS still called.
                        fSortCT->ShowLoadingIC_ART();
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G08
                        fSortCT->ShowSortIC();
// ===== GATE G09 -- golden SECSGEM/uHGemHT9045.cpp:1957 -- OPENED 20260916 =====
// AI(W906-G09) 20260916: OPENED with G07, one wave after G05 (user ruling).
//   WAS GATED : "fShowBinSelect has NO port object (see GATE G05)".  Expired.
//   DELTA NOW : none.  This is the SAME call as G05 (ShowCategoryBin) on the
//               other CLEAN_*_COUNT branch.  Leaving one of the two open was a
//               man-made inconsistency -- the same host command would repaint on
//               one arm and not on the other.  Display only.
                        fShowBinSelect->ShowCategoryBin();
                    }
                HCACK=0;
                RecordProcess("SECS/GEM CLEAN AUTO SORT COUNT");
                }
            }
        }
        else if(S.AnsiPos("RETEST_MRT")==1)                                     //Ifor 20170425 add RETEST_MRT Remote Command
        {
            if(MOT[MMPlate1].HasIC()       || MOT[MMPlate2].HasIC()         ||
                InArmSuck.HasIC()          || OutArmSuck.HasIC()            ||
                ShuttleHasIC()             || IndexHasIC()                  ||
                MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()   )    //2013-04-12    Dell
            {
                HCACK=2;
            }
            else
            {
                SetRunStartMode(rsmRetest_MRT);
                HCACK=0;
            }
        }
        else if(S.AnsiPos("INITIAL_START_MRT")==1)                              //Ifor 20170425 add INITIAL_START_MRT Remote Command
        {
            if(MOT[MMPlate1].HasIC()       || MOT[MMPlate2].HasIC()         ||
                InArmSuck.HasIC()          || OutArmSuck.HasIC()            ||
                ShuttleHasIC()             || IndexHasIC()                  ||
                MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()   )    //2013-04-12    Dell
            {
                HCACK=2;
            }
            else
            {
                SetRunStartMode(rsmInitial_MRT);

                if(TrayForm.bEnableAMR) //Eastsun 20260515 F019 AMR
                    bSameSetupFileNoDownload=true;
                else
                    bSameSetupFileNoDownload=false;

                HCACK=0;
            }
        }
        else if(S.AnsiPos("CONTINUE_START_MRT")==1)                             //Ifor 20170425 add CONTINUE_START_MRT Remote Command
        {
            if(MOT[MMPlate1].HasIC()       || MOT[MMPlate2].HasIC()         ||
                InArmSuck.HasIC()          || OutArmSuck.HasIC()            ||
                ShuttleHasIC()             || IndexHasIC()                  ||
                MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()   )    //2013-04-12    Dell
            {
                HCACK=2;
            }
            else
            {
                SetRunStartMode(rsmContinuStart_MRT);
                HCACK=0;
            }
        }
        else if(S.AnsiPos("INITIAL_START")==1)                                  //kevin 20180806 (wei) add Initial startR 新增S2F41 回盤
        {
            if(MOT[MMPlate1].HasIC()      || MOT[MMPlate2].HasIC()          ||
               InArmSuck.HasIC()          || OutArmSuck.HasIC()             ||
               ShuttleHasIC()             || IndexHasIC()                   ||
               MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()    )
            {
                HCACK=2;
            }
            else
            {
                SetRunStartMode(rsmInitialStart);

                if(TrayForm.bEnableAMR) //Eastsun 20260515 F019 AMR
                    bSameSetupFileNoDownload=true;
                else
                    bSameSetupFileNoDownload=false;

                HCACK=0;
            }
        }
        else if(S.AnsiPos("REMOTE_SAVE")==1)                                    //kevin 20180914 (Steven) : add read ESD data
        {
            if(HasICUnderMachine()==true || SystemStart==true)
            {
                HCACK=4;
            }
            else
            {
                HCACK=0;
            }
        }
        else if(S.AnsiPos("AUTOSITEMAP")==1)                                    //kevin 20180806 (wei) add AutoSiteMap 新增S2F41 回盤
        {
            if(MOT[MMPlate1].HasIC()      || MOT[MMPlate2].HasIC()          ||
               InArmSuck.HasIC()          || OutArmSuck.HasIC()             ||
               ShuttleHasIC()             || IndexHasIC()                   ||
               MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()    )
            {
                HCACK=2;
            }
            else
            {
                RecordProcess("Trigger Auto Site Map by SECS GEM");
                SetRunStartMode(rsmAutoSiteMap);
                HCACK=0;
            }
        }
        else if(S.AnsiPos("AUTO_RETEST")==1)                                    //wei 20170217 (Steven) TSMC ATR 新增S2F41 回盤
        {
            if(MOT[MMPlate1].HasIC()      || MOT[MMPlate2].HasIC()          ||
               InArmSuck.HasIC()          || OutArmSuck.HasIC()             ||
               ShuttleHasIC()             || IndexHasIC()                   ||
               MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()    )
            {
                HCACK=2;
            }
            else
            {
                DoAutoRetest(true);
                SetRunStartMode(rsmAutoRetest);
                SoftStart=true;
                HCACK=0;
            }
        }
        else if(S.AnsiPos("TRAY_FEED")==1 || S.AnsiPos("TRAY FEED")==1)         //wei 20170217 (Steven) TSMC ATR 新增S2F41 Tray Feed
        {
            if(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND)     //Ifor 20251018 add:Analog 泰國客戶要求Onecycle後需要可以Tray Feed
            {
                if(InArmSuck.HasIC()          || OutArmSuck.HasIC()             ||
                   ShuttleHasIC()             || IndexHasIC()                   ||
                   MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()    )
                {
                    HCACK=2;
                }
                else
                {
                    fMain->BtnTrayEndClick(fMain);
                    HCACK=0;
                }
            }
            else
            {
                if(MOT[MMPlate1].HasIC()      || MOT[MMPlate2].HasIC()          ||
                   InArmSuck.HasIC()          || OutArmSuck.HasIC()             ||
                   ShuttleHasIC()             || IndexHasIC()                   ||
                   MOT[MInRotateKit].HasIC()  || MOT[MOutRotateKit].HasIC()    )
                {
                    HCACK=2;
                }
                else
                {
                    NewRecordProcess("MES2118", "TRAY FEED pressed", "S2F42 1");
                    fMain->BtnTrayEndClick(fMain);
                    if(!(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND))
                        SoftStart=true;
                    HCACK=0;
                }
            }
        }
        else if (S.AnsiPos("RETRY")==1)
        {
#if 0   // ===== GATE G10 -- golden SECSGEM/uHGemHT9045.cpp:2117-2121 =====
//   WHY GATED : forms/fNote.h TfNote (:96-168) carries fShow but NOT ReturnCode and
//               NOT Close().
//   DELTA     : HCACK=0 is still returned (:2122, outside this gate), so the host sees
//               the same reply, but a displayed alarm dialog is NOT dismissed with
//               K_RETRY -- the operator still has to press Retry on the machine.
//               Offline fNote->fShow is always false, so the gated block would not
//               have executed anyway.
            if(fNote->fShow)
            {
                fNote->ReturnCode=K_RETRY;
                fNote->Close();
            }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G10
            HCACK=0;
        }
        else if(S.AnsiPos("TRAY END")==1)
        {
#if 0   // ===== GATE G11 -- golden SECSGEM/uHGemHT9045.cpp:2126-2130 =====
//   WHY GATED : fNote->ReturnCode / fNote->Close() absent (see GATE G10).
//   DELTA     : HCACK=0 still returned; the alarm dialog is not dismissed with
//               K_TRAY_END.
            if(fNote->fShow)
            {
                fNote->ReturnCode=K_TRAY_END;
                fNote->Close();
            }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G11
            HCACK=0;
        }
        else if(S.AnsiPos("INITIAL_START_ART")==1)                              //ChungHung 20150511 modify
        {
            if(HasICUnderMachine()==false)
            {
                SetRunStartMode(rsmInitial_ART);

                if(TrayForm.bEnableAMR) //Eastsun 20260515 F019 AMR
                    bSameSetupFileNoDownload=true;
                else
                    bSameSetupFileNoDownload=false;

                HCACK=0;
            }
            else
            {
                HCACK=2;
            }
        }
#if 0   // ===== GATE G12 -- golden SECSGEM/uHGemHT9045.cpp:2151-2215 =====
//   WHY GATED : the whole arm is fFTPClient (:2153/:2194/:2195/:2196), and there is NO
//               fFTPClient object anywhere in the port -- KYECFTP/FTPClient_*.{h,cpp}
//               translated four transfer FUNCTIONS as free functions and demoted a few
//               members to globals (KYECFTP/FTPClient_Transfer.h:85-90); the
//               TfFTPClient singleton itself was never created.
//   DELTA     : DOWNLOAD_RECIPE_BY_FTP leaves the ladder, so the host is answered
//               HCACK=1 by the final else instead of HCACK=0 + an FTP recipe pull.
//               A host-driven recipe download over FTP is simply not available; the
//               host learns that from the reply rather than timing out on a download
//               that never starts.
        else if(S.Pos("DOWNLOAD_RECIPE_BY_FTP")!=0)                             //ChungHung 20150511 modify
        {
            if(fFTPClient->bControlBySECSGEM==true)
            {
                HCACK=7;                                                        //Steven 20240923 : S2F42 HCACK 1 --> 7
            }
            else if(HasICUnderMachine()==true)
            {
                HCACK=2;
            }
            else
            {
                if(HGem->GetDataItemLenAndType(len, HType.LIST_TYPE))
                {
                    if(len==1)
                        HGem->DataItemIn(len, HType.LIST_TYPE, NULL);           //Steven 20231226 : 舊的版本少一個L

                    if(HGem->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                    {
                        HGem->GetDataItemLenAndType(len, Type);
                        CommandStr=new char [len+1];                            //JerryYang 20241205 : 字串陣列大小改成動態
//                        ZeroMemory(CommandStr, sizeof(CommandStr));
                        memset(CommandStr, 0, len+1);                           //JerryYang 20250325 : fix
                        ret=HGem->DataItemIn(len, HType.ASCII_TYPE, CommandStr);

                        if(ret==-1)
                            S2="";
                        S2=CommandStr;
                        delete[] CommandStr;                                    //Steven 20160912 : Add delete for save memory
                        CommandStr=NULL;

                        if(S2=="Setup_File")
                        {
                            HGem->GetDataItemLenAndType(len, Type);
                            CommandStr=new char [len+1];                        //JerryYang 20241205 : 字串陣列大小改成動態
//                            ZeroMemory(CommandStr, sizeof(CommandStr));
                            memset(CommandStr, 0, len+1);                       //JerryYang 20250325 : fix
                            ret=HGem->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                            if(ret==-1)
                                S3="";
                            S3=CommandStr;
                            delete[] CommandStr;                                //Steven 20160912 : Add delete for save memory
                            CommandStr=NULL;
                            fFTPClient->bControlBySECSGEM=true;
                            fFTPClient->aSetUpNameBySECSGEM=S3;
                            fFTPClient->ShowFTPModal(0);
                            HCACK=0;
                            bSecsGemDownloadFTP=true;                           //wei 20170119 (Steven) DownLoad 沒有馬上按掉會Time Out
                        }
                        else
                        {
                            HCACK=9;                                            //Steven 20240923 : S2F42 HCACK 1 --> 9
                        }
                    }
                    else
                    {
                        HCACK=8;                                                //Steven 20240923 : S2F42 HCACK 1 --> 8
                    }
                }
                else
                {
                    HCACK=10;                                                   //Steven 20240923 : S2F42 HCACK 1 --> 10
                }
            }
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G12
        else if(S.AnsiPos("CLEAN_OUT")==1 || S.AnsiPos("CLEAN OUT")==1)
        {
            if(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND)
            {
                if(fNote->fShow)
                {
#if 0   // ===== GATE G13 -- golden SECSGEM/uHGemHT9045.cpp:2222-2223 =====
//   WHY GATED : fNote->ReturnCode / fNote->Close() absent (see GATE G10).
//   DELTA     : HCACK=0 on :2224 still runs, so for CC_MAXIM / CC_MAXIM_THAILAND with a
//               dialog up the host is acked but the dialog is not closed with
//               K_CLEAN_OUT.  With no dialog up (always, offline) the else branch
//               at :2226-2230 runs fMain->CleanOut("S2F42 1") unchanged.
                    fNote->ReturnCode=K_CLEAN_OUT;
                    fNote -> Close();
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G13
                    HCACK=0;
                }
                else
                {
                    fMain->CleanOut("S2F42 1");
                    HCACK=0;
                }
            }
            else
            {
                fMain->CleanOut("S2F42 1");
                HCACK=0;
            }
        }
        else if(S.AnsiPos("SWITCH_TO_FT")==1)                                   //Steven 20141016 : Add RCMD SWITCH_TO_FT
        {
            iSecsGemSwitchFTRT=1;                                               //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功
            HCACK=fMain->FTClick();                                             //Steven 20210423 :修改FT/RT Click回覆動作
            if(HCACK!=0)
            {
                if(HCACK==1)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d (SystemStart)", HCACK);
                else if(HCACK==2)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d (Picker has IC)", HCACK);
                else if(HCACK==3)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d (RunStartMode is disable)", HCACK);
                else if(HCACK==4)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d (Tray has IC)", HCACK);
                else
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d", HCACK);
                ActiveWire->StringOut(S2);
            }

            if(SystemStart || iSecsGemSwitchFTRT==1 || HCACK!=0)
                HCACK=2;
            else
                HCACK=0;
            iSecsGemSwitchFTRT=0;
        }
        else if(S.AnsiPos("SWITCH_TO_RT")==1)                                   //Steven 20141016 : Add RCMD SWITCH_TO_RT
        {
            iSecsGemSwitchFTRT=1;                                               //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功
            HCACK=fMain->RTClick();                                             //Steven 20210423 :修改FT/RT Click回覆動作
            if(HCACK!=0)
            {
                if(HCACK==1)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_RT Fail! HCACK=%d (SystemStart)", HCACK);
                else if(HCACK==2)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_RT Fail! HCACK=%d (Tray has IC)", HCACK);
                else if(HCACK==3)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_RT Fail! HCACK=%d (Auto Site Map)", HCACK);
                else if(HCACK==4)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_RT Fail! HCACK=%d (Picker has IC)", HCACK);
                else if(HCACK==5)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d (RunStartMode is disable)", HCACK);
                else if(HCACK==6)
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_FT Fail! HCACK=%d (Tray has IC)", HCACK);
                else
                    S2.sprintf("[S2F42]RCMD SWITCH_TO_RT Fail! HCACK=%d", HCACK);
                ActiveWire->StringOut(S2);
            }

            if(SystemStart || iSecsGemSwitchFTRT==1 || HCACK!=0)
                HCACK=2;
            else
                HCACK=0;
            iSecsGemSwitchFTRT=0;
        }
#if 0   // ===== GATE G14 -- golden SECSGEM/uHGemHT9045.cpp:2292-2296 =====
//   WHY GATED : THGemSpeedButton (SECSGEM/uHGemEquipment.h:486-490) has only `Enabled`
//               -- no Click().  Additionally GemBtnOnlineLocal defaults to NULL
//               (uHGemEquipment.h:868) and `THGem *HGem` itself is NULL
//               (uHGemEquipment.cpp:3521), so the golden line is a double NULL deref
//               here even if Click() existed.
//   DELTA     : ONLINE_LOCAL leaves the ladder -> HCACK=1 from the final else.  The
//               host cannot drive the equipment into ONLINE/LOCAL remotely; the
//               control-state change must be made at the panel.
        else if(S.AnsiPos("ONLINE_LOCAL")==1)                                   //Steven 20150604 : Add ONLINE_LOCAL for SECS GEM
        {
            HGem->GemBtnOnlineLocal->Click();
            HCACK=0;
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G14
#if 0   // ===== GATE G15 -- golden SECSGEM/uHGemHT9045.cpp:2297-2301 =====
//   WHY GATED : THGemSpeedButton has no Click(); GemBtnOnlineRemote is NULL
//               (uHGemEquipment.h:867).  Same shape as GATE G14.
//   DELTA     : ONLINE_REMOTE leaves the ladder -> HCACK=1.  The host cannot drive the
//               equipment into ONLINE/REMOTE remotely.
        else if(S.AnsiPos("ONLINE_REMOTE")==1)                                  //Steven 20150604 : Add ONLINE_REMOTE for SECS GEM
        {
            HGem->GemBtnOnlineRemote->Click();
            HCACK=0;
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G15
        else if(S=="RESUME")                                                    //JerryYang 20250120 : modify
        {
            if(IniConfig.bEnable_SECS_GEM==true)
            {
                bSECSGEMAlarm=false;
                bSECSPause=false;
            }
            HCACK=0;
        }
        else if(S.AnsiPos("REMOTE_START")==1)                                   //20180817 add
        {
            if(SystemStart==false && CosFunction.bCanRemoteStart)               //Steven 20250712 : 有通過風險告知可以遠端start的客戶
            {
                HCACK=0;
                if(TrayForm.bEnableAMR) //Eastsun 20260515 F019 AMR
                {
                    if(RunInfo.bLotStart==true)
                    {
                        bSameSetupFileNoDownload=true;
                        fMain->Start("SECS GEM RCMD : REMOTE_START");
                    }
                }
                else
                {
                    fMain->Start("SECS GEM RCMD : REMOTE_START");
                }
            }
            else
            {
                HCACK=1;
            }
        }
//        else if(S.AnsiPos("START")==1)                                          //Steven 20141016 : Add RCMD START
//        {
//            if(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND)
//            {
//                if(SystemStart==false)
//                {
//                    HCACK=0;
//                    fMain->Start("SECS GEM RCMD : START");
//                }
//                else
//                {
//                    HCACK=1;
//                }
//            }
//            else if(IniConfig.bEnable_SECS_GEM==true &&
//                    CosFunction.bCanRemoteStart)                                //Steven 20250712 : 有通過風險告知可以遠端start的客戶
//            {
//                HCACK=0;
//                bSECSGEMAlarm=false;                                            //Ifor 20151208 :解除按鍵Lock
//                bHasSaveSet=false;                                              //Ifor 20151208 :清除設定檔變更旗標
//                fMain->Start("SECS GEM RCMD : START");
//            }
//            else if(IniConfig.bEnable_SECS_GEM==true &&                         //Steven 20141006 : SECS GEM使用Remote Start功能
//                    ((IniConfig.bRCMDStart==true && bPhysicalStart==true) ||
//                    bSecsGemCanStart))                                          //wei 20150630  SecsGem Can Start
//            {
//                SoftStart=true;
//                RecordProcess("SECS GEM RCMD : RUN CHECK START");
//                bSecsGemCanStart=false;
//                HCACK=0;
//                bSECSGEMAlarm=false;                                            //Ifor 20151208 :解除按鍵Lock
//                bHasSaveSet=false;                                              //Ifor 20151208 :清除設定檔變更旗標
//            }
//            else
//            {
//                HCACK=1;
//            }
//            bPhysicalStart=false;
//            bNeedDoRunCheck=false;                                              //JerryYang 20250120 : modify
//        }
        else if(S.AnsiPos("HOME")==1)                                           //Steven 20250712 : Add RCMD HOME
        {
            if(IniConfig.bEnable_SECS_GEM==true &&
               CosFunction.bCanRemoteStart)                                     //Steven 20250712 : 有通過風險告知可以遠端start的客戶
            {
                HCACK=0;
                bSECSGEMAlarm=false;                                            //Ifor 20151208 :解除按鍵Lock
                bHasSaveSet=false;                                              //Ifor 20151208 :清除設定檔變更旗標
                RecordProcess("SECS GEM RCMD : Home");
                fMain->Home("S2F42");
            }
            else
            {
                HCACK=1;
            }
        }
        else if(S.AnsiPos("HALT")==1)                                           //Steven 20141016 : Add RCMD HALT
        {
            if(IniConfig.bEnable_SECS_GEM==true &&
               IniConfig.bRCMDStart==true &&
               bPhysicalStart==true)                                            //Steven 20141006 : SECS GEM使用Remote Start功能
            {
                SoftStart=false;
                RecordProcess("SECS GEM RCMD : Handler halt by Host");
//                HSys.MyGem->SecsAlarmMessage->Add("Handler halt by Host");    //Steven 20150519 : AdanYu說不要顯示
                HCACK=0;
            }
            else
            {
                HCACK=2;
            }
            bPhysicalStart=false;
        }
#if 0   // ===== GATE G16 -- golden SECSGEM/uHGemHT9045.cpp:2407-2435 =====
//   WHY GATED : the arm exists only to write SECS_GEM_PPMUSIC_CONTROL_flag and
//               iSECS_GEM_PPMUSIC_CONTROL_CLASS (:2413/:2426/:2427).  Both are extern-
//               declared at ckernel.cpp:1272-1273 and DEFINED NOWHERE in the port, so
//               referencing either is a link error that -fsyntax-only cannot see.  The
//               sibling uHGemHT9045_SV.cpp hit the same hole for the tower-light trio
//               and gated it as its own [G3] (SV.cpp:459).
//   DELTA     : PP_MUSIC leaves the ladder -> HCACK=1.  Host-driven buzzer/music class
//               selection is unavailable (in this port nothing consumes those globals
//               either, so no other behaviour changes).
        else if(S.AnsiPos("PP_MUSIC")==1)                                       //Steven 20150604 : Add PP_MUSIC for SECS GEM
        {
            if(HGem->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)                                         //wei 20150630
            {
                if(SVlen==0)
                {
                    SECS_GEM_PPMUSIC_CONTROL_flag=false;
                    HCACK=0;
                }
                else
                {
                    if(HGem->DataItemIn(2, HType.LIST_TYPE,NULL)==1)
                    {
                        HGem->GetDataItemLenAndType(len, Type);                 //取得資料長度與格式
                        HGem->DataItemIn(len, Type,str);                        //取值
                        HGem->GetDataItemLenAndType(len, Type);                 //取得資料長度與格式
                        HGem->DataItemIn(len, Type, &ret);                      //取值

                        HCACK=0;
                        iSECS_GEM_PPMUSIC_CONTROL_CLASS=ret;
                        SECS_GEM_PPMUSIC_CONTROL_flag=true;
                    }
                }
            }
            else
            {
                HCACK=1;
            }
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G16
#if 0   // ===== GATE G17 -- golden SECSGEM/uHGemHT9045.cpp:2436-2479 =====
//   WHY GATED : the arm writes SECS_GEM_PPSIGNALTOWER_CONTROL_flag and
//               iSECS_GEM_PPSIGNALTOWER_CONTROL_RED/GREEN/YELLOW (:2442/:2459-:2466).
//               All four are extern-declared at ckernel.cpp:1275-1278 and DEFINED
//               NOWHERE -- link errors, same hole as GATE G16.  Gating the individual
//               assignments was rejected because they sit inside a
//               RED/GREEN/YELLOW/else if-chain whose else sets HCACK=1: removing only
//               the assignments would make a valid RED command answer 0 while doing
//               nothing, which is a worse lie than not offering the command.
//   DELTA     : PP_SIGNALTOWER leaves the ladder -> HCACK=1.  The host cannot drive the
//               three-colour tower.
        else if(S.AnsiPos("PP_SIGNALTOWER")==1)                                 //Steven 20150604 : Add PP_SIGNALTOWER for SECS GEM
        {
            if(HGem->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)                                         //wei 20150630
            {
                if(SVlen==0)
                {
                    SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;
                    HCACK=0;
                }
                else
                {
                    for(i=0; i<SVlen; i++)
                    {
                        if( HGem->DataItemIn( 2,HType.LIST_TYPE,NULL)==1)
                        {
                            HGem->GetDataItemLenAndType(len,Type);              //取得資料長度與格式
                            HGem->DataItemIn( len,Type,str);                    //取值

                            HGem->GetDataItemLenAndType(len,Type);              //取得資料長度與格式
                            HGem->DataItemIn( len,Type,&ret);                   //取值
                            S1=str;
                            HCACK=0;
                            if(S1=="RED")
                                iSECS_GEM_PPSIGNALTOWER_CONTROL_RED=ret;
                            else if(S1=="GREEN")
                                iSECS_GEM_PPSIGNALTOWER_CONTROL_GREEN=ret;
                            else if(S1=="YELLOW")
                                iSECS_GEM_PPSIGNALTOWER_CONTROL_YELLOW=ret;
                            else
                                HCACK=1;
                            SECS_GEM_PPSIGNALTOWER_CONTROL_flag=true;
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                }
            }
            else
            {
                HCACK=1;
            }
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G17
#if 0   // ===== GATE G18 -- golden SECSGEM/uHGemHT9045.cpp:2480-2495 =====
//   WHY GATED : STILL CLOSED BY USER RULING 20260916.  The old premise
//               ("fShowBinSelect has NO port object") expired -- G05/G07/G09 were
//               opened on that same expiry the same day.  This one was NOT: a
//               premise expiring is not the decision.  The action is
//               fShowBinSelect->btnAutoCleanClick() (golden :2488) = an auto-clean
//               ONE-CYCLE; Index/Shuttle/clean kit physically MOVE, commanded by a
//               remote host with nobody at the machine.  Re-opening is a machine-
//               side (BU) decision, not an FW wave.  bSECSGEMAutoclean=true (:2486)
//               alone starts nothing -- it is read once the cycle is under way.
        else if(S.AnsiPos("AUTO_CLEAN")==1)
        {
            if(IniConfig.bEnableAutoCleanFunction &&
               TestIF.iAutoClean_Function==true &&
               bRunAutoClean==false)                                            //Ifor 20220512 add:SECS GEM Auto Clean啟動中或無開啟回覆1
            {
                bSECSGEMAutoclean=true;                                         //Ifor 20220414 add SECS/GEM AUTO Clean
                RecordProcess("Start ONE CYCLE by SECS/GEM Auto Clean...");
                fShowBinSelect->btnAutoCleanClick(fShowBinSelect);
                HCACK=0;
            }
            else
            {
                HCACK=1;
            }
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G18
        else if(S.AnsiPos("PP_PASSWORD")==1)                                    //wei 20150803 SECSGEM更改PASSWORD
        {
            if(ActiveWire->GetDataItemLenAndType(len, HType.LIST_TYPE))
            {
                if(len==1)
                    ActiveWire->DataItemIn(len, HType.LIST_TYPE, NULL);               //Steven 20231226 : 舊的版本少一個L

                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                {
                    ActiveWire->GetDataItemLenAndType(len, Type);                     //取得資料長度與格式
                    ActiveWire->DataItemIn(len, Type, str);                           //取值

                    ActiveWire->GetDataItemLenAndType(len, Type);                     //取得資料長度與格式
                    ActiveWire->DataItemIn(len, Type, &ret);                          //取值

                    asSECSGEMChangeName=str;
                    asSECSGEMChangePassword=ret;
                    fMain->ChangePassword();                                    //wei 20150803
                    HCACK=0;
                }
                else
                {
                    HCACK=1;
                }
            }
        }
         else if(S.AnsiPos("REMOTE_SAVE")==1)                                   //kevin 20180823 add
        {
            if(HasICUnderMachine()==true || SystemStart==true)
            {
                HCACK=4;
            }
            else
            {
                  HCACK=0;
            }
        }
        else if(S.AnsiPos("PP_SELECT")==1 || S.AnsiPos("PP-SELECT")==1)         //Steven 20140929 : 新增PP-Select的Remote Command
        {
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20180821 add
            {
                ActiveWire->GetDataItemLenAndType(len, Type);
                if(Type==HType.ASCII_TYPE)
                {
                    ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, PPID);
                    if(ret==-1)
                        PPID="";
                }
                else
                {
                    PPID="";
                }

        // AI(W906-PT-W5-S2F42) 20260809: PORT-MECHANICAL (rename 2 in the banner).
        // golden :2549 is `fMain->cbSetupFileName->Items->Text.AnsiPos(PPID)==0`;
        // this port models TStringList::Text as a proxy object
        // (vclcompat/TStringList.h:112-120) that converts to AnsiString but has no
        // AnsiPos member, so the conversion is spelled out.  Same value tested.
                if(AnsiString(fMain->cbSetupFileName->Items->Text).AnsiPos(PPID)==0)
                {
                    HCACK=6;                                                    //Steven 20160425 : 改成HCACK=5-->6
                }
                else
                {
                    if(HasICUnderMachine()==true || SystemStart==true)
                    {
                        HCACK=4;
                    }
                    else
                    {
                        fMain->cbSetupFileName->Text=PPID;
                        fMain->cbSetupFileNameChange(fMain);
                        HCACK=0;
#if 0   // ===== GATE G19 -- golden SECSGEM/uHGemHT9045.cpp:2564-2565 =====
//   WHY GATED : forms/fLotInfo.h TfLotInfo has no ClearAllSetupFile member.  The
//               enclosing `if(CosFunction.bKeepOnly1SetupFile)` is gated with it so no
//               empty if-body is left behind.
//   DELTA     : For CC_ASE_KaohSiung with bKeepOnly1SetupFile on, the OTHER setup files
//               are not deleted after the switch.  HCACK is unaffected (already 0 at
//               :2563).  Disk housekeeping only -- but on a machine that relies on
//               "exactly one setup file present", later file-list logic sees extras.
                        if(CosFunction.bKeepOnly1SetupFile)
                            fLotInfo->ClearAllSetupFile(PPID);                  //Steven 20231209 : Add for 下載完工作檔後, 只留一個就好
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G19
                    }
                }
            }
            else if(CUSTOMER_CODE==CC_ONSEMI_M ||
                    CUSTOMER_CODE==CC_IFXTH_Thai ||
                    CUSTOMER_CODE==CC_ASE_CL ||                                 //JerryYang 20250120 : modify
                    IniConfig.bVTESTFunction==true)                             //RogerYang 20260608 : Add for VTEST SECS
            {
                if(ActiveWire->DataItemIn(1, HType.LIST_TYPE, NULL)==1)
                {
                    if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                    {
                        ActiveWire->GetDataItemLenAndType(len, Type);                 //取得資料長度與格式
                        ActiveWire->DataItemIn(len, Type, str);                       //取值
                        S1=str;
                        if(S1=="PPID" || CUSTOMER_CODE==CC_ASE_CL)              //JerryYang 20250120 : modify
                        {
                            ActiveWire->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, PPID);
                                if(ret==-1)
                                    PPID="";
                            }
                            else
                            {
                                PPID="";
                            }
                        }
                        else
                        {
                            PPID="";
                        }

                        bHasFile=false;
                        for(int i=0; i<fMain->cbSetupFileName->Items->Count; i++)
                        {
                            if(PPID!="" && PPID==fMain->cbSetupFileName->Items->Strings[i])
                                bHasFile=true;
                        }

                        if(bHasFile==false)
                        {
                            HCACK=6;                                            //Steven 20160425 : 改成HCACK=5-->6
                        }
                        else
                        {
                            if(HasICUnderMachine()==true || SystemStart==true)
                            {
                                HCACK=4;
                            }
                            else
                            {
                                fMain->cbSetupFileName->Text=PPID;
                                fMain->cbSetupFileNameChange(fMain);
                                HCACK=0;
                            }
                        }
                    }
                    else
                    {
                        HCACK=3;
                    }
                }
                else
                {
                    HCACK=2;
                }
            }
            else if(CUSTOMER_CODE==CC_SIGURD_ChungXing)                         //Sam 20250325 : 矽格中興 PPID
            {
                if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
                {
                    if(SVlen==0)
                    {
                        HCACK=2;
                    }
                    else
                    {
                        if(SVlen==2)
                        {
                            for(i=0; i<SVlen; i++)
                            {
                                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                                {
                                    ActiveWire->GetDataItemLenAndType(len, Type);     //取得資料長度與格式
                                    ActiveWire->DataItemIn(len, Type, str);           //取值
                                    S1=str;
                                    if(i==0)
                                    {
                                        if(S1=="PPID")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len, Type);
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, PPID);
                                                if(ret==-1)
                                                    PPID="";
                                            }
                                            else
                                            {
                                                PPID="";
                                            }
                                        }
                                        else
                                        {
                                            PPID="";
                                        }

                                        bHasFile=false;
                                        for(int i=0; i<fMain->cbSetupFileName->Items->Count; i++)
                                        {
                                            if(PPID!="" && PPID==fMain->cbSetupFileName->Items->Strings[i])
                                                bHasFile=true;
                                        }

                                        if(bHasFile==false)
                                        {
                                            HCACK=6;
                                        }
                                        else
                                        {
                                            if(HasICUnderMachine()==true || SystemStart==true)
                                            {
                                                HCACK=4;
                                            }
                                            else
                                            {
                                                fMain->cbSetupFileName->Text=PPID;
                                                fMain->cbSetupFileNameChange(fMain);
                                                HCACK=0;
                                            }
                                        }
                                    }
                                    else if(i==1)
                                    {
                                        if(S1=="ICQUANTITY")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S3);                                         //取值
                                                iHead=atoi(S3.c_str());
                                                if(iHead>0)
                                                {
                                                    LastSet.iP57_InputCT=iHead;                                         //Sam 20250605 : Loader Count AutoCleanOut
                                                    HCACK=0;
                                                }
                                                else
                                                {
                                                    HCACK=2;
                                                    break;
                                                }
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    HCACK=3;
                                }
                            }
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else if(ActiveWire->DataItemIn(1, HType.LIST_TYPE, NULL)==1)           // 需要補充多重 Command
            {
                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                {
                    ActiveWire->GetDataItemLenAndType(len, Type);

                    if(Type==HType.ASCII_TYPE)                                  //Steven 20250725
                    {
                        ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, str);
                        S1=str;
                        if(S1=="PPID")
                        {
                            ActiveWire->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, PPID);
                                if(ret==-1)
                                    PPID="";
                            }
                            else
                            {
                                PPID="";
                            }
                        }
                        else
                        {
                            PPID=str;
                            ActiveWire->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, str);
                            }
                        }
                    }
                    else
                    {
                        PPID="";
                        ActiveWire->GetDataItemLenAndType(len, Type);
                        if(Type==HType.ASCII_TYPE)
                        {
                            ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, str);
                        }
                    }

                    bHasFile=false;
                    for(int i=0; i<fMain->cbSetupFileName->Items->Count; i++)
                    {
                        if(PPID!="" && PPID==fMain->cbSetupFileName->Items->Strings[i])
                            bHasFile=true;
                    }

                    if(bHasFile==false)
                    {
                        HCACK=6;                                                //Steven 20160425 : 改成HCACK=5-->6
                    }
                    else
                    {
                        if(HasICUnderMachine()==true || SystemStart==true)
                        {
                            HCACK=4;
                        }
                        else
                        {
                            fMain->cbSetupFileName->Text=PPID;
                            fMain->cbSetupFileNameChange(fMain);
                            HCACK=0;
                        }
                    }

//                    HGemPtr->GetDataItemLenAndType(len, Type);        //JerryYang 20250725 : 多做的, MARK掉
//                    if(Type==HType.ASCII_TYPE)
//                    {
//                        ret=HGemPtr->DataItemIn(len, HType.ASCII_TYPE, PPID);
//                    }

                    #ifndef FOR_NVIDIA_2D_SORT                                  //JerryYang 20230222 : 中科佳毅要求切換工作檔時開啟run check
                    if(IniConfig.bSPILFunction && IniConfig.bRCMDStart==false)
                    {
                        AnsiString sPath=AuthPath+"config.ini";
                        IniConfig.bRCMDStart=true;
                        IniConfig.bRCMDStart=ReadWriteIni(sPath, "SECS GEM", "Enable RCMD START", IniConfig.bRCMDStart, false, false);
#if 0   // ===== GATE G20 -- golden SECSGEM/uHGemHT9045.cpp:2831 =====
//   WHY GATED : fConfiguration in this port is W5SckArtRem_ConfigStub
//               (Automation/SCK_ART_Remainder.h:594-599) and carries only mmoN04_IP --
//               no cbN07_EnableHostStart checkbox.
//   DELTA     : The ini IS still written (:2828-2830 run: IniConfig.bRCMDStart=true and
//               ReadWriteIni persists it to config.ini), so the FUNCTIONAL half of
//               golden's SPIL fix-up survives.  Only the on-screen checkbox is not
//               force-ticked, which in golden was there to stop the temperature page
//               saving the flag back off.  On a real machine that protection is lost:
//               a subsequent save from the temperature page can clear Enable RCMD START
//               again.
                        fConfiguration->cbN07_EnableHostStart->Checked=true;    //JerryYang 20250826 : 元件也要強制打勾避免進溫度頁面按存檔又被關閉
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G20
                    }
                    #endif
                }
                else
                {
                    HCACK=3;
                }
            }
            else
            {
                HCACK=2;
            }
        }
#if 0   // ===== GATE G21 -- golden SECSGEM/uHGemHT9045.cpp:2845-2850 =====
//   WHY GATED : fLotInfo->sbSECSLotStartClick(fLotInfo) (:2847) does not exist in
//               forms/fLotInfo.h; it is the entire arm.
//   DELTA     : LOTSTART leaves the ladder -> HCACK=1 instead of HCACK=0 + a lot start.
//               The TSMC lot-gating flow this arm exists for cannot be driven by the
//               host.
        else if(S.AnsiPos("LOTSTART")==1)                                       //wei 20160517 TSMC lot卡關
        {
            fLotInfo->sbSECSLotStartClick(fLotInfo);
            RecordProcess("SECS/GEM LOTSTART!");                                //Ifor 20200908 add:記錄SECS/GEM LOTSTART
            HCACK=0;
        }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G21
        else if(S.AnsiPos("AUTHORITY_CHECK")==1)
        {
            unsigned char  uint1EC;
            if(IniConfig.bN07_EnableEmployeeIdCheak==true)
            {
#if 0   // ===== GATE G22 -- golden SECSGEM/uHGemHT9045.cpp:2856-2859 =====
//   WHY GATED : (superseded -- see the 20260916 note below)
//               FormBarcodeReader has NO port object (nothing declares it).
//   ⚠ AI(W906-NL-ABSENCE) 20260916: that premise EXPIRED. FormBarcodeReader is
//               declared and defined -- BarcodeReader.cpp:40, with the class and
//               both members (bShow :102 / Close() :108) in BarcodeReader.h, and
//               CMakeLists.txt:2354 has compiled BarcodeReader.cpp since 20260824.
//               THE GATE STILL STANDS, for a different reason: the only ctor call
//               is InitForms() (FormsBootstrap.cpp:203), and FormsBootstrap.cpp:129
//               records that NOTHING CALLS InitForms() -- re-measured 20260916 with
//               `git grep InitForms`, still zero call sites. So the pointer is NULL
//               at run time and un-gating would dereference NULL.
//               The same stale premise is recorded at WebStart.cpp's
//               SAFETY-GATE(W906-ST-W2-A); both are corrected in the same commit.
//   DELTA     : If the barcode-reader form were up it stays up while the authority
//               check proceeds.  Offline there is no such form, so no observable
//               difference; on a real machine the dialog is not auto-closed.
                if(FormBarcodeReader->bShow)
                {
                    FormBarcodeReader->Close();
                }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G22

                // ===== GATE G23 OPENED 20260824 (FW-QWKEY4) -- golden :2861-2864 =====
                //   fPassword real since FW-QWKEY1 (fc08e09); Visible added to the
                //   facade for this exact site. Headless Visible stays false, so the
                //   auto-close branch is faithfully unreached until a real window
                //   subsystem raises it.
                if(fPassword->Visible==true)
                {
                    fPassword->Close();
                }

                // ===== GATE G24 OPENED 20260824 (FW-G24) -- golden :2866-2869 =====
                //   MyMessageBox reachable via mymessbox_shim.h (narrow seam); real
                //   non-NULL instance since acatchtray_shims.cpp:98. Offline fShow
                //   stays false, so the close branch is faithfully unreached.
                if(MyMessageBox->fShow && bSECSGEMAlarm==false)                 //Ifor 20180911 :Add 啟動 Employee ID Check
                {
                    MyMessageBox->Close();
                }

                if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)                                     //wei 20150630
                {
                    if(SVlen==0)
                    {
#if 0   // ===== GATE G25 -- golden SECSGEM/uHGemHT9045.cpp:2875 =====
//   WHY GATED : SECS_GEM_PPSIGNALTOWER_CONTROL_flag is extern-declared at
//               ckernel.cpp:1275 and DEFINED NOWHERE in the port (same hole as G16/G17).
//               golden uses the tower-light flag here as an unrelated scratch marker --
//               see GOLDEN BUG B4 in the banner.
//   DELTA     : HCACK=0 on :2876 still runs, so an empty AUTHORITY_CHECK list is still
//               acked.  Nothing in this port defines or reads that global, so the only
//               loss is the write itself.
                        SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G25
                        HCACK=0;
                    }
                    else
                    {
                        for(i=0; i<SVlen; i++)
                        {
                            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                            {
                                ActiveWire->GetDataItemLenAndType(len, Type);         //取得資料長度與格式
                                ActiveWire->DataItemIn(len, Type, str);               //取值
                                S1=str;
                                HCACK=0;
                                if(S1=="Action")
                                {
                                    if(ActiveWire->DataItemIn(1, HType.BINARY_TYPE, &uint1EC)==1)
                                    {
                                        ret=uint1EC;
                                    }
                                }
                                else if(S1=="Message")
                                {
                                    ActiveWire->GetDataItemLenAndType(len, Type);
                                    if(Type==HType.ASCII_TYPE)
                                    {
                                        CommandStr=new char [len+1];            //JerryYang 20241205 : 字串陣列大小改成動態
//                                        ZeroMemory(CommandStr, sizeof(CommandStr));
                                        memset(CommandStr, 0, len+1);           //JerryYang 20250325 : fix
                                        ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                                        str2=CommandStr;
                                        delete[] CommandStr;                    //Steven 20160912 : Add delete for save memory
                                        CommandStr=NULL;
                                    }
                                }
                                else
                                {
                                    HCACK=1;
                                }
                            }
                            else
                            {
                                HCACK=1;
                            }
                        }
                    }
                    bWaitSecsGemReply=false;                                    //Ifor 20180302 Time Out 判斷旗標
                    // ===== GATE G26 OPENED 20260824 (FW-G24) -- golden :2921 =====
                    //   Two-term OR restored; MyMessageBox via mymessbox_shim.h.
                    if(MyMessageBox->fShow || fNote->fShow)
                    {
                        if(uint1EC==0)
                        {
                            HCACK=0;
                            iShowAUTHORITY=2;                                   //Ifor 20180302 顯示判斷
                            bWaitSecsGemReply=true;
                        }
                        else if(uint1EC==1 || uint1EC==2)
                        {
                            iShowAUTHORITY=1;
                            bWaitSecsGemReply=true;
                            strShowAUTHORITY=str2;                              //JerryYang 20241205 : 字串陣列大小改成動態
                        }
                        else
                        {
                        }
                    }
                    else
                    {
                        iShowAUTHORITY=0;
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else
            {
                HCACK=0;
            }
        }
        else if(S=="SET_TEST_FLOW")                                             //JerryYang 20250120 : modify
        {
            if(HasICUnderMachine()==true || SystemStart==true)
            {
                HCACK=3;
            }
            else
            {
                if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen,HType.LIST_TYPE)==1)
                {
                    if(SVlen==0)
                    {
                        HCACK=2;
                    }
                    else
                    {
                        if(SVlen>=4)
                        {
                            for(i=0; i<SVlen; i++)
                            {
                                if( ActiveWire->DataItemIn(2,HType.LIST_TYPE,NULL)==1)
                                {
                                    ActiveWire->GetDataItemLenAndType(len,Type);      //取得資料長度與格式
                                    if(Type==HType.ASCII_TYPE)
                                    {
                                        ActiveWire->DataItemIn(len,Type,str);         //取值
                                        S1=str;

                                        if(S1=="AO")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S2);                                         //取值
                                                fLotInfo->edtASECL_LotID->Text=S2;
                                            }
                                            else
                                            {
                                                HCACK=2;
                                            }
                                        }
                                        else if(S1=="FLOW_ID")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S2);                                         //取值
                                                fLotInfo->edFlowID->Text=S2;
                                                HCACK=0;
                                            }
                                            else
                                            {
                                                HCACK=2;
                                            }
                                        }
                                        else if(S1=="INSERTION")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S2);                                         //取值
                                                fLotInfo->edInsertion->Text=S2;
                                                HCACK=0;
                                            }
                                            else
                                            {
                                                HCACK=2;
                                            }
                                        }
                                        else if(S1=="CUSTOMER_DEVICE")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S2);                                         //取值
#if 0   // ===== GATE G27 -- golden SECSGEM/uHGemHT9045.cpp:3029 =====
//   WHY GATED : forms/fLotInfo.h has no edCustomerDevice member (it does have
//               edtASECL_LotID/edInsertion/edFlowID, which the sibling CPNAMEs in this
//               same arm DO use -- :2988/:3001/:3015 are all live).
//   DELTA     : The CUSTOMER_DEVICE value from the host is dropped.  HCACK=0 still set
//               at :3030.
                                                fLotInfo->edCustomerDevice->Text=S2;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G27
                                                HCACK=0;
#if 0   // ===== GATE G28 -- golden SECSGEM/uHGemHT9045.cpp:3031-3032 =====
//   WHY GATED : fLotInfo->btnASECL_LotStartClick and fLotInfo->btnASECL_LotStart do not
//               exist in forms/fLotInfo.h.
//   DELTA     : THE ASE-CL LOT START DOES NOT HAPPEN, yet HCACK=0 was already set on
//               :3030 -- so the host is told SET_TEST_FLOW succeeded and no lot is
//               started.  This is the most misleading residue in this translation and
//               is called out here deliberately.  Setting HCACK to something else
//               instead was rejected: it would invent a reply golden never sends.
//               Whoever ports uLotInfo must un-gate this first.
                                                //JerryYang 20250411 : 這裡要按Lot start
                                                fLotInfo->btnASECL_LotStartClick(fLotInfo->btnASECL_LotStart);
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G28
                                            }
                                            else
                                            {
                                                HCACK=2;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                        }
                                    }
                                }
                                else
                                {
                                    HCACK=1;
                                }
                            }
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
        }
        else if(S.AnsiPos("SET_LOT_INFO")==1)                                   //JerryYang 20190409
        {
            if(HasICUnderMachine()==true || SystemStart==true)
            {
                if(IniConfig.bSPILFunction==true)
                {
                    if(IniConfig.bA37LotStartLotEnd==false &&
                       fSCKART->iCurrent93KARTStep!=8)                          //JerryYang 20220923 : 修改SPIL LOT INFO
                    {
                        HCACK=4;
                    }
                }
                else
                {
                    HCACK=4;
                }
            }

            if(HCACK!=4)
            {
                if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
                {
                    if(SVlen==0)
                    {
                        HCACK=2;
                    }
                    else
                    {
                        if(SVlen==2)                                            //JerryYang 20220923 : 修改SPIL LOT INFO
                        {
                            for(i=0; i<SVlen; i++)
                            {
                                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                                {
                                    ActiveWire->GetDataItemLenAndType(len, Type);     //取得資料長度與格式
                                    if(Type==HType.ASCII_TYPE)
                                    {
                                        ActiveWire->DataItemIn(len, Type, str);       //取值
                                        S1=str;
                                        if(i==0)
                                        {
                                            if(S1=="LOT_INFO")                  //JerryYang 20220923 : 修改SPIL LOT INFO
                                            {
                                                ActiveWire->GetDataItemLenAndType(len, Type);                                 //取得資料長度與格式
                                                if(Type==HType.ASCII_TYPE)
                                                {
                                                    ActiveWire->DataItemIn(len, Type, S2);                                    //取值
#if 0   // ===== GATE G29 -- golden SECSGEM/uHGemHT9045.cpp:3110 =====
//   WHY GATED : ProcessLotInfo(AnsiString) is forward-declared by golden itself at
//               :1144 (carried over above) but has NO definition anywhere in this port
//               -- golden's body is in BarcodeXML.cpp, which is untranslated (there is
//               no BarcodeXML.cpp in the port tree at all).  Calling it links to
//               nothing.
//   DELTA     : bHasFile is forced false, so golden's own next test (:3111) takes the
//               failure path and the host gets HCACK=3 "at least one parameter is
//               invalid" -- one of golden's own outcomes for this arm, and the honest
//               one: the lot-info payload was not parsed.  bReadLotInfoFromART is
//               therefore never set, so no downstream consumer acts on stale lot info.
                                                    bHasFile=ProcessLotInfo(S2);
#else
                                                    bHasFile=false;   // GATE G29 default -- see above
#endif  // GATE G29
                                                    if(bHasFile==false)
                                                    {
                                                        HCACK=3;
                                                    }
                                                    else
                                                    {
                                                        bReadLotInfoFromART=true;
                                                    }
                                                }
                                                else
                                                {
                                                    HCACK=7;
                                                }
                                            }
                                            else
                                            {
                                                HCACK=5;
                                            }
                                        }
                                        else if(i==1)
                                        {
                                            if(S1=="DISPLAY")
                                            {
                                                ActiveWire->GetDataItemLenAndType(len, Type);                                 //取得資料長度與格式
                                                if(Type==HType.ASCII_TYPE)
                                                {
                                                    ActiveWire->DataItemIn(len, Type, S3);                                    //取值
                                                    if(HCACK==1)
                                                        HCACK=0;
                                                }
                                                else
                                                {
                                                    HCACK=6;
                                                }
                                            }
                                            else
                                            {
                                                HCACK=2;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    HCACK=8;
                                }
                            }
                        }
                        else
                        {
                            HCACK=9;
                        }
                    }
                }
                else
                {
                    HCACK=10;
                }
            }
        }
        else if(S.AnsiPos("SET_BUNDLE_INFO")==1)                                //JerryYang 20240321 : add
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
            {
                if(SVlen==3 || SVlen==2)
                {
                    for(i=0; i<SVlen; i++)
                    {
                        if(ActiveWire->DataItemIn(2,HType.LIST_TYPE,NULL)==1)
                        {
                            ActiveWire->GetDataItemLenAndType(len,Type);              //取得資料長度與格式
                            if(Type==HType.ASCII_TYPE)
                            {
                                ActiveWire->DataItemIn(len,Type,str);                 //取值
                                S1=str;
                                if(i==0)
                                {
                                    if(S1=="BUNDLE_LIST")
                                    {
                                        ActiveWire->GetDataItemLenAndType(len, Type);                                         //取得資料長度與格式
                                        if(Type==HType.ASCII_TYPE)
                                        {
                                            ActiveWire->DataItemIn(len,Type, S2);     //取值
                                            slBundlID->Clear();
                                            slBundlID->CommaText=S2;

                                            fSCKART->sBundleList=slBundlID->CommaText;
#if 0   // ===== GATE G30 -- golden SECSGEM/uHGemHT9045.cpp:3198 =====
//   WHY GATED : fObserver (TfObserverShim, atester_shims.h) has no labBundleID.
//   DELTA     : Bundle list IS stored (:3197 runs); label not painted.
                                            fObserver->labBundleID->Caption=fSCKART->sBundleList;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G30
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        HCACK=2;
                                        break;
                                    }
                                }
                                else if(i==1)
                                {
                                    if(S1=="BUNDLE_IN")
                                    {
                                        ActiveWire->GetDataItemLenAndType(len,Type);  //取得資料長度與格式
                                        if(Type==HType.ASCII_TYPE)
                                        {
                                            ActiveWire->DataItemIn(len,Type, S3);     //取值
                                            iBundleIn=StrToIntDef(S3, 0);

                                            fSCKART->iBundleInCnt=iBundleIn;
#if 0   // ===== GATE G31 -- golden SECSGEM/uHGemHT9045.cpp:3223 =====
//   WHY GATED : fObserver has no labBundlIn (golden's own spelling).
//   DELTA     : iBundleIn / fSCKART->iBundleInCnt ARE set (:3220/:3222); label not
//               painted.
                                            fObserver->labBundlIn->Caption=fSCKART->iBundleInCnt;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G31

                                            if(iBundleIn>2)
                                            {
                                                HCACK=0;
                                            }
                                            else
                                            {
                                                iBundleIn=12;
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else if(S1=="BUNDLE_OUT")
                                    {
                                        ActiveWire->GetDataItemLenAndType(len,Type);  //取得資料長度與格式
                                        if(Type==HType.ASCII_TYPE)
                                        {
                                            ActiveWire->DataItemIn(len,Type, S3);     //取值
                                            iBundleOut=StrToIntDef(S3, 0);
                                            fSCKART->iBundleOutCnt=iBundleOut;
                                            if(iBundleOut>2)
                                            {
#if 0   // ===== GATE G32 -- golden SECSGEM/uHGemHT9045.cpp:3252 =====
//   WHY GATED : fObserver has no labBundOut (golden's own spelling).
//   DELTA     : HCACK=0 on :3253 still runs; iBundleOut / fSCKART->iBundleOutCnt ARE
//               set (:3248/:3249); label not painted.
                                                fObserver->labBundOut->Caption=fSCKART->iBundleOutCnt;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G32
                                                HCACK=0;
                                            }
                                            else
                                            {
                                                HCACK=1;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        HCACK=2;
                                        break;
                                    }
                                }
                                else if(i==2)
                                {
                                    if(S1=="BUNDLE_OUT")
                                    {
                                        ActiveWire->GetDataItemLenAndType(len,Type);  //取得資料長度與格式
                                        if(Type==HType.ASCII_TYPE)
                                        {
                                            ActiveWire->DataItemIn(len,Type, S3);     //取值
                                            iBundleOut=StrToIntDef(S3, 0);
                                            fSCKART->iBundleOutCnt=iBundleOut;
                                            if(iBundleOut>2)
                                            {
#if 0   // ===== GATE G33 -- golden SECSGEM/uHGemHT9045.cpp:3285 =====
//   WHY GATED : fObserver has no labBundOut.  Second occurrence (the i==2 BUNDLE_OUT
//               branch); golden writes the raw iBundleOut here instead of the member,
//               which is kept verbatim.
//   DELTA     : HCACK=0 on :3286 still runs; label not painted.
                                                fObserver->labBundOut->Caption=iBundleOut;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G33
                                                HCACK=0;
                                            }
                                            else
                                            {
                                                iBundleOut=12;
                                                HCACK=1;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        HCACK=2;
                                        break;
                                    }
                                }
                                fSCKART->AccessFile(false, 1);
                            }
                        }
                        else
                        {
                            HCACK=1;
                            break;
                        }
                    }
                }
                else
                {
                    HCACK=2;
                }
            }
        }
        else if(S.AnsiPos("DEVTEMPOFFSETADJUST")==1)                            //JerryYang 20190812 For Qualcomm 溫度offset funciton
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
            {
                if(SVlen==0)
                {
                    HCACK=2;
                }
                else
                {
                    if(SVlen==3)
                    {
                        for(i=0; i<SVlen; i++)
                        {
                            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE,NULL)==1)
                            {
                                ActiveWire->GetDataItemLenAndType(len, Type);         //取得資料長度與格式
                                if(Type==HType.ASCII_TYPE)
                                {
                                    ActiveWire->DataItemIn(len,Type,str);             //取值
                                    S1=str;
                                    if(i==0)
                                    {
                                        if(S1=="INDEX_ARM")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len, Type);                                     //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S2);                                         //取值
                                                if(S2!="F" && S2!="R")
                                                {
                                                    HCACK=2;
                                                    break;
                                                }
                                                else
                                                {
                                                    if(S2=="F")
                                                    {
                                                        iArm=1;
                                                    }
                                                    else
                                                    {
                                                        iArm=2;
                                                    }
                                                    HCACK=0;
                                                }
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else if(i==1)
                                    {
                                        if(S1=="THERMAL_HEAD")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len,Type);                                      //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S3);                                         //取值
                                                iHead=StrToIntDef(S3, 0);
                                                if(iHead>0 && iHead<=16)
                                                {
                                                    HCACK=0;
                                                }
                                                else
                                                {
                                                    HCACK=2;
                                                    break;
                                                }
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                    else if(i==2)
                                    {
                                        if(S1=="TEMP_OFFSET")
                                        {
                                            ActiveWire->GetDataItemLenAndType(len, Type);                                     //取得資料長度與格式
                                            if(Type==HType.ASCII_TYPE)
                                            {
                                                ActiveWire->DataItemIn(len,Type, S3);                                         //取值
                                                dOffset=StrToFloatDef(S3, 0.0);
#if 0   // ===== GATE G34 -- golden SECSGEM/uHGemHT9045.cpp:3422-3431 =====
//   WHY GATED : (superseded -- see the 20260916 note)
//               fTemp_Set has NO port object -- MyTempPanel.h mentions a much smaller
//               `fTemp_Set` stand-in in prose only; no header declares the pointer, and
//               SaveRemoteTempOffset / ReadRemoteTempOffset exist nowhere in the tree.
//   ⚠ AI(W906-NL-ABSENCE) 20260916: EVERY CLAUSE of that premise EXPIRED:
//               forms/fTemp_Set.h:1563  extern TfTemp_Set *fTemp_Set;
//               forms/fTemp_Set.h:529   int  SaveRemoteTempOffset(int,int,double);
//               forms/fTemp_Set.h:531   void ReadRemoteTempOffset();
//               nm --defined-only over build/*.a finds `_fTemp_Set`.
//               ⛔ THE GATE STAYS SHUT AND THE NIGHT LOOP WILL NOT OPEN IT.
//               This writes a TEMPERATURE OFFSET arriving from a remote SECS host
//               -- runtime setting + outward-facing interface, i.e. exactly the
//               class the loop is forbidden to arm unattended. Note forms/
//               fTemp_Set.h:259 marks SaveRemoteTempOffset itself as SAFETY GATE
//               (S3), so opening G34 would also have to answer what (S3) leaves
//               gated. Queued for the user.
//   DELTA     : The port takes golden's OWN failure branch (:3427-:3431 verbatim):
//               HCACK=1 and break out of the CPNAME loop.  So a Qualcomm-style host
//               temperature-offset write is refused rather than silently accepted.  The
//               per-head offset file is NOT updated; the machine keeps the offsets it
//               already had.
                                                if(fTemp_Set->SaveRemoteTempOffset(iArm, iHead, dOffset)==0)
                                                {
                                                    fTemp_Set->ReadRemoteTempOffset();
                                                    HCACK=0;
                                                }
                                                else
                                                {
                                                    HCACK=1;
                                                    break;
                                                }
#else
                                                HCACK=1;   // GATE G34 default -- golden's own
                                                break;     // SaveRemoteTempOffset!=0 arm (:3429-3430)
#endif  // GATE G34
                                            }
                                            else
                                            {
                                                HCACK=2;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            HCACK=2;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                HCACK=1;
                                break;
                            }
                        }
                    }
                    else
                    {
                        HCACK=1;
                    }
                }
            }
            else
            {
                HCACK=1;
            }
        }
        else if(S.AnsiPos("CLEAR_LOT_INFO")==1)                                 //JerryYang 20200330 : 修改SPIL LOT INFO
        {
            if(SystemStart==false)
            {
                fSCKART->iWaitGPIBLotR=0;
                ZeroMemory(iAutoTrayCount, sizeof(iAutoTrayCount));             //Sam 20191113 : TCP ART
                if(HasICUnderMachine()==false)
                {
                    fMain->Clarn_Data(1, "ART_LOTCLEARED");
                    fSCKART->ClearLotInfo();
                    RecordProcess("ART LOTCLEARED.");
                    fLotInfo->btClearBarcodeList->Click();                      //Steven 20190214 : 統一清除2DID方式
                    slDupBundlID->Clear();
                    slDupBundlID->SaveToFile(asDupBundleID);
                    slDupUnloadBundlID->Clear();
                    slDupUnloadBundlID->SaveToFile(aslDupUnloadBundlID);
                    HCACK=0;
                }
                else
                {
                    HCACK=2;
                }
            }
            else
            {
                HCACK=1;
            }
        }
        else if(S.AnsiPos("LOTORDER")==1)                                       //JerryYang 20200330 : 修改SPIL LOT ORDER
        {
            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
            {
                ActiveWire->GetDataItemLenAndType(len, Type);
                CommandStr=new char [len+1];                                    //JerryYang 20241205 : 字串陣列大小改成動態
//                ZeroMemory(CommandStr, sizeof(CommandStr));
                memset(CommandStr, 0, len+1);                                   //JerryYang 20250325 : fix
                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                if(ret==-1)
                    S2="";
                S2=CommandStr;
                delete[] CommandStr;                                            //Steven 20160912 : Add delete for save memory
                CommandStr=NULL;

                if(S2=="ORDER")
                {
                    ActiveWire->GetDataItemLenAndType(len, Type);
                    CommandStr=new char [len+1];                                //JerryYang 20241205 : 字串陣列大小改成動態
//                    ZeroMemory(CommandStr, sizeof(CommandStr));
                    memset(CommandStr, 0, len+1);                               //JerryYang 20250325 : fix
                    ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                    if(ret==-1)
                        S3="";
                    S3=CommandStr;
                    delete[] CommandStr;                                        //Steven 20160912 : Add delete for save memory
                    CommandStr=NULL;
                    // AI(W906-PT-W5-S2F42) 20260809: PORT-MECHANICAL (rename 3 in the
                    // banner).  golden :3520 is `if(S3==0 || S3==1 || S3==2 || S3==3)`.
                    // In BCB6 that compares S3 against "0"/"1"/"2"/"3" (the AnsiString(int)
                    // converting ctor is an exact match for the literal).  In this port the
                    // literal would instead bind operator==(const AnsiString&, const char*)
                    // with a null pointer and compare against "" four times, so the explicit
                    // AnsiString(...) is what KEEPS golden's behaviour.  golden's own oddity
                    // (it re-tests with S3.Pos("0") on the very next lines) is KEPT.
                    if(S3==AnsiString(0) || S3==AnsiString(1) || S3==AnsiString(2) || S3==AnsiString(3))
                    {
                        HCACK=0;
                        if(S3.Pos("0")!=0 || S3.Pos("1")!=0)
                        {
                            if(fSCKART->iFTRTCount==0)
                            {
                                fSCKART->iCurrent93KARTStep=3;
                            }
                            else
                            {
                                fSCKART->iCurrent93KARTStep=9;
                            }
                            LastSet.bWaitStartLotAutoRetestGPIB=true;           //SECS GEM ART
                        }
                        else if(S3.Pos("2")!=0 || S3.Pos("3")!=0)
                        {
                            if(fSCKART->iCurrent93KARTStep>=10)
                                fSCKART->iCurrent93KARTStep=12;
                            LastSet.bWaitEndLotAutoRetestGPIB=true;
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                    else
                    {
                        HCACK=1;
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else
            {
                HCACK=1;
            }
        }
        else if(S.AnsiPos("TRAY_MAP")==1)                                       //KenHsieh 20220912 : add AOSH 1600LT Tray map CCD Command
        {
            HCACK=0;
        }
        else if(S.AnsiPos("SET_2DID_BIN_CODE")==1)                              //KenHsieh 20220912 : add AOSH 1600LT Tray map CCD Command
        {
            if(HasICUnderMachine()==true || SystemStart==true)
            {
                HCACK=4;
            }
            else
            {
                sTotalLotID="";

                ActiveWire->GetDataItemLenAndType(len, Type);
                if(Type==HType.ASCII_TYPE)
                {
                    CommandStr=new char [len+1];                                //JerryYang 20241205 : 字串陣列大小改成動態
//                    ZeroMemory(CommandStr, sizeof(CommandStr));
                    memset(CommandStr, 0, len+1);                               //JerryYang 20250325 : fix
                    ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                    if(ret==-1)
                    {
                        HCACK=3;
                    }
                    else
                    {
#if 0   // ===== GATE G35 -- golden SECSGEM/uHGemHT9045.cpp:3588 =====
//   WHY GATED : S2F42_SET_2DID_BIN_CODE(AnsiString) is forward-declared by golden at
//               :1145 (carried over above) but has NO definition anywhere in the port
//               (grep over *.cpp/*.h finds only the declaration).  Calling it links to
//               nothing.
//   DELTA     : HCACK=3 -- exactly what golden itself returns two lines up (:3584) when
//               the payload could not be read.  The 2DID bin-code table from the host
//               is discarded, so a 2D-sort recipe pushed this way never takes effect;
//               sTotalLotID has already been cleared at :3573, matching golden.
                        HCACK=S2F42_SET_2DID_BIN_CODE(CommandStr);              //Steven 20241008 : S2F42的XML換位置
#else
                        HCACK=3;   // GATE G35 default -- golden's own ret==-1 value (:3584)
#endif  // GATE G35
                    }
                    delete[] CommandStr;                                        //Steven 20160912 : Add delete for save memory
                    CommandStr=NULL;
                }
                else
                {
                    HCACK=3;
                }
            }
        }
        else if(S=="START_LOT")                                                 //Ifor 20221019 add: Onsemi START_LOT
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE)==1)
            {
                if(SVlen==0)
                {
#if 0   // ===== GATE G36 -- golden SECSGEM/uHGemHT9045.cpp:3605 =====
//   WHY GATED : SECS_GEM_PPSIGNALTOWER_CONTROL_flag defined nowhere (see GATE G25).
//               Third of the three unrelated scratch uses -- GOLDEN BUG B4.
//   DELTA     : HCACK=0 on :3606 still runs, so an empty START_LOT list is still acked.
//               Only the write to the undefined global is lost.
                    SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G36
                    HCACK=0;
                }
                else
                {
                    for(i=0; i<SVlen; i++)
                    {
                        if(ActiveWire->DataItemIn(2, HType.LIST_TYPE,NULL)==1)
                        {
                            ActiveWire->GetDataItemLenAndType(len,Type);              //取得資料長度與格式
                            ActiveWire->DataItemIn( len,Type,str);                    //取值

                            ActiveWire->GetDataItemLenAndType(len, Type);
                            if(Type==HType.ASCII_TYPE)
                            {
                                ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, strLotID);
                                S1=str;
                                HCACK=0;
                                if(S1=="LOTID")
                                {
                                    fLotInfo->edtSysLotID->Text=strLotID;
                                }
                                else if(S1=="DEVICEID")
                                {
                                    fLotInfo->edtDevice->Text=strLotID;
                                }
                                else if(S1=="OPERATORID")
                                {
                                    fLotInfo->edtSysOperatorID->Text=strLotID;
                                }
                                else if(S1=="RUN_MODE")                         //RogerYang 20260608 : Add for VTEST SECS (LotID、OperatorID、Mode(FT0-FT15、RT0-RT5))
                                {
                                    if(IniConfig.bVTESTFunction==true)
                                    {
                                        if(fLotInfo->cbRunMode->Items->IndexOf(strLotID)>=0)
                                        {
                                            fLotInfo->cbRunMode->Text=strLotID;
                                        }
                                        else
                                        {
                                            AnsiString sLog;
                                            sLog.sprintf("[S2F42] START_LOT RUN_MODE value '%s' not in list", strLotID);
                                            ActiveWire->StringOut(sLog);
                                            HCACK=3;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        fLotInfo->cbRunMode->Text=strLotID;
                                    }
                                }
#if 0   // ===== GATE G37 -- golden SECSGEM/uHGemHT9045.cpp:3657-3671 =====
//   WHY GATED : forms/fLotInfo.h has no cbTestTimes member (it does have cbRunMode,
//               which the sibling RUN_MODE branch at :3635-:3656 uses live).
//   DELTA     : TEST_TIMES falls through to the arm's own final else (:3672-3675) which
//               sets HCACK=1, so a VTEST host that sends TEST_TIMES gets a rejection
//               for the whole START_LOT.  LOTID / DEVICEID / OPERATORID / RUN_MODE are
//               unaffected and still applied.
                                else if(S1=="TEST_TIMES")                       //RogerYang 20260608 : Add for VTEST SECS (LotID、OperatorID、Mode(FT0-FT15、RT0-RT5))
                                {
                                    if(fLotInfo->cbTestTimes->Items->IndexOf(strLotID)>=0)
                                    {
                                        fLotInfo->cbTestTimes->Text=strLotID;
                                    }
                                    else
                                    {
                                        AnsiString sLog;
                                        sLog.sprintf("[S2F42] START_LOT TEST_TIMES value '%s' not in list", strLotID);
                                        HGemPtr->StringOut(sLog);
                                        HCACK=3;
                                        break;
                                    }
                                }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G37
                                else
                                {
                                    HCACK=1;
                                }
                            }
                            else
                            {
                                HCACK=1;
                            }
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }

#if 0   // ===== GATE G38 -- golden SECSGEM/uHGemHT9045.cpp:3688-3689 =====
//   WHY GATED : fLotInfo->sbSECSLotStartClick does not exist (same absence as GATE
//               G01/G21).
//   DELTA     : The lot-info edits ARE written, and HCACK stays 0, but NO lot start is
//               performed -- the host is told START_LOT succeeded while the machine
//               never started the lot.  Same misleading shape as GATE G28 and for the
//               same reason (inventing a different HCACK would invent a reply).
                    if(HCACK==0)
                        fLotInfo->sbSECSLotStartClick(fLotInfo);
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G38
                }
            }
            else
            {
                HCACK=1;
            }
        }
        //==> Eastsun 20260520 整合
        else if(S.AnsiPos("START_AQL")==1)                                      //Ifor 20240311 "START_AQL"移至"START"上方避免直接進入"START"命令
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen,HType.LIST_TYPE)==1)  //wei 20150630
            {
                if(HasICUnderMachine()==false && IniConfig.bI52_bAQLSortMode==true)
                {
                    if(SVlen==0)
                    {
                        HCACK=1;
                    }
                    else
                    {
                        for(i=0;i<SVlen;i++)
                        {
                            if( ActiveWire->DataItemIn( 2,HType.LIST_TYPE,NULL)==1)
                            {
                                ActiveWire->GetDataItemLenAndType(len,Type);//取得資料長度與格式
                                ActiveWire->DataItemIn( len,Type,str); //取值

                                ActiveWire->GetDataItemLenAndType(len,Type);//取得資料長度與格式
                                ActiveWire->DataItemIn( len,Type,&ret); //取值
                                S1=str;
//                                HCACK=0;          //判斷後再回覆
                                if(S1=="COUNT")
                                    iAQLCount=ret;
                                else if(S1=="BIN")
                                    iAQLBin=ret;
                            }
                            else
                            {
                                HCACK=1;
                            }
                        }

#if 0   // ===== GATE G39 -- golden SECSGEM/uHGemHT9045.cpp:3732-3739 =====
//   WHY GATED : forms/fLotInfo.h has no SetAQLMode member.
//   DELTA     : The port takes golden's OWN failure value: HCACK=1.  So START_AQL is
//               refused.  NOTE the residue: iAQLCount / iAQLBin have ALREADY been
//               written by the loop above (:3722/:3724) before this point, exactly as in
//               golden when SetAQLMode fails -- the globals are set but AQL sort mode is
//               never entered.
                        if(fLotInfo->SetAQLMode(true)==false)
                        {
                             HCACK=1;
                        }
                        else
                        {
                            HCACK=0;
                        }
#else
                        HCACK=1;   // GATE G39 default -- golden's own SetAQLMode==false arm (:3734)
#endif  // GATE G39
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else
            {
                HCACK=1;
            }
        }
        //<== Eastsun 20260520 整合
        //==> Eastsun 20260520 整合
        else if(S.AnsiPos("START_AGV")==1)
        {
            if(ActiveWire->GetDataItemLenAndTypeAndDelete(SVlen,HType.LIST_TYPE)==1)    //wei 20150630
            {
                if(SVlen==0)
                {
#if 0   // ===== GATE G40 -- golden SECSGEM/uHGemHT9045.cpp:3760 =====
//   WHY GATED : SECS_GEM_PPSIGNALTOWER_CONTROL_flag defined nowhere (see GATE G25).
//   DELTA     : HCACK=0 on :3761 still runs, so an empty START_AGV list is still acked.
//               Only the write to the undefined global is lost.
                    SECS_GEM_PPSIGNALTOWER_CONTROL_flag=false;
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G40
                    HCACK=0;
                }
                else
                {
                    for(i=0;i<SVlen;i++)
                    {
                        if(ActiveWire->DataItemIn(2,HType.LIST_TYPE,NULL)==1)
                        {
                            ActiveWire->GetDataItemLenAndType(len,Type);//取得資料長度與格式
                            ActiveWire->DataItemIn(len,Type,S1); //取值

                            ActiveWire->GetDataItemLenAndType(len,Type);//取得資料長度與格式
                            ActiveWire->DataItemIn(len,Type,S2); //取值

                            if(S1=="Loader" && S2=="Action")
                            {
                                bLoaderSECSActionFlag[0]=true;
#if 0   // ===== GATE G41 -- golden SECSGEM/uHGemHT9045.cpp:3778 =====
//   WHY GATED : forms/fLotInfo.h has no InitialLoaderTask member (it DOES have
//               InitialUnLoaderTask, :43, which the AUTO1/2/3 branches below use live).
//   DELTA     : bLoaderSECSActionFlag[0] IS set (:3777), but the Loader task is not
//               initialised, so the AGV Loader action is half-armed: the flag says a
//               host action is pending and no task was started.  HCACK=0 is still
//               returned at :3810.
                                fLotInfo->InitialLoaderTask(0);
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G41
                            }
                            else if(S1=="LoaderTrayCount")
                            {
                                iSECSSetTrayCount=atoi(S2.c_str());
                            }
                            else if(S1=="Empty" && S2=="Action")
                            {
                                bLoaderSECSActionFlag[1]=true;
#if 0   // ===== GATE G42 -- golden SECSGEM/uHGemHT9045.cpp:3787 =====
//   WHY GATED : fLotInfo->InitialLoaderTask absent (see GATE G41).
//   DELTA     : bLoaderSECSActionFlag[1] (Empty) set, task not initialised.
                                fLotInfo->InitialLoaderTask(1);
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G42
                            }
                            else if(S1=="Color" && S2=="Action")
                            {
                                bLoaderSECSActionFlag[2]=true;
#if 0   // ===== GATE G43 -- golden SECSGEM/uHGemHT9045.cpp:3792 =====
//   WHY GATED : fLotInfo->InitialLoaderTask absent (see GATE G41).
//   DELTA     : bLoaderSECSActionFlag[2] (Color) set, task not initialised.
                                fLotInfo->InitialLoaderTask(2);
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G43
                            }
                            else if(S1=="AUTO1" && S2=="Action")
                            {
                                bUnLoaderSECSActionFlag[0]=true;
                                fLotInfo->InitialUnLoaderTask(0);
                            }
                            else if(S1=="AUTO2" && S2=="Action")
                            {
                                bUnLoaderSECSActionFlag[1]=true;
                                fLotInfo->InitialUnLoaderTask(1);
                            }
                            else if(S1=="AUTO3" && S2=="Action")
                            {
                                bUnLoaderSECSActionFlag[2]=true;
                                fLotInfo->InitialUnLoaderTask(2);
                            }

                            HCACK=0;
                        }
                        else
                        {
                            HCACK=1;
                        }
                    }
                }
            }
            else
            {
                HCACK=1;
            }
        }
        //<== Eastsun 20260520 整合
        else if(S=="STOP_LOT")                                                  //Ifor 20221019 add: Onsemi STOP_LOT
        {
            if(ActiveWire->DataItemIn(1, HType.LIST_TYPE, NULL)==1)
            {
                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
                {
                    ActiveWire->GetDataItemLenAndType(len,Type);                      //取得資料長度與格式
                    ActiveWire->DataItemIn( len,Type,str);                            //取值
                    S1=str;
                    HCACK=0;
                    if(S1=="LOTID")
                    {
                        ActiveWire->GetDataItemLenAndType(len, Type);
                        if(Type==HType.ASCII_TYPE)
                        {
                            ret=ActiveWire->DataItemIn(len, HType.ASCII_TYPE, strLotID);
                            if(ret==-1)
                                strLotID="";
                        }
                        else
                        {
                            strLotID="";
                        }
                    }
                    else
                    {
                        strLotID="";
                    }

                    sLotNo=fLotInfo->edtSysLotID->Text;
                    if(sLotNo.Pos(strLotID)>=1)
                    {
                        fMain->CleanOut("S2F42 2");
                        HCACK=0;
                    }
                    else
                    {
                        HCACK=1;
                    }
                }
                else
                {
                    HCACK=1;
                }
            }
            else
            {
                HCACK=1;
            }
        }
        else if(S.AnsiPos("SKIP")==1)
        {
#if 0   // ===== GATE G44 -- golden SECSGEM/uHGemHT9045.cpp:3877-3881 =====
//   WHY GATED : fNote->ReturnCode / fNote->Close() absent (see GATE G10).
//   DELTA     : HCACK=0 still returned at :3882; a displayed alarm dialog is not
//               dismissed with K_SKIP.
            if(fNote->fShow)
            {
                fNote->ReturnCode=K_SKIP;
                fNote->Close();
            }
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G44
            HCACK=0;                                                        //Steven 20260415 : align with RETRY
        }
        else if(S.AnsiPos("TERMINAL_DISPLAY")==1)                               //Ifor 20251018 add:Analog 泰國客戶要求新增同1028 SECS GEM 命令關閉視窗
        {
            // ===== GATE G45 OPENED 20260824 (FW-G24) -- golden :3886-3889 =====
            //   TERMINAL_DISPLAY closes the message box again (the entire point of
            //   the command for the Analog/Thailand customer). MyMessageBox via
            //   mymessbox_shim.h; offline Visible stays false -> branch unreached.
            if(MyMessageBox->Visible==true)
            {
                MyMessageBox->Close();
            }
            HCACK=0;
        }
        // ======================================================================
        // SET_RECIPE — HCACK 完整示範 RCMD                         Ifor 20260423
        // S2F41 格式：
        //   <L2
        //     <A "SET_RECIPE">
        //     <L1
        //       <L2 <A "PPID"> <A "RecipeName">>
        //     >
        //   >
        // HCACK 回傳規則：
        //   0 = 執行完成（同步）
        //   1 = 指令不存在（不應到此，防禦性保留）
        //   2 = 目前狀態不允許（機台運行中且不支援非同步）
        //   3 = 參數不合法（PPID 格式錯誤）
        //   4 = 接受，非同步執行（機台運行中，待 Idle 後切換，S6F11 通知）
        //   5 = 已在目標狀態（當前工作檔與要求相同）
        //   6 = 找不到物件（PPID 不在工作檔清單中）
        // ======================================================================
        else if(S.AnsiPos("SET_RECIPE")==1)                                     //Ifor 20260423 : HCACK 0-6 完整示範 RCMD
        {
            AnsiString sNewPPID = "";
            bool bParamValid    = false;
            bool bFoundInList   = false;

            // ---- 解析參數：讀取 CPNAME="PPID" / CPVAL="RecipeName" ----
            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL) == 1)
            {
                ActiveWire->GetDataItemLenAndType(len, Type);
                CommandStr = new char[len + 1];
                memset(CommandStr, 0, len + 1);
                ret = ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                S1 = CommandStr;
                delete[] CommandStr;
                CommandStr = NULL;

                if(S1 == "PPID")
                {
                    ActiveWire->GetDataItemLenAndType(len, Type);
                    if(Type == HType.ASCII_TYPE && len > 0 && len <= 256)       // 防禦：長度範圍合法
                    {
                        CommandStr = new char[len + 1];
                        memset(CommandStr, 0, len + 1);
                        ret = ActiveWire->DataItemIn(len, HType.ASCII_TYPE, CommandStr);
                        sNewPPID    = CommandStr;
                        bParamValid = (sNewPPID.Length() > 0);                  // 非空字串才合法
                        delete[] CommandStr;
                        CommandStr = NULL;
                    }
                    else
                    {
                        bParamValid = false;                                     // Type 或長度不合法
                    }
                }
            }

            if(!bParamValid)
            {
                // HCACK=3：CPNAME/CPVAL 不合法
                HCACK = 3;
            }
            else
            {
                // 確認 PPID 是否存在於工作檔清單
                for(int k = 0; k < fMain->cbSetupFileName->Items->Count; k++)
                {
                    if(sNewPPID == fMain->cbSetupFileName->Items->Strings[k])
                    {
                        bFoundInList = true;
                        break;
                    }
                }

                if(!bFoundInList)
                {
                    // HCACK=6：找不到指定的工作檔物件
                    HCACK = 6;
                }
                else if(sNewPPID == fMain->cbSetupFileName->Text)
                {
                    // HCACK=5：設備已在目標工作檔狀態，拒絕重複切換
                    HCACK = 5;
                }
                else if(SystemStart || HasICUnderMachine())
                {
                    // HCACK=4：接受指令，非同步執行（機台 Idle 後再切換）
                    // 儲存非同步指令，待 CheckAndExecuteAsyncRCMD() 執行後
                    // 透過 S6F11 CEID=SwitchSetupFile 通知主機
                    m_AsyncRCMD.bPending = true;
                    m_AsyncRCMD.sCommand = "SET_RECIPE";
                    m_AsyncRCMD.sParam   = sNewPPID;
                    HCACK = 4;
                    RecordProcess("[S2F42] SET_RECIPE accepted async, PPID=" + sNewPPID);
                }
                else
                {
                    // HCACK=0：同步執行，切換工作檔完成
                    fMain->cbSetupFileName->Text = sNewPPID;
                    fMain->cbSetupFileNameChange(fMain);
#if 0   // ===== GATE G46 -- golden SECSGEM/uHGemHT9045.cpp:3990 =====
//   WHY GATED : THGem in this port has no EventReport(unsigned,unsigned).  golden
//               declares it at uHGemEquipment.h:552 (`void __fastcall
//               EventReport(unsigned iDataID,unsigned iCeid)`); the port's THGem
//               (SECSGEM/uHGemEquipment.h class at :596) carries the
//               EnableDisableEventReport family but not EventReport itself.  There IS a
//               free `void EventReport(unsigned Ceid)` (SECSGEM/SecsEventReport.h:55)
//               but it is a DIFFERENT function with a different arity -- substituting it
//               would invent a message.
//   DELTA     : The setup file IS switched (:3988/:3989 run) and HCACK=0 is returned,
//               but the S6F11 CEID=15 SwitchSetupFile event report is NOT sent.  A host
//               that waits for that event to confirm the recipe change will wait
//               forever even though the change happened.  NOTE the asymmetry this
//               creates with the HCACK=4 async path just above, which IS fully active
//               (m_AsyncRCMD, uHGemHT9045.h:101): the queued switch is executed later by
//               CheckAndExecuteAsyncRCMD, whose own S6F11 notification is gated in its
//               own append block -- so NEITHER path notifies the host today.
                    HGemPtr->EventReport(1, SECS_EVENT.SwitchSetupFile);        // CEID=15 通知 Host
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G46
                    RecordProcess("[S2F42] SET_RECIPE done, PPID=" + sNewPPID);
                    HCACK = 0;
                }
            }
        }
        else if(S=="RECIPEVERIFYACK" || S=="RUNCHECKEND")                       //RogerYang 20260610 : XINYUN runcheck完成回覆
        {
            char strName[256]={0}, strResult[256]={0}, strReason[1024]={0};
            if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)                   // 外層 L2
            {
                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)               // 第一組 <A name><A result>
                {
                    ActiveWire->DataItemIn(256, HType.ASCII_TYPE, strName);           // name(丟棄)
                    ActiveWire->DataItemIn(256, HType.ASCII_TYPE, strResult);         // result 0/1
                }

                if(ActiveWire->DataItemIn(2, HType.LIST_TYPE, NULL)==1)               // 第二組 <A name><A reason>
                {
                    ActiveWire->DataItemIn(256, HType.ASCII_TYPE, strName);           // name(丟棄)
                    ActiveWire->DataItemIn(1024, HType.ASCII_TYPE, strReason);        // reason
                }
            }

            if(AnsiString(strResult).UpperCase()=="0")                          // 0=pass 1=fail
            {
                if(IniConfig.bEnable_SECS_GEM==true &&
                   ((IniConfig.bRCMDStart==true && bPhysicalStart==true) ||
                    bSecsGemCanStart))                                          // 與 START case 完全一致
                {
                    SoftStart=true;
                    bSecsGemCanStart=false;
                    bSECSGEMAlarm=false;
                    bHasSaveSet=false;
                    RecordProcess("SECS/GEM RunCheck Pass, Start!");
                }
                HCACK=0;
            }
            else                                                                // fail
            {
                if(AnsiString(strReason)!="")
                    SecsAlarmMessage->Add("RunCheck Failed#" + AnsiString(strReason));
                RecordProcess("SECS/GEM RunCheck Fail");
                HCACK=0;
            }
            bPhysicalStart=false;
            bNeedDoRunCheck=false;
        }
        else if(S=="START")                                                     //RogerYang 20260610 : 改強制相等，與"PAUSE"都搬到下面     //Steven 20141016 : Add RCMD START
        {
            if(CUSTOMER_CODE==CC_KYEC_LEE && CosFunction.bUseSECSGEMAutoStart==true) //Eastsun 20260515 F019 AMR SECS/GEM Auto Start
            {
                if(TrayForm.bEnableAMR)
                {
                    if(RunInfo.bLotStart==true)
                    {
                        bSameSetupFileNoDownload=true;
                        RecordProcess("SECS/GEM Auto Start!");
                        bAMRReceiveStart=true;
#if 0   // ===== GATE G47 -- golden SECSGEM/uHGemHT9045.cpp:4049 =====
//   WHY GATED : forms/fLotInfo.h has no CheckActionFlag member.
//   DELTA     : For CC_KYEC_LEE with SECS/GEM auto-start and AMR enabled,
//               bAMRReceiveStart IS set (:4048) but the action-flag dispatch that would
//               consume it never runs, so the AMR start stalls at "received".  HCACK=0
//               is still returned at :4064.  The non-AMR path (:4052-4056,
//               fMain->Start) is unaffected.
                        fLotInfo->CheckActionFlag();
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G47
                    }
                }
                else
                {
                    RecordProcess("SECS/GEM Auto Start!");
                    fMain->Start("SECS GEM RCMD : START Auto Start");
                }

                if(IniConfig.bI52_bAQLSortMode==true && bSetAQLSortMode==true)
                {
                    bStartAQLSortMode=true;
                    bSetAQLSortMode=false;
                }
                bSecsGemCanStart=false;
                HCACK=0;
                bSECSGEMAlarm=false;
                bHasSaveSet=false;
            }
            else if(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND)
            {
                if(SystemStart==false)
                {
                    HCACK=0;
                    fMain->Start("SECS GEM RCMD : START");
                }
                else
                {
                    HCACK=1;
                }
            }
            else if(IniConfig.bEnable_SECS_GEM==true &&
                    CosFunction.bCanRemoteStart)                                //Steven 20250712 : 有通過風險告知可以遠端start的客戶
            {
                HCACK=0;
                bSECSGEMAlarm=false;                                            //Ifor 20151208 :解除按鍵Lock
                bHasSaveSet=false;                                              //Ifor 20151208 :清除設定檔變更旗標
                fMain->Start("SECS GEM RCMD : START");
            }
            else if(IniConfig.bEnable_SECS_GEM==true &&                         //Steven 20141006 : SECS GEM使用Remote Start功能
                    ((IniConfig.bRCMDStart==true && bPhysicalStart==true) ||
                    bSecsGemCanStart))                                          //wei 20150630  SecsGem Can Start
            {
                SoftStart=true;
                RecordProcess("SECS GEM RCMD : RUN CHECK START");
                bSecsGemCanStart=false;
                HCACK=0;
                bSECSGEMAlarm=false;                                            //Ifor 20151208 :解除按鍵Lock
                bHasSaveSet=false;                                              //Ifor 20151208 :清除設定檔變更旗標
            }
            else
            {
                HCACK=1;
            }
            bPhysicalStart=false;
            bNeedDoRunCheck=false;                                              //JerryYang 20250120 : modify
        }
        else if(S.AnsiPos("PAUSE")==1 || S=="STOP")                             //Steven 20221218 : S2F41字串判斷從原本的S=="" 改成 S.AnsiPos()==1
        {
            if(CUSTOMER_CODE==CC_MAXIM || CUSTOMER_CODE==CC_MAXIM_THAILAND)
            {
                if(fNote->fShow)
                {
#if 0   // ===== GATE G48 -- golden SECSGEM/uHGemHT9045.cpp:4112-4113 =====
//   WHY GATED : forms/fNote.h has neither BtnPauseClick nor Close().
//   DELTA     : For CC_MAXIM / CC_MAXIM_THAILAND with an alarm dialog up, the dialog is
//               not pause-clicked and closed.  fMain->BtnPauseClick(fMain) on :4116 --
//               outside this gate -- still runs, so the pause itself is unaffected, and
//               HCACK=0 is still returned.
                    fNote->BtnPauseClick(fNote);
                    fNote->Close();
#else
        // (gated -- no port substitute; see WHY/DELTA above)
#endif  // GATE G48
                }

                fMain->BtnPauseClick(fMain);
                HCACK=0;
            }
            else
            {
                fMain->BtnPauseClick(fMain);
                if(IniConfig.bEnable_SECS_GEM==true &&
                   IniConfig.bRCMDStart==true &&
                   bPhysicalStart==true)                                        //Wei 20150815 : Fixed for  RCMD Start
                {
                    bPhysicalStart=false;
                }

                if(CUSTOMER_CODE==CC_ASE_CL)                                    //RogerYang 20260413 Add
                {
                    bSECSPause=true;                                            //JerryYang 20250120 : modify
                }

                if(CosFunction.RunCheckWhenRecPause )                           //JerryYang 20250120 : modify
                {
                    bNeedDoRunCheck=true;
                }

                if(SystemStart==false)
                {
                    if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)                      //KenHsieh 20220913 : ASEKH K1要求回傳2
                        HCACK=2;
                    else if(IniConfig.bVTESTFunction)                           //RogerYang 20260608 : Add for VTEST SECS
                        HCACK=5;                                                //5 = Rejected, Already inDesired Condition
                    else
                        HCACK=1;
                }
                else
                {
                    HCACK=0;
                }
            }
        }
        // ======================================================================
        else                                                                    //20140124 wei
        {
            HCACK=1;
        }

//        delete[] CommandStr;                                                  //JerryYang 20241205 : 字串陣列大小改成動態
//        CommandStr=NULL;

        ActiveWire->InitLocalHead(2, 42, 0);

//        if(HCACK==0)
//        {
//            HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
//            HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
//            HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
//        }
//        else
//        {
            ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
            ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
            ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
//        }

        ActiveWire->SendLocalData();
        return 1;
    }
    else
    {
        HCACK=3;
        ActiveWire->DataItemOut(2, HType.LIST_TYPE, NULL);
        ActiveWire->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
        ActiveWire->DataItemOut(0, HType.LIST_TYPE, NULL);
    }
    return HCACK;
}
//------------------------------------------------------------------------------
//  END of the PT-W5-S2F42 append block.
//------------------------------------------------------------------------------
