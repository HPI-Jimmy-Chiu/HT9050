// =============================================================================
//  test_b8_ct3b_contactstartpause.cpp -- B8 CT-3b': Contact page START / PAUSE
//
//  //AI(W906-B8-CT3B) 20261001 [W906] (St01) new file. Dispatch D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3"
//    item 8 second slice (P-2 START entry + census, P-3 running exceptions); Jimmy RULINGS_20261001 #0 (translate per golden and wire it);
//    Steven 20261001 09:4x (on-machine checks: EastSun).
//  golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003] (V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy in brackets):
//    cContact.cpp btnStartClick 906 :13965-13970 (V912 :14072-14077) = fMain->BtnStartClick(fMain); edTorue0 / edTorue1 ->Text="10";
//    cContact.cpp btnPauseClick 906 :13972-13975 (V912 :14079-14082) = fMain->BtnPauseClick(fMain);
//    main.cpp TfMain::BtnStartClick 906 :6261-6323 (V912 :6529-6593) (starts only with CosFunction.bEnableSoftWareControlButton, or in
//      SOFT_SIMULTE after CheckAutoOnlySetOneBin 906 :32523-32552 (V912 :33616-33645; the tree after c2f6c75a :33646-33675) / AAL flags 906 :6275-6306 (V912 :6545-6576) /
//      [I52] AQL 906 :6309-6312 (V912 :6579-6582) / [P28] CheckAuto1OnlyBin1 906 :32502-32521 (V912 :33595-33614; after c2f6c75a :33625-33644));
//    //AI(W906-E030A) 20261003 [W906] (St01): the V912-only Teradyne-US level return (V912 :6533-6534) is not in 906 :6263-6266 -> removed
//      (Jimmy RULINGS_20261002 #23 item 5 / Q77: Q-B dropped); [2] now pins the 906 behaviour (a low-level Teradyne-US user STARTs).
//    main.cpp TfMain::BtnPauseClick 906 :6965-6971 (V912 :7387-7393) -> Pause("BtnPauseClick") 906 :6325 (V912 :6595); cContact.cpp FormShow [D16] 906 :1601-1602 (V912 :1628-1629), SOFT_SIMULTE 906 :1635-1636 (V912 :1662-1663).
//  Under test: the same FileRW/DeviceForm_File.cpp wb_serve compiles (tail: W906_Contact_B8BtnStartClick / W906_Contact_B8BtnPauseClick /
//    Ct3bBtnStartClick / Ct3bRun; EvB3Table) + the generated FileRW/DeviceForm_File.gen.inc (DF_btnStartClick, DF_btnPauseClick, kDF_Events
//    rows 23-24), through the real WS entry W906_FormEvent (FileRW/_FormEvent.cpp: runexc rows 18-19, the after-ack queue, ack "afterAck")
//    and W906_FormEventRunAfterAck (what the wb_serve form.event arm calls after CompleteCommand); the real TfMain::BtnPauseClick / Pause
//    (forms/fMain.cpp) and the real fMain->edTorue0 / edTorue1 (forms/fMain.h:1262-1263). The START / PAUSE seat W906_RemoteRun
//    (forms/fMain.h tail) is a recorder here (wb_serve installs TfMainWeb::StartFromWeb / PauseFromWeb).
//    [0] guard (ctest sandbox), boot, first open: events lists btnStart / btnPause (click, golden names), operable with [D16]
//    [1] START, bEnableSoftWareControlButton: one after-ack item, nothing inside the handler (no START, torque boxes untouched);
//        after the reply: START once, Func "bEnableSoftWareControlButton", outside FormLock, BEFORE the torque boxes; then edTorue0/1 "10"
//    [2] Teradyne-US below the HonPrec level: START like any other customer (906 :6263-6266 has no level check; the V912-only return is gone)
//    [3] no software-control flag: ship build -> no START at all, torque boxes "10" (golden); SOFT_SIMULTE -> Func "BtnStartClick_SOFT_SIMULTE",
//        CheckAutoOnlySetOneBin / [P28] CheckAuto1OnlyBin1 refuse with their message, AAL flags as golden
//    [4] after-ack rules: RunAfterAck(false) drops; an item not run is voided by the next form.event
//    [5] PAUSE: queued, after the reply TfMain::BtnPauseClick -> Pause("BtnPauseClick") -> the PAUSE seat once, outside FormLock;
//        seat not installed -> no crash
//    [6] visibility: [D16] off -> ship: hidden, refused (bad-payload), nothing queued; SOFT_SIMULTE: still operable
//    [7] running: runexc has the 7 TfContact rows (+ CT-3d's 2 OTD rows, Steven Q65 = B); START / PAUSE rows SystemStart + SoftStart, golden cites; no page table -> running;
//        Contact open -> queued and run (TfMain::Start's own 906 main.cpp:5871 (V912 :6137) SystemStart refusal lives in StartFromWeb, not here)
//    [8] why PAUSE also waits for the reply: a browser-waiting box is reachable from PauseFromWeb (SetRunStartMode -> ShowMyMessage)
//    [9] source ratchet (argv[1] = port tree, argv[2] = web\page, read-only; comments and '\r' stripped, #if 0 skipped)
//  NOT COVERED: TfMainWeb::StartFromWeb / PauseFromWeb themselves (the START census only counts call sites; WebStart.cpp has its own
//    tests); the page's own JS (ctest B8_Ct3b_ContactPage); the golden OEE fProductionInfo->ClickPause() (not ported, forms/fMain.h:484-488).
//  Writes: only the ctest sandbox (W906_INIDATA_ROOT, the per-test Gerneral.ini, machine_log_scratch); refuses outside ctest
//    (w906_ctest_guard.h). Before / after compare: D:\HT9045\system\Gerneral.ini, ContactInfo.ini, Contact.ini, D:\HT9045\config\config.ini
//    and the D:\HT9045\IniData file list.
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
#include "mysensor.h"
#include "canary_support.h"     // W906_ShowMyMessage_Count / LastS1 (the golden check messages)
#include "forms/fMain.h"        // fMain (edTorue0 / edTorue1, BtnPauseClick, W906_PauseLastFunc), W906_RemoteRun
#include "w906_ctest_guard.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h virtual defaults (not this file's code)
#include "Motor/mymotor.h"
#include "w906_test_motors.h"
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

