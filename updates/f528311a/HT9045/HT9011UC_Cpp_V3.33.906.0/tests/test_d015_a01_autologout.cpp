// =============================================================================
//  test_d015_a01_autologout.cpp -- todo D-015：golden [A01] 閒置自動切回 Operator ＋ A01 30 分鐘自動重開 ＋ Offset 存檔歸 0。
//
//  //AI(W906-D015-A01) 20260930 [W906] St01 新檔。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp TfMain::Timer3Timer（:25820；Timer3 1000 ms，main.dfm:17333）
//    :25830-25831 InitialOK、:25957-25982 畫面開著歸 0、:26022-26078 計數＋登出、:26080-26096 A01 自動重開；cOffSet.cpp:2811 spbSaveClick 歸 0。
//  受測的是 wb_serve 編進去的同一份 FileRW/Main_A01AutoLogout.cpp；god-stack 用 RESCAN 連。三個安裝座都裝假的：
//    W906_WebLoginForceOperatorHook（登出本體；真的本體 WebLogin.cpp 由 ctest WebLogin_ForceOperator 驗）、
//    W906_FormFShowHook（頁面表的答案）、W906_FormProgramShowHook（程式關窗）。FormLock 是本檔的替身（記錄鎖的深度）。
//  **SIM／出貨兩種組態都要過**：#ifdef SOFT_SIMULTE 的那幾段（golden :26026-26028 在 SIM 立刻歸 0 ⇒ SIM 永遠不會登出）。
//    [0] 沙盒：W906_A01CONFIGINI_PATH 有設、不是真的 config.ini（沒有就拒跑）；本測試換到沙盒底下自己的資料夾
//    [1] 測試縫：W906_A01ConfigIniPath()＝環境變數；沒設或空字串＝AuthPath+"config.ini"（golden :26087，只比字串）
//    [2] 節拍：第一次只起算；500 ms 拍子＋抖動 ⇒ 剛好每秒一次（不會拉長）；主迴圈卡 30 秒 ⇒ 只跑一次、不補秒；InitialOK=false ⇒ 什麼都不動
//    [3] A01 關／AccessLevel 0 ⇒ 計數歸 0
//    [4] SIM：計數永遠 0、登出本體一次都沒叫；出貨：max(10, iA01ChangeOpTime) 那一秒剛好叫一次（鎖外面）、<10 的設定不會提早、
//        下一秒 AccessLevel 0 ⇒ 歸 0；fOffSet 開著時程式關窗（KYEC 例外才數得到）；安裝座是 0 不會當
//    [5] 畫面開著歸 0：三個成員（fiosetview／fObserver／fContact）＋頁面表（五個表單，含 fOffSet、fSpeed）、SPIL／VTEST 那一組、SJ 永遠不歸 0、KYEC 的 Offset 例外
//    [6] Offset 存檔的歸零函式
//    [7] 自動重開：1799 秒不寫、1800 秒 A01=true 並寫沙盒 [Function] bAutoSwitchToOperatorMode=1、之後歸 0；CosFunction 關 ⇒ 不動
//    [8] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄，唯讀）：wb_serve.cpp 那一行的呼叫在程式碼裡（不在 // 或 /* */ 裡、同一行）、
//        Offset_File.gen.inc 的 ELTodo 換成歸零函式、_editlist_sources.cmake／_integrated.txt、SOFT_SIMULTE 那一段還在；
//        AI(W906-D015-A01b) 20260930：W906_ModalWaitTick 第一行（St01 D-012 那一行）也呼叫、act.main.menuVisible 在 St01 那一行分派、
//        W906_PrintDataRedirects 列出 W906_A01CONFIGINI_PATH
//    [9] AI(W906-D015-A01b) 20260930 主畫面 Tools／Config 選單（golden palSetup／palConfig，:25965）：act.main.menuVisible 的格式檢查、
//        開著 ⇒ 每秒歸 0（SJ 不歸、SPIL 那一組不看選單、KYEC 的 Offset 例外）、主畫面不在（頁面表）⇒ 旗標不算、關了照數；
//        出貨：登出本體後旗標清掉（:26045-26048）；兩種組態：ChangeLevelAttr 的「降到 0 收選單」（:12929-12935／:13183-13189）
//    [10] AI(W906-D015-A01b) 20260930 重入保險：tick 裡面（頁面表問答那一刻）又叫 tick ⇒ 直接 return、不多跑一次、截止時間不動
//  不碰真的 D:\HT9045\config\config.ini：只看在不在／大小／修改時間（開跑前後相同）。
// =============================================================================
#include "cmydef.h"             // InitialOK／AccessLevel／CUSTOMER_CODE；MachineType.h：SOFT_SIMULTE、CC_*
#include "Config.h"             // IniConfig
#include "CosFunction.h"        // CosFunction
#include "common.h"             // AuthPath
#include "atester_shims.h"      // fContact／fiosetview
#include "forms/fObserver.h"    // fObserver

#include <sys/stat.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#endif

// _putenv：MinGW.org 在嚴格模式（CXX_EXTENSIONS OFF）不宣告 POSIX／_ 名字 —— 同 tests/test_b8_ag1_agvini.cpp
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

// ---- 受測（FileRW/Main_A01AutoLogout.cpp）----
void       W906_A01AutoLogoutTick();
void       W906_A01ResetOperatorModeCount();
int        W906_A01OperatorModeCount();
int        W906_A01OpenCount();
AnsiString W906_A01ConfigIniPath();
void       W906_A01SetClockForTest(unsigned long (*nowMs)());
void       W906_A01SetCountsForTest(int operatorMode, int openA01);
void       W906_A01RunStepsForTest(int mask);
unsigned   W906_A01PassesForTest();
std::string W906_A01MenuVisibleOp(const std::string& payloadJson, bool* ok);   // AI(W906-D015-A01b) 20260930
int        W906_A01MenuStateForTest();                                          // 1＝Tools（palSetup）、2＝Config（palConfig）
void       W906_A01ResetMenuForTest();
unsigned   W906_A01ReentriesForTest();

