// BarCode/BarCode.cpp -- AI(W906-BC-READFILE) 20260923
//
// golden: HT9011UC_Code_V3.33.906.0_20260618/BarCode/BarCode.cpp
//
// Holds TfBarCode's own bodies.  Two things landed here today:
//
//  (1) MOVED, unchanged apart from the class name: the 21 forwarding bodies and
//      the `fBarCode` object, which used to sit at aHotPlateSubstrate.cpp:1150-1187
//      under the name TfBarCode_Shim.  Nothing about them changed -- same
//      forwarding targets, same object, same library (ht9045_sm), so no call
//      site anywhere moved or changed meaning.  They are here because this is
//      golden's file for them; see BarCode/BarCode.h for the naming rationale.
//
//  (2) NEW: TfBarCode::ReadFile(), golden :663-1041.
//
// The nine BarCode_* includes below are copied verbatim from
// aHotPlateSubstrate.cpp:36-47 -- they are what the forwarding bodies need.

#include "BarCode/BarCode.h"

#include "BarCode/BarCode_Helpers.h"          // BarCode_IsSHT2DIDScanFinish
#include "BarCode/BarCode_Bottom2DID.h"       // BarCode_InitBottom2DIDScan / BarCode_DoBottom2DIDScan
#include "BarCode_Bottom2DID8CCD.h"           // BarCode_DoBottom2DID_8CCD_Scan
#include "BarCode/BarCode_Shuttle1_Scan.h"    // InitialBarcodeScanInShuttle1/OutShuttle1, DoBarcode{Scan,Trigger}InShuttle_1, ...
#include "BarCode/BarCode_Shuttle2_Scan.h"    // BarCode_Sh2_*
#include "BarCode/BarCode_Shuttle1_CCDScan.h" // BarCode_DoBarcodeCCDInShuttle_1
#include "BarCode/BarCode_Shuttle2_CCDScan.h" // BarCode_DoBarcodeCCDInShuttle_2
#include "BarCode/BarCode_Shuttle2_ScanRemainder1.h" // BarCode_Sh2_DoBarcodeScanInShuttle_2
#include "BarCode/BarCode_Shuttle2_ScanRemainder2.h" // BarCode_Sh2_DoShuttleFloatCheck_2

// AI(W906-BC-READFILE) 20260923: ReadFile's non-BarCode dependencies.
#include "forms/fLotInfo.h"   // fLotInfo->SetCheckCodeByLot (golden :834/:836),
                              //   fLotInfo->edtLotVerify    (golden :857/:858)
#include "forms/fMain.h"      // fMain->cbSetupFileName (golden :974),
                              //   fMain->SendMSG_CMD   (golden :1026-1033)
#include "MessageDef.h"       // MSG_CMD_{Enable,Disable}{BarCode,Pin1Function}

//==============================================================================
//  (D) [W6.2b] fBarCode shim (golden BarCode.h TfBarCode) -- offline: no CCD,
//      every bottom-2DID scan reports "finished" so the additional-fn SM
//      advances rather than hangs.
//==============================================================================
// AI(W5-BarCode-Integrate) 20260711: delegate to the real W5-BarCode translate-
// unit bodies (see includes above).
// AI(W5-BarCode-Final-Integrate) 20260711: DoBarcodeScanInShuttle_2/
// DoShuttleFloatCheck_2 were the 2-method "REMAINING / HANDED OFF" hand-off
// flagged by BarCode_Shuttle2_Scan.h -- both are now delivered by 2 separate
// completion units (BarCode_Shuttle2_ScanRemainder1/2) and wired below.
// TfBarCode is 20/20 real methods as of this integrate.
void TfBarCode::InitBottom2DIDScan()       { BarCode_InitBottom2DIDScan(); }
bool TfBarCode::DoBottom2DIDScan()         { return BarCode_DoBottom2DIDScan(); }
bool TfBarCode::DoBottom2DID_8CCD_Scan()   { return BarCode_DoBottom2DID_8CCD_Scan(); }

