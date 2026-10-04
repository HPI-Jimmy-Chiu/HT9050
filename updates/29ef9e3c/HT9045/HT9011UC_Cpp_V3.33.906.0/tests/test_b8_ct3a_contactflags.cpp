// =============================================================================
//  test_b8_ct3a_contactflags.cpp -- B8 CT-3a: Contact page mode switch + T.Start / T.Step / One Cycle (flags only)
//
//  //AI(W906-B8-CT3A) 20261001 [W906] (St01) new file. Dispatch D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3"
//    item 8 first slice; Jimmy RULINGS_20261001 #0 (translate per golden and wire it); Steven 20261001 09:4x (on-machine checks: EastSun).
//  golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp [AI(W906-E032) 20261003] (cp950; V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy in （）, AI(W906-E030-CITE) 20261003; the generated gen.inc golden labels stay V912):
//    rbModeNormalClick :15330-15339（V912 :15555-15564） (11 mode radios, V912 cContact.dfm:18125-18318; 906 :18125-18303 has 10, no rbVisualDetectionTest -- Q-C) -> ASE Kaohsiung + Auto Height => fAllMotorHome=false;
//      SetContactMode :15341-15434（V912 :15566-15696） (iContactMode; KYEC_CHEN + A16 ChangeContactMode :15436-15454（V912 :15698-15716）)
//    btnTStartClick :2251-2254（V912 :2280-2283） / btnTStepClick :2256-2259（V912 :2285-2288） -> the latches ckernel.cpp:96-131 / :54-94 (same in both) wait for
//    spbOneCycleClick :16861-16865（V912 :17152-17156） -> OneCycleProcess :11741-11746（V912 :11770-11775） + ledOneCycle
//    FormShow :1162-1164（V912 :1178-1180） / :1171（V912 :1187） / :1218（V912 :1235） / :1601-1606（V912 :1628-1633） / :1634-1640（V912 :1661-1667）, FormClose :1870-1872（V912 :1897-1899）
//  Under test: the same FileRW/DeviceForm_File.cpp wb_serve compiles (tail: FileRW_Contact_RbClickTrue / RbReimpose / Ct3aRun) + the
//    generated FileRW/DeviceForm_File.gen.inc (DF_rbModeNormalClick, DF_btnTStartClick, DF_btnTStepClick, DF_OneCycleProcess,
//    DF_spbOneCycleClick, kDF_Events rows 9-22, the FormShow / FormClose replacements), through the real WS entry W906_FormEvent
//    (FileRW/_FormEvent.cpp, incl. the runexc table) and RunPageEvent (FileRW/_EditPage.cpp); the real golden FormShow / FormClose run
//    (editlist.get = PageJson, window close = FileRW_Contact_WindowEdge). The flow side is the LIVE ckernel.cpp WaitManualStartKey /
//    WaitManualStepKey (god-stack by RESCAN): a latch is proven by the flow taking it.
//    [0] guard (ctest sandbox), boot, first open: events lists the 14 CT-3a rows (click, golden name)
//    [1] the flag object: btnTStart / btnTStep set fContact->bSetupStart / bSetupStep (TfContactShim, atester_shims.h:251), the form
//        object fContactForm (forms/fContact.cpp:90) is not touched, and WaitManualStartKey / WaitManualStepKey take the latch
//    [2] mode switch: each of the 11 radios -> golden iContactMode; exactly one radio checked (VCL TurnSiblingsOff); ack.changed carries
//        the radio that went off; clicking the checked radio runs nothing (VCL); the page's own "checked" value is not trusted;
//        K Temp -> chk_K_Temperature checked; Step Contact Test -> chkDailyCorrelation enabled; KYEC_CHEN + A16 -> ChangeContactMode
//    [3] ASE Kaohsiung: Auto Height -> fAllMotorHome=false; other radios / other customers leave it
//    [4] One Cycle: below CONTACT_TEST forced off; at CONTACT_TEST / STEP_CONTACT_TEST it toggles; LED = proxy Tag
//    [5] gates: T.Start / T.Step / One Cycle / Step Contact Test radio follow [D16] (FormShow :1600-1605（V912 :1627-1632）); SOFT_SIMULTE always shows the
//        three buttons (:1634-1640（V912 :1661-1667）); hidden -> click refused (bad-payload), latch untouched
//    [6] page open / save re-read / close: the post-save re-read keeps mode, latches, One Cycle (golden save :14154-14155（V912 :14262-14263） does not touch
//        them) and re-imposes the radio a stale page save applied; window close = golden FormClose -> Normal; a fresh open = golden
//        FormShow -> Normal + latches cleared even when FormClose did not run
//    [7] running: the runexc table has exactly the 3 CT-3a TfContact rows (rows 18-21: the CT-3b' / CT-3d tests); SystemStart / SoftStart + page table says fContact open -> they run;
//        no page table -> running; mode radios stay refused while running (stricter than golden, see FileRW/_FormEvent.cpp tail)
//    [8] source ratchet (argv[1] = port tree, argv[2] = web\page, read-only; comments and '\r' stripped, #if 0 skipped)
//  //AI(W906-E030A) 20261003 [W906] (St01): todo E-030 part A (Jimmy RULINGS_20261002 #20 / #23 item 6: main items whose content differs
//    from 906 go back to 906; 906 = D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\cContact.cpp, V912 lines in brackets):
//    [3b] ASE Chungli (CC_ASE_CL): the Contact height edits are never greyed (V912's Chrischen 20260316 lines -- DoIniDataToForm :1021-1028,
//         FormShow :1306-1307 / :1478-1485, SetContactMode :15577-15633 x4 -- are not in 906)
//    [3c] FormShow AMD conditions are 906's CUSTOMER_CODE==CC_AMD_M (906 :1418 / :1587), not V912's IniConfig.bAMDFunction (:1437 / :1614)
//    [8] + ratchets for both (generated DF_SetContactMode / DF_FormShow / DF_DoIniDataToForm)
//    NOT changed (ST01-E 20261003 HOLD, a question for Jimmy): the V912-only A02 `return;` after Close() in spbSaveClick (V912 :14191)
//  NOT COVERED: the golden contact state machines that read bContinueContact (gated in the port); a real START (CT-3b');
//    the page's own JS (ack applying, ct3aLoad) beyond the source ratchet.
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
#include "MachineType.h"
#include "mysensor.h"
#include "atester_shims.h"      // fContact (TfContactShim) -- the object the flow reads
#include "forms/fContact.h"     // fContactForm (the port's TfContact form object) -- must stay untouched
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
extern const char* FileRW_Contact_WindowEdge(bool open);
extern void W906_EditPageWindowClosed(const char* goldenForm);
extern bool (*W906_FormFShowHook)(const char* goldenForm);   // csystem.h:421; W906_FormShowing(obj,false) = hook(obj)
extern AnsiString DataPath;
extern AnsiString DefaultPath;
extern AnsiString asGeneralPath;
extern int iMaxLevelItem;   // cSecurity.cpp:47 (wb_serve W906_SecurityBoot sets 180)
void OpenGeneralIniFile();    // common.h (body common.cpp; golden common.cpp:1408): golden CheckAndReadIniDataGeneral needs it open (golden ReadFile :391); asGeneralPath = this test's own sandbox Gerneral.ini
void CloseGeneralIniFile();
bool WaitManualStartKey();   // ckernel.h:96 (body ckernel.cpp:301)
bool WaitManualStepKey();    // ckernel.h:97 (body ckernel.cpp:226)

