// =============================================================================
//  test_st02_w152_contact_data.cpp -- W-152 (URGENT, Jimmy 1007): the Contact run's DATA PATHS between the web's C-route
//  stand-ins and the TfContact facade the translated contact / auto-height state machines use, plus golden's ClearIndexOffset.
//
//  AI(W906-W152) 20261007 (St02-E).  Suite name (add_test): St02_W152ContactData.  Nothing here moves anything and nothing changes
//  when the run can start (MainProc's call is not part of this card).
//    [1] (C) run start: FileRW_Contact_SetContactModeSingleEntry (DoTestContactFunction case 1, golden 0618 cContact.cpp:11790)
//        copies every recipe box the state machines read from the C-route stand-ins into the facade (23 edits, 2 combos, 8
//        checkboxes) -- before W-152 the facade boxes were never filled (facade DoIniDataToForm GATE W-02), so the run read "".
//    [2] (A) run end: W906_ContactRunResultHook (case 1800, golden :13725) copies the 17 boxes the state machines write back to
//        the stand-ins, and hands over bAutoHighFinish; source pins on the case-1800 call, the single entry and the generator.
//    [3] (B) keep-while-shown: a re-read of the OPEN page (editlist.get = golden FormShow) keeps the run result; after the
//        window closes, a fresh open shows the file value again (golden FormClose ReadFile + DoIniDataToForm :1847-1848).
//    [4] ClearIndexOffset: a Save after an AUTO HEIGHT result writes Contact.Data (the found height) and zeroes the index-arm
//        offsets in <OffsetPath>\<recipe>\Position OffSet.Data + Offset / Offset_File (golden :14156-14160, cOffSet.cpp:1817-1853).
//    [5] a Save after a run WITHOUT auto height (Contact Test) leaves the offsets alone.
//    [6] MODE (AI(W906-W152-MODE) 20261007 laptop; golden 0618 csystem.cpp:17687-17691): MainProc's fAllMotorHome==false reset reaches the
//        web's radio through W906_ContactModeNormalHook: the stand-in goes from Auto Height to Normal under FormLock, golden's Click
//        (rbModeNormalClick -> SetContactMode) runs once and not again on a second call; a re-read of the open page then shows Normal and
//        keeps iContactMode CONTACT_NORMAL (the control without the hook: the re-read puts Auto Height back), and the shown radio clicks again.
//  Writes only the ctest sandbox (DataPath / OffsetPath / Gerneral.ini are ctest's redirects; refuses otherwise and refuses any
//  OffsetPath under D:\HT9045 that is not a build dir \obj\v906\).  argv[1] = the port tree (read-only source pins).
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
#include "common.h"
#include "cContact.h"
#include "canary_support.h"
#include "forms/fMain.h"
#include "forms/fContact.h"
#include "forms/fOffSet.h"
#include "IndexZTorque1203.h"
#include "w906_ctest_guard.h"
#include "Motor/mymotor.h"
#include "w906_test_motors.h"
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern void FileRW_Contact_Boot();
extern bool (*W906_FormFShowHook)(const char* goldenForm);
extern AnsiString DataPath;
extern AnsiString DefaultPath;
extern AnsiString asGeneralPath;
extern int iMaxLevelItem;
void OpenGeneralIniFile();
void FileRW_Contact_SetContactModeSingleEntry(int rbForTest);   // FileRW/DeviceForm_File.cpp (the contact-mode single entry)
int  W906_ContactRunResultSeq();                                // FileRW/DeviceForm_File.cpp EOF (W-152)
void W906_EditPageWindowClosed(const char* goldenForm);         // FileRW/_EditPage.cpp (the window-closed edge)
void FileRW_Contact_RbClickTrue(TRadioButton* self);            // FileRW/DeviceForm_File.cpp (VCL TRadioButton.SetChecked(True) on a C-route radio)
void W906_ContactModeNormalToPage();                            // FileRW/DeviceForm_File.cpp EOF (W-152 MODE: registered as W906_ContactModeNormalHook)