// -- W6.5 ADD: in/out-shuttle 2D-barcode + shuttle-float-check bodies the carry
//    engine derefs (golden BarCode/BarCode.h).  20 of 20 now delegate to real
//    W5-BarCode bodies (see W5-BarCode-Final-Integrate note above).
void TfBarCode::InitialBarcodeScanInShuttle1(bool bClear2DID)  { ::InitialBarcodeScanInShuttle1(bClear2DID); }      // golden :799
void TfBarCode::InitialBarcodeScanInShuttle2(bool bClear2DID)  { BarCode_Sh2_InitialBarcodeScanInShuttle2(bClear2DID); } // golden :800
void TfBarCode::InitialBarcodeScanOutShuttle1()                { ::InitialBarcodeScanOutShuttle1(); }              // golden :801
void TfBarCode::InitialBarcodeScanOutShuttle2()                { BarCode_Sh2_InitialBarcodeScanOutShuttle2(); }    // golden :802
bool TfBarCode::DoBarcodeScanInShuttle_1(bool bErrorSkip)     { return ::DoBarcodeScanInShuttle_1(bErrorSkip); }  // golden :803
bool TfBarCode::DoBarcodeScanInShuttle_2(bool bErrorSkip)     { return BarCode_Sh2_DoBarcodeScanInShuttle_2(bErrorSkip); } // golden :804
bool TfBarCode::DoBarcodeTriggerInShuttle_1()                 { return ::DoBarcodeTriggerInShuttle_1(); }         // golden :805
bool TfBarCode::DoBarcodeTriggerInShuttle_2()                 { return BarCode_Sh2_DoBarcodeTriggerInShuttle_2(); } // golden :806
bool TfBarCode::DoBarcodeCCDInShuttle_1(bool bVerify)         { return BarCode_DoBarcodeCCDInShuttle_1(bVerify); } // golden :807
bool TfBarCode::DoBarcodeCCDInShuttle_2(bool bVerify)         { return BarCode_DoBarcodeCCDInShuttle_2(bVerify); } // golden :808
bool TfBarCode::DoBarcodeScanOutShuttle_1()                   { return ::DoBarcodeScanOutShuttle_1(); }           // golden :810
bool TfBarCode::DoBarcodeScanOutShuttle_2()                   { return BarCode_Sh2_DoBarcodeScanOutShuttle_2(); } // golden :811
void TfBarCode::InitialShuttleFloatCheck1()                   { ::InitialShuttleFloatCheck1(); }                  // golden :888
void TfBarCode::InitialShuttleFloatCheck2()                   { BarCode_Sh2_InitialShuttleFloatCheck2(); }        // golden :889
bool TfBarCode::DoShuttleFloatCheck_1()                       { return ::DoShuttleFloatCheck_1(); }               // golden :890
bool TfBarCode::DoShuttleFloatCheck_2()                       { return BarCode_Sh2_DoShuttleFloatCheck_2(); }        // golden :891
bool TfBarCode::IsSHT2DIDScanFinish(int SHT)                  { return BarCode_IsSHT2DIDScanFinish(SHT); }        // golden :942
static TfBarCode g_fBarCode;
TfBarCode *fBarCode = &g_fBarCode;


