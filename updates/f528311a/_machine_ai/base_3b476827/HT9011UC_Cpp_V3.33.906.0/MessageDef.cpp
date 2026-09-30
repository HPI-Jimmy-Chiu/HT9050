// =============================================================================
//  MessageDef.cpp  --  definitions for the Handler<->GPIB/RS232/Refresh/2DID-
//                      CCD wire-message manifest declared in MessageDef.h.
//
//  Faithful translation of golden MessageDef.cpp (241 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 (rootleaf group).
//
//  ROLE: pure DATA definitions, zero functions (golden defines none). Every
//  MSG_CMD_* numeric value below is cross-process wire-format ABI (see
//  MessageDef.h's own banner) and is kept byte-identical to golden -- no
//  renumbering, no reordering, including the two upstream duplicate `extern`
//  declarations of MSG_CMD_MultiZoneTemp/MSG_CMD_MultiZoneEnable in the header
//  (golden's own duplication, harmless: a repeated `extern` of the same
//  external-linkage name is legal C++ and each is defined exactly ONCE below).
//
//  WAVE SCOPE: golden_fns = 0, ported_fns = 0 -- see MessageDef.h's own banner
//  for the full rationale (this is a manifest-constant + global-instance TU,
//  not a logic TU). GATE REGISTER: none -- no call in this TU reaches an
//  unbodied symbol; there is no call at all.
//
//  GLOBAL INSTANCES (golden :229-240, kept exactly): HGpib2Handler/
//  GHandler2Gpib/RefeshReceve/CCDReceve are left as NULL-initializing pointer
//  DEFINITIONS (golden itself never initializes them here either -- BCB6
//  zero-initializes global pointers by default, and so does standard C++ for
//  objects with static storage duration); HHandler2Gpib/GGpib2Handler/
//  RefeshSend/CCDSend are value-type struct instances, likewise golden-default
//  (zero-initialized) -- matching BCB6's own behaviour for a plain global
//  struct with no explicit initializer.
//
//  VCL/Borland conversions: #pragma hdrstop + #pragma package(smart_init)
//  dropped (golden :2, :8). No __fastcall / __property / __published in
//  golden MessageDef.cpp to strip.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8, in
//  its original column. Final gate: ZERO U+FFFD.
// =============================================================================
// BCB6 ORIGINAL include block (mirrored as a comment for provenance):
//   #include "MachineDefine.h" ; #pragma hdrstop
//   #include "MessageDef.h"    ; #pragma package(smart_init)
// =============================================================================
#include "MachineDefine.h"          // de-VCL'd include hub (vclcompat umbrella + portable STL + using namespace std)
#include "MessageDef.h"             // this unit's own contract (all constants + globals defined below)

//---------------------------------------------------------------------------

AnsiString GPIBVersion="12.13.905.0";                                          //AI(W906-GB-P0) 20260926: 883 -> 905.0, match H9046_32GPIB V12.13.905.0 (RS232Standard Rev12.13.902.0 carries 884.0 in its own copy)
AnsiString RS232Version="12.13.905.0";                                         //AI(W906-GB-P0) 20260926: same string as GPIBVersion (golden keeps both equal)
double GPIBVersionCheck=12.13;                                                  //Steven 20191007 : 改成判斷兩組版號, 所以使用Double
double TTLRS232VerCheck=10051302;                                               //Isaac 20210511 : TTLRS232板子版本檢查

const unsigned int MSG_CMD_NONE              =0;
const unsigned int MSG_CMD_CatalystSimpleGPIB=1;
const unsigned int MSG_CMD_SwitchArm         =2;
const unsigned int MSG_CMD_SwitchArmOK       =3;
const unsigned int MSG_CMD_AbortTest         =4;
const unsigned int MSG_CMD_AskArmTestMode    =5;
const unsigned int MSG_CMD_2ArmTestMode      =6;
const unsigned int MSG_CMD_1ArmTestMode      =7;
const unsigned int MSG_CMD_NoFullSiteRespon  =8;                                //Steven 20141016 : FullSite的Test Time Out
const unsigned int MSG_CMD_ECHONG            =9;                                //Steven 20250915 : 資料錯誤
const unsigned int MSG_CMD_DoubleContact     =10;

