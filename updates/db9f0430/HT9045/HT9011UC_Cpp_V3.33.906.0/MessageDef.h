// =============================================================================
//  MessageDef.h  --  Handler <-> GPIB (32-bit bridge) / Refresh / 2DID-CCD
//                    WM_COPYDATA message-shape contract: the VM/MV/VM1/MV1/
//                    VM2/MV2 wire structs and the full MSG_CMD_* command-code
//                    manifest.
//
//  Faithful translation of golden MessageDef.h (325 lines, BCB6, Big5/cp950).
//  Translator: AI(W906-PT-W2) 20260807
//  Translation wave: W906-PT-W2 (rootleaf group).
//
//  ROLE: pure DATA contract, zero functions (golden declares none). This is
//  the shared wire format golden's own file-head comment (kept verbatim just
//  below) warns must stay in lock-step across THREE separately-built EXEs
//  (Handler / GPIB bridge / RS232 bridge) -- hence "不能塞VCL元件在裡面,
//  只能用純C" (no VCL components allowed inside, C-only). Every VM/MV/VM1/
//  MV1/VM2/MV2 field, and every MSG_CMD_* numeric VALUE, is load-bearing
//  cross-process ABI and is kept byte-identical to golden (field order,
//  field width, numeric value) -- this is exactly the kind of struct rule 3
//  ("do NOT reorder branches, do NOT collapse duplicated code") extends to at
//  its strictest.
//
//  CROSS-UNIT LANDING NOTE (why several existing files were WAITING for this
//  exact header, and what it does NOT do): a whole-tree grep before writing
//  this file found Automation/HANA_ART.cpp, AutoRetest.cpp, database.cpp and
//  forms/fMain.cpp already citing "golden MessageDef.h" in their own banners
//  as untranslated infrastructure they gate around --
//    * Automation/HANA_ART.cpp:127-132 defines its OWN `static const unsigned
//      int MSG_CMD_RUN_HANA_ART/MSG_CMD_HANA_ART` pair (internal linkage,
//      `static`) as an explicitly-labelled TEMPORARY stand-in, "DELETE this
//      local pair once MessageDef.h lands for real" -- that deletion is a
//      cross-file edit this wave does NOT make (rule 7: this wave creates new
//      mirrored files only, it does not touch Automation/HANA_ART.cpp). Zero
//      ODR risk either way: `static` gives that file's copy internal linkage,
//      so it cannot collide at link time with this header's `extern`
//      declarations + MessageDef.cpp's external-linkage definitions of the
//      same names, regardless of which TUs get linked together.
//    * AutoRetest.cpp:338-339 does the same for MSG_CMD_EnableBarCode/
//      MSG_CMD_DisableBarCode (also `static`, same non-collision reasoning).
//    * database.cpp:1101 and forms/fMain.cpp:312-314 have `#if 0 //
//      TODO(...): ... MessageDef.h/.cpp untranslated` gates referencing
//      TTLRS232VerCheck and the MSG_CMD_* + SendMSG_CMD facade respectively --
//      both are pre-existing files this wave does not edit either; whichever
//      wave next touches those files can now un-gate against a real header.
//    * HHandler2Gpib/GGpib2Handler/GHandler2Gpib/HGpib2Handler/RefeshReceve/
//      RefeshSend/CCDReceve/CCDSend (golden :312-322) are declared here with
//      the same extern linkage as golden -- consumers that reach them all
//      pre-date this wave and gate their OWN touch sites individually (see
//      Automation/HANA_ART.cpp's file-head "GATED DEPENDENCIES" #1), so no
//      further action is needed here for those call sites to keep compiling.
//
//  WAVE SCOPE: golden_fns = 0, ported_fns = 0 -- every one of golden's 325
//  lines is a manifest constant, a struct definition, or an extern global;
//  there is no function to translate and therefore nothing to gate.
//
//  VCL/Borland conversions: `#include "MachineType.h"` (golden :12) is kept
//  (supplies AnsiString via its own vclcompat umbrella chain, and HWND via its
//  own `#include <windows.h>`, both used below). `//#define VERSION_5_DEBUG`
//  (golden :11) is dead text in golden itself (commented out), kept as-is.
//  No __fastcall / __property / __published / #pragma anywhere in golden
//  MessageDef.h to strip.
//
//  Big5: every Chinese comment decoded via cp950 and preserved as UTF-8, in
//  its original column. Final gate: ZERO U+FFFD.
// =============================================================================
//---------------------------------------------------------------------------
//MV & VM 結構請不要任意加東西
//就算要加也是加在最下面
//只要有改就是 Handler, GPIB, RS232三隻程式要一起動, 然後一起進版號
//不能塞VCL元件在裡面, 只能用純C
//---------------------------------------------------------------------------
#ifndef MessageDefH
#define MessageDefH
//---------------------------------------------------------------------------