// ============================================================================
//  AI(W906-BC-READFILE) 20260923: golden BarCode/BarCode.cpp:663-1041 (379 L),
//  transcribed VERBATIM (cp950 -> UTF-8) except where marked inline.
//
//  ⚠ ZERO WRITES.  86 ReadIniData, no CheckAndReadIniData, no ReadWriteIni, no
//  WriteIniData -- counted, not assumed.  This is the only one of this week's
//  six ReadFile translations that cannot modify a machine file.
//
//  WHAT IT LOADS: 127 assignments into the GLOBAL TestIF_File -- the whole
//  barcode/2DID configuration (enable, bottom-2D, multi-2D map, check-sum,
//  duplicate-code policy, retry counts, trigger mode, shuttle float check,
//  2DID yield, server 2DID, white/allow lists).  Source is
//  GetRecipeFileName("HandlerCondition.Data"), section "Configuration" -- or
//  "Configuration_Barcode(XILINX)" for that customer (golden :673).
//
//  EIGHT GATES, and between them they gate ONE ReadIniData out of 86
//  (G-BC-EDBARCODENO, whose only destination is a widget).  The other seven are
//  absent widgets or absent golden TfBarCode members.  Every TestIF_File field
//  golden loads here is loaded here.
//
//  RETIRES GATE [E4] (SECSGEM/uHGemHT9045.cpp).  That gate's own note said it
//  "would be retired by adding ReadFile() to the shim, not by any link change".
//  That is exactly what happened -- except the class is no longer called a shim.
// ============================================================================
void TfBarCode::ReadFile()   // golden BarCode/BarCode.cpp:663-1041
{
    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    AnsiString sGroup="";

    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Alick 20170207 (wei) add for Xilinx Barcode Length save by machine
    {
        szDir="D:\\HT9045\\system\\Barcode.ini";
        sGroup="Configuration_Barcode(XILINX)";
    }
    else
    {
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
        sGroup="Configuration";
    }

    if(BAR_CODE_INSTALL==ebctUninstall)
        TestIF_File.bEnableBarCode      =false;
    else
        TestIF_File.bEnableBarCode      =ReadIniData(szDir, sGroup, "Bar Code", false);

    if(BOTTOM_2DID)                                                             //Steven 20190308 : Bottom 2D
        TestIF_File.bEnableBottom2D     =ReadIniData(szDir, sGroup, "Bottom 2D", false);
    else
        TestIF_File.bEnableBottom2D     =false;

    if(CosFunction.bEnableMulti2D)                                              //Steven 20200810 : 一個IC使用多個2DID
    {
        TestIF_File.bEnableMulti2D      =ReadIniData(szDir, sGroup, "Multi 2D", false);
        TestIF_File.dMulti2DXPitch      =ReadIniData(szDir, sGroup, "Multi 2D X Pitch", 0.0);
        TestIF_File.iMulti2DType        =ReadIniData(szDir, sGroup, "Multi 2D Typeh", 0);

        if(TestIF_File.iMulti2DType==e1x2In1CCD ||
           TestIF_File.iMulti2DType==e2x1In1CCD ||
           TestIF_File.iMulti2DType==e2x2In2CCD)
        {
            TestIF_File.iMulti2DCount=2;
        }
        else //if(TestIF_File.iMulti2DType==e2x1In2CCD)
        {
            TestIF_File.iMulti2DCount=1;
        }
//        else
//        {
//            TestIF_File.iMulti2DCount=4;
//        }

        if(TestIF_File.iMulti2DType==e1x2In1CCD)
        {
            TestIF_File.iMulti2DXItem   =2;
            TestIF_File.iMulti2DYItem   =1;
        }
        else if(TestIF_File.iMulti2DType==e2x1In2CCD)
        {
            TestIF_File.iMulti2DXItem   =1;
            TestIF_File.iMulti2DYItem   =1;
        }
        else if(TestIF_File.iMulti2DType==e2x1In1CCD)
        {
            TestIF_File.iMulti2DXItem   =1;
            TestIF_File.iMulti2DYItem   =2;
        }
        else if(TestIF_File.iMulti2DType==e2x2In2CCD)
        {
            TestIF_File.iMulti2DXItem   =2;
            TestIF_File.iMulti2DYItem   =1;
        }

        for(int i=0; i<2; i++)
        {
            for(int j=0; j<2; j++)
            {
                S.sprintf("Multi 2D Map %d-%d", i+1, j+1);
                if(TestIF_File.iMulti2DType==e1x2In1CCD && i==1)
                    TestIF_File.iMulti2DMap[i][j]=0;
                else if((TestIF_File.iMulti2DType==e2x1In1CCD || TestIF_File.iMulti2DType==e2x1In2CCD) && j==1)
                    TestIF_File.iMulti2DMap[i][j]=0;
                else
                    TestIF_File.iMulti2DMap[i][j]=ReadIniData(szDir, sGroup, S, 0);
            }
        }
    }
    else
    {
        TestIF_File.iMulti2DCount       =1;
        TestIF_File.iMulti2DXItem       =1;
        TestIF_File.iMulti2DYItem       =1;
        TestIF_File.bEnableMulti2D      =false;
    }

    TestIF_File.dBottom2DOffsetX        =ReadIniData(szDir, sGroup, "Bottom 2D Offset X",  0.0);
    TestIF_File.dBottom2DOffsetY        =ReadIniData(szDir, sGroup, "Bottom 2D Offset Y",  0.0);

    if(TestIF_File.bEnableBottom2D)
        TestIF_File.bEnableBarCode      =true;

    TestIF_File.iBarCodeDelay           =ReadIniData(szDir, sGroup, "Bar Code Delay Time", 10000);
    TestIF_File.iBarCodePosDelay        =ReadIniData(szDir, sGroup, "Bar Code Pos Delay Time", 50);                 //wei 20151126
    TestIF_File.iBarCodePos1Delay       =ReadIniData(szDir, sGroup, "Bar Code Pos1 Delay Time", 50);                //wei 20151126

    TestIF_File.iBarCodeMinLength       =ReadIniData(szDir, sGroup, "Bar Code Min Length", 5);                      //wei 20151127 字元數比對
    TestIF_File.iBarCodeMaxLength       =ReadIniData(szDir, sGroup, "Bar Code Max Length", 30);

    TestIF_File.iCheckSumLength         =ReadIniData(szDir, sGroup, "Bar Code Check Sum Length" , 17);              //KaiChen 20191121 ：中壢日月光 2D Check Sum
    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        TestIF_File.bCheckSum            =ReadIniData(szDir, sGroup, "Bar Code Check Sum"        , false);          //KaiChen 20191121 ：中壢日月光 2D Check Sum
    }
    else
    {
        TestIF_File.bCheckSum           =false;
#if 0 // GATE(G-BC-CBCHECKSUM) -- cbCheckSum is a golden TfBarCode WIDGET; this class carries no widgets. Display only -- the paired TestIF_File.bCheckSum=false on the line above stays ACTIVE, so the DATA half of this else-arm is intact.
        cbCheckSum->Visible             =false;
#endif
    }

    TestIF_File.b2DIDAllowList          =ReadIniData(szDir, sGroup, "Check 2DID Allow List Function"        , false);           //JerryYang 20241104 : 支援2DID白名單功能

    TestIF_File.iConsecutiveFailure     =ReadIniData(szDir, sGroup, "Consecutive Failure", 3);                                  //wei 20160823 Consecutive Failure

    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                                                                           //Steven 20200909 : 將沒有2DID的IC的後續做法變成選項
        TestIF_File.iNoCodeDeviceToErr  =ReadIniData(szDir, sGroup, "Bar Code NoCodeDeviceToErr",  0);                          //Steven 20151221 : 將讀取異常的IC放到Error Bin
    else if(CUSTOMER_CODE==CC_ASE_CL || CosFunction.bBarcodeErrNoTestAndShowH)
        TestIF_File.iNoCodeDeviceToErr  =ReadIniData(szDir, sGroup, "Bar Code NoCodeDeviceToErr",  2);
    else
        TestIF_File.iNoCodeDeviceToErr  =ReadIniData(szDir, sGroup, "Bar Code NoCodeDeviceToErr",  1);

    if(CUSTOMER_CODE==CC_ASE_SG)                                                //Ifor 20260112 add:ASE_SG 要求不開啟
        TestIF_File.bNoCodeDeviceAutoSkip=false;
    else
        TestIF_File.bNoCodeDeviceAutoSkip   =ReadIniData(szDir, sGroup, "Bar Code NoCodeDeviceAutoSkip", true);                     //Steven 20151221 : 讀不到Code時,自動Skip跳下一顆

    if(CUSTOMER_CODE==CC_ASE_CL)                //JerryYang 20250120 : ASECL SONG要求
    {
        TestIF_File.bNoCodeDeviceAutoSkip=false;
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX)
        TestIF_File.iBarcodeRetryCount      =CheckRange(ReadIniData(szDir, sGroup, "Bar Code Auto Retry", 1), 1, 10);           //Steven 20151221 : 讀不到Code時,自動Retry的次數
    else
        TestIF_File.iBarcodeRetryCount      =CheckRange(ReadIniData(szDir, sGroup, "Bar Code Auto Retry", 0), 0, 10);           //Frank 20170508 (Steven) : 賽靈思最少要做一次

    TestIF_File.b2DUseUndefinedCMD      =ReadIniData(szDir, sGroup, "Bar Code Use Undefined CMD Mode", false);                  //Ifor 20151226 :改用 自行定義 Trigger Command
    TestIF_File.str2DTriggerONCMD       =ReadIniData(szDir, sGroup, "Bar Code Undefined CMD ON", AnsiString("LON"));            //Ifor 20151226 :Trigger ON Command
    TestIF_File.str2DTriggerOFFCMD      =ReadIniData(szDir, sGroup, "Bar Code Undefined CMD OFF", AnsiString("LOFF"));          //Ifor 20151226 :Trigger OFF Command

    TestIF_File.b2DTriggerMode          =ReadIniData(szDir, sGroup, "Bar Code Enable Trigger Mode", false);                     //Steven 20151225 : 改用拍完就跑的方式
    TestIF_File.i2DTriggerTime          =CheckRange(ReadIniData(szDir, sGroup, "Bar Code Trigger Time", 1000), 1000, 10000);    //Steven 20151225 : 拍照的等待時間 //Steven 20200513 : Bar Code Trigger Time最小值1000

    //**************
    //這部分會影響分Bin,如果要讓客戶可以選,要發信通知
    //**************
    if(IniConfig.bKoreaFunction)                                                //Steven 20171128 (Wei) : modify
        TestIF_File.bCheckCodeByShuttle =ReadIniData(szDir, sGroup, "Check duplicate code by shuttle", true);                   //Steven 20160428 : 檢查2D重複碼
    else
        TestIF_File.bCheckCodeByShuttle =true;
    //**************
    //這部分會影響分Bin,如果要讓客戶可以選,要發信通知
    //**************

    TestIF_File.iShtDuplicateRetryCnt   =ReadIniData(szDir, sGroup, "ShuttleDuplicateRertyCount", 0);                           //Steven 20160823 : 蝦頭重複碼要可以自動Retry
    TestIF_File.bCheckCodeByLot         =ReadIniData(szDir, sGroup, "Check duplicate code by lot", false);                      //Steven 20160428 : 檢查2D重複碼

    if(CUSTOMER_CODE==CC_ASE_CL && LastSet.iTester==ON_LINE)    //JerryYang 20250120 : ASECL SONG要求強制開啟
    {
        TestIF_File.bCheckCodeByLot=true;
    }

    if(fLotInfo!=NULL)                                                          //JimmyChiu 20211014 :   Show Duplication function "Enable" or "Disable" on the Lot Info.
    {
        if(TestIF_File.bCheckCodeByLot)
            fLotInfo->SetCheckCodeByLot(true);
        else
            fLotInfo->SetCheckCodeByLot(false);
    }

    if(CosFunction.bLotIDVerify==true)                                          //Steven 20240704 : Lot Verification function for ATK
    {
        TestIF_File.bLotIDVerify    =ReadIniData(szDir, "Lot Verification", "Enable", false);
        TestIF_File.sLotIDVerify    =ReadIniData(szDir, "Lot Verification", "Lot ID Name",      AnsiString(""));
        TestIF_File.sLotIDSubstr    =ReadIniData(szDir, "Lot Verification", "Lot ID Substr",    AnsiString(""));
        TestIF_File.iLotIDVerifyS   =ReadIniData(szDir, "Lot Verification", "Lot ID Start",     0);
        TestIF_File.iLotIDVerifyE   =ReadIniData(szDir, "Lot Verification", "Lot ID End",       0);
        TestIF_File.i2DIDStrStart   =ReadIniData(szDir, "Lot Verification", "2DID Start",       0);
        TestIF_File.i2DIDStrEnd     =ReadIniData(szDir, "Lot Verification", "2DID End",         0);
        TestIF_File.iLotIDLength    =TestIF_File.iLotIDVerifyE-TestIF_File.iLotIDVerifyS;
    }
    else
    {
        TestIF_File.bLotIDVerify=false;
    }

    if(fLotInfo!=NULL)
    {
        fLotInfo->edtLotVerify->Visible =TestIF_File.bLotIDVerify;
        fLotInfo->edtLotVerify->Text    =TestIF_File.sLotIDSubstr;
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX ||                                         //Steven 20170707 (wei) : Fixed 2DID for Korea
       CosFunction.b2DCodeCheckByCoustomerLot)                                  //Sam 20220223 : 2D Code Check by Coustomer Lot
    {
        TestIF_File.bCheckLotHaveCode   =ReadIniData(szDir, sGroup, "Check Lot have code", false);                  //wei 20160505 Barcode 比對Lot
    }
    else
    {
        TestIF_File.bCheckLotHaveCode   =false;
    }
    TestIF_File.bEnableConsecutiveFailure=ReadIniData(szDir, sGroup, "Enable Consecutive Failure", false);          //wei 20160823  Consecutive Failure
