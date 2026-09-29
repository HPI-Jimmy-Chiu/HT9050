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
std::string W906_TempSetTs9ExtraJson();                                        // AI(W906-Q41-TS9) 20260928 (St02-E helper): TS-9 kPage extraJson (:245); body FileRW/TempSet_Ts9.cpp. Global scope like the three above. Was a blank line
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
//        拉回來。觸發時機是待 Steven 決定的題目（S88 交件報告），Exit 送伺服器之後把 (2) 搬過去即可。（⛔ 20260927 更正：已裁決——RULINGS_20260926 S107-1「存檔後就跑，不改成等 Exit」，維持現狀）
//  ⚠ 會寫的真實檔：<DataPath><配方>\TestMode.Data（SaveTestMode，cprod.cpp:3504；InitialOK==false 或〔20260927 補：還有 UpdateMainOperateMode 寫的 D:\HT9045\system\lastdata.dat，見 :122〕
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

    fMain->UpdateMainOperateMode();                                             // golden main.cpp:28393（⛔ 20260927 更正：已不是 stub——真本體 forms/fMain_OperateMode.cpp 由 wb_serve 開機裝上（tools/wb_serve.cpp:4065），會寫 D:\HT9045\system\lastdata.dat（:514 WriteLastDataFile）、切加熱器繼電器、送 ATC7；D-005 查證）
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
    int  heatMode = -1;        // rgIndexHeatMode->ItemIndex —— AI(W906-FRW-S158) 20260927：存檔時由 BeforeApply 改成「套值前伺服器端的值」（見 SaveFlow (1)）
    // ⛔ 更正 AI(W906-FRW-S158) 20260927 [W906]：原本還有 calByRecipe（chkTempCalByRecipe->Checked 的開頁值）；改看 Temperature.bTempCalByRecipe（SaveFlow (1)）
    bool referSensor = false;  // cbATCReferTempSensor->Checked
    bool atcActive = false;    // rbATCActiveOn->Checked
    bool atc70 = false;        // rbATC70ActiveOn->Checked
};
OpenState g_open;

