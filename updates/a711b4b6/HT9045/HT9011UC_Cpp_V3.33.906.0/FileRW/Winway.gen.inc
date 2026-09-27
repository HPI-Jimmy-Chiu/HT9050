// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 Winway 的讀寫檔（HTEditList： → D:\HT9045\config\ATCWinWay.ini [COMPort1..4] CommName／BaudRate／ByteSize／StopBits／Parity／SetTemperature（開機：建構子每站 LoadCommData＋SaveCommData；開頁讀記憶體；Update 鈕寫該站））。
//  來源：golden D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ATC/WinWaySetting.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden ATC/WinWaySetting.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 12 個（header 宣告 25 個）。
//  索引：FileRW/README.md
// ---------------------------------------------------------------------------
#include "FileRW/_EditList.h"
#include "forms/fWinway.h"
#include "ATC/ATC_WinWay.h"
#include "vclcompat/IniFiles.h"
#include "vclcompat/SysUtils.h"
#include "cmydef.h"

#define ATCWinWayPath    "D:\\HT9045\\config\\ATCWinWay.ini"   // golden ATC/WinWaySetting.h:15（寫死字面值，--dry 蓋不到）
// golden SaveCommData :145 IniFile->WriteFloat：vclcompat::TIniFile 沒有 WriteFloat；BCB6 TCustomIniFile.WriteFloat 的本體就是
// WriteString(Section, Name, FloatToStr(Value))（同 ProductionInfo/uPAT_Function.cpp:621 的 G17 巨集）。放在所有 #include 之後。
#define WriteFloat(_sec_,_key_,_val_) WriteString(_sec_,_key_,FloatToStr(_val_))

// golden ATC/WinWaySetting.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
#define arrATC_Site (fWinway->arrATC_Site)             // golden WinWaySetting.h:52 —— 門面 forms/fWinway.h 那一份（bthermo.cpp／fLotInfo.cpp 的 golden 讀者用同一個）
#define iWinwayATCIndex (fWinway->iWinwayATCIndex)     // golden WinWaySetting.h:51 —— 門面的那一個
static bool bShow=false;                                      // golden WinWaySetting.h:53 —— 本 TU 自己的（網頁沒有關頁事件；同 GroundMan 的 fShow）
static TComm* WW_Comm[SiteNum]={new TComm(nullptr), new TComm(nullptr), new TComm(nullptr), new TComm(nullptr)};   // golden WinWaySetting.h:44-47 WinWayATCComm1..4（DFM 的 TComm 元件；vclcompat TComm 建構不碰硬體）

using filerw::EL;

// 前置宣告
static void WW_TfWinway();
static void WW_FormShow();
static void WW_ShowCommData(int iIndex);
static void WW_btnUpdateClick();
static bool WW_LoadCommData(TComm* Comm, int iIndex);
static bool WW_SaveCommData(TComm* Comm, int iIndex);
static void WW_cbbWinwayATCIndexChange();

// golden ATC/WinWaySetting.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void WW_DfmItems()
{
    { TStringList* s = EL<TComboBox>("TfWinway", "cbbBaudRate")->Items;
      s->Add("4800");
      s->Add("7200");
      s->Add("9600");
      s->Add("14400");
      s->Add("19200");
      s->Add("57600");
      s->Add("115200");
    }
    { TStringList* s = EL<TComboBox>("TfWinway", "cbbByteSize")->Items;
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("TfWinway", "cbbDevice")->Items;
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
    }
    { TStringList* s = EL<TComboBox>("TfWinway", "cbbParity")->Items;
      s->Add("None");
      s->Add("Odd");
      s->Add("Even");
      s->Add("Mark");
      s->Add("Space");
    }
    { TStringList* s = EL<TComboBox>("TfWinway", "cbbStopBit")->Items;
      s->Add("1");
      s->Add("1.5");
      s->Add("2");
    }
    { TStringList* s = EL<TComboBox>("TfWinway", "cbbWinwayATCIndex")->Items;
      s->Add("ATC 1");
      s->Add("ATC 2");
      s->Add("ATC 3");
      s->Add("ATC 4");
    }
}