#if 0 // GATE(G-BC-EDBARCODENO) -- ed_BarCodeNo is a golden TfBarCode WIDGET.  ⚠ This is the ONLY one of the 86 ReadIniData calls that is gated, and it costs nothing: its sole destination IS the widget -- it writes no TestIF_File field, so no engine value is lost.  golden key: [<group>] "Bar Code Text".
    ed_BarCodeNo->Text                  =ReadIniData(szDir, sGroup, "Bar Code Text", AnsiString("ABCDEFGHIJKLMNOP"));
#endif

    TestIF_File.bRetryOffsetMove        =ReadIniData(szDir, sGroup, "Enable Retry Offset Move", false);             //wei 20161116 Retry時先退出再進去讀取
    TestIF_File.dRetryOffsetMove        =ReadIniData(szDir, sGroup, "Retry Offset Move mm", 0);                     //wei 20161116 Retry時先退出再進去讀取

    TestIF_File.bRetryShiftOffsetMove   =ReadIniData(szDir, sGroup, "Enable Retry Shift Offset Move", false);       //wei 20161116 Retry時先退出再進去讀取(前中後)
    TestIF_File.dRetryShiftOffsetMove   =ReadIniData(szDir, sGroup, "Retry Shift Offset Move mm", 0);               //wei 20161116 Retry時先退出再進去讀取(前中後)

    TestIF_File.b2DIDYield              =ReadIniData(szDir, sGroup, "Enable Check 2DID Yield",  false);             //Steven 20171222 (Wei) : Yield Alarm of 2DID
    TestIF_File.d2DIDYield              =ReadIniData(szDir, sGroup, "2DID Controlled Yield",      99.0);            //Steven 20171222 (Wei) : Yield Alarm of 2DID

    TestIF_File.i2DYieldIgnoreCnt       =ReadIniData(szDir, sGroup, "i2DYieldIgnoreCnt",      100);                 //JerryYang 20241104 : Ignore count變更為可以修改
    TestIF_File.bSetCloseSite2DIDtoEmpty=ReadIniData(szDir, sGroup, "bSetCloseSite2DIDtoEmpty",   false);           //Steven 20190313 : Close site 2DID set to empty

    if(CosFunction.bUse2DIDAllSiteFailSetToErrBin)                              //Steven 20200702 : All site 2DID fail改成可以開關
        TestIF_File.iEnableAllSite2DIDErr=ReadIniData(szDir, sGroup, "iEnableAllSite2DIDErr",   2);
    else if(IniConfig.bKoreaFunction==true)
        TestIF_File.iEnableAllSite2DIDErr=ReadIniData(szDir, sGroup, "iEnableAllSite2DIDErr",   1);
    else
        TestIF_File.iEnableAllSite2DIDErr=ReadIniData(szDir, sGroup, "iEnableAllSite2DIDErr",   0);

    if(CosFunction.b2DUseSubJobFunction==true)                                  //Ifor 20200807 add:In House 2D Use Sub Job Function
        TestIF_File.b2DUseSubJob=ReadIniData(szDir, sGroup, "b2DUseSubJob",   true);
    else
        TestIF_File.b2DUseSubJob=false;

    if(CosFunction.b2DUsePinInspection==true)                                   //Ifor 20240528 add:Pin1 Function
        TestIF_File.b2DUsePinInspection=ReadIniData(szDir, sGroup, "b2DUsePinInspection",   false);
    else
        TestIF_File.b2DUsePinInspection=false;

    if(CosFunction.b2DUseAnyCharFunction==true)                                 //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
        TestIF_File.b2DUseAnyChar=ReadIniData(szDir, sGroup, "b2DUseAnyChar",   true);
    else
        TestIF_File.b2DUseAnyChar=false;

    //==> Eastsun 20260526 #026-4.P3.T2a Pin1 INI read :KYEC
    if(CosFunction.b2DUsePinInspection)                                            //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
        TestIF_File.b2DUsePinInspection=ReadIniData(szDir, sGroup, "b2DUsePinInspection",   false);
    else
        TestIF_File.b2DUsePinInspection=false;
    //<== Eastsun 20260526 #026-4.P3.T2a

    if(SHT_FLOATING_CHK==0)                                                     //Steven 20160920 : IC置偏檢查
        TestIF_File.bEnableShtFloatChk  =false;
    else
        TestIF_File.bEnableShtFloatChk  =ReadIniData(szDir, sGroup, "Shuttle Float Check Enable",              false);
    TestIF_File.iSFCStartDelay          =ReadIniData(szDir, sGroup, "Shuttle Float Check Start Delay",         100);
    TestIF_File.iSFCExposureTimeOut     =ReadIniData(szDir, sGroup, "Shuttle Float Check Exposure Time Out",   100);
    TestIF_File.iSFCGetResultTimeOut    =ReadIniData(szDir, sGroup, "Shuttle Float Check Get Result Time Out", 100);
    TestIF_File.iSFCAutoRetry           =ReadIniData(szDir, sGroup, "Shuttle Float Check Auto Retry",          1);
    TestIF_File.bSFCUse2Photo           =ReadIniData(szDir, sGroup, "Shuttle Float Check Use 2 Photo",         false);
    TestIF_File.iSFCUse2PhotoOffset     =ReadIniData(szDir, sGroup, "Shuttle Float Check Use 2 Photo Offset",  10);

    TestIF_File.bSearch2DIDByLot        =ReadIniData(szDir, sGroup, "Search 2DID By Lot",                      false);  //Frank 20170316 (wei) add Search 2DID By Lot
    TestIF_File.b2DIDListErrorBin       =ReadIniData(szDir, sGroup, "Search 2DID By Lot Error Bin",            2);      //Steven 20190604 : 2DID不在List內的另外分bin
    if(CosFunction.bSortingBy2DList==true)                                      //Frank 20221122 : 2DID sorting for ATK
    {
        TestIF_File.bSortingBy2DIDList  =ReadIniData(szDir, sGroup, "Sorting By 2DID List",                    false);  //JerryYang 20190313
        TestIF_File.iActionOf2DNotInList=ReadIniData(szDir, sGroup, "iActionOf2DNotInList", 0);                         //Steven 20250707 : Action Of 2D Not In List
    }
    else
    {
        TestIF_File.bSortingBy2DIDList  =false;
        TestIF_File.iActionOf2DNotInList=0;
    }

