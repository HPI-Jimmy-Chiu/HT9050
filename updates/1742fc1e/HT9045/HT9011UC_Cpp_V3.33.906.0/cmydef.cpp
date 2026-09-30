//AI(W0-TAIL) 20260626: de-VCL cmydef.cpp. Dropped MachineDefine.h (VCL+drivers) +
//  common.h (app). Kept cmydef.h. ACTIVE = the global var DEFINITIONS (AnsiString
//  name-tables, const ints, SortingBinTray/SiteData globals, TQPF_Timer instances).
//  Function bodies (SaveEventLog / InitialMemory / GetTotalYield_*) are gated:
//  they reach TMyStringList (incomplete fwd-decl) + state-machine/app globals.
#include "cmydef.h"
#include "vclcompat/vcl_compat.h"   //AI(W0-TAIL) 20260626: explicit (also via cmydef.h)
//---------------------------------------------------------------------------

// all type program can use
//----------------------------------------------------------------------------
//Steven 20091231 : 整合AlarmCode
//AnsiString InOurArmSuckName[MAX_ARM_Row][MAX_ARM_Col]={{" A", " C", " E", " G"}, {" B", " D", " F", " H"}};
AnsiString IndexSuckName[4][8]   ={{" Aa", " Ab", " Ac", " Ad", " Ae", " Af", " Ag", " Ah"},
                                   {" Ba", " Bb", " Bc", " Bd", " Be", " Bf", " Bg", " Bh"},
                                   {" Ca", " Cb", " Cc", " Cd", " Ce", " Cf", " Cg", " Ch"},
                                   {" Da", " Db", " Dc", " Dd", " De", " Df", " Dg", " Dh"}};

//AnsiString IndexSuckName[16]     ={" Aa", " Ab", " Ac", " Ad", " Ae", " Af", " Ag", " Ah", " Ba", " Bb", " Bc", " Bd", " Be", " Bf", " Bg", " Bh"};
AnsiString ShuttleSensorName[9]  ={" 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9"};
AnsiString sTrayPosName[MAX_TRACK]={"Loader", "Empty", "Color", "Auto 1", "Auto 2", "Auto 3", "Auto 4", "Auto 5", "Auto 6"};

AnsiString asTrayAlias[eTrayCount]={"A1",  "A2",  "A3",  "A4",  "A5",  "A6",
                                    "F1",  "F2",  "F3",  "F4",  "F5",  "F6",
                                    "F7",  "F8",  "F9",  "F10", "F11", "F12", "BB",
                                    "M1",  "M2",  "M3",  "M4",  "M5",  "M6",  "M7",
                                    "M8",  "M9",  "M10", "M11", "M12", "M13", "M14"};

AnsiString asTrayForBinDisp[eBinDispTotal]={"Loader", "Empty", "Color",
                                            "Auto1", "Auto2", "Auto3", "Auto4", "Auto5", "Auto6",
                                            "Fix1",  "Fix2",  "Fix3",  "Fix4",  "Fix5",  "Fix6",
                                            "Fix7",  "Fix8",  "Fix9",  "Fix10", "Fix11", "Fix12", "BulkBox",
                                            "Mag1",  "Mag2",  "Mag3",  "Mag4",  "Mag5",  "Mag6",  "Mag7",
                                            "Mag8",  "Mag9",  "Mag10", "Mag11", "Mag12", "Mag13", "Mag14"};  //JerryYang 20220909 : 12->eBinDispTotal

AnsiString s3TrayName[e3TrayCount]={"Auto1", "Auto2", "Auto3",
                                    "Fix1",  "Fix2",  "Fix3",  "Fix4",  "Fix5",  "Fix6",
                                    "BulkBox",
                                    "Magazine1",   "Magazine2",  "Magazine3",  "Magazine4",  "Magazine5",  "Magazine6",  "Magazine7",   //JerryYang 20221215 : add Magazine
                                    "Magazine8",   "Magazine9",  "Magazine10", "Magazine11", "Magazine12", "Magazine13", "Magazine14",
                                    "Auto4", "Auto5", "Auto6",
                                    "Fix7",  "Fix8",  "Fix9",  "Fix10",  "Fix11",  "Fix12"};

AnsiString s6TrayName[eTrayCount] ={"Auto1", "Auto2", "Auto3", "Auto4", "Auto5", "Auto6",
                                    "Fix1",  "Fix2",  "Fix3",  "Fix4",  "Fix5",  "Fix6",
                                    "Fix7",  "Fix8",  "Fix9",  "Fix10", "Fix11", "Fix12", "BulkBox",
                                    "Magazine1",   "Magazine2",  "Magazine3",  "Magazine4",  "Magazine5",  "Magazine6",  "Magazine7",   //JerryYang 20221215 : add Magazine
                                    "Magazine8",   "Magazine9",  "Magazine10", "Magazine11", "Magazine12", "Magazine13", "Magazine14"};

AnsiString s06TrayName[eTrayCount]={"Auto01", "Auto02", "Auto03", "Auto04", "Auto05", "Auto06",
                                    "Fix01",  "Fix02",  "Fix03",  "Fix04",  "Fix05",  "Fix06",
                                    "Fix07",  "Fix08",  "Fix09",  "Fix10", "Fix11", "Fix12", "BulkBox",
                                    "Magazine01",   "Magazine02",  "Magazine03",  "Magazine04",  "Magazine05",  "Magazine06",  "Magazine07",
                                    "Magazine08",   "Magazine09",  "Magazine10",  "Magazine11",  "Magazine12",  "Magazine13",  "Magazine14"};

AnsiString s6ShortTrayName[eTrayCount]={"Auto1", "Auto2", "Auto3", "Auto4", "Auto5", "Auto6",
                                        "Fix1",  "Fix2",  "Fix3",  "Fix4",  "Fix5",  "Fix6",
                                        "Fix7",  "Fix8",  "Fix9",  "Fix10",  "Fix11",  "Fix12",  "BulkBox",
                                        "Mag1",   "Mag2",   "Mag3",   "Mag4",   "Mag5",   "Mag6",   "Mag7",
                                        "Mag8",   "Mag9",   "Mag10",  "Mag11",  "Mag12",  "Mag13",  "Mag14"};

AnsiString StartModeName[rsmRunModeTotal]={"Continuous Start" , "Initial Start",
                                           "Re-Test Continuous", "Re-Test Initial Start",           //Steven 20130604 : SCS說要跟NS用一樣的字眼
                                           "Site Mapping Check", "QA Mode",
                                           "Continuous EQC" , "Initial EQC",
                                           "Initial_ART","ContinuStart_ART","ContinuRetest_ART",    //ChungHung 20141002 add for KYEC AutoRetest
                                           "Auto Retest",                                           //Steven 20140331 : Auto Retest
                                           "FIFO Mode",                                             //Steven 20160303 : FIFO Mode
                                           "Initial_MRT","ContinuStart_MRT","Retest_MRT"};   //Ifor 20170316 add KYEC MRT Mode

AnsiString sOEETimeDataName[tdTotal]={"PowerOn",        //0                     //Steven 20231120 : 紀錄機台稼動時間
                                      "PowerOff",
                                      "Start",
                                      "Pause",
                                      "ContactTest",
                                      "Home",           //5
                                      "Soak",
                                      "SOT",
                                      "EOT",
                                      "OpenLot",
                                      "EndLot",         //10
                                      "NoteOn",
                                      "NoteOff",
                                      "MessageOn",
                                      "MessageOff",
                                      "InitialStart",   //15
                                      "TrayFeed"};

AnsiString asTempCtrl[tcTotalCount]={"Plate 1", "Plate 2", "SH 1", "SH 2",      //溫控器要一起改
                                     "Head 1/2", "Head 3/4", "Head 5/6", "Head 7/8",
                                     "Dut", "Chamber", "CCD",
                                     "A1", "C1", "E1", "G1",
                                     "B1", "D1", "F1", "H1",
                                     "A2", "C2", "E2", "G2",
                                     "B2", "D2", "F2", "H2",
                                     "Heat Gun1", "Heat Gun2",
                                     "DUT1","DUT2","DUT3","DUT4",               //wei 20150213 : fixed溫度畫面顯示異常
                                     "I1", "K1", "M1", "O1",                    //Steven 20140923 : Index使用EJ1N版32組加熱器
                                     "J1", "L1", "N1", "P1",
                                     "I2", "K2", "M2", "O2",
                                     "J2", "L2", "N2", "P2",
                                     "2D", "L/B","Index ESD","CCD1",            //kevin 20200210 add
                                     "ATC hot air1", "ATC hot air2",            //JerryYang 20220408 : add for ATC3.5
                                     "Out SH 1","Out SH2","Base1","Base2",      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                                     "Base3","Base4","Base5","Base6",           //Ztex 2023.04.19 Add HT-1032 TriTemp Function
                                     "HotPlate3", "HotPlate4", "SH 3", "SH 4",
                                     "Door1", "Door2",                          //Ztex 2023.10.23 Add Index Door Heater
                                     "L/B Top", "L/B Bottom"                    //Frank 20241231 : add
                                    };

int iTempCode[INDEX_HEAT_COUNT]=                                                //Jimmychiu 20230628 : 減少DTME加熱迴圈次數
{
    tcAa1, tcBa1, tcAb1, tcBb1, tcAc1, tcBc1, tcAd1, tcBd1,
    tcAa2, tcBa2, tcAb2, tcBb2, tcAc2, tcBc2, tcAd2, tcBd2,
    tcAe1, tcBe1, tcAf1, tcBf1, tcAg1, tcBg1, tcAh1, tcBh1,
    tcAe2, tcBe2, tcAf2, tcBf2, tcAg2, tcBg2, tcAh2, tcBh2,
};

const byte MyBitMask[8]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
TMyStringList *slEventLog;                                                      //Steven 20161115 : EventLog存成文字檔  //Steven 20200116 : 改成全域變數, 避免fMain被解構造成記憶體異常
TMyStringList *sl2DMappingLog;                                                  //JerryYang 20230322 : add 2D mapping result
TStringList *slBundlID;                                                         //JerryYang 20240318 : add
TStringList *slDupBundlID;                                                      //JerryYang 20240318 : add
TStringList *slDupUnloadBundlID;                                                //JerryYang 20240318 : add

//---------------------------------------------------------------------------
//AI(W0-TAIL) 20260626: TODO(W3) -- SaveEventLog uses slEventLog->MySaveToFile()
//  (TMyStringList, only fwd-declared) + RunInfo.slEventLogFile state.
#if 0 // TODO(W3): TMyStringList body + RunInfo state
void SaveEventLog()                                                             //Steven 20200416 : 整合EventLog存檔
{
    slEventLog->MySaveToFile();
    int iCount=RunInfo.slEventLogFile->Count;
    AnsiString FileName=slEventLog->GetFileName();

    if(iCount==0)
    {
        RunInfo.slEventLogFile->Add(FileName);
    }
    else
    {
        if(RunInfo.slEventLogFile->Text.AnsiPos(FileName)==0)
        {
            RunInfo.slEventLogFile->Add(FileName);
        }
    }
}
#endif // TODO(W3) SaveEventLog
//---------------------------------------------------------------------------
const int CY_PUSH=1;                                                            //Steven 20120822 : 沒用到,馬克掉
const int CY_POP=2;

const int NULL_IC                   =0;
const int HAS_TESTING_IC            =1;
const int HAS_IC                    =2;
const int HOTED                     =3;
const int HAS_HOT_IC                =4;
const int IS_ID_TRAY                =4;
const int HAS_NULL_IC               =5;
const int RotateOK                  =6;
const int HAS_CLEAN_IC              =7;
const int HAS_NULL_CLEAN_IC         =8;
const int CLEAN_FINISH_IC           =9;
const int HAS_TRY_SUCK_IC           =10;                                        //ChungHung 20120206 Hotplate check
const int HAS_SUCK_IC               =11;                                        //ChungHung 20120206 Hotplate check
const int HAS_OCR_OK                =12;                                        //ChungHung 20120830 add OCR Function
const int HAS_OCR_NG                =13;                                        //ChungHung 20120830 add OCR Function
const int HAS_OCR_Err               =14;                                        //ChungHung 20120830 add OCR Function
const int HAS_BARCODEERROR_IC       =16;                                        //Steven 20121009 : Bar Code
const int HAS_CLEAN_FINSH_IC        =17;                                        //kevin 20130226
const int HAS_SKIP_IC               =18;                                        //Q_Q 2010.11.08 統整 SKIP
const int HAS_CASSETTE_EMPTY        =19;                                        //wei 20180702 MR
const int HAS_CASSETTE_TRAY         =20;                                        //wei 20180702 MR
const int HAS_CASSETTE_DEVICE       =21;                                        //wei 20180702 MR
const int HAS_CASSETTE_FULLDEVICE   =22;                                        //wei 20180702 MR
const int HAS_CASSETTE_FULLTRAY     =23;                                        //wei 20180702 MR
const int HAS_CASSETTE_PASS         =24;                                        //wei 20180702 MR
const int HAS_CASSETTE_FAIL         =25;                                        //wei 20180702 MR
const int HAS_NewLot_IC             =26;                                        //wei 20180529
const int WAIT_ALIGN_IC             =27;                                        //ChungHung 20210113 add for Alignment CCD   //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
const int WAIT_TRAYMAP_IC           =28;                                        //KenHsieh 20220923 : add Tray Map Throw IC Function

AnsiString sIC_Type[29]={"NULL_IC",
                         "HAS_TESTING_IC",
                         "HAS_IC",
                         "HOTED",
                         "HAS_HOT_IC",
                         "HAS_NULL_IC",
                         "RotateOK",
                         "HAS_CLEAN_IC",
                         "HAS_NULL_CLEAN_IC",
                         "CLEAN_FINISH_IC",
                         "HAS_TRY_SUCK_IC",
                         "HAS_SUCK_IC",
                         "HAS_OCR_OK",
                         "HAS_OCR_NG",
                         "HAS_OCR_Err",
                         "HAS_BARCODEERROR_IC",
                         "HAS_CLEAN_FINSH_IC",
                         "HAS_SKIP_IC",
                         "HAS_CASSETTE_EMPTY",
                         "HAS_CASSETTE_TRAY",
                         "HAS_CASSETTE_DEVICE",
                         "HAS_CASSETTE_FULLDEVICE",
                         "HAS_CASSETTE_FULLTRAY",
                         "HAS_CASSETTE_PASS",
                         "HAS_CASSETTE_FAIL",
                         "HAS_NewLot_IC",
                         "WAIT_ALIGN_IC",
                         "WAIT_TRAYMAP_IC"
                        };
//------------------------------------------------------------------------------
const bool bAutoPick =false;
const bool bAutoPlace=true;                                                     //kevin 20220916 change type int=> bool
//TTL Mode-------
const int _4Bit=0;
const int _8Bit=1;
const int _5BitPE=2;
const int _10BitPE=3;
const int _5Bit=4;
const int _10Bit=5;
const int _5BitPO=6;
const int _10BitPO=7;
const int _3Bit=8;

const int TC401   =0;
const int KT4H    =1;
const int E5DC    =2;
const int NoHeater=3;                                                           //Steven 20171227 (Wei) : Add for HT-9045L
const int DTK4848 =4;                                                           //KaiHuang 20190821 : 新增台達 DTK4848溫控器
int TC401HeaterControl=1;                                                       //Steven 20141030 : 新增OMRON E5DC溫控器

const int LogicPositive=0;
const int LogicNegative=1;

const int CHEach=0;
const int CHSingle=1;

const int DUTNONE=0;
const int DUTPosLevel=1;
const int DUTPosPluse=2;
const int DUTNegLevel=3;
const int DUTNegPluse=4;

const int CHOneByOne=0;
const int CHBinary=1;

//---------------
const int GaliPosOffSet=3000;                                                   //Wei 2014101 : 2000 --> 3000

long GALI_ERROR_MAX_PR1=0;                                                      //Chunghung 20131111 add
long GALI_ERROR_MAX_PR2=0;                                                      //Chunghung 20131111 add
long GALI_ERROR_MAX_PR3=0;                                                      //Chunghung 20131111 add
long GALI_ERROR_MAX_PR4=0;                                                      //Chunghung 20131111 add

const int MotErrPos=6;

// -----------------------------------------------------------------------------
const int Vaccum_On=0;
const int Vaccum_Off=1;
const int Vaccum_FallDown=2;
const int Vaccum_Initial_On=3;
const int Vaccum_Initial_Off=4;
//------------------------------------------------------------------------------
const int REALLY     =2;
const int HAS_TRAY   =1;
const int DUMMY      =0;
// -----------------------------------------------------------------------------
const int TYPE_A=1;
const int TYPE_B=0;
// -----------------------------------------------------------------------------
const int START_TEST =1000;
const int TEST_PASS  =1001;
const int TEST_FAIL1 =1002;
const int TEST_FAIL2 =1003;
const int TEST_FAIL3 =1004;
const int TEST_FAIL4 =1005;
const int TEST_FAIL5 =1006;
const int TEST_FAIL6 =1007;
const int TEST_FAIL7 =1008;
const int TEST_FAIL8 =1009;
const int SEND       =3000;
const int RECEIVE    =3001;
// -----------------------------------------------------------------------------
bool InitialOK=false;
bool SystemStart=false;
bool fAllMotorHome=false;
bool SoftStart=false;
bool SoftStop=false;
bool bPhysicalStart=false;                                                      //Steven 20141006 : SECS GEM使用Remote Start功能
Word SystemHour=9999, SystemMin=9999, SystemSec=9999, SystemMSec=9999;
Word SystemYear=9999, SystemMonth=9999, SystemDate;
Word SystemYearYesterday=9999, SystemMonthYesterday=9999, SystemDateYesterday=9999;
AnsiString CurrentDir;

bool bDoOCRFunction=false;                                                      //ChungHung 20121002 add OCR Function
bool bWaitTesterFinish=false;                                                   //ChungHung 20140716 add if testing not finish can not homing
int  iAMRCoverTray=0;                                                           //Eastsun 20260515 F009 整合: KYEC AMR Cover Tray 計數

AnsiString Com2Buffer;                                                          //Steven 20111028 : 改成AnsiString
AnsiString ComOmronBuffer;                                                      //Steven 20120220 : Omron EJ1N溫控器
bool Com2ReceiveOK=false;
bool ComOmronReceiveOK=false;                                                   //Steven 20120220 : Omron EJ1N溫控器

bool bATCInitialFinish=false;                                                   //jou 2012-03-16 ATC Initial 完成後才能開始Read/Write溫度值
int  iATCInitialTask[tcTotalCount];                                             //jou 2012-03-16 ATC Initial Task
int  iATCProcessTask[tcTotalCount];
bool bATCWriteCommand[tcTotalCount]={0};                                        //jou 2012-03-16 ATC Write=true / Read=false
bool bATCReceiveErr[tcTotalCount]={0};                                          //jou 2012-03-16 ATC 下指令回傳錯誤植Err=true
bool ComATCReceiveOK[tcTotalCount]={0};                                         //jou 2012-03-16 ATC 下指令後COM PORT回應Flag
double fATCReadBuffer[tcTotalCount]={0};                                        //jou 2012-03-16 ATC 回傳的溫度值
AnsiString asATCErrorString[tcTotalCount];                                      //jou 2012-03-16 ATC Error Ccde資訊紀錄

int  iOneCycle  =0;
int  iCleanOut  =0;
int  iFixOneCycle=0;                                                            //kevin 20130312 onecycle 兩支arm讓開維修

int  iTrayFeed  =0;
bool bAMRFullICBin=false;                                                       //a-side cmydef.cpp L5432  //Eastsun 20260515 F011 整合 (Phase4-F5T3 KYEC AMR FullTray flag)
bool bSameSetupFileNoDownload=false;                                            //Eastsun 20260515 F011 整合 (Phase4-F4T4 def bSameSetupFileNoDownload)
AnsiString asColorCoverTrayID="";                                               //Eastsun 20260515 F011 整合 (Phase4-F5T2 def asColorCoverTrayID)
int  iHome      =1;
int  iReset     =0;
int  iAlarmReset=0;

int  iCatchTrayControlManual=0;
int  OutArmPlaceToManual=0;
bool bAtuoTrayICDetectErr=false;                                                //Isaac 20180109 (Steven) : auto123可前進後退
bool bLoaderTrayICDetectErr=false;                                              //Sam 20200316 : Loader Detect Tray
int  HotTime[2][50][50];
int  iRowOnHotPlate[2][50][50];                                                 //JerryYang 20180718 (wei) : 放料至hot plate記錄吸嘴位置
bool SystemInitialOK=false;
int  iHotLineChange=0;
bool  bOneCycle_BackUp=false;                                                   //ChungHung 20141111 add for SCK junction temp issue  //JerryYang 20161129 iOneCycle_BackUp改成bool
bool  bRS232Delay=false;                                                        //RogerYang 20180901 add 矽格湖口Demo AI CCD Function

const int  K_RETRY    =0x0001;
const int  K_SKIP     =0x0002;
const int  K_CLEAN_OUT=0x0004;
const int  K_TRAY_FEED=0x0008;
const int  K_TRAY_END =0x0010;
const int  K_RESET    =0x0020;
const int  K_HOME     =0x0040;                                                  //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
const int  K_TRAIN    =0x0080;
const int  K_FIX      =0x0100;                                                  //kevin 20130218 onecycle 兩支arm讓開維修
const int  K_ONECYCLE =0x0200;                                                  //ChungHung 20140730 add ContinuousFailHaveOneCycle
const int  K_PAUSE    =0x0400;                                                  //Steven 20150722 : Add按鍵Type - Pause
const int  K_START    =0x0800;                                                  //Steven 20150722 : Add按鍵Type - Start

const int  N_INTEGER   =0x0001;                                                 //只有整數
const int  N_DOUBLE    =0x0002;                                                 //只有浮點數
const int  N_NO_SYMBOL =0x0004;                                                 //沒有特殊符號
const int  N_PASSWORD  =0x0008;                                                 //密碼文
const int  N_NO_SPACE  =0x0010;                                                 //無空白鍵
const int  N_UPPERCASE =0x0020;                                                 //大寫優先
const int  N_NO_NUM_PAD=0x0040;                                                 //不需要數字鍵
const int  N_PORT      =0x0080;                                                 //通訊埠
const int  N_IP_ADDR   =0x0100;                                                 //IP位置

bool CheckSystemPower=true;

//====================================================================
const int C_TrayZ_Selector              =0;
const int C_TrayY_Fixer                 =1;
const int C_Auto1Side_Fixer             =2;
const int C_Auto2Side_Fixer             =3;
const int C_Auto3Side_Fixer             =4;
const int C_Auto1_Selector              =5;                                     //Auto 1上頂汽缸 或 中間分離
const int C_Auto2_Selector              =6;                                     //Auto 2上頂汽缸 或 中間分離
const int C_Auto3_Selector              =7;                                     //Auto 3上頂汽缸 或 中間分離
const int C_TrayX_UpDown                =8;
const int C_EmptyLoaderZ_Select         =9;
const int C_ColorLoaderZ_Select         =10;
const int C_Empty_Fix                   =11;
const int C_TrayCover                   =12;                                    //Steven 20140409 : Auto Retest
const int C_CatchTray_Fix               =13;
const int C_Color_Fix                   =14;
const int C_LoaderEdgePush              =15;
const int C_Auto1EdgePush               =16;
const int C_Auto2EdgePush               =17;
const int C_Auto3EdgePush               =18;
const int C_Load_Up                     =19;
const int C_Load_Middle                 =20;
const int C_Color_Up                    =21;
const int C_Color_Middle                =22;
const int C_Empty_Up                    =23;
const int C_Empty_Middle                =24;
const int C_TrayVibration               =25;                                    //2011.01.19 Q_Q V207C 敲擊功能的改寫。
const int C_HotplateVibration           =26;                                    //jou 2011-08-09 Hotplate也要敲敲敲。
const int C_CoolingValve                =27;                                    //20111130  Dell
const int C_Auto1_Up                    =28;                                    //Auto 1上頂汽缸
const int C_Auto2_Up                    =29;                                    //Auto 2上頂汽缸
const int C_Auto3_Up                    =30;                                    //Auto 3上頂汽缸
const int C_Auto1LoaderZ_Select         =31;                                    //Auto 1分離汽缸   kevin 20120718 代號不能改
const int C_Auto2LoaderZ_Select         =32;                                    //Auto 2分離汽缸
const int C_Auto3LoaderZ_Select         =33;                                    //Auto 3分離汽缸
const int C_Fix1LoaderZ_Select          =34;                                    //Fix 1分離汽缸   kevin 20120718 代號不能改
const int C_Fix2LoaderZ_Select          =35;                                    //Fix 2分離汽缸
const int C_Fix3LoaderZ_Select          =36;                                    //Fix 3分離汽缸
const int C_Auto2_Middle                =37;                                    //kevin 20120725
const int C_Shuttle_Knocker_1           =38;                                    //Shuttle敲敲
const int C_Shuttle_Knocker_2           =39;                                    //Shuttle敲敲
const int C_InputRotateKIT              =40;                                    //Steven 20121001 : 旋轉Kit
const int C_OutputRotateKIT             =41;                                    //Steven 20121001 : 旋轉Kit
const int C_FixTray_FullPlace           =42;                                    //Steven 20140310 : Fix3滿盤氣缸
const int C_DockYAxisOn                 =43;                                    //Steven 20140310 : One Touch Docking
const int C_DockYAxisOff                =44;                                    //Steven 20140310 : One Touch Docking
const int C_DockXAxisOn                 =45;                                    //Steven 20140310 : One Touch Docking
const int C_DockXAxisOff                =46;                                    //Steven 20140310 : One Touch Docking
const int C_CatchTray_FixOn             =47;                                    //ChungHung 20140624 : Auto Retest
const int C_CatchTray_FixOff            =48;                                    //ChungHUng 20140624 : Auto Retest
const int C_TurnTrayArm                 =49;                                    //ChungHung 20140701 : AutoRetest
const int C_TurnTrayArmLock             =50;                                    //ChungHung 20140814 : AutoRetest
const int C_OCRLight_Up                 =51;                                    //wei 20150720 OCR觸發
const int C_OCRLight_Down               =52;                                    //wei 20150720 OCR觸發
const int C_SLK1_Clamp                  =53;                                    //JerryYang 20160524
const int C_SLK1_Unclamp                =54;                                    //JerryYang 20160524
const int C_SLK2_Clamp                  =55;                                    //JerryYang 20160524
const int C_SLK2_Unclamp                =56;                                    //JerryYang 20160524
const int C_Socket_Clamp                =57;                                    //JerryYang 20160524
const int C_Socket_Unclamp              =58;                                    //JerryYang 20160524
const int C_LoaderUpPress               =59;                                    //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray

const int C_HingeLookOn                 =60;                                    //wei 20170418 Hinge cassette
const int C_HingeLookOff                =61;                                    //wei 20170418 Hinge cassette
//2
const int C_CassetteArmCatchOn          =62;                                    //wei 20180702 MR
const int C_CassetteArmCatchOff         =63;                                    //wei 20180702 MR
//1
const int C_LoadPortYOn                 =64;                                    //wei 20180702 MR
const int C_LoadPortYOff                =65;                                    //wei 20180702 MR
//0
const int C_LoadPortCatchOn             =66;                                    //wei 20180702 MR
const int C_LoadPortCatchOff            =67;                                    //wei 20180702 MR
//3
const int C_TrayBracketUpOn             =68;                                    //wei 20180702 MR
const int C_TrayBracketUpOff            =69;                                    //wei 20180702 MR
//4
const int C_TrayBracketOpenOn           =70;                                    //wei 20180702 MR
const int C_TrayBracketOpenOff          =71;                                    //wei 20180702 MR
//5
const int C_StackedTrayCatchOn          =72;                                    //wei 20180702 MR
const int C_StackedTrayCatchOff         =73;                                    //wei 20180702 MR
//6
const int C_StackedTrayLockOn           =74;                                    //wei 20180702 MR
const int C_StackedTrayLockOff          =153;                                   //wei 20180702 MR //AI(W906-CYL75) 20260926: 75→153。golden 缺陷：與 :448 C_LoadRobotX 同為 75，InitialCylinderName（cinitial.cpp:7793/:7794）後寫者勝，本缸無名無 IO；使用者裁決修正（RULINGS_20260926 第 11 條）。153＝HT1032 已註解的 C_OutArmBh 空槽（137..153 無任何常數），MaxCylinderItem(295) 不動

const int C_LoadRobotX                  =75;                                    //Sam 20190112 LM
const int C_UnloadRobotX                =76;                                    //Sam 20190112 LM

const int C_Auto1UpPress                =77;                                    //JerryYang 20190423 新增unloader壓tray
const int C_Auto2UpPress                =78;                                    //JerryYang 20190423 新增unloader壓tray
const int C_Auto3UpPress                =79;                                    //JerryYang 20190423 新增unloader壓tray

const int C_InFlipper1                  =80;                                    //Frank 20210612 : Flipper Function //==>
const int C_InFlipper1Lock              =81;
const int C_InFlipper2                  =82;
const int C_InFlipper2Lock              =83;
const int C_InFlipper3                  =84;
const int C_InFlipper3Lock              =85;
const int C_OutFlipper1                 =86;
const int C_OutFlipper1Lock             =87;
const int C_OutFlipper2                 =88;
const int C_OutFlipper2Lock             =89;
const int C_OutFlipper3                 =90;
const int C_OutFlipper3Lock             =91;                                    //Frank 20210612 : Flipper Function //<==

const int C_LoaderCasstteLock           =92;
const int C_EmptyCasstteLock            =93;
const int C_ColorCasstteLock            =94;
const int C_Auto1CasstteLock            =95;
const int C_Auto2CasstteLock            =96;
const int C_Auto3CasstteLock            =97;

const int C_TeachGlassUp                =98;
const int C_Shuttle1Precisor            =99;
const int C_Shuttle2Precisor            =100;

const int C_MultileEmptyY_On            =101;                                   //KaiChen 20200716 ：OHT ==>
const int C_MultileEmptyY_Off           =102;

const int C_MultileEmptyCatch_On        =103;
const int C_MultileEmptyCatch_Off       =104;

const int C_MultileEmptyBracketUp_On    =105;
const int C_MultileEmptyBracketUp_Off   =106;

const int C_MultileEmptyCornerPush      =107;
const int C_MultileEmptyLoaderZ_Select  =108;
const int C_MultileEmptyZ_Select        =109;

const int C_TrayBracket2UpOn            =110;
const int C_TrayBracket2UpOff           =111;

const int C_MultileEmptyLock            =112;

const int C_TrayBracketOpen2On          =113;
const int C_TrayBracketOpen2Off         =114;                                   //KaiChen 20200716 ：OHT <==

const int C_CSTHoldDown                 =115;
const int C_CSTHoldDown2                =116;
const int C_LoaderCarEdgePush           =117;                                   //JimmyChiu 20220408 For Laser Scan

const int C_InAreaAlignment             =118;                                   //ChungHung 20210113 add for Alignment CCD    //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
const int C_OutAreaAlignment            =119;                                   //ChungHung 20210113 add for Alignment CCD     //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
const int C_PlacementArm                =120;                                   //JimmyChiu 20220908 add Pickup Error Placement
const int C_TesterSidePush              =121;                                   //Richard 20220321 : 渠梁Side Push

const int C_Load2CasstteLock            =122;
const int C_Load2_Middle                =123;
const int C_Load2_Up                    =124;
const int C_Tray2Z_Selector             =125;
const int C_Load2UpPress                =126;
const int C_Tray2Y_Fixer                =127;
const int C_Load2EdgePush               =128;
const int C_Load2TrackFloodgate         =129;
const int C_Load2PushBack_Push          =130;
const int C_Load2PushBack_Back          =131;
const int C_Load2Separate               =132;
const int C_Load2SeparateRL             =133;
const int C_Load2SeparateRR             =134;
const int C_Load2SeparateFL             =135;
const int C_Load2SeparateFR             =136;                                   //Steven 20240822 : For HT-9046AU

/*const int C_InArmAa                     =122;   //Steven for HT1032
const int C_InArmBa                     =123;
const int C_InArmAb                     =124;
const int C_InArmBb                     =125;
const int C_InArmAc                     =126;
const int C_InArmBc                     =127;
const int C_InArmAd                     =128;
const int C_InArmBd                     =129;
const int C_InArmAe                     =130;
const int C_InArmBe                     =131;
const int C_InArmAf                     =132;
const int C_InArmBf                     =133;
const int C_InArmAg                     =134;
const int C_InArmBg                     =135;
const int C_InArmAh                     =136;
const int C_InArmBh                     =137;
const int C_OutArmAa                    =138;
const int C_OutArmBa                    =139;
const int C_OutArmAb                    =140;
const int C_OutArmBb                    =141;
const int C_OutArmAc                    =142;
const int C_OutArmBc                    =143;
const int C_OutArmAd                    =144;
const int C_OutArmBd                    =145;
const int C_OutArmAe                    =146;
const int C_OutArmBe                    =147;
const int C_OutArmAf                    =148;
const int C_OutArmBf                    =149;
const int C_OutArmAg                    =150;
const int C_OutArmBg                    =151;
const int C_OutArmAh                    =152;
const int C_OutArmBh                    =153;  */

const int C_LoadTrackFloodgate          =154;                                   //Ztex 2023.04.13 Add HT-1032 IO ==>
const int C_EmptyTrackFloodgate         =155;
const int C_ColorTrackFloodgate         =156;
const int C_Auto1TrackFloodgate         =157;
const int C_Auto2TrackFloodgate         =158;
const int C_Auto3TrackFloodgate         =159;
const int C_SafeDoor1Lock               =160;
const int C_SafeDoor2Lock               =161;
const int C_SafeDoor3Lock               =162;
const int C_SafeDoor4Lock               =163;
const int C_SafeDoor5Lock               =164;
const int C_SafeDoor6Lock               =165;
const int C_SafeDoor7Lock               =166;
const int C_SafeDoor8Lock               =167;                                   //Ztex 2023.04.13 Add HT-1032 IO <==

const int C_LoaderPushBack_Push         =168;                                   //Frank 20230419
const int C_EmptyPushBack_Push          =169;
const int C_ColorPushBack_Push          =170;
const int C_Auto1PushBack_Push          =171;
const int C_Auto2PushBack_Push          =172;
const int C_Auto3PushBack_Push          =173;
const int C_LoaderSeparate              =174;
const int C_EmptySeparate               =175;
const int C_ColorSeparate               =176;
const int C_Auto1Separate               =177;
const int C_Auto2Separate               =178;
const int C_Auto3Separate               =179;

const int C_UnderTrayArmYCatch          =180;
const int C_Shuttle1Floodgate           =181;
const int C_Shuttle2Floodgate           =182;

const int C_EnhaustAirVentOpen          =183;                                   //Ztex 2023.04.26 Add HT-1032 IO Exhaust Air ==>
const int C_EnhaustAirVentClose         =184;
const int C_LUpEnhaustAirOpen           =185;
const int C_LUpEnhaustAirClose          =186;
const int C_RUpEnhaustAirOpen           =187;
const int C_RUpEnhaustAirClose          =188;                                   //Ztex 2023.04.26 Add HT-1032 IO Exhaust Air <==

const int C_CatchMagazineTray           =189;                                   //JerryYang 20220909 : add magazine
const int C_CatchMagazineTray1          =190;
const int C_Auto3BlockZ                 =191;
const int C_MagYTrayOut                 =192;
const int C_TrayXFloodgate1             =193;
const int C_TrayXFloodgate2             =194;
const int C_TrayXFloodgate3             =195;
const int C_TrayXFloodgate4             =196;
const int C_FixTray_UpDown              =197;
const int C_LoaderPushBack_Back         =198;                                   //Frank 20230419
const int C_EmptyPushBack_Back          =199;
const int C_ColorPushBack_Back          =200;
const int C_Auto1PushBack_Back          =201;
const int C_Auto2PushBack_Back          =202;
const int C_Auto3PushBack_Back          =203;

const int C_Auto4Side_Fixer             =204;                                   //Steven 20230907 : For HT-9011UC //==>
const int C_Auto5Side_Fixer             =205;
const int C_Auto6Side_Fixer             =206;
const int C_Auto4_Selector              =207;                                   //Auto 4上頂汽缸 或 中間分離
const int C_Auto5_Selector              =208;                                   //Auto 5上頂汽缸 或 中間分離
const int C_Auto6_Selector              =209;                                   //Auto 6上頂汽缸 或 中間分離
const int C_Auto4EdgePush               =210;
const int C_Auto5EdgePush               =211;
const int C_Auto6EdgePush               =212;
const int C_Auto4_Up                    =213;                                   //Auto 4上頂汽缸
const int C_Auto5_Up                    =214;                                   //Auto 5上頂汽缸
const int C_Auto6_Up                    =215;                                   //Auto 6上頂汽缸
const int C_Auto4LoaderZ_Select         =216;                                   //Auto 4分離汽缸
const int C_Auto5LoaderZ_Select         =217;                                   //Auto 5分離汽缸
const int C_Auto6LoaderZ_Select         =218;                                   //Auto 6分離汽缸
const int C_Fix4LoaderZ_Select          =219;                                   //Fix 4分離汽缸
const int C_Fix5LoaderZ_Select          =220;                                   //Fix 5分離汽缸
const int C_Fix6LoaderZ_Select          =221;                                   //Fix 6分離汽缸
const int C_Auto4UpPress                =222;                                   //Auto 4unloader壓tray
const int C_Auto5UpPress                =223;                                   //Auto 5unloader壓tray
const int C_Auto6UpPress                =224;                                   //Auto 6unloader壓tray
const int C_Auto4CasstteLock            =225;
const int C_Auto5CasstteLock            =226;
const int C_Auto6CasstteLock            =227;
const int C_Auto4TrackFloodgate         =228;
const int C_Auto5TrackFloodgate         =229;
const int C_Auto6TrackFloodgate         =230;
const int C_Auto4PushBack_Push          =231;
const int C_Auto5PushBack_Push          =232;
const int C_Auto6PushBack_Push          =233;
const int C_Auto4Separate               =234;
const int C_Auto5Separate               =235;
const int C_Auto6Separate               =236;
const int C_Auto4PushBack_Back          =237;
const int C_Auto5PushBack_Back          =238;
const int C_Auto6PushBack_Back          =239;

const int C_LoaderSeparateRL            =240;
const int C_LoaderSeparateRR            =241;
const int C_LoaderSeparateFL            =242;
const int C_LoaderSeparateFR            =243;
const int C_EmptySeparateRL             =244;
const int C_EmptySeparateRR             =245;
const int C_EmptySeparateFL             =246;
const int C_EmptySeparateFR             =247;
const int C_ColorSeparateRL             =248;
const int C_ColorSeparateRR             =249;
const int C_ColorSeparateFL             =250;
const int C_ColorSeparateFR             =251;
const int C_Auto1SeparateRL             =252;
const int C_Auto1SeparateRR             =253;
const int C_Auto1SeparateFL             =254;
const int C_Auto1SeparateFR             =255;
const int C_Auto2SeparateRL             =256;
const int C_Auto2SeparateRR             =257;
const int C_Auto2SeparateFL             =258;
const int C_Auto2SeparateFR             =259;
const int C_Auto3SeparateRL             =260;
const int C_Auto3SeparateRR             =261;
const int C_Auto3SeparateFL             =262;
const int C_Auto3SeparateFR             =263;
const int C_EmptyEdgePush               =264;
const int C_ColorEdgePush               =265;
const int C_Auto4SeparateRL             =266;
const int C_Auto4SeparateRR             =267;
const int C_Auto4SeparateFL             =268;
const int C_Auto4SeparateFR             =269;
const int C_Auto5SeparateRL             =270;
const int C_Auto5SeparateRR             =271;
const int C_Auto5SeparateFL             =272;
const int C_Auto5SeparateFR             =273;
const int C_Auto6SeparateRL             =274;
const int C_Auto6SeparateRR             =275;
const int C_Auto6SeparateFL             =276;
const int C_Auto6SeparateFR             =277;                                   //Steven 20230907 : For HT-9011UC //<==
const int C_OutShuttle1Floodgate        =278;                                   //Ifor 20240620 add:Out Shuttle Floodgate
const int C_OutShuttle2Floodgate        =279;                                   //Ifor 20240620 add:Out Shuttle Floodgate
const int C_LoadCarRFIDRotArmD          =280;                                   //RogerYang 20250828 add for Loader Rotate Arm
const int C_LoadCarRFIDRotArmU          =281;                                   //RogerYang 20250828 add for Loader Rotate Arm
const int C_LoadTrayDetD                =282;                                   //RogerYang 20250828 add for 殘料檢氣缸
const int C_LoadTrayDetU                =283;                                   //RogerYang 20250828 add for 殘料檢氣缸
const int C_LoadTrayDetF                =284;                                   //RogerYang 20250828 add for 殘料檢氣缸
const int C_LoadTrayDetB                =285;                                   //RogerYang 20250828 add for 殘料檢氣缸
const int C_FixedSeatTL                 =286;                                   //Jimmychiu 20240322 : Top & Bottom Inspect
const int C_FixedSeatTR                 =287;
const int C_FixedSeatBL                 =288;
const int C_FixedSeatBR                 =289;
const int C_TopBtmRotateLock            =290;

const int C_LoaderCarrier               =291;                                   //Ifor 20251216 add:Boat Carrier
const int C_Auto1Carrier                =292;                                   //Ifor 20251216 add:Boat Carrier
const int C_Auto2Carrier                =293;                                   //Ifor 20251216 add:Boat Carrier
const int C_DailyCorrelation            =294;    //KaiChen 20200525 ：Daily Correlation Function

//#define MaxCylinderItem 295  <-- 記得去改!!

int CynNeedHome[CynForHome]={   C_Load_Up,          C_Load_Middle,      C_TrayZ_Selector,       C_TrayY_Fixer,       C_LoaderEdgePush,       // 0~ 4    //Steven 20240123 : 改用enable確認氣缸是否要復歸
                                C_Empty_Up,         C_Empty_Middle,     C_EmptyLoaderZ_Select,  C_Empty_Fix,         C_EmptyEdgePush,        // 5~ 9
                                C_Color_Up,         C_Color_Middle,     C_ColorLoaderZ_Select,  C_Color_Fix,         C_ColorEdgePush,        //10~14
                                C_Auto1_Up,         C_Auto1_Selector,   C_Auto1LoaderZ_Select,  C_Auto1Side_Fixer,   C_Auto1EdgePush,        //15~19
                                C_Auto2_Up,         C_Auto2_Selector,   C_Auto2LoaderZ_Select,  C_Auto2Side_Fixer,   C_Auto2EdgePush,        //20~24
                                C_Auto3_Up,         C_Auto3_Selector,   C_Auto3LoaderZ_Select,  C_Auto3Side_Fixer,   C_Auto3EdgePush,        //25~29
                                C_Auto4_Up,         C_Auto4_Selector,   C_Auto4LoaderZ_Select,  C_Auto4Side_Fixer,   C_Auto4EdgePush,        //30~34
                                C_Auto5_Up,         C_Auto5_Selector,   C_Auto5LoaderZ_Select,  C_Auto5Side_Fixer,   C_Auto5EdgePush,        //35~39
                                C_Auto6_Up,         C_Auto6_Selector,   C_Auto6LoaderZ_Select,  C_Auto6Side_Fixer,   C_Auto6EdgePush,        //40~44
                                C_TrayX_UpDown,     C_TurnTrayArm,      C_Shuttle_Knocker_1,    C_Shuttle_Knocker_2, C_TrayXFloodgate1,      //45~49
                                C_TrayXFloodgate2,  C_TrayXFloodgate3,  C_TrayXFloodgate4,      C_Load2_Up,          C_Load2_Middle,         //50~54
                                C_Tray2Z_Selector,  C_Tray2Y_Fixer,     C_Load2EdgePush,        C_LoadCarRFIDRotArmD,C_LoadCarRFIDRotArmU,   //55~59    //RogerYang 20250828 add for Loader Rotate Arm
                                C_LoadTrayDetD,     C_LoadTrayDetU,     C_LoadTrayDetF,         C_LoadTrayDetB      ,C_LoaderSeparate,       //60~64    //RogerYang 20250722 Add for 9046AU
                                C_Load2Separate,    C_EmptySeparate,    C_ColorSeparate,        C_Auto1Separate     ,C_Auto2Separate,        //65~69    //RogerYang 20250722 Add for 9046AU
                                C_Auto3Separate,    C_Auto4Separate,    C_Auto5Separate,        C_Auto6Separate                              //70~74    //RogerYang 20250722 Add for 9046AU
                            };
//#define CynForHome 74       <-- 記得去改!!

//Steven 20230907 : For HT-9011UC
//==>
int C_AutoSide_Fixer    [MAX_AUTO_TRAY]={C_Auto1Side_Fixer,     C_Auto2Side_Fixer,     C_Auto3Side_Fixer,     C_Auto4Side_Fixer,     C_Auto5Side_Fixer,     C_Auto6Side_Fixer     };
int C_Auto_Selector     [MAX_AUTO_TRAY]={C_Auto1_Selector,      C_Auto2_Selector,      C_Auto3_Selector,      C_Auto4_Selector,      C_Auto5_Selector,      C_Auto6_Selector      };
int C_AutoEdgePush      [MAX_AUTO_TRAY]={C_Auto1EdgePush,       C_Auto2EdgePush,       C_Auto3EdgePush,       C_Auto4EdgePush,       C_Auto5EdgePush,       C_Auto6EdgePush       };
int C_Auto_Up           [MAX_AUTO_TRAY]={C_Auto1_Up,            C_Auto2_Up,            C_Auto3_Up,            C_Auto4_Up,            C_Auto5_Up,            C_Auto6_Up            };
int C_AutoZ_Select      [MAX_AUTO_TRAY]={C_Auto1LoaderZ_Select, C_Auto2LoaderZ_Select, C_Auto3LoaderZ_Select, C_Auto4LoaderZ_Select, C_Auto5LoaderZ_Select, C_Auto6LoaderZ_Select };
int C_AutoUpPress       [MAX_AUTO_TRAY]={C_Auto1UpPress,        C_Auto2UpPress,        C_Auto3UpPress,        C_Auto4UpPress,        C_Auto5UpPress,        C_Auto6UpPress        };
int C_AutoCasstteLock   [MAX_AUTO_TRAY]={C_Auto1CasstteLock,    C_Auto2CasstteLock,    C_Auto3CasstteLock,    C_Auto4CasstteLock,    C_Auto5CasstteLock,    C_Auto6CasstteLock    };
int C_AutoTrackFloodgate[MAX_AUTO_TRAY]={C_Auto1TrackFloodgate, C_Auto2TrackFloodgate, C_Auto3TrackFloodgate, C_Auto4TrackFloodgate, C_Auto5TrackFloodgate, C_Auto6TrackFloodgate };
int C_AutoPushBack_Push [MAX_AUTO_TRAY]={C_Auto1PushBack_Push,  C_Auto2PushBack_Push,  C_Auto3PushBack_Push,  C_Auto4PushBack_Push,  C_Auto5PushBack_Push,  C_Auto6PushBack_Push  };
int C_AutoSeparate      [MAX_AUTO_TRAY]={C_Auto1Separate,       C_Auto2Separate,       C_Auto3Separate,       C_Auto4Separate,       C_Auto5Separate,       C_Auto6Separate       };
int C_AutoPushBack_Back [MAX_AUTO_TRAY]={C_Auto1PushBack_Back,  C_Auto2PushBack_Back,  C_Auto3PushBack_Back,  C_Auto4PushBack_Back,  C_Auto5PushBack_Back,  C_Auto6PushBack_Back  };

int C_AutoCarrier       [2]={C_Auto1Carrier, C_Auto2Carrier};                   //RogerYang 20260202 : Add for CR

AnsiString sJAM1101     [MAX_AUTO_TRAY]={"JAM1101", "JAM1201", "JAM1301", "JAM2501", "JAM2601", "JAM2701"};
AnsiString sJAM1102     [MAX_AUTO_TRAY]={"JAM1102", "JAM1202", "JAM1302", "JAM2502", "JAM2602", "JAM2702"};
AnsiString sJAM1103     [MAX_AUTO_TRAY]={"JAM1103", "JAM1203", "JAM1303", "JAM2503", "JAM2603", "JAM2703"};
AnsiString sJAM1104     [MAX_AUTO_TRAY]={"JAM1104", "JAM1204", "JAM1304", "JAM2504", "JAM2604", "JAM2704"};
AnsiString sJAM1106     [MAX_AUTO_TRAY]={"JAM1106", "JAM1206", "JAM1306", "JAM2506", "JAM2606", "JAM2706"};
AnsiString sJAM1107     [MAX_AUTO_TRAY]={"JAM1107", "JAM1207", "JAM1307", "JAM2507", "JAM2607", "JAM2707"};
AnsiString sJAM1108     [MAX_AUTO_TRAY]={"JAM1108", "JAM1208", "JAM1308", "JAM2508", "JAM2608", "JAM2708"};
AnsiString sJAM1109     [MAX_AUTO_TRAY]={"JAM1109", "JAM1209", "JAM1309", "JAM2509", "JAM2609", "JAM2709"};
AnsiString sJAM1110     [MAX_AUTO_TRAY]={"JAM1110", "JAM1210", "JAM1310", "JAM2510", "JAM2610", "JAM2710"};
AnsiString sJAM1111     [MAX_AUTO_TRAY]={"JAM1111", "JAM1211", "JAM1311", "JAM2511", "JAM2611", "JAM2711"};
AnsiString sJAM1112     [MAX_AUTO_TRAY]={"JAM1112", "JAM1212", "JAM1312", "JAM2512", "JAM2612", "JAM2712"};
AnsiString sJAM1113     [MAX_AUTO_TRAY]={"JAM1113", "JAM1213", "JAM1313", "JAM2513", "JAM2613", "JAM2713"};
AnsiString sJAM1114     [MAX_AUTO_TRAY]={"JAM1114", "JAM1214", "JAM1314", "JAM2514", "JAM2614", "JAM2714"};
AnsiString sJAM1158     [MAX_AUTO_TRAY]={"JAM1158", "JAM1258", "JAM1358", "JAM2558", "JAM2658", "JAM2758"};  //AI(general) 20260323 (RogerYang) : UpSafe sensor 未到位5秒 Alarm
AnsiString sWAR1116     [MAX_AUTO_TRAY]={"WAR1116", "WAR1216", "WAR1316", "WAR2516", "WAR2616", "WAR2716"};
AnsiString sMES1117     [MAX_AUTO_TRAY]={"MES1117", "MES1217", "MES1317", "MES2517", "MES2617", "MES2717"};
AnsiString sMES1120     [MAX_AUTO_TRAY]={"MES1120", "MES1220", "MES1320", "MES2520", "MES2620", "MES2720"};
AnsiString sMES1121     [MAX_AUTO_TRAY]={"MES1121", "MES1221", "MES1321", "MES2521", "MES2621", "MES2721"};
AnsiString sMES1122     [MAX_AUTO_TRAY]={"MES1122", "MES1222", "MES1322", "MES2522", "MES2622", "MES2722"};
AnsiString sMES1123     [MAX_AUTO_TRAY]={"MES1123", "MES1223", "MES1323", "MES2523", "MES2623", "MES2723"};
AnsiString sWAR1130     [MAX_AUTO_TRAY]={"WAR1130", "WAR1230", "WAR1330", "WAR2530", "WAR2630", "WAR2730"};
AnsiString sWAR1151     [MAX_AUTO_TRAY]={"WAR1151", "WAR1251", "WAR1351", "WAR2551", "WAR2651", "WAR2751"};
AnsiString sJAM1170     [MAX_AUTO_TRAY]={"JAM1170", "JAM1270", "JAM1370", "JAM2570", "JAM2670", "JAM2770"};
AnsiString sWAR1181     [MAX_AUTO_TRAY]={"WAR1181", "WAR1281", "WAR1381", "WAR2581", "WAR2681", "WAR2781"};

int iC_Up               [MAX_TRACK]={C_Load_Up,             C_Empty_Up,             C_Color_Up,             C_Auto1_Up,             C_Auto2_Up,             C_Auto3_Up,             C_Auto4_Up,             C_Auto5_Up,             C_Auto6_Up              };
int iC_Middle           [MAX_TRACK]={C_Load_Middle,         C_Empty_Middle,         C_Color_Middle,         C_Auto1_Selector,       C_Auto2_Selector,       C_Auto3_Selector,       C_Auto4_Selector,       C_Auto5_Selector,       C_Auto6_Selector        };
int iC_EdgePush         [MAX_TRACK]={C_LoaderEdgePush,      C_EmptyEdgePush,        C_ColorEdgePush,        C_Auto1EdgePush,        C_Auto2EdgePush,        C_Auto3EdgePush,        C_Auto4EdgePush,        C_Auto5EdgePush,        C_Auto6EdgePush         };
int iTrackFloodgate     [MAX_TRACK]={C_LoadTrackFloodgate,  C_EmptyTrackFloodgate,  C_ColorTrackFloodgate,  C_Auto1TrackFloodgate,  C_Auto2TrackFloodgate,  C_Auto3TrackFloodgate,  C_Auto4TrackFloodgate,  C_Auto5TrackFloodgate,  C_Auto6TrackFloodgate   };
int iAutoBack           [MAX_TRACK]={C_LoaderPushBack_Back, C_EmptyPushBack_Back,   C_ColorPushBack_Back,   C_Auto1PushBack_Back,   C_Auto2PushBack_Back,   C_Auto3PushBack_Back,   C_Auto4PushBack_Back,   C_Auto5PushBack_Back,   C_Auto6PushBack_Back    };
int iAutoPush           [MAX_TRACK]={C_LoaderPushBack_Push, C_EmptyPushBack_Push,   C_ColorPushBack_Push,   C_Auto1PushBack_Push,   C_Auto2PushBack_Push,   C_Auto3PushBack_Push,   C_Auto4PushBack_Push,   C_Auto5PushBack_Push,   C_Auto6PushBack_Push    };

int C_FixLoaderZ_Select [MAX_FIX_TRAY]={C_Fix1LoaderZ_Select,  C_Fix2LoaderZ_Select,  C_Fix3LoaderZ_Select,  C_Fix4LoaderZ_Select,  C_Fix5LoaderZ_Select,  C_Fix6LoaderZ_Select  };
AnsiString sMES1712     [MAX_FIX_TRAY]={"MES1712", "MES1812", "MES1912", "MES2812", "MES2912", "MES3012"};
AnsiString sMES1713     [MAX_FIX_TRAY]={"MES1713", "MES1813", "MES1913", "MES2813", "MES2913", "MES3013"};   //RogerYang 20250626 偉測不可複測bin功能
AnsiString sMES1720     [MAX_FIX_TRAY]={"MES1720", "MES1820", "MES1920", "MES2820", "MES2920", "MES3020"};
AnsiString sMES1721     [MAX_FIX_TRAY]={"MES1721", "MES1821", "MES1921", "MES2821", "MES2921", "MES3021"};
AnsiString sWAR1722     [MAX_FIX_TRAY]={"WAR1722", "WAR1822", "WAR1922", "WAR2822", "WAR2922", "WAR3022"};
AnsiString sMES1723     [MAX_FIX_TRAY]={"MES1723", "MES1823", "MES1923", "MES2823", "MES2923", "MES3023"};
AnsiString sWAR1751     [MAX_FIX_TRAY]={"WAR1751", "WAR1851", "WAR1951", "WAR2851", "WAR2951", "WAR3051"};
AnsiString sWAR1752     [MAX_FIX_TRAY]={"WAR1752", "WAR1852", "WAR1952", "WAR2852", "WAR2952", "WAR3052"};
//<==
//Steven 20230907 : For HT-9011UC
//====================================================================
//Cylin define end
//====================================================================

//====================================================================
//Sensor define start
//====================================================================
const int SnFKPowerOff              =0;
const int SnFKPowerOn               =1;
const int SnFKReset                 =2;
const int SnFKPause                 =3;
const int SnFKHome                  =4;
const int SnFKStart                 =5;
const int SnFKOneCycle              =6;
const int SnFKRetry                 =7;
const int SnFKSkip                  =8;
const int SnFKCleanOut              =9;

const int SnFKTrayFeed              =10;
const int SnFKTrayEnd               =11;
const int SnFKAlarmReset            =12;
const int SnFKCoverOpen             =13;
const int SnRKPowerOff              =14;
const int SnRKPowerOn               =15;
const int SnRKReset                 =16;
const int SnRKPause                 =17;
const int SnRKHome                  =18;
const int SnRKStart                 =19;

const int SnRKOneCycle              =20;
const int SnRKRetry                 =21;
const int SnRKSkip                  =22;
const int SnRKCleanOut              =23;
const int SnRKTrayFeed              =24;
const int SnRKTrayEnd               =25;
const int SnRKAlarmReset            =26;
const int SnRKCoverOpen             =27;
const int SnRKManualStep            =28;
const int SnRKManualTStart          =29;
//------------------------------------------------------------------------------
//Loader
//------------------------------------------------------------------------------
const int SnLoaderTrayHasTray       =30;
const int SnLoaderCarHasTray        =31;
const int SnLoaderPreDete           =32;
//------------------------------------------------------------------------------
//In Put Shuttle
//------------------------------------------------------------------------------
const int SnInPutSHT1S1             =33;
const int SnInPutSHT1S2             =34;
const int SnInPutSHT1S3             =35;
const int SnInPutSHT1S4             =36;
const int SnInPutSHT1S5             =37;
const int SnInPutSHT1S6             =38;
const int SnInPutSHT1S7             =39;

const int SnInPutSHT2S1             =40;
const int SnInPutSHT2S2             =41;
const int SnInPutSHT2S3             =42;
const int SnInPutSHT2S4             =43;
const int SnInPutSHT2S5             =44;
const int SnInPutSHT2S6             =45;
const int SnInPutSHT2S7             =46;
//------------------------------------------------------------------------------
//Out Put Shuttle
//------------------------------------------------------------------------------
const int SnOutPutSHT1S1            =47;
const int SnOutPutSHT1S2            =48;
const int SnOutPutSHT1S3            =49;
const int SnOutPutSHT1S4            =50;
const int SnOutPutSHT1S5            =51;
const int SnOutPutSHT1S6            =52;
const int SnOutPutSHT1S7            =53;

const int SnOutPutSHT2S1            =54;
const int SnOutPutSHT2S2            =55;
const int SnOutPutSHT2S3            =56;
const int SnOutPutSHT2S4            =57;
const int SnOutPutSHT2S5            =58;
const int SnOutPutSHT2S6            =59;
const int SnOutPutSHT2S7            =60;

const int SnOutPutSHT1ZS1           =61;
const int SnOutPutSHT1ZS2           =62;
const int SnOutPutSHT2ZS1           =63;
const int SnOutPutSHT2ZS2           =64;
//------------------------------------------------------------------------------
//Auto
//------------------------------------------------------------------------------
const int SnAuto1TrayDetect         =65;
const int SnAuto2TrayDetect         =66;
const int SnAuto3TrayDetect         =67;
const int SnAuto1IsFull             =68;
const int SnAuto2IsFull             =69;
const int SnAuto3IsFull             =70;
//------------------------------------------------------------------------------
//Fix
//------------------------------------------------------------------------------
const int SnFixedTray1Detect        =71;
const int SnFixedTray2Detect        =72;
const int SnFixedTray3Detect        =73;
//------------------------------------------------------------------------------
//System
//------------------------------------------------------------------------------
const int SnSafeDoor1               =74;
const int SnSafeDoor2               =75;
const int SnSafeDoor3               =76;
const int SnSafeDoor4               =77;
const int SnSafeDoor5               =78;
const int SnSafeDoor6               =79;
const int SnSafeDoor7               =80;
const int SnSafeDoor8               =81;
const int SnAirIsEnough             =82;
const int SnFrontRightEMG           =83;
const int SnRearLeftEMG             =84;
const int SnFMotorDown              =85;
const int SnBMotorDown              =86;
const int SnMotorPower              =87;
const int SnSystemPower             =88;
//------------------------------------------------------------------------------
//Empty
//------------------------------------------------------------------------------
const int SnEmptyTrayHasTray1       =89;
const int SnEmptyTrayIsFull1        =90;
const int SnEmptyTrayIsLock1        =91;
const int SnEmptyTrayHasTray2       =92;
const int SnEmptyTrayIsFull2        =93;

const int SnRearPadActive           =94;
const int SnFrontLeftEMG            =95;
const int SnRearRightEMG            =96;
const int SnEPDieForce              =97;
//------------------------------------------------------------------------------
//TTL
//------------------------------------------------------------------------------
const int SenBit0                   =98;
const int SenBit1                   =99;
const int SenBit2                   =100;
const int SenBit3                   =101;
const int SenBit4                   =102;
const int SenBit5                   =103;
const int SenBit6                   =104;
const int SenBit7                   =105;
const int SenBit8                   =106;
const int SenBit9                   =107;
//------------------------------------------------------------------------------

const int SnAuto1_Tray_Car          =108;
const int SnAuto2_Tray_Car          =109;
const int SnAuto3_Tray_Car          =110;
const int SnAuto1PreDete            =111;
const int SnAuto2PreDete            =112;
const int SnAuto3PreDete            =113;

const int SnLoaderSureTray          =114;
//const int SnMotorYAlarm           =115;
//const int SnMotorZ1Alarm          =116;
//const int SnMotorZ2Alarm          =117;
const int SnAuto1FixCyPush          =118;
const int SnAuto2FixCyPush          =119;
const int SnAuto3FixCyPush          =120;
const int SnLoaderFixCyPush         =121;

//2003/03/26 for 10 bit bit
const int SenBit10                  =122;
const int SenBit11                  =123;
const int SenBit12                  =124;
const int SenBit13                  =125;
const int SenBit14                  =126;
const int SenBit15                  =127;
const int SenBit16                  =128;
const int SenBit17                  =129;
const int SenBit18                  =130;
const int SenBit19                  =131;
const int SenEmptyHasTray           =132;
const int SenEmptyCWDete            =133;
const int SenEmptySelectHasTray     =134;
const int SenEmptyCCWDete           =135;
const int SenEmptyCarHasTray        =136;

const int SenColorHasTray           =137;
const int SenColorCWDete            =138;
const int SenColorSelectHasTray     =139;
const int SenColorCarHasTray        =140;
const int SenEmptyFixCyPush         =141;

const int SnAuto1TrayHasTray        =142;
const int SnAuto1CWPreDetect        =143;
const int SnHeaterDoor              =144;
const int SnCatchTrayFix1On         =145;
const int SnCatchTrayFix2On         =146;

const int SnSafeLock                =147;

const int SnFPLevelOpe              =148;
const int SnFPLevelEng              =149;                                       //Steven 20190503 : 指紋辨識權限
const int SnFPLevelSup              =150;
const int SnFPLevelHon              =151;

const int SnIndexHeaterFan          =152;
const int SnHeaterDoor2             =153;
const int SnHeaterDoor3             =154;                                       //Steven 20191016 : For ATC3.3, 要第三個加熱門

const int SnAuto1TrackDetect        =155;
const int SnAuto2TrackDetect        =156;
const int SnAuto3TrackDetect        =157;
const int SnEmptyIsFull             =158;
const int SnColorIsFull             =159;
const int SenColorCCWDete           =160;
const int SenColorFixCyPush         =161;

const int SenEmpty1HasTray          =162;
const int SenEmpty1CCWDete          =163;
const int SenEmpty1CarHasTray       =164;

const int SnAutoUpSafedetect0       =165;
const int SnAutoUpSafedetect1       =166;
const int SnAutoUpSafedetect2       =167;

const int SnIonFanAlarm             =168;
const int SnIonFanLevelAlarm        =169;
const int SnIonBarrierAlarm         =170;
const int SnIonBarrierLevelAlarm    =171;
const int SnIonBarrierConditionAlarm=172;
const int SnAutoColorTrayDetect0    =173;
const int SnAutoColorTrayDetect1    =174;
const int SnAutoColorTrayDetect2    =175;

const int SnLoad2IsFull             =176;                                       //Steven 20240822 : For HT-9046AU
const int SnLoad2IsPreAlarm         =177;
const int SnLoad2TrayHasTray_AGV    =178;
const int SnLoad2TrayHasTray_ART    =179;
const int SnLoad2TrayHasTray        =180;
const int SnLoad2CarHasTray         =181;

const int SnSafeDoor9               =182;

const int SnCheckTray2Direction     =183;                                       //Steven 20240822 : For HT-9046AU
const int SenLoad2CCWDete           =184;
const int SnLoad2UpSafedetect       =185;
const int SenLoad2CCWDete_2         =186;

const int SnInPutSHT1S8             =187;                                       //Eliot 2009_12_27 start
const int SnInPutSHT1S9             =188;
const int SnInPutSHT2S8             =189;
const int SnInPutSHT2S9             =190;                                       //Eliot 2009_12_27 end
const int SnSafeDoor10              =191;

const int SnIonFan6Alarm            =192;
const int SnIonFan7Alarm            =193;
const int SnIonFan8Alarm            =194;
const int SnIonFan9Alarm            =195;

const int SnEPAlarm                 =196;
const int SnCheckLoadDirection      =197;
const int SnCheckTrayDirection      =198;

const int SnNegativePressureAir     =199;

//jou 2010-11-23
const int SnLoaderEdgePush          =200;
const int SnAuto1EdgePush           =201;
const int SnAuto2EdgePush           =202;
const int SnAuto3EdgePush           =203;
//Steven 20100105
const int SnIonFan10Alarm           =204;
const int SnIonFan11Alarm           =205;

//----- by dell ccd realtime-------------
const int SnRealTimeCCDStop         =206;
const int SnRealTimeCCDIndexArm     =207;
const int SnRTCCDTempCtrl           =208;
//---------------------------------------
const int SnUnLoaderFloating        =209;

const int SnRotateCheck             =210;                                       //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor
const int SnCheckConnectIndexArm_1  =210;
const int SnCheckConnectIndexArm_2  =211;

const int SnFixFloating1            =212;                                       //Steven 20120131 : Fix Tray置偏偵測
const int SnFixFloating2            =213;

const int SnATCAlarm1               =214;                                       //jou 2012-03-13 ATC Alarm 1 Sensor
const int SnATCAlarm2               =215;                                       //jou 2012-03-13 ATC Alarm 2 Sensor
const int SnATCAlarm3               =216;                                       //jou 2012-03-13 ATC Alarm 3 Sensor
const int SnATCAlarm4               =217;                                       //jou 2012-03-13 ATC Alarm 4 Sensor

const int SenInArmYPitch60          =218;                                       //ChungHung 20120505 : HT9045 WS Only
const int SenOutArmYPitch60         =219;                                       //ChungHung 20120505 : HT9045 WS Only

const int SnATC01ControllerHighAlarm=220;                                       //Steven 20120410 : Hontech ATC
const int SnATC02ControllerHighAlarm=221;                                       // 2011.05.26 , Joye , ATC Alarm ---------->>
const int SnATC03ControllerHighAlarm=222;
const int SnATC04ControllerHighAlarm=223;
const int SnATC01ControllerLowAlarm =224;
const int SnATC02ControllerLowAlarm =225;
const int SnATC03ControllerLowAlarm =226;
const int SnATC04ControllerLowAlarm =227;

const int SenLoaderCCWDete          =228;
const int SnLoaderIsFull            =229;
const int SnAuto2CWPreDetect        =230;
const int SnAuto3CWPreDetect        =231;
const int SnAuto2TrayHasTray        =232;
const int SnAuto3TrayHasTray        =233;
const int SnAutoDockingOff          =234;                                       //ChungHung 20120718 add UseAutoDocking Check Sensor
const int SnAutoDockingOn           =235;                                       //ChungHung 20120718 add UseAutoDocking Check Sensor

const int SnTesterDocking           =236;                                       //jou 2012-09-13 Tester Docking
const int SnTrain                   =237;                                       //ChungHung 20120911 add
const int SnFix3FullPlace           =238;                                       //Steven 20121020 : Fix3滿盤

const int SnIonFanPower01           =239;                                       //Steven 20130201 : Kasuga離子風扇電源偵測
const int SnIonFanPower02           =240;
const int SnIonFanPower03           =241;
const int SnIonFanPower04           =242;
const int SnIonFanPower05           =243;
const int SnIonFanPower06           =244;
const int SnIonFanPower07           =245;
const int SnIonFanPower08           =246;
const int SnIonFanPower09           =247;
const int SnIonFanPower10           =248;
const int SnIonFanPower11           =249;
const int SnIonFanPower12           =250;

const int SnIonFan12Alarm           =251;

const int SnSocket1                 =252;                                       //kevin 20130429  socket sensor
const int SnSocket2                 =253;                                       //kevin 20130429  socket sensor
const int SnSocket3                 =254;                                       //kevin 20130429  socket sensor
const int SnSocket4                 =255;                                       //kevin 20130429  socket sensor
const int SnSocket5                 =256;                                       //kevin 20130429  socket sensor
const int SnSocket6                 =257;                                       //kevin 20130429  socket sensor
const int SnSocket7                 =258;                                       //kevin 20130429  socket sensor
const int SnSocket8                 =259;                                       //kevin 20130429  socket sensor

const int SnSocket9                 =260;                                       //Steven 20200610 : Socket sensor 改成16顆
const int SnSocket10                =261;
const int SnSocket11                =262;
const int SnSocket12                =263;
const int SnSocket13                =264;
const int SnSocket14                =265;
const int SnSocket15                =266;
const int SnSocket16                =267;

const int SnServo                   =268;                                       //kevin 20140121 偵測servo  on 訊號
const int SnAutoTeach               =269;                                       //kevin 201400512 AUTOTEACH IN/OUT ARM SENSOR
const int SnEOF1                    =270;                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SnEOF2                    =271;                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SnEOF3                    =272;                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SnAOIResult1              =273;                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SnAOIResult2              =274;                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SnAOIResult3              =275;                                       //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SnFix3Lock                =276;                                       //Steven 20140512 : For HT-9046LA
const int SenAutoDocking240KG       =277;                                       //ChungHung 20140709 add for hardware control OTD
const int SenAutoDocking360KG       =278;                                       //ChungHung 20140709 add for hardware control OTD

const int SnFixColorTrayDetect0     =279;                                       //20140903 wei colcr Tray
const int SnFixColorTrayDetect1     =280;                                       //20140903 wei colcr Tray
const int SnFixColorTrayDetect2     =281;                                       //20140903 wei colcr Tray

const int SnLoaderColorTrayDetect   =282;                                       //20140903 wei colcr Tray
const int SnEmptyColorTrayDetect    =283;                                       //20140903 wei colcr Tray
const int SnColorColorTrayDetect    =284;                                       //20140903 wei colcr Tray

const int SnGroundMan               =285;                                       //wei 20150424 add SnGroundMan偵測
const int SnOCRTigger               =286;                                       //wei 20150720 OCR觸發
const int SnLowSpeed                =287;                                       //wei 20150720 OCR觸發
const int SnOCRPosition             =288;                                       //wei 20150720 OCR觸發
const int SnLoaderTrayHasTray_ART   =289;                                       //wei 20151210
const int SnSLK1UnclampButton       =290;                                       //JerryYang 20160524
const int SnSLK2UnclampButton       =291;                                       //JerryYang 20160524
const int SnSocketClampPush1        =292;                                       //JerryYang 20160606
const int SnSocketClampPush2        =293;                                       //JerryYang 20160606
const int SnSocketClampPull1        =294;                                       //JerryYang 20160606
const int SnSocketClampPull2        =295;                                       //JerryYang 20160606
const int SnSocketHasClamp1         =296;                                       //JerryYang 20160606
const int SnSocketHasClamp2         =297;                                       //JerryYang 20160606

//------------------------------------
//Steven 20161011 : TTL支援8Site
//------------------------------------
const int SenBit20                  =298;
const int SenBit21                  =299;
const int SenBit22                  =300;
const int SenBit23                  =301;
const int SenBit24                  =302;
const int SenBit25                  =303;
const int SenBit26                  =304;
const int SenBit27                  =305;
const int SenBit28                  =306;
const int SenBit29                  =307;
const int SenBit30                  =308;
const int SenBit31                  =309;
const int SenBit32                  =310;
const int SenBit33                  =311;
const int SenBit34                  =312;
const int SenBit35                  =313;
const int SenBit36                  =314;
const int SenBit37                  =315;
const int SenBit38                  =316;
const int SenBit39                  =317;
//------------------------------------
//RogerYang 20161212 (Steven) : 偵測SLK獨立加熱或共用加熱
//------------------------------------
const int SnIndex1Connect1          =318;
const int SnIndex1Connect2          =319;
const int SnIndex2Connect1          =320;
const int SnIndex2Connect2          =321;
const int SnTrayCover               =322;                                       //Steven 20170623 (wei) : Add for catch tray with cover

const int SnLoaderIsPreAlarm        =323;                                       //wei 20170802 Pre alarm sensor
const int SnEmptyIsPreAlarm         =324;                                       //wei 20170802 Pre alarm sensor
const int SnColorIsPreAlarm         =325;                                       //wei 20170802 Pre alarm sensor
const int SnAuto1IsPreAlarm         =326;                                       //wei 20170802 Pre alarm sensor
const int SnAuto2IsPreAlarm         =327;                                       //wei 20170802 Pre alarm sensor
const int SnAuto3IsPreAlarm         =328;                                       //wei 20170802 Pre alarm sensor
const int SnTrayArmSafePos          =329;                                       //kevin 20171006 (wei) Home tray arm must on
const int SnNegativePressureAir2    =330;                                       //Sam 20171110 (Steven) : 新增氣壓 Sensor
const int SnTJCurrent               =331;                                       //Steven 20180124 : Check ATC7.0 TJ Current

const int SnHingeTopTray            =332;                                       //wei 20170418 Hinge cassette
const int SnSafeDoor11              =333;                                       //wei 20180702 MR

const int SnEmptyFull               =334;                                       //wei 20170504 Use Empty Full Put Color

const int SnCassetteArmHave         =335;                                       //wei 20180702 MR
const int SnBuffer1HaveCassette     =336;                                       //wei 20180702 MR
const int SnBuffer2HaveCassette     =337;                                       //wei 20180702 MR
const int SnBuffer3HaveCassette     =338;                                       //wei 20180702 MR
const int SnBuffer4HaveCassette     =339;                                       //wei 20180702 MR
const int SnBuffer5HaveCassette     =340;                                       //wei 20180702 MR
const int SnBuffer6HaveCassette     =341;                                       //wei 20180702 MR
const int SnBuffer7HaveCassette     =342;                                       //wei 20180702 MR
const int SnBuffer8HaveCassette     =343;                                       //wei 20180702 MR
const int SnBuffer9HaveCassette     =344;                                       //wei 20180702 MR
const int SnBuffer10HaveCassette    =345;                                       //wei 20180702 MR

const int SnLoadPortCatch           =346;                                       //wei 20180702 MR
const int SnLoadPortPresent         =347;                                       //wei 20180702 MR
const int SnLoadPortPlacement1      =348;                                       //wei 20180702 MR
const int SnLoadPortPlacement2      =349;                                       //wei 20180702 MR
const int SnTrayBracketHave         =350;                                       //wei 20180702 MR
const int SnStackedTrayHave         =351;                                       //wei 20180702 MR
const int SnBuffer6HaveTray         =352;                                       //wei 20180702 MR
const int SnSafeDoor12              =353;                                       //wei 20180702 MR
const int SnSafeDoor13              =354;                                       //wei 20180702 MR
const int SnSafeDoor14              =355;                                       //wei 20180702 MR
const int SnSafeDoor15              =356;                                       //wei 20180702 MR

const int SnCassetteCatch           =357;                                       //wei 20180702 MR

const int SnE84VALID                =358;                                       //wei 20180702 E84
const int SnE84CS0                  =359;                                       //wei 20180702 E84
const int SnE84CS1                  =360;                                       //wei 20180702 E84
const int SnE84AMAVBL               =361;                                       //wei 20180702 E84
const int SnE84TRREQ                =362;                                       //wei 20180702 E84
const int SnE84BUSY                 =363;                                       //wei 20180702 E84
const int SnE84COMPT                =364;                                       //wei 20180702 E84
const int SnE84CONT                 =365;                                       //wei 20180702 E84
const int SnE84GO                   =366;                                       //wei 20180702 E84

const int SnCassette01              =367;                                       //wei 20180702 MR
const int SnCassette02              =368;                                       //wei 20180702 MR
const int SnCassette03              =369;                                       //wei 20180702 MR
const int SnCassette04              =370;                                       //wei 20180702 MR
const int SnCassette05              =371;                                       //wei 20180702 MR
const int SnCassette06              =372;                                       //wei 20180702 MR
const int SnCassette07              =373;                                       //wei 20180702 MR
const int SnCassette08              =374;                                       //wei 20180702 MR
const int SnCassette09              =375;                                       //wei 20180702 MR
const int SnCassette10              =376;                                       //wei 20180702 MR
const int SnMRStart                 =377;                                       //wei 20180702 MR
const int SnMRPause                 =378;                                       //wei 20180702 MR
const int SnMRUp                    =379;                                       //wei 20180702 MR
const int SnMRDown                  =380;                                       //wei 20180702 MR

//Sam 20190112 LM
//==>
const int SnCassetteArmCatch        =381;
const int SnLoadPortHaveCassette1_1 =382;
const int SnLoadPortHaveCassette1_2 =383;
const int SnLoadPortHaveCassette2_1 =384;
const int SnLoadPortHaveCassette2_2 =385;
const int SnLoadPortHaveCassette3_1 =386;
const int SnLoadPortHaveCassette3_2 =387;
const int SnLoadPortHaveCassette4_1 =388;
const int SnLoadPortHaveCassette4_2 =389;
const int SnLoadRobotHaveCassette1  =390;
const int SnLoadRobotHaveCassette2  =391;
const int SnLoadRobotPosUp          =392;
const int SnLoadRobotPosDown        =393;

const int SnUnloadPortHaveCassette1_1=394;
const int SnUnloadPortHaveCassette1_2=395;
const int SnUnloadPortHaveCassette2_1=396;
const int SnUnloadPortHaveCassette2_2=397;
const int SnUnloadPortHaveCassette3_1=398;
const int SnUnloadPortHaveCassette3_2=399;
const int SnUnloadPortHaveCassette4_1=400;
const int SnUnloadPortHaveCassette4_2=401;
const int SnUnloadRobotHaveCassette1=402;
const int SnUnloadRobotHaveCassette2=403;
const int SnUnloadRobotPosUp        =404;
const int SnUnloadRobotPosDown      =405;

const int SnSafeDoor16              =406;
const int SnSafeDoor17              =407;
const int SnSafeDoor18              =408;
const int SnSafeDoor19              =409;
//<==
//Sam 20190112 LM

const int SnLoaderUpSafedetect      =410;
const int SnEmptyUpSafedetect       =411;
const int SnColorUpSafedetect       =412;
const int SnLoaderUpPress           =413;                                       //JerryYang 20181120 (Steven) : (Steven) : 獨立控制loader壓tray
const int SnPreciserDetect1         =414;                                       //Frank 20180410 (Steven) : InArm Preciser Station
const int SnPreciserDetect2         =415;

const int SnLoadCell1               =416;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell2               =417;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell3               =418;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell4               =419;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell5               =420;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell6               =421;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell7               =422;                                       //kevin 20190307  read load cell pass  or fail
const int SnLoadCell8               =423;                                       //kevin 20190307  read load cell pass  or fail

const int SnHotGun1                 =424;                                       //kevin 20190621  hot gun 流量異常 使用 Out 2
const int SnHotGun2                 =425;                                       //kevin 20190621  hot gun 流量異常 使用 Out 2

const int SnGroundMan2              =426;
const int SnGroundMan3              =427;
const int SnGroundMan4              =428;
const int SnAuto1UpPress            =429;                                       //JerryYang 20190423 新增unloader壓tray
const int SnAuto2UpPress            =430;
const int SnAuto3UpPress            =431;
const int SnWaterLeakageUp          =432;                                       //wei 20190617 漏水檢測
const int SnWaterLeakageDown        =433;                                       //wei 20190617 漏水檢測
const int SnWaterLeakagePlate       =434;                                       //wei 20190617 漏水檢測
const int SnLoaderTrackDetect       =435;                                       //Sam 20200316 : Loader Detect Tray
const int SnTesterDryAir            =436;                                       //Ifor 20200115 add: add Tester Dry Air Control //Ifor 20200420 429->436

const int SnRotateRowIn1            =437;                                       //kevin 20130524 SnRotateRow_In_1 ->SnRotateRowOut1
const int SnRotateRowIn2            =438;
const int SnRotateRowOut1           =439;
const int SnRotateRowOut2           =440;
const int SnCrossSHT1S1             =441;                                       //2013-07-16    Dell    Shuttle cross sensor
const int SnCrossSHT1S2             =442;                                       //2013-07-16    Dell    Shuttle cross sensor
const int SnCrossSHT2S1             =443;                                       //2013-07-16    Dell    Shuttle cross sensor
const int SnCrossSHT2S2             =444;                                       //2013-07-16    Dell    Shuttle cross sensor

const int SnAuto1Z_Select1          =445;                                       //JerryYang 20200615 ART分離汽缸sensor
const int SnAuto2Z_Select1          =446;
const int SnAuto3Z_Select1          =447;
const int SnAuto1Z_Select2          =448;
const int SnAuto2Z_Select2          =449;
const int SnAuto3Z_Select2          =450;
const int SnAuto1Z_Select3          =451;
const int SnAuto2Z_Select3          =452;
const int SnAuto3Z_Select3          =453;
const int SnAuto1Z_Select4          =454;
const int SnAuto2Z_Select4          =455;
const int SnAuto3Z_Select4          =456;

const int SnHeaterDoor4             =457;                                       //wei 20200616 : For ATC3.3 MR, 要第四個加熱門
const int SnChamberHeatDetect       =458;                                       //JerryYang 20210107 : ChamberHeatDetect

//==>
const int SnInFlipper1DeviceDetect  =459;                                       //Frank 20210612 : Flipper Function
const int SnInFlipper2DeviceDetect  =460;
const int SnInFlipper3DeviceDetect  =461;
const int SnOutFlipper3DeviceDetect =462;
const int SnOutFlipper1DeviceDetect =463;
const int SnOutFlipper2DeviceDetect =464;
const int SnLoaderCasstteDetect     =465;
const int SnEmptyCasstteDetect      =466;
const int SnColorCasstteDetect      =467;
const int SnAuto1CasstteDetect      =468;
const int SnAuto2CasstteDetect      =469;
const int SnAuto3CasstteDetect      =470;

const int SnArm1SLK                 =471;
const int SnArm2SLK                 =472;

const int SnMRAuto21                =473;                                       //wei 20180702 MR
//E84驗證用
const int SnE84LREQ                 =474;                                       //wei 20180702 E84
const int SnE84UREQ                 =475;                                       //wei 20180702 E84
const int SnE84VA                   =476;                                       //wei 20180702 E84
const int SnE84READY                =477;                                       //wei 20180702 E84
const int SnE84VS0                  =478;                                       //wei 20180702 E84
const int SnE84VS1                  =479;                                       //wei 20180702 E84
const int SnE84HOAVBL               =480;                                       //wei 20180702 E84
const int SnE84ES                   =481;                                       //wei 20180702 E84
const int SnE84POWER                =482;                                       //wei 20180702 E84

const int SnBufferTop               =483;                                       //wei 20200305 MR
const int SnBufferCatch             =484;                                       //wei 20200305 MR
const int SnCassetteOpen            =486;

const int SnMultileEmpty_Door               =487;                               //KaiChen 20200716 ：OHT
const int SnMultileEmpty_MagazineTop        =488;
const int SnMultileEmpty_MagazineLow        =489;
const int SnMultileEmpty_MagazineCatch      =490;
const int SnMultileEmpty_SelectHasTray      =491;
const int SnMultileEmpty_HasTray            =492;
const int SnMultileEmpty_CornerPushCyPush   =493;
const int SnMultileEmpty_CatchHasTray       =494;

const int SnSafeDoor20                      =495;
const int SnSafeDoor21                      =496;

const int SnLoadPort2Present                =497;
const int SnLoadPort2Placement1             =498;
const int SnLoadPort2Placement2             =499;
const int SnTrayBracket2Have                =500;

const int SnCassetteArmIntoLoadPort         =501;
const int SnCassetteArmIntoLoadPort2        =502;

const int SnBuffer7HaveTray                 =503;

const int SnLoadPortAutoManual              =504;
const int SnLoadPort2AutoManual             =505;

const int SnOHTIntoLoadPort                 =506;
const int SnOHTIntoLoadPort2                =507;

const int SnCassetteClose                   =508;
const int SnCassetteOpen2                   =509;
const int SnCassetteClose2                  =510;

const int SnCassetteSlotMap                 =511;
const int SnCassetteFloating                =512;

const int SnMultileEmpty_ScanTrayID         =513;

const int SnE84_2_VALID             =514;
const int SnE84_2_CS0               =515;
const int SnE84_2_CS1               =516;
const int SnE84_2_AMAVBL            =517;
const int SnE84_2_TRREQ             =518;
const int SnE84_2_BUSY              =519;
const int SnE84_2_COMPT             =520;
const int SnE84_2_CONT              =521;
const int SnE84_2_GO                =522;

const int SnE84_1_VALID             =523;
const int SnE84_1_CS0               =524;
const int SnE84_1_CS1               =525;
const int SnE84_1_AMAVBL            =526;
const int SnE84_1_TRREQ             =527;
const int SnE84_1_BUSY              =528;
const int SnE84_1_COMPT             =529;
const int SnE84_1_CONT              =530;
const int SnE84_1_GO                =531;

const int SnMultileEmptyCatch_HasTray   =532;
const int SnCSTHoldDownOff              =533;
const int SnCSTHoldDown2Off             =534;
const int SnMultileEmpty_ScanTray2DID   =535;
const int SnDieDetect_Sh1               =536;
const int SnDieDetect_Sh2               =537;
const int SnBuffer1_Placement1          =538;
const int SnBuffer2_Placement1          =539;
const int SnBuffer3_Placement1          =540;
const int SnBuffer4_Placement1          =541;
const int SnBuffer5_Placement1          =542;
const int SnBuffer6_Placement1          =543;
const int SnBuffer7_Placement1          =544;
const int SnBuffer8_Placement1          =545;
const int SnBuffer9_Placement1          =546;
const int SnBuffer10_Placement1         =547;
const int SnStackedTrayYDetect_Loader   =548;
const int SnStackedTrayYDetect_Elevator2=549;

const int SnRKSafeLock              =550;                                       //KenHsieh 20211228 : 區分實體IO與通訊面板
const int SnSht1Left                =551;                                       //kevin 20220512 add SHUTTLE Left 位置偏移
const int SnSht1Right               =552;                                       //kevin 20220512 add SHUTTLE Left 位置偏移
const int SnSht2Left                =553;                                       //kevin 20220512 add SHUTTLE Left 位置偏移
const int SnSht2Right               =554;                                       //kevin 20220512 add SHUTTLE Left 位置偏移

const int SnLoaderTrayHasTray_AGV   =555;                                       //kevin 20220520 add AGV load
const int SnEmptyTrayHasTray_AGV    =556;                                       //kevin 20220520 add AGV Empty
const int SnColorTrayHasTray_AGV    =557;                                       //kevin 20220520 add AGV Color

const int SnInAreaAlignmentSenX     =558;                                       //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
const int SnInAreaAlignmentSenY     =559;
const int SnOutAreaAlignmentSenX    =560;
const int SnOutAreaAlignmentSenY    =561;

const int SnAseTrayBufferLeft       =562;                                       //kevin 20220709 ASEKH 左邊放空TRAY
const int SnAseTrayBufferRight      =563;                                       //kevin 20220709 ASEKH 右邊放空TRAY

const int SnATC1Ready               =564;                                       //Jimmychiu 20210902 add: ATC Winway IO ready
const int SnATC2Ready               =565;
const int SnATC3Ready               =566;
const int SnATC4Ready               =567;

const int SnPlate1TempOverDetect    =568;                                       //Ztex 2023.04.13 Add HT-1032 IO ==>
const int SnPlate2TempOverDetect    =569;
const int SnShuttle1TempOverDetect  =570;
const int SnShuttle2TempOverDetect  =571;
const int SnHead1TempOverDetect     =572;
const int SnHead2TempOverDetect     =573;
const int SnHead5TempOverDetect     =574;
const int SnHead6TempOverDetect     =575;
const int SnHumidityAnomaly1Detect  =576;
const int SnHumidityAnomaly2Detect  =577;
const int SnHumidityAnomaly3Detect  =578;
const int SnDryAirIsEnough          =579;
const int SnIonBarInAirIsEnough     =580;
const int SnIonBarOutAirIsEnough    =581;
const int SnSafeDoor1Hatchway       =582;
const int SnSafeDoor2Hatchway       =583;
const int SnSafeDoor3Hatchway       =584;
const int SnSafeDoor4Hatchway       =585;
const int SnSafeDoor5Hatchway       =586;
const int SnSafeDoor6Hatchway       =587;
const int SnSafeDoor7Hatchway       =588;
const int SnSafeDoor8Hatchway       =589;
const int SnSafeDoor9Hatchway       =590;
const int SnSafeDoor10Hatchway      =591;
const int SnSafeDoor11Hatchway      =592;
const int SnSafeDoor6PosFixPickPlace=593;
const int SnTriTempSafeDoor6Lock    =594;

const int SnTrayArmZSafePos         =595;
const int SnEnhaustAirFanAlarmDetect=596;                                       //Ztex 2023.04.26 Add HT-1032 IO Exhaust Air

const int SnMagazineSafeDoor            =597;                                   //JerryYang 20220909 : add magazine
const int SnMagazineDetect              =598;
const int SnMagazineTrackDetect         =599;
const int SnMagazineTrackSelectDetect   =600;
const int SnMagazineSafeDoor2           =601;
const int SnMagazineSafeDoor3           =602;
const int SnMagazineDetectTop           =603;
const int SnMagazineHasTrayInside       =604;
const int SnMagazineTrackDetect2        =605;                                   //Sam 20221116 : Magazine TrayArm 自動補 Tray

const int SnDockingAreaOpenCheck    =606;                                       //Ztex 2023.05.02 Add HT-1032 IO Docking Area Open Check
const int SnDewPointDetectIndexArm1 =607;                                       //Hmy 20170603  add By 三溫機 露點SENSOR Arm1偵測
const int SnDewPointDetectIndexArm2 =608;                                       //Hmy 20170603  add By 三溫機 露點SENSOR Arm2偵測

const int SnLoaderSeparateHasTray   =609;
const int SnEmptySeparateHasTray    =610;
const int SnColorSeparateHasTray    =611;
const int SnAuto1SeparateHasTray    =612;
const int SnAuto2SeparateHasTray    =613;
const int SnAuto3SeparateHasTray    =614;

const int SenLoaderCCWDete_2        =615;
const int SenEmptyCCWDete_2         =616;
const int SenColorCCWDete_2         =617;
const int SenAuto1CCWDete_2         =618;
const int SenAuto2CCWDete_2         =619;
const int SenAuto3CCWDete_2         =620;

const int SnOpenDoorChangeKit1      =621;
const int SnOpenDoorChangeKit2      =622;
const int SnOpenDoorChangeKit3      =623;

const int SnIndexCylinderDetectHead1=624;
const int SnIndexCylinderDetectHead2=625;
const int SnIndexCylinderDetectHead5=626;
const int SnIndexCylinderDetectHead6=627;

const int SnIonBar1                 =628;
const int SnIonBar2                 =629;
const int SnIonBar3                 =630;
const int SnIonBar4                 =631;
const int SnIonBar5                 =632;
const int SnIonBar6                 =633;
const int SnIonBar7                 =634;
const int SnIonBar8                 =635;
const int SnIonBar9                 =636;

const int SnTrayArmHasTray          =637;

const int SnInAreaAlignmentSenZ     =638;                                       //KenHsieh 20211110 ： AutoAlignment add Auto Z
const int SnOutAreaAlignmentSenZ    =639;                                       //KenHsieh 20211110 ： AutoAlignment add Auto Z

const int SnMCUSensor1              =640;                                       //Jimmychiu 20230630 : add color sensor MU-N in Loader
const int SnMCUSensor2              =641;                                       //Jimmychiu 20230630 : add color sensor MU-N in Loader
const int SnMCUSensor3              =642;                                       //Jimmychiu 20230630 : add color sensor MU-N in Loader
const int SnMCUSensor4              =643;                                       //Jimmychiu 20230630 : add color sensor MU-N in Loader

const int SnAuto4_Tray_Car          =644;                                       //Steven 20230907 : For HT-9011UC
const int SnAuto4PreDete            =645;
const int SnAuto4TrackDetect        =646;
const int SnAuto4FixCyPush          =647;
const int SnAuto4EdgePush           =648;
const int SnAuto4UpSafedetect       =649;
const int SnAuto4SeparateHasTray    =650;
const int SnAuto4TrayHasTray        =651;
const int SnAuto4CWPreDetect        =652;
const int SenAuto4CCWDete_2         =653;
const int SnAuto4Z_Select1          =654;
const int SnAuto4Z_Select2          =655;
const int SnAuto4Z_Select3          =656;
const int SnAuto4Z_Select4          =657;
const int SnAuto4IsPreAlarm         =658;
const int SnAuto4IsFull             =659;
const int SnAuto4UpPress            =660;
const int SnAutoColorTrayDetect4    =661;
const int SnAuto4TrayDetect         =662;

const int SnAuto5_Tray_Car          =663;
const int SnAuto5PreDete            =664;
const int SnAuto5TrackDetect        =665;
const int SnAuto5FixCyPush          =666;
const int SnAuto5EdgePush           =667;
const int SnAuto5UpSafedetect       =668;
const int SnAuto5SeparateHasTray    =669;
const int SnAuto5TrayHasTray        =670;
const int SnAuto5CWPreDetect        =671;
const int SenAuto5CCWDete_2         =672;
const int SnAuto5Z_Select1          =673;
const int SnAuto5Z_Select2          =674;
const int SnAuto5Z_Select3          =675;
const int SnAuto5Z_Select4          =676;
const int SnAuto5IsPreAlarm         =677;
const int SnAuto5IsFull             =678;
const int SnAuto5UpPress            =679;
const int SnAutoColorTrayDetect5    =680;
const int SnAuto5TrayDetect         =681;

const int SnAuto6_Tray_Car          =682;
const int SnAuto6PreDete            =683;
const int SnAuto6TrackDetect        =684;
const int SnAuto6FixCyPush          =685;
const int SnAuto6EdgePush           =686;
const int SnAuto6UpSafedetect       =687;
const int SnAuto6SeparateHasTray    =688;
const int SnAuto6TrayHasTray        =689;
const int SnAuto6CWPreDetect        =690;
const int SenAuto6CCWDete_2         =691;
const int SnAuto6Z_Select1          =692;
const int SnAuto6Z_Select2          =693;
const int SnAuto6Z_Select3          =694;
const int SnAuto6Z_Select4          =695;
const int SnAuto6IsPreAlarm         =696;
const int SnAuto6IsFull             =697;
const int SnAuto6UpPress            =698;
const int SnAutoColorTrayDetect6    =699;
const int SnAuto6TrayDetect         =700;

const int SnFixedTray4Detect        =701;
const int SnFixedTray5Detect        =702;
const int SnFixedTray6Detect        =703;
const int SnFix4ColorTrayDetect     =704;
const int SnFix5ColorTrayDetect     =705;
const int SnFix6ColorTrayDetect     =706;

const int SnTrayArmTrayDetect1      =707;
const int SnTrayArmTrayDetect2      =708;
const int SnTrayArmTrayDetect3      =709;
const int SnTrayArmTrayDetect4      =710;

const int SnEPDetect                =711;                                       //kevin 20230608 EP 流量計 偵測
const int SnSafeMode                =712;                                       //jou 20231016 : CE PLC safe mode
const int SnWaterLeakageChiller     =713;                                       //jou 20231019 : Water Leakage Chiller

const int SnIonBar10                =714;
const int SnIonBar11                =715;
const int SnFixFloating3            =716;                                       //Sam 20240129 : 新增第三組 Fix floating Sensor
const int SnTesterAlarm             =717;
const int SnDoubleLoadDetection     =718;                                       //Steven 20240426 : 偵測loader疊盤

const int SnLoader_Detect           =719;                                       //Ifor 20211005 add Tray 載盤上升下降前判斷是否有異常
const int SnEmpty_Detect            =720;
const int SnColor_Detect            =721;
const int SnAuto1_Detect            =722;
const int SnAuto2_Detect            =723;
const int SnAuto3_Detect            =724;
const int SnAuto4_Detect            =725;
const int SnAuto5_Detect            =726;
const int SnAuto6_Detect            =727;
const int SnIonFanCar               =728;                                       //Ifor 20220310 add: Bin Car Ion Fan Check
const int SnIonFanCarPower          =729;                                       //Ifor 20220816 add: Bin Car Ion Fan Power Check
const int SnEPFlowmeter             =730;                                       //Ifor 20220816 add: Bin Car Ion Fan Power Check

const int SnSmokeDetect01           =731;                                       //Sam 20240112 : 新增煙霧偵測

const int SnLoad2TrackDetect        =732;                                       //Steven 20240822 : For HT-9046AU
const int SnLoad2FixCyPush          =733;
const int SnLoad2EdgePush           =734;
const int SnLoad2SeparateHasTray    =735;
const int SnLoad2SureTray           =736;
const int SnLoad2PreDete            =737;
const int SnCheckLoad2Direction     =738;
const int SnLoad2CasstteDetect      =739;
const int SnDoubleLoad2Detection    =740;
const int SnLoad2UpPress            =741;

const int SnAuto1HasCoverTray       =742;                                       //JerryYang 20241021 : Unloader增加第二組Sensor檢查是否有cover tray
const int SnAuto2HasCoverTray       =743;
const int SnAuto3HasCoverTray       =744;
const int SnAuto4HasCoverTray       =745;
const int SnAuto5HasCoverTray       =746;
const int SnAuto6HasCoverTray       =747;

const int SnElectricControlBox      = 748;                                      //ChungHung 20230718 add for Safe plc
const int SnAllSafeDoor             = 749;                                      //ChungHung 20230718 add for Safe plc
const int SnAllEMG                  = 750;                                      //KenHsieh 20250212 : 新增PLC 斷線可瞬間判斷EMG及安全門

const int SnSocket17                =751;
const int SnSocket18                =752;
const int SnSocket19                =753;
const int SnSocket20                =754;
const int SnSocket21                =755;
const int SnSocket22                =756;
const int SnSocket23                =757;
const int SnSocket24                =758;
const int SnLoadIonGun              =759;                                       //Ifor 20230427 add:Loader Ionizer Gun
const int SnLoadLightGat            =760;                                       //Ifor 20230427 add:LD/ULD light gate
const int SnAuto4CasstteDetect      =761;                                       //RogerYang 20250825 : 新增Auto4~6
const int SnAuto5CasstteDetect      =762;
const int SnAuto6CasstteDetect      =763;
const int SnChamberDryAir           =764;                                       //Ifor 20240919 add: Chamber Dry Air

const int SnTopBtmAOIR180           =765;                                       //Jimmychiu 20240322 : Top & Bottom Inspect
const int SnFixedSeatTLOn           =766;
const int SnFixedSeatTLOff          =767;
const int SnFixedSeatTROn           =768;
const int SnFixedSeatTROff          =769;
const int SnFixedSeatBLOn           =770;
const int SnFixedSeatBLOff          =771;
const int SnFixedSeatBROn           =772;
const int SnFixedSeatBROff          =773;
const int SnTopBtmAirMaxAlarm       =774;
const int SnTopBtmAirMinAlarm       =775;
const int SnTopBtmRotateLockOn1     =776;
const int SnTopBtmRotateLockOn2     =777;
const int SnLightZORG               =778;
const int SnLightZINP               =779;
const int SnLightZREADY             =780;
const int SnLightZSERVO             =781;
const int SnLoadCarRFIDSW           =782;                                       //RogerYang 20250828 add for Loader Rotate Arm

const int SnLoaderCarrier1          =783;                                       //Ifor 20251216 add:Boat Carrier
const int SnAuto1Carrier1           =784;                                       //Ifor 20251216 add:Boat Carrier
const int SnAuto2Carrier1           =785;                                       //Ifor 20251216 add:Boat Carrier
const int SnLoaderCarrier2          =786;                                       //Ifor 20251216 add:Boat Carrier
const int SnAuto1Carrier2           =787;                                       //Ifor 20251216 add:Boat Carrier
const int SnAuto2Carrier2           =788;                                       //Ifor 20251216 add:Boat Carrier

const int SnLoaderBoatActDetect     =789;                                       //Ifor 20251216 add:Boat Carrier
const int SnAuto1BoatActDetect      =790;                                       //Ifor 20251216 add:Boat Carrier
const int SnAuto2BoatActDetect      =791;                                       //Ifor 20251216 add:Boat Carrier
const int SnDailyCorrelation_Open   =792;                                       //KaiChen 20200525 ：Daily Correlation Function
const int SnDailyCorrelation_Close  =793;                                       //KaiChen 20200525 ：Daily Correlation Function
const int SnSocket25                =794;                                       //JerryYang 20260205 : add第三組Socket sensor(8+16+8)
const int SnSocket26                =795;
const int SnSocket27                =796;
const int SnSocket28                =797;
const int SnSocket29                =798;
const int SnSocket30                =799;
const int SnSocket31                =800;
const int SnSocket32                =801;

//#define MAX_SENSOR_ITEM 820  <--記得去改!!

int SnAutoTrayCar       [MAX_AUTO_TRAY]={SnAuto1_Tray_Car,       SnAuto2_Tray_Car,       SnAuto3_Tray_Car,       SnAuto4_Tray_Car,       SnAuto5_Tray_Car,       SnAuto6_Tray_Car};
int SnAutoZSelect1      [MAX_AUTO_TRAY]={SnAuto1Z_Select1,       SnAuto2Z_Select1,       SnAuto3Z_Select1,       SnAuto4Z_Select1,       SnAuto5Z_Select1,       SnAuto6Z_Select1};
int SnAutoZSelect2      [MAX_AUTO_TRAY]={SnAuto1Z_Select2,       SnAuto2Z_Select2,       SnAuto3Z_Select2,       SnAuto4Z_Select2,       SnAuto5Z_Select2,       SnAuto6Z_Select2};
int SnAutoZSelect3      [MAX_AUTO_TRAY]={SnAuto1Z_Select3,       SnAuto2Z_Select3,       SnAuto3Z_Select3,       SnAuto4Z_Select3,       SnAuto5Z_Select3,       SnAuto6Z_Select3};
int SnAutoZSelect4      [MAX_AUTO_TRAY]={SnAuto1Z_Select4,       SnAuto2Z_Select4,       SnAuto3Z_Select4,       SnAuto4Z_Select4,       SnAuto5Z_Select4,       SnAuto6Z_Select4};
int SnAutoPreDete       [MAX_AUTO_TRAY]={SnAuto1PreDete,         SnAuto2PreDete,         SnAuto3PreDete,         SnAuto4PreDete,         SnAuto5PreDete,         SnAuto6PreDete};
int SnAutoUpSafe        [MAX_AUTO_TRAY]={SnAutoUpSafedetect0,    SnAutoUpSafedetect1,    SnAutoUpSafedetect2,    SnAuto4UpSafedetect,    SnAuto5UpSafedetect,    SnAuto6UpSafedetect};
int SnAutoDetect        [MAX_AUTO_TRAY]={SnAuto1_Detect,         SnAuto2_Detect,         SnAuto3_Detect,         SnAuto4_Detect,         SnAuto5_Detect,         SnAuto6_Detect};

int SnAutoTrackDetect   [MAX_AUTO_TRAY]={SnAuto1TrackDetect,     SnAuto2TrackDetect,     SnAuto3TrackDetect,     SnAuto4TrackDetect,     SnAuto5TrackDetect,     SnAuto6TrackDetect};
int SnAutoTrayDetect    [MAX_AUTO_TRAY]={SnAuto1TrayDetect,      SnAuto2TrayDetect,      SnAuto3TrayDetect,      SnAuto4TrayDetect,      SnAuto5TrayDetect,      SnAuto6TrayDetect};
int SnAutoColorTrayDete [MAX_AUTO_TRAY]={SnAutoColorTrayDetect0, SnAutoColorTrayDetect1, SnAutoColorTrayDetect2, SnAutoColorTrayDetect4, SnAutoColorTrayDetect5, SnAutoColorTrayDetect6};
int SnAutoSeparate      [MAX_AUTO_TRAY]={SnAuto1SeparateHasTray, SnAuto2SeparateHasTray, SnAuto3SeparateHasTray, SnAuto4SeparateHasTray, SnAuto5SeparateHasTray, SnAuto6SeparateHasTray};
int SnAutoFixCyPush     [MAX_AUTO_TRAY]={SnAuto1FixCyPush,       SnAuto2FixCyPush,       SnAuto3FixCyPush,       SnAuto4FixCyPush,       SnAuto5FixCyPush,       SnAuto6FixCyPush};
int SnAutoEdgePush      [MAX_AUTO_TRAY]={SnAuto1EdgePush,        SnAuto2EdgePush,        SnAuto3EdgePush,        SnAuto4EdgePush,        SnAuto5EdgePush,        SnAuto6EdgePush};
int SnAutoTrayHasTray   [MAX_AUTO_TRAY]={SnAuto1TrayHasTray,     SnAuto2TrayHasTray,     SnAuto3TrayHasTray,     SnAuto4TrayHasTray,     SnAuto5TrayHasTray,     SnAuto6TrayHasTray};
int SnAutoCWPreDetect   [MAX_AUTO_TRAY]={SnAuto1CWPreDetect,     SnAuto2CWPreDetect,     SnAuto3CWPreDetect,     SnAuto4CWPreDetect,     SnAuto5CWPreDetect,     SnAuto6CWPreDetect};
int SnAutoCCWDete       [MAX_AUTO_TRAY]={SenAuto1CCWDete_2,      SenAuto2CCWDete_2,      SenAuto3CCWDete_2,      SenAuto4CCWDete_2,      SenAuto5CCWDete_2,      SenAuto6CCWDete_2};
int SnAutoIsPreAlarm    [MAX_AUTO_TRAY]={SnAuto1IsPreAlarm,      SnAuto2IsPreAlarm,      SnAuto3IsPreAlarm,      SnAuto4IsPreAlarm,      SnAuto5IsPreAlarm,      SnAuto6IsPreAlarm};
int SnAutoIsFull        [MAX_AUTO_TRAY]={SnAuto1IsFull,          SnAuto2IsFull,          SnAuto3IsFull,          SnAuto4IsFull,          SnAuto5IsFull,          SnAuto6IsFull};
int SnAutoUpPress       [MAX_AUTO_TRAY]={SnAuto1UpPress,         SnAuto2UpPress,         SnAuto3UpPress,         SnAuto4UpPress,         SnAuto5UpPress,         SnAuto6UpPress};
int SnAutoHasCoverTray  [MAX_AUTO_TRAY]={SnAuto1HasCoverTray,    SnAuto2HasCoverTray,    SnAuto3HasCoverTray,    SnAuto4HasCoverTray,    SnAuto5HasCoverTray,    SnAuto6HasCoverTray};

int SnAutoBoatActDetect [2]={SnAuto1BoatActDetect, SnAuto2BoatActDetect};       //RogerYang 20260202 : Add for CR

int iC_SeparateRL       [MAX_AUTO_TRAY]={C_Auto1SeparateRL,      C_Auto2SeparateRL   ,   C_Auto3SeparateRL      ,C_Auto4SeparateRL,      C_Auto5SeparateRL,      C_Auto6SeparateRL};
int iC_SeparateFL       [MAX_AUTO_TRAY]={C_Auto1SeparateFL,      C_Auto2SeparateFL   ,   C_Auto3SeparateFL      ,C_Auto4SeparateFL,      C_Auto5SeparateFL,      C_Auto6SeparateFL};
int iC_SeparateRR       [MAX_AUTO_TRAY]={C_Auto1SeparateRR,      C_Auto2SeparateRR   ,   C_Auto3SeparateRR      ,C_Auto4SeparateRR,      C_Auto5SeparateRR,      C_Auto6SeparateRR};
int iC_SeparateFR       [MAX_AUTO_TRAY]={C_Auto1SeparateFR,      C_Auto2SeparateFR   ,   C_Auto3SeparateFR      ,C_Auto4SeparateFR,      C_Auto5SeparateFR,      C_Auto6SeparateFR};

int SnFixColorTrayDete  [MAX_FIX_TRAY] ={SnFixColorTrayDetect0,  SnFixColorTrayDetect1,  SnFixColorTrayDetect2,  SnFix4ColorTrayDetect,  SnFix5ColorTrayDetect,  SnFix6ColorTrayDetect};
int SnFixedTrayDetect   [MAX_FIX_TRAY] ={SnFixedTray1Detect,     SnFixedTray2Detect,     SnFixedTray3Detect,     SnFixedTray4Detect,     SnFixedTray5Detect,     SnFixedTray6Detect};
//====================================================================
//Sensor define end
//====================================================================

const int iSafeDoor[MAX_SAFE_DOOR_CNT]={SnSafeDoor1, SnSafeDoor2, SnSafeDoor3, SnSafeDoor4, SnSafeDoor5,
                                        SnSafeDoor6, SnSafeDoor7, SnSafeDoor8, SnSafeDoor9, SnSafeDoor10,
                                        SnSafeDoor11, SnSafeDoor12, SnSafeDoor13, SnSafeDoor14, SnSafeDoor15,
                                        SnSafeDoor16, SnSafeDoor17, SnSafeDoor18, SnSafeDoor19, SnSafeDoor20,
                                        SnSafeDoor21, SnMagazineSafeDoor, SnHeaterDoor, SnHeaterDoor2, SnHeaterDoor3,
                                        SnHeaterDoor4, SnMagazineSafeDoor2, SnMagazineSafeDoor3, SnOHTIntoLoadPort, SnOHTIntoLoadPort2};

AnsiString asSafeDoorAlarm[MAX_SAFE_DOOR_CNT]={"MES1611", "MES1612", "MES1613", "MES1614", "MES1615",  //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
                                               "MES1616", "MES1617", "MES1618", "MES1619", "MES1620",
                                               "MES1621", "MES1622", "MES1623", "MES1624", "MES1629",
                                               "MES1657", "MES1658", "MES1659", "MES1660", "MES1661",
                                               "MES1662", "MES16429", "MES1625", "MES1626", "MES1627",
                                               "MES1628", "MES16431", "MES16432", "MES16433", "MES16434"};

const int iSafeDoorPosition[MAX_SAFE_DOOR_CNT]={MMSafeDoor1,  MMSafeDoor2,  MMSafeDoor3,  MMSafeDoor4,  MMSafeDoor5,     //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
                                                MMSafeDoor6,  MMSafeDoor7,  MMSafeDoor8,  MMSafeDoor9,  MMSafeDoor10,
                                                MMSafeDoor11, MMSafeDoor12, MMSafeDoor13, MMSafeDoor14, MMSafeDoor15,
                                                MMSafeDoor16, MMSafeDoor17, MMSafeDoor18, MMSafeDoor19, MMSafeDoor20,
                                                MMSafeDoor21, MMSafeDoor22, MMSystem, MMSystem, MMSystem,
                                                MMSystem, MMSystem, MMSystem, MMSystem, MMSystem};

const int iSafeDoorHatchway[MAX_HATCH_DOOR_CNT]={SnSafeDoor1Hatchway, SnSafeDoor2Hatchway, SnSafeDoor3Hatchway, SnSafeDoor4Hatchway, SnSafeDoor5Hatchway,
                                                 SnSafeDoor6Hatchway, SnSafeDoor7Hatchway, SnSafeDoor8Hatchway, SnSafeDoor9Hatchway, SnSafeDoor10Hatchway,
                                                 SnSafeDoor11Hatchway, SnSafeDoor6PosFixPickPlace};

AnsiString asHatchwaySafeDoorAlarm[MAX_HATCH_DOOR_CNT]={"MES1657", "MES1658", "MES1659", "MES1660", "MES1661",
                                                        "MES1662", "MES1663", "MES1664", "MES1665", "MES1666",
                                                        "MES1667", "MES1668"};

const int iHatchwaySafeDoorPosition[MAX_HATCH_DOOR_CNT]={MMSafeDoor1,  MMSafeDoor2,  MMSafeDoor3,  MMSafeDoor4,  MMSafeDoor5,
                                                         MMSafeDoor6,  MMSafeDoor7,  MMSafeDoor8,  MMSafeDoor9,  MMSafeDoor10,
                                                         MMSafeDoor11, MMSafeDoor6};

const int iIonFan[MAX_IONFAN]={SnIonFanAlarm, SnIonFanLevelAlarm, SnIonBarrierAlarm, SnIonBarrierLevelAlarm,
                               SnIonBarrierConditionAlarm, SnIonFan6Alarm, SnIonFan7Alarm, SnIonFan8Alarm,
                               SnIonFan9Alarm, SnIonFan10Alarm, SnIonFan11Alarm, SnIonFan12Alarm}; //Steven 20100226

const int iHTIonBar[MAX_HTIONFAN]={SnIonBarrierLevelAlarm, SnIonBarrierConditionAlarm, SnIonFan8Alarm};    //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan

const int iIonFanPower[MAX_IONFAN]={SnIonFanPower01, SnIonFanPower02, SnIonFanPower03, SnIonFanPower04,
                                    SnIonFanPower05, SnIonFanPower06, SnIonFanPower07, SnIonFanPower08,
                                    SnIonFanPower09, SnIonFanPower10, SnIonFanPower11 , SnIonFanPower12
                                   };                                           //Steven 20130201 : Kasuga離子風扇電源偵測

const int iIonBar[MAX_IONBAR]={SnIonBar1, SnIonBar2, SnIonBar3, SnIonBar4, SnIonBar5,
                               SnIonBar6, SnIonBar7, SnIonBar8, SnIonBar9};                  //Ztex 2024.12.22 Add For HT1032AT IonBar

const int BackSenBit0               =0;
const int BackSenBit1               =1;
const int BackSenBit2               =2;
const int BackSenBit3               =3;
const int BackSenBit4               =4;
const int BackSenBit5               =5;
const int BackSenBit6               =6;
const int BackSenBit7               =7;
const int BackSenBit8               =8;
const int BackSenBit9               =9;
const int BackSenBit10              =10;
const int BackSenBit11              =11;
const int BackSenBit12              =12;
const int BackSenBit13              =13;
const int BackSenBit14              =14;
const int BackSenBit15              =15;
const int BackSenBit16              =16;
const int BackSenBit17              =17;
const int BackSenBit18              =18;
const int BackSenBit19              =19;
//Alick 20161011 (Steven) : TTL支援8Site
const int BackSenBit20              =20;
const int BackSenBit21              =21;
const int BackSenBit22              =22;
const int BackSenBit23              =23;
const int BackSenBit24              =24;
const int BackSenBit25              =25;
const int BackSenBit26              =26;
const int BackSenBit27              =27;
const int BackSenBit28              =28;
const int BackSenBit29              =29;
const int BackSenBit30              =30;
const int BackSenBit31              =31;
const int BackSenBit32              =32;
const int BackSenBit33              =33;
const int BackSenBit34              =34;
const int BackSenBit35              =35;
const int BackSenBit36              =36;
const int BackSenBit37              =37;
const int BackSenBit38              =38;
const int BackSenBit39              =39;

//MAX_TTL_BIT 40  <--記得去改!!

//====================================================================
//Switch define start
//====================================================================
const int SwFKPowerOff                  =0;
const int SwFKPowerOn                   =1;
const int SwFKReset                     =2;
const int SwFKPause                     =3;
const int SwFKHome                      =4;
const int SwFKStart                     =5;
const int SwFKOneCycle                  =6;
const int SwFKRetry                     =7;
const int SwFKSkip                      =8;
const int SwFKCleanOut                  =9;
const int SwFKTrayFeed                  =10;
const int SwFKTrayEnd                   =11;
const int SwFKAlarmReset                =12;
const int SwFKCoverOpen                 =13;
const int SwRKPowerOff                  =14;
const int SwRKPowerOn                   =15;
const int SwRKReset                     =16;
const int SwRKPause                     =17;
const int SwRKHome                      =18;
const int SwRKStart                     =19;
const int SwRKOneCycle                  =20;
const int SwRKRetry                     =21;
const int SwRKSkip                      =22;
const int SwRKCleanOut                  =23;
const int SwRKTrayFeed                  =24;
const int SwRKTrayEnd                   =25;
const int SwRKAlarmReset                =26;
const int SwRKCoverOpen                 =27;
const int SwRKManualStep                =28;
const int SwRKManualTStart              =29;
const int SwTowerRed                    =30;
const int SwTowerGreen                  =31;
const int SwTowerYellow                 =32;
const int SwFMotorBreaker               =33;
const int SwBMotorBreaker               =34;
const int SwMotorRelay                  =35;
const int SwHeaterRelay                 =36;
const int SwMusic1                      =37;
const int SwMusic2                      =38;
const int SwMusic3                      =39;
const int SwMusic4                      =40;
const int SwTestPassLed                 =41;
const int SwTestFailLed                 =42;
const int SwFrontActiveLed              =43;
const int SwRearActiveLed               =44;
const int SwClear0                      =45;                                    //Clear All
const int SwClear1                      =46;                                    //Anti-Start Signal
const int SwStart0                      =47;
const int SwStart1                      =48;
const int SwACTrayY                     =49;
const int SwACAuto1                     =50;
const int SwACAuto2                     =51;
const int SwACAuto3                     =52;
const int SwServerON                    =53;
const int SwManualZ1                    =54;
const int SwManualZ2                    =55;

const int SwACEmptyCW                   =56;
const int SwACEmptyCCW                  =57;
const int SwACColorCW                   =58;
const int SwClear2                      =59;                                    //Start Enable
const int SwClear3                      =60;                                    //Reserve
const int SwStart2                      =61;
const int SwStart3                      =62;
const int SwCCDAir                      =63;

const int SwACAutoCW                    =64;                                    // for auto as loader use
const int SwACAuto2CW                   =65;                                    //Auto2進Tray
const int SwACAuto3CW                   =66;                                    //Auto3進Tray
const int SwACAuto1Mode                 =67;                                    // for auto as loader use
const int SwReadTorue                   =68;
const int SwBigFan                      =69;
const int SwDut0                        =70;
const int SwDut1                        =71;
const int SwDut2                        =72;
const int SwDut3                        =73;
const int Sw10Bit                       =74;
const int SwAuto1SelectSlow             =75;
const int SwCCDDestroy                  =76;

const int SwSafeLock                    =77;
const int SwCCDLight                    =78;
const int SwHeaterFan                   =79;
const int SwSocketClean                 =80;
const int SwACColorCCW                  =81;
const int SwShuttleFan                  =82;

const int SwTesterDoubleContact         =83;
const int SwTesterPower                 =84;

const int SwACEmpty1CW                  =85;
const int SwACEmpty1CCW                 =86;
const int SwDutHeaterCoolFan            =87;

const int SwLoaderBin                   =88;
const int SwEmpty1Bin                   =89;
const int SwEmpty2Bin                   =90;
const int SwAuto1Bin                    =91;
const int SwAuto2Bin                    =92;
const int SwAuto3Bin                    =93;

const int SwFix1Bin                     =94;                                    //Steven 20090917 Start : 加入Fix的數字顯示器
const int SwFix2Bin                     =95;
const int SwFix3Bin                     =96;
const int SwFix4Bin                     =97;
const int SwFix5Bin                     =98;
const int SwFix6Bin                     =99;

const int SwZ1SuckMode0                 =100;                                   //Steven 20090916 Start : 加入1x2模式的Index電磁閥開關
const int SwZ1SuckMode1                 =101;
const int SwZ2SuckMode0                 =102;
const int SwZ2SuckMode1                 =103;

const int SwShuttleCooling              =104;                                   //jou 2010-06-09
const int SwCCDCooling                  =105;                                   //Steven 20110705
const int SwEpArm1                      =106;                                   //Steven 20110708
const int SwEpArm2                      =107;                                   //Steven 20110708
const int SwHeaterFanSpeed              =108;                                   //Steven 20110725
const int SwRotateCheckClear            =109;                                   //ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor

const int SwEP_D0                       =110;                                   //Dell 20111111 Start : 加入Digital E/P
const int SwEP_D1                       =111;
const int SwEP_D2                       =112;
const int SwEP_D3                       =113;
const int SwEP_D4                       =114;
const int SwEP_D5                       =115;
const int SwEP_D6                       =116;
const int SwEP_D7                       =117;
const int SwEP_D8                       =118;
const int SwEP_D9                       =119;

const int SwCoolingFan_Blower           =120;                                   //20111130  Dell
const int SwSafeDoorLock                =121;                                   //20111130  Dell
const int SwIndexIonFan                 =122;                                   //jou 2012-03-13 index離子槍出風開關控制

const int SwIndexChangeToque1           =123;                                   //jou 2012-06-21 Enable index I/O Change Toque
const int SwIndexChangeToque2           =124;                                   //jou 2012-06-21 Enable index I/O Change Toque

const int SwACLoaderCCW                 =125;                                   //Loader退Tray

const int SwHeatGun                     =126;                                   //ChungHung 20121107 add
const int SwVacuumPumpTogetherOn        =127;                                   //Dell  for HT9046LS 雙幫浦模式
const int SwAirConditioner              =128;                                   //Steven 20131011 : 冷氣機
const int SwHotplateCooling             =129;                                   //jou 2013-11-07
const int SwCarRecord                   =130;                                   //wei 2013-12-09

const int SwLoad2Bin                    =131;                                   //Steven 20240822 : For HT-9046AU
const int SwLoad2AirClean               =132;
const int SwACLoad2CCW                  =133;
const int SwACTray2Y                    =134;
const int SwLoad2Vibration              =135;
//const int Sw24VPowerOn_Site06           =136;
//const int Sw24VPowerOn_Site07           =137;
//const int Sw24VPowerOn_Site08           =138;
//const int Sw24VPowerOn_Site09           =139;
//const int Sw24VPowerOn_Site10           =140;
//const int Sw24VPowerOn_Site11           =141;
//const int Sw24VPowerOn_Site12           =142;
//const int Sw24VPowerOn_Site13           =143;
//const int Sw24VPowerOn_Site14           =144;
//const int Sw24VPowerOn_Site15           =145;
//const int Sw24VPowerOn_Site16           =146;
//const int Sw24VPowerOn_Site17           =147;
//const int Sw24VPowerOn_Site18           =148;
//const int Sw24VPowerOn_Site19           =149;
//const int Sw24VPowerOn_Site20           =150;
//const int Sw24VPowerOn_Site21           =151;
//const int Sw24VPowerOn_Site22           =152;
//const int Sw24VPowerOn_Site23           =153;
//const int Sw24VPowerOn_Site24           =154;
//const int Sw24VPowerOn_Site25           =155;
//const int Sw24VPowerOn_Site26           =156;
//const int Sw24VPowerOn_Site27           =157;
//const int Sw24VPowerOn_Site28           =158;
//const int Sw24VPowerOn_Site29           =159;
//const int Sw24VPowerOn_Site30           =160;
//const int Sw24VPowerOn_Site31           =161;
//const int Sw24VPowerOn_Site32           =162;

const int SwUnDock                      =163;                                   //Steven 20140310 : One Touch Docking
const int SwDockError                   =164;                                   //Steven 20140310 : One Touch Docking
const int SwStartTest1                  =165;                                   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SwStartTest2                  =166;                                   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SwStartTest3                  =167;                                   //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int SwOCRTigger                   =168;                                   //wei 20150720 OCR觸發
const int SwTesterAirCooling            =169;                                   //jou 2016-01-07 Tester Air Cooling Switch
const int SwFanDirection                =170;                                   //wei 20160215大風扇方向

//------------------------------------
//Steven 20161011 : TTL支援8Site
//------------------------------------
const int SwClear4                      =171;                                   //Clear All
const int SwClear5                      =172;                                   //Anti-Start Signal
const int SwClear6                      =173;                                   //Start Enable
const int SwClear7                      =174;                                   //Reserve
const int SwStart4                      =175;
const int SwStart5                      =176;
const int SwStart6                      =177;
const int SwStart7                      =178;
const int SwDut4                        =179;
const int SwDut5                        =180;
const int SwDut6                        =181;
const int SwDut7                        =182;
const int Sw10Bit2                      =183;
const int SwIonBarPower                 =184;                                   //kevin 20170310 (wei) Ion bar power
const int SwShuttleVibration1           =185;                                   //JerryYang 20171006 (wei) Shuttle 震動馬達
const int SwShuttleVibration2           =186;

const int SwDieClean                    =187;

const int SwLightOff                    =188;
const int SwAirOff                      =189;

const int SwCaselevatorZ                =190;
const int SwTrayBracketZ                =191;

const int SwE84VALID                    =192;
const int SwE84CS0                      =193;
const int SwE84CS1                      =194;
const int SwE84AMAVBL                   =195;
const int SwE84TRREQ                    =196;
const int SwE84BUSY                     =197;
const int SwE84COMPT                    =198;
const int SwE84CONT                     =199;
const int SwE84GO                       =200;

const int SwE84LREQ                     =192;
const int SwE84UREQ                     =193;
const int SwE84VA                       =194;
const int SwE84READY                    =195;
const int SwE84VS0                      =196;
const int SwE84VS1                      =197;
const int SwE84HOAVBL                   =198;
const int SwE84ES                       =199;
const int SwE84POWER                    =200;

const int SwCassette01                  =201;                                   //wei 20180702 MR
const int SwCassette02                  =202;                                   //wei 20180702 MR
const int SwCassette03                  =203;                                   //wei 20180702 MR
const int SwCassette04                  =204;                                   //wei 20180702 MR
const int SwCassette05                  =205;                                   //wei 20180702 MR
const int SwCassette06                  =206;                                   //wei 20180702 MR
const int SwCassette07                  =207;                                   //wei 20180702 MR
const int SwCassette08                  =208;                                   //wei 20180702 MR
const int SwCassette09                  =209;                                   //wei 20180702 MR
const int SwCassette10                  =210;                                   //wei 20180702 MR
const int SwMRStart                     =211;                                   //wei 20180702 MR
const int SwMRPause                     =212;                                   //wei 20180702 MR
const int SwMRUp                        =213;                                   //wei 20180702 MR
const int SwMRDown                      =214;                                   //wei 20180702 MR

const int SwMRError                     =215;                                   //wei 20180702 MR
const int SwMRUnLoadReady               =216;                                   //wei 20180702 MR
const int SwMRPresence                  =217;                                   //wei 20180702 MR
const int SwMRPlacement                 =218;                                   //wei 20180702 MR
const int SwMRLoadReady                 =219;                                   //wei 20180702 MR
const int SwMRManualMode                =220;                                   //wei 20180702 MR
const int SwMRLight                     =221;                                   //wei 20180702 MR

const int SwBufferArmZ                  =222;                                   //wei 20200302 MR

const int SwTrayBracket2Z               =223;                                   //KaiChen 20200716 ：OHT
const int SwCaselevator2Z               =224;
const int SwMultileEmptyZ               =225;

const int SwOHT_UnloadReady_1           =226;
const int SwOHT_CarrierPresence_1       =227;
const int SwOHT_CarrierPlacement_1      =228;
const int SwOHT_LoadReady_1             =229;
const int SwOHT_ManualMode_1            =230;
const int SwOHT_AutoMode_1              =231;
const int SwOHT_Error_1                 =232;

const int SwOHT_UnloadReady_2           =233;
const int SwOHT_CarrierPresence_2       =234;
const int SwOHT_CarrierPlacement_2      =235;
const int SwOHT_LoadReady_2             =236;
const int SwOHT_ManualMode_2            =237;
const int SwOHT_AutoMode_2              =238;
const int SwOHT_Error_2                 =239;

const int SwMultileEmpty_ScanTray_Open  =240;
const int SwMultileEmpty_ScanTray_Close =241;
const int SwMultileEmpty_ScanTrayID     =242;

const int SwE84_2_LREQ                  =252;
const int SwE84_2_UREQ                  =253;
const int SwE84_2_VA                    =254;
const int SwE84_2_READY                 =255;
const int SwE84_2_VS0                   =256;
const int SwE84_2_VS1                   =257;
const int SwE84_2_HOAVBL                =258;
const int SwE84_2_ES                    =259;
const int SwE84_2_POWER                 =260;

const int SwE84_1_LREQ                  =261;
const int SwE84_1_UREQ                  =262;
const int SwE84_1_VA                    =263;
const int SwE84_1_READY                 =264;
const int SwE84_1_VS0                   =265;
const int SwE84_1_VS1                   =266;
const int SwE84_1_HOAVBL                =267;
const int SwE84_1_ES                    =268;
const int SwE84_1_POWER                 =269;

const int SwSafeDoorLock_LoadPort1      =270;
const int SwSafeDoorLock_LoadPort2      =271;

const int SwLoadDoorLock                =272;                                   //KaiChen 20200716 ：OHT
const int SwLoadRobotZ                  =273;
const int SwUnloadDoorLock              =274;
const int SwUnloadRobotZ                =275;

const int SwPurgeAir                    =276;                                   //kevin 20180928 add load board blower
const int SwCDAGun                      =277;                                   //Steven 20181012 : 使用熱風槍吹冷風
const int SwLoadCellA                   =278;                                   //kevin 20190306  add load Cell  read 1
const int SwLoadCellB                   =279;                                   //kevin 20190306  add load Cell  read 2
const int SwSafeDoorLockLM              =280;

const int SwIonRelay                    =281;                                   //Ifor 20190114 : add Chamber 開啟時 關閉 Ion 離子槍 吹氣
const int SwSocketClean2                =282;                                   //JerryYang 20190715 Clean air arm1 arm2分開控制
const int SwLoaderVibration             =283;                                   //JerryYang 20191001 loader震動馬達
const int SwTesterDryAirSwitch          =284;                                   //Ifor 20200115 : add Tester Dry Air Control    //Ifor 20200420 232 -> 233
const int SwIndEpArm1                   =285;
const int SwIndEpArm2                   =286;
const int SwAutoCoolDown                =287;                                   //kevin 20201223 AutoCool down  Chambo 高溫轉常溫快速降溫

const int SwIonFanClean                 =288;                                   //Isaac 20210609 : IO觸發IonFan清針

const int SwTjSignal01                  =289;                                   //Ifor 20210622 add: ATC Switch TJ
const int SwTjSignal02                  =290;
const int SwTjSignal03                  =291;
const int SwTjSignal04                  =292;
const int SwTjSignal05                  =293;
const int SwTjSignal06                  =294;
const int SwTjSignal07                  =295;
const int SwTjSignal08                  =296;

const int SwDieCleanSuck                =297;                                   //wei 20210326
const int SwRKSafeLock                  =298;                                   //KenHsieh 20211228 : 區分實體IO與通訊面板
const int SwATCHeatGun                  =299;                                   //JerryYang 20220408 : add for ATC3.5
const int SwLBAir                       =300;                                   //JerryYang 20220923 : LB吹氣function

const int SwDryAirSwitch                =301;                                   //Ztex 2023.04.13 Add HT-1032 IO
const int SwColdAirSwitch               =302;                                   //Ztex 2023.04.13 Add HT-1032 IO
const int SwTriTempSafeDoor6Lock        =303;                                   //Ztex 2023.04.13 Add HT-1032 IO
const int SwEnhaustAirFanPowerOn        =304;                                   //Ztex 2023.04.26 Add HT-1032 IO Exhaust Air
const int SwMagazineMotorBreaker        =305;                                   //JerryYang 20220909 : add magazine
const int SwMagazineSafeDoorLock        =306;
const int SwMagazineSafeDoor2LockOn     =307;
const int SwMagazineSafeDoor2LockOff    =308;

const int SwESDAntennaRelay1            =309;
const int SwESDAntennaRelay2            =310;
const int SwESDAntennaRelay3            =311;
const int SwESDAntennaRelay4            =312;
const int SwESDAntennaRelay5            =313;
const int SwESDAntennaRelay6            =314;
const int SwESDAntennaRelay7            =315;
const int SwESDAntennaRelay8            =316;

const int SwLoaderAirClean              =317;

const int SwACAuto4                     =318;                                   //Steven 20230907 : For HT-9011UC
const int SwACAuto4CW                   =319;
const int SwACAuto5                     =320;
const int SwACAuto5CW                   =321;
const int SwACAuto6                     =322;
const int SwACAuto6CW                   =323;

const int SwSafeDoor1HatchwayLock       =324;                                   //ChungHung 20230718 add for Safe plc start
const int SwSafeDoor2HatchwayLock       =325;
const int SwSafeDoor3HatchwayLock       =326;
const int SwSafeDoor4HatchwayLock       =327;
const int SwSafeDoor5HatchwayLock       =328;
const int SwSafeDoor6HatchwayLock       =329;
const int SwSafeDoor7HatchwayLock       =330;
const int SwSafeDoor8HatchwayLock       =331;

const int SwElectricControlBoxLock1     =332;
const int SwElectricControlBoxLock2     =333;
const int SwElectricControlBoxLock3     =334;
const int SwElectricControlBoxLock4     =335;
const int SwElectricControlBoxLock5     =336;
const int SwElectricControlBoxLock6     =337;
const int SwElectricControlBoxLock7     =338;                                   //ChungHung 20230718 add for Safe plc end

const int SwInArmZBreaker               =339;                                   //Frank 20250812 : modify InOutArm1Motor
const int SwOutArmZBreaker              =340;                                   //Frank 20250812 : modify InOutArm1Motor

const int SwBottomBlower                =341;
const int SwFixedSeatTLOn               =342;                                   //Jimmychiu 20240322 : Top & Bottom Inspect
const int SwFixedSeatTLOff              =343;
const int SwFixedSeatTROn               =344;
const int SwFixedSeatTROff              =345;
const int SwFixedSeatBLOn               =346;
const int SwFixedSeatBLOff              =347;
const int SwFixedSeatBROn               =348;
const int SwFixedSeatBROff              =349;
const int SwCCDZBreaker                 =350;
const int SwTopBtmRotateLockOn          =351;
const int SwTopBtmRotateLockOff         =352;
const int SwLightStart                  =353;
const int SwPRGSEL0                     =354;
const int SwPRGSEL1                     =355;
const int SwPRGSEL2                     =356;
const int SwPRGSEL3                     =357;
const int SwPRGSEL4                     =358;
const int SwPRGSEL5                     =359;
const int SwPRGSEL6                     =360;
const int SwLightOrg                    =361;
const int SwMultiEp                     =362;                                   //Ifor 20250618 add:Auto Switch Multi EP

const int SwLoadCarRFIDZBreaker         =363;                                   //RogerYang 20250828 add for Loader Rotate Arm
const int SwCassetteLDMotBreaker        =364;                                   //Ifor 20251216 add:Boat Carrier
const int SwCassetteAuto1MotBreaker     =365;                                   //Ifor 20251216 add:Boat Carrier
const int SwCassetteAuto2MotBreaker     =366;                                   //Ifor 20251216 add:Boat Carrier

const int SwDryAirUseHandler            =367;
const int SwDryAirUseATCCar             =368;
//AI(W906-BRAKE-EMPTY-AUTO3) 20260930: EastSun「目前M40不會觸發」—— HT9050 的 IO_Table 有 5 顆卡匣 Z 煞車（st16 ch24..28），
//  golden 只有 LD／Auto1／Auto2 三顆（Ifor 20251216）。沒有 SW[] 物件的名字 InitialSwitch 找不到、永遠不會被驅動，所以補這兩顆。
//  golden V906／V912 都沒有這兩個名字（20260930 查過），編號接在 368 後面。
const int SwCassetteEmptyMotBreaker     =369;                                   //M36 MEmptyZ（IO 頁 1031）
const int SwCassetteAuto3MotBreaker     =370;                                   //M40 MAuto3Z（IO 頁 1034）

//MAX_SWITCH_ITEM 372  <--記得去改!!   （AI(W906-BRAKE-EMPTY-AUTO3) 20260930：原 370）

int SwAutoCCW[MAX_AUTO_TRAY]={SwACAuto1,   SwACAuto2,   SwACAuto3,   SwACAuto4,   SwACAuto5,   SwACAuto6};
int SwAutoCW [MAX_AUTO_TRAY]={SwACAutoCW,  SwACAuto2CW, SwACAuto3CW, SwACAuto4CW, SwACAuto5CW, SwACAuto6CW};

//====================================================================
//Switch define end
//====================================================================

const int BackSwStart0                  =0;
const int BackSwStart1                  =1;
const int BackSwStart2                  =2;
const int BackSwStart3                  =3;
//Alick 20161011 (Steven) : TTL支援8Site
const int BackSwStart4                  =4;
const int BackSwStart5                  =5;
const int BackSwStart6                  =6;
const int BackSwStart7                  =7;
const int BackSwDut0                    =8;
const int BackSwDut1                    =9;
const int BackSwDut2                    =10;
const int BackSwDut3                    =11;
const int BackSwDut4                    =12;
const int BackSwDut5                    =13;
const int BackSwDut6                    =14;
const int BackSwDut7                    =15;

//MAX_TTL_SWITCH 16  <--記得去改!!

//====================================================================
//Motor define start
//====================================================================
const int MInArmX       =0;
const int MInArmY       =1;
const int MInArmPitch   =2;
const int MInArmZA      =3;
const int MInArmZB      =4;
const int MInArmZC      =5;
const int MInArmZD      =6;
const int MInArmZE      =7;
const int MInArmZF      =8;
const int MInArmZG      =9;
const int MInArmZH      =10;
const int MInShuttle1   =11;
const int MInShuttle2   =12;
const int MTestY1       =13;
const int MTestZ1       =14;
const int MTestZ2       =15;
const int MTestY2       =16;
const int MOutShuttle1  =17;
const int _CCDX         =17;
const int MOutShuttle2  =18;
const int MOutArmX      =19;
const int MOutArmY      =20;
const int MOutArmPitch  =21;
const int MOutArmZA     =22;
const int MOutArmZB     =23;
const int MOutArmZC     =24;
const int MOutArmZD     =25;
const int MOutArmZE     =26;
const int MOutArmZF     =27;
const int MOutArmZG     =28;
const int MOutArmZH     =29;
const int MTrayX        =30;
const int MInArmPitchY  =31;                                                    //Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch
const int MInArmPitchX2 =32;                                                    //Steven 20131002 : XY變距
const int MOutArmPitchY =33;                                                    //Steven 20131002 : XY變距 //ChungHung 20131231 alter AutoYPitch
const int MOutArmPitchX2=34;                                                    //Steven 20131002 : XY變距
const int MLoaderZ      =35;
const int MEmptyZ       =36;
const int MColorZ       =37;
const int MAuto1Z       =38;
const int MAuto2Z       =39;
const int MAuto3Z       =40;
const int MInRotateKit  =41;                                                    //2013-04-12    Dell :旋轉站;馬達版
const int MOutRotateKit =42;                                                    //2013-04-12    Dell :旋轉站;馬達版
const int MAOIKit       =43;                                                    //2014-03-04    Dell    for SPIL WLP Add 5S Inspection
const int MLoaderY      =44;                                                    //Steven 20150910 : Add for OCR
const int MEmptyY       =45;
const int MColorY       =46;
const int MAuto1Y       =47;
const int MAuto2Y       =48;
const int MAuto3Y       =49;
const int MInArmZAe     =50;                                                    //Steven 20230323 : For HT1032
const int MInArmPitchX3 =51;
const int MInArmPitchX4 =52;
const int MInArmZAf     =53;
const int MOutArmPitchX3=54;
const int MOutArmPitchX4=55;
const int MTrayZ        =56;
//const int MTapeOutR     =57;
//const int MTapeOutZ     =58;
const int MOutSortAa    =57;                                                    //Steven 20240822 : For HT-9046AU
const int MOutSortAb    =58;
const int MInArmXScale  =59;                                                    //Steven 20160426 : 磁性尺
const int MInArmYScale  =60;
const int MOutArmXScale =61;
const int MOutArmYScale =62;
const int MShuttle1Pitch=63;
const int MShuttle2Pitch=64;                                                    //wei 20160914 Auto Shuttle Sensor

const int MInRotateB    =65;                                                    //Steven 20170329 (Wei) : Add individual rotate motor
const int MInRotateC    =66;
const int MInRotateD    =67;
const int MInRotateE    =68;
const int MInRotateF    =69;
const int MInRotateG    =70;
const int MInRotateH    =71;
const int MOutRotateB   =72;
const int MOutRotateC   =73;
const int MOutRotateD   =74;
const int MOutRotateE   =75;
const int MOutRotateF   =76;
const int MOutRotateG   =77;
const int MOutRotateH   =78;
const int MLightScale   =79;

int MInRotate[MAX_ARM_Row][MAX_ARM_Col];                                        //Steven 20170329 (Wei) : Add individual rotate motor
int MOutRotate[MAX_ARM_Row][MAX_ARM_Col];

const int MInArmZAg     =80;                                                    //Steven 20230323 : For HT1032
const int MInArmZAh     =81;

const int MArmAlignment =82;                                                    //Steven 20240507 : 只是為了Teaching存檔方便
const int MLoadHingeR   =83;                                                    //Steven 20170330 (Wei) : For TSMC
const int MLoadHingeZ   =84;                                                    //Steven 20170330 (Wei) : For TSMC
const int MPreciser     =85;                                                    //Steven 20180212 (Wei) : 定位器

const int MInArmZBe     =86;
const int MInArmZBf     =87;
const int MInArmZBg     =88;
const int MInArmZBh     =89;

const int MOutArmZAe    =90;
const int MOutArmZAf    =91;
const int MOutArmZAg    =92;
const int MOutArmZAh    =93;
const int MOutArmZBe    =94;
const int MOutArmZBf    =95;
const int MOutArmZBg    =96;
const int MOutArmZBh    =97;

const int MOutSortX     =98;                                                    //Steven 20240822 : For HT-9046AU
const int MCaselevatorZ =99;                                                    //wei 20180702 MR
const int MCasArmX      =100;                                                   //wei 20180702 MR
const int MCasArmZ      =101;                                                   //wei 20180702 MR
const int MTrayBracketZ =102;                                                   //wei 20180702 MR
const int MStackedTrayX =103;                                                   //wei 20180702 MR
const int MStackedTrayZ =104;                                                   //wei 20180702 MR
const int MUnloadRobotZ =105;                                                   //Sam 20190112 LM
const int MOutSortY     =106;

const int MCCDX         =107;
const int MCCDY         =108;
const int MCCDZ         =109;

const int MInFlipper1   =110;                                                   //Frank 20210612 : Flipper Function
const int MInFlipper2   =111;                                                   //Frank 20210612 : Flipper Function
const int MInFlipper3   =112;                                                   //Frank 20210612 : Flipper Function
const int MOutFlipper1  =113;                                                   //Frank 20210612 : Flipper Function
const int MOutFlipper2  =114;                                                   //Frank 20210612 : Flipper Function
const int MOutFlipper3  =115;                                                   //Frank 20210612 : Flipper Function
/*
const int M1_1X         =116;
const int M1_1Y         =117;
const int M1_1R         =118;
const int M1_2X         =119;
const int M1_2Y         =120;
const int M1_2R         =121;
const int M1_3X         =122;
const int M1_3Y         =123;
*/

const int MLdCarRotArm  =116;                                                   //RogerYang 20250828 add for Loader Rotate Arm
const int MLoaderY_CCW  =117;
const int MAuto1Y_CCW   =118;
const int MAuto2Y_CCW   =119;
const int MAuto3Y_CCW   =120;
const int MAuto4Y_CCW   =121;
const int MAuto5Y_CCW   =122;
const int MAuto6Y_CCW   =123;

const int M1_3R         =124;
const int M1_4X         =125;
const int M1_4Y         =126;
const int M1_4R         =127;

const int M1_5X         =128;
const int M1_5Y         =129;
const int M1_5R         =130;
const int M1_6X         =131;
const int M1_6Y         =132;
const int M1_6R         =133;
const int M1_7X         =134;
const int M1_7Y         =135;
const int M1_7R         =136;
const int M1_8X         =137;
const int M1_8Y         =138;
const int M1_8R         =139;

const int MMagazine     =140;                                                   //JerryYang 20220909 : add magazine
const int MCatchMgzTray =141;
const int MMagYTrayOut  =142;

const int MFix3Full     =143;                                                   //JimmyChiu 20220927 : Stepper Motor Control in Fix3
const int MAuto4Z       =144;                                                   //Steven 20230907 : For HT-9011UC
const int MAuto5Z       =145;
const int MAuto6Z       =146;
const int MAuto4Y       =147;
const int MAuto5Y       =148;
const int MAuto6Y       =149;

const int MTopAOIArmX   =150;
const int MTopAOIArmY   =151;
const int MTopAOIArmR   =152;
const int MTopAOICCDZ   =153;
const int MTopAOIElevZ1 =154;
const int MTopAOIElevZ2 =155;

const int MOutSortPitchX=156;                                                   //Steven 20240822 : For HT-9046AU
const int MOutSortSht   =157;
const int MLoad2Z       =158;
const int MLoad2Y       =159;

const int MInSh1LtcSenZ1=160;                                                   //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
const int MInSh1LtcSenZ2=161;
const int MInSh2LtcSenZ1=162;
const int MInSh2LtcSenZ2=163;
//#define TOTAL_MOTOR 164     <--記得去改 (這是實體馬達陣列)

const int MManualTray1  =164;
const int MManualTray2  =165;
const int MManualTray3  =166;
const int MMTrayY       =167;

const int MMTrayY_Car   =168;
const int MMPlate1      =169;
const int MMPlate2      =170;
const int MMAuto1       =171;
const int MMAuto2       =172;
const int MMAuto3       =173;
const int MMAuto1_Car   =174;
const int MMAuto2_Car   =175;
const int MMAuto3_Car   =176;
const int MMEmpty       =177;

const int MMColor       =178;
const int MMEmpty_Car   =179;
const int MMColor_Car   =180;
const int MMEmpty1      =181;
const int MMEmpty1_Car  =182;
const int MMHot1RecBuf  =183;                                                   //jou 2011-12-26 加入記憶尚未完成吸取的位置
const int MMHot2RecBuf  =184;                                                   //jou 2011-12-26 加入記憶尚未完成吸取的位置
const int MMAutoCleanKit=185;                                                   //jou 2012-05-21 Auto Clean
const int MMOCR         =186;                                                   //Steven 20120626 : OCR
const int MMBulkboxKit  =187;                                                   //kevin 20160822

const int MMCABuffer1   =188;                                                   //wei 20180702 MR
const int MMCABuffer2   =189;                                                   //wei 20180702 MR
const int MMCABuffer3   =190;                                                   //wei 20180702 MR
const int MMCABuffer4   =191;                                                   //wei 20180702 MR
const int MMCABuffer5   =192;                                                   //wei 20180702 MR
const int MMCABuffer6   =193;                                                   //wei 20180702 MR
const int MMCABuffer7   =194;                                                   //wei 20180702 MR
const int MMCABuffer8   =195;                                                   //wei 20180702 MR
const int MMCABuffer9   =196;                                                   //wei 20180702 MR
const int MMCABuffer10  =197;                                                   //wei 20180702 MR

const int MMLoadPort    =198;                                                   //wei 20180702 MR
const int MMTrayLoader  =199;                                                   //wei 20180702 MR
const int MMTrayEmpty   =200;                                                   //wei 20180702 MR
const int MMTrayConversion=201;                                                 //wei 20180702 MR
const int MMTrayAuto1   =202;                                                   //wei 20180702 MR
const int MMTrayAuto2   =203;                                                   //wei 20180702 MR
const int MMTrayAuto3   =204;                                                   //wei 20180702 MR
const int MMFixTray1    =205;                                                   //Steven 20100205 : 暫存Fix資料用
const int MMFixTray2    =206;                                                   //Steven 20100205 : 暫存Fix資料用
const int MMFixTray3    =207;                                                   //Steven 20100205 : 暫存Fix資料用
const int MMScanAOI     =208;                                                   //Ifor 20211026 add:ScanAOI Tray

const int MMInArmAOATray   =209;                                                //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
const int MMOutArmAOATray  =210;
const int MMAOASampleTray  =211;
const int MMAOASamplePlate =212;

int USE_InPlacement=0;
const int MInPlacementX  =213;                                                  //JimmyChiu 20220908 add Pickup Error Placement
const int MInPlacementY  =214;

const int MMMagazineTary1   =215;                                               //JerryYang 20220909 : add magazine
const int MMMagazineTary2   =216;
const int MMMagazineTary3   =217;
const int MMMagazineTary4   =218;
const int MMMagazineTary5   =219;
const int MMMagazineTary6   =220;
const int MMMagazineTary7   =221;
const int MMMagazineTary8   =222;
const int MMMagazineTary9   =223;
const int MMMagazineTary10  =224;
const int MMMagazineTary11  =225;
const int MMMagazineTary12  =226;
const int MMMagazineTary13  =227;
const int MMMagazineTary14  =228;
const int MMMagazineTaryTop =229;

const int MMMagazineBuffer  =230;

const int MMBackupMagazineTary1     =231;
const int MMBackupMagazineTary2     =232;
const int MMBackupMagazineTary3     =233;
const int MMBackupMagazineTary4     =234;
const int MMBackupMagazineTary5     =235;
const int MMBackupMagazineTary6     =236;
const int MMBackupMagazineTary7     =237;
const int MMBackupMagazineTary8     =238;
const int MMBackupMagazineTary9     =239;
const int MMBackupMagazineTary10    =240;
const int MMBackupMagazineTary11    =241;
const int MMBackupMagazineTary12    =242;
const int MMBackupMagazineTary13    =243;
const int MMBackupMagazineTary14    =244;

const int MMTrayZ       =245;
const int MMEmptyZ      =246;
const int MMColorZ      =247;
const int MMAuto1Z      =248;
const int MMAuto2Z      =249;
const int MMAuto3Z      =250;                                                   //ChungHung 20140317 add Auto Retest
const int MMAuto4Z      =251;                                                   //Steven 20230907 : For HT-9011UC
const int MMAuto5Z      =252;
const int MMAuto6Z      =253;
const int MManualTray4  =254;
const int MManualTray5  =255;
const int MManualTray6  =256;
const int MMAuto4       =257;
const int MMAuto5       =258;
const int MMAuto6       =259;
const int MMAuto4_Car   =260;
const int MMAuto5_Car   =261;
const int MMAuto6_Car   =262;
const int MMFixTray4    =263;                                                   //Steven 20100205 : 暫存Fix資料用
const int MMFixTray5    =264;                                                   //Steven 20100205 : 暫存Fix資料用
const int MMFixTray6    =265;                                                   //Steven 20100205 : 暫存Fix資料用

const int MMLoadPort1   =266;                                                   //Sam 20190112 LM
const int MMLoadPort2   =267;
const int MMLoadPort3   =268;
const int MMLoadPort4   =269;
const int MMUnloadPort1 =270;
const int MMUnloadPort2 =271;
const int MMUnloadPort3 =272;
const int MMUnloadPort4 =273;
const int MMDailyCorrelationKit=274; //KaiHuang 20200606 : For ASE-CL Daily Correlation

//Mot定義為MAX_TRAY_MOTOR 300 <--超過的話記得去改

//Steven 20230907 : For HT-9011UC
//==>
int iTrayZMotor[MAX_TRACK]={MLoaderZ,       MEmptyZ,        MColorZ,        MAuto1Z,        MAuto2Z,        MAuto3Z,        MAuto4Z,        MAuto5Z,        MAuto6Z};
int iInSwitch  [MAX_TRACK]={SwACTrayY,      SwACEmptyCW,    SwACColorCW,    SwACAutoCW,     SwACAuto2CW,    SwACAuto3CW,    SwACAuto4CW,    SwACAuto5CW,    SwACAuto6CW};
int iOutSwitch [MAX_TRACK]={SwACLoaderCCW,  SwACEmptyCCW,   SwACColorCCW,   SwACAuto1,      SwACAuto2,      SwACAuto3,      SwACAuto4,      SwACAuto5,      SwACAuto6};

int iSwAutoIn  [MAX_AUTO_TRAY]={SwACAutoCW,     SwACAuto2CW,    SwACAuto3CW,    SwACAuto4CW,    SwACAuto5CW,    SwACAuto6CW};
int iSwAutoOut [MAX_AUTO_TRAY]={SwACAuto1,      SwACAuto2,      SwACAuto3,      SwACAuto4,      SwACAuto5,      SwACAuto6};
int iAutoZMot  [MAX_AUTO_TRAY]={MAuto1Z,        MAuto2Z,        MAuto3Z,        MAuto4Z,        MAuto5Z,        MAuto6Z};
int iMMAuto_Car[MAX_AUTO_TRAY]={MMAuto1_Car,    MMAuto2_Car,    MMAuto3_Car,    MMAuto4_Car,    MMAuto5_Car,    MMAuto6_Car};

int iMMAuto    [eTrayCount   ]={MMAuto1,         MMAuto2,         MMAuto3,          MMAuto4,          MMAuto5,          MMAuto6,
                                MManualTray1,    MManualTray2,    MManualTray3,     MManualTray4,     MManualTray5,     MManualTray6,
                                MManualTray1,    MManualTray2,    MManualTray3,     MManualTray4,     MManualTray5,     MManualTray6,     MMBulkboxKit,
                                MMMagazineTary1, MMMagazineTary2, MMMagazineTary3,  MMMagazineTary4,  MMMagazineTary5,  MMMagazineTary6,  MMMagazineTary7,
                                MMMagazineTary8, MMMagazineTary9, MMMagazineTary10, MMMagazineTary11, MMMagazineTary12, MMMagazineTary13, MMMagazineTary14};

int iAutoIndex [eTrayCount   ]={0,               1,               2,                 3,                4,                5,
                                0,               1,               2,                 3,                4,                5,
                                0,               1,               2,                 3,                4,                5,                0,
                                0,               1,               2,                 3,                4,                5,                6,
                                7,               8,               9,                10,               11,               12,               13};

int iMFixTray  [MAX_FIX_TRAY] ={MManualTray1, MManualTray2, MManualTray3, MManualTray4, MManualTray5, MManualTray6};
int iTempFix   [MAX_FIX_TRAY] ={MMFixTray1,   MMFixTray2,   MMFixTray3,   MMFixTray4,   MMFixTray5,   MMFixTray6};
int iMMgzTray  [MAX_MGZ_TRAY] ={MMMagazineTary1, MMMagazineTary2, MMMagazineTary3,  MMMagazineTary4,  MMMagazineTary5,  MMMagazineTary6,  MMMagazineTary7,
                                MMMagazineTary8, MMMagazineTary9, MMMagazineTary10, MMMagazineTary11, MMMagazineTary12, MMMagazineTary13, MMMagazineTary14};

int MgzTrayBabk[MAX_MGZ_TRAY] ={MMBackupMagazineTary1, MMBackupMagazineTary2, MMBackupMagazineTary3,  MMBackupMagazineTary4,  MMBackupMagazineTary5,  MMBackupMagazineTary6,  MMBackupMagazineTary7,
                                MMBackupMagazineTary8, MMBackupMagazineTary9, MMBackupMagazineTary10, MMBackupMagazineTary11, MMBackupMagazineTary12, MMBackupMagazineTary13, MMBackupMagazineTary14};
//<==
//Steven 20230907 : For HT-9011UC

//以下超過350的不能用到 MOT[]裡面,會溢位    //Steven 20210621 : Modify from 200 to 210
const int MMSystem      =500;
const int MMInShuttle   =501;
const int MMOutShuttle  =502;
const int MMIndex       =503;
const int MMTemperature =504;
const int MMInterface   =505;
const int MMScanner     =506;
const int MMCCD         =507;
const int MMAutoClean   =508;                                                   //kevin 20120518 autoclean

const int MMSafeDoor1   =510;
const int MMSafeDoor2   =511;
const int MMSafeDoor3   =512;
const int MMSafeDoor4   =513;
const int MMSafeDoor5   =514;
const int MMSafeDoor6   =515;
const int MMSafeDoor7   =516;
const int MMSafeDoor8   =517;
const int MMSafeDoor9   =518;
const int MMSafeDoor10  =519;
const int MMSafeDoor11  =520;
const int MMSafeDoor12  =521;
const int MMSafeDoor13  =522;
const int MMSafeDoor14  =523;
const int MMSafeDoor15  =524;
const int MMSafeDoor16  =525;                                                   //JerryYang 20230704 : 整合安全門15->MAX_SAFE_DOOR_CNT
const int MMSafeDoor17  =526;
const int MMSafeDoor18  =527;
const int MMSafeDoor19  =528;
const int MMSafeDoor20  =529;
const int MMSafeDoor21  =530;
const int MMSafeDoor22  =531;

//Steven 20130205 : 離子風扇異常顯示
const int MMIonFan01    =540;
const int MMIonFan02    =541;
const int MMIonFan03    =542;
const int MMIonFan04    =543;
const int MMIonFan05    =544;
const int MMIonFan06    =545;
const int MMIonFan07    =546;
const int MMIonFan08    =547;
const int MMIonFan09    =548;
const int MMIonFan10    =549;
const int MMIonFan11    =550;
const int MMIonFan12    =551;
//Ifor 20160830 add ATC 異常位置顯示
const int MMATC_Handler     =560;
const int MMATC_TCPIP       =561;
const int MMATC_NI          =562;
const int MMATC_ATC         =563;
const int MMATC_Chiller     =564;
const int MMATC_RS232       =565;
const int MMATC_Head        =566;
const int MMATC_PowerSupply =567;
const int MMATC_WaterValve  =568;
const int MManualTrayAll    =569;

//KaiChen 20200716 ：OHT
//==>
const int MMMultileEmpty        =571;
const int MMMultileEmpty_Catch  =572;
const int MMMultileEmpty_Z      =573;
//<==
//KaiChen 20200716 ：OHT
//以上超過350的不能用到 MOT[]裡面,會溢位
//====================================================================
//Motor define end
//====================================================================

bool  fHeaterOK=false;
bool  fHeaterOK_Backup=false;                                                   //jou 2010-10-11
bool  fHeaterStableOK=false;                                                    //JerryYang 20210122 : ASE-CL新增待溫功能
bool  bHeatOKBellowError=false;                                                 //kevin 20130226 加熱ok後 溫度過低偵測    //jou 2014-06-12 修正偶發性秀低溫異常
int   iHeaterWait=0;
int   iHeaterCooling=0;
double UN150Read[tcTotalCount]={0};
double UN150ReadSecondpoint[tcTotalCount]={0};                                  //Ifor 20160629 add 第二點溫度資料
double UN150ReadReal[tcTotalCount]={0};
double UN150ReadHotplate[2]={0,0};
double UN150ReadShuttle[2]={0,0};                                               //kevin 20150508 shuttle 分段加熱
bool  UN150CommError[tcTotalCount]={false};
bool  FlushFlag;
int   RunState=LED_Pause;                                                       //Steven 20111030 : 初始值改成2
bool  PauseUT150Polling=false;

bool bLampPowerOff      =false;
bool bLampPowerOn       =false;
bool bLampReset         =false;
bool bLampPause         =false;
bool bLampHome          =false;
bool bLampTrain         =false;                                                 //ChungHung 20120905 add
bool bLampStart         =false;
bool bLampOneCycle      =false;
bool bLampRetry         =false;
bool bLampSkip          =false;
bool bLampCleanOut      =false;
bool bLampTrayFeed      =false;
bool bLampTrayEnd       =false;
bool bLampAlarmReset    =false;
bool bLampCoverOpenFR   =false;
bool bLampCoverOpenFL   =false;
bool bLampCoverOpenRR   =false;
bool bLampCoverOpenRL   =false;
bool bLampManualSetp    =false;
bool bLampManualStart   =false;
bool bLampFix           =false;                                                 //kevin 20130322 ONECYCLE 合併

bool bMoveInArm1ZToPlate1Safe   =false;
bool bMoveInArm1ZToPlate2Safe   =false;
bool bMoveInArm2ZToPlate1Safe   =false;
bool bMoveOutArm1ToFixSafe      =false;
bool bFrontPadActive            =false;
bool bSafeLockStatus            =false;                                         //Ken 20210702 AddPadInterface

int iResetTask=1;
int iTrayFeedTask=1;
int  MotorPowerOnDelay=10;
bool bMotorPowerState=false;                                                    // true:motor power is on
bool bAlarmBuzzer=false;
int iPauseBackUp=-1;                                                            //ChungHung 20110901 add
double Torque[5]={0.0, 0.0, 0.0, 0.0, 0.0};
int IndexStatus=0;
double IndexTime;
TQPF_Timer tIndexTimer;
TQPF_Timer tSendChangeTempDelay;
TQPF_Timer tRecordOEE;

bool bNeedArmZHome =false;
bool bIsInArmHome  =false;
bool bIsOutArmHome =false;
bool bIsSortArmHome =false;                                                     //RogerYang 20250512 Add for 9046AU

int Z1Safe              =0;
int XShiftF             =0;
int Z1Up                =0;
int Z2DownSafe          =0;
int Z2Down              =0;
int Z2Down2Speed        =0;
int XShiftF_nn          =0;

int Z2Safe              =0;
int XShiftR             =0;
int Z1DownSafe          =0;
int Z2Up                =0;
int Z1Down              =0;
int Z1Down2Speed        =0;
int XShiftR_nn          =0;
int AxisY_Pre_MovePos   =0;

bool bInArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col]={{false,false,false,false},{false,false,false,false}};
bool bOutArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col]={{false,false,false,false},{false,false,false,false}};
bool bSortArmZNeedHome[MAX_ARM_Row][MAX_ARM_Col]={{false,false,false,false},{false,false,false,false}};   //RogerYang 20250512 Add for 9046AU

bool bInArmPitchNeedHomeCheck=true;                                             //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
bool bOutArmPitchNeedHomeCheck=true;                                            //ChungHung HT9045 2011/12/13 //Input pickup device error時,按"retry"鍵,機台都會自動home
bool bSortArmPitchNeedHomeCheck=true;                                           //RogerYang 20250512 Add for 9046AU
bool bPrecisorNeedHomeCheck=false;
bool bShuttle1MoveToRight;                                                      //ChungHung 20120717 add Index Drop Error Can Retry and Start
bool bShuttle1MoveToLeft;                                                       //ChungHung 20120717 add Index Drop Error Can Retry and Start
bool bShuttle2MoveToRight;                                                      //ChungHung 20120717 add Index Drop Error Can Retry and Start
bool bShuttle2MoveToLeft;                                                       //ChungHung 20120717 add Index Drop Error Can Retry and Start
bool bShuttle1HasPickErr;                                                       //Steven 20220712 : 避免In arm 偷放料
bool bShuttle2HasPickErr;

bool bShuttle3MoveToRight;                                                      //RogerYang 20250509 Add for 9046AU
bool bShuttle3MoveToLeft;
bool bShuttle3HasPickErr;

bool bAutoCleanShuttle1MoveToLeft=false;                                        //Richard 20230418 : add Index Pick Error Can Retry and Start
bool bAutoCleanShuttle2MoveToLeft=false;                                        //Richard 20230418 : add Index Pick Error Can Retry and Start
bool bAutoCleanShuttle1HasPickErr=false;                                        //Richard 20230418 : add Index Pick Error Can Retry and Start
bool bAutoCleanShuttle2HasPickErr=false;                                        //Richard 20230418 : add Index Pick Error Can Retry and Start

bool bCheckNullIC1=false;                                                       //JerryYang 20170623 (Steven) bCheckNullIC1改成全域變數
bool bCheckNullIC2=false;

bool bShuttleMoveToLeftforFix3;                                                 //ChungHung 20140313 add Fix3 can Full Tray

//----- by dell ccd realtime-------------
bool bSendRealCCDSendStart=true;
bool bSendRealCCDSendVerify=true;
bool bSendRealCCDSendVerifyOK=false;
bool bRealCCDSendArm=false;
int  iRealCCDSendArmCT=0;

int iInArmPitch=3900;
int iOutArmPitch=3900;

bool bCanNotDisableOneCycle=false;

bool bPauseTester=false;

int  iReceiveEmptyTray=0;
int  iReceiveColorTray=0;
int  iReceiveAuto2Tray=0;                                                       //kevin 20120718 Auto2 KYEC MFC
int  iReceiveAutoTray[3]={0, 0, 0};                                             //Steven 20140331 : Auto Retest
int  iReceiveLoaderTray=0;                                                      //Steven 20140331 : Auto Retest
bool fEmptyCanSupplyNewTray=true;
bool fColorCanSupplyNewTray=true;
bool fAuto2CanSupplyNewTray=true;                                               //kevin 20120718 Auto2
bool fAutoCanSupplyNewTray[3]={true};                                           //Steven 20140331 : Auto Retest
bool fLoaderCanSupplyNewTray=true;                                              //Steven 20140331 : Auto Retest

int TTL_StartData[8]={SwStart0, SwStart1, SwStart2, SwStart3, SwStart4, SwStart5, SwStart6, SwStart7};
int TTL_Dut[8]={SwDut0, SwDut1, SwDut2, SwDut3, SwDut4, SwDut5, SwDut6, SwDut7};
int TTL_Sensor[8][5]={{SenBit0,  SenBit1,  SenBit2,  SenBit3,  SenBit4},
                      {SenBit5,  SenBit6,  SenBit7,  SenBit8,  SenBit9},
                      {SenBit10, SenBit11, SenBit12, SenBit13, SenBit14},
                      {SenBit15, SenBit16, SenBit17, SenBit18, SenBit19},       //Alick 20161011 (Steven) : TTL支援8Site
                      {SenBit20, SenBit21, SenBit22, SenBit23, SenBit24},
                      {SenBit25, SenBit26, SenBit27, SenBit28, SenBit29},
                      {SenBit30, SenBit31, SenBit32, SenBit33, SenBit34},
                      {SenBit35, SenBit36, SenBit37, SenBit38, SenBit39}};

int iGali_VsSpeed=0;
int iGali_SpSpeed=0;
bool _bHomeNeedOnecycle=false;

bool bCheckShuttle1Flag=false;
bool bCheckShuttle2Flag=false;
bool bCheckShuttle3Flag=false;                                                  //RogerYang 20250509 Add for 9046AU
bool bScanLightPause=false;
bool bIsTestSitICFallDown=false;
bool bIsTestSitICFallDownResetHT9045=false;
bool TestISTimeOut=false;
bool IsTest=false;
int  iCurrentTime=0;

bool bChangeToInitStartMode=false;                                              //ChungHung 20111230 Hot Wait Mode

bool bNeedTrySuckHotPlate=false;                                                //ChungHung 20120206 Hotplate check : 判斷是否要檢查HotPlate
bool bTrySuckHotPlateCleanOut=false;                                            //ChungHung 20120206 Hotplate check : 檢查完成後,作CleanOut
bool bTrySuckHotPlateOneCycle=false;                                            //Steven 20120323 : 檢查完成後,作OneCycle
bool bOneTimeHotPlateCheckAll=false;                                            //ChungHung 20120206 Hotplate check : 一次檢查全部的位置
bool bTryPickFromHotPlateShowError=false;                                       //ChungHung 20120206 Hotplate check : 有IC時,要顯示Alarm
bool bHotPlateCheckNeedTrayFeed=false;                                          //Steven 20120322 : 檢查完要自動TrayFeed

const int LED_Running=0;
const int LED_ErrJam=1;
const int LED_Pause=2;
const int LED_Message=3;
const int LED_Heating=4;
const int LED_Homeing=5;
const int LED_OfflineRun=6;
const int LED_AutoRetest=7;

const int Axis_Z1=0;
const int Axis_Z2=2;
const int Axis_Shuttle1=4;
const int Axis_Shuttle2=6;
const int SuckActiving=0;
const int DestroyActiving=1;
bool bEnterOffset=false;
bool bClearSrtart[8]={false,false,false,false,false,false,false,false};         //Alick 20161011 (Steven) : TTL支援8Site
bool bHomeByStart=false;
int  HistroyBin[4][8][22];                                                      //Steven 20100121   //Steven 20131004 : 2 --> 4 //Ifor 20221026 add:KYEC 要求增加比數10 => 22
bool HistroyPassFail[4][8][22];                                                 //Steven 20100121   //Steven 20131004 : 2 --> 4 //Ifor 20221026 add:KYEC 要求增加比數10 => 22
int  iHeaterWaitTime=0;
bool bInArmSuckErr=false;
bool bMustCleanAllTray=false;

bool bLoaderPause=false;
bool bEmptyPause=false;
bool bColorPause=false;
bool bUnLoaderPause=false;
bool bFinshTest=true;
bool bInitStartDelayNotFinish=true;                                             //Ifor 20181220 : add Init Start Delay Time Not Finish
bool bShuttle1Pause=false;
bool bShuttle2Pause=false;
bool bIndexArm1PickupErrStop=false;                                             //jou 2012-02-29 index pick up error,index arm move to center & alarm
bool bIndexArm2PickupErrStop=false;                                             //jou 2012-02-29 index pick up error,index arm move to center & alarm
bool bShowShuttle1Device=false;                                                 //kevin 20180504 index pick up error
bool bShowShuttle2Device=false;                                                 //kevin 20180504 index pick up error

bool bInArmNeedToSafePos=false;                                                 //ChungHung 20110302 start

bool bNeedSlapTray=false;                                                       //ChungHung 20140701 add Auto Retest

bool bQualSiteQuickMode=false;
int iSetShuttleToHasNullIC=0;

bool bRearHeadICFallDown=false;
const int Tempture_Ambient=0;
const int Tempture_Hot=1;
const int Tempture_AmbientHot=3;                                                //kevin 20160908 2->3 //kevin 20140918 常溫加熱 恆溫
const int USE_FRONT_TEST_HEAD=0;
const int USE_REAR_TEST_HEAD=1;
const int USE_TWOARM_TEST_HEAD=2;
const int CLEAR_FRONT_TEST_HEAD=1;
const int CLEAR_REAR_TEST_HEAD=2;
int PitchCylinderState[MAX_AUTO_TRAY]={0, 0, 0, 0, 0, 0};
bool bReadFrontTestArmTorque=false;
bool bReadRearTestArmTorque=false ;

bool bInitialLaserCheck=false;                                                  //Steven 20140228 : 雷射測距功能
bool bInitialLaserCheckPlate=false;                                             //Steven 20140228 : 雷射測距功能
int  iLaserShuttle=0;                                                           //Steven 20140228 : 雷射測距功能
bool bInitialICCheck=false;
int iInitialICCheckTask=1;
int iStepShuttleTask[2]={1, 1};
int iClearSocketFunction=0;
//Steven 20090907 Start
bool bCheckShuttle1MustHasIC=true;
bool bCheckShuttle2MustHasIC=true;
int iCheckShuttle1MustHasICTask=1;
int iCheckShuttle2MustHasICTask=1;
int iCheckShuttle1SlotTask=1;
int iCheckShuttle2SlotTask=1;
int iCheckShuttle1ProminentTasK=1;                                              //Alick 20160815 add for 力成
int iCheckShuttle2ProminentTasK=1;                                              //Alick 20160815 add for 力成
//Steven 20090907 End
//kevin 20161108 Start
bool bCheckShuttle1MustNoHasIC=true;
bool bCheckShuttle2MustNoHasIC=true;
int iCheckShuttle1MustNoHasICTask=1;
int iCheckShuttle2MustNoHasICTask=1;
int iCheckShuttle1SlotNoHasICTask=1;
int iCheckShuttle2SlotNoHasICTask=1;
int iCheckShuttle1ProminentNoHasICTasK=1;                                       //Alick 20160815 add for 力成
int iCheckShuttle2ProminentNoHasICTasK=1;                                       //Alick 20160815 add for 力成
//kevin 20161108 end
int iAuto1ZeroPos;

bool bLongTimePause=true;
int  iLongTimePauseCount=0;
bool bIsAutoOneCycle=false;
bool bBackupOneCycle_ByAutoClean=false;                                         //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 未完成時又觸發 AutoClean 動作，AutoClean 做完並不會提示 OneCycle Finish
bool bManualOneCycle=false;                                                     //Sam 20230309 : 避免觸發 OneCycle 後，OneCycle 未完成時又觸發 AutoClean 動作，AutoClean 做完並不會提示 OneCycle Finish
bool bIsAvoidIndexShiftOneCycle=false;                                          //Isaac 20210821 : 全智要求定時onecycle回home,start
bool bIsASMAutoOneCycle=false;
bool bIndexArmNoTestting=false;                                                 //ChungHung 20140730 add ContinuousFailHaveOneCycle

bool bLotStartEndNeedIndexCheck=false;                                          //Isaac 20211019 : 可選擇做index check的時機
bool bIndexJamNeedIndexcheck=false;                                             //Isaac 20211019 : 可選擇做index check的時機

int  AUTO_EMPTY_COLOR=0;                                                        //軌道 0:手動,  1:6軌自動,  2:7軌自動,  3:8軌自動  //Steven 20230907 : For HT-9011UC, bool --> int
int  iAutoCnt       =3;                                                         //Steven 20230907 : 給out arm計算要放的是auto還是fix
int  iFixCnt        =3;                                                         //Steven 20230907 : 給out arm計算要放的Fix總數量
int  iAutoRight     =2;                                                         //Steven 20230907 : 給out arm計算機台到Auto最右側的位置
int  iFixMin        =3;                                                         //Steven 20230907 : 給out arm計算要放的fix盤起始位置
int  iFixMax        =5;                                                         //Steven 20230907 : 給out arm計算要放的fix盤結束位置
int  iFixPosMin     =4;                                                         //Steven 20230907 : 給out arm計算要放的fix盤起始位置
int  iFixPosMax     =6;                                                         //Steven 20230907 : 給out arm計算要放的fix盤結束位置
int  iFixPosHalf    =9;                                                         //Steven 20230907 : 給out arm計算機台到Fix最右側的位置(分半盤)
int  iFixRight      =5;                                                         //Steven 20230907 : 給out arm計算機台到Fix最右側的位置
int  iFixRightHalf  =8;                                                         //Steven 20230907 : 給out arm計算機台到Fix最右側的位置(分半盤)
int  iMMAutoRight   =2;                                                         //Steven 20230919 : Auto最右側的陣列位置
int  iMMFixRight    =2;                                                         //Steven 20230919 : Fix最右側的陣列位置
int  iMMAoi         =1;                                                         //Steven 20230919 : AOI的陣列位置
int  iBinBoxAtFix   =2;                                                         //Steven 20230907 : Bin Box在Fix的位置
int  iMMBinBox      =2;                                                         //Steven 20230907 : Bin Box在陣列的位置
int  iBinBoxAtAuto  =9;                                                         //Steven 20230907 : Bin Box在iWhichAuto的位置
int  iBinBoxType    =3;                                                         //Steven 20230907 : Bin Box在Tray Form的type
int  iMagMin        =10;                                                        //Steven 20230907 : 給out arm計算要放的Magazine位置
int  iMagMax        =23;                                                        //Steven 20230907 : 給out arm計算要放的Magazine位置
int  iMagAtAuto     =2;                                                         //Steven 20230907 : 給out arm計算要放的Magazine對應Auto位置

bool SUPPORT_2_EMPTY_EMPTY=false;
int iOpenNewTTLBoardStratDelayCount=0;
bool bChangeTTLFlag=true;

int EP_Install=0;                                                               //20111111  Dell
double EP_MAXKPA=0.0;
double EP_MAXAFB=0.0;
double EP_MinAFB=0.0;
double EP_MINA=0.0;
double EP_MINMPA=0.0;                                                           //JerryYang 20171023 (wei) add PA Min

double EP_MAXKPA_1032=0.0;
double EP_MAXAFB_1032=0.0;
double EP_MinAFB_1032=0.0;
double EP_MINA_1032=0.0;
double EP_MINMPA_1032=0.0;                                                      //JerryYang 20171023 (wei) add PA Min

double EPDual_MAXKPA=0.0;
double EPDual_MAXAFB=0.0;
double EPDual_MinAFB=0.0;
double EPDual_MINA=0.0;
double EPDual_MINMPA=0.0;                                                       //JerryYang 20171023 (wei) add PA Min
AnsiString  JamCode;                                                            //ivy
bool        LOAD_Z_USE_MOTOR[9];                                                //Steven 20190813 : 入Tray改用步進馬達
bool        LOAD_Y_USE_MOTOR[9];                            //Jimmychiu 20240307 : Loader Tray改用步進馬達
bool        LOADUNLOAD_USE_CASSETTE[9];
int         NUMBER_PANEL_TYPE               =0;
int         WEIGHT_CALIBRATION              =0;                                 //Steven 20111108
int         ION_FAN_TYPE                    =1;                                 //Steven 20100226
int         SHUTTLE_SENSOR_TYPE             =eSensorLine;                       //Steven 20100319
int         NUEC_TYPE                       =eNUEC1;                            //Sam 20230707 : EtherCAT Shuttle sensor
bool        ENABLE_OUT_SHUTTLE_SENEOR       =true;                              //Steven 20120531 : by pass out shuttle sensor
bool        ENABLE_OUT_SHUTTLEY_LATCH       =true;                              //Steven 20170518 (jou) : Out shuttle Y sensor 使用 LATCH
bool        Use_AxisY_Sensor_2x3mode        =false;                             //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
bool        Bias_Mode_Use_Y_Sensor          =false;                             //Steven 20240805 : 1x2 Bias mode 用Y sensor 偵測Outshuttle殘料
int         TEMPCTRL_TYPE                   =0;                                 //Steven20100513
int         SAFE_DOOR_AMOUNT                =2;                                 //Steven 20100610
int         TRAY_VIBRATION                  =0;                                 //Steven 20110627 : new Tray Vibration  //JerryYang 20170531 (wei) 敲tray方式新增震動馬達
bool        REAL_TIME_CCD                   =false;                             //Steven 20110705 : 安裝Real Time CCD
int         RTC_TemperNumber                 =1;                                //Isaac 20201217 : RTC CCD增加第二組感溫
bool        CCD2_TEMPER                     =false;
bool        LB_TEMP                         =false;                             //Steven 20181023 : LB溫度
bool        LB_TEMP_UpDown                  =false;                             //Frank 20241231 : add
bool        Index_ESDAir                    =false;                             //kevin  20200207 : ESD air Temp

//bool        INDEX_CCD                       =false;                             //Ifor 20150720 : 安裝INDEX CCD  //Ifor 20150728 Mark整合於RTC CCD選項
int         USE_16_HEATER                   =eht4Heater;                        //Steven 20111011 : 使用16個Heater
int         EJ1N_Count                      =0;                                 //Steven 20200318 : 修正EJ1N讀取異常的問題
bool        USE_IO_CHANGE_TOQUE             =false;                             //jou 2012-06-21 Enable index I/O Change Toque
bool        USE_ReadIndex_TOQUE             =false;                             //kevin 20201027 add Read index Torque
//==================================================================
bool        bResetArm1Value                 =true;                              //KaiHuang 20201222 : Reset 標準值(第一次Contact And Change Speed)
bool        bResetArm2Value                 =true;                              //KaiHuang 20201222 : Reset 標準值(第一次Contact And Change Speed)

bool        bResetRecordSetArm1Value        =true;                              //kevin 20210118 : Reset 標準值下依次 cotract(第一次Contact And Change Speed)
bool        bResetRecordSetArm2Value        =true;                              //kevin 20210118 : Reset 標準值下依次 cotract(第一次Contact And Change Speed)

bool        bReadArm1_Torque                =false;                             //KaiHuang 20201222 : 讀取 Arm1 扭力Flag
bool        bReadArm2_Torque                =false;                             //KaiHuang 20201222 : 讀取 Arm2 扭力Flag
double      dSetArm1TorqueValue             =0.0;                               //KaiHuang 20201222 : Arm1 第一次 Contact 的值
double      dSetArm2TorqueValue             =0.0;                               //KaiHuang 20201222 : Arm2 第一次 Contact 的值
int         iReadTorqueError                =0;                                 //KaiHuang 20201222 : 判斷讀取扭力是否有異常(會導致扭力趨近穩態)
//=================================================================
bool        USE_NEW_TEMPCTRL_FUNCTION       =false;                             //Steven20100513
const int   iXPitch60                       =0;                                 //Steven 20131002 : XY變距
const int   iXPitchManual635                =1;                                 //Steven 20131002 : XY變距
const int   iXYPitchVariable                =2;                                 //Steven 20131002 : XY變距
const int   iXPitchManual360                =3;                                 //Steven 20140819 : Y-Pitch 36mm
const int   iXYPitchRowA                    =4;                                 //Steven 20170424 (wei) : new XY變距
const int   iXYPitch16Picker                =5;                                 //Steven for HT1032
const int   iXYPitchBb                      =6;                                 //Steven for HT7080
const int   iXYPitch16Bd_Be                 =7;                                 //Ztex 2023.12.06 Add HT-1032
const int   iXYPitchIn_Bb_Out_Bc            =8;                                 //Ztex 2024.02.24 Add HT-1132
const int   iXPitch40mm                     =0;                                 //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
const int   iXPitch50mm                     =1;                                 //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
const int   iXPitchAuto                     =2;                                 //JimmyChiu 20220708 : add Auto X pitch
const int   iXPitch16Pick                   =3;                                 //Steven for HT1032
int         USE_IN_OUT_ARM_Y_PITCH          =iXPitch60;                         //jou 2012-05-15 : 選擇 Y Pitch 機構模式
int         USE_OUT_ARM_Y_PITCH             =iXPitch60;                         //JerryYang 20251218 : IN/OUT ARM支援不同模組
bool        USE_IN_Y_IS_AUTO_PITCH          =false;
bool        USE_OUT_Y_IS_AUTO_PITCH         =false;
int         IN_OUT_ARM_Y_PITCH_MIN          =1500;
int         IN_OUT_ARM_Y_PITCH_MAX          =7500;
int         IN_OUT_ARM_X_PITCH_MIN          =6000;                              //JimmyChiu 20220708 : add Auto X pitch
int         IN_OUT_ARM_X_PITCH_MAX          =12000;                             //JimmyChiu 20220708 : add Auto X pitch
int         USE_IN_OUT_ARM_X_PITCH          =iXPitch40mm;                       //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式

const int   iOutSortX40mm                   =1;
int         USE_OUT_SORT_ARM                =eartUninstall;                     //Steven 20240822 : For HT-9046AU
int         USE_LD_Rot_Arm                  =0;                                 //RogerYang 20250828 add for Loader Rotate Arm
int         USE_E84_Sensor                  =0;                                 //Frank 20251204 add E84
int         USE_LdUldCassetteMode           =0;                                 //Frank 20251217 add
int         USE_OUT_SORT_X_PITCH_MIN        =1333;                              //RogerYang 20250609 Add for 9046AU
int         USE_OUT_SORT_X_PITCH_MAX        =4000;

int         BASE_X_TO_HP                    =7000;                              //Steven 20230826 : 基準軸X到加熱盤邊緣的距離  HT-9xxx: 7000, HT-7080B: 0, HT1040: 0, HT1028: 4000
int         USE_ROTATE_KIT                  =0;                                 //Steven 20121001 : 旋轉Kit
int         iRotate_In_Index                =0;                                 // 2012.06.11 , Joye , Rotate KIT
int         iRotate_Out_Tray6               =1;                                 // 2012.06.11 , Joye , Rotate KIT
int         iRotate_Type                    =eCynRotate;                        //2013-04-12    Dell :旋轉站;馬達版
int         USE_ROTATE                      =0;                                 //Steven 20230930 : 整合氣缸版旋轉Kit的Flag
int         iSpecialSequence                =0;                                 //2013-04-12    Dell :for KYEC 特殊排列方式 serio:29829
int         USE_HOTPLATE_TYPE               =0;                                 //jou 2012-05-15 : 選擇 Hotplate Type
int         USE_MAGNETIC_SCALE              =0;                                 //Steven 20160426 : 磁性尺
int         USE_PICKER_COUNT                =1;                                 //Steven 20161117 : for HT-9045S
int         USE_16PICKER_TYPE               =0;                                 //Ztex 2023.12.06 Add HT-1032
int         INOUT_ARM_Y_PITCH               =0;                                 //Ztex 2023.12.06 Add HT-1032
int         SHUTTLE_Z_TYPE                  =0;                                 //jou 2013-07-26 : 選擇 Shuttle Z Sensor Type
int         USE_PRECISER                    =0;                                 //Frank 20180410 (Steven) : InArm Preciser Station
//bool        PreciserInstallShuttleArea      =false;                             //Frank 20180410 (Steven) : InArm Preciser Station
int         iPreciserInstallArea            =0;                                 //Ifor 20191008 : add Preciser Install Area
bool        USE_LOADER_VIBRATION            =false;                             //JerryYang 20191001 loader震動馬達

double      SHUTTLE_COOLING                 =0.0;
bool        OFFLINE_ALARM                   =false;                             //Steven 20140805
double      VORTEX_COOLING                  =0.5;                               //Steven 20110922 : 渦流管吹氣的啟動Offset
double      SOCKET_OFFSET                   =0.0;
int         INDEX_SUCKER_TYPE               =0;                                 //jou 2010-05-19 start : 負壓
eIndexPressType         INDEX_PRESS_TYPE    =e85KG;                             //Steven 20110310 : 240KG
bool        bUserDefMaxContactHeight        =false;                             //Steven 20180412 : For new HT-9045高度較高
double      dUserDefMaxContactHeight        =-135.0;
bool        bUser_Define_IndexZ_SafePos     =false;                             //Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
double      dUserDefineIndexZSafePos        =200;
bool        TEMPCTRL_NEED_UNDER_20A         =false;                             //Steven 20100722
bool        TEMPCTRL_HOTPLATE_TOGTHER       =false;                             //jou 2010-08-16 start : SCC Hotplate不分段加熱
bool        AMBIENT_TEMP_CHECK[tcTotalCount]={0};                               //Steven 20100831 : 常溫檢查的位置  //Steven 20111208 : 記憶體破壞, 20->tcTotalCount
//AI(W0-TAIL) 20260626: TODO(W6) -- G9004_M204 is a Syntek driver-header constant.   //AI(W906-AMB-L2) 20260929: HELD at 0 on purpose (tray review of f381f076 #2; that commit restored golden :3172 and this batch took it back out). The constant exists now (Motor/vendor/CMNet.h:20 = 0xA7), but golden's 0xA7 default switches InitShuttleThreadParameter (cinitial.cpp:12672 / :12734) into M204 latch mode on every machine whose runtime does not assign it -- e.g. the PCI1203 path (cinitial.cpp:3989-3996); only SMC (cinitial.cpp:4009/:4436), MN200 (myMN200motor.cpp:1183) and SYNTEK (mySYNTEKmotor.cpp:296/:320/:350) assign it -- and the port's TfLtcSensor is an offline stand-in (acarry_shims.h:77-90, every LatchDataCnt 0; SetLtcSensor empty, acarry_shims.cpp:43), so the out-shuttle residual checks (acarry.cpp:2070/:2095/:2949) would read an empty latch buffer instead of Sen[SnOutPutSHT1S1]. Lift together with a real TfLtcSensor.
#if 0
int         SYN_TEK_MOTION_MODULE           =G9004_M204;
#endif
int         SYN_TEK_MOTION_MODULE           =0;  //AI(W0-TAIL) 20260626: placeholder until G9004_M204 (driver) lands (W6)                        //Steven 20110304 : 判斷是哪一種模組
bool        SAFE_DOOR_LOCK                  =false;                             //20111130  Dell
bool        CLEAN_AIR                       =false;                             //ChungHung 20120821 : Clean Air
int         INSTALL_OCR                     =0;                                 //Steven 20120716 : OCR
int         INSTALL_OCR_YMot                =0;                                 //Frank 20250214 add
bool        bGetLotIDFormTester             =true;                              //ChungHung 20121123 add
int         BAR_CODE_INSTALL                =0;                                 //Steven 20120921 : Barcode_2D
int         BAR_CODE_USECOUNT               =0;                                 //Ifor 20181206 :add COGNEX SYSTEM CCD Count
int         BOTTOM_2DID                     =0;                                 //Steven 20190308 : Bottom 2D
int         BOTTOM_2DID_CCD                 =0;                                 //KaiChen 20200513 : Bottom 2D 8CCD
int         SHT_FLOATING_CHK                =0;                                 //Steven 20160920 : IC置偏檢查
int         IN_SHT_LAST_SENSOR              =0;                                 //Steven 20181203 : In Shuttle最後一個Sensor定義
//int         BAR_CODE_MODE                   =0;                                 //Steven 20120921 : Barcode_2D
int         FIX3_FULL_PLACE                 =0;                                 //Steven 20130126 : Fix3滿盤功能
int         FIX3_INSTALL                    =0;                                 //2013-07-16    Dell    9046LS第一台沒Fix3;第二台以上Fix3標配
int         CROSS_SENSOR_INSTALL            =0;                                 //2013-07-16    Dell    Shuttle cross sensor
int         ESD_Monitor                     =0;                                 //2013-07-16    Dell    使用ESD monitor
int         USE_NOVX3360                    =0;                                 //Steven 20131127 : 使用Simco ION風扇
int         USE_AutoCleanIonFan             =0;                                 //Isaac 20210609 : IO觸發IonFan清針
int         USE_PULSE_TYPE                  =0;                                 //Ifor 20180316 : add Simco Use Pulse Type
int         ION_PULSE_COUNT                 =0;                                 //Ifor 20180316 : add Simco Use Pulse Type
int         USE_KASUGA                      =0;                                 //Ifor   20150731 : 使用Kasuga ION風扇
int         USE_OTD                         =0;                                 //Steven 20140312 : OTD
int         USE_46_SUCKER_DB                =0;                                 //Steven 20131008 : HT9045使用46配氣
int         USE_46_SENSOR_DB                =0;                                 //Steven 20131008 : HT9045使用46配電
int         INDEX_MOTION_CARD               =0;                                 //Steven 20210623 : Index使用Galil
int         GALI_PROTECT_MODE               =0;                                 //Steven 20260504 : Index in-motion protect (0=Original, 1=ER+OE, 2=TE)
int         USE_FINE_PITCH                  =0;                                 //Steven 20210624 : Fine Pitch
int         USE_OUT_SHT_MOT                 =0;                                 //Steven 20210624 : Out shuttle獨立馬達
int         USE_AIR_CONDITIONER             =0;                                 //Steven 20131011 : 冷氣機
int         USE_BARCODE_AS_KEYBOARD         =0;                                 //Steven 20140108 : 只能用Bar Code Reader輸入
int         HOT_PLATE_POSITION              =0;                                 //Steven 20140222 : Hot Plate Pin的位置
int         HOT_PLATE_LIMITATION            =0;                                 //Steven 20151117 : Hot Plate Pin的極限位置, 可以跑2x2 8吸嘴
int         USE_LASER_DISTANCE              =0;                                 //Steven 20140228 : 雷射測距功能
int         USE_AUTO_RETEST                 =eartUninstall;                     //ChungHung 20140317 add Auto Retest
int         UNLOADER_ART[MAX_AUTO_TRAY]     ={eartUninstall, eartUninstall, eartUninstall, eartUninstall, eartUninstall, eartUninstall};   //Steven 20161221 : for SCK only Auto 2 has ART
int         USE_AOI_Inspection              =0;                                 //2014-03-04    Dell    for SPIL WLP Add 5s Inspection
int         USE_VibrationCommunication      =0;                                 //JerryYang 20230814 : add震動馬達通訊調速版本
int         USE_COLOR_TRAY_SENSOR           =0;                                 //20140903 wei colcr Tray
int         USE_SOCKET_SENSOR               =0;
int         CANBUS_METHOD                   =0;                                 //Sam 20210518 : 新增 CanBus 軟體配置
bool        USE_CKD_FCM_CleanAir            =false;                             //Ifor 20150708 : CKD FCM Clean Air
int         USE_CATCH_TRAY_MODEL            =0;                                 //wei 20150807 Catch Tray Model
//Steven 20260924 (S12-C)：golden V912 有、移植樹沒有；FileRW/IniConfig.gen.inc（golden cConfiguration.cpp）要用到，逐字補上（golden cmydef.cpp:3221-3222）
int         USE_TRAY_ARM_TYPE               =0;                                 //Ifor 20260824 : Tray Arm hardware type (eTrayArmHWType)
int         TRAY_ARM_STANDBY_OFFSET         =15000;                              //Ifor 20260824 : new Tray Arm standby offset from Empty toward Color (pulse)
int         AUTO_SENSOR_INSTALL             =0;                                 //wei 20160914 Auto Shuttle Sensor
int         USE_TRAY_MAPPING                =etmUninstall;                      //wei 20161219 Tray Mapping
int         USE_TRAY_ROBOT                  =0;                                 //Steven 20170330 (Wei) : For HT-9046LM
int         USE_LOADER_HINGE                =0;                                 //Steven 20170330 (Wei) : For TSMC
int         ShuttleVibration_INSTALL        =0;                                 //JerryYang 20171006 (wei) Shuttle 震動馬達
int         USE_Fix_AI_CCD                  =0;                                 //RogerYang 20180901 add 矽格湖口Demo AI CCD Function
int         USE_DIE_CLEAN                   =0;                                 //wei 20170418
int         USE_MR_SYSTEM                   =0;                                 //wei 20180702 MR
int         USE_RFID_SYSTEM                 =0;                                 //wei 20180702 MR
int         USE_RFID_READER                 =0;                                 //Steven 20220713 : RFID Reader for SJSEMI
int         USE_AUTO_ALIGNMENT              =0;                                 //Sam 20181201 : AutoAlignment

int         USE_OHT_SYSTEM                  =0;                                 //KaiChen 20200716 ：OHT
int         USE_Multile_Empty               =0;                                 //KaiChen 20200716 ：OHT
int         USE_KEYENCE_LOADER              =0;
int         USE_KEYENCE_EMPTY               =0;
int         USE_MultileEmptyTrayID_Keyence  =0;
int         USE_COLORSENSOR_MUN             =0;                                 //Jimmychiu 20230630 : add color sensor MU-N in Loader
int         LoaderUnload_StepMotor          =0;                                 //Steven 20200529 : Loader入Tray改步進
//AnsiString  TrayStepMotor_ComPort           ="COM18";
int         i24V_PULSE_COUNT                =0;                                 //Ifor 20190311 : add 24V Monitor Use Pulse Type
int         USE_FINGER_PRINT                =0;                                 //Steven 20190503 : 指紋辨識權限
int         DETECT_CURRENT                  =0;                                 //Richard 20220928 :電流偵測模組
int         USE_DEVICE_FLIPPER              =0;                                 //Frank 20210612 : Flipper Function
int         CHAMBER_USE_PULSE_TYPE          =0;                                 //Ifor 20190422 : add Chamber Use Pulse Type
int         USE_GROUND_MAN                  =0;                                 //Steven 20190828 : 通訊式GroundMan
int         Ground_Man_ScanPoint            =0;                                 //KaiChen 20191005 ：通訊式GroundMan
int         Ground_Man_AlarmOhm             =0;                                 //KaiChen 20191005 ：通訊式GroundMan
bool        USE_KASUGA_Fan                  =0;                                 //KaiChen 20191225 ：KASUGA Fan 通訊
int         iUseHTIonBarFunction            =0;                                 //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
bool        bUseHTIonBar_PowerReset         =false;                             //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
int         iUseHTIonBar_PowerResetCount    =0;                                 //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
int         iUseHTIonBar_SendPowerStatus[MAX_HTIONFAN] = {0,0,0};               //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
int         SOCKET_AMP_QTY                  =0;
int         SOCKET_AMP_QTY_2nd              =0;
int         SOCKET_AMP_QTY_3rd              =0;                                 //JerryYang 20260205 : add第三組Socket sensor(8+16+8)
int         ROTATE_AMP_QTY                  =0;
int         COLOR_AMP_QTY                   =0;
int         VibrationMotorCount             =0;                                 //JerryYang 20230814 : add震動馬達通訊調速版本

int         NUDN1_QTY                       =0;                                 //Sam 20210518 : 新增 CanBus 軟體配置
int         NUDN1_MACID11_AMP_QTY           =0;                                 //Sam 20210518 : 新增 CanBus 軟體配置
int         NUDN1_MACID12_AMP_QTY           =0;                                 //Sam 20210518 : 新增 CanBus 軟體配置
int         NUDN1_MACID13_AMP_QTY           =0;                                 //Sam 20210518 : 新增 CanBus 軟體配置
int         NUDN1_MACID14_AMP_QTY           =0;             //Sam 20210518 : 新增 CanBus 軟體配置

int         TRAY_ARM_MODE                   =0;                                 //Frank 20230419
int         USE_2nd_LOADER                  =0;                                 //Steven 20240822 : For HT-9046AU
int         AUTO3_IS_MAGAZINE               =0;                                 //JerryYang 20220909 : add magazine
int         MAGAZINE_BIN_DISP_TYPE          =0;                                 //JerryYang 20220909 : add magazine
int         In_Shuttle_Auto_Latch           =0;                                 //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
bool bInSh1DoLtc=false;
bool bInSh2DoLtc=false;
bool bInSHLtcFin[2]={false, false};                                             //KenHsieh 20251106 : fix close site don't do In Sht Latch
int  iInShtZRange=250;                                                          //KenHsieh 20251107 : Add Range for InSHZ 避免初始高度太高造成校正之門檻值過高
bool bNeedGetSHRightLtc[2]={true, true};                                        //KenHsieh 20251118 : 修改Out Sht 會重新GetLtc 導致Lose IC 異常
bool bInSht1ZNeedHome=false;                                                    //KenHsieh 20250924 : 避免InSHZ 回Home時暫停重啟後全機回Home
bool bInSht2ZNeedHome=false;                                                    //KenHsieh 20250924 : 避免InSHZ 回Home時暫停重啟後全機回Home
bool bInSHLtcErrNo[2][10]={{false}};                                            //KenHsieh 20251128 : InShtLtc 新增異常重複判斷

bool bContactCTOverCHK=false;
bool bContactTimeOverStep=false;
bool bNowDoInterFaceErrorStep=false;

int iBackUpZ1DownPosition=0;
int iBackUpZ2DownPosition=0;

bool bPlaceToShuttleFirst=false;
TQPF_Timer MyTTLSOTTimer;                                                       //Steven 20180808 (wei) : TTL的時間單位改成microsecond(μs)
TQPF_Timer TestIntervalsTime;                                                   //kevin 20160311 取得測試機間隔時間
TQPF_Timer TestIntervalsBoostTime;                                              //JerryYang 20181122 (Steven) :  (Steven) : 將不同function計時器分開
TQPF_Timer PauseIntervalsTime;                                                  //kevin 20181009 取得機台停止時間
TQPF_Timer SOTPauseIntervalsTime;                                               //kevin 20181102 (Steven) : 取得機台停止時間
int iTriggerBoostFunction=-1;                                                   //Steven 20180817 : Boost Function
int iTriggerBoostFuncBack=-1;
bool bFlagBelowTurnOfValve=false;
bool bFlagBelowTurnOfValve1=false;
int iBoostFuncStep=0;
int iBoostCountDown=0;
double iBoostEotToSotTime=0.0;
TQPF_Timer BoostCoolTime;
TQPF_Timer BoostCoolCountDown;
TQPF_Timer BoostCoolStepTimer;

double dBoostOffset=0.0;
double dBoostCoolSec=0.0;
bool bBoostFirstBoost=true;
bool bSetBoostOffset=false;
//ChungHung 20130910 alter for SCK can close site by Index
bool bTestSiteUse[2][MAX_SOCKET_ROW][MAX_SOCKET_COL]={
                            {{false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false},
                             {false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false}},
                            {{false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false},
                             {false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false}},
                           };
bool bTestSiteUseEE[2][MAX_SOCKET_ROW][MAX_SOCKET_COL]={                        //Alick 20160923 add for 第三組工程師用開關SITE
                            {{false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false},
                             {false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false}},
                            {{false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false},
                             {false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false}},
                           };

int InArmZIndex[2][8]={{MInArmZA, MInArmZC, MInArmZE, MInArmZG, MInArmZAe, MInArmZAf, MInArmZAg, MInArmZAh},                //Steven 20240601 : 修正記憶體溢位
                       {MInArmZB, MInArmZD, MInArmZF, MInArmZH, MInArmZBe, MInArmZBf, MInArmZBg, MInArmZBh}};
int OutArmZIndex[2][8]={{MOutArmZA, MOutArmZC, MOutArmZE, MOutArmZG, MOutArmZAe, MOutArmZAf, MOutArmZAg, MOutArmZAh},
                        {MOutArmZB, MOutArmZD, MOutArmZF, MOutArmZH, MOutArmZBe, MOutArmZBf, MOutArmZBg, MOutArmZBh}};

int SortArmZIndex[2]={MOutSortAa, MOutSortAb};                                  //RogerYang 20250416 for HT9046AU add Sort Arm Z

bool bPlace8IC=true; //ChungHung 20110322 add
bool bTestSiteNeedSetErrBin[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false},   //JerryYang 20170609 (wei) 記錄out shuttle lose IC的site
                                                             {false,false,false,false,false,false,false,false},{false,false,false,false,false,false,false,false}};
bool bOutShtLoseICNeedSetErrBin=false;                                          //JerryYang 20170609 (wei) 此旗標為true需將out shuttle lose IC對應的site設成error bin
bool bIndexDropICNeedSetErrBin=false;                                           //JerryYang 20170609 (wei) 此旗標為true需將out shuttle lose IC對應的site設成error bin
//----------------------------------------------------------------------------
int iContactMode=0;
int iTestRunMode=0;
int iRunStartMode=0;
int iBinModelPrime=0;                                                           //ChungHung 20120912 add Bin Setting like Epson

int InArmClose_Pitch=0;
int OutArmClose_Pitch=0;

int InArmClose_PitchX=0;
int InArmClose_PitchY=0;                                                        //ChungHung 20131231 alter AutoYPitch

int OutArmClose_PitchX=0;
int OutArmClose_PitchY=0;                                                       //ChungHung 20131231 alter AutoYPitch

int SortArmClose_PitchX=0;                                                      //RogerYang 20250515 Add for 9046AU

const int PickMode=0;
const int PlaceMode=1;
int iForPlaceHPX6Step=0;
int iForPickHPX6Step=0;
int iForPlaceHPX3Step=0;                                                        //20111015  Dell
int iForPickHPX3Step=0;                                                         //20111015  Dell

int iForPlaceHPX10Step=0;                                                       //ChungHung 20150127 add for ATK 10X16 Hotplate
int iForPickHPX10Step=0;                                                        //ChungHung 20150127 add for ATK 10X16 Hotplate

int iPlaceHPOrder=0;
int iPickHPOrder=0;
int iRecPickHPOrder=0;                                                          //Ifor 20180521 :add 改全域
int iPickHotPlateFinish=0x00;                                                   //Ifor 20180521 :add 改全域

bool OnlyRow2=false;                                                            //ChungHung 20121001 : 移至cmydef Reset後此參數未清為false 一樣HangUp

int  iOutArmiWhichKit=0;
int  iOutArmiWhichKit_Sort=0;                                                   //RogerYang 20250710 Add for 9046AU
int  iSortArmiWhichKit=0;                                                       //RogerYang 20250526 Add for 9046AU
int  iInArmOrder=0;
int  iWhichSht32=0, iWhichKit32=0;                                              //Ifor 20180116 (Steven) : add 改成全域變數
int  iYHalf=0;
int  iXHalf=0;
bool bResetMode=false;
bool bCheckIndex=false;                                                         //Steven 20101116 : 固定數量後要做Index Check
int  iWhoTriggerPiggyBack=0;                                                    //Steven 20111207 : 誰觸發了Piggy Back
bool bQAModeQuickCleanOut=false;                                                //Steven 20111005 : 數量到的時候的快速CleanOut
bool bQAModeFinishCleanOut=false;                                               //Steven 20111005 : 做完QA模式的CleanOut
int  iQAModeLoaderCT=0;                                                         //jou 2012-05-03 增加QA mode使用獨立的Loader Count
int  iQAModePassCT=0;                                                           //JerryYang 20221004 : Maxim版本QA mode
bool bQAModeStart=false;                                                        //Ifor 20201102 add: 新增QA Mode Start 旗標 避免Count 被清除
bool bCalculatePauseTime=false;
int  iUPH_LoaderCount=0;
bool bRecordUPH=false;
TDateTime tUPH_PauseTime=0;
TDateTime tUPH_PauseStartTime=0;
TDateTime tUPH_StartTime=0;
TDateTime tUPH_EndTime=0;
AnsiString sAlarmTime;
AnsiString sFileNameTime;                                                       //Steven 20190801 : 修正檔名錯誤
unsigned char SENDBUFF[1024];
unsigned char READBUFF[1024];                                                   //Steven 20110927 : 會溢位,改成1024
bool bTakeAway=false;                                                           //kevin 20190806
AnsiString sLoadCellReceData="";                                                //kevin 20190906 add load cell NG
bool bloadcellRece=false;                                                       //kevin 20190906 add load cell command
int iloadcellRece=0;                                                            //kevin 20190906 add load cell command
int iloadcellCount=0;                                                           //kevin 20190906 add load cell command
//---------------------------------------------------------------------------
int iCloseSiteState=0;                                                          //Eliot 2009_03_06
int iCloseSiteStateForPick=0;                                                   //Steven 20150924 : 隨時開關Site功能
//----------------------------------------------------------------------------
int iSH1LED;
int iSH2LED;
int iSH1States;
int iSH2States;
bool bCheckShuttleStates=true;
AnsiString sSuperVisorString;
int CUSTOMER_CODE=0;
int InOutArmPickerUseMotor=0;
bool bAfterASMTest=false;
bool bASMCleanOutFinish=false;
//----------------------------------------------------------------------------
//Temp IC
bool bTempComm6ReceiveOK=false;
//----------------------------------------------------------------------------
bool bRunOcrInsp=false;                                                         //ChungHung 20120830 add OCR Function add
//----------------------------------------------------------------------------
//Auto Decay Test
bool bRunDecayTest=false;                                                       //Ifor 20151015 新增執行Decay Test 畫面顯示
//Auto Cleaning
bool bRunAutoClean=false;
int iAutoClean_IndexContactCount=0;
long lAutoClean_TimeCount=0;                                                    //jou 20250102 : auto clean triger time count
int iDoAutoCleanTask=1;
int iDoShuttle1AutoCleanTask=1;
int iDoShuttle2AutoCleanTask=1;
int iDoShuttleAutoCleanTask=1;
int iDoIndexAutoCleanTask=1;
int iAutoCleanPickFromShuttleTask=1;
int iAutoCleanPlaceToFix3Task=1;
int iAutoCleanPlaceToHotplateTask=1;                                            //kevin 20120217
int iAutoCleanPlaceToCleanKitTask=1;                                            //jou 2012-05-24
int iInArmCleaniWhichKit=0;                                                     //0: 第一次8SITE  1:9-16 SITE
int iCheckFinish_ByAutoClean=0;                                                 //pig 2011.09.01 AutoClean
bool bCleanKitPitchOver12000=false;                                             //jou 2013-02-20
bool bCleanKitPitchLess4000=false;                                              //jou 2015-03-24 tray x pitch太小導致無法吸取
bool bPlaceToShuttleByAutoClean=false;                                          //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
bool bPickFromShuttleByAutoClean=false;                                         //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
bool bPickFromKitByAutoClean=false;                                             //Steven 20210603 : 新增吸放Clean Kit的Flag
bool bLockPlaceToShuttleByAutoClean=false;                                      //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
bool bLockPickFromShuttleByAutoClean=false;                                     //ChungHung 20150129 add when Index Jam SCK want to Inarm move to safe postion
bool bRunATCCalibation=false;                                                   //Ifor 20160719 新增執行ATC Calibation Test 畫面顯示
const int M_INIT_START     =0x0001;
const int M_INIT_RESTART   =0x0002;
const int M_FINISH         =0x0004;
const int M_MANUAL         =0x0008;
const int M_SOCKET_ALARM   =0x0010;
const int M_INTERVAL       =0x0020;
//----------------------------------------------------------------------------
bool fRearNeedTest=false;
bool fFrontNeedTest=false;
bool fTwoArmNeedTest =false;                                                    //2013-01-15    Dell    Add nn Mode
bool fIndexNeedTest=false;

int  iUnLoaderCount=0;                                                          //Steven 20091214 : 表示位置: Auto 1, 2, 3
//----------------------------------------------------------------------------
const int DETECT_SENSOR_FLAG =1;                                                //jou 981215 start : InArmZSafe() & OutArmZSafe()
const int DETECT_POS_FLAG    =2;
const int DETECT_ALL_FLAG    =3;

AnsiString asRecordTestResult;
bool bBin16HangUp=false;

int INDEX_DRIVER_TYPE       =Panasonic_DRIVER;                                  //Eliot 2010_04_29
bool TorqueUseHPComCard     =false;                                             //Steven 20210204 : 使用鴻勁自製的通訊卡
int MOTOR_DRIVER_TYPE       =Panasonic_DRIVER;                                  //Steven 20110321 : 分辨其他軸使用的馬達廠牌
int iPanasonicDriverType    =Panasonic_DRIVER;                                  // 2011.08.11 , Joye , Panasonic A5 //Steven 20120629 add from 7045
int CONTECT_SHUTTLE_KG      =10;                                                //jou 2011-03-15
int CONTECT_SHUTTLE_KG_MITSUBISHI=6;                                            //Steven 20140530 : 4 --> 6
int CONTECT_SHUTTLE_240KG   =15;
int iTorqueCommMaxTime      =12;                                                //Time Out秒數

//AI(W0-TAIL) 20260626: TODO(W7) -- 'TMyLog myLog' definition deferred (TMyLog is
//  VCL-form-coupled, fwd-declared in cmydef.h). extern decl stays; def lands W7.
#if 0
TMyLog myLog;
#endif                                                                   //Steven 20100629 : 紀錄修改項目
AnsiString asLogPath="D:\\HT9045_Log\\ChangeLog";                               //Steven 20100907 : 換一下位置
AnsiString asUser="";

bool bClearJamRateCount=false;                                                  //jou 2010-08-13 計數jam rate,改為Tray Feed為一單位

bool bLoaderNoTrayAutoCleanOut=false;                                           //jou 2010-09-01 Loader沒有tray時,Loader不再進Tray,Tran End時自動clean out
bool bSecondTimeLoaderCheckNoTray=false;                                        //ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray
bool bNeedCheckOurArmDeviceSuck=false;                                          //jou 2010-09-07 start : 確認Out arm有確實將device吸走
bool bReOpenGpib=false;                                                         //Steven 20101013 : 每次OneCycle或CleanOut後，要重開GPIB
bool bLoaderHasSkip=false;                                                      //Steven 20101019 : Loader有被Skip IC。
bool bLoaderHasSuck=false;
int  iManualRemoveTrayCnt=0;                                                    //Sam 20190731 : 當 Load 發生 Pickup Error Skip 時，當前這盤除了要手動移除，下一盤也要手動移除。
bool bBackupCleanOut=false;                                                     //jou 2010-12-07 : 防止index check oncyle與clean out funtion衝突
bool bEnterTestIF=false;                                                        //ChungHung 20121221 add
bool bCleanOutTrayEnd=false;                                                    //jou 2010-12-15
bool bLifterPause[7]={false, false, false, false, false, false, false};         //jou 2011-02-21 start : magazine沒動作會alarm
double fHotPlateExpansionCoefficient=1.0;                                       //Steven 20110324 : 加熱盤的膨脹係數

bool bSiteMappingCHKOK=false;                                                   //是否完成Site mapping檢查,預設值是未完成
int  iDoSiteMappingStep=0;                                                      //目前做到第幾個Site
int  iNowSiteCT=0;                                                              //目前機台做幾個SiteCheck
bool BackSiteFinish=false;
int  iGetTestData[4][8];
bool bShowAutoSiteMappingError=false;
int  iShuttleHeaterWaitTime=0;
bool bGetOpenBin=false;

bool bRefreshFunction=false;                                                    //使用Refresh主畫面site跟著檔案變
int  MachineTypeChoice=Type_HT9045;
int  SubMachineType=Type_None;                                                  //0:None 1:HT9046LA //Steven 20190307 : Add for HT-9046LA
int  MachineNumber=0;                                                           //kevin  機台機號識別
int  NEW_MAX_Index_Col=8;

bool bIndexPlaceIcCheck=false;                                                  //kevin 20110505 ASE 高雄 index放完ic在shuttle 檢查是否殘留ic,在負壓系統會因發alrm 造成負壓降下來另一arm 因真空值不足關閉造成掉料
unsigned int ContinuousPassBinCount[256]={0};                                   //kevin 20140305 256 bin [30]={0};    //Steven 20110502
unsigned int ContinuousPassBinBySocket[MAX_SOCKET_ROW][MAX_SOCKET_COL]={0};     //Steven 20110919
int iSpeedFast=50000, iSpeedSlow=30000, iSpeedY=(50000*2);                      //Steven 20110503 : 240KG加速 //Ifor 20150703 加快Y移動速度

bool bIndexEveryTimeCheckEPing=false;                                           //Jou 20110505
int iInitSpeedSh1=0, iInitSpeedSh2=0;                                           //記錄DATABase 速度資料                kevin 20110531
int iPJogHighSpeedSh1=0, iPJogHighSpeedSh2=0;                                   //記錄DATABase shuttle速度資料 kevin 20110531

bool bRealTimeThreadDetect=false;
bool bWantToStopChamberFan=false;                                               //Steven 20110725 : 要可以關閉加熱風扇

double dCCDTemperature=0.0;                                                     //Steven 20110924 : 確認CCD溫度
int iTempOverShowAlarmT[tcTotalCount];                                          //kevin 20210112 溫度過低過高

int iSynTekCardType=eSCTPcil112;                                                //Steven 20111004
bool bUT150Install[tcTotalCount]={false};                                       //Steven 20111012 : 改用單獨宣告,不放在LastSet裡面
bool bUT150Install1To2[tcTotalCount]={false};                                   //kevin 20190921 1對2site
bool OLPClientConnect =true;

bool bInitialMaxTime=true;                                                      //jou 2011-11-09 增加initial max time set

bool bRecordPiggyBackStart=false;                                               //jou 2011-11-14 start : 紀錄piggyback時間
bool bRecordPiggyBackEnd=false;
int iPiggyBackTime[2]={0,0};

AnsiString pwPath("C:\\winnt\\system32\\tech.com");                             //2012-01-03    Dell
AnsiString pwName="tech.com";                                                   //Steven 20221216 : 吳如春要改成從網路抓密碼本

bool bRecIndexDropAlarm1=false;                                                 //jou 2012-01-17 紀錄index Drop alarm
bool bRecIndexDropAlarm2=false;                                                 //jou 2012-01-17 紀錄index Drop alarm
bool bHALTing=false;                                                            //jou 2012-01-30 常溫 & 機台沒在跑的時候chamber風扇不轉

bool bRTCArm1HalfViewError=false;                                               //Steven 20120206 : RTC重複錯誤
bool bRTCArm2HalfViewError=false;                                               //Steven 20120206 : RTC重複錯誤
bool bRTCFullViewError=false;                                                   //Steven 20120206 : RTC重複錯誤
int  XResolution=1024;                                                          //Steven 20120206 : 畫面解析度
int  YResolution=768;                                                           //Steven 20120206 : 畫面解析度
bool bFilterTheAgainData=false;                                                 //Steven 20120222 : 過濾掉Duplicate的訊息
bool bCanUseHotPlateCheck=false;                                                //Steven 20120323 : 可以檢查HotPlate的模式

double T_MODE_SPEED=0.9;                                                        //jou 2012-05-30 T Mode Speed

bool bNeedCheckIndexToque=false;                                                //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
bool bNeedCheckIndexToque1=false;                                               //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
bool bNeedCheckIndexToque2=false;                                               //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
const int ciIndex5FSetValue=80;                                                 //jou 2012-06-12 吸shuttle時，需檢測Torque，過大需alarm
//==============================================================================
// GEM function Start
//==============================================================================
bool GEM_EMGPressed=false;

int  ATC_SYSTEM                 =0;                                             // 2011.04.15 , Joye , ATC
int  ATC_MixMode                =0;                                             //JerryYang 20250729
int  iControllerCheckDelay      =5;                                             // 2012.03.26 , Joye & Pig , ATC Connect Issue
bool bATCConnectError           =false;                                         //pig 2011.12.21 ATC改
int  iStopATCChillerType        =0;                                             // 2012.05.07 , Joye , Chiller
int  iTestData=0;
bool MasheReCordData            =false;                                         //kevin 20120616記錄程式離開資料
bool bChangeCleanPad            =false;                                         //kevin 20120709 autoclean 需要開門
bool bIsAutoOneCycleAutoclean   =false;                                         //kevin 20120710 目前onecycle 中準備 autoclean
int  iAutoCleanShuttle          =0;                                             //kevin 20120711 Shuttle 偵測 Clean Pad是否放好
bool bACInitialStart            =false;                                         //kevin 20120712 Initial 模式一開始作autoclean
bool bACInterval                =false;                                         //kevin 20120712 index次數到再做autoclean
int  iAutocleanInitialStart     =0;                                             //kevin 20120712 initial start 被啟動
bool bACManual                  =false;                                         //kevin 20120712 Manual 主畫面秀 autoclean button
bool bAutoCleanCheckOpenDoor    =false;                                         //Steven 20121015 : Auto Clean有Alarm要開後門
int  bUseCleanArm                =1;                                            //kevin 20140903 AutoClean 使用哪一隻ARM  :  1 Arm2  0 Arm1

//-----------------------------jou-----------------------------------------------
bool bIndexCheckNoStopVaccum    =false;                                         //jou 2012-05-09 Index Check時,如果中途按暫停,也要繼續把suck()做完,避免負壓掉ic kevin 20120831
bool bCleanoutStart             =false;                                         //kevin 20120921 cleanout已使用 但onecycle 在做完了繼續cleanout

int  INSTALL_HEAT_GUN           =0;                                             //Kevin 20120523 AirGun 熱風槍安裝
int  INSTALL_ATC_HEAT_GUN       =0;                                             //JerryYang 20220408 : add for ATC3.5
//bool bChangeSiteMAP             =true;                                          //kevin 20130104 可以改變sitemap

bool bUseAuto2Empty=false;                                                      //kevin 20120718 京元 auto2 load 空 TRAY  改成分離氣缸
bool bAuto2Pause[7]={false, false, false, false, false, false, false};          //kevin 20120718   //jou 2011-02-21 start : magazine沒動作會alarm

int  iTestTimeUpErrContinueR    =0;                                             //jou 2012-08-28 菱生要求 Test Time Up Error 之後的兩次測試, 不管結果如何都排到R
bool bASMFinishOneCycle         =false;                                         //Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray
bool bWakeupGPIBFile            =false;                                         //Steven 20110116
bool bWakeupEventLogSaverFile   =false;                                         //Frank 20140902
bool bWakeupDMCCFile            =false;                                         //Alick 20160721 add Barcode File切換
int  iInitStartDelayTimeCT      =0;                                             //jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time
int  iTestBinCount              =16;                                            //kevin 20140308 bin0-15 15;    //Steven 20121112 : RS232支援32Bin
//bool bCleanOutStart             =false;                                         //Kevin 20120921
bool bContinusRotate            =false;                                         //2013-04-12    Dell :旋轉站;
bool bResetIsPressed            =false;                                         //Steven 20130625 : 新的Reset方式
bool bResetShuttle              =false;
bool bResetInArm                =false;
bool bResetInArmTask            =false;
bool bResetOutArm               =false;
bool bResetOutArmTask           =false;
bool bResetSortArm              =false;                                         //RogerYang 20250515 Add for 9046AU
bool bResetSortArmTask          =false;
bool bResetIndexArm             =false;
bool bResetIndexArm1Pick        =false;
bool bResetIndexArm2Pick        =false;
bool bResetLoadTray             =false;
bool bResetUnLoadTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};
bool bChamboFanCloseTemp        =false;                                         //kevin 20130407關閉chambo 溫度
bool bIndexTimeSet              =false;                                         //kevin 20130321 設定index time
double fIndexTime               =0.0;                                           //kevin 20130321 設定index time
bool bIsContactforce            =false;                                         //kevin 20130418 contact force over error
int iShowSocketSensor           =0;                                             //kevin 20130504 等index z讓開再秀alarm
bool bIsSocketSensor            =false;                                         //kevin 20130504 socket sensor detect error
bool bASEK15UsePW               =false;                                         //kevin 20130701  Ase高雄K15
AnsiString ASEK15PassWord       ="";                                            //kevin 20130701  Ase高雄K15
//------------TSMC kevin 20130425--------------------------------------------------
//2013.01.11 Q_Q TSMC GPIB COMMAND
bool BUFFERDATAUSEREG           =false;
AnsiString asArmForce1          ="";                                            //2013.01.11 Q_Q TSMC GPIB COMMAND
AnsiString asArmForce2          ="";                                            //2013.01.11 Q_Q TSMC GPIB COMMAND

int iReadTIntervelArm1          =0;                                             //2013.01.26 Q_Q TSMC GPIB COMMAND Part 3.
int iReadTIntervelArm2          =0;                                             //2013.01.26 Q_Q TSMC GPIB COMMAND Part 3.
AnsiString asGPIBTempShow[tcTotalCount];                                        //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
bool bAllTemperatureIsReady=false;                                              //kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND
//------------------------------------------------------------------------------
bool bNeedAskStartMode          =false;                                         //ChungHung 20130529 add manual change start mode 如果是InitialStart 按下Start時要詢問
int iInArmBlockIndex            =0;                                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
int iOutArmBlockIndex           =0;                                             //2014-03-04    Dell    for SPIL WLP Add Tray Block
int iSortArmBlockIndex          =0;                                             //RogerYang 20250516 Add for 9046AU
bool bFirstTrayNeedAlarm        =false;                                         //ChungHung 20140521 add for ATK

//Motion_Card_Mode==============================================================
int MOTION_CARD_TYPE            =0;                                             //Brian 20121015
int IO_CARD_TYPE                =0;                                             //Brian 20121015
int TTL_CARD_TYPE               =0;                                             //Steven 20121122 : 選擇TTL Card 模式
int TTL_CARD_USE_ADDRESS        =0;                                             //Isaac 20210922 : 選擇TTL板子是否帶站別
int MOTIONNET_SPEED             =3;                                             //Steven 20181122 : MNet速度要可以修改
//==============================================================================
double dTempMax                 =135.0;                                         //2013-01-15   定義加熱可設定最高溫度
double dTempMin                 =20;                                            //2023-07-07   定義加熱可設定最低溫度
bool bCCDOverTemp               =false;
//==============================================================================
//各機型DI/DO 數量定義
const int DI_IP_Max_9045 = 19;                                                  //JerryYang 20200615 16->19      //2014-03-04    Dell    12 -> 16 for SPIL WLP Add 5S Inspection
const int DI_IP_Min_9045 = 5;
const int DO_IP_Max_9045 = 19;                                                  //JerryYang 20200615 16->19    //2014-03-04    Dell    12 -> 16 for SPIL WLP Add 5S Inspection
const int DO_IP_Min_9045 = 5;

const int DI_IP_Max_9046 = 19;                                                  //JerryYang 20200615 16->19    //JerryYang 20160607 for京元分離式浮動頭,15->16
const int DI_IP_Min_9046 = 5;
const int DO_IP_Max_9046 = 19;                                                  //JerryYang 20200615 16->19    //JerryYang 20160607 for京元分離式浮動頭,15->17
const int DO_IP_Min_9046 = 5;

const int DI_IP_Max_9046LS = 19;                                                //JerryYang 20200615 16->19   //wei 20150428 for ART   //wei 20150930
const int DI_IP_Min_9046LS = 5;
const int DO_IP_Max_9046LS = 19;                                                //JerryYang 20200615 16->19   //wei 20150428 for ART   //wei 20150930
const int DO_IP_Min_9046LS = 5;

//ChungHung 20130621 add
const int DI_IP_Max_9045_12Site = 19;                                           //JerryYang 20200615 16->19//Steven 20131114 : 11 --> 12   //jou 2014-04-08 12 -> 13 HT9045修改12site
const int DI_IP_Min_9045_12Site = 5;
const int DO_IP_Max_9045_12Site = 19;                                           //JerryYang 20200615 16->19//jou 2014-04-08 12 -> 13 HT9045修改12site
const int DO_IP_Min_9045_12Site = 5;

//==============================================================================
//JerryYang 20151109
const int Bit0_HandlerReBoot                =0;                                 //JerryYang 20151109 bit0 PASS
const int Bit1_HandlerOutputFull            =1;                                 //JerryYang 20151109 bit1 無空盤可放至Auto
const int Bit2_HandlerInputEmpty            =2;                                 //JerryYang 20151109 bit2 Loader tray有無IC
const int Bit3_ContactorCleaning            =3;                                 //JerryYang 20151109 bit3 做AutoClean     //ATP Handler Memory Full(Gone To Retest)
const int Bit4_HandlerDiagnostics           =4;                                 //JerryYang 20151109 bit4 設定參數中      //ATP Handler Diagnostic(R&M mode)
const int Bit5_IndexCheck                   =5;                                 //JerryYang 20151109 bit5 PASS            //ATP Handler Sort off
const int Bit6_Reversed                     =6;                                 //JerryYang 20151109 bit6 PASS            //ATP Handler Input off
const int Bit7_HandlerGuardband             =7;                                 //JerryYang 20151109 bit7 PASS
const int Bit8_HandlerJam                   =8;                                 //JerryYang 20151109 bit8 機台JAM
const int Bit9_HandlerStop                  =9;                                 //JerryYang 20151109 bit9 機台停止中
const int Bit10_HandlerSoak                 =10;                                //JerryYang 20151109 bit10 機台等待加熱完成,停止中
const int Bit11_HandlerDoorOpen             =11;                                //JerryYang 20151109 bit11 門禁
const int Bit12_HandlerEmpty                =12;                                //JerryYang 20151109 bit12 機台內無IC
const int Bit13_HandlerOk                   =13;                                //JerryYang 20151109 bit13 PASS
const int Bit14_Unloading                   =14;                                //JerryYang 20151109 bit14 Unloading中
const int Bit15_Loading                     =15;                                //JerryYang 20151109 bit15 Loading中
const int Bit16_Reversed                    =16;                                //JerryYang 20151109 bit16 IC都在Auto    //ATP Contactor Cleaning

//==============================================================================
//JerryYang 20161007
const int iHPNoUse8suck        =0;
const int iHPWideHP            =1;
const int iHP_Pin_RightSide    =2;
const int iHP1x2Step2          =3;
const int iHP1x2Step3          =4;
int i8PickerHPMode=0;
//==============================================================================
//JerryYang 20170531 (wei) 敲tray方式新增震動馬達
const int NonVibration      =0;                                                 //沒有敲trat機構
const int VibrationCylinder =1;                                                 //敲擊氣缸
const int VibrationMotor    =2;                                                 //震動馬達
//==============================================================================
bool bGail_HighSpeedMode = false;                                               //提高Index 運動速度(目前by9046LS專用)
//==============================================================================
//---------------------------------kevin 20130321 Dell DUT(Socket Base)溫控器增加為4顆------------------------
int  iSocketBaseTempCount=eDut1ea;                                              //Steven 20140116 : Socket數量,改用數字設定
int  iSocketBaseAdd4TempType=0;                                                 //kevin 20130812  DUT 4個溫控器使用   0: pansonic  1:omron
bool bK12TempHeadChoiceChambo=false;                                            //kevin 20130507 TSMC HEAD溫度改成 CHAMBO溫度
int iIndexArm=0;                                                                //kevin 20130605 K12 contract mode 確認 哪支arm在下
int iArmEPUse=0;                                                                //kevin 20130611 0:不充氣    1:arm1 充氣   2:arm2

bool bShuttleCooling=false;                                                     //jou 2010-06-09 start
bool bChamboCooling=false;                                                      //kevin 20130109
bool bHotplateCooling=false;                                                    //jou 2013-11-07

int AccessLevel=0;
bool bFirstRecordLoaderData=true;                                               //jou 2013-05-31 first record loader data
bool bContinuoussPass=false;                                                    //kevin 20131009 pass IC超過多設定次數秀警告
bool bContactCounOven=false;                                                    //kevin 20131009 CONTACT COUNT 超過多設定次數秀警告
int iAutoSkipCT=0;                                                              //jou 2013-09-23 add Auto SKIP CT
int iLoaderDropErrRetryCT[MAX_ARM_Row][MAX_ARM_Col]={0};                        //jou 2013-09-23 add Auto SKIP CT
bool bMoveInArm2XYToWaitOk=true;                                                //Steven 20131025 : 要先動到安全位置才可以再到別的地方
bool bInarmZOk=false;                                                           //kevin 20131119 Z軸正在吸取不能被中斷
bool bContaceTorque=false;                                                      //kevin 20131217 TOR-> 單一顆KG

bool bHomeinitialCheckPushZ1=false;                                             //kevin 20131218 歸hom前檢查是否有tray放在hotplate 造成機構損壞 按z1 確認
bool bHomeUnlock=false;                                                         //kevin 20131218 home按z1 解開 機台可以動作

int iWhichShuttle0000=0;                                                        //ChungHung 20130910 alter for SCK can close site by Index

double dTestSec;                                                                //2013-11-27    Dell 測試時間 //jou 2014-09-24 int -> float bWhenHappenTestedTimeBelowUseInitialDelay 測試秒數小於1 sec會誤判
int iStableTime;                                                                //JerryYang 20210122 : ASE-CL新增待溫功能
int iSoakTimer;                                                                 //2013-11-27    Dell Index Soak time 計數
int iSoakTimer_Start;                                                           //2013-11-27    Dell Index Soak time 計數
int iInitialSoakTimer;                                                          //Steven 20140827
int iInitialSoakTimer_Start;                                                    //Steven 20140827

int iATCTempWaitTimer;                                                          //Ifor 20230608 add:KYEC 要求新增主畫面顯示ATC Temp Wait 倒數
int iATCTempWaitTimer_Start;                                                    //Ifor 20230608 add:KYEC 要求新增主畫面顯示ATC Temp Wait 倒數

bool bNeedInitialTestDelay=false;                                               //ChungHung 20140425 add for TSMC Device
int  iInitialCount=0;                                                           //ChungHung 20140801 add Korea Want to count down in main status
bool bFirstDeviceInitialTestDelayWhichOutAfterAutoClean=false;                  //ChungHung 20141023 add for SCK request
bool bDoAfterAutoCleanFunctionUseInitialDelay=false;                            //ChungHung 20140105 add for SCK
bool bDoEveryFirstDeviceFunctionUseInitialDelay=false;                          //ChungHung 20140105 add for SCK
bool bDoAfterShowAlarmMessageUseInitialDelay=false;                             //ChungHung 20140105 add for SCK
bool bDoAfterOpenHeatDoorUseInitialDelay=false;                                 //ChungHung 20140105 add for SCK
bool bDoWhenHappenTestedTimeBlowUseInitialDelay=false;                          //ChungHung 20140105 add for SCK
bool bDoOTDOffUseInitialDelay=false;                                            //Steven 20160818 : OTD打開Delay

bool bDoWhenPressStopOverUseInitialDelay=false;                                 //ChungHung 20150526 add for ATK want to even stop over will use initial delay
bool bDoWhenNoFullSiteUseInitialDelay=false;                                    //wei 20151228 No FullSite delay
bool bTestFinishToNextTestOver=false;                                           //kevin 20160310 測試完成到下一次完成超過所設時間須啟動
bool bTestStartToNextTestStart=false;                                           //kevin 20181101 SOT 間隔時間驅動
bool bTestStartToNextTestStartDelay=false;                                      //kevin 20181101 SOT 間隔時間驅動
bool bHISIInitiayDelay=false;                                                   //kevin 20200110 add 海司強至initial delay
bool bCancelHISIInitiayDelay=false;                                             //Ifor 20230201 add: 取消海司強至initial delay

int iAfterTestedCount;                                                          //ChungHung 20140730 add for ATK function after tested delay time

bool bButtonManualStep=false;                                                   //ChungHung 20150526 add for QualComm US
bool bButtonManualTStart=false;                                                 //ChungHung 20150526 add for QualComm US

//==============================================================================
AnsiString DownloadWorkFile_NET = "";                                           // 2013.12.03 , Joye , KYEC FTP     //20140103  wei
int LastSetTemperature_NET      = -1;                                           // 2013.12.03 , Joye , KYEC FTP
AnsiString SYS_SetupFile        = "";
bool bFTPDownloadSetupFile      = false;                                        // 2013.12.02 , Joye , KYEC FTP Download
bool bBarcodeReader             =false;                                         //20140120 wei

bool ReEnterBarcode[20]={false, false, false, false, false, false, false, false,
                         false, false, false, false, false, false, false, false,
                         false, false, false, false};                           //20140310  wei   Barcode Reader

//==============================================================================
const int ArmMaxPitch_13Suck=3000;                                              //使用1, 3吸嘴
const int ArmMaxPitch=3750;                                                     //Steven 20140212 : For Xilinx 31x31生產8Site          //wei 20141205 4000-->3750
bool bTestingStopAllMotor=false;                                                //jou 2013-09-25 Testing Need Stop All Motorextern
//==============================================================================
//行車紀錄器
bool bCarRecordTimeStart=false;                                                 //wei 2013-12-09
bool bCarRecordTimeEnd=false;                                                   //wei 2013-12-09
TQPF_Timer  CarRecord;
bool bDutHasOverTemp=false;                                                     //Steven 20140308 : DUT溫度異常到下次Start前不加熱
int  iBarcodeTimeCount[bcTotal];                                                //20140310  WEI : [A09] Barcode Reader持續時間
//==============================================================================
bool bDutflag[32]={false, false, false, false, false, false, false, false,
                   false, false, false, false, false, false, false, false,
                   false, false, false, false, false, false, false, false,
                   false, false, false, false, false, false, false, false};   //ChungHung 20140611 fix 紀錄測試的Dut位置 對應錯誤
//bool bIndexDutflag[2][4][8]={{{false, false, false, false, false, false, false, false},  //kevin 20140318 紀錄目前index有IC需測試
//                              {false, false, false, false, false, false, false, false},
//                              {false, false, false, false, false, false, false, false},
//                              {false, false, false, false, false, false, false, false}},
//                              {{false, false, false, false, false, false, false, false},  //kevin 20140318 紀錄目前index有IC需測試
//                              {false, false, false, false, false, false, false, false},
//                              {false, false, false, false, false, false, false, false},
//                              {false, false, false, false, false, false, false, false}}};
bool bSkipNeedCheckVac[2][2][8]={{{false, false, false, false, false, false, false, false},{false, false, false, false, false, false, false, false}},  //JerryYang 20170609 (wei) 記錄pick up error按skip的site
                                 {{false, false, false, false, false, false, false, false},{false, false, false, false, false, false, false, false}}};
bool bArm1PressSkipNeedDownCheckVac=false;
bool bArm2PressSkipNeedDownCheckVac=false;

AnsiString TestSiteFileName[2][TotalTestMode]=                                  //Eliot 2009_12_23  //JerryYang 20201125 : TestSiteFileName移到cmydef
{
    {"Single Site",                     //SingleSite
     "2-Site",                          //DualSite
     "3-Site (1x3)",                    //TriSite1X3
     "In-Line 4-Site(1X4)",             //QualSite1X4
     "2-Site (2x1)",                    //DualSite2x1
     "Square 4-Site(2X2)",              //QualSite2X2
     "Square 4-Site(2X2) NN Mode",      //QualSite2X2N                          //Frank 20200520 2X2NN Mode
     "6-Site",                          //_6Site2X3                             //ChungHung 20140115 add for 2x3_6
     "6-Site (2x3) NN Mode",            //_6Site2X3N                            //Steven 20220425 : 2x3 NN mode
     "8-Site",                          //_8Site2X4
     "8-Site (2x4) NN Mode",            //_8Site2X4N                            //Wei 20231211 : 2X4NN Mode
     "10-Site",                         //_10Site2X5
     "12-Site",                         //_12Site2X6
     "16-Site",                         //_16Site2X8
     "16-Site (4X4)",                   //_16Site4X4                            //Sam 20190226 : 16Site4X4
     "32-Site N Mode",                  //_32Site4X8N
     "32-Site M Mode",                  //_32Site4X8M
     "8-Site Pop"                       //_8Site1X4                             //ChungHung 20150528 add for 海思 _8Site1x4
    },
    {"1site.bmp",                       //SingleSite
     "2site.bmp",                       //DualSite
     "3site.bmp",                       //TriSite1X3
     "4site.bmp",                       //QualSite1X4
     "2site2x1.bmp",                    //DualSite2x1
     "4siteRow.bmp",                    //QualSite2X2
     "2x2siteRow_NN.bmp",               //QualSite2X2N                          //Frank 20200520 2X2NN Mode  2x2siteRow_NN.bmp
     "6Site.bmp",                       //_6Site2X3                             //ChungHung 20140115 add for 2x3_6
     "2x3site_NN.bmp",                  //_6Site2X3N                            //Steven 20220425 : 2x3 NN mode
     "8Site.bmp",                       //_8Site2X4
     "2x4site_NN.bmp",                  //_8Site2X4N                            //Wei 20231211 : 2X4NN Mode
     "10Site.bmp",                      //_10Site2X5
     "12Site.bmp",                      //_12Site2X6
     "16Site.bmp",                      //_16Site2X8
     "16Site4X4.bmp",                   //_16Site4X4                            //Sam 20190226 : 16Site4X4
     "32SiteN.bmp",                     //_32Site4X8N
     "32SiteM.bmp",                     //_32Site4X8M
     "8-Site Pop.bmp"                   //_8Site1X4                             //ChungHung 20150528 add for 海思 _8Site1x4
    }
};

bool bOneCycleOperateChangeON_line=false;                                       //kevin 20140411 one cycle完如果是op模式off-line 需切換 on-line 並且發警告
int iOff_LINE_Mode=0;                                                           //kevin 20140411 目前機台工作模式是off-line:1  on_line :2
bool bTesterDucking=false;                                                      //jou 2014-04-25 Tester Docking Wait delay time
int  iTesterDucking=0;                                                          //jou 2014-04-25 Tester Docking Wait delay time

bool bNeedTesterDuckingFinishLightYellowAndAlarmOn=false;                       //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)
bool bTesterDuckingFinishLightYellowAndAlarmOn=false;                           //ChungHung 20141015 add for SCK When the handler start running (finished Start count function),Yellow light blinking & alarm on (press "alarm reset" then clear)

bool bRTCAutoModelVerifyFirstTime=false;                                        //jou 2014-06-24 RTC 自動進行Model驗證

int iDefHonPrecLevel    =3;                                                     //jou 2014-06-19 Security Have 5 Level
int iDefSupervisorLevel =2;                                                     //jou 2014-06-19 Security Have 5 Level
int iDefEngineerLevel   =1;                                                     //jou 2014-06-19 Security Have 5 Level
//ChungHung 20140716 modify SCK complain count never clear when change add reset count
//放到全域變數
unsigned int ContinuousFailSKTCount[MAX_SOCKET_ROW][MAX_SOCKET_COL];
unsigned int ContinuousFailARMCount[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];

 //ChungHung 20131223 add for SCK request start
unsigned int ContinuousFailSKTCount_AutoClean[MAX_SOCKET_ROW][MAX_SOCKET_COL];

unsigned int ContinuousFailARMCount_AutoClean[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];

//ChungHung 20140730 add ContinuousFailHaveOneCycle start
unsigned int SpecialBinContinuousFailSKTCount[MAX_SOCKET_ROW][MAX_SOCKET_COL];
unsigned int SpecialBinContinuousFailARMCount[2][MAX_SOCKET_ROW][MAX_SOCKET_COL];

int iAutoCleanByBinCount[TEST_MAX_BIN];                                         //Steven 20160308 : By Bin count do auto clean
int iAutoCleanBySiteCount[MAX_SOCKET_ROW][MAX_SOCKET_COL][TEST_MAX_BIN];        //Steven 20160308 : By Bin pre site count do auto clean

//ChungHung 20140730 add ContinuousFailHaveOneCycle end.

int iLoadPersentCT[TEST_MAX_BIN];                                               //Steven 20140830 : 改成全域變數
int iLoadCountCT[TEST_MAX_BIN];                                                 //Steven 20140830 : 改成全域變數
bool bAlarmReset=false;                                                         //Steven 20140905 : 紀錄有被按下Alarm Reset

bool bPiggyBackIndexCheck=false;                                                //jou 2014-09-03 Function State Stop Firt Initital Delay Time
int iContinueAutoSkipAutoTrayEnd=0;                                             //ChungHung 20141002 add for KYEC AutoRetest

AnsiString sMachineState    ="NULL";                                            // 2013.09.30 , Joye , KYEC SECS/GEM
AnsiString sMachinePreState ="NULL";                                            // 2013.11.27 , Joye , KYEC SECS/GEM
TQPF_Timer lHandlerStopTime;                                                    //jou 2014-09-21 Show Handler Stop Time //Steven 20190716 : 改成Timer

bool bTesterLowYieldOneCycle=false;                                             //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm

int ASE_InTrayNum=0;                                                            //kevin 20140918 input tray 數量

int iLoadTrayCount=0;                                                           //JerryYang 20240318

AnsiString ASET_FileNAME;                                                       //kevin 20140918  ASE高雄自動記錄檔案檔名
AnsiString ASET_ScheduleNAME;                                                   //kevin 20140918  ASE高雄自動記錄檔案生產批號
AnsiString ASET_INTQTY;                                                         //kevin 20140918  ASE高雄自動記錄檔案生產數量
AnsiString ASET_StartTimeNAME;                                                  //kevin 20140918  ASE高雄自動記錄檔案開始時間
bool bSwitchArm2=false;                                                         //Steven 20140922 : Arm2當作指紋測試
AnsiString AseIcRecord="";                                                      //kevin 20161228 記錄一支ARM IC吸取 資料

bool bTimeOutForNoFullSite=false;                                               //Steven 20141016 : FullSite的Test Time Out
double fRecindexCycleTim=0.0;                                                   //kevin 20141024 ASE index cycle time  sec
AnsiString RecindexCycleTim="";                                                 //kevin 20141024 ASE index cycle time  sec
bool bStartRecordAseError=false;                                                //kevin 20141028開始紀錄 alarm code
AnsiString aRecordAseError="";                                                  //kevin 20141028 紀錄發生
bool bReceiveSchedule=false;                                                    //kevin 20141104 收到ase 批號
AnsiString AseSaveFilePath="";                                                  //kevin 20150212 紀錄發生

int iInArmXBase=2;                                                              //Steven 20141029 : XY-Pitch
int iInArmYBase=0;
int iOutArmXBase=2;
int iOutArmYBase=0;
int iSortArmXBase=1;                                                            //RogerYang 20250506 Add for 9046AU
int iSortArmYBase=0;

int iInArmShtXCenterPos=-2000;
int iInArmShtYCenterPos=-3000;
int iOutArmShtXCenterPos=-2000;
int iOutArmShtYCenterPos=-3000;
int iSortArmShtXCenterPos=-2000;                                                //RogerYang 20250506 Add for 9046AU
int iSortArmShtYCenterPos=0;

int iInitContactCount=0;                                                        //Steven 20141117 : 起測時溫度要補Offset

int iRotato_In_Row=0;                                                           //wei 20141208 2x2 1x2  Pitch >3750
int iRotato_Out_Row=0;                                                          //wei 20141208 2x2 1x2  Pitch >3750

bool bEnableTempLess30degShowLight=false;                                       //jou 2014-12-19 Temperature Less 30 deg. Show Light
bool bTempLess30degShowLightFlag=false;                                         //jou 2014-12-19 Temperature Less 30 deg. Show Light

bool bEnableTempHeaterOkShowLight=false;                                        //jou 20180529 : Temperature Heater Ok Show Light
bool bTempHeaterOkShowLightFlag=false;                                          //jou 20180529 : Temperature Heater Ok Show Light

int iAutoSiteMap[4][8];                                                         //kevin 20150113 記錄測試機site map
int iAutoSiteMapBin[4][8];                                                      //kevin 20150113 記錄測試機site map
int iAutoSiteRecordIC[2]={0, 0};                                                //kevin 20150114 記錄有ic位置
int iAutoSiteMapSocketPass[4][8];                                               //kevin 20160513 記錄測試機site map 此socket 是否pass過  0:沒使用 1;pass 2;fail
int bTestBinDataError=0;                                                        //kevin 20150202 測試bin 別沒設定或關site 有bin 資料
bool bLowYieldAutoCleanEnd=false;                                               //wei 20141216  LowYieldautoclean  避免重複進去

int iLowYieldSiteCount[4][8]={0};                                               //wei 20141201 Low Yield Auto Clean    low Yield 統計
bool bLowYieldCleanOut=false;                                                   //wei 20141201 Low Yield Auto Clean    是否clean out
bool bLowYieldCloseSite[2][4][8];                                               //wei 20141201 Low Yield Auto Clean    close site
int iLowYieldCloseCount=0;                                                      //wei 20141201 Low Yield Auto Clean    close site 統計
bool bStandardYield=false;                                                      //wei 20141201 Low Yield Auto Clean    Yield 設定
int iStandardYield[4][8]={0};                                                   //wei 20141201 Low Yield Auto Clean    StandardYield
int iYield[4][8]={0};                                                           //wei 20141201 Low Yield Auto Clean    Yield
int iDifferenceYield[4][8]={0};                                                 //wei 20141201 Low Yield Auto Clean    DifferenceYield
bool bSuckEnd[MAX_ARM_Row][MAX_ARM_Col]={{false, false, false, false}, {false, false, false, false}};                //wei 20150318 suck完就等於true

bool bAutoSkipFlag=false;                                                       //jou 20150320   auto skip 次數時間導致Hang up
bool bSystemClose=false;                                                        //kevin 20150423 mainForm移過來
bool bZ1PickShuttle=false;                                                      //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing
bool bZ2PickShuttle=false;                                                      //Steven 20150407 : 修正[D45] Out Arm等Index Z功能, 避免Auto Homing

bool bAseRetry=false;                                                           //kevin 20150427
bool bAseTrayFeed=false;                                                        //kevin 20150427
bool bAseTrayEnd=false;                                                         //kevin 20150427
bool bAseAlarmReset=false;                                                      //kevin 20150427
bool bAseOneCycle=false;                                                        //kevin 20150427
bool bAseCleanOut=false;                                                        //kevin 20150427
bool bAseSKIP=false;                                                            //kevin 20150427
bool bAseHome=false;                                                            //kevin 20150427

bool bAsePause=false;                                                           //kevin 20150427
bool bAseStart=false;                                                           //kevin 20150427
bool bAseReset=false;                                                           //kevin 20150427
bool bTSTART=false;                                                             //kevin 20160303
bool bSTEP=false;                                                               //kevin 20160303

bool bUse_NewAutoCleanForm=false;                                               //kevin 20150507 使用新AUTOCLEAN FORM

bool bTEMPCTRL_Shuttle_TOGTHER=false;                                           //kevin 20150508 Shuttle 不分段加熱
bool bUseFix3CylinderActive=false;                                              //ChungHung 20150520 add fix Fix3 full 和 ShuttleShanke 衝突

bool bAutoReTest_ART=false;                                                     //kevin 20150529 要執行AUTO RETEST
bool bFirstYieldCmp_ART=false;                                                  //kevin 20150529 第一次AUTO RETEST YIELD比對
double fFirstYieldSet_ART=0.0;                                                  //kevin 20150529 第一次AUTO RETEST YIELD比對值設定   First yield >= 設定值不須RT
bool bOpenShortYieldCmp_ART=false;                                              //kevin 20150529 第一次AUTO RETEST open Short YIELD比對
double fOpenShortYieldSet_ART=0.0;                                              //kevin 20150529 第一次AUTO RETEST open Short YIELD比對值設定    First yield >= 設定值不須RT
bool bRecoverRateYieldCmp_ART=false;                                            //kevin 20150529 第一次AUTO RETEST RecoverRate YIELD比對
double fRecoverRateYieldRT1Set_ART=0.0;                                         //kevin 20150529 第一次AUTO RecoverRateYieldRT1 YIELD比對值設定
bool bSetYielddiffCmp_ART=false;                                                //kevin 20150529 第一次AUTO RETEST YIELD比對  FT - FT+RT1 Pass yield
double fbSetYielddiffSet_ART=0.0;                                               //kevin 20150529 第一次AUTO RETEST YIELD比對值設定  FT - FT+RT1 Pass yield <= 設定值則不須RT2
bool bSetSpecialBin_ART=false;                                                  //kevin 20150529 第一次AUTO RETEST
bool bAutoDetectBin_ART=false;                                                  //kevin 20150529 第一次AUTO RETEST
int iAutoRetestLimit=0;                                                         //kevin 20150601 做AUTO RETEST最大次數
int iTrayArmAutoRetestBuffer=0;                                                 //kevin 20150606 高雄鎖住 tray arm速度
int iIonFanDleayAlarm=1000;                                                     //kevin 20150610 高雄離子風扇偵測發alarm時間
bool bRunTrayError=false;                                                       //kevin 20150612 run tray 模式吸到ic 發alarm 手動取出tray
int iCountReceiveTray_ART=0;                                                    //kevin 20150613 計算收回tray
//bool HighTemperatureSet150=false;                                               //wei 20150617 改機最高溫150度
//bool HighTemperatureSet155=false;                                               //Sam 20240118 新增 155度 模式
//bool HighTemperatureSet175=false;                                               //Frank 20160705 add 改機最高溫175度
//bool bHighTempSetCustomizeEnable=false;                                         //Jimmychiu 20250626 : 最高溫度自訂
//double dHighTempSetCustomizeValue=200.0;                                        //Jimmychiu 20250626 : 最高溫度自訂
int iTempLimitation=0;                                                          //Steven 20250701 : 整合機台溫度設定
int  DewPoint_Hardware_Install=0;                                               //Steven 20191017 : 露點計
double dAdamValue_mA=0.0;
double dAdamValue_Degree=0.0;
bool bDewPointalarm=false;
bool bGpibRS232Error=false;                                                     //wei 20150617 Add version control
int iEPControlValue=0;                                                          //Ifor 20150710 EP 流量控制
int iSendGpibTestHome=false;                                                    //kevin 20150626 送出測試訊號 等收到資料才能歸home
bool bSecsGemCanStart=false;                                                    //wei 20150630  SecsGem Can Start
double fTrayYield[eTrayCount];                                                  //JerryYang 20220909 : 10->eTrayCount        //kevin 20150630  pass fail 率
bool bUsePassYieldTray[eTrayCount];                                             //JerryYang 20220909 : 10->eTrayCount        //kevin 20150701 fail bin 在良率判斷裡面算pass yield
bool bUseNewCleanModeKit=false;                                                 //kevin 20150701 使用autoclean模組 校正位置在 基座上 tray pin1
bool bART_needRT2=false;                                                        //kevin 20150717  RT 完成還需要rt2
bool bART_RT2RunNoChangeMode=false;                                             //kevin 20150717  只退fail RT2 不能更改測試模式
//bool bEmptyAndColorMustManualRemove=false;                                    //Sam 20230203 : 修正P24放Tray異常 Mark //Frank 20150626 : for矽格 Loader有Skip要到Empty的位置做檢查
bool bRotateChange=false;                                                       //Frank 20150626 : for矽格 要有IC方向顯示
// for autoclean offset 20150720
//int iArmXShuttle1OffsetPos=0;                                                 //kevin 20150720 In arm X 吸取Shuttle 1 IC Offset
//int iArmYShuttle1OffsetPos=0;                                                 //kevin 20150720 In arm Y 吸取Shuttle 1 IC Offset
int HotplatlXOffset=0;                                                          //kevin 20150720 add hotplate iXPos
int HotplatlYOffset=0;                                                          //kevin 20150720 add hotplate iYPos
int HotplatlPickOffset=0;                                                       //kevin 20150720 add hotplate Pick
int HotplatlPlaceOffset=0;                                                      //kevin 20150720 add hotplate place

int HotplatePitchOffset=0;                                                      //kevin 20150720 add hotplate Pitch
//int ShuttlePitchOffset=0;                                                     //kevin 20150720 add Shuttle Pitch
//int iArmPickShuttlePos;                                                       //kevin 20120623 In out arm 吸取Shuttle IC Offset
//int iArmPlaceShuttlePos;                                                      //kevin 20120623 In out arm 放Shuttle IC Offset
int iArmPickTrayPos;                                                            //kevin 20120623 In out arm 吸取TRAY IC Offset
int iArmPlaceTrayPos;                                                           //kevin 20120623 In out arm 放TRAY IC Offset
int iIndexPickShuttlePos;                                                       //kevin 20120623 Index 吸取 Shuttle IC Offset
int iIndexPlaceShuttlePos;                                                      //kevin 20120623 Index 放 Shuttle IC Offset
int iIndexWorkDownPos;                                                          //kevin 20120623 Index Clean down pos
int iIndexWorkUpPos;                                                            //kevin 20120623 Index Clean down pos
int bCleanHotplate_ART=0;                                                       //kevin 20150722 ART清hotplate 開關site  先按onecycle 再按cleanout
// for autoclean offset 20150720
bool bSECSGEMAlarm=false;                                                       //wei 20150817 S10F3 Alarm Reset按鍵
AnsiString asSECSGEMChangeName="";                                              //wei 20150803
AnsiString asSECSGEMChangePassword="";                                          //wei 20150803
bool bSECSGEMNoSendEC=false;                                                    //wei 20150803
bool bSECSGEMbyPass=false;                                                      //wei 20150803
bool bCheckOTDStatus=false;                                                     //wei 20150810 OTD Log
bool bOTDRecord=false;                                                          //kevin 20150827 OTD Log
AnsiString asSecsGemBarCode="";                                                 //Frank 20170109 modify asBarCode=>asSecsGemBarCode
AnsiString asSecsGemBarCodePasswoed="";                                         //Ifor 20180227 (Steven) add BarCode Password
AnsiString asBarCode="";                                                        //wei 20150811 SecsGem BarcodeReader
bool bSECSGEMConnectionFail=false;                                              //wei 20150817 SECSGEM 斷線
bool bSECSGEMConnectionFailOneCycle=false;                                      //wei 20150817 SECSGEM 斷線
AnsiString asSECSGEMPassWord="";
int iAutoRetestLimitFile=0;                                                     //wei  20150825 做AUTO RETEST最大次數
double iFailYieldRate_ARTFile=0.0;                                              //wei  20150825 AUTO RETEST YIELD比對

int iAutoLeastRetestLimitFile=0;                                                //wei  20160204
double dFailYieldRate_ARTFTFile[eTrayCount]={0.0};                              //wei  20160204
double dFailYieldRate_ARTRTFile[eTrayCount]={0.0};                              //wei  20160204
bool bAutoLeastRetestFile=false;
bool bUseFailNoDistinction=false;
int iUseFTFailYield=0;
int iUseRTFailYield=0;
int iUseFTFailYieldModel=0;
int iUseRTFailYieldModel=0;

bool bPassTrayFeed=false;                                                       //wei 20150826 Pass Tray Feed
int iAutoTrayCount[eTrayCount]={0};                                             //wei 20150828
bool bIndexCheckState=false;
bool bErrorAutoClean=false;

int iTemperatureOk=0;                                                           //kevin 20150914 加熱完成
bool iTemperatureOkFirstTime=false;                                             //kevin 20150914 加熱完成程式一開始
int iRecordIndexPosition=0;                                                     //kevin 20150915
int  iAseRetry=0;                                                               //kevin 20150925
int  iAseTrayFeed=0;                                                            //kevin 20150925
int  iAseTrayEnd=0;                                                             //kevin 20150925
int  iAseAlarmReset=0;                                                          //kevin 20150925
int  iAseOneCycle=0;                                                            //kevin 20150925
int  iAseCleanOut=0;                                                            //kevin 20150925
int  iAseSKIP=0;                                                                //kevin 20150925
int  iAseHome=0;                                                                //kevin 20150925
int  iAsePause=0;                                                               //kevin 20150925
AnsiString ASE_ReturnState;                                                     //kevin 20150925  回傳目前機台狀況
bool bRunManualSortMode=false;                                                  //Steven 20150915 : For TSMC 手動整盤功能
bool bResetModeAndCleanOut=false;                                               //JerryYang 20151007 : ResetMode可選OneCycle或CleanOut
bool bResetModeAndCleanOutAndNoTest=false;                                      //Jimmychiu 20210927 Reset clean out with no tester
int  iATCForHSMode=0;                                                           //Ifor 20151026 :判斷ATC 模式是否符合 HS 規定  0:未偵測ATC 版本 1:符合海思版本 2:不符合海思版本

bool bBarCodeError=false;                                                       //wei 20150713
int  iBarCodeErrorC=0;                                                          //wei 20150713
int  iBarCodeErrorR=0;                                                          //wei 20150713
AnsiString  asOCRBarCode="";                                                    //wei 20150713
int  iOCRCount=0;                                                               //wei 20150713
int iNewOCRCount[10]={0};
int iOCRPosition=0;
bool bOCRReadOK=false;                                                          //wei 20150713
bool bOCRNoIC=false;                                                            //wei 20150713
bool bOCRConnect=false;
bool bInspectError=false;
bool bOCRStartPosition=false;
int  iOCRConnectCount=0;
bool NeedWaitTrayArm=false;
bool bOCROffLine=false;
bool bOCROnLine=false;
bool bChangeFile=false;
bool bOCRUser=false;
bool bSignIn=false;
bool bOCRPassWord=false;
bool bOCRNewFile=false;                                                         //wei 20161028回傳OCR檔名
bool bOCROK[15]={false};
bool bOCRError[10]={false};
bool bCheckFile=false;
AnsiString asCheckFileName="";
AnsiString asOCRCheck[20]={""};
int iOCRCheck=0;
int iOCRLotTrayCount=0;
bool bOCRCheck=false;
bool bOCRCheckNG=false;
int iSH1_1BarcodePosition=0;                                                    //wei 20150912 2D
int iSH1_2BarcodePosition=0;                                                    //wei 20150912 2D
int iSH2_1BarcodePosition=0;                                                    //wei 20150912 2D
int iSH2_2BarcodePosition=0;                                                    //wei 20150912 2D

bool bOffsetEnterBarcode=false;
bool bHeightCalibrationFinish=true;                                             //JerryYang 20151028 add for Amkor,download recipe比對不符後,是否做完Height Calibration
double dTempOffset[2][tcTotalCount]={0};                                        //kevin 20151023 紀錄讀取溫度offset 0:最新資料
bool bSendChangeTemp=false;                                                     //kevin 20151023  生產中改變溫度
bool bSendChangeTempError=false;                                                //kevin 20151023  生產中改變溫度溫度超過 +- 10發alarm
int  iSendChangeTempError=0;
bool bSendChangeTempDelayAlarm=false;                                           //kevin 20151023  生產中改變溫度溫度發alarm時間延長
int iTempSec=30;                                                                //kevin 20151023  溫度發alarm時間
//DWORD StartTemptim,EndTemptim, PassTemptim;                                   //kevin 20151023  AutoTemp Offset time
int iYeildCT[TEST_MAX_BIN]={0};                                                 //wei 20151111
bool bLowYeildAlarm=false;                                                      //wei 20151116 Low Yield Onecycle中不alarm

int iRecordJamRateByTime_LoaderCount    = 0;                                    /// 2015.11.11 , Joye , Add Jam Rate Record
int iRecordJamRateByTime_JamCount       = 0;                                    /// 2015.11.11 , Joye , Add Jam Rate Record
bool bRecordJamRateByTime_Clear         = true;                                 /// 2015.11.11 , Joye , Add Jam Rate Record
bool bGPIBChangeSiteMap=false;                                                  ///wei 20151127 GPIB Change Site Map
bool bGPIBChangeAlarm=false;                                                    ///wei 20151127 GPIB Change Site Map
bool bSetByDLL=false;                                                           ///JerryYang 20181126 (Steven) : support Epson DLL function
bool bUnloading=false;                                                          ///JerryYang 20151109 用來判斷機台是否正在Unload
bool bAutoCleaning=false;                                                       ///JerryYang 20151109 用來判斷機台是否正在AutoClean
bool bScanKeyNo=false;                                                          ///JerryYang 20151202 Add for TSMC 點SKIP或是RETRY都要再跳一次確認
bool bHasSaveSet=false;                                                         ///Ifor 20151204 新增判斷機台有無修改設定檔
bool bOneTimes=true;
bool bEnableEmployeeIDCheck=false;                                              ///Ifor 20180911 (Steven) : Add 啟動 Employee ID Check

const int SortingBinTray_NOACT  = 0;                                            //JerryYang 20150910 START:Auto Sorting BinTray by Out Arm when Clean Out
const int SortingBinTray_ACT    = 1;
const int SortingBinTray_CHECKOK= 2;
const int SortingBinTray_ERR    = -1;

TSortingBinTray_PAR MyParameter;
TSortingBinTray_SUCKDATA MySuckData;
TSortingBinTray_TRAYDATA MyTrayData;
TSortingBinTray_CALDATA MyCalData;

TSortingBinTray_Data SortingBinTray_Data;
bool bSortingAllBinTrayFinish;                                                  //是否所有的Bin Tray IC都整盤完成
bool bSortingSuckMode=false;                                                    //Sam 20250415 : 修正 P27 整盤功能

int iOldRunStartMode=0;                                                         //wei 20151207
int iArmsitecount[2]={0};                                                       //wei 20160107 No FullSite Add Offset
int  iOverDriveDistance     =0;                                                 //Steven 20151207 : OverDrive for TSMC
bool bDoOverDrive           =false;                                             //Steven 20151207 : OverDrive for TSMC
int  iReContactCount        =1;                                                 //Steven 20151207 : Recontact for TSMC
int  iCurrentReContactCount =0;                                                 //Steven 20151207 : Recontact for TSMC
bool bDoReContact           =false;                                             //Steven 20151207 : Recontact for TSMC
//AI(W0-TAIL) 20260626: TODO(W7) -- clGreen/clRed/clPurple/... are VCL Graphics.hpp
//  colour constants (incl system clBtnFace). Keep symbol defined (zero-init) for W0;
//  faithful colour values restored with the VCL colour table in W7.
#if 0
TColor tcBinColor[eTrayCount]        ={clGreen, clRed, TColor(0x000080FF), clPurple, clBlue, clGray, clSilver, clBtnFace, clOlive};
#endif
TColor tcBinColor[eTrayCount]        ={TColor(0x00008000), TColor(0x000000FF), TColor(0x000080FF), TColor(0x00800080), TColor(0x00FF0000), TColor(0x00808080), TColor(0x00C0C0C0), TColor(0x8000000F), TColor(0x00008080)};  /* clGreen, clRed, 0x80FF, clPurple, clBlue, clGray, clSilver, clBtnFace, clOlive -- BCB6 Graphics.hpp values; the vclcompat colour constants are not visible in this TU */  //AI(W906-BINCOLOR) 20260926: golden colour table restored (the #if 0 line above, value for value); vclcompat has no clPurple / clGray / clOlive, so those three are the BCB6 Graphics.hpp values 0x800080 / 0x808080 / 0x008080 (clBtnFace = 0x8000000F, the system-colour reference, as in golden). Readers are display only (cShowBinSelect.cpp:1443-1446 / :1784 / :2010). St01 AUDIT_PROD S116 (2). Was {0,...}  //AI(W0-TAIL) 20260626: placeholder colours (W7)  //JerryYang 20220909 : 9->eTrayCount  //Steven 20160310 : 改成有顏色的fail bin
int  CHECK_RANGE=500;                                                           //2013-04-12    Dell 500->750 在KYEC舊機調不進去 //jou 2013-05-21 750 -> 500 出現Out Shuttle lose //Steven 20160108 : 改去Teaching調整
bool bForKyecBu3RunART=false;                                                   //wei 20151210 Loader使用不同Sensor   //wei 20161118 bRunART-->bForKyecBu3RunART
bool bARTTrayArmOK=false;                                                       //wei 20151217 ART 回盤OK
bool bRTLoaderTrayCount=false;                                                  //wei 20160112 ART R1不算空盤
bool bContsFailIgnoreStart=false;                                               //wei 20160115 銅鑼前幾顆不計算ContsFail
int  iContsFailIgnoreCount=0;                                                   //wei 20160115 銅鑼前幾顆不計算ContsFail

bool bESDSendDataToHandle=false;                                                //kevin 20160106  送ESD資料給HANDLE
bool bTempHandleToESD=false;                                                    //kevin 20160106   送Temp HANDLE資料給 客戶
bool bEPHandleToESD=false;                                                      //kevin 20160106  送EP HANDLE資料給 客戶
AnsiString bEPMessage;                                                          //kevin 20160106   存取EP資料
AnsiString bTempMessage;                                                        //kevin 20160106   存取溫度資料
bool bESDSystemtype=false;                                                      //kevin 20160106 Frank 20150309 : 程式關閉後將ESD系統停止
int iSiteTotal=16;                                                              //kevin 20160125 目前機台模式所使用SITE數量
int iRetestNoIC[3]={0};                                                         //wei 20160203 回盤為空盤不計數
int iATC70_Count=0;                                                             //Ifor 20160223 Add ATC 7.0 未收到溫度回傳次數
bool bRemoteAutomaticDeploying=false;                                           //wei 20160223 自動部屬
bool bIsShowPMAlarmMessage=false;                                               //wei 20160225 PMAlarmFunction
bool bAutoRetestJam=false;                                                      //wei 20160302 Jam Skip輸入顆數
AnsiString iJamSkipIC="";                                                       //wei 20160302 Jam Skip輸入顆數
int iARTJamSkipICCount=0;                                                       //Frank 20160819
bool bCleanSkipICCount=false;                                                   //Frank 20160819
int iJamSkipICCount=0;                                                          //wei 20160302 Jam Skip輸入顆數
bool bSecsGemStatus=false;                                                      //wei 20160308
bool bReadSecsGemData=false;                                                    //wei 20160308 是否讀取SECSGEM資料
bool bSysLotStart = false;                                                      //Ifor 20160302 KYEC
bool bEPLogStart_KYEC=false;                                                    //Ifor 20160302 KYEC FTP UP Load EP Log Start
bool bEPLogEnd_KYEC=false;                                                      //Ifor 20160302 KYEC FTP UP Load EP Log End
bool bTempLogStart_KYEC=false;                                                  //Ifor 20160302 KYEC FTP UP Load Temperature Log Start
bool bTempLogEnd_KYEC=false;                                                    //Ifor 20160302 KYEC FTP UP Load Temperature Log End
AnsiString sEPLogFile="";                                                       //Ifor 20160302 KYEC FTP UP Load EP Log File
AnsiString sTemperatureLogFile="";                                              //Ifor 20160302 KYEC FTP UP Load Temperature Log File
AnsiString sESDLogFile="";                                                      //Ifor 20160305 KYEC FTP UP Load ESD Log File
AnsiString sGroundManLogFile="";                                                //Ifor 20250123 KYEC FTP UP Load GroundMan Log File
bool bESDLogStart_KYEC=false;                                                   //Ifor 20160302 KYEC FTP UP Load ESD Log Start
bool bESDLogEnd_KYEC=false;                                                     //Ifor 20160302 KYEC FTP UP Load ESD Log End
bool bNoUseAutoRecord=false;                                                    //wei 20160311
int iGalil_Z_SafePos=200;                                                       //Ifor 20160311 避免Home sensor 不亮    //jou 2016-11-18 100->200 避免有些機台組裝公差造成alarm
const int iGalil_Z_KYEC_SafePos=100;                                            //Ifor 20170509 (wei) 新增KYEC回Home後最小上升高度100
AnsiString sESDLogData="";                                                      //Ifor 20160316 KYEC ESD Log Data
bool bESDDataReady=false;                                                       //Ifor 20160316 KYEC ESD Log Data
bool bATCWaterTempAlarm=false;                                                  //Ifor 20160625 ATC Water Temp Error
AnsiString asATCEvenLogFile="";                                                 //Ifor 20161202 KYEC FTP UP Load ATC Even Log File
bool bATCEvenLogEnd_KYEC=false;                                                 //Ifor 20161202 KYEC ATC Even Log End

int iIntervalYield_TotalIntervalCount = 0;                                      //20150604 Mylin Interval Total Yield Difference {
int iIntervalYield_IntervalCount = 0;
int iIntervalYield_StartCount = 0;
int iIntervalYield_StartPassCount = 0;
double fIntervalYield_YieldHistory[5];
bool bIntervalYield_IsShowAlarm = false;                                        //20150604 Mylin Interval Total Yield Difference }

int iAlarm4ContinueType_ContinueCount_Last = 0;                                 // 2015.05.05 , Joye , Alarm4 Continue Type
bool bAlarm4ContinueType_NeedClearCount = false;                                // 2015.05.05 , Joye , Alarm4 Continue Type
AnsiString asBarCodeErrorSend="";                                               //wei 20160318 Barcode Error依客戶設定
AnsiString asBarCodeErrorCheckSum="CheckSumError";                              //KaiChen 20191121 ：中壢日月光 2D Check Sum
AnsiString sATCVerRead="";                                                      //Ifor 20160321 add ATC Ver
int iNeedBarcodeCount[4];                                                       //wei 20160325 Barcode Rate計算
int iBarcodeErrorCount[4];                                                      //wei 20160325 Barcode Rate計算
int iBarcodePassCount[4];                                                       //wei 20160325 Barcode Rate計算
int iBarcodeAutoRetry[4];                                                       //wei 20160325 Barcode Rate計算
int iBarcodeDuplicate[4];
bool bBarcodeFirstAutoRetry[4]={false};                                         //wei 20160325 Barcode Rate計算

//bool iBinTray[5][9]={false};                                                  //kevin 20160308 有設定Bin Tray  //kevin 20170223 (wei) 不使用
bool iBinTray[eTrayCount]={false};                                              //JerryYang 20220909 : 10->eTrayCount        //kevin 20170328 (Steven) add 判斷TRAY 是否使用 BIN
bool bFirstTest=true;                                                           //kevin 20160311 Clean out後第一次測試時間
bool bTestOverTimeTempOffsetF=false;                                            //kevin 20160312 起動距離上一次測試時間超過所設定時間溫度需補OFFSET
bool bAlmLowYieldDoOneCycle=false;                                              //JerryYang 20160401 Low Yield Alarm後要做OneCycle
bool bAlmSiteYieldDiffDoOneCycle=false;                                         //JerryYang 20160401 Arm Site Yield Different Alarm 後要做OneCycle
bool bAlmSiteYieldCmpDoOneCycle=false;                                          //JerryYang 20160401 Site Yield Different Alarm 後要做OneCycle
AnsiString asErrPart="";                                                        //JerryYang 20160401 One cycle後的yield要能顯示error part
AnsiString asErrPart1="";                                                       //JerryYang 20160401 One cycle後的yield要能顯示error part
bool bAlmBinYieldDoOneCycle=false;                                              //JerryYang 20160407 Bin Yield Alarm 後要做OneCycle
AnsiString asErrTemp="";                                                        //JerryYang 20160407 Error message temp
bool bAlmContsFailBySocketDoOneCycle=false;                                     //JerryYang 20160407 ContsFailBySocket後要做OneCycle
bool bAlmConsecutiveFailureDoOneCycle=false;                                    //JerryYang 20160407 Consecutive Failure後要做OneCycle
int iErrArmTemp=0;                                                              //JerryYang 20160407 Error Arm Temp
bool bNeedOneCycleByYieldAlm=false;                                             //JerryYang 20160408 是否已觸發Yield 相關alarm
bool bYieldAlarmDoOneCycle=false;                                               //JerryYang 20160322 Yield Alarm後要做One cycle的旗標
bool bSpecailBinConsFailByArmDoOneCycle=false;                                  //JerryYang 20161006 SpecailBinConsFailByArm 後要做OneCycle
bool bSpecialBinConsFailBySocketDoOneCycle=false;                               //JerryYang 20161006 SpecialBinConsFailBySocket 後要做OneCycle
bool bBinCountOverLimitDoOneCycle=false;                                        //JerryYang 20161006 BinCountOverLimit 後要做OneCycle
AnsiString sAlarmMes="";                                                        //wei 20160407 Alarm Message
bool bOpenAllDoor=true;                                                         //wei 20160407 Alarm 後需要開門確認
bool iATCOnLine=false;                                                          //Ifor 20160331
bool bRunATC=false;                                                             //Ifor 20160331
//int  iUnloadMotorIndex[eTrayCount];
int  iTo6PosUnload[ePosTrayCount];
int  iTo3PosUnload[ePosTrayCount];

int  iTo6Unload[ePosTrayCount];
int  iTo3Unload[ePosTrayCount];

//AI(W906-FW-Fix1) 20260817: golden fills these four cross-index maps in the
// TfMain ctor (main.cpp:1850-1986) -- untranslated, which left all four
// arrays ALL-ZERO here while translated runtime code consumes them
// (aoutarm.cpp:3332 AOA fix path, csystem, cinitial, SortingBinTray):
// every station silently mapped to slot 0. The table is pure enum
// constants, so a same-TU static initializer is order-safe (zero-init of
// the arrays above precedes dynamic init within this TU) and semantically
// equivalent to golden's set-before-first-use. Lines transcribed
// mechanically from golden, not typed.
namespace {
struct W906_TrayIndexMapInit
{
    W906_TrayIndexMapInit()
    {
        iTo6Unload[e3Auto1  ]=eAuto1  ;
        iTo6Unload[e3Auto2  ]=eAuto2  ;
        iTo6Unload[e3Auto3  ]=eAuto3  ;
        iTo6Unload[e3Fix1   ]=eFix1   ;
        iTo6Unload[e3Fix2   ]=eFix2   ;
        iTo6Unload[e3Fix3   ]=eFix3   ;
        iTo6Unload[e3Fix4   ]=eFix4   ;
        iTo6Unload[e3Fix5   ]=eFix5   ;
        iTo6Unload[e3Fix6   ]=eFix6   ;
        iTo6Unload[e3BulkBox]=eBulkBox;
        iTo6Unload[e3Mag1   ]=eMag1   ;
        iTo6Unload[e3Mag2   ]=eMag2   ;
        iTo6Unload[e3Mag3   ]=eMag3   ;
        iTo6Unload[e3Mag4   ]=eMag4   ;
        iTo6Unload[e3Mag5   ]=eMag5   ;
        iTo6Unload[e3Mag6   ]=eMag6   ;
        iTo6Unload[e3Mag7   ]=eMag7   ;
        iTo6Unload[e3Mag8   ]=eMag8   ;
        iTo6Unload[e3Mag9   ]=eMag9   ;
        iTo6Unload[e3Mag10  ]=eMag10  ;
        iTo6Unload[e3Mag11  ]=eMag11  ;
        iTo6Unload[e3Mag12  ]=eMag12  ;
        iTo6Unload[e3Mag13  ]=eMag13  ;
        iTo6Unload[e3Mag14  ]=eMag14  ;
        iTo6Unload[e3Auto4  ]=eAuto4  ;
        iTo6Unload[e3Auto5  ]=eAuto5  ;
        iTo6Unload[e3Auto6  ]=eAuto6  ;
        iTo6Unload[e3Fix7   ]=eFix7   ;
        iTo6Unload[e3Fix8   ]=eFix8   ;
        iTo6Unload[e3Fix9   ]=eFix9   ;
        iTo6Unload[e3Fix10  ]=eFix10  ;
        iTo6Unload[e3Fix11  ]=eFix11  ;
        iTo6Unload[e3Fix12  ]=eFix12  ;
        iTo6PosUnload[e3PosNoUse]=ePosNoUse;                                        //JerryYang 20230916 : bin顯示
        iTo6PosUnload[e3PosAuto1]=ePosAuto1;
        iTo6PosUnload[e3PosAuto2]=ePosAuto2;
        iTo6PosUnload[e3PosAuto3]=ePosAuto3;
        iTo6PosUnload[e3PosFix1 ]=ePosFix1 ;
        iTo6PosUnload[e3PosFix2 ]=ePosFix2 ;
        iTo6PosUnload[e3PosFix3 ]=ePosFix3 ;
        iTo6PosUnload[e3PosFix4 ]=ePosFix4 ;
        iTo6PosUnload[e3PosFix5 ]=ePosFix5 ;
        iTo6PosUnload[e3PosFix6 ]=ePosFix6 ;
        iTo6PosUnload[e3Bulkbox ]=eBulkbox ;
        iTo6PosUnload[e3PosMag1 ]=ePosMag1 ;
        iTo6PosUnload[e3PosMag2 ]=ePosMag2 ;
        iTo6PosUnload[e3PosMag3 ]=ePosMag3 ;
        iTo6PosUnload[e3PosMag4 ]=ePosMag4 ;
        iTo6PosUnload[e3PosMag5 ]=ePosMag5 ;
        iTo6PosUnload[e3PosMag6 ]=ePosMag6 ;
        iTo6PosUnload[e3PosMag7 ]=ePosMag7 ;
        iTo6PosUnload[e3PosMag8 ]=ePosMag8 ;
        iTo6PosUnload[e3PosMag9 ]=ePosMag9 ;
        iTo6PosUnload[e3PosMag10]=ePosMag10;
        iTo6PosUnload[e3PosMag11]=ePosMag11;
        iTo6PosUnload[e3PosMag12]=ePosMag12;
        iTo6PosUnload[e3PosMag13]=ePosMag13;
        iTo6PosUnload[e3PosMag14]=ePosMag14;
        iTo6PosUnload[e3PosAuto4]=ePosAuto4;
        iTo6PosUnload[e3PosAuto5]=ePosAuto5;
        iTo6PosUnload[e3PosAuto6]=ePosAuto6;
        iTo6PosUnload[e3PosFix7 ]=ePosFix7 ;
        iTo6PosUnload[e3PosFix8 ]=ePosFix8 ;
        iTo6PosUnload[e3PosFix9 ]=ePosFix9 ;
        iTo6PosUnload[e3PosFix10]=ePosFix10;
        iTo6PosUnload[e3PosFix11]=ePosFix11;
        iTo6PosUnload[e3PosFix12]=ePosFix12;
        iTo3PosUnload[ePosNoUse]=e3PosNoUse;
        iTo3PosUnload[ePosAuto1]=e3PosAuto1;
        iTo3PosUnload[ePosAuto2]=e3PosAuto2;
        iTo3PosUnload[ePosAuto3]=e3PosAuto3;
        iTo3PosUnload[ePosAuto4]=e3PosAuto4;
        iTo3PosUnload[ePosAuto5]=e3PosAuto5;
        iTo3PosUnload[ePosAuto6]=e3PosAuto6;
        iTo3PosUnload[ePosFix1 ]=e3PosFix1 ;
        iTo3PosUnload[ePosFix2 ]=e3PosFix2 ;
        iTo3PosUnload[ePosFix3 ]=e3PosFix3 ;
        iTo3PosUnload[ePosFix4 ]=e3PosFix4 ;
        iTo3PosUnload[ePosFix5 ]=e3PosFix5 ;
        iTo3PosUnload[ePosFix6 ]=e3PosFix6 ;
        iTo3PosUnload[ePosFix7 ]=e3PosFix7 ;
        iTo3PosUnload[ePosFix8 ]=e3PosFix8 ;
        iTo3PosUnload[ePosFix9 ]=e3PosFix9 ;
        iTo3PosUnload[ePosFix10]=e3PosFix10;
        iTo3PosUnload[ePosFix11]=e3PosFix11;
        iTo3PosUnload[ePosFix12]=e3PosFix12;
        iTo3PosUnload[eBulkbox ]=e3Bulkbox ;
        iTo3PosUnload[ePosMag1 ]=e3PosMag1 ;
        iTo3PosUnload[ePosMag2 ]=e3PosMag2 ;
        iTo3PosUnload[ePosMag3 ]=e3PosMag3 ;
        iTo3PosUnload[ePosMag4 ]=e3PosMag4 ;
        iTo3PosUnload[ePosMag5 ]=e3PosMag5 ;
        iTo3PosUnload[ePosMag6 ]=e3PosMag6 ;
        iTo3PosUnload[ePosMag7 ]=e3PosMag7 ;
        iTo3PosUnload[ePosMag8 ]=e3PosMag8 ;
        iTo3PosUnload[ePosMag9 ]=e3PosMag9 ;
        iTo3PosUnload[ePosMag10]=e3PosMag10;
        iTo3PosUnload[ePosMag11]=e3PosMag11;
        iTo3PosUnload[ePosMag12]=e3PosMag12;
        iTo3PosUnload[ePosMag13]=e3PosMag13;
        iTo3PosUnload[ePosMag14]=e3PosMag14;
        iTo3Unload[eAuto1  ]= e3Auto1   ;
        iTo3Unload[eAuto2  ]= e3Auto2   ;
        iTo3Unload[eAuto3  ]= e3Auto3   ;
        iTo3Unload[eAuto4  ]= e3Auto4   ;
        iTo3Unload[eAuto5  ]= e3Auto5   ;
        iTo3Unload[eAuto6  ]= e3Auto6   ;
        iTo3Unload[eFix1   ]= e3Fix1    ;
        iTo3Unload[eFix2   ]= e3Fix2    ;
        iTo3Unload[eFix3   ]= e3Fix3    ;
        iTo3Unload[eFix4   ]= e3Fix4    ;
        iTo3Unload[eFix5   ]= e3Fix5    ;
        iTo3Unload[eFix6   ]= e3Fix6    ;
        iTo3Unload[eFix7   ]= e3Fix7    ;
        iTo3Unload[eFix8   ]= e3Fix8    ;
        iTo3Unload[eFix9   ]= e3Fix9    ;
        iTo3Unload[eFix10  ]= e3Fix10   ;
        iTo3Unload[eFix11  ]= e3Fix11   ;
        iTo3Unload[eFix12  ]= e3Fix12   ;
        iTo3Unload[eBulkBox]= e3BulkBox ;
        iTo3Unload[eMag1   ]= e3Mag1    ;
        iTo3Unload[eMag2   ]= e3Mag2    ;
        iTo3Unload[eMag3   ]= e3Mag3    ;
        iTo3Unload[eMag4   ]= e3Mag4    ;
        iTo3Unload[eMag5   ]= e3Mag5    ;
        iTo3Unload[eMag6   ]= e3Mag6    ;
        iTo3Unload[eMag7   ]= e3Mag7    ;
        iTo3Unload[eMag8   ]= e3Mag8    ;
        iTo3Unload[eMag9   ]= e3Mag9    ;
        iTo3Unload[eMag10  ]= e3Mag10   ;
        iTo3Unload[eMag11  ]= e3Mag11   ;
        iTo3Unload[eMag12  ]= e3Mag12   ;
        iTo3Unload[eMag13  ]= e3Mag13   ;
        iTo3Unload[eMag14  ]= e3Mag14   ;
        // AI(W906-FW-Fix2) 20260818: the fifth map from the same golden ctor
        // block -- iSortTrayIndex reversal (golden main.cpp:1989-1992,
        // JerryYang 20230820). Same all-zero defect class as the four maps
        // above: translated aoutarm9045.cpp:2683-2802 walks this table to
        // pick the sort target, and an all-zero table makes every probe read
        // slot 0 (Auto1).
        for(int i=0; i<eTrayCount; i++)
        {
            iSortTrayIndex[i]=eTrayCount-i-1;
        }
    }
};
W906_TrayIndexMapInit g_w906TrayIndexMapInit;
} // namespace
int  iSortTrayIndex[eTrayCount];
int  AddBinDisp[MAX_BIN_UNIT];
bool bSetTempChange=false;                                                      //Ifor 20160331 ATC啟動狀態

int  iL03SocketAirCoolingCT=0;                                                  //jou 2016-04-28 Socket Air Cooling contact count trun on
bool bCheckCodeError[4]={false};                                                //wei 20160505 Barcode 比對Lot
bool bCheckLotError[4]={false};                                                 //wei 20160728
int iIONFANStatus_Secs[12]={-1};                                                //wei 20160506 SECS ION 偵測
int iIONFANPower_Secs[12]={-1};                                                 //wei 20160506 SECS ION 偵測
bool bATC_EnablesChannel[32];                                                   //Ifor 20160506 add Handler 送給ATC 開關Site資料
int  iATC_Use_Heat_Count=0;                                                     //Ifor 20160506 add ATC Use Heat Count
bool bATC_SITE_2ND_CHECK[2]={false,false};                                      //Ifor 20160509 add ATC 第二點溫度 sensor 判斷
bool bOutShuttleMissIC=false;                                                   //wei 20160509 Out Shuttle Miss IC
bool bNoSendSiteOnOff=false;                                                    //wei 20160511 Send Site On Off
bool bDownloadFTP=false;                                                        //wei 20160512 避免Download失敗，下次就無法Download
int iOpenBin=0;                                                                 //kevin 20160513 AutoSite map
int iBinLast=0;                                                                 //kevin 20160513 AutoSite map
int iOpenBinCount=0;                                                            //kevin 20160513 AutoSite map
int iBackupDutOnOff[4][8];                                                      //JerryYang 20160519 備份開關site
int iBackupTestMode;                                                            //JerryYang 20160519 記錄原本的Test mode
bool bLoadMachineRecord=false;                                                  //JerryYang 20160614 用來判斷是否執行過LoadMachineRecord函式 避免還沒讀取到機台資料Lastset就被改掉
bool INSTALL_SOCKET_CLAMP=false;                                                //JerryYang 20160523 是否改機為使用SLK、SOCKET clamp
bool HotGunFlowEnable=false;                                                    //KaiChen 20190729 ：Hot Gun Flow
unsigned int HotGunFlow_LineNo=0;                                               //KaiChen 20190729 ：Hot Gun Flow
unsigned int HotGunFlow_DevNo=0;                                                //KaiChen 20190729 ：Hot Gun Flow
unsigned int HotGunFlow_Gun1_ChannelNo=0;                                       //KaiChen 20190729 ：Hot Gun Flow
unsigned int HotGunFlow_Gun2_ChannelNo=0;                                       //KaiChen 20190729 ：Hot Gun Flow

int INSTALL_DOUBLE_EP=0;                                                        //Ifor 20190104 : Add Double EP Control
int CHECK_EP_SETTING=1;                                                         //Steven 20240701 : EP檢查功能加上開關
bool bAutoTrackCanGoRear=false;                                                 //Isaac 20180109 (Steven) : auto123可前進後退
bool bAvoidAddDoSiteMappingStep=false;                                          //JerryYang 20170316 (Steven) 避免tester time out時Retry會重複進入DoJCETSiteMappingCHK(), 造成auto site mapping一次跳兩顆
bool bOCRConnectOK=false;                                                       //wei 20160613 ocr連線測試
bool bOCRConnectTest=false;                                                     //wei 20160613 ocr連線測試
int bOCRConnectTestCount=0;                                                     //wei 20160613 ocr連線測試
bool bCleanOutClosedMonitorVideo=false;                                         //JerryYang 20160621 UseMonitorVideoFunction
bool bHPCleanout=false;                                                         //wei 20160624 Hotplate clean out
bool bAmbientHotPlate=false;                                                    //kevincheng 20260525 : 渠梁常溫模式使用hotplate
int iHPShuttle=0;                                                               //wei 20160624 Hotplate clean out
bool bGPIBLotEndCommand=false;                                                  //wei 20160624 GPIB Lot End Command
bool bASECleanOutCloseSite=false;                                               //kevin 20160715 ASE clean out close Site 不退tray
bool bCancelErrorBin=false;                                                     //kevin 20160724 Error Bin 取消
bool bGPIBLotStartCommand=true;                                                 //kevin 20190613 add lotstart GPIB Command lotstart

int  iHWFix_BinBox=0;                                                           //kevin 20160819 error bin 要放到 Bin Box
int  iErrorBinBoxAlarm=0;                                                       //kevin 20160819 error bin alarm 數量

bool bBinError[2]={false};                                                      //kevin 20160725 Error Bin 發生arm
AnsiString sArm1BinError="";                                                    //kevin 20160725 記錄 Error Bin
AnsiString sArm2BinError="";                                                    //kevin 20160725 記錄 Error Bin
bool bOpenSixDoorChk=false;                                                     //kevin 20160725 error bin 發生需開右後門確認
bool bFirstOpenPg=true;                                                         //kevin 20160725 程式一開始進來不往下執行
int iATC_TempIndex[4][2];                                                       //Ifor 20160719 第一點溫度索引  0:Index舊值 1:Index 差異次數 //Ifor 20160805 add Use Site
bool bNeedSendATCRunSelfTest=false;                                             //Ifor 20160720 add for ATC Safe Test start
bool bNeedWaitATCRunSelfTestFinish=false;                                       //Ifor 20160720 add Wait ATC slef Test Report
bool bATCHasAlarmBinNeedToError=false;                                          //Ifor 20160726 ATC Alarm Index IC Be Error
int iATCHasAlarmBinNeedToError=0;                                               //Ifor 20181219 :add KYEC 要求ATC 溫度異常時Device放置Out SH後才Alarm 0:未發生ATC溫度Alarm 1:ATC溫度Alarm 2:Index 上升完畢 3:Device 放置Out SH 上方並回到安全位置
bool bRunAOI=false;                                                             //wei 20160617 Vitrox
bool bAOIPassFail[2][MAX_ARM_Row][MAX_ARM_Col]={false};                         //wei 20160617 Vitrox
int iAOIBin[MAX_ARM_Row][MAX_ARM_Col]={0};                                      //wei 20160617 Vitrox
int bAOIBin[MAX_ARM_Row][MAX_ARM_Col]={false};                                  //wei 20160617 Vitrox
bool bBarcodeConnect=false;                                                     //wei 20160728
bool bChangeBarcodeFileOK=false;                                                //wei 20160729
bool bCatchTrayFinishAction=false;                                              //Frank 20160711 add
AnsiString asOCRImageName="";                                                   //wei 20160805
bool bOCRSkip=false;                                                            //wei 20160805
bool bAutoRetestMusic=false;                                                    //Frank 20160822 add AutoRetestMusic
int iBarcodeReject=0;                                                           //wei 20160823  Consecutive Failure
bool bBarcodePassword=false;                                                    //wei 20160823  Lot check 錯誤需輸入密碼
bool bFixBinBoxIsFull=false;                                                    //kevin 20160822 bulk box   pig 2013.04.08 FixBinBox
AnsiString asVer;                                                               //kevin 20160825
bool bSetupFileNotExist=false;                                                  //Ifor 20160822 add 工作擋不存在
bool bHasEnteredPEModel=false;                                                  //Ifor 20160822 add 進入工程模式
bool bEnablePEModel=false;                                                      //Ifor 20160822 add 啟動PE工程模式
bool bInitialATCSelfTest=true;                                                  //Ifor 20160829 add 機台Initial Start 需做 ATC Self Test
bool bStartATCRun=true;                                                         //Ifor 20160830 add Lot頁面啟動ATC 預設啟動
bool bSpecificBin[256]={false};                                                 //kevin 20170828 (wei) 33->256  JerryYang 20170221 (wei) 改成可複選bin  //JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm
AnsiString asBarcodeLotNumber="";                                               //wei 20160921 BarcodeLotNumber
bool bBarcodeAutoSkip[4]={false};                                               //wei 20160921 BarcodeLotAutoSkip
bool bBarcodeAutoSkipError[4]={false};                                          //wei 20160921 BarcodeLotAutoSkip
bool bSiteUseEE=false;                                                          //Alick 20160923 add for 第三組工程師用開關SITE
bool bNeedKeyInSkipIC=false;                                                    //Frank 20161006 add
bool bInitialCleanCount=false;                                                  //wei 20160923 Tray Feed後，下一次開始需要Clean Count
int iSECSGEMPass=0;                                                             //wei 20160923 Secs Gem 回傳Pass/Fail顆數
int iSECSGEMPass_ART=0;                                                         //wei 20160923 Secs Gem 回傳Pass/Fail顆數
int iSECSGEMFail=0;                                                             //wei 20160923 Secs Gem 回傳Pass/Fail顆數
int iSECSGEMFail_ART=0;                                                         //wei 20160923 Secs Gem 回傳Pass/Fail顆數
bool bPickErrorAutoHome=false;                                                  //wei 20161005
bool bCCDcheckShuttle[2]={false};                                               //wei 20161102 IC置偏檢查
bool bClearRetryCnt[iTotalFunction]={false};                                    //Sam 20200323 : Modify Tray Function //wei 20161219 Tray Mapping
bool bCCDTrayDeviceCount[3]={false};                                            //Sam 20190405 : Tray Decive Count
bool bNeedCCDTrayDeviceCount[3]={false};                                        //Sam 20190405 : Tray Decive Count
bool bCCDTrayID[2]={false};                                                     //wei 20180808 MR Tray ID 2
int iRecordSiteMapOrder[4][8]={0};                                              //kevin 20161003 Site 順序
int iCloseSiteMap[2][4][8]={0};                                                 //kevin 20161003 Site 順序
AnsiString asHandlerVersion="";                                                 //Ifor 20161109 Handler S/W Ver 顯示
AnsiString asMainVersion="";                                                    //JerryYang 20180619 (wei) : ASE_CL 尚智要求主畫面版號可以自己修改
AnsiString MainVersion="";                                                      //Steven 20250903 : 改成直接抓檔案版本
bool bAutoSiteMapWaitTestResult=false;                                          //jou 2016-11-03 JCET 要求Auto Site Mapping 需等待測試結果在繼續入料
bool bAutoSiteMapWaitTestPass=false;                                            //jou 2016-11-03 JCET 要求Auto Site Mapping 需等待測試結果在繼續入料
bool bAutoSiteMapAmbientResultCheck=false;                                      //Ifor 20190528 : add Site Mapping Ambient Check
int iOldSelShuttle=0;                                                           //Ifor 20161116 add BackUp Shuttle select
int iOldShuttleMode=0;                                                          //Ifor 20161116 add BackUp Shuttle Mode
int iUseSuckModeBackup=0;                                                       //Ifor 20161116 add BackUp Use Suck Mode
AnsiString asHandlingMode=0;                                                    //Ifor 20161117 add BackUp Handling Mode
AnsiString asOCRSaveName="";                                                    //wei 20161122 OCR 存圖檔名加入輸入值
int iOCRMoveSRead=0;                                                            //wei 20161118 OCR S型讀取
int iInArmAutoYTeachOffset=0;                                                   //kevin 20161124 auto teach Z Yoffset
int iOutArmAutoYTeachOffset=0;                                                  //kevin 20161124 auto teach Z Yoffset
int iInArmTeachZ =0;                                                            //kevin 20170502 (wei) auto teach Z Z check pos
int ioutArmTeachZ=0;                                                            //kevin 20170502 (wei) auto teach Z Z check pos
AnsiString asOCRSaveType="";                                                    //Alick 20170119 add OCR 存檔時多加入存檔原因
bool bSendATCLotStart=false;                                                    //Ifor 20161118 add Send ATC Lot Start
bool bSendATCLotEnd=false;                                                      //Ifor 20161118 add Send ATC Lot End
bool bReSendATCLotEven=false;                                                   //Ifor 20161118 add Resent ATC Lot Command
//Sorting Tray define Start
const int iSortingTrayAuto1 = 0;                                                //Ifor 20161123 add Sorting Tray Auto1
const int iSortingTrayAuto2 = 1;                                                //Ifor 20161123 add Sorting Tray Auto2
const int iSortingTrayAuto3 = 2;                                                //Ifor 20161123 add Sorting Tray Auto3
const int iSortingTrayFix1  = 3;                                                //Ifor 20161123 add Sorting Tray Fix1
const int iSortingTrayFix2  = 4;                                                //Ifor 20161123 add Sorting Tray Fix2
const int iSortingTrayFix3  = 5;                                                //Ifor 20161123 add Sorting Tray Fix3
//Sorting Tray define end
bool bFix3HasNullICSet=true;                                                    //Ifor 20161124 add Fix3 入料時設定HasNullIC
bool bAutoShuttleHome=false;                                                    //wei 20161206 Auto Shuttle Sensor 回Home
bool bAutoCleanFinishOnlyUseRTC=false;                                          //JerryYang 20161216 (jou) 做完auto clean後不需要做index check
bool bCanRunSCKART=false;                                                       //Steven 20170919 (wei) : Add for SCK ART
int  iSCKARTLoadingStatus=-1;                                                   //Steven 20161223 (jou) : For SCK ART loading count
bool bSCKARTLoadCntAlarm=false;                                                 //Steven 20161223 (jou) : For SCK ART loading count : 數量超過,要放下IC然後Alarm
int  iLoadPickX=0;                                                              //Steven 20161223 (jou) : For SCK ART loading count : 目前吸到的位置X
int  iLoadPickY=0;                                                              //Steven 20161223 (jou) : For SCK ART loading count : 目前吸到的位置Y
int iD43AutoRetryWhenIndexPickErrCnt[2]={0, 0};                                 //Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次
double dOldWorkTemp=0.0;                                                        //Ifor 20161220 (Steven) KYEC 要求Even Log 紀錄上一筆Temp設定
double dOldAmbWorkTemp=0.0;                                                     //Ifor 20161220 (Steven) KYEC 要求Even Log 紀錄上一筆AmbTemp設定
double dOldSockTime=0.0;                                                        //Ifor 20161220 (Steven) KYEC 要求Even Log 紀錄上一筆Sock設定
bool bFTPDownLoadHasTestMode=false;                                             //Ifor 20161221 (Steven) add KYEC Setup File Down Load是否有TestMode.Data
bool bUseATC_SelfTestFunction=false;                                            //Ifor 20170124 : add KYEC 要求ATC Self Test 可由General.ini 中開啟或關閉
AnsiString asATCEvenLotID="";                                                   //Ifor 20170124 (Steven) : add LotID By ATC Even Log
double iInitStartDelayDec=0;                                                    //kevin 20161214 每次遞減時間
int iInitStartDelayTimeDetCount=0;                                              //kevin 20161214 每次遞減計數
AnsiString Ase_SendESDTempEPData="";                                            //kevin 20161229 傳送資料給ASE
AnsiString Ase_SendESD="";                                                      //kevin 20161229 傳送資料給ASE
AnsiString Ase_SendTemp="";                                                     //kevin 20161229 傳送資料給ASE
AnsiString Ase_SendEP="";                                                       //kevin 20161229 傳送資料給ASE
bool bIndexCheck1=false,bIndexCheck2=false;                                     //kevin 20170120 autoclean index check 發ALARM
bool bOutArmPickShtHasRetry=false;                                              //Steven 20170202 (wei): For ATJ Out Arm pickup error put to define tray
bool bSortArmPickShtHasRetry=false;                                             //RogerYang 20250516 Add for 9046AU
bool bAlarmUnlockPassWord=false;                                                //Ifor 20170214 (wei) add 解除Alarm 需要獨立密碼
AnsiString asUnlockPassword="";                                                 //Ifor 20170214 (wei) add 解除Alarm 獨立密碼
bool bHT9045S_USE2x4=false;                                                     //Ifor 20170308 (wei) add HT9045S 2x4模式鎖定
AnsiString dSend_ASEData[10]={0};                                               //kevin 20170210 (wei) 送ASE  0:Index Time(Arm),1:Index Cycle Time,2:Test Time,3:UPH
bool bManualAutoClean=false;                                                    //kevin 20170215 (wei) 手動 autoclean
bool bASkStart=false;                                                           //kevin 20170224 (wei) Load Auto Skip 開始有吸取到空格
bool bAutoSkipHasIC=false;                                                      //kevin 20170224 (wei) LOAD AUTO SKIP HAS IC 需要 ALARM
AnsiString sAskStartDetect="";                                                  //kevin 20170224 (wei) LOAD AUTO SKIP HAS IC data
bool bSiteHasTurnOn[33];                                                        //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
bool bLoaderTrayAction=false;                                                   //wei 20161219 Tray Mapping
bool bSecsGemDownloadFTP=false;                                                 //wei 20170119 (Steven) DownLoad 沒有馬上按掉會Time Out
int iATRFtRtMode=0;                                                             //wei 20170119 (jou) ATR FT/RT count
int iATRPassCount[2]={0};                                                       //wei 20170119 (jou) ATR FT/RT count
int iATRFailCount[2]={0};                                                       //wei 20170119 (jou) ATR FT/RT count
int iATRTotalCount[2]={0};                                                      //wei 20170119 (jou) ATR FT/RT count
bool bTrayMapDataError[MAX_ARM_Row][MAX_ARM_Col]={false};                       //wei 20170302 (Steven) Tray map data error
int iTrayMapDataX[MAX_ARM_Row][MAX_ARM_Col]={0};                                //wei 20170302 (Steven) Tray map data error
int iTrayMapDataY[MAX_ARM_Row][MAX_ARM_Col]={0};                                //wei 20170302 (Steven) Tray map data error
bool bTrayMapDataCheckErrorSkip=false;                                          //wei 20170302 (Steven) Tray map data error
bool bTrayMapDataCheckError=false;                                              //wei 20170302 (Steven) Tray map data error
bool bCheckTrayDevice=false;                                                    //wei 20170302 (Steven) Tray map data error
bool bRealCCDROICheck=true;                                                     //wei 20170308 (jou) RTC ROI Check
bool bRealCCDROICheckOK=false;                                                  //wei 20170308 (jou) RTC ROI Check
bool bRealCCDROICountCheck=true;                                                //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
bool bRealCCDROICountCheckOK=false;                                             //jou 20171201 (Steven) : RTC ROI 確認數量是否正確
int iAse_LoadSkipTime=0;                                                        //kevin 20170314 (Steven) Load 開始SKIP 到吸到有IC時間記錄
DWORD AutoTrayendStartTime[2]={0}, AutoTrayendEndTime[2]={0}, AutoTrayendPassTime[2]={0};   //kevin 20170314 (Steven) 0:有吸到IC時間1:總使用時間 記錄AUTO TRAY END 花了多少時間
int iAutoTrayendhasIC=0;                                                        //kevin 20170314 (Steven) 計算 AUTO SKIP後又吸到IC數量
int iAse_LoadTrayEndTotalTime=0;                                                //kevin 20170320 (Steven) LOAD Auto skip 記算總時間
int iAutoTrayEndTotal=0;
bool bInSht1LtcDetectTesterCanMove=true;                                        //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
bool bInSht2LtcDetectTesterCanMove=true;                                        //Isaac 20170418 (Steven) 用Y Latch檢測Inshuttle有無IC
bool bReadAndCheckCPUName=false;                                                //Isaac 20170509 (Steven) 卡CPU資訊
AnsiString EPuser="HonPrec",EPPass="16943420";                                  //kevin 20170327 ep 加密碼保護
bool bHingeStart=false;                                                         //wei 20170413
bool bHingeNeedStart=false;                                                     //wei 20170413
int iReadAdamEP=0;                                                              //jou 20170413 (Steven) : Read Adam EP 提升UPH
bool bRTCModelNG=false;                                                         //wei 20170504 (Steven) RTC Model NG
bool bFirstInput=false;                                                         //Steven 20170511 (wei) : 使用initial delay當 Soak time
bool bFirstInputForIndex=false;                                                 //Steven 20170511 (wei) : 使用initial delay當 Soak time
bool bOneTimeWait=true;
bool bCheckPCI_MN200StateRun=false;                                             //JerryYang 20170510 (Steven) 改成全域變數
bool bF18CheckShuttle1MustHasIC=false;                                          //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
bool bF18CheckShuttle2MustHasIC=false;                                          //JerryYang 20170515 (Steven) 修正in shuttle detect異常後關閉F18會hang up
bool bReadEpTime=false;                                                         //kevin 20170524 (wei) add ep read change time
bool bNeedClearSortCount=false;                                                 //JerryYang 20170517 (wei) JCET吳如春要求tray end或tray feed後按Start要自動清除unloader數量
double dFixTrayDataCleanTime=0.0;                                               //Ifor 20170525 (wei) add Fix Tray Data Clean Time
bool bCutDirectTemp=false;                                                      //kevin 20170529 (wei) add power off關 Direch Heat
int iIndexZ1HomeToZPhaseDistanceFirst=0;                                        //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
int iIndexZ1HomeToZPhaseDistanceSecond=0;                                       //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
int iIndexZ2HomeToZPhaseDistanceFirst=0;                                        //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
int iIndexZ2HomeToZPhaseDistanceSecond=0;                                       //kevin 20170515 (wei) add Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range
int iIndexZ1HomeToZPosFirst=0;                                                  //kevin 20170605 (wei) read pos
int iIndexZ1HomeToZPosSecond=0;                                                 //kevin 20170605 (wei) read pos
int iIndexZ2HomeToZPosFirst=0;                                                  //kevin 20170605 (wei) read pos
int iIndexZ2HomeToZPosSecond=0;                                                 //kevin 20170605 (wei) read pos

int iIndexY1HomeToYPhaseDistanceFirst=0;                                        //Isaac 20201110 : Index Y find motor phase
int iIndexY1HomeToYPhaseDistanceSecond=0;
int iIndexY2HomeToYPhaseDistanceFirst=0;
int iIndexY2HomeToYPhaseDistanceSecond=0;
int iIndexY1HomeToYPosFirst=0;
int iIndexY1HomeToYPosSecond=0;
int iIndexY2HomeToYPosFirst=0;
int iIndexY2HomeToYPosSecond=0;

int MotorTask=FIRST_HOME;                                                       //kevin 20170605 (wei) define
bool bSECSGEM_NoteAlarm=false;                                                  //Ifor 20170616 (wei) add S10F3 後跳出Note Alarm旗標
bool bSECSOneCycleComm=false;                                                   //RogerYang 20170705 (wei) add SECS/GEN OneCycle命令旗標
AnsiString aARMSPassWordPath="";                                                //Ifor 20170621 (wei) add ARMS Function
bool bKitOutDiameter=false;                                                     //kevin 20170628 (Steven) Lay out Kit  外徑
AnsiString sKitOutDiameter[10]={""};                                            //kevin 20210813 change  //kevin 20170628 (Steven) add
int iIndexSpeed;                                                                //Steven 20170818 : 測試UPH用
int iIndexAcc;
int  iMonitoringOutlierCnt=0;                                                   //Steven 20180626 (wei) : 超過設定值的也要檢查
bool bInArmPickErrFromLoader=false;                                             //Steven 20170828 (wei) : 確認是否有在Loader吸料異常
bool bQAModeFlag=false;                                                         //Steven 20170830 (wei) : QA mode for ATK ART
bool bHasQwertyKeyForm=false;                                                   //Ifor 20170816 (Steven)add QWERTY小鍵盤旗標
bool bHasPasswordForm=false;                                                    //Ifor 20170816 (Steven)add QWERTY小鍵盤旗標
bool bAlarmAfterPreAlarm=false;                                                 //Ifor 20170906 add 避免 PreAlarm -> Alarm -> SECS GEM Alarm 同時發生造成當機問題
bool bUseNSKitKey=false;                                                        //kevin 20170814 (Steven) 低FORCE 需軟體鎖住
bool bNeedMusicAndAlarmOn=false;                                                //kevin 20170816 (Steven) add 需發出音樂及秀燈
bool bNeedMusicFinishLighAndAlarmOn=false;                                      //kevin 20170816 (Steven) add 需發出音樂及秀燈 完成
AnsiString ASE_Yield[4]={"","","",""};                                          //kevin 20170816 (Steven) add 傳送YIELD 給ASE
bool bAutoTrayEndHasIC=false;                                                   //kevin 20170822 (Steven) auto tray end 有吸到IC
AnsiString sAutoTrayendStartPosBuffer="";                                       //kevin 20170823 (Steven) auto tray start buffer 記錄一盤ＸＹ位置
AnsiString sAutoTrayendStartPos="";                                             //kevin 20170823 (Steven) auto tray start 記錄一盤ＸＹ位置
AnsiString sAutoTrayendEndPosBuffer="";                                         //kevin 20170823 (Steven) auto tray end buffer記錄一盤ＸＹ位置
AnsiString sAutoTrayendEndPos="";                                               //kevin 20170823 (Steven) auto tray end 記錄一盤ＸＹ位置
AnsiString sAutoTrayendabnormalBuffer="";                                       //kevin 20170823 (Steven) auto tray end buffer中途有料記錄一盤ＸＹ位置
AnsiString sAutoTrayendabnormalPos="";                                          //kevin 20170823 (Steven) auto tray end 中途有料 記錄一盤ＸＹ位置
bool bASEARTStart=false;                                                        //kevin 20170830 (Steven) ASE_KH ART SEND START
int iArt_AutoTotal[MAX_AUTO_TRAY][2]={{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};     //kevin 20170830 (Steven) 記錄ART 數量
bool bAutoSkipStartXYlog=false;                                                 //kevin 20170904 (Steven) 記錄SKIP TRAY X Y 位置
bool bAutoSkiplog=false;                                                        //kevin 20170904 (Steven) 記錄SKIP TRAY X Y 位置
bool bTrayHaveDevice=false;                                                     //wei 20170317 (steven) Device Remain 殘料檢測
bool bDoTrayDeviceCheck=false;                                                  //wei 20170317 (steven) Device Remain 殘料檢測
bool bFTPDownLoadFilediff=false;                                                //wei 20170518
bool bFTPDownLoadFileError=false;                                               //wei 20170518
int iYeildCantactCT[3]={0};                                                     //wei 20170602
int iMapErrorCount=0;
AnsiString asTrayIDFile;
bool bNoFileNoStart=false;
bool bDownOffsetFTP=false;                                                      //wei 20170821 offset ftp
bool bAutoCloseSite=false;
int iContactCountAlarm=0;                                                       //wei 20170327 add
bool bAutoCleanOut=false;
bool bRecordData[10][5]={false};                                                //wei 20171012 比對資料紀錄
bool bTCPIPChangeAlarm=false;                                                   //Isaac 20170613 (wei) TCP/IP
AnsiString InitialStartTime;                                                    //Isaac 20170613 (wei) TCP/IP
int iHandlerCommandServerPort;                                                  //Isaac 20170613 (wei) TCP/IP
int iHandlerResultServerPort;                                                   //Isaac 20170613 (wei) TCP/IP
bool bNoChangeContactHeight=false;                                              //Frank 20170918 (Steven) add Xilinx不覆蓋Contact Height
bool bTestEPaddKg=false;                                                        //jou 20171026 (wei) : 測試中加壓EP
bool bAutoSiteMapHotplateReady=false;                                           //Ifor 20170920 (Steven) : add Auto Site Mapping Hotplate 旗標
bool bHP2Inarm=false;                                                           //Ifor 20170926 (Steven) : add Hotplate 資料轉移置InARM
bool bAutoSiteMapHotplateSave=false;                                            //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate Save
int iAutoSiteMapHotplatePlateR=0;                                               //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate PlateY
int iAutoSiteMapHotplatePlateC=0;                                               //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate PlateX
int iAutoSiteMapHotplateSource=0;                                               //Ifor 20170926 (Steven) : add Auto Site Mapping Hotplate Source
int iAutoSiteMapHPToSht=0;                                                      //Ifor 20171212 (Steven) : add Auto Site Mapping Hotplate iWhichShuttle
int iAutoSiteMapHPToKit=0;                                                      //Ifor 20171212 (Steven) : add Auto Site Mapping Hotplate iWhichKit
int iAutoSiteMapInArmRow=0;                                                     //Steven 20220527 : for JCET Auto Site Map
int iAutoSiteMapInArmCol=0;
int iAutoSiteMapInArmCol_8=0;
int iAutoSiteMapHPNo=0;
int iAutoSiteMapHPR=0;
int iAutoSiteMapHPC=0;
int iAutoSiteMapSiteNo=0;
int iAutoSiteMapCount=-1;                                                       //Steven 20220811 : 紀錄目前是哪個Shuttle
int iAutoSiteCurrStep=-1;                                                       //Steven 20220811 : 紀錄目前是哪個Shuttle
bool bAutoSiteMapHasPickHP=false;                                               //紀錄是否有到加熱盤吸料
bool bAutoSiteMapHotICCanPick=true;                                             //Ifor 20171225 (Steven) : add 避免 One Cycle 後執行Site Mapping 發生 Hang up
bool bSiteMappingNeedCheck=true;                                                //Ifor 20190308 : add 隨時開關 Site Mapping
bool bSiteMappingPlaceHotplate=false;                                           //Ifor 20190308 : add 隨時開關 Site Mapping
bool bARTSeparate=false;                                                        //kevin 20170908 (wei) add art 分bin 狀態成立
bool bLoadInarmAutoHigh=false;                                                  //kevin 20170929 (wei) load initial load 一盤 inarm 校正高度
bool bOutArmDropICSkip=false;                                                   //kevin 20171005 (wei) add out arm drop IC open door
bool bPushHomeDetect=false;                                                     //kevin 20171006 (wei) 手動按home
bool bManualTrayend=false;                                                      //kevin 20171026 (wei) 手動 tray end
int  iSECSMessageCanCloseByOperator=0;                                          //Ifor 20171024 : add KYEC SECS GEM Can Close By Operator 0:一般流程 1:直接關閉Alarm 2:啟動工號檢查流程
bool bSPBinYieldAlarm=false;                                                    //Isaac 20171113 (Steven) : add ATK Special Bin Yield alarm
bool bInitStartDelayTime=false;                                                 //wei 20171020 (jou) InitStartDelayTime秒數倒數
int  iInitStartDelayCount=0;                                                    //wei 20171020 (jou) InitStartDelayTime秒數倒數
bool bAutocleanDelay=false;                                                     //wei 20171020 (jou) Auto clean後，不InitStartDelayTime
bool bInitialStart1Time=false;                                                  //wei 20171020 (jou) InitialStart1 秒數倒數
int  iInitialStart1Count=0;                                                     //wei 20171020 (jou) InitialStart1 秒數倒數
bool  bInitialTestDelayStatus[15]={false};                                      //wei 20171020 (jou) 延遲狀態顯示
int  iInitialStart2Count=0;                                                     //kevin 20180905 InitialStart2 秒數倒數
bool bInitialStart2Time=false;                                                  //kevin 20180905 InitialStart2 秒數倒數
bool bPickerMoving=false;
bool bPauseHappen=false;
bool bByBinPercentCompareAlarm=false;
bool bByArmPerSiteCompareAlarm[2]={false};
bool bByBinYeildAlarm=false;                                                    //wei 20151116 Low Yield Onecycle中不alarm
bool bLotCheckOK=false;
bool bCloseSiteCleanout=false;
bool bInitalStartAutoHeight=false;                                              //wei 20171227 Inital Start Auto Height
bool bPowersaving=false;                                                        //wei 20180301 顯示Powersaving
bool bPowersavingStart=false;
bool bSECSGEM_Report=false;                                                       //Eastsun 20260526 #026-4.A10 Ifor 20240409 add:SECS/GEM Power Report (KYEC only)
bool bGPIBPause=false;
bool bGPIBAutoClean=false;
bool bRunSetupStep[10]={false};                                                 //wei 20180614 TSMC Setup Step
bool bCanSetupStep=false;                                                       //wei 20180614 TSMC Setup Step
int iMRTrayCount[10]={0};                                                       //wei 20180808 MR 盤數計數
int iCassetteFeed=0;
int iCassetteFeedTask=1;
bool bRunQAMode=false;
bool bMRArmZNeedHome[2]={false};
bool bMRNeedArmZHome=false;
bool bIsMRArmHome[2]   ={false};
bool bTrayCassetteTrayFeed=false;
bool bLoadCassetteTrayFeed=false;
bool bUnloadCassetteTrayFeed=false;
bool bEmptyReadIDOK=false;
bool bNoCheckSafeDoor11=true;
AnsiString asTrayIDData1="";
AnsiString asTrayIDData2="";
AnsiString asTrayIDData3="";
AnsiString asAuto3TrayIDData1="";

AnsiString asTrayIDDataCorverLoader="";
AnsiString asTrayIDDataLoader2="";
AnsiString asAutoCoverTrayID1="";
AnsiString asAutoCoverTrayID2="";
AnsiString asAutoCoverTrayID3="";
AnsiString asAutoCoverTrayID4="";
AnsiString asAutoCoverTrayID5="";
AnsiString asAutoCoverTrayID6="";
int iLoadIsCoverTray=HAS_IC;

int iNeedDeviceCassette=0;
bool bNeedDeviceCassette=true;
//bool bManualCatchCassette=false;
bool bManualCatchCassetteDown[5]={false};
bool bManualCatchCassetteUp[5]={false};
bool bLampMRManualCassette[10]={false};
int bLampMRManualMove[2]={0};
bool bRTNoUseHotPlate1=false;                                                   //20181008 Continuous lot 不使用HP1
bool bRTNewLot=false;                                                           //20181008 Continuous lot NEW LOT
bool bNewLotHotplate=false;
bool bNewLotUseHotplate1=false;                                                 //20181008 New Lot Use Hot plate 1
bool bCassetteReadOK=false;
bool bContinuoustest=false;
bool bContinuouslotOnecycle=false;
int iContinuouslotLoader=0;
bool bInitalForm=false;
int iNewLotShuttle=0;
bool bNewLotFullHotPlate=false;
bool bMRNotRT=false;
int iDownNumber=0;
bool bCheckHotPlate2=false;
int iPickPlaceCount=0;
bool bAutoFullTray[3]={false};
AnsiString asLoaderLotID="";
bool bNewLotLoader=false;
AnsiString asLoadPortCassetteID="";
AnsiString asBuffer6CassetteID="";
bool bLotDataOK=false;
AnsiString asLoadPortCassetteLotID="";
AnsiString asLoadPortCassetteData="";
bool bPleasePutBuffer10=false;
bool bLackCassette=false;
bool bCatchArmNoCassette=false;
bool bLoadPortNoCassette=false;
bool bStackedTrayNoTray=false;
bool bTrayBracketNoTray=false;
bool bRFIDReadFirst=false;
bool bRFIDReadFirst1=false;
bool bRFIDReadFinish=false;
bool bMRAutoLotStart=false;
bool bMRAutoFTLotStart=false;
bool bMRAutoMode=false;
bool bMRAutoCassetteOut=false;
int  iMRAutoCassette=0;
bool bS10F5show=false;
AnsiString asSameLotID="";
int iLotCount=0;
bool bAuto2ToEmpty=false;
bool bNeedResetAlarm=false;
bool bATRStartTime=false;
int  iATRStartCount=0;
AnsiString asNextLotID="";
AnsiString asNowLotID="";
bool bSearchNowLotID=false;
bool bSearchNextLotID=false;
bool bCheckCassetteFinish=false;
bool bWaitStartDelay=false;
int iWaitStartDelayTask=1;
bool bFTStartTime=false;
int  iFTStartCount=0;
AnsiString asMRLotData[3][4]={""};
int iMRLotCount=0;
bool bFTOpenSite[4][8]={false};
bool bOutarmAutoHigh=false;                                                     //kevin 20171115 (wei) load initial load 一盤 outarm 校正高度Z軸先不要下去吸
bool bSaveNeedHome=false;
bool bTrayFeedAfterCleanOut=false;                                              //KaiChen 20180928 ：bTrayFeedAfterCleanOut
int iXpitchMax=4000;                                                            //Isaac 20171204 (Steven) : X-Pitch 40mm擴大至50mm
int iXpitchMaxX2=8000;                                                          //Isaac 20171204 (Steven) : X-Pitch 40mm擴大至50mm
int iXpitchMaxX3=12000;                                                         //Isaac 20171204 (Steven) : X-Pitch 40mm擴大至50mm
int iXpitchMaxX4=12000;
int iXpitchMaxX5=12000;
int iXpitchMaxX6=12000;
int iXpitchMaxX7=12000;
int iXpitchMin=1333;                                                            //Steven 20180522 : X-Pitch最小值
int iXpitchMinX2=2666;                                                          //Steven 20180522 : X-Pitch最小值
int iXpitchMinX3=4000;                                                          //Steven 20180522 : X-Pitch最小值
int iXpitchMinX4=4000;
int iXpitchMinX5=4000;
int iXpitchMinX6=4000;
int iXpitchMinX7=4000;
int iXpitchMaxX1_MM;
int iXpitchMaxX2_MM;
int iXpitchMaxX3_MM;
int iXpitchMaxX4_MM;
int iXpitchMaxX5_MM;
int iXpitchMaxX6_MM;
int iXpitchMaxX7_MM;
int iXpitchMinX1_MM;
int iXpitchMinX2_MM;
int iXpitchMinX3_MM;
int iXpitchMinX4_MM;
int iXpitchMinX5_MM;
int iXpitchMinX6_MM;
int iXpitchMinX7_MM;
int iPitch_Max_minus_Min=8000;                                                  //Isaac 20171204 (Steven) : Xpitch40->50mm, 新增150mm的Teaching點位，內差法公式計算
int iPitchY_Max_minus_Min=4500;                                                 //Jimmychiu 20221205 add y pitch minus
bool bIndexPickUpErrorWaitRetry=false;                                          //Ifor 20171119 (Steven) : add 避免Index Pick Up Err Inarm 偷跑造成資料異常導致Hangup
bool bIndexPickErrShtStayRight1=false;                                          //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
bool bIndexPickErrShtStayRight2=false;                                          //JerryYang 20181206 (Steven) : fix 啟用D43功能時,index arm pick up error後按retry可能發生hang up
bool bInArmPlaceToShuttleFinish=false;                                          //Ifor 20171121 : add iWhichShuttle & iWhichKit 切換旗標 避免沒切換狀態造成Hangup
bool bFirstTimeEnableSocketSensor=true;                                         //Ifor 20171121 : add JCET 如春要求第一次啟動時須強制開啟Socket Sensor 功能
int iInArmPutIcToSH[2][2];                                                      //Ifor 20171121 : Test 查看異常資料
AnsiString asMotorDatabaseErr="";                                               //jou 20180814 (Steven) : 增加Motor database 異常警示
bool bOutArmPlaceDevice[4][8]={{false, false, false, false, false, false, false, false},    //kevin 20180119 (Steven) Add outarm place ic not home
                               {false, false, false, false, false, false, false, false},
                               {false, false, false, false, false, false, false, false},
                               {false, false, false, false, false, false, false, false}};
AnsiString sBinData[eTrayCount]={""};                                           //JerryYang 20220909 : 10->eTrayCount        //kevin 20180202 (Steven) record bin
int iContractModeTest=0;                                                        //kevin 20180222 (Steven) contract mode 手動送測試訊號
bool bContractModeTest=false;                                                   //kevin 20180222 (Steven) contract mode 秀手動送測試訊號
bool bLoadContractModeTest=false;                                               //kevin 20180222 (Steven) contract mode Load 不入tray
double queue20[20]={0};                                                         //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
bool bResetflag=true;                                                           //Isaac 20180301 (Steven) Index Cycle Time Monitoring function
int  iAddInitStartDelayCT      =0;                                              //kevin 20180307 add delay count InitStartDelayTime
bool bFinishInitStartDelay =false;                                              //kevin 20180308 add 動作完成
bool bCheckInarmPlaceShuttle=false;                                             //kevin 20180315 add inarm place shuttle 確認是否有抖動情形
bool bZ1Isdownflag=false;                                                       //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
bool bZ2Isdownflag=false;                                                       //Isaac 20180307 (Steven) index arm在shuttle放置位子時，shuttle抖抖須等到arm起來才能抖
AnsiString sPassWord="";                                                        //kevin 20180411 password

bool bAllSameHotCountHotOK=true;                                                //Ifor 20171128 :add
int iInArmSpeed1=0;                                                             //KaiChen 20171225 (Steven)：Add Speed Display
int iOutArmSpeed1=0;                                                            //KaiChen 20171225 (Steven)：Add Speed Display
int iInShuttleSpeed1=0;                                                         //KaiChen 20171225 (Steven)：Add Speed Display
int iOutShuttleSpeed1=0;                                                        //KaiChen 20171225 (Steven)：Add Speed Display
int iIndexSpeed1=0;                                                             //KaiChen 20171225 (Steven)：Add Speed Display
int iMagneticScalePos[16][1000];
int iLogLightScaleCount_InArmX1=0;                                              //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_InArmX2=0;                                              //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_InArmY1=0;                                              //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_InArmY2=0;                                              //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_OutArmX1=0;                                             //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_OutArmX2=0;                                             //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_OutArmY1=0;                                             //KaiChen 20171228 (Steven) ：Log Light Scale Data
int iLogLightScaleCount_OutArmY2=0;                                             //KaiChen 20171228 (Steven) ：Log Light Scale Data
bool bLogLightScale_InArm=false;                                                //KaiChen 20171228 (Steven) ：Log Light Scale Data
bool bLogLightScale_OutArm=false;                                               //KaiChen 20171228 (Steven) ：Log Light Scale Data
bool bWaitSecsGemReply=false;                                                   //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
bool bNeedAUTHORITYCheck=false;                                                 //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
int iShowAUTHORITY=0;                                                           //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
AnsiString strShowAUTHORITY="";                                                 //Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID
bool bIndexDropIC[2]={false,false};                                             //Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air
int iInArmAutoZHeight=0;                                                        //Frank 20171213 (Steven) : Auto get Height
int iOutArmAutoZHeight=0;                                                       //Frank 20171213 (Steven) : Auto get Height
AnsiString strInRotateDutAngle="";                                              //wei 20180316
AnsiString strOutRotateDutAngle="";                                             //wei 20180316
int iInArmType=0;                                                               //Ifor 20180518 : add 整合Hot plate 資料轉換
int iInHPType=0;
bool bRunAutoSiteMapping=false;                                                 //Ifor 20180518 : add 簡化Site Mapping 旗標
int iYieldSiteCount[2]={0};                                                     //wei 20180606 Interval Low Yield By Site
bool bYieldSiteBin[2]={false};                                                  //wei 20180606 Interval Low Yield By Site
int iYieldSiteBinpass[2][4][8]={0};                                             //wei 20180606 Interval Low Yield By Site
bool bYieldSiteBinCheck=false;                                                  //wei 20180606 Interval Low Yield By Site
bool bIntervalYieldIsPass[2][4][8][200]={false};                                //wei 20180606 Interval Low Yield By Site 20180629
int iYieldTotalCount=0;                                                         //wei 20180718 Interval Low Yield By Total
bool bYieldTotalBinIsPass[300]={0};                                             //wei 20180718 Interval Low Yield By Total
bool bYieldTotalBin=false;                                                      //wei 20180718 Interval Low Yield By Total
int iYieldTotalBinpass=0;                                                       //wei 20180718 Interval Low Yield By Total
bool bEnable_KLT_Function=false;                                                //Ifor 20180517 : add BarCode 編碼規則 0:民國 1: 西元 //Ifor 20180802 : add KLT bBarCodeRules -> bEnable_KLT_Function 0:KYEC 1: KLT
bool bSetDataChange=false;                                                      //kevin 20180606 (wei) add SEM GEM send ESD DATA
int iAutoCleanAlarm=0;                                                          //Ifor 20180727 (wei) ：Auto Clean Clean Count > Alarm Count 0:正常 1: Alarm 2:Clean Count
AnsiString sVersion="";                                                         //JerryYang 20180619 (wei) : ASE_CL 尚智要求主畫面版號可以自己修改
AnsiString sHisiWorkName[6]={""};                                               //kevin 20180626 add kevin 20170129 海思工作檔命名
AnsiString sATCFileName="";                                                     //kevin 20180626 add ATC FILENAME
int iByBinTotal[256];                                                           //kevin 20180703 bin 數量 Bin total[0]
AnsiString BinAssign[eTrayCount];                                               //JerryYang 20220909 : 10->eTrayCount        //kevin 20180705 Auto tray put bin
AnsiString sSVBinAssign[eTrayCount];                                            //JerryYang 20250224 : add
bool bUnloadHasBin[eTrayCount];                                                 //JerryYang 20220909 : 10->eTrayCount        //kevin 20180705 有BIN TRAY set true
int iTrayLastBin[eTrayCount];                                                   //JerryYang 20220909 : 10->eTrayCount        //kevin 20180705 每一個TRAY最後一個BIN
int iRecordCatchTray=0;                                                         //kevin 20180711 display catch tray
int iallSitCount=0;                                                             //kevin 20180720 add
AnsiString PC_NAME="";                                                          //kevin 20180803 pc NAME
TMyQueue10 QueueTaskList[qTaskCount];                                           //Steven 20180808 (wei) : 修改紀錄Task的方式
TMyStrQueue100 QueueGalilCmd;                                                   //Steven 20200320 : 紀錄Galil命令
TMyTimerQueue100 QueueTestTime;                                                 //Steven 20200714 : 新的紀錄測試時間方式
TMyTimerQueue100 QueueCycleTime;                                                //Steven 20200714 : 新的紀錄測試時間方式
TMyTimerQueue100 QueueAirOnTime1;                                               //Sam 20220329 : Record Index Air On Time
TMyTimerQueue100 QueueAirOnTime2;                                               //Sam 20220329 : Record Index Air On Time
TQPF_Timer tTestTimer;
TQPF_Timer tCycleTimer;
AnsiString StartTestTime;
AnsiString EndTestTime;
AnsiString StartCycleTime;
AnsiString EndCycleTime;
bool bUseInitTempOffset=false;                                                  //Steven 20180820 : 簡化init temp offset判斷式
bool bUseInitDelay=false;                                                       //Steven 20180820 : 簡化init temp offset判斷式

bool bYpitchNeddHome=false;                                                     //kevin 20180823
bool bUseDefineVer=false;                                                       //kevin 20180829 addUSE DEFIN VER
AnsiString SUseDefineVer="";
long double iContractCount=0;                                                   //kevin 20180928 add contract count
bool bEOTToLongStopBlowAir=false;                                               //kevin 20181009 add上次測試訊號太久 需停止吹氣
int iHasReadeLastData=0;                                                        //Ifor 20180802 : add 重開程式後Loader有Tray 且開啟Auto SPIK 強制手動取Tray
int iLoaderNum=0;                                                               //Frank 20181121 add iLoaderNum
AnsiString strATCBackupErrCode="";                                              //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm
AnsiString strATCBackupErrPart="";                                              //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm
int iATCBackupErrKCode=0;                                                       //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm
int iATCBackupErrPos=0;                                                         //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm
bool bATCBackupDuplicateErr=false;                                              //Ifor 20181219 : add KYEC 要求溫度相關Alarm 測試中需先Index 上升才Alarm
bool bNoAutoZSelect=false;                                                      //kevin 20181023 (Steven) : 不使用2段分tray氣缸
bool bIndexAlarmInArmAway=false;                                                //kevin 20181102 (Steven) :  index掉料 inarm 讓位 inarm先不要動
bool bSOTToLongStopBlowAir=false;                                               //kevin 20181009 add上次測試訊號太久 需停止吹氣
AnsiString sBufferSOT="";                                                       //wei 20181211 (Steven) : 更換位置SOT
AnsiString sBufferEOT="";                                                       //wei 20181211 (Steven) : 更換位置SOT
AnsiString sGalilData[100];                                                     //JerryYang 20181130 (Steven) : add galil command log
AnsiString sLMData[10];                                                         //JerryYang 20181130 (Steven) : add galil command log
bool bRTCAutoVerifyControlEP=false;                                             //jou 2014-06-24 RTC 自動進行Model驗證
bool bNeedWaitRTCAutoVerify=false;                                              //jou 2014-06-24 RTC 自動進行Model驗證
bool bDoFRTCAutoModelVerify=false;                                              //jou 2014-06-24 RTC 自動進行Model驗證
bool bDoBRTCAutoModelVerify=false;                                              //jou 2014-06-24 RTC 自動進行Model驗證
bool bNeedWaitContactTestAutoVerify=false;                                      //JerryYang 20210420 : contact test執行auto verify旗標
bool bIndexArm1PickUpErrNeedPiggyback=false;                                    //jou 20180814 : Index pick up error need piggyback
bool bIndexArm2PickUpErrNeedPiggyback=false;                                    //jou 20180814 : Index pick up error need piggyback
bool bUseFTPOneCycle=false;                                                     //Sam 20200305 : 增加 FTP 檔案檢查，檢查到 VIE_STOP.txt 就執行 OneCycle
MySiteData SiteData[TotalTestMode];                                             //JerryYang 20181011 (Steven) : SiteData改成全域變數
MyNextTrayNeedManualRemoveTray MaualRemoveTray3Alarm;                           //Jimmychiu 20231116 : add detail MES code for Previous tray pick up error
bool bWaitPreciserFinish=false;                                                 //Ifor 20180703 (Steven) : : 等待 Preciser 完成

int Z1DownToShuttle=0;                                                          //ChungHung 20171116 modify for Index Action
int Z2DownToShuttle=0;                                                          //ChungHung 20171116 modify for Index Action
bool bZ1UpAndPlaceZ2Down=false;                                                 //ChungHung 20171116 modify for Index Action
bool bZ1DownZ2UpAndPlace=false;                                                 //ChungHung 20171116 modify for Index Action

int iAutoSiteMapRunStartMode=0;                                                 //Ifor 20190916 :add Auto Site Mapping 備份Start Mode
int iIndexTakeDeviceChk1=0,iIndexTakeDeviceChk2=0;                              //kevin 20190103 回吸檢測狀態
int ishuttleShake=0;                                                            //kevin 20190107 add shuttle shake state
int iIndexArmWhich=0;                                                           //kevin 20190130 add use test arm
bool bHandlerPause=false;                                                       //Steven 20190123 : 紀錄Handler被暫停
int iHandlerStartCount=0;

bool bArm1SuckComplete=true;                                                    //JerryYang 20190123 新增保護避免真空持續on會造成all site掉料
bool bArm2SuckComplete=true;

bool bArm1NeedSuck=false;
bool bArm2NeedSuck=false;

bool bArm1DuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false}};
bool bArm2DuplicateErr[MAX_SOCKET_ROW][MAX_SOCKET_COL]={{false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false}};
bool bDropAtSht1NeedCheckVac[4][8]={{false, false, false, false, false, false, false, false},    //JerryYang 20241002 : add
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false}};
bool bDropAtSht2NeedCheckVac[4][8]={{false, false, false, false, false, false, false, false},    //JerryYang 20241002 : add
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false}};

bool bSHT1_DropPosHasIC[4][8]={{false, false, false, false, false, false, false, false},    //JerryYang 20241002 : add
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false}};

bool bSHT2_DropPosHasIC[4][8]={{false, false, false, false, false, false, false, false},    //JerryYang 20241002 : add
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false},
                                      {false, false, false, false, false, false, false, false}};

bool bSht1OnlyHasICErr=false;
bool bSht2OnlyHasICErr=false;

bool bD44Arm1CheckVacOn=false;
bool bD44Arm2CheckVacOn=false;
TQPF_Timer DoFrontTestDestroyICDelay;
TQPF_Timer DoRearTestDestroyICDelay;
bool bArm1D44SuckCheck=false;
bool bArm2D44SuckCheck=false;

int iPickerOrder =0;                                                            //kevin 20210914 initial 0
bool bIndexplaceCheck=false;                                                    //kevin 20190201 index 回溪檢測中
bool bSkipFlagSH1=false;                                                        //Isacc (Steven) 20180329 2DID All site fail recheck twice and alarm        //Steven 20190311 : Change to globe var
bool bSkipFlagSH2=false;                                                        //Isacc (Steven) 20180329 2DID All site fail recheck twice and alarm
double iTotalGf=0.0;                                                            //Steven 20190314 : Change total gf to globe var
bool bContractModeCheckPushZ1=false;                                            //kevin 20190227 add 前檢查是否有CI放在COCKET 造成機構損壞 按z1 確認
bool bContactModeNeedOpenDoor=false;                                            //JerryYang 20231218 : G22提醒人員取tray功能
bool AsebLoadCellTest=false;                                                    //kevin 20190305 add one cycle run arm 1 arm 2 load cell test
bool bLoadCellTest=false;                                                       //kevin 20190305 add one cycle run arm 1 arm 2 load cell test
bool bUseTwoArm32Site=false;                                                    //kevin 20190322 add two arm tester 32SITE 16SITE(4X4)
bool bQASampleCnt=0;                                                            //Steven 20190326 : QA Sampling
double dATCTempAdjustmentOffset[32]={0};                                        //Ifor 20190215 : add ATC 使用 三點校正功能
bool bATCTempAdjustmentOffset=false;                                            //Ifor 20190215 : add ATC 使用 三點校正功能
bool bTJControlMode=false;                                                      //Ifor 20190328 : add TJ Temp Over Range
bool bCheckGiveWay=false;                                                       //Ifor 20190328 : add TJ Temp Over Range
int iBackupTestY2_Middle  =0;
int iBackupTestZ2_Test    =0;
int iBackupTestZ2_Drop    =0;
AnsiString sATCPath;
bool bIndexDropVacuumError=false;                                               //kevin 20190418 避免 inarm 來回跑

bool bViewCom=false;
int iTeachCount=0;
int iMRStartMode=0;
int iNeedATR=0;

bool bLastTray=false;
bool bSendFTFinish=false;
int iNeedATRSet=0;
bool bFTCloseSite=false;
bool bRTCloseSite[4][8]={false};
bool bRTOpenSite=false;
AnsiString asRTCloseSiteMessage="";
int iLotQuantity=0;
int iStepRunFunction=0;
bool bRT2flag=false;
bool bDiecountfailFlag=false;
AnsiString asLotQuantity="";
AnsiString asPreSendID="";
bool bRTNoRunTrayID=false;
bool bPlaceToBufferCheck=false;

int iAutoCount[3]={0};
AnsiString TotalErrPart="";                                                     //kevin 20190610 add TotalErrPart
bool bWaitTSV=false;                                                            //Steven 20190521 : ATK lot count
bool bUseHotGunCheck=false;                                                     //kevin 20190621 hot gun 流量 不足偵測
bool bStartModeComplete=true;                                                   //Sam 20190429 : Add CC_PTI_NEWWORK
bool bPickLoaderDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                         //Steven 20190704 : bSuckDuplicateErr分類並改成全域變數
bool bPickHPDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                             //Steven 20190704 : bSuckDuplicateErr分類並改成全域變數
bool bTryPickHPDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                          //Steven 20190704 : bSuckDuplicateErr分類並改成全域變數
int iLoadPitchStepY=0;                                                          //Steven 20190704 : pitchstepY改成全域變數

bool bZ1ModifyDistanceRef=false;                                                //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
bool bZ2ModifyDistanceRef=false;                                                //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
int iZ1ModifyDistanceRef=0;                                                     //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height
int iZ2ModifyDistanceRef=0;                                                     //Ifor 20190530 : add Z Phase 可SKIP 但需重新contact height

bool bY1ModifyDistanceRef=false;                                                //Isaac 20201110 : Index Y find motor phase
bool bY2ModifyDistanceRef=false;
int iY1ModifyDistanceRef=0;
int iY2ModifyDistanceRef=0;

bool bFrontTestArmTorqueFinish=true;                                            //Ifor 20190912 :add 海思 V02.30 版 Record Torque
bool bRearTestArmTorqueFinish=true;
bool bArmTestInfoEvenLogStart_KYEC=false;
bool bArmTestInfoEvenLogEnd_KYEC=false;
AnsiString sArmTestInfoEvenLogFile="";

bool bUseHotGunFlowCheck=false;                                                 //KaiChen 20190729 ：Hot Gun Flow
AnsiString sTempsite[64]={NULL};                                                //kevin 20190928 add 溫度error 秀site編號
int iOneCycleTask=0;                                                            //JerryYang 20190925 one cycle task log
int iCleanOutCycleTask=0;                                                       //JerryYang 20190925 clean out task log
bool bLoaderNeedVibrate=false;                                                  //JerryYang 20191001 loader震動馬達
//JerryYang 20180921 Setup Teach功能
//==>
bool bInArmSetupTeach[InOfsTotal];                                              //分別表示InOfsLoader=0, InOfsHP1=1, InOfsHP2=2, InOfsInSh1=3, InOfsInSh2=4
bool bInArmStop[InOfsTotal];                                                    //分別表示InOfsLoader=0, InOfsHP1=1, InOfsHP2=2, InOfsInSh1=3, InOfsInSh2=4
int  iInArmPickPlaceCnt[InOfsTotal];                                            //In arm在各區域擺放的次數，大於10次就當作完成點位確認
bool bOutArmSetupTeach[OutOfsTotal];                                            //分別表示OutOfsOutSh1=0, OutOfsOutSh2=1, OutOfsAuto1=2, OutOfsAuto2=3, OutOfsAuto3=4, OutOfsFix1=5, OutOfsFix2=6, OutOfsFix3=7
bool bOutArmStop[OutOfsTotal];                                                  //分別表示OutOfsOutSh1=0, OutOfsOutSh2=1, OutOfsAuto1=2, OutOfsAuto2=3, OutOfsAuto3=4, OutOfsFix1=5, OutOfsFix2=6, OutOfsFix3=7
int  iOutArmPickPlaceCnt[OutOfsTotal];                                          //用來計數Out arm在各區域擺放的次數，大於10次就當作完成點位確認   //Ifor 20200825 Fix:記憶體破壞 8->12
int  iSortUnloadT6=-1;
//<==
//JerryYang 20180921 Setup Teach功能
AnsiString asChangeLogByLotPath="D:\\HT9045_Log\\ChangeLogByLot";               //Ifor 20191002 : add Change Log By Lot
AnsiString sChangeEvenLogFile="";                                               //Ifor 20191002 : add Change Log By Lot
int iShuttleLog=0;                                                              //JerryYang 20181121 (Steven) : add log,記錄in shuttle device是被震動馬達or搖搖功能導正
bool bAfterHomeShtChkLoseICNeedSlow[2]={false,false};                           //Sam 20201020 : 回 Home 第一次的 Shuttle Check IC lose 需要變慢。
AnsiString asEESUG_Data[7]={""};                                                //Ifor 20200529 add: SECS/GEM EESUG Offset Function
bool bBTestSuckHasError=false;                                                  //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
bool bBTestSuckError[2][4];                                                     //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
bool bFTestSuckHasError=false;                                                  //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
bool bFTestSuckError[2][4];                                                     //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
bool bDoBRTCGiveWayCheck=false;                                                 //Ifor 20191120 : add 新增RTC2.0 Full Check 讓位
AnsiString sHiLotID="";                                                         //kevin 20191016 客戶  LOT OSAT
AnsiString sOSATLotID="";                                                       //kevin 20191016 客戶  LOT OSAT
AnsiString sTemp="";                                                            //kevin 20191016 工作溫度
AnsiString sHandleID="";                                                        //kevin 20191016 機台編號
AnsiString sFlow="";                                                            //kevin 20191016 FTRT
AnsiString sDeviceType="";                                                      //kevin 20191016 產品名稱
AnsiString sTestProgram="";                                                     //kevin 20191016 測試程式
AnsiString sInsertion ="";                                                      //kevin 20191016 Process  FT1 FT2
AnsiString sOSATName="";                                                        //Ifor 20200724 add:測試廠名稱
bool bDropRetry=false;
bool bPickUpHomeFinish=false;
bool bOneCycleInArmToLoader=false;                                              //JerryYang 20200206 one cycle, clean out, tray feed時in arm移動到loader
int iWhichArmDown=0;                                                            //JerryYang 20200316 add SVID 哪支arm下壓在測區
bool bCheckTrayBySoftWareOpen=false;                                            //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
bool bCheckTrayBySoftWareOpen_Sen[MAX_FIX_TRAY]        ={false, false, false, false, false, false};     //KaiChen 20191128 ：矽格-湖口，軟體重啟時檢查所有Tray
double dfComplianceUnit=1.0;                                                    //kevin 20200313 add 浮動頭對應缸徑

AnsiString FTPAutomation_Down_ServerIP="";                                      //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Down_UserID="";                                        //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Down_Password="";                                      //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Down_ServerPath="";                                    //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Up_ServerIP="";                                        //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Up_UserID="";                                          //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Up_Password="";                                        //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Up_ServerPath="";                                      //KaiChen 20190530 ：Sigurd FTP Automation
AnsiString FTPAutomation_Up_ErrorCodePath="";                                   //KaiChen 20190530 ：Sigurd FTP Automation
int iCheckList_Enable[eCL_Total]={0};                                           //KaiChen 20190530 ：Sigurd FTP Automation
bool bSigurdDownload_Recipe=false;                                              //KaiChen 20190530 ：Sigurd FTP Automation
bool bSigurdUpload_Recipe=false;                                                //KaiChen 20190530 ：Sigurd FTP Automation
bool bSigurdUpload_Jamcode=false;                                               //KaiChen 20190530 ：Sigurd FTP Automation
int iRecordSigurdGPIBFlag=0;                                                    //KaiChen 20200330 ：紀錄，矽格 GPIB Commend Flag
AnsiString CheckListValue_Check="";                                             //KaiChen 20200108 ：矽格-CheckList 增加顯示異常數值
AnsiString CheckListValue_Current="";                                           //KaiChen 20200108 ：矽格-CheckList 增加顯示異常數值
int iOneDayLoaderCount=0;                                                       //KaiChen 20200618 ：矽格，增加Jam統計頁面
bool bSetBINCOUNT=false;                                                        //Sam 20210329 : Add GPIB BINCOUNT_
bool bOneCycleAfterSHLossIC=false;                                              //KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道
int iIndexArmCheck_SG_Arm1=0;                                                   //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset
int iIndexArmCheck_SG_Arm2=0;                                                   //KaiChen 20200826 ：矽格-湖口，要求IndexCheck使用Contact高度不使用Offset

bool bCanAutoCloseSite=false;                                                   //Steven 20200420 : 整合Auto Site Off
int USE_Scanner_AOI_Inspection    =0;                                           //Ifor 20190725 : add Scanner AOI
int USE_Scanner_AOI_Inspection_FixLight_Z_Axis=1;                               //Eastsun 20260410 : AOI Fix Light Z Axis Mode
bool bAOIBallDamageCounterFailAlarm = false;                                    //Ifor 20190725 : add Scanner AOI
bool bAOIBallDamageCounterFailUnTest = false;                                   //Ifor 20190725 : add Scanner AOI
bool bPickSH1Flag=false;                                                        //Ifor 20191226 移至全域
bool bPickSH2Flag=false;                                                        //Ifor 20191226 移至全域
int iSLT_HeadContactCount[2][16];                                               //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
AnsiString asSLT_LotStartTime="";                                               //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
AnsiString asSLT_LotEndTime="";                                                 //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
int iDBQueryDays=3;                                                             //Ifor 20200324 : add KYEC 要求新增DB Qery 天數
int iLDTrayNeedManualRemoveTray=0;                                              //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray 0:Normal 1:Edit Loader Tray 2:Data Change
bool bNeedManualRemoveTray=false;                                               //Ifor 20200331 : add Edit Loader Tray Need Manual Remove Tray
bool bMOFileControlBinChangeAlarm=false;                                        //Sam 20200525 : Control Bin
bool bNewCatchTrayblock=false;                                                  //kevin 20200512 夾tray遮版削短
bool bHandlerModel=false;                                                       //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
bool bHasICinSocket=false;                                                      //Steven 20200612 : for Z1 PnP, Z2 test
AnsiString sSocketSensorErr="";
bool bUseStartSoundAlarm=false;                                                 //kevin 20201116  Start 發出聲音 不動 5sec
bool bStartMoveSpeed=false;                                                     //kevin 20201116  motor move speed 10 % 移鄧10 sec 系統暫停 恢復速度
bool bStartOpenDoor=false;                                                      //kevin 20201116  有開門停機

bool bTryPickLastRowIng=false;                                                  //JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看
int iBackupLoadDir=0;
bool bSht1LoseICErr=false;                                                      //JerryYang 20200610 Out shuttle lose IC需先讓位再發出alarm
bool bSht2LoseICErr=false;
int i3SigmaTempMonitior_NowContactCount =0;                                     //Hmy 20200515 Copy//Hmy 20200510 Add Enable 3 Sigma Temp Monitor//Hmy 20200510 Add Enable 3 Sigma Temp Monitor
int iReceiveATCData=0;                                                          //kevin 20200608 讀取ATC 參數資料
bool bD52IndexArmUp=false;                                                      //JerryYang 20200804 : fix D52 & Index arm在shuttle高度預熱功能同時啟用時，發生tester timeout時會誤發handler hang up
AnsiString asSetupFileCheckList="";                                             //Ifor 20200914 add:Setup File Check List
int USE_Top_Scanner_AOI_Inspection=0;                                           //Ifor 20200902 add: TFAMD Top AOI
bool bArm1Delay=false;
bool bArm2Delay=false;
bool bArm1IsTest=false;                                                         //JerryYang 20180629 (wei) : 用來判斷是否在測試中
bool bArm2IsTest=false;                                                         //JerryYang 20180629 (wei) : 用來判斷是否在測試中
bool bTesterSendPause=false;                                                    //Steven 20201022 : For RFMD
bool bNeedReplunge_RFMD=false;                                                  //Steven 20201022 : For RFMD
bool bTesterPauseMusic=false;                                                   //Steven 20220616 : Can select "Alarm Reset" when show "Tester Pause" for QORVO.
bool bPauseAlarmDelayActive=false;                                              //RogerYang 20260626 : Tester Pause 逾時計時    //AI(W906-GB-P2c) 20260926: golden 912 cmydef.cpp:5049
TQPF_Timer hPauseAlarmDelay;                                                    //RogerYang 20260626 : Tester Pause 逾時計時    //AI(W906-GB-P2c) 20260926: golden 912 cmydef.cpp:5050

bool bDoEmptySocketCheck=false;                                                 //Steven 20201022 : For RFMD Empty Socket Check Funstion
int  iESC_IndexContactCount=0;
int  iGetESCResult=0;
bool bDoEmptySocketOneCycle=false;

bool bAPAXConnectFileAlarm=false;                                               //ChungHung 20180912 add for APAX Alarm
bool bApaxWriteFinish=true;
bool bApaxReadFinish=true;
int iAPAXEPValue[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                         //Nickliu 20180827 Add APAX Dll Connection
int iAPAXDualEPValue[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};                     //Ifor 20250319 add:Mutil EP Control
bool bIndexCheckVacum=false;                                                    //kevin 20200716 add ASE_KH 卡輸入
int iSecsGemSwitchFTRT=0;                                                       //Steven 20210202 : 透過SECS/GEM切換動作狀態 0:無動作, 1:切換中, 2:切換成功

bool fRearNeedSuckIC=false, fFrontNeedSuck=false, fFrontNeedDestroy=false;      //Steven 20210217 : 改成全域
bool fFrontNeedSuckIC=false, fRearNeedSuck=false, fRearNeedDestroy=false;
bool f32SiteNeedDestroy=false, f32SiteNeedSuck=false;

int JCET_FOR_EVAN=0;                                                            //Steven 20210304 : 長電給如春的版本
int SPIL_FOR_QLE=0;                                                             //Steven 20230110 : For渠梁
int bAutoDeviceEjection=0;                                                      //AI(ht9045-v899) 20260417: config flag for AutoDeviceEjection panel
int iOutShuttle1HasICErrRetryCnt=0;                                             //Sam 20210409 : 改為全域
int iOutShuttle2HasICErrRetryCnt=0;                                             //Sam 20210409 : 改為全域
bool bIn_ICRotationCompleteOnKit=false;                                         //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
bool bOut_ICRotationCompleteOnKit=false;                                        //Sam 20210623 : 修正 IC 轉完後被 Home，導致 IC 轉向異常。
bool bIn_XYMoveFinishOnRotationKit=false;                                       //Sam 20240726 : 修正移動完 XY 準備放 IC 到 RotationKit 時 Home
bool bOut_XYMoveFinishOnRotationKit=false;
bool bLoadNewEmptyTrayToCarStart=false;                                         //Sam 20211119 : 放 Tray 增加保護
bool bUnLoadNewEmptyToStackStart=false;                                         //Sam 20211119 : 放 Tray 增加保護
bool bLoadNewColorTrayToCarStart=false;                                         //Sam 20211119 : 放 Tray 增加保護
bool bUnLoadNewColorToStackStart=false;                                         //Sam 20211119 : 放 Tray 增加保護

bool bShowIndexMotorError=false;                                                //Isaac 20201012 : index Y超過範圍，做一次Tmode，show alarm視窗變大
int iMaxCommandY1=0,iMinCommandY1=0,iMaxCommandY2=0,iMinCommandY2=0,iMaxTeachY1F=0,iMinTeachY1F=0,iMaxTeachY1M=0,iMinTeachY1M=0,iMaxTeachY2M=0,iMinTeachY2M=0,iMaxTeachY2R=0,iMinTeachY2R=0;    //Isaac 20201012 : 計算Encoder和commandpos/Teaching的差值
bool bOverRangeDoTMode=false;                                                   //Isaac 20201012 : index Y超過範圍，做一次Tmode，判斷超出範圍
bool bOverRange4Indexhome=false;                                                //Isaac 20201012 : index Y超過範圍，做一次Tmode，回home中，先不做Tmode
bool bTriger4Indexhome=false;                                                   //Isaac 20201012 : index Y超過範圍，做一次Tmode，觸發旗標
int iIndexOverRangeCount=0;                                                     //Isaac 20201012 : index Y超過範圍，做一次Tmode，自動回home三次，第三次完成後報alarm

int iIndexY1FindPhaseFrontPos=0;                                                //Isaac 20201110 : Index Y find motor phase
int iIndexY1FindPhaseMiddlePos=0;
int iIndexY2FindPhaseMiddlePos=0;
int iIndexY2FindPhaseRearPos=0;

bool iAutoZXYMove=false;                                                        //kevin 20210223 Auto Teach X Y offset
bool bLoadAutoTeachInarm=false;                                                 //kevin 20210305 add inarm Auto teach move  X Y PITCH Z 軸不動
bool bCleanOutFinish=false;                                                     //kevin 20210406 clean out finish
bool beKeepCloseSite=false;                                                     //kevin 20210406 clean out open close Site
bool bRetryReadToqu=false;                                                      //kevin 20210419 重讀扭力
AnsiString  sSetTorquValue[2]={"0.0"};                                          //kevin 20210421 扭力設定讀取值
int iIndEPCnt=16;                                                               //JerryYang 20210413 : 獨立EP數量
double dAdaptiveStardardYield=-0.01;                                            //Sam 20230914 : 自適應性良率監控
int iAdaptiveACInterval=-1;                                                     //Sam 20230914 : 自適應性良率監控
int iAdaptiveACIntervalNor=-1;                                                  //Sam 20240726 : AI Clean
int iAdaptiveLowYieldCntNor=0;
int iAdaptiveLowYieldCntMin=0;
bool bResetNotMsg=false;                                                        //Sam 20240215 : Tester time out show reset all ic
bool bRestModeBackupParm=false;                                                 //Sam 20250820 : [I49] 清料時 Contact Heigh 要拉高
int iRestModeBackContactMode=0;
double dRestModeBackContactHeigh1=0.0;
double dRestModeBackContactHeigh2=0.0;
bool bLoadingNewICTray=false;                                                   //Sam 20240827 : 新增 AMR 功能
int iSetTorqueAlarm[2]={0};                                                     //kevin 20210505 扭力連續幾次ALARM
bool bLowYeildAlarmSpecial=false;                                               //Sam 20210505 : PTI 要求的兩段 Low Yeild
bool bLowYeildAlarmSpecial1stPass=false;                                        //Sam 20210505 : PTI 要求的兩段 Low Yeild
int iCheckTorqueCount=100;
double dTorqueArray[2][2000];
int iTorqueArrayCnt[2]={0, 0};
bool bHasTrayCSV=false;                                                         //Steven 20210629 : Tray Form改成CSV
bool bHasPlateCSV=false;                                                        //Steven 20210629 : Plate Form改成CSV
bool bContraPoisitionFlag=false;
int  iControlPanelMode   =0;                                                    //Ken 20210702 AddPadInterface
int  VCCU_UNIT_TYPE=0;                                                          //Sam 20230210 : 新增 VacuumUnit 通訊模組
int iWhichIndexArm=0;                                                           //Sam 20231214 : Temp offset use ready temp range
bool bNeedReCheckHeat=false;

int iAutoSiteMappingErrCT=0;                                                    //jou 20200707 : VTEST auto site mapping

bool bOHTUseManual_LP1=false;                                                   //KaiChen 20200716 ：OHT
AnsiString asGETPFCPARAMETER="";
AnsiString asALLGETPFCPARAMETER[14][20]={""};
bool bSETPFCPARAMETER=false;
AnsiString asGETSLOPEOFFSET="";
AnsiString asALLGETSLOPEOFFSET[2][8]={""};
AnsiString asALLSETSLOPEOFFSET[2][8]={""};
bool bSETSLOPEOFFSET=false;
bool bOHTUseManual_LP2=false;
bool bLoadPortHasCSTWaitCatch=false;
bool bCassetteRejectAfterSlotMap=false;

int iCatch_Cassette_Push_OHT[10] ={1,1,1,1,1,1,1,1,1,1};
int iCatch_Cassette_Pop_OHT[10]  ={1,1,1,1,1,1,1,1,1,1};
bool bLoadNeedTray=false;
bool bLoadNeedFailTray=false;
bool bEmptyNeedTray=false;
bool bLoadHasStackedTray=false;
bool bEmptyHasStackedTray=false;
bool bBufferHasNewCassette=false;
bool bLoadPortLoad=false;
bool bBufferHasOldCassette=false;

bool bBuffer6NeedCassette=false;
bool bBuffer6NoNeedCassette=false;
bool bBuffer7NeedCassette=false;
bool bBuffer7NoNeedCassette=false;
bool bLoadPortHaveCassette=false;
bool bBuffer1HaveCassette=false;
bool bBuffer6CanUp=false;
bool bBuffer7CanUp=false;
bool bLoadPortHasCassette_OHT=false;
bool bBuffer6CombineTray=false;
bool bBuffer6SeparationTray=false;
bool bEmptyFull=false;
bool bMRQAMode=false;
bool bMRQAModePassBin=false;
bool bBuffer6NeedEmpty=false;
bool bBuffer7NeedEmpty=false;
bool bCassetteMessage=false;
bool bSTArmMove[3]={false, false, false};
bool bSTMove=false;

int iCatchFunction=0;
int iCatchBufferFunction=0;
int iPlaceBufferFunction=0;

bool bLoadPortHasCassette=false;
int iCatch_Cassette_Push[10]    ={1,1,1,1,1,1,1,1,1,1};
int iCatch_Cassette_Pop[10]     ={1,1,1,1,1,1,1,1,1,1};

AnsiString CSTData[7]={"EMPTY","TRAY","DEVICE","FULLDEVICE","FULLTRAY","PASS","FAIL"};

AnsiString SenIndexName_OHT[7]={"Loader Tray", "Empty Tray", "Tray Bracket", "Tray Bracket 2",       //QQQ
                                "Auto1 Tray", "Auto2 Tray", "Auto3 Tray"};

bool bSTArmRun=false;

int iLoadAutoCount=0;

bool bBuffer7CombineTray=false;
bool bBuffer7SeparationTray=false;

int iCSTArmHasTrayCount=0;                                                      //KaiChen 20200716 ：OHT Cassette Arm Scan Tray Count
int iRecordSenOnOff[150]={0};                                                   //KaiChen 20200716 ：OHT Cassette Arm Scan Tray Count
AnsiString asCSTArmHasTrayCount="";                                             //KaiChen 20201019 ：OHT Cassette Arm Scan Tray Count
int iLoaderTrayCount_OHT=0;                                                     //KaiChen 20201019 ：OHT 紀錄 Loader Tray Count
AnsiString asLoaderCassetteID_OHT="";                                           //KaiChen 20201019 ：OHT 紀錄 Loader Cassette ID

bool bSecsGem_SLOT_MAP=false;
bool bSecsGem_CST_IN_BUFFER=false;

bool bSecsGem_LoaderTrayID_OK=false;
bool bSecsGem_LoaderTrayID_NG=false;

bool bSecsGem_EmptyTrayID_OK=false;
bool bSecsGem_EmptyTrayID_NG=false;

bool bSecsGem_CHKEmptyTray_OK=false;
bool bSecsGem_CHKEmptyTray_NG=false;
AnsiString asManualEmptyTrayID="";

int iLoadPortNumber=0;

bool bMultileTrayIDAction=false;
bool bMultileTrayDoorOpen=false;

bool bTempOverLockDoor=false;

bool bMultileDoorIsOpen=true;

bool bMultileDoorState_Old=true;

bool bSendEven_NeedEmptyCST=false;                                              //KaiChen 20201222 ：OHT，Add
bool bSendEven_NeedEmptyCST_FristScan=false;                                    //KaiChen 20201222 ：OHT，Add
bool bSendEven_First=true;
bool bManualCSTOUT=false;
bool bManualRemoveLP2CST=false;

bool bDoIniStartAutoIonFanClean=false;                                          //Isaac 20210609 : IO觸發IonFan清針
bool bStartAutoIonFanClean=false;                                               //Ifor 20210720 add: IO觸發IonFan清針才顯示
bool bReadTrayID=false;                                                         //kevin 20210623 TRAY ID Read
bool bReadColorTrayID=false;                                                    //kevin 20210623 TRAY ID Read
AnsiString TrayID[eTrayCount][3]={""};                                          //kevin 20210623  [6]: load empty COLOR [3]: 0:read  1: send MOVE TRAY PLACE                //QQQ
int iTrayTotal[3]={0};                                                          //kevin 20210623  0:loader 1:Empty  2:olor  Tray 入軌道數量
int iOneTrayPickCount[ePosTrayCount]={0};                                       //kevin 20210623  loader 吸取數量
int iAutoTrayPlaceCount[ePosTrayCount]={0};                                     //kevin 20210623  Auto123 Fix 123 放ic數量
int iGPIBIndexStatus=0;
bool bOutArmPlaceToUnloaderDestroy=false;                                       // 2012.06.14 , Joye , OutArm Place To Unloader Destroy Check
bool bOutArmPlaceToUnloaderDestroy_EMGAndPowerOff=false;                        // 2012.06.14 , Joye , OutArm Place To Unloader Destroy Check

bool bCom1Error=false;                                                          //kevin 20210902 Torque error
AnsiString aSendSiteMapping="";                                                 //Ifor 20201030 add:送Site Mapping 資料給GPIB
int iESD_DecayTask=-1;                                                          //Ifor 20220112 add:改全域變數
bool bDecayComplete=false;                                                      //Ifor 20220119 add:Auto Decay Complete Flag
bool bInArmAutoAlignmentClearFlag=false;                                        //Kenhsieh 20211007 : 解決沒有掃描Kit問題
bool bOutArmAutoAlignmentClearFlag=false;                                       //Kenhsieh 20211007 : 解決沒有掃描Kit問題
int iAutoCleanRowSel=0;                                                         //KaiChen 20200622 ：修改 10Site2X5 AutoClean
int iShuttleRowKit=0;                                                           //KaiChen 20200622 ：修改 10Site2X5 AutoClean
int iStiCT=0;                                                                   //kevin 20211106 add 抽測數量計數
int StipulateInputCount=0;                                                      //kevin 20211106 抽測 數 量
bool bDoLoaderCleanOut =false;                                                  //kevin 20211106 抽測 數 量到達 LOAD 收TRAY
bool bTorueReceve=false;                                                        //kevin 20211117 扭力100 筆資料
bool bTorueReceveSTOP=false;                                                    //kevin 20211117 扭力STOP資料
bool RPLogRecord=false;                                                         //Sam 20201209 : Default Recipe ChangeLog
bool bIndex1Suck =false;                                                        //kevin 20220105 Index 在下真空建立 pause 不能關閉
bool bIndex2Suck =false;                                                        //kevin 20220105 Index 在下真空建立 pause 不能關閉
int  USE_INDEX_ARM_AXES=IndexArm_4_Axis;                                        //JimmyChiu 20220708 : add Index Arm Axis

const int AutoOffset_InShuttlePlace       =0;                                   //JimmyChiu 20211020 : Auto alignment mode
const int AutoOffset_HotPlatePlace        =1;
const int AutoOffset_LoaderPlace          =2;
const int AutoOffset_OutShuttlePlace      =3;
const int AutoOffset_UnLoaderPlace        =4;
const int AutoOffset_Homeing              =5;
const int AutoOffset_AutoHeight           =6;
const int AutoOffset_ShuttleSensorCheckL  =7;
const int AutoOffset_ShuttleSensorCheckR  =8;
const int AutoOffset_AutoTeachFinish      =9;
const int AutoOffset_HotPlatePick         =10;

bool bUse8Picker=false;                                                         //Steven 20201014 : 整合8吸嘴auto clean
bool bCleanKitSuckDuplicateErr[MAX_ARM_Row][MAX_ARM_Col];                       //Steven 20091218 : Avoid duplicate message
int iWhichAutoNeedTray=0;                                                       //Steven 20211228 : 方便Debug
bool USE_ATC_RS232_Check=false;                                                 //Ifor 20211116 add: ATC RS232 Check Function
TQPF_Timer tOEESystemCycle;                                                     //Jimmychiu 20220712 decrease loading
bool USE_ARM_PROTECTION=true;                                                   //Steven 20220314 : In Our Arm Z Sensor保護加上開關
AnsiString sGigasFTPPassWord="";                                                //Isaac 20210128 : 為了讓Enable FTP不被鎖住，supervisor以下需輸入密碼
AnsiString asTCPIPTemperature="";
AnsiString asTCPIPBarCode[32]={"0"};
int iSECSGEMMachineState=0;                                                     //JerryYang 20220215 : MachinePreState改成int
int iSECSGEMMachinePreState=0;                                                  //Ifor 20221019 add: Machine PreS tate改成int
AnsiString sMacStatus[30]={"LOCK", "EMG 1", "EMG 2", "EMG 3", "EMG 4", "Power Off", "Homing", "Auto Retest", "HP Check", "Cleaning",
                           "OCR Insp", "Reseting", "Piggy Back", "QA Mode", "Onecycle Cleaning", "No Tray", "Running", "Heater Wait", "Cooling Wait", "PAUSE",
                           "HALT", "Index Check", "RUN CHECK", "Defrosting", "ATC Self Test", "RTC Mode", "Alarm", "Decay Test", "Init", "Wait"};
bool bFTPDownlodFinish=false;
bool bHalfViewVerifyNeedAboveSocket=false;                                      //JerryYang 20220215 : RTC Auto Verify half view check
bool bPickUpErrReAutoVerify=false;
bool bAutoSkipCntOver=false;                                                    //JerryYang 20210420 : 矽品蘇州要求auto skip數量小於設定值要由工程師排除
double dIndexZOffset[3][15]={0};                                                //Ifor 20210114 add: Index Z Offset //Ifor 20230207 add: 10 => 11
bool bChangeTest_TempAlarm=false;                                               //Ifor 20210623 add: Test Temp Change
int bChangeTest_TempOffset=0;                                                   //Ifor 20210623 add: Test Temp Change
bool bDoATCTempRise=false;                                                      //Ifor 20230504 add: ATC 回溫功能flag
bool bReadMCU1=false, bReadMCU2=false;                                          //kevin 20220225 read MCU DATA
int iSelectArm=0;                                                               //kevin 20220426 0: Arm1 Arm 2  1: Arm1  2: Arm2
int iUnloaderTrayCountCal[MAX_TRACK]={0, 0, 0, 0, 0, 0, 0, 0, 0};               //Frank 20220322 Add
int iFixTrayCountCal[MAX_FIX_TRAY]={0, 0, 0, 0, 0, 0};                          //JerryYang 20240318 : add
//bool bUseAGV=false;                                                             //kevin 20220518 AGV Modal
AnsiString sLoadPickupErrorTrayPos[MAX_ARM_Row][MAX_ARM_Col]={""};              //kevin 20220521 Load pick error record X, Y 座標
bool bHotPlateHasSiteMap=false;                                                 //Stevn 20220602
int  iResetSiteMappingStep=0;                                                   //Stevn 20220602    //0:已補回去     //1:One Cycle或Clean Out, auto site map即將重置    //2:記錄auto site map要轉回continue start前, IC還沒補回加熱盤
int iStackCount[MAX_TRACK]={0};                                                 //kevin 20220527 add Tray Count 0:load 1:empty 2:Color 3:Auto1 4:Auto2 5:Auto3
int iPosWos=500;                                                                //kevin 20220616 add Test
bool bResetGalilTwoYMove=false;                                                 //Ifor 20220729 add Reset Galil Two Y Move時間計時
AnsiString sRFIDData="";                                                        //Steven 20220713 : RFID Reader for SJSEMI
int iRFIDTag=0;
bool bAUTORev[MAX_AUTO_TRAY]={false, false, false, false,false,false};
int iInShuttleJam[2][9]={0};                                                    //kevin 20220819 shuttle Jam  紀錄位置後續，要做放到垃圾桶動作
bool bIsOpenDoorNeedSetErrBin=false;                                            //JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN
bool bUpdateAutomatically=false;                                                //Sam 20220824 : FTP 自動下載安裝更新包
bool bUpdateAutomaticallyChecked=false;                                         //Sam 20220824 : FTP 自動下載安裝更新包
TMyStringList *slGroundManLog;                                                  //KenHsieh 20220728 : 新增GroundMan Value Log
bool bAutoReceTray=false;                                                       //kevin 20220908 add 退料TRAY 補空TRAY
int iSensor[4][9];                                                              //kevin 20220912 log Shuttle display

bool MACHINE_HAS_AUTO_ALIGNMENT_CCD = false;                                    //ChungHung 20210113 add for Alignment CCD
bool Device_Bottom_Blower=false;

unsigned long lTeachAlignmentFlag = 0 ;
unsigned long lInArmAutoAlignmentFlag = 0  ;
unsigned long lOutArmAutoAlignmentFlag = 0  ;

unsigned long lInArmAutoAlignmentTrayTimingFlag = 0;
unsigned long lOutArmAutoAlignmentTrayTimeingFlag = 0;

unsigned long lInArmAutoAlignmentCKTimingFlag = 0;
unsigned long lOutArmAutoAlignmentCKTimeingFlag = 0;

//ChungHung 20210113 add for Alignment CCD end

bool bRunTeachAlignment = false;                                                //ChungHung 20210113 add for Alignment CCD
bool bRunInArmAutoAlignment = false;                                            //ChungHung 20210113 add for Alignment CCD
bool bRunOutArmAutoAlignment = false;                                           //ChungHung 20210113 add for Alignment CCD
bool bLoaderNeedTrayMustFinish = false;                                         //ChungHung 20210113 add for Alignment CCD
bool bAutoNeedTrayMustFinish = false;                                           //ChungHung 20210113 add for Alignment CCD

bool bAutoAlignmentInArmShuttle1Lock = false;
bool bAutoAlignmentInArmShuttle2Lock = false;

bool bAutoAlignmentOutArmShuttle1Lock = false;
bool bAutoAlignmentOutArmShuttle2Lock = false;

bool bAlignmentChangeUnloadTray[eTrayCount];                                    //Steven 20240428 : Add for HT9011 AOA

bool bSpin = true;
int iSourceSystemHeight;                                                        // 20230601 Joseph , Auto Form Size //
int iSourceSystemWidth;                                                         // 20230601 Joseph , Auto Form Size //
int iNowSystemHeight;                                                           // 20230601 Joseph , Auto Form Size //
int iNowSystemWidth;                                                            // 20230601 Joseph , Auto Form Size //
double fNowSystemHeightScale;                                                   // 20231030 Jason , Auto Form Size //
double fNowSystemWidthScale;                                                    // 20231030 Jason , Auto Form Size //

int iCCDAlignmentMotorDelay=200;
int Teach_AutoAlignmentUnit=0;

bool bHasOutShuttleLose=false;                                                  //Ifor 20210911 add: KLT 要求out shuttle loss IC alarm之後，按下Start要跳出確認視窗
bool bHasNozzleEvent=false;                                                     //Ifor 20211014 add:按下Rester機台吸嘴中有正在丟料的狀態
int iIonAlarmState=0;                                                           //Ifor 20211014 add:Start Ion Alarm 延遲報警 0:初始化 1:設定時間 2:完成延遲
int bIonAlarmDelayTime=0;                                                       //Ifor 20211014 add:Start Ion Alarm 延遲報警
bool bContactModeCheckOpenDoor[2];                                              //Ifor 20211203 add: KLT要求Contact模式需人員確認InShuttle Sensor與開門

int iAOA_InArm_Loader_X=0;                                                      //KenHsieh 20210830 : add CCD AUTO ALIGNMENT for Offset
int iAOA_InArm_Shuttle1_X=0;
int iAOA_InArm_Shuttle2_X=0;
int iAOA_InArm_Hotplate1_X=0;
int iAOA_InArm_Hotplate2_X=0;

int iAOA_InArm_Loader_Y=0;                                                      //KenHsieh 20210918 : CCD AUTO ALIGNMENT offset新增
int iAOA_InArm_Shuttle1_Y=0;
int iAOA_InArm_Shuttle2_Y=0;
int iAOA_InArm_Hotplate1_Y=0;
int iAOA_InArm_Hotplate2_Y=0;
int iAOA_OutArm_Auto1_X=0;
int iAOA_OutArm_Auto1_Y=0;
int iAOA_OutArm_Auto2_X=0;
int iAOA_OutArm_Auto2_Y=0;
int iAOA_OutArm_Auto3_X=0;
int iAOA_OutArm_Auto3_Y=0;
int iAOA_OutArm_Auto4_X=0;                                                      //Steven 20240428 : Add for HT9011 AOA
int iAOA_OutArm_Auto4_Y=0;
int iAOA_OutArm_Auto5_X=0;
int iAOA_OutArm_Auto5_Y=0;
int iAOA_OutArm_Auto6_X=0;
int iAOA_OutArm_Auto6_Y=0;
int iAOA_OutArm_Fix1_X=0;
int iAOA_OutArm_Fix1_Y=0;
int iAOA_OutArm_Fix2_X=0;
int iAOA_OutArm_Fix2_Y=0;
int iAOA_OutArm_Fix3_X=0;
int iAOA_OutArm_Fix3_Y=0;
int iAOA_OutArm_Fix4_X=0;                                                       //Steven 20240428 : Add for HT9011 AOA
int iAOA_OutArm_Fix4_Y=0;
int iAOA_OutArm_Fix5_X=0;
int iAOA_OutArm_Fix5_Y=0;
int iAOA_OutArm_Fix6_X=0;
int iAOA_OutArm_Fix6_Y=0;
int iAOA_OutArm_Shuttle1_X=0;
int iAOA_OutArm_Shuttle1_Y=0;
int iAOA_OutArm_Shuttle2_X=0;
int iAOA_OutArm_Shuttle2_Y=0;

bool bInArmXPitch_40mm=false;                                                   //KenHsieh 20211023 : 新增X Pitch 40mm teach
bool bOutArmXPitch_40mm=false;                                                  //KenHsieh 20211023 : 新增X Pitch 40mm teach
bool bNeedOneCycleByAutoAlignment=false;                                        //KenHsieh 20211202 : 修改重新打開AOA後，先做Onecycle再回Home重做AOA
const int iZMovePos=10;
const int iXYMovePos=1;
bool bAutoCleanPlaceToSht=false;                                                //Steven 20220927 : Add flag for place pad to shuttle
int TRAY_MAPPING_GRAB=0;                                                        //KenHsieh 20220923 : add Tray Map Throw IC Function
bool bS2F42_TrayMap_Receive=false;                                              //KenHsieh 20220923 : add Tray Map Throw IC Function
bool bS2F42_TrayMap_FullTray=false;                                             //KenHsieh 20220923 : add Tray Map Throw IC Function
bool bNeedInputEQCQty=false;                                                    //JerryYang 20220923 : 手動輸入EQC數量
bool bStopART=false;
bool bBigMyMessage=false;
bool bInArmWaitOneCycle=false;
double dCycleTimeArr[20];                                                       //JerryYang 20220923 : Index cycle time監控
bool bDoingF16=false;                                                           //Steven 20221213 : 確認shuttle 有沒有斷線

int iByBinCnt[TEST_MAX_BIN+1];
int iExceptAutoCnt[eTrayCount];                                                 //JerryYang 20240927 : 2D SORT模式Output arm放料避免空洞
bool bInitNeedDownloadFTP=false;                                                //JerryYang 20200416 艾科要求切initial start按start要強制download recipe
double fIndexDownPos=-148.0;                                                    //Steven 20211109 : 改成全域變數
int iOneCycleFinishShowMsg;                                                     //Sam 20250115 : 矽格湖口 GPIB OneCycle 要強制切 ASM
AnsiString sOneCycleFinishShowMsg="";                                           //Sam 20221103 : OneCycle 完後顯示訊息
bool bInitialStartIndexCheckDone=true;                                          //Sam 20221214 : 當機台 Initail Start 時需要先做 Index Check
const double HeatGunLimit = 215;
const double TemperatureFuseLimit170 = 165.0;
const double TemperatureFuseLimit200 = 200.0;
const double TemperatureFuseLimit250 = 250.0;
const double ChamberTempLimit = 165;                                               //Ifor 20230328 add: Chamber Temp Limit 165
//double TempFuseLimitType=TemperatureFuseLimit170;
double TempFuseLimitType=0.0;
int iE1Count=0;                                                                 //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
int iE2Count=0;
int iE3Count=0;
int iENotDefinedCount=0;

int iByLotE1Count[5]={0, 0, 0, 0, 0};                                           //JerryYang 20230322 : Lot summary要計算各類型ERR的數量
int iByLotE2Count[5]={0, 0, 0, 0, 0};
int iByLotE3Count[5]={0, 0, 0, 0, 0};
int iByLotENotDefinedCount[5]={0, 0, 0, 0, 0};

bool bSuperHotTempNoChkHumidity = true;                                         //Hmy 20190625 Add超高溫，不判斷濕度    //Ztex 2023.04.13 Add HT-1032 IO ==>
bool bSuperHotTempChangeTemp[3] = {false, false, false};                        //Hmy 20190625 Add超高溫，不判斷濕度
int  iFixDoorOpenTime=0;                                                        //Hmy 20180827 Add Function,fix area frosting protection
double dFixAreaDewPoint=0.0;                                                    //Hmy 20180827 Add Function,fix area frosting protection
bool bDelayTimeAfterFixDoorOpen  =false;                                        //Hmy 20180827 Add Function,fix area frosting protection
int  iStepOfDelayAfterFixDoorOpen=-1;                                           //Hmy 20180827 Add Function,fix area frosting protection

bool bOverTempAlarm[8]={false,false,false,false,false,false,false,false};       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
bool biTriTempDefaultbyWork=false;                                              //kevin 20150507 存工作檔
int iATC51ThermoCount_SH1 = 1;                                                  //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
int iATC51ThermoCount_SH2 = 1;                                                  //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
int iATC51ThermoCount_HP1 = 1;                                                  //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
int iATC51ThermoCount_HP2 = 1;                                                  //Hmy 20210414 Add ATC系統溫控器數量(預設為1點，目前最多3點)
bool bDefrostKeepATCTemp        = false;                                        //Hmy 20170120 增加手動除霜功能(判斷用)
bool bAutoDefrost_Exec          = false;                                        //Hmy 20170120 增加手動除霜功能(判斷用)
bool bAutoDefrost_Done          = false;                                        //Hmy 20170120 增加手動除霜功能(判斷用)
bool bAutoDefrost_Halt          = false;
bool bAutoDefrost_ByMachineProduction_Req   = false;                            //Hmy 20170120 增加手動除霜功能(判斷用)
bool bAutoDefrost_ByMachineProduction_Finish= false;                            //Hmy 20170120 增加手動除霜功能(判斷用)
bool bAutoDefrost_ByMachineProduction_Termination = false;
bool bAutoDefrost_ByMachineProduction_Wait = false;
bool bAutoDefrost_ByMachineProduction_Force = false;
bool bAutoDefrost_ByMachineProduction_CountDownEnd = false;
bool bAutoDefrost_ByFormMainDeviceRefresh =false;
bool bDefrostNeedSendCommandToAtc = false;                                      //Hmy 20221016 Modify //Hmy 20220805 Add DEFROST Command
bool bDefrost_abnormalStatus = false;                                           //Hmy 20190614 Add Defrost Abnormal
bool bManualDefrost_Start     = false;                                          //Hmy 20190614 Add Defrost Abnormal
bool bSuperHotKitUseAmbient = true;                                             //Hmy 20190702 Add Check UltraHighTemperature kit
bool bSuperHotKitUseAmbient_TempUp2Down = false;                                //Hmy 20190702 Add Check UltraHighTemperature kit
bool bSuperHotKit_NowUsehine=false;                                             //Hmy 20190702 Add Check UltraHighTemperature kit
bool bAnyOpenDoorForceSwitchDryAir[5] = {false,false,false,false,false};
bool bChangeCheckDoorCheckTime =false;
int iCheckEmptyTrayHasIc=0;                                                     //pig 2011.11.29 CheckEmptyTrayHasIcForLoaderHasSkipOrTrayEnd
bool bInuptDoorOpend=false;                                                     //Hmy 20181120 Add Function,InArm area frosting protection
AnsiString asDockingAreaOpen_Function_OpenCloseDate[3] = {"","",""};            //Hmy 20181120 Add Function,InArm area frosting protection
bool bcheckSuperHotTemperatureKitTiming[5] ={false,false,false,false,false};    //Hmy 20190702 Add Check UltraHighTemperature kit
bool bcheckSuperHotTemperatureKitFail = false;                                  //Hmy 20190702 Add Check UltraHighTemperature kit
int iOldTemperatue = -9999;
int  iSuperHotKit_CheckFinish =-1;                                              //Hmy 20191216 Modify 0 ->-1 Reset Soft init
AnsiString asBackupSystemDataCopmareDate[2] = {"", ""};                         //Hmy 20200407 Add Backup SystemData
bool bIonBarAirOnEcho       =false;                                             //Jeffrey 20180821 add for Ion Bar Auto Calibration
bool bIonBarAirOffEcho      =false;                                             //Jeffrey 20180821 add for Ion Bar Auto Calibration
bool bAllPosTempInRange     =false;                                             //Hmy 20191225 Add Automatic operation after waiting for the temperature Reached
bool bAllPosTempOkOverTime  =false;                                             //Hmy 20191224  Add Wait Temperature Stable Time
bool bReChkTempInRange      =false;                                             //Hmy 20191224  Add Wait Temperature Stable Time
int iCheckTemperaturetStableOverTimeCount = 0;                                  //Hmy 20191224  Add Wait Temperature Stable Time
int iSiteCheckTemperaturetStableOverTimeCount[40];                              //Hmy 20191224  Add Wait Temperature Stable Time
AnsiString asLowTemperatureOldNowTime = "";
bool bLowTempAutoDefrost        = false;                                        //Hmy 20210329
int  iLowTempAutoDefrostTask    = 1;                                            //Hmy 20210329
bool bColdTemperatureOverSetTimeAutoDefrost_TrayFeedFinish=false;               //Hmy 20210329
bool bExecuteFsatCoolDownFunction = false;                                      //Hmy 20191231 Add Fsat CoolDown Function
int  iExecuteFsatCoolDownFunctionStatus = -1;                                   //Hmy 20191231 Add Fsat CoolDown Function
int  iExecuteFsatCoolDownFunctionTask   = -1;                                   //Hmy 20191231 Add Fsat CoolDown Function
//Hmy 20201207 Add Kit Change Optimiaztion==>
bool bUseChangTriTempKitFunction = false;
int  iUseChangTriTempKitFunctionStatus = -1;
int  iUseChangTriTempKitFunctionTask   = -1;
int  iUseChangTriTempKit_IndexArm1ZPosmm = -100;
int  iUseChangTriTempKit_IndexArm2ZPosmm = -100;
//Hmy 20201207 Add Kit Change Optimiaztion<==
int iTemperatureAlarmSecondByOffsetUpdate = 10;
int BASE_HEATER=0;
int SHUTTLE_FLOODGATE=0;
int Tri_Temp_Machine=0;
int AirStream_Select=0;
int Tri_Temperature_MaxDegree=0;
int Tri_Temperature_MinDegree=0;
int TriTemperature_TotalChannel=0;
bool bUT150State[tcTotalCount];
bool bUT150HasUse[tcTotalCount];
int SetHeaterTemp_MaxOutSht=0;
int SetHeaterTemp_MaxIndex=0;
int SetHeaterTemp_MaxBase=0;
int Total_Compressor=0;
int RefrigeratorUserModeState[8];
bool bShowTriTempAlarm_Msg=false;
bool bCheckUse_AirStream=false;
bool bReadMotorParameter=false;
bool bALLTempFirstReady=false;
bool bAutoDefrost_OpenAirStream=false;
int INDEXDOORHEATER=0;
//Ztex 2023.04.19 Add HT-1032 TriTemp Function
int iAutoFormSize=0;                                                            //Jimmychiu 20231130 : 可開關Auto form size
int iACSmartCount=0;                                                            //Sam 20230111 : Smart Auto Clean
int iACSmartCount_CTF=0;                                                        //Sam 20240726 : AI Clean
int iACUseParam=0;
int iRunACSmart=0;
bool bRunACAdaptive=false;                                                      //Sam 20230914 : 自適應性良率監控
int iAdaptiveContsLowerCnt=0;                                                   //Sam 20230914 : 自適應性良率監控
AnsiString sACRecAlarmCode="";                                                  //Sam 20250916 : Alarm後需要清除資料才能Start
AnsiString sACRecEPortCode="";
bool bShowNoteCleanSocket=false;
bool bBarCoderAutoLogin=false;                                                  //Sam 20221101 : 使用 BarCoder 自動登錄
bool bBarCoderSetupFile=false;                                                  //Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。
bool bADAM6024FWIsNew[3]={false, false, false};                                 //Nickliu 20230314 Add Check Adam FW Is New
bool bHandlerChangeState=false;                                                 //Sam 20230511 : 機台資料變更後須上傳 FTP
//JerryYang 20220909 : add magazine
//==>
int iYRegNum=0;
int iAuto3MagazineIndex=-1;                                                     //Aut3目前釋放哪個 Mag Tray 0-13
int iWhichBuff=-1;
int iPickWhichBuff=-1;
int iWhichMag=-1;
int iPickWhichMag=-1;                                                           //Magazine1-14,  0-13
int iMagazineFullTrayIndex=-1;
bool bMagazineTrayFeedFinish=false;
int TRAY_MISS_CHECK_TIME = 100;
bool bMgzTrayCatcCynHomeNeedOff=true;                                           //pig 2016.06.29 MgzTrayCatchCynHome
bool bMagazineTrayFull=false;
bool bMagNeedChangeTray=false;
int iOutArmWhichAuto=-1;
int iFixToWhichAuto=-1;                                                         //Steven 20260202 : for ATK AMR
bool bAuto3HasCover=false;
bool bIsAuto3ReceiveCover=false;
bool bDoSelectMagTrayOut=false;
bool bSelectMagNeedOut[14]={false,false,false,false,false,false,false,false,false,false,false,false,false,false};
bool bInitCheckMag=false;
bool bSetMagTopBin=false;
bool bClearMagTopBin=false;
int  iTopWhichBin=0;
bool bChaneMagTrayflag=false;
bool bMagCatchTrayfalg=false;                                                   //JerryYang 20221121 : AUTOI3 Magazine還在夾TRAY的旗標
bool bMagGetNewTrayflag=false;
int iMagChangeStep=0;
//<==
//JerryYang 20220909 : add magazine
int i1x2_4UseACEGPicker=0;                                                      //Steven 20230530 : 1x2_4改用Row A
AnsiString CleanKitRecord[50][50];                                              //Sam 20230619 : 新增 Clean吸放時間 Log
bool bIdleNeedCheckSafeDoor[4][64][32][8];                                      //Steven 20230704 : add bypass idle check safe door
bool bOutArmTrayDuplicateErr[9]={false, false, false, false, false, false, false, false, false};
int iTotalXPitchStep    =3;
int iMaxXPitch          =12000;
int iMinXPitch          =4000;
int iMaxXPitch4Pick     =12000;
int iMinXPitch4Pick     =4000;
int iMaxXPitch2Pick_13  =8000;
int iMinXPitch2Pick_13  =2667;
int iMaxXPitch2Pick_14  =12000;
int iMinXPitch2Pick_14  =4000;
int iHPXPos=0;
int iHPYPos=0;
bool bOutarmDropError=false;                                                    //jou 20241024 : out arm drop error show alarm提醒檢查出Tray是否疊料

const int iEtherCatRing=1;                                                      //Sam 20230707 : EtherCAT Shuttle sensor
bool bEtherCatMastCardError=false;
unsigned int uiDevhand=0;
unsigned long m_dwDevNum=0;

bool bOCRRunTime=false;
bool bQUERYTJ=false;
bool bATCAlarm=false;
AnsiString asLotID_ByFile="";                                                   //KenHsieh 20230727 : 更改工作檔與資料 By NetFile
AnsiString asStation_ByFile="";
AnsiString asType_ByFile="";
AnsiString asState_ByFile="";
AnsiString asProductionByFileNamePath="";
int iDynamicThresholdNum=0;                                                     //JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket )

int DOUBLE_BELT_MODE = 0;                                                       //Ztex 2023.12.13 Add Double Belt Mode
//Kevin 20231209 : for ASE KH
//===>
bool bColorFirst=false;                                                         //kevin 20221025 load new color tray
bool bLoadBFBackTray=false;                                                     //KenHsieh 20230325 : CleanOut時Loader BF 退Tray
bool bAutoUpOK[3];                                                              //kevin 20230331 Auto 1 23 上升中不能放tray
bool bEPDetect=false;                                                           //kevin 20230608 index check 流量計是否漏氣
AnsiString asInarmSkipICPos="";                                                 //KenHsieh 20230707 : No9 pickup error後，紀錄G17功能所記錄點位之Log
AnsiString asInarmSkipICSucker="";                                              //KenHsieh 20230907 : G17功能所記錄最終點位之Log改為"all吸嘴:all Pos"
AnsiString asInarmSkipICPos_No9="";                                             //KenHsieh 20230707 : No9 pickup error後，紀錄G17功能所記錄點位之Log
bool bContinueFailIndexUp=false;                                                //kevin 20230807 錯誤 ARM 需上升避免結霜
bool bD52ArmUp_ConsecutiveFail=false;                                           //KenHsieh 20230809 : 發socket連續性異常前，Indexarm需先上升
bool bSoftwareStart=false;                                                      //kevin 20230812  軟體剛開啟
bool bAutoReceTray_SECS=false;                                                  //KenHsieh 20231016 : ASEKH_K1 & K3 補上蓋與空盤事件流程
bool bCheckOpenDoor=true;                                                       //kevin 20231122 check door open close log
//<==
//Kevin 20231209 : for ASE KH

bool Enable_PLCSafety_IO=false;                                                 //Austin 20190531.01 增加PLC_IO模組及安全相關IO.
int iPLCSafetyVer=0;                                                            //joe 20220406.01 debug safe plc
int iWhichShtPickFor=0;
int iPickFromPlateMode=0;
int iEnabledSiteCount=0;                                                        //Steven 20231220 : 牛信保說要開site數量SVID 1250
bool bInitailQtyByLowYield=false;                                               //Sam 20230728 : 發生 LowYield 後重新 Start 清除 Sort Count 和 contact counterKinds
bool bOLPSetCategroy=false;                                                     //Sam 20230711 : 修正 OLP SetCategroy 異常
AnsiString sB03RunData="";                                                      //Sam 20231115 : PTI 新增 Tester report
AnsiString sB03StartTime="";                                                    //Sam 20231115 : PTI 新增 Tester report
bool bWaitOutArmCheckCylin=false;                                               //jou 20240131 : 修正out arm 與 auto tray互卡衝突hang up
int  iInZHomeCnt=0;
int  iOutZHomeCnt=0;
AnsiString sStackBinTemp[eTrayCount];                                           //JerryYang 20231218 : P53防混功能
bool bNoitceFixTray=false;

int iMagazineStatus=0;                                                          //Ifor 20231130 add: 0: Normal 1:Full Tray 2:Manual put tray
bool bOneCycleDoInitialTemp=false;
bool bDoAfterOpenSafeDoor3NeedDelay=false;                                      //Ifor 20220413 add: SafeDoor 3 Open Ovet SetTime Need Delay
bool bSECSGEMAutoclean=false;                                                   //Ifor 20220414 add SECS/GEM AUTO Clean
bool bBinDispAlarm=false;                                                       //Ifor 20220714 add:Bin Disp 異常報警 每次Onecycle 檢查一次
int iTrayDetectCount=0;                                                         //Ifor 20221117 add:Tray arm移動報警偵測次數
bool bEnableAutoMotive=false;                                                   //Ifor 20221024 add: KLT Auto Motive Function
bool bNeedAutoHeight=false;                                                     //Ifor 20221208 add: KLT 要求更換工作檔需要重新Auto Height 才可執行Contact Mode
TDateTime tAlarmNoteStartTime=0;                                                //Ifor 20230330 add:Show Alarm Note Init Temperature offset function
bool bShowAlarmNote=false;                                                      //Ifor 20230330 add:Show Alarm Note Init Temperature offset function
bool bCheckATCTemp=false;                                                       //Ifor 20230410 add: 按下Start確認ATC溫度是否達到設定
bool bOneKyePowerSaving=false;                                                  //Ifor 20230612 :add 一鍵Power Saving
int iAutoHasHod[6];                                                             //Ifor 20230817 add: 退Tray時有HasNullIC報警提示
bool bAQL_Sorting_Finish=false;                                                 //Ifor 20231124 add:避免AQL Mode 誤報空穴問題
AnsiString asAutoCleanEven="";                                                  //Ifor 20210527 add: Auto Clean Start Even
int USE_STM_Function=0;                                                         //Ifor 20201113 add: STM Function
int USE_BU5_Function=0;                                                         //Ifor 20210819 add:LYEC BU5 使用BU3程式版本
int iRTC_CCD_NG[4];                                                             //Ifor 20210203 : add CCD NG Result  //Ifor 20210422 add:RTC CCD 2 => 4
bool bStartAQLSortMode=false;
int iAQLBin=0;
int iAQLCount=0;
bool bStartAutoAdjustLight=false;                                               //Ifor 20210408 add:Barcode 自動調整光源
bool bSetAQLSortMode=false;                                                     //Ifor 20210513 add: SECS GEM Set AQL Count
bool bBarcodeNeedAutoAdjust[4];                                                 //Ifor 20210531 add: Barcode Auto Adjust Light
bool bErrBinAlm=false;
bool bStartCheckZ=true;
int iMagazineCheckZPos=0;
bool bSmartSetupAutoHeight=false;
bool bSmartSetupZCalibration=false;
bool bSmartSetupInOutArmSearchArea=false;
bool bSmartSetupUseSingleZ=false;
bool bSmartSetupSHValue=false;
bool bMagazineGetNewTray=false;
int iSECSGEM_ConsecutiveFailureAlarm=0;                                         //Ifor 20240430 add:secs gem cmd Index Yiel Fail 0:Normal 1:收到命令 2:Index 上升
bool bDoInitialAutoClean=false;                                                 //Ifor 20240604 add:Auto Clean Inital Contact Count
AnsiString asMagazineBinFull="";                                                //Ifor 20240722 add:
bool bResetMagGetTime=false;
bool bMN200Error=false;                                                         //Ifor 20240821 add:KYEC要求MNet異常需清料重開程式
bool bDoROILearning=false;                                                      //Ifor 20240919 add

AnsiString asSupplementBin="";
AnsiString asLDUnLDCheckAGV="";
AnsiString asLDUnLDFinsihAGV="";
bool bLoaderActionFlag[3]={false, false, false};
bool bUnLoaderActionFlag[3]={false, false, false};
bool bAMRReceiveAGVStart=false;                                                 //Eastsun 20260515 F009 KYEC AMR 3 receive flags
bool bAMRReceiveStart=false;                                                    //Eastsun 20260515 F009 KYEC AMR 3 receive flags
bool bAMRReceiveLoaderTotalTray=false;                                          //Eastsun 20260515 F009 KYEC AMR 3 receive flags
int iSECSSetTrayCount=0;                                                        //Eastsun 20260515 F020 AMR LoaderAction dependency
bool bAutoChangingWarn[3] = {false, false, false};                              //Eastsun 20260513 :[0]=Auto1, [1]=Auto2, [2]=Auto3 旗標
int  iAutoColorBak[3]     = {2, 2, 2};                                          //Eastsun 20260513 :預設 ColorMap[2]=clGreen

bool bLoaderSECSActionFlag[3]={false, false, false};
bool bUnLoaderSECSActionFlag[3]={false, false, false};

bool bLoaderLockActionFlag[3]={false, false, false};
bool bUnLoaderLockActionFlag[3]={false, false, false};

bool bDummyRunPosCheck=false;
bool bDummyRunPosCheckFinish=false;
bool bDummyRunPosCheckCatchTray[6]={false, false, false, false, false, false};
bool bNowUseArmSuck[2][2][8];
bool bNeedCheckShuttleSensor=false;
int USE_GPIBLOGBYDAY=0;                                                         //Ifor 20241029 add:GPIB LOG BY DAY
bool bATC_EnableSiteMap[32];                                                    //Ifor 20241105 add:ATC Enable Site Map
bool bGPIBLOGBYDAY=false;                                                       //Ifor 20241029 add:GPIB LOG BY DAY
bool bDoD80Function=false;                                                      //Ifor 20241202 add:避免[D80]無限Index Check 與未完成顯示One Cycle Finish
bool bInitialAutoCleanTimeOut=false;                                            //Ifor 20241212 add:避免Auto Clean 暫停過久Start後報警
bool bCustomerForMTKVersion=false;                                              //yunghsin 20241130 Add MTK File Name
AnsiString anSoftwareVersion="";                                                //yunghsin 20241130 Add MTK File Name
bool bHasOpenMagDoor=false;                                                     //JerryYang 20241225 : 暫停狀態開Magazine安全門
AnsiString anGroundManLog="";                                                   //Ifor 20250123 KYEC FTP UP Load GroundMan Log File
int TROLLEY_USE_IO_TYPE=0;                                                      //Ifor 20250211 : add TROLLEY USE IO TYPE
int iRTCErrorCount=0;                                                           //wei 20221222 RTC ARM Error
TQPF_Timer tRTCErrorTimeOut;                                                    //wei 20221222 RTC ARM Error
bool iRTCErrorSend=false;                                                       //wei 20221222 RTC ARM Error
bool bCheckThirdPos=false;                                                      //wei 20221222 RTC ARM Error
AnsiString anMultiEPData="";                                                    //Ifor 20250417 add:Multi EP Data
bool bOutArmIonFanGiveWay=false;
bool bLoaderCocverRemove=false;

bool bByPassAutoLock=false;
int iLoaderTrayCountCal=0;
int iLoaderTrayCountAMRCal=0;

bool bReflashTrayCount=false;
bool bNeedWaitTemp=false;

int USE_COVER_TRAYID=tCIDNotUse;

AnsiString asBundleTrayID[ePortTotal];                                          //JerryYang 20240318 : add
bool bNeed1DCoverTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};
bool bNeedCoverTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};

bool bHasCoverTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};   //JerryYang 20240318 : AUTO1-6, FIX1-6
bool bHas1DCoverTray[MAX_AUTO_TRAY]={false, false, false, false, false, false};

int iBundleIn=12;
int iBundleOut=12;
AnsiString sBundleEndInfo="";
AnsiString sProcessEndInfo="";
AnsiString sUnloadBundleID="";
AnsiString sTotalLotID="";
AnsiString sCoverTrayID="";
AnsiString sCIDOnTrayArm="";
AnsiString sFixBundleID[MAX_FIX_TRAY]={"", "", "", "", "", ""};
bool bMustCoverIDTray=false;
int iBundleIDLength=9;
bool bI22_NeedHomeDelay=false;
bool bRTCAutoTuning;                                                            //Sam 20240711 : RTC Auto Tuning
bool bReadLotInfoFromART=false;                                                 //Steven 20240830 : 讀取lot info的位置
bool bSuckingFlagZ1=false, bSuckingFlagZ2=false;                                //Steven 20240916 : index下降到shuttle吸放料
bool bTJtoTCWait=false;                                                         //wei 20251002 : TJ轉回TC等待
double dGPIBATCOffset[32]={0.0};
bool bGPIBOffsetCommand=false;
bool bNeedOneCycleByContactAlm1=false;                                          //Sam 20241226 : Contact Alarm 需要先做 OneCycle
bool bNeedOneCycleByContactAlm2=false;
bool bNeedOneCycleByContactWar1=false;
bool bNeedOneCycleByContactWar2=false;
bool bNeedOneCycleByPickerLifeAlm=false;                                        //AI(ht9045-config) 20260521 (RogerYang) : SCC吸嘴壽命報警OneCycle優化
bool bTriggerRTC_AutoSTD=false;                                                 //JerryYang 20240829 : SPIL訓永 要求手動觸發RTC AUTO STD
AnsiString sWhiteListLotID="";                                                  //JerryYang 20241104 : 支援2DID白名單功能
AnsiString sWhiteListProcess="";
int iBackupOneCycle=0;                                                          //JerryYang 20241015 : 修正觸發One cycle後又執行auto clean, 完成後會繼續跑
int iMN200_ErrorCode=0;
bool bReduceTrayExitTime=false;                                                 //Ztex 2024.03.25 Add Reduce tray exit time
bool bReduceTrayExitTime_TrayFeed[6];                                           //Ztex 2024.03.25 Add Reduce tray exit time
bool bServoOnOff=false;                                                         //ChungHung 20230718 add for Safe plc
bool bByPassDoor=false;                                                         //Ztex 2024.05.13 bByPassDoor
bool bTT_SetSpeed_Check=false;                                                  //Ztex 2024.08.11 Add Test Time Set Speed
bool bWaiteKeepRespond=false;                                                   //kevin 20231215 等待eKeep 回應
bool bAutoReceTrayAGV[MAX_AUTO_TRAY]={false};                                   //kevin 20240423 add 退料TRAY call AGV
bool bReceiveProcess[MAX_AUTO_TRAY];                                            //kevin 20240514 Auto 1 2 3
bool bUseSocketHeating_Wait=false;                                              //Ztex 2024.09.07 Add Use Socket Heating
int  iUseSocketHeating_Time=0;                                                  //Ztex 2024.09.07 Add Use Socket Heating
int  iAStreamErrorCompressOnecycle=0;                                           //Ztex 2024.10.01 Add AStream Error Compress Onecycle
bool bPickLoaderFromBack=false;
bool bRefreshCount=false;                                                       //Stteve 20250219 : 清除數量後的重整要delay一下
int iAuto_CarCarDeviceCnt[MAX_AUTO_TRAY]={0, 0, 0, 0, 0, 0};                    //Sam20250331 : Unloader Auto 退出時上面的 IC 數量。
int iSECS_Loader_Buffer_State=-1;                                               //Sam 20250605 : Loader buffer status change event
bool bWaitRotateFinish=false;                                                   //Ifor 20241015 add: 等待 Rotate 完成
int iAMD_Function=0;                                                            //Ifor 20231222 add TFAMD Function 0:AMD(新加坡、美國) 1:TFAMD(蘇州) 2:TFAMD(馬來西亞)
int iChangeFileHasErr=0;                                                        //Ifor 20240314 add:GPIB 切換工作檔時有異常
AnsiString asChangeSetupFileName="";
bool bHasFTPDownload=false;                                                     //Ifor 20240422 add:FTP 工作檔下載
AnsiString asReadTCWater[ATC_HEAD_COUNT]={""};
AnsiString asReadDynamicPID[ATC_HEAD_COUNT]={""};
AnsiString asSetDynamicPID[ATC_HEAD_COUNT]={""};
bool bGPIBAutoHeightMove=false;
AnsiString asGPIBAutoHeight=0;
int iGPIBAutoHeightCount=0;
bool bGPIBAutoHeightPass=false;
AnsiString asGPIBAutoHeightLimit="";
AnsiString asGPIBAutoHeightZPos="";
AnsiString asGPIBAutoHeightTorque="";
AnsiString asControlModeGPIB="";

int iTrayEnable[3]={0, 0, 0};

bool bE84LoaderActionflag[3]={false, false, false};
bool bE84UnloaderActionflag[3]={false, false, false};
bool bAOIConsecutiveFailFlag=false;
bool bAOIConsecutiveFailPictureFlag=false;
bool bAOIFailCountFlag=false;
int iAOILotCount=0;

bool bInArmLaserActionflag[2]={false, false};
bool bOutArmLaserActionflag[2]={false, false};

// AI(W906-cContactLeaf) 20260721: un-gated from the InitialMemory tail block below (was
// collateral-gated there by the W0-TAIL "gate to EOF" sweep even though it is a plain global
// definition with no state-machine/app-global dependency, same as bOutArmLaserActionflag above).
// Needed live now: TfContactShim::IsRun2DCheck() (atester_shims.cpp) calls the real
// ComputeIsRun2DCheck() (cContact.h/.cpp), which reads this global -- the declaration
// (cmydef.h:5830 extern bool bRun2DCheck;) already existed, but its definition was unreachable
// (link error) while gated. i2DMAPCHKSTEP (the next line in the gated block) is left gated;
// nothing in this wave needs it.
bool bRun2DCheck;                                                               //JerryYang 20250220 : 2DID硬體順序檢查功能

//------------------------------------------------------------------------------
//AI(W0-TAIL) 20260626: TODO(W6) -- InitialMemory/GetTotalYield_* reach state
//  machines + app globals + TMyStringList bodies. Gated to EOF.
// AI(W906-PT-W5b-integrate) 20260809 UPDATE: now split in THIRTEEN places, not six.
//   PT-W5b's completion of SECSGEM/uHGemHT9045.cpp binds SEVEN MORE of this block's plain
//   globals, and they surfaced the same way the first six did -- as undefined references
//   at link, not as anything a compile would catch:
//       :5970 bBindChkNG      :5973 bUnbindChkNG    :5976 bTRAYCHKNG
//       :6035 bNeedDoRunCheck :6041 bSECSPause      :6048 iSV_ErrBinCnt
//       :6051 iSVByBinCount
//   (line numbers as they stand AFTER the sandwiches, re-read from the file rather than
//   computed -- my first draft of this list guessed them and every one was wrong, which
//   is the same fabricated-citation failure this project keeps catching in audit agents.)
//   All seven were checked for a rival live definition first -- all seven clean -- so the
//   running total is 13 un-gated of the ~93, and the 16 that WOULD collide are still
//   untouched.  Everything the PT-W4 note below says still applies unchanged.
//
// AI(W906-PT-W4-integrate) 20260809: THIS GATE IS NOW SPLIT IN SIX PLACES, and the
//   reason is that its own stated premise only ever covered part of what it encloses.
//   The premise -- "function bodies depend on untranslated globals/state machines" -- is
//   true of InitialMemory / GetTotalYield_* at the head of the block, and is true of
//   NOTHING in the ~93 plain file-scope global DEFINITIONS in its tail: those are pure
//   storage with no dependency on any state machine.
//   SECSGEM/uHGemHT9045_EC.cpp (wave PT-W4) binds six of them, so each of those six is
//   individually un-gated below with an `#endif` / definition / `#if 0` sandwich, which
//   keeps it exactly where golden put it instead of moving it out of declaration order:
//       :5936 iBundleDieQty   :5986 sSiteLayoutASECL   :5987 sSiteMapASECL
//       :5988 sBinMap         :6003 iSECS_HotPlateSeclect   :6004 bSECS_REALTIMECCD
//   Each was checked for a rival live definition elsewhere in the tree before ungating;
//   all six are collision-free.  THAT CHECK MATTERS: 16 of the 93 are NOT -- iPortStatus
//   (:5950), iThisPortNo (:5948), bAskStopPort (:5946) and 13 more already have live
//   definitions in acatchtray_shims.cpp / canary_support.cpp / asendic_Loader.cpp /
//   AutoClean.cpp, so ungating the tail wholesale would manufacture 16 duplicate-symbol
//   errors.  Splitting the remaining 77 out properly (and deciding which of the 16 shim
//   homes should yield) is its OWN wave, deliberately not done here.
#if 0 // TODO(W6): function bodies depend on untranslated globals/state machines
void InitialMemory()                                                            //Steven 20160319 : 初始化數值   // AI(W906-INITMEM) 20260927: gated REFERENCE copy -- the live definition is cmydef_InitialMemory.cpp
{
    ZeroMemory(bCleanKitSuckDuplicateErr, sizeof(bCleanKitSuckDuplicateErr));
    ZeroMemory(asGPIBTempShow, sizeof(asGPIBTempShow));
    ZeroMemory(ContinuousFailSKTCount, sizeof(ContinuousFailSKTCount));
    ZeroMemory(ContinuousFailARMCount, sizeof(ContinuousFailARMCount));
    ZeroMemory(ContinuousFailSKTCount_AutoClean, sizeof(ContinuousFailSKTCount_AutoClean));
    ZeroMemory(ContinuousFailARMCount_AutoClean, sizeof(ContinuousFailARMCount_AutoClean));
    ZeroMemory(SpecialBinContinuousFailSKTCount, sizeof(SpecialBinContinuousFailSKTCount));
    ZeroMemory(SpecialBinContinuousFailARMCount, sizeof(SpecialBinContinuousFailARMCount));
    ZeroMemory(iAutoCleanByBinCount, sizeof(iAutoCleanByBinCount));
    ZeroMemory(iAutoCleanBySiteCount, sizeof(iAutoCleanBySiteCount));
    ZeroMemory(iLoadPersentCT, sizeof(iLoadPersentCT));
    ZeroMemory(iLoadCountCT, sizeof(iLoadCountCT));
    ZeroMemory(fTrayYield, sizeof(fTrayYield));
    ZeroMemory(fIntervalYield_YieldHistory, sizeof(fIntervalYield_YieldHistory));
    ZeroMemory(iNeedBarcodeCount, sizeof(iNeedBarcodeCount));
    ZeroMemory(iBarcodeDuplicate, sizeof(iBarcodeDuplicate));
    ZeroMemory(iBarcodeErrorCount, sizeof(iBarcodeErrorCount));
    ZeroMemory(iBarcodePassCount, sizeof(iBarcodePassCount));
    ZeroMemory(iBarcodeAutoRetry, sizeof(iBarcodeAutoRetry));
    ZeroMemory(bBarcodeFirstAutoRetry, sizeof(bBarcodeFirstAutoRetry));
    ZeroMemory(bLowYieldCloseSite, sizeof(bLowYieldCloseSite));
    //ZeroMemory(iBinTray, sizeof(iBinTray));                                   //kevin 20170223 (wei) 不使用
    ZeroMemory(iATC_TempIndex, sizeof(iATC_TempIndex));
    ZeroMemory(bSiteHasTurnOn, sizeof(bSiteHasTurnOn));                         //Steven 20170302 (wei) : 確認哪個Site有開, 從1開始~32
    ZeroMemory(iInArmPutIcToSH, sizeof(iInArmPutIcToSH));                       //Ifor 20171121 : Test 查看異常資料
    ZeroMemory(iMagneticScalePos, sizeof(iMagneticScalePos));                   //Ifor 20180227 : 初始值歸零
    ZeroMemory(dATCTempAdjustmentOffset, sizeof(dATCTempAdjustmentOffset));     //Ifor 20190215 : add ATC 使用 三點校正功能
    ZeroMemory(LOAD_Z_USE_MOTOR, sizeof(LOAD_Z_USE_MOTOR));                     //Steven 20190813 : 入Tray改用步進馬達
    ZeroMemory(LOADUNLOAD_USE_CASSETTE, sizeof(LOADUNLOAD_USE_CASSETTE));

    ZeroMemory(iByBinTotal, sizeof(iByBinTotal));
    ZeroMemory(bUnloadHasBin, sizeof(bUnloadHasBin));
    ZeroMemory(iTrayLastBin, sizeof(iTrayLastBin));
    ZeroMemory(bPickLoaderDuplicateErr, sizeof(bPickLoaderDuplicateErr));
    ZeroMemory(bPickHPDuplicateErr, sizeof(bPickHPDuplicateErr));
    ZeroMemory(bTryPickHPDuplicateErr, sizeof(bTryPickHPDuplicateErr));
    ZeroMemory(dTorqueArray, sizeof(dTorqueArray));

    ZeroMemory(iSLT_HeadContactCount, sizeof(iSLT_HeadContactCount));           //Ifor 20191218 : add KYEC 要求 同SLT輸出表格
//    ZeroMemory(iTrayXAutoSitemapping, sizeof(iTrayXAutoSitemapping));         //Ifor 20210524 add:mykitsuck移至cmydef
//    ZeroMemory(iTrayYAutoSitemapping, sizeof(iTrayYAutoSitemapping));         //Ifor 20210524 add:mykitsuck移至cmydef
    ZeroMemory(dIndexZOffset, sizeof(dIndexZOffset));                           //Ifor 20210114 add: Index Z Offset
    ZeroMemory(bFTestSuckError, sizeof(bFTestSuckError));                       //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    ZeroMemory(bBTestSuckError, sizeof(bBTestSuckError));                       //Ifor 20200622 add:Index Pick Shuttle Err Need Purge
    ZeroMemory(bATC_EnablesChannel, sizeof(bATC_EnablesChannel));
//    ZeroMemory(bFIFOStep, sizeof(bFIFOStep));                                 //Steven 20180305 : 一次跑一顆的FIFO版本

//    ZeroMemory(iBufferDataType, sizeof(iBufferDataType));
//    ZeroMemory(asBufferCassetteID, sizeof(asBufferCassetteID));
//    ZeroMemory(asBufferLotID, sizeof(asBufferLotID));
//    ZeroMemory(asViewMessage, sizeof(asViewMessage));

    //JerryYang 20181011 (Steven) : SiteData改成全域變數
    SiteData[SingleSite].SetData(1, 1);
    SiteData[DualSite].SetData(2, 1);
//    SiteData[DualSiteBS].SetData(2, 1);
    SiteData[TriSite1X3].SetData(3, 1);
    SiteData[QualSite1X4].SetData(4, 1);
    SiteData[DualSite2x1].SetData(1, 2);
    SiteData[QualSite2X2].SetData(2, 2);
    SiteData[QualSite2X2N].SetData(2, 2);                                       //Frank 20200520 2X2NN Mode
//    SiteData[QualSite2X2BS].SetData(2, 2);
    SiteData[_6Site2X3].SetData(3, 2);
    SiteData[_6Site2X3N].SetData(3, 2);                                         //Steven 20220425 : 2X3NN Mode
    SiteData[_8Site2X4].SetData(4, 2);
    SiteData[_10Site2X5].SetData(5, 2);
    SiteData[_12Site2X6].SetData(6, 2);
    SiteData[_16Site2X8].SetData(8, 2);
    SiteData[_16Site4X4].SetData(4, 4);                                         //Sam 20190226 : 16Site4X4
    SiteData[_32Site4X8N].SetData(8, 4);
    SiteData[_32Site4X8M].SetData(8, 4);
    SiteData[_8Site1X4].SetData(4, 1);
    SiteData[_8Site2X4N].SetData(4, 2);                                         //Wei 20231211 : 2X4NN Mode
    lHandlerStopTime.LatchCycleTime(true);
    ZeroMemory(iRTC_CCD_NG, sizeof(iRTC_CCD_NG));                               //Ifor 20210203 : add CCD NG Result
    ZeroMemory(bBarcodeNeedAutoAdjust, sizeof(bBarcodeNeedAutoAdjust));         //Ifor 20210531 add: Barcode Auto Adjust Light
    ZeroMemory(iAutoHasHod, sizeof(iAutoHasHod));                               //Ifor 20200803 add:改陣列處理

    ZeroMemory(bIdleNeedCheckSafeDoor, sizeof(bIdleNeedCheckSafeDoor));         //Steven 20230704 : add bypass idle check safe door
    ZeroMemory(iAutoHasHod, sizeof(iAutoHasHod));                               //Ifor 20200803 add:改陣列處理
    ZeroMemory(bNowUseArmSuck, sizeof(bNowUseArmSuck));                         //Ifor 20200803 add:改陣列處理
    ZeroMemory(bATC_EnableSiteMap, sizeof(bATC_EnableSiteMap));                 //Ifor 20241105 add:ATC Enable Site Map
    ZeroMemory(RefrigeratorUserModeState, sizeof(RefrigeratorUserModeState));   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    ZeroMemory(bAlignmentChangeUnloadTray, sizeof(bAlignmentChangeUnloadTray)); //Steven 20240428 : Add for HT9011 AOA
}
//------------------------------------------------------------------------------
bool bRunArmSuckZAuto=false;                                                    //Jimmychiu 20240712 : Auto Calibrate Suck Z height
bool bOncycleFinishRunArmSuckZAuto=false;
#endif // AI(W906-FRW-S70) 20260926: ungate 4 definitions (golden cmydef.cpp:5914-5917) -- AutoCalSuckZ.Data 讀寫（FileRW/AutoCalSuckZ.cpp elData）與門面 fProductionInfo->EnableIn/OutArmAutoCalSuckZ（forms/fProductionInfo.cpp）要它們；全樹沒有別的定義（20260926 grep，含 tests/）；上面 bRunArmSuckZAuto／bOncycleFinishRunArmSuckZAuto 是量測流程旗標（golden 0 個讀寫者），仍關著
bool bEnableInarmSuckZAuto=false;
bool bEnableOutarmSuckZAuto=false;
int iInArmZHeightDiff[MAX_ARM_Row][MAX_ARM_Col]={0};
int iOutArmZHeightDiff[MAX_ARM_Row][MAX_ARM_Col]={0};
#if 0 // ...resume the TODO(W6) gate -- AI(W906-FRW-S70) 20260926: 這兩個 uPoint2D 仍關著：建構子本體在 Public/HTEditList.cpp（ht9045_sm），本檔在 ht9045_globals（只連 vclcompat），在這裡定義會讓抽到 cmydef.o 卻沒連 ht9045_sm 的程式缺 uPoint2D::uPoint2D() —— 改定義在 FileRW/AutoCalSuckZ.cpp（原因見該檔檔頭）
uPoint2D InArmAutoCalSuckZPoint;
uPoint2D OutArmAutoCalSuckZPoint;
#endif // AI(W906-FRW-S70) 20260926: ungate 2 definitions (golden cmydef.cpp:5920-5921) -- 同上，AutoCalSuckZ.Data 的 iIn/OutArmSearchStartZ
int iInArmSearchStartZ;
int iOutArmSearchStartZ;
#if 0 // ...resume the TODO(W6) gate
//------------------------------------------------------------------------------
double GetTotalYield_double()   // AI(W906-TOTALYIELD) 20260927: gated REFERENCE copy -- the live definition is cmydef_TotalYield.cpp
{
    double dvalue=double(iSECSGEMPass+iSECSGEMFail);
    if(dvalue==0.0)
    {
        return 0.0;
    }
    else
    {
        return (double(iSECSGEMPass)/dvalue)*100.0;
    }
}
//------------------------------------------------------------------------------
AnsiString GetTotalYield_Str()   // AI(W906-TOTALYIELD) 20260927: gated REFERENCE copy -- the live definition is cmydef_TotalYield.cpp
{
    return AnsiString().sprintf("%02.2f%",GetTotalYield_double());
}
//------------------------------------------------------------------------------
int i2DMAPCHKSTEP;                                                              //JerryYang 20250220 : 2DID硬體順序檢查功能

#endif // AI(W906-PT-W4-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045_EC.cpp binds it; see the note at this block's head
int iBundleDieQty=0;                                                            //JerryYang 20250224 : add
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5b-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045.cpp binds it; see this block's head note
bool bBindChkNG=false;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5b-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045.cpp binds it; see this block's head note
bool bUnbindChkNG=false;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5b-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045.cpp binds it; see this block's head note
bool bTRAYCHKNG=false;
#if 0 // ...resume the TODO(W6) gate
AnsiString sUnloadBin[MAX_UNLOAD_TRAY];
#endif // AI(W906-PT-W5c-integrate) 20260809: ungate -- csystem.cpp / bthermo.cpp / uHeaterThread.cpp bind bNeedReportBundleID; plain data, no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8); see this block's head note
bool bNeedReportBundleID[eTrayCount];
TMyStringList *slHanaTrayMap[eTrayCount];                                       //AI(W906-LOGOBJ-W7) 20260927 (St02-E): ungate ONE definition (W7=A) -- LogObjects.cpp W906_CreateLogObjects news them (golden 906_0625_Steven main.cpp:1545-1553); golden cmydef.cpp:5914
#if 0 // ...resume the TODO(W6) gate
bool bBackupAutoClean=false;                                                    //JerryYang 20250514 : fix 2DID與Auto clean衝突, 做完2DID Map再把auto clean旗標打true
//bool bVTestNoRTBin=false;                                                     //RogerYang 20250814 : 改到TrayAssignment下  //RogerYang 20250626 偉測不可複測bin功能
#endif // AI(W906-T5-W4E) 20260919: ungate ONE definition -- forms/fMesSystem.cpp:585 (TfMesSystem::NeedNoRTBinID, ACTIVE since FW3-MES1 20260827) binds bNoRTBinFixFlag; that TU was never linked by any target until T5/W4-E pulled it in via CheckVTENGmode, and the full build then failed with `undefined reference to bNoRTBinFixFlag` at .text+0x2919. Plain bool[3], no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8); same class as bTRAYCHKNG / bNeedReportBundleID above.
bool bNoRTBinFixFlag[3];                                                        //RogerYang 20250626 偉測不可複測bin功能
#if 0 // ...resume the TODO(W6) gate
bool bAskStopPort[ePortTotal];                                                  //JerryYang 20250521 : For AMR
bool bPortIsBusy[ePortTotal];
int iThisPortNo;
int iThisPortStatus;
int iPortStatus[ePortTotal];
int iLastPortStatus[ePortTotal];
#endif // AI(W906-FW-CTBS-integrate) 20260818: ungate ONE definition -- cShowBinSelect.cpp (landed this wave) binds sBinCode_ATK (ShowCategoryBin ATK bin-code table); plain AnsiString array, no NULL-global ctor risk (PT_CAMPAIGN_PLAN.md section 8)
AnsiString sBinCode_ATK[eTrayCount];
#if 0 // ...resume the TODO(W6) gate
bool bWaitingAMR=false;                                                         //RogerYang 20250617 Load已清空但還沒滿bundle，等料車來
bool bForceSendLoaderIsEmpty;                                                   //JerryYang 20250618 : add
bool bIsPlacingToBuffer;                                                        //JerryYang 20250828 : fix color誤退TRAY
bool bIsCatchingFromBuffer;                                                     //RogerYang 20260225 : JSCC防止夾tray的時候Color/Empty誤退，驗證中
bool bMaintanceMode=false;                                                      //Steven 20251007 : maintance mode for Hana   //AI(W906-SHOWERR) 20260929: still inside the TODO(W6) #if 0 above -- the live definition is at the end of this file
bool bDoRTCVerify=false;
bool bRenesasFTCTAlarm=false;                                                   //RogerYang 20250923 : 瑞薩FT-CT
bool bAutoRestartAfterFTCTAlarm=false;                                          //RogerYang 20251016 : 瑞薩FT-CT  紀錄90命令關掉時是否要恢復啟動
bool bReplyFTCTAlarm=false;                                                     //RogerYang 20251021 : 瑞薩FT-CT  FTCT發送違法指令回傳Error需要報警
AnsiString sFTCTAlarmStr="SysErr_FTCT_Alarm_Req_Error";                         //RogerYang 20251016 : 瑞薩FT-CT
bool bContinueMessageByReply71=false;                                           //RogerYang 20251107 : 瑞薩FT-CT Add Continue Form
bool bDestoryOnSht=false;                                                       //RogerYang 20251021 : 從inarm2搬過來
bool bBoatChangeCasset[3]={false, false, false};                                //Ifor 20251220 add:Boat Change
#endif // AI(W906-PT-W5c-integrate) 20260809: ungate -- csystem.cpp / bthermo.cpp / uHeaterThread.cpp bind bHasTjTemp; plain data, no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8); see this block's head note
bool bHasTjTemp;                                                                //JerryYang 20251124 : Tj control吃不同的溫度range
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W7d-integrate) 20260811: ungate -- ainarm9045.cpp bodies landed this wave are the FIRST consumers (60 undefined-reference lines, 2 distinct symbols). golden home cmydef.cpp:5937.
int iCheckShuttleSensor=0;                                                      //Ifor 20251204 add:After Home Need Check Shuttle Sensor
bool bChangeModeING=false;   // AI(W906-RSMODE) 20260927: ungate -- golden main.cpp:23730/:23822 (cbRunStartModeChange, RunStartMode.cpp EOF) is the only user (writes only, golden has no reader); plain data. golden cmydef.cpp:5939
#if 0 // ...resume the TODO(W6) gate
bool bFreezeHandler=false;                                                      //Ifor 20260119 add:Freeze Handler
#endif // AI(W906-PT-W5f-integrate) 20260810: ungate -- DoInArmAutoSiteMapping (csystem.cpp wave 2) reads it. Plain data, no ctor touching a NULL global (PT_CAMPAIGN_PLAN.md section 8).
//AI(ht9045-v906) 20260810: PT-W5f integrate -- bGPIBError has NO definition in this port:
// golden defines it at main.cpp:15151 and main.cpp is UNPORTED, so every user carries only
// a local `extern bool bGPIBError;` (atester.cpp:653 here, golden atester.cpp:608 and
// aTester_Front.cpp:65). csystem.cpp wave 2 pulled atester.cpp into the link and exposed it.
// Golden writes `bool bGPIBError=false;` -- zero-initialised, no ctor -- so hosting it beside
// the other machine globals is byte-for-byte golden behaviour. WHOEVER PORTS main.cpp:
// delete this line then, or it becomes a duplicate definition.
bool bGPIBError=false;                                                          // golden main.cpp:15151
bool bASMFirstTime=false;
#if 0 // ...resume the TODO(W6) gate
DWORD startASMTick=0;
DWORD endASMTick=0;
TDateTime tASMDateTime=0;
bool bFullSiteTestComplete=false;
bool bWaitASVFullSiteTestResult=false;
bool bFullsiteBackHP=false;
int iBackupWihchSht=0;
int iBackupWihchKit=0;
bool bFindPickICFail=false;
int iSystemUpTime=0;
#endif // AI(W906-PT-W5c-integrate) 20260809: ungate -- csystem.cpp / bthermo.cpp / uHeaterThread.cpp bind tUNDER_20A_Delay; plain data, no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8); see this block's head note
TQPF_Timer tUNDER_20A_Delay;                                                    //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
TQPF_Timer tUNDER_20A_SHT_Delay;                                                //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
const int iUNDER_20A_Delay_Sec=30;                                              //Jimmychiu 20220713 for TEMPCTRL_NEED_UNDER_20A delay
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W7d-integrate) 20260811: ungate -- ainarm9045.cpp bodies landed this wave are the FIRST consumers (60 undefined-reference lines, 2 distinct symbols). golden home cmydef.cpp:5954.
bool bAutoRetryFlag=false;                                                      //JerryYang 20220901 : ASE-CL pick up error時吸取邏輯要改找吸最左上角的
#if 0 // ...resume the TODO(W6) gate
AnsiString sSiteLayoutSiteMap="";
#endif // AI(W906-PT-W4-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045_EC.cpp binds it; see the note at this block's head
AnsiString sSiteLayoutASECL="";
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W4-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045_EC.cpp binds it; see the note at this block's head
AnsiString sSiteMapASECL="";
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W4-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045_EC.cpp binds it; see the note at this block's head
AnsiString sBinMap="";
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W7a-integrate) 20260810: ungate -- cinitial.cpp:9497 (landed this wave) assigns iWhoTrigerASV and this is its only definition. Plain int, no ctor touching a NULL global (PT_CAMPAIGN_PLAN.md section 8).
int iWhoTrigerASV=0;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5b-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045.cpp binds it; see this block's head note
bool bNeedDoRunCheck;
// AI(W906-FW-CTBS-integrate) 20260818: gate around these three DISSOLVED --
// cShowBinSelect.cpp (landed this wave) is their first real consumer
// (CaculateUPH/pause bookkeeping); plain ints, PT_CAMPAIGN_PLAN.md section 8.
int iPauseTime=0;
int iNetUPH=0;
int iGrossUPH=0;
bool bSECSPause=false;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5c-integrate) 20260809: ungate -- csystem.cpp / bthermo.cpp / uHeaterThread.cpp bind bHasChangePlate; plain data, no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8); see this block's head note
bool bHasChangePlate=false;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5b-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045.cpp binds it; see this block's head note
bool bDoInZTeach=false;   //Steven 20260925 (S12-C Offset_File): ungate -- FileRW/Offset_File.gen.inc (golden TfOffSet FormShow) clears both; plain bool, no ctor touching a NULL global (PT_CAMPAIGN_PLAN.md section 8)
bool bDoOutZTeach=false;
// AI(W906-ST-W1) 20260915: ungate ONE definition -- WebStart.cpp
// (TfMainWeb::StartFromWeb, golden main.cpp:4515 `fTrayMapping->
// IsRunDeviceRemainLaser() && bNeedDoRemainCheck==true`) binds it.
// 照 :6233 與 :6239 同一個 pattern：純資料、預設 false、沒有會碰 NULL 全域的
// ctor（PT_CAMPAIGN_PLAN.md section 8）。全樹只有這一個讀取點，而它讀到的是
// false，所以解閘前後行為相同 —— 這是連結可達性修正，不是行為變更。
bool bNeedDoRemainCheck=false;
int iSV_ErrBinCnt=0;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5b-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045.cpp binds it; see this block's head note
int iSVByBinCount[TEST_MAX_BIN];
#if 0 // ...resume the TODO(W6) gate
bool bTrayArmIsPickColor=false;
bool bNeedBigMsg=false;
#endif // AI(W906-PT-W4-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045_EC.cpp binds it; see the note at this block's head
int iSECS_HotPlateSeclect=0;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W4-integrate) 20260809: ungate ONE definition -- SECSGEM/uHGemHT9045_EC.cpp binds it; see the note at this block's head
bool bSECS_REALTIMECCD;
#if 0 // ...resume the TODO(W6) gate
bool bNeedRestartSW=false;
#endif // AI(W906-PT-W5f-integrate) 20260810: ungate -- DoInitialCylinderCheck (csystem.cpp wave 2) needs this flag. Plain data, no ctor touching a NULL global (PT_CAMPAIGN_PLAN.md section 8).
bool bInitialCylinderCheck=false;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W5f-integrate) 20260810: ungate -- DoInitialCylinderCheck (csystem.cpp wave 2) needs its task cursor. Plain data, no ctor touching a NULL global (PT_CAMPAIGN_PLAN.md section 8).
int iInitialCylinderCheckTask=1;
#if 0 // ...resume the TODO(W6) gate
#endif // AI(W906-PT-W7b-integrate) 20260810: ungate -- aTester_Rear.cpp's DoTestYRear (golden :5362-9232) and DoBTestSuckTestIC (golden :2222-3878), landed this wave, are the FIRST consumers; the gate's "nobody needs these" premise is now false (60 undefined-reference lines, 2 distinct symbols, measured 20260810). golden home cmydef.cpp:5978-5979.
int iP65QAReTestCount=0;                                                        //Ifor 20260407 add: [P65] ARM QA Mode ReTest Counter
bool bP65QAReTest=false;                                                        //Ifor 20260407 add: [P65] QA ReTest flag
bool bP65QAING=false;                                                           //Ifor 20260407 add: [P65] QA Mode QA in progress    //AI(W906-GB-P2c) 20260926: golden 912 cmydef.cpp:6017
#if 0 // ...resume the TODO(W6) gate
bool bWaitSECS=false;                                                           //KevinCheng 20250919 : Wait SECS
bool bEject=false;                                                              //JerryYang 20251020 : 渠梁半清機功能
#endif // AI(W906-PT-W5f-integrate) 20260810: ungate -- csystem.cpp:23652 (CheckAllAutoTrayEjectFinsh, landed this wave) actively reads bNeedEject[i], and this is its ONLY definition, so the link now requires it. Plain bool array, no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8). NOTE: asendic_Auto.cpp:354/:404 hold a MACRO SEAM (static W7L1A_bNeedEject + #define) that keeps THAT file on its own private copy -- tracked separately, do not assume this ungate reconnects it.
bool bNeedEject[MAX_AUTO_TRAY];                                                 //JerryYang 20251020 : 渠梁半清機功能
int iMagBin = 0;                                                                //Eastsun 20260515 F011 整合 (Magazine AMR shared)   // AI(W906-ARM3) 20260927: ungate -- DoOutArmPlaceToAuto（aoutarm9045.cpp 檔尾，golden :2525-3075）讀它；plain data. golden cmydef.cpp:5985
#if 0 // ...resume the TODO(W6) gate
bool bPurgeOutAllDevice=false;                                                  //JerryYang 20251020 : 渠梁半清機功能


//==> Eastsun 20260515 F012 整合 (AMR cmydef globals)
bool bNeedAMRMagazineUnload=false;             //a-side cmydef.cpp L5437
bool bAMRMagazineAutoReceive=false;            //a-side cmydef.cpp L5441

//<== Eastsun 20260515
int iATC_RecipeFileTransfer=0;                                       //Eastsun 20260522 add: ATC Recipe FileTransfer state
int ASE_OutTrayNum[eTrayCount];                                               //Eastsun 20260515 F011 整合:KYEC AMR output tray count
//AI(W0-TAIL) 20260626: close tail function gate
#endif // TODO(W6)

// AI(W906-MSTATE-P2b) 20260924: ungate TWO definitions -- ShowRunLed（ckernel.cpp:1604，golden ckernel.cpp:704-932）
//   是第一個活的讀者（連結實測 undefined reference ×40 各）。原定義在本檔 :6170／:6172 的 `#if 0 // ...resume the TODO(W6) gate`
//   區塊裡；照本檔「ungate ONE definition」的慣例應在區塊中間插 #endif／#if 0，但那會位移本檔後段行號，所以補在檔尾，
//   區塊內的原文留著不編譯。plain bool、沒有建構子，初值照 golden（false）。
//   ⚠ 目前全樹沒有活的寫入者（瑞薩 FT-CT 的寫入端 uRENESAS 相關碼沒有引用它們，否則早就連結失敗）⇒ 值恆 false，
//   與非瑞薩機台的 golden 相同。
bool bReplyFTCTAlarm=false;                                                     //RogerYang 20251021 : 瑞薩FT-CT  FTCT發送違法指令回傳Error需要報警
bool bContinueMessageByReply71=false;                                           //RogerYang 20251107 : 瑞薩FT-CT Add Continue Form
//AI(W906-SHOWERR) 20260929: ungate ONE definition -- forms/fNote_ShowError.cpp (golden note.cpp ShowErrorMessage, the maintenance-mode
//  branch) binds bMaintanceMode; the golden line (:6175) sits inside the TODO(W6) #if 0 block, so it is defined here at the end of
//  the file instead and no line above moves.  Plain bool, no ctor that touches a NULL global (PT_CAMPAIGN_PLAN.md section 8).
bool bMaintanceMode=false;                                                      //Steven 20251007 : maintance mode for Hana
