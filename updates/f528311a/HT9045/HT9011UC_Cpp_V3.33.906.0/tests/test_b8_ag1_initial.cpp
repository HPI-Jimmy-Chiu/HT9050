// =============================================================================
//  test_b8_ag1_initial.cpp -- B8 AG-1 (B)：golden 開機讀 AGV.ini（main.cpp:9378）＋Initial Load／Initial Unload 兩顆鈕（假 IO）。
//
//  //AI(W906-B8-AG1) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「AG-1」
//    （Steven 20260929「請按照bcb的邏輯處理」⇒ 照 golden）。golden：V912（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy）
//    main.cpp:9378 fAGV->ReadFile()（TfMain::DoReadLastData）；Automation\AGV.cpp btInitalLoadClick :1316-1322、btInitalUnLoadClick :1324-1330、
//    InitialE84LoadSensor :221-229、InitialE84UnloadSensor :231-239。
//  受測：wb_serve 編進去的同一份 FileRW/TestIF_File_AGV.cpp（檔尾 FileRW_AGV_ReadFile、AG_EvBootProxies、事件表註冊）＋產生檔
//    FileRW/TestIF_File_AGV.gen.inc 的 AG_btInitalLoadClick／AG_btInitalUnLoadClick，經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp）與
//    RunPageEvent（FileRW/_EditPage.cpp）；Initial* 本體是 ht9045_sm 的 Automation/AGV_E84.cpp（jimmychiu 的檔，只呼叫）。
//    [0] 沙盒：W906_AGVINI_PATH（tests/CMakeLists.txt _ht9045_env_extra）換到沙盒底下自己的資料夾，沒有就拒跑
//    [1] 開機讀檔：沒有 AGV.ini ⇒ golden 預設值、bEnableE84=false（E84 交握不跑）；AGV.ini "E84 Enable"=1 ⇒ bEnableE84=true，逾時與盤數照檔
//        ⇒ AGVModal=1 時 csystem.cpp:30394 的條件成立（E84 交握從開機就跑）；AGVModal=0 不成立。讀檔不建檔、不改檔
//    [2] 事件表：editlist.get 的 events 有兩顆鈕（click、golden 出處、operable）
//    [3] 假 IO（SW[] Enable＋哨兵 ISABase，Status() 讀 OutValue，同 ctest AGV_E84 的做法）：Initial Load ⇒ E84_1 的 L_REQ／U_REQ／VA／READY／
//        VS_0／VS_1 關、HO_AVBL／ES／POWER 不動、E84_2 全不動、iE84LoadTask=1（iE84UnloadTask 不動）、bE84LoaderActionflag[0..2]=false
//        （Unloader 的不動）；Initial Unload 反過來；ack 走 C 路、golden 出處、changed 空
//    [4] 運轉中（//AI(W906-FE-RUNEXC) 20260930：FileRW/_FormEvent.cpp 檔尾的運轉中例外表；golden fAGV 非模態、運轉中按得到）：
//        表上 TfAGV 的列＝這兩顆（click、fAGV、SystemStart／SoftStart 都放行、golden AGV.cpp:1316-1322／:1324-1330）；
//        頁面表沒說 fAGV 開著 ⇒ 照舊 running、IO 與狀態都不動；開著 ⇒ SystemStart／SoftStart／兩個都立都照 golden 跑（只關自己那 6 顆）；
//        換了等級沒重開頁 ⇒ reload page、Panel3 看不見 ⇒ bad-payload（例外不跳過 RunPageEvent）；
//        不在表上的（spbSave、同一顆的 "change"、別的表單的鈕）運轉中照舊 running、訊息同以前
//    [5] 換了登入等級沒重開頁 ⇒ reload page；Panel3 看不見 ⇒ bad-payload（點不到）；兩種 IO 都不動
//    [6] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄、argv[2]＝D:\HT9045\web\page，唯讀）：wb_serve W906_DoReadLastData 在 FrmAOI 之後、DoIniDataToForm
//        段之前呼叫 FileRW_AGV_ReadFile（程式碼、不在 // 後面、不在 if (bBoot) 裡）；產生檔處理器照 golden；AGV_E84.cpp 的 6 顆 Off；頁面送出點
//    [7] 等級（//AI(W906-Q61-LEVEL) 20260930，Steven Q61「golden應該是有卡權限吧? 依照golden」）：golden fAGV 只能從工具選單開
//        （sbSettingClick V912 main.cpp:29030-29047：SystemStart return、Insufficient(0)＝[00] Main - Tools），處理器與 FormShow 不查等級。
//        [00] 不夠 ⇒ 停機開頁 not-authorized、按鈕 reload page；運轉中（頁面表說 fAGV 開著也一樣）、SoftStart 都按不到；[00] 夠 ⇒ 停機開頁、
//        停機／SystemStart／SoftStart 都照 golden 跑；運轉中開新頁 running（golden palSetup 藏起來）；開頁後等級降了 ⇒ reload page（比 golden 嚴，寫明）
//        //AI(W906-AUTHMAINFORM) 20260930：工具選單鈕 sbSetting->Enabled 另要 authMainForm[0]（ChangeLevelAttr main.cpp:12951，Security_new.def
//        [Main] Tool）：[00] 夠但開關 0 ⇒ disabled、PageJson 不跑；開關 1 ⇒ 照上面（本測試不跑 GetMainAuth，開關先設成 golden 預設 1）
//  NOT COVERED：真的 E84 交握（ctest AGV_E84／AGV_PortScan）、wb_serve 的 form.event 分派（整合者用探針驗）、Timer2（P-1）。
//  不碰真的 AGV.ini：只看它在不在、大小與修改時間（開跑前後相同），不開、不讀內容。
// =============================================================================
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "myswitch.h"
#include "Automation/AGV_E84.h"

