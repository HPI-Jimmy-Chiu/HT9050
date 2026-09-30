// =============================================================================
//  test_formevent_position.cpp -- WS form.event 的「控制項自己的新值」：position（TTrackBar／TScrollBar／TUpDown）與
//                                 activePageIndex（TPageControl）
//
//  AI(W906-EVB1) 20260928 [W906] X-2（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B1）
//  受測：FileRW/_FormEvent.cpp W906_FormEvent（解析兩個新鍵、A 形狀帶了就拒）、FileRW/_EditPage.cpp RunPageEvent
//    （第 3 步先驗、第 5 步先套再跑處理器、被夾過補 changed／todo）、FileRW/_EditList.cpp ELPageIndexRefused（同一種值的判斷）。
//    [1] 型別：position／activePageIndex 不是整數（字串、小數、布林、超出 ±1e9）⇒ bad-payload，處理器沒跑、替身沒動；null＝沒帶
//    [2] position 先套再跑處理器：處理器看到新值、只跑一次、替身自己的 OnChange 指標沒有再被觸發（不會跑兩次）；
//        changed 帶處理器改的欄位、不帶滑桿自己（頁面已經是那個值）
//    [3] 同一個值再送一次：處理器照跑（VCL TTrackBar 使用者拖動 CN_HSCROLL 一律 Changed）
//    [4] 超出 Min..Max：照 VCL 夾值、處理器看到夾過的值；changed 補帶 {"<滑桿>":{"position":實際值}}、todo 記一筆
//    [5] 沒帶 position＝舊行為：處理器看到伺服器端目前的值，todo 空
//    [6] state 裡控制項自己的 position 不算（以 position 鍵為準）；state 的別條滑桿照套、不觸發它的 OnChange
//    [7] position 帶在不是滑桿的元件（勾選框）⇒ bad-payload，state 也沒套（先驗後套）
//    [8] position 帶在 TUpDown 的 btNext（處理器自己照 Increment 走一格）⇒ bad-payload；不帶＝舊行為照走一格
//    [9] activePageIndex：先設分頁再跑處理器；超出範圍（父子表下限／ELSetPageOrder 精確）、負數、事件不是 change、
//        帶在不是分頁控制的元件 ⇒ bad-payload，分頁與 state 都沒動；處理器自己改回分頁 ⇒ changed 帶 activePageIndex；
//        沒帶＝舊行為
//    [10] 點不到的元件（自己停用、容器停用）照舊在第 2 步擋（先於新鍵的檢查）
//    [11] A 形狀（formbridge）帶新鍵 ⇒ bad-payload，RunEvent 沒被呼叫；不帶＝照舊分派
//    [12] ELPageIndexRefused 直接呼叫：合法回 ""、不是分頁控制回理由、不改替身
//    [13] 運轉中（SystemStart）照舊先擋
//  AI(W906-EVB1) 20260928 [W906] 第 4 步 state 的事件控制項（B2 頁面工程師回報：state 能不跑處理器就改掉有互鎖的控制項）：
//    [14] 事件表上有自己一列的控制項（單選群組、勾選框、UpDown、分頁控制）state 一律丟、伺服器保留原值、處理器沒跑；跟伺服器值不同的
//         各記一筆 todo，相同的不記；沒有事件列的照套；事件控制項的 state 型別不對也只是丟（不整批拒），沒有事件列的型別不對照舊拒；
//         要改它就送它自己的 form.event（處理器照跑、互鎖照查）
//    [15] TCustomEdit：只有 "click" 列（點一下開小鍵盤）的可打字輸入框 state 照收（golden 打字不經 OnClick）；有 "change" 列的丟；
//         ReadOnly 的照舊由 ELEditable 丟（不記 todo）
//    [16] 存檔（editlist.save）端：事件 state 被丟之後，存檔 BeforeApply 看到頁面值≠伺服器值 ⇒ 照 golden 重播、互鎖改回；
//         對照組：替身若被「不跑處理器」改掉（舊的第 4 步），BeforeApply 看到相同就不重查 —— 這一次修的洞
//  AI(W906-EVB10A) 20260929 [W906] 事件批次 B10 part a（SU-9／TS-10 的 Exit 鈕走 form.event）：
//    [17] ack.closed：處理器照 golden Close()（產生器轉成 "closed" 記號）⇒ closed:true、filerw::PageShownNow 變 false（之後的事件要重新開頁）；
//         守衛 return（沒有 Close）⇒ closed:false、訊息帶回、頁還開著；別的事件的 ack 也帶 closed:false
//  AI(W906-EVB10C-BC) 20260930 [W906] B10c 後續（BarCode 的 Exit 鈕 golden TfBarCode::sbtExitClick，V912 BarCode\BarCode.cpp:2408-2417）：
//    [18] 沒有擋關條件的 Exit：closed:true、FormClose 只跑一次、兩顆 2DID 檢查鈕在 changed 裡變回可按、兩個全域設 1；之後頁面關視窗時
//         關窗邊緣（filerw::PageCloseEdgeRefused，FileRW_BarCode_WindowEdge 用的同一支）拒絕再跑 FormClose；連點第二下＝reload page；
//         關窗邊緣清掉（W906_EditPageWindowClosed）之後重新開窗，✕ 又會跑 FormClose。真的產生碼（BC_sbtExitClick）只編進 wb_serve，不在這裡。
//  NOT COVERED：真的結構頁（Setup.Speed 的 R119 包裝 R119EvKeep、tb*Change 的 golden 本體）—— 要 god-stack＋配方檔
//    （FileRW/ArmSpeed_File.cpp 只編進 wb_serve），由整合者用 wb_serve 探針驗（交件列了步驟）。
//  god-stack 全域（AccessLevel、SystemStart、SoftStart、fSecurity…）、golden NewRecordProcess／RecordProcess、
//  HTEditList_RegisterControlName、JsonBridge 的 FindBridge／RunEvent／FormLock 由本檔給替身（同 test_openenter_log.cpp）。
//  不 link god-stack、不讀寫任何檔，秒級。
// =============================================================================
#include "FileRW/_FormEvent.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_EditList.h"
#include "JsonBridge/FormBridge.h"
#include "JsonBridge/FormJson.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "forms/fSecurity.h"

