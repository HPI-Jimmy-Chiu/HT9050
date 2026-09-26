// 產生檔 -- tools/gen_editlist.py（Steven 20260924，S12 C 類）。不要手改：改產生器後重跑。
// ---------------------------------------------------------------------------
//  結構 HSys 的讀寫檔（HTEditList： → system\Gerneral.ini、D:\RS232Standard\System\Setup.ini、D:\GPIB9045\system\general.ini）。
//  來源：golden D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\HandlerSys.cpp（cp950 → UTF-8）；widget 名稱與型別取自 golden HandlerSys.h。
//  元件改成具名替身 EL<T>("名稱")（FileRW/_EditList.h），名稱＝HTML 元件 id。
//  golden 程式用到的替身 276 個（header 宣告 426 個）。
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
#include "database.h"
#include "mysensor.h"
#include "Motor/vendor/mn200.h"


// golden HandlerSys.h 的非元件成員（本 TU 只有這一個表單，所以是 static）
static TStringList *slCustomerCode=nullptr;   // golden HandlerSys.h:513（建構子 :66 new；只給客戶碼搜尋 edtSearchCodeChange 用）

using filerw::EL;

// 前置宣告
static void HS_THandlerSystem();
static void HS_FormShow();
static void HS_LoaderSystemSet();
static void HS_LoaderSafeDoorSet();
static void HS_rgTTLCardClick();
static void HS_SaveSystemSet();
static void HS_SaveBtnClick();
static void HS_SaveSafeDoorSet();
static void HS_ExitBtnClick();

// golden HandlerSys.dfm 的設計期 Items.Strings（FileRW_IniConfig_Boot 在註冊之前呼叫）
static void HS_DfmItems()
{
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "ArmZAtoZH")->Items;
      s->Add("Cylinder");
      s->Add("Motor");
      s->Add("Motor + Cylinder");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "Auto2SelectCy")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "ElectronPressure")->Items;
      s->Add("Un Install");
      s->Add("Install (Analog)");
      s->Add("Install (Digital)");
      s->Add("Install (Digital and Encoding)");
      s->Add("PISO ET-7226");
      s->Add("Two EP");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "MachineTrack")->Items;
      s->Add("4 Track (E/C Manual)");
      s->Add("6 Track (E/C Automatic)");
      s->Add("7 Track (Empty Unloader)");
      s->Add("8 Track (5 Unloader)");
      s->Add("9 Track (6 Unloader)");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cb2DReader1")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cb2DReader2")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cb2DReader3")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cb2DReader4")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbAirCon")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbColorSenAmpCnt")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
      s->Add("5");
      s->Add("6");
      s->Add("7");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComBinDisp")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComDyTemp")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComIndex")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComLaser1")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComLaser2")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComLaserInArm")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComRTC")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComTTLRS232")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComTTLRS232_2")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComTemp")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComTempOmron")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbComTester")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbEMAwarePort1")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbEMAwarePort2")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbEMAwarePort3")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbEMAwarePort4")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbHandlerModel")->Items;
      s->Add("HT9045");
      s->Add("HT9046");
      s->Add("HT9045_12Site");
      s->Add("HT9046LS");
      s->Add("HT502");
      s->Add("HT1032");
      s->Add("HT7080");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbIndEPCnt")->Items;
      s->Add("16");
      s->Add("4");
      s->Add("8");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbKASUGA_Fan")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbNovx3360")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbOCR")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbOCRwithTester")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbRotateSenAmpCnt")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt2nd")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt3rd")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbVibrationCardQty")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbATC1")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbATC2")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbATC3")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbATC4")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbComBinDisp2")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbGroundMan")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbRFIDReader")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "cbbTrayStepMotor")->Items;
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
      s->Add("COM20");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "coCanBusNudn1")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
      s->Add("3");
      s->Add("4");
    }
    { TStringList* s = EL<TComboBox>("THandlerSystem", "coNudn1Macid11")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "coNudn1Macid12")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "coNudn1Macid13")->Items;
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
    { TStringList* s = EL<TComboBox>("THandlerSystem", "coNudn1Macid14")->Items;
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
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rg2DBarcode")->Items;
      s->Add("Un Install");
      s->Add("Out Shuttle");
      s->Add("In Shuttle (RS232)");
      s->Add("USE CCD");
      s->Add("In Shuttle (Ethernet)");
      s->Add("USE OCR");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rg2ndLoader")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rg3M_EM_AWARE_Monitor")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAOI")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgATC")->Items;
      s->Add("Un Install");
      s->Add("ATC 6.0");
      s->Add("ATC 3.0 (Air Cooling)");
      s->Add("Silicon Type");
      s->Add("HonPrec Type");
      s->Add("No Chamber");
      s->Add("NewATCSystem");
      s->Add("WinWay");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgATCHeatGun")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgATCMixMode")->Items;
      s->Add("None");
      s->Add("ATC3.3+6.0");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAirConditioner")->Items;
      s->Add("Un Install");
      s->Add("Install");
      s->Add("Install with RS232");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAirStreamSelect")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto1ART")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto2ART")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto3ART")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto3Magazine")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto4ART")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto5ART")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAuto6ART")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAutoAlignment")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAutoCleanIonFan")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAutoFormSize")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAutoShuttleSensor")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgAutoTrackCanGoRear")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgBaseHeaterCount")->Items;
      s->Add("Un Install");
      s->Add("6 EA");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgBottom2DID")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgBottom2DID_CCD")->Items;
      s->Add("4 CCD");
      s->Add("8 CCD");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCCDAutoAlignmentMode")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCCDTemp")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCKDFCM")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCOLORSENSOR_MUN")->Items;
      s->Add("Un Install");
      s->Add("Install in Loader");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCanBusMethod")->Items;
      s->Add("Default (old method)");
      s->Add("Manual (new method)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCatchTrayBlock")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCatchTrayModel")->Items;
      s->Add("4 Sensor");
      s->Add("2 Sensor");
      s->Add("2 Sensor(ART)");
      s->Add("4 Sensor(ART)");
      s->Add("2 Sensor(ART & No Cover)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgChamberUsePulseType")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCleanAir")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgColorSensor")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCoverTrayID")->Items;
      s->Add("Un Install");
      s->Add("Keyence");
      s->Add("NFC");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgCustomerList")->Items;
      s->Add("000 HonPrec              鴻勁");
      s->Add("731 Carsem_Thai          Carsem 泰國");
      s->Add("730 IFXTH                IFXTH泰國");
      s->Add("740 Ramos                Ramos Technology");
      s->Add("741 TechL_Vitenam        TechL-Vitenam");
      s->Add("742 TechL_Korea          TechL-Korea");
      s->Add("751 Mellanox_Israel      Mellanox 以色列");
      s->Add("743 SFA_Semicon          SFA_Semicon");
      s->Add("763 CC_UTAC_Indonesia    聯測科技 印尼");
      s->Add("764 Elmos Germany        Elmos德國");
      s->Add("765 Allegro Philippines  Allegro菲律賓");
      s->Add("766 Morningcore          宸芯 ");
      s->Add("767 Paceis               航芯源 ");
      s->Add("768 Sigmastar            銳力 ");
      s->Add("769 BRAVETEK             博發電子 ");
      s->Add("770 DJI_SZ               大疆創新 深圳");
      s->Add("771 JSCC_OS              長電微電子");
      s->Add("772 TYTAN                成都態坦");
      s->Add("773 CAPCON               北京華封");
      s->Add("780 NXP_TJ               恩智浦 天津");
      s->Add("781 XINYUN               杭州芯云");
      s->Add("783 SCX                  深測芯");
      s->Add("787 TAIJI_SEMI           太極半導體(蘇州)");
      s->Add("790 Forehope_NINGBO      甬矽 寧波");
      s->Add("791 SJ_Semiconductor     盛合晶微");
      s->Add("792 SJ_Semiconductor_OS  盛合晶微Open Short");
      s->Add("793 30JAVEE              成都三零嘉微電子");
      s->Add("794 CAMBRICON            上海寒武紀");
      s->Add("795 VATE                 立衛");
      s->Add("796 ASIAOPTICAL          亞洲光學");
      s->Add("797 HXYSEMI              浙江航源芯");
      s->Add("800 Broadcom US          博通 美國");
      s->Add("801 Tessolve_US          Tessolve-US");
      s->Add("802 Infineon             英飛凌");
      s->Add("803 TERADYNE_US          泰瑞達 美國");
      s->Add("804 TeraProbe            TeraProbe 日本");
      s->Add("805 TATA                 塔塔-印度");
      s->Add("806 STM                  意法半導體 馬爾他");
      s->Add("807 TFAMD_M              TFAMD 馬來西亞");
      s->Add("808 AMD_US               AMD 美國");
      s->Add("809 IMEC                 愛美科");
      s->Add("810 AVAGO_Korea          安華高 韓國");
      s->Add("812 PTI                  力成3C廠");
      s->Add("813 SFA_Semicon          SFA Semicon");
      s->Add("820 TSMC_TAINAN          台積電 台南 ");
      s->Add("825 TSMC_HSINCHU         台積電 新竹");
      s->Add("828 ChenYuanXiang_CHINA  晨元翔 西安");
      s->Add("829 EUROFINS             歐陸");
      s->Add("830 HTKJXA_CHINA         西安 天水華天");
      s->Add("831 RF360                RF360");
      s->Add("832 LBSH                 上海祿比");
      s->Add("833 Realtek              瑞昱半導體");
      s->Add("834 RIGGER_MICRO         鄭州銳傑微");
      s->Add("835 XINITECH             北京芯力");
      s->Add("836 GIGA_FORCE_Zhejiang  季豐 浙江 ");
      s->Add("837 GIGA_FORCE_Shanghai  季豐 上海");
      s->Add("838 CSAMQ                長沙安牧泉");
      s->Add("839 GONGJIN_SHANGHAI     上海共進");
      s->Add("840 SANDISK_CHINA        上海新帝");
      s->Add("841 GONGJIN_SUZHOU       蘇州共進");
      s->Add("842 Mathilda             馬舍科技");
      s->Add("843 DoosanTesna          DOOSAN TESNA");
      s->Add("844 Renesas Malaysia     Renesas 馬來西亞");
      s->Add("845 STK                  STK 日本");
      s->Add("846 Renesas              Renesas 日本");
      s->Add("847 THINE                Thine 日本");
      s->Add("848 SINOICTECH           上海華嶺");
      s->Add("849 ChipOn               上海芯旺");
      s->Add("850 ChipMos_TAINAN       南茂 台南");
      s->Add("851 ChipMos_ZHUBEI       南茂 竹北");
      s->Add("852 ITS                  創量科技");
      s->Add("853 NEXPERIA             安世 廣東");
      s->Add("854 Atec_Semiconductor   上海旻艾半導體");
      s->Add("855 JSSI_Semiconductor   江蘇芯德半導體");
      s->Add("856 Microchip_FR         Microchip 法國");
      s->Add("857 Indie_US             Indie 美國.");
      s->Add("858 CETC                 中國電子-第58研究所 ");
      s->Add("859 XDXCT                象帝先              ");
      s->Add("860 MAXIM_TAILAND        MAXIM 泰國");
      s->Add("861 Microchip_Thailand   Microchip 泰國");
      s->Add("862 Microchip_Philippines Microchip 菲律賓");
      s->Add("863 Microchip_China      Microchip 中國");
      s->Add("864 Microchip_US         Microchip 美國");
      s->Add("865 HANA Micron");
      s->Add("866 iTest_Inc            iTest,Inc. 美國");
      s->Add("867 EMemory              EMemory 力旺電子");
      s->Add("868 CYUEAN               CYUEAN 确安科技");
      s->Add("869 PANTHER              PANTHER 鴻谷科技");
      s->Add("870 ARDENTEC             欣詮");
      s->Add("871 FULCAP               詮容");
      s->Add("872 AOSL                 萬有半導體");
      s->Add("873 Nuvoton_Israel       新唐 以色列");
      s->Add("874 GT                   寰邦科技");
      s->Add("875 Novatek              聯詠科技");
      s->Add("876 Sunplus              凌陽科技");
      s->Add("877 Amlogic              晶晨半導體");
      s->Add("878 Higon                海光 成都");
      s->Add("879 Kingston             金士頓");
      s->Add("880 Spreadtrum           展訊 上海");
      s->Add("881 Amazon               Amazon");
      s->Add("882 Murata               Murata");
      s->Add("883 Goertek              歌爾微電子");
      s->Add("884 FMSH                 上海復旦微電子 ");
      s->Add("885 HDSC                 華大半導體");
      s->Add("886 SANECHIPS            中興微電子 深圳");
      s->Add("887 Habana Labs          Habana Labs");
      s->Add("889 CENTER               中芯國際");
      s->Add("890 UMC                  聯電");
      s->Add("891 ATEC                 艾科");
      s->Add("892 Winstek              台星科");
      s->Add("893 Sanan                三安");
      s->Add("894 UTAC_TW              聯測科技 台灣");
      s->Add("895 BARUN");
      s->Add("897 SILTERRA_CHINKIANG   矽佳 鎮江");
      s->Add("898 TFAMD_SUZHOU         TFAMD  蘇州");
      s->Add("899 YTEC                 久元");
      s->Add("900 SPIL                 矽品 中山");
      s->Add("901 ISE_USA              ISE");
      s->Add("902 HYGEIA_SUZHOU        海京 蘇州");
      s->Add("903 JSI_HAOXING          長電 紹興");
      s->Add("904 JINGJIAWEI_CHANGSHA  長沙景嘉微");
      s->Add("905 ISE_SH               上海月芯半導體科技");
      s->Add("906 INTEL_IL             INTEL 以色列");
      s->Add("907 IBM_CANADA           IBM 加拿大");
      s->Add("908 PGC                  巨有科");
      s->Add("909 ITESTSEMI            安測半導體    ");
      s->Add("910 SPIL SHINCHU         矽品 新竹");
      s->Add("911 SPIL TAICHUNG_LOGIC  矽品 台中");
      s->Add("912 SPIL CHINA_SUZHOU    矽品 蘇州");
      s->Add("913 HUAWEI               華為");
      s->Add("914 ANST                 蘇州安盛");
      s->Add("915 V-Test               無錫 偉測半導體");
      s->Add("916 TFME CHINA           通富微電");
      s->Add("918 BOJIAN               蘇州 博劍");
      s->Add("919 V-Test Shanghai      上海 偉測半導體");
      s->Add("920 KYEC CHEN            京元 記憶體");
      s->Add("921 KYEC LEE             京元 邏輯");
      s->Add("922 KYEC JCTHIU          京元");
      s->Add("923 DL_TEK               東琳.");
      s->Add("924 KYEC XILINX          京元 Xilinx");
      s->Add("925 KYEC STM             京元 STM ");
      s->Add("926 Advantest_GE         Advantest 德國");
      s->Add("927 LATTICESEMI          萊迪思半導體");
      s->Add("928 ITRI                 工業技術研究院 ");
      s->Add("929 ASE KaohSiung_K12    日月光 高雄K12");
      s->Add("930 ASE SG               日月光 新加坡");
      s->Add("931 ASE JP               日月光 日本");
      s->Add("932 ASE Korea            日月光 韓國");
      s->Add("933 ASE CL               日月光 中壢");
      s->Add("934 ASE SH               日月光 上海");
      s->Add("935 ASE N                日月光 NXP");
      s->Add("937 ASE Malaysia　　　　 日月光 馬來西亞");
      s->Add("938 ASE KaohSiung_K3　   日月光 高雄K3");
      s->Add("939 ASE KaohSiung_K11    日月光 高雄K11");
      s->Add("940 UTAC                 Open/Short");
      s->Add("941 SIGURD HUKOU         矽格 湖口");
      s->Add("942 RFMD BEIJING         RFMD 北京");
      s->Add("943 JSCC                 SC 江陰");
      s->Add("944 JSCS                 SC 新加坡");
      s->Add("945 SIGURD ChungXing     矽格 中興");
      s->Add("946 SIGURD PeiXing       矽格 北興");
      s->Add("947 JSCK                 SC 韓國");
      s->Add("948 RFMD USA             RFMD美國　　");
      s->Add("949 ASE KS               日月光 崑山");
      s->Add("950 APTOS                群豐");
      s->Add("951 WINBOND              華邦");
      s->Add("952 G-Link               G Link");
      s->Add("953 AMKOR                艾克爾");
      s->Add("954 GIGA                 GIGA");
      s->Add("955 LINGSEN              菱生");
      s->Add("956 Greatek              超豐");
      s->Add("957 PTI                  力成");
      s->Add("958 THAILIN              泰林");
      s->Add("959 JCET                 江蘇長電");
      s->Add("960 OSE                  華泰");
      s->Add("961 MTI                  聚成");
      s->Add("962 NUVOTON              新唐");
      s->Add("964 Eutrend              聿勤  ");
      s->Add("965 TICP                 高雄典範.  ");
      s->Add("966 THEIL                同欣電子");
      s->Add("967 TERAPOWER            晶兆成");
      s->Add("968 RICHTEK              立錡");
      s->Add("969 TSI                  誠遠科技");
      s->Add("970 GIGAS　　　　　　　　全智");
      s->Add("971 AMKOR Korea          安靠韓國.");
      s->Add("972 AMKOR China          安靠上海.");
      s->Add("973 AMKOR Japan          安靠日本.");
      s->Add("974 AMKOR Philippines    安靠菲律賓");
      s->Add("975 UTAC_TH              聯測科技 泰國");
      s->Add("976 ANALOG DEVICES PH    亞德諾半導體菲律賓");
      s->Add("977 ONSEMI_CA            On Semi CANADA");
      s->Add("978 INTEL_US             INTEL US");
      s->Add("979 INTEL_M              INTEL Malaysia");
      s->Add("980 CARSEM M             Carsem Malaysia");
      s->Add("981 UNISEM M             Unisem Malaysia");
      s->Add("982 AMD M                AMD Malaysia");
      s->Add("983 INARI M              INARI Malaysia ");
      s->Add("984 OnSemi M             On Semi Malaysia ");
      s->Add("985 MAXIM ");
      s->Add("986 MARVELL ");
      s->Add("987 ATMEL ");
      s->Add("988 USI                  環鴻科技");
      s->Add("989 DYNACARD             新東亞微電子");
      s->Add("990 CYPRESS ");
      s->Add("991 GERADTECH            智瑞達");
      s->Add("992 GM-Test ");
      s->Add("993 I-Tech ");
      s->Add("994 WinPac ");
      s->Add("995 Silicon_Labs SG      Silicon_Labs新加坡");
      s->Add("996 Silicon_Labs SZ      Silicon_Labs深圳");
      s->Add("997 Altera_USA           Altera 美國");
      s->Add("998 ETRENDTECH           逸昌科技");
      s->Add("999 Qualcomm             高通");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgDeviceFlipper")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgDewpointHW")->Items;
      s->Add("Un Install");
      s->Add("CAEL-DPA-136-2SA(-60~+60)");
      s->Add("Tekhne TK-80 TR(-80~+20)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgDieClean")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgDoubleBeltMode")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgDoubleEPControl")->Items;
      s->Add("Un Install");
      s->Add("Normal");
      s->Add("Indivial EP");
      s->Add("Multi EP (Max Qual Site)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgE84Sensor")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgESDTemp")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgEmptyKeyence")->Items;
      s->Add("Un Install");
      s->Add("Install(Auto 2)");
      s->Add("Install(Empty)");
      s->Add("Install(Color)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgFinePitch")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgFix3FullPlace")->Items;
      s->Add("Un Install");
      s->Add("Short Shuttle");
      s->Add("Use Cylinder");
      s->Add("Use Cylinder for 46LA");
      s->Add("Use Stepper Motor");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgFixAICCD")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgGroundMan")->Items;
      s->Add("Un Install");
      s->Add("Install with COM");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgGroundMan_ScanPoint")->Items;
      s->Add("8");
      s->Add("22");
      s->Add("28");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHTIonBar")->Items;
      s->Add("Un Install");
      s->Add("Loader Only");
      s->Add("UnloaderOnly (Ion 4/5/8)");
      s->Add("Full Area");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHeatGun")->Items;
      s->Add("Un Install");
      s->Add("Install");
      s->Add("Install with CDA Function");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHeater")->Items;
      s->Add("4 Heaters");
      s->Add("16 Heaters");
      s->Add("16 Heaters with Omron EJ1N");
      s->Add("32 Heaters with Omron EJ1N");
      s->Add("32 Heaters with KT4H");
      s->Add("16 Heaters with DTME08");
      s->Add("32 Heaters with DTME08");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->Items;
      s->Add("TC401  ");
      s->Add("Panasonic KT4H");
      s->Add("Omron E5DC");
      s->Add("No Heater");
      s->Add("DTK4848");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->Items;
      s->Add("130");
      s->Add("150");
      s->Add("155");
      s->Add("175");
      s->Add("200");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHotGunFlow")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHotPlateLimit")->Items;
      s->Add("Old");
      s->Add("New (For HT-9046LA)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHotPlatePos")->Items;
      s->Add("Left");
      s->Add("Right (New)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgHotplateType")->Items;
      s->Add("1.5 X ( HT9045W )");
      s->Add("2.0 X ( HT9045WS )");
      s->Add("1.0 X (HT9046LA)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIOCard")->Items;
      s->Add("SYN_TEK PCI-L122");
      s->Add("PISO MN200");
      s->Add("MN200 with New IO Table");
      s->Add("PISO P64 C64");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIOChangeToque")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInOutArmXPitch")->Items;
      s->Add("40 to 120 mm");
      s->Add("40 to 150 mm");
      s->Add("Manual Key In");
      s->Add("22 to 154 mm");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInOutArmYPitch")->Items;
      s->Add("Fixed Y Pitch 60 mm");
      s->Add("Manual Y Pitch 60 && 63.5 mm");
      s->Add("Auto Y Pitch In Bc / Out Bb");
      s->Add("Manual Y Pitch 60 && 36mm");
      s->Add("Auto Y Pitch in Row A");
      s->Add("Auto Y Pitch 16 Picker @ Bd");
      s->Add("Auto Y Pitch @ Bb (HT7080)");
      s->Add("Auto Y Pitch 16 @ In Bd / Out Be");
      s->Add("Auto Y Pitch In Bb / Out Bc");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInPlacement")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInShtAutoLatch")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInShtLastSensor")->Items;
      s->Add("45-->7 / 46-->9");
      s->Add("45-->9 / 46-->9");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIndexMotionCard")->Items;
      s->Add("Galil");
      s->Add("CSMC");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIndexMotorAxis")->Items;
      s->Add("Use 4 Axis");
      s->Add("Use 3 Axis");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIndexMotorType")->Items;
      s->Add("Panasonic A4");
      s->Add("Mitsubishi");
      s->Add("Panasonic A5");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIndexPressType")->Items;
      s->Add("Normal");
      s->Add("240KG");
      s->Add("120KG");
      s->Add("500KG");
      s->Add("260KG");
      s->Add("400KG");
      s->Add("360KG");
      s->Add("160KG");
      s->Add("640KG");
      s->Add("800KG");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIndexSuckerType")->Items;
      s->Add("Positive  (+)");
      s->Add("Negative (-)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInstallARTtwocylinder")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInstallAutoRestest")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgInstallFix3")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgIonFanType")->Items;
      s->Add("By Detection Board (5 I/O for all)");
      s->Add("By Alarm (1 I/O for one)");
      s->Add("None");
      s->Add("By Power and Alarm (2 I/O for one)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgKasuga")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgKasuga_Fan")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLBTemp")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLBTemp2")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLDCarRotAtm")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLaserDistance")->Items;
      s->Add("Un Install");
      s->Add("Install (Hot Plate + Shuttle)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLdUldCassetteMode")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLdUldUseStepMotor")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLoaderHinge")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLoaderKeyence")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgLoaderVibration")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMNetSpeed")->Items;
      s->Add("2.5M");
      s->Add("5M");
      s->Add("10M");
      s->Add("20M");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMRSystem")->Items;
      s->Add("Un Install");
      s->Add("Install MR");
      s->Add("Install LM");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMagBinDispType")->Items;
      s->Add("Uninstall");
      s->Add("HT-A18");
      s->Add("HT-BT007+HTBT008");
      s->Add("TFT");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMagneticScale")->Items;
      s->Add("Un Install");
      s->Add("Install (In / Out XY)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgModel")->Items;
      s->Add("Normal");
      s->Add("HT-9046LA");
      s->Add("HT-9016C");
      s->Add("HT-9046AU");
      s->Add("HT-9046CR");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMotionCard")->Items;
      s->Add("SYN-TEK Motion-Net");
      s->Add("CONTECT SMC-8DF-C01");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMultileEmpty")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgMultileEmptyTrayIDKeyence")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgNUECType")->Items;
      s->Add("NU-EC1");
      s->Add("NU-EC1A");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgNovx3360")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgNumberPanelType")->Items;
      s->Add("None");
      s->Add("1 Digital Type");
      s->Add("2 Digital Type");
      s->Add("2 Digital with Color");
      s->Add("TFT Type");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgOCR")->Items;
      s->Add("Un Install");
      s->Add("Install in Handler");
      s->Add("Install in OCR IPC");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgOCRYStepMot")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgOHTSystem")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgOTDInstall")->Items;
      s->Add("Type 0 (2 Sensor)");
      s->Add("Install Software control");
      s->Add("Install Hardware control");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgOutArmYPitch")->Items;
      s->Add("Fixed Y Pitch 60 mm");
      s->Add("Manual Y Pitch 60 && 63.5 mm");
      s->Add("Auto Y Pitch In Bc / Out Bb");
      s->Add("Manual Y Pitch 60 && 36mm");
      s->Add("Auto Y Pitch in Row A");
      s->Add("Auto Y Pitch 16 Picker @ Bd");
      s->Add("Auto Y Pitch @ Bb (HT7080)");
      s->Add("Auto Y Pitch 16 @ In Bd / Out Be");
      s->Add("Auto Y Pitch In Bb / Out Bc");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgOutSortArm")->Items;
      s->Add("Uninstall");
      s->Add("2 Picker");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgPickerCount")->Items;
      s->Add("4 Picker");
      s->Add("8 Picker");
      s->Add("2 Picker (A, G)");
      s->Add("16 Picker");
      s->Add("1 Picker");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgPreciser")->Items;
      s->Add("Uninstall");
      s->Add("Install with Motor");
      s->Add("Install without Motor");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgPreciserPos")->Items;
      s->Add("Normal");
      s->Add("Between Shuttle");
      s->Add("Hotplate");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRFIDReader")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRFIDSystem")->Items;
      s->Add("ASLENT");
      s->Add("BRILLIAN");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCD")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCDTempNum")->Items;
      s->Add("0");
      s->Add("1");
      s->Add("2");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRotateKit")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->Items;
      s->Add("Hot Plate 1");
      s->Add("Hot Plate 2");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->Items;
      s->Add("Auto 1");
      s->Add("Auto 2");
      s->Add("Auto 3");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->Items;
      s->Add("Cylinder");
      s->Add("1 Motor (1 to 8)");
      s->Add("4 Motor (2x2)");
      s->Add("8 Motor (2x4)");
      s->Add("1 Motor 1 Dut (1x1)");
      s->Add("2 Motor 2 Dut (1x2)");
      s->Add("In/Out Arm 1 Motor");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgSafeDoor")->Items;
      s->Add("7 Safe-Doors");
      s->Add("8 Safe-Doors");
      s->Add("9 Safe-Doors");
      s->Add("10 Safe-Doors");
      s->Add("7 Safe-Doors for 1032");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgSafeDoorLock")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgSafePlcIO")->Items;
      s->Add("Un Install");
      s->Add("Install_Schneider");
      s->Add("Install_Reer");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgScanner_AOI")->Items;
      s->Add("Un Install");
      s->Add("Bottom AOI");
      s->Add("TopBottom AOI");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgShuttleCrossSensor")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgShuttleFloating")->Items;
      s->Add("Un Install");
      s->Add("Use CCD");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgShuttleFloodgate")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgShuttleSensor")->Items;
      s->Add("Takex Sensor");
      s->Add("Line Sensor with Amplifier");
      s->Add("Line Sensor with CC-Link");
      s->Add("Line Sensor with CC-Link for 3 NU-CL1");
      s->Add("Line Sensor with CanBus");
      s->Add("Line Sensor with CanBus for 3 NU-DN1");
      s->Add("Line Sensor with EtherCAT");
      s->Add("Line Sensor with EtherCAT for 3 NU-EC1");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgShuttleVibration")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgShuttleZType")->Items;
      s->Add("Pitch Min : 50mm");
      s->Add("Pitch Min : 23mm");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgSocketClamp")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgSocketSen")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->Items;
      s->Add("ISA");
      s->Add("PCI 1735U");
      s->Add("RS-232(1Board)");
      s->Add("RS-232(2Board)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress")->Items;
      s->Add("No");
      s->Add("Yes");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTopScanner_AOI")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTrayArmMode")->Items;
      s->Add("Above Conveyor");
      s->Add("Under Conveyor");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTrayArmType")->Items;
      s->Add("Standard");
      s->Add("New Type (Wait Middle)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTrayMapping")->Items;
      s->Add("Un Install");
      s->Add("Install");
      s->Add("Device Remain Checking");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTrayMappingGrabImage")->Items;
      s->Add("Normal");
      s->Add("One Times");
      s->Add("Customer Plug-in");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTrayRobot")->Items;
      s->Add("Un Install");
      s->Add("Install (For HT-9046LM)");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTrayVibration")->Items;
      s->Add("Un Install");
      s->Add("Cylinder");
      s->Add("Motor");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgTriTempMachine")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgUse4DUT")->Items;
      s->Add("1 EA");
      s->Add("4 EA");
      s->Add("2 EA");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgUseOutSht")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgUsePulseType")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgUseSensor_9046_DB")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgUseSucker_9046_DB")->Items;
      s->Add("Un Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgVacuUnitType")->Items;
      s->Add("IO");
      s->Add("ECAT-VC8_ODM1");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgVibrationCommuncation")->Items;
      s->Add("Un-Install");
      s->Add("Install");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rgWeightCali")->Items;
      s->Add("Old Method");
      s->Add("New Method");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rg_ControlPanelMode")->Items;
      s->Add("IO control");
      s->Add("Communication");
    }
    { TStringList* s = EL<TRadioGroup>("THandlerSystem", "rg_IndexDoorHeater")->Items;
      s->Add("Uninstall");
      s->Add("Install");
    }
}