const unsigned int MSG_CMD_TimeOutSkip       =11;                               //Steven 20150304 : Add GPIB LOG
const unsigned int MSG_CMD_TimeOutRetryWait  =12;
const unsigned int MSG_CMD_TimeOutRetrySend  =13;
const unsigned int MSG_CMD_HandlerHomeStart  =14;
const unsigned int MSG_CMD_HandlerHomeFinish =15;
const unsigned int MSG_CMD_Arm1Down          =16;
const unsigned int MSG_CMD_Arm2Down          =17;
const unsigned int MSG_CMD_ContactTestArm1   =18;
const unsigned int MSG_CMD_ContactTestArm2   =19;
const unsigned int MSG_CMD_ContactTestAbort  =20;
const unsigned int MSG_CMD_CloseGpib         =21;                               //wei 20150408 Add Close GPIB Command
const unsigned int MSG_CMD_LotStatus         =22;                               //wei 20150409 Add Close GPIB Command
const unsigned int MSG_CMD_ChangeGpib        =23;                               //wei 20150409 Add Close GPIB Command
const unsigned int MSG_CMD_Version           =24;                               //wei 20150617 Add version control
const unsigned int MSG_CMD_RCMD              =25;                               //jou 2015-09-21 Auto Retest function
const unsigned int MSG_CMD_SVID              =26;                               //jou 2015-09-21 Auto Retest function
const unsigned int MSG_CMD_ECID              =27;                               //jou 2015-09-21 Auto Retest function
const unsigned int MSG_CMD_RetestFlag        =28;                               //jou 2015-09-21 Auto Retest function
const unsigned int MSG_CMD_CEIDON            =29;                               //Steven 20150901 : For GPIB V1.08 Command
const unsigned int MSG_CMD_CEIDOFF           =30;                               //Steven 20150901 : For GPIB V1.08 Command
const unsigned int MSG_CMD_EnableBarCode     =31;                               //Steven 20150713 : Add 2D code  //wei 20150924
const unsigned int MSG_CMD_DisableBarCode    =32;                               //Steven 20150713 : Add 2D code  //wei 20150924
const unsigned int MSG_CMD_BarCodeFlowErr    =33;                               //Steven 20150713 : Add 2D code  //wei 20150924
const unsigned int MSG_CMD_MachineState      =34;                               //JerryYang 20151109 add for 力成 回覆機台狀態
const unsigned int MSG_CMD_OverDrive         =35;                               //Steven 20151207 : OverDrive for TSMC
const unsigned int MSG_CMD_ReContact         =36;                               //Steven 20151207 : Recontact for TSMC
const unsigned int MSG_CMD_TesterBin         =37;                               //JerryYang 20160308 : BinCount for Maxim Philippine
const unsigned int MSG_CMD_SoakTime          =38;                               //JerryYang 20160308 : SoakTime for Maxim Philippine
const unsigned int MSG_CMD_JamCode           =39;                               //JerryYang 20160316 : Jam Code for Maxim Philippine
const unsigned int MSG_CMD_SiteMap           =40;                               //JerryYang 20160316 : Get site map for Maxim Philippine
const unsigned int MSG_CMD_AllMassTemp       =41;                               //JerryYang 20160325 : Get all mess temp for Maxim Philippine
const unsigned int MSG_CMD_BarcodeOFF        =42;                               //wei 20161024 No Open Barcode Function
const unsigned int MSG_CMD_SCKART_LOTCLEAR   =43;                               //Steven 20161025 : SCK ART function
const unsigned int MSG_CMD_SCKART_LOTRTCLEAR =44;
const unsigned int MSG_CMD_SCKART_INPUTQTY   =45;
const unsigned int MSG_CMD_SCKART_LOTSTATUS  =46;
const unsigned int MSG_CMD_SCKART_Alarm      =47;
const unsigned int MSG_CMD_SCKART_QTY        =48;
const unsigned int MSG_CMD_SCKART_INITIAL    =49;
const unsigned int MSG_CMD_SCKART_SRQMASK    =50;
const unsigned int MSG_CMD_TesterMode        =51;
const unsigned int MSG_CMD_State_Record      =52;                               //wei 20170911 State Record
const unsigned int MSG_CMD_SCKART_RunDummy   =53;                               //Steven 20180824 : Semi ART
const unsigned int MSG_CMD_Auto_Clean        =54;                               //wei 20180309
const unsigned int MSG_CMD_Pause             =55;                               //wei 20180309
//Steven 20190326 : Add for GPIB V9.0
//==>
const unsigned int MSG_CMD_TempArm           =56;
const unsigned int MSG_CMD_TestArm           =57;
const unsigned int MSG_CMD_ContactForce      =58;
const unsigned int MSG_CMD_ActualTemp        =59;
const unsigned int MSG_CMD_Assign            =60;
const unsigned int MSG_CMD_StartMode         =61;
const unsigned int MSG_CMD_HandlerID         =62;
const unsigned int MSG_CMD_HandlerSiteMap    =63;
const unsigned int MSG_CMD_HandlerSoakTime   =64;
const unsigned int MSG_CMD_HandlerTemperature=65;
const unsigned int MSG_CMD_Force             =66;
const unsigned int MSG_CMD_BinMap            =67;
const unsigned int MSG_CMD_TestMode          =68;
const unsigned int MSG_CMD_GetNowAllTemp     =69;
const unsigned int MSG_CMD_ChkSetup          =70;
const unsigned int MSG_CMD_GetTestArmPos     =71;
const unsigned int MSG_CMD_GetTestArmEP      =72;
const unsigned int MSG_CMD_SetTemp           =73;
const unsigned int MSG_CMD_SetSoakTime       =74;
const unsigned int MSG_CMD_SetTJ             =75;
const unsigned int MSG_CMD_SetSiteMapData    =76;
const unsigned int MSG_CMD_SetAlarmSetup     =77;
const unsigned int MSG_CMD_EnableAMDFunction =78;                               //V3.30.649 (GPIB V12.03) 以後不使用
const unsigned int MSG_CMD_DisableAMDFunction=79;                               //V3.30.649 (GPIB V12.03) 以後不使用
const unsigned int MSG_CMD_AMDNextStep1      =80;
const unsigned int MSG_CMD_AMDNextStep2      =81;
const unsigned int MSG_CMD_HanderIDRS232     =82;
const unsigned int MSG_CMD_GetSiteOnOff      =83;
const unsigned int MSG_CMD_GetNumOfSites     =84;

