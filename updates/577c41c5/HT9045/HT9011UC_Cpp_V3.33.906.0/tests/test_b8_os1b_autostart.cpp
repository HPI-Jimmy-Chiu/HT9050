// =============================================================================
//  test_b8_os1b_autostart.cpp -- B8 OS-1b：Offset 頁微調鈕存完、勾了「Auto Offset Position Check」就 START（golden fMain->Start）
//
//  //AI(W906-B8-OS1B) 20261001 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-1b」；
//    Jimmy RULINGS_20261001 第 0 條（照 golden 翻、會動的也接上，不用問）；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」。
//  golden：V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cOffSet.cpp
//    sb_AutoOffsetUp／Down／Right／LeftClick :2892-2942：if(SystemStart) return; ±0.1；spbSaveClick(this)；
//      if(cb_AutoOffsetPositionCheck->Checked==true) fMain->Start("sb_AutoOffsetUpClick")（:2901／:2914／:2927／:2940）。
//    UseAutoOffsetFunction :2944-2957（手臂流程問「這個部位要不要停下來看」）。
//  受測的是 wb_serve 編進去的同一份 FileRW/Offset_File.cpp（檔尾 W906_Offset_B8AutoCheckStart／OS_B8UseAutoOffsetFunction）＋產生檔
//    FileRW/Offset_File.gen.inc 的四支處理器，經真的 WS 入口 W906_FormEvent（FileRW/_FormEvent.cpp：after-ack 佇列、ack "afterAck"）與
//    W906_FormEventRunAfterAck（wb_serve form.event 臂在 CompleteCommand 之後呼叫的那一支）；START 的安裝座 W906_RemoteRun（forms/fMain.h 檔尾）
//    在這裡換成記錄器（wb_serve 裝的是 TfMainWeb::StartFromWeb）。god-stack 用 RESCAN 連（同 ctest B8_Os5_SortButtons）。
//    [0] 開機＋開頁（golden FormShow；W906_INIDATA_ROOT 沙盒，沒設就不跑存檔那幾段）
//    [1] 沒勾：微調照存（edArmY ±0.1）、沒有 afterAck、記錄器 0 次
//    [2] 勾了：ack 有一項 afterAck（golden 行號＋處理器名）、回覆前記錄器 0 次（不在處理器裡、不在 FormLock 裡 START）；
//        RunAfterAck(true) ⇒ 記錄器 1 次、Func＝"sb_AutoOffsetUpClick"、當時 FormLock 深度 0；再跑一次 ⇒ 不重複
//    [3] 四顆逐顆：Func 與 golden 行號（2901／2914／2927／2940）
//    [4] RunAfterAck(false)（form.event 失敗）⇒ 丟掉不跑
//    [5] 登記了沒被跑、下一個 form.event 進來 ⇒ 作廢（不會晚一拍才 START）
//    [6] 運轉中（SystemStart／SoftStart）⇒ running、沒有登記；處理器失敗（select-first）⇒ 沒有登記
//    [7] 安裝座沒裝（ctest／別的程式）⇒ RunAfterAck 不當掉、記錄器 0 次
//    [8] UseAutoOffsetFunction（fOffSet->UseAutoOffsetFunction → forms/fOffSet.cpp:10 → hook）：勾了＋Offset 開著＋bUseAutoOffsetFunction＋
//        form.event 選過的部位 ⇒ 那個部位名 true、別的部位 false；沒勾／沒開／功能關／重開頁後（Caption 停在最後一組）／沒選部位 ⇒ false；讀替身時持 FormLock
//    [9] 接上了沒（原始碼棘輪，去掉 // 與 /* */ 註解、跳過 #if 0；argv[1]＝移植樹根目錄、argv[2]＝D:\HT9045\web\page，唯讀）
//  NOT COVERED：TfMainWeb::StartFromWeb 本身（START 全套；ctest START_SitesCensus 只數呼叫點）；手臂流程真的停在檢查位置
//    （ainarm9045.cpp InArmNeedCheckOffset → Task 220，要 MainProc 跑起來）；golden 停下來時的 fMain->Pause（移植樹基底 Pause 是計數器）。
//  寫檔：只寫沙盒（W906_INIDATA_ROOT 底下的 Offset 檔）。開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 的內容，
//    以及 D:\HT9045\IniData\Offset、D:\HT9045\IniData\DefineOffset、D:\HT9045\data 的檔案清單（名稱、大小、修改時間），有變就失敗。
// =============================================================================
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "LastSet.h"
#include "MachineType.h"
#include "forms/fMain.h"
#include "forms/fOffSet.h"
#include "w906_ctest_guard.h"   // W906TestRequireCtestRedirects（不在 ctest 的沙盒環境就不跑：存檔會寫 Offset 檔）

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