// ---- 安裝座（LogObjects.cpp 檔尾、csystem.cpp）----
extern void (*W906_WebLoginForceOperatorHook)();
extern bool (*W906_FormFShowHook)(const char* goldenForm);
extern void (*W906_FormProgramShowHook)(const char* goldenForm, bool open, const char* where);

// FormJson 鎖的替身（JsonBridge/FormJson.cpp 只編進 wb_serve）：記深度，驗「叫登出本體時沒拿著鎖」
namespace ht9045 {
namespace formjson {
int g_depth = 0;
void FormLock()   { ++g_depth; }
void FormUnlock() { --g_depth; }
}  // namespace formjson
}  // namespace ht9045

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

std::string Num(long v) { char b[32]; std::snprintf(b, sizeof(b), "%ld", v); return b; }

// ---- 假的安裝座 ----
std::set<std::string> g_open;                         // 頁面表說「開著」的 golden 表單名
bool FakeFShow(const char* f) { return f && g_open.count(f) != 0; }

struct ProgShow { std::string form; bool open; };
std::vector<ProgShow> g_prog;
void FakeProgShow(const char* f, bool open, const char*) { g_prog.push_back(ProgShow{f ? f : "", open}); }

int g_logouts = 0;
int g_depthAtLogout = -1;
void FakeLogout()                                     // 真本體（WebLogin.cpp:531）會把 AccessLevel 設 0，這裡照做
{
    ++g_logouts;
    g_depthAtLogout = ht9045::formjson::g_depth;
    AccessLevel = 0;
}

unsigned long g_now = 0;
unsigned long FakeNow() { return g_now; }

void Baseline()
{
    InitialOK = true;
    IniConfig.bSPILFunction = false;
    IniConfig.bVTESTFunction = 0;
    IniConfig.bA01AutoSwitchToOperatorMode = true;
    IniConfig.iA01ChangeOpTime = 15;
    CosFunction.bAutoOpenConfigA01 = false;
    CUSTOMER_CODE = -1;                               // 不是 SJ、不是 KYEC
    AccessLevel = 1;
    fiosetview->fShow = false;
    fObserver->bShow = false;
    fContact->fShow = false;
    g_open.clear();
    g_prog.clear();
    W906_WebLoginForceOperatorHook = &FakeLogout;
    W906_FormFShowHook = &FakeFShow;
    W906_FormProgramShowHook = &FakeProgShow;
    W906_A01SetCountsForTest(0, 0);
    g_logouts = 0;                                    // AI(W906-D015-A01) 20260930：每一格從 0 算（出貨組態 gate 0812b7da：[4] 第一格登出一次後沒歸 0，後面三格 g_logouts 對不上）
    g_depthAtLogout = -1;
    W906_A01ResetMenuForTest();                       // AI(W906-D015-A01b) 20260930：兩個選單旗標關、ChangeLevelAttr 的上一次等級回到 -999
}

// AI(W906-D015-A01b) 20260930 [10]：頁面表問答的那一刻再叫一次 tick（模擬「登出本體開了阻塞框 → W906_ModalWaitTick → tick」）
bool g_reenterArmed = false;
bool ReentrantFShow(const char* f)
{
    if (g_reenterArmed) {
        g_reenterArmed = false;
        g_now += 1000;                                // 巢狀那一次已經到期：沒有保險就會多跑一次
        W906_A01AutoLogoutTick();
    }
    return FakeFShow(f);
}

bool MenuOp(const char* json)                         // act.main.menuVisible 的 value
{
    bool ok = false;
    W906_A01MenuVisibleOp(json, &ok);
    return ok;
}

// ---- 檔案 ----
struct Meta { bool exists; long long size; long long mtime; };
Meta MetaOf(const char* p)
{
    struct stat st;
    Meta m = {false, 0, 0};
    if (stat(p, &st) == 0) { m.exists = true; m.size = (long long)st.st_size; m.mtime = (long long)st.st_mtime; }
    return m;
}
bool SameMeta(const Meta& a, const Meta& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    out->erase(std::remove(out->begin(), out->end(), '\r'), out->end());   // 行尾不拘（gate 工作樹 checkout 成 CRLF）
    return true;
}

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s;
}

std::string IniValue(const std::string& text, const std::string& section, const std::string& key)
{
    std::istringstream in(text);
    std::string l;
    bool sec = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l[0] == '[') { sec = (l == "[" + section + "]"); continue; }
        if (!sec) continue;
        const std::string::size_type eq = l.find('=');
        if (eq != std::string::npos && l.substr(0, eq) == key) return l.substr(eq + 1);
    }
    return "<none>";
}

// 一行去掉註解（// 到行尾、/* ... */），字串常值原樣保留
std::string CodeOfLine(const std::string& l)
{
    std::string out;
    bool inStr = false, inBlock = false;
    for (std::size_t i = 0; i < l.size(); ++i) {
        const char c = l[i];
        const char n = i + 1 < l.size() ? l[i + 1] : '\0';
        if (inBlock) { if (c == '*' && n == '/') { inBlock = false; ++i; } continue; }
        if (inStr) { out += c; if (c == '\\' && n) { out += n; ++i; } else if (c == '"') inStr = false; continue; }
        if (c == '"') { inStr = true; out += c; continue; }
        if (c == '/' && n == '/') break;
        if (c == '/' && n == '*') { inBlock = true; ++i; continue; }
        out += c;
    }
    return out;
}

// 程式碼（跳過 #if 0 … #endif、去掉註解）裡 needle 出現的行數
int CodeCount(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    bool gated = false;
    int n = 0;
    while (std::getline(in, l)) {
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        if (CodeOfLine(l).find(needle) != std::string::npos) ++n;
    }
    return n;
}

