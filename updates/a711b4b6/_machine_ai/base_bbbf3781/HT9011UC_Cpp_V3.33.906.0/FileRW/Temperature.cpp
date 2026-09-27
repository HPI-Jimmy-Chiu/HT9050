// ===========================================================================
//  FileRW/Temperature.cpp -- Temperature（SYSTEM_TEMPERATURE）的讀寫檔，C 形狀（具名替身＋直接跑 golden 存檔鈕）。
//
//  Steven 團隊 20260925.  規格：.claude/skills/ht9045-json-bridge/references/write-inventory.md §四（Temperature）、
//  porting-gaps.md 十三（ATC.ini；開機要先呼叫 W906_ReadATCIni，forms/fATCHandlerSide.cpp）。
//
//  golden TfTemp_Set（uTemp_Set.cpp，912）由 tools/gen_editlist.py 轉成 Temperature.gen.inc：
//    FormShow（:433）＝開頁（golden ReadTempFile(true)（:1986，尾端 DoIniDataToForm(true)）＋顯示／權限）；
//    spbSaveClick（:4238）＝存檔鈕（A02 守衛 → SECS 檢查 → 溫度補償上限檢查 → 回溫守衛 → FFC 時序檢查
//    → SaveSetupFile（:4606，Temperature.Data／Tester.Data／Config\ATC.ini）→ DefineTemp\*.Data 補償檔
//    → dSingleTempLimit＋SaveLastSetIni → BackupSetupFile → ReadTempFile(true) → DoIniDataToForm(true)）。
//  沒有 HTEditList —— 存檔流程讀的具名替身全部是 mustSend。
//
//  元件：移植樹 forms/fTemp_Set.h 的同名同型別元件直接當替身（TS_AdoptPortWidgets，ELKeep），
//  myTempPal[] 用移植樹 fTemp_Set->myTempPal（fTemp_Set->Init()＝golden 建構子建的那一份），每通道的資料元件
//  以 "myTempPal<i>_<成員>" 登記（TS_AdoptPanels）＝頁面元件 id 合約。
//
//  開機順序（整合者，tools/wb_serve.cpp 的 temp.* 鏈）：
//    fTemp_Set = new TfTemp_Set();
//    FileRW_Temperature_Boot();        // golden CreateForm：DFM 設計期狀態（在建構子之前）
//    fTemp_Set->Init();                // golden 建構子 :95-408（移植樹）
//    FileRW_Temperature_BootPanels();  // myTempPal 的具名替身（建構子建好之後）
//    W906_ReadATCIni();                // golden main.cpp:9797（FormShow 裡，早於 DoReadLastData）
//    fTemp_Set->ReadTempFile(true);    // 開機讀檔仍是移植樹（golden main.cpp:9993 DoReadLastData → :9291/:9339）
// ===========================================================================
#include "FileRW/Temperature.gen.inc"

#include <cstdio>

#include "FileRW/_EditPage.h"

