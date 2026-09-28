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
//  AI(W906-FRW-S88) 20260926：存檔鈕之後補跑 golden 主畫面 TfMain::sbTempOffsetClick（main.cpp:28351-28399）的尾段
//    ＝MainTempOffsetTail（TestMode.Data 的 SaveTestMode、ReadTempFile、DoStructUnitConvert、SetWorkParameter…），觸發時機見該函式。
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

// AI(W906-FRW-S88) 20260926: MainTempOffsetTail（golden TfMain::sbTempOffsetClick 尾段）用到的三支。不 include 它們的標頭：
//   cinitial.h／cMyDB.h 會帶進 aHotPlateSubstrate.h 或另一份 LAST_GENERAL_SET（Temperature.gen.inc 檔頭同一理由，陷阱 #3），
//   只前置宣告（要在全域範圍：放進匿名 namespace 會變成另一支沒有本體的函式）。不帶預設引數，呼叫處明寫 golden 預設值。
bool SetWorkParameter();                                                        // cinitial.h:76（golden cinitial.h:17）
void DoStructUnitConvert();                                                     // cUnitConvert.h:118（golden cUnitConvert.h:5）
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug);    // cMyDB.h:129（Debug 預設 " "；本體 acatchtray_shims.cpp）

namespace {
bool g_booted = false;
bool g_panels = false;
bool Booted() { return g_booted && g_panels; }

// golden 關頁 FormClose（:4196）不重讀；沒寫檔時把替身還原成檔案值 ＝ golden 下次開頁 FormShow 的 ReadTempFile(true)
void Reload() { TS_ReadTempFile(true); }

// ---------------------------------------------------------------------------
//  S88　golden TfMain::sbTempOffsetClick（main.cpp:28345-28400，V912 D:\HT9045_ref）—— 主畫面「Temp. Offset」鈕：
//    :28349 NewRecordProcess("MES21109") → :28350 fTemp_Set->ShowModal() → 溫度視窗關掉之後的尾段 :28351-28399（本函式）。
//  AI(W906-FRW-S88) 20260926（Steven 團隊）：S88 盤點 golden 全樹 16 個 SaveTestMode 呼叫點，:28361／:28371 兩處（本段）
//    移植樹原本沒有 ⇒ CosFunction.bLastSetInSetUpFile 的機台在網頁溫度頁改了 Temperature Mode 存檔後，配方的
//    TestMode.Data [TestMode] Temperature Mode 仍是舊值，也不會像 golden 那樣 DoStructUnitConvert／SetWorkParameter 重讀參數
//    （golden 溫度頁的存檔鈕 spbSaveClick :4238-4575 本身不做這幾件，是靠這個尾段）。照 golden 補在這裡。
//  觸發時機（呼叫端 SaveFlow）：golden 是「溫度視窗關掉」（ShowModal 回傳，存不存檔都跑）。網頁沒有關頁事件：
//    (1) A02 權限不足 —— golden spbSaveClick :4244 Close() → modal 關窗 → FormClose（:4196）→ 本段：與 golden 相同
//        （FileRW/TestIF_File_Cleaning.cpp SaveFlow 的 "closed" 分支同一作法）。
//    (2) 正常存檔 —— golden 存完視窗還開著，要等使用者按 Exit（sbtExitClick :5175 → Close()）才跑本段；網頁的 Exit
//        目前不送伺服器，所以改在「真的寫了檔」（savedMark "SaveSetupFile"）之後跑。差別只在「開頁、沒存檔就關」那一種：
//        golden 會跑、這裡不跑 —— 沒存檔時 Temperature.iMachineTempMode 沒變，本段的寫檔與重讀結果和磁碟上已有的相同；
//        唯一例外是 LastSet.iTemperature 先被主畫面溫度鈕（golden Panel42Click → ChangeTempMode(10)，移植樹沒有）或
//        GPIB SETTEMP（Command.cpp WriteSetTempStatus_SIGURD）切過、而 iMachineTempMode 是 0／1／3 時，golden 關窗會把它
//        拉回來。觸發時機是待 Steven 決定的題目（S88 交件報告），Exit 送伺服器之後把 (2) 搬過去即可。
//  ⚠ 會寫的真實檔：<DataPath><配方>\TestMode.Data（SaveTestMode，cprod.cpp:3504；InitialOK==false 或
//    bLastSetInSetUpFile==false 時 SaveTestMode 自己 return，golden 同）。TS_ReadTempFile 以讀為主，但 golden 本體在
//    缺檔／缺鍵／版號不符時會補寫 DefineTemp\*.Data、[User OffSet]、[ATC] Chiller Temp（Temperature.gen.inc:3278-4026，
//    golden 同；開頁 FormShow 與存檔鈕尾端本來就各跑一次，這裡是同一份再跑一次）。
// ---------------------------------------------------------------------------
void MainTempOffsetTail()
{
    filerw::ELMark("MainTempOffsetTail");   // FileRW：存檔流程 trace（ack.trace）

    if(Temperature.iMachineTempMode==0 ||Temperature.iMachineTempMode==3)       //kevin 20140918 恆溫控制   // golden main.cpp:28351
    {
        if(Temperature.iMachineTempMode==3)
            LastSet.iTemperature=Tempture_AmbientHot;                           //kevin 20141004
        else
            LastSet.iTemperature=Tempture_Hot;

        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();                                                     // golden main.cpp:28361
        }
    }
    else if(Temperature.iMachineTempMode==1)                                    // golden main.cpp:28364
    {
        LastSet.iTemperature=Tempture_Ambient;

        if(CosFunction.bLastSetInSetUpFile)                                     //Steven 20111019
        {
            TestMode.iTemperatureMode=LastSet.iTemperature;
            SaveTestMode();                                                     // golden main.cpp:28371
        }
    }

