// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 TestIF_File_BarCode 的讀寫檔（HTEditList： → <recipe>\HandlerCondition.Data [Configuration]／[Lot Verification]、D:\HT9045\system\Barcode.ini [Configuration_Barcode(XILINX)]（CC_KYEC_XILINX）、AuthPath\UnloaderClipSetting.ini（elUnloaderClip，bReadClipCodeFromUnloader，未移植））。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\BarCode\BarCode.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden BarCode\BarCode.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 212 個（header 宣告 521 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cprod.h"
#include "Config.h"
#include "LastSet.h"
#include "cmydef.h"
#include "common.h"
#include "MachineType.h"
#include "CosFunction.h"
#include "vclcompat/SysUtils.h"
#include "MessageDef.h"
#include "forms/fMain.h"
#include "forms/fLotInfo.h"
#include "csystem.h"
#include "cinitial.h"
#include "database.h"

#ifdef SOFT_SIMULTE
static const bool kBC_SOFT_SIMULTE=true;    // golden InitSht2DCodeComPort 的 #ifndef SOFT_SIMULTE（見 replace）
#else
static const bool kBC_SOFT_SIMULTE=false;
#endif
// golden BarCode.h:83-91 enum eMulti2DType（移植樹在 BarCode/BarCode_Shuttle2_CCDScan.h:197，該標頭帶 aHotPlateSubstrate.h，// 與 HTEditList.h 重複定義 TList／uPlateInfo → 不 include，這裡照 golden 值宣告）
enum eMulti2DType { e1x2In1CCD=0, e2x1In1CCD=1, e2x1In2CCD=2, e2x2In1CCD=3, e2x2In2CCD=4, eMulti2DTypeTotal };

// golden BarCode\BarCode.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bShow=false;                        // golden BarCode.h:777（本 TU 自己的；移植樹 fBarCode 門面沒有這個成員）
static bool bStartTest[4]={false,false,false,false};   // golden BarCode.h:717
static bool bCSVCacheLoaded=false;              // golden BarCode.h:993
static HTEditList* elUnloaderClip=NULL;         // golden BarCode.h:982（客戶專屬，本 TU 不 new，見建構子的 replace）
static AnsiString GetUnloaderClipFileName(){return AnsiString("UnloaderClipSetting.ini");}   // golden BarCode.h:980
static TComboBox* Multi2DSiteCH[2][2]={{NULL,NULL},{NULL,NULL}};   // golden BarCode.cpp:49（檔案層級全域；建構子 :316-319 指到 cbAa／cbAb／cbBa／cbBb）
static void BC_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }

using filerw::EL;

// 前置宣告
static void BC_TfBarCode();
static void BC_FormShow();
static void BC_FormClose();
static void BC_SetVisible();
static void BC_ReadFile();
static void BC_DoIniDataToForm();
static void BC_spbSaveClick();
static void BC_SetMulti2DMap(int iType);
static void BC_rgMulti2DTypeClick();
static void BC_chkMulti2DIDClick();
static bool BC_CheckMulti2DMap();
static AnsiString BC_GetBarcodeByServerData();
static bool BC_JCETUseMakeWhite2DIDList();
static void BC_spbStartComClick();
static void BC_InitSht2DCodeComPort(int iStep);
static void BC_ReadUnloaderClipIni();
static void BC_WriteUnloaderClipIni();

// golden BarCode\BarCode.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void BC_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfBarCode", "cbAa")->Items;
      s->Add("---");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "cbAb")->Items;
      s->Add("---");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "cbBa")->Items;
      s->Add("---");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "cbBb")->Items;
      s->Add("---");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "cbReadMultiLine_2DBarcode")->Items;
      s->Add("1");
      s->Add("2");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
      s->Add("9");
      s->Add("10");
      s->Add("11");
      s->Add("12");
      s->Add("13");
      s->Add("14");
      s->Add("15");
      s->Add("16");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "coCommReader1")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "coCommReader2")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "coCommReader3")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TComboBox>("TfBarCode", "coCommReader4")->Items;
      s->Add("COM1");
      s->Add("COM2");
      s->Add("COM3");
      s->Add("COM4");
      s->Add("COM5");
      s->Add("COM6");
      s->Add("COM7");
      s->Add("COM8");
      s->Add("COM9");
      s->Add("COM10");
      s->Add("COM11");
      s->Add("COM12");
      s->Add("COM13");
      s->Add("COM14");
      s->Add("COM15");
      s->Add("COM16");
      s->Add("COM17");
      s->Add("COM18");
      s->Add("COM19");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgActionOf2DSort")->Items;
      s->Add("Alarm and one cycle");
      s->Add("Keep running");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgAllSiteFail")->Items;
      s->Add("Off");
      s->Add("Retry and alarm");
      s->Add("Put to error bin without test");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->Items;
      s->Add("1x2");
      s->Add("2x1 in 1 CCD");
      s->Add("2x1 in 2 CCD");
      s->Add("2x2 in 1 CCD");
      s->Add("2x2 in 2 CCD");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr")->Items;
      s->Add("Follow the test bin");
      s->Add("Test and put to error bin");
      s->Add("No-test and put to error bin");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgSelectSh1")->Items;
      s->Add("CCD1");
      s->Add("CCD2");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgSelectSh2")->Items;
      s->Add("CCD3");
      s->Add("CCD4");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rgStringFormat")->Items;
      s->Add("Normal ( XXXXXXXXX )");
      s->Add("Include Dash ( XXXX-XXXX )");
      s->Add("Include Dot ( XXXX.XXXX )");
      s->Add("Include Dot and Dash ( XXXX.XXXX-XXXX )");
      s->Add("Include Dash and Dot ( XXXX-XXXX.XXXX )");
      s->Add("[a-zA-Z0-9-: _.]");
    }
    { TStringList* s = EL<TRadioGroup>("TfBarCode", "rg_2DBarcodeWhichNum")->Items;
      s->Add("OutShuttle1-1");
      s->Add("OutShuttle1-2");
      s->Add("OutShuttle2-1");
      s->Add("OutShuttle2-2");
    }
}

