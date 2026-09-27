// ===========================================================================
//  FileRW/AOISetup.cpp -- golden TFrmAOI（fAOI.cpp，V912）的 C 路入口：<配方>\AOI.Data 的讀寫段。沒有頁面。
//
//  //AI(W906-FRW-S69) 20260926: 新檔（Steven 團隊，S69）。設定：tools/editlist/AOISetup.py（每條 replace／block 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 AOISetup.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    （CreateForm HT9045.cpp:234）＝ FileRW_AOISetup_Boot()：只建替身（DFM 設計期狀態）。golden 建構子（:160）沒有轉 ——
//                                 非 Top&Bottom 機台它沒有讀寫檔（iQuotient／iRemainder=0 是本 TU 的 static 零初值），
//                                 Top&Bottom 的 TTopBottomInspect＋elParameter 不在移植樹（見設定檔檔頭）。不讀檔、不寫檔。
//    fAOI_ReadFile（:3369）     ＝ FileRW_AOISetup_ReadFile()：golden DoReadLastData main.cpp:9374（開機 :9993、換配方
//                                 :25722／:25770；移植樹兩者都走 tools/wb_serve.cpp 的 W906_DoReadLastData）。
//                                 AOI.Data → tAOISetup／ScannerAOIIF（cprod.h）＋ bVitroxBGA/PADViewUse/Map（forms/fAOI.cpp）＋
//                                 MOT[MMScanAOI].Tray.Data（[AOITRAY] 三鍵都在且夠長才寫）→ fAOI_DoIniDataToForm（替身）→
//                                 UpDataTrayData（InitialOK 之後才把 MOT[MMScanAOI] 的 X/YItem 對齊 MOT[MManualTray2]）。
//                                 只用 ReadIniData —— 不補寫缺鍵，不改任何檔。
//    spbSaveClick（:3120）      ＝ FileRW_AOISetup_spbSaveClick()：golden 存檔鈕。⚠ 目前沒有呼叫者（沒有頁面、沒有 WS 指令、
//                                 沒有登記 PageDesc）。寫 <DataPath>\<配方>\AOI.Data（[SETTING]／[DutOnOff_*]／[RS232]／[AOITRAY]）→
//                                 fAOI_ReadFile（同一份讀檔器）→ fMain->BackupSetupFile()（移植樹目前是 offline no-op，
//                                 forms/fMain.h:270）。RS232 起停、CCD 燈、Top&Bottom 的 elParameter 存檔：ELTodo（見設定檔）。
//
//  ---- golden 呼叫點 → 本檔函式（呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）------------------------------
//    開機／換配方：DoReadLastData :9374 FrmAOI->fAOI_ReadFile() → FileRW_AOISetup_ReadFile()
//                  （移植樹 wb_serve.cpp 原本寫「:9375 FrmAOI->fAOI_ReadFile()…未接」那一行；fTeach->ReadFile 之後、fAGV 之前）。
//    開頁：FormShow :3776 fAOI_ReadFile（沒有頁面，不接）；存檔：spbSaveClick :3338（本檔 spbSaveClick 已含）。
//    uteach.cpp:1822（TfTeach 開頁）／:2390（Teach 存 Top&Bottom 座標後按 FrmAOI->spbSave）：只在 Top&Bottom 機台 → 不接。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    tAOISetup／ScannerAOIIF：cprod.cpp:58／:103（全樹一份）。bVitroxBGA/PADViewUse/Map：forms/fAOI.cpp（golden 檔案層級全域的家，
//    AOI 執行期將來也用同一份）。iQuotient／iRemainder：AOISetup.gen.inc 的 static（golden private，只有讀寫檔兩支用）。
//    門面 forms/fAOI.h 的 14 個元件（CheckFailBin 等用的）在 Boot 最先登記成同名替身（ELKeep），門面與本檔讀到同一組物件。
//
//  ---- 已知缺口（交件報告列出）-----------------------------------------------------------------------------------
//    ScannerAOIIF.bEnabledPositionByAOI：移植樹 cprod.h 沒有這一欄 → 讀／顯示／寫三處擋，存檔時這一鍵保持檔案原值。
//    Top&Bottom（USE_Scanner_AOI_Inspection==eBtnAOI_TopBottomInstall）：[TopBottomInspect]／[ZPickOffset]／[Function Setting]
//    共 70 多鍵沒有讀進任何結構、存檔保持檔案原值；Top&Bottom 機台開機會印一行提醒、存檔回報 todo。
//
//  ---- 偏離 golden --------------------------------------------------------------------------------------------------
//  //AI(W906-FRW-S152) 20260927 [W906] Q31＝A'（Steven，RULINGS_20260926 S152；906 單邊修，接受 AOI.Data 格式跟 BCB6 機台不一致，V912 不改）：
//    spbSaveClick 兩行（golden fAOI.cpp:3300／:3301，原行保留在 AOISetup.gen.inc 的 #if 0）：
//      Top 延遲 edt_SDelayTopScanAOI 改寫 "StartDelayTimeTopScanAOIView"（golden 寫進 Bottom 的 "StartDelayTimeScanAOIView"）；
//      Top Timeout 鍵名統一成讀檔用的 "TimeOutScanTopAOIView"（golden 寫 "TimeOutTopScanAOIView"、讀 "TimeOutScanTopAOIView"）。
//    fAOI_ReadFile 照 golden 不動 ⇒ BCB6 寫的舊檔在 906 讀到的值跟 BCB6 相同；906 存的檔 BCB6 也讀得到正確的 Top 值。
//    理由與另一個選項：tools/editlist/AOISetup.py 檔頭。
// ===========================================================================
#include "FileRW/AOISetup.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TFrmAOI 建構（HT9045.cpp:234 CreateForm）的替身部分：門面元件 → DFM 設計期狀態 → 存檔流程讀的替身 → 容器替身與父子。
// 冪等。不讀檔、不寫檔。
void FileRW_AOISetup_Boot()
{
    if (g_booted) return;
    //AI(W906-FRW-S69) 20260926: 門面 forms/fAOI.h 已有的 14 個同名同型別元件直接當替身（同 gen_editlist 的 adopt，但門面寫的是
    //   `new TComboBox()` 不是 `new vclcompat::TComboBox()`，產生器的 adopt 認不得，所以在這裡手動 ELKeep）。要在任何 EL<> 之前
    //   （AO_DfmItems 會先建 ComboBox1／cbAOIFialAndTestPass／rgAOIFailBinType），否則會多出一份、門面的 CheckFailBin 讀不到頁面值。
    filerw::ELKeep("TFrmAOI", "ComboBox1", FrmAOI->ComboBox1);
    filerw::ELKeep("TFrmAOI", "edt_FailPADView", FrmAOI->edt_FailPADView);
    filerw::ELKeep("TFrmAOI", "edt_FailBGAView", FrmAOI->edt_FailBGAView);
    filerw::ELKeep("TFrmAOI", "edt_PassFailBGAPADView", FrmAOI->edt_PassFailBGAPADView);
    filerw::ELKeep("TFrmAOI", "edt_FailFailBGAPADView", FrmAOI->edt_FailFailBGAPADView);
    filerw::ELKeep("TFrmAOI", "cbEnabledTopView", FrmAOI->cbEnabledTopView);
    filerw::ELKeep("TFrmAOI", "cbEnabledPADView", FrmAOI->cbEnabledPADView);
    filerw::ELKeep("TFrmAOI", "cbEnabledBGAView", FrmAOI->cbEnabledBGAView);
    filerw::ELKeep("TFrmAOI", "rgAOIFailBinType", FrmAOI->rgAOIFailBinType);
    filerw::ELKeep("TFrmAOI", "lblAOIBinSel1", FrmAOI->lblAOIBinSel1);
    filerw::ELKeep("TFrmAOI", "lblAOIBinSel2", FrmAOI->lblAOIBinSel2);
    filerw::ELKeep("TFrmAOI", "cbAOIFialAndTestPass", FrmAOI->cbAOIFialAndTestPass);
    filerw::ELKeep("TFrmAOI", "btnSimulateTopBtm", FrmAOI->btnSimulateTopBtm);
    filerw::ELKeep("TFrmAOI", "blTopBtmTask", FrmAOI->blTopBtmTask);
    AO_DfmItems();
    AO_DfmState();
    AO_CreateSaveProxies();
    AO_CreateContainerProxies();
    std::printf("FileRW AOISetup: TFrmAOI proxies ready (%d save reads) -- golden fAOI.cpp (CreateForm HT9045.cpp:234); "
                "USE_AOI_Inspection=%d USE_Scanner_AOI_Inspection=%d USE_Top_Scanner_AOI_Inspection=%d\n",
                (int)(sizeof(kAO_SaveReads) / sizeof(kAO_SaveReads[0])), USE_AOI_Inspection, USE_Scanner_AOI_Inspection,
                USE_Top_Scanner_AOI_Inspection);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9374 `FrmAOI->fAOI_ReadFile();`（開機與換配方）。只讀檔；寫的是結構、MOT[MMScanAOI] 盤面資料、替身。
void FileRW_AOISetup_ReadFile()
{
    if (!g_booted) FileRW_AOISetup_Boot();   // golden 的建構一定在 DoReadLastData 之前（CreateForm 先於 TfMain::FormShow）
    AO_fAOI_ReadFile();
    std::printf("aoi.* chain loaded: AOI.Data -> tAOISetup (EnabledAOI=%d Top=%d PAD=%d BGA=%d AlarmCount=%d) ScannerAOIIF "
                "(ScannerMode=%d TopScannerMode=%d StartDelay=%d TimeOut=%d FailBinType=%d IfError=%d Gain=%.2f LGA=%d "
                "Com=%s/%s TrayChunks=%d)\n",
                (int)tAOISetup.bEnabledAOI, (int)tAOISetup.tTopView.bEnabled, (int)tAOISetup.tPADView.bEnabled,
                (int)tAOISetup.tBGAView.bEnabled, tAOISetup.iAlarmCount,
                ScannerAOIIF.iEnableScannerMode, ScannerAOIIF.iEnableTopScannerMode, ScannerAOIIF.iStartDelayTime,
                ScannerAOIIF.iTimeOut, ScannerAOIIF.iAOIFailBinType, ScannerAOIIF.ScannerIfError, ScannerAOIIF.fScannerICGain,
                (int)ScannerAOIIF.bScanAOIUseLGAMode, ScannerAOIIF.sSCANNER_ComPort.c_str(), ScannerAOIIF.sSCANNER_BaudRate.c_str(),
                iQuotient);
}

// golden 存檔鈕 TFrmAOI::spbSaveClick（:3120）。目前沒有呼叫者（沒有頁面）；給之後的頁面／WS 指令用。
// 會寫 <DataPath>\<配方>\AOI.Data，最後重讀（fAOI_ReadFile）與 golden fMain->BackupSetupFile()。
// ⚠ 呼叫前替身必須已是頁面值：golden 的存檔鈕只有表單開著時按得到，開頁 FormShow 會先 fAOI_ReadFile 灌值；
//   開機 ReadFile 也灌過一次（本檔 FileRW_AOISetup_ReadFile）。沒灌值就存，會把 DFM 設計期值（多半是 0／空字串）寫進 AOI.Data。
void FileRW_AOISetup_spbSaveClick()
{
    if (!g_booted) FileRW_AOISetup_Boot();
    AO_spbSaveClick();
}
