# 原文 12／01

[證據入口](../evidence.md)。來源 `HT9011UC_Cpp_V3.33.906.0/TesterComm/Rs232/Rs232Ui.cpp`；function／region `TfRS232Main::~TfRS232Main()`。

來源 commit `7f1e937f2dcecba0c1937dea3731d90132f22803`；完整摘錄 SHA256 `daa3608bacb733c8edf95f6f96ad57587d4413d763224e1b84f11b697f3d0e88`。
此頁只保存選定原文，不代表實機執行；頁間接回同一完整摘錄。

```cpp
<!-- preserved-content:start -->
TfRS232Main::~TfRS232Main()
{
    delete CommTester;        CommTester       = NULL;
    delete CommTester_TTL;    CommTester_TTL   = NULL;
    delete CommTester_TTL_2;  CommTester_TTL_2 = NULL;
    if (uServer != NULL)
    {
        delete uServer;
        uServer = NULL;
    }

    for (size_t i = 0; i < MY_DUT_PAL.size(); ++i)
    {
        if (MY_DUT_PAL[i] != NULL)
            delete MY_DUT_PAL[i];
    }
    MY_DUT_PAL.clear();
    if (slCmdList != NULL)              // created by ShowVersion (golden MainForm.cpp:86)
    {
        delete slCmdList;
        slCmdList = NULL;
    }
    if (slRS232Log != NULL)             // golden ctor :361, freed by FormDestroy :395
    {
        delete slRS232Log;
        slRS232Log = NULL;
    }
    if (sBarCode != NULL)               // golden ctor :376, freed by FormClose :552
    {
        delete sBarCode;
        sBarCode = NULL;
    }
    if (sBarCode_ASE_CL != NULL)        // golden ctor :377, freed by FormClose :553
    {
        delete sBarCode_ASE_CL;
        sBarCode_ASE_CL = NULL;
    }

    // MainForm.dfm widgets (dfm order)
    delete labDebugMode;
    delete PageControl1;
    delete ts_STD;
    delete PageControl2;
    delete ts_STD_Log;
    delete ts_STD_BinLog;
    delete MemoBinData;
    delete Panel2;
    delete cbShowLog;
    delete btClear;
    delete cbSaveLog;
    delete ts_STD_Setup;
    delete GroupBox1;
    delete Label1;
    delete Label2;
    delete Label3;
    delete Label4;
    delete Label5;
    delete cbBaudRate;
    delete cbByteSize;
    delete cbStopBit;
    delete cbParity;
    delete cbDevice;
    delete btnUpdate;
    delete GroupBox14;
    delete Label6;
    delete edReadIntervalTimeout;
    delete ts_STD_Simu;
    delete btManualTest;
    delete btnAllUse;
    delete btnAllNoUse;
    delete ts_AVAGO;
    delete PageControl3;
    delete TabSheet4;
    delete PageControl4;
    delete ts_AVAGOLogSite1;
    delete mm_AvagoLogSite1;
    delete cb_AvagoShowLogSite1;
    delete bt_AvagoClearLogSite1;
    delete cb_AvagoSaveLogSite1;
    delete TabSheet2;
    delete bt_UpdateSite1;
    delete GroupBox2;
    delete Label17;
    delete Label18;
    delete Label25;
    delete Label26;
    delete Label27;
    delete cb_AvagoBaudSite1;
    delete cb_AvagoSizeSite1;
    delete cb_AvagoStopSite1;
    delete cb_AvagoParitySite1;
    delete cb_AvagoComSite1;
    delete GroupBox3;
    delete Label28;
    delete Label29;
    delete Label30;
    delete Label31;
    delete cb_AvagoDTRSite1;
    delete cb_AvagoRTSSite1;
    delete cb_AvagoTxCSite1;
    delete ed_AvagoTimeOutTotalSite1;
    delete ed_AvagoTimeOutSite1;
    delete TabSheet5;
    delete PageControl6;
    delete TabSheet6;
    delete mm_AvagoLogSite2;
    delete cb_AvagoShowLogSite2;
    delete bt_AvagoClearLogSite2;
    delete cb_AvagoSaveLogSite2;
    delete TabSheet7;
    delete bt_UpdateSite2;
    delete GroupBox4;
    delete Label32;
    delete Label33;
    delete Label34;
    delete Label35;
    delete Label36;
    delete cb_AvagoBaudSite2;
    delete cb_AvagoSizeSite2;
    delete cb_AvagoStopSite2;
    delete cb_AvagoParitySite2;
    delete cb_AvagoComSite2;
    delete GroupBox5;
    delete Label38;
    delete Label39;
    delete Label40;
    delete Label41;
    delete cb_AvagoDTRSite2;
    delete cb_AvagoRTSSite2;
    delete cb_AvagoTxCSite2;
    delete ed_AvagoTimeOutTotalSite2;
    delete ed_AvagoTimeOutSite2;
    delete TabSheet1;
    delete Label42;
    delete Label43;
    delete bt_SimulateManualTestSite2;
    delete ed_SimulateNu;
    delete ed_Number;
    delete tsSPRD;
    delete PageControl5;
    delete TabSheet3;
    delete Pc_SPRDSite1Control;
    delete ts_SPRDLogSite1;
    delete mm_SPRDLogSite1;
    delete cb_SPRDShowLogSite1;
    delete bt_SPRDClearLogSite1;
    delete cb_SPRDSaveLogSite1;
    delete ts_SPRDSetupSite1;
    delete Button2;
    delete GroupBox6;
    delete Label44;
    delete Label45;
    delete Label46;
    delete Label47;
    delete Label48;
    delete cb_SPRDBaudSite1;
    delete cb_SPRDSizeSite1;
    delete cb_SPRDStopSite1;
    delete cb_SPRDParitySite1;
    delete cb_SPRDComSite1;
    delete GroupBox7;
    delete Label49;
    delete Label50;
    delete Label51;
    delete Label52;
    delete cb_SPRDDTRSite1;
    delete cb_SPRDRTSSite1;
    delete cb_SPRDTxCSite1;
    delete ed_SPRDTimeOutTotalSite1;
    delete ed_SPRDTimeOutSite1;
    delete TabSheet10;
    delete Pc_SPRDSite2Control;
    delete ts_SPRDLogSite2;
    delete mm_SPRDLogSite2;
    delete cb_SPRDShowLogSite2;
    delete bt_SPRDClearLogSite2;
    delete cb_SPRDSaveLogSite2;
    delete ts_SPRDSetupSite2;
    delete Button4;
    delete GroupBox8;
    delete Label53;
    delete Label54;
    delete Label55;
    delete Label56;
    delete Label57;
    delete cb_SPRDBaudSite2;
    delete cb_SPRDSizeSite2;
    delete cb_SPRDStopSite2;
    delete cb_SPRDParitySite2;
    delete cb_SPRDComSite2;
    delete GroupBox9;
    delete Label58;
    delete Label59;
    delete Label60;
    delete Label61;
    delete cb_SPRDDTRSite2;
    delete cb_SPRDRTSSite2;
    delete cb_SPRDTxCSite2;
    delete ed_SPRDTimeOutTotalSite2;
    delete ed_SPRDTimeOutSite2;
    delete TabSheet8;
    delete Pc_SPRDSite3Control;
    delete ts_SPRDLogSite3;
    delete mm_SPRDLogSite3;
    delete cb_SPRDShowLogSite3;
    delete bt_SPRDClearLogSite3;
    delete cb_SPRDSaveLogSite3;
    delete ts_SPRDSetupSite3;
    delete Button3;
    delete GroupBox10;
    delete Label80;
    delete Label81;
    delete Label82;
    delete Label83;
    delete Label84;
    delete cb_SPRDBaudSite3;
    delete cb_SPRDSizeSite3;
    delete cb_SPRDStopSite3;
    delete cb_SPRDParitySite3;
    delete cb_SPRDComSite3;

<!-- preserved-content:end -->
```