// golden BarCode\BarCode.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kBC_ParentOf[][2] = {
    {"GroupBox1", "ts4CCD"},
    {"GroupBox2", "ts4CCD"},
    {"GroupBox3", "ts4CCD"},
    {"Label2D", "TabSheet1"},
    {"Panel7", "grpLotVerify"},
    {"TabSheet1", "pgcFunction"},
    {"TabSheet2", "pgcFunction"},
    {"btnGPIBReader4", "gb2"},
    {"cb2DIDAllowList", "TabSheet1"},
    {"cb2DUseAnyChar", "TabSheet2"},
    {"cb2DUseSubJob", "TabSheet1"},
    {"cb2DUseUndefinedCMD", "gb2DTriggerCmd"},
    {"cbAa", "grpMulti2DMap"},
    {"cbAb", "grpMulti2DMap"},
    {"cbAutoAdjustLight", "TabSheet2"},
    {"cbBa", "grpMulti2DMap"},
    {"cbBarCodeInspReport", "TabSheet2"},
    {"cbBarCodeMultiRecipe", "TabSheet2"},
    {"cbBarCodeSimulate", "gbSimu"},
    {"cbBarcodeCSVCompare", "TabSheet1"},
    {"cbBb", "grpMulti2DMap"},
    {"cbCheckCodeByLot", "TabSheet2"},
    {"cbCheckCodeByServer2DID", "gbCheckCodeByServer2DID"},
    {"cbCheckCodeByShuttle", "TabSheet2"},
    {"cbCheckEnable2DIDFunction", "TabSheet1"},
    {"cbCheckEnableBottom2DIDFunction", "TabSheet1"},
    {"cbCheckLotHaveCode", "TabSheet2"},
    {"cbCheckSum", "TabSheet1"},
    {"cbConsecutiveFailure", "TabSheet2"},
    {"cbEnableBarCode", "tsBarCodeSetting"},
    {"cbEnableShtFloatChk", "tsFloatSetting"},
    {"cbNoCodeDeviceAutoSkip", "TabSheet2"},
    {"cbReadMultiLine_2DBarcode", "tsOCR"},
    {"cbRetryOffsetMove", "TabSheet2"},
    {"cbRetryShiftOffsetMove", "TabSheet2"},
    {"cbSFCUse2Photo", "tsFloatSetting"},
    {"cbSaveFailImage", "TabSheet2"},
    {"cbSearch2DIDByLot", "TabSheet2"},
    {"cbSortingBy2DIDList", "grp2DIDSorting"},
    {"cbTriggerMode", "TabSheet2"},
    {"cbUseHandShakeCommunication", "TabSheet1"},
    {"cbUsePinInspection", "TabSheet2"},
    {"cbb2DIDListErrorBin", "TabSheet2"},
    {"chk2DIDYield", "TabSheet2"},
    {"chkBottom2DID", "tsBarCodeSetting"},
    {"chkCloseSite2DIDIsEmpty", "TabSheet2"},
    {"chkEnableLotVerify", "grpLotVerify"},
    {"chkMakeWhite2DIDList", "grpMakeWhite2DIDList"},
    {"chkMulti2DID", "tsBarCodeSetting"},
    {"chkSearch2DIDToError", "TabSheet1"},
    {"coCommReader1", "gb1"},
    {"coCommReader2", "gb1"},
    {"coCommReader3", "gb2"},
    {"coCommReader4", "gb2"},
    {"cobMain_BinMapping_Auto1", "tsBarcodeAuto"},
    {"cobMain_BinMapping_Auto2", "tsBarcodeAuto"},
    {"cobMain_BinMapping_Auto3", "tsBarcodeAuto"},
    {"cobMain_BinMapping_Auto4", "tsBarcodeAuto4"},
    {"cobMain_BinMapping_Auto5", "tsBarcodeAuto4"},
    {"cobMain_BinMapping_Auto6", "tsBarcodeAuto4"},
    {"cobMain_BinMapping_BinBox", "tsBarcodeBinBox"},
    {"cobMain_BinMapping_Fix1", "tsBarcodeFix"},
    {"cobMain_BinMapping_Fix2", "tsBarcodeFix"},
    {"cobMain_BinMapping_Fix3", "tsBarcodeFix"},
    {"cobMain_BinMapping_Fix4", "tsBarcodeFix4"},
    {"cobMain_BinMapping_Fix5", "tsBarcodeFix4"},
    {"cobMain_BinMapping_Fix6", "tsBarcodeFix4"},
    {"cobMain_BinMapping_Mag01", "tsMagazine1"},
    {"cobMain_BinMapping_Mag02", "tsMagazine1"},
    {"cobMain_BinMapping_Mag03", "tsMagazine1"},
    {"cobMain_BinMapping_Mag04", "tsMagazine1"},
    {"cobMain_BinMapping_Mag05", "tsMagazine1"},
    {"cobMain_BinMapping_Mag06", "tsMagazine1"},
    {"cobMain_BinMapping_Mag07", "tsMagazine1"},
    {"cobMain_BinMapping_Mag08", "tsMagazine1"},
    {"cobMain_BinMapping_Mag09", "tsMagazine2"},
    {"cobMain_BinMapping_Mag10", "tsMagazine2"},
    {"cobMain_BinMapping_Mag11", "tsMagazine2"},
    {"cobMain_BinMapping_Mag12", "tsMagazine2"},
    {"cobMain_BinMapping_Mag13", "tsMagazine2"},
    {"cobMain_BinMapping_Mag14", "tsMagazine2"},
    {"edAutoAdjustLightTimeOut", "TabSheet2"},
    {"edBarcodePosDelayTime", "TabSheet2"},
    {"edBarcodeRetryCount", "TabSheet2"},
    {"edBarcodeScanDelayTime", "TabSheet2"},
    {"edConsecutiveFailure", "TabSheet2"},
    {"edFirstDelay", "TabSheet2"},
    {"edHandShakeTimeOut", "TabSheet1"},
    {"edRetryOffsetMove", "TabSheet2"},
    {"edRetryShiftOffsetMove", "TabSheet2"},
    {"edSFCAutoRetry", "tsFloatSetting"},
    {"edSFCExposureTimeOut", "tsFloatSetting"},
    {"edSFCGetResultTimeOut", "tsFloatSetting"},
    {"edSFCStartDelay", "tsFloatSetting"},
    {"edSFCUse2PhotoOffset", "tsFloatSetting"},
    {"edShtDuplicateRetryCnt", "TabSheet2"},
    {"edShuttle_1A_Address", "gbATC"},
    {"edShuttle_1A_Port", "gbATC"},
    {"edShuttle_1B_Address", "GroupBox1"},
    {"edShuttle_1B_Port", "GroupBox1"},
    {"edShuttle_2A_Address", "GroupBox2"},
    {"edShuttle_2A_Port", "GroupBox2"},
    {"edShuttle_2B_Address", "GroupBox3"},
    {"edShuttle_2B_Port", "GroupBox3"},
    {"edTriggerTime", "TabSheet2"},
    {"ed_2D_YieldIgnoreCnt", "TabSheet2"},
    {"ed_BarCodeNo", "gbSimu"},
    {"edt1stLineLength_2DBarCode", "tsOCR"},
    {"edt2DEnd", "grpSubString"},
    {"edt2DFileName", "TabSheet1"},
    {"edt2DIDYield", "TabSheet2"},
    {"edt2DStart", "grpSubString"},
    {"edt2ndLineLength_2DBarCode", "tsOCR"},
    {"edtBarcodeMaxLength", "TabSheet2"},
    {"edtBarcodeMinLength", "TabSheet2"},
    {"edtBarcodeOFFCMD", "gb2DTriggerCmd"},
    {"edtBarcodeONCMD", "gb2DTriggerCmd"},
    {"edtCheckSumLength", "TabSheet2"},
    {"edtInsertString1stAnd2nd_2DBarCode", "tsOCR"},
    {"edtLotIDEnd", "grpSubString"},
    {"edtLotIDStart", "grpSubString"},
    {"edtLotIDSubstr", "grpSubString"},
    {"edtLotIDVerify", "grpSubString"},
    {"edtOffsetX", "grpBottom2D"},
    {"edtOffsetY", "grpBottom2D"},
    {"edtXPitch", "tsMulti2DID"},
    {"gb1", "tsCognexSetting"},
    {"gb2", "tsCognexSetting"},
    {"gb2DTriggerCmd", "pnlFunc2Left"},
    {"gbATC", "ts4CCD"},
    {"gbCheckCodeByServer2DID", "TabSheet1"},
    {"gbManualTest", "TabSheet1"},
    {"gbSelect2DIDUse", "TabSheet2"},
    {"gbSimu", "gbManualTest"},
    {"grp2DIDSorting", "pnlFunc2Left"},
    {"grpAllSiteFail", "pnlFunc2Left"},
    {"grpBottom2D", "pnlFunc2Left"},
    {"grpLotVerify", "TabSheet2"},
    {"grpMakeWhite2DIDList", "TabSheet1"},
    {"grpMulti2DMap", "tsMulti2DID"},
    {"grpSubString", "Panel7"},
    {"lbfinalpathShow", "gbCheckCodeByServer2DID"},
    {"lblCheckSum", "TabSheet2"},
    {"le2DID_ServerURL", "gbCheckCodeByServer2DID"},
    {"mtBarcodeInSh", "palInSht"},
    {"mtBarcodeOutSh", "palOutSht"},
    {"mtSFC_Label1", "tsFloatResult"},
    {"mtSFC_Shuttle", "tsFloatResult"},
    {"myInShuttleLabel", "palInSht"},
    {"myOutShuttleLabel", "palOutSht"},
    {"palCheck2DID", "tsResult"},
    {"palInSht", "tsResult"},
    {"palOutSht", "tsResult"},
    {"pc2DID", "ts2DID"},
    {"pcShtFloat", "tsFloating"},
    {"pgc2DID", "Panel55"},
    {"pgc2DMap", "tsBarcodeMapping"},
    {"pgcCCDSetting", "tsCCD"},
    {"pgcFunction", "tsBarCodeSetting"},
    {"pnlFunc2Left", "TabSheet1"},
    {"rgActionOf2DSort", "grp2DIDSorting"},
    {"rgAllSiteFail", "grpAllSiteFail"},
    {"rgMulti2DType", "tsMulti2DID"},
    {"rgNoCodeDeviceToErr", "TabSheet2"},
    {"rgSelectSh1", "gbSelect2DIDUse"},
    {"rgSelectSh2", "gbSelect2DIDUse"},
    {"rgStringFormat", "TabSheet2"},
    {"spbSave", "Panel1"},
    {"spbStartCom", "tsCognexSetting"},
    {"trShowBinMappingAuto1", "tsBarcodeAuto"},
    {"trShowBinMappingAuto2", "tsBarcodeAuto"},
    {"trShowBinMappingAuto3", "tsBarcodeAuto"},
    {"trShowBinMappingAuto4", "tsBarcodeAuto4"},
    {"trShowBinMappingAuto5", "tsBarcodeAuto4"},
    {"trShowBinMappingAuto6", "tsBarcodeAuto4"},
    {"trShowBinMappingBinBox", "tsBarcodeBinBox"},
    {"trShowBinMappingFix1", "tsBarcodeFix"},
    {"trShowBinMappingFix2", "tsBarcodeFix"},
    {"trShowBinMappingFix3", "tsBarcodeFix"},
    {"trShowBinMappingFix4", "tsBarcodeFix4"},
    {"trShowBinMappingFix5", "tsBarcodeFix4"},
    {"trShowBinMappingFix6", "tsBarcodeFix4"},
    {"trShowBinMappingMag01", "tsMagazine1"},
    {"trShowBinMappingMag02", "tsMagazine1"},
    {"trShowBinMappingMag03", "tsMagazine1"},
    {"trShowBinMappingMag04", "tsMagazine1"},
    {"trShowBinMappingMag05", "tsMagazine1"},
    {"trShowBinMappingMag06", "tsMagazine1"},
    {"trShowBinMappingMag07", "tsMagazine1"},
    {"trShowBinMappingMag08", "tsMagazine1"},
    {"trShowBinMappingMag09", "tsMagazine2"},
    {"trShowBinMappingMag10", "tsMagazine2"},
    {"trShowBinMappingMag11", "tsMagazine2"},
    {"trShowBinMappingMag12", "tsMagazine2"},
    {"trShowBinMappingMag13", "tsMagazine2"},
    {"trShowBinMappingMag14", "tsMagazine2"},
    {"ts2DID", "pgc2DID"},
    {"ts4CCD", "pgcCCDSetting"},
    {"ts8CCD", "pgcCCDSetting"},
    {"tsBarCodeSetting", "pc2DID"},
    {"tsBarcodeAuto", "pgc2DMap"},
    {"tsBarcodeAuto4", "pgc2DMap"},
    {"tsBarcodeBinBox", "pgc2DMap"},
    {"tsBarcodeFix", "pgc2DMap"},
    {"tsBarcodeFix4", "pgc2DMap"},
    {"tsBarcodeMapping", "pgc2DID"},
    {"tsCCD", "pgc2DID"},
    {"tsCCD_Unloader", "pgcCCDSetting"},
    {"tsCognexSetting", "pgc2DID"},
    {"tsFloatResult", "pcShtFloat"},
    {"tsFloatSetting", "pcShtFloat"},
    {"tsFloating", "pgc2DID"},
    {"tsLog", "pc2DID"},
    {"tsMagazine1", "pgc2DMap"},
    {"tsMagazine2", "pgc2DMap"},
    {"tsMulti2DID", "pc2DID"},
    {"tsOCR", "pc2DID"},
    {"tsResult", "pc2DID"},
    {"tsXML", "pgc2DID"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void BC_CreateContainerProxies()
{
    EL<TGroupBox>("TfBarCode", "GroupBox1");
    EL<TGroupBox>("TfBarCode", "GroupBox2");
    EL<TGroupBox>("TfBarCode", "GroupBox3");
    EL<TPanel>("TfBarCode", "Panel1");
    EL<TPanel>("TfBarCode", "Panel55");
    EL<TPanel>("TfBarCode", "Panel7");
    EL<TTabSheet>("TfBarCode", "TabSheet1");
    EL<TTabSheet>("TfBarCode", "TabSheet2");
    EL<TGroupBox>("TfBarCode", "gb1");
    EL<TGroupBox>("TfBarCode", "gb2");
    EL<TGroupBox>("TfBarCode", "gb2DTriggerCmd");
    EL<TGroupBox>("TfBarCode", "gbATC");
    EL<TGroupBox>("TfBarCode", "gbCheckCodeByServer2DID");
    EL<TGroupBox>("TfBarCode", "gbManualTest");
    EL<TGroupBox>("TfBarCode", "gbSelect2DIDUse");
    EL<TGroupBox>("TfBarCode", "gbSimu");
    EL<TGroupBox>("TfBarCode", "grp2DIDSorting");
    EL<TGroupBox>("TfBarCode", "grpAllSiteFail");
    EL<TGroupBox>("TfBarCode", "grpBottom2D");
    EL<TGroupBox>("TfBarCode", "grpLotVerify");
    EL<TGroupBox>("TfBarCode", "grpMakeWhite2DIDList");
    EL<TGroupBox>("TfBarCode", "grpMulti2DMap");
    EL<TGroupBox>("TfBarCode", "grpSubString");
    EL<TPanel>("TfBarCode", "palInSht");
    EL<TPanel>("TfBarCode", "palOutSht");
    EL<TPageControl>("TfBarCode", "pc2DID");
    EL<TPageControl>("TfBarCode", "pcShtFloat");
    EL<TPageControl>("TfBarCode", "pgc2DID");
    EL<TPageControl>("TfBarCode", "pgc2DMap");
    EL<TPageControl>("TfBarCode", "pgcCCDSetting");
    EL<TPageControl>("TfBarCode", "pgcFunction");
    EL<TPanel>("TfBarCode", "pnlFunc2Left");
    EL<TTabSheet>("TfBarCode", "ts2DID");
    EL<TTabSheet>("TfBarCode", "ts4CCD");
    EL<TTabSheet>("TfBarCode", "tsBarCodeSetting");
    EL<TTabSheet>("TfBarCode", "tsBarcodeAuto");
    EL<TTabSheet>("TfBarCode", "tsBarcodeAuto4");
    EL<TTabSheet>("TfBarCode", "tsBarcodeBinBox");
    EL<TTabSheet>("TfBarCode", "tsBarcodeFix");
    EL<TTabSheet>("TfBarCode", "tsBarcodeFix4");
    EL<TTabSheet>("TfBarCode", "tsBarcodeMapping");
    EL<TTabSheet>("TfBarCode", "tsCCD");
    EL<TTabSheet>("TfBarCode", "tsCognexSetting");
    EL<TTabSheet>("TfBarCode", "tsFloatResult");
    EL<TTabSheet>("TfBarCode", "tsFloatSetting");
    EL<TTabSheet>("TfBarCode", "tsFloating");
    EL<TTabSheet>("TfBarCode", "tsMagazine1");
    EL<TTabSheet>("TfBarCode", "tsMagazine2");
    EL<TTabSheet>("TfBarCode", "tsMulti2DID");
    EL<TTabSheet>("TfBarCode", "tsOCR");
    EL<TTabSheet>("TfBarCode", "tsResult");
    filerw::ELSetParents("TfBarCode", kBC_ParentOf, (int)(sizeof(kBC_ParentOf) / sizeof(kBC_ParentOf[0])));
}

// golden BarCode\BarCode.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void BC_DfmState()
{
    EL<TLabel>("TfBarCode", "Label2D")->Visible = false;
    EL<TControl>("TfBarCode", "TimerProcess2DData")->Enabled = false;
    EL<TCheckBox>("TfBarCode", "cbSaveFailImage")->Visible = false;
    EL<TCheckBox>("TfBarCode", "cbUsePinInspection")->Visible = false;
    EL<TEdit>("TfBarCode", "edt2DFileName")->Visible = false;
    EL<TGroupBox>("TfBarCode", "gbManualTest")->Visible = false;
    EL<TPanel>("TfBarCode", "palOutSht")->Visible = false;
    EL<TComboBox>("TfBarCode", "cbAa")->Text = "---";
    EL<TComboBox>("TfBarCode", "cbAa")->ItemIndex = 0;
    EL<TComboBox>("TfBarCode", "cbAb")->Text = "---";
    EL<TComboBox>("TfBarCode", "cbAb")->ItemIndex = 0;
    EL<TComboBox>("TfBarCode", "cbBa")->Text = "---";
    EL<TComboBox>("TfBarCode", "cbBa")->ItemIndex = 0;
    EL<TComboBox>("TfBarCode", "cbBb")->Text = "---";
    EL<TComboBox>("TfBarCode", "cbBb")->ItemIndex = 0;
    EL<TCheckBox>("TfBarCode", "cbEnableBarCode")->Checked = true;
    EL<TCheckBox>("TfBarCode", "cbEnableShtFloatChk")->Checked = true;
    EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Checked = true;
    EL<TComboBox>("TfBarCode", "cbReadMultiLine_2DBarcode")->Text = "1";
    EL<TComboBox>("TfBarCode", "cbReadMultiLine_2DBarcode")->ItemIndex = 0;
    EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin")->Text = "2";
    EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin")->ItemIndex = 2;
    EL<TComboBox>("TfBarCode", "coCommReader1")->Text = "COM15";
    EL<TComboBox>("TfBarCode", "coCommReader2")->Text = "COM16";
    EL<TComboBox>("TfBarCode", "coCommReader3")->Text = "COM17";
    EL<TComboBox>("TfBarCode", "coCommReader4")->Text = "COM18";
    EL<TEdit>("TfBarCode", "edAutoAdjustLightTimeOut")->Text = "600";
    EL<TEdit>("TfBarCode", "edBarcodePosDelayTime")->Text = "50";
    EL<TEdit>("TfBarCode", "edBarcodeRetryCount")->Text = "1";
    EL<TEdit>("TfBarCode", "edBarcodeScanDelayTime")->Text = "1";
    EL<TEdit>("TfBarCode", "edConsecutiveFailure")->Text = "5";
    EL<TEdit>("TfBarCode", "edFirstDelay")->Text = "50";
    EL<TEdit>("TfBarCode", "edHandShakeTimeOut")->Text = "2000";
    EL<TEdit>("TfBarCode", "edRetryOffsetMove")->Text = "5";
    EL<TEdit>("TfBarCode", "edRetryShiftOffsetMove")->Text = "5";
    EL<TEdit>("TfBarCode", "edSFCAutoRetry")->Text = "1";
    EL<TEdit>("TfBarCode", "edSFCExposureTimeOut")->Text = "10";
    EL<TEdit>("TfBarCode", "edSFCGetResultTimeOut")->Text = "10";
    EL<TEdit>("TfBarCode", "edSFCStartDelay")->Text = "10";
    EL<TEdit>("TfBarCode", "edSFCUse2PhotoOffset")->Text = "20";
    EL<TEdit>("TfBarCode", "edShtDuplicateRetryCnt")->Text = "0";
    EL<TEdit>("TfBarCode", "edShuttle_1A_Address")->Text = "127.000.000.100";
    EL<TEdit>("TfBarCode", "edShuttle_1A_Port")->Text = "1000";
    EL<TEdit>("TfBarCode", "edShuttle_1B_Address")->Text = "127.000.000.100";
    EL<TEdit>("TfBarCode", "edShuttle_1B_Port")->Text = "1000";
    EL<TEdit>("TfBarCode", "edShuttle_2A_Address")->Text = "127.000.000.100";
    EL<TEdit>("TfBarCode", "edShuttle_2A_Port")->Text = "1000";
    EL<TEdit>("TfBarCode", "edShuttle_2B_Address")->Text = "127.000.000.100";
    EL<TEdit>("TfBarCode", "edShuttle_2B_Port")->Text = "1000";
    EL<TEdit>("TfBarCode", "edTriggerTime")->Text = "1";
    EL<TEdit>("TfBarCode", "ed_2D_YieldIgnoreCnt")->Text = "100";
    EL<TEdit>("TfBarCode", "ed_BarCodeNo")->Text = "ABCD";
    EL<TEdit>("TfBarCode", "edt2DFileName")->Text = "1";
    EL<TEdit>("TfBarCode", "edt2DIDYield")->Text = "99.0";
    EL<TEdit>("TfBarCode", "edtBarcodeMaxLength")->Text = "5";
    EL<TEdit>("TfBarCode", "edtBarcodeMinLength")->Text = "5";
    EL<TEdit>("TfBarCode", "edtBarcodeOFFCMD")->Text = "LOFF";
    EL<TEdit>("TfBarCode", "edtBarcodeONCMD")->Text = "LON";
    EL<TEdit>("TfBarCode", "edtCheckSumLength")->Text = "5";
    EL<TEdit>("TfBarCode", "edtOffsetX")->Text = "0";
    EL<TEdit>("TfBarCode", "edtOffsetY")->Text = "0";
    EL<TRadioGroup>("TfBarCode", "rgActionOf2DSort")->ItemIndex = 0;
    EL<TRadioGroup>("TfBarCode", "rgAllSiteFail")->ItemIndex = 0;
    EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex = 0;
    EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr")->ItemIndex = 0;
    EL<TRadioGroup>("TfBarCode", "rgSelectSh1")->ItemIndex = 0;
    EL<TRadioGroup>("TfBarCode", "rgSelectSh2")->ItemIndex = 1;
    EL<TRadioGroup>("TfBarCode", "rgStringFormat")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kBC_EnableAll[] = {
    "GroupBox1", "GroupBox2", "GroupBox3", "Label2D", "Panel1", "Panel55",
    "Panel7", "TabSheet1", "TabSheet2", "btnGPIBReader4", "cb2DIDAllowList", "cb2DUseAnyChar",
    "cb2DUseSubJob", "cb2DUseUndefinedCMD", "cbAutoAdjustLight", "cbBarCodeInspReport", "cbBarCodeMultiRecipe", "cbBarCodeSimulate",
    "cbBarcodeCSVCompare", "cbCheckCodeByLot", "cbCheckCodeByServer2DID", "cbCheckCodeByShuttle", "cbCheckEnable2DIDFunction", "cbCheckEnableBottom2DIDFunction",
    "cbCheckLotHaveCode", "cbCheckSum", "cbConsecutiveFailure", "cbEnableBarCode", "cbEnableShtFloatChk", "cbNoCodeDeviceAutoSkip",
    "cbRetryOffsetMove", "cbRetryShiftOffsetMove", "cbSFCUse2Photo", "cbSaveFailImage", "cbSearch2DIDByLot", "cbSortingBy2DIDList",
    "cbTriggerMode", "cbUseHandShakeCommunication", "cbUsePinInspection", "chk2DIDYield", "chkBottom2DID", "chkCloseSite2DIDIsEmpty",
    "chkEnableLotVerify", "chkMakeWhite2DIDList", "chkMulti2DID", "chkSearch2DIDToError", "gb1", "gb2",
    "gb2DTriggerCmd", "gbATC", "gbCheckCodeByServer2DID", "gbManualTest", "gbSelect2DIDUse", "gbSimu",
    "grp2DIDSorting", "grpAllSiteFail", "grpBottom2D", "grpLotVerify", "grpMakeWhite2DIDList", "grpMulti2DMap",
    "grpSubString", "lbfinalpathShow", "lblCheckSum", "palCheck2DID", "palInSht", "palOutSht",
    "pc2DID", "pcShtFloat", "pgc2DID", "pgc2DMap", "pgcCCDSetting", "pgcFunction",
    "pnlFunc2Left", "rgActionOf2DSort", "rgAllSiteFail", "rgMulti2DType", "rgNoCodeDeviceToErr", "rgSelectSh1",
    "rgSelectSh2", "rgStringFormat", "spbSave", "spbStartCom", "ts2DID", "ts4CCD",
    "ts8CCD", "tsBarCodeSetting", "tsBarcodeAuto", "tsBarcodeAuto4", "tsBarcodeBinBox", "tsBarcodeFix",
    "tsBarcodeFix4", "tsBarcodeMapping", "tsCCD", "tsCCD_Unloader", "tsCognexSetting", "tsFloatResult",
    "tsFloatSetting", "tsFloating", "tsLog", "tsMagazine1", "tsMagazine2", "tsMulti2DID",
    "tsOCR", "tsResult", "tsXML",
};

// 存檔流程（spbSaveClick）讀的替身 83 個 —— FileRW/TestIF_File_BarCode.cpp 用它決定頁面必須送哪些值
static const char* const kBC_SaveReads[] = {
    "cb2DIDAllowList", "cb2DUseAnyChar", "cb2DUseSubJob", "cb2DUseUndefinedCMD", "cbAutoAdjustLight", "cbBarCodeInspReport",
    "cbBarCodeMultiRecipe", "cbBarCodeSimulate", "cbBarcodeCSVCompare", "cbCheckCodeByLot", "cbCheckCodeByServer2DID", "cbCheckCodeByShuttle",
    "cbCheckEnable2DIDFunction", "cbCheckEnableBottom2DIDFunction", "cbCheckLotHaveCode", "cbCheckSum", "cbConsecutiveFailure", "cbEnableBarCode",
    "cbEnableShtFloatChk", "cbNoCodeDeviceAutoSkip", "cbReadMultiLine_2DBarcode", "cbRetryOffsetMove", "cbRetryShiftOffsetMove", "cbSFCUse2Photo",
    "cbSaveFailImage", "cbSearch2DIDByLot", "cbSortingBy2DIDList", "cbTriggerMode", "cbUseHandShakeCommunication", "cbUsePinInspection",
    "cbb2DIDListErrorBin", "chk2DIDYield", "chkBottom2DID", "chkCloseSite2DIDIsEmpty", "chkEnableLotVerify", "chkMakeWhite2DIDList",
    "chkMulti2DID", "chkSearch2DIDToError", "edAutoAdjustLightTimeOut", "edBarcodePosDelayTime", "edBarcodeRetryCount", "edBarcodeScanDelayTime",
    "edConsecutiveFailure", "edFirstDelay", "edHandShakeTimeOut", "edRetryOffsetMove", "edRetryShiftOffsetMove", "edSFCAutoRetry",
    "edSFCExposureTimeOut", "edSFCGetResultTimeOut", "edSFCStartDelay", "edSFCUse2PhotoOffset", "edShtDuplicateRetryCnt", "edTriggerTime",
    "ed_2D_YieldIgnoreCnt", "ed_BarCodeNo", "edt1stLineLength_2DBarCode", "edt2DEnd", "edt2DFileName", "edt2DIDYield",
    "edt2DStart", "edt2ndLineLength_2DBarCode", "edtBarcodeMaxLength", "edtBarcodeMinLength", "edtBarcodeOFFCMD", "edtBarcodeONCMD",
    "edtCheckSumLength", "edtInsertString1stAnd2nd_2DBarCode", "edtLotIDEnd", "edtLotIDStart", "edtLotIDSubstr", "edtLotIDVerify",
    "edtOffsetX", "edtOffsetY", "edtXPitch", "le2DID_ServerURL", "rgActionOf2DSort", "rgAllSiteFail",
    "rgMulti2DType", "rgNoCodeDeviceToErr", "rgSelectSh1", "rgSelectSh2", "rgStringFormat",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void BC_CreateSaveProxies()
{
    EL<TCheckBox>("TfBarCode", "cb2DIDAllowList");
    EL<TCheckBox>("TfBarCode", "cb2DUseAnyChar");
    EL<TCheckBox>("TfBarCode", "cb2DUseSubJob");
    EL<TCheckBox>("TfBarCode", "cb2DUseUndefinedCMD");
    EL<TCheckBox>("TfBarCode", "cbAutoAdjustLight");
    EL<TCheckBox>("TfBarCode", "cbBarCodeInspReport");
    EL<TCheckBox>("TfBarCode", "cbBarCodeMultiRecipe");
    EL<TCheckBox>("TfBarCode", "cbBarCodeSimulate");
    EL<TCheckBox>("TfBarCode", "cbBarcodeCSVCompare");
    EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot");
    EL<TCheckBox>("TfBarCode", "cbCheckCodeByServer2DID");
    EL<TCheckBox>("TfBarCode", "cbCheckCodeByShuttle");
    EL<TCheckBox>("TfBarCode", "cbCheckEnable2DIDFunction");
    EL<TCheckBox>("TfBarCode", "cbCheckEnableBottom2DIDFunction");
    EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode");
    EL<TCheckBox>("TfBarCode", "cbCheckSum");
    EL<TCheckBox>("TfBarCode", "cbConsecutiveFailure");
    EL<TCheckBox>("TfBarCode", "cbEnableBarCode");
    EL<TCheckBox>("TfBarCode", "cbEnableShtFloatChk");
    EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip");
    EL<TComboBox>("TfBarCode", "cbReadMultiLine_2DBarcode");
    EL<TCheckBox>("TfBarCode", "cbRetryOffsetMove");
    EL<TCheckBox>("TfBarCode", "cbRetryShiftOffsetMove");
    EL<TCheckBox>("TfBarCode", "cbSFCUse2Photo");
    EL<TCheckBox>("TfBarCode", "cbSaveFailImage");
    EL<TCheckBox>("TfBarCode", "cbSearch2DIDByLot");
    EL<TCheckBox>("TfBarCode", "cbSortingBy2DIDList");
    EL<TCheckBox>("TfBarCode", "cbTriggerMode");
    EL<TCheckBox>("TfBarCode", "cbUseHandShakeCommunication");
    EL<TCheckBox>("TfBarCode", "cbUsePinInspection");
    EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin");
    EL<TCheckBox>("TfBarCode", "chk2DIDYield");
    EL<TCheckBox>("TfBarCode", "chkBottom2DID");
    EL<TCheckBox>("TfBarCode", "chkCloseSite2DIDIsEmpty");
    EL<TCheckBox>("TfBarCode", "chkEnableLotVerify");
    EL<TCheckBox>("TfBarCode", "chkMakeWhite2DIDList");
    EL<TCheckBox>("TfBarCode", "chkMulti2DID");
    EL<TCheckBox>("TfBarCode", "chkSearch2DIDToError");
    EL<TEdit>("TfBarCode", "edAutoAdjustLightTimeOut");
    EL<TEdit>("TfBarCode", "edBarcodePosDelayTime");
    EL<TEdit>("TfBarCode", "edBarcodeRetryCount");
    EL<TEdit>("TfBarCode", "edBarcodeScanDelayTime");
    EL<TEdit>("TfBarCode", "edConsecutiveFailure");
    EL<TEdit>("TfBarCode", "edFirstDelay");
    EL<TEdit>("TfBarCode", "edHandShakeTimeOut");
    EL<TEdit>("TfBarCode", "edRetryOffsetMove");
    EL<TEdit>("TfBarCode", "edRetryShiftOffsetMove");
    EL<TEdit>("TfBarCode", "edSFCAutoRetry");
    EL<TEdit>("TfBarCode", "edSFCExposureTimeOut");
    EL<TEdit>("TfBarCode", "edSFCGetResultTimeOut");
    EL<TEdit>("TfBarCode", "edSFCStartDelay");
    EL<TEdit>("TfBarCode", "edSFCUse2PhotoOffset");
    EL<TEdit>("TfBarCode", "edShtDuplicateRetryCnt");
    EL<TEdit>("TfBarCode", "edTriggerTime");
    EL<TEdit>("TfBarCode", "ed_2D_YieldIgnoreCnt");
    EL<TEdit>("TfBarCode", "ed_BarCodeNo");
    EL<TEdit>("TfBarCode", "edt1stLineLength_2DBarCode");
    EL<TEdit>("TfBarCode", "edt2DEnd");
    EL<TEdit>("TfBarCode", "edt2DFileName");
    EL<TEdit>("TfBarCode", "edt2DIDYield");
    EL<TEdit>("TfBarCode", "edt2DStart");
    EL<TEdit>("TfBarCode", "edt2ndLineLength_2DBarCode");
    EL<TEdit>("TfBarCode", "edtBarcodeMaxLength");
    EL<TEdit>("TfBarCode", "edtBarcodeMinLength");
    EL<TEdit>("TfBarCode", "edtBarcodeOFFCMD");
    EL<TEdit>("TfBarCode", "edtBarcodeONCMD");
    EL<TEdit>("TfBarCode", "edtCheckSumLength");
    EL<TEdit>("TfBarCode", "edtInsertString1stAnd2nd_2DBarCode");
    EL<TEdit>("TfBarCode", "edtLotIDEnd");
    EL<TEdit>("TfBarCode", "edtLotIDStart");
    EL<TEdit>("TfBarCode", "edtLotIDSubstr");
    EL<TEdit>("TfBarCode", "edtLotIDVerify");
    EL<TEdit>("TfBarCode", "edtOffsetX");
    EL<TEdit>("TfBarCode", "edtOffsetY");
    EL<TLabeledEdit>("TfBarCode", "edtXPitch");
    EL<TLabeledEdit>("TfBarCode", "le2DID_ServerURL");
    EL<TRadioGroup>("TfBarCode", "rgActionOf2DSort");
    EL<TRadioGroup>("TfBarCode", "rgAllSiteFail");
    EL<TRadioGroup>("TfBarCode", "rgMulti2DType");
    EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr");
    EL<TRadioGroup>("TfBarCode", "rgSelectSh1");
    EL<TRadioGroup>("TfBarCode", "rgSelectSh2");
    EL<TRadioGroup>("TfBarCode", "rgStringFormat");
}

// golden BarCode\BarCode.cpp:131  TfBarCode::TfBarCode(TComponent* Owner)
static void BC_TfBarCode()
{

    bShow=false;   // golden BarCode\BarCode.cpp:134-314 取代：建構子前段：BarcodeCOM[]（RS232 讀碼器 TComm）、2D socket 收資料清單、2DID 名單／排序清單、CSV 快取、iBarCodeNo 計數、Bottom 8CCD 通道、TMyTray 顏色表、GPIB 測試旗標、tmr1、mtBarcodeSetDefaultView、edCCDAddress／Port 指標、TimerProcess2DData、LastBarcodeLog.ini（執行期 log 檔名）、dVisionVer、Barcode Tray record 指標 —— 全是執行期物件，不是表單讀寫檔狀態（移植樹的執行期部分在 BarCode/BarCode_*.cpp）。表單橋只留 bShow=false（:229）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:134-314 -- 建構子前段：BarcodeCOM[]（RS232 讀碼器 TComm）、2D socket 收資料清單、2DID 名單／排序清單、CSV 快取、iBarCodeNo 計數、Bottom 8CCD 通道、TMyTray 顏色表、GPIB 測試旗標、tmr1、mtBarcodeSetDefaultView、edCCDAddress／Port 指標、TimerProcess2DData、LastBarcodeLog.ini（執行期 log 檔名）、dVisionVer、Barcode Tray record 指標 —— 全是執行期物件，不是表單讀寫檔狀態（移植樹的執行期部分在 BarCode/BarCode_*.cpp）。表單橋只留 bShow=false（:229）
    BarcodeCOM[0]=EL<TControl>("TfBarCode", "Barcode_1");
    BarcodeCOM[1]=EL<TControl>("TfBarCode", "Barcode_2");
    BarcodeCOM[2]=EL<TControl>("TfBarCode", "Barcode_3");
    BarcodeCOM[3]=EL<TControl>("TfBarCode", "Barcode_4");
    iConntectionOkTask=0;

    iBottomConntectionOkTask1=0;                                                //KaiChen 20200513 : Bottom 2D 8CCD
    iBottomConntectionOkTask2=0;                                                //KaiChen 20200513 : Bottom 2D 8CCD

    bGetLF=false;
    bGetST=false;
    bGetGF=false;

    for(int i=0; i<BAR_CODE_COUNT; i++)
    {
        bBarcodeStartDelay      [i]     =false;
        bBarcodeError           [i]     =false;
        bBarcodeDataSaveReady   [i]     =false;
        bHasPin1Error           [i]     =false;                                 //==> Eastsun 20260526 #026-4.Pin1 init :KYEC
        iGetSE2                 [i]     =-1;                                    //Steven 20160120 : Auto Teach 2D Matrix
        iGetSE3                 [i]     =-1;                                    //Steven 20120120 : Auto Learn 2D ROI
        iGetSE4                 [i]     =-1;                                    //Steven 20120126 : 只拍一張, 只確認有無讀取
        bGetSE9                 [i]     =false;
        iCurrentStep            [i]     =-1;                                    //Steven 20160503 : 確認移動位置與拍照位置相同
        bStepError              [i]     =false;                                 //Steven 20160503 : 確認移動位置與拍照位置相同

        bHasMultiLineBarcode    [i]     =false;                                 //Ifor 20210407 add: 自製OCR

        Socket2DReceiveList[i]=new TStringList;
        Socket2DReceiveList[i]->Clear();

        map2DShuttleList[i].clear();
        BarCode_SaveImage[i]=new TMemoryStream();
    }
    map2DList.clear();
    EL<TControl>("TfBarCode", "list2DByLot")=new TStringList;
    EL<TControl>("TfBarCode", "list2DByLot")->Clear();

    EL<TControl>("TfBarCode", "list2DSorting")=new TStringList;                                              //Frank 20221122 : 2DID sorting for ATK
    EL<TControl>("TfBarCode", "list2DSorting")->Clear();

    map2DSortingList.clear();                                                   //Steven 20240515 : modified for 2D sort

    EL<TControl>("TfBarCode", "s2DSorting")=new TStringList;
    EL<TControl>("TfBarCode", "s2DSorting")->Clear();

    EL<TControl>("TfBarCode", "listError2DID")=new TStringList;                                              //JerryYang 20231218 : 2DID黑名單功能
    EL<TControl>("TfBarCode", "listError2DID")->Clear();

    EL<TControl>("TfBarCode", "list2DWhitle")=new TStringList;                                               //JerryYang 20241104 : 支援2DID白名單功能
    EL<TControl>("TfBarCode", "list2DWhitle")->Clear();

    EL<TControl>("TfBarCode", "list2DWhitleResult")=new TStringList;
    EL<TControl>("TfBarCode", "list2DWhitleResult")->Clear();

    EL<TControl>("TfBarCode", "list2DWhitleTemp")=new TStringList;
    EL<TControl>("TfBarCode", "list2DWhitleTemp")->Clear();
    //Ifor 20260512 add: Barcode CSV Compare cache
    EL<TControl>("TfBarCode", "slCSVPassCache") = new TStringList;
    EL<TControl>("TfBarCode", "slCSVPassCache")->Clear();
    bCSVCacheLoaded = false;
    //Ifor 20260610 add: Barcode CSV Compare - per-IC record
    asCSVActiveLotID = "";
    asCSVLogFileName = "";
    iCSVRecordCT = 0;

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<8; j++)
        {
            iBarCodeNo[i][j]=0;
        }
        bEnableCCDChannelConnect[i]=true;
    }

    if(BOTTOM_2DID==1 && BOTTOM_2DID_CCD==1)                                    //KaiChen 20200513 : Bottom 2D 8CCD
    {
        for(int i=0; i<BAR_CODE_COUNT; i++)
        {
            bEnableCCDChannelConnect[i]=true;
        }
    }
    iBottom_1=0;
    iBottom_2=1;
    iBottom_3=2;
    iBottom_4=3;
    iBottom_5=4;
    iBottom_6=5;
    iBottom_7=6;
    iBottom_8=7;

    iBarCode1_1=0;
    iBarCode1_2=1;
    iBarCode2_1=2;
    iBarCode2_2=3;
    bShow=false;

    EL<TControl>("TfBarCode", "myOutShuttleLabel")->SetColorMap(0, TColor(0x00DFD9CC));
    EL<TControl>("TfBarCode", "myOutShuttleLabel")->SetColorMap(1, clGreen);
    EL<TControl>("TfBarCode", "myOutShuttleLabel")->SetColorMap(2, clRed);
    EL<TControl>("TfBarCode", "myOutShuttleLabel")->SetColorMap(3, clBtnFace);

    EL<TControl>("TfBarCode", "mtBarcodeOutSh")->SetColorMap(0, TColor(0x00DFD9CC));
    EL<TControl>("TfBarCode", "mtBarcodeOutSh")->SetColorMap(1, clGreen);
    EL<TControl>("TfBarCode", "mtBarcodeOutSh")->SetColorMap(2, clRed);
    EL<TControl>("TfBarCode", "mtBarcodeOutSh")->SetColorMap(3, clBtnFace);

    EL<TControl>("TfBarCode", "myInShuttleLabel")->SetColorMap(0, TColor(0x00DFD9CC));
    EL<TControl>("TfBarCode", "myInShuttleLabel")->SetColorMap(1, clGreen);
    EL<TControl>("TfBarCode", "myInShuttleLabel")->SetColorMap(2, clRed);
    EL<TControl>("TfBarCode", "myInShuttleLabel")->SetColorMap(3, clBtnFace);

    EL<TControl>("TfBarCode", "mtBarcodeInSh")->SetColorMap(0, TColor(0x00DFD9CC));
    EL<TControl>("TfBarCode", "mtBarcodeInSh")->SetColorMap(1, clGreen);
    EL<TControl>("TfBarCode", "mtBarcodeInSh")->SetColorMap(2, clRed);
    EL<TControl>("TfBarCode", "mtBarcodeInSh")->SetColorMap(3, clBtnFace);

    EL<TControl>("TfBarCode", "mtSFC_Label1")->SetColorMap(0, TColor(0x00DFD9CC));
    EL<TControl>("TfBarCode", "mtSFC_Label1")->SetColorMap(1, clGreen);
    EL<TControl>("TfBarCode", "mtSFC_Label1")->SetColorMap(2, clRed);
    EL<TControl>("TfBarCode", "mtSFC_Label1")->SetColorMap(3, clBtnFace);

    EL<TControl>("TfBarCode", "mtSFC_Shuttle")->SetColorMap(0, TColor(0x00DFD9CC));
    EL<TControl>("TfBarCode", "mtSFC_Shuttle")->SetColorMap(1, clGreen);
    EL<TControl>("TfBarCode", "mtSFC_Shuttle")->SetColorMap(2, clRed);
    EL<TControl>("TfBarCode", "mtSFC_Shuttle")->SetColorMap(3, clBtnFace);

    bGPIBTestBarCodeError=false;
    bGPIBTestBarCodeFormatError=false;

    for(int i=0; i<32; i++)
        iBinReturnMess[i]="";

    EL<TControl>("TfBarCode", "tmr1")->Enabled=false;
    mtBarcodeSetDefaultView();

    edCCDAddress[iBarCode1_1]=EL<TEdit>("TfBarCode", "edShuttle_1A_Address");
    edCCDAddress[iBarCode1_2]=EL<TEdit>("TfBarCode", "edShuttle_1B_Address");
    edCCDAddress[iBarCode2_1]=EL<TEdit>("TfBarCode", "edShuttle_2A_Address");
    edCCDAddress[iBarCode2_2]=EL<TEdit>("TfBarCode", "edShuttle_2B_Address");
    edCCDPort[iBarCode1_1]=EL<TEdit>("TfBarCode", "edShuttle_1A_Port");
    edCCDPort[iBarCode1_2]=EL<TEdit>("TfBarCode", "edShuttle_1B_Port");
    edCCDPort[iBarCode2_1]=EL<TEdit>("TfBarCode", "edShuttle_2A_Port");
    edCCDPort[iBarCode2_2]=EL<TEdit>("TfBarCode", "edShuttle_2B_Port");
    EL<TControl>("TfBarCode", "TimerProcess2DData")->Enabled=true;
    iRunningMode=LastSet.iRunStartMode;                                         //Steven 20160728 : Log by running mode
    asBarcodeLogName=ReadIniData(asBarCodeLogPath+"\\LastBarcodeLog.ini", "Bar Code", "Last Barcode Log Name", AnsiString(""));
    #ifdef SOFT_SIMULTE
        dVisionVer=2;
    #else
        dVisionVer=1;
    #endif

    //jou 20190930 : Barcode Tray record file
    TTMyTray *temp_ShowBinMappingUnloadPtr[eTrayCount]=
                {   EL<TControl>("TfBarCode", "trShowBinMappingAuto1"), EL<TControl>("TfBarCode", "trShowBinMappingAuto2"), EL<TControl>("TfBarCode", "trShowBinMappingAuto3"), EL<TControl>("TfBarCode", "trShowBinMappingAuto4"), EL<TControl>("TfBarCode", "trShowBinMappingAuto5"), EL<TControl>("TfBarCode", "trShowBinMappingAuto6"),
                    EL<TControl>("TfBarCode", "trShowBinMappingFix1"),  EL<TControl>("TfBarCode", "trShowBinMappingFix2"),  EL<TControl>("TfBarCode", "trShowBinMappingFix3"),  EL<TControl>("TfBarCode", "trShowBinMappingFix4"),  EL<TControl>("TfBarCode", "trShowBinMappingFix5"),  EL<TControl>("TfBarCode", "trShowBinMappingFix6"),
                    EL<TControl>("TfBarCode", "trShowBinMappingFix1"),  EL<TControl>("TfBarCode", "trShowBinMappingFix2"),  EL<TControl>("TfBarCode", "trShowBinMappingFix3"),  EL<TControl>("TfBarCode", "trShowBinMappingFix4"),  EL<TControl>("TfBarCode", "trShowBinMappingFix5"),  EL<TControl>("TfBarCode", "trShowBinMappingFix6"),  EL<TControl>("TfBarCode", "trShowBinMappingBinBox"),
                    EL<TControl>("TfBarCode", "trShowBinMappingMag01"), EL<TControl>("TfBarCode", "trShowBinMappingMag02"), EL<TControl>("TfBarCode", "trShowBinMappingMag03"), EL<TControl>("TfBarCode", "trShowBinMappingMag04"), EL<TControl>("TfBarCode", "trShowBinMappingMag05"), EL<TControl>("TfBarCode", "trShowBinMappingMag06"), EL<TControl>("TfBarCode", "trShowBinMappingMag07"),
                    EL<TControl>("TfBarCode", "trShowBinMappingMag08"), EL<TControl>("TfBarCode", "trShowBinMappingMag09"), EL<TControl>("TfBarCode", "trShowBinMappingMag10"), EL<TControl>("TfBarCode", "trShowBinMappingMag11"), EL<TControl>("TfBarCode", "trShowBinMappingMag12"), EL<TControl>("TfBarCode", "trShowBinMappingMag13"), EL<TControl>("TfBarCode", "trShowBinMappingMag14")
                };

    TComboBox *temp_ShowBinMappingAuto_Index_Ptr[eTrayCount]=
                {   EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Auto1"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Auto2"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Auto3"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Auto4"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Auto5"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Auto6"),
                    EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix1"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix2"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix3"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix4"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix5"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix6"),
                    EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix1"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix2"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix3"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix4"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix5"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Fix6"),  EL<TComboBox>("TfBarCode", "cobMain_BinMapping_BinBox"),
                    EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag01"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag02"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag03"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag04"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag05"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag06"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag07"),
                    EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag08"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag09"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag10"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag11"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag12"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag13"), EL<TComboBox>("TfBarCode", "cobMain_BinMapping_Mag14")
                };

    for(int i=0; i<eTrayCount; i++)
    {
        ShowBinMappingUnloadPtr[i]      =temp_ShowBinMappingUnloadPtr[i];
        ShowBinMappingAuto_Index_Ptr[i] =temp_ShowBinMappingAuto_Index_Ptr[i];
    }

    for(int i=0; i<eTrayCount; i++)
    {
        ShowBinMappingAuto_Index_Ptr[i]->OnChange   =cobMain_BinMapping_Auto1Change;
        ShowBinMappingAuto_Index_Ptr[i]->Tag        =i;
    }