// Link-only (not under test):
//  * FileRW_IniConfig_ChangeCBListProperty: same as tests/test_b8_ctl2_timerep.cpp (else FileRW/_fallback.cpp collides with _EditList.cpp).
//  * W906_Main_sbContactClickOpen (FileRW/MainClick.cpp, wb_serve only): called after golden FormShow (R84); nothing to log here.
//  * JsonBridge FindBridge / RunEvent / FormLock (wb_serve only): no A-shape page -> always the C route; the lock is a counter.
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
// golden V912 cContact.dfm:18125-18318 (rgHandlerMode; 906 :18125-18303 without rbVisualDetectionTest, Q-C) and the iContactMode each one gives (SetContactMode 906 :15341-15403 (V912 :15566-15665, + the V912-only VISUAL_DETECTION_TEST arm :15661-15665); cContact.cpp 906 :73-85 (V912 :74-86))
const char* const kRb[11] = {"rbModeNormal", "rbAutoHeight", "rbManualHeight", "rbContactTest", "rbAutoContactTest", "rbStepContactTest",
                             "rbDeviceMapping", "rbLoadCellAutoHigh", "rbKTempIndexMove", "rbDeviceLoopTest", "rbVisualDetectionTest"};
const int kMode[11] = {0, 1, 2, 3, 4, 5, 9, 8, 11, 10, 12};
const char* g_open = nullptr;   // the fake page table: which golden form is open on an HMI
bool FakePageTable(const char* form) { return g_open && form && std::strcmp(form, g_open) == 0; }
bool Starts(const std::string& s, const std::string& p) { return s.compare(0, p.size(), p) == 0; }
bool Has(const std::string& s, const std::string& n) { return s.find(n) != std::string::npos; }

std::string Str(const cJSON* o, const char* key)
{
    const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, key) : nullptr;
    return v && cJSON_IsString(v) ? v->valuestring : std::string();
}

// editlist.get (PageJson runs the real golden FormShow = FormShowAndSnap); returns the parsed response
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
// one WS form.event click, the shape D:\HT9045\web\page\ht9045_contact_ev.js sends (a TRadioButton label reads as {"itemIndex":0})
Ack Click(const std::string& control, const std::string& extra = "")
{
    Ack a;
    const std::string v = "{\"form\":\"TfContact\",\"control\":\"" + control + "\",\"event\":\"click\"" + extra + "}";
    a.sent = W906_FormEvent(kTag, v, &a.raw, &a.err);
    return a;
}
Ack ClickRb(int k, const std::string& extra = ",\"itemIndex\":0") { return Click(kRb[k], extra); }
// ack.changed.<id>.checked: 1 / 0, -1 = not in changed
int ChangedChecked(const Ack& a, const char* id)
{
    cJSON* j = a.sent ? cJSON_Parse(a.raw.c_str()) : nullptr;
    const cJSON* ch = j ? cJSON_GetObjectItemCaseSensitive(j, "changed") : nullptr;
    const cJSON* o = ch ? cJSON_GetObjectItemCaseSensitive(ch, id) : nullptr;
    const cJSON* c = o ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
    const int r = c ? (cJSON_IsTrue(c) ? 1 : 0) : -1;
    if (j) cJSON_Delete(j);
    return r;
}
std::string AckGolden(const Ack& a)
{
    cJSON* j = a.sent ? cJSON_Parse(a.raw.c_str()) : nullptr;
    const std::string g = Str(j, "golden");
    if (j) cJSON_Delete(j);
    return g;
}
// which radios are checked: "" = none, else the names joined by ','
std::string CheckedRadios()
{
    std::string s;
    for (const char* n : kRb)
        if (EL<TRadioButton>(kForm, n)->Checked) s += (s.empty() ? "" : ",") + std::string(n);
    return s;
}
int Led() { return EL<TControl>(kForm, "ledOneCycle")->Tag; }