// needle 所在那一行（第一個；沒有回空字串）
std::string LineWith(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) if (l.find(needle) != std::string::npos) return l;
    return std::string();
}

void SetEnv(const char* kv) { HT9045_TEST_PUTENV(kv); }   // Windows：putenv("K=") 會把 K 拿掉

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_d015_a01_autologout -- golden [A01] idle logout (V912 main.cpp:25957-26096, cOffSet.cpp:2811); build: %s\n",
#ifdef SOFT_SIMULTE
                "SIM (SOFT_SIMULTE defined)"
#else
                "SHIPPING (SOFT_SIMULTE undefined)"
#endif
                );
    const char* const kReal = "D:\\HT9045\\config\\config.ini";
    const Meta realBefore = MetaOf(kReal);

    std::printf("[0] sandbox\n");
    const char* env = std::getenv("W906_A01CONFIGINI_PATH");
    const std::string global = env ? env : "";
    if (global.empty() || Lower(global) == Lower(kReal) || global.find_last_of("\\/") == std::string::npos) {
        std::printf("  FAIL: W906_A01CONFIGINI_PATH is not set to a sandbox path (tests/CMakeLists.txt _ht9045_env_extra sets it) -- refusing to run\n");
        return 1;
    }
    const std::string dir = global.substr(0, global.find_last_of("\\/")) + "/D015_A01AutoLogout";
    const std::string sandbox = dir + "/config.ini";
    _mkdir(global.substr(0, global.find_last_of("\\/")).c_str());
    _mkdir(dir.c_str());
    std::remove(sandbox.c_str());
    SetEnv((std::string("W906_A01CONFIGINI_PATH=") + sandbox).c_str());
    Check(!MetaOf(sandbox.c_str()).exists, "[0] sandbox config.ini removed before the run: " + sandbox);

    std::printf("[1] test seam\n");
    Check(std::string(W906_A01ConfigIniPath().c_str()) == sandbox, "[1] W906_A01ConfigIniPath() = W906_A01CONFIGINI_PATH");
    SetEnv("W906_A01CONFIGINI_PATH=");
    Check(std::string(W906_A01ConfigIniPath().c_str()) == std::string(AuthPath.c_str()) + "config.ini",
          "[1] unset / empty -> AuthPath+\"config.ini\" (golden main.cpp:26087; string only, not opened): " + std::string(W906_A01ConfigIniPath().c_str()));
    SetEnv((std::string("W906_A01CONFIGINI_PATH=") + sandbox).c_str());
    Check(std::string(W906_A01ConfigIniPath().c_str()) == sandbox, "[1] restored");

    std::printf("[2] cadence (golden Timer3 = 1000 ms)\n");
    Baseline();
    IniConfig.bA01AutoSwitchToOperatorMode = false;   // 節拍段不要有副作用
    AccessLevel = 0;
    W906_A01SetClockForTest(&FakeNow);
    {
        const unsigned p0 = W906_A01PassesForTest();
        g_now = 100000; W906_A01AutoLogoutTick();
        Check(W906_A01PassesForTest() == p0, "[2] the first call only arms (golden Timer3 fires 1 s after Enabled)");
        g_now = 100500; W906_A01AutoLogoutTick();
        const bool noHalf = W906_A01PassesForTest() == p0;
        g_now = 101000; W906_A01AutoLogoutTick();
        const bool one = W906_A01PassesForTest() == p0 + 1;
        g_now = 101500; W906_A01AutoLogoutTick();
        const bool stillOne = W906_A01PassesForTest() == p0 + 1;
        g_now = 102000; W906_A01AutoLogoutTick();
        Check(noHalf && one && stillOne && W906_A01PassesForTest() == p0 + 2, "[2] 500 ms beats: a pass at +1000 and +2000 only");

        // 40 個 500 ms 拍子、每拍晚 0..45 ms（主迴圈 50 ms 粒度）⇒ 20 秒剛好 20 次，不會因為「距上次 >=1000」拉長成 1.5 秒一次
        const unsigned p1 = W906_A01PassesForTest();
        const unsigned long base = g_now;
        for (int k = 1; k <= 40; ++k) { g_now = base + 500ul * (unsigned long)k + (unsigned long)((k * 17) % 46); W906_A01AutoLogoutTick(); }
        Check(W906_A01PassesForTest() - p1 == 20, "[2] 40 jittered 500 ms beats (20 s) -> exactly 20 passes, got " + Num((long)(W906_A01PassesForTest() - p1)));

        // 主迴圈被模態框擋 30 秒：只跑一次，下一次從現在起算 1 秒（不補 30 次）
        const unsigned p2 = W906_A01PassesForTest();
        g_now += 30000; W906_A01AutoLogoutTick();
        const bool once = W906_A01PassesForTest() == p2 + 1;
        g_now += 500; W906_A01AutoLogoutTick();
        const bool noBurst = W906_A01PassesForTest() == p2 + 1;
        g_now += 500; W906_A01AutoLogoutTick();
        Check(once && noBurst && W906_A01PassesForTest() == p2 + 2, "[2] a 30 s stall -> one pass, then 1 s later (missed seconds are not replayed) [W906]");

        // golden :25830-25831 if(InitialOK==false) return;
        InitialOK = false;
        W906_A01SetCountsForTest(5, 5);
        CosFunction.bAutoOpenConfigA01 = true;
        g_now += 1000; W906_A01AutoLogoutTick();
        Check(W906_A01OperatorModeCount() == 5 && W906_A01OpenCount() == 5, "[2] InitialOK==false -> the pass returns before any A01 step (counts untouched)");
        InitialOK = true;
        CosFunction.bAutoOpenConfigA01 = false;
    }
    W906_A01SetClockForTest(0);

    std::printf("[3] A01 off / Operator -> count 0 (golden :26075-26078)\n");
    Baseline();
    IniConfig.bA01AutoSwitchToOperatorMode = false;
    W906_A01SetCountsForTest(7, 0);
    W906_A01RunStepsForTest(2);
    Check(W906_A01OperatorModeCount() == 0, "[3] A01 off, AccessLevel 1 -> 0");
    IniConfig.bA01AutoSwitchToOperatorMode = true;
    AccessLevel = 0;
    W906_A01SetCountsForTest(7, 0);
    W906_A01RunStepsForTest(2);
    Check(W906_A01OperatorModeCount() == 0 && g_logouts == 0, "[3] A01 on, AccessLevel 0 -> 0, no logout");

    std::printf("[4] count and logout (golden :26022-26072)\n");
