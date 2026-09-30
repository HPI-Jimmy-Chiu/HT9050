// =============================================================================
//  test_b8_ag1_agvini.cpp -- B8 AG-1 (A)：AMR Setting 頁（golden TfAGV）讀寫 D:\HT9045\config\AGV.ini，走沙盒。
//
//  //AI(W906-B8-AG1) 20260930 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「AG-1」
//    （Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp spbSaveClick :1184-1225、ReadFile :1227-1262、
//    DoIniDataToForm :1264-1295、FormShow :1304-1309；開窗 main.cpp:35776-35781 spbAGVClick（:35779 MES2198）、:24333 {spbAGV, USE_E84_Sensor}。
//  受測的是 wb_serve 編進去的同一份 FileRW/TestIF_File_AGV.cpp＋產生檔 FileRW/TestIF_File_AGV.gen.inc＋FileRW/_EditPage.cpp（PageJson／PageSave／
//    開窗閘／開窗記錄），god-stack 用 RESCAN 連（同 ctest B8_Os5_SortButtons）。
//    [0] 沙盒：W906_AGVINI_PATH 有設、不是真的 D:\HT9045\config\AGV.ini（沒有就拒跑）；沙盒檔先刪掉
//    [1] 測試縫：W906_AgvIniPath()＝環境變數；沒設或空字串＝golden 字面值 "D:\HT9045\config\AGV.ini"（只比字串，不開檔）
//    [2] 開機：頁面登記（tag、form、page、26 個 mustSend）、冪等
//    [3] 開窗閘：AGVModal=0（USE_E84_Sensor==0）⇒ hidden；=1 ⇒ 放行；SystemStart ⇒ running；開窗記錄 MES2198 "Enter AGV Form"；
//        //AI(W906-AUTHMAINFORM) 20260930：Security_new.def [Main] Tool=0（authMainForm[0]，golden ChangeLevelAttr main.cpp:12951）⇒ disabled
//    [4] 開頁（golden FormShow）沒有 AGV.ini：TestIF_File 全是 golden 預設值（2,2,60,60,2,2,2,120,60,60,60；0,0,0；false），
//        畫面 26 格＝預設值；讀檔不建檔
//    [5] 存檔（golden spbSaveClick）：26 鍵照 golden 的鍵名寫進沙盒、值＝atoi(畫面文字)、E84 Enable 1／0；之後 ReadFile 把值讀回 TestIF_File
//        ⇒ bEnableE84 跟著勾選（E84 交握的開關，讀者 csystem.cpp:30394；[7] 釘住）
//    [6] 頁面少送一格（mustSend）⇒ 400、一個鍵都不寫；golden atoi：「12x」⇒12、「abc」⇒0
//    [7] 接上了沒（原始碼棘輪，argv[1]＝移植樹根目錄、argv[2]＝D:\HT9045\web\page，唯讀）：產生檔兩處路徑走測試縫（golden 字面值只在 #if 0）、
//        E84 讀者 csystem.cpp:30394、wb_serve 開機建替身（行尾 // 之前）、_editlist_sources.cmake／_integrated.txt、頁面載入補件
//  NOT COVERED：WS editlist.get／save 在 wb_serve 的分派（整合者用探針驗）；E84 交握本身（ctest AGV_E84／AGV_PortScan）；開機讀檔（patch B）。
//  不碰真的 AGV.ini：只看它在不在、大小與修改時間（開跑前後相同），不開、不讀內容。
// =============================================================================
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"

#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>   // AI(W906-B8-AG1-CRLF) 20260930: std::remove (line-ending-agnostic source ratchet in [7])
#include <vector>
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
AnsiString W906_AgvIniPath();
extern bool authMainForm[12];   // cAuthority.h:56（本體 cAuthority.cpp:119；開機 GetMainAuth 填，本測試不跑它）。//AI(W906-AUTHMAINFORM) 20260930：[3] 用

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