#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#ifdef _WIN32
#include <direct.h>
#endif

// _putenv：MinGW.org 在嚴格模式（CXX_EXTENSIONS OFF）不宣告 POSIX／_ 名字 —— 同 tests/test_agv_e84.cpp:158-163
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

extern void FileRW_AGV_Boot();
extern void FileRW_AGV_ReadFile();
extern bool (*W906_FormFShowHook)(const char* goldenForm);   // csystem.h:421（本體 csystem.cpp）；W906_FormShowing(obj,false)＝hook(obj)。AI(W906-FE-RUNEXC) 20260930：[4] 用它當頁面表
extern int iMaxLevelItem;   // cSecurity.cpp:47（開機 W906_SecurityBoot 設 180；golden V912 cSecurity.cpp:210）。//AI(W906-Q61-LEVEL) 20260930：[7] 用
extern bool authMainForm[12];   // cAuthority.h:56（本體 cAuthority.cpp:119；開機 GetMainAuth 填，本測試不跑它）。//AI(W906-AUTHMAINFORM) 20260930：[7] 用
AnsiString W906_AgvIniPath();

// 連結用（不是受測碼；同 tests/test_b8_os5_sortbuttons.cpp）
void FileRW_IniConfig_ChangeCBListProperty() {}
namespace ht9045 {
namespace formbridge {
const BridgeDesc* FindBridge(const std::string&) { return nullptr; }
bool RunEvent(const BridgeDesc&, const formevent::Request&, formevent::Result* out)
{
    out->code = "handler-failed"; out->why = "test: no A-shape page";
    return false;
}
}  // namespace formbridge
namespace formjson {
void FormLock() {}
void FormUnlock() {}
}  // namespace formjson
}  // namespace ht9045

using filerw::EL;

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}

struct Meta { bool exists; long long size; long long mtime; };
Meta MetaOf(const char* p)
{
    struct stat st;
    Meta m = {false, 0, 0};
    if (stat(p, &st) == 0) { m.exists = true; m.size = (long long)st.st_size; m.mtime = (long long)st.st_mtime; }
    return m;
}
bool SameMeta(const Meta& a, const Meta& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s;
}

bool CodeHas(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    bool gated = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) return true;
    }
    return false;
}

// E84 交握輸出（cmydef.h）：golden Initial*Sensor 關的 6 顆＋不碰的 3 顆
const int kOff1[6]  = {SwE84_1_LREQ, SwE84_1_UREQ, SwE84_1_VA, SwE84_1_READY, SwE84_1_VS0, SwE84_1_VS1};
const int kKeep1[3] = {SwE84_1_HOAVBL, SwE84_1_ES, SwE84_1_POWER};
const int kOff2[6]  = {SwE84_2_LREQ, SwE84_2_UREQ, SwE84_2_VA, SwE84_2_READY, SwE84_2_VS0, SwE84_2_VS1};
const int kKeep2[3] = {SwE84_2_HOAVBL, SwE84_2_ES, SwE84_2_POWER};