extern void FileRW_Contact_Boot();
extern bool (*W906_FormFShowHook)(const char* goldenForm);   // csystem.h:421; W906_FormShowing(obj,false) = hook(obj)
extern AnsiString DataPath;
extern AnsiString DefaultPath;
extern AnsiString asGeneralPath;
extern int iMaxLevelItem;   // cSecurity.cpp:47 (wb_serve W906_SecurityBoot sets 180)
void OpenGeneralIniFile();    // common.h (golden common.cpp:1408): golden ReadFile needs it open; asGeneralPath = this test's sandbox Gerneral.ini
void CloseGeneralIniFile();
int FileRW_InArmSuckShtRow();   // FileRW/_KitSuck.cpp tail (golden main.cpp:6556 InArmSuck.iShtRow)

// Link-only (not under test), same as tests/test_b8_ct3a_contactflags.cpp.
void FileRW_IniConfig_ChangeCBListProperty() {}
const char* W906_Main_sbContactClickOpen() { return nullptr; }
void FileRW_LdUld_Boot() { std::printf("link-only stand-in reached: FileRW_LdUld_Boot\n"); std::abort(); }
void FileRW_LdUld_ReadFile() { std::printf("link-only stand-in reached: FileRW_LdUld_ReadFile\n"); std::abort(); }
namespace {
int g_lockDepth = 0;
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
void FormLock() { ++g_lockDepth; }
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

const char* const kTag = "DeviceForm_File";
const char* g_open = nullptr;   // the fake page table: which golden form is open on an HMI
bool FakePageTable(const char* form) { return g_open && form && std::strcmp(form, g_open) == 0; }
bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }
bool Has(const std::string& s, const std::string& n) { return s.find(n) != std::string::npos; }

// ---- the START / PAUSE seat recorders (wb_serve installs TfMainWeb::StartFromWeb / PauseFromWeb) ----
int g_startCalls = 0, g_startLock = -1;
std::string g_startFunc, g_torqueAtStart;
bool FakeStart(AnsiString f)
{
    ++g_startCalls;
    g_startFunc = f.c_str();
    g_startLock = g_lockDepth;
    g_torqueAtStart = std::string(fMain->edTorue0->Text.c_str()) + "/" + fMain->edTorue1->Text.c_str();
    return true;
}
int g_pauseCalls = 0, g_pauseLock = -1;
std::string g_pauseFunc;
bool FakePause(AnsiString f)
{
    ++g_pauseCalls;
    g_pauseFunc = f.c_str();
    g_pauseLock = g_lockDepth;
    return true;
}
void ResetRec()
{
    g_startCalls = 0; g_startLock = -1; g_startFunc.clear(); g_torqueAtStart.clear();
    g_pauseCalls = 0; g_pauseLock = -1; g_pauseFunc.clear();
}
void SetTorqueBoxes(const char* t0, const char* t1) { fMain->edTorue0->Text = t0; fMain->edTorue1->Text = t1; }
std::string TorqueNow() { return std::string(fMain->edTorue0->Text.c_str()) + "/" + fMain->edTorue1->Text.c_str(); }

std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}
// editlist.get (PageJson runs the real golden FormShow); returns the parsed response
cJSON* Open()
{
    const filerw::PageDesc* d = filerw::FindPage(kTag);
    if (!d) return nullptr;
    std::string json;
    if (filerw::PageJson(*d, &json) != 200) { std::printf("    PageJson: %s\n", json.c_str()); return nullptr; }
    return cJSON_Parse(json.c_str());
}
bool OpenOk() { cJSON* p = Open(); const bool ok = p != nullptr; if (p) cJSON_Delete(p); return ok; }
bool Operable(const cJSON* page, const char* id)
{
    const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
    const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, id) : nullptr;
    const cJSON* o = e ? cJSON_GetObjectItemCaseSensitive(e, "operable") : nullptr;
    return o && cJSON_IsTrue(o);
}
struct Ack {
    bool sent = false;
    std::string err, raw;
};
// one WS form.event click, the shape D:\HT9045\web\page\ht9045_contact_ev.js sends for a TButton
Ack Click(const std::string& control)
{
    Ack a;
    const std::string v = "{\"form\":\"TfContact\",\"control\":\"" + control + "\",\"event\":\"click\"}";
    a.sent = W906_FormEvent(kTag, v, &a.raw, &a.err);
    return a;
}
std::string AckGolden(const Ack& a)
{
    cJSON* j = a.sent ? cJSON_Parse(a.raw.c_str()) : nullptr;
    const std::string g = Str(j, "golden");
    if (j) cJSON_Delete(j);
    return g;
}
// ack "afterAck": item count (-1 = not an array), the first one into *first
int AfterAck(const Ack& a, std::string* first)
{
    first->clear();
    cJSON* j = a.sent ? cJSON_Parse(a.raw.c_str()) : nullptr;
    const cJSON* arr = j ? cJSON_GetObjectItemCaseSensitive(j, "afterAck") : nullptr;
    int n = -1;
    if (!arr) n = 0;
    else if (cJSON_IsArray(arr)) {
        n = cJSON_GetArraySize(arr);
        const cJSON* f = cJSON_GetArrayItem(arr, 0);
        if (f && cJSON_IsString(f)) *first = f->valuestring;
    }
    if (j) cJSON_Delete(j);
    return n;
}
// click START, then what the wb_serve form.event arm does after CompleteCommand
void StartAndRun(Ack* a, int* n, std::string* first)
{
    *a = Click("btnStart");
    *n = AfterAck(*a, first);
    W906_FormEventRunAfterAck(a->sent);
}