//#define VERSION_5_DEBUG
#include "MachineType.h"

extern AnsiString GPIBVersion;
extern double GPIBVersionCheck;
extern AnsiString RS232Version;
extern double TTLRS232VerCheck;                                                 //Isaac 20210511 : TTLRS232板子版本檢查
//---------------------------------------------------------------------------

extern const unsigned int MSG_CMD_NONE              ;
extern const unsigned int MSG_CMD_CatalystSimpleGPIB;
extern const unsigned int MSG_CMD_SwitchArm         ;
extern const unsigned int MSG_CMD_SwitchArmOK       ;
extern const unsigned int MSG_CMD_AbortTest         ;
extern const unsigned int MSG_CMD_AskArmTestMode    ;
extern const unsigned int MSG_CMD_2ArmTestMode      ;
extern const unsigned int MSG_CMD_1ArmTestMode      ;
extern const unsigned int MSG_CMD_NoFullSiteRespon  ;                           //Steven 20141016 : FullSite的Test Time Out
extern const unsigned int MSG_CMD_ECHONG            ;                           //Steven 20250915 : 資料錯誤
extern const unsigned int MSG_CMD_DoubleContact     ;

extern const unsigned int MSG_CMD_TimeOutSkip       ;                           //Steven 20150304 : Add GPIB LOG
extern const unsigned int MSG_CMD_TimeOutRetryWait  ;
extern const unsigned int MSG_CMD_TimeOutRetrySend  ;
extern const unsigned int MSG_CMD_HandlerHomeStart  ;
extern const unsigned int MSG_CMD_HandlerHomeFinish ;
extern const unsigned int MSG_CMD_Arm1Down          ;
extern const unsigned int MSG_CMD_Arm2Down          ;
extern const unsigned int MSG_CMD_ContactTestArm1   ;
extern const unsigned int MSG_CMD_ContactTestArm2   ;
extern const unsigned int MSG_CMD_ContactTestAbort  ;
extern const unsigned int MSG_CMD_CloseGpib         ;                           //wei 20150408 Add Close GPIB Command
extern const unsigned int MSG_CMD_LotStatus         ;                           //wei 20150409 Add Close GPIB Command
extern const unsigned int MSG_CMD_ChangeGpib        ;                           //wei 20150409 Add Close GPIB Command
extern const unsigned int MSG_CMD_Version           ;                           //wei 20150617 Add version control
extern const unsigned int MSG_CMD_RCMD              ;                           //jou 2015-09-21 Auto Retest function
extern const unsigned int MSG_CMD_SVID              ;                           //jou 2015-09-21 Auto Retest function
extern const unsigned int MSG_CMD_ECID              ;                           //jou 2015-09-21 Auto Retest function
extern const unsigned int MSG_CMD_RetestFlag        ;                           //jou 2015-09-21 Auto Retest function
extern const unsigned int MSG_CMD_CEIDON            ;                           //Steven 20150901 : For GPIB V1.08 Command
extern const unsigned int MSG_CMD_CEIDOFF           ;                           //Steven 20150901 : For GPIB V1.08 Command
extern const unsigned int MSG_CMD_EnableBarCode     ;                           //Steven 20150713 : Add 2D code  //wei 20150924
extern const unsigned int MSG_CMD_DisableBarCode    ;                           //Steven 20150713 : Add 2D code  //wei 20150924
extern const unsigned int MSG_CMD_BarCodeFlowErr    ;                           //Steven 20150713 : Add 2D code  //wei 20150924
extern const unsigned int MSG_CMD_MachineState      ;                           //JerryYang 20151109 add for 力成 回覆機台狀態
extern const unsigned int MSG_CMD_OverDrive         ;                           //Steven 20151207 : OverDrive for TSMC
extern const unsigned int MSG_CMD_ReContact         ;                           //Steven 20151207 : Recontact for TSMC
extern const unsigned int MSG_CMD_TesterBin         ;                           //JerryYang 20151207 : BinCount for Maxim Philippine
extern const unsigned int MSG_CMD_SoakTime          ;                           //JerryYang 20160308 : SoakTime for Maxim Philippine
extern const unsigned int MSG_CMD_JamCode           ;                           //JerryYang 20160316 : Jam Code for Maxim Philippine
extern const unsigned int MSG_CMD_SiteMap           ;                           //JerryYang 20160316 : Get site map for Maxim Philippine
extern const unsigned int MSG_CMD_AllMassTemp       ;                           //JerryYang 20160325 : Get all mess temp for Maxim Philippine
extern const unsigned int MSG_CMD_BarcodeOFF        ;                           //wei 20161024 No Open Barcode Function
extern const unsigned int MSG_CMD_SCKART_LOTCLEAR   ;                           //Steven 20161025 : SCK ART function
extern const unsigned int MSG_CMD_SCKART_LOTRTCLEAR ;
extern const unsigned int MSG_CMD_SCKART_INPUTQTY   ;
extern const unsigned int MSG_CMD_SCKART_LOTSTATUS  ;
extern const unsigned int MSG_CMD_SCKART_Alarm      ;
extern const unsigned int MSG_CMD_SCKART_QTY        ;
extern const unsigned int MSG_CMD_SCKART_INITIAL    ;
extern const unsigned int MSG_CMD_SCKART_SRQMASK    ;
extern const unsigned int MSG_CMD_TesterMode        ;
extern const unsigned int MSG_CMD_State_Record      ;                           //wei 20170911 (steven) State Record
extern const unsigned int MSG_CMD_SCKART_RunDummy   ;                           //Steven 20180824 : Semi ART
extern const unsigned int MSG_CMD_Auto_Clean        ;                           //wei 20180309
extern const unsigned int MSG_CMD_Pause             ;                           //wei 20180309
extern const unsigned int MSG_CMD_TempArm           ;
extern const unsigned int MSG_CMD_TestArm           ;
extern const unsigned int MSG_CMD_ContactForce      ;
extern const unsigned int MSG_CMD_ActualTemp        ;
extern const unsigned int MSG_CMD_Assign            ;
extern const unsigned int MSG_CMD_StartMode         ;
extern const unsigned int MSG_CMD_HandlerID         ;
extern const unsigned int MSG_CMD_HandlerSiteMap    ;
extern const unsigned int MSG_CMD_HandlerSoakTime   ;
extern const unsigned int MSG_CMD_HandlerTemperature;
extern const unsigned int MSG_CMD_Force             ;
extern const unsigned int MSG_CMD_BinMap            ;
extern const unsigned int MSG_CMD_TestMode          ;
extern const unsigned int MSG_CMD_GetNowAllTemp     ;
extern const unsigned int MSG_CMD_ChkSetup          ;
extern const unsigned int MSG_CMD_GetTestArmPos     ;
extern const unsigned int MSG_CMD_GetTestArmEP      ;
extern const unsigned int MSG_CMD_SetTemp           ;
extern const unsigned int MSG_CMD_SetSoakTime       ;
extern const unsigned int MSG_CMD_SetTJ             ;
extern const unsigned int MSG_CMD_SetSiteMapData    ;
extern const unsigned int MSG_CMD_SetAlarmSetup     ;
extern const unsigned int MSG_CMD_EnableAMDFunction ;                           //V3.30.649 (GPIB V12.03) 以後不使用
extern const unsigned int MSG_CMD_DisableAMDFunction;                           //V3.30.649 (GPIB V12.03) 以後不使用
extern const unsigned int MSG_CMD_AMDNextStep1      ;
extern const unsigned int MSG_CMD_AMDNextStep2      ;
extern const unsigned int MSG_CMD_HanderIDRS232     ;
extern const unsigned int MSG_CMD_GetSiteOnOff      ;
extern const unsigned int MSG_CMD_GetNumOfSites     ;

