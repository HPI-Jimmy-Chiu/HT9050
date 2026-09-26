// ===========================================================================
//  FileRW/HSys_C.cpp -- 結構 HSys（Handler System）的讀寫檔，C 路（具名替身）。
//    system\Gerneral.ini（asGeneralPath，WriteIniDataGeneral）＋ D:\RS232Standard\System\Setup.ini
//    ＋ D:\GPIB9045\system\general.ini（[Version] Model）。
//
//  Steven 團隊 20260925.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md「Gerneral.ini（形狀 E）」。
//
//  ⚠ 檔名：FileRW/HSys.cpp 是前一版 A 形狀（tools/formbridge/THandlerSystem.py 產生），整合時退役；
//    gen_editlist.py 的 _editlist_sources.cmake 列的是 FileRW/<struct>.cpp ＝ FileRW/HSys.cpp —— 整合者要把本檔改名
//    成 HSys.cpp（或改清單），否則建置會編到 A 形狀那支。
//
//  golden THandlerSystem（HT9011UC_Code_V3.33.912.0_20260908_Jimmy HandlerSys.cpp）由 tools/gen_editlist.py
//  （tools/editlist/HSys.py）轉成 HSys.gen.inc（元件改成具名替身）：
//    建構子（:63）＝ slCustomerCode；
//    FormShow（:132）＝開頁（SortItemToMap（純畫面）→ LoaderSystemSet :177 → LoaderSafeDoorSet :1132 → GPIB 型號）；
//    SaveBtnClick（:1121）＝存檔鈕（SaveSystemSet :575：YES/NO → 逐鍵 WriteIniDataGeneral／WriteIniData → 部分全域立即改）。
//  沒有 HTEditList —— 存檔流程讀的替身全部是 mustSend。
//
//  Steven 20260925 裁決：開頁照 golden FormShow、讀檔照 golden SYSTEM_MODULAR::ReadGeneralIni（移植樹 database.cpp:313，
//  LoadMachineConfig 開機跑）；golden 開表單補寫缺鍵、建 D:\RS232Standard\System、補寫 GPIB general.ini 的行為照 golden 保留。
//  editlist.get／editlist.save 都在主迴圈跑（呼叫端持 FormLock），HSys.ReadGeneralIni() 在這裡呼叫沒有 HTTP 執行緒問題
//  （審查第 8 輪 H-1）。
//
//  bHandlerModel（golden TfMain::FormShow main.cpp:9589-9594）：
//    golden 開機 HSys.ReadGeneralIni()（database.cpp:302）讀 D:\GPIB9045\system\general.ini [Version] Model，不在白名單 →
//    bHandlerModel=false 並提早 return（其餘 Gerneral.ini 全域都沒讀）；TfMain::FormShow 看到 false 就 MessageDlg ＋
//    Application->Terminate()，整個程式停住 —— 任何表單都開不到、存不到。
//    網頁版不可以結束程式 → 開頁與存檔都先檢查：false 時 ELMessage（golden 原字串）＋ ELMark("model_read_error")，
//    不跑 golden FormShow（它會 CheckAndReadIniDataGeneral 補寫 Gerneral.ini —— golden 在這個狀態下根本走不到）、
//    拒絕存檔。頁面要把這則訊息當告警顯示。
//
//  存檔後跑不跑 HSys.ReadGeneralIni()（判斷，Steven 團隊 20260925）：跑 —— 只在「真的寫了檔」（YES 且型號正常）之後，
//    照 golden 離開鈕 ExitBtnClick（:1179）的資料那一半：HSys.ReadGeneralIni() → SaveSafeDoorSet()。理由：
//    * golden 的存檔鈕只寫檔＋改一部分全域（AUTO_EMPTY_COLOR、InOutArmPickerUseMotor、LOAD_Z_USE_MOTOR[]、CUSTOMER_CODE…），
//      另一部分（NUMBER_PANEL_TYPE、各 COM 埠、ATC…）要等離開鈕的 ReadGeneralIni 才進記憶體；golden 使用者存完按「Exit」
//      離開（表單正規的離開路徑），兩個鈕合起來記憶體＝檔案。網頁沒有「離開」事件，存完不跑就永遠停在半新半舊。
//    * 不跑的只有 golden 用右上角 X 關（FormClose :1215，不重讀）這條路 —— 那是例外路徑，不是 golden 的設計意圖。
//    * 沒寫檔（答 NO、型號錯）時不跑：golden 離開鈕此時重讀的是原檔，結果與不跑相同（SaveSafeDoorSet 的門數覆寫除外，
//      那只在 SAFE_DOOR_AMOUNT 改過之後才有差，而改它必須先存檔）。
//    * 不跑離開鈕的 Close()：網頁存完頁面仍開著（不記 "closed"）。
//    * 照 golden 不重跑 TfMain 建構子那兩段（FileRW_HSys_ReadMainCtorKeys）：EP_Install／ION_FAN_TYPE 的新值 golden 也要
//      重開程式才生效（SaveSystemSet :592 `//EP_Install=…` 是 golden 自己註解掉的）。
//    ⚠ 副作用：ReadGeneralIni 會重讀 config.ini（ReadLastSetIni → IniConfig）、CustomerFunctionSelect、補寫 Gerneral.ini 缺鍵；
//      SaveSafeDoorSet 改 Sen[SnSafeDoor1..10／SnHeaterDoor／SnHeaterDoor2].Enable（安全門感測器啟用，golden 同樣在離開鈕做）。
//
//  開機：golden HT9045.cpp:210 CreateForm(THandlerSystem)（在 TfConfiguration :207 之後）。
// ===========================================================================
#include "FileRW/HSys.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