#ifdef SOFT_SIMULTE
    {
        Baseline();
        IniConfig.iA01ChangeOpTime = 10;
        bool allZero = true;
        for (int s = 1; s <= 30; ++s) { W906_A01RunStepsForTest(7); if (W906_A01OperatorModeCount() != 0) allZero = false; }
        AccessLevel = 3;
        for (int s = 1; s <= 30; ++s) { W906_A01RunStepsForTest(7); if (W906_A01OperatorModeCount() != 0) allZero = false; }
        Check(allZero && g_logouts == 0 && AccessLevel == 3,
              "[4] SIM: #ifdef SOFT_SIMULTE iOperatorModeCount=0 (golden :26026-26028) -> count stays 0 for 60 s at levels 1 and 3, the logout body is never called");
    }
#else
    {
        Baseline();
        IniConfig.iA01ChangeOpTime = 15;
        bool climb = true;
        for (int s = 1; s <= 14; ++s) { W906_A01RunStepsForTest(7); if (W906_A01OperatorModeCount() != s || g_logouts != 0) climb = false; }
        Check(climb, "[4] SHIP: seconds 1..14 count 1..14, no logout yet (iA01ChangeOpTime=15)");
        W906_A01RunStepsForTest(7);
        Check(g_logouts == 1 && AccessLevel == 0 && W906_A01OperatorModeCount() == 15,
              "[4] SHIP: second 15 -> the logout body runs once (golden :26038 count>=iA01ChangeOpTime); the body does not clear the count");
        Check(g_depthAtLogout == 0, "[4] SHIP: the FormJson lock is NOT held while the hook runs (the D4 body takes it itself)");
        W906_A01RunStepsForTest(7);
        Check(g_logouts == 1 && W906_A01OperatorModeCount() == 0, "[4] SHIP: next second AccessLevel 0 -> count 0 (golden :26077), no second logout");

        Baseline();
        IniConfig.iA01ChangeOpTime = 3;
        bool early = false;
        for (int s = 1; s <= 9; ++s) { W906_A01RunStepsForTest(7); if (g_logouts != 0) early = true; }
        W906_A01RunStepsForTest(7);
        Check(!early && g_logouts == 1, "[4] SHIP: iA01ChangeOpTime=3 still waits for 10 s (golden :26036 if(iOperatorModeCount>=10))");

        Baseline();
        IniConfig.iA01ChangeOpTime = 10;
        g_open.insert("fOffSet");                         // Offset 開著：非 KYEC 每秒歸 0 ⇒ 永遠到不了
        for (int s = 1; s <= 20; ++s) W906_A01RunStepsForTest(7);
        Check(g_logouts == 0 && W906_A01OperatorModeCount() == 1, "[4] SHIP: fOffSet showing (not KYEC) -> reset every second (golden :25979), never logs out");
        CUSTOMER_CODE = CC_KYEC_LEE;                      // KYEC：Offset 開著照數（golden :25974）
        W906_A01SetCountsForTest(0, 0);
        for (int s = 1; s <= 10; ++s) W906_A01RunStepsForTest(7);
        Check(g_logouts == 1 && g_prog.size() == 1 && g_prog[0].form == "fOffSet" && g_prog[0].open == false,
              "[4] SHIP: KYEC + fOffSet showing -> logout at 10 s and one program close of fOffSet (golden :26043-26044 via the page table)");

        Baseline();
        IniConfig.iA01ChangeOpTime = 10;
        W906_WebLoginForceOperatorHook = 0;
        for (int s = 1; s <= 12; ++s) W906_A01RunStepsForTest(7);
        Check(W906_A01OperatorModeCount() == 12 && AccessLevel == 1, "[4] SHIP: no hook installed -> no crash, nothing changes but the count (warned once)");
        W906_WebLoginForceOperatorHook = &FakeLogout;
    }