extern void FileRW_Offset_Boot();
extern int FileRW_Offset_Page(std::string* json);
void EnsureArmOffsetObjects();   // forms/fOffSet.h:450（本體 cOffSet.cpp:166）
extern bool (*W906_FormFShowHook)(const char* goldenForm);   // csystem.h:421；W906_FormShowing(obj,false)＝hook(obj)
extern bool (*W906_OffsetUseAutoOffsetHook)(AnsiString asWhich);   // forms/fOffSet.cpp:10
extern AnsiString OffsetPath;    // common.h:73（W906_INIDATA_ROOT 導向）
extern AnsiString DefaultPath;   // common.cpp:224（同上）
extern bool (*W906_OffsetSetWorkParameterFn)();   // FileRW/Offset_File.cpp 檔尾（golden spbSaveClick :2882 SetWorkParameter 的測試縫）
extern int iMaxLevelItem;        // cSecurity.cpp:47（開機 W906_SecurityBoot 設 180；golden V912 cSecurity.cpp:210）
extern bool authMainForm[12];    // cAuthority.h:56（D:\HT9045\config\Security_new.def [Main]，開機才讀；缺鍵預設 1）

// 連結用（不是受測碼），同 tests/test_b8_os5_sortbuttons.cpp：FileRW_IniConfig_ChangeCBListProperty、JsonBridge 的 FindBridge／RunEvent。
// FormLock 換成計數器：after-ack 的 START 必須在鎖外（深度 0）、UseAutoOffsetFunction 讀替身時在鎖裡。
void FileRW_IniConfig_ChangeCBListProperty() {}
namespace {
int g_lockDepth = 0;
int g_lockMax = 0;
}
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
void FormLock() { ++g_lockDepth; if (g_lockDepth > g_lockMax) g_lockMax = g_lockDepth; }
void FormUnlock() { --g_lockDepth; }
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