// golden HandlerSys.dfm 的父子關係（替身 → 容器），權限判斷用
static const char* const kHS_ParentOf[][2] = {
    {"ArmZAtoZH", "pnlInArm1"},
    {"Auto2SelectCy", "pnlLoader2"},
    {"ElectronPressure", "pnlIndex1"},
    {"GroupBox1", "pnlHandler3"},
    {"GroupBox2", "pnlHandler3"},
    {"GroupBox3", "pnlHandler1"},
    {"GroupBox4", "pnlIndex2"},
    {"GroupBox7", "pnlOther1"},
    {"GroupBox8", "pnlIndex5"},
    {"HeaterDoor1", "GroupBox2"},
    {"HeaterDoor2", "GroupBox2"},
    {"MachineTrack", "pnlLoader2"},
    {"Panel1", "TabSheet3"},
    {"Panel4", "pnlSht1"},
    {"SafeDoor1", "GroupBox1"},
    {"SafeDoor10", "GroupBox1"},
    {"SafeDoor2", "GroupBox1"},
    {"SafeDoor3", "GroupBox1"},
    {"SafeDoor4", "GroupBox1"},
    {"SafeDoor5", "GroupBox1"},
    {"SafeDoor6", "GroupBox1"},
    {"SafeDoor7", "GroupBox1"},
    {"SafeDoor8", "GroupBox1"},
    {"SafeDoor9", "GroupBox1"},
    {"TabSheet1", "pcSetting"},
    {"TabSheet2", "pcSetting"},
    {"TabSheet3", "pcSetting"},
    {"TabSheet4", "pcSetting"},
    {"TabSheet5", "pcSetting"},
    {"cb2DReader1", "TabSheet5"},
    {"cb2DReader2", "TabSheet5"},
    {"cb2DReader3", "TabSheet5"},
    {"cb2DReader4", "TabSheet5"},
    {"cbAirCon", "TabSheet5"},
    {"cbAuto1Cassette", "gbUseCassetteMode"},
    {"cbAuto2Cassette", "gbUseCassetteMode"},
    {"cbAuto3Cassette", "gbUseCassetteMode"},
    {"cbAuto4Cassette", "gbUseCassetteMode"},
    {"cbAuto5Cassette", "gbUseCassetteMode"},
    {"cbAuto6Cassette", "gbUseCassetteMode"},
    {"cbColorCassette", "gbUseCassetteMode"},
    {"cbColorSenAmpCnt", "grpOther"},
    {"cbComBinDisp", "TabSheet5"},
    {"cbComDyTemp", "TabSheet5"},
    {"cbComIndex", "TabSheet5"},
    {"cbComLaser1", "TabSheet5"},
    {"cbComLaser2", "TabSheet5"},
    {"cbComLaserInArm", "TabSheet5"},
    {"cbComRTC", "TabSheet5"},
    {"cbComTTLRS232", "TabSheet5"},
    {"cbComTTLRS232_2", "TabSheet5"},
    {"cbComTemp", "TabSheet5"},
    {"cbComTempOmron", "TabSheet5"},
    {"cbComTester", "TabSheet5"},
    {"cbEMAwarePort1", "TabSheet5"},
    {"cbEMAwarePort2", "TabSheet5"},
    {"cbEMAwarePort3", "TabSheet5"},
    {"cbEMAwarePort4", "TabSheet5"},
    {"cbESDUse4COM", "TabSheet5"},
    {"cbEmptyCassette", "gbUseCassetteMode"},
    {"cbEnableOutShtSensor", "Panel4"},
    {"cbHandlerModel", "GroupBox3"},
    {"cbIndEPCnt", "pnlIndex1"},
    {"cbKASUGA_Fan", "TabSheet5"},
    {"cbLoaderCassette", "gbUseCassetteMode"},
    {"cbNovx3360", "TabSheet5"},
    {"cbOCR", "TabSheet5"},
    {"cbOCRwithTester", "TabSheet5"},
    {"cbRotateSenAmpCnt", "grpOther"},
    {"cbSocketSenAmpCnt", "grpOther"},
    {"cbSocketSenAmpCnt2nd", "grpOther"},
    {"cbSocketSenAmpCnt3rd", "grpOther"},
    {"cbVibrationCardQty", "gbVibrationCommuncation"},
    {"cbbATC1", "TabSheet5"},
    {"cbbATC2", "TabSheet5"},
    {"cbbATC3", "TabSheet5"},
    {"cbbATC4", "TabSheet5"},
    {"cbbComBinDisp2", "TabSheet5"},
    {"cbbGroundMan", "TabSheet5"},
    {"cbbRFIDReader", "TabSheet5"},
    {"cbbTrayStepMotor", "TabSheet5"},
    {"chk2x3modeUseAxisYSensor", "Panel4"},
    {"chkAuto1", "grpTrayZUseMot"},
    {"chkAuto1Y", "grpTrayYUseMot"},
    {"chkAuto2", "grpTrayZUseMot"},
    {"chkAuto2Y", "grpTrayYUseMot"},
    {"chkAuto3", "grpTrayZUseMot"},
    {"chkAuto3Y", "grpTrayYUseMot"},
    {"chkAuto4", "grpTrayZUseMot"},
    {"chkAuto5", "grpTrayZUseMot"},
    {"chkAuto6", "grpTrayZUseMot"},
    {"chkBiasModeUseYSensor", "Panel4"},
    {"chkColor", "grpTrayZUseMot"},
    {"chkColorY", "grpTrayYUseMot"},
    {"chkEmpty", "grpTrayZUseMot"},
    {"chkEmptyY", "grpTrayYUseMot"},
    {"chkLoader", "grpTrayZUseMot"},
    {"chkLoaderY", "grpTrayYUseMot"},
    {"chkOutShtYSensorByLatch", "Panel4"},
    {"chkUseHPComCard", "TabSheet5"},
    {"chkUserDefMaxContactHeight", "grpContactHeight"},
    {"chkUser_Define_IndexZ_SafePos", "grpIndexZSafePos"},
    {"coCanBusNudn1", "grpCanBus"},
    {"coNudn1Macid11", "grpCanBus"},
    {"coNudn1Macid12", "grpCanBus"},
    {"coNudn1Macid13", "grpCanBus"},
    {"coNudn1Macid14", "grpCanBus"},
    {"ed24VMonitorPulseCount", "GroupBox7"},
    {"edATCSystemIP", "GroupBox4"},
    {"edATCSystemPort", "GroupBox4"},
    {"edATCSystemUseHeat", "GroupBox4"},
    {"edCognexSystemCCD", "grpCognexCCD"},
    {"edGroundMan_AlarmOhm", "grpGroundMan"},
    {"edHotGunFlow_DevNo", "GroupBox8"},
    {"edHotGunFlow_Gun1_ChannelNo", "GroupBox8"},
    {"edHotGunFlow_Gun2_ChannelNo", "GroupBox8"},
    {"edHotGunFlow_LineNo", "GroupBox8"},
    {"edIONPulseCount", "grpIONPulseCount"},
    {"edMaxKpa", "gbEPSetting"},
    {"edMaxMpaFB", "gbEPReadSetting"},
    {"edMinMpa", "gbEPSetting"},
    {"edtBaseMaxTemp", "grp_SetHeaterTemp"},
    {"edtCustomerCode", "GroupBox3"},
    {"edtHPLimit", "grpInOutArmYPitch"},
    {"edtIndexMaxTemp", "grp_SetHeaterTemp"},
    {"edtMaxXPitch", "grpInOutArmYPitch"},
    {"edtMaxYPitch", "grpInOutArmYPitch"},
    {"edtMinMpaFB", "gbEPReadSetting"},
    {"edtMinXPitch", "grpInOutArmYPitch"},
    {"edtMinYPitch", "grpInOutArmYPitch"},
    {"edtOutShtMaxTemp", "grp_SetHeaterTemp"},
    {"edtOutSortXPitchMax", "grpOutSortArm"},
    {"edtOutSortXPitchMin", "grpOutSortArm"},
    {"edtSeriaNo", "GroupBox3"},
    {"edtTriTempTotalCh", "grpTriTemperature"},
    {"edtTriTemperature_MaxDegree", "grpTriTemperature"},
    {"edtTriTemperature_MinDegree", "grpTriTemperature"},
    {"edtUserDefMaxContactHeight", "grpContactHeight"},
    {"edtUserDefineIndexZSafePos", "grpIndexZSafePos"},
    {"edt_Total_Compressor", "grpTriTemperature"},
    {"gbART", "pnlLoader4"},
    {"gbEPReadSetting", "pnlIndex1"},
    {"gbEPSetting", "pnlIndex1"},
    {"gbRotateKit", "tsInOutArm"},
    {"gbUseCassetteMode", "pnlLoader2"},
    {"gbVibrationCommuncation", "pnlOther1"},
    {"grpCanBus", "grpOther"},
    {"grpCognexCCD", "pnlSht2"},
    {"grpContactHeight", "pnlIndex5"},
    {"grpGroundMan", "pnlESD2"},
    {"grpHeater", "tsHeater"},
    {"grpIONPulseCount", "pnlESD1"},
    {"grpInOutArmYPitch", "pnlInArm1"},
    {"grpIndexZSafePos", "pnlIndex5"},
    {"grpOther", "TabSheet3"},
    {"grpOutSortArm", "pnlInArm4"},
    {"grpTrayYUseMot", "pnlLoader4"},
    {"grpTrayZUseMot", "pnlLoader2"},
    {"grpTriTemperature", "Panel1"},
    {"grp_SetHeaterTemp", "grpTriTemperature"},
    {"pcSetting", "Panel2"},
    {"pnlCloseLoop", "tsIONFan"},
    {"pnlESD1", "tsIONFan"},
    {"pnlESD2", "tsIONFan"},
    {"pnlHandler1", "TabSheet1"},
    {"pnlHandler2", "TabSheet1"},
    {"pnlHandler3", "TabSheet1"},
    {"pnlInArm1", "tsInOutArm"},
    {"pnlInArm2", "tsInOutArm"},
    {"pnlInArm3", "tsInOutArm"},
    {"pnlInArm4", "tsInOutArm"},
    {"pnlIndex1", "TabSheet4"},
    {"pnlIndex2", "TabSheet4"},
    {"pnlIndex3", "TabSheet4"},
    {"pnlIndex4", "TabSheet4"},
    {"pnlIndex5", "TabSheet4"},
    {"pnlLoader1", "TabSheet2"},
    {"pnlLoader2", "TabSheet2"},
    {"pnlLoader3", "TabSheet2"},
    {"pnlLoader4", "TabSheet2"},
    {"pnlLoader5", "TabSheet2"},
    {"pnlOther1", "TabSheet3"},
    {"pnlSht1", "tsShuttle"},
    {"pnlSht2", "tsShuttle"},
    {"pnlSht3", "tsShuttle"},
    {"rg2DBarcode", "pnlSht2"},
    {"rg2ndLoader", "pnlLoader3"},
    {"rg3M_EM_AWARE_Monitor", "pnlCloseLoop"},
    {"rgAOI", "pnlOther1"},
    {"rgATC", "pnlIndex2"},
    {"rgATCHeatGun", "pnlIndex4"},
    {"rgATCMixMode", "pnlIndex2"},
    {"rgAirConditioner", "pnlIndex4"},
    {"rgAirStreamSelect", "grpTriTemperature"},
    {"rgAuto1ART", "gbART"},
    {"rgAuto2ART", "gbART"},
    {"rgAuto3ART", "gbART"},
    {"rgAuto3Magazine", "pnlLoader3"},
    {"rgAuto4ART", "gbART"},
    {"rgAuto5ART", "gbART"},
    {"rgAuto6ART", "gbART"},
    {"rgAutoAlignment", "pnlLoader1"},
    {"rgAutoCleanIonFan", "pnlCloseLoop"},
    {"rgAutoShuttleSensor", "pnlSht1"},
    {"rgAutoTrackCanGoRear", "pnlLoader4"},
    {"rgBaseHeaterCount", "pnlOther1"},
    {"rgBottom2DID", "pnlSht2"},
    {"rgBottom2DID_CCD", "pnlSht2"},
    {"rgCCDAutoAlignmentMode", "pnlInArm2"},
    {"rgCCDTemp", "pnlSht2"},
    {"rgCKDFCM", "pnlLoader3"},
    {"rgCOLORSENSOR_MUN", "pnlLoader1"},
    {"rgCanBusMethod", "grpCanBus"},
    {"rgCatchTrayBlock", "pnlLoader3"},
    {"rgCatchTrayModel", "pnlLoader3"},
    {"rgChamberUsePulseType", "pnlESD1"},
    {"rgCleanAir", "pnlIndex3"},
    {"rgColorSensor", "grpOther"},
    {"rgCoverTrayID", "pnlLoader5"},
    {"rgCustomerList", "tsCustomerCode"},
    {"rgDeviceFlipper", "pnlInArm2"},
    {"rgDewpointHW", "pnlIndex5"},
    {"rgDieClean", "pnlInArm2"},
    {"rgDoubleBeltMode", "pnlLoader5"},
    {"rgDoubleEPControl", "pnlIndex1"},
    {"rgE84Sensor", "Panel1"},
    {"rgESDTemp", "pnlIndex4"},
    {"rgEmptyKeyence", "pnlLoader5"},
    {"rgFinePitch", "pnlIndex3"},
    {"rgFix3FullPlace", "pnlLoader1"},
    {"rgFixAICCD", "pnlInArm2"},
    {"rgGroundMan", "grpGroundMan"},
    {"rgGroundMan_ScanPoint", "grpGroundMan"},
    {"rgHTIonBar", "pnlCloseLoop"},
    {"rgHeatGun", "pnlIndex5"},
    {"rgHeater", "pnlIndex2"},
    {"rgHeaterType", "pnlIndex2"},
    {"rgHighTempLimit", "pnlIndex5"},
    {"rgHotGunFlow", "GroupBox8"},
    {"rgHotPlateLimit", "pnlInArm3"},
    {"rgHotPlatePos", "pnlInArm3"},
    {"rgHotplateType", "pnlInArm3"},
    {"rgIOCard", "pnlHandler1"},
    {"rgIOChangeToque", "pnlIndex3"},
    {"rgInOutArmXPitch", "grpInOutArmYPitch"},
    {"rgInOutArmYPitch", "grpInOutArmYPitch"},
    {"rgInShtAutoLatch", "pnlSht1"},
    {"rgInShtLastSensor", "pnlSht1"},
    {"rgIndexMotionCard", "pnlHandler2"},
    {"rgIndexMotorAxis", "pnlIndex3"},
    {"rgIndexMotorType", "pnlIndex3"},
    {"rgIndexPressType", "pnlIndex1"},
    {"rgIndexSuckerType", "pnlIndex3"},
    {"rgInstallARTtwocylinder", "gbART"},
    {"rgInstallAutoRestest", "gbART"},
    {"rgInstallFix3", "pnlLoader1"},
    {"rgIonFanType", "pnlESD1"},
    {"rgKasuga", "pnlCloseLoop"},
    {"rgKasuga_Fan", "pnlCloseLoop"},
    {"rgLBTemp", "pnlIndex4"},
    {"rgLBTemp2", "pnlIndex4"},
    {"rgLDCarRotAtm", "pnlLoader1"},
    {"rgLaserDistance", "pnlInArm2"},
    {"rgLdUldCassetteMode", "pnlLoader1"},
    {"rgLdUldUseStepMotor", "pnlLoader2"},
    {"rgLoaderHinge", "pnlLoader3"},
    {"rgLoaderKeyence", "pnlLoader5"},
    {"rgLoaderVibration", "pnlLoader1"},
    {"rgMNetSpeed", "pnlHandler2"},
    {"rgMRSystem", "pnlLoader3"},
    {"rgMagBinDispType", "Panel1"},
    {"rgMagneticScale", "pnlInArm3"},
    {"rgModel", "GroupBox3"},
    {"rgMotionCard", "pnlHandler1"},
    {"rgMultileEmpty", "pnlLoader3"},
    {"rgMultileEmptyTrayIDKeyence", "pnlLoader5"},
    {"rgNUECType", "pnlSht3"},
    {"rgNovx3360", "pnlCloseLoop"},
    {"rgNumberPanelType", "pnlLoader1"},
    {"rgOCR", "pnlInArm2"},
    {"rgOCRYStepMot", "pnlLoader1"},
    {"rgOHTSystem", "pnlLoader3"},
    {"rgOTDInstall", "pnlIndex4"},
    {"rgOutArmYPitch", "pnlInArm2"},
    {"rgOutSortArm", "grpOutSortArm"},
    {"rgPickerCount", "pnlInArm1"},
    {"rgPreciser", "pnlInArm2"},
    {"rgPreciserPos", "pnlInArm2"},
    {"rgRFIDReader", "pnlLoader5"},
    {"rgRFIDSystem", "pnlLoader5"},
    {"rgRealTimeCCD", "pnlIndex3"},
    {"rgRealTimeCCDTempNum", "pnlIndex3"},
    {"rgRotateKit", "gbRotateKit"},
    {"rgRotateKitIn", "gbRotateKit"},
    {"rgRotateKitOut", "gbRotateKit"},
    {"rgRotateKit_Type", "gbRotateKit"},
    {"rgSafeDoor", "pnlOther1"},
    {"rgSafeDoorLock", "pnlHandler2"},
    {"rgSafePlcIO", "pnlHandler2"},
    {"rgScanner_AOI", "pnlOther1"},
    {"rgShuttleCrossSensor", "pnlSht1"},
    {"rgShuttleFloating", "pnlSht2"},
    {"rgShuttleFloodgate", "pnlOther1"},
    {"rgShuttleSensor", "pnlSht1"},
    {"rgShuttleVibration", "pnlSht3"},
    {"rgShuttleZType", "pnlSht1"},
    {"rgSocketClamp", "pnlIndex3"},
    {"rgSocketSen", "grpOther"},
    {"rgTTLCard", "pnlHandler1"},
    {"rgTTLUseAddress", "pnlHandler1"},
    {"rgTopScanner_AOI", "pnlOther1"},
    {"rgTrayArmMode", "pnlLoader1"},
    {"rgTrayArmType", "pnlLoader3"},
    {"rgTrayMapping", "pnlLoader5"},
    {"rgTrayMappingGrabImage", "pnlLoader5"},
    {"rgTrayRobot", "pnlLoader3"},
    {"rgTrayVibration", "pnlLoader1"},
    {"rgTriTempMachine", "grpTriTemperature"},
    {"rgUse4DUT", "pnlIndex4"},
    {"rgUseOutSht", "pnlSht3"},
    {"rgUsePulseType", "pnlESD1"},
    {"rgUseSensor_9046_DB", "pnlHandler1"},
    {"rgUseSucker_9046_DB", "pnlHandler1"},
    {"rgVacuUnitType", "pnlHandler2"},
    {"rgVibrationCommuncation", "gbVibrationCommuncation"},
    {"rgWeightCali", "pnlIndex1"},
    {"rg_ControlPanelMode", "pnlHandler2"},
    {"rg_IndexDoorHeater", "Panel1"},
    {"tsCustomerCode", "pcSetting"},
    {"tsHeater", "pcSetting"},
    {"tsIONFan", "pcSetting"},
    {"tsInOutArm", "pcSetting"},
    {"tsShuttle", "pcSetting"},
};

// 上面用到的容器替身先建好（型別照 golden header）
static void HS_CreateContainerProxies()
{
    EL<TGroupBox>("THandlerSystem", "GroupBox1");
    EL<TGroupBox>("THandlerSystem", "GroupBox2");
    EL<TGroupBox>("THandlerSystem", "GroupBox3");
    EL<TGroupBox>("THandlerSystem", "GroupBox4");
    EL<TGroupBox>("THandlerSystem", "GroupBox7");
    EL<TGroupBox>("THandlerSystem", "GroupBox8");
    EL<TPanel>("THandlerSystem", "Panel1");
    EL<TPanel>("THandlerSystem", "Panel2");
    EL<TPanel>("THandlerSystem", "Panel4");
    EL<TTabSheet>("THandlerSystem", "TabSheet1");
    EL<TTabSheet>("THandlerSystem", "TabSheet2");
    EL<TTabSheet>("THandlerSystem", "TabSheet3");
    EL<TTabSheet>("THandlerSystem", "TabSheet4");
    EL<TTabSheet>("THandlerSystem", "TabSheet5");
    EL<TGroupBox>("THandlerSystem", "gbART");
    EL<TGroupBox>("THandlerSystem", "gbEPReadSetting");
    EL<TGroupBox>("THandlerSystem", "gbEPSetting");
    EL<TGroupBox>("THandlerSystem", "gbRotateKit");
    EL<TGroupBox>("THandlerSystem", "gbUseCassetteMode");
    EL<TGroupBox>("THandlerSystem", "gbVibrationCommuncation");
    EL<TGroupBox>("THandlerSystem", "grpCanBus");
    EL<TGroupBox>("THandlerSystem", "grpCognexCCD");
    EL<TGroupBox>("THandlerSystem", "grpContactHeight");
    EL<TGroupBox>("THandlerSystem", "grpGroundMan");
    EL<TGroupBox>("THandlerSystem", "grpIONPulseCount");
    EL<TGroupBox>("THandlerSystem", "grpInOutArmYPitch");
    EL<TGroupBox>("THandlerSystem", "grpIndexZSafePos");
    EL<TGroupBox>("THandlerSystem", "grpOther");
    EL<TGroupBox>("THandlerSystem", "grpOutSortArm");
    EL<TGroupBox>("THandlerSystem", "grpTrayYUseMot");
    EL<TGroupBox>("THandlerSystem", "grpTrayZUseMot");
    EL<TGroupBox>("THandlerSystem", "grpTriTemperature");
    EL<TGroupBox>("THandlerSystem", "grp_SetHeaterTemp");
    EL<TPageControl>("THandlerSystem", "pcSetting");
    EL<TPanel>("THandlerSystem", "pnlCloseLoop");
    EL<TPanel>("THandlerSystem", "pnlESD1");
    EL<TPanel>("THandlerSystem", "pnlESD2");
    EL<TPanel>("THandlerSystem", "pnlHandler1");
    EL<TPanel>("THandlerSystem", "pnlHandler2");
    EL<TPanel>("THandlerSystem", "pnlHandler3");
    EL<TPanel>("THandlerSystem", "pnlInArm1");
    EL<TPanel>("THandlerSystem", "pnlInArm2");
    EL<TPanel>("THandlerSystem", "pnlInArm3");
    EL<TPanel>("THandlerSystem", "pnlInArm4");
    EL<TPanel>("THandlerSystem", "pnlIndex1");
    EL<TPanel>("THandlerSystem", "pnlIndex2");
    EL<TPanel>("THandlerSystem", "pnlIndex3");
    EL<TPanel>("THandlerSystem", "pnlIndex4");
    EL<TPanel>("THandlerSystem", "pnlIndex5");
    EL<TPanel>("THandlerSystem", "pnlLoader1");
    EL<TPanel>("THandlerSystem", "pnlLoader2");
    EL<TPanel>("THandlerSystem", "pnlLoader3");
    EL<TPanel>("THandlerSystem", "pnlLoader4");
    EL<TPanel>("THandlerSystem", "pnlLoader5");
    EL<TPanel>("THandlerSystem", "pnlOther1");
    EL<TPanel>("THandlerSystem", "pnlSht1");
    EL<TPanel>("THandlerSystem", "pnlSht2");
    EL<TPanel>("THandlerSystem", "pnlSht3");
    EL<TTabSheet>("THandlerSystem", "tsCustomerCode");
    EL<TTabSheet>("THandlerSystem", "tsHeater");
    EL<TTabSheet>("THandlerSystem", "tsIONFan");
    EL<TTabSheet>("THandlerSystem", "tsInOutArm");
    EL<TTabSheet>("THandlerSystem", "tsShuttle");
    filerw::ELSetParents("THandlerSystem", kHS_ParentOf, (int)(sizeof(kHS_ParentOf) / sizeof(kHS_ParentOf[0])));
}

// golden HandlerSys.dfm 設計期的 Enabled=False／ReadOnly=True／Visible=False（開機註冊前套上）
static void HS_DfmState()
{
    EL<TGroupBox>("THandlerSystem", "GroupBox1")->Visible = false;
    EL<TGroupBox>("THandlerSystem", "GroupBox2")->Visible = false;
    EL<TComboBox>("THandlerSystem", "cbNovx3360")->Visible = false;
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->Enabled = false;
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->Enabled = false;
    EL<TRadioGroup>("THandlerSystem", "ArmZAtoZH")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "Auto2SelectCy")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "ElectronPressure")->ItemIndex = 3;
    EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex = 3;
    EL<TComboBox>("THandlerSystem", "cb2DReader1")->Text = "COM15";
    EL<TComboBox>("THandlerSystem", "cb2DReader2")->Text = "COM16";
    EL<TComboBox>("THandlerSystem", "cb2DReader3")->Text = "COM17";
    EL<TComboBox>("THandlerSystem", "cb2DReader4")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbAirCon")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbColorSenAmpCnt")->Text = "0";
    EL<TComboBox>("THandlerSystem", "cbComBinDisp")->Text = "COM14";
    EL<TComboBox>("THandlerSystem", "cbComDyTemp")->Text = "COM2";
    EL<TComboBox>("THandlerSystem", "cbComIndex")->Text = "COM11";
    EL<TComboBox>("THandlerSystem", "cbComLaser1")->Text = "COM16";
    EL<TComboBox>("THandlerSystem", "cbComLaser2")->Text = "COM17";
    EL<TComboBox>("THandlerSystem", "cbComLaserInArm")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbComRTC")->Text = "COM1";
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232")->Text = "COM3";
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232_2")->Text = "COM8";
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232_2")->ItemIndex = 7;
    EL<TComboBox>("THandlerSystem", "cbComTemp")->Text = "COM12";
    EL<TComboBox>("THandlerSystem", "cbComTempOmron")->Text = "COM13";
    EL<TComboBox>("THandlerSystem", "cbComTester")->Text = "COM2";
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort1")->Text = "COM15";
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort2")->Text = "COM16";
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort3")->Text = "COM17";
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort4")->Text = "COM18";
    EL<TCheckBox>("THandlerSystem", "cbESDUse4COM")->Checked = true;
    EL<TComboBox>("THandlerSystem", "cbHandlerModel")->Text = "HT9046LS";
    EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex = 3;
    EL<TComboBox>("THandlerSystem", "cbIndEPCnt")->Text = "16";
    EL<TComboBox>("THandlerSystem", "cbKASUGA_Fan")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbNovx3360")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbOCR")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbOCRwithTester")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbRotateSenAmpCnt")->Text = "0";
    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt")->Text = "0";
    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt2nd")->Text = "0";
    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt3rd")->Text = "0";
    EL<TComboBox>("THandlerSystem", "cbVibrationCardQty")->Text = "2";
    EL<TComboBox>("THandlerSystem", "cbbATC1")->Text = "COM15";
    EL<TComboBox>("THandlerSystem", "cbbATC2")->Text = "COM16";
    EL<TComboBox>("THandlerSystem", "cbbATC3")->Text = "COM17";
    EL<TComboBox>("THandlerSystem", "cbbATC4")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbbComBinDisp2")->Text = "COM14";
    EL<TComboBox>("THandlerSystem", "cbbGroundMan")->Text = "COM18";
    EL<TComboBox>("THandlerSystem", "cbbRFIDReader")->Text = "COM8";
    EL<TComboBox>("THandlerSystem", "cbbRFIDReader")->ItemIndex = 7;
    EL<TComboBox>("THandlerSystem", "cbbTrayStepMotor")->Text = "COM18";
    EL<TCheckBox>("THandlerSystem", "chkUseHPComCard")->Checked = true;
    EL<TComboBox>("THandlerSystem", "coCanBusNudn1")->Text = "0";
    EL<TComboBox>("THandlerSystem", "coNudn1Macid11")->Text = "0";
    EL<TComboBox>("THandlerSystem", "coNudn1Macid11")->ItemIndex = 0;
    EL<TComboBox>("THandlerSystem", "coNudn1Macid12")->Text = "0";
    EL<TComboBox>("THandlerSystem", "coNudn1Macid12")->ItemIndex = 0;
    EL<TComboBox>("THandlerSystem", "coNudn1Macid13")->Text = "0";
    EL<TComboBox>("THandlerSystem", "coNudn1Macid13")->ItemIndex = 0;
    EL<TComboBox>("THandlerSystem", "coNudn1Macid14")->Text = "0";
    EL<TComboBox>("THandlerSystem", "coNudn1Macid14")->ItemIndex = 0;
    EL<TEdit>("THandlerSystem", "ed24VMonitorPulseCount")->Text = "20";
    EL<TEdit>("THandlerSystem", "edATCSystemIP")->Text = "172.16.8.90";
    EL<TEdit>("THandlerSystem", "edATCSystemPort")->Text = "5001";
    EL<TEdit>("THandlerSystem", "edATCSystemUseHeat")->Text = "4";
    EL<TEdit>("THandlerSystem", "edCognexSystemCCD")->Text = "2";
    EL<TEdit>("THandlerSystem", "edGroundMan_AlarmOhm")->Text = "3";
    EL<TEdit>("THandlerSystem", "edHotGunFlow_DevNo")->Text = "0";
    EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun1_ChannelNo")->Text = "0";
    EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun2_ChannelNo")->Text = "0";
    EL<TEdit>("THandlerSystem", "edHotGunFlow_LineNo")->Text = "0";
    EL<TEdit>("THandlerSystem", "edIONPulseCount")->Text = "700";
    EL<TEdit>("THandlerSystem", "edMaxKpa")->Text = "499";
    EL<TEdit>("THandlerSystem", "edMaxMpaFB")->Text = "5.013";
    EL<TEdit>("THandlerSystem", "edMinMpa")->Text = "0.001";
    EL<TEdit>("THandlerSystem", "edtBaseMaxTemp")->Text = "60";
    EL<TEdit>("THandlerSystem", "edtCustomerCode")->Text = "0";
    EL<TEdit>("THandlerSystem", "edtHPLimit")->Text = "7000";
    EL<TEdit>("THandlerSystem", "edtIndexMaxTemp")->Text = "60";
    EL<TEdit>("THandlerSystem", "edtMaxXPitch")->Text = "12000";
    EL<TEdit>("THandlerSystem", "edtMaxYPitch")->Text = "6000";
    EL<TEdit>("THandlerSystem", "edtMinMpaFB")->Text = "0.968";
    EL<TEdit>("THandlerSystem", "edtMinXPitch")->Text = "6000";
    EL<TEdit>("THandlerSystem", "edtMinYPitch")->Text = "2000";
    EL<TEdit>("THandlerSystem", "edtOutShtMaxTemp")->Text = "60";
    EL<TEdit>("THandlerSystem", "edtOutSortXPitchMax")->Text = "4000";
    EL<TEdit>("THandlerSystem", "edtOutSortXPitchMin")->Text = "1500";
    EL<TEdit>("THandlerSystem", "edtSeriaNo")->Text = "29818";
    EL<TEdit>("THandlerSystem", "edtTriTempTotalCh")->Text = "40";
    EL<TEdit>("THandlerSystem", "edtTriTemperature_MaxDegree")->Text = "175";
    EL<TEdit>("THandlerSystem", "edtTriTemperature_MinDegree")->Text = "-55";
    EL<TEdit>("THandlerSystem", "edtUserDefMaxContactHeight")->Text = "-135.0";
    EL<TEdit>("THandlerSystem", "edtUserDefineIndexZSafePos")->Text = "200";
    EL<TEdit>("THandlerSystem", "edt_Total_Compressor")->Text = "3";
    EL<TRadioGroup>("THandlerSystem", "rg2DBarcode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rg2ndLoader")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rg3M_EM_AWARE_Monitor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAOI")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgATC")->ItemIndex = 3;
    EL<TRadioGroup>("THandlerSystem", "rgATCHeatGun")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgATCMixMode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAirConditioner")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAirStreamSelect")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgAuto1ART")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAuto2ART")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAuto3ART")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAuto3Magazine")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAuto4ART")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAuto5ART")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAuto6ART")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAutoAlignment")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAutoCleanIonFan")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAutoShuttleSensor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgAutoTrackCanGoRear")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgBaseHeaterCount")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgBottom2DID")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgBottom2DID_CCD")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCCDAutoAlignmentMode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCCDTemp")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCKDFCM")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCOLORSENSOR_MUN")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCanBusMethod")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCatchTrayBlock")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCatchTrayModel")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgChamberUsePulseType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCleanAir")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgColorSensor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCoverTrayID")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgCustomerList")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgDeviceFlipper")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgDewpointHW")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgDieClean")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgDoubleBeltMode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgDoubleEPControl")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgE84Sensor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgESDTemp")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgEmptyKeyence")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgFinePitch")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgFix3FullPlace")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgFixAICCD")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgGroundMan")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgGroundMan_ScanPoint")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHTIonBar")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHeatGun")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHeater")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex = 3;
    EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHotGunFlow")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHotPlateLimit")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHotPlatePos")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgHotplateType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgIOCard")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgIOChangeToque")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgInOutArmXPitch")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgInOutArmYPitch")->ItemIndex = 6;
    EL<TRadioGroup>("THandlerSystem", "rgInShtAutoLatch")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgInShtLastSensor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotionCard")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorAxis")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgIndexPressType")->ItemIndex = 6;
    EL<TRadioGroup>("THandlerSystem", "rgIndexSuckerType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgInstallARTtwocylinder")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgInstallAutoRestest")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgInstallFix3")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgIonFanType")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgKasuga")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgKasuga_Fan")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLBTemp")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLBTemp2")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLDCarRotAtm")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLaserDistance")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLdUldCassetteMode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLdUldUseStepMotor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLoaderHinge")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLoaderKeyence")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgLoaderVibration")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgMNetSpeed")->ItemIndex = 3;
    EL<TRadioGroup>("THandlerSystem", "rgMRSystem")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgMagBinDispType")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgMagneticScale")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgModel")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgMotionCard")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgMultileEmpty")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgMultileEmptyTrayIDKeyence")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgNUECType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgNovx3360")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgNumberPanelType")->ItemIndex = 3;
    EL<TRadioGroup>("THandlerSystem", "rgOCR")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgOCRYStepMot")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgOHTSystem")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgOTDInstall")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgOutArmYPitch")->ItemIndex = 6;
    EL<TRadioGroup>("THandlerSystem", "rgOutSortArm")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgPickerCount")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgPreciser")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgPreciserPos")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgRFIDReader")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgRFIDSystem")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCD")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCDTempNum")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgRotateKit")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgSafeDoor")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgSafeDoorLock")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgSafePlcIO")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgScanner_AOI")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleCrossSensor")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleFloating")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleFloodgate")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleSensor")->ItemIndex = 4;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleVibration")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleZType")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgSocketClamp")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgSocketSen")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTopScanner_AOI")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTrayArmMode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTrayArmType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTrayMapping")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTrayMappingGrabImage")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTrayRobot")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTrayVibration")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgTriTempMachine")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgUse4DUT")->ItemIndex = 2;
    EL<TRadioGroup>("THandlerSystem", "rgUseOutSht")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgUsePulseType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgUseSensor_9046_DB")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgUseSucker_9046_DB")->ItemIndex = 1;
    EL<TRadioGroup>("THandlerSystem", "rgVacuUnitType")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgVibrationCommuncation")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rgWeightCali")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rg_ControlPanelMode")->ItemIndex = 0;
    EL<TRadioGroup>("THandlerSystem", "rg_IndexDoorHeater")->ItemIndex = 0;
}