// 真檔只看 metadata（不開檔）
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

// 沙盒 AGV.ini 的 [Configuration] 某個鍵（原樣字串；沒有回 "<none>"）
std::string IniValue(const std::string& text, const std::string& key)
{
    std::istringstream in(text);
    std::string l;
    bool sec = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (!l.empty() && l[0] == '[') { sec = (l == "[Configuration]"); continue; }
        if (!sec) continue;
        const std::string::size_type eq = l.find('=');
        if (eq != std::string::npos && l.substr(0, eq) == key) return l.substr(eq + 1);
    }
    return "<none>";
}

// golden 的 26 個鍵（spbSaveClick :1189-1221 的順序）、對應的畫面格與 TestIF_File 欄位
struct Key { const char* key; const char* widget; int row; int col; int dflt; };
const Key kKeys[25] = {
    {"E84_1 Time Out 1",  "edE84_1_TP1", 0, 0, 2},   {"E84_1 Time Out 2",  "edE84_1_TP2", 0, 1, 2},
    {"E84_1 Time Out 3",  "edE84_1_TP3", 0, 2, 60},  {"E84_1 Time Out 4",  "edE84_1_TP4", 0, 3, 60},
    {"E84_1 Time Out 5",  "edE84_1_TP5", 0, 4, 2},   {"E84_1 Time Out 6",  "edE84_1_TP6", 0, 5, 2},
    {"E84_1 Time Out 7",  "edE84_1_TA1", 0, 6, 2},   {"E84_1 Time Out 8",  "edE84_1_TA2", 0, 7, 120},
    {"E84_1 Time Out 9",  "edE84_1_TA3", 0, 8, 60},  {"E84_1 Time Out 10", "edE84_1_TD0", 0, 9, 60},
    {"E84_1 Time Out 11", "edE84_1_TD1", 0, 10, 60},
    {"E84_2 Time Out 1",  "edE84_2_TP1", 1, 0, 2},   {"E84_2 Time Out 2",  "edE84_2_TP2", 1, 1, 2},
    {"E84_2 Time Out 3",  "edE84_2_TP3", 1, 2, 60},  {"E84_2 Time Out 4",  "edE84_2_TP4", 1, 3, 60},
    {"E84_2 Time Out 5",  "edE84_2_TP5", 1, 4, 2},   {"E84_2 Time Out 6",  "edE84_2_TP6", 1, 5, 2},
    {"E84_2 Time Out 7",  "edE84_2_TA1", 1, 6, 2},   {"E84_2 Time Out 8",  "edE84_2_TA2", 1, 7, 120},
    {"E84_2 Time Out 9",  "edE84_2_TA3", 1, 8, 60},  {"E84_2 Time Out 10", "edE84_2_TD0", 1, 9, 60},
    {"E84_2 Time Out 11", "edE84_2_TD1", 1, 10, 60},
    {"Auto1 Tray Count",  "edAuto1Count", -1, 0, 0}, {"Auto2 Tray Count",  "edAuto2Count", -1, 1, 0},
    {"Auto3 Tray Count",  "edAuto3Count", -1, 2, 0},
};
int Mem(const Key& k) { return k.row < 0 ? TestIF_File.iLoaderUnloaderTrayCount[k.col] : TestIF_File.iE84TimeOut_K12[k.row][k.col]; }
void SetMem(const Key& k, int v) { if (k.row < 0) TestIF_File.iLoaderUnloaderTrayCount[k.col] = v; else TestIF_File.iE84TimeOut_K12[k.row][k.col] = v; }

std::string Num(int v) { char b[24]; std::snprintf(b, sizeof(b), "%d", v); return b; }