// 目錄樹清單：相對路徑 → "大小/修改時間"（看有沒有被寫）
void ListTree(const std::string& root, const std::string& rel, std::map<std::string, std::string>* out)
{
    WIN32_FIND_DATAA fd;
    const std::string pat = root + (rel.empty() ? "" : "\\" + rel) + "\\*";
    HANDLE h = ::FindFirstFileA(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        const std::string r = rel.empty() ? name : rel + "\\" + name;
        char buf[96];
        std::snprintf(buf, sizeof(buf), "%lu/%lu/%lu/%lu", (unsigned long)fd.nFileSizeHigh, (unsigned long)fd.nFileSizeLow,
                      (unsigned long)fd.ftLastWriteTime.dwHighDateTime, (unsigned long)fd.ftLastWriteTime.dwLowDateTime);
        (*out)[r] = buf;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ListTree(root, r, out);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// START 安裝座的記錄器（wb_serve 裝的是 TfMainWeb::StartFromWeb）
int g_startCalls = 0;
std::string g_startFunc;
int g_startLockDepth = -1;
bool FakeStart(AnsiString f)
{
    ++g_startCalls;
    g_startFunc = f.c_str();
    g_startLockDepth = g_lockDepth;
    return true;
}
bool FakePause(AnsiString) { return true; }
int g_swp = 0;                   // golden spbSaveClick :2882 SetWorkParameter（真本體重讀 teach.ini 等機台檔 ⇒ 換成記錄器）
bool FakeSetWorkParameter() { ++g_swp; return true; }
void ResetRecorder() { g_startCalls = 0; g_startFunc.clear(); g_startLockDepth = -1; g_swp = 0; }

const char* g_open = nullptr;
bool FakePageTable(const char* form) { return g_open && form && std::strcmp(form, g_open) == 0; }
bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }

bool Ev(const std::string& value, std::string* ack, std::string* err)
{
    ack->clear(); err->clear();
    return W906_FormEvent("Setup.OffSet", value, ack, err);
}
bool Part(const char* button, std::string* ack, std::string* err)
{
    return Ev(std::string("{\"form\":\"TfOffSet\",\"control\":\"") + button + "\",\"event\":\"click\"}", ack, err);
}
bool Nudge(const char* button, bool checked, std::string* ack, std::string* err)
{
    return Ev(std::string("{\"form\":\"TfOffSet\",\"control\":\"") + button + "\",\"event\":\"click\",\"state\":{\"cb_AutoOffsetPositionCheck\":{\"checked\":" +
              (checked ? "true" : "false") + "}}}", ack, err);
}
// ack "afterAck"：項數，第一項放到 *first
int AfterAck(const std::string& ack, std::string* first)
{
    first->clear();
    cJSON* a = cJSON_Parse(ack.c_str());
    const cJSON* arr = a ? cJSON_GetObjectItemCaseSensitive(a, "afterAck") : nullptr;
    int n = -1;
    if (!arr) n = 0;
    else if (cJSON_IsArray(arr)) {
        n = cJSON_GetArraySize(arr);
        const cJSON* f = cJSON_GetArrayItem(arr, 0);
        if (f && cJSON_IsString(f)) *first = f->valuestring;
    }
    if (a) cJSON_Delete(a);
    return n;
}
std::string ChangedText(const std::string& ack, const char* id)
{
    cJSON* a = cJSON_Parse(ack.c_str());
    const cJSON* ch = a ? cJSON_GetObjectItemCaseSensitive(a, "changed") : nullptr;
    const cJSON* e = ch ? cJSON_GetObjectItemCaseSensitive(ch, id) : nullptr;
    const cJSON* t = e ? cJSON_GetObjectItemCaseSensitive(e, "text") : nullptr;
    const std::string s = t && cJSON_IsString(t) ? t->valuestring : std::string();
    if (a) cJSON_Delete(a);
    return s;
}

// 去掉 // 與 /* */ 註解（字串裡的不動），跳過 #if 0 … #endif；needle 在剩下的碼裡才算
std::string StripCode(const std::string& line, bool* inBlock)
{
    std::string out;
    bool inStr = false, inChr = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (*inBlock) { if (c == '*' && i + 1 < line.size() && line[i + 1] == '/') { *inBlock = false; ++i; } continue; }
        if (inStr) { out += c; if (c == '\\' && i + 1 < line.size()) { out += line[++i]; } else if (c == '"') inStr = false; continue; }
        if (inChr) { out += c; if (c == '\\' && i + 1 < line.size()) { out += line[++i]; } else if (c == '\'') inChr = false; continue; }
        if (c == '/' && i + 1 < line.size() && line[i + 1] == '/') break;
        if (c == '/' && i + 1 < line.size() && line[i + 1] == '*') { *inBlock = true; ++i; continue; }
        if (c == '"') inStr = true;
        else if (c == '\'') inChr = true;
        out += c;
    }
    return out;
}
// 每一行的「活碼」（'\r' 先去掉：gate 的 checkout 是 CRLF）
std::vector<std::string> LiveLines(const std::string& text)
{
    std::vector<std::string> v;
    std::istringstream in(text);
    std::string l;
    bool gated = false, inBlock = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; v.push_back(std::string()); continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; v.push_back(std::string()); continue; }
        v.push_back(StripCode(l, &inBlock));
    }
    return v;
}
bool CodeHas(const std::string& text, const std::string& needle)
{
    for (const std::string& l : LiveLines(text)) if (l.find(needle) != std::string::npos) return true;
    return false;
}
bool TextHas(const std::string& text, const std::string& needle) { return text.find(needle) != std::string::npos; }

}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);   // 當掉時 ctest 的輸出停在最後一行
    std::printf("test_b8_os1b_autostart -- B8 OS-1b golden TfOffSet::sb_AutoOffset*Click fMain->Start (V912 cOffSet.cpp:2892-2957)\n");
    {
        const char* const rt[] = {"OffsetPath", OffsetPath.c_str(), "DefaultPath", DefaultPath.c_str(), 0};   // golden ReadFile／SaveFile 的路徑（common.cpp W906IniDataRedirect）
        if (!W906TestRequireCtestRedirects("B8_Os1b_AutoCheckStart", rt))
            return 2;
    }
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[2];
    bool had[2];
    for (int i = 0; i < 2; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    const char* const kTrees[] = {"D:\\HT9045\\IniData\\Offset", "D:\\HT9045\\IniData\\DefineOffset", "D:\\HT9045\\data"};
    std::map<std::string, std::string> treeBefore[3];
    for (int i = 0; i < 3; ++i) ListTree(kTrees[i], "", &treeBefore[i]);

    std::string ack, err, first;
    const char* env = std::getenv("W906_INIDATA_ROOT");
    const bool sandbox = env && *env && OffsetPath.Pos(env) == 1;
    Check(sandbox, std::string("[0] W906_INIDATA_ROOT sandbox is on and OffsetPath is under it (") + OffsetPath.c_str() + ")");

    if (sandbox) {
        SystemStart = false; SoftStart = false;
        CosFunction.bUseInvisibleOffset = false;     // golden ReadFile :2289-2305 寫死 D:\HT9045\IniData\DefineOffset、D:\HT9045\data（不走沙盒）⇒ 這支測試關掉
        if (CUSTOMER_CODE == CC_ASE_CL) CUSTOMER_CODE = 0;   // golden spbSaveClick :2826-2877 寫死 D:\HT9045\data\<配方>.ini ⇒ 同上
        IniConfig.bUseAutoOffsetFunction = true;     // golden FormShow :572 pan_AutoOffsetMove->Visible（微調鈕的面板）
        AccessLevel = 3; LevelSet.AccessLevel[2] = 0; iMaxLevelItem = 180; authMainForm[2] = true;   // golden FormShow :584-590 不停用 pnlPicker（微調鈕在裡面）：Insufficient(2,false) && authMainForm[2]
        W906_RemoteRun.Start = &FakeStart;
        W906_RemoteRun.Pause = &FakePause;
        Check(W906_OffsetSetWorkParameterFn != nullptr, "[0] SetWorkParameter seam defaults to the real body (FileRW/Offset_File.cpp tail)");
        bool (*const realSwp)() = W906_OffsetSetWorkParameterFn;
        W906_OffsetSetWorkParameterFn = &FakeSetWorkParameter;
        EnsureArmOffsetObjects();                   // wb_serve.cpp:3502（FileRW_Offset_Boot 的前提）
        FileRW_Offset_Boot();
        std::string page;
        const int st = FileRW_Offset_Page(&page);
        Check(st == 200, "[0] editlist.get Offset_File (golden FormShow) -> 200");
        Check(g_lockDepth == 0, "[0] FormLock balanced after page open");
        Check(W906_OffsetUseAutoOffsetHook != nullptr, "[0] forms/fOffSet.cpp:10 hook installed by FileRW/Offset_File.cpp");
        Check(filerw::ELOperable("TfOffSet", "sb_AutoOffsetUp") && filerw::ELOperable("TfOffSet", "sbLoader"),
              "[0] the nudge buttons (pan_AutoOffsetMove > Panel6 > pnlPicker) and the part button are operable after golden FormShow");

        std::printf("[1] unchecked: save only\n");
        ResetRecorder();
        Check(Part("sbLoader", &ack, &err), "[1] part button sbLoader (golden SpBotSelClick) " + err);
        const std::string y0 = EL<TEdit>("TfOffSet", "edArmY")->Text.c_str();
        bool sent = Nudge("sb_AutoOffsetUp", false, &ack, &err);
        Check(sent, "[1] sb_AutoOffsetUp (unchecked) -> ok " + err);
        const std::string y1 = ChangedText(ack, "edArmY");
        Check(!y1.empty() && std::atof(y1.c_str()) > std::atof(y0.c_str()) + 0.05, "[1] edArmY " + y0 + " -> " + y1 + " (golden :2897 +0.1, spbSaveClick ran)");
        Check(g_swp == 1, "[1] golden spbSaveClick tail ran (SetWorkParameter :2882 once)");
        Check(AfterAck(ack, &first) == 0 && formevent::afterack::Pending() == 0, "[1] no afterAck in the ack, nothing queued");
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 0, "[1] no START (golden: cb_AutoOffsetPositionCheck unchecked)");

        std::printf("[2] checked: START after the reply\n");
        ResetRecorder();
        Check(Part("sbLoader", &ack, &err), "[2] part button sbLoader " + err);
        sent = Nudge("sb_AutoOffsetUp", true, &ack, &err);
        Check(sent, "[2] sb_AutoOffsetUp (checked) -> ok " + err);
        const int n2 = AfterAck(ack, &first);
        Check(n2 == 1 && TextHas(first, "cOffSet.cpp:2901") && TextHas(first, "sb_AutoOffsetUpClick"),
              "[2] ack.afterAck has one item naming golden cOffSet.cpp:2901 fMain->Start(\"sb_AutoOffsetUpClick\"): " + first);
        Check(g_startCalls == 0 && formevent::afterack::Pending() == 1, "[2] not started inside the handler (queued, waiting for the reply)");
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 1 && g_startFunc == "sb_AutoOffsetUpClick", "[2] after the reply: W906_RemoteRunStart(\"sb_AutoOffsetUpClick\") once (golden Func)");
        Check(g_startLockDepth == 0, "[2] START ran outside FormLock (depth " + std::to_string(g_startLockDepth) + ")");
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 1 && formevent::afterack::Pending() == 0, "[2] a second RunAfterAck does not start again");

        std::printf("[3] each button\n");
        {
            const char* const kBtn[4] = {"sb_AutoOffsetUp", "sb_AutoOffsetDown", "sb_AutoOffsetRight", "sb_AutoOffsetLeft"};
            const int kLine[4] = {2901, 2914, 2927, 2940};
            int ok = 0;
            for (int i = 0; i < 4; ++i) {
                ResetRecorder();
                const bool p = Part("sbLoader", &ack, &err);
                const bool s = Nudge(kBtn[i], true, &ack, &err);
                const int n = AfterAck(ack, &first);
                W906_FormEventRunAfterAck(s);
                const std::string fn = std::string(kBtn[i]) + "Click";
                if (p && s && n == 1 && TextHas(first, "cOffSet.cpp:" + std::to_string(kLine[i])) && g_startCalls == 1 && g_startFunc == fn) ++ok;
                else std::printf("    %s: part=%d sent=%d n=%d first=%s calls=%d func=%s err=%s\n", kBtn[i], p, s, n, first.c_str(), g_startCalls, g_startFunc.c_str(), err.c_str());
            }
            Check(ok == 4, "[3] Up/Down/Right/Left -> START Func = golden handler name, ack cites golden :2901/:2914/:2927/:2940");
        }

        std::printf("[4] form.event failed -> dropped\n");
        ResetRecorder();
        Part("sbLoader", &ack, &err);
        Nudge("sb_AutoOffsetDown", true, &ack, &err);
        W906_FormEventRunAfterAck(false);
        Check(g_startCalls == 0 && formevent::afterack::Pending() == 0, "[4] RunAfterAck(false) drops the queued START");

        std::printf("[5] queued but not run, next form.event -> void\n");
        ResetRecorder();
        Part("sbLoader", &ack, &err);
        Nudge("sb_AutoOffsetRight", true, &ack, &err);
        Check(formevent::afterack::Pending() == 1, "[5] queued");
        Part("sbLoader", &ack, &err);                // 下一個 form.event（wb_serve 每一則都會跑 RunAfterAck；這裡模擬漏跑）
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 0, "[5] the next form.event voided it -- no late START");

        std::printf("[6] running / handler failure -> nothing queued\n");
        ResetRecorder();
        Part("sbLoader", &ack, &err);
        SystemStart = true;
        sent = Nudge("sb_AutoOffsetLeft", true, &ack, &err);
        const bool runRefused = !sent && Starts(err, "running");
        W906_FormEventRunAfterAck(sent);
        SystemStart = false; SoftStart = true;
        sent = Nudge("sb_AutoOffsetLeft", true, &ack, &err);
        const bool softRefused = !sent && Starts(err, "running");
        W906_FormEventRunAfterAck(sent);
        SoftStart = false;
        Check(runRefused && softRefused && g_startCalls == 0 && formevent::afterack::Pending() == 0,
              "[6] SystemStart / SoftStart -> running (golden :2894 if(SystemStart) return; + the form.event guard), no START");
        ResetRecorder();
        {
            std::string pg;
            FileRW_Offset_Page(&pg);                 // 重開頁 ⇒ 伺服器不知道頁面在哪一組（select-first）
        }
        sent = Nudge("sb_AutoOffsetUp", true, &ack, &err);
        W906_FormEventRunAfterAck(sent);
        Check(!sent && TextHas(err, "select-first") && g_startCalls == 0 && formevent::afterack::Pending() == 0,
              "[6] handler failed (select-first after a page reopen) -> nothing queued, no START: " + err.substr(0, 80));

        std::printf("[7] seat not installed\n");
        ResetRecorder();
        W906_RemoteRun.Start = nullptr;
        Part("sbLoader", &ack, &err);
        sent = Nudge("sb_AutoOffsetUp", true, &ack, &err);
        W906_FormEventRunAfterAck(sent);
        Check(sent && g_startCalls == 0 && formevent::afterack::Pending() == 0, "[7] W906_RemoteRun.Start == 0 -> RunAfterAck runs, W906_RemoteRunStart says no, no crash");
        W906_RemoteRun.Start = &FakeStart;

        std::printf("[8] UseAutoOffsetFunction (golden cOffSet.cpp:2944-2957)\n");
        {
            W906_FormFShowHook = &FakePageTable;
            g_open = "fOffSet";
            Part("sbLoader", &ack, &err);
            Nudge("sb_AutoOffsetUp", true, &ack, &err);    // 勾選框＝true（state 套上去）
            W906_FormEventRunAfterAck(false);
            const std::string cap = EL<TPanel>("TfOffSet", "palOffsetParts")->Caption.c_str();
            Check(cap == "Loader", "[8] palOffsetParts->Caption after sbLoader = \"" + cap + "\" (golden SpBotSelClick :2503 CapStr[OfsLoader])");
            g_lockMax = 0;
            const bool yes = fOffSet->UseAutoOffsetFunction("Loader");
            const int lockUsed = g_lockMax;
            const bool other = fOffSet->UseAutoOffsetFunction("Input Shuttle1");
            Check(yes && !other, "[8] checked + Offset open + function on + part Loader: \"Loader\" -> true, \"Input Shuttle1\" -> false");
            Check(lockUsed >= 1 && g_lockDepth == 0, "[8] the hook reads the proxies under FormLock and releases it");
            EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked = false;
            const bool unchecked = fOffSet->UseAutoOffsetFunction("Loader");
            EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked = true;
            g_open = "fAGV";
            const bool closed = fOffSet->UseAutoOffsetFunction("Loader");
            g_open = "fOffSet";
            IniConfig.bUseAutoOffsetFunction = false;
            const bool off = fOffSet->UseAutoOffsetFunction("Loader");
            IniConfig.bUseAutoOffsetFunction = true;
            Check(!unchecked && !closed && !off, "[8] unchecked / Offset not open (page table) / bUseAutoOffsetFunction off -> false");
            {
                std::string pg;
                FileRW_Offset_Page(&pg);             // 重開頁：每一組都跑過一次，Caption 停在最後一組
            }
            EL<TCheckBox>("TfOffSet", "cb_AutoOffsetPositionCheck")->Checked = true;
            const std::string last = EL<TPanel>("TfOffSet", "palOffsetParts")->Caption.c_str();
            const bool reopened = fOffSet->UseAutoOffsetFunction(last.c_str());
            Check(!reopened, "[8] after a page reopen the caption is the last group (\"" + last + "\"), not the operator's pick -> false (golden: no part picked = '')");
            Ev("{\"form\":\"TfOffSet\",\"control\":\"sb_AutoOffsetUp\",\"event\":\"click\",\"noPart\":true,\"state\":{\"cb_AutoOffsetPositionCheck\":{\"checked\":true}}}", &ack, &err);
            W906_FormEventRunAfterAck(false);
            Check(!fOffSet->UseAutoOffsetFunction(last.c_str()) && !fOffSet->UseAutoOffsetFunction("Loader"), "[8] no part picked (R128 noPart) -> false");
            W906_FormFShowHook = nullptr;
            g_open = nullptr;
            Part("sbLoader", &ack, &err);
            Check(!fOffSet->UseAutoOffsetFunction("Loader"), "[8] no page table (ctest default) = Offset not open -> false");
        }
        W906_RemoteRun.Start = nullptr;
        W906_RemoteRun.Pause = nullptr;
        W906_OffsetSetWorkParameterFn = realSwp;
    }

    std::printf("[9] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[9] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        std::string gen, cpp, fo, wb, fe, js, py;
        const bool rd = ReadAll(std::string(argv[1]) + "/FileRW/Offset_File.gen.inc", &gen) &&
                        ReadAll(std::string(argv[1]) + "/FileRW/Offset_File.cpp", &cpp) &&
                        ReadAll(std::string(argv[1]) + "/forms/fOffSet.cpp", &fo) &&
                        ReadAll(std::string(argv[1]) + "/tools/wb_serve.cpp", &wb) &&
                        ReadAll(std::string(argv[1]) + "/FileRW/_FormEvent.cpp", &fe) &&
                        ReadAll(std::string(argv[1]) + "/tools/editlist/Offset_File.py", &py) &&
                        ReadAll(std::string(argv[2]) + "/ht9045_offset_ev.js", &js);
        Check(rd, "[9] read the seven files");
        Check(CodeHas(gen, "W906_Offset_B8AutoCheckStart(\"sb_AutoOffsetUpClick\", 2901);") &&
              CodeHas(gen, "W906_Offset_B8AutoCheckStart(\"sb_AutoOffsetDownClick\", 2914);") &&
              CodeHas(gen, "W906_Offset_B8AutoCheckStart(\"sb_AutoOffsetRightClick\", 2927);") &&
              CodeHas(gen, "W906_Offset_B8AutoCheckStart(\"sb_AutoOffsetLeftClick\", 2940);") &&
              !CodeHas(gen, "not ported -- B8 / Jimmy"),
              "[9] gen.inc: the four golden fMain->Start lines call W906_Offset_B8AutoCheckStart (live), the old todo is gone");
        Check(CodeHas(cpp, "const bool called = W906_RemoteRunStart(AnsiString(func.c_str()));") &&
              CodeHas(cpp, "formevent::afterack::Defer(&OS_B8RunAutoCheckStart,") &&
              CodeHas(cpp, "W906_OffsetUseAutoOffsetHook = &OS_B8UseAutoOffsetFunction;"),
              "[9] Offset_File.cpp: deferred START through W906_RemoteRunStart, hook installed (live code)");
        Check(CodeHas(gen, "W906_OffsetSetWorkParameterFn(); }") && CodeHas(cpp, "bool (*W906_OffsetSetWorkParameterFn)() = &SetWorkParameter;"),
              "[9] spbSaveClick :2882 calls SetWorkParameter through the seam, whose default is the real body (behaviour unchanged)");
        Check(CodeHas(fo, "bool TfOffSet::UseAutoOffsetFunction(AnsiString sName) { return W906_OffsetUseAutoOffsetHook != 0 && W906_OffsetUseAutoOffsetHook(sName); }"),
              "[9] forms/fOffSet.cpp: UseAutoOffsetFunction calls the hook (live, before the line's //)");
        {
            bool found = false;
            for (const std::string& l : LiveLines(wb)) {
                const std::string::size_type a = l.find("server.CompleteCommand((unsigned long long)wc.id, feOk, feOk ? feAck : feErr);");
                if (a == std::string::npos) continue;
                const std::string::size_type b = l.find("W906_FormEventRunAfterAck(feOk);", a);
                found = b != std::string::npos;
            }
            Check(found, "[9] wb_serve.cpp: the form.event arm calls W906_FormEventRunAfterAck(feOk) AFTER its CompleteCommand (live code)");
        }
        Check(CodeHas(fe, "formevent::afterack::Reset();   ") && CodeHas(fe, "if (!ok) {  formevent::afterack::Reset();") &&
              CodeHas(fe, "formevent::d013::Ack(w);  formevent::afterack::Ack(w);"),
              "[9] _FormEvent.cpp: reset at entry, reset on failure, ack key (live)");
        Check(TextHas(js, "a2.afterAck") && TextHas(js, "a0.afterAck"), "[9] page ht9045_offset_ev.js shows ack.afterAck (both nudge paths)");
    }

    for (int i = 0; i < 2; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("[guard] unchanged: ") + kGuard[i]);
    }
    for (int i = 0; i < 3; ++i) {
        std::map<std::string, std::string> now;
        ListTree(kTrees[i], "", &now);
        Check(now == treeBefore[i], std::string("[guard] file list (name, size, mtime) unchanged: ") + kTrees[i] + " (" + std::to_string(now.size()) + " entries)");
    }
    std::printf("test_b8_os1b_autostart: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