// ---- god-stack 替身（型別照 cmydef.h／cprod.h／Config.h／CosFunction.h／forms/fSecurity.h 的宣告；同 test_openenter_log.cpp）----
int  AccessLevel = 0;              // cmydef.h:3506
bool SystemStart = false;          // cmydef.h:221
bool SoftStart = false;            // cmydef.h:223（FileRW/_FormEvent.cpp 自己宣告）
int  iHome = 0;                    // cmydef.h:260
int  WEIGHT_CALIBRATION = 0;       // cmydef.h:2833
int  CUSTOMER_CODE = 0;            // cmydef.h:3184
int  iDefHonPrecLevel = 3;         // cmydef.h:3591
bool bResetMNet = false;           // Motor/myMN200motor.h:198（_EditPage.cpp 自己宣告）
int  iMaxLevelItem = 0;            // cSecurity.cpp:47（_EditPage.cpp 自己宣告）
bool authMainForm[12];             // cAuthority.h:56（_EditPage.cpp 自己宣告，開窗閘 GAuth 用；AI(W906-AUTHMAINFORM) 20260930）——本測試不經開窗閘
LAST_LEVEL_SET LevelSet;           // cprod.h:1154
HT9045_CONFIG IniConfig;           // Config.h:1516
HT9045_COUSTOMER_FUNCTION CosFunction;   // CosFunction.h:503
TfSecurity* fSecurity = nullptr;   // forms/fSecurity.h:609（本測試不經開窗閘）
bool TfSecurity::Insufficient(int, bool) { return true; }   // 只為了連結
void HTEditList_RegisterControlName(TControl*, const AnsiString&) {}   // Public/HTEditList.cpp:214 的替身
void NewRecordProcess(AnsiString, AnsiString, AnsiString) {}          // cMyDB.h:129（本測試的 tag 不在 kOpenEnters，不會被叫）
void RecordProcess(AnsiString, AnsiString) {}                         // cMyDB.h:130
bool W906_FormShowing(const char*, bool member) { return member; }   // csystem.cpp:30049 的替身＝hook 是 0 時的答案；FileRW/_FormEvent.cpp 運轉中例外表要它（AI(W906-FE-RUNEXC) 20260930；本測試的頁不在表上）

// ---- JsonBridge 替身：A 形狀只有一個假頁 "Setup.FakeA"，RunEvent 記次數 ----
static int g_aRuns = 0;
namespace ht9045 {
namespace formbridge {
static const BridgeDesc kFakeA = {"Setup.FakeA.html", "TfFakeA", "fakeA.cpp", nullptr, nullptr, nullptr, "", "", nullptr, 0, "cbFake"};
const BridgeDesc* FindBridge(const std::string& page) { return page == "Setup.FakeA.html" ? &kFakeA : nullptr; }
bool RunEvent(const BridgeDesc&, const formevent::Request&, formevent::Result* out)
{
    ++g_aRuns;
    out->golden = "fakeA.cpp:1 TfFakeA::cbFakeChange";
    return true;
}
}  // namespace formbridge
namespace formjson {
void FormLock() {}
void FormUnlock() {}
}  // namespace formjson
}  // namespace ht9045

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

using filerw::EL;
using filerw::ELTrackBar;

static const char* const kF = "TfEvB1Test";   // 假的 golden 表單類別
static const char* const kTag = "EvB1Test";   // 假的 C 路結構名（WS form.event 的 tag）

// ---- 假的 C 路頁 ----
static void FakeFormShow() {}
static void FakeSaveFlow() { filerw::ELMark("SaveSetupFile"); }   // [16]：當成 golden 存檔鈕寫了檔（PageSave 就不 reload）
static void FakeReload() {}
static bool FakeBooted() { return true; }
static void FakeBeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled);   // [16]：模仿 FileRW/TrayForm.cpp BeforeApply (1)
static const filerw::PageDesc kPage = {
    kTag, kF, "Setup.EvB1Test.html", nullptr, nullptr, 0, nullptr, 0,
    FakeFormShow, FakeSaveFlow, "SaveSetupFile", FakeReload, FakeBooted, FakeBeforeApply, nullptr};