extern const unsigned int MSG_CMD_DeviceMapSRQ      ;
extern const unsigned int MSG_CMD_PickLoad          ;
extern const unsigned int MSG_CMD_PlaceLoad         ;
extern const unsigned int MSG_CMD_TrayFeed          ;

extern const unsigned int MSG_CMD_SamSung_Tmp       ;                           //Steven 20191112 : 三星格式
extern const unsigned int MSG_CMD_SamSung_Map       ;
extern const unsigned int MSG_CMD_SamSung_Soak      ;
extern const unsigned int MSG_CMD_AMDRS232Connect   ;                           //Ifor 20200220 : add AMD Rs232 Connect Error Alarm
extern const unsigned int MSG_CMD_2DIDFormat        ;                           //JerryYang 20200422 2DID format
extern const unsigned int MSG_CMD_State_TTL         ;                           //Isaac 20200903 :TTL RS232通訊
extern const unsigned int MSG_CMD_Command_TTL       ;                           //Isaac 20200903 :TTL RS232通訊
extern const unsigned int MSG_CMD_ESC               ;                           //Steven 20201022 : For RFMD Empty Socket Check Funstion.
extern const unsigned int MSG_CMD_RESUME            ;                           //Steven 20201022 : For RFMD
extern const unsigned int MSG_CMD_TestAlarm         ;                           //Steven 20201022 : For RFMD
extern const unsigned int MSG_CMD_POWERFOLLOWING    ;
extern const unsigned int MSG_CMD_SetPID            ;
extern const unsigned int MSG_CMD_GET_PFC_PARAMETER ;                           //讀取單一溫度的PF參數值
extern const unsigned int MSG_CMD_SET_PFC_PARAMETER ;                           //設定單一溫度的PF參數值
extern const unsigned int MSG_CMD_DoubleContactCount;                           //Isaac 20210706 : add MSG_CMD_DoubleContactCount指令，詢問handler doublecontact次數
extern const unsigned int MSG_CMD_SetTestTemp       ;                           //Ifor 20210623 add: Test Temp Change
extern const unsigned int MSG_CMD_SIGURD_CHKSTATUS  ;                           //KaiChen 20180910 ：Add GPIB CHKSTATUS?
extern const unsigned int MSG_CMD_ONECYCLE          ;                           //KaiChen 20180910 ：Add GPIB ONECYCLE
extern const unsigned int MSG_CMD_ECHOOK_ONECYCLE   ;                           //KaiChen 20181114 ：Add GPIB ECHOOK:ONECYCLE
extern const unsigned int MSG_CMD_GETBINCATEGORY    ;                           //KaiChen 20180913 ：Add GPIB GETBINCATEGORY?
extern const unsigned int MSG_CMD_SETUPFILENAME     ;                           //KaiChen 20181022 ：Add GPIB GETSETUPFILENAME?
extern const unsigned int MSG_CMD_SIGURD_HANDLERID  ;                           //KaiChen 20200507 ：Add GPIB HANDLERID?
extern const unsigned int MSG_CMD_SGSETUP           ;                           //KaiChen 20190613 ：Add GPIB SGSETUP_
extern const unsigned int MSG_CMD_SETSTARTMODE      ;                           //KaiChen 20180910 ：Add GPIB SetStartMode_
extern const unsigned int MSG_CMD_CHECKLIST         ;                           //KaiChen 20190613 ：Add GPIB CHECKLIST?
extern const unsigned int MSG_CMD_BINPOS            ;                           //KaiChen 20190706 ：Add GPIB BINPOS_
extern const unsigned int MSG_CMD_GetSGFTP_STATUS   ;                           //Sam 20210329 : Add GPIB SGFTP_STATUS
extern const unsigned int MSG_CMD_SetSGFTP          ;                           //Sam 20210329 : Add GPIB SGFTP_ SGFTP_ON/SGFTP_OFF
extern const unsigned int MSG_CMD_SetNONDOUBLEBIN   ;                           //Sam 20210329 : Add GPIB NONDOUBLEBIN_
extern const unsigned int MSG_CMD_SetBINCOUNT       ;                           //Sam 20210329 : Add GPIB BINCOUNT_
extern const unsigned int MSG_CMD_SetSGOSBIN        ;                           //Sam 20210406 : Add GPIB SGOSBIN_
extern const unsigned int MSG_CMD_SetSGCONTFAIL     ;                           //Sam 20210422 : Add GPIB SGCONTFAIL_
extern const unsigned int MSG_CMD_SETTESTERID       ;                           //Sam 20210617 : Add GPIB SETTESTERID
extern const unsigned int MSG_CMD_GETTESTERID       ;                           //Sam 20210617 : Add GPIB SETTESTERID
extern const unsigned int MSG_CMD_SBIN              ;                           //Steven 20220120 : Amlogic需要收SBIN
extern const unsigned int MSG_CMD_Pause01           ;                           //Steven 20220517 : Add for GIGA
extern const unsigned int MSG_CMD_Stop_01           ;                           //Steven 20220517 : Add for GIGA
extern const unsigned int MSG_CMD_GetAutClean       ;                           //Sam 20220408 : Novatek 新增 AUTOCLEAN?
extern const unsigned int MSG_CMD_ForcePerPinN      ;                           //Sam 20220408 : Novatek 新增 DEVICEFORCEPERPIN?
extern const unsigned int MSG_CMD_ContactHeight     ;                           //Sam 20220408 : Novatek 新增 ARMCONTACTHIGHVALUE?
extern const unsigned int MSG_CMD_YieldContinusFail ;                           //Sam 20220408 : Novatek 新增 YIELDCONTINUESFAIL?
extern const unsigned int MSG_CMD_YieldSiteCompare  ;                           //Sam 20220408 : Novatek 新增 YIELDSITEUNBALANCE?
extern const unsigned int MSG_CMD_DUTStatus         ;                           //Sam 20220408 : Novatek 新增 DUTSTATUS?
extern const unsigned int MSG_CMD_UPH               ;                           //Sam 20220408 : Novatek 新增 UPH?
extern const unsigned int MSG_CMD_IndexCycleTime    ;                           //Sam 20220408 : Novatek 新增 INDEXCYCLETIME?
extern const unsigned int MSG_CMD_TempOfs           ;                           //Sam 20220408 : Novatek 新增 GETTEMPOFFSET?
extern const unsigned int MSG_CMD_TempRange         ;                           //Sam 20220408 : Novatek 新增 GETTEMPERATURETOLERANCE?
extern const unsigned int MSG_CMD_VACUUMAIR         ;                           //Sam 20220408 : Novatek 新增 VACUUMAIR?
extern const unsigned int MSG_CMD_Get_All           ;                           //Sam 20220408 : Novatek 新增 SET_ALL?
extern const unsigned int MSG_CMD_HandlerVersion    ;                           //Sam 20220408 : Novatek 新增 HANDLERVERSION?
extern const unsigned int MSG_CMD_PPSELECT          ;                           //Richard 20220929 :Add for UTAC 讀檔
extern const unsigned int MSG_CMD_ASKPPSELECT       ;                           //Richard 20220929 :Add for UTAC 讀檔 詢問Handler當前檔名
extern const unsigned int MSG_CMD_SetBinMap         ;                           //Steven 20230210 : Set Bin Map.
extern const unsigned int MSG_CMD_GETSHUTTLEMODE    ;                           //Sam 20230130 : Add GPIB GETSHUTTLEMODE?
extern const unsigned int MSG_CMD_SETMAXTEST        ;                           //Sam 20230201 : Add GPIB SETMAXTEST_
extern const unsigned int MSG_CMD_GETMAXTEST        ;                           //Sam 20230201 : Add GPIB GETMAXTEST
extern const unsigned int MSG_CMD_SETINITIALMAXTEST ;                           //Sam 20230201 : Add GPIB SETINITIALMAXTEST_
extern const unsigned int MSG_CMD_GETINITIALMAXTEST ;                           //Sam 20230201 : Add GPIB GETINITIALMAXTEST
extern const unsigned int MSG_CMD_READYNEXTSHOT ;                               //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
extern const unsigned int MSG_CMD_NEXT2DID ;                                    //Jimmychiu 20231011 : #[SCK_HT9046LS] Request for GPIB command adding for next 2DID information
extern const unsigned int MSG_CMD_CloseSiteHaveBin  ;                           //Steven 20231017 : GPIB flow error need alarm
extern const unsigned int MSG_CMD_BinonWithout0x41  ;                           //Steven 20231017 : GPIB flow error need alarm
extern const unsigned int MSG_CMD_BinonWithoutFullsite;                         //Steven 20231017 : GPIB flow error need alarm
extern const unsigned int MSG_CMD_SETAICCD          ;                           //Sam 20231108 : Add GPIB SETAICCD_
extern const unsigned int MSG_CMD_ASIF_TJ_EFUSED    ;                           //Steven 20240903 : for MTK ASIF data
extern const unsigned int MSG_CMD_ASIF_TJ_REQUEST   ;                           //Steven 20240903 : for MTK ASIF data
extern const unsigned int MSG_CMD_ASIF_TJ_FB        ;                           //Steven 20240903 : for MTK ASIF data
extern const unsigned int MSG_CMD_GETAICCD          ;                           //Sam 20240826 : Add GPIB GETAICCD?
extern const unsigned int MSG_CMD_QRA               ;                           //Steven 20241004 : Qorvo check ART enable
extern const unsigned int MSG_CMD_RUN_HANA_ART      ;                           //JimmyChiu 20241023 HANA ART Function
extern const unsigned int MSG_CMD_HANA_ART          ;                           //JimmyChiu 20241023 HANA ART Function
extern const unsigned int MSG_CMD_SETOSBIN          ;                           //Sam 20250115 : Add GPIB SETOSBIN_
extern const unsigned int MSG_CMD_GETOSBIN          ;                           //Sam 20250115 : Add GPIB GETOSBIN?
extern const unsigned int MSG_CMD_DUTCHK            ;                           //Steven 20250701 : for DOOSAN TESNA
extern const unsigned int MSG_CMD_GetFFC            ;                           //Steven 20250701 : for Ampere
extern const unsigned int MSG_CMD_GetTJFunction     ;
extern const unsigned int MSG_CMD_GetPowerFollowing ;
extern const unsigned int MSG_CMD_SetSiteOnOff      ;
extern const unsigned int MSG_CMD_FTPDownLoad       ;                           //Ifor 20231101 add:FTP Function
extern const unsigned int MSG_CMD_EnableFTPFunction ;                           //Ifor 20231101 add:FTP Function
extern const unsigned int MSG_CMD_DisableFTPFunction;                           //Ifor 20231101 add:FTP Function
extern const unsigned int MSG_CMD_SetupFileChange   ;                           //Ifor 20231101 add:FTP Function
extern const unsigned int MSG_CMD_GetContactCount   ;                           //Ifor 20240510 add: Get Head Contact Count
extern const unsigned int MSG_CMD_EnablePin1Function;                           //Ifor 20240528 add:Pin1 Function
extern const unsigned int MSG_CMD_DisablePin1Function;                          //Ifor 20240528 add:Pin1 Function
extern const unsigned int MSG_CMD_BarcodePin1ON;                                //Ifor 20240528 add:Pin1 Function
extern const unsigned int MSG_CMD_GetSocketCounter;                             //Ifor 20250607 add:Get Socket Counter
extern const unsigned int MSG_CMD_GetTIMCounter;                                //Ifor 20250607 add:Get TIM Counter

