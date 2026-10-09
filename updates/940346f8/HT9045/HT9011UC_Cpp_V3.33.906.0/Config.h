#ifndef ConfigH
#define ConfigH
#include "vclcompat/vcl_compat.h"   //AI(W0-TAIL) 20260626: AnsiString-by-value fields (was <vcl.h> via includer)
#include "MachineType.h"             //AI(W0-TAIL) 20260626: tcTotalCount/eTrayCount/eCL_Total/MAX_IONFAN array sizes (was via includer)

//---------------------------------------------------------------------------
typedef struct
{
    //這是使用網路硬碟做工作檔下載---------------------
    bool bEnableRms;                                                            //是否使用RMS
    AnsiString sRmsPath;                                                        //RMS的位置
    AnsiString sRmsDownPath;                                                    //RMS download的位置
    bool bEnableErms;                                                           //Steven 20160711 : 使用進階版RMS
    bool bClearLotInfoWhenTrayFeed;                                             //Steven 20240916 : Tray Feed之後, 要不要清除Device Name
    AnsiString sErmsPath;
    bool bEnableRmsCheckSetupFile;                                              //是否使用RMS

    //這是上海安靠的額外要求-----------------------
    double fAmbientTemp;                                                        //Steven 20101215 : 常溫的定義
    bool bCheckFile;                                                            //Steven 20101209 : 是否要檢查溫度
    AnsiString sProductName;                                                    //Steven 20110117 : 目前使用的工作檔
    AnsiString sProductTemp;                                                    //Steven 20110117 : 檢查用的溫度值

    bool bEnableEPCheckFuntion;                                                 //KYEC要每次都檢查EP

    bool bInOutArmPlaceSkipSuckDetect;                                          //jou 2011-05-26
    bool bUnloadTrayFree;                                                       //jou 2011-05-27

    bool bRotateShNeedCheck;                                                    //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭
    double dRoShCheckDelayTime;                                                 //Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭的延遲時間

    int  iInArm60mmOffset;                                                      //jou 2011-08-05 In Arm Y Pitch 60mm Offset
    int  iOutArm60mmOffset;                                                     //jou 2011-09-26 Out Arm Y Pitch 60mm Offset

    //Steven 20110819 : 換到Config可以調整--------------
    int iRotateADC;                                                             //Steven 20101018 : 轉轉蝦頭的加減速
    int iInitSpeed;                                                             //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    int iPJogHighSpeed;                                                         //kevin 20110531 旋轉SHUTTLE 鎖最高速度
    //--------------------------------------------------
    bool bQAModeFirstIn;                                                        //判斷是不是第一次進入QA模式
    int  iBackUpTesterMode;                                                     //備份測試機連線狀態
    bool bBackUpInArmMode;                                                      //備份In Arm使用的吸嘴模式
    bool bBackUpAutoFeed;                                                       //備份CleanOut模式
    double dSingleTempLimit[tcTotalCount];                                      //Steven 20111013 : 個別溫度Offset
    int  iContactForceMap[4][2];                                                //Steven 20111107 : Contact Force的公式         //wei 20151005 add 56mm

    bool bEnableSocketCommunication;                                            //ChungHung 20130112 add for ASE_KR Socket Tester
    int SocketTimeOut;                                                          //ChungHung 20130112 add for ASE_KR Socket Tester
    AnsiString SocketIP;                                                        //ChungHung 20130112 add for ASE_KR Socket Tester
    AnsiString SocketPort;                                                      //ChungHung 20130112 add for ASE_KR Socket Tester
    AnsiString SocketHandlerID;                                                 //ChungHung 20130204 add for ASE_KR Socket Tester
    AnsiString RMSTesterID;                                                     //Ifor 20231219 add Tester ID
    int iSocketTemptureRangeOver;                                               //Steven 20140308 : DUT溫度限制改成上下限分開
    int iSocketTemptureRangeUnder;                                              //Steven 20140308 : DUT溫度限制改成上下限分開
    bool bUseBinFailCount;                                                      //ChungHung 20120724 add BinFailCount
//    bool bDutTempOverNotAlarm;                                                //Steven 20140308 : DUT溫度異常到下次Start前不加熱    //Steven 20230308 : 沒用到Mark

    bool bHaveRTCCheckSiteMap;                                                  //Steven 20140513 : [D35]
    double dESDDataReportTime;                                                  //Ifor 20160321 : add ESD Data Report Time
    double dESDDelayTime;                                                       //Ifor 20160321 : add ESD Data Report Time
    bool bInitialStartNeedAsk;                                                  //ChungHung 20130529 add manual change start mode 如果是InitialStart 按下Start時要詢問
    //ChungHung 20121002 add OCR Function
    int iOCRConditions;                                                         //ByNewTray:0 ByInitialStart:1
    int iOCRByNewTrayIntrvalTray;
    int iOCRMaxInspDevices;
    int iOCRByAllDevice;                                                        //wei 20150713 BarCode掃全部  //wei 20150924
    int iOCRSkip;                                                               //wei 20150721
    int iOCRRetry;                                                              //wei 20150723
    int iOCRWordCount;                                                          //wei 20160803
    AnsiString asOCRWordType;                                                   //wei 20161128 確認字串各自Type是否正確
    int iBlueLight;                                                             //wei 20160803
    int iRedLight;                                                              //wei 20160803
    bool bDisabledKeyin;                                                        //wei 20161004 No IC 不能Keyin
    bool bCheckBarCodeMap;                                                      //Frank 20161025 確認四個角落的OCR Code
    bool bEnabledOCRCheckIC;                                                    //wei 20161228 確認Tray是否有IC
    bool bFTContinueON;                                                         //kevin 20121008 FT MODE 不能關連續FAIL
    int  iFTMAXValue;                                                           //kevin 20121008 FT MODE 連續FAIL 最大數量
    int  iAmbTemperatureRange;                                                  //jou 2013-04-11 Ambient Temperature Range
    int  iEP_Min_KG;                                                            //jou 2013-07-19 EP Min KG

    int  iCarRecordDelayTime;                                                   //wei 2013-12-09
    bool bEnable_ESD_COMERR_Report;                                             //Ifor 20150724 :Enable ESD COM ERR Report Function
    double fIndexCheckOffset;                                                   //ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset
    double dD41SocketInitialCheckOffset;                                        //Steven 20100818 :[D41]       //Steven 20230308 : [D41] LastSet 改 IniConfig
    int iD41SocketInitialICCheckPosition;
//    bool bD41TestSocketICCheckSkip;

    AnsiString sLotID;                                                          //Steven 20140814 : Add for ASE_M

    int  iATCTemperatureOverLimit;                                              //Steven 20140916 : [L11-4] ATC的最高上限溫度
    bool bUseTemperatureReferSensor;                                            //Steven 20150108 : [L11-5] For海思使用兩組感溫
    bool bEnable_SECS_GEM;                                                      //jou 2012-03-12 Enable SECS_GEM
    bool bRCMDStart;                                                            //Steven 20141006 : SECS GEM使用Remote Start功能

    //ChungHung 20141002 add for KYEC AutoRetest
    int  iAutoRetestLimit;
    int  iFailYieldRate_ART;
    int  iARTTrayArmSpeed;
    bool bIndexJamInArmAway;                                                    //Index掉料時,In Arm要移開
    bool bUseAutoSiteMapping;                                                   //開啟AutoSiteMapping功能
    bool bDutOnOffNeedASM;                                                      //Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping
    bool bInOutArmCanPushHome;                                                  //In/Out Arm Jam時,可以按Home對吸嘴歸零
    bool bIndexEveryTimeCheckEP;                                                //Index每一次都確認EP是否有充飽氣。
    bool bCanDisableTempMonitor;                                                //能夠關閉溫控器監視。
    bool bPowerSaveFunction;                                                    //省電模式
    bool bEventLogAutoSaveFunction;                                             //自動存EventLog
    int  iUserLanguage;                                                         //Steven 20120203 : 使用者的語系
    bool bShowFunctionWindow;                                                   //顯示在溫度值下面的功能開關畫面
    bool bHaveRotateShuttle;                                                    //旋轉蝦頭
    bool bOneCycleNeedPowerOff;                                                 //Steven : One Cycle後要關電
    bool bOneCycleCanTrayFeed;                                                  //Steven : OneCycle時,要可以選擇Tray Feed
    bool bCleanOutCanTrayEnd;                                                   //Steven : CleanOut後,可以選擇Tray End
    bool bEnableInOutArmPlaceSkipSuckDetect;                                    //skip in/out arm drop error
    bool bEnableUnloadTrayFree;                                                 //unload tray汽缸開門時也要可以打開
    bool bHighModeCanOffTemp;                                                   //Steven 20110524 : 加熱模式要可以隨時關加熱
    bool bOneCycleDoQuickCleanOut;                                              //Steven 20110524 : OneCycle後,只做快速CleanOut
    bool bEnableCCDUSETCPIP;                                                    //Steven 20110528 : 開啟CCD功能
    bool bEnableAutoCleanFunction;                                              //Steven 20110528 : 開啟Auto Clean功能
    bool bTempOverCannotRun;                                                    //Steven 20110607 : 溫度過高時,機器不能動
    bool bIndexDropNeedPwdByIni;                                                //Steven 20110627 : Index掉料時,需要輸入密碼,密碼放在D:\HT9045\system\SpecialErrNote.ini
    bool bChangeTempAutoSetDown;                                                //Steven 20110707 : 更換溫度時,自動按下Set鈕
    bool bDoorOpenShuttleContinueHeat;                                          //Steven 20110709 : Chamber門打開時不關閉加熱電源，只將Temp=0，Hotplate & shuttle除外。
    bool bOpenDoorNotStopFan;                                                   //Steven 20110725 : Chamber門打開時不關閉風扇。
    bool bLowYieldAlarmSameNS;                                                  //jou    2011-07-16 : Low Yield Alarm模式與NS機台相同,skip會清除單獨Site.
    bool bAlarmNeedServoOff;                                                    //Steven 20110802 : Alarm時,相關位置要可以Servo Off
    bool bEnableHotplateVibration;                                              //jou 2011-08-09 : Hotplate也要敲敲敲
    bool b1x2Use4Suck;                                                          //Hung 20110812 : 1x2使用4吸嘴
    bool b1x4Use8Suck;                                                          //Hung 20110812 : 1x4使用8吸嘴
    bool b2x2Use8Suck;                                                          //Hung 20110812 : 2x2使用8吸嘴
    bool bHotPlateMove1CM;                                                      //Hung 20110812 : HotPlate使用1CM轉板
    bool bKoreaFunction;                                                        //Steven 20110831 : 韓國代理商的需求
    bool bSingaporeFunction;                                                    //Steven 20120910 : 新加坡代理商的需求
    bool bSPILFunction;                                                         //JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction
    bool bMaximFunction;                                                        //JerryYang 20190522 Maxim統一軟體功能
    bool bSIGURDFunction;                                                       //KaiChen 20200506 ：矽格統一軟體功能
    bool bFinishSuckAfterPause;                                                 //Hung 20110901 : finish suck and destroy 後才暫停
    bool bOnlyRoomOrHot;                                                        //Steven 20110814 : 只要兩種,不要Ambient/Hot模式
    bool bLastLoaderNoInSide;                                                   //Steven 20110922 : 最後一盤不入料
    bool bLastLoaderAutoCleanOut;                                               //Steven 20111028 : 搭配"最後一盤不入料"功能,要不要自動CleanOut
    bool bRotateShNeedSensorCheck;                                              //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
    bool bTModeMotorFree;                                                       //Hung 20110923 : 開啟index T Mode時，開啟馬達煞車(測試驗證使用)
    bool bUseFix3;                                                              //kevin 20110916 暫時性ic超過 15mm就不使用Fix3 tray
    bool bQAMode;                                                               //Steven 20111005 : QA模式
    bool bCanByPassIonFan;                                                      //Steven 20111013 : 可以不檢查離子風扇
    int  iTempeAlarmSecond_Over;                                                //Steven 20111027 : 溫度過高的Alarm時間
    int  iTempeAlarmSecond_Below;                                               //Steven 20111027 : 溫度過低的Alarm時間
    bool bSiteMappingDisable;                                                   //jou 2011-10-04 Site Mapping Disable，"Security_new.def", "Setup", "Machine Setup=1 || 4"
    bool bSiteMappingFastSetDisable;                                            //jou 2011-10-11 Site Mapping 快速設定Disable
    bool bRecordPiggyBackStartEnd;                                              //jou 2011-11-14 start : 紀錄piggyback時間
    bool bAlarmMustRedColor;                                                    //Steven 20111116 : 特殊Alarm需要改紅底
    bool bTrayAssignUseGraphic;                                                 //Steven 20111121 : 使用圖片去顯示Tray Assign
    bool bContactAlwaysIncludeShuttle;                                          //Steven 20090926 for KYEC 29818 : Never Enable and Always True
    bool bNoHeadaddChamberOption;                                               //Steven :Head + Chamber disable
    bool bDisibleResetButton;                                                   //ChungHung 20111208 Disable Reset Button
    bool bShuttleMode50;                                                        //Dell 20111212   : 使用關Arm模式後,將被關的Arm測試高度設在-50mm
    bool bAbnormalStartCheck;                                                   //Steven 20111216 : 不正常關程式偵測
    bool bRetryNoNeedRestartGpib;                                               //Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB
    bool bResetCanServoOff;                                                     //Steven 20111228 : 按下Reset時,若In/Out Arm沒IC,就Servo Off
    bool bFix3PutAllFullIC;                                                     //ChungHung 201111215 Fix3盡量擺滿
    //bool bShuttleMustFull;                                                    //ChungHung 20111230 add Shuttle必須放滿
    bool bDisableSelectSearchLast;                                              //jou 2012-01-10 取消Setup，Search Last Mode功能。
    bool bShuttleModeAccseeLevel;                                               //jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] 請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force
    bool bAmbRunChamberFanCanStop;                                              //jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動
    bool bIndexDropOnlySKIP;                                                    //jou 2012-02-13 index drop error only skip
    bool bIndexPickErrOnlySKIP;                                                 //jou 2012-02-13 index pick-up error only skip
//    bool bOnlyUse30mmHead;                                                    //Steven 20120203 : 只使用30mm的浮動頭
//    bool bIndexYAxisServoOff;                                                 //jou 981214 Contect & OneCycle & CleanOut & Home 完成時，index Y axis servo off
    bool bIndexDropErrCanMove;                                                  //jou 2012-02-24 index drop error,have button can move arm Y front or Rear
    bool bIndexPickupErrStop;                                                   //jou 2012-02-29 index pick up error,index arm move to center & alarm
    bool bFTBin2RTBin;                                                          //jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣
    bool bDownLoadAutoCountClear;                                               //jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤
    bool bFTTrayAss2RTTrayAss;                                                  //jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣
    bool bIOFormCanControlHeaterFan;                                            //Steven 20120607 : 當chamber門開啟時,需要能開關風扇
    bool bShowOffYieldBlink;                                                    //Steven 20120609 : 功能關畫面要不要閃爍
    bool bHeadSocketMode;                                                       //jou 2012-05-30 增加 Head + Socket Mode
    bool bIndexPickupWait;                                                      //jou 2012-06-29 Index Pick up need wait Soak Time
    bool bAutoTrayLink;                                                         //jou 2012-06-14 Auto Tray Link
    bool bRTCbySystem;                                                          //ChungHung 20120716 RTC by System
    bool bDoorOpenHeadContinueHeat;                                             //ChungHung 20120719 add DoorOpenHeadContinueHeat
    bool bRemeberAutoHeight;                                                    //ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50
    bool bEnableKT4HAlarm1;                                                     //Steven 20120809 : KT4H使用Alarm1作加熱保護
    bool bResetClearArmIC;                                                      //Steven 20120830 : Reset按下時,把吸嘴上的IC清空
//    bool bShowMotionViewBtn;                                                  //Steven 20120830 : 顯示MotionView按鈕    //Steven 20160816 : mark for 全面啟用MotionViewBtn
    bool bTesterTimeUpErrorNeedPassword;                                        //jou 2012-08-28 Tester Time up Error Need Password
    bool bDualSiteCloseAbCanFullHotplate;                                       //ChungHung 20120911 add 與OneCycle can desable site 衝突
    bool bResetPutUntestToErrorBin;                                             //Steven 20120924 : For SCS, 使用Reset Mode,但是已測的要繼續分Bin
    bool bAnyLevelCanGetStateRecode;                                            //ChungHung 20120922 add
    bool bSocketCommunication;                                                  //ChungHung 20130112 add for ASE_KR Socket Tester
    bool bIndexArm2SupplyLight;                                                 //jou 2012-10-19 Index Arm 2 供應光源 for CMOS
//    bool bResetPortCOM1;                                                        //jou 2012-10-24 Reset Port COM1
    bool bDualSiteSupply4CH;                                                    //jou 2012-11-20 Dual Site supply 4's Channel
    bool bInitialStartDelayCount;                                               //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
    bool bBinBox;                                                               //jou 2012-12-11 support Bin Box
    bool bPasswordSecret;                                                       //jou 2013-01-04 Password Txt 加密
    bool bShowLotInfo;                                                          //jou 2013-01-18 Show Lot Info
    bool bShowMainDebugRecord;                                                  //jou 2013-02-25 Show Main form Debug Record
    bool bAutoCleanShuttleDisable;                                              //jou 2013-02-27 Auto Clean disable shuttle sensor detect
    bool bShowTrayAndDeviceDir;                                                 //jou 2013-03-25 show Tray & Device Direction
    bool bShowFTandRTButton;                                                    //jou 2013-04-27 Show FT & RT Buttion
    bool bShowFTandRTButtonCanClick;                                            //Steven 20131224 : FT & RT Buttion 可以按
    bool bRecordSkipPosition;                                                   //jou 2013-05-30 Record Skip position
    //------------------------TSMC---------------------------------------------------------------
    AnsiString sMachineType;                                                    //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
    AnsiString sGPIBMachineID;                                                  //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
    //------------------------TSMC---------------------------------------------------------------
    bool bNewResetFunction;                                                     //Steven 20130625 : 新的Reset方式
    int  iI25UseGPIBFormat;                                                     //kevin 20130705 台積電通訊規格  0:HT    1:NS   //Steven 20160301 : 改為海思格式
    bool bEnableStepShuttle;                                                    //jou 2013-07-16 Step Shuttle check Index -> Input
    bool bNoTrayAutoCleanOut;                                                   //jou 2013-08-01 Loader No Tray Auto Clean
    bool bHeadChamberSocketMode;                                                //2013-11-20    Dell    for TSMC Add Chamber + Head +Socket
    bool bG06HomeinitialCheckZ1;                                                //kevin 20131218 歸hom前檢查是否有tray放在hotplate 造成機構損壞 按z1 確認
    bool bUseAutoOffsetFunction;                                                //jou 2013-08-29 Use Auto Offset Funtion
    bool bEnableTestingNeedStopAllMotor;                                        //jou 2013-09-25 Testing Need Stop All Motor
    bool bControlTorque;                                                        //jou 2013-11-05 Index Control Torque
    bool bShowFormByInitPos;                                                    //jou 2013-12-02

    bool bIndexDropOnlyReset;                                                   //jou 2013-12-02 Index Drop Only Reset
    bool bOutShLoseNeedOpenChamber;                                             //jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1
    bool bFTPJamCodeUpload;                                                     //ChungHung 20140108 add FTP unload jam code
    bool bUseTrayBlockMode;
    bool bTestIcCheckInContact;                                                 //ChungHung 20140327 add by Customer
    bool bStartProductOnLine;                                                   //kevin 20140407 生產前OP OFF_LINE 強制 On line
    bool bTemp25degControl;                                                     //jou 2014-06-07 Temperature 25 deg. control
    bool bASE_Report;                                                           //kevin 20140918 ASE Kaoshsiung 每一顆IC履歷資料
    bool bASMAutoSiteMapBinComBine;                                             //kevin 20150115 Auto Site map 所有bin 別 放在fix 2

//    bool bEnableSendJamCodeByTcpIp;                                           //ChungHung 20150518 add for SCK Send JamCode By TcpIp        //Steven 20161201 : Mark of SCK no use TCP IP
    AnsiString aAlarmCodeByTcpIp_Adress;                                        //ChungHung 20150518 add for SCK Send JamCode By TcpIp
    AnsiString aAlarmCodeByTcpIp_Port;                                          //ChungHung 20150518 add for SCK Send JamCode By TcpIp
//    int  iART_CatchTrayOffset;                                                //kevin 20150603 Auto 1 2 3 搬TRAY OFFSET                   //Steven 20230308 : 沒用到Mark

    int  iClearLoaderCount;                                                     //wei 20150810 拍拍Tray次數設定
    bool bSECS_GEM_OneCycle;                                                    //wei 20150824 Secs_Gem 斷線Onecycle
    int  iCleanLoaderOffset;                                                    //wei 20150826 拍拍Tray X軸 Offset
    bool bHandlerCanUse8PickAtHotMode;                                          //Steven 20151117 : 2x2 8Picker at Hot mode
    bool bRecordJamRateByTime;                                                  // 2015.11.11 , Joye , Add Jam Rate Record
    int  iRecordJamRateIntervalTime;                                            // 2015.11.11 , Joye , Add Jam Rate Record
    bool bChangeKitNoHardStop;                                                  //jou 2015-12-08 Xilinx 驗證用
    AnsiString LotID_ART;
    AnsiString OperatorID_ART;
    bool bLifeTimeCount[3];                                                     //wei 20160509 Life Time Count
    AnsiString ContactConditionName[3];                                         //wei 20160509 Life Time Count
    int ContactSet[3][2][16];                                                   //wei 20160509 Life Time Count
    int HeadContactCount[3][2][16];                                             //wei 20160509 Life Time Count
    int HeadContactCountHistory[3][2][16];                                      //wei 20160509 Life Time Count
    bool bEnableOCRMoveSRead;                                                   //wei 20161118 OCR S型讀取
    int iImageRotate;
    int iVibratorHP1;                                                           //JerryYang 20200612 振動馬達作動時間累計
    int iVibratorSht1;
    int iVibratorSht2;
    int iVibratorUnloader;
    int bVTESTFunction;
    int SocketContactSet[4][8];                                                 //Sam 20220720 : 新增一組 Socket Count
    int SocketContactCount[4][8];                                               //Sam 20220720 : 新增一組 Socket Count
    int SocketContactCountHistory[4][8];                                        //Sam 20220720 : 新增一組 Socket Count
    int iSiteMapDirection;                                                      //Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt

    bool bEnableStartposshift;                                                  //wei 20181225 初始點位
    int iStartposshift;                                                         //wei 20181225 初始點位
    bool OCRLightChange;                                                        //wei 20181225 光源auto change
    bool OCRLightNoDown;                                                        //wei 20181225 光源不下降
    bool bCompareOCRData;                                                       //KenHsieh 20220825 : 新增OCR比對功能
    bool bOCRAndBinLog;                                                         //KenHsieh 20230406 : 新增OCR Data + Bin Log功能
    bool bOCRBinLogAddMark;                                                     //KenHsieh 20230412 : 新增OCR Data + Bin Log功能 + Mark
    int iOCRPort;                                                               //wei 20181225 ocr port
    //AI(ht9045-v899) 20260522: keep OCR capture trigger mode configurable while defaulting old machines to switch trigger
    int iOCRTriggerMode;
    bool bEnabledOCRCheckWordCount;                                             //wei 20220413 開關字數判斷功能
    int iByLotInputCount[5];
//以下是有照功能排序的----------------------------------------------------
    //[A]---------------------------
    bool bA01AutoSwitchToOperatorMode;                                          //kevin 20150206 超過時間變成op模式
    bool bA01PressStartAutoSwitchToOperatorMode;                                //JerryYang 20170705 add for JCET 按start要切為OP權限改為選項
    bool bA02DisableSaveParsWhenSwitchToOp;                                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
    int  iA01ChangeOpTime;                                                      //kevin 20150206 超過時間變成op模式
    bool bA02BinModelPrime;                                                     //ChungHung 20120912 add Bin Setting like Epson
    bool bA03UseAfterHomeCarryAndSuckIcToRBin;
    bool bA03_Enable;                                                           //JerryYang 20260504 : for SPIL -- A03 Enable (Lock by File)
    bool bA04LoaderTraySplitFailCanSkip;
    bool bA05UseAutoDocking;                                                    //ChungHung 20120718 add UseAutoDocking Check Sensor
    bool bA09_ByArmCloseSite;                                                   //ChungHung 20130910 alter for SCK can close site by Index
    bool bA09_1_AutoCloseArm;                                                   //Steven 20220819 : 單Arm Site全關時, 就把Arm關了
    bool bA08LastLoaderAutoCleanOutAndCheckAgain;                               //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
    bool bA10_AutoReTest;                                                       //ChungHung 20140317 add Auto Retest
    bool bA10TestModeForART;                                                    //Steven 20161208 : ART need to set to 32binGS for ATK
    int  iA10TestModeForART;                                                    //Steven 20161208 : ART need to set to 32binGS for ATK
    bool bA10_5SCKART_AutoCorrection;                                           //Steven 20171211 (Wei) : Auto correction for SCK ART
    bool bA10_6_HANA_ART_TestMode_Enable;                                       //JimmyChiu 20241023 HANA ART Function
    int  iA10_6_HANA_ART_TestMode;                                              //JimmyChiu 20241023 HANA ART Function
    bool bA10_7_Renesas_FTCT;                                                   //RogerYang 20251108 : 瑞薩FTCT
    bool bA11BarcodeTime;                                                       //20140310 Wei  Barcode Reader持續時間
    int  iA11BarcodeTime;                                                       //20140310 wei  Barcode Reader持續時間
    bool bA12ClearLoaderDevice;                                                 //ChungHung 20140701 add AutoRetest
    bool bA14UseBarCodeSetWorkFile;                                             //Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔
    bool bA15AutoDecayTest;                                                     //Ifor 20150924 :Add Auto Decay Test
    bool bA15_1ESDGiveWayFunction;                                              //Ifor 20240830 add:ESD 讓位功能
    bool bA16ContactTestDropContact;                                            //wei 20150831
    bool bA17RESETButtonDisable;                                                //kevin 20151113 RESET 按鍵使用
    bool bA17_1RESETCleanOutWithoutTest;                                        //JimmyChiu 20211012 reset->clean out 不進行檢測排料
    bool bA19UsePMAlarmFunction;                                                //wei 20160225 PMAlarmFunction
    bool bA20_1CheckRTCFunction;                                                //wei 20171023 Disable Start比對
    bool bA20_2CheckTrayIDFunction;                                             //wei 20171023 Disable Start比對
    bool bA20_3CheckAutocleanFunction;                                          //wei 20171023 Disable Start比對
    bool bA20_4CheckContsFailFunction;                                          //wei 20171023 Disable Start比對
    bool bA20_5CheckOCRFunction;                                                //wei 20171023 Disable Start比對
    bool bA21RotateDetectErrNeedShake;                                          //JerryYang 20160825 Rotate sensor偵測異常,要先試著旋轉三次再跳alarm
    bool bA22MagneticScale;                                                     //Frank 20161109 add 磁性尺
    double dA22MagneticScaleKeepRunRange;                                       //Frank 20161109 add 磁性尺
    double dA22MagneticScaleStopRunRange;                                       //Frank 20161109 add 磁性尺
    bool bA23CheckLotNoInSLTReport;                                             //JerryYang 20170421 (Steven) JCET吳如春要求必須輸入lot no才能start  //JerryYang 20170706 重新啟用A23
    bool bA24AutoBackupSetupFile;                                               //Ifor 20170508 (wei) add Auto BackUp Setup File & Last Data
    AnsiString asA25RunExecutFile;                                              //Sam 20170916 (Steven) 移植超豐外部呼叫執行檔功能 form HT-7045
    AnsiString asA25RunExecutButtonName;                                        //Sam 20170916 (Steven) 移植超豐外部呼叫執行檔功能 form HT-7045
    bool bA26MotorSpeedSortDisplay;                                             //KaiChen 20171225 (Steven)：Add Speed Display
    bool bA27EnableLightScale;
    bool bA27_1LogEnableLightScaleData;                                         //KaiChen 20171228 ：Log Light Scale Data
    AnsiString  asA28_1ShowPmSopReadFilePath;                                   //KaiChen 20171111 (Steven) ：超豐 開啟指定路徑的 HTML 檔案(PM SOP)
    bool bA29EnableAutoCleanFunction;                                           //wei 20171023 Auto clean開關
    bool bA30SetupTeachFunction;                                                //JerryYang 20180921 Setup Teach功能
    bool bA31EnableAutoCleanIonFanFunction;                                     //Isaac 20210609 : IO觸發IonFan清針     //Steven 20230308 : A33 --> A31
    bool bA31AutoCleanIonFanInitialStart;                                       //Isaac 20210609 : IO觸發IonFan清針
    bool bA32EnableFTPAutomation;                                               //KaiChen 20190530 ：Sigurd FTP Automation
    bool bA32EnableCheckList[eCL_Total];
    AnsiString asA32_1_HandlerID;                                               //KaiChen 20190530 ：Sigurd FTP Automation
    bool bA32_2ReturnHandlerID2OI;                                              //KaiChen 20200618 ：矽格，可以選擇是否回傳 HandlerID 給 OI
    bool bA32_3For93KFunction;                                                  //Sam 20220620 : 中興廠新增 A32-3 功能 for 93K function

    bool bA33SetICToErrBinAfterOutShtLossIC;                                    //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
    bool bA35SetErrBinWhenOutShtLoseAndPickupErr;                               //JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin
    bool bA36OpenDoorSetErrBin;                                                 //JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN
    bool bA37LotStartLotEnd;                                                    //JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制
    bool bA39RecordRunState;                                                    //Ifor 20230420 add:ASEM要求新增Run Status Log Function
    int  iA39RecordTime;                                                        //Ifor 20230420 add:ASEM要求新增Run Status Log Function
    bool bA40DoNotRecord2DDataWhenContactShow;                                  //Ifor 20201124 add:Turn Off 2D Function When Contact Show
    bool bA38_SLT_Summary;
    bool bA50Enable1x4BiasYOffset;                                              //Steven 20200715 : for Tinton                                              //Steven 20230308 : A32 --> A50
    bool bA51EnableEQCMode;                                                     //JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode  //Steven 20230308 : A31 --> A51

    bool bA55EnablePMAlarmUpdateFromServerbyFTP;                                //JimmyChiu 20230315 : PM Alarm Update From Server by FTP
    AnsiString asA55PMAlarmUpdateFromServerbyFTP;                               //JimmyChiu 20230315 : PM Alarm Update From Server by FTP
    bool bA56EnableAutoTeachFunciton;                                           //JimmyChiu 20211020 : Auto alignment mode
    bool bA56AutoTeachShuttleYPositionWhenAutoHeight;                           //JimmyChiu 20211020 : Auto alignment mode
    int iA56ShuttlePickUpOffsetWhenAutoTeach;                                   //JimmyChiu 20211020 : Auto alignment mode
    int iA56SocketPickUpOffsetWhenAutoTeachOnly;                                //JimmyChiu 20211020 : Auto alignment mode
    bool bA57_1SaveArmSpeedByMachine;                                           //JimmyChiu 20220618 : save by machine
    bool bA57_2SaveTemperatureByMachine;                                        //JimmyChiu 20220618 : save by machine
    bool bA57_3SaveOffsetByMachine;                                             //JimmyChiu 20220618 : save by machine
    bool bA58DoNotRecord2DDataWhenContactShow;                                  //Ifor 20201126 add:Contact 頁面開啟時不記錄2D 資料
    bool bA60EnableAMR;                                                         //Sam 20240304 : 新增 AMR 功能
    int iA60LoaderQtyAtOneTime;
    int iA60NotifyQty[9];
    bool bA61DisableCleanMUBA;                                                  //Jeff 20241001 add Continuous Mode disable  clean MUBA
    bool bA62bUseStopMachineArmHome;                                            //Ztex 2024.10.30 Add Use Stop Machine In/Out Arm Need To Home
    bool bShowCloseSiteAlarmWhenStart;                                          //Jimmychiu 20230925 : Show Close Sites Alarm When Start
    bool bA65_BundleIDList;
    bool bA66_2D_Sort;
    bool bA67TriggerOneCycleWhenAlarm;                                          //JerryYang 20241028 : 矽品彰化要求 特定ALARM要觸發ONE CYCLE
    bool bA68_AutoLoadUnload;                                                   //JerryYang 20250224 : add
    bool bA69_ScanBinLabelToBinCodeEdit;
    bool bA71UseBackupNowRecipe;
    bool bA72_FixAOIButton;                                                     //Eastsun 20260423 : 鎖定Button功能
    bool bA73UserOverrideFTRTBin;                                               //Eastsun 20260515 : KYEC RT FT Bin 分開設定
    bool bA74AOIFailCountLinkLotRunMode;                                        //Eastsun 20260515 : AOI Count 異常 跟Lot RunMode 連動設定
    bool bA75DownloadItemByAccessLevel;                                         //AI(safety-critical-change) 20260611 (RogerYang) : 下載複寫項目依權限控管(OP禁用)
    AnsiString sA71FolderPath;

    //[B]---------------------------
    bool bB01_UsePrecautionRecordFunction;                                      //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    int  iB01_AutoWakeupPrecautionRecordFormTime;                               //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)
    AnsiString asB01_PrecautionRecordSavePath;                                  //Sam 20171120 (Steven) AddPrecautionRecordFunction (form HT7045)

    bool bB02_HanderMajorMaintenanceRecordFunction;                             //Sam 20171120 (Steven) AddHanderMajorMaintenanceRecordFunction (form HT7045)
    AnsiString asB02_HanderMajorMaintenanceRecordSavePath;                      //Sam 20171120 (Steven) AddHanderMajorMaintenanceRecordFunction (form HT7045)

    bool bB03_TesterReport;                                                     //Sam 20231115 : PTI 新增 Tester report
    AnsiString sB03_Customer;                                                   //Sam 20231115 : PTI 新增 Tester report
    AnsiString sB03_DeviceID;                                                   //Sam 20231115 : PTI 新增 Tester report

    bool bB05_OSReport;
    AnsiString sB05_OSReportPath;                                               //Steven 20250513 : OS Report

    bool bB11UsePATServerFile;                                                  //Jimmychiu 20241110 : add PAT Class
    AnsiString sB11PATServerPath;                                               //Jimmychiu 20241110 : add PAT Class
    bool bB12UsePATSetup;                                                       //Jimmychiu 20241110 : add PAT Class
    AnsiString sB12PATSetupPath;                                                //Jimmychiu 20241110 : add PAT Class
    AnsiString sB13PATJobUploadPath;                                            //Jimmychiu 20241110 : add PAT Class
    AnsiString sB13PATJobDownloadPath;                                          //Jimmychiu 20241110 : add PAT Class
    int iB14IntervalTime;                                                       //Jimmychiu 20241110 : add PAT Class
    AnsiString sB14RealTimePath;                                                //Jimmychiu 20241110 : add PAT Class

    //[C]---------------------------
    bool bC01_FanDirection;                                                     //wei 20160215大風扇方向
    bool bC02InstallCCD;
    bool bC03UseCatchTray;
    bool bC04EnableTestTempIC;
    //新竹矽品的省電模式-------------------------------
    bool bC05_PowerSaveTemp;                                                    //kevin 20110328 啟動溫度省電模式
    bool bC05_PowerSaveMotor;                                                   //kevin 20110328 啟動馬達省電模式
    int  iHaltTime_Motor;                                                       //kevin 20110328設定省電模式時間
    int  iHaltTime_Temp;
    int  iPowersaveMode;
    bool bC05_PowerSaveVacuum;                                                  //Steven 20221215 : Power saving for vacuum pump
    int  iC05HaltTime_Vacuum;                                                   //Steven 20221215 : Power saving for vacuum pump
    bool bC05_PowerSaveATC;                                                     //Ifor 20240401 : Power saving for ATC System
    int  iHaltTime_ATC;                                                         //Ifor 20240401 : Power saving for ATC System
    int  iInOutSaveStartTime;                                                   //進入省電模式 0:尚未開始 1:開始計時 2:計時到
    bool bC06_ByPassIonFan[MAX_IONFAN];                                         //Steven 20111013 : 可以不檢查離子風扇
    bool bC08_SocketSensor;                                                     //kevin 20130504 使用socket sensor detect 功能 piggy back
    bool bC08_1_CheckSocketSensorDetectON;                                      //Isaac 20201130 : Socket Sensor按Start後要偵測是否啟動功能
    bool bC09_CarRecord;                                                        //wei 2013-12-09
    bool bC11UseMonitorView;                                                    //JerryYang 20160621 錄影監視功能
    bool bC12UsePEMode;                                                         //Ifor 20170125 : add PE Mode
    bool bC13NeedToRestartGroundWhenInitialStart;                               //Sam 20220107 : 矽格北興 Initail Start 要重啟 GroundMan
    bool bC14SaveBinDisplayLog;                                                 //Steven 20220309 : BinDisplay Log
    bool bC16UseBarCoderChangeSetupFile;                                        //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
    bool bC17UseLoaderColorSensor;                                              //Jimmychiu 20230630 : add color sensor MU-N in Loader
    int  iC17DelayTimes;                                                        //Jimmychiu 20230630 : add color sensor MU-N in Loader
    bool bC17_1_SkipAlarm;                                                      //Jimmychiu 20230630 : add color sensor MU-N in Loader
    //Ztex 2023.05.03 Add Add HT-1032 Dew point ==>
    bool   bC20_1EnableEnergySavingDryAir;
    int    iC20_2LowTempOffEnergySaving;                                        //Add check Low Temperature Not Use EnergySaving
    int    iC20_3TempOverUseEnergySaving;                                       //Add Ower  Temperature Open  EnergySaving
    bool   bC20_4IndexUseForstSensor;                                           //add check DewPointMeter
    //Ztex 2023.05.03 Add Add HT-1032 Dew point <==
    int iC21MotSp_HP1[3];                                                       //JerryYang 20230814 : add震動馬達通訊調速版本
    int iC21MotSp_HP2[3];
    int iC21MotSp_SHT1[3];
    int iC21MotSp_SHT2[3];
    int iC21MotSp_AUTO1[3];
    int iC21MotSp_AUTO2[3];
    int iC21MotSp_AUTO3[3];

    bool bC24InitialStartCheckCylinder;                                         //JerryYang 20250120 : add
    bool bC25FloodGateCloseByATCEnable;                                         //JerryYang 20260910 : add [C25] Shuttle Floodgate close by ATC enable only  AI(W906-W195) 20261009 (St02-E) L10: golden 913 Config.h:436 (config.ini [Function]; the settings-page row comes later through St01's generator -- until then 0 = golden's default)
    //[D]---------------------------
    double dIndex60mmLoadRate;                                                  //jou 2011-06-10
    double dIndex40mmLoadRate;                                                  //Steven 20110704
    double dIndex30mmLoadRate;                                                  //jou 2011-06-10
    double dIndex56mmLoadRate;                                                  //wei 20151005 add 56mm

    double dIndex60mmLoadRate_Offset;                                           //2014-06-26    Dell    for TSMC 高溫Load cell offset
    double dIndex40mmLoadRate_Offset;                                           //2014-06-26    Dell    for TSMC 高溫Load cell offset
    double dIndex30mmLoadRate_Offset;                                           //2014-06-26    Dell    for TSMC 高溫Load cell offset
    double dIndex56mmLoadRate_Offset;                                           //2014-06-26    Dell    for TSMC 高溫Load cell offset        //wei 20151005 add 56mm

    double dIndex60mmLoadRate_NS;                                               //wei 20150303   京元NS浮動頭
    double dIndex40mmLoadRate_NS;                                               //wei 20150303   京元NS浮動頭
    double dIndex30mmLoadRate_NS;                                               //wei 20150303   京元NS浮動頭
    double dIndex56mmLoadRate_NS;                                               //wei 20151005 add 56mm

    bool bD01EnableReadTorque;
    int  iD01ReadTorqueDelayTime;
    int  iD01ReadTorqueTimeCount;
    double dD01ReadTorque;
    double dD01ReadTorqueDelayTime;
    bool bD01_1EnableReadAndCheckTorque;

    bool bD02OffReadTorqueDuringTest;
//    int  iD03MaxPreasureIndex;                                                //Steven 20161024 : fixed for contact torque
    bool bD04MinForceByFile;                                                    //Steven 20190314 : Min force is read from file
    double dD04MinForceByFile;
    double dD04MinForceByFile_20mm;                                             //Steven 20220822 : Min force for different SLK
    double dD04MinForceByFile_30mm;
    double dD04MinForceByFile_40mm;
    double dD04MinForceByFile_60mm;
    double dD04MinForceByFile_80mm;                                             //Ifor 20240620 : add缸徑 80
    bool bD05ContactCountAlarm;                                                 //wei 20170327 add
    int  iD05_ContactCountAlarm;                                                //wei 20170327
    bool bD05_1SaveSocketCntByHandler;                                          //Steven 20250807 : By handler save contact count
    bool bD06ContactOffsetDefaultValue;                                         //JimmyChiu 20200120 add
    double  dD06_ContactOffsetDefaultValue;                                     //JimmyChiu 20200120 add
    bool bD10ManualHeightComptibleWithNS;
    bool bD11NoIcSkipAutoHeight;                                                //Steven 20110726 : Shuttle沒IC時,該Arm不要Auto Height
    bool bD12UseDeviceFormPressDoShtHeight;                                     //Steven 20140220 : 用生產ep去做蝦頭auto high
    bool bD13CheckIndexHomeSensor;                                              //Steven 20140828 : 歸零後檢查Index位置
    bool bD14_AutoHeightUseSetTorque;                                           //Steven 20141105 : 使用指定的扭力進行Auto Height
    int  iD14_AutoHeightUseSetTorque;
    bool bD15_AutoContactTest;                                                  //Steven 20150224 : Auto Contact Test
    bool bD16_StepContactTest;                                                  //Steven 20150811 : Step by Step Contact Test
    int  iD17_UseHardwareHeightToContact;                                       //Steven 20170411 (wei) : SCK的SIP怕刮傷,所以Contact Height使用硬體高度
    double dD17_3CheckEPLeakage;                                                //Steven 20231025 : 充氣跟不充氣都做一次auto height, 然後檢查有沒有漏氣
    bool bD18_AutoHeightWhenChangeRecipe;                                       //Steven 20221013 : ATK希望更換工作檔時, 需要提示有沒有做Auto Height.
    bool bD21EnableFinishTestUpWait;
    int  iD21FinishTestUpWaitHeight;
    int  iD21FinishTestUpWaitTime;
    bool bD22SupportMultiDoubleContact;
    int  iD22DoubleContactCount;
    bool bD22DoubleContactNoNeedReContact;                                      //Steven 20131202 : Double Contact不需要Index Arm上下動
    bool bD22VerifyMode;                                                        //Sam 20221012 : 新增 VerifyMode 功能
    bool bD22DoubleContactUseDiffSRQ;                                           //Steven 20230508 : 南茂鐘永生說要使用0x41  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號
    bool bD22_4_PassBinCanDoubleContact;                                        //JerryYang 20230914 : add
    bool bD23EveryDeviceDoubleContactFirstNoTesting;                            //ChungHung 20140709 add for SPIL
    int  iD23_MultiContactCount;                                                //Steven 20151001 : Add for TSMC
    int  iD23_DoubleContactSRQ;                                                 //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號
    bool bD24EnableEPCheckFuntion;
    bool bD26EnableEPEncoderRange;                                              //ChungHung 20111217
    bool bD26_3EnableDualEPEncoderRange;                                        //Ifor 20221228 add: Dual Force EP Check
    int  iD26EPEncoderRange;                                                    //ChungHung 20111217
    int  iD26_3DualEPEncoderRange;                                              //Ifor 20221228 add: Dual Force EP Check
    bool bD26EnableEPLog;                                                       //Ifor 20150706
    bool bD26EnableEncodeShow;                                                  //Ifor 20150706
    int  iD26_3FixValueOrPercentage;                                            //Richard 20230428 : EP固定值或百分比
    bool bD27UseSingleSite85kg;                                                 //Dell 2012-01-03 在1X2模式下,關Site能達85kg
    bool bD28MaxForceLimitByDiameter;                                           //Steven 20200813 : 用缸徑計算最大壓力
    bool bD29EnableIndexContactDelay;                                           //Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield)
    bool bD30EnableSiteModeSelect;
    bool bD31RTCChangeRecipeNeedreCreateModel;                                  //jou 2012-03-01 [D31] RTC Change Recipe Need reCreate RTC Model

    double dTrayXScale[eTrayCount];
    double dTrayYScale[eTrayCount];

    bool bD32_35TrayPitchIndexDOnMoreThen500MS;                                 //Steven 20120727 : Tray Pitch 大於35mm的話, Index吹氣至少要0.5秒
    bool bD33RTCInitStartVerify;                                                //Handler Use Model Verify
    bool bD34GailDMCProtection;                                                 //ChungHung 20131230 add for ATK
    bool bD35RTCCheckSiteMap;                                                   //Steven 20140513 : [D35]
    bool bD36EnableRTCAutoModelVerify;                                          //jou 2014-06-24 RTC 自動進行Model驗證
    bool bD36_1EnableRTCAutoModelVerifyLive;                                    //jou 2014-06-24 RTC 自動進行Model驗證
    int  iD36_RTCAutoVerifyReleaseHeight;                                       //jou 2014-06-24 RTC 自動進行Model驗證
    int  iD36_RTCAutoVerifyPickHeight;                                          //jou 2014-06-24 RTC 自動進行Model驗證
    bool bD36_2AfterOneCycleNeedAutoVerify;                                     //JerryYang 20201116 : One cycle後要執行RTC auto verify
    bool bD37EnableManualProcess;                                               //ChungHung 20150526 add for QualComm US
//    bool bD38EnableTopPogoPinMode;                                            //ChungHung 20150528 add for 海思 _8Site1x4
    bool bD38IndexPutICToShtNoWaitMotion;                                       //Steven 20181228 : Add Index Action   //ChungHung 20171116 modify for Index Action
    bool bD40IndexICFallDownMustPressFMotorDown;
    bool bD41_Active;                                                           //Steven 20140627 : Add for ASE-CL -- D41 打勾
    bool bD41_Enable;                                                           //Steven 20140627 : Add for ASE-CL -- D41 Enable
    int  iD41_Position;                                                         //Steven 20140627 : Add for ASE-CL -- Inside/Above
    double dD41_Offset;                                                         //Steven 20140627 : Add for ASE-CL -- D41 高度
    bool bD41CheckbySetup;                                                      //kevin 20200801 by setup 功能只能用一次

    bool bD42IndexPickICShuttlePause;
    bool bD42_Active;                                                           //JerryYang 20160220 add for Amkor-Philippine -- D42打勾
    bool bD42_Enable;                                                           //JerryYang 20160220 add for Amkor-Philippine -- D42 Enable
    bool bD43IndexDropErrorCanRetryandSkip;                                     //ChungHung 20120717 add Index Drop Error Can Retry and Start
    bool bD43AutoRetryWhenIndexPickErr;                                         //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
    bool bD43IndexPickErrCheckSocket;
    bool bD44_Active;                                                           //JerryYang 20160220 add for Amkor-Philippine -- D44打勾
    bool bD44_Enable;                                                           //JerryYang 20160220 add for Amkor-Philippine -- D44 Enable
    int  iD44TestHeadCheckVacuumHeight;                                         //wei 20150609 延遲破壞高度
    bool bD44CheckIndexICDestroy;
    int  iD44TestHeadCheckVacuumTime;

    bool bD45UseOutArmCheckIndex;
    int  iD46WaitIndexDestroyTime;
    bool bD47EnableSocketPurgeFunction;
    int  iD47SocketPurgeCount;
    int  iD47SocketPurgeTime;
    int  iD47SocketPurgeType;                                                   //Steven 20190703 : Socket Purge include shuttle
    double dD47_5_ContactOffset;                                                //Steven 20190703 : Socket Purge include shuttle
    double dD47_6_ShuttleOffset;                                                //Steven 20190703 : Socket Purge include shuttle
    bool bD48PowerOffEmgCanNotUseZ1Z2;
    bool bD49RTCAlarmSetIndexToErrBin;                                          //JerryYang 20160712 for 力成,發生RTC Alarm時把Index上所有IC設為Errorbin
    bool bD50IndexPickErrSkipNeedCheckVac;                                      //jou 20171031 (Steven) : 增加開關,JSCC要求index pick up error 需再慢速下降吸一次
    bool bD51UseOnecycleCleanOutFinishTestArmAtRear;
    bool bD52InterFaceErrHeadNeedUp;
    bool bD53CCDLightOn;
    int  iD53LightOnMin;
    bool bD54SlowDown;                                                          //ChungHung 20110816 add
    int  iD54SlowDownScale;                                                     //ChungHung 20110816 add
    bool bD55DisableIndexCheck;                                                 //ChungHung 20120606 Disable IndexCheck
    bool bD56YieldPiggyBackEnable;                                              //kevin 20131009 強致 Enable Yield 裡面piggyback 功能
    bool bD57SiteMapCloseDisplay;                                               //kevin 20141203 關site顯示 site號碼
    bool bD58UseArm1PickPlaceArm2Test;                                          //kevin 20150127 讓setup 出現功能  Arm1 下壓 arm2 測試
    bool bD59_32SitePnpTogether;                                                //Steven 20150910 : 32Site 雙Arm一起吸放
    bool bD61IndexArmVacOffErrNeedPiggyBack;                                    //JerryYang 20160815 Index arm 發生Vaccum off error要做piggy back
    bool bD62PickUpErrorNeedPurge;                                              //Steveb 20161024 : 吸取異常需要吹氣一次
    int  iD62IndexBlowAirTime;                                                  //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
    bool bD63CheckIndexZHomeToZPhaseDistanceRange;                              //kevin 20170515 (wei) Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    int  iD63IndexZHomeToZPhaseRange;                                           //kevin 20170515 (wei) Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
    bool bD63_1FindMotorPhaseEveryGoHomeProcess;                                //Isaac 20201110 : Index Y find motor phase
    bool bD64IndexPickErrOnlySKIP;                                              //kevin 20171103 (wei) index pick up error only skip
    bool bD65EnableCheckSocketsensorFunction;                                   //Ifor 20171123 add
    bool bD66Initialstartautoheight;
    bool bD67LoadCellMeasure;                                                   //kevin 20190907 Arm 測區次數道量測 功能;
    double dD68DistanceRange;                                                   //Ifor 20190925 : add
    int GaliPosRange;                                                           //Isaac 20201012 : index Y超過範圍，做一次Tmode   //Isaac 20210604 : IndexY偵測範圍名子統一成IniConfig.GaliPosRange
    int  iD69IndexCheckModeForAutoClean;                                        //Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check
    bool bD70IndexCycleTimeRecord;                                              //Sam 20200916 : Add Index Cycle Time Record
    int  iD71IndexCheckOnOffMode;                                               //Isaac 20211019 : 可選擇做index check的時機
    bool bD72NNModeMoveShtAfterContact;                                         //Steven 20220531 : index下壓之後才能移動shuttle
    bool bD73ContactModeFast;                                                   //kevin 20220817 : Conttact mode 加速
    bool bD74RTCAutoTuning;                                                     //Sam 20230419 : 新增 RTC Auto Tuning 功能
    bool bD75OneCycleFinishedAlawayLearnRTCGolden;                              //Sam 20240117 : OneCycle 完成做完 Full view check 後都需要做 RTC Learning golden
    bool bRTC_Active;                                                           //Sam 20240311 : 新增 RTC Lock by file 功能
    bool bRTC_Enable;                                                           //Sam 20240311 : 新增 RTC Lock by file 功能
    bool bD78EnableIndexCheckHasICNeedPurge;                                    //Ifor 20200622 add:Index Check Has IC Need Purge
    bool bD79EnableIndexPickShuttleErrNeedPurge;                                //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    bool bD80AfterOutSHToRightSiteIndexCheck;                                   //Ifor 20240415 add科園廠功能
    bool bD81IndexCheckVacuumOnShuttle;                                         //JerryYang 20241010 : add
    bool bD82CheckIndexHasIC;                                                   //Jimmychiu 20250826 : 每次下壓確認有IC在socket
    bool bE86_InArmPickErrOnLoaderOnlyCanSKIP;                                  //JerryYang 20250120 : add
    bool bE87PickupErrorAtLoaderNeedOpenDoor;
    bool bE88_InArmHeightFollow7000;
    bool bE89_InArmHotPlatePitchUseScale;

    //[E]---------------------------
    bool bE30InArmUseDifferentScale;
    bool bE31OutArmUseDifferentScale;
    bool bE32ShuttleUseDifferentScale;
    bool bE33InOutArmZOffsetSameOne;
    bool bE34InOutArmPitchZOffsetSameOne;
    bool bE38CheckHotPlateWhileInitialStart;                                    //ChungHung 20120206 Hotplate check
    bool bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed;                       //ChungHung 20120206 Hotplate check
    bool bE39_1PutTheDevicesToErrorBin;                                         //ChungHung 20120206 Hotplate check
    bool bE40ClearAllHotICThenPickLoadIC;                                       //ChungHung 20120514 Clear HotIC then Pick Load IC
    bool bE41_35TrayPitchInOutSpeedSmall80Percent;                              //Steven 20120727 : Tray Pitch 大於35mm的話, In Out Arm需要小於80%
    bool bE42OutShuttleAlarmInArmServoOff;                                      //ChungHung 20120723 Output Shuttle Alarm InArm Servo off
    bool bE43AutoCleanUseHotplate;                                              //ChungHung 20131120 AutoClean use Hotplate1
    bool bE43_1_AutoCleanCountSaveFolder;                                       //Steven 20250527 : Save auto clean count to DefineAutoClean folder
    bool bE44EnableLoseDeviceOutShuttleServoOff;                                //ChungHung 20140522 add OutShuttle lose devices can servo off
    bool bE45_AllSetupFileUseOneFile;                                           //Steven 20140827 : 所有工作檔共用同一個Offset檔案
    bool bE46_LoaderUse2Offset;                                                 //Steven 20140827 : Loader可以分別調前後排的Offset
    bool bE47_ShuttleUse4Offset;                                                //Steven 20140827 : Shuttle可以分別調前後排的Offset
    bool bE48_ShuttleUse4Offset_Autoclean;                                      //20140923 wei :  Auto clean Shuttle可以分別調前後排的Offset
    bool bE49_LoaderOnlyRetryAndCleanOut;                                       //Steven 20141105 : Loader吸取異常只能Retry與CleanOut
    int  iE50_OutArmPickUpErrorOption;                                          //JerryYang 20210813 : 改成選項  //Steven 20141121 : OutArm吸取異常只能Retry
    bool bE51_EnableInArmZADC;                                                  //Steven 20141212 : 使用固定的ADC
    int  iE51_EnableInArmZADC;
    bool bE52_EnableOutArmZADC;
    int  iE52_EnableOutArmZADC;
    bool bE53LowYieldAutoClean;                                                 //kevin 20160802 : CosFunction.bLowYieldAutoClean --> IniConfig.bE53LowYieldAutoClean
    bool bE54CheckCloseSiteNoIC;                                                //Steven 20160922 : 因為OneCycle永遠先跑蝦頭1, 檢查加熱盤錯誤功能與ByArmCloseSite衝突
    bool bE55UseFix3FullTray;                                                   //Ifor 20161121 : add Use Fix3 Full Tray Function
    bool bE56LoaderRetryAtSamePosition;                                         //Steven 20170828 (wei) : Loader吸取異常時,要在同一個位置作Retry
    bool bE57HPCanUseAnotherVacuumDelay;                                        //Steven 20180125 (Jou) : 加熱盤的真空等待時間
    bool bE57YPitchHome;                                                        //kevin 20180822 (Steven) : in out arm Y pitch 放完IC 歸 Y PITCH HOME
    bool bE59GroupOffsetFile;                                                   //Steven 20190327 : Offset file使用中括號做群組
    bool bE60PickLoaderDropAutoSkip;
    bool bE61InArmStandbyPosOnLoader;
    bool bE62TryPickLastRow;                                                    //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
    bool bE63RetryPickLoader;
    bool bE64_50TrayPitchInOutSpeedSmall50Percent;                              //Ifor 20201224 add: Tray Pitch 大於50mm 或 Tray  X-Division=1, In Out Arm需要小於50%
    bool bE65_ClearTrayDataWhenOutArmDestoryErr;                                //Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料
    bool bE66_LogHotPlateAction;                                                //Steven 20211104 : 紀錄加熱盤的動作
    bool bE67_LoadPickerrorMoveWaitpos;                                         //kevin 20220723  : Load Pick up error Move wait pos
    bool bE68RecheckInOutArmICFallDown;                                         //Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警
    int iE68RecheckInOutArmICFallDown;                                          //Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警
    bool bE69_PickupErrorPlacement;                                             //JimmyChiu 20220908 add Pickup Error Placement
    bool bE70_UseTrayThickAdjustZHeight;                                        //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
    bool bE71_10TrayPitchLockTrayAssign;                                        //Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto
    bool bE72_InarmPickICNeedWaitSH;                                            //KenHsieh 20230614 : Inarm pick IC from tray需待SH到位且為可取放料狀態
    bool bE73_InOutZStepMotorLossCheck;                                         //JerryYang 20240111 : add
    int  iE73StepMotorCheckCnt;
    bool bE74_InspectArmPosition;                                               //Jimmychiu 20240408 : debug for inarm position
    bool bE78OneByOneWhenPickErrAtLoader;                                       //Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader.
    bool bE77_OutamrCMotion;
    bool bE30_1InArmUseDifferentScale_Hot;                                      //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature -->
    bool bE31_1OutArmUseDifferentScale_Hot;
    bool bE32_1ShuttleUseDifferentScale_Hot;
    bool bE30_2InArmUseDifferentScale_Cold;
    bool bE31_2OutArmUseDifferentScale_Cold;
    bool bE32_2ShuttleUseDifferentScale_Cold;
    double dTrayXScale_Hot[eTrayCount];
    double dTrayYScale_Hot[eTrayCount];
    double dTrayXScale_Cold[eTrayCount];
    double dTrayYScale_Cold[eTrayCount];                                        //Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <--
    bool bE85_FillTray_Enable;                                                  //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    bool bE85_FillTray_Auto1;                                                   //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    bool bE85_FillTray_Auto3;                                                   //Jimmychiu 20240726 : Fill The Tray After Out Arm Place
    bool bE90_OutArmFixFullExtraY;                                              //Steven 20260428 : Fix Tray Full retreat OutArm Y by -15000 for back-door tray pickup
    //[F]---------------------------
    bool bF01ShakeShuttleWhenJam;
    int  iF01ShuttleShakeSpeed;
    bool bF03OutputShuttleSkipICMiss;
    bool bF05EnableShtPurgeFunction;
    int  iF05ShuttlePurgeCount;
    bool bF06InitialICCheck;
    bool bF06_Active;                                                           //Steven 20140627 : Add for ASE-CL -- F06 打勾
    bool bF06_Enable;                                                           //Steven 20140627 : Add for ASE-CL -- F06 Enable
    int  iF07OutShuttleSensorMode;
    bool bF09CheckICWhichFirstTimeLoad;
    bool bF11_Active;                                                           //Sam 20240202 : 新增 F11 Lock by file 功能
    bool bF11_Enable;                                                           //Sam 20240202 : 新增 F11 Lock by file 功能
    bool bF11OutShtUseFrontRearSensor;
    bool bF14KnockShuttle;                                                      //Steven 20120801 : Shuttle敲敲
    double dF14KnockShuttleInterval;                                            //Steven 20120801 : Shuttle敲敲
    int  iF14KnockShuttleNo;                                                    //wei 20121206
    bool bF14_1KnockShuttleFirst;                                               //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    double dF14KnockShuttleIntervalFirst;                                       //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    int  iF14KnockShuttleNoFirst;                                               //jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊
    bool bF15OutShuttleLoseICNeedPWD;                                           //ChungHung 20120912 Amkor 需求Shuttle lose ic need password
    bool bF16CheckShuttleSensorBroken;                                          //2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線
    bool bF17Sht1First;                                                         //Steven 20160926 : One cycle後要先跑蝦頭一
    bool bF18InshuttleDetect;                                                   //kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic
    bool bF19OutShuttleLoseICNeedPiggyback;                                     //Steven 20150709 : Out shuttle lose IC要做Piggyback
    bool bF20InShuttleProminentDetect;                                          //Alick 20160815 add for 力成 Input Shuttle 檢測IC是否放到外圍，避免壓壞
    bool bF21InOutArmZMotorPrivate;                                             //kevin 20161005 shuttle移動時判斷 In Out Arm Z軸在Home sensor
    bool bF22InShuttleDetectOutNoIC;                                            //kevin 20161108 shuttle 出來撿測有無IC殘留
    bool bF23ShuttleVibration;                                                  //JerryYang 20171006 (wei) Shuttle 震動馬達
    int  iF23ShuttleVibrationTime;                                              //JerryYang 20171205 (Steven) shuttle震動馬達功能
    int  iF23ShuttleVibrationCount;
    bool bF24OutShuttleLoseIcOpenIndexDoor;                                     //kevin 20180725 (wei) add out shuttle lose IC push Z1 open index door
    bool bF25VibrateForOutShuttle;
    int  iF25VibrateTime;
    int  iF26OutShuttleJamSelectSkipOrRetry;                                    //KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry
    bool bF26_Enable;                                                           //Sam 20220527 : for 矽格湖口 -- F26 Enable
    bool bF27OutShtLoseICNeedToPick;                                            //Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料
    bool bF28IndexCheckShuttlePos;                                              //kevin 20220512 add  Index check shuttle pos for Sensor
    bool bF29AlwaysVibrateOnShuttle;                                            //Sam 20210602 : 每次都要強制震動                                       //Steven 20230308 : A26 --> A29
    bool bF30InShuttleSensorFollow16site;                                       //JerryYang 20210426 : 2x4, 2x6 follow 2x8 shuttle sensor位置
    bool bF31_CheckShtMoveCnt;                                                  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    int iShtMoveCntSet[2];                                                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    int iShtMoveCntNow[2];                                                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    int iShtMoveCntHis[2];                                                      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    bool bF32CheckInSHSenBypass;                                                //KenHsieh 20230829 : add Check In Shuttle Sensor By pass
    int  iF32InSHSenBypassValue;                                                //KenHsieh 20230829 : add Check In Shuttle Sensor By pass
    bool bF33_Check2DHardware;                                                  //JerryYang 20250220 : 2DID硬體順序檢查功能
    bool bF35OutShtLoseNeedSetErrBin;
    bool bF34OutShtPickErrSetErrBin;
    bool bF36OutShtLoseICResetSetAllToErr;                                      //JerryYang 20250120 : add

    //[G]---------------------------
    bool bG01Show_TestRate;
    bool bG04ShowFailAlarmCount;
    bool bG05ShowSpeedMessage;
    bool bG07MultiColorForFailBin;                                              //Steven 20160310 : 改成有顏色的fail bin
    bool bG08VisibleAreyousure;                                                 //wei 20160413 顯示Are you sure
    bool bG09NeedPasswordWhenEditSiteMap;                                       //JerryYang 20160425 修改Site map需要密碼
    bool bG10ShowImmediateUPH;                                                  //Steven 20160727 : Show immediate UPH
    bool bG11ASEReport;                                                         //kevin 20170306
    bool bShowUPH;
    bool bShowIndexTime;
    bool bShowTimeInfo;
    bool bShowContactHeight;
    bool bShowLoaderCT;
    bool bShowContactCT;
    bool bShowTestCate;
    bool bShowScanCate;
    bool bShowTemper;
    bool bShowBinCT;
    int  iShowCateByArm;
    bool bG12ContractModeManualMessage;                                         //kevin 20180221 (Steven) Arm 1 Arm2 吸取IC 做CONTRACT MODE
    bool bG13ShowTempOffsetOnContact;
    bool bG14UseStartSoundAlarm;                                                //kevin 20201116  Start 發出聲音 不動 5sec
    int  iG14StartWarrTime;
    bool bG15LoadInputCount;                                                    //kevin 20211106 輸入顆數達成就 Clean out
    int  iG15KeyInTotal;                                                        //kevin 20211106 輸入抽測數量     //Steven 20230308 : 沒用到Mark
    bool bG16BinDispNeedAlarm;                                                  //Steven 20211130 : JSCC要求Bin顯示器異常要alarm
    bool b17bUseLoadCCDTrayMap;                                                 //kevin 20220610 ASE_KH call load CCD 拍照
    bool b17bUseLoadCCDTrayMapTrayend;                                          //kevin 20230530 trayend 拍照
    bool b18bUseAutoTrayMap;                                                    //kevin 20220610 AUTO 123 先放空盤，數量
    bool b18bDoubleUnloadTray;                                                  //kevin 20220610 add Unload Tray 2  倍 的設定
    bool bG22NoticeTakeoutTray;                                                 //JerryYang 20231218 : G22提醒人員取tray功能
    bool bG23DiasbleFuncStatusView;                                             //RogerYang 20250728 Vtest設開關決定要不要顯示View
    bool bG24DisableSECSGEMStatus;                                              //RogerYang 20251222 : 丁曉東要求新增選項(G24)是否顯示
    int  iUnloaderTrayCount[3];                                                 //kevin 20220610 add  Auto 123 判斷滿TRAY 補空TRAY
    bool b19InSHAutoCleanPA;                                                    //kevin 20230530 add Autoclean shuttle define new 參數
    bool b20FixfullWaitpos;                                                     //kevin 20230816 out arm move Wait
    bool b21colorfulltray;                                                      //kevin 20230918 colortray no Alarm
    //[I]---------------------------
    bool bI01TesterFinishThenHome;
    bool bI02HomeSetSocketICToErrBin;                                           //JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin
    bool bI02_Enable;                                                           //JerryYang 20260504 : for SPIL -- I02 Enable (Lock by File)
    bool bI03AmbientTempControl;                                                //kevin 20140918 常溫恆溫控制
    bool bI04EnableChangeBinDuringTesting;
    bool bI05LowYieldForcedOneCycle;                                            //JerryYang 20190709 黃剛要求low yield不要強制one cycle
    bool bI06_Active;                                                           //Sam 20220527 : for 矽格北興 -- I06打勾
    bool bI06_Enable;                                                           //Sam 20220527 : for 矽格北興 -- I06 Enable
    bool bI06TurnOnI01AfterHome;                                                //Sam 20220216 : Home 完成後強制開啟 I01 Function
    bool bI07ResetGPIBAfterOneCycleCleanOut;
    bool bI08Check2DIDEnableWhenInitialStart;                                   //Steven 20190412 : Initial Start時檢查有沒有開啟2DID
    bool bI09LowYieldOneCycleDontCleanShuttle;                                  //JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH)
    bool bI12TesterTimerOutNotNeedReTest;
    bool bI16TTLSaveInSetupFile;                                                //Steven 20180626 (wei) : TTL設定存到工作檔裡面
    bool bI13InitStartDelayHasFTandRT;                                          //Steven 20190313 : Initial Start Delay use different setting in FT and RT
    bool bI18CanReceiveEchoStop;                                                //ChungHung 20120326 add for ASE_KR
    bool bI19AuToSitMapPauseWaitBin;                                            //kevin 20150122 AutoSiteMAP 模式 用gpib模擬器 暫停等待改bin別
    int  iI20ErrorBinAlphabet;
    bool bI21EnableASM;                                                         //是否啟用Auto Site Mapping
    int  iI21FailRetryCount;                                                    //KenHsieh 20251002 : add auto site mapping fail retrey Func.
    bool bI21SkipSoakTime;                                                      //[I21] Auto Site Mapping Soak Time-------------------
    bool bI21UseSameSoakTime;
    double fI21UseSameSoakTime;
    bool bI21ASMNeedCheckEachSiteOpen;                                          //Steven 20120726 : AutoSiteMapping, 當確認Open Bin時,同時也要檢查是不是所有Dut都Open
    bool bI21ASMRemoveLTrayManually;                                            //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray
    bool bI21ASMRunTimeCHeck;                                                   //Steven 20140729 : AutoSiteMapping, 邊生產邊做
    bool bI21AutoSiteMappingUseHotplate;                                        //Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode
    bool bI21AutoSiteMappingFailBinSetting;                                     //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting
    int  iI21UseFailBinSetting;                                                 //Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting
    int  iI21AutoSiteMappingErrCT;                                              //jou 20200707 : VTEST auto site mapping
    bool bI21RTmodeDonotRunSiteMapping;                                         //Richard 20230427 : RT mode不跑sitemapping
    bool bI22TimeOutCanSkip;                                                    //Steven 20111220 : 測試TimeOut可以Skip
    int  iI22TestTimeOutOption;                                                 //Steven 20181226 : 測試Time out可以按的按鈕
    double fI22HomeDelay;
    bool bI23HotTestWaitingMode;                                                //ChungHung 20111230 Hot Test Waiting Mode
    bool bI24TestingNeedStopAllMotor;                                           //jou 2013-09-25 Testing Need Stop All Motor
    bool bI26TestCloseSiteHaveBin;                                              //kevin 20150202 測試時沒有 ic 出現bin資料或bin別沒設定需取出ic
    bool bI27_ManualSortMode;                                                   //Steven 20150915 : For TSMC 手動整盤功能
    bool bI28_OnOffSiteOnTheFly;                                                //Steven 20150924 : 隨時開關Site功能
    bool bI29EnableYieldRecord;                                                 //Ifor 20151221 :新增 Yield Record Function
    bool bI29_1SaveYieldBySocketByBin;                                          //Steven 20171108 (wei) : By Socket By Bin存檔
    double fI29YieldRecordInterval;                                             //Ifor 20151221 :新增 Yield Record Interval Time
    bool bI29YieldRecordIntervalIC;                                             //Sam 20231106 : 紀錄 Total yield
    int iI29YieldRecordIntervalIC;                                              //Sam 20231106 : 紀錄 Total yield
    bool bI30ContFailBin;                                                       //kevin 20160407 Fail bin 超過數量發警告訊息
    bool bI31_1GPIBLotEnd;                                                      //wei 20160726 TSMC GPIB Lot End
    bool bI31_2GPIBLotStart;                                                    //wei 20170607 GPIB Lot Start Command
    bool bI31_3GPIBReset;                                                       //wei 20170918 Reset Command
    bool bI32CanCelErrorBin;                                                    //kevin 20160802 取消error bin 要手到在 outshuttle 取出
    bool bI33ErrorBinBox;                                                       //kevin 20160819 error bin 要放到 Bin Box
    bool bI34AllSiteAreSameFailBinShowAlarm;                                    //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
    bool bI35UseThirdSiteControlByEngineer;                                     //Alick 20160926 add for 第三組工程師用開關SITE
    bool bI36TestTimeOut;                                                       //kevin 20161105 沒有收到測試資料需手動取出ic
    bool bI37_EnableFIFOMode;                                                   //Steven 20170302 (wei) : FIFO MODE
    bool bI37_EnableFIFOSiteOrder;                                              //Steven 20170302 (wei) : FIFO MODE
    bool bI37_LockLoaderDirection;                                              //Steven 20170302 (wei) : FIFO MODE
    int  iI37_LockLoaderDirection;                                              //Steven 20170302 (wei) : FIFO MODE
//    bool bI37_OneByOneTest;                                                     //Steven 20180305 : 一次跑一顆的FIFO版本
    bool iI38SETTEMPRespondSetTemp;                                             //kevin 20180308 Settemp? 回傳需要 Settemp +25.0.
    bool bI39SpiroxTesterLotEnd;                                                //JerryYang 20170515 (wei) JCET通知tester lot end command
    bool bI40_bStartProductOnLine;                                              //kevin 20180517 生產前OP OFF_LINE 強制 On line
    bool bOurArmDropICSkip;                                                     //kevin 20171005 (wei) out arm drop ic 只能強至取出ic 開6號門

    //Steven 20201022 : For RFMD Empty Socket Check Funstion.
    //==>
    bool bI41EnableEmptySocketCheck;
    bool bI41_1_StartOfLot;
    bool bI41_2_OpenChamberDoor;
    bool bI41_3_AfterContactorTeminated;
    bool bI41_4_AfterContactorJam;
    bool bI41_5_RegularExecutionCycle;
    bool bI41_6_Manual;
    int  iI41_5_RegularExecutionCycleCount;
    int  iI41_BinOfESC;                                                         //Steven 20220817 : Bin of ESC function //Steven 20230308 : I44 --> I41
    //<==
    //Steven 20201022 : For RFMD Empty Socket Check Funstion.
    bool bI42_bEnableBarcodeFlowErr;                                            //Ifor 20211116 add: Use Barcode Flow Err Check

    bool bI43ResetGPIBAfterTrayFeedFinish;                                      //Sam 20211107 : 新增 Tray Feed Finish Reset GPIB
    bool bI44_LowYieldAlarmIntervalTimeBySetting;                               //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
    int  iI44_LowYieldAlarmIntervalTime;                                        //JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間
    bool bI45_Use2DIDSort;                                                      //KenHsieh 20230607 : ASEKH add 2D SORT
    int  iI46_ActionWhenGpibFlowErr;                                            //Steven 20231017 : GPIB flow error need alarm

    bool bI49_TesterTimeOutResetAllIC;                                          //Sam 20240215 : Tester time out show reset all ic
    double fI49_ChangeAboveSocket;                                              //Sam 20240215 : Tester time out show reset all ic

    //Jimmychiu 20230707 : Auto Site Mapping Trigger Function
    //==>
    bool bI50_EnableAutoSiteMappingTrigger;
    bool bI50_StartLot;
    bool bI50_InitialStart;
    bool bI50_OnyCycle;
    bool bI50_Pause;
    bool bI50_RT;                                                               //Steven 20231031 : RT不做ASM
    bool bI50_TrayFeed;
    //<==
    //Jimmychiu 20230707 : Auto Site Mapping Trigger Function
    bool bI51_bNotSetErrBinForInput;
    bool bI52_bAQLSortMode;
    bool bI53_bKLTInitial;
    double fI53KLTInitialInterval;                                              //Ifor 20230210 add:新增 KLT 要求Onecycle Finish未超過設定時間不執行KL initial delay
    bool bI54_Enable;                                                           //Jimmychiu 20240916 : Check the temperature during index arm testing
    bool bI54_1_AllICErr;                                                       //Jimmychiu 20240916 : Check the temperature during index arm testing
    bool bI54_2_AbnormalICErr;                                                  //Jimmychiu 20240916 : Check the temperature during index arm testing

    //[L]---------------------------
    bool bL03SocketAirCoolingCT;                                                //jou 2016-04-28 Socket Air Cooling contact count trun on
    int  iL03SocketAirCoolingCT;                                                //jou 2016-04-28 Socket Air Cooling contact count trun on
    int  iL04TemptureRange;
    int  iL05ChamberTemptureRange;
    bool bL07UseSingleTenmpertureLimit;
    int iL09_1HightTemp_Sht_Shift;
    int iL09_2LowTemp_Sht_Shift;                                                //Ztex 2024.10.01 Add AStream Error Compress Onecycle
    bool bL09HotTempShuttleNoAddPos;                                            //kevin 20200718 : ASEKH 高溫shuttle不補位置  //JerryYang 20230204 : L23 -> L09
    int  iL10TempRecordInterval;
     //Ifor 20150911 新增ATC 突波於設定範圍內且持續發生超過設定時間後發出Alarm && 突波超出最大設定值發出Alarm && Sensor1與Sensor2 差值大於設定值發出Alarm
    int  iATCTemperatureOutside;                                                //Ifor 20150911 : [L11-6] ATC 溫度超出設定值
    int  iATCTemperatureContinuous;                                             //Ifor 20150911 : [L11-6] ATC 溫度持續超出設定值時間
    int  iATCMaxSurgeAlarm;                                                     //Ifor 20150911 : [L11-7] ATC 突波警報最大容許範圍
    int  iATCMaxAlarmContinuous;                                                //Ifor 20200803 add:Hisi V2.4 最大峰值Alarm計時
    bool bL10IndexTestlogTemp;                                                  //kevin 20190323 : index 測試時才記錄溫度
    bool bL11_6ATCUseTemperatureOutsideAlarm;                                   //Ifor 20150911 : [L11-6] ATC突波於設定範圍內且持續發生超過設定時間後發出Alarm
    bool bL11_7ATCUseMaxSurgeAlarm;                                             //Ifor 20150911 : [L11-7] ATC突波超出最大設定值發出AlarmedL11_6_Outside
    bool bL11_8ATCUseTemperatureCompare;                                        //Ifor 20150911 : [L11-8] ATC第1組sensor跟第2組sensor溫度差值超過設定值發出警報edL11_6_Continuous
    bool bL11_2ATCChillerProtectedFunction;                                     // 2012.05.07 , Joye , Chiller
    bool bL11_1ATCTemperatureOverAlarm;                                         // 2012.05.11 , Joye , ATC Temperature Over Check
    int  iATCChillerCheckTime;                                                  // 2012.05.07 , Joye , Chiller
    int  iATCTemperatureRange;                                                  // 2012.05.11 , Joye , ATC Temperature Over Check
    double dATCTemperatureCheckTime;                                            //Steven 20121222 : ATC Temperature Over Check Time
    double dATCAmbientTemperature;                                              //Steven 20130122 : [L11] ATC常溫的溫度
    double dHeatGunTempATC;                                                     //JerryYang 20220408 : add for ATC3.5

    bool bL12TempErrNoCloseHeater;                                              //ChungHung 20120913 SCK  要求Temp Error 不要關閉加熱
    bool bL13HotPlateAndShuttleUseOneTempOffset;                                //Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案
    bool bL15EnableChamberModeEvenBlowNeedWaitTime;                             //ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set
    bool bL17HeadHeaterOnWhenCloseSite;                                         //Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK)
    bool bL18NofullsiteaddTemperatureoffset;                                    //wei 20160107 No FullSite Add Offset
    bool bL19OpenHeatDoorgiveupchamberhot;                                      //kevin 20170520 (wei) 開chambo門只有不加熱chambo
    bool bL20AbientGuardBand;                                                   //kevin 20180115 (Steven) add Amient Guard Band
    bool bL21PowerOffTemperature;                                               //kevin 20181112 (Steven) : power off 開 Chamber door 斷所有加熱電
    bool bL22Enable3SigmaTempMonitor;                                           //kevin 20200521 : ASEKH 3SIGMA  溫度統計
//    bool bL23HotTempShuttlenoAddPos;                                          //kevin 20200718 : ASEKH 高溫shuttle不補位置  //JerryYang 20230204 : L23 -> L09
    bool bL24HeaterStableTime;                                                  //JerryYang 20210122 : ASE-CL新增待溫功能
    int  iL24HeaterStableTime;                                                  //JerryYang 20210122 : ASE-CL新增待溫功能
    bool bL25_1ATCFileNameWithTemp;
    bool bL28TempOfsUseReadyTempRange;                                          //Sam 20231214 : Temp offset use ready temp range
    bool bL29AmbientNotShowTemp;                                                //Sam 20221101 : 常溫模式不顯示溫度
    bool bL30Use1CableLayoutKitByConfig;                                        //Sam 20210715 : Use 1CableLayoutKit By Config
    bool bL30Use1CableLayoutKit;                                                //Sam 20210715 : Use 1CableLayoutKit By Config
//    bool bL19UseNoHeatMode;                                                   //Ifor 20160511   //Steven 20160816 : mark

    //Ztex 2023.04.17 Add HT-1032 Configuation ==>
    bool bL32_1ManuDefrost;
    bool bL32_2AutoDefrostFunction;
    bool bL32_3OneCycleDefrost;
    int  iL32_4SetDefrostTemp;                                                  //除霜的門檻溫度
    int  iL32_5SetDefrostTime;                                                  //除霜的時間
    int  iL32_6LowTempRunAlarmDegree;
    int  iL32_7LowTempRunAlarmHour;
    int  iL32_7LowTempRunAlarmMin;
    int  iL32_8SetAirStreamTemp;
    bool bL33_1CheckDoorOpenForTriTemp;
    bool bL33_2DoorOpenRunDefrost;
    int  iL33_3BDoorOpenTimeForLowTemp;
    int  iL33_4SDoorOpenTimeForLowTemp;
    int  iL33_5DoorOpenTempForLowTemp;
    int  iL33_6DoorOpenTempForHotTemp;
    bool bL34_1DelayOfFixDoorOpen;
    int  iL34_2DelaySecOfFixDoorOpen;
    double dL34_3DewPointOfFixDoorOpen;
    int  iL34_4OpenAuto3TrackGateSec;
    bool bL34_5FixTrayDoorCynAutoOpen;
    bool bL35_1OverSetTempOpenFan;
    int  iL35_2OpenFanTemp;
    bool bL35_3NotUseVaccumCheckKit;
    int iL36_1Tri_Temp_Rang_ATC;
    int iL36_2Tri_Temp_Rang_Heater;
    bool bL37UnDockTurnOffAir;
    bool bL38_1AirCoolerToCoolDown_SetEnable;                                   //Use air cooler to cool down
    int  iL38_2AirCoolerToCoolDown_SetTemperature;
    bool bL39_1AutoRunWhenTempOk;
    bool bL39_2WaitTempstabilize;
    int  iL39_2WaitTempstabilize;
    int  iL40ImmediateTempExceedsAlarm;
    int  iL41TemperatureAlarmSecond;
    int  bL40_2UnDockTurnOffAirDelay;
    int  iL40_3AirStreamRang;
    bool bL42_UseOutShuttleDesoakTime;
    int  iL42_UseOutShuttleDesoakTime;
    bool bL43EnableATCPowerFollow;                                              //KenHsieh 20240216 : add ATC Power Follow Function
    bool bL44_SetColdAirSwitchTemp;
    int  iL44_SetColdAirSwitchTemp;
    bool bL45_SetDewPointOffset;                                                //Ztex 2025.04.01 Add Set Dew Point Offset
    int  iL45_SetDewPointOffset;                                                //Ztex 2025.04.01 Add Set Dew Point Offset
    bool bL46_AStreamErrorCompressOnecycle;
    //Ztex 2023.04.17 Add HT-1032 Configuation <==

    //[M]---------------------------
    bool bM01EnableMonitorFunction;                                             //ChungHung 20131009 add for SCK
    bool bM0101ContactModeUseDifferentSpeed;                                    //ChungHung 20131009 add for SCK
    bool bM0102SiteYieldDifferentMustOn;                                        //ChungHung 20131009 add for SCK
    bool bM0103ContinueFailBySocketMustOn;                                      //ChungHung 20131009 add for SCK
    bool bM0104ContinueFailByHeadMustOn;                                        //ChungHung 20131009 add for SCK
    bool bM0105InOutArmDeviceCheckMustOn;                                       //ChungHung 20131009 add for SCK
    bool bM0106IndexDeviceCheckDestoryMustOn;                                   //ChungHung 20131009 add for SCK
    bool bM0107AutoSpeedMustOn;                                                 //ChungHung 20131009 add for SCK
    bool bM0108EveryFirstDeviceMustOn;                                          //ChungHung 20141210 add for SCK want to add Monitor every first device have delay time
    bool bM0109RTCOffCheckYieldPiggyBack;                                       //ChungHung 20150613 add for SCK want to check and show message
    bool bM1010Disable_I12;                                                     //Steven 20160727 : For SCK
//    bool bM1011Enable_I22;                                                    //Steven 20160727 : For SCK
    bool bM1012EnableAutoClean;                                                 //Steven 20160727 : For SCK
    bool bM1013Enable2DID;                                                      //Steven 20160727 : For SCK
    bool bM1014EnableATC;                                                       //Steven 20191129 : For SCK
    bool bM1015EnableBottom2DID;                                                //JerryYang 20250120 : add

    //[N]---------------------------
    bool bN05SCKWebService;                                                     //Steven 20161201 : For SCK Web Service
    bool bN05_RTCalarmUnload;                                                   //jou 20170210 (Steven) : RTC alarm image unload
    AnsiString asN05_RTCalarmUnload;                                            //jou 20170210 (Steven) : RTC alarm image unload
    int iN05_UpDLMethod;                                                        //Jimmychiu 20250707 : add RMS connect method
    //ChungHung 2011/01/17 start
    //這是KYEC的FTP功能
    bool       bEnableFTP;                                                      //Steven 20110216 : 加一個開關
    AnsiString FtpUserName;
    AnsiString FtpPassword;
    AnsiString FtpHost;
    AnsiString N06_FtpPort;                                                     //Ifor 20201015 add:使用者自定義 FTP Port
    AnsiString FtpDownloadPath;
    AnsiString FtpUplaodPath;
    int        FtpTransMode;                                                    //Steven 20230719 : 加入FTP傳輸模式
    int        TasterInputMethod;                                               //Steven 20110311 : Taster輸入方式
    AnsiString N06_TasterListFile;                                              //Steven 20110305 : Taster名稱對照表
    AnsiString N06_TasterListMap;                                               //Steven 20121018 : Handler與測試機連線的IP
    AnsiString TasterType;                                                      //Steven 20110305 : Taster型號
    AnsiString TasterNo;                                                        //Steven 20110305 : Taster號碼
    AnsiString TasterName;                                                      //Steven 20110311 : Taster名稱
    bool       FtpUseSystemCallToUnZip;                                         //Steven 20140609
    AnsiString FtpPasswordDownloadPath;                                         //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
    bool       bFtpPasswordDownload;                                            //Sam 20210526 : 從 N06 DownloadPath 下載密碼本
    int        iHDEnable;
    int        iServerEnable;
    AnsiString asN06_FileName;                                                  //RogerYang 20170406 (Steven) for 力成 Jam Alarm Data 帶入自定義檔案名稱
    bool       bN06_UseBarcode;                                                 //Ifor 20210421 add: Use
    bool       bN06_CopyTesterFile;                                             //Steven 20250327 : OS測試機的工作檔也要上傳
    AnsiString asN06_TesterPath;                                                //Steven 20250327 : OS測試機的工作檔也要上傳
    //ChungHung 2011/01/17 end

    bool bN07_EnableEmployeeIdCheak;                                            //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
    int  iN07_EmployeeIdCheakTime;                                              //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
    int  iN07RunCheckAlarmTime;                                                 //wei 20150512  Run Check Alarm Time
    bool bN07_EnableSecsLotCheck;
    bool bN07_6EnableUploadOSRecipe;                                            //Steven 20230710 : OS測試機的工作檔也要上傳
    bool bN07_6CompressedFile;                                                  //JimmyChiu 20250214 : OS測試機的工作檔上傳選擇要不要壓縮
    bool bN07_7SendRecipeAsBinary;                                              //Steven 20230710 : 工作檔使用二進制上傳下載
    double dN07_7_DelayTime;
    bool bA81WaitSECS;                                                          //KevinCheng 20250919 : Wait SECS

    AnsiString sN07_6OSRecipePath;                                              //Steven 20230710 : OS測試機的工作檔也要上傳

    bool bN08_1SaveOLPLog;                                                      //Steven 20141229 : OLP的Log要存檔
    AnsiString sN08OlpIP;                                                       //Sam 20190429 : Add CC_PTI_NEWWORK
    AnsiString sN08OlpPort;                                                     //Sam 20190429 : Add CC_PTI_NEWWORK

    bool        bN09_LotCountAutoFunc;                                          //Steven 20190521 : ATK lot count
    AnsiString  sN09_HandlerFolder;
//    AnsiString sN09_TesterFolder;
    double      dN09_SearchTime;
    int         iN09_4_UploadMethod;
    int         iN09_TSV_Port;
    bool        bN09_Enable_TSV;                                                //Steven 20231017 : add for ATK
    AnsiString  sN09_5_User;
    AnsiString  sN09_5_Password;
    AnsiString  sN09_5_Host;
    AnsiString  sN09_5_Path;
    bool        bN09_7_SkipIP;
    AnsiString  sN09_7_SkipIP;

    int  iN10UploadMethod;                                                      //Steven 20190119 : Log上傳方式改為可選擇的
    int  iN10UploadProductMethod;                                               //JerryYang 20190131 上傳production log可選擇00:00 or 08:00
    double dN10_3_1_SpecifiedTime;                                              //Jimmychiu 20250912 : CYUEAN wants to upload the log at a specified time.
    AnsiString sN10UploadDrivePath;
    AnsiString cN10FtpUserName;
    AnsiString cN10FtpPassword;
    AnsiString cN10FtpHost;
    AnsiString cN10FtpUplaodPath;
    int  iN10FtpPort;
    bool bN10FtpPassive;
    bool bN10_3_UpLoadByLot;                                                    //Steven 20250527 : Upload data by lot.
    bool bN10Enable_FTPUpLoadLog;                                               //Ifor 20160302 add Enable FTP Up Load Log
    bool bN10_UploadSummaryToFTP;                                               //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
    int  iN10UploadToHostIntervalTime;                                          //Ifor 20160302 add Enable FTP Up Load Log
    int  iN10DataType;                                                          //Sam 20170823 (wei) : 矽格中興 FTP Log 上傳增加時間格式選擇 0:yyyy 1:yyyymm 2:yyyymmdd
    bool bN10_DailyUploadProdData;                                              //Steven 20180514 : JCET吳如春要求每日上傳Event Log, Jam統計表, MTBF, MUBF資料
    bool bN10_9_UploadUnloadTrayToFTP;                                          //Steven 20200409 : production log by unloader tray存檔並上傳FTP
    bool bN10_11_Enable_UploadFTPEventLog;                                      //Jimmychiu 20251112 : add upload eventlog to FTP server
    bool bN10_12_Enable_UploadFTPGPIBLog;                                       //Jimmychiu 20251112 : add upload GPIBLog to FTP server
    bool bN11_1CleanOutCloseSite;                                               //kevin 20160802 ASE send command Clean out close Site

    bool bN12_EnableSocketIdProductDataFTP;                                     //Sam 20170525 力成 add Socket ID Product Data Upload To FTP
    AnsiString asN12_FtpUserName;                                               //Sam 20170525 力成 add Socket ID Product Data Upload To FTP
    AnsiString asN12_FtpPassword;                                               //Sam 20170525 力成 add Socket ID Product Data Upload To FTP
    AnsiString asN12_FtpHost;                                                   //Sam 20170525 力成 add Socket ID Product Data Upload To FTP
    AnsiString asN12_FtpUplaodPath;                                             //Sam 20170525 力成 add Socket ID Product Data Upload To FTP

    bool bN13_EnableARMSFunction;                                               //Ifor 20170621 (wei) add ARMS Function

    //Sam 20170603 (Steven) 移植超豐 OEE 功能 form HT-7045
    //==>
    bool        bN14_1_EnableOEEFunction;
    int         iN14_1_OEERecordCycleTime;
    bool        bN14_2_OEEUseSaveProdData;
    AnsiString  asN14_2_OEESaveProdPath;
    bool        bN14_3_OEEFTPUpload;
    AnsiString  asN14_3_OEEFTPUserName;
    AnsiString  asN14_3_OEEFTPPassword;
    AnsiString  asN14_3_OEEFTPHost;
    AnsiString  asN14_3_OEEFTPUploadPath;
    bool        bN14_4_OEEAutoLoadMOFile;
    AnsiString  asN14_4_MODownloadPath;
    int         iN14_5_PauseIntervalTime;
    int         iN14_6_BySiteContactCnt;
    double      dN14_6_BySiteLowYieldRate;
    double      dN14_6_BySiteCmpYield;
    int         iN14_6_BySiteAlarmYieldRate;
    bool        bN14_7_AutoMotive;
    AnsiString  asN14_7_AutoMotivePath;
    bool        bN14_8_ULSetup;
    AnsiString  asN14_8_ULSetupPath;
    bool        bN14_9_ULQtyReport;
    AnsiString  asN14_9_ULQtyReportPath;
    bool        bN14_10_DownFileByMO;
    bool        bN14_11_CheckSiteMapByMO;
    bool        bN14_12_ULTempLogToFTP;
    int         iN14_12_ULTempLogInterval;
    AnsiString  asN14_12_ULTempLogPath;
    bool        bN14_13_ULBinQtyToFTP;
    int         iN14_13_ULBinQtyInterval;
    AnsiString  asN14_13_ULBinQtyPath;
    bool        bN14_14_AlarmCtrlMachine;
    AnsiString  asN14_14_ExecutFilePath;
    AnsiString  asN14_14_MessageFilePath;
    AnsiString  asN14_14_FlagFilePath;
    bool        bN14_15_SocketLifeTime;
    AnsiString  asN14_15_ExecutFilePath;
    AnsiString  asN14_15_MessageFilePath;
    bool        bN14_16_EnableIPSC;
    AnsiString  asN14_16_ExecutFilePath;
    AnsiString  asN14_16_FlagFilePath;
    AnsiString  asN14_16_ProductionFilePath;
    int         iN14_16_IPSCInterval;
    int         iN14_17_AmbientULTemp;                                          //Sam 20200525 : 正賢要求溫度 by Config 以下都是常溫。
    bool        bN14_18_EnableTempOffset;                                       //Sam 20200806 : 溫度 By Servo
    AnsiString  asN14_18_TempOffsetPath;                                        //Sam 20200806 : 溫度 By Servo
    bool        bN14_19_TrayMappingToFTP;                                       //Sam 20201209 : 增加資料上傳
    AnsiString  asN14_19_TrayMappingPath;                                       //Sam 20201209 : 增加資料上傳
    bool        bN14_20_DefaultRecipeChangeLog;                                 //Sam 20201209 : Default Recipe ChangeLog
    AnsiString  asN14_20_ChangeLogPath;                                         //Sam 20201209 : Default Recipe ChangeLog
    bool        bN14_20_DefaultRecipeChangeLogCycleRecord;                      //Sam 20201209 : Default Recipe ChangeLog
    int         iN14_20_CycleTime;                                              //Sam 20201209 : Default Recipe ChangeLog
    bool        bN14_21_SetUpConfiguration;                                     //JimmyChiu 20220303 : Add class SetUpConfiguration
    AnsiString  asN14_21_SetUpConfiguration;                                    //JimmyChiu 20220303 : Add class SetUpConfiguration
    AnsiString  asN14_21_BinCategory;                                           //JimmyChiu 20221226 : SetUpConfiguration HandlerMode splite to SITE MAP and BIN CATEGORY
    AnsiString  asN14_21_HP_SetUpConfig;                                        //JimmyChiu 20220303 : Add class SetUpConfiguration
    AnsiString  asN14_21_TF_SetUpConfig;                                        //JimmyChiu 20220303 : Add class SetUpConfiguration
    bool        bN14_21_ConfigUpdateFromServerExport;                           //JimmyChiu 20230410 : Config update from server
    AnsiString  asN14_22_ConfigUpdateFromServerExport;                          //JimmyChiu 20230410 : Config update from server
    AnsiString  asN14_22_ConfigUpdateFromServerImport;                          //JimmyChiu 20230410 : Config update from server
    bool        bN14_23_ReadTextFileforPassword;                                //JimmyChiu 20240115 : Read Text File for Password
    bool        bN14_24_DynaMultiContinuPassSocket;                             //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )
    int         iN14_24_DyMultiPassPower;                                       //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )

    bool        bN15UserLevelByTxt;
    AnsiString  asN15UserLevelByTxtReadFilePath;
    bool        bN15UseESDControlMachine;
    AnsiString  asN15UseESDControlMachineReadFilePath;
    AnsiString  asN15ESDFTP_UserName;
    AnsiString  asN15ESDFTP_Password;
    AnsiString  asN15ESDFTP_Host;
    AnsiString  asN15ESDControlMachineSaveRecordFilePath;
    AnsiString  asN15HandlerAUTOMOTIVEDownloadPath;

    bool bEnableOffsetFTP;                                                      //wei 20170821 offset ftp
    AnsiString cN16FtpUserName;
    AnsiString cN16FtpPassword;
    AnsiString cN16FtpHost;
    AnsiString cN16FtpDownloadPath;
    AnsiString cN16FtpUplaodPath;
    //<==
    //Sam 20170603 (Steven) 移植超豐 OEE 功能 form HT-7045

    bool bN17UploadLotSummary;                                                  //JerryYang 20220923 : [N17] upload lot summary
    bool bN17UploadProdLog;
    bool bN17UploadSwVersion;                                                   //KevinCheng 20251120 : N17 SWVersion

    AnsiString sN19_5_User;
    AnsiString sN19_5_Password;
    AnsiString sN19_5_Host;
    AnsiString sN19_5_Path;
    bool bN19_7_SkipIP;
    AnsiString sN19_7_SkipIP;

    //Sam 20230511 : 機台資料變更後須上傳 FTP
    bool bN21_HandlerChangeStateUploadServer;
    AnsiString sN21_FTPUserName;
    AnsiString sN21_FTPPassword;
    AnsiString sN21_FTPHost;
    AnsiString sN21_FTPUploadPath;

    bool bN20_CheckMD5;                                                         //Steven 20170927 (wei) : 比對工作檔的檢查碼是否正確
    bool bN22Enable_EventLog;                                                   //Steven 20181224 : For ASE-CL
    bool bN22Enable_ASE_CL_FTP;
    AnsiString sN22ASE_CL_FTPUserName;
    AnsiString sN22ASE_CL_FTPPassword;
    AnsiString sN22ASE_CL_FTPHost;
    AnsiString sN22ASE_CL_FTPDownlaodPath;

    bool bN22_1_HANA_TrayMapFTP;
    AnsiString sN22_1_FTPUserName;
    AnsiString sN22_1_FTPPassword;
    AnsiString sN22_1_FTPHost;
    AnsiString sN22_1_FTPUploadPath;

    //Steven 20200409 : Murata 2DID比對功能
    //==>
    bool bN23_1_Enable2DIDCompare;
    AnsiString sN23_1_URL;
    AnsiString sN23_2_Line;
    AnsiString sN23_2_Process;
    AnsiString sN23_2_Product;
    bool bN23_3_UploadTestResult;
    AnsiString sN23_3_URL;
    //<==
    //Steven 20200409 : Murata 2DID比對功能

    AnsiString sN22_1ASE_CL_FTPUserName;                                        //JerryYang 20250120 : add
    AnsiString sN22_1ASE_CL_FTPPassword;
    AnsiString sN22_1ASE_CL_FTPHost;
    AnsiString sN22_1ASE_CL_FTPUplaodPath;
    AnsiString sN23_JHT_UploadPath;

    AnsiString sN23_4ASE_CL_FTPUserName;
    AnsiString sN23_4ASE_CL_FTPPassword;
    AnsiString sN23_4ASE_CL_FTPHost;
    AnsiString sN23_4ASE_CL_FTPDownlaodPath;

    bool bN35UploadJHT_Log;
    AnsiString sN35_1ASE_CL_FTPUserName;
    AnsiString sN35_1ASE_CL_FTPPassword;
    AnsiString sN35_1ASE_CL_FTPHost;
    AnsiString sN35_1ASE_CL_FTPUplaodPath;
    AnsiString sN35_JHT_UploadPath;

    //Steven 20200409 : JSCC RTM功能
    //==>
    bool bN24_EnableRTM;
    int  iN24_RTMPort;
    //<==
    //Steven 20200409 : JSCC RTM功能
    bool bN23UploadJHT_Log;                                                     //JerryYang 20250120 : add
    AnsiString asN17LotSummaryPath;                                             //JerryYang 20220923 : [N17] upload lot summary
    AnsiString asN17ProductionLogPath;
    AnsiString asN17SWVersionPath;                                              //KevinCheng 20251120 : N17 SWVersion
    int iN23DownloadMethod;                                                     //JerryYang 20190313 : 2D sorting
    AnsiString cN23FtpUserName;
    AnsiString cN23FtpPassword;
    AnsiString cN23FtpHost;
    AnsiString cN23FtpDownloadPath;
    AnsiString sN23DownloadDrivePath;
    AnsiString sN23LotInfoPath;
    AnsiString sN23_4_URL;                                                      //JerryYang 20241104 : 支援2DID白名單功能
    AnsiString sN23_5_UploadPath;
    bool bN23UseLotInfoFile;                                                    //Steven 20240829 : Lot info從檔案讀取
    bool bN25FolderWithoutYYMM;                                                 //Steven 20240830 : Summary資料夾不要有年月

    //Steven 20210413 : 南茂的自動Start功能
    //==>
    bool bN25_1_EnableStartControl;
    AnsiString sN25_1_FTPUserName;
    AnsiString sN25_1_FTPPassword;
    AnsiString sN25_1_FTPHost;
    AnsiString sN25_1_FTPPath;
    bool bN25_2_EnableUploadLog;
    AnsiString sN25_2_FTPUserName;
    AnsiString sN25_2_FTPPassword;
    AnsiString sN25_2_FTPHost;
    AnsiString sN25_2_FTPPath;
    int iN25_2_UploadInterval;
    bool bN25_3_EnableULJamLog;                                                 //JimmyChiu 20241009 : for 南茂Jam List上傳
    AnsiString sN25_3_JamLogFTPPath;
    bool bN25_4_EnableUpload;                                                   //JimmyChiu 20241009 : for 南茂Jam List上傳
    AnsiString sN25_4_UploadPath;
    bool bN25_5_EnableUpload;                                                   //JimmyChiu 20241009 : for 南茂Jam List上傳
    AnsiString sN25_5_UploadPath;
    //<==
    //Steven 20210413 : 南茂的自動Start功能

   //Sam 20210224 : Auto Upload FTP JAMRawData 功能
    //==>
    bool bN26_UseJamRawDataUpdataToFTP;
    bool bN26_UseJamRawDataRecord;
    AnsiString sN26_FTPUserName;
    AnsiString sN26_FTPPassword;
    AnsiString sN26_FTPHost;
    AnsiString sN26_FTPUplaodPath;
    //<==
    //Sam 20210224 : Auto Upload FTP JAMRawData 功能

    //Sam 20210621 : 新增 Event Log xml  Updata
    //==>
    bool bN27_UseAlarmLogXmlUpdataToFTP;
    AnsiString sN27_FTPUserName;
    AnsiString sN27_FTPPassword;
    AnsiString sN27_FTPHost;
    AnsiString sN27_FTPUplaodPath;
    AnsiString sN27_TesterID;
    //<==
    //Sam 20210621 : 新增 Event Log xml  Updata

    bool bN28_SCK_OEE;
    AnsiString sN28_Path;
    AnsiString sN28_IP;

    bool bN29_ParameterCheckForGMTest;                                          //Steven 20220311 : GM Test工作檔比對功能
    AnsiString sN29_FilePath;

    bool bN30_UseGroundESDUpdataToFTP;                                          //Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。
    AnsiString sN30_FTPUserName;
    AnsiString sN30_FTPPassword;
    AnsiString sN30_FTPHost;
    AnsiString sN30_FTPUploadPath;
    AnsiString sN30_FTPUploadPath2;                                             //Sam 20220816 : GroundESD 新增第二組上傳

    int  iN31_UseAutoTempOfsByFTP;                                              //Sam 20220406 : 溫度自動補償功能 By FTP
    AnsiString sN31_FTPUserName;
    AnsiString sN31_FTPPassword;
    AnsiString sN31_FTPHost;
    AnsiString sN31_FTPDownloadPath;
    int iN31_ContactCnt;
    double dN31_MinOffset;                                                      //Jimmychiu 20241226 : add N31 temp offset limit
    double dN31_MaxOffset;

    bool bN32_DownloadUpdatesAutomatically;                                     //Sam 20220824 : FTP 自動下載安裝更新包
    bool bN32_CheckForUpdatesOnceDay;
    bool bN32_CheckAtInitailStart;
    bool bN32_CheckAtTrayFeedFinish;
    AnsiString sN32_FTPUserName;
    AnsiString sN32_FTPPassword;
    AnsiString sN32_FTPHost;
    AnsiString sN32_FTPDownloadPath;
    AnsiString sN32_NetDownloadPath;                                            //Steven 20221216 : 使用網路硬碟下載安裝包
    AnsiString sN32_FTPDownloadPath2;                                           //Sam 20230328 : 改使用更新包的產品版本來判別是否更新。
    int iN32_DownloadMode;
    AnsiString asN32_UploadLogPath;
    bool bN33_UpLoadOCRBinLogByNet;                                             //KenHsieh 20230502 : 利揚要求上傳OCR + BIN Log上傳至Host
    AnsiString asN33_UploadLogPath;
    bool bN33_1_NetChangeFileAndData;                                           //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
    bool bN34_GenerateOEEAlarmRpt;                                              //Jimmychiu 20250324 : CC_CYUEAN OEE report
    AnsiString sN34_OEEAlarmRptPath;                                            //Jimmychiu 20250324 : CC_CYUEAN OEE report

    bool bN35_Ground_ESD_Upload;                                                //Sam 20250609 : Record Ground and ESD at intervals and upload
    AnsiString sN35_FTPUserName;
    AnsiString sN35_FTPPassword;
    AnsiString sN35_FTPHost;
    AnsiString sN35_FTPUploadPath;
    int iN35_Interval;

    bool bN40_1_HandlerDataBackUpUseFunction;
    bool bN41_1_HandlerDataBackUpToDiskUseFunction;
    AnsiString sN40_FTPUserName;
    AnsiString sN40_FTPPassword;
    AnsiString sN40_FTPHost;
    AnsiString sN40_FTPUploadPath;
    AnsiString sN41_DiskUploadPath;

    //[O]---------------------------
    bool bO01_ResetNeedClearAndCheckHP;                                         //Steven 20120322 : RESET後,清空In Arm吸嘴資料與HotPlate資料,然後作HotPlate檢查
    bool bO02ResetNotClearPlate;
    bool bO05ResetNeedRemoveAllTray;

    bool bO06_EventLogAutoSave;                                                 //Steven 20110221 Start : EventLogAutoSave
    bool bEnableAlarmHistroyAutoSave;
    bool bEnableAlarmStatistAutoSave;
    bool bEnableProductionAutoSave;                                             //Steven 20140816 : Production Data
    bool bAlarmStatistAutoSaveNetDrive;                                         //Steven 20140306 : 使用網路硬碟
    AnsiString asEventLogAutoSavePath;
    AnsiString asAlarmHistroyAutoSavePath;
    AnsiString asAlarmStatistAutoSavePath;
    AnsiString asProductionAutoSavePath;                                        //Steven 20140816 : Production Data
    AnsiString asAlarmRemoteDirectory;                                          //Steven 20140306 : 使用網路硬碟
    AnsiString asAlarmLocalDirectory;                                           //Steven 20140306 : 使用網路硬碟
    TDateTime dtEventLogLastRecordDate;
    AnsiString sEvenLogDataTime;                                                //Ifor 20160621 新增Even Log Record Date Time 字串
    AnsiString sEventLogLastRecordDate;                                         //Steven 20140702 : For ecs Gem
    int  iNextEventLogRecordSpace;
    bool bAutoSaveLogWeek[7];                                                   //jou 2012-10-15 Auto Save Log 支援 Week 選擇
    //Steven 20110221 End
    bool bO06SaveLogTimePeriod;                                                 //JerryYang 20151026 : 使用ProductionData週期時間存檔
    int  iO06SaveLogTimePeriod;                                                 //JerryYang 20151026 : ProductionData週期時間(0->10mins,1->30mins)
    bool bO10UseEventLogSaver;                                                  //Steven 20140902 : 使用外掛小程式

    int  iO15_SaveFilePeriod;                                                   //Steven 20170829 (wei) : Event Log檔案存檔設定
    bool bO15_EventLogFileNameWithMachineID;
    bool bO15_EventLogSaveSameFolder;                                           //KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾
    bool bO16ConAlarmNeedKeyInPasswordCT;                                       //Steven 20200513 : 改成可以設定[O16]
    int  iO16ConAlarmNeedKeyInPasswordCT;                                       //jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password
    bool bO17EnableLevelUpWhenContiAlarm;                                       //Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限
    int  iO17LevelUpWhenContiAlarmCount;                                        //Steven 20210127 : 連續alarm次數
    int  iO17LevelUpWhenContiAlarmTime;                                         //Steven 20210127 : 連續alarm時間

    bool bO18SafeDoorOnOffDurationDetect;                                       //JerryYang 20210112 : 安全門檢查機制,長時間未開啟要跳出alarm
    int  iO18SafeDoorOnOffDurationHour;
    //Sam 20210107 : Summary Report fuction
    //==>
    bool bO19_AutoRecordReportByEveryDay;
    bool bO19_AutoRecordReportByEveryWeek;
    int  iO19_WeekPeriod;
    bool bO19_AutoRecordReportByEveryMonth;
    int  iO19_MonthPeriod;
    AnsiString asO19_SavePath;
    //<==
    //Sam 20210107 : Summary Report fuction

    bool bO20InOutArmPickerLifeTimeCount;
    bool bO20_1ClearLifeTimeWhenInitialStart;                                   //Steven 20240531 : initial start的時候要清除資料

    bool bO21FTAfterTrayEndClearFailBinCount;                                   //Frank 20241114 : Add
    bool bO22_ClearSortCntByDoubleClick;                                        //Steven 20241206 : 點兩下可以清除數量
    bool bO23_InputLotIDByBarcode;                                              //Steven 20241224 : LotID只能用Barcode
    bool bO24_ProductionLogByLot;                                               //Steven 20250519 : Production Log By Lot

    //[P]---------------------------
    bool bP04ColorIsEmptyUnloader;
    bool bP05_LoaderCylinderPreOn;                                              //Steven 20150429 : 預先打兩下Loader汽缸
    bool bP05_1_UnloadCylinderPreOn;                                            //Steven 20240215 : 退tray前預先打兩下分離汽缸
    bool bP06_LoaderUseCarrierTray;
    bool bP07NoTrayaAutoTrayFeed;                                               //wei 20161121 TSMC 新增[P07]Loader No Traya Auto TrayFeed
    bool bP08TrayFeedCleanLotID;                                                //wei 20170316 Tray Feed Clean Lot ID
    bool bP09TrayEndCanSelectTray;                                              //Steven 20200317 : CleanOut後,可以選擇Tray End, 且要退的Tray要在工作檔設定
    bool bP10FixedTrayProposeTheInitialQuestion;
    bool bP11RecordUPH;                                                         //Frank 20150515
    bool bP13EnableAutoTrayEdgePushCylinderLoop;
    int  iP13EdgePushCylinderLoopDelay;
    int  iP13EdgePushCylinderOnDelay;                                           //JerryYang 20170531 (wei) 敲擊方式改成可以分別設定on off時間
    bool bP14EnableAutoTrayRecevieDelayCount;
    int  iP14AutoTrayRecevieDelayCount;
    int  iP14AutoTrayRecevieLoopDelayTime;
    bool bP16EnableHotplateEdgePushCylinderLoop;                                //jou 2011-08-09 : Hotplate也要敲敲敲
    int  iP16HotplateEdgePushCylinderLoopDelay;                                 //jou 2011-08-09 : Hotplate也要敲敲敲
    int  iP16HotplateEdgePushCylinderOnDelay;                                   //JerryYang 20170531 (wei) 敲擊方式改成可以分別設定on off時間
    bool bP17InArmFullPickFromLoader;                                           //Steven 20111026 : In Arm Full Pick from Loader Tray
    bool bP18FailAutoTrayManual;                                                //jou 2012-03-16 Fail Auto Tray手動補Tray
    bool bP19CatchTrayUpThenCheck;                                              //Steven 20120727 : 夾Tray發生異常時,要先把Arm上升再檢查一次,如果還是沒夾到才Alarm
    bool bP20ManualClearFixTrayDataAfterInitialStart;                           //ChungHung 20130305 add for Amkor Initial Strat後不清Tray盤資料需手動清除後才可Run
    bool bP20_1_ManualClrAutoTrayWhenInitialStart;                              //Steven 20210420 : add for TFME InitialStrat後不清Tray盤資料需手動清除後才可Run
    bool bP20_2_ManualClrLoadTrayWhenInitialStart;                              //Steven 20230117 : add for JSCK InitialStrat後需手動清除Loader Tray後才可Run

    bool bP21CheckFixTray;                                                      //kevin 20130408 Tray feed 時偵測fix tray需取出
    bool bP21_1_CheckFixTray;                                                   //Steven 20250321 initial start時偵測fix tray需放入
    bool bP21_2_LoaderTrayFeed;                                                 //Steven 20250606 : Tray Feed 包含 loader tray

    bool bP22EnableFirstTrayNeedAlarm;                                          //ChungHung 20140521 add for ATK
    int  iEnableFirstTrayNeedAlarmNum;                                          //ChungHung 20140521 add for ATK
    bool bP24_Active;                                                           //JerryYang 20160425 add for 矽格北興 -- P24打勾
    bool bP24_Enable;                                                           //JerryYang 20160425 add for 矽格北興 -- P24 Enable
    bool bP24SkipEventNeedRemoveEmptyAndColorTray;                              //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查
    bool bP24SkipEventNeedRemoveColorTrayForIDT;                                //Sam 20220530 : 當 Loader 發生 Skip/Edit 時，此盤做完後搬到 Empty 軌道後，會收盤起來並報警提示人員收盤(連兩盤)
    bool bP24TwoTrayMustManuallyRemovedForGenernal;                             //Sam 20220817 : 一般P24功能連兩盤手動移除功能設開關
    bool bP25EmptyColorNoSuppleAutoNoLoadEmpty;                                 //kevin 20151102  empty or color TRAY 不補AUTO 123 TRAY就不須載入一個空TRAY。
    bool bP26_OCRCheckLot;                                                      //wei 20151117 OCR Lot check
    bool bP27AutoSortingBinTrayByOutArmwhenCleanOut;                            //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
    bool bP28Auto1OnlyBin1;                                                     //Alick 20160729 add for SCC
    bool bP29LoaderCheckIsFull;                                                 //Steven 20160818 : CheckLoader滿盤
    double dP29LoaderCheckIsFullInterval;                                       //Steven 20160818 : CheckLoader滿盤
    bool bP30FixTryCheckRemainingAmount;                                        //Ifor 20160829 : Check Fix Try 到達設定剩餘IC數量Alarm不停機
    int  iP30FixCheckRemainingAmountInterval;                                   //Ifor 20160829 : Fix Try 剩餘可放IC數量
    bool bP31LoaderTryLastOneFeedContinueRun;                                   //Ifor 20160829 : Loader Try 最後一盤入料Alarm不停機
    bool bP32EmptyColorTrayPreAlarm;                                            //Ifor 20170315 (wei) add 新增Empty/Color Tray Pre Alarm 功能
    bool bP33AutoTrayPreAlarm;                                                  //Ifor 20170315 (wei) add 新增Auto Tray Pre Alarm 功能
    bool bP34CleanOutChangeInitialMode;                                         //kevin 20170417 (wei) : CleanOut change initial Mode
    bool bP35TrayArm;                                                           //kevin 20171006 (wei) tray arm home需遮住home sensor

    bool bIndexAddPressEP;                                                      //jou 20171026 (wei) : 測試中加壓EP
    int  iIndexAddPressEP_Time;                                                 //jou 20171026 (wei) : 測試中加壓EP
    double dIndexAddPressEP_Kg;                                                 //jou 20171026 (wei) : 測試中加壓EP
    double dIndexVibrateEP_Kg;                                                  //jou 20171026 (wei) : 測試中加壓EP
    int  iLotIDLength;                                                          //Frank 20170531 (Steven) add LotID 7碼

    int  iGalilSpeedAcc;                                                        //jou 20171102 (Steven) : Galil Acc 改由文件修改
    int  iGalilSpeedDec;                                                        //jou 20171102 (Steven) : Galil Dec 改由文件修改
    bool bP36BufferTrayNoSame;                                                  //kevin 20171117 (wei) Load && Unload 強制不能使用相同軌道
    bool bP37bAutoCylinderUP;                                                   //kevin 20180726 (wei) Auto 123 氣缸常態在上
    bool bP38UseEmptyFullPutColor;                                              //wei 20170504 Use Empty Full Put Color
    bool bP39ClearUnloaderTrayWhenInitial;                                      //Steven 20200826 : Murata希望在initial start的時候不要清除unloader tray,不然tray end的時候會導致疊料
    bool bP39LoaderHasSkipPlaceToEmpty;                                         //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
    bool bP40TrayYSpeedByMachine;                                               //Sam 20201221 : Tray y step motor by machine
    bool bP41UnloadTrayDisableEdit;                                             //jou 20240403 : Unload Tray Disable Edit
    bool bP42AlarmWhenExitTrayComplete;                                         //Ifor 20210428 add:Auto 退Tray 完成時報警停機
    bool bP44LockLoaderTrayToNone;                                              //JerryYang 20220331 : 矽品中山要求AUTO SKIP後可由TRAY ARM搬TRAY

    bool bP40LdUldUseEmptyAndColorTray;                                         //Ifor 20201218 add: Load & Unload Use Empty And Color Tray

    bool bP45LastLoaderNoInSide;                                                //kevin 20221008 load 空盤 不TRAY
    bool bP46_LoadTrayModeByHandler;                                            //Steven 20221117 : Loader Tray Mode設定跟著機台變
    bool bP47_UseTrayThickAdjustZHeight;                                        //Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度
    bool bP48UnloaderCylinderLoop;                                              //JimmyChiu 20230512 Unloader cylinder loop
    int iP48UnloaderCylinderLoopTimes;                                          //JimmyChiu 20230512 Unloader cylinder loop
    int iP48UnloaderCylinderLoopDelay;                                          //JimmyChiu 20230512 Unloader cylinder loop
    bool bP49UseLocalTraySpeed;                                                 //Ifor 20200825 add: Use Local Tray Speed

    bool bP50DisabledAutoTrackSensorDetect;                                     //Sam 20230221 : 矽格中興國桂要求要能關閉
    bool bP51TrayArmPutwaitUnloadOK;                                            //kevin 20230331 add TRAY arm 放 auto 123 等待tray上升避免輸 送帶滑進去 夾tray.
    bool bP52EmptyColorLastTrayCheck;                                           //KenHsieh 20230919 : Empty & Color Last Tray Check
    bool bP53_ForcedScanBinCodeOfUnloader;                                      //JerryYang 20240111 : add P53 function
    bool P54UnloaderTaryCheckHasErrorBinIC;                                     //Sam 20240108 : 新增退 Tray 時顯示裡面有多少 Error Bin

    AnsiString asMesSyURLPath;                                                  //jou 20200409 : VTest Mes system
    AnsiString asMesSyACodePath;                                                //jou 20200409 : VTest Mes system
    AnsiString asMesSyActionPath;                                               //jou 20200409 : VTest Mes system
    AnsiString asSummaryReportPath;                                             //jou 20220120 : VTEST Summary report path
    AnsiString asByTimeOEEPath;                                                 //jou 20220301 : VTEST By time OEE path
    AnsiString asUPHReportPath;                                                 //jou 20220525 : VTEST UPH report path
    AnsiString asCreateManualEOCAP_URL;                                         //jou 20221104 : VTest CreateManualEOCAP function;
    AnsiString asQueryEocapStatusURL;                                           //jou 20221104 : VTest CreateManualEOCAP function;
    AnsiString asGetRcsCheckingResultUrl;                                       //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
    AnsiString asGetRcsCheckingResultACode;                                     //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
    AnsiString asGetRcsCheckingResultAction;                                    //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult
    bool bGetRcsCheckingResult;                                                 //jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult

    bool bP55LdUldUseEmptyAndColorTray;                                         //Ifor 20201218 add: Load & Unload Use Empty And Color Tray
    bool bP56TrayArmWaitAtColorTrack;                                           //Steven 20240516 : Tray Arm等待位置改到Color
    bool bP57LoaderAutoCleanOutByInputCT;                                       //Sam 20250605 : Loader Count AutoCleanOut
    int  iP58RunModeAfterFT;                                                    //JimmyChiu 20250905 : FT模式結束後切換模式
    bool bP59UnloaderICFloattingAlarmAfterExit;                                 //Sam 20250415 : Unloader 偵測到置偏 IC 退出後再報警
    bool bP60ReadClipCodeFromUnloader;                                          //Jimmychiu 20250818 : Read Clip Code From Unloader(Auto1-3、Fix1-3)
    bool bP61UseTrayTap;                                                        //Ztex 2024.11.12 Add P53 Use Tray Tap

    bool bP62FirstTrayCheckOnUnloader;                                          //Jimmychiu 20251205 : First Tray Check On Unloader
    bool bP62Auto1;                                                             //Jimmychiu 20251205 : First Tray Check On Unloader
    bool bP62Auto2;                                                             //Jimmychiu 20251205 : First Tray Check On Unloader
    bool bP62Auto3;                                                             //Jimmychiu 20251205 : First Tray Check On Unloader
    bool bP62AlwaysEnabledAtLotStart;                                           //Jimmychiu 20251205 : First Tray Check On Unloader
//    bool bP63MachineStopAtIntervalTime;                                         //Jimmychiu 20251205 : First Tray Check On Unloader
//    int  iP63IntervalTime;                                                      //Jimmychiu 20251205 : First Tray Check On Unloader
    bool bP65EnableArmQAMode;                                                   //Ifor 20260407 : [P65] Enable ARM QA Mode
    int  iP65ArmQAModeValue;                                                    //Ifor 20260407 : [P65] ARM QA Mode Value
    bool bP66AutoChangingFlashWarn;                                             //Eastsun 20260515 : 因應防堵工傷意外，Auto1與Auto2動作時，Bin顯示器顯示紅字並持續閃爍，直至換盤動作整個完成後，顯示器恢復原本狀態
    //Steven 20260924 (S12-C)：golden V912 Config.h 有、移植樹沒有的 14 個欄位，逐字補上（含 golden 行號）。
    //   FileRW/IniConfig.gen.inc 由 golden 原檔產生的 HTEditList 註冊碼要用到；本結構不做二進位持久化，
    //   在尾端補欄位不影響任何檔案格式（config.ini 是以區段／鍵存取）。
    bool bAMDFunction;                                                          //Ifor 20260716 : AMD group function (CC_AMD_SG/CC_TFAMD_SUZHOU/CC_TFAMD_M/CC_AMD_US)   // golden Config.h:132
    bool bA76AfterHomeNeedAlm;                                                  //JerryYang 20260626 : add   // golden Config.h:364
    bool bA77_EnableHanaRMSInterlock;                                           //RogerYang 20260725 : HANA RMS Interlock enable   // golden Config.h:365
    bool bA78EnableQAMode;                                                      //AI(ht9045-config) 20260806 (RogerYang) : #R260723-ATK-H9-01 ATK QA Mode 開關(面板值,驅動 bQAMode)   // golden Config.h:366
    bool bD83NNAutoCleanTwoArmSimultaneous;                                     //AI(ht9045-inarm-flow) 20260820 (RogerYang) : NN 兩臂同時下壓+平行放回,預設關(部分機構反而變慢)   // golden Config.h:593
    bool bE79_InArmWaitShtLeftBeforePick;                                       //JerryYang 20260707 : add E79 InArm wait shuttle Left before pick at Ambient/AmbientHot   // golden Config.h:650
    bool bF37NNShuttlePreScan2DID;                                              //Steven 20260728 : NN mode 2DID pre-scan during index down   // golden Config.h:715
    int  iF37NNPreScanStableDelayMS;                                            //Steven 20260728 : NN mode 2DID pre-scan settle delay (ms)   // golden Config.h:716
    bool bL49UseBySetupOffsetRange;                                             //Ifor 20231219 add Use By Setup Offset Range   // golden Config.h:953
    int  iL49UseBySetupOffsetRange;                                             //Ifor 20231219 add Use By Setup Offset Range   // golden Config.h:954
    bool       bFtpUploadBackground;                                            //AI(ht9045-v899) 20260612(CASE-20260611-001): Lot End FTP上傳背景化開關, PTI預設true其餘false, 供S6/S7 uLotInfo分流(背景EnqueueUpload或原同步路徑)   // golden Config.h:983
    bool bN07_Alarm;                                                            //Steven 20260603 : Secs_Gem disconnect alarm   // golden Config.h:1015
    bool bO21_1RTContinueStartClearLoadingFailBin;                              //AI(ht9045-clearcount-flow) 20260703 (RogerYang) : ATK RT Continue Start clear Loading & Fail Bin count   // golden Config.h:1388
    bool bO25_RTResetCountBinWhenEnterRT;                                       //AI(ht9045-clearcount-flow) 20260713 (RogerYang) : SCK 進RT時把Count Bin(iBinData32)計數基準歸零,RT只算RT期   // golden Config.h:1392
}HT9045_CONFIG;
extern HT9045_CONFIG IniConfig;
#endif