const unsigned int MSG_CMD_DeviceMapSRQ      =85;
const unsigned int MSG_CMD_PickLoad          =86;
const unsigned int MSG_CMD_PlaceLoad         =87;
const unsigned int MSG_CMD_TrayFeed          =88;

const unsigned int MSG_CMD_SamSung_Tmp       =89;                               //Steven 20191112 : 三星格式
const unsigned int MSG_CMD_SamSung_Map       =90;
const unsigned int MSG_CMD_SamSung_Soak      =91;
const unsigned int MSG_CMD_AMDRS232Connect   =92;                               //Ifor 20200220 : add AMD Rs232 Connect Error Alarm
const unsigned int MSG_CMD_2DIDFormat        =93;                               //JerryYang 20200422 2DID format
const unsigned int MSG_CMD_State_TTL         =94;                               //Isaac 20200903 :TTL RS232通訊
const unsigned int MSG_CMD_Command_TTL       =95;                               //Isaac 20200903 :TTL RS232通訊
const unsigned int MSG_CMD_ESC               =96;                               //Steven 20201022 : For RFMD Empty Socket Check Funstion.
const unsigned int MSG_CMD_RESUME            =97;                               //Steven 20201022 : For RFMD
const unsigned int MSG_CMD_TestAlarm         =98;                               //Steven 20201022 : For RFMD
const unsigned int MSG_CMD_POWERFOLLOWING    =99;
const unsigned int MSG_CMD_SetPID            =100;
const unsigned int MSG_CMD_GET_PFC_PARAMETER =101;                              //讀取單一溫度的PF參數值
const unsigned int MSG_CMD_SET_PFC_PARAMETER =102;                              //設定單一溫度的PF參數值
const unsigned int MSG_CMD_DoubleContactCount=103;                              //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
const unsigned int MSG_CMD_SetTestTemp       =104;                              //Ifor 20210623 add: Test Temp Change

