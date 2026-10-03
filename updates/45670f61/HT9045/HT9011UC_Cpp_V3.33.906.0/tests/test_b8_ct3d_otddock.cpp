// =============================================================================
//  test_b8_ct3d_otddock.cpp -- B8 CT-3d: Contact page OTD (One Touch Docking) dock cylinders + OTDTimer
//
//  //AI(W906-B8-CT3D) 20261001 [W906] (St01) new file. Dispatch D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3"
//    item 8 fourth slice; Jimmy RULINGS_20261001 #0 (translate per golden and wire it); Steven 20261001 09:4x (on-machine checks: EastSun).
//  golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp [AI(W906-E032) 20261003] (V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy in （）; AI(W906-E030-CITE) 20261003; gen.inc golden labels stay V912):
//    palOTD_4Click :15170-15193（V912 :15395-15418） / palOTD_6Click :15195-15218（V912 :15420-15443） -> Cylinder[C_DockYAxisOn / C_DockYAxisOff / C_DockXAxisOn / C_DockXAxisOff].On() / Off()
//      (no check at all; one static bDown each; the other panel goes back to bvRaised);
//    OTDTimerTimer :15220-15282（V912 :15445-15507） -> ledOTD + SW[SwUnDock] / SW[SwDockError]; skips on bBusy / InitialOK==false / the IO page open (:15226-15236（V912 :15451-15461）);
//    FormShow :1310-1311（V912 :1329-1330） palOTD_4 / palOTD_6 Visible=(USE_OTD==1), :1619（V912 :1646） OTDTimer->Enabled=(USE_OTD==1); FormClose :1817（V912 :1844） OTDTimer->Enabled=false.
//  Under test: the same FileRW/DeviceForm_File.cpp wb_serve compiles (tail: FileRW_Contact_Ct3dBoot / FileRW_Contact_OTDTimerTick / Ct3dRun;
//    EvB3Table; the beats at FormShowAndSnap / EvB3Run / Ct3aRun / Ct3bRun) + the generated FileRW/DeviceForm_File.gen.inc (DF_palOTD_4Click,
//    DF_palOTD_6Click, DF_OTDTimerTimer, kDF_Events rows 25-26), through the real WS entry W906_FormEvent (FileRW/_FormEvent.cpp: runexc rows
//    20-21 for the OTD panels, Steven Q65 = B) and the real golden FormShow / FormClose (editlist.get = PageJson, window close = FileRW_Contact_WindowEdge).
//    Recorders: the real Cylinder[] / SW[] objects with every IO base set to a sentinel (-1) -- On() / Off() set Status / OutValue, no IO call;
//    the On sensors are read through the same sentinel (TYPE_A -> false, TYPE_B -> true). No real IO, no motor.
//    [0] guard (ctest sandbox), boot, open with USE_OTD=1: events palOTD_4 / palOTD_6 (click, golden names), operable; OTDTimer on
//    [1] 240 KG twice: Y lock group, then the off group; bevel Tags (1 bvLowered / 2 bvRaised); ack.changed carries them
//    [2] 360 KG twice: Y + X lock group, then the off group
//    [3] golden's two static bDown: 240, 360, 240, 360 -> lock Y, lock both, off (240's bDown was still true), off
//    [4] OTDTimer table (golden :15240-15279（V912 :15465-15504）), the :15253（V912 :15478） DockYAxisOn oddity has no effect, InitialOK==false / IO page open skip, bBusy cleared
//    [5] the beat is wired: after a palOTD click, after editlist.get, after a CT-3a event; the console line
//    [6] visibility: USE_OTD 2 / 0 -> hidden, refused (bad-payload), cylinders untouched, OTDTimer off -> no beat
//    [7] running: Steven Q65 = B (1002 08:0x) -- runexc rows 20-21 for palOTD_4 / palOTD_6 (click, fContact, SystemStart + SoftStart, golden
//        cites), 7 TfContact rows; Contact not open -> refused (the row's reason), cylinders untouched; Contact open -> both panels run golden's
//        handlers during SystemStart and SoftStart (1001 / 1010 / off group / off group); an allowed runexc event (T.Start) still gives the
//        OTDTimer beat (golden's timer runs during the run).  //AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01): was "no runexc row, refused"
//    [8] window close = golden FormClose: OTDTimer off (:1817（V912 :1844）) -> no beat
//    [9] source ratchet (argv[1] = port tree, argv[2] = web\page, read-only; comments and '\r' stripped, #if 0 skipped)
//  NOT COVERED: real IO (the On() / Off() IO calls are the existing TMyCylinder code, not this change); the page's own JS (ctest B8_Ct3d_ContactPage);
//    the P-1 100 ms beat (not wired: wb_serve.cpp is not St01's file).
//  Writes: only the ctest sandbox; refuses outside ctest (w906_ctest_guard.h). Before / after compare: D:\HT9045\system\Gerneral.ini, ContactInfo.ini,
//    Contact.ini, SmartDiagnosticRecord.txt, D:\HT9045\config\config.ini and the D:\HT9045\IniData file list.
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
#include "mycylin.h"
#include "myswitch.h"
#include "atester_shims.h"      // fContact (TfContactShim): T.Start latch in [5] / [7]
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
extern void FileRW_Contact_OTDTimerTick();
extern const char* FileRW_Contact_WindowEdge(bool open);
extern void W906_EditPageWindowClosed(const char* goldenForm);
extern bool (*W906_FormFShowHook)(const char* goldenForm);   // csystem.h:421; W906_FormShowing(obj,false) = hook(obj)
extern AnsiString DataPath;
extern AnsiString DefaultPath;
extern AnsiString asGeneralPath;
extern int iMaxLevelItem;   // cSecurity.cpp:47 (wb_serve W906_SecurityBoot sets 180)
void OpenGeneralIniFile();    // common.h (golden common.cpp:1408): golden ReadFile needs it open; asGeneralPath = this test's sandbox Gerneral.ini
void CloseGeneralIniFile();

