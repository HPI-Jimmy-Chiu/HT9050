// =============================================================================
//  test_evd013_barcode.cpp -- todo D-013 R126（網頁刷條碼框）／R128（form.event 期間的 value 原文）
//
//  AI(W906-D013) 20260929 [W906]  Steven 20260929「照 BCB 的邏輯」；decisions-decided R126／R128「20260929 結果」。
//  受測：FileRW/_FormEvent.cpp（value 的 "barcode"、formevent::BarcodeBoxOpen／BarcodeBoxClosed、ack 的 "barcode"、CurrentValueJson）、
//    BarcodeReader.cpp（Barcode_Reader、InputBarcodeNumber 裡的 W906_BarcodeBoxModal：golden 的 FormShow → TimerKeyIn → btnEnterClick →
//    FormClose 跑刷到的字），連同 FileRW/_EditPage.cpp RunPageEvent（C 路）與 A 形狀分派（JsonBridge 替身）。
//    [1] 不是 KYEC：golden Barcode_Reader 回 2 ⇒ 處理器照跑、框沒開、ack 沒有 "barcode"
//    [2] KYEC、頁面沒帶 barcode：框開了、照「沒刷就關」（FormClose 跑了）⇒ 回 0、處理器 return（不歸零）；ack.barcode 一筆
//        {caption:"Input Operator ID:", inputType:"", scanned:false, accepted:false}；沒有 RecordProcess、ReEnterBarcode 沒動
//    [3] 帶 barcode "1000123"：golden 的框收下 ⇒ 回 1、歸零；RecordProcess「ID:1000123 Login --- Change  Auto Clean」、
//        ReEnterBarcode[bcAutoClean]=true、asSecsGemBarCode、EventReport(SECS_EVENT.BarcodeReaderEnter)；ack accepted:true
//    [4] ReEnterBarcode 已是 true ⇒ golden 不再問（回 2），帶不帶 barcode 都照跑
//    [5] 不到 4 個字 ⇒ btnEnterClick 不關框 ⇒ 關掉＝空字串 ⇒ 不歸零；ack scanned:true accepted:false
//    [6] KYEC 條碼格式（TimerKeyIn，SHIP 組態才開；SOFT_SIMULTE 組態 golden FormShow 關掉計時器 ⇒ 只剩 [5] 的長度檢查）：
//        7 碼前 3 碼 123／6 碼前 2 碼 84 ⇒ SHIP 清掉＝不收、SIM 收；850123、1200000 兩種都收；KLT（bEnable_KLT_Function）3..31；
//        InitialOK=false 時 golden 計時器第一行 return ⇒ SHIP 也不檢查
//    [7] barcode 不是字串 ⇒ bad-payload，處理器沒跑、框沒開
//    [8] 一個處理器開兩個框：刷到的字只給第一個（golden 一個框刷一次），第二個照沒刷關 ⇒ ack.barcode 兩筆
//    [9] 不在 form.event 裡（開頁、存檔、主迴圈）：框照移植樹原本的離線空殼（FormShow／FormClose 都沒跑）——掛勾有裝、沒裝都一樣
//    [10] CurrentValueJson：處理器期間＝頁面送的 value 原文（R128 的 "noPart" 就是這樣讀），事件外是空字串
//    [11] A 形狀（formbridge）一樣收 barcode：處理器叫 InputBarcodeNumber 拿到刷到的字
//  組態：本檔編兩次 —— test_evd013_barcode（SIM，SOFT_SIMULTE）與 test_evd013_barcode_ship（-DW906_NO_SOFT_SIMULTE），[6] 依組態驗。
//  god-stack 全域、RecordProcess／EventReport、JsonBridge 的 FindBridge／RunEvent／FormLock 由本檔給替身（同 test_formevent_position.cpp）；
//  myTimer.cpp 直接連（TQPF_Timer，BarcodeReader 的 HDelayTime）。不 link god-stack、不讀寫任何檔，秒級。
// =============================================================================
#include "FileRW/_FormEvent.h"
#include "FileRW/_FormEventCtx.h"
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
#include "MachineType.h"
#include "BarcodeReader.h"
#include "SECSGEM/SecsEventType.h"
#include "forms/fSecurity.h"

