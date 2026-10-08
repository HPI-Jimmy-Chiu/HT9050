// =============================================================================
//  tests/test_st02_w156_flags.cpp -- AI(W906-W156) 20261007 (St02-E).  Suite: St02_W156Flags (both configs).
//
//  Card W-156 (W152-FLAGS): three Contact run flags had two or three copies (the C-route stand-ins of FileRW/DeviceForm_File.gen.inc,
//  the TfContact facade fContactForm that the translated contact SMs read since W-152, and the TfContactShim fContact).  One owner each
//  now, the facade:
//    [1] (a) bContinueContact: golden OneCycleProcess (0618 cContact.cpp:11741-11746) -- web One Cycle / panel ONE CYCLE -- flips
//        fContactForm->bContinueContact (the SMs' copy); below CONTACT_TEST (3) it is forced false
//    [2] (b) bSetHasIC: the pick SM sets fContactForm->bSetHasIC; the first read of a window-open clears it as golden FormShow (0618
//        :1651); a re-read in the same open keeps it (NOT GOLDEN glue: the port re-runs FormShow per editlist.get); FormClose clears it
//        (golden 0618 :1875-1879 / V912 :1902-1906, with the carry kits)
//    [3] (c) cbOneTouchAutoContactHight: ckernel WaitManualStepKey / WaitManualStartKey (golden ckernel.cpp:56-57 / :98-99) read the
//        facade box the run-start single entry fills (KYEC_LEE + Auto Height)
//    [4] (b2) the MODE reset (golden csystem.cpp:17690) bumps W906_ContactRunResultSeq once when the radio changes -> an open page
//        re-reads (tag contact.runResultSeq)
//    [5] (a2) KYEC_CHEN + A16: the MODE reset's programmatic cbContactMode change (golden ChangeContactMode 0618 :15436-15454) is not
//        an operator edit at the next save (EvB3Merge, FileRW/DeviceForm_File.cpp:1200) -> the drop offsets survive the save
//  Writes only ctest's sandbox (DataPath / OffsetPath / Gerneral.ini redirects; refuses an OffsetPath / DataPath under D:\HT9045 that
//  is not a build dir \obj\v906\).
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
#include "atester_shims.h"     // fContact (TfContactShim): fShow / bSetupStart, what ckernel reads beside the one-touch box
#include "ckernel.h"           // WaitManualStepKey / WaitManualStartKey
#include "IndexZTorque1203.h"
#include "w906_ctest_guard.h"
#include "Motor/mymotor.h"
#include "w906_test_motors.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern void FileRW_Contact_Boot();
extern bool (*W906_FormFShowHook)(const char* goldenForm);
extern AnsiString DataPath;
extern AnsiString DefaultPath;
extern AnsiString asGeneralPath;
extern int iMaxLevelItem;
extern bool bTSTART;
void OpenGeneralIniFile();
void FileRW_Contact_SetContactModeSingleEntry(int rbForTest);   // FileRW/DeviceForm_File.cpp (the contact-mode single entry, run start)
int  W906_ContactRunResultSeq();                                // FileRW/DeviceForm_File.cpp EOF (W-152 (B), W-156 (b2))
void W906_EditPageWindowClosed(const char* goldenForm);         // FileRW/_EditPage.cpp (the window-closed edge)
void FileRW_Contact_RbClickTrue(TRadioButton* self);            // FileRW/DeviceForm_File.cpp (VCL TRadioButton.SetChecked(True))
void W906_Contact_OneCycleProcess();                            // FileRW/DeviceForm_File.cpp (golden TfContact::OneCycleProcess)
const char* FileRW_Contact_WindowEdge(bool open);               // FileRW/DeviceForm_File.cpp (the close edge = golden FormClose)