// Link-only (not under test), same as tests/test_b8_ct3b_contactstartpause.cpp.
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
const char* const kForm = "TfContact";
const char* g_open = nullptr;   // the fake page table: which golden form is open on an HMI
bool FakePageTable(const char* form) { return g_open && form && std::strcmp(form, g_open) == 0; }
bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }
bool Has(const std::string& s, const std::string& n) { return s.find(n) != std::string::npos; }

std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}
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
// a proxy's number field in the page snapshot (proxies.<id>.<key>); -999 = absent
int ProxyNum(const cJSON* page, const char* id, const char* key)
{
    const cJSON* px = page ? cJSON_GetObjectItemCaseSensitive(page, "proxies") : nullptr;
    const cJSON* p = px ? cJSON_GetObjectItemCaseSensitive(px, id) : nullptr;
    const cJSON* v = p ? cJSON_GetObjectItemCaseSensitive(p, key) : nullptr;
    return v && cJSON_IsNumber(v) ? v->valueint : -999;
}
struct Ack {
    bool sent = false;
    std::string err, raw;
};
// one WS form.event click, the shape D:\HT9045\web\page\ht9045_contact_ev.js sends for a TPanel / TButton
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
// ack.changed.<id>.tag; -999 = not in changed / no tag
int AckTag(const Ack& a, const char* id)
{
    cJSON* j = a.sent ? cJSON_Parse(a.raw.c_str()) : nullptr;
    const cJSON* ch = j ? cJSON_GetObjectItemCaseSensitive(j, "changed") : nullptr;
    const cJSON* c = ch ? cJSON_GetObjectItemCaseSensitive(ch, id) : nullptr;
    const cJSON* t = c ? cJSON_GetObjectItemCaseSensitive(c, "tag") : nullptr;
    const int v = t && cJSON_IsNumber(t) ? t->valueint : -999;
    if (j) cJSON_Delete(j);
    return v;
}

// ---- the dock cylinders and the two outputs as recorders (every IO base = -1: On() / Off() set Status, no IO call) ----
int kCyl[4];   // C_DockYAxisOn, C_DockYAxisOff, C_DockXAxisOn, C_DockXAxisOff (extern const int, filled in main)
void CylRecorders(bool enable)
{
    CosFunction.bCylinderOnOffTimeLog = false;   // TfSmartDiagnostic::GetCyliderOn/OffCount return at once (no SmartDiagnosticRecord.txt)
    for (int i = 0; i < 4; ++i) {
        TMyCylinder& c = Cylinder[kCyl[i]];
        c.Enable = enable;            // false: OnSensor() is always true (mycylin.cpp:194-197); true: read through the sentinel
        c.OutISABase = -1; c.OnSenISABase = -1; c.OffSenISABase = -1;
        c.OnSenEnable = enable; c.OffSenEnable = false;
        c.OnSenType = TYPE_A;         // sentinel read = 0: TYPE_A -> OnStatus false, TYPE_B -> true (mycylin.cpp:164-189)
    }
    SW[SwUnDock].Enable = false; SW[SwDockError].Enable = false;   // On() / Off() still set OutValue first (myswitch.cpp), then return
}
void Sense(int i, bool on) { Cylinder[kCyl[i]].OnSenType = on ? TYPE_B : TYPE_A; }
void SenseAll(bool yOn, bool yOff, bool xOn, bool xOff) { Sense(0, yOn); Sense(1, yOff); Sense(2, xOn); Sense(3, xOff); }
void SetStatus(bool yOn, bool yOff, bool xOn, bool xOff)
{
    Cylinder[kCyl[0]].Status = yOn; Cylinder[kCyl[1]].Status = yOff; Cylinder[kCyl[2]].Status = xOn; Cylinder[kCyl[3]].Status = xOff;
}
std::string StatusNow()
{
    std::string s;
    for (int i = 0; i < 4; ++i) s += Cylinder[kCyl[i]].Status ? '1' : '0';
    return s;   // YOn YOff XOn XOff
}
int Led() { return (int)EL<TControl>(kForm, "ledOTD")->Tag; }
int Bevel(const char* p) { return (int)EL<TPanel>(kForm, p)->Tag; }
std::string Outs() { return std::string(SW[SwUnDock].OutValue ? "1" : "0") + (SW[SwDockError].OutValue ? "1" : "0"); }   // UnDock, DockError
void SetOuts(bool u, bool e) { SW[SwUnDock].OutValue = u; SW[SwDockError].OutValue = e; }