// golden FormShow 開頭 ChangeCompomentEnabled(頁, true, true) 會重新打開的替身（容器＋五種元件，golden cAuthority.cpp:470-507）
static const char* const kHS_EnableAll[] = {
    "ArmZAtoZH", "Auto2SelectCy", "ElectronPressure", "GroupBox1", "GroupBox2", "GroupBox3",
    "GroupBox4", "GroupBox7", "GroupBox8", "HeaterDoor1", "HeaterDoor2", "MachineTrack",
    "Panel1", "Panel2", "Panel4", "SafeDoor1", "SafeDoor10", "SafeDoor2",
    "SafeDoor3", "SafeDoor4", "SafeDoor5", "SafeDoor6", "SafeDoor7", "SafeDoor8",
    "SafeDoor9", "TabSheet1", "TabSheet2", "TabSheet3", "TabSheet4", "TabSheet5",
    "cbAuto1Cassette", "cbAuto2Cassette", "cbAuto3Cassette", "cbAuto4Cassette", "cbAuto5Cassette", "cbAuto6Cassette",
    "cbColorCassette", "cbESDUse4COM", "cbEmptyCassette", "cbEnableOutShtSensor", "cbLoaderCassette", "chk2x3modeUseAxisYSensor",
    "chkAuto1", "chkAuto1Y", "chkAuto2", "chkAuto2Y", "chkAuto3", "chkAuto3Y",
    "chkAuto4", "chkAuto5", "chkAuto6", "chkBiasModeUseYSensor", "chkColor", "chkColorY",
    "chkEmpty", "chkEmptyY", "chkLoader", "chkLoaderY", "chkOutShtYSensorByLatch", "chkUseHPComCard",
    "chkUserDefMaxContactHeight", "chkUser_Define_IndexZ_SafePos", "gbART", "gbEPReadSetting", "gbEPSetting", "gbRotateKit",
    "gbUseCassetteMode", "gbVibrationCommuncation", "grpCanBus", "grpCognexCCD", "grpContactHeight", "grpGroundMan",
    "grpHeater", "grpIONPulseCount", "grpInOutArmYPitch", "grpIndexZSafePos", "grpOther", "grpOutSortArm",
    "grpTrayYUseMot", "grpTrayZUseMot", "grpTriTemperature", "grp_SetHeaterTemp", "pcSetting", "pnlCloseLoop",
    "pnlESD1", "pnlESD2", "pnlHandler1", "pnlHandler2", "pnlHandler3", "pnlInArm1",
    "pnlInArm2", "pnlInArm3", "pnlInArm4", "pnlIndex1", "pnlIndex2", "pnlIndex3",
    "pnlIndex4", "pnlIndex5", "pnlLoader1", "pnlLoader2", "pnlLoader3", "pnlLoader4",
    "pnlLoader5", "pnlOther1", "pnlSht1", "pnlSht2", "pnlSht3", "rg2DBarcode",
    "rg2ndLoader", "rg3M_EM_AWARE_Monitor", "rgAOI", "rgATC", "rgATCHeatGun", "rgATCMixMode",
    "rgAirConditioner", "rgAirStreamSelect", "rgAuto1ART", "rgAuto2ART", "rgAuto3ART", "rgAuto3Magazine",
    "rgAuto4ART", "rgAuto5ART", "rgAuto6ART", "rgAutoAlignment", "rgAutoCleanIonFan", "rgAutoShuttleSensor",
    "rgAutoTrackCanGoRear", "rgBaseHeaterCount", "rgBottom2DID", "rgBottom2DID_CCD", "rgCCDAutoAlignmentMode", "rgCCDTemp",
    "rgCKDFCM", "rgCOLORSENSOR_MUN", "rgCanBusMethod", "rgCatchTrayBlock", "rgCatchTrayModel", "rgChamberUsePulseType",
    "rgCleanAir", "rgColorSensor", "rgCoverTrayID", "rgCustomerList", "rgDeviceFlipper", "rgDewpointHW",
    "rgDieClean", "rgDoubleBeltMode", "rgDoubleEPControl", "rgE84Sensor", "rgESDTemp", "rgEmptyKeyence",
    "rgFinePitch", "rgFix3FullPlace", "rgFixAICCD", "rgGroundMan", "rgGroundMan_ScanPoint", "rgHTIonBar",
    "rgHeatGun", "rgHeater", "rgHeaterType", "rgHighTempLimit", "rgHotGunFlow", "rgHotPlateLimit",
    "rgHotPlatePos", "rgHotplateType", "rgIOCard", "rgIOChangeToque", "rgInOutArmXPitch", "rgInOutArmYPitch",
    "rgInShtAutoLatch", "rgInShtLastSensor", "rgIndexMotionCard", "rgIndexMotorAxis", "rgIndexMotorType", "rgIndexPressType",
    "rgIndexSuckerType", "rgInstallARTtwocylinder", "rgInstallAutoRestest", "rgInstallFix3", "rgIonFanType", "rgKasuga",
    "rgKasuga_Fan", "rgLBTemp", "rgLBTemp2", "rgLDCarRotAtm", "rgLaserDistance", "rgLdUldCassetteMode",
    "rgLdUldUseStepMotor", "rgLoaderHinge", "rgLoaderKeyence", "rgLoaderVibration", "rgMNetSpeed", "rgMRSystem",
    "rgMagBinDispType", "rgMagneticScale", "rgModel", "rgMotionCard", "rgMultileEmpty", "rgMultileEmptyTrayIDKeyence",
    "rgNUECType", "rgNovx3360", "rgNumberPanelType", "rgOCR", "rgOCRYStepMot", "rgOHTSystem",
    "rgOTDInstall", "rgOutArmYPitch", "rgOutSortArm", "rgPickerCount", "rgPreciser", "rgPreciserPos",
    "rgRFIDReader", "rgRFIDSystem", "rgRealTimeCCD", "rgRealTimeCCDTempNum", "rgRotateKit", "rgRotateKitIn",
    "rgRotateKitOut", "rgRotateKit_Type", "rgSafeDoor", "rgSafeDoorLock", "rgSafePlcIO", "rgScanner_AOI",
    "rgShuttleCrossSensor", "rgShuttleFloating", "rgShuttleFloodgate", "rgShuttleSensor", "rgShuttleVibration", "rgShuttleZType",
    "rgSocketClamp", "rgSocketSen", "rgTTLCard", "rgTTLUseAddress", "rgTopScanner_AOI", "rgTrayArmMode",
    "rgTrayArmType", "rgTrayMapping", "rgTrayMappingGrabImage", "rgTrayRobot", "rgTrayVibration", "rgTriTempMachine",
    "rgUse4DUT", "rgUseOutSht", "rgUsePulseType", "rgUseSensor_9046_DB", "rgUseSucker_9046_DB", "rgVacuUnitType",
    "rgVibrationCommuncation", "rgWeightCali", "rg_ControlPanelMode", "rg_IndexDoorHeater", "tsCustomerCode", "tsHeater",
    "tsIONFan", "tsInOutArm", "tsShuttle",
};

// 存檔流程（SaveBtnClick、SaveSystemSet）讀的替身 257 個 —— FileRW/HSys.cpp 用它決定頁面必須送哪些值
static const char* const kHS_SaveReads[] = {
    "ArmZAtoZH", "Auto2SelectCy", "ElectronPressure", "MachineTrack", "cb2DReader1", "cb2DReader2",
    "cb2DReader3", "cb2DReader4", "cbAirCon", "cbAuto1Cassette", "cbAuto2Cassette", "cbAuto3Cassette",
    "cbAuto4Cassette", "cbAuto5Cassette", "cbAuto6Cassette", "cbColorCassette", "cbColorSenAmpCnt", "cbComBinDisp",
    "cbComDyTemp", "cbComIndex", "cbComLaser1", "cbComLaser2", "cbComLaserInArm", "cbComRTC",
    "cbComTTLRS232", "cbComTTLRS232_2", "cbComTemp", "cbComTempOmron", "cbComTester", "cbEMAwarePort1",
    "cbEMAwarePort2", "cbEMAwarePort3", "cbEMAwarePort4", "cbESDUse4COM", "cbEmptyCassette", "cbEnableOutShtSensor",
    "cbHandlerModel", "cbIndEPCnt", "cbKASUGA_Fan", "cbLoaderCassette", "cbNovx3360", "cbOCR",
    "cbOCRwithTester", "cbRotateSenAmpCnt", "cbSocketSenAmpCnt", "cbSocketSenAmpCnt2nd", "cbSocketSenAmpCnt3rd", "cbVibrationCardQty",
    "cbbATC1", "cbbATC2", "cbbATC3", "cbbATC4", "cbbComBinDisp2", "cbbGroundMan",
    "cbbRFIDReader", "cbbTrayStepMotor", "chk2x3modeUseAxisYSensor", "chkAuto1", "chkAuto1Y", "chkAuto2",
    "chkAuto2Y", "chkAuto3", "chkAuto3Y", "chkAuto4", "chkAuto5", "chkAuto6",
    "chkBiasModeUseYSensor", "chkColor", "chkColorY", "chkEmpty", "chkEmptyY", "chkLoader",
    "chkLoaderY", "chkOutShtYSensorByLatch", "chkUseHPComCard", "chkUserDefMaxContactHeight", "chkUser_Define_IndexZ_SafePos", "coCanBusNudn1",
    "coNudn1Macid11", "coNudn1Macid12", "coNudn1Macid13", "coNudn1Macid14", "ed24VMonitorPulseCount", "edATCSystemIP",
    "edATCSystemPort", "edATCSystemUseHeat", "edCognexSystemCCD", "edGroundMan_AlarmOhm", "edHotGunFlow_DevNo", "edHotGunFlow_Gun1_ChannelNo",
    "edHotGunFlow_Gun2_ChannelNo", "edHotGunFlow_LineNo", "edIONPulseCount", "edMaxKpa", "edMaxMpaFB", "edMinMpa",
    "edtBaseMaxTemp", "edtCustomerCode", "edtHPLimit", "edtIndexMaxTemp", "edtMaxXPitch", "edtMaxYPitch",
    "edtMinMpaFB", "edtMinXPitch", "edtMinYPitch", "edtOutShtMaxTemp", "edtOutSortXPitchMax", "edtOutSortXPitchMin",
    "edtSeriaNo", "edtTriTempTotalCh", "edtTriTemperature_MaxDegree", "edtTriTemperature_MinDegree", "edtUserDefMaxContactHeight", "edtUserDefineIndexZSafePos",
    "edt_Total_Compressor", "rg2DBarcode", "rg2ndLoader", "rg3M_EM_AWARE_Monitor", "rgAOI", "rgATC",
    "rgATCHeatGun", "rgATCMixMode", "rgAirConditioner", "rgAirStreamSelect", "rgAuto1ART", "rgAuto2ART",
    "rgAuto3ART", "rgAuto3Magazine", "rgAuto4ART", "rgAuto5ART", "rgAuto6ART", "rgAutoAlignment",
    "rgAutoCleanIonFan", "rgAutoShuttleSensor", "rgAutoTrackCanGoRear", "rgBaseHeaterCount", "rgBottom2DID", "rgBottom2DID_CCD",
    "rgCCDAutoAlignmentMode", "rgCCDTemp", "rgCKDFCM", "rgCOLORSENSOR_MUN", "rgCanBusMethod", "rgCatchTrayBlock",
    "rgCatchTrayModel", "rgChamberUsePulseType", "rgCleanAir", "rgColorSensor", "rgCoverTrayID", "rgDeviceFlipper",
    "rgDewpointHW", "rgDieClean", "rgDoubleBeltMode", "rgDoubleEPControl", "rgE84Sensor", "rgESDTemp",
    "rgEmptyKeyence", "rgFinePitch", "rgFix3FullPlace", "rgFixAICCD", "rgGroundMan", "rgGroundMan_ScanPoint",
    "rgHTIonBar", "rgHeatGun", "rgHeater", "rgHeaterType", "rgHighTempLimit", "rgHotGunFlow",
    "rgHotPlateLimit", "rgHotPlatePos", "rgHotplateType", "rgIOCard", "rgIOChangeToque", "rgInOutArmXPitch",
    "rgInOutArmYPitch", "rgInShtAutoLatch", "rgInShtLastSensor", "rgIndexMotionCard", "rgIndexMotorAxis", "rgIndexMotorType",
    "rgIndexPressType", "rgIndexSuckerType", "rgInstallARTtwocylinder", "rgInstallAutoRestest", "rgInstallFix3", "rgIonFanType",
    "rgKasuga", "rgKasuga_Fan", "rgLBTemp", "rgLBTemp2", "rgLDCarRotAtm", "rgLaserDistance",
    "rgLdUldCassetteMode", "rgLdUldUseStepMotor", "rgLoaderHinge", "rgLoaderKeyence", "rgLoaderVibration", "rgMNetSpeed",
    "rgMRSystem", "rgMagBinDispType", "rgMagneticScale", "rgModel", "rgMotionCard", "rgMultileEmpty",
    "rgMultileEmptyTrayIDKeyence", "rgNUECType", "rgNovx3360", "rgNumberPanelType", "rgOCR", "rgOCRYStepMot",
    "rgOHTSystem", "rgOTDInstall", "rgOutArmYPitch", "rgOutSortArm", "rgPickerCount", "rgPreciser",
    "rgPreciserPos", "rgRFIDReader", "rgRFIDSystem", "rgRealTimeCCD", "rgRealTimeCCDTempNum", "rgRotateKit",
    "rgRotateKitIn", "rgRotateKitOut", "rgRotateKit_Type", "rgSafeDoor", "rgSafeDoorLock", "rgSafePlcIO",
    "rgScanner_AOI", "rgShuttleCrossSensor", "rgShuttleFloating", "rgShuttleFloodgate", "rgShuttleSensor", "rgShuttleVibration",
    "rgShuttleZType", "rgSocketClamp", "rgSocketSen", "rgTTLCard", "rgTTLUseAddress", "rgTopScanner_AOI",
    "rgTrayArmMode", "rgTrayArmType", "rgTrayMapping", "rgTrayMappingGrabImage", "rgTrayRobot", "rgTrayVibration",
    "rgTriTempMachine", "rgUse4DUT", "rgUseOutSht", "rgUsePulseType", "rgUseSensor_9046_DB", "rgUseSucker_9046_DB",
    "rgVacuUnitType", "rgVibrationCommuncation", "rgWeightCali", "rg_ControlPanelMode", "rg_IndexDoorHeater",
};

// 存檔流程讀的替身，開機時預先建立（FileRW_IniConfig_Boot 呼叫）
static void HS_CreateSaveProxies()
{
    EL<TRadioGroup>("THandlerSystem", "ArmZAtoZH");
    EL<TRadioGroup>("THandlerSystem", "Auto2SelectCy");
    EL<TRadioGroup>("THandlerSystem", "ElectronPressure");
    EL<TRadioGroup>("THandlerSystem", "MachineTrack");
    EL<TComboBox>("THandlerSystem", "cb2DReader1");
    EL<TComboBox>("THandlerSystem", "cb2DReader2");
    EL<TComboBox>("THandlerSystem", "cb2DReader3");
    EL<TComboBox>("THandlerSystem", "cb2DReader4");
    EL<TComboBox>("THandlerSystem", "cbAirCon");
    EL<TCheckBox>("THandlerSystem", "cbAuto1Cassette");
    EL<TCheckBox>("THandlerSystem", "cbAuto2Cassette");
    EL<TCheckBox>("THandlerSystem", "cbAuto3Cassette");
    EL<TCheckBox>("THandlerSystem", "cbAuto4Cassette");
    EL<TCheckBox>("THandlerSystem", "cbAuto5Cassette");
    EL<TCheckBox>("THandlerSystem", "cbAuto6Cassette");
    EL<TCheckBox>("THandlerSystem", "cbColorCassette");
    EL<TComboBox>("THandlerSystem", "cbColorSenAmpCnt");
    EL<TComboBox>("THandlerSystem", "cbComBinDisp");
    EL<TComboBox>("THandlerSystem", "cbComDyTemp");
    EL<TComboBox>("THandlerSystem", "cbComIndex");
    EL<TComboBox>("THandlerSystem", "cbComLaser1");
    EL<TComboBox>("THandlerSystem", "cbComLaser2");
    EL<TComboBox>("THandlerSystem", "cbComLaserInArm");
    EL<TComboBox>("THandlerSystem", "cbComRTC");
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232");
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232_2");
    EL<TComboBox>("THandlerSystem", "cbComTemp");
    EL<TComboBox>("THandlerSystem", "cbComTempOmron");
    EL<TComboBox>("THandlerSystem", "cbComTester");
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort1");
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort2");
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort3");
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort4");
    EL<TCheckBox>("THandlerSystem", "cbESDUse4COM");
    EL<TCheckBox>("THandlerSystem", "cbEmptyCassette");
    EL<TCheckBox>("THandlerSystem", "cbEnableOutShtSensor");
    EL<TComboBox>("THandlerSystem", "cbHandlerModel");
    EL<TComboBox>("THandlerSystem", "cbIndEPCnt");
    EL<TComboBox>("THandlerSystem", "cbKASUGA_Fan");
    EL<TCheckBox>("THandlerSystem", "cbLoaderCassette");
    EL<TComboBox>("THandlerSystem", "cbNovx3360");
    EL<TComboBox>("THandlerSystem", "cbOCR");
    EL<TComboBox>("THandlerSystem", "cbOCRwithTester");
    EL<TComboBox>("THandlerSystem", "cbRotateSenAmpCnt");
    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt");
    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt2nd");
    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt3rd");
    EL<TComboBox>("THandlerSystem", "cbVibrationCardQty");
    EL<TComboBox>("THandlerSystem", "cbbATC1");
    EL<TComboBox>("THandlerSystem", "cbbATC2");
    EL<TComboBox>("THandlerSystem", "cbbATC3");
    EL<TComboBox>("THandlerSystem", "cbbATC4");
    EL<TComboBox>("THandlerSystem", "cbbComBinDisp2");
    EL<TComboBox>("THandlerSystem", "cbbGroundMan");
    EL<TComboBox>("THandlerSystem", "cbbRFIDReader");
    EL<TComboBox>("THandlerSystem", "cbbTrayStepMotor");
    EL<TCheckBox>("THandlerSystem", "chk2x3modeUseAxisYSensor");
    EL<TCheckBox>("THandlerSystem", "chkAuto1");
    EL<TCheckBox>("THandlerSystem", "chkAuto1Y");
    EL<TCheckBox>("THandlerSystem", "chkAuto2");
    EL<TCheckBox>("THandlerSystem", "chkAuto2Y");
    EL<TCheckBox>("THandlerSystem", "chkAuto3");
    EL<TCheckBox>("THandlerSystem", "chkAuto3Y");
    EL<TCheckBox>("THandlerSystem", "chkAuto4");
    EL<TCheckBox>("THandlerSystem", "chkAuto5");
    EL<TCheckBox>("THandlerSystem", "chkAuto6");
    EL<TCheckBox>("THandlerSystem", "chkBiasModeUseYSensor");
    EL<TCheckBox>("THandlerSystem", "chkColor");
    EL<TCheckBox>("THandlerSystem", "chkColorY");
    EL<TCheckBox>("THandlerSystem", "chkEmpty");
    EL<TCheckBox>("THandlerSystem", "chkEmptyY");
    EL<TCheckBox>("THandlerSystem", "chkLoader");
    EL<TCheckBox>("THandlerSystem", "chkLoaderY");
    EL<TCheckBox>("THandlerSystem", "chkOutShtYSensorByLatch");
    EL<TCheckBox>("THandlerSystem", "chkUseHPComCard");
    EL<TCheckBox>("THandlerSystem", "chkUserDefMaxContactHeight");
    EL<TCheckBox>("THandlerSystem", "chkUser_Define_IndexZ_SafePos");
    EL<TComboBox>("THandlerSystem", "coCanBusNudn1");
    EL<TComboBox>("THandlerSystem", "coNudn1Macid11");
    EL<TComboBox>("THandlerSystem", "coNudn1Macid12");
    EL<TComboBox>("THandlerSystem", "coNudn1Macid13");
    EL<TComboBox>("THandlerSystem", "coNudn1Macid14");
    EL<TEdit>("THandlerSystem", "ed24VMonitorPulseCount");
    EL<TEdit>("THandlerSystem", "edATCSystemIP");
    EL<TEdit>("THandlerSystem", "edATCSystemPort");
    EL<TEdit>("THandlerSystem", "edATCSystemUseHeat");
    EL<TEdit>("THandlerSystem", "edCognexSystemCCD");
    EL<TEdit>("THandlerSystem", "edGroundMan_AlarmOhm");
    EL<TEdit>("THandlerSystem", "edHotGunFlow_DevNo");
    EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun1_ChannelNo");
    EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun2_ChannelNo");
    EL<TEdit>("THandlerSystem", "edHotGunFlow_LineNo");
    EL<TEdit>("THandlerSystem", "edIONPulseCount");
    EL<TEdit>("THandlerSystem", "edMaxKpa");
    EL<TEdit>("THandlerSystem", "edMaxMpaFB");
    EL<TEdit>("THandlerSystem", "edMinMpa");
    EL<TEdit>("THandlerSystem", "edtBaseMaxTemp");
    EL<TEdit>("THandlerSystem", "edtCustomerCode");
    EL<TEdit>("THandlerSystem", "edtHPLimit");
    EL<TEdit>("THandlerSystem", "edtIndexMaxTemp");
    EL<TEdit>("THandlerSystem", "edtMaxXPitch");
    EL<TEdit>("THandlerSystem", "edtMaxYPitch");
    EL<TEdit>("THandlerSystem", "edtMinMpaFB");
    EL<TEdit>("THandlerSystem", "edtMinXPitch");
    EL<TEdit>("THandlerSystem", "edtMinYPitch");
    EL<TEdit>("THandlerSystem", "edtOutShtMaxTemp");
    EL<TEdit>("THandlerSystem", "edtOutSortXPitchMax");
    EL<TEdit>("THandlerSystem", "edtOutSortXPitchMin");
    EL<TEdit>("THandlerSystem", "edtSeriaNo");
    EL<TEdit>("THandlerSystem", "edtTriTempTotalCh");
    EL<TEdit>("THandlerSystem", "edtTriTemperature_MaxDegree");
    EL<TEdit>("THandlerSystem", "edtTriTemperature_MinDegree");
    EL<TEdit>("THandlerSystem", "edtUserDefMaxContactHeight");
    EL<TEdit>("THandlerSystem", "edtUserDefineIndexZSafePos");
    EL<TEdit>("THandlerSystem", "edt_Total_Compressor");
    EL<TRadioGroup>("THandlerSystem", "rg2DBarcode");
    EL<TRadioGroup>("THandlerSystem", "rg2ndLoader");
    EL<TRadioGroup>("THandlerSystem", "rg3M_EM_AWARE_Monitor");
    EL<TRadioGroup>("THandlerSystem", "rgAOI");
    EL<TRadioGroup>("THandlerSystem", "rgATC");
    EL<TRadioGroup>("THandlerSystem", "rgATCHeatGun");
    EL<TRadioGroup>("THandlerSystem", "rgATCMixMode");
    EL<TRadioGroup>("THandlerSystem", "rgAirConditioner");
    EL<TRadioGroup>("THandlerSystem", "rgAirStreamSelect");
    EL<TRadioGroup>("THandlerSystem", "rgAuto1ART");
    EL<TRadioGroup>("THandlerSystem", "rgAuto2ART");
    EL<TRadioGroup>("THandlerSystem", "rgAuto3ART");
    EL<TRadioGroup>("THandlerSystem", "rgAuto3Magazine");
    EL<TRadioGroup>("THandlerSystem", "rgAuto4ART");
    EL<TRadioGroup>("THandlerSystem", "rgAuto5ART");
    EL<TRadioGroup>("THandlerSystem", "rgAuto6ART");
    EL<TRadioGroup>("THandlerSystem", "rgAutoAlignment");
    EL<TRadioGroup>("THandlerSystem", "rgAutoCleanIonFan");
    EL<TRadioGroup>("THandlerSystem", "rgAutoShuttleSensor");
    EL<TRadioGroup>("THandlerSystem", "rgAutoTrackCanGoRear");
    EL<TRadioGroup>("THandlerSystem", "rgBaseHeaterCount");
    EL<TRadioGroup>("THandlerSystem", "rgBottom2DID");
    EL<TRadioGroup>("THandlerSystem", "rgBottom2DID_CCD");
    EL<TRadioGroup>("THandlerSystem", "rgCCDAutoAlignmentMode");
    EL<TRadioGroup>("THandlerSystem", "rgCCDTemp");
    EL<TRadioGroup>("THandlerSystem", "rgCKDFCM");
    EL<TRadioGroup>("THandlerSystem", "rgCOLORSENSOR_MUN");
    EL<TRadioGroup>("THandlerSystem", "rgCanBusMethod");
    EL<TRadioGroup>("THandlerSystem", "rgCatchTrayBlock");
    EL<TRadioGroup>("THandlerSystem", "rgCatchTrayModel");
    EL<TRadioGroup>("THandlerSystem", "rgChamberUsePulseType");
    EL<TRadioGroup>("THandlerSystem", "rgCleanAir");
    EL<TRadioGroup>("THandlerSystem", "rgColorSensor");
    EL<TRadioGroup>("THandlerSystem", "rgCoverTrayID");
    EL<TRadioGroup>("THandlerSystem", "rgDeviceFlipper");
    EL<TRadioGroup>("THandlerSystem", "rgDewpointHW");
    EL<TRadioGroup>("THandlerSystem", "rgDieClean");
    EL<TRadioGroup>("THandlerSystem", "rgDoubleBeltMode");
    EL<TRadioGroup>("THandlerSystem", "rgDoubleEPControl");
    EL<TRadioGroup>("THandlerSystem", "rgE84Sensor");
    EL<TRadioGroup>("THandlerSystem", "rgESDTemp");
    EL<TRadioGroup>("THandlerSystem", "rgEmptyKeyence");
    EL<TRadioGroup>("THandlerSystem", "rgFinePitch");
    EL<TRadioGroup>("THandlerSystem", "rgFix3FullPlace");
    EL<TRadioGroup>("THandlerSystem", "rgFixAICCD");
    EL<TRadioGroup>("THandlerSystem", "rgGroundMan");
    EL<TRadioGroup>("THandlerSystem", "rgGroundMan_ScanPoint");
    EL<TRadioGroup>("THandlerSystem", "rgHTIonBar");
    EL<TRadioGroup>("THandlerSystem", "rgHeatGun");
    EL<TRadioGroup>("THandlerSystem", "rgHeater");
    EL<TRadioGroup>("THandlerSystem", "rgHeaterType");
    EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit");
    EL<TRadioGroup>("THandlerSystem", "rgHotGunFlow");
    EL<TRadioGroup>("THandlerSystem", "rgHotPlateLimit");
    EL<TRadioGroup>("THandlerSystem", "rgHotPlatePos");
    EL<TRadioGroup>("THandlerSystem", "rgHotplateType");
    EL<TRadioGroup>("THandlerSystem", "rgIOCard");
    EL<TRadioGroup>("THandlerSystem", "rgIOChangeToque");
    EL<TRadioGroup>("THandlerSystem", "rgInOutArmXPitch");
    EL<TRadioGroup>("THandlerSystem", "rgInOutArmYPitch");
    EL<TRadioGroup>("THandlerSystem", "rgInShtAutoLatch");
    EL<TRadioGroup>("THandlerSystem", "rgInShtLastSensor");
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotionCard");
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorAxis");
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorType");
    EL<TRadioGroup>("THandlerSystem", "rgIndexPressType");
    EL<TRadioGroup>("THandlerSystem", "rgIndexSuckerType");
    EL<TRadioGroup>("THandlerSystem", "rgInstallARTtwocylinder");
    EL<TRadioGroup>("THandlerSystem", "rgInstallAutoRestest");
    EL<TRadioGroup>("THandlerSystem", "rgInstallFix3");
    EL<TRadioGroup>("THandlerSystem", "rgIonFanType");
    EL<TRadioGroup>("THandlerSystem", "rgKasuga");
    EL<TRadioGroup>("THandlerSystem", "rgKasuga_Fan");
    EL<TRadioGroup>("THandlerSystem", "rgLBTemp");
    EL<TRadioGroup>("THandlerSystem", "rgLBTemp2");
    EL<TRadioGroup>("THandlerSystem", "rgLDCarRotAtm");
    EL<TRadioGroup>("THandlerSystem", "rgLaserDistance");
    EL<TRadioGroup>("THandlerSystem", "rgLdUldCassetteMode");
    EL<TRadioGroup>("THandlerSystem", "rgLdUldUseStepMotor");
    EL<TRadioGroup>("THandlerSystem", "rgLoaderHinge");
    EL<TRadioGroup>("THandlerSystem", "rgLoaderKeyence");
    EL<TRadioGroup>("THandlerSystem", "rgLoaderVibration");
    EL<TRadioGroup>("THandlerSystem", "rgMNetSpeed");
    EL<TRadioGroup>("THandlerSystem", "rgMRSystem");
    EL<TRadioGroup>("THandlerSystem", "rgMagBinDispType");
    EL<TRadioGroup>("THandlerSystem", "rgMagneticScale");
    EL<TRadioGroup>("THandlerSystem", "rgModel");
    EL<TRadioGroup>("THandlerSystem", "rgMotionCard");
    EL<TRadioGroup>("THandlerSystem", "rgMultileEmpty");
    EL<TRadioGroup>("THandlerSystem", "rgMultileEmptyTrayIDKeyence");
    EL<TRadioGroup>("THandlerSystem", "rgNUECType");
    EL<TRadioGroup>("THandlerSystem", "rgNovx3360");
    EL<TRadioGroup>("THandlerSystem", "rgNumberPanelType");
    EL<TRadioGroup>("THandlerSystem", "rgOCR");
    EL<TRadioGroup>("THandlerSystem", "rgOCRYStepMot");
    EL<TRadioGroup>("THandlerSystem", "rgOHTSystem");
    EL<TRadioGroup>("THandlerSystem", "rgOTDInstall");
    EL<TRadioGroup>("THandlerSystem", "rgOutArmYPitch");
    EL<TRadioGroup>("THandlerSystem", "rgOutSortArm");
    EL<TRadioGroup>("THandlerSystem", "rgPickerCount");
    EL<TRadioGroup>("THandlerSystem", "rgPreciser");
    EL<TRadioGroup>("THandlerSystem", "rgPreciserPos");
    EL<TRadioGroup>("THandlerSystem", "rgRFIDReader");
    EL<TRadioGroup>("THandlerSystem", "rgRFIDSystem");
    EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCD");
    EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCDTempNum");
    EL<TRadioGroup>("THandlerSystem", "rgRotateKit");
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn");
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut");
    EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type");
    EL<TRadioGroup>("THandlerSystem", "rgSafeDoor");
    EL<TRadioGroup>("THandlerSystem", "rgSafeDoorLock");
    EL<TRadioGroup>("THandlerSystem", "rgSafePlcIO");
    EL<TRadioGroup>("THandlerSystem", "rgScanner_AOI");
    EL<TRadioGroup>("THandlerSystem", "rgShuttleCrossSensor");
    EL<TRadioGroup>("THandlerSystem", "rgShuttleFloating");
    EL<TRadioGroup>("THandlerSystem", "rgShuttleFloodgate");
    EL<TRadioGroup>("THandlerSystem", "rgShuttleSensor");
    EL<TRadioGroup>("THandlerSystem", "rgShuttleVibration");
    EL<TRadioGroup>("THandlerSystem", "rgShuttleZType");
    EL<TRadioGroup>("THandlerSystem", "rgSocketClamp");
    EL<TRadioGroup>("THandlerSystem", "rgSocketSen");
    EL<TRadioGroup>("THandlerSystem", "rgTTLCard");
    EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress");
    EL<TRadioGroup>("THandlerSystem", "rgTopScanner_AOI");
    EL<TRadioGroup>("THandlerSystem", "rgTrayArmMode");
    EL<TRadioGroup>("THandlerSystem", "rgTrayArmType");
    EL<TRadioGroup>("THandlerSystem", "rgTrayMapping");
    EL<TRadioGroup>("THandlerSystem", "rgTrayMappingGrabImage");
    EL<TRadioGroup>("THandlerSystem", "rgTrayRobot");
    EL<TRadioGroup>("THandlerSystem", "rgTrayVibration");
    EL<TRadioGroup>("THandlerSystem", "rgTriTempMachine");
    EL<TRadioGroup>("THandlerSystem", "rgUse4DUT");
    EL<TRadioGroup>("THandlerSystem", "rgUseOutSht");
    EL<TRadioGroup>("THandlerSystem", "rgUsePulseType");
    EL<TRadioGroup>("THandlerSystem", "rgUseSensor_9046_DB");
    EL<TRadioGroup>("THandlerSystem", "rgUseSucker_9046_DB");
    EL<TRadioGroup>("THandlerSystem", "rgVacuUnitType");
    EL<TRadioGroup>("THandlerSystem", "rgVibrationCommuncation");
    EL<TRadioGroup>("THandlerSystem", "rgWeightCali");
    EL<TRadioGroup>("THandlerSystem", "rg_ControlPanelMode");
    EL<TRadioGroup>("THandlerSystem", "rg_IndexDoorHeater");
}