const unsigned int MSG_CMD_SIGURD_CHKSTATUS  =105;                              //KaiChen 20180910 ：Add GPIB CHKSTATUS?
const unsigned int MSG_CMD_ONECYCLE          =106;                              //KaiChen 20180910 ：Add GPIB ONECYCLE
const unsigned int MSG_CMD_ECHOOK_ONECYCLE   =107;                              //KaiChen 20181114 ：Add GPIB ECHOOK:ONECYCLE
const unsigned int MSG_CMD_GETBINCATEGORY    =108;                              //KaiChen 20180913 ：Add GPIB GETBINCATEGORY?
const unsigned int MSG_CMD_SETUPFILENAME     =109;                              //KaiChen 20181022 ：Add GPIB GETSETUPFILENAME?
const unsigned int MSG_CMD_SIGURD_HANDLERID  =110;                              //KaiChen 20200507 ：Add GPIB HANDLERID?
const unsigned int MSG_CMD_SGSETUP           =111;                              //KaiChen 20190613 ：Add GPIB SGSETUP_
const unsigned int MSG_CMD_SETSTARTMODE      =112;                              //KaiChen 20180910 ：Add GPIB SetStartMode_
const unsigned int MSG_CMD_CHECKLIST         =113;                              //KaiChen 20190613 ：Add GPIB CHECKLIST?
const unsigned int MSG_CMD_BINPOS            =114;                              //KaiChen 20190706 ：Add GPIB BINPOS_
const unsigned int MSG_CMD_GetSGFTP_STATUS   =115;                              //Sam 20210329 : Add GPIB SGFTP_STATUS
const unsigned int MSG_CMD_SetSGFTP          =116;                              //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
const unsigned int MSG_CMD_SetNONDOUBLEBIN   =117;                              //Sam 20210329 : Add GPIB NONDOUBLEBIN_
const unsigned int MSG_CMD_SetBINCOUNT       =118;                              //Sam 20210329 : Add GPIB BINCOUNT_
const unsigned int MSG_CMD_SetSGOSBIN        =119;                              //Sam 20210406 : Add GPIB SGOSBIN_
const unsigned int MSG_CMD_SetSGCONTFAIL     =120;                              //Sam 20210422 : Add GPIB SGCONTFAIL_
const unsigned int MSG_CMD_SETTESTERID       =121;                              //Sam 20210617 : Add GPIB SETTESTERID
const unsigned int MSG_CMD_GETTESTERID       =122;                              //Sam 20210617 : Add GPIB SETTESTERID
const unsigned int MSG_CMD_SBIN              =123;                              //Steven 20220120 : Amlogic需要收SBIN
const unsigned int MSG_CMD_Pause01           =124;                              //Steven 20220517 : Add for GIGA
const unsigned int MSG_CMD_Stop_01           =125;                              //Steven 20220517 : Add for GIGA
const unsigned int MSG_CMD_GetAutClean       =126;                              //Sam 20220408 : Novatek 新增 AUTOCLEAN?
const unsigned int MSG_CMD_ForcePerPinN      =127;                              //Sam 20220408 : Novatek 新增 DEVICEFORCEPERPIN?
const unsigned int MSG_CMD_ContactHeight     =128;                              //Sam 20220408 : Novatek 新增 ARMCONTACTHIGHVALUE?
const unsigned int MSG_CMD_YieldContinusFail =129;                              //Sam 20220408 : Novatek 新增 YIELDCONTINUESFAIL?
const unsigned int MSG_CMD_YieldSiteCompare  =130;                              //Sam 20220408 : Novatek 新增 YIELDSITEUNBALANCE?
const unsigned int MSG_CMD_DUTStatus         =131;                              //Sam 20220408 : Novatek 新增 DUTSTATUS?
const unsigned int MSG_CMD_UPH               =132;                              //Sam 20220408 : Novatek 新增 UPH?
const unsigned int MSG_CMD_IndexCycleTime    =133;                              //Sam 20220408 : Novatek 新增 INDEXCYCLETIME?
const unsigned int MSG_CMD_TempOfs           =134;                              //Sam 20220408 : Novatek 新增 GETTEMPOFFSET?
const unsigned int MSG_CMD_TempRange         =135;                              //Sam 20220408 : Novatek 新增 GETTEMPERATURETOLERANCE?
const unsigned int MSG_CMD_VACUUMAIR         =136;                              //Sam 20220408 : Novatek 新增 VACUUMAIR?
const unsigned int MSG_CMD_Get_All           =137;                              //Sam 20220408 : Novatek 新增 SET_ALL?
const unsigned int MSG_CMD_HandlerVersion    =138;                              //Sam 20220408 : Novatek 新增 HANDLERVERSION?
const unsigned int MSG_CMD_PPSELECT          =139;                              //Richard 20220929 :Add for UTAC 讀檔
const unsigned int MSG_CMD_ASKPPSELECT       =140;                              //Richard 20220929 :Add for UTAC 讀檔 詢問Handler當前檔名
const unsigned int MSG_CMD_SetBinMap         =141;                              //Steven 20230210 : Set Bin Map.
const unsigned int MSG_CMD_GETSHUTTLEMODE    =142;                              //Sam 20230130 : Add GPIB GETSHUTTLEMODE?
const unsigned int MSG_CMD_SETMAXTEST        =143;                              //Sam 20230201 : Add GPIB SETMAXTEST_
const unsigned int MSG_CMD_GETMAXTEST        =144;                              //Sam 20230201 : Add GPIB GETMAXTEST
const unsigned int MSG_CMD_SETINITIALMAXTEST =145;                              //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
const unsigned int MSG_CMD_GETINITIALMAXTEST =146;                              //Sam 20230201 : Add GPIB GETINITIALMAXTEST
const unsigned int MSG_CMD_READYNEXTSHOT     =147;                              //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
const unsigned int MSG_CMD_NEXT2DID          =148;                              //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
const unsigned int MSG_CMD_CloseSiteHaveBin  =149;                              //Steven 20231017 : GPIB flow error need alarm
const unsigned int MSG_CMD_BinonWithout0x41  =150;                              //Steven 20231017 : GPIB flow error need alarm
const unsigned int MSG_CMD_BinonWithoutFullsite=151;                            //Steven 20231017 : GPIB flow error need alarm
const unsigned int MSG_CMD_SETAICCD          =152;                              //Sam 20231108 : Add GPIB SETAICCD_
const unsigned int MSG_CMD_ASIF_TJ_EFUSED    =153;                              //Steven 20240903 : for MTK ASIF data
const unsigned int MSG_CMD_ASIF_TJ_REQUEST   =154;                              //Steven 20240903 : for MTK ASIF data
const unsigned int MSG_CMD_ASIF_TJ_FB        =155;                              //Steven 20240903 : for MTK ASIF data
const unsigned int MSG_CMD_GETAICCD          =156;                              //Sam 20240826 : Add GPIB GETAICCD?
const unsigned int MSG_CMD_QRA               =157;                              //Steven 20241004 : Qorvo check ART enable
const unsigned int MSG_CMD_RUN_HANA_ART      =158;                              //JimmyChiu 20241023 HANA ART Function
const unsigned int MSG_CMD_HANA_ART          =159;                              //JimmyChiu 20241023 HANA ART Function
const unsigned int MSG_CMD_SETOSBIN          =160;                              //Sam 20250115 : Add GPIB SETOSBIN_
const unsigned int MSG_CMD_GETOSBIN          =161;                              //Sam 20250115 : Add GPIB GETOSBIN?
const unsigned int MSG_CMD_DUTCHK            =162;                              //Steven 20250701 : for DOOSAN TESNA
const unsigned int MSG_CMD_GetFFC            =163;                              //Steven 20250701 : for Ampere
const unsigned int MSG_CMD_GetTJFunction     =164;
const unsigned int MSG_CMD_GetPowerFollowing =165;
const unsigned int MSG_CMD_SetSiteOnOff      =166;
const unsigned int MSG_CMD_EnableFTPFunction =167;                              //Ifor 20231101 add:FTP Function
const unsigned int MSG_CMD_DisableFTPFunction=168;                              //Ifor 20231101 add:FTP Function
const unsigned int MSG_CMD_SetupFileChange   =169;                              //Ifor 20231101 add:FTP Function
const unsigned int MSG_CMD_GetContactCount   =170;                              //Ifor 20240510 add: Get Head Contact Count
const unsigned int MSG_CMD_EnablePin1Function =171;                             //Ifor 20240528 add:Pin1 Function
const unsigned int MSG_CMD_DisablePin1Function=172;                             //Ifor 20240528 add:Pin1 Function
const unsigned int MSG_CMD_BarcodePin1ON      =173;                             //Ifor 20240528 add:Pin1 Function
const unsigned int MSG_CMD_GetSocketCounter   =174;                             //Ifor 20250607 add:Get Socket Counter
const unsigned int MSG_CMD_GetTIMCounter      =175;                             //Ifor 20250607 add:Get TIM Counter
const unsigned int MSG_CMD_FTPDownLoad       =176;                              //Ifor 20231101 add:FTP Function
const unsigned int MSG_CMD_SetSocketToBinR   =177;                              //Ifor 20210911: Add KLT 要求BINON WITHOUT 0x41 ERROR直接分ErrBin
const unsigned int MSG_CMD_MultiZoneTemp     =178;                              //wei 20240617 Multi Zone Temp
const unsigned int MSG_CMD_MultiZoneEnable   =179;                              //wei 20240617 Multi Zone Temp