// Link-only (not under test), same as tests/test_st02_w152_contact_data.cpp.
void FileRW_IniConfig_ChangeCBListProperty() {}
const char* W906_Main_sbContactClickOpen() { return nullptr; }
void FileRW_LdUld_Boot() { std::printf("link-only stand-in reached: FileRW_LdUld_Boot\n"); std::abort(); }
void FileRW_LdUld_ReadFile() { std::printf("link-only stand-in reached: FileRW_LdUld_ReadFile\n"); std::abort(); }
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
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
const char* const kTag = "DeviceForm_File";
const char* const kForm = "TfContact";
const int kContactTest = 3;   // golden cContact.cpp:77 CONTACT_TEST (deliberately absent from the port's cContact.h:161)

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
// [section] key=value from an ini file ("" when missing) -- as tests/test_st02_w152_contact_data.cpp
std::string IniValue(const std::string& path, const std::string& sec, const std::string& key)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string l;
    bool in = false;
    while (std::getline(f, l)) {
        if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
        if (!l.empty() && l[0] == '[') { in = (l == "[" + sec + "]"); continue; }
        if (in && l.compare(0, key.size() + 1, key + "=") == 0) return l.substr(key.size() + 1);
    }
    return "";
}
cJSON* Open()   // editlist.get = golden FormShow
{
    const filerw::PageDesc* d = filerw::FindPage(kTag);
    if (!d) return nullptr;
    std::string json;
    if (filerw::PageJson(*d, &json) != 200) { std::printf("    PageJson: %s\n", json.c_str()); return nullptr; }
    return cJSON_Parse(json.c_str());
}
void SetProxy(cJSON* page, const char* name, const char* key, cJSON* v)   // what the page shows (and sends) for one widget
{
    cJSON* px = page ? cJSON_GetObjectItemCaseSensitive(page, "proxies") : nullptr;
    cJSON* o = px ? cJSON_GetObjectItemCaseSensitive(px, name) : nullptr;
    if (!o) { cJSON_Delete(v); return; }
    if (cJSON_GetObjectItemCaseSensitive(o, key)) cJSON_ReplaceItemInObjectCaseSensitive(o, key, v);
    else cJSON_AddItemToObject(o, key, v);
}
std::string SaveWidgets(const cJSON* page)   // as tests/test_st02_w152_contact_data.cpp: every mustSend widget with the page's value
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
// the page showing the server's state for every proxy (what a re-read would show) -- WITHOUT re-reading (no new snapshot)
void ServerView(cJSON* page)
{
    cJSON* px = page ? cJSON_GetObjectItemCaseSensitive(page, "proxies") : nullptr;
    for (cJSON* o = px ? px->child : nullptr; o; o = o->next) {
        TControl* c = filerw::ELFind(kForm, o->string);
        if (!c) continue;
        if (TComboBox* cb = dynamic_cast<TComboBox*>(c)) {
            SetProxy(page, o->string, "itemIndex", cJSON_CreateNumber(cb->ItemIndex));
            SetProxy(page, o->string, "text", cJSON_CreateString(cb->Text.c_str()));
        } else if (TEdit* ed = dynamic_cast<TEdit*>(c)) {
            SetProxy(page, o->string, "text", cJSON_CreateString(ed->Text.c_str()));
        } else if (TRadioButton* rb = dynamic_cast<TRadioButton*>(c)) {
            SetProxy(page, o->string, "checked", cJSON_CreateBool(rb->Checked));
        } else if (TCheckBox* ck = dynamic_cast<TCheckBox*>(c)) {
            SetProxy(page, o->string, "checked", cJSON_CreateBool(ck->Checked));
        } else if (TRadioGroup* rg = dynamic_cast<TRadioGroup*>(c)) {
            SetProxy(page, o->string, "itemIndex", cJSON_CreateNumber(rg->ItemIndex));
        }
    }
}
bool g_fShowContact = false;
bool FShowHook(const char* f) { return g_fShowContact && std::strcmp(f, "fContact") == 0; }
}  // namespace

