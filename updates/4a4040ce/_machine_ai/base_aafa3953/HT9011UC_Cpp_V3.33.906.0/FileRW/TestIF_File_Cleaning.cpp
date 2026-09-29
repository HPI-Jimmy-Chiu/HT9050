// ===========================================================================
//  FileRW/TestIF_File_Cleaning.cpp -- golden TfCleaning（AutoClean\uCleaning.cpp，912）的 C 路入口（C 形狀：具名替身）。
//  頁面：web/page/Setup.Cleaning.html（頁面補件 web/page/ht9045_cleaning_c.js）。
//
//  Steven 團隊 20260925.  設定：tools/editlist/TestIF_File_Cleaning.py（每條 replace／blocks 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 TestIF_File_Cleaning.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    建構子（:44）＝ LoadAutoCleanData（:51，「Steven 20211129 : Add for Auto Clean初始化的資料是錯的」）；
//    FormShow（:1465）＝開頁（權限／顯示＋SetDeviceMaxMin＋LoadAutoCleanData :1584＋SearchCleanNum）；
//    sbCleanSaveClick（:1778）＝存檔鈕；sbCleanExitClick（:1886）＝Exit。沒有 HTEditList：存檔讀的替身全部是 mustSend。
//
//  ---- 讀寫哪些檔（golden LoadAutoCleanData／SaveAutoCleanData／ReadWriteAutoCleanCount）----------------------------
//    <DataPath><recipe>\HandlerCondition.Data  [Configuration] iAutoClean_*／dBufferKit*／Hotplatl*／iCleanIndexOtherArm…、
//                                               [AutoClean] InArmVacuum／InArmAirOn／IndexVacuum／IndexAirOn、
//                                               [Configuration] iAutoCleanPad_CountTime_<Y>_<X>／iIndexArmAutoCleanCnt（計數）
//    D:\HT9045\IniData\DefineAutoClean\AutoClean.Data      CC_TSMC_TAINAN（整份）／CosFunction.bUseDefineAutoCleanOffset（高度與 offset）
//    D:\HT9045\IniData\DefineAutoClean\AutoCleanCount.Data IniConfig.bE43_1_AutoCleanCountSaveFolder（計數）
//    <recipe>\HotPlate.Data／Tray.Data                     IniConfig.bE43AutoCleanUseHotplate（只讀）
//    ⚠ LoadAutoCleanData 本身就會寫檔（golden 原樣）：:123 iAutoClean_Mode==0、:238 ShiftHeight<=0、:577 InitialOK 且片數改變、
//      :605 Fix3 有 Bin、:612 IniConfig.bEnableAutoCleanFunction==false（每次都寫 iAutoClean_Function=0）、以及
//      ReadWriteAutoCleanCount(true) 尾端無條件 WriteIniDataNoLog iIndexArmAutoCleanCnt（AutoClean.cpp:1320）。
//
//  ---- 網頁按鈕 → golden 處理器（SaveFlow）--------------------------------------------------------------------------
//    sbCleanSave（或沒帶按鈕）  golden sbCleanSaveClick（:1778）。A02 擋下時 golden Close() → modal 關窗 → FormClose（:2106）
//                               → ShowModal 回傳後 TfMain::sbAutoCleanClick 的 DoStructUnitConvert()（main.cpp:29673）
//    sbCleanExit                golden sbCleanExitClick（:1886 Close()）→ FormClose（SetWorkParameter）→ DoStructUnitConvert
//    頁面在 editlist.save 的 widgets 多帶 W906_clButton {"text":"sbCleanSave"|"sbCleanExit"}（ht9045_cleaning_c.js 注入），
//    由 BeforeApply 讀掉、不當元件套值。
//
//  ---- 存檔前重播的 golden 事件（PageDesc::beforeApply → BeforeApply）-------------------------------------------------
//    golden 的輸入途徑是「點元件 → 小鍵盤（fQwertyKey->ShowQwertyKey）→ 寫回 Text（→ TEdit.OnChange）」、「點 radio → OnClick」。
//    頁面送的是最後狀態；伺服器端把「使用者改過的」（頁面值 ≠ 開頁 FormShow 當下的值）依 golden 事件重播：
//      (1) rgCleanKitType 改了 → CL_RadioIndex（VCL SetItemIndex → OnClick=rgCleanKitTypeClick：換 pgCleanType 頁＋SetDeviceMaxMin）
//      (2) 小鍵盤元件（kCL_Kb，DFM OnClick／OnMouseDown，DFM 順序）改了、且點得到（自己＋祖先 Enabled／Visible；ReadOnly 不擋
//          OnClick —— edDevicePices／edAlarmCount／edCleaningCount 在 golden DFM 是 ReadOnly、只能用小鍵盤改）→ 跑 golden 處理器：
//          CL_QwertyKey 用頁面值（＝使用者在鍵盤上按確定的內容）做 golden 夾限（CheckRange）再寫回 → OnChange（XCT1Change／
//          YCT1Change／XPitch2Change）；edPinSingleGf／edPinSingleN／edPinsCount 的處理器接著重算力量（ShowCleanContactForce）。
//      (3) 使用者沒改、但 (1)(2) 的 golden 事件改了伺服器端的值（例：改 Gf → ShowTranGfToN 算出新的 N；改 XCT1 → XCT1Change
//          夾 XCT2）→ 保留伺服器端的值（不讓頁面上的舊值蓋回去）。
//    其餘元件照通用規則（頁面最後狀態、不觸發事件；不可改的丟掉進 ack.ignored）。ack.events 列出 (1)(2)(3) 處理的元件。
//
//  ---- 未移植（頁面停用並註明，見 ht9045_cleaning_c.js）--------------------------------------------------------------
//    udDeviceCT（TUpDown 上下鍵；頁面沒有這個元件 → 片數只能用小鍵盤改，golden 也可以）、btInclude（:2269，要伺服器當下的
//    TrayForm.Loader）、btnResetCleanCount（:2116，清潔計數歸零＋SetAutoCleanICCount＋SECS AutoCleanClearCount）、
//    btnResetInterval（:2842，ChangeACSmartInterval 直寫配方）、btnStartAutoClean（:2901，啟動 Auto Clean 動作）、
//    sbTrayAssign（:2311，主畫面）、cbbSelectTray（:2322，Tray CSV；CC_ASE_KaohSiung/OSE/K3 才顯示）、
//    rgAutoCleanOnOffClick／chkAutoCleanMode6MouseUp（KYEC 條碼，客戶專屬）、RPDefault（sbCleanSaveClick :1852-1877）、
//    tmyAutoClean 的 Clean Pad 配置預覽（DrawAutoClean :1932-1949，純畫面）。
//    ⛔ 更正 AI(W906-FRW-S157) 20260927 [W906]：cbbSelectTray 的 cbbSelectTrayChange（:2322）已轉，走 WS form.event（kCL_Events，
//    本檔 g_evreg）；golden DFM Enabled=False 沒人打開 ⇒ 目前 form.event 照 golden 回 bad-payload（見 g_evreg 的註解）。   ⛔ 更正 //AI(W906-EVB3) 20260928 [W906]：btInclude／btnResetCleanCount／btnResetInterval／sbTrayAssign 已轉、存檔重播的 rgCleanKitType 與小鍵盤元件也能當場送 form.event（檔尾「批次 B3」段）；頁面補件 web/page/ht9045_cleaning_ev.js
//
//  ---- 開機（整合者接到 wb_serve；位置見報告）------------------------------------------------------------------------
//    FileRW_Cleaning_Boot()          golden HT9045.cpp:226 CreateForm(TfCleaning) → 建構子（:44，含 LoadAutoCleanData）。
//                                     要在第一次 fSetup->ReadFile() 之前（golden CreateForm 早於 TfMain::FormShow 的 DoReadLastData）。
//    FileRW_Cleaning_MainFormShow()  golden TfMain::FormShow main.cpp:11513-11526（LoadAutoCleanData → SearchCleanNum →
//                                     MOT[MMAutoCleanKit].fHasTray → SetAutoCleanStringGrid → ReadWriteAutoCleanCount(false) →
//                                     SetAutoCleanICCount(false)）；在 InitialOK=true（golden :10898）之後。
//    FileRW_Cleaning_LoadAutoCleanData()／FileRW_Cleaning_SaveAutoCleanData()：golden TfSetup::ReadFile（cSetUp.cpp:2531-2535，
//                                     IniConfig.bEnableAutoCleanFunction 時 Load＋Save）與 SaveSetupFile（:4043）的 fCleaning-> 呼叫點。
//                                     TfSetup::ReadFile 在 golden 換配方（TfMain::ChangeSetUpFile main.cpp:25653 → DoReadLastData）
//                                     也會跑 —— 開機、換配方都讀一次（Steven 20260925 指示）。
// ===========================================================================
#include "Public/cJSON.h"             // 放在產生檔之前：產生檔的 #define（SearchCleanNum、iDeviceCount…）不能碰到這兩個 header
#include "WebBridge/JsonWriter.h"