const unsigned int MSG_CMD_GET_SLOPE_OFFSET  =180;                              //Get the Slope/Offset value            //Eliot 20210412
const unsigned int MSG_CMD_SET_SLOPE_OFFSET  =181;                              //Set the Slope/Offset value            //Eliot 20210412
const unsigned int MSG_CMD_READTJ            =182;                              //Set the Slope/Offset value            //Eliot 20210412
const unsigned int MSG_CMD_GET_VOLTAGE       =183;                              //Set the Slope/Offset value            //Eliot 20210412
const unsigned int MSG_CMD_SET_ATCCONTROLMODE=184;                              //Set the Slope/Offset value            //Eliot 20210412
const unsigned int MSG_CMD_SET_ATC_TEMP      =185;                              //Set ATC Temp
const unsigned int MSG_CMD_RECODETJ          =186;
const unsigned int MSG_CMD_QUERYTJ           =187;
const unsigned int MSG_CMD_GET_ATCCONTROLMODE=188;
const unsigned int MSG_CMD_GET_ATCERROR      =189;
const unsigned int MSG_CMD_RESET_ATCALARM    =190;

const unsigned int MSG_CMD_READ_WATER_VALVE  =191;                              //wei 20250512
const unsigned int MSG_CMD_SET_WATER_VALVE   =192;                              //wei 20250512
const unsigned int MSG_CMD_SET_DYNAMIC_PID   =193;                              //wei 20250512
const unsigned int MSG_CMD_READ_DYNAMIC_PID  =194;                              //wei 20250512