#endif // GATE (S12-C save)

    Multi2DSiteCH[0][0]=EL<TComboBox>("TfBarCode", "cbAa");                                                   //Steven 20200810 : 一個IC使用多個2DID
    Multi2DSiteCH[0][1]=EL<TComboBox>("TfBarCode", "cbAb");
    Multi2DSiteCH[1][0]=EL<TComboBox>("TfBarCode", "cbBa");
    Multi2DSiteCH[1][1]=EL<TComboBox>("TfBarCode", "cbBb");
    if(CosFunction.bReadClipCodeFromUnloader==true)
    {
    filerw::ELTodo("golden BarCode.cpp:322-323 CosFunction.bReadClipCodeFromUnloader: elUnloaderClip=new HTEditList + InitUnloaderClipEdtList() (UnloaderClipSetting.ini -> ccdUnloader[].sIP/sPort) not ported -- port has no ccdUnloader[] (uCCDUnloaderClip); elUnloaderClip stays NULL");   // golden BarCode\BarCode.cpp:322-323 取代：客戶專屬（bReadClipCodeFromUnloader：HONPREC_QC／JSSI）：new HTEditList＋InitUnloaderClipEdtList（:11537）註冊到 ccdUnloader[].sIP／sPort，移植樹沒有 ccdUnloader[]（uCCDUnloaderClip）→ 不建，elUnloaderClip 保持 NULL（golden Read／WriteUnloaderClipIni 自帶 NULL 守衛）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:322-323 -- 客戶專屬（bReadClipCodeFromUnloader：HONPREC_QC／JSSI）：new HTEditList＋InitUnloaderClipEdtList（:11537）註冊到 ccdUnloader[].sIP／sPort，移植樹沒有 ccdUnloader[]（uCCDUnloaderClip）→ 不建，elUnloaderClip 保持 NULL（golden Read／WriteUnloaderClipIni 自帶 NULL 守衛）
        elUnloaderClip    =new HTEditList;
        InitUnloaderClipEdtList();
#endif // GATE (S12-C save)
        EL<TTabSheet>("TfBarCode", "tsCCD_Unloader")->TabVisible=true;
    }
    else
    {
        EL<TTabSheet>("TfBarCode", "tsCCD_Unloader")->TabVisible=false;
    }

}