// golden HandlerSys.cpp:63  THandlerSystem::THandlerSystem(TComponent* Owner)
static void HS_THandlerSystem()
{

    slCustomerCode=new TStringList(); for(int i=0; i<EL<TRadioGroup>("THandlerSystem", "rgCustomerList")->Items->Count; i++) slCustomerCode->Add(EL<TRadioGroup>("THandlerSystem", "rgCustomerList")->Items->Strings[i]);   // golden HandlerSys.cpp:66-68 取代：slCustomerCode 是 TStringList 成員（不是 widget），但 golden header 的宣告形狀與 widget 相同，產生器會把它改成替身 —— 等價取代成 static 成員
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:66-68 -- slCustomerCode 是 TStringList 成員（不是 widget），但 golden header 的宣告形狀與 widget 相同，產生器會把它改成替身 —— 等價取代成 static 成員
    EL<TControl>("THandlerSystem", "slCustomerCode")=new TStringList();
    for(int i=0; i<EL<TRadioGroup>("THandlerSystem", "rgCustomerList")->Items->Count; i++)
        EL<TControl>("THandlerSystem", "slCustomerCode")->Add(EL<TRadioGroup>("THandlerSystem", "rgCustomerList")->Items->Strings[i]);
#endif // GATE (S12-C save)

    ;   // golden HandlerSys.cpp:70-112 取代：EN_HEATER_SHEET=1（golden 912 MachineType.h:658）的逐站溫控 ComboBox 動態建立：移植樹沒有 EN_HEATER_SHEET 也沒有 g_tHeaterInsInfo，照抄的話 `#if EN_HEATER_SHEET` 在移植樹默默變成空；開機沒有 session，待辦改在 LoaderSystemSet :262 回報
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:70-112 -- EN_HEATER_SHEET=1（golden 912 MachineType.h:658）的逐站溫控 ComboBox 動態建立：移植樹沒有 EN_HEATER_SHEET 也沒有 g_tHeaterInsInfo，照抄的話 `#if EN_HEATER_SHEET` 在移植樹默默變成空；開機沒有 session，待辦改在 LoaderSystemSet :262 回報
#if EN_HEATER_SHEET
    g_vecHeaterTypeIdxForShow.clear();
    g_mapHeaterTypeIdxToHeaterShowIdx.clear();
    g_mapHeaterShowIdxToHeaterTypeIdx.clear();
    {
        int iShowCount = 0;
        for(int i = 0; i<eHeaterType_Count; ++i)
        {
            if(g_tHeaterInsInfo[i].GetOccupy())
            {
                g_mapHeaterTypeIdxToHeaterShowIdx[i] = iShowCount;
                g_mapHeaterShowIdxToHeaterTypeIdx[iShowCount] = i;
                g_vecHeaterTypeIdxForShow.push_back(i);
                iShowCount++;
            }
            else
                g_mapHeaterTypeIdxToHeaterShowIdx[i] = INVALID_INT_VAL_NEG;
        }
        for(int i = 0; i<(int)g_vecHeaterTypeIdxForShow.size(); ++i)
        {
            int iTypeIdx = g_vecHeaterTypeIdxForShow[i];
            int iHorIdx  = i / COUNT_OF_ITEM_IN_COL;
            int iVerIdx  = i % COUNT_OF_ITEM_IN_COL;
            TLabel    *pLb = new TLabel(this);
            TComboBox *pCb = new TComboBox(this);
            pLb->Parent = EL<TGroupBox>("THandlerSystem", "grpHeater");
            pCb->Parent = EL<TGroupBox>("THandlerSystem", "grpHeater");
            pLb->Width = WIDTH_OF_LB_HEATER; pLb->Height = HEIGHT_OF_LB_HEATER;
            pCb->Width = WIDTH_OF_CB_HEATER; pCb->Height = HEIGHT_OF_CB_HEATER;
            pLb->Top  = TOP_OF_LB + iVerIdx*(HEIGHT_OF_LB_HEATER + GAP_OF_VER);
            pCb->Top  = TOP_OF_CB + iVerIdx*(HEIGHT_OF_CB_HEATER + GAP_OF_VER);
            pLb->Left = LEFT_OF_1ST + iHorIdx*(WIDTH_OF_LB_HEATER + GAP_OF_HOR_IN_SET + WIDTH_OF_CB_HEATER + GAP_OF_HOR_OUT_SET);
            pCb->Left = pLb->Left + (WIDTH_OF_LB_HEATER + GAP_OF_HOR_IN_SET);
            pLb->Caption = g_tHeaterInsInfo[iTypeIdx].GetShowName();
            for(int j = 0; j<eHeaterInsOpt_Count; ++j)
                pCb->Items->Add(g_HeaterInsOptStr[j]);
            pCb->ItemIndex = g_tHeaterInsInfo[iTypeIdx].GetHeaterInsOpt();   //fix: ref used i, must be iTypeIdx
            pCb->OnClick = cbHeaterInsOptChange_Base;
            pCb->Tag = i;
            g_tHeaterInsInfo[iTypeIdx].SetCtrlItemProp(GetCtrlItemVisProp(iTypeIdx), pLb, pCb);
        }
    }
#endif
#endif // GATE (S12-C save)

}

// golden HandlerSys.cpp:132  THandlerSystem::FormShow(TObject *Sender)
static void HS_FormShow()
{

    ;   // golden HandlerSys.cpp:134-134 取代：SortItemToMap()：功能搜尋框的元件索引（TempComp，edtSearchFunctionChange 搬 Parent），純畫面
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:134-134 -- SortItemToMap()：功能搜尋框的元件索引（TempComp，edtSearchFunctionChange 搬 Parent），純畫面
    SortItemToMap();
#endif // GATE (S12-C save)
    HS_LoaderSystemSet();
    HS_LoaderSafeDoorSet();
//    if(CUSTOMER_CODE!=0)                                                      //Steven 20131001 : Lee要求不准有關安全門的功能
//    {
        EL<TGroupBox>("THandlerSystem", "GroupBox1")->Visible=false;
        EL<TGroupBox>("THandlerSystem", "GroupBox2")->Visible=false;
//    }
//    else
//    {
//        GroupBox1->Visible=true;
//        GroupBox2->Visible=true;
//    }

    EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->Enabled  =(EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex == 0);
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->Enabled =(EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex == 0);
    if(CosFunction.bUserDefineIndexZSafePos==true)                              //Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
        EL<TEdit>("THandlerSystem", "edtUserDefineIndexZSafePos")->Enabled=true;
    else
        EL<TEdit>("THandlerSystem", "edtUserDefineIndexZSafePos")->Enabled=false;
    EL<TPageControl>("THandlerSystem", "pcSetting")->ActivePageIndex=0;
    EL<TTabSheet>("THandlerSystem", "tsCustomerCode")->TabVisible=false;

    AnsiString Str="D:\\GPIB9045\\system\\general.ini";                         //Steven 20140708 : 左左右加入機種的選擇
    AnsiString Str2=CheckAndReadIniData(Str, "Version", "Model", AnsiString("HT-9045W"));

    if(Str2=="9045GPIB")             EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=0;
    else if(Str2=="9046GPIB")        EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=1;
    else if(Str2=="9045GPIB_12Site") EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=2;
    else if(Str2=="9046_32GPIB")     EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=3;
    else if(Str2=="502GPIB")         EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=4;
    else if(Str2=="1032GPIB")        EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=5;
    else if(Str2=="7080GPIB")        EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex=6;

    if(EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex==0)                                            //Steven 20190307 : Add for HT-9046LA
        EL<TRadioGroup>("THandlerSystem", "rgModel")->ItemIndex=Type_None;
    else
        EL<TRadioGroup>("THandlerSystem", "rgModel")->ItemIndex=CheckAndReadIniDataGeneral("Version", "SubModel", Type_None);

    //這一行請保持在最下面!!-----------------
    filerw::ELTodo("golden HandlerSys.cpp:174 myLog.Do_Log(Sender, asUser, asLogPath) (per-control change log) not done");   // golden HandlerSys.cpp:174-174 取代：myLog.Do_Log(Sender, asUser, asLogPath)：TMyLog 逐一比對表單控制項記錄使用者改了什麼（伺服器端沒有表單物件）
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:174-174 -- myLog.Do_Log(Sender, asUser, asLogPath)：TMyLog 逐一比對表單控制項記錄使用者改了什麼（伺服器端沒有表單物件）
    myLog.Do_Log(Sender, asUser, asLogPath);                                    //Steven 20100629
#endif // GATE (S12-C save)

}