#endif

    std::printf("[5] resets while a page is showing (golden :25957-25982; step 1 only, both builds)\n");
    {
        struct Case { const char* what; int spil; int vtest; int cc; const char* open1; const char* open2; int member; int want; };
        //  member: 0 none, 2 fiosetview->fShow, 3 fObserver->bShow, 4 fContact->fShow（fSpeed 只問頁面表：Main_A01AutoLogout.cpp 檔頭 ⑦）
        const Case kCases[] = {
            {"nothing showing -> kept",                         0, 0, -1,                  nullptr,      nullptr,  0, 7},
            {"member fiosetview->fShow -> 0",                   0, 0, -1,                  nullptr,      nullptr,  2, 0},
            {"member fObserver->bShow -> 0",                    0, 0, -1,                  nullptr,      nullptr,  3, 0},
            {"member fContact->fShow -> 0",                     0, 0, -1,                  nullptr,      nullptr,  4, 0},
            {"page table fSpeed -> 0",                          0, 0, -1,                  "fSpeed",     nullptr,  0, 0},
            {"page table fiosetview -> 0",                      0, 0, -1,                  "fiosetview", nullptr,  0, 0},
            {"page table fObserver -> 0",                       0, 0, -1,                  "fObserver",  nullptr,  0, 0},
            {"page table fContact -> 0",                        0, 0, -1,                  "fContact",   nullptr,  0, 0},
            {"page table fOffSet -> 0",                         0, 0, -1,                  "fOffSet",    nullptr,  0, 0},
            {"another page (fSetup) -> kept",                   0, 0, -1,                  "fSetup",     nullptr,  0, 7},
            {"SPIL: fOffSet -> kept (not in the SPIL list)",    1, 0, -1,                  "fOffSet",    nullptr,  0, 7},
            {"SPIL: fSpeed -> 0",                               1, 0, -1,                  "fSpeed",     nullptr,  0, 0},
            {"VTEST: fOffSet -> kept",                          0, 1, -1,                  "fOffSet",    nullptr,  0, 7},
            {"VTEST: fContact member -> 0",                     0, 1, -1,                  nullptr,      nullptr,  4, 0},
            {"VTEST=2: golden ==true is false -> else branch, fOffSet -> 0", 0, 2, -1,     "fOffSet",    nullptr,  0, 0},
            {"SJ: fSpeed -> kept (never reset)",                0, 0, CC_SJ_Semiconductor, "fSpeed",     nullptr,  0, 7},
            {"SJ: fOffSet -> kept",                             0, 0, CC_SJ_Semiconductor, "fOffSet",    nullptr,  0, 7},
            {"SJ + SPIL: fSpeed -> 0 (the SPIL branch comes first)", 1, 0, CC_SJ_Semiconductor, "fSpeed", nullptr, 0, 0},
            {"KYEC: fOffSet -> kept (Offset page counts)",      0, 0, CC_KYEC_LEE,         "fOffSet",    nullptr,  0, 7},
            {"KYEC: fSpeed -> 0",                               0, 0, CC_KYEC_LEE,         "fSpeed",     nullptr,  0, 0},
            {"KYEC: fOffSet + fSpeed -> kept (the fOffSet arm wins)", 0, 0, CC_KYEC_LEE,   "fOffSet",    "fSpeed", 0, 7},
        };
        for (std::size_t i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
            const Case& c = kCases[i];
            Baseline();
            IniConfig.bSPILFunction = c.spil != 0;
            IniConfig.bVTESTFunction = c.vtest;
            CUSTOMER_CODE = c.cc;
            if (c.open1) g_open.insert(c.open1);
            if (c.open2) g_open.insert(c.open2);
            if (c.member == 2) fiosetview->fShow = true;
            if (c.member == 3) fObserver->bShow = true;
            if (c.member == 4) fContact->fShow = true;
            if (c.member != 0) W906_FormFShowHook = 0;   // ctest 的真實狀態：安裝座 0 ⇒ 成員決定
            W906_A01SetCountsForTest(7, 0);
            W906_A01RunStepsForTest(1);
            Check(W906_A01OperatorModeCount() == c.want, std::string("[5] ") + c.what + " (count " + Num(W906_A01OperatorModeCount()) + ")");
        }
        Baseline();
    }

    std::printf("[6] Offset save (golden cOffSet.cpp:2811 fMain->iOperatorModeCount=0)\n");
    W906_A01SetCountsForTest(7, 3);
    W906_A01ResetOperatorModeCount();
    Check(W906_A01OperatorModeCount() == 0 && W906_A01OpenCount() == 3, "[6] W906_A01ResetOperatorModeCount -> 0 (iOpenA01Count untouched)");

    std::printf("[7] A01 auto re-open (golden :26080-26096)\n");
    {
        Baseline();
        CosFunction.bAutoOpenConfigA01 = true;
        IniConfig.bA01AutoSwitchToOperatorMode = false;
        for (int s = 1; s <= 1799; ++s) W906_A01RunStepsForTest(4);
        Check(W906_A01OpenCount() == 1799 && !IniConfig.bA01AutoSwitchToOperatorMode && !MetaOf(sandbox.c_str()).exists,
              "[7] 1799 s with A01 off -> iOpenA01Count 1799, A01 still off, nothing written");
        W906_A01RunStepsForTest(4);
        std::string ini;
        const bool have = ReadAll(sandbox, &ini);
        Check(W906_A01OpenCount() == 1800 && IniConfig.bA01AutoSwitchToOperatorMode && have &&
              IniValue(ini, "Function", "bAutoSwitchToOperatorMode") == "1",
              "[7] second 1800 -> A01=true and the sandbox config.ini [Function] bAutoSwitchToOperatorMode=1 (value " +
              IniValue(ini, "Function", "bAutoSwitchToOperatorMode") + ")");
        W906_A01RunStepsForTest(4);
        Check(W906_A01OpenCount() == 0, "[7] A01 on -> iOpenA01Count 0 (golden :26094)");
        CosFunction.bAutoOpenConfigA01 = false;
        IniConfig.bA01AutoSwitchToOperatorMode = false;
        W906_A01SetCountsForTest(0, 5);
        W906_A01RunStepsForTest(4);
        const bool offKeep = W906_A01OpenCount() == 5;
        IniConfig.bA01AutoSwitchToOperatorMode = true;
        W906_A01RunStepsForTest(4);
        Check(offKeep && W906_A01OpenCount() == 5, "[7] CosFunction.bAutoOpenConfigA01 off -> iOpenA01Count untouched either way");
        Baseline();
    }

    std::printf("[8] wiring ratchet (source, read-only)\n");
    if (argc < 2) {
        Check(false, "[8] argv[1] (port tree root) missing");
    } else {
        const std::string root = argv[1];
        std::string wb, gen, py, cm, integ, me;
        const bool rd = ReadAll(root + "/tools/wb_serve.cpp", &wb) && ReadAll(root + "/FileRW/Offset_File.gen.inc", &gen) &&
                        ReadAll(root + "/tools/editlist/Offset_File.py", &py) && ReadAll(root + "/FileRW/_editlist_sources.cmake", &cm) &&
                        ReadAll(root + "/tools/editlist/_integrated.txt", &integ) && ReadAll(root + "/FileRW/Main_A01AutoLogout.cpp", &me);
        Check(rd, "[8] read wb_serve.cpp, Offset_File.gen.inc, Offset_File.py, _editlist_sources.cmake, _integrated.txt, Main_A01AutoLogout.cpp");
        const std::string call = "if (pumpBeat) { extern void W906_A01AutoLogoutTick(); W906_A01AutoLogoutTick(); }";
        // 棘輪自己的壞版本對照：接在 // 後面、包在 /* */ 裡都要被當成註解（不然這一條會假綠）
        Check(CodeOfLine("x;  // " + call).find(call) == std::string::npos &&
              CodeOfLine("x;  /* a // b */  y;  /* " + call + " */  z;").find(call) == std::string::npos &&
              CodeOfLine("x;  /* c */  " + call + "  // d").find(call) != std::string::npos,
              "[8] ratchet self-test: a call after // or inside /* */ is a comment, one between them is code");
        const std::string line = LineWith(wb, "W906_A01AutoLogoutTick();");
        const std::string code = CodeOfLine(line);
        Check(CodeCount(wb, call) == 1 && code.find(call) != std::string::npos &&
              code.find("W906_PageTableTick(SystemStart") != std::string::npos && code.find("W906_MainRecordTimer1Tick();") != std::string::npos,
              "[8] wb_serve.cpp: the pumpBeat call is code (not after // nor inside /* */), once, on St01's main-loop line (same line as MainRecord / PageTableTick)");
        Check(CodeCount(gen, "{ extern void W906_A01ResetOperatorModeCount(); W906_A01ResetOperatorModeCount(); }") == 1 &&
              CodeCount(gen, "ELTodo(\"golden cOffSet.cpp:2811") == 0 && gen.find("fMain->iOperatorModeCount=0;") != std::string::npos,
              "[8] Offset_File.gen.inc: OS_spbSaveClick calls the reset (code); the :2811 ELTodo is gone; golden's line survives only inside #if 0");
        Check(py.find("'{ extern void W906_A01ResetOperatorModeCount(); W906_A01ResetOperatorModeCount(); }'") != std::string::npos &&
              py.find("fMain->iOperatorModeCount=0 not done") == std::string::npos,
              "[8] tools/editlist/Offset_File.py carries the replacement (a full regen keeps it)");
        Check(cm.find("    FileRW/Main_A01AutoLogout.cpp\n") != std::string::npos && integ.find("\nMain_A01AutoLogout\n") != std::string::npos,
              "[8] FileRW/_editlist_sources.cmake lists the file; tools/editlist/_integrated.txt has the name");
        Check(CodeCount(me, "iOperatorModeCount=0;") >= 1 &&
              me.find("        #ifdef SOFT_SIMULTE\n        iOperatorModeCount=0;\n        #endif\n") != std::string::npos,
              "[8] Main_A01AutoLogout.cpp keeps golden :26026-26028 (#ifdef SOFT_SIMULTE iOperatorModeCount=0)");

        // AI(W906-D015-A01b) 20260930：阻塞框的等待迴圈（Jimmy 20260930：golden VCL Timer3 在 ShowModal 裡照跑）
        std::string modalLine;
        {
            std::istringstream in(wb);
            std::string l;
            bool next = false;
            while (std::getline(in, l)) {
                if (next) { modalLine = l; break; }
                if (l == "void W906_ModalWaitTick(int kind, int kcode)") next = true;   // 定義（:439 的宣告帶 "= 0"，不會對到）
            }
        }
        const std::string mcall = "{ extern void W906_A01AutoLogoutTick(); W906_A01AutoLogoutTick(); }";
        Check(CodeOfLine("{  { q(); }   // " + mcall).find(mcall) == std::string::npos,
              "[8] ratchet self-test: the modal call after the D-012 line's // would be a comment");
        Check(!modalLine.empty() && CodeOfLine(modalLine).find("W906_Q44ConsoleServiceTick();") != std::string::npos &&
              CodeOfLine(modalLine).find(mcall) != std::string::npos && CodeCount(wb, mcall) == 2,
              "[8] wb_serve.cpp W906_ModalWaitTick: the first line (St01's D-012 Q44 line) calls the A01 tick as code, before its //; "
              "two call sites in all (pumpBeat + modal wait)");
        // act.main.menuVisible：St01 那一行（act.main.* 的 W906_Main_EvB6Op 分派同一行）
        const std::string mvNeedle = "wc.cmd == \"act.main.menuVisible\"";
        const std::string dline = CodeOfLine(LineWith(wb, mvNeedle));
        Check(CodeCount(wb, mvNeedle) == 1 && dline.find(mvNeedle) != std::string::npos &&
              dline.find("W906_A01MenuVisibleOp(payload, &mvOk)") != std::string::npos &&
              dline.find("server.CompleteCommand((unsigned long long)wc.id, mvOk, mvRes);") != std::string::npos &&
              dline.find("W906_Main_EvB6Op(wc.cmd, payload, &b6Ok)") != std::string::npos,
              "[8] wb_serve.cpp: act.main.menuVisible is dispatched as code, once, to W906_A01MenuVisibleOp, on St01's act.main.* line");
        // W906_PrintDataRedirects（Jimmy 20260930：跟 eb60d61f 的 W906_AGVINI_PATH 一樣加在同一行）
        const std::string rline = CodeOfLine(LineWith(wb, "\"W906_AGVINI_PATH\", \"W906_AUTH_PATH\""));
        Check(rline.find("\"W906_A01CONFIGINI_PATH\", \"W906_AGVINI_PATH\", \"W906_AUTH_PATH\"") != std::string::npos &&
              wb.find("void W906_PrintDataRedirects()") != std::string::npos &&
              wb.find("void W906_PrintDataRedirects()") < wb.find("\"W906_A01CONFIGINI_PATH\", \"W906_AGVINI_PATH\"") &&
              me.find("std::getenv(\"W906_A01CONFIGINI_PATH\")") != std::string::npos,
              "[8] wb_serve.cpp W906_PrintDataRedirects lists W906_A01CONFIGINI_PATH as code (not in the /* */), the name Main_A01AutoLogout.cpp reads");
    }

    std::printf("[9] main-screen menus palSetup / palConfig (golden :25965 / :26045-26048 / ChangeLevelAttr :13183-13189; act.main.menuVisible)\n");
    {
        Baseline();
        const char* const kBad[] = { "", "{}", "not json", "[true,false]", "{\"setup\":true}", "{\"config\":false}",
                                     "{\"setup\":1,\"config\":false}", "{\"setup\":\"true\",\"config\":false}", "{\"setup\":true,\"config\":null}" };
        bool refused = true;
        for (std::size_t i = 0; i < sizeof(kBad) / sizeof(kBad[0]); ++i) {
            if (i == 3) MenuOp("{\"setup\":true,\"config\":true}");   // 後半段：旗標開著時壞訊框也不可以把它洗掉
            bool ok = true;
            const std::string r = W906_A01MenuVisibleOp(kBad[i], &ok);
            if (ok || r.compare(0, 11, "bad-payload") != 0 || W906_A01MenuStateForTest() != (i < 3 ? 0 : 3)) refused = false;
        }
        Check(refused, "[9] 9 bad payloads (empty, {}, not JSON, array, one key missing, number / string / null for a bool) -> ok=false bad-payload, flags untouched");
        bool ok = false;
        std::string r = W906_A01MenuVisibleOp("{\"setup\":true,\"config\":false}", &ok);
        Check(ok && W906_A01MenuStateForTest() == 1 && r.find("\"executed\":true") != std::string::npos &&
              r.find("\"setup\":true") != std::string::npos && r.find("\"config\":false") != std::string::npos,
              "[9] {setup:true,config:false} -> ok, Tools flag only; ack " + r.substr(0, 60));
        r = W906_A01MenuVisibleOp("{\"setup\":false,\"config\":true,\"extra\":1}", &ok);
        Check(ok && W906_A01MenuStateForTest() == 2, "[9] {setup:false,config:true,+extra key} -> Config flag only (extra keys ignored)");
        MenuOp("{\"setup\":false,\"config\":false}");
        Check(W906_A01MenuStateForTest() == 0, "[9] both false -> both flags off");

        // 第 1 段（mask 1）：計數 7 ⇒ 歸 0 或留著
        struct Case { const char* what; const char* menu; int screen; int spil; int cc; const char* open2; int noHook; int want; };
        const Case kCases[] = {
            {"Tools open, main screen showing -> 0",                              "{\"setup\":true,\"config\":false}",  1, 0, -1,                  nullptr,  0, 0},
            {"Config open -> 0",                                                  "{\"setup\":false,\"config\":true}",  1, 0, -1,                  nullptr,  0, 0},
            {"both open -> 0",                                                    "{\"setup\":true,\"config\":true}",   1, 0, -1,                  nullptr,  0, 0},
            {"both closed -> kept",                                               "{\"setup\":false,\"config\":false}", 1, 0, -1,                  nullptr,  0, 7},
            {"Tools open, main screen NOT showing (page table) -> kept [W906 stale guard]", "{\"setup\":true,\"config\":false}", 0, 0, -1,       nullptr,  0, 7},
            {"Config open, page-table hook not installed -> kept",                "{\"setup\":false,\"config\":true}",  1, 0, -1,                  nullptr,  1, 7},
            {"SJ: Tools open -> kept (golden :25967 never resets)",               "{\"setup\":true,\"config\":false}",  1, 0, CC_SJ_Semiconductor, nullptr,  0, 7},
            {"SPIL: Tools open -> kept (golden :25957-25964 SPIL list has no menus)", "{\"setup\":true,\"config\":true}", 1, 1, -1,                nullptr,  0, 7},
            {"KYEC: Config open -> 0 (fOffSet not showing, golden :25979)",       "{\"setup\":false,\"config\":true}",  1, 0, CC_KYEC_LEE,         nullptr,  0, 0},
            {"KYEC: Config open + fOffSet showing -> kept (the fOffSet arm wins, :25972-25975)", "{\"setup\":false,\"config\":true}", 1, 0, CC_KYEC_LEE, "fOffSet", 0, 7},
            {"not KYEC: Tools open + fOffSet showing -> 0",                       "{\"setup\":true,\"config\":false}",  1, 0, -1,                  "fOffSet", 0, 0},
        };
        for (std::size_t i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
            const Case& c = kCases[i];
            Baseline();
            IniConfig.bSPILFunction = c.spil != 0;
            CUSTOMER_CODE = c.cc;
            if (c.screen) g_open.insert("fMain");
            if (c.open2) g_open.insert(c.open2);
            if (c.noHook) W906_FormFShowHook = 0;
            const bool okc = MenuOp(c.menu);
            W906_A01SetCountsForTest(7, 0);
            W906_A01RunStepsForTest(1);
            Check(okc && W906_A01OperatorModeCount() == c.want, std::string("[9] ") + c.what + " (count " + Num(W906_A01OperatorModeCount()) + ")");
        }

        // 開著 ⇒ 每秒歸 0，30 秒都不會登出；關了照數
        Baseline();
        IniConfig.iA01ChangeOpTime = 15;
        g_open.insert("fMain");
        MenuOp("{\"setup\":true,\"config\":false}");
        bool held = true;
        for (int s = 1; s <= 30; ++s) {
            W906_A01RunStepsForTest(7);
#ifdef SOFT_SIMULTE
            if (W906_A01OperatorModeCount() != 0) held = false;
#else
            if (W906_A01OperatorModeCount() != 1) held = false;      // 第 1 段歸 0、第 2 段 +1
#endif
        }
        Check(held && g_logouts == 0 && AccessLevel == 1 && W906_A01MenuStateForTest() == 1,
              "[9] Tools menu open for 30 s -> reset every second, never logs out (golden :25965 + :26024)");
        MenuOp("{\"setup\":false,\"config\":false}");
        W906_A01SetCountsForTest(0, 0);
        int at = 0;
        for (int s = 1; s <= 20 && at == 0; ++s) { W906_A01RunStepsForTest(7); if (g_logouts == 1) at = s; }
#ifdef SOFT_SIMULTE
        Check(at == 0 && g_logouts == 0, "[9] SIM: menu closed -> still never logs out (#ifdef SOFT_SIMULTE)");
#else
        Check(at == 15, "[9] SHIP: menu closed -> the count climbs again, logout at second 15 (got " + Num(at) + ")");
#endif

        // 出貨：登出本體之後旗標清掉（golden :26045-26048）；SJ 開著選單也照數（:25967）
        Baseline();
        IniConfig.iA01ChangeOpTime = 10;
        CUSTOMER_CODE = CC_SJ_Semiconductor;
        g_open.insert("fMain");
        MenuOp("{\"setup\":true,\"config\":true}");
        for (int s = 1; s <= 10; ++s) W906_A01RunStepsForTest(7);
#ifdef SOFT_SIMULTE
        Check(g_logouts == 0 && W906_A01MenuStateForTest() == 3, "[9] SIM: SJ + both menus open, 10 s -> no logout, flags kept");
#else
        Check(g_logouts == 1 && AccessLevel == 0 && W906_A01MenuStateForTest() == 0,
              "[9] SHIP: SJ + both menus open -> logout at 10 s and both flags cleared (golden :26045-26048 palSetup/palConfig->Visible=false)");
        Baseline();
        IniConfig.iA01ChangeOpTime = 10;
        MenuOp("{\"setup\":true,\"config\":false}");          // 選單開著、主畫面不在（頁面表）⇒ 旗標不算 ⇒ 照數、照登出
        for (int s = 1; s <= 10; ++s) W906_A01RunStepsForTest(7);
        Check(g_logouts == 1 && W906_A01MenuStateForTest() == 0,
              "[9] SHIP: Tools flag on but no main screen (browser gone) -> not held, logout at 10 s, flag cleared [W906 stale guard]");
#endif

        // ChangeLevelAttr（:12929-12935＋:13183-13189）：權限比上一次低、而且是 0 ⇒ 收選單；兩種組態都跑（mask 0＝只看等級）
        Baseline();
        AccessLevel = 0;
        MenuOp("{\"setup\":true,\"config\":true}");
        W906_A01RunStepsForTest(0);
        const bool firstKept = W906_A01MenuStateForTest() == 3;   // golden static iOldAccessLevel=-999 ⇒ 第一次不收
        W906_A01RunStepsForTest(0);
        const bool zeroZeroKept = W906_A01MenuStateForTest() == 3;
        AccessLevel = 3; W906_A01RunStepsForTest(0);
        AccessLevel = 1; W906_A01RunStepsForTest(0);
        const bool dropNotZeroKept = W906_A01MenuStateForTest() == 3;
        AccessLevel = 0; W906_A01RunStepsForTest(0);
        Check(firstKept && zeroZeroKept && dropNotZeroKept && W906_A01MenuStateForTest() == 0,
              "[9] ChangeLevelAttr mirror: first sample at 0 kept, 0->0 kept, 3->1 kept, 1->0 -> both flags cleared");
        AccessLevel = 2; MenuOp("{\"setup\":false,\"config\":true}"); W906_A01RunStepsForTest(0);
        InitialOK = false;
        AccessLevel = 0; W906_A01RunStepsForTest(0);
        Check(W906_A01MenuStateForTest() == 0, "[9] the level-drop close is not gated by InitialOK (ChangeLevelAttr is not in Timer3)");
        InitialOK = true;
        Baseline();
    }

    std::printf("[10] re-entrancy guard (a box opened inside the tick -> W906_ModalWaitTick -> the tick again)\n");
    {
        Baseline();
        IniConfig.bA01AutoSwitchToOperatorMode = false;       // 沒有副作用
        AccessLevel = 0;
        W906_A01SetClockForTest(&FakeNow);
        g_now = 500000; W906_A01AutoLogoutTick();              // 起算
        g_now += 1000;
        W906_FormFShowHook = &ReentrantFShow;
        g_reenterArmed = true;
        const unsigned p0 = W906_A01PassesForTest(), r0 = W906_A01ReentriesForTest();
        W906_A01AutoLogoutTick();                              // 外層這一次到期；頁面表問答時巢狀叫一次（也到期了）
        const bool oneOuter = W906_A01PassesForTest() == p0 + 1 && W906_A01ReentriesForTest() == r0 + 1 && !g_reenterArmed;
        W906_A01AutoLogoutTick();                              // 巢狀那一次把時間推到了外層的下一個截止 ⇒ 這一次照常跑（保險已放開）
        Check(oneOuter && W906_A01PassesForTest() == p0 + 2 && W906_A01ReentriesForTest() == r0 + 1,
              "[10] the nested tick returns at once (no second pass in the same tick, deadline untouched); the next tick runs normally");
        W906_FormFShowHook = &FakeFShow;
        W906_A01SetClockForTest(0);
        Baseline();
    }

    W906_WebLoginForceOperatorHook = 0;
    W906_FormFShowHook = 0;
    W906_FormProgramShowHook = 0;
    const Meta realAfter = MetaOf(kReal);
    Check(SameMeta(realBefore, realAfter), std::string("[guard] real ") + kReal + (realAfter.exists ? " unchanged (size / mtime)" : " still absent"));
    std::printf("test_d015_a01_autologout: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