// golden BarCode\BarCode.cpp:332  TfBarCode::FormShow(TObject *Sender)
static void BC_FormShow()
{

    ;   // golden BarCode\BarCode.cpp:334-335 取代：Left／Top：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:334-335 -- Left／Top：視窗位置（HTML 不用）
    Left=10;                                                                    //Steven 20220407 : 避免跑到畫面外
    Top =0;
#endif // GATE (S12-C save)
    bShow=true;
    #ifdef SOFT_SIMULTE                                                         //Steven 20190604 : 非軟體模擬不顯示
    EL<TGroupBox>("TfBarCode", "gbManualTest")->Visible=(AccessLevel>=iDefHonPrecLevel);                      //jou 2014-06-19 Security Have 5 Level 3->iDefHonPrecLevel
    #else
    EL<TGroupBox>("TfBarCode", "gbManualTest")->Visible=(CUSTOMER_CODE==CC_HONPREC_QC && AccessLevel>=iDefHonPrecLevel);  //Steven 20260729 : QC mode can use cbBarCodeSimulate
    #endif

    EL<TPageControl>("TfBarCode", "pgc2DMap")->ActivePageIndex   =0;
    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBarCode", "pgc2DMap")->ActivePage        =EL<TTabSheet>("TfBarCode", "tsBarcodeAuto");
    EL<TPageControl>("TfBarCode", "pgcFunction")->ActivePageIndex=0;

    BC_DoIniDataToForm();
    bStartTest[3]=false;
    EL<TButton>("TfBarCode", "btnGPIBReader4")->Enabled=(HasICUnderMachine()==false);
    ;   // golden BarCode\BarCode.cpp:350-350 取代：mtBarcodeSetDefaultView（:1475）：純畫面（fLotInfo->sgBarcode 表頭字串、本表單 TMyTray 格子 mtBarcodeInSh／myInShuttleLabel 的格數與顏色），不讀寫檔、不改 TestIF_File；移植樹 TfLotInfo 門面沒有 sgBarcode
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:350-350 -- mtBarcodeSetDefaultView（:1475）：純畫面（fLotInfo->sgBarcode 表頭字串、本表單 TMyTray 格子 mtBarcodeInSh／myInShuttleLabel 的格數與顏色），不讀寫檔、不改 TestIF_File；移植樹 TfLotInfo 門面沒有 sgBarcode
    mtBarcodeSetDefaultView();
#endif // GATE (S12-C save)
    EL<TPageControl>("TfBarCode", "pgc2DID")->ActivePageIndex=0;
    EL<TGroupBox>("TfBarCode", "gbSelect2DIDUse")->Visible=false;

    if(CUSTOMER_CODE==CC_KYEC_XILINX)
    {
        EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr")->Enabled=false;
        EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Enabled=false;
    }
    else if(CUSTOMER_CODE==CC_SCK)                                              //Steven 20200225 : SCK想要獨立控管部分2DID功能
    {
        EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Enabled=(AccessLevel>=LevelSet.AccessLevel[161]);
    }
    else if(CUSTOMER_CODE==CC_ASE_SG)                                           //Ifor 20260112 add:ASE_SG 要求不開啟
    {
        EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Checked=false;
        EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Enabled=false;
    }

    if(TestIF_File.i2DIDFormat==eAMD)                                           //JerryYang 20200422 2DID format選項改用下拉選單
    {
        EL<TGroupBox>("TfBarCode", "gbManualTest")->Visible=false;
        EL<TRadioGroup>("TfBarCode", "rgStringFormat")->Visible=false;
    }
    else                                                                        //Steven 20210304 : 修正Visible=false之後就不顯示的問題
    {
        EL<TRadioGroup>("TfBarCode", "rgStringFormat")->Visible=true;
    }

    if(AccessLevel>=iDefEngineerLevel)                                          //Frank 20171011 add Shuttle Check 2DID Pos
        EL<TPanel>("TfBarCode", "palCheck2DID")->Visible=true;
    else
        EL<TPanel>("TfBarCode", "palCheck2DID")->Visible=false;

    if((TestIF_File.iTestMode<=QualSite1X4  ||
       IsNNMode()==NN_1Row))//   &&
       //BAR_CODE_INSTALL!=ebcUseOCR          )                                 //Ifor 20220322 add 自制OCR不使用 //Steven 20250808 : Mark for OCR
        EL<TGroupBox>("TfBarCode", "gbSelect2DIDUse")->Visible=true;
    else
        EL<TGroupBox>("TfBarCode", "gbSelect2DIDUse")->Visible=false;

    if(CosFunction.bBarcodeErrNoTestAndShowH)
        EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr")->Enabled=false;

    EL<TEdit>("TfBarCode", "edConsecutiveFailure")->Visible=(BAR_CODE_INSTALL==ebctInShtIntel ||          //JerryYang 20200616 2DID In-house版本支援累加fail alarm功能
                                   BAR_CODE_INSTALL==ebctEtherNetCCD ||         //Ifor 20190129 : add Cognex EtherNet 通訊
                                   BAR_CODE_INSTALL==ebctUseCCDMode ||
                                   BAR_CODE_INSTALL==ebcUseOCR);                //Ifor 20210407 add: 自製OCR

    EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode")->Visible  =((CUSTOMER_CODE==CC_KYEC_XILINX ||
                                    CosFunction.b2DCodeCheckByCoustomerLot) &&
                                   (BAR_CODE_INSTALL==ebctInShtIntel ||
                                    BAR_CODE_INSTALL==ebctUseCCDMode));         //Sam 20220223 : 2D Code Check by Coustomer Lot

    EL<TCheckBox>("TfBarCode", "cbConsecutiveFailure")->Visible=(BAR_CODE_INSTALL==ebctInShtIntel ||          //JerryYang 20200616 2DID In-house版本支援累加fail alarm功能
                                   BAR_CODE_INSTALL==ebctEtherNetCCD ||         //Ifor 20190129 : add Cognex EtherNet 通訊
                                   BAR_CODE_INSTALL==ebctUseCCDMode ||
                                   BAR_CODE_INSTALL==ebcUseOCR);                //Ifor 20210407 add: 自製OCR

    EL<TTabSheet>("TfBarCode", "tsLog")->TabVisible=(BAR_CODE_INSTALL==ebctInShtIntel ||
                       BAR_CODE_INSTALL==ebctEtherNetCCD ||
                       BAR_CODE_INSTALL==ebcUseOCR);                            //Ifor 20210407 add: 自製OCR

    //Steven 20170707 (wei) : Fixed for 2DID Display
    //==>
    EL<TCheckBox>("TfBarCode", "cbSearch2DIDByLot")->Visible      =(CUSTOMER_CODE==CC_KYEC_XILINX || CosFunction.bCanUseSearch2DIDByLot);
    EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin")->Visible    =(CUSTOMER_CODE==CC_KYEC_XILINX || CosFunction.bCanUseSearch2DIDByLot);    //Steven 20190604 : 2DID不在List內的另外分bin
    EL<TGroupBox>("TfBarCode", "grp2DIDSorting")->Visible         =CosFunction.bSortingBy2DList;              //Frank 20221122 : 2DID sorting for ATK
    EL<TCheckBox>("TfBarCode", "cbRetryOffsetMove")->Visible      =(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD || BAR_CODE_INSTALL==ebcUseOCR);  //Ifor 20190129 : add Cognex EtherNet 通訊    //Ifor 20210407 add: 自製OCR
    EL<TEdit>("TfBarCode", "edRetryOffsetMove")->Visible      =(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD || BAR_CODE_INSTALL==ebcUseOCR);  //Ifor 20190129 : add Cognex EtherNet 通訊    //Ifor 20210407 add: 自製OCR
    EL<TCheckBox>("TfBarCode", "cbRetryShiftOffsetMove")->Visible =(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD || BAR_CODE_INSTALL==ebcUseOCR);  //Ifor 20190129 : add Cognex EtherNet 通訊    //Ifor 20210407 add: 自製OCR
    EL<TEdit>("TfBarCode", "edRetryShiftOffsetMove")->Visible =(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctEtherNetCCD || BAR_CODE_INSTALL==ebcUseOCR);  //Ifor 20190129 : add Cognex EtherNet 通訊    //Ifor 20210407 add: 自製OCR
    EL<TCheckBox>("TfBarCode", "chk2DIDYield")->Visible           =(BAR_CODE_INSTALL==ebctUseCCDMode);
    EL<TEdit>("TfBarCode", "edt2DIDYield")->Visible           =(BAR_CODE_INSTALL==ebctUseCCDMode);
    EL<TEdit>("TfBarCode", "ed_2D_YieldIgnoreCnt")->Visible   =(BAR_CODE_INSTALL==ebctUseCCDMode);        //JerryYang 20241104 : Ignore count變更為可以修改
    EL<TCheckBox>("TfBarCode", "cb2DUseSubJob")->Visible          =CosFunction.b2DUseSubJobFunction;          //Ifor 20200807 add:In House 2D Use Sub Job Function
    EL<TCheckBox>("TfBarCode", "cb2DUseAnyChar")->Visible         =CosFunction.b2DUseAnyCharFunction;         //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
    //==> Eastsun 20260526 #026-4.PinN.PU4 Pin1 Visible :KYEC
    EL<TCheckBox>("TfBarCode", "cbUsePinInspection")->Visible     =CosFunction.b2DUsePinInspection;           //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
    //<== Eastsun 20260526 #026-4.PinN.PU4
    EL<TGroupBox>("TfBarCode", "gbCheckCodeByServer2DID")->Visible=CosFunction.bRead2DIDFromServer;           //Jimmychiu 20230925 : read 2did in json file
    EL<TGroupBox>("TfBarCode", "grpMakeWhite2DIDList")->Visible   =CosFunction.bMakeWhite2DIDList;            //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    EL<TCheckBox>("TfBarCode", "chkSearch2DIDToError")->Visible   =(CUSTOMER_CODE!=CC_JCET);                  //RogerYang 20251214 : JCET目前只有白名單，暫不開放
    //<==
    //Steven 20170707 (wei) : Fixed for 2DID Display

    EL<TTabSheet>("TfBarCode", "tsXML")->TabVisible=(CUSTOMER_CODE==CC_Murata);                               //Steven 20200409 : Murata 2DID比對功能
    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Steven 20170426 : 修正項目顯示
    {
        if(AccessLevel==iDefHonPrecLevel)                                       //Alick 20170223 (wei) add Xilinx 振偉需求只有最高權限hontech時，才能修改
        {
            EL<TEdit>("TfBarCode", "edtBarcodeMinLength")->Enabled=true;
            EL<TEdit>("TfBarCode", "edtBarcodeMaxLength")->Enabled=true;
        }
        else
        {
            EL<TEdit>("TfBarCode", "edtBarcodeMinLength")->Enabled=false;
            EL<TEdit>("TfBarCode", "edtBarcodeMaxLength")->Enabled=false;
        }

        if(AccessLevel>=iDefEngineerLevel)                                      //Frank 20171011 (Steven) add Shuttle Check 2DID Pos
            EL<TPanel>("TfBarCode", "palCheck2DID")->Visible=true;
        else
            EL<TPanel>("TfBarCode", "palCheck2DID")->Visible=false;
    }
    else
    {
        EL<TEdit>("TfBarCode", "edtBarcodeMinLength")->Enabled=true;
        EL<TEdit>("TfBarCode", "edtBarcodeMaxLength")->Enabled=true;
    }

    if(BAR_CODE_INSTALL==ebctEtherNetCCD)                                       //Ifor 20190225 :add Bar Code Use HandShake Communication
    {
        EL<TCheckBox>("TfBarCode", "cbUseHandShakeCommunication")->Visible=(CosFunction.bUseHandShakeCommunication);
        EL<TEdit>("TfBarCode", "edHandShakeTimeOut")->Visible         =(CosFunction.bUseHandShakeCommunication);
    }
    else
    {
        EL<TCheckBox>("TfBarCode", "cbUseHandShakeCommunication")->Visible=false;
        EL<TEdit>("TfBarCode", "edHandShakeTimeOut")->Visible         =false;
    }

    if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20210817 2D File Neme
    {
        EL<TLabel>("TfBarCode", "Label2D")->Visible=true;
        EL<TEdit>("TfBarCode", "edt2DFileName")->Visible=true;
    }
    //==> Eastsun 20260527 整合#028.AAL.P05 cbAutoAdjustLight Visible :KYEC
    if(BAR_CODE_INSTALL==ebctUseCCDMode)
    {
        EL<TCheckBox>("TfBarCode", "cbAutoAdjustLight")->Visible          =(CosFunction.bUseBarcodeAutoAdjustLight);
        EL<TEdit>("TfBarCode", "edAutoAdjustLightTimeOut")->Visible   =(CosFunction.bUseBarcodeAutoAdjustLight);
    }
    else
    {
        EL<TCheckBox>("TfBarCode", "cbAutoAdjustLight")->Visible          =false;
        EL<TEdit>("TfBarCode", "edAutoAdjustLightTimeOut")->Visible   =false;
    }
    //<== Eastsun 20260527 #028.AAL.P05
    BC_SetVisible();

    if(BAR_CODE_INSTALL==ebctUninstall && SHT_FLOATING_CHK!=0)                  //Ifor 20190225 :add Bar Code Use HandShake Communication
    {
        EL<TPageControl>("TfBarCode", "pgc2DID")->ActivePageIndex=1;
    ;   // golden BarCode\BarCode.cpp:494-494 取代：fBarCode->Caption：視窗標題（HTML 自己有）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:494-494 -- fBarCode->Caption：視窗標題（HTML 自己有）
        fBarCode->Caption="Shuttle floating detection";
#endif // GATE (S12-C save)
    }
    else
    {
        EL<TPageControl>("TfBarCode", "pgc2DID")->ActivePageIndex=0;
    }

    EL<TCheckBox>("TfBarCode", "chkMulti2DID")->Visible  =CosFunction.bEnableMulti2D;
    EL<TTabSheet>("TfBarCode", "tsMulti2DID")->TabVisible=TestIF_File.bEnableMulti2D;                         //Steven 20200810 : 一個IC使用多個2DID
    EL<TTabSheet>("TfBarCode", "tsOCR")->TabVisible=(BAR_CODE_INSTALL==ebcUseOCR);                            //Ifor 20210407 add: 自製OCR

    if(CUSTOMER_CODE!=CC_ASE_CL)                                                //KaiHuang 20200911 Add
    {
        EL<TCheckBox>("TfBarCode", "cbCheckSum")->Visible=false;
        EL<TLabel>("TfBarCode", "lblCheckSum")->Visible=false;
        EL<TEdit>("TfBarCode", "edtCheckSumLength")->Visible=false;
    }

    EL<TTabSheet>("TfBarCode", "ts4CCD")->TabVisible=true;
    EL<TTabSheet>("TfBarCode", "ts8CCD")->TabVisible=BOTTOM_2DID;
    if(BOTTOM_2DID && BOTTOM_2DID_CCD)                                          //Bottom 8CCD
        EL<TPageControl>("TfBarCode", "pgcCCDSetting")->ActivePageIndex=1;
    else                                                                        //Top + Bottom 4CCD
        EL<TPageControl>("TfBarCode", "pgcCCDSetting")->ActivePageIndex=0;

    EL<TCheckBox>("TfBarCode", "cbBarcodeCSVCompare")->Visible=true;                                          //Ifor 20260511 add: Barcode CSV Compare

    if(CosFunction.bReadClipCodeFromUnloader==true)
    {
//        pgc2DID->ActivePage=tsCCD;
        ;   // UI-only（HTML 端處理）：EL<TPageControl>("TfBarCode", "pgcCCDSetting")->ActivePage=EL<TTabSheet>("TfBarCode", "tsCCD_Unloader");
        BC_ReadUnloaderClipIni();
    }
    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);                                  //Steven 20100629

}