void CaptureOpenState()
{
    g_open.heatMode    = EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex;
    g_open.referSensor = EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked;
    g_open.atcActive   = EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked;   //AI(W906-EVB3) 20260928 [W906]：EL<TRadioButton> → EL<TCheckBox>（golden uTemp_Set.h:483 TCheckBox；替身也是 TCheckBox，原本是轉到別的類別）；同一行改寫
    g_open.atc70       = EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked;   //AI(W906-EVB3) 20260928 [W906]：EL<TRadioButton> → EL<TCheckBox>（golden uTemp_Set.h:112 TCheckBox；替身也是 TCheckBox，原本是轉到別的類別）；同一行改寫
}
void BasePointReplay(const std::string& widgetsJson, std::vector<std::string>* handled); }  void TsEvArmOffsetLevel(const std::string& widgetsJson); void TsEvArmOffsetRestore(); const filerw::PageEvent* TS_EvTable(); int TS_EvCount();  namespace {   // AI(W906-FRW-S158) 20260927 [W906]：TS-7 基準點數的存檔前重播，定義在檔尾；佔用原本的空行 //AI(W906-EVB3) 20260928 [W906]：TS-L1 存檔擋／TS-2 事件表（檔尾，全域範圍）；同一行附加
// AI(W906-FRW-S158) 20260927 [W906]：editlist.save 套值前（_EditPage.h PageDesc::beforeApply）—— rgIndexHeatMode 只記、不處理；TS-7 基準點數交給檔尾 BasePointReplay（:167 同一行呼叫）。
//   TS-1 之後，伺服器端的 rgIndexHeatMode 是「目前載入的那份補償表」對應的模式：開頁 FormShow（ReadTempFile(true) 依
//   Temperature.iIndexHeatMode 讀表、DoIniDataToForm(true) :3356 把 ItemIndex 設成它），或頁面送過 form.event
//   （RunPageEvent 先設 ItemIndex 再跑 golden rgIndexHeatModeClick → ReadTempFile(false) 依 ItemIndex 讀表，:2906）。
//   頁面送來的值跟它不同 ＝ 頁面改了模式卻沒送 form.event（表沒重讀）→ SaveFlow (1) 照舊拒存。
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    (void)widgetsJson;
    (void)handled;
    g_open.heatMode = EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex; BasePointReplay(widgetsJson, handled); TsEvArmOffsetLevel(widgetsJson);   // AI(W906-FRW-S158) 20260927 [W906]：TS-7（檔尾）；同一行附加 //AI(W906-EVB3) 20260928 [W906]：TS-L1 等級 17（檔尾）；同一行附加
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
    // AI(W906-FRW-S158) 20260927 [W906]：⛔ 更正 —— 兩個「改過」的比較對象換了（TS-1 接上 WS form.event 之後）：
    //   heat：比「套值前伺服器端的值」（BeforeApply 記的；頁面送過 form.event 就是頁面點的那個模式，表已重讀）。
    //   cal ：比 Temperature.bTempCalByRecipe —— 就是 ReadTempFile 選表用的那個全域（:3027）。開頁 ReadTempFile(true) :2363 從檔案讀、
    //         DoIniDataToForm(true) :3374 把勾選設成它；form.event 的 golden chkTempCalByRecipeClick :6337 先把它設成勾選值再重讀表。
    //         不比替身：form.event 的 state 可能把勾選框改掉卻沒跑它的處理器（例：送 rgIndexHeatMode 事件時 state 帶了勾選框），
    //         那時表是照舊的 bTempCalByRecipe 讀的，存檔卻會依新的勾選選檔（:4489-4490）→ 舊表寫進另一個檔。
    const bool heatModeChanged = EL<TRadioGroup>("TfTemp_Set", "rgIndexHeatMode")->ItemIndex != g_open.heatMode;
    const bool calChanged      = CosFunction.bTempCalByRecipe &&
                                 EL<TCheckBox>("TfTemp_Set", "chkTempCalByRecipe")->Checked != Temperature.bTempCalByRecipe;
    const bool atcChanged      = EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked != g_open.atcActive;   //AI(W906-EVB3) 20260928 [W906]：EL<TRadioButton> → EL<TCheckBox>（golden uTemp_Set.h:483 TCheckBox；替身也是 TCheckBox，原本是轉到別的類別）；同一行改寫
    const bool atc70Changed    = EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked != g_open.atc70;   //AI(W906-EVB3) 20260928 [W906]：EL<TRadioButton> → EL<TCheckBox>（golden uTemp_Set.h:112 TCheckBox；替身也是 TCheckBox，原本是轉到別的類別）；同一行改寫

    // (1) golden rgIndexHeatModeClick（:4232-4236）／chkTempCalByRecipeClick（:6333-6341）不是公式，是「換一份補償檔重讀」：
    //     ReadTempFile(false)（DefineTemp\TemperatureHeadChamber*.Data ↔ Temperature*.Data ↔ <recipe>\DefineTemperature.Data）
    //     ＋DoIniDataToForm(false)（把 myTempPal 補償值與非 bUpdateAll 段的元件換成新檔／結構值）。頁面沒有這個來回，
    //     送來的補償表是「舊檔」的值 —— 照存會把舊模式的補償值寫進新模式的 DefineTemp 檔（spbSaveClick :4393-4499 依
    //     rgIndexHeatMode／chkTempCalByRecipe 選檔）。伺服器分不出使用者是先換模式再改補償、還是反過來 → 不存（沒有寫任何檔）。
    //     AI(W906-FRW-S158) 20260927 [W906]：TS-1 接上 WS form.event（本檔 g_evreg；golden 處理器 Temperature.gen.inc 檔尾）——
    //     頁面在點的當下送 form.event，伺服器照 golden 重讀表、回 changed（myTempPal<i>_* 補償值等），頁面套上之後再改、再存，
    //     存檔時伺服器端的值已經等於頁面的值 ⇒ 這一段不擋。這一段保留，只擋「頁面改了卻沒送 form.event」：
    //       * 頁面還沒標 data-ht-event（送出點在 Jimmy 的 web/page/ht9045_wire_engine.js）、舊快取頁、別的客戶端直接送 editlist.save；
    //       * 這時頁面送來的補償表仍是舊表的值，照存會把舊模式的補償寫進新模式的 DefineTemp 檔（靜默弄壞校正資料，
    //         事後看不出來）—— 代價遠大於「這次沒存、請重點一次」。存檔時也不能替頁面重播（先重讀表再套頁面值，
    //         頁面改過的補償值會被當成新表的值存進去；先套值再重讀，頁面改的補償值被蓋掉）—— 同上面「分不出先後」。
    if (heatModeChanged || calChanged) {
        filerw::ELMessage(heatModeChanged
            ? "Index Heating Mode was changed on the page without the click event (form.event). BCB6 reloads the temperature-offset table for the new mode on that click (ReadTempFile(false)); after the page reloads, click the mode again (the page has to send form.event) and then save. Nothing was saved."
            : "Temperature calibration by recipe was changed on the page without the click event (form.event). BCB6 reloads the temperature-offset table from the other file on that click (ReadTempFile(false)); after the page reloads, click it again (the page has to send form.event) and then save. Nothing was saved.",
            heatModeChanged ? "Index 加熱模式在頁面上改了、但頁面沒有送點擊事件（form.event）：BCB6 點下去會重讀新模式的溫度補償表；頁面重讀後請再點一次（頁面要送 form.event），再存檔。這次沒有存檔。"
                            : "Temperature calibration by recipe 在頁面上改了、但頁面沒有送點擊事件（form.event）：BCB6 點下去會改讀另一份補償檔；頁面重讀後請再點一次（頁面要送 form.event），再存檔。這次沒有存檔。");
        filerw::ELTodo("golden rgIndexHeatModeClick/chkTempCalByRecipeClick: the page changed the value without sending form.event (the page needs data-ht-event=\"click\" on rgIndexHeatMode / chkTempCalByRecipe) -- save refused, nothing written");
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
    CaptureOpenState(); TsEvArmOffsetRestore();   // 存完 golden 已重讀並重填元件 → 新的「開頁值」 //AI(W906-EVB3) 20260928 [W906]：TS-L1 暫鎖的 pnlArm1／2Offset 還原（檔尾）；同一行附加
}

const filerw::PageDesc kPage = {
    "Temperature", "TfTemp_Set", "Setup.Temp_Set.html",
    nullptr, nullptr, 0,
    kTS_SaveReads, (int)(sizeof(kTS_SaveReads) / sizeof(kTS_SaveReads[0])),
    &FormShow, &SaveFlow, "SaveSetupFile", &Reload, &Booted,
    &BeforeApply, &W906_TempSetTs9ExtraJson,   // AI(W906-FRW-S158) 20260927 [W906]：beforeApply（上面，只記套值前的 rgIndexHeatMode）；AI(W906-Q41-TS9) 20260928 (St02-E helper): extraJson = TS-9, fMain->edWorkTemperBase->Text for the page clamp of golden edtSetTempature2AirMachineClick (906_0625_Steven uTemp_Set.cpp:6912; body FileRW/TempSet_Ts9.cpp, declaration :40; St01 20260927 20:20 TS-9 = B). Was nullptr
};
filerw::PageRegistrar g_reg(&kPage);
// AI(W906-FRW-S158) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 盤點 TS-1）—— 頁面點
//   rgIndexHeatMode／chkTempCalByRecipe → golden rgIndexHeatModeClick（uTemp_Set.cpp:4232）／chkTempCalByRecipeClick（:6333）
//   → ReadTempFile(false)（＋DoIniDataToForm(false)）重讀另一份溫度補償表，changed 回頁面。表 kTS_Events 由 tools/gen_editlist.py
//   產生（Temperature.gen.inc 檔尾），本體 FileRW/_EditPage.cpp RunPageEvent。
//   ⚠ golden 同：點下去就把全域 Temperature.fTempOffSet[][]／bTempCalByRecipe 換掉（存檔前、沒按存檔也一樣），bthermo.cpp:282
//     ConvertTempOffset 算加熱設定值讀的就是它；沒存檔時由下次開頁 FormShow（ReadTempFile(true)）或存檔失敗的 Reload 換回檔案值。
//   ⚠ ReadTempFile(false) 讀檔時會照 golden 補寫：CheckAndReadIniData 缺鍵、:2613 [ATC] Chiller Temp（ATC 機種）、另一模式的
//     DefineTemp 變體檔（*60mm／*_ATC／*_ATC_Cold）不在或版號舊時從基底檔複製（:2923-2940、:2954-2958、:2974-2991、:3007-3017）
//     —— 讀檔的補寫，不是存檔；Q14＝B 範圍的判斷見交件（R 題）。
filerw::PageEventsRegistrar g_evreg("Temperature", TS_EvTable(), TS_EvCount());   //AI(W906-EVB3) 20260928 [W906]：原本直接註冊 kTS_Events；改成檔尾同一張表、TS-2 那 6 列包一層（同步 g_open）；同一行改寫
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

// ---------------------------------------------------------------------------
//  AI(W906-FRW-S158) 20260927 [W906]：TS-7（Q41 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md §3.9；
//    St02 FROM_STEVEN §4 20260927 17:21 (b)）—— editlist.save 套值前，頁面換了基準點數（1／2／3／5／6 Points）或
//    「System／Kit Temp. Offset Setting」卻沒送 form.event 時的重播（上面 BeforeApply :167 呼叫）。檔尾附加（St02 的 TS-9 改 kPage extraJson，不同段）。
//
//    golden V912（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uTemp_Set.cpp）：
//      rb1PointClick :421（5 顆 TRadioButton，uTemp_Set.dfm:10719 起）＝SetBasePointIMG（示意圖，純畫面）＋UpDateEdit；
//      rgBasePointClick :6384＝UpDateEdit。UpDateEdit :3708-4138 只設 myTempPal[i] 各欄與 palTemp 的 Visible／Enabled
//      （另有 palTemp->Align 排版、尾端 dOld* 取結構值給 Event Log），結果只看「當下」的 rb*Point、rgBasePoint->ItemIndex、
//      btnSort->Tag 與機台設定 —— 跟點的先後、點了幾次無關（每次都從頭設：每通道先設 true，再依點數把用不到的設 false）。
//      存檔 SaveSetupFile :4625-4629 把勾的那顆存成 [Mode] Points；各欄的值不管看不看得見照樣寫 —— golden 使用者在舊點數下
//      改的欄位，換到看不見之後按存檔也會存進去。
//    伺服器端的問題：PageSave「不可改的丟掉」照存檔當下伺服器端的看得見／可改判；頁面換了點數卻沒送 form.event，伺服器端停在
//      開頁（或上一次事件）那一組 ⇒ 例：開頁 1 Points、頁面切 3 Points、填 Base／High Base 後存檔 → 伺服器認為那兩欄看不見、
//      丟掉頁面值，[Mode] Points 卻存成 3 Points（新點數的補償值其實是舊檔值，事後看不出來）。
//    所以重播、不拒存（跟 TS-1／R97 不同：TS-1 的處理器會重讀另一份補償檔、改資料，伺服器分不出先後；這裡只改看得見／可改、
//      只看最後狀態 ⇒ 存檔前重播一次就等於 golden 最後的樣子）：
//      (1) 先記每個頁面值在「重播之前」可不可改（開頁／上一次事件那一組）。
//      (2) 頁面勾的那顆跟伺服器端不同且點得到 ⇒ 照 VCL 設 Checked＋golden rb1PointClick（處理器開頭補 VCL 的 TurnSiblingsOff，
//          見 tools/editlist/Temperature.py）；rgBasePoint 不同、點得到、在範圍內 ⇒ 設 ItemIndex＋golden rgBasePointClick。
//      (3) 重播前可改、重播後不可改的欄位 ⇒ 這裡先照頁面值收下（使用者在舊點數下改的，golden 看不見照樣存）；
//          重播後可改的交給 PageSave（照重播後判）；兩邊都不可改的照舊丟掉（ack.ignored）。
//      只看得到開頁與最後兩組：換過第三種點數、只在那一組改的欄位（例：開頁 3 Points → 切 6 Points 改 S.High Base → 切回 3 Points 存）
//      仍會被丟 —— 那一欄在存下去的點數用不到；要完全照 golden，頁面每點一下就送 form.event（帶 state）。
//    btnSort（依 Site 排）伺服器端點不到（tools/editlist/Temperature.py 'events' 註解），Tag 停在 0 ⇒ 伺服器不藏通道 ⇒ 那些通道照收頁面值（＝golden）。
//    ack.events 會列出這裡收下的名稱（五顆鈕、rgBasePoint、(3) 的欄位）。
// ---------------------------------------------------------------------------
#include <cmath>
#include "Public/cJSON.h"

namespace {
void BasePointReplay(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {   // PageSave 已驗過是物件；保險
        if (root) cJSON_Delete(root);
        return;
    }
    const char* const F = "TfTemp_Set";
    static const char* const kRb[5] = {"rb1Point", "rb2Point", "rb3Point", "rb5Point", "rb6Point"};   // golden uTemp_Set.dfm gbBasePoint（Tag 1／2／4／8／16）
    // 頁面點的那一顆：頁面勾、伺服器端沒勾、點得到（VCL 點一下＝SetChecked(True)；取消勾選只會因為「點了別顆」）
    int clicked = -1;
    for (int k = 0; k < 5 && clicked < 0; ++k) {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, kRb[k]);
        const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
        if (v && cJSON_IsTrue(v) && !EL<TRadioButton>(F, kRb[k])->Checked && filerw::ELEditable(F, kRb[k])) clicked = k;
    }
    int baseIdx = -1;
    {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, "rgBasePoint");
        const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "itemIndex") : nullptr;
        TRadioGroup* g = EL<TRadioGroup>(F, "rgBasePoint");
        if (v && cJSON_IsNumber(v) && v->valuedouble == std::floor(v->valuedouble) && v->valuedouble >= 0 &&
            v->valuedouble < g->Items->Count && (int)v->valuedouble != g->ItemIndex && filerw::ELEditable(F, "rgBasePoint"))
            baseIdx = (int)v->valuedouble;
    }
    if (clicked < 0 && baseIdx < 0) {   // 頁面沒換，或送過 form.event、伺服器端已同步 → 什麼都不做（PageSave 照舊）
        cJSON_Delete(root);
        return;
    }
    // (1)
    std::vector<std::string> before;
    for (const cJSON* it = root->child; it; it = it->next)
        if (it->string && filerw::ELEditable(F, it->string)) before.push_back(it->string);
    // (2)
    if (clicked >= 0) {
        TRadioButton* r = EL<TRadioButton>(F, kRb[clicked]);
        r->Checked = true;                                                      // VCL：點一下 ⇒ SetChecked(True)
        TS_rb1PointClick(r);                                                    // golden uTemp_Set.cpp:421（TurnSiblingsOff＋UpDateEdit）
        for (int k = 0; k < 5; ++k) handled->push_back(kRb[k]);                 // 五顆的最後狀態由這一下決定（頁面值不再套）
    }
    if (baseIdx >= 0) {
        EL<TRadioGroup>(F, "rgBasePoint")->ItemIndex = baseIdx;                 // VCL：點選項 ⇒ ItemIndex 變 → OnClick
        TS_rgBasePointClick();                                                  // golden uTemp_Set.cpp:6384
        handled->push_back("rgBasePoint");
    }
    // (3)
    cJSON* keep = cJSON_CreateObject();
    int nKeep = 0;
    for (std::size_t i = 0; i < before.size(); ++i) {
        const std::string& n = before[i];
        if (filerw::ELEditable(F, n.c_str())) continue;                         // 重播後仍可改 → PageSave
        bool mine = false;                                                      // 上面已處理的（五顆鈕、rgBasePoint）不重複
        for (std::size_t j = 0; j < handled->size() && !mine; ++j) mine = ((*handled)[j] == n);
        if (mine) continue;
        cJSON_AddItemToObject(keep, n.c_str(), cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(root, n.c_str()), 1));
        ++nKeep;
    }
    if (nKeep > 0) {
        char* s = cJSON_PrintUnformatted(keep);
        std::vector<std::string> applied, unknown, notes;
        std::string err;
        if (filerw::ELApplyProxies(F, s ? s : "{}", &applied, &unknown, &err, &notes)) {   // 先全部驗、再一次套；失敗時一個都沒動
            for (std::size_t i = 0; i < applied.size(); ++i) handled->push_back(applied[i]);
        } else {                                                                // 型別不對：不收，交給 PageSave（看不見 → ack.ignored）
            filerw::ELTodo((std::string("golden uTemp_Set.cpp:421/:6384 base-point replay: values edited before the base-point change were not kept (") +
                            err + ")").c_str());
        }
        if (s) cJSON_free(s);
    }
    cJSON_Delete(keep);
    cJSON_Delete(root);
}
}  // namespace