#include "FileRW/TestIF_File_Cleaning.gen.inc"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "FileRW/_EditPage.h"

void FileRW_Cleaning_LoadAutoCleanData();   // 本檔下方（cSetUp.cpp hook 目標）
void FileRW_Cleaning_SaveAutoCleanData();

namespace {
const char* const kForm = "TfCleaning";
bool g_booted = false;
bool Booted() { return g_booted; }
std::string g_snap;       // 開頁（golden FormShow）當下的替身值：filerw::ProxyStateJson
std::string g_button;     // 這次存檔是哪一顆 golden 按鈕（W906_clButton）

// 存檔流程讀的替身＝產生器掃出的 kCL_SaveReads，去掉 udDeviceCT：golden 存檔流程只讀它的 ->Min／->Max
// （SetDeviceMaxMin :1413-1447 在伺服器端算的範圍，:910／:1451-1458／:1807），從不讀 ->Position（頁面值）；
// 頁面也沒有 TUpDown 元件。同 TestIF_File_SetUp.cpp BuildReads 排除容器的理由（伺服器端狀態，不是頁面值）。
std::vector<const char*> g_reads;
void BuildReads()
{
    for (const char* n : kCL_SaveReads)
        if (std::strcmp(n, "udDeviceCT") != 0) g_reads.push_back(n);
}

// PageDesc::formShow：golden FormShow，之後記下開頁值（BeforeApply 判斷「使用者改了哪些」）
void FormShow()
{
    CL_FormShow();
    g_snap = filerw::ProxyStateJson(kForm);
}

// 兩個替身值（{text|checked|itemIndex|position}）相同？只比 a 帶的欄位；b 缺欄位＝不同
bool SameValue(const cJSON* a, const cJSON* b)
{
    if (!a || !b) return false;
    static const char* const kKeys[] = {"text", "checked", "itemIndex", "position"};
    bool any = false;
    for (const char* k : kKeys) {
        const cJSON* x = cJSON_GetObjectItemCaseSensitive(a, k);
        if (!x) continue;
        any = true;
        const cJSON* y = cJSON_GetObjectItemCaseSensitive(b, k);
        if (!y) return false;
        if (cJSON_IsString(x)) { if (!cJSON_IsString(y) || std::strcmp(x->valuestring, y->valuestring) != 0) return false; }
        else if (cJSON_IsBool(x)) { if (!cJSON_IsBool(y) || cJSON_IsTrue(x) != cJSON_IsTrue(y)) return false; }
        else if (cJSON_IsNumber(x)) { if (!cJSON_IsNumber(y) || x->valuedouble != y->valuedouble) return false; }
    }
    return any;
}

// golden 使用者點得到這個元件嗎：自己 Enabled＋Visible，且祖先鏈可改（ReadOnly 不擋 OnClick —— VCL TEdit ReadOnly 只擋打字）
bool Clickable(const char* name, const char* parent)
{
    TControl* c = filerw::ELFind(kForm, name);
    if (!c || !c->Enabled || !c->Visible) return false;
    return !parent || !*parent || filerw::ELEditable(kForm, parent);
}

bool Has(const std::vector<std::string>& v, const char* s)
{
    for (const std::string& x : v) if (x == s) return true;
    return false;
}

// PageDesc::beforeApply（見檔頭「存檔前重播的 golden 事件」）
void BeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    g_button.clear();
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    if (!root) return;
    cJSON* snap = cJSON_Parse(g_snap.c_str());
    // (0) 按鈕
    const cJSON* btn = cJSON_GetObjectItemCaseSensitive(root, "W906_clButton");
    if (btn) {
        const cJSON* t = cJSON_IsObject(btn) ? cJSON_GetObjectItemCaseSensitive(btn, "text") : nullptr;
        if (t && cJSON_IsString(t)) g_button = t->valuestring;
        handled->push_back("W906_clButton");
    }
    // (1) rgCleanKitType：VCL 設 ItemIndex → OnClick=rgCleanKitTypeClick（:2214）
    {
        const cJSON* it = cJSON_GetObjectItemCaseSensitive(root, "rgCleanKitType");
        const cJSON* idx = (it && cJSON_IsObject(it)) ? cJSON_GetObjectItemCaseSensitive(it, "itemIndex") : nullptr;
        if (idx && cJSON_IsNumber(idx) && !SameValue(it, cJSON_GetObjectItemCaseSensitive(snap, "rgCleanKitType")) &&
            filerw::ELEditable(kForm, "rgCleanKitType")) {
            CL_RadioIndex(EL<TRadioGroup>(kForm, "rgCleanKitType"), idx->valueint);
            handled->push_back("rgCleanKitType");
            filerw::ELMark("replay:rgCleanKitType");
        }
    }
    // (2) 小鍵盤元件（DFM 順序）
    for (const CL_KbEntry& e : kCL_Kb) {
        const cJSON* it = cJSON_GetObjectItemCaseSensitive(root, e.name);
        const cJSON* t = (it && cJSON_IsObject(it)) ? cJSON_GetObjectItemCaseSensitive(it, "text") : nullptr;
        if (!t || !cJSON_IsString(t)) continue;
        if (SameValue(it, cJSON_GetObjectItemCaseSensitive(snap, e.name))) continue;   // 沒改 → (3)
        if (!Clickable(e.name, e.parent)) continue;                                    // 點不到 → 通用規則丟掉（ack.ignored）
        const AnsiString v(t->valuestring);
        CL_KbPending = &v;
        e.fn(filerw::ELFind(kForm, e.name));                                           // golden 處理器（Sender＝這個元件）
        CL_KbPending = nullptr;
        handled->push_back(e.name);
        filerw::ELMark((std::string("replay:") + e.name).c_str());
    }
    // (3) 使用者沒改、但事件改了伺服器端 → 保留伺服器端
    cJSON* now = cJSON_Parse(filerw::ProxyStateJson(kForm).c_str());
    for (const cJSON* it = root->child; it; it = it->next) {
        if (!it->string || Has(*handled, it->string)) continue;
        const cJSON* s = cJSON_GetObjectItemCaseSensitive(snap, it->string);
        const cJSON* n = cJSON_GetObjectItemCaseSensitive(now, it->string);
        if (s && n && SameValue(it, s) && !SameValue(it, n)) handled->push_back(it->string);
    }
    cJSON_Delete(now);
    cJSON_Delete(snap);
    cJSON_Delete(root);
}

