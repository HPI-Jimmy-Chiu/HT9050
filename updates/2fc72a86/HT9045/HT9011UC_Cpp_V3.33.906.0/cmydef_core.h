//==============================================================================
#ifndef cmydef_coreH
#define cmydef_coreH
//AI(W906-S1) 20261007 (Ifor01; W-124): split out of cmydef.h (header-slimming plan s A5, S1) -- A + B, PCI132 stays here.
//  Lines moved verbatim, original order; cmydef.h still includes all three, so no includer changes (that is S2).
#include "MachineType.h"
#include "myTimer.h"
#include "cprod.h"
#include "cpublic.h"
//---- old cmydef.h :12-301 (A system macros / card types incl. PCI132, B IC status / heater types / basic flags)
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
//---- old cmydef.h :5957 (was outside the guard): card type macro
//AI(W906-IOWEB-P4) 20260925: PCI1203_IO -- Advantech PCIE-1203 當 IO 卡（Jimmy 20260925 裁決 A；來源機台 wip/field-20260924 的 AI(W906-1203-IO1) 20260903）。golden 沒有這個值；4 是下一個空號，不會和 golden 的 {2,3} 比較或 MN200 專用的 {1,2} 路徑撞號。放在檔尾（guard 之外，同值重複 define 合法）以免行號位移。
#define PCI1203_IO 4
#endif
