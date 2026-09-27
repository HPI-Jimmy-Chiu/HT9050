// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 ACTForm 的讀寫檔（HTEditList： → D:\HT9045\System\AutoTemperature.ini [ACT] iThermoCtrlType／CheckIntervalTime／CalibrationRange／dCalibrationRange[0..2]／ReadScanTime／OffsetLimit／SingleOffsetLimit／OffsetMethod、[Display] Colums、[COMPort] CommName／BaudRate／ByteSize／StopBits／Parity（讀：FormShow；寫：btnUpdateClick）、D:\HT9045_Log\AutoTempCalibration\<YYYY-MM-DD hh_mm_ss>.txt（寫：palSaveLogClick））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\AutoTemperature.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden AutoTemperature.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 28 個（header 宣告 79 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "cmydef.h"
#include "common.h"
#include "cprod.h"
#include "MachineType.h"
#include "vclcompat/IniFiles.h"
#include "vclcompat/SysUtils.h"

// golden AutoTemperature.h:18-32（本表單私用的型別；移植樹沒有 —— 20260926 grep ACT_DATA／Type_Agilent34970A／ATC_MaxGroup 0 筆）
#define ATC_MaxGroup 20
typedef struct {
    int iThermoCtrlType;
    int iCheckIntervalTime;
    int iCalibrationRange;
    double dCalibrationRange[3];
    int iReadScanTime;
    int iOffsetLimit;
    int iSingleOffset;
    int iDisplayColumn;
    int iOffsetMethod;
} ACT_DATA;
enum eACTThermoType{Type_Agilent34970A=0, Type_DeltaDTB4824=1};  //Steven 20210510 : 自動K溫使用DTB4824
// golden SaveACTData :849-851 IniFile1->WriteFloat：vclcompat::TIniFile 沒有 WriteFloat；BCB6 TCustomIniFile.WriteFloat 的本體就是
// WriteString(Section, Name, FloatToStr(Value))（同 ProductionInfo/uPAT_Function.cpp:621 的 G17 巨集）。放在所有 #include 之後。
#define WriteFloat(_sec_,_key_,_val_) WriteString(_sec_,_key_,FloatToStr(_val_))

// golden AutoTemperature.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static bool bCommConnect=false;       // golden AutoTemperature.h:191（建構子 :291 設 false；只有 OpenCommPort／CloseCommPort 改它）
static bool bAutoStart=false;         // golden AutoTemperature.h:192
static ACT_DATA ACTData;              // golden AutoTemperature.h:194（golden 表單外沒有讀者：全樹只有 main.cpp:29889 ACTForm->Show()）
static TCheckBox*  _cbBase[3];        // golden AutoTemperature.h:196
static TPanel*     _pnlBaseTemp[3];   // golden AutoTemperature.h:197
static AnsiString  FilePath;          // golden AutoTemperature.h:198（建構子 :398 才設；:297 LoadACTData 時還是空字串，見設定檔檔頭）
static AnsiString CommName;           // golden AutoTemperature.h:178（private）
static int iTotalGroupCount=0;        // golden AutoTemperature.h:199（建構子依 iSocketBaseTempCount；只有 TimerACTTimer 用）
static int iGroupSet[ATC_MaxGroup];   // golden AutoTemperature.h:200（同上）
static bool bBaseFinish[5];           // golden AutoTemperature.h:206
static int iReceiveData=0;            // golden AutoTemperature.h:211
static int iAddr=0;                   // golden AutoTemperature.h:212
static TComm* ACT_Com=new TComm(nullptr);   // golden AutoTemperature.h:69 ACTCom（DFM 的 TComm，AutoTemperature.dfm:994；vclcompat TComm 建構不碰硬體，沒有人 StartComm）

using filerw::EL;

// 前置宣告
static void ACT_TACTForm();
static void ACT_FormShow();
static void ACT_btnUpdateClick();
static void ACT_SaveACTData(ACT_DATA& Data);
static void ACT_LoadACTData(ACT_DATA& Data);
static bool ACT_SaveCommData(const TComm* Comm);
static bool ACT_LoadCommData(TComm* Comm);
static void ACT_palSaveLogClick();

// golden AutoTemperature.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void ACT_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TACTForm", "cbBaudRate")->Items;
      s->Add("4800");
      s->Add("7200");
      s->Add("9600");
      s->Add("14400");
      s->Add("19200");
    }
    { TStringList* s = EL<TComboBox>("TACTForm", "cbByteSize")->Items;
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TACTForm", "cbDevice")->Items;
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
    }
    { TStringList* s = EL<TComboBox>("TACTForm", "cbMeasurePoint")->Items;
      s->Add("Plate 1");
      s->Add("Plate 2");
      s->Add("Shuttle 1");
      s->Add("Shuttle 2");
      s->Add("Head 1/2");
      s->Add("Head 3/4");
      s->Add("Head 5/6");
      s->Add("Head 7/8");
      s->Add("Dut");
      s->Add("Chamber");
      s->Add("CCD");
      s->Add("A1");
      s->Add("C1");
      s->Add("E1");
      s->Add("G1");
      s->Add("B1");
      s->Add("D1");
      s->Add("F1");
      s->Add("H1");
      s->Add("A2");
      s->Add("C2");
      s->Add("E2");
      s->Add("G2");
      s->Add("B2");
      s->Add("D2");
      s->Add("F2");
      s->Add("H2");
      s->Add("Heat Gun1");
      s->Add("Heat Gun2");
    }
    { TStringList* s = EL<TComboBox>("TACTForm", "cbParity")->Items;
      s->Add("None");
      s->Add("Odd");
      s->Add("Even");
      s->Add("Mark");
      s->Add("Space");
    }
    { TStringList* s = EL<TComboBox>("TACTForm", "cbStopBit")->Items;
      s->Add("1");
      s->Add("1.5");
      s->Add("2");
    }
    { TStringList* s = EL<TComboBox>("TACTForm", "cbbThermoCtrlType")->Items;
      s->Add("Agilent 34970A");
      s->Add("Delta DTB4824");
    }
    { TStringList* s = EL<TRadioGroup>("TACTForm", "rgOffsetMethod")->Items;
      s->Add("Max - Min");
      s->Add("Average");
    }
}