// PageDesc::saveFlow：頁面按的那一顆 golden 按鈕（見檔頭）
void SaveFlow()
{
    const std::string b = g_button;
    if (b.empty() || b == "sbCleanSave") {
        CL_sbCleanSaveClick();                                                  // golden :1778
        if (filerw::ELMarked("closed")) {                                       // A02：golden Close() → modal 關窗
            CL_FormClose();                                                     // golden :2106
            DoStructUnitConvert();                                              // golden main.cpp:29673（ShowModal 回傳後）
        }
    } else if (b == "sbCleanExit") {
        CL_sbCleanExitClick();                                                  // golden :1886（Close()）
        CL_FormClose();                                                         // golden :2106
        DoStructUnitConvert();                                                  // golden main.cpp:29673
    } else {
        filerw::ELMessage(AnsiString("Unknown Cleaning button: ") + AnsiString(b.c_str()) + "; nothing was saved.",
                          AnsiString("不認得的按鈕：") + AnsiString(b.c_str()) + "，這次沒有存檔。");
    }
}

// PageDesc::reload（沒寫檔時）：不動。golden 存檔鈕在寫檔前 return（Tray 格數不足、Smart AC 未開 Auto Clean、A02）時表單還開著、
// 元件保留使用者輸入；下次開頁 FormShow 的 LoadAutoCleanData（:1584）會把元件全部重設成檔案值（網頁存完一定重讀＝重開頁）。
// 不在這裡跑 LoadAutoCleanData：它本身會寫檔（見檔頭），golden 在這個時間點沒有跑它。
void Reload() {}