void Quiet()   // the manual-key inputs the flow also reads: off (same as tests/test_b8_ct3a_contactflags.cpp)
{
    Sen[SnRKManualStep].Enable = false;
    Sen[SnRKManualTStart].Enable = false;
    Sen[SnRKCoverOpen].Enable = false;
    Sen[SnRKSafeLock].Enable = false;
    bButtonManualStep = false; bButtonManualTStart = false; bSTEP = false; bTSTART = false;
    CosFunction.bEnableSoftWareControlButton = false;
}

// ---- source ratchet helpers: strip /* */ and // comments (not inside "..." or '...'), drop '\r', skip #if 0 ... #endif ----
std::vector<std::string> LiveLines(const std::string& text)
{
    std::vector<std::string> out;
    bool block = false, gated = false;
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        std::string code;
        char q = 0;
        for (std::size_t i = 0; i < l.size(); ++i) {
            const char c = l[i];
            if (block) { if (c == '*' && i + 1 < l.size() && l[i + 1] == '/') { block = false; ++i; } continue; }
            if (q) { code += c; if (c == '\\' && i + 1 < l.size()) { code += l[++i]; continue; } if (c == q) q = 0; continue; }
            if (c == '/' && i + 1 < l.size() && l[i + 1] == '/') break;
            if (c == '/' && i + 1 < l.size() && l[i + 1] == '*') { block = true; ++i; continue; }
            if (c == '"' || c == '\'') q = c;
            code += c;
        }
        std::string t = code;
        const std::string::size_type b = t.find_first_not_of(" \t");
        t = b == std::string::npos ? std::string() : t.substr(b);
        if (t.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (t.compare(0, 6, "#endif") == 0) gated = false; continue; }
        out.push_back(code);
    }
    return out;
}
bool LiveHas(const std::vector<std::string>& L, const std::string& needle)
{
    for (const std::string& l : L) if (l.find(needle) != std::string::npos) return true;
    return false;
}
int LiveCount(const std::vector<std::string>& L, const std::string& needle)
{
    int n = 0;
    for (const std::string& l : L)
        for (std::string::size_type p = l.find(needle); p != std::string::npos; p = l.find(needle, p + needle.size())) ++n;
    return n;
}
// the first live line index containing needle, -1 = none
int LiveAt(const std::vector<std::string>& L, const std::string& needle)
{
    for (std::size_t i = 0; i < L.size(); ++i) if (L[i].find(needle) != std::string::npos) return (int)i;
    return -1;
}
// a function body (from the line that contains head up to the first line that is exactly "}"), live lines only
std::vector<std::string> Body(const std::vector<std::string>& L, const std::string& head)
{
    std::vector<std::string> out;
    bool in = false;
    for (const std::string& l : L) {
        if (!in) { if (l.find(head) != std::string::npos && l.find(';') == std::string::npos) in = true; continue; }
        if (l == "}") break;
        out.push_back(l);
    }
    return out;
}

}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("test_b8_ct3b_contactstartpause -- B8 CT-3b' golden TfContact::btnStartClick / btnPauseClick (906 cContact.cpp:13965 (V912 :14072) / :13972 (V912 :14079))\n");
#ifdef SOFT_SIMULTE
    std::printf("  build: SOFT_SIMULTE (golden BtnStartClick's simulator branch is compiled)\n");
#else
    std::printf("  build: ship (no SOFT_SIMULTE: golden BtnStartClick starts only with CosFunction.bEnableSoftWareControlButton)\n");