// golden AutoTemperature.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kACT_ParentOf[][2] = {
    {"GroupBox1", "tsSetup"},
    {"MemoOffset", "Panel3"},
    {"Panel3", "tsTemperature"},
    {"ScrollBox1", "tsTemperature"},
    {"cbBase01", "pnlData"},
    {"cbBase02", "pnlData"},
    {"cbBase03", "pnlData"},
    {"cbBaudRate", "GroupBox1"},
    {"cbByteSize", "GroupBox1"},
    {"cbDevice", "GroupBox1"},
    {"cbParity", "GroupBox1"},
    {"cbStopBit", "GroupBox1"},
    {"cbbThermoCtrlType", "GroupBox1"},
    {"edColumns", "gbDisplay"},
    {"edSingleOffset", "gbACTSetting"},
    {"edtCalibrationRange", "gbACTSetting"},
    {"edtCheckIntervalTime", "gbACTSetting"},
    {"edtOffsetLimit", "gbACTSetting"},
    {"edtRange1", "pnlData"},
    {"edtRange2", "pnlData"},
    {"edtRange3", "pnlData"},
    {"edtReadScanTime", "gbACTSetting"},
    {"gbACTSetting", "tsSetup"},
    {"gbDisplay", "tsSetup"},
    {"pnlBaseTemp01", "pnlData"},
    {"pnlBaseTemp02", "pnlData"},
    {"pnlBaseTemp03", "pnlData"},
    {"pnlData", "tsTemperature"},
    {"rgOffsetMethod", "gbACTSetting"},
    {"tsSetup", "pcACT"},
    {"tsTemperature", "pcACT"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void ACT_CreateContainerProxies()
{
    EL<TGroupBox>("TACTForm", "GroupBox1");
    EL<TPanel>("TACTForm", "Panel3");
    EL<TGroupBox>("TACTForm", "gbACTSetting");
    EL<TGroupBox>("TACTForm", "gbDisplay");
    EL<TPageControl>("TACTForm", "pcACT");
    EL<TPanel>("TACTForm", "pnlData");
    EL<TTabSheet>("TACTForm", "tsSetup");
    EL<TTabSheet>("TACTForm", "tsTemperature");
    filerw::ELSetParents("TACTForm", kACT_ParentOf, (int)(sizeof(kACT_ParentOf) / sizeof(kACT_ParentOf[0])));
}

// golden AutoTemperature.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void ACT_DfmState()
{
    EL<TControl>("TACTForm", "TimerACT")->Enabled = false;
    EL<TEdit>("TACTForm", "edtCalibrationRange")->Visible = false;
    EL<TComboBox>("TACTForm", "cbBaudRate")->Text = "cbBaudRate";
    EL<TComboBox>("TACTForm", "cbByteSize")->Text = "cbByteSize";
    EL<TComboBox>("TACTForm", "cbDevice")->Text = "cbCOM";
    EL<TComboBox>("TACTForm", "cbParity")->Text = "cbParity";
    EL<TComboBox>("TACTForm", "cbStopBit")->Text = "cbStopBit";
    EL<TComboBox>("TACTForm", "cbbThermoCtrlType")->Text = "Delta DTB4824";
    EL<TComboBox>("TACTForm", "cbbThermoCtrlType")->ItemIndex = 1;
    EL<TEdit>("TACTForm", "edColumns")->Text = "3";
    EL<TEdit>("TACTForm", "edSingleOffset")->Text = "30";
    EL<TEdit>("TACTForm", "edtCalibrationRange")->Text = "3";
    EL<TEdit>("TACTForm", "edtCheckIntervalTime")->Text = "10";
    EL<TEdit>("TACTForm", "edtOffsetLimit")->Text = "30";
    EL<TEdit>("TACTForm", "edtRange1")->Text = "3";
    EL<TEdit>("TACTForm", "edtRange2")->Text = "3";
    EL<TEdit>("TACTForm", "edtRange3")->Text = "3";
    EL<TEdit>("TACTForm", "edtReadScanTime")->Text = "5";
    EL<TRadioGroup>("TACTForm", "rgOffsetMethod")->ItemIndex = 1;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kACT_EnableAll[] = {
    "GroupBox1", "Panel3", "ScrollBox1", "cbBase01", "cbBase02", "cbBase03",
    "gbACTSetting", "gbDisplay", "pcACT", "pnlBaseTemp01", "pnlBaseTemp02", "pnlBaseTemp03",
    "pnlData", "rgOffsetMethod", "tsSetup", "tsTemperature",
};

// 存檔流程（btnUpdateClick、SaveACTData、SaveCommData）讀的替身 16 個 —— FileRW/ACTForm.cpp 用它決定頁面必須送哪些值
static const char* const kACT_SaveReads[] = {
    "cbBaudRate", "cbByteSize", "cbDevice", "cbParity", "cbStopBit", "cbbThermoCtrlType",
    "edColumns", "edSingleOffset", "edtCalibrationRange", "edtCheckIntervalTime", "edtOffsetLimit", "edtRange1",
    "edtRange2", "edtRange3", "edtReadScanTime", "rgOffsetMethod",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void ACT_CreateSaveProxies()
{
    EL<TComboBox>("TACTForm", "cbBaudRate");
    EL<TComboBox>("TACTForm", "cbByteSize");
    EL<TComboBox>("TACTForm", "cbDevice");
    EL<TComboBox>("TACTForm", "cbParity");
    EL<TComboBox>("TACTForm", "cbStopBit");
    EL<TComboBox>("TACTForm", "cbbThermoCtrlType");
    EL<TEdit>("TACTForm", "edColumns");
    EL<TEdit>("TACTForm", "edSingleOffset");
    EL<TEdit>("TACTForm", "edtCalibrationRange");
    EL<TEdit>("TACTForm", "edtCheckIntervalTime");
    EL<TEdit>("TACTForm", "edtOffsetLimit");
    EL<TEdit>("TACTForm", "edtRange1");
    EL<TEdit>("TACTForm", "edtRange2");
    EL<TEdit>("TACTForm", "edtRange3");
    EL<TEdit>("TACTForm", "edtReadScanTime");
    EL<TRadioGroup>("TACTForm", "rgOffsetMethod");
}

// golden AutoTemperature.cpp:289  TACTForm::TACTForm(TComponent* Owner)
static void ACT_TACTForm()
{

    bCommConnect=false;
    bAutoStart=false;

    TCheckBox* pBase[3]={EL<TCheckBox>("TACTForm", "cbBase01"), EL<TCheckBox>("TACTForm", "cbBase02"), EL<TCheckBox>("TACTForm", "cbBase03")};
    TPanel*  pBaseTemp[3]={EL<TPanel>("TACTForm", "pnlBaseTemp01"), EL<TPanel>("TACTForm", "pnlBaseTemp02"), EL<TPanel>("TACTForm", "pnlBaseTemp03")};
    ACT_LoadACTData(ACTData);
    for(int i=0; i<3; i++)
    {
        _cbBase[i]=pBase[i];
        _pnlBaseTemp[i]=pBaseTemp[i];
        bBaseFinish[i]=false;
    }

    if(ACTData.iThermoCtrlType==Type_Agilent34970A)
    {
        for(int i=0; i<tcTotalCount; i++)   //決定使用幾個制冷片
        {
    ;   // golden AutoTemperature.cpp:309-309 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:309-309 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, i, asTempCtrl[i]));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:310-310 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:310-310 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal[i]->SetParent(EL<TControl>("TACTForm", "ScrollBox1"));
#endif // GATE (S12-C save)
        }
    }
    else if(ACTData.iThermoCtrlType==Type_DeltaDTB4824) //Steven 20210510 : 自動K溫使用DTB4824
    {
    ;   // golden AutoTemperature.cpp:315-315 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:315-315 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  0, "HP 1-1",    tcHotPlate1, "HP1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:316-316 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:316-316 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  1, "HP 1-2",    tcHotPlate1, "HP1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:317-317 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:317-317 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  2, "HP 1-3",    tcHotPlate1, "HP1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:318-318 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:318-318 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  3, "HP 1-4",    tcHotPlate1, "HP1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:319-319 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:319-319 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  4, "HP 1-5",    tcHotPlate1, "HP1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:320-320 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:320-320 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  5, "HP 2-1",    tcHotPlate2, "HP2"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:321-321 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:321-321 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  6, "HP 2-2",    tcHotPlate2, "HP2"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:322-322 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:322-322 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  7, "HP 2-3",    tcHotPlate2, "HP2"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:323-323 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:323-323 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  8, "HP 2-4",    tcHotPlate2, "HP2"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:324-324 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:324-324 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this,  9, "HP 2-5",    tcHotPlate2, "HP2"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:325-325 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:325-325 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 10, "SHT 1-1",   tcShuttle1,  "SHT1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:326-326 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:326-326 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 11, "SHT 1-2",   tcShuttle1,  "SHT1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:327-327 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:327-327 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 12, "SHT 2-1",   tcShuttle2,  "SHT2"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:328-328 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:328-328 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 13, "SHT 2-2",   tcShuttle2,  "SHT2"));
#endif // GATE (S12-C save)
        if(iSocketBaseTempCount==eDut2ea)   //Steven 20140116 : Socket數量,改用數字設定
        {
    ;   // golden AutoTemperature.cpp:331-331 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:331-331 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 14, "Base 1-1",    tcDUT1,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:332-332 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:332-332 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 15, "Base 1-2",    tcDUT1,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:333-333 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:333-333 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 16, "Base 2-1",    tcDUT2,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:334-334 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:334-334 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 17, "Base 2-2",    tcDUT2,    "Base"));
#endif // GATE (S12-C save)
            iTotalGroupCount=9;
        }
        else if(iSocketBaseTempCount==eDut4ea)
        {
    ;   // golden AutoTemperature.cpp:339-339 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:339-339 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 14, "Base 1",    tcDUT1,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:340-340 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:340-340 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 15, "Base 2",    tcDUT2,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:341-341 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:341-341 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 16, "Base 3",    tcDUT3,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:342-342 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:342-342 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 17, "Base 4",    tcDUT4,    "Base"));
#endif // GATE (S12-C save)
            iTotalGroupCount=11;
        }
        else
        {
    ;   // golden AutoTemperature.cpp:347-347 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:347-347 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 14, "Base 1",    tcSocket,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:348-348 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:348-348 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 15, "Base 2",    tcSocket,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:349-349 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:349-349 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 16, "Base 3",    tcSocket,    "Base"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:350-350 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:350-350 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal.push_back(new TMyATPanel(this, 17, "Base 4",    tcSocket,    "Base"));
#endif // GATE (S12-C save)
            iTotalGroupCount=8;
        }
    ;   // golden AutoTemperature.cpp:353-353 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:353-353 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 18, "Chamber 1", tcChamber,   "Chamber"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:354-354 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:354-354 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 19, "Chamber 2", tcChamber,   "Chamber"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:355-355 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:355-355 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 20, "Chamber 3", tcChamber,   "Chamber"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:356-356 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:356-356 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 21, "HotGun 1",  tcHeatGun1,  "HotGun1"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:357-357 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:357-357 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 22, "HotGun 2",  tcHeatGun2,  "HotGun2"));
#endif // GATE (S12-C save)

    ;   // golden AutoTemperature.cpp:359-359 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:359-359 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 23, "Reserved 1",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:360-360 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:360-360 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 24, "Reserved 2",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:361-361 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:361-361 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 25, "Reserved 3",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:362-362 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:362-362 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 26, "Reserved 4",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:363-363 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:363-363 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 27, "Reserved 5",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:364-364 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:364-364 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 28, "Reserved 6",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:365-365 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:365-365 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 29, "Reserved 7",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:366-366 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:366-366 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 30, "Reserved 8",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:367-367 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:367-367 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 31, "Reserved 9",  tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:368-368 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:368-368 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        myATPal.push_back(new TMyATPanel(this, 32, "Reserved 10", tcCCD_2,   "Reserved"));
#endif // GATE (S12-C save)

    ;   // golden AutoTemperature.cpp:370-370 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:370-370 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        for(int i=0; i<int(myATPal.size()); i++)
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:371-371 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:371-371 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal[i]->SetParent(EL<TControl>("TACTForm", "ScrollBox1"));
#endif // GATE (S12-C save)

        iGroupSet[0]=tcHotPlate1;
        iGroupSet[1]=tcHotPlate2;
        iGroupSet[2]=tcShuttle1;
        iGroupSet[3]=tcShuttle2;
        iGroupSet[4]=tcChamber;
        iGroupSet[5]=tcHeatGun1;
        iGroupSet[6]=tcHeatGun2;
        if(iSocketBaseTempCount==eDut2ea)
        {
            iGroupSet[7]=tcDUT1;
            iGroupSet[8]=tcDUT2;
        }
        else if(iSocketBaseTempCount==eDut4ea)
        {
            iGroupSet[7]=tcDUT1;
            iGroupSet[8]=tcDUT2;
            iGroupSet[9]=tcDUT3;
            iGroupSet[10]=tcDUT4;
        }
        else
        {
            iGroupSet[7]=tcSocket;
        }
    }

    FilePath="D:\\HT9045\\System\\AutoTemperature.ini";
    EL<TControl>("TACTForm", "TimerACT")->Enabled = false;

}

// golden AutoTemperature.cpp:419  TACTForm::FormShow(TObject *Sender)
static void ACT_FormShow()
{

    ;   // golden AutoTemperature.cpp:421-422 取代：Top／Left：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:421-422 -- Top／Left：視窗位置（HTML 不用）
    Top=4;
    Left=4;
#endif // GATE (S12-C save)
    bAutoStart = false;
    ;   // golden AutoTemperature.cpp:424-424 取代：ChangeStartState（:794）：START 鈕的字與顏色（bAutoStart=false → "START"）、true 時啟動 tTempScanTimer —— 自動 K 溫流程的 GUI，交 Jimmy
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:424-424 -- ChangeStartState（:794）：START 鈕的字與顏色（bAutoStart=false → "START"）、true 時啟動 tTempScanTimer —— 自動 K 溫流程的 GUI，交 Jimmy
    ChangeStartState();
#endif // GATE (S12-C save)
    int iGroup=0;

    // ACT Setting -------------------------------------->>
    ACT_LoadACTData(ACTData);
    filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbbThermoCtrlType"), ACTData.iThermoCtrlType);
    EL<TEdit>("TACTForm", "edtCheckIntervalTime")->Text = AnsiString(ACTData.iCheckIntervalTime);
    EL<TEdit>("TACTForm", "edtCalibrationRange") ->Text = AnsiString(ACTData.iCalibrationRange  );
    EL<TEdit>("TACTForm", "edtRange1")->Text = AnsiString(ACTData.dCalibrationRange[0]  );   //Steven 20210520 : K溫範圍改成三組
    EL<TEdit>("TACTForm", "edtRange2")->Text = AnsiString(ACTData.dCalibrationRange[1]  );
    EL<TEdit>("TACTForm", "edtRange3")->Text = AnsiString(ACTData.dCalibrationRange[2]  );
    EL<TEdit>("TACTForm", "edtReadScanTime")     ->Text = AnsiString(ACTData.iReadScanTime      );
    EL<TEdit>("TACTForm", "edtOffsetLimit")      ->Text = AnsiString(ACTData.iOffsetLimit       );
    EL<TEdit>("TACTForm", "edSingleOffset")      ->Text = AnsiString(ACTData.iSingleOffset      );
    EL<TEdit>("TACTForm", "edColumns")           ->Text = AnsiString(ACTData.iDisplayColumn     );
    EL<TRadioGroup>("TACTForm", "rgOffsetMethod")->ItemIndex  = ACTData.iOffsetMethod;
    ;   // golden AutoTemperature.cpp:440-440 取代：ShowDefaultPos（:1327）：量測點面板 myATPal（TMyATPanel）的排版 —— GUI，移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:440-440 -- ShowDefaultPos（:1327）：量測點面板 myATPal（TMyATPanel）的排版 —— GUI，移植樹沒有這個類別
    ShowDefaultPos();
#endif // GATE (S12-C save)
    // ACT Setting --------------------------------------<<

    // Comm Port ---------------------------------------->>
    ACT_LoadCommData(ACT_Com);   // golden AutoTemperature.cpp:444-444 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:444-444 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    ACT_LoadCommData(EL<TControl>("TACTForm", "ACTCom"));
#endif // GATE (S12-C save)

    filerw::ELComboText(EL<TComboBox>("TACTForm", "cbDevice"), CommName);   // golden AutoTemperature.cpp:446-446 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:446-446 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    EL<TComboBox>("TACTForm", "cbDevice")    ->Text = CommName;
#endif // GATE (S12-C save)
    filerw::ELComboText(EL<TComboBox>("TACTForm", "cbBaudRate"), IntToStr(ACT_Com->BaudRate));   // golden AutoTemperature.cpp:447-447 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:447-447 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    EL<TComboBox>("TACTForm", "cbBaudRate")  ->Text = IntToStr(EL<TControl>("TACTForm", "ACTCom")->BaudRate);
#endif // GATE (S12-C save)

    if(ACT_Com->ByteSize==_5)   // golden AutoTemperature.cpp:449-449 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:449-449 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    if(EL<TControl>("TACTForm", "ACTCom")->ByteSize==_5)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbByteSize"), 0);
    else if(ACT_Com->ByteSize==_6)   // golden AutoTemperature.cpp:451-451 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:451-451 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->ByteSize==_6)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbByteSize"), 1);
    else if(ACT_Com->ByteSize==_7)   // golden AutoTemperature.cpp:453-453 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:453-453 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->ByteSize==_7)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbByteSize"), 2);
    else if(ACT_Com->ByteSize==_8)   // golden AutoTemperature.cpp:455-455 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:455-455 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->ByteSize==_8)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbByteSize"), 3);

    if(ACT_Com->StopBits==_1)   // golden AutoTemperature.cpp:458-458 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:458-458 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    if(EL<TControl>("TACTForm", "ACTCom")->StopBits==_1)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbStopBit"), 0);
    else if(ACT_Com->StopBits==_1_5)   // golden AutoTemperature.cpp:460-460 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:460-460 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->StopBits==_1_5)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbStopBit"), 1);
    else if(ACT_Com->StopBits==_2)   // golden AutoTemperature.cpp:462-462 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:462-462 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->StopBits==_2)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbStopBit"), 2);

    if(ACT_Com->Parity==None)   // golden AutoTemperature.cpp:465-465 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:465-465 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    if(EL<TControl>("TACTForm", "ACTCom")->Parity==None)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbParity"), 0);
    else if(ACT_Com->Parity==Odd)   // golden AutoTemperature.cpp:467-467 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:467-467 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->Parity==Odd)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbParity"), 1);
    else if(ACT_Com->Parity==Even)   // golden AutoTemperature.cpp:469-469 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:469-469 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->Parity==Even)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbParity"), 2);
    else if(ACT_Com->Parity==Space)   // golden AutoTemperature.cpp:471-471 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:471-471 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    else if(EL<TControl>("TACTForm", "ACTCom")->Parity==Space)
#endif // GATE (S12-C save)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbParity"), 4);
    else//if(ACTCom->Parity==Mark)
        filerw::ELComboIndex(EL<TComboBox>("TACTForm", "cbParity"), 3);

    filerw::ELTodo("golden AutoTemperature.cpp:476 OpenCommPort(): opens the ACT thermometer COM port (ACTCom->StartComm) -- communication half not ported (Jimmy)");   // golden AutoTemperature.cpp:476-476 取代：OpenCommPort（:616）：開 ACTCom（GetCOMPortStatus＋StartComm）—— 通訊那一半，交 Jimmy
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:476-476 -- OpenCommPort（:616）：開 ACTCom（GetCOMPortStatus＋StartComm）—— 通訊那一半，交 Jimmy
    OpenCommPort();
#endif // GATE (S12-C save)
    // Comm Port ----------------------------------------<<

    EL<TPanel>("TACTForm", "pnlBaseTemp01")->Caption=AnsiString(Temperature.fLowBase);
    EL<TCheckBox>("TACTForm", "cbBase01")->Checked=true;

    if(Temperature.iTempMode==0)
    {
        EL<TPanel>("TACTForm", "pnlBaseTemp02")->Caption = "-----";
        EL<TCheckBox>("TACTForm", "cbBase02")->Checked = false;
        EL<TCheckBox>("TACTForm", "cbBase02")->Enabled = false;
    }
    else
    {
        EL<TPanel>("TACTForm", "pnlBaseTemp02")->Caption  = AnsiString(Temperature.fMiddBase);
        EL<TCheckBox>("TACTForm", "cbBase02")->Checked = true;
        EL<TCheckBox>("TACTForm", "cbBase02")->Enabled = true;
    }

    if(ACTData.iThermoCtrlType==Type_DeltaDTB4824)  //Steven 20210510 : 自動K溫使用DTB4824
    {
    ;   // golden AutoTemperature.cpp:497-497 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:497-497 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
        for(int i=0; i<(int)myATPal.size(); i++)
#endif // GATE (S12-C save)
    ;   // golden AutoTemperature.cpp:498-498 取代：量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:498-498 -- 量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別
            myATPal[i]->cbChannel->Visible=false;
#endif // GATE (S12-C save)
    }

    (void)iGroup;   // golden AutoTemperature.cpp:501-514 取代：量測點面板 myATPal 的原始／目前偏移圖（ImageShowOriginalOffset／ImageShowCurrentOffset，讀 Temperature.fTempOffSet）—— GUI，移植樹沒有 TMyATPanel（迴圈拿掉後 :425 的 iGroup 只剩宣告 → (void)，同 forms/fConfiguration.h 的 (void)dCount 慣例）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:501-514 -- 量測點面板 myATPal 的原始／目前偏移圖（ImageShowOriginalOffset／ImageShowCurrentOffset，讀 Temperature.fTempOffSet）—— GUI，移植樹沒有 TMyATPanel（迴圈拿掉後 :425 的 iGroup 只剩宣告 → (void)，同 forms/fConfiguration.h 的 (void)dCount 慣例）
    for(int i=0; i<(int)myATPal.size(); i++)
    {
        if(ACTData.iThermoCtrlType==Type_DeltaDTB4824)  //Steven 20210510 : 自動K溫使用DTB4824
            iGroup=myATPal[i]->_Group;
        else
            iGroup=i;

        myATPal[i]->ImageShowOriginalOffset(Temperature.fTempOffSet[LowBase][iGroup], Temperature.fTempOffSet[MidBase][iGroup], Temperature.fTempOffSet[HigBase][iGroup]);

        myATPal[i]->ImageShowCurrentOffset(LowBase, Temperature.fTempOffSet[LowBase][iGroup]);
        myATPal[i]->ImageShowCurrentOffset(MidBase, Temperature.fTempOffSet[MidBase][iGroup]);
        myATPal[i]->ImageShowCurrentOffset(HigBase, Temperature.fTempOffSet[HigBase][iGroup]);
        myATPal[i]->gbMeasurePoint->Visible=true;
    }
#endif // GATE (S12-C save)

    EL<TPanel>("TACTForm", "pnlBaseTemp03")->Caption=AnsiString(Temperature.fHighBase);
    EL<TCheckBox>("TACTForm", "cbBase03")->Checked=true;

    bBaseFinish[0]=false;
    bBaseFinish[1]=false;
    bBaseFinish[2]=false;

    ;   // UI-only（HTML 端處理）：_pnlBaseTemp[0]->Color=clBtnFace;
    ;   // UI-only（HTML 端處理）：_pnlBaseTemp[1]->Color=clBtnFace;
    ;   // UI-only（HTML 端處理）：_pnlBaseTemp[2]->Color=clBtnFace;

    ;   // UI-only（HTML 端處理）：EL<TPageControl>("TACTForm", "pcACT")->ActivePage=EL<TTabSheet>("TACTForm", "tsTemperature");
    ;   // golden AutoTemperature.cpp:528-528 取代：UpdateTemperatureData（:809）：量測點面板的溫度顯示 —— GUI
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:528-528 -- UpdateTemperatureData（:809）：量測點面板的溫度顯示 —— GUI
    UpdateTemperatureData();
#endif // GATE (S12-C save)
    iAddr=0;
    iReceiveData=0;
    filerw::ELTodo("golden AutoTemperature.cpp:531 TimerACT->Enabled=true: TimerACTTimer (auto temperature calibration run) not ported (Jimmy)");   // golden AutoTemperature.cpp:531-531 取代：TimerACT：自動 K 溫的量測計時器（TimerACTTimer :887：讀溫度計、算偏移、寫 Temperature.fTempOffSet）—— 機台動作／通訊，交 Jimmy
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:531-531 -- TimerACT：自動 K 溫的量測計時器（TimerACTTimer :887：讀溫度計、算偏移、寫 Temperature.fTempOffSet）—— 機台動作／通訊，交 Jimmy
    EL<TControl>("TACTForm", "TimerACT")->Enabled=true;
#endif // GATE (S12-C save)

}

// golden AutoTemperature.cpp:692  TACTForm::btnUpdateClick(TObject *Sender)
static void ACT_btnUpdateClick()
{
    filerw::ELMark("btnUpdateClick");   // FileRW：存檔流程 trace（ack.trace）

    // ACT Setting -------------------------------------->>
    ACTData.iThermoCtrlType     = EL<TComboBox>("TACTForm", "cbbThermoCtrlType")->ItemIndex;
    ACTData.iCheckIntervalTime  = atoi(EL<TEdit>("TACTForm", "edtCheckIntervalTime")->Text.c_str());
    ACTData.iCalibrationRange   = atoi(EL<TEdit>("TACTForm", "edtCalibrationRange")->Text.c_str() );
    ACTData.dCalibrationRange[0]= atoi(EL<TEdit>("TACTForm", "edtRange1")->Text.c_str() );   //Steven 20210520 : K溫範圍改成三組
    ACTData.dCalibrationRange[1]= atoi(EL<TEdit>("TACTForm", "edtRange2")->Text.c_str() );
    ACTData.dCalibrationRange[2]= atoi(EL<TEdit>("TACTForm", "edtRange3")->Text.c_str() );
    ACTData.iReadScanTime       = atoi(EL<TEdit>("TACTForm", "edtReadScanTime")->Text.c_str()     );
    ACTData.iOffsetLimit        = atoi(EL<TEdit>("TACTForm", "edtOffsetLimit")->Text.c_str()      );
    ACTData.iSingleOffset       = atoi(EL<TEdit>("TACTForm", "edSingleOffset")->Text.c_str()      );
    ACTData.iDisplayColumn      = atoi(EL<TEdit>("TACTForm", "edColumns")->Text.c_str()           );
    ACTData.iOffsetMethod       = EL<TRadioGroup>("TACTForm", "rgOffsetMethod")->ItemIndex;
    ACT_SaveACTData( ACTData );
    // ACT Setting --------------------------------------<<

    // Comm Port ---------------------------------------->>
    CommName         = EL<TComboBox>("TACTForm", "cbDevice")->Text;
    ACT_Com->CommName="\\\\.\\"+CommName;   // golden AutoTemperature.cpp:711-711 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:711-711 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    EL<TControl>("TACTForm", "ACTCom")->CommName="\\\\.\\"+CommName;
#endif // GATE (S12-C save)
    ACT_Com->BaudRate = atoi(EL<TComboBox>("TACTForm", "cbBaudRate")->Text.c_str());   // golden AutoTemperature.cpp:712-712 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:712-712 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    EL<TControl>("TACTForm", "ACTCom")->BaudRate = atoi(EL<TComboBox>("TACTForm", "cbBaudRate")->Text.c_str());
#endif // GATE (S12-C save)

    if(EL<TComboBox>("TACTForm", "cbByteSize")->ItemIndex==0)
        ACT_Com->ByteSize=_5;   // golden AutoTemperature.cpp:715-715 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:715-715 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->ByteSize=_5;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbByteSize")->ItemIndex==1)
        ACT_Com->ByteSize=_6;   // golden AutoTemperature.cpp:717-717 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:717-717 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->ByteSize=_6;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbByteSize")->ItemIndex==2)
        ACT_Com->ByteSize=_7;   // golden AutoTemperature.cpp:719-719 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:719-719 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->ByteSize=_7;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbByteSize")->ItemIndex==3)
        ACT_Com->ByteSize=_8;   // golden AutoTemperature.cpp:721-721 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:721-721 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->ByteSize=_8;
#endif // GATE (S12-C save)

    if(EL<TComboBox>("TACTForm", "cbStopBit")->ItemIndex==0)
        ACT_Com->StopBits=_1;   // golden AutoTemperature.cpp:724-724 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:724-724 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->StopBits=_1;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbStopBit")->ItemIndex==1)
        ACT_Com->StopBits=_1_5;   // golden AutoTemperature.cpp:726-726 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:726-726 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->StopBits=_1_5;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbStopBit")->ItemIndex==2)
        ACT_Com->StopBits=_2;   // golden AutoTemperature.cpp:728-728 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:728-728 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->StopBits=_2;
#endif // GATE (S12-C save)

    if(EL<TComboBox>("TACTForm", "cbParity")->ItemIndex==0)
        ACT_Com->Parity=None;   // golden AutoTemperature.cpp:731-731 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:731-731 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->Parity=None;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbParity")->ItemIndex==1)
        ACT_Com->Parity=Odd;   // golden AutoTemperature.cpp:733-733 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:733-733 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->Parity=Odd;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbParity")->ItemIndex==2)
        ACT_Com->Parity=Even;   // golden AutoTemperature.cpp:735-735 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:735-735 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->Parity=Even;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbParity")->ItemIndex==4)
        ACT_Com->Parity=Space;   // golden AutoTemperature.cpp:737-737 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:737-737 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->Parity=Space;
#endif // GATE (S12-C save)
    else if(EL<TComboBox>("TACTForm", "cbParity")->ItemIndex==3)
        ACT_Com->Parity=TParity(3);   // golden AutoTemperature.cpp:739-739 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:739-739 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
        EL<TControl>("TACTForm", "ACTCom")->Parity=TParity(3);
#endif // GATE (S12-C save)

    ACT_SaveCommData(ACT_Com);   // golden AutoTemperature.cpp:741-741 取代：ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
#if 0 // GATE (S12-C save) golden AutoTemperature.cpp:741-741 -- ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）
    ACT_SaveCommData(EL<TControl>("TACTForm", "ACTCom"));
#endif // GATE (S12-C save)
    // Comm Port ----------------------------------------<<

}

// golden AutoTemperature.cpp:834  TACTForm::SaveACTData(ACT_DATA& Data)
static void ACT_SaveACTData(ACT_DATA& Data)
{
    filerw::ELMark("SaveACTData");   // FileRW：存檔流程 trace（ack.trace）

    TIniFile* IniFile1=new TIniFile(FilePath);
    Data.iThermoCtrlType    = CheckRange(Data.iThermoCtrlType   ,  1, 0);
    Data.iCheckIntervalTime = CheckRange(Data.iCheckIntervalTime, 30, 1);
    Data.iCalibrationRange  = CheckRange(Data.iCalibrationRange , 10, 1);
    ACTData.dCalibrationRange[0]= CheckRange(Data.dCalibrationRange[0] , 10.0, 0.1);   //Steven 20210520 : K溫範圍改成三組
    ACTData.dCalibrationRange[1]= CheckRange(Data.dCalibrationRange[1] , 10.0, 0.1);
    ACTData.dCalibrationRange[2]= CheckRange(Data.dCalibrationRange[2] , 10.0, 0.1);
    Data.iReadScanTime      = CheckRange(Data.iReadScanTime     , 10, 1);
    Data.iOffsetLimit       = CheckRange(Data.iOffsetLimit      , 30, 1);
    Data.iSingleOffset      = CheckRange(Data.iSingleOffset     , 30, 1);
    IniFile1->WriteInteger("ACT", "iThermoCtrlType"   , Data.iThermoCtrlType     );
    IniFile1->WriteInteger("ACT", "CheckIntervalTime" , Data.iCheckIntervalTime  );
    IniFile1->WriteInteger("ACT", "CalibrationRange"  , Data.iCalibrationRange   );
    IniFile1->WriteFloat("ACT", "dCalibrationRange[0]"  , Data.dCalibrationRange[0]   );    //Steven 20210520 : K溫範圍改成三組
    IniFile1->WriteFloat("ACT", "dCalibrationRange[1]"  , Data.dCalibrationRange[1]   );
    IniFile1->WriteFloat("ACT", "dCalibrationRange[2]"  , Data.dCalibrationRange[2]   );
    IniFile1->WriteInteger("ACT", "ReadScanTime"      , Data.iReadScanTime       );
    IniFile1->WriteInteger("ACT", "OffsetLimit"       , Data.iOffsetLimit        );
    IniFile1->WriteInteger("ACT", "SingleOffsetLimit" , Data.iSingleOffset       );
    IniFile1->WriteInteger("Display", "Colums"        , Data.iDisplayColumn      );
    IniFile1->WriteInteger("ACT", "OffsetMethod"      , Data.iOffsetMethod       );
    delete IniFile1;

}

// golden AutoTemperature.cpp:860  TACTForm::LoadACTData(ACT_DATA& Data)
static void ACT_LoadACTData(ACT_DATA& Data)
{

    TIniFile* IniFile1 = new TIniFile(FilePath);
    Data.iThermoCtrlType    = IniFile1->ReadInteger("ACT", "iThermoCtrlType"  , 1 );
    Data.iCheckIntervalTime = IniFile1->ReadInteger("ACT", "CheckIntervalTime", 3 );
    Data.iCalibrationRange  = IniFile1->ReadInteger("ACT", "CalibrationRange" , 2  );
    Data.dCalibrationRange[0]= IniFile1->ReadFloat("ACT", "dCalibrationRange[0]" , 2.0  );  //Steven 20210520 : K溫範圍改成三組
    Data.dCalibrationRange[1]= IniFile1->ReadFloat("ACT", "dCalibrationRange[1]" , 2.0  );
    Data.dCalibrationRange[2]= IniFile1->ReadFloat("ACT", "dCalibrationRange[2]" , 2.0  );
    Data.iReadScanTime      = IniFile1->ReadInteger("ACT", "ReadScanTime"     , 3  );
    Data.iOffsetLimit       = IniFile1->ReadInteger("ACT", "OffsetLimit"      , 30);
    Data.iSingleOffset      = IniFile1->ReadInteger("ACT", "SingleOffsetLimit", 5  );
    Data.iDisplayColumn     = IniFile1->ReadInteger("Display", "Colums"       , 1  );
    Data.iOffsetMethod      = IniFile1->ReadInteger("ACT", "OffsetMethod"     , 1  );
    Data.iThermoCtrlType    = CheckRange(Data.iThermoCtrlType   ,  1, 0);
    Data.iCheckIntervalTime = CheckRange(Data.iCheckIntervalTime, 30, 1);
    Data.iCalibrationRange  = CheckRange(Data.iCalibrationRange , 10, 1);
    Data.dCalibrationRange[0]  = CheckRange(Data.dCalibrationRange[0] , 10.0, 1.0);    //Steven 20210520 : K溫範圍改成三組
    Data.dCalibrationRange[1]  = CheckRange(Data.dCalibrationRange[1] , 10.0, 1.0);
    Data.dCalibrationRange[2]  = CheckRange(Data.dCalibrationRange[2] , 10.0, 1.0);
    Data.iReadScanTime      = CheckRange(Data.iReadScanTime     , 10, 1);
    Data.iOffsetLimit       = CheckRange(Data.iOffsetLimit      , 30, 1);
    Data.iSingleOffset      = CheckRange(Data.iSingleOffset     , 30, 1);

    delete IniFile1;

}

// golden AutoTemperature.cpp:1275  TACTForm::SaveCommData(const TComm* Comm)
static bool ACT_SaveCommData(const TComm* Comm)
{
    filerw::ELMark("SaveCommData");   // FileRW：存檔流程 trace（ack.trace）

    TIniFile* IniFile1=new TIniFile(FilePath);
    IniFile1->WriteString ("COMPort", "CommName", CommName       );
    IniFile1->WriteInteger("COMPort", "BaudRate", Comm->BaudRate);
    IniFile1->WriteInteger("COMPort", "ByteSize", Comm->ByteSize);
    IniFile1->WriteInteger("COMPort", "StopBits", Comm->StopBits);
    IniFile1->WriteInteger("COMPort", "Parity"  , Comm->Parity   );
    delete IniFile1;
    return true;

}

// golden AutoTemperature.cpp:1287  TACTForm::LoadCommData(TComm* Comm)
static bool ACT_LoadCommData(TComm* Comm)
{

    TIniFile* IniFile1=new TIniFile(FilePath);
    CommName       =           IniFile1->ReadString ("COMPort", "CommName", "COM3");
    Comm->CommName =           "\\\\.\\"+CommName;
    Comm->BaudRate =           IniFile1->ReadInteger("COMPort", "BaudRate", 9600  );
    Comm->ByteSize = TByteSize(IniFile1->ReadInteger("COMPort", "ByteSize", _8)   );
    Comm->StopBits = TStopBits(IniFile1->ReadInteger("COMPort", "StopBits", _1)   );
    Comm->Parity   = TParity  (IniFile1->ReadInteger("COMPort", "Parity"  , None));
    delete IniFile1;
    return true;

}

// golden AutoTemperature.cpp:1355  TACTForm::palSaveLogClick(TObject *Sender)
static void ACT_palSaveLogClick()
{

    AnsiString Str="D:\\HT9045_Log\\AutoTempCalibration\\";
    MyForceDirectories(OffsetPath);
    Str.sprintf("D:\\HT9045_Log\\AutoTempCalibration\\%04d-%02d-%02d %02d_%02d_%02d.txt", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec);
    EL<TMemo>("TACTForm", "MemoOffset")->Lines->SaveToFile(Str);

}