// PageDesc::extraJson：頁面要、但不是元件值的 golden 狀態
//   activePage  golden pgCleanType（rgCleanKitTypeClick :2252-2259、ChangeEditToHPMode :2385 設）／pcCleanYield 的 ActivePageIndex
//   udDeviceCT  golden SetDeviceMaxMin（:1403）算的 Min／Max／Increment（edDevicePices 小鍵盤的範圍，:2649）
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("activePage").BeginObject();
    w.Key("pgCleanType").Number((wb_int64)EL<TPageControl>(kForm, "pgCleanType")->ActivePageIndex);
    w.Key("pcCleanYield").Number((wb_int64)EL<TPageControl>(kForm, "pcCleanYield")->ActivePageIndex);
    w.EndObject();
    filerw::ELTrackBar* ud = EL<filerw::ELTrackBar>(kForm, "udDeviceCT");
    w.Key("udDeviceCT").BeginObject();
    w.Key("min").Number((wb_int64)(int)ud->Min);
    w.Key("max").Number((wb_int64)(int)ud->Max);
    w.Key("increment").Number((wb_int64)ud->Increment);
    w.EndObject();
    w.EndObject();
    return w.Str();
}

filerw::PageDesc g_page = {
    "TestIF_File_Cleaning", "TfCleaning", "Setup.Cleaning.html",
    nullptr, nullptr, 0,
    kCL_SaveReads, (int)(sizeof(kCL_SaveReads) / sizeof(kCL_SaveReads[0])),
    &FormShow, &SaveFlow, "SaveAutoCleanData", &Reload, &Booted,
    &BeforeApply, &ExtraJson,
};
filerw::PageRegistrar g_reg(&g_page);
// AI(W906-FRW-S157) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157）—— cbbSelectTray 選一筆
//   → golden cbbSelectTrayChange（uCleaning.cpp:2322）。表 kCL_Events 由 tools/gen_editlist.py 產生（.gen.inc 檔尾）。
//   ⚠ golden cbbSelectTray 在 DFM 是 Enabled=False（uCleaning.dfm:906），V912 沒有任何地方打開它 ⇒ golden 使用者選不到；
//   RunPageEvent 的 ELOperable 照這個狀態回 bad-payload（照 golden 擋；要不要放行是 Steven 的 R 題）。
const filerw::PageEvent* EvB3Table(); filerw::PageEventsRegistrar g_evreg("TestIF_File_Cleaning", EvB3Table(), (int)(sizeof(kCL_Events) / sizeof(kCL_Events[0])));   //AI(W906-EVB3) 20260928 [W906]: kCL_Events → EvB3Table()（檔尾：同一張表、處理器換成跳板）；同一行改寫

// 開機函式裡 golden 的訊息框（ShowMyMessage → ELMessage）／待辦印到 wb_serve 的主控台（沒有頁面收）
void PrintSession(const char* where)
{
    std::printf("FileRW TestIF_File_Cleaning %s: session %s\n", where, filerw::SessionJson().c_str());
}
}  // namespace

// ---------------------------------------------------------------------------
//  golden AutoClean.cpp:1324 SearchCleanNum()（V912）—— golden 是 AutoClean.cpp 的自由函式；移植樹 AutoClean/AutoClean.cpp:1662
//  有一份，但是 static（本 TU 叫不到）、而且是 906 版：少了 V912 的 fShowBinSelect->ed_AutoCleanCount 顯示與
//  「Steven 20260427 : ATK P260427-ATK-H9-01」達到 AlarmCount 的 WAR1922 警報（golden :1360-1374）。
//  golden TfCleaning 的呼叫點（LoadAutoCleanData :809、FormShow :1635、sbCleanSaveClick :1838）與開機（main.cpp:11514）用這一份；
//  移植樹 AutoClean.cpp 自己的呼叫點（CheckCleaningCount :1707、:9070）仍用它那份 906 版 —— 兩份並存，整合者決定是否合一。
// ---------------------------------------------------------------------------
#undef SearchCleanNum
int FileRW_Cleaning_SearchCleanNum()
{
    AnsiString Str;
    int iMin=9999;                                                              //Steven 20171211 (Wei) : 修正計算方式
    int iNow=0;
    if(TestIF_File.iAutoClean_Function==0)                                      //Ifor 20191024 : add 避免Auto Clean 關閉後 Clean Count會歸零
    {
        iMin=atoi(fCleaning->edCleaningCount->Text.c_str());
    }
    else
    {
        for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)
        {
            for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
            {
                if(MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_FINSH_IC ||
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==CLEAN_FINISH_IC    ||
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_CLEAN_IC       ||   //Isaac 20180417 (jou) : fix auto clean做到一半按暫停, clean count會歸零
                   MOT[MMAutoCleanKit].Tray.Data[X][Y]==HAS_NULL_CLEAN_IC  )
                {
                    iNow=atoi(fMain->AutoCleanStringGrid->Cells[X][Y+1].c_str());
                    if(iNow!=0 && iNow<iMin)
                    {
                        iMin=iNow;
                    }
                }
            }
        }

        if(iMin==9999)                                                          //Steven 20171211 (Wei) : 修正計算方式
            iMin=0;

        AnsiString sAutoCleanCount=IntToStr(iMin);                              //Jimmychiu 20250103 : fixed for auto clean count to 0 issue
        fShowBinSelect->ed_AutoCleanCount->Text=sAutoCleanCount;                //Steven 20180524 : Fixed for clean count
        fCleaning->edCleaningCount->Text=sAutoCleanCount;
        Str.sprintf("Cleaned Count %d / %d", iMin, TestIF_File.iAutoClean_AlarmCount);
        fMain->pnlCleanCount->Caption=Str;                                      //Steven 20240731 : add for auto clean count
        if(iMin>=TestIF_File.iAutoClean_AlarmCount)
        {
            fMain->pnlCleanCountFont->Color=0x000000FF;                         // golden fMain->pnlCleanCount->Font->Color=clRed（VCL clRed=$000000FF；門面 forms/fMain.h:368）
            //Steven 20260427 : ATK P260427-ATK-H9-01 trigger alarm when reaching AlarmCount
            //                  Reuse WAR16313 (Autoclean count Alarm); replace with dedicated
            //                  WAR16314 after MDB Updater adds the new code.
            static int iLastAutoCleanAlarmCount=-1;
            if(TestIF_File.iAutoClean_AlarmCount>0 && iMin!=iLastAutoCleanAlarmCount)
            {
                iLastAutoCleanAlarmCount=iMin;
                AnsiString sLog;
                sLog.sprintf("Auto Clean Count reached AlarmCount: %d / %d", iMin, TestIF_File.iAutoClean_AlarmCount);
                RecordProcess(sLog);
                ShowErrorMessage("WAR1922", K_RETRY, MMSystem, false);          //JerryYang 20260814 :
            }
        }
        else
        {
            fMain->pnlCleanCountFont->Color=0x00800000;                         // golden clNavy（VCL $00800000）
        }
    }
    return iMin;
}