// golden BarCode\BarCode.cpp:531  TfBarCode::FormClose(TObject *Sender, TCloseAction &Action)
static void BC_FormClose()
{

    bShow=false;
    EL<TControl>("TfBarCode", "tmr1")->Enabled=false;

    ;   // golden BarCode\BarCode.cpp:536-543 取代：bStartTest[3]：只有 btnGPIBReader4Click（手動 GPIB 讀碼測試，未轉）會設 true；FormShow :348 已設 false → 這個分支在表單橋永遠不成立（分支內 SetNoiseDelay／TestISTimeOut／TestSocket.ClearAll 是執行期狀態）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:536-543 -- bStartTest[3]：只有 btnGPIBReader4Click（手動 GPIB 讀碼測試，未轉）會設 true；FormShow :348 已設 false → 這個分支在表單橋永遠不成立（分支內 SetNoiseDelay／TestISTimeOut／TestSocket.ClearAll 是執行期狀態）
    if(bStartTest[3]==true)
    {
        SetNoiseDelay=false;
        TestISTimeOut=false;
        TestSocket.ClearAll();
        bStartTest[3]=false;
        EL<TControl>("TfBarCode", "tmr1")->Enabled=false;
    }
#endif // GATE (S12-C save)
    BC_DoIniDataToForm();                                                          //JerryYang 20250411 : 離開頁面要刷新一次, 避免誤存檔

    //這一行請保持在最下面!!-----------------
//    myLog.Do_Log(Sender, asUser, asLogPath);    //Steven 20100629

}

// golden BarCode\BarCode.cpp:628  TfBarCode::SetVisible()
static void BC_SetVisible()
{

    EL<TTabSheet>("TfBarCode", "tsBarcodeMapping")->TabVisible =CosFunction.bBarcodeTrayRecFile;              //jou 20190930 : Barcode Tray record file

    EL<TTabSheet>("TfBarCode", "tsBarcodeAuto4")->TabVisible  =(AUTO_EMPTY_COLOR>=3);                         //Steven 20240704 : Add for HT-9011UC
    EL<TTabSheet>("TfBarCode", "tsBarcodeFix4")->TabVisible   =(AUTO_EMPTY_COLOR>=3);
    EL<TTabSheet>("TfBarCode", "tsBarcodeBinBox")->TabVisible =false;
    EL<TTabSheet>("TfBarCode", "tsMagazine1")->TabVisible     =(AUTO3_IS_MAGAZINE==1);
    EL<TTabSheet>("TfBarCode", "tsMagazine2")->TabVisible     =(AUTO3_IS_MAGAZINE==1);
    EL<TGroupBox>("TfBarCode", "grpLotVerify")->Visible       =CosFunction.bLotIDVerify;                      //Steven 20240704 : Lot Verification function for ATK

    EL<TPanel>("TfBarCode", "palInSht")->Visible           =(BAR_CODE_INSTALL==ebctInShtIntel || BAR_CODE_INSTALL==ebctUseCCDMode || BAR_CODE_INSTALL==ebctEtherNetCCD);   //Ifor 20190129 : add Cognex EtherNet 通訊
    EL<TPanel>("TfBarCode", "palOutSht")->Visible          =(BAR_CODE_INSTALL==ebctOutShtAMD);
    EL<TGroupBox>("TfBarCode", "gb2DTriggerCmd")->Visible     =(BAR_CODE_INSTALL!=ebctUseCCDMode);
    EL<TCheckBox>("TfBarCode", "cbTriggerMode")->Visible      =false;
//    cbExposureTimeOut->Visible  =(BAR_CODE_INSTALL==ebctUseCCDMode);          //Steven 20170425 : 沒用到Mark

    EL<TTabSheet>("TfBarCode", "ts2DID")->TabVisible          =(BAR_CODE_INSTALL!=ebctUninstall);
    EL<TTabSheet>("TfBarCode", "tsCognexSetting")->TabVisible =(BAR_CODE_INSTALL==ebctInShtIntel);
    EL<TTabSheet>("TfBarCode", "tsCCD")->TabVisible           =(BAR_CODE_INSTALL==ebctUseCCDMode || BAR_CODE_INSTALL==ebctEtherNetCCD || SHT_FLOATING_CHK!=0 || BAR_CODE_INSTALL==ebcUseOCR || CosFunction.bReadClipCodeFromUnloader); //Ifor 20190129 : add Cognex EtherNet 通訊  //Ifor 20210407 add: 自製OCR
    EL<TTabSheet>("TfBarCode", "tsFloating")->TabVisible      =(SHT_FLOATING_CHK!=0);
    EL<TCheckBox>("TfBarCode", "cbCheckCodeByShuttle")->Visible=(BAR_CODE_INSTALL==ebctUseCCDMode);
    EL<TCheckBox>("TfBarCode", "chkBottom2DID")->Visible      =BOTTOM_2DID;                                   //Steven 20190308 : Bottom 2D
    EL<TGroupBox>("TfBarCode", "grpBottom2D")->Visible        =BOTTOM_2DID;
    EL<TTabSheet>("TfBarCode", "tsCCD_Unloader")->Visible     =CosFunction.bReadClipCodeFromUnloader;
    //**************
    //這部分會影響分Bin,如果要讓客戶可以選,要發信通知
    //**************
    if(IniConfig.bKoreaFunction)                                                //Steven 20171128 (Wei) : modify
    {
        EL<TCheckBox>("TfBarCode", "cbCheckCodeByShuttle")->Enabled=true;                                     //Alick 20170203 (wei) add for XILINX 使用CCD時該選項反
        EL<TEdit>("TfBarCode", "edShtDuplicateRetryCnt")->Visible=(BAR_CODE_INSTALL==ebctUseCCDMode);
    }
    else
    {
        EL<TCheckBox>("TfBarCode", "cbCheckCodeByShuttle")->Enabled=!(BAR_CODE_INSTALL==ebctUseCCDMode);      //Alick 20170203 (wei) add for XILINX 使用CCD時該選項反
        EL<TEdit>("TfBarCode", "edShtDuplicateRetryCnt")->Visible=(BAR_CODE_INSTALL==ebctUseCCDMode);
    }
    //**************
    //這部分會影響分Bin,如果要讓客戶可以選,要發信通知
    //**************

}

// golden BarCode\BarCode.cpp:673  TfBarCode::ReadFile()
static void BC_ReadFile()
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
        EL<TCheckBox>("TfBarCode", "cbCheckSum")->Visible             =false;
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
    EL<TEdit>("TfBarCode", "ed_BarCodeNo")->Text                  =ReadIniData(szDir, sGroup, "Bar Code Text", AnsiString("ABCDEFGHIJKLMNOP"));

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

    ;   // golden BarCode\BarCode.cpp:948-959 取代：fMain->pnlSaveSummary->Visible：主畫面「手動存 Summary」面板的顯示（純畫面）；移植樹 TfMain 門面沒有 pnlSaveSummary（同 BarCode/BarCode.cpp GATE(G-BC-SAVESUMMARY)）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:948-959 -- fMain->pnlSaveSummary->Visible：主畫面「手動存 Summary」面板的顯示（純畫面）；移植樹 TfMain 門面沒有 pnlSaveSummary（同 BarCode/BarCode.cpp GATE(G-BC-SAVESUMMARY)）
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
#endif // GATE (S12-C save)

    if(CosFunction.bRead2DIDFromServer==true)                                   //Jimmychiu 20230925 : read 2did in json file
    {
        TestIF_File.bCheckCodeByServer2DID=ReadIniData(szDir, sGroup, "Check Code By Server 2DID",       false);
        TestIF_File.asMes2DID_URL         =ReadIniData(szDir, sGroup, "asMes2DID_URL",                   AnsiString(""));
        EL<TLabel>("TfBarCode", "lbfinalpathShow")->Caption          =BC_GetBarcodeByServerData();
    }
    else
    {
        TestIF_File.bCheckCodeByServer2DID=false;
        EL<TLabel>("TfBarCode", "lbfinalpathShow")->Caption="None";
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
    TestIF_File.bEnableBarcodeCSVCompare=ReadIniData(szDir, sGroup, "bEnableBarcodeCSVCompare", false); //Ifor 20260511 add: Barcode CSV Compare
    bCSVCacheLoaded = false;   // golden BarCode\BarCode.cpp:1016-1016 取代：bCSVCacheLoaded：CSV 比對快取旗標（TfBarCode 成員，LoadBarcodeCSVCache :11995 未轉）→ 本 TU 的 static 照設
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1016-1016 -- bCSVCacheLoaded：CSV 比對快取旗標（TfBarCode 成員，LoadBarcodeCSVCache :11995 未轉）→ 本 TU 的 static 照設
    bCSVCacheLoaded = false;  // Reset cache, reload on next compare
#endif // GATE (S12-C save)

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

    BC_SetVisible();

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

    ;   // golden BarCode\BarCode.cpp:1047-1047 取代：fMain->ShowOCRState(0)：主畫面 OCR 狀態燈（純畫面）；移植樹 TfMain 門面沒有（同 GATE(G-BC-OCRSTATE)）。上面四個 SendMSG_CMD 照 golden 送
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1047-1047 -- fMain->ShowOCRState(0)：主畫面 OCR 狀態燈（純畫面）；移植樹 TfMain 門面沒有（同 GATE(G-BC-OCRSTATE)）。上面四個 SendMSG_CMD 照 golden 送
        fMain->ShowOCRState(0);
#endif // GATE (S12-C save)
    }

    if(BAR_CODE_INSTALL!=ebctUninstall) filerw::ELTodo("golden BarCode.cpp:1050 Change2DSetupFile(): iConntectionOkTask/iBottomConntectionOkTask=1 (2D CCD re-init with the new recipe via TimerCCDInitialTimer) not ported -- the CCD is NOT told about this recipe");   // golden BarCode\BarCode.cpp:1050-1050 取代：Change2DSetupFile（:5548）：設 iConntectionOkTask／iBottomConntectionOkTask1/2=1，讓 TimerCCDInitialTimer 重新對 2D CCD 下換配方指令；移植樹沒有這三個 task 與計時器（同 GATE(G-BC-TAIL)、forms/fLotInfo.cpp 的 Change2DSetupFile gate）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1050-1050 -- Change2DSetupFile（:5548）：設 iConntectionOkTask／iBottomConntectionOkTask1/2=1，讓 TimerCCDInitialTimer 重新對 2D CCD 下換配方指令；移植樹沒有這三個 task 與計時器（同 GATE(G-BC-TAIL)、forms/fLotInfo.cpp 的 Change2DSetupFile gate）
    Change2DSetupFile();
#endif // GATE (S12-C save)
    ;   // golden BarCode\BarCode.cpp:1051-1051 取代：SetSFCCheckStepCount（:3150）：iSFCTotalMoveStep=InArmSuck.iShtCol(*2) —— TfBarCode 成員，移植樹沒有這個成員也沒有讀者（grep iSFCTotalMoveStep 0 筆）；Shuttle Float Check 本機 SHT_FLOATING_CHK=0
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1051-1051 -- SetSFCCheckStepCount（:3150）：iSFCTotalMoveStep=InArmSuck.iShtCol(*2) —— TfBarCode 成員，移植樹沒有這個成員也沒有讀者（grep iSFCTotalMoveStep 0 筆）；Shuttle Float Check 本機 SHT_FLOATING_CHK=0
    SetSFCCheckStepCount();
#endif // GATE (S12-C save)
    ;   // golden BarCode\BarCode.cpp:1052-1052 取代：mtBarcodeSetDefaultView：純畫面（見 FormShow 同一條）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1052-1052 -- mtBarcodeSetDefaultView：純畫面（見 FormShow 同一條）
    mtBarcodeSetDefaultView();
#endif // GATE (S12-C save)

}