// ---------------------------------------------------------------------------
// golden TfMain::TfMain（main.cpp）建構子裡讀 Gerneral.ini 的兩段，逐行翻譯。
// Steven 團隊 20260925：移植樹沒有 TfMain 建構子的這兩段（grep `EP_Install\s*=`／`ION_FAN_TYPE\s*=`／
// `InOutArmPickerUseMotor\s*=` 20260925：只有 cmydef.cpp 的初值、FileRW/HSys.cpp（A 形狀存檔）與 tests）⇒ 三個全域一直是
// cmydef.cpp 的初值（0／1／0），EP_MAXKPA… 也一直是 0.0；頁面顯示初值、存檔把初值寫回 EP_Install／INOUT_ARM_PICKER_USE_MOTOR／
// ION_FAN_TYPE（A 形狀回報的 sourceGap）。
// 呼叫時機：golden 的 SYSTEM_MODULAR HSys 是全域物件，建構子（ReadGeneralIni）在 TfMain 建構之前跑 ⇒ 整合者在
// LoadMachineConfig()（＝HSys.ReadGeneralIni()）之後、FileRW_HSys_Boot() 之前呼叫一次。
// golden 缺鍵時問 YES/NO：開機不能問 → 照 MessageBox(MB_YESNO) 的預設鈕（第一顆 YES；golden 沒給 MB_DEFBUTTON2）處理，
// printf 說明。INOUT_ARM_PICKER_USE_MOTOR 的題目 golden 自己已註解掉（:1801-1802），照 golden 直接寫 1。
// ---------------------------------------------------------------------------
void FileRW_HSys_ReadMainCtorKeys()
{
    int iTemp;
//----------------------------------                                            // golden main.cpp:1772
    if(CheckIniData(asGeneralPath, "System", "EP_Install")==false)
    {
        iTemp=1;
        // golden :1776  if(Application->MessageBox("Machine have install Electrons-Press(EP)?", "EP", MB_YESNO | MB_TOPMOST)!=IDYES) iTemp=0;
        std::printf("FileRW HSys: Gerneral.ini [System] EP_Install missing -- golden main.cpp:1776 asks \"Machine have install Electrons-Press(EP)?\" "
                    "(YES/NO); boot cannot ask, MB_YESNO default button YES taken => EP_Install=1 written\n");
        WriteIniDataGeneral("System", "EP_Install", iTemp);
        EP_Install=iTemp;
    }
    else
    {
        EP_Install=CheckAndReadIniDataGeneral("System", "EP_Install", 0);
        if(EP_Install!=0)                                                       //Steven 20140524 : 配合統一由外面讀取
        {
            EP_MAXKPA=CheckAndReadIniDataGeneral("System" , "EP_MAXKPA" , 499.0);
            EP_MAXAFB=CheckAndReadIniDataGeneral("System" , "EP_MAXA"   , 5.013);
            EP_MINMPA=CheckAndReadIniDataGeneral("System" , "EP_MINMPA" , 0.001);                                       //JerryYang 20171023 (wei) add PA Min
            EP_MinAFB=CheckAndReadIniDataGeneral("System" , "EP_MINA_FeedBack", 0.908);

            EPDual_MAXKPA=CheckAndReadIniDataGeneral("System" , "EPDual_MAXKPA" , 899.0);                               //kevin 20200325 add dual force EP
            EPDual_MAXAFB=CheckAndReadIniDataGeneral("System" , "EPDual_MAXAFB"   , 4.905);
            EPDual_MINMPA=CheckAndReadIniDataGeneral("System" , "EPDual_MINMPA" , 0.001);
            EPDual_MinAFB=CheckAndReadIniDataGeneral("System" , "EPDual_MinAFB", 0.968);                                //kevin 20200325 add dual force EP
        }
    }
//----------------------------------                                            // golden main.cpp:1797
    if(CheckIniData(asGeneralPath, "System", "INOUT_ARM_PICKER_USE_MOTOR")==false)
    {
        iTemp=1;
//        if(Application->MessageBox("In/Out Arm ZA to ZH is use motor control?", "In/Out Arm", MB_YESNO | MB_TOPMOST) != IDYES)
//            iTemp=0;
        WriteIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", iTemp);
        InOutArmPickerUseMotor=iTemp;
    }
    else
    {
        InOutArmPickerUseMotor=CheckAndReadIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", 1);

        if(InOutArmPickerUseMotor==0)                                           //Steven 20240603 : 強制寫1
        {
            InOutArmPickerUseMotor=1;
            WriteIniDataGeneral("System", "INOUT_ARM_PICKER_USE_MOTOR", InOutArmPickerUseMotor);
        }
    }                                                                           // golden main.cpp:1815
    // golden :1817-1828 SUPPORT_2_EMPTY_EMPTY 不在這裡：golden 912 已由 ReadGeneralIni 讀（HandlerSys.cpp:179 的註解
    // 「RogerYang 20250823:搬去ReadGeneralIni()」；移植樹 database.cpp:547），ReadGeneralIni 的 CheckAndRead 先補了鍵，
    // main.cpp 那一段的題目在 golden 也不會再出現。

    if(CheckIniData(asGeneralPath, "System", "ION_FAN_TYPE")==false)            //Steven 20100226 : ION Fan    // golden main.cpp:2042
    {
        iTemp=0;
        // golden :2045  if(Application->MessageBox("Machine Using KEYENCE's ION FAN?", NULL, MB_YESNO | MB_TOPMOST)==IDYES) iTemp=1; else iTemp=0;
        iTemp=1;
        std::printf("FileRW HSys: Gerneral.ini [System] ION_FAN_TYPE missing -- golden main.cpp:2045 asks \"Machine Using KEYENCE's ION FAN?\" "
                    "(YES/NO); boot cannot ask, MB_YESNO default button YES taken => ION_FAN_TYPE=1 written\n");
        WriteIniDataGeneral("System", "ION_FAN_TYPE", iTemp);
        ION_FAN_TYPE=iTemp;
    }
    else
    {
        ION_FAN_TYPE=CheckAndReadIniDataGeneral("System", "ION_FAN_TYPE", 1);
    }                                                                           // golden main.cpp:2055
    std::printf("FileRW HSys: golden TfMain ctor keys read (main.cpp:1772-1815, :2042-2055): EP_Install=%d InOutArmPickerUseMotor=%d ION_FAN_TYPE=%d\n",
                EP_Install, InOutArmPickerUseMotor, ION_FAN_TYPE);
}