extern const unsigned int MSG_CMD_SetSocketToBinR   ;                           //Ifor 20210911: Add KLT 要求BINON WITHOUT 0x41 ERROR直接分ErrBin
extern const unsigned int MSG_CMD_MultiZoneTemp     ;                           //wei 20240617 Multi Zone Temp
extern const unsigned int MSG_CMD_MultiZoneEnable   ;                           //wei 20240617 Multi Zone Temp
extern const unsigned int MSG_CMD_GET_SLOPE_OFFSET  ;                           //Get the Slope/Offset value            //Eliot 20210412
extern const unsigned int MSG_CMD_SET_SLOPE_OFFSET  ;                           //Set the Slope/Offset value            //Eliot 20210412
extern const unsigned int MSG_CMD_READTJ            ;                           //Set the Slope/Offset value            //Eliot 20210412
extern const unsigned int MSG_CMD_GET_VOLTAGE       ;                           //Set the Slope/Offset value            //Eliot 20210412
extern const unsigned int MSG_CMD_SET_ATCCONTROLMODE;                           //Set the Slope/Offset value            //Eliot 20210412
extern const unsigned int MSG_CMD_SET_ATC_TEMP      ;                           //Set ATC Temp
extern const unsigned int MSG_CMD_RECODETJ          ;
extern const unsigned int MSG_CMD_QUERYTJ           ;
extern const unsigned int MSG_CMD_GET_ATCCONTROLMODE;
extern const unsigned int MSG_CMD_GET_ATCERROR      ;
extern const unsigned int MSG_CMD_RESET_ATCALARM    ;

