// =============================================================================
//  test_d025_menuopen.cpp -- todo D-025：主畫面 Tools ▾／Config ▾ 選單鈕按下去（golden TfMain::sbSettingClick／sbConfigClick）
//
//  //AI(W906-D025) 20261001 [W906] St01 新檔。golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\main.cpp（cp950）：
//    :29030-29049 sbSettingClick：if(SystemStart) return; → Insufficient(0) → ProceeToolBar／palSetup 顯示 → EventReport(EnterTool)（bEnable_SECS_GEM）
//    :29009-29028 sbConfigClick ：if(SystemStart) return; → Insufficient(1) → ProceeToolBar／palConfig 顯示 → SetWorkParameter →
//                                 UpdateMainOperateMode → EventReport(EnterConfig)（bEnable_SECS_GEM）
//    按鈕 Enabled：ChangeLevelAttr :12951／:12952（authMainForm[0]／[1]＋等級；運轉中 :12937-12940 灰）。
//  受測的是 wb_serve 編進去的同一份 FileRW/Main_D025MenuOpen.cpp（W906_Main_MenuOpenOp，WS act.main.menuOpen）＋ FileRW/_EditPage.cpp
//    檔尾 filerw::MenuClickRefused（＝開窗閘的 GToolsMenu／GConfigMenu）；god-stack 用 RESCAN 連（fSecurity、LevelSet、authMainForm、
//    fMain、EventReport 都是活本體）。
//    [1] value 不對 ⇒ bad-payload、什麼都不跑
//    [2] Tools 放行：只送 EnterTool（SECS 開時），SetWorkParameter／UpdateMainOperateMode 都不跑（golden sbSettingClick 沒有）；SECS 關時什麼尾段都沒有
//    [3] Config 放行：SetWorkParameter → UpdateMainOperateMode → EventReport(EnterConfig) 照這個順序各一次；SECS 關時不送；
//        尾段在 FormJson 鎖裡面跑、跑完鎖放掉
//    [4] 運轉中（SystemStart）兩顆都拒（running），尾段一個都沒跑；只有 SoftStart 照 golden 放行
//    [5] 按鈕灰的（authMainForm[0]／[1] 關）⇒ disabled；等級不夠 ⇒ not-authorized（理由帶 golden 出處），尾段沒跑；等級夠了就放行
//    [6] 第二下（選單已經開著）照 golden 整段重跑：尾段再跑一次（開不開選單是頁面的事）
//    [7] 原始碼棘輪（argv[1]＝移植樹根目錄、argv[2]＝web\page，唯讀）：wb_serve.cpp 的分派在 act.main.menuVisible 那一行、
//        _editlist_sources.cmake／_integrated.txt 有登記、頁面在 window capture 攔下兩顆鈕、送 act.main.menuOpen
//  SetWorkParameter 經測試縫 W906_MenuOpenSetWorkParameter 換成記錄器（真本體會重讀 teach.ini 等機台檔）；UpdateMainOperateMode 在 ctest 裡
//  是計數樁（forms/fMain.cpp:507，真本體只有 wb_serve 開機才裝），這裡另裝記錄用的 hook 看順序；EventReport 是移植樹的模擬計數
//  （SECSGEM/SecsEventReport.cpp）。每個 case 開頭 Baseline() 把旗標、等級、計數歸回起點。不寫任何檔。
// =============================================================================
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "forms/fMain.h"
#include "SECSGEM/SecsEventType.h"
#include "SECSGEM/SecsEventReport.h"
#include "Public/cJSON.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::string W906_Main_MenuOpenOp(const std::string& payloadJson, bool* ok);   // FileRW/Main_D025MenuOpen.cpp
extern bool (*W906_MenuOpenSetWorkParameter)();                               // 同上（測試縫）
extern void (*W906_UpdateMainOperateModeHook)(TfMain*);                        // forms/fMain.cpp:507
extern bool authMainForm[12];                                                  // cAuthority.cpp:119
extern int iMaxLevelItem;                                                      // cSecurity.cpp:47