// ---- god-stack 替身（型別照 cmydef.h／cprod.h／Config.h／CosFunction.h／forms/fSecurity.h 的宣告；同 test_formevent_position.cpp）----
int  AccessLevel = 0;              // cmydef.h:3506
bool SystemStart = false;          // cmydef.h:221
bool SoftStart = false;            // cmydef.h:223
int  iHome = 0;                    // cmydef.h:260
int  WEIGHT_CALIBRATION = 0;       // cmydef.h:2833
int  CUSTOMER_CODE = 0;            // cmydef.h:3184
int  iDefHonPrecLevel = 3;         // cmydef.h:3591
bool bResetMNet = false;           // Motor/myMN200motor.h:198
int  iMaxLevelItem = 0;            // cSecurity.cpp:47
bool authMainForm[12];             // cAuthority.h:56（_EditPage.cpp 自己宣告，開窗閘 GAuth 用；AI(W906-AUTHMAINFORM) 20260930）——本測試不經開窗閘
LAST_LEVEL_SET LevelSet;           // cprod.h:1154
HT9045_CONFIG IniConfig;           // Config.h:1516
HT9045_COUSTOMER_FUNCTION CosFunction;   // CosFunction.h:503
TfSecurity* fSecurity = nullptr;   // forms/fSecurity.h:609
bool TfSecurity::Insufficient(int, bool) { return true; }
void HTEditList_RegisterControlName(TControl*, const AnsiString&) {}
void NewRecordProcess(AnsiString, AnsiString, AnsiString) {}
bool W906_FormShowing(const char*, bool member) { return member; }   // csystem.cpp:30049 的替身＝hook 是 0 時的答案；FileRW/_FormEvent.cpp 運轉中例外表要它（AI(W906-FE-RUNEXC) 20260930；本測試的頁不在表上）
// ---- BarcodeReader.cpp 讀寫的全域（cmydef.h:220／:2974／:3553／:3556／:3767／:3854／:4531／:4995／:5569-5570、cprod.h:2581）----
bool InitialOK = false;
int  USE_BARCODE_AS_KEYBOARD = 0;
bool bBarcodeReader = false;
bool ReEnterBarcode[20];
AnsiString asSecsGemBarCode;
bool bOffsetEnterBarcode = false;
bool bEnable_KLT_Function = false;
int  SPIL_FOR_QLE = 0;
bool bBarCoderAutoLogin = false;
bool bBarCoderSetupFile = false;
SYSTEM_TEST_IF TestIF_File;
static std::vector<std::string> g_records;
void RecordProcess(AnsiString s, AnsiString) { g_records.push_back(s.c_str()); }   // cMyDB.h:130
static int g_ceidN = 0;
static unsigned g_ceid = 0;
void EventReport(unsigned Ceid) { ++g_ceidN; g_ceid = Ceid; }                      // SECSGEM/SecsEventReport.h

// BarcodeReader.cpp 檔尾的掛勾（wb_serve 開機由 FileRW/TestIF_File_Cleaning.cpp FileRW_Cleaning_D013InstallBarcodeBox 裝）
extern bool (*g_W906_BarcodeBoxOpenHook)(const char* caption, const char* inputType, bool* haveScan, std::string* scan);
extern void (*g_W906_BarcodeBoxClosedHook)(const char* result);
static void Install()
{
    g_W906_BarcodeBoxOpenHook = &formevent::BarcodeBoxOpen;
    g_W906_BarcodeBoxClosedHook = &formevent::BarcodeBoxClosed;
}