// ---------------------------------------------------------------------------
//  開機
// ---------------------------------------------------------------------------
// golden HT9045.cpp:226 Application->CreateForm(__classid(TfCleaning), &fCleaning)：DFM 設計期狀態 → 建構子（:44）
void FileRW_Cleaning_Boot()
{
    if (g_booted) return;
    // golden 只有一個 TfCleaning：移植樹門面 forms/fCleaning.h 已有的同名同型別元件直接當替身（AutoClean.cpp 的
    // SearchCleanNum／CheckCleaningCount 讀 fCleaning->edCleaningCount，要看到同一個物件）。udDeviceCT 不收養：
    // 門面是 TfCleaningUpDown，替身是 filerw::ELTrackBar（型別不同）。
    filerw::ELKeep(kForm, "edCleaningCount", fCleaning->edCleaningCount);
    filerw::ELKeep(kForm, "edPinSingleGf", fCleaning->edPinSingleGf);
    filerw::ELKeep(kForm, "edPinSingleN", fCleaning->edPinSingleN);
    filerw::ELKeep(kForm, "XCT1", fCleaning->XCT1);
    filerw::ELKeep(kForm, "XCT2", fCleaning->XCT2);
    filerw::ELKeep(kForm, "OutArmSpeed", fCleaning->OutArmSpeed);
    filerw::ELKeep(kForm, "edAlarmCount", fCleaning->edAlarmCount);
    filerw::ELKeep(kForm, "lblInArm", fCleaning->lblInArm);
    filerw::ELKeep(kForm, "lblInArmZ", fCleaning->lblInArmZ);
    filerw::ELKeep(kForm, "pgCleanType", fCleaning->pgCleanType);
    filerw::ELKeep(kForm, "sbCleanExit", fCleaning->sbCleanExit);
    CL_DfmItems();
    CL_DfmState();
    // udDeviceCT：DFM Associate=edDevicePices。filerw::ELTrackBar 在 Min／Max 改變時夾 Position 並寫回 Associate 的 Text；
    // Win32 up-down 的 UDM_SETRANGE 不會改 buddy 的文字，golden 也從不設 Position（只設 Min／Max／Increment，edDevicePices
    // 的 Text 由程式直接寫）→ 拿掉 Associate，免得 SetDeviceMaxMin 設 Min 時把 edDevicePices 蓋成 Position。
    EL<filerw::ELTrackBar>(kForm, "udDeviceCT")->Associate = nullptr;
    CL_CreateSaveProxies();
    CL_CreateContainerProxies(); { void FileRW_Cleaning_EvBoot(); FileRW_Cleaning_EvBoot(); }   //AI(W906-EVB3) 20260928 [W906]: form.event 控制項缺的替身＋DFM 父層（檔尾）；同一行附加
    BuildReads();
    g_page.saveReads = g_reads.data();
    g_page.nSaveReads = (int)g_reads.size();
    filerw::SessionBegin("");
    CL_TfCleaning();                                                            // golden :44（:51 LoadAutoCleanData）
    PrintSession("ctor (golden HT9045.cpp:226 CreateForm -> uCleaning.cpp:44)");
    g_booted = true;
    // golden TfSetup::ReadFile（cSetUp.cpp:2531-2535）的 fCleaning->LoadAutoCleanData()／SaveAutoCleanData()：移植樹 cSetUp.cpp 在
    // ht9045_sm（其他程式也連、不能直接連到只編進 wb_serve 的本檔）→ 經 hook（cSetUp.cpp 定義，同 g_W906_TrayAssignmentReadFileHook）
    {
        extern void (*g_W906_CleaningLoadAutoCleanDataHook)();
        extern void (*g_W906_CleaningSaveAutoCleanDataHook)();
        g_W906_CleaningLoadAutoCleanDataHook = &FileRW_Cleaning_LoadAutoCleanData;
        g_W906_CleaningSaveAutoCleanDataHook = &FileRW_Cleaning_SaveAutoCleanData;
    }
    std::printf("FileRW TestIF_File_Cleaning: TfCleaning proxies ready (%d save reads) -- iAutoClean_Function=%d DeveicePices=%d\n",
                g_page.nSaveReads, TestIF_File.iAutoClean_Function, TestIF_File.iAutoClean_DeveicePices);
}