// ---------------------------------------------------------------------------
//  //AI(W906-EVB3) 20260928 [W906]：事件移植批次 B3（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md）TS-2／TS-L1。
//    Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」。檔尾附加
//    （St02 的 TS-9 只改 :40 那一行空行與 :245 kPage extraJson 那一格，不同段；本段另外只動 :157／:167／:237／:257 四行（同一行附加或改寫），檔頭到 :258 的行數不變）。
//
//  TS-2　ATC Active／ATC 7.0 二選一、取消「參考溫度感測器」藏 TC2 offset：golden（V912 uTemp_Set.cpp）
//    rbATCActiveOnClick :5430、rbATC70ActiveOnClick :5407、cbATCReferTempSensorClick :7073 —— 早就翻好（產生檔），原本只在存檔時由
//    SaveFlow (2)(3) 照「開頁值」重播（兩個 ATC 都改過就拒存）。現在頁面點的當下送 WS form.event（事件表 kTS_Events，
//    tools/editlist/Temperature.py 'events'；golden DFM 另外三格 cbEnableATCConsFailOffset／cbEnableATCQAModeOffset／cbATCTestTimeOffset
//    也綁 rbATC70ActiveOnClick，照留）。這裡把那幾列的處理器包一層（TS_EvAtc）：跑完 golden 處理器，把 SaveFlow 比對用的
//    g_open.atcActive／atc70／referSensor 同步成伺服器現值 —— 點的先後伺服器已經照 golden 跑過，存檔時 (2)(3) 不再重播、
//    也不會因為「兩個都改過」拒存；頁面沒送事件（舊頁面）照舊由 (2)(3) 處理。
//    ⚠ form.event 的 state 若同時帶了別的 ATC 勾選（沒跑它的處理器），同步之後伺服器分不出來 —— 頁面送這幾列時不帶 state
//      （D:\HT9045\web\page 端的送出點寫法見 B3 lane 2 交件；Setup.Temp_Set.html 由 B2／St02 的行載入）。
//  TS-L1　golden edArm1OffsetMouseDown（:5503，edArm1Offset／edArm2Offset 的 OnMouseDown，dfm:10276／:10314）：
//    AccessLevel<LevelSet.AccessLevel[17] 就 return、不開小鍵盤（觸控機台上＝改不了）。滑鼠按下不是 form.event 的 change／click，
//    改在存檔前（BeforeApply :167 同一行呼叫）照同一個條件擋：等級不夠、頁面值跟伺服器不同、而且伺服器端本來可改 ⇒ 那一格的
//    容器 pnlArm1Offset／pnlArm2Offset 暫時 Enabled=false，PageSave「不可改的丟掉」把頁面值丟進 ack.ignored（存的是原值），
//    再加一句 ELMessage；golden 存檔鈕跑完（SaveFlow 尾 :237）還原。
//    查證（20260928）：一般情況 golden FormShow :1107／:1109 已經依等級 17 設 pnlArm1／2Offset->Enabled（fSecurity->Insufficient(17,false)
//    ＝等級夠才 true），伺服器端早就丟值；只有 bEnablePEModel（FormShow :1140-1143 不看等級一律 Enabled=true）時要靠這一段。
//    ⚠ 另一個既有缺口（不在本段，X-5／B10）：golden FormShow :1106／:1108 把 pnlArm1／2Offset->Visible=false，只有切分頁的
//    pgcTempOffsetChange（:5258-5259）會打開；那支沒翻 ⇒ 伺服器端這兩格永遠看不見、網頁改 Arm offset 存檔一律 ack.ignored。
// ---------------------------------------------------------------------------
#include <stdexcept>
#include <string>

