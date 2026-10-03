// =============================================================================
//  test_b8_cl4_autoclean.cpp -- B8 CL-4：Cleaning 頁「Clean」鈕（golden TfCleaning::btnStartAutoCleanClick → TfShowBinSelect::btnAutoCleanClick）
//
//  //AI(W906-B8-CL4) 20261001 [W906] St01 新檔。派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CL-4」；
//    Jimmy RULINGS_20261001 第 0 條（照 golden 翻、會動的也接上）；Steven 20261001 09:4x「需要上機驗證的, 都是請Eastsun處理」。
//  golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003]（（V912 :N）＝D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy；AI(W906-E030-CITE) 20261003）：AutoClean\uCleaning.cpp:2869-2872（V912 :2901-2904） btnStartAutoCleanClick
//    ＝ fShowBinSelect->btnAutoCleanClick(fShowBinSelect)（cShowBinSelect.cpp:2101-2166（V912 :2248-2315））。下面 "golden :N" 是 906；gen.inc 的 golden 標籤是 V912（產生器讀 V912，kGolden 照它）。
//  受測的是 wb_serve 編進去的同一份 FileRW/TestIF_File_Cleaning.cpp（檔尾 W906_ShowBinSelect_btnAutoCleanClick、B3 段的事件父層與替身）＋
//    產生檔 FileRW/TestIF_File_Cleaning.gen.inc 的 CL_btnStartAutoCleanClick 與 kCL_Events 第 84 列，經真的 WS 入口 W906_FormEvent
//    （FileRW/_FormEvent.cpp → FileRW/_EditPage.cpp RunPageEvent → 本檔跳板 EvB3Run）。InitialAutoCleanTask 等（AutoClean/AutoClean.cpp）、
//    HasICUnderMachine／CheckIndexIsNormal（csystem.cpp）、TfMain::BtnOneCycleClick（cCleanOut.cpp）都是 god-stack 的活本體（RESCAN 連）。
//    [0] 開機（FileRW_Cleaning_Boot＝golden CreateForm → 建構子 LoadAutoCleanData，W906_INIDATA_ROOT 沙盒）＋開頁（golden FormShow 換成空函式，
//        同 ctest B8_Su7_RtcClick：FormShow 會讀寫配方檔）；events.btnStartAutoClean：click、golden 名、operable
//    [1] 都過（沒料）：bRunAutoClean=true、hang-up 看門狗開著、五個任務＝1、計數歸 0（主畫面計數格 "0"）、沒有訊息、沒有 One Cycle
//    [2] 前三道 return（已在清潔／ASM One Cycle／One Cycle 中）與功能、模式兩道：什麼都不動、沒有訊息
//    [3] 沒料四道（歸零、Tray arm X、Index Z、IndexStatus；[D51] 兩種）：golden 訊息（ack.messages）、bRunAutoClean 沒設；
//        golden 疑點照翻：任務已登記、計數已歸 0
//    [4] 機台有料：InitialAutoCleanAllTask → BtnOneCycleClick（bIsAutoOneCycle、BtnOneCycle->Down、bManualOneCycle）；bRunAutoClean 不設
//    [5] 運轉中（SystemStart／SoftStart）⇒ running、什麼都不動；父層 grpCleanPara 停用（golden FormShow :1511 等級 43 不足）⇒ bad-payload
//    [6] 不用 after-ack 佇列（golden 沒有 YES／NO、沒有 START）：每一次 click 之後 formevent::afterack::Pending()==0
//    [7] 接上了沒（原始碼棘輪：去掉 // 與 /* */ 註解、'\r'，跳過 #if 0；argv[1]＝移植樹根目錄、argv[2]＝D:\HT9045\web\page，唯讀）
//  NOT COVERED：按 START 之後主流程真的去清潔（DoAllProcess 階梯 csystem.cpp:33408、DoAutoCleanKit，要 MainProc 跑起來；ctest AutoClean／
//    DoIndexAutoClean 管那一半）；RecordProcess("AUTO CLEAN pressed") 的紀錄檔（只看原始碼）；馬達編碼器的 Galil 讀值（Index Z 用 golden 的
//    MovFlag 那一條驅動）。
//  寫檔：只寫沙盒（W906_INIDATA_ROOT 底下）。開跑前後比對 D:\HT9045\system\Gerneral.ini、D:\HT9045\config\config.ini 的內容，以及
//    D:\HT9045\IniData、D:\HT9045\data 的檔案清單（名稱、大小、修改時間），有變就失敗。
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
#include "MachineType.h"
#include "csystem.h"            // hAutoCleanHangUp、HasICUnderMachine、CheckIndexIsNormal
#include "forms/fMain.h"
#include "w906_ctest_guard.h"   // W906TestRequireCtestRedirects（不在 ctest 的沙盒環境就不跑：開機會寫配方檔）
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h 的虛擬預設函式（不是本檔的碼；mymotor.h 帶進來）
#include "Motor/mymotor.h"
#include "w906_test_motors.h"   // W906_TestEnsureSimMotors（golden 開機不變式：MOT[].Motor 不是 NULL）
#pragma GCC diagnostic pop

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_Cleaning_Boot();
extern AnsiString DataPath;      // common.cpp:225（W906_INIDATA_ROOT 導向）
extern AnsiString DefaultPath;   // common.cpp:224（同上）