// golden TfMain::FormShow main.cpp:11513-11526（開機序列裡，InitialOK=true :10898 之後）
void FileRW_Cleaning_MainFormShow()
{
    if (!g_booted) { std::printf("FileRW TestIF_File_Cleaning: MainFormShow skipped -- FileRW_Cleaning_Boot() has not run\n"); return; }
    filerw::SessionBegin("");
    CL_LoadAutoCleanData();                                                     // main.cpp:11513
    int iCount=FileRW_Cleaning_SearchCleanNum();                                // :11514 //Steven 20180524 : Fixed for clean count
    MOT[MMAutoCleanKit].fHasTray=true;                                          // :11515
    for(int Y=0; Y<MOT[MMAutoCleanKit].Tray.YItem; Y++)                         // :11516
    {
        for(int X=0; X<MOT[MMAutoCleanKit].Tray.XItem; X++)
        {
            if(MOT[MMAutoCleanKit].Tray.Data[X][Y]!=NULL_IC)                    //Steven 20210127 : HAS_CLEAN_IC --> NULL_IC
                SetAutoCleanStringGrid(X, Y+1, iCount);
        }
    }
    ReadWriteAutoCleanCount(false);                                             // :11525
    SetAutoCleanICCount(false);                                                 // :11526 //JerryYang 20180619 (wei) fix auto clean 資料錯誤
    PrintSession("TfMain::FormShow (golden main.cpp:11513-11526)");
    std::printf("FileRW TestIF_File_Cleaning: boot LoadAutoCleanData done -- InitialOK=%d iAutoClean_Function=%d Mode=%d "
                "DeveicePices=%d IntervalContact=%d clean count=%d\n",
                (int)InitialOK, TestIF_File.iAutoClean_Function, TestIF_File.iAutoClean_Mode,
                TestIF_File.iAutoClean_DeveicePices, TestIF_File.iAutoClean_IntervalContact, iCount);
}

// golden fCleaning->LoadAutoCleanData()／SaveAutoCleanData() 的外部呼叫點（cSetUp.cpp:2533-2534 TfSetup::ReadFile、:4043 SaveSetupFile；
// 移植樹 cSetUp.cpp GATE(G-SU-Clean)／(G-SU-CleanSave)、FileRW/TestIF_File_SetUp.gen.inc）。golden 的 fCleaning 在 CreateForm 之後
// 一定存在；這裡若還沒開機（替身沒建）就不跑並印出來 —— 那是開機順序錯了，不能靜默。
void FileRW_Cleaning_LoadAutoCleanData()
{
    if (!g_booted) { std::printf("FileRW TestIF_File_Cleaning: LoadAutoCleanData skipped -- FileRW_Cleaning_Boot() has not run (boot order)\n"); return; }
    CL_LoadAutoCleanData();
}
void FileRW_Cleaning_SaveAutoCleanData()
{
    if (!g_booted) { std::printf("FileRW TestIF_File_Cleaning: SaveAutoCleanData skipped -- FileRW_Cleaning_Boot() has not run (boot order)\n"); return; }
    CL_SaveAutoCleanData();
}

// ---- golden fContact->DutCount()／dDutCount／GetMaxIndexForceLimit()（ShowCleanContactForce :1746-1758）：移植樹 fContact 是
//      TfContactShim（atester_shims.h），真表單是 fContactForm（forms/fContact.h）。
#undef iDeviceCount
#undef iPosTemp
#undef bResetCleanCount
#undef b1x2SiteAbClosePutDummy
#undef b12SiteRun2x4
#include "forms/fContact.h"
#include "cContact.h"

void FileRW_Cleaning_ContactDutCount() { fContactForm->DutCount(); }                  // golden uCleaning.cpp:1746
double FileRW_Cleaning_ContactDutCountValue() { return fContactForm->dDutCount; }     // golden :1754-1755
// golden TfContact::GetMaxIndexForceLimit（cContact.cpp:19184，912）＝ cContact.h ComputeMaxIndexForceLimit（逐行相同的 if/else 階梯）；
// 只有 rgKitDiameter->ItemIndex 是表單元件 → C 路 TfContact 的替身（FileRW/DeviceForm_File.cpp 開機建，golden DFM ItemIndex=0）。
// 替身還沒建（FileRW_Contact_Boot 之前）＝ DFM 設計期值 0。
double FileRW_Cleaning_ContactMaxIndexForceLimit()
{
    const TRadioGroup* kit = dynamic_cast<TRadioGroup*>(filerw::ELFind("TfContact", "rgKitDiameter"));
    return ComputeMaxIndexForceLimit(INDEX_PRESS_TYPE, fContactForm->dDutCount, TestIF.iTestMode,
                                     IniConfig.bD27UseSingleSite85kg, kit ? kit->ItemIndex : 0);
}

