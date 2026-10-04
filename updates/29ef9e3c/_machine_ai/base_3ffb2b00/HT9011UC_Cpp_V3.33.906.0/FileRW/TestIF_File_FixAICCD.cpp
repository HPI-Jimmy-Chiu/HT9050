// ===========================================================================
//  FileRW/TestIF_File_FixAICCD.cpp -- golden TfFixAICCD（FixAICCD.cpp，V912）的 C 路入口：
//  <配方>\HandlerCondition.Data [Configuration]「Fix2 AI …」10 鍵的寫檔段＋開機／換配方 DoIniDataToForm。沒有頁面。
//
//  //AI(W906-FRW-S63) 20260926: 新檔（Steven 團隊，S63）。設定：tools/editlist/TestIF_File_FixAICCD.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 TestIF_File_FixAICCD.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    （CreateForm HT9045.cpp:266）＝ FileRW_FixAICCD_Boot()：只建替身（DFM 設計期狀態）。golden 建構子（:42）沒有轉 ——
//                                 它沒有讀寫檔，設的成員 DoIniDataToForm／spbSaveClick 都不讀（CCD 通訊／出料流程，Jimmy），
//                                 理由見設定檔檔頭。不讀檔、不寫檔。
//    DoIniDataToForm（:77）      ＝ FileRW_FixAICCD_DoIniDataToForm()：golden DoReadLastData main.cpp:9410（開機 :9993、換配方
//                                 :25722／:25770；移植樹兩者都走 tools/wb_serve.cpp 的 W906_DoReadLastData）。
//                                 HSys.asFix2BGAAICCDIP／Port＋TestIF_File 9 欄 → 替身；不改結構。
//                                 ⚠ 最後一行 CheckAndReadIniDataGeneral("AICCD","LockNoWaitAOIResult",1)：鍵不在就照 golden 補寫
//                                 D:\HT9045\system\Gerneral.ini（每台機台、每次開機／換配方都跑，不看 USE_Fix_AI_CCD）。
//                                 本機 Gerneral.ini:670-672 [AICCD] 已有這一鍵 → 不會寫。
//    spbSaveClick（:669）        ＝ FileRW_FixAICCD_spbSaveClick()：golden 存檔鈕。⚠ 目前沒有呼叫者（沒有頁面、沒有 WS 指令、
//                                 沒有登記 PageDesc）。寫 <DataPath>\<配方>\HandlerCondition.Data 10 鍵 → fFixAICCD->ReadFile()
//                                 （移植樹 forms/fFixAICCD.cpp 那一支）→ fMain->BackupSetupFile()（移植樹目前是 offline no-op，
//                                 forms/fMain.h:270 —— golden 會備份配方檔，移植樹沒有備份）。
//  讀檔器 ReadFile（:95）是移植樹 forms/fFixAICCD.cpp 的那一支（逐字、開機鏈 golden :9370 已接）；本檔不重複。
//
//  ---- golden 呼叫點 → 本檔函式（呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）------------------------------
//    開機／換配方：DoReadLastData :9370 fFixAICCD->ReadFile()（已接）→ :9410 FileRW_FixAICCD_DoIniDataToForm()
//                  （接在 :9407 FrmRotate->DoIniDataToForm 那一行；golden :9409 fTrayMapping->DoIniDataToForm 移植樹還沒有，S74）。
//    開頁：FormShow :124-127（fShow=true＋DoIniDataToForm）；關頁：FormClose :132-136（fShow=false＋DoIniDataToForm）—— 沒有頁面，不接。
//    TfMain::FormShow main.cpp:11325-11329（USE_Fix_AI_CCD 時 TimerDownFixAICCDConnect 開 CCD 連線＋ChangeFix2AICCDSetupFile 設捲軸
//      → OnChange → COM2 光源控制）：外部設備動作，不接（S48，Jimmy）。
//
//  ---- 存檔前替身必須已是頁面值 ------------------------------------------------------------------------------------
//    golden 的存檔鈕只有表單開著時按得到，而表單的元件在開機 DoReadLastData :9410 就已灌過值（FormShow／FormClose 各再灌一次）。
//    本檔的替身同樣在開機鏈 DoIniDataToForm 灌值；頁面接上時照 PageDesc 慣例：editlist.get＝FormShow、editlist.save＝套頁面值 →
//    FX_spbSaveClick。⚠ 替身清單 kFX_SaveReads 有 lblBGALightValue（golden 寫 Light Value 用的是這個標籤的 Caption）。
//
//  ---- golden 看起來錯（照翻，不修；交件報告列為決策題）----------------------------------------------------------
//    lblBGALightValue->Caption 只在 scrBGALightValueChange（InitialOK 之後）→ LightDataReflesh 才更新。golden 開機 DoReadLastData
//    （main.cpp:9993）早於 InitialOK=true（:10898），捲軸 OnChange 第一行 return；:11328 再設同值的 Position 不觸發 OnChange →
//    開機後沒動過捲軸就按存檔，「Fix2 AI CCD BGA Light Value」會寫成 DFM 的 '0'（FixAICCD.dfm:352）。移植樹沒有捲軸（GATE G-FX-BGALIGHT），
//    替身 Caption 照 golden DFM 種成 "0"（產生器的 DfmState 不收 TLabel Caption，所以在 Boot 這裡補）。
// ===========================================================================
#include "FileRW/TestIF_File_FixAICCD.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;
}  // namespace