// 連結用（不是受測碼），同 tests/test_b8_os1b_autostart.cpp：FileRW_IniConfig_ChangeCBListProperty、JsonBridge 的 FindBridge／RunEvent；
// FormLock 換成計數器（處理器在鎖裡跑、跑完要放掉）。
void FileRW_IniConfig_ChangeCBListProperty() {}
// libht9045_sm 的 cSpeed.cpp 引用、本體在只編進 wb_serve 的 FileRW/Ld_UldDelayTime.cpp：本測試不會走到（走到就停，同 tests/test_main_ctlbuttons_stubs.cpp）
void FileRW_LdUld_Boot() { std::printf("link-only stand-in reached: FileRW_LdUld_Boot\n"); std::abort(); }
void FileRW_LdUld_ReadFile() { std::printf("link-only stand-in reached: FileRW_LdUld_ReadFile\n"); std::abort(); }
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

const char* const kTag = "TestIF_File_Cleaning";
const char* const kForm = "TfCleaning";
const char* const kGolden = "AutoClean/uCleaning.cpp:2901 TfCleaning::btnStartAutoCleanClick";

void NoShow() {}

// editlist.get 的「開過頁」記號，但不跑 golden FormShow（它的 LoadAutoCleanData／SearchCleanNum 會讀寫配方檔）
cJSON* OpenPage()
{
    const filerw::PageDesc* d = filerw::FindPage(kTag);
    if (!d) return nullptr;
    filerw::PageDesc c = *d;
    c.formShow = &NoShow;
    std::string json;
    if (filerw::PageJson(c, &json) != 200) return nullptr;
    return cJSON_Parse(json.c_str());
}
std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}

// 一次 WS form.event click（頁面 ht9045_cleaning_ev.js 送的形狀）
struct Ack {
    bool sent = false;
    std::string err, raw, msgs;   // msgs：ack.messages 原文（JSON）
    int nMsg = -1;
};
void Click(Ack* a)
{
    a->raw.clear(); a->err.clear(); a->msgs.clear(); a->nMsg = -1;
    a->sent = W906_FormEvent(kTag, "{\"form\":\"TfCleaning\",\"control\":\"btnStartAutoClean\",\"event\":\"click\"}", &a->raw, &a->err);
    cJSON* j = a->sent ? cJSON_Parse(a->raw.c_str()) : nullptr;
    const cJSON* m = j ? cJSON_GetObjectItemCaseSensitive(j, "messages") : nullptr;
    if (m && cJSON_IsArray(m)) {
        a->nMsg = cJSON_GetArraySize(m);
        char* p = cJSON_PrintUnformatted(m);
        if (p) { a->msgs = p; cJSON_free(p); }
    }
    if (j) cJSON_Delete(j);
}
bool Has(const std::string& s, const std::string& n) { return s.find(n) != std::string::npos; }