// golden HandlerSys.cpp:177  THandlerSystem::LoaderSystemSet()
static void HS_LoaderSystemSet()
{

//    SUPPORT_2_EMPTY_EMPTY  =CheckAndReadIniDataGeneral("System", "SUPPORT_2_EMPTY_EMPTY", 0); //RogerYang 20250823:搬去ReadGeneralIni()  //Steven 20230907 : For HT-9011UC

    EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex=CheckAndReadIniDataGeneral("System", "AUTO_EMPTY_COLOR", EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex);
    if(SUPPORT_2_EMPTY_EMPTY)
    {
        EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex=2;                                              //7 Track (Empty Unloader)
    }

    EL<TRadioGroup>("THandlerSystem", "ElectronPressure")->ItemIndex=EP_Install;

    EL<TCheckBox>("THandlerSystem", "chkLoader")->Checked=LOAD_Z_USE_MOTOR[0];                                     //Steven 20190813 : 入Tray改用步進馬達
    EL<TCheckBox>("THandlerSystem", "chkEmpty")->Checked =LOAD_Z_USE_MOTOR[1];
    EL<TCheckBox>("THandlerSystem", "chkColor")->Checked =LOAD_Z_USE_MOTOR[2];
    EL<TCheckBox>("THandlerSystem", "chkAuto1")->Checked =LOAD_Z_USE_MOTOR[3];
    EL<TCheckBox>("THandlerSystem", "chkAuto2")->Checked =LOAD_Z_USE_MOTOR[4];
    EL<TCheckBox>("THandlerSystem", "chkAuto3")->Checked =LOAD_Z_USE_MOTOR[5];
    EL<TCheckBox>("THandlerSystem", "chkAuto4")->Checked =LOAD_Z_USE_MOTOR[6];                                     //Steven 20230907 : For HT-9011UC
    EL<TCheckBox>("THandlerSystem", "chkAuto5")->Checked =LOAD_Z_USE_MOTOR[7];
    EL<TCheckBox>("THandlerSystem", "chkAuto6")->Checked =LOAD_Z_USE_MOTOR[8];
    EL<TCheckBox>("THandlerSystem", "chkLoaderY")->Checked=LOAD_Y_USE_MOTOR[0];                                    //Jimmychiu 20240307 : Loader Tray改用步進馬達
    EL<TCheckBox>("THandlerSystem", "chkEmptyY")->Checked =LOAD_Y_USE_MOTOR[1];                                    //AI(ht9045-v899) 20260423: load empty y stepper checkbox
    EL<TCheckBox>("THandlerSystem", "chkColorY")->Checked =LOAD_Y_USE_MOTOR[2];                                    //AI(ht9045-v899) 20260423: load color y stepper checkbox
    EL<TCheckBox>("THandlerSystem", "chkAuto1Y")->Checked =LOAD_Y_USE_MOTOR[3];                                    //AI(ht9045-v899) 20260423: load auto1 y stepper checkbox
    EL<TCheckBox>("THandlerSystem", "chkAuto2Y")->Checked =LOAD_Y_USE_MOTOR[4];                                    //AI(ht9045-v899) 20260423: load auto2 y stepper checkbox
    EL<TCheckBox>("THandlerSystem", "chkAuto3Y")->Checked =LOAD_Y_USE_MOTOR[5];                                    //AI(ht9045-v899) 20260423: load auto3 y stepper checkbox

    EL<TCheckBox>("THandlerSystem", "cbLoaderCassette")->Checked=LOADUNLOAD_USE_CASSETTE[0];
    EL<TCheckBox>("THandlerSystem", "cbEmptyCassette")->Checked =LOADUNLOAD_USE_CASSETTE[1];
    EL<TCheckBox>("THandlerSystem", "cbColorCassette")->Checked =LOADUNLOAD_USE_CASSETTE[2];
    EL<TCheckBox>("THandlerSystem", "cbAuto1Cassette")->Checked =LOADUNLOAD_USE_CASSETTE[3];
    EL<TCheckBox>("THandlerSystem", "cbAuto2Cassette")->Checked =LOADUNLOAD_USE_CASSETTE[4];
    EL<TCheckBox>("THandlerSystem", "cbAuto3Cassette")->Checked =LOADUNLOAD_USE_CASSETTE[5];
    EL<TCheckBox>("THandlerSystem", "cbAuto4Cassette")->Checked =LOADUNLOAD_USE_CASSETTE[6];                       //Steven 20230907 : For HT-9011UC
    EL<TCheckBox>("THandlerSystem", "cbAuto5Cassette")->Checked =LOADUNLOAD_USE_CASSETTE[7];
    EL<TCheckBox>("THandlerSystem", "cbAuto6Cassette")->Checked =LOADUNLOAD_USE_CASSETTE[8];

    EL<TRadioGroup>("THandlerSystem", "ArmZAtoZH")->ItemIndex=InOutArmPickerUseMotor;

    EL<TRadioGroup>("THandlerSystem", "rgNumberPanelType")->ItemIndex=NUMBER_PANEL_TYPE;
    EL<TRadioGroup>("THandlerSystem", "rgIonFanType")->ItemIndex=ION_FAN_TYPE;
    EL<TRadioGroup>("THandlerSystem", "rgShuttleSensor")->ItemIndex=SHUTTLE_SENSOR_TYPE;
    EL<TRadioGroup>("THandlerSystem", "rgNUECType")->ItemIndex=NUEC_TYPE;                                            //Sam 20230707 : EtherCAT Shuttle sensor

    EL<TCheckBox>("THandlerSystem", "cbEnableOutShtSensor")->Checked=ENABLE_OUT_SHUTTLE_SENEOR;                    //Steven 20120531 : by pass out shuttle sensor
    EL<TCheckBox>("THandlerSystem", "chkOutShtYSensorByLatch")->Checked=ENABLE_OUT_SHUTTLEY_LATCH;                 //Steven 20170518 (jou) : Out shuttle Y sensor 使用 LATCH
    EL<TCheckBox>("THandlerSystem", "chk2x3modeUseAxisYSensor")->Checked=Use_AxisY_Sensor_2x3mode;                 //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
    EL<TCheckBox>("THandlerSystem", "chkBiasModeUseYSensor")->Checked=Bias_Mode_Use_Y_Sensor;                      //Steven 20240805 : 1x2 Bias mode 用Y sensor 偵測Outshuttle殘料
    EL<TRadioGroup>("THandlerSystem", "rgSafeDoor")->ItemIndex=SAFE_DOOR_AMOUNT;
    EL<TRadioGroup>("THandlerSystem", "rgWeightCali")->ItemIndex=WEIGHT_CALIBRATION;                                 //Steven 20111202 : 要初始化
    EL<TEdit>("THandlerSystem", "edtCustomerCode")->Text=CheckAndReadIniDataGeneral("System", "CUSTOMER_CODE", 0);
    EL<TEdit>("THandlerSystem", "edtSeriaNo")->Text=CheckAndReadIniDataGeneral("Version", "Serial No", AnsiString("29818"));
    EL<TRadioGroup>("THandlerSystem", "Auto2SelectCy")->ItemIndex=bUseAuto2Empty;                                    //kevin 20120718 AUTO2有分離氣缸裝置
    EL<TRadioGroup>("THandlerSystem", "rgInstallAutoRestest")->ItemIndex=USE_AUTO_RETEST;                            //ChungHung 20140317 add Auto Retest
    EL<TRadioGroup>("THandlerSystem", "rgAuto1ART")->ItemIndex=UNLOADER_ART[eAuto1];                                 //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    EL<TRadioGroup>("THandlerSystem", "rgAuto2ART")->ItemIndex=UNLOADER_ART[eAuto2];                                 //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    EL<TRadioGroup>("THandlerSystem", "rgAuto3ART")->ItemIndex=UNLOADER_ART[eAuto3];                                 //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    EL<TRadioGroup>("THandlerSystem", "rgAuto4ART")->ItemIndex=UNLOADER_ART[eAuto4];                                 //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    EL<TRadioGroup>("THandlerSystem", "rgAuto5ART")->ItemIndex=UNLOADER_ART[eAuto5];                                 //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    EL<TRadioGroup>("THandlerSystem", "rgAuto6ART")->ItemIndex=UNLOADER_ART[eAuto6];                                 //Steven 20161221 (jou) : for SCK only Auto 2 has ART

//----------------------------------
    EL<TRadioGroup>("THandlerSystem", "rgIndexSuckerType")->ItemIndex=CheckAndReadIniDataGeneral("System", "INDEX_SUCKER_TYPE",  0);             //jou 2010-05-19 start : 負壓
    EL<TRadioGroup>("THandlerSystem", "rgIndexPressType")->ItemIndex =CheckAndReadIniDataGeneral("System", "INDEX_PRESS_TYPE",   e85KG);         //Steven 20110310 : 240KG
    EL<TCheckBox>("THandlerSystem", "chkUserDefMaxContactHeight")->Checked =CheckAndReadIniDataGeneral("Index", "EnableUserDefMaxContactHeight",   false);                 //Steven 20180412 : For new HT-9045高度較高
    EL<TEdit>("THandlerSystem", "edtUserDefMaxContactHeight")->Text    =CheckAndReadIniDataGeneral("Index", "UserDefMaxContactHeight",   AnsiString("-135.0"));        //Steven 20180412 : For new HT-9045高度較高

    EL<TCheckBox>("THandlerSystem", "chkUser_Define_IndexZ_SafePos")->Checked =CheckAndReadIniDataGeneral("Index", "EnableUser_Define_IndexZ_SafePos",   false);           //Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
    EL<TEdit>("THandlerSystem", "edtUserDefineIndexZSafePos")->Text       =CheckAndReadIniDataGeneral("Index", "UserDefineIndexZSafePos",   AnsiString("200"));        //Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)

    EL<TRadioGroup>("THandlerSystem", "rgTrayVibration")->ItemIndex  =CheckAndReadIniDataGeneral("System", "TRAY_VIBRATION",     0);             //Steven 20110627 : Tray Vibration
    EL<TRadioGroup>("THandlerSystem", "rgTrayArmMode")->ItemIndex    =CheckAndReadIniDataGeneral("System", "TRAY_ARM_MODE",      0);             //Frank 20230419
    EL<TRadioGroup>("THandlerSystem", "rg2ndLoader")->ItemIndex      =CheckAndReadIniDataGeneral("System", "USE_2nd_LOADER",     0);             //Steven 20240822 : For HT-9046AU
    EL<TRadioGroup>("THandlerSystem", "rgTrayRobot")->ItemIndex      =CheckAndReadIniDataGeneral("System", "USE_TRAY_ROBOT",     0);             //Steven 20170330 (Wei) : For HT-9046LM
    EL<TRadioGroup>("THandlerSystem", "rgLoaderHinge")->ItemIndex    =CheckAndReadIniDataGeneral("System", "USE_LOADER_HINGE",   0);             //Steven 20170330 (Wei) : For TSMC
    EL<TRadioGroup>("THandlerSystem", "rgDieClean")->ItemIndex       =CheckAndReadIniDataGeneral("System", "USE_DIE_CLEAN",      0);             //wei 20170418
    EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCD")->ItemIndex    =CheckAndReadIniDataGeneral("System", "REAL_TIME_CCD",      0);             //Steven 20110705 : Real Time CCD
    EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCDTempNum")->ItemIndex  =CheckAndReadIniDataGeneral("System", "RTC_TemperNumber",      0);     //Isaac 20201217 : RTC CCD增加第二組感溫
    EL<TRadioGroup>("THandlerSystem", "rgCCDTemp")->ItemIndex        =CheckAndReadIniDataGeneral("System", "CCD2_TEMPER",        0);             //Steven 20110705 : Real Time CCD
    EL<TRadioGroup>("THandlerSystem", "rgLBTemp")->ItemIndex         =CheckAndReadIniDataGeneral("System", "LB_TEMP",            0);             //Steven 20181023 : LB溫度
    EL<TRadioGroup>("THandlerSystem", "rgLBTemp2")->ItemIndex        =CheckAndReadIniDataGeneral("System", "LB_TEMP_UpDown",     0);             //Frank 20241231 : add
    EL<TRadioGroup>("THandlerSystem", "rgESDTemp")->ItemIndex        =CheckAndReadIniDataGeneral("System", "Index_ESDAir",       0);             //kevin 20200207 add index ESD temp
    EL<TRadioGroup>("THandlerSystem", "rgHeater")->ItemIndex         =CheckAndReadIniDataGeneral("System", "USE_16_HEATER",      eht4Heater);    //Steven 20111011 : 使用16個Heater

    filerw::ELTodo("golden HandlerSys.cpp:70-112 + :262-278 EN_HEATER_SHEET=1 (golden 912 MachineType.h:658) branch not in the port (no EN_HEATER_SHEET, no g_tHeaterInsInfo/HeaterInsOpt_Read): per-station heater brand ComboBoxes are not built/shown, per-station [TempCtrl] keys are not read (golden HeaterInsOpt_Read also back-fills them into Gerneral.ini), rgHeaterType uses the :271 fallback HEATER_CTRL_TYPE (golden :269 uses the common per-station value when all stations agree)"); EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex=CheckAndReadIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",KT4H);   // golden HandlerSys.cpp:262-278 取代：#if !EN_HEATER_SHEET：移植樹沒定義 EN_HEATER_SHEET ⇒ 照抄會默默走 golden 912 沒在跑的舊分支；改成明寫 golden :271 的回退值
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:262-278 -- #if !EN_HEATER_SHEET：移植樹沒定義 EN_HEATER_SHEET ⇒ 照抄會默默走 golden 912 沒在跑的舊分支；改成明寫 golden :271 的回退值
#if !EN_HEATER_SHEET                                                            //AI(ht9045-heater-control) 20260618 (RogerYang) : 移植創發2type溫控模組設定
    EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex     =CheckAndReadIniDataGeneral("TempCtrl", "HEATER_CTRL_TYPE", KT4H);
#else
    HeaterInsOpt_Read();
    s_bSuppressHeaterTypeEvent=true;                                            //AI(ht9045-heater-control) 20260619 (RogerYang) : 回填rgHeaterType前先擋事件，避免洗掉混廠牌設定
    { int iFirst=-1;
      if(IsAllSame_HeaterInsOpt(true,&iFirst) && 0<=iFirst && iFirst<eHeaterInsOpt_Count)
          EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex=iFirst;
      else
          EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex=CheckAndReadIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",KT4H);
      for(int s=0; s<(int)g_vecHeaterTypeIdxForShow.size(); ++s)
      { int ti=g_vecHeaterTypeIdxForShow[s];
        if(ti<0||ti>=eHeaterType_Count) continue;
        TComboBox *pCb=g_tHeaterInsInfo[ti].GetCtrlItem_Cb();
        if(NULL!=pCb) pCb->ItemIndex=g_tHeaterInsInfo[ti].GetHeaterInsOpt(); } }
    s_bSuppressHeaterTypeEvent=false;                                           //AI(ht9045-heater-control) 20260619 (RogerYang) : 回填結束恢復事件
#endif          //Steven 20141030 : 新增OMRON E5DC溫控器
#endif // GATE (S12-C save)
    EL<TRadioGroup>("THandlerSystem", "rgColorSensor")->ItemIndex    =CheckAndReadIniDataGeneral("System",  "USE_COLOR_TRAY_SENSOR", 0);         //20140903 wei colcr Tray
    EL<TRadioGroup>("THandlerSystem", "rgSocketSen")->ItemIndex      =CheckAndReadIniDataGeneral("System",  "USE_SOCKET_SENSOR", 0);             //JerryYang 20200327 Socket sensor改用硬體選項判斷

    EL<TRadioGroup>("THandlerSystem", "rgCanBusMethod")->ItemIndex    =CheckAndReadIniDataGeneral("System",  "Canbus_Method", 0);                //Sam 20210518 : 新增 CanBus 軟體配置

//    rgIndexCCD->ItemIndex       =CheckAndReadIniDataGeneral("System", "INDEX_CCD",      0);               //Ifor 20150720 : Index CCD //Ifor 20150728 Mark整合於RTC選項
//OCR-------------------------------
    EL<TRadioGroup>("THandlerSystem", "rgOCR")->ItemIndex            =CheckAndReadIniDataGeneral("System", "INSTALL_OCR",        eocrUninstal);  //Steven 20120716 : OCR
    EL<TComboBox>("THandlerSystem", "cbOCR")->Text                 =CheckAndReadIniDataGeneral("OCR",    "OCR_COM_PORT",       AnsiString("COM18"));
    EL<TComboBox>("THandlerSystem", "cbOCRwithTester")->Text       =CheckAndReadIniDataGeneral("OCR",    "OCRwithTester_COM_PORT",       AnsiString("COM19"));

    EL<TRadioGroup>("THandlerSystem", "rgOCRYStepMot")->ItemIndex    =CheckAndReadIniDataGeneral("System", "INSTALL_OCR_YMot",   eocrUninstal);  //Frank 20250214 add
//Barcode_2D------------------------
    EL<TRadioGroup>("THandlerSystem", "rg2DBarcode")->ItemIndex      =CheckAndReadIniDataGeneral("2D_BarCode", "BAR_CODE_INSTALL",  ebctUninstall);  //Steven 20120921 : Barcode_2D
    EL<TRadioGroup>("THandlerSystem", "rgBottom2DID")->ItemIndex     =CheckAndReadIniDataGeneral("2D_BarCode", "BOTTOM_2DID",       ebctUninstall);  //Steven 20190308 : Bottom 2D
    EL<TRadioGroup>("THandlerSystem", "rgShuttleFloating")->ItemIndex=CheckAndReadIniDataGeneral("2D_BarCode", "SHT_FLOATING_CHK",  ebctUninstall);  //Steven 20160920 : IC置偏檢查
    EL<TEdit>("THandlerSystem", "edCognexSystemCCD")->Text     =CheckRange(CheckAndReadIniDataGeneral("2D_BarCode", "BAR_CODE_USECOUNT", 4), 2, 4);  //Ifor 20181206 :add COGNEX SYSTEM CCD Count
    EL<TRadioGroup>("THandlerSystem", "rgBottom2DID_CCD")->ItemIndex =CheckAndReadIniDataGeneral("2D_BarCode", "BOTTOM_2DID_CCD",   ebctUninstall);  //KaiChen 20200513 : Bottom 2D 8CCD
//----------------------------------
    EL<TRadioGroup>("THandlerSystem", "rgInShtLastSensor")->ItemIndex=CheckAndReadIniDataGeneral("System", "IN_SHT_LAST_SENSOR",  ebctUninstall);    //Steven 20181203 : In Shuttle最後一個Sensor定義
    EL<TRadioGroup>("THandlerSystem", "rgSafeDoorLock")->ItemIndex   =CheckAndReadIniDataGeneral("System", "SAFE_DOOR_LOCK",     0);             //20111130 Dell
    EL<TRadioGroup>("THandlerSystem", "rgHeatGun")->ItemIndex        =CheckAndReadIniDataGeneral("System", "INSTALL_HEAT_GUN",   0);             //kevin 20120523 : 選擇熱風槍機構模式
    EL<TRadioGroup>("THandlerSystem", "rgATCHeatGun")->ItemIndex     =CheckAndReadIniDataGeneral("System", "INSTALL_ATC_HEAT_GUN",   0);         //JerryYang 20220408 : add for ATC3.5
    EL<TRadioGroup>("THandlerSystem", "rgMagBinDispType")->ItemIndex =CheckAndReadIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", 0);         //JerryYang 20220909 : add magazine

    EL<TRadioGroup>("THandlerSystem", "rgMotionCard")->ItemIndex     =CheckAndReadIniDataGeneral("System", "MOTION_CARD_TYPE",   0);             //Brian 20121015 : 選擇Motion Card 模式
    EL<TRadioGroup>("THandlerSystem", "rgMNetSpeed")->ItemIndex      =CheckAndReadIniDataGeneral("System", "MOTIONNET_SPEED",    COMMSPEED_20M); //Steven 20181122 : MNet速度要可以修改
    EL<TRadioGroup>("THandlerSystem", "rgIOCard")->ItemIndex         =CheckAndReadIniDataGeneral("System", "IO_CARD_TYPE",       0);             //Brian 20121015 : 選擇I/O Card 模式
    { const int v_=CheckAndReadIniDataGeneral("System", "TTL_CARD_TYPE",      0); const bool ch_=(EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex!=v_); EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex=v_; if(ch_) HS_rgTTLCardClick(); }   // golden HandlerSys.cpp:307-307 取代：VCL 程式設定 TRadioGroup::ItemIndex（值有變）會觸發 OnClick（dfm :442 OnClick=rgTTLCardClick）；vclcompat 替身不觸發事件 ⇒ 值有變時明呼叫
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:307-307 -- VCL 程式設定 TRadioGroup::ItemIndex（值有變）會觸發 OnClick（dfm :442 OnClick=rgTTLCardClick）；vclcompat 替身不觸發事件 ⇒ 值有變時明呼叫
    EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex        =CheckAndReadIniDataGeneral("System", "TTL_CARD_TYPE",      0);             //Steven 20121122 : 選擇TTL Card 模式
#endif // GATE (S12-C save)
    EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress")->ItemIndex  =CheckAndReadIniDataGeneral("System", "TTL_CARD_USE_ADDRESS", 0);           //Isaac 20210922 : 選擇TTL板子是否帶站別
    EL<TRadioGroup>("THandlerSystem", "rgHotPlatePos")->ItemIndex    =CheckAndReadIniDataGeneral("System", "HOT_PLATE_POSITION", 0);             //Steven 20140222 : Hot Plate Pin的位置
    EL<TRadioGroup>("THandlerSystem", "rgHotPlateLimit")->ItemIndex  =CheckAndReadIniDataGeneral("System", "HOT_PLATE_LIMITATION", 0);           //Steven 20151117 : Hot Plate Pin的極限位置, 可以跑2x2 8吸嘴
    EL<TRadioGroup>("THandlerSystem", "rgLaserDistance")->ItemIndex  =CheckAndReadIniDataGeneral("System", "USE_LASER_DISTANCE", 0);             //Steven 20140228 : 雷射測距功能
    EL<TRadioGroup>("THandlerSystem", "rgDeviceFlipper")->ItemIndex  =CheckAndReadIniDataGeneral("System", "USE_DEVICE_FLIPPER", 0);             //Frank 20210612 : Flipper Function
//----------------------------------
    EL<TRadioGroup>("THandlerSystem", "rgCCDAutoAlignmentMode")->ItemIndex   = CheckAndReadIniDataGeneral("System", "MACHINE_HAS_AUTO_ALIGNMENT_CCD", 0);    //ChungHung 20210113 add for Alignment CCD   //KenHsieh 20210813 : add CCD AUTO ALIGNMENT
//----------------------------------
//    rgFingerprintReaders->ItemIndex=tFingerInterface.bUseFingerprint==true?1:0;
//    rgFingerprintReaders->ItemIndex=CheckAndReadIniDataGeneral("System",  "USE_FINGER_PRINT", 0);//Steven 20190503 : 指紋辨識權限
//Comm Port-------------------------                                            //Steven 20120217 : Com Port改成可定義
    MyForceDirectories("D:\\RS232Standard\\System");
    AnsiString Str="D:\\RS232Standard\\System\\Setup.ini";

    EL<TComboBox>("THandlerSystem", "cbComIndex")->Text            =CheckAndReadIniDataGeneral("IndexDriver",     "COM_PORT",         AnsiString("COM1"));
    EL<TComboBox>("THandlerSystem", "cbComTemp")->Text             =CheckAndReadIniDataGeneral("TempCtrl",        "COM_PORT",         AnsiString("COM2"));
    EL<TComboBox>("THandlerSystem", "cbComTempOmron")->Text        =CheckAndReadIniDataGeneral("TempCtrl",        "COM_PORT_OMRON",   AnsiString("COM7"));
    EL<TComboBox>("THandlerSystem", "cbComDyTemp")->Text           =CheckAndReadIniDataGeneral("TempCtrl",        "COM_PORT_DYNAMIC", AnsiString("COM6"));
    EL<TComboBox>("THandlerSystem", "cbComBinDisp")->Text          =CheckAndReadIniDataGeneral("NUMBER_PANEL",    "COM_PORT",         AnsiString("COM4"));
    EL<TComboBox>("THandlerSystem", "cbbComBinDisp2")->Text        =CheckAndReadIniDataGeneral("NUMBER_PANEL2",    "COM_PORT",      AnsiString("COM4"));

    EL<TComboBox>("THandlerSystem", "cbComRTC")->Text              =CheckAndReadIniDataGeneral("RealTimeCCD",     "Port",             AnsiString("COM3"));
    EL<TComboBox>("THandlerSystem", "cbComTester")->Text           =CheckAndReadIniData(Str,   "COMPort",         "CommName",         AnsiString("COM5"));
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232")->Text         =CheckAndReadIniData(Str,   "COMPort_TTL",     "CommName",         AnsiString("COM3"));
    EL<TComboBox>("THandlerSystem", "cbComTTLRS232_2")->Text       =CheckAndReadIniData(Str,   "COMPort_TTL_2",   "CommName",         AnsiString("COM8"));     //Isaac 20210309 :TTL RS232兩塊板子

    //ATC
    EL<TRadioGroup>("THandlerSystem", "rgATC")->ItemIndex            =CheckAndReadIniDataGeneral("ATC", "USE_ATC_MODE"   ,  eATCUninstall);  //jou 2012-03-08 Enable ATC mode
    EL<TComboBox>("THandlerSystem", "cbbATC1")->Text               =CheckAndReadIniDataGeneral("ATC", "ATC1_COM_PORT"  , AnsiString("COM15"));
    EL<TComboBox>("THandlerSystem", "cbbATC2")->Text               =CheckAndReadIniDataGeneral("ATC", "ATC2_COM_PORT"  , AnsiString("COM16"));
    EL<TComboBox>("THandlerSystem", "cbbATC3")->Text               =CheckAndReadIniDataGeneral("ATC", "ATC3_COM_PORT"  , AnsiString("COM17"));
    EL<TComboBox>("THandlerSystem", "cbbATC4")->Text               =CheckAndReadIniDataGeneral("ATC", "ATC4_COM_PORT"  , AnsiString("COM18"));

    EL<TRadioGroup>("THandlerSystem", "rgATCMixMode")->ItemIndex     =CheckAndReadIniDataGeneral("ATC", "ATC_Mix_Mode"   ,  eATCNonMix);  //jou 2012-03-08 Enable ATC mode

    EL<TEdit>("THandlerSystem", "edATCSystemIP")->Text         =CheckAndReadIniDataGeneral("ATC", "ATC_SYSTEM_IP"  , AnsiString("172.16.8.90"));   //Ifor 20151230 :add New ATC Interface IP

    if(ATC_SYSTEM==eNewATCSystem)                                               //Ifor 20170620 (wei) add 使用New ATCSystem 強制設定Port為1234
        EL<TEdit>("THandlerSystem", "edATCSystemPort")->Text   =1234;
    else
        EL<TEdit>("THandlerSystem", "edATCSystemPort")->Text   =CheckAndReadIniDataGeneral("ATC", "ATC_SYSTEM_PORT", 1234);            //Ifor 20151230 :add New ATC Interface Port

    EL<TEdit>("THandlerSystem", "edATCSystemUseHeat")->Text    =CheckAndReadIniDataGeneral("ATC", "ATC_SYSTEM_USEHEAT", 4);            //Ifor 20160506 :add New ATC Interface Use Heat Count

    EL<TRadioGroup>("THandlerSystem", "rgInOutArmYPitch")->ItemIndex =CheckAndReadIniDataGeneral("System", "USE_IN_OUT_ARM_Y_PITCH", iXPitch60);   //jou 2012-05-15 : 選擇 Y Pitch 機構模式

    EL<TRadioGroup>("THandlerSystem", "rgOutArmYPitch")->ItemIndex   =CheckAndReadIniDataGeneral("System", "USE_OUT_ARM_Y_PITCH", USE_IN_OUT_ARM_Y_PITCH);      //JerryYang 20251218 : IN/OUT ARM支援不同模組

    EL<TEdit>("THandlerSystem", "edtMinYPitch")->Text          =CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_Y_PITCH_MIN", AnsiString("1500"));
    EL<TEdit>("THandlerSystem", "edtMaxYPitch")->Text          =CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_Y_PITCH_MAX", AnsiString("7500"));
    EL<TRadioGroup>("THandlerSystem", "rgInOutArmXPitch")->ItemIndex =CheckAndReadIniDataGeneral("System", "USE_IN_OUT_ARM_X_PITCH", iXPitch40mm);//Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
    EL<TEdit>("THandlerSystem", "edtMinXPitch")->Text          =CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MIN", AnsiString("4000"));    //JimmyChiu 20220708 : add Auto X pitch
    EL<TEdit>("THandlerSystem", "edtMaxXPitch")->Text          =CheckAndReadIniDataGeneral("System", "IN_OUT_ARM_X_PITCH_MAX", AnsiString("12000"));   //JimmyChiu 20220708 : add Auto X pitch

    EL<TRadioGroup>("THandlerSystem", "rgOutSortArm")->ItemIndex     =CheckAndReadIniDataGeneral("OutSortArm", "USE_OUT_SORT_ARM", eartUninstall);           //Steven 20240822 : For HT-9046AU
    EL<TEdit>("THandlerSystem", "edtOutSortXPitchMin")->Text   =CheckAndReadIniDataGeneral("OutSortArm", "USE_OUT_SORT_X_PITCH_MIN", AnsiString("1333")); //RogerYang 20250609 Add for 9046AU
    EL<TEdit>("THandlerSystem", "edtOutSortXPitchMax")->Text   =CheckAndReadIniDataGeneral("OutSortArm", "USE_OUT_SORT_X_PITCH_MAX", AnsiString("4000"));

    EL<TEdit>("THandlerSystem", "edtHPLimit")->Text            =CheckAndReadIniDataGeneral("System", "BASE_X_TO_HP",           AnsiString("6800"));  //Steven 20230826 : 基準軸X到加熱盤邊緣的距離
    EL<TRadioGroup>("THandlerSystem", "rgHotplateType")->ItemIndex   =CheckAndReadIniDataGeneral("System", "USE_HOTPLATE_TYPE", 0);          //jou 2012-05-15 : 選擇 Hotplate Type
    EL<TRadioGroup>("THandlerSystem", "rgFix3FullPlace")->ItemIndex  =CheckAndReadIniDataGeneral("System", "FIX3_FULL_PLACE",  Fix3K_Uninstall);           //Steven 20130126 : Fix3滿盤功能
    EL<TRadioGroup>("THandlerSystem", "rgMagneticScale")->ItemIndex  =CheckAndReadIniDataGeneral("System", "USE_MAGNETIC_SCALE", 0);         //Steven 20160426 : 磁性尺
    EL<TRadioGroup>("THandlerSystem", "rgPickerCount")->ItemIndex    =CheckAndReadIniDataGeneral("System", "USE_PICKER_COUNT", 1);           //Steven 20161117 : for HT-9045S
    EL<TRadioGroup>("THandlerSystem", "rgPreciser")->ItemIndex       =CheckAndReadIniDataGeneral("System", "USE_PRECISER", 1);               //Frank 20180410 (Steven) : InArm Preciser Station
//    cbPreciserInShuttleArea->Checked=PreciserInstallShuttleArea;                                        //Frank 20180410 (Steven) : InArm Preciser Station;
    EL<TRadioGroup>("THandlerSystem", "rgPreciserPos")->ItemIndex   =CheckAndReadIniDataGeneral("System", "iPreciserInstallArea", 0);//Ifor 20191008 : add Preciser Install Area

    EL<TRadioGroup>("THandlerSystem", "rgRotateKit")->ItemIndex      =CheckAndReadIniDataGeneral("ROTATE_KIT", "USE_ROTATE_KIT",    0);      //Steven 20121001 : 旋轉Kit
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->ItemIndex    =CheckAndReadIniDataGeneral("ROTATE_KIT", "iRotate_In_Index",  0);      //Steven 20121001 : 旋轉Kit
    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->ItemIndex   =CheckAndReadIniDataGeneral("ROTATE_KIT", "iRotate_Out_Index", 2);      //Steven 20121001 : 旋轉Kit
    EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex =CheckAndReadIniDataGeneral("ROTATE_KIT", "RotateKit_Type", 0);         //2013-04-12    Dell :旋轉站;馬達版

    EL<TRadioGroup>("THandlerSystem", "rgIOChangeToque")->ItemIndex  =CheckAndReadIniDataGeneral("IndexDriver", "USE_IO_CHANGE_TOQUE", 0);   //jou 2012-06-21 Enable index I/O Change Toque
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorAxis")->ItemIndex =CheckAndReadIniDataGeneral("IndexDriver", "USE_INDEX_ARM_AXES",  0);   //JimmyChiu 20220708 : add Index Arm Axis
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorType")->ItemIndex =CheckAndReadIniDataGeneral("IndexDriver", "INDEX_DRIVER_TYPE",   0);
    EL<TCheckBox>("THandlerSystem", "chkUseHPComCard")->Checked    =CheckAndReadIniDataGeneral("IndexDriver", "USE_HP_COM_CARD", false);   //Steven 20210204 : 使用鴻勁自製的通訊卡
    EL<TRadioGroup>("THandlerSystem", "rgCleanAir")->ItemIndex       =CheckAndReadIniDataGeneral("IndexDriver", "CLEAN_AIR", 0);             //ChungHung 20120821 : Clean Air

    EL<TRadioGroup>("THandlerSystem", "rgInstallFix3")->ItemIndex        =CheckAndReadIniDataGeneral("System", "FIX3_INSTALL",  0);          //2013-07-16     Dell    9046LS第一台沒Fix3;第二台以上Fix3標配
    EL<TRadioGroup>("THandlerSystem", "rgShuttleCrossSensor")->ItemIndex =CheckAndReadIniDataGeneral("System", "CROSS_SENSOR_INSTALL",  0);  //2013-07-16    Dell    Shuttle cross sensor
    EL<TRadioGroup>("THandlerSystem", "rgShuttleZType")->ItemIndex       =CheckAndReadIniDataGeneral("System", "SHUTTLE_Z_TYPE", 0);         //jou 2013-07-26 : 選擇 Shuttle Z Sensor Type
    EL<TRadioGroup>("THandlerSystem", "rgAutoShuttleSensor")->ItemIndex  =CheckAndReadIniDataGeneral("System", "AUTO_SENSOR_INSTALL",  0);   //wei 20160914 Auto Shuttle Sensor
    EL<TRadioGroup>("THandlerSystem", "rgTrayMapping")->ItemIndex        =CheckAndReadIniDataGeneral("System", "USE_TRAY_MAPPING",     0);   //wei 20161219 Tray Mapping
    EL<TRadioGroup>("THandlerSystem", "rgShuttleVibration")->ItemIndex   =CheckAndReadIniDataGeneral("System", "ShuttleVibration",     0);   //JerryYang 20171006 (wei) Shuttle 震動馬達
    EL<TRadioGroup>("THandlerSystem", "rgFixAICCD")->ItemIndex           =CheckAndReadIniDataGeneral("System", "Fix_AI_CCD",           0);   //RogerYang 20180901 add 矽格湖口Demo AI CCD Function
    EL<TRadioGroup>("THandlerSystem", "rgMRSystem")->ItemIndex           =CheckAndReadIniDataGeneral("System", "USE_MR_SYSTEM",        0);   //wei 20180702 MR
    EL<TRadioGroup>("THandlerSystem", "rgRFIDSystem")->ItemIndex         =CheckAndReadIniDataGeneral("System", "USE_RFID_SYSTEM",      0);   //wei 20180702 MR
    EL<TRadioGroup>("THandlerSystem", "rgRFIDReader")->ItemIndex         =CheckAndReadIniDataGeneral("System", "USE_RFID_READER",      0);   //Steven 20220713 : RFID Reader for SJSEMI
    EL<TComboBox>("THandlerSystem", "cbbRFIDReader")->Text             =CheckAndReadIniDataGeneral("RFID",   "RFIDReader_PORT", AnsiString("COM15"));
    EL<TRadioGroup>("THandlerSystem", "rgAutoAlignment")->ItemIndex      =CheckAndReadIniDataGeneral("System", "USE_AUTO_ALIGNMENT",    0);  //Sam 20181201 : AutoAlignment
    EL<TRadioGroup>("THandlerSystem", "rgCOLORSENSOR_MUN")->ItemIndex    =CheckAndReadIniDataGeneral("System", "USE_COLORSENSOR_MUN",  eCSMUN_Uninstall);   //Jimmychiu 20230630 : add color sensor MU-N in Loader

    EL<TRadioGroup>("THandlerSystem", "rg3M_EM_AWARE_Monitor")->ItemIndex=CheckAndReadIniDataGeneral("System", "USE_ESD_Monior",  0);        //2013-07-16     Dell    使用ESD monitor
    EL<TRadioGroup>("THandlerSystem", "rgNovx3360")->ItemIndex           =CheckAndReadIniDataGeneral("System", "USE_NOVX3360",  0);          //Steven 20131127 : 使用Simco ION風扇
    EL<TRadioGroup>("THandlerSystem", "rgUsePulseType")->ItemIndex       =CheckAndReadIniDataGeneral("System", "USE_PULSE_TYPE",  0);        //Ifor 20180316 : add Simco Use Pulse Type
//    if(CUSTOMER_CODE==CC_SJ_Semiconductor)
        EL<TEdit>("THandlerSystem", "edIONPulseCount")->Text       =CheckRange(CheckAndReadIniDataGeneral("System", "ION_PULSE_COUNT", 3000), 3000, 100000);       //Steven 20220107 : 張寧要求ESD要快點Alarm     //Steven 20230322 : ION_PULSE_COUNT 最小值改成3000
//    else
//        edIONPulseCount->Text       =CheckRange(CheckAndReadIniDataGeneral("System", "ION_PULSE_COUNT", 5000), 5000, 20000);      //Ifor 20180316 : add Simco Use Pulse Type  //JerryYang 20210413 : 新電腦太快會誤alarm,最小值改為5000
    EL<TRadioGroup>("THandlerSystem", "rgKasuga")->ItemIndex             =CheckAndReadIniDataGeneral("System", "USE_KASUGA",  0);            //Ifor 20150731 : 使用Kasuga ION風扇
    EL<TRadioGroup>("THandlerSystem", "rgChamberUsePulseType")->ItemIndex=CheckAndReadIniDataGeneral("System", "CHAMBER_USE_PULSE_TYPE",  0);//Ifor 20190422 : add Chamber Use Pulse Type
    EL<TRadioGroup>("THandlerSystem", "rgAuto3Magazine")->ItemIndex      =CheckAndReadIniDataGeneral("System", "AUTO3_IS_MAGAZINE",       0);//JerryYang 20220909 : add magazine

    if(EL<TRadioGroup>("THandlerSystem", "rgSocketSen")->ItemIndex==0)                                               //JerryYang 20200327 重新整理第三個通訊模組
    {
        EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt")->ItemIndex=0;
    }
    else
    {
        EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt")->ItemIndex=CheckAndReadIniDataGeneral("System", "SocketSenAmpQty",  4);
    }

    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt2nd")->ItemIndex=CheckAndReadIniDataGeneral("System", "SocketSenAmpQty2nd",  0);

    EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt3rd")->ItemIndex=CheckAndReadIniDataGeneral("System", "SocketSenAmpQty3rd",  0);     //JerryYang 20260205 : add第三組Socket sensor(8+16+8)

    EL<TRadioGroup>("THandlerSystem", "rgAutoCleanIonFan")->ItemIndex    =CheckAndReadIniDataGeneral("System", "USE_AutoCleanIonFan",  0);   //Isaac 20210609 : IO觸發IonFan清針

    if(EL<TRadioGroup>("THandlerSystem", "rgRotateKit")->ItemIndex==0)
    {
        EL<TComboBox>("THandlerSystem", "cbRotateSenAmpCnt")->ItemIndex=0;
    }
    else
    {
        EL<TComboBox>("THandlerSystem", "cbRotateSenAmpCnt")->ItemIndex=CheckAndReadIniDataGeneral("System", "RotateSenAmpQty",  4);
    }

    EL<TComboBox>("THandlerSystem", "cbColorSenAmpCnt")->ItemIndex=CheckAndReadIniDataGeneral("System", "ColorSenAmpQty",  0);

    EL<TComboBox>("THandlerSystem", "cbVibrationCardQty")->ItemIndex=CheckAndReadIniDataGeneral("System", "VibrationCardQty",  2);    //JerryYang 20230814 : add震動馬達通訊調速版本

    //Sam 20210518 : 新增 CanBus 軟體配置
    //==>
    EL<TComboBox>("THandlerSystem", "coCanBusNudn1")->ItemIndex=CheckAndReadIniDataGeneral("System", "CanBusNudn1Qty",  0);
    EL<TComboBox>("THandlerSystem", "coNudn1Macid11")->ItemIndex=CheckAndReadIniDataGeneral("System", "Nudn1Macid11Qty",  0);
    EL<TComboBox>("THandlerSystem", "coNudn1Macid12")->ItemIndex=CheckAndReadIniDataGeneral("System", "Nudn1Macid12Qty",  0);
    EL<TComboBox>("THandlerSystem", "coNudn1Macid13")->ItemIndex=CheckAndReadIniDataGeneral("System", "Nudn1Macid13Qty",  0);
    EL<TComboBox>("THandlerSystem", "coNudn1Macid14")->ItemIndex=CheckAndReadIniDataGeneral("System", "Nudn1Macid14Qty",  0);
    //<==
    //Sam 20210518 : 新增 CanBus 軟體配置

    EL<TRadioGroup>("THandlerSystem", "rgKasuga_Fan")->ItemIndex         =CheckAndReadIniDataGeneral("System", "USE_KASUGA_Fan",  0);        //KaiChen 20191225 ：KASUGA Fan 通訊
    EL<TRadioGroup>("THandlerSystem", "rgHTIonBar")->ItemIndex           =CheckAndReadIniDataGeneral("System", "HTIonBarFunction",  0);      //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
    EL<TEdit>("THandlerSystem", "ed24VMonitorPulseCount")->Text     =CheckAndReadIniDataGeneral("System", "I24V_PULSE_COUNT", 200);    //JerryYang 20190813 count最小值保護避免誤發alarm //Ifor 20190311 : add 24V Monitor Use Pulse Type
    EL<TRadioGroup>("THandlerSystem", "rgOTDInstall")->ItemIndex         =CheckAndReadIniDataGeneral("System", "USE_OTD",  0);               //Steven 20140312 : OTD

    EL<TRadioGroup>("THandlerSystem", "rgUse4DUT")->ItemIndex            =CheckAndReadIniDataGeneral("System", "SocketBasedAdd4Temp", 0);    //Steven 20131008 : for HT9046AH
    EL<TRadioGroup>("THandlerSystem", "rgUseSucker_9046_DB")->ItemIndex  =CheckAndReadIniDataGeneral("System", "USE_46_SUCKER_DB", 0);       //Steven 20131008 : HT9045使用46配氣
    EL<TRadioGroup>("THandlerSystem", "rgUseSensor_9046_DB")->ItemIndex  =CheckAndReadIniDataGeneral("System", "USE_46_SENSOR_DB", 0);       //Steven 20131008 : HT9045使用46配電
    EL<TRadioGroup>("THandlerSystem", "rgIndexMotionCard")->ItemIndex    =CheckAndReadIniDataGeneral("System", "INDEX_MOTION_CARD", 0);      //Steven 20210623 : Index使用Galil
    EL<TRadioGroup>("THandlerSystem", "rg_ControlPanelMode")->ItemIndex  =CheckAndReadIniDataGeneral("System", "ControlPanelMode", 0);       //Ken 20210702 AddPadInterface
    EL<TRadioGroup>("THandlerSystem", "rgVacuUnitType")->ItemIndex       =CheckAndReadIniDataGeneral("System", "VacuUnitType",     0);       //Sam 20230210 : 新增 VacuumUnit 通訊模組
    EL<TRadioGroup>("THandlerSystem", "rgFinePitch")->ItemIndex          =CheckAndReadIniDataGeneral("System",  "USE_FINE_PITCH", 0);        //Steven 20210624 : Fine Pitch
    EL<TRadioGroup>("THandlerSystem", "rgUseOutSht")->ItemIndex          =CheckAndReadIniDataGeneral("System",  "USE_OUT_SHT_MOT", 0);       //Steven 20210624 : Out shuttle獨立馬達

    EL<TRadioGroup>("THandlerSystem", "rgAirConditioner")->ItemIndex     =CheckAndReadIniDataGeneral("AIR_CON", "USE_AIR_CONDITIONER", 0);   //Steven 20131011 : 冷氣機
    EL<TComboBox>("THandlerSystem", "cbAirCon")->Text                  =CheckAndReadIniDataGeneral("AIR_CON", "AIR_CON_PORT",  AnsiString("COM15"));   //Steven 20131011 : 冷氣機

    EL<TComboBox>("THandlerSystem", "cbEMAwarePort1")->Text            =CheckAndReadIniDataGeneral("EM Aware", "EM_AWARE_PORT1",  AnsiString("COM15"));//Steven 20131114 : ESD Monitor
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort2")->Text            =CheckAndReadIniDataGeneral("EM Aware", "EM_AWARE_PORT2",  AnsiString("COM16"));//Steven 20131114 : ESD Monitor
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort3")->Text            =CheckAndReadIniDataGeneral("EM Aware", "EM_AWARE_PORT3",  AnsiString("COM17"));//Steven 20131114 : ESD Monitor
    EL<TComboBox>("THandlerSystem", "cbEMAwarePort4")->Text            =CheckAndReadIniDataGeneral("EM Aware", "EM_AWARE_PORT4",  AnsiString("COM18"));//Steven 20131114 : ESD Monitor
    EL<TCheckBox>("THandlerSystem", "cbESDUse4COM")->Checked           =CheckAndReadIniDataGeneral("EM Aware", "EM_AWARE_USE_4_COM",  true);//Steven 20131114 : ESD Monitor
    EL<TComboBox>("THandlerSystem", "cbNovx3360")->Text                =CheckAndReadIniDataGeneral("EM Aware", "NOVX_3360_PORT",  AnsiString("COM18"));//Steven 20131127 : 使用Simco ION風扇
    EL<TComboBox>("THandlerSystem", "cbKASUGA_Fan")->Text              =CheckAndReadIniDataGeneral("System",   "KASUGA_Fan_PORT", AnsiString("COM18"));    //KaiChen 20191225 ：KASUGA Fan 通訊
    EL<TComboBox>("THandlerSystem", "cbComLaser1")->Text               =CheckAndReadIniDataGeneral("Laser",    "COM_Laser_1",     AnsiString("COM16"));//Steven 20140228 : 雷射測距功能
    EL<TComboBox>("THandlerSystem", "cbComLaser2")->Text               =CheckAndReadIniDataGeneral("Laser",    "COM_Laser_2",     AnsiString("COM17"));//Steven 20140228 : 雷射測距功能
    EL<TComboBox>("THandlerSystem", "cbComLaserInArm")->Text           =CheckAndReadIniDataGeneral("Laser",    "COM_Laser_InArm", AnsiString("COM18"));//Steven 20140228 : 雷射測距功能
    EL<TComboBox>("THandlerSystem", "cb2DReader1")->Text               =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode1_COM_PORT", AnsiString("COM15"));
    EL<TComboBox>("THandlerSystem", "cb2DReader2")->Text               =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode2_COM_PORT", AnsiString("COM16"));
    EL<TComboBox>("THandlerSystem", "cb2DReader3")->Text               =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode3_COM_PORT", AnsiString("COM17"));
    EL<TComboBox>("THandlerSystem", "cb2DReader4")->Text               =CheckAndReadIniDataGeneral("2D_BarCode",   "BarCode4_COM_PORT", AnsiString("COM18"));

    EL<TEdit>("THandlerSystem", "edMaxKpa")->Text                  =CheckAndReadIniDataGeneral("System" , "EP_MAXKPA" , 499.0);
    EL<TEdit>("THandlerSystem", "edMaxMpaFB")->Text                =CheckAndReadIniDataGeneral("System" , "EP_MAXA"   , 5.013);
    EL<TEdit>("THandlerSystem", "edMinMpa")->Text                  =CheckAndReadIniDataGeneral("System" , "EP_MINMPA"   , 0.001);          //JerryYang 20171023 (wei) add PA Min
    EL<TEdit>("THandlerSystem", "edtMinMpaFB")->Text               =CheckAndReadIniDataGeneral("System" , "EP_MINA_FeedBack"   , 0.908);

    EL<TRadioGroup>("THandlerSystem", "rgAOI")->ItemIndex                =CheckAndReadIniDataGeneral("System", "AOI",  0);                           //2014-04-07    Dell    for SPIL AOI
    EL<TRadioGroup>("THandlerSystem", "rgVibrationCommuncation")->ItemIndex=CheckAndReadIniDataGeneral("Vibration", "VibrationCommunication",  0);   //JerryYang 20230814 : add震動馬達通訊調速版本

    int iTemp175                    =CheckAndReadIniDataGeneral("System", "HighTemperatureSet175", 0);          //Frank 20160705 add 改機最高溫175度
    int iTemp155                    =CheckAndReadIniDataGeneral("System", "HighTemperatureSet155", 0);          //Sam 20240118 新增 155度 模式
    int iTemp150                    =CheckAndReadIniDataGeneral("System", "HighTemperatureSet150", 0);          //wei 20150617 改機最高溫150度
    if(iTemp175!=0)                                                             //Steven 20250701 : 整合機台溫度設定
    {
        EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex  =tTemp175;
    }
    else if(iTemp155!=0)
    {
        EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex  =tTemp155;
    }
    else if(iTemp150!=0)
    {
        EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex  =tTemp150;
    }
    else
    {
        EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex=CheckAndReadIniDataGeneral("System", "HighTempLimit", tTemp130);            //Steven 20250701 : 整合機台溫度設定
    }

    EL<TRadioGroup>("THandlerSystem", "rgDewpointHW")->ItemIndex         =CheckAndReadIniDataGeneral("System",  "DewPoint_Hardware_Install", 0); //Steven 20191017 : 露點計
    EL<TRadioGroup>("THandlerSystem", "rgCatchTrayModel")->ItemIndex     =CheckAndReadIniDataGeneral("System" , "USE_CATCH_TRAY_MODEL"   , 0);
    EL<TRadioGroup>("THandlerSystem", "rgTrayArmType")->ItemIndex        =CheckAndReadIniDataGeneral("System" , "USE_TRAY_ARM_TYPE"   , 0);          //Ifor 20260824 : Tray Arm hardware type
    EL<TRadioGroup>("THandlerSystem", "rgCatchTrayBlock")->ItemIndex     =CheckAndReadIniDataGeneral("System" , "UseCatchTrayBlock"   , 0);      ;    //kevin 20200512 夾tray遮版削短
//    if(USE_AUTO_RETEST==eartInstall)
//        rgCatchTrayModel->ItemIndex     =2;                                   //wei 20150810

    EL<TRadioGroup>("THandlerSystem", "rgCKDFCM")->ItemIndex             =CheckAndReadIniDataGeneral("System" , "USE_CKD_FCM_CleanAir",   0);
    EL<TRadioGroup>("THandlerSystem", "rgSocketClamp")->ItemIndex        =CheckAndReadIniDataGeneral("System" , "INSTALL_SOCKET_CLAMP",   0);            //JerryYang 20160523 是否改機為SLK、SOCKET夾持汽缸
    EL<TRadioGroup>("THandlerSystem", "rgDoubleEPControl")->ItemIndex    =CheckAndReadIniDataGeneral("System" , "INSTALL_DOUBLE_EP",   0);               //Ifor 20190104 : Add Double EP Control

    EL<TRadioGroup>("THandlerSystem", "rgGroundMan")->ItemIndex              =CheckAndReadIniDataGeneral("Ground_Man", "USE_GROUND_MAN",  0);            //Steven 20190828 : 通訊式GroundMan
    EL<TComboBox>("THandlerSystem", "cbbGroundMan")->Text                  =CheckAndReadIniDataGeneral("Ground_Man", "Ground_Man_COM_PORT", AnsiString("COM18"));
    EL<TRadioGroup>("THandlerSystem", "rgGroundMan_ScanPoint")->ItemIndex    =CheckAndReadIniDataGeneral("Ground_Man", "Ground_Man_ScanPoint",  0);      //KaiChen 20191005 ：通訊式GroundMan
    EL<TEdit>("THandlerSystem", "edGroundMan_AlarmOhm")->Text          =CheckAndReadIniDataGeneral("Ground_Man", "Ground_Man_AlarmOhm" , 3);       //KaiChen 20191005 ：通訊式GroundMan
    EL<TRadioGroup>("THandlerSystem", "rgHotGunFlow")->ItemIndex             =CheckAndReadIniDataGeneral("System", "HotGunFlowEnable", 0);               //KaiChen 20190729 ：Hot Gun Flow
    EL<TEdit>("THandlerSystem", "edHotGunFlow_LineNo")->Text           =CheckAndReadIniDataGeneral("System", "HotGunFlow_LineNo" , 0);             //KaiChen 20190729 ：Hot Gun Flow
    EL<TEdit>("THandlerSystem", "edHotGunFlow_DevNo")->Text            =CheckAndReadIniDataGeneral("System", "HotGunFlow_DevNo" , 0);              //KaiChen 20190729 ：Hot Gun Flow
    EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun1_ChannelNo")->Text   =CheckAndReadIniDataGeneral("System", "HotGunFlow_Gun1_ChannelNo" , 0);     //KaiChen 20190729 ：Hot Gun Flow
    EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun2_ChannelNo")->Text   =CheckAndReadIniDataGeneral("System", "HotGunFlow_Gun2_ChannelNo" , 0);     //KaiChen 20190729 ：Hot Gun Flow
    EL<TRadioGroup>("THandlerSystem", "rgScanner_AOI")->ItemIndex            =CheckAndReadIniDataGeneral("System", "Scanner_AOI",  0);                   //Ifor 20190725 : add Scanner AOI
    EL<TRadioGroup>("THandlerSystem", "rgTopScanner_AOI")->ItemIndex         =CheckAndReadIniDataGeneral("System", "Top_Scanner_AOI",  0);               //Ifor 20200902 add: TFAMD Top AOI
    EL<TRadioGroup>("THandlerSystem", "rgLdUldUseStepMotor")->ItemIndex      =CheckAndReadIniDataGeneral("TrayY", "LoaderUnload_StepMotor", 0);          //Steven 20200529 : Loader入Tray改步進
    EL<TComboBox>("THandlerSystem", "cbbTrayStepMotor")->Text              =CheckAndReadIniDataGeneral("TrayY", "COM PORT",  AnsiString("COM18"));
    if(iIndEPCnt==16)
    {
        EL<TComboBox>("THandlerSystem", "cbIndEPCnt")->ItemIndex=0;
    }
    else if(iIndEPCnt==8)                                                       //RogerYang 20260603 : Add 8EP
    {
        EL<TComboBox>("THandlerSystem", "cbIndEPCnt")->ItemIndex=2;
    }
    else
    {
        EL<TComboBox>("THandlerSystem", "cbIndEPCnt")->ItemIndex=1;                                                //JerryYang 20210413 : Add 4組獨立EP版本
    }

    EL<TRadioGroup>("THandlerSystem", "rgOHTSystem")->ItemIndex                  =CheckAndReadIniDataGeneral("System", "USE_OHT_SYSTEM",        0);          //KaiChen 20200716 ：OHT
    EL<TRadioGroup>("THandlerSystem", "rgMultileEmpty")->ItemIndex               =CheckAndReadIniDataGeneral("System", "USE_Multile_Empty",     0);          //KaiChen 20200716 ：OHT
    EL<TRadioGroup>("THandlerSystem", "rgLoaderKeyence")->ItemIndex              =CheckAndReadIniDataGeneral("System", "USE_KEYENCE_LOADER",     0);         //wei 20161219 Tray Mapping
    EL<TRadioGroup>("THandlerSystem", "rgEmptyKeyence")->ItemIndex               =CheckAndReadIniDataGeneral("System", "USE_KEYENCE_EMPTY",      0);         //wei 20161219 Tray Mapping
    EL<TRadioGroup>("THandlerSystem", "rgMultileEmptyTrayIDKeyence")->ItemIndex  =CheckAndReadIniDataGeneral("System", "USE_MultileEmptyTrayID_Keyence",      0);
    EL<TRadioGroup>("THandlerSystem", "rgTrayMappingGrabImage")->ItemIndex       =CheckAndReadIniDataGeneral("System", "TRAY_MAPPING_GRAB",      0);         //KenHsieh 20220923 : add Tray Map Throw IC Function
    EL<TRadioGroup>("THandlerSystem", "rgBaseHeaterCount")->ItemIndex            =CheckAndReadIniDataGeneral("System", "BASE_HEATER_COUNT",  0);             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TRadioGroup>("THandlerSystem", "rgShuttleFloodgate")->ItemIndex           =CheckAndReadIniDataGeneral("System", "SHUTTLE_FLOODGATE",  0);             //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TRadioGroup>("THandlerSystem", "rgTriTempMachine")->ItemIndex             =CheckAndReadIniDataGeneral("System", "Tri_Temp_Machine",  0);              //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TRadioGroup>("THandlerSystem", "rgAirStreamSelect")->ItemIndex            =CheckAndReadIniDataGeneral("System", "AirStream_Select",  0);              //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edtTriTemperature_MaxDegree")->Text       =CheckAndReadIniDataGeneral("System", "Tri_Temperature_MaxDegree",  0);     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edtTriTemperature_MinDegree")->Text       =CheckAndReadIniDataGeneral("System", "Tri_Temperature_MinDegree",  0);     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edtTriTempTotalCh")->Text                 =CheckAndReadIniDataGeneral("System", "TriTemperature_TotalChannel",  0);   //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edtOutShtMaxTemp")->Text                  =CheckAndReadIniDataGeneral("System", "SetHeaterTemp_MaxOutSht",  20);      //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edtIndexMaxTemp")->Text                   =CheckAndReadIniDataGeneral("System", "SetHeaterTemp_MaxIndex",  20);       //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edtBaseMaxTemp")->Text                    =CheckAndReadIniDataGeneral("System", "SetHeaterTemp_MaxBase",  20);        //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TEdit>("THandlerSystem", "edt_Total_Compressor")->Text              =CheckAndReadIniDataGeneral("System", "Total_Compressor",  3);              //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TRadioGroup>("THandlerSystem", "rgSafePlcIO")->ItemIndex=CheckAndReadIniDataGeneral("System", "SafePlcIO", 0 /* golden: eSafePLCIOType_Uninstall (=0) */);   // golden HandlerSys.cpp:563-563 取代：預設值 eSafePLCIOType_Uninstall（golden cmydef.h ESafePLCIOType，＝0）移植樹沒有這個列舉 → 字面值 0（值相同；移植樹 database.cpp 讀同鍵也是 0）
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:563-563 -- 預設值 eSafePLCIOType_Uninstall（golden cmydef.h ESafePLCIOType，＝0）移植樹沒有這個列舉 → 字面值 0（值相同；移植樹 database.cpp 讀同鍵也是 0）
    EL<TRadioGroup>("THandlerSystem", "rgSafePlcIO")->ItemIndex                  =CheckAndReadIniDataGeneral("System", "SafePlcIO", eSafePLCIOType_Uninstall);//Jason 20230619 增加PLC_IO模組及安全相關IO.
#endif // GATE (S12-C save)
    EL<TRadioGroup>("THandlerSystem", "rg_IndexDoorHeater")->ItemIndex           =CheckAndReadIniDataGeneral("System", "IndexDoorHeater",  0);               //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    EL<TRadioGroup>("THandlerSystem", "rgDoubleBeltMode")->ItemIndex             =CheckAndReadIniDataGeneral("System", "DOUBLE_BELT_MODE",       0);         //Ztex 2023.12.13 Add Double Belt Mode

    EL<TRadioGroup>("THandlerSystem", "rgCoverTrayID")->ItemIndex                =CheckAndReadIniDataGeneral("System", "USE_COVER_TRAYID",        0);        //JerryYang 20240318 : add
    EL<TRadioGroup>("THandlerSystem", "rgInShtAutoLatch")->ItemIndex             =CheckAndReadIniDataGeneral("System", "In_Shuttle_Auto_Latch", 0);          //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    EL<TRadioGroup>("THandlerSystem", "rgLDCarRotAtm")->ItemIndex                =CheckAndReadIniDataGeneral("System", "USE_LD_Rot_Arm",        0);          //RogerYang 20250828 add for Loader Rotate Arm

    EL<TRadioGroup>("THandlerSystem", "rgE84Sensor")->ItemIndex                  =CheckAndReadIniDataGeneral("System", "AGVModal",  0);                //Frank 20251204 add E84
    EL<TRadioGroup>("THandlerSystem", "rgLdUldCassetteMode")->ItemIndex          =CheckAndReadIniDataGeneral("System", "USE_LdUldCassetteMode",  0);         //Frank 20251217 add

}

