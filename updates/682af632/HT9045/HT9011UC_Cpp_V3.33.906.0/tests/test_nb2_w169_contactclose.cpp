// =============================================================================================
//  tests/test_nb2_w169_contactclose.cpp -- AI(W906-W169) 20261008 (NB2-1, README R266)
//
//  W-169 = the W-157 ruling (Steven 1008 07:5x, RULINGS_20261008 #1): a contact / auto-height run on the Contact page
//  (SystemStart and CarlibrationTask!=1) and the web's F5 / reload / disconnect / second tab.
//    (2a) a re-read of the page (editlist.get = golden FormShow) during the run does not run InitCarlibrationTask
//         (DeviceForm_File.gen.inc FormShow; tools/editlist/DeviceForm_File.py _W169_REPLACE; golden V912 cContact.cpp:1188)
//    (2b) the Contact window's close edge with the whole HMI screen gone (F5 / disconnect; W906_ContactScreenPresentHook false)
//         during the run does not run golden FormClose (FileRW/DeviceForm_File.cpp W906_ContactW169KeepOnReload)
//    (1)  an operator close on a live HMI (hook true), or no hook (other unit tests), or no run: golden FormClose as before
//    tag  W906_ContactRunBusy() = the tag contact.running the page uses to block F5 / ask before unload
//  Same harness as tests/test_b8_ct3a_contactflags.cpp (link-only stand-ins, the real golden FormShow / FormClose through
//  filerw::PageJson / FileRW_Contact_WindowEdge, sim motors).  Writes only the ctest sandbox (refuses outside ctest); compares
//  Gerneral.ini / ContactInfo.ini / Contact.ini / config.ini and the D:\HT9045\IniData file list.
//  REVERSE (done when this landed): drop the _W169_REPLACE row (regenerate) -> [2] red; drop the W906_ContactW169KeepOnReload call
//  in FileRW_Contact_WindowEdge -> [3] red.
// =============================================================================================
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
#include "atester_shims.h"
#include "forms/fContact.h"     // fContactForm (CarlibrationTask lives there; FileRW/DeviceForm_File.gen.inc #define)
#include "w906_ctest_guard.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
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

extern void FileRW_Contact_Boot();
extern const char* FileRW_Contact_WindowEdge(bool open);
extern void W906_EditPageWindowClosed(const char* goldenForm);
extern bool (*W906_FormFShowHook)(const char* goldenForm);
extern bool (*W906_ContactScreenPresentHook)();   // FileRW/DeviceForm_File.cpp EOF (W-169)
int W906_ContactRunBusy();                          // FileRW/DeviceForm_File.cpp EOF (W-169)
extern AnsiString DataPath;
extern AnsiString DefaultPath;
extern AnsiString asGeneralPath;
extern int iMaxLevelItem;
void OpenGeneralIniFile();
void CloseGeneralIniFile();

// Link-only (not under test) -- same as tests/test_b8_ct3a_contactflags.cpp.
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

bool Has(const char* s, const char* n) { return s && std::strstr(s, n) != nullptr; }

void Quiet()   // same as tests/test_b8_ct3a_contactflags.cpp: the manual-key inputs off
{
    Sen[SnRKManualStep].Enable = false;
    Sen[SnRKManualTStart].Enable = false;
    Sen[SnRKCoverOpen].Enable = false;
    Sen[SnRKSafeLock].Enable = false;
    bButtonManualStep = false; bButtonManualTStart = false; bSTEP = false; bTSTART = false;
    CosFunction.bEnableSoftWareControlButton = false;
}

// editlist.get (PageJson runs the real golden FormShow)
bool OpenOk()
{
    const filerw::PageDesc* d = filerw::FindPage("DeviceForm_File");
    if (!d) return false;
    std::string json;
    const int rc = filerw::PageJson(*d, &json);
    if (rc != 200) std::printf("    PageJson %d: %s\n", rc, json.c_str());
    return rc == 200;
}

bool g_screen = true;
bool FakeScreen() { return g_screen; }

}  // namespace