// editlist.save 的 widgets：每格 text、勾選框 checked；skip＝不送的那一格
std::string Widgets(const std::vector<std::string>& text, bool enable, const char* skip)
{
    std::string w = "{";
    for (int i = 0; i < 25; ++i) {
        if (skip && std::strcmp(skip, kKeys[i].widget) == 0) continue;
        w += std::string(w.size() > 1 ? "," : "") + "\"" + kKeys[i].widget + "\":{\"text\":\"" + text[i] + "\"}";
    }
    if (!(skip && std::strcmp(skip, "cbEnableAGVFunction") == 0))
        w += std::string(w.size() > 1 ? "," : "") + "\"cbEnableAGVFunction\":{\"checked\":" + (enable ? "true" : "false") + "}";
    return w + "}";
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
int CodeCount(const std::string& text, const std::string& needle)
{
    std::istringstream in(text);
    std::string l;
    bool gated = false;
    int n = 0;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        const std::string::size_type p = l.find("//");
        if ((p == std::string::npos ? l : l.substr(0, p)).find(needle) != std::string::npos) ++n;
    }
    return n;
}

void SetEnv(const char* kv) { HT9045_TEST_PUTENV(kv); }   // Windows：putenv("K=") 會把 K 拿掉

}  // namespace

int main(int argc, char** argv)
{
    std::printf("test_b8_ag1_agvini -- B8 AG-1 (A) golden TfAGV AGV.ini read / save (V912 Automation/AGV.cpp:1184-1309)\n");
    const char* const kReal = "D:\\HT9045\\config\\AGV.ini";
    const Meta realBefore = MetaOf(kReal);

    std::printf("[0] sandbox\n");
    const char* env = std::getenv("W906_AGVINI_PATH");
    const std::string global = env ? env : "";
    if (global.empty() || Lower(global) == Lower(kReal) || global.find_last_of("\\/") == std::string::npos) {
        std::printf("  FAIL: W906_AGVINI_PATH is not set to a sandbox path (tests/CMakeLists.txt _ht9045_env_extra sets it) -- refusing to run\n");
        return 1;
    }
    //   ctest -j 會同時跑 B8_Ag1_Initial（同一個沙盒根）⇒ 這一支用沙盒底下自己的資料夾
    const std::string dir = global.substr(0, global.find_last_of("\\/")) + "/B8_Ag1_AgvIni";
    const std::string sandbox = dir + "/AGV.ini";
    _mkdir(global.substr(0, global.find_last_of("\\/")).c_str());
    _mkdir(dir.c_str());
    std::remove(sandbox.c_str());
    SetEnv((std::string("W906_AGVINI_PATH=") + sandbox).c_str());
    Check(!MetaOf(sandbox.c_str()).exists, "[0] sandbox AGV.ini removed before the run: " + sandbox);

    std::printf("[1] test seam\n");
    Check(std::string(W906_AgvIniPath().c_str()) == sandbox, "[1] W906_AgvIniPath() = W906_AGVINI_PATH");
    SetEnv("W906_AGVINI_PATH=");
    Check(std::string(W906_AgvIniPath().c_str()) == kReal, "[1] unset / empty -> golden literal D:\\HT9045\\config\\AGV.ini (string only, not opened)");
    SetEnv((std::string("W906_AGVINI_PATH=") + sandbox).c_str());
    Check(std::string(W906_AgvIniPath().c_str()) == sandbox, "[1] restored");

    std::printf("[2] boot\n");
    SystemStart = false; SoftStart = false;
    FileRW_AGV_Boot();
    FileRW_AGV_Boot();
    const filerw::PageDesc* d = filerw::FindPage("TestIF_File_AGV");
    Check(d && std::strcmp(d->form, "TfAGV") == 0 && std::strcmp(d->page, "Setup.AGV.html") == 0 && d->nSaveReads == 26 &&
          std::strcmp(d->savedMark, "spbSaveClick") == 0 && d->booted(),
          "[2] PageDesc TestIF_File_AGV / TfAGV / Setup.AGV.html, 26 save reads, savedMark spbSaveClick, booted (twice = idempotent)");
    if (!d) { std::printf("test_b8_ag1_agvini: %d passed, %d failed\n", g_pass, g_fail + 1); return 1; }

    std::printf("[3] open gate + Enter record\n");
    {
        std::string why;
        //AI(W906-AUTHMAINFORM) 20260930：工具選單鈕 sbSetting->Enabled 另要 authMainForm[0]（golden ChangeLevelAttr main.cpp:12951，Security_new.def
        //   [Main] Tool；FileRW/_EditPage.cpp GToolsMenu）。本測試不跑 GetMainAuth ⇒ 先設成 golden 預設 1；0 ⇒ disabled（比「鈕看不見」先講）
        const bool auth0 = authMainForm[0];
        authMainForm[0] = false;
        USE_E84_Sensor = 1;
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && why.compare(0, 9, "disabled:") == 0 &&
              why.find("main.cpp:12951") != std::string::npos && why.find("[Main] Tool=0") != std::string::npos,
              "[3] Security_new.def [Main] Tool=0 -> disabled (golden sbSetting->Enabled=(... && authMainForm[0]), main.cpp:12951) " + why);
        authMainForm[0] = true;
        why.clear();
        USE_E84_Sensor = 0;
        Check(filerw::OpenGateRefused("TestIF_File_AGV", false, &why) && why.compare(0, 7, "hidden:") == 0 &&
              why.find("main.cpp:24333") != std::string::npos, "[3] AGVModal=0 -> hidden (golden spbAGV not visible; reason cites main.cpp:24333)");
        USE_E84_Sensor = 1;
        why.clear();
        Check(!filerw::OpenGateRefused("TestIF_File_AGV", false, &why), std::string("[3] AGVModal=1, stopped, level ok -> open allowed") + (why.empty() ? "" : " (unexpected reason)"));
        SystemStart = true;
        Check(filerw::OpenGateRefused("TestIF_File_AGV", true, &why) && why.compare(0, 8, "running:") == 0,
              "[3] SystemStart -> running (tools menu sbSettingClick if(SystemStart) return;)");
        SystemStart = false;
        const filerw::OpenEnter* e = filerw::FindOpenEnter("TestIF_File_AGV");
        Check(e && std::strcmp(e->code, "MES2198") == 0 && std::strcmp(e->text, "Enter AGV Form") == 0 &&
              std::strcmp(e->golden, "main.cpp:35779") == 0, "[3] open record = golden :35779 NewRecordProcess(\"MES2198\", \"Enter AGV Form\")");
        authMainForm[0] = auth0;   //AI(W906-AUTHMAINFORM) 20260930
    }

    std::printf("[4] open (golden FormShow) without an AGV.ini\n");
    for (int i = 0; i < 25; ++i) SetMem(kKeys[i], -7);
    TestIF_File.bEnableE84 = true;
    std::string json;
    Check(filerw::PageJson(*d, &json) == 200, "[4] editlist.get -> 200");
    {
        int ok = 0;
        for (int i = 0; i < 25; ++i) {
            const bool m = Mem(kKeys[i]) == kKeys[i].dflt;
            const bool w = std::string(EL<TEdit>("TfAGV", kKeys[i].widget)->Text.c_str()) == Num(kKeys[i].dflt);
            if (m && w) ++ok; else std::printf("    %s: mem %d widget %s want %d\n", kKeys[i].key, Mem(kKeys[i]), EL<TEdit>("TfAGV", kKeys[i].widget)->Text.c_str(), kKeys[i].dflt);
        }
        Check(ok == 25, "[4] 25 values = golden ReadFile defaults in TestIF_File and on the page (DoIniDataToForm)");
        Check(!TestIF_File.bEnableE84 && !EL<TCheckBox>("TfAGV", "cbEnableAGVFunction")->Checked, "[4] bEnableE84 = golden default false, checkbox off");
        cJSON* root = cJSON_Parse(json.c_str());
        const cJSON* ms = root ? cJSON_GetObjectItemCaseSensitive(root, "mustSend") : nullptr;
        const cJSON* px = root ? cJSON_GetObjectItemCaseSensitive(root, "proxies") : nullptr;
        const cJSON* ta2 = px ? cJSON_GetObjectItemCaseSensitive(px, "edE84_1_TA2") : nullptr;
        const cJSON* t = ta2 ? cJSON_GetObjectItemCaseSensitive(ta2, "text") : nullptr;
        Check(ms && cJSON_GetArraySize(ms) == 26 && t && cJSON_IsString(t) && std::string(t->valuestring) == "120",
              "[4] JSON: 26 mustSend, proxies.edE84_1_TA2.text = \"120\"");
        if (root) cJSON_Delete(root);
    }
    Check(!MetaOf(sandbox.c_str()).exists, "[4] golden ReadFile does not create AGV.ini");

    std::printf("[5] save (golden spbSaveClick)\n");
    std::vector<std::string> text(25);
    for (int i = 0; i < 25; ++i) text[i] = Num(100 + i);
    std::string ack, err;
    int rc = filerw::PageSave(*d, Widgets(text, true, nullptr), "{}", &ack, &err);
    std::string file;
    const bool haveFile = ReadAll(sandbox, &file);
    Check(rc == 200 && ack.find("\"saved\":true") != std::string::npos && haveFile, "[5] editlist.save -> 200, saved, sandbox AGV.ini written " + err);
    {
        int ok = 0;
        for (int i = 0; i < 25; ++i) {
            if (IniValue(file, kKeys[i].key) == text[i] && Mem(kKeys[i]) == 100 + i) ++ok;
            else std::printf("    %s: file %s mem %d want %s\n", kKeys[i].key, IniValue(file, kKeys[i].key).c_str(), Mem(kKeys[i]), text[i].c_str());
        }
        Check(ok == 25, "[5] 25 keys under [Configuration] with golden key names = atoi(page text); ReadFile read them back into TestIF_File");
    }
    Check(IniValue(file, "E84 Enable") == "1" && TestIF_File.bEnableE84, "[5] checkbox on -> \"E84 Enable\"=1 and TestIF_File.bEnableE84=true (E84 handshake switch)");
    rc = filerw::PageSave(*d, Widgets(text, false, nullptr), "{}", &ack, &err);
    ReadAll(sandbox, &file);
    Check(rc == 200 && IniValue(file, "E84 Enable") == "0" && !TestIF_File.bEnableE84, "[5] checkbox off -> \"E84 Enable\"=0, bEnableE84=false");

    std::printf("[6] mustSend / golden atoi\n");
    {
        const std::string before = file;
        std::vector<std::string> t2 = text;
        t2[0] = "999";
        rc = filerw::PageSave(*d, Widgets(t2, true, "edE84_2_TD1"), "{}", &ack, &err);
        std::string after;
        ReadAll(sandbox, &after);
        Check(rc == 400 && err.find("edE84_2_TD1") != std::string::npos && after == before && !TestIF_File.bEnableE84,
              "[6] a missing box (edE84_2_TD1) -> 400, nothing written, bEnableE84 unchanged");
        filerw::PageJson(*d, &json);
        t2 = text;
        t2[0] = "12x";
        t2[1] = "abc";
        rc = filerw::PageSave(*d, Widgets(t2, false, nullptr), "{}", &ack, &err);
        ReadAll(sandbox, &after);
        Check(rc == 200 && IniValue(after, "E84_1 Time Out 1") == "12" && IniValue(after, "E84_1 Time Out 2") == "0" &&
              TestIF_File.iE84TimeOut_K12[0][0] == 12 && TestIF_File.iE84TimeOut_K12[0][1] == 0,
              "[6] golden atoi: \"12x\" -> 12, \"abc\" -> 0 (no range check in golden spbSaveClick)");
    }

    std::printf("[7] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[7] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        const std::string root = argv[1], web = argv[2];
        std::string gen, cpp, cs, wb, cm, integ, html, js;
        const bool rd = ReadAll(root + "/FileRW/TestIF_File_AGV.gen.inc", &gen) && ReadAll(root + "/FileRW/TestIF_File_AGV.cpp", &cpp) &&
                        ReadAll(root + "/csystem.cpp", &cs) && ReadAll(root + "/tools/wb_serve.cpp", &wb) &&
                        ReadAll(root + "/FileRW/_editlist_sources.cmake", &cm) && ReadAll(root + "/tools/editlist/_integrated.txt", &integ) &&
                        ReadAll(web + "/Setup.AGV.html", &html) && ReadAll(web + "/ht9045_agv_c.js", &js);
        Check(rd, "[7] read the generated file, entry, csystem.cpp, wb_serve.cpp, sources cmake, _integrated.txt, Setup.AGV.html, ht9045_agv_c.js");
        // AI(W906-B8-AG1-CRLF) 20260930: ST01-M's a9f89386 gate failed the sources-cmake check below -- a fresh checkout with core.autocrlf=true
        //   (the gate worktree D:\AI_TempFile\st02-gb-p1) writes these LF-committed files with CRLF, so "...AGV.cpp\n" never matched.
        //   Strip CR from every source read here so each needle below is line-ending agnostic.
        std::string* const kSrc[] = {&gen, &cpp, &cs, &wb, &cm, &integ, &html, &js};
        for (std::string* s : kSrc) s->erase(std::remove(s->begin(), s->end(), '\r'), s->end());
        Check(CodeCount(gen, "szDir=W906_AgvIniPath();") == 2 && !CodeHas(gen, "szDir=\"D:\\\\HT9045\\\\config\\\\AGV.ini\";") &&
              gen.find("szDir=\"D:\\\\HT9045\\\\config\\\\AGV.ini\";") != std::string::npos,
              "[7] generated ReadFile / spbSaveClick go through the seam; the golden literal only survives inside #if 0");
        Check(CodeHas(gen, "TestIF_File.bEnableE84                  =ReadIniData(szDir, \"Configuration\", \"E84 Enable\"   , false);") &&
              CodeHas(gen, "(EL<TCheckBox>(\"TfAGV\", \"cbEnableAGVFunction\")->Checked)?1:0);"),
              "[7] golden ReadFile :1261 / spbSaveClick :1221 E84 Enable lines are live");
        Check(CodeHas(cs, "if(USE_E84_Sensor==1 && TestIF_File.bEnableE84)"),
              "[7] the E84 reader csystem.cpp:30394 (MainProc) still gates the handshake on TestIF_File.bEnableE84");
        Check(CodeHas(wb, "{ extern void FileRW_AGV_Boot(); FileRW_AGV_Boot(); }"),
              "[7] wb_serve.cpp boots the TfAGV proxies (code, not after //)");
        Check(cm.find("    FileRW/TestIF_File_AGV.cpp\n") != std::string::npos && integ.find("\nTestIF_File_AGV") != std::string::npos,
              "[7] FileRW/_editlist_sources.cmake lists the entry; tools/editlist/_integrated.txt has the struct");
        Check(html.find("<script src=\"ht9045_wire_engine.js\"></script><script src=\"ht9045_agv_c.js\"></script>") != std::string::npos &&
              js.find("var STRUCT = 'TestIF_File_AGV';") != std::string::npos && js.find("HT9045Page.save();") != std::string::npos,
              "[7] Setup.AGV.html loads the engine + ht9045_agv_c.js; the save button goes to the engine");
    }

    const Meta realAfter = MetaOf(kReal);
    Check(SameMeta(realBefore, realAfter), std::string("[guard] real ") + kReal + (realAfter.exists ? " unchanged (size / mtime)" : " still absent"));
    std::printf("test_b8_ag1_agvini: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
