// ===========================================================================
//  FileRW/IniConfig_OCR.cpp -- golden TfOCR（OCR.cpp，V912）的 C 路入口：<配方>\AOI.Data 的 [OCR SETTING] 讀寫段。沒有頁面。
//
//  //AI(W906-FRW-S86) 20260926: 新檔（Steven 團隊，S86）。設定：tools/editlist/IniConfig_OCR.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 IniConfig_OCR.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    （CreateForm HT9045.cpp:240）＝ FileRW_IniConfig_OCR_Boot()：只建替身（DFM 設計期狀態）。golden 建構子（:72）沒有轉 ——
//                                     它只設協定字串、Tester 參數與模擬 UI 指標，沒有讀寫檔。不讀檔、不寫檔。
//    fOCR_ReadFile（:2197）        ＝ FileRW_IniConfig_OCR_ReadFile()：golden DoReadLastData main.cpp:9365（開機 :9993、換配方
//                                     :25722／:25770；移植樹兩者都走 tools/wb_serve.cpp 的 W906_DoReadLastData）。
//                                     AOI.Data [OCR SETTING] 19 鍵（ReadIniData，不補寫）→ IniConfig；Gerneral.ini [OCR SETTING]
//                                     "OCR Port"（CheckAndReadIniData：缺鍵時照 golden 補寫 24）→ IniConfig.iOCRPort；
//                                     iOCRTriggerMode 夾 0..1；INSTALL_OCR!=eocrUninstal 時送 MSG_CMD_Enable/DisableBarCode、
//                                     Enable/DisablePin1Function（fMain->SendMSG_CMD，移植樹 offline no-op）。
//    FormShow（:211）              ＝ FileRW_IniConfig_OCR_FormShow()：golden 開頁（fOCR_ReadFile → fOCR_DoIniDataToForm → 兩個勾選框的
//                                     客戶別 Visible）。⚠ 目前沒有呼叫者（沒有頁面）。
//    spbSaveClick（:2151）         ＝ FileRW_IniConfig_OCR_spbSaveClick()：golden 存檔鈕。⚠ 目前沒有呼叫者（沒有頁面、沒有 WS 指令、
//                                     沒有登記 PageDesc）。A02 → WriteIniData 逐鍵寫 AOI.Data [OCR SETTING] 19 鍵 → fOCR_ReadFile →
//                                     fMain->BackupSetupFile()（移植樹 offline no-op，forms/fMain.h:270）。
//
//  ---- golden 呼叫點 → 本檔函式（呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）------------------------------
//    開機／換配方：DoReadLastData :9365 fOCR->fOCR_ReadFile() → FileRW_IniConfig_OCR_ReadFile()
//                  （移植樹 wb_serve.cpp 原本寫「golden :9366 fOCR->fOCR_ReadFile()：移植樹沒有 TfOCR —— 未接」那一行；
//                    fLaserSensor 第二次之後、fSCKART 之前）。
//    開頁：FormShow :216-217（沒有頁面，不接）；存檔：spbSaveClick :2187（本檔 spbSaveClick 已含）。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    IniConfig 的 OCR 欄位（Config.h:67-75、:246、:258-268）：全樹一份。讀者：OCRInsp.cpp、asendic_Loader.cpp、csystem.cpp、
//    asortarm.cpp、uhome.cpp、forms/fLotInfo.cpp、forms/fOCR.cpp（IsOCRCommandTrigger／CheckOCRWordType）。
//    ⚠ OCRInsp.cpp 的 TU 內替身類別 W906OCR_TfOCRSeam（`#define fOCR W906OCR_fOCR`）把 IsOCRCommandTrigger 寫死 false、
//      CheckOCRWordType 寫死 true，不看 IniConfig —— 本檔讀進來的 iOCRTriggerMode／asOCRWordType 對那條 OCR 流程沒有作用，
//      要等 OCR 執行期（Jimmy）拿掉那個替身。
//    bShow：IniConfig_OCR.gen.inc 的 static（golden OCR.h:292，表單外沒有讀者）。
//
//  ---- AOI.Data 兩個寫者 ----------------------------------------------------------------------------------------
//    TFrmAOI（FileRW/AOISetup.*，S69）寫 [SETTING]／[DutOnOff_*]／[RS232]／[AOITRAY]；本檔寫 [OCR SETTING]。兩邊都是 WriteIniData
//    逐鍵寫（vclcompat TIniFile write-through ＝ Win32 WritePrivateProfileStringA 就地改一行），不整檔回寫 → 互不覆蓋。
//
//  ---- 已知缺口（交件報告列出）-----------------------------------------------------------------------------------
//    V912 OCR.dfm 沒有 rgOCRTriggerMode（V899 有）：golden V912 開頁／存檔會存取違規（VCL 跳錯誤框、事件後半不執行；存檔只寫到
//    前 14 鍵）；移植樹替身存在、ItemIndex 預設 0，不會出錯。
//    替身沒灌值就呼叫 spbSaveClick 會把 DFM 設計期值寫進 AOI.Data（edOCRSkip／edOCRRetry／edOCRWordCount="2"、
//    edOCRWordType="AANN**"、其餘 0／false）—— 呼叫前先 FileRW_IniConfig_OCR_FormShow()（golden 存檔鈕只有表單開著才按得到）。
// ===========================================================================
#include "FileRW/IniConfig_OCR.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TfOCR 建構（HT9045.cpp:240 CreateForm）的替身部分：DFM 設計期狀態 → 存檔流程讀的替身 → 容器替身與父子。
// 冪等。不讀檔、不寫檔。沒有 HTEditList，CreateForm 先後不影響註冊。
void FileRW_IniConfig_OCR_Boot()
{
    if (g_booted) return;
    OC_DfmItems();
    OC_DfmState();
    OC_CreateSaveProxies();
    OC_CreateContainerProxies();
    std::printf("FileRW IniConfig_OCR: TfOCR proxies ready (%d save reads) -- golden OCR.cpp (CreateForm HT9045.cpp:240); "
                "INSTALL_OCR=%d\n",
                (int)(sizeof(kOC_SaveReads) / sizeof(kOC_SaveReads[0])), INSTALL_OCR);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9365 `fOCR->fOCR_ReadFile();`（開機與換配方）。
// 讀 AOI.Data（不補寫）；Gerneral.ini 的 "OCR Port" 缺鍵時照 golden 補寫。寫的是 IniConfig 的 OCR 欄位。
void FileRW_IniConfig_OCR_ReadFile()
{
    if (!g_booted) FileRW_IniConfig_OCR_Boot();   // golden 的建構一定在 DoReadLastData 之前（CreateForm 先於 TfMain::FormShow）
    OC_fOCR_ReadFile();
    std::printf("ocr.* chain loaded: AOI.Data [OCR SETTING] -> IniConfig (Skip=%d Retry=%d WordCount=%d WordType=\"%s\" "
                "Blue=%d Red=%d TriggerMode=%d DisabledKeyin=%d CheckBarCodeMap=%d CheckIC=%d MoveSRead=%d "
                "Startposshift=%d/%d LightChange=%d LightNoDown=%d CompareOCRData=%d OCRAndBinLog=%d BinLogAddMark=%d "
                "CheckWordCount=%d) Gerneral.ini OCR Port=%d INSTALL_OCR=%d\n",
                IniConfig.iOCRSkip, IniConfig.iOCRRetry, IniConfig.iOCRWordCount, IniConfig.asOCRWordType.c_str(),
                IniConfig.iBlueLight, IniConfig.iRedLight, IniConfig.iOCRTriggerMode, (int)IniConfig.bDisabledKeyin,
                (int)IniConfig.bCheckBarCodeMap, (int)IniConfig.bEnabledOCRCheckIC, (int)IniConfig.bEnableOCRMoveSRead,
                (int)IniConfig.bEnableStartposshift, IniConfig.iStartposshift, (int)IniConfig.OCRLightChange,
                (int)IniConfig.OCRLightNoDown, (int)IniConfig.bCompareOCRData, (int)IniConfig.bOCRAndBinLog,
                (int)IniConfig.bOCRBinLogAddMark, (int)IniConfig.bEnabledOCRCheckWordCount, IniConfig.iOCRPort, INSTALL_OCR);
}

// golden 開頁 TfOCR::FormShow（:211）：fOCR_ReadFile → fOCR_DoIniDataToForm → 勾選框 Visible。目前沒有呼叫者（沒有頁面）；
// 給之後的頁面（PageDesc formShow／reload）與存檔前灌值用。
void FileRW_IniConfig_OCR_FormShow()
{
    if (!g_booted) FileRW_IniConfig_OCR_Boot();
    OC_FormShow();
}

// golden 存檔鈕 TfOCR::spbSaveClick（:2151）。目前沒有呼叫者（沒有頁面）；給之後的頁面／WS 指令用。
// 會寫 <DataPath>\<配方>\AOI.Data [OCR SETTING]，最後重讀（fOCR_ReadFile）與 golden fMain->BackupSetupFile()。
// ⚠ 呼叫前替身必須已是頁面值（見檔頭「已知缺口」）。
void FileRW_IniConfig_OCR_spbSaveClick()
{
    if (!g_booted) FileRW_IniConfig_OCR_Boot();
    OC_spbSaveClick();
}