    TS_ReadTempFile(true);                                                      //Steven 20190408 : 改位置到外面來,避免LastSet.iTemperature還沒換狀態   // golden main.cpp:28375 fTemp_Set->ReadTempFile(true)（本 TU 的 golden 本體；替身就是移植樹 fTemp_Set 的元件，見檔頭）
    if(LastSet.iTemperature==Tempture_Hot ||                                    //jou 2011-12-27 如果溫度值改變時,重新設定溫度   // golden main.cpp:28376
       LastSet.iTemperature==Tempture_AmbientHot)
    {
        filerw::ELTodo("golden main.cpp:28379-28381 fHeaterOK=false; bHeatOKBellowError=false; iThermoTask=1 (restart the thermo task so the new set point goes to the heaters) -- heater control, Jimmy");
#if 0 // GATE(W906-FRW-S88-THERMO) golden main.cpp:28379-28381 -- 重新啟動溫控任務（bthermo.cpp iThermoTask，把新設定值送到加熱器）＝機台溫控流程，不在 St01 讀寫檔範圍 → Jimmy。VERBATIM
        fHeaterOK=false;
        bHeatOKBellowError=false;                                               //jou 2014-06-12 修正偶發性秀低溫異常
        iThermoTask=1;
#endif // GATE(W906-FRW-S88-THERMO)
    }