#endif
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("B8_Ct3b_ContactStartPause", rt))
            return 2;
    }
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\system\\ContactInfo.ini", "D:\\HT9045\\system\\Contact.ini",
                                  "D:\\HT9045\\config\\config.ini"};
    std::string before[4];
    bool had[4];
    for (int i = 0; i < 4; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    std::map<std::string, std::string> treeBefore;
    ListTree("D:\\HT9045\\IniData", "", &treeBefore);

    Ack a;
    int n = 0;
    std::string first;

    // ---------------------------------------------------------------- [0]
    std::printf("[0] boot + first open (golden FormShow)\n");
    SystemStart = false; SoftStart = false;
    CosFunction.bContactHeightSaveToContactIni = false;   // golden ReadFile would read D:\HT9045\system\Contact.ini (not sandboxed)
    CosFunction.bUseDynamicKitDiameter = false;           // golden TfContact ctor would read D:\HT9045\system\ContactInfo.ini
    CUSTOMER_CODE = 0;
    IniConfig.bD16_StepContactTest = true;
    Tri_Temp_Machine = 0;
    iMaxLevelItem = 180;
    for (int i = 0; i < 256; ++i) LevelSet.AccessLevel[i] = 0;
    AccessLevel = 4;
    Quiet();
    CosFunction.bUsePassBinOnlyCanSetOneBin = false;      // golden CheckAutoOnlySetOneBin off ([3] turns it on)
    IniConfig.bP28Auto1OnlyBin1 = false;
    IniConfig.bI52_bAQLSortMode = false;
    CosFunction.bUseBarcodeAutoAdjustLight = false;
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0 && MOT[MTrayX].Motor && MOT[MTestZ1].Motor, "[0] sim motors attached (" + std::to_string(made) + " axes)");
    Check(fMain != nullptr && fMain->edTorue0 != nullptr && fMain->edTorue1 != nullptr,
          "[0] fMain and its torque boxes edTorue0 / edTorue1 exist (forms/fMain.h:1262-1263)");
    W906_RemoteRun.Start = &FakeStart;
    W906_RemoteRun.Pause = &FakePause;
    OpenGeneralIniFile();
    FileRW_Contact_Boot();
    W906_FormFShowHook = nullptr;
    {
        cJSON* page = Open();
        Check(page != nullptr, "[0] editlist.get DeviceForm_File (golden FormShow) = 200");
        const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
        const cJSON* s = ev ? cJSON_GetObjectItemCaseSensitive(ev, "btnStart") : nullptr;
        const cJSON* p = ev ? cJSON_GetObjectItemCaseSensitive(ev, "btnPause") : nullptr;
        Check(s && Str(s, "event") == "click" && Str(s, "golden") == "cContact.cpp:14072 TfContact::btnStartClick" &&
              p && Str(p, "event") == "click" && Str(p, "golden") == "cContact.cpp:14079 TfContact::btnPauseClick",
              "[0] events: btnStart -> golden btnStartClick (:13965, V912 :14072), btnPause -> golden btnPauseClick (:13972, V912 :14079)");
        Check(Operable(page, "btnStart") && Operable(page, "btnPause"), "[0] [D16] on: START / PAUSE operable (golden FormShow :1601-1602 (V912 :1628-1629))");
        if (page) cJSON_Delete(page);
        Check(g_lockDepth == 0 && formevent::afterack::Pending() == 0, "[0] FormLock balanced, nothing queued");
    }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] START with CosFunction.bEnableSoftWareControlButton (golden BtnStartClick 906 :6263-6266 (V912 :6531-6536, which also has the V912-only Teradyne-US return :6533-6534)) -- after the reply, golden order\n");
    {
        CosFunction.bEnableSoftWareControlButton = true;
        ResetRec();
        SetTorqueBoxes("x0", "x1");
        a = Click("btnStart");
        n = AfterAck(a, &first);
        Check(a.sent && AckGolden(a) == "cContact.cpp:14072 TfContact::btnStartClick", "[1] form.event btnStart click -> ok, golden btnStartClick " + a.err);
        Check(n == 1 && Has(first, "cContact.cpp:13967-13969 (V912 :14074-14076)") && Has(first, "TfMain::BtnStartClick") && Has(first, "edTorue0"),
              "[1] ack.afterAck: one item naming golden cContact.cpp:13967-13969 (V912 :14074-14076) (BtnStartClick, then the torque boxes): " + first);
        Check(g_startCalls == 0 && formevent::afterack::Pending() == 1 && TorqueNow() == "x0/x1",
              "[1] inside the handler: no START, torque boxes untouched (queued, waiting for the reply)");
        Check(g_lockDepth == 0, "[1] the handler released FormLock");
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 1 && g_startFunc == "bEnableSoftWareControlButton",
              "[1] after the reply: the START seat once, Func \"bEnableSoftWareControlButton\" (golden 906 main.cpp:6265 (V912 :6535)) -- got " +
              std::to_string(g_startCalls) + " '" + g_startFunc + "'");
        Check(g_startLock == 0, "[1] START ran outside FormLock (depth " + std::to_string(g_startLock) + ")");
        Check(g_torqueAtStart == "x0/x1", "[1] golden order: BtnStartClick (:13967, V912 :14074) runs before the torque boxes are set (:13968-13969, V912 :14075-14076) -- at START: " + g_torqueAtStart);
        Check(TorqueNow() == "10/10", "[1] then fMain->edTorue0 / edTorue1 ->Text = \"10\" (golden :13968-13969 (V912 :14075-14076)): " + TorqueNow());
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 1 && formevent::afterack::Pending() == 0, "[1] a second RunAfterAck does not start again");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] Teradyne-US below the HonPrec level (906 main.cpp:6263-6266 has no level check; V912 :6533-6534 had one -- removed, Q-B)\n");
    {
        // the page is reopened at each level (RunPageEvent wants the level it was opened at); the customer code is set after the open
        //   (only BtnStartClick should see Teradyne-US, not a customer-specific golden FormShow)
        //AI(W906-E030A) 20261003 [W906] (St01): this case expected the V912 return (no START); Jimmy RULINGS_20261002 #23 item 5 / Q77 (Q-B
        //   dropped, follow 906) -> it now expects START, Func "bEnableSoftWareControlButton" (906 :6265 / V912 :6535)
        AccessLevel = iDefHonPrecLevel - 1;
        Check(OpenOk(), "[2] reopen at the lower level");
        CUSTOMER_CODE = CC_TERADYNE_US;
        ResetRec();
        SetTorqueBoxes("y0", "y1");
        StartAndRun(&a, &n, &first);
        Check(a.sent && n == 1 && g_startCalls == 1 && g_startFunc == "bEnableSoftWareControlButton",
              "[2] Teradyne-US, AccessLevel < iDefHonPrecLevel: START, Func \"bEnableSoftWareControlButton\" (906 :6265 -- no V912 level return) -- got " +
              std::to_string(g_startCalls) + " '" + g_startFunc + "' " + a.err);
        Check(TorqueNow() == "10/10", "[2] ... then the torque boxes are set to 10 (golden 906 cContact.cpp:13968-13969 / V912 :14075-14076)");
        CUSTOMER_CODE = 0;
        AccessLevel = iDefHonPrecLevel;
        Check(OpenOk(), "[2] reopen at the HonPrec level");
        CUSTOMER_CODE = CC_TERADYNE_US;
        ResetRec();
        StartAndRun(&a, &n, &first);
        Check(a.sent && g_startCalls == 1 && g_startFunc == "bEnableSoftWareControlButton", "[2] Teradyne-US, AccessLevel == iDefHonPrecLevel: START");
        CUSTOMER_CODE = 0;
        ResetRec();
        AccessLevel = iDefHonPrecLevel - 1;
        Check(OpenOk(), "[2] another customer at the lower level");
        StartAndRun(&a, &n, &first);
        Check(a.sent && g_startCalls == 1, "[2] another customer at the lower level -> START (906 checks no level for any customer)");
        AccessLevel = 4;
        Check(OpenOk(), "[2] level back");
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] no software-control flag (golden :6267-6322 (V912 :6537-6592))\n");
    {
        CosFunction.bEnableSoftWareControlButton = false;
        ResetRec();
        SetTorqueBoxes("z0", "z1");
        StartAndRun(&a, &n, &first);
        Check(a.sent && n == 1, "[3] still exactly one after-ack item (golden runs the whole click) " + a.err);
#ifndef SOFT_SIMULTE
        Check(g_startCalls == 0, "[3] ship build: golden BtnStartClick does nothing -- no START (a real machine starts from the panel START key)");
        Check(TorqueNow() == "10/10", "[3] ship build: the click only sets the torque boxes to 10 (golden :13968-13969 (V912 :14075-14076))");
#else
        Check(g_startCalls == 1 && g_startFunc == "BtnStartClick_SOFT_SIMULTE" && g_startLock == 0,
              "[3] SOFT_SIMULTE: START, Func \"BtnStartClick_SOFT_SIMULTE\" (golden :6320 (V912 :6590)), outside FormLock -- got '" + g_startFunc + "'");
        Check(TorqueNow() == "10/10", "[3] SOFT_SIMULTE: torque boxes 10");
        // golden CheckAutoOnlySetOneBin (:32523-32552（V912 :33616-33645）): an Auto tray set as Pass with two bins -> message, no START
        const int pass0 = Prod.iIsPassT6[0], cat0 = Prod.iT6CatData[0], cat1 = Prod.iT6CatData[1], pos0 = Prod.iT6PosCate[0], pos1 = Prod.iT6PosCate[1];
        const int bins0 = iTestBinCount;
        CosFunction.bUsePassBinOnlyCanSetOneBin = true;
        Prod.iIsPassT6[0] = 1; Prod.iT6CatData[0] = 0; Prod.iT6CatData[1] = 0;
        if (iTestBinCount < 2) iTestBinCount = 2;
        ResetRec(); W906_ShowMyMessage_Reset(); SetTorqueBoxes("z0", "z1");
        StartAndRun(&a, &n, &first);
        Check(a.sent && g_startCalls == 0 && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1.c_str(), "Pass Bin Only Can Set One Bin."),
              std::string("[3] SOFT_SIMULTE, CheckAutoOnlySetOneBin refuses: its message (golden :32544 (V912 :33637)), no START -- ") + W906_ShowMyMessage_LastS1.c_str());
        Check(TorqueNow() == "10/10", "[3] ... torque boxes still 10");
        CosFunction.bUsePassBinOnlyCanSetOneBin = false;
        Prod.iIsPassT6[0] = pass0; Prod.iT6CatData[0] = cat0; Prod.iT6CatData[1] = cat1;
        // golden [P28] CheckAuto1OnlyBin1 (:32502-32521（V912 :33595-33614）): bin 0 mapped to Auto1 -> message, no START
        IniConfig.bP28Auto1OnlyBin1 = true;
        Prod.iT6PosCate[0] = ePosAuto1; Prod.iT6PosCate[1] = ePosAuto1;
        ResetRec(); W906_ShowMyMessage_Reset();
        StartAndRun(&a, &n, &first);
        Check(a.sent && g_startCalls == 0 && W906_ShowMyMessage_Count == 1 && Has(W906_ShowMyMessage_LastS1.c_str(), "Only Bin 1 is allowed"),
              std::string("[3] SOFT_SIMULTE, [P28] CheckAuto1OnlyBin1 refuses: its message (golden :32516 (V912 :33609)), no START -- ") + W906_ShowMyMessage_LastS1.c_str());
        IniConfig.bP28Auto1OnlyBin1 = false;
        Prod.iT6PosCate[0] = pos0; Prod.iT6PosCate[1] = pos1;
        iTestBinCount = bins0;
        // golden AAL flags (:6275-6306（V912 :6545-6576）)
        const bool tif0 = TestIF_File.bUseBarcodeAutoAdjustLight;
        const int mode0 = TestIF_File.iShuttleMode, sel0 = TestIF_File.iShuttle_Sel;
        const AnsiString rsm0 = fMain->cbRunStartMode->Text;
        CosFunction.bUseBarcodeAutoAdjustLight = true; TestIF_File.bUseBarcodeAutoAdjustLight = true;
        TestIF_File.iShuttleMode = 1; TestIF_File.iShuttle_Sel = 0;
        fMain->cbRunStartMode->Text = "Continue Start";
        bStartAutoAdjustLight = false;
        for (int i = 0; i < 4; ++i) bBarcodeNeedAutoAdjust[i] = false;
        ResetRec();
        StartAndRun(&a, &n, &first);
        Check(a.sent && g_startCalls == 1 && !bStartAutoAdjustLight && !bBarcodeNeedAutoAdjust[0],
              "[3] SOFT_SIMULTE, AAL on but the run mode is not an Initial one: flags untouched (golden :6278 (V912 :6548)), START");
        fMain->cbRunStartMode->Text = "Initial Start";
        ResetRec();
        StartAndRun(&a, &n, &first);
        const bool row1 = FileRW_InArmSuckShtRow() == 1;
        Check(a.sent && g_startCalls == 1 && bStartAutoAdjustLight && bBarcodeNeedAutoAdjust[0] && bBarcodeNeedAutoAdjust[1] == !row1 &&
              !bBarcodeNeedAutoAdjust[2] && !bBarcodeNeedAutoAdjust[3],
              std::string("[3] SOFT_SIMULTE, AAL + Initial: bStartAutoAdjustLight, [0] on, [1] ") + (row1 ? "off (InArmSuck.iShtRow==1, golden :6286 (V912 :6556))" : "on") +
              ", Front Arm Only -> [2] [3] off (golden :6292-6298 (V912 :6562-6568)), START");
        TestIF_File.iShuttle_Sel = 1;
        bStartAutoAdjustLight = false;
        for (int i = 0; i < 4; ++i) bBarcodeNeedAutoAdjust[i] = false;
        ResetRec();
        StartAndRun(&a, &n, &first);
        Check(bStartAutoAdjustLight && !bBarcodeNeedAutoAdjust[0] && !bBarcodeNeedAutoAdjust[1] && bBarcodeNeedAutoAdjust[2] == !row1 && bBarcodeNeedAutoAdjust[3],
              "[3] SOFT_SIMULTE, AAL + Initial + Rear Arm Only -> [0] [1] off (golden :6299-6303 (V912 :6569-6573))");
        CosFunction.bUseBarcodeAutoAdjustLight = false; TestIF_File.bUseBarcodeAutoAdjustLight = tif0;
        TestIF_File.iShuttleMode = mode0; TestIF_File.iShuttle_Sel = sel0;
        fMain->cbRunStartMode->Text = rsm0;
        bStartAutoAdjustLight = false;
        for (int i = 0; i < 4; ++i) bBarcodeNeedAutoAdjust[i] = false;