// golden HandlerSys.cpp:1132  THandlerSystem::LoaderSafeDoorSet()
static void HS_LoaderSafeDoorSet()
{

    EL<TCheckBox>("THandlerSystem", "SafeDoor1")->Checked=Sen[SnSafeDoor1].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor2")->Checked=Sen[SnSafeDoor2].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor3")->Checked=Sen[SnSafeDoor3].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor4")->Checked=Sen[SnSafeDoor4].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor5")->Checked=Sen[SnSafeDoor5].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor6")->Checked=Sen[SnSafeDoor6].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor7")->Checked=Sen[SnSafeDoor7].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor8")->Checked=Sen[SnSafeDoor8].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor9")->Checked=Sen[SnSafeDoor9].Enable;
    EL<TCheckBox>("THandlerSystem", "SafeDoor10")->Checked=Sen[SnSafeDoor10].Enable;
    EL<TCheckBox>("THandlerSystem", "HeaterDoor1")->Checked=Sen[SnHeaterDoor].Enable;
    EL<TCheckBox>("THandlerSystem", "HeaterDoor2")->Checked=Sen[SnHeaterDoor2].Enable;

}

// golden HandlerSys.cpp:1388  THandlerSystem::rgTTLCardClick(TObject *Sender)
static void HS_rgTTLCardClick()
{

    if(EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex==3)
    {
        EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress")->ItemIndex=1;
        EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress")->Enabled=false;
    }
    else
        EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress")->Enabled=true;

}