// Link-only (not under test), same as tests/test_b8_ct3b_contactstartpause.cpp.
void FileRW_IniConfig_ChangeCBListProperty() {}
const char* W906_Main_sbContactClickOpen() { return nullptr; }
void FileRW_LdUld_Boot() { std::printf("link-only stand-in reached: FileRW_LdUld_Boot\n"); std::abort(); }
void FileRW_LdUld_ReadFile() { std::printf("link-only stand-in reached: FileRW_LdUld_ReadFile\n"); std::abort(); }
namespace {
int g_lockDepth = 0;
int g_lockCalls = 0;                                  // [6]: the MODE hook takes FormLock
int g_rbNormalAtLock = -1, g_rbNormalAtUnlock = -1;   // [6]: the rbModeNormal stand-in when the outermost lock is taken / released
int RbNormalNow()
{
    TRadioButton* r = dynamic_cast<TRadioButton*>(filerw::ELFind("TfContact", "rbModeNormal"));
    return r ? (r->Checked ? 1 : 0) : -1;
}
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
void FormLock() { if (g_lockDepth++ == 0) g_rbNormalAtLock = RbNormalNow(); ++g_lockCalls; }
void FormUnlock() { if (--g_lockDepth == 0) g_rbNormalAtUnlock = RbNormalNow(); }
}  // namespace formjson
}  // namespace ht9045

using filerw::EL;

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
const char* const kTag = "DeviceForm_File";
const char* const kForm = "TfContact";