// golden TfFixAICCD 建構（HT9045.cpp:266 CreateForm）的替身部分：DFM 設計期狀態 → 存檔流程讀的替身 → 容器替身與父子。冪等。
// 不讀檔、不寫檔。
void FileRW_FixAICCD_Boot()
{
    if (g_booted) return;
    FX_DfmItems();
    FX_DfmState();
    //AI(W906-FRW-S63) 20260926: golden FixAICCD.dfm:347-352 lblBGALightValue.Caption='0'（VCL 建構時載入）。產生器 DfmState 只收
    //   TEdit Text／TCheckBox Checked／ItemIndex，不收 TLabel Caption；不種的話替身是空字串，存檔會把 Light Value 寫成空值（golden 是 "0"）。
    EL<TLabel>("TfFixAICCD", "lblBGALightValue")->Caption = "0";
    FX_CreateSaveProxies();
    FX_CreateContainerProxies();
    std::printf("FileRW FixAICCD: TfFixAICCD proxies ready (%d save reads) -- golden FixAICCD.cpp (CreateForm HT9045.cpp:266); "
                "USE_Fix_AI_CCD=%d\n",
                (int)(sizeof(kFX_SaveReads) / sizeof(kFX_SaveReads[0])), USE_Fix_AI_CCD);
    g_booted = true;
}

// golden TfMain::DoReadLastData main.cpp:9410 `fFixAICCD->DoIniDataToForm();`（開機與換配方，:9370 ReadFile 之後）。
// 只寫替身；⚠ 會讀、缺鍵時會寫 Gerneral.ini [AICCD] LockNoWaitAOIResult（見檔頭）。
void FileRW_FixAICCD_DoIniDataToForm()
{
    if (!g_booted) FileRW_FixAICCD_Boot();   // golden 的建構一定在 DoReadLastData 之前（CreateForm 先於 TfMain::FormShow）
    FX_DoIniDataToForm();
    std::printf("fixaiccd.* DoIniDataToForm (golden main.cpp:9410): Enable=%d Learning=%d StartDelay=%d ExpTO=%d ResultTO=%d "
                "Retry=%d CycleInsp=%d Thres=%.3f ResultShowType=%d (rgResultShowType.Enabled=%d) CCD1=%s:%s CCD2=%s:%s\n",
                (int)TestIF_File.bEnableFix2BGAAICCD, (int)TestIF_File.bEnableLearningMode, TestIF_File.iFix2BGAAICCDStartDelay,
                TestIF_File.iFix2BGAAICCDExposureTimeOut, TestIF_File.iFix2BGAAICCDGetResultTimeOut, TestIF_File.iFix2BGAAICCDAutoRetry,
                TestIF_File.iFix2BGAAICCDOutArmCycleInsp, TestIF_File.dInspectResultThres, TestIF_File.iResultShowType,
                (int)EL<TRadioGroup>("TfFixAICCD", "rgResultShowType")->Enabled,
                HSys.asFix2BGAAICCDIP[0].c_str(), HSys.asFix2BGAAICCDPort[0].c_str(),
                HSys.asFix2BGAAICCDIP[1].c_str(), HSys.asFix2BGAAICCDPort[1].c_str());
}

// golden 存檔鈕 TfFixAICCD::spbSaveClick（:669）。目前沒有呼叫者（沒有頁面）；給之後的頁面／WS 指令用。
// 會寫 <DataPath>\<配方>\HandlerCondition.Data [Configuration] 10 鍵，最後重讀（fFixAICCD->ReadFile）。
void FileRW_FixAICCD_spbSaveClick()
{
    if (!g_booted) FileRW_FixAICCD_Boot();
    FX_spbSaveClick();
}