// ---- 處理器：記下看到的值（golden tb*Change 就是讀 Position 寫欄位）----
static int g_runs = 0, g_seenPos = -999, g_seenTab = -999;
static int g_hookA = 0, g_hookB = 0;   // 替身自己的 OnChange 指標被觸發幾次（不該被 RunPageEvent 觸發）
static void HookA() { ++g_hookA; }
static void HookB() { ++g_hookB; }
static std::string Num(int v)
{
    char b[24];
    std::snprintf(b, sizeof(b), "%d", v);
    return b;
}
static void H_tbA(TControl* s)   // 模仿 golden tbEPControlChange：edA->Text = Position
{
    ++g_runs;
    ELTrackBar* t = static_cast<ELTrackBar*>(s);
    g_seenPos = t->Position;
    EL<TEdit>(kF, "edA")->Text = AnsiString(Num(t->Position).c_str());
}
static void H_udBNext(TControl* s)   // 模仿 IniConfig udD46 btNext：伺服器自己走一格（Associate 跟著）
{
    ++g_runs;
    ELTrackBar* t = static_cast<ELTrackBar*>(s);
    t->Position = t->Position + t->Increment;
}
static void H_cbC(TControl*) { ++g_runs; }
static void H_pc(TControl* s)
{
    ++g_runs;
    g_seenTab = static_cast<TPageControl*>(s)->ActivePageIndex;
}
static void H_pcWRevert(TControl* s)   // 模仿 golden「不准換頁、改回第 0 頁」
{
    ++g_runs;
    g_seenTab = static_cast<TPageControl*>(s)->ActivePageIndex;
    static_cast<TPageControl*>(s)->ActivePageIndex = 0;
}
// [14]〜[16]：模仿 golden TfTrayAssignment::rgFixTrayModeClick（V912 cTrayAssignment.cpp:1166-1194）—— Fix 盤有 IC 就改回
//   TrayForm.iFixTrayMode；沒有 IC 才收下新模式
static bool g_fixHasIC = false;
static int g_fixMode = 0, g_fixClicks = 0;
static void H_rgFix(TControl* s)
{
    ++g_runs;
    ++g_fixClicks;
    TRadioGroup* g = static_cast<TRadioGroup*>(s);
    if (g_fixHasIC) g->ItemIndex = g_fixMode;
    else g_fixMode = g->ItemIndex;
}
static void H_edClick(TControl*) { ++g_runs; }   // 模仿「點輸入框開小鍵盤」
static void H_edChange(TControl*) { ++g_runs; }
// AI(W906-EVB10A) 20260929 [W906] [17]：模仿 golden sbtExitClick（例 V912 cSetUp.cpp:3476）—— 守衛過了才 Close()（產生器轉成 "closed" 記號）
static int g_exitRuns = 0;
static bool g_exitGuard = false;
static void H_btExit(TControl*) { ++g_exitRuns; if (g_exitGuard) { filerw::ELMessage("Auto Shuttle Sensor Need Reset!!"); return; } filerw::ELMark("closed"); }
// AI(W906-EVB10C-BC) 20260930 [W906] [18]：跟產生器轉出的 BarCode Exit 鈕同一個形狀（FileRW/TestIF_File_BarCode.gen.inc BC_sbtExitClick＝golden V912
//   BarCode\BarCode.cpp:2408-2417）：沒有擋關的條件；Close()＝記 "closed"＋跑 FormClose；bShow=false；兩個全域 task=1；兩顆 2DID 檢查鈕 Enabled=true
static int g_bcFormClose = 0, g_bcTask1 = 0, g_bcTask2 = 0;
static bool g_bcShow = true;
static void H_bcFormClose() { ++g_bcFormClose; g_bcShow = false; }   // golden FormClose :531 的 bShow=false（其餘 tmr1／DoIniDataToForm 不在這裡）
static void H_bcExit(TControl*)
{
    ;                                                   // :2410 sbtExit->Down=false（純畫面）
    filerw::ELMark("closed"); H_bcFormClose();          // :2411 Close()
    g_bcShow = false;                                   // :2412
    g_bcTask1 = 1;                                      // :2413 i2DIDCheckSH1Task=1
    g_bcTask2 = 1;                                      // :2414 i2DIDCheckSH2Task=1
    EL<TSpeedButton>(kF, "btChk1")->Enabled = true;     // :2415 btStart2DIDCheckSh1->Enabled=true
    EL<TSpeedButton>(kF, "btChk2")->Enabled = true;     // :2416 btStart2DIDCheckSh2->Enabled=true
}
static void FakeBeforeApply(const std::string& widgetsJson, std::vector<std::string>* handled)
{
    cJSON* root = cJSON_Parse(widgetsJson.c_str());
    const cJSON* o = root ? cJSON_GetObjectItemCaseSensitive(root, "rgFix") : nullptr;
    const cJSON* v = cJSON_IsObject(o) ? cJSON_GetObjectItemCaseSensitive(o, "itemIndex") : nullptr;
    TRadioGroup* g = EL<TRadioGroup>(kF, "rgFix");
    if (v && cJSON_IsNumber(v) && v->valueint != g->ItemIndex && filerw::ELOperable(kF, "rgFix")) {   // 同 TrayForm.cpp:345-347
        g->ItemIndex = v->valueint;
        H_rgFix(g);
        handled->push_back("rgFix");
    }
    if (root) cJSON_Delete(root);
}
static const filerw::PageEvent kEvents[] = {
    {"tbA", "change", "fake.cpp:10 TfEvB1Test::tbAChange", &H_tbA},
    {"tbOff", "change", "fake.cpp:11 TfEvB1Test::tbOffChange", &H_tbA},
    {"udB", "btNext", "fake.cpp:12 TfEvB1Test::udBClick(btNext)", &H_udBNext},
    {"cbC", "click", "fake.cpp:13 TfEvB1Test::cbCClick", &H_cbC},
    {"pcX", "change", "fake.cpp:14 TfEvB1Test::pcXChange", &H_pc},
    {"pcX", "click", "fake.cpp:15 TfEvB1Test::pcXClick", &H_pc},
    {"pcY", "change", "fake.cpp:16 TfEvB1Test::pcYChange", &H_pc},
    {"pcZ", "change", "fake.cpp:17 TfEvB1Test::pcZChange", &H_pc},
    {"pcW", "change", "fake.cpp:18 TfEvB1Test::pcWChange", &H_pcWRevert},
    {"rgFix", "click", "fake.cpp:19 TfEvB1Test::rgFixClick", &H_rgFix},
    {"edK", "click", "fake.cpp:20 TfEvB1Test::edKClick", &H_edClick},
    {"edC", "change", "fake.cpp:21 TfEvB1Test::edCChange", &H_edChange},
    {"edR", "click", "fake.cpp:22 TfEvB1Test::edRClick", &H_edClick},
    {"btExit", "click", "fake.cpp:23 TfEvB1Test::btExitClick", &H_btExit},   // AI(W906-EVB10A) 20260929 [W906] [17]
    {"btBcExit", "click", "fake.cpp:24 TfEvB1Test::btBcExitClick", &H_bcExit},   // AI(W906-EVB10C-BC) 20260930 [W906] [18]
};

// ---- 送一次 form.event、解析 ack ----
struct Ev {
    bool ok = false;
    std::string err;
    cJSON* ack = nullptr;
    ~Ev() { if (ack) cJSON_Delete(ack); }
};
static void Send(Ev* e, const char* tag, const std::string& value)
{
    std::string ack;
    e->ok = W906_FormEvent(tag, value, &ack, &e->err);
    e->ack = e->ok ? cJSON_Parse(ack.c_str()) : nullptr;
    if (!e->ok) std::printf("       (err) %s\n", e->err.c_str());
}
static std::string Val(const char* control, const char* event, const std::string& extra)
{
    return std::string("{\"form\":\"") + kF + "\",\"control\":\"" + control + "\",\"event\":\"" + event + "\"" +
           (extra.empty() ? "" : "," + extra) + "}";
}
static const cJSON* Changed(const Ev& e, const char* name, const char* key)
{
    const cJSON* ch = e.ack ? cJSON_GetObjectItemCaseSensitive(e.ack, "changed") : nullptr;
    const cJSON* o = ch ? cJSON_GetObjectItemCaseSensitive(ch, name) : nullptr;
    return o && key ? cJSON_GetObjectItemCaseSensitive(o, key) : o;
}
static int TodoCount(const Ev& e)
{
    const cJSON* t = e.ack ? cJSON_GetObjectItemCaseSensitive(e.ack, "todo") : nullptr;
    return t && cJSON_IsArray(t) ? cJSON_GetArraySize(t) : -1;
}
static bool TodoHas(const Ev& e, const char* s)
{
    const cJSON* t = e.ack ? cJSON_GetObjectItemCaseSensitive(e.ack, "todo") : nullptr;
    for (const cJSON* x = t ? t->child : nullptr; x; x = x->next)
        if (cJSON_IsString(x) && std::strstr(x->valuestring, s)) return true;
    return false;
}
static bool ErrHas(const Ev& e, const char* s) { return !e.ok && e.err.find(s) != std::string::npos; }
static std::string TextOf(TCustomEdit* x) { return x->Text.c_str(); }