    // //AI(W906-FRW-S88) 20260926: 照翻，但代碼看起來對調了 —— 同一件事在 golden ChangeSetUpFile（main.cpp:25737／:25747）與
    //   ChangeTempMode（:22565／:22578）是 Hot＝MES2151「Change Hot Mode」、Ambient＝MES2153「Change Ambient Mode」；這裡 iMachineTempMode==0（Hot）
    //   記 MES2153、==1（Ambient）記 MES2151，行尾註解「常溫控制／高溫控制」也跟條件相反。要不要改是 Jimmy／Steven 的決定，不順手修。
    if(Temperature.iMachineTempMode==0)                                         //kevin 20160908 常溫控制   // golden main.cpp:28384
        NewRecordProcess("MES2153", "Change to Hot Mode", " ");
    else if(Temperature.iMachineTempMode==1)                                    //kevin 20140918 高溫控制
        NewRecordProcess("MES2151", "Change to Ambient Mode", " ");
    else if(Temperature.iMachineTempMode==3)                                    //kevin 20140918 恆溫控制
        NewRecordProcess("MES2152", "Change to Ambient control Mode", " ");     //kevin 20190328 change
    else
        NewRecordProcess("MES2154", "Change to Ambient/Hot/Ambient control Mode", " ");                                 //kevin 20190328 change

    fMain->UpdateMainOperateMode();                                             // golden main.cpp:28393（移植樹門面計數 stub，forms/fMain.cpp；同 WebRecipeChange.cpp）
    DoStructUnitConvert();                                                      // golden main.cpp:28394
    SetWorkParameter();                                                         //Steven 20120130 : 存檔後要重新load參數   // golden main.cpp:28395
    if(CosFunction.bAmkorFunction || CUSTOMER_CODE==CC_QUALCOMM)                // golden TfAutomation::AmkorSendMessage（automation.cpp:2302）第一道條件；其他客戶 golden 什麼都不送
        filerw::ELTodo("golden main.cpp:28396 fAutomation->AmkorSendMessage(1) (ATK site-map UDP message, bAmkorFunction/CC_QUALCOMM) -- TfAutomation::AmkorSendMessage is not ported");
#if 0 // GATE(W906-FRW-S88-AMKOR) golden main.cpp:28396 -- TfAutomation::AmkorSendMessage 移植樹沒有（Automation/automation.h:331；UDP 對外通訊、客戶專屬）。VERBATIM
    fAutomation->AmkorSendMessage(1);                                           //Steven 20120330 : ATK Site Map Monitorning
#endif // GATE(W906-FRW-S88-AMKOR)

    fMain->ShowTestHeadComp(false);                                             //ChungHung 20130910 alter for SCK can close site by Index   // golden main.cpp:28398（移植樹 forms/fMain.cpp 空殼；golden ShowTestHeadComp1 的本體歸 Jimmy，見 S88 報告）
    if(ATKRecipeInfo) ATKRecipeInfo->SaveFile();                                //Steven 20170901 (wei) : For ATK要新增工作檔比對用的檔案   // golden main.cpp:28399
    else if(CUSTOMER_CODE==CC_AMKOR_Korea || CUSTOMER_CODE==CC_AMKOR_China)     // 同 FileRW/TestIF_File_TesterIF.gen.inc:1547 的取代：移植樹沒建 ATKRecipeInfo（database.cpp:149）
        filerw::ELTodo("golden main.cpp:28399 ATKRecipeInfo->SaveFile() (AMKOR Information.txt / D:\\eRMS) not run -- ATKRecipeInfo is not created in the port (database.cpp:149)");
}

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
    // (5) AI(W906-FRW-S88) 20260926: 溫度視窗關掉之後 TfMain::sbTempOffsetClick 的尾段（golden main.cpp:28351-28399，見 MainTempOffsetTail）。
    //     A02：golden :4244 Close() → FormClose（:4196）→ ShowModal 回傳 → 尾段（與 golden 相同）；
    //     有寫檔：golden 要等 Exit 才跑，網頁 Exit 不送伺服器 → 存完就跑（觸發時機待 Steven 決定，見 MainTempOffsetTail 註解）。
    if (filerw::ELMarked("closed")) {
        TS_FormClose();                                                         // golden uTemp_Set.cpp:4196
        MainTempOffsetTail();                                                   // golden main.cpp:28351
    } else if (filerw::ELMarked("SaveSetupFile")) {
        MainTempOffsetTail();                                                   // golden main.cpp:28351（觸發時機見上）
    }
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