// golden 都過的機台狀態（沒料、歸零了、Tray arm X 在 Empty 後面、Index Z 停著、IndexStatus 正常、配方開 Auto Clean 手動模式）
void Ready()
{
    SystemStart = false; SoftStart = false;
    bRunAutoClean = false; bIsASMAutoOneCycle = false; iOneCycle = 0;
    bIsAutoOneCycle = false; bManualOneCycle = false; bBackupOneCycle_ByAutoClean = false;
    fMain->BtnOneCycle->Down = false;
    IniConfig.bEnableAutoCleanFunction = true;
    TestIF.iAutoClean_Function = 1;
    TestIF.iAutoClean_Mode = M_MANUAL;
    IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear = false;
    fAllMotorHome = true;
    Prod.iXTrayEmpty = 1000;
    MOT[MTrayX].Motor->SetPosition(2000);                // golden :2130（V912 :2279） MOT[MTrayX].ReadPos() ≥ Prod.iXTrayEmpty
    // golden CheckIndexIsNormal（csystem.cpp:13433）：Gali_ReadEncoderInRandge 遇到 Motor->Enable==false 回 true（golden 自己的分支），
    // 不經 Galil 讀值；Z 有沒有在動走 golden 的 MovFlag 那一條
    MOT[MTestZ1].Motor->Enable = false; MOT[MTestZ2].Motor->Enable = false; MOT[MTestY1].Motor->Enable = false;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ2].MovFlag = false; MOT[MTestY1].MovFlag = false;
    IndexStatus = Z1_Z2_Normal;
    MOT[MMTrayZ].fHasTray = false;
    iDoAutoCleanTask = 0; iDoShuttle1AutoCleanTask = 0; iDoShuttle2AutoCleanTask = 0; iDoShuttleAutoCleanTask = 0; iDoIndexAutoCleanTask = 0;
    iAutoClean_IndexContactCount = 7;
    fMain->AutoCleanContactCountLabel->Caption = "7";
    Prod.iHangupMaxTime = 3600;
    hAutoCleanHangUp.SetSecAndOn(0);                     // 已到時（Off()==true）：按下之後 Off()==false ⇒ golden :2160（V912 :2309） 開了看門狗
    EL<TGroupBox>(kForm, "grpCleanPara")->Enabled = true;
}
int Tasks() { return iDoAutoCleanTask + iDoShuttle1AutoCleanTask + iDoShuttle2AutoCleanTask + iDoShuttleAutoCleanTask + iDoIndexAutoCleanTask; }
// 什麼都沒動（前三道 return、功能／模式兩道、運轉中、點不到）
bool Untouched()
{
    return Tasks() == 0 && iAutoClean_IndexContactCount == 7 && fMain->AutoCleanContactCountLabel->Caption == "7" &&
           hAutoCleanHangUp.Off() && !bIsAutoOneCycle && !fMain->BtnOneCycle->Down;
}