// golden ATC/WinWaySetting.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kWW_ParentOf[][2] = {
    {"cbbBaudRate", "grpcps"},
    {"cbbByteSize", "grpcps"},
    {"cbbDevice", "grpcps"},
    {"cbbParity", "grpcps"},
    {"cbbStopBit", "grpcps"},
    {"edtSetTemp", "grpcommu"},
    {"lblShowPT", "grpcommu"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void WW_CreateContainerProxies()
{
    EL<TGroupBox>("TfWinway", "grpcommu");
    EL<TGroupBox>("TfWinway", "grpcps");
    filerw::ELSetParents("TfWinway", kWW_ParentOf, (int)(sizeof(kWW_ParentOf) / sizeof(kWW_ParentOf[0])));
}

// golden ATC/WinWaySetting.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void WW_DfmState()
{
    EL<TComboBox>("TfWinway", "cbbBaudRate")->Text = "cbbBaudRate";
    EL<TComboBox>("TfWinway", "cbbByteSize")->Text = "cbbByteSize";
    EL<TComboBox>("TfWinway", "cbbDevice")->Text = "cbCOM";
    EL<TComboBox>("TfWinway", "cbbParity")->Text = "cbbParity";
    EL<TComboBox>("TfWinway", "cbbStopBit")->Text = "cbbStopBit";
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kWW_EnableAll[] = {
    "grpcommu", "grpcps", "lblShowPT",
};

// 存檔流程（btnUpdateClick、SaveCommData）讀的替身 5 個 —— FileRW/Winway.cpp 用它決定頁面必須送哪些值
static const char* const kWW_SaveReads[] = {
    "cbbBaudRate", "cbbByteSize", "cbbDevice", "cbbParity", "cbbStopBit",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void WW_CreateSaveProxies()
{
    EL<TComboBox>("TfWinway", "cbbBaudRate");
    EL<TComboBox>("TfWinway", "cbbByteSize");
    EL<TComboBox>("TfWinway", "cbbDevice");
    EL<TComboBox>("TfWinway", "cbbParity");
    EL<TComboBox>("TfWinway", "cbbStopBit");
}

// golden ATC/WinWaySetting.cpp:12  TfWinway::TfWinway(TComponent* Owner)
static void WW_TfWinway()
{

    EL<TEdit>("TfWinway", "edtSetTemp")->Text="0";
    ;   // golden ATC/WinWaySetting.cpp:16-17 取代：Width／Height：視窗大小（HTML 不用；門面 forms/fWinway.cpp:74-75 已存值）
#if 0 // GATE (S12-C save) golden ATC/WinWaySetting.cpp:16-17 -- Width／Height：視窗大小（HTML 不用；門面 forms/fWinway.cpp:74-75 已存值）
    Width =330;
    Height=518;
#endif // GATE (S12-C save)
    iWinwayATCIndex=0;

    bShow=false;
    //
    arrATC_Site[0]=new ATC_WinWay(WW_Comm[0]); arrATC_Site[1]=new ATC_WinWay(WW_Comm[1]); arrATC_Site[2]=new ATC_WinWay(WW_Comm[2]); arrATC_Site[3]=new ATC_WinWay(WW_Comm[3]);   // golden ATC/WinWaySetting.cpp:22-25 取代：WinWayATCComm1..4：golden DFM 的 TComm 元件；替身是 TControl、不能當 TComm* 傳 → 本 TU 的 WW_Comm[0..3]（其餘照 golden）
#if 0 // GATE (S12-C save) golden ATC/WinWaySetting.cpp:22-25 -- WinWayATCComm1..4：golden DFM 的 TComm 元件；替身是 TControl、不能當 TComm* 傳 → 本 TU 的 WW_Comm[0..3]（其餘照 golden）
    arrATC_Site[0]=new ATC_WinWay(EL<TControl>("TfWinway", "WinWayATCComm1"));
    arrATC_Site[1]=new ATC_WinWay(EL<TControl>("TfWinway", "WinWayATCComm2"));
    arrATC_Site[2]=new ATC_WinWay(EL<TControl>("TfWinway", "WinWayATCComm3"));
    arrATC_Site[3]=new ATC_WinWay(EL<TControl>("TfWinway", "WinWayATCComm4"));
#endif // GATE (S12-C save)
    //
    for(int i=0; i<SiteNum; i++)
    {
        WW_LoadCommData(arrATC_Site[i]->WinwayCOM , i);
        WW_SaveCommData(arrATC_Site[i]->WinwayCOM , i);
    }

}

// golden ATC/WinWaySetting.cpp:34  TfWinway::FormShow(TObject *Sender)
static void WW_FormShow()
{

    ;   // golden ATC/WinWaySetting.cpp:36-37 取代：Left／Top：視窗位置（HTML 不用）
#if 0 // GATE (S12-C save) golden ATC/WinWaySetting.cpp:36-37 -- Left／Top：視窗位置（HTML 不用）
    Left=(1024-Width)/2;
    Top =(768-Height)/2;
#endif // GATE (S12-C save)
    filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbWinwayATCIndex"), iWinwayATCIndex);
    WW_ShowCommData(iWinwayATCIndex);
    bShow=true;

}

// golden ATC/WinWaySetting.cpp:43  TfWinway::ShowCommData(int iIndex)
static void WW_ShowCommData(int iIndex)
{

    ATC_WinWay* tempATC_WinWay=arrATC_Site[iIndex];
    filerw::ELComboText(EL<TComboBox>("TfWinway", "cbbDevice"), tempATC_WinWay->sWinwayCommName);
    filerw::ELComboText(EL<TComboBox>("TfWinway", "cbbBaudRate"), AnsiString(tempATC_WinWay->WinwayCOM->BaudRate));

    if(tempATC_WinWay->WinwayCOM->ByteSize==_5)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbByteSize"), 0);
    else if(tempATC_WinWay->WinwayCOM->ByteSize==_6)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbByteSize"), 1);
    else if(tempATC_WinWay->WinwayCOM->ByteSize==_7)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbByteSize"), 2);
    else if(tempATC_WinWay->WinwayCOM->ByteSize==_8)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbByteSize"), 3);

    if(tempATC_WinWay->WinwayCOM->StopBits==_1)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbStopBit"), 0);
    else if(tempATC_WinWay->WinwayCOM->StopBits==_1_5)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbStopBit"), 1);
    else if(tempATC_WinWay->WinwayCOM->StopBits ==_2)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbStopBit"), 2);

    if(tempATC_WinWay->WinwayCOM->Parity==None)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbParity"), 0);
    else if(tempATC_WinWay->WinwayCOM->Parity==Odd)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbParity"), 1);
    else if(tempATC_WinWay->WinwayCOM->Parity==Even)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbParity"), 2);
    else if(tempATC_WinWay->WinwayCOM->Parity==Space)
        filerw::ELComboIndex(EL<TComboBox>("TfWinway", "cbbParity"), 4);
    //
    filerw::ELTodo("golden ATC/WinWaySetting.cpp:74 lblShowPT=GetPT(): ATC_WinWay::GetPT sends Modbus 01 03 when connected -- communication half not ported (Jimmy)");   // golden ATC/WinWaySetting.cpp:74-74 取代：GetPT()：連線時送 Modbus 01 03 讀溫度（ATC/ATC_WinWay.cpp GetPT），沒連線回 0 —— 通訊那一半，交 Jimmy
#if 0 // GATE (S12-C save) golden ATC/WinWaySetting.cpp:74-74 -- GetPT()：連線時送 Modbus 01 03 讀溫度（ATC/ATC_WinWay.cpp GetPT），沒連線回 0 —— 通訊那一半，交 Jimmy
    EL<TLabel>("TfWinway", "lblShowPT")->Caption=FloatToStr(tempATC_WinWay->GetPT());
#endif // GATE (S12-C save)
    EL<TEdit>("TfWinway", "edtSetTemp")->Text=FloatToStr(tempATC_WinWay->GetST());

}

// golden ATC/WinWaySetting.cpp:78  TfWinway::btnUpdateClick(TObject *Sender)
static void WW_btnUpdateClick()
{
    filerw::ELMark("btnUpdateClick");   // FileRW：存檔流程 trace（ack.trace）

    ATC_WinWay* tempATC_WinWay=arrATC_Site[iWinwayATCIndex];
    tempATC_WinWay->sWinwayCommName=EL<TComboBox>("TfWinway", "cbbDevice")->Text;
    tempATC_WinWay->WinwayCOM->CommName="\\\\.\\"+tempATC_WinWay->sWinwayCommName;
    tempATC_WinWay->WinwayCOM->BaudRate=atoi(EL<TComboBox>("TfWinway", "cbbBaudRate")->Text.c_str());

    if(EL<TComboBox>("TfWinway", "cbbByteSize")->ItemIndex==0)
        tempATC_WinWay->WinwayCOM->ByteSize=_5;
    else if(EL<TComboBox>("TfWinway", "cbbByteSize")->ItemIndex==1)
        tempATC_WinWay->WinwayCOM->ByteSize=_6;
    else if(EL<TComboBox>("TfWinway", "cbbByteSize")->ItemIndex==2)
        tempATC_WinWay->WinwayCOM->ByteSize=_7;
    else if(EL<TComboBox>("TfWinway", "cbbByteSize")->ItemIndex==3)
        tempATC_WinWay->WinwayCOM->ByteSize=_8;

    if(EL<TComboBox>("TfWinway", "cbbStopBit")->ItemIndex==0)
        tempATC_WinWay->WinwayCOM->StopBits=_1;
    else if(EL<TComboBox>("TfWinway", "cbbStopBit")->ItemIndex==1)
        tempATC_WinWay->WinwayCOM->StopBits=_1_5;
    else if(EL<TComboBox>("TfWinway", "cbbStopBit")->ItemIndex==2)
        tempATC_WinWay->WinwayCOM->StopBits=_2;

    if(EL<TComboBox>("TfWinway", "cbbParity")->ItemIndex==0)
        tempATC_WinWay->WinwayCOM->Parity=None;
    else if(EL<TComboBox>("TfWinway", "cbbParity")->ItemIndex==1)
        tempATC_WinWay->WinwayCOM->Parity=Odd;
    else if(EL<TComboBox>("TfWinway", "cbbParity")->ItemIndex==2)
        tempATC_WinWay->WinwayCOM->Parity=Even;
    else if(EL<TComboBox>("TfWinway", "cbbParity")->ItemIndex==4)
        tempATC_WinWay->WinwayCOM->Parity=Space;

    WW_SaveCommData(tempATC_WinWay->WinwayCOM, iWinwayATCIndex);
    filerw::ELTodo("golden ATC/WinWaySetting.cpp:111-112 CloseCommPort()/OpenCommPort(): reopen the WinWay COM port with the new settings -- communication half not ported (Jimmy)");   // golden ATC/WinWaySetting.cpp:111-112 取代：CloseCommPort／OpenCommPort：用新設定重開這一站的 COM（ATC_WinWay::OpenCommPort → CreateFile＋StartComm）—— 通訊那一半，交 Jimmy；檔已在上一行 SaveCommData 寫好
#if 0 // GATE (S12-C save) golden ATC/WinWaySetting.cpp:111-112 -- CloseCommPort／OpenCommPort：用新設定重開這一站的 COM（ATC_WinWay::OpenCommPort → CreateFile＋StartComm）—— 通訊那一半，交 Jimmy；檔已在上一行 SaveCommData 寫好
    tempATC_WinWay->CloseCommPort();
    tempATC_WinWay->OpenCommPort();
#endif // GATE (S12-C save)
    filerw::ELMessage("【Update】 Success");   // golden ATC/WinWaySetting.cpp:113-113 取代：ShowMessage（VCL 訊息框）→ filerw::ELMessage（進 ack.session.messages）
#if 0 // GATE (S12-C save) golden ATC/WinWaySetting.cpp:113-113 -- ShowMessage（VCL 訊息框）→ filerw::ELMessage（進 ack.session.messages）
    ShowMessage("【Update】 Success");
#endif // GATE (S12-C save)

}

// golden ATC/WinWaySetting.cpp:116  TfWinway::LoadCommData(TComm* Comm, int iIndex)
static bool WW_LoadCommData(TComm* Comm, int iIndex)
{

    ATC_WinWay* tempATC_WinWay=arrATC_Site[iIndex];
    TIniFile* IniFile=new TIniFile(ATCWinWayPath);

    AnsiString sSection="COMPort"+AnsiString(iIndex+1);
    AnsiString sSectionComport="COM"+AnsiString(iIndex+15);
    tempATC_WinWay->sWinwayCommName=IniFile->ReadString(sSection, "CommName", sSectionComport);
    Comm->CommName="\\\\.\\" + tempATC_WinWay->sWinwayCommName;
    Comm->BaudRate=(unsigned) IniFile->ReadInteger(sSection, "BaudRate", 9600);
    Comm->ByteSize=(TByteSize)IniFile->ReadInteger(sSection, "ByteSize", _8);
    Comm->StopBits=(TStopBits)IniFile->ReadInteger(sSection, "StopBits", _1);
    Comm->Parity  =(TParity)  IniFile->ReadInteger(sSection, "Parity"  , None);
    //set temprature
    tempATC_WinWay->SetST(IniFile->ReadFloat(sSection, "SetTemperature", 0.0));
    delete IniFile;
    return true;

}

// golden ATC/WinWaySetting.cpp:135  TfWinway::SaveCommData(TComm* Comm, int iIndex)
static bool WW_SaveCommData(TComm* Comm, int iIndex)
{
    filerw::ELMark("SaveCommData");   // FileRW：存檔流程 trace（ack.trace）

    ATC_WinWay* tempATC_WinWay =arrATC_Site[iIndex];
    TIniFile* IniFile   =new TIniFile(ATCWinWayPath);
    AnsiString sSection ="COMPort"+AnsiString(iIndex+1);
    IniFile->WriteString( sSection, "CommName"    , tempATC_WinWay->sWinwayCommName);
    IniFile->WriteInteger(sSection, "BaudRate"    , Comm->BaudRate);
    IniFile->WriteInteger(sSection, "ByteSize"    , Comm->ByteSize);
    IniFile->WriteInteger(sSection, "StopBits"    , Comm->StopBits);
    IniFile->WriteInteger(sSection, "Parity"      , Comm->Parity);
    IniFile->WriteFloat  (sSection, "SetTemperature" , tempATC_WinWay->GetST());
    delete IniFile;
    return true;

}

// golden ATC/WinWaySetting.cpp:150  TfWinway::cbbWinwayATCIndexChange(TObject *Sender)
static void WW_cbbWinwayATCIndexChange()
{

    iWinwayATCIndex=EL<TComboBox>("TfWinway", "cbbWinwayATCIndex")->ItemIndex;
    WW_ShowCommData(iWinwayATCIndex);

}