void Quiet()   // the manual-key inputs the flow also reads: off, so only the latch can answer (same as tests/test_w7_l2_ckernel.cpp)
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
    std::string cur;
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
    std::printf("test_b8_ct3a_contactflags -- B8 CT-3a golden TfContact rbModeNormalClick / btnTStartClick / btnTStepClick / spbOneCycleClick "
                "(906 cContact.cpp:15330 (V912 :15555) / :2251 (V912 :2280) / :2256 (V912 :2285) / :16861 (V912 :17152))\n");
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("B8_Ct3a_ContactFlags", rt))
            return 2;
    }
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\system\\ContactInfo.ini", "D:\\HT9045\\system\\Contact.ini",
                                  "D:\\HT9045\\config\\config.ini"};
    std::string before[4];
    bool had[4];
    for (int i = 0; i < 4; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    std::map<std::string, std::string> treeBefore;
    ListTree("D:\\HT9045\\IniData", "", &treeBefore);

    // ---------------------------------------------------------------- [0]
    std::printf("[0] boot + first open (golden FormShow)\n");
    SystemStart = false; SoftStart = false;
    CosFunction.bContactHeightSaveToContactIni = false;   // golden ReadFile would read D:\HT9045\system\Contact.ini (not sandboxed)
    CosFunction.bUseDynamicKitDiameter = false;           // golden TfContact ctor would read D:\HT9045\system\ContactInfo.ini
    CUSTOMER_CODE = 0;
    IniConfig.bD16_StepContactTest = true;
    Tri_Temp_Machine = 0;
    iMaxLevelItem = 180;                                  // wb_serve W906_SecurityBoot (cSecurity.cpp:47); else Insufficient(n) is always false
    for (int i = 0; i < 256; ++i) LevelSet.AccessLevel[i] = 0;
    AccessLevel = 4;                                      // FormShow :1206-1223（V912 :1222-1240; V912 :1234 rbVisualDetectionTest is not in 906）: [18] / [38] gbContactForce / [93] palHeightCalibration
    Quiet();
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0 && MOT[MTrayX].Motor && MOT[MTestZ1].Motor, "[0] sim motors attached (" + std::to_string(made) + " axes)");
    OpenGeneralIniFile();
    FileRW_Contact_Boot();
    W906_FormFShowHook = nullptr;
    {
        cJSON* page = Open();
        Check(page != nullptr, "[0] editlist.get DeviceForm_File (golden FormShow) = 200");
        const cJSON* ev = page ? cJSON_GetObjectItemCaseSensitive(page, "events") : nullptr;
        int rows = 0;
        for (const char* n : kRb) {
            const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, n) : nullptr;
            if (e && Str(e, "event") == "click" && Str(e, "golden") == "cContact.cpp:15555 TfContact::rbModeNormalClick") ++rows;
            else std::printf("    missing / wrong row: %s\n", n);
        }
        const char* const kBtn[3][2] = {{"btnTStart", "cContact.cpp:2280 TfContact::btnTStartClick"},
                                        {"btnTStep", "cContact.cpp:2285 TfContact::btnTStepClick"},
                                        {"spbOneCycle", "cContact.cpp:17152 TfContact::spbOneCycleClick"}};
        for (const auto& b : kBtn) {
            const cJSON* e = ev ? cJSON_GetObjectItemCaseSensitive(ev, b[0]) : nullptr;
            if (e && Str(e, "event") == "click" && Str(e, "golden") == b[1]) ++rows;
            else std::printf("    missing / wrong row: %s\n", b[0]);
        }
        Check(rows == 14, "[0] events: 11 mode radios -> rbModeNormalClick, btnTStart / btnTStep / spbOneCycle -> their golden handlers (14 rows)");
        Check(Operable(page, "btnTStart") && Operable(page, "btnTStep") && Operable(page, "spbOneCycle") && Operable(page, "rbAutoHeight"),
              "[0] [D16] on, machine empty: T.Start / T.Step / One Cycle / mode radios operable");
        if (page) cJSON_Delete(page);
        Check(iContactMode == 0 && CheckedRadios() == "rbModeNormal" && !fContact->bSetupStart && !fContact->bSetupStep && Led() == 0,
              "[0] after golden FormShow: iContactMode=CONTACT_NORMAL, only rbModeNormal checked, latches clear, One Cycle LED off");
        Check(g_lockDepth == 0, "[0] FormLock balanced");
    }

    // ---------------------------------------------------------------- [1]
    std::printf("[1] the flag object: fContact (TfContactShim) -- what ckernel.cpp WaitManualStartKey / WaitManualStepKey read\n");
    {
        fContact->bSetupStart = false; fContact->bSetupStep = false;
        fContactForm->bSetupStart = false; fContactForm->bSetupStep = false;
        Check(!WaitManualStartKey() && !WaitManualStepKey(), "[1] precondition: no latch, no key -> the flow keeps waiting (both false)");
        Ack a = Click("btnTStart");
        Check(a.sent && AckGolden(a) == "cContact.cpp:2280 TfContact::btnTStartClick", "[1] form.event btnTStart click -> ok, golden btnTStartClick " + a.err);
        Check(fContact->bSetupStart && !fContact->bSetupStep, "[1] fContact->bSetupStart=true (golden :2253 (V912 :2282)), bSetupStep untouched");
        Check(!fContactForm->bSetupStart && !fContactForm->bSetupStep,
              "[1] the port's form object fContactForm (forms/fContact.cpp:90) is NOT written -- nobody reads it");
        Check(WaitManualStartKey() && !fContact->bSetupStart, "[1] WaitManualStartKey() takes it: true, latch cleared (golden ckernel.cpp:116-128)");
        Check(!WaitManualStartKey(), "[1] ... and only once (next poll false)");
        a = Click("btnTStep");
        Check(a.sent && AckGolden(a) == "cContact.cpp:2285 TfContact::btnTStepClick" && fContact->bSetupStep && !fContact->bSetupStart,
              "[1] form.event btnTStep click -> fContact->bSetupStep=true (golden :2258 (V912 :2287))");
        Check(!fContactForm->bSetupStep, "[1] fContactForm->bSetupStep untouched");
        Check(WaitManualStepKey() && !fContact->bSetupStep, "[1] WaitManualStepKey() takes it: true, latch cleared (golden ckernel.cpp:74-91)");
        Check(iContactMode == 0 && CheckedRadios() == "rbModeNormal", "[1] the latches change no mode");
    }

    // ---------------------------------------------------------------- [2]
    std::printf("[2] mode switch: rbModeNormalClick -> SetContactMode, VCL TRadioButton.SetChecked(True)\n");
    IniConfig.bD15_AutoContactTest = true; CosFunction.bDeviceMapTest = true; IniConfig.bD67LoadCellMeasure = true;
    IniConfig.bAMDFunction = true; Tri_Temp_Machine = 1;
    CUSTOMER_CODE = CC_AMD_M;   //AI(W906-E035) 20261003: the 11th radio (rbVisualDetectionTest) now follows CUSTOMER_CODE==CC_AMD_M (code 982), not the flag;
                                //   the KYEC block below and [3c] put CUSTOMER_CODE back to 0
    {
        cJSON* page = Open();   // save re-read rule: same window-open -> FormShow recomputes visibility, keeps the mode
        int op = 0;
        for (const char* n : kRb) if (Operable(page, n)) ++op;
        Check(op == 11, "[2] with D15 / D16 / bDeviceMapTest / D67 / AMD / Tri-Temp on: all 11 radios operable (" + std::to_string(op) + ")");
        if (page) cJSON_Delete(page);
    }
    {
        int ok = 0, prev = 0;
        const int order[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0};
        for (int i = 0; i < 11; ++i) {
            const int k = order[i];
            Ack a = ClickRb(k);
            const bool good = a.sent && iContactMode == kMode[k] && CheckedRadios() == kRb[k] && ChangedChecked(a, kRb[k]) == 1 &&
                              ChangedChecked(a, kRb[prev]) == 0;
            if (good) ++ok;
            else std::printf("    %s: sent=%d err=%s iContactMode=%d checked=%s changed(self)=%d changed(prev %s)=%d\n", kRb[k], a.sent, a.err.c_str(),
                             iContactMode, CheckedRadios().c_str(), ChangedChecked(a, kRb[k]), kRb[prev], ChangedChecked(a, kRb[prev]));
            if (k == 8) Check(EL<TCheckBox>(kForm, "chk_K_Temperature")->Checked, "[2] K Temp Index Move -> chk_K_Temperature checked (golden :15402 (V912 :15659))");
            if (k == 5) Check(EL<TCheckBox>(kForm, "chkDailyCorrelation")->Enabled, "[2] Step Contact Test -> chkDailyCorrelation enabled (golden :15425-15428 (V912 :15687-15690))");
            if (k == 6) Check(!EL<TCheckBox>(kForm, "chkDailyCorrelation")->Enabled, "[2] other mode -> chkDailyCorrelation disabled (golden :15429-15433 (V912 :15691-15695))");
            prev = k;
        }
        Check(ok == 11, "[2] each radio -> golden iContactMode (1 2 3 4 5 9 8 11 10 12 0), only it checked, ack.changed: it on, the previous one off");
    }
    {
        Ack a = ClickRb(3);
        Check(a.sent && iContactMode == 3, "[2] rbContactTest -> CONTACT_TEST");
        iContactMode = 77;   // sentinel: a handler run would overwrite it
        a = ClickRb(3);
        Check(a.sent && iContactMode == 77 && CheckedRadios() == "rbContactTest" && ChangedChecked(a, "rbContactTest") == -1,
              "[2] click the checked radio -> VCL does not Click: no handler (iContactMode sentinel kept), nothing changed");
        a = ClickRb(3, ",\"checked\":false");
        Check(a.sent && iContactMode == 77 && CheckedRadios() == "rbContactTest",
              "[2] page sends checked:false for the checked radio -> a click is SetChecked(True): still checked, no handler");
        a = ClickRb(2, ",\"checked\":true");
        Check(a.sent && iContactMode == 2 && CheckedRadios() == "rbManualHeight",
              "[2] page sends checked:true for another radio (step 5 already set it) -> still a change: handler runs, siblings off");
        CUSTOMER_CODE = CC_KYEC_CHEN; IniConfig.bA16ContactTestDropContact = true;
        a = ClickRb(3);
        Check(a.sent && EL<TComboBox>(kForm, "cbContactMode")->ItemIndex == DropContact && EL<TEdit>(kForm, "edDropOffset1")->Enabled,
              "[2] KYEC_CHEN + A16, Contact Test -> ChangeContactMode(true): cbContactMode=DropContact, Drop Offset enabled (golden :15372-15376 (V912 :15621-15625) / :15440-15444 (V912 :15702-15706))");
        a = ClickRb(0);
        Check(a.sent && EL<TComboBox>(kForm, "cbContactMode")->ItemIndex == DirectContactMode && !EL<TEdit>(kForm, "edDropOffset1")->Enabled,
              "[2] KYEC_CHEN + A16, Normal -> ChangeContactMode(false): DirectContactMode, Drop Offset disabled (golden :15346-15350 (V912 :15571-15575) / :15448-15452 (V912 :15710-15714))");
        CUSTOMER_CODE = 0; IniConfig.bA16ContactTestDropContact = false;
    }
    Tri_Temp_Machine = 0; IniConfig.bAMDFunction = false;   // back to the defaults ([1] / [7] call the live IsSafeLockCheck, which reads Tri_Temp_Machine)

    // ---------------------------------------------------------------- [3]
    std::printf("[3] ASE Kaohsiung: Auto Height forces a full home (golden :15334-15337 (V912 :15559-15562))\n");
    {
        CUSTOMER_CODE = CC_ASE_KaohSiung;
        fAllMotorHome = true;
        Ack a = ClickRb(1);
        Check(a.sent && iContactMode == 1 && !fAllMotorHome, "[3] ASE Kaohsiung + rbAutoHeight -> fAllMotorHome=false, iContactMode=CONTACT_AUTO_GET_HEIGHT");
        fAllMotorHome = true;
        a = ClickRb(2);
        Check(a.sent && iContactMode == 2 && fAllMotorHome, "[3] ASE Kaohsiung + rbManualHeight -> fAllMotorHome untouched");
        fAllMotorHome = true;
        a = ClickRb(1);   // Auto Height again from Manual
        CUSTOMER_CODE = 0;
        fAllMotorHome = true;
        a = ClickRb(0);
        a = ClickRb(1);
        Check(a.sent && iContactMode == 1 && fAllMotorHome, "[3] other customer + rbAutoHeight -> fAllMotorHome untouched");
        a = ClickRb(0);
    }

    // ---------------------------------------------------------------- [3b]
    //AI(W906-E030A) 20261003 [W906] (St01): E-030 -- golden 906 never greys edContactHeight1 / edContactHeight2 for ASE Chungli; the V912-only
    //   Chrischen 20260316 lines (V912 cContact.cpp DoIniDataToForm :1021-1028, FormShow :1306-1307 / :1478-1485, SetContactMode :15577-15584 /
    //   :15595-15602 / :15608-15615 / :15626-15633) are replaced by ';' in tools/editlist/DeviceForm_File.py (tail, E-030 (1)).
    std::printf("[3b] ASE Chungli (CC_ASE_CL) + bContactShowOffset: the Contact height edits stay editable (906; V912's greying is gone)\n");
    {
        const bool off0 = CosFunction.bContactShowOffset;
        CUSTOMER_CODE = CC_ASE_CL; CosFunction.bContactShowOffset = true;
        Check(OpenOk() && EL<TEdit>(kForm, "edContactHeight1")->Enabled && EL<TEdit>(kForm, "edContactHeight2")->Enabled,
              "[3b] page open (golden FormShow + DoIniDataToForm): edContactHeight1 / edContactHeight2 enabled (906 :1224-1225, :1450-1453; "
              "V912 :1306-1307 / :1478-1485 / :1021-1028 would grey them)");
        const int order[4] = {1, 2, 3, 0};   // Auto Height, Manual Height, Contact Test, Normal: the four V912 SetContactMode blocks
        int ok = 0;
        for (int k : order) {
            EL<TEdit>(kForm, "edContactHeight1")->Enabled = true; EL<TEdit>(kForm, "edContactHeight2")->Enabled = true;
            Ack a = ClickRb(k);
            const bool good = a.sent && iContactMode == kMode[k] && EL<TEdit>(kForm, "edContactHeight1")->Enabled && EL<TEdit>(kForm, "edContactHeight2")->Enabled;
            if (good) ++ok;
            else std::printf("    %s: sent=%d err=%s iContactMode=%d edContactHeight1 %d edContactHeight2 %d\n", kRb[k], a.sent, a.err.c_str(), iContactMode,
                             (int)EL<TEdit>(kForm, "edContactHeight1")->Enabled, (int)EL<TEdit>(kForm, "edContactHeight2")->Enabled);
        }
        Check(ok == 4, "[3b] Auto Height / Manual Height / Contact Test / Normal -> golden 906 SetContactMode (:15341-15435) leaves both height edits enabled");
        CUSTOMER_CODE = 0; CosFunction.bContactShowOffset = off0;
        Check(OpenOk() && iContactMode == 0, "[3b] back to the defaults (reopen = Normal)");
    }

    // ---------------------------------------------------------------- [3c]
    //AI(W906-E030A) 20261003 [W906] (St01): E-030 -- FormShow's AMD conditions are golden 906's CUSTOMER_CODE==CC_AMD_M (906 cContact.cpp:1418 /
    //   :1587 / :1709); V912 :1437 / :1614 / :1736 test IniConfig.bAMDFunction (Ifor 20260716 AMD group flag). rbVisualDetectionTest's
    //   Visible=(IniConfig.bAMDFunction) (V912 :1234) stays (Q-C).
    //AI(W906-E035) 20261003 [W906] (St01 ST01-E2): todo E-035 -- that V912 :1234 condition was never true here (nothing in the port writes
    //   IniConfig.bAMDFunction), so the V912-only Visual Detection Test mode never showed. 906 0618 has no such control; V912 sets the flag
    //   only in FUNC_CC_AMD_SG (CosFunction.cpp:2991, code 982); 906's 982 is CC_AMD_M (MachineType.h:354) -> the generated line now tests
    //   CUSTOMER_CODE==CC_AMD_M (tools/editlist/DeviceForm_File.py AI(W906-E035); Q-C keep + Q81 keep-912 default). The two checks below
    //   go red on the V912 condition (982 with the flag off would hide it; another customer with the flag on would show it).
    std::printf("[3c] FormShow AMD conditions: 906 CUSTOMER_CODE==CC_AMD_M, not V912 IniConfig.bAMDFunction\n");
    {
        const int bar0 = BAR_CODE_INSTALL;
        const bool amd0 = IniConfig.bAMDFunction;
        BAR_CODE_INSTALL = ebctUseCCDMode;   // the not-AMD branch then shows both boxes (906 :1424 / :1593)
        CUSTOMER_CODE = CC_AMD_M; IniConfig.bAMDFunction = false;
        Check(OpenOk() && !EL<TCheckBox>(kForm, "cb2DMatrix")->Visible && !EL<TCheckBox>(kForm, "cbSFCAutoTune")->Visible,
              "[3c] CC_AMD_M, IniConfig.bAMDFunction off: cb2DMatrix / cbSFCAutoTune hidden (906 FormShow :1418-1421 / :1587-1590)");
        Check(EL<TRadioButton>(kForm, "rbVisualDetectionTest")->Visible,
              "[3c] E-035: CC_AMD_M (982), IniConfig.bAMDFunction off: the Visual Detection Test mode is shown (V912 :1234 tests the flag, which the port never sets)");
        CUSTOMER_CODE = 0; IniConfig.bAMDFunction = true;
        Check(OpenOk() && EL<TCheckBox>(kForm, "cb2DMatrix")->Visible && EL<TCheckBox>(kForm, "cbSFCAutoTune")->Visible,
              "[3c] another customer, IniConfig.bAMDFunction on, CCD barcode: both shown (906 :1424 / :1593; V912 :1437 / :1614 would hide them)");
        Check(!EL<TRadioButton>(kForm, "rbVisualDetectionTest")->Visible,
              "[3c] E-035: another customer, IniConfig.bAMDFunction on: the Visual Detection Test mode is hidden (the condition is CUSTOMER_CODE==CC_AMD_M, not the flag)");
        BAR_CODE_INSTALL = bar0; IniConfig.bAMDFunction = amd0; CUSTOMER_CODE = 0;
        Check(OpenOk(), "[3c] back to the defaults");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] One Cycle: OneCycleProcess (golden :11741-11746 (V912 :11770-11775)) + ledOneCycle\n");
    {
        Ack a = Click("spbOneCycle");
        Check(a.sent && Led() == 0 && AckGolden(a) == "cContact.cpp:17152 TfContact::spbOneCycleClick",
              "[4] Normal (below CONTACT_TEST): toggled then forced off -> LED off");
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 0, "[4] Normal again: still off");
        a = ClickRb(3);
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 1, "[4] CONTACT_TEST: on (LED = proxy Tag 1)");
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 0, "[4] CONTACT_TEST: click again -> off");
        a = ClickRb(5);
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 1, "[4] STEP_CONTACT_TEST: on");
        a = ClickRb(2);
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 0, "[4] MANUAL_GET_HEIGHT (2 < 3): the toggle is forced off -- golden OneCycleProcess");
        a = ClickRb(0);
    }

    // ---------------------------------------------------------------- [5]
    std::printf("[5] visibility gates: [D16] Step Contact Test (FormShow :1600-1605 (V912 :1627-1632)); SOFT_SIMULTE shows the three buttons (:1634-1640, V912 :1661-1667)\n");
    {
        IniConfig.bD16_StepContactTest = false;
        cJSON* page = Open();
#ifdef SOFT_SIMULTE
        const bool btnShown = true;
#else
        const bool btnShown = false;
#endif
        Check(Operable(page, "btnTStart") == btnShown && Operable(page, "btnTStep") == btnShown && Operable(page, "spbOneCycle") == btnShown,
              std::string("[5] [D16] off: T.Start / T.Step / One Cycle operable == ") + (btnShown ? "true (SOFT_SIMULTE build)" : "false (ship build)"));
        Check(!Operable(page, "rbStepContactTest"), "[5] [D16] off: the Step Contact Test radio is hidden in both builds (:1600 (V912 :1627) is not in the SOFT_SIMULTE block)");
        if (page) cJSON_Delete(page);
        fContact->bSetupStart = false;
        Ack a = Click("btnTStart");
        if (btnShown)
            Check(a.sent && fContact->bSetupStart, "[5] [D16] off, SOFT_SIMULTE: T.Start still works");
        else
            Check(!a.sent && Starts(a.err, "bad-payload") && !fContact->bSetupStart, "[5] [D16] off, ship: T.Start refused (bad-payload, cannot be operated), latch untouched");
        fContact->bSetupStart = false;
        IniConfig.bD16_StepContactTest = true;
        page = Open();
        Check(Operable(page, "btnTStart") && Operable(page, "btnTStep") && Operable(page, "spbOneCycle") && Operable(page, "rbStepContactTest"),
              "[5] [D16] on: all four operable");
        if (page) cJSON_Delete(page);
        // level: golden FormShow :1221-1223（V912 :1238-1240） gbContactForce->Enabled = Insufficient(38) (T.Start / T.Step / One Cycle live in it),
        //   palHeightCalibration->Enabled = Insufficient(93) (the mode radios: rgHandlerMode > panHeightMode > palHeightCalibration)
        LevelSet.AccessLevel[38] = AccessLevel + 1;
        page = Open();
        Check(!Operable(page, "btnTStart") && !Operable(page, "btnTStep") && !Operable(page, "spbOneCycle") && Operable(page, "rbAutoHeight"),
              "[5] level below [38]: gbContactForce disabled -> T.Start / T.Step / One Cycle not operable; mode radios still are");
        if (page) cJSON_Delete(page);
        a = Click("btnTStep");
        Check(!a.sent && Starts(a.err, "bad-payload") && !fContact->bSetupStep, "[5] level below [38]: T.Step refused (bad-payload), latch untouched");
        LevelSet.AccessLevel[38] = 0; LevelSet.AccessLevel[93] = AccessLevel + 1;
        page = Open();
        Check(Operable(page, "btnTStart") && !Operable(page, "rbAutoHeight") && !Operable(page, "rbModeNormal"),
              "[5] level below [93]: palHeightCalibration disabled -> mode radios not operable; T.Start is");
        if (page) cJSON_Delete(page);
        const int m0 = iContactMode;
        a = ClickRb(1);
        Check(!a.sent && Starts(a.err, "bad-payload") && iContactMode == m0, "[5] level below [93]: a mode radio refused, mode unchanged");
        LevelSet.AccessLevel[93] = 0;
        Check(OpenOk(), "[5] levels back");
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] open / post-save re-read / close\n");
    {
        Ack a = ClickRb(5);
        a = Click("spbOneCycle");
        fContact->bSetupStart = true;   // a pending T.Start
        Check(iContactMode == 5 && Led() == 1, "[6] set up: Step Contact Test, One Cycle on, T.Start pending");
        Check(OpenOk() && iContactMode == 5 && CheckedRadios() == "rbStepContactTest" && fContact->bSetupStart && Led() == 1,
              "[6] post-save re-read (same window-open): mode, radio, pending T.Start and One Cycle kept (golden save :14154-14155 (V912 :14262-14263) does not touch them)");
        EL<TRadioButton>(kForm, "rbAutoHeight")->Checked = true;   // what a stale page save would apply (ELApplyProxies)
        Check(OpenOk() && iContactMode == 5 && CheckedRadios() == "rbStepContactTest",
              "[6] a radio value applied by a page save is undone at the re-read (radios only change by a click / golden code)");
        SystemStart = true;
        const char* w = FileRW_Contact_WindowEdge(false);
        SystemStart = false;
        Check(w && Has(w, "FormClose") && iContactMode == 0 && CheckedRadios() == "rbModeNormal",
              std::string("[6] window close = golden FormClose: iContactMode=CONTACT_NORMAL, rbModeNormal checked (:1870-1872, V912 :1897-1899) -- ") + (w ? w : "(null)"));
        W906_EditPageWindowClosed("fContact");
        fContact->bSetupStart = true; fContact->bSetupStep = true;   // latches left over from before the close
        a = ClickRb(3);
        Check(!a.sent && Has(a.err, "reload page"), "[6] after the close the page must be re-opened before events (" + a.err + ")");
        Check(OpenOk() && iContactMode == 0 && CheckedRadios() == "rbModeNormal" && !fContact->bSetupStart && !fContact->bSetupStep && Led() == 0,
              "[6] fresh open = golden FormShow: Normal, latches cleared (:1162-1164 (V912 :1178-1180), :1171 (V912 :1187)), One Cycle off (:1218 (V912 :1235), :1606 (V912 :1633))");
        a = ClickRb(3);
        Check(a.sent && iContactMode == 3, "[6] Contact Test selected again");
        W906_EditPageWindowClosed("fContact");   // the window went away without the close edge (e.g. a lost browser): no FormClose ran
        Check(iContactMode == 3, "[6] precondition: FormClose did not run, the mode is still CONTACT_TEST");
        Check(OpenOk() && iContactMode == 0 && CheckedRadios() == "rbModeNormal",
              "[6] opening the Contact page puts the mode back to Normal, as golden (FormShow :1162 (V912 :1178) rbModeNormal->Checked=true -> OnClick + :1163 (V912 :1179) SetContactMode)");
    }

    // ---------------------------------------------------------------- [7]
    std::printf("[7] running: the runexc table (FileRW/_FormEvent.cpp tail) -- golden lets T.Start / T.Step / One Cycle work during a run\n");
    {
        int rows = 0, match = 0;
        for (int i = 0; i < formevent::runexc::RowCount(); ++i) {
            const formevent::runexc::RowInfo* r = formevent::runexc::RowAt(i);
            if (!r || std::strcmp(r->form, "TfContact") != 0) continue;  if (std::strcmp(r->control, "btnStart") == 0 || std::strcmp(r->control, "btnPause") == 0) continue;  if (std::strncmp(r->control, "palOTD_", 7) == 0) continue;   // AI(W906-B8-CT3B) 20261001 [W906] (St01): rows 18-19 are CT-3b' (START / PAUSE), checked by ctest B8_Ct3b_ContactStartPause; same line  // AI(W906-B8-CT3D-Q65B) 20261002 [W906] (St01): rows 20-21 are CT-3d (OTD, Steven Q65 = B), checked by ctest B8_Ct3d_OtdDock; same line, before the trailing //
            ++rows;
            const char* want = std::strcmp(r->control, "btnTStart") == 0 ? "cContact.cpp:2251-2254 (V912 :2280-2283)"
                             : std::strcmp(r->control, "btnTStep") == 0 ? "cContact.cpp:2256-2259 (V912 :2285-2288)"
                             : std::strcmp(r->control, "spbOneCycle") == 0 ? "cContact.cpp:16861-16865 (V912 :17152-17156)" : nullptr;
            if (want && std::strcmp(r->event, "click") == 0 && std::strcmp(r->shownObj, "fContact") == 0 && r->systemStart && r->softStart &&
                r->golden && std::strstr(r->golden, want) && std::strstr(r->golden, "main.cpp:27312-27327 (V912 :28302-28317)") && std::strstr(r->golden, "Show() :27324 (V912 :28314)")) ++match;
        }
        Check(rows == 3 && match == 3,
              "[7] table: the TfContact rows are exactly btnTStart / btnTStep / spbOneCycle (click, fContact, SystemStart + SoftStart, golden cites)");
        Quiet();
        fContact->bSetupStart = false; fContact->bSetupStep = false;
        SystemStart = true;
        W906_FormFShowHook = nullptr;
        Ack a = Click("btnTStart");
        Check(!a.sent && Starts(a.err, "running: ") && Has(a.err, "fContact") && !fContact->bSetupStart,
              "[7] SystemStart, no page table -> running (the row's reason names fContact), latch untouched");
        W906_FormFShowHook = &FakePageTable;
        g_open = "fOffSet";
        a = Click("btnTStart");
        Check(!a.sent && Starts(a.err, "running: ") && !fContact->bSetupStart, "[7] SystemStart, the page table says only fOffSet is open -> running");
        g_open = "fContact";
        a = Click("btnTStart");
        Check(a.sent && fContact->bSetupStart, "[7] SystemStart + Contact open: T.Start sets the latch");
        Check(WaitManualStartKey() && !fContact->bSetupStart, "[7] ... and the waiting flow takes it");
        a = Click("btnTStep");
        Check(a.sent && fContact->bSetupStep, "[7] SystemStart + Contact open: T.Step sets the latch");
        Check(WaitManualStepKey() && !fContact->bSetupStep, "[7] ... and the waiting flow takes it");
        const int mode0 = iContactMode;
        a = ClickRb(3);
        Check(!a.sent && Starts(a.err, "running: ") && iContactMode == mode0 && CheckedRadios() == "rbModeNormal",
              "[7] SystemStart: a mode radio is refused (not on the table; golden locks rgHandlerMode once a part is in, timerContact :21355 (V912 :21648)) -- mode unchanged");
        SystemStart = false;
        a = ClickRb(3);
        Check(a.sent && iContactMode == 3, "[7] stopped: Contact Test");
        SystemStart = true;
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 1, "[7] SystemStart + Contact open, CONTACT_TEST: One Cycle toggles on");
        SystemStart = false; SoftStart = true;
        a = Click("spbOneCycle");
        Check(a.sent && Led() == 0, "[7] SoftStart: One Cycle toggles off");
        a = Click("btnTStart");
        Check(a.sent && fContact->bSetupStart, "[7] SoftStart: T.Start sets the latch");
        SystemStart = true;
        a = Click("btnTStep");
        Check(a.sent && fContact->bSetupStep, "[7] SystemStart + SoftStart: T.Step sets the latch");
        a = Click("cbContactMode");
        Check(!a.sent && Starts(a.err, "running: "), "[7] a CT-1 control (cbContactMode) is still refused while running");
        IniConfig.bD16_StepContactTest = false;
        SystemStart = false; SoftStart = false;
        cJSON* page = Open();
        if (page) cJSON_Delete(page);
        SystemStart = true;