std::string Lower(std::string s)
{
    for (std::size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
bool UnderMachineTree(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;
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
std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    std::istringstream in(text);
    std::string l;
    while (std::getline(in, l)) { if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1); v.push_back(l); }
    return v;
}
// [section] key=value from an ini file ("" when missing)
std::string IniValue(const std::string& path, const std::string& sec, const std::string& key)
{
    std::string text;
    if (!ReadAll(path, &text)) return "";
    bool in = false;
    for (const std::string& l : Lines(text)) {
        if (!l.empty() && l[0] == '[') { in = (l == "[" + sec + "]"); continue; }
        if (in && l.compare(0, key.size() + 1, key + "=") == 0) return l.substr(key.size() + 1);
    }
    return "";
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
std::string ProxyText(const cJSON* page, const char* name)
{
    const cJSON* px = page ? cJSON_GetObjectItemCaseSensitive(page, "proxies") : nullptr;
    const cJSON* o = px ? cJSON_GetObjectItemCaseSensitive(px, name) : nullptr;
    const cJSON* t = o ? cJSON_GetObjectItemCaseSensitive(o, "text") : nullptr;
    return t && cJSON_IsString(t) ? t->valuestring : std::string("<none>");
}
// a radio's "checked" in the page's proxies: 1 / 0, -1 = missing
int ProxyChecked(const cJSON* page, const char* name)
{
    const cJSON* px = page ? cJSON_GetObjectItemCaseSensitive(page, "proxies") : nullptr;
    const cJSON* o = px ? cJSON_GetObjectItemCaseSensitive(px, name) : nullptr;
    const cJSON* c = o ? cJSON_GetObjectItemCaseSensitive(o, "checked") : nullptr;
    return cJSON_IsBool(c) ? (cJSON_IsTrue(c) ? 1 : 0) : -1;
}
// what the browser sends on Save: every mustSend widget with the value the page shows (proxies)
std::string SaveWidgets(const cJSON* page)
{
    const cJSON* must = page ? cJSON_GetObjectItemCaseSensitive(page, "mustSend") : nullptr;
    const cJSON* px = page ? cJSON_GetObjectItemCaseSensitive(page, "proxies") : nullptr;
    cJSON* out = cJSON_CreateObject();
    for (const cJSON* it = must ? must->child : nullptr; it; it = it->next) {
        if (!cJSON_IsString(it)) continue;
        const cJSON* o = px ? cJSON_GetObjectItemCaseSensitive(px, it->valuestring) : nullptr;
        cJSON* w = cJSON_CreateObject();
        const char* keys[] = {"text", "checked", "itemIndex", "position", "activePageIndex"};
        for (const char* k : keys) {
            const cJSON* v = o ? cJSON_GetObjectItemCaseSensitive(o, k) : nullptr;
            if (v) cJSON_AddItemToObject(w, k, cJSON_Duplicate(v, 1));
        }
        cJSON_AddItemToObject(out, it->valuestring, w);
    }
    char* s = cJSON_PrintUnformatted(out);
    const std::string r = s ? s : "{}";
    if (s) cJSON_free(s);
    cJSON_Delete(out);
    return r;
}
int Save(const cJSON* page, std::string* err)
{
    const filerw::PageDesc* d = filerw::FindPage(kTag);
    std::string ack;
    return d ? filerw::PageSave(*d, SaveWidgets(page), "{}", &ack, err) : -1;
}

struct EditRow  { const char* name; TEdit*     TfContact::*m; };
struct ComboRow { const char* name; TComboBox* TfContact::*m; };
struct CheckRow { const char* name; TCheckBox* TfContact::*m; };
const EditRow kRecipe[] = {
    {"edContactHeight1", &TfContact::edContactHeight1}, {"edContactHeight2", &TfContact::edContactHeight2},
    {"edDropOffset1", &TfContact::edDropOffset1}, {"edDropOffset2", &TfContact::edDropOffset2},
    {"edContactOffsetArm1", &TfContact::edContactOffsetArm1}, {"edContactOffsetArm2", &TfContact::edContactOffsetArm2},
    {"edReleaseHeight1", &TfContact::edReleaseHeight1}, {"edReleaseHeight2", &TfContact::edReleaseHeight2},
    {"edPickUp1", &TfContact::edPickUp1}, {"edPickUp2", &TfContact::edPickUp2},
    {"edLoadCellHeight1", &TfContact::edLoadCellHeight1}, {"edLoadCellHeight2", &TfContact::edLoadCellHeight2},
    {"edShtPickOffset1", &TfContact::edShtPickOffset1}, {"edShtPickOffset2", &TfContact::edShtPickOffset2},
    {"edOrgPick1", &TfContact::edOrgPick1}, {"edOrgPick2", &TfContact::edOrgPick2},
    {"edContactBackUp1", &TfContact::edContactBackUp1}, {"edContactBackUp2", &TfContact::edContactBackUp2},
    {"edXDimension", &TfContact::edXDimension}, {"edYDimension", &TfContact::edYDimension},
    {"edD41", &TfContact::edD41}, {"edTestSec", &TfContact::edTestSec}, {"edTestContactCount", &TfContact::edTestContactCount},
};
const ComboRow kCombo[] = { {"cbContactMode", &TfContact::cbContactMode}, {"cbVacuumMode", &TfContact::cbVacuumMode} };
const CheckRow kCheck[] = {
    {"cbTestContactMode", &TfContact::cbTestContactMode}, {"chkTeachInOutArmZ", &TfContact::chkTeachInOutArmZ},
    {"chkShuttle", &TfContact::chkShuttle}, {"cbCalibrateAboveHeight", &TfContact::cbCalibrateAboveHeight},
    {"cbTeachInSHSen", &TfContact::cbTeachInSHSen}, {"cbSFCAutoTune", &TfContact::cbSFCAutoTune},
    {"cbRTCModel", &TfContact::cbRTCModel}, {"cbRTCAutoTuning", &TfContact::cbRTCAutoTuning},
};
const EditRow kResult[] = {
    {"edContactHeight1", &TfContact::edContactHeight1}, {"edContactHeight2", &TfContact::edContactHeight2},
    {"edLoadCellHeight1", &TfContact::edLoadCellHeight1}, {"edLoadCellHeight2", &TfContact::edLoadCellHeight2},
    {"edContactOffsetArm1", &TfContact::edContactOffsetArm1}, {"edContactOffsetArm2", &TfContact::edContactOffsetArm2},
    {"edShtPickOffset1", &TfContact::edShtPickOffset1}, {"edShtPickOffset2", &TfContact::edShtPickOffset2},
    {"edReleaseHeight1", &TfContact::edReleaseHeight1}, {"edReleaseHeight2", &TfContact::edReleaseHeight2},
    {"edPickUp1", &TfContact::edPickUp1}, {"edPickUp2", &TfContact::edPickUp2},
    {"edOrgPick1", &TfContact::edOrgPick1}, {"edOrgPick2", &TfContact::edOrgPick2},
    {"edContactBackUp1", &TfContact::edContactBackUp1}, {"edContactBackUp2", &TfContact::edContactBackUp2},
    {"edD41", &TfContact::edD41},
};
std::string Num(const char* p, int i) { char b[32]; std::snprintf(b, sizeof(b), "%s%d.%02d", p, 10 + i, i); return b; }
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W152ContactData -- W-152 data paths (C)(A)(B) + ClearIndexOffset (golden 0618 cContact.cpp :11790 / :13725 / :14156-14160, cOffSet.cpp:1817-1853)\n");
    const std::string src = argc > 1 ? argv[1] : "";
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "asGeneralPath", asGeneralPath.c_str(),
                                  "OffsetPath", OffsetPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("St02_W152ContactData", rt)) return 2;
    }
    if (UnderMachineTree(OffsetPath.c_str()) || UnderMachineTree(DataPath.c_str())) {
        std::printf("  ABORT: OffsetPath %s / DataPath %s is under D:\\HT9045 and not a build dir -- nothing was called\n", OffsetPath.c_str(), DataPath.c_str());
        return 2;
    }

    // ---------------------------------------------------------------- [0]
    std::printf("[0] boot + first open (golden FormShow)\n");
    SystemStart = false; SoftStart = false;
    CosFunction.bContactHeightSaveToContactIni = false;   // golden would write D:\HT9045\system\Contact.ini (not sandboxed)
    CosFunction.bUseDynamicKitDiameter = false;           // golden TfContact ctor would read D:\HT9045\system\ContactInfo.ini
    CosFunction.bSaveOffsetByMachine = false;             // ClearIndexOffset: the GetOffsetPath (OffsetPath, sandboxed) branch
    CosFunction.bEnableSoftWareControlButton = false;
    IniConfig.bE45_AllSetupFileUseOneFile = false;  IniConfig.bE59GroupOffsetFile = false;
    CUSTOMER_CODE = 0;
    Tri_Temp_Machine = 0;
    iMaxLevelItem = 180;
    for (int i = 0; i < 256; ++i) LevelSet.AccessLevel[i] = 0;
    AccessLevel = 4;
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0 && fContactForm != nullptr && fOffSet != nullptr && W906_ContactRunResultHook != nullptr,
          "[0] sim motors, the TfContact / TfOffSet facades and the registered run-end hook exist");
    OpenGeneralIniFile();
    FileRW_Contact_Boot();
    W906_FormFShowHook = nullptr;
    cJSON* page = Open();
    Check(page != nullptr, "[0] editlist.get DeviceForm_File (golden FormShow) = 200");
    const std::string fileH1 = EL<TEdit>(kForm, "edContactHeight1")->Text.c_str();
    if (page) cJSON_Delete(page);
    TfContact* F = fContactForm;

    // ---------------------------------------------------------------- [1] (C)
    std::printf("[1] (C) run start: the single entry copies the recipe boxes C-route -> facade\n");
    {
        for (std::size_t i = 0; i < sizeof(kRecipe) / sizeof(kRecipe[0]); ++i) { EL<TEdit>(kForm, kRecipe[i].name)->Text = Num("-", (int)i).c_str(); (F->*kRecipe[i].m)->Text = ""; }
        for (std::size_t i = 0; i < sizeof(kCombo) / sizeof(kCombo[0]); ++i) { EL<TComboBox>(kForm, kCombo[i].name)->ItemIndex = 2 + (int)i;  EL<TComboBox>(kForm, kCombo[i].name)->Text = kCombo[i].name;  (F->*kCombo[i].m)->ItemIndex = 0;  (F->*kCombo[i].m)->Text = ""; }
        for (std::size_t i = 0; i < sizeof(kCheck) / sizeof(kCheck[0]); ++i) { EL<TCheckBox>(kForm, kCheck[i].name)->Checked = true;  (F->*kCheck[i].m)->Checked = false; }
        FileRW_Contact_SetContactModeSingleEntry(1);   // rbAutoHeight, as the web radio
        int ok = 0, n = 0;
        std::string bad;
        for (std::size_t i = 0; i < sizeof(kRecipe) / sizeof(kRecipe[0]); ++i, ++n) {
            if (std::string((F->*kRecipe[i].m)->Text.c_str()) == Num("-", (int)i)) ++ok; else bad += std::string(" ") + kRecipe[i].name;
        }
        for (std::size_t i = 0; i < sizeof(kCombo) / sizeof(kCombo[0]); ++i, ++n) {
            if ((F->*kCombo[i].m)->ItemIndex == 2 + (int)i && std::string((F->*kCombo[i].m)->Text.c_str()) == kCombo[i].name) ++ok; else bad += std::string(" ") + kCombo[i].name;
        }
        for (std::size_t i = 0; i < sizeof(kCheck) / sizeof(kCheck[0]); ++i, ++n) {
            if ((F->*kCheck[i].m)->Checked) ++ok; else bad += std::string(" ") + kCheck[i].name;
        }
        Check(n == 33 && ok == 33, "[1] all 33 recipe controls (23 edits, cbContactMode / cbVacuumMode, 8 checkboxes) reached the facade (" +
                                   std::to_string(ok) + "/" + std::to_string(n) + ")" + (bad.empty() ? "" : " missing:" + bad));
        Check(iContactMode == CONTACT_AUTO_GET_HEIGHT, "[1] the single entry still sets the mode the web picked (CONTACT_AUTO_GET_HEIGHT)");
        Check(atof(F->edContactHeight1->Text.c_str()) + atof(F->edDropOffset1->Text.c_str()) != 0.0,
              "[1] the run's fDropPos inputs are no longer empty (fContact_AutoHeight.cpp:1290-1294 read \"\" before W-152)");
        Check(g_lockDepth == 0, "[1] FormLock balanced");
    }

    // ---------------------------------------------------------------- [2] (A)
    std::printf("[2] (A) run end: the case-1800 hook copies the result facade -> C-route\n");
    {
        for (std::size_t i = 0; i < sizeof(kResult) / sizeof(kResult[0]); ++i) { (F->*kResult[i].m)->Text = Num("-1", (int)i).c_str(); EL<TEdit>(kForm, kResult[i].name)->Text = "9"; }
        const int seq0 = W906_ContactRunResultSeq();
        W906_ContactRunResultHook(false);
        int ok = 0;
        std::string bad;
        for (std::size_t i = 0; i < sizeof(kResult) / sizeof(kResult[0]); ++i) {
            if (std::string(EL<TEdit>(kForm, kResult[i].name)->Text.c_str()) == Num("-1", (int)i)) ++ok; else bad += std::string(" ") + kResult[i].name;
        }
        Check(ok == 17, "[2] all 17 result boxes the state machines write reached the page's stand-ins (" + std::to_string(ok) + "/17)" + (bad.empty() ? "" : " missing:" + bad));
        Check(W906_ContactRunResultSeq() == seq0 + 1, "[2] the result sequence (the page's re-read trigger) advanced by one");
        Check(g_lockDepth == 0, "[2] FormLock balanced");
        std::string sm, dfc, gen;
        const bool r = ReadAll(src + "/forms/fContact_ContactSM.cpp", &sm) && ReadAll(src + "/FileRW/DeviceForm_File.cpp", &dfc) &&
                       ReadAll(src + "/FileRW/DeviceForm_File.gen.inc", &gen);
        int callAt = -1, n = 0, copyAt = -1, caseAt = -1;
        const std::vector<std::string> L = Lines(sm);
        for (std::size_t i = 0; i < L.size(); ++i) {
            if (L[i].find("W906_ContactRunResultHook(bAutoHighFinish); bAutoHighFinish=false;") != std::string::npos) { ++n; callAt = (int)i; }
            if (L[i].find("edOrgPick2->Text=edPickUp2->Text;") != std::string::npos) copyAt = (int)i;
            if (L[i].find("case 1900:") != std::string::npos && caseAt < 0) caseAt = (int)i;
        }
        Check(r && n == 1 && callAt > copyAt && copyAt > 0 && callAt < caseAt && L[(std::size_t)callAt].compare(0, 19, "            Task=1;") == 0,
              "[2] source: case 1800 calls the hook on its last line (golden :13725 `Task=1;`), after golden's :13680-13684 copies");
        bool entry = false, genLive = false;
        for (const std::string& l : Lines(dfc)) if (l.find("W906_ContactRecipeToFacade(F);") != std::string::npos && l.find("chk_K_Temperature") != std::string::npos) entry = true;
        const std::vector<std::string> G = Lines(gen);
        for (std::size_t i = 0; i < G.size(); ++i) if (G[i].compare(0, 32, "    fOffSet->ClearIndexOffset();") == 0) genLive = true;
        Check(entry && genLive, "[2] source: the single entry calls the (C) copy; the generated save path calls fOffSet->ClearIndexOffset() live (GATE O-6 retired)");
    }

    // ---------------------------------------------------------------- [3] (B)
    std::printf("[3] (B) keep-while-shown\n");
    {
        page = Open();   // the page is still open: a re-read
        Check(page && ProxyText(page, "edContactHeight1") == Num("-1", 0) && std::string(EL<TEdit>(kForm, "edContactHeight1")->Text.c_str()) == Num("-1", 0),
              "[3] re-read of the OPEN page: golden FormShow reloads the file, the run result is re-applied (" + ProxyText(page, "edContactHeight1") + ")");
        if (page) cJSON_Delete(page);
        W906_EditPageWindowClosed("fContact");
        page = Open();   // fresh open
        Check(page && ProxyText(page, "edContactHeight1") == fileH1,
              "[3] after the window closed, a fresh open shows the file value again (golden FormClose ReadFile + DoIniDataToForm) (" + ProxyText(page, "edContactHeight1") + " == " + fileH1 + ")");
        if (page) cJSON_Delete(page);
    }

    // ---------------------------------------------------------------- [4] ClearIndexOffset after an auto height
    std::printf("[4] Save after an AUTO HEIGHT result: Contact.Data gets the height, the index-arm offsets are cleared\n");
    const std::string offDir = fOffSet->GetOffsetPath().c_str();
    const std::string offFile = offDir + "\\Position OffSet.Data";
    if (UnderMachineTree(offFile) || Lower(offFile).find(Lower(OffsetPath.c_str())) != 0) {
        std::printf("  ABORT: the offset file %s is not under the sandboxed OffsetPath %s\n", offFile.c_str(), OffsetPath.c_str());
        return 2;
    }
    {
        MyForceDirectories(offDir.c_str());
        { std::ofstream f(offFile.c_str(), std::ios::binary); f << "[Test Arm1]\r\nPick Up=11\r\nPlace=12\r\nContact=123\r\n[Test Arm2]\r\nPick Up=21\r\nPlace=22\r\nContact=223\r\n"; }
        for (int a = 0; a < 2; ++a) { Offset.iIndexArmContact[a] = 12.5; Offset_File.iIndexArmContact[a] = 12.5; Offset.iIndexArmPickUp[a] = 3; Offset_File.iIndexArmPickUp[a] = 3; Offset.iIndexArmPlace[a] = 4; Offset_File.iIndexArmPlace[a] = 4; }
        fOffSet->IndexArmOffSet3->Text = "4";
        page = Open();
        if (page) cJSON_Delete(page);
        F->edContactHeight1->Text = "-77.70";
        for (std::size_t i = 1; i < sizeof(kResult) / sizeof(kResult[0]); ++i) (F->*kResult[i].m)->Text = EL<TEdit>(kForm, kResult[i].name)->Text;   // only the height changes
        W906_ContactRunResultHook(true);       // an auto-height run ended (case 805 had set bAutoHighFinish)
        page = Open();                         // the page re-reads (the tag trigger) and shows the result
        std::string err;
        const int rc = Save(page, &err);
        if (page) cJSON_Delete(page);
        Check(rc == 200, "[4] Save = 200 " + err);
        Check(std::fabs(atof(EL<TEdit>(kForm, "edContactHeight1")->Text.c_str()) - (-77.70)) < 0.001,
              "[4] after the save golden's ReadFile + DoIniDataToForm show the found height from Contact.Data (" + std::string(EL<TEdit>(kForm, "edContactHeight1")->Text.c_str()) + ")");
        Check(IniValue(offFile, "Test Arm1", "Contact") == "0" && IniValue(offFile, "Test Arm2", "Contact") == "0" &&
              IniValue(offFile, "Test Arm1", "Pick Up") == "0" && IniValue(offFile, "Test Arm2", "Place") == "0",
              "[4] ClearIndexOffset wrote 0 to " + offFile + " [Test Arm1/2] Pick Up / Place / Contact (golden cOffSet.cpp:1837-1839)");
        Check(IniValue(offFile, "Test Arm1", "Place") == "0" && Offset.iIndexArmPlace[0] == 0 && Offset_File.iIndexArmPlace[1] == 0 &&
              Offset.iIndexArmPickUp[1] == 0 && std::string(fOffSet->IndexArmOffSet1->Text.c_str()) == "0" && std::string(fOffSet->IndexArmOffSet3->Text.c_str()) == "0",
              "[4] and zeroed Offset / Offset_File iIndexArm* and IndexArmOffSet1..3 (golden :1841-1852)");
    }

    // ---------------------------------------------------------------- [5] a run without auto height
    std::printf("[5] Save after a run WITHOUT auto height (Contact Test): the offsets stay\n");
    {
        // Only ClearIndexOffset touches "Place", Offset*.iIndexArmPlace and IndexArmOffSet1..3: golden SaveSetupFile itself rewrites
        // [Test Arm1/2] Contact (and Pick Up on some rows) from the page's edContactOffsetArm / edShtPickOffset on EVERY save
        // (DeviceForm_File.gen.inc SaveSetupFile, golden :14205 ff), so Contact cannot tell whether ClearIndexOffset ran.
        { std::ofstream f(offFile.c_str(), std::ios::binary); f << "[Test Arm1]\r\nPick Up=11\r\nPlace=12\r\nContact=55\r\n[Test Arm2]\r\nPlace=22\r\n"; }
        Offset.iIndexArmPlace[0] = 7; Offset_File.iIndexArmPlace[0] = 7; fOffSet->IndexArmOffSet3->Text = "7";
        W906_ContactRunResultHook(false);
        page = Open();
        std::string err;
        const int rc = Save(page, &err);
        if (page) cJSON_Delete(page);
        Check(rc == 200 && IniValue(offFile, "Test Arm1", "Place") == "12" && IniValue(offFile, "Test Arm2", "Place") == "22" &&
              Offset.iIndexArmPlace[0] == 7 && Offset_File.iIndexArmPlace[0] == 7 && std::string(fOffSet->IndexArmOffSet3->Text.c_str()) == "7",
              "[5] no auto height -> golden's bAutoHighFinish stays false -> ClearIndexOffset does not run (Place=" + IniValue(offFile, "Test Arm1", "Place") +
              ", iIndexArmPlace=" + std::to_string(Offset.iIndexArmPlace[0]) + ") " + err);
    }

    // ---------------------------------------------------------------- [6] MODE (golden 0618 csystem.cpp:17687-17691)
    std::printf("[6] MODE: MainProc's fAllMotorHome==false reset reaches the web's radio (golden csystem.cpp:17690 rbModeNormal->Checked=true)\n");
    {
        TRadioButton* rbN = EL<TRadioButton>(kForm, "rbModeNormal");
        TRadioButton* rbAH = EL<TRadioButton>(kForm, "rbAutoHeight");
        TMemo* memo = EL<TMemo>(kForm, "Memo1");
        TCheckBox* daily = EL<TCheckBox>(kForm, "chkDailyCorrelation");
        auto NormalLines = [memo]() {   // golden SetContactMode's Normal arm adds one "CONTACT_NORMAL" memo line per run (golden 0618 cContact.cpp:15351)
            int n = 0;
            for (int i = 0; i < memo->Lines->Count; ++i) if (std::string(memo->Lines->GetString(i).c_str()) == "CONTACT_NORMAL") ++n;
            return n;
        };
        Check(W906_ContactModeNormalHook == &W906_ContactModeNormalToPage, "[6] the C-route registered the MODE hook (W906_ContactModeNormalHook = W906_ContactModeNormalToPage)");
        page = Open();   // the page is open: a re-read (golden FormShow once per open; the port re-imposes the picked radio)
        if (page) cJSON_Delete(page);
        FileRW_Contact_RbClickTrue(rbAH);   // the operator picks Auto Height (VCL SetChecked(True) -> rbModeNormalClick -> SetContactMode)
        Check(iContactMode == CONTACT_AUTO_GET_HEIGHT && rbAH->Checked && !rbN->Checked, "[6] setup: the web radio is on Auto Height (iContactMode CONTACT_AUTO_GET_HEIGHT)");
        // control: MainProc's reset WITHOUT the hook (golden :17689 alone) -- the next re-read of the open page re-runs golden FormShow's
        // SetContactMode on the radio the page still holds and puts Auto Height back
        iContactMode = CONTACT_NORMAL;
        page = Open();
        Check(page && ProxyChecked(page, "rbAutoHeight") == 1 && ProxyChecked(page, "rbModeNormal") == 0 && iContactMode == CONTACT_AUTO_GET_HEIGHT,
              "[6] control (no hook): the page keeps showing Auto Height and its re-read sets iContactMode back to CONTACT_AUTO_GET_HEIGHT");
        if (page) cJSON_Delete(page);
        // MainProc's contact arm after DoTestContactFunction with fAllMotorHome==false: golden :17689 (csystem.cpp:31477), then :17690 = the hook
        iContactMode = CONTACT_NORMAL;
        daily->Enabled = true; daily->Checked = true;   // a witness of the Click: golden SetContactMode outside Step Contact clears it (:15425-15433)
        const int n0 = NormalLines(), calls0 = g_lockCalls;
        g_rbNormalAtLock = g_rbNormalAtUnlock = -1;
        W906_ContactModeNormalHook();
        Check(rbN->Checked && !rbAH->Checked && iContactMode == CONTACT_NORMAL,
              "[6] the hook puts the web radio on Normal (rbModeNormal checked, Auto Height off); iContactMode stays CONTACT_NORMAL");
        Check(NormalLines() == n0 + 1 && !daily->Checked && !daily->Enabled,
              "[6] ... and runs golden's Click once: rbModeNormalClick -> SetContactMode (one CONTACT_NORMAL memo line, Daily Correlation cleared)");
        Check(g_lockCalls > calls0 && g_lockDepth == 0 && g_rbNormalAtLock == 0 && g_rbNormalAtUnlock == 1,
              "[6] ... under FormLock: Auto Height when the lock was taken, Normal when it was released; lock balanced");
        daily->Enabled = true; daily->Checked = true;
        const int calls1 = g_lockCalls;
        W906_ContactModeNormalHook();   // again: VCL SetChecked(True) on a radio that is already checked does not Click
        Check(NormalLines() == n0 + 1 && daily->Checked && daily->Enabled && rbN->Checked && !rbAH->Checked && g_lockCalls > calls1 && g_lockDepth == 0,
              "[6] a second call does not Click again (no new memo line, Daily Correlation untouched); the radio stays Normal");
        page = Open();
        Check(page && ProxyChecked(page, "rbModeNormal") == 1 && ProxyChecked(page, "rbAutoHeight") == 0 && iContactMode == CONTACT_NORMAL,
              "[6] the page's next read shows Normal and keeps iContactMode CONTACT_NORMAL (golden: the one form's radio jumps to Normal)");
        if (page) cJSON_Delete(page);
        FileRW_Contact_RbClickTrue(rbAH);
        Check(iContactMode == CONTACT_AUTO_GET_HEIGHT && rbAH->Checked, "[6] clicking Auto Height again is a real click now (iContactMode CONTACT_AUTO_GET_HEIGHT), not a no-op");
        FileRW_Contact_RbClickTrue(rbN);   // leave the radio on Normal
        daily->Checked = false;
    }

    std::printf("St02_W152ContactData: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
