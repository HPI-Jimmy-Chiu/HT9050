// ===========================================================================
//  FileRW/ACTForm.cpp -- golden TACTForm（AutoTemperature.cpp，V912；Steven 20210510 自動 K 溫）的 C 路入口：
//  D:\HT9045\System\AutoTemperature.ini（FilePath）的讀寫段。表單不在移植樹；沒有頁面（golden 主畫面 sbAutoTemp 鈕，
//  CosFunction.bAutoKTemp 才顯示，main.cpp:24389／:29883-29890 權限 49 → ACTForm->Show()）。
//
//  //AI(W906-FRW-S108) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S108）。設定：tools/editlist/ACTForm.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 ACTForm.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    建構子（:289）          ＝ FileRW_ACTForm_Boot()：bCommConnect／bAutoStart=false、_cbBase[]／_pnlBaseTemp[]、:297 LoadACTData(ACTData)、
//                              iGroupSet／iTotalGroupCount（DeltaDTB4824 時）、:398 FilePath=…、TimerACT 停。量測點面板 myATPal 不建（GUI）。
//    FormShow（:419）        ＝ FileRW_ACTForm_FormShow()：LoadACTData → 11 個設定元件；LoadCommData → 5 個 COM 元件；基準溫度顯示
//                              （Temperature.fLowBase／fMiddBase／fHighBase）。OpenCommPort、TimerACT 啟動改記 ELTodo（交 Jimmy）。
//    btnUpdateClick（:692）  ＝ FileRW_ACTForm_btnUpdateClick()：元件 → ACTData → SaveACTData；元件 → ACT_Com → SaveCommData。
//    palSaveLogClick（:1355）＝ FileRW_ACTForm_palSaveLogClick()：MemoOffset → D:\HT9045_Log\AutoTempCalibration\<時間>.txt。
//    ⚠ 後三支目前都沒有呼叫者（沒有頁面、沒有 WS 指令）。
//
//  ---- golden 讀寫時機（本檔提供函式，呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）--------------------------
//    開機：CreateForm(TACTForm)（HT9045.cpp:221，TfContactForce :220 之後）→ 建構子 → FileRW_ACTForm_Boot()。
//          ⚠ golden :297 LoadACTData 在 :398 設 FilePath **之前**：那一刻 FilePath 是空字串 → TIniFile("") 什麼都讀不到 → ACTData 全是
//          預設值（iThermoCtrlType=1＝DeltaDTB4824、CheckIntervalTime=3、CalibrationRange=2、dCalibrationRange=2.0×3、ReadScanTime=3、
//          OffsetLimit=30、SingleOffset=5、Colums=1、OffsetMethod=1）。**開機不讀 AutoTemperature.ini、也不寫任何檔**。照 golden 留著。
//          （Win32 GetPrivateProfileString 給空檔名＝找不到檔，回預設值；vclcompat TIniFile("") 同樣讀不到檔。）
//    開頁：FormShow :428 才真的讀 AutoTemperature.ini（[ACT]／[Display]／[COMPort]；TIniFile::Read* 不補寫）。
//    存檔：btnUpdateClick → SaveACTData（寫 11 鍵）＋SaveCommData（寫 [COMPort] 5 鍵）；檔不在會建。
//    換配方：不讀（AutoTemperature.ini 不跟配方）。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    全部是本 TU 的 static（ACTData、FilePath、CommName、iGroupSet…，見 ACTForm.gen.inc 的 members）：golden 表單外沒有讀者
//    （全樹只有 main.cpp:29889 ACTForm->Show()）。ACTCom（golden DFM TComm）＝ACT_Com，沒有人 StartComm。
//
//  ---- 不在本檔（通訊／機台動作，交 Jimmy）--------------------------------------------------------------------------
//    OpenCommPort（:616）／CloseCommPort（:598）／SendCommand（:652）／ACTComReceiveData（:543）、TimerACTTimer（:887，自動 K 溫：
//    讀溫度計、算偏移、改 Temperature.fTempOffSet —— 會改溫控參數）、btnAutoStartClick（:745）、ChangeStartState（:794）、DTB4824_ReadPV（:1363）、
//    TCP 那組（btConnect／btDisConnect／btACTTrigger／ClientSocketACTRead／ACTConnectTimerTimer／ReceiceData）、FormClose（:534）、
//    量測點面板 GUI（TMyATPanel、ShowDefaultPos、UpdateTemperatureData、btnDutOnOffClick、btShowPosClick、FormResize）。
// ===========================================================================
#include "FileRW/ACTForm.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TACTForm 建構（HT9045.cpp:221 CreateForm）：DFM 設計期 Items／狀態 → 存檔流程讀的替身 → 容器替身與父子 → 建構子（:289）。
// 冪等。不讀檔、不寫檔（golden :297 那一次 LoadACTData 的 FilePath 還是空的，見檔頭）。
// 前提：LoadMachineConfig 之後（建構子讀 iSocketBaseTempCount，Gerneral.ini）。
void FileRW_ACTForm_Boot()
{
    if (g_booted) return;
    ACT_DfmItems();
    ACT_DfmState();
    ACT_CreateSaveProxies();
    ACT_CreateContainerProxies();
    ACT_TACTForm();
    g_booted = true;
    std::printf("FileRW ACTForm: TACTForm ctor :289 done (%d save reads) -- golden LoadACTData(:297) ran with FilePath=\"\" "
                "(set at :398) so ACTData = defaults: ThermoCtrlType=%d CheckInterval=%d CalRange=%d Range=%g/%g/%g ScanTime=%d "
                "OffsetLimit=%d SingleOffset=%d Colums=%d OffsetMethod=%d; FilePath now %s (read only when the page opens)\n",
                (int)(sizeof(kACT_SaveReads) / sizeof(kACT_SaveReads[0])), ACTData.iThermoCtrlType, ACTData.iCheckIntervalTime,
                ACTData.iCalibrationRange, ACTData.dCalibrationRange[0], ACTData.dCalibrationRange[1], ACTData.dCalibrationRange[2],
                ACTData.iReadScanTime, ACTData.iOffsetLimit, ACTData.iSingleOffset, ACTData.iDisplayColumn, ACTData.iOffsetMethod,
                FilePath.c_str());
}