// golden BarCode\BarCode.cpp:1055  TfBarCode::DoIniDataToForm()
static void BC_DoIniDataToForm()
{

    EL<TCheckBox>("TfBarCode", "cbEnableBarCode")->Checked        =TestIF_File.bEnableBarCode;
    EL<TEdit>("TfBarCode", "edBarcodeScanDelayTime")->Text    =AnsiString(TestIF_File.iBarCodeDelay);
    EL<TEdit>("TfBarCode", "edBarcodePosDelayTime")->Text     =AnsiString(TestIF_File.iBarCodePosDelay);  //wei 20151126 BarCode Pos Delay
    EL<TEdit>("TfBarCode", "edFirstDelay")->Text              =AnsiString(TestIF_File.iBarCodePos1Delay);
    EL<TEdit>("TfBarCode", "edtBarcodeMinLength")->Text       =AnsiString(TestIF_File.iBarCodeMinLength); //wei 20151127 字元數比對
    EL<TEdit>("TfBarCode", "edtBarcodeMaxLength")->Text       =AnsiString(TestIF_File.iBarCodeMaxLength);
    EL<TEdit>("TfBarCode", "edtCheckSumLength")->Text         =AnsiString(TestIF_File.iCheckSumLength);   //KaiChen 20191121 ：中壢日月光 2D Check Sum
    EL<TCheckBox>("TfBarCode", "cbCheckSum")->Checked             =TestIF_File.bCheckSum;                     //KaiChen 20191121 ：中壢日月光 2D Check Sum
    EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr")->ItemIndex  =TestIF_File.iNoCodeDeviceToErr;            //Steven 20151221 : 將讀取異常的IC放到Error Bin
    EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Checked =TestIF_File.bNoCodeDeviceAutoSkip;         //Steven 20151221 : 讀不到Code時,自動Skip跳下一顆
    EL<TEdit>("TfBarCode", "edBarcodeRetryCount")->Text       =AnsiString(TestIF_File.iBarcodeRetryCount);//Steven 20151221 : 讀不到Code時,自動Retry的次數
    EL<TCheckBox>("TfBarCode", "cbTriggerMode")->Checked          =TestIF_File.b2DTriggerMode;                //Steven 20151225 : 改用拍完就跑的方式
//    cbExposureTimeOut->Checked      =TestIF_File.bExposureTimeOut;
    EL<TCheckBox>("TfBarCode", "cbCheckCodeByShuttle")->Checked   =TestIF_File.bCheckCodeByShuttle;           //Steven 20160428 : 檢查2D重複碼
    EL<TCheckBox>("TfBarCode", "chkBottom2DID")->Checked          =TestIF_File.bEnableBottom2D;               //Steven 20190308 : Bottom 2D
    EL<TCheckBox>("TfBarCode", "chkMulti2DID")->Checked           =TestIF_File.bEnableMulti2D;                //Steven 20200810 : 一個IC使用多個2DID
    EL<TLabeledEdit>("TfBarCode", "edtXPitch")->Text                 =TestIF_File.dMulti2DXPitch;
    EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex        =TestIF_File.iMulti2DType;

    if(CosFunction.bLotIDVerify==true)                                          //Steven 20240704 : Lot Verification function for ATK
    {
        EL<TCheckBox>("TfBarCode", "chkEnableLotVerify")->Checked =TestIF_File.bLotIDVerify;
        EL<TEdit>("TfBarCode", "edtLotIDStart")->Text         =AnsiString(TestIF_File.iLotIDVerifyS);
        EL<TEdit>("TfBarCode", "edtLotIDEnd")->Text           =AnsiString(TestIF_File.iLotIDVerifyE);
        EL<TEdit>("TfBarCode", "edt2DStart")->Text            =AnsiString(TestIF_File.i2DIDStrStart);
        EL<TEdit>("TfBarCode", "edt2DEnd")->Text              =AnsiString(TestIF_File.i2DIDStrEnd);
        EL<TEdit>("TfBarCode", "edtLotIDVerify")->Text        =TestIF_File.sLotIDVerify;
        EL<TEdit>("TfBarCode", "edtLotIDSubstr")->Text        =TestIF_File.sLotIDSubstr;
    }

    BC_SetMulti2DMap(TestIF_File.iMulti2DType);
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            if(TestIF_File.iMulti2DType==e1x2In1CCD && i==1)
    filerw::ELComboIndex(Multi2DSiteCH[i][j], 0);   // golden BarCode\BarCode.cpp:1093-1093 取代：TComboBox ItemIndex 的 VCL 連動（帶出 Text）：vclcompat 只有欄位 → filerw::ELComboIndex
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1093-1093 -- TComboBox ItemIndex 的 VCL 連動（帶出 Text）：vclcompat 只有欄位 → filerw::ELComboIndex
                Multi2DSiteCH[i][j]->ItemIndex=0;
#endif // GATE (S12-C save)
            else if((TestIF_File.iMulti2DType==e2x1In1CCD || TestIF_File.iMulti2DType==e2x1In2CCD) && j==1)
    filerw::ELComboIndex(Multi2DSiteCH[i][j], 0);   // golden BarCode\BarCode.cpp:1095-1095 取代：同上
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1095-1095 -- 同上
                Multi2DSiteCH[i][j]->ItemIndex=0;
#endif // GATE (S12-C save)
            else
    filerw::ELComboIndex(Multi2DSiteCH[i][j], TestIF_File.iMulti2DMap[i][j]);   // golden BarCode\BarCode.cpp:1097-1097 取代：同上
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1097-1097 -- 同上
                Multi2DSiteCH[i][j]->ItemIndex=TestIF_File.iMulti2DMap[i][j];
#endif // GATE (S12-C save)
        }
    }

    EL<TEdit>("TfBarCode", "edtOffsetX")->Text                =TestIF_File.dBottom2DOffsetX;
    EL<TEdit>("TfBarCode", "edtOffsetY")->Text                =TestIF_File.dBottom2DOffsetY;
    EL<TEdit>("TfBarCode", "edShtDuplicateRetryCnt")->Text    =TestIF_File.iShtDuplicateRetryCnt;         //Steven 20160823 : 蝦頭重複碼要可以自動Retry
    EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot")->Checked       =TestIF_File.bCheckCodeByLot;               //Steven 20160428 : 檢查2D重複碼
    EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode")->Checked     =TestIF_File.bCheckLotHaveCode;             //wei 20160505 Barcode 比對Lot
    EL<TCheckBox>("TfBarCode", "cbConsecutiveFailure")->Checked   =TestIF_File.bEnableConsecutiveFailure;     //wei 20160823  Consecutive Failure
    EL<TEdit>("TfBarCode", "edTriggerTime")->Text             =AnsiString(TestIF_File.i2DTriggerTime);    //Steven 20151225 : 拍照的等待時間
    EL<TCheckBox>("TfBarCode", "cb2DUseUndefinedCMD")->Checked    =TestIF_File.b2DUseUndefinedCMD;            //Ifor 20151226 :改用 自行定義 Trigger Command
    EL<TEdit>("TfBarCode", "edtBarcodeONCMD")->Text           =TestIF_File.str2DTriggerONCMD;             //Ifor 20151226 :自行定義 Trigger ON Command
    EL<TEdit>("TfBarCode", "edtBarcodeOFFCMD")->Text          =TestIF_File.str2DTriggerOFFCMD;            //Ifor 20151226 :自行定義 Trigger ON Command

    EL<TCheckBox>("TfBarCode", "cbRetryOffsetMove")->Checked      =TestIF_File.bRetryOffsetMove;              //wei 20161116 Retry時先退出再進去讀取
    EL<TEdit>("TfBarCode", "edRetryOffsetMove")->Text         =TestIF_File.dRetryOffsetMove;              //wei 20161116 Retry時先退出再進去讀取

    EL<TCheckBox>("TfBarCode", "cbRetryShiftOffsetMove")->Checked =TestIF_File.bRetryShiftOffsetMove;         //wei 20161116 Retry時先退出再進去讀取(前中後)
    EL<TEdit>("TfBarCode", "edRetryShiftOffsetMove")->Text    =TestIF_File.dRetryShiftOffsetMove;         //wei 20161116 Retry時先退出再進去讀取(前中後)

    EL<TCheckBox>("TfBarCode", "chk2DIDYield")->Checked           =TestIF_File.b2DIDYield;                    //Steven 20171222 (Wei) : Yield Alarm of 2DID
    EL<TEdit>("TfBarCode", "edt2DIDYield")->Text              =TestIF_File.d2DIDYield;                    //Steven 20171222 (Wei) : Yield Alarm of 2DID
    EL<TEdit>("TfBarCode", "ed_2D_YieldIgnoreCnt")->Text      =TestIF_File.i2DYieldIgnoreCnt;             //JerryYang 20241104 : Ignore count變更為可以修改
    EL<TRadioGroup>("TfBarCode", "rgAllSiteFail")->ItemIndex        =TestIF_File.iEnableAllSite2DIDErr;         //Steven 20200702 : All site 2DID fail改成可以開關
    EL<TCheckBox>("TfBarCode", "chkCloseSite2DIDIsEmpty")->Checked=TestIF_File.bSetCloseSite2DIDtoEmpty;      //Steven 20190313 : Close site 2DID set to empty
    EL<TCheckBox>("TfBarCode", "cbSearch2DIDByLot")->Checked      =TestIF_File.bSearch2DIDByLot;              //Frank 20170316 (wei) add Search 2DID By Lot
    filerw::ELComboIndex(EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin"), TestIF_File.b2DIDListErrorBin);             //Steven 20190604 : 2DID不在List內的另外分bin
    EL<TCheckBox>("TfBarCode", "cbSortingBy2DIDList")->Checked    =TestIF_File.bSortingBy2DIDList;            //JerryYang 20190313

    EL<TRadioGroup>("TfBarCode", "rgActionOf2DSort")->ItemIndex     =TestIF_File.iActionOf2DNotInList;          //Steven 20250707 : Action Of 2D Not In List

    EL<TCheckBox>("TfBarCode", "chkSearch2DIDToError")->Checked   =TestIF_File.b2DIDNotExist2Error;           //JerryYang 20231218 : 2DID黑名單功能

    EL<TCheckBox>("TfBarCode", "cbSaveFailImage")->Checked        =TestIF_File.bSaveFailImage;                //Frank 20170408 (Steven) add Save Fail Image
    EL<TEdit>("TfBarCode", "edt2DFileName")->Text             =TestIF_File.s2DFileName;                   //kevin 20210817 2D FILENAME
    EL<TRadioGroup>("TfBarCode", "rgSelectSh1")->ItemIndex          =TestIF_File.iSelectUseCCDSh1;              //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos
    EL<TRadioGroup>("TfBarCode", "rgSelectSh2")->ItemIndex          =TestIF_File.iSelectUseCCDSh2;              //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos
    EL<TCheckBox>("TfBarCode", "cb2DUseSubJob")->Checked          =TestIF_File.b2DUseSubJob;                  //Ifor 20200807 add:In House 2D Use Sub Job Function
    EL<TCheckBox>("TfBarCode", "cb2DUseAnyChar")->Checked         =TestIF_File.b2DUseAnyChar;                 //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷
    //==> Eastsun 20260526 #026-4.PinN.PU5 Pin1 Checked :KYEC
    EL<TCheckBox>("TfBarCode", "cbUsePinInspection")->Checked     =TestIF_File.b2DUsePinInspection;           //Ifor 20230207 add:In House 2D Use Pin1 Inspection Function
    //<== Eastsun 20260526 #026-4.PinN.PU5
    filerw::ELComboText(EL<TComboBox>("TfBarCode", "coCommReader1"), HSys.asBarCodeComPort[0]);                               //wei 20150912 2D
    filerw::ELComboText(EL<TComboBox>("TfBarCode", "coCommReader2"), HSys.asBarCodeComPort[1]);                               //wei 20150912 2D
    filerw::ELComboText(EL<TComboBox>("TfBarCode", "coCommReader3"), HSys.asBarCodeComPort[2]);                               //wei 20150912 2D
    filerw::ELComboText(EL<TComboBox>("TfBarCode", "coCommReader4"), HSys.asBarCodeComPort[3]);                               //wei 20150912 2D

    //Ifor 20151224 add
    EL<TEdit>("TfBarCode", "edShuttle_1A_Address")->Text=HSys.asCCDBarCodeIP[0];                          //Ifor 20151224 CCD 2D
    EL<TEdit>("TfBarCode", "edShuttle_1B_Address")->Text=HSys.asCCDBarCodeIP[1];                          //Ifor 20151224 CCD 2D
    EL<TEdit>("TfBarCode", "edShuttle_2A_Address")->Text=HSys.asCCDBarCodeIP[2];                          //Ifor 20151224 CCD 2D
    EL<TEdit>("TfBarCode", "edShuttle_2B_Address")->Text=HSys.asCCDBarCodeIP[3];                          //Ifor 20151224 CCD 2D

    EL<TEdit>("TfBarCode", "edShuttle_1A_Port")->Text=HSys.asCCDBarCodePort[0];                           //Ifor 20151224 CCD 2D
    EL<TEdit>("TfBarCode", "edShuttle_1B_Port")->Text=HSys.asCCDBarCodePort[1];                           //Ifor 20151224 CCD 2D
    EL<TEdit>("TfBarCode", "edShuttle_2A_Port")->Text=HSys.asCCDBarCodePort[2];                           //Ifor 20151224 CCD 2D
    EL<TEdit>("TfBarCode", "edShuttle_2B_Port")->Text=HSys.asCCDBarCodePort[3];                           //Ifor 20151224 CCD 2D

    EL<TEdit>("TfBarCode", "edConsecutiveFailure")->Text     =TestIF_File.iConsecutiveFailure;            //wei 20160823  Consecutive Failure

    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //wei 20160318 Barcode Error依客戶設定
    {
        asBarCodeErrorSend="BARCODEREJECT";
        if(LastSet.iRealDummy==DUMMY && LastSet.iTester==OFF_LINE)
        {
            EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot")->Enabled=true;                                     //Alick 20170222 (wei) add NOTRAY NODEVICE時可關閉確認重複碼功能
            EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode")->Enabled=true;                                   //Alick 20170222 (wei) add NOTRAY NODEVICE時可關閉確認重複碼功能
        }
        else
        {
            EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot")->Enabled=false;                                    //wei 20161118 強制打開確認重複碼功能
            EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode")->Enabled=false;                                  //Alick 20170222 (wei) 強制打開確認重複碼功能
        }
    }
    else
    {
        if(CosFunction.bBarcodeErrNoTestAndShowH==true)                         //jou 20191007 : Barcode Error No Test & Show "H"
        {
            asBarCodeErrorSend="H";
        }

        if(TestIF_File.i2DIDFormat==eAMD)                                       //JerryYang 20200422 2DID format選項改用下拉選單
        {
            asBarCodeErrorSend="DECODEFAIL";
        }
        else
        {
            asBarCodeErrorSend="ERROR";
        }

        if(CosFunction.b2DCodeCheckByCoustomerLot)                              //Sam 20220223 : 2D Code Check by Coustomer Lot
        {
        }
        else if(CUSTOMER_CODE==CC_ASE_CL)    //JerryYang 20250120 : ASECL SONG要求強制開啟
        {
            if(LastSet.iTester==ON_LINE)
            {
                EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot")->Enabled=false;
            }
            else
            {
                EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot")->Enabled=true;
            }
        }
        else
        {
            EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode")->Checked=false;                                  //Steven 20170707 (wei) : Fixed 2DID for Korea
        }
    }

    EL<TCheckBox>("TfBarCode", "cbEnableShtFloatChk")->Checked    =TestIF_File.bEnableShtFloatChk;            //Steven 20160920 : IC置偏檢查
    EL<TEdit>("TfBarCode", "edSFCStartDelay")->Text           =TestIF_File.iSFCStartDelay;
    EL<TEdit>("TfBarCode", "edSFCExposureTimeOut")->Text      =TestIF_File.iSFCExposureTimeOut;
    EL<TEdit>("TfBarCode", "edSFCGetResultTimeOut")->Text     =TestIF_File.iSFCGetResultTimeOut;
    EL<TEdit>("TfBarCode", "edSFCAutoRetry")->Text            =TestIF_File.iSFCAutoRetry;
    EL<TCheckBox>("TfBarCode", "cbSFCUse2Photo")->Checked         =TestIF_File.bSFCUse2Photo;
    EL<TEdit>("TfBarCode", "edSFCUse2PhotoOffset")->Text      =TestIF_File.iSFCUse2PhotoOffset;

    EL<TRadioGroup>("TfBarCode", "rgStringFormat")->ItemIndex       =TestIF_File.b2DIDStringFormat;             //RogerYang 20181222 新增String format選項
    EL<TCheckBox>("TfBarCode", "cbUseHandShakeCommunication")->Checked    =TestIF_File.bUseHandShakeCommunication;    //Ifor 20190225 :add Bar Code Use HandShake Communication
    EL<TEdit>("TfBarCode", "edHandShakeTimeOut")->Text        =AnsiString(TestIF_File.i2DHandShakeTimeOut);       //Ifor 20190225 :add Bar Code Use HandShake Communication
    //==> Eastsun 20260527 整合#028.AAL.P07 UI bind AutoAdjustLight :KYEC
    EL<TCheckBox>("TfBarCode", "cbAutoAdjustLight")->Checked      =TestIF_File.bUseBarcodeAutoAdjustLight;
    EL<TEdit>("TfBarCode", "edAutoAdjustLightTimeOut")->Text  =AnsiString(TestIF_File.iAutoAdjustLightTimeOut);
    //<== Eastsun 20260527 #028.AAL.P07

    filerw::ELComboIndex(EL<TComboBox>("TfBarCode", "cbReadMultiLine_2DBarcode"), TestIF_File.i2DReadMultiLine);   //Ifor 20210407 add: 自製OCR
    EL<TEdit>("TfBarCode", "edt1stLineLength_2DBarCode")->Text           =TestIF_File.i2D1stLineLength;
    EL<TEdit>("TfBarCode", "edt2ndLineLength_2DBarCode")->Text           =TestIF_File.i2D2ndLineLength;
    EL<TEdit>("TfBarCode", "edtInsertString1stAnd2nd_2DBarCode")->Text   =TestIF_File.as2DInsertString;
    EL<TCheckBox>("TfBarCode", "cbBarCodeMultiRecipe")->Checked   =TestIF_File.bBarCodeMultiRecipe;           //Eastsun 20260527 整合//Ifor 20241031 add:使用BarCode Multi Recipe

    if(CUSTOMER_CODE==CC_ASE_CL)                                                //KaiHuang 20200911 : Add
    {
        EL<TEdit>("TfBarCode", "edtCheckSumLength")->Text         =AnsiString(TestIF_File.iCheckSumLength);   //KaiChen 20191121 ：中壢日月光 2D Check Sum
        EL<TCheckBox>("TfBarCode", "cbCheckSum")->Checked             =TestIF_File.bCheckSum;                     //KaiChen 20191121 ：中壢日月光 2D Check Sum
    }
    else
    {
        EL<TEdit>("TfBarCode", "edtCheckSumLength")->Text         =AnsiString(TestIF_File.iCheckSumLength);
        EL<TCheckBox>("TfBarCode", "cbCheckSum")->Checked             =false;
    }
    EL<TCheckBox>("TfBarCode", "cbCheckEnable2DIDFunction")->Checked       =TestIF_File.bCheckEnable2DIDFunction;         //KaiHuang 20201028 : ASE-CL 生產時檢查是否開啟 2D
    EL<TCheckBox>("TfBarCode", "cbCheckEnableBottom2DIDFunction")->Checked =TestIF_File.bCheckEnableBottom2DIDFunction;   //KaiHuang 20201028 : ASE-CL 生產時檢查是否開啟 Bottom 2D

    if(CosFunction.bRead2DIDFromServer==true)                                   //Jimmychiu 20230925 : read 2did in json file
    {
        EL<TCheckBox>("TfBarCode", "cbCheckCodeByServer2DID")->Checked=TestIF_File.bCheckCodeByServer2DID;
        EL<TLabeledEdit>("TfBarCode", "le2DID_ServerURL")->Text=TestIF_File.asMes2DID_URL;
        //==> Eastsun 20260527 整合#BCIR.P06 UI sync :KYEC
        EL<TCheckBox>("TfBarCode", "cbBarCodeInspReport")->Checked=TestIF_File.bBarCodeInspReport;                                       //Sam 20240426 : Add BarCoder Inspection Report
        //<== Eastsun 20260527 #BCIR.P06
        EL<TLabel>("TfBarCode", "lbfinalpathShow")->Caption=BC_GetBarcodeByServerData();
    }

    if(CosFunction.bMakeWhite2DIDList==true)                                    //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    {
        EL<TCheckBox>("TfBarCode", "chkMakeWhite2DIDList")->Checked=TestIF_File.bChkMakeWhite2DIDList;
    }

    EL<TCheckBox>("TfBarCode", "cb2DIDAllowList")->Checked       =TestIF_File.b2DIDAllowList;                 //JerryYang 20241104 : 支援2DID白名單功能
    EL<TCheckBox>("TfBarCode", "cbBarcodeCSVCompare")->Checked=TestIF_File.bEnableBarcodeCSVCompare;          //Ifor 20260511 add: Barcode CSV Compare
    if(CosFunction.bReadClipCodeFromUnloader==true)
    {
        BC_ReadUnloaderClipIni();
    }

}