int main()
{
    // ---- 替身：pnlMain ⊃ {tbA, tbB, tbOff, udB, edA, edB, edS, cbC, pcX, pcY, pcW}；pcX ⊃ {ts0, ts1, ts2}；
    //      pcW ⊃ {tsW0, tsW1}；pcY 用 ELSetPageOrder 登記 2 頁（父子表裡沒有它的分頁）；pnlOff（停用）⊃ pcZ ⊃ tsZ0
    EL<TPanel>(kF, "pnlMain");
    TPanel* pnlOff = EL<TPanel>(kF, "pnlOff");
    ELTrackBar* tbA = EL<ELTrackBar>(kF, "tbA");
    ELTrackBar* tbB = EL<ELTrackBar>(kF, "tbB");
    ELTrackBar* tbOff = EL<ELTrackBar>(kF, "tbOff");
    ELTrackBar* udB = EL<ELTrackBar>(kF, "udB");
    TEdit* edA = EL<TEdit>(kF, "edA");
    TEdit* edB = EL<TEdit>(kF, "edB");
    TEdit* edS = EL<TEdit>(kF, "edS");
    TCheckBox* cbC = EL<TCheckBox>(kF, "cbC");
    TPageControl* pcX = EL<TPageControl>(kF, "pcX");
    TPageControl* pcY = EL<TPageControl>(kF, "pcY");
    TPageControl* pcZ = EL<TPageControl>(kF, "pcZ");
    TPageControl* pcW = EL<TPageControl>(kF, "pcW");
    EL<TTabSheet>(kF, "ts0");
    EL<TTabSheet>(kF, "ts1");
    EL<TTabSheet>(kF, "ts2");
    EL<TTabSheet>(kF, "tsZ0");
    EL<TTabSheet>(kF, "tsW0");
    EL<TTabSheet>(kF, "tsW1");
    TRadioGroup* rgFix = EL<TRadioGroup>(kF, "rgFix");   // [14]〜[16]
    rgFix->Items->Add("Normal");
    rgFix->Items->Add("Fix");
    rgFix->ItemIndex = 0;
    TEdit* edK = EL<TEdit>(kF, "edK");                   // [15]：只有 click 列、可打字
    TEdit* edC = EL<TEdit>(kF, "edC");                   // [15]：有 change 列
    TEdit* edR = EL<TEdit>(kF, "edR");                   // [15]：click 列、ReadOnly
    filerw::ELSetReadOnly(kF, "edR");
    static const char* const kPairs[][2] = {
        {"tbA", "pnlMain"}, {"tbB", "pnlMain"}, {"tbOff", "pnlMain"}, {"udB", "pnlMain"}, {"edA", "pnlMain"},
        {"edB", "pnlMain"}, {"edS", "pnlMain"}, {"cbC", "pnlMain"}, {"pcX", "pnlMain"}, {"pcY", "pnlMain"},
        {"pcW", "pnlMain"}, {"ts0", "pcX"}, {"ts1", "pcX"}, {"ts2", "pcX"}, {"tsW0", "pcW"}, {"tsW1", "pcW"},
        {"pcZ", "pnlOff"}, {"tsZ0", "pcZ"},
        {"rgFix", "pnlMain"}, {"edK", "pnlMain"}, {"edC", "pnlMain"}, {"edR", "pnlMain"},
    };
    filerw::ELSetParents(kF, kPairs, (int)(sizeof(kPairs) / sizeof(kPairs[0])));
    static const char* const kYTabs[] = {"tsY0", "tsY1"};
    filerw::ELSetPageOrder(kF, "pcY", kYTabs, 2);
    tbA->DfmInit(1, 100, 1);           // 同 golden cSpeed.dfm tbAllSpeed Min=1 Max=100 Position=1
    tbA->OnChange = &HookA;
    tbB->DfmInit(0, 15, 0);            // 同 golden cTrayAssignment.dfm sbNormalTest（TScrollBar）Max=15
    tbB->OnChange = &HookB;
    tbOff->DfmInit(1, 100, 1);
    tbOff->Enabled = false;            // 同 golden TfSpeed 建構子 tbAllSpeed->Enabled=false（cSpeed.cpp:35）
    udB->DfmInit(5, 15, 5);            // 同 golden cConfiguration.dfm udD46（TUpDown）
    udB->Associate = edB;
    pnlOff->Enabled = false;

    filerw::RegisterPage(&kPage);
    filerw::RegisterPageEvents(kTag, kEvents, (int)(sizeof(kEvents) / sizeof(kEvents[0])));
    {
        std::string json;
        CHECK(filerw::PageJson(kPage, &json) == 200);   // 「開過頁」（RunPageEvent 第 1 步）
        cJSON* j = cJSON_Parse(json.c_str());
        const cJSON* ev = j ? cJSON_GetObjectItemCaseSensitive(j, "events") : nullptr;
        const cJSON* a = ev ? cJSON_GetObjectItemCaseSensitive(ev, "tbA") : nullptr;
        const cJSON* aEv = a ? cJSON_GetObjectItemCaseSensitive(a, "event") : nullptr;
        CHECK(aEv && cJSON_IsString(aEv) && std::strcmp(aEv->valuestring, "change") == 0);
        if (j) cJSON_Delete(j);
    }

    std::printf("[1] wrong types -> bad-payload, nothing ran, nothing moved; null = not sent\n");
    {
        const char* const kBad[] = {"\"position\":\"12\"", "\"position\":1.5", "\"position\":1e12", "\"position\":true"};
        for (const char* b : kBad) {
            Ev e;
            Send(&e, kTag, Val("tbA", "change", b));
            CHECK(ErrHas(e, "bad-payload: position must be a whole number or null"));
        }
        const char* const kBadTab[] = {"\"activePageIndex\":\"1\"", "\"activePageIndex\":0.5", "\"activePageIndex\":false"};
        for (const char* b : kBadTab) {
            Ev e;
            Send(&e, kTag, Val("pcX", "change", b));
            CHECK(ErrHas(e, "bad-payload: activePageIndex must be a whole number or null"));
        }
        CHECK(g_runs == 0 && (int)tbA->Position == 1 && pcX->ActivePageIndex == 0);
        Ev n;
        Send(&n, kTag, Val("tbA", "change", "\"position\":null,\"activePageIndex\":null"));
        CHECK(n.ok && g_runs == 1 && g_seenPos == 1 && (int)tbA->Position == 1);
    }

    std::printf("[2] position is set first, then the table's OnChange runs once (no double fire)\n");
    {
        g_runs = 0; g_hookA = 0;
        Ev e;
        Send(&e, kTag, Val("tbA", "change", "\"position\":37"));
        CHECK(e.ok && g_runs == 1 && g_seenPos == 37 && g_hookA == 0);
        CHECK((int)tbA->Position == 37 && TextOf(edA) == "37");
        const cJSON* t = Changed(e, "edA", "text");
        CHECK(t && cJSON_IsString(t) && std::strcmp(t->valuestring, "37") == 0);
        CHECK(Changed(e, "tbA", nullptr) == nullptr);   // 頁面已經是 37，不回
        CHECK(TodoCount(e) == 0);
    }

    std::printf("[3] same value again: the handler still runs (VCL TTrackBar user drag always calls Changed)\n");
    {
        g_runs = 0;
        Ev e;
        Send(&e, kTag, Val("tbA", "change", "\"position\":37"));
        CHECK(e.ok && g_runs == 1 && g_seenPos == 37 && g_hookA == 0);
    }

    std::printf("[4] outside Min..Max: clamped like VCL; changed carries the real position; todo notes it\n");
    {
        g_runs = 0;
        Ev hi;
        Send(&hi, kTag, Val("tbA", "change", "\"position\":150"));
        CHECK(hi.ok && g_runs == 1 && g_seenPos == 100 && (int)tbA->Position == 100 && TextOf(edA) == "100");
        const cJSON* p = Changed(hi, "tbA", "position");
        CHECK(p && cJSON_IsNumber(p) && p->valueint == 100);
        CHECK(TodoCount(hi) == 1 && TodoHas(hi, "position 150 of tbA is outside Min..Max (1..100)") && TodoHas(hi, "ran with 100"));
        const cJSON* t = Changed(hi, "edA", "text");
        CHECK(t && cJSON_IsString(t) && std::strcmp(t->valuestring, "100") == 0);   // 處理器改的照樣帶
        Ev lo;
        Send(&lo, kTag, Val("tbA", "change", "\"position\":-5"));
        const cJSON* q = Changed(lo, "tbA", "position");
        CHECK(lo.ok && g_seenPos == 1 && q && cJSON_IsNumber(q) && q->valueint == 1 && TodoHas(lo, "(1..100)"));
        CHECK(g_hookA == 0);
    }

    std::printf("[5] no position = old behaviour: the handler sees the server's current Position\n");
    {
        tbA->SetPosition(42, false);
        g_runs = 0;
        Ev e;
        Send(&e, kTag, Val("tbA", "change", ""));
        CHECK(e.ok && g_runs == 1 && g_seenPos == 42 && (int)tbA->Position == 42 && TodoCount(e) == 0);
        CHECK(Changed(e, "tbA", nullptr) == nullptr);
    }

    std::printf("[6] state's own entry is not used (position wins); other sliders in state are applied without OnChange\n");
    {
        g_runs = 0; g_hookB = 0;
        Ev e;
        Send(&e, kTag, Val("tbA", "change", "\"position\":20,\"state\":{\"tbA\":{\"position\":5},\"tbB\":{\"position\":9},\"edS\":{\"text\":\"hello\"}}"));
        CHECK(e.ok && g_runs == 1 && g_seenPos == 20 && (int)tbA->Position == 20);
        CHECK((int)tbB->Position == 9 && g_hookB == 0 && TextOf(edS) == "hello");
    }

    std::printf("[7] position on a non-slider -> bad-payload, and state was not applied either\n");
    {
        g_runs = 0;
        const bool before = cbC->Checked;
        Ev e;
        Send(&e, kTag, Val("cbC", "click", "\"position\":3,\"state\":{\"edS\":{\"text\":\"zzz\"}}"));
        CHECK(ErrHas(e, "bad-payload: position is only for a TTrackBar / TScrollBar / TUpDown proxy; cbC is not one"));
        CHECK(g_runs == 0 && cbC->Checked == before && TextOf(edS) == "hello");
        Ev ok;   // 舊請求照舊
        Send(&ok, kTag, Val("cbC", "click", "\"checked\":true"));
        CHECK(ok.ok && g_runs == 1 && cbC->Checked);
    }

    std::printf("[8] TUpDown btNext: position refused (the handler steps it); without it the old step still happens\n");
    {
        g_runs = 0;
        Ev e;
        Send(&e, kTag, Val("udB", "btNext", "\"position\":9"));
        CHECK(ErrHas(e, "only taken with the control's own OnChange (event \"change\"), not \"btNext\""));
        CHECK(g_runs == 0 && (int)udB->Position == 5);
        Ev ok;
        Send(&ok, kTag, Val("udB", "btNext", ""));
        CHECK(ok.ok && g_runs == 1 && (int)udB->Position == 6 && TextOf(edB) == "6");
        const cJSON* p = Changed(ok, "udB", "position");
        const cJSON* t = Changed(ok, "edB", "text");
        CHECK(p && cJSON_IsNumber(p) && p->valueint == 6 && t && cJSON_IsString(t) && std::strcmp(t->valuestring, "6") == 0);
    }

    std::printf("[9] activePageIndex: set first, then OnChange; validated like editlist.save's value kind\n");
    {
        g_runs = 0;
        Ev e;
        Send(&e, kTag, Val("pcX", "change", "\"activePageIndex\":2"));
        CHECK(e.ok && g_runs == 1 && g_seenTab == 2 && pcX->ActivePageIndex == 2);
        CHECK(Changed(e, "pcX", nullptr) == nullptr);
        Ev out;
        Send(&out, kTag, Val("pcX", "change", "\"activePageIndex\":3,\"state\":{\"edS\":{\"text\":\"nope\"}}"));
        CHECK(ErrHas(out, "activePageIndex 3 of pcX: outside 0..2") && ErrHas(out, "lower bound") && ErrHas(out, "reload the page"));
        CHECK(pcX->ActivePageIndex == 2 && TextOf(edS) == "hello" && g_runs == 1);
        Ev neg;
        Send(&neg, kTag, Val("pcX", "change", "\"activePageIndex\":-1"));
        CHECK(ErrHas(neg, "outside 0..2") && pcX->ActivePageIndex == 2);
        Ev click;
        Send(&click, kTag, Val("pcX", "click", "\"activePageIndex\":1"));
        CHECK(ErrHas(click, "only taken with the TPageControl's own OnChange (event \"change\"), not \"click\"") && pcX->ActivePageIndex == 2);
        Ev onTb;
        Send(&onTb, kTag, Val("tbA", "change", "\"activePageIndex\":1"));
        CHECK(ErrHas(onTb, "activePageIndex is only for a TPageControl proxy; tbA is not one") && g_runs == 1);
        Ev old;   // 沒帶＝舊行為
        Send(&old, kTag, Val("pcX", "change", ""));
        CHECK(old.ok && g_runs == 2 && g_seenTab == 2 && pcX->ActivePageIndex == 2);
        // ELSetPageOrder 登記的精確範圍（pcY：2 頁）
        Ev y1;
        Send(&y1, kTag, Val("pcY", "change", "\"activePageIndex\":1"));
        CHECK(y1.ok && g_seenTab == 1 && pcY->ActivePageIndex == 1);
        Ev y2;
        Send(&y2, kTag, Val("pcY", "change", "\"activePageIndex\":2"));
        CHECK(ErrHas(y2, "outside 0..1 (golden DFM tab order)") && pcY->ActivePageIndex == 1);
        // 處理器自己改回分頁 ⇒ changed 帶 activePageIndex
        Ev w;
        Send(&w, kTag, Val("pcW", "change", "\"activePageIndex\":1"));
        const cJSON* a = Changed(w, "pcW", "activePageIndex");
        CHECK(w.ok && g_seenTab == 1 && pcW->ActivePageIndex == 0 && a && cJSON_IsNumber(a) && a->valueint == 0);
    }

    std::printf("[10] cannot be operated (disabled self / disabled container): still refused at step 2\n");
    {
        g_runs = 0;
        Ev off;
        Send(&off, kTag, Val("tbOff", "change", "\"position\":50"));
        CHECK(ErrHas(off, "tbOff cannot be operated now") && (int)tbOff->Position == 1 && g_runs == 0);
        Ev z;
        Send(&z, kTag, Val("pcZ", "change", "\"activePageIndex\":0"));
        CHECK(ErrHas(z, "pcZ cannot be operated now") && g_runs == 0 && pcZ->ActivePageIndex == 0);
    }

    std::printf("[11] A shape: the new keys are refused, RunEvent is not called; without them A dispatch is unchanged\n");
    {
        g_aRuns = 0;
        Ev a;
        Send(&a, "Setup.FakeA", "{\"form\":\"TfFakeA\",\"control\":\"cbFake\",\"event\":\"change\",\"position\":3}");
        CHECK(ErrHas(a, "bad-payload: position is only read by the C route") && g_aRuns == 0);
        Ev b;
        Send(&b, "Setup.FakeA", "{\"form\":\"TfFakeA\",\"control\":\"cbFake\",\"event\":\"change\",\"activePageIndex\":0}");
        CHECK(ErrHas(b, "bad-payload: activePageIndex is only read by the C route") && g_aRuns == 0);
        Ev ok;
        Send(&ok, "Setup.FakeA", "{\"form\":\"TfFakeA\",\"control\":\"cbFake\",\"event\":\"change\",\"itemIndex\":0}");
        const cJSON* route = ok.ack ? cJSON_GetObjectItemCaseSensitive(ok.ack, "route") : nullptr;
        CHECK(ok.ok && g_aRuns == 1 && route && cJSON_IsString(route) && std::strcmp(route->valuestring, "A") == 0);
    }

    std::printf("[12] ELPageIndexRefused: same rule, does not touch the proxy\n");
    {
        CHECK(filerw::ELPageIndexRefused(kF, "pcX", 1).empty() && pcX->ActivePageIndex == 2);
        CHECK(filerw::ELPageIndexRefused(kF, "pcX", 3).find("outside 0..2") != std::string::npos);
        CHECK(filerw::ELPageIndexRefused(kF, "tbA", 0).find("is not a TPageControl proxy") != std::string::npos);
        CHECK(filerw::ELPageIndexRefused(kF, "pcZ", 0).find("disabled or hidden") != std::string::npos);
    }

    std::printf("[13] running (SystemStart) is still checked before the value\n");
    {
        g_runs = 0;
        SystemStart = true;
        Ev e;
        Send(&e, kTag, Val("tbA", "change", "\"position\":10"));
        SystemStart = false;
        CHECK(ErrHas(e, "running: ") && g_runs == 0 && (int)tbA->Position == 20);
    }

    std::printf("[14] state never changes a control that has its own event row (no handler would run): dropped, server keeps its value\n");
    {
        g_fixHasIC = true;   // Fix 盤上有 IC：golden rgFixTrayModeClick 會改回
        g_fixMode = 0;
        g_runs = 0; g_fixClicks = 0;
        const bool cbBefore = cbC->Checked;
        const int udBefore = udB->Position, pcBefore = pcX->ActivePageIndex;
        Ev e;
        Send(&e, kTag, Val("tbA", "change",
                           std::string("\"position\":21,\"state\":{\"rgFix\":{\"itemIndex\":1},\"cbC\":{\"checked\":") +
                           (cbBefore ? "false" : "true") + "},\"udB\":{\"position\":12},\"pcX\":{\"activePageIndex\":0},"
                           "\"tbB\":{\"position\":3},\"edS\":{\"text\":\"s14\"}}"));
        CHECK(e.ok && g_runs == 1 && g_fixClicks == 0 && g_seenPos == 21);                 // 只有 tbA 的處理器跑了
        CHECK(rgFix->ItemIndex == 0 && cbC->Checked == cbBefore && (int)udB->Position == udBefore && pcX->ActivePageIndex == pcBefore);
        CHECK((int)tbB->Position == 3 && TextOf(edS) == "s14");                             // 沒有事件列的照套
        CHECK(TodoCount(e) == 4 && TodoHas(e, "state.rgFix ignored: rgFix has its own golden event (fake.cpp:19") &&
              TodoHas(e, "state.cbC ignored") && TodoHas(e, "state.udB ignored") && TodoHas(e, "state.pcX ignored") &&
              TodoHas(e, "send form.event for it first"));
        CHECK(Changed(e, "rgFix", nullptr) == nullptr);
        Ev same;   // 跟伺服器值相同：丟了沒有影響，不記
        Send(&same, kTag, Val("tbA", "change", std::string("\"state\":{\"rgFix\":{\"itemIndex\":0},\"cbC\":{\"checked\":") +
                                                   (cbBefore ? "true" : "false") + "}}"));
        CHECK(same.ok && TodoCount(same) == 0 && rgFix->ItemIndex == 0);
        Ev bad;    // 事件控制項的 state 型別不對：只是丟、記一筆（不整批拒）
        Send(&bad, kTag, Val("tbA", "change", "\"state\":{\"rgFix\":\"x\"}"));
        CHECK(bad.ok && TodoCount(bad) == 1 && TodoHas(bad, "state.rgFix ignored"));
        Ev bad2;   // 沒有事件列的型別不對：照舊整批拒
        Send(&bad2, kTag, Val("tbA", "change", "\"state\":{\"edS\":{\"text\":5}}"));
        CHECK(ErrHas(bad2, "bad-payload: state: refused: wrong or missing value type for: edS.text"));
        // 正路：送它自己的事件 ⇒ golden 處理器照跑、互鎖改回
        g_fixClicks = 0;
        Ev own;
        Send(&own, kTag, Val("rgFix", "click", "\"itemIndex\":1"));
        const cJSON* ch = Changed(own, "rgFix", "itemIndex");
        CHECK(own.ok && g_fixClicks == 1 && rgFix->ItemIndex == 0 && ch && cJSON_IsNumber(ch) && ch->valueint == 0);
        g_fixHasIC = false;
        Ev own2;
        Send(&own2, kTag, Val("rgFix", "click", "\"itemIndex\":1"));
        CHECK(own2.ok && rgFix->ItemIndex == 1 && g_fixMode == 1);
        g_fixHasIC = false;
        g_fixMode = 0;
        rgFix->ItemIndex = 0;
    }

    std::printf("[15] TCustomEdit: a typable edit with only a click row keeps state; a change row drops it; ReadOnly drops silently\n");
    {
        edR->Text = "r0";
        Ev e;
        Send(&e, kTag, Val("tbA", "change", "\"state\":{\"edK\":{\"text\":\"k15\"},\"edC\":{\"text\":\"c15\"},\"edR\":{\"text\":\"r15\"}}"));
        CHECK(e.ok && TextOf(edK) == "k15" && TextOf(edC) == "" && TextOf(edR) == "r0");
        CHECK(TodoCount(e) == 1 && TodoHas(e, "state.edC ignored"));
    }

    std::printf("[16] editlist.save: with the event state dropped, BeforeApply sees page != server and replays the interlock\n");
    {
        g_fixHasIC = true;
        g_fixMode = 0;
        rgFix->ItemIndex = 0;
        Ev e;   // 頁面把「改 Fix 模式」塞進別的事件的 state（被丟）
        Send(&e, kTag, Val("tbA", "change", "\"state\":{\"rgFix\":{\"itemIndex\":1}}"));
        CHECK(e.ok && rgFix->ItemIndex == 0);
        g_fixClicks = 0;
        std::string ack, err;
        const int st = filerw::PageSave(kPage, "{\"rgFix\":{\"itemIndex\":1}}", "", &ack, &err);
        cJSON* j = cJSON_Parse(ack.c_str());
        const cJSON* evs = j ? cJSON_GetObjectItemCaseSensitive(j, "events") : nullptr;
        const cJSON* ev0 = evs && cJSON_IsArray(evs) ? cJSON_GetArrayItem(evs, 0) : nullptr;
        CHECK(st == 200 && g_fixClicks == 1 && rgFix->ItemIndex == 0 &&                  // 重播了、互鎖改回
              ev0 && cJSON_IsString(ev0) && std::strcmp(ev0->valuestring, "rgFix") == 0);
        if (j) cJSON_Delete(j);
        // 對照組（這一次修的洞）：替身若被「不跑處理器」改成 1（修之前第 4 步照套 state 就是這樣），存檔 BeforeApply 看到頁面值＝
        //   伺服器值 ⇒ 不重播、互鎖沒查，Fix 盤有 IC 還是存成 1
        rgFix->ItemIndex = 1;
        g_fixClicks = 0;
        std::string ack2, err2;
        CHECK(filerw::PageSave(kPage, "{\"rgFix\":{\"itemIndex\":1}}", "", &ack2, &err2) == 200 && g_fixClicks == 0 &&
              rgFix->ItemIndex == 1);
        g_fixHasIC = false;
        rgFix->ItemIndex = 0;
    }

    // AI(W906-EVB10A) 20260929 [W906]：事件批次 B10 part a（SU-9／TS-10 的 Exit 鈕走 form.event）—— ack.closed 與 filerw::PageShownNow
    std::printf("[17] ack.closed: golden Close() (\"closed\") -> closed:true and the page is no longer shown; guard return -> closed:false, still shown\n");
    {
        EL<TButton>(kF, "btExit");
        std::string page;
        CHECK(filerw::PageJson(kPage, &page) == 200 && filerw::PageShownNow(kTag));
        CHECK(!filerw::PageShownNow("NoSuchTag"));
        g_exitGuard = true;
        Ev stay;
        Send(&stay, kTag, Val("btExit", "click", ""));
        const cJSON* c1 = stay.ack ? cJSON_GetObjectItemCaseSensitive(stay.ack, "closed") : nullptr;
        const cJSON* m1 = stay.ack ? cJSON_GetObjectItemCaseSensitive(stay.ack, "messages") : nullptr;
        CHECK(stay.ok && g_exitRuns == 1 && cJSON_IsFalse(c1) && m1 && cJSON_GetArraySize(m1) == 1 && filerw::PageShownNow(kTag));
        g_exitGuard = false;
        Ev ex;
        Send(&ex, kTag, Val("btExit", "click", ""));
        const cJSON* c2 = ex.ack ? cJSON_GetObjectItemCaseSensitive(ex.ack, "closed") : nullptr;
        CHECK(ex.ok && g_exitRuns == 2 && cJSON_IsTrue(c2) && !filerw::PageShownNow(kTag));
        Ev after;   // golden：關掉的表單沒有事件 ⇒ 要先重新開頁
        Send(&after, kTag, Val("btExit", "click", ""));
        CHECK(ErrHas(after, "reload page") && g_exitRuns == 2);
        Ev other;   // 其他事件的 ack 也帶 closed:false
        CHECK(filerw::PageJson(kPage, &page) == 200 && filerw::PageShownNow(kTag));
        Send(&other, kTag, Val("cbC", "click", "\"checked\":true"));
        const cJSON* c3 = other.ack ? cJSON_GetObjectItemCaseSensitive(other.ack, "closed") : nullptr;
        CHECK(other.ok && cJSON_IsFalse(c3) && filerw::PageShownNow(kTag));
    }

    // AI(W906-EVB10C-BC) 20260930 [W906]：B10c 後續 —— BarCode 的 Exit 鈕（golden TfBarCode::sbtExitClick，V912 BarCode\BarCode.cpp:2408-2417）
    //   跟關窗邊緣（FileRW/TestIF_File_BarCode.cpp FileRW_BarCode_WindowEdge 問 filerw::PageCloseEdgeRefused）的交接：Exit 跑過 golden FormClose
    //   ⇒ 頁面關視窗時的關窗邊緣不再跑第二次；✕（沒按 Exit）⇒ 關窗邊緣照跑；關窗邊緣清掉之後下一次開窗重新算。
    std::printf("[18] BarCode-shaped Exit (no guard): closed:true, FormClose once, both 2DID-check buttons re-enabled in changed, globals set; "
                "the window-close edge then refuses (FormClose already ran); after the edge clears it, a fresh open lets the X path run again\n");
    {
        void W906_EditPageWindowClosed(const char* goldenForm);   // FileRW/_EditPage.cpp:1111（頁面表的關窗邊緣呼叫它；本測試直接叫）
        EL<TSpeedButton>(kF, "btBcExit");
        TSpeedButton* chk1 = EL<TSpeedButton>(kF, "btChk1");
        TSpeedButton* chk2 = EL<TSpeedButton>(kF, "btChk2");
        static const char* const kBcPairs[][2] = {{"btBcExit", "pnlMain"}, {"btChk1", "pnlMain"}, {"btChk2", "pnlMain"}};
        filerw::ELSetParents(kF, kBcPairs, 3);
        std::string page;
        W906_EditPageWindowClosed("fEvB1Test");                  // [17] 的 Exit 之後頁面關了視窗：關窗邊緣清掉「FormClose 跑過」（wb_serve 裡由頁面表做）
        CHECK(filerw::PageJson(kPage, &page) == 200 && filerw::PageShownNow(kTag));
        CHECK(filerw::PageCloseEdgeRefused(kTag) == nullptr);    // 開著、還沒 FormClose：✕ 的關窗邊緣會跑 FormClose
        chk1->Enabled = false;                                   // golden btStart2DIDCheckSh1Click（:6828）按下去會設 false
        chk2->Enabled = false;
        g_bcFormClose = 0; g_bcTask1 = 0; g_bcTask2 = 0; g_bcShow = true;
        Ev ex;
        Send(&ex, kTag, Val("btBcExit", "click", ""));
        const cJSON* cl = ex.ack ? cJSON_GetObjectItemCaseSensitive(ex.ack, "closed") : nullptr;
        CHECK(ex.ok && cJSON_IsTrue(cl));
        CHECK(g_bcFormClose == 1 && !g_bcShow && g_bcTask1 == 1 && g_bcTask2 == 1);
        CHECK(chk1->Enabled && chk2->Enabled);
        const cJSON* e1 = Changed(ex, "btChk1", "enabled");
        const cJSON* e2 = Changed(ex, "btChk2", "enabled");
        CHECK(e1 && cJSON_IsTrue(e1) && e2 && cJSON_IsTrue(e2));
        CHECK(!filerw::PageShownNow(kTag) && filerw::PageFormCloseRanNow(kTag));
        const char* why = filerw::PageCloseEdgeRefused(kTag);   // 頁面接著關視窗 ⇒ 關窗邊緣問：FormClose 已經跑過 ⇒ 不跑第二次
        CHECK(why != nullptr && std::strstr(why, "already ran") != nullptr);
        Ev again;                                                // 關掉的表單沒有事件（golden）：連點第二下
        Send(&again, kTag, Val("btBcExit", "click", ""));
        CHECK(ErrHas(again, "reload page") && g_bcFormClose == 1);
        W906_EditPageWindowClosed("fEvB1Test");                  // 關窗邊緣跑完清掉（"T"+物件名＝kF）
        CHECK(!filerw::PageFormCloseRanNow(kTag));
        CHECK(filerw::PageJson(kPage, &page) == 200 && filerw::PageCloseEdgeRefused(kTag) == nullptr);   // 下一次開窗：✕ 又會跑 FormClose
        W906_EditPageWindowClosed("fEvB1Test");
        CHECK(!filerw::PageShownNow(kTag) && filerw::PageCloseEdgeRefused(kTag) != nullptr);             // 沒開過：不跑（golden 表單沒開）
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