int main(int, char**)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("test_nb2_w169_contactclose -- W-169 (RULINGS_20261008 #1): Contact re-read / close edge during a contact / auto-height run\n");
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("NB2_W169ContactClose", rt))
            return 2;
    }
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\system\\ContactInfo.ini", "D:\\HT9045\\system\\Contact.ini",
                                  "D:\\HT9045\\config\\config.ini"};
    std::string before[4];
    bool had[4];
    for (int i = 0; i < 4; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    std::map<std::string, std::string> treeBefore;
    ListTree("D:\\HT9045\\IniData", "", &treeBefore);

    // ---------------------------------------------------------------- [0] boot (same as test_b8_ct3a_contactflags [0])
    std::printf("[0] boot + first open (golden FormShow)\n");
    SystemStart = false; SoftStart = false;
    CosFunction.bContactHeightSaveToContactIni = false;
    CosFunction.bUseDynamicKitDiameter = false;
    CUSTOMER_CODE = 0;
    Tri_Temp_Machine = 0;
    iMaxLevelItem = 180;
    for (int i = 0; i < 256; ++i) LevelSet.AccessLevel[i] = 0;
    AccessLevel = 4;
    Quiet();
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0, "[0] sim motors attached (" + std::to_string(made) + " axes)");
    OpenGeneralIniFile();
    FileRW_Contact_Boot();
    W906_FormFShowHook = nullptr;
    Check(OpenOk() && fContactForm->CarlibrationTask == 1, "[0] editlist.get = 200, golden FormShow InitCarlibrationTask -> CarlibrationTask 1");

    // ---------------------------------------------------------------- [1] not running: golden as before
    std::printf("[1] not running (SystemStart=0): a re-read runs InitCarlibrationTask; tag contact.running 0\n");
    fContactForm->CarlibrationTask = 5;
    Check(W906_ContactRunBusy() == 0, "[1] SystemStart=0, CarlibrationTask=5 -> contact.running 0");
    Check(OpenOk() && fContactForm->CarlibrationTask == 1, "[1] SystemStart=0: re-read resets CarlibrationTask to 1 (golden FormShow :1188)");

    // ---------------------------------------------------------------- [2] running: a re-read keeps the state machine
    std::printf("[2] running (SystemStart=1, CarlibrationTask=5): a re-read (second tab / back after F5) does not reset the run\n");
    SystemStart = true;
    fContactForm->CarlibrationTask = 5;
    Check(W906_ContactRunBusy() == 1, "[2] SystemStart=1, CarlibrationTask=5 -> contact.running 1");
    Check(OpenOk() && fContactForm->CarlibrationTask == 5, "[2] SystemStart=1, CarlibrationTask=5: re-read keeps CarlibrationTask 5 (W-169: no InitCarlibrationTask)");
    fContactForm->CarlibrationTask = 1;
    Check(W906_ContactRunBusy() == 0, "[2] SystemStart=1, CarlibrationTask=1 (no step running) -> contact.running 0");
    fContactForm->CarlibrationTask = 3;
    Check(OpenOk() && fContactForm->CarlibrationTask == 3, "[2] SystemStart=1, CarlibrationTask=3: kept too");

    // ---------------------------------------------------------------- [3] running, whole screen gone: no FormClose
    std::printf("[3] running, close edge with the whole HMI screen gone (F5 / disconnect)\n");
    W906_ContactScreenPresentHook = &FakeScreen;
    g_screen = false;
    SystemStart = true;
    fContactForm->CarlibrationTask = 5;
    const char* w = FileRW_Contact_WindowEdge(false);
    Check(Has(w, "not run (W-169)") && SystemStart && fContactForm->CarlibrationTask == 5,
          std::string("[3] FormClose not run: SystemStart stays 1, CarlibrationTask stays 5 -- ") + (w ? w : "(null)"));
    W906_EditPageWindowClosed("fContact");
    Check(OpenOk() && SystemStart && fContactForm->CarlibrationTask == 5, "[3] reopening Contact (re-read) keeps the run: SystemStart 1, CarlibrationTask 5");

    // ---------------------------------------------------------------- [4] running, operator close on a live HMI: golden FormClose
    std::printf("[4] running, close edge with the HMI screen present (operator closed Contact)\n");
    g_screen = true;
    SystemStart = true;
    fContactForm->CarlibrationTask = 5;
    w = FileRW_Contact_WindowEdge(false);
    Check(Has(w, "FormClose") && !Has(w, "W-169") && !SystemStart,
          std::string("[4] golden FormClose ran: SystemStart=false (closing Contact stops the machine) -- ") + (w ? w : "(null)"));
    W906_EditPageWindowClosed("fContact");

    // ---------------------------------------------------------------- [5] no hook (other unit tests): golden FormClose as before
    std::printf("[5] running, no screen hook installed\n");
    W906_ContactScreenPresentHook = nullptr;
    Check(OpenOk(), "[5] reopen");
    SystemStart = true;
    fContactForm->CarlibrationTask = 5;
    w = FileRW_Contact_WindowEdge(false);
    Check(Has(w, "FormClose") && !Has(w, "W-169") && !SystemStart, std::string("[5] no hook -> golden FormClose as before -- ") + (w ? w : "(null)"));
    W906_EditPageWindowClosed("fContact");

    // ---------------------------------------------------------------- [6] not running, screen gone: golden FormClose
    std::printf("[6] not running, close edge with the screen gone\n");
    W906_ContactScreenPresentHook = &FakeScreen;
    g_screen = false;
    Check(OpenOk(), "[6] reopen");
    SystemStart = false;
    fContactForm->CarlibrationTask = 1;
    w = FileRW_Contact_WindowEdge(false);
    Check(Has(w, "FormClose") && !Has(w, "W-169"), std::string("[6] no run -> golden FormClose (the W-169 skip is only for a run) -- ") + (w ? w : "(null)"));
    W906_EditPageWindowClosed("fContact");
    W906_ContactScreenPresentHook = nullptr;
    SystemStart = false;
    Check(g_lockDepth == 0, "[6] FormLock balanced");

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