// golden HandlerSys.cpp:575  THandlerSystem::SaveSystemSet()
static void HS_SaveSystemSet()
{
    filerw::ELMark("SaveSystemSet");   // FileRW：存檔流程 trace（ack.trace）

    if(filerw::ELAsk("Do you want to store the setting?", "是否儲存設定？")==1) filerw::ELMark("SaveSystemSet:write"); else return;   // golden HandlerSys.cpp:577-577 取代：Application->MessageBox YES/NO → ELAsk（題目＝golden 英文原字串，頁面帶答案 {"Do you want to store the setting?":1}）；答 YES 才記 SaveSystemSet:write（PageDesc.savedMark）
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:577-577 -- Application->MessageBox YES/NO → ELAsk（題目＝golden 英文原字串，頁面帶答案 {"Do you want to store the setting?":1}）；答 YES 才記 SaveSystemSet:write（PageDesc.savedMark）
    if(Application->MessageBox("Do you want to store the setting?", NULL, MB_YESNO | MB_TOPMOST)!=IDYES) return;
#endif // GATE (S12-C save)
//----------------------------------
    WriteIniDataGeneral("System", "AUTO_EMPTY_COLOR", EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex);
    AUTO_EMPTY_COLOR=EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex;
    if(EL<TRadioGroup>("THandlerSystem", "MachineTrack")->ItemIndex==2)                                              //7 Track (Empty Unloader)
    {
        WriteIniDataGeneral("System", "SUPPORT_2_EMPTY_EMPTY", 1);
    }
    else
    {
        WriteIniDataGeneral("System", "SUPPORT_2_EMPTY_EMPTY", 0);
    }

//----------------------------------
    WriteIniDataGeneral("System", "EP_Install", EL<TRadioGroup>("THandlerSystem", "ElectronPressure")->ItemIndex);
//    EP_Install=ElectronPressure->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", EL<TRadioGroup>("THandlerSystem", "ArmZAtoZH")->ItemIndex);
    InOutArmPickerUseMotor=EL<TRadioGroup>("THandlerSystem", "ArmZAtoZH")->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("TrayZ", "LOAD_Z_USE_MOTOR",  EL<TCheckBox>("THandlerSystem", "chkLoader")->Checked);      //Steven 20190813 : 入Tray改用步進馬達
    WriteIniDataGeneral("TrayZ", "EMPTY_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkEmpty")->Checked);
    WriteIniDataGeneral("TrayZ", "COLOR_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkColor")->Checked);
    WriteIniDataGeneral("TrayZ", "AUTO1_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto1")->Checked);
    WriteIniDataGeneral("TrayZ", "AUTO2_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto2")->Checked);
    WriteIniDataGeneral("TrayZ", "AUTO3_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto3")->Checked);
    WriteIniDataGeneral("TrayZ", "AUTO4_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto4")->Checked);       //Steven 20230907 : For HT-9011UC
    WriteIniDataGeneral("TrayZ", "AUTO5_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto5")->Checked);
    WriteIniDataGeneral("TrayZ", "AUTO6_Z_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto6")->Checked);

    WriteIniDataGeneral("TrayCassette", "LOAD_USE_Cassette",   EL<TCheckBox>("THandlerSystem", "cbLoaderCassette")->Checked);      //Steven 20190813 : 入Tray改用步進馬達

    //AI(ht9045-v899) 20260423: cassette mode (Boat Carrier) already owns MAuto1Y/MAuto2Y, force-disable LOAD_Y_USE_MOTOR[3]/[4] to avoid motor contention
    if(EL<TRadioGroup>("THandlerSystem", "rgLdUldCassetteMode")->ItemIndex==1 && (EL<TCheckBox>("THandlerSystem", "chkAuto1Y")->Checked || EL<TCheckBox>("THandlerSystem", "chkAuto2Y")->Checked))
    {
    filerw::ELMessage("Cassette mode (Boat Carrier) already uses MAuto1Y/MAuto2Y. AUTO1_Y_USE_MOTOR / AUTO2_Y_USE_MOTOR will be force-disabled.", "Cassette 模式（Boat Carrier）已使用 MAuto1Y／MAuto2Y，AUTO1_Y_USE_MOTOR／AUTO2_Y_USE_MOTOR 將強制關閉。");   // golden HandlerSys.cpp:612-612 取代：Application->MessageBox(MB_OK｜MB_ICONWARNING) → ELMessage（之後照 golden 強制關掉 chkAuto1Y／chkAuto2Y）
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:612-612 -- Application->MessageBox(MB_OK｜MB_ICONWARNING) → ELMessage（之後照 golden 強制關掉 chkAuto1Y／chkAuto2Y）
        Application->MessageBox("Cassette mode (Boat Carrier) already uses MAuto1Y/MAuto2Y. AUTO1_Y_USE_MOTOR / AUTO2_Y_USE_MOTOR will be force-disabled.", "Conflict", MB_OK | MB_ICONWARNING);
#endif // GATE (S12-C save)
        EL<TCheckBox>("THandlerSystem", "chkAuto1Y")->Checked=false;
        EL<TCheckBox>("THandlerSystem", "chkAuto2Y")->Checked=false;
    }

    WriteIniDataGeneral("TrayY", "LOAD_Y_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkLoaderY")->Checked);      //Jimmychiu 20240307 : Loader Tray改用步進馬達
    WriteIniDataGeneral("TrayY", "EMPTY_Y_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkEmptyY")->Checked);      //AI(ht9045-v899) 20260423: persist empty y stepper
    WriteIniDataGeneral("TrayY", "COLOR_Y_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkColorY")->Checked);      //AI(ht9045-v899) 20260423: persist color y stepper
    WriteIniDataGeneral("TrayY", "AUTO1_Y_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto1Y")->Checked);      //AI(ht9045-v899) 20260423: persist auto1 y stepper
    WriteIniDataGeneral("TrayY", "AUTO2_Y_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto2Y")->Checked);      //AI(ht9045-v899) 20260423: persist auto2 y stepper
    WriteIniDataGeneral("TrayY", "AUTO3_Y_USE_MOTOR", EL<TCheckBox>("THandlerSystem", "chkAuto3Y")->Checked);      //AI(ht9045-v899) 20260423: persist auto3 y stepper
    WriteIniDataGeneral("TrayCassette", "EMPTY_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbEmptyCassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "COLOR_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbColorCassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "AUTO1_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbAuto1Cassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "AUTO2_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbAuto2Cassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "AUTO3_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbAuto3Cassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "AUTO4_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbAuto4Cassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "AUTO5_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbAuto5Cassette")->Checked);
    WriteIniDataGeneral("TrayCassette", "AUTO6_USE_Cassette",  EL<TCheckBox>("THandlerSystem", "cbAuto6Cassette")->Checked);       //Steven 20230907 : For HT-9011UC

    LOAD_Z_USE_MOTOR[0]=EL<TCheckBox>("THandlerSystem", "chkLoader")->Checked;
    LOAD_Z_USE_MOTOR[1]=EL<TCheckBox>("THandlerSystem", "chkEmpty")->Checked;
    LOAD_Z_USE_MOTOR[2]=EL<TCheckBox>("THandlerSystem", "chkColor")->Checked;
    LOAD_Z_USE_MOTOR[3]=EL<TCheckBox>("THandlerSystem", "chkAuto1")->Checked;
    LOAD_Z_USE_MOTOR[4]=EL<TCheckBox>("THandlerSystem", "chkAuto2")->Checked;
    LOAD_Z_USE_MOTOR[5]=EL<TCheckBox>("THandlerSystem", "chkAuto3")->Checked;
    LOAD_Z_USE_MOTOR[6]=EL<TCheckBox>("THandlerSystem", "chkAuto4")->Checked;                                      //Steven 20230907 : For HT-9011UC
    LOAD_Z_USE_MOTOR[7]=EL<TCheckBox>("THandlerSystem", "chkAuto5")->Checked;
    LOAD_Z_USE_MOTOR[8]=EL<TCheckBox>("THandlerSystem", "chkAuto6")->Checked;
    LOAD_Y_USE_MOTOR[0]=EL<TCheckBox>("THandlerSystem", "chkLoaderY")->Checked;                                     //Jimmychiu 20240307 : Loader Tray改用步進馬達
    LOAD_Y_USE_MOTOR[1]=EL<TCheckBox>("THandlerSystem", "chkEmptyY")->Checked;                                      //AI(ht9045-v899) 20260423: apply empty y stepper
    LOAD_Y_USE_MOTOR[2]=EL<TCheckBox>("THandlerSystem", "chkColorY")->Checked;                                      //AI(ht9045-v899) 20260423: apply color y stepper
    LOAD_Y_USE_MOTOR[3]=EL<TCheckBox>("THandlerSystem", "chkAuto1Y")->Checked;                                      //AI(ht9045-v899) 20260423: apply auto1 y stepper
    LOAD_Y_USE_MOTOR[4]=EL<TCheckBox>("THandlerSystem", "chkAuto2Y")->Checked;                                      //AI(ht9045-v899) 20260423: apply auto2 y stepper
    LOAD_Y_USE_MOTOR[5]=EL<TCheckBox>("THandlerSystem", "chkAuto3Y")->Checked;                                      //AI(ht9045-v899) 20260423: apply auto3 y stepper

    LOADUNLOAD_USE_CASSETTE[0]=EL<TCheckBox>("THandlerSystem", "cbLoaderCassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[1]=EL<TCheckBox>("THandlerSystem", "cbEmptyCassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[2]=EL<TCheckBox>("THandlerSystem", "cbColorCassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[3]=EL<TCheckBox>("THandlerSystem", "cbAuto1Cassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[4]=EL<TCheckBox>("THandlerSystem", "cbAuto2Cassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[5]=EL<TCheckBox>("THandlerSystem", "cbAuto3Cassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[6]=EL<TCheckBox>("THandlerSystem", "cbAuto4Cassette")->Checked;                        //Steven 20230907 : For HT-9011UC
    LOADUNLOAD_USE_CASSETTE[7]=EL<TCheckBox>("THandlerSystem", "cbAuto5Cassette")->Checked;
    LOADUNLOAD_USE_CASSETTE[8]=EL<TCheckBox>("THandlerSystem", "cbAuto6Cassette")->Checked;

    WriteIniDataGeneral("TrayY", "LoaderUnload_StepMotor", EL<TRadioGroup>("THandlerSystem", "rgLdUldUseStepMotor")->ItemIndex); //Steven 20200529 : Loader入Tray改步進
    WriteIniDataGeneral("TrayY", "COM PORT",  EL<TComboBox>("THandlerSystem", "cbbTrayStepMotor")->Text);

//---------------------------------- kevin 20120718 Auto2分離氣缸裝置--------------------------
    WriteIniDataGeneral("System", "bUseAuto2Empty", EL<TRadioGroup>("THandlerSystem", "Auto2SelectCy")->ItemIndex);
    bUseAuto2Empty=EL<TRadioGroup>("THandlerSystem", "Auto2SelectCy")->ItemIndex;
//------------------------------------------------------------------------------
    USE_AUTO_RETEST=EL<TRadioGroup>("THandlerSystem", "rgInstallAutoRestest")->ItemIndex;
    if(USE_AUTO_RETEST)
    {
        UNLOADER_ART[eAuto1]=EL<TRadioGroup>("THandlerSystem", "rgAuto1ART")->ItemIndex;                             //Steven 20161221 (jou) : for SCK only Auto 2 has ART
        UNLOADER_ART[eAuto2]=EL<TRadioGroup>("THandlerSystem", "rgAuto2ART")->ItemIndex;                             //Steven 20161221 (jou) : for SCK only Auto 2 has ART
        UNLOADER_ART[eAuto3]=EL<TRadioGroup>("THandlerSystem", "rgAuto3ART")->ItemIndex;                             //Steven 20161221 (jou) : for SCK only Auto 2 has ART
        UNLOADER_ART[eAuto4]=EL<TRadioGroup>("THandlerSystem", "rgAuto4ART")->ItemIndex;                             //RogerYang 20250823 : Fix index
        UNLOADER_ART[eAuto5]=EL<TRadioGroup>("THandlerSystem", "rgAuto5ART")->ItemIndex;
        UNLOADER_ART[eAuto6]=EL<TRadioGroup>("THandlerSystem", "rgAuto6ART")->ItemIndex;
    }
    WriteIniDataGeneral("System", "USE_AUTO_RETEST",    USE_AUTO_RETEST);       //ChungHung 20140317 add Auto Retest
    WriteIniDataGeneral("System", "UNLOADER_AUTO1_ART", UNLOADER_ART[eAuto1]);  //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    WriteIniDataGeneral("System", "UNLOADER_AUTO2_ART", UNLOADER_ART[eAuto2]);  //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    WriteIniDataGeneral("System", "UNLOADER_AUTO3_ART", UNLOADER_ART[eAuto3]);  //Steven 20161221 (jou) : for SCK only Auto 2 has ART
    WriteIniDataGeneral("System", "UNLOADER_AUTO4_ART", UNLOADER_ART[eAuto4]);
    WriteIniDataGeneral("System", "UNLOADER_AUTO5_ART", UNLOADER_ART[eAuto5]);
    WriteIniDataGeneral("System", "UNLOADER_AUTO6_ART", UNLOADER_ART[eAuto6]);
    WriteIniDataGeneral("System", "bAutoTrackCanGoRear", EL<TRadioGroup>("THandlerSystem", "rgAutoTrackCanGoRear")->ItemIndex);  //Isaac 20180109 (Steven) : auto123可前進後退
    WriteIniDataGeneral("System", "bAutoZNoUseART", EL<TRadioGroup>("THandlerSystem", "rgInstallARTtwocylinder")->ItemIndex);    //kevin 20181023 add ART Auto 1 2 3  使用一段氣缸 k21

//----------------------------------
    CUSTOMER_CODE=atoi(EL<TEdit>("THandlerSystem", "edtCustomerCode")->Text.c_str());
    WriteIniDataGeneral("System", "CUSTOMER_CODE", CUSTOMER_CODE);
    filerw::ELTodo("golden HandlerSys.cpp:688 fMain->InitialSuperVisorPassword(CUSTOMER_CODE) not called -- the port has it only as a static in WebLogin.cpp:64 (boot time); the supervisor password is not re-initialised for the new CUSTOMER_CODE");   // golden HandlerSys.cpp:688-688 取代：fMain->InitialSuperVisorPassword(CUSTOMER_CODE)：golden 本體 main.cpp:26458；移植樹只在 WebLogin.cpp:64 以 static 翻譯（開機 WebLogin_Boot 呼叫），本 TU 呼叫不到
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:688-688 -- fMain->InitialSuperVisorPassword(CUSTOMER_CODE)：golden 本體 main.cpp:26458；移植樹只在 WebLogin.cpp:64 以 static 翻譯（開機 WebLogin_Boot 呼叫），本 TU 呼叫不到
    fMain->InitialSuperVisorPassword(CUSTOMER_CODE);
#endif // GATE (S12-C save)
//----------------------------------
    WriteIniDataGeneral("System", "NUMBER_PANEL_TYPE", EL<TRadioGroup>("THandlerSystem", "rgNumberPanelType")->ItemIndex);
//    NUMBER_PANEL_TYPE=rgNumberPanelType->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("System",    "WEIGHT_CALIBRATION", EL<TRadioGroup>("THandlerSystem", "rgWeightCali")->ItemIndex);    //Steven 20111108
    WEIGHT_CALIBRATION=EL<TRadioGroup>("THandlerSystem", "rgWeightCali")->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("System", "ION_FAN_TYPE", EL<TRadioGroup>("THandlerSystem", "rgIonFanType")->ItemIndex);
    ION_FAN_TYPE=EL<TRadioGroup>("THandlerSystem", "rgIonFanType")->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("System", "SHUTTLE_SENSOR_TYPE", EL<TRadioGroup>("THandlerSystem", "rgShuttleSensor")->ItemIndex);
    SHUTTLE_SENSOR_TYPE=EL<TRadioGroup>("THandlerSystem", "rgShuttleSensor")->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("System", "NUEC_TYPE", EL<TRadioGroup>("THandlerSystem", "rgNUECType")->ItemIndex);          //Sam 20230707 : EtherCAT Shuttle sensor
    NUEC_TYPE=EL<TRadioGroup>("THandlerSystem", "rgNUECType")->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("System", "ENABLE_OUT_SHUTTLE_SENEOR", EL<TCheckBox>("THandlerSystem", "cbEnableOutShtSensor")->Checked);      //Steven 20120531 : by pass out shuttle sensor
    ENABLE_OUT_SHUTTLE_SENEOR=EL<TCheckBox>("THandlerSystem", "cbEnableOutShtSensor")->Checked;                                        //Steven 20120531 : by pass out shuttle sensor
    WriteIniDataGeneral("System", "ENABLE_OUT_SHUTTLEY_LATCH", EL<TCheckBox>("THandlerSystem", "chkOutShtYSensorByLatch")->Checked);   //Steven 20170518 (jou) : Out shuttle Y sensor 使用 LATCH
    ENABLE_OUT_SHUTTLEY_LATCH=EL<TCheckBox>("THandlerSystem", "chkOutShtYSensorByLatch")->Checked;
    WriteIniDataGeneral("System", "Use_AxisY_Sensor_2x3mode", EL<TCheckBox>("THandlerSystem", "chk2x3modeUseAxisYSensor")->Checked);   //Isaac 20180402 (Steven) 2x3 mode 用Y sensor 偵測Outshuttle殘料
    Use_AxisY_Sensor_2x3mode=EL<TCheckBox>("THandlerSystem", "chk2x3modeUseAxisYSensor")->Checked;

    WriteIniDataGeneral("System", "Bias_Mode_Use_Y_Sensor", EL<TCheckBox>("THandlerSystem", "chkBiasModeUseYSensor")->Checked);
    Bias_Mode_Use_Y_Sensor=EL<TCheckBox>("THandlerSystem", "chkBiasModeUseYSensor")->Checked;                      //Steven 20240805 : 1x2 Bias mode 用Y sensor 偵測Outshuttle殘料
//----------------------------------
    WriteIniDataGeneral("System", "SAFE_DOOR_AMOUNT", EL<TRadioGroup>("THandlerSystem", "rgSafeDoor")->ItemIndex);
    SAFE_DOOR_AMOUNT=EL<TRadioGroup>("THandlerSystem", "rgSafeDoor")->ItemIndex;
//----------------------------------
    WriteIniDataGeneral("Version", "Serial No", EL<TEdit>("THandlerSystem", "edtSeriaNo")->Text);
//----------------------------------
    //jou 2010-05-19 start : 負壓
    WriteIniDataGeneral("System", "INDEX_SUCKER_TYPE", EL<TRadioGroup>("THandlerSystem", "rgIndexSuckerType")->ItemIndex);
    INDEX_SUCKER_TYPE=EL<TRadioGroup>("THandlerSystem", "rgIndexSuckerType")->ItemIndex;
    //jou 2010-05-19 end
//Steven 20110310 240KG----------------------------------
    WriteIniDataGeneral("System", "INDEX_PRESS_TYPE", EL<TRadioGroup>("THandlerSystem", "rgIndexPressType")->ItemIndex);
    INDEX_PRESS_TYPE=(eIndexPressType)EL<TRadioGroup>("THandlerSystem", "rgIndexPressType")->ItemIndex;
//Steven 20180412 : For new HT-9045高度較高--------------
    WriteIniDataGeneral("Index", "EnableUserDefMaxContactHeight",   EL<TCheckBox>("THandlerSystem", "chkUserDefMaxContactHeight")->Checked);
    WriteIniDataGeneral("Index", "UserDefMaxContactHeight",         EL<TEdit>("THandlerSystem", "edtUserDefMaxContactHeight")->Text);
//Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
//=>
    WriteIniDataGeneral("Index", "EnableUser_Define_IndexZ_SafePos",   EL<TCheckBox>("THandlerSystem", "chkUser_Define_IndexZ_SafePos")->Checked);
    WriteIniDataGeneral("Index", "UserDefineIndexZSafePos",         EL<TEdit>("THandlerSystem", "edtUserDefineIndexZSafePos")->Text);
//<=
//Richard 20230107 : SPIL客戶自定義機台安裝高度(200~800間)
//Steven 20110627 Tray Vibration-------------------------
    WriteIniDataGeneral("System", "TRAY_VIBRATION", EL<TRadioGroup>("THandlerSystem", "rgTrayVibration")->ItemIndex);
    TRAY_VIBRATION=EL<TRadioGroup>("THandlerSystem", "rgTrayVibration")->ItemIndex;

    WriteIniDataGeneral("System", "USE_2nd_LOADER", EL<TRadioGroup>("THandlerSystem", "rg2ndLoader")->ItemIndex);    //Steven 20240822 : For HT-9046AU
    USE_2nd_LOADER=EL<TRadioGroup>("THandlerSystem", "rg2ndLoader")->ItemIndex;

    WriteIniDataGeneral("System", "TRAY_ARM_MODE", EL<TRadioGroup>("THandlerSystem", "rgTrayArmMode")->ItemIndex);   //Frank 20230419
    TRAY_ARM_MODE=EL<TRadioGroup>("THandlerSystem", "rgTrayArmMode")->ItemIndex;

    WriteIniDataGeneral("System", "LOADER_VIBRATION", EL<TRadioGroup>("THandlerSystem", "rgLoaderVibration")->ItemIndex);  //JerryYang 20191001 loader震動馬達
    USE_LOADER_VIBRATION=EL<TRadioGroup>("THandlerSystem", "rgLoaderVibration")->ItemIndex;
//Steven 20170330 (Wei) : For HT-9046LM-------------------------
    WriteIniDataGeneral("System", "USE_TRAY_ROBOT", EL<TRadioGroup>("THandlerSystem", "rgTrayRobot")->ItemIndex);
    USE_TRAY_ROBOT=EL<TRadioGroup>("THandlerSystem", "rgTrayRobot")->ItemIndex;

//Steven 20170330 (Wei) : For TSMC------------------------------
    WriteIniDataGeneral("System", "USE_LOADER_HINGE", EL<TRadioGroup>("THandlerSystem", "rgLoaderHinge")->ItemIndex);
    USE_LOADER_HINGE=EL<TRadioGroup>("THandlerSystem", "rgLoaderHinge")->ItemIndex;

//wei 20170418
    WriteIniDataGeneral("System", "USE_DIE_CLEAN", EL<TRadioGroup>("THandlerSystem", "rgDieClean")->ItemIndex);
    USE_DIE_CLEAN=EL<TRadioGroup>("THandlerSystem", "rgDieClean")->ItemIndex;

//Ifor 20150708 CKD FCM Clean Air-------------------------
    WriteIniDataGeneral("System", "USE_CKD_FCM_CleanAir", EL<TRadioGroup>("THandlerSystem", "rgCKDFCM")->ItemIndex);
    USE_CKD_FCM_CleanAir=EL<TRadioGroup>("THandlerSystem", "rgCKDFCM")->ItemIndex;

//wei 20150807 Catch Tray Model
    WriteIniDataGeneral("System", "USE_CATCH_TRAY_MODEL", EL<TRadioGroup>("THandlerSystem", "rgCatchTrayModel")->ItemIndex);
    USE_CATCH_TRAY_MODEL=EL<TRadioGroup>("THandlerSystem", "rgCatchTrayModel")->ItemIndex;

//Ifor 20260824 : Tray Arm hardware type, new type standby at middle of Empty and Color
    WriteIniDataGeneral("System", "USE_TRAY_ARM_TYPE", EL<TRadioGroup>("THandlerSystem", "rgTrayArmType")->ItemIndex);
    USE_TRAY_ARM_TYPE=EL<TRadioGroup>("THandlerSystem", "rgTrayArmType")->ItemIndex;

    //kevin 20200512  夾tray 遮版削短 避免沒有夾好 TRAY ，讓TRAY移動過程 掉下去炸到 Pass TRAY
    WriteIniDataGeneral("System", "UseCatchTrayBlock", EL<TRadioGroup>("THandlerSystem", "rgCatchTrayBlock")->ItemIndex);
    bNewCatchTrayblock=EL<TRadioGroup>("THandlerSystem", "rgCatchTrayBlock")->ItemIndex;

//Steven 20111011 : 使用16個Heater
    WriteIniDataGeneral("System", "USE_16_HEATER",  EL<TRadioGroup>("THandlerSystem", "rgHeater")->ItemIndex);

    filerw::ELTodo("golden HandlerSys.cpp:779-794 EN_HEATER_SHEET=1 branch not in the port -- per-station [TempCtrl] keys (g_tHeaterInsInfo[].m_asSaveName) are NOT written; HEATER_CTRL_TYPE is written as rgHeaterType (golden :793; golden :791 writes the common per-station value when all stations agree)"); WriteIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex);   // golden HandlerSys.cpp:779-794 取代：#if !EN_HEATER_SHEET（同 LoaderSystemSet :262-278）
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:779-794 -- #if !EN_HEATER_SHEET（同 LoaderSystemSet :262-278）
#if !EN_HEATER_SHEET                                                            //AI(ht9045-heater-control) 20260618 (RogerYang) : 移植創發2type溫控模組設定
    WriteIniDataGeneral("TempCtrl", "HEATER_CTRL_TYPE",  EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex);
#else
    { for(int ti=0; ti<eHeaterType_Count; ++ti)
      { int sIdx=TypeIdxToShowIdx(ti); int iInsOpt=INVALID_INT_VAL_NEG;
        if(INVALID_INT_VAL_NEG!=sIdx)
        { TComboBox *pCb=g_tHeaterInsInfo[ti].GetCtrlItem_Cb();
          if(NULL!=pCb) iInsOpt=pCb->ItemIndex; }
        g_tHeaterInsInfo[ti].m_iHeaterInsOpt=iInsOpt;
        WriteIniDataGeneral("TempCtrl", g_tHeaterInsInfo[ti].m_asSaveName, iInsOpt); }
      int iFirst=-1;
      if(IsAllSame_HeaterInsOpt(true,&iFirst) && 0<=iFirst && iFirst<eHeaterInsOpt_Count)
          WriteIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",iFirst);
      else
          WriteIniDataGeneral("TempCtrl","HEATER_CTRL_TYPE",EL<TRadioGroup>("THandlerSystem", "rgHeaterType")->ItemIndex); }
#endif      //Steven 20141030 : 新增OMRON E5DC溫控器
#endif // GATE (S12-C save)

//Steven 20110704 Real Time CCD--------------------------
    WriteIniDataGeneral("System", "REAL_TIME_CCD",  EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCD")->ItemIndex);
    //Isaac 20201217 : RTC CCD增加第二組感溫
    //=>
    if(EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCD")->ItemIndex==0)
    {
        WriteIniDataGeneral("System", "RTC_TemperNumber",  0);
    }
    else
    {
        WriteIniDataGeneral("System", "RTC_TemperNumber",  EL<TRadioGroup>("THandlerSystem", "rgRealTimeCCDTempNum")->ItemIndex);
    }
    //<=
    //Isaac 20201217 : RTC CCD增加第二組感溫
    WriteIniDataGeneral("System", "CCD2_TEMPER",    EL<TRadioGroup>("THandlerSystem", "rgCCDTemp")->ItemIndex);
    WriteIniDataGeneral("System", "LB_TEMP",        EL<TRadioGroup>("THandlerSystem", "rgLBTemp")->ItemIndex);       //Steven 20181023 : LB溫度
    WriteIniDataGeneral("System", "LB_TEMP_UpDown", EL<TRadioGroup>("THandlerSystem", "rgLBTemp2")->ItemIndex);      //Frank 20241231 : add
    WriteIniDataGeneral("System", "Index_ESDAir",   EL<TRadioGroup>("THandlerSystem", "rgESDTemp")->ItemIndex);      //kevin 20200207 add index ESD temp

//Steven 20120716 : OCR----------------------------------
    WriteIniDataGeneral("System", "INSTALL_OCR",    EL<TRadioGroup>("THandlerSystem", "rgOCR")->ItemIndex);
    WriteIniDataGeneral("OCR",    "OCR_COM_PORT",   EL<TComboBox>("THandlerSystem", "cbOCR")->Text);
    WriteIniDataGeneral("OCR",    "OCRwithTester_COM_PORT",   EL<TComboBox>("THandlerSystem", "cbOCRwithTester")->Text);

    WriteIniDataGeneral("System", "INSTALL_OCR_YMot",EL<TRadioGroup>("THandlerSystem", "rgOCRYStepMot")->ItemIndex);             //Frank 20250214 add
//Barcode_2D------------------------
    WriteIniDataGeneral("2D_BarCode", "BOTTOM_2DID",       EL<TRadioGroup>("THandlerSystem", "rgBottom2DID")->ItemIndex);        //Steven 20190308 : Bottom 2D
    if(EL<TRadioGroup>("THandlerSystem", "rgBottom2DID")->ItemIndex==1)
        WriteIniDataGeneral("2D_BarCode", "BAR_CODE_INSTALL",  ebctUseCCDMode);             //Steven 20120921 : Barcode_2D
    else
        WriteIniDataGeneral("2D_BarCode", "BAR_CODE_INSTALL",  EL<TRadioGroup>("THandlerSystem", "rg2DBarcode")->ItemIndex);     //Steven 20120921 : Barcode_2D

    WriteIniDataGeneral("2D_BarCode", "SHT_FLOATING_CHK",  EL<TRadioGroup>("THandlerSystem", "rgShuttleFloating")->ItemIndex);   //Steven 20160920 : IC置偏檢查
    WriteIniDataGeneral("2D_BarCode", "BAR_CODE_USECOUNT",   EL<TEdit>("THandlerSystem", "edCognexSystemCCD")->Text);      //Ifor 20160506 :add New ATC Interface Use Heat Count
    WriteIniDataGeneral("2D_BarCode", "BOTTOM_2DID_CCD",   EL<TRadioGroup>("THandlerSystem", "rgBottom2DID_CCD")->ItemIndex);    //KaiChen 20200513 : Bottom 2D 8CCD
//----------------------------------
    WriteIniDataGeneral("System", "IN_SHT_LAST_SENSOR",  EL<TRadioGroup>("THandlerSystem", "rgInShtLastSensor")->ItemIndex);     //Steven 20181203 : In Shuttle最後一個Sensor定義
    WriteIniDataGeneral("System", "SAFE_DOOR_LOCK", EL<TRadioGroup>("THandlerSystem", "rgSafeDoorLock")->ItemIndex);             //20111130  Dell
    WriteIniDataGeneral("System", "HOT_PLATE_POSITION", EL<TRadioGroup>("THandlerSystem", "rgHotPlatePos")->ItemIndex);          //Steven 20140222 : Hot Plate Pin的位置
    WriteIniDataGeneral("System", "HOT_PLATE_LIMITATION", EL<TRadioGroup>("THandlerSystem", "rgHotPlateLimit")->ItemIndex);      //Steven 20151117 : Hot Plate Pin的極限位置, 可以跑2x2 8吸嘴
    WriteIniDataGeneral("System", "USE_LASER_DISTANCE", EL<TRadioGroup>("THandlerSystem", "rgLaserDistance")->ItemIndex);        //Steven 20140228 : 雷射測距功能
    WriteIniDataGeneral("System", "USE_DEVICE_FLIPPER", EL<TRadioGroup>("THandlerSystem", "rgDeviceFlipper")->ItemIndex);        //Frank 20210612 : Flipper Function
    WriteIniDataGeneral("System", "MAGAZINE_BIN_DISP_TYPE", EL<TRadioGroup>("THandlerSystem", "rgMagBinDispType")->ItemIndex);   //JerryYang 20220909 : add magazine
//----------------------------------
    WriteIniDataGeneral("System", "MACHINE_HAS_AUTO_ALIGNMENT_CCD", EL<TRadioGroup>("THandlerSystem", "rgCCDAutoAlignmentMode")->ItemIndex);     //ChungHung 20210113 add for Alignment CCD  //KenHsieh 20210813 : add CCD AUTO ALIGNMENT

//Comm Port-------------------------                                            //Steven 20120217 : Com Port改成可定義
    AnsiString Str;
    Str="D:\\RS232Standard\\System";
    MyForceDirectories(Str, "THandlerSystem::SaveSystemSet");                   //檢查資料夾是否存在，不存在就建立一個新的
    Str+="\\Setup.ini";

    WriteIniDataGeneral("IndexDriver",   "COM_PORT",            EL<TComboBox>("THandlerSystem", "cbComIndex")->Text);
    WriteIniDataGeneral("TempCtrl",      "COM_PORT",            EL<TComboBox>("THandlerSystem", "cbComTemp")->Text);
    WriteIniDataGeneral("TempCtrl",      "COM_PORT_OMRON",      EL<TComboBox>("THandlerSystem", "cbComTempOmron")->Text);
    WriteIniDataGeneral("TempCtrl",      "COM_PORT_DYNAMIC",    EL<TComboBox>("THandlerSystem", "cbComDyTemp")->Text);
    WriteIniDataGeneral("NUMBER_PANEL",  "COM_PORT",            EL<TComboBox>("THandlerSystem", "cbComBinDisp")->Text);
    WriteIniDataGeneral("NUMBER_PANEL2", "COM_PORT",            EL<TComboBox>("THandlerSystem", "cbbComBinDisp2")->Text);

    WriteIniDataGeneral("RealTimeCCD",   "Port",                EL<TComboBox>("THandlerSystem", "cbComRTC")->Text);
    WriteIniData(Str,   "COMPort",       "CommName",            EL<TComboBox>("THandlerSystem", "cbComTester")->Text);
    WriteIniData(Str,   "COMPort_TTL",   "CommName",            EL<TComboBox>("THandlerSystem", "cbComTTLRS232")->Text);
    WriteIniData(Str,   "COMPort_TTL_2", "CommName",            EL<TComboBox>("THandlerSystem", "cbComTTLRS232_2")->Text);         //Isaac 20210309 :TTL RS232兩塊板子

    WriteIniDataGeneral("EM Aware",     "EM_AWARE_PORT1",       EL<TComboBox>("THandlerSystem", "cbEMAwarePort1")->Text);          //Steven 20131114 : ESD Monitor
    WriteIniDataGeneral("EM Aware",     "EM_AWARE_PORT2",       EL<TComboBox>("THandlerSystem", "cbEMAwarePort2")->Text);          //Steven 20131114 : ESD Monitor
    WriteIniDataGeneral("EM Aware",     "EM_AWARE_PORT3",       EL<TComboBox>("THandlerSystem", "cbEMAwarePort3")->Text);          //Steven 20131114 : ESD Monitor
    WriteIniDataGeneral("EM Aware",     "EM_AWARE_PORT4",       EL<TComboBox>("THandlerSystem", "cbEMAwarePort4")->Text);          //Steven 20131114 : ESD Monitor
    WriteIniDataGeneral("EM Aware",     "EM_AWARE_USE_4_COM",   EL<TCheckBox>("THandlerSystem", "cbESDUse4COM")->Checked);         //Steven 20131114 : ESD Monitor
    WriteIniDataGeneral("EM Aware",     "NOVX_3360_PORT",       EL<TComboBox>("THandlerSystem", "cbNovx3360")->Text);              //Steven 20131127 : 使用Simco ION風扇

    WriteIniDataGeneral("System",       "KASUGA_Fan_PORT",      EL<TComboBox>("THandlerSystem", "cbKASUGA_Fan")->Text);            //KaiChen 20191225 ：KASUGA Fan 通訊

    //ATC
    WriteIniDataGeneral("ATC",          "USE_ATC_MODE",         EL<TRadioGroup>("THandlerSystem", "rgATC")->ItemIndex);              //jou 2012-03-08 Enable ATC mode
    WriteIniDataGeneral("ATC",          "ATC1_COM_PORT",        EL<TComboBox>("THandlerSystem", "cbbATC1")->Text);
    WriteIniDataGeneral("ATC",          "ATC2_COM_PORT",        EL<TComboBox>("THandlerSystem", "cbbATC2")->Text);
    WriteIniDataGeneral("ATC",          "ATC3_COM_PORT",        EL<TComboBox>("THandlerSystem", "cbbATC3")->Text);
    WriteIniDataGeneral("ATC",          "ATC4_COM_PORT",        EL<TComboBox>("THandlerSystem", "cbbATC4")->Text);

    WriteIniDataGeneral("ATC",          "ATC_Mix_Mode",         EL<TRadioGroup>("THandlerSystem", "rgATCMixMode")->ItemIndex);              //jou 2012-03-08 Enable ATC mode

    WriteIniDataGeneral("ATC",          "ATC_SYSTEM_IP",        EL<TEdit>("THandlerSystem", "edATCSystemIP")->Text);
    if(ATC_SYSTEM==eNewATCSystem)                                                               //Ifor 20170620 (wei) add 使用New ATCSystem 強制設定Port為1234
    {
        WriteIniDataGeneral("ATC",      "ATC_SYSTEM_PORT",      1234);
        EL<TEdit>("THandlerSystem", "edATCSystemPort")->Text=1234;
    }
    else
    {
        WriteIniDataGeneral("ATC",      "ATC_SYSTEM_PORT",      EL<TEdit>("THandlerSystem", "edATCSystemPort")->Text);
    }
    WriteIniDataGeneral("ATC",          "ATC_SYSTEM_USEHEAT",   EL<TEdit>("THandlerSystem", "edATCSystemUseHeat")->Text);      //Ifor 20160506 :add New ATC Interface Use Heat Count

    WriteIniDataGeneral("System",       "USE_IN_OUT_ARM_Y_PITCH", EL<TRadioGroup>("THandlerSystem", "rgInOutArmYPitch")->ItemIndex); //jou 2012-05-15 : 選擇 Y Pitch 機構模式

    WriteIniDataGeneral("System",       "USE_OUT_ARM_Y_PITCH", EL<TRadioGroup>("THandlerSystem", "rgOutArmYPitch")->ItemIndex);      //JerryYang 20251218 : IN/OUT ARM支援不同模組

    WriteIniDataGeneral("System",       "IN_OUT_ARM_Y_PITCH_MIN", EL<TEdit>("THandlerSystem", "edtMinYPitch")->Text);
    WriteIniDataGeneral("System",       "IN_OUT_ARM_Y_PITCH_MAX", EL<TEdit>("THandlerSystem", "edtMaxYPitch")->Text);
    WriteIniDataGeneral("System",       "USE_IN_OUT_ARM_X_PITCH", EL<TRadioGroup>("THandlerSystem", "rgInOutArmXPitch")->ItemIndex); //Isaac 20171204 (Steven) : Xpitch40->50mm 可選擇X Pitch機構模式
    WriteIniDataGeneral("System",       "IN_OUT_ARM_X_PITCH_MIN", EL<TEdit>("THandlerSystem", "edtMinXPitch")->Text);          //JimmyChiu 20220708 : add Auto X pitch
    WriteIniDataGeneral("System",       "IN_OUT_ARM_X_PITCH_MAX", EL<TEdit>("THandlerSystem", "edtMaxXPitch")->Text);          //JimmyChiu 20220708 : add Auto X pitch
    WriteIniDataGeneral("System",       "BASE_X_TO_HP",          EL<TEdit>("THandlerSystem", "edtHPLimit")->Text);             //Steven 20230826 : 基準軸X到加熱盤邊緣的距離
    WriteIniDataGeneral("System",       "INSTALL_HEAT_GUN",     EL<TRadioGroup>("THandlerSystem", "rgHeatGun")->ItemIndex);          //kevin 20120523 : 選擇熱風槍機構模式
    WriteIniDataGeneral("System",       "INSTALL_ATC_HEAT_GUN", EL<TRadioGroup>("THandlerSystem", "rgATCHeatGun")->ItemIndex);       //JerryYang 20220408 : add for ATC3.5
    WriteIniDataGeneral("System",       "FIX3_FULL_PLACE",      EL<TRadioGroup>("THandlerSystem", "rgFix3FullPlace")->ItemIndex);    //Steven 20130126 : Fix3滿盤功能
    WriteIniDataGeneral("System",       "USE_MAGNETIC_SCALE",   EL<TRadioGroup>("THandlerSystem", "rgMagneticScale")->ItemIndex);    //Steven 20160426 : 磁性尺
    WriteIniDataGeneral("System",       "USE_PICKER_COUNT",     EL<TRadioGroup>("THandlerSystem", "rgPickerCount")->ItemIndex);      //Steven 20161117 : for HT-9045S
    WriteIniDataGeneral("System",       "USE_COLOR_TRAY_SENSOR", EL<TRadioGroup>("THandlerSystem", "rgColorSensor")->ItemIndex);     //20140903 wei colcr Tray
    WriteIniDataGeneral("System",       "USE_SOCKET_SENSOR",    EL<TRadioGroup>("THandlerSystem", "rgSocketSen")->ItemIndex);        //JerryYang 20200327 Socket sensor改用硬體選項判斷

    WriteIniDataGeneral("OutSortArm",   "USE_OUT_SORT_ARM",     EL<TRadioGroup>("THandlerSystem", "rgOutSortArm")->ItemIndex);       //Steven 20240822 : For HT-9046AU
    WriteIniDataGeneral("OutSortArm",   "USE_OUT_SORT_X_PITCH_MIN", EL<TEdit>("THandlerSystem", "edtOutSortXPitchMin")->Text);
    WriteIniDataGeneral("OutSortArm",   "USE_OUT_SORT_X_PITCH_MAX", EL<TEdit>("THandlerSystem", "edtOutSortXPitchMax")->Text);

    WriteIniDataGeneral("System",       "Canbus_Method",        EL<TRadioGroup>("THandlerSystem", "rgCanBusMethod")->ItemIndex);     //Sam 20210518 : 新增 CanBus 軟體配置

    WriteIniDataGeneral("System",       "USE_PRECISER",         EL<TRadioGroup>("THandlerSystem", "rgPreciser")->ItemIndex);         //Frank 20180410 (Steven) : InArm Preciser Station
    WriteIniDataGeneral("System",       "iPreciserInstallArea", EL<TRadioGroup>("THandlerSystem", "rgPreciserPos")->ItemIndex);      //Ifor 20191008 : add Preciser Install Area

    WriteIniDataGeneral("ROTATE_KIT",   "USE_ROTATE_KIT",       EL<TRadioGroup>("THandlerSystem", "rgRotateKit")->ItemIndex);        //Steven 20121001 : 旋轉Kit
    WriteIniDataGeneral("ROTATE_KIT",   "RotateKit_Type",       EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex);   //2013-04-12    Dell :旋轉站;馬達版

    if(EL<TRadioGroup>("THandlerSystem", "rgRotateKit_Type")->ItemIndex)
    {
        EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->ItemIndex =-1;
        EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->ItemIndex =-1;
    }
    WriteIniDataGeneral("ROTATE_KIT",   "iRotate_In_Index",     EL<TRadioGroup>("THandlerSystem", "rgRotateKitIn")->ItemIndex);      //Steven 20121001 : 旋轉Kit
    WriteIniDataGeneral("ROTATE_KIT",   "iRotate_Out_Index",    EL<TRadioGroup>("THandlerSystem", "rgRotateKitOut")->ItemIndex);     //Steven 20121001 : 旋轉Kit
    WriteIniDataGeneral("IndexDriver",  "USE_IO_CHANGE_TOQUE",  EL<TRadioGroup>("THandlerSystem", "rgIOChangeToque")->ItemIndex);    //jou 2012-06-21 Enable index I/O Change Toque
    WriteIniDataGeneral("IndexDriver",  "INDEX_DRIVER_TYPE",    EL<TRadioGroup>("THandlerSystem", "rgIndexMotorType")->ItemIndex);
    WriteIniDataGeneral("IndexDriver",  "USE_HP_COM_CARD",      EL<TCheckBox>("THandlerSystem", "chkUseHPComCard")->Checked);      //Steven 20210204 : 使用鴻勁自製的通訊卡
    WriteIniDataGeneral("IndexDriver",  "CLEAN_AIR",            EL<TRadioGroup>("THandlerSystem", "rgCleanAir")->ItemIndex);         //ChungHung 20120821 : Clean Air
    WriteIniDataGeneral("IndexDriver",  "USE_INDEX_ARM_AXES",   EL<TRadioGroup>("THandlerSystem", "rgIndexMotorAxis")->ItemIndex);   //JimmyChiu 20220708 : add Index Arm Axis
    WriteIniDataGeneral("System",       "USE_HOTPLATE_TYPE",    EL<TRadioGroup>("THandlerSystem", "rgHotplateType")->ItemIndex);     //jou 2012-05-15 : 選擇 Hotplate Type
    WriteIniDataGeneral("System",       "MOTION_CARD_TYPE",     EL<TRadioGroup>("THandlerSystem", "rgMotionCard")->ItemIndex);       //Brian 20121015 : 選擇Motion Card 模式
    WriteIniDataGeneral("System",       "MOTIONNET_SPEED",      EL<TRadioGroup>("THandlerSystem", "rgMNetSpeed")->ItemIndex);        //Steven 20181122 : MNet速度要可以修改
    WriteIniDataGeneral("System",       "IO_CARD_TYPE",         EL<TRadioGroup>("THandlerSystem", "rgIOCard")->ItemIndex);           //Brian 20121015 : 選擇I/O Card 模式
    WriteIniDataGeneral("System",       "TTL_CARD_TYPE",        EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex);          //Steven 20121122 : 選擇TTL Card 模式

    if(EL<TRadioGroup>("THandlerSystem", "rgTTLCard")->ItemIndex==3)                                                 //Isaac 20210922 : 選擇TTL板子是否帶站別，兩塊板子必帶站別
        WriteIniDataGeneral("System",       "TTL_CARD_USE_ADDRESS", 1);         //兩塊板子一定要有站別
    else
        WriteIniDataGeneral("System",       "TTL_CARD_USE_ADDRESS", EL<TRadioGroup>("THandlerSystem", "rgTTLUseAddress")->ItemIndex);

    WriteIniDataGeneral("System",       "SHUTTLE_Z_TYPE",       EL<TRadioGroup>("THandlerSystem", "rgShuttleZType")->ItemIndex);     //jou 2013-07-26 : 選擇 Shuttle Z Sensor Type
    WriteIniDataGeneral("System", "FIX3_INSTALL",          EL<TRadioGroup>("THandlerSystem", "rgInstallFix3")->ItemIndex);           //2013-07-16    Dell    9046LS第一台沒Fix3;第二台以上Fix3標配
    WriteIniDataGeneral("System", "CROSS_SENSOR_INSTALL",  EL<TRadioGroup>("THandlerSystem", "rgShuttleCrossSensor")->ItemIndex);    //2013-07-16    Dell    Shuttle cross sensor
    WriteIniDataGeneral("System", "AUTO_SENSOR_INSTALL",   EL<TRadioGroup>("THandlerSystem", "rgAutoShuttleSensor")->ItemIndex);     //wei 20160914 Auto Shuttle Sensor
    WriteIniDataGeneral("System", "ShuttleVibration",      EL<TRadioGroup>("THandlerSystem", "rgShuttleVibration")->ItemIndex);      //JerryYang 20171006 (wei) Shuttle 震動馬達
    WriteIniDataGeneral("System", "USE_TRAY_MAPPING",      EL<TRadioGroup>("THandlerSystem", "rgTrayMapping")->ItemIndex);           //wei 20161219 Tray Mapping
    WriteIniDataGeneral("System", "Fix_AI_CCD",            EL<TRadioGroup>("THandlerSystem", "rgFixAICCD")->ItemIndex);              //RogerYang 20180901 add 矽格湖口Demo AI CCD Function
    WriteIniDataGeneral("System", "USE_AUTO_ALIGNMENT",    EL<TRadioGroup>("THandlerSystem", "rgAutoAlignment")->ItemIndex);         //Sam 20181201 : AutoAlignment
    WriteIniDataGeneral("System", "USE_COLORSENSOR_MUN",   EL<TRadioGroup>("THandlerSystem", "rgCOLORSENSOR_MUN")->ItemIndex);       //Jimmychiu 20230630 : add color sensor MU-N in Loader
    WriteIniDataGeneral("System", "USE_ESD_Monior",        EL<TRadioGroup>("THandlerSystem", "rg3M_EM_AWARE_Monitor")->ItemIndex);   //2013-07-16     Dell    使用ESD monitor
    WriteIniDataGeneral("System", "USE_OTD",               EL<TRadioGroup>("THandlerSystem", "rgOTDInstall")->ItemIndex);            //Steven 20140312 : OTD
    WriteIniDataGeneral("System", "USE_NOVX3360",          EL<TRadioGroup>("THandlerSystem", "rgNovx3360")->ItemIndex);              //Steven 20131127 : 使用Simco ION風扇
    WriteIniDataGeneral("System", "USE_AutoCleanIonFan",   EL<TRadioGroup>("THandlerSystem", "rgAutoCleanIonFan")->ItemIndex);       //Isaac 20210609 : IO觸發IonFan清針
    WriteIniDataGeneral("System", "USE_PULSE_TYPE",        EL<TRadioGroup>("THandlerSystem", "rgUsePulseType")->ItemIndex);          //Ifor 20180316 : add Simco Use Pulse Type
    WriteIniDataGeneral("System", "ION_PULSE_COUNT",       EL<TEdit>("THandlerSystem", "edIONPulseCount")->Text);              //Ifor 20180316 : add Simco Use Pulse Type
    WriteIniDataGeneral("System", "USE_KASUGA",            EL<TRadioGroup>("THandlerSystem", "rgKasuga")->ItemIndex);                //Ifor 20150731 : 使用Kasuga ION風扇
    WriteIniDataGeneral("System", "USE_KASUGA_Fan",        EL<TRadioGroup>("THandlerSystem", "rgKasuga_Fan")->ItemIndex);            //KaiChen 20191225 ：KASUGA Fan 通訊
    WriteIniDataGeneral("System", "HTIonBarFunction",      EL<TRadioGroup>("THandlerSystem", "rgHTIonBar")->ItemIndex);              //RogerYang 20250825 : Unloader新增3支IonBar，取代4 5 8 ion fan
    WriteIniDataGeneral("System", "SocketBasedAdd4Temp",   EL<TRadioGroup>("THandlerSystem", "rgUse4DUT")->ItemIndex);               //Steven 20131008 : for HT9046AH
    WriteIniDataGeneral("System", "USE_46_SUCKER_DB",      EL<TRadioGroup>("THandlerSystem", "rgUseSucker_9046_DB")->ItemIndex);     //Steven 20131008 : HT9045使用46配氣
    WriteIniDataGeneral("System", "USE_46_SENSOR_DB",      EL<TRadioGroup>("THandlerSystem", "rgUseSensor_9046_DB")->ItemIndex);     //Steven 20131008 : HT9045使用46配電
    WriteIniDataGeneral("System", "INDEX_MOTION_CARD",     EL<TRadioGroup>("THandlerSystem", "rgIndexMotionCard")->ItemIndex);       //Steven 20210623 : Index使用Galil
    WriteIniDataGeneral("System", "ControlPanelMode",      EL<TRadioGroup>("THandlerSystem", "rg_ControlPanelMode")->ItemIndex);     //Ken 20210702 AddPadInterface
    WriteIniDataGeneral("System", "VacuUnitType",          EL<TRadioGroup>("THandlerSystem", "rgVacuUnitType")->ItemIndex);          //Sam 20230210 : 新增 VacuumUnit 通訊模組
    WriteIniDataGeneral("System",  "USE_FINE_PITCH",       EL<TRadioGroup>("THandlerSystem", "rgFinePitch")->ItemIndex);             //Steven 20210624 : Fine Pitch
    WriteIniDataGeneral("System", "USE_OUT_SHT_MOT",       EL<TRadioGroup>("THandlerSystem", "rgUseOutSht")->ItemIndex);             //Steven 20210624 : Out shuttle獨立馬達
    WriteIniDataGeneral("AIR_CON", "USE_AIR_CONDITIONER",  EL<TRadioGroup>("THandlerSystem", "rgAirConditioner")->ItemIndex);        //Steven 20131011 : 冷氣機
    WriteIniDataGeneral("AIR_CON", "AIR_CON_PORT",         EL<TComboBox>("THandlerSystem", "cbAirCon")->Text);                     //Steven 20131011 : 冷氣機
    WriteIniDataGeneral("Laser",   "COM_Laser_1",          EL<TComboBox>("THandlerSystem", "cbComLaser1")->Text);                  //Steven 20140228 : 雷射測距功能
    WriteIniDataGeneral("Laser",   "COM_Laser_2",          EL<TComboBox>("THandlerSystem", "cbComLaser2")->Text);                  //Steven 20140228 : 雷射測距功能
    WriteIniDataGeneral("Laser",   "COM_Laser_InArm",      EL<TComboBox>("THandlerSystem", "cbComLaserInArm")->Text);              //Steven 20140228 : 雷射測距功能
    WriteIniDataGeneral("System", "AOI",  EL<TRadioGroup>("THandlerSystem", "rgAOI")->ItemIndex);                                    //2014-04-07    Dell    //2014-04-07    Dell    for SPIL AOI
    WriteIniDataGeneral("Vibration", "VibrationCommunication",  EL<TRadioGroup>("THandlerSystem", "rgVibrationCommuncation")->ItemIndex);       //JerryYang 20230814 : add震動馬達通訊調速版本
    WriteIniDataGeneral("System", "EP_MAXKPA", EL<TEdit>("THandlerSystem", "edMaxKpa")->Text);
    WriteIniDataGeneral("System", "EP_MAXA"  , EL<TEdit>("THandlerSystem", "edMaxMpaFB")->Text);
    WriteIniDataGeneral("System", "EP_MINMPA"  , EL<TEdit>("THandlerSystem", "edMinMpa")->Text);                               //JerryYang 20171023 (wei) add PA Min
    WriteIniDataGeneral("System", "EP_MINA_FeedBack", EL<TEdit>("THandlerSystem", "edtMinMpaFB")->Text);
    WriteIniDataGeneral("System", "USE_MR_SYSTEM",EL<TRadioGroup>("THandlerSystem", "rgMRSystem")->ItemIndex);                       //wei 20180822 MR
    WriteIniDataGeneral("System", "I24V_PULSE_COUNT",         EL<TEdit>("THandlerSystem", "ed24VMonitorPulseCount")->Text);    //Ifor 20190311 : add 24V Monitor Use Pulse Type
    WriteIniDataGeneral("System", "USE_RFID_SYSTEM",        EL<TRadioGroup>("THandlerSystem", "rgRFIDSystem")->ItemIndex);           //wei 20180702 MR
    WriteIniDataGeneral("System", "USE_RFID_READER",        EL<TRadioGroup>("THandlerSystem", "rgRFIDReader")->ItemIndex);           //Steven 20220713 : RFID Reader for SJSEMI
    WriteIniDataGeneral("RFID",   "RFIDReader_PORT", EL<TComboBox>("THandlerSystem", "cbbRFIDReader")->Text);
    WriteIniDataGeneral("System", "CHAMBER_USE_PULSE_TYPE", EL<TRadioGroup>("THandlerSystem", "rgChamberUsePulseType")->ItemIndex);  //Ifor 20190422 : add Chamber Use Pulse Type
    WriteIniDataGeneral("System", "SocketSenAmpQty",        EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt")->ItemIndex);      //JerryYang 20200327 socket/ rotate/ color的ampifier數量可分別設定
    WriteIniDataGeneral("System", "RotateSenAmpQty",        EL<TComboBox>("THandlerSystem", "cbRotateSenAmpCnt")->ItemIndex);
    WriteIniDataGeneral("System", "ColorSenAmpQty",         EL<TComboBox>("THandlerSystem", "cbColorSenAmpCnt")->ItemIndex);

    WriteIniDataGeneral("System", "SocketSenAmpQty2nd",     EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt2nd")->ItemIndex);      //JerryYang 20200327 socket/ rotate/ color的ampifier數量可分別設定
    WriteIniDataGeneral("System", "SocketSenAmpQty3rd",     EL<TComboBox>("THandlerSystem", "cbSocketSenAmpCnt3rd")->ItemIndex);      //JerryYang 20260205 : add第三組Socket sensor(8+16+8)

    WriteIniDataGeneral("System", "VibrationCardQty",       EL<TComboBox>("THandlerSystem", "cbVibrationCardQty")->ItemIndex); //JerryYang 20230814 : add震動馬達通訊調速版本

    //Sam 20210518 : 新增 CanBus 軟體配置
    //<==
    WriteIniDataGeneral("System", "CanBusNudn1Qty",         EL<TComboBox>("THandlerSystem", "coCanBusNudn1")->ItemIndex);
    WriteIniDataGeneral("System", "Nudn1Macid11Qty",        EL<TComboBox>("THandlerSystem", "coNudn1Macid11")->ItemIndex);
    WriteIniDataGeneral("System", "Nudn1Macid12Qty",        EL<TComboBox>("THandlerSystem", "coNudn1Macid12")->ItemIndex);
    WriteIniDataGeneral("System", "Nudn1Macid13Qty",        EL<TComboBox>("THandlerSystem", "coNudn1Macid13")->ItemIndex);
    WriteIniDataGeneral("System", "Nudn1Macid14Qty",        EL<TComboBox>("THandlerSystem", "coNudn1Macid14")->ItemIndex);

    //<==
    //Sam 20210518 : 新增 CanBus 軟體配置

    WriteIniDataGeneral("System", "Scanner_AOI",  EL<TRadioGroup>("THandlerSystem", "rgScanner_AOI")->ItemIndex);        //Ifor 20190725 : add Scanner AOI
    WriteIniDataGeneral("System", "Top_Scanner_AOI",  EL<TRadioGroup>("THandlerSystem", "rgTopScanner_AOI")->ItemIndex); //Ifor 20200902 add: TFAMD Top AOI
    if(EP_Install!=0)                                                           //Steven 20140524 : 配合統一由外面讀取
    {
        EP_MAXKPA=CheckAndReadIniDataGeneral("System" , "EP_MAXKPA" , 499.0);
        EP_MAXAFB=CheckAndReadIniDataGeneral("System" , "EP_MAXA"   , 5.013);
        EP_MINMPA=CheckAndReadIniDataGeneral("System" , "EP_MINMPA" , 0.001);   //JerryYang 20171023 (wei) add PA Min
        EP_MinAFB=CheckAndReadIniDataGeneral("System" , "EP_MINA_FeedBack", 0.908);
    }

    WriteIniDataGeneral("2D_BarCode", "BarCode1_COM_PORT", EL<TComboBox>("THandlerSystem", "cb2DReader1")->Text);
    WriteIniDataGeneral("2D_BarCode", "BarCode2_COM_PORT", EL<TComboBox>("THandlerSystem", "cb2DReader2")->Text);
    WriteIniDataGeneral("2D_BarCode", "BarCode3_COM_PORT", EL<TComboBox>("THandlerSystem", "cb2DReader3")->Text);
    WriteIniDataGeneral("2D_BarCode", "BarCode4_COM_PORT", EL<TComboBox>("THandlerSystem", "cb2DReader4")->Text);

    //Steven 20140708 : 左左右加入機種的選擇
    Str="D:\\GPIB9045\\system\\general.ini";
    AnsiString Str2="9045GPIB";
    switch(EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex)
    {
        case 0: Str2="9045GPIB";        break;
        case 1: Str2="9046GPIB";        break;
        case 2: Str2="9045GPIB_12Site"; break;
        case 3: Str2="9046_32GPIB";     break;
        case 4: Str2="502GPIB";         break;
        case 5: Str2="1032GPIB";        break;
        case 6: Str2="7080GPIB";        break;
    }
    WriteIniData(Str, "Version", "Model", Str2);
    if(EL<TComboBox>("THandlerSystem", "cbHandlerModel")->ItemIndex==0)                                            //Steven 20190307 : Add for HT-9046LA
        WriteIniDataGeneral("Version", "SubModel", Type_None);
    else
        WriteIniDataGeneral("Version", "SubModel", EL<TRadioGroup>("THandlerSystem", "rgModel")->ItemIndex);

    WriteIniDataGeneral("Version", "Ver",                   asHandlerVersion);                      //Ifor 20161109 Handler Version Modify

    if(EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex==tTemp150)                                    //Steven 20250701 : 整合機台溫度設定
    {
        WriteIniDataGeneral("System",  "HighTemperatureSet150", 1);             //wei 20150617 改機最高溫150度
        WriteIniDataGeneral("System",  "HighTemperatureSet155", 0);             //Sam 20240118 新增 155度 模式
        WriteIniDataGeneral("System",  "HighTemperatureSet175", 0);             //Frank 20160705 add 改機最高溫150度
    }
    else if(EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex==tTemp155)
    {
        WriteIniDataGeneral("System",  "HighTemperatureSet150", 0);             //wei 20150617 改機最高溫150度
        WriteIniDataGeneral("System",  "HighTemperatureSet155", 1);             //Sam 20240118 新增 155度 模式
        WriteIniDataGeneral("System",  "HighTemperatureSet175", 0);             //Frank 20160705 add 改機最高溫150度
    }
    else if(EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex==tTemp175)
    {
        WriteIniDataGeneral("System",  "HighTemperatureSet150", 0);             //wei 20150617 改機最高溫150度
        WriteIniDataGeneral("System",  "HighTemperatureSet155", 0);             //Sam 20240118 新增 155度 模式
        WriteIniDataGeneral("System",  "HighTemperatureSet175", 1);             //Frank 20160705 add 改機最高溫150度
    }
    else
    {
        WriteIniDataGeneral("System",  "HighTemperatureSet150", 0);             //wei 20150617 改機最高溫150度
        WriteIniDataGeneral("System",  "HighTemperatureSet155", 0);             //Sam 20240118 新增 155度 模式
        WriteIniDataGeneral("System",  "HighTemperatureSet175", 0);             //Frank 20160705 add 改機最高溫150度
    }
    WriteIniDataGeneral("System", "HighTempLimit", EL<TRadioGroup>("THandlerSystem", "rgHighTempLimit")->ItemIndex); //Steven 20250701 : 整合機台溫度設定

    WriteIniDataGeneral("System",  "DewPoint_Hardware_Install", EL<TRadioGroup>("THandlerSystem", "rgDewpointHW")->ItemIndex);           //Steven 20191017 : 露點計
    WriteIniDataGeneral("System",  "INSTALL_SOCKET_CLAMP",  EL<TRadioGroup>("THandlerSystem", "rgSocketClamp")->ItemIndex);              //JerryYang 20160523 是否改機為SLK、SOCKET夾持汽缸
    WriteIniDataGeneral("System",  "INSTALL_DOUBLE_EP",     EL<TRadioGroup>("THandlerSystem", "rgDoubleEPControl")->ItemIndex);          //Ifor 20190104 : Add Double EP Control

    WriteIniDataGeneral("Ground_Man", "USE_GROUND_MAN",  EL<TRadioGroup>("THandlerSystem", "rgGroundMan")->ItemIndex);                   //Steven 20190828 : 通訊式GroundMan
    WriteIniDataGeneral("Ground_Man", "Ground_Man_COM_PORT", EL<TComboBox>("THandlerSystem", "cbbGroundMan")->Text);
    WriteIniDataGeneral("Ground_Man", "Ground_Man_ScanPoint",   EL<TRadioGroup>("THandlerSystem", "rgGroundMan_ScanPoint")->ItemIndex);      //KaiChen 20191005 ：通訊式GroundMan
    WriteIniDataGeneral("Ground_Man", "Ground_Man_AlarmOhm",    EL<TEdit>("THandlerSystem", "edGroundMan_AlarmOhm")->Text);            //KaiChen 20191005 ：通訊式GroundMan

    WriteIniDataGeneral("System",  "HotGunFlowEnable",              EL<TRadioGroup>("THandlerSystem", "rgHotGunFlow")->ItemIndex);               //KaiChen 20190729 ：Hot Gun Flow
    WriteIniDataGeneral("System",  "HotGunFlow_LineNo",             EL<TEdit>("THandlerSystem", "edHotGunFlow_LineNo")->Text);             //KaiChen 20190729 ：Hot Gun Flow
    WriteIniDataGeneral("System",  "HotGunFlow_DevNo",              EL<TEdit>("THandlerSystem", "edHotGunFlow_DevNo")->Text);              //KaiChen 20190729 ：Hot Gun Flow
    WriteIniDataGeneral("System",  "HotGunFlow_Gun1_ChannelNo",     EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun1_ChannelNo")->Text);     //KaiChen 20190729 ：Hot Gun Flow
    WriteIniDataGeneral("System",  "HotGunFlow_Gun2_ChannelNo",     EL<TEdit>("THandlerSystem", "edHotGunFlow_Gun2_ChannelNo")->Text);     //KaiChen 20190729 ：Hot Gun Flow

    switch(EL<TComboBox>("THandlerSystem", "cbIndEPCnt")->ItemIndex)                                               //JerryYang 20210413 : Add 4組獨立EP版本
    {
        case 0: Str2="16";      break;
        case 1: Str2="4";       break;
        case 2: Str2="8";       break;                                          //RogerYang 20260603 : Add 8EP
    }
    WriteIniDataGeneral("System", "Individual_EP_COUNT", Str2);
    WriteIniDataGeneral("System", "AUTO3_IS_MAGAZINE",      EL<TRadioGroup>("THandlerSystem", "rgAuto3Magazine")->ItemIndex);                    //JerryYang 20210610 : add Auto3 Magazine

    WriteIniDataGeneral("System", "USE_OHT_SYSTEM",                 EL<TRadioGroup>("THandlerSystem", "rgOHTSystem")->ItemIndex);                //KaiChen 20200716 ：OHT
    WriteIniDataGeneral("System", "USE_Multile_Empty",              EL<TRadioGroup>("THandlerSystem", "rgMultileEmpty")->ItemIndex);             //KaiChen 20200716 ：OHT
    WriteIniDataGeneral("System", "USE_KEYENCE_LOADER",             EL<TRadioGroup>("THandlerSystem", "rgLoaderKeyence")->ItemIndex);            //wei 20161219 Tray Mapping
    WriteIniDataGeneral("System", "USE_KEYENCE_EMPTY",              EL<TRadioGroup>("THandlerSystem", "rgEmptyKeyence")->ItemIndex);             //wei 20161219 Tray Mapping
    WriteIniDataGeneral("System", "USE_MultileEmptyTrayID_Keyence", EL<TRadioGroup>("THandlerSystem", "rgMultileEmptyTrayIDKeyence")->ItemIndex);
    WriteIniDataGeneral("System", "TRAY_MAPPING_GRAB",              EL<TRadioGroup>("THandlerSystem", "rgTrayMappingGrabImage")->ItemIndex);     //KenHsieh 20220923 : add Tray Map Throw IC Function
    WriteIniDataGeneral("System", "BASE_HEATER_COUNT",              EL<TRadioGroup>("THandlerSystem", "rgBaseHeaterCount")->ItemIndex);          //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "SHUTTLE_FLOODGATE",              EL<TRadioGroup>("THandlerSystem", "rgShuttleFloodgate")->ItemIndex);         //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "Tri_Temp_Machine",               EL<TRadioGroup>("THandlerSystem", "rgTriTempMachine")->ItemIndex);           //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "AirStream_Select",               EL<TRadioGroup>("THandlerSystem", "rgAirStreamSelect")->ItemIndex);          //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "Tri_Temperature_MaxDegree",      EL<TEdit>("THandlerSystem", "edtTriTemperature_MaxDegree")->Text);     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "Tri_Temperature_MinDegree",      EL<TEdit>("THandlerSystem", "edtTriTemperature_MinDegree")->Text);     //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "TriTemperature_TotalChannel",    EL<TEdit>("THandlerSystem", "edtTriTempTotalCh")->Text);               //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "SetHeaterTemp_MaxOutSht",        EL<TEdit>("THandlerSystem", "edtOutShtMaxTemp")->Text);                //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "SetHeaterTemp_MaxIndex",         EL<TEdit>("THandlerSystem", "edtIndexMaxTemp")->Text);                 //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "SetHeaterTemp_MaxBase",          EL<TEdit>("THandlerSystem", "edtBaseMaxTemp")->Text);                  //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "Total_Compressor",               EL<TEdit>("THandlerSystem", "edt_Total_Compressor")->Text);            //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "IndexDoorHeater",                EL<TRadioGroup>("THandlerSystem", "rg_IndexDoorHeater")->ItemIndex);         //Ztex 2023.04.19 Add HT-1032 TriTemp Function
    WriteIniDataGeneral("System", "SafePlcIO",                      EL<TRadioGroup>("THandlerSystem", "rgSafePlcIO")->ItemIndex);                //Jason 20230619 增加PLC_IO模組及安全相關IO.
    WriteIniDataGeneral("System", "DOUBLE_BELT_MODE",               EL<TRadioGroup>("THandlerSystem", "rgDoubleBeltMode")->ItemIndex);           //Ztex 2023.12.13 Add Double Belt Mode

    WriteIniDataGeneral("System", "USE_COVER_TRAYID",               EL<TRadioGroup>("THandlerSystem", "rgCoverTrayID")->ItemIndex);
    WriteIniDataGeneral("MachineDefine", "Tri_Temp_Machine",        EL<TRadioGroup>("THandlerSystem", "rgTriTempMachine")->ItemIndex);           //Ztex 2024.03.05 Add MachineDefine Tri_Temp_Machine
    WriteIniDataGeneral("System", "In_Shuttle_Auto_Latch",          EL<TRadioGroup>("THandlerSystem", "rgInShtAutoLatch")->ItemIndex);           //KenHsieh 20250722 : InSht sensor 改為2顆，並用Latch 判別疊料以及飛料
    WriteIniDataGeneral("System", "USE_LD_Rot_Arm",                 EL<TRadioGroup>("THandlerSystem", "rgLDCarRotAtm")->ItemIndex);              //RogerYang 20250828 add for Loader Rotate Arm
    WriteIniDataGeneral("System", "AGVModal",  EL<TRadioGroup>("THandlerSystem", "rgE84Sensor")->ItemIndex);                                     //Frank 20251204 add E84
    WriteIniDataGeneral("System", "USE_LdUldCassetteMode",          EL<TRadioGroup>("THandlerSystem", "rgLdUldCassetteMode")->ItemIndex);        //Ifor 20251216 add:Boat Carrier

}

// golden HandlerSys.cpp:1121  THandlerSystem::SaveBtnClick(TObject *Sender)
static void HS_SaveBtnClick()
{
    filerw::ELMark("SaveBtnClick");   // FileRW：存檔流程 trace（ack.trace）

    HS_SaveSystemSet();
    filerw::ELMessage("Please restart the program to active new parameters.", "請重新啟動程式以套用新參數。");   // golden HandlerSys.cpp:1124-1124 取代：Application->MessageBox(MB_OK) → ELMessage（golden 答 NO 也會跳這一則）
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:1124-1124 -- Application->MessageBox(MB_OK) → ELMessage（golden 答 NO 也會跳這一則）
    Application->MessageBox("Please restart the program to active new parameters.", NULL, MB_OK|MB_TOPMOST);
#endif // GATE (S12-C save)

}

// golden HandlerSys.cpp:1148  THandlerSystem::SaveSafeDoorSet()
static void HS_SaveSafeDoorSet()
{

    Sen[SnSafeDoor1].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor1")->Checked;
    Sen[SnSafeDoor2].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor2")->Checked;
    Sen[SnSafeDoor3].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor3")->Checked;
    Sen[SnSafeDoor4].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor4")->Checked;
    Sen[SnSafeDoor5].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor5")->Checked;
    Sen[SnSafeDoor6].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor6")->Checked;
    Sen[SnSafeDoor7].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor7")->Checked;
    Sen[SnSafeDoor8].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor8")->Checked;
    Sen[SnSafeDoor9].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor9")->Checked;
    Sen[SnSafeDoor10].Enable=EL<TCheckBox>("THandlerSystem", "SafeDoor10")->Checked;
    Sen[SnHeaterDoor].Enable=EL<TCheckBox>("THandlerSystem", "HeaterDoor1")->Checked;
    Sen[SnHeaterDoor2].Enable=EL<TCheckBox>("THandlerSystem", "HeaterDoor2")->Checked;
    if(SAFE_DOOR_AMOUNT==0)                                                     //7扇門
    {
        Sen[SnSafeDoor4].Enable=false;
        Sen[SnSafeDoor5].Enable=false;
        Sen[SnSafeDoor10].Enable=false;
    }
    else if(SAFE_DOOR_AMOUNT==1)                                                //8扇門
    {
        Sen[SnSafeDoor4].Enable=false;
        Sen[SnSafeDoor5].Enable=false;
    }
    else if(SAFE_DOOR_AMOUNT==2)                                                //9扇門
    {
        Sen[SnSafeDoor10].Enable=false;
    }

}

// golden HandlerSys.cpp:1179  THandlerSystem::ExitBtnClick(TObject *Sender)
static void HS_ExitBtnClick()
{

    EL<TTabSheet>("THandlerSystem", "tsCustomerCode")->TabVisible=false;
    HSys.ReadGeneralIni();                                                      //Steven 20190320 : Handler Sys讀檔獨立成Function
    HS_SaveSafeDoorSet();
    ;   // golden HandlerSys.cpp:1184-1184 取代：Close()：網頁的存檔不是離開鈕 —— FileRW/HSys_C.cpp 只在存檔成功後借用離開鈕的資料半段（ReadGeneralIni＋SaveSafeDoorSet），不關頁
#if 0 // GATE (S12-C save) golden HandlerSys.cpp:1184-1184 -- Close()：網頁的存檔不是離開鈕 —— FileRW/HSys_C.cpp 只在存檔成功後借用離開鈕的資料半段（ReadGeneralIni＋SaveSafeDoorSet），不關頁
    Close();
#endif // GATE (S12-C save)

}