const unsigned int MSG_CMD_AUTOZSTART        =195;                              //wei 20250512
const unsigned int MSG_CMD_AUTOZMOVE         =196;                              //wei 20250512
const unsigned int MSG_CMD_AUTOZMOVEOK       =197;                              //wei 20250512
const unsigned int MSG_CMD_AUTOZPASS         =198;                              //wei 20250512
const unsigned int MSG_CMD_AUTOZREACHLIMIT   =199;                              //wei 20250512
const unsigned int MSG_CMD_AUTOZOK           =200;                              //wei 20250512
const unsigned int MSG_CMD_READAUTOZLIMIT    =201;                              //wei 20250512
const unsigned int MSG_CMD_READZPOS          =202;                              //wei 20250625
const unsigned int MSG_CMD_READZTORQUE       =203;                              //wei 20250625
const unsigned int MSG_CMD_RemoteStart       =204;                              //JerryYang 20260828 : Qualcomm remote START by GPIB    //AI(W906-GB-P2c) 20260926: golden 912 MessageDef.h:231
const unsigned int MSG_CMD_RemoteStop        =205;                              //JerryYang 20260828 : Qualcomm remote STOP by GPIB    //AI(W906-GB-P2c) 20260926: golden 912 MessageDef.h:232

//For Handler----------
VM *HGpib2Handler;
MV HHandler2Gpib;

//For GPIB-------------
VM GGpib2Handler;
MV *GHandler2Gpib;
//---------------------
VM1 *RefeshReceve;
MV1 RefeshSend;
VM2 *CCDReceve;
MV2 CCDSend;