extern const unsigned int MSG_CMD_MultiZoneTemp     ;                           //wei 20240617 Multi Zone Temp
extern const unsigned int MSG_CMD_MultiZoneEnable   ;                           //wei 20240617 Multi Zone Temp
extern const unsigned int MSG_CMD_READ_WATER_VALVE  ;                           //wei 20250512
extern const unsigned int MSG_CMD_SET_WATER_VALVE   ;                           //wei 20250512
extern const unsigned int MSG_CMD_SET_DYNAMIC_PID   ;                           //wei 20250512
extern const unsigned int MSG_CMD_READ_DYNAMIC_PID  ;                           //wei 20250512

extern const unsigned int MSG_CMD_AUTOZSTART        ;                           //wei 20250512
extern const unsigned int MSG_CMD_AUTOZMOVE         ;                           //wei 20250512
extern const unsigned int MSG_CMD_AUTOZMOVEOK       ;                           //wei 20250512
extern const unsigned int MSG_CMD_AUTOZPASS         ;                           //wei 20250512
extern const unsigned int MSG_CMD_AUTOZREACHLIMIT   ;                           //wei 20250512
extern const unsigned int MSG_CMD_AUTOZOK           ;                           //wei 20250512
extern const unsigned int MSG_CMD_READAUTOZLIMIT    ;                           //wei 20250512
extern const unsigned int MSG_CMD_READZPOS          ;                           //wei 20250625
extern const unsigned int MSG_CMD_READZTORQUE       ;                           //wei 20250625
// AI(W906-ST02-C912) 20261003 (St02-E): MSG_CMD_RemoteStart =204 removed -- golden 906_0625 has no GPIB remote START (MessageDef ends at MSG_CMD_AUTOZOK =200; 912-only JerryYang 20260828 Qualcomm, RULINGS_20261002 #20 / #23-6); comment kept so the line count stays (St02 6fff0960 had added this line)
// AI(W906-ST02-C912) 20261003 (St02-E): MSG_CMD_RemoteStop =205 removed -- golden 906_0625 has no GPIB remote STOP (MessageDef ends at MSG_CMD_AUTOZOK =200; 912-only JerryYang 20260828 Qualcomm, RULINGS_20261002 #20 / #23-6); comment kept so the line count stays (St02 6fff0960 had added this line)
//---------------------------------------------------------------------------
typedef struct
{
    unsigned int iCommand;                                                      //Steven 20141008 : iTemp --> iCommand
    unsigned int Result[32];
    bool bError;
    bool bEchoStop;                                                             //ChungHung 20130326 add
    char cReturn[256];                                                          //kevin 20160307
    char GpibStatus[32];                                                        // kevin 20130425
    char GpibData[256];                                                         // kevin 20130425
    int  GPIBBin;                                                               //kevin 20140305 gpib bin 數量   //Steven 20151219 : 沒在用,拿來傳輸簡單的資料
    bool bOneCycle;                                                             //jou 2014-09-23 Tester Low Yield Handler need One Cycle & Alarm
}VM;
//------------------------------------------------------------------------------
typedef struct
{
    unsigned int iSendCommand;                                                  //Steven 20141008 : iTimeOutSecond --> iSendCommand
    unsigned int Site[32];
    bool bSimulate;
    bool bSupport32Bin;                                                         //Steven 20121112 : RS232支援32Bin  b8080 --> bSupport32Bin
    bool bCloseGpib;
    bool bTimeOutProcess;
    int  GpibAddress;
    bool MachineISRun;
    bool IsTest;
    bool bGpibMode;
    int  iLotStatus;
    HWND HandlerHwnd;
    HWND GpibHwnd;
    int  GPIBBin;                                                               //kevin 20140305 gpib bin 數量
    int  iStatus[17];                                                           //JerryYang 20151109 add for 力成 回覆tester機台狀態
    char Message[2048];                                                         //Steven 20150713 : for 2D Code
//    #ifdef AMD_Version
    char UseSiteMapData[256];                                                   //Ifor 20201030 add:送Site Mapping 資料給GPIB
    char asATC_TYPE[32];                                                        //Ifor 20230828 add:送ATC TYPE 資料給GPIB
    char MultiMessage[4096];                                                    //Ifor 20241003 add: for Multi 2D Code
//    #endif
}MV;
//------------------------------------------------------------------------------
//  從Refresh程式收訊息  kevin 20110317
//------------------------------------------------------------------------------
typedef struct
{
    unsigned int iType;                                                         // 0 : normal model
    char cSendData[300];
    bool bError;
}VM1;
//------------------------------------------------------------------------------
//  送訊息給Refresh程式   kevin 20110317
//------------------------------------------------------------------------------
typedef struct
{
    unsigned int iType;                                                         // 0 : normal model
    char cSendData[300];
    bool bError;
}MV1;
//---------------------------------------------------------------------------
//  從CCD程式收訊息  kevin 20110811
//------------------------------------------------------------------------------
typedef struct
{
    unsigned int iType;                                                         // 0 : normal model
    char cSendData[300];
    int  iCustomer;                                                             //客戶碼
    int  iRespondState;                                                         //回應狀態  CDHT_EXISTENCE  CDHT_FILE_READY
    bool bError;
}VM2;
//------------------------------------------------------------------------------
//  送訊息給CCD程式   kevin 20110811
//------------------------------------------------------------------------------
typedef struct
{
    unsigned int iType;                                                         // 0 : normal model
    char cSendData[300];
    int  iCustomer;
    int  iCCDDoState;                                                           //讓CCD作事狀態   CDHT_EXISTENCE  CDHT_FILE_READY
    bool bError;
}MV2;
//---------------------------------------------------------------------------
//For Handler----------
extern VM *HGpib2Handler;
extern MV HHandler2Gpib;

//For GPIB-------------
extern VM GGpib2Handler;
extern MV *GHandler2Gpib;
//---------------------
extern VM1 *RefeshReceve;
extern MV1 RefeshSend;
extern VM2 *CCDReceve;
extern MV2 CCDSend;
//---------------------------------------------------------------------------
extern AnsiString W906_asObsGPIBVersion; extern AnsiString W906_asObsTTLRS232Version;   //AI(W906-H022-T3) 20260930: MessageDef.cpp end of file (Observer web mirrors)
#endif