// 假 IO：Enable＋哨兵 ISABase（不對應任何後端）＋Type=1 ⇒ Status() 回 On()／Off() 最後設的 OutValue（tests/test_agv_e84.cpp:195-203）
void FakeIo()
{
    const int* all[4] = {kOff1, kKeep1, kOff2, kKeep2};
    const int n[4] = {6, 3, 6, 3};
    for (int g = 0; g < 4; ++g)
        for (int i = 0; i < n[g]; ++i) { SW[all[g][i]].Enable = true; SW[all[g][i]].ISABase = -1; SW[all[g][i]].Type = 1; }
}
// 18 顆全開、兩邊狀態機停在交握中、軌道互鎖旗標全立
void Arm()
{
    const int* all[4] = {kOff1, kKeep1, kOff2, kKeep2};
    const int n[4] = {6, 3, 6, 3};
    for (int g = 0; g < 4; ++g)
        for (int i = 0; i < n[g]; ++i) SW[all[g][i]].On();
    iE84LoadTask = 900;
    iE84UnloadTask = 900;
    for (int i = 0; i < 3; ++i) { bE84LoaderActionflag[i] = true; bE84UnloaderActionflag[i] = true; }
}
int CountOn(const int* ids, int n) { int c = 0; for (int i = 0; i < n; ++i) if (SW[ids[i]].Status()) ++c; return c; }
int CountFlags(const bool* f) { return (f[0] ? 1 : 0) + (f[1] ? 1 : 0) + (f[2] ? 1 : 0); }
bool Untouched()
{
    return CountOn(kOff1, 6) == 6 && CountOn(kKeep1, 3) == 3 && CountOn(kOff2, 6) == 6 && CountOn(kKeep2, 3) == 3 &&
           iE84LoadTask == 900 && iE84UnloadTask == 900 && CountFlags(bE84LoaderActionflag) == 3 && CountFlags(bE84UnloaderActionflag) == 3;
}

bool Click(const char* control, std::string* ack, std::string* err)
{
    ack->clear(); err->clear();
    const std::string v = std::string("{\"form\":\"TfAGV\",\"control\":\"") + control + "\",\"event\":\"click\"}";
    return W906_FormEvent("Setup.AGV", v, ack, err);
}
//AI(W906-FE-RUNEXC) 20260930：[4] 用 —— 任意元件／事件，與頁面表的替身（g_open＝哪一個 golden 表單開在 HMI 上，nullptr＝都沒開）
bool ClickEv(const char* control, const char* event, std::string* ack, std::string* err)
{
    ack->clear(); err->clear();
    const std::string v = std::string("{\"form\":\"TfAGV\",\"control\":\"") + control + "\",\"event\":\"" + event + "\"}";
    return W906_FormEvent("Setup.AGV", v, ack, err);
}
const char* g_open = nullptr;
bool FakePageTable(const char* form) { return g_open && form && std::strcmp(form, g_open) == 0; }
bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }
std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}
bool AckShape(const std::string& ack, const char* golden)
{
    cJSON* a = cJSON_Parse(ack.c_str());
    const cJSON* ch = a ? cJSON_GetObjectItemCaseSensitive(a, "changed") : nullptr;
    const bool ok = a && Str(a, "route") == "C" && Str(a, "golden") == golden && ch && cJSON_IsObject(ch) && !ch->child;
    if (a) cJSON_Delete(a);
    return ok;
}