int main(int argc, char** argv)
{
    (void)argc; (void)argv;
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W156Flags -- one owner per Contact run flag (bContinueContact / bSetHasIC / cbOneTouchAutoContactHight) + MODE re-read + A16 save\n");
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "asGeneralPath", asGeneralPath.c_str(),
                                  "OffsetPath", OffsetPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("St02_W156Flags", rt)) return 2;
    }
    if (UnderMachineTree(OffsetPath.c_str()) || UnderMachineTree(DataPath.c_str())) {
        std::printf("  ABORT: OffsetPath %s / DataPath %s is under D:\\HT9045 and not a build dir -- nothing was called\n", OffsetPath.c_str(), DataPath.c_str());
        return 2;
    }

    // ---------------------------------------------------------------- [0] boot (as St02_W152ContactData [0])
    std::printf("[0] boot + first open (golden FormShow)\n");
    SystemStart = false; SoftStart = false;
    CosFunction.bContactHeightSaveToContactIni = false;   // golden would write D:\HT9045\system\Contact.ini (not sandboxed)
    CosFunction.bUseDynamicKitDiameter = false;           // golden TfContact ctor would read D:\HT9045\system\ContactInfo.ini
    CosFunction.bSaveOffsetByMachine = false;
    CosFunction.bEnableSoftWareControlButton = false;
    IniConfig.bE45_AllSetupFileUseOneFile = false;  IniConfig.bE59GroupOffsetFile = false;
    CUSTOMER_CODE = 0;
    Tri_Temp_Machine = 0;
    iMaxLevelItem = 180;
    for (int i = 0; i < 256; ++i) LevelSet.AccessLevel[i] = 0;
    AccessLevel = 4;
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0 && fContactForm != nullptr && fContact != nullptr, "[0] sim motors, the TfContact facade and the TfContactShim exist");
    OpenGeneralIniFile();
    FileRW_Contact_Boot();
    W906_FormFShowHook = nullptr;
    cJSON* page = Open();
    Check(page != nullptr, "[0] editlist.get DeviceForm_File (golden FormShow) = 200");
    if (page) cJSON_Delete(page);
    TfContact* F = fContactForm;

    // ---------------------------------------------------------------- [1] (a) bContinueContact
    std::printf("[1] (a) golden OneCycleProcess (web One Cycle / panel ONE CYCLE) flips the copy the contact SMs read\n");
    {
        F->bContinueContact = false;
        iContactMode = kContactTest;
        W906_Contact_OneCycleProcess();
        Check(F->bContinueContact == true, "[1] CONTACT_TEST: ONE CYCLE -> fContactForm->bContinueContact true (golden :11743-11744 bContinueContact=!bContinueContact)");
        W906_Contact_OneCycleProcess();
        Check(F->bContinueContact == false, "[1] ... and again -> false");
        F->bContinueContact = true;
        iContactMode = CONTACT_NORMAL;
        W906_Contact_OneCycleProcess();
        Check(F->bContinueContact == false, "[1] below CONTACT_TEST the golden OneCycleProcess forces it false (golden :11745)");
    }

    // ---------------------------------------------------------------- [2] (b) bSetHasIC
    std::printf("[2] (b) bSetHasIC: the pick SM's flag survives a re-read and is cleared by FormClose\n");
    {
        W906_EditPageWindowClosed("fContact");   // a fresh window-open
        F->bSetHasIC = true;
        page = Open();
        if (page) cJSON_Delete(page);
        Check(F->bSetHasIC == false, "[2] the first read of a window-open = golden FormShow: bSetHasIC=false (0618 :1651, V912 :1678)");
        F->bSetHasIC = true;   // what the pick SM does on a barcode / 2DID contact test (fContact_IndexPickPlace.cpp:340 / :365)
        page = Open();         // the W-152 re-read (or any editlist.get of the open page)
        if (page) cJSON_Delete(page);
        Check(F->bSetHasIC == true, "[2] a re-read in the same window-open keeps it (NOT GOLDEN glue: the port re-runs FormShow per editlist.get)");
        const char* w = FileRW_Contact_WindowEdge(false);
        Check(w != nullptr && std::strstr(w, "FormClose") != nullptr && F->bSetHasIC == false,
              std::string("[2] the close edge = golden FormClose: bSetHasIC -> carry kits SetAll(NULL_IC), bSetHasIC=false (0618 :1875-1879) -- ") + (w ? w : "(null)"));
        W906_EditPageWindowClosed("fContact");
        SystemStart = false;
    }

    // ---------------------------------------------------------------- [3] (c) cbOneTouchAutoContactHight
    std::printf("[3] (c) one-touch Auto Contact Height: ckernel reads the facade box the run-start single entry fills\n");
    {
        page = Open();
        if (page) cJSON_Delete(page);
        CUSTOMER_CODE = CC_KYEC_LEE;               // golden SetContactMode :15405-15411: KYEC_LEE + Auto Height shows the box
        g_fShowContact = true;  W906_FormFShowHook = &FShowHook;   // the Contact page is open (W906_FormShowing("fContact", ...))
        bTSTART = false;
        TCheckBox* stand = EL<TCheckBox>(kForm, "cbOneTouchAutoContactHight");
        stand->Checked = true;
        FileRW_Contact_SetContactModeSingleEntry(1);   // run start on Auto Height (the single entry copies the stand-in -> facade)
        Check(F->cbOneTouchAutoContactHight->Visible && F->cbOneTouchAutoContactHight->Checked, "[3] run start: the facade box is visible and ticked (as the web)");
        Check(WaitManualStepKey() == true, "[3] WaitManualStepKey: one-touch grants every step (golden ckernel.cpp:56-57)");
        fContact->bSetupStart = true;
        Check(WaitManualStartKey() == false && fContact->bSetupStart == true, "[3] WaitManualStartKey: one-touch refuses start, the latched T.Start is not consumed (golden :98-99)");
        stand->Checked = false;
        FileRW_Contact_SetContactModeSingleEntry(1);
        Check(!F->cbOneTouchAutoContactHight->Checked, "[3] control: unticked on the web -> the facade box unticked at the next run start");
        Check(WaitManualStartKey() == true && fContact->bSetupStart == false, "[3] control: without one-touch the latched T.Start is consumed (golden :103-121)");
        CUSTOMER_CODE = 0;  g_fShowContact = false;  W906_FormFShowHook = nullptr;  fContact->bSetupStart = false;
    }

    // ---------------------------------------------------------------- [4] (b2) MODE reset -> the open page re-reads
    std::printf("[4] (b2) MODE reset bumps the re-read tag once when the radio changes\n");
    {
        TRadioButton* rbN = EL<TRadioButton>(kForm, "rbModeNormal");
        TRadioButton* rbAH = EL<TRadioButton>(kForm, "rbAutoHeight");
        FileRW_Contact_RbClickTrue(rbAH);
        iContactMode = CONTACT_NORMAL;           // golden csystem.cpp:17689
        const int s0 = W906_ContactRunResultSeq();
        W906_ContactModeNormalHook();            // golden :17690 rbModeNormal->Checked=true
        Check(rbN->Checked && W906_ContactRunResultSeq() == s0 + 1, "[4] radio Auto Height -> Normal: seq +1 (the page's contact.runResultSeq re-read)");
        W906_ContactModeNormalHook();
        Check(W906_ContactRunResultSeq() == s0 + 1, "[4] already Normal (VCL: no Click): no second re-read");
    }

    // ---------------------------------------------------------------- [5] (a2) KYEC_CHEN + A16
    std::printf("[5] (a2) KYEC_CHEN + A16: the MODE reset's cbContactMode change is not an operator edit at the next save\n");
    {
        CUSTOMER_CODE = CC_KYEC_CHEN;
        IniConfig.bA16ContactTestDropContact = true;
        CosFunction.bFixedDropSpeed = false;
        FileRW_Contact_RbClickTrue(EL<TRadioButton>(kForm, "rbContactTest"));   // golden :15372-15376 -> ChangeContactMode(true)
        cJSON* p0 = Open();
        TComboBox* cbm = EL<TComboBox>(kForm, "cbContactMode");
        TEdit* d1 = EL<TEdit>(kForm, "edDropOffset1");
        TEdit* d2 = EL<TEdit>(kForm, "edDropOffset2");
        Check(p0 != nullptr && cbm->ItemIndex == DropContact && d1->Enabled, "[5] setup: Contact Test on KYEC_CHEN + A16 = Drop Contact, drop offsets editable");
        d1->Text = "1.50";  d2->Text = "1.60";
        iContactMode = CONTACT_NORMAL;
        W906_ContactModeNormalHook();            // golden :17690 -> rbModeNormalClick -> SetContactMode Normal arm :15346-15350 -> ChangeContactMode(false)
        Check(cbm->ItemIndex == DirectContactMode && !d1->Enabled, "[5] the MODE reset ran golden ChangeContactMode(false): Direct, drop offsets disabled");
        std::printf("    [5] after the MODE reset: edDropOffset1=\"%s\" edDropOffset2=\"%s\"\n", d1->Text.c_str(), d2->Text.c_str());
        // the page shows the server's state (radio Normal, Direct, drops 1.50 / 1.60 -- what the (b2) re-read would show) but has NOT
        // re-read: its snapshot is still the pre-reset one, so only EvB3Merge (DeviceForm_File.cpp:1200) tells the save that
        // cbContactMode moved by program, not by the operator
        ServerView(p0);
        std::string err;
        const int rc = Save(p0, &err);
        if (p0) cJSON_Delete(p0);
        // what was SAVED is the observable: golden cbContactModeChange (0618 :13997-14003 Direct / :14011-14014 KYEC_CHEN+A16) would have
        // written 0 into the boxes before SaveSetupFile wrote "Test Arm1/2 Drop" (:14260-14261)
        const std::string cdata = std::string(GetRecipePath().c_str()) + "Contact.Data";
        const bool sandboxed = !UnderMachineTree(cdata);
        const std::string s1 = sandboxed ? IniValue(cdata, "Test Arm1", "Drop") : "<not sandboxed>";
        const std::string s2 = sandboxed ? IniValue(cdata, "Test Arm2", "Drop") : "<not sandboxed>";
        Check(rc == 200 && s1 == "1.50" && s2 == "1.60",
              "[5] save: cbContactModeChange did not run -- Contact.Data keeps Drop " + s1 + " / " + s2 + " " + err);
        Check(std::string(d1->Text.c_str()) == "0.00" && std::string(d2->Text.c_str()) == "0.00",
              std::string("[5] ... and golden's post-save ReadFile shows 0 in Direct mode (0618 cContact.cpp:509-514 \"Direct Contact Mode 但是 Drop 不等於 0\") -- ") +
              d1->Text.c_str() + " / " + d2->Text.c_str());
        CUSTOMER_CODE = 0;  IniConfig.bA16ContactTestDropContact = false;
        FileRW_Contact_RbClickTrue(EL<TRadioButton>(kForm, "rbModeNormal"));
        W906_EditPageWindowClosed("fContact");
    }

    std::printf("St02_W156Flags: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