namespace {
// ---- TS-2 ------------------------------------------------------------------------------------------------------------
const char* const kTsEvAtc[] = {"rbATCActiveOn", "rbATC70ActiveOn", "cbEnableATCConsFailOffset", "cbEnableATCQAModeOffset",
                                "cbATCTestTimeOffset", "cbATCReferTempSensor"};
bool TsEvIsAtc(const char* control)
{
    for (std::size_t i = 0; i < sizeof(kTsEvAtc) / sizeof(kTsEvAtc[0]); ++i)
        if (std::string(kTsEvAtc[i]) == control) return true;
    return false;
}
void TS_EvAtc(TControl* Sender)
{
    for (std::size_t i = 0; i < sizeof(kTS_Events) / sizeof(kTS_Events[0]); ++i) {
        if (!TsEvIsAtc(kTS_Events[i].control) || filerw::ELFind("TfTemp_Set", kTS_Events[i].control) != Sender) continue;
        kTS_Events[i].handler(Sender);                                          // golden :5430／:5407／:7073
        g_open.referSensor = EL<TCheckBox>("TfTemp_Set", "cbATCReferTempSensor")->Checked;   // SaveFlow (3) 的比對基準
        g_open.atcActive   = EL<TCheckBox>("TfTemp_Set", "rbATCActiveOn")->Checked;          // SaveFlow (2)。golden uTemp_Set.h:483／:112 是 TCheckBox
        g_open.atc70       = EL<TCheckBox>("TfTemp_Set", "rbATC70ActiveOn")->Checked;        //（CaptureOpenState／SaveFlow 同一組替身，20260928 起也改成 EL<TCheckBox>）
        return;
    }
    throw std::runtime_error("internal: form.event sender is not a TS-2 control of kTS_Events");
}
filerw::PageEvent g_tsEvents[sizeof(kTS_Events) / sizeof(kTS_Events[0])];
int g_tsNEvents = 0;
}  // namespace