// 去掉 // 與 /* */ 註解（字串裡的不動），跳過 #if 0 … #endif；'\r' 先去掉（gate 的 checkout 是 CRLF）
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
// 函式定義 `head`（整行開頭，後面沒有 ';'）到第一個只有 "}" 的行之間的活碼
std::string LiveBody(const std::string& text, const std::string& head)
{
    std::istringstream in(text);
    std::string l, out;
    std::vector<std::string> live = LiveLines(text);
    std::size_t i = 0;
    bool on = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (!on && l == head) { on = true; ++i; continue; }
        if (on) {
            if (l == "}") break;
            if (i < live.size()) out += live[i] + "\n";
        }
        ++i;
    }
    return out;
}
// 從 marker 那一行到檔尾的活碼
std::string LiveTail(const std::string& text, const std::string& marker)
{
    const std::string::size_type p = text.find(marker);
    if (p == std::string::npos) return std::string();
    std::string out;
    for (const std::string& l : LiveLines(text.substr(p))) out += l + "\n";
    return out;
}

}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);   // 當掉時 ctest 的輸出停在最後一行
    std::printf("test_b8_cl4_autoclean -- B8 CL-4 golden TfCleaning::btnStartAutoCleanClick -> TfShowBinSelect::btnAutoCleanClick "
                "(906 uCleaning.cpp:2869-2872 (V912 :2901-2904), 906 cShowBinSelect.cpp:2101-2166 (V912 :2248-2315))\n");
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), 0};   // golden LoadAutoCleanData 的配方路徑（common.cpp W906IniDataRedirect）
        if (!W906TestRequireCtestRedirects("B8_Cl4_StartAutoClean", rt))
            return 2;
    }
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[2];
    bool had[2];
    for (int i = 0; i < 2; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    const char* const kTrees[] = {"D:\\HT9045\\IniData", "D:\\HT9045\\data"};
    std::map<std::string, std::string> treeBefore[2];
    for (int i = 0; i < 2; ++i) ListTree(kTrees[i], "", &treeBefore[i]);

    const char* env = std::getenv("W906_INIDATA_ROOT");
    const bool sandbox = env && *env && DataPath.Pos(env) == 1;
    Check(sandbox, std::string("[0] W906_INIDATA_ROOT sandbox is on and DataPath is under it (") + DataPath.c_str() + ")");

    if (sandbox) {
        SystemStart = false; SoftStart = false;
        // golden LoadAutoCleanData／ReadWriteAutoCleanCount 在這幾個條件下寫死 D:\HT9045\IniData\DefineAutoClean（不走沙盒）⇒ 這支測試關掉
        CosFunction.bUseDefineAutoCleanOffset = false;
        if (CUSTOMER_CODE == CC_TSMC_TAINAN) CUSTOMER_CODE = 0;
        IniConfig.bE43_1_AutoCleanCountSaveFolder = false;
        IniConfig.bEnableAutoCleanFunction = true;
        const int made = W906_TestEnsureSimMotors();
        Check(made > 0 && MOT[MTrayX].Motor && MOT[MTestZ1].Motor, "[0] sim motors attached (" + std::to_string(made) + " axes)");
        FileRW_Cleaning_Boot();
        Check(filerw::FindPage(kTag) != nullptr, "[0] boot: page TestIF_File_Cleaning registered");
        cJSON* page = OpenPage();
        Check(page != nullptr, "[0] editlist.get (golden FormShow replaced by a no-op) = 200");
        {
            const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
            const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, "btnStartAutoClean") : nullptr;
            const cJSON* o = e ? cJSON_GetObjectItemCaseSensitive(e, "operable") : nullptr;
            Check(e && Str(e, "event") == "click" && Str(e, "golden") == kGolden,
                  "[0] events.btnStartAutoClean = click, golden \"" + Str(e, "golden") + "\"");
            Check(o && cJSON_IsTrue(o), "[0] events.btnStartAutoClean.operable (proxy + DFM parents grpCleanMode > grpCleanPara > pnlLeft)");
            Check(ev && cJSON_GetArraySize(ev) >= 84, "[0] the page lists " + std::to_string(ev ? cJSON_GetArraySize(ev) : 0) + " event controls (kCL_Events 84 rows)");
        }
        if (page) cJSON_Delete(page);
        Check(g_lockDepth == 0, "[0] FormLock balanced after boot + page open");

        Ack a;
        std::printf("[1] all golden checks pass, no part in the machine\n");
        Ready();
        Check(!HasICUnderMachine() && CheckIndexIsNormal(), "[1] precondition: HasICUnderMachine()==false, CheckIndexIsNormal()==true (live csystem.cpp bodies)");
        Click(&a);
        Check(a.sent, "[1] form.event btnStartAutoClean click -> ok " + a.err);
        Check(bRunAutoClean, "[1] bRunAutoClean = true (golden :2159 (V912 :2308))");
        Check(!hAutoCleanHangUp.Off(), "[1] hang-up watchdog on with Prod.iHangupMaxTime (golden :2160 (V912 :2309))");
        Check(iDoAutoCleanTask == 1 && iDoShuttle1AutoCleanTask == 1 && iDoShuttle2AutoCleanTask == 1 && iDoShuttleAutoCleanTask == 1 &&
              iDoIndexAutoCleanTask == 1, "[1] InitialAutoCleanTask / InitialShuttleAutoCleanTask / InitialIndexAutoCleanTask (golden :2114-2116 (V912 :2263-2265))");
        Check(iAutoClean_IndexContactCount == 0 && fMain->AutoCleanContactCountLabel->Caption == "0",
              "[1] contact count 0 and main-screen label \"0\" (golden :2112-2113 (V912 :2261-2262)), label=\"" + std::string(fMain->AutoCleanContactCountLabel->Caption.c_str()) + "\"");
        Check(a.nMsg == 0, "[1] no golden message (ack.messages " + a.msgs + ")");
        Check(!bIsAutoOneCycle && !fMain->BtnOneCycle->Down && iOneCycle == 0, "[1] no One Cycle (that arm is for a machine with parts)");
        Check(formevent::afterack::Pending() == 0, "[1] nothing queued for after the ack (golden has no YES/NO and no START)");
        Check(g_lockMax >= 1 && g_lockDepth == 0, "[1] the handler ran under FormLock and released it");
        {
            Ack b;
            Click(&b);
            Check(b.sent && bRunAutoClean && b.nMsg == 0, "[1] second click while bRunAutoClean: golden :2103 (V912 :2250) return (ok, no message)");
        }

        std::printf("[2] golden returns without doing anything\n");
        {
            struct Case { const char* what; void (*set)(); };
            const Case cases[] = {
                {"bRunAutoClean already (golden :2103 (V912 :2250))", [] { bRunAutoClean = true; }},
                {"bIsASMAutoOneCycle (golden :2105 (V912 :2252))", [] { bIsASMAutoOneCycle = true; }},
                {"iOneCycle!=0 (V912 :2254, RogerYang 20260810; not in 906 :2101-2166 -- kept as a #20 exception, RULINGS_20261002 #23-5 / Q77 Q-A)", [] { iOneCycle = 1; }},   //AI(W906-E030A) 20261003 [W906] (St01): label only; this case pins the kept guard
                {"IniConfig.bEnableAutoCleanFunction off (golden :2108 (V912 :2257))", [] { IniConfig.bEnableAutoCleanFunction = false; }},
                {"TestIF.iAutoClean_Function off (golden :2108 (V912 :2257))", [] { TestIF.iAutoClean_Function = 0; }},
                {"iAutoClean_Mode without M_MANUAL (golden :2110 (V912 :2259))", [] { TestIF.iAutoClean_Mode = 0xFF & ~M_MANUAL; }},
            };
            for (const Case& c : cases) {
                Ready();
                c.set();
                const bool run0 = bRunAutoClean;
                Click(&a);
                Check(a.sent && Untouched() && bRunAutoClean == run0 && a.nMsg == 0,
                      std::string("[2] ") + c.what + ": nothing done, no message (tasks=" + std::to_string(Tasks()) + ", msgs " + a.msgs + ")");
            }
        }

        std::printf("[3] no part: the four golden checks (message, bRunAutoClean not set)\n");
        {
            struct Case { const char* what; void (*set)(); const char* en; };
            const Case cases[] = {
                {"fAllMotorHome==false (golden :2124-2127 (V912 :2273-2276))", [] { fAllMotorHome = false; }, "Auto Clean Need Home"},
                {"MOT[MTrayX] < Prod.iXTrayEmpty (golden :2130-2134 (V912 :2279-2283))", [] { MOT[MTrayX].Motor->SetPosition(999); }, "Tray arm not Safe pos"},
                {"CheckIndexIsNormal()==false: MTestZ1 moving (golden :2136-2140 (V912 :2285-2289))", [] { MOT[MTestZ1].MovFlag = true; }, "Index arm Z axis is not in safe position."},
                {"IndexStatus!=Z1_Z2_Normal, [D51] off (golden :2150-2156 (V912 :2299-2305))", [] { IndexStatus = IndexIsBack; }, "Index arm axis is not in standy position."},
                {"[D51] on, IndexStatus!=IndexIsBack (golden :2142-2148 (V912 :2291-2297))", [] { IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear = true; }, "Index arm axis is not in standy position."},
            };
            for (const Case& c : cases) {
                Ready();
                c.set();
                Click(&a);
                Check(a.sent && !bRunAutoClean && a.nMsg == 1 && Has(a.msgs, c.en) && hAutoCleanHangUp.Off(),
                      std::string("[3] ") + c.what + ": message \"" + c.en + "\", bRunAutoClean stays false, no watchdog (msgs " + a.msgs + ")");
                Check(Tasks() == 5 && iAutoClean_IndexContactCount == 0,
                      std::string("[3] ") + c.what + ": golden oddity kept -- tasks registered and count zeroed before the check (:2112-2116, V912 :2261-2265)");
            }
            Ready();
            fAllMotorHome = false;
            Click(&a);
            Check(Has(a.msgs, "Auto Clean \xe9\x9c\x80\xe8\xa6\x81\xe6\xad\xb8\xe9\x9b\xb6"), "[3] the home message carries golden's Chinese text (S2 \"Auto Clean 需要歸零\")");
            Ready();
            IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear = true;
            IndexStatus = IndexIsBack;
            Click(&a);
            Check(a.sent && bRunAutoClean && a.nMsg == 0, "[3] [D51] on + IndexStatus==IndexIsBack: armed (golden :2144 (V912 :2293) passes)");
        }

        std::printf("[4] part in the machine: InitialAutoCleanAllTask (One Cycle first)\n");
        Ready();
        MOT[MMTrayZ].fHasTray = true;                    // golden HasICUnderMachine 第一項（csystem.cpp:13307）
        Check(HasICUnderMachine(), "[4] precondition: HasICUnderMachine()==true");
        fAllMotorHome = true;
        Click(&a);
        Check(a.sent && !bRunAutoClean && a.nMsg == 0, "[4] bRunAutoClean not set, no message (golden :2118-2121 (V912 :2267-2270))");
        Check(bIsAutoOneCycle && fMain->BtnOneCycle->Down && bManualOneCycle,
              "[4] InitialAutoCleanAllTask -> TfMain::BtnOneCycleClick (live cCleanOut.cpp body): bIsAutoOneCycle, BtnOneCycle->Down, bManualOneCycle");
        Check(Tasks() == 5, "[4] tasks registered (golden :2114-2116 (V912 :2263-2265) + 906 AutoClean.cpp:689-693 (V912 :767-771))");
        Ready();
        MOT[MMTrayZ].fHasTray = true;
        fAllMotorHome = false;                           // golden 有料那一臂不看歸零（BtnOneCycleClick 自己第一行看）
        Click(&a);
        Check(a.sent && !bRunAutoClean && a.nMsg == 0 && bIsAutoOneCycle && !fMain->BtnOneCycle->Down,
              "[4] part + not homed: golden skips the four checks, bIsAutoOneCycle=true, BtnOneCycleClick's own first line returns");
        MOT[MMTrayZ].fHasTray = false;

        std::printf("[5] running / not clickable\n");
        Ready();
        SystemStart = true;
        Click(&a);
        const bool sysRefused = !a.sent && a.err.compare(0, 7, "running") == 0;
        SystemStart = false;
        Check(sysRefused && Untouched() && !bRunAutoClean, "[5] SystemStart -> running, nothing done: " + a.err.substr(0, 60));
        Ready();
        SoftStart = true;
        Click(&a);
        const bool softRefused = !a.sent && a.err.compare(0, 7, "running") == 0;
        SoftStart = false;
        Check(softRefused && Untouched() && !bRunAutoClean, "[5] SoftStart -> running, nothing done");
        Ready();
        EL<TGroupBox>(kForm, "grpCleanPara")->Enabled = false;   // golden FormShow :1511 fSecurity->Insufficient(43,false)＝false
        Click(&a);
        EL<TGroupBox>(kForm, "grpCleanPara")->Enabled = true;
        Check(!a.sent && Has(a.err, "bad-payload") && Has(a.err, "cannot be operated") && Untouched() && !bRunAutoClean,
              "[5] grpCleanPara disabled (golden level 43) -> bad-payload, nothing done: " + a.err.substr(0, 70));
        Check(formevent::afterack::Pending() == 0 && g_lockDepth == 0, "[6] after every click: nothing queued after the ack, FormLock released");
        Ready();
        bRunAutoClean = false;
    }

    std::printf("[7] wiring ratchet (source, read-only)\n");
    if (argc < 3) {
        Check(false, "[7] argv[1] (port tree root) / argv[2] (web page dir) missing");
    } else {
        std::string gen, cpp, py, js, gpib, secs, fsb;
        const std::string root = argv[1];
        const bool rd = ReadAll(root + "/FileRW/TestIF_File_Cleaning.gen.inc", &gen) &&
                        ReadAll(root + "/FileRW/TestIF_File_Cleaning.cpp", &cpp) &&
                        ReadAll(root + "/tools/editlist/TestIF_File_Cleaning.py", &py) &&
                        ReadAll(root + "/TesterComm/Handler/HandlerGpibMsg.cpp", &gpib) &&
                        ReadAll(root + "/SECSGEM/uHGemHT9045.cpp", &secs) &&
                        ReadAll(root + "/forms/fShowBinSelect.h", &fsb) &&
                        ReadAll(std::string(argv[2]) + "/ht9045_cleaning_ev.js", &js);
        Check(rd, "[7] read the seven files");
        {
            const std::string body = LiveBody(gen, "static void CL_btnStartAutoCleanClick()");
            Check(Has(body, "W906_ShowBinSelect_btnAutoCleanClick();") && !Has(body, "fShowBinSelect->btnAutoCleanClick("),
                  "[7] gen.inc: CL_btnStartAutoCleanClick calls W906_ShowBinSelect_btnAutoCleanClick() as live code (golden V912 :2903 = 906 :2871 only inside #if 0)");
        }
        Check(CodeHas(gen, "{\"btnStartAutoClean\", \"click\", \"AutoClean/uCleaning.cpp:2901 TfCleaning::btnStartAutoCleanClick\", &CL_Ev_btnStartAutoCleanClick},") &&
              CodeHas(gen, "static void CL_Ev_btnStartAutoCleanClick(TControl* Sender) { (void)Sender; CL_btnStartAutoCleanClick(); }") &&
              CodeHas(gen, "void   W906_ShowBinSelect_btnAutoCleanClick();"),
              "[7] gen.inc: kCL_Events row + wrapper + declaration (live)");
        Check(CodeHas(cpp, "{{\"btnResetInterval\", \"tsSmart\"}, {\"btnStartAutoClean\", \"grpCleanMode\"}};") &&
              CodeHas(cpp, "EL<TButton>(kForm, \"btnStartAutoClean\");"),
              "[7] TestIF_File_Cleaning.cpp: DFM parent grpCleanMode and the proxy are created at boot (live)");
        {
            const std::string tail = LiveTail(cpp, "//AI(W906-B8-CL4) 20261001 [W906] (St01)\xef\xbc\x9a" "B8 CL-4");
            Check(Has(tail, "void W906_ShowBinSelect_btnAutoCleanClick()") && Has(tail, "CL4_GoldenBtnAutoCleanClick();") &&
                  Has(tail, "bRunAutoClean=true;") && Has(tail, "hAutoCleanHangUp.SetSecAndOn(Prod.iHangupMaxTime);") &&
                  Has(tail, "RecordProcess(\"AUTO CLEAN pressed\");") && Has(tail, "InitialAutoCleanAllTask();") &&
                  Has(tail, "if(HasICUnderMachine())") && Has(tail, "if(CheckIndexIsNormal()==false)"),
                  "[7] TestIF_File_Cleaning.cpp tail: the golden body is live (bRunAutoClean, watchdog, RecordProcess, InitialAutoCleanAllTask)");
            Check(!tail.empty() && !Has(tail, "afterack::Defer(") && !Has(tail, "W906_RemoteRunStart(") && !Has(tail, "->Start(") &&
                  !Has(tail, "ShowMyMessage("),
                  "[7] tail: no after-ack queue, no START, messages only through filerw::ELMessage (golden has no dialog that waits)");
        }
        Check(Has(py, "('btnStartAutoClean', 'click', 'btnStartAutoCleanClick')") && Has(py, "'W906_ShowBinSelect_btnAutoCleanClick();')]") &&
              Has(py, "_sa_bc = _expect(L('btnStartAutoCleanClick', 'btnAutoCleanClick('), 'fShowBinSelect->btnAutoCleanClick(fShowBinSelect);')"),
              "[7] TestIF_File_Cleaning.py: event row, replace, golden V912 :2903 (906 :2871) pinned");
        Check(Has(js, "btnStartAutoClean: 'CL-4") && Has(js, "if (item.control === 'btnStartAutoClean')"),
              "[7] page ht9045_cleaning_ev.js: reopens btnStartAutoClean (PORTED) and explains the ack");
        Check(!CodeHas(gpib, "fShowBinSelect->btnAutoCleanClick(") && !CodeHas(secs, "fShowBinSelect->btnAutoCleanClick("),
              "[7] GPIB (HandlerGpibMsg.cpp:625) and SECS (uHGemHT9045.cpp GATE G18) callers still gated -- todo D-033, not this row");
        Check(!CodeHas(fsb, "btnAutoCleanClick("), "[7] forms/fShowBinSelect.h (jimmychiu) untouched: still no btnAutoCleanClick member");
    }

    for (int i = 0; i < 2; ++i) {
        std::string after;
        const bool has = ReadAll(kGuard[i], &after);
        Check(has == had[i] && after == before[i], std::string("[guard] unchanged: ") + kGuard[i]);
    }
    for (int i = 0; i < 2; ++i) {
        std::map<std::string, std::string> now;
        ListTree(kTrees[i], "", &now);
        Check(now == treeBefore[i], std::string("[guard] file list (name, size, mtime) unchanged: ") + kTrees[i] + " (" + std::to_string(now.size()) + " entries)");
    }
    std::printf("test_b8_cl4_autoclean: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