// golden TfMain::FormShow main.cpp:9589-9594 的網頁版（見檔頭）。回 true ＝ 型號讀取失敗（golden 在這裡停機）。
static bool HS_ModelReadError()
{
    if(bHandlerModel==false)                                                    //jou 20200601 : GPIB 型號讀取失敗需Alarm,不應該回寫型號
    {
        // golden: MessageDlg("D:\\GPIB9045\\system\\general.ini \"Model\" read error!!", mtConfirmation, TMsgDlgButtons()<<mbOK, 0);
        //         Application->Terminate(); return;
        filerw::ELMessage("D:\\GPIB9045\\system\\general.ini \"Model\" read error!!",
                          "D:\\GPIB9045\\system\\general.ini 的 \"Model\" 讀取失敗（golden 會結束程式；網頁版不開頁資料、不存檔）");
        filerw::ELMark("model_read_error");
        return true;
    }
    return false;
}

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// editlist.get：golden FormShow（型號錯時不跑）
void OpenPage()
{
    if (HS_ModelReadError()) return;
    HS_FormShow();
}

// editlist.save：golden 存檔鈕；真的寫了檔 → golden 離開鈕的資料半段（理由見檔頭）
void SaveFlow()
{
    if (HS_ModelReadError()) return;                 // golden 停機：走不到存檔
    HS_SaveBtnClick();
    if (filerw::ELMarked("SaveSystemSet:write")) {
        filerw::ELMark("ExitBtnClick(data half): HSys.ReadGeneralIni + SaveSafeDoorSet");
        HS_ExitBtnClick();
    }
}

// 沒寫檔時把替身還原成檔案值 —— golden 重開表單（FormShow）。型號錯時不跑（它會補寫 Gerneral.ini）。
void Reload()
{
    if (bHandlerModel == false) return;
    HS_FormShow();
}

const filerw::PageDesc kPage = {
    "HSys", "THandlerSystem", "HW.HandlerSys.html",
    nullptr, nullptr, 0,
    kHS_SaveReads, (int)(sizeof(kHS_SaveReads) / sizeof(kHS_SaveReads[0])),
    &OpenPage, &SaveFlow, "SaveSystemSet:write", &Reload, &Booted,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden THandlerSystem 建構（HT9045.cpp:210）：DFM 設計期狀態 → 建構子 → 存檔流程讀的替身
void FileRW_HSys_Boot()
{
    if (g_booted) return;
    HS_DfmItems();
    HS_DfmState();
    HS_THandlerSystem();
    HS_CreateSaveProxies();
    HS_CreateContainerProxies();
    std::printf("FileRW HSys: THandlerSystem proxies ready (%d save reads) -- golden HandlerSys.cpp\n",
                (int)(sizeof(kHS_SaveReads) / sizeof(kHS_SaveReads[0])));
    g_booted = true;
}