// ---- JsonBridge 替身：A 形狀只有一個假頁 "Setup.FakeA"，RunEvent 叫 golden 的框（[11]）----
static int g_aRuns = 0;
static std::string g_aGot = "?";
namespace ht9045 {
namespace formbridge {
static const BridgeDesc kFakeA = {"Setup.FakeA.html", "TfFakeA", "fakeA.cpp", nullptr, nullptr, nullptr, "", "", nullptr, 0, "cbFake"};
const BridgeDesc* FindBridge(const std::string& page) { return page == "Setup.FakeA.html" ? &kFakeA : nullptr; }
bool RunEvent(const BridgeDesc&, const formevent::Request&, formevent::Result* out)
{
    ++g_aRuns;
    g_aGot = InputBarcodeNumber("Input Operator ID:").c_str();
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

static const char* const kF = "TfD013Test";   // 假的 golden 表單類別
static const char* const kTag = "D013Test";   // 假的 C 路結構名（WS form.event 的 tag）

static void FakeFormShow() {}
static void FakeSaveFlow() {}
static void FakeReload() {}
static bool FakeBooted() { return true; }
static const filerw::PageDesc kPage = {
    kTag, kF, "Setup.D013Test.html", nullptr, nullptr, 0, nullptr, 0,
    FakeFormShow, FakeSaveFlow, "SaveSetupFile", FakeReload, FakeBooted, nullptr, nullptr};

// ---- 處理器：照 golden btnResetCleanCountClick（V912 AutoClean\uCleaning.cpp:2116-2121）的第一行 ----
static int g_runs = 0, g_zeroed = 0, g_two = 0;
static std::string g_seenValue = "?";
static void H_reset(TControl*)
{
    ++g_runs;
    if(Barcode_Reader(bcAutoClean)==0)                                          // 20140103 wei KYEC Barcode Reader（golden :2118）
    {
        return;
    }
    ++g_zeroed;                                                                 // golden :2123 起：ReadWriteAutoCleanCount(false, true)…
}
static void H_two(TControl*)                                                    // [8] 同一個處理器開兩個框
{
    ++g_runs;
    if (Barcode_Reader(bcAutoClean) == 0) { g_two = -1; return; }
    if (Barcode_Reader(bcOffset) == 0) { g_two = -2; return; }
    g_two = 1;
}
static void H_value(TControl*) { ++g_runs; g_seenValue = formevent::CurrentValueJson(); }   // [10]
static const filerw::PageEvent kEvents[] = {
    {"btReset", "click", "fake.cpp:1 TfD013Test::btResetClick", &H_reset},
    {"btTwo", "click", "fake.cpp:2 TfD013Test::btTwoClick", &H_two},
    {"btValue", "click", "fake.cpp:3 TfD013Test::btValueClick", &H_value},
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
static std::string Val(const char* form, const char* control, const std::string& extra)
{
    return std::string("{\"form\":\"") + form + "\",\"control\":\"" + control + "\",\"event\":\"click\"" +
           (extra.empty() ? "" : "," + extra) + "}";
}
static std::string Scan(const char* s) { return std::string("\"barcode\":\"") + s + "\""; }
static const cJSON* Box(const Ev& e, int i)                                     // ack.barcode[i]（沒有＝nullptr）
{
    const cJSON* b = e.ack ? cJSON_GetObjectItemCaseSensitive(e.ack, "barcode") : nullptr;
    return b && cJSON_IsArray(b) ? cJSON_GetArrayItem(b, i) : nullptr;
}
static int Boxes(const Ev& e)
{
    const cJSON* b = e.ack ? cJSON_GetObjectItemCaseSensitive(e.ack, "barcode") : nullptr;
    return b ? (cJSON_IsArray(b) ? cJSON_GetArraySize(b) : -1) : 0;
}
static bool BoxIs(const Ev& e, int i, bool scanned, bool accepted)
{
    const cJSON* b = Box(e, i);
    const cJSON* s = b ? cJSON_GetObjectItemCaseSensitive(b, "scanned") : nullptr;
    const cJSON* a = b ? cJSON_GetObjectItemCaseSensitive(b, "accepted") : nullptr;
    return s && a && cJSON_IsBool(s) && cJSON_IsBool(a) && (cJSON_IsTrue(s) != 0) == scanned && (cJSON_IsTrue(a) != 0) == accepted;
}
static std::string BoxStr(const Ev& e, int i, const char* key)
{
    const cJSON* b = Box(e, i);
    const cJSON* v = b ? cJSON_GetObjectItemCaseSensitive(b, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : "?";
}
static void KyecReady()                                                         // golden Barcode_Reader 會開框的條件（BarcodeReader.cpp:449-450）
{
    CUSTOMER_CODE = CC_KYEC_LEE;
    USE_BARCODE_AS_KEYBOARD = 1;
    bBarcodeReader = true;
    for (bool& b : ReEnterBarcode) b = false;
    bOffsetEnterBarcode = false;
}
// 帶一個 barcode 按一次 Reset：golden 收下（歸零）了嗎
static bool ResetWith(const char* scan)
{
    for (bool& b : ReEnterBarcode) b = false;
    const int z = g_zeroed;
    Ev e;
    Send(&e, kTag, Val(kF, "btReset", Scan(scan)));
    const bool took = e.ok && g_zeroed == z + 1;
    std::printf("       scan \"%s\" -> %s\n", scan, took ? "accepted" : "refused");
    return took && BoxIs(e, 0, true, true);
}

int main()
{
    // ---- 替身：pnlMain ⊃ {btReset, btTwo, btValue}；golden 的框（FormsBootstrap.cpp:203）
    EL<TPanel>(kF, "pnlMain");
    EL<TButton>(kF, "btReset");
    EL<TButton>(kF, "btTwo");
    EL<TButton>(kF, "btValue");
    static const char* const kPairs[][2] = {{"btReset", "pnlMain"}, {"btTwo", "pnlMain"}, {"btValue", "pnlMain"}};
    filerw::ELSetParents(kF, kPairs, (int)(sizeof(kPairs) / sizeof(kPairs[0])));
    filerw::RegisterPage(&kPage);
    filerw::RegisterPageEvents(kTag, kEvents, (int)(sizeof(kEvents) / sizeof(kEvents[0])));
    FormBarcodeReader = new TFormBarcodeReader();
    InitialOK = true;
    Install();
    {
        std::string json;
        CHECK(filerw::PageJson(kPage, &json) == 200);                           // 「開過頁」（RunPageEvent 第 1 步）
    }
#ifdef SOFT_SIMULTE
    std::printf("[config] SIM (SOFT_SIMULTE defined: golden FormShow keeps TimerKeyIn off)\n");
#else
    std::printf("[config] SHIP (SOFT_SIMULTE undefined: TimerKeyIn checks the KYEC code format)\n");
#endif

    std::printf("[1] not KYEC: golden Barcode_Reader returns 2, no box\n");
    {
        CUSTOMER_CODE = 0;
        Ev e;
        Send(&e, kTag, Val(kF, "btReset", ""));
        CHECK(e.ok && g_zeroed == 1 && Boxes(e) == 0 && g_records.empty());
        CHECK(!FormBarcodeReader->bShow);
    }

    std::printf("[2] KYEC, nothing scanned yet: the box closes empty, golden returns, ack.barcode asks\n");
    {
        KyecReady();
        FormBarcodeReader->sBarcodeInfo = "stale";
        FormBarcodeReader->edtBarcodeNumber->Text = "stale";
        Ev e;
        Send(&e, kTag, Val(kF, "btReset", ""));
        CHECK(e.ok && g_zeroed == 1 && Boxes(e) == 1 && BoxIs(e, 0, false, false));
        CHECK(BoxStr(e, 0, "caption") == "Input Operator ID:" && BoxStr(e, 0, "inputType") == "");
        CHECK(g_records.empty() && !ReEnterBarcode[bcAutoClean] && g_ceidN == 0);
        CHECK(!FormBarcodeReader->bShow && FormBarcodeReader->edtBarcodeNumber->Text == "");   // golden FormShow ran (clears the edit)
        CHECK(FormBarcodeReader->sBarcodeInfo == "" && FormBarcodeReader->_sInputType == "");   // golden FormClose ran (KEY_NONE)
    }

    std::printf("[3] KYEC, the page sends the scan: golden takes it, zeroes, records the login\n");
    {
        Ev e;
        Send(&e, kTag, Val(kF, "btReset", Scan("1000123")));
        CHECK(e.ok && g_zeroed == 2 && Boxes(e) == 1 && BoxIs(e, 0, true, true));
        CHECK(g_records.size() == 1 && g_records.back() == "ID:1000123 Login --- Change  Auto Clean");
        CHECK(ReEnterBarcode[bcAutoClean] && !bOffsetEnterBarcode);
        CHECK(asSecsGemBarCode == "1000123" && g_ceidN == 1 && g_ceid == (unsigned)SECS_EVENT.BarcodeReaderEnter);   // golden FormClose :100-102
        CHECK(!FormBarcodeReader->bShow && FormBarcodeReader->sBarcodeInfo == "1000123");
    }

    std::printf("[4] ReEnterBarcode already true: golden does not ask again\n");
    {
        Ev e;
        Send(&e, kTag, Val(kF, "btReset", ""));
        CHECK(e.ok && g_zeroed == 3 && Boxes(e) == 0);
        Ev e2;
        Send(&e2, kTag, Val(kF, "btReset", Scan("1000123")));                   // 帶了也用不到（沒有框）
        CHECK(e2.ok && g_zeroed == 4 && Boxes(e2) == 0 && g_records.size() == 1);
    }

    std::printf("[5] fewer than 4 characters: btnEnterClick keeps the box open, closing it = nothing\n");
    {
        ReEnterBarcode[bcAutoClean] = false;
        Ev e;
        Send(&e, kTag, Val(kF, "btReset", Scan("12")));
        CHECK(e.ok && g_zeroed == 4 && Boxes(e) == 1 && BoxIs(e, 0, true, false));
        CHECK(!ReEnterBarcode[bcAutoClean] && g_records.size() == 1 && FormBarcodeReader->sBarcodeInfo == "");
    }

    std::printf("[6] KYEC code format (golden TimerKeyInTimer :353-401)\n");
    {
        CHECK(ResetWith("850123"));                                             // 6 碼、前 2 碼 85：兩種組態都收
        CHECK(ResetWith("1200000"));                                            // 7 碼、前 3 碼 120
#ifdef SOFT_SIMULTE
        CHECK(ResetWith("1234567"));                                            // SIM：計時器關著，只看長度
        CHECK(ResetWith("840123"));
        CHECK(ResetWith("12345"));
#else
        CHECK(!ResetWith("1234567"));                                           // 前 3 碼 123 > 120 ⇒ 清掉
        CHECK(!ResetWith("840123"));                                            // 前 2 碼 84 < 85
        CHECK(!ResetWith("12345"));                                             // 不是 6／7 碼 ⇒ 清掉
        CHECK(!ResetWith("12345678"));
#endif
        bEnable_KLT_Function = true;                                            // KLT：3..31（2003～2031）
        CHECK(ResetWith("0301234"));
#ifdef SOFT_SIMULTE
        CHECK(ResetWith("850123"));
#else
        CHECK(!ResetWith("850123"));                                            // KLT 不收 85
#endif
        bEnable_KLT_Function = false;
        InitialOK = false;                                                      // golden 計時器第一行 `if(InitialOK==false) return;`
        CHECK(ResetWith("1234567"));
        InitialOK = true;
    }

    std::printf("[7] barcode must be a string\n");
    {
        for (bool& b : ReEnterBarcode) b = false;
        const int runs = g_runs;
        Ev e;
        Send(&e, kTag, Val(kF, "btReset", "\"barcode\":123456"));
        CHECK(!e.ok && e.err.find("bad-payload") == 0 && e.err.find("barcode") != std::string::npos && g_runs == runs);
        Ev e2;
        Send(&e2, kTag, Val(kF, "btReset", "\"barcode\":null"));                // null＝沒帶
        CHECK(e2.ok && g_runs == runs + 1 && BoxIs(e2, 0, false, false));
    }

    std::printf("[8] two boxes in one handler: the scan goes to the first box only\n");
    {
        for (bool& b : ReEnterBarcode) b = false;
        bOffsetEnterBarcode = false;
        Ev e;
        Send(&e, kTag, Val(kF, "btTwo", Scan("850123")));
        CHECK(e.ok && g_two == -2 && Boxes(e) == 2 && BoxIs(e, 0, true, true) && BoxIs(e, 1, false, false));
        CHECK(ReEnterBarcode[bcAutoClean] && !bOffsetEnterBarcode);
    }

    std::printf("[9] outside a form.event the box stays the offline shell (no FormShow / FormClose)\n");
    {
        for (bool& b : ReEnterBarcode) b = false;
        Ev pre;                                                                 // 先在網頁框收下一段字（golden FormClose 把它留在 sBarcodeInfo）
        Send(&pre, kTag, Val(kF, "btReset", Scan("850123")));
        CHECK(pre.ok && BoxIs(pre, 0, true, true) && FormBarcodeReader->sBarcodeInfo == "850123");
        FormBarcodeReader->edtBarcodeNumber->Text = "untouched";
        const AnsiString r = InputBarcodeNumber("Input Lot ID:", "LotID");
        CHECK(r == "" && FormBarcodeReader->_sInputType == "LotID" && !FormBarcodeReader->bShow);   // 上一次網頁刷到的字不會漏進來；FormClose 會清 _sInputType
        CHECK(FormBarcodeReader->edtBarcodeNumber->Text == "untouched");        // FormShow 沒跑
        g_W906_BarcodeBoxOpenHook = nullptr;
        g_W906_BarcodeBoxClosedHook = nullptr;
        Ev e;                                                                   // 掛勾沒裝（每一支別的程式）＝移植樹原本：回 0、沒有 ack.barcode
        for (bool& b : ReEnterBarcode) b = false;
        const int z = g_zeroed;
        Send(&e, kTag, Val(kF, "btReset", Scan("850123")));
        CHECK(e.ok && g_zeroed == z && Boxes(e) == 0);
        Install();
    }

    std::printf("[10] CurrentValueJson: the event's value while the handler runs, empty outside\n");
    {
        CHECK(formevent::CurrentValueJson().empty());
        const std::string v = Val(kF, "btValue", "\"noPart\":true");
        Ev e;
        Send(&e, kTag, v);
        CHECK(e.ok && g_seenValue == v && Boxes(e) == 0);
        CHECK(formevent::CurrentValueJson().empty());
    }

    std::printf("[11] A shape: the same box, the handler gets the scanned text\n");
    {
        Ev e;
        Send(&e, "Setup.FakeA", Val("TfFakeA", "cbFake", Scan("850123")));
        const cJSON* route = e.ack ? cJSON_GetObjectItemCaseSensitive(e.ack, "route") : nullptr;
        CHECK(e.ok && g_aRuns == 1 && g_aGot == "850123" && Boxes(e) == 1 && BoxIs(e, 0, true, true));
        CHECK(route && cJSON_IsString(route) && std::strcmp(route->valuestring, "A") == 0);
        Ev e2;
        Send(&e2, "Setup.FakeA", Val("TfFakeA", "cbFake", ""));
        CHECK(e2.ok && g_aRuns == 2 && g_aGot == "" && BoxIs(e2, 0, false, false));
    }

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