// ---- source ratchet helpers (same as tests/test_b8_ct3b_contactstartpause.cpp) ----
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
    std::printf("test_b8_ct3d_otddock -- B8 CT-3d golden TfContact::palOTD_4Click / palOTD_6Click / OTDTimerTimer (906 cContact.cpp:15170 (V912 :15395) / :15195 (V912 :15420) / :15220 (V912 :15445))\n");
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("B8_Ct3d_OtdDock", rt))
            return 2;
    }
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\system\\ContactInfo.ini", "D:\\HT9045\\system\\Contact.ini",
                                  "D:\\HT9045\\system\\SmartDiagnosticRecord.txt", "D:\\HT9045\\config\\config.ini"};
    const int kNG = (int)(sizeof(kGuard) / sizeof(kGuard[0]));
    std::string before[5];
    bool had[5];
    for (int i = 0; i < kNG; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    std::map<std::string, std::string> treeBefore;
    ListTree("D:\\HT9045\\IniData", "", &treeBefore);

    kCyl[0] = C_DockYAxisOn; kCyl[1] = C_DockYAxisOff; kCyl[2] = C_DockXAxisOn; kCyl[3] = C_DockXAxisOff;
    const bool init0 = InitialOK;
    const int otd0 = USE_OTD;
    Ack a;

    // ---------------------------------------------------------------- [0]
    std::printf("[0] boot + first open with USE_OTD=1 (golden FormShow :1310-1311 (V912 :1329-1330) / :1619 (V912 :1646))\n");
    SystemStart = false; SoftStart = false;
    CosFunction.bContactHeightSaveToContactIni = false;   // golden ReadFile would read D:\HT9045\system\Contact.ini (not sandboxed)
    CosFunction.bUseDynamicKitDiameter = false;           // golden TfContact ctor would read D:\HT9045\system\ContactInfo.ini
    CUSTOMER_CODE = 0;
    IniConfig.bD16_StepContactTest = true;                // T.Start visible ([5] / [7] use it)
    Tri_Temp_Machine = 0;
    iMaxLevelItem = 180;
    for (int i = 0; i < 256; ++i) LevelSet.AccessLevel[i] = 0;
    AccessLevel = 4;
    InitialOK = false;
    USE_OTD = 1;
    CylRecorders(false);
    SetStatus(false, false, false, false);
    SetOuts(false, false);
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0, "[0] sim motors attached (" + std::to_string(made) + " axes)");
    OpenGeneralIniFile();
    FileRW_Contact_Boot();
    W906_FormFShowHook = nullptr;
    Check(filerw::ELFind(kForm, "palOTD_4") && filerw::ELFind(kForm, "palOTD_6") && filerw::ELFind(kForm, "ledOTD") && filerw::ELFind(kForm, "OTDTimer"),
          "[0] boot: palOTD_4 / palOTD_6 / ledOTD / OTDTimer proxies exist (FileRW_Contact_Ct3dBoot)");
    {
        cJSON* page = Open();
        Check(page != nullptr, "[0] editlist.get DeviceForm_File (golden FormShow) = 200");
        const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
        const cJSON* p4 = ev ? cJSON_GetObjectItemCaseSensitive(ev, "palOTD_4") : nullptr;
        const cJSON* p6 = ev ? cJSON_GetObjectItemCaseSensitive(ev, "palOTD_6") : nullptr;
        Check(p4 && Str(p4, "event") == "click" && Str(p4, "golden") == "cContact.cpp:15395 TfContact::palOTD_4Click" &&
              p6 && Str(p6, "event") == "click" && Str(p6, "golden") == "cContact.cpp:15420 TfContact::palOTD_6Click",
              "[0] events: palOTD_4 -> golden palOTD_4Click (:15170, V912 :15395), palOTD_6 -> golden palOTD_6Click (:15195, V912 :15420)");
        Check(Operable(page, "palOTD_4") && Operable(page, "palOTD_6"), "[0] USE_OTD==1: both panels visible and operable (golden FormShow :1310-1311 (V912 :1329-1330))");
        Check(EL<TControl>(kForm, "OTDTimer")->Enabled, "[0] USE_OTD==1: OTDTimer enabled (golden FormShow :1619 (V912 :1646))");
        if (page) cJSON_Delete(page);
        Check(g_lockDepth == 0, "[0] FormLock balanced");
    }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] OTD Under 240KG (golden palOTD_4Click :15170-15193 (V912 :15395-15418)), cylinders as recorders, InitialOK false (no timer beat)\n");
    {
        SetStatus(false, true, false, true);              // the Off group (YOff / XOff on)
        a = Click("palOTD_4");
        Check(a.sent && AckGolden(a) == "cContact.cpp:15395 TfContact::palOTD_4Click", "[1] form.event palOTD_4 click -> ok, golden palOTD_4Click " + a.err);
        Check(StatusNow() == "1001", "[1] first click: YOn On, YOff Off, XOn Off, XOff On (golden :15179-15182 (V912 :15404-15407)) -- Status " + StatusNow());
        Check(Bevel("palOTD_4") == 1 && Bevel("palOTD_6") == 2, "[1] palOTD_4 bvLowered (Tag 1, :15178 (V912 :15403)), palOTD_6 bvRaised (Tag 2, :15173 (V912 :15398))");
        Check(AckTag(a, "palOTD_4") == 1 && AckTag(a, "palOTD_6") == 2, "[1] ack.changed carries both bevel Tags (the page draws inset / outset)");
        Check(Led() == 0 && Outs() == "00", "[1] InitialOK==false: the OTDTimer beat does nothing (golden :15226 (V912 :15451)) -- led " + std::to_string(Led()) + " outs " + Outs());
        a = Click("palOTD_4");
        Check(a.sent && StatusNow() == "0101", "[1] second click: YOn Off, YOff On, XOn Off, XOff On (golden :15188-15191 (V912 :15413-15416)) -- Status " + StatusNow());
        Check(Bevel("palOTD_4") == 2 && AckTag(a, "palOTD_4") == 2, "[1] palOTD_4 back to bvRaised (Tag 2, :15187 (V912 :15412)), in ack.changed");
        Check(g_lockDepth == 0, "[1] FormLock balanced");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] OTD Over 360KG (golden palOTD_6Click :15195-15218 (V912 :15420-15443))\n");
    {
        a = Click("palOTD_6");
        Check(a.sent && AckGolden(a) == "cContact.cpp:15420 TfContact::palOTD_6Click", "[2] form.event palOTD_6 click -> ok, golden palOTD_6Click " + a.err);
        Check(StatusNow() == "1010", "[2] first click: YOn On, YOff Off, XOn On, XOff Off (golden :15204-15207 (V912 :15429-15432)) -- Status " + StatusNow());
        Check(Bevel("palOTD_6") == 1 && Bevel("palOTD_4") == 2, "[2] palOTD_6 bvLowered (:15203, V912 :15428), palOTD_4 bvRaised (:15198, V912 :15423)");
        a = Click("palOTD_6");
        Check(a.sent && StatusNow() == "0101" && Bevel("palOTD_6") == 2, "[2] second click: the off group (golden :15212-15216 (V912 :15437-15441)) -- Status " + StatusNow());
    }

    // ---------------------------------------------------------------- [3]
    std::printf("[3] golden's two static bDown (each handler has its own; the other panel only goes back to bvRaised)\n");
    {
        a = Click("palOTD_4");
        const std::string s1 = StatusNow();
        Ack b = Click("palOTD_6");
        const std::string s2 = StatusNow();
        const int b4 = Bevel("palOTD_4");
        Ack c = Click("palOTD_4");
        const std::string s3 = StatusNow();
        Ack d = Click("palOTD_6");
        const std::string s4 = StatusNow();
        Check(a.sent && b.sent && c.sent && d.sent && s1 == "1001" && s2 == "1010" && b4 == 2 && s3 == "0101" && s4 == "0101",
              "[3] 240 -> lock Y (1001), 360 -> lock both (1010; 240 drawn raised), 240 again -> off group (its bDown was still true), "
              "360 again -> off group: " + s1 + " " + s2 + " " + s3 + " " + s4);
        Check(Bevel("palOTD_4") == 2 && Bevel("palOTD_6") == 2, "[3] both drawn raised at the end");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] golden OTDTimerTimer :15220-15282 (V912 :15445-15507) (InitialOK true; sensors read through the sentinel)\n");
    {
        InitialOK = true;
        CylRecorders(true);
        struct Row { const char* what; bool st[4]; bool sen[4]; int led; const char* outs; };
        const Row rows[] = {
            {"YOn sensor on (:15240 (V912 :15465), not inserted)",              {1, 0, 0, 1}, {1, 0, 0, 0}, 2, "01"},
            {"XOn sensor on (:15240, V912 :15465)",                            {1, 0, 1, 0}, {0, 0, 1, 0}, 2, "01"},
            {"240 KG locked: YOn St, XOn not (:15246-15251, V912 :15471-15476)",      {1, 0, 0, 1}, {0, 0, 0, 0}, 1, "00"},
            {"360 KG locked: YOn St, XOn St (:15252-15257, V912 :15477-15482)",       {1, 0, 1, 0}, {0, 0, 0, 0}, 1, "00"},
            {"all open: YOff / XOff St + sensors (:15265-15270, V912 :15490-15495)",  {0, 1, 0, 1}, {0, 1, 0, 1}, 0, "10"},
            {"YOff St, XOff St, XOff sensor off -> else (:15271, V912 :15496)", {0, 1, 0, 1}, {0, 1, 0, 0}, 2, "01"},
            {"nothing on -> else (:15271-15276, V912 :15496-15501)",                  {0, 0, 0, 0}, {0, 0, 0, 0}, 2, "01"},
        };
        for (const Row& r : rows) {
            SetStatus(r.st[0], r.st[1], r.st[2], r.st[3]);
            SenseAll(r.sen[0], r.sen[1], r.sen[2], r.sen[3]);
            EL<TControl>(kForm, "ledOTD")->Tag = 9;
            SetOuts(!SW[SwUnDock].OutValue, !SW[SwDockError].OutValue);   // the beat must write both
            FileRW_Contact_OTDTimerTick();
            Check(Led() == r.led && Outs() == r.outs,
                  std::string("[4] ") + r.what + ": ledOTD " + std::to_string(Led()) + " (want " + std::to_string(r.led) + "), SwUnDock/SwDockError " + Outs() +
                  " (want " + r.outs + ")");
        }
        // the :15253（V912 :15478） oddity (second OnSensor is DockYAxisOn): :15240（V912 :15465） already took any XOn sensor-on case, so the result equals "as if X"
        SetStatus(true, false, true, false);
        SenseAll(false, false, true, false);
        FileRW_Contact_OTDTimerTick();
        Check(Led() == 2 && Outs() == "01", "[4] :15253 (V912 :15478) oddity kept: 360 KG with the XOn sensor on is still red / error (the :15240 (V912 :15465) branch catches it first)");
        // the dead :15258-15260（V912 :15483-15485） branch (YOff sensor ==false and ==true): YOn St + XOn not + YOff sensor on -> the :15246（V912 :15471） branch (lime) as before
        SetStatus(true, false, false, true);
        SenseAll(false, true, false, false);
        FileRW_Contact_OTDTimerTick();
        Check(Led() == 1 && Outs() == "00", "[4] :15258-15260 (V912 :15483-15485) never decides: the same Status is taken by :15246 (V912 :15471) (lime)");
        // InitialOK==false: nothing (golden :15226（V912 :15451）)
        SetStatus(false, false, false, false);
        SenseAll(false, false, false, false);
        EL<TControl>(kForm, "ledOTD")->Tag = 7; SetOuts(true, false);
        InitialOK = false;
        FileRW_Contact_OTDTimerTick();
        Check(Led() == 7 && Outs() == "10", "[4] InitialOK==false: no change (golden :15226-15229 (V912 :15451-15454))");
        InitialOK = true;
        // the IO page open: nothing, bBusy cleared (golden :15232-15236（V912 :15457-15461）)
        W906_FormFShowHook = &FakePageTable;
        g_open = "fiosetview";
        FileRW_Contact_OTDTimerTick();
        Check(Led() == 7 && Outs() == "10", "[4] IO page open (W906_FormShowing(\"fiosetview\")): no change (golden :15232-15236 (V912 :15457-15461))");
        g_open = nullptr;
        W906_FormFShowHook = nullptr;
        FileRW_Contact_OTDTimerTick();
        Check(Led() == 2 && Outs() == "01", "[4] IO page closed again: the next beat runs (bBusy was cleared, golden :15234 (V912 :15459))");
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] the beat is wired (golden runs it every 100 ms while Contact is open): after a click, after editlist.get, after other events\n");
    {
        // click 240 with readable sensors (all off) -> lock Y group -> the beat that follows the click makes it lime
        SetStatus(false, true, false, true);
        SenseAll(false, false, false, false);
        EL<TControl>(kForm, "ledOTD")->Tag = 9; SetOuts(true, true);
        a = Click("palOTD_4");
        Check(a.sent && StatusNow() == "1001" && Led() == 1 && Outs() == "00",
              "[5] palOTD_4 click, then the OTDTimer beat (Ct3dRun): lime, both outputs off -- Status " + StatusNow() + " led " + std::to_string(Led()) + " outs " + Outs());
        Check(AckTag(a, "ledOTD") == 1, "[5] ack.changed carries ledOTD tag 1 (the page lights it green)");
        // editlist.get (golden FormShow): the beat after it
        SenseAll(true, false, false, false);
        cJSON* page = Open();
        Check(page && ProxyNum(page, "ledOTD", "tag") == 2 && Outs() == "01", "[5] editlist.get: the beat after golden FormShow -> proxies.ledOTD.tag 2 (red), DockError on");
        if (page) cJSON_Delete(page);
        // a CT-3a event (T.Start, Ct3aRun) also gives the beat
        SenseAll(false, false, false, false);
        SetStatus(false, true, false, true);
        Cylinder[kCyl[1]].OnSenType = TYPE_B; Cylinder[kCyl[3]].OnSenType = TYPE_B;   // YOff / XOff sensors on: all open
        a = Click("btnTStart");
        Check(a.sent && Led() == 0 && Outs() == "10", "[5] after a T.Start click (Ct3aRun): the beat ran -> all open = led off, SwUnDock on");
        fContact->bSetupStart = false;
        // a CT-1 event (EvB3Run) also gives the beat: the first operable CT-1 radio group / check box, sent with its current value
        //   (VCL: value unchanged -> no OnClick, so no handler side effect; EvB3Run still gives the beat)
        SenseAll(false, false, false, false);
        SetStatus(false, false, false, false);
        {
            cJSON* pg = Open();
            std::string v, used;
            for (const char* n : {"rgKitDiameter", "rgOutKitDiameter", "rgDieForceKitDiameter", "chkUseAddWeight", "cbEnableUK"}) {
                if (!Operable(pg, n)) continue;
                used = n;
                if (TRadioGroup* rg = dynamic_cast<TRadioGroup*>(filerw::ELFind(kForm, n)))
                    v = "{\"form\":\"TfContact\",\"control\":\"" + used + "\",\"event\":\"click\",\"itemIndex\":" + std::to_string(rg->ItemIndex) + "}";
                else if (TCheckBox* cb = dynamic_cast<TCheckBox*>(filerw::ELFind(kForm, n)))
                    v = std::string("{\"form\":\"TfContact\",\"control\":\"") + used + "\",\"event\":\"click\",\"checked\":" + (cb->Checked ? "true" : "false") + "}";
                break;
            }
            if (pg) cJSON_Delete(pg);
            // the editlist.get above gave its own beat (red); reset the recorders so only the event's beat can light it
            EL<TControl>(kForm, "ledOTD")->Tag = 9; SetOuts(true, true);
            std::string raw, err;
            const bool ok = !v.empty() && W906_FormEvent(kTag, v, &raw, &err);
            Check(ok && Led() == 2 && Outs() == "01", "[5] after a CT-1 event (" + (used.empty() ? std::string("none operable") : used) +
                  ", EvB3Run): the beat ran -> red, DockError on " + err);
        }
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] visibility: palOTD_4 / palOTD_6 only with USE_OTD==1 (golden FormShow :1310-1311 (V912 :1329-1330), :1619 (V912 :1646))\n");
    {
        for (int otd : {2, 0}) {
            USE_OTD = otd;
            cJSON* page = Open();
            Check(page && !Operable(page, "palOTD_4") && !Operable(page, "palOTD_6") && !EL<TControl>(kForm, "OTDTimer")->Enabled,
                  "[6] USE_OTD=" + std::to_string(otd) + ": both panels hidden (not operable), OTDTimer off");
            if (page) cJSON_Delete(page);
            SetStatus(false, true, false, true);
            const std::string s0 = StatusNow();
            a = Click("palOTD_4");
            Ack b = Click("palOTD_6");
            Check(!a.sent && Starts(a.err, "bad-payload") && !b.sent && Starts(b.err, "bad-payload") && StatusNow() == s0,
                  "[6] USE_OTD=" + std::to_string(otd) + ": both clicks refused (bad-payload: golden cannot click a hidden panel), cylinders untouched");
            EL<TControl>(kForm, "ledOTD")->Tag = 7; SetOuts(true, true);
            FileRW_Contact_OTDTimerTick();
            Check(Led() == 7 && Outs() == "11", "[6] USE_OTD=" + std::to_string(otd) + ": OTDTimer disabled -> the beat does nothing");
        }
        USE_OTD = 1;
        Check(OpenOk() && EL<TControl>(kForm, "OTDTimer")->Enabled, "[6] USE_OTD back to 1: OTDTimer on again");
    }

    // ---------------------------------------------------------------- [7]
    std::printf("[7] running: Steven Q65 = B (1002 08:0x) -- runexc rows 20-21 let the OTD panels work during a run, as golden\n");
    {
        int rows = 0, otdRows = 0, otdMatch = 0;
        for (int i = 0; i < formevent::runexc::RowCount(); ++i) {
            const formevent::runexc::RowInfo* r = formevent::runexc::RowAt(i);
            if (!r || std::strcmp(r->form, "TfContact") != 0) continue;
            ++rows;
            if (!std::strstr(r->control, "palOTD")) continue;
            ++otdRows;
            const bool p4 = std::strcmp(r->control, "palOTD_4") == 0, p6 = std::strcmp(r->control, "palOTD_6") == 0;
            if ((p4 || p6) && std::strcmp(r->event, "click") == 0 && std::strcmp(r->shownObj, "fContact") == 0 && r->systemStart && r->softStart &&
                r->golden && std::strstr(r->golden, "cContact.cpp:15170-15218 (V912 :15395-15443)") &&
                std::strstr(r->golden, p4 ? "palOTD_4Click :15170-15193 (V912 :15395-15418)" : "palOTD_6Click :15195-15218 (V912 :15420-15443)") &&
                std::strstr(r->golden, "main.cpp:27312-27327 (V912 :28302-28317)") && std::strstr(r->golden, "Steven Q65 = B (1002 08:0x)")) ++otdMatch;
        }
        Check(rows == 7 && otdRows == 2 && otdMatch == 2,
              "[7] runexc: 7 TfContact rows (CT-3a 3 + CT-3b' 2 + CT-3d 2); palOTD_4 / palOTD_6: click, fContact, SystemStart + SoftStart, golden cites, "
              "Steven Q65 = B (" + std::to_string(rows) + " / " + std::to_string(otdRows) + " / " + std::to_string(otdMatch) + ")");
        // Contact not open: the row is there, the page table is not -> running with the row's reason; nothing moves
        SystemStart = true; SoftStart = false;
        SetStatus(false, true, false, true);
        W906_FormFShowHook = nullptr;
        a = Click("palOTD_4");
        Check(!a.sent && Starts(a.err, "running: ") && Has(a.err, "fContact") && StatusNow() == "0101",
              "[7] SystemStart, no page table (Contact not open) -> running (the row's reason names fContact), cylinders untouched -- " + a.err);
        W906_FormFShowHook = &FakePageTable;
        g_open = "fOffSet";
        Ack b = Click("palOTD_6");
        Check(!b.sent && Starts(b.err, "running: ") && Has(b.err, "fContact") && StatusNow() == "0101",
              "[7] SystemStart, the page table says only fOffSet is open -> running, cylinders untouched");
        g_open = "fContact";
        // [5] left palOTD_4's static bDown true: one stopped click puts it back (golden: the off group), so each state starts with both false
        SystemStart = false; SoftStart = false;
        a = Click("palOTD_4");
        Check(a.sent && StatusNow() == "0101", "[7] stopped: palOTD_4 once more -> the off group (its bDown from [5] was true) -- Status " + StatusNow());
        for (int st = 0; st < 2; ++st) {
            const std::string sn = st == 0 ? "SystemStart" : "SoftStart";
            SystemStart = st == 0; SoftStart = st == 1;
            SetStatus(false, true, false, true);
            a = Click("palOTD_4");
            const std::string s1 = StatusNow();
            b = Click("palOTD_6");
            const std::string s2 = StatusNow();
            Ack c = Click("palOTD_4");
            const std::string s3 = StatusNow();
            Ack d = Click("palOTD_6");
            const std::string s4 = StatusNow();
            Check(a.sent && b.sent && c.sent && d.sent && AckGolden(a) == "cContact.cpp:15395 TfContact::palOTD_4Click" &&
                  AckGolden(b) == "cContact.cpp:15420 TfContact::palOTD_6Click" && s1 == "1001" && s2 == "1010" && s3 == "0101" && s4 == "0101",
                  "[7] " + sn + " + Contact open: allowed (runexc rows 20-21), golden handlers ran: 240 -> 1001, 360 -> 1010, 240 -> off, 360 -> off: " +
                  s1 + " " + s2 + " " + s3 + " " + s4 + " " + a.err + b.err + c.err + d.err);
            Check(Bevel("palOTD_4") == 2 && Bevel("palOTD_6") == 2 && g_lockDepth == 0, "[7] " + sn + ": both drawn raised at the end, FormLock balanced");
        }
        // golden's OTDTimer keeps running during the run: an allowed runexc event (T.Start) still gives the beat
        SystemStart = true; SoftStart = false;
        SenseAll(false, false, false, false);
        SetStatus(true, false, false, true);
        EL<TControl>(kForm, "ledOTD")->Tag = 9; SetOuts(true, true);
        a = Click("btnTStart");
        Check(a.sent && Led() == 1 && Outs() == "00", "[7] SystemStart + Contact open: T.Start allowed (runexc row 15) and the OTDTimer beat follows (lime) " + a.err);
        fContact->bSetupStart = false;
        SystemStart = false; SoftStart = false;
        g_open = nullptr; W906_FormFShowHook = nullptr;
    }

    // ---------------------------------------------------------------- [8]
    std::printf("[8] window close = golden FormClose: OTDTimer->Enabled=false (:1817, V912 :1844) -> no beat\n");
    {
        const char* w = FileRW_Contact_WindowEdge(false);
        Check(w && Has(w, "FormClose") && !EL<TControl>(kForm, "OTDTimer")->Enabled, std::string("[8] FormClose ran, OTDTimer off -- ") + (w ? w : "(null)"));
        SystemStart = false;
        EL<TControl>(kForm, "ledOTD")->Tag = 7; SetOuts(true, true);
        FileRW_Contact_OTDTimerTick();
        Check(Led() == 7 && Outs() == "11", "[8] after the close the beat does nothing (golden's timer is stopped)");
        W906_EditPageWindowClosed("fContact");
        Check(OpenOk() && EL<TControl>(kForm, "OTDTimer")->Enabled, "[8] reopen (golden FormShow :1619 (V912 :1646)): OTDTimer on again");
    }

    const std::string root = argc > 1 ? argv[1] : "";
    const std::string web = argc > 2 ? argv[2] : "";

    // ---------------------------------------------------------------- [9]
    std::printf("[9] source ratchet (comments / '\\r' stripped, #if 0 skipped)\n");
    {
        std::string gen, cpp, fe, js;
        const bool ok = !root.empty() && ReadAll(root + "/FileRW/DeviceForm_File.gen.inc", &gen) && ReadAll(root + "/FileRW/DeviceForm_File.cpp", &cpp) &&
                        ReadAll(root + "/FileRW/_FormEvent.cpp", &fe) && !web.empty() && ReadAll(web + "/ht9045_contact_ev.js", &js);
        Check(ok, "[9] read the generated file, the entry cpp, _FormEvent.cpp and the page js");
        const std::vector<std::string> G = LiveLines(gen), C = LiveLines(cpp), F = LiveLines(fe), J = LiveLines(js);
        const std::vector<std::string> g4 = Body(G, "static void DF_palOTD_4Click()"), g6 = Body(G, "static void DF_palOTD_6Click()"),
                                       gt = Body(G, "static void DF_OTDTimerTimer()");
        Check(LiveCount(g4, "Cylinder[C_Dock") == 8 && LiveHas(g4, "Cylinder[C_DockYAxisOn].On();") && LiveHas(g4, "Cylinder[C_DockXAxisOff].On();") &&
              LiveHas(g4, "static bool bDown=false;") && LiveHas(g4, "\"palOTD_4\")->Tag=DF_bvLowered;") && !LiveHas(g4, "BevelOuter"),
              "[9] generated: DF_palOTD_4Click has the 8 golden cylinder commands live, its own static bDown, the bevel on the Tag");
        Check(LiveCount(g6, "Cylinder[C_Dock") == 8 && LiveHas(g6, "Cylinder[C_DockXAxisOn].On();") && LiveHas(g6, "\"palOTD_6\")->Tag=DF_bvLowered;") &&
              !LiveHas(g6, "BevelOuter"), "[9] generated: DF_palOTD_6Click likewise");
        Check(LiveHas(gt, "if(bBusy || InitialOK==false)") && LiveHas(gt, "if(W906_FormShowing(\"fiosetview\", fiosetview->fShow))") &&
              LiveHas(gt, "Cylinder[C_DockXAxisOn].Status==true && Cylinder[C_DockYAxisOn].OnSensor()==false)") &&
              LiveHas(gt, "SW[SwUnDock].OnOff(bUnDock);") && LiveHas(gt, "SW[SwDockError].OnOff(bError);") && LiveCount(gt, "\"ledOTD\")->Tag=") == 6 &&
              !LiveHas(gt, "TrueColor") && !LiveHas(gt, "->Value="),
              "[9] generated: DF_OTDTimerTimer live with golden's guards, the :15253 (V912 :15478) oddity kept, both outputs, 6 LED writes on the Tag");
        Check(LiveHas(G, "{\"palOTD_4\", \"click\", \"cContact.cpp:15395 TfContact::palOTD_4Click\", &DF_Ev_palOTD_4Click}") &&
              LiveHas(G, "{\"palOTD_6\", \"click\", \"cContact.cpp:15420 TfContact::palOTD_6Click\", &DF_Ev_palOTD_6Click}"),
              "[9] generated: kDF_Events has the 2 CT-3d rows");
        Check(LiveHas(C, "if (Ct3bOwns(kDF_Events[i].control)) t[i].handler = &Ct3bRun;  if (Ct3dOwns(kDF_Events[i].control)) t[i].handler = &Ct3dRun;"),
              "[9] entry cpp: EvB3Table routes palOTD_4 / palOTD_6 to Ct3dRun (live, before the trailing //)");
        Check(LiveHas(C, "DF_FormShow();  FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();") &&
              LiveHas(C, "EvB3Body(*e, sender);  FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();") &&
              LiveCount(C, "FileRW_Contact_TimerEPTick();  FileRW_Contact_OTDTimerTick();") == 5,
              "[9] entry cpp: the OTDTimer beat next to the TimerEP beat at all 5 places (FormShowAndSnap, EvB3Run, Ct3aRun, Ct3bRun, Ct3dRun)");
        const std::vector<std::string> tk = Body(C, "void FileRW_Contact_OTDTimerTick()");
        Check(LiveHas(tk, "if (!EL<TControl>(kForm, \"OTDTimer\")->Enabled) return;") && LiveHas(tk, "DF_OTDTimerTimer();"),
              "[9] entry cpp: FileRW_Contact_OTDTimerTick runs golden OTDTimerTimer only while OTDTimer is enabled");
        Check(LiveCount(C, "W906_RemoteRunStart(") == 1, "[9] entry cpp: still exactly one live START call site (no START path added; START_SitesCensus unchanged)");
        Check(LiveHas(F, "{\"TfContact\", \"palOTD_4\",      \"click\", \"fContact\", true,        true,      kCtOtd4Golden},") &&
              LiveHas(F, "{\"TfContact\", \"palOTD_6\",      \"click\", \"fContact\", true,        true,      kCtOtd6Golden},") &&
              LiveCount(F, "\"palOTD_") == 2,
              "[9] _FormEvent.cpp: runexc rows 20-21 (palOTD_4 / palOTD_6, both states) are live code, no other live palOTD row (Steven Q65 = B)");
        Check(LiveHas(J, "function ct3dLook(id, el, tag)") && LiveHas(J, "function ct3dLoad()") &&
              LiveHas(J, "el.classList.toggle('on', !!p.tag); });  ct3dLoad(); }") && LiveHas(J, "try { hook(); reopenPorted(); ct3aLoad(); refreshKnown(); }") &&
              LiveHas(J, "if (v.tag !== undefined) ct3dLook(id, el, v.tag);") && LiveHas(J, "ev.stopPropagation();"),
              "[9] page js: ct3dLook / ct3dLoad live (ct3dLoad at the end of ct3aLoad = after every load), on every ack.changed tag; the lamp stops its click");
    }

    // ---------------------------------------------------------------- guard
    {
        bool same = true;
        for (int i = 0; i < kNG; ++i) {
            std::string now;
            const bool has = ReadAll(kGuard[i], &now);
            if (has != had[i] || now != before[i]) { same = false; std::printf("    changed: %s\n", kGuard[i]); }
        }
        std::map<std::string, std::string> treeAfter;
        ListTree("D:\\HT9045\\IniData", "", &treeAfter);
        if (treeAfter != treeBefore) { same = false; std::printf("    changed: D:\\HT9045\\IniData file list\n"); }
        Check(same, "[guard] real machine files untouched (Gerneral.ini, ContactInfo.ini, Contact.ini, SmartDiagnosticRecord.txt, config.ini, D:\\HT9045\\IniData list)");
    }

    InitialOK = init0;
    USE_OTD = otd0;
    CloseGeneralIniFile();
    std::printf("\n%d / %d passed\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