// golden BarCode\BarCode.cpp:1268  TfBarCode::spbSaveClick(TObject *Sender)
static void BC_spbSaveClick()
{
    filerw::ELMark("spbSaveClick");   // FileRW：存檔流程 trace（ack.trace）

    if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&                     //RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2)
        AccessLevel==0)
    {
        filerw::ELMessage("[A01_2]目前已切換到Operator權限，\r\n請重新登入再做設定!");
    filerw::ELMark("closed");   // golden BarCode\BarCode.cpp:1274-1274 取代：Close()：golden A02 權限不足時關表單 → 伺服器端記 closed，頁面自己關；沒寫檔 → PageDesc::reload（golden FormClose 的 DoIniDataToForm）還原替身
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1274-1274 -- Close()：golden A02 權限不足時關表單 → 伺服器端記 closed，頁面自己關；沒寫檔 → PageDesc::reload（golden FormClose 的 DoIniDataToForm）還原替身
        Close();
#endif // GATE (S12-C save)
        return;
    }

    AnsiString S="";
    S=GetLastOpenFN();
    AnsiString szDir="";
    AnsiString sGroup;
    AnsiString szDir_Xilinx="D:\\HT9045\\system\\Barcode.ini";                  //Alick 20170207 (wei) add for Xilinx Barcode Length save by machine
    szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Alick 20170207 (wei) add for Xilinx Barcode Length save by machine
    {
        sGroup="Configuration_Barcode(XILINX)";
        szDir ="D:\\HT9045\\system\\Barcode.ini";
    }
    else
    {
        sGroup="Configuration";
        szDir.sprintf("%s%s\\HandlerCondition.Data", DataPath, S);
    }

    if(CosFunction.bEnableMulti2D)
    {
        if(EL<TCheckBox>("TfBarCode", "chkMulti2DID")->Checked)
            if(BC_CheckMulti2DMap())
                return;
    }

    filerw::ELMark("BC_WriteIniData"); if(BOTTOM_2DID)   // golden BarCode\BarCode.cpp:1302-1302 取代：存檔標記：這一行之後第一個 WriteIniData（"Bottom 2D"）就落地 —— golden 在它之後仍可能 return（:1309 Multi2D 2CCD 檢查），但檔案已經寫了一鍵，所以 saved 以這裡為準（savedMark="BC_WriteIniData"）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1302-1302 -- 存檔標記：這一行之後第一個 WriteIniData（"Bottom 2D"）就落地 —— golden 在它之後仍可能 return（:1309 Multi2D 2CCD 檢查），但檔案已經寫了一鍵，所以 saved 以這裡為準（savedMark="BC_WriteIniData"）
    if(BOTTOM_2DID)                                                             //Steven 20190308 : Bottom 2D
#endif // GATE (S12-C save)
        WriteIniData(szDir, sGroup, "Bottom 2D", (EL<TCheckBox>("TfBarCode", "chkBottom2DID")->Checked)?1:0);
    else
        WriteIniData(szDir, sGroup, "Bottom 2D", 0);

    if(CosFunction.bEnableMulti2D)                                              //Steven 20200810 : 一個IC使用多個2DID
    {
        if((EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x1In2CCD || EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x2In2CCD) &&
           TestIF_File.iTestMode>=DualSite2x1  &&
           IsNNMode()!=NN_1Row)
        {
            filerw::ELMessage("Test mode with 2 mode, can not select multi 2D in 2CCD!");
    EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex=0;   // golden BarCode\BarCode.cpp:1314-1314 取代：產生器的元件改寫不接在 "->" 後面：同一行等價改寫（Multi2D 2CCD 不合法時 golden 把選項改回 0）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1314-1314 -- 產生器的元件改寫不接在 "->" 後面：同一行等價改寫（Multi2D 2CCD 不合法時 golden 把選項改回 0）
            EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex=0;
#endif // GATE (S12-C save)
            return;
        }

        WriteIniData(szDir, sGroup, "Multi 2D", (EL<TCheckBox>("TfBarCode", "chkMulti2DID")->Checked)?1:0);
        WriteIniData(szDir, sGroup, "Multi 2D X Pitch", EL<TLabeledEdit>("TfBarCode", "edtXPitch")->Text);
        WriteIniData(szDir, sGroup, "Multi 2D Typeh", EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex);

        for(int i=0; i<2; i++)
        {
            for(int j=0; j<2; j++)
            {
                S.sprintf("Multi 2D Map %d-%d", i+1, j+1);
                if((EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e1x2In1CCD && i==1) ||
                   ((EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x1In1CCD || EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x1In2CCD) && j==1))
                    ;
                else
                    WriteIniData(szDir, sGroup, S, Multi2DSiteCH[i][j]->ItemIndex);
            }
        }
    }

    if(CosFunction.bLotIDVerify)                                                //Steven 20240704 : Lot Verification function for ATK
    {
        WriteIniData(szDir, "Lot Verification", "Enable", (EL<TCheckBox>("TfBarCode", "chkEnableLotVerify")->Checked)?1:0);
        WriteIniData(szDir, "Lot Verification", "Lot ID Name",      EL<TEdit>("TfBarCode", "edtLotIDVerify")->Text);
        WriteIniData(szDir, "Lot Verification", "Lot ID Substr",    EL<TEdit>("TfBarCode", "edtLotIDSubstr")->Text);
        WriteIniData(szDir, "Lot Verification", "Lot ID Start",     EL<TEdit>("TfBarCode", "edtLotIDStart")->Text);
        WriteIniData(szDir, "Lot Verification", "Lot ID End",       EL<TEdit>("TfBarCode", "edtLotIDEnd")->Text);
        WriteIniData(szDir, "Lot Verification", "2DID Start",       EL<TEdit>("TfBarCode", "edt2DStart")->Text);
        WriteIniData(szDir, "Lot Verification", "2DID End",         EL<TEdit>("TfBarCode", "edt2DEnd")->Text);
    }

    WriteIniData(szDir, sGroup, "Bottom 2D Offset X",             atof(EL<TEdit>("TfBarCode", "edtOffsetX")->Text.c_str()));
    WriteIniData(szDir, sGroup, "Bottom 2D Offset Y",             atof(EL<TEdit>("TfBarCode", "edtOffsetY")->Text.c_str()));

    if(BAR_CODE_INSTALL==ebctUninstall)
    {
        WriteIniData(szDir, sGroup, "Bar Code",                    false);
    }
    else
    {
        if(BOTTOM_2DID && EL<TCheckBox>("TfBarCode", "chkBottom2DID")->Checked)
            WriteIniData(szDir, sGroup, "Bar Code",                1);
        else
            WriteIniData(szDir, sGroup, "Bar Code",                (EL<TCheckBox>("TfBarCode", "cbEnableBarCode")->Checked)?1:0);
    }

    WriteIniData(szDir, sGroup, "Bar Code Delay Time",             atoi(EL<TEdit>("TfBarCode", "edBarcodeScanDelayTime")->Text.c_str()));
    WriteIniData(szDir, sGroup, "Bar Code Pos Delay Time",         atoi(EL<TEdit>("TfBarCode", "edBarcodePosDelayTime")->Text.c_str()));     //wei 20151126 BarCode Pos Delay
    WriteIniData(szDir, sGroup, "Bar Code Pos1 Delay Time",        atoi(EL<TEdit>("TfBarCode", "edFirstDelay")->Text.c_str()));
    WriteIniData(szDir, sGroup, "Bar Code Min Length",             atoi(EL<TEdit>("TfBarCode", "edtBarcodeMinLength")->Text.c_str()));       //wei 20151127 字元數比對
    WriteIniData(szDir, sGroup, "Bar Code Max Length",             atoi(EL<TEdit>("TfBarCode", "edtBarcodeMaxLength")->Text.c_str()));
    WriteIniData(szDir, sGroup, "Bar Code Check Sum Length",       atoi(EL<TEdit>("TfBarCode", "edtCheckSumLength")->Text.c_str()));         //KaiChen 20191121 ：中壢日月光 2D Check Sum
    WriteIniData(szDir, sGroup, "Bar Code Check Sum",              (EL<TCheckBox>("TfBarCode", "cbCheckSum")->Checked)?1:0);                     //KaiChen 20191121 ：中壢日月光 2D Check Sum
    WriteIniData(szDir, sGroup, "Check Enable 2DID Function",       (EL<TCheckBox>("TfBarCode", "cbCheckEnable2DIDFunction")->Checked)?1:0);      //KaiHuang 20201028 : ASE-CL 生產時檢查是否開啟 2D
    WriteIniData(szDir, sGroup, "Check 2DID Allow List Function",   (EL<TCheckBox>("TfBarCode", "cb2DIDAllowList")->Checked)?1:0);               //JerryYang 20241104 : 支援2DID白名單功能
    WriteIniData(szDir, sGroup, "Check Enable Bottom 2DID Function",(EL<TCheckBox>("TfBarCode", "cbCheckEnableBottom2DIDFunction")->Checked)?1:0);//KaiHuang 20201028 : ASE-CL 生產時檢查是否開啟 2D
    WriteIniData(szDir, sGroup, "Bar Code NoCodeDeviceToErr",      EL<TRadioGroup>("TfBarCode", "rgNoCodeDeviceToErr")->ItemIndex);                //Steven 20151221 : 將讀取異常的IC放到Error Bin
    WriteIniData(szDir, sGroup, "Bar Code NoCodeDeviceAutoSkip",   (EL<TCheckBox>("TfBarCode", "cbNoCodeDeviceAutoSkip")->Checked)?1:0);         //Steven 20151221 : 讀不到Code時,自動Skip跳下一顆
    WriteIniData(szDir, sGroup, "Bar Code Auto Retry",             EL<TEdit>("TfBarCode", "edBarcodeRetryCount")->Text);                     //Steven 20151221 : 讀不到Code時,自動Retry的次數
    WriteIniData(szDir, sGroup, "Bar Code Enable Trigger Mode",    (EL<TCheckBox>("TfBarCode", "cbTriggerMode")->Checked)?1:0);                  //Steven 20151225 : 改用拍完就跑的方式
    WriteIniData(szDir, sGroup, "Check duplicate code by shuttle", (EL<TCheckBox>("TfBarCode", "cbCheckCodeByShuttle")->Checked)?1:0);           //Steven 20160428 : 檢查2D重複碼
    WriteIniData(szDir, sGroup, "ShuttleDuplicateRertyCount",      (EL<TEdit>("TfBarCode", "edShtDuplicateRetryCnt")->Text));                //Steven 20160823 : 蝦頭重複碼要可以自動Retry
    WriteIniData(szDir, sGroup, "Check duplicate code by lot",     (EL<TCheckBox>("TfBarCode", "cbCheckCodeByLot")->Checked)?1:0);               //Steven 20160428 : 檢查2D重複碼
    WriteIniData(szDir, sGroup, "Bar Code Trigger Time",           EL<TEdit>("TfBarCode", "edTriggerTime")->Text);                           //Steven 20151225 : 拍照的等待時間
    WriteIniData(szDir, sGroup, "Bar Code tEXT",                   EL<TEdit>("TfBarCode", "ed_BarCodeNo")->Text);

    WriteIniData(szDir, sGroup, "Bar Code Use Undefined CMD Mode", (EL<TCheckBox>("TfBarCode", "cb2DUseUndefinedCMD")->Checked)?1:0);            //Ifor 20151226 :改用 自行定義 Trigger Command
    WriteIniData(szDir, sGroup, "Bar Code Undefined CMD ON",       EL<TEdit>("TfBarCode", "edtBarcodeONCMD")->Text);                         //Ifor 20151226 :Trigger ON Command
    WriteIniData(szDir, sGroup, "Bar Code Undefined CMD OFF",      EL<TEdit>("TfBarCode", "edtBarcodeOFFCMD")->Text);                        //Ifor 20151226 :Trigger OFF Command
    WriteIniData(szDir, sGroup, "Check Lot have code",             (EL<TCheckBox>("TfBarCode", "cbCheckLotHaveCode")->Checked)?1:0);             //wei 20160505 Barcode 比對Lot
    WriteIniData(szDir, sGroup, "Enable Consecutive Failure",      (EL<TCheckBox>("TfBarCode", "cbConsecutiveFailure")->Checked)?1:0);           //wei 20160823 Consecutive Failure
    WriteIniData(szDir, sGroup, "Consecutive Failure",             EL<TEdit>("TfBarCode", "edConsecutiveFailure")->Text);                    //wei 20160823 Consecutive Failure

    WriteIniData(szDir, sGroup, "Enable Retry Offset Move",        (EL<TCheckBox>("TfBarCode", "cbRetryOffsetMove")->Checked)?1:0);              //wei 20161116 Retry時先退出再進去讀取
    WriteIniData(szDir, sGroup, "Retry Offset Move mm",            EL<TEdit>("TfBarCode", "edRetryOffsetMove")->Text);                       //wei 20161116 Retry時先退出再進去讀取

    WriteIniData(szDir, sGroup, "Enable Retry Shift Offset Move",  (EL<TCheckBox>("TfBarCode", "cbRetryShiftOffsetMove")->Checked)?1:0);         //wei 20161116 Retry時先退出再進去讀取(前中後)
    WriteIniData(szDir, sGroup, "Retry Shift Offset Move mm",      EL<TEdit>("TfBarCode", "edRetryShiftOffsetMove")->Text);                  //wei 20161116 Retry時先退出再進去讀取(前中後)

    WriteIniData(szDir, sGroup, "Enable Check 2DID Yield",         (EL<TCheckBox>("TfBarCode", "chk2DIDYield")->Checked)?1:0);                   //Steven 20171222 (Wei) : Yield Alarm of 2DID
    WriteIniData(szDir, sGroup, "2DID Controlled Yield",           EL<TEdit>("TfBarCode", "edt2DIDYield")->Text);                            //Steven 20171222 (Wei) : Yield Alarm of 2DID
    WriteIniData(szDir, sGroup, "i2DYieldIgnoreCnt",               EL<TEdit>("TfBarCode", "ed_2D_YieldIgnoreCnt")->Text);                    //JerryYang 20241104 : Ignore count變更為可以修改

    WriteIniData(szDir, sGroup, "bSetCloseSite2DIDtoEmpty",        (EL<TCheckBox>("TfBarCode", "chkCloseSite2DIDIsEmpty")->Checked)?1:0);        //Steven 20190313 : Close site 2DID set to empty
    WriteIniData(szDir, sGroup, "iEnableAllSite2DIDErr",           EL<TRadioGroup>("TfBarCode", "rgAllSiteFail")->ItemIndex);                      //Steven 20200702 : All site 2DID fail改成可以開關

    WriteIniData(szDir, sGroup, "b2DUseSubJob",                     (EL<TCheckBox>("TfBarCode", "cb2DUseSubJob")->Checked)?1:0);                 //Ifor 20200807 add:In House 2D Use Sub Job Function
    WriteIniData(szDir, sGroup, "b2DUseAnyChar",                    (EL<TCheckBox>("TfBarCode", "cb2DUseAnyChar")->Checked)?1:0);                 //Ifor 20210723 add:2D Use Any Char 收到2D資料不判斷

    WriteIniData(szDir, sGroup, "bBarCodeMultiRecipe",(EL<TCheckBox>("TfBarCode", "cbBarCodeMultiRecipe")->Checked)?1:0);                         //Eastsun 20260527 整合//Ifor 20241031 add:使用BarCode Multi Recipe

    WriteIniData(szDir, sGroup, "b2DUsePinInspection",              (EL<TCheckBox>("TfBarCode", "cbUsePinInspection")->Checked)?1:0);              //Eastsun 20260526 整合//Ifor 20230207 add:In House 2D Use Pin1 Inspection Function  //PU6 20260526 fix target

    //Steven 20160920 : IC置偏檢查
    WriteIniData(szDir, sGroup, "Shuttle Float Check Enable",              (EL<TCheckBox>("TfBarCode", "cbEnableShtFloatChk")->Checked)?1:0);
    WriteIniData(szDir, sGroup, "Shuttle Float Check Start Delay",         EL<TEdit>("TfBarCode", "edSFCStartDelay")->Text);
    WriteIniData(szDir, sGroup, "Shuttle Float Check Exposure Time Out",   EL<TEdit>("TfBarCode", "edSFCExposureTimeOut")->Text);
    WriteIniData(szDir, sGroup, "Shuttle Float Check Get Result Time Out", EL<TEdit>("TfBarCode", "edSFCGetResultTimeOut")->Text);
    WriteIniData(szDir, sGroup, "Shuttle Float Check Auto Retry",          EL<TEdit>("TfBarCode", "edSFCAutoRetry")->Text);
    WriteIniData(szDir, sGroup, "Shuttle Float Check Use 2 Photo",         (EL<TCheckBox>("TfBarCode", "cbSFCUse2Photo")->Checked)?1:0);
    WriteIniData(szDir, sGroup, "Shuttle Float Check Use 2 Photo Offset",  EL<TEdit>("TfBarCode", "edSFCUse2PhotoOffset")->Text);

    WriteIniData(szDir, sGroup, "Search 2DID By Lot",              (EL<TCheckBox>("TfBarCode", "cbSearch2DIDByLot")->Checked)?1:0);       //Frank 20170316 (wei) add Search 2DID By Lot
    WriteIniData(szDir, sGroup, "Search 2DID By Lot Error Bin",     EL<TComboBox>("TfBarCode", "cbb2DIDListErrorBin")->ItemIndex);        //Steven 20190604 : 2DID不在List內的另外分bin
    WriteIniData(szDir, sGroup, "Sorting By 2DID List",            (EL<TCheckBox>("TfBarCode", "cbSortingBy2DIDList")->Checked)?1:0);     //JerryYang 20190313
    WriteIniData(szDir, sGroup, "iActionOf2DNotInList",             EL<TRadioGroup>("TfBarCode", "rgActionOf2DSort")->ItemIndex);           //Steven 20250707 : Action Of 2D Not In List

    WriteIniData(szDir, sGroup, "Search2DIDToErrorBin",            (EL<TCheckBox>("TfBarCode", "chkSearch2DIDToError")->Checked)?1:0);    //JerryYang 20231218 : 2DID黑名單功能

    WriteIniData(szDir, sGroup, "Save Fail Image",                 (EL<TCheckBox>("TfBarCode", "cbSaveFailImage")->Checked)?1:0);         //Frank 20170408 (Steven) add Save Fail Image

    WriteIniData(szDir, sGroup, "Select Use CCD Sh1",              (EL<TRadioGroup>("TfBarCode", "rgSelectSh1")->ItemIndex)?1:0);
    WriteIniData(szDir, sGroup, "Select Use CCD Sh2",              (EL<TRadioGroup>("TfBarCode", "rgSelectSh2")->ItemIndex)?1:0);           //kevin 20210814 add Frank 20171011 add Shuttle Check 2DID Pos
    WriteIniData(szDir, sGroup, "2DFileName",                       EL<TEdit>("TfBarCode", "edt2DFileName")->Text);                   //kevin 20210817 2D FILENAME
    WriteIniData(szDir, sGroup, "String Format",                    EL<TRadioGroup>("TfBarCode", "rgStringFormat")->ItemIndex);             //RogerYang 20181222 新增String format選項

    WriteIniData(szDir, sGroup, "Bar Code Use HandShake Communication",(EL<TCheckBox>("TfBarCode", "cbUseHandShakeCommunication")->Checked)?1:0);  //Ifor 20190225 :add Bar Code Use HandShake Communication
    WriteIniData(szDir, sGroup, "Bar Code HandShake Time Out",           EL<TEdit>("TfBarCode", "edHandShakeTimeOut")->Text);                  //Ifor 20190225 :add Bar Code Use HandShake Communication

    WriteIniData(szDir, sGroup, "Bar Code Use Auto Adjust Light",      (EL<TCheckBox>("TfBarCode", "cbAutoAdjustLight")->Checked)?1:0);          //Eastsun 20260527 整合
    WriteIniData(szDir, sGroup, "Bar Code Auto Adjust Ligh Time Out",   EL<TEdit>("TfBarCode", "edAutoAdjustLightTimeOut")->Text);           //Eastsun 20260527 整合

    WriteIniData(szDir, sGroup, "Bar Code Read Multi Line", EL<TComboBox>("TfBarCode", "cbReadMultiLine_2DBarcode")->ItemIndex);          //Ifor 20210407 add: 自製OCR
    WriteIniData(szDir, sGroup, "Bar Code 1st Line Length", EL<TEdit>("TfBarCode", "edt1stLineLength_2DBarCode")->Text);
    WriteIniData(szDir, sGroup, "Bar Code 2nd Line Length", EL<TEdit>("TfBarCode", "edt2ndLineLength_2DBarCode")->Text);
    WriteIniData(szDir, sGroup, "Bar Code Insert String",   EL<TEdit>("TfBarCode", "edtInsertString1stAnd2nd_2DBarCode")->Text);

    if(CosFunction.bRead2DIDFromServer==true)                                   //Jimmychiu 20230925 : read 2did in json file
    {
        WriteIniData(szDir, sGroup, "Check Code By Server 2DID", (EL<TCheckBox>("TfBarCode", "cbCheckCodeByServer2DID")->Checked)?1:0);
        WriteIniData(szDir, sGroup, "asMes2DID_URL            ", EL<TLabeledEdit>("TfBarCode", "le2DID_ServerURL")->Text);
    }
    //==> Eastsun 20260527 整合#BCIR.P05 WriteIniData :KYEC
    WriteIniData(szDir, sGroup, "bBarCodeInspReport", (EL<TCheckBox>("TfBarCode", "cbBarCodeInspReport")->Checked)?1:0);                 //Sam 20240426 : Add BarCoder Inspection Report
    //<== Eastsun 20260527 #BCIR.P05

    if(CosFunction.bMakeWhite2DIDList==true)                                    //RogerYang 20251202 : JCET 2D FT1白名單/FT2比對功能
    {
        WriteIniData(szDir, sGroup, "Make White 2DID List",     (EL<TCheckBox>("TfBarCode", "chkMakeWhite2DIDList")->Checked)?1:0);
    }
    WriteIniData(szDir, sGroup, "bEnableBarcodeCSVCompare", (EL<TCheckBox>("TfBarCode", "cbBarcodeCSVCompare")->Checked)?1:0);            //Ifor 20260511 add: Barcode CSV Compare

    BC_ReadFile();
    if(EL<TCheckBox>("TfBarCode", "cbBarCodeSimulate")->Checked==false)
    BC_spbStartComClick();   // golden BarCode\BarCode.cpp:1460-1460 取代：spbStartCom->Click()：VCL Click() 觸發 OnClick＝spbStartComClick（:2520，DFM 綁定）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1460-1460 -- spbStartCom->Click()：VCL Click() 觸發 OnClick＝spbStartComClick（:2520，DFM 綁定）
        EL<TSpeedButton>("TfBarCode", "spbStartCom")->Click();