#if 0 // GATE(G-BC-SAVESUMMARY) -- fMain->pnlSaveSummary has no port (0 hits tree-wide).  Pure visibility of the manual save-summary panel.  The whole #ifdef SOFT_SIMULTE / #else / #ifdef BETA_VERSION ladder is gated as one block because every arm assigns the same absent widget.
    #ifdef SOFT_SIMULTE
    fMain->pnlSaveSummary->Visible=true;
    #else
    fMain->pnlSaveSummary->Visible=(IniConfig.bSPILFunction && TestIF_File.bSortingBy2DIDList);           //Steven 20240604 : 2D Sort加上手動存Summary功能
        #ifdef BETA_VERSION
            if(IniConfig.bSPILFunction==false)
            {
                fMain->pnlSaveSummary->Visible=(CUSTOMER_CODE==CC_AMKOR_Korea);
            }
        #endif

    #endif
#endif

    if(CosFunction.bRead2DIDFromServer==true)                                   //Jimmychiu 20230925 : read 2did in json file
    {
        TestIF_File.bCheckCodeByServer2DID=ReadIniData(szDir, sGroup, "Check Code By Server 2DID",       false);
        TestIF_File.asMes2DID_URL         =ReadIniData(szDir, sGroup, "asMes2DID_URL",                   AnsiString(""));
#if 0 // GATE(G-BC-SERVERPATH1) -- lbfinalpathShow (widget) and GetBarcodeByServerData() (0 hits tree-wide). The two TestIF_File assignments above it -- bCheckCodeByServer2DID and asMes2DID_URL -- stay ACTIVE.
        lbfinalpathShow->Caption          =GetBarcodeByServerData();
#endif
    }
    else
    {
        TestIF_File.bCheckCodeByServer2DID=false;
#if 0 // GATE(G-BC-SERVERPATH2) -- same widget as G-BC-SERVERPATH1, else-arm.  TestIF_File.bCheckCodeByServer2DID=false above it stays ACTIVE.
        lbfinalpathShow->Caption="None";
#endif
    }

    if(CosFunction.bMakeWhite2DIDList==true)                                    //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    {
        TestIF_File.bChkMakeWhite2DIDList   =ReadIniData(szDir, sGroup, "Make White 2DID List",       false);
    }
    else
    {
        TestIF_File.bChkMakeWhite2DIDList   =false;
    }

    TestIF_File.b2DIDNotExist2Error     =ReadIniData(szDir, sGroup, "Search2DIDToErrorBin",                     false);     //JerryYang 20231218 : 2DID黑名單功能
    TestIF_File.bSaveFailImage          =ReadIniData(szDir, sGroup, "Save Fail Image",                          false);     //Frank 20170408 (Steven) add Save Fail Image
    TestIF_File.s2DFileName             =ReadIniData(szDir, sGroup, "2DFileName",       fMain->cbSetupFileName->Text);      //kevin 20210817 2D FILENAME

    TestIF_File.iSelectUseCCDSh1        =ReadIniData(szDir, sGroup, "Select Use CCD Sh1",                      0);          //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos
    TestIF_File.iSelectUseCCDSh2        =ReadIniData(szDir, sGroup, "Select Use CCD Sh2",                      1);          //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos

    TestIF_File.b2DIDStringFormat       =ReadIniData(szDir, sGroup, "String Format",                           0);          //RogerYang 20181222 新增String format選項

    TestIF_File.bUseHandShakeCommunication  =ReadIniData(szDir, sGroup, "Bar Code Use HandShake Communication",    false);  //Ifor 20190225 :add Bar Code Use HandShake Communication
    TestIF_File.i2DHandShakeTimeOut         =ReadIniData(szDir, sGroup, "Bar Code HandShake Time Out", 2000);               //Ifor 20190225 :add Bar Code Use HandShake Communication

    TestIF_File.bBarCodeInspReport          =ReadIniData(szDir, sGroup, "bBarCodeInspReport",                false);  //Eastsun 20260527 整合//Sam 20240426 : Add BarCoder Inspection Report

    //==> Eastsun 20260527 整合#028.AAL.P06 ReadIniData AutoAdjustLight :KYEC
    if(CosFunction.bUseBarcodeAutoAdjustLight==true)
        TestIF_File.bUseBarcodeAutoAdjustLight  =ReadIniData(szDir, sGroup, "Bar Code Use Auto Adjust Light",    false);
    else
        TestIF_File.bUseBarcodeAutoAdjustLight  =false;
    TestIF_File.iAutoAdjustLightTimeOut         =ReadIniData(szDir, sGroup, "Bar Code Auto Adjust Ligh Time Out", 600);
    //<== Eastsun 20260527 #028.AAL.P06

    //Ifor 20210407 add: 自製OCR
    //==>
    TestIF_File.i2DReadMultiLine         =ReadIniData(szDir, sGroup, "Bar Code Read Multi Line", 0);
    TestIF_File.i2D1stLineLength         =ReadIniData(szDir, sGroup, "Bar Code 1st Line Length", 8);
    TestIF_File.i2D2ndLineLength         =ReadIniData(szDir, sGroup, "Bar Code 2nd Line Length", 4);
    TestIF_File.as2DInsertString         =ReadIniData(szDir, sGroup, "Bar Code Insert String", AnsiString(""));
    //==> Eastsun 20260527 整合#027-1.MR.R1 bBarCodeMultiRecipe INI read :KYEC
    TestIF_File.bBarCodeMultiRecipe     =ReadIniData(szDir, sGroup, "bBarCodeMultiRecipe", false);   //Ifor 20241031 add:使用BarCode Multi Recipe
    //<== Eastsun 20260527 #027-1.MR.R1
    //<==
    //Ifor 20210407 add: 自製OCR

    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Steven 20170425 : 簡化2DID存檔
    {
        TestIF_File.iNoCodeDeviceToErr      =0;                                 //Steven 20151221 : 將讀取異常的IC放到Error Bin
        TestIF_File.bNoCodeDeviceAutoSkip   =false;                             //wei 20160331 XILINX Barcode NoCodeDeviceToErr and NoCodeDeviceAutoSkip 強制關閉

        if(BAR_CODE_INSTALL==ebctUseCCDMode)                                    //Alick 20170203 add XILINX CCD模式下強制打勾
            TestIF_File.bCheckCodeByShuttle =true;

        if(LastSet.iRealDummy!=DUMMY || LastSet.iTester!=OFF_LINE)              //wei 20161118 強制打開確認重複碼功能  //Alick 20170120 add 修正no tray nodevice時無法關閉重複碼
        {
             TestIF_File.bCheckCodeByLot=true;
             TestIF_File.bCheckLotHaveCode=true;
        }
    }

