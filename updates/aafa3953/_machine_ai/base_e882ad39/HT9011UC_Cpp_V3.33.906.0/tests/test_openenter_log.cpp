// =============================================================================
//  test_openenter_log.cpp -- C 路開頁記 golden 開窗鈕的 "Enter ..."（RULINGS_20260926 S165＝R101「要記」）
//
//  AI(W906-FRW-S165) 20260927 [W906]  受測：FileRW/_EditPage.cpp FindOpenEnter／OpenEnterRecord（檔尾）與 PageJson 的呼叫點。
//    [1] 表：21 列逐列＝golden V912 main.cpp 的呼叫（開窗鈕、代碼、字樣、行號；下面 kWant 是手抄的期望表，每一行
//        20260927 對過 golden 該行與 D:\HT9045\Error\AlarmCodeList.txt 同代碼的字樣）
//    [2] 每一列的 tag 都登記在 kOpenGates（OpenGateRefused 的理由不是 no-gate）；沒有列的 4 頁也在 kOpenGates
//    [3] golden 開窗鈕不記的 4 頁（ContactForce／TestIF_File_VacuumUnit／AOAOffset／HSys）＋ 不認得的 tag：不記
//    [4] OpenEnterRecord(tag, false) ⇒ NewRecordProcess(code, text, " ") 一次（Debug＝golden 預設 " "，cMyDB.h:62）；
//        (tag, true)＝這一頁已經開著 ⇒ 不記
//    [5] GroundMan ⇒ RecordProcess("Enter Ground Man Form", "")（golden V912 main.cpp:34643 沒有代碼，S2 預設 ""）
//    [6] PageJson：第一次（還沒開過）記、而且記在 golden FormShow 之前（golden 先 NewRecordProcess 再 ShowModal）；
//        第二次（＝存檔後引擎自動重讀，web\page\ht9045_wire_engine.js:1290）不記
//    [7] PageSave 一般存檔（沒有 golden Close()）之後的 PageJson 不記；saveFlow 記了 "closed"（golden Close()）之後的 PageJson 再記
//    [8] 沒開機（booted()==false）⇒ PageJson 409、不記
//  NOT COVERED：三個自己的入口（FileRW/Teach.cpp:256、Offset_File.cpp:353、IniConfig.cpp:493 的呼叫在 god-stack 裡；BinSelect 走
//    PageJson）與 tools/wb_serve.cpp editlist.get 臂「開窗閘先、golden 後」的順序 —— 由整合者用 wb_serve 探針驗（交件列了步驟）。
//  _EditPage.cpp 開窗閘段用到的 god-stack 全域（AccessLevel、SystemStart、fSecurity…）與 golden 的 NewRecordProcess／
//  RecordProcess（cMyDB.h:129-130；移植樹本體 acatchtray_shims.cpp:152／canary_support.cpp:117）由本檔給替身；
//  HTEditList_RegisterControlName（Public/HTEditList.cpp，ht9045_sm）同 test_editlist_pageindex.cpp 給只記名字的替身。
//  不 link god-stack、不讀寫任何檔，秒級。
// =============================================================================
#include "FileRW/_EditPage.h"
#include "FileRW/_EditList.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "forms/fSecurity.h"

// ---- god-stack 替身（型別照 cmydef.h／cprod.h／Config.h／CosFunction.h／forms/fSecurity.h 的宣告）--------------------
int  AccessLevel = 0;              // cmydef.h:3506
bool SystemStart = false;          // cmydef.h:221
int  iHome = 0;                    // cmydef.h:260
int  WEIGHT_CALIBRATION = 0;       // cmydef.h:2833
int  CUSTOMER_CODE = 0;            // cmydef.h:3184
int  iDefHonPrecLevel = 3;         // cmydef.h:3591
bool bResetMNet = false;           // Motor/myMN200motor.h:198（_EditPage.cpp 自己宣告）
int  iMaxLevelItem = 0;            // cSecurity.cpp:47（_EditPage.cpp 自己宣告）
LAST_LEVEL_SET LevelSet;           // cprod.h:1154
HT9045_CONFIG IniConfig;           // Config.h:1516
HT9045_COUSTOMER_FUNCTION CosFunction;   // CosFunction.h:503
TfSecurity* fSecurity = nullptr;   // forms/fSecurity.h:609（nullptr ⇒ 開窗閘的等級一律不夠；[2] 只看「有沒有登記」）
bool TfSecurity::Insufficient(int, bool) { return true; }   // fSecurity 是 nullptr，不會被呼叫；只為了連結

