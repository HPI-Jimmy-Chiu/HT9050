//==============================================================================
#ifndef cmydefH
#define cmydefH

//AI(W0-TAIL) 20260626: de-VCL cmydef.h. Dropped handlerlog.h / MyStringList.h /
//  MyBinDisp.h / HTEditList.h (VCL-form/UI; cmydef references no types from
//  MyBinDisp/HTEditList, and uses TMyLog by value + TMyStringList* only).
#include "MachineType.h"        //Steven 20130809 : OK
#include "myTimer.h"
#include "cprod.h"              //JerryYang 20150910 Auto Sorting BinTray by Out Arm when Clean Out
#include "cpublic.h"
//AI(W0-TAIL) 20260626: forward-decls (defs deferred): TMyLog->W7 (VCL TWinControl/TObject),
//  TMyStringList->W3 (Public/MyStringList.h : public TStringList + __property + MyMemo.h).
class TMyLog;
class TMyStringList;
typedef int TColor;            //AI(W0-TAIL) 20260626: VCL Graphics.hpp alias (extern TColor tcBinColor[])
//AI(W0-TAIL) 20260626: faithful stub of uPoint2D (real def in Public/HTEditList.h, TODO W3).
//  Used by value in 'extern uPoint2D In/OutArmAutoCalSuckZPoint'. Layout = {int X; int Y;}
//  matches HTEditList.h:249-259 exactly; ctor/Clear bodies live in HTEditList.cpp (W3).
class uPoint2D
{
public:
    uPoint2D();
    uPoint2D(int x, int y);
    ~uPoint2D(){}
    int X;
    int Y;
    void Clear();
};

#define Gali_MaxAxis 4
#define MAX_X_ITEM 30
#define MAX_Y_ITEM 70
#define SERVER_MOTOR_POWER_ON_DELAY 4

//old parameter
//#define MAX_SUCK 49 //33->49 Eliot 2009_12_27
//#define MAX_SUCK 67 //49->51 20111130 Dell
//new parameter
//-------------------------
//Sam 20190112 LM
//Steven 20170329 (Wei) : Add individual rotate motor 65 --> 85
//JerryYang 20180213 100 -> 99
//Jimmychiu 20220928 140 -> 150
//Steven 20240822 :  150 -> 158
#define TOTAL_MOTOR 164

#define KitPitchX     1000
#define KitPitchY     1000
#define Z1_Z2_Normal    0
#define Z1Up_Z2Down     1
#define Z1Down_Z2Up     2
#define IndexIsBack     3
#define Z1_Z2_Down      4

#define QUAD_SITE_X_PITCH  2000
#define KIT_PITCH     12000

//#define ARM_Y_PITCH 6000  //ChungHung 20120505 HT9045WS

#define TTL_MODE   0
#define GPIB_MODE  1
#define RS232_MODE 2
#define TCP_IP_MODE 3       //wei 20211027 open short TCP/IP

//2011.09.19 Q_Q GPIBInterface start
#define InterfaceType_ADVAN_Type1       0
#define InterfaceType_256Bin            1 //kevin 20140317 256 bin ADD
#define InterfaceType_16Bin             2 //Steven 20140805 ADD
#define InterfaceType_32Bin             3 //Steven 20140805 ADD
#define InterfaceType_SPEA_Type         4 //1
#define InterfaceType_16BinGS           5 //Steven 20161122 : For SCK ART
#define InterfaceType_32BinGS           6 //Steven 20161122 : For SCK ART
#define InterfaceType_15BinT6577        7 //Steven 20181221 : Add T6577 Tester
#define InterfaceType_15BinQorvo        8 //Steven 20201022 : Add Qorvo protocol
#define InterfaceType_Delta_Castle      9 //Ifor 20200603 add: Delta Castle Flow
//#define InterfaceType_Catalyst_SC       5 //2

#define InterfaceType_TTL            1000       //Isaac 20200903 :TTL RS232通訊
#define InterfaceType_Standard          0       //Isaac 20200903 :TTL RS232通訊

#define WM_Interface_GPIB             WM_USER+100
//2011.09.19 Q_Q GPIBInterface end

#define OFF_LINE  0
#define ON_LINE   1
#define _2D_SORT   2        //Frank 20221122 : 2DID sorting for ATK
//#define MANUAL    2

#define PCI132

//Eliot 2010_05_01 start
#define Panasonic_DRIVER        0   // A4
#define Mitsubishi_DRIVER       1
#define Panasonic_DRIVER_A5     2   // A5
//Eliot 2010_05_01 end

#define MotionnetIO_L112  0
#define MotionnetIO_MN200 1
#define NewIO_MN200 2                           //Steven 20181018 : Add for new IO排列
#define PCI_MC88X1 3                            //Nickliu 20230306 add for 7080b

#define MotionCard_SYN      0
#define MotionCard_Contec   1
#define PCI_P64C64          3                   //Nickliu 20230306 add for 7080b

//extern int HT8080TransKIT;                    //Steven 20120822 : 沒用到,馬克掉
//#define SortingBinTray_MAX_TRAY_ITEM 6          //JerryYang 20150910 Fix3 不整盤    //Ifor 20161123 add Fix 5->6

//ATC Index
#define OLD_VALUES         0
#define SAME_COUNT         1

#define VERSION_INFO_KEY_ROOT TEXT("\\StringFileInfo\\")
#define VERSION_INFO_KEY_TRANS TEXT("\\VarFileInfo\\Translation")               //Sam 20230328 : 改使用更新包的產品版本來判別是否更新。

extern TMyStringList *slEventLog;                                               //Steven 20161115 : EventLog存成文字檔  //Steven 20200116 : 改成全域變數, 避免fMain被解構造成記憶體異常
extern TMyStringList *sl2DMappingLog;                                           //JerryYang 20230322 : add 2D mapping result
extern TStringList *slBundlID;
extern TStringList *slDupBundlID;
extern TStringList *slDupUnloadBundlID;

extern AnsiString asTempCtrl[tcTotalCount];

extern void SaveEventLog();
extern unsigned int MyLongMask[32];
extern const byte MyBitMask[8];
extern const int GaliPosOffSet;
//extern int GaliPosYRange;  //JerryYang 20180411 (jou) : Z軸移動前確認Y軸位置保護   //Isaac 20201012 : const int-> int    //Isaac 20210604 : IndexY偵測範圍名子統一成IniConfig.GaliPosRange
extern const int MotErrPos;
extern AnsiString sOEETimeDataName[tdTotal];                                    //Steven 20231120 : 紀錄機台稼動時間

extern AnsiString sTrayPosName[MAX_TRACK];
extern AnsiString s06TrayName[eTrayCount];
extern AnsiString s6ShortTrayName[eTrayCount];
extern AnsiString asTrayAlias[eTrayCount];
extern AnsiString s3TrayName[e3TrayCount];
extern AnsiString s6TrayName[eTrayCount];
extern AnsiString asTrayForBinDisp[eBinDispTotal];
extern TMyStringList *slHanaTrayMap[eTrayCount];
extern long GALI_ERROR_MAX_PR1;//Chunghung 20131111 add
extern long GALI_ERROR_MAX_PR2;//Chunghung 20131111 add
extern long GALI_ERROR_MAX_PR3;//Chunghung 20131111 add
extern long GALI_ERROR_MAX_PR4;//Chunghung 20131111 add

//extern const int CY_PUSH;                 //Steven 20120822 : 沒用到,馬克掉
extern const int CY_POP;

extern const int TC401;
extern const int KT4H ;
extern const int E5DC ;
extern const int NoHeater;                  //Steven 20171227 (Wei) : Add for HT-9045L
extern const int DTK4848;                   //KaiHuang 20190821 : 新增台達 DTK4848溫控器
extern int TC401HeaterControl;              //Steven 2014030 : 新增OMRON E5DC溫控器

extern const int NULL_IC;
extern const int HAS_TESTING_IC;
extern const int HAS_IC;
extern const int HOTED;
extern const int HAS_HOT_IC;
extern const int IS_ID_TRAY;
extern const int HAS_NULL_IC;
extern const int RotateOK;
extern const int HAS_CLEAN_IC;
extern const int HAS_NULL_CLEAN_IC;
extern const int CLEAN_FINISH_IC;
extern const int HAS_TRY_SUCK_IC;           //ChungHung 20120206 Hotplate check
extern const int HAS_SUCK_IC;               //ChungHung 20120206 Hotplate check

extern const int HAS_OCR_OK;                //ChungHung 20120830 add OCR Function
extern const int HAS_OCR_NG;                //ChungHung 20120830 add OCR Function
extern const int HAS_OCR_Err;               //ChungHung 20120830 add OCR Function
extern const int HAS_BARCODEERROR_IC;       //Steven 20121009 : Bar Code
extern const int HAS_CLEAN_FINSH_IC;        //kevin 20130226
extern const int HAS_SKIP_IC;
extern const int HAS_CASSETTE_EMPTY;        //wei 20180702 MR
extern const int HAS_CASSETTE_TRAY;         //wei 20180702 MR
extern const int HAS_CASSETTE_DEVICE;       //wei 20180702 MR
extern const int HAS_CASSETTE_FULLDEVICE;   //wei 20180702 MR
extern const int HAS_CASSETTE_FULLTRAY;     //wei 20180702 MR
extern const int HAS_CASSETTE_PASS;         //wei 20180702 MR
extern const int HAS_CASSETTE_FAIL;         //wei 20180702 MR
extern const int HAS_NewLot_IC;             //wei 20180529
extern const int WAIT_ALIGN_IC;             //ChungHung 20210113 add for Alignment CCD  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
extern const int WAIT_TRAYMAP_IC;           //KenHsieh 20220923 : add Tray Map Throw IC Function

extern AnsiString sIC_Type[29];

extern const int Vaccum_On;
extern const int Vaccum_Off;
extern const int Vaccum_FallDown;
extern const int Vaccum_Initial_On;
extern const int Vaccum_Initial_Off;
// -----------------------------------------------------------------------------
//extern const int bAutoPick ;
//extern const int bAutoPlace;
extern const bool bAutoPick;     //kevin 20220916 change type int=> bool
extern const bool bAutoPlace;     //kevin 20220916 change type int=> bool
// -----------------------------------------------------------------------------
extern const int REALLY     ;
extern const int HAS_TRAY   ;
extern const int DUMMY      ;
// -----------------------------------------------------------------------------
extern const int TYPE_A;
extern const int TYPE_B;
// -----------------------------------------------------------------------------
extern const int START_TEST ;
extern const int TEST_PASS  ;
extern const int TEST_FAIL1 ;
extern const int TEST_FAIL2 ;
extern const int TEST_FAIL3 ;
extern const int TEST_FAIL4 ;
extern const int TEST_FAIL5 ;
extern const int TEST_FAIL6 ;
extern const int TEST_FAIL7 ;
extern const int TEST_FAIL8 ;
extern const int SEND       ;
extern const int RECEIVE    ;
// -----------------------------------------------------------------------------
extern bool InitialOK;
extern bool SystemStart;
extern bool fAllMotorHome;
extern bool SoftStart;
extern bool SoftStop;
extern bool bPhysicalStart;                         //Steven 20141006 : SECS GEM使用Remote Start功能
extern Word SystemHour, SystemMin, SystemSec, SystemMSec;
extern Word SystemYear, SystemMonth, SystemDate;
extern Word SystemYearYesterday, SystemMonthYesterday, SystemDateYesterday;
extern AnsiString CurrentDir;

extern bool bDoOCRFunction;                         //ChungHung 20121002 add OCR Function
extern bool bWaitTesterFinish;                      //ChungHung 20140716 add if testing not finish can not homing
extern int  iAMRCoverTray;                                         //Eastsun 20260515 F009 整合: KYEC AMR Cover Tray 計數
// -----------------------------------------------------------------------------
//extern char Com2Buffer[1024];
extern AnsiString Com2Buffer;                       //Steven 20111028 : 改成AnsiString
extern AnsiString ComOmronBuffer;                   //Steven 20120220 : Omron EJ1N溫控器
extern bool  Com2ReceiveOK;
extern bool  ComOmronReceiveOK;                     //Steven 20120220 : Omron EJ1N溫控器
//jou 2012-03-13 ATC start:
extern bool  bATCInitialFinish;                     //jou 2012-03-16 ATC Initial 完成後才能開始Read/Write溫度值
extern int   iATCInitialTask[tcTotalCount];         //jou 2012-03-16 ATC Initial Task
extern int   iATCProcessTask[tcTotalCount];
extern bool  bATCWriteCommand[tcTotalCount];        //jou 2012-03-16 ATC Write=true / Read=false
extern bool  bATCReceiveErr[tcTotalCount];          //jou 2012-03-16 ATC 下指令回傳錯誤植Err=true
extern bool  ComATCReceiveOK[tcTotalCount];         //jou 2012-03-16 ATC 下指令後COM PORT回應Flag
extern double fATCReadBuffer[tcTotalCount];         //jou 2012-03-16 ATC 回傳的溫度值
extern AnsiString asATCErrorString[tcTotalCount];   //jou 2012-03-16 ATC Error Ccde資訊紀錄
//jou 2012-03-13 ATC end

extern AnsiString TestSiteFileName[2][TotalTestMode];   //JerryYang 20201125 : TestSiteFileName移到cmydef

extern int  iOneCycle;

extern int  iCleanOut;
extern int  iFixOneCycle;                                                       //kevin 20130312 onecycle 兩支arm讓開維修
extern int  iTrayFeed;
extern bool bAMRFullICBin;                                              //a-side cmydef.cpp L5432  //Eastsun 20260515 F011 整合 (Phase4-F5T3 KYEC AMR FullTray flag)
extern bool bSameSetupFileNoDownload;  //Eastsun 20260515 F011 整合 (Phase4-F4T4 extern bSameSetupFileNoDownload)
extern int  iHome;
extern int  iReset;
extern int  iAlarmReset;

extern int  iCatchTrayControlManual;
extern int  OutArmPlaceToManual;
extern bool bAtuoTrayICDetectErr;   //Isaac 20180109 (Steven) : auto123可前進後退
extern bool bLoaderTrayICDetectErr;   //Sam 20200316 : Loader Detect Tray

extern int  HotTime[2][50][50];
extern int  iRowOnHotPlate[2][50][50];  //JerryYang 20180718 (wei) : 放料至hot plate記錄吸嘴位置
extern bool SystemInitialOK;
extern int  iHotLineChange;
extern bool bOneCycle_BackUp;  //ChungHung 20141111 add for SCK junction temp issue   //JerryYang 20161129 iOneCycle_BackUp改成bool
extern bool bRS232Delay;       //RogerYang 20180901 add 矽格湖口Demo AI CCD Function

extern const int  K_RETRY    ;
extern const int  K_SKIP     ;
extern const int  K_CLEAN_OUT;
extern const int  K_TRAY_FEED;
extern const int  K_TRAY_END ;
extern const int  K_RESET;
extern const int  K_HOME;       //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
extern const int  K_TRAIN;
extern const int  K_FIX;        //kevin 20130218 onecycle 兩支arm讓開維修
extern const int  K_ONECYCLE;   //ChungHung 20140730 add ContinuousFailHaveOneCycle
extern const int  K_PAUSE;      //Steven 20150722 : Add按鍵Type - Pause
extern const int  K_START;      //Steven 20150722 : Add按鍵Type - Start

extern const int  N_INTEGER   ; //只有整數
extern const int  N_DOUBLE    ; //只有浮點數
extern const int  N_NO_SYMBOL ; //沒有特殊符號
extern const int  N_PASSWORD  ; //密碼文
extern const int  N_NO_SPACE  ; //無空白鍵
extern const int  N_UPPERCASE ; //大寫優先
extern const int  N_NO_NUM_PAD; //不需要數字鍵
extern const int  N_PORT      ; //通訊埠
extern const int  N_IP_ADDR   ; //IP位置
extern bool CheckSystemPower;

//##############################################################################
//====================================================================
extern const int C_TrayZ_Selector  ;
extern const int C_TrayY_Fixer     ;
extern const int C_Auto1Side_Fixer ;
extern const int C_Auto2Side_Fixer ;
extern const int C_Auto3Side_Fixer ;
extern const int C_Auto1_Selector  ;
extern const int C_Auto2_Selector  ;
extern const int C_Auto3_Selector  ;
extern const int C_TrayX_UpDown    ;
extern const int C_EmptyLoaderZ_Select;
extern const int C_ColorLoaderZ_Select;
extern const int C_Empty_Fix;
extern const int C_TrayCover     ;      //Steven 20140409 : Auto Retest
extern const int C_CatchTray_Fix;
extern const int C_Color_Fix;
// loader

extern const int C_LoaderEdgePush;
extern const int C_Auto1EdgePush;
extern const int C_Auto2EdgePush;
extern const int C_Auto3EdgePush;

extern const int C_Load_Up;
extern const int C_Load_Middle;
extern const int C_Color_Up;
extern const int C_Color_Middle;
extern const int C_Empty_Up;
extern const int C_Empty_Middle;
extern const int C_TrayVibration;
extern const int C_HotplateVibration;
extern const int C_CoolingValve;            //20111130  Dell
extern const int C_Auto1_Up            ;    //Auto 3上頂汽缸
extern const int C_Auto2_Up            ;    //Auto 2上頂汽缸
extern const int C_Auto3_Up            ;    //Auto 3上頂汽缸
extern const int C_Auto1LoaderZ_Select ;    //Auto 1分離汽缸
extern const int C_Auto2LoaderZ_Select ;    //Auto 2分離汽缸
extern const int C_Auto3LoaderZ_Select ;    //Auto 3分離汽缸
extern const int C_Fix1LoaderZ_Select;      //Fix 1分離汽缸   kevin 20120718 代號不能改
extern const int C_Fix2LoaderZ_Select;      //Fix 2分離汽缸
extern const int C_Fix3LoaderZ_Select;      //Fix 3分離汽缸
extern const int C_Auto2_Middle;            //kevin 20120725
extern const int C_Shuttle_Knocker_1   ;    //Shuttle敲敲
extern const int C_Shuttle_Knocker_2   ;    //Shuttle敲敲
extern const int C_InputRotateKIT      ;    //Steven 20121001 : 旋轉Kit
extern const int C_OutputRotateKIT     ;    //Steven 20121001 : 旋轉Kit
extern const int C_FixTray_FullPlace   ;    //Steven 20140310 : Fix3滿盤氣缸
extern const int C_DockYAxisOn         ;    //Steven 20140310 : One Touch Docking
extern const int C_DockYAxisOff        ;    //Steven 20140310 : One Touch Docking
extern const int C_DockXAxisOn         ;    //Steven 20140310 : One Touch Docking
extern const int C_DockXAxisOff        ;    //Steven 20140310 : One Touch Docking
extern const int C_CatchTray_FixOn    ;    //ChungHung 20140624 : Auto Retest
extern const int C_CatchTray_FixOff   ;    //ChungHung 20140624 : Auto Retest
extern const int C_TurnTrayArm        ;    //ChungHung 20140701 : AutoRetest
extern const int C_TurnTrayArmLock    ;    //ChungHung 20140814 : AutoRetest
extern const int C_OCRLight_Up        ;    //wei 20150720 OCR觸發
extern const int C_OCRLight_Down      ;    //wei 20150720 OCR觸發
extern const int C_SLK1_Clamp         ;    //JerryYang 20160524
extern const int C_SLK1_Unclamp       ;    //JerryYang 20160524
extern const int C_SLK2_Clamp         ;    //JerryYang 20160524
extern const int C_SLK2_Unclamp       ;    //JerryYang 20160524
extern const int C_Socket_Clamp       ;    //JerryYang 20160524
extern const int C_Socket_Unclamp     ;    //JerryYang 20160524
extern const int C_LoaderUpPress      ;    //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
extern const int C_HingeLookOn        ;    //wei 20170413
extern const int C_HingeLookOff       ;    //wei 20170413

extern const int C_CassetteArmCatchOn ;    //wei 20180702 MR
extern const int C_CassetteArmCatchOff;    //wei 20180702 MR
extern const int C_LoadPortYOn        ;    //wei 20180702 MR
extern const int C_LoadPortYOff       ;    //wei 20180702 MR
extern const int C_LoadPortCatchOn    ;    //wei 20180702 MR
extern const int C_LoadPortCatchOff   ;    //wei 20180702 MR

extern const int C_TrayBracketUpOn    ;    //wei 20180702 MR
extern const int C_TrayBracketUpOff   ;    //wei 20180702 MR
extern const int C_TrayBracketOpenOn  ;    //wei 20180702 MR
extern const int C_TrayBracketOpenOff ;    //wei 20180702 MR
extern const int C_StackedTrayCatchOn ;    //wei 20180702 MR
extern const int C_StackedTrayCatchOff;    //wei 20180702 MR
extern const int C_StackedTrayLockOn  ;    //wei 20180702 MR
extern const int C_StackedTrayLockOff ;    //wei 20180702 MR
//Sam 20190112 LM
//==>
extern const int C_LoadRobotX  ;
extern const int C_UnloadRobotX ;
//<==
//Sam 20190112 LM
extern const int C_Auto1UpPress       ;    //JerryYang 20190423 新增unloader壓tray
extern const int C_Auto2UpPress       ;    //JerryYang 20190423 新增unloader壓tray
extern const int C_Auto3UpPress       ;    //JerryYang 20190423 新增unloader壓tray

extern const int C_InFlipper1         ;     //Frank 20210612 : Flipper Function
extern const int C_InFlipper1Lock     ;
extern const int C_InFlipper2         ;
extern const int C_InFlipper2Lock     ;
extern const int C_InFlipper3         ;
extern const int C_InFlipper3Lock     ;

extern const int C_OutFlipper1         ;     //Frank 20210612 : Flipper Function
extern const int C_OutFlipper1Lock     ;
extern const int C_OutFlipper2         ;
extern const int C_OutFlipper2Lock     ;
extern const int C_OutFlipper3         ;
extern const int C_OutFlipper3Lock     ;

extern const int C_LoaderCasstteLock  ;
extern const int C_EmptyCasstteLock   ;
extern const int C_ColorCasstteLock   ;
extern const int C_Auto1CasstteLock   ;
extern const int C_Auto2CasstteLock   ;
extern const int C_Auto3CasstteLock   ;

extern const int C_TeachGlassUp        ;
extern const int C_Shuttle1Precisor    ;
extern const int C_Shuttle2Precisor    ;

extern const int C_MultileEmptyY_On             ;                               //KaiChen 20200716 ：OHT
extern const int C_MultileEmptyY_Off            ;
extern const int C_MultileEmptyCatch_On         ;
extern const int C_MultileEmptyCatch_Off        ;
extern const int C_MultileEmptyBracketUp_On     ;
extern const int C_MultileEmptyBracketUp_Off    ;
extern const int C_MultileEmptyCornerPush       ;
extern const int C_MultileEmptyLoaderZ_Select   ;
extern const int C_MultileEmptyZ_Select         ;
extern const int C_TrayBracket2UpOn             ;
extern const int C_TrayBracket2UpOff            ;
extern const int C_MultileEmptyLock             ;
extern const int C_TrayBracketOpen2On           ;
extern const int C_TrayBracketOpen2Off          ;

extern const int C_CSTHoldDown                  ;
extern const int C_CSTHoldDown2                 ;
extern const int C_LoaderCarEdgePush            ;                               //JimmyChiu 20220408 For Laser Scan

extern const int C_InAreaAlignment              ;                               //ChungHung 20210113 add for Alignment CCD
extern const int C_OutAreaAlignment             ;                               //ChungHung 20210113 add for Alignment CCD

extern const int C_PlacementArm                 ;                               //JimmyChiu 20220908 add Pickup Error Placement
extern const int C_TesterSidePush               ;                               //Richard 20220321 : 渠梁Side Push

extern const int C_Load2CasstteLock             ;
extern const int C_Load2_Middle                 ;
extern const int C_Load2_Up                     ;
extern const int C_Tray2Z_Selector              ;
extern const int C_Load2UpPress                 ;
extern const int C_Tray2Y_Fixer                 ;
extern const int C_Load2EdgePush                ;
extern const int C_Load2TrackFloodgate          ;
extern const int C_Load2PushBack_Push           ;
extern const int C_Load2PushBack_Back           ;
extern const int C_Load2Separate                ;
extern const int C_Load2SeparateRL              ;
extern const int C_Load2SeparateRR              ;
extern const int C_Load2SeparateFL              ;
extern const int C_Load2SeparateFR              ;                               //Steven 20240822 : For HT-9046AU

//Ztex 2023.04.13 Add HT-1032 IO ==>
extern const int C_LoadTrackFloodgate ;
extern const int C_EmptyTrackFloodgate;
extern const int C_ColorTrackFloodgate;
extern const int C_Auto1TrackFloodgate;
extern const int C_Auto2TrackFloodgate;
extern const int C_Auto3TrackFloodgate;
extern const int C_SafeDoor1Lock;
extern const int C_SafeDoor2Lock;
extern const int C_SafeDoor3Lock;
extern const int C_SafeDoor4Lock;
extern const int C_SafeDoor5Lock;
extern const int C_SafeDoor6Lock;
extern const int C_SafeDoor7Lock;
extern const int C_SafeDoor8Lock;
extern const int C_Shuttle1Floodgate;
extern const int C_Shuttle2Floodgate;
extern const int C_OutShuttle1Floodgate;                                        //Ifor 20240620 add:Out Shuttle Floodgate
extern const int C_OutShuttle2Floodgate;                                        //Ifor 20240620 add:Out Shuttle Floodgate
//Ztex 2023.04.13 Add HT-1032 IO <==

extern const int C_LoaderPushBack_Push      ;
extern const int C_EmptyPushBack_Push       ;
extern const int C_ColorPushBack_Push       ;
extern const int C_Auto1PushBack_Push       ;
extern const int C_Auto2PushBack_Push       ;
extern const int C_Auto3PushBack_Push       ;
extern const int C_LoaderSeparate      ;
extern const int C_EmptySeparate       ;
extern const int C_ColorSeparate       ;
extern const int C_Auto1Separate       ;
extern const int C_Auto2Separate       ;
extern const int C_Auto3Separate       ;

extern const int C_UnderTrayArmYCatch  ;

//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air ==>
extern const int C_EnhaustAirVentOpen ;
extern const int C_EnhaustAirVentClose;
extern const int C_LUpEnhaustAirOpen   ;
extern const int C_LUpEnhaustAirClose  ;
extern const int C_RUpEnhaustAirOpen  ;
extern const int C_RUpEnhaustAirClose ;
//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air <==

extern const int C_CatchMagazineTray      ;  //JerryYang 20220909 : add magazine
extern const int C_CatchMagazineTray1     ;
extern const int C_Auto3BlockZ            ;
extern const int C_MagYTrayOut            ;
extern const int C_TrayXFloodgate1      ;
extern const int C_TrayXFloodgate2      ;
extern const int C_TrayXFloodgate3      ;
extern const int C_TrayXFloodgate4      ;
extern const int C_FixTray_UpDown       ;
extern const int C_LoaderPushBack_Back      ;
extern const int C_EmptyPushBack_Back       ;
extern const int C_ColorPushBack_Back       ;
extern const int C_Auto1PushBack_Back       ;
extern const int C_Auto2PushBack_Back       ;
extern const int C_Auto3PushBack_Back       ;

extern const int C_Auto4Side_Fixer             ;
extern const int C_Auto5Side_Fixer             ;
extern const int C_Auto6Side_Fixer             ;
extern const int C_Auto4_Selector              ;           //Auto 4上頂汽缸 或 中間分離
extern const int C_Auto5_Selector              ;           //Auto 5上頂汽缸 或 中間分離
extern const int C_Auto6_Selector              ;           //Auto 6上頂汽缸 或 中間分離
extern const int C_Auto4EdgePush               ;
extern const int C_Auto5EdgePush               ;
extern const int C_Auto6EdgePush               ;
extern const int C_Auto4_Up                    ;           //Auto 4上頂汽缸
extern const int C_Auto5_Up                    ;           //Auto 5上頂汽缸
extern const int C_Auto6_Up                    ;           //Auto 6上頂汽缸
extern const int C_Auto4LoaderZ_Select         ;           //Auto 4分離汽缸
extern const int C_Auto5LoaderZ_Select         ;           //Auto 5分離汽缸
extern const int C_Auto6LoaderZ_Select         ;           //Auto 6分離汽缸
extern const int C_Fix4LoaderZ_Select          ;           //Fix 4分離汽缸
extern const int C_Fix5LoaderZ_Select          ;           //Fix 5分離汽缸
extern const int C_Fix6LoaderZ_Select          ;           //Fix 6分離汽缸
extern const int C_Auto4UpPress                ;           //Auto 4unloader壓tray
extern const int C_Auto5UpPress                ;           //Auto 5unloader壓tray
extern const int C_Auto6UpPress                ;           //Auto 6unloader壓tray
extern const int C_Auto4CasstteLock            ;
extern const int C_Auto5CasstteLock            ;
extern const int C_Auto6CasstteLock            ;
extern const int C_Auto4TrackFloodgate         ;
extern const int C_Auto5TrackFloodgate         ;
extern const int C_Auto6TrackFloodgate         ;
extern const int C_Auto4PushBack_Push          ;
extern const int C_Auto5PushBack_Push          ;
extern const int C_Auto6PushBack_Push          ;
extern const int C_Auto4Separate               ;
extern const int C_Auto5Separate               ;
extern const int C_Auto6Separate               ;
extern const int C_Auto4PushBack_Back          ;
extern const int C_Auto5PushBack_Back          ;
extern const int C_Auto6PushBack_Back          ;
extern const int C_FixedSeatTL                 ;            //Jimmychiu 20240322 : Top & Bottom Inspect
extern const int C_FixedSeatTR                 ;
extern const int C_FixedSeatBL                 ;
extern const int C_FixedSeatBR                 ;
extern const int C_TopBtmRotateLock            ;
extern const int C_LoaderSeparateRL            ;
extern const int C_LoaderSeparateRR            ;
extern const int C_LoaderSeparateFL            ;
extern const int C_LoaderSeparateFR            ;
extern const int C_EmptySeparateRL             ;
extern const int C_EmptySeparateRR             ;
extern const int C_EmptySeparateFL             ;
extern const int C_EmptySeparateFR             ;
extern const int C_ColorSeparateRL             ;
extern const int C_ColorSeparateRR             ;
extern const int C_ColorSeparateFL             ;
extern const int C_ColorSeparateFR             ;
extern const int C_Auto1SeparateRL             ;
extern const int C_Auto1SeparateRR             ;
extern const int C_Auto1SeparateFL             ;
extern const int C_Auto1SeparateFR             ;
extern const int C_Auto2SeparateRL             ;
extern const int C_Auto2SeparateRR             ;
extern const int C_Auto2SeparateFL             ;
extern const int C_Auto2SeparateFR             ;
extern const int C_Auto3SeparateRL             ;
extern const int C_Auto3SeparateRR             ;
extern const int C_Auto3SeparateFL             ;
extern const int C_Auto3SeparateFR             ;
extern const int C_EmptyEdgePush               ;
extern const int C_ColorEdgePush               ;

extern const int C_Auto4SeparateRL             ;
extern const int C_Auto4SeparateRR             ;
extern const int C_Auto4SeparateFL             ;
extern const int C_Auto4SeparateFR             ;
extern const int C_Auto5SeparateRL             ;
extern const int C_Auto5SeparateRR             ;
extern const int C_Auto5SeparateFL             ;
extern const int C_Auto5SeparateFR             ;
extern const int C_Auto6SeparateRL             ;
extern const int C_Auto6SeparateRR             ;
extern const int C_Auto6SeparateFL             ;
extern const int C_Auto6SeparateFR             ;
extern const int C_LoadCarRFIDRotArmD          ;                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int C_LoadCarRFIDRotArmU          ;                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int C_LoadTrayDetD                ;                                //RogerYang 20250828 add for 殘料檢氣缸
extern const int C_LoadTrayDetU                ;                                //RogerYang 20250828 add for 殘料檢氣缸
extern const int C_LoadTrayDetF                ;                                //RogerYang 20250828 add for 殘料檢氣缸
extern const int C_LoadTrayDetB                ;                                //RogerYang 20250828 add for 殘料檢氣缸

extern const int C_LoaderCarrier               ;                                //Ifor 20251216 add:Boat Carrier
extern const int C_Auto1Carrier                ;                                //Ifor 20251216 add:Boat Carrier
extern const int C_Auto2Carrier                ;                                //Ifor 20251216 add:Boat Carrier

#define CynForHome 74       //記得改!!
extern int CynNeedHome[CynForHome];                                             //Steven 20240123 : 改用enable確認氣缸是否要復歸
//Steven 20230907 : For HT-9011UC
//==>
extern int C_AutoSide_Fixer    [MAX_AUTO_TRAY];
extern int C_Auto_Selector     [MAX_AUTO_TRAY];
extern int C_AutoEdgePush      [MAX_AUTO_TRAY];
extern int C_Auto_Up           [MAX_AUTO_TRAY];
extern int C_AutoZ_Select      [MAX_AUTO_TRAY];
extern int C_AutoUpPress       [MAX_AUTO_TRAY];
extern int C_AutoCasstteLock   [MAX_AUTO_TRAY];
extern int C_AutoTrackFloodgate[MAX_AUTO_TRAY];
extern int C_AutoPushBack_Push [MAX_AUTO_TRAY];
extern int C_AutoSeparate      [MAX_AUTO_TRAY];
extern int C_AutoPushBack_Back [MAX_AUTO_TRAY];
extern int C_FixLoaderZ_Select [MAX_AUTO_TRAY];

extern int C_AutoCarrier       [2];                                             //RogerYang 20260202 : Add for CR

extern AnsiString sJAM1101     [MAX_AUTO_TRAY];
extern AnsiString sJAM1102     [MAX_AUTO_TRAY];
extern AnsiString sJAM1103     [MAX_AUTO_TRAY];
extern AnsiString sJAM1104     [MAX_AUTO_TRAY];
extern AnsiString sJAM1106     [MAX_AUTO_TRAY];
extern AnsiString sJAM1107     [MAX_AUTO_TRAY];
extern AnsiString sJAM1108     [MAX_AUTO_TRAY];
extern AnsiString sJAM1109     [MAX_AUTO_TRAY];
extern AnsiString sJAM1110     [MAX_AUTO_TRAY];
extern AnsiString sJAM1111     [MAX_AUTO_TRAY];
extern AnsiString sJAM1112     [MAX_AUTO_TRAY];
extern AnsiString sJAM1113     [MAX_AUTO_TRAY];
extern AnsiString sJAM1114     [MAX_AUTO_TRAY];
extern AnsiString sJAM1158     [MAX_AUTO_TRAY];                                 //AI(general) 20260323 (RogerYang) : UpSafe sensor 未到位5秒 Alarm
extern AnsiString sMES1120     [MAX_AUTO_TRAY];
extern AnsiString sMES1121     [MAX_AUTO_TRAY];
extern AnsiString sMES1122     [MAX_AUTO_TRAY];
extern AnsiString sMES1123     [MAX_AUTO_TRAY];
extern AnsiString sWAR1130     [MAX_AUTO_TRAY];
extern AnsiString sWAR1151     [MAX_AUTO_TRAY];
extern AnsiString sJAM1170     [MAX_AUTO_TRAY];
extern AnsiString sMES1712     [MAX_FIX_TRAY];
extern AnsiString sMES1713     [MAX_FIX_TRAY];                                  //RogerYang 20250626 偉測不可複測bin功能
extern AnsiString sMES1720     [MAX_FIX_TRAY];
extern AnsiString sMES1721     [MAX_FIX_TRAY];
extern AnsiString sWAR1722     [MAX_FIX_TRAY];
extern AnsiString sMES1723     [MAX_FIX_TRAY];
extern AnsiString sWAR1751     [MAX_FIX_TRAY];
extern AnsiString sWAR1752     [MAX_FIX_TRAY];

extern int iC_Up          [MAX_TRACK];
extern int iC_Middle      [MAX_TRACK];
extern int iC_EdgePush    [MAX_TRACK];
extern int iTrackFloodgate[MAX_TRACK];
extern int iAutoBack      [MAX_TRACK];
extern int iAutoPush      [MAX_TRACK];
//<==
//Steven 20230907 : For HT-9011UC
extern const int C_DailyCorrelation   ;     //KaiChen 20200525 ：Daily Correlation Function
//====================================================================
extern const int SnFKPowerOff           ;
extern const int SnFKPowerOn            ;
extern const int SnFKReset              ;
extern const int SnFKPause              ;
extern const int SnFKHome               ;
extern const int SnFKStart              ;
extern const int SnFKOneCycle           ;
extern const int SnFKRetry              ;

extern const int SnFKSkip               ;
extern const int SnFKCleanOut           ;
extern const int SnFKTrayFeed           ;
extern const int SnFKTrayEnd            ;
extern const int SnFKAlarmReset         ;
extern const int SnFKCoverOpen          ;
extern const int SnRKPowerOff           ;
extern const int SnRKPowerOn            ;

extern const int SnRKReset              ;
extern const int SnRKPause              ;
extern const int SnRKHome               ;
extern const int SnRKStart              ;
extern const int SnRKOneCycle           ;
extern const int SnRKRetry              ;
extern const int SnRKSkip               ;
extern const int SnRKCleanOut           ;

extern const int SnRKTrayFeed           ;
extern const int SnRKTrayEnd            ;
extern const int SnRKAlarmReset         ;
extern const int SnRKCoverOpen          ;
extern const int SnRKManualStep         ;
extern const int SnRKManualTStart       ;
//-----------------------------------------panel sensor finish
extern const int SnLoaderTrayHasTray    ;
extern const int SnLoaderCarHasTray     ;

extern const int SnLoaderPreDete        ;

extern const int SnAuto1TrayDetect      ;
extern const int SnAuto2TrayDetect      ;
extern const int SnAuto3TrayDetect      ;

extern const int SnAuto1IsFull          ;
extern const int SnAuto2IsFull          ;
extern const int SnAuto3IsFull          ;
extern const int SnFixedTray1Detect     ;
extern const int SnFixedTray2Detect     ;
extern const int SnFixedTray3Detect     ;
extern const int SnSafeDoor1            ;
extern const int SnSafeDoor2            ;

extern const int SnSafeDoor3            ;
extern const int SnSafeDoor4            ;
extern const int SnSafeDoor5            ;
extern const int SnSafeDoor6            ;
extern const int SnSafeDoor7            ;
extern const int SnSafeDoor8            ;
extern const int SnAirIsEnough          ;
extern const int SnFrontRightEMG        ;

extern const int SnRearLeftEMG          ;
extern const int SnFMotorDown           ;
extern const int SnBMotorDown           ;
extern const int SnMotorPower           ;
extern const int SnSystemPower          ;
extern const int SnEmptyTrayHasTray1    ;
extern const int SnEmptyTrayIsFull1     ;
extern const int SnEmptyTrayIsLock1     ;

extern const int SnEmptyTrayHasTray2    ;
extern const int SnEmptyTrayIsFull2     ;
extern const int SnRearPadActive        ;
extern const int SnFrontLeftEMG         ;
extern const int SnRearRightEMG         ;
extern const int SnEPDieForce           ;
//extern const int SnCatchTrayDown        ;

extern const int SenBit0               ;
extern const int SenBit1               ;
extern const int SenBit2               ;
extern const int SenBit3               ;
extern const int SenBit4               ;
extern const int SenBit5               ;
extern const int SenBit6               ;
extern const int SenBit7               ;
extern const int SenBit8               ;
extern const int SenBit9               ;

extern const int SnAuto1_Tray_Car      ;
extern const int SnAuto2_Tray_Car      ;
extern const int SnAuto3_Tray_Car      ;

extern const int SnInPutSHT1S1           ;
extern const int SnInPutSHT1S2           ;
extern const int SnInPutSHT1S3           ;
extern const int SnInPutSHT1S4           ;
extern const int SnInPutSHT1S5           ;
extern const int SnInPutSHT1S6           ;
extern const int SnInPutSHT1S7           ;

extern const int SnInPutSHT2S1           ;
extern const int SnInPutSHT2S2           ;
extern const int SnInPutSHT2S3           ;
extern const int SnInPutSHT2S4           ;
extern const int SnInPutSHT2S5           ;
extern const int SnInPutSHT2S6           ;
extern const int SnInPutSHT2S7           ;

extern const int SnOutPutSHT1S1          ;
extern const int SnOutPutSHT1S2          ;
extern const int SnOutPutSHT1S3          ;
extern const int SnOutPutSHT1S4          ;
extern const int SnOutPutSHT1S5          ;
extern const int SnOutPutSHT1S6          ;
extern const int SnOutPutSHT1S7          ;

extern const int SnOutPutSHT2S1          ;
extern const int SnOutPutSHT2S2          ;
extern const int SnOutPutSHT2S3          ;
extern const int SnOutPutSHT2S4          ;
extern const int SnOutPutSHT2S5          ;
extern const int SnOutPutSHT2S6          ;
extern const int SnOutPutSHT2S7          ;

extern const int SnOutPutSHT1ZS1         ;
extern const int SnOutPutSHT1ZS2         ;
extern const int SnOutPutSHT2ZS1         ;
extern const int SnOutPutSHT2ZS2         ;

extern const int SnAuto1PreDete        ;
extern const int SnAuto2PreDete        ;
extern const int SnAuto3PreDete        ;
extern const int SnLoaderSureTray      ;
//extern const int SnMotorYAlarm         ;
//extern const int SnMotorZ1Alarm        ;
//extern const int SnMotorZ2Alarm        ;
extern const int SnAuto1FixCyPush      ;
extern const int SnAuto2FixCyPush      ;
extern const int SnAuto3FixCyPush      ;
extern const int SnLoaderFixCyPush     ;

extern const int SenBit10              ;
extern const int SenBit11              ;
extern const int SenBit12              ;
extern const int SenBit13              ;
extern const int SenBit14              ;
extern const int SenBit15              ;
extern const int SenBit16              ;
extern const int SenBit17              ;
extern const int SenBit18              ;
extern const int SenBit19              ;

extern const int SenEmptyHasTray                ;
extern const int SenEmptyCWDete                 ;
extern const int SenEmptySelectHasTray          ;
extern const int SenEmptyCCWDete                ;
extern const int SenEmptyCarHasTray             ;
extern const int SenColorHasTray                ;
extern const int SenColorCWDete                 ;
extern const int SenColorSelectHasTray          ;
extern const int SenColorCarHasTray             ;
extern const int SenEmptyFixCyPush              ;

extern const int SnAuto1TrayHasTray    ;
extern const int SnAuto1CWPreDetect    ;
extern const int SnHeaterDoor          ;
extern const int SnCatchTrayFix1On     ;
extern const int SnCatchTrayFix2On     ;

extern const int SnSafeLock            ;
extern const int SnRKSafeLock          ;                                        //KenHsieh 20211228 : 區分實體IO與通訊面板

extern const int SnFPLevelOpe          ;
extern const int SnFPLevelEng          ;                                        //Steven 20190503 : 指紋辨識權限
extern const int SnFPLevelSup          ;
extern const int SnFPLevelHon          ;

extern const int SnIndexHeaterFan      ;
extern const int SnHeaterDoor2         ;
extern const int SnHeaterDoor3         ;
extern const int SnHeaterDoor4         ;
extern const int SnChamberHeatDetect   ;                                        //JerryYang 20210107 : ChamberHeatDetect
extern const int SnAuto1TrackDetect    ;
extern const int SnAuto2TrackDetect    ;
extern const int SnAuto3TrackDetect    ;
extern const int SnEmptyIsFull         ;
extern const int SnColorIsFull         ;
extern const int SenColorCCWDete       ;
extern const int SenColorFixCyPush     ;

extern const int SenEmpty1HasTray      ;
extern const int SenEmpty1CCWDete      ;
extern const int SenEmpty1CarHasTray   ;

extern const int SnLoaderUpSafedetect  ;
extern const int SnEmptyUpSafedetect   ;
extern const int SnColorUpSafedetect   ;
extern const int SnAutoUpSafedetect0   ;
extern const int SnAutoUpSafedetect1   ;
extern const int SnAutoUpSafedetect2   ;

extern const int SnIonFanAlarm         ;
extern const int SnIonFanLevelAlarm    ;
extern const int SnIonBarrierAlarm     ;
extern const int SnIonBarrierLevelAlarm;
extern const int SnIonBarrierConditionAlarm;

extern const int SnAutoColorTrayDetect0;
extern const int SnAutoColorTrayDetect1;
extern const int SnAutoColorTrayDetect2;

extern const int SnLoad2IsFull            ;
extern const int SnLoad2IsPreAlarm        ;
extern const int SnLoad2TrayHasTray_AGV   ;
extern const int SnLoad2TrayHasTray_ART   ;
extern const int SnLoad2TrayHasTray       ;
extern const int SnLoad2CarHasTray        ;
extern const int SnCheckTray2Direction    ;
extern const int SenLoad2CCWDete          ;
extern const int SnLoad2UpSafedetect      ;
extern const int SenLoad2CCWDete_2        ;
extern const int SnLoad2TrackDetect       ;
extern const int SnLoad2FixCyPush         ;
extern const int SnLoad2EdgePush          ;
extern const int SnLoad2SeparateHasTray   ;
extern const int SnLoad2SureTray          ;
extern const int SnLoad2PreDete           ;
extern const int SnCheckLoad2Direction    ;
extern const int SnLoad2CasstteDetect     ;
extern const int SnDoubleLoad2Detection   ;
extern const int SnLoad2UpPress           ;

extern const int SnSafeDoor9;

extern const int SnInPutSHT1S8;
extern const int SnInPutSHT1S9;
extern const int SnInPutSHT2S8;
extern const int SnInPutSHT2S9;
extern const int SnSafeDoor10;

extern const int SnIonFan6Alarm;
extern const int SnIonFan7Alarm;
extern const int SnIonFan8Alarm;
extern const int SnIonFan9Alarm;
extern const int SnIonFan10Alarm;
extern const int SnIonFan11Alarm;

extern const int SnEPAlarm;
extern const int SnCheckTrayDirection;
extern const int SnCheckLoadDirection;

extern const int SnNegativePressureAir;
extern const int SnNegativePressureAir2;                                        //Sam 20171110 (Steven) : 新增氣壓 Sensor
//jou 2010-11-23
extern const int SnLoaderEdgePush ;
extern const int SnAuto1EdgePush  ;
extern const int SnAuto2EdgePush  ;
extern const int SnAuto3EdgePush  ;
//----- by dell ccd realtime-------------
extern const int SnRealTimeCCDStop;
extern const int SnRealTimeCCDIndexArm;
extern const int SnRTCCDTempCtrl;
//---------------------------------------
extern const int SnUnLoaderFloating ;

extern const int SnRotateCheck;                                                 //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
extern const int SnCheckConnectIndexArm_1;                                      //20111130  Dell
extern const int SnCheckConnectIndexArm_2;                                      //20111130  Dell

extern const int SnFixFloating1;                                                //Steven 20120131 : Fix Tray置偏偵測
extern const int SnFixFloating2;
extern const int SnFixFloating3;                                                //Sam 20240129 : 新增第三組 Fix floating Sensor

extern const int SnATCAlarm1;                                                   //jou 2012-03-13 ATC Alarm 1 Sensor
extern const int SnATCAlarm2;                                                   //jou 2012-03-13 ATC Alarm 2 Sensor
extern const int SnATCAlarm3;                                                   //jou 2012-03-13 ATC Alarm 3 Sensor
extern const int SnATCAlarm4;                                                   //jou 2012-03-13 ATC Alarm 4 Sensor

extern const int SenInArmYPitch60;                                              //ChungHung 20120505 : HT9045 WS Only
extern const int SenOutArmYPitch60;                                             //ChungHung 20120505 : HT9045 WS Only
//------------------------------------------------------------------------------
// 2011.05.26 , Joye , ATC Alarm ---------->>
extern const int SnATC01ControllerHighAlarm;  //Steven 20120410 : Hontech ATC
extern const int SnATC02ControllerHighAlarm;
extern const int SnATC03ControllerHighAlarm;
extern const int SnATC04ControllerHighAlarm;
extern const int SnATC01ControllerLowAlarm;
extern const int SnATC02ControllerLowAlarm;
extern const int SnATC03ControllerLowAlarm;
extern const int SnATC04ControllerLowAlarm;
// 2011.05.26 , Joye , ATC Alarm ----------<<

extern const int SenLoaderCCWDete   ;
extern const int SnLoaderIsFull     ;
extern const int SnAuto2CWPreDetect ;
extern const int SnAuto3CWPreDetect ;
extern const int SnAuto2TrayHasTray ;
extern const int SnAuto3TrayHasTray ;
extern const int SnAutoDockingOff;                                              //ChungHung 20120718 add UseAutoDocking Check Sensor
extern const int SnAutoDockingOn;                                               //ChungHung 20120718 add UseAutoDocking Check Sensor

extern const int SnTesterDocking;                                               //jou 2012-09-13 Tester Docking
extern const int SnTrain;                                                       //ChungHung 20120911 add
extern const int SnFix3FullPlace;                                               //Steven 20121020 : Fix3滿盤

//Steven 20130201 : Kasuga離子風扇電源偵測
extern const int SnIonFanPower01;
extern const int SnIonFanPower02;
extern const int SnIonFanPower03;
extern const int SnIonFanPower04;
extern const int SnIonFanPower05;
extern const int SnIonFanPower06;
extern const int SnIonFanPower07;
extern const int SnIonFanPower08;
extern const int SnIonFanPower09;
extern const int SnIonFanPower10;
extern const int SnIonFanPower11;
extern const int SnIonFanPower12;

extern const int SnIonFan12Alarm;
extern const int SnRotateRowIn1;                                                //kevin 20130524  Dell :旋轉站;馬達版
extern const int SnRotateRowIn2;
extern const int SnRotateRowOut1;
extern const int SnRotateRowOut2;

extern const int SnSocket1;                                                     //kevin 20130429  socket sensor
extern const int SnSocket2;
extern const int SnSocket3;
extern const int SnSocket4;
extern const int SnSocket5;
extern const int SnSocket6;
extern const int SnSocket7;
extern const int SnSocket8;

extern const int SnSocket9;                                                     //Steven 20200610 : Socket sensor 改成16顆
extern const int SnSocket10;
extern const int SnSocket11;
extern const int SnSocket12;
extern const int SnSocket13;
extern const int SnSocket14;
extern const int SnSocket15;
extern const int SnSocket16;

#define iSnSocketCnt          24                                                //JerryYang 20260506 : 16->24

extern const int SnCrossSHT1S1;                                                 //2013-07-16    Dell    Shuttle cross sensor
extern const int SnCrossSHT1S2;
extern const int SnCrossSHT2S1;
extern const int SnCrossSHT2S2;
extern const int SnServo;                                                       //kevin 20140121 偵測servon 訊號
extern const int SnAutoTeach;                                                   //kevin 201400512 AUTOTEACH IN/OUT ARM SENSOR
extern const int SnEOF1      ;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SnEOF2      ;
extern const int SnEOF3      ;
extern const int SnAOIResult1;
extern const int SnAOIResult2;
extern const int SnAOIResult3;
extern const int SnFix3Lock;                                                    //Steven 20140512 : For HT-9046LA

extern const int SenAutoDocking240KG;                                           //ChungHung 20140709 add for hardware control OTD
extern const int SenAutoDocking360KG;                                           //ChungHung 20140709 add for hardware control OTD

extern const int SnFixColorTrayDetect0;                                         //20140903 wei colcr Tray
extern const int SnFixColorTrayDetect1;
extern const int SnFixColorTrayDetect2;

extern const int SnLoaderColorTrayDetect;
extern const int SnEmptyColorTrayDetect;
extern const int SnColorColorTrayDetect;

extern const int SnGroundMan;                                                   //wei 20150424 add SnGroundMan偵測
extern const int SnOCRTigger;                                                   //wei 20150720 OCR觸發
extern const int SnLowSpeed;
extern const int SnOCRPosition;
extern const int SnLoaderTrayHasTray_ART;                                       //wei 20151210
extern const int SnSLK1UnclampButton;                                           //JerryYang 20160524
extern const int SnSLK2UnclampButton;
extern const int SnSocketClampPush1;                                            //JerryYang 20160606
extern const int SnSocketClampPush2;
extern const int SnSocketClampPull1;
extern const int SnSocketClampPull2;
extern const int SnSocketHasClamp1;
extern const int SnSocketHasClamp2;
extern const int SnLoaderUpPress;                                               //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray

extern const int SnSafeMode;                                                    //jou 20231016 : CE PLC safe mode
extern const int SnWaterLeakageChiller;                                         //jou 20231019 : Water Leakage Chiller
extern const int SnSmokeDetect01;                                               //Sam 20240112 : 新增煙霧偵測
extern const int SnTopBtmAOIR180;                                               //Jimmychiu 20240322 : Top & Bottom Inspect
extern const int SnFixedSeatTLOn;
extern const int SnFixedSeatTLOff;
extern const int SnFixedSeatTROn;
extern const int SnFixedSeatTROff;
extern const int SnFixedSeatBLOn;
extern const int SnFixedSeatBLOff;
extern const int SnFixedSeatBROn;
extern const int SnFixedSeatBROff;
extern const int SnTopBtmAirMaxAlarm;
extern const int SnTopBtmAirMinAlarm;
extern const int SnTopBtmRotateLockOn1;
extern const int SnTopBtmRotateLockOn2;
extern const int SnLightZORG;
extern const int SnLightZINP;
extern const int SnLightZREADY;
extern const int SnLightZSERVO;
//------------------------------------
//Steven 20161011 : TTL支援8Site
//------------------------------------
extern const int SenBit20              ;
extern const int SenBit21              ;
extern const int SenBit22              ;
extern const int SenBit23              ;
extern const int SenBit24              ;
extern const int SenBit25              ;
extern const int SenBit26              ;
extern const int SenBit27              ;
extern const int SenBit28              ;
extern const int SenBit29              ;
extern const int SenBit30              ;
extern const int SenBit31              ;
extern const int SenBit32              ;
extern const int SenBit33              ;
extern const int SenBit34              ;
extern const int SenBit35              ;
extern const int SenBit36              ;
extern const int SenBit37              ;
extern const int SenBit38              ;
extern const int SenBit39              ;

extern const int SnIndex1Connect1     ;                                         //RogerYang 20161212 : 偵測SLK獨立加熱或共用加熱
extern const int SnIndex1Connect2     ;
extern const int SnIndex2Connect1     ;
extern const int SnIndex2Connect2     ;
extern const int SnTrayCover          ;                                         //Steven 20170623 (wei) : Add for catch tray with cover

extern const int SnLoaderIsPreAlarm   ;                                         //wei 20170802 Pre alarm sensor
extern const int SnEmptyIsPreAlarm    ;
extern const int SnColorIsPreAlarm    ;
extern const int SnAuto1IsPreAlarm    ;
extern const int SnAuto2IsPreAlarm    ;
extern const int SnAuto3IsPreAlarm    ;
extern const int SnTrayArmSafePos     ;                                         //kevin 20171006 (wei) Home tray arm must on
extern const int SnTJCurrent          ;                                         //Steven 20180124 : Check ATC7.0 TJ Current
extern const int SnEmptyFull          ;                                         //wei 20170504 Use Empty Full Put Color
extern const int SnCassetteArmHave    ;                                         //wei 20180702 MR
extern const int SnBuffer1HaveCassette;
extern const int SnBuffer2HaveCassette;
extern const int SnBuffer3HaveCassette;
extern const int SnBuffer4HaveCassette;
extern const int SnBuffer5HaveCassette;
extern const int SnBuffer6HaveCassette;
extern const int SnBuffer7HaveCassette;
extern const int SnBuffer8HaveCassette;
extern const int SnBuffer9HaveCassette;
extern const int SnBuffer10HaveCassette;
extern const int SnLoadPortCatch       ;
extern const int SnCassetteCatch       ;

extern const int SnLoadPortPresent     ;
extern const int SnLoadPortPlacement1  ;
extern const int SnLoadPortPlacement2  ;
extern const int SnTrayBracketHave     ;
extern const int SnStackedTrayHave     ;
extern const int SnBuffer6HaveTray     ;

extern const int SnE84VALID            ;                                        //wei 20180702 E84
extern const int SnE84CS0              ;
extern const int SnE84CS1              ;
extern const int SnE84AMAVBL           ;
extern const int SnE84TRREQ            ;
extern const int SnE84BUSY             ;
extern const int SnE84COMPT            ;
extern const int SnE84CONT             ;
extern const int SnE84GO               ;

extern const int SnHingeTopTray        ;                                        //wei 20170405
extern const int SnSafeDoor11          ;
extern const int SnSafeDoor12          ;
extern const int SnSafeDoor13          ;
extern const int SnSafeDoor14          ;
extern const int SnSafeDoor15          ;

extern const int SnCassette01          ;                                        //wei 20180702 MR
extern const int SnCassette02          ;
extern const int SnCassette03          ;
extern const int SnCassette04          ;
extern const int SnCassette05          ;
extern const int SnCassette06          ;
extern const int SnCassette07          ;
extern const int SnCassette08          ;
extern const int SnCassette09          ;
extern const int SnCassette10          ;
extern const int SnMRStart             ;
extern const int SnMRPause             ;
extern const int SnMRUp                ;
extern const int SnMRDown              ;

//Sam 20190112 LM
//==>
extern const int SnCassetteArmCatch  ;
extern const int SnLoadPortHaveCassette1_1  ;
extern const int SnLoadPortHaveCassette1_2  ;
extern const int SnLoadPortHaveCassette2_1  ;
extern const int SnLoadPortHaveCassette2_2  ;
extern const int SnLoadPortHaveCassette3_1  ;
extern const int SnLoadPortHaveCassette3_2  ;
extern const int SnLoadPortHaveCassette4_1  ;
extern const int SnLoadPortHaveCassette4_2  ;
extern const int SnLoadRobotHaveCassette1   ;
extern const int SnLoadRobotHaveCassette2   ;
extern const int SnLoadRobotPosUp           ;
extern const int SnLoadRobotPosDown         ;

extern const int SnUnloadPortHaveCassette1_1;
extern const int SnUnloadPortHaveCassette1_2;
extern const int SnUnloadPortHaveCassette2_1;
extern const int SnUnloadPortHaveCassette2_2;
extern const int SnUnloadPortHaveCassette3_1;
extern const int SnUnloadPortHaveCassette3_2;
extern const int SnUnloadPortHaveCassette4_1;
extern const int SnUnloadPortHaveCassette4_2;
extern const int SnUnloadRobotHaveCassette1 ;
extern const int SnUnloadRobotHaveCassette2 ;
extern const int SnUnloadRobotPosUp         ;
extern const int SnUnloadRobotPosDown       ;

extern const int SnSafeDoor16               ;
extern const int SnSafeDoor17               ;
extern const int SnSafeDoor18               ;
extern const int SnSafeDoor19               ;
//<==
//Sam 20190112 LM

extern const int SnPreciserDetect1    ;     //Frank 20180410 (Steven) : InArm Preciser Station
extern const int SnPreciserDetect2    ;
extern const int SnLoadCell1          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell2          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell3          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell4          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell5          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell6          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell7          ;    //kevin 20190307  read load cell pass  or fail
extern const int SnLoadCell8          ;    //kevin 20190307  read load cell pass  or fail

extern const int SnHotGun1;                 //kevin 20190621  hot gun 流量異常 使用 Out 2
extern const int SnHotGun2;                 //kevin 20190621  hot gun 流量異常 使用 Out 2

extern const int SnGroundMan2         ;
extern const int SnGroundMan3         ;
extern const int SnGroundMan4         ;
extern const int SnAuto1UpPress       ;
extern const int SnAuto2UpPress       ;
extern const int SnAuto3UpPress       ;
extern const int SnWaterLeakageUp     ;     //wei 20190617 漏水檢測
extern const int SnWaterLeakageDown   ;     //wei 20190617 漏水檢測
extern const int SnWaterLeakagePlate  ;     //wei 20190617 漏水檢測
extern const int SnLoaderTrackDetect  ;     //Sam 20200316 : Loader Detect Tray
extern const int SnAuto1Z_Select1     ;    //JerryYang 20200615 ART分離汽缸sensor
extern const int SnAuto1Z_Select2     ;
extern const int SnAuto1Z_Select3     ;
extern const int SnAuto1Z_Select4     ;
extern const int SnAuto2Z_Select1     ;
extern const int SnAuto2Z_Select2     ;
extern const int SnAuto2Z_Select3     ;
extern const int SnAuto2Z_Select4     ;
extern const int SnAuto3Z_Select1     ;
extern const int SnAuto3Z_Select2     ;
extern const int SnAuto3Z_Select3     ;
extern const int SnAuto3Z_Select4     ;
extern const int SnTesterDryAir       ;   //Ifor 20200115 add: add Tester Dry Air Control

extern const int SnInFlipper1DeviceDetect ;
extern const int SnInFlipper2DeviceDetect ;
extern const int SnInFlipper3DeviceDetect ;

extern const int SnOutFlipper1DeviceDetect ;
extern const int SnOutFlipper2DeviceDetect ;
extern const int SnOutFlipper3DeviceDetect ;

extern const int SnLoaderCasstteDetect    ;
extern const int SnEmptyCasstteDetect     ;
extern const int SnColorCasstteDetect     ;
extern const int SnAuto1CasstteDetect     ;
extern const int SnAuto2CasstteDetect     ;
extern const int SnAuto3CasstteDetect     ;
extern const int SnAuto4CasstteDetect     ;                                       //RogerYang 20250825 : 新增Auto4~6
extern const int SnAuto5CasstteDetect     ;
extern const int SnAuto6CasstteDetect     ;
extern const int SnArm1SLK                ;
extern const int SnArm2SLK                ;

extern const int SnMRAuto21     ;   //wei 20180702 MR
//E84驗證用
extern const int SnE84LREQ      ;   //wei 20180702 E84
extern const int SnE84UREQ      ;   //wei 20180702 E84
extern const int SnE84VA        ;   //wei 20180702 E84
extern const int SnE84READY     ;   //wei 20180702 E84
extern const int SnE84VS0       ;   //wei 20180702 E84
extern const int SnE84VS1       ;   //wei 20180702 E84
extern const int SnE84HOAVBL    ;   //wei 20180702 E84
extern const int SnE84ES        ;   //wei 20180702 E84
extern const int SnE84POWER     ;   //wei 20180702 E84
extern const int SnBufferTop    ;   //wei 20200305 MR
extern const int SnBufferCatch  ;   //wei 20200305 MR
extern const int SnCassetteOpen ;
//KaiChen 20200716 ：OHT
//==>
extern const int SnMultileEmpty_Door               ;
extern const int SnMultileEmpty_MagazineTop        ;
extern const int SnMultileEmpty_MagazineLow        ;
extern const int SnMultileEmpty_MagazineCatch      ;
extern const int SnMultileEmpty_SelectHasTray      ;
extern const int SnMultileEmpty_HasTray            ;
extern const int SnMultileEmpty_CornerPushCyPush   ;
extern const int SnMultileEmpty_CatchHasTray       ;
extern const int SnSafeDoor20                      ;
extern const int SnSafeDoor21                      ;
extern const int SnLoadPort2Present                ;
extern const int SnLoadPort2Placement1             ;
extern const int SnLoadPort2Placement2             ;
extern const int SnTrayBracket2Have                ;
extern const int SnCassetteArmIntoLoadPort         ;
extern const int SnCassetteArmIntoLoadPort2        ;
extern const int SnBuffer7HaveTray                 ;
extern const int SnLoadPortAutoManual              ;
extern const int SnLoadPort2AutoManual             ;
extern const int SnOHTIntoLoadPort                 ;
extern const int SnOHTIntoLoadPort2                ;
extern const int SnCassetteClose                   ;
extern const int SnCassetteOpen2                   ;
extern const int SnCassetteClose2                  ;
extern const int SnCassetteSlotMap                 ;
extern const int SnCassetteFloating                ;
extern const int SnMultileEmpty_ScanTrayID         ;
extern const int SnE84_2_VALID              ;
extern const int SnE84_2_CS0                ;
extern const int SnE84_2_CS1                ;
extern const int SnE84_2_AMAVBL             ;
extern const int SnE84_2_TRREQ              ;
extern const int SnE84_2_BUSY               ;
extern const int SnE84_2_COMPT              ;
extern const int SnE84_2_CONT               ;
extern const int SnE84_2_GO                 ;
extern const int SnE84_1_VALID              ;
extern const int SnE84_1_CS0                ;
extern const int SnE84_1_CS1                ;
extern const int SnE84_1_AMAVBL             ;
extern const int SnE84_1_TRREQ              ;
extern const int SnE84_1_BUSY               ;
extern const int SnE84_1_COMPT              ;
extern const int SnE84_1_CONT               ;
extern const int SnE84_1_GO                 ;
extern const int SnMultileEmptyCatch_HasTray;
extern const int SnCSTHoldDownOff              ;
extern const int SnCSTHoldDown2Off             ;
extern const int SnMultileEmpty_ScanTray2DID   ;
extern const int SnDieDetect_Sh1               ;
extern const int SnDieDetect_Sh2               ;
extern const int SnBuffer1_Placement1          ;
extern const int SnBuffer2_Placement1          ;
extern const int SnBuffer3_Placement1          ;
extern const int SnBuffer4_Placement1          ;
extern const int SnBuffer5_Placement1          ;
extern const int SnBuffer6_Placement1          ;
extern const int SnBuffer7_Placement1          ;
extern const int SnBuffer8_Placement1          ;
extern const int SnBuffer9_Placement1          ;
extern const int SnBuffer10_Placement1         ;
extern const int SnStackedTrayYDetect_Loader   ;
extern const int SnStackedTrayYDetect_Elevator2;
extern const int SnMCUSensor1                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader
extern const int SnMCUSensor2                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader
extern const int SnMCUSensor3                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader
extern const int SnMCUSensor4                 ;//Jimmychiu 20230630 : add color sensor MU-N in Loader

extern const int SnSht1Left                   ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnSht1Right                  ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnSht2Left                   ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnSht2Right                  ;   //kevin 20220512 add SHUTTLE Left 位置偏移
extern const int SnLoaderTrayHasTray_AGV      ;   //kevin 20220520 add AGV load
extern const int SnEmptyTrayHasTray_AGV       ;   //kevin 20220520 add AGV Empty
extern const int SnColorTrayHasTray_AGV       ;   //kevin 20220520 add AGV Color
extern const int SnAseTrayBufferLeft          ;   //kevin 20220709 ASEKH 左邊放空TRAY
extern const int SnAseTrayBufferRight         ;   //kevin 20220709 ASEKH 右邊放空TRAY

extern const int SnDailyCorrelation_Open  ;   //KaiChen 20200525 ：Daily Correlation Function
extern const int SnDailyCorrelation_Close ;   //KaiChen 20200525 ：Daily Correlation Function
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
//ChungHung 20210113 add for Alignment CCD start
extern const int SnInAreaAlignmentSenX ;
extern const int SnInAreaAlignmentSenY ;
extern const int SnOutAreaAlignmentSenX;
extern const int SnOutAreaAlignmentSenY;
//ChungHung 20210113 add for Alignment CCD end
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//Jimmychiu 20210902 add: ATC Winway IO ready
//==>
extern const int SnATC1Ready          ;
extern const int SnATC2Ready          ;
extern const int SnATC3Ready          ;
extern const int SnATC4Ready          ;
//<==
//Jimmychiu 20210902 add: ATC Winway IO ready

//Ztex 2023.04.13 Add HT-1032 IO ==>
extern const int SnPlate1TempOverDetect  ;
extern const int SnPlate2TempOverDetect  ;
extern const int SnShuttle1TempOverDetect;
extern const int SnShuttle2TempOverDetect;
extern const int SnHead1TempOverDetect   ;
extern const int SnHead2TempOverDetect   ;
extern const int SnHead5TempOverDetect   ;
extern const int SnHead6TempOverDetect   ;
extern const int SnHumidityAnomaly1Detect;
extern const int SnHumidityAnomaly2Detect;
extern const int SnHumidityAnomaly3Detect;
extern const int SnDryAirIsEnough        ;
extern const int SnIonBarInAirIsEnough   ;
extern const int SnIonBarOutAirIsEnough  ;
extern const int SnSafeDoor1Hatchway;
extern const int SnSafeDoor2Hatchway;
extern const int SnSafeDoor3Hatchway;
extern const int SnSafeDoor4Hatchway;
extern const int SnSafeDoor5Hatchway;
extern const int SnSafeDoor6Hatchway;
extern const int SnSafeDoor7Hatchway;
extern const int SnSafeDoor8Hatchway;
extern const int SnSafeDoor9Hatchway;
extern const int SnSafeDoor10Hatchway;
extern const int SnSafeDoor11Hatchway;
extern const int SnSafeDoor6PosFixPickPlace;
extern const int SnTriTempSafeDoor6Lock;
//Ztex 2023.04.13 Add HT-1032 IO <==
extern const int SnTrayArmZSafePos;
extern const int SnEnhaustAirFanAlarmDetect;//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air
extern const int SnDockingAreaOpenCheck;//Ztex 2023.05.02 Add HT-1032 IO Docking Area Open Check
extern const int SnDewPointDetectIndexArm1 ;   //Hmy 20170603  add By 三溫機 露點SENSOR Arm1偵測
extern const int SnDewPointDetectIndexArm2 ;   //Hmy 20170603  add By 三溫機 露點SENSOR Arm2偵測

extern const int SnLoaderSeparateHasTray ;
extern const int SnEmptySeparateHasTray  ;
extern const int SnColorSeparateHasTray  ;
extern const int SnAuto1SeparateHasTray  ;
extern const int SnAuto2SeparateHasTray  ;
extern const int SnAuto3SeparateHasTray  ;

extern const int SenLoaderCCWDete_2      ;
extern const int SenEmptyCCWDete_2       ;
extern const int SenColorCCWDete_2       ;
extern const int SenAuto1CCWDete_2       ;
extern const int SenAuto2CCWDete_2       ;
extern const int SenAuto3CCWDete_2       ;

extern const int SnOpenDoorChangeKit1    ;
extern const int SnOpenDoorChangeKit2    ;
extern const int SnOpenDoorChangeKit3    ;

extern const int SnIndexCylinderDetectHead1 ;
extern const int SnIndexCylinderDetectHead2 ;
extern const int SnIndexCylinderDetectHead5 ;
extern const int SnIndexCylinderDetectHead6 ;

extern const int SnIonBar1               ;
extern const int SnIonBar2               ;
extern const int SnIonBar3               ;
extern const int SnIonBar4               ;
extern const int SnIonBar5               ;
extern const int SnIonBar6               ;
extern const int SnIonBar7               ;
extern const int SnIonBar8               ;
extern const int SnIonBar9               ;

extern const int SnTrayArmHasTray        ;
extern const int SnInAreaAlignmentSenZ   ;                                      //KenHsieh 20211110 ： AutoAlignment add Auto Z
extern const int SnOutAreaAlignmentSenZ  ;                                      //KenHsieh 20211110 ： AutoAlignment add Auto Z

extern const int SnAuto4_Tray_Car          ;                                       //Steven 20230907 : For HT-9011UC
extern const int SnAuto4PreDete            ;
extern const int SnAuto4TrackDetect        ;
extern const int SnAuto4FixCyPush          ;
extern const int SnAuto4EdgePush           ;
extern const int SnAuto4UpSafedetect       ;
extern const int SnAuto4SeparateHasTray    ;
extern const int SnAuto4TrayHasTray        ;
extern const int SnAuto4CWPreDetect        ;
extern const int SenAuto4CCWDete_2         ;
extern const int SnAuto4Z_Select1          ;
extern const int SnAuto4Z_Select2          ;
extern const int SnAuto4Z_Select3          ;
extern const int SnAuto4Z_Select4          ;
extern const int SnAuto4IsPreAlarm         ;
extern const int SnAuto4IsFull             ;
extern const int SnAuto4UpPress            ;
extern const int SnAutoColorTrayDetect4    ;

extern const int SnAuto5_Tray_Car          ;
extern const int SnAuto5PreDete            ;
extern const int SnAuto5TrackDetect        ;
extern const int SnAuto5FixCyPush          ;
extern const int SnAuto5EdgePush           ;
extern const int SnAuto5UpSafedetect       ;
extern const int SnAuto5SeparateHasTray    ;
extern const int SnAuto5TrayHasTray        ;
extern const int SnAuto5CWPreDetect        ;
extern const int SenAuto5CCWDete_2         ;
extern const int SnAuto5Z_Select1          ;
extern const int SnAuto5Z_Select2          ;
extern const int SnAuto5Z_Select3          ;
extern const int SnAuto5Z_Select4          ;
extern const int SnAuto5IsPreAlarm         ;
extern const int SnAuto5IsFull             ;
extern const int SnAuto5UpPress            ;
extern const int SnAutoColorTrayDetect5    ;

extern const int SnAuto6_Tray_Car          ;
extern const int SnAuto6PreDete            ;
extern const int SnAuto6TrackDetect        ;
extern const int SnAuto6FixCyPush          ;
extern const int SnAuto6EdgePush           ;
extern const int SnAuto6UpSafedetect       ;
extern const int SnAuto6SeparateHasTray    ;
extern const int SnAuto6TrayHasTray        ;
extern const int SnAuto6CWPreDetect        ;
extern const int SenAuto6CCWDete_2         ;
extern const int SnAuto6Z_Select1          ;
extern const int SnAuto6Z_Select2          ;
extern const int SnAuto6Z_Select3          ;
extern const int SnAuto6Z_Select4          ;
extern const int SnAuto6IsPreAlarm         ;
extern const int SnAuto6IsFull             ;
extern const int SnAuto6UpPress            ;
extern const int SnAutoColorTrayDetect6    ;

extern const int SnFixedTray4Detect        ;
extern const int SnFixedTray5Detect        ;
extern const int SnFixedTray6Detect        ;
extern const int SnFix5ColorTrayDetect     ;
extern const int SnFix4ColorTrayDetect     ;
extern const int SnFix6ColorTrayDetect     ;
extern const int SnAuto4TrayDetect         ;
extern const int SnAuto5TrayDetect         ;
extern const int SnAuto6TrayDetect         ;

extern const int SnTrayArmTrayDetect1      ;
extern const int SnTrayArmTrayDetect2      ;
extern const int SnTrayArmTrayDetect3      ;
extern const int SnTrayArmTrayDetect4      ;

extern const int SnIonBar10                ;
extern const int SnIonBar11                ;

extern const int SnElectricControlBox      ;                                    //ChungHung 20230718 add for Safe plc
extern const int SnAllSafeDoor             ;                                    //ChungHung 20230718 add for Safe plc
extern const int SnAllEMG                  ;                                    //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門

extern const int SnTesterAlarm  ;
extern const int SnDoubleLoadDetection     ;                                    //Steven 20240426 : 偵測loader疊盤

extern const int SnAuto1HasCoverTray;                                           //JerryYang 20241021 : Unloader增加第二組Sensor檢查是否有cover tray
extern const int SnAuto2HasCoverTray;
extern const int SnAuto3HasCoverTray;
extern const int SnAuto4HasCoverTray;
extern const int SnAuto5HasCoverTray;
extern const int SnAuto6HasCoverTray;
extern const int SnLoadIonGun;                                                  //Ifor 20230427 add:Loader Ionizer Gun
extern const int SnLoadLightGat;                                                //Ifor 20230427 add:LD/ULD light gate

extern int SnAutoTrayCar       [MAX_AUTO_TRAY];
extern int SnAutoZSelect1      [MAX_AUTO_TRAY];
extern int SnAutoZSelect2      [MAX_AUTO_TRAY];
extern int SnAutoZSelect3      [MAX_AUTO_TRAY];
extern int SnAutoZSelect4      [MAX_AUTO_TRAY];
extern int SnAutoPreDete       [MAX_AUTO_TRAY];
extern int SnAutoUpSafe        [MAX_AUTO_TRAY];
extern int SnAutoTrackDetect   [MAX_AUTO_TRAY];
extern int SnAutoTrayDetect    [MAX_AUTO_TRAY];
extern int SnAutoColorTrayDete [MAX_AUTO_TRAY];
extern int SnAutoSeparate      [MAX_AUTO_TRAY];
extern int SnAutoFixCyPush     [MAX_AUTO_TRAY];
extern int SnAutoEdgePush      [MAX_AUTO_TRAY];
extern int SnAutoTrayHasTray   [MAX_AUTO_TRAY];
extern int SnAutoCWPreDetect   [MAX_AUTO_TRAY];
extern int SnAutoCCWDete       [MAX_AUTO_TRAY];
extern int SnAutoIsPreAlarm    [MAX_AUTO_TRAY];
extern int SnAutoIsFull        [MAX_AUTO_TRAY];
extern int SnAutoUpPress       [MAX_AUTO_TRAY];
extern int SnAutoHasCoverTray  [MAX_AUTO_TRAY];

extern int SnAutoBoatActDetect [2];                                             //RogerYang 20260202 : Add for CR

extern int iC_SeparateRL       [MAX_AUTO_TRAY];
extern int iC_SeparateFL       [MAX_AUTO_TRAY];
extern int iC_SeparateRR       [MAX_AUTO_TRAY];
extern int iC_SeparateFR       [MAX_AUTO_TRAY];

extern int SnFixColorTrayDete  [MAX_FIX_TRAY];
extern int SnFixedTrayDetect   [MAX_FIX_TRAY];

extern const int iSafeDoor[MAX_SAFE_DOOR_CNT];                                  //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern const int iSafeDoorPosition[MAX_SAFE_DOOR_CNT];                          //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern AnsiString asSafeDoorAlarm[MAX_SAFE_DOOR_CNT];                           //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern const int iSafeDoorHatchway[MAX_HATCH_DOOR_CNT];
extern const int iHatchwaySafeDoorPosition[MAX_HATCH_DOOR_CNT];
extern AnsiString asHatchwaySafeDoorAlarm[MAX_HATCH_DOOR_CNT];
extern const int iIonFan[MAX_IONFAN]       ;                                    //Steven 20100226
extern const int iHTIonBar[MAX_HTIONFAN]   ;                                    //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
extern const int iIonFanPower[MAX_IONFAN]  ;                                    //Steven 20130201 : Kasuga離子風扇電源偵測
extern const int iIonBar[MAX_IONBAR]       ;                                    //Ztex 2024.12.22 Add For HT1032AT IonBar
extern const int BackSenBit0               ;
extern const int BackSenBit1               ;
extern const int BackSenBit2               ;
extern const int BackSenBit3               ;
extern const int BackSenBit4               ;
extern const int BackSenBit5               ;
extern const int BackSenBit6               ;
extern const int BackSenBit7               ;
extern const int BackSenBit8               ;
extern const int BackSenBit9               ;
extern const int BackSenBit10              ;
extern const int BackSenBit11              ;
extern const int BackSenBit12              ;
extern const int BackSenBit13              ;
extern const int BackSenBit14              ;
extern const int BackSenBit15              ;
extern const int BackSenBit16              ;
extern const int BackSenBit17              ;
extern const int BackSenBit18              ;
extern const int BackSenBit19              ;

//Alick 20161011 (Steven) : TTL支援8Site
extern const int BackSenBit20              ;
extern const int BackSenBit21              ;
extern const int BackSenBit22              ;
extern const int BackSenBit23              ;
extern const int BackSenBit24              ;
extern const int BackSenBit25              ;
extern const int BackSenBit26              ;
extern const int BackSenBit27              ;
extern const int BackSenBit28              ;
extern const int BackSenBit29              ;
extern const int BackSenBit30              ;
extern const int BackSenBit31              ;
extern const int BackSenBit32              ;
extern const int BackSenBit33              ;
extern const int BackSenBit34              ;
extern const int BackSenBit35              ;
extern const int BackSenBit36              ;
extern const int BackSenBit37              ;
extern const int BackSenBit38              ;
extern const int BackSenBit39              ;

extern const int SnMagazineSafeDoor               ; //JerryYang 20220909 : add magazine
extern const int SnMagazineDetect                 ;
extern const int SnMagazineTrackDetect            ;
extern const int SnMagazineTrackDetect2           ; //Sam 20221116 : Magazine TrayArm 自動補 Tray
extern const int SnMagazineTrackSelectDetect      ;
extern const int SnMagazineSafeDoor2              ;
extern const int SnMagazineSafeDoor3              ;
extern const int SnMagazineDetectTop              ;
extern const int SnMagazineHasTrayInside          ;
extern const int SnEPDetect                       ; //kevin 20230608 EP 流量計 偵測
//Ifor 20211005 add Tray 載盤上升下降前判斷是否有異常
//==>
extern const int SnLoader_Detect            ;
extern const int SnEmpty_Detect             ;
extern const int SnColor_Detect             ;
extern const int SnAuto1_Detect             ;
extern const int SnAuto2_Detect             ;
extern const int SnAuto3_Detect             ;
extern const int SnAuto4_Detect             ;
extern const int SnAuto5_Detect             ;
extern const int SnAuto6_Detect             ;
//<==
//Ifor 20211005 add Tray 載盤上升下降前判斷是否有異常
extern const int SnIonFanCar                ;                                   //Ifor 20220310 add: Bin Car Ion Fan Check
extern const int SnIonFanCarPower           ;                                   //Ifor 20220816 add: Bin Car Ion Fan Power Check
extern const int SnEPFlowmeter;                                                 //Ifor 20240326 add: EP流量計監控

extern const int SnSocket17              ;
extern const int SnSocket18              ;
extern const int SnSocket19              ;
extern const int SnSocket20              ;
extern const int SnSocket21              ;
extern const int SnSocket22              ;
extern const int SnSocket23              ;
extern const int SnSocket24              ;
extern const int SnSocket25               ;
extern const int SnSocket26               ;
extern const int SnSocket27               ;
extern const int SnSocket28               ;
extern const int SnSocket29               ;
extern const int SnSocket30               ;
extern const int SnSocket31               ;
extern const int SnSocket32               ;
extern const int SnChamberDryAir         ;                                      //Ifor 20240919 add: Chamber Dry Air
extern const int SnLoadCarRFIDSW         ;                                      //RogerYang 20250828 : add for Loader Rotate Arm

extern const int SnLoaderCarrier1        ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto1Carrier1         ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto2Carrier1         ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnLoaderCarrier2        ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto1Carrier2         ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto2Carrier2         ;                                      //Ifor 20251216 add:Boat Carrier

extern const int SnLoaderBoatActDetect   ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto1BoatActDetect    ;                                      //Ifor 20251216 add:Boat Carrier
extern const int SnAuto2BoatActDetect    ;                                      //Ifor 20251216 add:Boat Carrier

//===================================================================
extern const int SwFKPowerOff                  ;
extern const int SwFKPowerOn                   ;
extern const int SwFKReset                     ;
extern const int SwFKPause                     ;
extern const int SwFKHome                      ;
extern const int SwFKStart                     ;
extern const int SwFKOneCycle                  ;
extern const int SwFKRetry                     ;

extern const int SwFKSkip                      ;
extern const int SwFKCleanOut                  ;
extern const int SwFKTrayFeed                  ;
extern const int SwFKTrayEnd                   ;
extern const int SwFKAlarmReset                ;
extern const int SwFKCoverOpen                 ;
extern const int SwRKPowerOff                  ;
extern const int SwRKPowerOn                   ;

extern const int SwRKReset                     ;
extern const int SwRKPause                     ;
extern const int SwRKHome                      ;
extern const int SwRKStart                     ;
extern const int SwRKOneCycle                  ;
extern const int SwRKRetry                     ;
extern const int SwRKSkip                      ;
extern const int SwRKCleanOut                  ;

extern const int SwRKTrayFeed                  ;
extern const int SwRKTrayEnd                   ;
extern const int SwRKAlarmReset                ;
extern const int SwRKCoverOpen                 ;
extern const int SwRKManualStep                ;
extern const int SwRKManualTStart              ;
extern const int SwTowerRed                    ;
extern const int SwTowerGreen                  ;

extern const int SwTowerYellow                 ;
extern const int SwFMotorBreaker               ;
extern const int SwBMotorBreaker               ;
extern const int SwMotorRelay                  ;
extern const int SwHeaterRelay                 ;
extern const int SwMusic1                      ;
extern const int SwMusic2                      ;
extern const int SwMusic3                      ;

extern const int SwMusic4                      ;
extern const int SwTestPassLed                 ;
extern const int SwTestFailLed                 ;
extern const int SwFrontActiveLed              ;
extern const int SwRearActiveLed               ;

extern const int SwClear0                      ;
extern const int SwClear1                      ;
extern const int SwStart0                      ;
extern const int SwStart1                      ;
extern const int SwACTrayY                     ;
extern const int SwACAuto1                     ;
extern const int SwACAuto2                     ;
extern const int SwACAuto3                     ;
extern const int SwServerON                    ;
extern const int SwManualZ1                    ;
extern const int SwManualZ2                    ;
extern const int SwACEmptyCW                   ;
extern const int SwACEmptyCCW                  ;
extern const int SwACColorCW                   ;
extern const int SwClear2                      ;
extern const int SwClear3                      ;
extern const int SwStart2                      ;
extern const int SwStart3                      ;
extern const int SwCCDAir                      ;
extern const int SwACAutoCW                    ;  // for auto as loader use
extern const int SwACAuto1Mode                 ;  // for auto as loader use
extern const int SwSiteMode0                   ;
extern const int SwSiteMode1                   ;
extern const int SwSiteMode2                   ;
extern const int SwSiteMode3                   ;
extern const int SwAuto1SelectSlow             ;
extern const int SwCCDDestroy                  ;
extern const int SwReadTorue                   ;
extern const int SwBigFan                      ;
extern const int Sw10Bit                       ;
extern const int SwDut0                        ;
extern const int SwDut1                        ;
extern const int SwDut2                        ;
extern const int SwDut3                        ;
extern const int SwSafeLock                    ;
extern const int SwRKSafeLock                  ;    //KenHsieh 20211228 : 區分實體IO與通訊面板
extern const int SwCCDLight                    ;
extern const int SwHeaterFan                   ;
extern const int SwSocketClean                 ;
extern const int SwACColorCCW                  ;
extern const int SwShuttleFan                  ;
extern const int SwTesterDoubleContact         ;
extern const int SwTesterPower                 ;
extern const int SwACEmpty1CW                  ;
extern const int SwACEmpty1CCW                 ;
extern const int SwDutHeaterCoolFan            ;
extern const int SwLoaderBin                   ;
extern const int SwEmpty1Bin                   ;
extern const int SwEmpty2Bin                   ;
extern const int SwAuto1Bin                    ;
extern const int SwAuto2Bin                    ;
extern const int SwAuto3Bin                    ;
//Steven 20090917 Start
extern const int SwFix1Bin                     ;
extern const int SwFix2Bin                     ;
extern const int SwFix3Bin                     ;
extern const int SwFix4Bin                     ;
extern const int SwFix5Bin                     ;
extern const int SwFix6Bin                     ;
//Steven 20090916 Start
extern const int SwZ1SuckMode0                 ;
extern const int SwZ1SuckMode1                 ;
extern const int SwZ2SuckMode0                 ;
extern const int SwZ2SuckMode1                 ;
//Steven 20090916 End
extern const int SwShuttleCooling              ;    //jou 2010-06-09
extern const int SwCCDCooling                  ;    //Steven 20110705
extern const int SwEpArm1                      ;    //Steven 20110708
extern const int SwEpArm2                      ;    //Steven 20110708
extern const int SwHeaterFanSpeed              ;    //Steven 20110725
extern const int SwRotateCheckClear            ;    //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor

//Dell 20111111 Start : 加入Digital E/P
extern const int SwEP_D0;
extern const int SwEP_D1;
extern const int SwEP_D2;
extern const int SwEP_D3;
extern const int SwEP_D4;
extern const int SwEP_D5;
extern const int SwEP_D6;
extern const int SwEP_D7;
extern const int SwEP_D8;
extern const int SwEP_D9;
//Dell 20111111 End
extern const int SwCoolingFan_Blower;                                           //20111130  Dell
extern const int SwSafeDoorLock;                                                //20111130  Dell
extern const int SwIndexIonFan;                                                 //jou 2012-03-13 index離子槍出風開關控制

extern const int SwIndexChangeToque1;                                           //jou 2012-06-21 Enable index I/O Change Toque
extern const int SwIndexChangeToque2;                                           //jou 2012-06-21 Enable index I/O Change Toque
extern const int SwACLoaderCCW;                                                 //Loader退Tray
extern const int SwACAuto2CW;                                                   //Auto2進Tray
extern const int SwACAuto3CW;                                                   //Auto3進Tray

extern const int SwHeatGun;                                                     //ChungHung 20121107 add
extern const int SwCDAGun;                                                      //Steven 20181012 : 使用熱風槍吹冷風
extern const int SwVacuumPumpTogetherOn;                                        //Dell  for HT9046LS 雙幫浦模式
extern const int SwAirConditioner;                                              //Steven 20131011 : 冷氣機
extern const int SwLoadCellA;                                                   //kevin 20190306  add load Cell  read 1
extern const int SwLoadCellB;                                                   //kevin 20190306  add load Cell  read 2

extern const int SwHotplateCooling;                                             //jou 2013-11-07
extern const int SwCarRecord;                                                   //wei 2013-12-09
extern const int SwIonFanClean;                                                 //Isaac 20210609 : IO觸發IonFan清針

extern const int SwLoad2Bin      ;                                              //Steven 20240822 : For HT-9046AU
extern const int SwLoad2AirClean ;
extern const int SwACLoad2CCW    ;
extern const int SwACTray2Y      ;
extern const int SwLoad2Vibration;

extern const int SwUnDock;                                                      //Steven 20140310 : One Touch Docking
extern const int SwDockError;                                                   //Steven 20140310 : One Touch Docking
extern const int SwStartTest1;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SwStartTest2;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SwStartTest3;                                                  //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern const int SwOCRTigger;                                                   //wei 20150720 OCR觸發

extern const int SwTesterAirCooling;                                            //jou 2016-01-07 Tester Air Cooling Switch
extern const int SwFanDirection;                                                //wei 20160215大風扇方向
extern const int SwIonRelay;                                                    //Ifor 20190114 : add Chamber 開啟時 關閉 Ion 離子槍 吹氣

//------------------------------------
//Steven 20161011 : TTL支援8Site
//------------------------------------
extern const int SwClear4                      ;
extern const int SwClear5                      ;
extern const int SwClear6                      ;
extern const int SwClear7                      ;
extern const int SwStart4                      ;
extern const int SwStart5                      ;
extern const int SwStart6                      ;
extern const int SwStart7                      ;
extern const int SwDut4                        ;
extern const int SwDut5                        ;
extern const int SwDut6                        ;
extern const int SwDut7                        ;
extern const int Sw10Bit2                      ;
extern const int SwIonBarPower                 ;                                //kevin 20170310 (wei) Ion bar power
extern const int SwShuttleVibration1           ;                                //JerryYang 20171006 (wei) Shuttle 震動馬達
extern const int SwShuttleVibration2           ;
extern const int SwPurgeAir                    ;                                //kevin 20180928 add load board blower
extern const int SwDieClean                    ;
extern const int SwDieCleanSuck                ;                                //wei 20210326
extern const int SwLightOff                    ;
extern const int SwAirOff                      ;
extern const int SwCaselevatorZ                ;
extern const int SwTrayBracketZ                ;
extern const int SwE84VALID                    ;
extern const int SwE84CS0                      ;
extern const int SwE84CS1                      ;
extern const int SwE84AMAVBL                   ;
extern const int SwE84TRREQ                    ;
extern const int SwE84BUSY                     ;
extern const int SwE84COMPT                    ;
extern const int SwE84CONT                     ;
extern const int SwE84GO                       ;

extern const int SwE84LREQ                    ;
extern const int SwE84UREQ                    ;
extern const int SwE84VA                      ;
extern const int SwE84READY                   ;
extern const int SwE84VS0                     ;
extern const int SwE84VS1                     ;
extern const int SwE84HOAVBL                  ;
extern const int SwE84ES                      ;
extern const int SwE84POWER                   ;

extern const int SwCassette01          ;   //wei 20180702 MR
extern const int SwCassette02          ;   //wei 20180702 MR
extern const int SwCassette03          ;   //wei 20180702 MR
extern const int SwCassette04          ;   //wei 20180702 MR
extern const int SwCassette05          ;   //wei 20180702 MR
extern const int SwCassette06          ;   //wei 20180702 MR
extern const int SwCassette07          ;   //wei 20180702 MR
extern const int SwCassette08          ;   //wei 20180702 MR
extern const int SwCassette09          ;   //wei 20180702 MR
extern const int SwCassette10          ;   //wei 20180702 MR
extern const int SwMRStart             ;   //wei 20180702 MR
extern const int SwMRPause             ;   //wei 20180702 MR
extern const int SwMRUp                ;   //wei 20180702 MR
extern const int SwMRDown              ;   //wei 20180702 MR

extern const int SwMRError             ;   //wei 20180702 MR
extern const int SwMRUnLoadReady       ;   //wei 20180702 MR
extern const int SwMRPresence          ;   //wei 20180702 MR
extern const int SwMRPlacement         ;   //wei 20180702 MR
extern const int SwMRLoadReady         ;   //wei 20180702 MR
extern const int SwMRManualMode        ;   //wei 20180702 MR
extern const int SwMRLight             ;   //wei 20180702 MR
extern const int SwBufferArmZ          ;   //wei 20200302 MR

//KaiChen 20200716 ：OHT
//==>
extern const int SwTrayBracket2Z       ;
extern const int SwCaselevator2Z       ;
extern const int SwMultileEmptyZ       ;

extern const int SwOHT_UnloadReady_1       ;
extern const int SwOHT_CarrierPresence_1   ;
extern const int SwOHT_CarrierPlacement_1  ;
extern const int SwOHT_LoadReady_1         ;
extern const int SwOHT_ManualMode_1        ;
extern const int SwOHT_AutoMode_1          ;
extern const int SwOHT_Error_1             ;

extern const int SwOHT_UnloadReady_2       ;
extern const int SwOHT_CarrierPresence_2   ;
extern const int SwOHT_CarrierPlacement_2  ;
extern const int SwOHT_LoadReady_2         ;
extern const int SwOHT_ManualMode_2        ;
extern const int SwOHT_AutoMode_2          ;
extern const int SwOHT_Error_2             ;

extern const int SwMultileEmpty_ScanTray_Open       ;
extern const int SwMultileEmpty_ScanTray_Close      ;
extern const int SwMultileEmpty_ScanTrayID          ;

extern const int SwE84_2_LREQ                    ;
extern const int SwE84_2_UREQ                    ;
extern const int SwE84_2_VA                      ;
extern const int SwE84_2_READY                   ;
extern const int SwE84_2_VS0                     ;
extern const int SwE84_2_VS1                     ;
extern const int SwE84_2_HOAVBL                  ;
extern const int SwE84_2_ES                      ;
extern const int SwE84_2_POWER                   ;

extern const int SwE84_1_LREQ                    ;
extern const int SwE84_1_UREQ                    ;
extern const int SwE84_1_VA                      ;
extern const int SwE84_1_READY                   ;
extern const int SwE84_1_VS0                     ;
extern const int SwE84_1_VS1                     ;
extern const int SwE84_1_HOAVBL                  ;
extern const int SwE84_1_ES                      ;
extern const int SwE84_1_POWER                   ;

extern const int SwSafeDoorLock_LoadPort1        ;
extern const int SwSafeDoorLock_LoadPort2        ;
//<==
//KaiChen 20200716 ：OHT

extern const int SwSocketClean2        ;   //JerryYang 20190715 Clean air arm1 arm2分開控制
extern const int SwTesterDryAirSwitch  ;   //Ifor 20200115 : add Tester Dry Air Control
//Sam 20190112 LM
//==>
extern const int SwLoadDoorLock       ;
extern const int SwLoadRobotZ         ;
extern const int SwUnloadDoorLock     ;
extern const int SwUnloadRobotZ       ;
extern const int SwSafeDoorLockLM     ;
extern const int SwIndEpArm1          ;
extern const int SwIndEpArm2          ;
//<==
//Ifor 20210622 add: ATC Switch TJ
//==>
extern const int SwTjSignal01;
extern const int SwTjSignal02;
extern const int SwTjSignal03;
extern const int SwTjSignal04;
extern const int SwTjSignal05;
extern const int SwTjSignal06;
extern const int SwTjSignal07;
extern const int SwTjSignal08;
//<==
//Ifor 20210622 add: ATC Switch TJ
extern const int SwATCHeatGun;  //JerryYang 20220408 : add for ATC3.5
extern const int SwLBAir;       //JerryYang 20220923 : LB吹氣function
extern const int SwDryAirSwitch ;//Ztex 2023.04.13 Add HT-1032 IO
extern const int SwColdAirSwitch;//Ztex 2023.04.13 Add HT-1032 IO
extern const int SwTriTempSafeDoor6Lock;//Ztex 2023.04.13 Add HT-1032 IO
extern const int SwEnhaustAirFanPowerOn;//Ztex 2023.04.26 Add HT-1032 IO Exhaust Air
//====================================================================
extern const int BackSwStart0                  ;
extern const int BackSwStart1                  ;
extern const int BackSwStart2                  ;
extern const int BackSwStart3                  ;
//Alick 20161011 (Steven) : TTL支援8Site
extern const int BackSwStart4                  ;
extern const int BackSwStart5                  ;
extern const int BackSwStart6                  ;
extern const int BackSwStart7                  ;
extern const int BackSwDut0                    ;
extern const int BackSwDut1                    ;
extern const int BackSwDut2                    ;
extern const int BackSwDut3                    ;
extern const int BackSwDut4                    ;
extern const int BackSwDut5                    ;
extern const int BackSwDut6                    ;
extern const int BackSwDut7                    ;
extern const int SwLoaderVibration             ;
extern const int SwAutoCoolDown                ;   //kevin 20201223 AutoCool down
extern const int SwMagazineMotorBreaker        ;  //JerryYang 20220909 : add magazine
extern const int SwMagazineSafeDoorLock        ;
extern const int SwMagazineSafeDoor2LockOn     ;
extern const int SwMagazineSafeDoor2LockOff    ;
extern const int SwESDAntennaRelay1            ;
extern const int SwESDAntennaRelay2            ;
extern const int SwESDAntennaRelay3            ;
extern const int SwESDAntennaRelay4            ;
extern const int SwESDAntennaRelay5            ;
extern const int SwESDAntennaRelay6            ;
extern const int SwESDAntennaRelay7            ;
extern const int SwESDAntennaRelay8            ;

extern const int SwLoaderAirClean              ;

extern const int SwACAuto4    ;                                                 //Steven 20230907 : For HT-9011UC
extern const int SwACAuto4CW  ;
extern const int SwACAuto5    ;
extern const int SwACAuto5CW  ;
extern const int SwACAuto6    ;
extern const int SwACAuto6CW  ;

extern const int SwSafeDoor1HatchwayLock       ;                                //ChungHung 20230718 add for Safe plc start
extern const int SwSafeDoor2HatchwayLock       ;
extern const int SwSafeDoor3HatchwayLock       ;
extern const int SwSafeDoor4HatchwayLock       ;
extern const int SwSafeDoor5HatchwayLock       ;
extern const int SwSafeDoor6HatchwayLock       ;
extern const int SwSafeDoor7HatchwayLock       ;
extern const int SwSafeDoor8HatchwayLock       ;

extern const int SwElectricControlBoxLock1     ;
extern const int SwElectricControlBoxLock2     ;
extern const int SwElectricControlBoxLock3     ;
extern const int SwElectricControlBoxLock4     ;
extern const int SwElectricControlBoxLock5     ;
extern const int SwElectricControlBoxLock6     ;
extern const int SwElectricControlBoxLock7     ;                                //ChungHung 20230718 add for Safe plc end

extern const int SwInArmZBreaker               ;                                //add One sucker with rotate
extern const int SwOutArmZBreaker              ;                                //add One sucker with rotate

extern const int SwLoadCarRFIDZBreaker         ;                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int SwBottomBlower;
extern const int SwFixedSeatTLOn;                                               //Jimmychiu 20240322 : Top & Bottom Inspect
extern const int SwFixedSeatTLOff;
extern const int SwFixedSeatTROn;
extern const int SwFixedSeatTROff;
extern const int SwFixedSeatBLOn;
extern const int SwFixedSeatBLOff;
extern const int SwFixedSeatBROn;
extern const int SwFixedSeatBROff;
extern const int SwCCDZBreaker;
extern const int SwTopBtmRotateLockOn;
extern const int SwTopBtmRotateLockOff;
extern const int SwLightStart;
extern const int SwPRGSEL0;
extern const int SwPRGSEL1;
extern const int SwPRGSEL2;
extern const int SwPRGSEL3;
extern const int SwPRGSEL4;
extern const int SwPRGSEL5;
extern const int SwPRGSEL6;
extern const int SwLightOrg;
extern const int SwDryAirUseHandler            ;
extern const int SwDryAirUseATCCar             ;
extern const int SwMultiEp;                                                     //Ifor 20250618 add:Auto Switch Multi EP
extern const int SwCassetteLDMotBreaker;                                        //Ifor 20251216 add:Boat Carrier
extern const int SwCassetteAuto1MotBreaker;                                     //Ifor 20251216 add:Boat Carrier
extern const int SwCassetteAuto2MotBreaker;                                     //Ifor 20251216 add:Boat Carrier

extern int SwAutoCCW[MAX_AUTO_TRAY];
extern int SwAutoCW [MAX_AUTO_TRAY];

//====================================================================
extern const int MInArmX       ;
extern const int MInArmY       ;
extern const int MInArmPitch   ;
extern const int MInArmZA      ;  //0=i+j*2 (0, 0)
extern const int MInArmZB      ;  //1=i+j*2 (1, 0)
extern const int MInArmZC      ;  //2=i+j*2 (0, 1)
extern const int MInArmZD      ;  //3=i+j*2 (1, 1)
extern const int MInArmZE      ;  //4=i+j*2 (0, 2)
extern const int MInArmZF      ;  //5=i+j*2 (1, 2)
extern const int MInArmZG      ;  //6=i+j*2 (0, 3)
extern const int MInArmZH      ;  //7=i+j*2 (1, 3)
extern const int MInShuttle1   ;
extern const int MInShuttle2   ;
extern const int MTestY1       ;
extern const int MTestZ1       ;
extern const int MTestZ2       ;
extern const int MTestY2       ;
extern const int MOutShuttle1  ;
extern const int _CCDX         ;
extern const int MOutShuttle2  ;
extern const int MOutArmX      ;
extern const int MOutArmY      ;
extern const int MOutArmPitch  ;
extern const int MOutArmZA     ;
extern const int MOutArmZB     ;
extern const int MOutArmZC     ;
extern const int MOutArmZD     ;
extern const int MOutArmZE     ;
extern const int MOutArmZF     ;
extern const int MOutArmZG     ;
extern const int MOutArmZH     ;
extern const int MTrayX        ;
extern const int MInArmPitchY  ;    //Steven 20131002 : XY變距
extern const int MInArmPitchX2 ;    //Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch
extern const int MOutArmPitchY ;    //Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch
extern const int MOutArmPitchX2;    //Steven 20131002 : XY變距

extern const int MLoaderZ      ;
extern const int MEmptyZ       ;
extern const int MColorZ       ;
extern const int MAuto1Z       ;
extern const int MAuto2Z       ;
extern const int MAuto3Z       ;
extern const int MInRotateKit  ;        //2013-04-12    Dell :旋轉站;馬達版
extern const int MOutRotateKit ;        //2013-04-12    Dell :旋轉站;馬達版
extern const int MAOIKit       ;        //2014-03-04    Dell    for SPIL WLP Add 5S Inspection

extern const int MLoaderY      ;        //Steven 20150910 : Add for OCR
extern const int MEmptyY       ;
extern const int MColorY       ;
extern const int MAuto1Y       ;
extern const int MAuto2Y       ;
extern const int MAuto3Y       ;
//extern const int MTapeReelFront;        //Steven 20150910 : Add for Tape Reel
//extern const int MReelRotateR  ;
//extern const int MTapeReelZ    ;
//extern const int MTapeShuttle1 ;
//extern const int MTapeShuttle2 ;
//extern const int MTapeInX      ;
//extern const int MTapeInZ      ;
extern const int MInArmZAe       ;        //Steven 20230323 : For HT1032
extern const int MInArmPitchX3 ;
extern const int MInArmPitchX4 ;
extern const int MInArmZAf     ;
extern const int MOutArmPitchX3;
extern const int MOutArmPitchX4;
extern const int MTrayZ        ;

extern const int MOutSortX     ;                                                //Steven 20240822 : For HT-9046AU
extern const int MOutSortY     ;
extern const int MOutSortPitchX;
extern const int MOutSortAa    ;
extern const int MOutSortAb    ;
extern const int MOutSortSht   ;
extern const int MLdCarRotArm  ;                                               //RogerYang 20250828 add for Loader Rotate Arm

extern const int MInArmXScale  ;        //Steven 20160426 : 磁性尺
extern const int MInArmYScale  ;
extern const int MOutArmXScale ;
extern const int MOutArmYScale ;

extern const int MShuttle1Pitch ;       //wei 20160914 Auto Shuttle Sensor
extern const int MShuttle2Pitch ;       //wei 20160914 Auto Shuttle Sensor

//Steven 20170329 (Wei) : Add individual rotate motor
//==>
extern const int MInRotateB    ;
extern const int MInRotateC    ;
extern const int MInRotateD    ;
extern const int MInRotateE    ;
extern const int MInRotateF    ;
extern const int MInRotateG    ;
extern const int MInRotateH    ;
extern const int MOutRotateB   ;
extern const int MOutRotateC   ;
extern const int MOutRotateD   ;
extern const int MOutRotateE   ;
extern const int MOutRotateF   ;
extern const int MOutRotateG   ;
extern const int MOutRotateH   ;
extern const int MLightScale   ;

extern int MInRotate[MAX_ARM_Row][MAX_ARM_Col];
extern int MOutRotate[MAX_ARM_Row][MAX_ARM_Col];
//<==
//Steven 20170329 (Wei) : Add individual rotate motor

extern const int MPreciser     ;    //Steven 20180212 : 定位器  ;
//extern const int MTrayRobotX   ;    //Steven 20170330 (Wei) : For HT-9046LM
//extern const int MTrayRobotY   ;    //Steven 20170330 (Wei) : For HT-9046LM
//extern const int MTrayRobotZ   ;    //Steven 20170330 (Wei) : For HT-9046LM
//extern const int MNoUse82      ;
extern const int MArmAlignment ;    //Steven 20240507 : 只是為了Teaching存檔方便
extern const int MLoadHingeR   ;    //Steven 20170330 (Wei) : For TSMC
extern const int MLoadHingeZ   ;    //Steven 20170330 (Wei) : For TSMC

extern const int MInArmZAg    ;         //Steven 20230323 : For HT1032
extern const int MInArmZAh    ;
extern const int MInArmZBe    ;
extern const int MInArmZBf    ;
extern const int MInArmZBg    ;
extern const int MInArmZBh    ;

extern const int MOutArmZAe  ;
extern const int MOutArmZAf  ;
extern const int MOutArmZAg  ;
extern const int MOutArmZAh  ;
extern const int MOutArmZBe  ;
extern const int MOutArmZBf  ;
extern const int MOutArmZBg  ;
extern const int MOutArmZBh  ;

extern const int MCasArmX      ;    //wei 20180702 MR
extern const int MCasArmZ      ;    //wei 20180702 MR
extern const int MTrayBracketZ ;    //wei 20180702 MR
extern const int MStackedTrayX ;    //wei 20180702 MR
extern const int MStackedTrayZ ;    //wei 20180702 MR
//extern const int MLoadRobotZ   ;    //Sam 20190112 LM
extern const int MUnloadRobotZ ;    //Sam 20190112 LM
extern const int MCaselevatorZ ;    //wei 20180702 MR
extern const int MMagazine     ;    //JerryYang 20220909 : add magazine
extern const int MCatchMgzTray ;    //JerryYang 20220909 : add magazine
extern const int MMagYTrayOut  ;    //JerryYang 20220909 : add magazine

extern const int MMTrayZ       ;
extern const int MMEmptyZ      ;
extern const int MMColorZ      ;
extern const int MMAuto1Z      ;
extern const int MMAuto2Z      ;
extern const int MMAuto3Z      ;        //ChungHung 20140317 add Auto Retest

extern const int MManualTray1  ;
extern const int MManualTray2  ;
extern const int MManualTray3  ;
extern const int MMTrayY       ;
extern const int MMTrayY_Car   ;
extern const int MMPlate1      ;
extern const int MMPlate2      ;
extern const int MMAuto1       ;
extern const int MMAuto2       ;
extern const int MMAuto3       ;
extern const int MMAuto1_Car   ;
extern const int MMAuto2_Car   ;
extern const int MMAuto3_Car   ;
extern const int MMEmpty       ;
extern const int MMColor       ;
extern const int MMEmpty_Car   ;
extern const int MMColor_Car   ;
extern const int MMEmpty1      ;
extern const int MMEmpty1_Car  ;

extern const int MMHot1RecBuf  ;   //jou 2011-12-26 加入記憶尚未完成吸取的位置
extern const int MMHot2RecBuf  ;   //jou 2011-12-26 加入記憶尚未完成吸取的位置

extern const int MMAutoCleanKit;   //jou 2012-05-21 Auto Clean
extern const int MMOCR         ;   //Steven 20120626 : OCR

//extern const int MMInRotateKit ;   //jou 2013-03-01 Rotate kit
//extern const int MMOutRotateKit;   //jou 2013-03-01 Rotate kit
extern const int MMBulkboxKit;     //kevin 20160822

extern const int MMCABuffer1   ;   //wei 20180702 MR
extern const int MMCABuffer2   ;   //wei 20180702 MR
extern const int MMCABuffer3   ;   //wei 20180702 MR
extern const int MMCABuffer4   ;   //wei 20180702 MR
extern const int MMCABuffer5   ;   //wei 20180702 MR
extern const int MMCABuffer6   ;   //wei 20180702 MR
extern const int MMCABuffer7   ;   //wei 20180702 MR
extern const int MMCABuffer8   ;   //wei 20180702 MR
extern const int MMCABuffer9   ;   //wei 20180702 MR
extern const int MMCABuffer10  ;   //wei 20180702 MR
extern const int MMLoadPort    ;   //wei 20180702 MR

extern const int MMTrayLoader  ;   //wei 20180702 MR
extern const int MMTrayEmpty   ;   //wei 20180702 MR

extern const int MMTrayConversion; //wei 20180702 MR
extern const int MMTrayAuto1   ;   //wei 20180702 MR
extern const int MMTrayAuto2   ;   //wei 20180702 MR
extern const int MMTrayAuto3   ;   //wei 20180702 MR

extern const int MMDailyCorrelationKit; //KaiHuang 20200606 : For ASE-CL Daily Correlation

//Sam 20190112 LM
//==>
extern const int MCCDX         ;
extern const int MCCDY         ;
extern const int MCCDZ         ;
/*extern const int M1_1X         ;
extern const int M1_1Y         ;
extern const int M1_1R         ;
extern const int M1_2X         ;
extern const int M1_2Y         ;
extern const int M1_2R         ;
extern const int M1_3X         ;
extern const int M1_3Y         ;  */
extern const int MLdCarRotArm  ;                                                //RogerYang 20250828 add for Loader Rotate Arm
extern const int MLoaderY_CCW  ;
extern const int MAuto1Y_CCW   ;
extern const int MAuto2Y_CCW   ;
extern const int MAuto3Y_CCW   ;
extern const int MAuto4Y_CCW   ;
extern const int MAuto5Y_CCW   ;
extern const int MAuto6Y_CCW   ;
extern const int M1_3R         ;
extern const int M1_4X         ;
extern const int M1_4Y         ;
extern const int M1_4R         ;

extern const int MInFlipper1   ;        //Frank 20210612 : Flipper Function
extern const int MInFlipper2   ;
extern const int MInFlipper3   ;
extern const int MOutFlipper1  ;
extern const int MOutFlipper2  ;
extern const int MOutFlipper3  ;

extern const int M1_5X         ;
extern const int M1_5Y         ;
extern const int M1_5R         ;
extern const int M1_6X         ;
extern const int M1_6Y         ;
extern const int M1_6R         ;
extern const int M1_7X         ;
extern const int M1_7Y         ;
extern const int M1_7R         ;
extern const int M1_8X         ;
extern const int M1_8Y         ;
extern const int M1_8R         ;

extern const int MFix3Full     ;    //JimmyChiu 20220927 : Stepper Motor Control in Fix3

extern const int MTopAOIArmX   ;
extern const int MTopAOIArmY   ;
extern const int MTopAOIArmR   ;
extern const int MTopAOICCDZ   ;
extern const int MTopAOIElevZ1 ;
extern const int MTopAOIElevZ2 ;

extern const int MMLoadPort1   ;
extern const int MMLoadPort2   ;
extern const int MMLoadPort3   ;
extern const int MMLoadPort4   ;
extern const int MMUnloadPort1 ;
extern const int MMUnloadPort2 ;
extern const int MMUnloadPort3 ;
extern const int MMUnloadPort4 ;
//<==
//Sam 20190112 LM

extern const int MAuto4Z;                                                       //Steven 20230907 : For HT-9011UC
extern const int MAuto5Z;
extern const int MAuto6Z;
extern const int MAuto4Y;
extern const int MAuto5Y;
extern const int MAuto6Y;

extern const int MLoad2Z       ;                                                //Steven 20240822 : For HT-9046AU
extern const int MLoad2Y       ;

extern const int MInSh1LtcSenZ1;                                                //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
extern const int MInSh1LtcSenZ2;
extern const int MInSh2LtcSenZ1;
extern const int MInSh2LtcSenZ2;

extern const int MMFixTray1    ;    //Steven 20100205 : 暫存Fix資料用
extern const int MMFixTray2    ;    //Steven 20100205 : 暫存Fix資料用
extern const int MMFixTray3    ;    //Steven 20100205 : 暫存Fix資料用

//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
extern const int MMInArmAOATray ;
extern const int MMOutArmAOATray;
extern const int MMAOASampleTray;
extern const int MMAOASamplePlate;
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT

extern const int MMSystem      ;
extern const int MMInShuttle   ;
extern const int MMOutShuttle  ;
extern const int MMIndex       ;
extern const int MMTemperature ;
extern const int MMInterface   ;
extern const int MMScanner     ;
extern const int MMCCD         ;
extern const int MMAutoClean   ;

extern const int MMSafeDoor1   ;
extern const int MMSafeDoor2   ;
extern const int MMSafeDoor3   ;
extern const int MMSafeDoor4   ;
extern const int MMSafeDoor5   ;
extern const int MMSafeDoor6   ;
extern const int MMSafeDoor7   ;
extern const int MMSafeDoor8   ;
extern const int MMSafeDoor9   ;
extern const int MMSafeDoor10  ;
extern const int MManualTrayAll;

//Steven 20130205 : 離子風扇異常顯示
extern const int MMIonFan01    ;
extern const int MMIonFan02    ;
extern const int MMIonFan03    ;
extern const int MMIonFan04    ;
extern const int MMIonFan05    ;
extern const int MMIonFan06    ;
extern const int MMIonFan07    ;
extern const int MMIonFan08    ;
extern const int MMIonFan09    ;
extern const int MMIonFan10    ;
extern const int MMIonFan11    ;
extern const int MMIonFan12    ;
//Ifor 20160830 add ATC 異常位置顯示
extern const int MMATC_Handler      ;
extern const int MMATC_TCPIP        ;
extern const int MMATC_NI           ;
extern const int MMATC_ATC          ;
extern const int MMATC_Chiller      ;
extern const int MMATC_RS232        ;
extern const int MMATC_Head         ;
extern const int MMATC_PowerSupply  ;
extern const int MMATC_WaterValve   ;
extern const int MMSafeDoor11       ;
extern const int MMSafeDoor12       ;
extern const int MMSafeDoor13       ;
extern const int MMSafeDoor14       ;
extern const int MMSafeDoor15       ;
extern const int MMSafeDoor16       ;
extern const int MMSafeDoor17       ;   //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
extern const int MMSafeDoor18       ;
extern const int MMSafeDoor19       ;
extern const int MMSafeDoor20       ;
extern const int MMSafeDoor21       ;
extern const int MMSafeDoor22       ;

//KaiChen 20200716 ：OHT
//==>
extern const int MMMultileEmpty        ;
extern const int MMMultileEmpty_Catch  ;
extern const int MMMultileEmpty_Z      ;
//<==
//KaiChen 20200716 ：OHT

//JerryYang 20220909 : add magazine
//==>
extern const int MMMagazineTary1    ;
extern const int MMMagazineTary2    ;
extern const int MMMagazineTary3    ;
extern const int MMMagazineTary4    ;
extern const int MMMagazineTary5    ;
extern const int MMMagazineTary6    ;
extern const int MMMagazineTary7    ;
extern const int MMMagazineTary8    ;
extern const int MMMagazineTary9    ;
extern const int MMMagazineTary10   ;
extern const int MMMagazineTary11   ;
extern const int MMMagazineTary12   ;
extern const int MMMagazineTary13   ;
extern const int MMMagazineTary14   ;
extern const int MMMagazineTaryTop  ;

extern const int MMMagazineBuffer   ;

extern const int MMBackupMagazineTary1;
extern const int MMBackupMagazineTary2;
extern const int MMBackupMagazineTary3;
extern const int MMBackupMagazineTary4;
extern const int MMBackupMagazineTary5;
extern const int MMBackupMagazineTary6;
extern const int MMBackupMagazineTary7;
extern const int MMBackupMagazineTary8;
extern const int MMBackupMagazineTary9;
extern const int MMBackupMagazineTary10;
extern const int MMBackupMagazineTary11;
extern const int MMBackupMagazineTary12;
extern const int MMBackupMagazineTary13;
extern const int MMBackupMagazineTary14;
//<==
//JerryYang 20220909 : add magazine

extern const int MMAuto4Z      ;                                                //Steven 20230907 : For HT-9011UC
extern const int MMAuto5Z      ;
extern const int MMAuto6Z      ;
extern const int MManualTray4  ;
extern const int MManualTray5  ;
extern const int MManualTray6  ;
extern const int MMAuto4       ;
extern const int MMAuto5       ;
extern const int MMAuto6       ;
extern const int MMAuto4_Car   ;
extern const int MMAuto5_Car   ;
extern const int MMAuto6_Car   ;
extern const int MMFixTray4    ;
extern const int MMFixTray5    ;
extern const int MMFixTray6    ;
extern const int MTopAOIArmX;                                                   //Ian 20230823 Top AOI Function
extern const int MTopAOIArmY;
extern const int MTopAOIArmR;
extern const int MTopAOICCDZ;

extern int iTrayZMotor[MAX_TRACK];
extern int iInSwitch  [MAX_TRACK];
extern int iOutSwitch [MAX_TRACK];

extern int iSwAutoIn  [MAX_AUTO_TRAY];
extern int iSwAutoOut [MAX_AUTO_TRAY];
extern int iAutoZMot  [MAX_AUTO_TRAY];
extern int iMMAuto    [eTrayCount];
extern int iAutoIndex [eTrayCount];
extern int iMMAuto_Car[MAX_AUTO_TRAY];

extern int iMFixTray  [MAX_FIX_TRAY];
extern int iTempFix   [MAX_FIX_TRAY];
extern int iMMgzTray  [MAX_MGZ_TRAY];
extern int MgzTrayBabk[MAX_MGZ_TRAY];

extern bool  fHeaterOK;
extern bool  fHeaterOK_Backup;                      //jou 2010-10-11
extern bool  fHeaterStableOK;                       //JerryYang 20210122 : ASE-CL新增待溫功能
extern bool bHeatOKBellowError;                     //kevin 20130226 加熱ok後 溫度過低偵測    //jou 2014-06-12 修正偶發性秀低溫異常
extern int   iHeaterWait;
extern int   iHeaterCooling;
extern double UN150Read[tcTotalCount];
extern double UN150ReadSecondpoint[tcTotalCount];   //Ifor 20160629 add 第二點溫度資料
//extern double KT4HOutputRate[tcTotalCount];
//extern double KT4HReadHistory[tcTotalCount];        //Steven 20110412
extern double UN150ReadReal[tcTotalCount];
extern double UN150ReadHotplate[2];
extern double UN150ReadShuttle[2];                  //kevin 20150508 shuttle 分段加熱
extern bool  UN150CommError[tcTotalCount];
extern bool  FlushFlag;
extern int   RunState;
extern bool  PauseUT150Polling;
//extern char  UT150SendBuffer[1024];

extern bool bLampPowerOff  ;
extern bool bLampPowerOn   ;
extern bool bLampReset     ;
extern bool bLampPause     ;
extern bool bLampHome      ;
extern bool bLampTrain     ;                        //ChungHung 20120905 add
extern bool bLampStart     ;
extern bool bLampOneCycle  ;
extern bool bLampRetry     ;
extern bool bLampSkip      ;
extern bool bLampCleanOut  ;
extern bool bLampTrayFeed  ;
extern bool bLampTrayEnd   ;
extern bool bLampAlarmReset;
extern bool bLampCoverOpenFR;
extern bool bLampCoverOpenFL;
extern bool bLampCoverOpenRR;
extern bool bLampCoverOpenRL;
extern bool bLampManualSetp;
extern bool bLampManualStart;
extern bool bLampFix;                               //kevin 20130322 ONECYCLE 合併
extern bool bMoveInArm1ZToPlate1Safe;
extern bool bMoveInArm1ZToPlate2Safe;
extern bool bMoveInArm2ZToPlate1Safe;
extern bool bMoveOutArm1ToFixSafe;
extern bool bFrontPadActive;
extern bool bSafeLockStatus;                        //Ken 20210702 AddPadInterface
extern int  iResetTask;
extern int  iTrayFeedTask;
extern int  MotorPowerOnDelay;
extern bool bMotorPowerState;
extern bool bAlarmBuzzer;
extern int  iPauseBackUp;                           //ChungHung 20110901 add
extern double Torque[5];
extern int  IndexStatus;
extern double IndexTime;
//extern int  TempIndexTime[7];                       //Steven 20140619 : 3 --> 5
//extern int  TempTestTime[2];
extern TQPF_Timer tIndexTimer;
//extern TQPF_Timer tIndexTime2;
extern TQPF_Timer tSendChangeTempDelay;
extern TQPF_Timer tRecordOEE;
extern bool bNeedArmZHome;
extern bool bIsInArmHome;
extern bool bIsOutArmHome;
extern bool bIsSortArmHome;                                                     //RogerYang 20250512 Add for 9046AU
//extern bool bRotateInHome;                          //kevin 20130706 rotat single home
//extern bool bRotateOutHome;                         //kevin 20130706 rotat single home
extern int  Z1Safe;
extern int  XShiftF;
extern int  Z1Up;
extern int  Z2DownSafe;
extern int  Z2Down;
extern int  Z2Down2Speed;
extern int  Z1Down2Speed;
extern int  XShiftF_nn;                             //2013-01-15    Dell    Add nn Mode
extern int  Z2Safe;
extern int  XShiftR;
extern int  Z1DownSafe;
extern int  Z2Up;
extern int  Z1Down;
extern int  XShiftR_nn;                             //2013-01-15    Dell    Add nn Mode
extern int  AxisY_Pre_MovePos;                      //2013-01-15    Dell    Add nn Mode
extern bool bInArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col];
extern bool bOutArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col];
extern bool bSortArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col];                        //RogerYang 20250512 Add for 9046AU
extern int iInArmPitch;
extern int iOutArmPitch;
extern bool bCanNotDisableOneCycle;

extern bool bInArmPitchNeedHomeCheck;   //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
extern bool bOutArmPitchNeedHomeCheck;  //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
extern bool bSortArmPitchNeedHomeCheck;                                         //RogerYang 20250512 Add for 9046AU
extern bool bPrecisorNeedHomeCheck;

extern bool bShuttle1MoveToRight;       //ChungHung 20120717 add Index Drop Error Can Retry and Start
extern bool bShuttle1MoveToLeft;        //ChungHung 20120717 add Index Drop Error Can Retry and Start
extern bool bShuttle2MoveToRight;       //ChungHung 20120717 add Index Drop Error Can Retry and Start
extern bool bShuttle2MoveToLeft;        //ChungHung 20120717 add Index Drop Error Can Retry and Start
extern bool bShuttle1HasPickErr;        //Steven 20220712 : 避免In arm 偷放料
extern bool bShuttle2HasPickErr;

extern bool bShuttle3MoveToRight;                                               //RogerYang 20250509 Add for 9046AU
extern bool bShuttle3MoveToLeft;
extern bool bShuttle3HasPickErr;

extern bool bAutoCleanShuttle1MoveToLeft;        //Richard 20230418 : add Index Pick Error Can Retry and Start
extern bool bAutoCleanShuttle2MoveToLeft;        //Richard 20230418 : add Index Pick Error Can Retry and Start
extern bool bAutoCleanShuttle1HasPickErr;        //Richard 20230418 : add Index Pick Error Can Retry and Start
extern bool bAutoCleanShuttle2HasPickErr;        //Richard 20230418 : add Index Pick Error Can Retry and Start

extern bool bCheckNullIC1;              //JerryYang 20170623 (Steven) bCheckNullIC1改成全域變數
extern bool bCheckNullIC2;

extern bool bShuttleMoveToLeftforFix3;  //ChungHung 20140313 add Fix3 can Full Tray
extern const int MMScanAOI; //Ifor 20211026 add:
//JimmyChiu 20220908 add Pickup Error Placement
//==>
extern int  USE_InPlacement;
extern const int MInPlacementX;
extern const int MInPlacementY;
//<==
//JimmyChiu 20220908 add Pickup Error Placement
//----- by dell ccd realtime-------------
extern bool bSendRealCCDSendStart;
extern bool bSendRealCCDSendVerify;
extern bool bSendRealCCDSendVerifyOK;
extern bool bRealCCDSendArm;
extern int  iRealCCDSendArmCT;

#define FULLVIEWIMAGEPATH1   "D:\\VisionNG\\gold6.bmp"
#define FULLVIEWIMAGEPATH2   "D:\\VisionNG\\gold5.bmp"
#define FULLVIEWIMAGEPATHNG1 "D:\\VisionNG\\image6.bmp"
#define FULLVIEWIMAGEPATHNG2 "D:\\VisionNG\\image5.bmp"
#define FULLVIEWIMAGEPATH3   "D:\\HT9045\\IMG\\BMP\\default.bmp"
#define FULLVIEWIMAGEPATH4   "D:\\VisionNG\\gold8.bmp"
#define FULLVIEWIMAGEPATH5   "D:\\VisionNG\\gold7.bmp"
#define FULLVIEWIMAGEPATHNG3 "D:\\VisionNG\\image8.bmp"
#define FULLVIEWIMAGEPATHNG4 "D:\\VisionNG\\image7.bmp"

#define OCRFONTMODEL "D:\\HontechOCR\\ShareImage\\ModelCrop.bmp"
#define OCRINSPNG "D:\\HontechOCR\\ShareImage\\InspCrop.bmp"

//---------------------------------------

extern bool bPauseTester;
extern bool fEmptyCanSupplyNewTray;
extern bool fColorCanSupplyNewTray;
extern bool fAuto2CanSupplyNewTray;     //kevin 20120718 Auto2
extern bool fAutoCanSupplyNewTray[3];   //Steven 20140331 : Auto Retest
extern bool fLoaderCanSupplyNewTray;    //Steven 20140331 : Auto Retest
extern int iReceiveColorTray;
extern int iReceiveEmptyTray;
extern int iReceiveAuto2Tray;           //kevin 20120718 Auto2 KYEC MFC
extern int iReceiveAutoTray[3];         //Steven 20140331 : Auto Retest
extern int iReceiveLoaderTray;          //Steven 20140331 : Auto Retest

extern int TTL_StartData[8];            //Alick 20161011 (Steven) : TTL支援8Site
extern int TTL_Dut[8];                  //Alick 20161011 (Steven) : TTL支援8Site
//extern int TTL_ClearData[8];
extern int TTL_Sensor[8][5];            //Alick 20161011 (Steven) : TTL支援8Site
extern int  iGali_VsSpeed;
extern int  iGali_SpSpeed;
extern bool _bHomeNeedOnecycle;
extern bool bCheckShuttle1Flag;
extern bool bCheckShuttle2Flag;
extern bool bCheckShuttle3Flag;                                                 //RogerYang 20250510 Add for 9046AU
extern bool bChangeToInitStartMode;             //ChungHung 20111230 Hot Wait Mode
extern bool bNeedTrySuckHotPlate;               //ChungHung 20120206 Hotplate check
extern bool bTrySuckHotPlateCleanOut;           //ChungHung 20120206 Hotplate check
extern bool bTrySuckHotPlateOneCycle;           //Steven 20120323 : 檢查完成後,作OneCycle
extern bool bOneTimeHotPlateCheckAll;           //ChungHung 20120206 Hotplate check
extern bool bTryPickFromHotPlateShowError;      //ChungHung 20120206 Hotplate check
extern bool bHotPlateCheckNeedTrayFeed;         //Steven 20120322 : 檢查完要自動TrayFeed

extern bool bScanLightPause;

extern const int LED_Running;
extern const int LED_ErrJam;
extern const int LED_Pause;
extern const int LED_Message;
extern const int LED_Heating;
extern const int LED_Homeing;
extern const int LED_OfflineRun;
extern const int LED_AutoRetest;
//extern const int LED_TrayCounter;

extern bool bIsTestSitICFallDown;
extern bool bIsTestSitICFallDownResetHT9045;
extern bool TestISTimeOut;
extern bool IsTest;
extern int  iCurrentTime;
extern const int Axis_Z1;
extern const int Axis_Z2;
extern const int Axis_Shuttle1;
extern const int Axis_Shuttle2;
extern const int SuckActiving;
extern const int DestroyActiving;
extern bool bEnterOffset;
extern bool bClearSrtart[8];           //Alick 20161011 (Steven) : TTL支援8Site
extern bool bHomeByStart;
extern int  HistroyBin[4][8][22];      //Steven 20131004 : 2 --> 4
extern bool HistroyPassFail[4][8][22]; //Steven 20131004 : 2 --> 4
extern int  iHeaterWaitTime;
extern bool bInArmSuckErr;
extern bool bMustCleanAllTray;
extern bool bLoaderPause;
extern bool bEmptyPause;
extern bool bColorPause;
extern bool bUnLoaderPause;
extern bool bFinshTest;
extern bool bInitStartDelayNotFinish;   //Ifor 20181220 : add Init Start Delay Time Not Finish
extern bool bShuttle1Pause;
extern bool bShuttle2Pause;
extern bool bIndexArm1PickupErrStop;    //jou 2012-02-29 index pick up error,index arm move to center & alarm
extern bool bIndexArm2PickupErrStop;    //jou 2012-02-29 index pick up error,index arm move to center & alarm

extern bool bInArmNeedToSafePos;        //ChungHung 20110302 start
extern bool bNeedSlapTray;              //ChungHung 20140701 add Auto Retest

extern bool bQualSiteQuickMode;
extern int  iSetShuttleToHasNullIC;
extern bool bRearHeadICFallDown;
extern const int Tempture_Ambient;
extern const int Tempture_Hot;
extern const int Tempture_AmbientHot;   //kevin 20140918 常溫加熱 恆溫
extern const int USE_FRONT_TEST_HEAD;
extern const int USE_REAR_TEST_HEAD;
extern const int CLEAR_FRONT_TEST_HEAD;
extern const int CLEAR_REAR_TEST_HEAD;
extern AnsiString sSuperVisorString;
extern int PitchCylinderState[MAX_AUTO_TRAY];
extern bool bReadFrontTestArmTorque;
extern bool bReadRearTestArmTorque ;
extern bool bInitialLaserCheck;         //Steven 20140228 : 雷射測距功能
extern bool bInitialLaserCheckPlate;    //Steven 20140228 : 雷射測距功能
extern int  iLaserShuttle;              //Steven 20140228 : 雷射測距功能
extern bool bInitialICCheck;
extern int iInitialICCheckTask;
extern int iInitialCylinderCheckTask;
extern int iStepShuttleTask[2];
extern int iAuto1ZeroPos;
extern int iClearSocketFunction;
//Steven 20090907 Start
extern bool bCheckShuttle1MustHasIC;
extern bool bCheckShuttle2MustHasIC;
extern int iCheckShuttle1MustHasICTask;
extern int iCheckShuttle2MustHasICTask;
extern int iCheckShuttle1SlotTask;
extern int iCheckShuttle2SlotTask;
extern int iCheckShuttle1ProminentTasK;                                         //Alick 20160815 add for 力成
extern int iCheckShuttle2ProminentTasK;                                         //Alick 20160815 add for 力成
//Steven 20090908 End
//kevin 20161108 Start
extern bool bCheckShuttle1MustNoHasIC;
extern bool bCheckShuttle2MustNoHasIC;
extern int iCheckShuttle1MustNoHasICTask;
extern int iCheckShuttle2MustNoHasICTask;
extern int iCheckShuttle1SlotNoHasICTask;
extern int iCheckShuttle2SlotNoHasICTask;
extern int iCheckShuttle1ProminentNoHasICTasK;
extern int iCheckShuttle2ProminentNoHasICTasK;
//kevin 20161108 end
extern bool bLongTimePause;
extern int  iLongTimePauseCount;
extern bool bIsAutoOneCycle;
extern bool bBackupOneCycle_ByAutoClean;                                        //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 未完成時又觸發 AutoClean 動作，AutoClean 做完並不會提示 OneCycle Finish
extern bool bManualOneCycle;                                                    //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 未完成時又觸發 AutoClean 動作，AutoClean 做完並不會提示 OneCycle Finish
extern bool bIsAvoidIndexShiftOneCycle;                     //Isaac 20210821 : 全智要求定時onecycle回home,start
extern bool bIndexArmNoTestting;                            //ChungHung 20140730 add ContinuousFailHaveOneCycle
extern bool bIsASMAutoOneCycle;
extern bool bLotStartEndNeedIndexCheck;                     //Isaac 20211019 : 可選擇做index check的時機
extern bool bIndexJamNeedIndexcheck;                        //Isaac 20211019 : 可選擇做index check的時機

extern int EP_Install;
extern double EP_MAXKPA;
extern double EP_MAXAFB;
extern double EP_MinAFB;
extern double EP_MINMPA;                                    //JerryYang 20171023 (wei) add PA Min

extern int EP_Install_1032;
extern double EP_MAXKPA_1032;
extern double EP_MAXAFB_1032;
extern double EP_MinAFB_1032;
extern double EP_MINMPA_1032;

extern double EPDual_MAXKPA;   //kevin 20200325 add dual force EP
extern double EPDual_MAXAFB;
extern double EPDual_MinAFB;
extern double EPDual_MINA;
extern double EPDual_MINMPA;   //kevin 20200325 add dual force EP

extern AnsiString   JamCode                             ;
extern bool         LOAD_Z_USE_MOTOR[9]                 ; //Steven 20190813 : 入Tray改用步進馬達
extern bool         LOAD_Y_USE_MOTOR[9]                 ; //Jimmychiu 20240307 : Loader Tray改用步進馬達
extern bool         LOADUNLOAD_USE_CASSETTE[9]          ;
extern int          NUMBER_PANEL_TYPE                   ;
extern int          WEIGHT_CALIBRATION                  ; //Steven 20111108
extern int          ION_FAN_TYPE                        ;
extern int          SHUTTLE_SENSOR_TYPE                 ;
extern int          NUEC_TYPE                           ; //Sam 20230707 : EtherCAT Shuttle sensor
extern bool         ENABLE_OUT_SHUTTLE_SENEOR           ; //Steven 20120531 : by pass out shuttle sensor
extern bool         ENABLE_OUT_SHUTTLEY_LATCH           ; //Steven 20170518 (jou) : Out shuttle Y sensor 使用 LATCH
extern bool         Use_AxisY_Sensor_2x3mode            ; //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
extern bool         Bias_Mode_Use_Y_Sensor              ; //Steven 20240805 : 1x2 Bias mode 用Y sensor 偵測Outshuttle殘料
extern int          TEMPCTRL_TYPE                       ;
extern int          SAFE_DOOR_AMOUNT                    ;
extern bool         USE_NEW_TEMPCTRL_FUNCTION           ;
extern bool         TEMPCTRL_NEED_UNDER_20A             ;
extern bool         TEMPCTRL_HOTPLATE_TOGTHER           ; //jou 2010-08-16 start : SCC Hotplate不分段加熱
extern bool         AMBIENT_TEMP_CHECK[tcTotalCount]    ; //Steven 20100831 : 常溫檢查的位置     //Steven 20111208 : 記憶體破壞, 20->tcTotalCount
extern double       SHUTTLE_COOLING                     ;
extern bool         OFFLINE_ALARM                       ; //Steven 20140805
extern double       VORTEX_COOLING                      ;
extern double       SOCKET_OFFSET                       ;
extern int          SYN_TEK_MOTION_MODULE               ; //Steven 20110304
extern int          TRAY_VIBRATION                      ; //Steven 20110627 : new Tray Vibration  //JerryYang 20170531 (wei) bool->int
extern int          USE_16_HEATER                       ; //Steven 20111011 : 使用16個Heater
extern int          EJ1N_Count                          ; //Steven 20200318 : 修正EJ1N讀取異常的問題
extern bool         USE_ATC_MODE                        ; //jou 2012-03-08 Enable ATC mode;
extern bool         USE_IO_CHANGE_TOQUE                 ; //jou 2012-06-21 Enable index I/O Change Toque
extern bool         USE_ReadIndex_TOQUE                 ; //kevin 20201027 add Read index Torque
extern bool         bResetArm1Value                     ; //KaiHuang 20201222 : Reset 標準值(第一次Contact And Change Speed)
extern bool         bResetArm2Value                     ; //KaiHuang 20201222 : Reset 標準值(第一次Contact And Change Speed)
extern bool         bReadArm1_Torque                    ; //KaiHuang 20201222 : 讀取 Arm1 扭力Flag
extern bool         bReadArm2_Torque                    ; //KaiHuang 20201222 : 讀取 Arm2 扭力Flag
extern bool        bResetRecordSetArm1Value             ;          //kevin 20210118 : Reset 標準值下依次 cotract(第一次Contact And Change Speed)
extern bool        bResetRecordSetArm2Value             ;          //kevin 20210118 : Reset 標準值下依次 cotract(第一次Contact And Change Speed)
extern double       dSetArm1TorqueValue                 ; //KaiHuang 20201222 : Arm1 第一次 Contact 的值
extern double       dSetArm2TorqueValue                 ; //KaiHuang 20201222 : Arm2 第一次 Contact 的值
extern int          iReadTorqueError                    ; //KaiHuang 20201222 : 判斷讀取扭力是否有異常(會導致扭力趨近穩態)

extern bool         REAL_TIME_CCD                       ; //Steven 20110705 : 安裝Real Time CCD
extern int          RTC_TemperNumber                    ; //Isaac 20201217 : RTC CCD增加第二組感溫
extern bool         CCD2_TEMPER                         ;
extern bool         LB_TEMP                             ; //Steven 20181023 : LB溫度
extern bool         LB_TEMP_UpDown                      ; //Frank 20241231 : add
extern bool         Index_ESDAir                        ;         //kevin  20200207 : ESD air Temp
//extern bool         INDEX_CCD                         ; //Ifor 20150720 : 安裝INDEX CCD //Ifor 20150728 Mark整合於RTC CCD選項
extern const int    iXPitch60                           ; //Steven 20131002 : XY變距
extern const int    iXPitchManual635                    ; //Steven 20131002 : XY變距
extern const int    iXYPitchVariable                    ; //Steven 20131002 : XY變距
extern const int    iXPitchManual360                    ; //Steven 20140819 : Y-Pitch 36mm
extern const int    iXYPitchRowA                        ; //Steven 20170424 (wei) : new XY變距
extern const int    iXYPitch16Picker                    ; //Steven for HT1032
extern const int    iXYPitchBb                          ; //Steven for HT7080
extern const int    iXYPitch16Bd_Be                     ; //Ztex 2023.12.06 Add HT-1032
extern const int    iXYPitchIn_Bb_Out_Bc                ; //Ztex 2024.02.24 Add HT-1132
extern const int    iXPitch40mm                         ; //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
extern const int    iXPitch50mm                         ; //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
extern const int    iXPitchAuto                         ; //JimmyChiu 20220708 : add Auto X pitch
extern const int    iXPitch16Pick                       ; //Steven for HT1032
extern int          USE_IN_OUT_ARM_Y_PITCH              ; //jou 2012-05-15 : 選擇 Y Pitch 機構模式
extern int          USE_OUT_ARM_Y_PITCH                 ;
extern bool         USE_IN_Y_IS_AUTO_PITCH              ; //JerryYang 20251218 : IN/OUT ARM支援不同模組
extern bool         USE_OUT_Y_IS_AUTO_PITCH             ;
extern int          IN_OUT_ARM_Y_PITCH_MIN              ;
extern int          IN_OUT_ARM_Y_PITCH_MAX              ;
extern int          IN_OUT_ARM_X_PITCH_MIN              ; //JimmyChiu 20220708 : add Auto X pitch
extern int          IN_OUT_ARM_X_PITCH_MAX              ; //JimmyChiu 20220708 : add Auto X pitch
extern int          USE_IN_OUT_ARM_X_PITCH              ; //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式

extern const int    iOutSortX40mm                       ;
extern int          USE_OUT_SORT_ARM                    ; //Steven 20240822 : For HT-9046AU
extern int          USE_LD_Rot_Arm                      ;                       //RogerYang 20250828 add for Loader Rotate Arm
extern int          USE_E84_Sensor                      ; //Frank 20251204 add E84
extern int          USE_LdUldCassetteMode               ; //Frank 20251217 add
extern int          USE_OUT_SORT_X_PITCH_MIN            ;
extern int          USE_OUT_SORT_X_PITCH_MAX            ;

extern int          BASE_X_TO_HP                        ; //Steven 20230826 : 基準軸X到加熱盤邊緣的距離
extern int          USE_ROTATE_KIT                      ; //Steven 20121001 : 旋轉Kit
extern int          iRotate_In_Index                    ; // 2012.06.11 , Joye , Rotate KIT
extern int          iRotate_Out_Tray6                   ; // 2012.06.11 , Joye , Rotate KIT
extern int          iRotate_Type                        ; //2013-04-12    Dell :旋轉站;馬達版
extern int          USE_ROTATE                          ; //Steven 20230930 : 整合氣缸版旋轉Kit的Flag
extern int          iSpecialSequence                    ;
extern int          USE_HOTPLATE_TYPE                   ; //jou 2012-05-15 : 選擇 Hotplate Type
extern int          USE_MAGNETIC_SCALE                  ; //Steven 20160426 : 磁性尺
extern int          USE_PICKER_COUNT                    ; //Steven 20161117 : for HT-9045S
extern int          USE_16PICKER_TYPE                   ; //Ztex 2023.12.06 Add HT-1032
extern int          INOUT_ARM_Y_PITCH                   ; //Ztex 2023.12.06 Add HT-1032
extern int          SHUTTLE_Z_TYPE                      ; //jou 2013-07-26 : 選擇 Shuttle Z Sensor Type
extern int          AUTO_EMPTY_COLOR                    ; //軌道 0:手動,  1:6軌自動,  2:7軌自動,  3:8軌自動  //Steven 20230907 : For HT-9011UC, bool --> int
extern int          iUnloadCnt                          ; //Steven 20230907 : 給out arm計算要放的Tray總數量
extern int          iAutoCnt                            ; //Steven 20230907 : 給out arm計算要放的是auto還是fix
extern int          iFixCnt                             ; //Steven 20230907 : 給out arm計算要放的Fix總數量
extern int          iAutoRight                          ; //Steven 20230907 : 給out arm計算機台到Auto最右側的位置
extern int          iFixMin                             ; //Steven 20230907 : 給out arm計算要放的fix盤起始位置 0=Auto 1
extern int          iFixMax                             ; //Steven 20230907 : 給out arm計算要放的fix盤結束位置 0=Auto 1
extern int          iFixPosMin                          ; //Steven 20230907 : 給out arm計算要放的fix盤起始位置 0=no use
extern int          iFixPosMax                          ; //Steven 20230907 : 給out arm計算要放的fix盤結束位置 0=no use
extern int          iFixPosHalf                         ; //Steven 20230907 : 給out arm計算機台到Fix最右側的位置(分半盤) 0=no use
extern int          iFix3Pos                            ; //Steven 20230907 : 給out arm計算要放的Fix3的位置
extern int          iFixRight                           ; //Steven 20230907 : 給out arm計算機台到Fix最右側的位置
extern int          iFixRightHalf                       ; //Steven 20230907 : 給out arm計算機台到Fix最右側的位置(分半盤)
extern int          iMMAutoRight                        ; //Steven 20230919 : Auto最右側的陣列位置
extern int          iMMBinBox                           ; //Steven 20230907 : Bin Box在陣列的位置
extern int          iMMFixRight                         ; //Steven 20230919 : Fix最右側的陣列位置
extern int          iMMAoi                              ; //Steven 20230919 : AOI的陣列位置
extern int          iBinBoxAtFix                        ; //Steven 20230907 : Bin Box在Fix Tray的位置
extern int          iBinBoxAtAuto                       ; //Steven 20230907 : Bin Box在iWhichAuto的位置
extern int          iBinBoxType                         ; //Steven 20230907 : Bin Box在Tray Form的type
extern int          iMagMin                             ; //Steven 20230907 : 給out arm計算要放的Magazine位置 0=Auto 1
extern int          iMagMax                             ; //Steven 20230907 : 給out arm計算要放的Magazine位置 0=Auto 1
extern int          iMagAtAuto                          ; //Steven 20230907 : 給out arm計算要放的Magazine對應Auto位置
extern bool         SUPPORT_2_EMPTY_EMPTY               ;
extern int          INDEX_SUCKER_TYPE                   ; //jou 2010-05-19 start : 負壓
extern eIndexPressType          INDEX_PRESS_TYPE        ; //Steven 20110310 : 240KG
extern bool         bUserDefMaxContactHeight            ; //Steven 20180412 : For new HT-9045高度較高
extern double       dUserDefMaxContactHeight            ;
extern bool         bUser_Define_IndexZ_SafePos         ; //Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
extern double       dUserDefineIndexZSafePos            ;
extern bool         bContactCTOverCHK                   ;
extern bool         bContactTimeOverStep                ;
extern bool         bNowDoInterFaceErrorStep            ;
extern bool         SAFE_DOOR_LOCK                      ; //20111130  Dell
extern bool         CLEAN_AIR                           ; //ChungHung 20120821 : Clean Air
extern int          INSTALL_OCR                         ; //Steven 20120716 : OCR
extern int          INSTALL_OCR_YMot                    ; //Frank 20250214 add
extern int          BAR_CODE_INSTALL                    ; //Steven 20120921 : Barcode_2D
extern int          BAR_CODE_USECOUNT                   ; //Ifor 20181206 :add COGNEX SYSTEM CCD Count
extern int          BOTTOM_2DID                         ; //Steven 20190308 : Bottom 2D
extern int          BOTTOM_2DID_CCD                     ; //KaiChen 20200513 : Bottom 2D 8CCD
extern int          SHT_FLOATING_CHK                    ; //Steven 20160920 : IC置偏檢查
extern int          IN_SHT_LAST_SENSOR                  ; //Steven 20181203 : In Shuttle最後一個Sensor定義
//extern int          BAR_CODE_MODE                     ; //Steven 20120921 : Barcode_2D
extern bool         bGetLotIDFormTester                 ; //ChungHung 20121123
extern int          FIX3_FULL_PLACE                     ; //Steven 20130126 : Fix3滿盤功能
extern int          FIX3_INSTALL                        ; //2013-07-16    Dell    9046LS第一台沒Fix3;第二台以上Fix3標配
extern int          CROSS_SENSOR_INSTALL                ; //2013-07-16    Dell    Shuttle cross sensor
extern int          ESD_Monitor                         ; //2013-07-16     Dell    使用ESD monitor
extern int          USE_NOVX3360                        ; //Steven 20131127 : 使用Simco ION風扇
extern int          USE_AutoCleanIonFan                 ; //Isaac 20210609 : IO觸發IonFan清針
extern int          USE_PULSE_TYPE                      ; //Ifor 20180316 : add Simco Use Pulse Type
extern int          ION_PULSE_COUNT                     ; //Ifor 20180316 : add Simco Use Pulse Type
extern int          USE_KASUGA                          ; //Ifor   20150731 : 使用Kasuga ION風扇
extern int          USE_OTD                             ; //Steven 20140312 : OTD
extern int          USE_BARCODE_AS_KEYBOARD             ; //Steven 20140108 : 只能用Bar Code Reader輸入
extern int          USE_46_SUCKER_DB                    ; //Steven 20131008 : HT9045使用46配氣
extern int          USE_46_SENSOR_DB                    ; //Steven 20131008 : HT9045使用46配電
extern int          INDEX_MOTION_CARD                   ; //Steven 20210623 : Index使用Galil
extern int          GALI_PROTECT_MODE                   ; //Steven 20260504 : Index in-motion protect (0=Original, 1=ER+OE, 2=TE)
extern int          USE_FINE_PITCH                      ; //Steven 20210624 : Fine Pitch
extern int          USE_OUT_SHT_MOT                     ; //Steven 20210624 : Out shuttle獨立馬達
extern int          USE_AIR_CONDITIONER                 ; //Steven 20131011 : 冷氣機
extern int          HOT_PLATE_POSITION                  ; //Steven 20140222 : Hot Plate Pin的位置
extern int          HOT_PLATE_LIMITATION                ; //Steven 20151117 : Hot Plate Pin的極限位置, 可以跑2x2 8吸嘴
extern int          USE_LASER_DISTANCE                  ; //Steven 20140228 : 雷射測距功能
extern int          USE_AUTO_RETEST                     ; //ChungHung 20140317 add Auto Retest
extern int          UNLOADER_ART[MAX_AUTO_TRAY]         ;   //Steven 20161221 : for SCK only Auto 2 has ART
extern int          USE_AOI_Inspection                  ;   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
extern int          USE_VibrationCommunication          ;   //JerryYang 20230814 : add震動馬達通訊調速版本
extern int          USE_COLOR_TRAY_SENSOR               ;   //20140903 wei colcr Tray
extern int          USE_SOCKET_SENSOR                   ;
extern int          CANBUS_METHOD                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
extern bool         USE_CKD_FCM_CleanAir                ;   //Ifor 20150708 : CKD FCM Clean Air
extern int          USE_CATCH_TRAY_MODEL                ;   //wei 20150807 Catch Tray Model
//Steven 20260924 (S12-C)：golden V912 有、移植樹沒有；FileRW/IniConfig.gen.inc（golden cConfiguration.cpp）要用到，逐字補上（golden cmydef.h:3023-3024）
extern int          USE_TRAY_ARM_TYPE                   ;   //Ifor 20260824 : Tray Arm hardware type (eTrayArmHWType), new type standby at middle of Empty and Color
extern int          TRAY_ARM_STANDBY_OFFSET             ;   //Ifor 20260824 : new Tray Arm standby offset from Empty toward Color (pulse), avoid InArm collision
extern int          USE_FINGER_PRINT                    ;   //Steven 20190503 : 指紋辨識權限
extern int          DETECT_CURRENT                      ;   //Richard 20220928 :電流偵測模組
extern int          USE_DEVICE_FLIPPER                  ;   //Frank 20210612 : Flipper Function
extern int          TRAY_ARM_MODE                       ;   //Frank 20230419
extern int          USE_2nd_LOADER                      ;   //Steven 20240822 : For HT-9046AU

extern int iBackUpZ1DownPosition;
extern int iBackUpZ2DownPosition;
extern int iOpenNewTTLBoardStratDelayCount;
extern bool bChangeTTLFlag;
extern int AUTO_SENSOR_INSTALL                          ; //wei 20160914 Auto Shuttle Sensor
extern int ShuttleVibration_INSTALL                     ; //JerryYang 20171006 (wei) Shuttle 震動馬達
extern int USE_TRAY_MAPPING                             ; //wei 20161219 Tray Mapping
extern int USE_TRAY_ROBOT                               ; //Steven 20170330 (Wei) : For HT-9046LM
extern int USE_LOADER_HINGE                             ; //Steven 20170330 (Wei) : For TSMC
extern int USE_Fix_AI_CCD                               ; //RogerYang 20180901 add 矽格湖口Demo AI CCD Function
extern int USE_DIE_CLEAN                                ; //wei 20170418
extern int USE_MR_SYSTEM                                ; //wei 20180702 MR
extern int USE_RFID_SYSTEM                              ; //wei 20180702 MR
extern int USE_RFID_READER                              ; //Steven 20220713 : RFID Reader for SJSEMI
extern int USE_AUTO_ALIGNMENT                           ; //Sam 20181201 : AutoAlignment

extern int USE_OHT_SYSTEM                               ;   //KaiChen 20200716 ：OHT
extern int USE_Multile_Empty                            ;   //KaiChen 20200716 ：OHT
extern int USE_KEYENCE_LOADER;
extern int USE_KEYENCE_EMPTY;
extern int USE_MultileEmptyTrayID_Keyence;
extern int USE_COLORSENSOR_MUN                          ; //Jimmychiu 20230630 : add color sensor MU-N in Loader

extern int LoaderUnload_StepMotor                       ; //Steven 20200529 : Loader入Tray改步進
//extern AnsiString TrayStepMotor_ComPort                 ;
extern bool bPlaceToShuttleFirst;
extern int  USE_PRECISER                                ; //Frank 20180410 (Steven) : InArm Preciser Station
//extern bool PreciserInstallShuttleArea                  ; //Frank 20180410 (Steven) : InArm Preciser Station
extern int iPreciserInstallArea                         ; //Ifor 20191008 : add Preciser Install Area
extern int  i24V_PULSE_COUNT                            ; //Ifor 20190311 : add 24V Monitor Use Pulse Type
extern int  CHAMBER_USE_PULSE_TYPE                      ; //Ifor 20190422 : add Chamber Use Pulse Type
extern int  USE_GROUND_MAN                              ; //Steven 20190828 : 通訊式GroundMan
extern bool USE_LOADER_VIBRATION                        ;
extern int  Ground_Man_ScanPoint                        ; //KaiChen 20191005 ：通訊式GroundMan
extern int  Ground_Man_AlarmOhm                         ; //KaiChen 20191005 ：通訊式GroundMan
extern bool USE_KASUGA_Fan                              ; //KaiChen 20191225 ：KASUGA Fan 通訊
extern int  iUseHTIonBarFunction                        ;                       //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
extern bool bUseHTIonBar_PowerReset                     ;                       //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
extern int  iUseHTIonBar_PowerResetCount                ;                       //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
extern int  iUseHTIonBar_SendPowerStatus[MAX_HTIONFAN]  ;                       //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
extern int  SOCKET_AMP_QTY                              ;
extern int  SOCKET_AMP_QTY_2nd                          ;
extern int  SOCKET_AMP_QTY_3rd                          ;                       //JerryYang 20250729
extern int  ROTATE_AMP_QTY                              ;
extern int  COLOR_AMP_QTY                               ;
extern int  NUDN1_QTY                                   ;   //Sam 20210518 : 新增 CanBus 軟體配置
extern int  NUDN1_MACID11_AMP_QTY                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
extern int  NUDN1_MACID12_AMP_QTY                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
extern int  NUDN1_MACID13_AMP_QTY                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
extern int  NUDN1_MACID14_AMP_QTY                       ;   //Sam 20210518 : 新增 CanBus 軟體配置
extern int  AUTO3_IS_MAGAZINE                           ;   //JerryYang 20220909 : add magazine
extern int  MAGAZINE_BIN_DISP_TYPE                      ;   //JerryYang 20220909 : add magazine
extern int  VibrationMotorCount                         ;   //JerryYang 20230814 : add震動馬達通訊調速版本
//Motion Mode----------------------------------------------------------------------
extern int MOTION_CARD_TYPE;                    //Brian 20121015
extern int MOTIONNET_SPEED;                     //Steven 20181122 : MNet速度要可以修改
extern int IO_CARD_TYPE;                        //Brian 20121015
extern int TTL_CARD_TYPE;                       //Steven 20121122 : 選擇TTL Card 模式
extern int TTL_CARD_USE_ADDRESS;                //Isaac 20210922 : 選擇TTL板子是否帶站別
//TTL Mode----------------------------------------------------------------------
extern const int _3Bit;
extern const int _4Bit;
extern const int _5Bit;
extern const int _8Bit;
extern const int _10Bit;
extern const int _5BitPE;
extern const int _5BitPO;
extern const int _10BitPE;
extern const int _10BitPO;
extern const int LogicPositive;
extern const int LogicNegative;
extern const int CHEach;
extern const int CHSingle;
extern const int DUTNONE;
extern const int DUTPosLevel;
extern const int DUTPosPluse;
extern const int DUTNegLevel;
extern const int DUTNegPluse;
extern const int CHOneByOne;
extern const int CHBinary;
//extern HTimer bTTL_StartTimer[8];             //Steven 20161011 : TTL支援8Site 4 --> 8
extern TQPF_Timer TestIntervalsTime;            //kevin 20160311 取得測試機間隔時間
extern TQPF_Timer TestIntervalsBoostTime;       //JerryYang 20181122 (Steven) :  (Steven) : 將不同function計時器分開
extern TQPF_Timer MyTTLSOTTimer;                //Steven 20180808 (wei) : TTL的時間單位改成microsecond(μs)
extern TQPF_Timer PauseIntervalsTime;           //kevin 20181009 取得機台停止時間
extern TQPF_Timer SOTPauseIntervalsTime;        //kevin 20181102 (Steven) : 取得機台停止時間
extern int iTriggerBoostFunction;               //Steven 20180817 : Boost Function
extern int iTriggerBoostFuncBack;
extern bool bFlagBelowTurnOfValve;
extern bool bFlagBelowTurnOfValve1;
extern int iBoostFuncStep;
extern TQPF_Timer BoostCoolTime;                //總共需要多長的降溫Timer
extern TQPF_Timer BoostCoolCountDown;           //倒數用的
extern TQPF_Timer BoostCoolStepTimer;           //降溫的間隔時間(預設為一秒)
extern int iBoostCountDown;
extern double dBoostOffset;
extern double dBoostCoolSec;
extern bool bSetBoostOffset;
extern double iBoostEotToSotTime;
extern bool bBoostFirstBoost;
extern bool bTestSiteUse[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];  //ChungHung 20130910 alter for SCK can close site by Index

extern int iContactMode;
extern int InArmZIndex[2][8];
extern int OutArmZIndex[2][8];
extern int SortArmZIndex[2];                                                    //RogerYang 20250416 for HT9046AU add Sort Arm Z
extern bool bPlace8IC;                      //ChungHung 20110322 add
extern bool bTestSiteNeedSetErrBin[MAX_SOCKET_ROW][MAX_SOCKET_COL]; //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site
extern bool bOutShtLoseICNeedSetErrBin;                           //JerryYang 20170609 (wei) 此旗標為true需將out shuttle lose IC對應的site設成error bin
extern bool bIndexDropICNeedSetErrBin;  //JerryYang 20170609 (wei) 此旗標為true需將out shuttle lose IC對應的site設成error bin

extern int iTestRunMode;                    //0=RT
extern int iRunStartMode;
extern int iBinModelPrime;                  //ChungHung 20120912 add Bin Setting like Epson

extern int InArmClose_Pitch;
extern int OutArmClose_Pitch;

extern int InArmClose_PitchX;
extern int InArmClose_PitchY;               //ChungHung 20131231 alter AutoYPitch

extern int OutArmClose_PitchX;
extern int OutArmClose_PitchY;              //ChungHung 20131231 alter AutoYPitch

extern int SortArmClose_PitchX;                                                 //RogerYang 20250515 Add for 9046AU

extern const int PickMode;
extern const int PlaceMode;
extern int iForPlaceHPX6Step;
extern int iForPickHPX6Step;
extern int iForPlaceHPX3Step;               //20111015  Dell
extern int iForPickHPX3Step;                //20111015  Dell

extern int iForPlaceHPX10Step;              //ChungHung 20150127 add for ATK 10X16 Hotplate
extern int iForPickHPX10Step;               //ChungHung 20150127 add for ATK 10X16 Hotplate

//extern const int XPHSuckToSht_2x4_16[3][8];
//extern const int XPHShtToSuck_2x4_16[2][3][4];
//extern const int XPHNotStander_2x8_16[4][8];

extern int iPlaceHPOrder;
extern int iPickHPOrder;
extern int iRecPickHPOrder;                 //Ifor 20180521 :add 改全域
extern int iPickHotPlateFinish;             //Ifor 20180521 :add 改全域

extern bool OnlyRow2;                       //ChungHung 20121001 : 移至cmydef Reset後此參數未清為false 一樣HangUp

extern int  iInArmOrder;                    //iWhichShuttle,iInArmiWhichKit,
extern int  iWhichSht32, iWhichKit32;       //Ifor 20180116 (Steven) : add 改成全域變數
extern int  iOutArmiWhichKit;
extern int  iOutArmiWhichKit_Sort;                                              //RogerYang 20250710 Add for 9046AU
extern int  iSortArmiWhichKit;                                                  //RogerYang 20250526 Add for 9046AU
extern int  iYHalf;
extern int  iXHalf;
extern bool bResetMode;
extern bool bCheckIndex;
extern int  iWhoTriggerPiggyBack;   //Steven 20111207 : 誰觸發了Piggy Back
extern bool bQAModeQuickCleanOut;   //Steven 20111005 : 數量到的時候的快速CleanOut
extern bool bQAModeFinishCleanOut;  //Steven 20111005 : 做完QA模式的CleanOut
extern int  iQAModeLoaderCT;        //jou 2012-05-03 增加QA mode使用獨立的Loader Count
extern int  iQAModePassCT;          //JerryYang 20221004 : Maxim版本QA mode
extern bool bQAModeStart;           //Ifor 20201102 add: 新增QA Mode Start 旗標 避免Count 被清除
extern bool bCalculatePauseTime;
extern int  iUPH_LoaderCount;
extern bool bRecordUPH;
extern TDateTime tUPH_PauseTime;
extern TDateTime tUPH_PauseStartTime;
extern TDateTime tUPH_StartTime;
extern TDateTime tUPH_EndTime;
extern AnsiString sAlarmTime;
extern AnsiString sFileNameTime;                        //Steven 20190801 : 修正檔名錯誤
extern unsigned char SENDBUFF[1024];                    //Steven 20110927 : 會溢位,改成1024
extern unsigned char READBUFF[1024];
//extern char SENDBUFF_DTK[1024],READBUFF_DTK[1024];      //KaiHuang 20190821 : 新增台達 DTK4848溫控器
extern int  iCloseSiteState;                            //Eliot 2009_03_06
extern int  iCloseSiteStateForPick;                     //Steven 20150924 : 隨時開關Site功能
extern int  iSH1LED;
extern int  iSH2LED;
extern int  iSH1States;
extern int  iSH2States;
extern bool bCheckShuttleStates;
extern int  CUSTOMER_CODE;
extern int  InOutArmPickerUseMotor;
extern bool bAfterASMTest;
extern bool bASMCleanOutFinish;
//----------------------------------------------------------------------------
//Temp IC
extern bool bTempComm6ReceiveOK;
//----------------------------------------------------------------------------
extern bool bRunOcrInsp;                                //ChungHung 20120830 add OCR Function add
//----------------------------------------------------------------------------
//Auto Decay Test
extern bool bRunDecayTest;                              //Ifor 20151015 新增執行Decay Test 畫面顯示
//Auto Cleaning
extern bool bRunAutoClean;
extern int iAutoClean_IndexContactCount;
extern long lAutoClean_TimeCount;                //jou 20250102 : auto clean triger time count
extern int iDoAutoCleanTask;
extern int iDoShuttle1AutoCleanTask;
extern int iDoShuttle2AutoCleanTask;
extern int iDoShuttleAutoCleanTask;
extern int iDoIndexAutoCleanTask;
extern int iAutoCleanPickFromShuttleTask;
extern int iAutoCleanPlaceToFix3Task;
extern int iAutoCleanPlaceToHotplateTask;       //kevin 20120217
extern int iAutoCleanPlaceToCleanKitTask;       //jou 2012-05-24
extern int iCheckFinish_ByAutoClean;            //pig 2011.09.01 AutoClean
extern bool bCleanKitPitchOver12000;            //jou 2013-02-20
extern bool bCleanKitPitchLess4000;             //jou 2015-03-24 tray x pitch太小導致無法吸取
extern bool bPlaceToShuttleByAutoClean;         //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
extern bool bPickFromShuttleByAutoClean;        //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
extern bool bPickFromKitByAutoClean;            //Steven 20210603 : 新增吸放Clean Kit的Flag
extern bool bLockPlaceToShuttleByAutoClean;     //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
extern bool bLockPickFromShuttleByAutoClean;    //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
extern bool bRunATCCalibation;                  //Ifor 20160719 新增執行ATC Calibation 畫面顯示

extern const int M_INIT_START  ;
extern const int M_INIT_RESTART;
extern const int M_FINISH      ;
extern const int M_MANUAL      ;
extern const int M_SOCKET_ALARM;
extern const int M_INTERVAL    ;
//----------------------------------------------------------------------------
extern bool fRearNeedTest;
extern bool fFrontNeedTest;
extern bool fTwoArmNeedTest;    //2013-01-15    Dell    Add nn Mode
extern bool fIndexNeedTest;

extern int  iUnLoaderCount;
//----------------------------------------------------------------------------
//jou 981215 start : InArmZSafe() & OutArmZSafe()
extern const int DETECT_SENSOR_FLAG   ;
extern const int DETECT_POS_FLAG   ;
extern const int DETECT_ALL_FLAG   ;
//jou 981215 end

//----------------------------------------------------------------------------
//Steven 20091231 : 整合AlarmCode
//extern AnsiString InOurArmSuckName[MAX_ARM_Row][MAX_ARM_Col];
extern AnsiString IndexSuckName[4][8];
//extern AnsiString IndexSuckName[16];
extern AnsiString ShuttleSensorName[9];
extern AnsiString StartModeName[rsmRunModeTotal];               //Steven 20120612 : Start mode顯示修改
extern AnsiString asRecordTestResult;
extern bool bBin16HangUp;

//Eliot 2010_05_01 start
extern int INDEX_DRIVER_TYPE;
extern bool TorqueUseHPComCard;                 //Steven 20210204 : 使用鴻勁自製的通訊卡
extern int MOTOR_DRIVER_TYPE;                   //Steven 20110321 : 分辨其他軸使用的馬達廠牌
extern int iPanasonicDriverType;                // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045
extern int CONTECT_SHUTTLE_KG;                  //jou 2011-03-15
extern int CONTECT_SHUTTLE_KG_MITSUBISHI;       //Steven 20140530 : 4 --> 6
extern int CONTECT_SHUTTLE_240KG;
extern int iTorqueCommMaxTime;
//Eliot 2010_05_01 end

//Steven 20100629 Start: 紀錄修改項目
extern TMyLog myLog;
extern AnsiString asLogPath;
extern AnsiString asUser;
//Steven 20100629 End
extern bool bClearJamRateCount;                 //jou 2010-08-13 計數jam rate,改為Tray Feed為一單位
extern bool bLoaderNoTrayAutoCleanOut;          //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
extern bool bSecondTimeLoaderCheckNoTray;       //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
extern bool bNeedCheckOurArmDeviceSuck;         //jou 2010-09-07 start : 確認Out arm有確實將device吸走
extern bool bReOpenGpib;                        //Steven 20101013 : 每次OneCycle或CleanOut後，要重開GPIB
extern bool bLoaderHasSkip;                     //Steven 20101019 : Loader有被Skip IC。
extern bool bLoaderHasSuck;                     //Steven 20101019 : Loader有成功吸到料
extern int  iManualRemoveTrayCnt;               //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
extern bool bBackupCleanOut;                    //jou 2010-12-07 : 防止index check oncyle與clean out funtion衝突
extern bool bEnterTestIF; //ChungHung 20121221
extern bool bCleanOutTrayEnd;                   //jou 2010-12-15
extern bool bLifterPause[7];                    //jou 2011-02-21 start : magazine沒動作會alarm
extern double fHotPlateExpansionCoefficient;    //Steven 20110324 : 加熱盤的膨脹係數
//jou 2011-03-24 start : Auto Site Mapping
extern bool bSiteMappingCHKOK;
extern int  iDoSiteMappingStep;
extern int  iNowSiteCT;
extern bool BackSiteFinish;
extern int  iGetTestData[4][8];
extern bool bShowAutoSiteMappingError;
extern int  iShuttleHeaterWaitTime;
extern bool bGetOpenBin;
//jou 2011-03-24 end
extern bool bRefreshFunction;//kevin 20110401 使用Refresh主畫面site跟著檔案變
extern int  MachineTypeChoice;  //0:HT9045 1:HT9046
extern int  SubMachineType;     //0:None 1:HT9046LA                             //Steven 20190307 : Add for HT-9046LA
extern int  MachineNumber;    //kevin  機台機號識別
extern int  NEW_MAX_Index_Col;
extern bool bIndexPlaceIcCheck;
extern unsigned int ContinuousPassBinCount[256]; //kevin 20140305 256 bin [30];         //Steven 20110502
extern unsigned int ContinuousPassBinBySocket[MAX_SOCKET_ROW][MAX_SOCKET_COL];    //Steven 20110919
extern int  iSpeedFast, iSpeedSlow, iSpeedY;            //Steven 20110503 : 240KG加速
extern bool bIndexEveryTimeCheckEPing;                  //Jou 20110505
extern int  iInitSpeedSh1, iInitSpeedSh2;               //記錄DATABase 速度資料 kevin 20110531
extern int  iPJogHighSpeedSh1,iPJogHighSpeedSh2;        //記錄DATABase shuttle速度資料 kevin 20110531
extern bool bWantToStopChamberFan;   //Steven 20110725 : 要可以關閉加熱風扇

//extern int iTrayXPitch[6],iTrayYPitch[6];   // [0] Aut01 [1] Aut02 [2] Aut03 [3]Fix1  [4]Fix2 [5]Fix3  kevin 20110729
//extern bool bTempOverCannotRunShowAlarmT[tcTotalCount]; //jou 2011-08-04 //Steven 20110607 : 溫度過高或過低不可以動
extern double dCCDTemperature;                      //Steven 20110924 : 確認CCD溫度
extern bool bUseFix3;                               //kevin 20110916 FIX3暫時不使用
enum eSynCardType{eSCTPcil112=0, eSCTPcil122};      //Steven 20111004 : 先達卡總類
extern int  iSynTekCardType;
extern bool bUT150Install[tcTotalCount];            //Steven 20111012 : 改用單獨宣告,不放在LastSet裡面
extern bool bUT150Install1To2[tcTotalCount];         //kevin 20190921 1對2site
extern bool OLPClientConnect;
extern bool bInitialMaxTime;                        //jou 2011-11-09 增加initial max time set

//jou 2011-11-14 start : 紀錄piggyback時間
extern bool bRecordPiggyBackStart;
extern bool bRecordPiggyBackEnd;
extern int iPiggyBackTime[2];
extern AnsiString pwPath;
extern AnsiString pwName;   //Steven 20221216 : 吳如春要改成從網路抓密碼本
//jou 2011-11-14 end

//ChungHung 20130910 add for SCK
//extern int iAutoCleanOutTime[2];
//ChungHung 20130910 add for SCK

extern int  iInArmCleaniWhichKit;//kevin 20120217 //0: 第一次8SITE  1:9-16 SITE
extern int  iTestData;
extern bool UseIndexCount;              //20120501 使用Index計數功能
extern bool MasheReCordData;            //kevin 20120616記錄程式離開資料
extern bool bChangeCleanPad;            //kevin 20120709 autoclean 需要開門
extern bool bIsAutoOneCycleAutoclean;   //kevin 20120710 目前onecycle 中準備 autoclean
extern int  iAutoCleanShuttle;          //kevin 20120711 Shuttle 偵測 Clean Pad是否放好
extern bool bACInitialStart;            //kevin 20120712 Initial 模式一開始作autoclean
extern bool bACInterval;                //kevin 20120712 index次數到再做autoclean
extern int  iAutocleanInitialStart;     //kevin 20120712 initial start 被啟動
extern bool bACManual;                  //kevin 20120712 Manual 主畫面秀 autoclean button
extern bool bAutoCleanCheckOpenDoor;    //Steven 20121015 : Auto Clean有Alarm要開後門

extern bool bRecIndexDropAlarm1;                    //jou 2012-01-17 紀錄index Drop alarm
extern bool bRecIndexDropAlarm2;                    //jou 2012-01-17 紀錄index Drop alarm
extern bool bHALTing;                               //jou 2012-01-30 常溫 & 機台沒在跑的時候chamber風扇不轉
extern bool bRTCArm1HalfViewError;                  //Steven 20120206 : RTC重複錯誤
extern bool bRTCArm2HalfViewError;                  //Steven 20120206 : RTC重複錯誤
extern bool bRTCFullViewError;                      //Steven 20120206 : RTC重複錯誤
extern int  XResolution;                            //Steven 20120206 : 畫面解析度
extern int  YResolution;                            //Steven 20120206 : 畫面解析度
extern bool bFilterTheAgainData;                    //Steven 20120222 : 過濾掉Duplicate的訊息
extern bool bCanUseHotPlateCheck;                   //Steven 20120323 : 可以檢查HotPlate的模式
extern bool bIndexCheckNoStopVaccum;                //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic
extern bool bCleanoutStart;                         //kevin 20120921 cleanout已使用 但onecycle 在做完了繼續cleanout
extern double T_MODE_SPEED;                         //jou 2012-05-30 T Mode Speed
extern bool bNeedCheckIndexToque;                   //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
extern bool bNeedCheckIndexToque1;                  //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
extern bool bNeedCheckIndexToque2;                  //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
extern const int ciIndex5FSetValue;                 //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
//==============================================================================
// GEM function Start
//==============================================================================
extern bool GEM_EMGPressed;                // 120

//Steven 20120410 Start: Hontech ATC
extern int  ATC_SYSTEM;                         // 2011.04.15 , Joye , ATC

extern int  ATC_MixMode;

extern int  iControllerCheckDelay;              // 2012.03.26 , Joye & Pig , ATC Connect Issue
extern bool bATCConnectError;                   //pig 2011.12.21 ATC改
extern int  iStopATCChillerType;                // 2012.05.07 , Joye , Chiller
//Steven 20120410 End: Hontech ATC
extern int INSTALL_HEAT_GUN;                    //Kevin 20120523 AirGun
extern int INSTALL_ATC_HEAT_GUN;                //JerryYang 20220408 : add for ATC3.5
extern int bUseCleanArm;                        //kevin 20140903
//extern bool bChangeSiteMAP;                     //kevin 20130104 可以改變sitemap

//---------------------------------kevin 20130321     Dell DUT(Socket Base)溫控器增加為4顆------------------------
extern int  iSocketBaseTempCount;               //Steven 20140116 : Socket數量,改用數字設定
extern int  iSocketBaseAdd4TempType;            //kevin 20130812 DUT 4個溫控器使用   0: pansonic  1:omron
extern bool bChamboFanCloseTemp;                //kevin 20130407關閉chambo 溫度
extern bool bIndexTimeSet;                      //kevin 20130321 設定index time
extern double fIndexTime;                       //kevin 20130321 設定index time

extern bool bIsContactforce;                    //kevin 20130418 contact force over error
extern int iShowSocketSensor;                   //kevin 20130504 等index z讓開再秀alarm
extern bool bIsSocketSensor;                    //kevin 20130504 socket sensor detect error
extern bool bASEK15UsePW;                       //kevin 20130701  Ase高雄K15
extern AnsiString ASEK15PassWord;               //kevin 20130701

//kevin 20130425   2013.01.11 Q_Q TSMC GPIB COMMAND
extern bool BUFFERDATAUSEREG;
extern int iReadTIntervelArm1;                  //kevin 20130425//2013.01.26 Q_Q TSMC GPIB COMMAND Part 3.
extern int iReadTIntervelArm2;                  //kevin 20130425//2013.01.26 Q_Q TSMC GPIB COMMAND Part 3.
extern AnsiString asGPIBTempShow[tcTotalCount];           //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
extern bool bAllTemperatureIsReady;             //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND

extern bool bUseAuto2Empty;                     //kevin 20120718 京元 auto2 load 空 TRAY
extern bool bAuto2Pause[7];                     //kevin 20120718

extern int iTestTimeUpErrContinueR;             //jou 2012-08-28 菱生要求 Test Time Up Error 之後的兩次測試, 不管結果如何都排到R
extern bool bASMFinishOneCycle;                 //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray
extern int iTestBinCount;                       //Steven 20121112 : RS232支援32Bin

extern bool bWakeupGPIBFile;                    //Steven 20110116
extern bool bWakeupEventLogSaverFile;           //Frank 20140902
extern bool bWakeupDMCCFile;                    //Alick 20160721 add Barcode File切換
extern int iInitStartDelayTimeCT;               //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time

//extern bool bCleanOutStart;                     //Kevin 20120921
extern bool bContinusRotate;
extern bool bResetIsPressed;                    //Steven 20130625 : 新的Reset方式
extern bool bResetShuttle;
extern bool bResetInArm;
extern bool bResetInArmTask;
extern bool bResetOutArm;
extern bool bResetOutArmTask;
extern bool bResetSortArm;                                                      //RogerYang 20250515 Add for 9046AU
extern bool bResetSortArmTask;
extern bool bResetIndexArm;
extern bool bResetIndexArm1Pick;
extern bool bResetIndexArm2Pick;
extern bool bResetLoadTray;
extern bool bResetUnLoadTray[MAX_AUTO_TRAY];

//------------TSMC kevin 20130425--------------------------------------------------
//2013.01.11 Q_Q TSMC GPIB COMMAND
extern bool BUFFERDATAUSEREG;
extern AnsiString asArmForce1;              //2013.01.11 Q_Q TSMC GPIB COMMAND
extern AnsiString asArmForce2;              //2013.01.11 Q_Q TSMC GPIB COMMAND

//extern int iReadTIntervelArm1;              //2013.01.26 Q_Q TSMC GPIB COMMAND Part 3.
//extern int iReadTIntervelArm2;              //2013.01.26 Q_Q TSMC GPIB COMMAND Part 3.
//extern AnsiString asGPIBTempShow[50];       //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
//extern bool bAllTemperatureIsReady;         //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
//------------------------------------------------------------------------------
extern bool bNeedAskStartMode;              //ChungHung 20130529 add manual change start mode 如果是InitialStart 按下Start時要詢問
extern int iInArmBlockIndex;                //2014-03-04    Dell    for SPIL WLP Add Tray Block
extern int iOutArmBlockIndex;               //2014-03-04    Dell    for SPIL WLP Add Tray Block
extern int iSortArmBlockIndex;                                                  //RogerYang 20250516 Add for 9046AU
extern bool bFirstTrayNeedAlarm;            //ChungHung 20140521 add for ATK

//==============================================================================
extern double dTempMax;
extern double dTempMin;
extern bool bCCDOverTemp;
//==============================================================================
//各機型DI/DO 數量定義
extern const int DI_IP_Max_9045;
extern const int DI_IP_Min_9045;
extern const int DO_IP_Max_9045;
extern const int DO_IP_Min_9045;

extern const int DI_IP_Max_9046;
extern const int DI_IP_Min_9046;
extern const int DO_IP_Max_9046;
extern const int DO_IP_Min_9046;

extern const int DI_IP_Max_9046LS;
extern const int DI_IP_Min_9046LS;
extern const int DO_IP_Max_9046LS;
extern const int DO_IP_Min_9046LS;

extern const int DI_IP_Max_9045_12Site;
extern const int DI_IP_Min_9045_12Site;
extern const int DO_IP_Max_9045_12Site;
extern const int DO_IP_Min_9045_12Site;
//==============================================================================
//JerryYang 20151109
extern const int Bit0_HandlerReBoot;            //JerryYang 20151109 bit0 PASS
extern const int Bit1_HandlerOutputFull;        //JerryYang 20151109 bit1 無空盤可放至Auto
extern const int Bit2_HandlerInputEmpty;        //JerryYang 20151109 bit2 Loader tray有無IC
extern const int Bit3_ContactorCleaning;        //JerryYang 20151109 bit3 做AutoClean
extern const int Bit4_HandlerDiagnostics;       //JerryYang 20151109 bit4 設定參數中
extern const int Bit5_IndexCheck;               //JerryYang 20151109 bit5 PASS
extern const int Bit6_Reversed;                 //JerryYang 20151109 bit6 PASS
extern const int Bit7_HandlerGuardband;         //JerryYang 20151109 bit7 PASS
extern const int Bit8_HandlerJam;               //JerryYang 20151109 bit8 機台JAM
extern const int Bit9_HandlerStop;              //JerryYang 20151109 bit9 機台停止中
extern const int Bit10_HandlerSoak;             //JerryYang 20151109 bit10 機台等待加熱完成,停止中
extern const int Bit11_HandlerDoorOpen;         //JerryYang 20151109 bit11 門禁
extern const int Bit12_HandlerEmpty;            //JerryYang 20151109 bit12 機台內無IC
extern const int Bit13_HandlerOk;               //JerryYang 20151109 bit13 PASS
extern const int Bit14_Unloading;               //JerryYang 20151109 bit14 Unloading中
extern const int Bit15_Loading;                 //JerryYang 20151109 bit15 Loading中
extern const int Bit16_Reversed;               //JerryYang 20151109 bit16 IC都在Auto
//==============================================================================
//JerryYang 20161007
extern const int iHPNoUse8suck;
extern const int iHPWideHP;
extern const int iHP_Pin_RightSide;
extern const int iHP1x2Step2;
extern const int iHP1x2Step3;
extern int i8PickerHPMode;
//==============================================================================
//JerryYang 20170531 (wei) 敲tray方式新增震動馬達
extern const int NonVibration;
extern const int VibrationCylinder;
extern const int VibrationMotor;
//==============================================================================
extern bool bGail_HighSpeedMode;
//==============================================================================
extern bool bK12TempHeadChoiceChambo;                                           //kevin 20130507 K12  TSMC HEAD溫度改成 CHAMBO溫度
extern int  iIndexArm;                                                          //kevin 20130605 K12 contract mode 確認 哪支arm在下
extern int  iArmEPUse;                                                          //kevin 20130611 0:不充氣    1:arm1 充氣   2:arm2
extern bool bShuttleCooling;                                                    //jou 2010-06-09 start
extern bool bChamboCooling;                                                     //kevin 20130109
extern bool bHotplateCooling;                                                   //jou 2013-11-07

extern int AccessLevel;
extern bool bRefreshFunction;                                                   //kevin 20110401 使用Refresh主畫面site跟著檔案變
extern bool bChamboCooling;                                                     //kevin 20130109
extern bool bFirstRecordLoaderData;                                             //jou 2013-05-31 first record loader data
extern bool bContinuoussPass;                                                   //kevin 20131009 pass IC過多秀警告
extern bool bContactCounOven;                                                   //kevin 20131009 CONTACT COUNT 超過多設定次數秀警告
extern bool bMoveInArm2XYToWaitOk;                                              //Steven 20131025 : 要先動到安全位置才可以再到別的地方
extern bool bInarmZOk;                                                          //kevin 20131119 Z軸正在吸取不能被中斷
extern bool bContaceTorque;                                                     //kevin 20131217 TOR-> 單一顆KG
extern bool bHomeinitialCheckPushZ1;                                            //kevin 20131218 歸hom前檢查是否有tray放在hotplate 造成機構損壞 按z1 確認
extern bool bHomeUnlock;                                                        //kevin 20131218 home按z1 解開 機台可以動作

extern int iWhichShuttle0000;                                                   //ChungHung 20130910 alter for SCK can close site by Index
extern double dTestSec;                                                         //2013-11-27    Dell 測試時間  //jou 2014-09-24 int -> float bWhenHappenTestedTimeBelowUseInitialDelay 測試秒數小於1 sec會誤判
extern int iSoakTimer;                                                          //2013-11-27    Dell Index Soak time 計數
extern int iStableTime;                                                         //JerryYang 20210122 : ASE-CL新增待溫功能
extern int iSoakTimer_Start;                                                    //2013-11-27    Dell Index Soak time 計數
extern int iInitialSoakTimer;                                                   //Steven 20140827
extern int iInitialSoakTimer_Start;                                             //Steven 20140827

extern int iATCTempWaitTimer;                                                   //Ifor 20230608 add:KYEC 要求新增主畫面顯示ATC Temp Wait 倒數
extern int iATCTempWaitTimer_Start;                                             //Ifor 20230608 add:KYEC 要求新增主畫面顯示ATC Temp Wait 倒數

extern bool bNeedInitialTestDelay;                                              //ChungHung 20140425 add for TSMC Device
extern int  iInitialCount;                                                      //ChungHung 20140801 add Korea Want to count down in main status
extern bool bFirstDeviceInitialTestDelayWhichOutAfterAutoClean;                 //ChungHung 20141023 add for SCK request
extern bool bDoAfterAutoCleanFunctionUseInitialDelay;                           //ChungHung 20140105 add for SCK
extern bool bDoEveryFirstDeviceFunctionUseInitialDelay;                         //ChungHung 20140105 add for SCK
extern bool bDoAfterShowAlarmMessageUseInitialDelay;                            //ChungHung 20140105 add for SCK
extern bool bDoOTDOffUseInitialDelay;                                           //Steven 20160818 : OTD打開Delay
extern bool bDoAfterOpenHeatDoorUseInitialDelay;                                //ChungHung 20140105 add for SCK
extern bool bDoWhenHappenTestedTimeBlowUseInitialDelay;                         //ChungHung 20140105 add for SCK
extern int  iAfterTestedCount;                                                  //ChungHung 20140730 add for ATK function after tested delay time
extern bool bDoWhenNoFullSiteUseInitialDelay;                                   //wei 20151228 No FullSite delay
extern bool bTestFinishToNextTestOver;                                          //kevin 20160310 測試完成到下一次完成超過所設時間須啟動

extern bool bButtonManualStep;                                                  //ChungHung 20150526 add for QualComm US
extern bool bButtonManualTStart;                                                //ChungHung 20150526 add for QualComm US

extern bool bDoWhenPressStopOverUseInitialDelay;                                //ChungHung 20150526 add for ATK want to even stop over will use initial delay
extern bool bHISIInitiayDelay;                                                  //kevin 20200110 add 海司強至initial delay
extern bool bCancelHISIInitiayDelay;                                            //Ifor 20230201 add: 取消海司強至initial delay
//==============================================================================
extern AnsiString DownloadWorkFile_NET;                                         // 2013.12.03 , Joye , KYEC FTP //20140103  wei
extern int LastSetTemperature_NET;                                              // 2013.12.03 , Joye , KYEC FTP
extern AnsiString SYS_SetupFile;
extern bool bFTPDownloadSetupFile;                                              // 2013.12.02 , Joye , KYEC FTP Download
extern bool bBarcodeReader;                                                     //20140120 wei
//extern bool bTempSet;                                                         //20140103 wei

extern bool ReEnterBarcode[20];                                                 //20140310  wei   Barcode Reader
//extern char PcName[255] ;                                                     //Steven 20110131 : 電腦名稱
//extern unsigned long PcNameLen;                                               //Steven 20110131 : 電腦名稱長度
//==============================================================================

extern const int ArmMaxPitch_13Suck;                                            //使用1, 3吸嘴
extern const int ArmMaxPitch;                                                   //Steven 20140212 : For Xilinx 31x31生產8Site
extern int iAutoSkipCT;                                                         //jou 2013-09-23 add Auto SKIP CT
extern int iLoaderDropErrRetryCT[MAX_ARM_Row][MAX_ARM_Col];

extern bool bTestingStopAllMotor;                                               //jou 2013-09-25 Testing Need Stop All Motor
//==============================================================================
//行車紀錄器
extern bool bCarRecordTimeStart;                                                //wei 2013-12-09
extern bool bCarRecordTimeEnd;                                                  //wei 2013-12-09
extern TQPF_Timer  CarRecord;
//==============================================================================
extern bool bDutHasOverTemp;                                                    //Steven 20140308 : DUT溫度異常到下次Start前不加熱
extern bool bDutflag[32];                                                       //ChungHung 20140611 fix 紀錄測試的Dut位置 對應錯誤

//extern bool bIndexDutflag[2][4][8];             //kevin 20140318 紀錄目前index有IC需測試
extern bool bSkipNeedCheckVac[2][2][8];                                         //JerryYang 20170609 (wei) 記錄pick up error按skip的site
extern bool bArm1PressSkipNeedDownCheckVac;
extern bool bArm2PressSkipNeedDownCheckVac;
extern int  iBarcodeTimeCount[bcTotal];                                         //20140310  WEI : [A09] Barcode Reader持續時間
extern bool bOneCycleOperateChangeON_line;                                      //kevin 20140411 one cycle完如果是op模式off-line 需切換 on-line 並且發警告
extern int iOff_LINE_Mode;                                                      //kevin 20140411 目前機台工作模式是off-line

extern bool bTesterDucking;                                                     //jou 2014-04-25 Tester Docking Wait delay time
extern int  iTesterDucking;                                                     //jou 2014-04-25 Tester Docking Wait delay time

extern bool bNeedTesterDuckingFinishLightYellowAndAlarmOn;  //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
extern bool bTesterDuckingFinishLightYellowAndAlarmOn;      //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
extern bool bRTCAutoModelVerifyFirstTime;                   //jou 2014-06-24 RTC 自動進行Model驗證

extern int iDefHonPrecLevel   ; //jou 2014-06-19 Security Have 5 Level
extern int iDefSupervisorLevel; //jou 2014-06-19 Security Have 5 Level
extern int iDefEngineerLevel  ; //jou 2014-06-19 Security Have 5 Level
//ChungHung 20140716 modify SCK complain count never clear when change add reset count
//放到全域變數
extern unsigned int ContinuousFailSKTCount[MAX_SOCKET_ROW][MAX_SOCKET_COL];
extern unsigned int ContinuousFailARMCount[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];
 //ChungHung 20131223 add for SCK request start
extern unsigned int ContinuousFailSKTCount_AutoClean[MAX_SOCKET_ROW][MAX_SOCKET_COL];
extern unsigned int ContinuousFailARMCount_AutoClean[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];

//ChungHung 20140730 add ContinuousFailHaveOneCycle start
extern unsigned int SpecialBinContinuousFailSKTCount[MAX_SOCKET_ROW][MAX_SOCKET_COL];
extern unsigned int SpecialBinContinuousFailARMCount[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];
//ChungHung 20140730 add ContinuousFailHaveOneCycle end

extern int iAutoCleanByBinCount[TEST_MAX_BIN];                                      //Steven 20160308 : By Bin count do auto clean
extern int iAutoCleanBySiteCount[MAX_SOCKET_ROW][MAX_SOCKET_COL][TEST_MAX_BIN];     //Steven 20160308 : By Bin pre site count do auto clean

extern int iLoadPersentCT[TEST_MAX_BIN];            //Steven 20140830 : 改成全域變數
extern int iLoadCountCT[TEST_MAX_BIN];              //Steven 20140830 : 改成全域變數
extern bool bAlarmReset;                            //Steven 20140905 : 紀錄有被按下Alarm Reset

extern bool bPiggyBackIndexCheck;                   //jou 2014-09-03 Function State Stop Firt Initital Delay Time
extern int iContinueAutoSkipAutoTrayEnd;            //ChungHung 20141002 add for KYEC AutoRetest
extern AnsiString sMachineState;                    // 2013.09.30 , Joye , KYEC SECS/GEM
extern AnsiString sMachinePreState;                 // 2013.11.27 , Joye , KYEC SECS/GEM
extern TQPF_Timer lHandlerStopTime;                       //jou 2014-09-21 Show Handler Stop Time

extern bool bTesterLowYieldOneCycle;                //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
extern bool bSwitchArm2;                            //Steven 20140922 : Arm2當作指紋測試
extern AnsiString AseIcRecord;                      //kevin 20161228 記錄一支ARM IC吸取 資料

extern bool bTimeOutForNoFullSite;                      //Steven 20141016 : FullSite的Test Time Out
extern int ASE_InTrayNum;                               //input tray 數量
extern int iLoadTrayCount;

extern AnsiString ASET_FileNAME;                        //kevin 20140918  ASE高雄自動記錄檔案檔名
extern AnsiString ASET_ScheduleNAME;                    //kevin 20140918  ASE高雄自動記錄檔案生產批號
extern AnsiString ASET_INTQTY;                          //kevin 20140918  ASE高雄自動記錄檔案生產數量
extern AnsiString ASET_StartTimeNAME;                   //kevin 20140918  ASE高雄自動記錄檔案開始時間
extern double fRecindexCycleTim;                        //kevin 20141024 ASE index cycle time  sec
extern AnsiString RecindexCycleTim;                     //kevin 20141024 ASE index cycle time  sec

extern AnsiString aRecordAseError;                      //kevin 20141028 紀錄發生 alarm
extern AnsiString AseSaveFilePath;                      //kevin 20150212 紀錄發生

extern bool bStartRecordAseError;                       //kevin 20141028開始紀錄 alarm code
extern bool bReceiveSchedule;                           //kevin 20141104 收到ase 批號

extern int iInArmXBase;                                 //Steven 20141029 : XY-Pitch
extern int iInArmYBase;
extern int iOutArmXBase;
extern int iOutArmYBase;
extern int iSortArmXBase;                                                       //RogerYang 20250506 Add for 9046AU
extern int iSortArmYBase;

extern int iInArmShtXCenterPos;
extern int iInArmShtYCenterPos;
extern int iOutArmShtXCenterPos;
extern int iOutArmShtYCenterPos;
extern int iSortArmShtXCenterPos;                                               //RogerYang 20250506 Add for 9046AU
extern int iSortArmShtYCenterPos;

extern int iInitContactCount;                                                   //Steven 20141117 : 起測時溫度要補Offset

extern int iRotato_In_Row;                                                      //wei 20141208 2x2 1x2  Pitch >3750
extern int iRotato_Out_Row;                                                     //wei 20141208 2x2 1x2  Pitch >3750

extern bool bEnableTempLess30degShowLight;                                      //jou 2014-12-19 Temperature Less 30 deg. Show Light
extern bool bTempLess30degShowLightFlag;                                        //jou 2014-12-19 Temperature Less 30 deg. Show Light

extern bool bEnableTempHeaterOkShowLight;                                       //jou 20180529 : Temperature Heater Ok Show Light
extern bool bTempHeaterOkShowLightFlag;                                         //jou 20180529 : Temperature Heater Ok Show Light

extern int iAutoSiteMap[4][8];                                                  //kevin 20150113 記錄測試機site map
extern int iAutoSiteMapBin[4][8];                                               //kevin 20150113 記錄測試機site map
extern int iAutoSiteRecordIC[2];                                                //kevin 20150114 記錄有ic位置
extern int iAutoSiteMapSocketPass[4][8];                                        //kevin 20160513 記錄測試機site map 此socket 是否pass過  0:沒使用 1;pass 2;fail
extern int bTestBinDataError;                                                   //kevin 20150202 測試bin 別沒設定或關site 有bin 資料  0:不使用 1; arm 1 2;arm2
extern bool bLowYieldAutoCleanEnd;                                              //wei 20141216  LowYieldautoclean  避免重複進去

extern int iLowYieldSiteCount[4][8];                                            //wei 20141201 Low Yield Auto Clean    low Yield 統計
extern bool bLowYieldCleanOut;                                                  //wei 20141201 Low Yield Auto Clean    是否clean out
extern bool bLowYieldCloseSite[2][4][8];                                        //wei 20141201 Low Yield Auto Clean    close site
extern int iLowYieldCloseCount;                                                 //wei 20141201 Low Yield Auto Clean    close site  統計
extern bool bStandardYield;                                                     //wei 20141201 Low Yield Auto Clean    Yield 設定
extern int iStandardYield[4][8];                                                //wei 20141201 Low Yield Auto Clean    StandardYield
extern int iYield[4][8];                                                        //wei 20141201 Low Yield Auto Clean    Yield
extern int iDifferenceYield[4][8];                                              //wei 20141201 Low Yield Auto Clean    DifferenceYield
extern bool bSuckEnd[MAX_ARM_Row][MAX_ARM_Col];                                 //wei 20150318 suck完就等於true

extern bool bAutoSkipFlag;                                                      //jou 20150320   auto skip 次數時間導致Hang up
extern bool bSystemClose;                                                       //kevin 20150423 mainForm移過來

extern bool bZ1PickShuttle;                                                     //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
extern bool bZ2PickShuttle;                                                     //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
extern bool bAseStart;                                                          //kevin 20150427
extern bool bAseHome;                                                           //kevin 20150427
extern bool bAsePause;                                                          //kevin 20150427
extern bool bAseReset;                                                          //kevin 20150427
extern bool bAseCleanOut;                                                       //kevin 20150427
extern bool bAseSKIP;                                                           //kevin 20150427
extern bool bAseRetry;                                                          //kevin 20150427
extern bool bAseOneCycle;                                                       //kevin 20150427
extern bool bAseAlarmReset;                                                     //kevin 20150427
extern bool bAseTrayEnd;                                                        //kevin 20150427
extern bool bAseTrayFeed;                                                       //kevin 20150427
extern bool bTSTART;                                                            //kevin 20160303
extern bool bSTEP;                                                              //kevin 20160303

extern bool bUse_NewAutoCleanForm;                                              //kevin 20150507 使用新AUTOCLEAN FORM
extern bool bTEMPCTRL_Shuttle_TOGTHER;                                          //kevin 20150508 Shuttle不分段加熱
extern bool bUseFix3CylinderActive;                                             //ChungHung 20150520 add fix Fix3 full 和 ShuttleShanke 衝突

extern bool bAutoReTest_ART;                                                    //kevin 20150529 要執行AUTO RETEST
extern bool bFirstYieldCmp_ART;                                                 //kevin 20150529 第一次AUTO RETEST YIELD比對
extern double fFirstYieldSet_ART;                                               //kevin 20150529 第一次AUTO RETEST YIELD比對值設定   First yield >= 設定值不須RT
extern bool bOpenShortYieldCmp_ART;                                             //kevin 20150529 第一次AUTO RETEST open Short YIELD比對
extern double  fOpenShortYieldSet_ART;                                          //kevin 20150529 第一次AUTO RETEST open Short YIELD比對值設定    First yield >= 設定值不須RT
extern bool bRecoverRateYieldCmp_ART;                                           //kevin 20150529 第一次AUTO RETEST RecoverRate YIELD比對
extern double  fRecoverRateYieldRT1Set_ART;                                     //kevin 20150529 第一次AUTO RecoverRateYieldRT1 YIELD比對值設定
extern bool bSetYielddiffCmp_ART;                                               //kevin 20150529 第一次AUTO RETEST YIELD比對  FT - FT+RT1 Pass yield
extern double fbSetYielddiffSet_ART;                                            //kevin 20150529 第一次AUTO RETEST YIELD比對值設定  FT - FT+RT1 Pass yield <= 設定值則不須RT2
extern bool bSetSpecialBin_ART;                                                 //kevin 20150529 第一次AUTO RETEST
extern bool bAutoDetectBin_ART;                                                 //kevin 20150529 第一次AUTO RETEST
extern int iAutoRetestLimit;                                                    //kevin 20150601 做AUTO RETEST最大次數
extern int iTrayArmAutoRetestBuffer;                                            //kevin 20150606 高雄鎖住 tray arm速度
extern int iIonFanDleayAlarm;                                                   //kevin 20150610 高雄離子風扇偵測發alarm時間
extern bool bRunTrayError;                                                      //kevin 20150612 run tray 模式吸到ic 發alarm 手動取出tray
extern int iCountReceiveTray_ART;                                               //kevin 20150613 計算收回tray
//extern bool HighTemperatureSet150;                                              //wei 20150617 改機最高溫150度
//extern bool HighTemperatureSet155;                                              //Sam 20240118 新增 155度 模式
//extern bool HighTemperatureSet175;                                              //Frank 20160705 add 改機最高溫175度
//extern bool bHighTempSetCustomizeEnable;                                        //Jimmychiu 20250626 : 最高溫度自訂
//extern double dHighTempSetCustomizeValue;                                       //Jimmychiu 20250626 : 最高溫度自訂
extern int iTempLimitation;                                                     //Steven 20250701 : 整合機台溫度設定
extern int  DewPoint_Hardware_Install;                                          //Steven 20191017 : 露點計
extern double dAdamValue_mA;
extern double dAdamValue_Degree;
extern bool bDewPointalarm;
extern bool bGpibRS232Error;                                                    //wei 20150617 Add version control
extern int iEPControlValue;                                                     //Ifor 20150710 EP 流量控制
extern int iSendGpibTestHome;                                                   //kevin 20150626 送出測試訊號 等收到資料才能歸home
extern bool bSecsGemCanStart;                                                   //wei 20150630  SecsGem Can Start
extern double fTrayYield[eTrayCount];                                           //JerryYang 20220909 : 10->eTrayCount        //kevin 20150630  pass fail 率
extern bool bUsePassYieldTray[eTrayCount];                                      //JerryYang 20220909 : 10->eTrayCount        //kevin 20150701 fail bin 在良率判斷裡面算pass yield
extern bool bUseNewCleanModeKit;                                                //kevin 20150701 使用autoclean模組 校正位置在 基座上 tray pin1
extern bool bART_needRT2;                                                       //kevin 20150717  RT 完成還需要rt2
extern bool bART_RT2RunNoChangeMode;                                            //kevin 20150717  只退fail RT2 不能更改測試模式
//extern bool bEmptyAndColorMustManualRemove;                                     //Sam 20230203 : 修正P24放Tray異常 Mark //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查
extern bool bRotateChange;                                                      //Frank 20150626 : for矽格 要有IC方向顯示
//extern int iArmXShuttle1OffsetPos;                                              //kevin 20150720 In arm X AutoClean offset 吸取Shuttle 1 IC Offset
//extern int iArmYShuttle1OffsetPos;                                              //kevin 20150720 In arm Y AutoClean offset 吸取Shuttle 1 IC Offset
extern int HotplatlXOffset;                                                     //kevin 20150720 add hotplate Xpos
extern int HotplatlYOffset;                                                     //kevin 20150720 add hotplate Ypos
extern int HotplatlPickOffset;                                                  //kevin 20150720 add hotplate Pick
extern int HotplatlPlaceOffset;                                                 //kevin 20150720 add hotplate place
extern int HotplatePitchOffset;                                                 //kevin 20150720 add hotplate Pitch
//extern int ShuttlePitchOffset;                                                  //kevin 20150720 add Shuttle Pitch
//extern int iArmPickShuttlePos;                                                  //kevin 20120623 In out arm 吸取Shuttle IC Offset
//extern int iArmPlaceShuttlePos;                                                 //kevin 20120623 In out arm 放Shuttle IC Offset
extern int iArmPickTrayPos;                                                     //kevin 20120623 In out arm 吸取TRAY IC Offset
extern int iArmPlaceTrayPos;                                                    //kevin 20120623 In out arm 放TRAY IC Offset
extern int iIndexPickShuttlePos;                                                //kevin 20120623 Index 吸取 Shuttle IC Offset
extern int iIndexPlaceShuttlePos;                                               //kevin 20120623 Index 放 Shuttle IC Offset
extern int iIndexWorkDownPos;                                                   //kevin 20120623 Index Clean down pos
extern int iIndexWorkUpPos;                                                     //kevin 20120623 Index Clean down pos
extern int bCleanHotplate_ART;                                                  //kevin 20150722 清hotplate 開關site
// for autoclean offset 20150720
extern bool bSECSGEMAlarm;                                                      //wei 20150817 S10F3 Alarm Reset按鍵
extern AnsiString asSECSGEMChangeName;                                          //wei 20150803
extern AnsiString asSECSGEMChangePassword;                                      //wei 20150803
extern bool bSECSGEMNoSendEC;                                                   //wei 20150803
extern bool bSECSGEMbyPass;                                                     //wei 20150803
extern bool bCheckOTDStatus;                                                    //wei 20150810  OTD Log
extern AnsiString asSecsGemBarCode;                                             //Frank 20170109 modify asBarCode=>asSecsGemBarCode
extern AnsiString asSecsGemBarCodePasswoed;                                     //Ifor 20180227 (Steven) add BarCode Password
extern AnsiString asBarCode;                                                    //wei 20150811 SecsGem BarcodeReader
extern bool bOTDRecord;                                                         //kevin 20150827 OTD Log
extern bool bSECSGEMConnectionFail;                                             //wei 20150817 SECSGEM 斷線
extern bool bSECSGEMConnectionFailOneCycle;                                     //wei 20150824 Secs_Gem 斷線Onecycle
extern AnsiString asSECSGEMPassWord;
extern int iAutoRetestLimitFile;                                                //wei  20150825 做AUTO RETEST最大次數
extern double iFailYieldRate_ARTFile;                                           //wei  20150825 AUTO RETEST YIELD比對

extern int iAutoLeastRetestLimitFile;                                           //wei  20160204
extern double dFailYieldRate_ARTFTFile[eTrayCount];                             //wei  20160204
extern double dFailYieldRate_ARTRTFile[eTrayCount];                             //wei  20160204
extern bool bAutoLeastRetestFile;
extern bool bUseFailNoDistinction;
extern int iUseFTFailYield;
extern int iUseRTFailYield;
extern int iUseFTFailYieldModel;
extern int iUseRTFailYieldModel;

extern bool bPassTrayFeed;                                                      //wei 20150826 Pass Tray Feed
extern int iAutoTrayCount[eTrayCount];                                          //wei 20150828
extern bool bIndexCheckState;
extern bool bErrorAutoClean;

//extern bool bWaitStartLotAutoRetestGPIB;                                        //jou 2015-10-02 Auto Retest GPIB mode
//extern bool bWaitEndLotAutoRetestGPIB;                                          //jou 2015-10-02 Auto Retest GPIB mode
//extern bool bEndLotAutoRetestGPIB;                                              //jou 2015-10-02 Auto Retest GPIB mode
//extern bool bFinEndLotAutoRetestGPIB;                                           //jou 2015-10-02 Auto Retest GPIB mode
//extern int iRetestFlagART;                                                      //jou 2015-10-02 Auto Retest GPIB mode
//extern bool bFirstTestAutoRetestGPIB;                                           //jou 2015-10-02 Auto Retest GPIB mode        //Steven 20170314 : ART的變數移到LastSet

extern int iTemperatureOk;                                                      //kevin 20150914 加熱完成
extern bool iTemperatureOkFirstTime;                                            //kevin 20150914 加熱完成程式一開始
extern int iRecordIndexPosition;                                                //kevin 20150915

extern int  iAseRetry;                                                          //kevin 20150925
extern int  iAseTrayFeed;                                                       //kevin 20150925
extern int  iAseTrayEnd;                                                        //kevin 20150925
extern int  iAseAlarmReset;                                                     //kevin 20150925
extern int  iAseOneCycle;                                                       //kevin 20150925
extern int  iAseCleanOut;                                                       //kevin 20150925
extern int  iAseSKIP;                                                           //kevin 20150925
extern int  iAseHome;                                                           //kevin 20150925
extern int  iAsePause;                                                          //kevin 20150925
extern AnsiString ASE_ReturnState;                                              //kevin 20150925  回傳目前機台狀況
extern bool bRunManualSortMode;                                                 //Steven 20150915 : For TSMC 手動整盤功能
extern bool bResetModeAndCleanOut;                                              //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
extern bool bResetModeAndCleanOutAndNoTest;                                     //Jimmychiu 20210927 Reset clean out with no tester

//wei 20150922 OCR
extern bool bBarCodeError;                                                      //wei 20150713 BarCode掃全部
extern int iBarCodeErrorC;                                                      //wei 20150713 BarCode掃全部
extern int iBarCodeErrorR;                                                      //wei 20150713 BarCode掃全部
extern AnsiString  asOCRBarCode;                                                //wei 20150713
extern int iOCRCount;                                                           //wei 20150713
extern int iNewOCRCount[10];
extern int iOCRPosition;
extern bool bOCRReadOK;                                                         //wei 20150713
extern bool bOCRNoIC;                                                           //wei 20150713
extern bool bOCRConnect;
extern bool bInspectError;
extern bool bOCRStartPosition;
extern int iOCRConnectCount;
extern bool NeedWaitTrayArm;
extern bool bOCROffLine;
extern bool bOCROnLine;
extern bool bChangeFile;
extern bool bOCRUser;
extern bool bOCRPassWord;
extern bool bSignIn;
extern bool bOCRNewFile;                                                        //wei 20161028回傳OCR檔名
extern bool bOCROK[15];
extern bool bOCRError[10];
extern bool bCheckFile;
extern AnsiString asCheckFileName;
extern AnsiString asOCRCheck[20];
extern int iOCRCheck;
extern int iOCRLotTrayCount;
extern bool bOCRCheck;
extern bool bOCRCheckNG;
extern int  iATCForHSMode;                                                      //Ifor 20151026 :判斷ATC 模式是否符合 HS 規定
extern int iSH1_1BarcodePosition;                                               //wei 20150912 2D
extern int iSH1_2BarcodePosition;                                               //wei 20150912 2D
extern int iSH2_1BarcodePosition;                                               //wei 20150912 2D
extern int iSH2_2BarcodePosition;                                               //wei 20150912 2D

extern bool bOffsetEnterBarcode;
extern bool bHeightCalibrationFinish;                                           //JerryYang 20151028 add for Amkor,download recipe比對不符後,是否做完Height Calibration
extern int iRecordIndexPosition;                                                //kevin 20150915

extern int  iAseRetry;                                                          //kevin 20150925
extern int  iAseTrayFeed;                                                       //kevin 20150925
extern int  iAseTrayEnd;                                                        //kevin 20150925
extern int  iAseAlarmReset;                                                     //kevin 20150925
extern int  iAseOneCycle;                                                       //kevin 20150925
extern int  iAseCleanOut;                                                       //kevin 20150925
extern int  iAseSKIP;                                                           //kevin 20150925
extern int  iAseHome;                                                           //kevin 20150925
extern int  iAsePause;                                                          //kevin 20150925
extern AnsiString ASE_ReturnState;                                              //kevin 20150925  回傳目前機台狀況
extern double dTempOffset[2][tcTotalCount];                                     //kevin 20151023 紀錄讀取溫度offset 0:最新資料
extern bool bSendChangeTemp;                                                    //kevin 20151023  生產中改變溫度
extern bool bSendChangeTempError;                                               //kevin 20151023  生產中改變溫度溫度超過 +- 10發alarm
extern int  iSendChangeTempError;
extern bool bSendChangeTempDelayAlarm;                                          //kevin 20151023  生產中改變溫度溫度發alarm時間延長
extern int iTempSec;                                                            //kevin 20151023  溫度發alarm時間
//extern DWORD StartTemptim,EndTemptim, PassTemptim;                            //kevin 20151023  AutoTemp Offset time
extern int iYeildCT[TEST_MAX_BIN];                                              //wei 20151111
extern bool bLowYeildAlarm;                                                     //wei 20151116  Low Yield Onecycle中不alarm
//extern bool bOneCycleByAutoOffSite;
extern bool bSPBinYieldAlarm;                                                   //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm

extern int iRecordJamRateByTime_LoaderCount;    // 2015.11.11 , Joye , Add Jam Rate Record
extern int iRecordJamRateByTime_JamCount;       // 2015.11.11 , Joye , Add Jam Rate Record
extern bool bRecordJamRateByTime_Clear;         // 2015.11.11 , Joye , Add Jam Rate Record
extern bool bGPIBChangeSiteMap;                 //wei 20151127 GPIB Change Site Map
extern bool bGPIBChangeAlarm;                   //wei 20151127 GPIB Change Site Map
extern bool bSetByDLL;                          //JerryYang 20181126 (Steven) : support Epson DLL function
extern bool bHeightCalibrationFinish;           //JerryYang 20151028 add for Amkor,download recipe比對不符後,是否做完Height Calibration
extern bool bUnloading;                         //JerryYang 20151109 用來判斷機台是否正在Unload
extern bool bAutoCleaning;                      //JerryYang 20151109 用來判斷機台是否正在AutoClean
extern bool bScanKeyNo;                         //JerryYang 20151202 Add for TSMC 點SKIP或是RETRY都要再跳一次確認
extern bool bHasSaveSet;                        //Ifor 20151204 新增判斷機台有無修改設定檔
extern bool bOneTimes;
extern bool bEnableEmployeeIDCheck;             //Ifor 20180911 (Steven) : Add 啟動 Employee ID Check
//JerryYang 20150910 START:Auto Sorting BinTray by Out Arm when Clean Out
extern const int SortingBinTray_NOACT;
extern const int SortingBinTray_ACT;
extern const int SortingBinTray_CHECKOK;
extern const int SortingBinTray_ERR;
extern double dAdaptiveStardardYield;           //Sam 20230914 : 自適應性良率監控
extern int iAdaptiveACInterval;                 //Sam 20230914 : 自適應性良率監控
extern int iAdaptiveACIntervalNor;              //Sam 20240726 : AI Clean
extern int iAdaptiveLowYieldCntNor;
extern int iAdaptiveLowYieldCntMin;
extern int iAdaptiveContsLowerCnt;              //Sam 20230914 : 自適應性良率監控
extern bool bResetNotMsg;                       //Sam 20240215 : Tester time out show reset all ic
extern bool bRestModeBackupParm;                //Sam 20250820 : [I49] 清料時 Contact Heigh 要拉高
extern int iRestModeBackContactMode;
extern double dRestModeBackContactHeigh1;
extern double dRestModeBackContactHeigh2;

typedef struct TSortingBinTray_PAR_STRUCR
{
    bool bIsPickIC;
    bool bIsFindMaxInTrayXLine;

    int iPickSuckX;
    int iPickSuckY;

    int iPickTrayX;
    int iPickTrayY;

    int iICNum;
}TSortingBinTray_PAR;
typedef struct TSortingBinTray_SUCKDATA_STRUCR
{   //所有浮點數資料，轉成整數，單位為um
    int iXPitch;
    int iYPitch;
    int iXItem;
    int iYItem;
    int iBaseSuckX;
    int iBaseSuckY;
    int iSuckData[10][10];
}TSortingBinTray_SUCKDATA;

typedef struct TSortingBinTray_TRAYDATA_STRUCR
{   //所有浮點數資料，轉成整數，單位為um
    int iXPitch;
    int iYPitch;
    int iXDivision;
    int iYDivision;
    int iDirection;
    int iTrayData[100][100];
}TSortingBinTray_TRAYDATA;

typedef struct TSortingBinTray_CALDATA_STRUCR
{   //所有浮點數資料，轉成整數，單位為um
    int iCalXPos;
    int iCalYPos;
    int iUseSuck[10][10];
    int iUseTray[100][100];
}TSortingBinTray_CALDATA;

typedef struct TSortingBinTray_Data
{
    //X、Y、Z軸的馬達編號
    int iMotorX;
    int iMotorY;
    int iMotorZ;
    //固定吸嘴是第幾支吸嘴
    int iActionSucker;

    //吸嘴最大的X、Y數量
    int iMaxSuckerX;
    int iMaxSuckerY;

    //Z軸的安全高度
    int iSuckerSafeZPos;
    //Z軸取IC高度
    int iSuckerPickZPos[eTrayCount];
    //Z軸放IC高度
    int iSuckerPlaceZPos[eTrayCount];

    //此Tray盤是否有使用
    bool bTrayAction[eTrayCount]; //是否使用此Tray盤

    //Tray盤左上角基準點位
    int iXBasePos[eTrayCount];
    int iYBasePos[eTrayCount];

    //Auto 1~3、Fix 1~3的Tray盤資料
    int iTrayData[eTrayCount][MAX_X_ITEM][MAX_Y_ITEM]; //此Tray盤的IC資料

    //到Auto 1~3、Fix 1~3的點位、偏移量
    TRAY_TYPE_PARA TTraySpec[eTrayCount]; //此Tray盤的點位資料

    //Out Arm X方向最大吸嘴數量
    int iMAXSUCK_OutArmSuckX;
}TSortingBinTray_Data;

extern TSortingBinTray_Data SortingBinTray_Data;

extern TSortingBinTray_PAR MyParameter;
extern TSortingBinTray_SUCKDATA MySuckData;
extern TSortingBinTray_TRAYDATA MyTrayData;
extern TSortingBinTray_CALDATA MyCalData;

extern bool bSortingAllBinTrayFinish; //是否所有的Bin Tray IC都整盤完成
extern bool bSortingSuckMode;                   //Sam 20250415 : 修正 P27 整盤功能
extern int  iMainVCL_SeverDownload_Show ;       //2013.12.11 Q_Q SPIL Main Level
//JerryYang 20150910 End:Auto Sorting BinTray by Out Arm when Clean Out
extern int iOldRunStartMode;                    //wei 20151207
extern int iArmsitecount[2];                    //wei 20160107 No FullSite Add Offset
extern int  iOverDriveDistance     ;            //Steven 20151207 : OverDrive for TSMC
extern bool bDoOverDrive           ;            //Steven 20151207 : OverDrive for TSMC
extern int  iReContactCount        ;            //Steven 20151207 : Recontact for TSMC
extern int  iCurrentReContactCount ;            //Steven 20151207 : Recontact for TSMC
extern bool bDoReContact           ;            //Steven 20151207 : Recontact for TSMC
extern TColor tcBinColor[eTrayCount]       ;            //JerryYang 20220909 : 9->24 //Steven 20160310 : 改成有顏色的fail bin
extern int  CHECK_RANGE            ;            //2013-04-12    Dell 500->750 在KYEC舊機調不進去 //jou 2013-05-21 750 -> 500 出現Out Shuttle lose //Steven 20160108 : 改去Teaching調整
extern bool bForKyecBu3RunART;                  //wei 20151210 Loader使用不同Sensor  //wei 20161118 bRunART-->bForKyecBu3RunART
extern bool bARTTrayArmOK;                      //wei 20151217 ART 回盤OK
extern bool bRTLoaderTrayCount;                 //wei 20160112 ART R1不算空盤
extern bool bContsFailIgnoreStart;              //wei 20160115 銅鑼前幾顆不計算ContsFail
extern int  iContsFailIgnoreCount;              //wei 20160115 銅鑼前幾顆不計算ContsFail
extern bool bESDSendDataToHandle;               //kevin 20160106  送ESD資料給HANDLE
extern bool bTempHandleToESD;                   //kevin 20160106   送EP HANDLE資料給 客戶
extern bool bEPHandleToESD;                     //kevin 20160106  送EP HANDLE資料給 客戶
extern AnsiString bEPMessage;                   //kevin 20160106   存取EP資料
extern AnsiString bTempMessage;                 //kevin 20160106   存取溫度資料
extern  bool bESDSystemtype;                    //kevin 20160106 Frank 20150309 : 程式關閉後將ESD系統停止
extern int iSiteTotal;                          //kevin 20160125 目前機台模式所使用SITE數量
extern int iRetestNoIC[3];                      //wei 20160203 回盤為空盤不計數
extern int iATC70_Count;                        //Ifor 20160223 Add ATC 7.0 未收到溫度回傳次數
extern bool bRemoteAutomaticDeploying;          //wei 20160223 自動部屬
extern bool bIsShowPMAlarmMessage;              //wei 20160225 PMAlarmFunction
extern bool bAutoRetestJam;                     //wei 20160302 Jam Skip輸入顆數
extern AnsiString iJamSkipIC;                   //wei 20160302 Jam Skip輸入顆數
extern int iARTJamSkipICCount;                  //Frank 20160819
extern bool bCleanSkipICCount;                  //Frank 20160819
extern int iJamSkipICCount;                     //wei 20160302 Jam Skip輸入顆數
extern bool bSecsGemStatus;                     //wei 20160308
extern bool bReadSecsGemData;                   //wei 20160308 是否讀取SECSGEM資料
extern bool bSysLotStart;                       //Ifor 20160302 KYEC Lot Start
extern bool bEPLogStart_KYEC;                   //Ifor 20160302 KYEC EP Log Start
extern bool bEPLogEnd_KYEC;                     //Ifor 20160302 KYEC EP Log End
extern bool bTempLogStart_KYEC;                 //Ifor 20160302 KYEC Temp Log Start
extern bool bTempLogEnd_KYEC;                   //Ifor 20160302 KYEC Temp Log End
extern bool bESDLogStart_KYEC;                  //Ifor 20160305 KYEC ESD Log Start
extern bool bESDLogEnd_KYEC;                    //Ifor 20160305 KYEC ESD Log End
extern AnsiString sEPLogFile;                   //Ifor 20160302 KYEC EP Log File
extern AnsiString sTemperatureLogFile;          //Ifor 20160302 KYEC Temperature Log File
extern AnsiString sESDLogFile;                  //Ifor 20160305 KYEC ESD Log File
extern AnsiString sGroundManLogFile;            //Ifor 20250123 KYEC FTP UP Load GroundMan Log File
extern bool bNoUseAutoRecord;                   //wei 20160311
extern int iGalil_Z_SafePos;                    //Ifor 20160311 避免Home sensor 不亮
extern const int iGalil_Z_KYEC_SafePos;         //Ifor 20170509 (wei) 新增KYEC回Home後最小上升高度100
extern AnsiString sESDLogData;                  //Ifor 20160316 KYEC ESD Log Data
extern bool bESDDataReady;                      //Ifor 20160316 KYEC ESD Log Data
extern bool bATCWaterTempAlarm;                 //Ifor 20160625 ATC Water Temp Error
extern AnsiString asATCEvenLogFile;             //Ifor 20161202 KYEC FTP UP Load ATC Even Log File
extern bool bATCEvenLogEnd_KYEC;                //Ifor 20161202 KYEC ATC Even Log End

//20150604 Mylin Interval Total Yield Difference {
extern int iIntervalYield_TotalIntervalCount;
extern int iIntervalYield_IntervalCount;
extern int iIntervalYield_StartCount;
extern int iIntervalYield_StartPassCount;
extern double fIntervalYield_YieldHistory[5];
extern bool bIntervalYield_IsShowAlarm;
//20150604 Mylin Interval Total Yield Difference }
extern int iAlarm4ContinueType_ContinueCount_Last;      // 2015.05.05 , Joye , Alarm4 Continue Type
extern bool bAlarm4ContinueType_NeedClearCount;         // 2015.05.05 , Joye , Alarm4 Continue Type
extern AnsiString asBarCodeErrorSend;                   //wei 20160318 Barcode Error依客戶設定
extern AnsiString asBarCodeErrorCheckSum;               //KaiChen 20191121 ：中壢日月光 2D Check Sum
extern AnsiString sATCVerRead;                          //Ifor 20160321 add ATC Ver
extern int iNeedBarcodeCount[4];                        //wei 20160325 Barcode Rate計算
extern int iBarcodeErrorCount[4];                       //wei 20160325 Barcode Rate計算
extern int iBarcodePassCount[4];                        //wei 20160325 Barcode Rate計算
extern int iBarcodeAutoRetry[4];                        //wei 20160325 Barcode Rate計算
extern int iBarcodeDuplicate[4];
extern bool bBarcodeFirstAutoRetry[4];                  //wei 20160325 Barcode Rate計算
//extern bool iBinTray[5][9];                             //kevin 20160308 有設定Bin Tray  //kevin 20170223 (wei) 不使用
extern bool iBinTray[eTrayCount];                       //JerryYang 20220909 : 10->eTrayCount        //kevin 20170328 (Steven) add 判斷TRAY 是否使用 BIN
extern bool bFirstTest;                                 //kevin 20160311 Clean out後第一次測試時間
extern bool bTestOverTimeTempOffsetF;                   //kevin 20160312 起動距離上一次測試時間超過所設定時間溫度需補OFFSET
extern void InitialMemory();                            //Steven 20160319 : 初始化數值
extern bool bAlmLowYieldDoOneCycle;                     //JerryYang 20160401 Low Yield Alarm後要做OneCycle
extern bool bAlmSiteYieldDiffDoOneCycle;                //JerryYang 20160401 Arm Site Yield Different Alarm 後要做OneCycle
extern bool bAlmSiteYieldCmpDoOneCycle;                 //JerryYang 20160401 Site Yield Different Alarm 後要做OneCycle
extern AnsiString asErrPart;                            //JerryYang 20160401 One cycle後的yield要能顯示error part
extern AnsiString asErrPart1;                            //JerryYang 20160401 One cycle後的yield要能顯示error part
extern bool bAlmBinYieldDoOneCycle;                     //JerryYang 20160407 Bin Yield Alarm 後要做OneCycle
extern AnsiString asErrTemp;                            //JerryYang 20160407 Error message temp
extern bool bAlmContsFailBySocketDoOneCycle;            //JerryYang 20160407 ContsFailBySocket後要做OneCycle
extern bool bAlmConsecutiveFailureDoOneCycle;           //JerryYang 20160407 Consecutive Failure後要做OneCycle
extern int iErrArmTemp;                                 //JerryYang 20160407 Error Arm Temp
extern bool bNeedOneCycleByYieldAlm;                    //JerryYang 20160408 是否已觸發Yield 相關alarm
extern bool bYieldAlarmDoOneCycle;                      //JerryYang 20160322 Yield Alarm後要做One cycle的旗標
extern bool bSpecailBinConsFailByArmDoOneCycle;         //JerryYang 20161006 SpecailBinConsFailByArm 後要做OneCycle
extern bool bSpecialBinConsFailBySocketDoOneCycle;      //JerryYang 20161006 bSpecialBinConsFailBySocket 後要做OneCycle
extern bool bBinCountOverLimitDoOneCycle;               //JerryYang 20161006 BinCountOverLimit 後要做OneCycle
extern AnsiString sAlarmMes;                            //wei 20160407 Alarm Message
extern bool bOpenAllDoor;                               //wei 20160407 Alarm 後需要開門確認
extern bool iATCOnLine;                                 //Ifor 20160331
extern bool bRunATC;                                    //Ifor 20160331
//extern int  iUnloadMotorIndex[eTrayCount];
extern int  iTo6PosUnload[ePosTrayCount];
extern int  iTo3PosUnload[ePosTrayCount];

extern int  iTo6Unload[ePosTrayCount];
extern int  iTo3Unload[ePosTrayCount];
extern int  iSortTrayIndex[eTrayCount];
extern int  AddBinDisp[MAX_BIN_UNIT];
extern bool bSetTempChange;                             //Ifor 20160412 SetTemp Cheange
extern int  iL03SocketAirCoolingCT;                     //jou 2016-04-28 Socket Air Cooling contact count trun on
extern bool bCheckCodeError[4];                         //wei 20160505 Barcode 比對Lot
extern bool bCheckLotError[4];                          //wei 20160728
extern int iIONFANStatus_Secs[12];                      //wei 20160506 SECS ION 偵測
extern int iIONFANPower_Secs[12];                       //wei 20160506 SECS ION 偵測
extern bool bATC_EnablesChannel[32];                    //Ifor 20160506 add Handler 送給ATC 開關Site資料
extern int  iATC_Use_Heat_Count;                        //Ifor 20160506 add ATC Use Heat Count
extern bool bATC_SITE_2ND_CHECK[2];                     //Ifor 20160509 add ATC 第二點溫度 sensor 判斷
extern bool bOutShuttleMissIC;                          //wei 20160509 Out Shuttle Miss IC
extern bool bNoSendSiteOnOff;                           //wei 20160511 Send Site On Off
extern bool bDownloadFTP;                               //wei 20160512 避免Download失敗，下次就無法Download
extern bool bAmbientHotPlate;                           //kevincheng 20260525 : 渠梁常溫模式使用hotplate
extern int iOpenBin;                                    //kevin 20160513 AutoSite map
extern int iBinLast;                                    //kevin 20160513 AutoSite map
extern int iOpenBinCount;                               //kevin 20160513 AutoSite map
extern int iBackupDutOnOff[4][8];                       //JerryYang 20160519 備份開關site
extern int iBackupTestMode;                             //JerryYang 20160519 記錄原本的Test mode-
extern bool bLoadMachineRecord;                         //JerryYang 20160614 用來判斷是否執行過LoadMachineRecord函式 避免還沒讀取到機台資料Lastset就被改掉
extern bool bOCRConnectOK;                              //wei 20160613 ocr連線測試
extern bool bOCRConnectTest;                            //wei 20160613 ocr連線測試
extern int bOCRConnectTestCount;                        //wei 20160613 ocr連線測試
extern bool INSTALL_SOCKET_CLAMP;                       //JerryYang 20160523
extern int INSTALL_DOUBLE_EP;                           //Ifor 20190104 : Add Double EP Control
//AI(ht9045-v899) 20260504: INSTALL_DOUBLE_EP value constants. ADR-0004 / SPEC-V899-MultiEP-FullPort.
//   Existing code uses raw 0/1/2/3; new code may use these for readability.
//   DO NOT change to enum: would break INI int serialization and trigger BCB6 conversion warnings on 75+ existing usages.
#define DOUBLE_EP_NONE       0                          // Un Install
#define DOUBLE_EP_NORMAL     1                          // Normal (single Die Force)
#define DOUBLE_EP_INDIVIAL   2                          // Indivial EP (independent EP per site)
#define DOUBLE_EP_MULTI      3                          // Multi EP (Max Qual Site, SIGURD 16ch full port from V874.3)
extern int CHECK_EP_SETTING;                            //Steven 20240701 : EP檢查功能加上開關
extern bool bAutoTrackCanGoRear;                        //Isaac 20180109 (Steven) : auto123可前進後退
extern bool bCleanOutClosedMonitorVideo;                //JerryYang 20160621 UseMonitorVideoFunction
extern bool bHPCleanout;                                //wei 20160624 Hotplate clean out
extern int iHPShuttle;                                  //wei 20160624 Hotplate clean out
extern bool bGPIBLotEndCommand;                         //wei 20160624 GPIB Lot End Command
extern bool bGPIBLotStartCommand;                       //kevin 20190613 add lotstart GPIB Command lotstart
extern bool bASECleanOutCloseSite;                      //kevin 20160715 ASE clean out close Site 不退tray
extern bool bCancelErrorBin;                            //kevin 20160706 Error Bin 取消
extern int  iHWFix_BinBox;                              //kevin 20160819 error bin 要放到 Bin Box
extern int  iErrorBinBoxAlarm;                          //kevin 20160819 error bin alarm 數量
extern bool bBinError[2];                               //kevin 20160725 Error Bin 發生arm
extern AnsiString sArm1BinError;                        //kevin 20160725 記錄 Error Bin
extern AnsiString sArm2BinError;                        //kevin 20160725 記錄 Error Bin
extern bool bOpenSixDoorChk;                            //kevin 20160725 error bin 發生需開右後門確認
extern bool bFirstOpenPg;                               //kevin 20160725 程式一開始進來不往下執行
extern int iATC_TempIndex[4][2];                        //Ifor 20160719 第一點溫度索引  0:Index舊值 1:Index 差異次數 //Ifor 20160805 add Use Site
extern bool bNeedSendATCRunSelfTest;                    //Ifor 20160720 add for ATC Safe Test start
extern bool bNeedWaitATCRunSelfTestFinish;              //Ifor 20160720 add Wait ATC slef Test Report
extern bool bATCHasAlarmBinNeedToError;                 //Ifor 20160726 ATC Alarm Index IC Be Error
//extern int iATCHasAlarmBinNeedToError;                  //Ifor 20181219 :add KYEC 要求ATC 溫度異常時Device放置Out SH後才Alarm 0:未發生ATC溫度Alarm 1:ATC溫度Alarm 2:Index 上升完畢 3:Device 放置Out SH 上方並回到安全位置 //Ifor 20191209 取消ATC溫度異常Index上升功能
extern bool bRunAOI;                                    //wei 20160617 Vitrox
extern bool bAOIPassFail[2][MAX_ARM_Row][MAX_ARM_Col];                      //wei 20160617 Vitrox
extern int iAOIBin[MAX_ARM_Row][MAX_ARM_Col];                               //wei 20160617 Vitrox
extern int bAOIBin[MAX_ARM_Row][MAX_ARM_Col];                               //wei 20160617 Vitrox
extern bool bBarcodeConnect;                            //wei 20160728
extern bool bChangeBarcodeFileOK;                       //wei 20160729
extern bool bCatchTrayFinishAction;                     //Frank 20160711 add
extern AnsiString asOCRImageName;                       //wei 20160805
extern bool bOCRSkip;                                   //wei 20160805
extern bool bAutoRetestMusic;                           //Frank 20160822 add AutoRetestMusic
extern int iBarcodeReject;                              //wei 20160823  Consecutive Failure
extern bool bBarcodePassword;                           //wei 20160823  Lot check 錯誤需輸入密碼
extern bool bFixBinBoxIsFull;                           //kevin 20160822 bulk box
extern AnsiString asVer;//kevin 20160825
extern bool bSetupFileNotExist;                         //Ifor 20160822 add 工作擋不存在
extern bool bHasEnteredPEModel;                         //Ifor 20160822 add 進入工程模式
extern bool bEnablePEModel;                             //Ifor 20160822 add 啟動PE工程模式
extern bool bInitialATCSelfTest;                        //Ifor 20160829 add 機台Initial Start 需做 ATC Self Test
extern bool bStartATCRun;                               //Ifor 20160830 add Lot頁面啟動ATC
extern bool bSpecificBin[256];                          //kevin 20170828 33->256  JerryYang 20170221 (wei) 改成可複選bin //JerryYang 20160913 (wei) 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
extern AnsiString asBarcodeLotNumber;                   //wei 20160921 BarcodeLotNumber
extern bool bBarcodeAutoSkip[4];                        //wei 20160921 BarcodeLotAutoSkip
extern bool bBarcodeAutoSkipError[4];                   //wei 20160921 BarcodeLotAutoSkip
extern bool bTestSiteUseEE[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];//Alick 20160923 add for 第三組工程師用開關SITE
extern bool bSiteUseEE;                                 //Alick 20160923 add for 第三組工程師用開關SITE
extern bool bNeedKeyInSkipIC;                           //Frank 20161006 add
extern bool bInitialCleanCount;                         //wei 20160923 Tray Feed後，下一次開始需要Clean Count
extern int iSECSGEMPass;                                //wei 20160923 Secs Gem 回傳Pass/Fail顆數
extern int iSECSGEMPass_ART;                            //wei 20160923 Secs Gem 回傳Pass/Fail顆數
extern int iSECSGEMFail;                                //wei 20160923 Secs Gem 回傳Pass/Fail顆數
extern int iSECSGEMFail_ART;                            //wei 20160923 Secs Gem 回傳Pass/Fail顆數
extern bool bPickErrorAutoHome;                         //wei 20161005
extern bool bCCDcheckShuttle[2];                        //wei 20161102 IC置偏檢查
extern bool bClearRetryCnt[iTotalFunction];             //Sam 20200323 : Modify Tray Function//wei 20161219 Tray Mapping
extern bool bCCDTrayDeviceCount[3];                     //Sam 20190405 : Tray Decive Count
extern bool bNeedCCDTrayDeviceCount[3];                 //Sam 20190405 : Tray Decive Count
extern bool bCCDTrayID[2];                              //wei 20180808 MR Tray ID 2
extern int iRecordSiteMapOrder[4][8];                   //kevin 20161003 Site 順序
extern int iCloseSiteMap[2][4][8];                      //kevin 20161003 Site 順序
extern AnsiString asHandlerVersion;                     //Ifor 20161109 Handler S/W Ver 顯示
extern AnsiString asMainVersion;                        //JerryYang 20180619 (wei) : ASE_CL 尚智要求主畫面版號可以自己修改
extern AnsiString MainVersion;                          //Steven 20250903 : 改成直接抓檔案版本
extern bool bAutoSiteMapWaitTestResult;                 //jou 2016-11-03 JCET 要求Auto Site Mapping 需等待測試結果在繼續入料
extern bool bAutoSiteMapWaitTestPass;                   //jou 2016-11-03 JCET 要求Auto Site Mapping 需等待測試結果在繼續入料
extern bool bAutoSiteMapAmbientResultCheck;             //Ifor 20190528 : add Site Mapping Ambient Check
extern int iOldSelShuttle;                              //Ifor 20161116 add BackUp Shuttle select
extern int iOldShuttleMode;                             //Ifor 20161116 add BackUp Shuttle Mode
extern int iUseSuckModeBackup;                          //Ifor 20161116 add BackUp Use Suck Mode
extern AnsiString asHandlingMode;                       //Ifor 20161117 add BackUp Handling Mode
extern AnsiString asOCRSaveName;                        //wei 20161122 OCR 存圖檔名加入輸入值
extern int iOCRMoveSRead;                               //wei 20161118 OCR S型讀取
extern int iInArmAutoYTeachOffset;                      //kevin 20161124 auto teach Z Yoffset
extern int iOutArmAutoYTeachOffset;                     //kevin 20161124 auto teach Z Yoffset
extern AnsiString asOCRSaveType;                        //Alick 20170119 add OCR 存檔時多加入存檔原因
extern bool bSendATCLotStart;                           //Ifor 20161118 add Send ATC Lot Start
extern bool bSendATCLotEnd;                             //Ifor 20161118 add Send ATC Lot End
extern bool bReSendATCLotEven;                          //Ifor 20161118 add Resent ATC Lot Command
//Sorting Tray define Start
extern const int iSortingTrayAuto1;                     //Ifor 20161123 add Sorting Tray Auto1
extern const int iSortingTrayAuto2;                     //Ifor 20161123 add Sorting Tray Auto2
extern const int iSortingTrayAuto3;                     //Ifor 20161123 add Sorting Tray Auto3
extern const int iSortingTrayFix1;                      //Ifor 20161123 add Sorting Tray Fix1
extern const int iSortingTrayFix2;                      //Ifor 20161123 add Sorting Tray Fix2
extern const int iSortingTrayFix3;                      //Ifor 20161123 add Sorting Tray Fix3      //QQQ
//Sorting Tray define end
extern bool bFix3HasNullICSet;                          //Ifor 20161124 add Fix3 入料時設定HasNullIC
extern bool bAutoShuttleHome;                           //wei 20161206 Auto Shuttle Sensor 回Home
extern double dOldWorkTemp;                             //Ifor 20161220 KYEC 要求Even Log 紀錄上一筆Temp設定
extern double dOldAmbWorkTemp;                          //Ifor 20161220 KYEC 要求Even Log 紀錄上一筆AmbTemp設定
extern double dOldSockTime;                             //Ifor 20161220 KYEC 要求Even Log 紀錄上一筆Sock設定
extern bool bFTPDownLoadHasTestMode;                    //Ifor 20161221 add KYEC Setup File Down Load是否有TestMode.Data
extern bool bAutoCleanFinishOnlyUseRTC;                 //JerryYang 20161216 做完auto clean後不需要做index check
extern bool bCanRunSCKART;                              //Steven 20170919 (wei) : Add for SCK ART
extern int  iSCKARTLoadingStatus;                       //Steven 20161223 : For SCK ART loading count
extern bool bSCKARTLoadCntAlarm;                        //Steven 20161223 : For SCK ART loading count : 數量超過,要放下IC然後Alarm
extern int  iLoadPickX;                                 //Steven 20161223 : For SCK ART loading count : 目前吸到的位置X
extern int  iLoadPickY;                                 //Steven 20161223 : For SCK ART loading count : 目前吸到的位置Y
//extern bool bARTCheckLoaderTray;                        //Steven 20170110 : For SCK ART loading count
extern int  iD43AutoRetryWhenIndexPickErrCnt[2];        //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
extern bool bUseATC_SelfTestFunction;                   //Ifor 20170124 : add KYEC 要求ATC Self Test 可由General.ini 中開啟或關閉
extern AnsiString asATCEvenLotID;                       //Ifor 20170124 (Steven) : add LotID By ATC Even Log
extern double iInitStartDelayDec;                       //kevin 20161214 每次遞減時間
extern int iInitStartDelayTimeDetCount;                 //kevin 20161214 每次遞減計數
extern AnsiString Ase_SendESDTempEPData;                //kevin 20161229 傳送資料給ASE
extern AnsiString Ase_SendESD;                          //kevin 20161229 傳送資料給ASE
extern AnsiString Ase_SendTemp;                         //kevin 20161229 傳送資料給ASE
extern AnsiString Ase_SendEP;                           //kevin 20161229 傳送資料給ASE
extern bool bIndexCheck1;
extern bool bIndexCheck2;                               //kevin 20170120 autoclean index check 發ALARM
//extern int iUseArmNum;                                  //kevin 20170126 0:2支    1: ARM 1   2: ARM2
extern bool bOutArmPickShtHasRetry;                     //Steven 20170202 (wei): For ATJ Out Arm pickup error put to define tray
extern bool bSortArmPickShtHasRetry;                    //RogerYang 20250516 Add for 9046AU
extern bool bAlarmUnlockPassWord;                       //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
extern AnsiString asUnlockPassword;                     //Ifor 20170214 (wei) add 解除Alarm 獨立密碼
extern bool bHT9045S_USE2x4;                            //Ifor 20170308 (wei) add HT9045S 2x4模式鎖定
extern AnsiString dSend_ASEData[10];                    //kevin 20170210 (wei) 送ASE  0:Index Time(Arm),1:Inmdex Cycle Time,2:Test Time,3:UPH
extern bool bManualAutoClean;                           //kevin 20170215 (wei) 手動 autoclean
extern bool bASkStart;                                  //kevin 20170224 (wei) Load Auto Skip 開始有吸取到空格
extern bool bAutoSkipHasIC;                             //kevin 20170224 (wei) LOAD AUTO SKIP HAS IC 需要 ALARM
extern AnsiString sAskStartDetect;                      //kevin 20170224 (wei) LOAD AUTO SKIP HAS IC data
extern bool bSiteHasTurnOn[33];                         //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
extern bool bLoaderTrayAction;                          //wei 20161219 Tray Mapping
extern bool bSecsGemDownloadFTP;                        //wei 20170119 (Steven) DownLoad 沒有馬上按掉會Time Out
extern int iATRFtRtMode;                                //wei 20170119 (jou) ATR FT/RT count
extern int iATRPassCount[2];                            //wei 20170119 (jou) ATR FT/RT count
extern int iATRFailCount[2];                            //wei 20170119 (jou) ATR FT/RT count
extern int iATRTotalCount[2];                           //wei 20170119 (jou) ATR FT/RT count
extern bool bTrayMapDataError[MAX_ARM_Row][MAX_ARM_Col];                    //wei 20170302 (Steven) Tray map data error
extern int iTrayMapDataX[MAX_ARM_Row][MAX_ARM_Col];                         //wei 20170302 (Steven) Tray map data error
extern int iTrayMapDataY[MAX_ARM_Row][MAX_ARM_Col];                         //wei 20170302 (Steven) Tray map data error
extern bool bTrayMapDataCheckError;                     //wei 20170302 (Steven) Tray map data error
extern bool bTrayMapDataCheckErrorSkip;                 //wei 20170302 (Steven) Tray map data error
extern bool bCheckTrayDevice;                           //wei 20170302 (Steven) Tray map data error
extern bool bRealCCDROICheck;                           //wei 20170308 (jou) RTC ROI Check
extern bool bRealCCDROICheckOK;                         //wei 20170308 (jou) RTC ROI Check
extern bool bRealCCDROICountCheck;                      //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
extern bool bRealCCDROICountCheckOK;                    //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
extern bool bAvoidAddDoSiteMappingStep;                 //JerryYang 20170316 (Steven) 避免tester time out時Retry會重複進入DoJCETSiteMappingCHK(), 造成auto site mapping一次跳兩顆
extern int iAse_LoadSkipTime;                           //kevin 20170314 (Steven) Load 開始SKIP 時間記錄
extern DWORD AutoTrayendStartTime[2], AutoTrayendEndTime[2], AutoTrayendPassTime[2];
extern int iAutoTrayendhasIC;                           //kevin 20170314 (Steven) 計算 AUTO SKIP後又吸到IC數量
extern int iAse_LoadTrayEndTotalTime;                   //kevin 20170320 (Steven) LOAD Auto skip 記算總時間
extern int iAutoTrayEndTotal;
extern AnsiString EPuser;                               //kevin 20170328 (Steven) EP 密碼輸入
extern AnsiString EPPass;                               //kevin 20170328 (Steven) EP 密碼輸入

extern bool bInSht1LtcDetectTesterCanMove;              //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
extern bool bInSht2LtcDetectTesterCanMove;              //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
extern bool bReadAndCheckCPUName;                       //Isaac 20170509 (Steven) 卡CPU資訊
extern bool bHingeStart;                                //wei 20170413
extern bool bHingeNeedStart;                            //wei 20170413
extern int iReadAdamEP;                                 //jou 20170413 (Steven) : Read Adam EP 提升UPH
extern bool bFTPDownLoadFilediff;                       //wei 20170518
extern bool bFTPDownLoadFileError;                      //wei 20170518
extern int iInArmTeachZ;                                //kevin 20170502 (wei) auto teach Z Z check pos
extern int ioutArmTeachZ;                               //kevin 20170502 (wei) auto teach Z Z check pos
extern bool bRTCModelNG;                                //wei 20170504 (Steven) RTC Model NG
extern bool bFirstInput;                                //Steven 20170511 (wei) : 使用initial delay當 Soak time
extern bool bFirstInputForIndex;                        //Steven 20170511 (wei) : 使用initial delay當 Soak time
extern bool bOneTimeWait;
extern bool bCheckPCI_MN200StateRun;                    //JerryYang 20170510 (Steven) 改成全域變數
extern bool bF18CheckShuttle1MustHasIC;                 //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
extern bool bF18CheckShuttle2MustHasIC;                 //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
extern bool bReadEpTime;                                //kevin 20170524 (wei) add ep read change time
extern bool bNeedClearSortCount;                        //JerryYang 20170517 (wei) JCET吳如春要求tray end或tray feed後按Start要自動清除unloader數量
extern double dFixTrayDataCleanTime;                    //Ifor 20170525 (wei) add Fix Tray Data Clean Time
extern bool bCutDirectTemp;                             //kevin 20170529 (wei) add power off open heat door Direct Heat close heat
extern int iIndexZ1HomeToZPhaseDistanceFirst;           //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
extern int iIndexZ1HomeToZPhaseDistanceSecond;          //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
extern int iIndexZ2HomeToZPhaseDistanceFirst;           //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
extern int iIndexZ2HomeToZPhaseDistanceSecond;          //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
extern int iIndexZ1HomeToZPosFirst;                     //kevin 20170605 (wei) read pos
extern int iIndexZ1HomeToZPosSecond;                    //kevin 20170605 (wei) read pos
extern int iIndexZ2HomeToZPosFirst;                     //kevin 20170605 (wei) read pos
extern int iIndexZ2HomeToZPosSecond;                    //kevin 20170605 (wei) read pos

//Isaac 20201110 : Index Y find motor phase
//=>
extern int iIndexY1HomeToYPhaseDistanceFirst;
extern int iIndexY1HomeToYPhaseDistanceSecond;
extern int iIndexY2HomeToYPhaseDistanceFirst;
extern int iIndexY2HomeToYPhaseDistanceSecond;
extern int iIndexY1HomeToYPosFirst;
extern int iIndexY1HomeToYPosSecond;
extern int iIndexY2HomeToYPosFirst;
extern int iIndexY2HomeToYPosSecond;
//<=
//Isaac 20201110 : Index Y find motor phase

extern int MotorTask;                                   //kevin 20170515 (wei) define
extern bool bSECSGEM_NoteAlarm;                         //Ifor 20170616 (wei) add S10F3 後跳出Note Alarm 旗標
//extern AnsiString sHandlerSummaryPath;                  //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
//extern AnsiString sHandlerSummaryFileName;              //JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP
extern bool bSECSOneCycleComm;                          //Rogeryang 20170705 (wei) add SECS/GEN OneCycle命令旗標
extern AnsiString aARMSPassWordPath;                    //Ifor 20170621 (wei) add ARMS Function
extern bool bKitOutDiameter;                            //kevin 20170628 (Steven) Lay out Kit  外徑
extern AnsiString sKitOutDiameter[10];                   //kevin 20170628 (Steven) add
extern int iIndexSpeed;                                 //Steven 20170818 : 測試UPH用
extern int iIndexAcc;
extern int  iMonitoringOutlierCnt;                      //Steven 20180626 (wei) : 超過設定值的也要檢查
extern bool bInArmPickErrFromLoader;                    //Steven 20170828 (wei) : 確認是否有在Loader吸料異常
extern bool bQAModeFlag;                                //Steven 20170830 (wei) : QA mode for ATK ART
extern bool bHasQwertyKeyForm;                          //Ifor 20170816 (Steven) add QWERTY小鍵盤旗標
extern bool bHasPasswordForm;                           //Ifor 20170816 (Steven) add QWERTY小鍵盤旗標
extern bool bAlarmAfterPreAlarm;                        //Ifor 20170906 (Steven) add 避免 PreAlarm -> Alarm -> SECS GEM Alarm 同時發生造成當機問題
extern bool bUseNSKitKey;                               //kevin 20170814 (Steven) 低FORCE 需軟體鎖住
extern bool bNeedMusicAndAlarmOn;                       //kevin 20170816 (Steven) add 需發出音樂及秀燈
extern bool bNeedMusicFinishLighAndAlarmOn;             //kevin 20170816 (Steven) add 需發出音樂及秀燈 完成
extern AnsiString ASE_Yield[4];                         //kevin 20170816 (Steven) add 傳送YIELD 給ASE
extern bool bAutoTrayEndHasIC;                          //kevin 20170822 (Steven) auto tray end 有吸到IC
extern AnsiString sAutoTrayendStartPosBuffer;           //kevin 20170823 (Steven) auto tray end 記錄一盤ＸＹ位置
extern AnsiString sAutoTrayendStartPos;                 //kevin 20170823 (Steven) add
extern AnsiString sAutoTrayendEndPosBuffer;             //kevin 20170823 (Steven) auto tray end 記錄一盤ＸＹ位置
extern AnsiString sAutoTrayendEndPos;
extern AnsiString sAutoTrayendabnormalBuffer;           //kevin 20170823 (Steven) auto tray end 中途有料記錄一盤ＸＹ位置
extern AnsiString sAutoTrayendabnormalPos;              //kevin 20170823 (Steven) auto tray end 中途有料 記錄一盤ＸＹ位置
extern bool bASEARTStart;                               //kevin 20170830 (Steven) ASE_KH ART SEND START
extern int iArt_AutoTotal[MAX_AUTO_TRAY][2];                        //kevin 20170830 (Steven) 記錄ART 數量
extern bool bAutoSkiplog;                               //kevin 20170904 (Steven) 記錄SKIP TRAY X Y 位置
extern bool bAutoSkipStartXYlog;                        //kevin 20170904 (Steven) 記錄SKIP TRAY X Y 位置
extern bool bTrayHaveDevice;                            //wei 20170317 (steven) Device Remain 殘料檢測
extern bool bDoTrayDeviceCheck;                         //wei 20170317 (steven) Device Remain 殘料檢測
extern bool bFTPDownLoadFilediff;                       //wei 20170518
extern bool bFTPDownLoadFileError;                      //wei 20170518
extern int iYeildCantactCT[3];                          //wei 20170602
extern int iMapErrorCount;
extern AnsiString asTrayIDFile;
extern bool bNoFileNoStart;
extern bool bDownOffsetFTP;                             //wei 20170821 offset ftp
extern bool bAutoCloseSite;
extern int iContactCountAlarm;                          //wei 20170327 add
extern bool bSaveNeedHome;
extern bool bTrayFeedAfterCleanOut;                     //KaiChen 20180928 ：bTrayFeedAfterCleanOut
extern bool bTCPIPChangeAlarm;                          //Isaac 20170613 (wei) TCP/IP
extern AnsiString InitialStartTime;                     //Isaac 20170613 (wei) TCP/IP
extern int iHandlerCommandServerPort;                   //Isaac 20170613 (wei) TCP/IP
extern int iHandlerResultServerPort;                    //Isaac 20170613 (wei) TCP/IP
extern bool bNoChangeContactHeight;                     //Frank 20170918 (Steven) add Xilinx不覆蓋Contact Height
extern bool bTestEPaddKg;                               //jou 20171026 (wei) : 測試中加壓EP
extern bool bAutoSiteMapHotplateReady;                  //Ifor 20170920 (Steven) : add Auto Site Mapping Hotplate 旗標
extern bool bHP2Inarm;                                  //Ifor 20170926 (Steven) : add Hotplate 資料轉移置InARM
extern bool bAutoSiteMapHotplateSave;                   //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate Save
extern int iAutoSiteMapHotplatePlateR;                  //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate PlateX
extern int iAutoSiteMapHotplatePlateC;                  //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate PlateY
extern int iAutoSiteMapHotplateSource;                  //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate Source
//extern int iAutoSiteMapHotplatePlacePlate;              //Ifor 20171005 (Steven) : add Auto Site Mapping Hotplate 備份最後位置
//extern int iAutoSiteMapHotplatePlacePlateX;             //Ifor 20171005 (Steven) : add Auto Site Mapping Hotplate 備份最後位置
//extern int iAutoSiteMapHotplatePlacePlateY;             //Ifor 20171005 (Steven) : add Auto Site Mapping Hotplate 備份最後位置
//extern int iAutoSiteMapHotplatePickPlate;               //Ifor 20171005 (Steven) : add Auto Site Mapping Hotplate 備份最後位置
//extern int iAutoSiteMapHotplatePickPlateX;              //Ifor 20171005 (Steven) : add Auto Site Mapping Hotplate 備份最後位置
//extern int iAutoSiteMapHotplatePickPlateY;              //Ifor 20171005 (Steven) : add Auto Site Mapping Hotplate 備份最後位置
//extern int iStartAutoSiteMapPickPlate;                  //Ifor 20180529 :add 備份Auto Site Mapping 啟始位置
//extern int iStartAutoSiteMapPickPlateX;                 //Ifor 20180529 :add 備份Auto Site Mapping 啟始位置
//extern int iStartAutoSiteMapPickPlateY;                 //Ifor 20180529 :add 備份Auto Site Mapping 啟始位置
extern int iAutoSiteMapHPToSht;                         //Ifor 20171212 (Steven) : add Auto Site Mapping Hotplate iWhichShuttle
extern int iAutoSiteMapHPToKit;                         //Ifor 20171212 (Steven) : add Auto Site Mapping Hotplate iWhichKit
extern bool bAutoSiteMapHotICCanPick;                   //Ifor 20171225 (Steven) : add 避免 One Cycle 後執行Site Mapping 發生 Hang up
extern int iAutoSiteMapInArmRow;                        //Steven 20220527 : for JCET Auto Site Map
extern int iAutoSiteMapInArmCol;
extern int iAutoSiteMapInArmCol_8;
extern int iAutoSiteMapHPNo;
extern int iAutoSiteMapHPR;
extern int iAutoSiteMapHPC;
extern int iAutoSiteMapSiteNo;
extern int iAutoSiteMapCount;                           //Steven 20220811 : 紀錄目前是哪個Shuttle
extern int iAutoSiteCurrStep;                           //Steven 20220811 : 紀錄目前是哪個Shuttle
extern bool bAutoSiteMapHasPickHP;
extern bool bSiteMappingNeedCheck;                      //Ifor 20190308 : add 隨時開關 Site Mapping
extern bool bSiteMappingPlaceHotplate;                  //Ifor 20190308 : add 隨時開關 Site Mapping
extern bool bARTSeparate;                               //kevin 20170908 (wei) add art 分bin 狀態成立
extern bool bLoadInarmAutoHigh;                         //kevin 20170929 (wei) load initial load 一盤 inarm 校正高度
extern bool bOutArmDropICSkip;                          //kevin 20171005 (wei) add out arm drop IC open door
extern bool bPushHomeDetect;                            //kevin 20171006 (wei) 手動按home
extern bool bManualTrayend;                             //kevin 20171026 (wei) 手縱 tray end
extern int  iSECSMessageCanCloseByOperator;             //Ifor 20171024 (Steven) : add KYEC SECS GEM Can Close By Operator
extern bool bAutoCleanOut;
extern bool bRecordData[10][5];                         //wei 20171012 比對資料紀錄
extern bool bInitStartDelayTime;                        //wei 20171020 (jou) InitStartDelayTime秒數倒數
extern int  iInitStartDelayCount;                       //wei 20171020 (jou) InitStartDelayTime秒數倒數
extern bool bAutocleanDelay;                            //wei 20171020 (jou) Auto clean後，不InitStartDelayTime
extern bool bInitialStart1Time;                         //wei 20171020 (jou) InitialStart1 秒數倒數
extern int  iInitialStart1Count;                        //wei 20171020 (jou) InitialStart1 秒數倒數
extern bool bInitialTestDelayStatus[15];                //wei 20171020 (jou) 延遲狀態顯示     //Steven 20200730 : 10 --> 15
extern int  iInitialStart2Count;                         //kevin 20180905 InitialStart2 秒數倒數
extern bool bInitialStart2Time;                          //kevin 20180905 InitialStart2 秒數倒數
extern bool bPickerMoving;
extern bool bPauseHappen;
extern bool bByBinPercentCompareAlarm;
extern bool bByArmPerSiteCompareAlarm[2];
extern bool bByBinYeildAlarm;                           //wei 20151116 Low Yield Onecycle中不alarm
extern bool bLotCheckOK;
extern bool bCloseSiteCleanout;
extern bool bInitalStartAutoHeight;                     //wei 20171227 Inital Start Auto Height
extern bool bPowersaving;                               //wei 20180301 顯示Powersaving
extern bool bPowersavingStart;
extern bool bSECSGEM_Report;                                                      //Eastsun 20260526 #026-4.A10 Ifor 20240409 add:SECS/GEM Power Report (KYEC only)
extern bool bGPIBPause;
extern bool bGPIBAutoClean;
extern bool bRunSetupStep[10];                          //wei 20180614 TSMC Setup Step
extern bool bCanSetupStep;                              //wei 20180614 TSMC Setup Step
extern int iMRTrayCount[10];                            //wei 20180808 MR 盤數計數
extern int iCassetteFeed;
extern int iCassetteFeedTask;
extern bool bRunQAMode;
extern bool bMRArmZNeedHome[2];
extern bool bMRNeedArmZHome;
extern bool bIsMRArmHome[2];
extern bool bTrayCassetteTrayFeed;
extern bool bLoadCassetteTrayFeed;                      //Sam 20190112 LM
extern bool bUnloadCassetteTrayFeed;                    //Sam 20190112 LM
extern bool bEmptyReadIDOK;
extern bool bNoCheckSafeDoor11;
extern AnsiString asTrayIDData1;
extern AnsiString asTrayIDData2;
extern AnsiString asTrayIDData3;
extern AnsiString asAuto3TrayIDData1;
extern int iLoadIsCoverTray;
extern AnsiString asColorCoverTrayID;
extern AnsiString asTrayIDDataCorverLoader;
extern AnsiString asTrayIDDataLoader2;
extern AnsiString asAutoCoverTrayID1;
extern AnsiString asAutoCoverTrayID2;
extern AnsiString asAutoCoverTrayID3;
extern AnsiString asAutoCoverTrayID4;
extern AnsiString asAutoCoverTrayID5;
extern AnsiString asAutoCoverTrayID6;

extern int iNeedDeviceCassette;
extern bool bNeedDeviceCassette;
extern AnsiString asTrayIDData1;
extern AnsiString asTrayIDData2;
extern bool bManualCatchCassetteDown[5];
extern bool bManualCatchCassetteUp[5];
extern bool bLampMRManualCassette[10];
extern int bLampMRManualMove[2];
extern bool bRTNoUseHotPlate1;                          //20181008 Continuous lot 不使用HP1
extern bool bRTNewLot;                                  //20181008 Continuous lot NEW LOT
extern bool bNewLotHotplate;
extern bool bNewLotUseHotplate1;                        //20181008 New Lot Use Hot plate 1
extern bool bCassetteReadOK;
extern bool bContinuoustest;
extern bool bContinuouslotOnecycle;
extern int iContinuouslotLoader;
extern bool bInitalForm;
extern int iNewLotShuttle;
extern bool bNewLotFullHotPlate;
extern bool bMRNotRT;
extern int iDownNumber;
extern bool bCheckHotPlate2;
extern int iPickPlaceCount;
extern bool bAutoFullTray[3];
extern AnsiString asLoaderLotID;
extern bool bNewLotLoader;
extern AnsiString asLoadPortCassetteID;
extern AnsiString asBuffer6CassetteID;
extern bool bLotDataOK;
extern AnsiString asLoadPortCassetteLotID;
extern AnsiString asLoadPortCassetteData;
//extern AnsiString asBufferCassetteID[10];
//extern AnsiString asBufferLotID[10];
//extern int iBufferDataType[10];
extern bool bPleasePutBuffer10;
extern bool bLackCassette;
extern bool bCatchArmNoCassette;
extern bool bLoadPortNoCassette;
extern bool bStackedTrayNoTray;
extern bool bTrayBracketNoTray;
extern bool bRFIDReadFirst;
extern bool bRFIDReadFirst1;
extern bool bRFIDReadFinish;
extern bool bMRAutoLotStart;
extern bool bMRAutoFTLotStart;
extern bool bMRAutoMode;
extern bool bMRAutoCassetteOut;
extern int  iMRAutoCassette;
extern bool bS10F5show;
extern AnsiString asSameLotID;
extern int iLotCount;
extern bool bAuto2ToEmpty;
extern bool bNeedResetAlarm;
extern bool bATRStartTime;
extern int  iATRStartCount;
extern AnsiString asNextLotID;
extern AnsiString asNowLotID;
extern bool bSearchNowLotID;
extern bool bSearchNextLotID;
extern bool bCheckCassetteFinish;
extern bool bWaitStartDelay;
extern int iWaitStartDelayTask;
extern bool bFTStartTime;
extern int  iFTStartCount;
extern AnsiString asMRLotData[3][4];
extern int iMRLotCount;
extern bool bFTOpenSite[4][8];
extern bool bOutarmAutoHigh;                            //kevin 20171115 (wei) load initial load 一盤 outarm 校正高度Z軸先不要下去吸
extern bool bSaveNeedHome;
extern bool bEnable_KLT_Function;                       //Ifor 20180517 : add BarCode 編碼規則 0:民國 1: 西元   //Ifor 20180802 : add KLT bBarCodeRules -> bEnable_KLT_Function 0:KYEC 1: KLT
extern int iXpitchMax;                                  //Isaac 20171128: X-Pitch 40mm擴大至50mm
extern int iXpitchMaxX2;                                //Isaac 20171204 (Steven) : X-Pitch 40mm擴大至50mm
extern int iXpitchMaxX3;                                //Isaac 20171204 (Steven) : X-Pitch 40mm擴大至50mm
extern int iXpitchMaxX4;
extern int iXpitchMaxX5;
extern int iXpitchMaxX6;
extern int iXpitchMaxX7;
extern int iXpitchMin;                                  //Steven 20180522 : X-Pitch最小值
extern int iXpitchMinX2;                                //Steven 20180522 : X-Pitch最小值
extern int iXpitchMinX3;                                //Steven 20180522 : X-Pitch最小值
extern int iXpitchMinX4;
extern int iXpitchMinX5;
extern int iXpitchMinX6;
extern int iXpitchMinX7;
extern int iXpitchMaxX1_MM;
extern int iXpitchMaxX2_MM;
extern int iXpitchMaxX3_MM;
extern int iXpitchMaxX4_MM;
extern int iXpitchMaxX5_MM;
extern int iXpitchMaxX6_MM;
extern int iXpitchMaxX7_MM;
extern int iXpitchMinX1_MM;
extern int iXpitchMinX2_MM;
extern int iXpitchMinX3_MM;
extern int iXpitchMinX4_MM;
extern int iXpitchMinX5_MM;
extern int iXpitchMinX6_MM;
extern int iXpitchMinX7_MM;
extern int iPitch_Max_minus_Min;                        //Isaac 20171204 (Steven) : Xpitch40->50mm, 新增150mm的Teaching點位，內差法公式計算
extern int iPitchY_Max_minus_Min;                       //Jimmychiu 20221205 add y pitch minus
//------------------------------------------------------------------------------
extern bool bNeedWaitRTCAutoVerify;                                             //jou 2014-06-24 RTC 自動進行Model驗證
extern bool bIndexPickUpErrorWaitRetry;                                         //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
extern bool bIndexPickErrShtStayRight1;                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
extern bool bIndexPickErrShtStayRight2;                                         //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
extern bool bFirstTimeEnableSocketSensor;                                       //Ifor 20171121 (Steven) : add JCET 如春要求第一次啟動時須強制開啟Socket Sensor 功能
extern bool bInArmPlaceToShuttleFinish;                                         //Ifor 20171121 : add iWhichShuttle & iWhichKit 切換旗標 避免沒切換狀態造成Hangup
extern int iInArmPutIcToSH[2][2];                                               //Ifor 20171121 : Test 查看異常資料
extern AnsiString asMotorDatabaseErr;                                           //jou 20180814 (Steven) : 增加Motor database 異常警示
extern bool bAllSameHotCountHotOK;                                              //Ifor 20171128 :add
//------------------------------------------------------------------------------
extern int iInArmSpeed1;        //KaiChen 20171225 (Steven)：Add Speed Display
extern int iOutArmSpeed1;       //KaiChen 20171225 (Steven)：Add Speed Display
extern int iInShuttleSpeed1;    //KaiChen 20171225 (Steven)：Add Speed Display
extern int iOutShuttleSpeed1;   //KaiChen 20171225 (Steven)：Add Speed Display
extern int iIndexSpeed1;        //KaiChen 20171225 (Steven)：Add Speed Display
//------------------------------------------------------------------------------
extern int iMagneticScalePos[16][1000];

extern int iLogLightScaleCount_InArmX1;     //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_InArmX2;     //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_InArmY1;     //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_InArmY2;     //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_OutArmX1;    //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_OutArmX2;    //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_OutArmY1;    //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern int iLogLightScaleCount_OutArmY2;    //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern bool bLogLightScale_InArm;           //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern bool bLogLightScale_OutArm;          //KaiChen 20171228 (Steven) ：Log Light Scale Data
extern bool bWaitSecsGemReply;              //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
extern bool bNeedAUTHORITYCheck;            //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
extern int iShowAUTHORITY;                  //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
extern AnsiString strShowAUTHORITY;         //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
extern AnsiString strInRotateDutAngle;      //wei 20180316
extern AnsiString strOutRotateDutAngle;     //wei 20180316
extern int iInArmType;                      //Ifor 20180518 : add 整合Hot plate 資料轉換
extern int iInHPType;
extern bool bRunAutoSiteMapping;            //Ifor 20180518 : add 簡化Site Mapping 旗標
extern AnsiString asGETPFCPARAMETER;
extern AnsiString asALLGETPFCPARAMETER[14][20];
extern bool bSETPFCPARAMETER;
extern AnsiString asGETSLOPEOFFSET;
extern AnsiString asALLGETSLOPEOFFSET[2][8];
extern AnsiString asALLSETSLOPEOFFSET[2][8];
extern bool bSETSLOPEOFFSET;

extern int iYieldSiteCount[2];              //wei 20180606 Interval Low Yield By Site
extern bool bYieldSiteBin[2];               //wei 20180606 Interval Low Yield By Site
extern int iYieldSiteBinpass[2][4][8];      //wei 20180606 Interval Low Yield By Site
extern bool bYieldSiteBinCheck;             //wei 20180606 Interval Low Yield By Site
extern bool bIntervalYieldIsPass[2][4][8][200];  //wei 20180606 Interval Low Yield By Site 20180629
extern int iYieldTotalCount;                //wei 20180718 Interval Low Yield By Total
extern bool bYieldTotalBinIsPass[300];      //wei 20180718 Interval Low Yield By Total
extern bool bYieldTotalBin;                 //wei 20180718 Interval Low Yield By Total
extern int iYieldTotalBinpass;              //wei 20180718 Interval Low Yield By Total

extern int iAutoCleanAlarm;                 //Ifor 20180727 (wei) ：Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
extern bool bStartModeComplete;             //Sam 20190429 : Add CC_PTI_NEWWORK
extern int iHasReadeLastData;               //Ifor 20180802 : add 重開程式後Loader有Tray 且開啟Auto SPIK 強制手動取Tray
//extern AnsiString strATCBackupErrCode;      //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm    //Ifor 20191209 取消ATC溫度異常Index上升功能
//extern AnsiString strATCBackupErrPart;      //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm    //Ifor 20191209 取消ATC溫度異常Index上升功能
//extern int iATCBackupErrKCode;              //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm    //Ifor 20191209 取消ATC溫度異常Index上升功能
//extern int iATCBackupErrPos;                //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm    //Ifor 20191209 取消ATC溫度異常Index上升功能
//extern bool bATCBackupDuplicateErr;         //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm    //Ifor 20191209 取消ATC溫度異常Index上升功能
extern bool bZ1ModifyDistanceRef;           //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
extern bool bZ2ModifyDistanceRef;           //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
extern int iZ1ModifyDistanceRef;            //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
extern int iZ2ModifyDistanceRef;            //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height

//Isaac 20201110 : Index Y find motor phase
//=>
extern bool bY1ModifyDistanceRef;
extern bool bY2ModifyDistanceRef;
extern int iY1ModifyDistanceRef;
extern int iY2ModifyDistanceRef;
//<=
//Isaac 20201110 : Index Y find motor phase

//Ifor 20190912 :add 海思 V02.30 版 Record Torque
extern int iGPIBIndexStatus;
//==>
extern bool bFrontTestArmTorqueFinish;
extern bool bRearTestArmTorqueFinish;
extern bool bArmTestInfoEvenLogStart_KYEC;
extern bool bArmTestInfoEvenLogEnd_KYEC;
extern AnsiString sArmTestInfoEvenLogFile;
//<==
//Ifor 20190912 :add 海思 V02.30 版 Record Torque
extern AnsiString asChangeLogByLotPath;     //Ifor 20191002 : add Change Log By Lot
extern AnsiString sChangeEvenLogFile;       //Ifor 20191002 : add Change Log By Lot

extern bool bDropRetry;
extern AnsiString asTCPIPTemperature;
extern AnsiString asTCPIPBarCode[32];
//------------------------------------------------------------------------------
extern bool bOutArmPlaceDevice[4][8];                   //kevin 20180119 (Steven) Add outarm place ic not home
extern AnsiString sBinData[eTrayCount];                 //JerryYang 20220909 : 10->eTrayCount        //kevin 20180202 (Steven) record bin
extern int iContractModeTest;                           //kevin 20180222 (Steven) contract mode 手動送測試訊號
extern bool bContractModeTest;                          //kevin 20180222 (Steven) contract mode 秀手動送測試訊號
extern bool bLoadContractModeTest;                      //kevin 20180222 (Steven) contract mode Load 不入tray
extern int  iAddInitStartDelayCT;                       //kevin 20180307 add delay count InitStartDelayTime
extern bool bFinishInitStartDelay;                      //kevin 20180308 add 動作完成
extern bool bCheckInarmPlaceShuttle;                    //kevin 20180315 add inarm place shuttle 確認是否有抖動情形
extern double queue20[20];                              //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
extern bool bResetflag;                                 //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
extern bool bZ1Isdownflag;                              //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
extern bool bZ2Isdownflag;                              //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
extern int iLoaderNum;                                  //Frank 20181121 add iLoaderNum
extern bool bIndexDropIC[2];                            //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
extern int iInArmAutoZHeight;                           //Frank 20171213 (Steven) : Auto get Height
extern int iOutArmAutoZHeight;                          //Frank 20171213 (Steven) : Auto get Height
extern AnsiString sPassWord;                            //kevin 20180411 password
extern bool bShowShuttle1Device;                        //kevin 20180504 index pick up error
extern bool bShowShuttle2Device;                        //kevin 20180504 index pick up error
extern bool bSetDataChange;                             //kevin 20180606 (wei) add SEM GEM send ESD DATA
extern AnsiString sVersion;                             //JerryYang 20180619 (wei) : ASE_CL 尚智要求主畫面版號可以自己修改
extern AnsiString sHisiWorkName[6];                     //kevin 20180626 20170129 海思工作檔命名
extern AnsiString sATCFileName;                         //kevin 20180626 add ATC FILENAME
extern int iByBinTotal[256];                            //kevin 20180703 bin 數量       Bin total[0]
extern AnsiString BinAssign[eTrayCount];                //JerryYang 20220909 : 10->eTrayCount        //kevin 20180705 Auto tray put bin
extern AnsiString sSVBinAssign[eTrayCount];             //JerryYang 20250224 : add
extern bool bUnloadHasBin[eTrayCount];                  //JerryYang 20220909 : 10->eTrayCount        //kevin 20180705
extern int iTrayLastBin[eTrayCount];                    //JerryYang 20220909 : 10->eTrayCount        //kevin 20180705 每一個TRAY最後一個BIN
extern int iRecordCatchTray;                            //kevin 20180711 display catch tray
extern int iallSitCount;                                //kevin 20180720 add
extern bool bSetDataChange;                             //kevin 20180606 add SEM GEM send ESD DATA
extern AnsiString PC_NAME;                              //kevin 20180803 pc NAME
const int qTaskCount=300;
extern TMyQueue10 QueueTaskList[qTaskCount];            //Steven 20180808 (wei) : 修改紀錄Task的方式
extern TMyStrQueue100 QueueGalilCmd;                    //Steven 20200320 : 紀錄Galil命令
extern TMyTimerQueue100 QueueTestTime;                  //Steven 20200714 : 新的紀錄測試時間方式
extern TMyTimerQueue100 QueueCycleTime;                 //Steven 20200714 : 新的紀錄測試時間方式
extern TMyTimerQueue100 QueueAirOnTime1;                //Sam 20220329 : Record Index Air On Time
extern TMyTimerQueue100 QueueAirOnTime2;                //Sam 20220329 : Record Index Air On Time
extern TQPF_Timer tTestTimer;
extern TQPF_Timer tCycleTimer;
extern AnsiString StartTestTime;
extern AnsiString EndTestTime;
extern AnsiString StartCycleTime;
extern AnsiString EndCycleTime;
extern bool bUseInitTempOffset;                         //Steven 20180820 : 簡化init temp offset判斷式
extern bool bUseInitDelay;                              //Steven 20180820 : 簡化init temp offset判斷式
extern bool bYpitchNeddHome;                            //kevin 20180823
extern bool bUseDefineVer;                              //kevin 20180829 addUSE DEFIN VER
extern AnsiString SUseDefineVer;                        //kevin 20180829 addUSE DEFIN VER
extern long double iContractCount;                      //kevin 20180928 add contract count
extern bool bEOTToLongStopBlowAir;                      //kevin 20181009 add上次測試訊號太久 需停止吹氣
extern bool bSOTToLongStopBlowAir;                         //kevin 20181009 add上次測試訊號太久 需停止吹氣
extern bool bNoAutoZSelect;                                //kevin 20181023 (Steven) : 不使用2段分tray氣缸
extern bool bTestStartToNextTestStartDelay;                               //kevin 20181101 SOT 間隔時間驅動
extern bool bIndexAlarmInArmAway;                          //kevin 20181102 (Steven) :  index掉料 inarm 讓位 inarm先不要動
extern bool bTestStartToNextTestStart;                               //kevin 20181101 SOT 間隔時間驅動
extern AnsiString sBufferSOT;                           //wei 20181211 (Steven) : 更換位置SOT
extern AnsiString sBufferEOT;                           //wei 20181211 (Steven) : 更換位置SOT
extern bool bRTCAutoVerifyControlEP;                    //jou 2014-06-24 RTC 自動進行Model驗證
extern bool bNeedWaitRTCAutoVerify;                     //jou 2014-06-24 RTC 自動進行Model驗證
extern bool bNeedWaitContactTestAutoVerify;             //JerryYang 20210420 : contact test執行auto verify旗標
extern bool bDoFRTCAutoModelVerify;                     //jou 2014-06-24 RTC 自動進行Model驗證
extern bool bDoBRTCAutoModelVerify;                     //jou 2014-06-24 RTC 自動進行Model驗證
//extern bool bResetFirstTestToErr;                       //jou 20180103 : Reset第一個測試完的IC要丟到Error bin
extern bool bIndexArm1PickUpErrNeedPiggyback;           //jou 20180814 : Index pick up error need piggyback
extern bool bIndexArm2PickUpErrNeedPiggyback;           //jou 20180814 : Index pick up error need piggyback
extern bool bUseFTPOneCycle;                            //Sam 20200305 : 增加 FTP 檔案檢查，檢查到 VIE_STOP.txt 就執行 OneCycle
extern AnsiString sGalilData[100];                      //JerryYang 20181130 (Steven) : add galil command log
extern AnsiString sLMData[10];                          //JerryYang 20181130 (Steven) : add galil command log
extern int iAuto_CarCarDeviceCnt[MAX_AUTO_TRAY];        //Sam20250331 : Unloader Auto 退出時上面的 IC 數量。
extern int iSECS_Loader_Buffer_State;                   //Sam 20250605 : Loader buffer status change event
typedef struct
{
    int XItem;
    int YItem;
    int Cnt;
    void SetData(int X, int Y)
    {
        XItem=X;
        YItem=Y;
        Cnt=X*Y;
    };
}MySiteData;

extern MySiteData SiteData[TotalTestMode];              //JerryYang 20181011 (Steven) : SiteData改成全域變數

typedef struct                                                                  //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
{
    //異常分三種，1. 第一次Skip error；
    //            2. 連續兩次或以上Skip error；
    //            3. 本次無Skip error但上一次有；
    int iManualRemoveTrayCnt;
    bool b2ndUpSkip;
    void Clear()
    {
        iManualRemoveTrayCnt=0;
        b2ndUpSkip=false;
    }
    void SetTakePickUpError()
    {
        if(iManualRemoveTrayCnt>0)
        {
            iManualRemoveTrayCnt=2;
            b2ndUpSkip=true;
        }
        else
        {
            iManualRemoveTrayCnt=2;
            b2ndUpSkip=false;
        }
    };
    void DoRemoveTray()
    {
        iManualRemoveTrayCnt--;
    }
    int GetTakePickUpError()
    {
        if(iManualRemoveTrayCnt==2)
        {
            if(b2ndUpSkip==true)
            {
                return 1;            //Check whether cookies on the Tray and under the Tray
            }
            else
            {
                return 0;            //Check whether cookies on the Tray
            }
        }
        return 2;                    //Check whether cookies under the Tray
    }
}MyNextTrayNeedManualRemoveTray;
extern MyNextTrayNeedManualRemoveTray MaualRemoveTray3Alarm;                    //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error

extern bool bWaitPreciserFinish;                        //Ifor 20180703 (Steven) : : 等待 Preciser 完成
//Steven 20181228 : Add Index Action
//==>
extern int Z1DownToShuttle;                             //ChungHung 20171116 modify for Index Action
extern int Z2DownToShuttle;                             //ChungHung 20171116 modify for Index Action
extern bool bZ1UpAndPlaceZ2Down;                        //ChungHung 20171116 modify for Index Action
extern bool bZ1DownZ2UpAndPlace;                        //ChungHung 20171116 modify for Index Action
//<==
//Steven 20181228 : Add Index Action
extern int iAutoSiteMapRunStartMode;                    //Ifor 20190916 :add Auto Site Mapping 備份Start Mode
extern int iIndexTakeDeviceChk1,iIndexTakeDeviceChk2;           //kevin 20190103 回吸檢測狀態
extern int ishuttleShake;                                       //kevin 20190107 add shuttle shake state
extern int iIndexArmWhich;                                      //kevin 20190130 add use test arm
extern bool bHandlerPause;                                      //Steven 20190123 : 紀錄Handler被暫停
extern int iHandlerStartCount;
//JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
//==>
extern bool bArm1SuckComplete;
extern bool bArm2SuckComplete;
extern bool bArm1NeedSuck;
extern bool bArm2NeedSuck;
extern bool bArm1DuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL];
extern bool bArm2DuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL];
extern bool bDropAtSht1NeedCheckVac[4][8];
extern bool bDropAtSht2NeedCheckVac[4][8];

extern bool bSHT1_DropPosHasIC[4][8];
extern bool bSHT2_DropPosHasIC[4][8];

extern bool bSht1OnlyHasICErr;
extern bool bSht2OnlyHasICErr;
extern bool bD44Arm1CheckVacOn;
extern bool bD44Arm2CheckVacOn;
extern TQPF_Timer DoFrontTestDestroyICDelay;
extern TQPF_Timer DoRearTestDestroyICDelay;
extern bool bArm1D44SuckCheck;
extern bool bArm2D44SuckCheck;
//<==
//JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
extern int iPickerOrder;
extern bool bIndexplaceCheck;                                //kevin 20190201 index 回溪檢測中
extern bool bSkipFlagSH1;       //Isacc (Steven) 20180329 2DID All site fail recheck twice and alarm
extern bool bSkipFlagSH2;       //Isacc (Steven) 20180329 2DID All site fail recheck twice and alarm
extern double iTotalGf;         //Steven 20190314 : Change total gf to globe var
extern bool bContractModeCheckPushZ1;                           //kevin 20190227 add 前檢查是否有CI放在COCKET 造成機構損壞 按z1 確認
extern bool bContactModeNeedOpenDoor;                           //JerryYang 20231218 : G22提醒人員取tray功能
extern bool AsebLoadCellTest;                                   //kevin 20190305 add one cycle run arm 1 arm 2 load cell test
extern bool bLoadCellTest;                                      //kevin 20190305 add one cycle run arm 1 arm 2 load cell test
extern bool bUseTwoArm32Site;                                   //kevin 20190322 add two arm tester 32SITE 16SITE(4X4)
extern bool bQASampleCnt;                                       //Steven 20190326 : QA Sampling

extern double  dATCTempAdjustmentOffset[32];            //Ifor 20190215 : add ATC 使用 三點校正功能
extern bool bATCTempAdjustmentOffset;                   //Ifor 20190215 : add ATC 使用 三點校正功能
extern bool bTJControlMode;                             //Ifor 20190328 : add TJ Temp Over Range
extern bool bCheckGiveWay;                              //Ifor 20190328 : add TJ Temp Over Range
extern int iBackupTestY2_Middle;
extern int iBackupTestZ2_Test;
extern int iBackupTestZ2_Drop;
extern AnsiString sATCPath;
extern bool bIndexDropVacuumError;                              //kevin 20190418 避免 inarm 來回跑
//extern AnsiString asViewList[30];
//extern AnsiString asViewListOld[30];
//extern AnsiString asViewMessage[5];
//extern AnsiString asViewMessageOld[5];
extern bool bViewCom;
extern int iTeachCount;
extern int iMRStartMode;
extern int iNeedATR;
extern bool bLastTray;
extern bool bSendFTFinish;
extern int iNeedATRSet;
extern bool bFTCloseSite;
extern bool bRTCloseSite[4][8];
extern bool bRTOpenSite;
extern AnsiString asRTCloseSiteMessage;
extern int iLotQuantity;
extern int iStepRunFunction;
extern bool bRT2flag;
extern bool bDiecountfailFlag;
extern AnsiString asLotQuantity;
extern AnsiString asPreSendID;
extern bool bRTNoRunTrayID;
extern bool bPlaceToBufferCheck;
extern int iAutoCount[3];
extern AnsiString TotalErrPart;                                 //kevin 20190610 add TotalErrPart
extern bool bWaitTSV;                                           //Steven 20190521 : ATK lot count
extern bool bUseHotGunCheck;                                    //kevin 20190621 hot gun 流量 不足偵測
extern bool bPickLoaderDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                      //Steven 20190704 : bSuckDuplicateErr分類並改成全域變數
extern bool bPickHPDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                          //Steven 20190704 : bSuckDuplicateErr分類並改成全域變數
extern bool bTryPickHPDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                       //Steven 20190704 : bSuckDuplicateErr分類並改成全域變數
extern int  iLoadPitchStepY;                                                    //Steven 20190704 : pitchstepY改成全域變數
extern bool bAPAXConnectFileAlarm;              //ChungHung 20180912 add for APAX Alarm
extern bool bApaxWriteFinish;
extern bool bApaxReadFinish;
extern int iAPAXEPValue[16];           //Nickliu 20180827 Add APAX Dll Connection
extern int iAPAXDualEPValue[16];                                                //Ifor 20250319 add:Mutil EP Control
extern bool bTakeAway;                       //kevin 20190806
extern AnsiString sLoadCellReceData;           //kevin 20190906 add load cell NG
extern bool bloadcellRece;                  //kevin 20190906 add load cell command
extern int iloadcellRece;                  //kevin 20190906 add load cell command
extern int iloadcellCount;                       //kevin 20190906 add load cell command
extern bool HotGunFlowEnable;                           //KaiChen 20190729 ：Hot Gun Flow
extern unsigned int HotGunFlow_LineNo;                  //KaiChen 20190729 ：Hot Gun Flow
extern unsigned int HotGunFlow_DevNo;                   //KaiChen 20190729 ：Hot Gun Flow
extern unsigned int HotGunFlow_Gun1_ChannelNo;          //KaiChen 20190729 ：Hot Gun Flow
extern unsigned int HotGunFlow_Gun2_ChannelNo;          //KaiChen 20190729 ：Hot Gun Flow
extern bool bUseHotGunFlowCheck;                                //KaiChen 20190729 ：Hot Gun Flow
extern AnsiString sTempsite[64];                   //kevin 20190928 add 溫度error 秀site編號
extern int iOneCycleTask;                               //JerryYang 20190925 one cycle task log
extern int iCleanOutCycleTask;                          //JerryYang 20190925 clean out task log
extern bool bLoaderNeedVibrate;  //JerryYang 20191001 loader震動馬達
//JerryYang 20180921 Setup Teach功能
//<==
extern bool bInArmSetupTeach[InOfsTotal];
extern bool bInArmStop[InOfsTotal];
extern int  iInArmPickPlaceCnt[InOfsTotal];
extern bool bOutArmSetupTeach[OutOfsTotal];
extern bool bOutArmStop[OutOfsTotal];
extern int  iOutArmPickPlaceCnt[OutOfsTotal];    //Ifor 20200825 Fix:記憶體破壞 8->12
extern int  iSortUnloadT6;
//<==
//JerryYang 20180921 Setup Teach功能
extern int iShuttleLog;                 //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
extern bool bDoBRTCGiveWayCheck;        //Ifor 20191120 : add 新增RTC2.0 Full Check 讓位
extern AnsiString sHiLotID;             //kevin 20191016 客戶  LOT OSAT
extern AnsiString sOSATLotID;           //kevin 20191016 客戶  LOT OSAT
extern AnsiString sTemp;                //kevin 20191016 工作溫度
extern AnsiString sHandleID;            //kevin 20191016 機台編號
extern AnsiString sFlow;                //kevin 20191016 FTRT
extern AnsiString sDeviceType;          //kevin 20191016 產品名稱
extern AnsiString sInsertion;           //kevin 20191016 Process  FT1 FT2.
extern AnsiString sTestProgram;         //kevin 20191016 測試程式
extern AnsiString sOSATName;            //Ifor 20200724 add:測試廠名稱
extern bool bPickUpHomeFinish;
extern bool bOneCycleInArmToLoader;
extern int iWhichArmDown;               //JerryYang 20200316 add SVID 哪支arm下壓在測區
extern bool bCheckTrayBySoftWareOpen;   //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
extern bool bCheckTrayBySoftWareOpen_Sen[MAX_FIX_TRAY];    //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
extern double dfComplianceUnit;         //kevin 20200313 add 浮動頭對應缸徑
extern bool bCanAutoCloseSite;          //Steven 20200420 : 整合Auto Site Off
extern int USE_Scanner_AOI_Inspection;          //Ifor 20190725 : add Scanner AOI
extern int USE_Scanner_AOI_Inspection_FixLight_Z_Axis; //Eastsun 20260410 : AOI Fix Light Z Axis Mode
extern bool bAOIBallDamageCounterFailAlarm;     //Ifor 20190725 : add Scanner AOI
extern bool bAOIBallDamageCounterFailUnTest;    //Ifor 20190725 : add Scanner AOI
extern bool bPickSH1Flag;                       //Ifor 20191226 移至全域
extern bool bPickSH2Flag;                       //Ifor 20191226 移至全域
extern int iSLT_HeadContactCount[2][16];                        //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
extern AnsiString asSLT_LotStartTime;                           //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
extern AnsiString asSLT_LotEndTime;                             //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
extern int iDBQueryDays;                                        //Ifor 20200324 : add KYEC 要求新增DB Qery 天數
extern int iLDTrayNeedManualRemoveTray;                         //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray 0:Normal 1:Edit Loader Tray 2:Data Change
extern bool bNeedManualRemoveTray;                              //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
extern bool bMOFileControlBinChangeAlarm;                         //Sam 20200525 : Control Bin
extern bool bNewCatchTrayblock;                                 //kevin 20200512 夾tray遮版削短
extern bool bHandlerModel;  //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
extern bool bHasICinSocket;
extern AnsiString sSocketSensorErr;
extern bool bTryPickLastRowIng; //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
extern int iBackupLoadDir;
extern bool bSht1LoseICErr;     //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm
extern bool bSht2LoseICErr;
extern int i3SigmaTempMonitior_NowContactCount; //Hmy 20200515 Copy//Hmy 20200510 Add Enable 3 Sigma Temp Monitor//Hmy 20200510 Add Enable 3 Sigma Temp Monitor
extern int iReceiveATCData;                      //kevin 20200608 讀取ATC 參數資料
extern bool bD52IndexArmUp;     //JerryYang 20200804 : fix D52 & Index arm在shuttle高度預熱功能同時啟用時，發生tester timeout時會誤發handler hang up
extern AnsiString asEESUG_Data[7];                              //Ifor 20200529 add: SECS/GEM EESUG Offset Function
extern bool bBTestSuckHasError;                                 //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
extern bool bBTestSuckError[MAX_ARM_Row][MAX_ARM_Col];          //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
extern bool bFTestSuckHasError;                                 //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
extern bool bFTestSuckError[MAX_ARM_Row][MAX_ARM_Col];          //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
extern AnsiString asSetupFileCheckList;   //Ifor 20200914 add:Setup File Check List
extern int USE_Top_Scanner_AOI_Inspection;      //Ifor 20200902 add: TFAMD Top AOI
extern bool bArm1Delay;
extern bool bArm2Delay;
extern bool bArm1IsTest;  //JerryYang 20180629 (wei) : 用來判斷是否在測試中
extern bool bArm2IsTest;  //JerryYang 20180629 (wei) : 用來判斷是否在測試中
extern bool bIndexCheckVacum;     //kevin 20200716 add ASE_KH 卡輸入
extern bool bUseStartSoundAlarm;                 //kevin 20201116  Start 發出聲音 不動 5sec
extern bool bStartMoveSpeed;                     //kevin 20201116  motor move speed 10 % 移鄧10 sec 系統暫停 恢復速度
extern bool bStartOpenDoor;                      //kevin 20201116  有開門停機
extern bool bTesterSendPause;   //Steven 20201022 : For RFMD
extern bool bNeedReplunge_RFMD; //Steven 20201022 : For RFMD
extern bool bTesterPauseMusic;   //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
extern bool bPauseAlarmDelayActive;          //RogerYang 20260626 : Tester Pause 逾時計時啟用旗標    //AI(W906-GB-P2c) 20260926: golden 912 cmydef.h:5006 (P2c, 使用者 0926 14:3x「906 為底補 912」)
extern TQPF_Timer hPauseAlarmDelay;          //RogerYang 20260626 : Tester Pause 逾時(=MaxTestTime)計時器    //AI(W906-GB-P2c) 20260926: golden 912 cmydef.h:5007

//Steven 20201022 : For RFMD Empty Socket Check Funstion
//==>
extern bool bDoEmptySocketCheck;
extern int  iESC_IndexContactCount;
extern int  iGetESCResult;
extern bool bDoEmptySocketOneCycle;
//<==
//Steven 20201022 : For RFMD Empty Socket Check Funstion.

extern int iSecsGemSwitchFTRT;      //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功
extern int iAutoSiteMappingErrCT;   //jou 20200707 : VTEST auto site mapping
//Steven 20210217 : 改成全域
//==>
extern bool fRearNeedSuckIC, fFrontNeedSuck, fFrontNeedDestroy;
extern bool fFrontNeedSuckIC, fRearNeedSuck, fRearNeedDestroy;
extern bool f32SiteNeedDestroy, f32SiteNeedSuck;
//<==
//Steven 20210217 : 改成全域
extern int JCET_FOR_EVAN;                //Steven 20210304 : 長電給如春的版本
extern int SPIL_FOR_QLE;                 //Steven 20230110 : For渠梁
//AI(ht9045-v899) 20260417: add config flag to show AutoDeviceEjection panel
extern int bAutoDeviceEjection;
extern bool bAfterHomeShtChkLoseICNeedSlow[2];//Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
extern bool bShowIndexMotorError;       //Isaac 20201012 : encoder和command/Teaching點比較，show alarm視窗變大
extern int iMaxCommandY1,iMinCommandY1,iMaxCommandY2,iMinCommandY2,iMaxTeachY1F,iMinTeachY1F,iMaxTeachY1M,iMinTeachY1M,iMaxTeachY2M,iMinTeachY2M,iMaxTeachY2R,iMinTeachY2R; //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值
extern bool bOverRangeDoTMode;          //Isaac 20201012 : index Y超過範圍，做一次Tmode
extern bool bOverRange4Indexhome;       //Isaac 20201012 : index Y超過範圍，做一次Tmode
extern bool bTriger4Indexhome;          //Isaac 20201012 : index Y超過範圍，做一次Tmode
extern int iIndexOverRangeCount;        //Isaac 20201012 : index Y超過範圍，做一次Tmode
extern int iOutShuttle1HasICErrRetryCnt; //Sam 20210409 : 改為全域
extern int iOutShuttle2HasICErrRetryCnt; //Sam 20210409 : 改為全域
extern bool bIn_ICRotationCompleteOnKit;   //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
extern bool bOut_ICRotationCompleteOnKit;  //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
extern bool bIn_XYMoveFinishOnRotationKit;   //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
extern bool bOut_XYMoveFinishOnRotationKit;
extern bool bLoadNewEmptyTrayToCarStart;    //Sam 20211119 : 放 Tray 增加保護
extern bool bUnLoadNewEmptyToStackStart;    //Sam 20211119 : 放 Tray 增加保護
extern bool bLoadNewColorTrayToCarStart;    //Sam 20211119 : 放 Tray 增加保護
extern bool bUnLoadNewColorToStackStart;    //Sam 20211119 : 放 Tray 增加保護
//Isaac 20201110 : Index Y find motor phase
//=>
extern int iIndexY1FindPhaseFrontPos;
extern int iIndexY1FindPhaseMiddlePos;
extern int iIndexY2FindPhaseMiddlePos;
extern int iIndexY2FindPhaseRearPos;
//<=
//Isaac 20201110 : Index Y find motor phase
extern int iTempOverShowAlarmT[tcTotalCount];    //kevin 20210112 溫度過低過高 過高  1 過低  :2
extern bool iAutoZXYMove;                        //kevin 20210223 Auto Teach X Y offset
extern bool bLoadAutoTeachInarm;                 //kevin 20210305 add inarm Auto teach move  X Y PITCH Z 軸不動
extern bool bCleanOutFinish;                     //kevin 20210406 clean out finish
extern bool beKeepCloseSite;                     //kevin 20210406 clean out open close Site
extern bool bRetryReadToqu;                      //kevin 20210419 重讀扭力
extern AnsiString  sSetTorquValue[2];            //kevin 20210421 扭力設定讀取值
extern int iSetTorqueAlarm[2];                      //kevin 20210505 扭力連續幾次ALARM
extern int iIndEPCnt;   //JerryYang 20210413 : 獨立EP數量
//extern int  iTrayXAutoSitemapping[4][8];    //Ifor 20210524 add:mykitsuck移至cmydef
//extern int  iTrayYAutoSitemapping[4][8];    //Ifor 20210524 add:mykitsuck移至cmydef
//extern TStringList *ansiFile;  //Steven 20210506 : 紀錄New出來的元件
//extern TList       *listPtr;
extern void LogSoftwareOnTime(AnsiString Flag);
extern void LogSoftwareOffTime(AnsiString Flag);
extern TMyStringList *tsSoftwareExeTime;      //Steven 20210526 : 紀錄軟體執行時間
extern int iSoftwareExeTag;
extern int iCheckTorqueCount;
extern double dTorqueArray[2][2000];
extern int iTorqueArrayCnt[2];
extern bool bLowYeildAlarmSpecial;              //Sam 20210505 : PTI 要求的兩段 Low Yeild
extern bool bLowYeildAlarmSpecial1stPass;       //Sam 20210505 : PTI 要求的兩段 Low Yeild
extern bool bHasTrayCSV;                        //Steven 20210629 : Tray Form改成CSV
extern bool bHasPlateCSV;                       //Steven 20210629 : Plate Form改成CSV
extern int  iControlPanelMode;                  //Ken 20210702 AddPadInterface
extern bool bContraPoisitionFlag;
extern int iWhichIndexArm;                      //Sam 20231214 : Temp offset use ready temp range
extern bool bNeedReCheckHeat;

//KaiChen 20200716 ：OHT
//==>
extern bool bOHTUseManual_LP1;
extern bool bOHTUseManual_LP2;
extern bool bLoadPortHasCSTWaitCatch;
extern bool bCassetteRejectAfterSlotMap;

extern int iCatch_Cassette_Push_OHT[10];
extern int iCatch_Cassette_Pop_OHT[10];
extern bool bLoadNeedTray;
extern bool bLoadNeedFailTray;
extern bool bEmptyNeedTray;
extern bool bLoadHasStackedTray;
extern bool bEmptyHasStackedTray;
extern bool bBufferHasNewCassette;
extern bool bLoadPortLoad;
extern bool bBufferHasOldCassette;

extern bool bBuffer6NeedCassette;
extern bool bBuffer6NoNeedCassette;
extern bool bBuffer7NeedCassette;
extern bool bBuffer7NoNeedCassette;
extern bool bLoadPortHaveCassette;
extern bool bBuffer1HaveCassette;
extern bool bBuffer6CanUp;
extern bool bBuffer7CanUp;
extern bool bLoadPortHasCassette_OHT;
extern bool bBuffer6CombineTray;
extern bool bBuffer6SeparationTray;
extern bool bEmptyFull;
extern bool bMRQAMode;
extern bool bMRQAModePassBin;
extern bool bBuffer6NeedEmpty;
extern bool bBuffer7NeedEmpty;
extern bool bCassetteMessage;
extern bool bSTArmMove[3];
extern bool bSTMove;

extern int iCatchFunction;
extern int iCatchBufferFunction;
extern int iPlaceBufferFunction;

extern bool bLoadPortHasCassette;
extern int iCatch_Cassette_Push[10];
extern int iCatch_Cassette_Pop[10];

extern AnsiString CSTData[7];

extern AnsiString SenIndexName_OHT[7];
extern bool bSTArmRun;

extern int iLoadAutoCount;

extern bool bBuffer7CombineTray;
extern bool bBuffer7SeparationTray;

extern int iCSTArmHasTrayCount;                 //KaiChen 20200716 ：OHT Cassette Arm Scan Tray Count
extern int iRecordSenOnOff[150];                //KaiChen 20200716 ：OHT Cassette Arm Scan Tray Count
extern AnsiString asCSTArmHasTrayCount;         //KaiChen 20201019 ：OHT Cassette Arm Scan Tray Count
extern int iLoaderTrayCount_OHT;                       //KaiChen 20201019 ：OHT 紀錄 Loader Tray Count
extern AnsiString asLoaderCassetteID_OHT;       //KaiChen 20201019 ：OHT 紀錄 Loader Cassette ID

extern bool bSecsGem_SLOT_MAP;
extern bool bSecsGem_CST_IN_BUFFER;

extern bool bSecsGem_LoaderTrayID_OK;
extern bool bSecsGem_LoaderTrayID_NG;
extern bool bSecsGem_EmptyTrayID_OK;
extern bool bSecsGem_EmptyTrayID_NG;

extern bool bSecsGem_CHKEmptyTray_OK;
extern bool bSecsGem_CHKEmptyTray_NG;

extern AnsiString asManualEmptyTrayID;

extern int iLoadPortNumber;

extern bool bMultileTrayIDAction;
extern bool bMultileTrayDoorOpen;

extern bool bTempOverLockDoor;

extern bool bMultileDoorIsOpen;

extern bool bMultileDoorState_Old;

extern bool bSendEven_NeedEmptyCST;
extern bool bSendEven_NeedEmptyCST_FristScan;   //KaiChen 20201222 ：OHT，Add

extern bool bSendEven_First;
extern bool bManualCSTOUT;
extern bool bManualRemoveLP2CST;
//<==
//KaiChen 20200716 ：OHT
extern bool bDoIniStartAutoIonFanClean;             //Isaac 20210609 : IO觸發IonFan清針
extern bool bStartAutoIonFanClean;                  //Ifor 20210720 add: IO觸發IonFan清針才顯示
extern bool bReadTrayID;                            //kevin 20210623 TRAY ID Read
extern bool bReadColorTrayID;                       //kevin 20210623 TRAY ID Read
extern AnsiString TrayID[eTrayCount][3];                                        //kevin 20210623  [6]: load empty  [2]: 0:read  1: send pick pos
extern int iTrayTotal[3];                           //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
extern int iOneTrayPickCount[ePosTrayCount];                     //kevin 20210623  loader 吸取數量
extern int iAutoTrayPlaceCount[ePosTrayCount];                    //kevin 20210623  Auto123 Fix 123 放ic數量
extern bool bIsOpenDoorNeedSetErrBin;                 //JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN
//Ifor 20210810 add:Copy 7000
//==>
//extern bool bOutArmPlaceToUnloaderDestroy;                        // 2012.06.14 , Joye , OutArm Place To Unloader Destroy Check
//extern bool bOutArmPlaceToUnloaderDestroy_EMGAndPowerOff;         // 2012.06.14 , Joye , OutArm Place To Unloader Destroy Check
//extern bool bArm1PlaceToShuttleDestroy;                           //Steven 20210818 : Index放料中判斷
//extern bool bArm2PlaceToShuttleDestroy;                           //Steven 20210818 : Index放料中判斷
//<==
//Ifor 20210810 add:Copy 7000
extern int iESD_DecayTask;      //Ifor 20220112 add:改全域變數
extern bool bDecayComplete;     //Ifor 20220119 add:Auto Decay Complete Flag
extern bool bCom1Error;                                             //kevin 20210902 Torque error
extern AnsiString aSendSiteMapping;                                 //Ifor 20201030 add:送Site Mapping 資料給GPIB
extern bool bInArmAutoAlignmentClearFlag;                           //Kenhsieh 20211007 : 解決沒有掃描Kit問題
extern bool bOutArmAutoAlignmentClearFlag;                          //Kenhsieh 20211007 : 解決沒有掃描Kit問題
extern AnsiString sKitOutDiameter[10];                              //kevin 20210813 change  //kevin 20170628 (Steven) add
extern int iAutoCleanRowSel;                                        //KaiChen 20200622 ：修改 10Site2X5 AutoClean
extern int iShuttleRowKit;                                          //KaiChen 20200622 ：修改 10Site2X5 AutoClean
extern int iStiCT;                                                  //kevin 20211106 add 抽測數量計數
extern int StipulateInputCount;                                     //kevin 20211106 抽測 數 量
extern bool bDoLoaderCleanOut;                                      //kevin 20211106 抽測 數 量到達 LOAD 收TRAY
extern bool bTorueReceve;                                           //kevin 20211117 扭力100 筆資料
extern bool bTorueReceveSTOP;                                       //kevin 20211117 扭力STOP資料
extern bool RPLogRecord;                                            //Sam 20201209 : Default Recipe ChangeLog
extern bool bUse8Picker;
extern AnsiString FTPAutomation_Down_ServerIP;          //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Down_UserID;            //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Down_Password;          //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Down_ServerPath;        //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Up_ServerIP;            //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Up_UserID;              //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Up_Password;            //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Up_ServerPath;          //KaiChen 20190530 ：Sigurd FTP Automation
extern AnsiString FTPAutomation_Up_ErrorCodePath;       //KaiChen 20190530 ：Sigurd FTP Automation
extern int iCheckList_Enable[eCL_Total];                //KaiChen 20190530 ：Sigurd FTP Automation
extern bool bSigurdDownload_Recipe;                     //KaiChen 20190530 ：Sigurd FTP Automation
extern bool bSigurdUpload_Recipe;                       //KaiChen 20190530 ：Sigurd FTP Automation
extern bool bSigurdUpload_Jamcode;                      //KaiChen 20190530 ：Sigurd FTP Automation
extern int iRecordSigurdGPIBFlag;                       //KaiChen 20200330 ：紀錄，矽格 GPIB Commend Flag
extern AnsiString CheckListValue_Check;                 //KaiChen 20200108 ：矽格-CheckList 增加顯示異常數值
extern AnsiString CheckListValue_Current;               //KaiChen 20200108 ：矽格-CheckList 增加顯示異常數值
extern int iOneDayLoaderCount;                          //KaiChen 20200618 ：矽格，增加Jam統計頁面
extern bool bOneCycleAfterSHLossIC;                     //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
extern int iIndexArmCheck_SG_Arm1;                      //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
extern int iIndexArmCheck_SG_Arm2;                      //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
//extern bool bLoaderHasSkip_SG;                          //KaiChen 20201125 ： 矽格湖口，Loader 有 Skip 強制放Loader
extern bool bSetBINCOUNT;                               //Sam 20210329 : Add GPIB BINCOUNT_
extern bool bCleanKitSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];            //Steven 20091218 : Avoid duplicate message
extern int iWhichAutoNeedTray;                          //Steven 20211228 : 方便Debug
extern bool USE_ATC_RS232_Check;
extern TQPF_Timer tOEESystemCycle;                      //Jimmychiu 20220712 decrease loading
extern bool USE_ARM_PROTECTION;                         //Steven 20220314 : In Our Arm Z Sensor保護加上開關
extern bool bIndex1Suck;                                //kevin 20220105 Index 在下真空建立 pause 不能關閉
extern bool bIndex2Suck;                                //kevin 20220105 Index 在下真空建立 pause 不能關閉
extern AnsiString sGigasFTPPassWord;                    //Isaac 20210128 : 為了讓Enable FTP不被鎖住，supervisor以下需輸入密碼
extern int iSECSGEMMachineState;                        //JerryYang 20220215 : MachinePreState改成int
extern int iSECSGEMMachinePreState;                     //Ifor 20221019 add: Machine PreS tate改成int
extern AnsiString sMacStatus[30];                       //JerryYang 20220215 : MachinePreState改成int
extern bool bFTPDownlodFinish;
extern bool bHalfViewVerifyNeedAboveSocket;             //JerryYang 20220215 : RTC Auto Verify half view check
extern bool bPickUpErrReAutoVerify;                     //JerryYang 20220215 : RTC Auto Verify half view check
extern bool bAutoSkipCntOver;                           //JerryYang 20210420 : 矽品蘇州要求auto skip數量小於設定值要由工程師排除
extern double dIndexZOffset[3][15];                     //Ifor 20210114 add: Index Z Offset //Ifor 20230207 add: 10 => 11   //Ifor 20230830 add: 11 => 15
extern bool bChangeTest_TempAlarm;                      //Ifor 20210623 add: Test Temp Change
extern int bChangeTest_TempOffset;                      //Ifor 20210623 add: Test Temp Change
extern bool bDoATCTempRise;                             //Ifor 20230504 add: ATC 回溫功能flag
extern bool bReadMCU1,bReadMCU2;                                                //kevin 20220225 read MCU DATA
extern int iUnloaderTrayCountCal[MAX_TRACK];
extern int iFixTrayCountCal[MAX_FIX_TRAY];
extern int iSelectArm;                                                          //kevin 20220426 0: Arm1 Arm 2  1: Arm1  2: Arm2
//extern bool bUseAGV;                                                            //kevin 20220518 AGV Modal
extern AnsiString sLoadPickupErrorTrayPos[MAX_ARM_Row][MAX_ARM_Col];            //kevin 20220521 Load pick error record X, Y 座標
extern int USE_INDEX_ARM_AXES;                                                  //JimmyChiu 20220708 : add Index Arm Axis
extern bool bHotPlateHasSiteMap;                                                //Stevn 20220602
extern int  iResetSiteMappingStep;                                              //Stevn 20220602
extern int iStackCount[MAX_TRACK];                      //kevin 20220527 add Tray Count 0:load 1:empty 2:Color 3:Auto1 4:Auto2 5:Auto3
extern bool bResetGalilTwoYMove;                        //Ifor 20220729 add Reset Galil Two Y Move時間計時
extern AnsiString sRFIDData;                            //Steven 20220713 : RFID Reader for SJSEMI
extern int iRFIDTag;
extern int iInShuttleJam[2][9];                         //kevin 20220819 shuttle Jam  紀錄位置後續，要做放到垃圾桶動作
extern bool bUpdateAutomatically;                       //Sam 20220824 : FTP 自動下載安裝更新包
extern bool bUpdateAutomaticallyChecked;                //Sam 20220824 : FTP 自動下載安裝更新包
extern TMyStringList *slGroundManLog;                           //KenHsieh 20220728 : 新增GroundMan Value Log
extern bool bAutoReceTray;                      //kevin 20220908 add 退料TRAY 補空TRAY
extern int iSensor[4][9]; //kevin 20220912 log Shuttle display
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//==>
//ChungHung 20210113 add for Alignment CCD start
extern bool MACHINE_HAS_AUTO_ALIGNMENT_CCD;                 //ChungHung 20210113 add for Alignment CCD
extern bool Device_Bottom_Blower;
//Action Timing
const unsigned long AutoAlignmentTray_ChangeSetupFile     =0x00000001;
const unsigned long AutoAlignmentTray_InitialStart        =0x00000002;
const unsigned long AutoAlignmentTray_AfterHome           =0x00000004;
const unsigned long AutoAlignmentTray_EveryTray           =0x00000008;
const unsigned long AutoAlignmentTray_AfterOneCycle       =0x00000010;

const unsigned long AutoAlignmentCK_ChangeSetupFile       =0x00000001;
const unsigned long AutoAlignmentCK_InitialStart          =0x00000002;
const unsigned long AutoAlignmentCK_AfterHome             =0x00000004;
const unsigned long AutoAlignmentCK_AfterOneCycle         =0x00000008;

//Do Action
const unsigned long INARM_ALIGNMENT_TEACH_UNIT            =0x00000001;
const unsigned long INARM_ALIGNMENT_TEACH_PICKER          =0x00000002;
const unsigned long INARM_ALIGNMENT_TEACH_LOADER          =0x00000004;
const unsigned long INARM_ALIGNMENT_TEACH_HOTPLAYE1       =0x00000008;
const unsigned long INARM_ALIGNMENT_TEACH_HOTPLAYE2       =0x00000010;
const unsigned long INARM_ALIGNMENT_TEACH_SHUTTLE1        =0x00000020;
const unsigned long INARM_ALIGNMENT_TEACH_SHUTTLE2        =0x00000040;
const unsigned long INARM_ALIGNMENT_TEACH_AUTOCLEAN       =0x00004000;          //KenHsieh 20211214 : AOA add AutoClean

const unsigned long INARM_ALIGNMENT_AUTO_UNIT             =0x00000080;
const unsigned long INARM_ALIGNMENT_AUTO_PICKER           =0x00000100;
const unsigned long INARM_ALIGNMENT_AUTO_LOADER           =0x00000200;
const unsigned long INARM_ALIGNMENT_AUTO_HOTPLAYE1        =0x00000400;
const unsigned long INARM_ALIGNMENT_AUTO_HOTPLAYE2        =0x00000800;
const unsigned long INARM_ALIGNMENT_AUTO_SHUTTLE1         =0x00001000;
const unsigned long INARM_ALIGNMENT_AUTO_SHUTTLE2         =0x00002000;
const unsigned long INARM_ALIGNMENT_AUTO_AUTOCLEAN        =0x00008000;          //KenHsieh 20211214 : AOA add AutoClean

const unsigned long OUTARM_ALIGNMENT_TEACH_UNIT           =0x00000001;
const unsigned long OUTARM_ALIGNMENT_TEACH_PICKER         =0x00000002;
const unsigned long OUTARM_ALIGNMENT_TEACH_AUTO1          =0x00000004;
const unsigned long OUTARM_ALIGNMENT_TEACH_AUTO2          =0x00000008;
const unsigned long OUTARM_ALIGNMENT_TEACH_AUTO3          =0x00000010;
const unsigned long OUTARM_ALIGNMENT_TEACH_AUTO4          =0x00000020;          //Steven 20240428 : Add for HT9011 AOA
const unsigned long OUTARM_ALIGNMENT_TEACH_AUTO5          =0x00000040;
const unsigned long OUTARM_ALIGNMENT_TEACH_AUTO6          =0x00000080;
const unsigned long OUTARM_ALIGNMENT_TEACH_FIX1           =0x00000100;
const unsigned long OUTARM_ALIGNMENT_TEACH_FIX2           =0x00000200;
const unsigned long OUTARM_ALIGNMENT_TEACH_FIX3           =0x00000400;
const unsigned long OUTARM_ALIGNMENT_TEACH_FIX4           =0x00000800;          //Steven 20240428 : Add for HT9011 AOA
const unsigned long OUTARM_ALIGNMENT_TEACH_FIX5           =0x00001000;
const unsigned long OUTARM_ALIGNMENT_TEACH_FIX6           =0x00002000;
const unsigned long OUTARM_ALIGNMENT_TEACH_SHUTTLE1       =0x00004000;
const unsigned long OUTARM_ALIGNMENT_TEACH_SHUTTLE2       =0x00008000;

const unsigned long OUTARM_ALIGNMENT_AUTO_UNIT            =0x00010000;
const unsigned long OUTARM_ALIGNMENT_AUTO_PICKER          =0x00020000;
const unsigned long OUTARM_ALIGNMENT_AUTO_AUTO1           =0x00040000;
const unsigned long OUTARM_ALIGNMENT_AUTO_AUTO2           =0x00080000;
const unsigned long OUTARM_ALIGNMENT_AUTO_AUTO3           =0x00100000;
const unsigned long OUTARM_ALIGNMENT_AUTO_AUTO4           =0x00200000;          //Steven 20240428 : Add for HT9011 AOA
const unsigned long OUTARM_ALIGNMENT_AUTO_AUTO5           =0x00400000;
const unsigned long OUTARM_ALIGNMENT_AUTO_AUTO6           =0x00800000;
const unsigned long OUTARM_ALIGNMENT_AUTO_FIX1            =0x01000000;
const unsigned long OUTARM_ALIGNMENT_AUTO_FIX2            =0x02000000;
const unsigned long OUTARM_ALIGNMENT_AUTO_FIX3            =0x04000000;
const unsigned long OUTARM_ALIGNMENT_AUTO_FIX4            =0x08000000;          //Steven 20240428 : Add for HT9011 AOA
const unsigned long OUTARM_ALIGNMENT_AUTO_FIX5            =0x10000000;
const unsigned long OUTARM_ALIGNMENT_AUTO_FIX6            =0x20000000;
const unsigned long OUTARM_ALIGNMENT_AUTO_SHUTTLE1        =0x40000000;
const unsigned long OUTARM_ALIGNMENT_AUTO_SHUTTLE2        =0x80000000;

extern unsigned long lTeachAlignmentFlag;
extern unsigned long lInArmAutoAlignmentFlag;
extern unsigned long lOutArmAutoAlignmentFlag;

extern unsigned long lInArmAutoAlignmentTrayTimingFlag;
extern unsigned long lOutArmAutoAlignmentTrayTimeingFlag;

extern unsigned long lInArmAutoAlignmentCKTimingFlag;
extern unsigned long lOutArmAutoAlignmentCKTimeingFlag;

//ChungHung 20210113 add for Alignment CCD end

extern bool bRunTeachAlignment;                       //ChungHung 20210113 add for Alignment CCD
extern bool bRunInArmAutoAlignment;                 //ChungHung 20210113 add for Alignment CCD
extern bool bRunOutArmAutoAlignment;                 //ChungHung 20210113 add for Alignment CCD

extern bool bLoaderNeedTrayMustFinish;                 //ChungHung 20210113 add for Alignment CCD
extern bool bAutoNeedTrayMustFinish;                 //ChungHung 20210113 add for Alignment CCD

extern bool bAutoAlignmentInArmShuttle1Lock;
extern bool bAutoAlignmentInArmShuttle2Lock;

extern bool bAutoAlignmentOutArmShuttle1Lock;
extern bool bAutoAlignmentOutArmShuttle2Lock;

extern bool bAlignmentChangeUnloadTray[eTrayCount];

extern bool bSpin;
extern int iCCDAlignmentMotorDelay;
extern int Teach_AutoAlignmentUnit;
//<==
//KenHsieh 20210813 : add CCD AUTO ALIGNMENT
extern bool bHasOutShuttleLose;     //Ifor 20210911 add: KLT 要求out shuttle loss IC alarm之後，按下Start要跳出確認視窗
extern bool bHasNozzleEvent;        //Ifor 20211014 add:按下Rester機台吸嘴中有正在丟料的狀態
extern int iIonAlarmState;          //Ifor 20211014 add:Start Ion Alarm 延遲報警
extern int bIonAlarmDelayTime;      //Ifor 20211014 add:Start Ion Alarm 延遲報警
extern bool bContactModeCheckOpenDoor[2];  //Ifor 20211203 add: KLT要求Contact模式需人員確認InShuttle Sensor與開門
//KenHsieh 20210830 : add CCD AUTO ALIGNMENT for Offset
//==>
extern int iAOA_InArm_Loader_X;
extern int iAOA_InArm_Shuttle1_X;
extern int iAOA_InArm_Shuttle2_X;
extern int iAOA_InArm_Hotplate1_X;
extern int iAOA_InArm_Hotplate2_X;
//<==
//KenHsieh 20210830 : add CCD AUTO ALIGNMENT for Offset
//KenHsieh 20210918 : CCD AUTO ALIGNMENT offset新增
//==>
extern int iAOA_InArm_Loader_Y;
extern int iAOA_InArm_Shuttle1_Y;
extern int iAOA_InArm_Shuttle2_Y;
extern int iAOA_InArm_Hotplate1_Y;
extern int iAOA_InArm_Hotplate2_Y;
extern int iAOA_OutArm_Auto1_X;
extern int iAOA_OutArm_Auto1_Y;
extern int iAOA_OutArm_Auto2_X;
extern int iAOA_OutArm_Auto2_Y;
extern int iAOA_OutArm_Auto3_X;
extern int iAOA_OutArm_Auto3_Y;
extern int iAOA_OutArm_Auto4_X;                                                 //Steven 20240428 : Add for HT9011 AOA
extern int iAOA_OutArm_Auto4_Y;
extern int iAOA_OutArm_Auto5_X;
extern int iAOA_OutArm_Auto5_Y;
extern int iAOA_OutArm_Auto6_X;
extern int iAOA_OutArm_Auto6_Y;
extern int iAOA_OutArm_Fix1_X;
extern int iAOA_OutArm_Fix1_Y;
extern int iAOA_OutArm_Fix2_X;
extern int iAOA_OutArm_Fix2_Y;
extern int iAOA_OutArm_Fix3_X;
extern int iAOA_OutArm_Fix3_Y;
extern int iAOA_OutArm_Fix4_X;                                                  //Steven 20240428 : Add for HT9011 AOA
extern int iAOA_OutArm_Fix4_Y;
extern int iAOA_OutArm_Fix5_X;
extern int iAOA_OutArm_Fix5_Y;
extern int iAOA_OutArm_Fix6_X;
extern int iAOA_OutArm_Fix6_Y;
extern int iAOA_OutArm_Shuttle1_X;
extern int iAOA_OutArm_Shuttle1_Y;
extern int iAOA_OutArm_Shuttle2_X;
extern int iAOA_OutArm_Shuttle2_Y;
//<==
//KenHsieh 20210918 : CCD AUTO ALIGNMENT offset新增
extern bool bInArmXPitch_40mm;                                                  //KenHsieh 20211023 : 新增X Pitch 40mm teach
extern bool bOutArmXPitch_40mm;                                                 //KenHsieh 20211023 : 新增X Pitch 40mm teach
extern bool bNeedOneCycleByAutoAlignment;                                       //KenHsieh 20211202 : 修改重新打開AOA後，先做Onecycle再回Home重做AOA
extern const int iZMovePos;
extern const int iXYMovePos;
extern bool bAutoCleanPlaceToSht;                                               //Steven 20220927 : Add flag for place pad to shuttle
extern int TRAY_MAPPING_GRAB;                                                   //KenHsieh 20220923 : add Tray Map Throw IC Function
extern bool bS2F42_TrayMap_Receive;                                             //KenHsieh 20220923 : add Tray Map Throw IC Function
extern bool bS2F42_TrayMap_FullTray;                                            //KenHsieh 20220923 : add Tray Map Throw IC Function
extern bool bNeedInputEQCQty;                                                   //JerryYang 20220923 : 手動輸入EQC數量
extern bool bStopART;
extern bool bBigMyMessage;
extern bool bInArmWaitOneCycle;
extern double dCycleTimeArr[20];                                                //JerryYang 20220923 : Index cycle time監控
//Jimmychiu 20221011 from mn200motor to mydef
//<==
//-------------
//MotorType
//-------------
extern const int Step_Motor  ;
extern const int Servo_Motor ;
extern const int Rotate_Motor;                                                  //Steven 20170425 (wei) : Add rotate motor
extern const int YASKAWA_Servo_Motor;                                           //Steven 20181111 : Yaskawa馬達
//<==
//Jimmychiu 20221011 from mn200motor to mydef

extern int iByBinCnt[TEST_MAX_BIN+1];
extern int iExceptAutoCnt[eTrayCount];                                          //JerryYang 20240927 : 2D SORT模式Output arm放料避免空洞
extern bool bInitNeedDownloadFTP;                                               //JerryYang 20200416 艾科要求切initial start按start要強制download recipe
extern bool bDoingF16;                                                          //Steven 20221213 : 確認shuttle 有沒有斷線
extern double fIndexDownPos;
extern int iOneCycleFinishShowMsg;                                              //Sam 20250115 : 矽格湖口 GPIB OneCycle 要強制切 ASM //Sam 20221103 : OneCycle 完後顯示訊息
extern AnsiString sOneCycleFinishShowMsg;                                       //Sam 20221103 : OneCycle 完後顯示訊息
extern bool bInitialStartIndexCheckDone;                                        //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
extern double TempFuseLimitType;
extern const double TemperatureFuseLimit170;
extern const double TemperatureFuseLimit200;
extern const double TemperatureFuseLimit250;
extern const double HeatGunLimit;
extern const double ChamberTempLimit;                                           //Ifor 20230328 add: Chamber Temp Limit 165
extern int iE1Count;                                                            //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
extern int iE2Count;
extern int iE3Count;
extern int iENotDefinedCount;

extern int iByLotE1Count[5];                                                    //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
extern int iByLotE2Count[5];
extern int iByLotE3Count[5];
extern int iByLotENotDefinedCount[5];
//Ztex 2023.04.13 Add HT-1032 IO ==>
extern bool bSuperHotTempNoChkHumidity;                                         //Hmy 20190625 Add超高溫，不判斷濕度
extern bool bSuperHotTempChangeTemp[3];                                         //Hmy 20190625 Add超高溫，不判斷濕度
extern int  iFixDoorOpenTime;                                                   //Hmy 20180827 Add Function,fix area frosting protection
extern double dFixAreaDewPoint;                                                 //Hmy 20180827 Add Function,fix area frosting protection
extern bool bDelayTimeAfterFixDoorOpen;                                         //Hmy 20180827 Add Function,fix area frosting protection
extern int iStepOfDelayAfterFixDoorOpen;                                        //Hmy 20180827 Add Function,fix area frosting protection
extern bool bCheckUse_AirStream;
//Ztex 2023.04.13 Add HT-1032 IO <==

//JerryYang 20220909 : add magazine
//==>
extern int iYRegNum;
extern int iAuto3MagazineIndex;                                                 //Auto3 當前的 Mag Tray
extern int iWhichBuff;
extern int iPickWhichBuff;
extern int iWhichMag;                                                           //OutArm 放到哪個 Mag 1-14
extern int iPickWhichMag;
extern int iMagazineFullTrayIndex;
extern bool bMagazineTrayFeedFinish;
extern int TRAY_MISS_CHECK_TIME;
extern bool bMgzTrayCatcCynHomeNeedOff;                                         //pig 2016.06.29 MgzTrayCatchCynHome
extern bool bMagazineTrayFull;                                                  //kevin 20140604 滿TRAY需開門確認
extern bool bMagNeedChangeTray;
extern int iOutArmWhichAuto;                                                    //OutArm 放到哪個 Auto ，放到 Mag 需要是2才是放到 Auo3
extern int iFixToWhichAuto;                                                     //Steven 20260202 : for ATK AMR
extern bool bAuto3HasCover;
extern bool bIsAuto3ReceiveCover;
extern bool bDoSelectMagTrayOut;
extern bool bSelectMagNeedOut[14];
extern bool bInitCheckMag;
extern bool bSetMagTopBin;
extern bool bClearMagTopBin;
extern int  iTopWhichBin;
extern bool bChaneMagTrayflag;
extern bool bMagCatchTrayfalg;                                                  //JerryYang 20221121 : AUTOI3 Magazine還在夾TRAY的旗標
extern bool bMagGetNewTrayflag;
extern int iMagChangeStep;
//<==
//JerryYang 20220909 : add magazine
extern bool bQUERYTJ;
extern bool bATCAlarm;
//Ztex 2023.04.19 Add HT-1032 TriTemp Function
extern bool bOverTempAlarm[8];                          //ChungHung 20151230 add
extern bool biTriTempDefaultbyWork;                     //kevin 20150507 Tri Temp 存工作檔
extern bool bDefrostKeepATCTemp;                        //Hmy 20170120 增加手動除霜功能(判斷用)
extern int iATC51ThermoCount_SH1;                       //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
extern int iATC51ThermoCount_SH2;                       //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
extern int iATC51ThermoCount_HP1;                       //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
extern int iATC51ThermoCount_HP2;                       //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
extern bool bAutoDefrost_Exec;                          //Hmy 20170120 增加手動除霜功能(判斷用)
extern bool bAutoDefrost_Done;                          //Hmy 20170120 增加手動除霜功能(判斷用)
extern bool bAutoDefrost_Halt;                          //Hmy 20170120 增加手動除霜功能(判斷用)
extern bool bAutoDefrost_ByMachineProduction_Req;       //Hmy 20170120 增加手動除霜功能(判斷用)
extern bool bAutoDefrost_ByMachineProduction_Finish;    //Hmy 20170120 增加手動除霜功能(判斷用)
extern bool bAutoDefrost_ByMachineProduction_Termination;
extern bool bAutoDefrost_ByMachineProduction_Wait;
extern bool bAutoDefrost_ByMachineProduction_Force;
extern bool bAutoDefrost_ByMachineProduction_CountDownEnd;
extern bool bAutoDefrost_ByFormMainDeviceRefresh;
extern bool bDefrostNeedSendCommandToAtc;
extern bool bDefrost_abnormalStatus;                    //Hmy 20190614 Add Defrost Abnormal
extern bool bManualDefrost_Start;                       //Hmy 20190614 Add Defrost Abnormal
extern bool bSuperHotKitUseAmbient;                     //Hmy 20190806 Add Check UltraHighTemperature kit
extern bool bSuperHotKitUseAmbient_TempUp2Down;         //Hmy 20190702 Add Check UltraHighTemperature kit
extern bool bSuperHotKit_NowUsehine;                    //Hmy 20190702 Add Check UltraHighTemperature kit
extern bool bAnyOpenDoorForceSwitchDryAir[5];           //Hmy 20190702 Add
extern bool bChangeCheckDoorCheckTime;
extern int iCheckEmptyTrayHasIc;                        //pig 2011.11.29 CheckEmptyTrayHasIcForLoaderHasSkipOrTrayEnd
extern bool bInuptDoorOpend;                            //Hmy 20181120 Add Function,InArm area frosting protection
extern AnsiString asDockingAreaOpen_Function_OpenCloseDate[3];
extern bool bcheckSuperHotTemperatureKitTiming[5];      //Hmy 20190702 Add Check UltraHighTemperature kit
extern bool bcheckSuperHotTemperatureKitFail;           //Hmy 20190702 Add Check UltraHighTemperature kit
extern int iOldTemperatue;                              //Hmy 20190806 Add Check UltraHighTemperature kit
extern int  iSuperHotKit_CheckFinish;                   //Hmy 20191107 Add 超高溫Kit完成完成。
extern AnsiString asBackupSystemDataCopmareDate[2];     //Hmy 20200407 Add Backup SystemData
extern bool bIonBarAirOnEcho;                           //Jeffrey 20180821 add for Ion Bar Auto Calibration
extern bool bIonBarAirOffEcho;                          //Jeffrey 20180821 add for Ion Bar Auto Calibration
extern bool bAllPosTempInRange;                         //Hmy 20191225 Add Automatic operation after waiting for the temperature Reached
extern bool bAllPosTempOkOverTime ;                     //Hmy 20191224  Add Wait Temperature Stable Time
extern bool bReChkTempInRange;                          //Hmy 20191224  Add Wait Temperature Stable Time
extern int iCheckTemperaturetStableOverTimeCount;       //Hmy 20191224  Add Wait Temperature Stable Time
extern int iSiteCheckTemperaturetStableOverTimeCount[40];           //Hmy 20191224  Add Wait Temperature Stable Time
extern AnsiString asLowTemperatureOldNowTime;
extern bool bLowTempAutoDefrost;                        //Hmy 20210329
extern int  iLowTempAutoDefrostTask;                    //Hmy 20210329
extern bool bColdTemperatureOverSetTimeAutoDefrost_TrayFeedFinish;  //Hmy 20210329
extern bool bExecuteFsatCoolDownFunction;               //Hmy 20191231 Add Fsat CoolDown Function
extern int  iExecuteFsatCoolDownFunctionStatus;         //Hmy 20191231 Add Fsat CoolDown Function
extern int  iExecuteFsatCoolDownFunctionTask;           //Hmy 20191231 Add Fsat CoolDown Function
//Hmy 20201207 Add Kit Change Optimiaztion==>
extern bool bUseChangTriTempKitFunction;
extern int  iUseChangTriTempKitFunctionStatus;
extern int  iUseChangTriTempKitFunctionTask;
extern int  iUseChangTriTempKit_IndexArm1ZPosmm;
extern int  iUseChangTriTempKit_IndexArm2ZPosmm;
//Hmy 20201207 Add Kit Change Optimiaztion<==
extern int iTemperatureAlarmSecondByOffsetUpdate;
extern int BASE_HEATER;
extern int SHUTTLE_FLOODGATE;
extern int Tri_Temp_Machine;
extern int AirStream_Select;
extern int Tri_Temperature_MaxDegree;
extern int Tri_Temperature_MinDegree;
extern int TriTemperature_TotalChannel;
extern bool bUT150State[tcTotalCount];
extern bool bUT150HasUse[tcTotalCount];
extern int SetHeaterTemp_MaxOutSht;
extern int SetHeaterTemp_MaxIndex;
extern int SetHeaterTemp_MaxBase;
extern int Total_Compressor;
extern int RefrigeratorUserModeState[8];
extern bool bShowTriTempAlarm_Msg;
extern bool bReadMotorParameter;
extern bool bALLTempFirstReady;
extern bool bAutoDefrost_OpenAirStream;
extern int INDEXDOORHEATER;
//Ztex 2023.04.19 Add HT-1032 TriTemp Function
extern int iAutoFormSize;                               //Jimmychiu 20231130 : 可開關Auto form size
extern int iACSmartCount;                               //Sam 20230111 : Smart Auto Clean
extern int iACSmartCount_CTF;                           //Sam 20240726 : AI Clean
extern int iACUseParam;
extern int iRunACSmart;
extern bool bRunACAdaptive;                             //Sam 20230914 : 自適應性良率監控
extern AnsiString sACRecAlarmCode;                      //Sam 20250916 : Alarm後需要清除資料才能Start
extern AnsiString sACRecEPortCode;
extern bool bLoadingNewICTray;                          //Sam 20240827 : 新增 AMR 功能
extern bool bShowNoteCleanSocket;
extern bool bBarCoderAutoLogin;                         //Sam 20221101 : 使用 BarCoder 自動登錄
extern bool bBarCoderSetupFile;                         //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
extern bool bADAM6024FWIsNew[3];
extern bool bHandlerChangeState;                        //Sam 20230511 : 機台資料變更後須上傳 FTP
extern int i1x2_4UseACEGPicker;                         //Steven 20230530 : 1x2_4改用Row A
extern AnsiString CleanKitRecord[50][50];               //Sam 20230619 : 新增 Clean吸放時間 Log
extern bool bIdleNeedCheckSafeDoor[4][64][32][8];       //Steven 20230704 : add bypass idle check safe door
extern bool bOutArmTrayDuplicateErr[9];
extern bool bTJtoTCWait;                                //wei 20251002 : TJ轉回TC等待
extern int iTotalXPitchStep;
extern int iMaxXPitch;
extern int iMinXPitch;
extern int iMaxXPitch4Pick;
extern int iMinXPitch4Pick;
extern int iMaxXPitch2Pick_13;
extern int iMinXPitch2Pick_13;
extern int iMaxXPitch2Pick_14;
extern int iMinXPitch2Pick_14;
extern int iHPXPos;
extern int iHPYPos;
extern bool bOutarmDropError;                                                   //jou 20241024 : out arm drop error show alarm提醒檢查出Tray是否疊料
//Sam 20230707 : EtherCAT Shuttle sensor
//==>
extern bool bEtherCatMastCardError;                                             //KaiHuang 20200930 : Fix 因斷線造成系統死掉
extern unsigned int uiDevhand;
extern const int iEtherCatRing;
extern unsigned long m_dwDevNum;
//<==
//Sam 20230707 : EtherCAT Shuttle sensor
extern bool bOCRRunTime;
//KenHsieh 20230727 : 更改工作檔與資料 By NetFile
//==>
extern AnsiString asLotID_ByFile;
extern AnsiString asStation_ByFile;
extern AnsiString asType_ByFile;
extern AnsiString asState_ByFile;
extern AnsiString asProductionByFileNamePath;
//<==
//KenHsieh 20230727 : 更改工作檔與資料 By NetFile
extern int iDynamicThresholdNum;                                                //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )

extern bool Enable_PLCSafety_IO;                                                //Austin 20190531.01 增加PLC_IO模組及安全相關IO.
extern int iPLCSafetyVer;                                                       //joe 20220406.01 debug safe plc
extern int iWhichShtPickFor;                                                    //Steven 20231208 : 判斷多吸嘴模式的Shuttle
//Kevin 20231209 : for ASE KH
//===>
extern bool bColorFirst;                                                        //kevin 20221025 load new color tray
extern bool bLoadBFBackTray;                                                    //KenHsieh 20230325 : CleanOut時Loader BF 退Tray
extern bool bAutoUpOK[3];                                                       //kevin 20230331 Auto 1 23 上升中不能放tray
extern bool bEPDetect;                                                          //kevin 20230608 index check 流量計是否漏氣
//extern int iLotStart;                                                           //kevin 20211105
extern AnsiString asInarmSkipICPos;                                             //KenHsieh 20230707 : No9 pickup error後，紀錄G17功能所記錄點位之Log
extern AnsiString asInarmSkipICSucker;                                          //KenHsieh 20230907 : G17功能所記錄最終點位之Log改為"all吸嘴:all Pos"
extern AnsiString asInarmSkipICPos_No9;                                         //KenHsieh 20230707 : No9 pickup error後，紀錄G17功能所記錄點位之Log
extern bool bContinueFailIndexUp;                                               //kevin 20230807 錯誤 ARM 需上升避免結霜
extern bool bD52ArmUp_ConsecutiveFail;                                          //KenHsieh 20230809 : 發socket連續性異常前，Indexarm需先上升
extern bool bSoftwareStart;                                                     //kevin 20230812  軟體剛開啟
extern bool bAutoReceTray_SECS;                                                 //KenHsieh 20231016 : ASEKH_K1 & K3 補上蓋與空盤事件流程
extern bool bCheckOpenDoor;                                                     //kevin 20231122 check door open close log
//<==
//Kevin 20231209 : for ASE KH
extern bool bInitailQtyByLowYield;                                              //Sam 20230728 : 發生 LowYield 後重新 Start 清除 Sort Count 和 contact counterKinds
extern bool bOLPSetCategroy;                                                    //Sam 20230711 : 修正 OLP SetCategroy 異常
extern AnsiString sB03RunData;                                                  //Sam 20231115 : PTI 新增 Tester report
extern AnsiString sB03StartTime;                                                //Sam 20231115 : PTI 新增 Tester report
extern int iSourceSystemHeight;                                                 // 20230601 Joseph , Auto Form Size //
extern int iSourceSystemWidth;                                                  // 20230601 Joseph , Auto Form Size //
extern int iNowSystemHeight;                                                    // 20230601 Joseph , Auto Form Size //
extern int iNowSystemWidth;                                                     // 20230601 Joseph , Auto Form Size //
extern double fNowSystemHeightScale;                                            // 20231030 Jason , Auto Form Size //
extern double fNowSystemWidthScale;                                             // 20231030 Jason , Auto Form Size //
extern int iPickFromPlateMode;
extern int DOUBLE_BELT_MODE;                                                    //Ztex 2023.12.13 Add Double Belt Mode
extern int iEnabledSiteCount;                                                   //Steven 20231220 : 牛信保說要開site數量SVID 1250
extern bool bWaitOutArmCheckCylin;                                              //jou 20240131 : 修正out arm 與 auto tray互卡衝突hang up
extern int  iInZHomeCnt;
extern int  iOutZHomeCnt;
extern AnsiString sStackBinTemp[eTrayCount];                                    //JerryYang 20231218 : P53防混功能
extern bool bNoitceFixTray;

extern int iMagazineStatus;                                                     //Ifor 20231130 add: 0: Normal 1:Full Tray 2:Manual put tray
extern bool bOneCycleDoInitialTemp;
extern bool bDoAfterOpenSafeDoor3NeedDelay;                                     //Ifor 20220413 add: SafeDoor 3 Open Ovet SetTime Need Delay

extern bool bSECSGEMAutoclean;                                                  //Ifor 20220414 add SECS/GEM AUTO Clean
extern bool bBinDispAlarm;                                                      //Ifor 20220714 add:Bin Disp 異常報警 每次Onecycle 檢查一次
extern int iTrayDetectCount;                                                    //Ifor 20221117 add:Tray arm移動報警偵測次數
extern bool bResetGalilTwoYMove;                                                //Ifor 20220729 add Reset Galil Two Y Move時間計時
extern bool bEnableAutoMotive;                                                  //Ifor 20221024 add: KLT Auto Motive Function
extern bool bNeedAutoHeight;                                                    //Ifor 20221208 add: KLT 要求更換工作檔需要重新Auto Height 才可執行Contact Mode
extern TDateTime tAlarmNoteStartTime;                                           //Ifor 20230330 add:Show Alarm Note Init Temperature offset function
extern bool bShowAlarmNote;                                                     //Ifor 20230330 add:Show Alarm Note Init Temperature offset function
extern bool bCheckATCTemp;                                                      //Ifor 20230410 add: 按下Start確認ATC溫度是否達到設定
extern bool bOneKyePowerSaving;                                                 //Ifor 20230612 :add 一鍵Power Saving
extern int iAutoHasHod[6];                                                      //Ifor 20230817 add: 退Tray時有HasNullIC報警提示
extern bool bAQL_Sorting_Finish;                                                //Ifor 20231124 add:避免AQL Mode 誤報空穴問題
extern AnsiString asAutoCleanEven;                                              //Ifor 20210527 add: Auto Clean Start Even
extern int USE_STM_Function;                                                    //Ifor 20201113 add: STM Function
extern int USE_BU5_Function;                                                    //Ifor 20210819 add:LYEC BU5 使用BU3程式版本
extern int iRTC_CCD_NG[4];                                                      //Ifor 20210203 : add CCD NG Result  //Ifor 20210422 add:RTC CCD 2 => 4
extern bool bStartAQLSortMode;
extern int iAQLBin;
extern int iAQLCount;
extern bool bStartAutoAdjustLight;                                              //Ifor 20210408 add:Barcode 自動調整光源
extern bool bSetAQLSortMode;                                                    //Ifor 20210513 add: SECS GEM Set AQL Count
extern bool bBarcodeNeedAutoAdjust[4];                                          //Ifor 20210531 add: Barcode Auto Adjust Light
extern bool bErrBinAlm;
extern bool bStartCheckZ;
extern int iMagazineCheckZPos;
extern bool bSmartSetupAutoHeight;
extern bool bSmartSetupZCalibration;
extern bool bSmartSetupInOutArmSearchArea;
extern bool bSmartSetupUseSingleZ;
extern bool bSmartSetupSHValue;
extern bool bMagazineGetNewTray;
extern int iSECSGEM_ConsecutiveFailureAlarm;                                    //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
extern bool bDoInitialAutoClean;                                                //Ifor 20240604 add:Auto Clean Inital Contact Count
extern AnsiString asMagazineBinFull;                                            //Ifor 20240722 add:
extern int iSECSGEM_ConsecutiveFailureAlarm;                                    //Ifor 20240425 add:secs gem cmd
extern int USE_COVER_TRAYID;
extern AnsiString asBundleTrayID[ePortTotal];
extern bool bNeed1DCoverTray[MAX_AUTO_TRAY];
extern bool bNeedCoverTray[MAX_AUTO_TRAY];

extern bool bHasCoverTray[MAX_AUTO_TRAY];                                       //JerryYang 20240318 : AUTO1-6, FIX1-6
extern bool bHas1DCoverTray[MAX_AUTO_TRAY];

extern int iBundleIn;
extern int iBundleOut;
extern bool bAUTORev[MAX_AUTO_TRAY];
extern AnsiString sBundleEndInfo;
extern AnsiString sProcessEndInfo;
extern AnsiString sUnloadBundleID;
extern AnsiString sTotalLotID;
extern AnsiString sCoverTrayID;
extern AnsiString sCIDOnTrayArm;
extern bool bMustCoverIDTray;
extern AnsiString sFixBundleID[MAX_FIX_TRAY];
extern int iBundleIDLength;
extern bool bI22_NeedHomeDelay;
extern bool bRTCAutoTuning;                                                     //Sam 20240711 : RTC Auto Tuning
extern bool bReadLotInfoFromART;                                                //Steven 20240830 : 讀取lot info的位置
extern int VCCU_UNIT_TYPE;                                                      //Sam 20230210 : 新增 VacuumUnit 通訊模組
extern bool bSuckingFlagZ1, bSuckingFlagZ2;                                     //Steven 20240916 : index下降到shuttle吸放料
extern int iGPIBIndexStatus;
extern double dGPIBATCOffset[32];
extern int iTempCode[INDEX_HEAT_COUNT];                                         //Jimmychiu 20230628 : 減少DTME加熱迴圈次數
extern bool bGPIBOffsetCommand;
extern int iMN200_ErrorCode;
extern bool bReduceTrayExitTime;                                                //Ztex 2024.03.25 Add Reduce tray exit time
extern bool bReduceTrayExitTime_TrayFeed[6];                                    //Ztex 2024.03.25 Add Reduce tray exit time
extern bool bServoOnOff;                                                        //ChungHung 20230718 add for Safe plc
extern bool bByPassDoor;                                                        //Ztex 2024.05.13 bByPassDoor
extern bool bTT_SetSpeed_Check;                                                 //Ztex 2024.08.11 Add Test Time Set Speed
extern bool bWaiteKeepRespond;                                                  //kevin 20231215 等待eKeep 回應
extern bool bAutoReceTrayAGV[MAX_AUTO_TRAY];                                    //kevin 20240423 add 退料TRAY call AGV
extern bool bReceiveProcess[MAX_AUTO_TRAY];                                     //kevin 20240514 Auto 1 2 3
extern bool bUseSocketHeating_Wait;                                             //Ztex 2024.09.07 Add Use Socket Heating
extern int  iUseSocketHeating_Time;                                             //Ztex 2024.09.07 Add Use Socket Heating
extern int  iAStreamErrorCompressOnecycle;                                      //Ztex 2024.10.01 Add AStream Error Compress Onecycle
extern bool bPickLoaderFromBack;
extern bool bWaitRotateFinish;                                                  //Ifor 20241015 add: 等待 Rotate 完成
extern int iAMD_Function;                                                       //Ifor 20231222 add TFAMD Function 0:AMD(新加坡、美國) 1:TFAMD(蘇州) 2:TFAMD(馬來西亞)
extern bool bHasFTPDownload;                                                    //Ifor 20240422 add:FTP 工作檔下載
extern int iChangeFileHasErr;                                                   //Ifor 20240314 add:GPIB 切換工作檔時有異常
extern AnsiString asChangeSetupFileName;

extern bool bResetMagGetTime;
extern bool bMN200Error;
extern bool bDoROILearning;                                                     //Ifor 20240919 add
extern bool bDummyRunPosCheck;
extern bool bDummyRunPosCheckFinish;
extern bool bDummyRunPosCheckCatchTray[6];
extern bool bNowUseArmSuck[2][2][8];
extern bool bNeedCheckShuttleSensor;
extern int USE_GPIBLOGBYDAY;                                                    //Ifor 20241029 add:GPIB LOG BY DAY
extern bool bATC_EnableSiteMap[32];                                             //Ifor 20241105 add:ATC Enable Site Map
extern bool bGPIBLOGBYDAY;                                                      //Ifor 20241029 add:GPIB LOG BY DAY
extern bool bDoD80Function;                                                     //Ifor 20241202 add:避免[D80]無限Index Check 與未完成顯示One Cycle Finish
extern bool bInitialAutoCleanTimeOut;                                           //Ifor 20241212 add:避免Auto Clean 暫停過久Start後報警
extern bool bCustomerForMTKVersion;                                             //yunghsin 20241130 Add MTK File Name
extern AnsiString anSoftwareVersion;                                            //yunghsin 20241130 Add MTK File Name
extern bool bHasOpenMagDoor;                                                    //JerryYang 20241225 : 暫停狀態開Magazine安全門
extern AnsiString anGroundManLog;                                               //Ifor 20250123 KYEC FTP UP Load GroundMan Log File
extern int TROLLEY_USE_IO_TYPE;                                                 //Ifor 20250211 : add TROLLEY USE IO TYPE
extern int iRTCErrorCount;                                                      //wei 20221222 RTC ARM Error
extern TQPF_Timer tRTCErrorTimeOut;                                             //wei 20221222 RTC ARM Error
extern bool iRTCErrorSend;                                                      //wei 20221222 RTC ARM Error
extern bool bCheckThirdPos;                                                     //wei 20221222 RTC ARM Error
extern AnsiString anMultiEPData;                                                //Ifor 20250417 add:Multi EP Data
extern bool bOutArmIonFanGiveWay;

extern AnsiString asSupplementBin;
extern AnsiString asLDUnLDCheckAGV;
extern AnsiString asLDUnLDFinsihAGV;

extern bool bLoaderActionFlag[3];
extern bool bUnLoaderActionFlag[3];
extern bool bAMRReceiveAGVStart;//Eastsun 20260515 F009 KYEC AMR 3 receive flags
extern bool bAMRReceiveStart;//Eastsun 20260515 F009 KYEC AMR 3 receive flags
extern bool bAMRReceiveLoaderTotalTray;//Eastsun 20260515 F009 KYEC AMR 3 receive flags
extern bool bLoaderSECSActionFlag[3];
extern bool bUnLoaderSECSActionFlag[3];

extern bool bLoaderLockActionFlag[3];
extern bool bUnLoaderLockActionFlag[3];

//EtherCat
extern const int EtherCat_IO_Ring;
extern bool bFirstClickJog;

extern bool bLoaderCocverRemove;
extern bool bByPassAutoLock;

extern int iLoaderTrayCountCal;
extern int iLoaderTrayCountAMRCal;
extern bool bReflashTrayCount;
extern int iSECSSetTrayCount;//Eastsun 20260515 F020 AMR LoaderAction dependency
extern bool bNeedWaitTemp;
extern AnsiString asReadTCWater[ATC_HEAD_COUNT];
extern AnsiString asReadDynamicPID[ATC_HEAD_COUNT];
extern AnsiString asSetDynamicPID[ATC_HEAD_COUNT];
extern bool bGPIBAutoHeightMove;
extern AnsiString  asGPIBAutoHeight;
extern int iGPIBAutoHeightCount;
extern bool bGPIBAutoHeightPass;
extern AnsiString asGPIBAutoHeightLimit;
extern AnsiString asGPIBAutoHeightZPos;
extern AnsiString asGPIBAutoHeightTorque;
extern AnsiString asControlModeGPIB;
extern int iMagBin;                                                             //Eastsun 20260515 F011 整合 (Magazine AMR shared)
extern int ASE_OutTrayNum[eTrayCount];                                       //Eastsun 20260515 F011 整合:KYEC AMR output tray count
//==> Eastsun 20260525 laser 整合 : LaserSensorShuttle InArm 互鎖
extern bool bInArmLaserActionflag[2];
extern bool bOutArmLaserActionflag[2];
//<== Eastsun 20260525 laser 整合
#endif
//------------------------------------------------------------------------------
//Jimmychiu 20240712 : Auto Calibrate Suck Z height
//<==
extern bool bRunArmSuckZAuto;
extern bool bOncycleFinishRunArmSuckZAuto;
extern bool bEnableInarmSuckZAuto;
extern bool bEnableOutarmSuckZAuto;
extern int iInArmZHeightDiff[MAX_ARM_Row][MAX_ARM_Col];
extern int iOutArmZHeightDiff[MAX_ARM_Row][MAX_ARM_Col];
extern uPoint2D InArmAutoCalSuckZPoint;
extern uPoint2D OutArmAutoCalSuckZPoint;
extern int iInArmSearchStartZ;
extern int iOutArmSearchStartZ;
//<==
//Jimmychiu 20240712 : Auto Calibrate Suck Z height
//------------------------------------------------------------------------------
extern double GetTotalYield_double();
extern AnsiString GetTotalYield_Str();
extern bool bNeedOneCycleByContactAlm1;                                         //Sam 20241226 : Contact Alarm 需要先做 OneCycle
extern bool bNeedOneCycleByContactAlm2;
extern bool bNeedOneCycleByContactWar1;
extern bool bNeedOneCycleByContactWar2;
extern bool bNeedOneCycleByPickerLifeAlm;                                       //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
extern void CheckPickerLifeNeedOneCycle();                                      //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
extern bool bTriggerRTC_AutoSTD;                                                //JerryYang 20240829 : SPIL訓永 要求手動觸發RTC AUTO STD
extern AnsiString sWhiteListLotID;                                              //JerryYang 20241104 : 支援2DID白名單功能
extern AnsiString sWhiteListProcess;
extern int iBackupOneCycle;                                                     //JerryYang 20241015 : 修正觸發One cycle後又執行auto clean, 完成後會繼續跑
extern bool bRefreshCount;                                                      //Stteve 20250219 : 清除數量後的重整要delay一下
extern bool bRun2DCheck;                                                        //JerryYang 20250220 : 2DID硬體順序檢查功能
extern int i2DMAPCHKSTEP;                                                       //JerryYang 20250220 : 2DID硬體順序檢查功能
extern int iBundleDieQty;                                                       //JerryYang 20250224 : add
extern bool bBindChkNG;
extern bool bUnbindChkNG;
extern bool bTRAYCHKNG;
extern AnsiString sUnloadBin[MAX_UNLOAD_TRAY];
extern bool bNeedReportBundleID[eTrayCount];
extern bool bBackupAutoClean;                                                   //JerryYang 20250514 : fix 2DID與Auto clean衝突, 做完2DID Map再把auto clean旗標打true
//extern bool bVTestNoRTBin;                                                    //RogerYang 20250814 : 改到TrayAssignment下  //RogerYang 20250626 偉測不可複測bin功能
extern bool bNoRTBinFixFlag[3];                                                 //RogerYang 20250626 偉測不可複測bin功能
extern bool bAskStopPort[ePortTotal];                                           //JerryYang 20250521 : For AMR
extern bool bPortIsBusy[ePortTotal];
extern int iThisPortNo;
extern int iThisPortStatus;
extern int iPortStatus[ePortTotal];
extern int iLastPortStatus[ePortTotal];
extern AnsiString sBinCode_ATK[eTrayCount];
extern bool bWaitingAMR;                                                        //RogerYang 20250617 Load已清空但還沒滿bundle，等料車來
extern bool bForceSendLoaderIsEmpty;
extern bool bIsPlacingToBuffer;                                                 //JerryYang 20250828 : fix color誤退TRAY
extern bool bIsCatchingFromBuffer;                                              //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中
extern bool bMaintanceMode;                                                     //Steven 20251007 : maintance mode for Hana
extern int  In_Shuttle_Auto_Latch;                                              //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
extern bool bInSh1DoLtc;
extern bool bInSh2DoLtc;
extern bool bInSHLtcFin[2];                                                     //KenHsieh 20251106 : fix close site don't do In Sht Latch
extern int  iInShtZRange;                                                       //KenHsieh 20251107 : Add Range for InSHZ 避免初始高度太高造成校正之門檻值過高
extern bool bNeedGetSHRightLtc[2];                                              //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
extern bool bInSht1ZNeedHome;                                                   //KenHsieh 20250924 : 避免InSHZ 回Home時暫停重啟後全機回Home
extern bool bInSht2ZNeedHome;                                                   //KenHsieh 20250924 : 避免InSHZ 回Home時暫停重啟後全機回Home
extern bool bInSHLtcErrNo[2][10];                                               //KenHsieh 20251128 : InShtLtc 新增異常重複判斷
extern bool bDoRTCVerify;
extern bool bRenesasFTCTAlarm;                                                  //RogerYang 20250923 : 瑞薩FT-CT
extern bool bAutoRestartAfterFTCTAlarm;                                         //RogerYang 20251016 : 瑞薩FT-CT
extern bool bReplyFTCTAlarm;                                                    //RogerYang 20251021 : 瑞薩FT-CT FTCT發送違法指令回傳Error需要報警
extern AnsiString sFTCTAlarmStr;                                                //RogerYang 20251016 : 瑞薩FT-CT
extern bool bContinueMessageByReply71;                                          //RogerYang 20251107 : 瑞薩FT-CT Add Continue Form
extern bool bDestoryOnSht;                                                      //RogerYang 20251021 : 從inarm2搬過來 //Steven 20170905 (wei) : 在Shuttle吹氣與資料交換的Flag

extern int iTrayEnable[3];

extern bool bE84LoaderActionflag[3];
extern bool bE84UnloaderActionflag[3];
extern bool bBoatChangeCasset[3];                                               //Ifor 20251220 add:Boat Change
extern bool bHasTjTemp;                                                         //JerryYang 20251124 : Tj control吃不同的溫度range
extern int iCheckShuttleSensor;                                                 //Ifor 20251204 add:After Home Need Check Shuttle Sensor
extern bool bFreezeHandler;                                                     //Ifor 20260119 add:Freeze Handler
extern int iSECSGEMMachineState;
extern AnsiString sMacStatus[30];
extern bool bChangeModeING;
extern bool bASMFirstTime;
extern DWORD startASMTick;
extern DWORD endASMTick;
extern TDateTime tASMDateTime;
extern bool bFullSiteTestComplete;
extern bool bWaitASVFullSiteTestResult;
extern bool bFullsiteBackHP;
extern int iBackupWihchSht;
extern int iBackupWihchKit;
extern bool bFindPickICFail;
extern int iSystemUpTime;
extern TQPF_Timer tUNDER_20A_Delay;                      //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
extern TQPF_Timer tUNDER_20A_SHT_Delay;                      //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
extern const int iUNDER_20A_Delay_Sec;                   //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
extern bool bAutoRetryFlag;                              //JerryYang 20220901 : ASE-CL pick up error時吸取邏輯要改找吸最左上角的

extern AnsiString sSiteLayoutSiteMap;
extern AnsiString sSiteLayoutASECL;
extern AnsiString sSiteMapASECL;
extern AnsiString sBinMap;
extern int iWhoTrigerASV;
extern bool bNeedDoRunCheck;
extern int iPauseTime;
extern int iNetUPH;
extern int iGrossUPH;
extern bool bSECSPause;
extern bool bHasChangePlate;
extern bool bDoInZTeach;
extern bool bDoOutZTeach;
extern bool bNeedDoRemainCheck;
extern int  iSV_ErrBinCnt;
extern bool bTrayArmIsPickColor;
extern bool bNeedBigMsg;                                           //JerryYang 20250411 : add
extern int iSECS_HotPlateSeclect;

extern int iSVByBinCount[TEST_MAX_BIN];
extern bool bSECS_REALTIMECCD;
extern bool bNeedRestartSW;
extern bool bInitialCylinderCheck;
extern int iP65QAReTestCount;                                                   //Ifor 20260407 add: [P65] ARM QA Mode ReTest Counter
extern bool bP65QAReTest;                                                       //Ifor 20260407 add: [P65] QA ReTest flag, notify RunTestProgram to send iLotStatus=2
extern bool bP65QAING;                                                          //Ifor 20260407 add: [P65] QA Mode QA in progress    //AI(W906-GB-P2c) 20260926: golden 912 cmydef.h:6037
extern bool bAOIConsecutiveFailFlag;
extern bool bAOIConsecutiveFailPictureFlag;
extern bool bAOIFailCountFlag;

extern int iAOILotCount;

extern bool bInArmLaserActionflag[2];
extern bool bOutArmLaserActionflag[2];

extern int iRTCErrorCount;              //wei 20221222 RTC ARM Error
extern TQPF_Timer tRTCErrorTimeOut;     //wei 20221222 RTC ARM Error
extern bool iRTCErrorSend;              //wei 20221222 RTC ARM Error
extern bool bCheckThirdPos;             //wei 20221222 RTC ARM Error

extern bool bWaitSECS;                                                           //KevinCheng 20250922 : Wait SECS
extern bool bEject;                                                             //JerryYang 20251020 : 渠梁半清機功能
extern bool bNeedEject[MAX_AUTO_TRAY];                                          //JerryYang 20251020 : 渠梁半清機功能
extern bool bPurgeOutAllDevice;                                                 //JerryYang 20251020 : 渠梁半清機功能

extern bool bNeedAMRMagazineUnload;                                             //Eastsun 20260515 : 整合
extern bool bAMRMagazineAutoReceive;                                            //Eastsun 20260515 : 整合

extern bool bAutoChangingWarn[3];                                               //Eastsun 20260513 :[0]=Auto1, [1]=Auto2, [2]=Auto3 旗標
extern int  iAutoColorBak[3];                                                   //Eastsun 20260513 :原顏色代碼還原備份 (0~4 對應 ColorMap)
extern int  iATC_RecipeFileTransfer;                                            //Eastsun 20260522 add: ATC Recipe FileTransfer state
//AI(W906-IOWEB-P4) 20260925: PCI1203_IO -- Advantech PCIE-1203 當 IO 卡（Jimmy 20260925 裁決 A；來源機台 wip/field-20260924 的 AI(W906-1203-IO1) 20260903）。golden 沒有這個值；4 是下一個空號，不會和 golden 的 {2,3} 比較或 MN200 專用的 {1,2} 路徑撞號。放在檔尾（guard 之外，同值重複 define 合法）以免行號位移。
#define PCI1203_IO 4