// ===========================================================================
//  AI(W906-EVB3) 20260928 [W906] 批次 B3（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md CL-1／CL-2／CL-3／CL-5／CL-7；
//    Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」）—— WS form.event。
//    事件表＝產生的 kCL_Events（tools/editlist/TestIF_File_Cleaning.py 的 events，83 列）抄一份、處理器換成跳板 EvB3Run：
//      CL-1 btInclude            golden btIncludeClick（V912 AutoClean\uCleaning.cpp:2269-2283）：用伺服器當下的 TrayForm.Loader 填 Clean Tray 的
//                                起點／間距／格數（XPitch2／XCT2／YCT2 照 VCL 觸發 OnChange）＋SetDeviceMaxMin＋片數設成最小值。只改畫面元件。
//      CL-2 btnResetCleanCount   golden btnResetCleanCountClick（:2116-2138）：KYEC 條碼（Barcode_Reader，非 KYEC 回 2 照常走）→
//                                ReadWriteAutoCleanCount(false,true) 把清潔計數歸零並**寫檔**（HandlerCondition.Data 或 AutoCleanCount.Data）→
//                                主畫面／本頁計數格清 0 → SearchCleanNum → iAutoCleanAlarm 1→2 → SetAutoCleanICCount(false) → iAutoCleanAlarm=0、
//                                iIndexArmAutoCleanCnt=0 → SECS AutoCleanClearCount → bErrorAutoClean=false。照 golden 當下就做（動作鈕，不等存檔）。
//      CL-3 btnResetInterval     golden btnResetIntervalClick（:2842-2847）→ ChangeACSmartInterval(0)（:2779-2840，CosFunction.bSmartAutoClean &&
//                                TestIF.bACSmart 才做）：iAdaptiveACInterval=-1、記 AC Smart Interval 記錄檔、**直接寫配方**
//                                HandlerCondition.Data [Configuration] iAutoClean_IntervalContact＝-1 → fLotInfo->RefreshYieldMonitor()。
//                                //AI(W906-EVB3) 20260928: 照翻，但看起來是 golden 的漏洞 —— mode 0 把 -1 同時寫進 TestIF／TestIF_File 與配方，
//                                golden 接著 RefreshYieldMonitor_TERAPOWER（uLotInfo.cpp:13618）的 ChangeACSmartInterval(2) 是「回到 TestIF_File 的值」
//                                ＝還是 -1，所以按完配方裡的 Interval Contact 就是 -1。移植樹那一行是 GATE WD-2（forms/fLotInfo.cpp），不會再寫第二次
//                                （記錄檔少一行，值相同）。要不要改是 Steven 的 R 題。
//      CL-5 sbTrayAssign         golden sbTrayAssignClick（:2311-2315）：只動主畫面 —— fMain->MainFormSizeToEpson(false)（main.cpp:8966，換到 Motion View
//                                頁、調視窗、SECS EnterDebug）移植樹門面沒有 → ack.todo 記 "main:AutoCleanStringGrid …"（主畫面那一半列給 B6）；
//                                fMain->AutoCleanStringGrid->Visible=true 照 golden 寫門面（網頁主畫面目前沒有讀它）。
//      CL-7 rgCleanKitType／小鍵盤元件（kCL_Kb 77 格）：golden 的事件原本只在存檔時由 BeforeApply 重播（本檔檔頭「存檔前重播的 golden 事件」
//                                (1)(2)）；現在頁面每改一格就送一次，伺服器當場夾值、重算（XCT1Change／YCT1Change／XPitch2Change、ShowCleanContactForce、
//                                ShowTranGfToN／ShowTranNToGf、SetDeviceMaxMin…），ack.changed 回頁面。
//    跳板做的三件事：
//      (a) 小鍵盤：RunPageEvent 第 5 步已把頁面送的 text 套進替身；golden 的順序是「鍵盤打完 → 寫回 Text（值變了才觸發 OnChange）」，
//          所以先把替身還原成事件前的伺服器值（g_snap 裡那一格），再用 CL_KbPending＝頁面值跑 golden 處理器（CL_QwertyKey 夾限後
//          CL_EditText 寫回 → OnChange）。同 BeforeApply (2)。
//      (b) rgCleanKitType：同樣先還原，再 CL_RadioIndex（VCL SetItemIndex：夾範圍、值變了才 OnClick＝rgCleanKitTypeClick）。同 BeforeApply (1)。
//      (c) 其他（按鈕、cbbSelectTray）：直接跑產生的處理器。
//      每一次（處理器丟例外也一樣）之後更新存檔重播的基準 g_snap：處理器前後有變的替身＋送事件的那一格換成現在的值 ——
//      存檔時 BeforeApply 就不會把已經當場跑過的事件再跑一次；頁面沒送到的（網路失敗、舊頁面）照舊在存檔時補重播。
//      state（RunPageEvent 第 4 步）只改了、處理器沒動到的格子不進基準（存檔時它們照「頁面最後狀態」收）。
//    ⚠ 頁面的 state 不能帶事件表裡的元件：替身被 state 靜默改掉之後，存檔重播看到「伺服器值＝頁面值」就不會觸發 OnChange（例 XCT1 的夾值）。
//      web/page/ht9045_cleaning_ev.js 照這條送。伺服器端 29a13bdb（RunPageEvent 第 4 步）會丟掉有事件列的元件，但 TCustomEdit 只有
//      click 列（＝這一頁的 77 個小鍵盤格）的照收 ⇒ 小鍵盤格仍只靠頁面不放（交件列給 ST01-E）。
//    ⚠ 29a13bdb 說明裡「Cleaning 存檔比的是開頁快照」從這一批起不再完全成立：g_snap 在每個事件後更新（上面 (2)）。
//    產生器只替 golden 方法本體裡用到的名字建替身：btnResetInterval（golden uCleaning.h TButton，DFM :800 在 tsSmart :684 裡）不在任何
//    本體裡 → FileRW_Cleaning_EvBoot 開機時建、接 DFM 父層（同 d7fa099a 的 ArmSpeed spbSetToDef）；其他 82 個事件控制項產生器已建。
// ===========================================================================
#include <stdexcept>