// golden TACTForm::FormShow（:419）：開頁，讀 AutoTemperature.ini。目前沒有呼叫者（沒有頁面）。
void FileRW_ACTForm_FormShow()
{
    if (!g_booted) FileRW_ACTForm_Boot();
    ACT_FormShow();
}

// golden TACTForm::btnUpdateClick（:692）：寫 AutoTemperature.ini（[ACT] 10 鍵、[Display] Colums、[COMPort] 5 鍵）。
// 目前沒有呼叫者；給之後的頁面用（頁面值先套到替身，見 kACT_SaveReads）。
void FileRW_ACTForm_btnUpdateClick()
{
    if (!g_booted) FileRW_ACTForm_Boot();
    ACT_btnUpdateClick();
}

// golden TACTForm::palSaveLogClick（:1355）：MemoOffset（K 溫過程的紀錄，只有 TimerACTTimer 會填 —— 未移植，所以現在是空的）
// → D:\HT9045_Log\AutoTempCalibration\YYYY-MM-DD hh_mm_ss.txt。⚠ golden 建的資料夾是 OffsetPath（:1358 MyForceDirectories(OffsetPath)），
// 不是 AutoTempCalibration；那個資料夾不在時 golden SaveToFile 丟 EFCreateError，vclcompat 的 SaveToFile 不寫也不報。照 golden 留著。目前沒有呼叫者。
void FileRW_ACTForm_palSaveLogClick()
{
    if (!g_booted) FileRW_ACTForm_Boot();
    ACT_palSaveLogClick();
}