#endif // GATE (S12-C save)
    ;   // golden BarCode\BarCode.cpp:1461-1461 取代：spbSave->Down：按鈕外觀
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:1461-1461 -- spbSave->Down：按鈕外觀
    EL<TSpeedButton>("TfBarCode", "spbSave")->Down=false;
#endif // GATE (S12-C save)
    fMain->BackupSetupFile();                                                   //Ifor 20170620 (wei) add Auto BackUp Setup File & Last Data
    if(CosFunction.bReadClipCodeFromUnloader==true)
    {
        BC_WriteUnloaderClipIni();
        BC_ReadUnloaderClipIni();
    }

    if(CUSTOMER_CODE==CC_JCET)                                              //RogerYang 20251215 : JCET 2D FT1白名單/FT2比對功能  刷新uLotInfo選項
    {
        fLotInfo->JCETWhite2DIDShow(BC_JCETUseMakeWhite2DIDList());
    }

}

// golden BarCode\BarCode.cpp:8830  TfBarCode::SetMulti2DMap(int iType)
static void BC_SetMulti2DMap(int iType)
{

    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            Multi2DSiteCH[i][j]->Visible=true;
    BC_CbClear(Multi2DSiteCH[i][j]);   // golden BarCode\BarCode.cpp:8837-8837 取代：TComboBox::Clear 的 VCL 語意（Items 清空＋ItemIndex=-1＋Text=""；vclcompat Clear 只清 Items）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:8837-8837 -- TComboBox::Clear 的 VCL 語意（Items 清空＋ItemIndex=-1＋Text=""；vclcompat Clear 只清 Items）
            Multi2DSiteCH[i][j]->Clear();
#endif // GATE (S12-C save)
            Multi2DSiteCH[i][j]->Items->Add("---");
            Multi2DSiteCH[i][j]->Items->Add("1");
            Multi2DSiteCH[i][j]->Items->Add("2");
            if(iType>=3)
            {
                Multi2DSiteCH[i][j]->Items->Add("3");
                Multi2DSiteCH[i][j]->Items->Add("4");
            }
    filerw::ELComboIndex(Multi2DSiteCH[i][j], 0);   // golden BarCode\BarCode.cpp:8846-8846 取代：TComboBox ItemIndex 的 VCL 連動（帶出 Text）
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:8846-8846 -- TComboBox ItemIndex 的 VCL 連動（帶出 Text）
            Multi2DSiteCH[i][j]->ItemIndex=0;
#endif // GATE (S12-C save)

            if(i==1 && iType==e1x2In1CCD)
            {
                Multi2DSiteCH[i][j]->Visible=false;
            }

            if(j==1 && (iType==e2x1In1CCD || iType==e2x1In2CCD))
            {
                Multi2DSiteCH[i][j]->Visible=false;
            }
        }
    }

}

// golden BarCode\BarCode.cpp:8861  TfBarCode::rgMulti2DTypeClick(TObject *Sender)
static void BC_rgMulti2DTypeClick()
{

    BC_SetMulti2DMap(EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex);

}

// golden BarCode\BarCode.cpp:8825  TfBarCode::chkMulti2DIDClick(TObject *Sender)
static void BC_chkMulti2DIDClick()
{

    EL<TTabSheet>("TfBarCode", "tsMulti2DID")->TabVisible=EL<TCheckBox>("TfBarCode", "chkMulti2DID")->Checked;

}

// golden BarCode\BarCode.cpp:8866  TfBarCode::CheckMulti2DMap()
static bool BC_CheckMulti2DMap()
{

//    for(int i=0; i<2; i++)                                                    //Steven 20240612 : Mark for 對角2D
//    {
//        for(int j=i+1; j<2; j++)
//        {
//            if(Multi2DSiteCH[i][j]->ItemIndex<=0 && Multi2DSiteCH[i][j]->Visible)
//            {
//                ShowMyMessage("2DID map not select!");
//                return true;
//            }
//        }
//    }

    if(EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e1x2In1CCD)
    {
        if(Multi2DSiteCH[0][0]->ItemIndex==Multi2DSiteCH[0][1]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }
    }
    else if(EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x1In1CCD ||
            EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x1In2CCD)
    {
        if(Multi2DSiteCH[0][0]->ItemIndex==Multi2DSiteCH[1][0]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }
    }
    else if(EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x2In1CCD ||
            EL<TRadioGroup>("TfBarCode", "rgMulti2DType")->ItemIndex==e2x2In2CCD)
    {
        if(Multi2DSiteCH[0][0]->ItemIndex!=0 &&                                 //Steven 20240612 : Mark for 對角2D
           Multi2DSiteCH[0][1]->ItemIndex!=0 &&
           Multi2DSiteCH[0][0]->ItemIndex==Multi2DSiteCH[0][1]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }

        if(Multi2DSiteCH[0][0]->ItemIndex!=0 &&
           Multi2DSiteCH[1][0]->ItemIndex!=0 &&
           Multi2DSiteCH[0][0]->ItemIndex==Multi2DSiteCH[1][0]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }

        if(Multi2DSiteCH[0][0]->ItemIndex!=0 &&
           Multi2DSiteCH[1][1]->ItemIndex!=0 &&
           Multi2DSiteCH[0][0]->ItemIndex==Multi2DSiteCH[1][1]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }

        if(Multi2DSiteCH[0][1]->ItemIndex!=0 &&
           Multi2DSiteCH[1][0]->ItemIndex!=0 &&
           Multi2DSiteCH[0][1]->ItemIndex==Multi2DSiteCH[1][0]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }

        if(Multi2DSiteCH[0][1]->ItemIndex!=0 &&
           Multi2DSiteCH[1][1]->ItemIndex!=0 &&
           Multi2DSiteCH[0][1]->ItemIndex==Multi2DSiteCH[1][1]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }

        if(Multi2DSiteCH[1][0]->ItemIndex!=0 &&
           Multi2DSiteCH[1][1]->ItemIndex!=0 &&
           Multi2DSiteCH[1][0]->ItemIndex==Multi2DSiteCH[1][1]->ItemIndex)
        {
            filerw::ELMessage("2DID map set same CH!");
            return true;
        }
    }
    return false;

}

// golden BarCode\BarCode.cpp:11348  TfBarCode::GetBarcodeByServerData()
static AnsiString BC_GetBarcodeByServerData()
{

    //target path = FormSetPath+lotid
    return AnsiString().sprintf("%s%s",TestIF_File.asMes2DID_URL.c_str(),fLotInfo->edtSysLotID->Text.c_str());   // golden BarCode\BarCode.cpp:11351-11351 取代：AnsiString 經 "..." 傳給 sprintf：BCB 可以（AnsiString 就是一個 char*），C++17／g++ 不行 → 同一個格式、傳 c_str()
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:11351-11351 -- AnsiString 經 "..." 傳給 sprintf：BCB 可以（AnsiString 就是一個 char*），C++17／g++ 不行 → 同一個格式、傳 c_str()
    return AnsiString().sprintf("%s%s",TestIF_File.asMes2DID_URL,fLotInfo->edtSysLotID->Text);
#endif // GATE (S12-C save)

}

// golden BarCode\BarCode.cpp:11340  TfBarCode::JCETUseMakeWhite2DIDList()
static bool BC_JCETUseMakeWhite2DIDList()
{

    return (TestIF_File.bEnableBarCode==true &&
            CosFunction.bMakeWhite2DIDList==true &&
            TestIF_File.bChkMakeWhite2DIDList==true &&
            TestIF_File.b2DIDAllowList==true);

}

// golden BarCode\BarCode.cpp:2520  TfBarCode::spbStartComClick(TObject *Sender)
static void BC_spbStartComClick()
{

    if(TestIF_File.bEnableBarCode)
    {
        BC_InitSht2DCodeComPort(2);
    }

}

// golden BarCode\BarCode.cpp:2528  TfBarCode::InitSht2DCodeComPort(int iStep)
static void BC_InitSht2DCodeComPort(int iStep)
{

    if(BAR_CODE_INSTALL==ebctUninstall ||                                       //Steven 20160213 : For 2D code
       BAR_CODE_INSTALL==ebctUseCCDMode ||                                      //Ifor 20190129 : add Cognex EtherNet 通訊
       BAR_CODE_INSTALL==ebctEtherNetCCD ||
       BAR_CODE_INSTALL==ebcUseOCR)                                             //Ifor 20210407 add: 自製OCR
        return;

    if(!kBC_SOFT_SIMULTE) filerw::ELTodo("golden BarCode.cpp:2536-2568 InitSht2DCodeComPort: RS232 2D reader COM restart (Barcode_1..4 StopComm/StartComm, InitBarCodeRS232) not ported");   // golden BarCode\BarCode.cpp:2536-2568 取代：RS232 讀碼器（BAR_CODE_INSTALL=OutShtAMD／InShtIntel）重開 COM：Barcode_1..4 是 TComm 元件、InitBarCodeRS232（:1715）開 COM port，移植樹沒有。上面的 return 條件照 golden（CCD／EtherNet／OCR／未安裝直接 return），SOFT_SIMULTE 下 golden 本來就不做
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:2536-2568 -- RS232 讀碼器（BAR_CODE_INSTALL=OutShtAMD／InShtIntel）重開 COM：Barcode_1..4 是 TComm 元件、InitBarCodeRS232（:1715）開 COM port，移植樹沒有。上面的 return 條件照 golden（CCD／EtherNet／OCR／未安裝直接 return），SOFT_SIMULTE 下 golden 本來就不做
    #ifndef SOFT_SIMULTE
    if(iStep==0)
    {
        EL<TControl>("TfBarCode", "Barcode_1")->StopComm();
        EL<TControl>("TfBarCode", "Barcode_1")->StartComm();
        if(BAR_CODE_USECOUNT>=4)                                                //Ifor 20190409 : add Shuttle 僅用兩顆CCD
        {
            EL<TControl>("TfBarCode", "Barcode_2")->StopComm();
            EL<TControl>("TfBarCode", "Barcode_2")->StartComm();
        }
    }
    else if(iStep==1)
    {
        EL<TControl>("TfBarCode", "Barcode_3")->StopComm();
        EL<TControl>("TfBarCode", "Barcode_3")->StartComm();
        if(BAR_CODE_USECOUNT>=4)                                                //Ifor 20190409 : add Shuttle 僅用兩顆CCD
        {
            EL<TControl>("TfBarCode", "Barcode_4")->StopComm();
            EL<TControl>("TfBarCode", "Barcode_4")->StartComm();
        }
    }
    else
    {
        EL<TControl>("TfBarCode", "Barcode_1")->StopComm();
        EL<TControl>("TfBarCode", "Barcode_3")->StopComm();
        if(BAR_CODE_USECOUNT>=4)                                                //Ifor 20190409 : add Shuttle 僅用兩顆CCD
        {
            EL<TControl>("TfBarCode", "Barcode_2")->StopComm();
            EL<TControl>("TfBarCode", "Barcode_4")->StopComm();
        }
        InitBarCodeRS232(2);                                                    //wei 20150912 2D
    }
    #endif
#endif // GATE (S12-C save)

}

// golden BarCode\BarCode.cpp:11562  TfBarCode::ReadUnloaderClipIni()
static void BC_ReadUnloaderClipIni()
{

    if(elUnloaderClip!=NULL)
    {
        elUnloaderClip->ReadEditTextFromFile(AuthPath, GetUnloaderClipFileName());
        elUnloaderClip->InitialDataToEdit();
    filerw::ELTodo("golden BarCode.cpp:11568-11571 ccdUnloader[].SetCommParameter not ported (no uCCDUnloaderClip in the port)");   // golden BarCode\BarCode.cpp:11568-11571 取代：ccdUnloader[i].SetCommParameter：移植樹沒有 ccdUnloader[]（見建構子那條）；elUnloaderClip 在本 TU 永遠 NULL，走不到這裡
#if 0 // GATE (S12-C save) golden BarCode\BarCode.cpp:11568-11571 -- ccdUnloader[i].SetCommParameter：移植樹沒有 ccdUnloader[]（見建構子那條）；elUnloaderClip 在本 TU 永遠 NULL，走不到這裡
        for(int i=eUnloaderClipAuto1;i<eUnloaderClipTotal;i++)
        {
            ccdUnloader[i].SetCommParameter(ccdUnloader[i].sIP,ccdUnloader[i].sPort);
        }
#endif // GATE (S12-C save)
    }

}

// golden BarCode\BarCode.cpp:11575  TfBarCode::WriteUnloaderClipIni()
static void BC_WriteUnloaderClipIni()
{

    if(elUnloaderClip!=NULL)
    {
        elUnloaderClip->SaveEditTextToFile(AuthPath, GetUnloaderClipFileName());
    }

}