#ifndef SOFT_SIMULTE
        fContact->bSetupStart = false;
        a = Click("btnTStart");
        Check(!a.sent && Starts(a.err, "bad-payload") && !fContact->bSetupStart,
              "[7] ship build, [D16] off, running + Contact open: still bad-payload (the exception does not skip RunPageEvent's ELOperable)");
#else
        Check(true, "[7] SOFT_SIMULTE build: the [D16]-off hidden case is the ship build's (T.Start always visible here)");
#endif
        SystemStart = false; SoftStart = false;
        IniConfig.bD16_StepContactTest = true;
        W906_FormFShowHook = nullptr; g_open = nullptr;
        fContact->bSetupStart = false; fContact->bSetupStep = false;
    }

    // ---------------------------------------------------------------- [8]
    std::printf("[8] source ratchet (comments / '\\r' stripped, #if 0 skipped)\n");
    {
        const std::string root = argc > 1 ? argv[1] : "";
        const std::string web = argc > 2 ? argv[2] : "";
        std::string gen, cpp, fe, js;
        const bool ok = !root.empty() && ReadAll(root + "/FileRW/DeviceForm_File.gen.inc", &gen) && ReadAll(root + "/FileRW/DeviceForm_File.cpp", &cpp) &&
                        ReadAll(root + "/FileRW/_FormEvent.cpp", &fe) && !web.empty() && ReadAll(web + "/ht9045_contact_ev.js", &js);
        Check(ok, "[8] read the generated file, the entry cpp, _FormEvent.cpp and the page js");
        const std::vector<std::string> G = LiveLines(gen), C = LiveLines(cpp), F = LiveLines(fe), J = LiveLines(js);
        Check(LiveHas(G, "#define bSetupStart (fContact->bSetupStart)") && LiveHas(G, "#define bSetupStep (fContact->bSetupStep)") &&
              !LiveHas(G, "static bool bSetupStart") && !LiveHas(G, "static bool bSetupStep"),
              "[8] generated: bSetupStart / bSetupStep are fContact's (live #define), no C-route copy left");
        Check(LiveHas(Body(G, "static void DF_btnTStartClick()"), "bSetupStart=true;") && LiveHas(Body(G, "static void DF_btnTStepClick()"), "bSetupStep=true;"),
              "[8] generated: DF_btnTStartClick / DF_btnTStepClick bodies are live golden code");
        const std::vector<std::string> rb = Body(G, "static void DF_rbModeNormalClick(TObject *Sender)");
        Check(LiveHas(rb, "fAllMotorHome=false;") && LiveHas(rb, "DF_SetContactMode();") &&
              LiveHas(rb, "CUSTOMER_CODE==CC_ASE_KaohSiung && Ptr==EL<TRadioButton>(\"TfContact\", \"rbAutoHeight\")"),
              "[8] generated: DF_rbModeNormalClick live (ASE Kaohsiung Auto Height check, SetContactMode)");
        Check(LiveHas(Body(G, "static void DF_OneCycleProcess()"), "bContinueContact=!bContinueContact;") &&
              LiveHas(Body(G, "static void DF_spbOneCycleClick()"), "DF_OneCycleProcess();") &&
              LiveHas(Body(G, "static void DF_spbOneCycleClick()"), "\"ledOneCycle\")->Tag=bContinueContact;"),
              "[8] generated: One Cycle toggles and lights the LED (live)");
        Check(LiveHas(Body(G, "static void DF_FormShow()"), "FileRW_Contact_RbClickTrue(EL<TRadioButton>(\"TfContact\", \"rbModeNormal\"))") &&
              LiveHas(Body(G, "static void DF_FormClose()"), "FileRW_Contact_RbClickTrue(EL<TRadioButton>(\"TfContact\", \"rbModeNormal\"))"),
              "[8] generated: FormShow :1162 (V912 :1178) / FormClose :1871 (V912 :1898) go through VCL SetChecked(True)");
        int evRows = 0;
        for (const char* n : kRb) if (LiveHas(G, std::string("{\"") + n + "\", \"click\", \"cContact.cpp:15555 TfContact::rbModeNormalClick\", &DF_Ev_rbModeNormalClick}")) ++evRows;
        if (LiveHas(G, "{\"btnTStart\", \"click\", \"cContact.cpp:2280 TfContact::btnTStartClick\", &DF_Ev_btnTStartClick}")) ++evRows;
        if (LiveHas(G, "{\"btnTStep\", \"click\", \"cContact.cpp:2285 TfContact::btnTStepClick\", &DF_Ev_btnTStepClick}")) ++evRows;
        if (LiveHas(G, "{\"spbOneCycle\", \"click\", \"cContact.cpp:17152 TfContact::spbOneCycleClick\", &DF_Ev_spbOneCycleClick}")) ++evRows;
        Check(evRows == 14, "[8] generated: kDF_Events has the 14 CT-3a rows (" + std::to_string(evRows) + ")");
        Check(LiveHas(C, "t[i].handler = Ct3aOwns(kDF_Events[i].control) ? &Ct3aRun : &EvB3Run;"),
              "[8] entry cpp: EvB3Table routes the CT-3a rows to Ct3aRun (live, before the trailing //)");
        const std::vector<std::string> run = Body(C, "void Ct3aRun(TControl* sender)");
        Check(LiveHas(run, "FileRW_Contact_RbClickTrue(r);") && LiveHas(run, "e->handler(sender);") && LiveHas(run, "FileRW_Contact_TimerEPTick();"),
              "[8] entry cpp: Ct3aRun clicks radios through SetChecked(True), runs the golden handler otherwise, then the TimerEP beat");
        Check(LiveHas(F, "{\"TfContact\", \"btnTStart\",     \"click\", \"fContact\", true,        true,      kCtTStartGolden},") &&
              LiveHas(F, "{\"TfContact\", \"btnTStep\",      \"click\", \"fContact\", true,        true,      kCtTStepGolden},") &&
              LiveHas(F, "{\"TfContact\", \"spbOneCycle\",   \"click\", \"fContact\", true,        true,      kCtOneCycleGolden},"),
              "[8] _FormEvent.cpp: the 3 runexc rows are live code");
        //AI(W906-E030A) 20261003 [W906] (St01): E-030 ratchets (906 content in the generated bodies; see [3b] / [3c])
        {
            int greys = 0;
            for (const std::string& l : G) if (l.find("\"edContactHeight1\")->Enabled = false;") != std::string::npos || l.find("\"edContactHeight2\")->Enabled = false;") != std::string::npos) ++greys;
            Check(greys == 0 && !LiveHas(Body(G, "static void DF_SetContactMode()"), "CC_ASE_CL") && !LiveHas(Body(G, "static void DF_DoIniDataToForm()"), "CC_ASE_CL") &&
                  !LiveHas(Body(G, "static void DF_FormShow()"), "CC_ASE_CL && CosFunction.bContactShowOffset"),
                  "[8] generated: no live V912 Chrischen ASE-CL greying (" + std::to_string(greys) + " live ' = false' lines)");
            const std::vector<std::string> fs = Body(G, "static void DF_FormShow()");
            int amdM = 0;
            for (const std::string& l : fs) if (l.find("if(CUSTOMER_CODE==CC_AMD_M)") != std::string::npos) ++amdM;
            Check(amdM == 2 && LiveHas(fs, "if(CosFunction.bHiSiliconFunction==true && CUSTOMER_CODE==CC_AMD_M)") && !LiveHas(fs, "if(IniConfig.bAMDFunction)") &&
                  !LiveHas(fs, "&& IniConfig.bAMDFunction)") && LiveHas(fs, "\"rbVisualDetectionTest\")->Visible=(CUSTOMER_CODE==CC_AMD_M);") &&
                  !LiveHas(fs, "\"rbVisualDetectionTest\")->Visible=(IniConfig.bAMDFunction);"),   //AI(W906-E035) 20261003: was the V912 flag (never set here)
                  "[8] generated: FormShow's AMD conditions are 906's CUSTOMER_CODE==CC_AMD_M (x3); rbVisualDetectionTest stays (Q-C), its Visible is CUSTOMER_CODE==CC_AMD_M (E-035)");
        }
        Check(LiveHas(J, "try { hook(); reopenPorted(); ct3aLoad(); refreshKnown(); }") && LiveHas(J, "function ct3aLoad()") &&
              LiveHas(J, "el.classList.toggle('on', !!v.tag);"),
              "[8] page js: ct3aLoad runs after load (shows T.Start / T.Step, LED), the ack lights the LED (live, before the trailing //)");
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

    CloseGeneralIniFile();
    std::printf("\n%d / %d passed\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