#endif
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] after-ack rules (FileRW/_FormEvent.cpp tail)\n");
    {
        CosFunction.bEnableSoftWareControlButton = true;
        ResetRec();
        SetTorqueBoxes("w0", "w1");
        a = Click("btnStart");
        W906_FormEventRunAfterAck(false);
        Check(a.sent && g_startCalls == 0 && formevent::afterack::Pending() == 0 && TorqueNow() == "w0/w1",
              "[4] RunAfterAck(false) (the form.event failed) drops the whole click: no START, torque boxes untouched");
        a = Click("btnStart");
        Check(a.sent && formevent::afterack::Pending() == 1, "[4] START queued ...");
        a = Click("btnPause");
        Check(a.sent && formevent::afterack::Pending() == 1, "[4] ... a next form.event (PAUSE) voids the START that was not run");
        W906_FormEventRunAfterAck(true);
        Check(g_startCalls == 0 && g_pauseCalls == 1, "[4] only the PAUSE runs (never a late START)");
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] PAUSE: golden TfMain::BtnPauseClick -> Pause(\"BtnPauseClick\") (906 main.cpp:6965-6971, V912 :7387-7393), after the reply\n");
    {
        ResetRec();
        const int bpc0 = fMain->W906_BtnPauseClickCallCount, pc0 = fMain->W906_PauseCallCount;
        fMain->W906_PauseLastFunc = "";
        a = Click("btnPause");
        n = AfterAck(a, &first);
        Check(a.sent && AckGolden(a) == "cContact.cpp:14079 TfContact::btnPauseClick", "[5] form.event btnPause click -> ok, golden btnPauseClick " + a.err);
        Check(n == 1 && Has(first, "cContact.cpp:13974 (V912 :14081)") && Has(first, "TfMain::BtnPauseClick"),
              "[5] ack.afterAck: one item naming golden cContact.cpp:13974 (V912 :14081): " + first);
        Check(g_pauseCalls == 0 && fMain->W906_BtnPauseClickCallCount == bpc0, "[5] inside the handler: no pause yet");
        W906_FormEventRunAfterAck(true);
        Check(fMain->W906_BtnPauseClickCallCount == bpc0 + 1 && fMain->W906_PauseCallCount == pc0 + 1 && fMain->W906_PauseLastFunc == "BtnPauseClick",
              "[5] after the reply: the port's TfMain::BtnPauseClick ran and forwarded Pause(\"BtnPauseClick\") (forms/fMain.cpp:497 / :246)");
        Check(g_pauseCalls == 1 && g_pauseFunc == "BtnPauseClick" && g_pauseLock == 0,
              "[5] ... which reached the PAUSE seat once with Func \"BtnPauseClick\", outside FormLock (depth " + std::to_string(g_pauseLock) + ")");
        Check(g_startCalls == 0, "[5] PAUSE starts nothing");
        W906_RemoteRun.Pause = nullptr;
        a = Click("btnPause");
        W906_FormEventRunAfterAck(a.sent);
        Check(a.sent && g_pauseCalls == 1 && fMain->W906_BtnPauseClickCallCount == bpc0 + 2,
              "[5] seat not installed (ctest / other binaries): BtnPauseClick still runs, no crash, no pause");
        W906_RemoteRun.Pause = &FakePause;
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] visibility: [D16] Step Contact Test (FormShow :1601-1602 (V912 :1628-1629)); SOFT_SIMULTE always shows them (:1635-1636, V912 :1662-1663)\n");
    {
        IniConfig.bD16_StepContactTest = false;
        cJSON* page = Open();
#ifdef SOFT_SIMULTE
        const bool shown = true;
#else
        const bool shown = false;
#endif
        Check(Operable(page, "btnStart") == shown && Operable(page, "btnPause") == shown,
              std::string("[6] [D16] off: START / PAUSE operable == ") + (shown ? "true (SOFT_SIMULTE build)" : "false (ship build)"));
        if (page) cJSON_Delete(page);
        ResetRec();
        a = Click("btnStart");
        const int qs = formevent::afterack::Pending();
        W906_FormEventRunAfterAck(a.sent);
        Ack b = Click("btnPause");
        const int qp = formevent::afterack::Pending();
        W906_FormEventRunAfterAck(b.sent);
        if (shown)
            Check(a.sent && b.sent && g_startCalls == 1 && g_pauseCalls == 1, "[6] [D16] off, SOFT_SIMULTE: both still work");
        else
            Check(!a.sent && Starts(a.err, "bad-payload") && !b.sent && Starts(b.err, "bad-payload") && qs == 0 && qp == 0 && g_startCalls == 0 && g_pauseCalls == 0,
                  "[6] [D16] off, ship: both refused (bad-payload, cannot be operated), nothing queued, nothing run");
        IniConfig.bD16_StepContactTest = true;
        page = Open();
        Check(Operable(page, "btnStart") && Operable(page, "btnPause"), "[6] [D16] on: both operable again");
        if (page) cJSON_Delete(page);
    }

    // ---------------------------------------------------------------- [7]
    std::printf("[7] running: runexc rows 18-19 (FileRW/_FormEvent.cpp tail) -- golden fContact is non-modal, main BtnStart stays enabled\n");
    {
        int rows = 0, match = 0;
        for (int i = 0; i < formevent::runexc::RowCount(); ++i) {
            const formevent::runexc::RowInfo* r = formevent::runexc::RowAt(i);
            if (!r || std::strcmp(r->form, "TfContact") != 0) continue;
            ++rows;
            const char* want = std::strcmp(r->control, "btnStart") == 0 ? "cContact.cpp:13965-13970 (V912 :14072-14077)"
                             : std::strcmp(r->control, "btnPause") == 0 ? "cContact.cpp:13972-13975 (V912 :14079-14082)" : nullptr;
            if (want && std::strcmp(r->event, "click") == 0 && std::strcmp(r->shownObj, "fContact") == 0 && r->systemStart && r->softStart &&
                r->golden && std::strstr(r->golden, want) && std::strstr(r->golden, "main.cpp:27312-27327 (V912 :28302-28317)") && std::strstr(r->golden, "Show() :27324 (V912 :28314)")) ++match;
        }
        Check(rows == 7 && match == 2,   // AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01): + CT-3d's 2 OTD rows (20-21, Steven Q65 = B; ctest B8_Ct3d_OtdDock)
              "[7] table: 7 TfContact rows (CT-3a's 3 + START / PAUSE + CT-3d's 2 OTD); START / PAUSE: click, fContact, SystemStart + SoftStart, golden cites (" +
              std::to_string(rows) + " rows, " + std::to_string(match) + " matched)");
        CosFunction.bEnableSoftWareControlButton = true;
        ResetRec();
        SystemStart = true;
        W906_FormFShowHook = nullptr;
        a = Click("btnStart");
        Check(!a.sent && Starts(a.err, "running: ") && Has(a.err, "fContact") && formevent::afterack::Pending() == 0,
              "[7] SystemStart, no page table -> running (the row's reason names fContact), nothing queued");
        W906_FormFShowHook = &FakePageTable;
        g_open = "fContact";
        StartAndRun(&a, &n, &first);
        Check(a.sent && n == 1 && g_startCalls == 1 && g_startFunc == "bEnableSoftWareControlButton",
              "[7] SystemStart + Contact open: START reaches TfMain::Start (golden: Start itself refuses at 906 main.cpp:5871 / V912 :6137 when SystemStart -- in StartFromWeb)");
        a = Click("btnPause");
        W906_FormEventRunAfterAck(a.sent);
        Check(a.sent && g_pauseCalls == 1, "[7] SystemStart + Contact open: PAUSE runs");
        SystemStart = false; SoftStart = true;
        StartAndRun(&a, &n, &first);
        Ack b = Click("btnPause");
        W906_FormEventRunAfterAck(b.sent);
        Check(a.sent && b.sent && g_startCalls == 2 && g_pauseCalls == 2, "[7] SoftStart: both allowed (golden TfMain::Start does not check SoftStart)");
        SystemStart = true;
        a = Click("cbContactMode");
        Check(!a.sent && Starts(a.err, "running: "), "[7] a CT-1 control (cbContactMode) is still refused while running");
        SystemStart = false; SoftStart = false;
        W906_FormFShowHook = nullptr; g_open = nullptr;
        CosFunction.bEnableSoftWareControlButton = false;
    }

    const std::string root = argc > 1 ? argv[1] : "";
    const std::string web = argc > 2 ? argv[2] : "";

    // ---------------------------------------------------------------- [8]
    std::printf("[8] why PAUSE also runs after the reply: a browser-waiting box is reachable from PauseFromWeb\n");
    {
        std::string ws, rsm;
        const bool ok = !root.empty() && ReadAll(root + "/WebStart.cpp", &ws) && ReadAll(root + "/RunStartMode.cpp", &rsm);
        Check(ok, "[8] read WebStart.cpp and RunStartMode.cpp");
        const std::vector<std::string> W = LiveLines(ws), M = LiveLines(rsm);
        const std::vector<std::string> pause = Body(W, "bool TfMainWeb::PauseFromWeb(AnsiString Func)");
        Check(!pause.empty() && LiveHas(pause, "SetRunStartMode(rsmAutoSiteMap);") && LiveHas(pause, "SoftStop = true;"),
              "[8] PauseFromWeb (golden :6325-6379) calls SetRunStartMode(rsmAutoSiteMap) ([I50] Pause trigger, golden :6369)");
        Check(LiveHas(M, "Mode=rsmInitial_ART;") && LiveHas(M, "ShowMyMessage(\"No Run ART Mode\", \"不能生產ART模式\");"),
              "[8] SetRunStartMode can turn it into rsmInitial_ART (SCK ART) and then ShowMyMessage (KYEC_LEE without [A10]) -- a box that waits "
              "for the browser, so PAUSE must not run while the handler holds FormLock");
    }

    // ---------------------------------------------------------------- [9]
    std::printf("[9] source ratchet (comments / '\\r' stripped, #if 0 skipped)\n");
    {
        std::string gen, cpp, fe, js, ks;
        const bool ok = !root.empty() && ReadAll(root + "/FileRW/DeviceForm_File.gen.inc", &gen) && ReadAll(root + "/FileRW/DeviceForm_File.cpp", &cpp) &&
                        ReadAll(root + "/FileRW/_FormEvent.cpp", &fe) && ReadAll(root + "/FileRW/_KitSuck.cpp", &ks) &&
                        !web.empty() && ReadAll(web + "/ht9045_contact_ev.js", &js);
        Check(ok, "[9] read the generated file, the entry cpp, _FormEvent.cpp, _KitSuck.cpp and the page js");
        const std::vector<std::string> G = LiveLines(gen), C = LiveLines(cpp), F = LiveLines(fe), J = LiveLines(js), K = LiveLines(ks);
        const std::string bsc = std::string("fMain->") + "BtnStartClick(";   // not spelled whole in a literal (CT-3b' rule)
        const std::vector<std::string> gs = Body(G, "static void DF_btnStartClick()"), gp = Body(G, "static void DF_btnPauseClick()");
        Check(LiveHas(gs, "W906_Contact_B8BtnStartClick();") && !LiveHas(gs, bsc) && !LiveHas(gs, "edTorue"),
              "[9] generated: DF_btnStartClick only queues (the three golden lines run after the reply)");
        Check(LiveHas(gp, "W906_Contact_B8BtnPauseClick();") && !LiveHas(gp, "BtnPauseClick(fMain)"), "[9] generated: DF_btnPauseClick only queues");
        Check(LiveHas(G, "{\"btnStart\", \"click\", \"cContact.cpp:14072 TfContact::btnStartClick\", &DF_Ev_btnStartClick}") &&
              LiveHas(G, "{\"btnPause\", \"click\", \"cContact.cpp:14079 TfContact::btnPauseClick\", &DF_Ev_btnPauseClick}"),
              "[9] generated: kDF_Events has the 2 CT-3b' rows");
        Check(LiveHas(C, "t[i].handler = Ct3aOwns(kDF_Events[i].control) ? &Ct3aRun : &EvB3Run;  if (Ct3bOwns(kDF_Events[i].control)) t[i].handler = &Ct3bRun;"),
              "[9] entry cpp: EvB3Table routes btnStart / btnPause to Ct3bRun (live, before the trailing //)");
        Check(LiveCount(C, "W906_RemoteRunStart(") == 1 && LiveHas(Body(C, "bool Ct3bStart(const char* func)"), "W906_RemoteRunStart(AnsiString(func));"),
              "[9] entry cpp: exactly one live START call site (Ct3bStart; ctest START_SitesCensus)");
        const std::vector<std::string> ra = Body(C, "void Ct3bRunStartAfterAck(const std::string&)");
        const int i0 = LiveAt(ra, "Ct3bBtnStartClick();"), i1 = LiveAt(ra, "fMain->edTorue0->Text=\"10\";"), i2 = LiveAt(ra, "fMain->edTorue1->Text=\"10\";");
        Check(i0 >= 0 && i1 > i0 && i2 > i1, "[9] entry cpp: the START item runs BtnStartClick, then edTorue0, then edTorue1 (golden :13967-13969 (V912 :14074-14076) order)");
        Check(LiveHas(Body(C, "void Ct3bRunPauseAfterAck(const std::string&)"), "fMain->BtnPauseClick(fMain);"),
              "[9] entry cpp: the PAUSE item is golden :13974 (V912 :14081) verbatim (the port's TfMain::BtnPauseClick)");
        const std::vector<std::string> bs = Body(C, "int Ct3bBtnStartClick()");
        Check(LiveHas(bs, "if(CosFunction.bEnableSoftWareControlButton)") && !LiveHas(bs, "CC_TERADYNE_US") && !LiveHas(bs, "iDefHonPrecLevel") &&   /* AI(W906-E030A) 20261003 [W906] (St01): no V912-only Teradyne-US level return (906 :6263-6266, Q-B) */
              LiveHas(bs, "Ct3bStart(\"bEnableSoftWareControlButton\")") && LiveHas(bs, "Ct3bStart(\"BtnStartClick_SOFT_SIMULTE\")") &&
              LiveHas(bs, "if(Ct3bCheckAutoOnlySetOneBin())") && LiveHas(bs, "if(FileRW_InArmSuckShtRow()==1)") &&
              LiveHas(bs, "if(IniConfig.bP28Auto1OnlyBin1==true && Ct3bCheckAuto1OnlyBin1())"),
              "[9] entry cpp: Ct3bBtnStartClick has golden 906 BtnStartClick's branches live (906 :6263-6320 / V912 :6531-6590), no Teradyne-US level return");
        Check(LiveHas(Body(C, "void W906_Contact_B8BtnStartClick()"), "formevent::afterack::Defer(&Ct3bRunStartAfterAck, \"\",") &&
              LiveHas(Body(C, "void W906_Contact_B8BtnPauseClick()"), "formevent::afterack::Defer(&Ct3bRunPauseAfterAck, \"\","),
              "[9] entry cpp: the two handler-side calls only Defer");
        Check(LiveHas(K, "return InArmSuck.iShtRow;"), "[9] _KitSuck.cpp: FileRW_InArmSuckShtRow (golden 906 main.cpp:6286 (V912 :6556))");
        Check(LiveHas(F, "{\"TfContact\", \"btnStart\",      \"click\", \"fContact\", true,        true,      kCtStartGolden},") &&
              LiveHas(F, "{\"TfContact\", \"btnPause\",      \"click\", \"fContact\", true,        true,      kCtPauseGolden},"),
              "[9] _FormEvent.cpp: runexc rows 18-19 are live code");
        Check(LiveHas(J, "['btnTStart', 'btnTStep', 'spbOneCycle', 'ledOneCycle', 'btnStart', 'btnPause'].forEach(") &&
              LiveHas(J, "a.afterAck.join(' | ')"),
              "[9] page js: ct3aLoad shows START / PAUSE, the ack's afterAck is shown (live, before the trailing //)");
    }

    // ---------------------------------------------------------------- guard
    {
        bool same = true;
        for (int i = 0; i < 4; ++i) {
            std::string now;
            const bool has = ReadAll(kGuard[i], &now);
            if (has != had[i] || now != before[i]) { same = false; std::printf("    changed: %s\n", kGuard[i]); }
        }
        std::map<std::string, std::string> treeAfter;
        ListTree("D:\\HT9045\\IniData", "", &treeAfter);
        if (treeAfter != treeBefore) { same = false; std::printf("    changed: D:\\HT9045\\IniData file list\n"); }
        Check(same, "[guard] real machine files untouched (Gerneral.ini, ContactInfo.ini, Contact.ini, config.ini, D:\\HT9045\\IniData list)");
    }

    W906_RemoteRun.Start = nullptr; W906_RemoteRun.Pause = nullptr;
    CloseGeneralIniFile();
    std::printf("\n%d / %d passed\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