#if 0 // GATE(G-BC-SETVISIBLE) -- TfBarCode::SetVisible() -- a golden member this partial class does not carry.  It is a widget-visibility sweep over the barcode form; no TestIF_File field is written by it.
    SetVisible();
#endif

    if(BAR_CODE_INSTALL!=ebctUninstall)                                         //jou 20211029 : 需判斷是否有安裝2DID
    {
        if(TestIF_File.bEnableBarCode)                                          //Steven 20150713 : GPIB update to V2.01 for 2D Code   //wei 20151120
            fMain->SendMSG_CMD(MSG_CMD_EnableBarCode);
        else
            fMain->SendMSG_CMD(MSG_CMD_DisableBarCode);

        if(TestIF_File.b2DUsePinInspection)                                     //Ifor 20240528 add:Pin1 Function
            fMain->SendMSG_CMD(MSG_CMD_EnablePin1Function);
        else
            fMain->SendMSG_CMD(MSG_CMD_DisablePin1Function);

#if 0 // GATE(G-BC-OCRSTATE) -- fMain->ShowOCRState() has no port (0 hits tree-wide).  Display only.  The four fMain->SendMSG_CMD calls above it are NOT gated -- SendMSG_CMD and MSG_CMD_{Enable,Disable}{BarCode,Pin1Function} all exist here, so the barcode enable/disable message to the tester is LIVE.
        fMain->ShowOCRState(0);
#endif
    }