namespace {
const char* const kCL_EvParents[][2] = {{"btnResetInterval", "tsSmart"}};   // golden uCleaning.dfm:684 tsSmart > :800 btnResetInterval

// g_snap 裡某一格的值（{text|itemIndex…}）
const cJSON* SnapOf(const cJSON* snap, const char* name) { return snap ? cJSON_GetObjectItemCaseSensitive(snap, name) : nullptr; }

// (a)(b)(c) 之後：前後有變的替身＋sender 那一格 → g_snap（見上面說明）
void EvB3Merge(const std::string& preJson, const char* sender)
{
    cJSON* snap = cJSON_Parse(g_snap.c_str());
    if (!snap || !cJSON_IsObject(snap)) { if (snap) cJSON_Delete(snap); snap = cJSON_CreateObject(); }
    cJSON* pre = cJSON_Parse(preJson.c_str());
    cJSON* post = cJSON_Parse(filerw::ProxyStateJson(kForm).c_str());
    for (const cJSON* it = post ? post->child : nullptr; it; it = it->next) {
        if (!it->string) continue;
        const cJSON* p = SnapOf(pre, it->string);
        if (p && cJSON_Compare(p, it, 1) && std::strcmp(it->string, sender) != 0) continue;
        cJSON_DeleteItemFromObjectCaseSensitive(snap, it->string);
        cJSON_AddItemToObject(snap, it->string, cJSON_Duplicate(it, 1));
    }
    char* s = cJSON_PrintUnformatted(snap);
    if (s) { g_snap = s; cJSON_free(s); }
    if (post) cJSON_Delete(post);
    if (pre) cJSON_Delete(pre);
    cJSON_Delete(snap);
}

void EvB3Body(const filerw::PageEvent& e, TControl* sender)
{
    cJSON* snap = cJSON_Parse(g_snap.c_str());
    const cJSON* base = SnapOf(snap, e.control);
    // (a) 小鍵盤元件
    for (const CL_KbEntry& k : kCL_Kb) {
        if (std::strcmp(k.name, e.control) != 0) continue;
        TCustomEdit* ed = dynamic_cast<TCustomEdit*>(sender);
        const AnsiString typed = ed ? ed->Text : AnsiString("");
        const cJSON* bt = base ? cJSON_GetObjectItemCaseSensitive(base, "text") : nullptr;
        if (ed && bt && cJSON_IsString(bt)) ed->Text = AnsiString(bt->valuestring);   // 還原成事件前的伺服器值（不觸發 OnChange）
        if (snap) cJSON_Delete(snap);
        CL_KbPending = &typed;
        try {
            k.fn(sender);                                                       // golden 小鍵盤處理器（Sender＝這個元件）
        } catch (...) {
            CL_KbPending = nullptr;
            throw;
        }
        CL_KbPending = nullptr;
        return;
    }
    // (b) rgCleanKitType：VCL TCustomRadioGroup.SetItemIndex → OnClick
    if (std::strcmp(e.control, "rgCleanKitType") == 0) {
        TRadioGroup* rg = dynamic_cast<TRadioGroup*>(sender);
        const cJSON* bi = base ? cJSON_GetObjectItemCaseSensitive(base, "itemIndex") : nullptr;
        const bool haveBase = bi && cJSON_IsNumber(bi);
        const int baseIndex = haveBase ? bi->valueint : -1;
        if (snap) cJSON_Delete(snap);
        if (!rg) throw std::runtime_error("EVB3: rgCleanKitType proxy is not a TRadioGroup");
        const int want = rg->ItemIndex;
        if (haveBase) rg->ItemIndex = baseIndex;                                // 還原
        CL_RadioIndex(rg, want);                                                // golden :2214 rgCleanKitTypeClick（值變了才跑）
        return;
    }
    if (snap) cJSON_Delete(snap);
    e.handler(sender);                                                          // (c) 產生的處理器
}

// RunPageEvent 以 Sender＝ELFind(TfCleaning, control) 呼叫 → 找回 kCL_Events 那一列
void EvB3Run(TControl* sender)
{
    const filerw::PageEvent* e = nullptr;
    for (std::size_t i = 0; i < sizeof(kCL_Events) / sizeof(kCL_Events[0]) && !e; ++i)
        if (sender && filerw::ELFind(kForm, kCL_Events[i].control) == sender) e = &kCL_Events[i];
    if (!e) throw std::runtime_error("EVB3: form.event sender is not a TfCleaning event control proxy");
    const std::string pre = filerw::ProxyStateJson(kForm);
    try {
        EvB3Body(*e, sender);
    } catch (...) {
        EvB3Merge(pre, e->control);                                             // 例外之前改過的替身 golden 不還原（同 RunPageEvent 第 6 步）
        throw;
    }
    EvB3Merge(pre, e->control);
}

const filerw::PageEvent* EvB3Table()
{
    static filerw::PageEvent t[sizeof(kCL_Events) / sizeof(kCL_Events[0])];
    for (std::size_t i = 0; i < sizeof(kCL_Events) / sizeof(kCL_Events[0]); ++i) {
        t[i] = kCL_Events[i];
        t[i].handler = &EvB3Run;
    }
    return t;
}
}  // namespace

void FileRW_Cleaning_EvBoot()
{
    EL<TButton>(kForm, "btnResetInterval");                                    // golden uCleaning.h TButton *btnResetInterval（DFM :800）
    filerw::ELSetParents(kForm, kCL_EvParents, (int)(sizeof(kCL_EvParents) / sizeof(kCL_EvParents[0])));
    for (std::size_t i = 0; i < sizeof(kCL_Events) / sizeof(kCL_Events[0]); ++i)
        if (!filerw::ELFind(kForm, kCL_Events[i].control))
            std::printf("FileRW TestIF_File_Cleaning: WARNING form.event control %s has no proxy (add it to FileRW_Cleaning_EvBoot)\n",
                        kCL_Events[i].control);
}