// 連結用（不是受測碼）：FileRW_IniConfig_ChangeCBListProperty 同 tests/test_b8_os5_sortbuttons.cpp；FormJson 鎖只在 wb_serve ⇒ 這裡記深度。
void FileRW_IniConfig_ChangeCBListProperty() {}
static int g_lockDepth = 0, g_lockTaken = 0;
namespace ht9045 {
namespace formjson {
void FormLock() { ++g_lockDepth; ++g_lockTaken; }
void FormUnlock() { --g_lockDepth; }
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
bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    const std::string raw = ss.str();   // AI(W906-D025) 20261001: drop '\r' so the ratchet needles (e.g. "...cpp\n") match a CRLF checkout too (gate worktree core.autocrlf=true; ST01-M e2c30aed gate; gotcha d3d85db6 / 29d088be)
    out->clear(); out->reserve(raw.size());
    for (std::string::size_type i = 0; i < raw.size(); ++i) if (raw[i] != '\r') out->push_back(raw[i]);
    return true;
}

// 尾段的順序紀錄：S＝SetWorkParameter、U＝UpdateMainOperateMode、E<ceid>＝EventReport（在下一個呼叫時看 g_SimEventReportCount 的變化）
std::string N(unsigned v) { char b[24]; std::snprintf(b, sizeof(b), "%u", v); return b; }   // MinGW 6.3：不用 std::to_string
std::string g_seq;
unsigned long g_evSeen = 0;
int g_swp = 0, g_swpLockDepth = -1, g_umom = 0;
void NoteEvents()
{
    while (g_evSeen < g_SimEventReportCount) { ++g_evSeen; g_seq += "E" + N(g_SimLastEventReportCeid); }
}
bool FakeSetWorkParameter() { NoteEvents(); ++g_swp; g_swpLockDepth = g_lockDepth; g_seq += "S"; return true; }
void FakeUmomHook(TfMain*) { NoteEvents(); ++g_umom; g_seq += "U"; }

void Baseline()
{
    SystemStart = false; SoftStart = false;
    IniConfig.bEnable_SECS_GEM = false;
    iMaxLevelItem = 180;                                  // 開機後的值（W906_SecurityBoot；golden cSecurity.cpp:210）
    AccessLevel = 0;
    LevelSet.AccessLevel[0] = 0; LevelSet.AccessLevel[1] = 0;
    authMainForm[0] = true; authMainForm[1] = true;
    W906_MenuOpenSetWorkParameter = &FakeSetWorkParameter;
    W906_UpdateMainOperateModeHook = &FakeUmomHook;
    ResetSimEventReport();
    g_evSeen = 0; g_seq.clear(); g_swp = 0; g_swpLockDepth = -1; g_umom = 0; g_lockDepth = 0; g_lockTaken = 0;
}
std::string Op(const char* menu, bool* ok)
{
    const std::string r = W906_Main_MenuOpenOp(std::string("{\"menu\":\"") + menu + "\"}", ok);
    NoteEvents();
    return r;
}
std::string Str(const std::string& json, const char* key)
{
    cJSON* o = cJSON_Parse(json.c_str());
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    const std::string s = v && cJSON_IsString(v) ? v->valuestring : std::string();
    if (o) cJSON_Delete(o);
    return s;
}
bool Allow(const std::string& json)
{
    cJSON* o = cJSON_Parse(json.c_str());
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, "allow") : nullptr;
    const bool a = v && cJSON_IsTrue(v);
    if (o) cJSON_Delete(o);
    return a;
}
bool CodeHas(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) {
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) return true;
    }
    return false;
}

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_d025_menuopen -- D-025 golden TfMain::sbSettingClick / sbConfigClick (V912 main.cpp:29030 / :29009)\n");
    const std::string kTool = "E" + N(SECS_EVENT.EnterTool), kConf = "E" + N(SECS_EVENT.EnterConfig);
    Check(SECS_EVENT.EnterTool == 17 && SECS_EVENT.EnterConfig == 18, "[0] SECS_EVENT.EnterTool 17 / EnterConfig 18 (golden comments main.cpp:29048 / :29027)");
    bool ok = false;
    std::string r;

    std::printf("[1] bad value\n");
    Baseline();
    r = W906_Main_MenuOpenOp("{\"menu\":\"debug\"}", &ok);
    Check(!ok && r.find("bad-payload") == 0 && g_seq.empty() && g_lockTaken == 0, "[1] menu=debug -> bad-payload, nothing ran, no lock");
    r = W906_Main_MenuOpenOp("", &ok);
    Check(!ok && r.find("bad-payload") == 0, "[1] empty value -> bad-payload");

    std::printf("[2] Tools (sbSettingClick): only EnterTool\n");
    Baseline();
    r = Op("setup", &ok);
    Check(ok && Allow(r) && g_seq.empty() && g_swp == 0 && g_umom == 0, "[2] SECS off: allow, no tail at all (seq '" + g_seq + "')");
    Baseline();
    IniConfig.bEnable_SECS_GEM = true;
    r = Op("setup", &ok);
    Check(ok && Allow(r) && g_seq == kTool && g_swp == 0 && g_umom == 0,
          "[2] SECS on: allow, EventReport(EnterTool) once, no SetWorkParameter / UpdateMainOperateMode (seq '" + g_seq + "')");
    Check(Str(r, "golden").find("main.cpp:29030-29049") != std::string::npos, "[2] ack golden cites sbSettingClick :29030-29049");

    std::printf("[3] Config (sbConfigClick): SetWorkParameter -> UpdateMainOperateMode -> EventReport(EnterConfig)\n");
    Baseline();
    IniConfig.bEnable_SECS_GEM = true;
    const int umom0 = fMain->W906_UpdateMainOperateModeCallCount;
    r = Op("config", &ok);
    Check(ok && Allow(r) && g_seq == "SU" + kConf, "[3] SECS on: order S, U, " + kConf + " (seq '" + g_seq + "')");
    Check(g_swp == 1 && g_umom == 1 && fMain->W906_UpdateMainOperateModeCallCount == umom0 + 1 && g_SimEventReportCount == 1,
          "[3] each exactly once (UpdateMainOperateMode through fMain, the ctest counting stub + hook)");
    Check(g_swpLockDepth == 1 && g_lockDepth == 0 && g_lockTaken == 1, "[3] tail ran inside the FormJson lock, lock released afterwards");
    Baseline();
    r = Op("config", &ok);
    Check(ok && Allow(r) && g_seq == "SU" && g_SimEventReportCount == 0, "[3] SECS off: S, U, no EventReport (seq '" + g_seq + "')");

    std::printf("[4] running\n");
    Baseline();
    IniConfig.bEnable_SECS_GEM = true;
    SystemStart = true;
    r = Op("setup", &ok);
    Check(!ok && !Allow(r) && Str(r, "guard") == "running" && Str(r, "golden").find(":29033-29034") != std::string::npos && g_seq.empty(),
          "[4] SystemStart: Tools refused (running, golden :29033-29034), nothing ran");
    r = Op("config", &ok);
    Check(!ok && Str(r, "guard") == "running" && Str(r, "golden").find(":29012-29013") != std::string::npos && g_seq.empty() && g_swp == 0,
          "[4] SystemStart: Config refused (running, golden :29012-29013), SetWorkParameter not run");
    SystemStart = false; SoftStart = true;
    r = Op("config", &ok);
    Check(ok && Allow(r) && g_seq == "SU" + kConf, "[4] SoftStart only: allowed (golden checks SystemStart only)");

    std::printf("[5] button disabled / level\n");
    Baseline();
    IniConfig.bEnable_SECS_GEM = true;
    authMainForm[1] = false;
    r = Op("config", &ok);
    Check(!ok && Str(r, "guard") == "disabled" && g_seq.empty(), "[5] authMainForm[1] off -> disabled, nothing ran (" + Str(r, "golden").substr(0, 40) + ")");
    authMainForm[1] = true; authMainForm[0] = false;
    r = Op("setup", &ok);
    Check(!ok && Str(r, "guard") == "disabled" && g_seq.empty(), "[5] authMainForm[0] off -> Tools disabled");
    Baseline();
    IniConfig.bEnable_SECS_GEM = true;
    LevelSet.AccessLevel[1] = 2; AccessLevel = 1;
    r = Op("config", &ok);
    Check(!ok && Str(r, "guard") == "not-authorized" && Str(r, "golden").find("Insufficient(1)") != std::string::npos && g_seq.empty() && g_swp == 0,
          "[5] Config needs level 2, logged in 1 -> not-authorized (golden :29015 Insufficient(1)), nothing ran");
    r = Op("setup", &ok);
    Check(ok && g_seq == kTool, "[5] same login, Tools needs 0 -> allowed");
    AccessLevel = 2; g_seq.clear();
    r = Op("config", &ok);
    Check(ok && g_seq == "SU" + kConf, "[5] level 2 -> Config allowed");

    std::printf("[6] second click while open: golden runs the whole body again\n");
    Baseline();
    IniConfig.bEnable_SECS_GEM = true;
    Op("config", &ok);
    r = Op("config", &ok);
    Check(ok && Allow(r) && g_swp == 2 && g_umom == 2 && g_seq == "SU" + kConf + "SU" + kConf, "[6] twice -> tail twice (seq '" + g_seq + "')");

    std::printf("[7] source ratchets (read-only)\n");
    if (argc >= 3) {
        const std::string root = argv[1], web = argv[2];
        std::string wb, cm, it, js;
        bool sameLine = false;
        if (ReadAll(root + "/tools/wb_serve.cpp", &wb)) {
            std::istringstream in(wb);
            std::string l;
            while (std::getline(in, l))
                if (l.find("wc.cmd == \"act.main.menuVisible\"") != std::string::npos &&
                    l.find("wc.cmd == \"act.main.menuOpen\"") != std::string::npos && l.find("W906_Main_MenuOpenOp(payload, &moOk)") != std::string::npos)
                    sameLine = true;
        }
        Check(sameLine, "[7] tools/wb_serve.cpp: act.main.menuOpen dispatch on the act.main.menuVisible line (same line, St01 :4826)");
        Check(ReadAll(root + "/FileRW/_editlist_sources.cmake", &cm) && cm.find("    FileRW/Main_D025MenuOpen.cpp\n") != std::string::npos &&
              ReadAll(root + "/tools/editlist/_integrated.txt", &it) && it.find("\nMain_D025MenuOpen") != std::string::npos,
              "[7] registered: _editlist_sources.cmake + _integrated.txt (kept by a full gen_editlist.py run)");
        Check(ReadAll(web + "/ht9045_main_st01_ev.js", &js) && CodeHas(js, "window.addEventListener('click', onClick, true)") &&
              CodeHas(js, "var CMD = 'act.main.menuOpen'") && CodeHas(js, "initMenuReport(); initMenuOpen();"),
              "[7] web/page/ht9045_main_st01_ev.js: capture-phase interception + act.main.menuOpen, started next to initMenuReport");
    } else {
        Check(false, "[7] argv: <port root> <web/page>");
    }

    W906_MenuOpenSetWorkParameter = nullptr;
    W906_UpdateMainOperateModeHook = nullptr;
    std::printf("test_d025_menuopen: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
