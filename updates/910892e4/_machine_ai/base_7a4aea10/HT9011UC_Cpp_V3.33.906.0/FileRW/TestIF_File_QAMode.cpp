// ===========================================================================
//  FileRW/TestIF_File_QAMode.cpp -- TestIF_File 的 TfQAMode 半邊（<recipe>\Tester.Data [QA Mode]／[QA Sampling]）讀寫檔，C 形狀。
//  頁面：web/page/Setup.QAMode.html（頁面補件 web/page/ht9045_qamode_c.js）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_QAMode.py（每條 replace 附原因）。
//
//  golden TfQAMode（QAMode.cpp，V912）由 tools/gen_editlist.py 轉成 TestIF_File_QAMode.gen.inc：
//    FormShow（:113）＝開頁（重建 cbQAModeBin／cbbQASampleTray 的 Items → ReadFile（:179）→ DoIniDataToForm → 權限／顯示）；
//    btnApplyClick（:73）＝存檔鈕（A02 守衛 → DoFormToData → Tester.Data 12 鍵 → BackupSetupFile → ReadFile →
//    DoIniDataToForm → fBinSel->ReadFile）。沒有 HTEditList：存檔流程讀的 11 個具名替身全部是 mustSend。
//  savedMark＝"DoFormToData"：golden 只有通過 A02 守衛才呼叫它，之後無條件寫 12 鍵（:92-105）。
//  reload＝golden FormClose（:171，fShow=false＋DoIniDataToForm）：golden A02 路徑的 Close() 之後跑的就是它
//    （TestIF_File 沒被改過，DoIniDataToForm 把替身還原成記憶體值）。
//
//  ---- 開機與換配方（golden 呼叫點）-----------------------------------------------------------------------
//    golden TfMain::DoReadLastData（main.cpp:9264）：:9351 fQAMode->ReadFile()、:9407 fQAMode->DoIniDataToForm()。
//    DoReadLastData 的呼叫點：TfMain::FormShow :9995（開程式）、TfMain::cbSetupFileNameChange :25724／:25772（換工作檔）。
//    另 cBinSel.cpp:1126 TfBinSel::ReadFile 開頭呼叫 fQAMode->ReadFile()（移植樹 cBinSel.cpp:1415 GATE(G1)）、
//    uLotInfo.cpp:11706（EQC 模式存檔）。（行號＝D:\HT9045 的 V912；D:\HT9045_ref 同名樹 main.cpp 少 2 行）
//    ReadFile：移植樹 forms/fQAMode.cpp:346 TfQAMode::ReadFile 與 golden 912 :179-233 逐句相同，開機鏈仍用它
//      （wb_serve.cpp 兩處）；本檔另給 FileRW_QAMode_ReadFile()（golden 912 版，與開頁同一支）給整合者選用。
//    DoIniDataToForm：FileRW_QAMode_DoIniDataToForm()（golden :9407；移植樹 forms/fQAMode.cpp 那一支仍是 GATE(Q-1)）——
//      替身＋fLotInfo->edQAMode（golden :40 主畫面 EQC 計數）。
//
//  ---- 不在這張表單（照 V912 的讀寫位置）------------------------------------------------------------------
//    config.ini [Index] QAMode_BackupTestSiteUse<Z>／QAMode_BackupUseTestSocket<Z>：golden QABackupStatus（QAMode.cpp:286-336，
//      檔案層級函式），呼叫點 TfMain::SetRunStartMode main.cpp:546-551（CosFunction.bVerifyMode，切進／切出 rsmQAMode 時備份／還原
//      SiteMap，還原時另寫 Tester.Data bQAD22DoubleContact=false／bQATrayEndCloseYield100Site=true）。是模式切換的副作用，
//      不是存檔鈕 —— 本檔不轉；移植樹 forms/fQAMode.cpp GATE(Q-F)。
//    Image1Click（:235，點圖改 TestIF_File.iQATrayDirect）：gbLoaderDirection 只有 CosFunction.bQAmodeSupplyTrayDir 才顯示
//      （CosFunction.cpp:2041 AMKOR_China／:2478 RF360／:2583 QUALCOMM）＝客戶專屬，Steven 20260925 決定先跳過；
//      產生器把 Image1->Picture 那行當畫面處理，Image1 沒有替身 → 頁面點不動、存檔照 golden 寫記憶體裡的 iQATrayDirect。
// ===========================================================================
#include "FileRW/TestIF_File_QAMode.gen.inc"

#include <cstdio>
#include <string>

#include "FileRW/_EditPage.h"
#include "WebBridge/JsonWriter.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// PageDesc::extraJson：golden FormShow（:115-124）執行期重建的兩個下拉選項 —— DFM 的選項（cbQAModeBin 0..16、
// cbbQASampleTray Auto 1..Fix 3）不是 golden 執行期的選項（iTestBinCount 個；Prod.iTrayType[i]==tTrayAuto 的 s6TrayName）。
// 頁面（ht9045_qamode_c.js）在引擎套值之前照這份重建。
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("items").BeginObject();
    for (const char* n : {"cbQAModeBin", "cbbQASampleTray"}) {
        TStringList* it = EL<TComboBox>("TfQAMode", n)->Items;
        w.Key(n).BeginArray();
        for (int i = 0; i < it->Count; ++i) w.String(it->Strings[i].c_str());
        w.EndArray();
    }
    w.EndObject();
    w.EndObject();
    return w.Str();
}

const filerw::PageDesc kPage = {
    "TestIF_File_QAMode", "TfQAMode", "Setup.QAMode.html",
    nullptr, nullptr, 0,
    kQA_SaveReads, (int)(sizeof(kQA_SaveReads) / sizeof(kQA_SaveReads[0])),
    &QA_FormShow, &QA_btnApplyClick, "DoFormToData", &QA_FormClose, &Booted,
    nullptr, &ExtraJson,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfQAMode 建構（HT9045.cpp:228 CreateForm）：DFM 設計期狀態 → 建構子（空）→ 存檔流程讀的替身 → 容器替身與父子。
// 不讀檔（golden 讀檔在 DoReadLastData，見檔頭）。
void FileRW_QAMode_Boot()
{
    if (g_booted) return;
    QA_DfmItems();
    QA_DfmState();
    QA_TfQAMode();
    QA_CreateSaveProxies();
    QA_CreateContainerProxies();
    std::printf("FileRW TestIF_File_QAMode: TfQAMode proxies ready (%d save reads) -- golden QAMode.cpp\n",
                (int)(sizeof(kQA_SaveReads) / sizeof(kQA_SaveReads[0])));
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9351 fQAMode->ReadFile()（golden 912 版；移植樹 fQAMode->ReadFile() 逐句相同）
void FileRW_QAMode_ReadFile()
{
    QA_ReadFile();
}

// golden TfMain::DoReadLastData main.cpp:9407 fQAMode->DoIniDataToForm()（替身＋fLotInfo->edQAMode）
void FileRW_QAMode_DoIniDataToForm()
{
    if (g_booted) QA_DoIniDataToForm();
}