// Public/HTEditList.cpp:214 的替身（同 test_editlist_pageindex.cpp）
void HTEditList_RegisterControlName(TControl*, const AnsiString&) {}

// golden 的兩個入口（cMyDB.h:129-130）：記下每一次呼叫，連同呼叫當下 golden FormShow 已經跑了幾次
struct Rec {
    std::string fn, code, s, dbg;
    int formShowsBefore;
};
static std::vector<Rec> g_rec;
static int g_formShows = 0;
void NewRecordProcess(AnsiString AlarmCode, AnsiString S, AnsiString Debug)
{
    g_rec.push_back(Rec{"NewRecordProcess", AlarmCode.c_str(), S.c_str(), Debug.c_str(), g_formShows});
}
void RecordProcess(AnsiString S, AnsiString S2)
{
    g_rec.push_back(Rec{"RecordProcess", "", S.c_str(), S2.c_str(), g_formShows});
}

static int g_fail = 0, g_pass = 0;
#define CHECK(c) do { if (c) { g_pass++; std::printf("  ok   %s\n", #c); } \
                       else { g_fail++; std::printf("  FAIL %s  (line %d)\n", #c, __LINE__); } } while (0)

// ---- 假的 C 路頁（tag 用真的表列 Ld_UldDelayTime；表單類別是假的，沒有替身）--------------------------------------
static bool g_booted = true;
static bool g_saveCloses = false;   // saveFlow 要不要照 golden Close() 記 "closed"
static void FakeFormShow() { ++g_formShows; }
static void FakeSaveFlow() { if (g_saveCloses) filerw::ELMark("closed"); }
static void FakeReload() {}
static bool FakeBooted() { return g_booted; }
static bool NeverBooted() { return false; }
static const filerw::PageDesc kLd = {
    "Ld_UldDelayTime", "TfEnterTest", "Setup.Ld_ULd.html", nullptr, nullptr, 0, nullptr, 0,
    FakeFormShow, FakeSaveFlow, "SaveSetupFile", FakeReload, FakeBooted, nullptr, nullptr};
static const filerw::PageDesc kOff = {
    "TrayForm", "TfEnterTest2", "Setup.TrayAssignment.html", nullptr, nullptr, 0, nullptr, 0,
    FakeFormShow, FakeSaveFlow, "SaveSetupFile", FakeReload, NeverBooted, nullptr, nullptr};

struct Want {
    const char* tag;
    const char* button;
    const char* code;
    const char* text;
    const char* golden;
};
// golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（cp950）
static const Want kWant[] = {
    {"Ld_UldDelayTime",             "TfMain::sbLdUldClick",           "MES2176",  "Enter Load / Unload",    "main.cpp:28452"},
    {"UserDefForm_File",            "TfMain::sbTrayFormClick",        "MES2173",  "Enter Tray Form",        "main.cpp:28408"},
    {"TrayForm",                    "TfMain::sbTrayAssignClick",      "MES2175",  "Enter Tray Assignment",  "main.cpp:28432"},
    {"BinSelect",                   "TfMain::sbBinClick",             "MES2171",  "Enter Bin",              "main.cpp:28323"},
    {"TestIF_File_TesterIF",        "TfMain::sbTesterClick",          "MES2172",  "Enter Tester I/F",       "main.cpp:28335"},
    {"Temperature",                 "TfMain::sbTempOffsetClick",      "MES21109", "Enter Temp. Offset",     "main.cpp:28351"},
    {"DeviceForm_File",             "TfMain::sbContactClick",         "MES2170",  "Enter Contact",          "main.cpp:28306"},
    {"TestIF_File_SetUp",           "TfMain::sbSetupClick",           "MES2177",  "Enter Set Up",           "main.cpp:28469"},
    {"TestIF_File_YieldMonitoring", "TfMain::sbYieldClick",           "MES2178",  "Enter Yield Monitoring", "main.cpp:28508"},
    {"TestIF_File_Cleaning",        "TfMain::sbAutoCleanClick",       "MES2194",  "Enter Auto Clean Form",  "main.cpp:29667"},
    {"TestIF_File_QAMode",          "TfMain::sbQAModeClick",          "MES21102", "Enter QA Mode Form",     "main.cpp:29924"},
    {"TestIF_File_BarCode",         "TfMain::sbBarCodeClick",         "MES21101", "Enter 2D Bar Code Form", "main.cpp:29915"},
    {"GroundMan",                   "TfMain::spbGroundManClick",      "",         "Enter Ground Man Form",  "main.cpp:34643"},
    {"IniConfig",                   "TfMain::sbConfigurationClick",   "MES2185",  "Enter Configuration",    "main.cpp:28616"},
    {"TTLCfg",                      "TfMain::sbDioSetClick",          "MES2186",  "Enter DIO Form",         "main.cpp:28668"},
    {"StartCondition",              "TfMain::sbStartModeClick",       "MES2180",  "Enter Start Condition",  "main.cpp:28552"},
    {"IniConfig_CounterSel",        "TfMain::sbSeleteClick",          "MES2181",  "Enter Counter Select",   "main.cpp:28564"},
    {"Teach",                       "TfMain::sbTeachingClick",        "MES2189",  "Enter Teach Form",       "main.cpp:28845"},
    {"ShuttleMove",                 "TfMain::sbShuttleMaintainClick", "MES21107", "Enter Shuttle Maintain", "main.cpp:33692"},
    {"ArmSpeed_File",               "TfMain::sbSpeedClick",           "MES2187",  "Enter Speed",            "main.cpp:28694"},
    {"Offset_File",                 "TfMain::sbOffsetClick",          "MES2188",  "Enter Offset",           "main.cpp:28717"},
};
// kOpenGates 裡 golden 開窗鈕不記的頁
static const char* const kNoRow[] = {"ContactForce", "TestIF_File_VacuumUnit", "AOAOffset", "HSys"};

static bool Same(const char* a, const char* b) { return a && b && std::strcmp(a, b) == 0; }

int main()
{
    const int nWant = (int)(sizeof(kWant) / sizeof(kWant[0]));
    const int nNoRow = (int)(sizeof(kNoRow) / sizeof(kNoRow[0]));

    std::printf("[1] 表逐列＝golden\n");
    int rowsOk = 0;
    for (int i = 0; i < nWant; ++i) {
        const filerw::OpenEnter* e = filerw::FindOpenEnter(kWant[i].tag);
        const bool ok = e && Same(e->tag, kWant[i].tag) && Same(e->button, kWant[i].button) && Same(e->code, kWant[i].code) &&
                        Same(e->text, kWant[i].text) && Same(e->golden, kWant[i].golden);
        if (!ok) std::printf("  row mismatch: %s\n", kWant[i].tag);
        rowsOk += ok ? 1 : 0;
    }
    CHECK(rowsOk == 21 && nWant == 21);

    std::printf("[2] 每一個 tag 都登記在 kOpenGates（理由不是 no-gate）\n");
    int gated = 0;
    for (int i = 0; i < nWant + nNoRow; ++i) {
        const std::string tag = i < nWant ? kWant[i].tag : kNoRow[i - nWant];
        std::string why;
        const bool refused = filerw::OpenGateRefused(tag, false, &why);
        const bool registered = !refused || why.compare(0, 7, "no-gate") != 0;
        if (!registered) std::printf("  not in kOpenGates: %s\n", tag.c_str());
        gated += registered ? 1 : 0;
    }
    CHECK(gated == nWant + nNoRow);
    {
        std::string why;
        CHECK(filerw::OpenGateRefused("NoSuchTag", false, &why) && why.compare(0, 7, "no-gate") == 0);   // 對照組
    }

    std::printf("[3] golden 開窗鈕不記的頁、不認得的 tag：不記\n");
    for (int i = 0; i < nNoRow; ++i) {
        CHECK(filerw::FindOpenEnter(kNoRow[i]) == nullptr);
        CHECK(!filerw::OpenEnterRecord(kNoRow[i], false));
    }
    CHECK(filerw::FindOpenEnter("NoSuchTag") == nullptr);
    CHECK(!filerw::OpenEnterRecord("NoSuchTag", false));
    CHECK(g_rec.empty());

    std::printf("[4] OpenEnterRecord：沒開過記一次、已經開著不記\n");
    CHECK(!filerw::OpenEnterRecord("Teach", true));
    CHECK(g_rec.empty());
    CHECK(filerw::OpenEnterRecord("Teach", false));
    CHECK(g_rec.size() == 1);
    if (g_rec.size() == 1) {
        CHECK(g_rec[0].fn == "NewRecordProcess");
        CHECK(g_rec[0].code == "MES2189");
        CHECK(g_rec[0].s == "Enter Teach Form");
        CHECK(g_rec[0].dbg == " ");
    }

    std::printf("[5] GroundMan：golden RecordProcess（沒有代碼）\n");
    g_rec.clear();
    CHECK(filerw::OpenEnterRecord("GroundMan", false));
    CHECK(g_rec.size() == 1);
    if (g_rec.size() == 1) {
        CHECK(g_rec[0].fn == "RecordProcess");
        CHECK(g_rec[0].s == "Enter Ground Man Form");
        CHECK(g_rec[0].dbg.empty());
    }

    std::printf("[6] PageJson：第一次記（在 golden FormShow 之前）、第二次（存檔後重讀）不記\n");
    g_rec.clear();
    std::string json;
    CHECK(filerw::PageJson(kLd, &json) == 200);
    CHECK(g_formShows == 1);
    CHECK(g_rec.size() == 1);
    if (g_rec.size() == 1) {
        CHECK(g_rec[0].fn == "NewRecordProcess");
        CHECK(g_rec[0].code == "MES2176");
        CHECK(g_rec[0].s == "Enter Load / Unload");
        CHECK(g_rec[0].dbg == " ");
        CHECK(g_rec[0].formShowsBefore == 0);   // golden：NewRecordProcess（:28452）在 ShowModal（:28453 → FormShow）之前
    }
    CHECK(filerw::PageJson(kLd, &json) == 200);
    CHECK(g_formShows == 2);
    CHECK(g_rec.size() == 1);

    std::printf("[7] 一般存檔之後的重讀不記；golden Close()（\"closed\"）之後的 editlist.get 再記\n");
    std::string ack, err;
    g_saveCloses = false;
    CHECK(filerw::PageSave(kLd, "{}", "{}", &ack, &err) == 200);
    CHECK(filerw::PageJson(kLd, &json) == 200);
    CHECK(g_rec.size() == 1);
    g_saveCloses = true;
    ack.clear(); err.clear();
    CHECK(filerw::PageSave(kLd, "{}", "{}", &ack, &err) == 200);
    CHECK(filerw::PageJson(kLd, &json) == 200);
    CHECK(g_rec.size() == 2);
    if (g_rec.size() == 2) CHECK(g_rec[1].code == "MES2176" && g_rec[1].formShowsBefore == g_formShows - 1);
    g_saveCloses = false;
    CHECK(filerw::PageJson(kLd, &json) == 200);
    CHECK(g_rec.size() == 2);

    std::printf("[8] 沒開機：409、不記\n");
    json.clear();
    const int showsBefore = g_formShows;
    CHECK(filerw::PageJson(kOff, &json) == 409);
    CHECK(g_formShows == showsBefore);
    CHECK(g_rec.size() == 2);

    std::printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