namespace {
bool g_booted = false;
bool g_panels = false;
bool Booted() { return g_booted && g_panels; }

// golden 關頁 FormClose（:4196）不重讀；沒寫檔時把替身還原成檔案值 ＝ golden 下次開頁 FormShow 的 ReadTempFile(true)
void Reload() { TS_ReadTempFile(true); }

// ---------------------------------------------------------------------------
// 開頁時（golden FormShow 之後）的「點擊來源」元件值。頁面一次送回全部元件，伺服器看不到使用者點了什麼；
// 跟開頁值不同 ＝ 使用者在頁面上點過它 → 照 golden 的 OnClick 處理器補做它的資料效果（見 SaveFlow）。
// ---------------------------------------------------------------------------
struct OpenState {
    int  heatMode = -1;        // rgIndexHeatMode->ItemIndex
    bool calByRecipe = false;  // chkTempCalByRecipe->Checked
    bool referSensor = false;  // cbATCReferTempSensor->Checked
    bool atcActive = false;    // rbATCActiveOn->Checked
    bool atc70 = false;        // rbATC70ActiveOn->Checked
};
OpenState g_open;

void CaptureOpenState()
{
    g_open.heatMode    = EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex;
    g_open.calByRecipe = EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Checked;
    g_open.referSensor = EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked;
    g_open.atcActive   = EL<TRadioButton>("TfTemp_Set", "rbATCActiveOn")->Checked;
    g_open.atc70       = EL<TRadioButton>("TfTemp_Set", "rbATC70ActiveOn")->Checked;
}

// WS editlist.get：golden FormShow（:433）＋記下點擊來源元件的開頁值
void FormShow()
{
    TS_FormShow();
    CaptureOpenState();
}

// WS editlist.save：頁面值已套到替身（_EditPage.cpp PageSave）→ 補做使用者點擊的 golden OnClick 資料效果 → golden 存檔鈕。
void SaveFlow()
{
    const bool heatModeChanged = EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex != g_open.heatMode;
    const bool calChanged      = CosFunction.bTempCalByRecipe &&
                                 EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Checked != g_open.calByRecipe;
    const bool atcChanged      = EL<TRadioButton>("TfTemp_Set", "rbATCActiveOn")->Checked != g_open.atcActive;
    const bool atc70Changed    = EL<TRadioButton>("TfTemp_Set", "rbATC70ActiveOn")->Checked != g_open.atc70;

    // (1) golden rgIndexHeatModeClick（:4232-4236）／chkTempCalByRecipeClick（:6333-6341）不是公式，是「換一份補償檔重讀」：
    //     ReadTempFile(false)（DefineTemp\TemperatureHeadChamber*.Data ↔ Temperature*.Data ↔ <recipe>\DefineTemperature.Data）
    //     ＋DoIniDataToForm(false)（把 myTempPal 補償值與非 bUpdateAll 段的元件換成新檔／結構值）。頁面沒有這個來回，
    //     送來的補償表是「舊檔」的值 —— 照存會把舊模式的補償值寫進新模式的 DefineTemp 檔（spbSaveClick :4393-4499 依
    //     rgIndexHeatMode／chkTempCalByRecipe 選檔）。伺服器分不出使用者是先換模式再改補償、還是反過來 → 不存（沒有寫任何檔）。
    if (heatModeChanged || calChanged) {
        filerw::ELMessage(heatModeChanged
            ? "Index Heating Mode was changed on the page. BCB6 reloads the temperature-offset table for the new mode on that click (ReadTempFile(false)); save the mode change after the page reloads the table. Nothing was saved."
            : "Temperature calibration by recipe was changed on the page. BCB6 reloads the temperature-offset table from the other file on that click (ReadTempFile(false)); nothing was saved.",
            heatModeChanged ? "Index 加熱模式在頁面上改了：BCB6 點下去會重讀新模式的溫度補償表，頁面要先重讀再存；這次沒有存檔。"
                            : "Temperature calibration by recipe 在頁面上改了：BCB6 點下去會改讀另一份補償檔，頁面要先重讀再存；這次沒有存檔。");
        filerw::ELTodo("golden rgIndexHeatModeClick/chkTempCalByRecipeClick reload round-trip (ReadTempFile(false)+DoIniDataToForm(false)) has no page action yet -- save refused, nothing written");
        return;
    }
    // (2) golden rbATCActiveOnClick（:5430-5451）／rbATC70ActiveOnClick（:5407-5428）互斥（兩個 radio 在不同父容器
    //     Panel30／gbATC70，VCL 不會自動互斥，靠處理器）。兩個都被點過 → 分不出先後 → 不存。
    if (atcChanged && atc70Changed) {
        filerw::ELMessage("ATC Active Cooling and ATC 7.0 were both changed on the page; BCB6 resolves them by click order, which the server cannot see. Nothing was saved.",
                          "ATC Active Cooling 與 ATC 7.0 同時改了：BCB6 依點擊先後互斥，伺服器分不出先後；這次沒有存檔。");
        return;
    }
    if (atcChanged)   TS_rbATCActiveOnClick();
    if (atc70Changed) TS_rbATC70ActiveOnClick();
    // (3) golden cbATCReferTempSensorClick（:7073-7084）：取消勾選 → cbUseTC2Offset 隱藏且 Checked=false（存檔 :4906 寫 UseTC2Offset）
    if (EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked != g_open.referSensor) TS_cbATCReferTempSensorClick();

    // (4) golden 存檔鈕：SaveSetupFile → DefineTemp → SaveLastSetIni → BackupSetupFile → ReadTempFile(true)
    //     → DoIniDataToForm(true)（衍生值 fHotPlateExpansionCoefficient／dIndexATCInitTempOffset[]… 在這裡照 golden 重算）
    TS_spbSaveClick();
    CaptureOpenState();   // 存完 golden 已重讀並重填元件 → 新的「開頁值」
}

const filerw::PageDesc kPage = {
    "Temperature", "TfTemp_Set", "Setup.Temp_Set.html",
    nullptr, nullptr, 0,
    kTS_SaveReads, (int)(sizeof(kTS_SaveReads) / sizeof(kTS_SaveReads[0])),
    &FormShow, &SaveFlow, "SaveSetupFile", &Reload, &Booted,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfTemp_Set 建構（HT9045.cpp CreateForm）的 DFM 半邊：移植樹元件收養 → DFM Items／設計期狀態
// → 存檔流程讀的替身 → 容器替身與父子。要在 fTemp_Set->Init()（golden 建構子）之前。
void FileRW_Temperature_Boot()
{
    if (g_booted || fTemp_Set == NULL) return;
    TS_AdoptPortWidgets();
    TS_DfmItems();
    TS_DfmState();
    TS_CreateSaveProxies();
    TS_CreateContainerProxies();
    std::printf("FileRW Temperature: TfTemp_Set proxies ready (%d save reads) -- golden uTemp_Set.cpp\n",
                (int)(sizeof(kTS_SaveReads) / sizeof(kTS_SaveReads[0])));
    g_booted = true;
}

// myTempPal[i] 的資料元件登記成 "myTempPal<i>_<成員>"。要在 fTemp_Set->Init()（建 myTempPal）之後。
void FileRW_Temperature_BootPanels()
{
    if (g_panels || !TS_PortPanelsBuilt()) return;
    TS_AdoptPanels();
    std::printf("FileRW Temperature: myTempPal[%d] named proxies ready (myTempPal<i>_<member>)\n", (int)tcTotalCount);
    g_panels = true;
}