#if 0 // GATE(G-BC-TAIL) -- three golden TfBarCode members this partial class does not carry: Change2DSetupFile (already gated at forms/fLotInfo.cpp:5622 for the same reason), SetSFCCheckStepCount, mtBarcodeSetDefaultView.  All three run AFTER every ReadIniData, so TestIF_File is fully populated before this point; what is lost is re-deriving the shuttle-float-check step count and refreshing the form's default view.
    Change2DSetupFile();
    SetSFCCheckStepCount();
    mtBarcodeSetDefaultView();
#endif
}

// AI(W906-EVB10C-BC) 20260930: golden BarCode.cpp:46-47 file-scope globals (extern in golden BarCode.h:1008-1009; NOT added to the port's
//   BarCode/BarCode.h, which reaches 221 TUs through aHotPlateSubstrate.h -- the one reader today forward-declares them itself).
//   Written by golden TfBarCode::sbtExitClick :2413-2414 (the Exit button; port = FileRW/TestIF_File_BarCode.gen.inc BC_sbtExitClick via WS
//   form.event). golden's only reader is the State Record task ring (main.cpp:10412-10413); the port's two rows are still gated
//   (cStateRecord.cpp:1944-1949 GATE(W906-TASKLIST), Jimmy's -- not touched here). Zero-initialised like golden's globals.
int i2DIDCheckSH1Task;
int i2DIDCheckSH2Task;