void SetEnv(const char* kv) { HT9045_TEST_PUTENV(kv); }

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_b8_ag1_initial -- B8 AG-1 (B) golden boot ReadFile (main.cpp:9378) + btInitalLoadClick / btInitalUnLoadClick (AGV.cpp:1316-1330)\n");
    const char* const kReal = "D:\\HT9045\\config\\AGV.ini";
    const Meta realBefore = MetaOf(kReal);

    std::printf("[0] sandbox\n");
    const char* env = std::getenv("W906_AGVINI_PATH");
    const std::string global = env ? env : "";
    if (global.empty() || Lower(global) == Lower(kReal) || global.find_last_of("\\/") == std::string::npos) {
        std::printf("  FAIL: W906_AGVINI_PATH is not set to a sandbox path (tests/CMakeLists.txt _ht9045_env_extra sets it) -- refusing to run\n");
        return 1;
    }
    const std::string root = global.substr(0, global.find_last_of("\\/"));
    const std::string dir = root + "/B8_Ag1_Initial";           // ctest -j：B8_Ag1_AgvIni 用同一個根的另一個資料夾
    const std::string sandbox = dir + "/AGV.ini";
    _mkdir(root.c_str());
    _mkdir(dir.c_str());
    std::remove(sandbox.c_str());
    SetEnv((std::string("W906_AGVINI_PATH=") + sandbox).c_str());
    Check(std::string(W906_AgvIniPath().c_str()) == sandbox && !MetaOf(sandbox.c_str()).exists, "[0] seam -> own sandbox folder, AGV.ini absent: " + sandbox);

    SystemStart = false; SoftStart = false;
    FileRW_AGV_Boot();

    std::printf("[1] boot read (golden DoReadLastData main.cpp:9378 fAGV->ReadFile())\n");
    TestIF_File.bEnableE84 = true;
    TestIF_File.iE84TimeOut_K12[1][7] = -1;
    TestIF_File.iLoaderUnloaderTrayCount[2] = -1;
    FileRW_AGV_ReadFile();
    Check(!TestIF_File.bEnableE84 && TestIF_File.iE84TimeOut_K12[1][7] == 120 && TestIF_File.iLoaderUnloaderTrayCount[2] == 0 &&
          !MetaOf(sandbox.c_str()).exists, "[1] no AGV.ini -> golden defaults (TA2 120, tray count 0), bEnableE84=false, file not created");
    {
        std::ofstream f(sandbox.c_str(), std::ios::binary);
        f << "[Configuration]\r\nE84_1 Time Out 1=7\r\nE84_2 Time Out 8=33\r\nAuto3 Tray Count=4\r\nE84 Enable=1\r\n";
    }
    const Meta sbBefore = MetaOf(sandbox.c_str());
    std::string sbText0;
    ReadAll(sandbox, &sbText0);
    FileRW_AGV_ReadFile();
    std::string sbText1;
    ReadAll(sandbox, &sbText1);
    Check(TestIF_File.bEnableE84 && TestIF_File.iE84TimeOut_K12[0][0] == 7 && TestIF_File.iE84TimeOut_K12[1][7] == 33 &&
          TestIF_File.iLoaderUnloaderTrayCount[2] == 4 && TestIF_File.iE84TimeOut_K12[0][1] == 2,
          "[1] AGV.ini \"E84 Enable\"=1 -> bEnableE84=true; timeouts / tray counts from the file, missing keys = golden defaults");
    Check(sbText1 == sbText0 && SameMeta(sbBefore, MetaOf(sandbox.c_str())), "[1] the boot read is read-only (file bytes / size / mtime unchanged)");
    USE_E84_Sensor = 1;
    Check(USE_E84_Sensor == 1 && TestIF_File.bEnableE84, "[1] AGVModal=1 + E84 Enable=1 -> the csystem.cpp:30394 condition holds from boot (E84 handshake runs)");
    USE_E84_Sensor = 0;
    Check(!(USE_E84_Sensor == 1 && TestIF_File.bEnableE84), "[1] AGVModal=0 -> the condition does not hold (read, but no handshake)");
    USE_E84_Sensor = 1;

    std::printf("[2] event table\n");
    const filerw::PageDesc* d = filerw::FindPageForEvent("Setup.AGV");
    std::string json;
    Check(d && std::strcmp(d->tag, "TestIF_File_AGV") == 0 && filerw::PageJson(*d, &json) == 200, "[2] Setup.AGV -> TestIF_File_AGV, editlist.get 200 (opens the page at this level)");
    {
        cJSON* root2 = cJSON_Parse(json.c_str());
        const cJSON* ev = root2 ? cJSON_GetObjectItemCaseSensitive(root2, "events") : nullptr;
        const cJSON* l = ev ? cJSON_GetObjectItemCaseSensitive(ev, "btInitalLoad") : nullptr;
        const cJSON* u = ev ? cJSON_GetObjectItemCaseSensitive(ev, "btInitalUnLoad") : nullptr;
        const cJSON* lo = l ? cJSON_GetObjectItemCaseSensitive(l, "operable") : nullptr;
        const cJSON* uo = u ? cJSON_GetObjectItemCaseSensitive(u, "operable") : nullptr;
        Check(ev && cJSON_GetArraySize(ev) == 2 && Str(l, "event") == "click" && Str(u, "event") == "click" &&
              Str(l, "golden") == "Automation/AGV.cpp:1316 TfAGV::btInitalLoadClick" &&
              Str(u, "golden") == "Automation/AGV.cpp:1324 TfAGV::btInitalUnLoadClick" && lo && cJSON_IsTrue(lo) && uo && cJSON_IsTrue(uo),
              "[2] events: btInitalLoad / btInitalUnLoad click, golden :1316 / :1324, operable");
        if (root2) cJSON_Delete(root2);
    }

    std::printf("[3] fake IO\n");
    FakeIo();
    Arm();
    Check(Untouched(), "[3] armed: 18 E84 outputs on, both tasks at 900, all 6 transfer-lock flags set");
    std::string ack, err;
    bool sent = Click("btInitalLoad", &ack, &err);
    Check(sent && AckShape(ack, "Automation/AGV.cpp:1316 TfAGV::btInitalLoadClick"), "[3] Initial Load -> ack route C, golden :1316, no changed " + err);
    Check(CountOn(kOff1, 6) == 0 && CountOn(kKeep1, 3) == 3, "[3] Initial Load: E84_1 L_REQ / U_REQ / VA / READY / VS_0 / VS_1 off; HO_AVBL / ES / POWER untouched");
    Check(CountOn(kOff2, 6) == 6 && CountOn(kKeep2, 3) == 3, "[3] Initial Load: every E84_2 output untouched");
    Check(iE84LoadTask == 1 && iE84UnloadTask == 900, "[3] Initial Load: iE84LoadTask=1, iE84UnloadTask untouched");
    Check(CountFlags(bE84LoaderActionflag) == 0 && CountFlags(bE84UnloaderActionflag) == 3, "[3] Initial Load: bE84LoaderActionflag[0..2]=false (V912), unloader flags untouched");
    Arm();
    sent = Click("btInitalUnLoad", &ack, &err);
    Check(sent && AckShape(ack, "Automation/AGV.cpp:1324 TfAGV::btInitalUnLoadClick"), "[3] Initial Unload -> ack route C, golden :1324, no changed " + err);
    Check(CountOn(kOff2, 6) == 0 && CountOn(kKeep2, 3) == 3 && CountOn(kOff1, 6) == 6 && CountOn(kKeep1, 3) == 3,
          "[3] Initial Unload: E84_2 six outputs off, E84_2 HO_AVBL / ES / POWER and every E84_1 output untouched");
    Check(iE84UnloadTask == 1 && iE84LoadTask == 900 && CountFlags(bE84UnloaderActionflag) == 0 && CountFlags(bE84LoaderActionflag) == 3,
          "[3] Initial Unload: iE84UnloadTask=1, bE84UnloaderActionflag[0..2]=false; loader side untouched");

    std::printf("[4] running: the running-exception table (FileRW/_FormEvent.cpp tail, AI(W906-FE-RUNEXC) 20260930; golden fAGV is non-modal)\n");
    {
        int rows = 0, match = 0;
        for (int i = 0; i < formevent::runexc::RowCount(); ++i) {
            const formevent::runexc::RowInfo* r = formevent::runexc::RowAt(i);
            if (!r || std::strcmp(r->form, "TfAGV") != 0) continue;
            ++rows;
            const bool load = std::strcmp(r->control, "btInitalLoad") == 0, unload = std::strcmp(r->control, "btInitalUnLoad") == 0;
            if ((load || unload) && std::strcmp(r->event, "click") == 0 && std::strcmp(r->shownObj, "fAGV") == 0 && r->systemStart && r->softStart &&
                r->golden && std::strstr(r->golden, load ? "AGV.cpp:1316-1322" : "AGV.cpp:1324-1330")) ++match;
        }
        Check(rows == 2 && match == 2,
              "[4] table: the TfAGV rows are exactly btInitalLoad / btInitalUnLoad (click, fAGV, SystemStart + SoftStart allowed, golden AGV.cpp:1316-1322 / :1324-1330)");
    }
    Arm();
    SystemStart = true;
    W906_FormFShowHook = nullptr;                // 沒有頁面表（ctest 的預設）＝ AGV 沒開
    Check(!Click("btInitalLoad", &ack, &err) && Starts(err, "running:") && err.find("fAGV") != std::string::npos && Untouched(),
          "[4] SystemStart, no page table -> running (the row's reason names fAGV), IO / tasks / flags untouched");
    W906_FormFShowHook = &FakePageTable;
    g_open = "fOffSet";                          // 開著的是別的表單
    Check(!Click("btInitalUnLoad", &ack, &err) && Starts(err, "running:") && Untouched(), "[4] SystemStart, the page table says only fOffSet is open -> running, untouched");
    g_open = "fAGV";
    sent = Click("btInitalLoad", &ack, &err);
    Check(sent && AckShape(ack, "Automation/AGV.cpp:1316 TfAGV::btInitalLoadClick") && CountOn(kOff1, 6) == 0 && CountOn(kKeep1, 3) == 3 &&
          CountOn(kOff2, 6) == 6 && CountOn(kKeep2, 3) == 3 && iE84LoadTask == 1 && iE84UnloadTask == 900 &&
          CountFlags(bE84LoaderActionflag) == 0 && CountFlags(bE84UnloaderActionflag) == 3,
          "[4] SystemStart + AGV open: Initial Load runs golden :1316 (E84_1 six outputs off, iE84LoadTask=1, loader flags cleared; the rest untouched) " + err);
    Arm();
    SystemStart = false; SoftStart = true;
    sent = Click("btInitalUnLoad", &ack, &err);
    Check(sent && AckShape(ack, "Automation/AGV.cpp:1324 TfAGV::btInitalUnLoadClick") && CountOn(kOff2, 6) == 0 && CountOn(kKeep2, 3) == 3 &&
          CountOn(kOff1, 6) == 6 && CountOn(kKeep1, 3) == 3 && iE84UnloadTask == 1 && iE84LoadTask == 900 &&
          CountFlags(bE84UnloaderActionflag) == 0 && CountFlags(bE84LoaderActionflag) == 3,
          "[4] SoftStart + AGV open: Initial Unload runs golden :1324 (E84_2 six outputs off, iE84UnloadTask=1, unloader flags cleared; the rest untouched) " + err);
    Arm();
    SystemStart = true;                          // 兩個都立
    sent = Click("btInitalLoad", &ack, &err);
    Check(sent && CountOn(kOff1, 6) == 0 && iE84LoadTask == 1 && CountOn(kOff2, 6) == 6 && iE84UnloadTask == 900,
          "[4] SystemStart + SoftStart + AGV open: Initial Load runs " + err);
    SoftStart = false;
    Arm();
    {
        const int lvR = AccessLevel;
        AccessLevel = lvR + 1;
        Check(!Click("btInitalLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
              "[4] SystemStart + AGV open, access level changed since editlist.get -> reload page (RunPageEvent step 1 still applies), untouched");
        AccessLevel = lvR;
    }
    EL<TPanel>("TfAGV", "Panel3")->Visible = false;
    Check(!Click("btInitalUnLoad", &ack, &err) && Starts(err, "bad-payload") && Untouched(),
          "[4] SystemStart + AGV open, Panel3 hidden -> bad-payload (RunPageEvent ELOperable still applies), untouched");
    EL<TPanel>("TfAGV", "Panel3")->Visible = true;
    {
        const std::string gen = "running: 機台運轉中（SystemStart）不能從網頁操作設定畫面";   // 原本的訊息（FileRW/_FormEvent.cpp:79）
        const bool save = ClickEv("spbSave", "click", &ack, &err) || !Starts(err, gen);
        const bool chg = ClickEv("btInitalLoad", "change", &ack, &err) || !Starts(err, gen);
        const bool other = ClickEv("btnSortAuto1", "click", &ack, &err) || !Starts(err, gen);
        Check(!save && Untouched(), "[4] not listed, SystemStart + AGV open: spbSave click -> running (message as before, before the handler lookup), untouched");
        Check(!chg && Untouched(), "[4] not listed: btInitalLoad \"change\" (the table is per event) -> running, untouched");
        Check(!other && Untouched(), "[4] not listed for TfAGV: btnSortAuto1 on the AGV page (the table is keyed by the dispatched form) -> running, untouched");
    }
    SystemStart = false;
    W906_FormFShowHook = nullptr;
    g_open = nullptr;

    std::printf("[5] page not open at this level / not operable\n");
    const int lv0 = AccessLevel;
    AccessLevel = lv0 + 1;
    Check(!Click("btInitalLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
          "[5] access level changed since editlist.get -> reload page, untouched");
    AccessLevel = lv0;
    EL<TPanel>("TfAGV", "Panel3")->Visible = false;
    Check(!Click("btInitalUnLoad", &ack, &err) && err.compare(0, 11, "bad-payload") == 0 && Untouched(),
          "[5] Panel3 hidden -> bad-payload (cannot be operated), untouched");
    EL<TPanel>("TfAGV", "Panel3")->Visible = true;

    std::printf("[6] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[6] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        const std::string src = argv[1], web = argv[2];
        std::string wb, gen, e84, js, cs;
        const bool rd = ReadAll(src + "/tools/wb_serve.cpp", &wb) && ReadAll(src + "/FileRW/TestIF_File_AGV.gen.inc", &gen) &&
                        ReadAll(src + "/Automation/AGV_E84.cpp", &e84) && ReadAll(src + "/csystem.cpp", &cs) && ReadAll(web + "/ht9045_agv_c.js", &js);
        Check(rd, "[6] read wb_serve.cpp, the generated file, AGV_E84.cpp, csystem.cpp, ht9045_agv_c.js");
        const std::string call = "{ extern void FileRW_AGV_ReadFile(); FileRW_AGV_ReadFile(); }";
        const std::string::size_type fn = wb.find("void W906_DoReadLastData(bool bBoot");
        const std::string::size_type aoi = wb.find("{ extern void FileRW_AOISetup_ReadFile(); FileRW_AOISetup_ReadFile(); }", fn);
        const std::string::size_type agv = wb.find(call, fn);
        const std::string::size_type hp = wb.find("fHotPlate->DoIniDataToForm();", fn);
        bool sameLineCode = false, inBootIf = true;
        if (agv != std::string::npos) {
            const std::string::size_type b = wb.rfind('\n', agv) + 1, e = wb.find('\n', agv);
            const std::string line = wb.substr(b, e - b);
            const std::string::size_type sl = line.find("//"), at = line.find(call);
            sameLineCode = at != std::string::npos && (sl == std::string::npos || at < sl) && line.find("FileRW_AOISetup_ReadFile(); }") != std::string::npos;
            inBootIf = line.find("if (bBoot)") != std::string::npos && line.find("if (bBoot)") < at;
        }
        Check(fn != std::string::npos && aoi != std::string::npos && agv != std::string::npos && hp != std::string::npos && aoi < agv && agv < hp &&
              sameLineCode && !inBootIf,
              "[6] W906_DoReadLastData calls FileRW_AGV_ReadFile after FrmAOI (golden :9376 -> :9378), before the DoIniDataToForm block, "
              "as code on the FrmAOI line (before its //), boot and recipe change alike");
        Check(CodeHas(gen, "InitialE84LoadTask();") && CodeHas(gen, "InitialE84LoadSensor();") && CodeHas(gen, "bE84LoaderActionflag[i]=false;") &&
              CodeHas(gen, "InitialE84UnLoaderTask();") && CodeHas(gen, "InitialE84UnloadSensor();") && CodeHas(gen, "bE84UnloaderActionflag[i]=false;") &&
              gen.find("// golden Automation/AGV.cpp:1316  TfAGV::btInitalLoadClick(TObject *Sender)") != std::string::npos,
              "[6] generated AG_btInitalLoadClick / AG_btInitalUnLoadClick are the golden bodies (live code)");
        Check(CodeHas(e84, "SW[SwE84_1_LREQ].Off();") && CodeHas(e84, "SW[SwE84_1_VS1].Off();") && CodeHas(e84, "SW[SwE84_2_LREQ].Off();") &&
              CodeHas(e84, "SW[SwE84_2_VS1].Off();"), "[6] Automation/AGV_E84.cpp InitialE84LoadSensor / InitialE84UnloadSensor switch the golden outputs off");
        Check(CodeHas(cs, "if(USE_E84_Sensor==1 && TestIF_File.bEnableE84)"), "[6] csystem.cpp:30394 E84 reader still gates on bEnableE84");
        Check(js.find("send({ form: FORM, control: id, event: 'click' }, 0)") != std::string::npos && js.find("EV_TAG = 'Setup.AGV'") != std::string::npos &&
              js.find("btInitalLoad:") != std::string::npos && js.find("btInitalUnLoad:") != std::string::npos,
              "[6] page: both buttons send form.event click with tag Setup.AGV");
    }

    //AI(W906-Q61-LEVEL) 20260930 [W906]：Steven 20260930 Q61「golden應該是有卡權限吧? 依照golden」——等級照 golden，停機與運轉中同一套。
    //   golden V912（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\，cp950）：spbAGV 在工具選單 palSetup（main.dfm:10380）；palSetup 只有
    //   sbSettingClick 打得開（main.cpp:29030-29047：if(SystemStart) return; → fSecurity->Insufficient(0)＝[00] Main - Tools；sbSetting->Enabled＝
    //   AccessLevel>=LevelSet.AccessLevel[0] && authMainForm[0]，ChangeLevelAttr :12951，SystemStart 時 false :12939）。spbAGVClick（:35776-35781）、
    //   TfAGV::FormShow（Automation\AGV.cpp:1304-1309）、btInitalLoadClick／btInitalUnLoadClick（:1316-1330）都不查等級；開著的 fAGV 換等級也不關
    //   （全樹沒有 fAGV->Close）。移植樹：開頁 GAgv→GToolsMenu（FileRW/_EditPage.cpp，同一條 SystemStart＋Insufficient(0)）＋事件要「在同一個
    //   等級開過頁」（RunPageEvent 第 1 步）；運轉中例外（FileRW/_FormEvent.cpp 檔尾）自己不查等級，放行後照樣走這兩道。
    //   這裡照 wb_serve editlist.get 臂的順序：OpenGateRefused 放行才跑 PageJson（golden FormShow）。
    std::printf("[7] access level (Q61): golden tools-menu gate [00] (sbSettingClick main.cpp:29036), stopped and running\n");
    {
        const int lv0 = AccessLevel, need0 = LevelSet.AccessLevel[0], mx0 = iMaxLevelItem;
        const bool auth0 = authMainForm[0];          //AI(W906-AUTHMAINFORM) 20260930
        authMainForm[0] = true;                      // golden 預設 1（cAuthority.cpp:366 缺鍵預設 1）；GetMainAuth 只在 wb_serve 開機跑
        std::string why;
        iMaxLevelItem = 180;                         // 開機後的值（W906_SecurityBoot；golden cSecurity.cpp:210）
        LevelSet.AccessLevel[0] = 2;                 // [00] Main - Tools 要 Supervisor(2)
        USE_E84_Sensor = 1;                          // spbAGV 看得見（:24333）——只剩等級
        W906_FormFShowHook = &FakePageTable;
        g_open = "fAGV";                             // 頁面表說 AGV 開著（運轉中例外的第 3 條成立）——等級不夠時也不能因此放行
        SystemStart = false; SoftStart = false;
        AccessLevel = 1;                             // Engineer(1) < Supervisor(2)
        Arm();
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && Starts(why, "not-authorized:") &&
              why.find("[00] Main - Tools") != std::string::npos && why.find("main.cpp:29036") != std::string::npos,
              "[7] stopped, Engineer(1) < [00] Supervisor(2): editlist.get refused not-authorized (golden sbSettingClick main.cpp:29036 Insufficient(0))");
        Check(!Click("btInitalLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
              "[7] stopped, Engineer(1): the page never opened at this level -> Initial Load refused (reload page), IO / tasks / flags untouched");
        SystemStart = true;
        why.clear();
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && Starts(why, "running:"),
              "[7] SystemStart, Engineer(1): editlist.get refused running (golden sbSettingClick if(SystemStart) return; palSetup hidden :3969-3978)");
        Check(!Click("btInitalUnLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
              "[7] SystemStart + page table says fAGV open, Engineer(1) never opened it at this level -> refused, untouched "
              "(the running exception does not bypass the level)");
        SystemStart = false; SoftStart = true;
        why.clear();
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && Starts(why, "not-authorized:"),
              "[7] SoftStart, Engineer(1): editlist.get refused not-authorized (golden sbSettingClick looks only at SystemStart, then Insufficient(0))");
        Check(!Click("btInitalLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
              "[7] SoftStart + fAGV open, Engineer(1): Initial Load refused, untouched");
        SoftStart = false;

        AccessLevel = 2;                             // Supervisor(2) >= [00]
        //AI(W906-AUTHMAINFORM) 20260930：等級夠、[Main] Tool=0 ⇒ golden sbSetting 是灰的（ChangeLevelAttr main.cpp:12951）⇒ disabled、golden FormShow 不跑
        authMainForm[0] = false;
        why.clear();
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && Starts(why, "disabled:") && why.find("main.cpp:12951") != std::string::npos &&
              why.find("[Main] Tool=0") != std::string::npos,
              "[7] stopped, Supervisor(2) >= [00] but Security_new.def [Main] Tool=0: editlist.get refused disabled (golden ChangeLevelAttr main.cpp:12951 "
              "sbSetting->Enabled=(... && authMainForm[0])) " + why);
        Check(!Click("btInitalLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
              "[7] stopped, [Main] Tool=0: the page never opened -> Initial Load refused (reload page), untouched");
        authMainForm[0] = true;
        why.clear();
        const filerw::PageDesc* pd = filerw::FindPage("TestIF_File_AGV");
        const bool gRefused = filerw::OpenGateRefused("TestIF_File_AGV", false, &why);
        std::string pj;
        const int st = !gRefused && pd ? filerw::PageJson(*pd, &pj) : -1;
        Check(!gRefused && st == 200, "[7] stopped, Supervisor(2): editlist.get allowed, golden FormShow ran (200) -- the page is open at this level " + why);
        Arm();
        sent = Click("btInitalLoad", &ack, &err);
        Check(sent && CountOn(kOff1, 6) == 0 && CountOn(kKeep1, 3) == 3 && iE84LoadTask == 1 && CountOn(kOff2, 6) == 6 && iE84UnloadTask == 900,
              "[7] stopped, Supervisor(2): Initial Load runs golden :1316 (E84_1 six off, iE84LoadTask=1; E84_2 untouched) " + err);
        Arm();
        SystemStart = true;
        why.clear();
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && Starts(why, "running:"),
              "[7] SystemStart, Supervisor(2): a new AGV page cannot be opened during a run (golden palSetup hidden) -- only one opened before START");
        sent = Click("btInitalUnLoad", &ack, &err);
        Check(sent && CountOn(kOff2, 6) == 0 && CountOn(kKeep2, 3) == 3 && iE84UnloadTask == 1 && CountOn(kOff1, 6) == 6 && iE84LoadTask == 900,
              "[7] SystemStart + fAGV open, opened at Supervisor(2) before START: Initial Unload runs golden :1324 (same level rule as stopped) " + err);
        Arm();
        SystemStart = false; SoftStart = true;
        sent = Click("btInitalLoad", &ack, &err);
        Check(sent && CountOn(kOff1, 6) == 0 && iE84LoadTask == 1 && CountOn(kOff2, 6) == 6 && iE84UnloadTask == 900,
              "[7] SoftStart + fAGV open, Supervisor(2): Initial Load runs " + err);
        SoftStart = false; SystemStart = true;
        Arm();
        AccessLevel = 1;                             // 運轉中等級降到 Engineer（golden A01 自動登出 Timer3Timer main.cpp:26036-26046；fAGV 不關）
        Check(!Click("btInitalLoad", &ack, &err) && err.find("reload page") != std::string::npos && Untouched(),
              "[7] SystemStart, level dropped to Engineer(1) after the page was opened -> refused (reload page), untouched "
              "-- stricter than golden (golden keeps fAGV open and clickable; AGV.cpp has no level check)");

        SystemStart = false; SoftStart = false;
        AccessLevel = lv0; LevelSet.AccessLevel[0] = need0; iMaxLevelItem = mx0;
        authMainForm[0] = auth0;                     //AI(W906-AUTHMAINFORM) 20260930
        W906_FormFShowHook = nullptr;
        g_open = nullptr;
    }

    const Meta realAfter = MetaOf(kReal);
    Check(SameMeta(realBefore, realAfter), std::string("[guard] real ") + kReal + (realAfter.exists ? " unchanged (size / mtime)" : " still absent"));
    std::printf("test_b8_ag1_initial: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