// :257 的 g_evreg 用（函式內建表：:257 在檔尾之前做動態初始化，kTS_Events 是常數初始化、g_tsEvents 是零初始化，順序安全）
const filerw::PageEvent* TS_EvTable()
{
    if (g_tsNEvents == 0) {
        for (std::size_t i = 0; i < sizeof(kTS_Events) / sizeof(kTS_Events[0]); ++i) {
            g_tsEvents[i] = kTS_Events[i];
            if (TsEvIsAtc(kTS_Events[i].control)) g_tsEvents[i].handler = &TS_EvAtc;
        }
        g_tsNEvents = (int)(sizeof(kTS_Events) / sizeof(kTS_Events[0]));
    }
    return g_tsEvents;
}
int TS_EvCount() { TS_EvTable(); return g_tsNEvents; }

// ---- TS-L1 -----------------------------------------------------------------------------------------------------------
namespace {
bool g_armOffLocked[2] = {false, false};
}  // namespace
void TsEvArmOffsetRestore()
{
    static const char* const kPnl[2] = {"pnlArm1Offset", "pnlArm2Offset"};
    for (int i = 0; i < 2; ++i)
        if (g_armOffLocked[i]) { EL<TPanel>("TfTemp_Set", kPnl[i])->Enabled = true; g_armOffLocked[i] = false; }   // 鎖之前是可改的（見下）
}
void TsEvArmOffsetLevel(const std::string& widgetsJson)
{
    TsEvArmOffsetRestore();                                                     // 上一次存檔回 400（SaveFlow 沒跑）時留下的暫鎖先還原
    if (AccessLevel >= LevelSet.AccessLevel[17]) return;                       // golden uTemp_Set.cpp:5506（等級夠 → 開小鍵盤，照收）
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return;
    }
    static const char* const kEd[2]  = {"edArm1Offset", "edArm2Offset"};     // golden uTemp_Set.dfm:10314／:10276（OnMouseDown＝edArm1OffsetMouseDown）
    static const char* const kPnl[2] = {"pnlArm1Offset", "pnlArm2Offset"};   // 各自的父容器（dfm 父子，Temperature.gen.inc kTS_ParentOf）
    for (int i = 0; i < 2; ++i) {
        const cJSON* o = cJSON_GetObjectItemCaseSensitive(root, kEd[i]);
        const cJSON* t = (o && cJSON_IsObject(o)) ? cJSON_GetObjectItemCaseSensitive(o, "text") : nullptr;
        if (!t || !cJSON_IsString(t) || !t->valuestring) continue;
        if (AnsiString(t->valuestring) == EL<TEdit>("TfTemp_Set", kEd[i])->Text) continue;   // 沒改
        if (!filerw::ELEditable("TfTemp_Set", kEd[i])) continue;                // 本來就會被丟（FormShow :1107／:1109 等）
        EL<TPanel>("TfTemp_Set", kPnl[i])->Enabled = false;                     // → PageSave「不可改的丟掉」（ack.ignored）
        g_armOffLocked[i] = true;
        filerw::ELMessage(AnsiString(kEd[i]) + " needs access level item 17 (BCB6 edArm1OffsetMouseDown does not open the keypad below it); the page value was not saved.",
                          AnsiString(kEd[i]) + " 要權限等級第 17 項（BCB6 等級不夠點了不開小鍵盤）；這一格的頁面值沒有存，保留原值。");
    }
    cJSON_Delete(root);
}
